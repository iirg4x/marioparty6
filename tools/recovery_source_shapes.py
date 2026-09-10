"""Bounded, reviewed function-island shapes; never compile or mutate source."""
from __future__ import annotations
import hashlib
import importlib.metadata
import json
import re
import sys
from pathlib import Path

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import tree_sitter
import tree_sitter_c

from tools.recovery_call_contract_repair import _function_span
from tools import recovery_snapshot_repair as snapshot

FAMILIES = {"snapshot_point_use", "loop_predicate_guards", "sequence_result_consumer", "indexed_base_snapshot", "sequence_scalar_argument", "shared_initializer_producer"}


def analyze_shared_initializers(source_bytes, function, *, closed_macro_context=False,
                                reviewed_type_aliases=None, reviewed_constants=None,
                                target_producer=None, max_sites=32):
    """Census source assignment flow, not compiler ownership or original C identity.

    External typedef/constant reviews are explicit caller contracts. Only unique
    enclosing automatic scalar/fixed-array declarations are supported; globals,
    escaped storage and incomplete evidence remain UNKNOWN. Target evidence is
    one hash-bound statement and one desired producer, never a permutation list.
    """
    if type(max_sites) is not int or not 1 <= max_sites <= 64:
        raise ValueError('max_sites must be 1..64')
    start, _, raw, nodes = _island(source_bytes, function)
    textof = lambda n: raw[n.start_byte:n.end_byte].decode('ascii')
    aliases, constants = reviewed_type_aliases or {}, reviewed_constants or {}
    source_hash = _sha(source_bytes)
    masked_source = snapshot.mask_c(source_bytes.decode('latin-1'))
    if target_producer is not None and target_producer.get('source_sha256') != source_hash:
        raise ValueError('target producer source binding is stale')
    sites = []
    for statement in nodes:
        if statement.type != 'expression_statement' or len(statement.named_children) != 1:
            continue
        expr = statement.named_children[0]
        lefts = []
        while expr.type == 'assignment_expression' and textof(expr.child_by_field_name('operator')) == '=':
            lefts.append(expr.child_by_field_name('left'))
            expr = expr.child_by_field_name('right')
        if len(lefts) < 2:
            continue
        if len(sites) >= max_sites:
            raise ValueError('shared initializer census exceeds bound')
        diagnostics, owners, types, storage_keys = [], [], [], []
        if closed_macro_context is not True:
            diagnostics.append('UNKNOWN: closed macro context not reviewed')
        rhs = textof(expr)
        macro_values = re.findall(r'^\s*#\s*define\s+'+re.escape(rhs)+r'\b([^\n]*)', masked_source, re.M)
        if macro_values and (len(macro_values) != 1 or macro_values[0].strip() != '0'):
            diagnostics.append('UNKNOWN: conflicting constant macro')
        # Zero is representable in every supported integer type. No floating,
        # pointer, conversion-chain or arithmetic equivalence is inferred.
        if not (expr.type == 'number_literal' and rhs == '0') and not (
                expr.type in {'identifier', 'false'} and constants.get(rhs) == 0 and type(constants.get(rhs)) is int):
            diagnostics.append('UNKNOWN: RHS is not reviewed integral zero')
        for left in lefts:
            base, index = left, None
            if left.type == 'subscript_expression':
                base, idx = left.child_by_field_name('argument'), left.child_by_field_name('index')
                if idx.type == 'number_literal' and re.fullmatch(r'[0-9]+', textof(idx)):
                    index = int(textof(idx))
                else:
                    diagnostics.append('UNKNOWN: side-effecting or nonconstant subscript')
            if base.type != 'identifier' or left.type not in {'identifier', 'subscript_expression'}:
                diagnostics.append('UNKNOWN: indirect or side-effecting lvalue')
                continue
            name = textof(base)
            storage_keys.append((name, index))
            if re.search(r'^\s*#\s*define\s+'+re.escape(name)+r'\b', masked_source, re.M):
                diagnostics.append('UNKNOWN: macro lvalue '+name)
            owners.append(name)
            decls = [n for n in nodes if n.type in {'declaration', 'parameter_declaration'}
                     and any(c.type == 'identifier' and textof(c) == name for c in _nodes(n))]
            if len(decls) != 1:
                diagnostics.append('UNKNOWN: missing/global or shadowed declaration '+name)
                continue
            decl = decls[0]
            match = re.fullmatch(r'\s*([A-Za-z_]\w*)\s+'+re.escape(name)+r'\s*(?:\[\s*([0-9]+)\s*\])?\s*;', textof(decl))
            if (not match or decl.type != 'declaration' or decl.parent.type != 'compound_statement'
                    or decl.end_byte > statement.start_byte
                    or not decl.parent.start_byte <= statement.start_byte < decl.parent.end_byte):
                diagnostics.append('UNKNOWN: qualified, indirect or nonlocal storage '+name)
                continue
            typ, count = match.groups()
            resolved = aliases.get(typ, typ)
            if typ in {'int', 'short', 'char', 'long', 'unsigned', 'signed'} and resolved != typ:
                diagnostics.append('UNKNOWN: builtin type cannot be overridden')
            typedefs = re.findall(r'\btypedef\s+([^;]+)\b'+re.escape(typ)+r'\s*;', masked_source)
            if typedefs and (len(typedefs) != 1 or typedefs[0].strip() != resolved):
                diagnostics.append('UNKNOWN: conflicting or qualified typedef '+typ)
            if resolved not in {'int', 'short', 'char', 'long', 'unsigned', 'signed'}:
                diagnostics.append('UNKNOWN: incomplete integral type evidence '+typ)
            types.append({'owner': name, 'spelling': typ, 'reviewed_type': resolved})
            if (index is None) != (count is None) or (index is not None and index >= int(count or 0)):
                diagnostics.append('UNKNOWN: array shape/bounds ambiguity '+name)
            for use in nodes:
                if use.type != 'identifier' or textof(use) != name or decl.start_byte <= use.start_byte < decl.end_byte:
                    continue
                if count is not None and not (use.parent.type == 'subscript_expression'
                        and use.parent.child_by_field_name('argument') == use):
                    diagnostics.append('UNKNOWN: array escape '+name)
                ancestor = use.parent
                while ancestor.type in {'parenthesized_expression', 'subscript_expression'}:
                    ancestor = ancestor.parent
                if ancestor.type == 'pointer_expression' and textof(ancestor).lstrip().startswith('&'):
                    diagnostics.append('UNKNOWN: address escape '+name)
                if use.parent.type == 'subscript_expression' and use.parent.parent.type == 'pointer_expression':
                    diagnostics.append('UNKNOWN: array element address escape '+name)
        lhs = [textof(n) for n in lefts]
        if len({t['reviewed_type'] for t in types}) > 1:
            diagnostics.append('UNKNOWN: differing value conversion types')
        if len(set(lhs)) != len(lhs) or len(set(storage_keys)) != len(storage_keys):
            diagnostics.append('UNKNOWN: overlapping recipients')
        span = {'start_byte': start+statement.start_byte, 'end_byte': start+statement.end_byte,
                'sha256': _sha(raw[statement.start_byte:statement.end_byte])}
        selected = False
        if target_producer is not None and all(target_producer.get(k) == v for k, v in span.items()):
            observed = target_producer.get('observed', {})
            if (target_producer.get('producer') not in lhs or not target_producer.get('rationale')
                    or not re.fullmatch(r'[0-9a-f]{64}', observed.get('artifact_sha256', ''))
                    or not isinstance(observed.get('location'), str) or not observed['location']
                    or not isinstance(observed.get('fact'), str) or not observed['fact']):
                raise ValueError('target evidence requires existing producer and rationale')
            selected = True
        sites.append({**span, 'lvalues': lhs, 'producer': lhs[-1], 'recipients': list(reversed(lhs[:-1])),
                      'rhs': rhs, 'types': types, 'status': 'UNKNOWN' if diagnostics else 'SAFE',
                      'diagnostics': sorted(set(diagnostics)), 'target_supported': selected,
                      'rank': 0 if selected else 1, 'locally_exact_regions_included': True})
    if target_producer is not None and sum(s['target_supported'] for s in sites) != 1:
        raise ValueError('target producer statement binding is stale or ambiguous')
    return {'source_sha256': source_hash, 'function': function, 'sites': sorted(sites, key=lambda s: (s['rank'], s['start_byte'])),
            'closed_macro_context': closed_macro_context, 'reviewed_type_aliases': aliases,
            'reviewed_constants': constants, 'target_producer': target_producer,
            'authority_advanced': False}


