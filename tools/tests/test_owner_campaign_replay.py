from __future__ import annotations

import hashlib
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock

from tools import owner_campaign_replay as replay


def digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def digest_json(value: object) -> str:
    return digest(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode())


def _init_fixture_git(root: Path, runner) -> None:
    """Create a byte-stable fixture repository independent of user Git config."""

    runner("init", "-q")
    runner("config", "core.autocrlf", "false")


class FakeRuntime:
    def __init__(self) -> None:
        self.calls: list[str] = []
        self.report: Path | None = None
        self.descriptor: dict[str, object] | None = None

    def load_campaign(self, root: Path, path: Path) -> dict[str, object]:
        self.calls.append("load")
        value = json.loads(path.read_text(encoding="utf-8"))
        return {**value, "_source": root / value["source_relpath"]}

    def snapshot_frontier(self, root: Path, campaign: dict[str, object], function: str) -> dict[str, object]:
        self.calls.append("snapshot")
        return {"function": function, "frontier_sha256": "a" * 64}

    def run_candidate(self, root: Path, campaign: dict[str, object], descriptor: Path) -> dict[str, object]:
        self.calls.append("candidate")
        value = json.loads(descriptor.read_text(encoding="utf-8"))
        self.descriptor = value
        candidate = root / value["candidate_source"]["path"]
        # The production runtime compiles from its worker scratch tree and
        # leaves the replay root's bound baseline source untouched.  Reading
        # the candidate here also proves the descriptor path is usable.
        candidate.read_bytes()
        body = {
            "schema": replay.REPORT_SCHEMA,
            "status": "exact",
            "completed": True,
            "authority_advanced": False,
            "owner": campaign["owner"],
            "function": campaign["functions"][0],
            "campaign_id": campaign["campaign_id"],
            "manifest_sha256": campaign["manifest_sha256"],
            "frontier_sha256": "a" * 64,
            "source_sha256": value["candidate_source"]["sha256"],
            "target_object_sha256": campaign["target_object"]["sha256"],
            "candidate_object_sha256": campaign["target_object"]["sha256"],
            "result": {
                "strict_percent": 100,
                "data_percent": 100,
                "target_bytes": 4,
                "candidate_bytes": 4,
                "physical_differences": 0,
                "protected_total": 0,
                "protected_losses": 0,
                "source_link_exact": True,
            },
            "proof_receipts": {key: "b" * 64 for key in ("strict", "data", "physical", "siblings", "source_link")},
            "completed_at": "2026-08-31T00:00:00Z",
        }
        report = {**body, "report_sha256": digest_json(body)}
        path = root / "build/owner-campaign/reports/report.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(report, sort_keys=True), encoding="utf-8")
        self.report = path
        metrics = {
            "strict": {"target_bytes": 4, "candidate_bytes": 4, "differences": 0},
            "data": {"target_bytes": 4, "candidate_bytes": 4, "differences": 0},
            "physical_target_count": 1,
            "physical_candidate_count": 1,
            "physical_differences": 0,
            "protected_total": 0,
            "protected_losses": 0,
            "source_link_exact": True,
        }
        return {
            "status": "exact",
            "metrics": metrics,
            "exact": {"report_path": str(path), "report_sha256": report["report_sha256"]},
        }


