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
from tools.claim_diff import check
from tools.workspace_policy import AI_WORKSPACE_BRANCH


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


class ClaimDiffTests(unittest.TestCase):
    def test_ownerless_ai_workspace_resource_allows_only_orchestration_paths(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", AI_WORKSPACE_BRANCH)
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "config/recovery").mkdir(parents=True)
            (root / "config/recovery/project.json").write_text(
                "{}\n", encoding="utf-8"
            )
            (root / "src").mkdir()
            (root / "src/a.c").write_text("int a;\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            acquire_resource(root, "integration", agent="orchestrator")

            (root / "config/recovery/project.json").write_text(
                '{"schema_version": 1}\n', encoding="utf-8"
            )
            result = check(root)
            self.assertEqual(result["mode"], "orchestrator")
            self.assertEqual(result["errors"], [])

            (root / "src/a.c").write_text("int a = 1;\n", encoding="utf-8")
            result = check(root)
            self.assertTrue(
                any("src/a.c" in error for error in result["errors"])
            )

    def test_ownerless_resource_does_not_bypass_human_main(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "tools").mkdir()
            (root / "tools/a.py").write_text("value = 0\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            acquire_resource(root, "integration", agent="orchestrator")
            (root / "tools/a.py").write_text("value = 1\n", encoding="utf-8")
            with self.assertRaisesRegex(
                Exception, "integration resource must name one ready task"
            ):
                check(root)

    def test_integration_resource_allows_only_ready_task_paths(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "src").mkdir()
            (root / "src/a.c").write_text("int a;\n", encoding="utf-8")
            (root / "other.txt").write_text("base\n", encoding="utf-8")
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
            claim_task(
                worker,
                "a",
                agent="claude",
                source="src/a.c",
                change_class="documentation",
                base_ref="origin/main",
            )
            record_verification(
                worker, "a", agent="claude", public_gate="pass"
            )
            update_task(worker, "a", agent="claude", status="ready")
            acquire_resource(root, "integration", agent="integrator", owner="a")
            (root / "src/a.c").write_text("int a = 1;\n", encoding="utf-8")
            result = check(root, "origin/main")
            self.assertEqual(result["mode"], "integration")
            self.assertEqual(result["errors"], [])
            (root / "other.txt").write_text("outside\n", encoding="utf-8")
            result = check(root, "origin/main")
            self.assertTrue(
                any("other.txt" in error for error in result["errors"])
            )


if __name__ == "__main__":
    unittest.main()