def shared_initializer_producer(source, function, constraints):
    evidence = constraints.get('shared_initializer_analysis', {})
    if evidence.get('source_sha256') != _sha(source) or evidence.get('function') != function:
        raise ValueError('shared initializer analysis binding is stale')
    fresh = analyze_shared_initializers(source, function, **{k: evidence.get(k) for k in
        ('closed_macro_context', 'reviewed_type_aliases', 'reviewed_constants', 'target_producer')})
    target = fresh['target_producer']
    if target is None:
        return []
    site = next(s for s in fresh['sites'] if s['target_supported'])
    if site['status'] != 'SAFE':
        raise ValueError('; '.join(site['diagnostics']))
    producer = target['producer']
    if site['producer'] == producer:
        return []
    ordered = [v for v in site['lvalues'] if v != producer]+[producer]
    return [{k: site[k] for k in ('start_byte', 'end_byte', 'sha256')} |
            {'replacement': (' = '.join(ordered)+' = 0;').encode('ascii')}]


def sequence_scalar_argument(source, function, constraints):
    """Explicit fold or snapshot_existing using an existing reviewed scalar.

    Fold embeds an adjacent assignment in its direct-call argument. The reverse
    snapshot_existing mode splits a pure operand into assignment then call,
    preserving multiline indentation and the source's local newline spelling.

    Review flags cover typedef/volatile/alias knowledge outside the island; this
    syntactic check is deliberately not advertised as whole-program proof.
    """
    start, _, raw, nodes = _island(source, function)
    direction = constraints.get('direction')
    if direction == 'snapshot_existing':
        if any(constraints.get(k) is not True for k in ('incoming_value_dead_reviewed', 'later_use_safe_reviewed')):
            raise ValueError('reviewed incoming-dead/later-use-safe scalar required')
        local = constraints.get('local')
        if not isinstance(local, str) or not re.fullmatch(r'[A-Za-z_]\w*', local):
            raise ValueError('existing local identifier required')
        spans = constraints.get('statements', [])
        if len(spans) != 1:
            raise ValueError('one bound call statement required')
        span = spans[0]
        matches = [n for n in nodes if n.type == 'expression_statement'
                   and start+n.start_byte == span.get('start_byte') and start+n.end_byte == span.get('end_byte')]
        if len(matches) != 1:
            raise ValueError('call span mismatch')
        statement = matches[0]
        text = raw[statement.start_byte:statement.end_byte]
        original = span.get('original')
        original = original.encode() if isinstance(original, str) else original
        if text != original or _sha(text) != span.get('sha256'):
            raise ValueError('call text/hash mismatch')
        call = statement.named_children[0]
        if call.type != 'call_expression':
            raise ValueError('direct call required')
        args = call.child_by_field_name('arguments').named_children
        index = constraints.get('argument_index')
        if type(index) is not int or not 0 <= index < len(args):
            raise ValueError('bound argument index required')
        operand = args[index]
        if operand.type == 'unary_expression':
            operator = operand.child_by_field_name('operator')
            if raw[operator.start_byte:operator.end_byte] not in (b'+', b'-'):
                raise ValueError('only unary sign supported')
            operand = operand.child_by_field_name('argument')
        if operand.type != 'identifier':
            raise ValueError('plain scalar operand required')
        rhs = raw[operand.start_byte:operand.end_byte]
        first = local.encode()+b' = '+rhs+b';'
        second = (raw[statement.start_byte:operand.start_byte]+local.encode()
                  +raw[operand.end_byte:statement.end_byte])
        lo, hi = start+statement.start_byte, start+statement.end_byte
        line_start = source.rfind(b'\n', 0, lo)+1
        indentation = source[line_start:lo]
        separator = b' '
        if not indentation.strip():
            following_lf = source.find(b'\n', hi)
            previous_lf = source.rfind(b'\n', 0, lo)
            newline_at = following_lf if following_lf >= 0 else previous_lf
            newline = b'\r\n' if newline_at > 0 and source[newline_at-1:newline_at] == b'\r' else b'\n'
            separator = newline+indentation
        replacement = first+separator+second
        staged = source[:lo]+replacement+source[hi:]
        reviewed = dict(constraints, direction='fold', statements=[
            {'original': value, 'start_byte': pos, 'end_byte': pos+len(value), 'sha256': _sha(value)}
            for value, pos in ((first, lo), (second, lo+len(first)+len(separator)))])
        # Reuse the same declaration, alias, shadowing, and independent-argument checks.
        sequence_scalar_argument(staged, function, reviewed)
        return [{'start_byte': lo, 'end_byte': hi, 'sha256': _sha(text), 'replacement': replacement}]
    if direction != 'fold':
        raise ValueError('unknown scalar sequencing direction')
    if any(constraints.get(k) is not True for k in
           ('compatible_value_conversions', 'nonvolatile_scalar_reads_reviewed', 'local_no_alias_reviewed')):
        raise ValueError('reviewed scalar conversions, reads and local alias ownership required')
    local = constraints.get('local')
    if not isinstance(local, str) or not re.fullmatch(r'[A-Za-z_]\w*', local):
        raise ValueError('plain existing local required')
    textof = lambda n: raw[n.start_byte:n.end_byte]
    selected = []
    spans = constraints.get('statements', [])
    if len(spans) != 2:
        raise ValueError('two bound statements required')
    for span in spans:
        text = span.get('original')
        text = text.encode() if isinstance(text, str) else text
        matches = [n for n in nodes if n.type == 'expression_statement'
                   and start+n.start_byte == span.get('start_byte') and start+n.end_byte == span.get('end_byte')]
        if len(matches) != 1 or textof(matches[0]) != text or _sha(text) != span.get('sha256'):
            raise ValueError('statement span/text/hash mismatch')
        selected.append(matches[0])
    first, second = selected
    if first.parent != second.parent or first.parent.type != 'compound_statement':
        raise ValueError('same immediate block required')
    siblings = [n for n in first.parent.named_children if n.type != 'comment']
    if siblings.index(first)+1 != siblings.index(second):
        raise ValueError('adjacent ordered statements required')
    a, call = first.named_children[0], second.named_children[0]
    if a.type != 'assignment_expression' or call.type != 'call_expression':
        raise ValueError('assignment then direct call required')
    left, right = a.child_by_field_name('left'), a.child_by_field_name('right')
    if (textof(a.child_by_field_name('operator')) != b'=' or left.type != 'identifier'
            or textof(left) != local.encode() or right.type != 'identifier' or textof(right) == local.encode()
            or call.child_by_field_name('function').type != 'identifier'):
        raise ValueError('plain scalar identifier assignment/direct callee required')
    declarations = [n for n in nodes if n.type in {'declaration', 'parameter_declaration'}
                    and any(c.type == 'identifier' and textof(c) == local.encode() for c in _nodes(n))]
    if len(declarations) != 1:
        raise ValueError('ambiguous or shadowed local')
    decl = declarations[0]
    types = {'int', 'short', 'char', 'long', 'unsigned int', 'signed int', 'unsigned short', 'signed short'}
    types.update(constraints.get('reviewed_scalar_types', []))
    if (decl.type != 'declaration' or decl.end_byte > first.start_byte
            or decl.parent.type != 'compound_statement'
            or not (decl.parent.start_byte <= first.start_byte < decl.parent.end_byte)
            or not any(re.fullmatch(re.escape(t.encode())+rb'\s+'+local.encode()+rb'\s*;', textof(decl)) for t in types)):
        raise ValueError('unique prior nonvolatile scalar local declaration required')
    if re.search(rb'&\s*'+local.encode()+rb'\b', raw):
        raise ValueError('address escape unsupported')
    args = call.child_by_field_name('arguments').named_children
    read_names = {textof(right)} | {textof(n) for arg in args for n in _nodes(arg) if n.type == 'identifier'}
    for declaration in nodes:
        if declaration.type not in {'declaration', 'parameter_declaration'}:
            continue
        names = {textof(n) for n in _nodes(declaration) if n.type == 'identifier'}
        if names & read_names and (b'volatile' in textof(declaration).split()
                or any(n.type in {'pointer_declarator', 'array_declarator'} for n in _nodes(declaration))):
            raise ValueError('known volatile/pointer/array scalar input unsupported')
    uses = [n for n in _nodes(call) if n.type == 'identifier' and textof(n) == local.encode()]
    if len(uses) != 1:
        raise ValueError('exactly one argument use required')
    use = uses[0]
    owner_args = [arg for arg in args if arg.start_byte <= use.start_byte < arg.end_byte]
    if len(owner_args) != 1:
        raise ValueError('local must occur in argument')
    arg = owner_args[0]
    if arg != use and not (arg.type == 'unary_expression' and arg.child_by_field_name('argument') == use
                          and textof(arg.child_by_field_name('operator')) in (b'+', b'-')):
        raise ValueError('only plain or unary sign argument supported')
    for other in args:
        if other == arg:
            continue
        if other.type not in {'identifier', 'number_literal'} or textof(other) in (local.encode(), textof(right)):
            raise ValueError('other arguments must be independent side-effect-free scalars')
    replacement = (raw[second.start_byte:use.start_byte]+b'('+textof(a)+b')'
                   +raw[use.end_byte:second.end_byte]+raw[first.end_byte:second.start_byte])
    return [{'start_byte': start+first.start_byte, 'end_byte': start+second.end_byte,
             'sha256': _sha(raw[first.start_byte:second.end_byte]), 'replacement': replacement}]


