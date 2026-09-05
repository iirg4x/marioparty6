from __future__ import annotations

import copy
import importlib.util
import io
import json
from contextlib import redirect_stdout
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch


TOOL = Path(__file__).resolve().parents[1] / "capsule_same_session_capture.py"
SPEC = importlib.util.spec_from_file_location("same_session_capture_cli_tests", TOOL)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class SameSessionCaptureCliTests(unittest.TestCase):
    def _envelope(self, directory: Path) -> dict[str, object]:
        events = [{"payload": "large diagnostic event " * 100}] * 1000
        return {
            "schema": MODULE.SCHEMA,
            "status": "CAPTURED_UNKNOWN_OWNERSHIP",
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "context": {
                "request": {"path": str(directory / "request.json")},
                "session_id": "session-0000000000000001",
                "function": "SNpcMoveExec",
            },
            "envelope_sha256": "a" * 64,
            "events": events,
            "event_count": len(events),
            "unknown": ["incomplete stack evidence"],
            "outputs": {
                "event_stream_stack": {
                    "path": str(directory / "stack.events.jsonl"),
                    "size": 12,
                    "sha256": "b" * 64,
                },
                "event_stream_pcode": {
                    "path": str(directory / "pcode.events.jsonl"),
                    "size": 13,
                    "sha256": "c" * 64,
                },
            },
            "inventory": {"objects": events},
            "frontend_chronology": {"packet": {"events": events}},
        }

    def _main(self, argv: list[str]) -> tuple[int, str]:
        output = io.StringIO()
        with redirect_stdout(output):
            code = MODULE.main(argv)
        return code, output.getvalue()

    def test_capture_defaults_to_bounded_summary_without_envelope_loss(self) -> None:
        with TemporaryDirectory() as temporary:
            directory = Path(temporary)
            envelope = self._envelope(directory)
            original = copy.deepcopy(envelope)
            artifacts = {
                directory / "same-session.envelope.json": json.dumps(envelope).encode("utf-8"),
                directory / "stack.events.jsonl": b"stack event\n",
                directory / "pcode.events.jsonl": b"pcode event\n",
            }
            for path, data in artifacts.items():
                path.write_bytes(data)
            with patch.object(MODULE, "_load_trust_root", return_value=None), patch.object(
                MODULE, "launch_native_capture", return_value=envelope
            ) as capture:
                code, output = self._main([
                    "capture", str(directory / "request.json"), "--trust-root", "trust.json",
                ])
            summary = json.loads(output)
            self.assertEqual(code, 0)
            self.assertLess(len(output), 4096)
            self.assertEqual(summary["status"], envelope["status"])
            self.assertEqual(summary["event_count"], 1000)
            self.assertEqual(summary["unknown_count"], 1)
            self.assertEqual(summary["envelope_canonical_sha256"], "a" * 64)
            self.assertEqual(summary["envelope_path"], str(directory / "same-session.envelope.json"))
            self.assertEqual(summary["outputs"], envelope["outputs"])
            self.assertNotIn("events", summary)
            self.assertNotIn("inventory", summary)
            self.assertNotIn("frontend_chronology", summary)
            self.assertTrue(summary["diagnostic_only"])
            self.assertFalse(summary["board_admission"])
            self.assertFalse(summary["exactness_claim"])
            self.assertFalse(summary["authority_advanced"])
            self.assertEqual(envelope, original)
            capture.assert_called_once_with(
                directory / "request.json", external_trust_root=None, partial_evidence_dir=None,
            )
            for path, data in artifacts.items():
                self.assertEqual(path.read_bytes(), data)

    def test_validate_defaults_to_same_compact_envelope_summary(self) -> None:
        envelope = self._envelope(Path("capture"))
        with patch.object(MODULE, "_load_trust_root", return_value=None), patch.object(
            MODULE, "validate_envelope", return_value=envelope
        ) as validate:
            code, output = self._main([
                "validate", "capture/same-session.envelope.json", "--trust-root", "trust.json",
            ])
        summary = json.loads(output)
        self.assertEqual(code, 0)
        self.assertEqual(summary["schema"], f"{MODULE.SCHEMA}/cli-summary")
        self.assertEqual(summary["event_count"], envelope["event_count"])
        self.assertNotIn("events", summary)
        validate.assert_called_once_with(
            Path("capture/same-session.envelope.json"), external_trust_root=None,
        )

    def test_full_output_restores_capture_and_validate_results(self) -> None:
        envelope = self._envelope(Path("capture"))
        for command, function, argument in (
            ("capture", "launch_native_capture", "capture/request.json"),
            ("validate", "validate_envelope", "capture/same-session.envelope.json"),
        ):
            with self.subTest(command=command), patch.object(
                MODULE, "_load_trust_root", return_value=None
            ), patch.object(MODULE, function, return_value=envelope):
                code, output = self._main([
                    command, argument, "--trust-root", "trust.json", "--full-output",
                ])
                self.assertEqual(code, 0)
                self.assertEqual(output, json.dumps(envelope, indent=2, sort_keys=True) + "\n")

    def test_errors_preserve_unknown_output_and_exit_code(self) -> None:
        for full_output in (False, True):
            with self.subTest(full_output=full_output), patch.object(
                MODULE, "_load_trust_root", return_value=None
            ), patch.object(MODULE, "launch_native_capture", side_effect=MODULE.Rejected("fixture rejected")):
                argv = ["capture", "request.json", "--trust-root", "trust.json"]
                if full_output:
                    argv.append("--full-output")
                code, output = self._main(argv)
                self.assertEqual(code, 2)
                self.assertEqual(json.loads(output), MODULE.unknown_result("fixture rejected"))

    def test_non_envelope_partial_result_and_exit_code_are_unchanged(self) -> None:
        result = {
            "schema": "partial-capture/capture", "status": "UNKNOWN",
            "package": {"path": "partial/manifest.json"}, "validation_failure": "unknown ownership",
        }
        with patch.object(MODULE, "_load_trust_root", return_value=None), patch.object(
            MODULE, "launch_native_capture", return_value=result
        ):
            code, output = self._main(["capture", "request.json", "--trust-root", "trust.json"])
        self.assertEqual(code, 2)
        self.assertEqual(json.loads(output), result)

    def test_sole_stdout_causal_map_result_is_not_summarized(self) -> None:
        result = {"schema": "source-aware-causal-map", "status": "READY", "rows": [{"edge": 1}]}
        with patch.object(MODULE, "_load_trust_root", return_value=object()), patch.object(
            MODULE, "build_source_aware_causal_map", return_value=result
        ):
            code, output = self._main([
                "causal-map", "--envelope", "envelope.json", "--trust-root", "trust.json",
                "--source-spans", "spans.json",
            ])
        self.assertEqual(code, 0)
        self.assertEqual(json.loads(output), result)

    def test_every_subcommand_accepts_full_output(self) -> None:
        commands = (
            ["prepare", "--manifest", "manifest.json", "--output-dir", "capture"],
            ["preflight", "request.json", "--trust-root", "trust.json"],
            ["capture", "request.json", "--trust-root", "trust.json"],
            ["validate", "envelope.json", "--trust-root", "trust.json"],
            ["causal-map", "--envelope", "envelope.json", "--trust-root", "trust.json", "--source-spans", "spans.json"],
            ["seal-source-spans", "--input", "spans.json", "--output", "sealed.json"],
            ["normalize-source-spans", "--envelope", "envelope.json", "--trust-root", "trust.json", "--template", "template.json", "--binding-plan", "plan.json", "--output", "spans.json"],
            ["self-test"],
        )
        for command in commands:
            with self.subTest(command=command[0]):
                self.assertTrue(MODULE.parser().parse_args(command + ["--full-output"]).full_output)


if __name__ == "__main__":
    unittest.main()
