from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

from tools import owner_campaign_release as release


def _git(root: Path, *args: str) -> str:
    completed = subprocess.run(
        ["git", *args],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode:
        raise AssertionError(completed.stderr or completed.stdout)
    return completed.stdout.strip()


def _init_git(root: Path, message: str) -> str:
    _git(root, "init", "-q")
    _git(root, "config", "user.email", "release-launcher-test@example.invalid")
    _git(root, "config", "user.name", "Owner Campaign Release Test")
    _git(root, "add", ".")
    _git(root, "commit", "-qm", message)
    return _git(root, "rev-parse", "HEAD")


class OwnerCampaignReleaseTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        self.release_root = self.root / "workflow-release"
        self.install_root = self.root / "stable-runtime"
        self.lane_root = self.root / "owner-lane"
        (self.release_root / "tools").mkdir(parents=True)
        (self.lane_root / "build" / "owner-campaign").mkdir(parents=True)
        (self.lane_root / "tools").mkdir()

        agent = """\
import json
import subprocess
import sys
import time
from pathlib import Path

argv = sys.argv[1:]
if len(argv) < 2 or argv[0] != '--root':
    raise SystemExit(19)
root = Path(argv[1])
out = root / 'build' / 'owner-campaign' / 'released-agent.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(argv), encoding='utf-8')
if '--sleep' in argv:
    time.sleep(0.75)
if '--spawn-child' in argv:
    marker = root / 'build' / 'owner-campaign' / 'descendant-marker'
    subprocess.Popen(
        [sys.executable, str(Path(__file__).with_name('descendant.py')), str(marker)],
        close_fds=True,
    )
    time.sleep(1.2)
if '--spawn-child-ready' in argv:
    marker = root / 'build' / 'owner-campaign' / 'descendant-marker'
    ready = root / 'build' / 'owner-campaign' / 'descendant-ready'
    subprocess.Popen(
        [
            sys.executable,
            str(Path(__file__).with_name('descendant_ready.py')),
            str(marker),
            str(ready),
        ],
        close_fds=True,
    )
    deadline = time.monotonic() + 2.0
    while not ready.exists() and time.monotonic() < deadline:
        time.sleep(0.01)
    time.sleep(1.2)
if '--fail' in argv:
    raise SystemExit(7)
"""
        (self.release_root / "tools" / "agent.py").write_text(agent, encoding="utf-8")
        (self.release_root / "tools" / "descendant.py").write_text(
            "import sys, time\n"
            "from pathlib import Path\n"
            "time.sleep(0.75)\n"
            "Path(sys.argv[1]).write_text('survived\\n', encoding='utf-8')\n",
            encoding="utf-8",
        )
        (self.release_root / "tools" / "descendant_ready.py").write_text(
            "import sys, time\n"
            "from pathlib import Path\n"
            "Path(sys.argv[2]).write_text('ready\\n', encoding='utf-8')\n"
            "time.sleep(2.0)\n"
            "Path(sys.argv[1]).write_text('survived\\n', encoding='utf-8')\n",
            encoding="utf-8",
        )
        (self.release_root / "tools" / "owner_campaign_measure.py").write_text(
            "# release measurement producer\n", encoding="utf-8"
        )
        source_launcher = Path(__file__).parents[1] / "owner_campaign_release.py"
        shutil.copyfile(source_launcher, self.release_root / "tools" / "owner_campaign_release.py")
        self.release_commit = _init_git(self.release_root, "release fixture")

        (self.lane_root / "README").write_text("lane\n", encoding="utf-8")
        self.lane_commit = _init_git(self.lane_root, "lane fixture")
        (self.lane_root / "tools" / "agent.py").write_text(
            "raise SystemExit('stale local agent must never run')\n", encoding="utf-8"
        )
        toolchain = self.lane_root / "build" / "owner-campaign" / "tool-cas" / ("0" * 64) / "toolchain.json"
        toolchain.parent.mkdir(parents=True)
        toolchain.write_text("{\"toolchain\": true}\n", encoding="utf-8")
        measure = self.lane_root / "build" / "owner-campaign" / "tool-cas" / ("1" * 64) / "measure.py"
        measure.parent.mkdir(parents=True)
        measure.write_text("# lane producer\n", encoding="utf-8")
        toolchain_hash = release._sha_file(toolchain)
        measure_hash = release._sha_file(measure)
        # The CAS directory must be content-addressed by the actual bytes.
        correct_toolchain = toolchain.parent.parent / toolchain_hash / "toolchain.json"
        correct_measure = measure.parent.parent / measure_hash / "measure.py"
        correct_toolchain.parent.mkdir(parents=True)
        correct_toolchain.write_bytes(toolchain.read_bytes())
        correct_measure.parent.mkdir(parents=True)
        correct_measure.write_bytes(measure.read_bytes())
        toolchain.unlink()
        measure.unlink()
        toolchain.parent.rmdir()
        measure.parent.rmdir()
        (self.lane_root / "build" / "owner-campaign" / "campaign.json").write_text(
            json.dumps(
                {
                    "toolchain": {
                        "path": correct_toolchain.relative_to(self.lane_root).as_posix(),
                        "sha256": toolchain_hash,
                    },
                    "measurement_producer": {
                        "path": correct_measure.relative_to(self.lane_root).as_posix(),
                        "sha256": measure_hash,
                    },
                }
            ),
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _install(self) -> dict[str, object]:
        return release.install_release(self.release_root, self.install_root)

    def _args(self, *extra: str) -> list[str]:
        return [
            "owner-campaign",
            "status",
            "--campaign",
            "build/owner-campaign/campaign.json",
            *extra,
        ]

    def _make_second_release(self) -> tuple[Path, str]:
        """Create a distinct clean release checkout for upgrade coverage."""

        release_root = self.root / "workflow-release-b"
        shutil.copytree(
            self.release_root,
            release_root,
            ignore=shutil.ignore_patterns(".git"),
        )
        (release_root / "tools" / "owner_campaign_measure.py").write_text(
            "# release measurement producer B\n",
            encoding="utf-8",
        )
        launcher = release_root / "tools" / release.LAUNCHER_FILENAME
        launcher.write_bytes(launcher.read_bytes() + b"\n# release B launcher generation\n")
        return release_root, _init_git(release_root, "release B fixture")

    def _make_third_release(self) -> tuple[Path, str]:
        """Create a third clean release for one-generation fallback coverage."""

        release_root = self.root / "workflow-release-c"
        shutil.copytree(
            self.release_root,
            release_root,
            ignore=shutil.ignore_patterns(".git"),
        )
        (release_root / "tools" / "owner_campaign_measure.py").write_text(
            "# release measurement producer C\n",
            encoding="utf-8",
        )
        launcher = release_root / "tools" / release.LAUNCHER_FILENAME
        launcher.write_bytes(launcher.read_bytes() + b"\n# release C launcher generation\n")
        return release_root, _init_git(release_root, "release C fixture")

    def _lock_path(self) -> Path:
        return self.lane_root / release.ADOPTION_RELATIVE_PATH.parent / release.ADOPTION_LOCK_FILENAME

    def test_install_publishes_hash_bound_pointer_and_status(self) -> None:
        installed = self._install()
        self.assertEqual(installed["status"], "installed")
        pointer_path = self.install_root / release.POINTER_FILENAME
        pointer = json.loads(pointer_path.read_text(encoding="utf-8"))
        self.assertEqual(pointer["schema"], release.POINTER_SCHEMA)
        self.assertEqual(pointer["commit"], self.release_commit)
        self.assertEqual(pointer["agent_sha256"], release._sha_file(self.release_root / "tools" / "agent.py"))
        self.assertEqual(
            pointer["measurement_sha256"],
            release._sha_file(self.release_root / "tools" / "owner_campaign_measure.py"),
        )
        self.assertEqual(
            release._digest_json(pointer, "pointer_sha256"), pointer["pointer_sha256"]
        )
        status = release.release_status(self.install_root)
        self.assertEqual(status["status"], "ready")
        self.assertEqual(status["workflow_commit"], self.release_commit)

    def test_dirty_release_and_tool_hash_drift_fail_closed(self) -> None:
        self._install()
        (self.release_root / "tools" / "owner_campaign_measure.py").write_text(
            "# drift\n", encoding="utf-8"
        )
        status = release.release_status(self.install_root)
        self.assertEqual(status["status"], "drift")
        self.assertIn("clean", status["error"])

    def test_install_rejects_dirty_release_before_publication(self) -> None:
        (self.release_root / "untracked.txt").write_text("dirty\n", encoding="utf-8")
        with self.assertRaisesRegex(release.ReleaseError, "not clean"):
            self._install()
        self.assertFalse(self.install_root.exists())

    def test_stable_launcher_uses_release_agent_and_writes_terminal_receipt(self) -> None:
        self._install()
        result = release.run_agent(self.install_root, self.lane_root, self._args())
        self.assertEqual(result["terminal_status"], "completed")
        marker = self.lane_root / "build" / "owner-campaign" / "released-agent.json"
        argv = json.loads(marker.read_text(encoding="utf-8"))
        self.assertEqual(argv[0], "--root")
        self.assertEqual(Path(argv[1]), self.lane_root)
        receipt_path = self.lane_root / release.ADOPTION_RELATIVE_PATH
        receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
        self.assertEqual(receipt["status"], "terminal")
        self.assertEqual(receipt["terminal_status"], "completed")
        self.assertEqual(receipt["workflow_commit"], self.release_commit)
        self.assertEqual(receipt["child_argv"][0], sys.executable)
        self.assertEqual(receipt["child_argv"][2], "--root")
        self.assertGreater(receipt["child_pid"], 0)
        self.assertEqual(receipt["lane"]["lane_head"], self.lane_commit)
        self.assertEqual(receipt["lane"]["manifest"]["path"], "build/owner-campaign/campaign.json")
        self.assertEqual(len(receipt["lane"]["tool_cas"]), 2)
        self.assertEqual(
            release._digest_json(receipt, "receipt_sha256"), receipt["receipt_sha256"]
        )
        status = release.release_status(self.install_root, self.lane_root)
        self.assertEqual(status["status"], "ready")
        self.assertEqual(status["adoption"]["terminal_status"], "completed")

    def test_nonzero_agent_exit_is_terminal_failed(self) -> None:
        self._install()
        result = release.run_agent(self.install_root, self.lane_root, self._args("--fail"))
        self.assertEqual(result["terminal_status"], "failed")
        self.assertEqual(result["exit_code"], 7)
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["status"], "terminal")
        self.assertEqual(receipt["exit_code"], 7)

    def test_live_adoption_lock_rejects_concurrent_launch(self) -> None:
        self._install()
        first_result: list[dict[str, object]] = []
        first_error: list[BaseException] = []

        def launch_first() -> None:
            try:
                first_result.append(
                    release.run_agent(
                        self.install_root,
                        self.lane_root,
                        self._args("--sleep"),
                    )
                )
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                first_error.append(exc)

        thread = threading.Thread(target=launch_first)
        thread.start()
        lock_path = (
            self.lane_root
            / release.ADOPTION_RELATIVE_PATH.parent
            / release.ADOPTION_LOCK_FILENAME
        )
        deadline = time.monotonic() + 2.0
        while not lock_path.exists() and thread.is_alive() and time.monotonic() < deadline:
            time.sleep(0.01)

        try:
            self.assertTrue(lock_path.exists(), "first adoption never published its lock")
            with self.assertRaisesRegex(release.ReleaseError, "lock|active"):
                release.run_agent(
                    self.install_root,
                    self.lane_root,
                    self._args("--sleep"),
                )
        finally:
            thread.join(3.0)

        self.assertFalse(thread.is_alive(), "first adoption thread leaked")
        self.assertEqual(first_error, [])
        self.assertEqual(len(first_result), 1)
        self.assertEqual(first_result[0]["terminal_status"], "completed")
        self.assertFalse(lock_path.exists(), "adoption lock leaked after terminal state")

    def test_pid_liveness_probe_does_not_signal_the_current_process(self) -> None:
        # In particular, Windows must not implement this with os.kill(pid, 0):
        # that is a console/process signal there, not a harmless POSIX probe.
        self.assertTrue(release._pid_alive(os.getpid()))

    def test_stale_dead_adoption_lock_is_recovered(self) -> None:
        self._install()
        lock_path = self._lock_path()
        lock_path.write_text(str(max(os.getpid() + 100000, 2000000)), encoding="utf-8")

        result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertFalse(lock_path.exists(), "stale adoption lock was not removed")

    def test_malformed_adoption_lock_payloads_fail_closed(self) -> None:
        """Malformed JSON must never become a PID or raise a raw TypeError."""

        self._install()
        lock_path = self._lock_path()
        malformed_payloads = ([1, 2], [["nested"]], True)

        for payload in malformed_payloads:
            with self.subTest(payload=payload):
                lock_path.write_text(json.dumps(payload), encoding="utf-8")
                with self.assertRaisesRegex(
                    release.ReleaseError,
                    "JSON object|schema|fields|PID|payload|unsupported|invalid",
                ):
                    with release._adoption_lock(self.lane_root):
                        self.fail("malformed adoption lock was acquired")
                self.assertTrue(
                    lock_path.exists(),
                    "malformed adoption lock was discarded instead of retained",
                )

    def test_adoption_lock_is_invisible_until_complete_payload_is_published(self) -> None:
        self._install()
        lock_path = self._lock_path()
        temp_created = threading.Event()
        allow_publish = threading.Event()
        acquired = threading.Event()
        release_holder = threading.Event()
        holder_errors: list[BaseException] = []
        real_mkstemp = release.tempfile.mkstemp

        def delayed_mkstemp(*args: object, **kwargs: object) -> tuple[int, str]:
            result = real_mkstemp(*args, **kwargs)
            directory = kwargs.get("dir")
            if directory is not None and Path(directory).resolve() == lock_path.parent.resolve():
                temp_created.set()
                if not allow_publish.wait(3.0):
                    raise AssertionError("lock publication was not released")
            return result

        def hold_lock() -> None:
            try:
                with release._adoption_lock(self.lane_root):
                    acquired.set()
                    self.assertTrue(release_holder.wait(3.0))
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                holder_errors.append(exc)

        with mock.patch.object(release.tempfile, "mkstemp", side_effect=delayed_mkstemp):
            thread = threading.Thread(target=hold_lock)
            thread.start()
            self.assertTrue(temp_created.wait(2.0), "lock temp file was never created")
            self.assertFalse(
                lock_path.exists(),
                "the lock path became visible before its payload was durable",
            )
            allow_publish.set()
            self.assertTrue(acquired.wait(2.0), "lock was not acquired")
            payload = json.loads(lock_path.read_text(encoding="utf-8"))
            self.assertEqual(
                set(payload),
                {"schema", "pid", "created_at", "recoverable", "retained_reason", "child_pid"},
            )
            self.assertEqual(payload["schema"], release.ADOPTION_LOCK_SCHEMA)
            self.assertEqual(payload["pid"], os.getpid())
            self.assertTrue(payload["recoverable"])
            self.assertIsNone(payload["child_pid"])
            release_holder.set()
            thread.join(3.0)

        self.assertFalse(thread.is_alive(), "lock holder thread leaked")
        self.assertEqual(holder_errors, [])
        self.assertFalse(lock_path.exists(), "lock cleanup leaked after a normal release")

    def test_unproved_stale_lock_remains_quarantined_until_safe(self) -> None:
        self._install()
        lock_path = self._lock_path()
        lock_path.write_text(
            json.dumps(
                {
                    "schema": release.ADOPTION_LOCK_SCHEMA,
                    "pid": max(os.getpid() + 100000, 2000000),
                    "created_at": release._iso_now(),
                    "recoverable": False,
                    "retained_reason": "descendant termination was not proved",
                    "child_pid": max(os.getpid() + 100001, 2000001),
                }
            ),
            encoding="utf-8",
        )

        with self.assertRaisesRegex(release.ReleaseError, "retained fail-closed|not proved"):
            with release._adoption_lock(self.lane_root):
                self.fail("an unproved stale lock was reclaimed")
        self.assertTrue(lock_path.exists(), "an unproved stale lock was discarded")

    def test_taskkill_nonzero_root_exit_race_never_claims_tree_cleanup(self) -> None:
        class RootExitRaceProcess:
            pid = 42424242

            def __init__(self) -> None:
                self.poll_calls = 0
                self.wait_calls = 0

            def poll(self) -> int | None:
                self.poll_calls += 1
                return None if self.poll_calls == 1 else 0

            def wait(self, timeout: float | None = None) -> int:
                self.wait_calls += 1
                return 0

        process = RootExitRaceProcess()
        taskkill_calls: list[list[str]] = []
        real_run = subprocess.run

        def taskkill_failure(command: list[str], *args: object, **kwargs: object) -> object:
            if command and command[0].lower() == "taskkill":
                taskkill_calls.append(command)
                return subprocess.CompletedProcess(command, 1, stdout="", stderr="root exited")
            return real_run(command, *args, **kwargs)

        with mock.patch.object(release.subprocess, "run", side_effect=taskkill_failure):
            with self.assertRaisesRegex(release.ReleaseError, "descendant|taskkill"):
                release._terminate_process_tree(process)  # type: ignore[arg-type]

        self.assertEqual(len(taskkill_calls), 1)
        self.assertEqual(process.wait_calls, 0)

    def test_failed_tree_cleanup_quarantines_lane_and_blocks_next_launch(self) -> None:
        self._install()

        class UnprovedProcess:
            pid = 42424243

            def __init__(self) -> None:
                self.wait_calls = 0

            def poll(self) -> None:
                return None

            def wait(self, timeout: float | None = None) -> int:
                self.wait_calls += 1
                raise subprocess.TimeoutExpired(["released-agent"], timeout)

        process = UnprovedProcess()
        starts = 0

        def start(*args: object, **kwargs: object) -> UnprovedProcess:
            nonlocal starts
            starts += 1
            return process

        with mock.patch.object(release, "_start_released_agent", side_effect=start):
            with mock.patch.object(
                release,
                "_terminate_process_tree",
                side_effect=release.ReleaseError("taskkill nonzero; descendant status unknown"),
            ):
                first_result: dict[str, object] | None = None
                first_error: BaseException | None = None
                try:
                    first_result = release.run_agent(
                        self.install_root,
                        self.lane_root,
                        self._args(),
                        timeout=0.01,
                    )
                except BaseException as exc:  # a terminal error is still acceptable
                    first_error = exc

                if first_result is not None:
                    self.assertNotEqual(first_result.get("terminal_status"), "completed")
                if first_error is not None:
                    self.assertRegex(str(first_error), "taskkill|descendant|cleanup|quarant")

                with self.assertRaisesRegex(
                    release.ReleaseError,
                    "held|retained|cleanup|quarant|safe",
                ):
                    release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(starts, 1, "a quarantined lane was spawned again")

    def test_run_rejects_install_owner_overlap_before_adoption(self) -> None:
        self._install()
        with self.assertRaisesRegex(release.ReleaseError, "overlap"):
            release.run_agent(self.install_root, self.install_root, self._args())
        self.assertFalse(
            (self.install_root / release.ADOPTION_RELATIVE_PATH).exists(),
            "overlap rejection wrote an adoption receipt",
        )

    def test_timeout_terminates_descendant_process(self) -> None:
        self._install()
        marker = self.lane_root / "build" / "owner-campaign" / "descendant-marker"

        result = release.run_agent(
            self.install_root,
            self.lane_root,
            self._args("--spawn-child"),
            timeout=0.1,
        )

        self.assertEqual(result["terminal_status"], "timed_out")
        # The child waits 0.75s before writing.  Waiting past that deadline
        # distinguishes process-tree termination from killing only the parent.
        time.sleep(1.0)
        self.assertFalse(marker.exists(), "timeout left a released-agent descendant alive")

    def _assert_wait_failure_terminates_descendant(
        self, failure: BaseException
    ) -> None:
        """Drive a live root/descendant through a non-timeout wait failure."""

        self._install()
        marker = self.lane_root / "build" / "owner-campaign" / "descendant-marker"
        ready = self.lane_root / "build" / "owner-campaign" / "descendant-ready"
        real_start = release._start_released_agent

        class WaitFailureProcess:
            def __init__(self, inner: subprocess.Popen[bytes]) -> None:
                self.inner = inner
                self.pid = inner.pid
                self.failed = False

            def poll(self) -> int | None:
                return self.inner.poll()

            def wait(self, timeout: float | None = None) -> int:
                if not self.failed:
                    self.failed = True
                    raise failure
                return self.inner.wait(timeout=timeout)

            def kill(self) -> None:
                self.inner.kill()

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> WaitFailureProcess:
            inner = real_start(command, release_root, environment)
            deadline = time.monotonic() + 2.0
            while not ready.exists() and inner.poll() is None and time.monotonic() < deadline:
                time.sleep(0.01)
            if not ready.exists():
                inner.kill()
                inner.wait(timeout=5.0)
                raise AssertionError("released-agent descendant did not become ready")
            return WaitFailureProcess(inner)

        with mock.patch.object(release, "_start_released_agent", side_effect=start):
            outcome: dict[str, object] | None = None
            caught: BaseException | None = None
            try:
                outcome = release.run_agent(
                    self.install_root,
                    self.lane_root,
                    self._args("--spawn-child-ready"),
                )
            except BaseException as exc:
                caught = exc

        if outcome is not None:
            self.assertNotEqual(outcome.get("terminal_status"), "completed")
        if caught is not None:
            self.assertIsInstance(caught, (type(failure), release.ReleaseError))
        time.sleep(2.2)
        self.assertFalse(marker.exists(), "wait failure left a released-agent descendant alive")
        if (self.lane_root / release.ADOPTION_RELATIVE_PATH).exists():
            receipt = json.loads(
                (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
            )
            self.assertNotEqual(receipt.get("terminal_status"), "completed")

    def test_wait_oserror_terminates_root_and_descendant(self) -> None:
        self._assert_wait_failure_terminates_descendant(OSError("wait sentinel"))

    def test_wait_keyboard_interrupt_terminates_root_and_descendant(self) -> None:
        self._assert_wait_failure_terminates_descendant(KeyboardInterrupt())

    def test_pid_receipt_publication_failure_terminates_started_tree(self) -> None:
        self._install()
        marker = self.lane_root / "build" / "owner-campaign" / "descendant-marker"
        ready = self.lane_root / "build" / "owner-campaign" / "descendant-ready"
        real_start = release._start_released_agent
        real_write = release._write_atomic
        started = threading.Event()

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> subprocess.Popen[bytes]:
            process = real_start(command, release_root, environment)
            deadline = time.monotonic() + 2.0
            while not ready.exists() and process.poll() is None and time.monotonic() < deadline:
                time.sleep(0.01)
            if not ready.exists():
                process.kill()
                process.wait(timeout=5.0)
                raise AssertionError("released-agent descendant did not become ready")
            started.set()
            return process

        def fail_pid_receipt(
            path: Path, value: object, label: str
        ) -> str:
            if (
                label == "active adoption receipt"
                and isinstance(value, dict)
                and value.get("child_pid") is not None
            ):
                raise release.ReleaseError("PID-bearing receipt publication sentinel")
            return real_write(path, value, label)

        with mock.patch.object(release, "_start_released_agent", side_effect=start):
            with mock.patch.object(release, "_write_atomic", side_effect=fail_pid_receipt):
                outcome: dict[str, object] | None = None
                caught: BaseException | None = None
                try:
                    outcome = release.run_agent(
                        self.install_root,
                        self.lane_root,
                        self._args("--spawn-child-ready"),
                    )
                except BaseException as exc:
                    caught = exc

        self.assertTrue(started.is_set(), "the PID-bearing publication path was not reached")
        if outcome is not None:
            self.assertNotEqual(outcome.get("terminal_status"), "completed")
        if caught is not None:
            self.assertRegex(str(caught), "PID-bearing|receipt|cleanup|failed")
        time.sleep(2.2)
        self.assertFalse(marker.exists(), "receipt failure left a released-agent descendant alive")
        receipt_path = self.lane_root / release.ADOPTION_RELATIVE_PATH
        if receipt_path.exists():
            receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
            self.assertNotEqual(receipt.get("status"), "active")

    def test_popen_failure_has_nonzero_terminal_and_cli_status(self) -> None:
        self._install()
        with mock.patch.object(
            release,
            "_start_released_agent",
            side_effect=OSError("spawn sentinel"),
        ):
            result = release.run_agent(self.install_root, self.lane_root, self._args())
            self.assertEqual(result["terminal_status"], "failed")
            self.assertIsInstance(result["exit_code"], int)
            self.assertNotEqual(result["exit_code"], 0)

            cli_status = release.main(
                [
                    "run",
                    "--install-root",
                    str(self.install_root),
                    "--root",
                    str(self.lane_root),
                    "--",
                    *self._args(),
                ]
            )
            self.assertNotEqual(cli_status, 0)

    def test_install_pointer_failure_rolls_back_existing_launcher_and_pointer(self) -> None:
        self._install()
        pointer_path = self.install_root / release.POINTER_FILENAME
        launcher_path = self.install_root / release.LAUNCHER_FILENAME
        old_pointer = pointer_path.read_bytes()
        old_launcher = launcher_path.read_bytes()
        release_b, release_b_commit = self._make_second_release()

        def fail_pointer(path: Path, value: object, label: str) -> str:
            if label == "release pointer":
                raise release.ReleaseError("pointer failure sentinel")
            return release._write_atomic(path, value, label)

        with mock.patch.object(release, "_write_atomic", side_effect=fail_pointer):
            with self.assertRaisesRegex(release.ReleaseError, "pointer failure"):
                release.install_release(release_b, self.install_root)

        self.assertEqual(pointer_path.read_bytes(), old_pointer)
        self.assertEqual(launcher_path.read_bytes(), old_launcher)
        _, pointer, _ = release._verify_pointer(self.install_root)
        self.assertEqual(pointer["commit"], self.release_commit)
        self.assertNotEqual(pointer["commit"], release_b_commit)

    def test_fresh_install_pointer_failure_leaves_no_inconsistent_artifacts(self) -> None:
        fresh_root = self.root / "fresh-runtime"
        with mock.patch.object(
            release,
            "_write_atomic",
            side_effect=release.ReleaseError("pointer failure sentinel"),
        ):
            with self.assertRaisesRegex(release.ReleaseError, "pointer failure"):
                release.install_release(self.release_root, fresh_root)

        if fresh_root.exists():
            self.assertEqual(
                [path for path in fresh_root.rglob("*") if path.is_file()],
                [],
            )
        self.assertFalse((fresh_root / release.POINTER_FILENAME).exists())
        self.assertFalse((fresh_root / release.LAUNCHER_FILENAME).exists())

    def test_interrupted_install_rolls_back_existing_launcher_and_pointer(self) -> None:
        self._install()
        pointer_path = self.install_root / release.POINTER_FILENAME
        launcher_path = self.install_root / release.LAUNCHER_FILENAME
        old_pointer = pointer_path.read_bytes()
        old_launcher = launcher_path.read_bytes()
        release_b, _ = self._make_second_release()

        def interrupt_pointer(path: Path, value: object, label: str) -> str:
            if label == "release pointer":
                raise KeyboardInterrupt
            return release._write_atomic(path, value, label)

        with mock.patch.object(release, "_write_atomic", side_effect=interrupt_pointer):
            with self.assertRaises(KeyboardInterrupt):
                release.install_release(release_b, self.install_root)

        self.assertEqual(pointer_path.read_bytes(), old_pointer)
        self.assertEqual(launcher_path.read_bytes(), old_launcher)

    def test_terminal_lane_can_adopt_distinct_new_release(self) -> None:
        self._install()
        first = release.run_agent(self.install_root, self.lane_root, self._args())
        self.assertEqual(first["terminal_status"], "completed")
        release_b, release_b_commit = self._make_second_release()

        installed_b = release.install_release(release_b, self.install_root)
        self.assertEqual(installed_b["commit"], release_b_commit)
        second = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(second["terminal_status"], "completed")
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["workflow_commit"], release_b_commit)
        self.assertEqual(receipt["release_root"], str(release_b.resolve()))

    def test_drifted_current_release_uses_one_generation_fallback_before_spawn(self) -> None:
        self._install()
        release_b, release_b_commit = self._make_second_release()
        release.install_release(release_b, self.install_root)
        pointer = json.loads(
            (self.install_root / release.POINTER_FILENAME).read_text(encoding="utf-8")
        )
        self.assertEqual(pointer["fallback"]["commit"], self.release_commit)
        self.assertNotIn("fallback", pointer["fallback"])
        (release_b / "tools" / "owner_campaign_measure.py").write_text(
            "# drift after publication\n", encoding="utf-8"
        )

        started_roots: list[Path] = []
        real_start = release._start_released_agent

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> subprocess.Popen[bytes]:
            started_roots.append(release_root.resolve())
            return real_start(command, release_root, environment)

        with mock.patch.object(release, "_start_released_agent", side_effect=start):
            result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(result["workflow_commit"], self.release_commit)
        self.assertEqual(started_roots, [self.release_root.resolve()])
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["workflow_selection"], "fallback")
        self.assertEqual(receipt["pointer_commit"], release_b_commit)
        self.assertEqual(receipt["workflow_commit"], self.release_commit)
        self.assertEqual(receipt["release_root"], str(self.release_root.resolve()))
        status = release.release_status(self.install_root, self.lane_root)
        self.assertEqual(status["status"], "ready")
        self.assertEqual(status["workflow_commit"], self.release_commit)
        self.assertEqual(status["adoption"]["workflow_selection"], "fallback")
        self.assertEqual(status["adoption"]["pointer_commit"], release_b_commit)

    def test_removed_current_release_uses_verified_previous_generation(self) -> None:
        self._install()
        release_b, release_b_commit = self._make_second_release()
        release.install_release(release_b, self.install_root)
        # The fixture's Git object files can inherit read-only attributes on
        # Windows; renaming the checkout is enough to make the published path
        # disappear while avoiding a platform-specific recursive delete.
        release_b.rename(self.root / "workflow-release-b-removed")

        result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(result["workflow_commit"], self.release_commit)
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["workflow_selection"], "fallback")
        self.assertEqual(receipt["pointer_commit"], release_b_commit)
        self.assertEqual(receipt["workflow_commit"], self.release_commit)
        self.assertEqual(
            release.release_status(self.install_root, self.lane_root)["status"],
            "ready",
        )

    def test_release_selection_and_validation_finish_before_child_spawn(self) -> None:
        self._install()
        events: list[str] = []
        real_select = release._select_pointer_release
        real_verify = release.verify_release
        real_start = release._start_released_agent

        def select(*args: object, **kwargs: object) -> object:
            events.append("select")
            return real_select(*args, **kwargs)

        def verify(*args: object, **kwargs: object) -> dict[str, str]:
            if "spawn" in events:
                self.fail("release validation attempted after the child was spawned")
            events.append("verify")
            return real_verify(*args, **kwargs)

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> subprocess.Popen[bytes]:
            process = real_start(command, release_root, environment)
            events.append("spawn")
            return process

        with mock.patch.object(release, "_select_pointer_release", side_effect=select):
            with mock.patch.object(release, "verify_release", side_effect=verify):
                with mock.patch.object(release, "_start_released_agent", side_effect=start):
                    result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(events[0], "select")
        self.assertLess(events.index("verify"), events.index("spawn"))
        self.assertEqual(events.count("select"), 1)

    def test_fallback_is_not_revalidated_after_child_start(self) -> None:
        """A fallback decision is sealed before Popen and never retried later."""

        self._install()
        release_b, release_b_commit = self._make_second_release()
        release.install_release(release_b, self.install_root)
        (release_b / "tools" / "owner_campaign_measure.py").write_text(
            "# drift after publication\n", encoding="utf-8"
        )
        started = False
        real_verify = release._verify_release_descriptor
        real_start = release._start_released_agent

        def verify(*args: object, **kwargs: object) -> dict[str, str]:
            if started:
                self.fail("fallback validation was retried after child start")
            return real_verify(*args, **kwargs)

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> subprocess.Popen[bytes]:
            nonlocal started
            process = real_start(command, release_root, environment)
            started = True
            return process

        with mock.patch.object(release, "_verify_release_descriptor", side_effect=verify):
            with mock.patch.object(release, "_start_released_agent", side_effect=start):
                result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(result["workflow_selection"], "fallback")
        self.assertEqual(result["pointer_commit"], release_b_commit)

    def test_status_waits_for_install_transaction_before_reading_pointer(self) -> None:
        self._install()
        release_b, release_b_commit = self._make_second_release()
        launcher_written = threading.Event()
        allow_install = threading.Event()
        install_errors: list[BaseException] = []
        status_result: list[dict[str, object]] = []
        status_errors: list[BaseException] = []
        status_done = threading.Event()
        real_write_bytes = release._write_atomic_bytes

        def delayed_launcher_write(path: Path, payload: bytes, label: str) -> str:
            result = real_write_bytes(path, payload, label)
            if label == "stable launcher" and path.resolve() == (
                self.install_root / release.LAUNCHER_FILENAME
            ).resolve():
                launcher_written.set()
                if not allow_install.wait(3.0):
                    raise AssertionError("install transaction was not released")
            return result

        def install() -> None:
            try:
                release.install_release(release_b, self.install_root)
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                install_errors.append(exc)

        def read_status() -> None:
            try:
                status_result.append(release.release_status(self.install_root))
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                status_errors.append(exc)
            finally:
                status_done.set()

        with mock.patch.object(
            release, "_write_atomic_bytes", side_effect=delayed_launcher_write
        ):
            installer = threading.Thread(target=install)
            installer.start()
            self.assertTrue(launcher_written.wait(2.0), "install did not reach launcher publication")
            reader = threading.Thread(target=read_status)
            reader.start()
            time.sleep(0.1)
            self.assertFalse(status_done.is_set(), "status observed a mixed pointer/launcher pair")
            allow_install.set()
            installer.join(3.0)
            reader.join(3.0)

        self.assertFalse(installer.is_alive(), "install thread leaked")
        self.assertFalse(reader.is_alive(), "status thread leaked")
        self.assertEqual(install_errors, [])
        self.assertEqual(status_errors, [])
        self.assertEqual(len(status_result), 1)
        self.assertEqual(status_result[0]["status"], "ready")
        self.assertEqual(status_result[0]["workflow_commit"], release_b_commit)

    def test_run_waits_for_install_transaction_before_spawning_child(self) -> None:
        self._install()
        release_b, release_b_commit = self._make_second_release()
        launcher_written = threading.Event()
        allow_install = threading.Event()
        child_started = threading.Event()
        install_errors: list[BaseException] = []
        run_result: list[dict[str, object]] = []
        run_errors: list[BaseException] = []
        real_write_bytes = release._write_atomic_bytes
        real_start = release._start_released_agent

        def delayed_launcher_write(path: Path, payload: bytes, label: str) -> str:
            result = real_write_bytes(path, payload, label)
            if label == "stable launcher" and path.resolve() == (
                self.install_root / release.LAUNCHER_FILENAME
            ).resolve():
                launcher_written.set()
                if not allow_install.wait(3.0):
                    raise AssertionError("install transaction was not released")
            return result

        def install() -> None:
            try:
                release.install_release(release_b, self.install_root)
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                install_errors.append(exc)

        def start(
            command: list[str],
            release_root: Path,
            environment: dict[str, str],
        ) -> subprocess.Popen[bytes]:
            child_started.set()
            return real_start(command, release_root, environment)

        def run() -> None:
            try:
                run_result.append(release.run_agent(self.install_root, self.lane_root, self._args()))
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                run_errors.append(exc)

        with mock.patch.object(
            release, "_write_atomic_bytes", side_effect=delayed_launcher_write
        ):
            with mock.patch.object(release, "_start_released_agent", side_effect=start):
                installer = threading.Thread(target=install)
                installer.start()
                self.assertTrue(launcher_written.wait(2.0))
                runner = threading.Thread(target=run)
                runner.start()
                time.sleep(0.1)
                self.assertFalse(child_started.is_set(), "run spawned against mixed pointer/launcher state")
                allow_install.set()
                installer.join(3.0)
                runner.join(3.0)

        self.assertFalse(installer.is_alive(), "install thread leaked")
        self.assertFalse(runner.is_alive(), "run thread leaked")
        self.assertEqual(install_errors, [])
        self.assertEqual(run_errors, [])
        self.assertEqual(len(run_result), 1)
        self.assertEqual(run_result[0]["terminal_status"], "completed")
        self.assertEqual(run_result[0]["workflow_commit"], release_b_commit)

    def test_concurrent_installs_publish_one_consistent_launcher_pointer_pair(self) -> None:
        self._install()
        release_b, release_b_commit = self._make_second_release()
        release_c, release_c_commit = self._make_third_release()
        start_gate = threading.Barrier(2)
        results: list[dict[str, object]] = []
        errors: list[BaseException] = []
        real_write_bytes = release._write_atomic_bytes

        def delayed_write(path: Path, payload: bytes, label: str) -> str:
            result = real_write_bytes(path, payload, label)
            if label == "stable launcher":
                time.sleep(0.05)
            return result

        def install(root: Path) -> None:
            try:
                start_gate.wait(2.0)
                results.append(release.install_release(root, self.install_root))
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                errors.append(exc)

        with mock.patch.object(release, "_write_atomic_bytes", side_effect=delayed_write):
            first = threading.Thread(target=install, args=(release_b,))
            second = threading.Thread(target=install, args=(release_c,))
            first.start()
            second.start()
            first.join(5.0)
            second.join(5.0)

        self.assertFalse(first.is_alive(), "first concurrent install leaked")
        self.assertFalse(second.is_alive(), "second concurrent install leaked")
        self.assertEqual(errors, [])
        self.assertEqual(len(results), 2)
        _, pointer, identity = release._verify_pointer(self.install_root)
        self.assertIn(pointer["commit"], {release_b_commit, release_c_commit})
        self.assertEqual(pointer["agent_sha256"], identity["agent_sha256"])
        self.assertEqual(pointer["measurement_sha256"], identity["measurement_sha256"])
        self.assertEqual(release.release_status(self.install_root)["status"], "ready")

    def test_lock_cleanup_failure_preserves_primary_terminal_outcome(self) -> None:
        self._install()
        lock_path = self._lock_path().resolve()
        real_unlink = Path.unlink

        def fail_lock_unlink(path: Path, *args: object, **kwargs: object) -> None:
            if path.resolve() == lock_path:
                raise OSError("lock cleanup sentinel")
            real_unlink(path, *args, **kwargs)

        with mock.patch.object(
            Path, "unlink", autospec=True, side_effect=fail_lock_unlink
        ):
            result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["terminal_status"], "completed")
        cleanup_detail = (
            result.get("cleanup_error")
            or receipt.get("cleanup_error")
            or receipt.get("error")
            or result.get("cleanup_incomplete")
            or receipt.get("cleanup_incomplete")
        )
        self.assertTrue(cleanup_detail, "lock cleanup failure was not attached to the terminal")
        self.assertIn("lock cleanup sentinel", str(cleanup_detail))
        self.assertTrue(lock_path.exists(), "failed lock cleanup was silently discarded")
        real_unlink(Path(lock_path))

    def test_run_requires_explicit_owner_root_and_rejects_duplicate(self) -> None:
        self._install()
        with self.assertRaisesRegex(release.ReleaseError, "owner root"):
            release.run_agent(self.install_root, self.root / "missing-lane", self._args())
        with self.assertRaisesRegex(release.ReleaseError, "must not contain --root"):
            release.run_agent(self.install_root, self.lane_root, self._args() + ["--root", str(self.lane_root)])

    def test_pointer_or_launcher_tampering_is_not_runnable(self) -> None:
        self._install()
        pointer_path = self.install_root / release.POINTER_FILENAME
        pointer = json.loads(pointer_path.read_text(encoding="utf-8"))
        pointer["commit"] = "0" * 40
        pointer_path.write_text(json.dumps(pointer), encoding="utf-8")
        with self.assertRaisesRegex(release.ReleaseError, "self-hash"):
            release.run_agent(self.install_root, self.lane_root, self._args())

    def test_status_marks_dead_active_receipt_stale(self) -> None:
        self._install()
        _, pointer, _ = release._verify_pointer(self.install_root)
        lane = release._lane_bindings(self.lane_root, self._args())
        command = [
            sys.executable,
            str(self.release_root / "tools" / "agent.py"),
            "--root",
            str(self.lane_root),
            *self._args(),
        ]
        active = release._receipt_payload(
            pointer,
            self.lane_root,
            self._args(),
            lane,
            status="active",
            started_at=release._iso_now(),
            finished_at=None,
            terminal_status=None,
            exit_code=None,
            child_pid=999999,
            child_argv=command,
        )
        path = release._adoption_path(self.lane_root)
        release._write_atomic(path, active, "active adoption receipt")
        status = release.release_status(self.install_root, self.lane_root)
        self.assertEqual(status["status"], "drift")
        self.assertEqual(status["adoption"]["status"], "stale_active")

    def test_dead_active_receipt_does_not_permanently_block_restart(self) -> None:
        self._install()
        _, pointer, _ = release._verify_pointer(self.install_root)
        lane = release._lane_bindings(self.lane_root, self._args())
        command = [
            sys.executable,
            str(self.release_root / "tools" / "agent.py"),
            "--root",
            str(self.lane_root),
            *self._args(),
        ]
        active = release._receipt_payload(
            pointer,
            self.lane_root,
            self._args(),
            lane,
            status="active",
            started_at=release._iso_now(),
            finished_at=None,
            terminal_status=None,
            exit_code=None,
            child_pid=999999,
            child_argv=command,
        )
        release._write_atomic(
            release._adoption_path(self.lane_root),
            active,
            "active adoption receipt",
        )

        result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")

    def test_status_revalidates_lane_manifest_and_tool_cas_binding(self) -> None:
        self._install()
        result = release.run_agent(self.install_root, self.lane_root, self._args())
        self.assertEqual(result["terminal_status"], "completed")
        manifest = self.lane_root / "build" / "owner-campaign" / "campaign.json"
        manifest.write_bytes(manifest.read_bytes() + b"\n")

        status = release.release_status(self.install_root, self.lane_root)

        self.assertEqual(status["status"], "drift")
        self.assertIn("lane binding drift", status["error"])

    def test_command_line_stable_launcher_round_trip_has_no_owner_compile(self) -> None:
        self._install()
        command = [
            sys.executable,
            str(self.install_root / release.LAUNCHER_FILENAME),
            "run",
            "--root",
            str(self.lane_root),
            *self._args(),
        ]
        completed = subprocess.run(
            command,
            cwd=self.release_root,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        self.assertTrue(
            (self.lane_root / "build" / "owner-campaign" / "released-agent.json").is_file()
        )

    def test_stale_reclaim_contenders_never_overlap_or_delete_new_lock(self) -> None:
        """A stale read cannot let a second holder race the reclaiming holder."""

        lock_dir = self.root / "stale-reclaim-race"
        lock_dir.mkdir()
        lock_path = lock_dir / "race.lock"
        dead_pid = max(os.getpid() + 100000, 2000000)
        lock_path.write_text(str(dead_pid), encoding="utf-8")
        stale_checked = threading.Event()
        allow_reclaimer = threading.Event()
        reclaimer_acquired = threading.Event()
        contender_started = threading.Event()
        contender_acquired = threading.Event()
        reclaimer_release = threading.Event()
        contender_release = threading.Event()
        reclaimer_errors: list[BaseException] = []
        contender_errors: list[BaseException] = []
        first_pid_check = True
        pid_check_lock = threading.Lock()
        real_pid_alive = release._pid_alive

        def gated_pid_alive(pid: int) -> bool:
            nonlocal first_pid_check
            with pid_check_lock:
                pause = first_pid_check and pid == dead_pid
                if pause:
                    first_pid_check = False
            if pause:
                stale_checked.set()
                if not allow_reclaimer.wait(3.0):
                    raise AssertionError("stale reclaim was not released")
                return False
            return False if pid == dead_pid else real_pid_alive(pid)

        def reclaim() -> None:
            try:
                with release._cross_process_lock(
                    lock_dir,
                    lock_path.name,
                    "race lock",
                    release.ADOPTION_LOCK_SCHEMA,
                ):
                    reclaimer_acquired.set()
                    if not reclaimer_release.wait(3.0):
                        raise AssertionError("reclaimer was not released")
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                reclaimer_errors.append(exc)

        def contend() -> None:
            contender_started.set()
            try:
                with release._cross_process_lock(
                    lock_dir,
                    lock_path.name,
                    "race lock",
                    release.ADOPTION_LOCK_SCHEMA,
                ):
                    contender_acquired.set()
                    if not contender_release.wait(3.0):
                        raise AssertionError("contender was not released")
            except BaseException as exc:  # pragma: no cover - assertion below reports it
                contender_errors.append(exc)

        with mock.patch.object(release, "_pid_alive", side_effect=gated_pid_alive):
            reclaimer = threading.Thread(target=reclaim)
            reclaimer.start()
            self.assertTrue(stale_checked.wait(2.0), "reclaimer never read the stale lock")
            contender = threading.Thread(target=contend)
            contender.start()
            self.assertTrue(contender_started.wait(2.0), "contender never started")
            # The old read/unlink sequence lets the contender acquire here;
            # the guarded implementation keeps it behind the reclaimer.
            time.sleep(0.1)
            allow_reclaimer.set()
            deadline = time.monotonic() + 2.0
            while (
                not reclaimer_acquired.is_set()
                and not contender_acquired.is_set()
                and time.monotonic() < deadline
            ):
                time.sleep(0.01)
            self.assertTrue(
                reclaimer_acquired.is_set() or contender_acquired.is_set(),
                f"neither contender acquired: {reclaimer_errors!r} {contender_errors!r}",
            )
            # Whichever contender wins the stale-reclaim race must remain the
            # sole holder.  The other contender may not overlap it or remove
            # its newly published lock.
            time.sleep(0.1)
            self.assertFalse(
                reclaimer_acquired.is_set() and contender_acquired.is_set(),
                "both contenders acquired the lock concurrently",
            )
            if reclaimer_acquired.is_set():
                reclaimer_release.set()
                reclaimer.join(3.0)
                if contender.is_alive():
                    self.assertTrue(
                        contender_acquired.wait(2.0),
                        "contender neither rejected nor acquired after release",
                    )
                    contender_release.set()
            else:
                contender_release.set()
                contender.join(3.0)
            reclaimer.join(3.0)
            contender.join(3.0)

        self.assertFalse(reclaimer.is_alive(), "reclaimer thread leaked")
        self.assertFalse(contender.is_alive(), "contender thread leaked")
        self.assertTrue(
            all(isinstance(error, release.ReleaseError) for error in reclaimer_errors),
            reclaimer_errors,
        )
        self.assertTrue(
            all(isinstance(error, release.ReleaseError) for error in contender_errors),
            contender_errors,
        )
        self.assertFalse(lock_path.exists(), "race lock leaked after both holders released")

    def test_final_cleanup_identity_replacement_is_not_removed(self) -> None:
        """Cleanup must leave a replacement lock owned by another contender."""

        lock_dir = self.root / "cleanup-identity-race"
        lock_dir.mkdir()
        lock_path = lock_dir / "race.lock"
        replacement = {
            "schema": release.ADOPTION_LOCK_SCHEMA,
            "pid": os.getpid(),
            "created_at": release._iso_now(),
            "recoverable": True,
            "retained_reason": None,
            "child_pid": None,
        }

        with release._cross_process_lock(
            lock_dir,
            lock_path.name,
            "race lock",
            release.ADOPTION_LOCK_SCHEMA,
        ) as held:
            replacement_path = lock_dir / "replacement.lock"
            replacement_path.write_bytes(release._canonical(replacement))
            os.replace(replacement_path, lock_path)

        self.assertTrue(lock_path.exists(), "replacement lock was removed during cleanup")
        self.assertEqual(json.loads(lock_path.read_text(encoding="utf-8")), replacement)
        self.assertIsNotNone(held.cleanup_error)
        self.assertIn("identity changed", str(held.cleanup_error))

    def _assert_tree_cleanup_failure_preserves_primary(
        self, extra_args: tuple[str, ...], expected_status: str, expected_exit: int
    ) -> None:
        self._install()
        with mock.patch.object(
            release,
            "_terminate_process_tree",
            side_effect=release.ReleaseError("tree cleanup sentinel"),
        ):
            result = release.run_agent(
                self.install_root,
                self.lane_root,
                self._args(*extra_args),
            )

        self.assertEqual(result["terminal_status"], expected_status)
        self.assertEqual(result["exit_code"], expected_exit)
        self.assertTrue(result.get("lane_quarantined"))
        self.assertIn("tree cleanup sentinel", str(result.get("cleanup_error")))
        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["terminal_status"], expected_status)
        self.assertEqual(receipt["exit_code"], expected_exit)
        self.assertIn("tree cleanup sentinel", str(receipt["error"]))
        self.assertTrue(self._lock_path().exists(), "unproved tree cleanup released the lane lock")
        with self.assertRaisesRegex(release.ReleaseError, "held|retained|cleanup|quarant"):
            release.run_agent(self.install_root, self.lane_root, self._args())

    def test_tree_cleanup_failure_preserves_completed_primary_and_blocks_restart(self) -> None:
        self._assert_tree_cleanup_failure_preserves_primary((), "completed", 0)

    def test_tree_cleanup_failure_preserves_failed_primary_and_blocks_restart(self) -> None:
        self._assert_tree_cleanup_failure_preserves_primary(("--fail",), "failed", 7)

    def test_terminal_receipt_failure_preserves_primary_and_blocks_restart(self) -> None:
        self._install()
        real_write = release._write_atomic

        def fail_terminal_receipt(path: Path, value: object, label: str) -> str:
            if label == "terminal adoption receipt":
                raise release.ReleaseError("terminal receipt publication sentinel")
            return real_write(path, value, label)

        with mock.patch.object(release, "_write_atomic", side_effect=fail_terminal_receipt):
            result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(result["exit_code"], 0)
        self.assertFalse(result["receipt_published"])
        self.assertIn("terminal receipt publication sentinel", result["receipt_publication_error"])
        self.assertTrue(result.get("lane_quarantined"))
        self.assertTrue(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).exists(),
            "receipt publication failure discarded the only durable lane state",
        )
        self.assertTrue(self._lock_path().exists(), "receipt failure released the lane lock")
        with self.assertRaisesRegex(release.ReleaseError, "held|retained|cleanup|quarant"):
            release.run_agent(self.install_root, self.lane_root, self._args())

    def test_cleanup_base_exception_quarantines_lane_without_releasing_primary(self) -> None:
        self._install()

        class CleanupBaseException(BaseException):
            pass

        with mock.patch.object(
            release,
            "_terminate_process_tree",
            side_effect=CleanupBaseException("base cleanup sentinel"),
        ):
            result = release.run_agent(self.install_root, self.lane_root, self._args())

        self.assertEqual(result["terminal_status"], "completed")
        self.assertEqual(result["exit_code"], 0)
        self.assertTrue(result.get("lane_quarantined"))
        self.assertIn("base cleanup sentinel", str(result.get("cleanup_error")))
        self.assertTrue(self._lock_path().exists(), "BaseException cleanup released the lane")
        with self.assertRaisesRegex(release.ReleaseError, "held|retained|cleanup|quarant"):
            release.run_agent(self.install_root, self.lane_root, self._args())

    def test_cleanup_keyboard_interrupt_preserves_receipt_and_quarantines_lane(self) -> None:
        self._install()
        with mock.patch.object(
            release,
            "_terminate_process_tree",
            side_effect=KeyboardInterrupt("keyboard cleanup sentinel"),
        ):
            with self.assertRaises(KeyboardInterrupt):
                release.run_agent(self.install_root, self.lane_root, self._args())

        receipt = json.loads(
            (self.lane_root / release.ADOPTION_RELATIVE_PATH).read_text(encoding="utf-8")
        )
        self.assertEqual(receipt["terminal_status"], "completed")
        self.assertEqual(receipt["exit_code"], 0)
        self.assertIn("keyboard cleanup sentinel", str(receipt["error"]))
        self.assertTrue(self._lock_path().exists(), "KeyboardInterrupt cleanup released the lane")
        with self.assertRaisesRegex(release.ReleaseError, "held|retained|cleanup|quarant"):
            release.run_agent(self.install_root, self.lane_root, self._args())


if __name__ == "__main__":
    unittest.main()
