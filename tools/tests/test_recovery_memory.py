import json
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from tools.recovery_memory import (
    RecoveryMemory,
    RecoveryMemoryError,
    parse_crack_report,
)


SHA_A = "a" * 64
SHA_B = "b" * 64
SHA_C = "c" * 64


class RecoveryMemoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.store = RecoveryMemory(self.root / "recovery-memory.sqlite3")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def identity(self, *, source: str = SHA_A, shape: str | None = None):
        return RecoveryMemory.identity(
            owner="main:board/example",
            function="ExampleExec",
            base_commit="base-commit",
            toolchain_key="mwcc-gc-2.6",
            target_sha256=SHA_B,
            compiler_sha256=SHA_C,
            source_sha256=source,
            shape_key=shape,
            hypothesis="direct consumer",
            axis="lifetime",
        )

    def test_admission_is_shared_and_record_is_deduplicated(self) -> None:
        identity = self.identity()
        admitted = self.store.admit(identity, requester="lane-a")
        self.assertEqual(admitted["status"], "admitted")
        blocked = self.store.admit(identity, requester="lane-b")
        self.assertEqual(blocked["status"], "pending_in_other_lane")
        recorded = self.store.record(
            identity,
            requester="lane-a",
            object_sha256=SHA_C,
            status="exact",
            reason="zero rows",
            admission_token=admitted["admission_token"],
        )
        self.assertEqual(recorded["status"], "recorded")
        known = self.store.admit(identity, requester="lane-b")
        self.assertEqual(known["status"], "known_global_source")
        self.assertTrue(known["skip_compile"])
        unchanged = self.store.record(
            identity,
            requester="lane-b",
            object_sha256=SHA_C,
            status="exact",
            reason="zero rows",
        )
        self.assertEqual(unchanged["status"], "unchanged")

    def test_record_without_precompile_admission_fails(self) -> None:
        with self.assertRaisesRegex(RecoveryMemoryError, "no pending central"):
            self.store.record(
                self.identity(),
                requester="lane-a",
                object_sha256=SHA_C,
                status="measured",
                reason="candidate measured",
            )

    def test_queue_task_suffix_normalizes_to_owner_namespace(self) -> None:
        identity = RecoveryMemory.identity(
            owner="main:board/example#full-owner-closure-v1",
            function="ExampleExec",
            base_commit="base-commit",
            toolchain_key="mwcc-gc-2.6",
            target_sha256=SHA_B,
            compiler_sha256=SHA_C,
            source_sha256=SHA_A,
        )
        self.assertEqual(identity["owner"], "main:board/example")

    def test_negative_shape_blocks_equivalent_source(self) -> None:
        first = self.identity(shape="direct-pointer-consumer")
        admission = self.store.admit(first, requester="lane-a")
        self.store.record(
            first,
            requester="lane-a",
            object_sha256=SHA_C,
            status="regressed",
            reason="frame shrank",
            admission_token=admission["admission_token"],
        )
        second = self.identity(source="d" * 64, shape="direct-pointer-consumer")
        result = self.store.admit(second, requester="lane-b")
        self.assertEqual(result["status"], "known_negative_shape")
        self.assertTrue(result["skip_compile"])

    def test_conflicting_historical_objects_are_preserved_and_quarantined(self) -> None:
        identity = self.identity()
        first = self.store.import_historical_experiment(
            identity,
            object_sha256=SHA_B,
            status="nonexact",
            reason="first immutable record",
            candidate_id="c001",
            candidate_record_sha256="d" * 64,
            strict_report_sha256=None,
            data_report_sha256=None,
            workspace="lane-a/workbench",
            source_path="lane-a/candidate.c",
        )
        self.assertEqual(first["status"], "imported")
        second = self.store.import_historical_experiment(
            identity,
            object_sha256=SHA_C,
            status="nonexact",
            reason="second immutable record",
            candidate_id="c002",
            candidate_record_sha256="e" * 64,
            strict_report_sha256=None,
            data_report_sha256=None,
            workspace="lane-b/workbench",
            source_path="lane-b/candidate.c",
        )
        self.assertEqual(second["status"], "conflict_imported")
        self.assertEqual(len(second["observations"]), 2)
        blocked = self.store.admit(identity, requester="lane-c")
        self.assertEqual(blocked["status"], "conflicting_historical_source")
        self.assertTrue(blocked["skip_compile"])

    def test_concurrent_admission_has_one_owner(self) -> None:
        identity = self.identity()
        with ThreadPoolExecutor(max_workers=2) as pool:
            results = list(
                pool.map(
                    lambda lane: self.store.admit(identity, requester=lane),
                    ("lane-a", "lane-b"),
                )
            )
        statuses = sorted(item["status"] for item in results)
        self.assertEqual(statuses, ["admitted", "pending_in_other_lane"])

    def test_json_crack_report_is_distilled_idempotently(self) -> None:
        report = self.root / "CRACK_REPORT_ExampleExec.json"
        report.write_text(
            json.dumps(
                {
                    "schema": "CRACK_REPORT/v1",
                    "owner": "main:board/example",
                    "function": "ExampleExec",
                    "result": {
                        "strict_percent": 100.0,
                        "data_percent": 100.0,
                        "target_bytes": 12,
                        "candidate_bytes": 12,
                    },
                    "chronological_attempt_ledger": [
                        {
                            "id": "c001",
                            "result": "regressed",
                            "decision": "rejected",
                        },
                        {
                            "id": "c002",
                            "result": "exact",
                            "decision": "retained",
                        },
                    ],
                    "causal_explanation": "A direct consumer removed one owner.",
                    "generalized_improvement_request": {
                        "title": "rank direct consumers",
                        "requested_behavior": "Query exact siblings first.",
                    },
                }
            ),
            encoding="utf-8",
        )
        first = self.store.ingest_report(report)
        second = self.store.ingest_report(report)
        self.assertEqual(first["status"], "ingested")
        self.assertEqual(first["constraints"], 3)
        self.assertEqual(second["status"], "unchanged")
        context = self.store.context_memory(
            "main:board/example", "ExampleExec"
        )
        self.assertEqual(len(context["reports"]), 1)
        self.assertEqual(len(context["reports"][0]["constraints"]), 3)

    def test_markdown_crack_report_is_parsed(self) -> None:
        report = self.root / "ExampleExec_CRACK_REPORT_v1.md"
        report.write_text(
            """CRACK_REPORT/v1

Owner: main:board/example
Function: ExampleExec
Result: strict 100%, data 100%, 4/4 bytes.

## Retained natural C
Use the live typed result directly.

## Causal explanation
The named temporary changed allocation.

## Generalized improvement
Rank direct consumers before declaration permutations.
""",
            encoding="utf-8",
        )
        parsed = parse_crack_report(report, report.read_bytes())
        self.assertEqual(parsed["owner"], "main:board/example")
        self.assertEqual(parsed["function"], "ExampleExec")
        result = self.store.ingest_report(report)
        self.assertEqual(result["status"], "ingested")

    def test_nonexact_report_is_rejected(self) -> None:
        report = self.root / "CRACK_REPORT_bad.json"
        report.write_text(
            json.dumps(
                {
                    "schema": "CRACK_REPORT/v1",
                    "owner": "main:board/example",
                    "function": "Bad",
                    "result": {"strict_percent": 99.0, "data_percent": 100.0},
                }
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(RecoveryMemoryError, "completed exact"):
            self.store.ingest_report(report)


if __name__ == "__main__":
    unittest.main()
