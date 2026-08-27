from __future__ import annotations

import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import single_use_final_call_consumer as direct_consumer


FUNCTION = "ev_CapEffKillerBoostCreate"
RESIDUAL_ROWS = [2, 5, 6, 10]
TARGET_FORMS = [
    "mr r26, r3",
    "lwz r25, 0xcc(r1)",
    "lwz r3, 0xbd4(r26)",
    "mr r6, r25",
]
CANDIDATE_FORMS = [
    "mr r25, r3",
    "lwz r26, 0xcc(r1)",
    "lwz r3, 0xbd4(r25)",
    "mr r6, r26",
]


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
            "type": 10,
            "type_name": "R_PPC_REL24",
            "target_symbol": 1,
        }
    row: dict[str, object] = {"instruction": nested}
    if mismatch:
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report() -> dict[str, object]:
    target_text = [
        "stwu r1, -0x120(r1)",
        "bl _savegpr_25",
        TARGET_FORMS[0],
        "fctiwz f0, f0",
        "stfd f0, 0xc8(r1)",
        TARGET_FORMS[1],
        TARGET_FORMS[2],
        "mr r4, r27",
        "mr r5, r28",
        "fmr f2, f30",
        TARGET_FORMS[3],
        "mr r7, r29",
        "bl mbev_CapEffBoostAdd",
        "bl _restgpr_25",
        "blr",
    ]
    candidate_text = list(target_text)
    for row, form in zip(RESIDUAL_ROWS, CANDIDATE_FORMS):
        candidate_text[row] = form

    def side(text: list[str]) -> list[dict[str, object]]:
        return [
            _instruction(
                0x100 + index * 4,
                formatted,
                mismatch=index in RESIDUAL_ROWS,
                relocation=index in {1, 12, 13},
            )
            for index, formatted in enumerate(text)
        ]

    def function(text: list[str]) -> dict[str, object]:
        return {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "address": "0x100",
            "size": "1292",
            "match_percent": 99.93808,
            "instructions": side(text),
        }

    return {
        "left": {"symbols": [function(target_text)]},
        "right": {"symbols": [function(candidate_text)]},
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    return {
        "schema": direct_consumer.CONTEXT_SCHEMA,
        "report_artifact_sha256": "70" * 32,
        "precursor": {
            "function": FUNCTION,
            "candidate_id": "killerboost-baseline",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "source_sha256": "63" * 32,
            "object_sha256": "59" * 32,
            "strict_report_sha256": "e0" * 32,
            "data_report_sha256": "97" * 32,
            "target_bytes": 1292,
            "candidate_bytes": 1292,
            "target_frame": 0x120,
            "candidate_frame": 0x120,
            "match_percent": 99.93808,
            "target_physical_relocations": 3,
            "candidate_physical_relocations": 3,
            "residual_pairs": [
                {"row": row, "target": left, "candidate": right}
                for row, left, right in zip(
                    RESIDUAL_ROWS, TARGET_FORMS, CANDIDATE_FORMS
                )
            ],
            "operation_order_exact": True,
            "cfg_calls_exact": True,
            "data_exact": True,
            "stack_homes_exact": True,
            "physical_relocations_exact": True,
            "protected_siblings_preserved": True,
        },
        "owners": {
            "long_lived": {
                "name": "work",
                "source_role": "function_parameter",
                "target_register": "r26",
                "candidate_register": "r25",
                "capture_row": 2,
                "final_use_row": 6,
                "evidence_sha256": "cf" * 32,
            },
            "single_use": {
                "name": "time",
                "source_role": "single_use_scalar_conversion",
                "target_register": "r25",
                "candidate_register": "r26",
                "conversion_load_row": 5,
                "final_argument_row": 10,
                "assignment_count": 1,
                "consumer_count": 1,
                "source_expression": "(int)(60.0f * (1.0f + (0.5f * MBCapsuleEffRandF())))",
                "evidence_sha256": "cf" * 32,
            },
            "unaffected_final_arguments": [
                {
                    "name": "colorP",
                    "register": "r29",
                    "abi_register": "r7",
                    "argument_row": 11,
                    "evidence_sha256": "cf" * 32,
                }
            ],
        },
        "final_call": {
            "symbol": "mbev_CapEffBoostAdd",
            "call_row": 12,
            "integer_argument_index": 5,
            "abi_register": "r6",
            "conversion_rows": [3, 4, 5],
            "source_template": "mbev_CapEffBoostAdd(..., (int)(...), colorP)",
            "typed_consumer_proven": True,
            "call_count_exact": True,
            "call_order_exact": True,
            "evidence_sha256": "cf" * 32,
        },
        "negative_controls": [
            {
                "axis": "declaration_chronology",
                "outcome": "object_identical",
                "candidate_id": "declaration-only",
                "target_bytes": 1292,
                "candidate_bytes": 1292,
                "match_percent": 99.93808,
                "evidence_sha256": "58" * 32,
            },
            {
                "axis": "unrelated_pointer_birth",
                "outcome": "regressed_topology",
                "candidate_id": "boost001",
                "target_bytes": 1292,
                "candidate_bytes": 1292,
                "match_percent": 99.318886,
                "evidence_sha256": "35" * 32,
            },
            {
                "axis": "unrelated_pointer_final_consumer",
                "outcome": "regressed_topology",
                "candidate_id": "boost002",
                "target_bytes": 1292,
                "candidate_bytes": 1292,
                "match_percent": 98.49845,
                "evidence_sha256": "da" * 32,
            },
            {
                "axis": "assignment_expression",
                "outcome": "grew_function",
                "candidate_id": "boost003",
                "target_bytes": 1292,
                "candidate_bytes": 1296,
                "match_percent": 99.56347,
                "evidence_sha256": "43" * 32,
            },
        ],
        "exact_result": {
            "candidate_id": "boost005-direct-time-expression",
            "target_bytes": 1292,
            "candidate_bytes": 1292,
            "physical_relocations": 3,
            "source_sha256": "3b" * 32,
            "object_sha256": "db" * 32,
            "strict_report_sha256": "b6" * 32,
            "data_report_sha256": "58" * 32,
            "candidate_record_sha256": "2c" * 32,
        },
        "telemetry": {
            "candidate_count": 5,
            "tracer_runs": 0,
            "donor_searches": 1,
            "telemetry_complete": False,
            "interval_log_sha256": "c6" * 32,
        },
        "authority_advanced": False,
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]
        if item["rule_id"] == direct_consumer.RULE_ID
    )


