import types
import unittest
from unittest import mock
from pathlib import Path
import sys

from tools import recovery_extsh_decision as trace


class DecisionTests(unittest.TestCase):
    def test_inverse_accepts_same_event_selector_predecessor(self):
        event = {'event_kind': 'extsh_selector_decision', 'sequence': 7,
                 'already_extended_ebx': 0, 'type_predicate_al': 0, 'reuse_predicate_al': 0,
                 'result': 'emit_extsh_0x65', 'source_vreg': 54, 'source_backend_kind': 0,
                 'reuse_inputs': {'status': 'CAPTURED', 'source_vreg': 54,
                                  'blockers': ['predecessor_not_extsh_or_extsb']}}
        capture = {'status': 'CAPTURED', 'compiler': {'sha256': '316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8'},
                   'events': [event]}
        result = trace.inverse_selector_constraints(capture, 7)
        self.assertEqual(result['selector_result'], 'emit_extsh_0x65')
        self.assertFalse(result['source_candidate_supported'])
        event['source_vreg'] = 53
        with self.assertRaises(ValueError):
            trace.inverse_selector_constraints(capture, 7)

    def test_load_capture_returns_module_and_restores_path(self):
        module = types.ModuleType('koopa_coin_expression_counter')
        module.wrapper = object()
        before = list(sys.path)
        with mock.patch.dict(sys.modules, {'koopa_coin_expression_counter': module}):
            self.assertIs(trace.load_capture(Path('test-tooling')), module)
        self.assertEqual(sys.path, before)

    def test_reuse_inputs_actual_predecessor_blockers(self):
        values = {0x5EAA88: 1000, 1000+0x28: 4, 1000+0x18: 2000,
                  3002: 52, 2000+0x20: 0x65, 2000+0x24: 0,
                  2000+0x25: 4, 2000+0x28: 52}
        reader = types.SimpleNamespace(runtime=lambda x: x, read_u32=lambda x: values[x],
                                       value=lambda x, size, signed=False: values[x])
        self.assertEqual(trace.reuse_inputs(reader, 3000)['blockers'], [])
        values[2000+0x28] = 51
        self.assertEqual(trace.reuse_inputs(reader, 3000)['blockers'], ['different_destination_vreg'])
        values[2000+0x20] = 7
        self.assertIn('predecessor_not_extsh_or_extsb', trace.reuse_inputs(reader, 3000)['blockers'])
        values[1000+0x28] = 0
        self.assertIsNone(trace.reuse_inputs(reader, 3000)['last_pcode'])

    def test_inverse_omission_constraints_preserve_missing_edges(self):
        typ = {'native_kind': 1, 'native_basic_code': 5, 'byte_width': 2}
        tree = {'native_kind': 54, 'token': 'call',
                'parameter_contract': {'compiler_sha256': '316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8',
                                       'formals': [{'type': typ}]},
                'arguments': {'items': [{'expression': {'native_kind': 48, 'token': 'arg', 'type': typ,
                    'operand': {'native_kind': 4, 'type': {'byte_width': 1, 'native_basic_code': 3},
                                'operand': {'native_kind': 49}}}}]}}
        capture = {'status': 'CAPTURED', 'compiler': {'sha256': tree['parameter_contract']['compiler_sha256']},
                   'session_id': 'one', 'source': {'sha256': 'a'*64}, 'function_sha256': 'b'*64,
                   'events': [{'sequence': 18, 'normalized_expression_tree': {'tree': tree}}]}
        result = trace.inverse_selector_constraints(capture, 18)
        self.assertEqual(result['formal_argument_status'], 'signed_short_agrees')
        self.assertIsNone(result['selector_facts'])
        self.assertFalse(result['source_candidate_supported'])
        self.assertIn('last PCode opcode +0x20 in {0x65,0x64}', result['omission_branches'][1]['requires'])
        with self.assertRaisesRegex(ValueError, 'share authenticated'):
            trace.inverse_selector_constraints(capture, 18, selector_capture=dict(capture, session_id='other'))
        with self.assertRaises(ValueError):
            trace.inverse_selector_constraints(capture, 19)

    def test_branches(self):
        self.assertEqual(trace.selector_result(1, None, None), 'already_extended')
        self.assertEqual(trace.selector_result(0, 1, None), 'alternate_opcode_0x67')
        self.assertEqual(trace.selector_result(0, 0, 1), 'reused_extsh')
        self.assertEqual(trace.selector_result(0, 0, 0), 'emit_extsh_0x65')
        self.assertEqual(trace.selector_result(0, 0x100, 0), 'emit_extsh_0x65')

    def test_preserve_all_debug_slots(self):
        context = types.SimpleNamespace(Dr0=11, Dr1=12, Dr2=13, Dr3=14,
                                        Dr6=0x400F, Dr7=0xD00001)
        # DR0 write control is legal; DR1/DR2 controls must be free.
        context.Dr7 = 0xD0001
        saved = trace.debug_state(context)
        trace.arm_execution(context, 0x4E1DA5)
        self.assertEqual(context.Dr0, 11)
        self.assertEqual(context.Dr3, 14)
        self.assertEqual(context.Dr7, saved[-1] | 4)
        trace.restore_debug_state(context, saved)
        self.assertEqual(trace.debug_state(context), saved)

    def test_occupied_slots_rejected(self):
        for control in (4, 8, 16, 32, 1 << 20, 1 << 24):
            context = types.SimpleNamespace(Dr0=0, Dr1=0, Dr2=0, Dr3=0,
                                            Dr6=0, Dr7=control)
            before = trace.debug_state(context)
            with self.assertRaises(ValueError):
                trace.arm_execution(context, 123)
            self.assertEqual(trace.debug_state(context), before)

    def test_simulated_observation_chain(self):
        base = types.SimpleNamespace(CounterDebugger=object,
                                     EXPECTED_CODEGEN_GLOBAL=100,
                                     TARGET_FUNCTION='test_function')
        capture = types.SimpleNamespace(base=base,
            expression_names=lambda reader, expression: ['CharModelLandDustCreate'])
        cls = trace.debugger_type(capture)
        debugger = cls.__new__(cls)
        debugger.base = 0x400000
        debugger.pending_decisions = {}
        debugger.result = {'events': []}
        debugger.runtime = lambda value: value
        debugger._read_codegen_token = lambda: 'opaque-session-token'
        debugger._append_event = debugger.result['events'].append
        values = {100: 200, 210: 300, 300: 54, 400: 1, 402: 2,
                  406: 5, 500: 0, 502: 73, 636: 3,
                  604: 101, 608: 3, 612: 73, 624: 0x444659,
                  0x5EAA88: 1000, 1040: 0}
        debugger.read_u32 = lambda address: values[address]
        debugger.value = lambda address, size, signed=False: values[address]
        context = types.SimpleNamespace(Eip=0x4E1DA5, Ebp=400, Esi=500,
            Esp=600, Ebx=0, Eax=0, Dr2=0, Dr7=4)
        debugger.observe(1, context)
        self.assertEqual(context.Dr2, 0x4E1DB7)
        for va in (0x4E1DB7, 0x4E1E10, 0x4E1E1C, 0x4E1E54, 0x4DD2D0):
            context.Eip = va
            debugger.observe(1, context)
        self.assertFalse(debugger.pending_decisions)
        self.assertEqual(context.Dr2, 0)
        row = debugger.result['events'][0]
        self.assertEqual(row['result'], 'emit_extsh_0x65')
        self.assertEqual(row['emission'], {'opcode': 101,
            'source_vreg': 73, 'destination_vreg': 3})
        self.assertEqual(row['source_field_join']['status'], 'MISSING_EDGE')
        self.assertEqual(row['type_predicate_al'], 0)
        self.assertEqual(row['reuse_predicate_al'], 0)

    def test_argument_join_requires_native_caller_and_type_identity(self):
        values = {624: 0x528D4A, 496: 700, 706: 400,
                  700: 0x38, 400: 1, 402: 2, 406: 5, 714: 800}
        reader = types.SimpleNamespace(base=0x400000,
            value=lambda address, size, signed=False: values[address],
            read_u32=lambda address: values[address], read_object_name=lambda address: 'namedChild')
        context = types.SimpleNamespace(Esp=600, Esi=500, Ebp=400)
        result = trace.argument_join(reader, context)
        self.assertEqual(result['status'], 'ARGUMENT_EXPRESSION_JOINED')
        self.assertEqual(result['expression']['object_name'], 'namedChild')
        self.assertEqual(result['expression']['type']['chain'][0]['byte_width'], 2)
        values[706] = 401
        self.assertEqual(trace.argument_join(reader, context)['status'], 'MISSING_EDGE')
        values[624] = 0x444659
        self.assertEqual(trace.argument_join(reader, context)['status'], 'MISSING_EDGE')

    def test_type_cycle_is_bounded(self):
        values = {100: 3, 102: 2, 114: 100}
        reader = types.SimpleNamespace(value=lambda address, size: values[address],
                                      read_u32=lambda address: values[address])
        self.assertEqual(trace.type_lineage(reader, 100)['status'], 'MISSING_EDGE')

    def test_conversion_stack_join_and_context_drift(self):
        values = {624: 0x44FFA6, 732: 700, 616: 900, 612: 500,
                  700: 0x30, 706: 400, 714: 900, 400: 1, 402: 2, 406: 5,
                  900: 0x38, 906: 1000, 914: 1100, 1000: 1, 1002: 4, 1006: 7}
        reader = types.SimpleNamespace(base=0x400000,
            value=lambda address, size, signed=False: values[address],
            read_u32=lambda address: values[address], read_object_name=lambda address: 'sourceChild')
        context = types.SimpleNamespace(Esp=600, Esi=500, Ebp=400)
        result = trace.argument_join(reader, context)
        self.assertEqual(result['status'], 'CONVERSION_EXPRESSION_JOINED')
        self.assertEqual(result['expression']['operand']['object_name'], 'sourceChild')
        self.assertEqual(result['expression']['operand']['type']['chain'][0]['byte_width'], 4)
        values.update({900: 4, 914: 1200, 1200: 49, 1206: 1000, 0x5E4A58: 0x450000})
        reader.runtime = lambda address: address
        joined = trace.argument_join(reader, context)['conversion_materialization']
        self.assertTrue(joined['width_forces_materialization_path'])
        self.assertEqual(joined['load_dispatch_target_rva'], '0x00050000')
        values[612] = 501
        self.assertEqual(trace.argument_join(reader, context)['status'], 'MISSING_EDGE')

    def test_bitfield_narrow_conversion_forces_materialization(self):
        tree = {'type': {'chain': [{'byte_width': 2}]}, 'operand': {
            'native_expression_kind': 4, 'operand': {'native_expression_kind': 49}}}
        result = trace.conversion_materialization(tree)
        self.assertTrue(result['width_forces_materialization_path'])
        self.assertFalse(result['source_candidate_supported'])
        tree['type']['chain'][0]['byte_width'] = 4
        self.assertFalse(trace.conversion_materialization(tree)['width_forces_materialization_path'])
        tree['operand']['operand']['native_expression_kind'] = 56
        self.assertFalse(trace.conversion_materialization(tree)['direct_bitfield_load'])


if __name__ == '__main__':
    unittest.main()
