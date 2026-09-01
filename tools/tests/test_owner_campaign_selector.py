from __future__ import annotations

import hashlib
import json
from pathlib import Path
import tempfile
import threading
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
        self.base_source = self.root / "build" / "candidates" / "frontier.base.c"
        self.base_source.write_text(
            "int focus(void) { return 0; } /* frontier base */\n",
            encoding="utf-8",
        )
        base_source_sha = _sha_bytes(self.base_source.read_bytes())
        frontier_body: dict[str, object] = {
            "function": "focus",
            "unit": "main/test",
            "source_sha256": base_source_sha,
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
        function: str = "focus",
        frontier: dict[str, object] | None = None,
    ) -> tuple[Path, Path, Path]:
        frontier = frontier or self.frontier
        residual = residual if residual is not None else ["strict:focus:row:1"]
        predicted = predicted if predicted is not None else list(residual)
        predicted_counts = predicted_counts or {"strict": 0, "data": 0, "physical": 0}
        source = self.root / "build" / "candidates" / f"{name}.c"
        source.write_text(f"int {function}(void) {{ return 0; }} /* {name} */\n", encoding="utf-8")
        source_sha = _sha_bytes(source.read_bytes())
        descriptor_body: dict[str, object] = {
            "schema": owner_campaign.CANDIDATE_SCHEMA,
            "campaign_id": self.campaign["campaign_id"],
            "function": function,
            "base_frontier_sha256": frontier["frontier_sha256"],
            "base_source": {
                "path": self.base_source.relative_to(self.root).as_posix(),
                "sha256": frontier["source_sha256"],
            },
            "candidate_source": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": source_sha,
            },
            "function_span": {
                "base_start_line": 1,
                "base_end_line": 1,
                "candidate_start_line": 1,
                "candidate_end_line": 1,
                "base_sha256": frontier["source_sha256"],
                "candidate_sha256": source_sha,
            },
            "hypothesis_family": f"family-{name}",
            "natural_c": True,
            "rebase_depth": 0,
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
            "function": function,
            "rank": rank,
            "source_class": source_class,
            "candidate": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": source_sha,
            },
            "frontier": {
                "sha256": frontier["frontier_sha256"],
                "source_sha256": frontier["source_sha256"],
                "function": function,
                "unit": self.campaign["unit"],
                "toolchain_sha256": frontier["toolchain_sha256"],
            },
            "source_sha256": frontier["source_sha256"],
            "toolchain_sha256": frontier["toolchain_sha256"],
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

    def _decomposition_proposal(
        self,
        name: str,
        *,
        expected_terminal: str = "improved",
        predicted: list[str] | None = None,
        cluster_rows: list[str] | None = None,
        wrong_region: bool = False,
    ) -> tuple[Path, Path]:
        residual = ["strict:focus:row:1", "strict:focus:row:2"]
        predicted = predicted or ["strict:focus:row:1"]
        descriptor, _source, sidecar = self._proposal(
            name,
            residual=residual,
            predicted=predicted,
            predicted_counts={"strict": 2 - len(predicted), "data": 0, "physical": 0},
            ownership_complete=False,
        )
        region = {
            "cluster_id": "other" if wrong_region else "cluster-000",
            "closed": True,
            "strict_row_ids": cluster_rows or ["strict:focus:row:1"],
        }
        packet_body: dict[str, object] = {
            "status": "UNKNOWN",
            "owner": self.campaign["owner"],
            "unit": self.campaign["unit"],
            "function": "focus",
            "source_sha256": self.frontier["source_sha256"],
            "toolchain_sha256": self.frontier["toolchain_sha256"],
            # Live broad packets may have no parent frontier.
            "parent_frontier_sha256": None,
            "target_first_signal": {
                "status": "UNKNOWN",
                "next_action": "DECOMPOSE",
                "exact_terminal_possible": False,
            },
            "decomposition_regions": [region],
            "causal_clusters": [{
                "cluster_id": "cluster-000",
                "strict_row_ids": cluster_rows or ["strict:focus:row:1"],
            }],
        }
        packet = _seal(packet_body, "packet_sha256")
        packet_path = self.root / "build" / "evidence" / f"{name}.reconstruction.json"
        packet_path.parent.mkdir(parents=True, exist_ok=True)
        packet_path.write_text(json.dumps(packet, sort_keys=True), encoding="utf-8")
        evidence = json.loads(sidecar.read_text(encoding="utf-8"))
        evidence.pop("evidence_sha256")
        evidence["expected_terminal"] = expected_terminal
        evidence["ownership_scope"] = "bounded_decomposition_region"
        evidence["reconstruction"] = {
            "path": packet_path.relative_to(self.root).as_posix(),
            "sha256": _sha_bytes(packet_path.read_bytes()),
            "packet_sha256": packet["packet_sha256"],
            "status": "UNKNOWN",
            "next_action": "DECOMPOSE",
            "causal_cluster_id": "cluster-000",
            "bounded_region": region,
        }
        sidecar.write_text(
            json.dumps(_seal(evidence, "evidence_sha256"), sort_keys=True),
            encoding="utf-8",
        )
        return descriptor, packet_path

    def _write_outcome(
        self, *, status: str, source_class: str = "direct_semantic_owner",
        predicted: list[str] | None = None,
        candidate_sha: str = "d" * 64,
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
            "candidate_source_sha256": candidate_sha,
            "candidate_object_sha256": "e" * 64,
        }
        record = _seal(body, "outcome_sha256")
        path = selector.selection_outcome_ledger_path(self.root, self.campaign, "focus")
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("a", encoding="utf-8") as stream:
            stream.write(json.dumps(record, sort_keys=True) + "\n")
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

    def test_bounded_unknown_decomposition_dispatches_improved_with_parent_none(self) -> None:
        descriptor, _packet = self._decomposition_proposal("dossun-region")
        with patch.object(
            selector.owner_campaign_reconstruction, "verify_packet", return_value=None
        ):
            result = selector.select_winning_candidate(
                self.root, self.campaign, [descriptor]
            )

        self.assertEqual(result["status"], selector.SELECTED)
        self.assertEqual(result["selected"]["expected_terminal"], "improved")

    def test_bounded_decomposition_wrong_region_fails_closed(self) -> None:
        descriptor, _packet = self._decomposition_proposal(
            "wrong-region", wrong_region=True
        )
        with patch.object(
            selector.owner_campaign_reconstruction, "verify_packet", return_value=None
        ):
            result = selector.select_winning_candidate(
                self.root, self.campaign, [descriptor]
            )
        self.assertEqual(result["status"], selector.UNKNOWN)

    def test_bounded_decomposition_rows_outside_cluster_fail_closed(self) -> None:
        descriptor, _packet = self._decomposition_proposal(
            "wrong-rows", predicted=["strict:focus:row:2"]
        )
        with patch.object(
            selector.owner_campaign_reconstruction, "verify_packet", return_value=None
        ):
            result = selector.select_winning_candidate(
                self.root, self.campaign, [descriptor]
            )
        self.assertEqual(result["status"], selector.UNKNOWN)
        self.assertIn("first mismatch", result["reason"])

    def test_selection_rejects_later_only_probe(self) -> None:
        frontier_body = {
            key: value
            for key, value in self.frontier.items()
            if key != "frontier_sha256"
        }
        frontier_body["metrics"] = {
            "strict": {
                "target_bytes": 1000,
                "candidate_bytes": 1000,
                "differences": 2,
            },
            "data": {
                "target_bytes": 1000,
                "candidate_bytes": 1000,
                "differences": 2,
            },
        }
        frontier = _seal(frontier_body, "frontier_sha256")
        self.campaign["_selection_frontier"] = frontier
        residual = [
            "strict:focus:first", "strict:focus:later",
            "data:focus:first", "data:focus:later",
        ]
        descriptor, _source, _sidecar = self._proposal(
            "later-only",
            frontier=frontier,
            residual=residual,
            predicted=["strict:focus:later", "data:focus:later"],
            predicted_counts={"strict": 1, "data": 1, "physical": 0},
        )

        result = selector.select_winning_candidate(
            self.root, self.campaign, [descriptor]
        )
        self.assertEqual(result["status"], selector.UNKNOWN)
        self.assertIn("first mismatch", result["reason"])

    def test_selection_admits_first_mismatch_cluster(self) -> None:
        frontier_body = {
            key: value
            for key, value in self.frontier.items()
            if key != "frontier_sha256"
        }
        frontier_body["metrics"] = {
            "strict": {
                "target_bytes": 1000,
                "candidate_bytes": 1000,
                "differences": 2,
            },
            "data": {
                "target_bytes": 1000,
                "candidate_bytes": 1000,
                "differences": 2,
            },
        }
        frontier = _seal(frontier_body, "frontier_sha256")
        self.campaign["_selection_frontier"] = frontier
        residual = [
            "strict:focus:first", "strict:focus:later",
            "data:focus:first", "data:focus:later",
        ]
        descriptor, _source, _sidecar = self._proposal(
            "first-cluster",
            frontier=frontier,
            residual=residual,
            predicted=["strict:focus:first", "data:focus:first"],
            predicted_counts={"strict": 1, "data": 1, "physical": 0},
        )

        result = selector.select_winning_candidate(
            self.root, self.campaign, [descriptor]
        )
        self.assertEqual(result["status"], selector.SELECTED)
        self.assertEqual(
            result["selected"]["first_mismatch_rows"],
            ["strict:focus:first", "data:focus:first"],
        )

    def test_bounded_decomposition_never_authorizes_exact(self) -> None:
        descriptor, _packet = self._decomposition_proposal(
            "exact-prohibited", expected_terminal="exact"
        )
        with patch.object(
            selector.owner_campaign_reconstruction, "verify_packet", return_value=None
        ):
            result = selector.select_winning_candidate(
                self.root, self.campaign, [descriptor]
            )
        self.assertEqual(result["status"], selector.UNKNOWN)

    def test_bounded_decomposition_unverified_packet_fails_closed(self) -> None:
        descriptor, _packet = self._decomposition_proposal("unverified-packet")
        with patch.object(
            selector.owner_campaign_reconstruction,
            "verify_packet",
            side_effect=selector.owner_campaign_reconstruction.ReconstructionPacketError(
                "packet_sha256 mismatch"
            ),
        ):
            result = selector.select_winning_candidate(
                self.root, self.campaign, [descriptor]
            )
        self.assertEqual(result["status"], selector.UNKNOWN)
        self.assertIn("ownership is incomplete", result["reason"])

    def test_rank_one_tie_dispatches_one_deterministically(self) -> None:
        first = self._proposal("first")
        second = self._proposal("second")
        forward = selector.select_winning_candidate(
            self.root, self.campaign, [first[0], second[0]]
        )
        reverse = selector.select_winning_candidate(
            self.root, self.campaign, [second[0], first[0]]
        )
        expected = Path(forward["selected"]["descriptor_path"])
        self.assertEqual(
            expected, Path(reverse["selected"]["descriptor_path"])
        )
        observed: list[Path] = []

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            observed.extend(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "processed")
        self.assertIn("arbitrated", result["selection"]["reason"])
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(observed, [expected])
        consumed, preserved = (first, second) if expected == first[0] else (second, first)
        self.assertFalse(consumed[0].exists())
        self.assertFalse(consumed[1].exists())
        self.assertTrue(preserved[1].exists())

    def test_proposal_validation_uses_bounded_concurrent_workers(self) -> None:
        proposals = [self._proposal(f"parallel-{index}")[0] for index in range(5)]
        barrier = threading.Barrier(len(proposals), timeout=5)
        worker_ids: set[int] = set()
        original = selector._validate_proposal

        def validate(*args: object, **kwargs: object) -> dict[str, object]:
            worker_ids.add(threading.get_ident())
            barrier.wait()
            return original(*args, **kwargs)  # type: ignore[arg-type]

        with patch.object(selector, "_validate_proposal", side_effect=validate):
            result = selector.select_winning_candidate(self.root, self.campaign, proposals)

        self.assertEqual(result["status"], selector.SELECTED)
        self.assertEqual(len(worker_ids), len(proposals))
        self.assertLessEqual(len(worker_ids), selector.MAX_VALIDATION_WORKERS)

    def test_parallel_validation_keeps_selection_and_error_order_deterministic(self) -> None:
        proposals = [
            self._proposal("stable-winner")[0],
            self._proposal("stable-invalid", ownership_complete=False)[0],
            self._proposal("stable-lower", rank=2)[0],
        ]
        original = selector._validate_proposal

        def run_with_concurrent_barrier() -> dict[str, object]:
            barrier = threading.Barrier(len(proposals), timeout=5)

            def validate(*args: object, **kwargs: object) -> dict[str, object]:
                barrier.wait()
                return original(*args, **kwargs)  # type: ignore[arg-type]

            with patch.object(selector, "_validate_proposal", side_effect=validate):
                return selector.select_winning_candidate(self.root, self.campaign, proposals)

        first = run_with_concurrent_barrier()
        second = run_with_concurrent_barrier()

        self.assertEqual(first, second)
        self.assertEqual(first["status"], selector.SELECTED)
        self.assertEqual(
            [
                item["descriptor_relative"]
                for item in first["eligible"]
                if "descriptor_relative" in item
            ],
            [proposals[0].relative_to(self.root).as_posix()],
        )
        self.assertEqual(
            [item["descriptor"] for item in first["eligible"] if "reason" in item],
            [
                proposals[1].relative_to(self.root).as_posix(),
                proposals[2].relative_to(self.root).as_posix(),
            ],
        )

    def test_candidate_missing_base_source_is_rejected(self) -> None:
        descriptor, _source, _sidecar = self._proposal("missing-base-source")
        value = json.loads(descriptor.read_text(encoding="utf-8"))
        value.pop("base_source")
        descriptor.write_text(json.dumps(value, sort_keys=True), encoding="utf-8")

        result = selector.select_winning_candidate(self.root, self.campaign, [descriptor])

        self.assertEqual(result["status"], selector.UNKNOWN)
        self.assertIn("candidate descriptor is not a closed object", result["reason"])

    def test_candidate_base_source_hash_drift_is_rejected(self) -> None:
        descriptor, _source, _sidecar = self._proposal("drifted-base-source")
        value = json.loads(descriptor.read_text(encoding="utf-8"))
        base_path = self.root / value["base_source"]["path"]
        base_path.write_text("int focus(void) { return 1; }\n", encoding="utf-8")

        result = selector.select_winning_candidate(self.root, self.campaign, [descriptor])

        self.assertEqual(result["status"], selector.UNKNOWN)
        self.assertIn("candidate base source hash drift", result["reason"])

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

    def test_ranked_partial_improvement_with_remaining_rows_is_accepted(self) -> None:
        residual = [
            "strict:focus:row:1",
            "strict:focus:row:2",
            "data:focus:row:1",
            "data:focus:row:2",
            "physical:focus:row:1",
            "physical:focus:row:2",
        ]
        predicted = [
            "strict:focus:row:1",
            "data:focus:row:1",
            "physical:focus:row:1",
        ]
        descriptor, _source, _sidecar = self._proposal(
            "partial-frontier",
            residual=residual,
            predicted=predicted,
            predicted_counts={"strict": 1, "data": 1, "physical": 1},
        )

        selection = selector.select_winning_candidate(self.root, self.campaign, [descriptor])

        self.assertEqual(selection["status"], selector.SELECTED)
        self.assertEqual(selection["selected"]["predicted_rows"], predicted)
        self.assertEqual(
            selection["selected"]["predicted_remaining_counts"],
            {"strict": 1, "data": 1, "physical": 1},
        )

    def test_ranked_zero_improvement_is_rejected_by_count_binding(self) -> None:
        residual = [
            "strict:focus:row:1",
            "strict:focus:row:2",
            "data:focus:row:1",
            "data:focus:row:2",
            "physical:focus:row:1",
            "physical:focus:row:2",
        ]
        descriptor, _source, _sidecar = self._proposal(
            "zero-improvement",
            residual=residual,
            predicted=["strict:focus:row:1", "data:focus:row:1"],
            predicted_counts={"strict": 2, "data": 2, "physical": 2},
        )

        selection = selector.select_winning_candidate(self.root, self.campaign, [descriptor])

        self.assertEqual(selection["status"], selector.UNKNOWN)
        self.assertIn("predicted remaining counts", selection["reason"])

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

    def test_terminal_no_gain_outcome_survives_sidecar_cleanup(self) -> None:
        descriptor, source, sidecar = self._proposal("recorded")

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            return [{
                "status": "no_gain",
                "candidate_key": "candidate-key",
                "authority_advanced": False,
            }]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["results"][0]["status"], "no_gain")
        self.assertFalse(descriptor.exists())
        self.assertFalse(source.exists())
        self.assertFalse(sidecar.exists())
        ledger = selector.selection_outcome_ledger_path(
            self.root, self.campaign, "focus"
        )
        records = selector._ledger_records(ledger)
        self.assertEqual(len(records), 1)
        record = records[0]
        self.assertEqual(record["status"], "no_gain")
        self.assertEqual(record["source_class"], "direct_semantic_owner")
        self.assertEqual(record["predicted_rows"], ["strict:focus:row:1"])
        self.assertEqual(record["selection_key_sha256"], result["selection"]["selected"]["selection_key_sha256"])
        self.assertNotIn("candidate_source_path", record)
        self.assertNotIn("candidate_object_path", record)

    def test_selection_outcome_append_is_idempotent_after_recovery(self) -> None:
        descriptor, _source, _sidecar = self._proposal("idempotent")
        selection_result = selector.select_winning_candidate(
            self.root, self.campaign, [descriptor]
        )
        selection = selection_result["selected"]
        result = {
            "status": "no_gain",
            "result_sha256": "f" * 64,
            "authority_advanced": False,
        }

        first = selector.append_selection_outcome(
            self.root, self.campaign, selection, result
        )
        recovered = selector.append_selection_outcome(
            self.root, self.campaign, selection, result
        )

        self.assertEqual(recovered, first)
        ledger = selector.selection_outcome_ledger_path(
            self.root, self.campaign, "focus"
        )
        self.assertEqual(len(selector._ledger_records(ledger)), 1)

    def test_function_no_gain_budget_pivots_after_six_compiles(self) -> None:
        classes = [
            "index-owner", "base-owner", "result-owner", "aggregate-owner",
            "call-owner", "lifetime-owner", "loop-owner", "field-owner",
            "cursor-owner", "scalar-owner", "vector-owner", "return-owner",
        ]
        for index, source_class in enumerate(classes):
            self._proposal(
                f"budget-{index:02d}",
                residual=[f"strict:focus:row:{index + 1}"],
                predicted=[f"strict:focus:row:{index + 1}"],
                predicted_counts={"strict": 0, "data": 0, "physical": 0},
                source_class=source_class,
            )

        compile_calls: list[list[Path]] = []

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            compile_calls.append(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            for _ in range(6):
                result = lane.run_inbox(self.root, self.campaign)
            blocked = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(compile_calls), 6)
        self.assertEqual(blocked["status"], "pivot_required")
        self.assertEqual(blocked["dispatched"], 0)
        self.assertIn("function no_gain budget exhausted", blocked["reason"])

    def test_hypothesis_family_budget_closes_after_two_cosmetic_variants(self) -> None:
        proposals = []
        for index, source_class in enumerate(("owner-birth-v1", "owner-birth-v2", "owner-birth-v3")):
            proposals.append(self._proposal(
                f"family-{index}",
                residual=[f"strict:focus:row:{index + 1}"],
                predicted=[f"strict:focus:row:{index + 1}"],
                predicted_counts={"strict": 0, "data": 0, "physical": 0},
                source_class=source_class,
            )[0])

        calls: list[list[Path]] = []

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            calls.append(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            lane.run_inbox(self.root, self.campaign)
            lane.run_inbox(self.root, self.campaign)
            blocked = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(calls), 2)
        self.assertEqual(blocked["status"], "pivot_required")
        self.assertIn("hypothesis family no_gain budget exhausted", blocked["reason"])
        self.assertEqual(
            selector.normalize_source_class("owner-birth-v1"),
            selector.normalize_source_class("owner-birth-v2"),
        )

    def test_improved_frontier_resets_no_gain_budget(self) -> None:
        self._proposal("old-frontier", source_class="old-owner")
        calls: list[list[Path]] = []

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            calls.append(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            lane.run_inbox(self.root, self.campaign)
            old_body = dict(self.frontier)
            old_body.pop("frontier_sha256", None)
            old_body["generation"] = 1
            new_frontier = _seal(old_body, "frontier_sha256")
            self.frontier = new_frontier
            self.campaign["_selection_frontier"] = new_frontier
            self._proposal("new-frontier", source_class="new-owner")
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(calls), 2)
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(result["results"][0]["status"], "no_gain")

    def test_cross_function_selection_continues_when_first_frontier_is_exhausted(self) -> None:
        self.campaign["functions"] = ["focus", "other"]
        other_body: dict[str, object] = {
            "function": "other",
            "unit": "main/test",
            "source_sha256": self.frontier["source_sha256"],
            "toolchain_sha256": "b" * 64,
        }
        other_frontier = _seal(other_body, "frontier_sha256")
        self.campaign["_selection_frontiers"] = {
            "focus": self.frontier,
            "other": other_frontier,
        }
        self._proposal(
            "focus-next",
            residual=["strict:focus:row:99"],
            predicted=["strict:focus:row:99"],
            predicted_counts={"strict": 0, "data": 0, "physical": 0},
            source_class="focus-owner",
        )
        self._proposal(
            "other-next",
            function="other",
            frontier=other_frontier,
            residual=["strict:other:row:1"],
            predicted=["strict:other:row:1"],
            predicted_counts={"strict": 0, "data": 0, "physical": 0},
            source_class="other-owner",
        )
        for index in range(6):
            self._write_outcome(
                status="no_gain",
                source_class=f"closed-{index}",
                predicted=[f"strict:focus:row:{index + 10}"],
                candidate_sha=f"{index + 1:064x}",
            )
        # The six seeded rows are function-scoped to focus and do not overlap
        # the eligible focus proposal; they exhaust only that frontier.
        calls: list[list[Path]] = []

        def dispatch(
            root: Path, campaign: dict[str, object], paths: list[Path]
        ) -> list[dict[str, object]]:
            calls.append(paths)
            return [{"status": "no_gain", "authority_advanced": False}]

        with patch.object(owner_campaign, "run_loop", side_effect=dispatch):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(len(calls), 1)
        self.assertTrue(calls[0][0].name.startswith("other-next"))


if __name__ == "__main__":
    unittest.main()
