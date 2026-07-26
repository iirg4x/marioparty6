import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.agent_queue import check_diff_claim, record_verification, release_task
from tools.worktree_manager import close_worktree, create_worktree


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )


class WorktreeManagerTests(unittest.TestCase):
    def test_create_claim_verify_and_close(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "README").write_text("fixture\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            main_commit = subprocess.run(
                ["git", "rev-parse", "HEAD"],
                cwd=root,
                check=True,
                text=True,
                capture_output=True,
            ).stdout.strip()
            run(root, "git", "update-ref", "refs/remotes/origin/main", "HEAD")
            run(root, "git", "checkout", "-qb", "agent/recovery-context-workflow")
            (root / "AI_WORKSPACE.md").write_text(
                "AI workspace fixture\n", encoding="utf-8"
            )
            run(root, "git", "add", "AI_WORKSPACE.md")
            run(root, "git", "commit", "-qm", "AI workspace")
            ai_commit = subprocess.run(
                ["git", "rev-parse", "HEAD"],
                cwd=root,
                check=True,
                text=True,
                capture_output=True,
            ).stdout.strip()

            value = create_worktree(
                root,
                agent="claude",
                owner="docs-task",
                base=ai_commit,
                source="README",
                change_class="documentation",
            )
            worktree = Path(value["worktree"])
            self.assertTrue(worktree.is_dir())
            self.assertTrue((worktree / "AI_WORKSPACE.md").is_file())
            self.assertEqual(value["task"]["base_ref"], ai_commit)

            (worktree / "README").write_text("pilot change\n", encoding="utf-8")
            ai_diff = check_diff_claim(worktree)
            self.assertEqual(ai_diff["base"], ai_commit)
            self.assertEqual(ai_diff["changed"], ["README"])
            self.assertEqual(ai_diff["errors"], [])
            main_diff = check_diff_claim(worktree, base=main_commit)
            self.assertIn("AI_WORKSPACE.md", main_diff["changed"])
            self.assertTrue(
                any("AI_WORKSPACE.md" in error for error in main_diff["errors"])
            )
            run(worktree, "git", "add", "README")
            run(worktree, "git", "commit", "-qm", "Pilot task change")
            record_verification(
                worktree,
                "docs-task",
                agent="claude",
                public_gate="pass",
            )
            release_task(
                worktree,
                "docs-task",
                agent="claude",
                status="done",
            )
            close_worktree(root, owner="docs-task")
            self.assertFalse(worktree.exists())


if __name__ == "__main__":
    unittest.main()