def indexed_base_snapshot(source, function, constraints):
    start, _, raw, nodes = _island(source, function)
    typ, name, array = [constraints.get(k) for k in ('element_type', 'snapshot_name', 'array_name')]
    if any(not isinstance(v, str) or not re.fullmatch(r'[A-Za-z_]\w*', v) for v in (typ, name, array)):
        raise ValueError('plain reviewed type/array/new identifier required')
    if typ not in constraints.get('reviewed_element_types', []) or constraints.get('array_declaration_reviewed') is not True:
        raise ValueError('reviewed canonical array declaration/type required')
    evidence = constraints.get('array_declaration_evidence', {})
    text = evidence.get('text')
    text = text.encode() if isinstance(text, str) else text
    if not isinstance(text, bytes) or _sha(text) != evidence.get('sha256') or not re.fullmatch(
            rb'\s*extern\s+'+typ.encode()+rb'\s+'+array.encode()+rb'\s*\[\s*[A-Za-z_0-9]+\s*\]\s*;\s*', text):
        raise ValueError('canonical nonvolatile array evidence text/hash mismatch')
    # Full-source lexical collision check also protects visible globals/macros.
    if re.search(r'\b'+re.escape(name)+r'\b', snapshot.mask_c(source.decode('latin-1'))):
        raise ValueError('snapshot name already exists')
    site = constraints.get('declaration', {})
    matches = [n for n in nodes if n.type == 'declaration' and start+n.start_byte == site.get('start_byte') and start+n.end_byte == site.get('end_byte')]
    if len(matches) != 1 or _sha(raw[matches[0].start_byte:matches[0].end_byte]) != site.get('sha256'):
        raise ValueError('declaration span/hash mismatch')
    decl = matches[0]
    if decl.parent.type != 'compound_statement':
        raise ValueError('ordinary block declaration required')
    init = [n for n in decl.named_children if n.type == 'init_declarator']
    if len(init) != 1 or len([n for n in decl.named_children if n.type in {'identifier', 'init_declarator', 'pointer_declarator', 'array_declarator'}]) != 1:
        raise ValueError('single initialized owner required')
    value = init[0].child_by_field_name('value')
    if value is None or value.type != 'field_expression':
        raise ValueError('array-index member initializer required')
    subscript = value.child_by_field_name('argument')
    if subscript.type != 'subscript_expression':
        raise ValueError('direct array subscript required')
    base, index = subscript.child_by_field_name('argument'), subscript.child_by_field_name('index')
    if base.type != 'identifier' or raw[base.start_byte:base.end_byte] != array.encode() or index.type != 'identifier' or raw[subscript.end_byte:value.child_by_field_name('field').start_byte].strip() != b'.':
        raise ValueError('plain array/index identifiers and member required')
    if b'volatile' in raw[decl.start_byte:decl.end_byte].split():
        raise ValueError('volatile owner unsupported')
    index_text = raw[index.start_byte:index.end_byte]
    if any(n.type in {'declaration', 'parameter_declaration'} and b'volatile' in raw[n.start_byte:n.end_byte].split()
           and any(c.type == 'identifier' and raw[c.start_byte:c.end_byte] == index_text for c in _nodes(n)) for n in nodes):
        raise ValueError('volatile index unsupported')
    position = start+decl.start_byte
    return [{'start_byte': position, 'end_byte': position, 'sha256': _sha(b''),
             'replacement': typ.encode()+b' *'+name.encode()+b' = '+array.encode()+b';\n'},
            {'start_byte': start+base.start_byte, 'end_byte': start+base.end_byte,
             'sha256': _sha(array.encode()), 'replacement': name.encode()}]


