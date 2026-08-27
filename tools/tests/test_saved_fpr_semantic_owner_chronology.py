from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import saved_fpr_semantic_owner_chronology as semantic_owner


FUNCTION = "mbev_CapEffMasuHitOMExec"
ROWS = [110, 112, 129, 130, 132, 133, 136, 138, 144, 146, 149, 150, 162]
TARGET_FORMS = [
    "fmr f26, f1",
    "fmuls f29, f0, f26",
    "lfs f28, 0xc(r1)",
    "fmr f1, f28",
    "fmr f25, f1",
    "fmr f24, f25",
    "fmr f23, f1",
    "fmuls f0, f23, f24",
    "fmr f22, f1",
    "fmuls f0, f30, f22",
    "lfs f27, 0xc(r1)",
    "fmr f1, f27",
    "stfs f29, 0x40(r31)",
]
CANDIDATE_FORMS = [
    "fmr f29, f1",
    "fmuls f28, f0, f29",
    "lfs f27, 0xc(r1)",
    "fmr f1, f27",
    "fmr f26, f1",
    "fmr f25, f26",
    "fmr f24, f1",
    "fmuls f0, f24, f25",
    "fmr f23, f1",
    "fmuls f0, f30, f23",
    "lfs f22, 0xc(r1)",
    "fmr f1, f22",
    "stfs f28, 0x40(r31)",
]
SDA_ROWS = list(range(10, 18))


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


def _report(stage: str) -> dict[str, object]:
    target_text = ["nop"] * 215
    candidate_text = ["nop"] * 215
    target_frame = 0x120
    candidate_frame = 0xC0 if stage == "baseline" else 0x120
    target_text[0] = f"stwu r1, -0x{target_frame:x}(r1)"
    candidate_text[0] = f"stwu r1, -0x{candidate_frame:x}(r1)"

    if stage in {"precursor", "exact"}:
        for offset, row in enumerate(SDA_ROWS):
            target_text[row] = f"lfs f0, lbl_802C46{offset:02X}@sda21"
            candidate_text[row] = f"lfs f0, @{700 + offset}@sda21"
    if stage == "precursor":
        for row, target_form, candidate_form in zip(ROWS, TARGET_FORMS, CANDIDATE_FORMS):
            target_text[row] = target_form
            candidate_text[row] = candidate_form

    def side(text: list[str], *, target: bool) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=stage == "precursor" and index in ROWS,
                relocation=index < 34,
            )
            for index, formatted in enumerate(text)
        ]

    candidate_size = 736 if stage == "baseline" else 860
    match_percent = {
        "baseline": 82.995346,
        "precursor": 99.62791,
        "exact": 100.0,
    }[stage]
    return {
        "left": {
            "symbols": [
                {"name": "pool", "kind": "SYMBOL_DATA"},
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "860",
                    "match_percent": match_percent,
                    "instructions": side(target_text, target=True),
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
                    "match_percent": match_percent,
                    "instructions": side(candidate_text, target=False),
                },
            ]
        },
    }


