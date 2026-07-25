import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.agent_queue import record_verification, release_task
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
            run(root, "git", "update-ref", "refs/remotes/origin/main", "HEAD")

            value = create_worktree(
                root,
                agent="claude",
                owner="docs-task",
                source="README",
                change_class="documentation",
            )
            worktree = Path(value["worktree"])
            self.assertTrue(worktree.is_dir())
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
