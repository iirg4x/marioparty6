from __future__ import annotations

import contextlib
import io
import json
import re
import tempfile
import unittest
from pathlib import Path

from tools import candidate_interaction_planner as planner
from tools import crack_learning_rules as rules


def _instruction(
    address: int,
    formatted: str,
    *,
    diff_kind: str | None = None,
    branch_dest: int | None = None,
    relocation: dict[str, object] | None = None,
) -> dict[str, object]:
    nested: dict[str, object] = {
        "address": str(address),
        "size": 4,
        "formatted": formatted,
    }
    if branch_dest is not None:
        nested["branch_dest"] = str(branch_dest)
    if relocation is not None:
        nested["relocation"] = relocation
    row: dict[str, object] = {"instruction": nested}
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _placeholder(diff_kind: str = "DIFF_DELETE") -> dict[str, object]:
    return {"diff_kind": diff_kind}


def _function(
    name: str,
    instructions: list[dict[str, object]],
    *,
    size: int | None = None,
    match_percent: float = 90.0,
) -> dict[str, object]:
    return {
        "name": name,
        "kind": "SYMBOL_FUNCTION",
        "address": "100",
        "size": str(size if size is not None else len(instructions) * 4),
        "match_percent": match_percent,
        "instructions": instructions,
    }


def _report(
    focus: str,
    target: list[dict[str, object]],
    candidate: list[dict[str, object]],
    *,
    target_size: int | None = None,
    candidate_size: int | None = None,
    extra_pairs: tuple[tuple[dict[str, object], dict[str, object]], ...] = (),
) -> dict[str, object]:
    return {
        "left": {
            "symbols": [
                _function(focus, target, size=target_size),
                *(pair[0] for pair in extra_pairs),
            ]
        },
        "right": {
            "symbols": [
                _function(focus, candidate, size=candidate_size),
                *(pair[1] for pair in extra_pairs),
            ]
        },
    }


def _evaluation(result: dict[str, object], rule_id: str) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == rule_id
    )


def _assignment_cycle_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -32(r1)",
        "mr r29, r4",
        "mr r30, r5",
        "mr r31, r6",
        "bl get_result",
        "mr r29, r3",
        "cmpwi r29, 0",
        "bne 0x84",
        "mr r3, r30",
        "mr r4, r31",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -32(r1)",
        "mr r30, r4",
        "mr r31, r5",
        "mr r29, r6",
        "bl get_result",
        "mr r30, r3",
        "cmpwi r30, 0",
        "bne 0x84",
        "mr r3, r31",
        "mr r4, r29",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        address = 100 + index * 4
        branch_dest = 132 if left.startswith("bne") else None
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target.append(
            _instruction(address, left, diff_kind=kind, branch_dest=branch_dest)
        )
        candidate.append(
            _instruction(address, right, diff_kind=kind, branch_dest=branch_dest)
        )
    return _report("ev_CapMiracleCoinTrade", target, candidate)


def _allocator_swap_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -64(r1)",
        "mr r27, r3",
        "mr r26, r4",
        "addi r3, r27, 1",
        "addi r4, r26, -1",
        "bl mbDiceExec",
        "mr r26, r3",
        "add r3, r27, r26",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -64(r1)",
        "mr r26, r3",
        "mr r27, r4",
        "addi r3, r26, 1",
        "addi r4, r27, -1",
        "bl mbDiceExec",
        "mr r27, r3",
        "add r3, r26, r27",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        address = 100 + index * 4
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target.append(_instruction(address, left, diff_kind=kind))
        candidate.append(_instruction(address, right, diff_kind=kind))
    return _report(
        "mbev_CapKuribo",
        target,
        candidate,
        target_size=2612,
        candidate_size=2612,
    )


def _allocator_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _allocator_swap_report()
    selections = [
        ("control", "existing", "split", "1"),
        ("declaration-only", "long-lived-first", "split", "2"),
        ("boundary-only", "existing", "fused", "3"),
    ]
    return {
        "schema": rules.ALLOCATOR_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "stack_frame_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "varinfo_receipt_sha256": "4" * 64,
            "source_boundary_receipt_sha256": "5" * 64,
        },
        "owners": [
            {
                "name": "playerNo",
                "usage_class": 13,
                "target_register": "r27",
                "candidate_register": "r26",
                "lifetime_role": "long_lived",
                "evidence_sha256": "6" * 64,
            },
            {
                "name": "diceValue",
                "usage_class": 14,
                "target_register": "r26",
                "candidate_register": "r27",
                "lifetime_role": "producer_consumer_boundary",
                "evidence_sha256": "7" * 64,
            },
        ],
        "boundary": {
            "producer": "mbDiceExec return value",
            "consumer": "kuriboCoinTbl indexed load",
            "transformations": ["subtract one", "table lookup"],
            "evidence_sha256": "8" * 64,
        },
        "observations": [
            {
                "selection": {
                    "declaration_chronology": declaration,
                    "value_identity_boundary": boundary,
                },
                "candidate_id": candidate_id,
                "source_sha256": source_digit * 64,
                "object_sha256": "d" * 64,
            }
            for candidate_id, declaration, boundary, source_digit in selections
        ],
    }


