"""Read-only first residual -> captured operand -> alias/source evidence slice.

No target virtual IDs, source transformation, or source-root-cause inference.
"""
import argparse
import json
from pathlib import Path
import re
import sys

if __package__ in (None, ''):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import recovery_frontier as frontier
from tools import recovery_expression_join as join
from tools import recovery_evaluate as evaluator
from tools import recovery_causal_groups as groups


def lifecycle_constraint(events, result):
    """Join observed identities, not numeric-ID proximity or source guesses."""
    operand = result.get('operand', {})
    original = operand.get('original_id')
    creation = result.get('creation', {})
    by_id = {e.get('event_id'): e for e in events if e.get('event_id')}
    for union in reversed(result.get('alias_unions', [])):
        origin = union['origin']
        token = origin.get('expression_token')
        births = [e for e in events if e.get('event_kind') == 'return_temp_allocation'
                  and token is not None and e.get('expression_token') == token
                  and e.get('allocated_vreg') == original]
        if len(births) != 1:
            continue
        birth = births[0]
        reset = by_id.get(birth.get('preceding_gpr_reset_event'))
        if reset is None:
            continue
        early = by_id.get(creation.get('event_id'))
        union_event = by_id.get(union.get('event_id'))
        identity = ('session_id', 'process_id', 'function')
        valid = (early is not None and union_event is not None
                 and reset.get('event_kind') == 'temporary_lane_reset'
                 and reset.get('temporary_class') == 4 and operand.get('bank') == 'GPR'
                 and all(e.get('status') == 'CAPTURED' for e in (birth, reset, early))
                 and all(all(e.get(k) == birth.get(k) for k in identity) for e in (reset, early, union_event))
                 and all(type(e.get('sequence')) is int for e in (early, reset, birth, union_event))
                 and early['sequence'] < reset['sequence'] < birth['sequence'] < union_event['sequence']
                 and reset.get('counter_after') == reset.get('saved_base') == original
                 and birth.get('counter_after') == original + 1)
        if not valid:
            raise ValueError('invalid reset/return allocation identity or chronology')
        before = reset.get('counter_before')
        if type(before) is not int:
            raise ValueError('missing observed pre-reset counter')
        return {'status': 'observed_cross_reset_identity_reuse', 'original_id': original,
                'reset_event': reset['event_id'], 'allocation_event': birth['event_id'],
                'union_event': union['event_id'], 'return_expression_token': token,
                'direct_callee_name': origin.get('direct_callee_name'),
                'counter_before': before, 'counter_after': reset['counter_after'],
                'required_identity_separation': 'initial operand and later return result must not share this alias identity',
                'conditional_counter_budget_alternative': {
                    'assumed_reset_threshold': 256, 'observed_in_this_event': False,
                    'minimum_pre_reset_reduction': max(0, before - 256),
                    'conditions': 'Only if the pinned reset predicate is counter > 256, and all other allocation/CFG decisions remain unchanged.',
                    'safe_edit': False, 'predicted_match': False},
                'source_constraint_proven': False}
    return {'status': 'unknown', 'next_missing_edge': 'same-session return allocation expression with actual preceding GPR reset'}


