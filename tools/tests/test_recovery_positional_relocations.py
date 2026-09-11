from __future__ import annotations

import copy
import tempfile
import unittest
from pathlib import Path

from tools import recovery_evaluate as evaluate
from tools import recovery_object_inventory as inventory
from tools.tests.test_recovery_object_inventory import _write_elf


class PositionalRelocationTests(unittest.TestCase):
    def setUp(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.o"
            _write_elf(path)
            self.target = inventory.inventory(path)
        self.base = copy.deepcopy(self.target)
        self.candidate = copy.deepcopy(self.target)
        for value, offsets, size, raw in (
                (self.target, [0, 8, 16, 24], 40, "target"),
                (self.base, [0, 12, 20, 28], 48, "base"),
                (self.candidate, [4, 8, 16, 24], 44, "candidate")):
            row = value["functions"]["foo"]
            references = [{"offset": offset, "type": 10,
                           "effective_target": {"kind": "undefined", "name": name, "addend": 0}}
                          for offset, name in zip(offsets, ["a", "b", "c", "d"])]
            row.update(size=size, raw_sha256=raw, relocations=references,
                       relocations_sha256=inventory._sha(references),
                       physical_relocations=copy.deepcopy(references))

    def classify(self, *, exact=False, focus=None, after_rows=15):
        comparison = inventory.compare(self.target, self.base, self.candidate, ["foo"])
        def metric(size, differences, is_exact=False):
            return [{"function": "foo", "candidate_bytes": size, "target_bytes": 40,
                     "diff_rows": differences, "instruction_exact": is_exact, "match_percent": None}]
        before = metric(self.base["functions"]["foo"]["size"], 18, exact)
        after = metric(self.candidate["functions"]["foo"]["size"], after_rows)
        return evaluate._classify(before, before, after, after, comparison,
                                  ["foo"] if focus is None else focus), comparison

    def test_positional_loss_can_be_qualified_partial_gain(self):
        result, comparison = self.classify()
        row = comparison["functions"]["foo"]
        self.assertEqual(row["closed_normalized_row_loss_count"], 1)
        self.assertEqual((row["normalized_diff_before"], row["normalized_diff_after"]), (6, 2))
        self.assertTrue(row["ordered_normalized_relocations_equal"])
        self.assertEqual(result["status"], "improved")
        self.assertTrue(result["retention_ready"])
        self.assertEqual(result["qualified_positional_relocation_shifts"], ["foo"])
        self.assertFalse(row["candidate_physical_exact"])

    def test_reference_changes_fail_closed(self):
        original = copy.deepcopy(self.candidate)
        for mutation in ("symbol", "addend", "type", "reorder", "count", "destination"):
            with self.subTest(mutation=mutation):
                self.candidate = copy.deepcopy(original)
                rows = self.candidate["functions"]["foo"]["relocations"]
                if mutation == "symbol":
                    rows[1]["effective_target"]["name"] = "other"
                elif mutation == "addend":
                    rows[1]["effective_target"]["addend"] = 4
                elif mutation == "type":
                    rows[1]["type"] = 1
                elif mutation == "reorder":
                    rows[1]["effective_target"], rows[2]["effective_target"] = (
                        rows[2]["effective_target"], rows[1]["effective_target"])
                elif mutation == "count":
                    rows.pop()
                else:
                    rows[1]["effective_target"] = {"kind": "function", "name": "b", "offset": 4}
                result, comparison = self.classify()
                self.assertFalse(comparison["functions"]["foo"]["ordered_normalized_relocations_equal"])
                self.assertEqual(result["status"], "rejected")

    def test_base_candidate_equality_without_target_equality_is_insufficient(self):
        for value in (self.base, self.candidate):
            value["functions"]["foo"]["relocations"][1]["effective_target"]["name"] = "other"
        self.assertEqual(self.classify()[0]["status"], "rejected")

    def test_exact_function_is_not_eligible(self):
        result, _ = self.classify(exact=True)
        self.assertEqual(result["status"], "rejected")
        self.assertEqual(result["qualified_positional_relocation_shifts"], [])

    def test_same_inexact_size_with_code_and_canonical_gain_is_eligible(self):
        self.candidate["functions"]["foo"]["size"] = 48
        self.assertEqual(self.classify()[0]["status"], "improved")
        self.assertEqual(self.classify(after_rows=18)[0]["status"], "rejected")

    def test_raw_unchanged_nontext_reference_with_changed_code_owner_fails(self):
        for value in (self.base, self.candidate):
            value["allocated_relocations"].append(dict(section=".data", offset=1000,
                type=1, symbol={}, addend=0,
                effective_target=dict(kind="section", section=".text", offset=4)))
        self.candidate["functions"]["foo"]["offset"] += 4
        result, comparison = self.classify()
        self.assertFalse(comparison["allocated_nontext_changed"])
        self.assertFalse(comparison["allocated_nontext_normalized_relocations_equal"])
        self.assertEqual(result["status"], "rejected")

    def test_exact_raw_function_is_not_eligible(self):
        self.base["functions"]["foo"]["raw_sha256"] = "target"
        self.assertEqual(self.classify()[0]["status"], "rejected")

    def test_changed_exact_sibling_is_not_eligible(self):
        siblings = set(self.candidate["functions"]) - {"foo"}
        self.assertTrue(siblings)
        self.candidate["functions"][sorted(siblings)[0]]["raw_sha256"] = "changed"
        self.assertEqual(self.classify()[0]["status"], "rejected")

    def test_nonfocus_is_not_eligible(self):
        self.assertEqual(self.classify(focus=[])[0]["status"], "rejected")

    def test_changed_nontext_is_not_eligible(self):
        for name, section in self.candidate["allocated_sections"].items():
            if not section["flags"] & inventory.SHF_EXECINSTR:
                section["sha256"] = "changed"
                break
        self.assertEqual(self.classify()[0]["status"], "rejected")


if __name__ == "__main__":
    unittest.main()
