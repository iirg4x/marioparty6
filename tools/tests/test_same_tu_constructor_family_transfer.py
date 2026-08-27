from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


FUNCTION = "mbev_CapEffExplodeCircleAdd"


def _instruction(address: int, formatted: str, *, mismatch: bool = False, relocation: bool = False) -> dict[str, object]:
    instruction: dict[str, object] = {"address": str(address), "size": 4, "formatted": formatted}
    if relocation:
        instruction["relocation"] = {"type": 10, "type_name": "R_PPC_REL24", "target_symbol": 1}
    row: dict[str, object] = {"instruction": instruction}
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report(stage: str) -> dict[str, object]:
    facts = {
        "baseline": (1376, 1264, 0x1E0, 0x180, 54, 52, 85.049416),
        "c001": (1376, 1360, 0x1E0, 0x1D0, 54, 54, 97.816864),
        "c002": (1376, 1376, 0x1E0, 0x1E0, 54, 54, 99.76744),
        "exact": (1376, 1376, 0x1E0, 0x1E0, 54, 54, 100.0),
    }[stage]
    target_size, candidate_size, target_frame, candidate_frame, target_reloc, candidate_reloc, score = facts
    target_count = target_size // 4
    candidate_count = candidate_size // 4
    target_text = ["nop"] * target_count
    candidate_text = ["nop"] * candidate_count
    target_text[0] = f"stwu r1, -0x{target_frame:X}(r1)"
    candidate_text[0] = f"stwu r1, -0x{candidate_frame:X}(r1)"
    mismatch_rows: set[int] = set()
    if stage == "c002":
        mismatch_rows = set(range(20, 36))
        for index in mismatch_rows:
            target_text[index] = f"fmuls f{21 + (index % 7)}, f1, f2"
            candidate_text[index] = f"fmuls f{14 + (index % 7)}, f1, f2"
    elif stage != "exact":
        mismatch_rows = set(range(1, min(10, candidate_count)))
        for index in mismatch_rows:
            target_text[index] = "fmuls f25, f1, f2"
            candidate_text[index] = "fmuls f20, f1, f2"

    def side(text: list[str], relocations: int) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=index in mismatch_rows,
                relocation=index < relocations,
            )
            for index, formatted in enumerate(text)
        ]

    return {
        "left": {"symbols": [{"name": "pool", "kind": "SYMBOL_DATA"}, {"name": FUNCTION, "kind": "SYMBOL_FUNCTION", "address": "0x100", "size": str(target_size), "match_percent": score, "instructions": side(target_text, target_reloc)}]},
        "right": {"symbols": [{"name": "pool", "kind": "SYMBOL_DATA"}, {"name": FUNCTION, "kind": "SYMBOL_FUNCTION", "address": "0x100", "size": str(candidate_size), "match_percent": score, "instructions": side(candidate_text, candidate_reloc)}]},
    }