def slice_events(document, events, function):
    pair = evaluator._diagnostic_rows(document, function)
    if pair is None:
        raise ValueError('missing function')
    left, right = pair
    projection = frontier._register_permutation(left, right)
    if len(left) != len(right) or projection.get('nonregister_differences'):
        raise ValueError('structural or nonregister residual; unsupported slice')
    mismatches = evaluator._diagnostic_mismatches(left, right)
    result = {'schema': 'recovery_alias_cause/v1', 'function': function,
              'diagnostic_only': True, 'source_constraint': None, 'cause_proven': False}
    if not mismatches:
        return dict(result, status='exact', next_missing_edge=None)
    index = min(mismatches)
    a, b = left[index]['instruction'], right[index]['instruction']
    ta, tb = a['formatted'], b['formatted']
    regs_a, regs_b = frontier._register_tokens(ta), frontier._register_tokens(tb)
    if frontier._without_registers(ta) != frontier._without_registers(tb):
        raise ValueError('first residual is not register-only')
    positions = [n for n, (x, y) in enumerate(zip(regs_a, regs_b)) if x != y]
    result['first_divergence'] = {'row': index, 'target': ta, 'candidate': tb, 'target_address': a.get('address')}
    if not positions:
        return dict(result, status='unknown', next_missing_edge='nontext operand/relocation identity')
    # First release authenticates destination operand zero of simple PPC forms.
    # Other roles cannot be inferred from numeric register equality alone.
    opcode = tb.split()[0]
    if len(positions) != 1:
        return dict(result, status='unknown', next_missing_edge='multiple changed operands; only a single destination difference is supported')
    if positions[0] != 0 or opcode not in {'mulli', 'addi', 'add', 'subf', 'mr', 'fmuls', 'fadds', 'fmr'}:
        return dict(result, status='unknown', next_missing_edge='machine operand role to PCode ordinal decoder')
    wanted, actual = regs_a[0], regs_b[0]
    bank = 'GPR' if actual.startswith('r') else 'FPR' if actual.startswith('f') else None
    if bank is None or wanted[0] != actual[0]:
        raise ValueError('unsupported register bank')
    ordinal = sum(bool(r.get('instruction')) for r in right[:index])
    machines = [e for e in events if e.get('event_kind') == 'machine_emission' and e.get('instruction_index') == ordinal]
    if len(machines) != 1:
        return dict(result, status='unknown', next_missing_edge='unique captured candidate machine instruction')
    machine = machines[0]
    token = machine['pcode_token']
    operands = [e for e in events if e.get('event_kind') == 'pcode_capture' and e.get('pcode_token') == token
                and e.get('operand_ordinal') == 0 and e.get('operand_bank') == bank]
    identities = {(e.get('operand_index'), e.get('final_color')) for e in operands}
    if (not operands or len(identities) != 1 or any(e.get('confirmed') is not True or e.get('status') != 'CAPTURED'
            or e.get('operand_flags') != 2 for e in operands)
            or next(iter(identities))[1] != int(actual[1:])):
        return dict(result, status='unknown', next_missing_edge='confirmed destination PCode operand/color')
    rewrites = [e for e in events if e.get('event_kind') == 'pcode_alias_rewrite' and e.get('pcode_token') == token
                and e.get('operand_ordinal') == 0]
    if len(rewrites) != 1 or rewrites[0].get('new_index') != operands[0]['operand_index']:
        return dict(result, status='unknown', next_missing_edge='unique original operand rewrite chain')
    original = rewrites[0]['old_index']
    trace = join.trace_events(events, function, original)
    unions = [e for e in events if e.get('event_kind') == 'pcode_alias_union' and e.get('old_index') == original]
    if any(e.get('confirmed') is not True or e.get('status') != 'CAPTURED' for e in unions):
        raise ValueError('unconfirmed alias union')
    result.update(status='observed_alias_slice', operand={'bank': bank, 'ordinal': 0, 'target': wanted,
                  'candidate': actual, 'original_id': original, 'canonical_id': operands[0]['operand_index'],
                  'pcode_token': token, 'machine_index': ordinal},
                  creation=join.compact_origin(join.source_origin_join(events, machine['session_id'], function, token)),
                  alias_unions=[{'event_id': e.get('event_id'), 'old_index': original, 'new_index': e.get('new_index'),
                     'origin': join.compact_origin(join.source_origin_join(events, machine['session_id'], function, e.get('pcode_token')))}
                     for e in unions],
                  affected_rewrite_count=len(trace['rewrites']),
                  next_missing_edge='source expression lifetime/consumer constraint explaining alias eligibility; target source IDs unavailable')
    return result


def analyze(root, index, envelope, function):
    root = Path(root).resolve()
    base = frontier.load_json(Path(index).read_bytes())
    frontier.verify(root, base)
    inputs = base['inputs']
    source = frontier.local(root, Path(inputs['source']['path']))
    obj = frontier.local(root, Path(inputs['candidate_object']['path']))
    binding = join.bind(envelope, obj, source, function)
    if binding['comparison']['status'] != 'exact_words':
        raise ValueError('capture/object machine words not exact')
    capture = json.loads(Path(envelope).read_bytes())
    join.validate_session(capture, function)
    document = frontier.load_json(frontier.local(root, Path(inputs['strict_report']['path'])).read_bytes())
    summary = groups.summarize_groups(document, function)
    if not summary.get('size_exact') or summary.get('structural_hazard_count'):
        raise ValueError('requires exact size/CFG/opcodes/relocations and register-only residuals')
    result = slice_events(document, capture['events'], function)
    result['lifecycle'] = lifecycle_constraint(capture['events'], result)
    lines = source.read_text(encoding='utf-8').splitlines()
    origins = [result.get('creation', {})] + [u['origin'] for u in result.get('alias_unions', [])]
    for origin in origins:
        origin['source_boundary'] = join.source_boundary(source.read_bytes(), function, origin)
        line = origin.get('source_offset')
        if type(line) is int and 1 <= line <= len(lines):
            origin['enclosing_source_line'] = lines[line-1][:240]
            origin['line_scope'] = 'enclosing CodeGen source position, not exact child span'
    frontier.verify(root, base)
    result['binding'] = binding
    result['source_regions'] = join.actionable_source_regions(
        source.read_bytes(), function, origins, binding['source_sha256'],
        reuse_observed=result['lifecycle']['status'] == 'observed_cross_reset_identity_reuse')
    result['baseline_index_sha256'] = join.sha(Path(index).read_bytes())
    return result


