from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


FUNCTION = "mbev_CapDebugWarp"
ROWS = [249, 251, 253, 255, 272, 273, 275]
TARGET_FORMS = [
    "fabs f27, f31",
    "fcmpo cr0, f27, f0",
    "fabs f26, f30",
    "fcmpo cr0, f26, f0",
    "fabs f25, f28",
    "fmr f24, f25",
    "fcmpo cr0, f24, f0",
]
CANDIDATE_FORMS = [
    "fabs f26, f31",
    "fcmpo cr0, f26, f0",
    "fabs f25, f30",
    "fcmpo cr0, f25, f0",
    "fabs f24, f28",
    "fmr f27, f24",
    "fcmpo cr0, f27, f0",
]


def _instruction(
    address: int,
    formatted: str,
    *,
    mismatch: bool = False,
    relocation: bool = False,
) -> dict[str, object]:
    instruction: dict[str, object] = {
        "address": str(address),
        "size": 4,
        "formatted": formatted,
    }
    if relocation:
        instruction["relocation"] = {
            "type": 10,
            "type_name": "R_PPC_REL24",
            "target_symbol": 1,
        }
    row: dict[str, object] = {"instruction": instruction}
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report() -> dict[str, object]:
    count = 461
    target_text = ["nop"] * count
    candidate_text = ["nop"] * count
    target_text[0] = candidate_text[0] = "stwu r1, -0x130(r1)"
    target_text[270] = candidate_text[270] = "bl mbev_CapAngleWrap"
    target_text[271] = candidate_text[271] = "fmr f28, f1"
    target_text[274] = candidate_text[274] = "lfd f0, threshold@sda21(r13)"
    for row, target_form, candidate_form in zip(ROWS, TARGET_FORMS, CANDIDATE_FORMS):
        target_text[row] = target_form
        candidate_text[row] = candidate_form

    def side(text: list[str]) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=index in ROWS,
                relocation=index < 104,
            )
            for index, formatted in enumerate(text)
        ]

    return {
        "left": {
            "symbols": [
                {"name": "pool", "kind": "SYMBOL_DATA"},
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "1844",
                    "match_percent": 99.91323,
                    "instructions": side(target_text),
                },
            ]
        },
        "right": {
            "symbols": [
                {"name": "pool", "kind": "SYMBOL_DATA"},
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "1844",
                    "match_percent": 99.91323,
                    "instructions": side(candidate_text),
                },
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    return {
        "schema": rules.DIRECT_SCALAR_FABS_CONTEXT_SCHEMA,
        "report_artifact_sha256": "51" * 32,
        "precursor": {
            "function": FUNCTION,
            "candidate_id": "debugwarp012-exact-capacities",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "source_sha256": "61" * 32,
            "object_sha256": "9f" * 32,
            "strict_report_sha256": "f2" * 32,
            "data_report_sha256": "f2" * 32,
            "target_bytes": 1844,
            "candidate_bytes": 1844,
            "target_frame": 0x130,
            "candidate_frame": 0x130,
            "match_percent": 99.91323,
            "target_physical_relocations": 104,
            "candidate_physical_relocations": 104,
            "residual_pairs": [
                {"row": row, "target": left, "candidate": right}
                for row, left, right in zip(ROWS, TARGET_FORMS, CANDIDATE_FORMS)
            ],
            "operation_order_exact": True,
            "cfg_calls_exact": True,
            "data_exact": True,
            "stack_homes_exact": True,
            "protected_siblings_preserved": True,
        },
        "call_chain": {
            "call_symbol": "mbev_CapAngleWrap",
            "return_register": "f1",
            "wrapped_return_register": "f28",
            "target_abs_register": "f25",
            "candidate_abs_register": "f24",
            "target_compare_register": "f24",
            "candidate_compare_register": "f27",
            "call_index": 270,
            "return_bind_index": 271,
            "fabs_index": 272,
            "bridge_index": 273,
            "compare_index": 275,
            "immediate_consumer_count": 1,
            "consumer_kind": "fabs_then_immediate_compare",
            "source_template": "fabs((double)mbev_CapAngleWrap(...)) < threshold",
        },
        "donors": [
            {
                "function": "mbev_CapDebugCam",
                "same_translation_unit": True,
                "strict_exact": True,
                "source_location": "src/board/capevent.c:L2248",
                "source_shape": "direct_scalar_return_to_fabs",
                "strict_report_sha256": "86" * 32,
            },
            {
                "function": "mbev_CapPointCullCheck",
                "same_translation_unit": True,
                "strict_exact": True,
                "source_location": "src/board/capevent.c:L8004",
                "source_shape": "direct_fabs_to_immediate_comparison",
                "strict_report_sha256": "86" * 32,
            },
            {
                "function": "mbev_CapAngleSumLerp",
                "same_translation_unit": True,
                "strict_exact": True,
                "source_location": "src/board/capevent.c:L8538",
                "source_shape": "direct_scalar_to_typed_consumer",
                "strict_report_sha256": "86" * 32,
            },
        ],
        "trace": {
            "status": "DIAGNOSTIC_UNKNOWN",
            "same_session": True,
            "authority_advanced": False,
            "event_count": 61,
            "ownership_unknown_present": True,
            "regalloc_complete": False,
            "used_for_recommendation": False,
            "repeat_allowed": False,
            "envelope_file_sha256": "ad" * 32,
            "envelope_sha256": "b2" * 32,
            "trust_root_sha256": "36" * 32,
        },
        "negative_controls": [
            {
                "candidate_id": "debugwarp013-block-abs-angle",
                "axis": "block_or_function_scope",
                "result": "object_identical",
                "measured": True,
                "evidence_sha256": "0e" * 32,
            },
            {
                "candidate_id": "debugwarp014-reversed-compare",
                "axis": "comparison_commutation",
                "result": "regressed_topology",
                "measured": True,
                "evidence_sha256": "14" * 32,
            },
            {
                "candidate_id": "debugwarp015-fabsf",
                "axis": "fabsf_or_prototype_guess",
                "result": "regressed_inadmissible",
                "measured": True,
                "evidence_sha256": "15" * 32,
            },
        ],
        "telemetry": {
            "parent_active_seconds": 3179.795514,
            "helper_active_seconds_sum": 0.0,
            "team_active_seconds_sum": 3179.795514,
            "active_wall_union_seconds": 3179.795514,
            "heavy_seconds": 11.65737,
            "candidate_count": 16,
            "tracer_runs": 1,
            "donor_searches": 2,
            "telemetry_complete": False,
            "excluded_from_measured_crack_per_hour": True,
            "no_imputation": True,
            "telemetry_sha256": "3e" * 32,
            "active_interval_log_sha256": "ec" * 32,
        },
        "exact_result": {
            "source_sha256": "c3" * 32,
            "function_sha256": "7a" * 32,
            "object_sha256": "b1" * 32,
            "strict_report_sha256": "86" * 32,
            "data_report_sha256": "86" * 32,
            "compile_attestation_sha256": "b7" * 32,
            "candidate_record_sha256": "cc" * 32,
            "target_bytes": 1844,
            "candidate_bytes": 1844,
            "physical_relocations": 104,
            "strict_percent": 100.0,
            "data_percent": 100.0,
            "protected_sibling_losses": 0,
        },
        "authority_advanced": False,
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]
        if item["rule_id"] == "direct_scalar_fabs_consumer"
    )


