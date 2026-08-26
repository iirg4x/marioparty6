#!/usr/bin/env python3
"""Fail-closed traced numbered-aggregate and rounded-reciprocal diagnosis."""

from __future__ import annotations

import math
import re
import struct
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "traced_naggregate_reciprocal_fold_context/v1"
RULE_ID = "traced_naggregate_reciprocal_fold"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_SESSION_RE = re.compile(r"session-[0-9a-f]{16}")
_TOKEN_RE = re.compile(r"local-[0-9]{6}")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_DFORM_RE = re.compile(
    r"^\s*(?P<opcode>[A-Za-z][A-Za-z0-9_.]*)\s+"
    r"(?P<register>[rf](?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*"
    r"\(\s*(?P<base>r(?:[0-9]|[12][0-9]|3[01]))\s*\)(?:\s*,.*)?\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "cfg_calls_exact",
    "physical_relocations_exact",
    "trace_same_session_authenticated",
    "traced_home_swap_closed",
    "same_tu_numbered_precedent_authenticated",
    "typed_pool_decoder_authenticated",
    "semantic_literals_authenticated",
    "exact_result_verified",
    "protected_siblings_preserved",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "trace_envelope_sha256",
    "trace_stack_events_sha256",
    "trace_pcode_events_sha256",
    "same_tu_precedent_receipt_sha256",
    "typed_pool_receipt_sha256",
    "precursor_source_sha256",
    "precursor_object_sha256",
    "precursor_record_sha256",
    "aggregate_exact_source_sha256",
    "aggregate_exact_object_sha256",
    "aggregate_exact_record_sha256",
    "pool_precursor_source_sha256",
    "pool_precursor_object_sha256",
    "pool_precursor_record_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "independent_rebuild_receipt_sha256",
    "report_artifact_sha256",
)
_POOL_ROLES = ("blue_base", "gravity_scaled", "vertical_offset")