def sequence_result_consumer(source, function, constraints):
    """One reviewed adjacent assignment pair; exact original text supports rebinding."""
    start, _, raw, nodes = _island(source, function)
    if constraints.get('compatible_value_conversions') is not True or constraints.get('field_stability_reviewed') is not True:
        raise ValueError('reviewed conversion compatibility and field stability required')
    local = constraints.get('local')
    if not isinstance(local, str) or not re.fullmatch(r'[A-Za-z_]\w*', local):
        raise ValueError('named existing local required')
    spans = constraints.get('statements', [])
    if len(spans) != 2:
        raise ValueError('two exact statement descriptors required')
    selected = []
    for span in spans:
        text = span.get('original')
        text = text.encode('utf-8') if isinstance(text, str) else text
        if not isinstance(text, bytes) or _sha(text) != span.get('sha256'):
            raise ValueError('original statement bytes/hash required')
        matches = [n for n in nodes if n.type == 'expression_statement' and raw[n.start_byte:n.end_byte] == text]
        if len(matches) != 1:
            raise ValueError('original statement missing or ambiguous')
        node = matches[0]
        if not constraints.get('rebind_exact_statements', False) and (start+node.start_byte != span.get('start_byte') or start+node.end_byte != span.get('end_byte')):
            raise ValueError('stale statement span')
        selected.append(node)
    first, second = selected
    if first.parent != second.parent or first.parent.type != 'compound_statement':
        raise ValueError('assignments must share an immediate block')
    siblings = [n for n in first.parent.named_children if n.type != 'comment']
    if siblings.index(first)+1 != siblings.index(second):
        raise ValueError('assignments must be adjacent and ordered')
    expressions = [[n for n in s.named_children if n.type != 'comment'] for s in selected]
    if any(len(es) != 1 or es[0].type != 'assignment_expression' for es in expressions):
        raise ValueError('plain assignments required')
    a, b = [es[0] for es in expressions]
    al, ar = a.child_by_field_name('left'), a.child_by_field_name('right')
    bl, br = b.child_by_field_name('left'), b.child_by_field_name('right')
    textof = lambda n: raw[n.start_byte:n.end_byte]
    if any(textof(e.child_by_field_name('operator')) != b'=' for e in (a,b)) or al.type != 'identifier' or textof(al) != local.encode() or br.type != 'identifier' or textof(br) != local.encode() or ar.type != 'call_expression':
        raise ValueError('expected local=call then field=local')
    if ar.child_by_field_name('function').type != 'identifier':
        raise ValueError('direct call required')
    def stable(n):
        if n.type == 'identifier':
            return textof(n) != local.encode()
        if n.type == 'field_expression':
            return stable(n.child_by_field_name('argument'))
        if n.type == 'subscript_expression':
            index = n.child_by_field_name('index')
            return index.type == 'number_literal' and bool(re.fullmatch(rb'[0-9]+', textof(index))) and stable(n.child_by_field_name('argument'))
        return False
    if bl.type not in {'field_expression','subscript_expression'} or not stable(bl):
        raise ValueError('stable member/literal-index lvalue required')
    # Exact typed declaration must exist; references remain at original scope.
    declarations = [n for n in nodes if n.type == 'declaration' and any(c.type == 'identifier' and textof(c) == local.encode() for c in _nodes(n))]
    if len(declarations) != 1 or any(word in textof(declarations[0]).split() for word in (b'volatile', b'const', b'static', b'extern')):
        raise ValueError('unique ordinary local declaration required')
    owner = declarations[0]
    if owner.end_byte > first.start_byte or not (owner.parent.start_byte <= first.start_byte < owner.parent.end_byte):
        raise ValueError('local declaration does not enclose the original consumer')
    middle = raw[first.end_byte:second.start_byte]
    replacement = textof(bl)+b' = ('+textof(a)+b', '+textof(br)+b');'+middle
    return [{'start_byte': start+first.start_byte, 'end_byte': start+second.end_byte,
             'sha256': _sha(raw[first.start_byte:second.end_byte]), 'replacement': replacement}]
