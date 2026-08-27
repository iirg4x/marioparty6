from __future__ import annotations

import contextlib
import copy
import gzip
import io
import json
import re
import tempfile
import unittest
from pathlib import Path

from tools import candidate_interaction_planner as planner
from tools import crack_learning_rules as rules
from tools.tests import test_single_use_final_call_consumer as single_use_fixture
from tools.tests import test_switch_default_constant_fold as switch_fold_fixture


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


def _aggregate_use_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -64(r1)",
        "mr r31, r4",
        "mr r30, r5",
        "mr r29, r6",
        "mr r3, r31",
        "mr r4, r30",
        "mr r5, r29",
        "bl mbev_CapEffExplodeAdd",
        "mr r3, r31",
        "mr r4, r30",
        "mr r5, r29",
        "bl mbev_CapEffExplodeAdd",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -64(r1)",
        "mr r30, r4",
        "mr r29, r5",
        "mr r31, r6",
        "mr r3, r30",
        "mr r4, r29",
        "mr r5, r31",
        "bl mbev_CapEffExplodeAdd",
        "mr r3, r30",
        "mr r4, r29",
        "mr r5, r31",
        "bl mbev_CapEffExplodeAdd",
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
        "mbev_CapEffExplodeKillerAdd",
        target,
        candidate,
        target_size=592,
        candidate_size=592,
    )


def _aggregate_use_context(
    report: dict[str, object] | None = None,
    *,
    copy_count: int = 2,
    independent_consumers: list[dict[str, object]] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _aggregate_use_report()
    destinations = ["color1", "color2"][:copy_count]
    return {
        "schema": rules.AGGREGATE_USE_CONTEXT_SCHEMA,
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
            "source_use_receipt_sha256": "4" * 64,
            "trace_receipt_sha256": "5" * 64,
            "exact_precedent_receipt_sha256": "6" * 64,
        },
        "owners": [
            {
                "name": "pos",
                "target_register": "r31",
                "candidate_register": "r30",
                "evidence_sha256": "7" * 64,
            },
            {
                "name": "vel",
                "target_register": "r30",
                "candidate_register": "r29",
                "evidence_sha256": "8" * 64,
            },
            {
                "name": "color",
                "target_register": "r29",
                "candidate_register": "r31",
                "evidence_sha256": "9" * 64,
            },
        ],
        "aggregate_parameter": {
            "name": "color",
            "type": "GXColor",
            "fields": ["r", "g", "b", "a"],
            "target_register": "r29",
            "candidate_register": "r31",
            "evidence_sha256": "a" * 64,
        },
        "copy_groups": [
            {
                "destination": destination,
                "destination_type": "GXColor",
                "source": "color",
                "fields": ["r", "g", "b", "a"],
                "consumer": "mbev_CapEffExplodeAdd",
                "evidence_sha256": chr(ord("b") + index) * 64,
            }
            for index, destination in enumerate(destinations)
        ],
        "independent_consumers": (
            independent_consumers if independent_consumers is not None else []
        ),
        "rejected_axes": [
            {
                "axis": "input_pointer_aliases",
                "candidate_record_sha256": "d" * 64,
                "regressed": True,
            }
        ],
    }


def _aggregate_followup_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -64(r1)",
        "mr r24, r3",
        "mr r23, r4",
        "mr r3, r24",
        "mr r4, r23",
        "bl Hu3DModelAttrSet",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -64(r1)",
        "mr r23, r3",
        "mr r24, r4",
        "mr r3, r23",
        "mr r4, r24",
        "bl Hu3DModelAttrSet",
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
        "mbev_CapEffElectricAdd",
        target,
        candidate,
        target_size=496,
        candidate_size=496,
    )


def _aggregate_followup_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _aggregate_followup_report()
    return {
        "schema": rules.AGGREGATE_FOLLOWUP_CONTEXT_SCHEMA,
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
            "aggregate_reconstruction_receipt_sha256": "4" * 64,
            "fusion_observation_receipt_sha256": "5" * 64,
        },
        "owners": [
            {
                "name": "particleSystemP",
                "type": "CAPEFFPARTICLESYSTEMWORK",
                "target_register": "r24",
                "candidate_register": "r23",
                "evidence_sha256": "6" * 64,
            },
            {
                "name": "modelP",
                "type": "HU3D_MODEL",
                "target_register": "r23",
                "candidate_register": "r24",
                "evidence_sha256": "7" * 64,
            },
        ],
        "aggregate_boundary": {
            "expression": "particleWorkP->color = ev_CapEffElectricColor[mbRandMod(4)]",
            "already_applied": True,
            "evidence_sha256": "8" * 64,
        },
        "declaration_axis": {
            "recommended_order": ["particleSystemP", "modelP"],
            "evidence_sha256": "9" * 64,
        },
        "fusion_observation": {
            "source_shape": "fused modelP producer and consumer",
            "target_size": 496,
            "candidate_size": 500,
            "strict_regressed": True,
            "topology_changed": True,
            "candidate_record_sha256": "a" * 64,
        },
    }


