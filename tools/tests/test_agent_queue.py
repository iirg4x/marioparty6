import subprocess
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from tools.agent_queue import (
    QueueError,
    acquire_resource,
    active_tasks,
    add_task,
    check_diff_claim,
    claim_next,
    claim_task,
    queue_path,
    read_queue,
    record_verification,
    release_resource,
    release_task,
    update_task,
    validate_queue,
)


def run(cwd: Path, *args: str) -> str:
    process = subprocess.run(
        args,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    )
    return process.stdout.strip()


class AgentQueueTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.base = Path(self.temporary.name)
        self.main = self.base / "main"
        self.main.mkdir()
        run(self.main, "git", "init", "-q", "-b", "main")
        run(self.main, "git", "config", "user.email", "test@example.com")
        run(self.main, "git", "config", "user.name", "Test")
        (self.main / "src").mkdir()
        (self.main / "include").mkdir()
        (self.main / "src/a.c").write_text("int a;\n", encoding="utf-8")
        (self.main / "src/b.c").write_text("int b;\n", encoding="utf-8")
        (self.main / "include/common.h").write_text(
            "#pragma once\n", encoding="utf-8"
        )
        run(self.main, "git", "add", ".")
        run(self.main, "git", "commit", "-qm", "base")
        run(self.main, "git", "remote", "add", "origin", str(self.main))
        run(self.main, "git", "update-ref", "refs/remotes/origin/main", "HEAD")
        self.claude = self.base / "claude"
        self.codex = self.base / "codex"
        run(
            self.main,
            "git",
            "worktree",
            "add",
            "-q",
            "-b",
            "agent/claude",
            str(self.claude),
        )
        run(
            self.main,
            "git",
            "worktree",
            "add",
            "-q",
            "-b",
            "agent/codex",
            str(self.codex),
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def test_priority_is_preserved(self) -> None:
        add_task(self.main, "a", source="src/a.c", priority="high")
        task = claim_task(self.claude, "a", agent="claude")
        self.assertEqual(task["priority"], "high")

    def test_worktree_and_build_validation(self) -> None:
        with self.assertRaisesRegex(QueueError, "build directory"):
            claim_task(
                self.claude,
                "a",
                agent="claude",
                source="src/a.c",
                build_dir=self.base / "shared-build",
            )
        with self.assertRaisesRegex(QueueError, "branch"):
            claim_task(self.main, "a", agent="claude", source="src/a.c")

    def test_diff_must_be_declared(self) -> None:
        claim_task(self.claude, "a", agent="claude", source="src/a.c")
        (self.claude / "src/a.c").write_text("int a = 1;\n", encoding="utf-8")
        (self.claude / "include/common.h").write_text(
            "#define X 1\n", encoding="utf-8"
        )
        result = check_diff_claim(
            self.claude, base="origin/main", agent="claude"
        )
        self.assertTrue(
            any("include/common.h" in error for error in result["errors"])
        )
        update_task(
            self.claude,
            "a",
            agent="claude",
            add_shared=["include/common.h"],
        )
        result = check_diff_claim(
            self.claude, base="origin/main", agent="claude"
        )
        self.assertEqual(result["errors"], [])

    def test_verification_is_bound_to_clean_head(self) -> None:
        claim_task(
            self.claude,
            "a",
            agent="claude",
            source="src/a.c",
            change_class="documentation",
        )
        (self.claude / "src/a.c").write_text("int a = 1;\n", encoding="utf-8")
        run(self.claude, "git", "add", "src/a.c")
        run(self.claude, "git", "commit", "-qm", "change")
        task = record_verification(
            self.claude, "a", agent="claude", public_gate="pass"
        )
        self.assertEqual(
            task["verification"]["verified_commit"],
            run(self.claude, "git", "rev-parse", "HEAD"),
        )
        (self.claude / "src/a.c").write_text("int a = 2;\n", encoding="utf-8")
        with self.assertRaisesRegex(QueueError, "clean"):
            release_task(self.claude, "a", agent="claude", status="done")
        run(self.claude, "git", "checkout", "--", "src/a.c")
        done = release_task(self.claude, "a", agent="claude", status="done")
        self.assertEqual(done["status"], "done")

    def test_source_task_needs_retail_proof_for_done(self) -> None:
        claim_task(self.claude, "a", agent="claude", source="src/a.c")
        record_verification(
            self.claude,
            "a",
            agent="claude",
            public_gate="pass",
            object_report="build/report.json",
            functions_exact="1/1",
            relocations="exact",
        )
        with self.assertRaisesRegex(QueueError, "retail_gate"):
            release_task(self.claude, "a", agent="claude", status="done")

    def test_dependencies_and_claim_next(self) -> None:
        add_task(self.main, "a", source="src/a.c", priority="high")
        add_task(
            self.main,
            "b",
            source="src/b.c",
            depends_on=["a"],
            priority="critical",
        )
        first = claim_next(self.claude, agent="claude")
        self.assertEqual(first["owner"], "a")

    def test_resource_lock(self) -> None:
        record = acquire_resource(
            self.claude, "retail-build", agent="claude", owner="a"
        )
        self.assertEqual(record["agent"], "claude")
        with self.assertRaisesRegex(QueueError, "held"):
            acquire_resource(
                self.codex, "retail-build", agent="codex", owner="b"
            )
        release_resource(self.claude, "retail-build", agent="claude")

    def test_simultaneous_distinct_claims_are_atomic(self) -> None:
        def claim(path: Path, owner: str, agent: str, source: str) -> None:
            claim_task(path, owner, agent=agent, source=source)

        with ThreadPoolExecutor(max_workers=2) as pool:
            first = pool.submit(
                claim, self.claude, "a", "claude", "src/a.c"
            )
            second = pool.submit(
                claim, self.codex, "b", "codex", "src/b.c"
            )
            first.result()
            second.result()
        queue = read_queue(queue_path(self.main))
        self.assertEqual(validate_queue(queue), [])
        self.assertEqual(
            {task["owner"] for task in active_tasks(queue)}, {"a", "b"}
        )


if __name__ == "__main__":
    unittest.main()