LIMIT = 1024 * 1024


def _sha(raw):
    return hashlib.sha256(raw).hexdigest()


def _nodes(node):
    pending = [node]
    count = 0
    while pending:
        current = pending.pop()
        count += 1
        if count > 100000:
            raise ValueError("function AST exceeds bound")
        yield current
        pending.extend(reversed(current.children))


def _island(source, function):
    if not isinstance(source, bytes) or len(source) > LIMIT:
        raise ValueError("bounded source bytes required")
    start, end, _ = _function_span(source, function)
    raw = source[start:end]
    parser = tree_sitter.Parser(tree_sitter.Language(tree_sitter_c.language()))
    tree = parser.parse(raw)
    nodes = list(_nodes(tree.root_node))
    if tree.root_node.has_error or any(n.is_missing or n.type.startswith("preproc_") for n in nodes):
        raise ValueError("invalid or preprocessor-dependent function island")
    if any(n.type == "identifier" and raw[n.start_byte:n.end_byte] in
           {b"__LINE__", b"__FILE__", b"__COUNTER__"} for n in nodes):
        raise ValueError("location-sensitive macro in function")
    return start, end, raw, nodes


def fingerprint_function(source_bytes, function, *, closed_macro_context=False, preprocessed_token_binding=None):
    """Lexical-token identity, NOT semantic equivalence or compilation authority."""
    start, end, raw, nodes = _island(source_bytes, function)
    tokens = [(n.type, raw[n.start_byte:n.end_byte].hex()) for n in nodes
              if not n.children and n.type != "comment"]
    parser_identity = {"tree_sitter": importlib.metadata.version("tree-sitter"),
                       "tree_sitter_c": importlib.metadata.version("tree-sitter-c")}
    lexical = _sha(json.dumps(tokens, separators=(",", ":")).encode())
    line_tokens = [(n.type, raw[n.start_byte:n.end_byte].hex(),
                    source_bytes.count(b"\n", 0, start + n.start_byte) + 1)
                   for n in nodes if not n.children and n.type != "comment"]
    outside = _sha(source_bytes[:start] + b"\x00FUNCTION_ISLAND\x00" + source_bytes[end:])
    masked = snapshot.mask_c(source_bytes.decode("latin-1"))
    sensitive = bool(re.search(r"\b(?:__LINE__|__FILE__|__COUNTER__)\b", masked))
    if sensitive:
        raise ValueError("location-sensitive macro anywhere in translation unit")
    if preprocessed_token_binding is not None and not (
            isinstance(preprocessed_token_binding, str) and re.fullmatch(r"[0-9a-f]{64}", preprocessed_token_binding)):
        raise ValueError("preprocessed token binding must be SHA-256")
    safe = closed_macro_context is True or preprocessed_token_binding is not None
    reuse = {"tokens_and_lines": line_tokens, "outside_sha256": outside,
             "parser": parser_identity, "preprocessed_token_binding": preprocessed_token_binding,
             "closed_macro_context": closed_macro_context is True}
    return {"function": function, "token_sha256": lexical, "lexical_sha256": lexical,
            "precompile_reuse_key": _sha(json.dumps(reuse, sort_keys=True).encode()),
            "precompile_reuse_safe": safe, "outside_island_sha256": outside,
            "token_line_sha256": _sha(json.dumps(line_tokens).encode()),
            "source_sha256": _sha(source_bytes), "function_sha256": _sha(raw),
            "span": {"start_byte": start, "end_byte": end},
            "parser": parser_identity,
            "token_count": len(tokens), "compile_context_bound": False}


