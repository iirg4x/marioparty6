from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


MODULE_PATH = Path(__file__).resolve().parents[1] / "mwcc_execution_receipt.py"
SPEC = importlib.util.spec_from_file_location("mwcc_execution_receipt", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ExecutionReceiptTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.plan = self._file("live-plan.json", b'{"status":"READY"}\n')
        self.child_a = self._file("retained-envelope.json", b'{"events":12}\n')
        self.child_b = self._file("v491-envelope.json", b'{"events":12}\n')
        self.measurement = self._file(
            "active-measurement.json",
            json.dumps(
                {
                    "schema": MODULE.MEASUREMENT_SCHEMA,
                    "generator": MODULE.MEASUREMENT_GENERATOR,
                    "clock": "perf_counter_ns+monotonic_ns",
                    "intervals": [
                        {
                            "started_utc": "2026-08-25T08:00:00.000000Z",
                            "stopped_utc": "2026-08-25T08:00:05.000000Z",
                            "start_perf_counter_ns": 100_000_000_000,
                            "end_perf_counter_ns": 105_000_000_000,
                            "start_monotonic_ns": 200_000_000_000,
                            "end_monotonic_ns": 205_000_000_000,
                        },
                        {
                            "started_utc": "2026-08-25T08:00:10.000000Z",
                            "stopped_utc": "2026-08-25T08:00:17.500000Z",
                            "start_perf_counter_ns": 110_000_000_000,
                            "end_perf_counter_ns": 117_500_000_000,
                            "start_monotonic_ns": 210_000_000_000,
                            "end_monotonic_ns": 217_500_000_000,
                        },
                    ],
                    "active_seconds": 12.5,
                    "measurement_complete": True,
                    "diagnostic_only": True,
                    "authority_advanced": False,
                },
                sort_keys=True,
            ).encode("utf-8"),
        )
        self.request_path = self.root / "request.json"
        self.journal = self.root / "execution-receipts.jsonl"

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _file(self, name: str, payload: bytes) -> Path:
        path = self.root / name
        path.write_bytes(payload)
        return path

    def _descriptor(self, path: Path) -> dict[str, object]:
        return MODULE.descriptor(path)

    def _request(self) -> dict[str, object]:
        return {
            "schema": MODULE.REQUEST_SCHEMA,
            "task_id": "4af0c0e3ce3a4e2b96dfa7dd2f5258df",
            "live_plan": self._descriptor(self.plan),
            "children": [
                {
                    "label": "retained",
                    "artifact": self._descriptor(self.child_a),
                    "source_span_join": {
                        "status": "MATCHED_AUTHENTICATED",
                        "reason": None,
                    },
                },
                {
                    "label": "v491",
                    "artifact": self._descriptor(self.child_b),
                    "source_span_join": {
                        "status": "UNKNOWN",
                        "reason": "stack aggregate has no authenticated source token",
                    },
                },
            ],
            "active_seconds": 12.5,
            "active_seconds_measured": True,
            "measurement_receipt": self._descriptor(self.measurement),
            "policy": {"diagnostic_only": True, "authority_advanced": False},
        }

    def _write_request(self, value: dict[str, object] | None = None) -> None:
        self.request_path.write_text(
            json.dumps(value if value is not None else self._request(), sort_keys=True),
            encoding="utf-8",
        )

    def test_append_preserves_prefix_and_unknown_status(self) -> None:
        self._write_request()
        first = MODULE.append_receipt(self.request_path, self.journal)
        prefix = self.journal.read_bytes()
        second = MODULE.append_receipt(self.request_path, self.journal)
        final = self.journal.read_bytes()

        self.assertTrue(final.startswith(prefix))
        self.assertEqual(first["sequence"], 0)
        self.assertEqual(second["sequence"], 1)
        self.assertEqual(second["previous_receipt_sha256"], first["receipt_sha256"])
        self.assertEqual(first["children"][1]["source_span_join"]["status"], "UNKNOWN")
        self.assertIn("no authenticated", first["children"][1]["source_span_join"]["reason"])
        validation = MODULE.validate_journal(self.journal)
        self.assertEqual(validation["entry_count"], 2)
        self.assertEqual(validation["head_receipt_sha256"], second["receipt_sha256"])

    def test_tampered_prior_line_or_hash_is_rejected(self) -> None:
        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        original = json.loads(self.journal.read_text(encoding="utf-8"))
        original["receipt_sha256"] = "0" * 64
        self.journal.write_text(json.dumps(original, sort_keys=True) + "\n", encoding="utf-8")
        with self.assertRaisesRegex(MODULE.Rejected, "receipt hash mismatch"):
            MODULE.validate_journal(self.journal)
        with self.assertRaisesRegex(MODULE.Rejected, "receipt hash mismatch"):
            MODULE.append_receipt(self.request_path, self.journal)

        self.journal.unlink()
        first = MODULE.append_receipt(self.request_path, self.journal)
        second = MODULE.append_receipt(self.request_path, self.journal)
        rows = [json.loads(line) for line in self.journal.read_text(encoding="utf-8").splitlines()]
        rows[1]["previous_receipt_sha256"] = "0" * 64
        unsigned = dict(rows[1])
        del unsigned["receipt_sha256"]
        rows[1]["receipt_sha256"] = MODULE._receipt_hash(unsigned)
        self.journal.write_text(
            "".join(json.dumps(row, sort_keys=True, separators=(",", ":")) + "\n" for row in rows),
            encoding="utf-8",
        )
        self.assertNotEqual(first["receipt_sha256"], "0" * 64)
        self.assertEqual(second["sequence"], 1)
        with self.assertRaisesRegex(MODULE.Rejected, "previous hash"):
            MODULE.validate_journal(self.journal)

    def test_recomputed_row_cannot_diverge_from_authenticated_request(self) -> None:
        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        row = json.loads(self.journal.read_text(encoding="utf-8"))
        row["task_id"] = "different-valid-task"
        unsigned = dict(row)
        del unsigned["receipt_sha256"]
        row["receipt_sha256"] = MODULE._receipt_hash(unsigned)
        self.journal.write_bytes(MODULE._canonical(row))
        with self.assertRaisesRegex(
            MODULE.Rejected, "task_id does not match authenticated request"
        ):
            MODULE.validate_journal(self.journal)

    def test_descriptor_drift_is_rejected_before_append(self) -> None:
        self._write_request()
        self.child_a.write_bytes(b"changed")
        with self.assertRaisesRegex(MODULE.Rejected, "descriptor drift"):
            MODULE.append_receipt(self.request_path, self.journal)
        self.assertFalse(self.journal.exists())

    def test_journal_cannot_collide_with_authenticated_artifact(self) -> None:
        self._write_request()
        original = self.plan.read_bytes()
        with self.assertRaisesRegex(MODULE.Rejected, "journal aliases request.live_plan"):
            MODULE.append_receipt(self.request_path, self.plan)
        self.assertEqual(self.plan.read_bytes(), original)

    def test_descriptor_drift_is_rejected_when_revalidating_journal(self) -> None:
        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        self.plan.write_bytes(b"drifted plan")
        with self.assertRaisesRegex(MODULE.Rejected, "descriptor drift"):
            MODULE.validate_journal(self.journal)

    def test_duplicate_child_labels_are_rejected(self) -> None:
        request = self._request()
        request["children"][1]["label"] = "retained"  # type: ignore[index]
        self._write_request(request)
        with self.assertRaisesRegex(MODULE.Rejected, "duplicate child label"):
            MODULE.validate_request(self.request_path)

    def test_distinct_labels_cannot_alias_one_artifact(self) -> None:
        request = self._request()
        request["children"][1]["artifact"] = (  # type: ignore[index]
            request["children"][0]["artifact"]  # type: ignore[index]
        )
        self._write_request(request)
        with self.assertRaisesRegex(
            MODULE.Rejected,
            r"request.children\[0\].artifact aliases request.children\[1\].artifact",
        ):
            MODULE.validate_request(self.request_path)

    def test_missing_zero_and_unmeasured_active_seconds_are_rejected(self) -> None:
        missing = self._request()
        del missing["active_seconds"]
        self._write_request(missing)
        with self.assertRaisesRegex(MODULE.Rejected, "missing required keys: active_seconds"):
            MODULE.validate_request(self.request_path)

        zero = self._request()
        zero["active_seconds"] = 0
        self._write_request(zero)
        with self.assertRaisesRegex(MODULE.Rejected, "finite and > 0"):
            MODULE.validate_request(self.request_path)

        unmeasured = self._request()
        unmeasured["active_seconds_measured"] = False
        self._write_request(unmeasured)
        with self.assertRaisesRegex(MODULE.Rejected, "must be true"):
            MODULE.validate_request(self.request_path)

        huge = self._request()
        huge["active_seconds"] = 10**1000
        self._write_request(huge)
        with self.assertRaisesRegex(MODULE.Rejected, "finite measured number"):
            MODULE.validate_request(self.request_path)

    def test_measurement_receipt_is_closed_complete_and_interval_bound(self) -> None:
        measurement = json.loads(self.measurement.read_text(encoding="utf-8"))
        measurement["extra"] = True
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, "unsupported keys: extra"):
            MODULE.validate_request(self.request_path)

        measurement.pop("extra")
        measurement["active_seconds"] = 13.0
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, "does not equal request"):
            MODULE.validate_request(self.request_path)

        measurement["active_seconds"] = 12.5
        measurement["measurement_complete"] = False
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, "measurement_complete must be true"):
            MODULE.validate_request(self.request_path)

        measurement["measurement_complete"] = True
        measurement["intervals"][1]["started_utc"] = "2026-08-25T08:00:04.000000Z"
        measurement["intervals"][1]["start_perf_counter_ns"] = 104_000_000_000
        measurement["intervals"][1]["start_monotonic_ns"] = 204_000_000_000
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, "overlap or are out of order"):
            MODULE.validate_request(self.request_path)

        measurement["intervals"] = [
            {
                "started_utc": "2026-08-25T08:00:00.000000Z",
                "stopped_utc": "2026-08-25T08:00:12.000000Z",
                "start_perf_counter_ns": 100_000_000_000,
                "end_perf_counter_ns": 112_000_000_000,
                "start_monotonic_ns": 200_000_000_000,
                "end_monotonic_ns": 212_000_000_000,
            }
        ]
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, "durations do not exactly equal"):
            MODULE.validate_request(self.request_path)

    def test_pointer_and_placeholder_text_are_rejected(self) -> None:
        pointer = self._request()
        pointer["children"][1]["source_span_join"]["reason"] = (  # type: ignore[index]
            "compiler owner remained at 0x1"
        )
        self._write_request(pointer)
        with self.assertRaisesRegex(MODULE.Rejected, "raw pointer/address"):
            MODULE.validate_request(self.request_path)

        placeholder = self._request()
        placeholder["children"][1]["source_span_join"]["reason"] = (  # type: ignore[index]
            "replace_me after capture"
        )
        self._write_request(placeholder)
        with self.assertRaisesRegex(MODULE.Rejected, "placeholder"):
            MODULE.validate_request(self.request_path)

    def test_authenticated_absolute_path_can_contain_hex_like_component(self) -> None:
        artifact_dir = self.root / "artifact-0x1"
        artifact_dir.mkdir()
        plan = artifact_dir / "live-plan.json"
        plan.write_bytes(b'{"status":"READY"}\n')

        request = self._request()
        request["live_plan"] = self._descriptor(plan)
        self._write_request(request)
        validated = MODULE.validate_request(self.request_path)
        self.assertEqual(validated["live_plan"]["path"], str(plan.resolve()))

        bare_address = self._file("0x1", b'{"status":"READY"}\n')
        request["live_plan"] = self._descriptor(bare_address)
        request["live_plan"]["path"] = "0x1"  # type: ignore[index]
        self._write_request(request)
        with self.assertRaisesRegex(MODULE.Rejected, "raw pointer/address"):
            MODULE.validate_request(self.request_path)

    def test_request_and_receipt_schemas_are_closed(self) -> None:
        request = self._request()
        request["extra"] = True
        self._write_request(request)
        with self.assertRaisesRegex(MODULE.Rejected, "unsupported keys: extra"):
            MODULE.validate_request(self.request_path)

        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        row = json.loads(self.journal.read_text(encoding="utf-8"))
        row["extra"] = True
        self.journal.write_text(json.dumps(row) + "\n", encoding="utf-8")
        with self.assertRaisesRegex(MODULE.Rejected, "unsupported keys: extra"):
            MODULE.validate_journal(self.journal)

    def test_noncanonical_journal_encoding_is_rejected(self) -> None:
        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        row = json.loads(self.journal.read_text(encoding="utf-8"))
        self.journal.write_text(json.dumps(row, sort_keys=True) + "\n", encoding="utf-8")
        with self.assertRaisesRegex(MODULE.Rejected, "not canonically encoded"):
            MODULE.validate_journal(self.journal)

    def test_partial_final_line_and_authority_policy_are_rejected(self) -> None:
        self._write_request()
        request = self._request()
        request["policy"] = {"diagnostic_only": True, "authority_advanced": True}
        self._write_request(request)
        with self.assertRaisesRegex(MODULE.Rejected, "diagnostic_only=true"):
            MODULE.validate_request(self.request_path)

        self._write_request()
        MODULE.append_receipt(self.request_path, self.journal)
        self.journal.write_bytes(self.journal.read_bytes().rstrip(b"\n"))
        with self.assertRaisesRegex(MODULE.Rejected, "partial final line"):
            MODULE.validate_journal(self.journal)

    def test_cli_outputs_deterministic_validation(self) -> None:
        self._write_request()
        expected = MODULE._request_validation(self.request_path)
        self.assertEqual(expected, MODULE._request_validation(self.request_path))
        self.assertEqual(expected["source_span_status_counts"]["UNKNOWN"], 1)
        self.assertEqual(expected["active_seconds"], 12.5)

    def test_cli_validate_append_and_validate_journal(self) -> None:
        self._write_request()
        commands = (
            ("validate-request", self.request_path),
            ("append", self.request_path, self.journal),
            ("validate-journal", self.journal),
        )
        outputs = []
        for command in commands:
            completed = subprocess.run(
                [sys.executable, str(MODULE_PATH), *(str(item) for item in command)],
                check=False,
                capture_output=True,
                text=True,
            )
            self.assertEqual(completed.returncode, 0, completed.stderr)
            outputs.append(json.loads(completed.stdout))
        self.assertEqual(outputs[0]["status"], "READY")
        self.assertEqual(outputs[1]["sequence"], 0)
        self.assertEqual(outputs[2]["status"], "VALID")
        self.assertEqual(outputs[2]["entry_count"], 1)

    def test_measure_commands_create_append_safe_consumed_receipt(self) -> None:
        opened = self.root / "active.open.jsonl"
        closed = self.root / "active.closed.json"
        with (
            mock.patch.object(
                MODULE, "_now_utc", side_effect=[
                    "2026-08-25T09:00:00.000000Z",
                    "2026-08-25T09:00:02.000000Z",
                ]
            ),
            mock.patch.object(
                MODULE.time,
                "perf_counter_ns",
                side_effect=[300_000_000_000, 302_000_000_000],
            ),
            mock.patch.object(
                MODULE.time,
                "monotonic_ns",
                side_effect=[400_000_000_000, 402_000_000_000],
            ),
        ):
            start = MODULE.measure_start(opened)
            prefix = opened.read_bytes()
            result = MODULE.measure_stop(opened, closed)
        self.assertTrue(opened.read_bytes().startswith(prefix))
        self.assertEqual(start["event"], "START")
        self.assertEqual(result["schema"], MODULE.MEASUREMENT_SCHEMA)
        self.assertEqual(result["active_seconds"], 2.0)
        self.assertEqual(result["clock"], "perf_counter_ns+monotonic_ns")
        rows = MODULE._parse_measurement_sidecar(opened.read_bytes())
        self.assertEqual([row["event"] for row in rows], ["START", "STOP"])
        self.assertEqual(rows[1]["closed_receipt"], MODULE.descriptor(closed))
        with self.assertRaisesRegex(MODULE.Rejected, "already closed"):
            MODULE.measure_stop(opened, self.root / "another.closed.json")
        with self.assertRaisesRegex(MODULE.Rejected, "already exists"):
            MODULE.measure_start(opened)

    def test_measure_stop_rejects_alias_zero_and_stale_intervals(self) -> None:
        opened = self.root / "alias.open.jsonl"
        with (
            mock.patch.object(
                MODULE, "_now_utc", return_value="2026-08-25T10:00:00.000000Z"
            ),
            mock.patch.object(MODULE.time, "perf_counter_ns", return_value=500_000_000_000),
            mock.patch.object(MODULE.time, "monotonic_ns", return_value=600_000_000_000),
        ):
            MODULE.measure_start(opened)
        with self.assertRaisesRegex(MODULE.Rejected, "aliases"):
            MODULE.measure_stop(opened, opened)

        zero_open = self.root / "zero.open.jsonl"
        with (
            mock.patch.object(
                MODULE, "_now_utc", side_effect=[
                    "2026-08-25T10:00:00.000000Z",
                    "2026-08-25T10:00:01.000000Z",
                ]
            ),
            mock.patch.object(
                MODULE.time,
                "perf_counter_ns",
                side_effect=[700_000_000_000, 700_000_000_000],
            ),
            mock.patch.object(
                MODULE.time,
                "monotonic_ns",
                side_effect=[800_000_000_000, 800_000_000_000],
            ),
        ):
            MODULE.measure_start(zero_open)
            with self.assertRaisesRegex(MODULE.Rejected, "zero or negative"):
                MODULE.measure_stop(zero_open, self.root / "zero.closed.json")

        negative_open = self.root / "negative.open.jsonl"
        with (
            mock.patch.object(
                MODULE, "_now_utc", side_effect=[
                    "2026-08-25T10:00:00.000000Z",
                    "2026-08-25T10:00:01.000000Z",
                ]
            ),
            mock.patch.object(
                MODULE.time,
                "perf_counter_ns",
                side_effect=[710_000_000_000, 709_000_000_000],
            ),
            mock.patch.object(
                MODULE.time,
                "monotonic_ns",
                side_effect=[810_000_000_000, 809_000_000_000],
            ),
        ):
            MODULE.measure_start(negative_open)
            with self.assertRaisesRegex(MODULE.Rejected, "zero or negative"):
                MODULE.measure_stop(
                    negative_open, self.root / "negative.closed.json"
                )

        stale_open = self.root / "stale.open.jsonl"
        stale_ns = (MODULE.MAX_MEASUREMENT_SECONDS + 1) * 1_000_000_000
        with (
            mock.patch.object(
                MODULE, "_now_utc", side_effect=[
                    "2026-08-24T09:00:00.000000Z",
                    "2026-08-25T09:00:01.000000Z",
                ]
            ),
            mock.patch.object(
                MODULE.time,
                "perf_counter_ns",
                side_effect=[900_000_000_000, 900_000_000_000 + stale_ns],
            ),
            mock.patch.object(
                MODULE.time,
                "monotonic_ns",
                side_effect=[1_000_000_000_000, 1_000_000_000_000 + stale_ns],
            ),
        ):
            MODULE.measure_start(stale_open)
            with self.assertRaisesRegex(MODULE.Rejected, "stale"):
                MODULE.measure_stop(stale_open, self.root / "stale.closed.json")

        overwrite_open = self.root / "overwrite.open.jsonl"
        overwrite_closed = self.root / "overwrite.closed.json"
        MODULE.measure_start(overwrite_open)
        overwrite_closed.write_bytes(b"preserve me")
        with self.assertRaisesRegex(MODULE.Rejected, "already exists"):
            MODULE.measure_stop(overwrite_open, overwrite_closed)
        self.assertEqual(overwrite_closed.read_bytes(), b"preserve me")

    def test_measurement_rejects_wall_clock_only_and_hardlink(self) -> None:
        measurement = json.loads(self.measurement.read_text(encoding="utf-8"))
        measurement["clock"] = "utc"
        self.measurement.write_text(json.dumps(measurement), encoding="utf-8")
        self._write_request()
        with self.assertRaisesRegex(MODULE.Rejected, r"perf_counter_ns\+monotonic_ns"):
            MODULE.validate_request(self.request_path)

        opened = self.root / "hardlink.open.jsonl"
        MODULE.measure_start(opened)
        alias = self.root / "hardlink.alias.jsonl"
        try:
            alias.hardlink_to(opened)
        except OSError as exc:
            self.skipTest(f"hard links unavailable: {exc}")
        with self.assertRaisesRegex(MODULE.Rejected, "exactly one hard link"):
            MODULE.measure_stop(opened, self.root / "hardlink.closed.json")

    def test_measure_start_rejects_symlink_when_supported(self) -> None:
        target = self._file("symlink-target.jsonl", b"target")
        link = self.root / "symlink-open.jsonl"
        try:
            link.symlink_to(target)
        except OSError as exc:
            self.skipTest(f"symlinks unavailable: {exc}")
        with self.assertRaisesRegex(MODULE.Rejected, "symlink/reparse"):
            MODULE.measure_start(link)

    def test_cli_help_documents_measurement_commands(self) -> None:
        completed = subprocess.run(
            [sys.executable, str(MODULE_PATH), "--help"],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertIn("measure-start", completed.stdout)
        self.assertIn("measure-stop", completed.stdout)


if __name__ == "__main__":
    unittest.main()
