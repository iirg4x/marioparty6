#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools import mismatch_cluster_audit as reducer
from tools import crack_learning_rules as rules
from tools import same_file_history_contract_closure as history_contract


FUNCTION = "history_target"
REPORT_SHA = "d8" * 32
DONOR_SOURCE = """typedef struct ExistingType {
    int value;
} ExistingType;
extern int existingCounter;
extern const float missingPool;
#define MISSING_SCALE missingPool

static void history_target(ExistingType *item)
{
    existingCounter += (int)(MISSING_SCALE * item->value);
}
"""
DESTINATION_SOURCE = """typedef struct ExistingType {
    int value;
} ExistingType;
extern int existingCounter;

static void history_target(ExistingType *item)
{
    existingCounter += item->value;
}
"""


def _git(repo: Path, *arguments: str) -> str:
    completed = subprocess.run(
        ["git", "-C", str(repo), *arguments],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return completed.stdout.strip()


def _rehash_manifest(manifest: dict[str, object]) -> None:
    manifest.pop(history_contract.HASH_FIELD, None)
    manifest[history_contract.HASH_FIELD] = hashlib.sha256(
        history_contract._canonical(manifest)
    ).hexdigest()


def _instruction(index: int, formatted: str, *, relocation: bool) -> reducer.Instruction:
    return reducer.Instruction(
        index=index,
        diff_kind="DIFF_ARG_MISMATCH",
        address=0x100 + index * 4,
        size=4,
        formatted=formatted,
        mnemonic=formatted.split()[0],
        relocation=(
            {"type_name": "R_PPC_REL24", "target_symbol": index + 1}
            if relocation
            else None
        ),
        branch_dest=None,
        has_instruction=True,
        raw={},
    )


def _instructions(count: int, frame: int) -> list[reducer.Instruction]:
    return [
        _instruction(
            index,
            f"stwu r1, -0x{frame:x}(r1)" if index == 0 else "bl donor_call",
            relocation=True,
        )
        for index in range(count)
    ]


def _report() -> dict[str, object]:
    target_rows: list[dict[str, object]] = []
    candidate_rows: list[dict[str, object]] = []
    for index in range(385):
        target_form = f"stwu r1, -0xb0(r1)" if index == 0 else "bl target_semantics"
        target_instruction: dict[str, object] = {
            "address": str(0x100 + index * 4),
            "size": 4,
            "formatted": target_form,
        }
        if index < 97:
            target_instruction["relocation"] = {
                "type_name": "R_PPC_REL24",
                "target_symbol": index + 1,
            }
        target_rows.append(
            {"diff_kind": "DIFF_ARG_MISMATCH", "instruction": target_instruction}
        )
        if index < 120:
            candidate_form = (
                "stwu r1, -0x50(r1)" if index == 0 else "bl incomplete_semantics"
            )
            candidate_instruction: dict[str, object] = {
                "address": str(0x200 + index * 4),
                "size": 4,
                "formatted": candidate_form,
            }
            if index < 16:
                candidate_instruction["relocation"] = {
                    "type_name": "R_PPC_REL24",
                    "target_symbol": index + 1,
                }
            candidate_rows.append(
                {"diff_kind": "DIFF_ARG_MISMATCH", "instruction": candidate_instruction}
            )
        else:
            candidate_rows.append({"diff_kind": "DIFF_DELETE"})
    return {
        "left": {
            "symbols": [
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x100",
                    "size": "1540",
                    "instructions": target_rows,
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "0x200",
                    "size": "480",
                    "match_percent": 21.987013,
                    "instructions": candidate_rows,
                }
            ]
        },
    }


class SameFileHistoryContractClosureTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        root = Path(self.temporary.name)
        self.repo = root / "repo"
        self.repo.mkdir()
        _git(self.repo, "init", "--quiet")
        source = self.repo / "src" / "fixture.c"
        source.parent.mkdir()
        source.write_text(DONOR_SOURCE, encoding="utf-8")
        _git(self.repo, "add", "src/fixture.c")
        _git(
            self.repo,
            "-c",
            "user.name=Contract Test",
            "-c",
            "user.email=contract@example.invalid",
            "commit",
            "--quiet",
            "-m",
            "fixture",
        )
        self.commit = _git(self.repo, "rev-parse", "HEAD")
        self.destination = root / "destination.c"
        self.destination.write_text(DESTINATION_SOURCE, encoding="utf-8")
        sealed_source, _ = history_contract.load_immutable_blob(
            self.repo, self.commit, "src/fixture.c"
        )
        self.function_sha = history_contract.extract_function(
            sealed_source, FUNCTION
        ).source_sha256

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _manifest(self, destination: Path | None = None) -> dict[str, object]:
        destination = destination or self.destination
        return history_contract.build_manifest(
            repo=self.repo,
            commit=self.commit,
            source_path="src/fixture.c",
            function=FUNCTION,
            graphify_location="game/src/fixture.c:L7",
            report_sha256=REPORT_SHA,
            destination_file=destination,
            expected_function_sha256=self.function_sha,
            expected_destination_sha256=hashlib.sha256(destination.read_bytes()).hexdigest(),
            required_contracts={
                "missingPool": "extern",
                "MISSING_SCALE": "macro",
            },
        )

    def _context(
        self,
        manifest: dict[str, object],
        *,
        baseline_sha256: str = "aa" * 32,
    ) -> dict[str, object]:
        return {
            "schema": history_contract.CONTEXT_SCHEMA,
            "owner": "main:board/fixture",
            "function": FUNCTION,
            "report_sha256": REPORT_SHA,
            "manifest": manifest,
            "baseline": {
                "objdiff_canonical_sha256": baseline_sha256,
                "strict_report_sha256": "10" * 32,
                "target_bytes": 1540,
                "candidate_bytes": 480,
                "target_frame": 0xB0,
                "candidate_frame": 0x50,
                "target_physical_relocations": 97,
                "candidate_physical_relocations": 16,
                "match_percent": 21.987013,
                "semantically_incomplete": True,
            },
            "failed_preflight": {
                "body_only_attempted": True,
                "failure_stage": "before_object",
                "missing_symbol": "MISSING_SCALE",
                "object_created": False,
            },
            "exact_result": {
                "objdiff_canonical_sha256": "bb" * 32,
                "source_sha256": "01" * 32,
                "object_sha256": "02" * 32,
                "strict_report_sha256": "03" * 32,
                "data_report_sha256": "04" * 32,
                "candidate_record_sha256": "05" * 32,
                "target_bytes": 1540,
                "candidate_bytes": 1540,
                "physical_relocations": 97,
                "zero_rows": 0,
                "protected_exact_before": 21,
                "protected_exact_after": 22,
                "protected_losses": 0,
            },
            "telemetry": {
                "candidate_launches": 2,
                "compiled_candidates": 1,
                "proof_rebuilds": 1,
                "tracer_runs": 0,
                "donor_searches": 1,
                "telemetry_complete": False,
                "interval_log_sha256": "06" * 32,
            },
            "authority_advanced": False,
        }

    def test_emits_only_missing_dependencies_in_dependency_order(self) -> None:
        manifest = history_contract.parse_manifest(self._manifest())
        self.assertEqual(
            [(item["symbol"], item["kind"]) for item in manifest["dependencies"]],
            [("missingPool", "extern"), ("MISSING_SCALE", "macro")],
        )
        self.assertEqual(
            [item["symbol"] for item in manifest["satisfied_dependencies"]],
            ["ExistingType", "existingCounter"],
        )
        self.assertEqual(
            manifest["package_order"],
            ["missingPool", "MISSING_SCALE", FUNCTION],
        )
        self.assertEqual(manifest["dependencies"][1]["requires"], ["missingPool"])
        self.assertFalse(manifest["authority_advanced"])

    def test_destination_hash_and_contract_drift_fail_closed(self) -> None:
        with self.assertRaisesRegex(
            history_contract.HistoryContractInputError, "destination source hash drifted"
        ):
            history_contract.build_manifest(
                repo=self.repo,
                commit=self.commit,
                source_path="src/fixture.c",
                function=FUNCTION,
                graphify_location="game/src/fixture.c:L7",
                report_sha256=REPORT_SHA,
                destination_file=self.destination,
                expected_destination_sha256="ff" * 32,
            )

        incompatible = Path(self.temporary.name) / "incompatible.c"
        incompatible.write_text(
            DESTINATION_SOURCE.replace("extern int existingCounter;", "extern short existingCounter;"),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(
            history_contract.HistoryContractInputError, "present but incompatible"
        ):
            self._manifest(incompatible)

    def test_manifest_tampering_or_omission_is_rejected(self) -> None:
        manifest = self._manifest()
        manifest["dependencies"][0]["source"] = "extern const float changed;"
        with self.assertRaisesRegex(
            history_contract.HistoryContractInputError, "self-hash mismatch"
        ):
            history_contract.parse_manifest(manifest)

        manifest = self._manifest()
        del manifest["dependencies"][0]
        manifest["dependencies"][0]["requires"] = []
        manifest["package_order"] = ["MISSING_SCALE", FUNCTION]
        _rehash_manifest(manifest)
        with self.assertRaisesRegex(
            history_contract.HistoryContractInputError, "required_contracts.*absent"
        ):
            history_contract.parse_manifest(manifest)

    def test_context_evaluator_schedules_one_package_and_suppresses_exact(self) -> None:
        context = history_contract.parse_context(self._context(self._manifest()))
        pair = reducer.FunctionPair(
            name=FUNCTION,
            target={"size": "1540"},
            candidate={"size": "480", "match_percent": 21.987013},
        )
        result = history_contract.evaluate(
            pair,
            _instructions(97, 0xB0),
            _instructions(16, 0x50),
            context,
            "aa" * 32,
        )
        self.assertTrue(result["matched"])
        self.assertEqual(
            result["evidence"]["package_order"],
            ["missingPool", "MISSING_SCALE", FUNCTION],
        )
        self.assertEqual(
            result["evidence"]["recommended_cells"][0]["kind"],
            "authenticated_same_file_history_contract_package",
        )
        self.assertFalse(result["evidence"]["authority_advanced"])
        exact = history_contract.evaluate(
            pair,
            _instructions(97, 0xB0),
            _instructions(16, 0x50),
            context,
            "bb" * 32,
        )
        self.assertFalse(exact["matched"])
        self.assertIn("already exact", exact["reason"])

    def test_cli_writes_a_self_verified_manifest(self) -> None:
        output = Path(self.temporary.name) / "manifest.json"
        exit_code = history_contract.main(
            [
                "--repo", str(self.repo),
                "--commit", self.commit,
                "--path", "src/fixture.c",
                "--function", FUNCTION,
                "--graphify-location", "game/src/fixture.c:L7",
                "--report-sha256", REPORT_SHA,
                "--destination", str(self.destination),
                "--expect-function-sha256", self.function_sha,
                "--expect-destination-sha256", hashlib.sha256(
                    self.destination.read_bytes()
                ).hexdigest(),
                "--require-contract", "missingPool=extern",
                "--require-contract", "MISSING_SCALE=macro",
                "--output", str(output),
            ]
        )
        self.assertEqual(exit_code, 0)
        parsed = history_contract.parse_manifest(json.loads(output.read_text(encoding="utf-8")))
        self.assertEqual(parsed["function"]["source_sha256"], self.function_sha)

    def test_central_dispatcher_and_cli_place_rule_third(self) -> None:
        report = _report()
        report_sha = rules._sha256(rules._canonical(report))
        context = self._context(self._manifest(), baseline_sha256=report_sha)
        result = rules.diagnose_document(
            report,
            focus_symbol=FUNCTION,
            same_file_history_contract_context=context,
        )
        evaluation = next(
            item
            for item in result["evaluations"]
            if item["rule_id"] == history_contract.RULE_ID
        )
        self.assertTrue(evaluation["matched"])
        self.assertEqual(result["schema"], "crack_learning_diagnosis/v35")
        self.assertEqual(result["evaluations"][2]["rule_id"], history_contract.RULE_ID)

        root = Path(self.temporary.name)
        report_path = root / "objdiff.json"
        context_path = root / "context.json"
        report_path.write_text(json.dumps(report), encoding="utf-8")
        context_path.write_text(json.dumps(context), encoding="utf-8")
        import contextlib
        import io

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            exit_code = rules.main(
                [
                    "--report", str(report_path),
                    "--function", FUNCTION,
                    "--same-file-history-contract-context", str(context_path),
                ]
            )
        self.assertEqual(exit_code, 0)
        cli_result = json.loads(output.getvalue())
        self.assertEqual(cli_result, result)


if __name__ == "__main__":
    unittest.main()