def known_shape_duplicate(candidate, known, *, compile_context_sha256, known_compile_context_sha256):
    """Only bound exact-byte identity or macro-proven token+line identity suppresses work."""
    if not re.fullmatch(r"[0-9a-f]{64}", compile_context_sha256 or ""):
        raise ValueError("compile context SHA-256 required")
    if compile_context_sha256 != known_compile_context_sha256:
        return False
    if candidate["source_sha256"] == known["source_sha256"]:
        return True
    return (candidate.get("precompile_reuse_safe") is True and known.get("precompile_reuse_safe") is True
            and candidate.get("precompile_reuse_key") == known.get("precompile_reuse_key"))


def _integral(node, raw, calls, identifiers):
    if node.type == "parenthesized_expression":
        return _integral(node.named_children[0], raw, calls, identifiers)
    if node.type == "number_literal":
        return bool(re.fullmatch(rb"(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*", raw[node.start_byte:node.end_byte]))
    if node.type == "identifier":
        return raw[node.start_byte:node.end_byte].decode() in identifiers
    if node.type == "call_expression":
        callee = node.child_by_field_name("function")
        return callee.type == "identifier" and raw[callee.start_byte:callee.end_byte].decode() in calls
    if node.type == "unary_expression":
        return raw[node.start_byte:node.named_children[0].start_byte].strip() == b"!" and _integral(node.named_children[0], raw, calls, identifiers)
    if node.type == "binary_expression":
        left, right = node.child_by_field_name("left"), node.child_by_field_name("right")
        op = raw[node.child_by_field_name("operator").start_byte:node.child_by_field_name("operator").end_byte]
        return op in {b"==", b"!=", b"<", b">", b"<=", b">=", b"&&", b"||"} and all(
            _integral(n, raw, calls, identifiers) for n in (left, right))
    return False


