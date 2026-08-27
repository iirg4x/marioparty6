#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import unittest

from tools import mismatch_cluster_audit as reducer
from tools import crack_learning_rules as rules
from tools import mwcc_score_delta_reducer as score_delta


FUNCTION = "PauseGuideMain"
OWNER = "weight"
REPORT_SHA = "a75123db425fd9395400de8709bdc68411106ff71ba68629ed39e8bff885338f"
BASELINE_REPORT = "a91fdb6fecee5f1fd1fbb6124a81aa7e2a85a9335ca1d8df8ff51ac6dd8da1b2"
CONTROL_REPORT = "5573e52c7788583505bb8fd9ed4aab13afd0c0d2ec5c0e1fd46027f2d974326b"
EXACT_REPORT = "4f5583d6952618295f5d853e28a12449a6ac137b7fb3b3ba88e1ad0cc8666e2f"
SCORE_ROWS = [
    287, 294, 304, 367, 371, 373, 375, 384, 388, 391, 397, 421,
    422, 423, 424, 427, 428, 439, 442, 445, 448, 455, 456,
]
OTHER_ROWS = [
    55, 76, 122, 123, 129, 160, 172, 184, 237, 248, 284, 309,
    333, 334, 370, 372, 374, 382, 383, 386, 387, 390, 396, 418,
    438, 485,
]


