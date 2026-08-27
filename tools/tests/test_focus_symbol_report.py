from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import focus_symbol_report as focus_report


FUNCTION = "FocusFunction"


def _instruction(address: int, formatted: str, *, relocation: dict[str, object] | None = None) -> dict[str, object]:
    instruction: dict[str, object] = {
        "address": str(address),
        "formatted": formatted,
        "parts": [{"opcode": {"mnemonic": formatted.split()[0]}}],
        "size": 4,
    }
    if relocation is not None:
        instruction["relocation"] = relocation
    return {"instruction": instruction}


def _report(*, focus_exact: bool, sibling_exact: bool, extra_exact: bool = False) -> dict[str, object]:
    target_rows = [
        _instruction(
            0x100,
            "lfs f1, pool@sda21",
            relocation={
                "target_symbol": 3,
                "type": 109,
                "type_name": "R_PPC_EMB_SDA21",
            },
        ),
        _instruction(0x104, "blr"),
    ]
    candidate_rows = copy.deepcopy(target_rows)
    if not focus_exact:
        target_rows[0]["diff_kind"] = "DIFF_ARG_MISMATCH"
        target_rows[0]["arg_diff"] = [{}, {"diff_index": 0}]
        candidate_rows[0]["diff_kind"] = "DIFF_ARG_MISMATCH"
        candidate_rows[0]["arg_diff"] = [{}, {"diff_index": 0}]
        candidate_rows[0]["instruction"]["formatted"] = "lfs f2, @1@sda21"
    sibling_target_rows = [_instruction(0x200, "blr")]
    sibling_candidate_rows = copy.deepcopy(sibling_target_rows)
    if not sibling_exact:
        sibling_target_rows[0]["diff_kind"] = "DIFF_ARG_MISMATCH"
        sibling_candidate_rows[0]["diff_kind"] = "DIFF_ARG_MISMATCH"
    left = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "size": "8",
            "target_symbol": 1,
            "match_percent": 100.0 if focus_exact else 75.0,
            "instructions": target_rows,
        },
        {
            "name": "ProtectedSibling",
            "kind": "SYMBOL_FUNCTION",
            "size": "4",
            "target_symbol": 2,
            "match_percent": 100.0 if sibling_exact else 50.0,
            "instructions": sibling_target_rows,
        },
        {
            "name": "pool",
            "kind": "SYMBOL_OBJECT",
            "address": "0",
            "size": "4",
            "data_diff": [{"data": "P4AAAA==", "size": "4"}],
        },
    ]
    right = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "size": "8",
            "instructions": candidate_rows,
        },
        {
            "name": "ProtectedSibling",
            "kind": "SYMBOL_FUNCTION",
            "size": "4",
            "instructions": sibling_candidate_rows,
        },
        {
            "name": "@1",
            "kind": "SYMBOL_OBJECT",
            "address": "0",
            "size": "4",
            "data_diff": [{"data": "P4AAAA==", "size": "4"}],
        },
    ]
    if extra_exact:
        left.append(
            {
                "name": "NewExactSibling",
                "kind": "SYMBOL_FUNCTION",
                "size": "4",
                "target_symbol": 4,
                "match_percent": 100.0,
                "instructions": [_instruction(0x300, "blr")],
            }
        )
        right.append(
            {
                "name": "NewExactSibling",
                "kind": "SYMBOL_FUNCTION",
                "size": "4",
                "instructions": [_instruction(0x300, "blr")],
            }
        )
    return {
        "left": {
            "sections": [{"name": ".text", "kind": "SECTION_CODE", "size": "12"}],
            "symbols": left,
        },
        "right": {
            "sections": [{"name": ".text", "kind": "SECTION_CODE", "size": "12"}],
            "symbols": right,
        },
    }


def _binding() -> dict[str, object]:
    return {
        "strict_report": {"path": "strict.json", "sha256": "11" * 32, "size_bytes": 100},
        "data_report": {"path": "data.json", "sha256": "22" * 32, "size_bytes": 100},
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }


def _physical_receipt() -> dict[str, object]:
    row = {
        "offset": 0,
        "type": 109,
        "type_name": "R_PPC_EMB_SDA21",
        "target_symbol": "pool",
        "addend": "0",
    }
    return {
        "schema": "physical/v1",
        "report": {"path": "strict.json", "sha256": "11" * 32},
        "target": {
            "size": "8",
            "instruction_count": 2,
            "physical_relocation_count": 1,
            "physical_relocations": [row],
        },
        "candidate": {
            "size": "8",
            "instruction_count": 2,
            "physical_relocation_count": 1,
            "physical_relocations": [row],
        },
        "physical_relocations_exact": True,
        "physical_relocation_differences": [],
        "symbol_attribution_aliases": [],
    }