def _parameter_allocation_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -48(r1)",
        "mr r29, r3",
        "slwi r5, r29, 2",
        "bl HuMemDirectMallocNum",
        "mr r28, r3",
        "stw r28, 92(r30)",
        "mr r31, r28",
        "stw r29, 0(r31)",
        "mr r3, r29",
        "bl mbPlayerColSnapPlayerSet",
        "lfs f0, lbl_802C476C@sda21",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -48(r1)",
        "mr r28, r3",
        "slwi r5, r28, 2",
        "bl HuMemDirectMallocNum",
        "mr r29, r3",
        "stw r29, 92(r30)",
        "mr r31, r29",
        "stw r28, 0(r31)",
        "mr r3, r28",
        "bl mbPlayerColSnapPlayerSet",
        "lfs f0, @1326@sda21",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        address = 100 + index * 4
        kind = "DIFF_ARG_MISMATCH" if left != right and "@sda21" not in left else None
        relocation = (
            {"type": 109, "type_name": "R_PPC_EMB_SDA21"} if "@sda21" in left else None
        )
        target.append(
            _instruction(address, left, diff_kind=kind, relocation=relocation)
        )
        candidate.append(
            _instruction(address, right, diff_kind=kind, relocation=relocation)
        )
    return _report(
        "mbev_CapPlayerMoveEjectCreate",
        target,
        candidate,
        target_size=288,
        candidate_size=288,
    )


def _parameter_allocation_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _parameter_allocation_report()
    return {
        "schema": rules.PARAMETER_ALLOCATION_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "stack_frame_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "trace_receipt_sha256": "4" * 64,
            "source_boundary_receipt_sha256": "5" * 64,
            "same_tu_donor_receipt_sha256": "6" * 64,
        },
        "owners": {
            "parameter": {
                "name": "playerNo",
                "target_register": "r29",
                "candidate_register": "r28",
                "evidence_sha256": "7" * 64,
            },
            "allocation_result": {
                "name": "workData",
                "target_register": "r28",
                "candidate_register": "r29",
                "evidence_sha256": "8" * 64,
            },
        },
        "producer": {
            "call_name": "HuMemDirectMallocNum",
            "call_row": 3,
            "capture_row": 4,
            "return_register": "r3",
            "preserve_explicit_identity": True,
            "evidence_sha256": "9" * 64,
        },
        "consumer_chain": {
            "typed_pointer": "workP",
            "field_owner": "obj",
            "field_name": "data",
            "allocation_result": "workData",
            "evaluation_order": ["field_store", "typed_pointer_copy"],
            "consumer_rows": [5, 6],
            "evidence_sha256": "a" * 64,
        },
    }


def _capacity_report() -> dict[str, object]:
    instructions = [
        _instruction(100, "stwu r1, -720(r1)"),
        _instruction(104, "addi r3, r1, 224"),
        _instruction(108, "bl consume_capsules"),
        _instruction(112, "blr"),
    ]
    return _report(
        "mbev_CapKokamekku",
        instructions,
        instructions,
        target_size=6424,
        candidate_size=6424,
    )


def _capacity_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _capacity_report()
    return {
        "schema": rules.CAPACITY_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "all_non_extent_structure_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "stack_extent_receipt_sha256": "4" * 64,
            "interface_contract_receipt_sha256": "5" * 64,
        },
        "array": {
            "name": "capsuleObjId",
            "element_size": 4,
            "candidate_capacity": 3,
            "used_prefix_elements": 3,
            "candidate_extent_bytes": 12,
            "target_extent_bytes": 20,
        },
        "producer_contracts": [
            {
                "provider": "mbPlayerCapsuleMaxGet",
                "source_location": "game/src/board/player.c:L3838-L3841",
                "maximum": 5,
                "evidence_sha256": "6" * 64,
            },
            {
                "provider": "mbPlayerCapsuleNumGet",
                "source_location": "game/src/board/player.c:L3942-L3954",
                "maximum": 5,
                "evidence_sha256": "7" * 64,
            },
        ],
        "declaration_positions": [
            "before_moveDir",
            "after_moveDir",
            "after_next_aggregate",
        ],
    }


