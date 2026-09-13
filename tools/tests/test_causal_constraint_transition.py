import copy
import json
from pathlib import Path
import unittest

from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import row, report


class ConstraintTransitionTests(unittest.TestCase):
    def pair(self):
        target = [row('stwu r1, -0x20(r1)', 0), row('addi r5, r3, 20', 4), row('lwz r0, 0(r5)', 8)]
        old = report(copy.deepcopy(target), [target[0], row('addi r3, r3, 20', 4), row('lwz r0, 0(r3)', 8)])
        new = report(copy.deepcopy(target), copy.deepcopy(target))
        for doc, score in ((old, 90), (new, 100)):
            doc['left']['symbols'][0]['match_percent'] = score
        return old, new

    def test_resolution_and_neutral(self):
        old, new = self.pair()
        result = groups.compare_function_constraints(old, new, 'f')
        self.assertEqual(result['counts'], dict(resolved=2, persisting=0, introduced=0))
        self.assertFalse(result['retention_authorized'])
        self.assertEqual(groups.compare_function_constraints(old, old, 'f')['counts'],
                         dict(resolved=0, persisting=2, introduced=0))

    def test_changed_expression_persists_not_resolved(self):
        old, new = self.pair()
        new['right']['symbols'][0]['instructions'][1] = row('addi r5, r3, 24', 4)
        result = groups.compare_function_constraints(old, new, 'f')
        self.assertEqual(result['counts'], dict(resolved=1, persisting=1, introduced=0))

    def test_alignment_shift_does_not_change_target_identity(self):
        old, _ = self.pair()
        new = copy.deepcopy(old)
        new['left']['symbols'][0]['instructions'].insert(1, {})
        new['right']['symbols'][0]['instructions'].insert(1, row('li r6, 0', 2))
        result = groups.compare_function_constraints(old, new, 'f')
        self.assertEqual(result['counts'], dict(resolved=0, persisting=2, introduced=1))
        new['left']['symbols'][0]['instructions'].insert(1, {})
        new['right']['symbols'][0]['instructions'].insert(1, row('li r7, 0', 1))
        result = groups.compare_function_constraints(old, new, 'f')
        self.assertEqual(result['counts'], dict(resolved=0, persisting=2, introduced=0))
        self.assertEqual(result['unresolved_insertion_group_count'], 1)
        self.assertEqual(result['unresolved_insertion_groups'][0]['after_count'], 2)

    def test_incompatible_target_and_ambiguous_deletion(self):
        old, new = self.pair()
        new['left']['symbols'][0]['instructions'][1]['instruction']['address'] = '12'
        with self.assertRaisesRegex(ValueError, 'incompatible target'):
            groups.compare_function_constraints(old, new, 'f')
        old, new = self.pair()
        for side in ('left', 'right'):
            new[side]['symbols'][0]['instructions'].insert(1, {})
        with self.assertRaisesRegex(ValueError, 'ambiguous aligned deletions'):
            groups.compare_function_constraints(old, new, 'f')

    def test_active_pair(self):
        base = Path(__file__).resolve().parents[2] / 'build/small-first-20260913'
        paths = [base / lane / 'strict.json' for lane in
                 ('mqueue-control-shared-loop-index', 'mqueue-batch-slot-cursor')]
        if not all(path.is_file() for path in paths):
            self.skipTest('local active acceptance reports unavailable')
        old, new = [json.loads(path.read_bytes()) for path in paths]
        result = groups.compare_function_constraints(old, new, 'qCheckDeQueueOne')
        self.assertEqual(result['counts'], dict(resolved=8, persisting=2, introduced=2))
        self.assertTrue(any(r['function'] == 'qCheckDeQueueOne'
                            for r in result['score_size_frame_gates']['regressed']))
        self.assertEqual(result['structural_hazards'], [0, 3])
        self.assertEqual(groups.compare_function_constraints(old, old, 'qCheckDeQueueOne')['counts']['resolved'], 0)


if __name__ == '__main__':
    unittest.main()
