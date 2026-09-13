#!/usr/bin/env python3

###
# Generates a ctx.c file, usable for "Context" on https://decomp.me.
#
# Usage:
#   python3 tools/decompctx.py src/file.cpp
#
# This is an include flattener, not a C preprocessor. Conditional compiler or
# system includes require the actual compiler's preprocessing path. For MWCC:
#   mwcceppc.exe <original defines/include/language flags> -P -EP <source> -o <context.i>
# Remove the original -c, -MMD and object-output options. For C, feed the result
# to m2c with -t ppc-mwcc-c --context context.i (ppc alone selects C++).
#
# If changes are made, please submit a PR to
# https://github.com/encounter/dtk-template
###

import argparse
import hashlib
import json
import os
import re
import sys
import tempfile
import locale
from pathlib import Path
from typing import List

script_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.abspath(os.path.join(script_dir, ".."))
src_dir = os.path.join(root_dir, "src")
include_dirs = [
    os.path.join(root_dir, "include"),
    # Add additional include directories here
]

include_pattern = re.compile(r'^#\s*include\s*[<"](.+?)[>"]\s*(?://.*|/\*.*?\*/\s*)?$')
guard_pattern = re.compile(r"^#ifndef\s+(.*)$")

class ContextError(ValueError):
    pass


class ImportState:
    def __init__(self, root=None):
        self.root = Path(root or root_dir).resolve()
        self.includes = [self.root / "include"]
        self.guards = set()
        self.active = []
        self.dependencies = set()
        self.bytes_read = 0

    def chain(self, leaf):
        return " -> ".join([*(str(p) for p in self.active), str(leaf)])


def _guard(text):
    # Recognize the conventional opening pair, not arbitrary #if semantics.
    prefix = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S).lstrip()
    match = re.match(r'#ifndef[ \t]+([A-Za-z_]\w*)[ \t]*\r?\n\s*#define[ \t]+\1(?:[ \t]*\r?\n)', prefix)
    return match[1] if match else None


def import_h_file(in_file: str, r_path: str, deps: List[str], state=None) -> str:
    state = state or ImportState()
    for path in [state.root / r_path / in_file, *(p / in_file for p in state.includes)]:
        if path.is_file():
            return import_c_file(str(path), deps, state)
    raise ContextError("Missing required include: " + state.chain(in_file))


def import_c_file(in_file: str, deps: List[str], state=None, *, root=None) -> str:
    state = state or ImportState(root)
    path = (state.root / in_file).resolve()
    if len(state.active) >= 128 or len(state.dependencies) >= 4096:
        raise ContextError("Include traversal bound exceeded: " + state.chain(path))
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise ContextError("Cannot read include chain: " + state.chain(path)) from exc
    state.bytes_read += len(raw)
    if state.bytes_read > 16 * 1024 * 1024:
        raise ContextError("Context input size bound exceeded")
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        text = raw.decode(locale.getpreferredencoding(False), errors="strict")
    relative = os.path.relpath(path, state.root)
    if path not in state.dependencies:
        state.dependencies.add(path)
        deps.append(relative)
    guard = _guard(text)
    if guard and guard in state.guards:
        return ""
    if path in state.active:
        raise ContextError("Unguarded include cycle: " + state.chain(path))
    if guard:
        state.guards.add(guard)
    state.active.append(path)
    try:
        return process_file(relative, text.splitlines(keepends=True), deps, state)
    finally:
        state.active.pop()


def process_file(in_file: str, lines: List[str], deps: List[str], state=None) -> str:
    state = state or ImportState()
    out_text = ""
    for idx, line in enumerate(lines):
        if idx == 0:
            print("Processing file", in_file)
        include_match = include_pattern.match(line.strip())
        if include_match and not include_match[1].endswith(".s"):
            out_text += f'/* "{in_file}" line {idx} "{include_match[1]}" */\n'
            out_text += import_h_file(include_match[1], os.path.dirname(in_file), deps, state)
            out_text += f'/* end "{include_match[1]}" */\n'
        else:
            out_text += line

    return out_text


def sanitize_path(path: str) -> str:
    return path.replace("\\", "/").replace(" ", "\\ ")


def _atomic(path, text):
    path = Path(path)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                         prefix=path.name+".", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(text)
        os.replace(temporary, path)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def _mask_c(text: str) -> str:
    """Keep offsets/lines while removing comments, literals and directives."""
    pattern = r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    masked = re.sub(pattern, lambda m: re.sub(r'[^\n]', ' ', m[0]), text, flags=re.S)
    return re.sub(r'^[ \t]*#(?:[^\n\\]|\\[^\n]|\\\n)*',
                  lambda m: re.sub(r'[^\n]', ' ', m[0]), masked, flags=re.M)


def declared_functions(text: str) -> dict[str, list[dict]]:
    """Locate ordinary C declarations/definitions, never infer a signature.

    This deliberately small scanner is a discovery aid, not a C/C++ parser.
    Macros, K&R definitions and unsupported declarators remain unresolved.
    The actual compiler must preprocess every proposed include before m2c.
    """
    masked = _mask_c(text)
    result: dict[str, list[dict]] = {}
    depth, start = 0, 0
    # extern "C" wrappers are transparent; other braces enclose types/bodies.
    wrappers: list[int] = []
    for match in re.finditer(r'[;{}]', masked):
        token, end = match[0], match.start()
        visible = depth == len(wrappers)
        fragment = masked[start:end].strip()
        if visible and token in ';{':
            declaration = re.fullmatch(
                r'([\w\s*]+?)\b([A-Za-z_]\w*)\s*\((.*)\)\s*', fragment, re.S)
            if declaration:
                prefix, name, arguments = declaration.groups()
                words = set(re.findall(r'\w+', prefix))
                if not words.intersection({'return', 'typedef', 'if', 'while', 'for', 'switch', 'else'}):
                    offset = start + len(masked[start:end]) - len(masked[start:end].lstrip())
                    result.setdefault(name, []).append({
                        'line': text.count('\n', 0, offset) + 1,
                        'declaration': re.sub(r'\s+', ' ', text[offset:end]).strip(),
                        'definition': token == '{',
                        'prototyped': bool(arguments.strip()),
                    })
            if token == '{' and fragment == 'extern':
                wrappers.append(depth)
        if token == '{':
            depth += 1
        elif token == '}':
            depth -= 1
            if wrappers and depth == wrappers[-1]:
                wrappers.pop()
        if depth == len(wrappers):
            start = match.end()
    return result


