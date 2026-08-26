from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


def _instruction(
    address: int,
    formatted: str,
    *,
    diff_kind: str | None = None,
) -> dict[str, object]:
    row: dict[str, object] = {
        "instruction": {
            "address": str(address),
            "size": 4,
            "formatted": formatted,
        }
    }
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _report() -> dict[str, object]:
    target_text = [
        "stwu r1, -0x140(r1)",
        "addi r3, r1, 0xcc",
        "addi r4, r1, 0xd8",
        "addi r5, r1, 0xe4",
        "addi r6, r1, 0xf0",
        "addi r7, r1, 0xfc",
        "bl mbev_CapEffRayAdd",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -0x140(r1)",
        "addi r3, r1, 0xf0",
        "addi r4, r1, 0xcc",
        "addi r5, r1, 0xd8",
        "addi r6, r1, 0xe4",
        "addi r7, r1, 0xfc",
        "bl mbev_CapEffRayAdd",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target.append(_instruction(100 + 4 * index, left, diff_kind=kind))
        candidate.append(_instruction(100 + 4 * index, right, diff_kind=kind))
    return {
        "left": {
            "symbols": [
                {
                    "name": "ev_CapEffOpen",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "3836",
                    "match_percent": 99.899895,
                    "instructions": target,
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": "ev_CapEffOpen",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "3836",
                    "match_percent": 99.899895,
                    "instructions": candidate,
                }
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    return {
        "schema": rules.MIXED_BANK_HOME_CYCLE_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "cfg_calls_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "source_interface_authenticated": True,
            "pinned_mwcc_right_to_left": True,
            "stack_home_evidence_authenticated": True,
            "protected_siblings_preserved": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "precursor_source_sha256": "3" * 64,
            "precursor_object_sha256": "4" * 64,
            "precursor_record_sha256": "5" * 64,
            "interface_receipt_sha256": "6" * 64,
            "stack_home_receipt_sha256": "7" * 64,
            "exact_source_sha256": "8" * 64,
            "exact_object_sha256": "9" * 64,
            "exact_strict_report_sha256": "a" * 64,
            "exact_data_report_sha256": "b" * 64,
            "exact_record_sha256": "c" * 64,
            "report_artifact_sha256": "d" * 64,
        },
        "precursor": {
            "candidate_id": "capeffopen-v93",
            "target_bytes": 3836,
            "candidate_bytes": 3836,
            "target_frame": 0x140,
            "candidate_frame": 0x140,
            "match_percent": 99.899895,
            "physical_relocations": 236,
            "residual_rows": [1, 2, 3, 4],
        },
        "call_boundary": {
            "helper_symbol": "mbev_CapEffRayAdd",
            "source_expression": "mbev_CapEffRayAdd(obj, posP, color, scale, time)",
            "frontend_rule": "right_to_left",
            "abi_assignment_preserved": True,
            "arguments": [
                {
                    "identity": "scale",
                    "source_index": 0,
                    "type": "float",
                    "abi_bank": "fpr",
                    "live_expression": True,
                },
                {
                    "identity": "time",
                    "source_index": 1,
                    "type": "int",
                    "abi_bank": "gpr",
                    "live_expression": True,
                },
            ],
            "evaluation_order": ["time", "scale"],
        },
        "frozen_owners": [
            {
                "identity": "pos",
                "type": "HuVecF",
                "size": 12,
                "target_home": 0xFC,
                "candidate_home": 0xFC,
                "exact": True,
            }
        ],
        "owner_cycle": {
            "type": "HuVecF",
            "size": 12,
            "owners": [
                {
                    "identity": "particlePos",
                    "type": "HuVecF",
                    "size": 12,
                    "target_home": 0xCC,
                    "candidate_home": 0xF0,
                    "exact": False,
                },
                {
                    "identity": "scale",
                    "type": "HuVecF",
                    "size": 12,
                    "target_home": 0xD8,
                    "candidate_home": 0xCC,
                    "exact": False,
                },
                {
                    "identity": "vel",
                    "type": "HuVecF",
                    "size": 12,
                    "target_home": 0xE4,
                    "candidate_home": 0xD8,
                    "exact": False,
                },
                {
                    "identity": "rot",
                    "type": "HuVecF",
                    "size": 12,
                    "target_home": 0xF0,
                    "candidate_home": 0xE4,
                    "exact": False,
                },
            ],
            "declaration_order": ["pos", "particlePos", "rot", "vel", "scale"],
        },
        "exact_result": {
            "candidate_id": "capeffopen-exact",
            "target_bytes": 3836,
            "candidate_bytes": 3836,
            "physical_relocations": 236,
            "source_sha256": "8" * 64,
            "object_sha256": "9" * 64,
            "strict_report_sha256": "a" * 64,
            "data_report_sha256": "b" * 64,
            "candidate_record_sha256": "c" * 64,
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "mixed_bank_argument_aggregate_home_cycle"
    )


class MixedBankHomeCycleTests(unittest.TestCase):
    def test_matches_mixed_bank_seam_and_unique_frozen_cycle(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol="ev_CapEffOpen",
            mixed_bank_home_cycle_context=_context(report),
        )
        diagnosis = _evaluation(result)

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["source_class"],
            "mixed_bank_direct_call_plus_unique_typed_aggregate_home_cycle",
        )
        evidence = diagnosis["evidence"]
        self.assertEqual(
            evidence["mixed_bank_call"]["evaluation_order"],  # type: ignore[index]
            ["time", "scale"],
        )
        self.assertEqual(
            evidence["frozen_owners"][0]["identity"],  # type: ignore[index]
            "pos",
        )
        self.assertEqual(
            evidence["owner_cycle"]["home_mapping"],  # type: ignore[index]
            {
                "particlePos": "scale",
                "scale": "vel",
                "vel": "rot",
                "rot": "particlePos",
            },
        )
        self.assertEqual(
            evidence["recommended_cells"][1]["declaration_order"],  # type: ignore[index]
            ["pos", "particlePos", "rot", "vel", "scale"],
        )
        self.assertIn("moving_frozen_owners", evidence["suppressed_axes"])  # type: ignore[operator]
        self.assertFalse(result["authority_advanced"])

    def test_fails_closed_without_authenticated_context(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol="ev_CapEffOpen")
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_unsafe_or_ambiguous_evidence(self) -> None:
        report = _report()
        mutations: list[tuple[str, callable]] = [
            (
                "left_to_right",
                lambda value: value["call_boundary"].__setitem__(  # type: ignore[union-attr]
                    "evaluation_order", ["scale", "time"]
                ),
            ),
            (
                "same_bank",
                lambda value: value["call_boundary"]["arguments"][0].__setitem__(  # type: ignore[index,union-attr]
                    "abi_bank", "gpr"
                ),
            ),
            (
                "false_proof",
                lambda value: value["proofs"].__setitem__(  # type: ignore[union-attr]
                    "pinned_mwcc_right_to_left", False
                ),
            ),
            (
                "moving_frozen",
                lambda value: value["frozen_owners"][0].__setitem__(  # type: ignore[index,union-attr]
                    "candidate_home", 0x108
                ),
            ),
            (
                "overlap",
                lambda value: value["owner_cycle"]["owners"][1].__setitem__(  # type: ignore[index,union-attr]
                    "target_home", 0xCC
                ),
            ),
        ]
        for name, mutate in mutations:
            unsafe = _context(report)
            mutate(unsafe)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol="ev_CapEffOpen",
                        mixed_bank_home_cycle_context=unsafe,
                    )

    def test_report_rejects_non_arg_and_frozen_owner_residuals(self) -> None:
        non_arg = _report()
        non_arg["left"]["symbols"][0]["instructions"][1]["diff_kind"] = "DIFF_OPCODE_MISMATCH"  # type: ignore[index]
        non_arg["right"]["symbols"][0]["instructions"][1]["diff_kind"] = "DIFF_OPCODE_MISMATCH"  # type: ignore[index]
        result = rules.diagnose_document(
            non_arg,
            focus_symbol="ev_CapEffOpen",
            mixed_bank_home_cycle_context=_context(non_arg),
        )
        self.assertFalse(_evaluation(result)["matched"])

        moving_frozen = _report()
        moving_frozen["left"]["symbols"][0]["instructions"][1]["instruction"]["formatted"] = "addi r3, r1, 0xfc"  # type: ignore[index]
        result = rules.diagnose_document(
            moving_frozen,
            focus_symbol="ev_CapEffOpen",
            mixed_bank_home_cycle_context=_context(moving_frozen),
        )
        diagnosis = _evaluation(result)
        self.assertFalse(diagnosis["matched"])
        self.assertIn("frozen", diagnosis["reason"])

    def test_cli_emits_same_document(self) -> None:
        report = _report()
        context = _context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "ev_CapEffOpen",
                            "--mixed-bank-home-cycle-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="ev_CapEffOpen",
                mixed_bank_home_cycle_context=context,
            ),
        )


if __name__ == "__main__":
    unittest.main()
