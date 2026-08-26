#!/usr/bin/env python3
"""Fail-closed saved-FPR donor composition and typed-pool handoff."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "saved_fpr_stack_pool_composer_context/v1"
RULE_ID = "saved_fpr_stack_pool_composer"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")


class SavedFprStackPoolInputError(ValueError):
    """The supplied evidence cannot safely support this diagnosis."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SavedFprStackPoolInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise SavedFprStackPoolInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise SavedFprStackPoolInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SavedFprStackPoolInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise SavedFprStackPoolInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise SavedFprStackPoolInputError(f"{label} must be a bounded owner token")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if result != result.lower() or _SHA256_RE.fullmatch(result) is None:
        raise SavedFprStackPoolInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(
    value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24
) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise SavedFprStackPoolInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise SavedFprStackPoolInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SavedFprStackPoolInputError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise SavedFprStackPoolInputError(
            f"{label} must be finite and at least {minimum}"
        )
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise SavedFprStackPoolInputError(
            f"{label} must be {str(expected).lower()}"
        )
    return expected


def _rows(value: Any, label: str, *, minimum: int = 1) -> list[int]:
    if not isinstance(value, list):
        raise SavedFprStackPoolInputError(f"{label} must be an array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) < minimum:
        raise SavedFprStackPoolInputError(
            f"{label} must be sorted, unique, and contain at least {minimum} row(s)"
        )
    return rows


def _stage(
    value: Any, *, label: str, pool: bool
) -> dict[str, Any]:
    fields = {
        "candidate_id",
        "objdiff_canonical_sha256",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "target_bytes",
        "candidate_bytes",
        "match_percent",
        "target_physical_relocations",
        "candidate_physical_relocations",
    }
    if pool:
        fields |= {
            "residual_rows",
            "target_owner",
            "candidate_owner",
            "target_value",
            "candidate_value",
        }
    else:
        fields |= {
            "candidate_only_stack_store",
            "candidate_only_stack_owner",
            "candidate_only_stack_offset",
            "stack_home_delta_bytes",
            "minimum_residual_rows",
        }
    stage = _closed(value, allowed=fields, required=fields, label=label)
    target_bytes = _uint(stage.get("target_bytes"), f"{label}.target_bytes", minimum=32)
    candidate_bytes = _uint(
        stage.get("candidate_bytes"), f"{label}.candidate_bytes", minimum=32
    )
    target_relocs = _uint(
        stage.get("target_physical_relocations"),
        f"{label}.target_physical_relocations",
        minimum=1,
    )
    candidate_relocs = _uint(
        stage.get("candidate_physical_relocations"),
        f"{label}.candidate_physical_relocations",
        minimum=1,
    )
    match_percent = _number(
        stage.get("match_percent"), f"{label}.match_percent", minimum=90.0
    )
    if target_relocs != candidate_relocs or not match_percent < 100.0:
        raise SavedFprStackPoolInputError(
            f"{label} must be nonexact with equal physical relocation counts"
        )
    if pool and target_bytes != candidate_bytes:
        raise SavedFprStackPoolInputError(f"{label} must have exact code size")
    if not pool and candidate_bytes <= target_bytes:
        raise SavedFprStackPoolInputError(
            f"{label} must preserve the candidate-only stack-store size excess"
        )
    result = {
        "candidate_id": _owner(stage.get("candidate_id"), f"{label}.candidate_id"),
        "objdiff_canonical_sha256": _sha256(
            stage.get("objdiff_canonical_sha256"),
            f"{label}.objdiff_canonical_sha256",
        ),
        "source_sha256": _sha256(stage.get("source_sha256"), f"{label}.source_sha256"),
        "object_sha256": _sha256(stage.get("object_sha256"), f"{label}.object_sha256"),
        "strict_report_sha256": _sha256(
            stage.get("strict_report_sha256"), f"{label}.strict_report_sha256"
        ),
        "data_report_sha256": _sha256(
            stage.get("data_report_sha256"), f"{label}.data_report_sha256"
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "match_percent": match_percent,
        "target_physical_relocations": target_relocs,
        "candidate_physical_relocations": candidate_relocs,
    }
    if pool:
        target_value = _number(stage.get("target_value"), f"{label}.target_value")
        candidate_value = _number(
            stage.get("candidate_value"), f"{label}.candidate_value"
        )
        if target_value == candidate_value:
            raise SavedFprStackPoolInputError(
                f"{label} target and candidate pool values must differ"
            )
        residual_rows = _rows(
            stage.get("residual_rows"), f"{label}.residual_rows", minimum=3
        )
        if len(residual_rows) != 3:
            raise SavedFprStackPoolInputError(
                f"{label}.residual_rows must contain exactly three pool rows"
            )
        result.update(
            {
                "residual_rows": residual_rows,
                "target_owner": _owner(
                    stage.get("target_owner"), f"{label}.target_owner"
                ),
                "candidate_owner": _owner(
                    stage.get("candidate_owner"), f"{label}.candidate_owner"
                ),
                "target_value": target_value,
                "candidate_value": candidate_value,
            }
        )
    else:
        delta = _uint(
            stage.get("stack_home_delta_bytes"),
            f"{label}.stack_home_delta_bytes",
            minimum=4,
            maximum=64,
        )
        offset = _uint(
            stage.get("candidate_only_stack_offset"),
            f"{label}.candidate_only_stack_offset",
            minimum=4,
            maximum=4096,
        )
        if delta % 4 != 0 or offset % 4 != 0:
            raise SavedFprStackPoolInputError(
                f"{label} stack offset and delta must be word aligned"
            )
        result.update(
            {
                "candidate_only_stack_store": _text(
                    stage.get("candidate_only_stack_store"),
                    f"{label}.candidate_only_stack_store",
                    limit=96,
                ),
                "candidate_only_stack_owner": _identifier(
                    stage.get("candidate_only_stack_owner"),
                    f"{label}.candidate_only_stack_owner",
                ),
                "candidate_only_stack_offset": offset,
                "stack_home_delta_bytes": delta,
                "minimum_residual_rows": _uint(
                    stage.get("minimum_residual_rows"),
                    f"{label}.minimum_residual_rows",
                    minimum=10,
                ),
            }
        )
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "saved-FPR stack/pool context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "function",
        "composition_stage",
        "pool_handoff_stage",
        "trace",
        "donor",
        "interaction",
        "negative_controls",
        "telemetry",
        "exact_result",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SavedFprStackPoolInputError(
            f"{label}.schema must be {CONTEXT_SCHEMA}"
        )
    function = _identifier(context.get("function"), f"{label}.function")
    composition = _stage(
        context.get("composition_stage"),
        label=f"{label}.composition_stage",
        pool=False,
    )
    pool_handoff = _stage(
        context.get("pool_handoff_stage"),
        label=f"{label}.pool_handoff_stage",
        pool=True,
    )
    if (
        composition["target_bytes"] != pool_handoff["target_bytes"]
        or composition["target_physical_relocations"]
        != pool_handoff["target_physical_relocations"]
        or composition["objdiff_canonical_sha256"]
        == pool_handoff["objdiff_canonical_sha256"]
    ):
        raise SavedFprStackPoolInputError(
            f"{label} stages must share the target but bind distinct reports"
        )

    trace_fields = {
        "status",
        "authoritative_scope",
        "unknown_ownership_present",
        "regalloc_complete",
        "object_inventory_authenticated",
        "stack_offset_authenticated",
        "envelope_sha256",
        "source_sha256",
        "request_sha256",
        "stack_stream_sha256",
        "pcode_stream_sha256",
    }
    trace = _closed(
        context.get("trace"),
        allowed=trace_fields,
        required=trace_fields,
        label=f"{label}.trace",
    )
    if (
        _text(trace.get("status"), f"{label}.trace.status")
        != "CAPTURED_UNKNOWN_OWNERSHIP"
        or _text(
            trace.get("authoritative_scope"),
            f"{label}.trace.authoritative_scope",
        )
        != "object_inventory_and_stack_offset_only"
    ):
        raise SavedFprStackPoolInputError(
            f"{label}.trace must preserve the UNKNOWN ownership boundary"
        )
    normalized_trace = {
        "status": "CAPTURED_UNKNOWN_OWNERSHIP",
        "authoritative_scope": "object_inventory_and_stack_offset_only",
        "unknown_ownership_present": _bool(
            trace.get("unknown_ownership_present"),
            f"{label}.trace.unknown_ownership_present",
            True,
        ),
        "regalloc_complete": _bool(
            trace.get("regalloc_complete"),
            f"{label}.trace.regalloc_complete",
            False,
        ),
        "object_inventory_authenticated": _bool(
            trace.get("object_inventory_authenticated"),
            f"{label}.trace.object_inventory_authenticated",
            True,
        ),
        "stack_offset_authenticated": _bool(
            trace.get("stack_offset_authenticated"),
            f"{label}.trace.stack_offset_authenticated",
            True,
        ),
    }
    for field in (
        "envelope_sha256",
        "source_sha256",
        "request_sha256",
        "stack_stream_sha256",
        "pcode_stream_sha256",
    ):
        normalized_trace[field] = _sha256(trace.get(field), f"{label}.trace.{field}")
    if normalized_trace["source_sha256"] != composition["source_sha256"]:
        raise SavedFprStackPoolInputError(
            f"{label}.trace.source_sha256 must bind the composition stage"
        )

    donor_fields = {
        "function",
        "same_translation_unit",
        "strict_exact",
        "graphify_location",
        "source_shape",
    }
    donor = _closed(
        context.get("donor"),
        allowed=donor_fields,
        required=donor_fields,
        label=f"{label}.donor",
    )
    normalized_donor = {
        "function": _identifier(donor.get("function"), f"{label}.donor.function"),
        "same_translation_unit": _bool(
            donor.get("same_translation_unit"),
            f"{label}.donor.same_translation_unit",
            True,
        ),
        "strict_exact": _bool(
            donor.get("strict_exact"), f"{label}.donor.strict_exact", True
        ),
        "graphify_location": _text(
            donor.get("graphify_location"), f"{label}.donor.graphify_location"
        ),
        "source_shape": _text(
            donor.get("source_shape"), f"{label}.donor.source_shape"
        ),
    }
    if normalized_donor["function"] == function:
        raise SavedFprStackPoolInputError(
            f"{label}.donor must be a distinct same-TU exact function"
        )

    interaction_fields = {
        "request_sha256",
        "planner_sha256",
        "priority_cell",
        "axes",
        "rank_within_top",
    }
    interaction = _closed(
        context.get("interaction"),
        allowed=interaction_fields,
        required=interaction_fields,
        label=f"{label}.interaction",
    )
    axes = interaction.get("axes")
    expected_axes = ["reuse_value", "distinct_distance2", "exact_donor_extended"]
    if axes != expected_axes:
        raise SavedFprStackPoolInputError(
            f"{label}.interaction.axes must select the sealed three-axis cell"
        )
    normalized_interaction = {
        "request_sha256": _sha256(
            interaction.get("request_sha256"), f"{label}.interaction.request_sha256"
        ),
        "planner_sha256": _sha256(
            interaction.get("planner_sha256"), f"{label}.interaction.planner_sha256"
        ),
        "priority_cell": _owner(
            interaction.get("priority_cell"), f"{label}.interaction.priority_cell"
        ),
        "axes": list(axes),
        "rank_within_top": _uint(
            interaction.get("rank_within_top"),
            f"{label}.interaction.rank_within_top",
            minimum=1,
            maximum=3,
        ),
    }

    raw_controls = context.get("negative_controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != 4:
        raise SavedFprStackPoolInputError(
            f"{label}.negative_controls must contain the four measured regressions"
        )
    controls: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_controls):
        item = _closed(
            raw,
            allowed={"candidate_id", "axis", "strict_percent", "measured", "result"},
            required={"candidate_id", "axis", "strict_percent", "measured", "result"},
            label=f"{label}.negative_controls[{index}]",
        )
        controls.append(
            {
                "candidate_id": _owner(
                    item.get("candidate_id"),
                    f"{label}.negative_controls[{index}].candidate_id",
                ),
                "axis": _identifier(
                    item.get("axis"), f"{label}.negative_controls[{index}].axis"
                ),
                "strict_percent": _number(
                    item.get("strict_percent"),
                    f"{label}.negative_controls[{index}].strict_percent",
                    minimum=90.0,
                ),
                "measured": _bool(
                    item.get("measured"),
                    f"{label}.negative_controls[{index}].measured",
                    True,
                ),
                "result": _text(
                    item.get("result"),
                    f"{label}.negative_controls[{index}].result",
                ),
            }
        )
    if [item["axis"] for item in controls] != [
        "distinct_distance_only",
        "broad_historical_prefix",
        "broad_fpr_grouping",
        "direct_trig_consumption",
    ]:
        raise SavedFprStackPoolInputError(
            f"{label}.negative_controls must preserve the canonical exhausted axes"
        )
    if any(item["strict_percent"] >= composition["match_percent"] for item in controls):
        raise SavedFprStackPoolInputError(
            f"{label}.negative_controls must all be measured regressions from the composition precursor"
        )

    telemetry_fields = {
        "parent_active_seconds",
        "helper_active_seconds_sum",
        "team_active_seconds_sum",
        "active_wall_union_seconds",
        "telemetry_complete",
        "heavy_seconds_complete",
        "excluded_from_measured_crack_per_hour",
        "no_imputation",
        "uncovered_seconds",
        "telemetry_sha256",
        "active_interval_log_sha256",
        "matrix_sha256",
    }
    telemetry = _closed(
        context.get("telemetry"),
        allowed=telemetry_fields,
        required=telemetry_fields,
        label=f"{label}.telemetry",
    )
    normalized_telemetry = {
        "parent_active_seconds": _number(
            telemetry.get("parent_active_seconds"),
            f"{label}.telemetry.parent_active_seconds",
            minimum=0.001,
        ),
        "helper_active_seconds_sum": _number(
            telemetry.get("helper_active_seconds_sum"),
            f"{label}.telemetry.helper_active_seconds_sum",
        ),
        "team_active_seconds_sum": _number(
            telemetry.get("team_active_seconds_sum"),
            f"{label}.telemetry.team_active_seconds_sum",
            minimum=0.001,
        ),
        "active_wall_union_seconds": _number(
            telemetry.get("active_wall_union_seconds"),
            f"{label}.telemetry.active_wall_union_seconds",
            minimum=0.001,
        ),
        "telemetry_complete": _bool(
            telemetry.get("telemetry_complete"),
            f"{label}.telemetry.telemetry_complete",
            False,
        ),
        "heavy_seconds_complete": _bool(
            telemetry.get("heavy_seconds_complete"),
            f"{label}.telemetry.heavy_seconds_complete",
            False,
        ),
        "excluded_from_measured_crack_per_hour": _bool(
            telemetry.get("excluded_from_measured_crack_per_hour"),
            f"{label}.telemetry.excluded_from_measured_crack_per_hour",
            True,
        ),
        "no_imputation": _bool(
            telemetry.get("no_imputation"),
            f"{label}.telemetry.no_imputation",
            True,
        ),
        "uncovered_seconds": _number(
            telemetry.get("uncovered_seconds"),
            f"{label}.telemetry.uncovered_seconds",
            minimum=0.001,
        ),
    }
    for field in (
        "telemetry_sha256",
        "active_interval_log_sha256",
        "matrix_sha256",
    ):
        normalized_telemetry[field] = _sha256(
            telemetry.get(field), f"{label}.telemetry.{field}"
        )
    if not math.isclose(
        normalized_telemetry["team_active_seconds_sum"],
        normalized_telemetry["parent_active_seconds"]
        + normalized_telemetry["helper_active_seconds_sum"],
        rel_tol=0.0,
        abs_tol=0.001,
    ):
        raise SavedFprStackPoolInputError(
            f"{label}.telemetry team total does not equal parent plus helper"
        )

    exact_fields = {
        "source_sha256",
        "function_hunk_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "compile_attestation_sha256",
        "candidate_record_sha256",
        "target_bytes",
        "candidate_bytes",
        "physical_relocations",
        "strict_percent",
        "data_percent",
        "protected_sibling_losses",
    }
    exact = _closed(
        context.get("exact_result"),
        allowed=exact_fields,
        required=exact_fields,
        label=f"{label}.exact_result",
    )
    normalized_exact = {
        "source_sha256": _sha256(
            exact.get("source_sha256"), f"{label}.exact_result.source_sha256"
        ),
        "function_hunk_sha256": _sha256(
            exact.get("function_hunk_sha256"),
            f"{label}.exact_result.function_hunk_sha256",
        ),
        "object_sha256": _sha256(
            exact.get("object_sha256"), f"{label}.exact_result.object_sha256"
        ),
        "strict_report_sha256": _sha256(
            exact.get("strict_report_sha256"),
            f"{label}.exact_result.strict_report_sha256",
        ),
        "data_report_sha256": _sha256(
            exact.get("data_report_sha256"),
            f"{label}.exact_result.data_report_sha256",
        ),
        "compile_attestation_sha256": _sha256(
            exact.get("compile_attestation_sha256"),
            f"{label}.exact_result.compile_attestation_sha256",
        ),
        "candidate_record_sha256": _sha256(
            exact.get("candidate_record_sha256"),
            f"{label}.exact_result.candidate_record_sha256",
        ),
        "target_bytes": _uint(
            exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=32
        ),
        "candidate_bytes": _uint(
            exact.get("candidate_bytes"),
            f"{label}.exact_result.candidate_bytes",
            minimum=32,
        ),
        "physical_relocations": _uint(
            exact.get("physical_relocations"),
            f"{label}.exact_result.physical_relocations",
            minimum=1,
        ),
        "strict_percent": _number(
            exact.get("strict_percent"), f"{label}.exact_result.strict_percent"
        ),
        "data_percent": _number(
            exact.get("data_percent"), f"{label}.exact_result.data_percent"
        ),
        "protected_sibling_losses": _uint(
            exact.get("protected_sibling_losses"),
            f"{label}.exact_result.protected_sibling_losses",
        ),
    }
    if (
        normalized_exact["target_bytes"] != normalized_exact["candidate_bytes"]
        or normalized_exact["target_bytes"] != composition["target_bytes"]
        or normalized_exact["physical_relocations"]
        != composition["target_physical_relocations"]
        or normalized_exact["strict_percent"] != 100.0
        or normalized_exact["data_percent"] != 100.0
        or normalized_exact["protected_sibling_losses"] != 0
    ):
        raise SavedFprStackPoolInputError(
            f"{label}.exact_result must close size/strict/data/relocations/siblings"
        )

    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": _sha256(
            context.get("report_artifact_sha256"),
            f"{label}.report_artifact_sha256",
        ),
        "function": function,
        "composition_stage": composition,
        "pool_handoff_stage": pool_handoff,
        "trace": normalized_trace,
        "donor": normalized_donor,
        "interaction": normalized_interaction,
        "negative_controls": controls,
        "telemetry": normalized_telemetry,
        "exact_result": normalized_exact,
        "authority_advanced": _bool(
            context.get("authority_advanced"), f"{label}.authority_advanced", False
        ),
    }


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _mismatch_rows(target: Sequence[Any], candidate: Sequence[Any]) -> list[int]:
    return [
        index
        for index, (left, right) in enumerate(
            causal_reducer._paired_records(target, candidate)
        )
        if (left is not None and causal_reducer._is_diff_kind(left.diff_kind))
        or (right is not None and causal_reducer._is_diff_kind(right.diff_kind))
    ]


