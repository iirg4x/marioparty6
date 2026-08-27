#!/usr/bin/env python3
"""Fail-closed direct scalar-return to fabs/comparison diagnosis."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "direct_scalar_fabs_consumer_context/v1"
RULE_ID = "direct_scalar_fabs_consumer"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FPR_RE = re.compile(r"f(?:[0-9]|[12][0-9]|3[01])")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_CONTROL_RESULTS = {
    "block_or_function_scope": "object_identical",
    "comparison_commutation": "regressed_topology",
    "fabsf_or_prototype_guess": "regressed_inadmissible",
}
_RESIDUAL_MNEMONICS = ("fabs", "fcmpo", "fabs", "fcmpo", "fabs", "fmr", "fcmpo")
_DONOR_SHAPES = {
    "direct_scalar_return_to_fabs",
    "direct_fabs_to_immediate_comparison",
    "direct_scalar_to_typed_consumer",
}


class DirectScalarFabsInputError(ValueError):
    """The supplied evidence cannot safely support direct composition."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise DirectScalarFabsInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise DirectScalarFabsInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise DirectScalarFabsInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise DirectScalarFabsInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise DirectScalarFabsInputError(f"{label} must be a C identifier")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise DirectScalarFabsInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise DirectScalarFabsInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise DirectScalarFabsInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, positive: bool = False) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise DirectScalarFabsInputError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or (positive and result <= 0.0) or result < 0.0:
        raise DirectScalarFabsInputError(f"{label} has an invalid numeric value")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise DirectScalarFabsInputError(
            f"{label} must be {str(expected).lower()}"
        )
    return expected


