from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


FUNCTION = "mbev_CapEffDustCloudAdd"
POOL_ROWS = [478, 486, 494]


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
            "type": 21,
            "type_name": "R_PPC_EMB_SDA21",
            "target_symbol": 1,
        }
    row: dict[str, object] = {"instruction": instruction}
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report(stage: str) -> dict[str, object]:
    count = 745
    target_text = ["nop"] * count
    candidate_text = ["nop"] * count
    if stage == "composition":
        residual = set(range(20, 340))
        candidate_text[71] = "stfs f1, 0x18(r1)"
        target_text[71] = "nop"
        target_size = 2980
        candidate_size = 2984
    else:
        residual = set(POOL_ROWS)
        for row in POOL_ROWS:
            target_text[row] = "lfs f1, lbl_802C46F8@sda21(r13)"
            candidate_text[row] = "lfs f1, @1604@sda21(r13)"
        target_size = candidate_size = 2980

    def side(text: list[str]) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=index in residual,
                relocation=index < 142,
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
                    "size": str(target_size),
                    "match_percent": 98.585236 if stage == "composition" else 99.979866,
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
                    "size": str(candidate_size),
                    "match_percent": 98.585236 if stage == "composition" else 99.979866,
                    "instructions": side(candidate_text),
                },
            ]
        },
    }


def _context(
    composition_report: dict[str, object], pool_report: dict[str, object]
) -> dict[str, object]:
    return {
        "schema": rules.SAVED_FPR_STACK_POOL_CONTEXT_SCHEMA,
        "report_artifact_sha256": "01" * 32,
        "function": FUNCTION,
        "composition_stage": {
            "candidate_id": "capevent-dustcloud-c009-second-loop-trig-owners",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(composition_report)),
            "source_sha256": "02" * 32,
            "object_sha256": "03" * 32,
            "strict_report_sha256": "04" * 32,
            "data_report_sha256": "05" * 32,
            "target_bytes": 2980,
            "candidate_bytes": 2984,
            "match_percent": 98.585236,
            "target_physical_relocations": 142,
            "candidate_physical_relocations": 142,
            "candidate_only_stack_store": "stfs f1, 0x18(r1)",
            "candidate_only_stack_owner": "posSin",
            "candidate_only_stack_offset": 16,
            "stack_home_delta_bytes": 4,
            "minimum_residual_rows": 300,
        },
        "pool_handoff_stage": {
            "candidate_id": "capevent-dustcloud-c011-donor-owner-chronology",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(pool_report)),
            "source_sha256": "06" * 32,
            "object_sha256": "07" * 32,
            "strict_report_sha256": "08" * 32,
            "data_report_sha256": "09" * 32,
            "target_bytes": 2980,
            "candidate_bytes": 2980,
            "match_percent": 99.979866,
            "target_physical_relocations": 142,
            "candidate_physical_relocations": 142,
            "residual_rows": POOL_ROWS,
            "target_owner": "lbl_802C46F8@sda21",
            "candidate_owner": "@1604@sda21",
            "target_value": 32.0,
            "candidate_value": 192.0,
        },
        "trace": {
            "status": "CAPTURED_UNKNOWN_OWNERSHIP",
            "authoritative_scope": "object_inventory_and_stack_offset_only",
            "unknown_ownership_present": True,
            "regalloc_complete": False,
            "object_inventory_authenticated": True,
            "stack_offset_authenticated": True,
            "envelope_sha256": "0a" * 32,
            "source_sha256": "02" * 32,
            "request_sha256": "0b" * 32,
            "stack_stream_sha256": "0c" * 32,
            "pcode_stream_sha256": "0d" * 32,
        },
        "donor": {
            "function": "mbev_CapEffDustExplodeAdd",
            "same_translation_unit": True,
            "strict_exact": True,
            "graphify_location": "src/board/capevent.c:L3531",
            "source_shape": "single value owner reused for radius velocity and color randomness",
        },
        "interaction": {
            "request_sha256": "0e" * 32,
            "planner_sha256": "0f" * 32,
            "priority_cell": "cell-98ce4d8849523a73",
            "axes": ["reuse_value", "distinct_distance2", "exact_donor_extended"],
            "rank_within_top": 3,
        },
        "negative_controls": [
            {
                "candidate_id": "c005",
                "axis": "distinct_distance_only",
                "strict_percent": 96.507385,
                "measured": True,
                "result": "regressed",
            },
            {
                "candidate_id": "c006",
                "axis": "broad_historical_prefix",
                "strict_percent": 96.6698,
                "measured": True,
                "result": "regressed",
            },
            {
                "candidate_id": "c007",
                "axis": "broad_fpr_grouping",
                "strict_percent": 96.68322,
                "measured": True,
                "result": "regressed",
            },
            {
                "candidate_id": "c010",
                "axis": "direct_trig_consumption",
                "strict_percent": 96.20805,
                "measured": True,
                "result": "regressed_frame_0x2a0",
            },
        ],
        "telemetry": {
            "parent_active_seconds": 3249.621518099957,
            "helper_active_seconds_sum": 935.512994699995,
            "team_active_seconds_sum": 4185.134512799952,
            "active_wall_union_seconds": 4185.192162,
            "telemetry_complete": False,
            "heavy_seconds_complete": False,
            "excluded_from_measured_crack_per_hour": True,
            "no_imputation": True,
            "uncovered_seconds": 185.637031,
            "telemetry_sha256": "10" * 32,
            "active_interval_log_sha256": "11" * 32,
            "matrix_sha256": "12" * 32,
        },
        "exact_result": {
            "source_sha256": "13" * 32,
            "function_hunk_sha256": "14" * 32,
            "object_sha256": "15" * 32,
            "strict_report_sha256": "16" * 32,
            "data_report_sha256": "17" * 32,
            "compile_attestation_sha256": "18" * 32,
            "candidate_record_sha256": "19" * 32,
            "target_bytes": 2980,
            "candidate_bytes": 2980,
            "physical_relocations": 142,
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
        if item["rule_id"] == "saved_fpr_stack_pool_composer"
    )