class FocusSymbolReportTests(unittest.TestCase):
    def test_extract_preserves_rows_relocations_pool_and_physical_receipt(self) -> None:
        report = _report(focus_exact=False, sibling_exact=True)
        artifact = focus_report.build_artifact(
            report,
            copy.deepcopy(report),
            FUNCTION,
            _binding(),
            physical_receipt=_physical_receipt(),
            physical_binding={"path": "physical.json", "sha256": "33" * 32, "size_bytes": 100},
            require_physical=True,
        )

        strict = artifact["channels"]["strict"]
        data = artifact["channels"]["data"]
        self.assertEqual(strict["target"]["rows_kind"], "all")
        self.assertEqual(len(strict["target"]["rows"]), 2)
        self.assertNotIn("parts", strict["target"]["rows"][0]["instruction"])
        self.assertIn("parts_sha256", strict["target"]["rows"][0])
        self.assertEqual(data["target"]["rows_kind"], "diff_only")
        self.assertEqual(len(data["target"]["rows"]), 1)
        self.assertEqual(
            data["relocation_annotations"]["storage"], "strict_channel_only"
        )
        relocation = strict["relocation_annotations"]["target"]
        self.assertEqual(relocation["count"], 1)
        self.assertEqual(relocation["pool_dependencies"][0]["name"], "pool")
        self.assertEqual(artifact["physical_relocations"]["status"], "exact")
        self.assertFalse(artifact["authority_advanced"])

    def test_cross_channel_digest_ignores_diff_annotations_only(self) -> None:
        strict_report = _report(focus_exact=False, sibling_exact=True)
        data_report = copy.deepcopy(strict_report)
        for side in ("left", "right"):
            focus = next(
                symbol
                for symbol in data_report[side]["symbols"]
                if symbol.get("name") == FUNCTION
            )
            focus["instructions"][0]["diff_kind"] = "DATA_DIFF"
            focus["instructions"][0]["arg_diff"] = [{"data_only": True}]
        artifact = focus_report.build_artifact(
            strict_report,
            data_report,
            FUNCTION,
            _binding(),
        )
        self.assertNotEqual(
            artifact["channels"]["strict"]["target"]["raw_instruction_sha256"],
            artifact["channels"]["data"]["target"]["raw_instruction_sha256"],
        )
        self.assertEqual(
            artifact["channels"]["strict"]["target"][
                "instruction_payload_sha256"
            ],
            artifact["channels"]["data"]["target"][
                "instruction_payload_sha256"
            ],
        )

    def test_protected_gate_matches_exact_identity_subset(self) -> None:
        baseline_report = _report(focus_exact=False, sibling_exact=True)
        candidate_report = _report(focus_exact=True, sibling_exact=True, extra_exact=True)
        baseline = focus_report.build_artifact(
            baseline_report, copy.deepcopy(baseline_report), FUNCTION, _binding()
        )
        candidate = focus_report.build_artifact(
            candidate_report, copy.deepcopy(candidate_report), FUNCTION, _binding()
        )
        gate = focus_report.gate_artifacts(
            baseline,
            candidate,
            {"baseline_artifact": {}, "candidate_artifact": {}, "authority_advanced": False},
        )
        self.assertTrue(gate["passed"])
        self.assertEqual(gate["channels"]["strict"]["missing_exact_siblings"], [])
        self.assertEqual(
            gate["channels"]["strict"]["gained_exact_siblings"], ["NewExactSibling"]
        )

    def test_protected_gate_detects_exact_sibling_regression(self) -> None:
        before_report = _report(focus_exact=False, sibling_exact=True)
        after_report = _report(focus_exact=True, sibling_exact=False)
        before = focus_report.build_artifact(
            before_report, copy.deepcopy(before_report), FUNCTION, _binding()
        )
        after = focus_report.build_artifact(
            after_report, copy.deepcopy(after_report), FUNCTION, _binding()
        )
        gate = focus_report.gate_artifacts(before, after, {"authority_advanced": False})
        self.assertFalse(gate["passed"])
        self.assertEqual(
            gate["channels"]["strict"]["missing_exact_siblings"], ["ProtectedSibling"]
        )

    def test_physical_receipt_is_unknown_without_independent_receipt(self) -> None:
        report = _report(focus_exact=False, sibling_exact=True)
        artifact = focus_report.build_artifact(
            report, copy.deepcopy(report), FUNCTION, _binding()
        )
        self.assertEqual(artifact["physical_relocations"]["status"], "UNKNOWN")
        with self.assertRaisesRegex(focus_report.FocusReportError, "required"):
            focus_report.build_artifact(
                report,
                copy.deepcopy(report),
                FUNCTION,
                _binding(),
                require_physical=True,
            )

    def test_path_extraction_is_hash_bound_and_budgeted(self) -> None:
        report = _report(focus_exact=False, sibling_exact=True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            strict = root / "strict.json"
            data = root / "data.json"
            strict.write_text(json.dumps(report), encoding="utf-8")
            data.write_text(json.dumps(report), encoding="utf-8")
            strict_sha = hashlib.sha256(strict.read_bytes()).hexdigest()
            data_sha = hashlib.sha256(data.read_bytes()).hexdigest()
            artifact = focus_report.build_from_paths(
                strict_report_path=strict,
                data_report_path=data,
                function=FUNCTION,
                expected_strict_report_sha256=strict_sha,
                expected_data_report_sha256=data_sha,
            )
            self.assertEqual(artifact["input_binding"]["strict_report"]["sha256"], strict_sha)
            with self.assertRaisesRegex(focus_report.FocusReportError, "mismatch"):
                focus_report.build_from_paths(
                    strict_report_path=strict,
                    data_report_path=data,
                    function=FUNCTION,
                    expected_strict_report_sha256="00" * 32,
                    expected_data_report_sha256=data_sha,
                )
            with self.assertRaisesRegex(focus_report.FocusReportError, "exceeds"):
                focus_report._write_result(artifact, None, 8, pretty=False)

    def test_duplicate_focus_fails_closed(self) -> None:
        report = _report(focus_exact=False, sibling_exact=True)
        report["left"]["symbols"].append(copy.deepcopy(report["left"]["symbols"][1]))
        with self.assertRaisesRegex(focus_report.FocusReportError, "exactly one"):
            focus_report.build_artifact(
                report, copy.deepcopy(report), FUNCTION, _binding()
            )


if __name__ == "__main__":
    unittest.main()