def _context(reports: dict[str, dict[str, object]]) -> dict[str, object]:
    return {
        "schema": rules.SAVED_FPR_SEMANTIC_OWNER_CONTEXT_SCHEMA,
        "owner": "main:board/capevent",
        "function": FUNCTION,
        "source_owner_task": "e64e849c02704a8e9bcc9023f62c421e",
        "authority_advanced": False,
        "report": {
            "report_sha256": "4f" * 32,
            "base_source_sha256": "da" * 32,
            "target_object_sha256": "ef" * 32,
        },
        "provenance": {
            "graphify_report_location": "game/src/board/capevent.c:L4718",
            "narrow_verified_location": "src/board/capevent.c:L4628",
            "graphify_bound": True,
            "graft_ask_count": 1,
            "graft_status": "no_nodes",
            "narrow_named_file_verified": True,
            "broad_searches": 0,
        },
        "donors": [
            {
                "function": "mbev_CapEffSnowOMExec",
                "role": "post_call_scalar_copy_sine",
                "source_location": "src/board/capevent.c:L5597",
                "same_translation_unit": True,
                "strict_exact": True,
                "data_exact": True,
                "source_shape": "distinct_post_call_scalar_consumer",
            },
            {
                "function": "mbev_CapEffRingOMExec",
                "role": "post_call_scalar_copy_cosine",
                "source_location": "src/board/capevent.c:L5940",
                "same_translation_unit": True,
                "strict_exact": True,
                "data_exact": True,
                "source_shape": "distinct_post_call_scalar_consumer",
            },
            {
                "function": "mbev_CapColorLerp",
                "role": "integer_conversion_mask_255",
                "source_location": "src/board/capevent.c:L5463",
                "same_translation_unit": True,
                "strict_exact": True,
                "data_exact": True,
                "source_shape": "float_to_int_then_mask_255",
            },
        ],
        "baseline": {
            "target_size": 860,
            "candidate_size": 736,
            "target_frame": 0x120,
            "candidate_frame": 0xC0,
            "target_objdiff_relocation_records": 34,
            "candidate_objdiff_relocation_records": 34,
            "match_percent": 82.995346,
            "target_saved_fprs": [f"f{index}" for index in range(18, 32)],
            "candidate_saved_fprs": [f"f{index}" for index in range(24, 32)],
            "missing_semantic_owner_count": 6,
            "verified_physical_relocations": 34,
            "recommended_components": list(semantic_owner.SEMANTIC_OWNER_COMPONENTS),
        },
        "semantic_owner_stage": {
            "candidate_id": "masuhit003-exact-topology-owner-consumers",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["precursor"])),
            "target_size": 860,
            "candidate_size": 860,
            "target_frame": 0x120,
            "candidate_frame": 0x120,
            "target_objdiff_relocation_records": 34,
            "candidate_objdiff_relocation_records": 34,
            "match_percent": 99.62791,
            "verified_physical_relocations": 34,
            "residual_count": 13,
            "diff_kind": "DIFF_ARG_MISMATCH",
            "operation_order_exact": True,
            "cfg_calls_exact": True,
            "stack_homes_exact": True,
            "data_exact": True,
            "protected_siblings_preserved": True,
            "artifact": {
                "source_sha256": "c3" * 32,
                "object_sha256": "1f" * 32,
                "strict_report_sha256": "14" * 32,
                "candidate_record_sha256": "da" * 32,
            },
        },
        "chronology": {
            "declaration_order": list(semantic_owner.DECLARATION_ORDER),
            "target_fpr_map": dict(semantic_owner.TARGET_FPR_MAP),
            "all_owners_live": True,
            "unknown_owners_present": False,
        },
        "exact_result": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["exact"])),
            "target_size": 860,
            "candidate_size": 860,
            "physical_relocations": 34,
            "objdiff_relocation_records": 34,
            "match_percent": 100.0,
            "artifact": {
                "source_sha256": "40" * 32,
                "object_sha256": "e2" * 32,
                "strict_report_sha256": "a9" * 32,
                "candidate_record_sha256": "e0" * 32,
                "compile_attestation_sha256": "0d" * 32,
            },
        },
        "negative_controls": [
            {
                "candidate_id": "masuhit001-composed-owner-alpha",
                "axis": "typed_u8_alpha_local",
                "result": "saved_gpr_and_missing_mask",
                "measured": True,
                "match_percent": 94.05116,
            },
            {
                "candidate_id": "masuhit002-block-initializers",
                "axis": "block_scoped_initializers",
                "result": "topology_neutral",
                "measured": True,
                "match_percent": 94.2,
            },
        ],
        "telemetry": {
            "parent_active_seconds": 1090.504012,
            "helper_active_seconds_sum": 0.0,
            "team_active_seconds_sum": 1090.504012,
            "active_wall_union_seconds": 1090.504012,
            "heavy_seconds": 1.2,
            "candidate_count": 4,
            "tracer_runs": 0,
            "donor_searches": 1,
            "telemetry_complete": False,
            "eligible_for_measured_crack_per_hour": False,
            "no_imputation": True,
            "uncovered_start_utc": "2026-08-27T00:18:32.632966Z",
            "uncovered_end_utc": "2026-08-27T00:26:29.415343Z",
            "uncovered_seconds": 476.782377,
            "telemetry_sha256": "b7" * 32,
            "active_interval_log_sha256": "e8" * 32,
        },
        "forbidden_axes": list(semantic_owner.FORBIDDEN_AXES),
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]
        if item["rule_id"] == semantic_owner.RULE_ID
    )


class SavedFprSemanticOwnerChronologyTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reports = {stage: _report(stage) for stage in ("baseline", "precursor", "exact")}

    def _diagnose(
        self,
        stage: str,
        context: dict[str, object] | None = None,
    ) -> dict[str, object]:
        result = rules.diagnose_document(
            self.reports[stage],
            focus_symbol=FUNCTION,
            saved_fpr_semantic_owner_context=(
                context if context is not None else _context(self.reports)
            ),
        )
        return _evaluation(result)

    def test_baseline_ranks_complete_semantic_owner_family(self) -> None:
        evaluation = self._diagnose("baseline")
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        cell = evidence["recommended_cells"][0]
        self.assertEqual(cell["kind"], "complete_saved_fpr_semantic_owner_family")
        self.assertEqual(cell["components"], list(semantic_owner.SEMANTIC_OWNER_COMPONENTS))
        self.assertTrue(evidence["suppress_tracer"])
        self.assertFalse(evidence["telemetry"]["telemetry_complete"])
        self.assertFalse(evidence["telemetry"]["eligible_for_measured_crack_per_hour"])
        self.assertTrue(evidence["telemetry"]["no_imputation"])

    def test_precursor_ranks_only_exact_live_declaration_chronology(self) -> None:
        evaluation = self._diagnose("precursor")
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(len(evidence["residual_rows"]), 13)
        self.assertEqual(evidence["value_equivalent_sda_owner_rows"], 8)
        cell = evidence["recommended_cells"][0]
        self.assertEqual(cell["kind"], "exact_live_saved_fpr_declaration_chronology")
        self.assertEqual(cell["declaration_order"], list(semantic_owner.DECLARATION_ORDER))
        self.assertEqual(cell["target_fpr_map"]["sinWeight"], "f18")

    def test_exact_result_schedules_nothing_and_tolerates_owner_spelling(self) -> None:
        evaluation = self._diagnose("exact")
        self.assertFalse(evaluation["matched"])
        self.assertIn("already exact", evaluation["reason"])
        self.assertEqual(evaluation["evidence"]["value_equivalent_sda_owner_rows"], 8)

    def test_partial_or_non_fpr_cycle_fails_closed(self) -> None:
        for mutation in ("partial", "non_fpr"):
            report = copy.deepcopy(self.reports["precursor"])
            target_row = report["left"]["symbols"][1]["instructions"][ROWS[0]]
            candidate_row = report["right"]["symbols"][1]["instructions"][ROWS[0]]
            if mutation == "partial":
                candidate_row["instruction"]["formatted"] = target_row["instruction"]["formatted"]
                candidate_row.pop("diff_kind", None)
                target_row.pop("diff_kind", None)
            else:
                candidate_row["instruction"]["formatted"] = "mr r3, r4"
            context = _context(self.reports)
            context["semantic_owner_stage"]["objdiff_canonical_sha256"] = rules._sha256(
                rules._canonical(report)
            )
            evaluation = _evaluation(
                rules.diagnose_document(
                    report,
                    focus_symbol=FUNCTION,
                    saved_fpr_semantic_owner_context=context,
                )
            )
            self.assertFalse(evaluation["matched"], mutation)

    def test_provenance_donor_and_telemetry_drift_fail_closed(self) -> None:
        mutations = [
            lambda context: context["provenance"].__setitem__("graft_ask_count", 2),
            lambda context: context["provenance"].__setitem__("broad_searches", 1),
            lambda context: context["donors"][0].__setitem__("strict_exact", False),
            lambda context: context["chronology"].__setitem__("unknown_owners_present", True),
            lambda context: context["telemetry"].__setitem__(
                "eligible_for_measured_crack_per_hour", True
            ),
        ]
        for mutate in mutations:
            context = _context(self.reports)
            mutate(context)
            with self.assertRaises(rules.LearningInputError):
                rules.diagnose_document(
                    self.reports["precursor"],
                    focus_symbol=FUNCTION,
                    saved_fpr_semantic_owner_context=context,
                )

    def test_cli_accepts_authenticated_precursor_context(self) -> None:
        context = _context(self.reports)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(self.reports["precursor"]), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(
                    [
                        "--report",
                        str(report_path),
                        "--function",
                        FUNCTION,
                        "--saved-fpr-semantic-owner-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
