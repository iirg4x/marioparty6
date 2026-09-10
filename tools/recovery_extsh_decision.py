"""Private, pinned GC2.6 Donkey extsh selector trace; no source transformations.

Compose the existing private debugger lifecycle and compile mutex. Hardware
execution slots DR1/DR2 observe selector inputs, returned predicates and emission.
The source-field-to-backend-value edge is explicitly not inferred.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys
from typing import Any

DEBUG_FIELDS = ('Dr0', 'Dr1', 'Dr2', 'Dr3', 'Dr6', 'Dr7')
HOOKS = {
    0x44FEF0: '8b0c248b4102894424148b4424108b48023b4c24147d5a8a063c0475088b5e0e803b31744c3c1e720f3c28770b8b560e8b5a0e803b3174393c03770b8b6e0e8b550e803a31742a',
    0x44FF61: '394c24147c098b0424837802047d40',
    0x4E1DA5: '85db7408',
    0x4E1DB7: '5984c07454',
    0x4E1E1C: '83c41084c0740d',
    0x4E1E54: '6a65e875b4ffff',
    0x4E1E10: '6a006a006a6556e864000000',
    0x4E1E80: '53558b5c240c8b542410a188aa5e006683782800',
    0x4E1EA8: '807d24007513807d2504750d668b4d28668b43026639c17405',
    0x4DD2D0: '538d5c24',
    0x4E7FC0: '8b4c240480390375038b490e51e85e7ffdff59c3',
    0x4E1CBC: '8b4d028b45024883f8010f8794010000ff248590b15b00',
    0x528D32: '0fbf472e83e0087412668b472450558d470850e8468ffbff',
    0x5292B0: '0fbf462e83e008740f6a00578d460850e8cb89fbff',
    0x529166: '8b7424308b56048b7a06',
    0x4BFF30: '8b4c24040fbe014883f80a7729ff24854c185b000fb6410683f80c7713ff248518185b00b001c3a01db15e00c38d400030c0c3b001c330c0c3',
    0x5B1818: '54ff4b0057ff4b0060ff4b0054ff4b0054ff4b0060ff4b0054ff4b0060ff4b0054ff4b0060ff4b0054ff4b0060ff4b0054ff4b00',
    0x5B184C: '44ff4b0066ff4b0066ff4b0066ff4b0066ff4b0066ff4b0066ff4b0066ff4b0066ff4b0066ff4b0063ff4b00',
    0x44FA20: '5356575583ec488b5c245c8b6c24648b7c24688b730e8b4606894424108b4306890424',
    0x44FF70: '576a006a00560fb61eff149d484a5e0083c410803f00740f6a00ff74241457e87c2c090083c40c8b44246050ff74240457e8ea1c0900',
}


def debug_state(context: Any) -> tuple[int, ...]:
    return tuple(int(getattr(context, key)) for key in DEBUG_FIELDS)


def restore_debug_state(context: Any, saved: tuple[int, ...]) -> None:
    if len(saved) != len(DEBUG_FIELDS):
        raise ValueError('invalid saved debug-register state')
    for key, value in zip(DEBUG_FIELDS, saved):
        setattr(context, key, value)


def arm_execution(context: Any, entry: int) -> None:
    # Reject occupied DR1/DR2; leave DR0/DR3 controls intact.
    mask = 0x3C | (0xFF << 20)
    if int(context.Dr7) & mask:
        raise ValueError('DR1/DR2 already occupied')
    context.Dr1 = entry
    context.Dr2 = 0
    context.Dr6 &= ~0xF
    context.Dr7 |= 0x4


def selector_result(ebx: int, predicate: int | None, reuse: int | None) -> str:
    if ebx:
        return 'already_extended'
    if predicate is None:
        return 'awaiting_type_predicate'
    if predicate & 0xFF:
        return 'alternate_opcode_0x67'
    if reuse is None:
        return 'awaiting_extsh_reuse_predicate'
    return 'reused_extsh' if reuse & 0xFF else 'emit_extsh_0x65'


def load_capture(folder: Path) -> Any:
    sys.path.insert(0, str(folder))
    try:
        import koopa_coin_expression_counter as capture
    finally:
        sys.path.remove(str(folder))
    return capture


def inverse_selector_constraints(capture: dict, sequence: int, *, selector_capture: dict | None = None,
                                 selector_sequence: int | None = None) -> dict:
    """Bound a typed call to omission prerequisites; never infer missing predecessor state."""
    pinned = '316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8'
    if capture.get('compiler', {}).get('sha256') != pinned or capture.get('status') != 'CAPTURED':
        raise ValueError('successful pinned GC2.6 capture required')
    events = capture.get('events', [])
    if len(events) > 20000:
        raise ValueError('capture event bound exceeded')
    matches = [e for e in events if e.get('sequence') == sequence]
    if len(matches) != 1:
        raise ValueError('typed call sequence missing or ambiguous')
    event = matches[0]
    if event.get('event_kind') == 'extsh_selector_decision':
        reuse = event.get('reuse_inputs')
        if not isinstance(reuse, dict) or reuse.get('status') != 'CAPTURED':
            raise ValueError('sealed predecessor inputs unavailable')
        expected = selector_result(event['already_extended_ebx'], event.get('type_predicate_al'), event.get('reuse_predicate_al'))
        if expected != event.get('result') or reuse.get('source_vreg') != event.get('source_vreg'):
            raise ValueError('inconsistent selector outcome/source identity')
        return {'schema': 'extsh_inverse_constraints/v1', 'authority_advanced': False,
                'session_id': capture.get('session_id'), 'source': capture.get('source'),
                'compiler': capture.get('compiler'), 'event_sequence': sequence,
                'selector_result': expected, 'source_backend_kind': event.get('source_backend_kind'),
                'already_extended_ebx': event['already_extended_ebx'],
                'source_vreg': event['source_vreg'], 'reuse_inputs': reuse,
                'selector_type': event.get('selector_type'), 'argument_join': event.get('source_field_join'),
                'conversion_materialization': (conversion_materialization(event['source_field_join']['expression'])
                    if event.get('source_field_join', {}).get('status') == 'CONVERSION_EXPRESSION_JOINED' else None),
                'normalization_rule': '4E1C9F reads source backend kind; 4E1CA1 cmp AL,9; setae BL; and EBX,1',
                'backend_kind_domain': backend_kind_domain(),
                'required_changed_inputs': ['entry source backend kind >=9 for EBX fast return',
                    'or same-vreg predecessor opcode 0x65/0x64 instead of 0x67 for signed-short reuse'],
                'next_missing_edge': 'For44FFA6 conversion, inspect child dispatch result at44FF83; new traces record runtime kind4 dispatch target. The sealed narrow-bitfield path forces materialization before selector regardless of child deferred representation.',
                'formal_contract_status': 'not captured in this selector event; do not join other sessions',
                'source_candidate_supported': False}
    tree = event.get('normalized_expression_tree', {}).get('tree', {})
    contract = tree.get('parameter_contract', {})
    if tree.get('native_kind') != 54 or contract.get('compiler_sha256') != pinned:
        raise ValueError('direct normalized call and captured formal contract required')
    formals = contract.get('formals', [])
    arguments = tree.get('arguments', {}).get('items', [])
    if not formals or not arguments:
        raise ValueError('first formal/argument unavailable')
    formal = formals[0].get('type', {})
    expression = arguments[0].get('expression', {})
    lineage = []
    node = expression
    for _ in range(8):
        lineage.append({'kind': node.get('native_kind'), 'type': node.get('type')})
        if 'operand' not in node:
            break
        node = node['operand']
    else:
        raise ValueError('argument lineage exceeds bound')
    signed_short = {'native_kind': 1, 'native_basic_code': 5, 'byte_width': 2}
    agrees = all(formal.get(k) == v and expression.get('type', {}).get(k) == v for k, v in signed_short.items())
    selector = None
    if selector_capture is not None:
        for field in ('session_id', 'source', 'compiler', 'function_sha256'):
            if selector_capture.get(field) != capture.get(field):
                raise ValueError('selector and argument must share authenticated capture context')
        rows = [e for e in selector_capture.get('events', []) if e.get('sequence') == selector_sequence]
        if len(rows) != 1 or rows[0].get('event_kind') != 'extsh_selector_decision':
            raise ValueError('selector sequence unavailable')
        selector = rows[0]
        # CodeGen token alone is insufficient to identify an individual argument.
        if selector.get('argument_expression_token') != expression.get('token') or not expression.get('token'):
            raise ValueError('missing selector-to-argument expression identity')
    return {'schema': 'extsh_inverse_constraints/v1', 'authority_advanced': False,
            'source': capture.get('source'), 'compiler': capture.get('compiler'),
            'session_id': capture.get('session_id'), 'event_sequence': sequence,
            'call_expression_token': tree.get('token'), 'argument_expression_token': expression.get('token'),
            'formal_type': formal, 'argument_lineage': lineage,
            'formal_argument_status': 'signed_short_agrees' if agrees else 'not_established',
            'selector_facts': selector,
            'omission_branches': [
                {'branch': 'already_extended', 'site': '0x4E1DA5', 'requires': ['EBX != 0'],
                 'missing_evidence': 'producer of EBX normalization fact joined to this argument backend'},
                {'branch': 'reuse_extsh', 'site': '0x4E1EA8',
                 'requires': ['EBX == 0', 'type_predicate AL == 0', 'current block +0x28 > 0',
                              'last PCode +0x24 == 0', 'last PCode +0x25 == 4',
                              'last PCode destination +0x28 == source backend vreg +2',
                              'last PCode opcode +0x20 in {0x65,0x64}'],
                 'missing_evidence': 'same-session last-PCode descriptor and exact source-vreg identity'},
                {'branch': 'unsigned_alternative', 'site': '0x4E1DB7',
                 'requires': ['EBX == 0', 'type_predicate AL != 0'],
                 'omits_conversion': False,
                 'constraint': 'not a valid signed-short formal repair' if agrees else 'formal compatibility unproven'},
                {'branch': 'reuse_unsigned_alternative', 'site': '0x4E1DBC',
                 'requires': ['EBX == 0', 'type_predicate AL != 0', 'current block +0x28 > 0',
                              'last PCode +0x24 == 0', 'last PCode +0x25 == 4',
                              'last PCode destination +0x28 == source backend vreg +2',
                              'last PCode opcode +0x20 == 0x67',
                              'last PCode +0x3E == 0', 'last PCode +0x4A == 16', 'last PCode +0x56 == 31'],
                 'constraint': 'blocked by current signed-short formal' if agrees else 'formal compatibility unproven'}],
            'range_does_not_prove_selector_fact': True,
            'missing_frontend_edge': 'bitfield kind49 -> load kind4 -> conversion kind48: capture conversion construction and backend normalization flag provenance; numeric range alone does not establish EBX or predecessor',
            'source_candidate_supported': False}


def backend_kind_domain() -> dict:
    """Closed conclusions from GC2.6 4E2C10's switch at 5BB284; no guessed enum names."""
    return {
        '0': {'class': 'materialized register', 'evidence': '4E2C8C returns without materialization; selector consumes +2 vreg'},
        '4': {'class': 'integer constant payload', 'evidence': '4E2D11 reads +0C and emits immediate materialization',
              'ebx_fast_path': False},
        '9': {'class': 'deferred type-directed materialization, address-like payload',
              'evidence': '5BB284[9]=4E2DB3; type selects opcode; +2,+8,+10 feed4EA6A0; +14 feeds4DCF00; then kind=0',
              'semantic_enum_name': 'UNKNOWN', 'ebx_fast_path': True},
        '10': {'class': 'deferred type-directed materialization, two-register payload',
               'evidence': '5BB284[10]=4E2E8E; type selects opcode; +2/+6 feed4DD2D0; +14 feeds4DCF00; then kind=0',
               'semantic_enum_name': 'UNKNOWN', 'ebx_fast_path': True},
        '11..255': {'class': 'UNKNOWN entry domain',
                    'evidence': 'after helper4E3040, remaining kind>10 reaches4E3013 diagnostic; helper may transform entry descriptors, so no blanket invalid-entry claim'},
        'generator_constraint': 'No cast/local proposal may target the numeric tag. The guard recognizes entry representation before type-directed materialization, not source range or a constant. Current bitfield result is already kind0; kind9/10 require a genuine deferred producer with matching payload and semantics.'}


