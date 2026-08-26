#!/usr/bin/env python3
"""Fail-closed saved-owner semantic split and callback-form composition."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "saved_owner_semantic_split_context/v1"
RULE_ID = "saved_owner_semantic_split_composer"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_SESSION_RE = re.compile(r"session-[0-9a-f]{16}")
_REGISTER_RE = re.compile(r"r(?:[0-9]|[12][0-9]|3[01])")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "cfg_calls_exact",
    "data_exact",
    "physical_relocations_exact",
    "same_session_object_inventory_authenticated",
    "measured_controls_complete",
    "interaction_plan_authenticated",
    "exact_result_verified",
    "protected_siblings_preserved",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "trace_envelope_file_sha256",
    "trace_envelope_sha256",
    "interaction_request_sha256",
    "interaction_plan_sha256",
    "precursor_source_sha256",
    "precursor_object_sha256",
    "function_scope_source_sha256",
    "function_scope_object_sha256",
    "direct_callback_source_sha256",
    "direct_callback_object_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "compile_attestation_sha256",
    "report_artifact_sha256",
)
_EXPECTED_GROUPS = (
    "saved_range",
    "data_format",
    "callback",
    "outer_i",
    "pat_x",
    "pat_y",
    "inner_j",
)
_EXPECTED_TRACE_OWNERS = ("hook", "patX", "patY", "i", "j")


class SavedOwnerSplitInputError(ValueError):
    """The supplied evidence cannot safely support this composition."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SavedOwnerSplitInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise SavedOwnerSplitInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise SavedOwnerSplitInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SavedOwnerSplitInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise SavedOwnerSplitInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise SavedOwnerSplitInputError(f"{label} must be a bounded owner path")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise SavedOwnerSplitInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise SavedOwnerSplitInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise SavedOwnerSplitInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _positive_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SavedOwnerSplitInputError(f"{label} must be a positive number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise SavedOwnerSplitInputError(f"{label} must be positive and finite")
    return result


def _nonnegative_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SavedOwnerSplitInputError(f"{label} must be a nonnegative number")
    result = float(value)
    if not math.isfinite(result) or result < 0.0:
        raise SavedOwnerSplitInputError(f"{label} must be nonnegative and finite")
    return result