def _target_stack_captures(rows: list, label_rows: set[int]) -> list[dict]:
    """Narrow, whole-function straight-line review aid, not dead-store analysis.

    Only adjacent computed-value/call-result stores are candidates. Branches,
    stack escapes, unsupported instructions and overlapping accesses make the
    observation unknown. Ordinary calls can consume outgoing stack arguments:
    absence of an explicit reload here proves neither unused storage nor a
    local-slot classification, source identity, or callee behavior.
    """
    def record(i):
        return {'row': i, 'byte_offset': i * 4, 'instruction': rows[i][2]}

    widths = {'lbz': 1, 'lhz': 2, 'lha': 2, 'lwz': 4, 'lfs': 4, 'lfd': 8,
              'stb': 1, 'sth': 2, 'stw': 4, 'stfs': 4, 'stfd': 8}
    ordinary = {'li', 'lis', 'mr', 'mflr', 'mtlr', 'extsh', 'extsb', 'add',
                'addi', 'mulli', 'slwi', 'clrlwi', 'cmpwi', 'cmplwi', 'cmpw',
                'cmplw', 'andi.', 'ori', 'or', 'subf', 'fmr', 'fadds', 'fmuls',
                'fsubs', 'fcmpu', 'nop'}
    uncertainties, accesses = [], []
    frame = None
    for i, (op, args, _) in enumerate(rows):
        if i == 0 and op == 'stwu' and len(args) == 2 and args[0] == 'r1':
            match = re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)', args[1])
            if match and int(match[1], 0) < 0:
                frame = -int(match[1], 0)
                continue
        if op == 'blr' and i == len(rows) - 1:
            continue
        if op == 'bl' and len(args) == 1 and re.fullmatch(r'[A-Za-z_$][\w$]*', args[0]):
            if args[0].startswith(('_save', '_rest')):
                uncertainties.append((i, 'compiler helper stack effects'))
            continue
        if (op == 'addi' and args[:2] == ['r1', 'r1'] and len(args) == 3
                and frame is not None and args[2] in {str(frame), hex(frame)}
                and i == len(rows) - 2 and rows[-1][0] == 'blr'):
            continue
        if op in widths and len(args) == 2:
            memory = re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)', args[1])
            if memory:
                accesses.append((i, int(memory[1], 0), widths[op], op.startswith('l')))
                if args[0] == 'r1':
                    uncertainties.append((i, 'stack pointer read/write'))
                continue
            if re.search(r'\br1\b', ','.join(args)) or not re.fullmatch(r'.+\(r\d+\)', args[1]):
                uncertainties.append((i, 'unresolved memory/stack alias'))
            continue
        if op not in ordinary or re.search(r'\br1\b', ','.join(args)):
            uncertainties.append((i, 'control flow, unsupported instruction or stack alias'))
    if not rows or rows[-1][0] != 'blr' or frame is None:
        uncertainties.append((0, 'missing canonical frame/terminal return'))
    result = []
    for i, offset, width, is_load in accesses:
        op, args, _ = rows[i]
        if is_load or not i or op not in {'stw', 'stfs'} or offset < 8 or frame is None or offset + width > frame:
            continue
        prev_op, prev_args, _ = rows[i - 1]
        call = prev_op == 'bl' and args[0] in {'r3', 'f1'}
        computed = prev_op == 'add' and prev_args and prev_args[0] == args[0]
        if not (call or computed) or i in label_rows:
            continue
        overlaps = [j for j, start, size, _ in accesses
                    if j > i and start < offset + width and offset < start + size]
        issues = list(uncertainties)
        if overlaps:
            issues.append((overlaps[0], 'overlapping later stack read/write'))
        result.append({'store': record(i), 'producer': record(i - 1),
            'call': record(i - 1) if call else None,
            'stack_offset': offset, 'width_bytes': width,
            'status': 'unknown' if issues else 'no_direct_reload_observed',
            'uncertainty_count': len(issues),
            'uncertainties': [{'at': record(j), 'reason': why} for j, why in issues[:8]],
            'scope': 'complete supplied function; explicit straight-line stack accesses only',
            'review': 'No direct reload observed is not unused-storage or local-slot proof: calls may consume outgoing stack arguments. Review target-observed stores against source/decompiler output; never infer padding, fake locals, or original-source authentication.'})
    return result