def _loop_branch_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -720(r1)",
        "cmpwi r3, 0",
        "beq 0x98",
        "bl movement_body",
        "addi r27, r27, 1",
        "cmpw r27, r30",
        "blt 0x74",
        "bl post_path_sleep",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -720(r1)",
        "cmpwi r3, 0",
        "beq 0x84",
        "bl movement_body",
        "addi r27, r27, 1",
        "cmpw r27, r30",
        "blt 0x74",
        "bl post_path_sleep",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        address = 100 + index * 4
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target_dest = 152 if index == 2 else (116 if index == 6 else None)
        candidate_dest = 132 if index == 2 else (116 if index == 6 else None)
        target.append(
            _instruction(address, left, diff_kind=kind, branch_dest=target_dest)
        )
        candidate.append(
            _instruction(address, right, diff_kind=kind, branch_dest=candidate_dest)
        )
    return _report(
        "mbev_CapKokamekku",
        target,
        candidate,
        target_size=6424,
        candidate_size=6424,
    )


def _branch_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _loop_branch_report()
    return {
        "schema": rules.BRANCH_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "stack_frame_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "all_non_branch_rows_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "branch_destination_receipt_sha256": "4" * 64,
        },
        "branch": {
            "row_index": 2,
            "guard_class": "zero_terminator",
            "target_destination": "loop_exit",
            "candidate_destination": "loop_increment",
            "target_relative_target": 44,
            "candidate_relative_target": 24,
        },
    }


def _reciprocal_report() -> dict[str, object]:
    sda_target_a = {"type_name": "SDA21", "target_name": "lbl_alpha"}
    sda_candidate_a = {"type_name": "SDA21", "target_name": "@alpha"}
    sda_target_b = {"type_name": "SDA21", "target_name": "lbl_one"}
    sda_candidate_b = {"type_name": "SDA21", "target_name": "@one"}
    sda_target_recip = {"type_name": "SDA21", "target_name": "lbl_recip"}
    sda_candidate_recip = {"type_name": "SDA21", "target_name": "@recip"}
    target = [
        _instruction(100, "stwu r1, -64(r1)"),
        _instruction(
            104,
            "lfs f3, lbl_alpha@sda21",
            diff_kind="DIFF_ARG_MISMATCH",
            relocation=sda_target_a,
        ),
        _instruction(
            108,
            "lfs f2, lbl_one@sda21",
            diff_kind="DIFF_ARG_MISMATCH",
            relocation=sda_target_b,
        ),
        _instruction(112, "lfs f1, 56(r31)", diff_kind="DIFF_REPLACE"),
        _instruction(
            116,
            "lfs f0, lbl_recip@sda21",
            diff_kind="DIFF_REPLACE",
            relocation=sda_target_recip,
        ),
        _instruction(120, "fmuls f0, f1, f0"),
        _instruction(124, "fsubs f0, f2, f0"),
        _instruction(128, "fmuls f1, f3, f0"),
        _instruction(132, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -64(r1)"),
        _instruction(
            104,
            "lfs f3, @alpha@sda21",
            diff_kind="DIFF_ARG_MISMATCH",
            relocation=sda_candidate_a,
        ),
        _instruction(
            108,
            "lfs f2, @one@sda21",
            diff_kind="DIFF_ARG_MISMATCH",
            relocation=sda_candidate_b,
        ),
        _instruction(
            112,
            "lfs f1, @recip@sda21",
            diff_kind="DIFF_REPLACE",
            relocation=sda_candidate_recip,
        ),
        _instruction(116, "lfs f0, 56(r31)", diff_kind="DIFF_REPLACE"),
        _instruction(120, "fmuls f0, f1, f0"),
        _instruction(124, "fsubs f0, f2, f0"),
        _instruction(128, "fmuls f1, f3, f0"),
        _instruction(132, "blr"),
    ]
    return _report(
        "mbev_CapEffExplodeOMExec",
        target,
        candidate,
        target_size=524,
        candidate_size=524,
    )


def _reciprocal_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _reciprocal_report()
    return {
        "schema": rules.RECIPROCAL_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "all_non_window_rows_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "typed_constant_receipt_sha256": "4" * 64,
            "neutral_observation_receipt_sha256": "5" * 64,
        },
        "window": {
            "invariant_constant_rows": [1, 2],
            "target_variable_row": 3,
            "candidate_variable_row": 4,
            "target_reciprocal_row": 4,
            "candidate_reciprocal_row": 3,
            "multiply_row": 5,
            "denominator": 16,
            "reciprocal_f32_bits": "3d800000",
        },
        "neutral_observation": {
            "axis": "commuted_multiply",
            "baseline_object_sha256": "6" * 64,
            "candidate_object_sha256": "6" * 64,
        },
    }


