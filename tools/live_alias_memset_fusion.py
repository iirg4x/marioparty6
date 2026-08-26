#!/usr/bin/env python3
"""Fail-closed historical live-alias fusion at an immediate memset boundary."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "historical_live_alias_memset_fusion_context/v1"
RULE_ID = "historical_live_alias_memset_fusion"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_COMMIT_RE = re.compile(r"[0-9a-f]{7,40}")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_STORE_RE = re.compile(
    r"^\s*stw\s+(?P<register>r(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*(?P<base>r(?:[0-9]|[12][0-9]|3[01]))\s*\)\s*$",
    re.IGNORECASE,
)
_MR_RE = re.compile(
    r"^\s*mr\s+(?P<destination>r(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<source>r(?:[0-9]|[12][0-9]|3[01]))\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "cfg_calls_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "allocation_contract_authenticated",
    "target_store_forward_order_authenticated",
    "historical_alias_authenticated",
    "negative_controls_measured",
    "protected_siblings_preserved",
    "exact_result_verified",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "history_receipt_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "report_artifact_sha256",
)
_CONTROL_CLASSES = {
    "fused_without_alias": "object_identical",
    "direct_allocator_chain": "regressed",
    "typed_allocation_owner": "object_identical",
    "sizeof_owner": "object_identical",
    "separate_historical_alias": "size_exact_saved_gpr_regression",
}


class LiveAliasInputError(ValueError):
    """The supplied evidence cannot support the rule safely."""


def _closed(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise LiveAliasInputError(f"{label} must be a JSON object")
    fields = set(value)
    missing = required - fields
    extra = fields - allowed
    if missing or extra:
        raise LiveAliasInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise LiveAliasInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise LiveAliasInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise LiveAliasInputError(f"{label} must be a C source identifier")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise LiveAliasInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise LiveAliasInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise LiveAliasInputError(f"{label} must be from {minimum} through {maximum}")
    return value


def _positive_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise LiveAliasInputError(f"{label} must be a positive finite number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise LiveAliasInputError(f"{label} must be a positive finite number")
    return result


def _rows(value: Any, label: str) -> list[int]:
    if not isinstance(value, list) or not value:
        raise LiveAliasInputError(f"{label} must be a non-empty row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) > 64:
        raise LiveAliasInputError(f"{label} must be sorted, unique, and bounded")
    return rows


def _register(value: Any, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if re.fullmatch(r"r(?:[0-9]|[12][0-9]|3[01])", result) is None:
        raise LiveAliasInputError(f"{label} must be a GPR")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "historical live-alias memset context"
    context = _closed(
        value,
        allowed={
            "schema",
            "proofs",
            "precursor",
            "producer_consumer",
            "historical_alias",
            "controls",
            "telemetry",
            "exact_result",
        },
        required={
            "schema",
            "proofs",
            "precursor",
            "producer_consumer",
            "historical_alias",
            "controls",
            "telemetry",
            "exact_result",
        },
        label=label,
    )
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise LiveAliasInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES)
    proofs = _closed(
        context.get("proofs"), allowed=proof_fields, required=proof_fields, label=f"{label}.proofs"
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LiveAliasInputError(f"{label}.proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _PROOF_HASHES:
        normalized_proofs[field] = _sha256(proofs.get(field), f"{label}.proofs.{field}")

    precursor = _closed(
        context.get("precursor"),
        allowed={
            "function",
            "candidate_id",
            "target_bytes",
            "candidate_bytes",
            "target_frame",
            "candidate_frame",
            "match_percent",
            "target_physical_relocations",
            "candidate_physical_relocations",
            "residual_rows",
            "target_home_store",
        },
        required={
            "function",
            "candidate_id",
            "target_bytes",
            "candidate_bytes",
            "target_frame",
            "candidate_frame",
            "match_percent",
            "target_physical_relocations",
            "candidate_physical_relocations",
            "residual_rows",
            "target_home_store",
        },
        label=f"{label}.precursor",
    )
    match_percent = _positive_number(
        precursor.get("match_percent"), f"{label}.precursor.match_percent"
    )
    if match_percent >= 100.0:
        raise LiveAliasInputError(f"{label}.precursor.match_percent must be nonexact")
    target_bytes = _uint(
        precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=8
    )
    candidate_bytes = _uint(
        precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=4
    )
    target_frame = _uint(
        precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16
    )
    candidate_frame = _uint(
        precursor.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16
    )
    if target_bytes - candidate_bytes != 4 or target_frame - candidate_frame != 16:
        raise LiveAliasInputError(
            f"{label}.precursor must be exact-size-minus-four with one alignment-frame step"
        )
    target_relocations = _uint(
        precursor.get("target_physical_relocations"),
        f"{label}.precursor.target_physical_relocations",
        minimum=1,
    )
    candidate_relocations = _uint(
        precursor.get("candidate_physical_relocations"),
        f"{label}.precursor.candidate_physical_relocations",
        minimum=1,
    )
    if target_relocations != candidate_relocations:
        raise LiveAliasInputError(f"{label}.precursor physical relocations must already be exact")
    home = _closed(
        precursor.get("target_home_store"),
        allowed={"opcode", "register", "base_register", "offset", "candidate_absent"},
        required={"opcode", "register", "base_register", "offset", "candidate_absent"},
        label=f"{label}.precursor.target_home_store",
    )
    if _text(home.get("opcode"), f"{label}.precursor.target_home_store.opcode").lower() != "stw":
        raise LiveAliasInputError(f"{label}.precursor target home must be stw")
    if _register(home.get("base_register"), f"{label}.precursor.target_home_store.base_register") != "r1":
        raise LiveAliasInputError(f"{label}.precursor target home must use r1")
    if home.get("candidate_absent") is not True:
        raise LiveAliasInputError(f"{label}.precursor target home must be absent from candidate")
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _text(
            precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocations,
        "residual_rows": _rows(
            precursor.get("residual_rows"), f"{label}.precursor.residual_rows"
        ),
        "target_home_store": {
            "opcode": "stw",
            "register": _register(
                home.get("register"), f"{label}.precursor.target_home_store.register"
            ),
            "base_register": "r1",
            "offset": _uint(
                home.get("offset"), f"{label}.precursor.target_home_store.offset", minimum=8
            ),
            "candidate_absent": True,
        },
    }
    if normalized_precursor["target_home_store"]["offset"] != 8:
        raise LiveAliasInputError(f"{label}.precursor target home offset must be 8")

    producer = _closed(
        context.get("producer_consumer"),
        allowed={
            "allocation_symbol",
            "consumer_symbol",
            "allocation_owner",
            "field_owner",
            "field_name",
            "live_owner",
            "element_type",
            "element_count",
            "zero_value",
            "return_register",
            "live_register",
            "destination_order",
        },
        required={
            "allocation_symbol",
            "consumer_symbol",
            "allocation_owner",
            "field_owner",
            "field_name",
            "live_owner",
            "element_type",
            "element_count",
            "zero_value",
            "return_register",
            "live_register",
            "destination_order",
        },
        label=f"{label}.producer_consumer",
    )
    names = {
        field: _identifier(producer.get(field), f"{label}.producer_consumer.{field}")
        for field in (
            "allocation_symbol",
            "consumer_symbol",
            "allocation_owner",
            "field_owner",
            "field_name",
            "live_owner",
            "element_type",
        )
    }
    if names["consumer_symbol"] != "memset":
        raise LiveAliasInputError(f"{label}.producer_consumer.consumer_symbol must be memset")
    if producer.get("zero_value") != 0:
        raise LiveAliasInputError(f"{label}.producer_consumer.zero_value must be integer zero")
    raw_order = producer.get("destination_order")
    if not isinstance(raw_order, list) or len(raw_order) != 4:
        raise LiveAliasInputError(f"{label}.producer_consumer.destination_order must have four entries")
    destination_order = [
        _text(item, f"{label}.producer_consumer.destination_order", limit=128)
        for item in raw_order
    ]
    expected_inner = [
        names["live_owner"],
        f"{names['field_owner']}.{names['field_name']}",
        names["allocation_owner"],
    ]
    normalized_producer = {
        **names,
        "element_count": _uint(
            producer.get("element_count"), f"{label}.producer_consumer.element_count", minimum=1
        ),
        "zero_value": 0,
        "return_register": _register(
            producer.get("return_register"), f"{label}.producer_consumer.return_register"
        ),
        "live_register": _register(
            producer.get("live_register"), f"{label}.producer_consumer.live_register"
        ),
        "destination_order": destination_order,
    }
    if normalized_producer["return_register"] != "r3":
        raise LiveAliasInputError(f"{label}.producer_consumer return register must be r3")
    if destination_order[1:] != expected_inner:
        raise LiveAliasInputError(
            f"{label}.producer_consumer inner destination order must preserve live owner, field, allocation result"
        )

    alias = _closed(
        context.get("historical_alias"),
        allowed={
            "name",
            "type",
            "commit",
            "declaration_authenticated",
            "live_at_consumer_boundary",
            "outer_assignment",
            "stack_home_offset",
        },
        required={
            "name",
            "type",
            "commit",
            "declaration_authenticated",
            "live_at_consumer_boundary",
            "outer_assignment",
            "stack_home_offset",
        },
        label=f"{label}.historical_alias",
    )
    alias_name = _identifier(alias.get("name"), f"{label}.historical_alias.name")
    alias_type = _identifier(alias.get("type"), f"{label}.historical_alias.type")
    commit = _text(alias.get("commit"), f"{label}.historical_alias.commit", limit=40).lower()
    if _COMMIT_RE.fullmatch(commit) is None:
        raise LiveAliasInputError(f"{label}.historical_alias.commit must be a git object prefix")
    for field in ("declaration_authenticated", "live_at_consumer_boundary", "outer_assignment"):
        if alias.get(field) is not True:
            raise LiveAliasInputError(f"{label}.historical_alias.{field} must be true")
    if alias_type != normalized_producer["element_type"]:
        raise LiveAliasInputError(f"{label}.historical_alias.type must equal element_type")
    if destination_order[0] != alias_name:
        raise LiveAliasInputError(f"{label}.historical_alias must be the outer destination")
    alias_home = _uint(
        alias.get("stack_home_offset"), f"{label}.historical_alias.stack_home_offset", minimum=8
    )
    if alias_home != normalized_precursor["target_home_store"]["offset"]:
        raise LiveAliasInputError(f"{label}.historical_alias stack home does not match target store")
    normalized_alias = {
        "name": alias_name,
        "type": alias_type,
        "commit": commit,
        "declaration_authenticated": True,
        "live_at_consumer_boundary": True,
        "outer_assignment": True,
        "stack_home_offset": alias_home,
    }

    raw_controls = context.get("controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != len(_CONTROL_CLASSES):
        raise LiveAliasInputError(f"{label}.controls must contain the five sealed controls")
    controls: list[dict[str, str]] = []
    seen: set[str] = set()
    for index, raw in enumerate(raw_controls):
        control = _closed(
            raw,
            allowed={"kind", "result_class", "candidate_record_sha256"},
            required={"kind", "result_class", "candidate_record_sha256"},
            label=f"{label}.controls[{index}]",
        )
        kind = _text(control.get("kind"), f"{label}.controls[{index}].kind", limit=64)
        result_class = _text(
            control.get("result_class"), f"{label}.controls[{index}].result_class", limit=64
        )
        if kind not in _CONTROL_CLASSES or kind in seen or result_class != _CONTROL_CLASSES[kind]:
            raise LiveAliasInputError(f"{label}.controls do not match the sealed negative-control matrix")
        seen.add(kind)
        controls.append(
            {
                "kind": kind,
                "result_class": result_class,
                "candidate_record_sha256": _sha256(
                    control.get("candidate_record_sha256"),
                    f"{label}.controls[{index}].candidate_record_sha256",
                ),
            }
        )
    if seen != set(_CONTROL_CLASSES):
        raise LiveAliasInputError(f"{label}.controls must cover every sealed negative control")
    controls.sort(key=lambda item: item["kind"])

    telemetry = _closed(
        context.get("telemetry"),
        allowed={
            "active_seconds",
            "telemetry_complete",
            "exclude_from_measured_crack_hour",
            "telemetry_sha256",
        },
        required={
            "active_seconds",
            "telemetry_complete",
            "exclude_from_measured_crack_hour",
            "telemetry_sha256",
        },
        label=f"{label}.telemetry",
    )
    telemetry_complete = telemetry.get("telemetry_complete")
    excluded = telemetry.get("exclude_from_measured_crack_hour")
    if not isinstance(telemetry_complete, bool) or not isinstance(excluded, bool):
        raise LiveAliasInputError(f"{label}.telemetry completeness fields must be booleans")
    if not telemetry_complete and not excluded:
        raise LiveAliasInputError(f"{label}.telemetry incomplete evidence must be excluded")
    normalized_telemetry = {
        "active_seconds": _positive_number(
            telemetry.get("active_seconds"), f"{label}.telemetry.active_seconds"
        ),
        "telemetry_complete": telemetry_complete,
        "exclude_from_measured_crack_hour": excluded,
        "telemetry_sha256": _sha256(
            telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"
        ),
    }

    exact = _closed(
        context.get("exact_result"),
        allowed={
            "candidate_id",
            "target_bytes",
            "candidate_bytes",
            "physical_relocations",
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "candidate_record_sha256",
        },
        required={
            "candidate_id",
            "target_bytes",
            "candidate_bytes",
            "physical_relocations",
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "candidate_record_sha256",
        },
        label=f"{label}.exact_result",
    )
    exact_target = _uint(exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=8)
    exact_candidate = _uint(
        exact.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=8
    )
    exact_relocations = _uint(
        exact.get("physical_relocations"),
        f"{label}.exact_result.physical_relocations",
        minimum=1,
    )
    if (
        exact_target != exact_candidate
        or exact_target != target_bytes
        or exact_relocations != target_relocations
    ):
        raise LiveAliasInputError(f"{label}.exact_result must close target size and preserve relocations")
    normalized_exact: dict[str, Any] = {
        "candidate_id": _text(
            exact.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128
        ),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
    }
    for field in (
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "candidate_record_sha256",
    ):
        normalized_exact[field] = _sha256(exact.get(field), f"{label}.exact_result.{field}")

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "producer_consumer": normalized_producer,
        "historical_alias": normalized_alias,
        "controls": controls,
        "telemetry": normalized_telemetry,
        "exact_result": normalized_exact,
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:12]:
        if not instruction.has_instruction:
            continue
        match = _FRAME_RE.fullmatch(instruction.formatted)
        if match is not None:
            return int(match.group("size"), 0)
    return None


def _call_indices(instructions: Sequence[Any], symbol: str) -> list[int]:
    return [
        index
        for index, instruction in enumerate(instructions)
        if instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and re.search(rf"\b{re.escape(symbol)}\b", instruction.formatted) is not None
    ]


def _bind_indices(instructions: Sequence[Any], destination: str, source: str) -> list[int]:
    result: list[int] = []
    for index, instruction in enumerate(instructions):
        if not instruction.has_instruction:
            continue
        match = _MR_RE.fullmatch(instruction.formatted)
        if match is not None and (
            match.group("destination").lower(), match.group("source").lower()
        ) == (destination, source):
            result.append(index)
    return result


def _mismatch_rows(
    target: Sequence[Any], candidate: Sequence[Any]
) -> list[tuple[int, Any | None, Any | None]]:
    result: list[tuple[int, Any | None, Any | None]] = []
    for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate)):
        if causal_reducer._instruction_mismatch(left, right):
            result.append((index, left, right))
    return result


def evaluate(
    pair: Any,
    target: Sequence[Any],
    candidate: Sequence[Any],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {
            "matched": False,
            "reason": "no authenticated historical live-alias memset context was supplied",
        }
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {
            "matched": False,
            "reason": "the historical live-alias context is bound to another objdiff report",
        }

    precursor = context["precursor"]
    if pair.name != precursor["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = (
        causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    )
    observed = (
        target_size,
        candidate_size,
        _frame_size(target),
        _frame_size(candidate),
    )
    sealed = (
        precursor["target_bytes"],
        precursor["candidate_bytes"],
        precursor["target_frame"],
        precursor["candidate_frame"],
    )
    if observed != sealed:
        return {
            "matched": False,
            "reason": "the size/frame signature no longer matches the sealed precursor",
            "evidence": {"observed": list(observed), "sealed": list(sealed)},
        }

    mismatches = _mismatch_rows(target, candidate)
    mismatch_indices = [item[0] for item in mismatches]
    if mismatch_indices != precursor["residual_rows"]:
        return {
            "matched": False,
            "reason": "the physical residual rows differ from the sealed precursor",
            "evidence": {
                "report_residual_rows": mismatch_indices,
                "context_residual_rows": precursor["residual_rows"],
            },
        }
    target_only = [
        item
        for item in mismatches
        if item[1] is not None
        and item[1].has_instruction
        and (item[2] is None or not item[2].has_instruction)
    ]
    candidate_only = [
        item
        for item in mismatches
        if item[2] is not None
        and item[2].has_instruction
        and (item[1] is None or not item[1].has_instruction)
    ]
    if len(target_only) != 1 or candidate_only:
        return {
            "matched": False,
            "reason": "the report must contain exactly one target-only instruction and no candidate-only instruction",
        }
    home_row, home_instruction, _ = target_only[0]
    home_match = _STORE_RE.fullmatch(home_instruction.formatted)
    expected_home = precursor["target_home_store"]
    if home_match is None or (
        home_instruction.mnemonic,
        home_match.group("register").lower(),
        home_match.group("base").lower(),
        int(home_match.group("offset"), 0),
    ) != (
        expected_home["opcode"],
        expected_home["register"],
        expected_home["base_register"],
        expected_home["offset"],
    ):
        return {
            "matched": False,
            "reason": "the unique target-only instruction is not the sealed historical-alias home",
            "evidence": {"row": home_row, "formatted": home_instruction.formatted},
        }
    paired_mismatches = [item for item in mismatches if item[0] != home_row]
    if len(paired_mismatches) != 1:
        return {
            "matched": False,
            "reason": "only the aligned frame instruction may differ beside the target home",
        }
    _, target_frame_row, candidate_frame_row = paired_mismatches[0]
    if (
        target_frame_row is None
        or candidate_frame_row is None
        or target_frame_row.mnemonic != "stwu"
        or candidate_frame_row.mnemonic != "stwu"
    ):
        return {
            "matched": False,
            "reason": "the paired residual beside the target home is not the frame allocation",
        }

    producer = context["producer_consumer"]
    target_alloc = _call_indices(target, producer["allocation_symbol"])
    candidate_alloc = _call_indices(candidate, producer["allocation_symbol"])
    target_consumer = _call_indices(target, producer["consumer_symbol"])
    candidate_consumer = _call_indices(candidate, producer["consumer_symbol"])
    if any(len(items) != 1 for items in (target_alloc, candidate_alloc, target_consumer, candidate_consumer)):
        return {
            "matched": False,
            "reason": "the allocation and immediate memset call boundary must each occur exactly once",
        }
    target_bind = _bind_indices(
        target, producer["live_register"], producer["return_register"]
    )
    candidate_bind = _bind_indices(
        candidate, producer["live_register"], producer["return_register"]
    )
    if len(target_bind) != 1 or len(candidate_bind) != 1:
        return {
            "matched": False,
            "reason": "the sealed allocation-result to live-owner register bind is missing or ambiguous",
        }
    if not (
        target_alloc[0] < target_bind[0] < home_row < target_consumer[0]
        and candidate_alloc[0] < candidate_bind[0] < candidate_consumer[0]
    ):
        return {
            "matched": False,
            "reason": "the target store/forward order does not match the immediate consumer boundary",
        }

    alias = context["historical_alias"]
    expression = " = ".join(producer["destination_order"])
    evidence = {
        "precursor": precursor,
        "producer_consumer": producer,
        "historical_alias": alias,
        "target_home_row": home_row,
        "negative_controls": context["controls"],
        "recommended_cells": [
            {
                "kind": "historical_live_alias_outer_assignment_at_immediate_consumer",
                "consumer_symbol": producer["consumer_symbol"],
                "destination_expression": expression,
                "element_count": producer["element_count"],
                "element_type": producer["element_type"],
                "preserve_all_other_source_axes": True,
            }
        ],
        "suppressed_axes": [
            "fusion_without_authenticated_alias",
            "direct_allocator_result_chain",
            "type_only_alias",
            "sizeof_owner_changes",
            "separate_alias_statement",
            "declaration_permutations",
            "dead_or_fake_aliases",
            "padding",
            "register_shaping",
            "repeat_tracer_capture",
            "automatic_retention",
        ],
        "telemetry": context["telemetry"],
        "combined_exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "the exact-size-minus-four creator has one authenticated target-only r1+8 alias home, exact allocation/memset topology, a historical typed alias, and five measured controls that isolate outer assignment at the immediate consumer boundary",
        "confidence": 0.997,
        "source_class": "historical_live_alias_outer_assignment_at_memset_boundary",
        "recommendation": (
            f"Preserve {producer['allocation_owner']}, {producer['field_owner']}.{producer['field_name']}, "
            f"and {producer['live_owner']}; compile only memset({expression}, 0, "
            f"{producer['element_count']} * sizeof({producer['element_type']}))."
        ),
        "evidence": evidence,
    }
