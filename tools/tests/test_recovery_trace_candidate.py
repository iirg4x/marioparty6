from __future__ import annotations

import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools import compile_recovery_candidate as compiler
from tools import recovery_frontier as frontier
from tools import recovery_temp_epochs
from tools import recovery_trace_candidate as trace
from tools.tests.test_focus_symbol_report import _report


class TraceCandidateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name) / "repo"
        (self.root / "src").mkdir(parents=True)
        (self.root / "build").mkdir()
        self.source = self.root / "src" / "unit.c"
        self.function = "FocusFunction"
        self.definition = "static int FocusFunction(int x)"
        self.before = b"int prefix = 1;\n\n" + self.definition.encode() + b"\n{\n    return x;\n}\n\nint suffix = 2;\n"
        self.after = b"int prefix = 1;\n\n" + self.definition.encode() + b"\n{\n    return x + 1;\n}\n\nint suffix = 2;\n"
        self.source.write_bytes(self.before)
        self.candidate = self.root / "build" / "candidate.c"
        self.candidate.write_bytes(self.after)
        self.target = self.root / "target.o"
        self.baseline = self.root / "baseline.o"
        self.target.write_bytes(b"target")
        self.baseline.write_bytes(b"baseline")
        self.compiler = self.root / "compiler.exe"
        self.wrapper = self.root / "wrapper.exe"
        self.capture_script = self.root / "capture.py"
        self.objdiff = self.root / "objdiff.exe"
        self.readelf = self.root / "readelf.exe"
        for path in (self.compiler, self.wrapper, self.capture_script, self.objdiff, self.readelf):
            path.write_bytes(path.name.encode())
        self.start = self.before.index(self.definition.encode())
        self.end = self.before.index(b"}\n", self.start) + 1
        strict = self.root / "strict.json"
        data = self.root / "data.json"
        strict.write_text(json.dumps(_report(focus_exact=False, sibling_exact=True)), encoding="utf-8")
        data.write_text(json.dumps(_report(focus_exact=False, sibling_exact=True)), encoding="utf-8")
        index = frontier.snapshot(
            root=self.root, owner="test:trace", source=self.source,
            target=self.target, candidate=self.baseline, strict=strict, data=data,
            toolchain_key="GC/2.6/test")
        self.index = self.root / "build" / "index.json"
        frontier.publish(self.root, self.index, index)
        self.template = self.root / "build" / "request-template.json"
        self.template.write_text(json.dumps(self._request()), encoding="utf-8")
        self.old_pin = recovery_temp_epochs.PINNED_COMPILER_SHA256
        recovery_temp_epochs.PINNED_COMPILER_SHA256 = compiler.digest(self.compiler)
        self.capture_calls: list[dict[str, object]] = []
        self.evaluate_calls: list[dict[str, object]] = []

    def tearDown(self) -> None:
        recovery_temp_epochs.PINNED_COMPILER_SHA256 = self.old_pin
        self.tmp.cleanup()

    def _request(self) -> dict[str, object]:
        return {
            "schema": "mwcc_capsule_same_session_capture_request/v1",
            "diagnostic_only": True,
            "exactness_claim": False,
            "function": self.function,
            "function_sha256": hashlib.sha256(self.before[self.start:self.end]).hexdigest(),
            "function_source_span": {"start_byte": self.start, "end_byte": self.end},
            "source": {"path": str(self.source), "sha256": compiler.digest(self.source), "size": self.source.stat().st_size},
            "compiler": {"path": str(self.compiler), "sha256": compiler.digest(self.compiler), "size": self.compiler.stat().st_size},
            "wrapper": {"path": str(self.wrapper), "sha256": compiler.digest(self.wrapper), "size": self.wrapper.stat().st_size},
            "cwd": str(self.root),
            "argv": [str(self.wrapper), str(self.compiler), "-c", str(self.source), "-o", str(self.root / "build" / "template.o")],
        }

    def _capture(self, kwargs: dict[str, object], *, different: bool = True, drift: str | None = None) -> subprocess.CompletedProcess[bytes]:
        self.capture_calls.append(kwargs)
        prefix = Path(kwargs["output_prefix"])
        output = prefix.with_suffix(".o")
        capture = prefix.with_suffix(".json")
        output.write_bytes(b"different" if different else self.baseline.read_bytes())
        source = Path(kwargs["request"]).read_bytes()
        request = json.loads(source)
        candidate = Path(request["source"]["path"])
        source_path = str(candidate if drift != "source" else self.source)
        source_sha = compiler.digest(candidate) if drift != "source" else compiler.digest(self.source)
        compiler_path = Path(request["compiler"]["path"])
        baseline = Path(kwargs["baseline"])
        events = [{
            "schema": recovery_temp_epochs.EVENT_SCHEMA, "status": "CAPTURED",
            "session_id": "session", "function": self.function, "thread_ordinal": 0,
            "sequence": 0, "event_ordinal": 1, "process_id": 1,
            "counter_before": 0, "counter_after": 1, "counter_delta": 1,
            "codegen_token": "codegen", "instruction_pointer_rva": "0x1",
            "event_id": "event-0",
        }]
        document = {
            "schema": recovery_temp_epochs.CAPTURE_SCHEMA, "status": "CAPTURED",
            "diagnostic_only": True, "authority_advanced": False,
            "function": self.function, "session_id": "session", "target_thread_ordinal": 0,
            "capture_end_reason": "complete", "counter_before_first_write": 0,
            "event_count": 1, "events": events,
            "source": {"path": source_path, "sha256": source_sha, "size_bytes": candidate.stat().st_size if drift != "source" else self.source.stat().st_size},
            "compiler": {"path": str(compiler_path), "sha256": compiler.digest(compiler_path), "size_bytes": compiler_path.stat().st_size},
            "baseline_object": {"path": str(baseline), "sha256": compiler.digest(baseline), "size_bytes": baseline.stat().st_size},
            "output_object": {"path": str(output), "sha256": compiler.digest(output), "size_bytes": output.stat().st_size},
            "request": {"path": str(kwargs["request"]), "sha256": compiler.digest(Path(kwargs["request"])), "size_bytes": Path(kwargs["request"]).stat().st_size},
            "request_argv": request["argv"],
            "launch": {"cwd": str(self.root), "output_object": str(output),
                        "request_argv": request["argv"]},
            "object_comparison": {"status": "different" if different else "byte_identical", "byte_compare_only": True, "objdiff_or_link_exactness_proven": False},
        }
        capture.write_text(json.dumps(document), encoding="utf-8")
        summary = {"status": "CAPTURED", "exit_code": 0, "events": 1,
                   "baseline_equal": not different, "output": str(capture), "cleanup_errors": []}
        return subprocess.CompletedProcess(["capture"], 1, json.dumps(summary).encode(), b"")

    def _evaluate(self, **kwargs: object) -> dict[str, object]:
        self.evaluate_calls.append(kwargs)
        return {"schema": "recovery_candidate_evaluation/v1", "status": "improved",
                "compile_binding": "caller_supplied_object; not source proof"}

    def _run(self, out_name: str, *, candidate: Path | None = None, runner=None,
             support=None, keep_reports: bool = False) -> dict[str, object]:
        return trace.trace_candidate(
            root=self.root, index=self.index, request_template=self.template,
            capture_script=self.capture_script, candidate=candidate or self.candidate,
            function=self.function, definition_prefix=self.definition, vregs=[0],
            out_dir=self.root / "build" / out_name, objdiff=self.objdiff, readelf=self.readelf,
            support_prelude=support,
            keep_reports=keep_reports,
            capture_runner=runner or (lambda _script, **kwargs: self._capture(kwargs)),
            evaluator=self._evaluate)

    def test_outer_rc_one_different_object_runs_once_and_binds_existing_object(self) -> None:
        result = self._run("trace-one")
        self.assertEqual(result["status"], "improved")
        self.assertEqual(len(self.capture_calls), 1)
        self.assertEqual(len(self.evaluate_calls), 1)
        self.assertEqual(result["binding"]["caller_supplied_object_is_not_source_proof"], True)
        self.assertEqual(self.evaluate_calls[0]["candidate_object"], self.root / "build" / "trace-one" / "capture.o")
        self.assertFalse(self.evaluate_calls[0]["keep_reports"])
        self.assertEqual(self.source.read_bytes(), self.before)

    def test_keep_reports_opt_in_is_forwarded_to_evaluator(self) -> None:
        result = self._run("trace-keep-reports", keep_reports=True)
        self.assertEqual(result["status"], "improved")
        self.assertEqual(len(self.evaluate_calls), 1)
        self.assertTrue(self.evaluate_calls[0]["keep_reports"])

    def test_byte_identical_claim_mismatch_fails_before_evaluation(self) -> None:
        def bad_runner(**kwargs):
            completed = self._capture(kwargs, different=True)
            path = Path(kwargs["output_prefix"]).with_suffix(".json")
            document = json.loads(path.read_text())
            document["object_comparison"]["status"] = "byte_identical"
            path.write_text(json.dumps(document), encoding="utf-8")
            return completed
        result = self._run("trace-bad", runner=bad_runner)
        self.assertEqual(result["status"], "failed")
        self.assertEqual(len(self.evaluate_calls), 0)

    def test_capture_source_drift_is_rejected_before_evaluation(self) -> None:
        result = self._run("trace-drift", runner=lambda _script, **kwargs: self._capture(kwargs, drift="source"))
        self.assertEqual(result["status"], "failed")
        self.assertEqual(len(self.evaluate_calls), 0)

    def test_capture_failure_and_malformed_summary_do_not_evaluate(self) -> None:
        def failed(**kwargs):
            self.capture_calls.append(kwargs)
            return subprocess.CompletedProcess(["capture"], 1, b'{"status":"FAILED","exit_code":1}', b"")
        result = self._run("trace-failed", runner=failed)
        self.assertEqual(result["status"], "failed")
        self.assertEqual(len(self.evaluate_calls), 0)

        def malformed(**kwargs):
            self.capture_calls.append(kwargs)
            return subprocess.CompletedProcess(["capture"], 1, b"truncated", b"")
        result = self._run("trace-malformed", runner=malformed)
        self.assertEqual(result["status"], "failed")
        self.assertEqual(len(self.evaluate_calls), 0)

    def test_candidate_out_of_scope_is_rejected_without_capture(self) -> None:
        outside = self.root / "build" / "outside.c"
        outside.write_bytes(self.after.replace(b"int prefix = 1", b"int prefix = 9"))
        result = self._run("trace-scope", candidate=outside)
        self.assertEqual(result["status"], "failed")
        self.assertEqual(len(self.capture_calls), 0)
        self.assertEqual(len(self.evaluate_calls), 0)

    def test_support_prelude_is_exact_and_shifts_function_span(self) -> None:
        prelude = self.root / "build" / "prelude.c"
        prelude.write_bytes(b"static inline int helper(int x) { return x; }\n\n")
        supported = self.root / "build" / "supported.c"
        supported.write_bytes(self.before[:self.start] + prelude.read_bytes() + self.after[self.start:])
        result = self._run("trace-prelude", candidate=supported, support=prelude)
        self.assertEqual(result["status"], "improved")
        request = json.loads((self.root / "build" / "trace-prelude" / "request.json").read_text())
        self.assertEqual(request["function_source_span"]["start_byte"], self.start + prelude.stat().st_size)
        self.assertEqual(result["support_prelude"]["sha256"], compiler.digest(prelude))

    def test_support_prelude_accepts_one_two_or_three_eof_newlines(self) -> None:
        helper = b"static inline int helper(int x) { return x; }"
        supported = self.root / "build" / "newline-supported.c"
        prelude = self.root / "build" / "newline-prelude.c"
        supported.write_bytes(self.before[:self.start] + helper + b"\n\n" + self.after[self.start:])

        for newline_count in (1, 2, 3):
            prelude.write_bytes(helper + b"\n" * newline_count)
            result = self._run(
                f"trace-prelude-newline-{newline_count}", candidate=supported, support=prelude
            )
            self.assertEqual(result["status"], "improved")
            request = json.loads(
                (self.root / "build" / f"trace-prelude-newline-{newline_count}" / "request.json").read_text()
            )
            actual_size = len(helper) + 2
            self.assertEqual(
                request["function_source_span"]["start_byte"], self.start + actual_size
            )
            self.assertEqual(result["support_prelude"]["size_bytes"], len(helper) + newline_count)
            self.assertEqual(result["support_prelude"]["actual_insertion_size_bytes"], actual_size)
            self.assertEqual(
                result["support_prelude"]["actual_insertion_sha256"],
                hashlib.sha256(helper + b"\n\n").hexdigest(),
            )
            self.assertEqual(
                result["support_prelude"]["trailing_crlf_normalized"], newline_count != 2
            )

    def test_support_prelude_rejects_code_or_definition_prefix_drift(self) -> None:
        helper = b"static inline int helper(int x) { return x; }"
        prelude = self.root / "build" / "strict-prelude.c"
        supported = self.root / "build" / "strict-supported.c"
        prelude.write_bytes(helper + b"\n")
        supported.write_bytes(self.before[:self.start] + helper + b"\n\n" + self.after[self.start:])

        prelude.write_bytes(b"static inline int helper(int x) { return x + 1; }\n")
        result = self._run("trace-prelude-code-drift", candidate=supported, support=prelude)
        self.assertEqual(result["status"], "failed")
        self.assertFalse(self.capture_calls)

        prelude.write_bytes(helper + b"\n")
        changed_prefix = self.after.replace(
            self.definition.encode(), b"static long FocusFunction(int x)", 1
        )
        supported.write_bytes(self.before[:self.start] + helper + b"\n\n" + changed_prefix[self.start:])
        result = self._run("trace-prelude-prefix-drift", candidate=supported, support=prelude)
        self.assertEqual(result["status"], "failed")
        self.assertFalse(self.capture_calls)

    def test_same_source_short_circuits_without_writing_live_inputs(self) -> None:
        result = self._run("trace-duplicate", candidate=self.source)
        self.assertEqual(result["status"], "duplicate_source")
        self.assertFalse(self.capture_calls)
        self.assertFalse(self.evaluate_calls)
        self.assertEqual(self.source.read_bytes(), self.before)


if __name__ == "__main__":
    unittest.main()