def _hash(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def _instruction(
    index: int,
    *,
    frame: int,
    residual_rows: set[int],
    relocation: bool,
) -> reducer.Instruction:
    formatted = f"stwu r1, -0x{frame:x}(r1)" if index == 0 else "fmr f1, f2"
    return reducer.Instruction(
        index=index,
        diff_kind="DIFF_ARG_MISMATCH" if index in residual_rows else None,
        address=0x100 + index * 4,
        size=4,
        formatted=formatted,
        mnemonic=formatted.split()[0],
        relocation=(
            {"type_name": "R_PPC_EMB_SDA21", "target_symbol": index + 1}
            if relocation
            else None
        ),
        branch_dest=None,
        has_instruction=True,
        raw={},
    )


def _instructions(residual_rows: list[int]) -> list[reducer.Instruction]:
    marked = set(residual_rows)
    return [
        _instruction(index, frame=0x1A0, residual_rows=marked, relocation=index < 89)
        for index in range(486)
    ]


def _context() -> dict[str, object]:
    retained_source = "6402eee5a5ab5aeed329801f1752e032223059936e609199bd4f74f81bf7ae34"
    source_text = "weight = weight;"
    return {
        "schema": score_delta.CONTEXT_SCHEMA,
        "report_artifact_sha256": REPORT_SHA,
        "focus": {"function": FUNCTION, "owner": OWNER, "bank": "FPR"},
        "baseline": {
            "candidate_id": "config-full-owner-c096",
            "objdiff_canonical_sha256": BASELINE_REPORT,
            "source_sha256": "93fe5dc8fc1e211673044f26582ab27f65f7d476182dcc03c4854b98f0be1342",
            "object_sha256": "56a435950a5e42e379db9564911302088b897b4a7c3dd72a8520070b716f988e",
            "strict_report_sha256": "5b20c765cc24e06d206e8d9e126b374637e435074a74f509e8203c9b2dc47fd0",
            "data_report_sha256": "5b1f2f4563884bc45c3b3a44f6a7a155c89bc2fe4c74385e63afbd83d77cc43b",
            "target_bytes": 2180,
            "candidate_bytes": 2180,
            "target_frame": 0x1A0,
            "candidate_frame": 0x1A0,
            "match_percent": 99.70,
            "target_physical_relocations": 89,
            "candidate_physical_relocations": 89,
            "score_residual_rows": SCORE_ROWS,
            "other_residual_rows": OTHER_ROWS,
            "instruction_count_exact": True,
            "operation_order_exact": True,
            "cfg_calls_exact": True,
            "frame_exact": True,
            "physical_relocation_topology_exact": True,
            "protected_siblings_preserved": True,
        },
        "retained_control": {
            "candidate_id": "config-full-owner-c102",
            "objdiff_canonical_sha256": CONTROL_REPORT,
            "source_sha256": retained_source,
            "object_sha256": "5d01189b0852640f21c858b9dbff7072488df0b3d8dc3472509dc85c480df5dd",
            "strict_report_sha256": "c984bf3ad216b86a5c82e2c8bb1d7c44f50e15f50a4151fb766e8983ceda060b",
            "data_report_sha256": "3e4166d71936be3c18a0c829063535cc0e4cf0e276b4387c992c29a5a1448e61",
            "target_bytes": 2180,
            "candidate_bytes": 2180,
            "match_percent": 99.90,
            "physical_relocations": 89,
            "allocation_rows_removed": len(SCORE_ROWS),
            "remaining_non_score_rows": len(OTHER_ROWS),
            "new_instruction_rows": 0,
            "new_call_rows": 0,
            "new_store_rows": 0,
            "new_branch_rows": 0,
            "codegen_neutral_after_allocation": True,
            "protected_siblings_preserved": True,
        },
        "exact_result": {
            "candidate_id": "config-full-owner-c109",
            "objdiff_canonical_sha256": EXACT_REPORT,
            "source_sha256": "76cfc28aab515075e5e8b935619fc5c6adc5f9c217f1e5b0d0252b651fd30946",
            "object_sha256": "312bdc9e28970fdc33f9cf2970c067b4509fd0db2f88705e35f68cb4f3ceddd8",
            "strict_report_sha256": "91698007465222ff5976694d5071a621173cae430c2d9c5634e9fb18faf32975",
            "data_report_sha256": "97af8d660dc5d6d316ad22e7fea1fc4e8562b7f6d52a3659790d72e052634e7a",
            "candidate_record_sha256": "c5bead63b11ebe99eb47a837a26a0105272446c8e77e63e72cc344dc8ff2e25a",
            "target_bytes": 2180,
            "candidate_bytes": 2180,
            "physical_relocations": 89,
            "strict_percent": 100.0,
            "data_percent": 100.0,
            "zero_diff_rows": True,
            "protected_siblings_preserved": True,
        },
        "trace_comparison": {
            "comparison_sha256": "0c366d89a46e5a5f38f070c72a5fbf8d8cb675a3d8947e8720615958c8f0d14a",
            "baseline": {
                "label": "c096",
                "trace_sha256": "59cbdbaf5f7ffb30ff0e3ed4d9dfa776f71346dd1c45e40ad752ccbc2e991783",
                "envelope_sha256": "566e18a0d03906bf167634fe29dd5fb04738d8397946a96b6beca7cf707356c1",
                "allocation_score": 4,
                "bank": "FPR",
                "physical_register": "f23",
            },
            "retained": {
                "label": "c102",
                "trace_sha256": "85b1507cbd4bb26671d1287405d45da9eefc194b1c85de48bc99759603ac311c",
                "envelope_sha256": "7d767911762af3b0081befa186946311d38413fb3906365db5604b85c5df2b31",
                "allocation_score": 10,
                "bank": "FPR",
                "physical_register": "f27",
            },
        },
        "def_use_pairs": [
            {
                "pair_id": f"weight-def-use-{line}",
                "source_sha256": retained_source,
                "start_line": line,
                "end_line": line,
                "source_text": source_text,
                "source_text_sha256": _hash(source_text),
                "definition_increment": 1,
                "use_increment": 1,
            }
            for line in (1243, 1244, 1245)
        ],
        "telemetry": {
            "candidate_count": 109,
            "tracer_runs": 2,
            "donor_searches": 1,
            "telemetry_complete": False,
            "interval_log_sha256": "823e6a90f49f43b8a13cd49707d2fb7e918c84c74ab42d06ea70d6a7bbcb5aed",
        },
        "authority_advanced": False,
    }


class MwccScoreDeltaReducerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.context = score_delta.parse_context(_context())
        self.pair = reducer.FunctionPair(
            name=FUNCTION,
            target={"size": "2180"},
            candidate={"size": "2180", "match_percent": 99.70},
        )
        rows = sorted(SCORE_ROWS + OTHER_ROWS)
        self.target = _instructions(rows)
        self.candidate = _instructions(rows)

    def test_pauseguide_replay_binds_minimal_source_delta(self) -> None:
        result = score_delta.evaluate(
            self.pair, self.target, self.candidate, self.context, BASELINE_REPORT
        )
        self.assertTrue(result["matched"])
        evidence = result["evidence"]
        self.assertEqual(evidence["trace_comparison"]["score_delta"], 6)
        self.assertEqual(
            evidence["trace_comparison"]["physical_color_change"],
            {"from": "f23", "to": "f27"},
        )
        cell = evidence["recommended_cells"][0]
        self.assertEqual(cell["minimal_def_use_pair_count"], 3)
        self.assertEqual([span["start_line"] for span in cell["source_spans"]], [1243, 1244, 1245])
        self.assertFalse(cell["retention_authorized"])
        self.assertFalse(evidence["authority_advanced"])

    def test_exact_and_retained_reports_suppress_retries(self) -> None:
        exact = score_delta.evaluate(
            self.pair, self.target, self.candidate, self.context, EXACT_REPORT
        )
        self.assertFalse(exact["matched"])
        self.assertIn("already exact", exact["reason"])
        retained = score_delta.evaluate(
            self.pair, self.target, self.candidate, self.context, CONTROL_REPORT
        )
        self.assertFalse(retained["matched"])
        self.assertIn("already present", retained["reason"])

    def test_dispatcher_wrapper_preserves_rule_and_schema(self) -> None:
        parsed = rules._parse_mwcc_score_delta_context(_context())
        result = rules._mwcc_score_delta_evaluation(
            self.pair, self.target, self.candidate, parsed, BASELINE_REPORT
        )
        self.assertTrue(result["matched"])
        self.assertEqual(result["rule_id"], score_delta.RULE_ID)
        self.assertEqual(rules.SCHEMA, "crack_learning_diagnosis/v36")

    def test_missing_or_unbound_increment_fails_closed(self) -> None:
        context = _context()
        context["def_use_pairs"] = context["def_use_pairs"][:-1]
        with self.assertRaisesRegex(
            score_delta.ScoreDeltaInputError, "do not minimally explain"
        ):
            score_delta.parse_context(context)
        context = _context()
        context["def_use_pairs"][0]["source_text"] = "weight += 0.0f;"
        with self.assertRaisesRegex(
            score_delta.ScoreDeltaInputError, "exact owner self-assignment"
        ):
            score_delta.parse_context(context)

    def test_signature_or_row_drift_suppresses_diagnosis(self) -> None:
        drifted = copy.deepcopy(self.candidate)
        drifted[0] = _instruction(
            0, frame=0x190, residual_rows=set(SCORE_ROWS + OTHER_ROWS), relocation=True
        )
        result = score_delta.evaluate(
            self.pair, self.target, drifted, self.context, BASELINE_REPORT
        )
        self.assertFalse(result["matched"])
        self.assertIn("signature drifted", result["reason"])
        rows = sorted(SCORE_ROWS + OTHER_ROWS[:-1])
        result = score_delta.evaluate(
            self.pair, _instructions(rows), _instructions(rows), self.context, BASELINE_REPORT
        )
        self.assertFalse(result["matched"])
        self.assertIn("residual no longer matches", result["reason"])


if __name__ == "__main__":
    unittest.main()
