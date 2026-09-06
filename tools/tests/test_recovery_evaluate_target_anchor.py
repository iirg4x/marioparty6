from __future__ import annotations

import copy
import unittest
from pathlib import Path

from tools import recovery_evaluate as evaluate
from tools.tests.test_focus_symbol_report import _instruction, _report


class RecoveryTargetAnchorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.target = [_instruction(256, "lfs f1, 0(r3)"), _instruction(260, "blr")]
        self.candidate = copy.deepcopy(self.target)
        self.candidate[0]["instruction"]["formatted"] = "lfs f2, 0(r3)"
        self.first = {"row": 0}

    def test_inserted_prologue_does_not_hide_original_site(self) -> None:
        target = [{}] * 30 + copy.deepcopy(self.target)
        candidate = [_instruction(i * 4, "nop") for i in range(30)] + self.candidate
        result = evaluate._target_anchor_context(self.target, target, candidate, self.first)
        self.assertEqual(result["status"], "located")
        self.assertEqual((result["baseline_row"], result["current_row"]), (0, 30))
        self.assertEqual(result["target_address"], 256)
        self.assertEqual(result["candidate"]["instruction"]["formatted"], "lfs f2, 0(r3)")
        self.assertEqual([row["row"] for row in result["context"]], [29, 31])
        self.assertTrue(result["diagnostic_only"])
        self.assertNotIn("closed", result)

    def test_changed_unrelated_target_instruction_is_unknown(self) -> None:
        after = copy.deepcopy(self.target)
        after[1]["instruction"]["formatted"] = "nop"
        result = evaluate._target_anchor_context(self.target, after, self.candidate, self.first)
        self.assertEqual(result["status"], "unknown")
        self.assertIn("stream changed", result["reason"])

    def test_duplicate_and_missing_addresses_are_unknown(self) -> None:
        for value in (256, None, True, -1, 2 ** 64):
            with self.subTest(value=value):
                rows = copy.deepcopy(self.target)
                rows[1]["instruction"]["address"] = value
                result = evaluate._target_anchor_context(rows, rows, self.candidate, self.first)
                self.assertEqual(result["status"], "unknown")
                self.assertNotIn("current_row", result)

    def test_gap_first_mismatch_and_absent_first_are_not_guessed(self) -> None:
        rows = [{}] + self.target
        result = evaluate._target_anchor_context(rows, rows, self.candidate, self.first)
        self.assertEqual(result["status"], "unknown")
        result = evaluate._target_anchor_context(self.target, self.target, self.candidate, None)
        self.assertEqual(result["status"], "none")

    def test_address_representations_and_missing_candidate_are_supported(self) -> None:
        after = copy.deepcopy(self.target)
        after[0]["instruction"]["address"] = "0x100"
        result = evaluate._target_anchor_context(self.target, after, [], self.first)
        self.assertEqual(result["status"], "located")
        self.assertIsNone(result["candidate"]["instruction"])
        self.assertLessEqual(len(result["context"]), 2)

    def test_integration_preserves_unknown_alignment_with_located_context(self) -> None:
        before = _report(focus_exact=False, sibling_exact=True)
        after = copy.deepcopy(before)
        after["left"]["symbols"][1]["instructions"].insert(0, {})
        after["right"]["symbols"][1]["instructions"].insert(0, _instruction(0, "nop"))
        result = evaluate._changed_result_diagnostics(
            root=Path("."), baseline_documents={"strict": before, "data": before},
            after_documents={"strict": after, "data": after},
            strict_path=Path("strict.json"), data_path=Path("data.json"), metric_changes=[],
            object_comparison={"functions": {"FocusFunction": {"raw_equal_base": False}}})
        channel = result["functions"][0]["channels"]["data"]
        self.assertEqual(channel["status"], "unknown")
        self.assertIn("row count changed", channel["reason"])
        anchor = channel["baseline_target_context"]
        self.assertEqual(anchor["status"], "located")
        self.assertEqual(anchor["current_row"], 1)

    def test_annotation_only_baseline_anchors_actual_instruction(self) -> None:
        before = _report(focus_exact=True, sibling_exact=True)
        for side in ("left", "right"):
            before[side]["symbols"][1]["instructions"][0]["diff_kind"] = "DIFF_ARG"
        after = copy.deepcopy(before)
        result = evaluate._changed_result_diagnostics(
            root=Path("."), baseline_documents={"strict": before, "data": before},
            after_documents={"strict": after, "data": after},
            strict_path=Path("strict.json"), data_path=Path("data.json"), metric_changes=[],
            object_comparison={"functions": {"FocusFunction": {"raw_equal_base": False}}})
        anchor = result["functions"][0]["channels"]["data"]["baseline_target_context"]
        self.assertIsNone(result["functions"][0]["channels"]["data"]["existing_first_instruction_mismatch"])
        self.assertEqual(anchor["status"], "located")
        self.assertEqual(anchor["anchor_basis"], "annotation")
        self.assertEqual(anchor["target_address"], 256)

    def test_instruction_anchor_precedes_earlier_annotation(self) -> None:
        before = _report(focus_exact=True, sibling_exact=True)
        for side in ("left", "right"):
            before[side]["symbols"][1]["instructions"][0]["diff_kind"] = "DIFF_ARG"
        before["right"]["symbols"][1]["instructions"][1]["instruction"]["formatted"] = "nop"
        result = evaluate._changed_result_diagnostics(
            root=Path("."), baseline_documents={"strict": before, "data": before},
            after_documents={"strict": before, "data": before},
            strict_path=Path("strict.json"), data_path=Path("data.json"), metric_changes=[],
            object_comparison={"functions": {"FocusFunction": {"raw_equal_base": False}}})
        channel = result["functions"][0]["channels"]["data"]
        self.assertEqual(channel["existing_first_instruction_mismatch"]["row"], 1)
        self.assertEqual(channel["baseline_target_context"]["anchor_basis"], "instruction")
        self.assertEqual(channel["baseline_target_context"]["target_address"], 260)


if __name__ == "__main__":
    unittest.main()
