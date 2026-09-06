from __future__ import annotations

import argparse
import inspect
import json
import threading
import unittest
from pathlib import Path
from unittest import mock

from tools import recovery_evaluate as evaluate
from tools import recovery_frontier as frontier
from tools.tests import test_recovery_evaluate as batch_tests


class _BatchFixture:
    """Reuse the established bounded batch fixture without inheriting its tests."""

    def __init__(self) -> None:
        self.case = batch_tests.RecoveryEvaluateTests("runTest")
        self.case.setUp()

    def close(self) -> None:
        self.case.tearDown()

    def __getattr__(self, name: str):
        return getattr(self.case, name)


class RecoveryEvaluateStreamTests(unittest.TestCase):
    def setUp(self) -> None:
        self.fixture = _BatchFixture()

    def tearDown(self) -> None:
        self.fixture.close()

    def _result(self, kwargs: dict[str, object], status: str = "improved",
                gains: list[str] | None = None) -> dict[str, object]:
        return batch_tests.RecoveryEvaluateTests._batch_result(kwargs, status, gains)

    def test_callback_delivers_fast_durable_result_before_slow_sibling(self) -> None:
        fast = self.fixture.root / "stream-fast.c"
        alias = self.fixture.root / "stream-alias.c"
        slow = self.fixture.root / "stream-slow.c"
        fast.write_text("int f(void) { return 10; }\n", encoding="utf-8")
        alias.write_bytes(fast.read_bytes())
        slow.write_text("int f(void) { return 11; }\n", encoding="utf-8")
        jobs = [
            {"id": "fast", "candidate": fast.name, "functions": ["FocusFunction"]},
            {"id": "slow", "candidate": slow.name, "functions": ["FocusFunction"]},
            {"id": "alias", "candidate": alias.name, "functions": ["FocusFunction"]},
        ]
        slow_started = threading.Event()
        slow_release = threading.Event()
        slow_finished = threading.Event()
        first_seen = threading.Event()
        events: list[dict[str, object]] = []
        result_holder: dict[str, object] = {}
        errors: list[BaseException] = []

        def fake_measure(**kwargs: object) -> dict[str, object]:
            if Path(kwargs["candidate"]).stem == "stream-slow":
                slow_started.set()
                slow_release.wait(5)
                slow_finished.set()
            return self._result(kwargs)

        def on_result(event: dict[str, object]) -> None:
            events.append(event)
            if event["id"] == "fast":
                result_path = self.fixture.root / event["result"]["path"]
                self.assertTrue(result_path.is_file())
                first_seen.set()

        def run() -> None:
            try:
                result_holder["value"] = self.fixture._evaluate_batch(
                    jobs, "stream-early.json", workers=2, on_result=on_result)
            except BaseException as exc:  # surface worker failures after join
                errors.append(exc)

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            worker = threading.Thread(target=run)
            worker.start()
            try:
                self.assertTrue(slow_started.wait(2))
                self.assertTrue(first_seen.wait(2))
                self.assertFalse(slow_finished.is_set())
            finally:
                slow_release.set()
                worker.join(5)
        self.assertFalse(worker.is_alive())
        self.assertEqual(errors, [])
        summary = result_holder["value"]
        self.assertIsInstance(summary, dict)
        assert isinstance(summary, dict)
        self.assertEqual(summary["status"], "complete")
        self.assertEqual({event["id"] for event in events}, {"fast", "slow", "alias"})
        self.assertEqual(len(events), 3)
        self.assertEqual(events[0]["id"], "fast")
        self.assertEqual(events[0]["event"], "job_completed")
        self.assertEqual(events[0]["schema"], evaluate.BATCH_EVENT_SCHEMA)
        self.assertEqual(events[0]["functions"], ["FocusFunction"])
        self.assertIsInstance(events[0]["baseline_index"], dict)
        self.assertIsInstance(events[0]["summary"], dict)
        self.assertEqual(events[0]["summary"]["status"], "improved")
        self.assertIn("sha256", events[0]["result"])
        self.assertTrue(events[0]["diagnostic_only"])
        self.assertFalse(events[0]["adoption_ready"])
        self.assertFalse(events[0]["batch_complete"])
        self.assertFalse(events[0]["drift_detected_so_far"])
        self.assertEqual(events[-1]["id"], "alias")
        self.assertEqual(events[-1]["status"], "duplicate_source")

    def test_callback_is_optional_and_default_batch_shape_is_unchanged(self) -> None:
        self.assertIsNone(inspect.signature(evaluate.evaluate_batch).parameters["on_result"].default)
        candidate = self.fixture.root / "no-stream.c"
        candidate.write_text("int f(void) { return 12; }\n", encoding="utf-8")

        def fake_measure(**kwargs: object) -> dict[str, object]:
            return self._result(kwargs)

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            summary = self.fixture._evaluate_batch(
                [{"id": "plain", "candidate": candidate.name,
                  "functions": ["FocusFunction"]}], "no-stream.json", workers=1)
        self.assertEqual(summary["status"], "complete")
        self.assertNotIn("notification_errors", summary)
        self.assertFalse(summary["adoption_ready"])
        self.assertEqual(summary["jobs"][0]["status"], "improved")

    def test_callback_mutation_cannot_change_frozen_baseline_or_results(self) -> None:
        candidate = self.fixture.root / "annotate-event.c"
        candidate.write_text("int f(void) { return 42; }\n", encoding="utf-8")

        def fake_measure(**kwargs: object) -> dict[str, object]:
            return self._result(kwargs, gains=["FocusFunction: 2 -> 1"])

        def annotate(event: dict[str, object]) -> None:
            event["baseline_index"]["sha256"] = "0" * 64
            event["summary"]["gains"].append("not measured")
            event["functions"].append("not measured")

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            summary = self.fixture._evaluate_batch(
                [{"id": "annotated", "candidate": candidate.name,
                  "functions": ["FocusFunction"]}], "annotate-event.json",
                workers=1, on_result=annotate)
        self.assertEqual(summary["status"], "complete")
        self.assertNotEqual(summary["baseline_index"]["sha256"], "0" * 64)
        self.assertEqual(summary["jobs"][0]["functions"], ["FocusFunction"])
        self.assertEqual(summary["measured_gains"][0]["gains"], ["FocusFunction: 2 -> 1"])

    def test_notification_failure_does_not_erase_positive_results(self) -> None:
        jobs = []
        for name in ("notify-a", "notify-b"):
            candidate = self.fixture.root / f"{name}.c"
            candidate.write_text(f"int f(void) {{ return {len(jobs) + 20}; }}\n", encoding="utf-8")
            jobs.append({"id": name, "candidate": candidate.name,
                         "functions": ["FocusFunction"]})
        callback_calls: list[str] = []

        def fake_measure(**kwargs: object) -> dict[str, object]:
            return self._result(kwargs, gains=["FocusFunction: positive"])

        def broken_sink(event: dict[str, object]) -> None:
            callback_calls.append(str(event["id"]))
            raise RuntimeError("notification sink closed")

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            summary = self.fixture._evaluate_batch(
                jobs, "notify-failure.json", workers=2, on_result=broken_sink)
        self.assertEqual(summary["status"], "complete")
        self.assertEqual(len(callback_calls), 1)
        self.assertTrue(summary["notification_errors"])
        self.assertIn("notification sink closed", summary["notification_errors"][0])
        self.assertEqual([row["status"] for row in summary["jobs"]], ["improved", "improved"])
        self.assertEqual(len(summary["measured_gains"]), 2)
        self.assertTrue((self.fixture.root / "build/notify-failure.json").is_file())
        job_dir = self.fixture.root / "build/notify-failure.jobs"
        self.assertEqual(sorted(path.name for path in job_dir.glob("*.json")),
                         ["notify-a.json", "notify-b.json"])

    def test_stream_events_show_drift_and_never_claim_adoption(self) -> None:
        candidates: dict[str, Path] = {}
        objects: dict[str, Path] = {}
        jobs = []
        for name in ("drift-first", "drift-pending", "drift-last"):
            candidate = self.fixture.root / f"{name}.c"
            candidate.write_text(f"int f(void) {{ return {30 + len(jobs)}; }}\n", encoding="utf-8")
            obj = self.fixture.root / f"{name}.o"
            obj.write_bytes(name.encode("ascii"))
            candidates[name] = candidate
            objects[name] = obj
            jobs.append({"id": name, "candidate": candidate.name,
                         "functions": ["FocusFunction"], "candidate_object": obj.name})
        events: list[dict[str, object]] = []
        calls: list[str] = []

        def fake_measure(**kwargs: object) -> dict[str, object]:
            calls.append(Path(kwargs["candidate"]).stem)
            if len(calls) == 1:
                candidates["drift-first"].write_text("int f(void) { return 999; }\n", encoding="utf-8")
                self.fixture.index.write_bytes(self.fixture.index.read_bytes() + b" ")
                objects["drift-last"].write_bytes(b"changed object")
            return self._result(kwargs, gains=["FocusFunction: measured"])

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            summary = self.fixture._evaluate_batch(
                jobs, "stream-drift.json", command_json=None, compiler_tools=[],
                workers=1, on_result=events.append)
        self.assertEqual(calls, ["drift-first"])
        self.assertEqual(summary["status"], "drifted")
        self.assertTrue(summary["drift_detected"])
        self.assertFalse(summary["retention_ready"])
        self.assertFalse(summary["adoption_ready"])
        self.assertEqual([event["id"] for event in events],
                         ["drift-first", "drift-pending", "drift-last"])
        self.assertTrue(all(event["diagnostic_only"] for event in events))
        self.assertTrue(all(not event["adoption_ready"] for event in events))
        self.assertTrue(all(not event["batch_complete"] for event in events))
        self.assertTrue(all(event["drift_detected_so_far"] for event in events))
        self.assertEqual(events[1]["status"], "not_scheduled")
        self.assertEqual(events[2]["status"], "not_scheduled")

    def test_large_result_event_is_bounded(self) -> None:
        candidate = self.fixture.root / "large-event.c"
        candidate.write_text("int f(void) { return 13; }\n", encoding="utf-8")
        events: list[dict[str, object]] = []

        def fake_measure(**kwargs: object) -> dict[str, object]:
            result = self._result(kwargs)
            result["gains"] = ["g" * 5000] * 4
            result["regressions"] = ["r" * 5000] * 2
            return result

        with mock.patch.object(evaluate, "evaluate", side_effect=fake_measure):
            self.fixture._evaluate_batch(
                [{"id": "large", "candidate": candidate.name,
                  "functions": ["FocusFunction"]}], "large-event.json",
                workers=1, on_result=events.append)
        self.assertEqual(len(events), 1)
        self.assertLessEqual(len(frontier.canonical(events[0])), 16 * 1024)
        self.assertLessEqual(len(json.dumps(events[0]["summary"]).encode("utf-8")),
                             evaluate._EVALUATE_STDOUT_LIMIT)

    def test_stream_cli_flushes_job_and_terminal_jsonl(self) -> None:
        parser = argparse.ArgumentParser()
        parser.add_argument("--root", type=Path, default=self.fixture.root)
        evaluate.add_batch_arguments(parser)
        args = parser.parse_args([
            "--index", "build/index.json", "--manifest", "build/manifest.json",
            "--out", "build/stream-cli.json", "--objdiff", str(self.fixture.objdiff),
            "--readelf", str(self.fixture.readelf), "--stream",
        ])

        def fake_batch(**kwargs: object) -> dict[str, object]:
            callback = kwargs["on_result"]
            callback({"schema": evaluate.BATCH_EVENT_SCHEMA, "event": "job_completed",
                      "id": "cli-job", "status": "improved", "functions": ["FocusFunction"],
                      "result": {"path": "job.json", "sha256": "a" * 64, "size_bytes": 2},
                      "diagnostic_only": True, "adoption_ready": False,
                      "batch_complete": False, "drift_detected_so_far": False})
            output = args.root / args.out
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text("{}\n", encoding="utf-8")
            return {"status": "complete", "drift_detected": False, "drift_reasons": [],
                    "compiler_runs": 1, "objdiff_runs": 2, "retention_ready": False,
                    "seconds": 0.01, "jobs": [{"id": "cli-job", "status": "improved"}],
                    "best_positive_candidates": [], "adoption_ready": False,
                    "authority_advanced": False}

        with mock.patch.object(evaluate, "evaluate_batch", side_effect=fake_batch), \
                mock.patch("builtins.print") as printer:
            self.assertEqual(evaluate.dispatch_batch(args), 0)
        self.assertEqual(printer.call_count, 2)
        self.assertTrue(all(call.kwargs.get("flush") is True
                            for call in printer.call_args_list))
        first = json.loads(printer.call_args_list[0].args[0])
        final = json.loads(printer.call_args_list[-1].args[0])
        self.assertEqual(first["event"], "job_completed")
        self.assertEqual(final["event"], "batch_completed")
        self.assertTrue(final["batch_complete"])
        self.assertFalse(final["adoption_ready"])
        self.assertEqual(final["result"]["size_bytes"],
                         (args.root / args.out).stat().st_size)


if __name__ == "__main__":
    unittest.main()
