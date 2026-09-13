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
    return '\n'.join(assembly) + '\n', {'function': function, 'bytes': size,
        'target_sha256': hashlib.sha256(data).hexdigest(),
        'inference_caveat': 'm2c source and call arguments remain inferred; no callee signatures supplied',
        'instructions': len(instructions), 'internal_branches': branch_count,
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
    args = parser.parse_args(argv)

    deps = []
    try:
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
