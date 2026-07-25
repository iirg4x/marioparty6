import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.agent_queue import (
    acquire_resource,
    claim_task,
    record_verification,
    update_task,
)
from tools.integration_finalize import finalize_task


def run(cwd: Path, *args: str) -> str:
    process = subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return process.stdout.strip()


class IntegrationFinalizeTests(unittest.TestCase):
    def test_ready_worker_is_finalized_from_integration_tree(self) -> None:
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
            run(root, "git", "remote", "add", "origin", str(root))
            run(root, "git", "update-ref", "refs/remotes/origin/main", "HEAD")
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
            (worker / "src/a.c").write_text("int a = 1;\n", encoding="utf-8")
            run(worker, "git", "add", "src/a.c")
            run(worker, "git", "commit", "-qm", "recover a")
            record_verification(
                worker,
                "a",
                agent="claude",
                public_gate="pass",
                object_report="build/report.json",
                functions_exact="1/1",
                relocations="exact",
            )
            update_task(worker, "a", agent="claude", status="ready")
            worker_commit = run(worker, "git", "rev-parse", "HEAD")
            run(root, "git", "cherry-pick", worker_commit)
            acquire_resource(root, "integration", agent="integrator", owner="a")
            task = finalize_task(
                root,
                "a",
                agent="integrator",
                retail_gate="pass",
                checksum="pass",
            )
            self.assertEqual(task["status"], "done")
            self.assertEqual(
                task["verification"]["integration_commit"],
                run(root, "git", "rev-parse", "HEAD"),
            )


if __name__ == "__main__":
    unittest.main()