class ReplayTests(unittest.TestCase):
    def _git_run(self, *args: str) -> subprocess.CompletedProcess[bytes]:
        """Run fixture Git through the replay's native, bounded resolver."""

        return replay._run(
            replay._git_argv(*args),
            cwd=self.root,
            timeout=30,
        )

    def _git_output(self, *args: str) -> str:
        return self._git_run(*args).stdout.decode("utf-8", "replace")

    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name) / "repo"
        self.root.mkdir()
        (self.root / "src").mkdir()
        self.base = b"int focus(void) { return 0; }\n"
        self.candidate = b"int focus(void) { return 1; }\n"
        (self.root / "src/test.c").write_bytes(self.base)
        (self.root / "candidate.c").write_bytes(self.candidate)
        (self.root / "target.o").write_bytes(b"targ")
        (self.root / "toolchain.json").write_text("{}", encoding="utf-8")
        config = {
            "units": [{
                "name": "main/test/owner",
                "target_path": "target.o",
                "base_path": "build/test.o",
            }]
        }
        (self.root / "objdiff.json").write_text(json.dumps(config), encoding="utf-8")
        (self.root / "configure.py").write_text("# fixture\n", encoding="utf-8")
        _init_fixture_git(self.root, self._git_run)
        self._git_run("config", "user.email", "test@example.invalid")
        self._git_run("config", "user.name", "test")
        self._git_run("add", ".")
        self._git_run("commit", "-qm", "fixture")
        self.commit = self._git_output("rev-parse", "HEAD").strip()
        self.inventory = {
            "name": "fixture",
            "repository": str(self.root),
            "release_commit": self.commit,
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "function": "focus",
            "source_relpath": "src/test.c",
            "base": {"kind": "file", "path": str(self.root / "src/test.c"), "sha256": digest(self.base)},
            "candidate": {"kind": "file", "path": str(self.root / "candidate.c"), "sha256": digest(self.candidate)},
            "target_path": str(self.root / "target.o"),
            "target_sha256": digest(b"targ"),
            "toolchain_path": str(self.root / "toolchain.json"),
            "objdiff_path": str(self.root / "objdiff.json"),
            "protected_exact_functions": [],
        }

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_fixture_names_and_reconstruction(self) -> None:
        self.assertEqual(
            replay.fixture_names(),
            ("SetupMgType", "mbev_CapBomheiMove", "ev_CapBobleOMExec"),
        )
        self.assertEqual(digest(replay.reconstruct_function(
            b"void f(void) { return; }\nvoid focus(void) { return; }\n",
            b"void f(void) { return; }\nvoid focus(void) { return 1; }\n",
            "focus",
        )), digest(b"void f(void) { return; }\nvoid focus(void) { return 1; }\n"))

    def test_source_generator_copies_bounded_template_directory(self) -> None:
        """The production Boble generator's directory input stays in the clone."""

        historical = Path(self.temp.name) / "historical"
        template = historical / "build/requests/template"
        nested = template / "nested"
        nested.mkdir(parents=True)
        (template / "approval-draft.json").write_bytes(b"draft")
        (nested / "support.txt").write_bytes(b"support")
        (historical / "residual.json").write_bytes(b"residual")
        generator = historical / "generate.py"
        generator.write_text(
            "\n".join(
                (
                    "from pathlib import Path",
                    f"ROOT = Path(r'{historical.as_posix()}')",
                    "TEMPLATE_DIR = ROOT / 'build/requests/template'",
                    "RESIDUAL = ROOT / 'residual.json'",
                    "output = ROOT / 'build/generated.c'",
                    "output.parent.mkdir(parents=True, exist_ok=True)",
                    "output.write_bytes((TEMPLATE_DIR / 'nested/support.txt').read_bytes() + RESIDUAL.read_bytes())",
                    "",
                )
            ),
            encoding="utf-8",
        )
        repository = Path(self.temp.name) / "replay"
        repository.mkdir()

        generated = replay._run_source_generator(
            repository, generator, Path("build/generated.c")
        )

        self.assertEqual(generated.read_bytes(), b"supportresidual")
        self.assertEqual(
            (repository / "build/requests/template/approval-draft.json").read_bytes(),
            b"draft",
        )
        self.assertEqual(
            (repository / "build/requests/template/nested/support.txt").read_bytes(),
            b"support",
        )
        self.assertEqual(
            replay._sha_file(repository / "residual.json"),
            digest(b"residual"),
        )

    def test_git_stage_commit_force_adds_only_explicit_ignored_paths(self) -> None:
        """Ignored sealed build inputs are staged, but unrelated scratch is not."""

        (self.root / ".gitignore").write_text("build/\n*.scratch\n", encoding="utf-8")
        self._git_run("add", ".gitignore")
        self._git_run("commit", "-qm", "ignore build inputs")
        release_commit = self._git_output("rev-parse", "HEAD").strip()
        selected = self.root / "build/selected.c"
        unrelated = self.root / "build/unrelated.c"
        scratch = self.root / "unrelated.scratch"
        selected.parent.mkdir(parents=True)
        selected.write_bytes(b"selected")
        unrelated.write_bytes(b"unrelated")
        scratch.write_bytes(b"scratch")

        campaign_commit = replay._git_stage_commit(
            self.root,
            self.root,
            release_commit,
            ["build/selected.c"],
            Path(self.temp.name) / "replay.index",
        )
        tracked = set(
            self._git_output("ls-tree", "-r", "--name-only", campaign_commit).splitlines()
        )
        self.assertIn("build/selected.c", tracked)
        self.assertNotIn("build/unrelated.c", tracked)
        self.assertNotIn("unrelated.scratch", tracked)

    def test_replay_schemas_are_valid_json_and_match_runtime_constants(self) -> None:
        replay_schema = json.loads(
            (Path(__file__).parents[1] / "OWNER_CAMPAIGN_REPLAY_V1.schema.json").read_text()
        )
        aggregate_schema = json.loads(
            (Path(__file__).parents[1] / "OWNER_CAMPAIGN_REPLAY_AGGREGATE_V1.schema.json").read_text()
        )
        handle_schema = json.loads(
            (Path(__file__).parents[1] / "OWNER_CAMPAIGN_REPLAY_HANDLE_V1.schema.json").read_text()
        )
        self.assertEqual(replay_schema["$id"], replay.SCHEMA)
        self.assertEqual(aggregate_schema["$id"], replay.AGGREGATE_SCHEMA)
        self.assertEqual(handle_schema["$id"], replay.HANDLE_SCHEMA)

    def test_prepare_uses_detached_tree_and_hashes_sources(self) -> None:
        output = Path(self.temp.name) / "out"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        self.assertTrue(Path(prepared["worktree"]).is_dir())
        self.assertEqual(prepared["base_source_sha256"], digest(self.base))
        self.assertEqual(prepared["candidate_source_sha256"], digest(self.candidate))
        self.assertEqual(
            json.loads(Path(prepared["manifest_path"]).read_text())["base_commit"],
            prepared["campaign_commit"],
        )
        manifest = json.loads(Path(prepared["manifest_path"]).read_text())
        self.assertEqual(manifest["limits"]["focus_evidence_bytes"], 256 << 10)
        self.assertEqual(manifest["limits"]["scratch_soft_bytes"], 384 << 20)
        self.assertEqual(manifest["limits"]["scratch_hard_bytes"], 512 << 20)
        self.assertEqual((self.root / "src/test.c").read_bytes(), self.base)
        self.assertEqual(Path(prepared["source_path"]).read_bytes(), self.base)
        replay.cleanup_replay(prepared)
        self.assertFalse(Path(prepared["raw_root"]).exists())
        listed = self._git_output("worktree", "list", "--porcelain")
        self.assertNotIn(str(Path(prepared["worktree"])), listed)

    def test_explicit_root_overrides_builtin_repository(self) -> None:
        spec = replay.fixture_spec("SetupMgType", root=self.root)
        self.assertEqual(Path(spec["repository"]), self.root.absolute())

    def test_native_git_resolver_does_not_select_devkit_msys(self) -> None:
        executable = replay._git_argv("--version")[0].casefold().replace("/", "\\")
        self.assertNotIn("\\devkitpro\\msys2\\", executable)

    def test_command_timeout_terminates_bounded_child(self) -> None:
        started = time.monotonic()
        with self.assertRaisesRegex(replay.ReplayError, "command timed out"):
            replay._run(
                [sys.executable, "-c", "import time; time.sleep(60)"],
                cwd=self.root,
                timeout=0.05,
            )
        self.assertLess(time.monotonic() - started, 10.0)


    def test_replay_commit_is_not_written_to_authoritative_object_database(self) -> None:
        before = self._git_output("count-objects", "-v")
        prepared = replay.prepare_replay(
            self.root, self.inventory, Path(self.temp.name) / "out"
        )
        self.assertNotEqual(Path(prepared["repository"]), self.root.absolute())
        self.assertEqual(
            replay._run(
                replay._git_argv("rev-parse", "HEAD"),
                cwd=Path(prepared["worktree"]),
                timeout=30,
            ).stdout.decode("utf-8", "replace").strip(),
            prepared["campaign_commit"],
        )
        after = self._git_output("count-objects", "-v")
        self.assertEqual(before, after)
        replay.cleanup_replay(prepared)

    def test_authoritative_protected_fixture_censuses_are_bound(self) -> None:
        setup = replay.fixture_spec("SetupMgType")
        bomhei = replay.fixture_spec("mbev_CapBomheiMove")
        boble = replay.fixture_spec("ev_CapBobleOMExec")
        self.assertEqual(len(setup["protected_exact_functions"]), 26)
        self.assertEqual(len(bomhei["protected_exact_functions"]), 11)
        self.assertEqual(len(boble["protected_exact_functions"]), 12)
        self.assertEqual(
            setup["target_sha256"],
            replay.MG_TARGET_SHA,
        )
        self.assertEqual(bomhei["target_sha256"], replay.CAPTRAP_TARGET_SHA)
        self.assertEqual(boble["target_sha256"], replay.CAPTRAP_TARGET_SHA)
        self.assertIn("current-residual", setup["target_path"])

    def test_run_exact_publishes_report_and_cleans_raw_tree(self) -> None:
        output = Path(self.temp.name) / "out"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        runtime = FakeRuntime()
        result = replay.run_replay(prepared, runtime=runtime)
        self.assertEqual(runtime.calls, ["load", "snapshot", "candidate"])
        self.assertTrue(result["proof"]["exact"])
        self.assertEqual(result["cleanup"]["status"], "complete")
        self.assertFalse(Path(prepared["raw_root"]).exists())
        self.assertTrue((output / "replay-result.json").is_file())
        self.assertTrue((output / "CRACK_REPORT-focus.json").is_file())
        self.assertGreaterEqual(result["storage"]["peak_bytes"], result["storage"]["final_bytes"])

    def test_candidate_descriptor_binds_base_and_candidate_function_spans(self) -> None:
        output = Path(self.temp.name) / "span-binding"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        runtime = FakeRuntime()
        try:
            replay.run_replay(prepared, runtime=runtime, clean=False)
            self.assertIsNotNone(runtime.descriptor)
            assert runtime.descriptor is not None
            self.assertEqual(
                runtime.descriptor["function_span"],
                {
                    "base_start_line": 1,
                    "base_end_line": 1,
                    "candidate_start_line": 1,
                    "candidate_end_line": 1,
                    "base_sha256": digest(self.base),
                    "candidate_sha256": digest(self.candidate),
                },
            )
        finally:
            if not prepared.get("cleaned"):
                replay.cleanup_replay(prepared)

    def test_moved_static_owner_uses_bounded_cell_envelope_accepted_by_runtime(self) -> None:
        from tools import owner_campaign as campaign_runtime

        base = (
            b"static const int owner = 7;\n"
            b"int helper(void) { return 2; }\n"
            b"int focus(void) { return 0; }\n"
            b"int tail(void) { return owner; }\n"
        )
        candidate = (
            b"int helper(void) { return 2; }\n"
            b"int focus(void) { return 0; }\n"
            b"static const int owner = 7;\n"
            b"int tail(void) { return owner; }\n"
        )
        base_path = Path(self.temp.name) / "moved-owner-base.c"
        candidate_path = Path(self.temp.name) / "moved-owner-candidate.c"
        base_path.write_bytes(base)
        candidate_path.write_bytes(candidate)
        inventory = copy.deepcopy(self.inventory)
        inventory["base"] = {
            "kind": "file",
            "path": str(base_path),
            "sha256": digest(base),
        }
        inventory["candidate"] = {
            "kind": "file",
            "path": str(candidate_path),
            "sha256": digest(candidate),
        }

        output = Path(self.temp.name) / "moved-owner-envelope"
        prepared = replay.prepare_replay(self.root, inventory, output)
        try:
            self.assertEqual(
                prepared["function_span"],
                {
                    "base_start_line": 1,
                    "base_end_line": 3,
                    "candidate_start_line": 1,
                    "candidate_end_line": 3,
                    "base_sha256": digest(b"".join(base.splitlines(keepends=True)[:3])),
                    "candidate_sha256": digest(
                        b"".join(candidate.splitlines(keepends=True)[:3])
                    ),
                },
            )
            worktree = Path(prepared["worktree"])
            candidate_relpath = Path(prepared["candidate_path"]).relative_to(
                worktree
            ).as_posix()
            campaign = {
                "campaign_id": "fixture",
                "functions": ["focus"],
                "allowed_source_paths": ["src/test.c"],
                "allowed_build_paths": ["build"],
                "forbidden_constructs": [],
                "_source": Path(prepared["source_path"]),
            }
            frontier = {
                "function": "focus", "frontier_sha256": "a" * 64,
                "source_sha256": prepared["base_source_sha256"],
            }
            descriptor = replay._candidate_descriptor(
                prepared["spec"],
                campaign,
                frontier,
                candidate_relpath,
                prepared["candidate_source_sha256"],
                prepared["function_span"],
                base_path=Path(prepared["base_path"]).relative_to(worktree).as_posix(),
                base_sha256=prepared["base_source_sha256"],
            )
            descriptor_path = worktree / "build/owner-replay/moved-owner.json"
            descriptor_path.parent.mkdir(parents=True, exist_ok=True)
            descriptor_path.write_text(
                json.dumps(descriptor, sort_keys=True), encoding="utf-8"
            )

            loaded = campaign_runtime._load_candidate(
                worktree, descriptor_path, campaign, frontier
            )
            self.assertEqual(loaded["function_span"], prepared["function_span"])
            self.assertEqual(
                loaded["_source_sha256"], prepared["candidate_source_sha256"]
            )
        finally:
            if not prepared.get("cleaned"):
                replay.cleanup_replay(prepared)

    def test_candidate_descriptor_uses_requested_function_not_first_campaign_function(self) -> None:
        output = Path(self.temp.name) / "requested-function"
        inventory = copy.deepcopy(self.inventory)
        inventory["functions"] = ["unrelated", "focus"]
        prepared = replay.prepare_replay(self.root, inventory, output)
        try:
            descriptor = replay._candidate_descriptor(
                prepared["spec"],
                {"campaign_id": "fixture", "functions": ["unrelated", "focus"]},
                {"frontier_sha256": "a" * 64},
                "build/owner-replay/input/candidate.c",
                prepared["candidate_source_sha256"],
                prepared["function_span"],
                base_path="build/owner-replay/input/base.c",
                base_sha256=prepared["base_source_sha256"],
            )
            self.assertEqual(descriptor["function"], "focus")
            self.assertEqual(descriptor["hypothesis_family"], "historical-replay-focus")
        finally:
            if not prepared.get("cleaned"):
                replay.cleanup_replay(prepared)

    def test_restart_handle_round_trip_and_resume(self) -> None:
        output = Path(self.temp.name) / "restart"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        handle_path = Path(prepared["handle_path"])
        reloaded = replay.load_replay_handle(handle_path, expected_fixture="fixture")
        self.assertEqual(reloaded["campaign_commit"], prepared["campaign_commit"])
        self.assertEqual(reloaded["function_span"], prepared["function_span"])
        result = replay.resume_replay(handle_path, runtime=FakeRuntime())
        self.assertTrue(result["proof"]["exact"])
        self.assertFalse(Path(prepared["raw_root"]).exists())
        # A restart after terminal completion is idempotent and cannot rerun
        # or consume a second candidate.
        self.assertEqual(
            replay.resume_replay(handle_path)["receipt_sha256"],
            result["receipt_sha256"],
        )

    def test_restart_handle_rejects_stale_candidate_binding(self) -> None:
        output = Path(self.temp.name) / "stale"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        with self.assertRaisesRegex(replay.ReplayError, "candidate binding is stale"):
            replay.load_replay_handle(
                prepared["handle_path"],
                expected_candidate_sha256="0" * 64,
            )
        replay.cleanup_replay(prepared)

    def test_restart_rejects_self_hashed_stale_terminal_result(self) -> None:
        output = Path(self.temp.name) / "stale-result"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        result = replay.run_replay(prepared, runtime=FakeRuntime())
        result_path = output / "replay-result.json"
        stale = json.loads(result_path.read_text(encoding="utf-8"))
        stale["function"] = "other"
        body = dict(stale)
        body.pop("receipt_sha256")
        stale["receipt_sha256"] = digest_json(body)
        result_path.write_text(json.dumps(stale), encoding="utf-8")
        with self.assertRaisesRegex(replay.ReplayError, "replay result function is stale"):
            replay.resume_replay(prepared["handle_path"])

    def test_storage_and_timing_gates_are_recorded(self) -> None:
        output = Path(self.temp.name) / "gates"
        prepared = replay.prepare_replay(
            self.root,
            self.inventory,
            output,
            storage_cap_bytes=10_000_000,
        )
        result = replay.run_replay(
            prepared,
            runtime=FakeRuntime(),
            max_seconds=30.0,
            storage_cap_bytes=10_000_000,
        )
        self.assertTrue(result["timing"]["within_cap"])
        self.assertTrue(result["storage"]["within_cap"])
        self.assertEqual(result["storage"]["cap_bytes"], 10_000_000)

    def test_prepare_runs_generator_at_pinned_release_not_current_head(self) -> None:
        retained_base = b"int focus(void) { return 2; }\n"
        retained_base_path = Path(self.temp.name) / "retained-base.c"
        retained_base_path.write_bytes(retained_base)
        generator = Path(self.temp.name) / "release-bound-generator.py"
        generator.write_text(
            "from pathlib import Path\n"
            "import subprocess\n"
            f"ROOT = Path({str(self.root)!r})\n"
            f"BASE_COMMIT = {self.commit!r}\n"
            "head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()\n"
            "assert head == BASE_COMMIT, (head, BASE_COMMIT)\n"
            f"assert (ROOT / 'src/test.c').read_bytes() == {retained_base!r}\n"
            "output = ROOT / 'build/generated-candidate.c'\n"
            f"output.write_bytes({self.candidate!r})\n",
            encoding="utf-8",
        )
        (self.root / "later.txt").write_text("later\n", encoding="utf-8")
        self._git_run("add", "later.txt")
        self._git_run("commit", "-qm", "later head")
        self.assertNotEqual(self._git_output("rev-parse", "HEAD").strip(), self.commit)
        inventory = copy.deepcopy(self.inventory)
        inventory["base"] = {
            "kind": "file",
            "path": str(retained_base_path),
            "sha256": digest(retained_base),
        }
        inventory["candidate"] = {
            "kind": "generator",
            "generator_path": str(generator),
            "generated_relpath": "build/generated-candidate.c",
            "sha256": digest(self.candidate),
        }
        prepared = replay.prepare_replay(
            self.root,
            inventory,
            Path(self.temp.name) / "release-bound-output",
        )
        try:
            self.assertEqual(Path(prepared["candidate_path"]).read_bytes(), self.candidate)
            self.assertEqual(prepared["release_commit"], self.commit)
        finally:
            replay.cleanup_replay(prepared)

    def test_hash_drift_fails_before_worktree_creation(self) -> None:
        self.inventory["candidate"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(replay.ReplayError, "candidate source hash drift"):
            replay.prepare_replay(self.root, self.inventory, Path(self.temp.name) / "out")
        listing = self._git_output("worktree", "list", "--porcelain")
        self.assertNotIn(str(Path(self.temp.name) / "out"), listing)

    def test_target_drift_removes_disposable_clone(self) -> None:
        self.inventory["target_sha256"] = "0" * 64
        output = Path(self.temp.name) / "target-drift"
        with self.assertRaisesRegex(replay.ReplayError, "target object hash drift"):
            replay.prepare_replay(self.root, self.inventory, output)
        self.assertFalse(output.exists() and any(output.iterdir()))

    def test_failed_report_is_cleaned_and_not_accepted(self) -> None:
        class BadRuntime(FakeRuntime):
            def run_candidate(self, root: Path, campaign: dict[str, object], descriptor: Path) -> dict[str, object]:
                result = super().run_candidate(root, campaign, descriptor)
                assert self.report is not None
                report = json.loads(self.report.read_text())
                report["status"] = "improved"
                self.report.write_text(json.dumps(report), encoding="utf-8")
                return result

        output = Path(self.temp.name) / "out"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        with self.assertRaises(replay.ReplayError):
            replay.run_replay(prepared, runtime=BadRuntime())
        self.assertTrue(prepared["cleaned"])
        self.assertFalse(Path(prepared["raw_root"]).exists())

    def test_cleanup_failure_is_visible_with_primary_error(self) -> None:
        output = Path(self.temp.name) / "out"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        with mock.patch.object(
            replay, "cleanup_replay", side_effect=replay.ReplayError("sentinel cleanup")
        ):
            with self.assertRaisesRegex(
                replay.ReplayError, "replay failed: .*cleanup failed: sentinel cleanup"
            ):
                replay.run_replay(prepared, runtime=object())
        self.assertFalse(prepared["cleaned"])

    def test_post_build_input_drift_is_rejected(self) -> None:
        class DriftingRuntime(FakeRuntime):
            def run_candidate(self, root: Path, campaign: dict[str, object], descriptor: Path) -> dict[str, object]:
                result = super().run_candidate(root, campaign, descriptor)
                (root / "build/owner-replay/input/base.c").write_bytes(b"drift")
                return result

        output = Path(self.temp.name) / "out"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        with self.assertRaisesRegex(replay.ReplayError, "base source hash drift"):
            replay.run_replay(prepared, runtime=DriftingRuntime())
        self.assertTrue(prepared["cleaned"])

    def test_post_build_verifier_accepts_authenticated_retained_candidate(self) -> None:
        output = Path(self.temp.name) / "retained-candidate"
        prepared = replay.prepare_replay(self.root, self.inventory, output)
        try:
            Path(prepared["source_path"]).write_bytes(self.candidate)
            Path(prepared["candidate_path"]).unlink()
            replay._verify_replay_inputs(
                prepared,
                phase="after candidate build",
                allow_candidate_cleanup=True,
            )
            with self.assertRaisesRegex(replay.ReplayError, "campaign source hash drift"):
                replay._verify_replay_inputs(prepared, phase="before replay")
        finally:
            replay.cleanup_replay(prepared)

    def _batch_inventory(self, suffix: str) -> dict[str, object]:
        value = copy.deepcopy(self.inventory)
        value["name"] = f"fixture-{suffix}"
        value["campaign_id"] = f"replay-focus-{suffix}"
        return value

    def test_three_replay_sequential_aggregate_is_compact_and_exact(self) -> None:
        fixtures = tuple(self._batch_inventory(str(i)) for i in range(3))
        result = replay.run_replay_batch(
            self.root,
            fixtures,
            Path(self.temp.name) / "batch",
            runtime_factory=lambda _fixture, _index: FakeRuntime(),
        )
        self.assertEqual(result["schema"], replay.AGGREGATE_SCHEMA)
        self.assertEqual(result["mode"], "sequential")
        self.assertEqual(result["aggregate"]["requested"], 3)
        self.assertEqual(result["aggregate"]["exact"], 3)
        self.assertTrue(result["aggregate"]["all_exact"])
        self.assertEqual(
            result["storage"]["peak_bytes"],
            result["storage"]["max_child_peak_bytes"],
        )
        self.assertEqual(len(result["results"]), 3)
        self.assertTrue((Path(self.temp.name) / "batch" / "replay-aggregate.json").is_file())
        for child in result["results"]:
            self.assertIn("timing", child)
            self.assertIn("storage", child)

    def test_concurrent_replays_use_isolated_children(self) -> None:
        fixtures = tuple(self._batch_inventory(str(i)) for i in range(3))
        result = replay.run_concurrent_replays(
            self.root,
            fixtures,
            Path(self.temp.name) / "concurrent",
            runtime_factory=lambda _fixture, _index: FakeRuntime(),
        )
        self.assertEqual(result["mode"], "concurrent")
        self.assertEqual(result["aggregate"]["exact"], 3)
        self.assertEqual(
            result["storage"]["peak_bytes"],
            sum(item["storage"]["peak_bytes"] for item in result["results"]),
        )
        self.assertEqual(len({item["receipt_sha256"] for item in result["results"]}), 3)
        children = sorted((Path(self.temp.name) / "concurrent").glob("0*"))
        self.assertEqual(len(children), 3)
        self.assertEqual([child.name for child in children], ["00", "01", "02"])
        self.assertTrue(all(not (child / ".r").exists() for child in children))

    def test_concurrent_storage_cap_is_enforced_per_lane(self) -> None:
        output = Path(self.temp.name) / "aggregate-storage"
        output.mkdir()
        result = replay._aggregate_receipt(
            output=output,
            mode="concurrent",
            fixtures=("a", "b"),
            results=(
                {"fixture": "a", "proof": {"exact": True}, "storage": {"peak_bytes": 60}},
                {"fixture": "b", "proof": {"exact": True}, "storage": {"peak_bytes": 60}},
            ),
            started=time.monotonic(),
            storage_cap_bytes=100,
        )
        self.assertEqual(result["storage"]["peak_bytes"], 120)
        self.assertEqual(result["storage"]["max_child_peak_bytes"], 60)
        self.assertTrue(result["storage"]["within_cap"])

    def test_batch_failure_publishes_terminal_failure_receipt(self) -> None:
        class NoGainRuntime(FakeRuntime):
            def run_candidate(self, root: Path, campaign: dict[str, object], descriptor: Path) -> dict[str, object]:
                self.calls.append("candidate")
                return {"status": "no_gain"}

        output = Path(self.temp.name) / "failed-batch"
        with self.assertRaisesRegex(replay.ReplayError, "did not reach exact"):
            replay.run_replay_batch(
                self.root,
                (self._batch_inventory("failed"),),
                output,
                runtime_factory=lambda _fixture, _index: NoGainRuntime(),
            )
        aggregate_path = output / "replay-aggregate.json"
        aggregate = json.loads(aggregate_path.read_text(encoding="utf-8"))
        self.assertEqual(aggregate["status"], "failed")
        self.assertFalse(aggregate["aggregate"]["all_exact"])
        self.assertEqual(len(aggregate["errors"]), 1)
        body = dict(aggregate)
        digest_value = body.pop("aggregate_sha256")
        self.assertEqual(digest_json(body), digest_value)

    def test_batch_rejects_duplicate_fixture_names_before_preparation(self) -> None:
        first = self._batch_inventory("same")
        second = copy.deepcopy(first)
        with self.assertRaisesRegex(replay.ReplayError, "fixture names must be unique"):
            replay.run_replay_batch(
                self.root,
                (first, second),
                Path(self.temp.name) / "duplicate",
                runtime_factory=lambda _fixture, _index: FakeRuntime(),
            )

    def test_unresolved_function_is_always_present(self) -> None:
        prepared = replay.prepare_replay(self.root, self.inventory, Path(self.temp.name) / "out")
        manifest = json.loads(Path(prepared["manifest_path"]).read_text())
        self.assertIn("__unresolved_owner__", manifest["functions"])
        self.assertNotEqual(len(manifest["functions"]), len(manifest["protected_exact_functions"]))
        replay.cleanup_replay(prepared)


if __name__ == "__main__":
    unittest.main()