def type_lineage(reader: Any, address: int) -> dict[str, Any]:
    """Pinned native type tag/+2 byte width; kind3 unwrap is not guessed."""
    rows = []
    seen = set()
    while address and address not in seen and len(rows) < 4:
        seen.add(address)
        kind = reader.value(address, 1)
        row = {'native_kind': kind, 'byte_width': reader.value(address + 2, 4)}
        if kind in (1, 2):
            row['native_basic_type_code'] = reader.value(address + 6, 1)
        rows.append(row)
        if kind != 3:
            return {'status': 'CAPTURED_NATIVE_TYPE', 'chain': rows}
        address = reader.read_u32(address + 14)
    return {'status': 'MISSING_EDGE', 'chain': rows,
            'edge': 'type wrapper is cyclic, null or exceeds depth4'}


def expression_lineage(reader: Any, address: int, depth: int = 0,
                       seen: set[int] | None = None) -> dict[str, Any]:
    """Only native-validated child layouts; never traverse arbitrary pointers."""
    seen = set() if seen is None else seen
    if not address or address in seen or depth >= 8 or len(seen) >= 48:
        return {'status': 'MISSING_EDGE', 'edge': 'expression null/cycle/budget'}
    seen.add(address)
    kind = reader.value(address, 1)
    row = {'native_expression_kind': kind,
           'type': type_lineage(reader, reader.read_u32(address + 6))}
    if kind == 0x38:
        name = reader.read_object_name(reader.read_u32(address + 14))
        row['object_name'] = name if name and name.isidentifier() else None
    elif kind == 0x32:
        row['integer_constant'] = reader.value(address + 14, 8, True)
    elif kind in (0x04, 0x30):
        row['operand'] = expression_lineage(reader, reader.read_u32(address + 14), depth + 1, seen)
    elif kind in (0x09, 0x0F, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1E):
        row['left'] = expression_lineage(reader, reader.read_u32(address + 14), depth + 1, seen)
        row['right'] = expression_lineage(reader, reader.read_u32(address + 18), depth + 1, seen)
    else:
        row['missing_edge'] = 'native child layout not authenticated for this expression kind'
    return row


