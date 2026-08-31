from __future__ import annotations

import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools import owner_campaign
from tools import owner_campaign_lane as lane
from tools import owner_campaign_selector as selector


def _sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _seal(value: dict[str, object], field: str) -> dict[str, object]:
    result = dict(value)
    result[field] = owner_campaign._digest_json(value)
    return result


class OwnerCampaignSelectorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        self.campaign: dict[str, object] = {
            "campaign_id": "selector-test",
            "owner": "main:test/owner",
            "unit": "main/test",
            "functions": ["focus"],
            "allowed_build_paths": ["build"],
        }
        self.inbox = lane.inbox_path(self.root, self.campaign)
        self.inbox.mkdir(parents=True)
        (self.root / "build" / "candidates").mkdir(parents=True)
        frontier_body: dict[str, object] = {
            "function": "focus",
            "unit": "main/test",
            "source_sha256": "a" * 64,
            "toolchain_sha256": "b" * 64,
        }
        self.frontier = _seal(frontier_body, "frontier_sha256")
        self.campaign["_selection_frontier"] = self.frontier
        self.protected_digest = "c" * 64

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _artifact(self, name: str, value: dict[str, object], *, self_hash: bool = False) -> tuple[Path, str]:
        payload = _seal(value, "artifact_sha256") if self_hash else value
        path = self.root / "build" / "candidates" / name
        path.write_text(json.dumps(payload, sort_keys=True), encoding="utf-8")
        return path, _sha_bytes(path.read_bytes())

    def _proposal(
        self,
        name: str,
        *,
        rank: int = 1,
        status: str = "RANKED_SOURCE_CLASS",
        residual: list[str] | None = None,
        predicted: list[str] | None = None,
        predicted_counts: dict[str, int] | None = None,
        ownership_complete: bool = True,
        source_class: str = "direct_semantic_owner",
    ) -> tuple[Path, Path, Path]:
        residual = residual if residual is not None else ["strict:focus:row:1"]
        predicted = predicted if predicted is not None else list(residual)
        predicted_counts = predicted_counts or {"strict": 0, "data": 0, "physical": 0}
        source = self.root / "build" / "candidates" / f"{name}.c"
        source.write_text(f"int focus(void) {{ return 0; }} /* {name} */\n", encoding="utf-8")
        source_sha = _sha_bytes(source.read_bytes())
        descriptor_body: dict[str, object] = {
            "schema": owner_campaign.CANDIDATE_SCHEMA,
            "campaign_id": self.campaign["campaign_id"],
            "function": "focus",
            "base_frontier_sha256": self.frontier["frontier_sha256"],
            "candidate_source": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": source_sha,
            },
            "function_span": {
                "base_start_line": 1,
                "base_end_line": 1,
                "candidate_start_line": 1,
                "candidate_end_line": 1,
                "base_sha256": source_sha,
                "candidate_sha256": source_sha,
            },
            "hypothesis_family": f"family-{name}",
            "natural_c": True,
            "created_at": "2026-08-31T00:00:00Z",
        }
        descriptor = self.inbox / f"{name}.json"
        descriptor.write_text(
            json.dumps(_seal(descriptor_body, "candidate_sha256"), sort_keys=True),
            encoding="utf-8",
        )
        focus_rows = {
            "strict_row_ids": [row for row in residual if row.startswith("strict:")],
            "data_row_ids": [row for row in residual if row.startswith("data:")],
            "physical_difference_ids": [row for row in residual if row.startswith("physical:")],
        }
        focus, focus_sha = self._artifact(
            f"{name}.focus.json",
            {**focus_rows, "sibling_digest": self.protected_digest},
            self_hash=True,
        )
        physical, physical_sha = self._artifact(
            f"{name}.physical.json",
            {"physical_difference_ids": [row for row in residual if row.startswith("physical:")]},
        )
        evidence_body: dict[str, object] = {
            "schema": selector.SCHEMA,
            "status": status,
            "selection_kind": status,
            "campaign_id": self.campaign["campaign_id"],
            "owner": self.campaign["owner"],
            "unit": self.campaign["unit"],
            "function": "focus",
            "rank": rank,
            "source_class": source_class,
            "candidate": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": source_sha,
            },
            "frontier": {
                "sha256": self.frontier["frontier_sha256"],
                "source_sha256": self.frontier["source_sha256"],
                "function": "focus",
                "unit": self.campaign["unit"],
                "toolchain_sha256": self.frontier["toolchain_sha256"],
            },
            "source_sha256": self.frontier["source_sha256"],
            "toolchain_sha256": self.frontier["toolchain_sha256"],
            "focus_artifact": {
                "path": focus.relative_to(self.root).as_posix(),
                "sha256": focus_sha,
            },
            "physical_artifact": {
                "path": physical.relative_to(self.root).as_posix(),
                "sha256": physical_sha,
            },
            "residual_rows": residual,
            "predicted_rows": predicted,
            "predicted_remaining_counts": predicted_counts,
            "protected_sibling_digest": self.protected_digest,
            "ownership_complete": ownership_complete,
        }
        sidecar = selector.selection_evidence_path(descriptor)
        sidecar.write_text(
            json.dumps(_seal(evidence_body, "evidence_sha256"), sort_keys=True),
            encoding="utf-8",
        )
        return descriptor, source, sidecar

    def _write_outcome(
        self, *, status: str, source_class: str = "direct_semantic_owner",
        predicted: list[str] | None = None,
    ) -> Path:
        predicted = predicted if predicted is not None else ["strict:focus:row:1"]
        body: dict[str, object] = {
            "schema": selector.OUTCOME_SCHEMA,
            "campaign_id": self.campaign["campaign_id"],
            "owner": self.campaign["owner"],
            "unit": self.campaign["unit"],
            "function": "focus",
            "status": status,
            "frontier_sha256": self.frontier["frontier_sha256"],
            "source_class": source_class,
            "predicted_rows": predicted,
            "predicted_row_group_sha256": selector._row_group_digest(predicted),
            "candidate_source_sha256": "d" * 64,
            "candidate_object_sha256": "e" * 64,
        }
        record = _seal(body, "outcome_sha256")
        path = selector.selection_outcome_ledger_path(self.root, self.campaign, "focus")
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(record, sort_keys=True) + "\n", encoding="utf-8")
        return path

    def test_one_valid_proposal_among_five_dispatches_only_one(self) -> None:
        proposals = [
            self._proposal("winner", rank=1),
            self._proposal("lower-a", rank=2),
            self._proposal("lower-b", rank=2),
            self._proposal("lower-c", rank=2),
            self._proposal("lower-d", rank=2),
        ]
        observed: list[Path] = []

        def dispatch(root: Path, campaign: dict[str, object], paths: list[Path]) -> list[dict[str, object]]:
            observed.extend(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(observed), 1)
        self.assertEqual(observed[0], proposals[0][0])
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(result["selection"]["status"], selector.SELECTED)
        self.assertFalse(proposals[0][0].exists())
        self.assertFalse(proposals[0][1].exists())
        for _descriptor, source, sidecar in proposals[1:]:
            self.assertTrue(source.exists())
            self.assertTrue(sidecar.exists())

    def test_captured_unknown_owner_dispatches_zero(self) -> None:
        residual = [f"strict:ConfigPadMain:row:{index}" for index in range(116)]
        descriptor, source, sidecar = self._proposal(
            "config-pad-main",
            status="UNKNOWN",
            ownership_complete=False,
            residual=residual,
        )
        with patch.object(owner_campaign, "run_loop", side_effect=AssertionError("must not compile")):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "selection_unknown")
        self.assertEqual(result["dispatched"], 0)
        self.assertEqual(result["selection"]["selection_status"], selector.UNKNOWN)
        self.assertTrue(descriptor.exists())
        self.assertTrue(source.exists())
        self.assertTrue(sidecar.exists())

    def test_rank_one_tie_dispatches_zero(self) -> None:
        first = self._proposal("first")
        second = self._proposal("second")
        with patch.object(owner_campaign, "run_loop", side_effect=AssertionError("must not compile")):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "selection_unknown")
        self.assertIn("tie", result["reason"])
        self.assertEqual(result["dispatched"], 0)
        self.assertTrue(first[1].exists())
        self.assertTrue(second[1].exists())

    def test_predicted_row_outside_current_residual_dispatches_zero(self) -> None:
        descriptor, source, sidecar = self._proposal(
            "forged-row", predicted=["strict:focus:row:forged"]
        )
        with patch.object(owner_campaign, "run_loop", side_effect=AssertionError("must not compile")):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "selection_unknown")
        self.assertEqual(result["dispatched"], 0)
        self.assertTrue(descriptor.exists())
        self.assertTrue(source.exists())
        self.assertTrue(sidecar.exists())

    def test_historical_exact_style_prediction_is_accepted(self) -> None:
        descriptor, _source, _sidecar = self._proposal(
            "setup-mg-type",
            residual=["strict:SetupMgType:row:11", "data:SetupMgType:row:11"],
            predicted=["strict:SetupMgType:row:11", "data:SetupMgType:row:11"],
            source_class="historical_exact_source_shape",
        )
        selection = selector.select_winning_candidate(self.root, self.campaign, [descriptor])
        self.assertEqual(selection["status"], selector.SELECTED)
        self.assertEqual(selection["selected"]["source_class"], "historical_exact_source_shape")
        self.assertEqual(selection["selected"]["predicted_remaining_counts"], {"strict": 0, "data": 0, "physical": 0})

    def test_prior_no_gain_same_frontier_class_and_rows_is_suppressed(self) -> None:
        descriptor, source, sidecar = self._proposal("already-measured")
        self._write_outcome(status="no_gain")
        with patch.object(owner_campaign, "run_loop", side_effect=AssertionError("must not compile")):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "selection_unknown")
        self.assertEqual(result["dispatched"], 0)
        self.assertIn("suppressed", result["reason"])
        self.assertTrue(descriptor.exists())
        self.assertTrue(source.exists())
        self.assertTrue(sidecar.exists())

    def test_infrastructure_failure_does_not_suppress_same_selection(self) -> None:
        descriptor, _source, _sidecar = self._proposal("retryable")
        self._write_outcome(status="infrastructure_failed")
        selection = selector.select_winning_candidate(self.root, self.campaign, [descriptor])

        self.assertEqual(selection["status"], selector.SELECTED)
        self.assertEqual(selection["selected"]["selection_key_sha256"], selector._selection_key(
            self.frontier["frontier_sha256"], "direct_semantic_owner", ["strict:focus:row:1"]
        ))


if __name__ == "__main__":
    unittest.main()
