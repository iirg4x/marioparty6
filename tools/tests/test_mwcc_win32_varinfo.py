from __future__ import annotations

import contextlib
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import mwcc_win32_varinfo as varinfo


class MwccWin32VarInfoTests(unittest.TestCase):
    def test_default_command_and_arguments_are_stable(self) -> None:
        parsed = varinfo.parse_args([])
        self.assertEqual(Path(parsed.compiler), varinfo.DEFAULT_COMPILER)
        self.assertEqual(Path(parsed.output), varinfo.DEFAULT_OUTPUT)
        self.assertEqual(parsed.timeout, varinfo.DEFAULT_TIMEOUT_SECONDS)
        self.assertFalse(hasattr(parsed, "compiler_sha256"))

        command = varinfo.default_command(varinfo.REPO_ROOT, varinfo.DEFAULT_OUTPUT.parent)
        self.assertIn("src/board/telop.c", command)
        self.assertEqual(command[-1], str(varinfo.DEFAULT_OUTPUT.parent))

        explicit = varinfo.parse_args(["--target", "sample", "--", "-O0,p", "-c", "sample.c"])
        self.assertEqual(explicit.target, "sample")
        self.assertEqual(explicit.compiler_args, ["--", "-O0,p", "-c", "sample.c"])

    def test_atomic_output_is_schema_valid_and_leaves_no_temp(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "nested" / "report.json"
            payload = {
                "schema_version": 1,
                "tool": "mwcc_win32_varinfo",
                "target": "sample",
                "capture_assignments": False,
                "known_image_base": varinfo.KNOWN_IMAGE_BASE,
                "breakpoints": {},
            }
            varinfo.atomic_write_json(output, payload)
            loaded = json.loads(output.read_text(encoding="utf-8"))
            varinfo.validate_result_schema(loaded)
            self.assertEqual(loaded, payload)
            self.assertEqual(list(output.parent.glob("*.tmp")), [])

    def test_hook_bytes_must_match_before_writes(self) -> None:
        memory = {
            absolute + 0x1000: expected
            for absolute, expected in varinfo.EXPECTED_HOOK_BYTES.items()
        }

        def read(address: int, size: int) -> bytes:
            return memory[address][:size]

        varinfo.validate_hook_bytes(read, lambda absolute: absolute + 0x1000)

        with self.assertRaisesRegex(RuntimeError, "hook byte validation failed"):
            varinfo.validate_hook_bytes(
                lambda address, size: b"\0" * size,
                lambda absolute: absolute,
            )

    def test_compiler_name_and_fingerprint_are_checked_without_launching(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            compiler = Path(directory) / "mwcceppc.exe"
            compiler.write_bytes(b"test compiler")
            expected = hashlib.sha256(compiler.read_bytes()).hexdigest()
            self.assertEqual(varinfo.validate_compiler_fingerprint(compiler, expected), expected)
            with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
                varinfo.validate_compiler_fingerprint(compiler, "0" * 64)
            with self.assertRaisesRegex(ValueError, "must be named"):
                varinfo.validate_compiler_path(compiler.with_name("wrapper.exe"))

    def test_non_windows_main_exits_before_compiler_launch(self) -> None:
        stderr = io.StringIO()
        with mock.patch.object(varinfo.os, "name", "posix"), mock.patch.object(
            varinfo, "kernel32"
        ) as kernel32, contextlib.redirect_stderr(stderr):
            result = varinfo.main([])
        self.assertEqual(result, 2)
        self.assertIn("requires Windows", stderr.getvalue())
        kernel32.CreateProcessW.assert_not_called()

    def test_missing_compiler_is_clear_and_does_not_launch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            missing = Path(directory) / "mwcceppc.exe"
            cwd = Path(directory)
            stderr = io.StringIO()
            with mock.patch.object(varinfo.os, "name", "nt"), mock.patch.object(
                varinfo, "kernel32"
            ) as kernel32, contextlib.redirect_stderr(stderr):
                result = varinfo.main(
                    ["--compiler", str(missing), "--cwd", str(cwd)]
                )
            self.assertEqual(result, 2)
            self.assertIn("compiler not found", stderr.getvalue())
            kernel32.CreateProcessW.assert_not_called()

    def test_invalid_timeout_is_rejected_before_launch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            compiler = Path(directory) / "mwcceppc.exe"
            compiler.write_bytes(b"test compiler")
            cwd = Path(directory)
            stderr = io.StringIO()
            with mock.patch.object(varinfo.os, "name", "nt"), mock.patch.object(
                varinfo, "kernel32"
            ) as kernel32, contextlib.redirect_stderr(stderr):
                result = varinfo.main(
                    [
                        "--compiler",
                        str(compiler),
                        "--cwd",
                        str(cwd),
                        "--timeout",
                        "0",
                    ]
                )
            self.assertEqual(result, 2)
            self.assertIn("timeout must be between", stderr.getvalue())
            kernel32.CreateProcessW.assert_not_called()


if __name__ == "__main__":
    unittest.main()
