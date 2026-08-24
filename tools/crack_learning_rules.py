#!/usr/bin/env python3
"""Compose proven CRACK_REPORT lessons with the causal objdiff reducer.

The rules in this module are intentionally evidence-only.  They recognize
narrow instruction/topology signatures, expose the evidence and confidence
used for each diagnosis, and recommend only natural source-shape classes.
They never edit source, retain a candidate, or advance recovery authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import mismatch_cluster_audit as causal_reducer


SCHEMA = "crack_learning_diagnosis/v1"
SCHEMA_VERSION = 1
HASH_FIELD = "diagnosis_sha256"

_REGISTER_RE = re.compile(r"\b(?P<kind>[rRfF])(?P<number>[0-9]|[12][0-9]|3[01])\b")
_STACK_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)",
    re.IGNORECASE,
)
_CALL_MNEMONICS = frozenset({"bl", "bla", "bctrl", "blrl"})
_CONDITIONAL_MNEMONICS = frozenset(
    {
        "bc",
        "bca",
        "beq",
        "beqa",
        "bge",
        "bgea",
        "bgt",
        "bgta",
        "ble",
        "blea",
        "blt",
        "blta",
        "bne",
        "bnea",
        "bso",
        "bns",
        "bdnz",
        "bdz",
    }
)
_SWITCH_MNEMONICS = frozenset({"bctr", "bcctr"})
_AGGREGATE_LOADS = frozenset({"lfs", "lfd", "lwz", "lhz", "lha", "lbz"})
_AGGREGATE_STORES = frozenset({"stfs", "stfd", "stw", "sth", "stb"})

_RULE_ORDER = (
    "explicit_else_return_cfg",
    "assignment_condition_saved_gpr_cycle",
    "switch_case_scoped_fpr_lifetimes",
    "aggregate_self_copy_final_consumer",
)


class LearningInputError(ValueError):
    """An input cannot support a closed, deterministic diagnosis."""


def _canonical(value: Any) -> bytes:
    try:
        return json.dumps(
            value,
            sort_keys=True,
            separators=(",", ":"),
            ensure_ascii=True,
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise LearningInputError(f"input is not canonical JSON: {exc}") from exc


def _sha256(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _with_self_hash(value: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop(HASH_FIELD, None)
    result[HASH_FIELD] = _sha256(_canonical(result))
    return result


def _registers(text: str, kind: str | None = None) -> list[str]:
    result: list[str] = []
    for match in _REGISTER_RE.finditer(text):
        register = f"{match.group('kind').lower()}{int(match.group('number'))}"
        if kind is None or register.startswith(kind):
            result.append(register)
    return result


def _saved(register: str, kind: str) -> bool:
    return register.startswith(kind) and 14 <= int(register[1:]) <= 31


def _without_registers(text: str) -> str:
    return _REGISTER_RE.sub("<reg>", text.lower()).strip()


def _stack_offset(text: str) -> int | None:
    match = _STACK_RE.search(text)
    if match is None:
        return None
    return causal_reducer._parse_number(match.group("offset"))


def _pair(document: Mapping[str, Any], symbol: str) -> causal_reducer.FunctionPair:
    try:
        return causal_reducer._focus_pairs(
            causal_reducer._paired_functions(document), symbol
        )[0]
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(f"objdiff report rejected ({exc.code}): {exc.message}") from exc


def _entries(
    pair: causal_reducer.FunctionPair,
) -> tuple[list[causal_reducer.Instruction], list[causal_reducer.Instruction]]:
    try:
        return (
            causal_reducer._entries(pair.target, "target", pair.name),
            causal_reducer._entries(pair.candidate, "candidate", pair.name),
        )
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(f"objdiff report rejected ({exc.code}): {exc.message}") from exc


def _function_size(symbol: Mapping[str, Any] | None) -> int | None:
    if symbol is None:
        return None
    return causal_reducer._parse_number(symbol.get("size"))


def _evaluation(
    rule_id: str,
    *,
    matched: bool,
    reason: str,
    confidence: float | None = None,
    source_class: str | None = None,
    recommendation: str | None = None,
    evidence: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "rule_id": rule_id,
        "matched": matched,
        "reason": reason,
        "evidence": dict(evidence or {}),
    }
    if matched:
        assert confidence is not None
        assert source_class is not None
        assert recommendation is not None
        result.update(
            {
                "confidence": confidence,
                "source_class": source_class,
                "recommendation": recommendation,
                "limitations": [
                    "The diagnosis ranks a natural source-shape class; it does not prove original spelling or provenance.",
                    "Do not edit or retain source from this result alone; strict/data/physical-relocation/section and protected-sibling gates remain required.",
                ],
            }
        )
    return result


def _explicit_else_evaluation(audit: Mapping[str, Any]) -> dict[str, Any]:
    matches = [
        item
        for item in audit.get("hypotheses", [])
        if isinstance(item, Mapping)
        and item.get("classification") == "explicit_else_return_epilogue"
    ]
    if not matches:
        return _evaluation(
            "explicit_else_return_cfg",
            matched=False,
            reason="the installed causal reducer found no explicit else-return epilogue topology",
        )
    primary = matches[0]
    evidence = primary.get("evidence")
    return _evaluation(
        "explicit_else_return_cfg",
        matched=True,
        reason="the installed causal reducer matched its narrow explicit else-return CFG signature",
        confidence=float(primary.get("confidence", 0.0)),
        source_class="explicit_else_return_control_flow",
        recommendation="Test an explicit else-return control-flow form around the guarded body.",
        evidence={
            "causal_classification": primary.get("classification"),
            "causal_rank": primary.get("rank"),
            "causal_evidence": dict(evidence) if isinstance(evidence, Mapping) else {},
        },
    )


def _compatible_register_only_pair(
    left: causal_reducer.Instruction,
    right: causal_reducer.Instruction,
) -> bool:
    if not left.has_instruction or not right.has_instruction:
        return False
    if left.mnemonic != right.mnemonic:
        return False
    if causal_reducer._relocation_diff(left, right):
        return False
    if left.mnemonic in causal_reducer._BRANCH_MNEMONICS:
        return causal_reducer._branch_relative(left) == causal_reducer._branch_relative(right)
    return _without_registers(left.formatted) == _without_registers(right.formatted)


def _closed_cycles(mapping: Mapping[str, str]) -> list[list[str]]:
    if set(mapping) != set(mapping.values()):
        return []
    cycles: list[list[str]] = []
    visited: set[str] = set()
    for start in sorted(mapping):
        if start in visited:
            continue
        cycle: list[str] = []
        current = start
        while current not in cycle and current not in visited:
            cycle.append(current)
            visited.add(current)
            current = mapping[current]
        if current != start:
            return []
        if len(cycle) > 1:
            cycles.append(cycle)
    return cycles


def _call_result_consumers(
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    mapping: Mapping[str, str],
) -> list[dict[str, Any]]:
    rows = causal_reducer._paired_records(target, candidate)
    result: list[dict[str, Any]] = []
    for call_index, (left_call, right_call) in enumerate(rows):
        if (
            left_call is None
            or right_call is None
            or left_call.mnemonic not in _CALL_MNEMONICS
            or right_call.mnemonic != left_call.mnemonic
        ):
            continue
        for capture_index in range(call_index + 1, min(len(rows), call_index + 4)):
            left_capture, right_capture = rows[capture_index]
            if left_capture is None or right_capture is None:
                continue
            left_regs = _registers(left_capture.formatted, "r")
            right_regs = _registers(right_capture.formatted, "r")
            if (
                left_capture.mnemonic != "mr"
                or right_capture.mnemonic != "mr"
                or len(left_regs) != 2
                or len(right_regs) != 2
                or left_regs[1] != "r3"
                or right_regs[1] != "r3"
                or mapping.get(left_regs[0]) != right_regs[0]
            ):
                continue
            for compare_index in range(capture_index + 1, min(len(rows), capture_index + 4)):
                left_compare, right_compare = rows[compare_index]
                if left_compare is None or right_compare is None:
                    continue
                if (
                    not left_compare.mnemonic.startswith("cmp")
                    or right_compare.mnemonic != left_compare.mnemonic
                    or left_regs[0] not in _registers(left_compare.formatted, "r")
                    or right_regs[0] not in _registers(right_compare.formatted, "r")
                ):
                    continue
                branch_index = next(
                    (
                        index
                        for index in range(compare_index + 1, min(len(rows), compare_index + 3))
                        if rows[index][0] is not None
                        and rows[index][1] is not None
                        and rows[index][0].mnemonic in _CONDITIONAL_MNEMONICS
                        and rows[index][1].mnemonic == rows[index][0].mnemonic
                        and causal_reducer._branch_relative(rows[index][0])
                        == causal_reducer._branch_relative(rows[index][1])
                    ),
                    None,
                )
                if branch_index is not None:
                    result.append(
                        {
                            "call_index": call_index,
                            "capture_index": capture_index,
                            "compare_index": compare_index,
                            "branch_index": branch_index,
                            "target_result_register": left_regs[0],
                            "candidate_result_register": right_regs[0],
                        }
                    )
                    break
            if result and result[-1]["call_index"] == call_index:
                break
    return result


def _assignment_condition_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
) -> dict[str, Any]:
    if _function_size(pair.target) != _function_size(pair.candidate):
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="target and candidate function sizes differ",
        )
    rows = causal_reducer._paired_records(target, candidate)
    if any(
        left is None
        or right is None
        or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="the residual is not an operation-, CFG-, relocation-, and immediate-identical register-only difference",
        )

    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_regs = _registers(left.formatted)
        right_regs = _registers(right.formatted)
        if len(left_regs) != len(right_regs):
            return _evaluation(
                "assignment_condition_saved_gpr_cycle",
                matched=False,
                reason="a register-only row has a different operand count",
            )
        row_mismatch = False
        for target_reg, candidate_reg in zip(left_regs, right_regs):
            if target_reg == candidate_reg:
                continue
            if not (_saved(target_reg, "r") and _saved(candidate_reg, "r")):
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the register difference is not confined to nonvolatile GPRs",
                )
            if mapping.get(target_reg, candidate_reg) != candidate_reg:
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the target-to-candidate GPR mapping is inconsistent",
                )
            if reverse.get(candidate_reg, target_reg) != target_reg:
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the saved-GPR mapping is not one-to-one",
                )
            mapping[target_reg] = candidate_reg
            reverse[candidate_reg] = target_reg
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)

    cycles = _closed_cycles(mapping)
    if not cycles or max(map(len, cycles)) < 3:
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="no closed saved-GPR cycle of length three or greater is present",
            evidence={"register_mapping": dict(sorted(mapping.items()))},
        )
    consumers = _call_result_consumers(target, candidate, mapping)
    if not consumers:
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="the saved-GPR cycle has no call-result assignment immediately consumed by a condition",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    return _evaluation(
        "assignment_condition_saved_gpr_cycle",
        matched=True,
        reason="an otherwise identical function contains a closed saved-GPR cycle joined to an immediately consumed call-result assignment",
        confidence=0.96,
        source_class="assignment_in_consuming_condition",
        recommendation="Test a natural condition that combines the existing result assignment with its immediate comparison.",
        evidence={
            "target_size": _function_size(pair.target),
            "candidate_size": _function_size(pair.candidate),
            "register_mapping": dict(sorted(mapping.items())),
            "cycles": cycles,
            "mismatch_rows": mismatch_rows,
            "call_result_consumers": consumers,
            "structural_invariants": [
                "mnemonic_sequence",
                "branch_relative_targets",
                "relocations",
                "non_register_operands",
            ],
        },
    )


def _frame_size(entries: Sequence[causal_reducer.Instruction]) -> int | None:
    for item in entries[:24]:
        if item.mnemonic not in {"stwu", "stdu"}:
            continue
        offset = _stack_offset(item.formatted)
        if offset is not None and offset < 0:
            return -offset
    return None


def _causal_stack_deltas(audit: Mapping[str, Any]) -> list[int]:
    result: set[int] = set()
    for group in audit.get("causal_groups", []):
        if not isinstance(group, Mapping) or group.get("classification") != "stack_home_uniform_delta":
            continue
        signature = group.get("signature", [])
        if not isinstance(signature, list):
            continue
        for part in signature[1:]:
            # The reducer deliberately uses tuple signatures internally and
            # only converts the outer tuple when building its JSON object.
            if isinstance(part, (list, tuple)):
                result.update(value for value in part if isinstance(value, int) and value != 0)
            elif isinstance(part, int) and part != 0:
                result.add(part)
    return sorted(result)


def _preceded_by_call(entries: Sequence[causal_reducer.Instruction], index: int) -> bool:
    return any(
        entries[prior].has_instruction and entries[prior].mnemonic in _CALL_MNEMONICS
        for prior in range(max(0, index - 3), index)
    )


def _switch_fpr_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    audit: Mapping[str, Any],
) -> dict[str, Any]:
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if target_frame is None or candidate_frame is None or target_frame <= candidate_frame:
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the target does not have a larger measurable stack frame",
        )
    frame_delta = target_frame - candidate_frame
    stack_deltas = _causal_stack_deltas(audit)
    if frame_delta > 256 or not any(abs(value) == frame_delta for value in stack_deltas):
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the causal reducer did not corroborate the prologue frame delta with a uniform stack-home delta",
            evidence={
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
                "frame_delta": frame_delta,
                "causal_stack_deltas": stack_deltas,
            },
        )
    if not any(item.mnemonic in _SWITCH_MNEMONICS for item in target):
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the focus has no indirect switch dispatch instruction",
            evidence={"frame_delta": frame_delta, "causal_stack_deltas": stack_deltas},
        )

    captures: list[dict[str, Any]] = []
    rows = causal_reducer._paired_records(target, candidate)
    for index, (left, right) in enumerate(rows):
        if left is None or left.mnemonic != "fmr" or not _preceded_by_call(target, index):
            continue
        registers = _registers(left.formatted, "f")
        if len(registers) < 2 or registers[1] != "f1" or not _saved(registers[0], "f"):
            continue
        candidate_registers = _registers(right.formatted, "f") if right is not None else []
        if (
            right is not None
            and right.has_instruction
            and right.mnemonic == "fmr"
            and candidate_registers == registers
        ):
            continue
        captures.append(
            {
                "index": index,
                "target_result_register": registers[0],
                "candidate_mnemonic": right.mnemonic if right is not None and right.has_instruction else None,
            }
        )
    if len(captures) < 3:
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="fewer than three target-only nonvolatile FPR call-result lifetimes are present",
            evidence={
                "frame_delta": frame_delta,
                "causal_stack_deltas": stack_deltas,
                "result_captures": captures,
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or candidate_size is None or target_size <= candidate_size:
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="target-only FPR lifetimes are not accompanied by a larger target function",
        )
    return _evaluation(
        "switch_case_scoped_fpr_lifetimes",
        matched=True,
        reason="switch dispatch, a corroborated frame delta, and multiple target-only nonvolatile FPR result captures occur together",
        confidence=0.97,
        source_class="switch_case_scoped_used_result_locals",
        recommendation="Test used floating-point call-result locals scoped to the individual switch cases that consume them.",
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "frame_delta": frame_delta,
            "causal_stack_deltas": stack_deltas,
            "switch_mnemonics": sorted(
                {item.mnemonic for item in target if item.mnemonic in _SWITCH_MNEMONICS}
            ),
            "result_captures": captures,
        },
    )


def _copy_run(
    entries: Sequence[causal_reducer.Instruction],
    *,
    corresponding: Sequence[causal_reducer.Instruction] | None = None,
    require_asymmetry: bool,
) -> dict[str, Any] | None:
    # "Final" is established by the absence of later calls, not by a
    # percentage of function length.  Keep the physical search bounded to the
    # last 64 aligned rows so short functions are not treated differently.
    start_floor = max(0, len(entries) - 64)
    for start in range(start_floor, len(entries)):
        for end in range(start + 4, min(len(entries), start + 10) + 1):
            window = entries[start:end]
            if any(not item.has_instruction for item in window):
                continue
            offsets = [_stack_offset(item.formatted) for item in window]
            if any(offset is None for offset in offsets):
                continue
            loads = [
                offset
                for item, offset in zip(window, offsets)
                if item.mnemonic in _AGGREGATE_LOADS
            ]
            stores = [
                offset
                for item, offset in zip(window, offsets)
                if item.mnemonic in _AGGREGATE_STORES
            ]
            if len(loads) < 3 or len(loads) != len(stores) or sorted(loads) != sorted(stores):
                continue
            if len(loads) + len(stores) != len(window):
                continue
            if require_asymmetry:
                assert corresponding is not None
                other = corresponding[start:end]
                if len(other) != len(window) or any(item.has_instruction for item in other):
                    continue
            consumers = [
                {
                    "index": index,
                    "formatted": entries[index].formatted,
                }
                for index in range(end, min(len(entries), end + 12))
                if entries[index].has_instruction and entries[index].mnemonic in _CALL_MNEMONICS
            ]
            if not consumers:
                continue
            if any(item.mnemonic in _CALL_MNEMONICS for item in entries[end + 12 :]):
                continue
            return {
                "index_start": start,
                "index_end": end - 1,
                "component_count": len(loads),
                "stack_offsets": sorted(loads),
                "mnemonics": [item.mnemonic for item in window],
                "final_consumers": consumers,
            }
    return None


def _exact_donor_evidence(
    document: Mapping[str, Any], donor_symbols: Sequence[str]
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    for symbol in donor_symbols:
        if symbol in seen:
            continue
        seen.add(symbol)
        pair = _pair(document, symbol)
        if not causal_reducer._is_exact_pair(pair):
            continue
        target, candidate = _entries(pair)
        target_copy = _copy_run(target, require_asymmetry=False)
        candidate_copy = _copy_run(candidate, require_asymmetry=False)
        if target_copy is None or candidate_copy is None:
            continue
        signature_keys = ("component_count", "mnemonics")
        if any(target_copy[key] != candidate_copy[key] for key in signature_keys):
            continue
        result.append(
            {
                "symbol": symbol,
                "target_match_percent": pair.target.get("match_percent") if pair.target else None,
                "candidate_match_percent": pair.candidate.get("match_percent") if pair.candidate else None,
                "copy": target_copy,
                "signature_sha256": _sha256(
                    _canonical({key: target_copy[key] for key in signature_keys})
                ),
            }
        )
    return result


def _aggregate_self_copy_evaluation(
    document: Mapping[str, Any],
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    donor_symbols: Sequence[str],
) -> dict[str, Any]:
    focus_copy = _copy_run(
        target, corresponding=candidate, require_asymmetry=True
    )
    if focus_copy is None:
        return _evaluation(
            "aggregate_self_copy_final_consumer",
            matched=False,
            reason="no target-only aggregate self-copy occurs at the final consumer boundary",
        )
    donors = _exact_donor_evidence(document, donor_symbols)
    compatible = [
        donor
        for donor in donors
        if donor["copy"]["component_count"] == focus_copy["component_count"]
        and donor["copy"]["mnemonics"] == focus_copy["mnemonics"]
    ]
    if not compatible:
        return _evaluation(
            "aggregate_self_copy_final_consumer",
            matched=False,
            reason="the focus signature has no explicitly named, exact same-report/TU donor with the same copy shape",
            evidence={
                "focus_copy": focus_copy,
                "requested_donor_symbols": list(dict.fromkeys(donor_symbols)),
                "exact_donors": donors,
            },
        )
    return _evaluation(
        "aggregate_self_copy_final_consumer",
        matched=True,
        reason="a target-only final-consumer self-copy has an exact structural donor in the same object/TU report",
        confidence=0.98,
        source_class="used_aggregate_self_assignment_at_final_consumer",
        recommendation="Test a natural aggregate self-assignment immediately before the final consumers, following the exact same-TU donor shape.",
        evidence={
            "focus_copy": focus_copy,
            "same_tu_basis": "focus and donor are paired functions in the same objdiff object report",
            "exact_donors": compatible,
        },
    )


def diagnose_document(
    document: Mapping[str, Any],
    *,
    focus_symbol: str,
    same_tu_donor_symbols: Sequence[str] = (),
) -> dict[str, Any]:
    """Return a self-hashed, authority-free diagnosis for one function."""

    if not isinstance(document, Mapping):
        raise LearningInputError("objdiff report must be a JSON object")
    if not isinstance(focus_symbol, str) or not focus_symbol.strip():
        raise LearningInputError("focus_symbol must be non-empty text")
    focus = focus_symbol.strip()
    if any(not isinstance(value, str) or not value.strip() for value in same_tu_donor_symbols):
        raise LearningInputError("same_tu_donor_symbols must contain non-empty text")
    donors = tuple(value.strip() for value in same_tu_donor_symbols)
    pair = _pair(document, focus)
    target, candidate = _entries(pair)
    try:
        audit = causal_reducer.audit_document(
            document,
            focus_symbol=focus,
            include_exact_residuals=True,
            summary_only=False,
        )
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(f"causal reducer rejected report ({exc.code}): {exc.message}") from exc
    if audit.get("fail_closed") or audit.get("status") != "ok":
        raise LearningInputError("causal reducer did not produce a closed successful audit")

    evaluations = [
        _explicit_else_evaluation(audit),
        _assignment_condition_evaluation(pair, target, candidate),
        _switch_fpr_evaluation(pair, target, candidate, audit),
        _aggregate_self_copy_evaluation(document, target, candidate, donors),
    ]
    if tuple(item["rule_id"] for item in evaluations) != _RULE_ORDER:
        raise AssertionError("rule evaluation order drifted")
    tool_path = Path(__file__).resolve()
    reducer_path = Path(causal_reducer.__file__).resolve()
    body = {
        "schema": SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbol": focus,
        "inputs": {
            "objdiff_canonical_sha256": _sha256(_canonical(document)),
            "same_tu_donor_symbols": list(dict.fromkeys(donors)),
        },
        "implementations": {
            "learning_rules": {
                "path": tool_path.name,
                "sha256": _sha256(tool_path.read_bytes()),
            },
            "causal_reducer": {
                "path": reducer_path.name,
                "schema_version": audit.get("schema_version"),
                "sha256": _sha256(reducer_path.read_bytes()),
            },
        },
        "evaluations": evaluations,
        "diagnoses": [dict(item) for item in evaluations if item["matched"]],
        "limitations": [
            "These rules compose deterministic physical signatures; they do not infer semantic variable names or original-source provenance.",
            "Recommendations are natural source classes only and never authorize source edits, candidate retention, promotion, or authority advancement.",
            "An exact donor is evidence for source shape only; the focus still requires its own complete proof chain.",
        ],
        "authority_advanced": False,
    }
    return _with_self_hash(body)


def _load_json(path: Path) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise LearningInputError(f"cannot read objdiff report {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise LearningInputError(f"invalid JSON in objdiff report {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise LearningInputError(f"objdiff report {path} must contain a JSON object")
    return value


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Apply evidence-only CRACK_REPORT learning rules to one objdiff function."
    )
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--function", required=True, dest="focus_symbol")
    parser.add_argument(
        "--same-tu-donor",
        action="append",
        default=[],
        dest="same_tu_donors",
        help="explicitly named exact donor function from the same object report",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    try:
        result = diagnose_document(
            _load_json(args.report),
            focus_symbol=args.focus_symbol,
            same_tu_donor_symbols=args.same_tu_donors,
        )
    except LearningInputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(result, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
