from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import threading
import unittest
from unittest import mock

from tools import owner_campaign as campaign


def _fixture_type():
    # Import inside the helper so unittest does not rediscover every fixture
    # test merely because the fixture class is an attribute of this module.
    from tools.tests.test_owner_campaign import OwnerCampaignTests

    return OwnerCampaignTests


class FrontierRaceTests(unittest.TestCase):
    """Adversarial coverage for persisted frontier/restart consistency."""

    def setUp(self) -> None:
        _fixture_type().setUp(self)

    def tearDown(self) -> None:
        _fixture_type().tearDown(self)

    def load(self) -> dict[str, object]:
        return _fixture_type().load(self)

    def candidate(
        self, marker: str, frontier: dict[str, object], name: str = "cell",
    ) -> Path:
        return _fixture_type().candidate(self, marker, frontier, name)

    def test_snapshot_can_publish_stale_frontier_after_retained_gain(self) -> None:
        """Expose the missing source/frontier CAS around snapshot publication."""

        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate_path = self.candidate("IMPROVE", base, "race")
        candidate = campaign._load_candidate(self.root, candidate_path, loaded, base)

        scratch = campaign._ensure_scratch(self.root, loaded)
        campaign._sync_scratch_source(
            self.root, scratch, loaded, candidate["_source_bytes"]
        )
        measurement = campaign._run_hook(
            self.root, scratch, loaded, "focus", candidate["_source_sha256"], "candidate"
        )
        campaign._cleanup_cell_outputs(scratch, loaded)

        verified = threading.Event()
        release = threading.Event()
        snapshot_result: dict[str, object] = {}
        snapshot_error: list[BaseException] = []
        original_verify = campaign._verify_publication_sources

        def verify_and_pause(
            campaign_value: dict[str, object], scratch_value: Path, *,
            live_sha256: str | None, scratch_sha256: str,
        ) -> None:
            original_verify(
                campaign_value, scratch_value,
                live_sha256=live_sha256, scratch_sha256=scratch_sha256,
            )
            if live_sha256 == base["source_sha256"]:
                verified.set()
                if not release.wait(10):
                    raise AssertionError("snapshot publication barrier was not released")

        def snapshot() -> None:
            try:
                snapshot_result["frontier"] = campaign.snapshot_frontier(
                    self.root, loaded, "focus", force=True
                )
            except BaseException as exc:  # pragma: no cover - surfaced below
                snapshot_error.append(exc)

        with mock.patch.object(
            campaign, "_verify_publication_sources", side_effect=verify_and_pause
        ):
            thread = threading.Thread(target=snapshot, daemon=True)
            thread.start()
            self.assertTrue(verified.wait(10), "snapshot did not reach publication barrier")

            retained, _ = campaign._retain(
                self.root, loaded, base, candidate, measurement, "improved"
            )
            self.assertIsNotNone(retained)
            self.assertEqual(
                campaign._digest_file(self.source), candidate["_source_sha256"]
            )
            release.set()
            thread.join(10)

        self.assertFalse(thread.is_alive(), "snapshot thread did not terminate")
        self.assertEqual(snapshot_error, [])
        self.assertIn("frontier", snapshot_result)

        latest_path = campaign._function_root(self.root, loaded, "focus") / "latest-frontier.json"
        latest = json.loads(latest_path.read_text(encoding="utf-8"))
        self.assertEqual(
            latest["source_sha256"], candidate["_source_sha256"],
            "snapshot overwrote the retained candidate frontier",
        )

    def test_persisted_pending_frontier_survives_restart_shape(self) -> None:
        """A killed writer leaves compact pending state recoverable on restart."""

        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate_path = self.candidate("IMPROVE", base, "restart")
        directory = campaign._function_root(self.root, loaded, "focus")
        state_file = self.root / "build" / "restart-state.json"
        env = dict(os.environ)
        package_root = str(Path(__file__).resolve().parents[2])
        env["PYTHONPATH"] = os.pathsep.join(
            item for item in (package_root, env.get("PYTHONPATH")) if item
        )
        crash_script = (
            "import os, pathlib, sys\n"
            "from tools import owner_campaign as c\n"
            "root = pathlib.Path(sys.argv[1])\n"
            "manifest = root / 'build' / 'campaign.json'\n"
            "loaded = c.load_campaign(root, manifest)\n"
            "base = c.snapshot_frontier(root, loaded, 'focus')\n"
            "candidate_path = root / 'build' / 'candidates' / 'restart.json'\n"
            "candidate = c._load_candidate(root, candidate_path, loaded, base)\n"
            "scratch = c._ensure_scratch(root, loaded)\n"
            "c._sync_scratch_source(root, scratch, loaded, candidate['_source_bytes'])\n"
            "measurement = c._run_hook(root, scratch, loaded, 'focus', candidate['_source_sha256'], 'candidate')\n"
            "original = c._atomic_bytes\n"
            "source = loaded['_source'].resolve()\n"
            "def crash_after_source(path, value):\n"
            "    original(path, value)\n"
            "    if pathlib.Path(path).resolve() == source:\n"
            "        os._exit(17)\n"
            "c._atomic_bytes = crash_after_source\n"
            "c._retain(root, loaded, base, candidate, measurement, 'improved')\n"
        )
        crashed = subprocess.run(
            [sys.executable, "-c", crash_script,
             os.fspath(self.root)],
            cwd=package_root, env=env, capture_output=True, text=True, check=False,
        )
        self.assertEqual(crashed.returncode, 17, crashed.stderr)
        self.assertTrue((directory / "frontier.pending.json").is_file())
        candidate_sha = campaign._digest_file(self.source)
        self.assertNotEqual(candidate_sha, base["source_sha256"])

        restart_script = (
            "import json, pathlib, sys\n"
            "from tools import owner_campaign as c\n"
            "root = pathlib.Path(sys.argv[1])\n"
            "manifest = root / 'build' / 'campaign.json'\n"
            "loaded = c.load_campaign(root, manifest)\n"
            "frontier = c.snapshot_frontier(root, loaded, 'focus')\n"
            "pathlib.Path(sys.argv[2]).write_text(json.dumps(frontier), encoding='utf-8')\n"
        )
        restarted = subprocess.run(
            [sys.executable, "-c", restart_script,
             os.fspath(self.root), os.fspath(state_file)],
            cwd=package_root, env=env, capture_output=True, text=True, check=False,
        )
        self.assertEqual(restarted.returncode, 0, restarted.stderr)
        recovered = json.loads(state_file.read_text(encoding="utf-8"))
        self.assertEqual(recovered["source_sha256"], candidate_sha)
        self.assertFalse((directory / "frontier.pending.json").exists())
        self.assertLessEqual(
            (directory / "latest-frontier.json").stat().st_size,
            loaded["limits"]["frontier_bytes"],
        )


if __name__ == "__main__":
    unittest.main()
