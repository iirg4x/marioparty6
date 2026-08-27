from __future__ import annotations

import copy
import unittest

from tools import owner_flow_matcher as matcher
from tools import typed_pool_owner_manifest as owner_manifest


FUNCTION = "ConfigExec"
POOL_ROWS = [188, 199, 251, 253, 258, 499, 650]


def _row(index: int, formatted: str, *, branch_dest: int | None = None, pool_symbol: int | None = None) -> dict[str, object]:
    instruction: dict[str, object] = {
        "address": str(1800 + index * 4),
        "formatted": formatted,
        "size": 4,
    }
    if branch_dest is not None:
        instruction["branch_dest"] = str(branch_dest)
    if pool_symbol is not None:
        instruction["relocation"] = {
            "target_symbol": pool_symbol,
            "type": 109,
            "type_name": "R_PPC_EMB_SDA21",
        }
    return {
        "arg_diff": [{"diff_index": index % 12}],
        "diff_kind": "DIFF_ARG_MISMATCH",
        "index": index,
        "instruction": instruction,
    }


def _side(rows: list[dict[str, object]], *, diff_only: bool = False) -> dict[str, object]:
    return {
        "rows": rows,
        "rows_kind": "diff_only" if diff_only else "all",
        "symbol": {"name": FUNCTION, "size": "3772", "kind": "SYMBOL_FUNCTION"},
    }


def focus_artifact() -> dict[str, object]:
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    target_pool = ["lbl_A", "lbl_B", "lbl_C", "lbl_D", "lbl_E", "lbl_B", "lbl_B"]
    candidate_pool = ["@536", "@537", "@538", "@539", "@540", "@536", "@536"]
    for ordinal, (index, target_owner, candidate_owner) in enumerate(
        zip(POOL_ROWS, target_pool, candidate_pool), start=10
    ):
        destination = "f1" if index in {188, 199, 499, 650} else "f0"
        target.append(_row(index, f"lfs {destination}, {target_owner}@sda21", pool_symbol=ordinal))
        candidate.append(_row(index, f"lfs {destination}, {candidate_owner}@sda21", pool_symbol=ordinal + 20))

    pairs = [
        (284, "stw r0, 0x38(r1)", "stw r0, 0x30(r1)"),
        (563, "addi r3, r3, 0x21", "addi r3, r3, 0x20"),
        (714, "ble 0x13c4", "ble 0x1344"),
        (720, "stw r0, 0x34(r1)", "stw r0, 0x38(r1)"),
        (823, "stw r0, 0x30(r1)", "stw r0, 0x34(r1)"),
        (824, "lwz r0, 0x30(r1)", "lwz r0, 0x34(r1)"),
        (831, "lwz r0, 0x30(r1)", "lwz r0, 0x34(r1)"),
        (861, "cntlzw r3, r0", "cntlzw r0, r0"),
        (862, "srwi r0, r3, 5", "srwi r3, r0, 5"),
        (863, "stw r0, 0x2c(r1)", "stw r3, 0x2c(r1)"),
    ]
    for index, target_text, candidate_text in pairs:
        if index == 714:
            target.append(_row(index, target_text, branch_dest=5060))
            candidate.append(_row(index, candidate_text, branch_dest=4932))
        else:
            target.append(_row(index, target_text))
            candidate.append(_row(index, candidate_text))
    target.sort(key=lambda row: int(row["index"]))
    candidate.sort(key=lambda row: int(row["index"]))
    data_target = [copy.deepcopy(row) for row in target if int(row["index"]) not in POOL_ROWS]
    data_candidate = [copy.deepcopy(row) for row in candidate if int(row["index"]) not in POOL_ROWS]
    metric = {
        "target_size": 3772,
        "candidate_size": 3772,
        "diff_rows": 17,
        "diff_kinds": {"DIFF_ARG_MISMATCH": 17},
        "exact": False,
    }
    return {
        "schema": "focus_symbol_report/v1",
        "artifact_sha256": "10" * 32,
        "authority_advanced": False,
        "function": FUNCTION,
        "channels": {
            "strict": {
                "target": _side(target),
                "candidate": _side(candidate),
                "metric": metric,
            },
            "data": {
                "target": _side(data_target, diff_only=True),
                "candidate": _side(data_candidate, diff_only=True),
                "metric": {**metric, "diff_rows": 10},
            },
        },
    }