class SavedFprStackPoolComposerTests(unittest.TestCase):
    def test_composition_stage_ranks_only_sealed_three_axis_cell(self) -> None:
        composition = _report("composition")
        pool = _report("pool")
        result = rules.diagnose_document(
            composition,
            focus_symbol=FUNCTION,
            saved_fpr_stack_pool_context=_context(composition, pool),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(evidence["stage"], "owner_composition")
        self.assertEqual(
            evidence["recommended_cells"][0]["axes"],
            ["reuse_value", "distinct_distance2", "exact_donor_extended"],
        )
        self.assertFalse(evidence["trace"]["regalloc_complete"])
        self.assertFalse(evidence["authority_advanced"])

    def test_pool_stage_hands_exact_three_rows_to_typed_pool(self) -> None:
        composition = _report("composition")
        pool = _report("pool")
        result = rules.diagnose_document(
            pool,
            focus_symbol=FUNCTION,
            saved_fpr_stack_pool_context=_context(composition, pool),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(evidence["stage"], "typed_pool_handoff")
        batch = evidence["recommended_batches"]
        self.assertEqual(len(batch), 1)
        self.assertEqual(batch[0]["rows"], POOL_ROWS)
        self.assertEqual(batch[0]["target_value"], 32.0)
        self.assertEqual(batch[0]["candidate_value"], 192.0)

    def test_missing_context_fails_closed(self) -> None:
        result = rules.diagnose_document(_report("composition"), focus_symbol=FUNCTION)
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_drift_is_rejected(self) -> None:
        mutations = {
            "unknown": lambda value: value["trace"].__setitem__(
                "unknown_ownership_present", False
            ),
            "regalloc": lambda value: value["trace"].__setitem__(
                "regalloc_complete", True
            ),
            "axis": lambda value: value["interaction"]["axes"].__setitem__(
                0, "distinct_value"
            ),
            "control": lambda value: value["negative_controls"][0].__setitem__(
                "measured", False
            ),
            "telemetry": lambda value: value["telemetry"].__setitem__(
                "telemetry_complete", True
            ),
            "imputation": lambda value: value["telemetry"].__setitem__(
                "no_imputation", False
            ),
            "authority": lambda value: value.__setitem__("authority_advanced", True),
            "pool_rows": lambda value: value["pool_handoff_stage"].__setitem__(
                "residual_rows", [478, 486]
            ),
            "extra": lambda value: value.__setitem__("unexpected", True),
        }
        composition = _report("composition")
        pool = _report("pool")
        for name, mutate in mutations.items():
            context = _context(composition, pool)
            mutate(context)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        composition,
                        focus_symbol=FUNCTION,
                        saved_fpr_stack_pool_context=context,
                    )

    def test_report_drift_fails_closed(self) -> None:
        composition = _report("composition")
        pool = _report("pool")
        mutations = {
            "store": lambda report: report["right"]["symbols"][1]["instructions"][71][
                "instruction"
            ].__setitem__("formatted", "stfs f1, 0x1c(r1)"),
            "relocation": lambda report: report["right"]["symbols"][1][
                "instructions"
            ][141]["instruction"].pop("relocation"),
        }
        for name, mutate in mutations.items():
            report = copy.deepcopy(composition)
            context = _context(report, pool)
            mutate(report)
            context["composition_stage"]["objdiff_canonical_sha256"] = rules._sha256(
                rules._canonical(report)
            )
            with self.subTest(name=name):
                result = rules.diagnose_document(
                    report,
                    focus_symbol=FUNCTION,
                    saved_fpr_stack_pool_context=context,
                )
                self.assertFalse(_evaluation(result)["matched"])

    def test_pool_row_drift_fails_closed(self) -> None:
        composition = _report("composition")
        pool = _report("pool")
        context = _context(composition, pool)
        pool["right"]["symbols"][1]["instructions"][494]["instruction"][
            "formatted"
        ] = "lfs f1, @1605@sda21(r13)"
        context["pool_handoff_stage"]["objdiff_canonical_sha256"] = rules._sha256(
            rules._canonical(pool)
        )
        result = rules.diagnose_document(
            pool,
            focus_symbol=FUNCTION,
            saved_fpr_stack_pool_context=context,
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_cli_accepts_authenticated_context(self) -> None:
        composition = _report("composition")
        pool = _report("pool")
        context = _context(composition, pool)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(composition), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(
                    [
                        "--report",
                        str(report_path),
                        "--function",
                        FUNCTION,
                        "--saved-fpr-stack-pool-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