def argument_join(reader: Any, context: Any) -> dict[str, Any]:
    """Join by native argument-descriptor containment, not vreg coincidence."""
    caller = reader.value(int(context.Esp) + 0x18, 4) - reader.base + 0x400000
    if caller == 0x44FFA6:
        expression = reader.read_u32(int(context.Esp) + 0x84)
        child = reader.read_u32(int(context.Esp) + 0x10)
        backend = reader.read_u32(int(context.Esp) + 0x0C)
        if (backend != int(context.Esi)
                or reader.read_u32(expression + 14) != child
                or reader.read_u32(expression + 6) != int(context.Ebp)
                or reader.value(expression, 1) != 0x30):
            return {'status': 'MISSING_EDGE', 'edge': 'conversion caller stack/AST/backend identity validation failed',
                    'caller_rva': '0x0004FFA6'}
        lineage = expression_lineage(reader, expression)
        producer = conversion_materialization(lineage)
        if producer['direct_bitfield_load']:
            target = reader.read_u32(reader.runtime(0x5E4A48 + 4 * 4))
            producer['load_dispatch_target_rva'] = f'0x{target - reader.base:08X}'
        return {'status': 'CONVERSION_EXPRESSION_JOINED',
                'join_basis': 'pinned ETYPCON caller AST+0E equals saved source expression, AST+6 equals selector type, saved backend equals ESI',
                'caller_rva': '0x0004FFA6',
                'expression': lineage,
                'conversion_materialization': producer,
                'field_name_status': 'not inferred from layout offset or register number'}
    if caller not in (0x528D4A, 0x5292C5):
        return {'status': 'MISSING_EDGE', 'edge': 'selector caller has no authenticated argument-descriptor layout',
                'caller_rva': f'0x{caller - 0x400000:08X}'}
    # Both pinned callers pass descriptor+8 as backend value; +4 is the AST.
    expression = reader.read_u32(int(context.Esi) - 4)
    if reader.read_u32(expression + 6) != int(context.Ebp):
        return {'status': 'MISSING_EDGE', 'edge': 'argument AST type differs from selector type',
                'caller_rva': f'0x{caller - 0x400000:08X}'}
    return {'status': 'ARGUMENT_EXPRESSION_JOINED',
            'join_basis': 'pinned caller descriptor+8 backend, descriptor+4 AST, AST+6 equals selector type',
            'caller_rva': f'0x{caller - 0x400000:08X}',
            'expression': expression_lineage(reader, expression),
            'field_name_status': 'not inferred from layout offset or register number'}


