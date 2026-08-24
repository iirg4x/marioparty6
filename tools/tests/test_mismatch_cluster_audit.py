import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools.mismatch_cluster_audit import DEFAULT_MAX_HYPOTHESES, AuditInputError, audit_document, main


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tools" / "mismatch_cluster_audit.py"


def instruction(address, text, *, diff_kind=None, relocation=None, branch_dest=None):
    item = {"instruction": {"address": str(address), "size": 4, "formatted": text}}
    if diff_kind is not None:
        item["diff_kind"] = diff_kind
    if relocation is not None:
        item["instruction"]["relocation"] = relocation
    if branch_dest is not None:
        item["instruction"]["branch_dest"] = str(branch_dest)
    return item


def symbol(name, address, entries, *, size=None, match_percent=None):
    result = {"name": name, "address": str(address), "kind": "SYMBOL_FUNCTION", "instructions": entries}
    if size is not None:
        result["size"] = str(size)
    if match_percent is not None:
        result["match_percent"] = match_percent
    return result


def report(target_symbols, candidate_symbols, *, target_sections=None, candidate_sections=None):
    return {
        "left": {"symbols": target_symbols, "sections": target_sections or []},
        "right": {"symbols": candidate_symbols, "sections": candidate_sections or []},
    }


class MismatchClusterAuditTests(unittest.TestCase):
    def test_uniform_stack_delta_is_clustered_and_ranked(self):
        target = [
            instruction(100, "stw r3, 0x44(r1)", diff_kind="DIFF_ARG_MISMATCH"),
            instruction(104, "lwz r4, 0x48(r1)", diff_kind="DIFF_ARG_MISMATCH"),
            instruction(108, "stw r5, 0x4c(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        ]
        candidate = [
            instruction(500, "stw r3, 0x30(r1)", diff_kind="DIFF_ARG_MISMATCH"),
            instruction(504, "lwz r4, 0x34(r1)", diff_kind="DIFF_ARG_MISMATCH"),
            instruction(508, "stw r5, 0x38(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        ]
        result = audit_document(report([symbol("foo", 100, target)], [symbol("foo", 500, candidate)]))
        self.assertEqual(result["status"], "ok")
        self.assertEqual(len(result["hypotheses"]), 1)
        hypothesis = result["hypotheses"][0]
        self.assertEqual(hypothesis["classification"], "stack_home_uniform_delta")
        self.assertEqual(hypothesis["evidence"]["stack_deltas"], [20, 20, 20])
        self.assertEqual(hypothesis["rank"], 1)

    def test_sign_extension_insertion_is_not_a_semantic_claim(self):
        target = [
            instruction(100, "lbz r0, 0x10(r3)", diff_kind="DIFF_ARG_MISMATCH"),
            {"diff_kind": "DIFF_INSERT"},
            instruction(108, "bl use_value"),
        ]
        candidate = [
            instruction(500, "lbz r0, 0x10(r3)", diff_kind="DIFF_ARG_MISMATCH"),
            instruction(504, "extsb r0, r0", diff_kind="DIFF_INSERT"),
            instruction(508, "bl use_value"),
        ]
        result = audit_document(report([symbol("signed_boundary", 100, target)], [symbol("signed_boundary", 500, candidate)]))
        hypothesis = result["hypotheses"][0]
        self.assertEqual(hypothesis["classification"], "missing_sign_extension_or_prototype")
        self.assertIn("Semantic variable names", " ".join(hypothesis["limitations"]))
        self.assertNotIn("signed_boundary", hypothesis["evidence"].get("semantic_variable", ""))

    def test_branch_shape_and_assembly_context(self):
        target = [instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128)]
        candidate = [instruction(500, "beq 0x514", diff_kind="DIFF_ARG_MISMATCH", branch_dest=130)]
        with tempfile.TemporaryDirectory() as directory:
            target_asm = Path(directory) / "target.s"
            candidate_asm = Path(directory) / "candidate.s"
            target_asm.write_text(".fn foo, local\n/* 00000100 00000000 */\tbeq .L1\n.endfn foo\n", encoding="utf-8")
            candidate_asm.write_text(".fn foo, local\n/* 00000200 00000000 */\tbne .L2\n.endfn foo\n", encoding="utf-8")
            result = audit_document(
                report([symbol("foo", 100, target)], [symbol("foo", 500, candidate)]),
                target_assembly=target_asm,
                candidate_assembly=candidate_asm,
            )
        hypothesis = result["hypotheses"][0]
        self.assertEqual(hypothesis["classification"], "branch_shape")
        self.assertTrue(hypothesis["evidence"]["target_assembly"]["function_label_found"])
        self.assertTrue(hypothesis["evidence"]["candidate_assembly"]["function_label_found"])

    def test_explicit_else_return_epilogue_is_ranked_and_reduced(self):
        target = [
            instruction(100, "cmpwi r3, 1"),
            instruction(104, "bne 0x74", diff_kind="DIFF_ARG_MISMATCH", branch_dest=116),
            instruction(108, "bl body"),
            instruction(112, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            instruction(116, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            instruction(120, "blr"),
        ]
        candidate = [
            instruction(500, "cmpwi r3, 1"),
            instruction(504, "bne 0x214", diff_kind="DIFF_ARG_MISMATCH", branch_dest=516),
            instruction(508, "bl body"),
            {"diff_kind": "DIFF_DELETE"},
            {"diff_kind": "DIFF_DELETE"},
            instruction(516, "blr"),
        ]
        result = audit_document(report([symbol("hook", 100, target)], [symbol("hook", 500, candidate)]))
        patterns = result["functions"][0]["patterns"]
        self.assertEqual(len(patterns), 1)
        self.assertEqual(patterns[0]["classification"], "explicit_else_return_epilogue")
        self.assertIn("else-return", patterns[0]["recommended_source_axis"])
        self.assertEqual(patterns[0]["evidence"]["target_exit_branch_indices"], [3, 4])
        groups = result["functions"][0]["causal_groups"]
        explicit = next(item for item in groups if item["classification"] == "explicit_else_return_epilogue")
        self.assertEqual(explicit["root_index"], 1)
        self.assertEqual(explicit["diff_pair_count"], 3)

    def test_aggregate_copy_lifetime_and_section_signal_are_separate(self):
        target = [
            instruction(100, "stfd f1, 0x20(r1)", diff_kind="DIFF_REPLACE"),
            instruction(104, "lfd f2, 0x20(r1)", diff_kind="DIFF_REPLACE"),
        ]
        candidate = [
            instruction(500, "stfs f1, 0x20(r1)", diff_kind="DIFF_REPLACE"),
            instruction(504, "lfs f2, 0x20(r1)", diff_kind="DIFF_REPLACE"),
        ]
        sections = [{"name": ".sdata2", "match_percent": 80.0, "data_diff": [{"size": "4"}]}]
        result = audit_document(
            report([symbol("copy", 100, target)], [symbol("copy", 500, candidate)], target_sections=sections, candidate_sections=sections)
        )
        self.assertEqual(result["functions"][0]["clusters"][0]["classification"], "aggregate_copy_or_lifetime")
        self.assertEqual(len(result["section_hypotheses"]), 1)
        self.assertEqual(result["section_hypotheses"][0]["section"], ".sdata2")

    def test_unknown_is_explicit_for_unclassified_register_mismatch(self):
        target = [instruction(100, "mr r3, r4", diff_kind="DIFF_ARG_MISMATCH")]
        candidate = [instruction(500, "mr r5, r6", diff_kind="DIFF_ARG_MISMATCH")]
        result = audit_document(report([symbol("registers", 100, target)], [symbol("registers", 500, candidate)]))
        self.assertEqual(result["hypotheses"][0]["classification"], "unknown")
        self.assertLessEqual(result["hypotheses"][0]["confidence"], 0.2)

    def test_unmatched_functions_are_fail_safe_unknown(self):
        result = audit_document(report([symbol("only_target", 100, [instruction(100, "blr")])], []))
        self.assertEqual(result["status"], "ok")
        self.assertEqual(result["hypotheses"], [])
        self.assertEqual(result["unmatched_functions"][0]["classification"], "unknown")

    def test_unmatched_malformed_instruction_is_not_silently_skipped(self):
        malformed = symbol("only_target", 100, [{"instruction": 7}])
        with self.assertRaises(AuditInputError) as raised:
            audit_document(report([malformed], []))
        self.assertEqual(raised.exception.code, "invalid_instruction")

    def test_malformed_input_returns_fail_closed_json_and_exit_code(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "bad.json"
            path.write_text("{not json", encoding="utf-8")
            completed = subprocess.run(
                [sys.executable, str(SCRIPT), str(path)],
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(completed.returncode, 2)
        output = json.loads(completed.stdout)
        self.assertTrue(output["fail_closed"])
        self.assertEqual(output["error"]["code"], "invalid_json")

    def test_help_is_available(self):
        completed = subprocess.run([sys.executable, str(SCRIPT), "--help"], check=False, capture_output=True, text=True)
        self.assertEqual(completed.returncode, 0)
        self.assertIn("objdiff", completed.stdout)

    def test_rank_order_is_stable(self):
        target = [instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128)]
        candidate = [instruction(500, "bne 0x514", diff_kind="DIFF_ARG_MISMATCH", branch_dest=130)]
        document = report([symbol("zeta", 100, target), symbol("alpha", 200, target)], [symbol("zeta", 500, candidate), symbol("alpha", 600, candidate)])
        first = audit_document(document)
        second = audit_document(document)
        self.assertEqual(json.dumps(first, sort_keys=True), json.dumps(second, sort_keys=True))
        self.assertEqual([item["function"] for item in first["functions"]], ["alpha", "zeta"])

    def test_focus_symbol_selects_one_pair_and_preserves_sections_and_schema_order(self):
        target = [instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128)]
        candidate = [instruction(500, "bne 0x514", diff_kind="DIFF_ARG_MISMATCH", branch_dest=130)]
        sections = [{"name": ".sdata2", "match_percent": 80.0, "data_diff": [{"size": "4"}]}]
        result = audit_document(
            report(
                [symbol("alpha", 100, target), symbol("focus", 200, target)],
                [symbol("alpha", 500, candidate), symbol("focus", 600, candidate)],
                target_sections=sections,
                candidate_sections=sections,
            ),
            focus_symbol="focus",
        )
        self.assertEqual([item["function"] for item in result["functions"]], ["focus"])
        self.assertEqual(len(result["section_hypotheses"]), 1)
        self.assertEqual(result["section_hypotheses"][0]["section"], ".sdata2")
        self.assertEqual(
            list(result),
            [
                "schema_version",
                "status",
                "fail_closed",
                "functions",
                "hypotheses",
                "causal_groups",
                "section_hypotheses",
                "unmatched_functions",
                "limitations",
            ],
        )

    def test_focus_symbol_not_found_ambiguous_and_one_sided_fail_closed(self):
        entries = [instruction(100, "blr")]
        cases = (
            (
                "not_found",
                report([symbol("present", 100, entries)], [symbol("present", 500, entries)]),
                "missing",
                "focus_not_found",
            ),
            (
                "ambiguous",
                report(
                    [symbol("duplicate", 100, entries), symbol("duplicate", 200, entries)],
                    [symbol("duplicate", 500, entries), symbol("duplicate", 600, entries)],
                ),
                "duplicate",
                "focus_ambiguous",
            ),
            (
                "one_sided",
                report([symbol("target_only", 100, entries)], []),
                "target_only",
                "focus_one_sided",
            ),
        )
        for label, document, focus, code in cases:
            with self.subTest(label=label):
                with self.assertRaises(AuditInputError) as raised:
                    audit_document(document, focus_symbol=focus)
                self.assertEqual(raised.exception.code, code)

    def test_exact_100_percent_pairs_skip_residuals_by_default(self):
        target = [instruction(100, "mr r3, r4", diff_kind="DIFF_ARG_MISMATCH")]
        candidate = [instruction(500, "mr r5, r6", diff_kind="DIFF_ARG_MISMATCH")]
        document = report(
            [symbol("exact", 100, target, match_percent=100.0)],
            [symbol("exact", 500, candidate, match_percent="100.0")],
            target_sections=[{"name": ".sdata2", "data_diff": [{"size": "4"}]}],
            candidate_sections=[{"name": ".sdata2", "data_diff": [{"size": "4"}]}],
        )
        result = audit_document(document)
        self.assertEqual(result["functions"][0]["residual_cluster_count"], 0)
        self.assertEqual(result["functions"][0]["clusters"], [])
        self.assertEqual(len(result["section_hypotheses"]), 1)
        self.assertEqual(result["hypotheses"], result["section_hypotheses"])

        compatibility = audit_document(document, include_exact_residuals=True)
        self.assertEqual(compatibility["functions"][0]["residual_cluster_count"], 1)
        self.assertEqual(len(compatibility["hypotheses"]), 2)
        self.assertIn("exact", [item["function"] for item in compatibility["hypotheses"]])

    def test_focus_fail_closed_is_reported_by_cli(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(
                json.dumps(report([symbol("present", 100, [instruction(100, "blr")])], [symbol("present", 500, [instruction(500, "blr")])])),
                encoding="utf-8",
            )
            completed = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), "--focus-symbol", "missing"],
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(completed.returncode, 2)
        output = json.loads(completed.stdout)
        self.assertTrue(output["fail_closed"])
        self.assertEqual(output["error"]["code"], "focus_not_found")

    def test_summary_mode_bounds_ranked_clusters_and_omits_instruction_pairs(self):
        target = [
            instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128),
            instruction(104, "blr"),
            instruction(108, "beq 0x128", diff_kind="DIFF_ARG_MISMATCH", branch_dest=136),
        ]
        candidate = [
            instruction(500, "bne 0x520", diff_kind="DIFF_ARG_MISMATCH", branch_dest=520),
            instruction(504, "blr"),
            instruction(508, "bne 0x528", diff_kind="DIFF_ARG_MISMATCH", branch_dest=528),
        ]
        document = report([symbol("bounded", 100, target)], [symbol("bounded", 500, candidate)])
        full = audit_document(document)
        summary = audit_document(document, summary_only=True, max_hypotheses=1)

        self.assertEqual(summary["status"], "ok")
        self.assertTrue(summary["summary_only"])
        self.assertEqual(summary["max_hypotheses"], 1)
        self.assertEqual(summary["hypothesis_count"], 2)
        self.assertEqual(summary["returned_hypothesis_count"], 1)
        self.assertEqual(len(summary["hypotheses"]), 1)
        self.assertEqual(summary["classification_counts"], {"branch_shape": 2})
        function = summary["functions"][0]
        self.assertEqual(function["residual_cluster_count"], 2)
        self.assertEqual(function["classification_counts"], {"branch_shape": 2})
        self.assertEqual(function["retained_cluster_count"], 1)
        self.assertEqual(len(function["clusters"]), 1)
        self.assertNotIn("instruction_pairs", summary["hypotheses"][0])
        self.assertNotIn("instruction_pairs_truncated", summary["hypotheses"][0])
        self.assertEqual(summary["hypotheses"][0]["rank"], 1)
        self.assertGreater(len(json.dumps(full)), len(json.dumps(summary)))

    def test_default_full_output_remains_backward_compatible(self):
        target = [instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128)]
        candidate = [instruction(500, "bne 0x520", diff_kind="DIFF_ARG_MISMATCH", branch_dest=520)]
        document = report([symbol("full", 100, target)], [symbol("full", 500, candidate)])
        implicit = audit_document(document)
        explicit = audit_document(document, summary_only=False, max_hypotheses=DEFAULT_MAX_HYPOTHESES)
        self.assertEqual(implicit, explicit)
        self.assertNotIn("summary_only", implicit)

    def test_summary_preserves_all_section_evidence(self):
        target = [instruction(100, "beq 0x120", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128)]
        candidate = [instruction(500, "bne 0x520", diff_kind="DIFF_ARG_MISMATCH", branch_dest=520)]
        sections = [
            {"name": ".sdata", "match_percent": 80.0, "data_diff": [{"size": "4"}]},
            {"name": ".sdata2", "match_percent": 75.0, "reloc_diff": [{"type": "R"}]},
        ]
        summary = audit_document(
            report(
                [symbol("with_sections", 100, target)],
                [symbol("with_sections", 500, candidate)],
                target_sections=sections,
                candidate_sections=sections,
            ),
            summary_only=True,
            max_hypotheses=1,
        )
        self.assertEqual([item["section"] for item in summary["section_hypotheses"]], [".sdata", ".sdata2"])
        self.assertEqual(len(summary["section_hypotheses"]), 2)
        self.assertEqual(summary["hypothesis_count"], 3)
        self.assertEqual(summary["classification_counts"]["relocation_or_data_mismatch"], 2)

    def test_summary_limits_fail_closed_for_api(self):
        document = report([symbol("foo", 100, [instruction(100, "blr")])], [symbol("foo", 500, [instruction(500, "blr")])])
        for invalid in (0, -1, 1001, True, "2"):
            with self.subTest(invalid=invalid):
                with self.assertRaises(AuditInputError) as raised:
                    audit_document(document, summary_only=True, max_hypotheses=invalid)
                self.assertEqual(raised.exception.code, "invalid_max_hypotheses")
        with self.assertRaises(AuditInputError) as raised:
            audit_document(document, summary_only="yes")
        self.assertEqual(raised.exception.code, "invalid_summary_only")

    def test_summary_cli_and_invalid_limit_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(
                json.dumps(report([symbol("foo", 100, [instruction(100, "blr")])], [symbol("foo", 500, [instruction(500, "blr")])])),
                encoding="utf-8",
            )
            compact = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), "--summary-only", "--max-hypotheses", "1"],
                check=False,
                capture_output=True,
                text=True,
            )
            invalid = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), "--summary-only", "--max-hypotheses", "not-an-int"],
                check=False,
                capture_output=True,
                text=True,
            )
            zero = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), "--summary-only", "--max-hypotheses", "0"],
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(compact.returncode, 0)
        compact_output = json.loads(compact.stdout)
        self.assertTrue(compact_output["summary_only"])
        self.assertEqual(compact_output["max_hypotheses"], 1)
        self.assertEqual(invalid.returncode, 2)
        invalid_output = json.loads(invalid.stdout)
        self.assertTrue(invalid_output["fail_closed"])
        self.assertEqual(invalid_output["error"]["code"], "invalid_max_hypotheses")
        self.assertEqual(zero.returncode, 2)
        zero_output = json.loads(zero.stdout)
        self.assertTrue(zero_output["fail_closed"])
        self.assertEqual(zero_output["error"]["code"], "invalid_max_hypotheses")


if __name__ == "__main__":
    unittest.main()