def target_integer_shapes(assembly: str, function: str) -> dict:
    """Target-only MWCC reconstruction cues, available before any C compile.

    This is deliberately not C type inference. A sign-extending load specifies
    memory interpretation; a loop's compare specifies its observable narrowing.
    They need not specify the same width as a local that holds the loaded value.
    Only immediate, instruction-backed patterns receive a source-class hint.
    """
    if not isinstance(assembly, str) or len(assembly.encode()) > 8 * 1024 * 1024:
        raise ContextError('Bounded target assembly required')
    if not re.fullmatch(r'[A-Za-z_$][\w$]*', function):
        raise ContextError('Invalid target function')
    code = re.sub(r'/\*.*?\*/|#[^\n]*', '', assembly, flags=re.S)
    bodies = re.findall(r'^\s*\.fn\s+' + re.escape(function)
                        + r'\s*[^\n]*\n(.*?)^\s*\.endfn\b', code, re.M | re.S)
    if len(bodies) != 1:
        raise ContextError('Missing/ambiguous target function: ' + function)
    rows, labels = [], {}
    for line in bodies[0].splitlines():
        line = line.strip()
        if not line:
            continue
        if re.fullmatch(r'[.\w$]+:', line):
            if line[:-1] in labels:
                raise ContextError('Duplicate target label')
            labels[line[:-1]] = len(rows)
            continue
        match = re.fullmatch(r'([a-z][a-z0-9_.+-]*)(?:\s+(.+))?', line)
        if not match or len(rows) >= 8192:
            raise ContextError('Unsupported/bounds-exceeded target assembly row: ' + line)
        rows.append((match[1], [a.strip() for a in (match[2] or '').split(',')], line))
    def record(i):
        return {'row': i, 'instruction': rows[i][2]}
    terminal_branches = []
    # A narrow MWCC terminal shape: b to the very next instruction, a shared
    # conditional exit, then only verified frame/LR/GPR restoration and blr.
    # This is a review cue, not a return-vs-goto/source-spelling determination.
    def stack_offset(operand):
        match = re.fullmatch(r'(-?(?:0x[0-9a-f]+|\d+))\(r1\)', operand)
        return int(match[1], 0) if match else None
    frame = None
    if rows and rows[0][0] == 'stwu' and len(rows[0][1]) == 2 and rows[0][1][0] == 'r1':
        offset = stack_offset(rows[0][1][1])
        frame = -offset if offset is not None and offset < 0 else None
    if frame and len(rows) >= 7 and rows[-1][0] == 'blr':
        epilogue = len(rows) - 4
        tail = rows[epilogue:]
        if (tail[0][0] == 'lwz' and len(tail[0][1]) == 2 and tail[0][1][0] == 'r0'
                and stack_offset(tail[0][1][1]) == frame + 4
                and tail[1][:2] == ('mtlr', ['r0'])
                and tail[2][0] == 'addi' and tail[2][1] in (['r1', 'r1', str(frame)], ['r1', 'r1', hex(frame)])
                and any(op == 'stw' and args == ['r0', tail[0][1][1]] for op, args, _ in rows[:6])):
            helper = None
            if (epilogue >= 2 and rows[epilogue-1][0] == 'bl'
                    and len(rows[epilogue-1][1]) == 1
                    and re.fullmatch(r'_restgpr_(?:1[4-9]|2\d|3[01])', rows[epilogue-1][1][0])
                    and rows[epilogue-2][0] == 'addi'
                    and rows[epilogue-2][1] in (['r11', 'r1', str(frame)], ['r11', 'r1', hex(frame)])):
                helper = rows[epilogue-1][1][0]
                if any(op == 'bl' and args == [helper.replace('_restgpr_', '_savegpr_')]
                       for op, args, _ in rows[:8]):
                    epilogue -= 2
            if helper is None:
                while epilogue > 0:
                    op, args, _ = rows[epilogue-1]
                    if (op != 'lwz' or len(args) != 2 or not re.fullmatch(r'r(?:1[4-9]|2\d|3[01])', args[0])
                            or stack_offset(args[1]) is None or not 8 <= stack_offset(args[1]) < frame
                            or not any(saved_op == 'stw' and saved_args == args for saved_op, saved_args, _ in rows[:20])):
                        break
                    epilogue -= 1
            branch = epilogue - 1
            if branch >= 0 and rows[branch][0] == 'b' and len(rows[branch][1]) == 1:
                label = rows[branch][1][0]
                conditionals = [i for i, (op, args, _) in enumerate(rows[:branch])
                                if re.fullmatch(r'b(?:eq|ne|lt|le|gt|ge)(?:[+-])?', op) and args == [label]]
                if labels.get(label) == epilogue and conditionals:
                    terminal_branches.append({'branch': record(branch), 'target_label': label,
                        'epilogue_start': record(epilogue), 'terminal_return': record(len(rows)-1),
                        'conditional_predecessors': [record(i) for i in conditionals[:8]],
                        'conditional_predecessor_count': len(conditionals),
                        'review': 'Final unconditional branch reaches the immediately following shared epilogue. Review an explicit return at the end of the conditional source region; this does not distinguish return from goto or guarantee original spelling, return type, or argument count.'})
    loops, loads, captures, narrowed, returns, stack_bases, masks, array_indices = [], [], [], [], [], [], [], []
    # GNU adapter output labels every instruction. Only referenced branch
    # destinations are CFG boundaries; decorative labels are not joins.
    label_rows = {labels[arg] for opcode, operands, _ in rows
                  if opcode.startswith('b') for arg in operands if arg in labels}
    saved = re.compile(r'r(?:1[4-9]|2\d|3[01])$')
    for i, (op, args, _) in enumerate(rows):
        # Consecutive MWCC two-dimensional word-array address formation. Keep
        # the actual owner order: equal dimension lengths do not make the two
        # subscript meanings interchangeable (e.g. day/night versus team).
        if (op == 'slwi' and i + 6 < len(rows)
                and not any(j in label_rows for j in range(i + 1, i + 7))):
            window = rows[i:i + 7]
            if [r[0] for r in window] == ['slwi', 'lis', 'addi', 'add', 'slwi', 'add', 'lwz']:
                a, b, c, d, e, f, g = [r[1] for r in window]
                if ([len(x) for x in (a, b, c, d, e, f, g)] == [3, 2, 3, 3, 3, 3, 2]
                        and b[1].endswith('@ha') and c[2] == b[1][:-3] + '@l'
                        and c[1] == b[0] and d[1:] == [c[0], a[0]]
                        and f[1:] == [d[0], e[0]] and g[1] in {'0(' + f[0] + ')', '0x0(' + f[0] + ')'}):
                    try:
                        outer_shift, inner_shift = int(a[2], 0), int(e[2], 0)
                    except ValueError:
                        outer_shift, inner_shift = -1, -1
                    # Address-building temporaries must not kill the second
                    # index before it is scaled, or the first scale before use.
                    if (2 < outer_shift <= 10 and inner_shift == 2
                            and a[0] not in {b[0], c[0]}
                            and e[1] not in {a[0], b[0], c[0], d[0]}):
                        array_indices.append({'symbol': b[1][:-3], 'source_class': 'nested_array_index_order',
                            'outer_index_register': a[1], 'inner_index_register': e[1],
                            'row_stride_bytes': 1 << outer_shift, 'element_stride_bytes': 4,
                            'columns': 1 << (outer_shift - 2),
                            'outer_scale': record(i), 'inner_scale': record(i + 4), 'load': record(i + 6),
                            'outer_producer': record(i - 1) if i and rows[i - 1][1]
                                and rows[i - 1][1][0] == a[1] and rows[i - 1][0] in {'lha', 'lhz', 'lwz', 'lbz'} else None,
                            'reason': 'Preserve row-stride owner before element-stride owner; source names/types need the real global/member mapping.'})
        if op in {'lhz', 'lhzx', 'lha', 'lhax'} and len(args) >= 2:
            loads.append({**record(i), 'memory_bits': 16,
                          'extension': 'signed' if op.startswith('lha') else 'unsigned',
                          'destination': args[0]})
            if saved.fullmatch(args[0]):
                # A direct multiply before any next write is a local cue, not
                # an all-path lifetime or semantic source-owner proof.
                for j in range(i + 1, min(len(rows), i + 65)):
                    next_op, operands, _ = rows[j]
                    # A lexical scan must not join producers across a CFG join
                    # or branch. Ordinary ABI calls preserve these saved GPRs;
                    # indirect calls and restore helpers are not covered here.
                    if j in label_rows or (next_op.startswith('b') and next_op != 'bl'):
                        break
                    if next_op == 'bl' and (not operands or operands[0].startswith('_restgpr_')):
                        break
                    if next_op in {'mulli', 'slwi'} and len(operands) == 3 and operands[1] == args[0]:
                        captures.append({'register': args[0], 'load': record(i), 'use': record(j),
                            'source_class': 'promoted_int_capture',
                            'reason': 'Halfword load already extends the value; arithmetic consumes the saved value without another narrowing.',
                            'scope': 'lexical producer/use cue; review source assignment and incoming branches'})
                        break
                    if next_op in {'extsh', 'extsb', 'clrlwi'} and args[0] in operands[1:]:
                        if (next_op == 'extsh' and len(operands) == 2
                                and operands[1] == args[0] and j + 1 < len(rows)
                                and j + 1 not in label_rows):
                            use_op, use_args, _ = rows[j + 1]
                            if ((use_op in {'mulli', 'slwi'} and len(use_args) == 3
                                 and use_args[1] == operands[0])
                                    or (use_op == 'cmpwi' and len(use_args) == 2
                                        and use_args[0] == operands[0])):
                                narrowed.append({'register': args[0], 'load': record(i),
                                    'narrowing': record(j), 'use': record(j + 1),
                                    'source_class': 'signed_short_capture',
                                    'reason': 'The saved halfword value is narrowed again before arithmetic/comparison; do not widen it solely because lha already extends.',
                                    'scope': 'single lexical region; casts and actual source types still require review'})
                        break
                    if next_op in {'blr', 'bctr', 'blrl', 'bctrl', 'lmw'}:
                        break
                    if operands and operands[0] == args[0] and not next_op.startswith(('st', 'cmp', 'b')):
                        break
        if op == 'addi' and len(args) == 3 and args[1] == 'r1':
            stack_bases.append(record(i))
        if op == 'andi.' and len(args) == 3:
            masks.append(record(i))
        if op not in {'blt', 'blt+', 'blt-'} or len(args) != 1 or i < 2:
            continue
        body = labels.get(args[0])
        if body is None or body >= i:
            continue
        cmp_op, cmp_args, _ = rows[i-1]
        if cmp_op != 'cmpwi' or len(cmp_args) != 2:
            continue
        try:
            limit = int(cmp_args[1], 0)
        except ValueError:
            continue
        counter, kind, compare_start = cmp_args[0], 'int_counter', i-1
        prev_op, prev_args, _ = rows[i-2]
        if prev_op == 'extsh' and len(prev_args) == 2 and prev_args[0] == counter:
            counter, kind, compare_start = prev_args[1], 'signed_short_counter', i-2
        if not saved.fullmatch(counter) or not 0 < limit < 32767:
            continue
        increment = compare_start-1
        if increment < body or rows[increment][:2] not in (
                ('addi', [counter, counter, '1']), ('addi', [counter, counter, '0x1'])):
            continue
        initializers = [j for j in range(body) if rows[j][:2] in (
            ('li', [counter, '0']), ('li', [counter, '0x0']))]
        if not initializers:
            continue
        loops.append({'register': counter, 'source_class': kind, 'limit': limit,
                      'body_row': body, 'initialization': record(initializers[-1]),
                      'increment': record(increment), 'compare': record(i-1),
                      'narrowing': record(i-2) if kind == 'signed_short_counter' else None,
                      'branch': record(i),
                      'reason': 'Use the loop comparison width; call-argument casts do not determine counter width.'})
    nesting = []
    for loop in loops:
        parents = [outer for outer in loops
                   if outer['body_row'] < loop['body_row']
                   and loop['branch']['row'] < outer['branch']['row']]
        if parents:
            parent = min(parents, key=lambda outer: outer['branch']['row'] - outer['body_row'])
            nesting.append({'inner_register': loop['register'], 'outer_register': parent['register'],
                'inner_body_row': loop['body_row'], 'outer_body_row': parent['body_row'],
                'reason': 'Nested target back-edge intervals: review nested source loops before flattening indices.',
                'scope': 'lexical back-edge containment, not a full CFG/dominance proof'})
    # The final straight-line epilogue cannot prove a return type, but does
    # distinguish an explicit signed narrowing from a saved-value transfer.
    for i, (op, _, _) in enumerate(rows):
        if op != 'blr':
            continue
        for j in range(i-1, max(-1, i-24), -1):
            tail_op, args, _ = rows[j]
            if tail_op.startswith('b'):
                break
            if args and args[0] == 'r3' and not tail_op.startswith(('st', 'cmp')):
                if tail_op in {'mr', 'extsh', 'clrlwi'}:
                    returns.append({**record(j), 'kind': tail_op,
                                    'source_register': args[1] if len(args) > 1 else None})
                break
    return {'schema': 'decomp_target_integer_shapes/v1', 'function': function,
            'assembly_sha256': hashlib.sha256(assembly.encode()).hexdigest(),
            'instruction_count': len(rows), 'loops': loops, 'halfword_loads': loads,
            'promoted_captures': captures, 'narrowed_captures': narrowed,
            'loop_nesting': nesting, 'return_transfers': returns,
            'array_index_strides': array_indices,
            'stack_capture_reviews': _target_stack_captures(rows, label_rows),
            'terminal_branch_reviews': terminal_branches,
            'indexed_stack_bases': stack_bases, 'immediate_masks': masks,
            'caveat': 'Target cues, not original C types or source mappings. Review real consumers, header types and compiler mode. Never infer declaration order solely from stack offsets.',
            'authority_advanced': False, 'source_patch_emitted': False}