class SingleUseFinalCallConsumerTests(unittest.TestCase):
    def test_emits_only_direct_consumption_cell(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            single_use_final_call_context=_context(report),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        cells = evaluation["evidence"]["recommended_cells"]
        self.assertEqual(len(cells), 1)
        self.assertEqual(cells[0]["kind"], "direct_single_use_scalar_consumption")
        self.assertIn("named_local_assignment_expression", evaluation["evidence"]["suppressed_axes"])
        self.assertFalse(evaluation["evidence"]["authority_advanced"])

    def test_missing_context_fails_closed(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol=FUNCTION)
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_proof_drift_is_rejected(self) -> None:
        mutations = {
            "single_use": lambda value: value["owners"]["single_use"].__setitem__("consumer_count", 2),
            "assignment_control": lambda value: value["negative_controls"][3].__setitem__("candidate_bytes", 1292),
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
                        single_use_final_call_context=context,
                    )

    def test_call_or_unaffected_owner_drift_fails_closed(self) -> None:
        for name, row, form in (
            ("call", 12, "bl other_helper"),
            ("unaffected", 11, "mr r7, r28"),
        ):
            report = _report()
            context = _context(report)
            for side in ("left", "right"):
                report[side]["symbols"][0]["instructions"][row]["instruction"]["formatted"] = form
            context["precursor"]["objdiff_canonical_sha256"] = rules._sha256(
                rules._canonical(report)
            )
            with self.subTest(name=name):
                result = rules.diagnose_document(
                    report,
                    focus_symbol=FUNCTION,
                    single_use_final_call_context=context,
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
                        "--single-use-final-call-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
