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


def _boundary_report(stage: str) -> dict[str, object]:
    size, relocations = global_pool.BOUNDARY_REPORT_CONTRACT
    count = size // 4
    target_text = ["nop"] * count
    candidate_text = ["nop"] * count
    if stage == "boundary-control":
        for row, target_owner, candidate_owner in global_pool.BOUNDARY_RESIDUAL_ROWS:
            target_text[row] = f"lfs f0, {target_owner}@sda21"
            candidate_text[row] = f"lfs f0, {candidate_owner}@sda21"
    mismatch_rows = {row for row, _, _ in global_pool.BOUNDARY_RESIDUAL_ROWS}

    def symbol(text: list[str], *, target: bool) -> dict[str, object]:
        return {
            "name": global_pool.BOUNDARY_FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "address": "0x100",
            "size": str(size),
            "match_percent": 99.97872 if stage == "boundary-control" else 100.0,
            "instructions": [
                _instruction(
                    0x100 + index * 4,
                    formatted,
                    mismatch=stage == "boundary-control" and index in mismatch_rows,
                    relocation=index < relocations,
                )
                for index, formatted in enumerate(text)
            ],
        }

    return {
        "left": {"symbols": [symbol(target_text, target=True)]},
        "right": {"symbols": [symbol(candidate_text, target=False)]},
    }


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


def _boundary_context(reports: dict[str, dict[str, object]]) -> dict[str, object]:
    controls = []
    for index, kind in enumerate(global_pool.BOUNDARY_CONTROL_KINDS):
        controls.append(
            {
                "kind": kind,
                "candidate_id": f"c69{index + 3}-{kind}",
                "source_sha256": f"{index + 1:02x}" * 32,
                "object_sha256": f"{index + 11:02x}" * 32,
                "regressed_exact_functions": 6 if index < 3 else 0,
                "focus_rows_unchanged": index == 3,
                "outcome": "rejected",
            }
        )
    return {
        "schema": rules.TU_GLOBAL_POOL_PRODUCER_BOUNDARY_CONTEXT_SCHEMA,
        "owner": "main:board/capspecial",
        "function": global_pool.BOUNDARY_FUNCTION,
        "source_owner_task": "01a0310ea0dd7a02b41890e8174ded3f",
        "source_report_sha256": "e3b8dffc48bdd34b4c06c69a98af54cecb78ff9c3cb69c21bb32bbb074fec864",
        "authority_advanced": False,
        "report": {
            "target_size": 4700,
            "candidate_size": 4700,
            "physical_relocation_annotations": 384,
            "strict_exact_after": True,
            "data_exact_after": True,
            "focus_residual_rows_before": 5,
            "protected_siblings": "31/31",
            "owner_strict_exact_after": 37,
            "owner_function_total": 44,
            "full_owner_closed": False,
            "later_production_drift_excluded": True,
        },
        "toolchain": {
            "base_commit": "ba0ae784f1062b836a0bd64ab67a41afd6091a01",
            "compiler_sha256": "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8",
            "wrapper_sha256": "27a3c5d4f263e4eb96e5619cfcda22f45d33ccd121104c7ff6a37e15b3f427cd",
            "dtk_sha256": "94a3ae31212d070d1ae72bd51461e7c361b46820fd620750576f7b61a9df7108",
            "objdiff_sha256": "3023818f7fdd2f2dc6ade16e68d2c37f5f5754f96881d18d68ddfce77ced15e1",
            "target_object_sha256": "a1799b041c6bb18b9ea60410518007c90887510d9e07288cb9db373525c7679b",
        },
        "provenance": {
            "graphify_status": "existing_graph_no_symbol",
            "graphify_bound": False,
            "graft_ask_count": 1,
            "graft_status": "no_nodes",
            "narrow_report_verified": True,
            "broad_searches": 0,
        },
        "mapped_consumer_contract": {
            "target_symbol": "lbl_802C4370",
            "c_type": "const float",
            "value": 10.0,
            "consumer_count": 7,
            "strict_rows_closed": True,
        },
        "pool_gap": {
            "section": ".sdata2",
            "target_offset": 0x1AC,
            "c_type": "const float",
            "width": 4,
            "bits": "0x437A0000",
            "value": 250.0,
            "downstream_values": [80.0, 135.0],
            "predicted_downstream_shift": 4,
            "residual_rows": [
                {"row": row, "target_owner": target_owner, "candidate_owner": candidate_owner}
                for row, target_owner, candidate_owner in global_pool.BOUNDARY_RESIDUAL_ROWS
            ],
        },
        "rejected_controls": controls,
        "producer": {
            "declaration": "const float capspecialKettouHeight = 250.0f;",
            "source_name": "capspecialKettouHeight",
            "c_type": "const float",
            "value": 250.0,
            "source_line": 3977,
            "after_function": "ev_CapKettouMesGet",
            "before_function": "mbev_CapDonkey",
            "file_scope": True,
            "semantic_live": True,
        },
        "baseline": {
            "candidate_id": "c697-distinct-physical-owner-and-mapped-contract",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["boundary-control"])),
            "source_sha256": "76" * 32,
            "object_sha256": "42" * 32,
            "strict_report_sha256": "58" * 32,
            "data_report_sha256": "87" * 32,
            "strict_match_percent": 99.97872,
            "data_exact": True,
            "diff_rows": 5,
        },
        "exact_result": {
            "candidate_id": "c698-kettou-250-physical-producer",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["boundary-exact"])),
            "source_sha256": "e6" * 32,
            "object_sha256": "2a" * 32,
            "strict_report_sha256": "0c" * 32,
            "data_report_sha256": "cb" * 32,
            "strict_match_percent": 100.0,
            "data_exact": True,
            "diff_rows": 0,
            "compile_attestation_record_sha256": "a4" * 32,
            "compile_attestation_file_sha256": "00" * 32,
            "candidate_record_sha256": "ac" * 32,
            "independent_record_sha256": "51" * 32,
            "strict_data_exact": True,
            "later_production_drift_excluded": True,
        },
        "telemetry": {
            "parent_active_seconds": 1651.5189765,
            "candidate_heavy_seconds": 0.1309643,
            "independent_heavy_seconds": 0.1270606,
            "wait_seconds": 0.0,
            "telemetry_complete": True,
            "current_campaign_eligible": True,
            "no_imputation": True,
            "active_interval_log_sha256": "9813367290cac203f04ec1d2617e1700614481eaacbc17ececf71edec1307059",
        },
        "forbidden_axes": list(global_pool.BOUNDARY_FORBIDDEN_AXES),
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(item for item in result["evaluations"] if item["rule_id"] == global_pool.RULE_ID)


class TuGlobalPoolProducerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reports = {stage: _report(stage) for stage in ("control", "exact")}
        self.boundary_reports = {
            stage: _boundary_report(stage)
            for stage in ("boundary-control", "boundary-exact")
        }

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

    def test_boundary_precursor_ranks_only_typed_producer(self) -> None:
        result = rules.diagnose_document(
            self.boundary_reports["boundary-control"],
            focus_symbol=global_pool.BOUNDARY_FUNCTION,
            tu_global_pool_producer_context=_boundary_context(self.boundary_reports),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(evidence["stage"], "missing_typed_boundary_producer")
        self.assertEqual(len(evidence["recommended_cells"]), 1)
        self.assertEqual(
            evidence["recommended_cells"][0]["declaration"],
            "const float capspecialKettouHeight = 250.0f;",
        )
        self.assertEqual(evidence["pool_gap"]["predicted_downstream_shift"], 4)
        self.assertTrue(evidence["suppress_downstream_body_edits"])
        self.assertTrue(evidence["suppress_linkage_retries"])
        self.assertTrue(evidence["suppress_tracer"])
        self.assertFalse(evidence["full_owner_closed"])
        self.assertTrue(evidence["later_production_drift_excluded"])

    def test_boundary_exact_schedules_nothing(self) -> None:
        result = rules.diagnose_document(
            self.boundary_reports["boundary-exact"],
            focus_symbol=global_pool.BOUNDARY_FUNCTION,
            tu_global_pool_producer_context=_boundary_context(self.boundary_reports),
        )
        evaluation = _evaluation(result)
        self.assertFalse(evaluation["matched"])
        self.assertIn("already exact", evaluation["reason"])
        self.assertFalse(evaluation["evidence"]["full_owner_closed"])
        self.assertTrue(evaluation["evidence"]["later_production_drift_excluded"])

    def test_boundary_non_owner_residual_fails_closed(self) -> None:
        report = copy.deepcopy(self.boundary_reports["boundary-control"])
        report["right"]["symbols"][0]["instructions"][200]["instruction"]["formatted"] = "fmr f0, f1"
        context = _boundary_context(self.boundary_reports)
        context["baseline"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
        evaluation = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol=global_pool.BOUNDARY_FUNCTION,
                tu_global_pool_producer_context=context,
            )
        )
        self.assertFalse(evaluation["matched"])
        self.assertIn("non-pool-owner", evaluation["reason"])

    def test_boundary_context_drift_fails_closed(self) -> None:
        mutations = [
            lambda context: context["pool_gap"].__setitem__("target_offset", 0x1B0),
            lambda context: context["pool_gap"]["residual_rows"].reverse(),
            lambda context: context["rejected_controls"][0].__setitem__("regressed_exact_functions", 5),
            lambda context: context["producer"].__setitem__("source_line", 3978),
            lambda context: context["telemetry"].__setitem__("telemetry_complete", False),
        ]
        for mutate in mutations:
            context = _boundary_context(self.boundary_reports)
            mutate(context)
            with self.assertRaises(rules.LearningInputError):
                rules.diagnose_document(
                    self.boundary_reports["boundary-control"],
                    focus_symbol=global_pool.BOUNDARY_FUNCTION,
                    tu_global_pool_producer_context=context,
                )

    def test_boundary_cli_accepts_authenticated_context(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(
                json.dumps(self.boundary_reports["boundary-control"]), encoding="utf-8"
            )
            context_path.write_text(
                json.dumps(_boundary_context(self.boundary_reports)), encoding="utf-8"
            )
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(
                    [
                        "--report", str(report_path),
                        "--function", global_pool.BOUNDARY_FUNCTION,
                        "--tu-global-pool-producer-context", str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