def build_return_allocation_census(root, index, envelope, function):
    """Observe return births and unused results; never generate source requests."""
    from tools import recovery_source_shapes as shapes
    root = Path(root).resolve()
    index, envelope = Path(index).resolve(), Path(envelope).resolve()
    sliced = analyze(root, index, envelope, function)
    reset_id = sliced.get('lifecycle', {}).get('reset_event')
    if not reset_id:
        return {'status': 'not_applicable', 'reason': 'no validated cross-reset alias slice'}
    capture = json.loads(envelope.read_bytes())
    events = capture['events']
    reset = next(e for e in events if e.get('event_id') == reset_id)
    base = frontier.load_json(index.read_bytes())
    source = frontier.local(root, Path(base['inputs']['source']['path'])).read_bytes()
    start, _, raw, nodes = shapes._island(source, function)
    selected, skipped = {}, 0
    births = [e for e in events if e.get('event_kind') == 'return_temp_allocation']
    for birth in events:
        if birth.get('event_kind') != 'return_temp_allocation' or birth.get('sequence', reset['sequence']) >= reset['sequence']:
            continue
        if birth.get('status') != 'CAPTURED' or type(birth.get('allocated_vreg')) is not int:
            raise ValueError('invalid return allocation evidence')
        origins = [e for e in events if e.get('event_kind') == 'source_pcode_origin'
                   and e.get('expression_token') == birth.get('expression_token')
                   and e.get('session_id') == birth.get('session_id')
                   and e.get('direct_callee_name') and e.get('status') == 'CAPTURED']
        # Several emitted instructions can share one actual expression; require
        # one consistent enclosing/callee identity, not one arbitrary PCode.
        identities = {(e.get('direct_callee_name'), e.get('source_offset'), e.get('expression_kind')) for e in origins}
        if len(identities) != 1 or not birth.get('expression_token'):
            skipped += 1
            continue
        origin = origins[0]
        boundary = join.source_boundary(source, function, origin)
        container = boundary.get('container', {})
        if (boundary.get('candidate_count') != 1 or boundary.get('truncated')
                or container.get('type') != 'expression_statement'):
            skipped += 1
            continue
        matches = [n for n in nodes if n.type == 'expression_statement'
                   and start+n.start_byte == container['start_byte'] and start+n.end_byte == container['end_byte']]
        children = [n for n in matches[0].named_children if n.type != 'comment'] if len(matches) == 1 else []
        if len(children) != 1 or children[0].type != 'call_expression':
            skipped += 1
            continue
        callee = children[0].child_by_field_name('function')
        name = origin['direct_callee_name']
        if callee is None or callee.type != 'identifier' or raw[callee.start_byte:callee.end_byte].decode() != name:
            skipped += 1
            continue
        key = container['start_byte']
        if key in selected:
            selected[key]['allocation_events'].append(birth['event_id'])
        else:
            selected[key] = {'callee': name, 'site': {k: container[k] for k in ('start_byte', 'end_byte', 'sha256')},
                             'allocation_events': [birth['event_id']], 'source_boundary': boundary}
    cells = [selected[k] for k in sorted(selected)]
    if not cells:
        return {'status': 'not_applicable', 'reason': 'no unique plain discarded calls'}
    omitted_sites = max(0, len(cells)-32)
    cells = cells[:32]
    evidence = {'path': envelope.relative_to(root).as_posix(), 'sha256': join.sha(envelope.read_bytes())}
    frontier.verify(root, base)
    return {'schema': 'recovery_return_allocation_census/v1', 'status': 'observed',
            'source_patch': None, 'predicted_gain': False, 'canonical_contracts': 'unknown_review_required',
            'baseline_index_sha256': join.sha(index.read_bytes()), 'source_sha256': join.sha(source),
            'evidence': evidence, 'reset_event': reset_id,
            'total_return_allocations': len(births),
            'pre_reset_return_allocations': sum(e['sequence'] < reset['sequence'] for e in births),
            'unused_plain_call_allocation_count': sum(len(c['allocation_events']) for c in cells),
            'canonical_review_needed_callees': sorted({c['callee'] for c in cells}),
            'omitted_sites': omitted_sites, 'selection_order': 'source byte order, first 32',
            'unresolved_or_consumed_pre_reset_allocations': skipped, 'unused_plain_call_sites': cells,
            'limitations': 'Actual nonvoid allocation is not a canonical contract. Explicit void discard does not prevent native return allocation; no source transformation is proposed.'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'index', 'envelope'):
        p.add_argument('--'+name, type=Path, required=True)
    p.add_argument('--function', required=True)
    args = p.parse_args()
    print(json.dumps(analyze(**vars(args)), sort_keys=True))


if __name__ == '__main__':
    main()
