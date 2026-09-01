from __future__ import annotations

import hashlib
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from tools import crack_cell_runner as runner
from tools import crack_current_residual as residual


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class CrackCellRunnerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name) / "repo"
        self.root.mkdir()
        subprocess.run(["git", "init", "-q"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "user.name", "Test"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "user.email", "test@example.com"], cwd=self.root, check=True)
        source = self.root / "src" / "board" / "owner.c"
        source.parent.mkdir(parents=True)
        source.write_text("int before;\nint Focus(void) {\n return 1;\n}\nint after;\n", encoding="utf-8")
        subprocess.run(["git", "add", "."], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "base"], cwd=self.root, check=True)
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()
        self.source = source
        self.candidate = self.root / "candidate" / "owner.c"
        self.candidate.parent.mkdir()
        self.candidate.write_text(
            "int before;\nint Focus(void) {\n return 2;\n}\nint after;\n", encoding="utf-8"
        )
        build = self.root / "build" / "current-residual"
        build.mkdir(parents=True)
        self.focus = build / "focus.json"
        focus_body = {"schema": "focus_symbol_report/v1", "channels": {}}
        self.focus.write_text(
            json.dumps({**focus_body, "artifact_sha256": runner._json_sha(focus_body)}),
            encoding="utf-8",
        )
        artifact_body = {
            "schema": residual.SCHEMA,
            "owner": "main:board/owner",
            "function": "Focus",
            "base_commit": self.commit,
            "unit": "main/board/owner",
            "base_sha256": sha(source),
            "source_sha256": sha(source),
            "target_sha256": "a" * 64,
            "function_span": {
                "start_line": 2, "end_line": 4,
                "base_span_sha256": residual._function_span_sha(source, 2, 4),
            },
            "toolchain_key": "c" * 64,
            "base_object": {"path": "base.o", "sha256": "d" * 64, "size_bytes": 1},
            "target_object": {"path": "target.o", "sha256": "e" * 64, "size_bytes": 1},
            "focus_report": {"path": self.focus.relative_to(self.root).as_posix(), "sha256": sha(self.focus), "size_bytes": self.focus.stat().st_size},
            "physical_summary": {"path": "physical.json", "sha256": "f" * 64, "size_bytes": 1},
            "strict_report": {"sha256": "1" * 64, "size_bytes": 1},
            "data_report": {"sha256": "2" * 64, "size_bytes": 1},
            "physical_receipt": {"sha256": "3" * 64, "size_bytes": 1},
            "producer": {"path": "tools/crack_current_residual.py", "sha256": "4" * 64, "size_bytes": 1},
            "residual_rows": ["strict:Focus:row:0"],
            "current_source_bound": True,
            "authority_advanced": False,
        }
        self.baseline = build / "baseline.json"
        self.baseline.write_text(
            json.dumps({**artifact_body, "residual_sha256": runner._json_sha(artifact_body)}),
            encoding="utf-8",
        )
        self.toolchain = self.root / "toolchain.json"
        self.toolchain.write_text("{}", encoding="utf-8")
        self.worktree = Path(self.temporary.name) / "scratch"

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def measurement(self, status: str = "success") -> dict[str, object]:
        return {
            "compile_status": status,
            "candidate_source_sha256": sha(self.candidate),
            "active_seconds": 0.25,
            "channels": {},
        }

    def run_mocked(self, measurement: dict[str, object] | None = None, cleanup: str | None = None):
        output = self.root / "build" / "crack-cell" / "cell.json"
        with mock.patch.object(runner, "_resolve_toolchain", return_value=self.toolchain), \
             mock.patch.object(residual, "_create_disposable_worktree", return_value=self.worktree), \
             mock.patch.object(runner, "_validate_worktree_identity"), \
             mock.patch.object(residual, "_remove_disposable_worktree", return_value=cleanup), \
             mock.patch.object(runner, "_execute_candidate", return_value=measurement or self.measurement()):
            value = runner.run_cell(
                root=self.root, baseline=self.baseline, candidate=self.candidate,
                function="Focus", label="cell", output=output,
            )
        return value, output

    def test_success_publishes_one_compact_nonretaining_result(self) -> None:
        value, output = self.run_mocked()
        self.assertEqual(value["measurement"]["compile_status"], "success")
        self.assertEqual(value["cleanup_status"], "complete")
        self.assertFalse(value["source_retained"])
        self.assertTrue(output.is_file())
        self.assertLess(output.stat().st_size, runner.MAX_OUTPUT_BYTES)

    def test_compile_failure_is_a_compact_terminal_measurement(self) -> None:
        value, output = self.run_mocked(self.measurement("failed"))
        self.assertEqual(value["measurement"]["compile_status"], "failed")
        self.assertTrue(output.is_file())

    def test_stale_baseline_and_candidate_are_rejected(self) -> None:
        stale = json.loads(self.baseline.read_text(encoding="utf-8"))
        stale["source_sha256"] = "9" * 64
        self.baseline.write_text(json.dumps(stale), encoding="utf-8")
        with self.assertRaisesRegex(runner.CellRunnerError, "self-hash"):
            runner._load_baseline(self.root, self.baseline, "Focus")
        outside = self.root / "outside.c"
        outside.write_text(
            "int changed;\nint Focus(void) {\n return 2;\n}\nint after;\n", encoding="utf-8"
        )
        artifact = {
            "base_sha256": sha(self.source),
            "function_span": {
                "start_line": 2, "end_line": 4,
                "base_span_sha256": residual._function_span_sha(self.source, 2, 4),
            },
        }
        with self.assertRaisesRegex(runner.CellRunnerError, "escape"):
            runner._validate_candidate_cell(self.source, outside, artifact)

    def test_cleanup_failure_is_fail_closed(self) -> None:
        output = self.root / "build" / "crack-cell" / "cell.json"
        with self.assertRaisesRegex(runner.CellRunnerError, "cleanup sentinel"):
            self.run_mocked(cleanup="cleanup sentinel")
        self.assertFalse(output.exists())

    def test_cleanup_failure_preserves_previous_compact_result(self) -> None:
        _value, output = self.run_mocked()
        previous = output.read_bytes()
        with self.assertRaisesRegex(runner.CellRunnerError, "cleanup sentinel"):
            self.run_mocked(cleanup="cleanup sentinel")
        self.assertEqual(output.read_bytes(), previous)

    def test_output_size_bound_fails_closed(self) -> None:
        huge = self.measurement()
        huge["differences"] = ["x" * runner.MAX_OUTPUT_BYTES]
        output = self.root / "build" / "crack-cell" / "cell.json"
        with self.assertRaisesRegex(runner.CellRunnerError, "compact result exceeds"):
            self.run_mocked(huge)
        self.assertFalse(output.exists())

    def test_no_authoritative_tree_writes_beyond_requested_output(self) -> None:
        before = {
            path.relative_to(self.root).as_posix(): sha(path)
            for path in self.root.rglob("*") if path.is_file() and ".git" not in path.parts
        }
        _value, output = self.run_mocked()
        after = {
            path.relative_to(self.root).as_posix(): sha(path)
            for path in self.root.rglob("*") if path.is_file() and ".git" not in path.parts
        }
        self.assertEqual(
            {key: value for key, value in after.items() if key != output.relative_to(self.root).as_posix()},
            before,
        )

    def test_executor_invokes_the_unit_build_exactly_once(self) -> None:
        self.worktree.mkdir()
        work_source = self.worktree / "src" / "board" / "owner.c"
        work_source.parent.mkdir(parents=True)
        work_source.write_bytes(self.source.read_bytes())
        target = self.worktree / "build" / "target.o"
        candidate_object = self.worktree / "build" / "candidate.o"
        target.parent.mkdir(parents=True)
        target.write_bytes(b"target")
        tools = {}
        for name in ("objdiff", "readelf", "ninja", "dtk"):
            path = self.worktree / name
            path.write_bytes(name.encode("ascii"))
            tools[name] = path
        (self.worktree / "powerpc-eabi-readelf.exe").write_bytes(b"readelf")
        artifact = json.loads(self.baseline.read_text(encoding="utf-8"))
        artifact["target_sha256"] = sha(target)
        focus = {
            "channels": {
                name: {
                    "metric": {"match_percent": 100.0, "target_size": 4, "candidate_size": 4},
                    "target": {"rows": [], "instruction_count": 1, "diff_row_count": 0},
                    "candidate": {"rows": [], "instruction_count": 1, "diff_row_count": 0},
                }
                for name in ("strict", "data")
            },
            "physical_relocations": {
                "status": "exact",
                "target": {"physical_relocation_count": 0},
                "candidate": {"physical_relocation_count": 0},
                "physical_relocation_differences": [],
            },
        }
        compile_calls = []

        def compile_once(command, **_kwargs):
            compile_calls.append(command)
            candidate_object.write_bytes(b"object")
            return "compiled"

        def reports(_objdiff, _target, _candidate, path, **_kwargs):
            path.write_text("{}", encoding="utf-8")

        with mock.patch.object(bundle := runner.bundle, "_load_toolchain", return_value={
                 "objdiff": {"path_object": str(tools["objdiff"])},
                 "binutils": {"path_object": str(self.worktree)},
                 "ninja": {"path_object": str(tools["ninja"])},
                 "dtk": {"path_object": str(tools["dtk"])},
             }), \
             mock.patch.object(runner.residual, "_ensure_configured_bounded", return_value=None), \
             mock.patch.object(bundle, "_unit_paths", return_value=(target, candidate_object)), \
             mock.patch.object(runner.residual, "_run_bounded", side_effect=compile_once), \
             mock.patch.object(runner.residual, "_run_objdiff_bounded", side_effect=reports), \
             mock.patch.object(runner.residual, "_physical_receipt_bounded", return_value={}), \
             mock.patch.object(bundle, "_atomic_json", side_effect=lambda path, _value: path.write_text("{}", encoding="utf-8")), \
             mock.patch.object(runner.focus_symbol_report, "build_from_paths", return_value=focus), \
             mock.patch.object(runner.focus_symbol_report, "gate_artifacts", return_value={"channels": {}}):
            value = runner._execute_candidate(
                repository=self.root, worktree=self.worktree, artifact=artifact,
                baseline_focus={}, base_source=self.source,
                candidate_source=self.candidate,
                toolchain_manifest=self.toolchain, timeout=10,
            )
        self.assertEqual(value["compile_status"], "success")
        self.assertEqual(len(compile_calls), 1)

    def test_stale_head_is_retryable_and_does_not_consume_cell(self) -> None:
        extra = self.root / "README"
        extra.write_text("new head\n", encoding="utf-8")
        subprocess.run(["git", "add", "README"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "advance"], cwd=self.root, check=True)
        value = runner.run_cell(
            root=self.root, baseline=self.baseline, candidate=self.candidate,
            function="Focus", label="stale",
        )
        self.assertEqual(value["status"], "stale_context")
        self.assertTrue(value["retryable"])
        self.assertFalse(value["compile_performed"])
        self.assertFalse(value["cell_consumed"])

    def test_porcelain_status_preserves_leading_state_columns(self) -> None:
        other = self.root / "src" / "board" / "other.c"
        other.write_text("int other;\n", encoding="utf-8")
        subprocess.run(["git", "add", "."], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "add other"], cwd=self.root, check=True)
        other.write_text("int changed;\n", encoding="utf-8")

        status = runner._git_text(
            self.root, "status", "--porcelain", "--untracked-files=no"
        )

        self.assertEqual(status, " M src/board/other.c")

    def test_tracked_write_error_keeps_complete_source_path(self) -> None:
        other = self.root / "src" / "board" / "other.c"
        other.write_text("int other;\n", encoding="utf-8")
        subprocess.run(["git", "add", "."], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "add other"], cwd=self.root, check=True)
        artifact = json.loads(self.baseline.read_text(encoding="utf-8"))
        artifact["base_commit"] = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()
        other.write_text("int changed;\n", encoding="utf-8")

        with self.assertRaisesRegex(
            runner.CellRunnerError,
            r"tracked write outside retained frontier source: src/board/other\.c",
        ):
            runner._authoritative_source(self.root, self.candidate, artifact)

    def test_foreign_posix_worktree_marker_is_rejected_before_compile(self) -> None:
        self.worktree.mkdir()
        (self.worktree / ".git").write_text(
            "gitdir: /home/Anony/repo/.git/worktrees/x\n", encoding="utf-8"
        )
        responses = iter(["", self.commit, "", str(self.root / ".git")])
        with mock.patch.object(runner, "_git_text", side_effect=lambda *_args: next(responses)):
            with self.assertRaisesRegex(runner.CellRunnerError, "escapes common Git state"):
                runner._validate_worktree_identity(self.root, self.worktree, self.commit)

    def test_real_windows_worktree_identity_is_accepted(self) -> None:
        checkout = Path(self.temporary.name) / "real-worktree"
        subprocess.run(
            ["git", "worktree", "add", "--detach", str(checkout), self.commit],
            cwd=self.root, check=True, capture_output=True,
        )
        try:
            runner._validate_worktree_identity(self.root, checkout, self.commit)
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(checkout)],
                cwd=self.root, check=True, capture_output=True,
            )


if __name__ == "__main__":
    unittest.main()
