import contextlib
import json
import os
import sqlite3
import subprocess
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from unittest import mock

from tools.agent_queue import canonical_queue_path
from tools.recovery_memory import (
    RecoveryMemory,
    RecoveryMemoryError,
    _canonical_workflow_root,
    parse_crack_report,
    startup_check,
)
from tools.tests import test_recovery_workflow as recovery_fixture
import tools.recovery_memory as memory_module


SHA_A = "a" * 64
SHA_B = "b" * 64
SHA_C = "c" * 64


class RecoveryMemoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.store = RecoveryMemory(self.root / "recovery-memory.sqlite3")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def identity(self, *, source: str = SHA_A, shape: str | None = None):
        return RecoveryMemory.identity(
            owner="main:board/example",
            function="ExampleExec",
            base_commit="base-commit",
            toolchain_key="mwcc-gc-2.6",
            target_sha256=SHA_B,
            compiler_sha256=SHA_C,
            source_sha256=source,
            shape_key=shape,
            hypothesis="direct consumer",
            axis="lifetime",
        )

    @staticmethod
    def git(root: Path, *args: str) -> str:
        process = subprocess.run(
            ["git", *args],
            cwd=root,
            text=True,
            capture_output=True,
            check=True,
        )
        return process.stdout.strip()

    def split_lane_and_workflow(self) -> tuple[Path, Path]:
        lane = self.root / "lane"
        workflow = self.root / "workflow"
        lane.mkdir()
        (lane / "workflow-marker.txt").write_text(
            "permanent workflow ancestry marker\n", encoding="utf-8"
        )
        self.git(lane, "init", "-q", "-b", "main")
        self.git(lane, "config", "user.email", "test@example.com")
        self.git(lane, "config", "user.name", "Test")
        self.git(lane, "add", "workflow-marker.txt")
        self.git(lane, "commit", "-qm", "permanent workflow marker")
        self.git(
            lane,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        self.git(lane, "worktree", "add", "-q", "-b", "workflow", str(workflow))
        recovery_fixture.RecoveryWorkflowTests().fixture(workflow)
        freshness = workflow / "config/recovery/knowledge_freshness.json"
        freshness.write_text(
            json.dumps({"schema_version": 1, "cards": {}}),
            encoding="utf-8",
        )
        runtime = workflow / "tools/workflow_runtime.py"
        runtime.parent.mkdir(parents=True, exist_ok=True)
        runtime.write_text("WORKFLOW_VERSION = 1\n", encoding="utf-8")
        self.git(workflow, "add", ".")
        self.git(workflow, "commit", "-qm", "add recovery workflow metadata")
        self.git(
            workflow,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        return lane, workflow

    def independent_workflow(self, lane: Path, name: str) -> Path:
        workflow = self.root / name
        permanent = self.git(
            lane,
            "rev-parse",
            "refs/remotes/origin/agent/recovery-context-workflow",
        )
        self.git(
            self.root,
            "clone",
            "-q",
            "--branch",
            "workflow",
            str(lane),
            str(workflow),
        )
        self.git(
            workflow,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            permanent,
        )
        return workflow

    def test_startup_check_uses_authenticated_workflow_root_for_legacy_lane(
        self,
    ) -> None:
        lane, _linked_workflow = self.split_lane_and_workflow()
        workflow = self.independent_workflow(lane, "workflow-independent")
        self.assertFalse((lane / "config/recovery/project.json").exists())
        with self.assertRaisesRegex(
            RecoveryMemoryError, "lane is stale"
        ):
            startup_check(lane, sync_reports=False)
        self.git(
            lane,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )

        result = startup_check(
            lane,
            sync_reports=False,
            workflow_root=workflow,
        )
        self.assertEqual(result["status"], "pass")
        self.assertEqual(result["lane_root"], str(lane.resolve()))
        self.assertEqual(result["workflow_root"], str(workflow.resolve()))
        self.assertNotEqual(
            canonical_queue_path(lane), canonical_queue_path(workflow)
        )
        self.assertEqual(
            Path(result["queue_path"]), canonical_queue_path(lane)
        )
        self.assertEqual(
            Path(result["memory_path"]).parent,
            canonical_queue_path(lane).parent,
        )
        self.assertFalse(canonical_queue_path(workflow).exists())
        self.assertFalse(
            (canonical_queue_path(workflow).parent / "recovery-memory.sqlite3").exists()
        )
        self.assertNotEqual(result["lane_head"], result["permanent_ref_commit"])
        self.assertEqual(
            result["workflow_head"], result["permanent_ref_commit"]
        )
        self.assertEqual(result["merge_base"], result["lane_head"])
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(len(result["workflow_root_sha256"]), 64)
        self.assertIn(
            "config/recovery/project.json", result["workflow_files"]
        )
        self.assertIn(
            "config/recovery/knowledge_freshness.json",
            result["workflow_files"],
        )
        repeated = startup_check(
            lane,
            sync_reports=False,
            workflow_root=workflow,
        )
        self.assertEqual(
            repeated["workflow_root_sha256"], result["workflow_root_sha256"]
        )

    def test_startup_check_uses_workflow_objects_across_unfetched_clone(
        self,
    ) -> None:
        upstream = self.root / "history-upstream"
        lane = self.root / "history-lane"
        workflow = self.root / "history-workflow"
        upstream.mkdir()
        (upstream / "workflow-marker.txt").write_text(
            "shared history\n", encoding="utf-8"
        )
        self.git(upstream, "init", "-q", "-b", "main")
        self.git(upstream, "config", "user.email", "test@example.com")
        self.git(upstream, "config", "user.name", "Test")
        self.git(upstream, "add", "workflow-marker.txt")
        self.git(upstream, "commit", "-qm", "shared base")
        self.git(self.root, "clone", "-q", str(upstream), str(lane))
        self.git(self.root, "clone", "-q", str(upstream), str(workflow))

        self.git(workflow, "config", "user.email", "test@example.com")
        self.git(workflow, "config", "user.name", "Test")
        recovery_fixture.RecoveryWorkflowTests().fixture(workflow)
        (workflow / "config/recovery/knowledge_freshness.json").write_text(
            json.dumps({"schema_version": 1, "cards": {}}),
            encoding="utf-8",
        )
        self.git(workflow, "add", ".")
        self.git(workflow, "commit", "-qm", "new workflow metadata")
        self.git(
            workflow,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        workflow_head = self.git(workflow, "rev-parse", "HEAD")
        lane_head = self.git(lane, "rev-parse", "HEAD")
        unseen = subprocess.run(
            ["git", "cat-file", "-e", f"{workflow_head}^{{commit}}"],
            cwd=lane,
            capture_output=True,
            check=False,
        )
        self.assertNotEqual(unseen.returncode, 0)

        result = startup_check(
            lane,
            sync_reports=False,
            workflow_root=workflow,
        )
        self.assertEqual(result["status"], "pass")
        self.assertEqual(result["lane_head"], lane_head)
        self.assertEqual(result["workflow_head"], workflow_head)
        self.assertEqual(result["permanent_ref_commit"], workflow_head)
        self.assertEqual(result["merge_base"], lane_head)

    def test_startup_check_rejects_relative_and_alias_workflow_roots(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        with self.assertRaisesRegex(RecoveryMemoryError, "absolute canonical"):
            startup_check(
                lane,
                sync_reports=False,
                workflow_root=Path("workflow"),
            )
        alias = workflow / "config" / ".."
        with self.assertRaisesRegex(RecoveryMemoryError, "path alias"):
            startup_check(lane, sync_reports=False, workflow_root=alias)

    def test_workflow_root_rejects_non_worktree_and_symlink(self) -> None:
        not_worktree = self.root / "not-worktree"
        not_worktree.mkdir()
        with self.assertRaisesRegex(RecoveryMemoryError, "not a Git worktree"):
            _canonical_workflow_root(not_worktree)

        lane, workflow = self.split_lane_and_workflow()
        del lane
        with mock.patch.object(Path, "is_symlink", return_value=True):
            with self.assertRaisesRegex(RecoveryMemoryError, "symlink"):
                _canonical_workflow_root(workflow)

    def test_startup_check_rejects_unrelated_workflow_history(self) -> None:
        lane, _workflow = self.split_lane_and_workflow()
        unrelated = self.root / "workflow-unrelated"
        unrelated.mkdir()
        recovery_fixture.RecoveryWorkflowTests().fixture(unrelated)
        (unrelated / "config/recovery/knowledge_freshness.json").write_text(
            json.dumps({"schema_version": 1, "cards": {}}),
            encoding="utf-8",
        )
        self.git(unrelated, "init", "-q", "-b", "workflow")
        self.git(unrelated, "config", "user.email", "test@example.com")
        self.git(unrelated, "config", "user.name", "Test")
        self.git(unrelated, "add", ".")
        self.git(unrelated, "commit", "-qm", "unrelated workflow")
        self.git(
            unrelated,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        with self.assertRaisesRegex(
            RecoveryMemoryError, "deterministic common merge-base"
        ):
            startup_check(lane, sync_reports=False, workflow_root=unrelated)

    def test_startup_check_rejects_stale_workflow_root(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        permanent = self.git(
            lane,
            "rev-parse",
            "refs/remotes/origin/agent/recovery-context-workflow",
        )
        stale = self.root / "workflow-stale"
        self.git(
            self.root,
            "clone",
            "-q",
            "--branch",
            "workflow",
            str(lane),
            str(stale),
        )
        self.git(stale, "config", "user.email", "test@example.com")
        self.git(stale, "config", "user.name", "Test")
        self.git(stale, "checkout", "-q", "--orphan", "stale-workflow")
        self.git(stale, "rm", "-q", "-rf", ".")
        recovery_fixture.RecoveryWorkflowTests().fixture(stale)
        (stale / "config/recovery/knowledge_freshness.json").write_text(
            json.dumps({"schema_version": 1, "cards": {}}),
            encoding="utf-8",
        )
        self.git(stale, "add", ".")
        self.git(stale, "commit", "-qm", "unrelated workflow metadata")
        self.git(
            stale,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            permanent,
        )
        with self.assertRaisesRegex(RecoveryMemoryError, "must equal released"):
            startup_check(lane, sync_reports=False, workflow_root=stale)

    def test_startup_check_rejects_workflow_head_ahead_of_release(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        (workflow / "tools/workflow_runtime.py").write_text(
            "WORKFLOW_VERSION = 2\n", encoding="utf-8"
        )
        self.git(workflow, "add", "tools/workflow_runtime.py")
        self.git(workflow, "commit", "-qm", "unreleased workflow change")
        with self.assertRaisesRegex(RecoveryMemoryError, "must equal released"):
            startup_check(lane, sync_reports=False, workflow_root=workflow)

    def test_startup_check_rejects_dirty_tracked_workflow_tooling(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        (workflow / "tools/workflow_runtime.py").write_text(
            "WORKFLOW_VERSION = 99\n", encoding="utf-8"
        )
        with self.assertRaisesRegex(RecoveryMemoryError, "worktree is not clean"):
            startup_check(lane, sync_reports=False, workflow_root=workflow)

    def test_startup_check_rejects_untracked_workflow_tooling(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        (workflow / "tools/untracked_runtime.py").write_text(
            "UNRELEASED = True\n", encoding="utf-8"
        )
        with self.assertRaisesRegex(RecoveryMemoryError, "worktree is not clean"):
            startup_check(lane, sync_reports=False, workflow_root=workflow)

    def test_default_startup_preserves_dirty_lane_compatibility(self) -> None:
        lane = self.root / "default-lane"
        lane.mkdir()
        recovery_fixture.RecoveryWorkflowTests().fixture(lane)
        self.git(lane, "init", "-q", "-b", "main")
        self.git(lane, "config", "user.email", "test@example.com")
        self.git(lane, "config", "user.name", "Test")
        self.git(lane, "add", ".")
        self.git(lane, "commit", "-qm", "lane metadata")
        self.git(
            lane,
            "update-ref",
            "refs/remotes/origin/agent/recovery-context-workflow",
            "HEAD",
        )
        untracked = lane / "tools/local_probe.py"
        untracked.parent.mkdir(parents=True, exist_ok=True)
        untracked.write_text("LOCAL = True\n", encoding="utf-8")
        result = startup_check(lane, sync_reports=False)
        self.assertEqual(result["status"], "pass")

    def test_startup_check_rejects_tampered_workflow_metadata(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        patterns = workflow / "config/recovery/compiler_patterns.json"
        self.git(
            workflow,
            "update-index",
            "--assume-unchanged",
            "config/recovery/compiler_patterns.json",
        )
        patterns.write_text(
            patterns.read_text(encoding="utf-8") + "\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(
            RecoveryMemoryError, "does not match authenticated HEAD"
        ):
            startup_check(lane, sync_reports=False, workflow_root=workflow)

    def test_startup_check_rejects_missing_workflow_metadata(self) -> None:
        lane, workflow = self.split_lane_and_workflow()
        (workflow / "config/recovery/knowledge_freshness.json").unlink()
        with self.assertRaisesRegex(
            RecoveryMemoryError, "worktree is not clean"
        ):
            startup_check(lane, sync_reports=False, workflow_root=workflow)

    def test_admission_is_shared_and_record_is_deduplicated(self) -> None:
        identity = self.identity()
        admitted = self.store.admit(identity, requester="lane-a")
        self.assertEqual(admitted["status"], "admitted")
        blocked = self.store.admit(identity, requester="lane-b")
        self.assertEqual(blocked["status"], "pending_in_other_lane")
        recorded = self.store.record(
            identity,
            requester="lane-a",
            object_sha256=SHA_C,
            status="exact",
            reason="zero rows",
            admission_token=admitted["admission_token"],
        )
        self.assertEqual(recorded["status"], "recorded")
        known = self.store.admit(identity, requester="lane-b")
        self.assertEqual(known["status"], "known_global_source")
        self.assertTrue(known["skip_compile"])
        unchanged = self.store.record(
            identity,
            requester="lane-b",
            object_sha256=SHA_C,
            status="exact",
            reason="zero rows",
        )
        self.assertEqual(unchanged["status"], "unchanged")

    def test_retained_verification_binds_target_and_recovery_record_digest(self) -> None:
        identity = self.identity()
        admitted = self.store.admit(identity, requester="lane-a")
        recorded = self.store.record(
            identity,
            requester="lane-a",
            object_sha256=SHA_C,
            status="exact",
            reason="zero rows",
            admission_token=admitted["admission_token"],
            candidate_record_sha256="d" * 64,
        )
        experiment = dict(recorded["experiment"])
        binding = {
            "input_key": experiment["input_key"],
            "owner": experiment["owner"],
            "function": experiment["function_name"],
            "target_sha256": experiment["target_sha256"],
            "source_sha256": experiment["source_sha256"],
            "object_sha256": experiment["object_sha256"],
            "candidate_record_sha256": experiment["candidate_record_sha256"],
            "status": experiment["status"],
            "record_sha256": experiment["record_sha256"],
        }
        self.assertEqual(self.store.verify_retained(**binding), experiment)
        self.assertTrue(self.store.retained_matches(**binding))

        for label, overrides in (
            ("target", {"target_sha256": "e" * 64}),
            ("record", {"record_sha256": "f" * 64}),
        ):
            with self.subTest(label=label):
                forged = {**binding, **overrides}
                self.assertFalse(self.store.retained_matches(**forged))
                with self.assertRaises(RecoveryMemoryError):
                    self.store.invalidate_retained(**forged)
                with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
                    self.assertEqual(
                        connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0],
                        1,
                    )

    def test_unretained_attempts_and_consumed_admissions_do_not_accumulate(self) -> None:
        for index in range(8):
            identity = self.identity(source=f"{index + 10:064x}")
            admitted = self.store.admit(identity, requester="lane-a")
            self.assertEqual(admitted["status"], "admitted")
            discarded = self.store.discard(
                identity, requester="lane-a",
                admission_token=admitted["admission_token"],
            )
            self.assertEqual(discarded["status"], "discarded")
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM admissions").fetchone()[0], 0)
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 0)

        latest = None
        for index in range(8):
            identity = self.identity(source=f"{index + 100:064x}")
            latest = self.store.admit(identity, requester="lane-a")
            self.assertEqual(latest["status"], "admitted")
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM admissions").fetchone()[0], 1)
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 0)
        assert latest is not None
        self.store.discard(identity, requester="lane-a", admission_token=latest["admission_token"])

        identity = self.identity(source="f" * 64)
        admitted = self.store.admit(identity, requester="lane-a")
        self.store.record(
            identity, requester="lane-a", object_sha256=SHA_C,
            status="improved", reason="retained non-regressed gain",
            admission_token=admitted["admission_token"],
        )
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM admissions").fetchone()[0], 0)
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 1)

    def test_record_without_precompile_admission_fails(self) -> None:
        with self.assertRaisesRegex(RecoveryMemoryError, "no pending central"):
            self.store.record(
                self.identity(),
                requester="lane-a",
                object_sha256=SHA_C,
                status="improved",
                reason="candidate measured",
            )

    def test_queue_task_suffix_normalizes_to_owner_namespace(self) -> None:
        identity = RecoveryMemory.identity(
            owner="main:board/example#full-owner-closure-v1",
            function="ExampleExec",
            base_commit="base-commit",
            toolchain_key="mwcc-gc-2.6",
            target_sha256=SHA_B,
            compiler_sha256=SHA_C,
            source_sha256=SHA_A,
        )
        self.assertEqual(identity["owner"], "main:board/example")

    def test_direct_unretained_records_never_create_experiments(self) -> None:
        for index, status in enumerate(("no_gain", "failed", "regressed") * 4):
            identity = self.identity(source=f"{index + 300:064x}")
            admission = self.store.admit(identity, requester="lane-a")
            with self.assertRaisesRegex(
                RecoveryMemoryError, "retained improved/exact outcomes only"
            ):
                self.store.record(
                    identity, requester="lane-a", object_sha256=SHA_C,
                    status=status, reason="not retained",
                    admission_token=admission["admission_token"],
                )
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 0)
            self.assertLessEqual(connection.execute("SELECT COUNT(*) FROM admissions").fetchone()[0], 1)

    def test_oversized_memory_and_historical_payload_fail_closed(self) -> None:
        oversized = self.root / "oversized.sqlite3"
        with oversized.open("wb") as handle:
            handle.seek(memory_module.MAX_RECOVERY_MEMORY_BYTES)
            handle.write(b"x")
        with self.assertRaisesRegex(RecoveryMemoryError, "64 MiB"):
            RecoveryMemory(oversized)

        identity = self.identity(source="9" * 64)
        with self.assertRaisesRegex(RecoveryMemoryError, "64 KiB"):
            self.store.import_historical_experiment(
                identity, object_sha256=SHA_C, status="improved",
                reason="x" * (memory_module.MAX_COMPACT_FIELD_BYTES + 1),
                candidate_id="large", candidate_record_sha256="8" * 64,
                strict_report_sha256=None, data_report_sha256=None,
                workspace="lane/workbench", source_path="candidate.c",
            )
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 0)

    def test_central_path_environment_overrides_are_rejected(self) -> None:
        lane = self.root / "env-lane"
        lane.mkdir()
        self.git(lane, "init", "-q")
        for variable in ("MP6_RECOVERY_MEMORY", "MP6_AGENT_QUEUE", "GIT_DIR"):
            with self.subTest(variable=variable), mock.patch.dict(
                os.environ, {variable: str(self.root / "shadow")}, clear=False,
            ):
                with self.assertRaisesRegex(RecoveryMemoryError, "fixed"):
                    RecoveryMemory.for_root(lane)

    def test_git_hook_index_environment_does_not_redirect_central_path(self) -> None:
        lane = self.root / "hook-lane"
        lane.mkdir()
        self.git(lane, "init", "-q")
        hook_index = lane / ".git" / "index.lock"
        with mock.patch.dict(
            os.environ, {"GIT_INDEX_FILE": str(hook_index)}, clear=False,
        ):
            resolved = memory_module.recovery_memory_path(lane)
        self.assertEqual(
            resolved,
            lane / ".git" / "agent-coordination" / "recovery-memory.sqlite3",
        )

    def test_git_hook_canonical_git_dir_is_accepted(self) -> None:
        lane = self.root / "git-dir-hook-lane"
        lane.mkdir()
        self.git(lane, "init", "-q")
        git_dir = self.git(
            lane, "rev-parse", "--path-format=absolute", "--git-dir",
        )
        with mock.patch.dict(os.environ, {"GIT_DIR": git_dir}, clear=False):
            resolved = memory_module.recovery_memory_path(lane)
        self.assertEqual(
            resolved,
            lane / ".git" / "agent-coordination" / "recovery-memory.sqlite3",
        )

    def test_historical_import_is_retained_only_latest_and_nonappend(self) -> None:
        identity = self.identity()
        first = self.store.import_historical_experiment(
            identity,
            object_sha256=SHA_B,
            status="improved",
            reason="first retained record",
            candidate_id="c001",
            candidate_record_sha256="d" * 64,
            strict_report_sha256=None,
            data_report_sha256=None,
            workspace="lane-a/workbench",
            source_path="lane-a/candidate.c",
        )
        self.assertEqual(first["status"], "imported")
        with self.assertRaisesRegex(RecoveryMemoryError, "append-only"):
            self.store.import_historical_experiment(
                identity, object_sha256=SHA_C, status="exact",
                reason="second retained record", candidate_id="c002",
                candidate_record_sha256="e" * 64,
                strict_report_sha256=None, data_report_sha256=None,
                workspace="lane-b/workbench", source_path="lane-b/candidate.c",
            )
        latest_identity = self.identity(source="d" * 64)
        latest = self.store.import_historical_experiment(
            latest_identity, object_sha256=SHA_C, status="exact",
            reason="latest retained record", candidate_id="c003",
            candidate_record_sha256="f" * 64,
            strict_report_sha256=None, data_report_sha256=None,
            workspace="lane-c/workbench", source_path="lane-c/candidate.c",
        )
        self.assertEqual(latest["status"], "imported")
        with contextlib.closing(sqlite3.connect(self.store.path)) as connection:
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiments").fetchone()[0], 1)
            self.assertEqual(connection.execute("SELECT COUNT(*) FROM experiment_observations").fetchone()[0], 1)

    def test_concurrent_admission_has_one_owner(self) -> None:
        identity = self.identity()
        with ThreadPoolExecutor(max_workers=2) as pool:
            results = list(
                pool.map(
                    lambda lane: self.store.admit(identity, requester=lane),
                    ("lane-a", "lane-b"),
                )
            )
        statuses = sorted(item["status"] for item in results)
        self.assertEqual(statuses, ["admitted", "pending_in_other_lane"])

    def test_json_crack_report_is_distilled_idempotently(self) -> None:
        report = self.root / "CRACK_REPORT_ExampleExec.json"
        report.write_text(
            json.dumps(
                {
                    "schema": "CRACK_REPORT/v1",
                    "owner": "main:board/example",
                    "function": "ExampleExec",
                    "result": {
                        "strict_percent": 100.0,
                        "data_percent": 100.0,
                        "target_bytes": 12,
                        "candidate_bytes": 12,
                    },
                    "chronological_attempt_ledger": [
                        {
                            "id": "c001",
                            "result": "regressed",
                            "decision": "rejected",
                        },
                        {
                            "id": "c002",
                            "result": "exact",
                            "decision": "retained",
                        },
                    ],
                    "causal_explanation": "A direct consumer removed one owner.",
                    "generalized_improvement_request": {
                        "title": "rank direct consumers",
                        "requested_behavior": "Query exact siblings first.",
                    },
                }
            ),
            encoding="utf-8",
        )
        first = self.store.ingest_report(report)
        second = self.store.ingest_report(report)
        self.assertEqual(first["status"], "ingested")
        self.assertEqual(first["constraints"], 3)
        self.assertEqual(second["status"], "unchanged")
        context = self.store.context_memory(
            "main:board/example", "ExampleExec"
        )
        self.assertEqual(len(context["reports"]), 1)
        self.assertEqual(len(context["reports"][0]["constraints"]), 3)

    def test_markdown_crack_report_is_parsed(self) -> None:
        report = self.root / "ExampleExec_CRACK_REPORT_v1.md"
        report.write_text(
            """CRACK_REPORT/v1

Owner: main:board/example
Function: ExampleExec
Result: strict 100%, data 100%, 4/4 bytes.

## Retained natural C
Use the live typed result directly.

## Causal explanation
The named temporary changed allocation.

## Generalized improvement
Rank direct consumers before declaration permutations.
""",
            encoding="utf-8",
        )
        parsed = parse_crack_report(report, report.read_bytes())
        self.assertEqual(parsed["owner"], "main:board/example")
        self.assertEqual(parsed["function"], "ExampleExec")
        result = self.store.ingest_report(report)
        self.assertEqual(result["status"], "ingested")

    def test_nonexact_report_is_rejected(self) -> None:
        report = self.root / "CRACK_REPORT_bad.json"
        report.write_text(
            json.dumps(
                {
                    "schema": "CRACK_REPORT/v1",
                    "owner": "main:board/example",
                    "function": "Bad",
                    "result": {"strict_percent": 99.0, "data_percent": 100.0},
                }
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(RecoveryMemoryError, "completed exact"):
            self.store.ingest_report(report)


if __name__ == "__main__":
    unittest.main()