def discover_call_context(root: Path, assembly: str, context: str,
                          providers=()) -> dict:
    """Find missing direct-call prototypes and their real header/provider sites.

    A header hit is an include suggestion, not an ABI proof. All alternatives
    remain visible; no arbitrary first match or source-text-to-prototype rewrite.
    This is read-only and shared across constructors, callbacks and normal code.
    """
    root = Path(root).resolve()
    code = re.sub(r'/\*.*?\*/|#[^\n]*', '', assembly, flags=re.S)
    calls = sorted(set(re.findall(r'\bbl\s+([A-Za-z_$][\w$]*)\b', code)))
    locals_ = set(re.findall(r'^\s*\.fn\s+([\w$]+)', code, re.M))
    locals_.update(re.findall(r'^\s*([A-Za-z_$][\w$]*):', code, re.M))
    visible = declared_functions(context)
    runtime = {name for name in calls if re.fullmatch(r'_(?:save|rest)gpr_(?:1[4-9]|2\d|3[01])', name)}
    missing = [name for name in calls if name not in locals_ and name not in runtime
               and not any(row['prototyped'] for row in visible.get(name, []))]
    local_calls = sorted((set(calls) & locals_) - runtime)
    local_missing = [name for name in local_calls
                     if not any(row['prototyped'] for row in visible.get(name, []))]
    sought = sorted(set(missing) | set(local_missing))
    files = sorted(path for path in (root / 'include').rglob('*.h') if path.is_file())
    for provider in providers:
        path = (root / provider).resolve()
        if not path.is_relative_to(root) or not path.is_file():
            raise ContextError('Provider must be an existing file inside root: ' + str(path))
        files.append(path)
    if len(files) > 4096:
        raise ContextError('Call-context file limit exceeded')
    candidates: dict[str, list[dict]] = {name: [] for name in sought}
    read_bytes = 0
    for path in dict.fromkeys(files):
        resolved = path.resolve()
        if not resolved.is_relative_to(root):
            raise ContextError('Context dependency escapes root: ' + str(path))
        if resolved.stat().st_size > 32 * 1024 * 1024 - read_bytes:
            raise ContextError('Call-context input size limit exceeded: ' + str(path))
        raw = resolved.read_bytes()
        read_bytes += len(raw)
        if read_bytes > 32 * 1024 * 1024:
            raise ContextError('Call-context input size limit exceeded')
        text = raw.decode('utf-8', errors='replace')
        if not any(re.search(r'\b' + re.escape(name) + r'\b', text) for name in sought):
            continue
        declarations = declared_functions(text)
        relative = resolved.relative_to(root).as_posix()
        for name in sought:
            for row in declarations.get(name, []):
                if not row['prototyped']:
                    continue
                candidates[name].append({**row, 'path': relative,
                    'sha256': hashlib.sha256(raw).hexdigest(),
                    'role': 'header' if relative.startswith('include/') else 'provider'})
    local_candidates = {name: candidates[name] for name in local_missing}
    candidates = {name: candidates[name] for name in missing}
    include_hints = sorted({row['path'][8:] for rows in candidates.values()
                            for row in rows if row['role'] == 'header'})
    return_warnings = []
    for body in re.finditer(r'^\s*\.fn\s+([\w$]+)[^\n]*\n(.*?)^\s*\.endfn', code, re.M | re.S):
        name = body[1]
        declarations = visible.get(name, [])
        if not any(re.match(r'(?:(?:extern|static|inline)\s+)*void\s+' + re.escape(name) + r'\b',
                            row['declaration']) for row in declarations):
            continue
        instructions = [line.strip() for line in body[2].splitlines()
                        if line.strip() and not line.strip().endswith(':')]
        # A deliberately narrow cue: an explicit saved-GPR result transfer in
        # the final call-free return tail. Not generic return-type inference.
        tail = []
        for instruction in reversed(instructions):
            if re.match(r'b(?:l|ctr)\b', instruction) and not re.search(r'_(?:rest|save)gpr_', instruction):
                break
            tail.append(instruction)
        last_write = next((line for line in tail if re.match(
            r'(?:mr|li|lis|addi|addis|lwz|lha|lhz|lbz|add|subf|mullw|rlwinm|or|xor|andi\.)\s+r3,', line)), '')
        transfer = last_write if re.fullmatch(r'mr\s+r3,\s*r(?:1[4-9]|2\d|3[01])', last_write) else None
        if transfer and 'blr' in tail:
            return_warnings.append({'function': name, 'instruction': transfer,
                'context_declarations': declarations,
                'reason': 'void context conflicts with an explicit saved-register result transfer; '
                          'review the standalone helper return and its inlined callers. '
                          'This cue alone does not establish a return type.'})
    integer_shapes, shape_issues = [], []
    for name in dict.fromkeys(re.findall(r'^\s*\.fn\s+([\w$]+)', code, re.M)):
        try:
            integer_shapes.append(target_integer_shapes(assembly, name))
        except ContextError as exc:
            # A useful missing-prototype census must still work on an assembly
            # fragment. Optional shape analysis cannot gate reconstruction.
            shape_issues.append({'function': name, 'status': 'UNKNOWN', 'reason': str(exc)})
    return {'schema': 'decomp_call_context/v1',
            'assembly_sha256': hashlib.sha256(assembly.encode()).hexdigest(),
            'context_sha256': hashlib.sha256(context.encode()).hexdigest(),
            'calls': calls, 'local_calls': sorted(set(calls) & locals_),
            'compiler_helper_calls': sorted(runtime),
            'covered_calls': [name for name in calls if name not in locals_ and name not in missing and name not in runtime],
            'missing': candidates, 'include_hints': include_hints,
            'local_prototype_review': {
                'covered_calls': [name for name in local_calls if name not in local_missing],
                'missing': local_candidates,
                'present_without_prototype': [name for name in local_missing if name in visible],
                'unresolved': [name for name, rows in local_candidates.items() if not rows and name not in visible],
                'include_hints': sorted({row['path'][8:] for rows in local_candidates.values()
                                         for row in rows if row['role'] == 'header'}),
                'caveat': 'Same-assembly local calls still need visible typed context for decompilation. '
                          'Review existing callee declarations and bodies; live registers alone do not '
                          'prove argument count or validate decompiler-inferred extra arguments. '
                          'Assembly membership does not authenticate an original TU. '
                          'Discovery only, not a compilation gate or signature inference.',
                'authority_advanced': False},
            'present_without_prototype': [name for name in missing if name in visible],
            'return_warnings': return_warnings,
            'target_integer_shapes': integer_shapes, 'integer_shape_issues': shape_issues,
            'unresolved': [name for name, rows in candidates.items() if not rows and name not in visible],
            'caveat': 'Discovery only. Preprocess chosen headers with the actual compiler; '
                      'provider definitions and header alternatives require review. '
                      'No signature, constructor layout, or call arguments are inferred.'}


