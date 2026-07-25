import subprocess
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from tools.agent_queue import (
    QueueError,
    active_tasks,
    add_task,
    claim_task,
    queue_path,
    read_queue,
    release_task,
    update_task,
    validate_queue,
)


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


class AgentQueueTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.base = Path(self.temporary.name)
        self.main = self.base / "main"
        self.main.mkdir()
        run(self.main, "git", "init", "-q")
        run(self.main, "git", "config", "user.email", "test@example.com")
        run(self.main, "git", "config", "user.name", "Test")
        (self.main / "README").write_text("fixture\n", encoding="utf-8")
        run(self.main, "git", "add", "README")
        run(self.main, "git", "commit", "-qm", "fixture")

        self.claude = self.base / "claude"
        self.codex = self.base / "codex"
        run(
            self.main,
            "git",
            "worktree",
            "add",
            "-q",
            "-b",
            "agent/claude-task",
            str(self.claude),
        )
        run(
            self.main,
            "git",
            "worktree",
            "add",
            "-q",
            "-b",
            "agent/codex-task",
            str(self.codex),
        )

    def tearDown(self) -> None:
        subprocess.run(
            ["git", "worktree", "remove", "--force", str(self.claude)],
            cwd=self.main,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        subprocess.run(
            ["git", "worktree", "remove", "--force", str(self.codex)],
            cwd=self.main,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.temporary.cleanup()

    def test_all_worktrees_share_one_queue(self) -> None:
        self.assertEqual(queue_path(self.claude), queue_path(self.codex))
        claim_task(
            self.claude,
            "owner-a",
            agent="claude",
            source="src/a.c",
        )
        queue = read_queue(queue_path(self.codex))
        self.assertEqual(len(active_tasks(queue)), 1)
        self.assertEqual(active_tasks(queue)[0]["agent"], "claude")

    def test_duplicate_owner_and_shared_file_are_blocked(self) -> None:
        claim_task(
            self.claude,
            "owner-a",
            agent="claude",
            source="src/a.c",
            shared_files=["include/game/common.h"],
        )
        with self.assertRaisesRegex(QueueError, "already"):
            claim_task(
                self.codex,
                "owner-a",
                agent="codex",
                source="src/a.c",
            )
        with self.assertRaisesRegex(QueueError, "overlaps"):
            claim_task(
                self.codex,
                "owner-b",
                agent="codex",
                source="src/b.c",
                shared_files=["include/game/common.h"],
            )

    def test_build_directory_conflict_is_blocked(self) -> None:
        claim_task(self.claude, "owner-a", agent="claude", source="src/a.c")
        with self.assertRaisesRegex(QueueError, "build_dir"):
            claim_task(
                self.codex,
                "owner-b",
                agent="codex",
                source="src/b.c",
                build_dir=self.claude / "build",
            )

    def test_pending_update_verification_and_release(self) -> None:
        add_task(
            self.main,
            "owner-a",
            source="src/a.c",
            priority="high",
        )
        claimed = claim_task(self.claude, "owner-a", agent="claude")
        self.assertEqual(claimed["priority"], "normal")
        updated = update_task(
            self.claude,
            "owner-a",
            agent="claude",
            status="verifying",
            add_shared=["include/game/a.h"],
            verified_commit="HEAD",
        )
        self.assertEqual(updated["status"], "verifying")
        self.assertEqual(len(updated["last_verified_commit"]), 40)
        finished = release_task(
            self.claude,
            "owner-a",
            agent="claude",
            status="done",
            verified_commit="HEAD",
        )
        self.assertEqual(finished["status"], "done")
        self.assertEqual(active_tasks(read_queue(queue_path(self.main))), [])

    def test_simultaneous_distinct_claims_are_atomic(self) -> None:
        def claim(worktree: Path, owner: str, agent: str) -> None:
            claim_task(
                worktree,
                owner,
                agent=agent,
                source=f"src/{owner}.c",
            )

        with ThreadPoolExecutor(max_workers=2) as executor:
            first = executor.submit(claim, self.claude, "owner-a", "claude")
            second = executor.submit(claim, self.codex, "owner-b", "codex")
            first.result()
            second.result()

        queue = read_queue(queue_path(self.main))
        self.assertEqual(validate_queue(queue), [])
        self.assertEqual(
            {task["owner"] for task in active_tasks(queue)},
            {"owner-a", "owner-b"},
        )


if __name__ == "__main__":
    unittest.main()
