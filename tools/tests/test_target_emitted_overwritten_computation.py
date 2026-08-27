from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import target_emitted_overwritten_computation as overwritten


def _instruction(
    address: int,
    formatted: str,
    *,
    mismatch: bool = False,
    relocation: bool = False,
) -> dict[str, object]:
    nested: dict[str, object] = {
        "address": str(address),
        "size": 4,
        "formatted": formatted,
    }
    if relocation:
        nested["relocation"] = {
            "type": 109,
            "type_name": "R_PPC_EMB_SDA21",
            "target_symbol": 1,
        }
    row: dict[str, object] = {"instruction": nested}
    if mismatch:
        row["diff_kind"] = "DIFF_DELETE"
    return row


def _placeholder() -> dict[str, object]:
    return {"diff_kind": "DIFF_DELETE"}


def _function(*, target: bool, exact: bool) -> dict[str, object]:
    count = 695
    text = ["nop"] * count
    text[607] = "fdivs f30, f1, f0"
    chain = (
        "lfd f1, lbl_802C42B0@sda21",
        "lfs f0, lbl_802C4348@sda21",
        "fmuls f0, f0, f30",
        "fmul f1, f1, f0",
        "lfd f0, lbl_802C42A8@sda21",
        "fdiv f1, f1, f0",
        "bl cos",
        "frsp f31, f1",
    )
    text[608:616] = chain
    text[616] = "lfs f0, lbl_802C42C8@sda21"
    instructions: list[dict[str, object]] = []
    for index, formatted in enumerate(text):
        if not exact and not target and 608 <= index <= 615:
            instructions.append(_placeholder())
            continue
        instructions.append(
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=not exact and target and 608 <= index <= 615,
                relocation=index < 172,
            )
        )
    return {
        "name": overwritten.FUNCTION,
        "kind": "SYMBOL_FUNCTION",
        "address": "0x100",
        "size": str(2780 if target or exact else 2748),
        "match_percent": 100.0 if exact else 98.84892,
        "instructions": instructions,
    }


def _report(exact: bool) -> dict[str, object]:
    return {
        "left": {
            "symbols": [
                {"name": "pool", "kind": "SYMBOL_DATA"},
                _function(target=True, exact=exact),
            ]
        },
        "right": {
            "symbols": [
                {"name": "pool", "kind": "SYMBOL_DATA"},
                _function(target=False, exact=exact),
            ]
        },
    }


def _context(reports: dict[str, dict[str, object]]) -> dict[str, object]:
    return {
        "schema": rules.TARGET_EMITTED_OVERWRITTEN_CONTEXT_SCHEMA,
        "owner": "main:board/capspecial",
        "function": overwritten.FUNCTION,
        "source_owner_task": "c01b2b0aef2c436ea19d2f398489f9d0",
        "authority_advanced": False,
        "report_sha256": overwritten.REPORT_SHA256,
        "toolchain": {
            "target_object_sha256": overwritten.TARGET_OBJECT_SHA256,
            "compile_context_authenticated": True,
        },
        "provenance": {
            "graphify_status": "no_graph",
            "graft_ask_count": 1,
            "graft_status": "no_relevant_nodes",
            "narrow_named_file_verified": True,
            "broad_searches": 0,
        },
        "program_point": {
            "switch_case": 64,
            "source_line": 2910,
            "after_statement": "time = (float)work->time / 60.0f;",
            "before_condition": "if (time < 1.0f)",
            "destination": "scale",
            "input": "time",
            "source_statement": overwritten.SOURCE_STATEMENT,
            "result_read_before_overwrite": False,
            "later_source_consumer_exists": True,
            "overwritten_before_later_consumer": True,
        },
        "target_chain": {
            "row_start": 608,
            "row_end": 615,
            "mnemonics": list(overwritten.TARGET_CHAIN),
            "call": "cos",
            "instruction_count": 8,
            "byte_delta": 32,
            "preceding_instruction": "fdivs f30, f1, f0",
            "following_instruction": "lfs f0, lbl_802C42C8@sda21",
        },
        "baseline": {
            "candidate_id": "c704-koopastart-boardno-branch-local",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["baseline"])),
            "source_sha256": overwritten.BASELINE_SOURCE_SHA256,
            "object_sha256": "70" * 32,
            "strict_report_sha256": "71" * 32,
            "target_size": 2780,
            "candidate_size": 2748,
            "strict_match_percent": 98.84892,
        },
        "exact_result": {
            "candidate_id": "c714-sprupdate-retail-cosine-chronology",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(reports["exact"])),
            **overwritten.EXACT_HASHES,
            "target_size": 2780,
            "candidate_size": 2780,
            "physical_relocations": 172,
            "protected_siblings": "37/37",
            "owner_exact_frontier": "38/44",
        },
        "admissibility": {
            "owner_decision": "retained",
            "retained_record_bound": True,
            "policy_correction_supersedes_quarantine": True,
            "independent_test_pass": True,
            "exact_bytes_alone_sufficient": False,
            "blanket_dead_assignment_waiver": False,
            "requires_owner_local_review": True,
        },
        "telemetry": {
            "mixed_parent_interval_seconds": 7365.1560854,
            "complete_decision_interval_seconds": 978.2983101,
            "compile_heavy_seconds": 0.2821299,
            "telemetry_complete": False,
            "crack_hour_eligible": False,
            "no_imputation": True,
            "telemetry_receipt_sha256": overwritten.TELEMETRY_SHA256,
            "active_interval_prefix_sha256": overwritten.INTERVAL_PREFIX_SHA256,
        },
        "forbidden_axes": list(overwritten.FORBIDDEN_AXES),
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item for item in result["evaluations"]
        if item["rule_id"] == overwritten.RULE_ID
    )


class TargetEmittedOverwrittenComputationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.reports = {"baseline": _report(False), "exact": _report(True)}

    def test_baseline_ranks_only_retained_computation(self) -> None:
        result = rules.diagnose_document(
            self.reports["baseline"],
            focus_symbol=overwritten.FUNCTION,
            target_emitted_overwritten_context=_context(self.reports),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        evidence = evaluation["evidence"]
        self.assertEqual(
            evidence["recommended_cells"],
            [{
                "kind": "restore_owner_retained_target_emitted_computation",
                "source_statement": overwritten.SOURCE_STATEMENT,
                "after_statement": "time = (float)work->time / 60.0f;",
                "before_condition": "if (time < 1.0f)",
            }],
        )
        self.assertTrue(evidence["requires_owner_admissibility_record"])
        self.assertFalse(evidence["blanket_dead_assignment_waiver"])
        self.assertFalse(evidence["telemetry"]["telemetry_complete"])
        self.assertFalse(evidence["telemetry"]["crack_hour_eligible"])

    def test_exact_pair_schedules_nothing(self) -> None:
        evaluation = _evaluation(
            rules.diagnose_document(
                self.reports["exact"],
                focus_symbol=overwritten.FUNCTION,
                target_emitted_overwritten_context=_context(self.reports),
            )
        )
        self.assertFalse(evaluation["matched"])
        self.assertIn("already exact", evaluation["reason"])

    def test_chain_call_or_anchor_drift_fails_closed(self) -> None:
        for row, replacement in (
            (614, "bl sin"),
            (607, "fdivs f29, f1, f0"),
            (616, "lfs f1, lbl_802C42C8@sda21"),
        ):
            report = copy.deepcopy(self.reports["baseline"])
            report["left"]["symbols"][1]["instructions"][row]["instruction"]["formatted"] = replacement
            if row not in range(608, 616):
                report["right"]["symbols"][1]["instructions"][row]["instruction"]["formatted"] = replacement
            context = _context(self.reports)
            context["baseline"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
            evaluation = _evaluation(
                rules.diagnose_document(
                    report,
                    focus_symbol=overwritten.FUNCTION,
                    target_emitted_overwritten_context=context,
                )
            )
            self.assertFalse(evaluation["matched"])

    def test_admissibility_provenance_and_telemetry_drift_fail_closed(self) -> None:
        mutations = (
            lambda context: context["admissibility"].__setitem__("blanket_dead_assignment_waiver", True),
            lambda context: context["admissibility"].__setitem__("owner_decision", "quarantined"),
            lambda context: context["provenance"].__setitem__("graft_ask_count", 2),
            lambda context: context["telemetry"].__setitem__("crack_hour_eligible", True),
            lambda context: context["telemetry"].__setitem__("no_imputation", False),
        )
        for mutate in mutations:
            context = _context(self.reports)
            mutate(context)
            with self.assertRaises(rules.LearningInputError):
                rules.diagnose_document(
                    self.reports["baseline"],
                    focus_symbol=overwritten.FUNCTION,
                    target_emitted_overwritten_context=context,
                )

    def test_output_binds_context_and_implementation(self) -> None:
        result = rules.diagnose_document(
            self.reports["baseline"],
            focus_symbol=overwritten.FUNCTION,
            target_emitted_overwritten_context=_context(self.reports),
        )
        self.assertIsNotNone(
            result["inputs"]["target_emitted_overwritten_context_canonical_sha256"]
        )
        implementation = result["implementations"]["target_emitted_overwritten_computation"]
        self.assertEqual(implementation["schema"], overwritten.CONTEXT_SCHEMA)
        self.assertRegex(implementation["sha256"], r"^[0-9a-f]{64}$")

    def test_cli_accepts_authenticated_context(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(self.reports["baseline"]), encoding="utf-8")
            context_path.write_text(json.dumps(_context(self.reports)), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                exit_code = rules.main([
                    "--report", str(report_path),
                    "--function", overwritten.FUNCTION,
                    "--target-emitted-overwritten-context", str(context_path),
                ])
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
