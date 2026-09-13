"""Keep target constraints usable when candidate-only boundary pairing is unknown."""
import copy
import json
from pathlib import Path
import unittest

from tools import recovery_causal_groups as groups


def row(text, address):
    return {'instruction': {'formatted': text, 'address': str(address), 'size': 4}}


def report(target, candidate):
    return {side: {'symbols': [{'name': 'f', 'kind': 'SYMBOL_FUNCTION',
            'size': str(4 * sum(bool(r.get('instruction')) for r in rows)),
            'instructions': rows, 'match_percent': 90}]}
            for side, rows in (('left', target), ('right', candidate))}


def pair():
    target = [row('stwu r1, -0x20(r1)', 0), row('addi r5, r3, 20', 4), row('blr', 8)]
    baseline = report(copy.deepcopy(target),
                      [target[0], row('addi r3, r3, 20', 4), target[2]])
    candidate = report([target[0], {}, {}, target[1], target[2]],
                       [target[0], row('li r6, 1', 4), row('li r7, 2', 8), target[1], target[2]])
    return baseline, candidate


class TransitionAmbiguityTests(unittest.TestCase):
    def test_shared_boundary_does_not_discard_target_resolution(self):
        baseline, candidate = pair()
        result = groups.compare_function_constraints(baseline, candidate, 'f')
        self.assertEqual(result['counts']['resolved'], 1)
        self.assertEqual(result['counts']['introduced'], 0)
        unresolved = result['unresolved_insertion_groups']
        self.assertEqual(len(unresolved), 1)
        self.assertEqual(len(unresolved[0]['before']), 0)
        self.assertEqual(len(unresolved[0]['after']), 2)
        self.assertTrue(unresolved[0]['reason'])
        self.assertFalse(result['retention_authorized'])

    def test_ambiguous_neutral_never_invents_resolution(self):
        _, candidate = pair()
        result = groups.compare_function_constraints(candidate, copy.deepcopy(candidate), 'f')
        self.assertEqual(result['counts']['resolved'], 0)
        self.assertEqual(result['counts']['introduced'], 0)
        self.assertEqual(len(result['unresolved_insertion_groups']), 1)

    def test_singleton_to_multiple_does_not_resolve_singleton(self):
        _, candidate = pair()
        baseline = copy.deepcopy(candidate)
        for side in ('left', 'right'):
            baseline[side]['symbols'][0]['instructions'].pop(2)
        baseline['right']['symbols'][0]['size'] = '16'
        result = groups.compare_function_constraints(baseline, candidate, 'f')
        self.assertEqual(result['counts']['resolved'], 0)
        self.assertEqual(result['counts']['introduced'], 0)
        self.assertEqual(len(result['unresolved_insertion_groups'][0]['before']), 1)
        self.assertEqual(len(result['unresolved_insertion_groups'][0]['after']), 2)

    def test_exact_neutral_and_incompatible_target(self):
        baseline, _ = pair()
        baseline['right']['symbols'][0]['instructions'] = copy.deepcopy(
            baseline['left']['symbols'][0]['instructions'])
        result = groups.compare_function_constraints(baseline, baseline, 'f')
        self.assertTrue(all(count == 0 for count in result['counts'].values()))
        other = copy.deepcopy(baseline)
        other['left']['symbols'][0]['instructions'][1]['instruction']['address'] = '12'
        with self.assertRaisesRegex(ValueError, 'incompatible target'):
            groups.compare_function_constraints(baseline, other, 'f')

    def test_ambiguous_detail_limits_preserve_full_counts(self):
        target = [row('addi r3, r3, 1', i * 4) for i in range(30)]
        baseline = report(copy.deepcopy(target), copy.deepcopy(target))
        left, right = [], []
        for index, instruction in enumerate(target):
            inserted = 14 if index == 0 else 2
            left.extend({} for _ in range(inserted))
            right.extend(row('li r6, 1', 1000 + len(right) * 4 + n * 4)
                         for n in range(inserted))
            left.append(instruction)
            right.append(instruction)
        candidate = report(left, right)
        result = groups.compare_function_constraints(baseline, candidate, 'f')
        self.assertEqual(result['unresolved_insertion_group_count'], 30)
        self.assertEqual(len(result['unresolved_insertion_groups']), 24)
        first = result['unresolved_insertion_groups'][0]
        self.assertEqual(first['after_count'], 14)
        self.assertEqual(len(first['after']), 12)
        self.assertTrue(first['after_truncated'])
        self.assertTrue(result['details_truncated'])
        self.assertEqual(result['counts'], dict(resolved=0, persisting=0, introduced=0))

    def test_active_smoothing_pair(self):
        root = Path(__file__).resolve().parents[2]
        paths = [root / 'build/small-remaining-baseline-20260913/gssdk_lib/asrpho/common/blocks/flblocks/smoother/current-headers/base.json',
                 root / 'build/small-first-20260913/smoother-input-end-recurrence/strict.json']
        if not all(path.is_file() for path in paths):
            self.skipTest('local active Smoothing reports unavailable')
        baseline, candidate = [json.loads(path.read_bytes()) for path in paths]
        result = groups.compare_function_constraints(baseline, candidate, 'Smoothing')
        self.assertGreater(result['counts']['resolved'], 0)
        self.assertEqual(len(result['unresolved_insertion_groups']), 1)
        self.assertEqual(len(result['unresolved_insertion_groups'][0]['after']), 2)
        neutral = groups.compare_function_constraints(candidate, candidate, 'Smoothing')
        self.assertEqual(neutral['counts']['resolved'], 0)
        self.assertEqual(neutral['counts']['introduced'], 0)
        self.assertFalse(result['retention_authorized'])


if __name__ == '__main__':
    unittest.main()
