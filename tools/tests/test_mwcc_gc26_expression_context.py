import unittest
from unittest.mock import patch
from tools import mwcc_gc26_expression_context as c


class ContextTests(unittest.TestCase):
    def test_counter_watch_materializer_join_uses_actual_frame(self):
        from types import SimpleNamespace
        from unittest.mock import Mock
        from tools import capsule_same_session_capture as capture
        backend = capture.NativeWow64Backend.__new__(capture.NativeWow64Backend)
        backend.base = 0x400000
        backend.compiler_sha256 = c.COMPILER_SHA256
        backend._counter_saved = (1,)
        backend._counter_value = 54
        context = SimpleNamespace(Eip=0x4E2EA4, Esp=0x1000, Ebx=0, Ebp=54, Dr6=1)
        backend.threads = {1: 1}
        backend._read = Mock(side_effect=lambda address, size: {
            0x5EAA3C: (55).to_bytes(4, 'little'),
            0x4E2E9E: bytes.fromhex('ff053caa5e00'),
        }[address])
        backend.native = SimpleNamespace(WOW64_CONTEXT=lambda: context, kernel32=SimpleNamespace(
            Wow64GetThreadContext=Mock(return_value=True), Wow64SetThreadContext=Mock(return_value=True)))
        session = SimpleNamespace(bus=Mock(), session_id='session-test')
        expected = {'actual_destination_zero': True}
        with patch.object(capture, 'COUNTER_WRITES', True), \
                patch.object(capture.ctypes, 'byref', side_effect=lambda value: value), \
                patch.object(capture._expression_context, 'allocation_destination_request', return_value=expected) as decode:
            self.assertTrue(backend.counter_watch_event(session, 1))
        self.assertEqual(decode.call_args.kwargs['esp'], 0x1000)
        self.assertEqual(decode.call_args.kwargs['allocation_site'], 0xE2E9E)
        self.assertEqual(decode.call_args.kwargs['counter_before'], 54)
        self.assertIsNone(decode.call_args.kwargs['active'])
        self.assertEqual(session.bus.emit.call_args.args[2]['destination_request'], expected)

    def test_actual_materializer_destination_frame(self):
        base, esp, ret = 0x400000, 0x1000, 0x44FF94
        data = {base+0xE2C10: bytes.fromhex('5356575583ec088b7424208b5c2424'),
                base+0xE2E8E: bytes.fromhex('6685db74050fbfebeb0c'),
                esp+0x24: bytes(4), esp+0x18: ret.to_bytes(4, 'little'),
                ret-5: b'\xe8'+(base+0xE2C10-ret).to_bytes(4, 'little', signed=True)}
        args = dict(allocation_site=0xE2E9E, esp=esp, ebx=0, ebp=54,
                    counter_before=54, base=base,
                    active=dict(handler=0x44FA20, esp=0x1100, expression_kind=48),
                    read=lambda address, size: data[address][:size])
        result = c.allocation_destination_request(**args)
        self.assertEqual(result['caller_rva'], 0x4FF8F)
        self.assertEqual(result['active_expression_join'], 'direct_caller_inside_active_handler')
        self.assertFalse(result['source_repair_supported'])
        args['active'] = None
        self.assertEqual(c.allocation_destination_request(**args)['active_expression_join'], 'not_established')
        data[esp+0x24] = (1).to_bytes(4, 'little')
        with self.assertRaises(c.ContextError):
            c.allocation_destination_request(**args)
        args['allocation_site'] = 1
        self.assertIsNone(c.allocation_destination_request(**args))

    def test_counter_watch_opt_in_restored(self):
        from tools import capsule_same_session_capture as capture
        before = capture.COUNTER_WRITES
        with patch.object(capture, '_main_args', side_effect=lambda args: self.assertTrue(capture.COUNTER_WRITES) or 0):
            capture.main(['preflight', 'unused', '--trust-root', 'unused', '--expression-origins', '--counter-writes'])
        self.assertEqual(capture.COUNTER_WRITES, before)
        backend = capture.NativeWow64Backend.__new__(capture.NativeWow64Backend)
        with patch.object(capture, 'COUNTER_WRITES', False):
            self.assertFalse(backend.counter_watch_event(None, 1))

    def test_counter_schema_never_allows_raw_handler_arguments(self):
        from tools import capsule_same_session_capture as capture
        fields = capture._EVENT_ALLOWED_FIELDS['temporary_counter_write']
        self.assertNotIn('destination_arg2_raw', fields)
        self.assertNotIn('destination_arg3_raw', fields)
        self.assertIn('handler_arg2_state', fields)
        self.assertIn('handler_arg3_state', fields)

    def tracker(self, **kwargs):
        return c.ExpressionTracker(c.COMPILER_SHA256, **kwargs)

    def enter(self, t, thread=1, esp=4096, kind=54):
        return t.enter(thread, c.DISPATCH[kind], 8192, kind, esp, 12288, 16384)

    def test_map_and_deterministic_hooks(self):
        self.assertEqual(len(c.DISPATCH), 78)
        self.assertEqual(len(c.HANDLERS), 34)
        self.assertEqual(len(c.RETURN_OWNERS), 87)
        hooks = c.hook_descriptors()
        self.assertEqual(len(hooks), 121)
        self.assertEqual(hooks, c.hook_descriptors())
        self.assertEqual([h['address'] for h in hooks], sorted(h['address'] for h in hooks))
        self.assertEqual(c.HANDLERS[0x44d130]['returns'], (0x44d161,))
        self.assertEqual(c.HANDLERS[0x44d130]['prefix'].hex(), '53558b6c240c8b5c')
        with self.assertRaises(c.ContextError):
            c.hook_descriptors('wrong')

    def test_nested_same_handler_restores_parent_and_thread_isolation(self):
        t = self.tracker()
        self.enter(t)
        self.enter(t, esp=4000, kind=55)
        self.enter(t, thread=2)
        self.assertEqual(t.active(1)['expression_kind'], 55)
        t.exit(1, 0x44d161, 4000, 12288)
        self.assertEqual(t.active(1)['expression_kind'], 54)
        copy = t.active(1)
        copy['esp'] = 0
        self.assertEqual(t.active(1)['esp'], 4096)
        t.exit(1, 0x44d161, 4096, 12288)
        t.exit(2, 0x44d161, 4096, 12288)
        t.assert_empty()

    def test_primitive_reader_offsets(self):
        t = self.tracker()
        memory = {4096: 12288, 4100: 8192, 4112: 16384}
        t.read_entry(1, 0x44d130, 4096, memory.__getitem__, lambda p: 54)
        self.assertEqual(t.active(1)['result_descriptor'], 16384)
        t.read_exit(1, 0x44d161, 4096, memory.__getitem__)
        t.assert_empty()

    def test_mismatched_exit_poisoned(self):
        for site, esp, ret in [(0x45386c, 4096, 12288), (0x44d161, 4000, 12288),
                               (0x44d161, 4096, 12289)]:
            t = self.tracker()
            self.enter(t)
            with self.assertRaises(c.ContextError):
                t.exit(1, site, esp, ret)
            with self.assertRaises(c.ContextError):
                t.active(1)

    def test_wrong_kind_depth_and_non_descending_stack(self):
        t = self.tracker()
        with self.assertRaises(c.ContextError):
            t.enter(1, 0x44d130, 8192, 8, 4096, 12288, 16384)
        for bound, esp in [(1, 4000), (256, 4096)]:
            t = self.tracker(max_depth=bound)
            self.enter(t)
            with self.assertRaises(c.ContextError):
                self.enter(t, esp=esp)

    def test_unclosed_and_read_failure_fail_closed(self):
        t = self.tracker()
        self.enter(t)
        with self.assertRaises(c.ContextError):
            t.assert_empty()
        t = self.tracker()
        with self.assertRaises(KeyError):
            t.read_entry(1, 0x44d130, 4096, {}.__getitem__, lambda x: 54)
        with self.assertRaises(c.ContextError):
            t.active(1)


if __name__ == '__main__':
    unittest.main()