def conversion_materialization(lineage: dict) -> dict:
    """Interpret only the sealed 44FFA6 caller path, not a range-based guess."""
    child = lineage.get('operand', {})
    bitfield = child.get('native_expression_kind') == 4 and child.get('operand', {}).get('native_expression_kind') == 49
    types = lineage.get('type', {}).get('chain', [])
    width = types[-1].get('byte_width') if types else None
    return {'direct_bitfield_load': bitfield, 'destination_width': width,
            'path': '44FF70 -> child dispatch -> 44FF83 -> optional4E2C10 -> 44FFA1',
            'backend_at_selector': 'kind0: already register or explicitly materialized before selector',
            'width_forces_materialization_path': bool(bitfield and width is not None and width < 4),
            'omission_constraint': 'Deferred kind9/10 cannot survive this path. Width>=4 is necessary but not sufficient for44FFB0; changing the s16 formal is not an admissible repair.',
            'source_candidate_supported': False}


def reuse_inputs(reader: Any, source_backend: int) -> dict:
    """Exact before-call inputs consumed by sealed 4E1E80/4E1EA8; no AST union."""
    source = reader.value(source_backend + 2, 2, True)
    block = reader.read_u32(reader.runtime(0x5EAA88))
    if not block:
        raise RuntimeError('reuse current block pointer is null')
    count = reader.value(block + 0x28, 2, True)
    result = {'status': 'CAPTURED', 'site_rva': '0x000E1E10',
              'source_vreg': source, 'block_instruction_count': count,
              'requested_opcode': 0x65, 'last_pcode': None}
    if count <= 0:
        result['blockers'] = ['block_has_no_predecessor']
        return result
    last = reader.read_u32(block + 0x18)
    if not last:
        raise RuntimeError('nonempty reuse block has null last PCode')
    fields = {'opcode_20': reader.value(last + 0x20, 2),
              'operand_kind_24': reader.value(last + 0x24, 1),
              'operand_class_25': reader.value(last + 0x25, 1),
              'destination_vreg_28': reader.value(last + 0x28, 2, True)}
    result['last_pcode'] = fields
    result['blockers'] = [name for name, passed in (
        ('operand_kind_not_zero', fields['operand_kind_24'] == 0),
        ('operand_class_not_four', fields['operand_class_25'] == 4),
        ('different_destination_vreg', fields['destination_vreg_28'] == source),
        ('predecessor_not_extsh_or_extsb', fields['opcode_20'] in (0x65, 0x64))) if not passed]
    return result


