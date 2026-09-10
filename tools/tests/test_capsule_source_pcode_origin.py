import unittest
import tempfile
from pathlib import Path
from unittest.mock import patch
from tools import capsule_same_session_capture as c


class OriginTests(unittest.TestCase):

    def test_reset_and_allocation_native_fields(self):
        backend = c.NativeWow64Backend.__new__(c.NativeWow64Backend)
        backend.compiler_sha256 = c.GC26_COMPILER_SHA256
        backend._runtime = lambda x: x
        regs = {'eax': 100, 'ecx': 32, 'esp': 1000}
        backend.read_register = lambda thread, reg: regs[reg]
        values = {102: 32, 0x5eaa3c: 33, 1012: 2000, 2000: 1, 2002: 4, 2006: 7}
        backend._read = lambda address, size: values[address].to_bytes(size, 'little')
        self.assertEqual(backend.capture_return_temp_allocation(1)['allocated_vreg'], 32)
        self.assertEqual(backend.capture_return_temp_allocation(1)['native_return_type_width'], 4)
        values[102] = 31
        with self.assertRaises(c.Rejected):
            backend.capture_return_temp_allocation(1)

    def test_reset_epoch_link(self):
        session = c.CombinedCaptureSession.__new__(c.CombinedCaptureSession)
        session.session_id = 'session-0123456789abcdef'
        session.function_entered, session.target_complete = True, False
        session.bus = c.EventBus(session_id=session.session_id, function='f')
        session.bus.bind_process(1)
        raw = dict(temporary_class=4, current=257, saved_base=32, high_water=257,
                   source_offset=20, reset_inhibition=0)
        session._call_backend = lambda *a: dict(raw)
        session.on_hook(next(r for r in c.GC26_TEMP_RESET_HOOKS if r['role'] == 'temp_reset_pre'), 1)
        raw['current'] = 32
        session.on_hook(next(r for r in c.GC26_TEMP_RESET_HOOKS if r['role'] == 'temp_reset_post'), 1)
        self.assertEqual(session.bus.events[0]['counter_before'], 257)
        self.assertEqual(session._preceding_gpr_reset, session.bus.events[0]['event_id'])

    def test_expression_tracker_seal(self):
        with patch.object(c, 'HOOKS', c.GC26_EXPRESSION_ORIGIN_HOOKS):
            self.assertEqual(c._hooks_for_compiler(c.GC26_COMPILER_SHA256), c.GC26_EXPRESSION_ORIGIN_HOOKS)
            with patch.object(c, 'GC26_EXPRESSION_TRACKER_SHA256', '0' * 64):
                with self.assertRaises(c.Rejected):
                    c._hooks_for_compiler(c.GC26_COMPILER_SHA256)

    def test_postcapture_only_exact_request_output(self):
        output = Path('/private/capture')
        names = c._postcapture_output_names(output, [output / 'capture.o', Path('/other/rogue.o')])
        self.assertIn('capture.o', names)
        self.assertNotIn('rogue.o', names)
        self.assertNotIn('extra.o', names)

    def test_output_parent_preflight(self):
        with tempfile.TemporaryDirectory() as directory:
            c._require_compiler_output_parent(Path(directory) / 'capture.o')
            with self.assertRaises(c.Rejected):
                c._require_compiler_output_parent(Path(directory) / 'absent' / 'capture.o')

    def test_cli_selection_restored(self):
        original = c.HOOKS
        with patch.object(c, '_main_args', side_effect=lambda args: self.assertEqual(
                c.HOOKS, c.GC26_SOURCE_ORIGIN_HOOKS) or 0):
            self.assertEqual(c.main(['preflight', 'unused', '--trust-root', 'unused',
                                     '--source-origins']), 0)
        self.assertIs(c.HOOKS, original)
        with patch.object(c, '_main_args', side_effect=lambda args: self.assertEqual(
                c.HOOKS, c.GC26_EXPRESSION_ORIGIN_HOOKS) or 0):
            self.assertEqual(c.main(['preflight', 'unused', '--trust-root', 'unused',
                                     '--expression-origins']), 0)
        self.assertIs(c.HOOKS, original)
        with patch.object(c, '_main_args', side_effect=RuntimeError('stop')):
            with self.assertRaises(RuntimeError):
                c.main(['validate', 'unused', '--trust-root', 'unused', '--source-origins'])
        self.assertIs(c.HOOKS, original)

    def test_opt_in_and_unsupported(self):
        with patch.object(c, 'HOOKS', c.GC26_SOURCE_ORIGIN_HOOKS):
            c._validate_runtime_hook_patch(c.GC26_COMPILER_SHA256)
            with self.assertRaises(c.Rejected):
                c._validate_runtime_hook_patch(c.GC27_COMPILER_SHA256)

    def test_native_frame_and_missing(self):
        backend = c.NativeWow64Backend.__new__(c.NativeWow64Backend)
        backend.compiler_sha256 = c.GC26_COMPILER_SHA256
        backend._runtime = lambda x: x
        backend.read_register = lambda thread, reg: 100
        values = {0x5e9f10: 200, 222: 42, 128: 42, 0x5eaa3c: 33, 0x5ea810: 0}
        backend._read = lambda address, size: values[address].to_bytes(size, 'little')
        row = backend.capture_source_pcode_origin(1)
        self.assertNotIn('expression_pointer', row)
        self.assertEqual(row['source_offset'], 42)
        values[128] = 43
        with self.assertRaises(c.Rejected):
            backend.capture_source_pcode_origin(1)

    def test_call_frame_lifo(self):
        session = c.CombinedCaptureSession.__new__(c.CombinedCaptureSession)
        session.function_entered = True
        session.target_complete = False
        raw = {'stack': 100, 'return': 500, 'expression': 300}
        session._call_backend = lambda *args: dict(raw)
        session.on_hook(c.GC26_CALL_CONTEXT_HOOKS[0], 1)
        raw['stack'] = 80
        session.on_hook(c.GC26_CALL_CONTEXT_HOOKS[0], 1)
        session.on_hook(c.GC26_CALL_CONTEXT_HOOKS[1], 1)
        with self.assertRaises(c.Rejected):
            session.on_hook(c.GC26_CALL_CONTEXT_HOOKS[1], 1)
        raw['stack'] = 100
        session.on_hook(c.GC26_CALL_CONTEXT_HOOKS[1], 1)
        self.assertEqual(session._call_context_stacks[1], [])

    def test_default_unchanged(self):
        with patch.object(c, 'HOOKS', c.LEGACY_HOOKS):
            self.assertEqual(c._hooks_for_compiler(c.GC26_COMPILER_SHA256), c.GC26_HOOKS)

    def test_emit_shared_identity_and_reject_reuse(self):
        session = c.CombinedCaptureSession.__new__(c.CombinedCaptureSession)
        session.session_id = 'session-0123456789abcdef'
        session.function_entered = True
        session.target_complete = False
        session.pcode_tokens = {}
        session.bus = c.EventBus(session_id=session.session_id, function='f')
        session.bus.bind_process(1)
        session._call_backend = lambda *a: dict(pcode_pointer=100, codegen_pointer=200,
            expression_pointer=300, expression_kind=54, source_offset=42)
        session.on_hook(c.GC26_SOURCE_ORIGIN_HOOK, 1)
        event = session.bus.events[0]
        self.assertEqual(event['pcode_token'], session._pcode_token(100))
        self.assertEqual(event['owner_role'], 'enclosing_codegen')
        context = dict(session_id=session.session_id, process_id=1, function='f',
                       compiler={'sha256': c.GC26_COMPILER_SHA256})
        c._validate_event(event, 0, context)
        with self.assertRaises(c.Rejected):
            c._validate_event(dict(event, owner_role='actual_call'), 0, context)
        with self.assertRaises(c.Rejected):
            session.on_hook(c.GC26_SOURCE_ORIGIN_HOOK, 1)

    def test_recursive_origin_exact_and_outside_context(self):
        session = c.CombinedCaptureSession.__new__(c.CombinedCaptureSession)
        session.session_id = 'session-0123456789abcdef'
        session.function_entered, session.target_complete = True, False
        session.pcode_tokens = {}
        session.bus = c.EventBus(session_id=session.session_id, function='f')
        session.bus.bind_process(1)
        rows = {r['address']: r for r in c.GC26_EXPRESSION_CONTEXT_HOOKS}
        session._call_backend = lambda *a: dict(esp=1000, return_pc=2000,
            expression_pointer=3000, expression_kind=54, result_descriptor=4000)
        session.on_hook(rows[0x44D130], 1)
        session._call_backend = lambda *a: 'KnownCall' if a[0] == 'capture_direct_callee' else dict(pcode_pointer=100, codegen_pointer=200, source_offset=42)
        session.on_hook(c.GC26_SOURCE_ORIGIN_HOOK, 1)
        event = session.bus.events[0]
        self.assertEqual(event['child_edge'], 'CAPTURED_ACTIVE_HANDLER')
        self.assertEqual(event['expression_kind'], 54)
        context = dict(session_id=session.session_id, process_id=1, function='f',
                       compiler={'sha256': c.GC26_COMPILER_SHA256})
        with patch.object(c, 'HOOKS', c.GC26_EXPRESSION_ORIGIN_HOOKS):
            c._validate_event(event, 0, context)
        session._call_backend = lambda *a: dict(esp=1000, return_pc=2000)
        session.on_hook(rows[0x44D161], 1)
        session._call_backend = lambda *a: dict(pcode_pointer=101, codegen_pointer=200, source_offset=42)
        session.on_hook(c.GC26_SOURCE_ORIGIN_HOOK, 1)
        self.assertEqual(session.bus.events[1]['child_edge'], 'MISSING_RECURSIVE_CHILD')


if __name__ == '__main__':
    unittest.main()
