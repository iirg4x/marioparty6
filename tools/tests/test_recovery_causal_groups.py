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
    def test_numeric_domains_fft_unsigned_and_single(self):
        left = [row("cmplwi r3, 2", 0), row("srwi r4, r3, 1", 4),
                row("fmuls f0, f1, f2", 8), row("lfs f2, 0(r2)", 12)]
        right = [row("cmpwi r3, 2", 0), row("srawi r4, r3, 1", 4),
                 row("fmul f0, f1, f2", 8), row("lfd f2, 0(r2)", 12)]
        evidence = groups.summarize_groups(report(left, right), "f")["numeric_domain_evidence"]
        signals = {s["family"]: s for s in evidence["signals"]}
        self.assertEqual(signals["integer"]["target_domain"], "unsigned")
        self.assertEqual(signals["integer"]["row_count"], 2)
        self.assertEqual(signals["arithmetic"]["target_domain"], "single")
        self.assertEqual(signals["load"]["review_priority"], "support_only")
        self.assertEqual(evidence["source_type_identity"], "UNKNOWN")

    def test_domain_reverse_and_mixed_directions(self):
        left = [row("cmpw r3, r4", 0), row("cmplw r3, r4", 4)]
        right = list(reversed(left))
        signals = groups.numeric_domain_evidence(left, right)["signals"]
        self.assertEqual({s["target_domain"] for s in signals}, {"signed", "unsigned"})
        self.assertTrue(all(s["mixed_direction"] and s["review_priority"] == "support_only" for s in signals))
        double = groups.numeric_domain_evidence([row("fdiv f0, f1, f2", 0)], [row("fdivs f0, f1, f2", 0)])
        self.assertEqual(double["signals"][0]["target_domain"], "double")

    def test_domain_opcode_families_and_malformed_instructions(self):
        for single, double in (("fmuls", "fmul"), ("fadds", "fadd"), ("fsubs", "fsub"), ("fdivs", "fdiv")):
            with self.subTest(op=single):
                result = groups.numeric_domain_evidence([row(single+" f0, f1, f2", 0)],
                                                       [row(double+" f0, f1, f2", 0)])
                self.assertEqual(result["signals"][0]["target_domain"], "single")
        for malformed in (None, {"instruction": None}, {"instruction": {"formatted": 3}},
                          {"instruction": "bad"}, row("cmplwi", 0)):
            self.assertEqual(groups.numeric_domain_evidence([malformed], [row("cmpwi r3, 1", 0)])["signals"], [])

    def test_domain_missing_unchanged_inserted_and_bounded(self):
        left = [row("cmplwi r3, 1", 0), {}, row("cmpwi r3, 1", 8),
                {**row("cmplwi r3, 1", 12), "diff_kind": "DIFF_DELETE"}]
        right = [{}, row("cmpwi r3, 1", 4), row("cmpwi r4, 1", 8), row("cmpwi r3, 1", 12)]
        self.assertEqual(groups.numeric_domain_evidence(left, right)["signals"], [])
        many = groups.numeric_domain_evidence([row("cmplwi r3, 1", i*4) for i in range(20)],
                                             [row("cmpwi r3, 1", i*4) for i in range(20)])["signals"][0]
        self.assertEqual(many["row_count"], 20)
        self.assertEqual(len(many["sites"]), 12)
        self.assertTrue(many["sites_truncated"])

    def test_owner_counts_and_no_physical_closure_claim(self):
        doc = report([row("cmplwi r3, 1", 0)], [row("cmpwi r3, 1", 0)])
        for side in ("left", "right"):
            exact = copy.deepcopy(doc[side]["symbols"][0])
            exact.update(name="already_exact", instructions=[row("blr", 0)])
            doc[side]["symbols"].append(exact)
        result = groups.summarize_owner(doc)
        self.assertEqual((result["instruction_exact_count"], result["remaining_count"]), (1, 1))
        self.assertTrue(result["residuals"][0]["domain_review_first"])
        self.assertFalse(result["owner_closed"])
        self.assertFalse(result["physical_proof"])

    def test_owner_missing_candidate_is_a_residual_not_a_crash_or_match(self):
        doc = report([row("blr", 0)], [row("blr", 0)])
        doc["right"]["symbols"] = []
        result = groups.summarize_owner(doc)
        self.assertEqual((result["instruction_exact_count"], result["remaining_count"]), (0, 1))
        self.assertEqual(result["residuals"][0]["status"], "candidate_symbol_missing")
        self.assertIsNone(result["residuals"][0]["score"])
        self.assertFalse(result["owner_closed"])

    def test_support_excerpt_is_bound_and_does_not_dump_full_stream(self):
        left = [row("cmplwi r3, 1", 0)] + [row("blr", i*4) for i in range(1, 100)]
        right = [row("cmpwi r3, 1", 0)] + copy.deepcopy(left[1:])
        right[90] = row("li r3, 0", 360)
        source = "float unrelated;\r\nvoid f(void) { }\r\nfloat other;\r\n"
        result = groups.support_packet(report(left, right), "f", source, 2, 2)
        self.assertEqual(result["source_excerpt"], "void f(void) { }")
        self.assertEqual(result["source_sha256"], hashlib.sha256(source.encode()).hexdigest())
        self.assertEqual(result["omitted_residual_row_count"], 1)
        self.assertLess(len(result["paired_rows"]), 10)
        self.assertNotIn("unrelated", json.dumps(result))
        with self.assertRaisesRegex(ValueError, "narrow"):
            groups.support_packet(report(left, right), "f", "a"*10000, 1, 1, max_bytes=1000)
        with self.assertRaises(ValueError):
            groups.support_packet(report(left, right), "f", source, 0, 1)

    def test_support_retains_late_extra_load_behind_early_register_cycle(self):
        left = [row("mr r4, r3", 0)] + [row("nop", i*4) for i in range(1, 120)]
        right = copy.deepcopy(left)
        right[0] = row("mr r5, r3", 0)
        left[90] = {}
        right[90] = row("lwz r4, 0x2c(r31)", 360)
        result = groups.support_packet(report(left, right), "f", "void f(void) {}", 1, 1)
        self.assertEqual(result["structural_context_anchors"], {"added_instruction": 90})
        self.assertTrue({0, 89, 90, 91}.issubset({r["row"] for r in result["paired_rows"]}))
        self.assertEqual(result["omitted_residual_row_count"], 0)
        self.assertFalse(result["size_exact"])
        self.assertLess(len(result["paired_rows"]), 20)

    def test_producer_field_order_and_downstream_use(self):
        left = [row("lwz r4, 0x28(r30)", 0), row("lwz r3, 0x2c(r30)", 4), row("add r6, r5, r3", 8)]
        right = [row("lwz r4, 0x2c(r30)", 0), row("lwz r3, 0x28(r30)", 4), row("add r4, r5, r4", 8)]
        result = groups.summarize_groups(report(left, right), "f", producers=8)["producer_slice"]
        self.assertEqual(result["target"]["nodes"][0]["memory_root"]["offset"], 40)
        self.assertEqual(result["candidate"]["nodes"][0]["memory_root"]["offset"], 44)
        self.assertEqual(result["target"]["nodes"][2]["uses"][1]["definition_row"], 1)
        self.assertEqual(result["candidate"]["nodes"][2]["uses"][1]["definition_row"], 0)

    def test_producer_boundaries(self):
        for text, reason in (("bl helper", "call boundary"), ("b 0x8", "CFG/function entry"), ("mystery r8", "unsupported opcode")):
            result = groups.producer_slice([row("li r3, 1", 0), row(text, 4), row("mr r4, r3", 8)], [2])
            self.assertEqual(result["nodes"][-1]["uses"][0]["reason"], reason)
            self.assertNotIn("definition_row", result["nodes"][-1]["uses"][0])

    def test_producer_branch_entry_alias_and_symbolic_memory(self):
        rows = [row("li r3, 1", 0), row("b 0xc", 4, branch_dest="12"), row("li r3, 2", 8),
                row("mr r4, r3", 12), row("lfs f0, label@sda21(r2)", 12)]
        result = groups.producer_slice(rows, [3, 4])
        self.assertEqual(result["nodes"][0]["uses"][0]["status"], "UNKNOWN")
        self.assertEqual(result["nodes"][1]["memory_root"]["status"], "UNKNOWN")

    def test_producer_bounds_and_optional_default(self):
        rows = [row("addi r3, r3, 1", i * 4) for i in range(100)]
        result = groups.producer_slice(rows, [99, 98], limit=1)
        self.assertEqual(len(result["nodes"]), 4)
        self.assertTrue(result["truncated"])
        self.assertNotIn("producer_slice", groups.summarize_groups(report(rows, rows), "f"))
        with self.assertRaises(ValueError):
            groups.producer_slice(rows, [1], limit=65)

    def test_producer_label_and_zero_base(self):
        rows = [row("li r3, 1", 0), {"label": "alias"}, row("mr r4, r3", 4),
                row("lwz r5, 0x28(r0)", 8), row("addi r6, r0, 4", 12)]
        nodes = groups.producer_slice(rows, [2, 3, 4])["nodes"]
        self.assertEqual(nodes[0]["uses"][0]["reason"], "unsupported/label boundary")
        self.assertEqual(nodes[1]["uses"], [])
        self.assertTrue(nodes[1]["memory_root"]["zero_base"])
        self.assertEqual(nodes[2]["uses"], [])

    def test_rotate_mask_aliases_preserve_unrelated_definitions(self):
        for alias in ("clrlwi r0, r3, 16", "clrlslwi r0, r3, 16, 2"):
            rows = [row("li r5, 1", 0), row("li r6, 2", 4), row("li r3, 3", 8),
                    row(alias, 12), row("add r7, r5, r6", 16)]
            nodes = groups.producer_slice(rows, [3, 4])["nodes"]
            self.assertEqual(nodes[3]["defines"], "r0")
            self.assertEqual(nodes[3]["uses"], [{"register": "r3", "definition_row": 2}])
            self.assertEqual(nodes[4]["uses"], [{"register": "r5", "definition_row": 0},
                                                {"register": "r6", "definition_row": 1}])

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