class TracedAggregateFoldInputError(ValueError):
    """The supplied evidence cannot support this diagnosis safely."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise TracedAggregateFoldInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise TracedAggregateFoldInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise TracedAggregateFoldInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise TracedAggregateFoldInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise TracedAggregateFoldInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise TracedAggregateFoldInputError(f"{label} must be a bounded owner path")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise TracedAggregateFoldInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TracedAggregateFoldInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise TracedAggregateFoldInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _positive_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TracedAggregateFoldInputError(f"{label} must be a positive number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise TracedAggregateFoldInputError(f"{label} must be positive and finite")
    return result


def _rows(value: Any, label: str, *, minimum: int, maximum: int) -> list[int]:
    if not isinstance(value, list):
        raise TracedAggregateFoldInputError(f"{label} must be a row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or not minimum <= len(rows) <= maximum:
        raise TracedAggregateFoldInputError(
            f"{label} must be sorted, unique, and contain {minimum}-{maximum} rows"
        )
    return rows


def _bits(value: Any, label: str) -> str:
    result = _text(value, label, limit=8).lower()
    if re.fullmatch(r"[0-9a-f]{8}", result) is None:
        raise TracedAggregateFoldInputError(f"{label} must be eight lowercase hex digits")
    return result


def _f32(value: float) -> float:
    return struct.unpack(">f", struct.pack(">f", value))[0]


def _f32_bits(value: float) -> str:
    return struct.pack(">f", _f32(value)).hex()


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "traced numbered-aggregate reciprocal-fold context"
    fields = {
        "schema",
        "proofs",
        "precursor",
        "trace_cycle",
        "same_tu_precedent",
        "semantic_pool_batch",
        "rounded_reciprocal",
        "telemetry",
        "exact_result",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise TracedAggregateFoldInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES) | {"authority_advanced"}
    proofs = _closed(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label=f"{label}.proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise TracedAggregateFoldInputError(f"{label}.proofs.{field} must be true")
        normalized_proofs[field] = True
    if proofs.get("authority_advanced") is not False:
        raise TracedAggregateFoldInputError(f"{label}.proofs.authority_advanced must be false")
    normalized_proofs["authority_advanced"] = False
    for field in _PROOF_HASHES:
        normalized_proofs[field] = _sha256(proofs.get(field), f"{label}.proofs.{field}")

    precursor_fields = {
        "function",
        "candidate_id",
        "target_bytes",
        "candidate_bytes",
        "target_frame",
        "candidate_frame",
        "match_percent",
        "target_physical_relocations",
        "candidate_physical_relocations",
        "cycle_rows",
        "pool_rows",
    }
    precursor = _closed(
        context.get("precursor"),
        allowed=precursor_fields,
        required=precursor_fields,
        label=f"{label}.precursor",
    )
    target_bytes = _uint(precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=32)
    candidate_bytes = _uint(precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=32)
    target_frame = _uint(precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16)
    candidate_frame = _uint(precursor.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16)
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
    match_percent = _positive_number(precursor.get("match_percent"), f"{label}.precursor.match_percent")
    cycle_rows = _rows(precursor.get("cycle_rows"), f"{label}.precursor.cycle_rows", minimum=2, maximum=16)
    pool_rows = _rows(precursor.get("pool_rows"), f"{label}.precursor.pool_rows", minimum=3, maximum=3)
    if target_bytes != candidate_bytes or target_frame != candidate_frame:
        raise TracedAggregateFoldInputError(f"{label}.precursor size and frame must be exact")
    if target_relocations != candidate_relocations or match_percent >= 100.0:
        raise TracedAggregateFoldInputError(
            f"{label}.precursor must be nonexact with exact relocation count"
        )
    if set(cycle_rows) & set(pool_rows):
        raise TracedAggregateFoldInputError(f"{label}.precursor row groups must be disjoint")
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _owner(precursor.get("candidate_id"), f"{label}.precursor.candidate_id"),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "target_physical_relocations": target_relocations,
        "candidate_physical_relocations": candidate_relocations,
        "cycle_rows": cycle_rows,
        "pool_rows": pool_rows,
    }

    cycle_fields = {
        "session_id",
        "aggregate_type",
        "aggregate_base_name",
        "aggregate_count",
        "aggregate_size",
        "target_aggregate_homes",
        "scalar_identity",
        "scalar_trace_token",
        "aggregate_identity",
        "aggregate_trace_token",
        "scalar_target_home",
        "scalar_candidate_home",
        "aggregate_target_home",
        "aggregate_candidate_home",
        "seam_unknown_count",
        "alias_summary_complete",
    }
    cycle = _closed(
        context.get("trace_cycle"),
        allowed=cycle_fields,
        required=cycle_fields,
        label=f"{label}.trace_cycle",
    )
    session_id = _text(cycle.get("session_id"), f"{label}.trace_cycle.session_id", limit=64)
    if _SESSION_RE.fullmatch(session_id) is None:
        raise TracedAggregateFoldInputError(f"{label}.trace_cycle.session_id is not canonical")
    aggregate_type = _identifier(cycle.get("aggregate_type"), f"{label}.trace_cycle.aggregate_type")
    base_name = _identifier(cycle.get("aggregate_base_name"), f"{label}.trace_cycle.aggregate_base_name")
    count = _uint(cycle.get("aggregate_count"), f"{label}.trace_cycle.aggregate_count", minimum=2, maximum=8)
    size = _uint(cycle.get("aggregate_size"), f"{label}.trace_cycle.aggregate_size", minimum=1, maximum=64)
    if aggregate_type != "GXColor" or base_name != "color" or size != 4:
        raise TracedAggregateFoldInputError(
            f"{label}.trace_cycle must describe numbered four-byte GXColor snapshots"
        )
    raw_homes = cycle.get("target_aggregate_homes")
    if not isinstance(raw_homes, list) or len(raw_homes) != count:
        raise TracedAggregateFoldInputError(f"{label}.trace_cycle target homes must cover all aggregates")
    homes = [_uint(item, f"{label}.trace_cycle.target_aggregate_homes") for item in raw_homes]
    if homes != [homes[0] - size * index for index in range(count)]:
        raise TracedAggregateFoldInputError(
            f"{label}.trace_cycle target aggregate homes must be descending and adjacent"
        )
    scalar_target = _uint(cycle.get("scalar_target_home"), f"{label}.trace_cycle.scalar_target_home")
    scalar_candidate = _uint(cycle.get("scalar_candidate_home"), f"{label}.trace_cycle.scalar_candidate_home")
    aggregate_target = _uint(cycle.get("aggregate_target_home"), f"{label}.trace_cycle.aggregate_target_home")
    aggregate_candidate = _uint(cycle.get("aggregate_candidate_home"), f"{label}.trace_cycle.aggregate_candidate_home")
    if aggregate_target != homes[-1] or scalar_target != homes[-1] - size:
        raise TracedAggregateFoldInputError(
            f"{label}.trace_cycle target homes do not form the sealed aggregate/scalar seam"
        )
    if (scalar_candidate, aggregate_candidate) != (aggregate_target, scalar_target):
        raise TracedAggregateFoldInputError(
            f"{label}.trace_cycle candidate homes must be the closed two-owner swap"
        )
    if cycle.get("seam_unknown_count") != 0 or cycle.get("alias_summary_complete") is not True:
        raise TracedAggregateFoldInputError(
            f"{label}.trace_cycle must have zero UNKNOWN and a complete alias summary"
        )
    scalar_token = _text(cycle.get("scalar_trace_token"), f"{label}.trace_cycle.scalar_trace_token", limit=16)
    aggregate_token = _text(cycle.get("aggregate_trace_token"), f"{label}.trace_cycle.aggregate_trace_token", limit=16)
    if _TOKEN_RE.fullmatch(scalar_token) is None or _TOKEN_RE.fullmatch(aggregate_token) is None or scalar_token == aggregate_token:
        raise TracedAggregateFoldInputError(f"{label}.trace_cycle trace tokens must be distinct canonical locals")
    normalized_cycle = {
        "session_id": session_id,
        "aggregate_type": aggregate_type,
        "aggregate_base_name": base_name,
        "aggregate_count": count,
        "aggregate_size": size,
        "target_aggregate_homes": homes,
        "scalar_identity": _identifier(cycle.get("scalar_identity"), f"{label}.trace_cycle.scalar_identity"),
        "scalar_trace_token": scalar_token,
        "aggregate_identity": _identifier(cycle.get("aggregate_identity"), f"{label}.trace_cycle.aggregate_identity"),
        "aggregate_trace_token": aggregate_token,
        "scalar_target_home": scalar_target,
        "scalar_candidate_home": scalar_candidate,
        "aggregate_target_home": aggregate_target,
        "aggregate_candidate_home": aggregate_candidate,
        "seam_unknown_count": 0,
        "alias_summary_complete": True,
    }

    precedent_fields = {
        "owner",
        "source_file",
        "same_translation_unit",
        "narrow_verified",
        "numbered_precedent_authenticated",
        "declarations",
    }
    precedent = _closed(
        context.get("same_tu_precedent"),
        allowed=precedent_fields,
        required=precedent_fields,
        label=f"{label}.same_tu_precedent",
    )
    for field in ("same_translation_unit", "narrow_verified", "numbered_precedent_authenticated"):
        if precedent.get(field) is not True:
            raise TracedAggregateFoldInputError(f"{label}.same_tu_precedent.{field} must be true")
    raw_declarations = precedent.get("declarations")
    if not isinstance(raw_declarations, list) or len(raw_declarations) < 2:
        raise TracedAggregateFoldInputError(f"{label}.same_tu_precedent requires at least two declarations")
    declarations: list[dict[str, Any]] = []
    identities: list[str] = []
    for index, raw in enumerate(raw_declarations):
        declaration = _closed(
            raw,
            allowed={"function", "identity", "source_line"},
            required={"function", "identity", "source_line"},
            label=f"{label}.same_tu_precedent.declarations[{index}]",
        )
        identity = _identifier(declaration.get("identity"), f"{label}.same_tu_precedent.declarations[{index}].identity")
        identities.append(identity)
        declarations.append(
            {
                "function": _identifier(declaration.get("function"), f"{label}.same_tu_precedent.declarations[{index}].function"),
                "identity": identity,
                "source_line": _uint(declaration.get("source_line"), f"{label}.same_tu_precedent.declarations[{index}].source_line", minimum=1),
            }
        )
    expected_prefix = [f"{base_name}{index}" for index in range(1, len(identities) + 1)]
    if identities != expected_prefix or len(set(item["function"] for item in declarations)) < 2:
        raise TracedAggregateFoldInputError(
            f"{label}.same_tu_precedent must authenticate gap-free numbering in at least two functions"
        )
    normalized_precedent = {
        "owner": _owner(precedent.get("owner"), f"{label}.same_tu_precedent.owner"),
        "source_file": _owner(precedent.get("source_file"), f"{label}.same_tu_precedent.source_file"),
        "same_translation_unit": True,
        "narrow_verified": True,
        "numbered_precedent_authenticated": True,
        "declarations": declarations,
    }

    raw_pool = context.get("semantic_pool_batch")
    if not isinstance(raw_pool, list) or len(raw_pool) != 3:
        raise TracedAggregateFoldInputError(f"{label}.semantic_pool_batch must have exactly three entries")
    pool: list[dict[str, Any]] = []
    roles: list[str] = []
    for index, raw in enumerate(raw_pool):
        entry = _closed(
            raw,
            allowed={"role", "consumer_state", "row", "target_f32_bits", "expression", "typed_f32", "semantic_consumer_authenticated"},
            required={"role", "consumer_state", "row", "target_f32_bits", "expression", "typed_f32", "semantic_consumer_authenticated"},
            label=f"{label}.semantic_pool_batch[{index}]",
        )
        role = _identifier(entry.get("role"), f"{label}.semantic_pool_batch[{index}].role")
        expression = _text(entry.get("expression"), f"{label}.semantic_pool_batch[{index}].expression", limit=256)
        if entry.get("typed_f32") is not True or entry.get("semantic_consumer_authenticated") is not True:
            raise TracedAggregateFoldInputError(f"{label}.semantic_pool_batch entries must be authenticated f32 consumers")
        if "0x" in expression.lower() or "union" in expression.lower() or "memcpy" in expression.lower():
            raise TracedAggregateFoldInputError(f"{label}.semantic_pool_batch forbids opaque bit-pattern source")
        roles.append(role)
        pool.append(
            {
                "role": role,
                "consumer_state": _uint(entry.get("consumer_state"), f"{label}.semantic_pool_batch[{index}].consumer_state", maximum=32),
                "row": _uint(entry.get("row"), f"{label}.semantic_pool_batch[{index}].row", maximum=1 << 20),
                "target_f32_bits": _bits(entry.get("target_f32_bits"), f"{label}.semantic_pool_batch[{index}].target_f32_bits"),
                "expression": expression,
                "typed_f32": True,
                "semantic_consumer_authenticated": True,
            }
        )
    if tuple(roles) != _POOL_ROLES or [item["row"] for item in pool] != pool_rows:
        raise TracedAggregateFoldInputError(
            f"{label}.semantic_pool_batch must be ordered blue/gravity/vertical and bind all pool rows"
        )
    if pool[0]["target_f32_bits"] != "42800000" or pool[2]["target_f32_bits"] != "42480000":
        raise TracedAggregateFoldInputError(f"{label}.semantic literal bits must bind 64.0f and 50.0f")

    reciprocal_fields = {
        "numerator_identity",
        "numerator_f32_bits",
        "denominator",
        "reciprocal_f32_bits",
        "direct_division_f32_bits",
        "reciprocal_multiply_f32_bits",
        "target_f32_bits",
        "direct_expression",
        "reciprocal_expression",
        "one_ulp_residual",
    }
    reciprocal = _closed(
        context.get("rounded_reciprocal"),
        allowed=reciprocal_fields,
        required=reciprocal_fields,
        label=f"{label}.rounded_reciprocal",
    )
    numerator_bits = _bits(reciprocal.get("numerator_f32_bits"), f"{label}.rounded_reciprocal.numerator_f32_bits")
    denominator = _uint(reciprocal.get("denominator"), f"{label}.rounded_reciprocal.denominator", minimum=3, maximum=1 << 20)
    if denominator & (denominator - 1) == 0:
        raise TracedAggregateFoldInputError(f"{label}.rounded_reciprocal denominator must be non-power-of-two")
    reciprocal_bits = _bits(reciprocal.get("reciprocal_f32_bits"), f"{label}.rounded_reciprocal.reciprocal_f32_bits")
    direct_bits = _bits(reciprocal.get("direct_division_f32_bits"), f"{label}.rounded_reciprocal.direct_division_f32_bits")
    multiply_bits = _bits(reciprocal.get("reciprocal_multiply_f32_bits"), f"{label}.rounded_reciprocal.reciprocal_multiply_f32_bits")
    target_bits = _bits(reciprocal.get("target_f32_bits"), f"{label}.rounded_reciprocal.target_f32_bits")
    numerator = struct.unpack(">f", bytes.fromhex(numerator_bits))[0]
    expected_reciprocal = _f32(1.0 / denominator)
    expected_direct = _f32(numerator / _f32(float(denominator)))
    expected_multiply = _f32(numerator * expected_reciprocal)
    if reciprocal_bits != _f32_bits(expected_reciprocal) or direct_bits != _f32_bits(expected_direct) or multiply_bits != _f32_bits(expected_multiply):
        raise TracedAggregateFoldInputError(f"{label}.rounded_reciprocal does not reproduce two-stage f32 folding")
    if target_bits != multiply_bits or abs(int(target_bits, 16) - int(direct_bits, 16)) != 1 or reciprocal.get("one_ulp_residual") is not True:
        raise TracedAggregateFoldInputError(f"{label}.rounded_reciprocal must bind one ULP to the multiply result")
    direct_expression = _text(reciprocal.get("direct_expression"), f"{label}.rounded_reciprocal.direct_expression", limit=256)
    multiply_expression = _text(reciprocal.get("reciprocal_expression"), f"{label}.rounded_reciprocal.reciprocal_expression", limit=256)
    if "0x" in direct_expression.lower() or "0x" in multiply_expression.lower():
        raise TracedAggregateFoldInputError(f"{label}.rounded_reciprocal expressions must be semantic C")
    normalized_reciprocal = {
        "numerator_identity": _identifier(reciprocal.get("numerator_identity"), f"{label}.rounded_reciprocal.numerator_identity"),
        "numerator_f32_bits": numerator_bits,
        "denominator": denominator,
        "reciprocal_f32_bits": reciprocal_bits,
        "direct_division_f32_bits": direct_bits,
        "reciprocal_multiply_f32_bits": multiply_bits,
        "target_f32_bits": target_bits,
        "direct_expression": direct_expression,
        "reciprocal_expression": multiply_expression,
        "one_ulp_residual": True,
    }
    if pool[1]["target_f32_bits"] != target_bits or pool[1]["expression"] != direct_expression:
        raise TracedAggregateFoldInputError(f"{label}.gravity pool row must bind the direct-fold precursor")

    telemetry_fields = {
        "parent_active_seconds",
        "active_seconds_measured",
        "telemetry_complete",
        "exclude_from_measured_crack_hour",
        "no_imputation",
        "telemetry_sha256",
        "active_interval_log_sha256",
    }
    telemetry = _closed(
        context.get("telemetry"),
        allowed=telemetry_fields,
        required=telemetry_fields,
        label=f"{label}.telemetry",
    )
    if telemetry.get("active_seconds_measured") is not True or telemetry.get("telemetry_complete") is not False or telemetry.get("exclude_from_measured_crack_hour") is not True or telemetry.get("no_imputation") is not True:
        raise TracedAggregateFoldInputError(f"{label}.telemetry must preserve incomplete-coverage exclusion without imputation")
    normalized_telemetry = {
        "parent_active_seconds": _positive_number(telemetry.get("parent_active_seconds"), f"{label}.telemetry.parent_active_seconds"),
        "active_seconds_measured": True,
        "telemetry_complete": False,
        "exclude_from_measured_crack_hour": True,
        "no_imputation": True,
        "telemetry_sha256": _sha256(telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"),
        "active_interval_log_sha256": _sha256(telemetry.get("active_interval_log_sha256"), f"{label}.telemetry.active_interval_log_sha256"),
    }

    exact_fields = {
        "candidate_id",
        "target_bytes",
        "candidate_bytes",
        "physical_relocations",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "candidate_record_sha256",
    }
    exact = _closed(
        context.get("exact_result"),
        allowed=exact_fields,
        required=exact_fields,
        label=f"{label}.exact_result",
    )
    exact_target = _uint(exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=32)
    exact_candidate = _uint(exact.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=32)
    exact_relocations = _uint(exact.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1)
    if (exact_target, exact_candidate, exact_relocations) != (target_bytes, candidate_bytes, target_relocations):
        raise TracedAggregateFoldInputError(f"{label}.exact_result drifts from precursor identity")
    normalized_exact = {
        "candidate_id": _owner(exact.get("candidate_id"), f"{label}.exact_result.candidate_id"),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
    }
    hash_pairs = {
        "source_sha256": "exact_source_sha256",
        "object_sha256": "exact_object_sha256",
        "strict_report_sha256": "exact_strict_report_sha256",
        "data_report_sha256": "exact_data_report_sha256",
        "candidate_record_sha256": "exact_record_sha256",
    }
    for field, proof_field in hash_pairs.items():
        normalized_exact[field] = _sha256(exact.get(field), f"{label}.exact_result.{field}")
        if normalized_exact[field] != normalized_proofs[proof_field]:
            raise TracedAggregateFoldInputError(f"{label}.exact_result.{field} drifts from proofs")

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "trace_cycle": normalized_cycle,
        "same_tu_precedent": normalized_precedent,
        "semantic_pool_batch": pool,
        "rounded_reciprocal": normalized_reciprocal,
        "telemetry": normalized_telemetry,
        "exact_result": normalized_exact,
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:16]:
        if instruction.has_instruction:
            match = _FRAME_RE.fullmatch(instruction.formatted)
            if match is not None:
                return int(match.group("size"), 0)
    return None


def _mismatch_rows(target: Sequence[Any], candidate: Sequence[Any]) -> list[int]:
    return [
        index
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if causal_reducer._instruction_mismatch(left, right)
    ]


def _dform(instruction: Any) -> tuple[str, int, str] | None:
    if instruction is None or not instruction.has_instruction:
        return None
    match = _DFORM_RE.fullmatch(instruction.formatted)
    if match is None:
        return None
    return match.group("opcode").lower(), int(match.group("offset"), 0), match.group("base").lower()


def evaluate(
    pair: Any,
    target: Sequence[Any],
    candidate: Sequence[Any],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {"matched": False, "reason": "no authenticated traced numbered-aggregate reciprocal-fold context was supplied"}
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {"matched": False, "reason": "the context is bound to another objdiff report"}
    precursor = context["precursor"]
    if pair.name != precursor["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    observed = (target_size, candidate_size, _frame_size(target), _frame_size(candidate))
    sealed = (
        precursor["target_bytes"],
        precursor["candidate_bytes"],
        precursor["target_frame"],
        precursor["candidate_frame"],
    )
    if observed != sealed:
        return {"matched": False, "reason": "the size/frame signature drifted", "evidence": {"observed": list(observed), "sealed": list(sealed)}}
    residual_rows = _mismatch_rows(target, candidate)
    sealed_rows = sorted(precursor["cycle_rows"] + precursor["pool_rows"])
    if residual_rows != sealed_rows:
        return {"matched": False, "reason": "the physical residual rows differ from the sealed cycle and pool groups", "evidence": {"report_residual_rows": residual_rows, "context_residual_rows": sealed_rows}}
    paired = causal_reducer._paired_records(target, candidate)
    cycle = context["trace_cycle"]
    observed_stack_pairs: set[tuple[int, int]] = set()
    for row in precursor["cycle_rows"]:
        left, right = paired[row]
        left_dform = _dform(left)
        right_dform = _dform(right)
        if left_dform is None or right_dform is None or left_dform[0] != right_dform[0] or left_dform[2] != "r1" or right_dform[2] != "r1":
            return {"matched": False, "reason": "a cycle row is not one paired same-opcode r1 stack access", "evidence": {"row": row}}
        observed_stack_pairs.add((left_dform[1], right_dform[1]))
    expected_pairs = {
        (cycle["scalar_target_home"], cycle["scalar_candidate_home"]),
        (cycle["aggregate_target_home"], cycle["aggregate_candidate_home"]),
    }
    if not expected_pairs.issubset(observed_stack_pairs):
        return {"matched": False, "reason": "the report does not contain the sealed aggregate/scalar home swap", "evidence": {"observed_stack_pairs": sorted(observed_stack_pairs)}}

    declarations = [f"GXColor {cycle['aggregate_base_name']}{index};" for index in range(1, cycle["aggregate_count"] + 1)]
    reciprocal = context["rounded_reciprocal"]
    evidence = {
        "precursor": precursor,
        "trace_cycle": cycle,
        "same_tu_precedent": context["same_tu_precedent"],
        "semantic_pool_batch": context["semantic_pool_batch"],
        "rounded_reciprocal": reciprocal,
        "recommended_cells": [
            {
                "order": 1,
                "kind": "numbered_function_scope_aggregate_chronology",
                "declarations": declarations,
                "target_homes": cycle["target_aggregate_homes"],
                "preserve_scalar_identity": cycle["scalar_identity"],
                "suppress_lexical_scope_permutations": True,
            },
            {
                "order": 2,
                "kind": "typed_semantic_pool_batch",
                "entries": context["semantic_pool_batch"],
                "compile_as_one_cell": True,
            },
            {
                "order": 3,
                "kind": "rounded_reciprocal_multiply",
                "replace": reciprocal["direct_expression"],
                "with": reciprocal["reciprocal_expression"],
                "direct_f32_bits": reciprocal["direct_division_f32_bits"],
                "target_f32_bits": reciprocal["target_f32_bits"],
                "ulp_distance": 1,
            },
        ],
        "suppressed_axes": [
            "lexical_scope_permutations",
            "initializer_only_permutations",
            "single_outer_aggregate_probes",
            "declaration_only_scalar_probes",
            "guessed_decimal_literal",
            "opaque_bit_pattern_literal",
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "repeat_tracer_after_closed_cycle",
            "automatic_retention_or_promotion",
        ],
        "telemetry": context["telemetry"],
        "exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "same-session owners close one scalar/aggregate stack-home swap, same-TU numbering predicts all aggregate homes, and typed pool evidence proves a one-ULP rounded-reciprocal fold",
        "confidence": 0.995,
        "source_class": "numbered_function_scope_aggregates_then_semantic_pool_and_reciprocal_fold",
        "recommendation": "Compile the numbered function-scope aggregate chronology first, batch the three authenticated semantic pool values second, then test only the rounded-reciprocal multiplication for the sealed one-ULP residual.",
        "evidence": evidence,
    }
