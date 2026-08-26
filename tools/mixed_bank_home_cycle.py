#!/usr/bin/env python3
"""Fail-closed mixed-bank call chronology and aggregate-home cycle evidence."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "mixed_bank_argument_aggregate_home_cycle_context/v1"
RULE_ID = "mixed_bank_argument_aggregate_home_cycle"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_STACK_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)",
    re.IGNORECASE,
)
_ADDI_R1_RE = re.compile(
    r"^\s*addi\s+r(?:[0-9]|[12][0-9]|3[01])\s*,\s*r1\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*$",
    re.IGNORECASE,
)
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "cfg_calls_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "source_interface_authenticated",
    "pinned_mwcc_right_to_left",
    "stack_home_evidence_authenticated",
    "protected_siblings_preserved",
    "exact_result_verified",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "precursor_source_sha256",
    "precursor_object_sha256",
    "precursor_record_sha256",
    "interface_receipt_sha256",
    "stack_home_receipt_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "report_artifact_sha256",
)


class MixedBankInputError(ValueError):
    """The supplied evidence cannot support the rule safely."""


def _closed(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise MixedBankInputError(f"{label} must be a JSON object")
    fields = set(value)
    missing = required - fields
    extra = fields - allowed
    if missing or extra:
        raise MixedBankInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise MixedBankInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise MixedBankInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise MixedBankInputError(f"{label} must be a C source identifier")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise MixedBankInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise MixedBankInputError(f"{label} must be from {minimum} through {maximum}")
    return value


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise MixedBankInputError(f"{label} must be lowercase SHA-256")
    return result


def _rows(value: Any, label: str) -> list[int]:
    if not isinstance(value, list) or not value:
        raise MixedBankInputError(f"{label} must be a non-empty row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) > 1024:
        raise MixedBankInputError(f"{label} must be sorted, unique, and bounded")
    return rows


def _intervals_disjoint(items: Sequence[Mapping[str, Any]], field: str) -> bool:
    intervals = sorted((item[field], item[field] + item["size"]) for item in items)
    return all(left[1] <= right[0] for left, right in zip(intervals, intervals[1:]))


def _cycles(mapping: Mapping[str, str]) -> list[list[str]]:
    cycles: list[list[str]] = []
    seen: set[str] = set()
    for start in sorted(mapping):
        if start in seen:
            continue
        path: list[str] = []
        current = start
        while current not in path and current not in seen and current in mapping:
            path.append(current)
            current = mapping[current]
        if current in path:
            cycle = path[path.index(current) :]
            smallest = min(range(len(cycle)), key=lambda index: cycle[index])
            cycles.append(cycle[smallest:] + cycle[:smallest])
        seen.update(path)
    return sorted(cycles)


def _parse_owner(raw: Any, label: str, *, frozen: bool) -> dict[str, Any]:
    owner = _closed(
        raw,
        allowed={"identity", "type", "size", "target_home", "candidate_home", "exact"},
        required={"identity", "type", "size", "target_home", "candidate_home", "exact"},
        label=label,
    )
    if owner.get("exact") is not frozen:
        expected = "true" if frozen else "false"
        raise MixedBankInputError(f"{label}.exact must be {expected}")
    result = {
        "identity": _identifier(owner.get("identity"), f"{label}.identity"),
        "type": _identifier(owner.get("type"), f"{label}.type"),
        "size": _uint(owner.get("size"), f"{label}.size", minimum=12, maximum=12),
        "target_home": _uint(owner.get("target_home"), f"{label}.target_home", minimum=8),
        "candidate_home": _uint(
            owner.get("candidate_home"), f"{label}.candidate_home", minimum=8
        ),
        "exact": frozen,
    }
    if result["type"] != "HuVecF":
        raise MixedBankInputError(f"{label}.type must be HuVecF")
    if frozen and result["target_home"] != result["candidate_home"]:
        raise MixedBankInputError(f"{label} is frozen but its homes differ")
    if not frozen and result["target_home"] == result["candidate_home"]:
        raise MixedBankInputError(f"{label} is in the cycle but does not move")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "mixed-bank aggregate-home context"
    context = _closed(
        value,
        allowed={"schema", "proofs", "precursor", "call_boundary", "frozen_owners", "owner_cycle", "exact_result"},
        required={"schema", "proofs", "precursor", "call_boundary", "frozen_owners", "owner_cycle", "exact_result"},
        label=label,
    )
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise MixedBankInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES)
    proofs = _closed(
        context.get("proofs"), allowed=proof_fields, required=proof_fields, label=f"{label}.proofs"
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise MixedBankInputError(f"{label}.proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _PROOF_HASHES:
        normalized_proofs[field] = _sha256(proofs.get(field), f"{label}.proofs.{field}")

    precursor = _closed(
        context.get("precursor"),
        allowed={"candidate_id", "target_bytes", "candidate_bytes", "target_frame", "candidate_frame", "match_percent", "physical_relocations", "residual_rows"},
        required={"candidate_id", "target_bytes", "candidate_bytes", "target_frame", "candidate_frame", "match_percent", "physical_relocations", "residual_rows"},
        label=f"{label}.precursor",
    )
    match_percent = precursor.get("match_percent")
    if isinstance(match_percent, bool) or not isinstance(match_percent, (int, float)) or not math.isfinite(float(match_percent)) or not 0.0 < float(match_percent) < 100.0:
        raise MixedBankInputError(f"{label}.precursor.match_percent must be finite and nonexact")
    normalized_precursor = {
        "candidate_id": _text(precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128),
        "target_bytes": _uint(precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=4),
        "candidate_bytes": _uint(precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=4),
        "target_frame": _uint(precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16),
        "candidate_frame": _uint(precursor.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16),
        "match_percent": float(match_percent),
        "physical_relocations": _uint(precursor.get("physical_relocations"), f"{label}.precursor.physical_relocations", minimum=1),
        "residual_rows": _rows(precursor.get("residual_rows"), f"{label}.precursor.residual_rows"),
    }
    if normalized_precursor["target_bytes"] != normalized_precursor["candidate_bytes"] or normalized_precursor["target_frame"] != normalized_precursor["candidate_frame"]:
        raise MixedBankInputError(f"{label}.precursor must have exact size and frame")

    call = _closed(
        context.get("call_boundary"),
        allowed={"helper_symbol", "source_expression", "frontend_rule", "abi_assignment_preserved", "arguments", "evaluation_order"},
        required={"helper_symbol", "source_expression", "frontend_rule", "abi_assignment_preserved", "arguments", "evaluation_order"},
        label=f"{label}.call_boundary",
    )
    if _text(call.get("frontend_rule"), f"{label}.call_boundary.frontend_rule") != "right_to_left":
        raise MixedBankInputError(f"{label}.call_boundary.frontend_rule must be right_to_left")
    if call.get("abi_assignment_preserved") is not True:
        raise MixedBankInputError(f"{label}.call_boundary.abi_assignment_preserved must be true")
    raw_arguments = call.get("arguments")
    if not isinstance(raw_arguments, list) or len(raw_arguments) < 2:
        raise MixedBankInputError(f"{label}.call_boundary.arguments must contain a mixed-bank seam")
    arguments: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_arguments):
        argument = _closed(
            raw,
            allowed={"identity", "source_index", "type", "abi_bank", "live_expression"},
            required={"identity", "source_index", "type", "abi_bank", "live_expression"},
            label=f"{label}.call_boundary.arguments[{index}]",
        )
        bank = _text(argument.get("abi_bank"), f"{label}.call_boundary.arguments[{index}].abi_bank", limit=3).lower()
        if bank not in {"gpr", "fpr"} or argument.get("live_expression") is not True:
            raise MixedBankInputError(f"{label}.call_boundary arguments require live gpr/fpr expressions")
        arguments.append({
            "identity": _identifier(argument.get("identity"), f"{label}.call_boundary.arguments[{index}].identity"),
            "source_index": _uint(argument.get("source_index"), f"{label}.call_boundary.arguments[{index}].source_index", maximum=64),
            "type": _identifier(argument.get("type"), f"{label}.call_boundary.arguments[{index}].type"),
            "abi_bank": bank,
            "live_expression": True,
        })
    arguments.sort(key=lambda item: item["source_index"])
    identities = [item["identity"] for item in arguments]
    if [item["source_index"] for item in arguments] != list(range(len(arguments))) or len(set(identities)) != len(identities) or {item["abi_bank"] for item in arguments} != {"gpr", "fpr"}:
        raise MixedBankInputError(f"{label}.call_boundary arguments must be unique, contiguous, and mixed-bank")
    raw_evaluation = call.get("evaluation_order")
    if not isinstance(raw_evaluation, list):
        raise MixedBankInputError(f"{label}.call_boundary.evaluation_order must be an array")
    evaluation_order = [_identifier(item, f"{label}.call_boundary.evaluation_order") for item in raw_evaluation]
    if evaluation_order != list(reversed(identities)):
        raise MixedBankInputError(f"{label}.call_boundary.evaluation_order must reverse source order")
    normalized_call = {
        "helper_symbol": _identifier(call.get("helper_symbol"), f"{label}.call_boundary.helper_symbol"),
        "source_expression": _text(call.get("source_expression"), f"{label}.call_boundary.source_expression", limit=512),
        "frontend_rule": "right_to_left",
        "abi_assignment_preserved": True,
        "arguments": arguments,
        "source_order": identities,
        "evaluation_order": evaluation_order,
    }

    raw_frozen = context.get("frozen_owners")
    if not isinstance(raw_frozen, list) or not raw_frozen:
        raise MixedBankInputError(f"{label}.frozen_owners must not be empty")
    frozen = [_parse_owner(raw, f"{label}.frozen_owners[{index}]", frozen=True) for index, raw in enumerate(raw_frozen)]

    cycle = _closed(
        context.get("owner_cycle"),
        allowed={"type", "size", "owners", "declaration_order"},
        required={"type", "size", "owners", "declaration_order"},
        label=f"{label}.owner_cycle",
    )
    if _identifier(cycle.get("type"), f"{label}.owner_cycle.type") != "HuVecF" or _uint(cycle.get("size"), f"{label}.owner_cycle.size", minimum=12, maximum=12) != 12:
        raise MixedBankInputError(f"{label}.owner_cycle must describe 12-byte HuVecF owners")
    raw_cycle_owners = cycle.get("owners")
    if not isinstance(raw_cycle_owners, list) or len(raw_cycle_owners) < 3:
        raise MixedBankInputError(f"{label}.owner_cycle.owners must contain at least three owners")
    owners = [_parse_owner(raw, f"{label}.owner_cycle.owners[{index}]", frozen=False) for index, raw in enumerate(raw_cycle_owners)]
    all_owners = frozen + owners
    owner_ids = [item["identity"] for item in all_owners]
    if len(set(owner_ids)) != len(owner_ids):
        raise MixedBankInputError(f"{label} owner identities must be unique")
    if not _intervals_disjoint(all_owners, "target_home") or not _intervals_disjoint(all_owners, "candidate_home"):
        raise MixedBankInputError(f"{label} owner intervals overlap")
    target_homes = {item["target_home"] for item in owners}
    candidate_homes = {item["candidate_home"] for item in owners}
    if target_homes != candidate_homes:
        raise MixedBankInputError(f"{label}.owner_cycle must permute one closed home set")
    by_candidate_home = {item["candidate_home"]: item["identity"] for item in owners}
    home_mapping = {item["identity"]: by_candidate_home[item["target_home"]] for item in owners}
    cycles = _cycles(home_mapping)
    if len(cycles) != 1 or len(cycles[0]) != len(owners):
        raise MixedBankInputError(f"{label}.owner_cycle must be one unique complete cycle")
    raw_declarations = cycle.get("declaration_order")
    if not isinstance(raw_declarations, list):
        raise MixedBankInputError(f"{label}.owner_cycle.declaration_order must be an array")
    declaration_order = [_identifier(item, f"{label}.owner_cycle.declaration_order") for item in raw_declarations]
    if len(declaration_order) != len(all_owners) or set(declaration_order) != set(owner_ids):
        raise MixedBankInputError(f"{label}.owner_cycle.declaration_order must cover each sealed owner once")

    exact = _closed(
        context.get("exact_result"),
        allowed={"candidate_id", "target_bytes", "candidate_bytes", "physical_relocations", "source_sha256", "object_sha256", "strict_report_sha256", "data_report_sha256", "candidate_record_sha256"},
        required={"candidate_id", "target_bytes", "candidate_bytes", "physical_relocations", "source_sha256", "object_sha256", "strict_report_sha256", "data_report_sha256", "candidate_record_sha256"},
        label=f"{label}.exact_result",
    )
    normalized_exact = {
        "candidate_id": _text(exact.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128),
        "target_bytes": _uint(exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=4),
        "candidate_bytes": _uint(exact.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=4),
        "physical_relocations": _uint(exact.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1),
    }
    if normalized_exact["target_bytes"] != normalized_exact["candidate_bytes"] or normalized_exact["target_bytes"] != normalized_precursor["target_bytes"] or normalized_exact["physical_relocations"] != normalized_precursor["physical_relocations"]:
        raise MixedBankInputError(f"{label}.exact_result must preserve bytes and relocations")
    for field in ("source_sha256", "object_sha256", "strict_report_sha256", "data_report_sha256", "candidate_record_sha256"):
        normalized_exact[field] = _sha256(exact.get(field), f"{label}.exact_result.{field}")

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "call_boundary": normalized_call,
        "frozen_owners": frozen,
        "owner_cycle": {
            "type": "HuVecF",
            "size": 12,
            "owners": owners,
            "declaration_order": declaration_order,
            "home_mapping": home_mapping,
            "cycle": cycles[0],
        },
        "exact_result": normalized_exact,
    }


def _stack_home(formatted: str) -> int | None:
    addi = _ADDI_R1_RE.fullmatch(formatted)
    match = addi if addi is not None else _STACK_RE.search(formatted)
    return int(match.group("offset"), 0) if match is not None else None


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:12]:
        match = _FRAME_RE.fullmatch(instruction.formatted)
        if match is not None:
            return int(match.group("size"), 0)
    return None


def _owner_at(items: Sequence[Mapping[str, Any]], field: str, offset: int) -> tuple[str, int] | None:
    matches = [item for item in items if item[field] <= offset < item[field] + item["size"]]
    if len(matches) != 1:
        return None
    item = matches[0]
    return item["identity"], offset - item[field]


def evaluate(
    pair: Any,
    target: Sequence[Any],
    candidate: Sequence[Any],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {"matched": False, "reason": "no authenticated mixed-bank aggregate-home context was supplied"}
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {"matched": False, "reason": "the mixed-bank aggregate-home context is bound to another objdiff report"}

    precursor = context["precursor"]
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (target_size, candidate_size, target_frame, candidate_frame) != (
        precursor["target_bytes"], precursor["candidate_bytes"], precursor["target_frame"], precursor["candidate_frame"]
    ):
        return {"matched": False, "reason": "the function size or frame no longer matches the sealed precursor", "evidence": {"target_size": target_size, "candidate_size": candidate_size, "target_frame": target_frame, "candidate_frame": candidate_frame}}

    rows = causal_reducer._paired_records(target, candidate)
    mismatch_rows = [index for index, (left, right) in enumerate(rows) if left is None or right is None or left.diff_kind is not None or right.diff_kind is not None]
    if mismatch_rows != precursor["residual_rows"]:
        return {"matched": False, "reason": "the physical residual rows differ from the sealed precursor", "evidence": {"report_residual_rows": mismatch_rows, "context_residual_rows": precursor["residual_rows"]}}

    cycle_owners = context["owner_cycle"]["owners"]
    frozen = context["frozen_owners"]
    observed: set[str] = set()
    observed_pairs: set[tuple[int, int]] = set()
    for row_index in mismatch_rows:
        left, right = rows[row_index]
        if left is None or right is None or left.mnemonic != right.mnemonic:
            return {"matched": False, "reason": "a residual row changes opcode or alignment", "evidence": {"row": row_index}}
        kinds = {value for value in (left.diff_kind, right.diff_kind) if value is not None}
        if not kinds or any("ARG" not in value.upper() for value in kinds):
            return {"matched": False, "reason": "a residual row is not ARG-only", "evidence": {"row": row_index, "diff_kinds": sorted(kinds)}}
        target_home = _stack_home(left.formatted)
        candidate_home = _stack_home(right.formatted)
        if target_home is None or candidate_home is None:
            return {"matched": False, "reason": "a residual row lacks one target and candidate r1 stack home", "evidence": {"row": row_index}}
        if _owner_at(frozen, "target_home", target_home) is not None or _owner_at(frozen, "candidate_home", candidate_home) is not None:
            return {"matched": False, "reason": "a residual attempts to move a frozen exact owner", "evidence": {"row": row_index}}
        target_owner = _owner_at(cycle_owners, "target_home", target_home)
        candidate_owner = _owner_at(cycle_owners, "candidate_home", candidate_home)
        if target_owner is None or candidate_owner is None or target_owner != candidate_owner:
            return {"matched": False, "reason": "a residual row falls outside the sealed same-owner home transfer", "evidence": {"row": row_index, "target_home": target_home, "candidate_home": candidate_home}}
        observed.add(target_owner[0])
        observed_pairs.add((target_home - target_owner[1], candidate_home - candidate_owner[1]))
    expected_ids = {item["identity"] for item in cycle_owners}
    expected_pairs = {(item["target_home"], item["candidate_home"]) for item in cycle_owners}
    if observed != expected_ids or observed_pairs != expected_pairs:
        return {"matched": False, "reason": "the report does not cover the complete sealed aggregate-home cycle", "evidence": {"observed_identities": sorted(observed), "expected_identities": sorted(expected_ids), "observed_home_pairs": sorted(observed_pairs), "expected_home_pairs": sorted(expected_pairs)}}

    call = context["call_boundary"]
    owner_cycle = context["owner_cycle"]
    evidence = {
        "mixed_bank_call": call,
        "frozen_owners": frozen,
        "owner_cycle": owner_cycle,
        "observed_home_pairs": [list(pair) for pair in sorted(observed_pairs)],
        "recommended_cells": [
            {"kind": "mixed_bank_direct_call_interface", "helper_symbol": call["helper_symbol"], "source_order": call["source_order"], "evaluation_order": call["evaluation_order"], "preserve_abi_banks": True, "preserve_direct_live_expressions": True},
            {"kind": "freeze_exact_and_apply_unique_typed_home_cycle", "frozen_identities": [item["identity"] for item in frozen], "declaration_order": owner_cycle["declaration_order"], "home_cycle": owner_cycle["home_mapping"], "preserve_all_other_source_axes": True},
        ],
        "suppressed_axes": ["moving_frozen_owners", "dead_or_fake_locals", "padding", "pointer_aliases", "register_shaping", "global_declaration_permutations", "repeat_tracer_capture", "automatic_retention"],
        "combined_exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "the sealed right-to-left mixed-bank call boundary preserves ABI banks, every residual row is one authenticated same-owner HuVecF home transfer, exact owners stay frozen, and the remaining homes form one unique complete cycle",
        "confidence": 0.995,
        "source_class": "mixed_bank_direct_call_plus_unique_typed_aggregate_home_cycle",
        "recommendation": f"Preserve direct {call['helper_symbol']} arguments in source order {', '.join(call['source_order'])} so GC/2.6 evaluates {', '.join(call['evaluation_order'])}; freeze {', '.join(item['identity'] for item in frozen)} and compile only the authenticated declaration order {', '.join(owner_cycle['declaration_order'])}.",
        "evidence": evidence,
    }
