from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import frontend_temporary_birth_order as reducer


FUNCTION = "mbev_CapKillerMove"


def _row(index: int, formatted: str) -> dict[str, object]:
    return {
        "arg_diff": [{}, {"diff_index": index % 3}, {}],
        "diff_kind": "DIFF_ARG_MISMATCH",
        "index": index,
        "instruction": {
            "address": str(17000 + index * 4),
            "formatted": formatted,
            "size": 4,
        },
    }


def _side(rows: list[dict[str, object]]) -> dict[str, object]:
    return {
        "rows": rows,
        "rows_kind": "diff_only",
        "symbol": {
            "name": FUNCTION,
            "size": "6368",
            "kind": "SYMBOL_FUNCTION",
        },
    }


def focus_artifact() -> dict[str, object]:
    candidate = [
        _row(218, "stfs f0, 0xc(r1)"),
        _row(219, "lfs f24, 0xc(r1)"),
        _row(919, "stfs f0, 0x8(r1)"),
        _row(920, "lfs f23, 0x8(r1)"),
        _row(1335, "stw r0, 0x10(r1)"),
        _row(1337, "lwz r4, 0x10(r1)"),
    ]
    target = [
        _row(218, "stfs f0, 0x10(r1)"),
        _row(219, "lfs f24, 0x10(r1)"),
        _row(919, "stfs f0, 0xc(r1)"),
        _row(920, "lfs f23, 0xc(r1)"),
        _row(1335, "stw r0, 0x8(r1)"),
        _row(1337, "lwz r4, 0x8(r1)"),
    ]
    metric = {
        "target_size": 6368,
        "candidate_size": 6368,
        "diff_rows": 6,
        "diff_kinds": {"DIFF_ARG_MISMATCH": 6},
        "exact": False,
    }
    return {
        "schema": "focus_symbol_report/v1",
        "artifact_sha256": "10" * 32,
        "authority_advanced": False,
        "function": FUNCTION,
        "channels": {
            "strict": {
                "target": _side(copy.deepcopy(target)),
                "candidate": _side(copy.deepcopy(candidate)),
                "metric": copy.deepcopy(metric),
            },
            "data": {
                "target": _side(copy.deepcopy(target)),
                "candidate": _side(copy.deepcopy(candidate)),
                "metric": copy.deepcopy(metric),
            },
        },
    }


def context() -> dict[str, object]:
    return {
        "schema": reducer.CONTEXT_SCHEMA,
        "function": FUNCTION,
        "focus_file_sha256": "11" * 32,
        "focus_artifact_sha256": "10" * 32,
        "report_sha256": "20" * 32,
        "candidate_source_sha256": "30" * 32,
        "candidate_object_sha256": "40" * 32,
        "target_object_sha256": "50" * 32,
        "protected_siblings_zero_loss": True,
        "preconditions": {
            "function_size_exact": True,
            "cfg_exact": True,
            "calls_exact": True,
            "data_values_exact": True,
            "physical_relocations": {"status": "unknown"},
        },
        "temporaries": [
            {
                "id": "initial_sqrtf",
                "kind": "compiler_call_temporary",
                "source_type": "float",
                "row_indices": [218, 219],
                "candidate_home": "0x0c",
                "target_home": "0x10",
                "current_birth_rank": 2,
                "proposed_birth_rank": 1,
                "producer": "initial sqrtf result",
                "consumer": "initial atan2 argument",
                "evaluation_order_sealed": True,
                "use_count": 1,
            },
            {
                "id": "route_sqrtf",
                "kind": "compiler_call_temporary",
                "source_type": "float",
                "row_indices": [919, 920],
                "candidate_home": "0x08",
                "target_home": "0x0c",
                "current_birth_rank": 3,
                "proposed_birth_rank": 2,
                "producer": "route sqrtf result",
                "consumer": "route atan2 argument",
                "evaluation_order_sealed": True,
                "use_count": 1,
            },
            {
                "id": "final_dust_address",
                "kind": "live_typed_address",
                "source_type": "HuVecF *",
                "row_indices": [1335, 1337],
                "candidate_home": "0x10",
                "target_home": "0x08",
                "current_birth_rank": 1,
                "proposed_birth_rank": 3,
                "producer": "address of live dust snapshot",
                "consumer": "mbev_CapEffDustCloudAdd argument 2",
                "evaluation_order_sealed": True,
                "use_count": 1,
            },
        ],
        "consumer_boundary": {
            "temporary_id": "final_dust_address",
            "aggregate_type": "HuVecF",
            "copy_expression": "dustPos = killerPos",
            "typed_consumer": "mbev_CapEffDustCloudAdd:arg2",
            "aggregate_copy_required": True,
            "use_count": 1,
            "current_source_class": "explicit_pointer_local",
            "proposed_source_class": "live_aggregate_copy_right_argument_temporary",
        },
        "negative_controls": [
            {
                "id": "h116",
                "source_class": "narrow_type_change",
                "outcome": "size_drift",
                "evidence": "grew to 6372 bytes and 15 rows",
            },
            {
                "id": "h117",
                "source_class": "function_scope_pointer",
                "outcome": "topology_drift",
                "evidence": "grew to 6372 bytes and 93 rows",
            },
            {
                "id": "h118",
                "source_class": "spanning_pointer",
                "outcome": "object_identical",
                "evidence": "object-neutral control",
            },
            {
                "id": "h120",
                "source_class": "pointer_role_swap",
                "outcome": "topology_drift",
                "evidence": "grew to 6372 bytes and 90 rows",
            },
            {
                "id": "h121",
                "source_class": "direct_aggregate_argument",
                "outcome": "size_drift",
                "evidence": "shrunk to 6364 bytes and 569 rows",
            },
        ],
    }


