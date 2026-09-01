from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest.mock import patch

from tools import owner_campaign
from tools import owner_campaign_lane as lane
from tools import owner_campaign_reconstruction as reconstruction
from tools.tests.test_owner_campaign import HOOK as CAMPAIGN_HOOK


def _digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _seal(body: dict[str, object], field: str) -> dict[str, object]:
    value = dict(body)
    value[field] = owner_campaign._digest_json(body)
    return value


def _reconstruction_focus_report(
    frontier: dict[str, object],
    *,
    physical: bool = False,
    size_drift: bool = False,
    include_residual: bool = True,
    broad: bool = False,
    residual_indexes: tuple[int, ...] | None = None,
) -> dict[str, object]:
    """Build a small, complete focus report for packet-builder fixtures.

    The lane's compact focus CAS object intentionally remains in the older
    selector shape.  Reconstruction packets consume a separate synthetic
    ``focus_symbol_report`` so the test exercises the production packet
    builder/verifier rather than hand-authoring a partial packet.
    """

    def instruction_row(index: int, *, candidate: bool) -> dict[str, object]:
        register = "r4" if candidate else "r3"
        offset = "0x14" if candidate else "0x10"
        row: dict[str, object] = {
            "index": index,
            "instruction": {
                "address": hex(0x1000 + index * 4),
                "formatted": f"lwz {register},{offset}(r1)",
                "size": 4,
                "parts": [{"opcode": "lwz"}],
            },
        }
        kind = {20: "DIFF_INSERT", 40: "DIFF_DELETE", 60: "DIFF_REPLACE"}.get(index)
        if kind is not None:
            row["diff_kind"] = kind
        return row

    indexes = list(
        residual_indexes
        if residual_indexes is not None
        else ([1, 20, 40, 60] if broad else [1])
    )
    target_rows = [instruction_row(index, candidate=False) for index in indexes]
    candidate_rows = [instruction_row(index, candidate=True) for index in indexes]
    target_relocations: list[dict[str, object]] = []
    candidate_relocations: list[dict[str, object]] = []
    differences: list[dict[str, object]] = []
    if physical:
        differences = [{"offset": 4, "target": ["helper"], "candidate": ["other"]}]

    if not include_residual:
        target_rows = []
        candidate_rows = []
    target_size = max(4, len(target_rows) * 4)
    candidate_size = 5 if size_drift else target_size
    strict_row_ids = []
    if include_residual:
        for index in indexes:
            kind = {20: "DIFF_INSERT", 40: "DIFF_DELETE", 60: "DIFF_REPLACE"}.get(index)
            suffix = f"kind={kind}" if kind is not None else ""
            strict_row_ids.append(f"strict:focus:row:{index}:{suffix}")
    body: dict[str, object] = {
        "schema": "focus_symbol_report/v1",
        "owner": frontier["owner"],
        "unit": frontier["unit"],
        "function": frontier["function"],
        "source_path": frontier["source_relpath"],
        "base_commit": "0" * 40,
        "source_sha256": frontier["source_sha256"],
        "target_object_sha256": frontier["target_object_sha256"],
        "candidate_object_sha256": frontier["candidate_object_sha256"],
        "toolchain_sha256": frontier["toolchain_sha256"],
        "strict_row_ids": strict_row_ids,
        "data_row_ids": [],
        "channels": {
            "strict": {
                "metric": {
                    "target_size": target_size,
                    "candidate_size": candidate_size,
                    "diff_rows": len(target_rows),
                },
                "target": {
                    "instruction_count": len(target_rows),
                    "rows": target_rows,
                },
                "candidate": {
                    "instruction_count": len(candidate_rows),
                    "rows": candidate_rows,
                },
            },
            "data": {
                "metric": {
                    "target_size": target_size,
                    "candidate_size": candidate_size,
                    "diff_rows": 0,
                },
                "target": {"instruction_count": 1, "rows": []},
                "candidate": {"instruction_count": 1, "rows": []},
            },
        },
        "physical_relocations": {
            "status": "mismatch" if physical else "exact",
            "target": {
                "physical_relocation_count": 0,
                "physical_relocations": target_relocations,
            },
            "candidate": {
                "physical_relocation_count": 0,
                "physical_relocations": candidate_relocations,
            },
            "physical_relocation_differences": differences,
        },
    }
    body["artifact_sha256"] = reconstruction.canonical_sha256(body)
    return body


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

    def _candidate(
        self,
        name: str,
        *,
        function: str = "focus",
        created_at: str = "2026-08-31T00:00:00Z",
    ) -> Path:
        source = self.root / "build" / "candidates" / f"{name}.c"
        source.write_text(
            f"int {function}(void) {{ return 0; }} /* {name} */\n",
            encoding="utf-8",
        )
        span_sha = owner_campaign._digest_file(source)
        body: dict[str, object] = {
            "schema": owner_campaign.CANDIDATE_SCHEMA,
            "campaign_id": self.campaign["campaign_id"],
            "function": function,
            "base_frontier_sha256": "a" * 64,
            "base_source": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": owner_campaign._digest_file(source),
            },
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
            "rebase_depth": 0,
            "created_at": created_at,
        }
        descriptor = self.inbox / f"{name}.json"
        descriptor.write_text(
            json.dumps(_seal(body, "candidate_sha256")), encoding="utf-8"
        )
        return descriptor

    def _candidate_source(self, name: str, result: int) -> Path:
        source = self.root / "build" / "candidates" / f"{name}.c"
        source.write_text(
            "int before = 1;\n\n"
            f"int focus(void) {{ return {result}; }}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        return source

    def _enable_streaming_pipeline(self) -> None:
        """Mark a compact fixture as the loaded v2 pipeline contract."""

        self.campaign["_source"] = self.root / "src" / "owner.c"
        self.campaign["limits"] = {}

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

    def test_terminal_compaction_failure_is_retryable_and_not_reported_processed(self) -> None:
        descriptor = self._candidate("cleanup-failure")

        def failed_compaction(
            root: Path, campaign: dict[str, object], path: Path,
        ) -> list[str]:
            return [f"cleanup-error:{path}:injected"]

        with patch.object(
            owner_campaign,
            "run_loop",
            return_value=[{"status": "no_gain", "authority_advanced": False}],
        ), patch.object(lane, "_compact_terminal_input", side_effect=failed_compaction):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "infra_retry")
        self.assertEqual(len(result["cleanup_failures"]), 1)
        self.assertIn(descriptor.relative_to(self.root).as_posix(), result["preserved_infrastructure"])
        self.assertTrue(descriptor.exists())

    def test_terminal_compaction_retries_a_transient_unlink(self) -> None:
        descriptor = self._candidate("cleanup-retry")
        source = self.root / "build" / "candidates" / "cleanup-retry.c"
        original_unlink = Path.unlink
        failed = False

        def flaky_unlink(path: Path, *args: object, **kwargs: object) -> None:
            nonlocal failed
            if path == source and not failed:
                failed = True
                raise OSError("transient sharing violation")
            original_unlink(path, *args, **kwargs)

        with patch.object(Path, "unlink", autospec=True, side_effect=flaky_unlink):
            cleaned = lane._compact_terminal_input(
                self.root, self.campaign, descriptor
            )

        self.assertTrue(failed)
        self.assertFalse(descriptor.exists())
        self.assertFalse(source.exists())
        self.assertFalse(any(str(item).startswith("cleanup-error:") for item in cleaned))

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

    def test_v2_dispatches_one_winner_per_function_to_five_workers(self) -> None:
        self._enable_streaming_pipeline()
        functions = [f"focus{index}" for index in range(5)]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        descriptors = [
            self._candidate(f"cell-{function}", function=function)
            for function in functions
        ]
        selector_calls: list[tuple[str, int]] = []
        observed: list[Path] = []

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            selector_calls.append((function, len(paths)))
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "evidence_path": str(paths[0].with_suffix(".evidence.json")),
                    "function": function,
                },
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            observed.append(path)
            return {
                "status": "infra_retry",
                "function": json.loads(path.read_text(encoding="utf-8"))["function"],
                "authority_advanced": False,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance"),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(len(selector_calls), 5)
        self.assertEqual({function for function, _count in selector_calls}, set(functions))
        self.assertEqual([count for _function, count in selector_calls], [1] * 5)
        self.assertEqual(set(observed), set(descriptors))
        self.assertEqual(result["dispatched"], 5)
        self.assertEqual(result["status"], "infra_retry")
        self.assertIsNone(result["selection"])
        self.assertEqual(len(result["selections"]), 5)
        self.assertEqual(
            [item["selected"]["function"] for item in result["selections"]],
            functions,
        )

    def test_v2_same_function_still_dispatches_only_one_winner(self) -> None:
        self._enable_streaming_pipeline()
        self.campaign["base_commit"] = "base-commit"
        descriptors = [self._candidate(f"same-{index}") for index in range(3)]
        selector_calls: list[list[Path]] = []
        observed: list[Path] = []

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            selector_calls.append(paths)
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {"descriptor_path": str(paths[0]), "function": "focus"},
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            observed.append(path)
            return {"status": "infra_retry", "function": "focus"}

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance"),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(selector_calls, [descriptors])
        self.assertEqual(observed, [descriptors[0]])
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(result["selection"]["status"], lane.owner_campaign_selector.SELECTED)
        self.assertEqual(len(result["selections"]), 1)

    def test_v2_unknown_function_does_not_block_other_winner(self) -> None:
        self._enable_streaming_pipeline()
        functions = ["blocked", "ready"]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        blocked = self._candidate("blocked-cell", function="blocked")
        ready = self._candidate("ready-cell", function="ready")
        observed: list[Path] = []

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            if function == "blocked":
                return {
                    "status": lane.owner_campaign_selector.UNKNOWN,
                    "reason": "no current-bound winner",
                }
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "function": function,
                },
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            observed.append(path)
            return {"status": "infra_retry", "function": "ready"}

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance"),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(observed, [ready])
        self.assertEqual(result["status"], "infra_retry")
        self.assertEqual(result["dispatched"], 1)
        self.assertIn(blocked.relative_to(self.root).as_posix(), result["preserved_infrastructure"])
        self.assertEqual(
            [item["status"] for item in result["selections"]],
            [lane.owner_campaign_selector.UNKNOWN, lane.owner_campaign_selector.SELECTED],
        )

    def test_v2_terminal_outcomes_bind_to_their_function_selection(self) -> None:
        self._enable_streaming_pipeline()
        functions = [f"owner{index}" for index in range(5)]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        descriptors = [
            self._candidate(f"terminal-{function}", function=function)
            for function in functions
        ]
        outcome_calls: list[tuple[str, str]] = []

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            descriptor_path = paths[0]
            function = json.loads(descriptor_path.read_text(encoding="utf-8"))["function"]
            evidence_path = descriptor_path.with_suffix(".evidence.json")
            evidence_path.write_text("{}", encoding="utf-8")
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(descriptor_path),
                    "evidence_path": str(evidence_path),
                    "function": function,
                },
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            return {
                "status": "no_gain",
                "function": json.loads(path.read_text(encoding="utf-8"))["function"],
                "authority_advanced": False,
            }

        def record(
            root: Path,
            campaign: dict[str, object],
            selected: dict[str, object],
            result: dict[str, object],
        ) -> dict[str, object]:
            outcome_calls.append((str(selected["function"]), str(result["function"])))
            return {"function": selected["function"], "status": result["status"]}

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(
                lane.owner_campaign_selector,
                "append_selection_outcome",
                side_effect=record,
            ),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(outcome_calls, [(function, function) for function in functions])
        self.assertEqual(
            [item["function"] for item in result["recorded_outcomes"]],
            functions,
        )
        self.assertEqual(
            [item["function"] for item in result["results"]],
            functions,
        )
        self.assertEqual(result["dispatched"], 5)

    def test_v2_ready_function_dispatches_while_slow_selector_is_blocked(self) -> None:
        """Selection is a per-function pipeline, not a whole-batch barrier."""

        self._enable_streaming_pipeline()
        functions = ["slow", "ready"]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        descriptors = [
            self._candidate(f"parallel-{function}", function=function)
            for function in functions
        ]
        selector_calls: list[str] = []
        ready_measurement_started = threading.Event()

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            selector_calls.append(function)
            if function == "slow":
                self.assertTrue(
                    ready_measurement_started.wait(timeout=5),
                    "ready function remained behind the selector batch barrier",
                )
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "function": function,
                },
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            function = json.loads(path.read_text(encoding="utf-8"))["function"]
            if function == "ready":
                ready_measurement_started.set()
            return {"status": "infra_retry", "function": function}

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance") as maintenance,
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(set(selector_calls), set(functions))
        self.assertEqual(
            [item["selected"]["function"] for item in result["selections"]],
            functions,
        )
        self.assertEqual(
            [item["function"] for item in result["results"]],
            functions,
        )
        maintenance.assert_called_once()

    def test_v2_selector_exception_isolated_to_its_function_pipeline(self) -> None:
        self._enable_streaming_pipeline()
        self.campaign["base_commit"] = "base-commit"
        self._candidate("selector-error")
        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=RuntimeError("selector exploded"),
            ),
            patch.object(lane, "_dispatch_selected_candidate") as dispatch,
        ):
            result = lane.run_inbox(self.root, self.campaign)
        self.assertEqual(result["status"], "selection_unknown")
        self.assertIn("selector arbitration failed", result["reason"])
        dispatch.assert_not_called()

    def test_v2_selector_error_does_not_mask_ready_retained_outcome(self) -> None:
        self._enable_streaming_pipeline()
        functions = ["broken", "ready"]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        broken = self._candidate("broken-cell", function="broken")
        ready = self._candidate("ready-cell-error-peer", function="ready")
        ready_started = threading.Event()

        def select(root: Path, campaign: dict[str, object], paths: list[Path]):
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            if function == "broken":
                self.assertTrue(ready_started.wait(timeout=5))
                raise RuntimeError("selector exploded late")
            evidence = paths[0].with_suffix(".evidence.json")
            evidence.write_text("{}", encoding="utf-8")
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "evidence_path": str(evidence),
                    "function": function,
                },
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            ready_started.set()
            return {
                "status": "improved",
                "function": "ready",
                "authority_advanced": False,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance") as maintenance,
            patch.object(
                lane.owner_campaign_selector,
                "append_selection_outcome",
                return_value={"function": "ready", "status": "improved"},
            ),
        ):
            result = lane.run_inbox(self.root, self.campaign)

        self.assertEqual(result["status"], "processed")
        self.assertEqual(result["results"], [{
            "status": "improved",
            "function": "ready",
            "authority_advanced": False,
        }])
        self.assertTrue(broken.exists())
        self.assertFalse(ready.exists())
        self.assertIn("selector arbitration failed", result["selections"][0]["reason"])
        maintenance.assert_called_once()

    def test_supervisor_continues_after_selection_unknown_and_dispatches_later_function(self) -> None:
        """An UNKNOWN selector result is a search miss, not a supervisor terminal state."""

        self._enable_streaming_pipeline()
        self.campaign["functions"] = ["blocked", "ready"]
        self.campaign["base_commit"] = "base-commit"
        blocked = self._candidate("supervisor-blocked", function="blocked")
        ready = self._candidate("supervisor-ready", function="ready")
        discovery_calls = 0
        dispatched: list[str] = []

        def discover(
            root: Path,
            campaign: dict[str, object],
            *,
            limit: int,
        ) -> list[Path]:
            nonlocal discovery_calls
            discovery_calls += 1
            if discovery_calls == 1:
                return [blocked]
            if discovery_calls == 2:
                # The first streaming drain sees only the blocked selector.
                return []
            if discovery_calls == 3:
                # The next supervisor poll must still be live and find this
                # distinct function after the UNKNOWN result.
                return [ready]
            return []

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            if function == "blocked":
                return {
                    "status": lane.owner_campaign_selector.UNKNOWN,
                    "reason": "no current-bound winner",
                }
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {"descriptor_path": str(paths[0]), "function": function},
            }

        def dispatch(
            root: Path,
            campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            function = json.loads(path.read_text(encoding="utf-8"))["function"]
            dispatched.append(function)
            return {
                "status": "infra_retry",
                "function": function,
                "authority_advanced": False,
            }

        with (
            patch.object(lane, "discover_candidates", side_effect=discover),
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_dispatch_selected_candidate", side_effect=dispatch),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(
                lane.owner_campaign_selector,
                "append_selection_outcome",
                return_value={"function": "ready", "status": "no_gain"},
            ),
            patch.object(owner_campaign, "_check_cancelled"),
            patch.object(
                owner_campaign,
                "campaign_terminal_progress",
                return_value={"exact_count": 0, "total": 2, "closed": False},
            ),
        ):
            result = lane.run_supervisor(
                self.root,
                self.campaign,
                idle_timeout_seconds=0.05,
                watchdog_seconds=2.0,
                poll_interval_seconds=0.01,
            )

        self.assertEqual(dispatched, ["ready"])
        self.assertEqual(result["status"], "idle_timeout")
        self.assertEqual(result["dispatched"], 1)
        self.assertEqual(result["outcomes"], {"infra_retry": 1}, result)
        self.assertGreaterEqual(discovery_calls, 3)

    def test_supervisor_reuses_one_pre_discovered_batch(self) -> None:
        functions = ["focus0", "focus1"]
        self.campaign["functions"] = functions
        descriptors = [
            self._candidate(f"supervisor-{function}", function=function)
            for function in functions
        ]
        discovery_limits: list[int] = []
        inbox_batches: list[list[Path] | None] = []

        def discover(
            root: Path,
            campaign: dict[str, object],
            *,
            limit: int,
        ) -> list[Path]:
            discovery_limits.append(limit)
            return descriptors

        def inbox(
            root: Path,
            campaign: dict[str, object],
            *,
            max_candidates: int,
            _pre_discovered: list[Path] | None = None,
        ) -> dict[str, object]:
            inbox_batches.append(_pre_discovered)
            return {
                "status": "processed",
                "dispatched": 2,
                "results": [
                    {"status": "improved"},
                    {"status": "improved"},
                ],
            }

        with (
            patch.object(lane, "discover_candidates", side_effect=discover),
            patch.object(lane, "run_inbox", side_effect=inbox),
            patch.object(
                lane,
                "_campaign_terminal_state",
                side_effect=[(None, None), ("closed", "done")],
            ),
        ):
            result = lane.run_supervisor(
                self.root,
                self.campaign,
                clock=lambda: 0.0,
                sleeper=lambda _duration: self.fail("closed supervisor slept"),
            )

        self.assertEqual(result["status"], "closed")
        self.assertEqual(result["batches"], 1)
        self.assertEqual(result["dispatched"], 2)
        self.assertEqual(discovery_limits, [10])
        self.assertEqual(inbox_batches, [descriptors])

    def test_supervisor_uses_constant_cost_terminal_progress(self) -> None:
        campaign = dict(self.campaign)
        campaign["_source"] = self.root / "src" / "owner.c"
        campaign["limits"] = {"command_timeout_seconds": 1}
        with (
            patch.object(
                owner_campaign,
                "campaign_terminal_progress",
                return_value={"exact_count": 1, "total": 1, "closed": True},
            ) as progress,
            patch.object(
                owner_campaign,
                "campaign_status",
                side_effect=AssertionError("full status must not be polled"),
            ),
        ):
            result = lane.run_supervisor(
                self.root,
                campaign,
                clock=lambda: 0.0,
                sleeper=lambda _duration: self.fail("closed supervisor slept"),
            )

        self.assertEqual(result["status"], "closed")
        progress.assert_called_once_with(self.root, campaign)

    def test_streaming_refills_sixth_slot_before_long_fifth_finishes(self) -> None:
        """A freed slot is refilled without waiting for the slowest slot."""

        self._enable_streaming_pipeline()
        functions = [f"owner{index}" for index in range(6)]
        self.campaign["functions"] = functions
        self.campaign["base_commit"] = "base-commit"
        descriptors = [
            self._candidate(f"stream-{function}", function=function)
            for function in functions
        ]
        short_finished = threading.Event()
        long_started = threading.Event()
        long_finished = threading.Event()
        sixth_started = threading.Event()
        release_long = threading.Event()
        starts: list[tuple[str, int]] = []

        def pipeline(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
            *,
            worker: int,
        ) -> dict[str, object]:
            path = paths[0]
            function = json.loads(path.read_text(encoding="utf-8"))["function"]
            starts.append((function, worker))
            if function == "owner0":
                short_finished.set()
            elif function == "owner4":
                long_started.set()
                self.assertTrue(release_long.wait(timeout=5))
                long_finished.set()
            elif function == "owner5":
                # The fifth original slot is deliberately still blocked when
                # the sixth function is admitted.
                self.assertTrue(short_finished.is_set())
                self.assertTrue(long_started.is_set())
                self.assertFalse(long_finished.is_set())
                sixth_started.set()
            return {
                "schema": lane.LANE_RESULT_SCHEMA,
                "status": "infra_retry",
                "campaign_id": self.campaign["campaign_id"],
                "discovered": 1,
                "dispatched": 1,
                "results": [{
                    "status": "infra_retry",
                    "function": function,
                    "authority_advanced": False,
                }],
                "cleaned": [],
                "preserved_infrastructure": [],
                "selection": None,
                "selections": [],
                "recorded_outcomes": [],
                "authority_advanced": False,
            }

        def discover(
            root: Path,
            campaign: dict[str, object],
            *,
            limit: int,
        ) -> list[Path]:
            return descriptors

        with (
            patch.object(lane, "_streaming_pipeline", side_effect=pipeline),
            patch.object(lane, "discover_candidates", side_effect=discover),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(lane.owner_campaign, "_check_cancelled"),
        ):
            # ``owner4`` cannot release itself; the test releases it only after
            # observing that owner5 was admitted into the newly free slot.
            result_holder: list[dict[str, object]] = []
            error_holder: list[BaseException] = []

            def drain() -> None:
                try:
                    result_holder.append(
                        lane._run_streaming_inbox(
                            self.root,
                            self.campaign,
                            max_candidates=5,
                            initial_descriptors=descriptors[:5],
                            poll_interval=0.01,
                            clock=time.monotonic,
                            watchdog_deadline=time.monotonic() + 5,
                        )
                    )
                except BaseException as exc:
                    error_holder.append(exc)

            thread = threading.Thread(target=drain)
            thread.start()
            self.assertTrue(sixth_started.wait(timeout=5), starts)
            release_long.set()
            thread.join(timeout=5)

        self.assertFalse(thread.is_alive())
        self.assertFalse(error_holder, error_holder)
        self.assertEqual(result_holder[0]["dispatched"], 6)
        self.assertEqual(
            {function for function, _worker in starts},
            set(functions),
        )
        self.assertEqual(len({worker for _function, worker in starts}), 5)

    def test_streaming_fills_five_slots_across_three_functions(self) -> None:
        """Ranked siblings may occupy free slots and race through one CAS."""

        self._enable_streaming_pipeline()
        functions = ["owner0", "owner0", "owner0", "owner1", "owner2"]
        self.campaign["functions"] = ["owner0", "owner1", "owner2"]
        self.campaign["base_commit"] = "base-commit"
        descriptors = [
            self._candidate(f"duplicate-{index}", function=function)
            for index, function in enumerate(functions)
        ]
        selector_calls: list[list[Path]] = []
        starts: list[tuple[Path, int]] = []
        start_lock = threading.Lock()
        all_started = threading.Event()
        release = threading.Event()
        cas_lock = threading.Lock()
        same_function_winner = False

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            selector_calls.append(list(paths))
            function = json.loads(paths[0].read_text(encoding="utf-8"))["function"]
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "function": function,
                },
            }

        def pipeline(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
            *,
            worker: int,
            preselection: dict[str, object],
        ) -> dict[str, object]:
            nonlocal same_function_winner
            path = paths[0]
            function = json.loads(path.read_text(encoding="utf-8"))["function"]
            self.assertEqual(
                preselection["selected"]["descriptor_path"], str(path)
            )
            with start_lock:
                starts.append((path, worker))
                if len(starts) == 5:
                    all_started.set()
            self.assertTrue(release.wait(timeout=5))
            status = "infra_retry"
            if function == "owner0":
                with cas_lock:
                    if not same_function_winner:
                        same_function_winner = True
                        status = "improved"
                    else:
                        status = "stale"
            return {
                "schema": lane.LANE_RESULT_SCHEMA,
                "status": "processed",
                "campaign_id": self.campaign["campaign_id"],
                "discovered": 1,
                "dispatched": 1,
                "results": [{
                    "status": status,
                    "function": function,
                    "authority_advanced": False,
                }],
                "cleaned": [],
                "preserved_infrastructure": [],
                "selection": None,
                "selections": [],
                "recorded_outcomes": [],
                "authority_advanced": False,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_streaming_pipeline", side_effect=pipeline),
            patch.object(lane, "discover_candidates", return_value=descriptors),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(lane.owner_campaign, "_check_cancelled"),
        ):
            result_holder: list[dict[str, object]] = []
            error_holder: list[BaseException] = []

            def drain() -> None:
                try:
                    result_holder.append(
                        lane._run_streaming_inbox(
                            self.root,
                            self.campaign,
                            max_candidates=5,
                            initial_descriptors=descriptors,
                            poll_interval=0.01,
                            clock=time.monotonic,
                            watchdog_deadline=time.monotonic() + 10,
                        )
                    )
                except BaseException as exc:
                    error_holder.append(exc)

            thread = threading.Thread(target=drain)
            thread.start()
            try:
                self.assertTrue(all_started.wait(timeout=5), starts)
                self.assertEqual(len(starts), 5)
            finally:
                release.set()
            thread.join(timeout=5)

        self.assertFalse(thread.is_alive())
        self.assertFalse(error_holder, error_holder)
        self.assertEqual(len(selector_calls), 5)
        self.assertEqual([len(paths) for paths in selector_calls], [3, 2, 1, 1, 1])
        self.assertEqual(
            {json.loads(path.read_text(encoding="utf-8"))["function"] for path, _ in starts},
            {"owner0", "owner1", "owner2"},
        )
        self.assertEqual(len({worker for _path, worker in starts}), 5)
        owner0_results = [
            item for item in result_holder[0]["results"]
            if item.get("function") == "owner0"
        ]
        self.assertEqual(
            sorted(item["status"] for item in owner0_results),
            ["improved", "stale", "stale"],
        )
        self.assertEqual(result_holder[0]["dispatched"], 5)

    def test_streaming_gives_selector_the_complete_same_function_group(self) -> None:
        """A lower-ranked first entry cannot bypass its ranked sibling."""

        self._enable_streaming_pipeline()
        self.campaign["base_commit"] = "base-commit"
        lower = self._candidate("ranked-lower")
        higher = self._candidate("ranked-higher")
        descriptors = [lower, higher]
        selector_inputs: list[list[Path]] = []

        def inbox(
            root: Path,
            campaign: dict[str, object],
            *,
            max_candidates: int,
            _pre_discovered: list[Path],
            _defer_maintenance: bool,
            _worker: int,
        ) -> dict[str, object]:
            selector_inputs.append(list(_pre_discovered))
            # This stub stands in for the real selector's deterministic rank;
            # the assertion is that both siblings reach it in one group.
            selected = _pre_discovered[-1]
            return {
                "schema": lane.LANE_RESULT_SCHEMA,
                "status": "processed",
                "campaign_id": campaign["campaign_id"],
                "discovered": len(_pre_discovered),
                "dispatched": 1,
                "results": [{
                    "status": "no_gain",
                    "function": "focus",
                    "candidate": str(selected),
                    "authority_advanced": False,
                }],
                "cleaned": [],
                "preserved_infrastructure": [],
                "selection": None,
                "selections": [],
                "recorded_outcomes": [],
                "authority_advanced": False,
            }

        with (
            patch.object(lane, "run_inbox", side_effect=inbox),
            patch.object(lane, "discover_candidates", return_value=descriptors),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(lane.owner_campaign, "_check_cancelled"),
        ):
            result = lane._run_streaming_inbox(
                self.root,
                self.campaign,
                max_candidates=5,
                initial_descriptors=descriptors,
                poll_interval=0.01,
                clock=time.monotonic,
                watchdog_deadline=time.monotonic() + 5,
            )

        self.assertEqual(selector_inputs, [descriptors])
        self.assertEqual(result["results"][0]["candidate"], str(higher))
        self.assertEqual(result["dispatched"], 1)

    def test_streaming_retries_source_advance_without_consuming_descriptor(self) -> None:
        """A snapshot race is retried once while the drain remains live."""

        self._enable_streaming_pipeline()
        self.campaign["base_commit"] = "base-commit"
        descriptor = self._candidate("source-race")
        selector_calls = 0
        pipeline_calls = 0

        def select(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            nonlocal selector_calls
            selector_calls += 1
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": {
                    "descriptor_path": str(paths[0]),
                    "function": "focus",
                },
            }

        def pipeline(
            root: Path,
            campaign: dict[str, object],
            paths: list[Path],
            *,
            worker: int,
            preselection: dict[str, object],
        ) -> dict[str, object]:
            nonlocal pipeline_calls
            pipeline_calls += 1
            status = "infra_retry" if pipeline_calls == 1 else "no_gain"
            reason = (
                "frontier snapshot became stale before publication"
                if pipeline_calls == 1
                else "second attempt completed"
            )
            return {
                "schema": lane.LANE_RESULT_SCHEMA,
                "status": "infra_retry" if pipeline_calls == 1 else "processed",
                "campaign_id": self.campaign["campaign_id"],
                "discovered": 1,
                "dispatched": 1,
                "results": [{
                    "status": status,
                    "reason": reason,
                    "function": "focus",
                    "authority_advanced": False,
                }],
                "cleaned": [],
                "preserved_infrastructure": [],
                "selection": None,
                "selections": [],
                "recorded_outcomes": [],
                "authority_advanced": False,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select,
            ),
            patch.object(lane, "_streaming_pipeline", side_effect=pipeline),
            patch.object(lane, "discover_candidates", return_value=[descriptor]),
            patch.object(lane, "_post_pipeline_maintenance"),
            patch.object(lane.owner_campaign, "_check_cancelled"),
        ):
            result = lane._run_streaming_inbox(
                self.root,
                self.campaign,
                max_candidates=1,
                initial_descriptors=[descriptor],
                poll_interval=0.01,
                clock=time.monotonic,
                watchdog_deadline=time.monotonic() + 5,
            )

        self.assertEqual(selector_calls, 2)
        self.assertEqual(pipeline_calls, 2)
        self.assertEqual(result["dispatched"], 2)
        self.assertEqual(
            [item["status"] for item in result["results"]],
            ["infra_retry", "no_gain"],
        )

    def test_driver_has_no_legacy_control_dependency(self) -> None:
        source = Path(lane.__file__).read_text(encoding="utf-8").lower()
        for forbidden in ("stop", "permit", "hmac"):
            self.assertNotIn(forbidden, source)

    def _proposal_fixture(
        self,
        *,
        residual_indexes: tuple[int, ...] = (1,),
    ) -> tuple[dict[str, object], Path, Path]:
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
        def focus_row_id(index: int) -> str:
            kind = {20: "DIFF_INSERT", 40: "DIFF_DELETE", 60: "DIFF_REPLACE"}.get(index)
            suffix = f"kind={kind}" if kind is not None else ""
            return f"strict:focus:row:{index}:{suffix}"

        strict_row_ids = [focus_row_id(index) for index in residual_indexes]
        focus_body: dict[str, object] = {
            "schema": "owner_campaign_focus_evidence/v1",
            "owner": campaign["owner"],
            "function": "focus",
            "unit": campaign["unit"],
            "source_path": campaign["source_relpath"],
            "base_commit": "0" * 40,
            "source_sha256": source_sha256,
            "target_object_sha256": "d" * 64,
            "strict_rows": strict_row_ids,
            "data_rows": [],
            "physical_differences": [],
            "strict_row_ids": strict_row_ids,
            "strict_row_ids_sha256": owner_campaign._digest_json(strict_row_ids),
            "data_row_ids": [],
            "data_row_ids_sha256": owner_campaign._digest_json([]),
            "physical_difference_ids": [],
            "physical_difference_ids_sha256": owner_campaign._digest_json([]),
            "physical_target_identity_sha256": "e" * 64,
            "physical_candidate_identity_sha256": "f" * 64,
            "strict_row_count": len(strict_row_ids),
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

    def _attach_reconstruction(
        self,
        campaign: dict[str, object],
        *,
        status: str = "READY",
        exact_terminal_possible: bool = True,
        next_action: str | None = None,
        residual_indexes: tuple[int, ...] | None = None,
    ) -> dict[str, object]:
        frontier_path = (
            owner_campaign._function_root(self.root, campaign, "focus")
            / "latest-frontier.json"
        )
        frontier = json.loads(frontier_path.read_text(encoding="utf-8"))
        # Every fixture packet starts from the production builder.  A broad
        # report naturally produces UNKNOWN/DECOMPOSE; a physical-only report
        # naturally produces UNKNOWN/PIVOT.
        report = _reconstruction_focus_report(
            frontier,
            physical=(
                (status == "UNKNOWN" and next_action != "DECOMPOSE")
                or (status == "READY" and not exact_terminal_possible)
            ),
            broad=status == "UNKNOWN" and next_action == "DECOMPOSE",
            include_residual=not (status == "UNKNOWN" and next_action != "DECOMPOSE"),
            residual_indexes=residual_indexes,
        )
        binding = {
            "owner": frontier["owner"],
            "unit": frontier["unit"],
            "function": frontier["function"],
            "source_path": frontier["source_relpath"],
            "source_sha256": frontier["source_sha256"],
            "base_commit": "0" * 40,
            "target_object_sha256": frontier["target_object_sha256"],
            "candidate_object_sha256": frontier["candidate_object_sha256"],
            "toolchain_sha256": frontier["toolchain_sha256"],
            "frontier_source_sha256": frontier["source_sha256"],
        }
        packet = reconstruction.build_packet(
            report,
            binding,
            {"function": frontier["function"], "start_line": 3, "end_line": 5},
        )
        packet_sha = packet["packet_sha256"]
        packet_path = (
            owner_campaign._state_root(self.root) / "proof-cas" / "reconstruction"
            / packet_sha[:2] / f"{packet_sha}.json"
        )
        packet_path.parent.mkdir(parents=True, exist_ok=True)
        packet_path.write_text(
            json.dumps(packet, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )
        frontier_body = dict(frontier)
        frontier_body["reconstruction_evidence_sha256"] = packet_sha
        frontier_body["reconstruction_status"] = status
        frontier_body.pop("frontier_sha256", None)
        frontier = {
            **frontier_body,
            "frontier_sha256": owner_campaign._digest_json(frontier_body),
        }
        frontier_path.write_text(
            json.dumps(frontier, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )
        return packet

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
        base_path = self.root / descriptor["base_source"]["path"]
        self.assertEqual(base_path, descriptor_path.parent / "base.c")
        self.assertTrue(base_path.is_file())
        self.assertEqual(
            descriptor["base_source"]["sha256"],
            owner_campaign._digest_file(base_path),
        )
        self.assertNotEqual(base_path, _base)
        self.assertEqual(descriptor["rebase_depth"], 0)
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

    def test_reconstruct_frontier_reads_verified_ready_packet_without_compile(self) -> None:
        campaign, _base, _candidate = self._proposal_fixture()
        packet = self._attach_reconstruction(campaign)
        frontier = json.loads(
            (
                owner_campaign._function_root(self.root, campaign, "focus")
                / "latest-frontier.json"
            ).read_text(encoding="utf-8")
        )
        with patch.object(
            owner_campaign,
            "snapshot_frontier",
            side_effect=AssertionError("reconstruct must use supplied snapshotter"),
        ):
            result = lane.reconstruct_frontier(
                self.root,
                campaign,
                "focus",
                snapshotter=lambda _root, _campaign, _function: frontier,
            )
        self.assertEqual(result["schema"], lane.RECONSTRUCTION_RESULT_SCHEMA)
        self.assertEqual(result["status"], "READY")
        self.assertEqual(result["packet_sha256"], packet["packet_sha256"])
        self.assertTrue(result["ownership_complete"])
        self.assertEqual(result["next_action"], "CRACK")
        self.assertFalse(result["authority_advanced"])

    def test_retained_frontier_race_fails_closed_before_read_only_triage(self) -> None:
        campaign, _base, _candidate = self._proposal_fixture()
        frontier = json.loads(
            (
                owner_campaign._function_root(self.root, campaign, "focus")
                / "latest-frontier.json"
            ).read_text(encoding="utf-8")
        )
        campaign.update(
            {
                "_retained_frontier": frontier,
                "_retained_frontier_sha256": frontier["frontier_sha256"],
                "_retained_frontier_function": "focus",
                "_retained_frontier_read_only": True,
            }
        )
        changed = dict(frontier)
        changed["frontier_sha256"] = "f" * 64
        with patch.object(
            owner_campaign, "_read_latest_frontier", return_value=changed
        ):
            with self.assertRaisesRegex(
                owner_campaign.CampaignError,
                "retained frontier changed during read-only triage",
            ):
                lane._frontier_for_proposal(self.root, campaign, "focus")

    def test_propose_remains_strict_when_retained_frontier_source_is_advanced(self) -> None:
        campaign, campaign_source, candidate_source = self._proposal_fixture()
        frontier = json.loads(
            (
                owner_campaign._function_root(self.root, campaign, "focus")
                / "latest-frontier.json"
            ).read_text(encoding="utf-8")
        )
        campaign.update(
            {
                "_retained_frontier": frontier,
                "_retained_frontier_sha256": frontier["frontier_sha256"],
                "_retained_frontier_function": "focus",
                "_retained_frontier_read_only": True,
            }
        )
        campaign_source.write_text(
            "int before = 9;\n\n"
            "int focus(void) { return 9; } /* ADVANCED */\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; } /* CANDIDATE */\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with patch.object(
            owner_campaign, "_read_latest_frontier", return_value=frontier
        ):
            with self.assertRaisesRegex(
                owner_campaign.CampaignError,
                "current source has drifted from frontier",
            ):
                lane.propose_candidate(
                    self.root, campaign, "focus", candidate_source, "advanced-source"
                )
        self.assertEqual(list(lane.inbox_path(self.root, campaign).rglob("*")), [])

    def test_packet_ready_exact_proposal_carries_reconstruction_reference(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        self._attach_reconstruction(campaign, exact_terminal_possible=True)
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        result = lane.propose_candidate(
            self.root, campaign, "focus", candidate_source, "reconstructed-return"
        )
        sidecar = json.loads(
            (self.root / result["candidate_selection"]).read_text(encoding="utf-8")
        )
        self.assertEqual(sidecar["reconstruction"]["status"], "READY")
        self.assertEqual(sidecar["reconstruction"]["exact_terminal_possible"], True)
        self.assertEqual(sidecar["reconstruction"]["causal_cluster_id"], "cluster-000")
        self.assertEqual(sidecar["ownership_complete"], True)

    def test_packet_ready_exact_proposal_can_span_causal_clusters(self) -> None:
        residual_indexes = (1, 20)
        campaign, _base, candidate_source = self._proposal_fixture(
            residual_indexes=residual_indexes
        )
        self._attach_reconstruction(
            campaign,
            exact_terminal_possible=True,
            residual_indexes=residual_indexes,
        )
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
            "reconstructed-multicluster-return",
            expected_terminal="exact",
        )
        sidecar = json.loads(
            (self.root / result["candidate_selection"]).read_text(encoding="utf-8")
        )
        self.assertEqual(sidecar["predicted_rows"], sidecar["residual_rows"])
        self.assertEqual(
            sidecar["reconstruction"]["causal_cluster_count"],
            2,
        )
        self.assertEqual(sidecar["reconstruction"]["causal_cluster_id"], "cluster-000")

    def test_ready_packet_without_exact_support_allows_only_improved(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        self._attach_reconstruction(campaign, exact_terminal_possible=False)
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "exact terminal"):
            lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "unsupported-exact"
            )
        result = lane.propose_candidate(
            self.root,
            campaign,
            "focus",
            candidate_source,
            "improved-reconstruction",
            expected_terminal="improved",
            predicted_rows=["strict:focus:row:1:"],
            predicted_remaining_counts={"strict": 0, "data": 0, "physical": 0},
        )
        self.assertEqual(result["status"], "queued")

    def test_unknown_packet_requires_pivot_when_no_bounded_decomposition(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        self._attach_reconstruction(campaign, status="UNKNOWN")
        frontier = json.loads(
            (
                owner_campaign._function_root(self.root, campaign, "focus")
                / "latest-frontier.json"
            ).read_text(encoding="utf-8")
        )
        result = lane.reconstruct_frontier(
            self.root,
            campaign,
            "focus",
            snapshotter=lambda _root, _campaign, _function: frontier,
        )
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["next_action"], "PIVOT")
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "pivot"):
            lane.propose_candidate(
                self.root,
                campaign,
                "focus",
                candidate_source,
                "unknown-reconstruction",
                expected_terminal="improved",
                predicted_rows=["strict:focus:row:1:"],
                predicted_remaining_counts={"strict": 0, "data": 0, "physical": 0},
            )

    def test_unknown_decompose_packet_accepts_one_closed_improved_region(self) -> None:
        campaign, _base, candidate_source = self._proposal_fixture()
        self._attach_reconstruction(
            campaign,
            status="UNKNOWN",
            next_action="DECOMPOSE",
        )
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
            "bounded-reconstruction",
            expected_terminal="improved",
            predicted_rows=["strict:focus:row:1:"],
            predicted_remaining_counts={"strict": 0, "data": 0, "physical": 0},
        )
        sidecar = json.loads(
            (self.root / result["candidate_selection"]).read_text(encoding="utf-8")
        )
        self.assertEqual(sidecar["reconstruction"]["next_action"], "DECOMPOSE")
        self.assertEqual(
            sidecar["reconstruction"]["bounded_region"]["cluster_id"],
            "cluster-000",
        )
        self.assertFalse(sidecar["ownership_complete"])
        self.assertEqual(
            sidecar["ownership_scope"], "bounded_decomposition_region"
        )
        frontier = lane._frontier_for_proposal(
            self.root, campaign, "focus"
        )
        selection = lane.owner_campaign_selector._validate_proposal(
            self.root,
            campaign,
            self.root / result["candidate_descriptor"],
            frontier,
        )
        self.assertEqual(selection["expected_terminal"], "improved")

    def test_reconstruction_tamper_and_stale_identity_fail_closed(self) -> None:
        campaign, _base, _candidate = self._proposal_fixture()
        packet = self._attach_reconstruction(campaign)
        packet_path = (
            owner_campaign._state_root(self.root) / "proof-cas" / "reconstruction"
            / packet["packet_sha256"][:2] / f"{packet['packet_sha256']}.json"
        )
        tampered = dict(packet)
        tampered["function"] = "other"
        tampered = reconstruction.seal(tampered)
        packet_path.write_text(
            json.dumps(tampered, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )
        frontier_path = (
            owner_campaign._function_root(self.root, campaign, "focus")
            / "latest-frontier.json"
        )
        frontier = json.loads(frontier_path.read_text(encoding="utf-8"))
        with self.assertRaisesRegex(owner_campaign.CampaignError, r"packet .*drift"):
            lane.reconstruct_frontier(
                self.root,
                campaign,
                "focus",
                snapshotter=lambda _root, _campaign, _function: frontier,
            )

    def test_reconstruction_predicted_rows_cannot_cross_clusters(self) -> None:
        reconstruction_view = {
            "causal_clusters": [
                {"cluster_id": "a", "strict_row_ids": ["strict:row:1"]},
                {"cluster_id": "b", "strict_row_ids": ["strict:row:2"]},
            ]
        }
        with self.assertRaisesRegex(owner_campaign.CampaignError, "cross causal"):
            lane._reconstruction_cluster_for_rows(
                reconstruction_view,
                ["strict:row:1", "strict:row:2"],
            )

    def test_exact_reconstruction_binds_complete_multicluster_residual(self) -> None:
        reconstruction_view = {
            "causal_clusters": [
                {
                    "cluster_id": "a",
                    "strict_row_ids": ["strict:row:1"],
                    "data_row_ids": ["data:row:1"],
                },
                {
                    "cluster_id": "b",
                    "strict_row_ids": ["strict:row:2"],
                    "data_row_ids": ["data:row:2"],
                },
            ]
        }
        selected = lane._reconstruction_cluster_for_rows(
            reconstruction_view,
            [
                "strict:row:1",
                "data:row:1",
                "strict:row:2",
                "data:row:2",
            ],
            allow_multiple=True,
        )
        self.assertEqual(selected["cluster_ids"], ["a", "b"])
        self.assertEqual(
            set(selected["row_ids"]),
            {
                "strict:row:1",
                "data:row:1",
                "strict:row:2",
                "data:row:2",
            },
        )

    def test_exact_reconstruction_rejects_incomplete_multicluster_residual(self) -> None:
        reconstruction_view = {
            "causal_clusters": [
                {"cluster_id": "a", "strict_row_ids": ["strict:row:1"]},
                {"cluster_id": "b", "strict_row_ids": ["strict:row:2"]},
            ]
        }
        with self.assertRaisesRegex(owner_campaign.CampaignError, "exact predicted rows"):
            lane._reconstruction_cluster_for_rows(
                reconstruction_view,
                ["strict:row:1"],
                allow_multiple=True,
            )

    def test_mirrored_clusters_are_one_atomic_prediction_group(self) -> None:
        reconstruction_view = {
            "causal_clusters": [
                {
                    "cluster_id": "a",
                    "mirror_group": "mirrored-0",
                    "strict_row_ids": ["strict:row:1"],
                },
                {
                    "cluster_id": "b",
                    "mirror_group": "mirrored-0",
                    "strict_row_ids": ["strict:row:2"],
                },
            ]
        }
        selected = lane._reconstruction_cluster_for_rows(
            reconstruction_view,
            ["strict:row:1", "strict:row:2"],
        )
        self.assertEqual(selected["mirror_group"], "mirrored-0")
        self.assertEqual(selected["cluster_ids"], ["a", "b"])
        with self.assertRaisesRegex(owner_campaign.CampaignError, "mirrored"):
            lane._reconstruction_cluster_for_rows(
                reconstruction_view,
                ["strict:row:1"],
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
                predicted_rows=["strict:focus:row:1:"],
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
            predicted_rows=["strict:focus:row:1:"],
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
                predicted_rows=["strict:focus:row:1:"],
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

    def _v2_rebase_fixture(
        self, *, rebase_depth: int = 0
    ) -> tuple[dict[str, object], Path, Path, Path, dict[str, object]]:
        """Create one valid v2 proposal and return its selector binding.

        The source/frontier mutation is deliberately performed later, after
        selector arbitration, by the tests below.  That models the real race
        where another function is retained while a candidate is compiling.
        """

        campaign, campaign_source, candidate_source = self._proposal_fixture()
        campaign["base_commit"] = "0" * 40
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        queued = lane.propose_candidate(
            self.root,
            campaign,
            "focus",
            candidate_source,
            "rebase-winning-cell",
            rebase_depth=rebase_depth,
        )
        descriptor_path = self.root / queued["candidate_descriptor"]
        descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
        selection_path = descriptor_path.parent / "candidate.selection.json"
        selection = json.loads(selection_path.read_text(encoding="utf-8"))
        selected = {
            "descriptor_path": str(descriptor_path),
            "descriptor_relative": descriptor_path.relative_to(self.root).as_posix(),
            "descriptor_sha256": owner_campaign._digest_file(descriptor_path),
            "source_path": str(descriptor_path.parent / "candidate.c"),
            "source_relative": descriptor["candidate_source"]["path"],
            "candidate_sha256": descriptor["candidate_source"]["sha256"],
            "base_source_path": str(descriptor_path.parent / "base.c"),
            "base_source_relative": descriptor["base_source"]["path"],
            "base_source_sha256": descriptor["base_source"]["sha256"],
            "rebase_depth": descriptor["rebase_depth"],
            "evidence_path": str(selection_path),
            "evidence_relative": selection_path.relative_to(self.root).as_posix(),
            "evidence_sha256": selection["evidence_sha256"],
            "frontier_sha256": descriptor["base_frontier_sha256"],
            "function": descriptor["function"],
            "unit": campaign["unit"],
            "source_class": descriptor["hypothesis_family"],
            "source_class_normalized": descriptor["hypothesis_family"],
            "status": "RANKED_SOURCE_CLASS",
            "rank": 1,
            "expected_terminal": selection["expected_terminal"],
            "residual_rows": selection["residual_rows"],
            "predicted_rows": selection["predicted_rows"],
            "predicted_remaining_counts": selection["predicted_remaining_counts"],
            "protected_sibling_digest": selection["protected_sibling_digest"],
            "focus_artifact_sha256": selection["focus_artifact"]["sha256"],
            "physical_artifact_sha256": selection["physical_artifact"]["sha256"],
            "predicted_row_group_sha256": owner_campaign._digest_json(
                selection["predicted_rows"]
            ),
            "selection_key_sha256": "a" * 64,
            "candidate_identity_sha256": "b" * 64,
        }
        return campaign, campaign_source, candidate_source, descriptor_path, selected

    def _refresh_v2_source_binding(
        self,
        campaign: dict[str, object],
        campaign_source: Path,
        *,
        before: int,
        focus_result: int,
    ) -> None:
        """Advance only the live source/frontier while preserving focus rows."""

        campaign_source.write_text(
            f"int before = {before};\n\n"
            "int focus(void) {\n"
            f"    return {focus_result};\n"
            "}\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        source_sha = owner_campaign._digest_file(campaign_source)
        frontier_path = (
            owner_campaign._function_root(self.root, campaign, "focus")
            / "latest-frontier.json"
        )
        frontier = json.loads(frontier_path.read_text(encoding="utf-8"))
        old_focus_digest = frontier["focus_evidence_sha256"]
        old_focus_path = (
            owner_campaign._state_root(self.root)
            / "proof-cas"
            / "focus"
            / old_focus_digest[:2]
            / f"{old_focus_digest}.json"
        )
        focus = json.loads(old_focus_path.read_text(encoding="utf-8"))
        focus_body = dict(focus)
        focus_body.pop("focus_evidence_sha256", None)
        focus_body["source_sha256"] = source_sha
        new_focus_digest = owner_campaign._digest_json(focus_body)
        focus = {
            **focus_body,
            "focus_evidence_sha256": new_focus_digest,
        }
        new_focus_path = (
            owner_campaign._state_root(self.root)
            / "proof-cas"
            / "focus"
            / new_focus_digest[:2]
            / f"{new_focus_digest}.json"
        )
        new_focus_path.parent.mkdir(parents=True, exist_ok=True)
        new_focus_path.write_text(
            json.dumps(focus, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )
        frontier_body = dict(frontier)
        frontier_body.pop("frontier_sha256", None)
        frontier_body["source_sha256"] = source_sha
        frontier_body["focus_evidence_sha256"] = new_focus_digest
        frontier = {
            **frontier_body,
            "frontier_sha256": owner_campaign._digest_json(frontier_body),
        }
        frontier_path.write_text(
            json.dumps(frontier, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )

    def _stale_result(
        self, descriptor_path: Path, *, function: str = "focus"
    ) -> dict[str, object]:
        descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
        return {
            "schema": "owner_campaign_result/v1",
            "status": "stale_rebase",
            "function": function,
            "authority_advanced": False,
            "rebase_input": {
                "descriptor_path": descriptor_path.relative_to(self.root).as_posix(),
                "descriptor_sha256": owner_campaign._digest_file(descriptor_path),
                "candidate_source_path": descriptor["candidate_source"]["path"],
                "candidate_source_sha256": descriptor["candidate_source"]["sha256"],
                "base_source_path": descriptor["base_source"]["path"],
                "base_source_sha256": descriptor["base_source"]["sha256"],
                "rebase_depth": descriptor["rebase_depth"],
                "function_span": descriptor["function_span"],
            },
        }

    def _find_rebase_tombstones(self) -> list[dict[str, object]]:
        state_root = owner_campaign._state_root(self.root)
        found: list[dict[str, object]] = []
        if not state_root.is_dir():
            return found
        for path in state_root.rglob("*"):
            if not path.is_file():
                continue
            try:
                value = json.loads(path.read_text(encoding="utf-8"))
            except (OSError, UnicodeError, json.JSONDecodeError):
                continue
            if isinstance(value, dict) and value.get("schema") == lane.REBASE_TOMBSTONE_SCHEMA:
                found.append(value)
        return found

    def _assert_sealed_tombstone(self, tombstone: dict[str, object], *, status: str) -> None:
        self.assertEqual(set(tombstone), lane.REBASE_TOMBSTONE_FIELDS)
        body = dict(tombstone)
        digest = body.pop("tombstone_sha256")
        self.assertEqual(digest, owner_campaign._digest_json(body))
        self.assertEqual(tombstone["status"], status)
        self.assertEqual(tombstone["function"], "focus")

    def test_v2_disjoint_stale_candidate_rebases_and_combines_edits(self) -> None:
        """A disjoint live-source advance must not discard a winning cell."""

        campaign, campaign_source, _candidate_source, descriptor_path, selected = (
            self._v2_rebase_fixture()
        )
        dispatch_calls: list[list[Path]] = []

        def dispatch(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> list[dict[str, object]]:
            dispatch_calls.append(paths)
            return [self._stale_result(paths[0])]

        def select_then_advance(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            self._refresh_v2_source_binding(
                current_campaign, campaign_source, before=9, focus_result=0
            )
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": selected,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select_then_advance,
            ),
            patch.object(owner_campaign, "run_loop", side_effect=dispatch),
        ):
            result = lane.run_inbox(self.root, campaign)

        self.assertEqual(dispatch_calls, [[descriptor_path]])
        self.assertEqual(result["status"], "processed")
        self.assertEqual(result["results"][0]["status"], lane.REBASED_STATUS)
        self.assertEqual(result["results"][0]["rebase_depth"], 1)
        new_descriptor = self.root / result["results"][0]["new_descriptor"]
        self.assertTrue(new_descriptor.is_file())
        self.assertFalse(descriptor_path.exists())
        new_candidate = self.root / result["results"][0]["new_candidate"]
        self.assertEqual(
            new_candidate.read_text(encoding="utf-8"),
            "int before = 9;\n\n"
            "int focus(void) {\n"
            "    return 1;\n"
            "}\n"
            "int after = 2;\n",
        )
        new_descriptor_body = json.loads(new_descriptor.read_text(encoding="utf-8"))
        self.assertEqual(new_descriptor_body["rebase_depth"], 1)
        self.assertEqual(
            new_descriptor_body["base_source"]["sha256"],
            owner_campaign._digest_file(new_descriptor.parent / "base.c"),
        )
        self.assertEqual(len(self._find_rebase_tombstones()), 1)
        tombstone = self._find_rebase_tombstones()[0]
        self._assert_sealed_tombstone(tombstone, status=lane.REBASED_STATUS)
        self.assertEqual(tombstone["old_descriptor_sha256"], selected["descriptor_sha256"])
        self.assertEqual(tombstone["new_candidate_sha256"], new_descriptor_body["candidate_source"]["sha256"])

    def test_v2_overlapping_stale_candidate_is_rejected_without_retry_loop(self) -> None:
        """Changing the named function invalidates, retires, and cannot spin."""

        campaign, campaign_source, _candidate_source, descriptor_path, selected = (
            self._v2_rebase_fixture()
        )
        dispatch_calls: list[list[Path]] = []

        def dispatch(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> list[dict[str, object]]:
            dispatch_calls.append(paths)
            return [self._stale_result(paths[0])]

        def select_then_overlap(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            self._refresh_v2_source_binding(
                current_campaign, campaign_source, before=1, focus_result=7
            )
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": selected,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select_then_overlap,
            ),
            patch.object(owner_campaign, "run_loop", side_effect=dispatch),
        ):
            result = lane.run_inbox(self.root, campaign)

        self.assertEqual(dispatch_calls, [[descriptor_path]])
        self.assertEqual(result["results"][0]["status"], lane.REBASE_REJECTED_STATUS)
        self.assertFalse(descriptor_path.exists())
        tombstones = self._find_rebase_tombstones()
        self.assertEqual(len(tombstones), 1)
        self._assert_sealed_tombstone(tombstones[0], status=lane.REBASE_REJECTED_STATUS)

        with patch.object(owner_campaign, "run_loop") as retry:
            second = lane.run_inbox(self.root, campaign)
        self.assertEqual(second["status"], "idle")
        retry.assert_not_called()

    def test_v2_rebase_depth_limit_is_sealed_and_idempotent(self) -> None:
        """Depth five is a terminal tombstone, not an endlessly stale input."""

        campaign, campaign_source, _candidate_source, descriptor_path, selected = (
            self._v2_rebase_fixture(rebase_depth=5)
        )
        dispatch_calls: list[list[Path]] = []

        def dispatch(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> list[dict[str, object]]:
            dispatch_calls.append(paths)
            return [self._stale_result(paths[0])]

        def select_then_advance(
            root: Path,
            current_campaign: dict[str, object],
            paths: list[Path],
        ) -> dict[str, object]:
            self._refresh_v2_source_binding(
                current_campaign, campaign_source, before=8, focus_result=0
            )
            return {
                "status": lane.owner_campaign_selector.SELECTED,
                "selected": selected,
            }

        with (
            patch.object(
                lane.owner_campaign_selector,
                "select_winning_candidate",
                side_effect=select_then_advance,
            ),
            patch.object(owner_campaign, "run_loop", side_effect=dispatch),
        ):
            result = lane.run_inbox(self.root, campaign)

        self.assertEqual(dispatch_calls, [[descriptor_path]])
        self.assertEqual(result["results"][0]["status"], lane.REBASE_REJECTED_STATUS)
        self.assertFalse(descriptor_path.exists())
        tombstones = self._find_rebase_tombstones()
        self.assertEqual(len(tombstones), 1)
        self._assert_sealed_tombstone(tombstones[0], status=lane.REBASE_REJECTED_STATUS)
        self.assertEqual(tombstones[0]["rebase_depth"], 5)

        with patch.object(owner_campaign, "run_loop") as retry:
            second = lane.run_inbox(self.root, campaign)
        self.assertEqual(second["status"], "idle")
        retry.assert_not_called()

    def test_loaded_campaign_concurrently_rebases_two_disjoint_stale_cells(self) -> None:
        """Real snapshots/proposals refresh source and publish tombstones once."""

        source = self.root / "src" / "owner.c"
        source.parent.mkdir()
        source.write_text(
            "int focus(void) { /* BASE */ return 0; }\n"
            "int other(void) { /* BASE */ return 0; }\n"
            "int anchor(void) { return 0; }\n",
            encoding="utf-8",
        )
        target = self.root / "build" / "evidence" / "target.o"
        target.parent.mkdir(parents=True)
        target.write_bytes(b"target")
        toolchain = self.root / "build" / "evidence" / "toolchain.json"
        toolchain.write_text("{}\n", encoding="utf-8")
        hook = self.root / "hook.py"
        # Make physical row identity source-bound.  A disjoint TU edit then
        # exercises the production unmatched-physical downgrade path while
        # strict/data row identities remain remappable.
        hook.write_text(
            CAMPAIGN_HOOK.replace(
                '"physical_difference_ids": [f"physical:{index}" for index in range(physical)],',
                '"physical_difference_ids": [f"physical:{index}:sha256={hashlib.sha256((source_sha + str(index)).encode()).hexdigest()}" for index in range(physical)],',
            ),
            encoding="utf-8",
        )
        subprocess.run(["git", "init", "-q"], cwd=self.root, check=True)
        subprocess.run(
            ["git", "config", "core.autocrlf", "false"], cwd=self.root, check=True
        )
        subprocess.run(
            ["git", "config", "user.email", "test@example.invalid"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Lane Test"], cwd=self.root, check=True
        )
        subprocess.run(["git", "add", "src/owner.c", "hook.py"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "fixture"], cwd=self.root, check=True)
        commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()
        manifest_path = self.root / "build" / "campaign.json"
        manifest_body: dict[str, object] = {
            "schema": owner_campaign.CAMPAIGN_SCHEMA,
            "campaign_id": "loaded-lane-rebase-v2",
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "source_relpath": "src/owner.c",
            "base_commit": commit,
            "target_object": {
                "path": "build/evidence/target.o",
                "sha256": _digest(b"target"),
            },
            "toolchain": {
                "path": "build/evidence/toolchain.json",
                "sha256": owner_campaign._digest_file(toolchain),
            },
            "measurement_producer": {
                "path": "hook.py",
                "sha256": owner_campaign._digest_file(hook),
            },
            "functions": ["focus", "other", "anchor"],
            "protected_exact_functions": ["anchor"],
            "allowed_source_paths": ["src/owner.c"],
            "allowed_build_paths": ["build"],
            "forbidden_constructs": [r"\b(?:asm|volatile|register)\b", r"#\s*pragma"],
            "commands": {
                phase: {
                    "argv": [
                        sys.executable,
                        "{MEASUREMENT_PRODUCER}",
                        "{SOURCE}",
                        f"build/hook/{phase}.json",
                        "{ROOT}/build/invocations.log",
                    ],
                    "measurement_relpath": f"build/hook/{phase}.json",
                }
                for phase in ("snapshot", "candidate", "final_owner")
            },
            "cancellation_epoch": 1,
            "limits": {
                "command_timeout_seconds": 20,
                "scratch_soft_bytes": 32 << 20,
                "scratch_hard_bytes": 64 << 20,
                "cell_temporary_bytes": 1 << 20,
                "focus_evidence_bytes": 256 << 10,
                "frontier_bytes": 64 << 10,
                "report_bytes": 64 << 10,
                "dedupe_bytes": 1 << 20,
                "owner_state_bytes": 16 << 20,
            },
        }
        manifest = _seal(manifest_body, "manifest_sha256")
        manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
        campaign = owner_campaign.load_campaign(self.root, manifest_path)

        descriptors: list[Path] = []
        for function, replacement in (("focus", "return 1"), ("other", "return 2")):
            frontier = owner_campaign.snapshot_frontier(self.root, campaign, function)
            focus_path, focus_sha, focus = lane._focus_artifact_for_proposal(
                self.root, campaign, function, frontier
            )
            _physical_path, _physical_sha, physical = lane._physical_cas_for_proposal(
                self.root, campaign, frontier, focus_path, focus_sha, focus
            )
            strict_rows, data_rows, physical_rows = (
                lane.owner_campaign_selector._artifact_row_groups(focus, physical)
            )
            predicted_rows = lane.owner_campaign_selector._ordered_union(
                strict_rows, data_rows, physical_rows
            )
            candidate = self.root / "build" / "candidates" / f"{function}.c"
            candidate.parent.mkdir(parents=True, exist_ok=True)
            candidate.write_text(
                source.read_text(encoding="utf-8").replace(
                    f"int {function}(void) {{ /* BASE */ return 0; }}",
                    f"int {function}(void) {{ /* BASE */ {replacement}; }}",
                ),
                encoding="utf-8",
            )
            proposal = lane.propose_candidate(
                self.root,
                campaign,
                function,
                candidate,
                f"{function}-winning-cell",
                expected_terminal="improved",
                predicted_rows=predicted_rows,
                predicted_remaining_counts={"strict": 0, "data": 0, "physical": 0},
            )
            descriptors.append(self.root / proposal["candidate_descriptor"])

        dispatch_barrier = threading.Barrier(2, timeout=5)
        source_advanced = threading.Event()

        def stale_dispatch(
            root: Path,
            current_campaign: dict[str, object],
            path: Path,
            *,
            worker: int,
        ) -> dict[str, object]:
            descriptor = json.loads(path.read_text(encoding="utf-8"))
            dispatch_barrier.wait()
            if descriptor["function"] == "focus":
                source.write_text(
                    source.read_text(encoding="utf-8").replace(
                        "int anchor(void) { return 0; }",
                        "int anchor(void) { return 9; }",
                    ),
                    encoding="utf-8",
                )
                source_advanced.set()
            else:
                self.assertTrue(source_advanced.wait(timeout=5))
            return self._stale_result(path, function=str(descriptor["function"]))

        with (
            patch.object(lane, "_dispatch_selected_candidate", side_effect=stale_dispatch),
            patch.object(
                owner_campaign,
                "snapshot_frontier",
                wraps=owner_campaign.snapshot_frontier,
            ) as snapshots,
            patch.object(lane, "_post_pipeline_maintenance") as maintenance,
        ):
            result = lane.run_inbox(self.root, campaign, _pre_discovered=descriptors)

        maintenance.assert_called_once_with(self.root, campaign, result["results"])
        self.assertEqual(snapshots.call_count, 2)
        self.assertTrue(
            all(
                invocation.kwargs.get("_defer_maintenance") is True
                for invocation in snapshots.call_args_list
            )
        )
        self.assertEqual([item["status"] for item in result["results"]], [
            lane.REBASED_STATUS,
            lane.REBASED_STATUS,
        ], result)
        self.assertEqual([item["function"] for item in result["results"]], [
            "focus",
            "other",
        ])
        for item in result["results"]:
            descriptor = json.loads(
                (self.root / item["new_descriptor"]).read_text(encoding="utf-8")
            )
            selection = json.loads(
                (self.root / item["new_descriptor"]).with_name(
                    "candidate.selection.json"
                ).read_text(encoding="utf-8")
            )
            self.assertEqual(descriptor["base_source"]["sha256"], owner_campaign._digest_file(source))
            self.assertEqual(descriptor["rebase_depth"], 1)
            self.assertEqual(selection["expected_terminal"], "improved")
            self.assertFalse(any(row.startswith("physical:") for row in selection["predicted_rows"]))
        tombstones = self._find_rebase_tombstones()
        self.assertEqual(len(tombstones), 2)
        self.assertEqual({item["function"] for item in tombstones}, {"focus", "other"})
        tombstone_digests = {item["tombstone_sha256"] for item in tombstones}

        with patch.object(lane, "_dispatch_selected_candidate") as retry:
            second = lane.run_inbox(self.root, campaign, _pre_discovered=[])
        self.assertEqual(second["status"], "idle")
        retry.assert_not_called()
        self.assertEqual(
            {item["tombstone_sha256"] for item in self._find_rebase_tombstones()},
            tombstone_digests,
        )

    def test_stale_row_remap_downgrades_unmatched_physical_identity(self) -> None:
        old = [
            "strict:focus:row:7:kind=DIFF_ARG_MISMATCH:target=10:candidate=12",
            "physical:focus:row:0:sha256=" + "1" * 64,
        ]
        current = [
            "strict:focus:row:7:kind=DIFF_ARG_MISMATCH:target=10:candidate=44",
            "physical:focus:row:0:sha256=" + "2" * 64,
        ]

        self.assertEqual(lane._remap_predicted_rows(old, current), current[:1])
        with self.assertRaisesRegex(
            owner_campaign.CampaignError, "no current predicted rows"
        ):
            lane._remap_predicted_rows(old[1:], current)

    def test_proposal_preparation_runs_concurrently_before_publication_lock(self) -> None:
        """Immutable evidence work must not serialize on the frontier lock."""

        campaign, campaign_source, _unused = self._proposal_fixture()
        campaign["_source"] = campaign_source
        campaign["limits"] = {"command_timeout_seconds": 1}
        candidate_a = self._candidate_source("parallel-a", 1)
        candidate_b = self._candidate_source("parallel-b", 2)
        focus_path = self.root / "build" / "focus.json"
        physical_path = self.root / "build" / "physical.json"
        focus_path.write_bytes(b"focus")
        physical_path.write_bytes(b"physical")
        frontier = json.loads(
            (
                owner_campaign._function_root(self.root, campaign, "focus")
                / "latest-frontier.json"
            ).read_text(encoding="utf-8")
        )
        selection = {
            "evidence_sha256": "a" * 64,
            "physical_artifact": {
                "path": physical_path.relative_to(self.root).as_posix(),
                "sha256": _digest(physical_path.read_bytes()),
            },
        }
        barrier = threading.Barrier(2, timeout=5)
        publish_lock = threading.Lock()

        class SerialLock:
            def __enter__(self):
                publish_lock.acquire()
                return self

            def __exit__(self, exc_type, exc_value, traceback):
                publish_lock.release()
                return False

        def evidence(*_args, **_kwargs):
            # Both workers must reach the expensive preparation barrier before
            # either can enter the serialized publication phase.
            barrier.wait()
            return (
                selection,
                focus_path,
                _digest(focus_path.read_bytes()),
                ["strict:focus:row:1:"],
                {"strict": 0, "data": 0, "physical": 0},
            )

        results: list[dict[str, object]] = []
        errors: list[BaseException] = []

        def worker(candidate: Path, family: str) -> None:
            try:
                results.append(
                    lane.propose_candidate(
                        self.root,
                        campaign,
                        "focus",
                        candidate,
                        family,
                        expected_terminal="improved",
                        predicted_rows=["strict:focus:row:1:"],
                        predicted_remaining_counts={
                            "strict": 0,
                            "data": 0,
                            "physical": 0,
                        },
                    )
                )
            except BaseException as exc:  # surfaced below with thread context
                errors.append(exc)

        with (
            patch.object(lane, "_frontier_for_proposal", return_value=frontier),
            patch.object(
                lane, "_selection_evidence_for_proposal", side_effect=evidence
            ),
            patch.object(
                owner_campaign,
                "_frontier_lock_chain",
                side_effect=lambda *_args, **_kwargs: SerialLock(),
            ),
        ):
            threads = [
                threading.Thread(target=worker, args=(candidate_a, "parallel-a")),
                threading.Thread(target=worker, args=(candidate_b, "parallel-b")),
            ]
            for thread in threads:
                thread.start()
            for thread in threads:
                thread.join(timeout=10)

        self.assertFalse(errors, errors)
        self.assertEqual(len(results), 2)
        self.assertEqual(
            {result["hypothesis_family"] for result in results},
            {"parallel-a", "parallel-b"},
        )

    def test_proposal_rejects_source_drift_after_immutable_preparation(self) -> None:
        """A source change between preparation and publish must fail closed."""

        campaign, campaign_source, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        original_stage = lane._stage_prepared_proposal

        def stage_then_drift(prepared):
            stage = original_stage(prepared)
            campaign_source.write_text(
                "int before = 9;\n\n"
                "int focus(void) { return 0; }\n"
                "int after = 2;\n",
                encoding="utf-8",
            )
            return stage

        with patch.object(
            lane, "_stage_prepared_proposal", side_effect=stage_then_drift
        ):
            with self.assertRaisesRegex(owner_campaign.CampaignError, "drift"):
                lane.propose_candidate(
                    self.root, campaign, "focus", candidate_source, "stale-after-prep"
                )
        self.assertFalse(
            any(path.is_dir() for path in lane.inbox_path(self.root, campaign).iterdir())
        )

    def test_proposal_publication_retries_transient_windows_access_denied(self) -> None:
        """A transient Windows directory-link denial does not lose the cell."""

        campaign, _campaign_source, candidate_source = self._proposal_fixture()
        candidate_source.write_text(
            "int before = 1;\n\n"
            "int focus(void) { return 1; }\n"
            "int after = 2;\n",
            encoding="utf-8",
        )
        real_rename = lane.os.rename
        attempts = 0

        def transient_once(source: Path, destination: Path) -> None:
            nonlocal attempts
            attempts += 1
            if attempts == 1:
                error = PermissionError("transient directory notification")
                error.winerror = 5
                raise error
            real_rename(source, destination)

        with (
            patch.object(lane.os, "name", "nt"),
            patch.object(lane.os, "rename", side_effect=transient_once),
        ):
            result = lane.propose_candidate(
                self.root, campaign, "focus", candidate_source, "transient-link"
            )

        self.assertEqual(attempts, 2)
        self.assertEqual(result["status"], "queued")
        self.assertTrue((self.root / result["candidate_descriptor"]).is_file())


if __name__ == "__main__":
    unittest.main()
