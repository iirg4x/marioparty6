import struct
import unittest
from unittest.mock import patch
from tools import recovery_expression_join as j


class JoinTests(unittest.TestCase):
    def test_actionable_regions_binding_and_no_trace(self):
        source = b'int f(void) { return 0; }'
        with self.assertRaises(ValueError):
            j.actionable_source_regions(source, 'f', [], 'stale')
        result = j.actionable_source_regions(source, 'f', [], j.sha(source), True)
        self.assertEqual(result['observed'], [])
        self.assertIsNone(result['shared_initializers'])

    def test_actionable_regions_keep_ambiguous_calls_and_exact_initializers(self):
        source = b'int f(void) {\n int count, flags[2];\n count = flags[0] = flags[1] = 0;\n sink() + sink();\n return count;\n}\n'
        origins = [{'event_id': 'one', 'source_offset': 4, 'expression_kind': 54,
                    'direct_callee_name': 'sink'},
                   {'event_id': 'two', 'source_offset': 4, 'expression_kind': 54,
                    'direct_callee_name': 'sink'}]
        result = j.actionable_source_regions(source, 'f', origins, j.sha(source), True)
        self.assertEqual(len(result['observed']), 2)
        self.assertEqual(result['inferred'][0]['candidate_count'], 2)
        self.assertEqual(result['inferred'][0]['status'], 'inferred')
        self.assertTrue(any('ambiguous_child' in x for x in result['unknown_edges']))
        self.assertEqual(len(result['shared_initializers']['sites']), 1)
        self.assertIsNone(result['source_patch'])

    def test_actionable_brace_is_inferred_enclosure(self):
        source = b'int f(void) {\n while (limit()) {\n tick();\n }\n return 0;\n}\n'
        result = j.actionable_source_regions(source, 'f', [
            {'event_id': 'brace', 'source_offset': 4, 'expression_kind': 54,
             'direct_callee_name': 'limit'}], j.sha(source))
        self.assertEqual(result['observed'][0]['source_line'], 4)
        self.assertEqual(result['inferred'][0]['region']['start_line'], 2)
        self.assertEqual(result['inferred'][0]['region']['end_line'], 4)

    def test_multiple_statements_on_captured_line_are_not_selected(self):
        source = b'int f(void) {\n sink(); sink();\n return 0;\n}\n'
        result = j.source_boundary(source, 'f', {'source_offset': 2,
                                   'expression_kind': 54, 'direct_callee_name': 'sink'})
        self.assertEqual(result['status'], 'UNKNOWN')
        self.assertEqual(result['reason'], 'multiple_source_statements_at_coordinate')

    def event(self, kind, **extra):
        return dict(session_id='session-test', process_id=1, function='f',
                    event_kind=kind, pcode_token='pcode-session-test-000001', **extra)

    def origin(self):
        return self.event('source_pcode_origin', owner_role='enclosing_codegen',
                          child_edge='CAPTURED_ACTIVE_HANDLER', status='CAPTURED')

    def test_exact_word_unknown_decoder(self):
        event = self.event('machine_emission', instruction_index=0, emitted_offset=0,
                           ppc_word=0x1c760108, ppc_bytes='1c760108', status='UNKNOWN')
        self.assertEqual(j.compare_words([event], struct.pack('>I', 0x1c760108), 'f')['status'], 'exact_words')
        self.assertEqual(j.compare_words([event], struct.pack('>I', 0), 'f')['status'], 'mismatch')

    def test_cross_session(self):
        event = self.origin()
        envelope = dict(context=dict(session_id='session-test', process_id=1), events=[event])
        j.validate_session(envelope, 'f')
        event['session_id'] = 'session-other'
        with self.assertRaises(ValueError):
            j.validate_session(envelope, 'f')

    def test_missing_duplicate_and_child_authority(self):
        token = 'pcode-session-test-000001'
        self.assertEqual(j.source_origin_join([], 'session-test', 'f', token)['status'], 'UNKNOWN')
        self.assertEqual(j.source_origin_join([self.origin()] * 2, 'session-test', 'f', token)['status'], 'UNKNOWN')
        event = self.origin()
        event['owner_role'] = 'actual_call'
        self.assertEqual(j.source_origin_join([event], 'session-test', 'f', token)['status'], 'UNKNOWN')

    def test_exact_token_join(self):
        events = [self.origin(), self.event('machine_emission')]
        joined = j.source_origin_join(events, 'session-test', 'f', 'pcode-session-test-000001')
        self.assertEqual(len(joined['machine']), 1)
        self.assertEqual(joined['child_edge'], 'CAPTURED_ACTIVE_HANDLER')

    def test_loop_end_is_context_not_call_position(self):
        source = b'int f(void) {\n int i;\n for(i=0; i<limit(); i++) {\n  tick();\n }\n return 0;\n}\n'
        origin = dict(source_offset=5, expression_kind=54)
        result = j.source_boundary(source, 'f', origin)
        self.assertEqual(result['container']['type'], 'for_statement')
        self.assertEqual(result['candidate_count'], 2)
        self.assertFalse(result['exact_child_source_span'])
        named = j.source_boundary(source, 'f', dict(origin, direct_callee_name='limit'))
        self.assertEqual(named['candidate_count'], 1)
        self.assertEqual(named['candidates'][0]['start_line'], 3)
        self.assertFalse(named['source_patch_ranked'])

    def test_native_multiply_can_be_implicit_array_scale(self):
        source = b'int f(int i) {\n int n = players[i].field;\n return n;\n}\n'
        result = j.source_boundary(source, 'f', dict(source_offset=2, expression_kind=9))
        self.assertEqual(result['container']['type'], 'declaration')
        self.assertEqual(result['candidate_count'], 1)
        self.assertEqual(result['candidates'][0]['type'], 'subscript_expression')
        self.assertFalse(result['exact_child_source_span'])

    def test_source_coordinate_does_not_leak_into_other_function(self):
        source = b'int other(void) { return g(); }\nint f(void) { return 0; }\n'
        self.assertEqual(j.source_boundary(source, 'f', dict(source_offset=1, expression_kind=54))['status'], 'UNKNOWN')

    def test_compact_output_is_bounded_and_details_explicit(self):
        for details in (False, True):
            rows = {'rewrites': [{'source_origin': {'status': 'UNKNOWN'}} for _ in range(40)],
                    'unions': [{'confirmed': True, 'status': 'CAPTURED', 'pcode_token': 'p'} for _ in range(40)]}
            with patch.object(j.Path, 'read_bytes', return_value=b'{"events": []}'), \
                 patch.object(j, 'validate_session', return_value='session-test'), \
                 patch.object(j, 'bind', return_value={'comparison': {'status': 'exact_words'},
                     'source_sha256': j.sha(b'{"events": []}')}), \
                 patch.object(j, 'trace_events', return_value=rows):
                result = j.analyze('envelope', 'object', 'source', 'f', 32, details)
            self.assertEqual(len(result['rewrites']), 40 if details else 32)
            self.assertEqual(len(result['unions']), 40 if details else 32)
            if not details:
                self.assertTrue(result['truncated'])
                self.assertEqual(result['total_rewrites'], 40)

    def test_confirmed_missing_union_cannot_join_source(self):
        event = self.event('pcode_alias_union', confirmed=True, status='MISSING', old_index=32, new_index=3)
        with patch.object(j.Path, 'read_bytes', return_value=b'{"events": []}'), \
             patch.object(j, 'validate_session', return_value='session-test'), \
             patch.object(j, 'bind', return_value={'comparison': {'status': 'exact_words'}}), \
             patch.object(j, 'trace_events', return_value=j.trace_events([event], 'f', 32)), \
             patch.object(j, 'source_origin_join') as source_join:
            with self.assertRaisesRegex(ValueError, 'unconfirmed union'):
                j.analyze('envelope', 'object', 'source', 'f', 32)
            source_join.assert_not_called()


if __name__ == '__main__':
    unittest.main()
