import contextlib
import hashlib
import io
import json
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from unittest.mock import patch

from tools.agent import ProbeError, main, probe_lookup, probe_record


class AgentProbeTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.history = self.root / "build/board-autonomy/batch-history.json"
        self.strict = self.root / "strict.json"
        self.value = self.root / "value.json"
        self.strict.write_bytes(b"strict report")
        self.value.write_bytes(b"value report")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def record(self, probe_key: str = "shape") -> dict:
        return probe_record(
            self.root,
            "main:board/math",
            "MathFn",
            probe_key,
            "input-v1",
            "GC/1.3.2",
            "mwcc-2.7",
            "A" * 64,
            "B" * 64,
            self.strict,
            "rejected",
            "does not match",
            value_report=self.value,
            metrics={"bytes": "12"},
            history=self.history,
        )

    def test_record_migrates_legacy_batches_and_hashes_reports(self) -> None:
        self.history.parent.mkdir(parents=True)
        self.history.write_text(
            json.dumps({"batches": [{"owners": ["src/a.c"]}], "legacy": True}),
            encoding="utf-8",
        )

        result = self.record()
        history = json.loads(self.history.read_text(encoding="utf-8"))
        record = history["probes"]["main:board/math|MathFn|shape"]

        self.assertEqual(result["status"], "recorded")
        self.assertEqual(history["schema_version"], 2)
        self.assertEqual(history["batches"], [{"owners": ["src/a.c"]}])
        self.assertTrue(history["legacy"])
        self.assertEqual(record["target_sha256"], "a" * 64)
        self.assertEqual(record["candidate_sha256"], "b" * 64)
        self.assertEqual(
            record["outputs"]["strict"]["sha256"],
            hashlib.sha256(b"strict report").hexdigest(),
        )
        self.assertEqual(record["outputs"]["value"]["artifact"], str(self.value))
        self.assertEqual(
            history["result_index"][
                "main:board/math|GC/1.3.2|"
                + "a" * 64
                + "|"
                + record["outputs"]["strict"]["sha256"]
                + "|"
                + record["outputs"]["value"]["sha256"]
            ],
            "main:board/math|MathFn|shape",
        )

    def test_lookup_idempotency_duplicate_and_conflict(self) -> None:
        self.assertEqual(self.record()["status"], "recorded")
        self.assertEqual(
            probe_lookup(
                self.root,
                "main:board/math",
                "MathFn",
                "shape",
                "input-v1",
                history=self.history,
            )["status"],
            "known",
        )
        self.assertEqual(self.record()["status"], "unchanged")
        duplicate = self.record("equivalent-shape")
        self.assertEqual(duplicate["status"], "duplicate")
        self.assertEqual(
            duplicate["record"]["duplicate_of"],
            "main:board/math|MathFn|shape",
        )
        conflict = probe_lookup(
            self.root,
            "main:board/math",
            "MathFn",
            "shape",
            "different-input",
            history=self.history,
        )
        self.assertEqual(conflict["status"], "conflict")
        self.assertEqual(conflict["record"]["input_key"], "input-v1")
        self.assertIn("new evidence-descriptive probe key", conflict["reason"])
        with self.assertRaisesRegex(ProbeError, "conflicting evidence"):
            probe_record(
                self.root,
                "main:board/math",
                "MathFn",
                "shape",
                "different-input",
                "GC/1.3.2",
                "mwcc-2.7",
                "A" * 64,
                "B" * 64,
                self.strict,
                "accepted",
                "changed evidence",
                history=self.history,
            )

    def test_record_rejects_non_sha256_target_hash(self) -> None:
        with self.assertRaisesRegex(ProbeError, "64 hexadecimal"):
            probe_record(
                self.root,
                "main:board/math",
                "MathFn",
                "invalid-hash",
                "input-v1",
                "GC/1.3.2",
                "mwcc-2.7",
                "not-a-hash",
                "B" * 64,
                self.strict,
                "rejected",
                "invalid target",
                history=self.history,
            )

    def test_cli_lookup_json(self) -> None:
        self.record()
        output = io.StringIO()
        argv = [
            "agent.py",
            "--root",
            str(self.root),
            "probe",
            "lookup",
            "--owner",
            "main:board/math",
            "--symbol",
            "MathFn",
            "--probe-key",
            "shape",
            "--input-key",
            "input-v1",
            "--history",
            str(self.history),
            "--json",
        ]
        with patch("sys.argv", argv), contextlib.redirect_stdout(output), patch(
            "tools.agent.root_from", return_value=self.root
        ):
            self.assertEqual(main(), 0)
        self.assertEqual(json.loads(output.getvalue())["status"], "known")

    def test_concurrent_records_are_atomic_and_deduplicated(self) -> None:
        def write(probe_key: str) -> str:
            return self.record(probe_key)["status"]

        with ThreadPoolExecutor(max_workers=2) as pool:
            statuses = sorted(pool.map(write, ("parallel-a", "parallel-b")))
        self.assertEqual(statuses, ["duplicate", "recorded"])
        history = json.loads(self.history.read_text(encoding="utf-8"))
        self.assertEqual(len(history["probes"]), 2)
        self.assertEqual(len(history["result_index"]), 1)


if __name__ == "__main__":
    unittest.main()