def _switch_fpr_report(*, include_switch: bool = True) -> dict[str, object]:
    target = [
        _instruction(100, "stwu r1, -160(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "mflr r0"),
        _instruction(108, "stfd f26, 96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "stfd f25, 104(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bctr" if include_switch else "nop"),
        _instruction(120, "bl sin"),
        _instruction(124, "fmr f26, f1", diff_kind="DIFF_DELETE"),
        _instruction(128, "nop"),
        _instruction(132, "bl sin"),
        _instruction(136, "fmr f25, f1", diff_kind="DIFF_DELETE"),
        _instruction(140, "nop"),
        _instruction(144, "bl cos"),
        _instruction(148, "fmr f24, f1", diff_kind="DIFF_DELETE"),
        _instruction(152, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "mflr r0"),
        _instruction(108, "stfd f26, 32(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "stfd f25, 40(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bctr" if include_switch else "nop"),
        _instruction(120, "bl sin"),
        _placeholder(),
        _instruction(128, "nop"),
        _instruction(132, "bl sin"),
        _placeholder(),
        _instruction(140, "nop"),
        _instruction(144, "bl cos"),
        _placeholder(),
        _instruction(152, "blr"),
    ]
    return _report(
        "ev_CapBobleOMExec",
        target,
        candidate,
        target_size=2000,
        candidate_size=1928,
    )


def _aggregate_report(*, donor_exact: bool = True) -> dict[str, object]:
    prefix = [_instruction(100 + index * 4, "nop") for index in range(12)]
    copy = [
        _instruction(148, "lfs f0, 32(r1)", diff_kind="DIFF_DELETE"),
        _instruction(152, "lfs f1, 36(r1)", diff_kind="DIFF_DELETE"),
        _instruction(156, "lfs f2, 40(r1)", diff_kind="DIFF_DELETE"),
        _instruction(160, "stfs f0, 32(r1)", diff_kind="DIFF_DELETE"),
        _instruction(164, "stfs f1, 36(r1)", diff_kind="DIFF_DELETE"),
        _instruction(168, "stfs f2, 40(r1)", diff_kind="DIFF_DELETE"),
    ]
    consumers = [
        _instruction(172, "bl mbPlayerPosSetV"),
        _instruction(176, "bl mbPlayerRotSetV"),
        _instruction(180, "blr"),
    ]
    focus_target = [*prefix, *copy, *consumers]
    focus_candidate = [
        *prefix,
        *(_placeholder() for _ in copy),
        *consumers,
    ]
    donor_copy = [
        _instruction(148, "lfs f0, 48(r1)"),
        _instruction(152, "lfs f1, 52(r1)"),
        _instruction(156, "lfs f2, 56(r1)"),
        _instruction(160, "stfs f0, 48(r1)"),
        _instruction(164, "stfs f1, 52(r1)"),
        _instruction(168, "stfs f2, 56(r1)"),
    ]
    donor_instructions = [*prefix, *donor_copy, *consumers]
    percent = 100.0 if donor_exact else 99.0
    donor_pair = (
        _function("mbev_CapBobleMove", donor_instructions, match_percent=percent),
        _function("mbev_CapBobleMove", donor_instructions, match_percent=percent),
    )
    return _report(
        "mbev_CapBomheiMove",
        focus_target,
        focus_candidate,
        extra_pairs=(donor_pair,),
    )


class CrackLearningRulesTest(unittest.TestCase):
    def test_explicit_else_return_reuses_causal_reducer_signature(self) -> None:
        target = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(
                104,
                "bne 0x74",
                diff_kind="DIFF_ARG_MISMATCH",
                branch_dest=116,
            ),
            _instruction(108, "bl body"),
            _instruction(112, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            _instruction(116, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            _instruction(120, "blr"),
        ]
        candidate = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(
                104,
                "bne 0x78",
                diff_kind="DIFF_ARG_MISMATCH",
                branch_dest=120,
            ),
            _instruction(108, "bl body"),
            _placeholder(),
            _placeholder(),
            _instruction(120, "blr"),
        ]
        result = rules.diagnose_document(
            _report("ev_CapTeresaFadeMatHook", target, candidate),
            focus_symbol="ev_CapTeresaFadeMatHook",
        )
        diagnosis = _evaluation(result, "explicit_else_return_cfg")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["confidence"], 0.98)
        self.assertEqual(
            diagnosis["evidence"]["causal_classification"],  # type: ignore[index]
            "explicit_else_return_epilogue",
        )

    def test_assignment_in_condition_requires_closed_saved_gpr_cycle(self) -> None:
        result = rules.diagnose_document(
            _assignment_cycle_report(), focus_symbol="ev_CapMiracleCoinTrade"
        )
        diagnosis = _evaluation(result, "assignment_condition_saved_gpr_cycle")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],  # type: ignore[index]
            {"r29": "r30", "r30": "r31", "r31": "r29"},
        )
        self.assertEqual(
            len(diagnosis["evidence"]["call_result_consumers"]),  # type: ignore[index]
            1,
        )

    def test_allocator_two_register_swap_emits_only_missing_interaction(self) -> None:
        report = _allocator_swap_report()
        context = _allocator_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapKuribo",
            allocator_context=context,
        )
        diagnosis = _evaluation(result, "allocator_two_register_swap_interaction")
        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(
            evidence["register_mapping"],
            {"r26": "r27", "r27": "r26"},
        )
        self.assertEqual(evidence["observed_selection_count"], 3)
        self.assertEqual(
            evidence["missing_selections"],
            [
                {
                    "declaration_chronology": "long-lived-first",
                    "value_identity_boundary": "fused",
                }
            ],
        )

        with tempfile.TemporaryDirectory() as directory:
            request_path = Path(directory) / "request.json"
            request_path.write_text(
                json.dumps(evidence["interaction_request"]), encoding="utf-8"
            )
            plan = planner.build_interaction_plan(request_path)
        self.assertEqual(plan["summary"]["raw_cell_count"], 4)
        self.assertEqual(plan["summary"]["observed_cell_count"], 3)
        self.assertEqual(plan["summary"]["generate_and_compile_count"], 1)
        runnable = [
            cell for cell in plan["cells"] if cell["action"] == "generate_and_compile"
        ]
        self.assertEqual(
            runnable[0]["selection"],
            {
                "declaration_chronology": "long-lived-first",
                "value_identity_boundary": "fused",
            },
        )

    def test_allocator_two_register_swap_fails_closed_on_unproved_context(self) -> None:
        no_context = rules.diagnose_document(
            _allocator_swap_report(), focus_symbol="mbev_CapKuribo"
        )
        self.assertFalse(
            _evaluation(no_context, "allocator_two_register_swap_interaction")[
                "matched"
            ]
        )

        false_proof = _allocator_context()
        false_proof["proofs"]["data_values_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "data_values_exact"):
            rules.diagnose_document(
                _allocator_swap_report(),
                focus_symbol="mbev_CapKuribo",
                allocator_context=false_proof,
            )

        wrong_owner = _allocator_context()
        wrong_owner["owners"][0]["target_register"] = "r25"  # type: ignore[index]
        result = rules.diagnose_document(
            _allocator_swap_report(),
            focus_symbol="mbev_CapKuribo",
            allocator_context=wrong_owner,
        )
        self.assertFalse(
            _evaluation(result, "allocator_two_register_swap_interaction")["matched"]
        )

        wrong_report = _allocator_context()
        wrong_report["proofs"]["objdiff_canonical_sha256"] = "0" * 64  # type: ignore[index]
        result = rules.diagnose_document(
            _allocator_swap_report(),
            focus_symbol="mbev_CapKuribo",
            allocator_context=wrong_report,
        )
        self.assertFalse(
            _evaluation(result, "allocator_two_register_swap_interaction")["matched"]
        )

        uppercase_hash = _allocator_context()
        uppercase_hash["proofs"]["strict_report_sha256"] = "A" * 64  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "lowercase"):
            rules.diagnose_document(
                _allocator_swap_report(),
                focus_symbol="mbev_CapKuribo",
                allocator_context=uppercase_hash,
            )

    def test_parameter_allocation_swap_ranks_consumer_chain_only(self) -> None:
        report = _parameter_allocation_report()
        context = _parameter_allocation_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
            parameter_allocation_context=context,
        )
        diagnosis = _evaluation(result, "parameter_allocation_consumer_chain")
        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(
            evidence["register_mapping"],
            {"r28": "r29", "r29": "r28"},
        )
        self.assertEqual(evidence["source_expression"], "workP = obj->data = workData")
        self.assertEqual(
            evidence["physical_boundary"]["target_capture"],  # type: ignore[index]
            "mr r28, r3",
        )
        self.assertEqual(
            [item["axis"] for item in evidence["suppressed_axes"]],
            ["parameter_declaration_chronology", "producer_elimination"],
        )
        self.assertIn("preserve", diagnosis["recommendation"])
        self.assertIn("suppress", diagnosis["recommendation"])

    def test_parameter_allocation_swap_fails_closed(self) -> None:
        no_context = rules.diagnose_document(
            _parameter_allocation_report(),
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
        )
        self.assertFalse(
            _evaluation(no_context, "parameter_allocation_consumer_chain")["matched"]
        )

        removed_identity = _parameter_allocation_report()
        removed_identity["left"]["symbols"][0]["instructions"][4]["instruction"]["formatted"] = "mr r31, r3"  # type: ignore[index]
        context = _parameter_allocation_context(removed_identity)
        result = rules.diagnose_document(
            removed_identity,
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
            parameter_allocation_context=context,
        )
        self.assertFalse(
            _evaluation(result, "parameter_allocation_consumer_chain")["matched"]
        )

        wrong_consumer = _parameter_allocation_context()
        wrong_consumer["consumer_chain"]["consumer_rows"] = [6, 7]  # type: ignore[index]
        result = rules.diagnose_document(
            _parameter_allocation_report(),
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
            parameter_allocation_context=wrong_consumer,
        )
        self.assertFalse(
            _evaluation(result, "parameter_allocation_consumer_chain")["matched"]
        )

        wrong_owner = _parameter_allocation_context()
        wrong_owner["owners"]["parameter"]["target_register"] = "r27"  # type: ignore[index]
        result = rules.diagnose_document(
            _parameter_allocation_report(),
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
            parameter_allocation_context=wrong_owner,
        )
        self.assertFalse(
            _evaluation(result, "parameter_allocation_consumer_chain")["matched"]
        )

        false_proof = _parameter_allocation_context()
        false_proof["proofs"]["physical_relocations_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "physical_relocations_exact"
        ):
            rules.diagnose_document(
                _parameter_allocation_report(),
                focus_symbol="mbev_CapPlayerMoveEjectCreate",
                parameter_allocation_context=false_proof,
            )

        malformed_order = _parameter_allocation_context()
        malformed_order["consumer_chain"]["evaluation_order"] = ["typed_pointer_copy", "field_store"]  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "evaluation_order"):
            rules.diagnose_document(
                _parameter_allocation_report(),
                focus_symbol="mbev_CapPlayerMoveEjectCreate",
                parameter_allocation_context=malformed_order,
            )

        strict_pool_residual = _parameter_allocation_report()
        strict_pool_residual["right"]["symbols"][0]["instructions"][10][  # type: ignore[index]
            "diff_kind"
        ] = "DIFF_ARG_MISMATCH"
        context = _parameter_allocation_context(strict_pool_residual)
        result = rules.diagnose_document(
            strict_pool_residual,
            focus_symbol="mbev_CapPlayerMoveEjectCreate",
            parameter_allocation_context=context,
        )
        self.assertFalse(
            _evaluation(result, "parameter_allocation_consumer_chain")["matched"]
        )

    def test_stack_extent_interface_capacity_converges_on_live_capacity(self) -> None:
        report = _capacity_report()
        context = _capacity_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapKokamekku",
            capacity_context=context,
        )
        diagnosis = _evaluation(result, "stack_extent_interface_capacity")
        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["candidate_capacity"], 3)
        self.assertEqual(evidence["missing_extent_bytes"], 8)
        self.assertEqual(evidence["extra_elements"], 2)
        self.assertEqual(evidence["predicted_capacity"], 5)
        self.assertEqual(
            [item["provider"] for item in evidence["producer_contracts"]],
            ["mbPlayerCapsuleMaxGet", "mbPlayerCapsuleNumGet"],
        )
        self.assertEqual(
            evidence["declaration_positions"],
            ["before_moveDir", "after_moveDir", "after_next_aggregate"],
        )

    def test_stack_extent_interface_capacity_fails_closed(self) -> None:
        no_context = rules.diagnose_document(
            _capacity_report(), focus_symbol="mbev_CapKokamekku"
        )
        self.assertFalse(
            _evaluation(no_context, "stack_extent_interface_capacity")["matched"]
        )

        contradictory_contract = _capacity_context()
        contradictory_contract["producer_contracts"][0]["maximum"] = 4  # type: ignore[index]
        result = rules.diagnose_document(
            _capacity_report(),
            focus_symbol="mbev_CapKokamekku",
            capacity_context=contradictory_contract,
        )
        self.assertFalse(
            _evaluation(result, "stack_extent_interface_capacity")["matched"]
        )

        partial_element = _capacity_context()
        partial_element["array"]["target_extent_bytes"] = 19  # type: ignore[index]
        result = rules.diagnose_document(
            _capacity_report(),
            focus_symbol="mbev_CapKokamekku",
            capacity_context=partial_element,
        )
        self.assertFalse(
            _evaluation(result, "stack_extent_interface_capacity")["matched"]
        )

        false_proof = _capacity_context()
        false_proof["proofs"]["physical_relocations_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "physical_relocations_exact"
        ):
            rules.diagnose_document(
                _capacity_report(),
                focus_symbol="mbev_CapKokamekku",
                capacity_context=false_proof,
            )

    def test_loop_branch_destination_ranks_one_else_break_cell(self) -> None:
        report = _loop_branch_report()
        context = _branch_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapKokamekku",
            branch_context=context,
        )
        diagnosis = _evaluation(result, "loop_branch_destination")
        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["row_index"], 2)
        self.assertEqual(evidence["candidate_destination"], "loop_increment")
        self.assertEqual(evidence["target_destination"], "loop_exit")
        self.assertEqual(evidence["candidate_relative_target"], 24)
        self.assertEqual(evidence["target_relative_target"], 44)

    def test_loop_branch_destination_fails_closed(self) -> None:
        wrong_target = _branch_context()
        wrong_target["branch"]["target_relative_target"] = 40  # type: ignore[index]
        result = rules.diagnose_document(
            _loop_branch_report(),
            focus_symbol="mbev_CapKokamekku",
            branch_context=wrong_target,
        )
        self.assertFalse(_evaluation(result, "loop_branch_destination")["matched"])

        extra_residual = _loop_branch_report()
        extra_residual["left"]["symbols"][0]["instructions"][1]["diff_kind"] = "DIFF_ARG_MISMATCH"  # type: ignore[index]
        context = _branch_context(extra_residual)
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapKokamekku",
            branch_context=context,
        )
        self.assertFalse(_evaluation(result, "loop_branch_destination")["matched"])

        invalid_destination = _branch_context()
        invalid_destination["branch"]["target_destination"] = "epilogue"  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "destinations"):
            rules.diagnose_document(
                _loop_branch_report(),
                focus_symbol="mbev_CapKokamekku",
                branch_context=invalid_destination,
            )

    def test_reciprocal_source_shape_ranks_one_division_cell(self) -> None:
        report = _reciprocal_report()
        context = _reciprocal_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=context,
        )
        diagnosis = _evaluation(result, "reciprocal_source_shape")
        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["denominator"], 16)
        self.assertEqual(evidence["reciprocal_f32_bits"], "3d800000")
        self.assertEqual(evidence["target_variable_row"], 3)
        self.assertEqual(evidence["target_reciprocal_row"], 4)
        self.assertEqual(evidence["multiply_row"], 5)
        self.assertIn("division by 16.0f", diagnosis["recommendation"])
        self.assertIn("suppress", diagnosis["recommendation"])

    def test_reciprocal_source_shape_fails_closed(self) -> None:
        no_context = rules.diagnose_document(
            _reciprocal_report(), focus_symbol="mbev_CapEffExplodeOMExec"
        )
        self.assertFalse(_evaluation(no_context, "reciprocal_source_shape")["matched"])

        non_power_of_two = _reciprocal_context()
        non_power_of_two["window"]["denominator"] = 10  # type: ignore[index]
        non_power_of_two["window"]["reciprocal_f32_bits"] = "3dcccccd"  # type: ignore[index]
        result = rules.diagnose_document(
            _reciprocal_report(),
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=non_power_of_two,
        )
        self.assertFalse(_evaluation(result, "reciprocal_source_shape")["matched"])

        wrong_bits = _reciprocal_context()
        wrong_bits["window"]["reciprocal_f32_bits"] = "3f000000"  # type: ignore[index]
        result = rules.diagnose_document(
            _reciprocal_report(),
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=wrong_bits,
        )
        self.assertFalse(_evaluation(result, "reciprocal_source_shape")["matched"])

        nonneutral = _reciprocal_context()
        nonneutral["neutral_observation"]["candidate_object_sha256"] = "7" * 64  # type: ignore[index]
        result = rules.diagnose_document(
            _reciprocal_report(),
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=nonneutral,
        )
        self.assertFalse(_evaluation(result, "reciprocal_source_shape")["matched"])

        extra_residual = _reciprocal_report()
        extra_residual["right"]["symbols"][0]["instructions"][7]["instruction"]["formatted"] = "fmuls f1, f2, f0"  # type: ignore[index]
        context = _reciprocal_context(extra_residual)
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=context,
        )
        self.assertFalse(_evaluation(result, "reciprocal_source_shape")["matched"])

        wrong_relocation = _reciprocal_report()
        wrong_relocation["right"]["symbols"][0]["instructions"][3]["instruction"]["relocation"]["type_name"] = "ADDR16_HA"  # type: ignore[index]
        context = _reciprocal_context(wrong_relocation)
        result = rules.diagnose_document(
            wrong_relocation,
            focus_symbol="mbev_CapEffExplodeOMExec",
            reciprocal_context=context,
        )
        self.assertFalse(_evaluation(result, "reciprocal_source_shape")["matched"])

        false_proof = _reciprocal_context()
        false_proof["proofs"]["cfg_calls_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "cfg_calls_exact"):
            rules.diagnose_document(
                _reciprocal_report(),
                focus_symbol="mbev_CapEffExplodeOMExec",
                reciprocal_context=false_proof,
            )

    def test_switch_case_scoped_fpr_lifetimes_require_frame_join(self) -> None:
        result = rules.diagnose_document(
            _switch_fpr_report(), focus_symbol="ev_CapBobleOMExec"
        )
        diagnosis = _evaluation(result, "switch_case_scoped_fpr_lifetimes")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["evidence"]["frame_delta"], 64)  # type: ignore[index]
        self.assertEqual(len(diagnosis["evidence"]["result_captures"]), 3)  # type: ignore[index]

    def test_final_consumer_self_copy_requires_exact_same_tu_donor(self) -> None:
        result = rules.diagnose_document(
            _aggregate_report(),
            focus_symbol="mbev_CapBomheiMove",
            same_tu_donor_symbols=("mbev_CapBobleMove",),
        )
        diagnosis = _evaluation(result, "aggregate_self_copy_final_consumer")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["focus_copy"]["component_count"],  # type: ignore[index]
            3,
        )
        self.assertEqual(
            diagnosis["evidence"]["exact_donors"][0]["symbol"],  # type: ignore[index]
            "mbev_CapBobleMove",
        )

    def test_adversarial_partial_signatures_are_rejected(self) -> None:
        cycle_report = _assignment_cycle_report()
        cycle_report["left"]["symbols"][0]["instructions"][4] = _instruction(116, "nop")  # type: ignore[index]
        cycle_report["right"]["symbols"][0]["instructions"][4] = _instruction(116, "nop")  # type: ignore[index]
        cycle = rules.diagnose_document(
            cycle_report, focus_symbol="ev_CapMiracleCoinTrade"
        )
        self.assertFalse(
            _evaluation(cycle, "assignment_condition_saved_gpr_cycle")["matched"]
        )

        no_switch = rules.diagnose_document(
            _switch_fpr_report(include_switch=False), focus_symbol="ev_CapBobleOMExec"
        )
        self.assertFalse(
            _evaluation(no_switch, "switch_case_scoped_fpr_lifetimes")["matched"]
        )

        inexact_donor = rules.diagnose_document(
            _aggregate_report(donor_exact=False),
            focus_symbol="mbev_CapBomheiMove",
            same_tu_donor_symbols=("mbev_CapBobleMove",),
        )
        self.assertFalse(
            _evaluation(inexact_donor, "aggregate_self_copy_final_consumer")["matched"]
        )

        else_target = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(104, "bne 0x74", branch_dest=116),
            _instruction(108, "bl body"),
            _instruction(112, "b 0x78", branch_dest=120),
            _instruction(116, "b 0x7c", branch_dest=124),
            _instruction(120, "blr"),
        ]
        else_candidate = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(104, "bne 0x78", branch_dest=120),
            _instruction(108, "bl body"),
            _placeholder(),
            _placeholder(),
            _instruction(120, "blr"),
        ]
        near_else = rules.diagnose_document(
            _report("near_else", else_target, else_candidate), focus_symbol="near_else"
        )
        self.assertFalse(_evaluation(near_else, "explicit_else_return_cfg")["matched"])

    def test_output_is_deterministic_self_hashed_and_authority_free(self) -> None:
        report = _assignment_cycle_report()
        first = rules.diagnose_document(report, focus_symbol="ev_CapMiracleCoinTrade")
        second = rules.diagnose_document(report, focus_symbol="ev_CapMiracleCoinTrade")
        self.assertEqual(first, second)
        self.assertEqual(first["schema"], rules.SCHEMA)
        self.assertFalse(first["authority_advanced"])
        self.assertRegex(first["diagnosis_sha256"], re.compile(r"^[0-9a-f]{64}$"))
        body = dict(first)
        claimed = body.pop("diagnosis_sha256")
        self.assertEqual(claimed, rules._sha256(rules._canonical(body)))
        for diagnosis in first["diagnoses"]:
            self.assertIn("confidence", diagnosis)
            self.assertTrue(diagnosis["evidence"])
            self.assertNotIn("retain", diagnosis["recommendation"].lower())

    def test_direct_cli_emits_the_same_closed_document(self) -> None:
        report = _assignment_cycle_report()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(json.dumps(report), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(path),
                            "--function",
                            "ev_CapMiracleCoinTrade",
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(report, focus_symbol="ev_CapMiracleCoinTrade"),
        )

    def test_allocator_context_cli_emits_the_same_closed_document(self) -> None:
        report = _allocator_swap_report()
        context = _allocator_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapKuribo",
                            "--allocator-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapKuribo",
                allocator_context=context,
            ),
        )

    def test_parameter_allocation_context_cli_emits_same_document(self) -> None:
        report = _parameter_allocation_report()
        context = _parameter_allocation_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "parameter-allocation.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapPlayerMoveEjectCreate",
                            "--parameter-allocation-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapPlayerMoveEjectCreate",
                parameter_allocation_context=context,
            ),
        )

    def test_kokamekku_context_cli_emits_the_same_closed_document(self) -> None:
        report = _loop_branch_report()
        capacity = _capacity_context(report)
        branch = _branch_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            capacity_path = Path(directory) / "capacity.json"
            branch_path = Path(directory) / "branch.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            capacity_path.write_text(json.dumps(capacity), encoding="utf-8")
            branch_path.write_text(json.dumps(branch), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapKokamekku",
                            "--capacity-context",
                            str(capacity_path),
                            "--branch-context",
                            str(branch_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapKokamekku",
                capacity_context=capacity,
                branch_context=branch,
            ),
        )

    def test_reciprocal_context_cli_emits_the_same_closed_document(self) -> None:
        report = _reciprocal_report()
        context = _reciprocal_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "reciprocal.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapEffExplodeOMExec",
                            "--reciprocal-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeOMExec",
                reciprocal_context=context,
            ),
        )


if __name__ == "__main__":
    unittest.main()