class FrontendTemporaryBirthOrderTests(unittest.TestCase):
    def test_killermove_h105_ranks_one_consumer_boundary_cell(self) -> None:
        result = reducer.build_diagnosis(focus_artifact(), context())

        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["route"], reducer.ROUTE)
        self.assertEqual(result["facts"]["strict_diff_row_count"], 6)
        self.assertEqual(result["facts"]["data_diff_row_count"], 6)
        self.assertEqual(result["facts"]["temporary_count"], 3)
        self.assertTrue(result["facts"]["closed_home_permutation"])
        self.assertTrue(result["facts"]["all_rows_accounted"])
        self.assertEqual(
            [row["target_home"] for row in result["predicted_target_homes"]],
            ["0x10", "0xc", "0x8"],
        )
        self.assertEqual(
            [cell["id"] for cell in result["candidate_cells"]],
            ["compose_live_address_at_consumer_boundary"],
        )
        self.assertEqual(result["compile_candidate_budget"], 1)
        self.assertEqual(result["trace_budget"], 0)
        self.assertIn("function_scope_pointer", result["suppressed_axes"])
        self.assertIn("direct_aggregate_argument", result["suppressed_axes"])
        self.assertIn("physical_relocation_authority_unknown", result["warnings"])
        self.assertFalse(result["source_patch_emitted"])
        self.assertFalse(result["authority_advanced"])

    def test_blocks_unsupported_non_home_row(self) -> None:
        focus = focus_artifact()
        focus["channels"]["strict"]["target"]["rows"][0]["instruction"][
            "formatted"
        ] = "addi r3, r1, 0x10"
        result = reducer.build_diagnosis(focus, context())
        self.assertEqual(result["status"], "blocked")
        self.assertIn("row_218_not_supported_stack_memory", result["blockers"])
        self.assertEqual(result["candidate_cells"], [])

    def test_blocks_when_reverse_birth_model_does_not_predict_candidate(self) -> None:
        value = context()
        value["temporaries"][0]["current_birth_rank"] = 1
        value["temporaries"][2]["current_birth_rank"] = 2
        result = reducer.build_diagnosis(focus_artifact(), value)
        self.assertEqual(result["status"], "blocked")
        self.assertTrue(
            any(
                "candidate_not_reverse_birth_order" in item
                for item in result["blockers"]
            )
        )

    def test_blocks_multiple_live_typed_addresses(self) -> None:
        value = context()
        value["temporaries"][1]["kind"] = "live_typed_address"
        result = reducer.build_diagnosis(focus_artifact(), value)
        self.assertEqual(result["status"], "blocked")
        self.assertIn(
            "requires_exactly_one_live_typed_address", result["blockers"]
        )

    def test_full_candidate_channel_filters_exact_rows(self) -> None:
        focus = focus_artifact()
        for side_name in ("target", "candidate"):
            side = focus["channels"]["strict"][side_name]
            exact_row = _row(0, "stwu r1, -0x2b0(r1)")
            exact_row["diff_kind"] = "DIFF_NONE"
            side["rows"].append(exact_row)
            side["rows_kind"] = "all"
        result = reducer.build_diagnosis(focus, context())
        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["facts"]["strict_diff_row_count"], 6)

    def test_path_binding_and_diagnosis_hash(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            focus_path = root / "focus.json"
            context_path = root / "context.json"
            focus_text = json.dumps(focus_artifact(), sort_keys=True) + "\n"
            focus_path.write_bytes(focus_text.encode("utf-8"))
            value = context()
            value["focus_file_sha256"] = hashlib.sha256(
                focus_text.encode("utf-8")
            ).hexdigest()
            context_path.write_text(
                json.dumps(value, sort_keys=True) + "\n", encoding="utf-8"
            )

            result = reducer.build_from_paths(focus_path, context_path)
            digest = result.pop("diagnosis_sha256")
            self.assertEqual(digest, reducer._canonical_sha256(result))


if __name__ == "__main__":
    unittest.main()
