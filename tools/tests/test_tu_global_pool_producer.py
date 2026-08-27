from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import tu_global_pool_producer as global_pool


def _instruction(address: int, formatted: str, *, mismatch: bool, relocation: bool) -> dict[str, object]:
    instruction: dict[str, object] = {"address": str(address), "size": 4, "formatted": formatted}
    if relocation:
        instruction["relocation"] = {"type": 109, "type_name": "R_PPC_EMB_SDA21", "target_symbol": 1}
    row: dict[str, object] = {"instruction": instruction}
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _function(name: str, size: int, relocations: int, stage: str, *, target: bool) -> dict[str, object]:
    count = size // 4
    text = ["nop"] * count
    mismatch_row = 8 if name == "mbev_CapMiracle" else None
    if stage == "control" and mismatch_row is not None:
        text[mismatch_row] = "lfs f0, lbl_802C4370@sda21" if target else "lfs f0, @1734@sda21"
    return {
        "name": name,
        "kind": "SYMBOL_FUNCTION",
        "address": "0x100",
        "size": str(size),
        "match_percent": 99.98408 if stage == "control" and name == "mbev_CapMiracle" else 100.0,
        "instructions": [
            _instruction(0x100 + index * 4, formatted, mismatch=stage == "control" and index == mismatch_row, relocation=index < relocations)
            for index, formatted in enumerate(text)
        ],
    }


def _report(stage: str) -> dict[str, object]:
    left = [{"name": "pool", "kind": "SYMBOL_DATA"}]
    right = [{"name": "pool", "kind": "SYMBOL_DATA"}]
    for name, (size, relocations) in global_pool.REPORT_CONTRACTS.items():
        left.append(_function(name, size, relocations, stage, target=True))
        right.append(_function(name, size, relocations, stage, target=False))
    return {"left": {"symbols": left}, "right": {"symbols": right}}


