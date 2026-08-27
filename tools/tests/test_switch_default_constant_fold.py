#!/usr/bin/env python3

from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import switch_default_constant_fold as switch_fold


FUNCTION = "ev_CapHanachanOMExec"
POOL_ROW = 191
TERMINAL_ROW = 204


def _instruction(
    address: int,
    formatted: str | None,
    *,
    diff_kind: str | None = None,
    relocation: bool = False,
) -> dict[str, object]:
    row: dict[str, object] = {}
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    if formatted is None:
        return row
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
    row["instruction"] = nested
    return row


def _report() -> dict[str, object]:
    target_text = ["nop"] * 216
    candidate_text: list[str | None] = ["nop"] * 216
    shared = {
        0: "stwu r1, -0x100(r1)",
        18: "lwz r0, 0x4c(r30)",
        19: "cmpwi r0, 0x0",
        22: "lwz r3, 0x50(r30)",
        193: "bl mbev_CapEffGlowAdd",
        201: "lwz r3, mbObjMan@sda21",
        202: "mr r4, r30",
        203: "bl omDelObjEx",
        205: "psq_l f31, 0xf8(r1), 0, qr0",
        215: "blr",
    }
    for row, form in shared.items():
        target_text[row] = form
        candidate_text[row] = form
    target_text[17], candidate_text[17] = "b 0x3e5c", "b 0x3d88"
    target_text[20], candidate_text[20] = "beq 0x3b80", "beq 0x3ab0"
    target_text[21], candidate_text[21] = "b 0x3e4c", "b 0x3d7c"
    target_text[POOL_ROW] = "lfs f3, lbl_802C3E50@sda21"
    candidate_text[POOL_ROW] = "lfs f3, @723@sda21"
    target_text[200], candidate_text[200] = "b 0x3e5c", "b 0x3d88"
    target_text[TERMINAL_ROW] = "b 0x3e5c"
    candidate_text[TERMINAL_ROW] = None
    relocation_rows = set(range(53)) | {193, 203}

    def side(text: list[str | None], *, candidate: bool) -> list[dict[str, object]]:
        rows: list[dict[str, object]] = []
        for index, formatted in enumerate(text):
            diff_kind = None
            if index == POOL_ROW:
                diff_kind = "DIFF_ARG_MISMATCH"
            elif index == TERMINAL_ROW:
                diff_kind = "DIFF_DELETE"
            rows.append(
                _instruction(
                    0x100 + index * 4,
                    formatted,
                    diff_kind=diff_kind,
                    relocation=index in relocation_rows and formatted is not None,
                )
            )
        return rows

    def function(text: list[str | None], size: int, *, candidate: bool) -> dict[str, object]:
        return {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "address": "0x100",
            "size": str(size),
            "match_percent": 99.51163,
            "instructions": side(text, candidate=candidate),
        }

    return {
        "left": {"symbols": [function(target_text, 860, candidate=False)]},
        "right": {"symbols": [function(candidate_text, 856, candidate=True)]},
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    return {
        "schema": switch_fold.CONTEXT_SCHEMA,
        "report_artifact_sha256": "4e" * 32,
        "precursor": {
            "function": FUNCTION,
            "candidate_id": "hanachanom006-two-arm-switch",
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "51" * 32,
            "data_report_sha256": "e7" * 32,
            "object_sha256": "23" * 32,
            "target_bytes": 860,
            "candidate_bytes": 856,
            "target_frame": 0x100,
            "candidate_frame": 0x100,
            "match_percent": 99.51163,
            "target_physical_relocations": 55,
            "candidate_physical_relocations": 55,
            "residual_pairs": [
                {
                    "row": POOL_ROW,
                    "kind": "pool_operand",
                    "target": "lfs f3, lbl_802C3E50@sda21",
                    "candidate": "lfs f3, @723@sda21",
                },
                {
                    "row": TERMINAL_ROW,
                    "kind": "target_only_terminal_branch",
                    "target": "b 0x3e5c",
                    "candidate": None,
                },
            ],
            "operation_order_exact": True,
            "stack_homes_exact": True,
            "physical_relocations_exact": True,
            "protected_siblings_preserved": True,
        },
        "topology": {
            "source_shape": "switch_terminal_default_cleanup_return",
            "state_load_row": 18,
            "state_load_form": "lwz r0, 0x4c(r30)",
            "state_compare_row": 19,
            "state_compare_form": "cmpwi r0, 0x0",
            "zero_branch_row": 20,
            "target_zero_branch": "beq 0x3b80",
            "candidate_zero_branch": "beq 0x3ab0",
            "body_exit_row": 200,
            "target_body_exit": "b 0x3e5c",
            "candidate_body_exit": "b 0x3d88",
            "cleanup_start_row": 201,
            "cleanup_window": [
                "lwz r3, mbObjMan@sda21",
                "mr r4, r30",
                "bl omDelObjEx",
            ],
            "terminal_branch_row": TERMINAL_ROW,
            "target_terminal_branch": "b 0x3e5c",
            "epilogue_row": 205,
            "epilogue_form": "psq_l f31, 0xf8(r1), 0, qr0",
            "pool_consumer_row": 193,
            "pool_consumer_form": "bl mbev_CapEffGlowAdd",
        },
        "negative_controls": [
            {
                "axis": "outer_zero_if_else",
                "outcome": "wrong_size_topology",
                "candidate_id": "hanachanom001-outer-zero-if-else",
                "strict_report_sha256": "be" * 32,
                "target_bytes": 860,
                "candidate_bytes": 852,
                "match_percent": 99.0,
            },
            {
                "axis": "explicit_else_return",
                "outcome": "wrong_size_topology",
                "candidate_id": "hanachanom002-explicit-else-return",
                "strict_report_sha256": "a4" * 32,
                "target_bytes": 860,
                "candidate_bytes": 856,
                "match_percent": 99.46512,
            },
            {
                "axis": "terminal_if_chain",
                "outcome": "baseline_equivalent",
                "candidate_id": "hanachanom003-terminal-if-chain",
                "strict_report_sha256": "1d" * 32,
                "target_bytes": 860,
                "candidate_bytes": 852,
                "match_percent": 96.23256,
            },
            {
                "axis": "explicit_two_arm_returns",
                "outcome": "wrong_size_topology",
                "candidate_id": "hanachanom004-explicit-two-arm-returns",
                "strict_report_sha256": "16" * 32,
                "target_bytes": 860,
                "candidate_bytes": 852,
                "match_percent": 98.976746,
            },
            {
                "axis": "inner_else_return",
                "outcome": "wrong_size_topology",
                "candidate_id": "hanachanom005-inner-else-return",
                "strict_report_sha256": "0b" * 32,
                "target_bytes": 860,
                "candidate_bytes": 864,
                "match_percent": 97.53488,
            },
        ],
        "topology_result": {
            "candidate_id": "hanachanom007-switch-default-return",
            "objdiff_canonical_sha256": "6b" * 32,
            "strict_report_sha256": "06" * 32,
            "data_report_sha256": "7b" * 32,
            "object_sha256": "bf" * 32,
            "target_bytes": 860,
            "candidate_bytes": 860,
            "match_percent": 99.976746,
            "physical_relocations": 55,
            "residual_row": POOL_ROW,
            "target_form": "lfs f3, lbl_802C3E50@sda21",
            "candidate_form": "lfs f3, @723@sda21",
        },
        "pool_residual": {
            "decoder_receipt_sha256": "28" * 32,
            "decoder_schema": "match_workbench_pool_decoder/v1",
            "row": POOL_ROW,
            "register": "f3",
            "target_operand": "lbl_802C3E50@sda21",
            "candidate_operand": "@723@sda21",
            "target_bits": "3da740da",
            "candidate_bits": "3da740db",
            "value_type": "f32",
            "consumer_symbol": "mbev_CapEffGlowAdd",
            "consumer_row": 193,
            "consumer_count": 1,
            "relocation_topology_exact": True,
            "owner_chronology_exact": True,
        },
        "typed_fold": {
            "candidate_source": "4.9f / 60.0f",
            "exact_source": "4.9 / 60.0",
            "numerator": 4.9,
            "denominator": 60.0,
            "candidate_domain": "f32",
            "exact_domain": "f64",
            "destination_type": "f32",
            "candidate_bits": "3da740db",
            "exact_bits": "3da740da",
            "opaque_bit_literals_forbidden": True,
            "arbitrary_numeric_search_forbidden": True,
        },
        "exact_result": {
            "candidate_id": "hanachanom008-double-ratio",
            "source_sha256": "b1" * 32,
            "object_sha256": "0c" * 32,
            "strict_report_sha256": "2b" * 32,
            "data_report_sha256": "38" * 32,
            "candidate_record_sha256": "74" * 32,
            "target_bytes": 860,
            "candidate_bytes": 860,
            "physical_relocations": 55,
        },
        "telemetry": {
            "candidate_count": 9,
            "tracer_runs": 0,
            "donor_searches": 0,
            "telemetry_complete": False,
            "interval_log_sha256": "79" * 32,
        },
        "authority_advanced": False,
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]
        if item["rule_id"] == switch_fold.RULE_ID
    )


