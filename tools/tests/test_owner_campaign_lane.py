from __future__ import annotations

import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools import owner_campaign
from tools import owner_campaign_lane as lane


def _digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _seal(body: dict[str, object], field: str) -> dict[str, object]:
    value = dict(body)
    value[field] = owner_campaign._digest_json(body)
    return value


class OwnerCampaignLaneTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        self.campaign = {
            "campaign_id": "lane-driver-test",
            "owner": "main:test/owner",
            "functions": ["focus"],
            "allowed_build_paths": ["build"],
        }
        self.inbox = lane.inbox_path(self.root, self.campaign)
        self.inbox.mkdir(parents=True)
        (self.root / "build" / "candidates").mkdir(parents=True)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _candidate(self, name: str, *, created_at: str = "2026-08-31T00:00:00Z") -> Path:
        source = self.root / "build" / "candidates" / f"{name}.c"
        source.write_text(f"int focus(void) {{ return 0; }} /* {name} */\n", encoding="utf-8")
        span_sha = owner_campaign._digest_file(source)
        body: dict[str, object] = {
            "schema": owner_campaign.CANDIDATE_SCHEMA,
            "campaign_id": self.campaign["campaign_id"],
            "function": "focus",
            "base_frontier_sha256": "a" * 64,
            "candidate_source": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": owner_campaign._digest_file(source),
            },
            "function_span": {
                "base_start_line": 1,
                "base_end_line": 1,
                "candidate_start_line": 1,
                "candidate_end_line": 1,
                "base_sha256": span_sha,
                "candidate_sha256": span_sha,
            },
            "hypothesis_family": f"family-{name}",
            "natural_c": True,
            "created_at": created_at,
        }
        descriptor = self.inbox / f"{name}.json"
        descriptor.write_text(
            json.dumps(_seal(body, "candidate_sha256")), encoding="utf-8"
        )
        return descriptor

    def test_empty_inbox_is_explicit_idle(self) -> None:
        result = lane.run_inbox(self.root, self.campaign)
        self.assertEqual(result["schema"], lane.LANE_RESULT_SCHEMA)
        self.assertEqual(result["status"], "idle")
        self.assertEqual(result["discovered"], 0)
        self.assertEqual(result["dispatched"], 0)
        self.assertEqual(result["results"], [])

    def test_dispatches_at_most_five_and_compacts_terminal_inputs(self) -> None:
        descriptors = [self._candidate(f"cell-{index}") for index in range(6)]
        observed: list[Path] = []

        def dispatch(root: Path, campaign: dict[str, object], paths: list[Path]) -> list[dict[str, object]]:
            observed.extend(paths)
            return [
                {"status": "no_gain", "authority_advanced": False}
                for _ in paths
            ]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(observed), 5)
        self.assertEqual(result["status"], "processed")
        self.assertEqual(result["dispatched"], 5)
        self.assertEqual(len(result["cleaned"]), 10)
        self.assertTrue(descriptors[5].exists())
        self.assertTrue(
            (self.root / "build" / "candidates" / "cell-5.c").exists()
        )
        for descriptor in descriptors[:5]:
            self.assertFalse(descriptor.exists())
        for index in range(5):
            self.assertFalse((self.root / "build" / "candidates" / f"cell-{index}.c").exists())

    def test_infrastructure_retry_preserves_descriptor_and_source(self) -> None:
        descriptor = self._candidate("retry")
        source = self.root / "build" / "candidates" / "retry.c"
        with patch.object(
            owner_campaign,
            "run_loop",
            return_value=[{"status": "infra_retry", "authority_advanced": False}],
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "infra_retry")
        self.assertEqual(result["preserved_infrastructure"], [
            descriptor.relative_to(self.root).as_posix()
        ])
        self.assertTrue(descriptor.exists())
        self.assertTrue(source.exists())

    def test_stale_rebase_preserves_descriptor_and_source(self) -> None:
        descriptor = self._candidate("stale")
        source = self.root / "build" / "candidates" / "stale.c"
        with patch.object(
            owner_campaign,
            "run_loop",
            return_value=[{"status": "stale_rebase", "authority_advanced": False}],
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "processed")
        self.assertEqual(
            result["preserved_infrastructure"],
            [descriptor.relative_to(self.root).as_posix()],
        )
        self.assertTrue(descriptor.exists())
        self.assertTrue(source.exists())

    def test_supervisor_survives_empty_inbox_then_processes_candidate(self) -> None:
        clock_value = [0.0]
        sleep_calls: list[float] = []
        created = [False]
        observed: list[Path] = []

        def clock() -> float:
            return clock_value[0]

        def sleeper(duration: float) -> None:
            sleep_calls.append(duration)
            clock_value[0] += duration
            if not created[0]:
                self._candidate("late")
                created[0] = True

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> list[dict[str, object]]:
            observed.extend(paths)
            return [
                {"status": "improved", "authority_advanced": False}
                for _ in paths
            ]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_supervisor(
                self.root,
                self.campaign,
                idle_timeout_seconds=0.25,
                watchdog_seconds=2.0,
                poll_interval_seconds=0.05,
                clock=clock,
                sleeper=sleeper,
            )

        self.assertEqual(result["status"], "idle_timeout")
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(result["outcomes"], {"improved": 1})
        self.assertEqual(len(observed), 1)
        self.assertTrue(sleep_calls)

    def test_supervisor_honors_cancellation(self) -> None:
        with patch.object(
            owner_campaign,
            "_check_cancelled",
            side_effect=owner_campaign.CampaignError(
                "campaign is cancelled at the active epoch"
            ),
        ):
            result = lane.run_supervisor(
                self.root,
                self.campaign,
                idle_timeout_seconds=1.0,
                watchdog_seconds=1.0,
                poll_interval_seconds=0.05,
                clock=lambda: 0.0,
                sleeper=lambda _duration: self.fail("cancelled lane slept"),
            )

        self.assertEqual(result["status"], "cancelled")
        self.assertEqual(result["dispatched"], 0)

    def test_supervisor_honors_closed_campaign(self) -> None:
        with patch.object(
            owner_campaign,
            "campaign_status",
            return_value={"exact_count": 1, "total": 1},
        ):
            result = lane.run_supervisor(
                self.root,
                self.campaign,
                idle_timeout_seconds=1.0,
                watchdog_seconds=1.0,
                poll_interval_seconds=0.05,
                clock=lambda: 0.0,
                sleeper=lambda _duration: self.fail("closed lane slept"),
            )

        self.assertEqual(result["status"], "closed")
        self.assertEqual(result["reason"], "all campaign functions are exact")
        self.assertEqual(result["batches"], 0)

    def test_batch_infrastructure_error_preserves_every_input(self) -> None:
        descriptors = [self._candidate(f"batch-{index}") for index in range(2)]
        with patch.object(
            owner_campaign,
            "run_loop",
            side_effect=owner_campaign.InfrastructureError("compiler unavailable"),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "infra_retry")
        self.assertEqual(len(result["results"]), 2)
        self.assertTrue(all(item["status"] == "infra_retry" for item in result["results"]))
        self.assertTrue(all(path.exists() for path in descriptors))
        self.assertEqual(len(list((self.root / "build" / "candidates").glob("*.c"))), 2)

    def test_driver_has_no_legacy_control_dependency(self) -> None:
        source = Path(lane.__file__).read_text(encoding="utf-8").lower()
        for forbidden in ("stop", "permit", "hmac"):
            self.assertNotIn(forbidden, source)


if __name__ == "__main__":
    unittest.main()