def _formatted(instructions: Sequence[Any]) -> list[str]:
    return [item.formatted.lower() for item in instructions if item.has_instruction]


def evaluate(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {
            "matched": False,
            "reason": "no authenticated saved-FPR stack/pool context was supplied",
        }
    if pair.name != context["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    composition = context["composition_stage"]
    pool = context["pool_handoff_stage"]
    if objdiff_canonical_sha256 == composition["objdiff_canonical_sha256"]:
        stage = "owner_composition"
        sealed = composition
    elif objdiff_canonical_sha256 == pool["objdiff_canonical_sha256"]:
        stage = "typed_pool_handoff"
        sealed = pool
    else:
        return {
            "matched": False,
            "reason": "the context is bound to other composition and pool reports",
        }
    target_size = (
        causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    )
    candidate_size = (
        causal_reducer._parse_number(pair.candidate.get("size"))
        if pair.candidate
        else None
    )
    observed = (
        target_size,
        candidate_size,
        _physical_relocations(target),
        _physical_relocations(candidate),
    )
    expected = (
        sealed["target_bytes"],
        sealed["candidate_bytes"],
        sealed["target_physical_relocations"],
        sealed["candidate_physical_relocations"],
    )
    if observed != expected:
        return {
            "matched": False,
            "reason": "the size or physical relocation signature drifted",
            "evidence": {"observed": list(observed), "sealed": list(expected)},
        }
    residual_rows = _mismatch_rows(target, candidate)
    if stage == "owner_composition":
        store = composition["candidate_only_stack_store"].lower()
        target_text = _formatted(target)
        candidate_text = _formatted(candidate)
        if (
            len(residual_rows) < composition["minimum_residual_rows"]
            or store not in candidate_text
            or store in target_text
        ):
            return {
                "matched": False,
                "reason": "the candidate-only stack owner or its sealed cascade drifted",
                "evidence": {
                    "residual_row_count": len(residual_rows),
                    "candidate_only_store": store,
                },
            }
        cell = {
            "order": 1,
            "kind": "saved_fpr_same_tu_donor_composition",
            "priority_cell": context["interaction"]["priority_cell"],
            "axes": context["interaction"]["axes"],
            "rank_within_top": context["interaction"]["rank_within_top"],
            "compile_as_one_cell": True,
            "requires_repeat_trace": False,
        }
        return {
            "matched": True,
            "reason": "a candidate-only stack owner and saved-FPR cascade are closed by one exact-same-TU donor interaction",
            "confidence": 0.995,
            "source_class": "saved_fpr_stack_owner_same_tu_donor_composition",
            "recommendation": "Compile only the sealed reuse_value x distinct_distance2 x exact_donor_extended cell, then rerun the physical reducer before any further source experiment.",
            "evidence": {
                "stage": stage,
                "precursor": composition,
                "trace": context["trace"],
                "donor": context["donor"],
                "recommended_cells": [cell],
                "negative_controls": context["negative_controls"],
                "suppressed_axes": [
                    "distinct_distance_only",
                    "broad_historical_prefix",
                    "broad_fpr_grouping",
                    "direct_trig_consumption",
                    "broad_declaration_permutations",
                    "repeat_tracer",
                    "dead_or_fake_local",
                    "padding",
                    "register_shaping",
                    "automatic_retention_or_promotion",
                ],
                "telemetry": context["telemetry"],
                "exact_result": context["exact_result"],
                "authority_advanced": False,
            },
        }

    if residual_rows != pool["residual_rows"]:
        return {
            "matched": False,
            "reason": "the literal-only pool residual rows drifted",
            "evidence": {
                "observed_rows": residual_rows,
                "sealed_rows": pool["residual_rows"],
            },
        }
    paired = causal_reducer._paired_records(target, candidate)
    for row in residual_rows:
        left, right = paired[row]
        if (
            left is None
            or right is None
            or not left.has_instruction
            or not right.has_instruction
            or left.mnemonic != right.mnemonic
            or pool["target_owner"].lower() not in left.formatted.lower()
            or pool["candidate_owner"].lower() not in right.formatted.lower()
        ):
            return {
                "matched": False,
                "reason": "a typed-pool residual no longer carries its sealed owner substitution",
                "evidence": {"row": row},
            }
    return {
        "matched": True,
        "reason": "code topology is closed and exactly three value-only pool rows remain",
        "confidence": 0.999,
        "source_class": "typed_pool_value_handoff_after_saved_fpr_closure",
        "recommendation": "Stop owner/lifetime experimentation and hand the three sealed rows directly to typed-pool decoding as one semantic 192.0f-to-32.0f correction batch.",
        "evidence": {
            "stage": stage,
            "pool_handoff": pool,
            "recommended_batches": [
                {
                    "order": 1,
                    "kind": "typed_pool_semantic_value_correction",
                    "rows": pool["residual_rows"],
                    "target_value": pool["target_value"],
                    "candidate_value": pool["candidate_value"],
                    "single_semantic_batch": True,
                }
            ],
            "suppressed_axes": [
                "more_saved_fpr_owner_changes",
                "declaration_permutations",
                "repeat_tracer",
                "literal_owner_shaping",
                "automatic_retention_or_promotion",
            ],
            "telemetry": context["telemetry"],
            "exact_result": context["exact_result"],
            "authority_advanced": False,
        },
    }
