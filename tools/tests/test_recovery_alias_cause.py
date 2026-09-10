import unittest
from unittest.mock import patch
from tools import recovery_alias_cause as c


class SliceTests(unittest.TestCase):
    def test_no_discard_plan_api(self):
        self.assertFalse(hasattr(c, 'build_discard_plan'))
        self.assertTrue(callable(c.build_return_allocation_census))

    def test_census_without_reset_is_not_source_plan(self):
        with patch.object(c, 'analyze', return_value={'lifecycle': {'status': 'unknown'}}):
            result = c.build_return_allocation_census('.', 'index', 'envelope', 'f')
        self.assertEqual(result['status'], 'not_applicable')
        self.assertNotIn('requests', result)

    def setUp(self):
        self.left = [{'instruction': {'formatted': 'mulli r4, r22, 264', 'size': 4, 'address': '100'}}]
        self.right = [{'instruction': {'formatted': 'mulli r3, r22, 264', 'size': 4, 'address': '100'}}]
        common = dict(function='f', session_id='session-test', status='CAPTURED', confirmed=True)
        self.events = [dict(common, event_kind='machine_emission', instruction_index=0, pcode_token='p'),
            dict(common, event_kind='pcode_capture', pcode_token='p', operand_ordinal=0,
                 operand_bank='GPR', operand_flags=2, operand_index=3, final_color=3),
            dict(common, event_kind='pcode_alias_rewrite', pcode_token='p', operand_ordinal=0, old_index=32, new_index=3),
            dict(common, event_kind='pcode_alias_union', pcode_token='q', old_index=32, new_index=3),
            dict(common, event_kind='source_pcode_origin', pcode_token='p', owner_role='enclosing_codegen',
                 child_edge='CAPTURED_ACTIVE_HANDLER', expression_kind=9, source_offset=10),
            dict(common, event_kind='source_pcode_origin', pcode_token='q', owner_role='enclosing_codegen',
                 child_edge='CAPTURED_ACTIVE_HANDLER', expression_kind=54, source_offset=20)]

    def run_slice(self):
        with patch.object(c.evaluator, '_diagnostic_rows', return_value=(self.left, self.right)), \
             patch.object(c.evaluator, '_diagnostic_mismatches', return_value={0: None}):
            return c.slice_events({}, self.events, 'f')

    def test_discovers_original_id_and_alias_origin_without_input_id(self):
        r = self.run_slice()
        self.assertEqual(r['operand']['original_id'], 32)
        self.assertEqual(r['operand']['target'], 'r4')
        self.assertEqual(r['alias_unions'][0]['origin']['source_offset'], 20)
        self.assertIsNone(r['source_constraint'])
        self.assertFalse(r['cause_proven'])

    def test_structural_change_refused(self):
        self.right[0]['instruction']['formatted'] = 'mulli r3, r22, 265'
        with self.assertRaises(ValueError):
            self.run_slice()

    def test_multiple_changed_operands_are_unknown(self):
        self.left[0]['instruction']['formatted'] = 'add r4, r5, r6'
        self.right[0]['instruction']['formatted'] = 'add r3, r7, r8'
        result = self.run_slice()
        self.assertEqual(result['status'], 'unknown')
        self.assertIn('multiple changed operands', result['next_missing_edge'])
        self.assertNotIn('operand', result)

    def test_missing_operand_and_ambiguous_rewrite(self):
        self.events[1]['final_color'] = 5
        self.assertEqual(self.run_slice()['status'], 'unknown')
        self.events[1]['final_color'] = 3
        self.events.append(dict(self.events[2]))
        self.assertEqual(self.run_slice()['next_missing_edge'], 'unique original operand rewrite chain')

    def test_unconfirmed_union_refused(self):
        self.events[3]['confirmed'] = False
        with self.assertRaises(ValueError):
            self.run_slice()

    def test_missing_creation_never_infers_source(self):
        self.events = self.events[:4]
        self.assertEqual(self.run_slice()['creation']['status'], 'UNKNOWN')

    def test_lifecycle_exact_expression_reset_and_conditional_budget(self):
        common = dict(session_id='s', process_id=1, function='f', status='CAPTURED')
        events = [dict(common, event_id='early', sequence=2),
                  dict(common, event_id='reset', sequence=10, event_kind='temporary_lane_reset',
                       temporary_class=4, saved_base=32, counter_before=259, counter_after=32),
                  dict(common, event_id='birth', sequence=11, event_kind='return_temp_allocation',
                       expression_token='expr', allocated_vreg=32, counter_after=33, preceding_gpr_reset_event='reset'),
                  dict(common, event_id='union', sequence=12)]
        result = dict(operand={'original_id': 32, 'bank': 'GPR'}, creation={'event_id': 'early'},
                      alias_unions=[{'event_id': 'union', 'origin': {'expression_token': 'expr'}}])
        got = c.lifecycle_constraint(events, result)
        self.assertEqual(got['status'], 'observed_cross_reset_identity_reuse')
        self.assertEqual(got['conditional_counter_budget_alternative']['minimum_pre_reset_reduction'], 3)
        self.assertFalse(got['conditional_counter_budget_alternative']['predicted_match'])
        for key, value in [('session_id', 'other'), ('temporary_class', 5), ('sequence', 20)]:
            old = events[1][key]
            events[1][key] = value
            with self.assertRaises(ValueError):
                c.lifecycle_constraint(events, result)
            events[1][key] = old
        events[2]['expression_token'] = 'unrelated'
        self.assertEqual(c.lifecycle_constraint(events, result)['status'], 'unknown')


if __name__ == '__main__':
    unittest.main()