class DirectScalarFabsConsumerTests(unittest.TestCase):
    def test_debugwarp_emits_only_direct_composition_cell(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            direct_scalar_fabs_context=_context(report),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(len(evidence["recommended_cells"]), 1)
        self.assertEqual(
            evidence["recommended_cells"][0]["kind"],
            "direct_scalar_return_fabs_composition",
        )
        self.assertFalse(evidence["trace_used_for_ownership"])
        self.assertFalse(evidence["authority_advanced"])

    def test_missing_context_fails_closed(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol=FUNCTION)
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_drift_is_rejected(self) -> None:
        mutations = {
            "trace_use": lambda value: value["trace"].__setitem__(
                "used_for_recommendation", True
            ),
            "trace_unknown": lambda value: value["trace"].__setitem__(
                "ownership_unknown_present", False
            ),
            "donor_exact": lambda value: value["donors"][0].__setitem__(
                "strict_exact", False
            ),
            "donor_shape": lambda value: value["donors"][0].__setitem__(
                "source_shape", "named_intermediate"
            ),
            "control": lambda value: value["negative_controls"][1].__setitem__(
                "result", "object_identical"
            ),
            "telemetry": lambda value: value["telemetry"].__setitem__(
                "telemetry_complete", True
            ),
            "imputation": lambda value: value["telemetry"].__setitem__(
                "no_imputation", False
            ),
            "authority": lambda value: value.__setitem__("authority_advanced", True),
            "extra": lambda value: value.__setitem__("unexpected", True),
        }
        report = _report()
        for name, mutate in mutations.items():
            context = _context(report)
            mutate(context)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol=FUNCTION,
                        direct_scalar_fabs_context=context,
                    )

    def test_report_hash_drift_fails_closed(self) -> None:
        report = _report()
        context = _context(report)
        report["right"]["symbols"][1]["instructions"][249]["instruction"][
            "formatted"
        ] = "fabs f25, f31"
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            direct_scalar_fabs_context=context,
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_call_or_chain_drift_fails_closed(self) -> None:
        for name, row, form in (
            ("call", 270, "bl other_helper"),
            ("bind", 271, "fmr f27, f1"),
            ("bridge", 273, "fmr f26, f25"),
        ):
            report = _report()
            context = _context(report)
            report["left"]["symbols"][1]["instructions"][row]["instruction"][
                "formatted"
            ] = form
            context["precursor"]["objdiff_canonical_sha256"] = rules._sha256(
                rules._canonical(report)
            )
            with self.subTest(name=name):
                result = rules.diagnose_document(
                    report,
                    focus_symbol=FUNCTION,
                    direct_scalar_fabs_context=context,
                )
                self.assertFalse(_evaluation(result)["matched"])

    def test_cli_accepts_authenticated_context(self) -> None:
        report = _report()
        context = _context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(
                    [
                        "--report",
                        str(report_path),
                        "--function",
                        FUNCTION,
                        "--direct-scalar-fabs-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