def _register(value: Any, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if _REGISTER_RE.fullmatch(result) is None:
        raise SavedOwnerSplitInputError(f"{label} must be a physical GPR")
    return result


def _rows(value: Any, label: str, *, minimum: int = 1) -> list[int]:
    if not isinstance(value, list):
        raise SavedOwnerSplitInputError(f"{label} must be a row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) < minimum:
        raise SavedOwnerSplitInputError(
            f"{label} must be sorted, unique, and contain at least {minimum} row(s)"
        )
    return rows


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise SavedOwnerSplitInputError(f"{label} must be {str(expected).lower()}")
    return expected


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "saved-owner semantic-split context"
    fields = {
        "schema",
        "proofs",
        "precursor",
        "trace_inventory",
        "measured_controls",
        "interaction",
        "telemetry",
        "exact_result",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SavedOwnerSplitInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES) | {"authority_advanced"}
    proofs = _closed(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label=f"{label}.proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        normalized_proofs[field] = _bool(
            proofs.get(field), f"{label}.proofs.{field}", True
        )
    normalized_proofs["authority_advanced"] = _bool(
        proofs.get("authority_advanced"),
        f"{label}.proofs.authority_advanced",
        False,
    )
    for field in _PROOF_HASHES:
        normalized_proofs[field] = _sha256(
            proofs.get(field), f"{label}.proofs.{field}"
        )

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
        "target_instruction_relocations",
        "candidate_instruction_relocations",
        "residual_rows",
        "residual_groups",
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
    target_relocs = _uint(
        precursor.get("target_physical_relocations"),
        f"{label}.precursor.target_physical_relocations",
        minimum=1,
    )
    candidate_relocs = _uint(
        precursor.get("candidate_physical_relocations"),
        f"{label}.precursor.candidate_physical_relocations",
        minimum=1,
    )
    instruction_target_relocs = _uint(
        precursor.get("target_instruction_relocations"),
        f"{label}.precursor.target_instruction_relocations",
        minimum=1,
    )
    instruction_candidate_relocs = _uint(
        precursor.get("candidate_instruction_relocations"),
        f"{label}.precursor.candidate_instruction_relocations",
        minimum=1,
    )
    match_percent = _positive_number(
        precursor.get("match_percent"), f"{label}.precursor.match_percent"
    )
    if (
        target_bytes != candidate_bytes
        or target_frame != candidate_frame
        or target_relocs != candidate_relocs
        or instruction_target_relocs != instruction_candidate_relocs
        or instruction_target_relocs > target_relocs
        or not 99.0 <= match_percent < 100.0
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.precursor must be high-scoring with exact size/frame/relocations"
        )
    residual_rows = _rows(
        precursor.get("residual_rows"), f"{label}.precursor.residual_rows", minimum=6
    )
    raw_groups = precursor.get("residual_groups")
    if not isinstance(raw_groups, list) or len(raw_groups) != len(_EXPECTED_GROUPS):
        raise SavedOwnerSplitInputError(
            f"{label}.precursor.residual_groups must cover the seven sealed owner groups"
        )
    groups: list[dict[str, Any]] = []
    covered: list[int] = []
    for index, raw in enumerate(raw_groups):
        group = _closed(
            raw,
            allowed={
                "name",
                "owner",
                "rows",
                "target_token",
                "candidate_token",
            },
            required={
                "name",
                "owner",
                "rows",
                "target_token",
                "candidate_token",
            },
            label=f"{label}.precursor.residual_groups[{index}]",
        )
        name = _identifier(
            group.get("name"), f"{label}.precursor.residual_groups[{index}].name"
        )
        rows = _rows(
            group.get("rows"), f"{label}.precursor.residual_groups[{index}].rows"
        )
        target_token = _text(
            group.get("target_token"),
            f"{label}.precursor.residual_groups[{index}].target_token",
            limit=32,
        )
        candidate_token = _text(
            group.get("candidate_token"),
            f"{label}.precursor.residual_groups[{index}].candidate_token",
            limit=32,
        )
        if target_token == candidate_token:
            raise SavedOwnerSplitInputError(
                f"{label}.precursor.residual_groups[{index}] tokens must differ"
            )
        groups.append(
            {
                "name": name,
                "owner": _identifier(
                    group.get("owner"),
                    f"{label}.precursor.residual_groups[{index}].owner",
                ),
                "rows": rows,
                "target_token": target_token,
                "candidate_token": candidate_token,
            }
        )
        covered.extend(rows)
    if tuple(item["name"] for item in groups) != _EXPECTED_GROUPS:
        raise SavedOwnerSplitInputError(
            f"{label}.precursor residual groups must use the canonical order"
        )
    if sorted(covered) != residual_rows or len(set(covered)) != len(covered):
        raise SavedOwnerSplitInputError(
            f"{label}.precursor residual groups must partition every residual row"
        )
    normalized_precursor = {
        "function": _identifier(
            precursor.get("function"), f"{label}.precursor.function"
        ),
        "candidate_id": _owner(
            precursor.get("candidate_id"), f"{label}.precursor.candidate_id"
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "target_physical_relocations": target_relocs,
        "candidate_physical_relocations": candidate_relocs,
        "target_instruction_relocations": instruction_target_relocs,
        "candidate_instruction_relocations": instruction_candidate_relocs,
        "residual_rows": residual_rows,
        "residual_groups": groups,
    }

    trace_fields = {
        "session_id",
        "function",
        "candidate_id",
        "complete_object_inventory",
        "unknown_owner_count",
        "source_spans_narrow_verified",
        "owners",
    }
    trace = _closed(
        context.get("trace_inventory"),
        allowed=trace_fields,
        required=trace_fields,
        label=f"{label}.trace_inventory",
    )
    session_id = _text(
        trace.get("session_id"), f"{label}.trace_inventory.session_id", limit=64
    )
    if _SESSION_RE.fullmatch(session_id) is None:
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory.session_id is not canonical"
        )
    if (
        trace.get("complete_object_inventory") is not True
        or trace.get("unknown_owner_count") != 0
        or trace.get("source_spans_narrow_verified") is not True
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory must be complete, narrow, and have zero UNKNOWN"
        )
    raw_owners = trace.get("owners")
    if not isinstance(raw_owners, list) or len(raw_owners) != len(_EXPECTED_TRACE_OWNERS):
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory must bind hook/patX/patY/i/j"
        )
    owners: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_owners):
        owner = _closed(
            raw,
            allowed={"identity", "candidate_register", "target_register", "role"},
            required={"identity", "candidate_register", "target_register", "role"},
            label=f"{label}.trace_inventory.owners[{index}]",
        )
        owners.append(
            {
                "identity": _identifier(
                    owner.get("identity"),
                    f"{label}.trace_inventory.owners[{index}].identity",
                ),
                "candidate_register": _register(
                    owner.get("candidate_register"),
                    f"{label}.trace_inventory.owners[{index}].candidate_register",
                ),
                "target_register": _register(
                    owner.get("target_register"),
                    f"{label}.trace_inventory.owners[{index}].target_register",
                ),
                "role": _identifier(
                    owner.get("role"),
                    f"{label}.trace_inventory.owners[{index}].role",
                ),
            }
        )
    if tuple(item["identity"] for item in owners) != _EXPECTED_TRACE_OWNERS:
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory owner order must be hook/patX/patY/i/j"
        )
    observed_trace_colors = {
        item["identity"]: (
            item["candidate_register"],
            item["target_register"],
        )
        for item in owners
    }
    if observed_trace_colors != {
        "hook": ("r22", "r19"),
        "patX": ("r21", "r21"),
        "patY": ("r20", "r20"),
        "i": ("r26", "r25"),
        "j": ("r25", "r26"),
    }:
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory does not close the saved-owner cycle"
        )
    normalized_trace = {
        "session_id": session_id,
        "function": _identifier(
            trace.get("function"), f"{label}.trace_inventory.function"
        ),
        "candidate_id": _owner(
            trace.get("candidate_id"), f"{label}.trace_inventory.candidate_id"
        ),
        "complete_object_inventory": True,
        "unknown_owner_count": 0,
        "source_spans_narrow_verified": True,
        "owners": owners,
    }
    if normalized_trace["function"] != normalized_precursor["function"]:
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory is bound to another function"
        )

    controls_fields = {
        "block_callback_precursor",
        "function_scope_callback",
        "direct_callback_with_split",
        "declaration_order_control",
    }
    controls = _closed(
        context.get("measured_controls"),
        allowed=controls_fields,
        required=controls_fields,
        label=f"{label}.measured_controls",
    )

    def control(name: str, *, exact_size: bool) -> dict[str, Any]:
        raw = _closed(
            controls.get(name),
            allowed={
                "candidate_id",
                "source_sha256",
                "object_sha256",
                "target_bytes",
                "candidate_bytes",
                "match_percent",
                "callback_form",
                "data_format_form",
                "measured",
            },
            required={
                "candidate_id",
                "source_sha256",
                "object_sha256",
                "target_bytes",
                "candidate_bytes",
                "match_percent",
                "callback_form",
                "data_format_form",
                "measured",
            },
            label=f"{label}.measured_controls.{name}",
        )
        target = _uint(raw.get("target_bytes"), f"{label}.measured_controls.{name}.target_bytes", minimum=32)
        candidate = _uint(raw.get("candidate_bytes"), f"{label}.measured_controls.{name}.candidate_bytes", minimum=32)
        if raw.get("measured") is not True or (target == candidate) is not exact_size:
            raise SavedOwnerSplitInputError(
                f"{label}.measured_controls.{name} size/control contract drifted"
            )
        return {
            "candidate_id": _owner(raw.get("candidate_id"), f"{label}.measured_controls.{name}.candidate_id"),
            "source_sha256": _sha256(raw.get("source_sha256"), f"{label}.measured_controls.{name}.source_sha256"),
            "object_sha256": _sha256(raw.get("object_sha256"), f"{label}.measured_controls.{name}.object_sha256"),
            "target_bytes": target,
            "candidate_bytes": candidate,
            "match_percent": _positive_number(raw.get("match_percent"), f"{label}.measured_controls.{name}.match_percent"),
            "callback_form": _identifier(raw.get("callback_form"), f"{label}.measured_controls.{name}.callback_form"),
            "data_format_form": _identifier(raw.get("data_format_form"), f"{label}.measured_controls.{name}.data_format_form"),
            "measured": True,
        }

    normalized_controls = {
        "block_callback_precursor": control("block_callback_precursor", exact_size=True),
        "function_scope_callback": control("function_scope_callback", exact_size=True),
        "direct_callback_with_split": control("direct_callback_with_split", exact_size=False),
    }
    sealed_control_hashes = {
        "block_callback_precursor": (
            "precursor_source_sha256",
            "precursor_object_sha256",
        ),
        "function_scope_callback": (
            "function_scope_source_sha256",
            "function_scope_object_sha256",
        ),
        "direct_callback_with_split": (
            "direct_callback_source_sha256",
            "direct_callback_object_sha256",
        ),
    }
    for control_name, (source_proof, object_proof) in sealed_control_hashes.items():
        measured_control = normalized_controls[control_name]
        if (
            measured_control["source_sha256"] != normalized_proofs[source_proof]
            or measured_control["object_sha256"] != normalized_proofs[object_proof]
        ):
            raise SavedOwnerSplitInputError(
                f"{label}.measured_controls.{control_name} drifts from proofs"
            )
    if (
        normalized_controls["block_callback_precursor"]["candidate_id"]
        != normalized_precursor["candidate_id"]
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.measured_controls.block_callback_precursor is not the sealed precursor"
        )
    if (
        normalized_controls["function_scope_callback"]["candidate_id"]
        != normalized_trace["candidate_id"]
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.trace_inventory is not bound to the measured function-scope control"
        )
    declaration = _closed(
        controls.get("declaration_order_control"),
        allowed={"candidate_id", "object_sha256", "same_as_precursor", "measured"},
        required={"candidate_id", "object_sha256", "same_as_precursor", "measured"},
        label=f"{label}.measured_controls.declaration_order_control",
    )
    if declaration.get("same_as_precursor") is not True or declaration.get("measured") is not True:
        raise SavedOwnerSplitInputError(
            f"{label}.measured_controls.declaration_order_control must be object-neutral"
        )
    declaration_object = _sha256(
        declaration.get("object_sha256"),
        f"{label}.measured_controls.declaration_order_control.object_sha256",
    )
    if declaration_object != normalized_proofs["precursor_object_sha256"]:
        raise SavedOwnerSplitInputError(
            f"{label}.measured_controls.declaration_order_control must duplicate the precursor object"
        )
    normalized_controls["declaration_order_control"] = {
        "candidate_id": _owner(
            declaration.get("candidate_id"),
            f"{label}.measured_controls.declaration_order_control.candidate_id",
        ),
        "object_sha256": declaration_object,
        "same_as_precursor": True,
        "measured": True,
    }
    if (
        normalized_controls["block_callback_precursor"]["callback_form"] != "block_local"
        or normalized_controls["block_callback_precursor"]["data_format_form"] != "reuse_outer_i"
        or normalized_controls["function_scope_callback"]["callback_form"] != "function_local"
        or normalized_controls["function_scope_callback"]["data_format_form"] != "reuse_outer_i"
        or normalized_controls["direct_callback_with_split"]["callback_form"] != "direct_field_call"
        or normalized_controls["direct_callback_with_split"]["data_format_form"] != "distinct_s16_dataFmt"
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.measured_controls do not close the two intended axes"
        )

    interaction_fields = {
        "axes",
        "only_missing_cell",
        "callback_form",
        "semantic_owner_form",
        "semantic_owner_type",
        "semantic_owner_identity",
        "suppressed_axes",
    }
    interaction = _closed(
        context.get("interaction"),
        allowed=interaction_fields,
        required=interaction_fields,
        label=f"{label}.interaction",
    )
    axes = interaction.get("axes")
    if axes != ["callback_consumption", "data_format_owner"]:
        raise SavedOwnerSplitInputError(
            f"{label}.interaction must contain only callback and data-format axes"
        )
    if interaction.get("only_missing_cell") is not True:
        raise SavedOwnerSplitInputError(
            f"{label}.interaction.only_missing_cell must be true"
        )
    suppressed = interaction.get("suppressed_axes")
    expected_suppressed = [
        "direct_field_call",
        "function_local_only",
        "declaration_order_i_j",
        "repeat_tracer",
    ]
    if suppressed != expected_suppressed:
        raise SavedOwnerSplitInputError(
            f"{label}.interaction.suppressed_axes must be canonical"
        )
    normalized_interaction = {
        "axes": list(axes),
        "only_missing_cell": True,
        "callback_form": _identifier(
            interaction.get("callback_form"), f"{label}.interaction.callback_form"
        ),
        "semantic_owner_form": _identifier(
            interaction.get("semantic_owner_form"),
            f"{label}.interaction.semantic_owner_form",
        ),
        "semantic_owner_type": _identifier(
            interaction.get("semantic_owner_type"),
            f"{label}.interaction.semantic_owner_type",
        ),
        "semantic_owner_identity": _identifier(
            interaction.get("semantic_owner_identity"),
            f"{label}.interaction.semantic_owner_identity",
        ),
        "suppressed_axes": list(suppressed),
    }
    if normalized_interaction != {
        "axes": ["callback_consumption", "data_format_owner"],
        "only_missing_cell": True,
        "callback_form": "block_local",
        "semantic_owner_form": "distinct_s16_dataFmt",
        "semantic_owner_type": "s16",
        "semantic_owner_identity": "dataFmt",
        "suppressed_axes": expected_suppressed,
    }:
        raise SavedOwnerSplitInputError(
            f"{label}.interaction must select block_local x distinct_s16_dataFmt"
        )

    telemetry_fields = {
        "parent_active_seconds",
        "helper_active_seconds_sum",
        "team_active_seconds_sum",
        "active_wall_union_seconds",
        "active_time_telemetry_complete",
        "heavy_seconds_complete",
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
    normalized_telemetry = {
        "parent_active_seconds": _positive_number(telemetry.get("parent_active_seconds"), f"{label}.telemetry.parent_active_seconds"),
        "helper_active_seconds_sum": _nonnegative_number(telemetry.get("helper_active_seconds_sum"), f"{label}.telemetry.helper_active_seconds_sum"),
        "team_active_seconds_sum": _positive_number(telemetry.get("team_active_seconds_sum"), f"{label}.telemetry.team_active_seconds_sum"),
        "active_wall_union_seconds": _positive_number(telemetry.get("active_wall_union_seconds"), f"{label}.telemetry.active_wall_union_seconds"),
        "active_time_telemetry_complete": _bool(telemetry.get("active_time_telemetry_complete"), f"{label}.telemetry.active_time_telemetry_complete", True),
        "heavy_seconds_complete": _bool(telemetry.get("heavy_seconds_complete"), f"{label}.telemetry.heavy_seconds_complete", False),
        "no_imputation": _bool(telemetry.get("no_imputation"), f"{label}.telemetry.no_imputation", True),
        "telemetry_sha256": _sha256(telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"),
        "active_interval_log_sha256": _sha256(telemetry.get("active_interval_log_sha256"), f"{label}.telemetry.active_interval_log_sha256"),
    }
    if not math.isclose(
        normalized_telemetry["team_active_seconds_sum"],
        normalized_telemetry["parent_active_seconds"]
        + normalized_telemetry["helper_active_seconds_sum"],
        rel_tol=0.0,
        abs_tol=0.001,
    ):
        raise SavedOwnerSplitInputError(
            f"{label}.telemetry team total does not equal parent plus helper"
        )

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
    if exact_target != exact_candidate or exact_target != target_bytes:
        raise SavedOwnerSplitInputError(
            f"{label}.exact_result size must close the precursor"
        )
    normalized_exact = {
        "candidate_id": _owner(exact.get("candidate_id"), f"{label}.exact_result.candidate_id"),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": _uint(exact.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1),
        "source_sha256": _sha256(exact.get("source_sha256"), f"{label}.exact_result.source_sha256"),
        "object_sha256": _sha256(exact.get("object_sha256"), f"{label}.exact_result.object_sha256"),
        "strict_report_sha256": _sha256(exact.get("strict_report_sha256"), f"{label}.exact_result.strict_report_sha256"),
        "data_report_sha256": _sha256(exact.get("data_report_sha256"), f"{label}.exact_result.data_report_sha256"),
        "candidate_record_sha256": _sha256(exact.get("candidate_record_sha256"), f"{label}.exact_result.candidate_record_sha256"),
    }
    if normalized_exact["physical_relocations"] != target_relocs:
        raise SavedOwnerSplitInputError(
            f"{label}.exact_result relocation count must close the precursor"
        )
    proof_links = {
        "source_sha256": "exact_source_sha256",
        "object_sha256": "exact_object_sha256",
        "strict_report_sha256": "exact_strict_report_sha256",
        "data_report_sha256": "exact_data_report_sha256",
        "candidate_record_sha256": "exact_record_sha256",
    }
    for exact_field, proof_field in proof_links.items():
        if normalized_exact[exact_field] != normalized_proofs[proof_field]:
            raise SavedOwnerSplitInputError(
                f"{label}.exact_result.{exact_field} drifts from proofs"
            )

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "trace_inventory": normalized_trace,
        "measured_controls": normalized_controls,
        "interaction": normalized_interaction,
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
        for index, (left, right) in enumerate(
            causal_reducer._paired_records(target, candidate)
        )
        if (left is not None and causal_reducer._is_diff_kind(left.diff_kind))
        or (right is not None and causal_reducer._is_diff_kind(right.diff_kind))
    ]


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


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
            "reason": "no authenticated saved-owner semantic-split context was supplied",
        }
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {
            "matched": False,
            "reason": "the context is bound to another objdiff report",
        }
    precursor = context["precursor"]
    if pair.name != precursor["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
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
        precursor["target_instruction_relocations"],
        precursor["candidate_instruction_relocations"],
    )
    if observed_signature != sealed_signature:
        return {
            "matched": False,
            "reason": "the size/frame/relocation signature drifted",
            "evidence": {
                "observed": list(observed_signature),
                "sealed": list(sealed_signature),
            },
        }
    residual_rows = _mismatch_rows(target, candidate)
    if residual_rows != precursor["residual_rows"]:
        return {
            "matched": False,
            "reason": "the physical residual rows differ from the sealed owner cycle",
            "evidence": {
                "observed_rows": residual_rows,
                "sealed_rows": precursor["residual_rows"],
            },
        }
    paired = causal_reducer._paired_records(target, candidate)
    for group in precursor["residual_groups"]:
        for row in group["rows"]:
            left, right = paired[row]
            if (
                left is None
                or right is None
                or not left.has_instruction
                or not right.has_instruction
                or left.mnemonic != right.mnemonic
                or group["target_token"].lower() not in left.formatted.lower()
                or group["candidate_token"].lower() not in right.formatted.lower()
            ):
                return {
                    "matched": False,
                    "reason": "a residual row no longer carries its sealed owner substitution",
                    "evidence": {"group": group["name"], "row": row},
                }

    trace = context["trace_inventory"]
    target_map = {
        "dataFmt": "r22",
        **{item["identity"]: item["target_register"] for item in trace["owners"]},
    }
    if target_map != {
        "dataFmt": "r22",
        "hook": "r19",
        "patX": "r21",
        "patY": "r20",
        "i": "r25",
        "j": "r26",
    }:
        return {
            "matched": False,
            "reason": "the authenticated owner inventory no longer closes the target colors",
            "evidence": {"target_owner_registers": target_map},
        }

    recommended_cell = {
        "order": 1,
        "kind": "saved_owner_semantic_split_composition",
        "callback_form": "typed_callback_guard_block_local",
        "semantic_owner_declaration": "s16 dataFmt;",
        "semantic_owner_assignment": "dataFmt = workP->animP->bmp->dataFmt & ANIM_BMP_FMTMASK;",
        "semantic_owner_consumers": ["ANIM_BMP_I8", "ANIM_BMP_I4"],
        "target_owner_registers": target_map,
        "compile_as_one_cell": True,
        "requires_repeat_trace": False,
    }
    evidence = {
        "precursor": precursor,
        "trace_inventory": trace,
        "measured_controls": context["measured_controls"],
        "recommended_cells": [recommended_cell],
        "suppressed_axes": context["interaction"]["suppressed_axes"]
        + [
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "automatic_retention_or_promotion",
        ],
        "telemetry": context["telemetry"],
        "exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "an authenticated Object inventory and measured callback controls close one early semantic-owner/loop/callback allocation cycle",
        "confidence": 0.995,
        "source_class": "distinct_truthful_early_owner_composed_with_topology_exact_block_callback",
        "recommendation": "Compile exactly one cell: keep the typed callback in its non-null guard and split the early bitmap format into a truthful s16 dataFmt owner; do not retry callback scope or loop declaration order.",
        "evidence": evidence,
    }
