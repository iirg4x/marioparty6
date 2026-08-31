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

    def _proposal_fixture(self) -> tuple[dict[str, object], Path, Path]:
        source_dir = self.root / "src"
        source_dir.mkdir(parents=True)
        campaign_source = source_dir / "owner.c"
        base = (
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 0;\n"
            "}\n"
            "int after = 2;\n"
        )
        campaign_source.write_text(base, encoding="utf-8")
        campaign = {
            "campaign_id": "proposal-test",
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "source_relpath": "src/owner.c",
            "toolchain_sha256": "b" * 64,
            "functions": ["focus"],
            "allowed_source_paths": ["src/owner.c"],
            "allowed_build_paths": ["build"],
            "forbidden_constructs": [r"\basm\b", r"\bvolatile\b"],
        }
        source_sha256 = owner_campaign._digest_file(campaign_source)
        sibling_digest = owner_campaign._digest_json([])
        focus_body: dict[str, object] = {
            "schema": "owner_campaign_focus_evidence/v1",
            "owner": campaign["owner"],
            "function": "focus",
            "unit": campaign["unit"],
            "source_path": campaign["source_relpath"],
            "base_commit": "0" * 40,
            "source_sha256": source_sha256,
            "target_object_sha256": "d" * 64,
            "strict_rows": ["strict:focus:row:1"],
            "data_rows": [],
            "physical_differences": [],
            "strict_row_ids": ["strict:focus:row:1"],
            "strict_row_ids_sha256": owner_campaign._digest_json(["strict:focus:row:1"]),
            "data_row_ids": [],
            "data_row_ids_sha256": owner_campaign._digest_json([]),
            "physical_difference_ids": [],
            "physical_difference_ids_sha256": owner_campaign._digest_json([]),
            "physical_target_identity_sha256": "e" * 64,
            "physical_candidate_identity_sha256": "f" * 64,
            "strict_row_count": 1,
            "data_row_count": 0,
            "physical_target_count": 0,
            "physical_candidate_count": 0,
            "physical_difference_count": 0,
            "protected_total": 0,
            "protected_losses": 0,
            "sibling_identities": [],
            "sibling_digest": sibling_digest,
        }
        focus_body["focus_evidence_sha256"] = owner_campaign._digest_json(focus_body)
        focus_digest = focus_body["focus_evidence_sha256"]
        focus_path = (
            owner_campaign._state_root(self.root) / "proof-cas" / "focus"
            / focus_digest[:2] / f"{focus_digest}.json"
        )
        focus_path.parent.mkdir(parents=True)
        focus_path.write_text(json.dumps(focus_body), encoding="utf-8")
        frontier_body: dict[str, object] = {
            "schema": owner_campaign.FRONTIER_SCHEMA,
            "campaign_id": campaign["campaign_id"],
            "owner": campaign["owner"],
            "unit": campaign["unit"],
            "function": "focus",
            "source_relpath": campaign["source_relpath"],
            "source_sha256": source_sha256,
            "target_object_sha256": "d" * 64,
            "toolchain_sha256": campaign["toolchain_sha256"],
            "candidate_object_sha256": "a" * 64,
            "metrics": {},
            "report_receipts": {"physical": "c" * 64},
            "focus_evidence_sha256": focus_digest,
            "parent_frontier_sha256": None,
            "generation": 0,
            "retained_at": "2026-08-31T00:00:00Z",
        }
        frontier = {
            **frontier_body,
            "frontier_sha256": owner_campaign._digest_json(frontier_body),
        }
        frontier_path = (
            owner_campaign._function_root(self.root, campaign, "focus")
            / "latest-frontier.json"
        )
        frontier_path.parent.mkdir(parents=True)
        frontier_path.write_text(json.dumps(frontier), encoding="utf-8")
        candidate_dir = self.root / "build" / "candidates"
        candidate_dir.mkdir(parents=True, exist_ok=True)
        candidate_source = candidate_dir / "focus.c"
        return campaign, campaign_source, candidate_source

    def test_propose_seals_current_frontier_bound_source_pair(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        result = lane.propose_candidate(
            self.root, campaign, "focus", candidate_source, "return-shape"
        )
        self.assertEqual(result["schema"], lane.PROPOSAL_RESULT_SCHEMA)
        self.assertEqual(result["status"], "queued")
        descriptor_path = self.root / result["candidate_descriptor"]
        source_path = self.root / result["candidate_source"]
        self.assertTrue(descriptor_path.is_file())
        self.assertTrue(source_path.is_file())
        descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
        self.assertEqual(set(descriptor), owner_campaign.CANDIDATE_FIELDS)
        self.assertEqual(descriptor["function"], "focus")
        self.assertEqual(descriptor["hypothesis_family"], "return-shape")
        self.assertEqual(
            descriptor["candidate_source"]["sha256"],
            owner_campaign._digest_file(source_path),
        )
        self.assertEqual(lane.discover_candidates(self.root, campaign), [descriptor_path])

    def test_propose_publishes_selector_sidecar_and_exact_defaults(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        result = lane.propose_candidate(
            self.root, campaign, "focus", candidate_source, "return-shape"
        )
        sidecar_path = self.root / result["candidate_selection"]
        self.assertTrue(sidecar_path.is_file())
        sidecar = json.loads(sidecar_path.read_text(encoding="utf-8"))
        self.assertEqual(sidecar["schema"], "owner_campaign_selection/v1")
        self.assertEqual(sidecar["status"], "RANKED_SOURCE_CLASS")
        self.assertEqual(sidecar["selection_kind"], "RANKED_SOURCE_CLASS")
        self.assertEqual(sidecar["rank"], 1)
        self.assertTrue(sidecar["ownership_complete"])
        self.assertEqual(sidecar["predicted_rows"], sidecar["residual_rows"])
        self.assertEqual(
            sidecar["predicted_remaining_counts"],
            {"strict": 0, "data": 0, "physical": 0},
        )
        self.assertTrue((self.root / sidecar["focus_artifact"]["path"]).is_file())
        self.assertTrue((self.root / sidecar["physical_artifact"]["path"]).is_file())
        self.assertEqual(
            sidecar["evidence_sha256"],
            owner_campaign._digest_json(
                {key: value for key, value in sidecar.items() if key != "evidence_sha256"}
            ),
        )

    def test_propose_requires_improved_remaining_counts(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "remaining counts"):
            lane.propose_candidate(
                self.root,
                campaign,
                "focus",
                candidate_source,
                "return-shape",
                expected_terminal="improved",
                predicted_rows=["strict:focus:row:1"],
            )
        self.assertEqual(list(lane.inbox_path(self.root, campaign).rglob("*")), [])

    def test_propose_accepts_improved_counts_and_binds_rows(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        result = lane.propose_candidate(
            self.root,
            campaign,
            "focus",
            candidate_source,
            "return-shape",
            expected_terminal="improved",
            predicted_rows=["strict:focus:row:1"],
            predicted_remaining_counts={"strict": 0, "data": 0, "physical": 0},
        )
        sidecar = json.loads(
            (self.root / result["candidate_selection"]).read_text(encoding="utf-8")
        )
        self.assertEqual(sidecar["expected_terminal"], "improved")
        self.assertEqual(sidecar["predicted_remaining_counts"]["strict"], 0)

    def test_propose_rejects_improved_counts_that_do_not_match_rows(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "do not match"):
            lane.propose_candidate(
                self.root,
                campaign,
                "focus",
                candidate_source,
                "return-shape",
                expected_terminal="improved",
                predicted_rows=["strict:focus:row:1"],
                predicted_remaining_counts={"strict": 1, "data": 0, "physical": 0},
            )

    def test_propose_rejects_predicted_row_outside_frontier(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "outside"):
            lane.propose_candidate(
                self.root,
                campaign,
                "focus",
                candidate_source,
                "return-shape",
                expected_terminal="improved",
                predicted_rows=["strict:focus:row:forged"],
                predicted_remaining_counts={"strict": 1, "data": 0, "physical": 0},
            )

    def test_propose_rejects_out_of_function_edits(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 3;\n",
            encoding="utf-8",
        )
        with self.assertRaises(owner_campaign.CampaignError):
            lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "bad-tail"
            )
        self.assertEqual(list(lane.inbox_path(self.root, campaign).rglob("*")), [])

    def test_propose_rejects_ambiguous_function_definition(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int focus(void) { return 2; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "ambiguous"):
            lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "ambiguous"
            )

    def test_propose_rejects_forbidden_construct_in_changed_source(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    asm(\"nop\");\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "forbidden"):
            lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "forbidden"
            )

    def test_propose_rejects_duplicate_candidate(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        lane.propose_candidate(self.root, campaign, "focus", candidate_source, "first")
        with self.assertRaisesRegex(owner_campaign.CampaignError, "duplicate"):
            lane.propose_candidate(self.root, campaign, "focus", candidate_source, "second")

    def test_propose_rejects_frontier_source_drift(self) -> None:
        campaign, campaign_source, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        campaign_source.write_text(
            "int before = 9;\n\n"
            "int focus(void) {\n"
            "    return 0;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "drift"):
            lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "drift"
            )


if __name__ == "__main__":
    unittest.main()
