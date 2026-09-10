from __future__ import annotations

import copy
import hashlib
import json
import os
from pathlib import Path
import unittest

from tools import recovery_causal_groups as groups


def row(text, address, **metadata):
    return {"instruction": {"formatted": text, "address": str(address), "size": 4, **metadata}}


def report(left, right):
    return {side: {"symbols": [{"name": "f", "kind": "SYMBOL_FUNCTION", "size": str(4 * sum(bool(r.get('instruction')) for r in rows)),
                                "instructions": rows}]}
            for side, rows in (("left", left), ("right", right))}


class CausalGroupsTests(unittest.TestCase):
    def test_identical(self):
        rows = [row("li r3, 0", 0), row("blr", 4)]
        result = groups.summarize_groups(report(rows, copy.deepcopy(rows)), "f")
        self.assertTrue(result["instruction_exact"])
        self.assertEqual(result["groups"], [])
        self.assertIsNone(result["first_machine_divergence"])
        self.assertFalse(result["physical_proof"])

    def test_global_permutation(self):
        result = groups.summarize_groups(report([row("add r3, r4, r3", 0)], [row("add r4, r3, r4", 0)]), "f")
        self.assertEqual(result["register_mapping"]["status"], "confirmed")
        self.assertEqual(result["category_rows"], {"register_permutation": 1})

    def test_conflict_stays_unknown(self):
        result = groups.summarize_groups(report([row("mr r3, r4", 0), row("mr r3, r4", 4)],
                                                [row("mr r4, r3", 0), row("mr r5, r3", 4)]), "f")
        self.assertEqual(result["register_mapping"]["mapping"], {})
        self.assertEqual(result["unresolved_row_count"], 2)
        self.assertTrue(all(not g["cause_proven"] for g in result["groups"]))

    def test_added_deleted_moves(self):
        result = groups.summarize_groups(report([row("mr r3, r4", 0), {}], [{}, row("mr r4, r3", 0)]), "f")
        self.assertEqual(result["category_rows"], {"added_move": 1, "deleted_move": 1})
        self.assertEqual(result["coverage"]["grouped_rows"], 2)

    def test_cfg_and_opcode(self):
        left = [row("b 0x4", 0, branch_dest="4"), row("li r3, 1", 4), row("li r3, 2", 8)]
        right = [row("b 0x8", 0, branch_dest="8"), row("li r3, 1", 4), row("addi r3, r3, 2", 8)]
        result = groups.summarize_groups(report(left, right), "f")
        self.assertEqual(result["category_rows"], {"cfg_changed": 1, "opcode": 1})
        self.assertEqual(result["structural_hazard_count"], 2)

    def test_relocation_addend(self):
        left = [row("lis r3, f@ha", 0, relocation={"type_name": "R_PPC_ADDR16_HA", "target_symbol": 0, "addend": 0})]
        right = copy.deepcopy(left)
        right[0]["instruction"]["relocation"]["addend"] = 4
        result = groups.summarize_groups(report(left, right), "f")
        self.assertEqual(result["category_rows"], {"relocation": 1})
        self.assertFalse(result["instruction_exact"])

    def test_ranking_protects_exact_before_score(self):
        result = groups.summarize_groups(report([row("blr", 0)], [row("blr", 0)]), "f")
        self.assertLess(groups.ranking_tuple(result, score=0), groups.ranking_tuple(result, protected_loss_count=1, score=100))
        self.assertLess(groups.ranking_tuple(result, exact_function_count=2, score=0), groups.ranking_tuple(result, exact_function_count=1, score=100))

    def summarize_pair(self, candidate):
        target = [row("mr r3, r4", 0), row("mr r3, r4", 4)]
        return groups.summarize_groups(report(target, candidate), "f")

    def test_changed_register_relation_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r0, r4", 0), row("mr r0, r4", 4)])
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])

    def test_changed_category_is_reclassified_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("addi r3, r4, 1", 0), row("addi r3, r4, 1", 4)])
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])
        self.assertTrue(delta["reclassified_groups"])

    def test_split_relation_bucket_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r0, r4", 0), row("mr r6, r4", 4)])
        self.assertGreater(len(after["groups"]), len(before["groups"]))
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])
        self.assertTrue(delta["reclassified_groups"])

    def test_partially_resolved_group_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r6, r4", 4)])
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [])

    def test_all_prior_sites_resolved_closes_group(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [before["groups"][0]["id"]])

    def test_insertion_alignment_shift_does_not_close_target_sites(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = groups.summarize_groups(report(
            [{}, row("mr r3, r4", 0), row("mr r3, r4", 4)],
            [row("nop", 0), row("mr r6, r4", 4), row("mr r6, r4", 8)]), "f")
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [])
        self.assertEqual(before["target_binding"], after["target_binding"])
        self.assertEqual({m["target_index"] for g in after["groups"]
                          for m in g["members"] if m["anchor"] == "target"}, {0, 1})

    def test_different_target_stream_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        target = [row("mr r7, r4", 0), row("mr r7, r4", 4)]
        after = groups.summarize_groups(report(target, copy.deepcopy(target)), "f")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_legacy_missing_members_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        before["groups"][0].pop("members")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_legacy_missing_target_binding_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        before.pop("target_binding")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_actual_fixture_when_explicitly_selected(self):
        # Retail-derived local reports are never read by default public checks.
        directory = os.environ.get("MP6_CAUSAL_GROUP_FIXTURE_ROOT")
        if not directory:
            self.skipTest("explicit local fixture directory not selected")
        root = Path(directory)
        raw = (root / "kettou-guide-field-reconstruction/.evaluate-jpuewz_e/strict.json").read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(), "5de00fed59959dbbea921bb07b67922231a81b969e2e177e2db1773bd8d258c3")
        after = groups.summarize_groups(json.loads(raw), "ev_CapKettouStart")
        before = groups.summarize_groups(json.loads((root / "evaluations/.evaluate-uzle7on4/strict.json").read_bytes()), "ev_CapKettouStart")
        self.assertEqual(before["coverage"]["residual_rows"], 314)
        self.assertEqual(after["coverage"]["residual_rows"], 291)
        self.assertEqual(after["category_rows"], {"added_move": 13, "deleted_move": 13, "register_relation_unknown": 263, "abi_helper_call": 2})
        delta = groups.compare_groups(before, after)
        self.assertEqual(len(delta["disappeared_observation_buckets"]), 5)
        self.assertEqual(len(delta["closed_groups"]), 2)
        self.assertEqual(len(delta["reclassified_groups"]), 3)
        closed = [g for g in before["groups"] if g["id"] in delta["closed_groups"]]
        self.assertTrue(all("r24" in str(g["relation"]) for g in closed))
        residual_sites = {member["target_index"] for group in after["groups"] for member in group["members"]}
        self.assertEqual(sum(group["row_count"] for group in closed), 23)
        self.assertTrue(all(not {member["target_index"] for member in group["members"]} & residual_sites
                            for group in closed))
        reclassified = [group for group in before["groups"] if group["id"] in delta["reclassified_groups"]]
        self.assertTrue(all({member["target_index"] for member in group["members"]} & residual_sites
                            for group in reclassified))
        self.assertEqual(delta["new_groups"], [])
        self.assertEqual(after["register_mapping"]["mapping"], {})


if __name__ == "__main__":
    unittest.main()