def _context(reports: dict[str, dict[str, object]]) -> dict[str, object]:
    return {
        "schema": rules.TU_GLOBAL_POOL_PRODUCER_CONTEXT_SCHEMA,
        "owner": "main:board/capspecial",
        "functions": list(global_pool.FUNCTIONS),
        "source_owner_task": "96749815b59f484cb763893457d10591",
        "authority_advanced": False,
        "reports": [
            {"function": "mbev_CapMiracle", "report_sha256": "2b" * 32, "target_size": 1256, "candidate_size": 1256, "physical_relocations": 96, "strict_exact": True, "data_exact": True},
            {"function": "ev_CapMiracleMasu", "report_sha256": "86" * 32, "target_size": 6136, "candidate_size": 6136, "physical_relocations": 397, "strict_exact": True, "data_exact": True},
        ],
        "toolchain": {"base_commit": "ba0ae784f1062b836a0bd64ab67a41afd6091a01", "compiler_sha256": "31" * 32, "wrapper_sha256": "27" * 32, "target_object_sha256": "a1" * 32},
        "provenance": {"graphify_status": "no_usable_graph", "graphify_bound": False, "graft_ask_count": 1, "graft_status": "no_nodes", "narrow_named_file_verified": True, "broad_searches": 0},
        "producer": {"source_name": "capspecialTen", "c_type": "const float", "value": 10.0, "target_symbol": "lbl_802C4370", "target_global": True, "source_linkage": "external", "source_line": 1511, "before_function": "mbev_CapMiracle"},
        "consumers": [{"function": function, "source_line": line, "role": role} for function, line, role in global_pool.CONSUMER_CENSUS],
        "static_control": {
            "candidate_id": "c691-shared-tu-ten-owner", "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["control"])),
            "source_sha256": "32" * 32, "object_sha256": "7c" * 32, "baseline_object_sha256": "7c" * 32,
            "object_identical_to_baseline": True, "consumer_count": 7, "linkage": "internal", "strict_match_percent": 99.98408,
            "strict_diff_rows": 1, "data_exact": True, "outcome": "rejected_object_neutral", "compile_attestation_sha256": "29" * 32,
            "candidate_record_sha256": "00" * 32,
        },
        "exact_result": {
            "candidate_id": "c692-global-tu-ten-owner", "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["exact"])),
            "source_sha256": "c5" * 32, "object_sha256": "12" * 32, "strict_report_sha256": "ae" * 32,
            "data_report_sha256": "3b" * 32, "compile_attestation_sha256": "e7" * 32, "candidate_record_sha256": "6e" * 32,
            "strict_data_exact": True, "protected_siblings": "31/31",
        },
        "telemetry": {
            "parent_active_seconds": 893.2932908, "parent_heavy_seconds": 0.2495924, "wait_seconds": 0.0,
            "telemetry_complete": True, "current_campaign_eligible": True, "historical_denominator_allowed": False,
            "no_historical_imputation": True, "historical_gap_seconds": [4708.980946, 1359.7626529, 514.721192],
            "telemetry_receipt_sha256": "76" * 32, "paired_telemetry_receipt_sha256": "19" * 32,
            "active_interval_log_sha256": "4e" * 32,
        },
        "forbidden_axes": list(global_pool.FORBIDDEN_AXES),
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(item for item in result["evaluations"] if item["rule_id"] == global_pool.RULE_ID)


class TuGlobalPoolProducerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reports = {stage: _report(stage) for stage in ("control", "exact")}

    def test_local_static_control_ranks_only_global_producer(self) -> None:
        result = rules.diagnose_document(self.reports["control"], focus_symbol="mbev_CapMiracle", tu_global_pool_producer_context=_context(self.reports))
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(evidence["recommended_cells"][0]["kind"], "restore_authenticated_global_tu_pool_producer")
        self.assertEqual(len(evidence["recommended_cells"][0]["consumers"]), 7)
        self.assertTrue(evidence["suppress_downstream_body_edits"])
        self.assertTrue(evidence["suppress_tracer"])
        self.assertTrue(evidence["telemetry"]["telemetry_complete"])
        self.assertFalse(evidence["telemetry"]["historical_denominator_allowed"])

    def test_exact_pair_schedules_nothing(self) -> None:
        context = _context(self.reports)
        for function in global_pool.FUNCTIONS:
            evaluation = _evaluation(rules.diagnose_document(self.reports["exact"], focus_symbol=function, tu_global_pool_producer_context=context))
            self.assertFalse(evaluation["matched"])
            self.assertIn("already exact", evaluation["reason"])

    def test_non_owner_residual_fails_closed(self) -> None:
        report = copy.deepcopy(self.reports["control"])
        report["right"]["symbols"][1]["instructions"][8]["instruction"]["formatted"] = "fmr f0, f1"
        context = _context(self.reports)
        context["static_control"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
        evaluation = _evaluation(rules.diagnose_document(report, focus_symbol="mbev_CapMiracle", tu_global_pool_producer_context=context))
        self.assertFalse(evaluation["matched"])

    def test_producer_consumer_control_and_telemetry_drift_fail_closed(self) -> None:
        mutations = [
            lambda context: context["producer"].__setitem__("source_linkage", "internal"),
            lambda context: context["consumers"].pop(),
            lambda context: context["static_control"].__setitem__("object_identical_to_baseline", False),
            lambda context: context["provenance"].__setitem__("graft_ask_count", 2),
            lambda context: context["telemetry"].__setitem__("historical_denominator_allowed", True),
        ]
        for mutate in mutations:
            context = _context(self.reports)
            mutate(context)
            with self.assertRaises(rules.LearningInputError):
                rules.diagnose_document(self.reports["control"], focus_symbol="mbev_CapMiracle", tu_global_pool_producer_context=context)

    def test_output_binds_context_and_implementation(self) -> None:
        result = rules.diagnose_document(self.reports["control"], focus_symbol="mbev_CapMiracle", tu_global_pool_producer_context=_context(self.reports))
        self.assertIsNotNone(result["inputs"]["tu_global_pool_producer_context_canonical_sha256"])
        implementation = result["implementations"]["tu_global_pool_producer"]
        self.assertEqual(implementation["schema"], global_pool.CONTEXT_SCHEMA)

    def test_cli_accepts_authenticated_context(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(self.reports["control"]), encoding="utf-8")
            context_path.write_text(json.dumps(_context(self.reports)), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(["--report", str(report_path), "--function", "mbev_CapMiracle", "--tu-global-pool-producer-context", str(context_path)])
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
