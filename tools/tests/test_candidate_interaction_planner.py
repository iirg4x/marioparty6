from __future__ import annotations

import copy
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools import candidate_interaction_planner as module


SHA_A = "a" * 64
SHA_B = "b" * 64
SHA_C = "c" * 64
SHA_D = "d" * 64


def _level(
    level_id: str,
    topology: str,
    *,
    admissibility: str = "natural",
) -> dict[str, object]:
    return {
        "id": level_id,
        "topology_token": topology,
        "source_action": f"apply {level_id}",
        "evidence": [f"target evidence for {level_id}"],
        "admissibility": admissibility,
    }


def _request() -> dict[str, object]:
    return {
        "schema": module.REQUEST_SCHEMA,
        "planner_id": "kamekku-state2-v1",
        "focus_symbols": ["ev_CapKamekkuOMExec"],
        "axes": [
            {
                "id": "divisor",
                "hypothesis": "state-2 frame horizon",
                "control_level": "frames36",
                "levels": [
                    _level("frames36", "divisor-f32-36"),
                    _level("frames60", "divisor-f32-60"),
                ],
            },
            {
                "id": "rng_owner",
                "hypothesis": "RNG result lifetime",
                "control_level": "discard",
                "levels": [
                    _level("discard", "rng-discard"),
                    _level("time", "rng-assign-existing-time"),
                ],
            },
        ],
    }


class CandidateInteractionPlannerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _write(self, value: object, name: str = "request.json") -> Path:
        path = self.root / name
        path.write_text(json.dumps(value, indent=2), encoding="utf-8")
        return path

    def _build(self, value: object) -> dict[str, object]:
        return module.build_interaction_plan(self._write(value))

    def test_kamekku_two_by_two_emits_controls_singles_and_combined(self) -> None:
        plan = self._build(_request())
        self.assertEqual(plan["schema"], module.PLAN_SCHEMA)
        self.assertFalse(plan["production_modified"])
        self.assertFalse(plan["authority_advanced"])
        self.assertEqual(
            plan["summary"],
            {
                "raw_cell_count": 4,
                "unique_topology_count": 4,
                "topology_duplicate_count": 0,
                "blocked_cell_count": 0,
                "observed_cell_count": 0,
                "generate_and_compile_count": 4,
                "source_duplicate_observation_count": 0,
                "object_duplicate_observation_count": 0,
            },
        )
        self.assertEqual(
            [batch["interaction_order"] for batch in plan["batches"]], [0, 1, 2]
        )
        combined = next(
            cell
            for cell in plan["cells"]
            if cell["selection"] == {"divisor": "frames60", "rng_owner": "time"}
        )
        self.assertEqual(combined["interaction_order"], 2)
        self.assertEqual(combined["action"], "generate_and_compile")
        self.assertEqual(len(plan["recommended_execution_order"]), 4)

    def test_explicit_topology_tokens_dedupe_without_semantic_inference(self) -> None:
        request = _request()
        request["axes"][0]["levels"].append(
            _level("frames60_parenthesized", "divisor-f32-60")
        )
        plan = self._build(request)
        self.assertEqual(plan["summary"]["raw_cell_count"], 6)
        self.assertEqual(plan["summary"]["unique_topology_count"], 4)
        self.assertEqual(plan["summary"]["topology_duplicate_count"], 2)
        duplicates = [
            cell for cell in plan["cells"] if cell["action"] == "skip_duplicate_topology"
        ]
        self.assertEqual(len(duplicates), 2)
        self.assertTrue(all(cell["topology_duplicate_of"] for cell in duplicates))

    def test_observations_distinguish_source_and_object_duplicates(self) -> None:
        request = _request()
        request["observations"] = [
            {
                "selection": {"divisor": "frames36", "rng_owner": "discard"},
                "candidate_id": "baseline",
                "source_sha256": SHA_A,
                "object_sha256": SHA_B,
            },
            {
                "selection": {"divisor": "frames36", "rng_owner": "time"},
                "candidate_id": "rng-only",
                "source_sha256": SHA_C,
                "object_sha256": SHA_B,
            },
            {
                "selection": {"divisor": "frames60", "rng_owner": "discard"},
                "candidate_id": "divisor-only",
                "source_sha256": SHA_A,
                "object_sha256": SHA_D,
            },
        ]
        plan = self._build(request)
        rows = {
            cell["observation"]["candidate_id"]: cell
            for cell in plan["cells"]
            if cell["observation"] is not None
        }
        self.assertEqual(rows["rng-only"]["observation"]["duplicate_object_of"], "baseline")
        self.assertIsNone(rows["rng-only"]["observation"]["duplicate_source_of"])
        self.assertEqual(rows["divisor-only"]["observation"]["duplicate_source_of"], "baseline")
        self.assertIsNone(rows["divisor-only"]["observation"]["duplicate_object_of"])
        self.assertEqual(plan["summary"]["observed_cell_count"], 3)
        self.assertEqual(plan["summary"]["generate_and_compile_count"], 1)

    def test_constraints_and_blocked_levels_are_not_scheduled(self) -> None:
        request = _request()
        request["axes"][1]["levels"].append(
            _level("invented", "rng-invented-local", admissibility="blocked")
        )
        request["constraints"] = [
            {
                "when": {"divisor": "frames60", "rng_owner": "discard"},
                "reason": "already measured neutral control",
            }
        ]
        plan = self._build(request)
        blocked = [cell for cell in plan["cells"] if cell["action"] == "blocked"]
        self.assertEqual(len(blocked), 3)
        self.assertTrue(
            any(cell["blocked_reason"] == "already measured neutral control" for cell in blocked)
        )
        self.assertTrue(
            any("blocked source level" in cell["blocked_reason"] for cell in blocked)
        )

    def test_deterministic_under_axis_level_and_observation_reordering(self) -> None:
        first = _request()
        first["observations"] = [
            {
                "selection": {"divisor": "frames60", "rng_owner": "time"},
                "candidate_id": "combined",
                "source_sha256": SHA_A,
                "object_sha256": SHA_B,
            }
        ]
        second = copy.deepcopy(first)
        second["axes"].reverse()
        for axis in second["axes"]:
            axis["levels"].reverse()
        first_path = self._write(first, "first.json")
        second_path = self._write(second, "second.json")
        first_plan = module.build_interaction_plan(first_path)
        second_plan = module.build_interaction_plan(second_path)
        first_plan.pop("request_sha256")
        first_plan.pop("interaction_plan_sha256")
        second_plan.pop("request_sha256")
        second_plan.pop("interaction_plan_sha256")
        self.assertEqual(first_plan, second_plan)

    def test_fails_closed_on_unknown_duplicate_and_oversized_product(self) -> None:
        unknown = _request()
        unknown["unexpected"] = True
        with self.assertRaisesRegex(module.InteractionPlanError, "unknown field"):
            self._build(unknown)

        duplicate = _request()
        duplicate["axes"][0]["levels"][1]["id"] = "frames36"
        with self.assertRaisesRegex(module.InteractionPlanError, "level ids must be unique"):
            self._build(duplicate)

        oversized = _request()
        oversized["max_cells"] = 3
        with self.assertRaisesRegex(module.InteractionPlanError, "exceeding max_cells"):
            self._build(oversized)

    def test_rejects_partial_or_unknown_observation_selection(self) -> None:
        request = _request()
        request["observations"] = [
            {
                "selection": {"divisor": "frames60"},
                "candidate_id": "partial",
                "source_sha256": SHA_A,
                "object_sha256": SHA_B,
            }
        ]
        with self.assertRaisesRegex(module.InteractionPlanError, "name every axis"):
            self._build(request)

    def test_cli_is_json_and_does_not_mutate_request(self) -> None:
        request_path = self._write(_request())
        before = request_path.read_bytes()
        process = subprocess.run(
            [sys.executable, str(Path(module.__file__)), str(request_path)],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(json.loads(process.stdout)["schema"], module.PLAN_SCHEMA)
        self.assertEqual(request_path.read_bytes(), before)


if __name__ == "__main__":
    unittest.main()