def debugger_type(capture: Any) -> type:
    base = capture.base

    class DecisionDebugger(base.CounterDebugger):
        def __init__(self, *args: Any, **kwargs: Any) -> None:
            super().__init__(*args, **kwargs)
            self.pending_decisions: dict[int, dict[str, Any]] = {}

        def validate_hooks(self) -> None:
            super().validate_hooks()
            for va, expected in HOOKS.items():
                expected_bytes = bytes.fromhex(expected)
                if self.read(self.runtime(va), len(expected_bytes)) != expected_bytes:
                    raise ValueError(f'extsh hook bytes differ at RVA {va - 0x400000:x}')

        def _arm_thread(self, tid: int) -> None:
            if tid in self._armed_threads:
                return
            context = self.get_context(self.threads[tid])
            saved = debug_state(context)
            arm_execution(context, self.runtime(0x4E1DA5))
            self.set_context(self.threads[tid], context)
            self._saved_debug_state[tid] = saved
            self._armed_threads.add(tid)

        def _disarm_thread(self, tid: int, strict: bool = True) -> None:
            if tid not in self._armed_threads:
                return
            try:
                context = self.get_context(self.threads[tid])
                restore_debug_state(context, self._saved_debug_state[tid])
                self.set_context(self.threads[tid], context)
            except Exception:
                if strict:
                    raise
            finally:
                self._saved_debug_state.pop(tid, None)
                self._armed_threads.discard(tid)

        def value(self, address: int, size: int, signed: bool = False) -> int:
            data = self.read(address, size)
            if len(data) != size:
                raise RuntimeError('selector input is unreadable')
            return int.from_bytes(data, 'little', signed=signed)

        def advance(self, context: Any, va: int | None) -> None:
            context.Dr2 = self.runtime(va) if va else 0
            context.Dr7 = (int(context.Dr7) & ~0x30) | (0x10 if va else 0)

        def finish(self, tid: int, context: Any) -> None:
            row = self.pending_decisions.pop(tid)
            self._append_event(row)
            self.advance(context, None)

        def observe(self, tid: int, context: Any) -> None:
            va = int(context.Eip) - self.base + 0x400000
            if va == 0x4E1DA5:
                if tid in self.pending_decisions:
                    raise RuntimeError('nested selector exceeds bounded trace model')
                joined = argument_join(self, context)
                operand = int(context.Ebp)
                kind = self.value(operand, 1)
                unwrapped = self.read_u32(operand + 14) if kind == 3 else operand
                row = {
                    'event_kind': 'extsh_selector_decision', 'function': base.TARGET_FUNCTION,
                    'sequence': len(self.result['events']),
                    'codegen_token': self._read_codegen_token(),
                    'expression_kind': joined.get('expression', {}).get('native_expression_kind'),
                    'observed_expression_names': [],
                    'selector_type_byte_width': self.value(operand + 2, 4),
                    'selector_type': type_lineage(self, operand),
                    'source_backend_kind': self.value(int(context.Esi), 1),
                    'source_vreg': self.value(int(context.Esi) + 2, 2, True),
                    'already_extended_ebx': int(context.Ebx),
                    'predicate_input': {'operand_kind': kind,
                        'unwrapped_kind': self.value(unwrapped, 1),
                        'unwrapped_byte_6': self.value(unwrapped + 6, 1)},
                    'requested_destination': self.value(int(context.Esp) + 0x24, 2, True),
                    'result': selector_result(int(context.Ebx), None, None),
                    'source_field_join': joined,
                }
                self.pending_decisions[tid] = row
                if context.Ebx:
                    self.finish(tid, context)
                else:
                    self.advance(context, 0x4E1DB7)
                return
            row = self.pending_decisions.get(tid)
            if row is None:
                raise RuntimeError('selector continuation without entry')
            if va == 0x4E1DB7:
                row['type_predicate_al'] = int(context.Eax) & 255
                row['result'] = selector_result(0, row['type_predicate_al'], None)
                if row['type_predicate_al']:
                    self.finish(tid, context)
                else:
                    self.advance(context, 0x4E1E10)
            elif va == 0x4E1E10:
                row['reuse_inputs'] = reuse_inputs(self, int(context.Esi))
                if row['reuse_inputs']['source_vreg'] != row['source_vreg']:
                    raise RuntimeError('selector backend vreg drift before reuse')
                self.advance(context, 0x4E1E1C)
            elif va == 0x4E1E1C:
                row['reuse_predicate_al'] = int(context.Eax) & 255
                if bool(row['reuse_predicate_al']) != (not row['reuse_inputs']['blockers']):
                    raise RuntimeError('sealed reuse inputs disagree with native predicate')
                row['result'] = selector_result(0, 0, row['reuse_predicate_al'])
                if row['reuse_predicate_al']:
                    self.finish(tid, context)
                else:
                    self.advance(context, 0x4E1E54)
            elif va == 0x4E1E54:
                row['emission_site_rva'] = '0x000E1E54'
                self.advance(context, 0x4DD2D0)
            elif va == 0x4DD2D0:
                row['emission'] = {
                    'opcode': self.value(int(context.Esp) + 4, 4),
                    'destination_vreg': self.value(int(context.Esp) + 8, 4, True),
                    'source_vreg': self.value(int(context.Esp) + 12, 4, True),
                }
                if row['emission']['opcode'] != 0x65:
                    raise RuntimeError('unexpected emission opcode')
                self.finish(tid, context)
            else:
                raise RuntimeError('unexpected selector execution hook')

        def _handle_single_step(self, event: Any, exception: Any) -> None:
            tid = int(event.dwThreadId)
            context = self.get_context(self.threads[tid])
            if self.capture_active and tid in self._armed_threads and int(context.Dr6) & 6:
                self.observe(tid, context)
                context.Dr6 &= ~6
                context.EFlags |= 0x10000  # RF: execute the stopped instruction once.
                self.set_context(self.threads[tid], context)
            super()._handle_single_step(event, exception)

    return DecisionDebugger


