from __future__ import annotations

import hashlib
import io
import json
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch

from tools import prepare_owner_capture as module


class PrepareOwnerCaptureTests(unittest.TestCase):
    def _fixture(self) -> tuple[Path, Path, Path, list[str]]:
        temporary = TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        (root / "src/board").mkdir(parents=True)
        (root / "include").mkdir()
        (root / "build/GP6E01/include").mkdir(parents=True)
        source = root / "src/board/demo.c"
        source.write_bytes(b"void Demo(void) {\n    return;\n}\n")
        wrapper = root / "wrapper.exe"
        compiler = root / "compiler.exe"
        wrapper.write_bytes(b"wrapper fixture")
        compiler.write_bytes(b"compiler fixture")
        command = [
            str(wrapper),
            str(compiler),
            "-O0,p",
            "-i",
            "include",
            "-c",
            "placeholder.c",
            "-o",
            "placeholder.o",
        ]
        return root, source, root / "build/capture", command

    def _prepare(
        self,
        root: Path,
        source: Path,
        output: Path,
        command: list[str],
        **kwargs: object,
    ) -> Path:
        return module.prepare_capture(
            root,
            source,
            "Demo",
            hashlib.sha256(source.read_bytes()).hexdigest(),
            command,
            output,
            **kwargs,
        )

    def test_prepare_is_no_compile_and_seals_normalized_argv(self) -> None:
        root, source, output, command = self._fixture()
        with patch("subprocess.run", side_effect=AssertionError("prepare launched a process")):
            prepared_path = self._prepare(
                root,
                source,
                output,
                command,
                authority_purpose="Demo diagnostic capture",
                scratch_basename="demo.o",
            )
        prepared = json.loads(prepared_path.read_text(encoding="utf-8"))
        self.assertEqual(prepared["status"], module.PREPARED_STATUS)
        self.assertEqual(prepared["authority_purpose"], "Demo diagnostic capture")
        self.assertEqual(prepared["scratch_basename"], "demo.o")
        request = json.loads(
            (output / "capture/request.json").read_text(encoding="utf-8")
        )
        self.assertEqual(request["function"], "Demo")
        self.assertEqual(request["argv"][request["argv"].index("-c") + 1], str(source.resolve()))
        self.assertEqual(
            request["argv"][request["argv"].index("-o") + 1],
            str((output / "object/demo.o").resolve()),
        )
        self.assertFalse((output / "object/demo.o").exists())
        self.assertFalse((output / "launch-started.json").exists())
        self.assertEqual(sorted(path.name for path in (output / "capture").iterdir()), ["request.json"])
        self.assertIn("VarInfo.usage+0x04", prepared["limitations"][0])

    def test_default_scratch_basename_is_capture_object(self) -> None:
        root, source, output, command = self._fixture()
        prepared = json.loads(self._prepare(root, source, output, command).read_text(encoding="utf-8"))
        self.assertEqual(prepared["scratch_basename"], "capture.o")
        self.assertTrue(prepared["scratch_object"].endswith("\\object\\capture.o"))

    def test_wrong_source_hash_rejects_before_creating_output(self) -> None:
        root, source, output, command = self._fixture()
        with self.assertRaisesRegex(module.OwnerCaptureError, "unchanged-source guard"):
            module.prepare_capture(root, source, "Demo", "0" * 64, command, output)
        self.assertFalse(output.exists())

    def test_existing_output_is_rejected_without_overwrite(self) -> None:
        root, source, output, command = self._fixture()
        output.mkdir(parents=True)
        sentinel = output / "sentinel.txt"
        sentinel.write_text("keep", encoding="utf-8")
        with self.assertRaisesRegex(module.OwnerCaptureError, "fresh directory"):
            self._prepare(root, source, output, command)
        self.assertEqual(sentinel.read_text(encoding="utf-8"), "keep")

    def test_command_json_file_is_loaded_without_batch_reparse(self) -> None:
        root, source, output, command = self._fixture()
        command_path = root / "build/compiler-command.json"
        command_path.write_text(json.dumps(command), encoding="utf-8")
        prepared_path = self._prepare(root, source, output, command_path)
        prepared = json.loads(prepared_path.read_text(encoding="utf-8"))
        authority = json.loads((output / "authority.json").read_text(encoding="utf-8"))
        self.assertEqual(authority["compiler_command"]["kind"], "file")
        self.assertEqual(authority["compiler_command"]["path"], str(command_path.resolve()))
        self.assertEqual(prepared["status"], module.PREPARED_STATUS)

    def test_source_drift_blocks_run_before_marker_or_runner(self) -> None:
        root, source, output, command = self._fixture()
        prepared_path = self._prepare(root, source, output, command)
        source.write_bytes(b"void Demo(void) {\n    return;\n    /* drift */\n}\n")
        calls: list[list[str]] = []

        def runner(argv: list[str]) -> int:
            calls.append(argv)
            return 0

        with self.assertRaisesRegex(module.OwnerCaptureError, "identity changed|identity mismatch|authentication"):
            module.run_capture(prepared_path, capture_runner=runner)
        self.assertEqual(calls, [])
        self.assertFalse((output / "launch-started.json").exists())

    def test_run_writes_exclusive_marker_and_invokes_once(self) -> None:
        root, source, output, command = self._fixture()
        prepared_path = self._prepare(root, source, output, command)
        calls: list[list[str]] = []

        def runner(argv: list[str]) -> int:
            calls.append(list(argv))
            return 0

        self.assertEqual(module.run_capture(prepared_path, capture_runner=runner), 0)
        self.assertEqual(len(calls), 1)
        marker = json.loads((output / "launch-started.json").read_text(encoding="utf-8"))
        self.assertEqual(marker["status"], module.LAUNCH_STATUS)
        self.assertEqual(marker["capture_argv"], calls[0])
        self.assertTrue((root / "build/.compiler-lane.lock").exists())
        with self.assertRaisesRegex(module.OwnerCaptureError, "launch marker already exists"):
            module.run_capture(prepared_path, capture_runner=runner)
        self.assertEqual(len(calls), 1)

    def test_runner_failure_after_marker_reports_unknown_after_launch(self) -> None:
        root, source, output, command = self._fixture()
        prepared_path = self._prepare(root, source, output, command)
        (output / "launch-started.json").write_text("{}", encoding="utf-8")
        stderr = io.StringIO()
        with patch.object(module, "run_capture", side_effect=module.OwnerCaptureError("native failure")), redirect_stderr(stderr):
            self.assertEqual(module.main(["run", str(prepared_path)]), 2)
        error = json.loads(stderr.getvalue())
        self.assertEqual(error["status"], "UNKNOWN_AFTER_LAUNCH")
        self.assertEqual(error["reason"], "native failure")

    def test_help_and_prepare_stdout_expose_capture_limitation(self) -> None:
        help_text = io.StringIO()
        with redirect_stdout(help_text):
            with self.assertRaises(SystemExit) as raised:
                module.main(["--help"])
        self.assertEqual(raised.exception.code, 0)
        self.assertIn("VarInfo.usage+0x04", help_text.getvalue())

        root, source, output, command = self._fixture()
        stdout = io.StringIO()
        with redirect_stdout(stdout):
            self.assertEqual(
                module.main([
                    "prepare",
                    "--root", str(root),
                    "--source", str(source),
                    "--function", "Demo",
                    "--expected-source-sha256", hashlib.sha256(source.read_bytes()).hexdigest(),
                    "--compiler-command", json.dumps(command),
                    "--output-root", str(output),
                ]),
                0,
            )
        prepared = json.loads(stdout.getvalue())
        self.assertEqual(prepared["status"], module.PREPARED_STATUS)
        self.assertIn("VarInfo.usage+0x04", prepared["limitations"][0])

    def test_scratch_basename_rejects_path_components(self) -> None:
        root, source, output, command = self._fixture()
        with self.assertRaisesRegex(module.OwnerCaptureError, "plain filename"):
            self._prepare(root, source, output, command, scratch_basename="nested/demo.o")


if __name__ == "__main__":
    unittest.main()