def _guards(source, function, constraints):
    start, _, raw, nodes = _island(source, function)
    sites = constraints.get("guard_sites", [])
    if not isinstance(sites, list) or not 1 <= len(sites) <= 8:
        raise ValueError("1..8 exact reviewed guard spans required")
    calls = set(constraints.get("integral_calls", []))
    identifiers = set(constraints.get("integral_identifiers", []))
    edits = []
    for site in sites:
        matches = [n for n in nodes if n.type == "if_statement" and
                   start + n.start_byte == site["start_byte"] and start + n.end_byte == site["end_byte"]]
        if len(matches) != 1:
            raise ValueError("guard span is stale or not an if")
        node = matches[0]
        if _sha(source[site["start_byte"]:site["end_byte"]]) != site["sha256"]:
            raise ValueError("guard span hash mismatch")
        parent = node.parent
        if parent.type != "compound_statement" or parent.parent.type not in {"for_statement", "while_statement", "do_statement"}:
            raise ValueError("guard must be a direct loop-body statement")
        if any(n.type in {"goto_statement", "labeled_statement", "continue_statement", "case_statement"}
               for n in _nodes(parent.parent)):
            raise ValueError("edited loop has labels or ambiguous control transfers")
        if node.child_by_field_name("alternative"):
            raise ValueError("if/else is not an eligibility guard")
        # Skipping the loop tail would change semantics: require final statement.
        if parent.named_children[-1] != node:
            raise ValueError("guard would skip following loop-body statements")
        cond = node.child_by_field_name("condition").named_children[0]
        if cond.type != "binary_expression" or raw[cond.child_by_field_name("operator").start_byte:cond.child_by_field_name("operator").end_byte] != b"&&":
            raise ValueError("top-level short-circuit conjunction required")
        left, right = cond.child_by_field_name("left"), cond.child_by_field_name("right")
        if not all(_integral(n, raw, calls, identifiers) for n in (left, right)):
            raise ValueError("predicate integral contracts not reviewed")
        body = node.child_by_field_name("consequence")
        if body.type != "compound_statement" or any(n.type in {"break_statement", "continue_statement", "return_statement"} for n in _nodes(body)):
            raise ValueError("ambiguous branch-body control transfer")
        ltext, rtext = raw[left.start_byte:left.end_byte], raw[right.start_byte:right.end_byte]
        reject = ltext[1:].strip() if ltext.startswith(b"!") and left.type == "unary_expression" else b"!(" + ltext + b")"
        replacement = b"if (" + reject + b") { continue; }\nif (" + rtext + b") " + raw[body.start_byte:body.end_byte]
        edits.append({**site, "replacement": replacement})
    return edits