def bind_current(root: Path, index_path: Path, expected_object: str) -> tuple[dict, dict]:
    try:
        from tools import recovery_frontier as frontier
    except ModuleNotFoundError:
        import recovery_frontier as frontier
    raw, binding = frontier.read_bound(root, index_path, frontier.INDEX_LIMIT)
    index = frontier.load_json(raw)
    frontier.verify(root, index)
    if index['inputs']['candidate_object']['sha256'] != expected_object:
        raise ValueError('current candidate object differs from --expected-object-sha256')
    return index, binding


def main(argv: list[str] | None = None) -> int:
    incoming = list(sys.argv[1:] if argv is None else argv)
    if '--constraints-capture' in incoming:
        report = argparse.ArgumentParser(description='Read-only sealed selector constraints')
        report.add_argument('--constraints-capture', type=Path, required=True)
        report.add_argument('--capture-sha256', required=True)
        report.add_argument('--sequence', type=int, required=True)
        args = report.parse_args(incoming)
        raw = args.constraints_capture.read_bytes()
        if len(raw) > 4 * 1024 * 1024 or hashlib.sha256(raw).hexdigest() != args.capture_sha256:
            raise ValueError('constraints capture size/hash mismatch')
        result = inverse_selector_constraints(json.loads(raw), args.sequence)
        result['capture'] = {'path': str(args.constraints_capture), 'sha256': args.capture_sha256}
        print(json.dumps(result, sort_keys=True))
        return 0
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan', action='store_true')
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--current-index', type=Path, required=True)
    parser.add_argument('--request-template', type=Path, required=True)
    parser.add_argument('--capture-tooling', type=Path, required=True)
    parser.add_argument('--function', required=True)
    parser.add_argument('--definition-prefix', required=True)
    parser.add_argument('--expected-object-sha256', required=True)
    parser.add_argument('--output-directory', type=Path, required=True)
    args = parser.parse_args(argv)
    root = args.root.resolve()
    index, index_binding = bind_current(root, args.current_index, args.expected_object_sha256)
    if not args.function.isidentifier() or args.function not in args.definition_prefix:
        parser.error('function must be an identifier contained in definition-prefix')
    capture = load_capture((root / args.capture_tooling).resolve())
    wrapper, base = capture.wrapper, capture.base
    source = root / index['inputs']['source']['path']
    source_sha = index['inputs']['source']['sha256']
    data = source.read_bytes()
    if hashlib.sha256(data).hexdigest() != source_sha:
        raise ValueError(f'current source hash differs: {source}')
    prefix = (args.definition_prefix + '\n{').encode('ascii')
    if data.count(prefix) != 1:
        raise ValueError('definition prefix must select exactly one current definition')
    start = data.index(prefix)
    # Definition terminator is column-zero, unlike nested blocks in this pin.
    end = data.index(b'\n}', start) + 2
    function_sha = hashlib.sha256(data[start:end]).hexdigest()
    destination = (root / args.output_directory).resolve()
    destination.relative_to(root / 'build')
    request = json.loads((root / args.request_template).read_text('utf-8'))
    request.update(function=args.function, function_sha256=function_sha,
                   function_source_span={'start_byte': start, 'end_byte': end})
    old_source = request['source']['path']
    request['source'].update(path=str(source), sha256=source_sha, size=len(data))
    request['argv'] = [str(source) if value == old_source else value for value in request['argv']]
    request['cwd'] = str(root)
    request_path = destination / 'request.json'
    wrapper.ROOT = base.ROOT = root
    wrapper.COMPILE_LOCK_TOOL = Path(__file__).with_name('compile_recovery_candidate.py')
    wrapper.FUNCTION, wrapper.FUNCTION_PREFIX = args.function, prefix
    wrapper.FUNCTION_SHA256 = function_sha
    wrapper.SOURCE_SHA256, wrapper.SOURCE_SIZE = source_sha, len(data)
    wrapper.OBJECT_SHA256 = args.expected_object_sha256
    wrapper.BASELINE_PATH = root / index['inputs']['candidate_object']['path']
    wrapper.OUTPUT_PATH = destination / 'trace.json'
    wrapper.OBJECT_PATH = destination / 'capspecial.o'
    if not args.plan:
        destination.mkdir(parents=True, exist_ok=True)
        if request_path.exists():
            raise FileExistsError(f'refusing to overwrite {request_path}')
        base.central.atomic_write_json(request_path, request)
    else:
        print(json.dumps({'source_sha256': source_sha, 'function_sha256': function_sha,
                          'current_index': index_binding,
                          'destination': str(destination), 'hooks': HOOKS}, indent=2))
        return 0
    original_validate = wrapper.validate_request
    def validate_request() -> dict:
        current, current_binding = bind_current(root, args.current_index, args.expected_object_sha256)
        if current_binding != index_binding:
            raise ValueError('current index changed since capture planning')
        return original_validate(request_path)
    wrapper.validate_request = validate_request
    original_destinations = wrapper.validate_capture_destinations
    wrapper.validate_capture_destinations = lambda: original_destinations(
        wrapper.OUTPUT_PATH, wrapper.OBJECT_PATH, wrapper.BASELINE_PATH)
    base.CounterDebugger = debugger_type(capture)
    def annotate(debugger: Any) -> None:
        validate_request()
        debugger.result.update(schema='mwcc_extsh_decision/v2', authority_advanced=False,
            current_index=index_binding,
            watchpoint={'registers': ['DR1', 'DR2'], 'access': 'execute'},
            limitations=['Argument expression joins use native descriptor containment; unsupported field layouts remain missing edges.',
                         'Native type code/width is not automatically an authenticated C spelling.',
                         'Object equality is current baseline equality, not retail exactness.'])
        debugger.result['producer']['decision_capture'] = base.file_descriptor(Path(__file__))
        debugger.result['hook_bytes'] = HOOKS
    wrapper._annotate_result = annotate
    return wrapper.main([])


if __name__ == '__main__':
    raise SystemExit(main())
