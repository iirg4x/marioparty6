"""Join captured expression/PCode identities to exact object words. Read-only.

Centralized from private capture_word_binding/prealias_trace; no private imports.
Source text identity is checked, not historical source authenticity or retail identity.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

if __package__ in (None, ''):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def function_bytes(data, name):
    if len(data) < 52 or data[:6] != b'\x7fELF\x01\x02':
        raise ValueError('requires big-endian ELF32')
    if struct.unpack_from('>HH', data, 16) != (1, 20):
        raise ValueError('requires relocatable PowerPC ELF')
    table = struct.unpack_from('>I', data, 32)[0]
    stride, count = struct.unpack_from('>HH', data, 46)
    if stride != 40 or not count or table + stride * count > len(data):
        raise ValueError('invalid section table')
    sections = [struct.unpack_from('>10I', data, table + i * stride) for i in range(count)]

    def payload(section):
        start, size = section[4:6]
        if start + size > len(data):
            raise ValueError('section outside object')
        return data[start:start + size]

    found = []
    for section in sections:
        if section[1] != 2:
            continue
        if section[9] != 16 or section[5] % 16 or section[6] >= count:
            raise ValueError('invalid symbol table')
        strings = payload(sections[section[6]])
        for off in range(0, section[5], 16):
            no, value, size, info, other, index = struct.unpack_from('>IIIBBH', payload(section), off)
            end = strings.find(b'\0', no)
            if end < no:
                raise ValueError('invalid symbol name')
            if strings[no:end].decode('utf-8', errors='strict') != name or info & 15 != 2:
                continue
            if not 0 < index < count or not size or size % 4:
                raise ValueError('invalid function definition')
            owner = sections[index]
            start = value - owner[3]
            if not owner[2] & 4 or start < 0 or start + size > owner[5]:
                raise ValueError('function outside executable section')
            found.append(payload(owner)[start:start + size])
    if len(found) != 1:
        raise ValueError('function must have one unambiguous ELF definition')
    return found[0]


def compare_words(events, code, function):
    if not isinstance(events, list) or any(not isinstance(x, dict) for x in events):
        raise ValueError('capture events must be a list of objects')
    if not code or len(code) % 4:
        raise ValueError('function code must contain complete PPC words')
    rows = [x for x in events if x.get('event_kind') == 'machine_emission' and x.get('function') == function]
    if len(rows) != len(code) // 4:
        return {'status': 'mismatch', 'reason': 'instruction_count', 'captured': len(rows), 'object': len(code) // 4}
    missing, mismatches = [], []
    for i, row in enumerate(rows):
        if (type(row.get('instruction_index')) is not int or type(row.get('emitted_offset')) is not int
                or row['instruction_index'] != i or row['emitted_offset'] != i * 4):
            raise ValueError('capture instruction ordering/offset is not canonical')
        word = row.get('ppc_word')
        if word is None:
            missing.append(i)
            continue
        if type(word) is not int or not 0 <= word <= 0xffffffff:
            raise ValueError('invalid capture machine word')
        if row.get('ppc_bytes') is not None and (not isinstance(row['ppc_bytes'], str)
                or row['ppc_bytes'].lower() != f'{word:08x}'):
            raise ValueError('contradictory capture bytes/word')
        if word != struct.unpack_from('>I', code, i * 4)[0]:
            mismatches.append(i)
    return {'status': 'mismatch' if mismatches else 'unknown' if missing else 'exact_words',
            'word_count': len(rows), 'missing_count': len(missing), 'mismatch_count': len(mismatches),
            'missing_examples': missing[:8], 'mismatch_examples': mismatches[:8]}


def bind(envelope_path, object_path, source_path, function):
    raw = Path(envelope_path).read_bytes()
    envelope = json.loads(raw)
    if not isinstance(envelope, dict):
        raise ValueError('capture envelope must be an object')
    context = envelope.get('context', {})
    if not isinstance(context, dict) or not isinstance(context.get('source'), dict):
        raise ValueError('capture source context must be an object')
    source = Path(source_path).read_bytes()
    if context.get('function') != function or context.get('source', {}).get('sha256') != sha(source):
        raise ValueError('capture function/source binding mismatch or absent')
    obj = Path(object_path).read_bytes()
    code = function_bytes(obj, function)
    result = compare_words(envelope['events'], code, function)
    return {'schema': 'capspecial_capture_word_binding/v1', 'function': function,
            'envelope_sha256': sha(raw), 'object_sha256': sha(obj), 'source_sha256': sha(source),
            'function_code_sha256': sha(code), 'comparison': result,
            'diagnostic_only': True, 'linked_exact': False, 'source_authenticity_proven': False,
            'scope': 'one captured function machine-word stream, not whole-object compiler provenance'}



def source_origin_join(events, session_id, function, pcode_token):
    """Exact identity join; absence/duplicate births never become a guess."""
    selected = [e for e in events if e.get("session_id") == session_id
                and e.get("function") == function and e.get("pcode_token") == pcode_token]
    origins = [e for e in selected if e.get("event_kind") == "source_pcode_origin"]
    if len(origins) != 1:
        return {"status": "UNKNOWN", "reason": "missing_or_ambiguous_creation"}
    origin = origins[0]
    if (origin.get("owner_role") != "enclosing_codegen"
            or origin.get("child_edge") not in {"MISSING_RECURSIVE_CHILD", "CAPTURED_ACTIVE_HANDLER"}
            or origin.get("status") != "CAPTURED"):
        return {"status": "UNKNOWN", "reason": "unsupported_source_authority"}
    return {"status": "ENCLOSING_CODEGEN_JOINED", "origin": origin,
            "child_edge": origin["child_edge"],
            "operands": [e for e in selected if e.get("event_kind") == "pcode_capture"],
            "rewrites": [e for e in selected if e.get("event_kind") == "pcode_alias_rewrite"],
            "machine": [e for e in selected if e.get("event_kind") == "machine_emission"]}


def trace_events(events, function, original_id):
    if type(original_id) is not int or original_id < 0:
        raise ValueError("original_id must be a non-negative integer")
    if not isinstance(events, list) or any(not isinstance(e, dict) for e in events):
        raise ValueError("events must be a list of event objects")
    selected = [e for e in events if e.get("function") == function]
    rewrites = [e for e in selected
                if e.get("event_kind") == "pcode_alias_rewrite"
                and e.get("old_index") == original_id]
    unions = [e for e in selected
              if e.get("event_kind") == "pcode_alias_union"
              and e.get("old_index") == original_id]
    rows = []
    for rewrite in rewrites:
        if rewrite.get("confirmed") is not True or rewrite.get("status") != "CAPTURED":
            raise ValueError("unconfirmed alias rewrite")
        token = rewrite.get("pcode_token")
        ordinal = rewrite.get("operand_ordinal")
        new_id = rewrite.get("new_index")
        if (not isinstance(token, str) or not token or type(ordinal) is not int
                or ordinal < 0 or type(new_id) is not int or new_id < 0):
            raise ValueError("malformed alias rewrite identity")
        operands = [e for e in selected if e.get("event_kind") == "pcode_capture"
                    and e.get("pcode_token") == token
                    and e.get("operand_ordinal") == ordinal]
        # Alias events lack a bank. Never infer a bank from numeric IDs alone.
        banks = {e.get("operand_bank") for e in operands}
        confirmed = bool(operands) and all(
            e.get("confirmed") is True and e.get("status") == "CAPTURED"
            and e.get("operand_index") == new_id for e in operands)
        bank = next(iter(banks)) if len(banks) == 1 else None
        resolved = confirmed and bank in {"GPR", "FPR"}
        machines = [e for e in selected if e.get("event_kind") == "machine_emission"
                    and e.get("pcode_token") == token]
        rows.append({
            "rewrite_event": rewrite.get("event_id"), "pcode_token": token,
            "operand_ordinal": ordinal, "original_id": original_id,
            "canonical_id": new_id, "bank": bank if resolved else None,
            "bank_join": "confirmed" if resolved else "unknown",
            "operand_flags": rewrite.get("operand_flags"),
            "source_origin": source_origin_join(events, rewrite.get("session_id"), function, token),
            "machine_anchors": [{"index": m.get("instruction_index"),
                                 "word": m.get("ppc_word"),
                                 "word_hex": f'{m["ppc_word"]:08x}' if type(m.get("ppc_word")) is int else None,
                                 "recorded_decoder_status": m.get("status")} for m in machines],
        })
    return {
        "original_id": original_id,
        "status": "observed_rewrites" if rows else "no_observed_rewrites",
        "unions": [{"event_id": e.get("event_id"), "new_id": e.get("new_index"),
                    "confirmed": e.get("confirmed") is True,
                    "status": e.get("status"),
                    "pcode_token": e.get("pcode_token")} for e in unions],
        "rewrites": rows,
        "limitations": ["Only recorded old-ID rewrites are enumerated; missing events are not reconstructed.",
                        "Unions alone have no bank identity.",
                        "Operand flags are reported at rewrite time, not asserted unchanged across later compiler stages.",
                        "Target IDs and source statement allocation chronology remain unknown."],
    }


def validate_session(envelope, function):
    context = envelope['context']
    session = context.get('session_id')
    process = context.get('process_id')
    if not isinstance(session, str) or not session.startswith('session-') or type(process) is not int:
        raise ValueError('missing session identity')
    for event in envelope['events']:
        if (event.get('session_id') != session or event.get('process_id') != process
                or event.get('function') != function):
            raise ValueError('disconnected session/function/process event')
        token = event.get('pcode_token')
        if token is not None and not token.startswith('pcode-' + session + '-'):
            raise ValueError('PCode token crosses session')
    return session


def compact_origin(join):
    if join['status'] == 'UNKNOWN':
        return join
    origin = join['origin']
    return {key: origin.get(key) for key in
            ('event_id', 'child_edge', 'expression_token', 'expression_kind', 'source_offset',
             'direct_callee_name', 'temporary_counter', 'reset_inhibition',
             'preceding_gpr_reset_event') if key in origin}


def source_boundary(source, function, origin):
    """Resolve an enclosing CodeGen coordinate, never invent a child position.

    CodeGen stores a line coordinate. A loop condition may carry its closing
    brace's line, so treating this field as the call's exact line is unsound.
    AST matches are candidates; even a unique name match is not a column-level
    native AST/source identity proof.
    """
    from tools import recovery_source_shapes as shapes
    line = origin.get('source_offset')
    if type(line) is not int or line < 1:
        return {'status': 'UNKNOWN', 'reason': 'missing_enclosing_line_coordinate'}
    try:
        start, end, raw, nodes = shapes._island(source, function)
    except ValueError as error:
        return {'status': 'UNKNOWN', 'reason': str(error)}
    before = source.count(b'\n', 0, start)
    relative_line = line - before - 1
    lines = raw.splitlines(keepends=True)
    if not 0 <= relative_line < len(lines):
        return {'status': 'UNKNOWN', 'reason': 'coordinate_outside_function'}
    lo = sum(map(len, lines[:relative_line]))
    hi = lo + len(lines[relative_line].rstrip(b'\r\n'))
    text = raw[lo:hi]
    # A closing brace belongs to the controlled compound statement. Select its
    # immediate control construct, not every call appearing earlier in the TU.
    if text.strip() == b'}':
        containers = [n for n in nodes if n.type == 'compound_statement'
                      and lo <= n.end_byte - 1 < hi]
        if not containers:
            return {'status': 'UNKNOWN', 'reason': 'closing_coordinate_not_bound_to_compound'}
        container = min(containers, key=lambda n: n.end_byte - n.start_byte)
        if container.parent is not None and container.parent.type in {
                'for_statement', 'while_statement', 'do_statement', 'if_statement', 'switch_statement'}:
            container = container.parent
    else:
        containers = [n for n in nodes if n.type in {
            'declaration', 'expression_statement', 'return_statement', 'for_statement',
            'while_statement', 'do_statement', 'if_statement', 'switch_statement'}
            and n.start_byte < hi and n.end_byte > lo]
        if not containers:
            return {'status': 'UNKNOWN', 'reason': 'coordinate_has_no_source_statement'}
        container = min(containers, key=lambda n: n.end_byte - n.start_byte)
        if any(n.end_byte <= container.start_byte or n.start_byte >= container.end_byte
               for n in containers):
            return {'status': 'UNKNOWN', 'reason': 'multiple_source_statements_at_coordinate'}

    def descriptor(node):
        fragment = raw[node.start_byte:node.end_byte]
        return {'type': node.type, 'start_byte': start + node.start_byte,
                'end_byte': start + node.end_byte, 'sha256': sha(fragment),
                'start_line': before + raw.count(b'\n', 0, node.start_byte) + 1,
                'end_line': before + raw.count(b'\n', 0, max(node.start_byte, node.end_byte - 1)) + 1}

    within = [n for n in nodes if container.start_byte <= n.start_byte
              and n.end_byte <= container.end_byte]
    kind, callee = origin.get('expression_kind'), origin.get('direct_callee_name')
    if kind in {54, 55}:
        candidates = [n for n in within if n.type == 'call_expression']
        if isinstance(callee, str) and callee:
            candidates = [n for n in candidates if (f := n.child_by_field_name('function')) is not None
                          and f.type == 'identifier' and raw[f.start_byte:f.end_byte].decode('ascii') == callee]
        basis = 'captured_callee_name_in_enclosing_statement' if callee else 'call_class_in_enclosing_statement'
    elif kind in {9, 15}:
        # Native multiplication/addition may be implicit array scale/address
        # arithmetic. Do not turn it into a requested literal * or + C edit.
        candidates = [n for n in within if n.type == 'subscript_expression' or (
            n.type == 'binary_expression' and (op := n.child_by_field_name('operator')) is not None
            and raw[op.start_byte:op.end_byte] == (b'*' if kind == 9 else b'+'))]
        basis = 'possible_normalized_arithmetic_or_array_address'
    else:
        candidates, basis = [], 'native_kind_has_no_reviewed_source_class'
    return {'status': 'SOURCE_CONTEXT', 'coordinate_scope': 'enclosing_codegen_line',
            'coordinate_line': line, 'line_text': text.decode('latin-1').strip()[:240],
            'container': descriptor(container), 'candidate_basis': basis,
            'candidate_count': len(candidates), 'candidates': [descriptor(n) for n in candidates[:8]],
            'truncated': len(candidates) > 8, 'exact_child_source_span': False,
            'source_patch_ranked': False}


def actionable_source_regions(source, function, origins, expected_source_sha256, reuse_observed=False):
    """Review regions from already session-validated origins, not source causality.

    The caller owns trace/session validation. AST containment is explicitly an
    inference even when the captured line and callee are unambiguous.
    """
    if not expected_source_sha256 or sha(source) != expected_source_sha256:
        raise ValueError('source region binding mismatch')
    observed, inferred, unknown = [], [], []
    seen = set()
    for item in origins:
        origin = item.get('origin', item)
        if origin.get('status') == 'UNKNOWN' or not origin.get('event_id'):
            unknown.append('missing_or_ambiguous_trace_origin')
            continue
        key = origin['event_id']
        if key in seen:
            continue
        seen.add(key)
        if len(observed) >= 32:
            unknown.append('origin_region_limit_32')
            break
        boundary = source_boundary(source, function, origin)
        observed.append({'status': 'observed', 'event_id': key,
                         'source_line': origin.get('source_offset'),
                         'callee': origin.get('direct_callee_name'),
                         'coordinate_scope': 'enclosing_codegen_line_not_child_span'})
        if boundary.get('status') != 'SOURCE_CONTEXT':
            unknown.append(boundary.get('reason', 'source_boundary_unknown'))
            continue
        inferred.append({'status': 'inferred', 'region': boundary['container'],
                         'event_id': key, 'why_implicated': 'AST enclosure of observed CodeGen coordinate',
                         'candidate_count': boundary['candidate_count'],
                         'next_source_question': 'Which real value producer or consumer in this enclosure owns the captured expression?'})
        if boundary['candidate_count'] != 1 or boundary['truncated']:
            unknown.append('missing_or_ambiguous_child_source_identity:' + str(key))
    census = None
    if not observed:
        unknown.append('no_observed_trace_origin')
    if reuse_observed and observed:
        from tools import recovery_source_shapes as shapes
        try:
            census = shapes.analyze_shared_initializers(source, function)
            for site in census.get('sites', []):
                lo, hi = site['start_byte'], site['end_byte']
                inferred.append({'status': 'inferred',
                                 'region': {'start_byte': lo, 'end_byte': hi,
                                            'sha256': sha(source[lo:hi]),
                                            'start_line': source.count(b'\n', 0, lo) + 1,
                                            'end_line': source.count(b'\n', 0, max(lo, hi - 1)) + 1},
                                 'why_implicated': 'Real shared producer in function with observed alias/reuse evidence; local machine mismatch is not required',
                                 'trace_to_initializer_edge': 'unknown',
                                 'next_source_question': 'Which actual lvalue produces the shared value, and do its later consumers constrain lifetime ownership?'})
        except ValueError as error:
            unknown.append('shared_initializer_scope_unknown:' + str(error))
    unknown.extend(['target_source_and_virtual_identities_unavailable',
                    'source_region_to_alias_causality_unproved',
                    'post_change_counter_sequence_not_observed'])
    return {'schema': 'recovery_actionable_source_regions/v1',
            'source_sha256': expected_source_sha256, 'function': function,
            'diagnostic_only': True, 'authority_advanced': False, 'source_patch': None,
            'observed': observed, 'inferred': inferred,
            'shared_initializers': census,
            'shared_initializer_scope': 'inferred function-wide producer review, including locally exact machine regions' if census is not None else None,
            'next_source_question': 'Does a real shared initializer producer change lifetime ownership while preserving the required stores and reloads?' if census is not None else 'Resolve the missing trace/source edge before attributing a source cause.',
            'unknown_edges': sorted(set(unknown))}


def analyze(envelope, object_path, source, function, original_id, details=False):
    data = json.loads(Path(envelope).read_bytes())
    session = validate_session(data, function)
    binding = bind(envelope, object_path, source, function)
    if binding['comparison']['status'] != 'exact_words':
        raise ValueError('capture is not exactly word-bound to source/object')
    result = trace_events(data['events'], function, original_id)
    for union in result['unions']:
        if union['confirmed'] is not True or union.get('status') != 'CAPTURED':
            raise ValueError('unconfirmed union')
        join = source_origin_join(data['events'], session, function, union['pcode_token'])
        union['source_origin'] = join if details else compact_origin(join)
    for row in result['rewrites']:
        join = row['source_origin']
        operands = join.get('operands', [])
        colors = sorted({e['final_color'] for e in operands
                         if e.get('operand_ordinal') == row['operand_ordinal']
                         and e.get('confirmed') is True and e.get('status') == 'CAPTURED'
                         and e.get('operand_bank') == row['bank']
                         and e.get('operand_index') == row['canonical_id']})
        row['final_color'] = colors[0] if len(colors) == 1 else None
        if not details:
            row['source_origin'] = compact_origin(join)
    result.update(schema='recovery_expression_join/v1', function=function,
                  session_id=session, binding=binding, diagnostic_only=True,
                  source_authenticity_proven=False, linked_exact=False)
    origins = [r['source_origin'] for r in result['rewrites']] + [u['source_origin'] for u in result['unions']]
    result['source_regions'] = actionable_source_regions(
        Path(source).read_bytes(), function, origins, binding['source_sha256'],
        reuse_observed=bool(result['rewrites'] and result['unions']))
    if any(type((o.get('origin', o)).get('source_offset')) is int for o in origins):
        source_bytes = Path(source).read_bytes()
        if sha(source_bytes) != binding['source_sha256']:
            raise ValueError('source drift while resolving source boundaries')
        for item in origins:
            origin = item.get('origin', item)
            if type(origin.get('source_offset')) is int:
                item['source_boundary'] = source_boundary(source_bytes, function, origin)
    if not details:
        result['total_rewrites'] = len(result['rewrites'])
        result['total_unions'] = len(result['unions'])
        result['truncated'] = any(len(result[key]) > 32 for key in ('rewrites', 'unions'))
        for key in ('rewrites', 'unions'):
            result[key] = result[key][:32]
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for field in ('envelope', 'object', 'source', 'function'):
        parser.add_argument('--' + field, required=True)
    parser.add_argument('--original-id', type=int, required=True)
    parser.add_argument('--details', action='store_true')
    args = parser.parse_args(argv)
    try:
        result = analyze(args.envelope, args.object, args.source, args.function,
                         args.original_id, args.details)
    except (ValueError, TypeError, KeyError, OSError, struct.error) as error:
        print(json.dumps({'status': 'UNKNOWN', 'reason': str(error), 'diagnostic_only': True}))
        return 2
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
