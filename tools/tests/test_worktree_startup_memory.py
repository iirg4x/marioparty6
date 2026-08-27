import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from tools.recovery_memory import RecoveryMemoryError, startup_check
from tools.tests import test_recovery_workflow as recovery_fixture
from tools.worktree_manager import close_worktree, create_worktree


def run(cwd: Path, *args: str) -> str:
    process = subprocess.run(
        args,
        cwd=cwd,
        text=True,
        capture_output=True,
        check=True,
    )
    return process.stdout.strip()


class WorktreeStartupMemoryTests(unittest.TestCase):
    def repository(self, directory: str) -> Path:
        root = Path(directory) / "repo"
        root.mkdir()
        recovery_fixture.RecoveryWorkflowTests().fixture(root)
        run(root, "git", "init", "-q", "-b", "main")
        run(root, "git", "config", "user.email", "test@example.com")
        run(root, "git", "config", "user.name", "Test")
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "base")
        run(
            root,
            "git",
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        return root

    def test_startup_records_canonical_lane_snapshot(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = self.repository(directory)
            result = startup_check(root, sync_reports=False)
            self.assertEqual(result["status"], "pass")
            self.assertEqual(result["queue_locations"]["status"], "canonical")
            self.assertTrue(Path(result["memory_path"]).is_file())
            self.assertEqual(result["permanent_commit"], result["head_commit"])

    def test_startup_rejects_lane_behind_permanent_workflow(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = self.repository(directory)
            base = run(root, "git", "rev-parse", "HEAD")
            run(root, "git", "checkout", "-qb", "workflow-update")
            (root / "workflow.txt").write_text("new workflow\n", encoding="utf-8")
            run(root, "git", "add", "workflow.txt")
            run(root, "git", "commit", "-qm", "workflow update")
            run(
                root,
                "git",
                "update-ref",
                "refs/remotes/origin/agent/recovery-context-workflow",
                "HEAD",
            )
            run(root, "git", "checkout", "-qb", "stale-lane", base)
            with self.assertRaisesRegex(RecoveryMemoryError, "lane is stale"):
                startup_check(root, sync_reports=False)

    def test_recovery_worktree_create_checks_manager_and_lane(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = self.repository(directory)
            startup_result = {
                "status": "pass",
                "snapshot_sha256": "a" * 64,
            }
            with mock.patch(
                "tools.worktree_manager.startup_check",
                return_value=startup_result,
            ) as check:
                result = create_worktree(
                    root,
                    agent="tester",
                    owner="REL:test:a",
                    base="HEAD",
                    source="src/a.c",
                )
            self.assertEqual(check.call_count, 2)
            self.assertTrue(check.call_args_list[0].kwargs["sync_reports"])
            self.assertFalse(check.call_args_list[1].kwargs["sync_reports"])
            self.assertEqual(result["manager_startup"], startup_result)
            self.assertEqual(result["lane_startup"], startup_result)
            close_worktree(root, owner="REL:test:a", force=True)


if __name__ == "__main__":
    unittest.main()