def context() -> dict[str, object]:
    return {
        "schema": matcher.CONTEXT_SCHEMA,
        "function": FUNCTION,
        "focus_artifact_sha256": "10" * 32,
        "report_sha256": "20" * 32,
        "source_sha256": "30" * 32,
        "candidate_object_sha256": "40" * 32,
        "target_object_sha256": "50" * 32,
        "protected_siblings_zero_loss": True,
        "physical_relocations": {"status": "unknown"},
        "owners": [
            {
                "id": "doneF",
                "kind": "stack_home",
                "source_type": "BOOL",
                "candidate_tokens": ["stack:0x30"],
                "row_indices": [284],
                "declaration_line": 501,
                "definition_lines": [603],
                "use_lines": [],
                "call_boundaries": ["post_PauseCursorHiliteSet"],
                "write_only_target_observed": True,
            },
            {
                "id": "oldValue",
                "kind": "stack_home",
                "source_type": "s32",
                "candidate_tokens": ["stack:0x38"],
                "row_indices": [720],
                "declaration_line": 498,
                "definition_lines": [789],
                "use_lines": [801],
                "call_boundaries": [],
                "write_only_target_observed": False,
            },
            {
                "id": "value",
                "kind": "stack_home",
                "source_type": "s32",
                "candidate_tokens": ["stack:0x34"],
                "row_indices": [823, 824, 831],
                "declaration_line": 499,
                "definition_lines": [815],
                "use_lines": [816, 817],
                "call_boundaries": [],
                "write_only_target_observed": False,
            },
            {
                "id": "vibrateF",
                "kind": "register_flow",
                "source_type": "BOOL",
                "candidate_tokens": ["register-flow:861-863"],
                "row_indices": [861, 862, 863],
                "declaration_line": 502,
                "definition_lines": [831],
                "use_lines": [833, 834],
                "call_boundaries": ["pre_HuPadRumbleAllStop"],
                "write_only_target_observed": False,
            },
        ],
        "semantic_groups": [
            {
                "kind": "pool_owner",
                "row_indices": list(POOL_ROWS),
                "action": "Bind the five target-named float owners at their existing consumers.",
                "evidence": "Seven SDA21 rows reduce to five unique target pool owners.",
            },
            {
                "kind": "immediate_semantic",
                "row_indices": [563],
                "action": "Use the target-proven FLAG_BOARD_TURN_NOSTART identifier.",
                "evidence": "The target immediate is 0x10021 and the candidate immediate is 0x10020.",
            },
            {
                "kind": "branch_topology",
                "row_indices": [714],
                "action": "Keep the Batsu update inside the valueMax guard.",
                "evidence": "The target and candidate differ only in the branch destination.",
            },
        ],
    }


class OwnerFlowMatcherTests(unittest.TestCase):
    def test_configexec_c196_maps_all_rows_and_emits_two_cells(self) -> None:
        result = matcher.build_diagnosis(focus_artifact(), context())

        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["route"], matcher.ROUTE)
        self.assertEqual(result["facts"]["strict_diff_row_count"], 17)
        self.assertEqual(result["facts"]["data_diff_row_count"], 10)
        self.assertEqual(result["facts"]["classification_counts"]["pool_owner"], 7)
        self.assertEqual(result["facts"]["classification_counts"]["stack_home"], 5)
        self.assertEqual(result["facts"]["classification_counts"]["register_flow"], 3)
        self.assertEqual(result["facts"]["owner_mapping_count"], 4)
        self.assertTrue(result["facts"]["all_strict_rows_accounted"])
        self.assertEqual(result["owner_cycles"], [["doneF", "oldValue", "value"]])
        self.assertEqual([cell["id"] for cell in result["candidate_cells"]], [
            "close_owner_flow_cycles",
            "compose_remaining_semantic_closure",
        ])
        self.assertEqual(result["compile_candidate_budget"], 2)
        self.assertEqual(result["trace_budget"], 0)
        self.assertIn("physical_relocation_authority_unknown", result["warnings"])
        self.assertFalse(result["source_patch_emitted"])
        self.assertFalse(result["authority_advanced"])

    def test_fails_closed_on_unclassified_row(self) -> None:
        focus = focus_artifact()
        target_rows = focus["channels"]["strict"]["target"]["rows"]
        row = next(item for item in target_rows if item["index"] == 563)
        row["instruction"]["formatted"] = "xor r3, r4, r5"
        result = matcher.build_diagnosis(focus, context())
        self.assertEqual(result["status"], "blocked")
        self.assertTrue(any(blocker.startswith("row_563_unclassified") for blocker in result["blockers"]))
        self.assertEqual(result["candidate_cells"], [])

    def test_fails_closed_when_owner_assignment_is_not_unique(self) -> None:
        value = context()
        value["owners"][0]["candidate_tokens"] = []
        value["owners"][0]["row_indices"] = [284, 720]
        value["owners"][1]["candidate_tokens"] = []
        value["owners"][1]["row_indices"] = [284, 720]
        result = matcher.build_diagnosis(focus_artifact(), value)
        self.assertEqual(result["status"], "blocked")
        self.assertTrue(any("assignment_ambiguous" in blocker for blocker in result["blockers"]))

    def test_fails_closed_when_semantic_group_is_incomplete(self) -> None:
        value = context()
        value["semantic_groups"][0]["row_indices"].remove(650)
        result = matcher.build_diagnosis(focus_artifact(), value)
        self.assertEqual(result["status"], "blocked")
        self.assertIn("semantic_group_rows_differ:pool_owner", result["blockers"])

    def test_diagnosis_hash_is_canonical(self) -> None:
        result = matcher.build_diagnosis(focus_artifact(), context())
        digest = result["diagnosis_sha256"]
        unhashed = copy.deepcopy(result)
        unhashed.pop("diagnosis_sha256")
        self.assertEqual(digest, owner_manifest.canonical_sha256(unhashed))


if __name__ == "__main__":
    unittest.main()
