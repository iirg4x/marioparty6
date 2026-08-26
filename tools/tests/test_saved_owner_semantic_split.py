from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


RESIDUAL_GROUPS = {
    "saved_range": [4, 666],
    "data_format": [63, 64, 66],
    "callback": [199, 203],
    "outer_i": [320, 441, 443, 457, 505, 507, 512, 538, 540, 549, 569, 571],
    "pat_x": [461, 468],
    "pat_y": [464, 484],
    "inner_j": [465, 477, 493, 500, 502, 514, 517, 525, 533, 535, 551, 553, 558, 564, 566],
}
TOKENS = {
    "saved_range": ("gpr_19", "gpr_20"),
    "data_format": ("r22", "r26"),
    "callback": ("r19", "r20"),
    "outer_i": ("r25", "r26"),
    "pat_x": ("r21", "r22"),
    "pat_y": ("r20", "r21"),
    "inner_j": ("r26", "r25"),
}


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
    target_text = ["nop"] * 671
    candidate_text = ["nop"] * 671
    target_text[0] = candidate_text[0] = "stwu r1, -0x1a0(r1)"
    target_text[4], candidate_text[4] = "bl _savegpr_19", "bl _savegpr_20"
    target_text[666], candidate_text[666] = "bl _restgpr_19", "bl _restgpr_20"
    for name, rows in RESIDUAL_GROUPS.items():
        if name == "saved_range":
            continue
        target_token, candidate_token = TOKENS[name]
        for row in rows:
            target_text[row] = f"mr r3, {target_token}"
            candidate_text[row] = f"mr r3, {candidate_token}"
    residual_rows = {row for rows in RESIDUAL_GROUPS.values() for row in rows}

    def side(text: list[str]) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + 4 * index,
                formatted,
                mismatch=index in residual_rows,
                relocation=index < 113,
            )
            for index, formatted in enumerate(text)
        ]

    return {
        "left": {
            "symbols": [
                {"name": "reloc_target", "kind": "SYMBOL_DATA"},
                {
                    "name": "ev_CapEffDraw",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "2684",
                    "match_percent": 99.66468,
                    "instructions": side(target_text),
                },
            ]
        },
        "right": {
            "symbols": [
                {"name": "reloc_target", "kind": "SYMBOL_DATA"},
                {
                    "name": "ev_CapEffDraw",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "2684",
                    "match_percent": 99.66468,
                    "instructions": side(candidate_text),
                },
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    precursor_source = "01" * 32
    precursor_object = "02" * 32
    function_source = "03" * 32
    function_object = "04" * 32
    direct_source = "05" * 32
    direct_object = "06" * 32
    exact_source = "07" * 32
    exact_object = "08" * 32
    exact_strict = "09" * 32
    exact_data = "0a" * 32
    exact_record = "0b" * 32
    residual_rows = sorted(row for rows in RESIDUAL_GROUPS.values() for row in rows)
    return {
        "schema": rules.SAVED_OWNER_SEMANTIC_SPLIT_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "cfg_calls_exact": True,
            "data_exact": True,
            "physical_relocations_exact": True,
            "same_session_object_inventory_authenticated": True,
            "measured_controls_complete": True,
            "interaction_plan_authenticated": True,
            "exact_result_verified": True,
            "protected_siblings_preserved": True,
            "authority_advanced": False,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "0c" * 32,
            "data_report_sha256": "0d" * 32,
            "trace_envelope_file_sha256": "0e" * 32,
            "trace_envelope_sha256": "0f" * 32,
            "interaction_request_sha256": "10" * 32,
            "interaction_plan_sha256": "11" * 32,
            "precursor_source_sha256": precursor_source,
            "precursor_object_sha256": precursor_object,
            "function_scope_source_sha256": function_source,
            "function_scope_object_sha256": function_object,
            "direct_callback_source_sha256": direct_source,
            "direct_callback_object_sha256": direct_object,
            "exact_source_sha256": exact_source,
            "exact_object_sha256": exact_object,
            "exact_strict_report_sha256": exact_strict,
            "exact_data_report_sha256": exact_data,
            "exact_record_sha256": exact_record,
            "compile_attestation_sha256": "12" * 32,
            "report_artifact_sha256": "13" * 32,
        },
        "precursor": {
            "function": "ev_CapEffDraw",
            "candidate_id": "capevent-effdraw-c005",
            "target_bytes": 2684,
            "candidate_bytes": 2684,
            "target_frame": 0x1A0,
            "candidate_frame": 0x1A0,
            "match_percent": 99.66468,
            "target_physical_relocations": 113,
            "candidate_physical_relocations": 113,
            "target_instruction_relocations": 113,
            "candidate_instruction_relocations": 113,
            "residual_rows": residual_rows,
            "residual_groups": [
                {
                    "name": name,
                    "owner": {
                        "saved_range": "savedRange",
                        "data_format": "dataFmt",
                        "callback": "hook",
                        "outer_i": "i",
                        "pat_x": "patX",
                        "pat_y": "patY",
                        "inner_j": "j",
                    }[name],
                    "rows": rows,
                    "target_token": TOKENS[name][0],
                    "candidate_token": TOKENS[name][1],
                }
                for name, rows in RESIDUAL_GROUPS.items()
            ],
        },
        "trace_inventory": {
            "session_id": "session-0123456789abcdef",
            "function": "ev_CapEffDraw",
            "candidate_id": "capevent-effdraw-c008-function-callback",
            "complete_object_inventory": True,
            "unknown_owner_count": 0,
            "source_spans_narrow_verified": True,
            "owners": [
                {"identity": "hook", "candidate_register": "r22", "target_register": "r19", "role": "typedCallback"},
                {"identity": "patX", "candidate_register": "r21", "target_register": "r21", "role": "patternX"},
                {"identity": "patY", "candidate_register": "r20", "target_register": "r20", "role": "patternY"},
                {"identity": "i", "candidate_register": "r26", "target_register": "r25", "role": "outerLoop"},
                {"identity": "j", "candidate_register": "r25", "target_register": "r26", "role": "innerLoop"},
            ],
        },
        "measured_controls": {
            "block_callback_precursor": {
                "candidate_id": "capevent-effdraw-c005",
                "source_sha256": precursor_source,
                "object_sha256": precursor_object,
                "target_bytes": 2684,
                "candidate_bytes": 2684,
                "match_percent": 99.66468,
                "callback_form": "block_local",
                "data_format_form": "reuse_outer_i",
                "measured": True,
            },
            "function_scope_callback": {
                "candidate_id": "capevent-effdraw-c008-function-callback",
                "source_sha256": function_source,
                "object_sha256": function_object,
                "target_bytes": 2684,
                "candidate_bytes": 2684,
                "match_percent": 99.69449,
                "callback_form": "function_local",
                "data_format_form": "reuse_outer_i",
                "measured": True,
            },
            "direct_callback_with_split": {
                "candidate_id": "capevent-effdraw-c009-direct-split",
                "source_sha256": direct_source,
                "object_sha256": direct_object,
                "target_bytes": 2684,
                "candidate_bytes": 2680,
                "match_percent": 99.49329,
                "callback_form": "direct_field_call",
                "data_format_form": "distinct_s16_dataFmt",
                "measured": True,
            },
            "declaration_order_control": {
                "candidate_id": "capevent-effdraw-c007-declaration-order",
                "object_sha256": precursor_object,
                "same_as_precursor": True,
                "measured": True,
            },
        },
        "interaction": {
            "axes": ["callback_consumption", "data_format_owner"],
            "only_missing_cell": True,
            "callback_form": "block_local",
            "semantic_owner_form": "distinct_s16_dataFmt",
            "semantic_owner_type": "s16",
            "semantic_owner_identity": "dataFmt",
            "suppressed_axes": [
                "direct_field_call",
                "function_local_only",
                "declaration_order_i_j",
                "repeat_tracer",
            ],
        },
        "telemetry": {
            "parent_active_seconds": 2652.6958587,
            "helper_active_seconds_sum": 1493.6242754,
            "team_active_seconds_sum": 4146.3201341,
            "active_wall_union_seconds": 4146.380995,
            "active_time_telemetry_complete": True,
            "heavy_seconds_complete": False,
            "no_imputation": True,
            "telemetry_sha256": "14" * 32,
            "active_interval_log_sha256": "15" * 32,
        },
        "exact_result": {
            "candidate_id": "capevent-effdraw-c010-exact",
            "target_bytes": 2684,
            "candidate_bytes": 2684,
            "physical_relocations": 113,
            "source_sha256": exact_source,
            "object_sha256": exact_object,
            "strict_report_sha256": exact_strict,
            "data_report_sha256": exact_data,
            "candidate_record_sha256": exact_record,
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "saved_owner_semantic_split_composer"
    )


class SavedOwnerSemanticSplitTests(unittest.TestCase):
    def test_effdraw_emits_only_the_closed_composed_cell(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol="ev_CapEffDraw",
            saved_owner_semantic_split_context=_context(report),
        )
        diagnosis = _evaluation(result)
        self.assertTrue(diagnosis["matched"])
        cells = diagnosis["evidence"]["recommended_cells"]  # type: ignore[index]
        self.assertEqual(len(cells), 1)
        self.assertEqual(cells[0]["semantic_owner_declaration"], "s16 dataFmt;")
        self.assertEqual(cells[0]["callback_form"], "typed_callback_guard_block_local")
        self.assertEqual(
            cells[0]["target_owner_registers"],
            {"dataFmt": "r22", "hook": "r19", "patX": "r21", "patY": "r20", "i": "r25", "j": "r26"},
        )
        self.assertIn("repeat_tracer", diagnosis["evidence"]["suppressed_axes"])  # type: ignore[index]
        self.assertFalse(result["authority_advanced"])

    def test_fails_closed_without_context(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol="ev_CapEffDraw")
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_trace_control_interaction_and_telemetry_drift(self) -> None:
        report = _report()
        mutations = {
            "session": lambda value: value["trace_inventory"].__setitem__("session_id", "session-1234"),
            "unknown": lambda value: value["trace_inventory"].__setitem__("unknown_owner_count", 1),
            "trace_owner": lambda value: value["trace_inventory"]["owners"][0].__setitem__("target_register", "r18"),
            "trace_candidate": lambda value: value["trace_inventory"].__setitem__("candidate_id", "other-candidate"),
            "control_hash": lambda value: value["measured_controls"]["block_callback_precursor"].__setitem__("source_sha256", "aa" * 32),
            "direct_size": lambda value: value["measured_controls"]["direct_callback_with_split"].__setitem__("candidate_bytes", 2684),
            "declaration": lambda value: value["measured_controls"]["declaration_order_control"].__setitem__("same_as_precursor", False),
            "callback_form": lambda value: value["interaction"].__setitem__("callback_form", "function_local"),
            "heavy": lambda value: value["telemetry"].__setitem__("heavy_seconds_complete", True),
            "imputation": lambda value: value["telemetry"].__setitem__("no_imputation", False),
            "authority": lambda value: value["proofs"].__setitem__("authority_advanced", True),
            "extra": lambda value: value.__setitem__("unexpected", True),
        }
        for name, mutate in mutations.items():
            context = _context(report)
            mutate(context)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol="ev_CapEffDraw",
                        saved_owner_semantic_split_context=context,
                    )

    def test_report_drift_fails_closed(self) -> None:
        mutations = {
            "frame": lambda report: report["right"]["symbols"][1]["instructions"][0]["instruction"].__setitem__("formatted", "stwu r1, -0x190(r1)"),
            "row": lambda report: report["right"]["symbols"][1]["instructions"][63]["instruction"].__setitem__("formatted", "mr r3, r27"),
            "extra_mismatch": lambda report: report["right"]["symbols"][1]["instructions"][100].__setitem__("diff_kind", "DIFF_ARG_MISMATCH"),
            "relocation": lambda report: report["right"]["symbols"][1]["instructions"][112]["instruction"].pop("relocation"),
        }
        for name, mutate in mutations.items():
            report = _report()
            context = _context(report)
            mutate(report)
            context["proofs"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
            with self.subTest(name=name):
                result = rules.diagnose_document(
                    report,
                    focus_symbol="ev_CapEffDraw",
                    saved_owner_semantic_split_context=context,
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
                        "ev_CapEffDraw",
                        "--saved-owner-semantic-split-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
