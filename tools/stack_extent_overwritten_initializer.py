#!/usr/bin/env python3
"""Fail-closed stack-extent diagnosis for an overwritten initializer seam."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "stack_extent_overwritten_initializer_context/v1"
RULE_ID = "stack_extent_overwritten_initializer"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_GPR_RE = re.compile(r"r(?:[0-9]|[12][0-9]|3[01])")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_LI_RE = re.compile(
    r"^\s*li\s+(?P<register>r(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<value>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*$",
    re.IGNORECASE,
)
_DFORM_RE = re.compile(
    r"^\s*(?P<opcode>[A-Za-z][A-Za-z0-9_.]*)\s+"
    r"(?P<register>[rf](?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*"
    r"\(\s*(?P<base>r(?:[0-9]|[12][0-9]|3[01]))\s*\)"
    r"(?:\s*,.*)?\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "cfg_calls_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "stack_residue_authenticated",
    "target_home_chronology_authenticated",
    "overwritten_slot_one_write_zero_read",
    "duplicate_same_home_initializer_authenticated",
    "negative_controls_measured",
    "protected_siblings_preserved",
    "exact_result_verified",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "stack_residue_receipt_sha256",
    "target_chronology_receipt_sha256",
    "precursor_source_sha256",
    "precursor_object_sha256",
    "precursor_record_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "report_artifact_sha256",
)
_CONTROL_CLASSES = {
    "max_vertex_overwrite": "regressed",
    "self_chain": "regressed",
    "address_sizeof_visibility": "object_identical",
    "constant_bound_visibility": "object_identical",
    "two_word_array": "exact",
    "two_field_struct": "exact_same_object",
    "one_word_aggregate": "regressed",
    "particle_data_aliases": "regressed",
    "three_word_semantic_state": "regressed",
    "tracer_capture": "failed_closed_no_retry",
}
_LOAD_OPCODES = {
    "lbz",
    "lbzu",
    "lha",
    "lhz",
    "lwz",
    "lwzu",
    "lfs",
    "lfsu",
    "lfd",
    "lfdu",
    "lmw",
    "psq_l",
    "psq_lu",
}
_STORE_OPCODES = {
    "stb", "stbu", "sth", "sthu", "stw", "stwu", "stfs", "stfsu",
    "stfd", "stfdu", "stmw", "psq_st", "psq_stu",
}


class StackExtentInitializerInputError(ValueError):
    """The supplied evidence cannot support the rule safely."""


def _closed(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise StackExtentInitializerInputError(f"{label} must be a JSON object")
    fields = set(value)
    missing = required - fields
    extra = fields - allowed
    if missing or extra:
        raise StackExtentInitializerInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise StackExtentInitializerInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise StackExtentInitializerInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise StackExtentInitializerInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise StackExtentInitializerInputError(f"{label} must be a bounded owner path")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise StackExtentInitializerInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise StackExtentInitializerInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise StackExtentInitializerInputError(f"{label} must be from {minimum} through {maximum}")
    return value


def _positive_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise StackExtentInitializerInputError(f"{label} must be a positive finite number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise StackExtentInitializerInputError(f"{label} must be a positive finite number")
    return result


def _rows(value: Any, label: str) -> list[int]:
    if not isinstance(value, list) or not value:
        raise StackExtentInitializerInputError(f"{label} must be a non-empty row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) > 8:
        raise StackExtentInitializerInputError(f"{label} must be sorted, unique, and bounded")
    return rows


def _gpr(value: Any, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if _GPR_RE.fullmatch(result) is None:
        raise StackExtentInitializerInputError(f"{label} must be a GPR")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "stack-extent overwritten-initializer context"
    fields = {
        "schema",
        "proofs",
        "precursor",
        "stack_seam",
        "controls",
        "provenance_boundary",
        "telemetry",
        "exact_result",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise StackExtentInitializerInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES) | {"authority_advanced"}
    proofs = _closed(
        context.get("proofs"), allowed=proof_fields, required=proof_fields, label=f"{label}.proofs"
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise StackExtentInitializerInputError(f"{label}.proofs.{field} must be true")
        normalized_proofs[field] = True
    if proofs.get("authority_advanced") is not False:
        raise StackExtentInitializerInputError(f"{label}.proofs.authority_advanced must be false")
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
        "residual_rows",
    }
    precursor = _closed(
        context.get("precursor"),
        allowed=precursor_fields,
        required=precursor_fields,
        label=f"{label}.precursor",
    )
    target_bytes = _uint(precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=24)
    candidate_bytes = _uint(
        precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=24
    )
    target_frame = _uint(precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16)
    candidate_frame = _uint(
        precursor.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16
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
    match_percent = _positive_number(precursor.get("match_percent"), f"{label}.precursor.match_percent")
    residual_rows = _rows(precursor.get("residual_rows"), f"{label}.precursor.residual_rows")
    if target_bytes != candidate_bytes or target_frame != candidate_frame:
        raise StackExtentInitializerInputError(f"{label}.precursor size and frame must already be exact")
    if target_relocations != candidate_relocations:
        raise StackExtentInitializerInputError(f"{label}.precursor relocations must already be exact")
    if match_percent >= 100.0 or len(residual_rows) != 1:
        raise StackExtentInitializerInputError(f"{label}.precursor must contain one nonexact residual row")
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _text(precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocations,
        "residual_rows": residual_rows,
    }

    seam_fields = {
        "base_register",
        "value_register",
        "access_width",
        "selected_home_offset",
        "candidate_selected_home_offset",
        "overwritten_slot_offset",
        "adjacent_zero_offset",
        "negative_initializer",
        "zero_initializer",
        "missing_extent_bytes",
        "current_extent_bytes",
        "target_extent_bytes",
        "element_size",
        "current_capacity",
        "predicted_capacity",
        "overwritten_write_count",
        "overwritten_read_count",
    }
    seam = _closed(
        context.get("stack_seam"),
        allowed=seam_fields,
        required=seam_fields,
        label=f"{label}.stack_seam",
    )
    normalized_seam = {
        "base_register": _gpr(seam.get("base_register"), f"{label}.stack_seam.base_register"),
        "value_register": _gpr(seam.get("value_register"), f"{label}.stack_seam.value_register"),
        "access_width": _uint(seam.get("access_width"), f"{label}.stack_seam.access_width", minimum=1, maximum=16),
        "selected_home_offset": _uint(seam.get("selected_home_offset"), f"{label}.stack_seam.selected_home_offset"),
        "candidate_selected_home_offset": _uint(
            seam.get("candidate_selected_home_offset"), f"{label}.stack_seam.candidate_selected_home_offset"
        ),
        "overwritten_slot_offset": _uint(
            seam.get("overwritten_slot_offset"), f"{label}.stack_seam.overwritten_slot_offset"
        ),
        "adjacent_zero_offset": _uint(seam.get("adjacent_zero_offset"), f"{label}.stack_seam.adjacent_zero_offset"),
        "negative_initializer": seam.get("negative_initializer"),
        "zero_initializer": seam.get("zero_initializer"),
        "missing_extent_bytes": _uint(
            seam.get("missing_extent_bytes"), f"{label}.stack_seam.missing_extent_bytes", minimum=1
        ),
        "current_extent_bytes": _uint(
            seam.get("current_extent_bytes"), f"{label}.stack_seam.current_extent_bytes", minimum=1
        ),
        "target_extent_bytes": _uint(
            seam.get("target_extent_bytes"), f"{label}.stack_seam.target_extent_bytes", minimum=1
        ),
        "element_size": _uint(seam.get("element_size"), f"{label}.stack_seam.element_size", minimum=1),
        "current_capacity": _uint(
            seam.get("current_capacity"), f"{label}.stack_seam.current_capacity", minimum=1
        ),
        "predicted_capacity": _uint(
            seam.get("predicted_capacity"), f"{label}.stack_seam.predicted_capacity", minimum=2
        ),
        "overwritten_write_count": _uint(
            seam.get("overwritten_write_count"), f"{label}.stack_seam.overwritten_write_count"
        ),
        "overwritten_read_count": _uint(
            seam.get("overwritten_read_count"), f"{label}.stack_seam.overwritten_read_count"
        ),
    }
    if isinstance(normalized_seam["negative_initializer"], bool) or not isinstance(
        normalized_seam["negative_initializer"], int
    ):
        raise StackExtentInitializerInputError(f"{label}.stack_seam.negative_initializer must be integer")
    if isinstance(normalized_seam["zero_initializer"], bool) or not isinstance(
        normalized_seam["zero_initializer"], int
    ):
        raise StackExtentInitializerInputError(f"{label}.stack_seam.zero_initializer must be integer")
    if (
        normalized_seam["base_register"] != "r1"
        or normalized_seam["access_width"] != 4
        or normalized_seam["negative_initializer"] != -1
        or normalized_seam["zero_initializer"] != 0
        or normalized_seam["candidate_selected_home_offset"]
        - normalized_seam["selected_home_offset"]
        != normalized_seam["missing_extent_bytes"]
        or normalized_seam["selected_home_offset"] - normalized_seam["overwritten_slot_offset"]
        != normalized_seam["element_size"]
        or normalized_seam["overwritten_slot_offset"] - normalized_seam["adjacent_zero_offset"]
        != normalized_seam["element_size"]
        or normalized_seam["missing_extent_bytes"] != normalized_seam["element_size"]
        or normalized_seam["current_extent_bytes"] != normalized_seam["element_size"]
        or normalized_seam["target_extent_bytes"] != 2 * normalized_seam["element_size"]
        or normalized_seam["current_capacity"] != 1
        or normalized_seam["predicted_capacity"] != 2
        or normalized_seam["overwritten_write_count"] != 1
        or normalized_seam["overwritten_read_count"] != 0
    ):
        raise StackExtentInitializerInputError(f"{label}.stack_seam must be the sealed +4-byte/2-word seam")

    raw_controls = context.get("controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != len(_CONTROL_CLASSES):
        raise StackExtentInitializerInputError(f"{label}.controls must contain ten sealed controls")
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
            raise StackExtentInitializerInputError(f"{label}.controls drift from the sealed matrix")
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
        raise StackExtentInitializerInputError(f"{label}.controls must cover every sealed control")
    controls.sort(key=lambda item: item["kind"])

    provenance_fields = {
        "owner",
        "function",
        "source_provenance_authenticated",
        "residue_reconstruction_only",
        "unused_second_element",
        "general_dead_storage_waiver",
        "promotion_authority",
    }
    provenance = _closed(
        context.get("provenance_boundary"),
        allowed=provenance_fields,
        required=provenance_fields,
        label=f"{label}.provenance_boundary",
    )
    normalized_provenance = {
        "owner": _owner(provenance.get("owner"), f"{label}.provenance_boundary.owner"),
        "function": _identifier(provenance.get("function"), f"{label}.provenance_boundary.function"),
        "source_provenance_authenticated": provenance.get("source_provenance_authenticated"),
        "residue_reconstruction_only": provenance.get("residue_reconstruction_only"),
        "unused_second_element": provenance.get("unused_second_element"),
        "general_dead_storage_waiver": provenance.get("general_dead_storage_waiver"),
        "promotion_authority": provenance.get("promotion_authority"),
    }
    expected_provenance = {
        "source_provenance_authenticated": False,
        "residue_reconstruction_only": True,
        "unused_second_element": True,
        "general_dead_storage_waiver": False,
        "promotion_authority": False,
    }
    if any(normalized_provenance[key] is not value for key, value in expected_provenance.items()):
        raise StackExtentInitializerInputError(f"{label}.provenance_boundary must remain residue-only")
    if normalized_provenance["function"] != normalized_precursor["function"]:
        raise StackExtentInitializerInputError(f"{label}.provenance function drifts from precursor")

    telemetry_fields = {
        "parent_active_seconds",
        "active_seconds_measured",
        "parent_intervals_complete",
        "helper_coverage_complete",
        "candidate_heavy_coverage_complete",
        "throughput_complete",
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
    expected_telemetry = {
        "active_seconds_measured": True,
        "parent_intervals_complete": True,
        "helper_coverage_complete": False,
        "candidate_heavy_coverage_complete": False,
        "throughput_complete": False,
        "exclude_from_measured_crack_hour": True,
        "no_imputation": True,
    }
    for field, expected in expected_telemetry.items():
        if telemetry.get(field) is not expected:
            raise StackExtentInitializerInputError(f"{label}.telemetry.{field} must be {expected}")
    normalized_telemetry = {
        "parent_active_seconds": _positive_number(
            telemetry.get("parent_active_seconds"), f"{label}.telemetry.parent_active_seconds"
        ),
        **expected_telemetry,
        "telemetry_sha256": _sha256(
            telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"
        ),
        "active_interval_log_sha256": _sha256(
            telemetry.get("active_interval_log_sha256"),
            f"{label}.telemetry.active_interval_log_sha256",
        ),
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
    exact_target = _uint(exact.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=24)
    exact_candidate = _uint(
        exact.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=24
    )
    exact_relocations = _uint(
        exact.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1
    )
    if (
        exact_target != exact_candidate
        or exact_target != target_bytes
        or exact_relocations != target_relocations
    ):
        raise StackExtentInitializerInputError(f"{label}.exact_result must close size and relocations")
    normalized_exact: dict[str, Any] = {
        "candidate_id": _text(exact.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
    }
    exact_hash_pairs = {
        "source_sha256": "exact_source_sha256",
        "object_sha256": "exact_object_sha256",
        "strict_report_sha256": "exact_strict_report_sha256",
        "data_report_sha256": "exact_data_report_sha256",
        "candidate_record_sha256": "exact_record_sha256",
    }
    for field, proof_field in exact_hash_pairs.items():
        normalized_exact[field] = _sha256(exact.get(field), f"{label}.exact_result.{field}")
        if normalized_exact[field] != normalized_proofs[proof_field]:
            raise StackExtentInitializerInputError(f"{label}.exact_result.{field} drifts from proofs")

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "stack_seam": normalized_seam,
        "controls": controls,
        "provenance_boundary": normalized_provenance,
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


def _mismatch_rows(target: Sequence[Any], candidate: Sequence[Any]) -> list[tuple[int, Any, Any]]:
    result: list[tuple[int, Any, Any]] = []
    for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate)):
        if causal_reducer._instruction_mismatch(left, right):
            result.append((index, left, right))
    return result


def _li(instruction: Any) -> tuple[str, int] | None:
    if not instruction.has_instruction:
        return None
    match = _LI_RE.fullmatch(instruction.formatted)
    if match is None:
        return None
    return match.group("register").lower(), int(match.group("value"), 0)


def _dform(instruction: Any) -> tuple[str, str, int, str] | None:
    if not instruction.has_instruction:
        return None
    match = _DFORM_RE.fullmatch(instruction.formatted)
    if match is None:
        return None
    return (
        match.group("opcode").lower(),
        match.group("register").lower(),
        int(match.group("offset"), 0),
        match.group("base").lower(),
    )


def _initializer_sequence_indices(instructions: Sequence[Any], seam: Mapping[str, Any]) -> list[int]:
    expected = [
        ("li", seam["value_register"], seam["negative_initializer"], ""),
        ("stw", seam["value_register"], seam["selected_home_offset"], seam["base_register"]),
        ("stw", seam["value_register"], seam["selected_home_offset"], seam["base_register"]),
        ("li", seam["value_register"], seam["zero_initializer"], ""),
        ("stw", seam["value_register"], seam["overwritten_slot_offset"], seam["base_register"]),
        ("stw", seam["value_register"], seam["adjacent_zero_offset"], seam["base_register"]),
    ]
    actual: list[tuple[int, tuple[str, str, int, str]]] = []
    for index, instruction in enumerate(instructions):
        li = _li(instruction)
        if li is not None:
            actual.append((index, ("li", li[0], li[1], "")))
            continue
        dform = _dform(instruction)
        if dform is not None:
            actual.append((index, dform))
    starts: list[int] = []
    for index in range(0, len(actual) - len(expected) + 1):
        window = actual[index : index + len(expected)]
        if [item[1] for item in window] == expected:
            starts.append(window[0][0])
    return starts


def _slot_accesses(instructions: Sequence[Any], *, base: str, offset: int) -> tuple[int, int]:
    writes = 0
    reads = 0
    for instruction in instructions:
        parsed = _dform(instruction)
        if parsed is None or parsed[2:] != (offset, base):
            continue
        if parsed[0] in _STORE_OPCODES:
            writes += 1
        elif parsed[0] in _LOAD_OPCODES:
            reads += 1
        else:
            reads += 1
    return writes, reads


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
            "reason": "no authenticated stack-extent overwritten-initializer context was supplied",
        }
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
            "reason": "the physical residual rows differ from the sealed one-row precursor",
            "evidence": {
                "report_residual_rows": mismatch_indices,
                "context_residual_rows": precursor["residual_rows"],
            },
        }
    row, target_instruction, candidate_instruction = mismatches[0]
    if not (
        target_instruction is not None
        and target_instruction.has_instruction
        and candidate_instruction is not None
        and candidate_instruction.has_instruction
    ):
        return {"matched": False, "reason": "the residual must be one paired physical instruction"}

    seam = context["stack_seam"]
    target_store = _dform(target_instruction)
    candidate_store = _dform(candidate_instruction)
    expected_target = (
        "stw",
        seam["value_register"],
        seam["selected_home_offset"],
        seam["base_register"],
    )
    expected_candidate = (
        "stw",
        seam["value_register"],
        seam["candidate_selected_home_offset"],
        seam["base_register"],
    )
    if target_store != expected_target or candidate_store != expected_candidate:
        return {
            "matched": False,
            "reason": "the residual is not the sealed same-register +4 stack-home store",
            "evidence": {"row": row},
        }

    starts = _initializer_sequence_indices(target, seam)
    if len(starts) != 1:
        return {
            "matched": False,
            "reason": "the target must contain exactly one sealed six-instruction initializer sequence",
            "evidence": {"sequence_starts": starts},
        }
    writes, reads = _slot_accesses(
        target, base=seam["base_register"], offset=seam["overwritten_slot_offset"]
    )
    if (writes, reads) != (
        seam["overwritten_write_count"],
        seam["overwritten_read_count"],
    ):
        return {
            "matched": False,
            "reason": "the overwritten target slot is not exactly one-write/zero-read",
            "evidence": {"observed_writes": writes, "observed_reads": reads},
        }

    evidence = {
        "precursor": precursor,
        "stack_seam": seam,
        "target_initializer_sequence_start": starts[0],
        "target_residual_row": row,
        "overwritten_slot_accesses": {"writes": writes, "reads": reads},
        "negative_controls": context["controls"],
        "recommended_cells": [
            {
                "kind": "two_word_natural_aggregate_extent",
                "declaration": "int objectNo[2]",
                "initializer": "objectNo[0] = objectNo[0] = -1",
                "current_extent_bytes": seam["current_extent_bytes"],
                "target_extent_bytes": seam["target_extent_bytes"],
                "element_size": seam["element_size"],
                "predicted_capacity": seam["predicted_capacity"],
                "preserve_all_other_source_axes": True,
                "provenance_scope": "path_and_function_scoped_target_residue",
            }
        ],
        "suppressed_axes": [
            "repeat_one_word_aggregate",
            "three_word_or_larger_state",
            "particle_data_aliases",
            "max_vertex_overwrite",
            "self_chain",
            "address_or_sizeof_visibility",
            "constant_bound_visibility",
            "declaration_or_scope_permutations",
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "repeat_failed_closed_tracer",
            "claim_original_source_provenance",
            "general_dead_storage_waiver",
            "automatic_retention_or_promotion",
        ],
        "provenance_boundary": context["provenance_boundary"],
        "telemetry": context["telemetry"],
        "combined_exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "one authenticated +4-byte stack-home seam, a one-write/zero-read overwritten slot, and sealed controls isolate the two-word aggregate extent",
        "confidence": 0.999,
        "source_class": "two_word_stack_extent_with_duplicate_first_initializer",
        "recommendation": (
            "Compile only the two-word natural aggregate with the duplicate first-element "
            "initializer; preserve the residue-only provenance caveat."
        ),
        "evidence": evidence,
    }