def _context(reports: dict[str, dict[str, object]]) -> dict[str, object]:
    return {
        "schema": rules.SAME_TU_CONSTRUCTOR_FAMILY_CONTEXT_SCHEMA,
        "owner": "main:board/capevent",
        "function": FUNCTION,
        "authority_advanced": False,
        "report": {"report_sha256": "47" * 32, "base_commit": "28" * 32, "target_object_sha256": "ef" * 32},
        "provenance": {"graphify_location": "game/src/board/capevent.c:L3541", "graphify_bound": True, "graft_ask_count": 1, "graft_status": "no_nodes", "narrow_named_file_verified": True},
        "donor": {"function": "mbev_CapEffDustExplodeAdd", "same_translation_unit": True, "strict_exact": True, "data_exact": True, "target_size": 1492, "candidate_size": 1492, "source_sha256": "8b" * 32, "object_sha256": "89" * 32, "strict_report_sha256": "8d" * 32, "family_components": ["named_scalar_trig_results", "per_call_typed_aggregate_snapshots", "typed_pointer_address_consumers", "live_integer_result_stores"]},
        "baseline": {"target_size": 1376, "candidate_size": 1264, "target_frame": 0x1E0, "candidate_frame": 0x180, "target_relocations": 54, "candidate_relocations": 52, "match_percent": 85.049416},
        "stages": {
            "donor_family": {"objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["c001"])), "candidate_size": 1360, "candidate_frame": 0x1D0, "physical_relocations": 54, "match_percent": 97.816864, "artifact": {"source_sha256": "a3" * 32, "object_sha256": "1f" * 32, "attestation_sha256": "29" * 32}, "cell": ["named_scalar_trig_results", "per_call_typed_aggregate_snapshots", "typed_pointer_address_consumers", "live_integer_result_stores"]},
            "semantic_order": {"objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["c002"])), "candidate_size": 1376, "candidate_frame": 0x1E0, "physical_relocations": 54, "match_percent": 99.76744, "artifact": {"source_sha256": "b7" * 32, "object_sha256": "5c" * 32, "attestation_sha256": "37" * 32}, "cell": ["distinct_shared_rand_owner", "aggregate_assignment_vel_before_pos", "direction_store_x_z_y"]},
            "donor_chronology": {"objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["c002"])), "residual_count": 16, "diff_kind": "DIFF_ARG_MISMATCH", "declaration_order": ["distance", "posCos", "posSin", "velCos", "velSin", "value", "active", "fadeStep", "halfDistance", "randF"], "target_fpr_map": {"distance": "f25", "posCos": "f24", "posSin": "f23", "velCos": "f22", "velSin": "f21", "active": "f27", "fadeStep": "f26"}},
            "exact": {"objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["exact"])), "target_size": 1376, "candidate_size": 1376, "physical_relocations": 54, "match_percent": 100.0, "artifact": {"source_sha256": "8b" * 32, "object_sha256": "89" * 32, "attestation_sha256": "1f" * 32, "strict_report_sha256": "8d" * 32, "data_report_sha256": "8d" * 32}, "candidate_record_sha256": "61" * 32},
        },
        "telemetry": {"parent_active_seconds": 925.205318, "helper_active_seconds_sum": 0.0, "team_active_seconds_sum": 925.205318, "active_wall_union_seconds": 925.205318, "heavy_seconds": 1.8, "candidate_count": 3, "tracer_runs": 0, "donor_searches": 1, "telemetry_complete": True, "eligible_for_measured_crack_per_hour": True, "no_imputation": True, "telemetry_sha256": "c1" * 32, "active_interval_log_sha256": "fc" * 32},
        "forbidden_axes": ["isolated_declaration_permutations_before_structure", "scope_permutations", "dead_or_fake_locals", "padding", "register_shaping", "unauthenticated_donor", "tracer_before_static_closure", "source_retention", "promotion"],
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(item for item in result["evaluations"] if item["rule_id"] == "same_tu_constructor_family_transfer")


class SameTuConstructorFamilyTransferTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reports = {name: _report(name) for name in ("baseline", "c001", "c002", "exact")}

    def _diagnose(self, stage: str, context: dict[str, object] | None = None) -> dict[str, object]:
        result = rules.diagnose_document(self.reports[stage], focus_symbol=FUNCTION, same_tu_constructor_family_context=context if context is not None else _context(self.reports))
        return _evaluation(result)

    def test_baseline_ranks_complete_family_before_local_permutations(self) -> None:
        evaluation = self._diagnose("baseline")
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(evidence["stage"], "baseline_to_donor_family")
        self.assertEqual(len(evidence["recommended_cells"]), 1)
        self.assertTrue(evidence["suppress_tracer"])
        self.assertFalse(evidence["authority_advanced"])

    def test_c001_ranks_only_shared_random_and_order_closure(self) -> None:
        evaluation = self._diagnose("c001")
        self.assertTrue(evaluation["matched"])
        cell = evaluation["evidence"]["recommended_cells"][0]
        self.assertEqual(cell["kind"], "constructor_family_semantic_order_closure")
        self.assertEqual(len(cell["components"]), 3)

    def test_c002_ranks_only_exact_donor_declaration_chronology(self) -> None:
        evaluation = self._diagnose("c002")
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(len(evidence["residual_rows"]), 16)
        cell = evidence["recommended_cells"][0]
        self.assertEqual(cell["kind"], "exact_donor_scalar_declaration_chronology")
        self.assertEqual(cell["target_fpr_map"]["distance"], "f25")

    def test_c002_ignores_only_value_equivalent_sda_owner_alias_rows(self) -> None:
        report = copy.deepcopy(self.reports["c002"])
        report["left"]["symbols"][1]["instructions"][40]["instruction"]["formatted"] = (
            "lfs f1, lbl_802C46DC@sda21"
        )
        report["right"]["symbols"][1]["instructions"][40]["instruction"]["formatted"] = (
            "lfs f1, @969@sda21"
        )
        report["right"]["symbols"][1]["instructions"][40]["diff_kind"] = "DIFF_ARG_MISMATCH"
        context = _context(self.reports)
        context["stages"]["semantic_order"]["objdiff_canonical_sha256"] = rules._sha256(
            rules._canonical(report)
        )
        evaluation = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol=FUNCTION,
                same_tu_constructor_family_context=context,
            )
        )
        self.assertTrue(evaluation["matched"])
        self.assertEqual(evaluation["evidence"]["value_equivalent_sda_owner_rows"], 1)
        self.assertEqual(len(evaluation["evidence"]["residual_rows"]), 16)


    def test_exact_result_schedules_nothing(self) -> None:
        self.assertFalse(self._diagnose("exact")["matched"])

    def test_exact_result_tolerates_only_value_equivalent_sda_owner_aliases(self) -> None:
        report = copy.deepcopy(self.reports["exact"])
        report["left"]["symbols"][1]["instructions"][40]["instruction"]["formatted"] = (
            "lfs f1, lbl_802C46DC@sda21"
        )
        report["right"]["symbols"][1]["instructions"][40]["instruction"]["formatted"] = (
            "lfs f1, @969@sda21"
        )
        context = _context(self.reports)
        context["stages"]["exact"]["objdiff_canonical_sha256"] = rules._sha256(
            rules._canonical(report)
        )
        evaluation = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol=FUNCTION,
                same_tu_constructor_family_context=context,
            )
        )
        self.assertFalse(evaluation["matched"])
        self.assertIn("already exact", evaluation["reason"])
        self.assertEqual(evaluation["evidence"]["value_equivalent_sda_owner_rows"], 1)

    def test_missing_or_drifted_context_fails_closed(self) -> None:
        result = rules.diagnose_document(self.reports["baseline"], focus_symbol=FUNCTION)
        self.assertFalse(_evaluation(result)["matched"])
        mutations = [
            lambda value: value["donor"].__setitem__("strict_exact", False),
            lambda value: value["provenance"].__setitem__("graphify_bound", False),
            lambda value: value["telemetry"].__setitem__("telemetry_complete", False),
            lambda value: value.__setitem__("authority_advanced", True),
            lambda value: value["forbidden_axes"].pop(),
        ]
        for mutate in mutations:
            context = _context(self.reports)
            mutate(context)
            with self.assertRaises(rules.LearningInputError):
                rules.diagnose_document(self.reports["baseline"], focus_symbol=FUNCTION, same_tu_constructor_family_context=context)

    def test_non_fpr_cycle_or_report_hash_drift_fails_closed(self) -> None:
        report = copy.deepcopy(self.reports["c002"])
        context = _context(self.reports)
        report["right"]["symbols"][1]["instructions"][20]["instruction"]["formatted"] = "mr r3, r4"
        context["stages"]["semantic_order"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
        evaluation = _evaluation(rules.diagnose_document(report, focus_symbol=FUNCTION, same_tu_constructor_family_context=context))
        self.assertFalse(evaluation["matched"])

    def test_cli_accepts_authenticated_c002_context(self) -> None:
        context = _context(self.reports)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(self.reports["c002"]), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main(["--report", str(report_path), "--function", FUNCTION, "--same-tu-constructor-family-context", str(context_path)])
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
