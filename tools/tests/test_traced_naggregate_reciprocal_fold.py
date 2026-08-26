from __future__ import annotations

import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


def _instruction(address: int, formatted: str, *, mismatch: bool = False) -> dict[str, object]:
    row: dict[str, object] = {
        "instruction": {"address": str(address), "size": 4, "formatted": formatted}
    }
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report() -> dict[str, object]:
    target_text = [
        "stwu r1, -0x100(r1)",
        "stfs f1, 0x1c(r1)",
        "stw r3, 0x20(r1)",
        "lfs f0, lbl_blue@sda21(r13)",
        "lfs f1, lbl_gravity@sda21(r13)",
        "lfs f2, lbl_offset@sda21(r13)",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -0x100(r1)",
        "stfs f1, 0x20(r1)",
        "stw r3, 0x1c(r1)",
        "lfs f0, @blue128@sda21(r13)",
        "lfs f1, @gravity_direct@sda21(r13)",
        "lfs f2, @offset15@sda21(r13)",
        "blr",
    ]
    return {
        "left": {
            "symbols": [
                {
                    "name": "mbev_CapEffCoinOMExec",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "3216",
                    "match_percent": 99.97263,
                    "instructions": [
                        _instruction(100 + 4 * index, text, mismatch=index in {1, 2, 3, 4, 5})
                        for index, text in enumerate(target_text)
                    ],
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": "mbev_CapEffCoinOMExec",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "3216",
                    "match_percent": 99.97263,
                    "instructions": [
                        _instruction(100 + 4 * index, text, mismatch=index in {1, 2, 3, 4, 5})
                        for index, text in enumerate(candidate_text)
                    ],
                }
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    source = "9496f708be33fc831c561525ac93f58949bdd3144547da6cf48fe8d360e1ba3b"
    obj = "26f0cac81be482a031365e92487b85246d0f53a730ac47ba70ca08e29e45b3b2"
    strict = "1b2c00d7e2bb3347368da0ef5b782e21b83981395382e4a3ef47142947a4feb1"
    data = "0b58429796d34a4d175b522224acbbf62482afe964a503fee0f92b0697eff0d9"
    record = "2f550898f0f78b558d7bb92c64fd5d7cd1d644a717210449c5d3f60a5d16b5d7"
    proof_hashes = {
        "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
        "strict_report_sha256": "01" * 32,
        "data_report_sha256": "02" * 32,
        "trace_envelope_sha256": "03" * 32,
        "trace_stack_events_sha256": "04" * 32,
        "trace_pcode_events_sha256": "05" * 32,
        "same_tu_precedent_receipt_sha256": "06" * 32,
        "typed_pool_receipt_sha256": "07" * 32,
        "precursor_source_sha256": "08" * 32,
        "precursor_object_sha256": "09" * 32,
        "precursor_record_sha256": "0a" * 32,
        "aggregate_exact_source_sha256": "0b" * 32,
        "aggregate_exact_object_sha256": "0c" * 32,
        "aggregate_exact_record_sha256": "0d" * 32,
        "pool_precursor_source_sha256": "0e" * 32,
        "pool_precursor_object_sha256": "0f" * 32,
        "pool_precursor_record_sha256": "10" * 32,
        "exact_source_sha256": source,
        "exact_object_sha256": obj,
        "exact_strict_report_sha256": strict,
        "exact_data_report_sha256": data,
        "exact_record_sha256": record,
        "independent_rebuild_receipt_sha256": "921ae11ae507df83cef8d60a468029b32fb1bb6217de1158a1ff64dc4ce85acd",
        "report_artifact_sha256": "255f669b93fc813cafb7afdedb37aefb2875028b659d0b67237c049e5613a65f",
    }
    return {
        "schema": rules.TRACED_NAGGREGATE_RECIPROCAL_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "cfg_calls_exact": True,
            "physical_relocations_exact": True,
            "trace_same_session_authenticated": True,
            "traced_home_swap_closed": True,
            "same_tu_numbered_precedent_authenticated": True,
            "typed_pool_decoder_authenticated": True,
            "semantic_literals_authenticated": True,
            "exact_result_verified": True,
            "protected_siblings_preserved": True,
            "authority_advanced": False,
            **proof_hashes,
        },
        "precursor": {
            "function": "mbev_CapEffCoinOMExec",
            "candidate_id": "capevent-coinom-c037",
            "target_bytes": 3216,
            "candidate_bytes": 3216,
            "target_frame": 0x100,
            "candidate_frame": 0x100,
            "match_percent": 99.97263,
            "target_physical_relocations": 172,
            "candidate_physical_relocations": 172,
            "cycle_rows": [1, 2],
            "pool_rows": [3, 4, 5],
        },
        "trace_cycle": {
            "session_id": "session-0123456789abcdef",
            "aggregate_type": "GXColor",
            "aggregate_base_name": "color",
            "aggregate_count": 3,
            "aggregate_size": 4,
            "target_aggregate_homes": [0x28, 0x24, 0x20],
            "scalar_identity": "sinAngleX",
            "scalar_trace_token": "local-000011",
            "aggregate_identity": "state3Color",
            "aggregate_trace_token": "local-000030",
            "scalar_target_home": 0x1C,
            "scalar_candidate_home": 0x20,
            "aggregate_target_home": 0x20,
            "aggregate_candidate_home": 0x1C,
            "seam_unknown_count": 0,
            "alias_summary_complete": True,
        },
        "same_tu_precedent": {
            "owner": "main:board/capevent",
            "source_file": "src/board/capevent.c",
            "same_translation_unit": True,
            "narrow_verified": True,
            "numbered_precedent_authenticated": True,
            "declarations": [
                {"function": "mbev_CapEffExplodeAdd", "identity": "color1", "source_line": 3275},
                {"function": "mbev_CapEffDustCloudAdd", "identity": "color2", "source_line": 3335},
            ],
        },
        "semantic_pool_batch": [
            {"role": "blue_base", "consumer_state": 3, "row": 3, "target_f32_bits": "42800000", "expression": "64.0f", "typed_f32": True, "semantic_consumer_authenticated": True},
            {"role": "gravity_scaled", "consumer_state": 3, "row": 4, "target_f32_bits": "40511112", "expression": "CAPEVENT_GRAVITY / 3.0f", "typed_f32": True, "semantic_consumer_authenticated": True},
            {"role": "vertical_offset", "consumer_state": 4, "row": 5, "target_f32_bits": "42480000", "expression": "50.0f", "typed_f32": True, "semantic_consumer_authenticated": True},
        ],
        "rounded_reciprocal": {
            "numerator_identity": "CAPEVENT_GRAVITY",
            "numerator_f32_bits": "411ccccd",
            "denominator": 3,
            "reciprocal_f32_bits": "3eaaaaab",
            "direct_division_f32_bits": "40511111",
            "reciprocal_multiply_f32_bits": "40511112",
            "target_f32_bits": "40511112",
            "direct_expression": "CAPEVENT_GRAVITY / 3.0f",
            "reciprocal_expression": "CAPEVENT_GRAVITY * (1.0f / 3.0f)",
            "one_ulp_residual": True,
        },
        "telemetry": {
            "parent_active_seconds": 4141.6411053,
            "active_seconds_measured": True,
            "telemetry_complete": False,
            "exclude_from_measured_crack_hour": True,
            "no_imputation": True,
            "telemetry_sha256": "27456d7a9f3190f208928807a46c20421f9580606cccce2f80ddb0ba1e69d544",
            "active_interval_log_sha256": "615694810825a78aecc392d312e153445172342f9dbd5206ed8ed9114975461c",
        },
        "exact_result": {
            "candidate_id": "capevent-coinom-c047-reciprocal-multiply-exact",
            "target_bytes": 3216,
            "candidate_bytes": 3216,
            "physical_relocations": 172,
            "source_sha256": source,
            "object_sha256": obj,
            "strict_report_sha256": strict,
            "data_report_sha256": data,
            "candidate_record_sha256": record,
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "traced_naggregate_reciprocal_fold"
    )


class TracedNAggregateReciprocalFoldTests(unittest.TestCase):
    def test_coinom_schedules_three_bounded_cells(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffCoinOMExec",
            traced_naggregate_reciprocal_context=_context(report),
        )
        diagnosis = _evaluation(result)
        self.assertTrue(diagnosis["matched"])
        cells = diagnosis["evidence"]["recommended_cells"]  # type: ignore[index]
        self.assertEqual([cell["order"] for cell in cells], [1, 2, 3])
        self.assertEqual(
            cells[0]["declarations"],
            ["GXColor color1;", "GXColor color2;", "GXColor color3;"],
        )
        self.assertEqual(cells[0]["target_homes"], [0x28, 0x24, 0x20])
        self.assertEqual(cells[2]["direct_f32_bits"], "40511111")
        self.assertEqual(cells[2]["target_f32_bits"], "40511112")
        self.assertEqual(cells[2]["ulp_distance"], 1)
        self.assertFalse(result["authority_advanced"])

    def test_fails_closed_without_context(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol="mbev_CapEffCoinOMExec")
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_trace_precedent_fold_and_telemetry_drift(self) -> None:
        report = _report()
        mutations: list[tuple[str, callable]] = [
            ("session", lambda value: value["trace_cycle"].__setitem__("session_id", "session-1234")),
            ("unknown", lambda value: value["trace_cycle"].__setitem__("seam_unknown_count", 1)),
            ("alias", lambda value: value["trace_cycle"].__setitem__("alias_summary_complete", False)),
            ("homes", lambda value: value["trace_cycle"].__setitem__("target_aggregate_homes", [0x28, 0x20, 0x24])),
            ("swap", lambda value: value["trace_cycle"].__setitem__("aggregate_candidate_home", 0x18)),
            ("same_tu", lambda value: value["same_tu_precedent"].__setitem__("same_translation_unit", False)),
            ("numbering", lambda value: value["same_tu_precedent"]["declarations"][1].__setitem__("identity", "color3")),
            ("opaque", lambda value: value["semantic_pool_batch"][0].__setitem__("expression", "0x42800000")),
            ("power_two", lambda value: value["rounded_reciprocal"].__setitem__("denominator", 4)),
            ("target_fold", lambda value: value["rounded_reciprocal"].__setitem__("target_f32_bits", "40511111")),
            ("throughput", lambda value: value["telemetry"].__setitem__("exclude_from_measured_crack_hour", False)),
            ("authority", lambda value: value["proofs"].__setitem__("authority_advanced", True)),
        ]
        for name, mutate in mutations:
            unsafe = _context(report)
            mutate(unsafe)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol="mbev_CapEffCoinOMExec",
                        traced_naggregate_reciprocal_context=unsafe,
                    )

    def test_report_residual_and_stack_signature_drift_fail_closed(self) -> None:
        extra = _report()
        extra["right"]["symbols"][0]["instructions"][6]["instruction"]["formatted"] = "nop"  # type: ignore[index]
        result = rules.diagnose_document(
            extra,
            focus_symbol="mbev_CapEffCoinOMExec",
            traced_naggregate_reciprocal_context=_context(extra),
        )
        self.assertFalse(_evaluation(result)["matched"])

        wrong_stack = _report()
        wrong_stack["right"]["symbols"][0]["instructions"][2]["instruction"]["formatted"] = "stw r3, 0x18(r1)"  # type: ignore[index]
        result = rules.diagnose_document(
            wrong_stack,
            focus_symbol="mbev_CapEffCoinOMExec",
            traced_naggregate_reciprocal_context=_context(wrong_stack),
        )
        self.assertFalse(_evaluation(result)["matched"])

        wrong_frame = _report()
        wrong_frame["right"]["symbols"][0]["instructions"][0]["instruction"]["formatted"] = "stwu r1, -0xf0(r1)"  # type: ignore[index]
        result = rules.diagnose_document(
            wrong_frame,
            focus_symbol="mbev_CapEffCoinOMExec",
            traced_naggregate_reciprocal_context=_context(wrong_frame),
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_exact_result_hash_drift_is_rejected(self) -> None:
        report = _report()
        context = _context(report)
        context["exact_result"]["object_sha256"] = "ff" * 32  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "drifts from proofs"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffCoinOMExec",
                traced_naggregate_reciprocal_context=context,
            )

    def test_cli_accepts_authenticated_context(self) -> None:
        report = _report()
        context = _context(report)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            report_path = root / "report.json"
            context_path = root / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                exit_code = rules.main(
                    [
                        "--report",
                        str(report_path),
                        "--function",
                        "mbev_CapEffCoinOMExec",
                        "--traced-naggregate-reciprocal-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        payload = json.loads(stdout.getvalue())
        self.assertTrue(_evaluation(payload)["matched"])


if __name__ == "__main__":
    unittest.main()