def enumerate_shapes(source_bytes, function, approved_families, source_constraints):
    """Emit at most one composed candidate per explicitly approved family.

    Constraints bind source_sha256. Snapshot sites use snapshot.repair's owner
    descriptors. Guard sites are exact byte spans+sha256; integral_calls and
    integral_identifiers are caller-reviewed type contracts, not inferred types.
    """
    before = fingerprint_function(source_bytes, function)
    if source_constraints.get("source_sha256") != before["source_sha256"]:
        raise ValueError("source constraint binding is stale")
    if not isinstance(approved_families, (list, tuple)) or any(f not in FAMILIES for f in approved_families):
        raise ValueError("unknown or unapproved family")
    results = []
    for family in dict.fromkeys(approved_families):
        if family == "snapshot_point_use":
            _, raw_edits = snapshot.repair(source_bytes.decode("latin-1"), function, source_constraints.get("snapshot_sites", []))
            edits = [{"start_byte": e["start"], "end_byte": e["end"],
                      "sha256": _sha(source_bytes[e["start"]:e["end"]]),
                      "replacement": e["replacement"].encode("latin-1")} for e in raw_edits]
        elif family == 'sequence_scalar_argument':
            edits = sequence_scalar_argument(source_bytes, function, source_constraints)
        elif family == 'sequence_result_consumer':
            edits = sequence_result_consumer(source_bytes, function, source_constraints)
        elif family == 'indexed_base_snapshot':
            edits = indexed_base_snapshot(source_bytes, function, source_constraints)
        elif family == 'shared_initializer_producer':
            edits = shared_initializer_producer(source_bytes, function, source_constraints)
            if not edits:
                continue
        else:
            edits = _guards(source_bytes, function, source_constraints)
        edits.sort(key=lambda e: (e["start_byte"], e["end_byte"]))
        if any(a["end_byte"] > b["start_byte"] for a, b in zip(edits, edits[1:])):
            raise ValueError("overlapping edits")
        candidate = source_bytes
        for e in reversed(edits):
            candidate = candidate[:e["start_byte"]] + e["replacement"] + candidate[e["end_byte"]:]
        after = fingerprint_function(candidate, function)
        public_edits = [{**e, "start": e["start_byte"], "end": e["end_byte"],
                         "original": source_bytes[e["start_byte"]:e["end_byte"]]} for e in edits]
        results.append({"family": family, "source": candidate, "candidate_source": candidate,
                        "source_sha256": before["source_sha256"], "candidate_sha256": _sha(candidate),
                        "constraint_id": source_constraints.get("id", family), "edits": public_edits,
                        "binding": before, "fingerprint": after,
                        "rationale": "reviewed existing-owner shape; evaluation order and source types retained",
                        "authority_advanced": False})
    return results


def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("function")
    args = parser.parse_args(argv)
    print(json.dumps(fingerprint_function(args.source.read_bytes(), args.function), sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
