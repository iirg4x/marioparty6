import json
import subprocess
import tempfile
import unittest
import warnings
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from unittest import mock

import tools.agent_queue as agent_queue

from tools.agent_queue import (
    QueueError,
    QUEUE_AUDIT_PENDING_SUFFIX,
    QUEUE_AUDIT_SUFFIX,
    QUEUE_BACKUP_SUFFIX,
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
    locked_queue,
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
        run(
            self.main,
            "git",
            "update-ref",
            "refs/heads/agent/recovery-context-workflow",
            "HEAD",
        )
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

    def test_claimed_source_rejects_second_open_task(self) -> None:
        add_task(self.main, "a", source="src/a.c", priority="high")
        claim_task(self.claude, "a", agent="claude")
        with self.assertRaisesRegex(QueueError, "write path.*overlaps"):
            add_task(self.main, "duplicate-a", source="src/a.c")

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

    def test_base_ref_repin_stores_resolved_commit(self) -> None:
        claim_task(self.claude, "a", agent="claude", source="src/a.c")
        pinned = run(
            self.claude, "git", "rev-parse", "agent/recovery-context-workflow"
        )
        task = update_task(
            self.claude,
            "a",
            agent="claude",
            base_ref="agent/recovery-context-workflow",
        )
        self.assertEqual(task["base_ref"], pinned)
        with self.assertRaises(QueueError):
            update_task(
                self.claude, "a", agent="claude", base_ref="no-such-ref"
            )

    def test_base_ref_repin_clears_stale_default_base(self) -> None:
        claim_task(self.claude, "a", agent="claude", source="src/a.c")
        (self.claude / "src/b.c").write_text("int b = 2;\n", encoding="utf-8")
        run(self.claude, "git", "add", "src/b.c")
        run(self.claude, "git", "commit", "-qm", "landed by an earlier task")
        result = check_diff_claim(self.claude, agent="claude")
        self.assertTrue(
            any("src/b.c" in error for error in result["errors"])
        )
        head = run(self.claude, "git", "rev-parse", "HEAD")
        task = update_task(self.claude, "a", agent="claude", base_ref=head)
        self.assertEqual(task["base_ref"], head)
        result = check_diff_claim(self.claude, agent="claude")
        self.assertEqual(result["errors"], [])

    def test_base_ref_repin_requires_ancestor(self) -> None:
        claim_task(self.claude, "a", agent="claude", source="src/a.c")
        (self.codex / "src/b.c").write_text("int b = 3;\n", encoding="utf-8")
        run(self.codex, "git", "add", "src/b.c")
        run(self.codex, "git", "commit", "-qm", "divergent")
        stray = run(self.codex, "git", "rev-parse", "HEAD")
        with self.assertRaisesRegex(QueueError, "not an ancestor"):
            update_task(self.claude, "a", agent="claude", base_ref=stray)

    def test_base_ref_is_frozen_after_verification(self) -> None:
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
        record_verification(
            self.claude, "a", agent="claude", public_gate="pass"
        )
        with self.assertRaisesRegex(QueueError, "frozen"):
            update_task(self.claude, "a", agent="claude", base_ref="HEAD")

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

    def test_invalid_queue_is_preserved_as_unique_evidence(self) -> None:
        path = self.base / "queue.json"
        path.write_bytes(b"\x00" * 8)
        with self.assertRaisesRegex(QueueError, "all-NUL"):
            read_queue(path)
        evidence = list(path.parent.glob("queue.json.corrupt.*"))
        self.assertEqual(len(evidence), 1)
        self.assertEqual(evidence[0].read_bytes(), b"\x00" * 8)
        with self.assertRaises(QueueError):
            read_queue(path)
        self.assertEqual(len(list(path.parent.glob("queue.json.corrupt.*"))), 1)

    def test_schema_v1_migrates_without_dropping_records(self) -> None:
        path = self.base / "queue.json"
        path.write_text(
            json.dumps(
                {
                    "updated_at": "2024-01-01T00:00:00+00:00",
                    "tasks": [{"id": "one", "owner": "one", "priority": "normal"}],
                }
            ),
            encoding="utf-8",
        )
        queue = read_queue(path)
        self.assertEqual(queue["schema_version"], 2)
        self.assertEqual([task["id"] for task in queue["tasks"]], ["one"])
        self.assertEqual(queue["resources"], {})

    def test_malformed_task_and_resource_are_rejected(self) -> None:
        path = self.base / "queue.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "updated_at": "2024-01-01T00:00:00+00:00",
                    "tasks": [None],
                    "resources": {"x": "not-an-object"},
                }
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(QueueError, "invalid queue schema") as raised:
            read_queue(path)
        self.assertIn("tasks[0]", str(raised.exception))
        self.assertIn("resources", str(raised.exception))

    def test_malformed_task_field_type_is_rejected_before_migration(self) -> None:
        path = self.base / "queue.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "tasks": [{"id": 7, "owner": "bad", "status": "done"}],
                    "resources": {},
                }
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(QueueError, r"tasks\[0\].id"):
            read_queue(path)

    def test_incomplete_resource_record_is_rejected(self) -> None:
        path = self.base / "queue.json"
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "updated_at": "2024-01-01T00:00:00+00:00",
                    "tasks": [],
                    "resources": {"x": {}},
                }
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(QueueError, "resources"):
            read_queue(path)

    def test_backup_audit_and_noop_lock(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        first = path.read_bytes()
        with locked_queue(path):
            pass
        self.assertEqual(path.read_bytes(), first)
        queue = read_queue(path)
        queue["tasks"][0]["note"] = "changed"
        with locked_queue(path) as locked:
            locked["tasks"][0]["note"] = "changed"
        self.assertEqual((path.with_name(path.name + QUEUE_BACKUP_SUFFIX)).read_bytes(), first)
        audit = path.with_name(path.name + QUEUE_AUDIT_SUFFIX)
        records = [json.loads(line) for line in audit.read_text(encoding="utf-8").splitlines()]
        self.assertGreaterEqual(len(records), 2)
        self.assertEqual(records[-1]["old_sha256"], __import__("hashlib").sha256(first).hexdigest())
        self.assertTrue(records[-1]["changed_tasks"])
        self.assertFalse(path.with_name(path.name + QUEUE_AUDIT_PENDING_SUFFIX).exists())

    def test_audit_failure_leaves_pending_without_rollback(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        before = path.read_bytes()
        real_open = Path.open

        def fail_audit(handle: Path, *args: object, **kwargs: object):
            if str(handle).endswith(QUEUE_AUDIT_SUFFIX):
                raise OSError("audit unavailable")
            return real_open(handle, *args, **kwargs)

        with mock.patch.object(Path, "open", new=fail_audit):
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "pending"
        self.assertTrue(any("queue committed" in str(item.message) for item in caught))
        self.assertNotEqual(path.read_bytes(), before)
        self.assertTrue(path.with_name(path.name + QUEUE_AUDIT_PENDING_SUFFIX).exists())

    def test_pending_replay_deduplicates_audit_append(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        real_unlink = Path.unlink

        def keep_pending(handle: Path, *args: object, **kwargs: object) -> None:
            if str(handle).endswith(QUEUE_AUDIT_PENDING_SUFFIX):
                raise OSError("pending cleanup unavailable")
            real_unlink(handle, *args, **kwargs)

        with mock.patch.object(Path, "unlink", new=keep_pending):
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "pending-cleanup"
        self.assertTrue(any("audit pending" in str(item.message) for item in caught))
        audit = path.with_name(path.name + QUEUE_AUDIT_SUFFIX)
        first_records = audit.read_text(encoding="utf-8").splitlines()
        self.assertTrue(path.with_name(path.name + QUEUE_AUDIT_PENDING_SUFFIX).exists())
        with locked_queue(path) as queue:
            queue["tasks"][0]["note"] = "replayed"
        second_records = audit.read_text(encoding="utf-8").splitlines()
        self.assertEqual(len(second_records), len(first_records) + 1)

    def test_candidate_hash_failure_preserves_evidence_and_original(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        before = path.read_bytes()
        real_read = Path.read_bytes

        def tamper(handle: Path) -> bytes:
            data = real_read(handle)
            return b"tampered" if ".candidate-" in handle.name else data

        with mock.patch.object(Path, "read_bytes", new=tamper):
            with self.assertRaisesRegex(QueueError, "candidate.*hash mismatch"):
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "tampered"
        self.assertEqual(path.read_bytes(), before)
        self.assertTrue(list(path.parent.glob(f".{path.name}.candidate-*.json")))

    def test_replace_failure_preserves_original(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        before = path.read_bytes()
        real_replace = agent_queue.os.replace

        def fail_queue_replace(source: str | bytes, target: str | bytes) -> None:
            if Path(target).resolve() == path.resolve():
                raise OSError("replace unavailable")
            real_replace(source, target)

        with mock.patch.object(agent_queue.os, "replace", new=fail_queue_replace):
            with self.assertRaisesRegex(QueueError, "atomic queue replace failed"):
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "replace"
        self.assertEqual(path.read_bytes(), before)
        self.assertTrue(list(path.parent.glob("*.candidate.*")))

    def test_backup_temp_hash_failure_leaves_queue_and_backup_unchanged(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        before = path.read_bytes()
        backup = path.with_name(path.name + QUEUE_BACKUP_SUFFIX)
        real_read = Path.read_bytes

        def tamper_backup(handle: Path) -> bytes:
            data = real_read(handle)
            return b"bad backup" if ".backup-" in handle.name else data

        with mock.patch.object(Path, "read_bytes", new=tamper_backup):
            with self.assertRaisesRegex(QueueError, "backup.*hash mismatch"):
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "backup"
        self.assertEqual(path.read_bytes(), before)
        self.assertFalse(backup.exists())

    def test_post_replace_failure_rolls_back_and_preserves_corrupt_bytes(self) -> None:
        path = self.base / "queue.json"
        add_task(self.main, "one", queue_file=path, source="src/a.c")
        before = path.read_bytes()
        real_read_queue = agent_queue.read_queue
        calls = 0

        def fail_post_check(handle: Path):
            nonlocal calls
            calls += 1
            if calls == 1:
                return real_read_queue(handle)
            raise QueueError("post-check failed")

        with mock.patch.object(agent_queue, "read_queue", new=fail_post_check):
            with self.assertRaisesRegex(QueueError, "post-replace queue validation failed"):
                with locked_queue(path) as queue:
                    queue["tasks"][0]["note"] = "post-check"
        self.assertEqual(path.read_bytes(), before)
        self.assertTrue(list(path.parent.glob("queue.json.corrupt.*")))


if __name__ == "__main__":
    unittest.main()