def adapt_target_function(data, metadata, disassembly, relocation_table, function):
    """Pure GNU-to-m2c input adapter; no builds, files, or inferred signatures.

    Inputs are bound to immutable target bytes. Returned assembly is evidence for
    reconstruction, not proof of source identity or m2c-inferred call arguments.
    GNU text must come from the caller's pinned objdump; subprocess/tool binding
    is deliberately outside this include/context utility.
    """
    if (not isinstance(data, bytes) or len(data) < 52 or data[:7] != b'\x7fELF\x01\x02\x01'
            or int.from_bytes(data[16:18], 'big') != 1
            or int.from_bytes(data[18:20], 'big') != 20
            or int.from_bytes(data[20:24], 'big') != 1
            or int.from_bytes(data[40:42], 'big') != 52):
        raise ValueError('target must be an ELF32 big-endian PowerPC relocatable object')
    if not isinstance(function, str) or not re.fullmatch(r'[A-Za-z_$][\w$]*', function):
        raise ValueError('unsupported function identifier')
    if not all(isinstance(text, str) for text in (metadata, disassembly, relocation_table)):
        raise ValueError('GNU evidence must be text')
    symbols = re.findall(r'^([0-9a-f]+)\s+\w\s+F\s+(\S+)\s+([0-9a-f]+)\s+(\S+)\s*$', metadata, re.M)
    selected = [s for s in symbols if s[3] == function]
    if len(selected) != 1:
        raise ValueError('missing/ambiguous function: ' + function)
    start, section, size, _ = selected[0]
    start, size = int(start, 16), int(size, 16)
    if size <= 0 or size % 4 or start % 4:
        raise ValueError('invalid function extent')
    section_rows = re.findall(r'^\s*\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+', metadata, re.M)
    section_info = [s for s in section_rows if s[0] == section]
    if len(section_info) != 1:
        raise ValueError('missing section extent')
    _, section_size, vma, offset = section_info[0]
    section_size, vma, offset = map(lambda s: int(s, 16), (section_size, vma, offset))
    if start < vma or start + size > vma + section_size:
        raise ValueError('function outside section')
    expected = data[offset + start - vma:offset + start - vma + size]
    if len(expected) != size:
        raise ValueError('function outside file')
    instructions, rows_reloc = {}, []
    active = None
    for line in disassembly.splitlines():
        header = re.match(r'Disassembly of section (.+):', line)
        if header:
            active = header[1]
        if active != section:
            continue
        match = re.match(r'\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+){4})(.+)', line)
        if match:
            address = int(match[1], 16)
            if start <= address < start + size:
                if address in instructions:
                    raise ValueError('duplicate instruction row')
                encoded = bytes.fromhex(match[2])
                if encoded != expected[address-start:address-start+4]:
                    raise ValueError('instruction bytes differ from target')
                instructions[address] = match[3].strip()
            continue
        match = re.match(r'\s*([0-9a-f]+):\s+(R_PPC_\w+)\s+(\S+)', line)
        if match and start <= int(match[1], 16) < start + size:
            rows_reloc.append((int(match[1], 16), match[2], match[3]))
    if list(instructions) != list(range(start, start + size, 4)):
        raise ValueError('incomplete/unordered instruction census')
    active, expected_reloc = None, []
    for line in relocation_table.splitlines():
        header = re.match(r'RELOCATION RECORDS FOR \[(.+)\]:', line)
        if header:
            active = header[1]
        match = re.match(r'([0-9a-f]+)\s+(R_PPC_\w+)\s+(\S+)', line)
        if active == section and match and start <= int(match[1], 16) < start + size:
            expected_reloc.append((int(match[1], 16), match[2], match[3]))
    if rows_reloc != expected_reloc:
        raise ValueError('disassembly/relocation census differs')
    relocated = set()
    for address, kind, symbol in rows_reloc:
        base = address & ~3
        if base not in instructions or base in relocated:
            raise ValueError('invalid/duplicate instruction relocation')
        relocated.add(base)
        if not re.fullmatch(r'[A-Za-z_.$][\w.$]*(?:[+-]0x[0-9a-fA-F]+)?', symbol):
            raise ValueError('unsupported relocation symbol expression: ' + symbol)
        inst = instructions[base]
        parts = inst.split(None, 1)
        if len(parts) != 2:
            raise ValueError('relocated instruction lacks operands')
        opcode, operands = parts
        word = int.from_bytes(expected[base-start:base-start+4], 'big')
        primary = word >> 26
        if kind == 'R_PPC_REL24':
            if (address != base or primary != 18 or word & 2
                    or opcode != ('bl' if word & 1 else 'b')):
                raise ValueError('unsupported REL24 instruction')
            inst = opcode + ' ' + symbol  # Preserve LK; never invent calls/arguments.
        elif kind in ('R_PPC_ADDR16_HA', 'R_PPC_ADDR16_HI', 'R_PPC_ADDR16_LO', 'R_PPC_EMB_SDA21'):
            suffix = {'R_PPC_ADDR16_HA': '@ha', 'R_PPC_ADDR16_HI': '@h',
                      'R_PPC_ADDR16_LO': '@l', 'R_PPC_EMB_SDA21': '@sda21'}[kind]
            if address - base != (0 if kind == 'R_PPC_EMB_SDA21' else 2):
                raise ValueError('invalid immediate relocation offset')
            memory_ops = {'lwz': 32, 'lwzu': 33, 'lbz': 34, 'lbzu': 35,
                          'stw': 36, 'stwu': 37, 'stb': 38, 'stbu': 39,
                          'lhz': 40, 'lhzu': 41, 'lha': 42, 'lhau': 43,
                          'sth': 44, 'sthu': 45, 'lmw': 46, 'stmw': 47,
                          'lfs': 48, 'lfsu': 49, 'lfd': 50, 'lfdu': 51,
                          'stfs': 52, 'stfsu': 53, 'stfd': 54, 'stfdu': 55}
            immediate_ops = {'addi': 14, 'li': 14, 'addis': 15, 'lis': 15,
                             'ori': 24, 'oris': 25}
            if primary != {**memory_ops, **immediate_ops}.get(opcode):
                raise ValueError('relocated opcode differs from target encoding')
            if kind == 'R_PPC_EMB_SDA21' and primary not in {14, 32, 34, 36, 38, 40, 42, 44, 48, 50, 52, 54}:
                raise ValueError('unsupported SDA21 instruction form')
            if kind == 'R_PPC_ADDR16_HA' and primary != 15:
                raise ValueError('unsupported ADDR16_HA instruction form')
            if kind == 'R_PPC_ADDR16_HI' and primary not in {15, 25}:
                raise ValueError('unsupported ADDR16_HI instruction form')
            if kind == 'R_PPC_ADDR16_LO' and primary not in {14, 24, *memory_ops.values()}:
                raise ValueError('unsupported ADDR16_LO instruction form')
            registers = re.fullmatch(r'([rf])(\d+),(-?\d+)\((?:r)?(\d+)\)', operands)
            if opcode in memory_ops:
                if (not registers or int(registers[2]) != (word >> 21) & 31
                        or int(registers[4]) != (word >> 16) & 31
                        or int(registers[3]) & 65535 != word & 65535
                        or registers[1] != ('f' if primary >= 48 else 'r')):
                    raise ValueError('relocated memory operands differ from target encoding')
            else:
                operands_parts = operands.split(',')
                expected_regs = ([(word >> 21) & 31] if opcode in ('li', 'lis') else
                                 [(word >> 16) & 31, (word >> 21) & 31] if primary in (24, 25) else
                                 [(word >> 21) & 31, (word >> 16) & 31])
                if (len(operands_parts) != len(expected_regs) + 1
                        or operands_parts[:-1] != ['r' + str(r) for r in expected_regs]
                        or not re.fullmatch(r'-?\d+', operands_parts[-1])
                        or int(operands_parts[-1]) & 65535 != word & 65535
                        or (opcode in ('li', 'lis') and (word >> 16) & 31)):
                    raise ValueError('relocated immediate operands differ from target encoding')
            # Preserve the actual base register and displacement form.
            memory = re.fullmatch(r'(.+,)(-?\d+)(\((?:r\d+|0)\))', operands)
            immediate = re.fullmatch(r'(.+,)(-?\d+)', operands)
            if memory:
                inst = opcode + ' ' + memory[1] + symbol + suffix + memory[3].replace('(0)', '(r0)')
            elif immediate and opcode in ('addi', 'addis', 'lis', 'li', 'ori', 'oris'):
                inst = opcode + ' ' + immediate[1] + symbol + suffix
                if kind == 'R_PPC_EMB_SDA21' and opcode == 'li':
                    inst = 'addi ' + immediate[1] + 'r0,' + symbol + suffix
            else:
                raise ValueError('unsupported relocated operand: ' + inst)
        else:
            raise ValueError('unsupported relocation: ' + kind)
        instructions[base] = inst
    assembly = ['.section .text', '.global ' + function, function + ':']
    branch_count = 0
    for address, inst in instructions.items():
        branch = re.search(r'\b([0-9a-f]+) <([^>]+)>', inst)
        if branch:
            destination = int(branch[1], 16)
            if not inst.split()[0].startswith('b') or destination not in instructions:
                raise ValueError('unsupported external/noninstruction branch: ' + inst)
            word = int.from_bytes(expected[address-start:address-start+4], 'big')
            opcode = word >> 26
            if opcode not in (16, 18):
                raise ValueError('branch text not a direct branch encoding')
            bits = 26 if opcode == 18 else 16
            displacement = word & ((1 << bits) - 4)
            if displacement & (1 << (bits - 1)):
                displacement -= 1 << bits
            encoded_destination = displacement if word & 2 else address + displacement
            if destination != encoded_destination:
                raise ValueError('branch destination differs from target encoding')
            branch_count += 1
            inst = inst[:branch.start()] + f'.L_{function}_{destination:x}'
        if '<' in inst or inst.startswith(('.long', '.word')):
            raise ValueError('unresolved/unsupported instruction: ' + inst)
        assembly.extend([f'.L_{function}_{address:x}:', f'/* {address:08x} */ {inst}'])
    text = '\n'.join(assembly) + '\n'
    shapes = target_integer_shapes('.fn ' + function + ', global\n'
                                   + '\n'.join(assembly[3:]) + '\n.endfn\n', function)
    return text, {'function': function, 'bytes': size,
        'target_sha256': hashlib.sha256(data).hexdigest(),
        'inference_caveat': 'm2c source and call arguments remain inferred; no callee signatures supplied',
        'instructions': len(instructions), 'internal_branches': branch_count,
        'target_integer_shapes': shapes,
        'instruction_bytes_sha256': hashlib.sha256(expected).hexdigest(), 'relocations': rows_reloc}


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Create a decomp.me context by flattening includes, not preprocessing C.",
        epilog="Conditional compiler/system includes require the actual compiler: "
               "MWCC <original defines/include/language flags> -P -EP <source> -o <context.i> "
               "(remove -c, -MMD and object-output options). For C use m2c "
               "-t ppc-mwcc-c --context context.i; ppc alone selects C++.",
    )
    parser.add_argument(
        "c_file",
        help="""File from which to create context""",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="""Output file""",
        default="ctx.c",
    )
    parser.add_argument(
        "-d",
        "--depfile",
        help="""Dependency file""",
    )
    parser.add_argument("--root", type=Path, default=Path(root_dir), help="Source/include/output root")
    parser.add_argument("--calls-from", type=Path,
                        help="Audit direct calls in target assembly against c_file (a compiler-preprocessed context); output JSON")
    parser.add_argument("--provider", action="append", default=[],
                        help="Root-relative provider C file to inspect for APIs missing public headers; repeatable")
    args = parser.parse_args(argv)

    deps = []
    try:
        if args.calls_from:
            report = discover_call_context(args.root,
                (args.root / args.calls_from).read_text(encoding='utf-8'),
                (args.root / args.c_file).read_text(encoding='utf-8'), args.provider)
            _atomic(args.root / args.output, json.dumps(report, indent=2) + '\n')
            print(f"Call context: {len(report['covered_calls'])} covered; "
                  f"{len(report['missing'])} need context/prototype review; {len(report['unresolved'])} unresolved; "
                  f"{len(report['compiler_helper_calls'])} compiler save/restore helpers. "
                  f"{len(report['return_warnings'])} return-contract warnings. "
                  f"Report: {args.output}")
            return 0
        if args.provider:
            raise ContextError('--provider requires --calls-from')
        output = import_c_file(args.c_file, deps, root=args.root)
        dependency_text = sanitize_path(args.output) + ":" + "".join(f" \\\n\t{sanitize_path(dep)}" for dep in deps)
        _atomic(args.root / args.output, output)
        if args.depfile:
            _atomic(args.root / args.depfile, dependency_text)
    except (OSError, UnicodeError, ContextError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
