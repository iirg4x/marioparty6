import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.agent_queue import claim_task
from tools.worktree_audit import audit_active_worktrees


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


class WorktreeAuditTests(unittest.TestCase):
    def test_missing_active_worktree_is_reported(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "src").mkdir()
            (root / "src/a.c").write_text("int a;\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            worker = Path(directory) / "worker"
            run(
                root,
                "git",
                "worktree",
                "add",
                "-q",
                "-b",
                "agent/worker",
                str(worker),
            )
            claim_task(worker, "a", agent="claude", source="src/a.c")
            self.assertEqual(audit_active_worktrees(root)[0]["errors"], [])
            shutil.rmtree(worker)
            values = audit_active_worktrees(root)
            self.assertTrue(values[0]["errors"])


if __name__ == "__main__":
    unittest.main()