def _fpr(value: Any, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if _FPR_RE.fullmatch(result) is None:
        raise DirectScalarFabsInputError(f"{label} must be an FPR")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "direct scalar-fabs consumer context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "precursor",
        "call_chain",
        "donors",
        "trace",
        "negative_controls",
        "telemetry",
        "exact_result",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise DirectScalarFabsInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    report_artifact_sha256 = _sha256(
        context.get("report_artifact_sha256"), f"{label}.report_artifact_sha256"
    )
    _bool(context.get("authority_advanced"), f"{label}.authority_advanced", False)

    precursor_fields = {
        "function",
        "candidate_id",
        "objdiff_canonical_sha256",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "target_bytes",
        "candidate_bytes",
        "target_frame",
        "candidate_frame",
        "match_percent",
        "target_physical_relocations",
        "candidate_physical_relocations",
        "residual_pairs",
        "operation_order_exact",
        "cfg_calls_exact",
        "data_exact",
        "stack_homes_exact",
        "protected_siblings_preserved",
    }
    precursor = _closed(
        context.get("precursor"),
        allowed=precursor_fields,
        required=precursor_fields,
        label=f"{label}.precursor",
    )
    target_bytes = _uint(
        precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=32
    )
    candidate_bytes = _uint(
        precursor.get("candidate_bytes"),
        f"{label}.precursor.candidate_bytes",
        minimum=32,
    )
    target_frame = _uint(
        precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16
    )
    candidate_frame = _uint(
        precursor.get("candidate_frame"),
        f"{label}.precursor.candidate_frame",
        minimum=16,
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
    match_percent = _number(
        precursor.get("match_percent"), f"{label}.precursor.match_percent", positive=True
    )
    if (
        target_bytes != candidate_bytes
        or target_frame != candidate_frame
        or target_relocations != candidate_relocations
        or match_percent >= 100.0
    ):
        raise DirectScalarFabsInputError(
            f"{label}.precursor must be nonexact with exact size/frame/relocations"
        )
    for field in (
        "operation_order_exact",
        "cfg_calls_exact",
        "data_exact",
        "stack_homes_exact",
        "protected_siblings_preserved",
    ):
        _bool(precursor.get(field), f"{label}.precursor.{field}", True)
    raw_pairs = precursor.get("residual_pairs")
    if not isinstance(raw_pairs, list) or len(raw_pairs) != 7:
        raise DirectScalarFabsInputError(
            f"{label}.precursor.residual_pairs must contain seven rows"
        )
    residual_pairs: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_pairs):
        pair = _closed(
            raw,
            allowed={"row", "target", "candidate"},
            required={"row", "target", "candidate"},
            label=f"{label}.precursor.residual_pairs[{index}]",
        )
        target_form = _text(
            pair.get("target"), f"{label}.precursor.residual_pairs[{index}].target"
        )
        candidate_form = _text(
            pair.get("candidate"),
            f"{label}.precursor.residual_pairs[{index}].candidate",
        )
        target_mnemonic = target_form.split()[0].lower()
        candidate_mnemonic = candidate_form.split()[0].lower()
        if (
            target_mnemonic != candidate_mnemonic
            or target_mnemonic != _RESIDUAL_MNEMONICS[index]
        ):
            raise DirectScalarFabsInputError(
                f"{label}.precursor residual opcode sequence is not the sealed fabs cycle"
            )
        residual_pairs.append(
            {
                "row": _uint(
                    pair.get("row"),
                    f"{label}.precursor.residual_pairs[{index}].row",
                    maximum=1 << 20,
                ),
                "target": target_form,
                "candidate": candidate_form,
            }
        )
    rows = [item["row"] for item in residual_pairs]
    if rows != sorted(set(rows)):
        raise DirectScalarFabsInputError(
            f"{label}.precursor residual rows must be sorted and unique"
        )
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _text(
            precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128
        ),
        "objdiff_canonical_sha256": _sha256(
            precursor.get("objdiff_canonical_sha256"),
            f"{label}.precursor.objdiff_canonical_sha256",
        ),
        "source_sha256": _sha256(
            precursor.get("source_sha256"), f"{label}.precursor.source_sha256"
        ),
        "object_sha256": _sha256(
            precursor.get("object_sha256"), f"{label}.precursor.object_sha256"
        ),
        "strict_report_sha256": _sha256(
            precursor.get("strict_report_sha256"),
            f"{label}.precursor.strict_report_sha256",
        ),
        "data_report_sha256": _sha256(
            precursor.get("data_report_sha256"),
            f"{label}.precursor.data_report_sha256",
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocations,
        "residual_pairs": residual_pairs,
        "operation_order_exact": True,
        "cfg_calls_exact": True,
        "data_exact": True,
        "stack_homes_exact": True,
        "protected_siblings_preserved": True,
    }

    chain_fields = {
        "call_symbol",
        "return_register",
        "wrapped_return_register",
        "target_abs_register",
        "candidate_abs_register",
        "target_compare_register",
        "candidate_compare_register",
        "call_index",
        "return_bind_index",
        "fabs_index",
        "bridge_index",
        "compare_index",
        "immediate_consumer_count",
        "consumer_kind",
        "source_template",
    }
    chain = _closed(
        context.get("call_chain"),
        allowed=chain_fields,
        required=chain_fields,
        label=f"{label}.call_chain",
    )
    normalized_chain = {
        "call_symbol": _identifier(chain.get("call_symbol"), f"{label}.call_chain.call_symbol"),
        "return_register": _fpr(
            chain.get("return_register"), f"{label}.call_chain.return_register"
        ),
        "wrapped_return_register": _fpr(
            chain.get("wrapped_return_register"),
            f"{label}.call_chain.wrapped_return_register",
        ),
        "target_abs_register": _fpr(
            chain.get("target_abs_register"), f"{label}.call_chain.target_abs_register"
        ),
        "candidate_abs_register": _fpr(
            chain.get("candidate_abs_register"),
            f"{label}.call_chain.candidate_abs_register",
        ),
        "target_compare_register": _fpr(
            chain.get("target_compare_register"),
            f"{label}.call_chain.target_compare_register",
        ),
        "candidate_compare_register": _fpr(
            chain.get("candidate_compare_register"),
            f"{label}.call_chain.candidate_compare_register",
        ),
        "call_index": _uint(chain.get("call_index"), f"{label}.call_chain.call_index"),
        "return_bind_index": _uint(
            chain.get("return_bind_index"), f"{label}.call_chain.return_bind_index"
        ),
        "fabs_index": _uint(chain.get("fabs_index"), f"{label}.call_chain.fabs_index"),
        "bridge_index": _uint(
            chain.get("bridge_index"), f"{label}.call_chain.bridge_index"
        ),
        "compare_index": _uint(
            chain.get("compare_index"), f"{label}.call_chain.compare_index"
        ),
        "immediate_consumer_count": _uint(
            chain.get("immediate_consumer_count"),
            f"{label}.call_chain.immediate_consumer_count",
            minimum=1,
            maximum=8,
        ),
        "consumer_kind": _text(
            chain.get("consumer_kind"), f"{label}.call_chain.consumer_kind", limit=64
        ),
        "source_template": _text(
            chain.get("source_template"), f"{label}.call_chain.source_template", limit=256
        ),
    }
    if (
        normalized_chain["return_register"] != "f1"
        or normalized_chain["consumer_kind"] != "fabs_then_immediate_compare"
        or normalized_chain["immediate_consumer_count"] != 1
        or normalized_chain["return_bind_index"] != normalized_chain["call_index"] + 1
        or normalized_chain["fabs_index"] != normalized_chain["return_bind_index"] + 1
        or normalized_chain["bridge_index"] != normalized_chain["fabs_index"] + 1
        or normalized_chain["compare_index"] != normalized_chain["bridge_index"] + 2
        or normalized_chain["fabs_index"] != rows[4]
        or normalized_chain["bridge_index"] != rows[5]
        or normalized_chain["compare_index"] != rows[6]
    ):
        raise DirectScalarFabsInputError(
            f"{label}.call_chain is not the sealed call/bind/fabs/bridge/compare sequence"
        )

    raw_donors = context.get("donors")
    if not isinstance(raw_donors, list) or not 1 <= len(raw_donors) <= 8:
        raise DirectScalarFabsInputError(f"{label}.donors must contain one through eight donors")
    donors: list[dict[str, Any]] = []
    donor_names: set[str] = set()
    for index, raw in enumerate(raw_donors):
        donor = _closed(
            raw,
            allowed={
                "function",
                "same_translation_unit",
                "strict_exact",
                "source_location",
                "source_shape",
                "strict_report_sha256",
            },
            required={
                "function",
                "same_translation_unit",
                "strict_exact",
                "source_location",
                "source_shape",
                "strict_report_sha256",
            },
            label=f"{label}.donors[{index}]",
        )
        name = _identifier(donor.get("function"), f"{label}.donors[{index}].function")
        if name in donor_names:
            raise DirectScalarFabsInputError(f"{label}.donors must be unique")
        donor_names.add(name)
        _bool(
            donor.get("same_translation_unit"),
            f"{label}.donors[{index}].same_translation_unit",
            True,
        )
        _bool(donor.get("strict_exact"), f"{label}.donors[{index}].strict_exact", True)
        source_shape = _text(
            donor.get("source_shape"), f"{label}.donors[{index}].source_shape", limit=64
        )
        if source_shape not in _DONOR_SHAPES:
            raise DirectScalarFabsInputError(f"{label}.donors[{index}] has unsupported shape")
        donors.append(
            {
                "function": name,
                "same_translation_unit": True,
                "strict_exact": True,
                "source_location": _text(
                    donor.get("source_location"),
                    f"{label}.donors[{index}].source_location",
                    limit=192,
                ),
                "source_shape": source_shape,
                "strict_report_sha256": _sha256(
                    donor.get("strict_report_sha256"),
                    f"{label}.donors[{index}].strict_report_sha256",
                ),
            }
        )
    if not any(item["source_shape"] == "direct_scalar_return_to_fabs" for item in donors):
        raise DirectScalarFabsInputError(
            f"{label}.donors must include an exact direct scalar-return-to-fabs precedent"
        )
    donors.sort(key=lambda item: item["function"])

    trace_fields = {
        "status",
        "same_session",
        "authority_advanced",
        "event_count",
        "ownership_unknown_present",
        "regalloc_complete",
        "used_for_recommendation",
        "repeat_allowed",
        "envelope_file_sha256",
        "envelope_sha256",
        "trust_root_sha256",
    }
    trace = _closed(
        context.get("trace"), allowed=trace_fields, required=trace_fields, label=f"{label}.trace"
    )
    if _text(trace.get("status"), f"{label}.trace.status") != "DIAGNOSTIC_UNKNOWN":
        raise DirectScalarFabsInputError(f"{label}.trace.status must preserve UNKNOWN")
    for field, expected in (
        ("same_session", True),
        ("authority_advanced", False),
        ("ownership_unknown_present", True),
        ("regalloc_complete", False),
        ("used_for_recommendation", False),
        ("repeat_allowed", False),
    ):
        _bool(trace.get(field), f"{label}.trace.{field}", expected)
    normalized_trace = {
        "status": "DIAGNOSTIC_UNKNOWN",
        "same_session": True,
        "authority_advanced": False,
        "event_count": _uint(
            trace.get("event_count"), f"{label}.trace.event_count", minimum=1
        ),
        "ownership_unknown_present": True,
        "regalloc_complete": False,
        "used_for_recommendation": False,
        "repeat_allowed": False,
        "envelope_file_sha256": _sha256(
            trace.get("envelope_file_sha256"), f"{label}.trace.envelope_file_sha256"
        ),
        "envelope_sha256": _sha256(
            trace.get("envelope_sha256"), f"{label}.trace.envelope_sha256"
        ),
        "trust_root_sha256": _sha256(
            trace.get("trust_root_sha256"), f"{label}.trace.trust_root_sha256"
        ),
    }

    raw_controls = context.get("negative_controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != len(_CONTROL_RESULTS):
        raise DirectScalarFabsInputError(f"{label}.negative_controls must contain three controls")
    controls: list[dict[str, Any]] = []
    seen_controls: set[str] = set()
    for index, raw in enumerate(raw_controls):
        control = _closed(
            raw,
            allowed={"candidate_id", "axis", "result", "measured", "evidence_sha256"},
            required={"candidate_id", "axis", "result", "measured", "evidence_sha256"},
            label=f"{label}.negative_controls[{index}]",
        )
        axis = _text(control.get("axis"), f"{label}.negative_controls[{index}].axis", limit=64)
        result = _text(
            control.get("result"), f"{label}.negative_controls[{index}].result", limit=64
        )
        if axis in seen_controls or _CONTROL_RESULTS.get(axis) != result:
            raise DirectScalarFabsInputError(
                f"{label}.negative_controls do not match the sealed control matrix"
            )
        seen_controls.add(axis)
        _bool(control.get("measured"), f"{label}.negative_controls[{index}].measured", True)
        controls.append(
            {
                "candidate_id": _text(
                    control.get("candidate_id"),
                    f"{label}.negative_controls[{index}].candidate_id",
                    limit=128,
                ),
                "axis": axis,
                "result": result,
                "measured": True,
                "evidence_sha256": _sha256(
                    control.get("evidence_sha256"),
                    f"{label}.negative_controls[{index}].evidence_sha256",
                ),
            }
        )
    if seen_controls != set(_CONTROL_RESULTS):
        raise DirectScalarFabsInputError(f"{label}.negative_controls are incomplete")
    controls.sort(key=lambda item: item["axis"])

    telemetry_fields = {
        "parent_active_seconds",
        "helper_active_seconds_sum",
        "team_active_seconds_sum",
        "active_wall_union_seconds",
        "heavy_seconds",
        "candidate_count",
        "tracer_runs",
        "donor_searches",
        "telemetry_complete",
        "excluded_from_measured_crack_per_hour",
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
    parent = _number(
        telemetry.get("parent_active_seconds"),
        f"{label}.telemetry.parent_active_seconds",
        positive=True,
    )
    helper = _number(
        telemetry.get("helper_active_seconds_sum"),
        f"{label}.telemetry.helper_active_seconds_sum",
    )
    team = _number(
        telemetry.get("team_active_seconds_sum"),
        f"{label}.telemetry.team_active_seconds_sum",
        positive=True,
    )
    union = _number(
        telemetry.get("active_wall_union_seconds"),
        f"{label}.telemetry.active_wall_union_seconds",
        positive=True,
    )
    if not math.isclose(team, parent + helper, rel_tol=0.0, abs_tol=1e-6) or union > team + 1e-6:
        raise DirectScalarFabsInputError(f"{label}.telemetry active-time totals drift")
    telemetry_complete = telemetry.get("telemetry_complete")
    excluded = telemetry.get("excluded_from_measured_crack_per_hour")
    if telemetry_complete is not False or excluded is not True:
        raise DirectScalarFabsInputError(
            f"{label}.telemetry must preserve incomplete/excluded crack-hour status"
        )
    _bool(telemetry.get("no_imputation"), f"{label}.telemetry.no_imputation", True)
    normalized_telemetry = {
        "parent_active_seconds": parent,
        "helper_active_seconds_sum": helper,
        "team_active_seconds_sum": team,
        "active_wall_union_seconds": union,
        "heavy_seconds": _number(
            telemetry.get("heavy_seconds"), f"{label}.telemetry.heavy_seconds"
        ),
        "candidate_count": _uint(
            telemetry.get("candidate_count"), f"{label}.telemetry.candidate_count", minimum=1
        ),
        "tracer_runs": _uint(
            telemetry.get("tracer_runs"), f"{label}.telemetry.tracer_runs"
        ),
        "donor_searches": _uint(
            telemetry.get("donor_searches"), f"{label}.telemetry.donor_searches"
        ),
        "telemetry_complete": False,
        "excluded_from_measured_crack_per_hour": True,
        "no_imputation": True,
        "telemetry_sha256": _sha256(
            telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"
        ),
        "active_interval_log_sha256": _sha256(
            telemetry.get("active_interval_log_sha256"),
            f"{label}.telemetry.active_interval_log_sha256",
        ),
    }

    exact_fields = {
        "source_sha256",
        "function_sha256",
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
    exact_target = _uint(
        exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=32
    )
    exact_candidate = _uint(
        exact.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=32
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
        or _number(exact.get("strict_percent"), f"{label}.exact_result.strict_percent") != 100.0
        or _number(exact.get("data_percent"), f"{label}.exact_result.data_percent") != 100.0
        or _uint(
            exact.get("protected_sibling_losses"),
            f"{label}.exact_result.protected_sibling_losses",
        )
        != 0
    ):
        raise DirectScalarFabsInputError(
            f"{label}.exact_result must close size/relocations/strict/data without sibling loss"
        )
    normalized_exact = {
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
        "strict_percent": 100.0,
        "data_percent": 100.0,
        "protected_sibling_losses": 0,
    }
    for field in (
        "source_sha256",
        "function_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "compile_attestation_sha256",
        "candidate_record_sha256",
    ):
        normalized_exact[field] = _sha256(
            exact.get(field), f"{label}.exact_result.{field}"
        )

    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": report_artifact_sha256,
        "precursor": normalized_precursor,
        "call_chain": normalized_chain,
        "donors": donors,
        "trace": normalized_trace,
        "negative_controls": controls,
        "telemetry": normalized_telemetry,
        "exact_result": normalized_exact,
        "authority_advanced": False,
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
        for index, (left, right) in enumerate(
            causal_reducer._paired_records(target, candidate)
        )
        if causal_reducer._instruction_mismatch(left, right)
    ]


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _call_indices(instructions: Sequence[Any], symbol: str) -> list[int]:
    return [
        index
        for index, instruction in enumerate(instructions)
        if instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and re.search(rf"\b{re.escape(symbol)}\b", instruction.formatted) is not None
    ]


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
            "reason": "no authenticated direct scalar-fabs consumer context was supplied",
        }
    precursor = context["precursor"]
    if precursor["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {
            "matched": False,
            "reason": "the direct scalar-fabs context is bound to another objdiff report",
        }
    if pair.name != precursor["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = (
        causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    )
    observed_signature = (
        target_size,
        candidate_size,
        _frame_size(target),
        _frame_size(candidate),
        _physical_relocations(target),
        _physical_relocations(candidate),
    )
    sealed_signature = (
        precursor["target_bytes"],
        precursor["candidate_bytes"],
        precursor["target_frame"],
        precursor["candidate_frame"],
        precursor["physical_relocations"],
        precursor["physical_relocations"],
    )
    if observed_signature != sealed_signature:
        return {
            "matched": False,
            "reason": "the exact size/frame/physical-relocation signature drifted",
            "evidence": {"observed": list(observed_signature), "sealed": list(sealed_signature)},
        }
    observed_rows = _mismatch_rows(target, candidate)
    sealed_rows = [item["row"] for item in precursor["residual_pairs"]]
    if observed_rows != sealed_rows:
        return {
            "matched": False,
            "reason": "the residual rows differ from the sealed seven-row FPR cascade",
            "evidence": {"observed_rows": observed_rows, "sealed_rows": sealed_rows},
        }
    paired = causal_reducer._paired_records(target, candidate)
    for item in precursor["residual_pairs"]:
        left, right = paired[item["row"]]
        if (
            left is None
            or right is None
            or not left.has_instruction
            or not right.has_instruction
            or left.formatted != item["target"]
            or right.formatted != item["candidate"]
            or left.mnemonic != right.mnemonic
        ):
            return {
                "matched": False,
                "reason": "a residual row no longer carries its sealed register-only form",
                "evidence": {"row": item["row"]},
            }

    chain = context["call_chain"]
    target_calls = _call_indices(target, chain["call_symbol"])
    candidate_calls = _call_indices(candidate, chain["call_symbol"])
    if target_calls != [chain["call_index"]] or candidate_calls != [chain["call_index"]]:
        return {
            "matched": False,
            "reason": "the scalar helper call is missing, duplicated, or moved",
            "evidence": {"target_calls": target_calls, "candidate_calls": candidate_calls},
        }
    bind = chain["return_bind_index"]
    expected_bind = f"fmr {chain['wrapped_return_register']}, {chain['return_register']}"
    if target[bind].formatted != expected_bind or candidate[bind].formatted != expected_bind:
        return {
            "matched": False,
            "reason": "the scalar return bind no longer matches the sealed chain",
        }
    fabs_row = chain["fabs_index"]
    bridge_row = chain["bridge_index"]
    compare_row = chain["compare_index"]
    expected_forms = (
        f"fabs {chain['target_abs_register']}, {chain['wrapped_return_register']}",
        f"fabs {chain['candidate_abs_register']}, {chain['wrapped_return_register']}",
        f"fmr {chain['target_compare_register']}, {chain['target_abs_register']}",
        f"fmr {chain['candidate_compare_register']}, {chain['candidate_abs_register']}",
    )
    if (
        target[fabs_row].formatted,
        candidate[fabs_row].formatted,
        target[bridge_row].formatted,
        candidate[bridge_row].formatted,
    ) != expected_forms:
        return {
            "matched": False,
            "reason": "the fabs/bridge ownership chain drifted",
        }
    if (
        target[compare_row].mnemonic != "fcmpo"
        or candidate[compare_row].mnemonic != "fcmpo"
        or chain["target_compare_register"] not in target[compare_row].formatted
        or chain["candidate_compare_register"] not in candidate[compare_row].formatted
    ):
        return {
            "matched": False,
            "reason": "the immediate comparison consumer drifted",
        }

    recommended_cell = {
        "order": 1,
        "kind": "direct_scalar_return_fabs_composition",
        "call_symbol": chain["call_symbol"],
        "consumer": "fabs_then_immediate_compare",
        "source_template": chain["source_template"],
        "compile_as_one_cell": True,
        "requires_trace": False,
        "preserve_all_other_source_axes": True,
    }
    evidence = {
        "precursor": precursor,
        "call_chain": chain,
        "exact_same_tu_donors": context["donors"],
        "trace": context["trace"],
        "trace_used_for_ownership": False,
        "recommended_cells": [recommended_cell],
        "suppressed_axes": [
            "block_or_function_scope_local_permutations",
            "comparison_commutation",
            "fabsf_or_prototype_guess",
            "repeat_tracer_capture",
            "global_declaration_permutations",
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "automatic_retention_or_promotion",
        ],
        "negative_controls": context["negative_controls"],
        "telemetry": context["telemetry"],
        "exact_result": context["exact_result"],
        "report_artifact_sha256": context["report_artifact_sha256"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "exact static topology plus an exact same-TU scalar-return/fabs donor closes one direct one-consumer source boundary without using UNKNOWN trace ownership",
        "confidence": 0.997,
        "source_class": "direct_scalar_return_to_fabs_immediate_comparison",
        "recommendation": (
            f"Compile only the direct {chain['call_symbol']} return-to-fabs comparison cell; "
            "do not retry scope, comparison, prototype, or tracing axes."
        ),
        "evidence": evidence,
    }