def _address_taken_report() -> dict[str, object]:
    target = [
        _instruction(100, "stwu r1, -48(r1)", diff_kind="DIFF_INSERT"),
        _instruction(104, "stw r3, 8(r1)", diff_kind="DIFF_INSERT"),
        _instruction(108, "mr r30, r4", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "stfs f1, 16(r1)"),
        _instruction(116, "addi r31, r1, 16", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(120, "mr r4, r31", diff_kind="DIFF_INSERT"),
        _instruction(124, "bl mbev_CapEffGlowKinokoAdd"),
        _instruction(128, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -32(r1)", diff_kind="DIFF_DELETE"),
        _instruction(104, "mr r31, r4", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "stfs f1, 16(r1)"),
        _instruction(112, "addi r4, r1, 16", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bl mbev_CapEffGlowKinokoAdd"),
        _instruction(120, "blr"),
    ]
    return _report(
        "mbev_CapEffGlowKinokoAddAlt",
        target,
        candidate,
        target_size=272,
        candidate_size=260,
    )


def _address_taken_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _address_taken_report()
    return {
        "schema": rules.ADDRESS_TAKEN_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "source_boundary_receipt_sha256": "4" * 64,
            "typed_consumer_receipt_sha256": "5" * 64,
        },
        "expected_size_delta": 12,
        "aggregate": {
            "name": "pos",
            "type": "HuVecF",
            "stack_offset": 16,
            "evidence_sha256": "6" * 64,
        },
        "incoming_pointer": {
            "name": "posP",
            "target_register": "r30",
            "candidate_register": "r31",
            "evidence_sha256": "7" * 64,
        },
        "local_pointer": {
            "name": "posLocalP",
            "target_register": "r31",
            "argument_register": "r4",
            "consumer": "mbev_CapEffGlowKinokoAdd",
            "evidence_sha256": "8" * 64,
        },
        "object_home": {
            "parameter": "obj",
            "target_stack_offset": 8,
            "evidence_sha256": "9" * 64,
        },
    }


def _aggregate_pointer_branch_report() -> dict[str, object]:
    target = [
        _instruction(100, "cmpwi r3, 0"),
        _instruction(104, "beq 0x180", branch_dest=384),
        _instruction(108, "lbz r0, 0(r29)"),
        _instruction(112, "stb r0, 40(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "lbz r0, 1(r29)"),
        _instruction(120, "stb r0, 41(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(124, "lbz r0, 2(r29)"),
        _instruction(128, "stb r0, 42(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "lbz r0, 3(r29)"),
        _instruction(136, "stb r0, 43(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(
            140,
            "b 0x160",
            diff_kind="DIFF_ARG_MISMATCH",
            branch_dest=352,
        ),
        _instruction(144, "blr"),
    ]
    candidate = [
        _instruction(100, "cmpwi r3, 0"),
        _instruction(104, "beq 0x180", branch_dest=384),
        _instruction(108, "lbz r0, 0(r29)"),
        _instruction(112, "stb r0, 36(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "lbz r0, 1(r29)"),
        _instruction(120, "stb r0, 37(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(124, "lbz r0, 2(r29)"),
        _instruction(128, "stb r0, 38(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "lbz r0, 3(r29)"),
        _instruction(136, "stb r0, 39(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(
            140,
            "b 0x180",
            diff_kind="DIFF_ARG_MISMATCH",
            branch_dest=384,
        ),
        _instruction(144, "blr"),
    ]
    report = _report(
        "mbev_CapEffGlowKinokoAdd",
        target,
        candidate,
        target_size=1204,
        candidate_size=1204,
    )
    report["left"]["symbols"][0]["match_percent"] = 99.9701  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 99.9701  # type: ignore[index]
    return report


def _aggregate_snapshot_pointer_report() -> dict[str, object]:
    target = [_instruction(100 + (index * 4), "nop") for index in range(48)]
    candidate = copy.deepcopy(target)
    target[0] = _instruction(100, "stwu r1, -0x60(r1)")
    candidate[0] = _instruction(100, "stwu r1, -0x60(r1)")
    target[17] = _instruction(168, "addi r31, r1, 0xc")
    candidate[17] = _instruction(168, "addi r31, r1, 0xc")
    copy_groups = [
        (18, "r27", 0x10),
        (25, "r26", 0x1C),
        (32, "r25", 0x28),
    ]
    for start, source, stack_offset in copy_groups:
        instructions = [
            f"lwz r3, 0x0({source})",
            f"lwz r0, 0x4({source})",
            f"stw r3, 0x{stack_offset:x}(r1)",
            f"stw r0, 0x{stack_offset + 4:x}(r1)",
            f"lwz r0, 0x8({source})",
            f"stw r0, 0x{stack_offset + 8:x}(r1)",
        ]
        for offset, formatted in enumerate(instructions):
            target[start + offset] = _instruction(
                100 + ((start + offset) * 4), formatted
            )
            candidate[start + offset] = copy.deepcopy(target[start + offset])
    target[24] = _instruction(
        196, "addi r30, r1, 0x10", diff_kind="DIFF_ARG_MISMATCH"
    )
    candidate[24] = _instruction(
        196, "addi r28, r1, 0x10", diff_kind="DIFF_ARG_MISMATCH"
    )
    target[31] = _instruction(224, "addi r29, r1, 0x1c")
    candidate[31] = _instruction(224, "addi r29, r1, 0x1c")
    target[38] = _instruction(
        252, "addi r28, r1, 0x28", diff_kind="DIFF_ARG_MISMATCH"
    )
    candidate[38] = _instruction(
        252, "addi r30, r1, 0x28", diff_kind="DIFF_ARG_MISMATCH"
    )
    target[40] = _instruction(260, "mr r4, r28", diff_kind="DIFF_ARG_MISMATCH")
    candidate[40] = _instruction(
        260, "mr r4, r30", diff_kind="DIFF_ARG_MISMATCH"
    )
    target[41] = _instruction(264, "mr r5, r29")
    candidate[41] = _instruction(264, "mr r5, r29")
    target[42] = _instruction(268, "mr r6, r30", diff_kind="DIFF_ARG_MISMATCH")
    candidate[42] = _instruction(
        268, "mr r6, r28", diff_kind="DIFF_ARG_MISMATCH"
    )
    target[46] = _instruction(284, "mr r10, r31")
    candidate[46] = _instruction(284, "mr r10, r31")
    target[47] = _instruction(288, "bl mbev_CapEffRingAdd")
    candidate[47] = _instruction(288, "bl mbev_CapEffRingAdd")
    report = _report(
        "mbev_CapEffRingHitAdd",
        target,
        candidate,
        target_size=216,
        candidate_size=216,
    )
    report["left"]["symbols"][0]["match_percent"] = 99.62963  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 99.62963  # type: ignore[index]
    return report


def _aggregate_snapshot_pointer_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _aggregate_snapshot_pointer_report()
    snapshot_record = "1" * 64
    pointer_record = "2" * 64
    exact_record = "3" * 64
    return {
        "schema": rules.AGGREGATE_SNAPSHOT_POINTER_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "aggregate_snapshots_authenticated": True,
            "typed_pointer_consumers_authenticated": True,
            "pointer_owner_cycle_authenticated": True,
            "pinned_mwcc_frontend": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "strict_report_sha256": "4" * 64,
            "data_report_sha256": "5" * 64,
            "physical_relocation_receipt_sha256": "6" * 64,
            "graph_receipt_sha256": "7" * 64,
            "graft_receipt_sha256": "8" * 64,
            "source_range_receipt_sha256": "9" * 64,
            "snapshot_control_record_sha256": snapshot_record,
            "pointer_control_record_sha256": pointer_record,
            "exact_result_report_sha256": "a" * 64,
            "exact_result_record_sha256": exact_record,
        },
        "precursor": {
            "candidate_id": "ringhit-c002",
            "target_bytes": 216,
            "candidate_bytes": 216,
            "match_percent": 99.62963,
            "physical_relocations": 7,
            "residual_rows": [24, 38, 40, 42],
        },
        "consumer": {
            "symbol": "mbev_CapEffRingAdd",
            "call_row": 47,
            "evidence_sha256": "b" * 64,
        },
        "color_pointer": {
            "local": "color",
            "type": "GXColor",
            "stack_offset": 12,
            "pointer_owner": "colorLocalP",
            "pointer_row": 17,
            "target_register": "r31",
            "candidate_register": "r31",
            "argument_row": 46,
            "argument_register": "r10",
            "source_expression": "color = capsuleRingColor; colorLocalP = &color",
            "evidence_sha256": "c" * 64,
        },
        "snapshots": [
            {
                "source_pointer": "scale",
                "source_register": "r27",
                "local": "scaleLocal",
                "type": "HuVecF",
                "size": 12,
                "stack_offset": 16,
                "copy_rows": [18, 19, 20, 21, 22, 23],
                "pointer_owner": "scaleLocalP",
                "pointer_row": 24,
                "target_pointer_register": "r30",
                "candidate_pointer_register": "r28",
                "argument_row": 42,
                "argument_register": "r6",
                "source_expression": "scaleLocal = *scale; scaleLocalP = &scaleLocal",
                "evidence_sha256": "d" * 64,
            },
            {
                "source_pointer": "rot",
                "source_register": "r26",
                "local": "rotLocal",
                "type": "HuVecF",
                "size": 12,
                "stack_offset": 28,
                "copy_rows": [25, 26, 27, 28, 29, 30],
                "pointer_owner": "rotLocalP",
                "pointer_row": 31,
                "target_pointer_register": "r29",
                "candidate_pointer_register": "r29",
                "argument_row": 41,
                "argument_register": "r5",
                "source_expression": "rotLocal = *rot; rotLocalP = &rotLocal",
                "evidence_sha256": "e" * 64,
            },
            {
                "source_pointer": "pos",
                "source_register": "r25",
                "local": "posLocal",
                "type": "HuVecF",
                "size": 12,
                "stack_offset": 40,
                "copy_rows": [32, 33, 34, 35, 36, 37],
                "pointer_owner": "posLocalP",
                "pointer_row": 38,
                "target_pointer_register": "r28",
                "candidate_pointer_register": "r30",
                "argument_row": 40,
                "argument_register": "r4",
                "source_expression": "posLocal = *pos; posLocalP = &posLocal",
                "evidence_sha256": "f" * 64,
            },
        ],
        "controls": [
            {
                "kind": "snapshots_only",
                "candidate_id": "ringhit-c001",
                "target_bytes": 216,
                "candidate_bytes": 208,
                "strict_exact": False,
                "source_sha256": "1" * 64,
                "object_sha256": "2" * 64,
                "strict_report_sha256": "3" * 64,
                "data_report_sha256": "4" * 64,
                "candidate_record_sha256": snapshot_record,
                "unresolved_boundary": "saved_local_address_owners",
            },
            {
                "kind": "typed_pointer_chain",
                "candidate_id": "ringhit-c002",
                "target_bytes": 216,
                "candidate_bytes": 216,
                "strict_exact": False,
                "residual_rows": [24, 38, 40, 42],
                "source_sha256": "5" * 64,
                "object_sha256": "6" * 64,
                "strict_report_sha256": "7" * 64,
                "data_report_sha256": "8" * 64,
                "candidate_record_sha256": pointer_record,
                "unresolved_boundary": "scale_position_pointer_owner_cycle",
            },
        ],
        "combined_cell": {
            "candidate_id": "ringhit-c003-exact",
            "target_bytes": 216,
            "candidate_bytes": 216,
            "strict_exact": True,
            "data_exact": True,
            "physical_relocations": 7,
            "source_sha256": "9" * 64,
            "object_sha256": "a" * 64,
            "strict_report_sha256": "b" * 64,
            "data_report_sha256": "c" * 64,
            "candidate_record_sha256": exact_record,
        },
    }


def _typed_aggregate_copy_report() -> dict[str, object]:
    target = [_instruction(100 + (index * 4), "nop") for index in range(22)]
    candidate = copy.deepcopy(target)
    target[0] = _instruction(100, "stwu r1, -0x120(r1)")
    candidate[0] = copy.deepcopy(target[0])

    register_rows = {
        1: ("mr r30, r4", "mr r23, r4"),
        2: ("mr r23, r6", "mr r24, r6"),
        3: ("li r29, 0x0", "li r30, 0x0"),
        10: ("mr r24, r7", "mr r25, r7"),
        11: ("mr r25, r8", "mr r26, r8"),
        12: ("mr r26, r9", "mr r27, r9"),
        13: ("mr r27, r10", "mr r28, r10"),
        14: ("lwz r28, 0x8c(r1)", "lwz r29, 0x8c(r1)"),
        15: ("addi r26, r1, 0xc", "addi r27, r1, 0xc"),
        16: ("addi r25, r1, 0x14", "addi r26, r1, 0x14"),
        17: ("addi r24, r1, 0x20", "addi r25, r1, 0x20"),
        18: ("cmpw r29, r23", "cmpw r30, r24"),
    }
    for row, (left, right) in register_rows.items():
        target[row] = _instruction(
            100 + (row * 4), left, diff_kind="DIFF_ARG_MISMATCH"
        )
        candidate[row] = _instruction(
            100 + (row * 4), right, diff_kind="DIFF_ARG_MISMATCH"
        )

    target_copy = [
        "lfs f0, 0x0(r30)",
        "stfs f0, 0x38(r1)",
        "lfs f0, 0x4(r30)",
        "stfs f0, 0x3c(r1)",
        "lfs f0, 0x8(r30)",
        "stfs f0, 0x40(r1)",
    ]
    candidate_copy = [
        "lwz r3, 0x0(r23)",
        "lwz r0, 0x4(r23)",
        "stw r3, 0x38(r1)",
        "stw r0, 0x3c(r1)",
        "lwz r0, 0x8(r23)",
        "stw r0, 0x40(r1)",
    ]
    for offset, (left, right) in enumerate(zip(target_copy, candidate_copy)):
        row = 4 + offset
        target[row] = _instruction(
            100 + (row * 4), left, diff_kind="DIFF_REPLACE"
        )
        candidate[row] = _instruction(
            100 + (row * 4), right, diff_kind="DIFF_REPLACE"
        )
    target[19] = _instruction(176, "bl mbev_CapEffExplodeAdd")
    candidate[19] = copy.deepcopy(target[19])
    target[21] = _instruction(184, "blr")
    candidate[21] = copy.deepcopy(target[21])
    report = _report(
        "mbev_CapEffDustMultiAdd",
        target,
        candidate,
        target_size=836,
        candidate_size=836,
    )
    report["left"]["symbols"][0]["match_percent"] = 97.7512  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 97.7512  # type: ignore[index]
    return report


def _typed_aggregate_copy_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _typed_aggregate_copy_report()
    residual_rows = list(range(1, 19))
    return {
        "schema": rules.TYPED_AGGREGATE_COPY_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "typed_member_copy_authenticated": True,
            "whole_aggregate_control_authenticated": True,
            "owner_cycle_authenticated": True,
            "pinned_mwcc_frontend": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "precursor_source_sha256": "3" * 64,
            "precursor_object_sha256": "4" * 64,
            "precursor_record_sha256": "5" * 64,
            "exact_source_sha256": "6" * 64,
            "exact_object_sha256": "7" * 64,
            "exact_strict_report_sha256": "8" * 64,
            "exact_data_report_sha256": "9" * 64,
            "exact_record_sha256": "a" * 64,
            "interaction_plan_sha256": "b" * 64,
            "causal_reducer_sha256": "c" * 64,
            "report_artifact_sha256": "d" * 64,
        },
        "precursor": {
            "candidate_id": "dustmulti001-composed",
            "target_bytes": 836,
            "candidate_bytes": 836,
            "match_percent": 97.7512,
            "physical_relocations": 43,
            "residual_rows": residual_rows,
        },
        "aggregate": {
            "type": "HuVecF",
            "source_pointer": "posP",
            "local": "posTemp",
            "size": 12,
            "stack_offset": 56,
            "member_offsets": [0, 4, 8],
            "copy_rows": [4, 5, 6, 7, 8, 9],
            "target_source_register": "r30",
            "candidate_source_register": "r23",
            "source_expression": "posTemp.x = posP->x; posTemp.y = posP->y; posTemp.z = posP->z",
        },
        "exact_result": {
            "candidate_id": "dustmulti002-member-copy-exact",
            "target_bytes": 836,
            "candidate_bytes": 836,
            "physical_relocations": 43,
            "source_sha256": "6" * 64,
            "object_sha256": "7" * 64,
            "strict_report_sha256": "8" * 64,
            "data_report_sha256": "9" * 64,
            "candidate_record_sha256": "a" * 64,
        },
    }


def _dform_copy_owner_cycle_report() -> dict[str, object]:
    target = [_instruction(100 + (index * 4), "nop") for index in range(14)]
    candidate = copy.deepcopy(target)
    target[0] = _instruction(
        100, "stwu r1, -0x80(r1)", diff_kind="DIFF_ARG_MISMATCH"
    )
    candidate[0] = _instruction(
        100, "stwu r1, -0x70(r1)", diff_kind="DIFF_ARG_MISMATCH"
    )
    target_copy = [
        "psq_l f0, 0x0(r30)",
        "lfs f1, 0x8(r30)",
        "psq_st f0, 0x0(r31)",
        "stfs f1, 0x8(r31)",
    ]
    candidate_copy = [
        "psq_lx f0, r0, r23",
        "lfs f1, 0x8(r23)",
        "psq_stx f0, r0, r24",
        "stfs f1, 0x8(r24)",
    ]
    for offset, (left, right) in enumerate(zip(target_copy, candidate_copy, strict=True)):
        row = 1 + offset
        target[row] = _instruction(
            100 + (row * 4), left, diff_kind="DIFF_REPLACE"
        )
        candidate[row] = _instruction(
            100 + (row * 4), right, diff_kind="DIFF_REPLACE"
        )
    cascade = {
        5: ("mr r25, r3", "mr r26, r3"),
        6: ("mr r26, r4", "mr r27, r4"),
        7: ("mr r27, r5", "mr r25, r5"),
        8: ("cmpw r25, r26", "cmpw r26, r27"),
        9: ("stw r27, 0x20(r1)", "stw r25, 0x10(r1)"),
        10: ("lwz r25, 0x24(r1)", "lwz r26, 0x14(r1)"),
    }
    for row, (left, right) in cascade.items():
        target[row] = _instruction(
            100 + (row * 4), left, diff_kind="DIFF_ARG_MISMATCH"
        )
        candidate[row] = _instruction(
            100 + (row * 4), right, diff_kind="DIFF_ARG_MISMATCH"
        )
    target[11] = _instruction(144, "bl mbMasuPosGet")
    candidate[11] = copy.deepcopy(target[11])
    target[13] = _instruction(152, "blr")
    candidate[13] = copy.deepcopy(target[13])
    report = _report(
        "mbev_PlayerColBall",
        target,
        candidate,
        target_size=716,
        candidate_size=716,
    )
    report["left"]["symbols"][0]["match_percent"] = 98.88827  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 98.88827  # type: ignore[index]
    return report


def _dform_copy_trace_report() -> dict[str, object]:
    target = [_instruction(200 + (index * 4), "nop") for index in range(10)]
    candidate = copy.deepcopy(target)
    target[0] = _instruction(
        200, "stwu r1, -0xd0(r1)", diff_kind="DIFF_ARG_MISMATCH"
    )
    candidate[0] = _instruction(
        200, "stwu r1, -0xa0(r1)", diff_kind="DIFF_ARG_MISMATCH"
    )
    target_copy = [
        "psq_l f0, 0x8(r1)",
        "lfs f1, 0x10(r1)",
        "psq_st f0, 0x14(r1)",
        "stfs f1, 0x1c(r1)",
    ]
    candidate_copy = [
        "lwz r3, 0x8(r1)",
        "lwz r0, 0xc(r1)",
        "stw r3, 0x14(r1)",
        "stw r0, 0x18(r1)",
    ]
    for offset, (left, right) in enumerate(zip(target_copy, candidate_copy, strict=True)):
        row = 1 + offset
        target[row] = _instruction(
            200 + (row * 4), left, diff_kind="DIFF_REPLACE"
        )
        candidate[row] = _instruction(
            200 + (row * 4), right, diff_kind="DIFF_REPLACE"
        )
    target[5] = _instruction(220, "nop", diff_kind="DIFF_DELETE")
    candidate[5] = _instruction(220, "lwz r0, 0x10(r1)", diff_kind="DIFF_INSERT")
    target[6] = _instruction(224, "nop", diff_kind="DIFF_DELETE")
    candidate[6] = _instruction(224, "stw r0, 0x1c(r1)", diff_kind="DIFF_INSERT")
    target[8] = _instruction(232, "bl mbObjPosSetV")
    candidate[8] = copy.deepcopy(target[8])
    target[9] = _instruction(236, "blr")
    candidate[9] = copy.deepcopy(target[9])
    report = _report(
        "MoveNumOMExec",
        target,
        candidate,
        target_size=1152,
        candidate_size=1120,
    )
    report["left"]["symbols"][0]["match_percent"] = 94.270836  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 94.270836  # type: ignore[index]
    return report


def _dform_copy_helper_context(
    report: dict[str, object],
    *,
    mode: str,
) -> dict[str, object]:
    is_owner_cycle = mode == "existing_owner_cycle"
    target_bytes = 716 if is_owner_cycle else 1152
    candidate_bytes = 716 if is_owner_cycle else 1120
    physical_relocations = 19 if is_owner_cycle else 64
    helper_expression = (
        "HuVecCopy(&pos[playerNoTbl[i]], &posTbl[i])"
        if is_owner_cycle
        else "HuVecCopy(&posNorm, &pos)"
    )
    context: dict[str, object] = {
        "schema": rules.DFORM_COPY_HELPER_CONTEXT_SCHEMA,
        "proofs": {
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "copy_semantics_authenticated": True,
            "same_tu_helper_authenticated": True,
            "pinned_mwcc_frontend": True,
            "protected_siblings_preserved": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "precursor_source_sha256": "3" * 64,
            "precursor_object_sha256": "4" * 64,
            "precursor_record_sha256": "5" * 64,
            "helper_source_sha256": "6" * 64,
            "exact_source_sha256": "7" * 64,
            "exact_object_sha256": "8" * 64,
            "exact_strict_report_sha256": "9" * 64,
            "exact_data_report_sha256": "a" * 64,
            "exact_record_sha256": "b" * 64,
            "causal_reducer_sha256": "c" * 64,
            "report_artifact_sha256": "d" * 64,
        },
        "precursor": {
            "candidate_id": "colball-helper-precursor" if is_owner_cycle else "movenum-baseline",
            "target_bytes": target_bytes,
            "candidate_bytes": candidate_bytes,
            "target_frame": 128 if is_owner_cycle else 208,
            "candidate_frame": 112 if is_owner_cycle else 160,
            "match_percent": 98.88827 if is_owner_cycle else 94.270836,
            "physical_relocations": physical_relocations,
            "residual_rows": list(range(0, 11)) if is_owner_cycle else list(range(0, 7)),
        },
        "copy": {
            "type": "HuVecF",
            "source_identity": "pos" if is_owner_cycle else "posNorm",
            "destination_identity": "posTbl" if is_owner_cycle else "pos",
            "size": 12,
            "helper_symbol": "HuVecCopy",
            "helper_expression": helper_expression,
            "target_lowering": ["psq_l", "lfs", "psq_st", "stfs"],
            "candidate_lowering": (
                ["psq_lx", "lfs", "psq_stx", "stfs"]
                if is_owner_cycle
                else ["lwz", "lwz", "stw", "stw", "lwz", "stw"]
            ),
        },
        "exact_result": {
            "candidate_id": "colball-exact" if is_owner_cycle else "movenum-exact",
            "target_bytes": target_bytes,
            "candidate_bytes": target_bytes,
            "physical_relocations": physical_relocations,
            "source_sha256": "7" * 64,
            "object_sha256": "8" * 64,
            "strict_report_sha256": "9" * 64,
            "data_report_sha256": "a" * 64,
            "candidate_record_sha256": "b" * 64,
        },
    }
    if is_owner_cycle:
        context["evidence"] = {
            "mode": mode,
            "copy_rows": [1, 2, 3, 4],
            "cascade_rows": [0, 5, 6, 7, 8, 9, 10],
            "owner_mapping": {"r25": "r26", "r26": "r27", "r27": "r25"},
            "existing_live_owners": [
                {
                    "identity": "j",
                    "target_register": "r25",
                    "candidate_register": "r26",
                    "used_after_copy": True,
                },
                {
                    "identity": "i",
                    "target_register": "r26",
                    "candidate_register": "r27",
                    "used_after_copy": True,
                },
                {
                    "identity": "temp",
                    "target_register": "r27",
                    "candidate_register": "r25",
                    "used_after_copy": True,
                },
            ],
        }
    else:
        context["evidence"] = {
            "mode": mode,
            "session_id": "session-8e7a3c19b4d260f1",
            "source_interval": {"base": "r1", "start": 8, "end": 20},
            "destination_interval": {"base": "r1", "start": 20, "end": 32},
            "loads": [113, 114, 117],
            "stores": [115, 116, 118],
            "dependencies": [[113, 115], [114, 116], [117, 118]],
            "target_copy_rows": [1, 2, 3, 4],
            "seam_unknown_count": 0,
            "paired_codegen_proof": False,
            "address_definitions_authenticated": True,
            "request_sha256": "e" * 64,
            "causal_map_sha256": "f" * 64,
            "execution_receipt_sha256": "0" * 64,
        }
    return context
def _aggregate_pointer_branch_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _aggregate_pointer_branch_report()
    return {
        "schema": rules.AGGREGATE_POINTER_BRANCH_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "same_tu_donors_exact": True,
            "precursor_structural_groups_closed": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "graph_receipt_sha256": "4" * 64,
            "same_tu_donor_receipt_sha256": "5" * 64,
            "precursor_candidate_record_sha256": "6" * 64,
            "exact_result_report_sha256": "7" * 64,
            "exact_result_candidate_record_sha256": "8" * 64,
        },
        "precursor": {
            "candidate_id": "glowkinokoadd-c001-focus-v1",
            "target_bytes": 1204,
            "candidate_bytes": 1204,
            "match_percent": 99.9701,
            "physical_relocations": 69,
            "residual_rows": [3, 5, 7, 9, 10],
        },
        "aggregate_chain": {
            "consumer": "mbev_CapEffGlowAdd",
            "groups": [
                {
                    "temporary": "colorTemp",
                    "final": "color",
                    "type": "GXColor",
                    "size": 4,
                    "pointer_owner": "colorLocalP",
                    "target_register": "r28",
                    "evidence_sha256": "9" * 64,
                },
                {
                    "temporary": "velTemp",
                    "final": "vel",
                    "type": "HuVecF",
                    "size": 12,
                    "pointer_owner": "velLocalP",
                    "target_register": "r27",
                    "evidence_sha256": "a" * 64,
                },
                {
                    "temporary": "posTemp",
                    "final": "pos",
                    "type": "HuVecF",
                    "size": 12,
                    "pointer_owner": "posLocalP",
                    "target_register": "r26",
                    "evidence_sha256": "b" * 64,
                },
            ],
            "negative_one_expression": "-1.0f * (1.0f + 0.2f * rand)",
            "recommended_first_cell": "compose the three temp-to-final aggregate copies, live typed pointer consumers, and explicit -1.0f multiply",
            "evidence_sha256": "c" * 64,
        },
        "branch_result": {
            "temporary": "colorTemp",
            "final": "color",
            "type": "GXColor",
            "byte_count": 4,
            "source_pointer_register": "r29",
            "source_load_rows": [2, 4, 6, 8],
            "copy_rows": [3, 5, 7, 9],
            "branch_row": 10,
            "target_stack_offset": 40,
            "candidate_stack_offset": 36,
            "target_branch_relative": 212,
            "candidate_branch_relative": 244,
            "source_shape": "both branches assign colorTemp, followed by unconditional color = colorTemp",
            "evidence_sha256": "d" * 64,
        },
        "exact_result": {
            "candidate_id": "glowkinokoadd-c002-focus-v1",
            "source_sha256": "e" * 64,
            "object_sha256": "f" * 64,
            "strict_report_sha256": "0" * 64,
            "data_report_sha256": "1" * 64,
            "candidate_record_sha256": "2" * 64,
            "target_bytes": 1204,
            "candidate_bytes": 1204,
            "physical_relocations": 69,
        },
    }


def _same_tu_shape_report() -> dict[str, object]:
    zero_relocation = {"type": 109, "type_name": "R_PPC_EMB_SDA21"}
    target = [
        _instruction(100, "stwu r1, -32(r1)"),
        _instruction(104, "li r0, 0", diff_kind="DIFF_INSERT"),
        _instruction(108, "srawi r3, r0, 31", diff_kind="DIFF_INSERT"),
        _instruction(112, "srwi r3, r3, 29", diff_kind="DIFF_INSERT"),
        _instruction(116, "subfc r0, r0, r3", diff_kind="DIFF_INSERT"),
        _instruction(120, "adde r0, r3, r0", diff_kind="DIFF_INSERT"),
        _placeholder("DIFF_DELETE"),
        _instruction(128, "stw r4, 20(r5)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "lfs f0, @zero@sda21", relocation=zero_relocation),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _instruction(156, "stfs f0, 8(r6)", diff_kind="DIFF_INSERT"),
        _instruction(160, "stfs f0, 4(r6)", diff_kind="DIFF_INSERT"),
        _instruction(164, "stfs f0, 0(r6)", diff_kind="DIFF_INSERT"),
        _instruction(168, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -32(r1)"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _instruction(124, "extsh r0, r4", diff_kind="DIFF_DELETE"),
        _instruction(128, "stw r0, 20(r5)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "lfs f0, @zero@sda21", relocation=zero_relocation),
        _instruction(136, "stfs f0, 0(r6)", diff_kind="DIFF_DELETE"),
        _instruction(
            140,
            "lfs f0, @zero@sda21",
            diff_kind="DIFF_DELETE",
            relocation=zero_relocation,
        ),
        _instruction(144, "stfs f0, 4(r6)", diff_kind="DIFF_DELETE"),
        _instruction(
            148,
            "lfs f0, @zero@sda21",
            diff_kind="DIFF_DELETE",
            relocation=zero_relocation,
        ),
        _instruction(152, "stfs f0, 8(r6)", diff_kind="DIFF_DELETE"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _instruction(168, "blr"),
    ]
    return _report(
        "mbev_CapEffElectricModelSet",
        target,
        candidate,
        target_size=252,
        candidate_size=244,
    )


def _same_tu_shape_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _same_tu_shape_report()
    return {
        "schema": rules.SAME_TU_SHAPE_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "donor_strict_exact": True,
            "donor_data_exact": True,
            "caller_contract_authenticated": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "same_tu_donor_receipt_sha256": "4" * 64,
            "caller_contract_receipt_sha256": "5" * 64,
            "source_shape_receipt_sha256": "6" * 64,
        },
        "donor": {
            "symbol": "mbev_CapEffElectricAdd",
            "source_location": "game/src/board/capevent.c:L4421",
            "source_expression": "objIdx >= 8",
            "array_bound": 8,
            "evidence_sha256": "7" * 64,
        },
        "fixed_array_tail": {
            "target_rows": [1, 2, 3, 4, 5],
            "array_bound": 8,
            "source_expression": "objIdx >= 8",
            "evidence_sha256": "8" * 64,
        },
        "abi_boundary": {
            "parameter": "modelId",
            "parameter_register": "r4",
            "producer_type": "s16",
            "callee_type": "int",
            "candidate_normalization_row": 6,
            "store_row": 7,
            "caller_symbol": "mbev_CapBiriQMetalShock",
            "source_location": "game/src/board/captrap.c:L913",
            "evidence_sha256": "9" * 64,
        },
        "zero_chain": {
            "destination": "partP->modelPos",
            "fields": ["x", "y", "z"],
            "target_load_row": 8,
            "target_store_rows": [14, 15, 16],
            "candidate_load_rows": [8, 10, 12],
            "candidate_store_rows": [9, 11, 13],
            "source_expression": (
                "partP->modelPos.x = partP->modelPos.y = "
                "partP->modelPos.z = 0.0f"
            ),
            "evidence_sha256": "a" * 64,
        },
        "combined_cell": {
            "candidate_id": "capevent-electricmodelset-exact",
            "target_size": 252,
            "candidate_size": 252,
            "object_sha256": (
                "1957f15be546225c3d6fd8e9ad4ad40cb2124e10b3b70610112572252e71d1e6"
            ),
            "candidate_record_sha256": (
                "ea45ef1a151f2ba62b8f620bf70f19b86139032a4712e36607826ec16bcd1167"
            ),
        },
    }


def _short_circuit_report() -> dict[str, object]:
    target = [
        _instruction(100, "stwu r1, -64(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "bl mbBranchAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "bl mbMasuAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "bne 0x8c", diff_kind="DIFF_ARG_MISMATCH", branch_dest=140),
        _instruction(116, "bl mbBranchMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(120, "bl mbMasuMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(124, "beq 0x90", diff_kind="DIFF_ARG_MISMATCH", branch_dest=144),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _instruction(140, "li r27, 1", diff_kind="DIFF_INSERT"),
        _instruction(144, "li r27, 0", diff_kind="DIFF_INSERT"),
        _instruction(148, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -80(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "bl mbMasuAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "bl mbBranchAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "bne 0x80", diff_kind="DIFF_ARG_MISMATCH", branch_dest=128),
        _instruction(116, "bl mbMasuMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(120, "bl mbBranchMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(124, "beq 0x88", diff_kind="DIFF_ARG_MISMATCH", branch_dest=136),
        _instruction(128, "li r27, 1", diff_kind="DIFF_DELETE"),
        _instruction(132, "li r27, 1", diff_kind="DIFF_DELETE"),
        _instruction(136, "li r27, 0", diff_kind="DIFF_DELETE"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _instruction(148, "blr"),
    ]
    return _report(
        "mbev_CapMasuLinkNextGet",
        target,
        candidate,
        target_size=356,
        candidate_size=364,
    )


def _short_circuit_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _short_circuit_report()
    return {
        "schema": rules.SHORT_CIRCUIT_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "physical_relocations_exact": True,
            "data_sections_exact": True,
            "protected_siblings_preserved": True,
            "pinned_mwcc_frontend": True,
            "topology_observation_size_exact": True,
            "topology_observation_cfg_exact": True,
            "topology_observation_relocations_exact": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "call_order_receipt_sha256": "4" * 64,
            "topology_observation_report_sha256": "5" * 64,
            "declaration_owner_receipt_sha256": "6" * 64,
        },
        "mask_tests": [
            {
                "source_left": "mbMasuAttrGet(linkMasu)",
                "source_right": "mbBranchAttrGet()",
                "source_expression": (
                    "mbMasuAttrGet(linkMasu) & mbBranchAttrGet()"
                ),
                "branch_getter": "mbBranchAttrGet",
                "masu_getter": "mbMasuAttrGet",
                "target_branch_call_row": 1,
                "target_masu_call_row": 2,
                "evidence_sha256": "7" * 64,
            },
            {
                "source_left": "mbMasuMAttrGet(linkMasu)",
                "source_right": "mbBranchMAttrGet()",
                "source_expression": (
                    "mbMasuMAttrGet(linkMasu) & mbBranchMAttrGet()"
                ),
                "branch_getter": "mbBranchMAttrGet",
                "masu_getter": "mbMasuMAttrGet",
                "target_branch_call_row": 4,
                "target_masu_call_row": 5,
                "evidence_sha256": "8" * 64,
            },
        ],
        "shared_boolean": {
            "target_branch_rows": [3, 6],
            "target_true_assignment_row": 10,
            "target_false_assignment_row": 11,
            "candidate_true_assignment_rows": [7, 8],
            "candidate_false_assignment_row": 9,
            "result_register": "r27",
            "evidence_sha256": "9" * 64,
        },
        "direct_assignment_rejection": {
            "candidate_record_sha256": "a" * 64,
            "reversed_call_order": True,
            "strict_regressed": True,
            "evidence_sha256": "b" * 64,
        },
        "topology_observation": {
            "candidate_id": "capevent-masulink003-exact",
            "target_size": 356,
            "candidate_size": 356,
            "residual_kind": "ARG_ONLY",
            "owners": [
                {
                    "name": "nextMasu",
                    "type": "s16",
                    "target_register": "r29",
                    "candidate_register": "r28",
                    "evidence_sha256": "c" * 64,
                },
                {
                    "name": "battanF",
                    "type": "s16",
                    "target_register": "r28",
                    "candidate_register": "r27",
                    "evidence_sha256": "d" * 64,
                },
                {
                    "name": "blockedF",
                    "type": "BOOL",
                    "target_register": "r27",
                    "candidate_register": "r26",
                    "evidence_sha256": "e" * 64,
                },
                {
                    "name": "linkMasu",
                    "type": "int",
                    "target_register": "r26",
                    "candidate_register": "r29",
                    "evidence_sha256": "f" * 64,
                },
            ],
            "recommended_declaration_order": [
                "nextMasu",
                "battanF",
                "blockedF",
                "linkMasu",
            ],
            "candidate_record_sha256": (
                "27be0898fc890c69111b174d7055c7e57302643c3707437d536c308a900a2d02"
            ),
            "evidence_sha256": "0" * 64,
        },
    }


def _exact_sibling_transfer_report() -> dict[str, object]:
    target = [
        _instruction(100, "stwu r1, -80(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "bl mbBranchAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "extsh r3, r26", diff_kind="DIFF_INSERT"),
        _instruction(112, "bl mbMasuAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bne 0x90", diff_kind="DIFF_ARG_MISMATCH", branch_dest=144),
        _instruction(120, "bl mbBranchMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(124, "extsh r3, r26", diff_kind="DIFF_INSERT"),
        _instruction(128, "bl mbMasuMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "beq 0x94", diff_kind="DIFF_ARG_MISMATCH", branch_dest=148),
        _placeholder("DIFF_DELETE"),
        _placeholder("DIFF_DELETE"),
        _instruction(144, "li r27, 1", diff_kind="DIFF_INSERT"),
        _instruction(148, "li r27, 0", diff_kind="DIFF_INSERT"),
        _instruction(152, "extsh r3, r26", diff_kind="DIFF_INSERT"),
        _instruction(156, "bl mbMasuPosGet"),
        _instruction(160, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "bl mbMasuAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _placeholder("DIFF_INSERT"),
        _instruction(112, "bl mbBranchAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bne 0x88", diff_kind="DIFF_ARG_MISMATCH", branch_dest=136),
        _instruction(120, "bl mbMasuMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _placeholder("DIFF_INSERT"),
        _instruction(128, "bl mbBranchMAttrGet", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(132, "beq 0x90", diff_kind="DIFF_ARG_MISMATCH", branch_dest=144),
        _instruction(136, "li r27, 1", diff_kind="DIFF_DELETE"),
        _instruction(140, "li r27, 1", diff_kind="DIFF_DELETE"),
        _instruction(144, "li r27, 0", diff_kind="DIFF_DELETE"),
        _placeholder("DIFF_INSERT"),
        _placeholder("DIFF_INSERT"),
        _instruction(156, "bl mbMasuPosGet"),
        _instruction(160, "blr"),
    ]
    return _report(
        "mbev_CapMasuLinkNextRandomGet",
        target,
        candidate,
        target_size=428,
        candidate_size=436,
    )


def _exact_sibling_transfer_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _exact_sibling_transfer_report()
    expressions = [
        "mbMasuAttrGet(linkMasu) & mbBranchAttrGet()",
        "mbMasuMAttrGet(linkMasu) & mbBranchMAttrGet()",
    ]
    return {
        "schema": rules.EXACT_SIBLING_TRANSFER_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "physical_relocations_exact": True,
            "data_sections_exact": True,
            "protected_siblings_preserved": True,
            "donor_strict_exact": True,
            "donor_data_exact": True,
            "dependency_graph_equivalent": True,
            "capacity_authenticated": True,
            "pinned_mwcc_frontend": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "donor_record_sha256": "4" * 64,
            "dependency_graph_receipt_sha256": "5" * 64,
            "capacity_receipt_sha256": "6" * 64,
            "type_boundary_receipt_sha256": "7" * 64,
        },
        "donor": {
            "symbol": "mbev_CapMasuLinkNextGet",
            "source_location": "game/src/board/capevent.c:L5371",
            "transformation_class": "shared_boolean_call_order",
            "source_expressions": expressions,
            "candidate_record_sha256": (
                "27be0898fc890c69111b174d7055c7e57302643c3707437d536c308a900a2d02"
            ),
            "evidence_sha256": "8" * 64,
        },
        "baseline": {
            "mask_tests": [
                {
                    "source_left": "mbMasuAttrGet(linkMasu)",
                    "source_right": "mbBranchAttrGet()",
                    "source_expression": expressions[0],
                    "branch_getter": "mbBranchAttrGet",
                    "masu_getter": "mbMasuAttrGet",
                    "target_branch_call_row": 1,
                    "target_masu_call_row": 3,
                    "candidate_masu_call_row": 1,
                    "candidate_branch_call_row": 3,
                    "evidence_sha256": "9" * 64,
                },
                {
                    "source_left": "mbMasuMAttrGet(linkMasu)",
                    "source_right": "mbBranchMAttrGet()",
                    "source_expression": expressions[1],
                    "branch_getter": "mbBranchMAttrGet",
                    "masu_getter": "mbMasuMAttrGet",
                    "target_branch_call_row": 5,
                    "target_masu_call_row": 7,
                    "candidate_masu_call_row": 5,
                    "candidate_branch_call_row": 7,
                    "evidence_sha256": "a" * 64,
                },
            ],
            "shared_boolean": {
                "target_branch_rows": [4, 8],
                "target_true_assignment_row": 11,
                "target_false_assignment_row": 12,
                "candidate_true_assignment_rows": [9, 10],
                "candidate_false_assignment_row": 11,
                "result_register": "r27",
                "result_owner": "blockedF",
                "evidence_sha256": "b" * 64,
            },
        },
        "type_boundary": {
            "owner": "linkMasu",
            "source_type": "int",
            "consumer_type": "s16",
            "target_extsh_rows": [2, 6, 13],
            "target_consumer_call_rows": [3, 7, 14],
            "consumer_symbols": [
                "mbMasuAttrGet",
                "mbMasuMAttrGet",
                "mbMasuPosGet",
            ],
            "evidence_sha256": "c" * 64,
        },
        "capacity": {
            "array_name": "masuTbl",
            "macro": "MASU_LINK_MAX",
            "value": 5,
            "element_size": 2,
            "target_extent_bytes": 10,
            "source_location": "include/game/board/masu.h:L6",
            "evidence_sha256": "d" * 64,
        },
        "combined_cell": {
            "candidate_id": "capevent-masurandom001-exact",
            "target_size": 428,
            "candidate_size": 428,
            "object_sha256": (
                "5d2b15050f845f6c799bcaab562f812883b5c1f2e6aace24f0a0fbb2cf894e1a"
            ),
            "candidate_record_sha256": (
                "32d02ecbdd54b6eb08a34b282acf3eba750b9e60790976d5f17b4b7e834bac83"
            ),
        },
    }


def _wide_validation_narrow_result_report() -> dict[str, object]:
    target = [
        _instruction(100, "lhax r27, r3, r0", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "extsh r3, r27", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "bl mbMasuAttrGet"),
        _instruction(112, "extsh r3, r27", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bl mbMasuMAttrGet"),
        _instruction(120, "lhax r30, r3, r0"),
        _instruction(124, "mr r3, r30", diff_kind="DIFF_REPLACE"),
        _instruction(128, "mr r4, r26"),
        _instruction(132, "bl mbMasuPosGet"),
        _instruction(136, "mr r3, r30", diff_kind="DIFF_REPLACE"),
        _instruction(140, "blr"),
    ]
    candidate = [
        _instruction(100, "lhax r30, r3, r0", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "extsh r3, r30", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "bl mbMasuAttrGet"),
        _instruction(112, "extsh r3, r30", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bl mbMasuMAttrGet"),
        _instruction(120, "lhax r30, r3, r0"),
        _instruction(124, "extsh r3, r30", diff_kind="DIFF_REPLACE"),
        _instruction(128, "mr r4, r26"),
        _instruction(132, "bl mbMasuPosGet"),
        _instruction(136, "extsh r3, r30", diff_kind="DIFF_REPLACE"),
        _instruction(140, "blr"),
    ]
    return _report(
        "mbev_CapMasuValidPrevGet",
        target,
        candidate,
        target_size=284,
        candidate_size=284,
    )


def _wide_validation_narrow_result_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _wide_validation_narrow_result_report()
    wide_record = "7d90f425e405e4772db13660e76440ad807f9c3f96c46875c755f15caa885d6a"
    narrow_record = "afef08adbef83b354b3f7fa04205c11a40dc341aa1b66b7e1f9c2fe6ce49c069"
    exact_record = "fe8c625ce736b0c9a381354bda0db08e5def7d339b3c276789c9e5518fc4ba27"
    return {
        "schema": rules.WIDE_VALIDATION_NARROW_RESULT_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "exact_sibling_authenticated": True,
            "repeated_index_authenticated": True,
            "pinned_mwcc_frontend": True,
            "exact_result_verified": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "graph_receipt_sha256": "4" * 64,
            "graft_receipt_sha256": "5" * 64,
            "exact_sibling_record_sha256": (
                "27be0898fc890c69111b174d7055c7e57302643c3707437d536c308a900a2d02"
            ),
            "wide_control_record_sha256": wide_record,
            "narrow_control_record_sha256": narrow_record,
            "exact_result_record_sha256": exact_record,
        },
        "exact_sibling": {
            "symbol": "mbev_CapMasuLinkNextGet",
            "source_location": "game/src/board/capevent.c:L5371",
            "transformation_class": "shared_boolean_call_order",
            "source_expressions": [
                "mbMasuAttrGet(linkMasu) & mbBranchAttrGet()",
                "mbMasuMAttrGet(linkMasu) & mbBranchMAttrGet()",
            ],
            "candidate_record_sha256": (
                "27be0898fc890c69111b174d7055c7e57302643c3707437d536c308a900a2d02"
            ),
            "evidence_sha256": "6" * 64,
        },
        "repeated_load": {
            "array_name": "linkTbl",
            "index_owner": "i",
            "element_type": "s16",
            "target_rows": [0, 5],
            "candidate_rows": [0, 5],
            "target_registers": ["r27", "r30"],
            "candidate_registers": ["r30", "r30"],
            "address_registers": ["r3", "r0"],
            "evidence_sha256": "7" * 64,
        },
        "validation_identity": {
            "owner": "linkMasu",
            "source_type": "int",
            "consumer_type": "s16",
            "load_row": 0,
            "target_register": "r27",
            "candidate_register": "r30",
            "normalization_rows": [1, 3],
            "consumer_call_rows": [2, 4],
            "consumer_symbols": ["mbMasuAttrGet", "mbMasuMAttrGet"],
            "evidence_sha256": "8" * 64,
        },
        "selected_identity": {
            "owner": "prevMasu",
            "source_type": "s16",
            "load_row": 5,
            "target_register": "r30",
            "candidate_register": "r30",
            "argument_row": 6,
            "consumer_call_row": 8,
            "consumer_symbol": "mbMasuPosGet",
            "return_row": 9,
            "target_has_no_normalization": True,
            "evidence_sha256": "9" * 64,
        },
        "controls": [
            {
                "kind": "wide_only",
                "candidate_id": "capevent-masuvalidprev001-wide-local",
                "target_size": 284,
                "candidate_size": 284,
                "strict_exact": False,
                "object_sha256": "a" * 64,
                "strict_report_sha256": "b" * 64,
                "data_report_sha256": "c" * 64,
                "candidate_record_sha256": wide_record,
                "unresolved_boundary": "selected_result_normalization",
            },
            {
                "kind": "narrow_only",
                "candidate_id": "capevent-masuvalidprev002-narrow-local",
                "target_size": 284,
                "candidate_size": 284,
                "strict_exact": False,
                "object_sha256": "d" * 64,
                "strict_report_sha256": "e" * 64,
                "data_report_sha256": "f" * 64,
                "candidate_record_sha256": narrow_record,
                "unresolved_boundary": "validation_normalization",
            },
        ],
        "combined_cell": {
            "candidate_id": "capevent-masuvalidprev003-exact",
            "target_size": 284,
            "candidate_size": 284,
            "strict_exact": True,
            "data_exact": True,
            "physical_relocations": "8/8",
            "source_sha256": "1" * 64,
            "object_sha256": "2" * 64,
            "strict_report_sha256": "3" * 64,
            "data_report_sha256": "4" * 64,
            "candidate_record_sha256": exact_record,
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


def _stack_gap_capacity_report() -> dict[str, object]:
    target_text = [
        "lwz r3, 0x58(r1)",
        "lfs f1, 0x5c(r1)",
        "addi r4, r1, 0x60",
        "stw r5, 0x64(r1)",
        "blr",
    ]
    candidate_text = [
        "lwz r3, 0x28(r1)",
        "lfs f1, 0x2c(r1)",
        "addi r4, r1, 0x30",
        "stw r5, 0x34(r1)",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target.append(_instruction(100 + index * 4, left, diff_kind=kind))
        candidate.append(_instruction(100 + index * 4, right, diff_kind=kind))
    return _report(
        "mbev_CapPatapata",
        target,
        candidate,
        target_size=6868,
        candidate_size=6868,
    )


def _stack_gap_attribution_report() -> dict[str, object]:
    target_text = [
        "frsp f31, f1",
        "fmr f1, f31",
        "cmpwi r31, 0xa",
        "addi r4, r1, 0x58",
        "lfs f0, 0x58(r1)",
        "lfs f1, 0x5c(r1)",
        "lfs f2, 0x60(r1)",
    ]
    candidate_text = [
        "frsp f30, f1",
        "fmr f1, f30",
        "cmpwi r30, 0xa",
        "addi r4, r1, 0x4c",
        "lfs f0, 0x4c(r1)",
        "lfs f1, 0x50(r1)",
        "lfs f2, 0x54(r1)",
    ]
    target = [
        _instruction(100 + index * 4, text, diff_kind="DIFF_ARG_MISMATCH")
        for index, text in enumerate(target_text)
    ]
    candidate = [
        _instruction(100 + index * 4, text, diff_kind="DIFF_ARG_MISMATCH")
        for index, text in enumerate(candidate_text)
    ]
    return _report(
        "mbev_CapPatapata",
        target,
        candidate,
        target_size=6868,
        candidate_size=6868,
    )


def _stack_gap_capacity_context(
    capacity_report: dict[str, object] | None = None,
    attribution_report: dict[str, object] | None = None,
) -> dict[str, object]:
    capacity_bound = (
        capacity_report if capacity_report is not None else _stack_gap_capacity_report()
    )
    attribution_bound = (
        attribution_report
        if attribution_report is not None
        else _stack_gap_attribution_report()
    )
    return {
        "schema": rules.STACK_GAP_CAPACITY_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "pinned_mwcc_frontend": True,
            "capacity_result_verified": True,
            "exact_result_verified": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "stack_gap_receipt_sha256": "4" * 64,
            "capacity_provenance_receipt_sha256": "5" * 64,
            "capacity_candidate_record_sha256": "6" * 64,
            "exact_result_report_sha256": "7" * 64,
            "exact_result_record_sha256": "8" * 64,
        },
        "capacity_stage": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(capacity_bound)),
            "uniform_gap": {
                "target_minus_candidate_bytes": 48,
                "minimum_row_count": 3,
            },
            "array": {
                "name": "motFile",
                "element_size": 4,
                "candidate_capacity": 4,
                "used_prefix_elements": 4,
                "candidate_extent_bytes": 16,
                "target_extent_bytes": 64,
                "source_expression": "int motFile[16]",
            },
            "capacity_sources": [
                {
                    "provider": "mbev_CapBomheiTrap",
                    "source_location": "game/src/board/captrap.c:L1803",
                    "capacity": 16,
                    "relationship": "same_game_exact",
                    "strict_exact": True,
                    "evidence_sha256": "9" * 64,
                },
                {
                    "provider": "mbev_CapJango",
                    "source_location": "game/src/board/capthrow.c:L900",
                    "capacity": 16,
                    "relationship": "same_owner_exact",
                    "strict_exact": True,
                    "evidence_sha256": "a" * 64,
                },
            ],
        },
        "attribution_stage": {
            "objdiff_canonical_sha256": rules._sha256(
                rules._canonical(attribution_bound)
            ),
            "residual_rows": [0, 1, 2, 3, 4, 5, 6],
            "attributions": [
                {
                    "kind": "live_value_reuse",
                    "target_owner": "t",
                    "candidate_owner": "weight",
                    "row_indices": [0, 1],
                    "source_expression": "t = cosResult",
                    "source_location": "game/src/board/capthrow.c:L1400",
                    "provenance_refs": ["same_owner_live_value"],
                    "evidence_sha256": "b" * 64,
                },
                {
                    "kind": "historical_condition_owner",
                    "target_owner": "i",
                    "candidate_owner": "j",
                    "row_indices": [2],
                    "source_expression": "if (i == 10)",
                    "source_location": "game/src/board/capthrow.c:L1450",
                    "provenance_refs": ["02bc5244", "9bf815b9"],
                    "evidence_sha256": "c" * 64,
                },
                {
                    "kind": "live_stack_object_reuse",
                    "target_owner": "tempPos",
                    "candidate_owner": "landingPos",
                    "row_indices": [3, 4, 5, 6],
                    "source_expression": "tempPos = chaseTarget",
                    "source_location": "game/src/board/capthrow.c:L1460",
                    "provenance_refs": ["same_owner_live_stack_object"],
                    "evidence_sha256": "d" * 64,
                },
            ],
        },
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


def _metadata_owner_report() -> dict[str, object]:
    relocation = {
        "type": 109,
        "type_name": "R_PPC_EMB_SDA21",
        "addend": 0,
    }
    instructions = [
        _instruction(100, "lbz r3, lbl_802C324C@sda21(r13)", relocation=relocation),
        _instruction(104, "addi r3, r3, 1"),
        _instruction(108, "blr"),
    ]
    report = _report(
        "PlayerBiriQOMExec",
        instructions,
        json.loads(json.dumps(instructions)),
        target_size=1140,
        candidate_size=1140,
    )
    report["left"]["symbols"][0]["match_percent"] = 100.0  # type: ignore[index]
    report["right"]["symbols"][0]["match_percent"] = 100.0  # type: ignore[index]
    return report


def _metadata_owner_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _metadata_owner_report()

    def merged_object(base: int) -> dict[str, object]:
        return {
            "name": f"lbl_{base:08X}",
            "address": base,
            "size": 4,
            "data_kind": "byte",
            "removed_interior_labels": [
                {"name": f"lbl_{base + offset:08X}", "address": base + offset}
                for offset in range(1, 4)
            ],
        }

    return {
        "schema": rules.METADATA_OWNER_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "focus_strict_exact": True,
            "focus_data_exact": True,
            "source_unchanged": True,
            "candidate_object_unchanged": True,
            "payload_sections_equal": True,
            "physical_relocation_keys_equal": True,
            "effective_targets_equal": True,
            "protected_siblings_preserved": True,
            "linked_retail_exact": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "source_sha256": "3" * 64,
            "candidate_object_sha256": "4" * 64,
            "prior_target_object_sha256": "5" * 64,
            "corrected_target_object_sha256": "6" * 64,
            "metadata_before_sha256": "7" * 64,
            "metadata_after_sha256": "8" * 64,
            "relocation_identity_receipt_sha256": "9" * 64,
            "linked_retail_receipt_sha256": "a" * 64,
        },
        "metadata": {
            "section": ".sdata",
            "objects": [
                merged_object(0x802C324C),
                merged_object(0x802C3250),
                merged_object(0x802C3258),
            ],
            "attribution_changes_outside_objects": 0,
        },
        "relocations": {
            "prior_rows": 2249,
            "corrected_rows": 2249,
            "name_rebindings": 9,
            "effective_target_differences": 0,
        },
        "focus_functions": [
            {
                "name": "PlayerBiriQOMExec",
                "target_bytes": 1140,
                "candidate_bytes": 1140,
                "physical_relocations": 49,
            },
            {
                "name": "mbPlayerBiriQSet",
                "target_bytes": 512,
                "candidate_bytes": 512,
                "physical_relocations": 40,
            },
        ],
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


def _pool_live_range_report() -> dict[str, object]:
    target_relocation = {
        "type": 109,
        "type_name": "R_PPC_EMB_SDA21",
        "addend": 0,
    }
    candidate_relocation = dict(target_relocation)
    target = [
        _instruction(100, "stwu r1, -96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "fmr f30, f1", diff_kind="DIFF_DELETE"),
        _instruction(108, "addi r3, r3, 1", diff_kind="DIFF_REPLACE"),
        _instruction(112, "lfs f1, 32(r31)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "lfs f0, 0(r30)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(
            120,
            "lfs f2, lbl_802C46D8@sda21(r13)",
            relocation=target_relocation,
        ),
        _instruction(
            124,
            "lfs f3, lbl_802C46D8@sda21(r13)",
            relocation=target_relocation,
        ),
        _instruction(128, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -80(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _placeholder(),
        _instruction(108, "lwz r3, 8(r3)", diff_kind="DIFF_REPLACE"),
        _instruction(112, "lfs f0, 0(r30)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "lfs f1, 32(r31)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(
            120,
            "lfs f2, @anonymous@sda21(r13)",
            relocation=candidate_relocation,
        ),
        _instruction(
            124,
            "lfs f3, @anonymous@sda21(r13)",
            relocation=candidate_relocation,
        ),
        _instruction(128, "blr"),
    ]
    return _report(
        "mbev_CapEffGlowOMExec",
        target,
        candidate,
        target_size=1080,
        candidate_size=1072,
    )


def _pool_live_range_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _pool_live_range_report()
    return {
        "schema": rules.POOL_LIVE_RANGE_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "protected_siblings_preserved": True,
            "pinned_mwcc_frontend": True,
            "row_groups_disjoint": True,
            "pool_values_equivalent": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "pool_decoder_receipt_sha256": "4" * 64,
            "same_tu_owner_receipt_sha256": "5" * 64,
            "source_range_receipt_sha256": "6" * 64,
        },
        "residual_groups": {
            "live_range_rows": [0, 1, 2],
            "comparison_rows": [3, 4],
            "pool_owner_rows": [5, 6],
        },
        "pool_owner": {
            "decoder_schema": "match_workbench_pool_decoder/v1",
            "symbol": "lbl_802C46D8",
            "value_type": "f32",
            "value_bits": "41200000",
            "target_consumer_count": 2,
            "source_location": "game/src/board/capevent.c:L4894",
        },
        "source_actions": {
            "live_temporaries": ["phaseRatio", "fadeFactor", "oneMinus"],
            "preincrement_expression": "consume ++particleWorkP->cycle directly",
            "comparison_expression": "if (particleWorkP->gravity)",
            "pool_expression": "use lbl_802C46D8 for both authenticated consumers",
        },
        "precursor": {
            "candidate_id": "capevent-glowom001-arithmetic",
            "target_size": 1080,
            "candidate_size": 1080,
            "object_sha256": "7" * 64,
            "candidate_record_sha256": "8" * 64,
            "residual_rows": [3, 4, 5, 6],
        },
        "combined_cell": {
            "candidate_id": "capevent-glowom002-exact",
            "target_size": 1080,
            "candidate_size": 1080,
            "object_sha256": (
                "86d08e3dd8234f322ffa591c0c2d45fd35b020281483824ea70b60e16e01e5f4"
            ),
            "candidate_record_sha256": (
                "1798b5e31546aeae1b796ebe110bd7d7d9b806432e50efdddf1e3a56556287c4"
            ),
            "residual_rows": [],
        },
    }


def _float_truthiness_report() -> dict[str, object]:
    target = [
        _instruction(100, "lfs f1, 32(r31)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "lfs f0, 0(r2)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "fcmpu cr0, f1, f0"),
        _instruction(112, "beq 128"),
        _instruction(116, "blr"),
    ]
    candidate = [
        _instruction(100, "lfs f0, 0(r2)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "lfs f1, 32(r31)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(108, "fcmpu cr0, f1, f0"),
        _instruction(112, "beq 128"),
        _instruction(116, "blr"),
    ]
    return _report(
        "mbev_CapStarManOMExec",
        target,
        candidate,
        target_size=632,
        candidate_size=632,
    )


def _float_truthiness_context(
    report: dict[str, object] | None = None,
) -> dict[str, object]:
    bound_report = report if report is not None else _float_truthiness_report()
    return {
        "schema": rules.FLOAT_TRUTHINESS_CONTEXT_SCHEMA,
        "proofs": {
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(bound_report)),
            "function_size_exact": True,
            "stack_frame_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "cfg_calls_exact": True,
            "all_non_comparison_rows_exact": True,
            "protected_siblings_preserved": True,
            "pinned_mwcc_frontend": True,
            "exact_precedent_authenticated": True,
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "physical_relocation_receipt_sha256": "3" * 64,
            "neutral_observation_receipt_sha256": "4" * 64,
            "exact_precedent_receipt_sha256": "5" * 64,
            "source_range_receipt_sha256": "6" * 64,
        },
        "comparison": {
            "rows": [0, 1],
            "compare_row": 2,
            "branch_row": 3,
            "field_access": {"base_register": "r31", "offset": 32},
            "zero_access": {"base_register": "r2", "offset": 0},
            "field_expression": "workP->_unk20",
            "truthiness_expression": "if (workP->_unk20)",
        },
        "neutral_observation": {
            "axis": "commuted_explicit_zero_comparison",
            "baseline_expression": "workP->_unk20 != 0.0f",
            "commuted_expression": "0.0f != workP->_unk20",
            "baseline_object_sha256": "7" * 64,
            "commuted_object_sha256": "7" * 64,
        },
        "exact_precedent": {
            "symbol": "mbev_CapEffGlowOMExec",
            "source_location": "game/src/board/capevent.c:L7778",
            "source_expression": "if (particleWorkP->gravity)",
            "candidate_record_sha256": "8" * 64,
        },
        "exact_cell": {
            "candidate_id": "capevent-starman002-exact",
            "target_size": 632,
            "candidate_size": 632,
            "object_sha256": (
                "adb60fea3670631e7a7e2d6d10df4ecfe918c4b5b1d9315b8d5172a51f0d6395"
            ),
            "candidate_record_sha256": (
                "d5c3f8edc2d69568ec1659ba4d657556759b308017fc86dd0f08e0ce099f80ad"
            ),
            "residual_rows": [],
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
    def test_metadata_owner_coherence_ranks_target_metadata_audit(self) -> None:
        report = _metadata_owner_report()
        context = _metadata_owner_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="PlayerBiriQOMExec",
            metadata_owner_context=context,
        )
        diagnosis = _evaluation(result, "metadata_owner_coherence")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["confidence"], 0.99)
        self.assertEqual(
            diagnosis["source_class"],
            "target_metadata_owner_merge",
        )
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["section"], ".sdata")
        self.assertEqual(evidence["removed_interior_label_count"], 9)
        self.assertEqual(evidence["relocations"]["name_rebindings"], 9)
        self.assertFalse(result["authority_advanced"])

    def test_metadata_owner_coherence_fails_closed_without_bound_proof(self) -> None:
        report = _metadata_owner_report()
        no_context = rules.diagnose_document(
            report,
            focus_symbol="PlayerBiriQOMExec",
        )
        self.assertFalse(
            _evaluation(no_context, "metadata_owner_coherence")["matched"]
        )

        wrong_report = _metadata_owner_context(report)
        wrong_report["proofs"]["objdiff_canonical_sha256"] = "0" * 64  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="PlayerBiriQOMExec",
            metadata_owner_context=wrong_report,
        )
        self.assertFalse(
            _evaluation(result, "metadata_owner_coherence")["matched"]
        )

        false_proof = _metadata_owner_context(report)
        false_proof["proofs"]["payload_sections_equal"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "payload_sections_equal"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=false_proof,
            )

        unlisted_focus = _metadata_owner_context(report)
        unlisted_focus["focus_functions"] = [  # type: ignore[index]
            unlisted_focus["focus_functions"][1]  # type: ignore[index]
        ]
        result = rules.diagnose_document(
            report,
            focus_symbol="PlayerBiriQOMExec",
            metadata_owner_context=unlisted_focus,
        )
        self.assertFalse(
            _evaluation(result, "metadata_owner_coherence")["matched"]
        )

        residual_report = _metadata_owner_report()
        residual_report["left"]["symbols"][0]["match_percent"] = 99.0  # type: ignore[index]
        residual_context = _metadata_owner_context(residual_report)
        result = rules.diagnose_document(
            residual_report,
            focus_symbol="PlayerBiriQOMExec",
            metadata_owner_context=residual_context,
        )
        self.assertFalse(
            _evaluation(result, "metadata_owner_coherence")["matched"]
        )

    def test_metadata_owner_context_rejects_incoherent_extent_or_relocations(self) -> None:
        report = _metadata_owner_report()

        missing_label = _metadata_owner_context(report)
        missing_label["metadata"]["objects"][0]["removed_interior_labels"].pop()  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "every removed interior"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=missing_label,
            )

        overlapping = _metadata_owner_context(report)
        overlapping["metadata"]["objects"][1]["address"] = 0x802C324F  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "must not overlap"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=overlapping,
            )

        changed_rows = _metadata_owner_context(report)
        changed_rows["relocations"]["corrected_rows"] = 2248  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "row counts"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=changed_rows,
            )

        changed_targets = _metadata_owner_context(report)
        changed_targets["relocations"]["effective_target_differences"] = 1  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "effective-target"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=changed_targets,
            )

        wrong_rebindings = _metadata_owner_context(report)
        wrong_rebindings["relocations"]["name_rebindings"] = 8  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "name rebindings"):
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=wrong_rebindings,
            )

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

    def test_parameter_allocation_rule_transfers_to_same_tu_bonus_coin(self) -> None:
        report = _parameter_allocation_report()
        for side in ("left", "right"):
            report[side]["symbols"][0]["name"] = "mbev_CapBonusCoin"  # type: ignore[index]
            report[side]["symbols"][0]["size"] = "244"  # type: ignore[index]
        context = _parameter_allocation_context(report)
        context["consumer_chain"]["field_owner"] = "process"  # type: ignore[index]
        context["consumer_chain"]["field_name"] = "property"  # type: ignore[index]

        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapBonusCoin",
            parameter_allocation_context=context,
        )
        diagnosis = _evaluation(result, "parameter_allocation_consumer_chain")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["source_expression"],
            "workP = process->property = workData",
        )
        self.assertFalse(result["authority_advanced"])

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

    def test_aggregate_use_multiplicity_ranks_complete_copy_groups(self) -> None:
        report = _aggregate_use_report()
        context = _aggregate_use_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffExplodeKillerAdd",
            aggregate_use_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_use_multiplicity")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],
            {"r29": "r31", "r30": "r29", "r31": "r30"},
        )
        self.assertEqual(
            diagnosis["evidence"]["source_expressions"],
            ["color1 = *color", "color2 = *color"],
        )
        self.assertEqual(
            diagnosis["evidence"]["suppressed_axes"],
            ["input_pointer_aliases", "parameter_declaration_order"],
        )
        self.assertFalse(result["authority_advanced"])

    def test_aggregate_use_multiplicity_preserves_independent_consumers(self) -> None:
        report = _aggregate_use_report()
        for side in ("left", "right"):
            report[side]["symbols"][0]["name"] = "mbev_CapEffBoostAdd"  # type: ignore[index]
            report[side]["symbols"][0]["size"] = "436"  # type: ignore[index]
        context = _aggregate_use_context(
            report,
            copy_count=1,
            independent_consumers=[
                {
                    "expression": "particleWorkP->alpha = color->a",
                    "fields": ["a"],
                    "evidence_sha256": "e" * 64,
                }
            ],
        )
        context["copy_groups"][0]["destination"] = "particleWorkP->color"  # type: ignore[index]
        context["copy_groups"][0]["consumer"] = "particleWorkP->color"  # type: ignore[index]

        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffBoostAdd",
            aggregate_use_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_use_multiplicity")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["source_expressions"],
            ["particleWorkP->color = *color"],
        )
        self.assertEqual(
            diagnosis["evidence"]["preserved_independent_consumers"][0]["expression"],
            "particleWorkP->alpha = color->a",
        )
        self.assertIn("Preserve", diagnosis["recommendation"])

    def test_aggregate_use_multiplicity_transfers_to_glow_add(self) -> None:
        report = _aggregate_use_report()
        for side in ("left", "right"):
            report[side]["symbols"][0]["name"] = "mbev_CapEffGlowAdd"  # type: ignore[index]
            report[side]["symbols"][0]["size"] = "504"  # type: ignore[index]
        context = _aggregate_use_context(report, copy_count=1)
        context["copy_groups"][0]["destination"] = "particleWorkP->color"  # type: ignore[index]
        context["copy_groups"][0]["consumer"] = "particle work color"  # type: ignore[index]

        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowAdd",
            aggregate_use_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_use_multiplicity")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["source_expressions"],
            ["particleWorkP->color = *color"],
        )
        self.assertEqual(diagnosis["evidence"]["preserved_independent_consumers"], [])

    def test_aggregate_use_multiplicity_fails_closed(self) -> None:
        report = _aggregate_use_report()
        no_context = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffExplodeKillerAdd",
        )
        self.assertFalse(
            _evaluation(no_context, "aggregate_use_multiplicity")["matched"]
        )

        incomplete_copy = _aggregate_use_context(report)
        incomplete_copy["copy_groups"][0]["fields"] = ["r", "g", "b"]  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "complete sealed aggregate"
        ):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeKillerAdd",
                aggregate_use_context=incomplete_copy,
            )

        wrong_owner = _aggregate_use_context(report)
        wrong_owner["owners"][0]["candidate_register"] = "r28"  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "complete register cycle"
        ):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeKillerAdd",
                aggregate_use_context=wrong_owner,
            )

        operation_difference = _aggregate_use_report()
        operation_difference["right"]["symbols"][0]["instructions"][4]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "addi r3, r30, 0"
        context = _aggregate_use_context(operation_difference)
        result = rules.diagnose_document(
            operation_difference,
            focus_symbol="mbev_CapEffExplodeKillerAdd",
            aggregate_use_context=context,
        )
        self.assertFalse(_evaluation(result, "aggregate_use_multiplicity")["matched"])

        malformed_consumer = _aggregate_use_context(report)
        malformed_consumer["independent_consumers"] = [
            {
                "expression": "particleWorkP->alpha = color->unknown",
                "fields": ["unknown"],
                "evidence_sha256": "e" * 64,
            }
        ]
        with self.assertRaisesRegex(rules.LearningInputError, "unique subset"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeKillerAdd",
                aggregate_use_context=malformed_consumer,
            )

        unsafe_destination = _aggregate_use_context(report)
        unsafe_destination["copy_groups"][0]["destination"] = "color1; injected()"  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "identifier/member lvalue"
        ):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeKillerAdd",
                aggregate_use_context=unsafe_destination,
            )

    def test_aggregate_two_owner_followup_schedules_declaration_only(self) -> None:
        report = _aggregate_followup_report()
        context = _aggregate_followup_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffElectricAdd",
            aggregate_followup_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_two_owner_followup")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],
            {"r23": "r24", "r24": "r23"},
        )
        self.assertEqual(
            diagnosis["evidence"]["recommended_cells"],
            [
                {
                    "declaration_chronology": ["particleSystemP", "modelP"],
                    "expression_topology": "split",
                }
            ],
        )
        self.assertEqual(len(diagnosis["evidence"]["suppressed_cells"]), 2)
        self.assertIn("do not combine", diagnosis["recommendation"])
        self.assertFalse(result["authority_advanced"])

    def test_aggregate_two_owner_followup_fails_closed(self) -> None:
        report = _aggregate_followup_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffElectricAdd"
        )
        self.assertFalse(
            _evaluation(no_context, "aggregate_two_owner_followup")["matched"]
        )

        unmeasured_fusion = _aggregate_followup_context(report)
        unmeasured_fusion["fusion_observation"]["candidate_size"] = 496  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "measured size change"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffElectricAdd",
                aggregate_followup_context=unmeasured_fusion,
            )

        wrong_mapping = _aggregate_followup_context(report)
        wrong_mapping["owners"][0]["candidate_register"] = "r22"  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "complete two-register swap"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffElectricAdd",
                aggregate_followup_context=wrong_mapping,
            )

        operation_difference = _aggregate_followup_report()
        operation_difference["right"]["symbols"][0]["instructions"][3]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "addi r3, r23, 0"
        context = _aggregate_followup_context(operation_difference)
        result = rules.diagnose_document(
            operation_difference,
            focus_symbol="mbev_CapEffElectricAdd",
            aggregate_followup_context=context,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_two_owner_followup")["matched"]
        )

    def test_address_taken_local_pointer_ranks_one_live_typed_owner(self) -> None:
        report = _address_taken_report()
        context = _address_taken_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowKinokoAddAlt",
            address_taken_context=context,
        )
        diagnosis = _evaluation(result, "address_taken_local_pointer_consumer")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["evidence"]["size_delta"], 12)
        self.assertEqual(diagnosis["evidence"]["target_home_row"], 1)
        self.assertEqual(diagnosis["evidence"]["target_incoming_row"], 2)
        self.assertEqual(diagnosis["evidence"]["candidate_incoming_row"], 1)
        self.assertEqual(diagnosis["evidence"]["target_materialization_row"], 4)
        self.assertEqual(diagnosis["evidence"]["target_copy_row"], 5)
        self.assertEqual(
            diagnosis["evidence"]["candidate_direct_materialization_row"], 3
        )
        self.assertIn("posLocalP = &pos", diagnosis["evidence"]["source_expression"])
        self.assertEqual(
            diagnosis["evidence"]["suppressed_axes"],
            [
                "declaration_order_only",
                "dead_pointer_storage",
                "artificial_lifetime_extension",
            ],
        )
        self.assertFalse(result["authority_advanced"])

    def test_address_taken_local_pointer_fails_closed(self) -> None:
        report = _address_taken_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffGlowKinokoAddAlt"
        )
        self.assertFalse(
            _evaluation(no_context, "address_taken_local_pointer_consumer")["matched"]
        )

        wrong_offset = _address_taken_context(report)
        wrong_offset["aggregate"]["stack_offset"] = 20  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowKinokoAddAlt",
            address_taken_context=wrong_offset,
        )
        self.assertFalse(
            _evaluation(result, "address_taken_local_pointer_consumer")["matched"]
        )

        missing_home = _address_taken_report()
        missing_home["left"]["symbols"][0]["instructions"][1]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "stw r3, 12(r1)"
        context = _address_taken_context(missing_home)
        result = rules.diagnose_document(
            missing_home,
            focus_symbol="mbev_CapEffGlowKinokoAddAlt",
            address_taken_context=context,
        )
        self.assertFalse(
            _evaluation(result, "address_taken_local_pointer_consumer")["matched"]
        )

        wrong_owner = _address_taken_context(report)
        wrong_owner["local_pointer"]["target_register"] = "r29"  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "candidate incoming-pointer color"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowKinokoAddAlt",
                address_taken_context=wrong_owner,
            )

    def test_aggregate_snapshot_pointer_chain_schedules_one_composed_cell(
        self,
    ) -> None:
        report = _aggregate_snapshot_pointer_report()
        context = _aggregate_snapshot_pointer_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffRingHitAdd",
            aggregate_snapshot_pointer_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_snapshot_pointer_chain")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["evidence"]["target_frame"], 96)
        self.assertEqual(
            diagnosis["evidence"]["mismatch_rows"], [24, 38, 40, 42]
        )
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],
            {"r28": "r30", "r30": "r28"},
        )
        self.assertEqual(
            diagnosis["evidence"]["recommended_cells"][0][
                "pointer_declaration_chronology"
            ],
            ["scaleLocalP", "rotLocalP", "posLocalP"],
        )
        self.assertIn("aggregate_snapshot_only", diagnosis["evidence"]["suppressed_axes"])
        self.assertFalse(result["authority_advanced"])

    def test_aggregate_snapshot_pointer_chain_fails_closed(self) -> None:
        report = _aggregate_snapshot_pointer_report()
        no_context = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffRingHitAdd",
        )
        self.assertFalse(
            _evaluation(no_context, "aggregate_snapshot_pointer_chain")["matched"]
        )

        wrong_copy = copy.deepcopy(report)
        wrong_copy["left"]["symbols"][0]["instructions"][18]["instruction"][  # type: ignore[index]
            "formatted"
        ] = (
            "lwz r3, 0x0(r26)"
        )
        context = _aggregate_snapshot_pointer_context(wrong_copy)
        result = rules.diagnose_document(
            wrong_copy,
            focus_symbol="mbev_CapEffRingHitAdd",
            aggregate_snapshot_pointer_context=context,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_snapshot_pointer_chain")["matched"]
        )

        extra_residual = copy.deepcopy(report)
        for side in ("left", "right"):
            extra_residual[side]["symbols"][0]["instructions"][41]["diff_kind"] = (  # type: ignore[index]
                "DIFF_ARG_MISMATCH"
            )
        context = _aggregate_snapshot_pointer_context(extra_residual)
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapEffRingHitAdd",
            aggregate_snapshot_pointer_context=context,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_snapshot_pointer_chain")["matched"]
        )

        invalid_control = _aggregate_snapshot_pointer_context(report)
        invalid_control["controls"][1]["candidate_record_sha256"] = "f" * 64  # type: ignore[index]
        with self.assertRaises(rules.LearningInputError):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffRingHitAdd",
                aggregate_snapshot_pointer_context=invalid_control,
            )

    def test_typed_aggregate_copy_lowering_schedules_one_member_cell(self) -> None:
        report = _typed_aggregate_copy_report()
        context = _typed_aggregate_copy_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffDustMultiAdd",
            typed_aggregate_copy_context=context,
        )
        diagnosis = _evaluation(result, "typed_aggregate_copy_lowering")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["evidence"]["target_frame"], 288)
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],
            {
                "r23": "r24",
                "r24": "r25",
                "r25": "r26",
                "r26": "r27",
                "r27": "r28",
                "r28": "r29",
                "r29": "r30",
                "r30": "r23",
            },
        )
        self.assertEqual(len(diagnosis["evidence"]["typed_copy_rows"]), 6)
        self.assertIn("posTemp.x = posP->x", diagnosis["recommendation"])
        self.assertEqual(len(diagnosis["evidence"]["recommended_cells"]), 1)
        self.assertIn(
            "declaration_order_permutations",
            diagnosis["evidence"]["suppressed_axes"],
        )
        self.assertFalse(result["authority_advanced"])

    def test_typed_aggregate_copy_lowering_fails_closed(self) -> None:
        report = _typed_aggregate_copy_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffDustMultiAdd"
        )
        self.assertFalse(
            _evaluation(no_context, "typed_aggregate_copy_lowering")["matched"]
        )

        exact_report = copy.deepcopy(report)
        exact_report["right"]["symbols"][0]["instructions"] = copy.deepcopy(  # type: ignore[index]
            exact_report["left"]["symbols"][0]["instructions"]  # type: ignore[index]
        )
        context = _typed_aggregate_copy_context(exact_report)
        result = rules.diagnose_document(
            exact_report,
            focus_symbol="mbev_CapEffDustMultiAdd",
            typed_aggregate_copy_context=context,
        )
        self.assertFalse(
            _evaluation(result, "typed_aggregate_copy_lowering")["matched"]
        )

        wrong_offset = copy.deepcopy(report)
        wrong_offset["left"]["symbols"][0]["instructions"][8]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lfs f0, 0xc(r30)"
        context = _typed_aggregate_copy_context(wrong_offset)
        result = rules.diagnose_document(
            wrong_offset,
            focus_symbol="mbev_CapEffDustMultiAdd",
            typed_aggregate_copy_context=context,
        )
        self.assertFalse(
            _evaluation(result, "typed_aggregate_copy_lowering")["matched"]
        )

        broken_cycle = copy.deepcopy(report)
        broken_cycle["right"]["symbols"][0]["instructions"][14]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lwz r30, 0x8c(r1)"
        context = _typed_aggregate_copy_context(broken_cycle)
        result = rules.diagnose_document(
            broken_cycle,
            focus_symbol="mbev_CapEffDustMultiAdd",
            typed_aggregate_copy_context=context,
        )
        self.assertFalse(
            _evaluation(result, "typed_aggregate_copy_lowering")["matched"]
        )

        false_proof = _typed_aggregate_copy_context(report)
        false_proof["proofs"]["owner_cycle_authenticated"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "owner_cycle_authenticated"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffDustMultiAdd",
                typed_aggregate_copy_context=false_proof,
            )


    def test_dform_copy_helper_closes_existing_owner_cycle(self) -> None:
        report = _dform_copy_owner_cycle_report()
        context = _dform_copy_helper_context(
            report, mode="existing_owner_cycle"
        )
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_PlayerColBall",
            dform_copy_helper_context=context,
        )
        diagnosis = _evaluation(result, "dform_aggregate_copy_helper_boundary")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["source_class"],
            "dform_copy_helper_existing_owner_interaction",
        )
        evidence = diagnosis["evidence"]
        self.assertEqual(
            evidence["observed_target_lowering"],
            ["psq_l", "lfs", "psq_st", "stfs"],
        )
        self.assertEqual(
            evidence["observed_candidate_lowering"],
            ["psq_lx", "lfs", "psq_stx", "stfs"],
        )
        self.assertEqual(
            evidence["owner_mapping"],
            {"r25": "r26", "r26": "r27", "r27": "r25"},
        )
        self.assertEqual(
            evidence["recommended_cells"][0]["reuse_existing_identities"],
            ["j", "i", "temp"],
        )
        self.assertIn(
            "fresh_local_identities", evidence["suppressed_axes"]
        )
        self.assertFalse(result["authority_advanced"])

    def test_dform_copy_helper_closes_stack_interval_trace(self) -> None:
        report = _dform_copy_trace_report()
        context = _dform_copy_helper_context(
            report, mode="stack_interval_trace"
        )
        result = rules.diagnose_document(
            report,
            focus_symbol="MoveNumOMExec",
            dform_copy_helper_context=context,
        )
        diagnosis = _evaluation(result, "dform_aggregate_copy_helper_boundary")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["source_class"],
            "traced_stack_interval_dform_copy_helper_boundary",
        )
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["source_interval"], {"base": "r1", "start": 8, "end": 20})
        self.assertEqual(
            evidence["destination_interval"],
            {"base": "r1", "start": 20, "end": 32},
        )
        self.assertEqual(
            evidence["dependencies"],
            [[113, 115], [114, 116], [117, 118]],
        )
        self.assertEqual(evidence["seam_unknown_count"], 0)
        self.assertFalse(evidence["paired_codegen_proof"])
        self.assertIn("new_live_capture", evidence["suppressed_axes"])
        self.assertEqual(
            evidence["recommended_cells"][0]["helper_expression"],
            "HuVecCopy(&posNorm, &pos)",
        )
        self.assertFalse(result["authority_advanced"])

    def test_dform_copy_helper_fails_closed(self) -> None:
        report = _dform_copy_trace_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="MoveNumOMExec"
        )
        self.assertFalse(
            _evaluation(
                no_context, "dform_aggregate_copy_helper_boundary"
            )["matched"]
        )

        wrong_target = copy.deepcopy(report)
        wrong_target["left"]["symbols"][0]["instructions"][1]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lwz r3, 0x8(r1)"
        context = _dform_copy_helper_context(
            wrong_target, mode="stack_interval_trace"
        )
        result = rules.diagnose_document(
            wrong_target,
            focus_symbol="MoveNumOMExec",
            dform_copy_helper_context=context,
        )
        self.assertFalse(
            _evaluation(
                result, "dform_aggregate_copy_helper_boundary"
            )["matched"]
        )

        for mutation in ("unknown", "missing_dependency", "paired"):
            unsafe = _dform_copy_helper_context(
                report, mode="stack_interval_trace"
            )
            evidence = unsafe["evidence"]  # type: ignore[index]
            if mutation == "unknown":
                evidence["seam_unknown_count"] = 1  # type: ignore[index]
            elif mutation == "missing_dependency":
                evidence["dependencies"] = [[113, 115], [114, 116]]  # type: ignore[index]
            else:
                evidence["paired_codegen_proof"] = True  # type: ignore[index]
            with self.subTest(mutation=mutation):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol="MoveNumOMExec",
                        dform_copy_helper_context=unsafe,
                    )

        owner_report = _dform_copy_owner_cycle_report()
        unsafe_owner = _dform_copy_helper_context(
            owner_report, mode="existing_owner_cycle"
        )
        unsafe_owner["evidence"]["existing_live_owners"][0][  # type: ignore[index]
            "used_after_copy"
        ] = False
        with self.assertRaisesRegex(rules.LearningInputError, "semantic values"):
            rules.diagnose_document(
                owner_report,
                focus_symbol="mbev_PlayerColBall",
                dform_copy_helper_context=unsafe_owner,
            )

    def test_aggregate_pointer_branch_convergence_schedules_two_cells(self) -> None:
        report = _aggregate_pointer_branch_report()
        context = _aggregate_pointer_branch_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowKinokoAdd",
            aggregate_pointer_branch_context=context,
        )
        diagnosis = _evaluation(result, "aggregate_pointer_branch_convergence")

        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["confidence"], 0.99)
        self.assertEqual(
            diagnosis["source_class"],
            "composed_aggregate_pointer_chain_then_shared_branch_result",
        )
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["precursor"]["residual_rows"], [3, 5, 7, 9, 10])
        self.assertEqual(
            [item["target"] for item in evidence["measured_copies"]],
            [
                "stb r0, 40(r1)",
                "stb r0, 41(r1)",
                "stb r0, 42(r1)",
                "stb r0, 43(r1)",
            ],
        )
        self.assertEqual(
            evidence["measured_branch"],
            {"row": 10, "target_relative": 212, "candidate_relative": 244},
        )
        self.assertEqual(len(evidence["two_step_schedule"]), 2)
        self.assertIn("-1.0f", evidence["aggregate_chain"]["negative_one_expression"])
        self.assertFalse(result["authority_advanced"])

    def test_aggregate_pointer_branch_convergence_fails_closed(self) -> None:
        report = _aggregate_pointer_branch_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffGlowKinokoAdd"
        )
        self.assertFalse(
            _evaluation(no_context, "aggregate_pointer_branch_convergence")["matched"]
        )

        wrong_report = _aggregate_pointer_branch_context(report)
        wrong_report["proofs"]["objdiff_canonical_sha256"] = "f" * 64  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowKinokoAdd",
            aggregate_pointer_branch_context=wrong_report,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_pointer_branch_convergence")["matched"]
        )

        wrong_load = _aggregate_pointer_branch_report()
        wrong_load["right"]["symbols"][0]["instructions"][2]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lbz r0, 0(r28)"
        context = _aggregate_pointer_branch_context(wrong_load)
        result = rules.diagnose_document(
            wrong_load,
            focus_symbol="mbev_CapEffGlowKinokoAdd",
            aggregate_pointer_branch_context=context,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_pointer_branch_convergence")["matched"]
        )

        extra_residual = _aggregate_pointer_branch_report()
        extra_residual["left"]["symbols"][0]["instructions"][0][  # type: ignore[index]
            "diff_kind"
        ] = "DIFF_ARG_MISMATCH"
        extra_residual["right"]["symbols"][0]["instructions"][0][  # type: ignore[index]
            "diff_kind"
        ] = "DIFF_ARG_MISMATCH"
        context = _aggregate_pointer_branch_context(extra_residual)
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapEffGlowKinokoAdd",
            aggregate_pointer_branch_context=context,
        )
        self.assertFalse(
            _evaluation(result, "aggregate_pointer_branch_convergence")["matched"]
        )

        false_proof = _aggregate_pointer_branch_context(report)
        false_proof["proofs"]["same_tu_donors_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "same_tu_donors_exact"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowKinokoAdd",
                aggregate_pointer_branch_context=false_proof,
            )

    def test_aggregate_pointer_branch_context_rejects_unauthenticated_shapes(self) -> None:
        report = _aggregate_pointer_branch_report()
        mutations: list[tuple[str, object]] = []

        same_identity = _aggregate_pointer_branch_context(report)
        same_identity["aggregate_chain"]["groups"][0]["final"] = "colorTemp"  # type: ignore[index]
        mutations.append(("identities must be distinct", same_identity))

        duplicate_register = _aggregate_pointer_branch_context(report)
        duplicate_register["aggregate_chain"]["groups"][1]["target_register"] = "r28"  # type: ignore[index]
        mutations.append(("unique nonvolatile GPRs", duplicate_register))

        wrong_group = _aggregate_pointer_branch_context(report)
        wrong_group["branch_result"]["temporary"] = "otherTemp"  # type: ignore[index]
        mutations.append(("must name one authenticated aggregate group", wrong_group))

        missing_negative = _aggregate_pointer_branch_context(report)
        missing_negative["aggregate_chain"]["negative_one_expression"] = "-(1.0f + 0.2f * rand)"  # type: ignore[index]
        mutations.append(("explicit -1.0f multiply", missing_negative))

        wrong_exact_size = _aggregate_pointer_branch_context(report)
        wrong_exact_size["exact_result"]["candidate_bytes"] = 1208  # type: ignore[index]
        mutations.append(("preserve precursor size and relocations", wrong_exact_size))

        for message, context in mutations:
            with self.subTest(message=message):
                with self.assertRaisesRegex(rules.LearningInputError, message):
                    rules.diagnose_document(
                        report,
                        focus_symbol="mbev_CapEffGlowKinokoAdd",
                        aggregate_pointer_branch_context=context,  # type: ignore[arg-type]
                    )

    def test_same_tu_exact_sibling_shapes_schedule_only_combined_cell(self) -> None:
        report = _same_tu_shape_report()
        context = _same_tu_shape_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffElectricModelSet",
            same_tu_shape_context=context,
        )
        diagnosis = _evaluation(result, "same_tu_exact_sibling_source_shapes")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["target_size"], 252)
        self.assertEqual(evidence["candidate_size"], 244)
        self.assertEqual(evidence["donor"]["symbol"], "mbev_CapEffElectricAdd")
        self.assertEqual(
            [item["target_formatted"].split()[0] for item in evidence["fixed_array_tail"]["target_instructions"]],
            ["li", "srawi", "srwi", "subfc", "adde"],
        )
        self.assertEqual(evidence["abi_boundary"]["normalized_register"], "r0")
        self.assertEqual(evidence["zero_chain"]["target_store_offsets"], [8, 4, 0])
        self.assertEqual(evidence["zero_chain"]["candidate_store_offsets"], [0, 4, 8])
        self.assertEqual(len(evidence["scheduled_cells"]), 1)
        self.assertEqual(
            evidence["scheduled_cells"][0]["expected_object_sha256"],
            "1957f15be546225c3d6fd8e9ad4ad40cb2124e10b3b70610112572252e71d1e6",
        )
        self.assertFalse(result["authority_advanced"])

    def test_same_tu_exact_sibling_shapes_fail_closed(self) -> None:
        report = _same_tu_shape_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffElectricModelSet"
        )
        self.assertFalse(
            _evaluation(no_context, "same_tu_exact_sibling_source_shapes")["matched"]
        )

        wrong_donor = _same_tu_shape_context(report)
        wrong_donor["donor"]["source_expression"] = "objIdx == 8"  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffElectricModelSet",
            same_tu_shape_context=wrong_donor,
        )
        self.assertFalse(
            _evaluation(result, "same_tu_exact_sibling_source_shapes")["matched"]
        )

        wrong_tail = _same_tu_shape_report()
        wrong_tail["left"]["symbols"][0]["instructions"][3]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "slwi r3, r3, 3"
        context = _same_tu_shape_context(wrong_tail)
        result = rules.diagnose_document(
            wrong_tail,
            focus_symbol="mbev_CapEffElectricModelSet",
            same_tu_shape_context=context,
        )
        self.assertFalse(
            _evaluation(result, "same_tu_exact_sibling_source_shapes")["matched"]
        )

        false_proof = _same_tu_shape_context(report)
        false_proof["proofs"]["caller_contract_authenticated"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "caller_contract_authenticated"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffElectricModelSet",
                same_tu_shape_context=false_proof,
            )

    def test_short_circuit_boolean_call_order_schedules_two_bounded_cells(
        self,
    ) -> None:
        report = _short_circuit_report()
        context = _short_circuit_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapMasuLinkNextGet",
            short_circuit_context=context,
        )
        diagnosis = _evaluation(result, "short_circuit_boolean_call_order")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(
            [
                item["callee"]
                for test in evidence["mask_tests"]
                for item in test["target_call_order"]
            ],
            [
                "mbBranchAttrGet",
                "mbMasuAttrGet",
                "mbBranchMAttrGet",
                "mbMasuMAttrGet",
            ],
        )
        self.assertEqual(len(evidence["scheduled_cells"]), 2)
        self.assertEqual(
            evidence["scheduled_cells"][0]["id"],
            "explicit-if-else-right-to-left-call-order",
        )
        self.assertIn(
            "blockedF = TRUE",
            evidence["scheduled_cells"][0]["source_expression"],
        )
        self.assertEqual(
            evidence["scheduled_cells"][1]["declaration_order"],
            ["nextMasu", "battanF", "blockedF", "linkMasu"],
        )
        self.assertEqual(len(evidence["register_cycle"]), 4)
        self.assertFalse(result["authority_advanced"])

    def test_short_circuit_boolean_call_order_fails_closed(self) -> None:
        report = _short_circuit_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapMasuLinkNextGet"
        )
        self.assertFalse(
            _evaluation(no_context, "short_circuit_boolean_call_order")["matched"]
        )

        wrong_call = _short_circuit_report()
        wrong_call["left"]["symbols"][0]["instructions"][1]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "bl mbMasuAttrGet"
        context = _short_circuit_context(wrong_call)
        result = rules.diagnose_document(
            wrong_call,
            focus_symbol="mbev_CapMasuLinkNextGet",
            short_circuit_context=context,
        )
        self.assertFalse(
            _evaluation(result, "short_circuit_boolean_call_order")["matched"]
        )

        wrong_branch = _short_circuit_report()
        wrong_branch["left"]["symbols"][0]["instructions"][3]["instruction"][  # type: ignore[index]
            "branch_dest"
        ] = 144
        context = _short_circuit_context(wrong_branch)
        result = rules.diagnose_document(
            wrong_branch,
            focus_symbol="mbev_CapMasuLinkNextGet",
            short_circuit_context=context,
        )
        self.assertFalse(
            _evaluation(result, "short_circuit_boolean_call_order")["matched"]
        )

        two_cycles = _short_circuit_context(report)
        two_cycles["topology_observation"]["owners"][0]["candidate_register"] = "r28"  # type: ignore[index]
        two_cycles["topology_observation"]["owners"][1]["candidate_register"] = "r29"  # type: ignore[index]
        two_cycles["topology_observation"]["owners"][2]["candidate_register"] = "r26"  # type: ignore[index]
        two_cycles["topology_observation"]["owners"][3]["candidate_register"] = "r27"  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapMasuLinkNextGet",
            short_circuit_context=two_cycles,
        )
        self.assertFalse(
            _evaluation(result, "short_circuit_boolean_call_order")["matched"]
        )

        false_proof = _short_circuit_context(report)
        false_proof["proofs"]["pinned_mwcc_frontend"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "pinned_mwcc_frontend"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuLinkNextGet",
                short_circuit_context=false_proof,
            )

    def test_dependency_equivalent_exact_sibling_transfer_schedules_one_cell(
        self,
    ) -> None:
        report = _exact_sibling_transfer_report()
        context = _exact_sibling_transfer_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapMasuLinkNextRandomGet",
            exact_sibling_transfer_context=context,
        )
        diagnosis = _evaluation(
            result, "dependency_equivalent_exact_sibling_transfer"
        )

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["donor"]["symbol"], "mbev_CapMasuLinkNextGet")
        self.assertEqual(
            [item["target_call_order"] for item in evidence["call_order"]],
            [
                ["mbBranchAttrGet", "mbMasuAttrGet"],
                ["mbBranchMAttrGet", "mbMasuMAttrGet"],
            ],
        )
        self.assertEqual(
            [item["consumer"] for item in evidence["type_boundary"]["extsh_calls"]],
            ["mbMasuAttrGet", "mbMasuMAttrGet", "mbMasuPosGet"],
        )
        self.assertEqual(evidence["capacity"]["value"], 5)
        self.assertEqual(len(evidence["scheduled_cells"]), 1)
        cell = evidence["scheduled_cells"][0]
        self.assertEqual(cell["type_declaration"], "int linkMasu")
        self.assertEqual(cell["capacity_declaration"], "s16 masuTbl[MASU_LINK_MAX]")
        self.assertEqual(
            cell["expected_object_sha256"],
            "5d2b15050f845f6c799bcaab562f812883b5c1f2e6aace24f0a0fbb2cf894e1a",
        )
        self.assertFalse(result["authority_advanced"])

    def test_dependency_equivalent_exact_sibling_transfer_fails_closed(
        self,
    ) -> None:
        report = _exact_sibling_transfer_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapMasuLinkNextRandomGet"
        )
        self.assertFalse(
            _evaluation(
                no_context, "dependency_equivalent_exact_sibling_transfer"
            )["matched"]
        )

        wrong_call = _exact_sibling_transfer_report()
        wrong_call["left"]["symbols"][0]["instructions"][1]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "bl mbMasuAttrGet"
        result = rules.diagnose_document(
            wrong_call,
            focus_symbol="mbev_CapMasuLinkNextRandomGet",
            exact_sibling_transfer_context=_exact_sibling_transfer_context(wrong_call),
        )
        self.assertFalse(
            _evaluation(result, "dependency_equivalent_exact_sibling_transfer")[
                "matched"
            ]
        )

        wrong_extsh = _exact_sibling_transfer_report()
        wrong_extsh["left"]["symbols"][0]["instructions"][2]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "mr r3, r26"
        result = rules.diagnose_document(
            wrong_extsh,
            focus_symbol="mbev_CapMasuLinkNextRandomGet",
            exact_sibling_transfer_context=_exact_sibling_transfer_context(wrong_extsh),
        )
        self.assertFalse(
            _evaluation(result, "dependency_equivalent_exact_sibling_transfer")[
                "matched"
            ]
        )

        same_symbol = _exact_sibling_transfer_context(report)
        same_symbol["donor"]["symbol"] = "mbev_CapMasuLinkNextRandomGet"  # type: ignore[index]
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapMasuLinkNextRandomGet",
            exact_sibling_transfer_context=same_symbol,
        )
        self.assertFalse(
            _evaluation(result, "dependency_equivalent_exact_sibling_transfer")[
                "matched"
            ]
        )

        wrong_capacity = _exact_sibling_transfer_context(report)
        wrong_capacity["capacity"]["target_extent_bytes"] = 12  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, r"value \* element_size"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuLinkNextRandomGet",
                exact_sibling_transfer_context=wrong_capacity,
            )

        false_proof = _exact_sibling_transfer_context(report)
        false_proof["proofs"]["donor_strict_exact"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "donor_strict_exact"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuLinkNextRandomGet",
                exact_sibling_transfer_context=false_proof,
            )

    def test_wide_validation_narrow_selected_result_schedules_combined_cell(
        self,
    ) -> None:
        report = _wide_validation_narrow_result_report()
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapMasuValidPrevGet",
            wide_validation_narrow_result_context=(
                _wide_validation_narrow_result_context(report)
            ),
        )
        diagnosis = _evaluation(result, "wide_validation_narrow_selected_result")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(
            evidence["repeated_load"]["target_registers"], ["r27", "r30"]
        )
        self.assertEqual(
            evidence["repeated_load"]["candidate_registers"], ["r30", "r30"]
        )
        self.assertEqual(
            [item["consumer"] for item in evidence["validation_identity"]["normalization_calls"]],
            ["mbMasuAttrGet", "mbMasuMAttrGet"],
        )
        cell = evidence["scheduled_cells"][0]
        self.assertEqual(cell["declarations"], ["int linkMasu", "s16 prevMasu"])
        self.assertIn("wide_only_serial_probe", evidence["suppressed_axes"])
        self.assertIn("narrow_only_serial_probe", evidence["suppressed_axes"])
        self.assertFalse(result["authority_advanced"])

    def test_wide_validation_narrow_selected_result_fails_closed(self) -> None:
        report = _wide_validation_narrow_result_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapMasuValidPrevGet"
        )
        self.assertFalse(
            _evaluation(no_context, "wide_validation_narrow_selected_result")[
                "matched"
            ]
        )

        wrong_second_load = _wide_validation_narrow_result_report()
        wrong_second_load["left"]["symbols"][0]["instructions"][5]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lhax r30, r4, r0"
        result = rules.diagnose_document(
            wrong_second_load,
            focus_symbol="mbev_CapMasuValidPrevGet",
            wide_validation_narrow_result_context=(
                _wide_validation_narrow_result_context(wrong_second_load)
            ),
        )
        self.assertFalse(
            _evaluation(result, "wide_validation_narrow_selected_result")["matched"]
        )

        wrong_validation = _wide_validation_narrow_result_report()
        wrong_validation["left"]["symbols"][0]["instructions"][1]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "mr r3, r27"
        result = rules.diagnose_document(
            wrong_validation,
            focus_symbol="mbev_CapMasuValidPrevGet",
            wide_validation_narrow_result_context=(
                _wide_validation_narrow_result_context(wrong_validation)
            ),
        )
        self.assertFalse(
            _evaluation(result, "wide_validation_narrow_selected_result")["matched"]
        )

        wrong_selected = _wide_validation_narrow_result_report()
        wrong_selected["left"]["symbols"][0]["instructions"][6]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "extsh r3, r30"
        result = rules.diagnose_document(
            wrong_selected,
            focus_symbol="mbev_CapMasuValidPrevGet",
            wide_validation_narrow_result_context=(
                _wide_validation_narrow_result_context(wrong_selected)
            ),
        )
        self.assertFalse(
            _evaluation(result, "wide_validation_narrow_selected_result")["matched"]
        )

        false_proof = _wide_validation_narrow_result_context(report)
        false_proof["proofs"]["repeated_index_authenticated"] = False  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "repeated_index_authenticated"
        ):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuValidPrevGet",
                wide_validation_narrow_result_context=false_proof,
            )

    def test_pool_live_range_interaction_schedules_one_combined_cell(self) -> None:
        report = _pool_live_range_report()
        context = _pool_live_range_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffGlowOMExec",
            pool_live_range_context=context,
        )
        diagnosis = _evaluation(result, "pool_live_range_interaction")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["pool_owner"]["symbol"], "lbl_802C46D8")
        self.assertEqual(evidence["pool_owner"]["value_bits"], "41200000")
        self.assertEqual(len(evidence["live_range_rows"]), 3)
        self.assertEqual(len(evidence["comparison_rows"]), 2)
        self.assertEqual(len(evidence["pool_owner"]["rows"]), 2)
        self.assertEqual(
            evidence["measured_precursor"]["candidate_id"],
            "capevent-glowom001-arithmetic",
        )
        self.assertEqual(len(evidence["scheduled_cells"]), 1)
        self.assertEqual(
            evidence["scheduled_cells"][0]["expected_object_sha256"],
            "86d08e3dd8234f322ffa591c0c2d45fd35b020281483824ea70b60e16e01e5f4",
        )
        self.assertEqual(
            result["implementations"]["typed_pool_decoder"]["schema"],
            "match_workbench_pool_decoder/v1",
        )
        self.assertFalse(result["authority_advanced"])

    def test_pool_live_range_interaction_fails_closed(self) -> None:
        report = _pool_live_range_report()
        no_context = rules.diagnose_document(
            report, focus_symbol="mbev_CapEffGlowOMExec"
        )
        self.assertFalse(
            _evaluation(no_context, "pool_live_range_interaction")["matched"]
        )

        overlapping = _pool_live_range_context(report)
        overlapping["residual_groups"]["comparison_rows"] = [2, 4]  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "must be disjoint"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowOMExec",
                pool_live_range_context=overlapping,
            )

        wrong_pool = _pool_live_range_report()
        wrong_pool["right"]["symbols"][0]["instructions"][5]["instruction"][  # type: ignore[index]
            "relocation"
        ]["type_name"] = "R_PPC_ADDR32"
        result = rules.diagnose_document(
            wrong_pool,
            focus_symbol="mbev_CapEffGlowOMExec",
            pool_live_range_context=_pool_live_range_context(wrong_pool),
        )
        self.assertFalse(
            _evaluation(result, "pool_live_range_interaction")["matched"]
        )

        extra_residual = _pool_live_range_report()
        extra_residual["right"]["symbols"][0]["instructions"][7]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "nop"
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapEffGlowOMExec",
            pool_live_range_context=_pool_live_range_context(extra_residual),
        )
        self.assertFalse(
            _evaluation(result, "pool_live_range_interaction")["matched"]
        )

        false_proof = _pool_live_range_context(report)
        false_proof["proofs"]["pool_values_equivalent"] = False  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "pool_values_equivalent"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowOMExec",
                pool_live_range_context=false_proof,
            )

    def test_float_truthiness_ranks_one_natural_cell(self) -> None:
        report = _float_truthiness_report()
        context = _float_truthiness_context(report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapStarManOMExec",
            float_truthiness_context=context,
        )
        diagnosis = _evaluation(
            result, "float_truthiness_comparison_ranking"
        )

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["field_expression"], "workP->_unk20")
        self.assertEqual(len(evidence["comparison_rows"]), 2)
        self.assertEqual(
            evidence["exact_precedent"]["symbol"], "mbev_CapEffGlowOMExec"
        )
        self.assertEqual(
            evidence["scheduled_cells"][0]["source_expression"],
            "if (workP->_unk20)",
        )
        self.assertEqual(
            evidence["suppressed_axes"],
            [
                "field_not_equal_zero",
                "zero_not_equal_field",
                "commuted_explicit_zero_comparison",
            ],
        )
        self.assertFalse(result["authority_advanced"])

    def test_float_truthiness_fails_closed(self) -> None:
        report = _float_truthiness_report()
        result = rules.diagnose_document(
            report, focus_symbol="mbev_CapStarManOMExec"
        )
        self.assertFalse(
            _evaluation(result, "float_truthiness_comparison_ranking")["matched"]
        )

        wrong_order = _float_truthiness_report()
        wrong_order["right"]["symbols"][0]["instructions"][0]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "lfs f0, 4(r2)"
        result = rules.diagnose_document(
            wrong_order,
            focus_symbol="mbev_CapStarManOMExec",
            float_truthiness_context=_float_truthiness_context(wrong_order),
        )
        self.assertFalse(
            _evaluation(result, "float_truthiness_comparison_ranking")["matched"]
        )

        extra_residual = _float_truthiness_report()
        extra_residual["right"]["symbols"][0]["instructions"][4]["instruction"][  # type: ignore[index]
            "formatted"
        ] = "nop"
        result = rules.diagnose_document(
            extra_residual,
            focus_symbol="mbev_CapStarManOMExec",
            float_truthiness_context=_float_truthiness_context(extra_residual),
        )
        self.assertFalse(
            _evaluation(result, "float_truthiness_comparison_ranking")["matched"]
        )

        nonneutral = _float_truthiness_context(report)
        nonneutral["neutral_observation"]["commuted_object_sha256"] = "9" * 64  # type: ignore[index]
        with self.assertRaisesRegex(rules.LearningInputError, "object-identical"):
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapStarManOMExec",
                float_truthiness_context=nonneutral,
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

    def test_stack_gap_capacity_stage_ranks_one_capacity_cell(self) -> None:
        report = _stack_gap_capacity_report()
        context = _stack_gap_capacity_context(capacity_report=report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapPatapata",
            stack_gap_capacity_context=context,
        )
        diagnosis = _evaluation(result, "stack_gap_capacity_expression_attribution")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["stage"], "capacity")
        self.assertEqual(evidence["missing_extent_bytes"], 48)
        self.assertEqual(evidence["extra_elements"], 12)
        self.assertEqual(evidence["predicted_capacity"], 16)
        self.assertEqual(evidence["stack_delta_histogram"], {"48": 3})
        self.assertEqual(evidence["bounded_source_cells"], ["int motFile[16]"])
        self.assertFalse(result["authority_advanced"])

    def test_stack_gap_attribution_stage_emits_three_source_causes(self) -> None:
        report = _stack_gap_attribution_report()
        context = _stack_gap_capacity_context(attribution_report=report)
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapPatapata",
            stack_gap_capacity_context=context,
        )
        diagnosis = _evaluation(result, "stack_gap_capacity_expression_attribution")

        self.assertTrue(diagnosis["matched"])
        evidence = diagnosis["evidence"]
        self.assertEqual(evidence["stage"], "attribution")
        self.assertEqual(evidence["residual_rows"], [0, 1, 2, 3, 4, 5, 6])
        self.assertEqual(
            [item["kind"] for item in evidence["source_causes"]],
            [
                "live_value_reuse",
                "historical_condition_owner",
                "live_stack_object_reuse",
            ],
        )
        self.assertIn("global_declaration_permutations", evidence["suppressed_actions"])
        self.assertFalse(result["authority_advanced"])

    def test_stack_gap_capacity_attribution_fails_closed(self) -> None:
        no_context = rules.diagnose_document(
            _stack_gap_capacity_report(), focus_symbol="mbev_CapPatapata"
        )
        self.assertFalse(
            _evaluation(no_context, "stack_gap_capacity_expression_attribution")[
                "matched"
            ]
        )

        contradictory_donor = _stack_gap_capacity_context()
        contradictory_donor["capacity_stage"]["capacity_sources"][0][  # type: ignore[index]
            "capacity"
        ] = 15
        result = rules.diagnose_document(
            _stack_gap_capacity_report(),
            focus_symbol="mbev_CapPatapata",
            stack_gap_capacity_context=contradictory_donor,
        )
        self.assertFalse(
            _evaluation(result, "stack_gap_capacity_expression_attribution")["matched"]
        )

        wrong_gap = _stack_gap_capacity_context()
        wrong_gap["capacity_stage"]["uniform_gap"][  # type: ignore[index]
            "target_minus_candidate_bytes"
        ] = 44
        result = rules.diagnose_document(
            _stack_gap_capacity_report(),
            focus_symbol="mbev_CapPatapata",
            stack_gap_capacity_context=wrong_gap,
        )
        self.assertFalse(
            _evaluation(result, "stack_gap_capacity_expression_attribution")["matched"]
        )

        bad_partition = _stack_gap_capacity_context()
        bad_partition["attribution_stage"]["attributions"][2][  # type: ignore[index]
            "row_indices"
        ] = [3, 4, 5]
        with self.assertRaisesRegex(rules.LearningInputError, "partition"):
            rules.diagnose_document(
                _stack_gap_attribution_report(),
                focus_symbol="mbev_CapPatapata",
                stack_gap_capacity_context=bad_partition,
            )

        false_proof = _stack_gap_capacity_context()
        false_proof["proofs"]["capacity_result_verified"] = False  # type: ignore[index]
        with self.assertRaisesRegex(
            rules.LearningInputError, "capacity_result_verified"
        ):
            rules.diagnose_document(
                _stack_gap_capacity_report(),
                focus_symbol="mbev_CapPatapata",
                stack_gap_capacity_context=false_proof,
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

    def test_stack_gap_capacity_context_cli_emits_same_document(self) -> None:
        report = _stack_gap_capacity_report()
        context = _stack_gap_capacity_context(capacity_report=report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "stack-gap-capacity.json"
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
                            "mbev_CapPatapata",
                            "--stack-gap-capacity-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapPatapata",
                stack_gap_capacity_context=context,
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

    def test_aggregate_use_context_cli_emits_the_same_closed_document(self) -> None:
        report = _aggregate_use_report()
        context = _aggregate_use_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "aggregate-use.json"
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
                            "mbev_CapEffExplodeKillerAdd",
                            "--aggregate-use-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffExplodeKillerAdd",
                aggregate_use_context=context,
            ),
        )

    def test_aggregate_followup_context_cli_emits_same_document(self) -> None:
        report = _aggregate_followup_report()
        context = _aggregate_followup_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "aggregate-followup.json"
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
                            "mbev_CapEffElectricAdd",
                            "--aggregate-followup-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffElectricAdd",
                aggregate_followup_context=context,
            ),
        )

    def test_address_taken_context_cli_emits_same_document(self) -> None:
        report = _address_taken_report()
        context = _address_taken_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "address-taken.json"
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
                            "mbev_CapEffGlowKinokoAddAlt",
                            "--address-taken-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowKinokoAddAlt",
                address_taken_context=context,
            ),
        )

    def test_aggregate_pointer_branch_context_cli_emits_same_document(self) -> None:
        report = _aggregate_pointer_branch_report()
        context = _aggregate_pointer_branch_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json.gz"
            context_path = Path(directory) / "aggregate-pointer-branch.json"
            with gzip.open(report_path, "wt", encoding="utf-8") as stream:
                json.dump(report, stream)
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapEffGlowKinokoAdd",
                            "--aggregate-pointer-branch-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowKinokoAdd",
                aggregate_pointer_branch_context=context,
            ),
        )

    def test_same_tu_shape_context_cli_emits_same_document(self) -> None:
        report = _same_tu_shape_report()
        context = _same_tu_shape_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "same-tu-shape.json"
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
                            "mbev_CapEffElectricModelSet",
                            "--same-tu-shape-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffElectricModelSet",
                same_tu_shape_context=context,
            ),
        )

    def test_short_circuit_context_cli_emits_same_document(self) -> None:
        report = _short_circuit_report()
        context = _short_circuit_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "short-circuit.json"
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
                            "mbev_CapMasuLinkNextGet",
                            "--short-circuit-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuLinkNextGet",
                short_circuit_context=context,
            ),
        )

    def test_exact_sibling_transfer_context_cli_emits_same_document(self) -> None:
        report = _exact_sibling_transfer_report()
        context = _exact_sibling_transfer_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "exact-sibling-transfer.json"
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
                            "mbev_CapMasuLinkNextRandomGet",
                            "--exact-sibling-transfer-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuLinkNextRandomGet",
                exact_sibling_transfer_context=context,
            ),
        )

    def test_aggregate_snapshot_pointer_context_cli_emits_same_document(
        self,
    ) -> None:
        report = _aggregate_snapshot_pointer_report()
        context = _aggregate_snapshot_pointer_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "aggregate-snapshot-pointer.json"
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
                            "mbev_CapEffRingHitAdd",
                            "--aggregate-snapshot-pointer-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffRingHitAdd",
                aggregate_snapshot_pointer_context=context,
            ),
        )

    def test_typed_aggregate_copy_context_cli_emits_same_document(self) -> None:
        report = _typed_aggregate_copy_report()
        context = _typed_aggregate_copy_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "typed-aggregate-copy.json"
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
                            "mbev_CapEffDustMultiAdd",
                            "--typed-aggregate-copy-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffDustMultiAdd",
                typed_aggregate_copy_context=context,
            ),
        )

    def test_dform_copy_helper_context_cli_emits_same_document(self) -> None:
        report = _dform_copy_trace_report()
        context = _dform_copy_helper_context(report, mode="stack_interval_trace")
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "dform-copy-helper.json"
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
                            "MoveNumOMExec",
                            "--dform-copy-helper-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="MoveNumOMExec",
                dform_copy_helper_context=context,
            ),
        )

    def test_wide_validation_narrow_result_context_cli_emits_same_document(
        self,
    ) -> None:
        report = _wide_validation_narrow_result_report()
        context = _wide_validation_narrow_result_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "wide-validation-narrow-result.json"
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
                            "mbev_CapMasuValidPrevGet",
                            "--wide-validation-narrow-result-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapMasuValidPrevGet",
                wide_validation_narrow_result_context=context,
            ),
        )

    def test_pool_live_range_context_cli_emits_same_document(self) -> None:
        report = _pool_live_range_report()
        context = _pool_live_range_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "pool-live-range.json"
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
                            "mbev_CapEffGlowOMExec",
                            "--pool-live-range-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffGlowOMExec",
                pool_live_range_context=context,
            ),
        )

    def test_float_truthiness_context_cli_emits_same_document(self) -> None:
        report = _float_truthiness_report()
        context = _float_truthiness_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "float-truthiness.json"
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
                            "mbev_CapStarManOMExec",
                            "--float-truthiness-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapStarManOMExec",
                float_truthiness_context=context,
            ),
        )

    def test_metadata_owner_context_cli_emits_same_document(self) -> None:
        report = _metadata_owner_report()
        context = _metadata_owner_context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "metadata-owner.json"
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
                            "PlayerBiriQOMExec",
                            "--metadata-owner-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="PlayerBiriQOMExec",
                metadata_owner_context=context,
            ),
        )

    def test_single_use_final_call_rule_is_in_dispatcher_order(self) -> None:
        report = single_use_fixture._report()
        result = rules.diagnose_document(
            report,
            focus_symbol=single_use_fixture.FUNCTION,
            single_use_final_call_context=single_use_fixture._context(report),
        )
        evaluation = _evaluation(result, "single_use_final_call_direct_consumption")
        self.assertTrue(evaluation["matched"])
        self.assertEqual(result["schema"], "crack_learning_diagnosis/v34")
        self.assertEqual(
            result["evaluations"][7]["rule_id"],
            "single_use_final_call_direct_consumption",
        )

    def test_switch_default_fold_rule_is_in_dispatcher_order(self) -> None:
        report = switch_fold_fixture._report()
        result = rules.diagnose_document(
            report,
            focus_symbol=switch_fold_fixture.FUNCTION,
            switch_default_fold_context=switch_fold_fixture._context(report),
        )
        evaluation = _evaluation(result, "switch_default_typed_constant_fold")
        self.assertTrue(evaluation["matched"])
        self.assertEqual(result["schema"], "crack_learning_diagnosis/v34")
        self.assertEqual(
            result["evaluations"][4]["rule_id"],
            "switch_default_typed_constant_fold",
        )


if __name__ == "__main__":
    unittest.main()