class SwitchDefaultConstantFoldTests(unittest.TestCase):
    def test_emits_two_ordered_cells(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            switch_default_fold_context=_context(report),
        )
        evaluation = _evaluation(result)
        self.assertTrue(evaluation["matched"])
        cells = evaluation["evidence"]["recommended_cells"]
        self.assertEqual(
            [cell["kind"] for cell in cells],
            ["switch_terminal_default_cleanup_return", "typed_f32_vs_f64_constant_fold"],
        )
        self.assertEqual([cell["order"] for cell in cells], [1, 2])
        self.assertIn("opaque_bit_literals", evaluation["evidence"]["suppressed_axes"])
        self.assertFalse(evaluation["evidence"]["authority_advanced"])

    def test_missing_context_fails_closed(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol=FUNCTION)
        self.assertFalse(_evaluation(result)["matched"])

    def test_hash_or_topology_drift_fails_closed(self) -> None:
        report = _report()
        context = _context(report)
        context["precursor"]["objdiff_canonical_sha256"] = "ff" * 32
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            switch_default_fold_context=context,
        )
        self.assertFalse(_evaluation(result)["matched"])

        report = _report()
        context = _context(report)
        report["right"]["symbols"][0]["instructions"][203]["instruction"]["formatted"] = "bl other_cleanup"
        context["precursor"]["objdiff_canonical_sha256"] = rules._sha256(rules._canonical(report))
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            switch_default_fold_context=context,
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_typed_fold_or_closed_context_drift_is_rejected(self) -> None:
        report = _report()
        for name, mutate in (
            ("bits", lambda value: value["typed_fold"].__setitem__("exact_bits", "3da740db")),
            ("guessing", lambda value: value["typed_fold"].__setitem__("arbitrary_numeric_search_forbidden", False)),
            ("extra", lambda value: value.__setitem__("unexpected", True)),
        ):
            context = copy.deepcopy(_context(report))
            mutate(context)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol=FUNCTION,
                        switch_default_fold_context=context,
                    )

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
                        "--switch-default-fold-context",
                        str(context_path),
                    ]
                )
        self.assertEqual(exit_code, 0)
        self.assertTrue(_evaluation(json.loads(output.getvalue()))["matched"])


if __name__ == "__main__":
    unittest.main()
