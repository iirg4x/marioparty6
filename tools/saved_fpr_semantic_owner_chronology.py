#!/usr/bin/env python3
"""Fail-closed saved-FPR semantic-owner reconstruction and chronology diagnosis."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "saved_fpr_semantic_owner_chronology_context/v1"
RULE_ID = "saved_fpr_semantic_owner_chronology"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@#-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FRAME_RE = re.compile(r"stwu\s+r1,\s*-(?P<size>0x[0-9A-Fa-f]+|[0-9]+)\(r1\)")
_FPR_RE = re.compile(r"\bf(?:[0-9]|[12][0-9]|3[01])\b")
_SDA_OWNER_RE = re.compile(r"[^,\s]+@sda21\b")

SEMANTIC_OWNER_COMPONENTS = (
    "distinct_rot_y_snapshots",
    "named_trig_call_results",
    "post_call_scalar_consumer_copies",
    "integer_conversion_mask",
)
DECLARATION_ORDER = (
    "weight",
    "radius",
    "scale",
    "rotY",
    "rotYSecond",
    "cosWeight",
    "sinRotYResult",
    "sinRotY",
    "sinRotX",
    "cosRotX",
    "cosRotYResult",
    "cosRotY",
    "sinRotXSecond",
    "sinWeight",
)
TARGET_FPR_MAP = {
    "weight": "f31",
    "radius": "f30",
    "scale": "f29",
    "rotY": "f28",
    "rotYSecond": "f27",
    "cosWeight": "f26",
    "sinRotYResult": "f25",
    "sinRotY": "f24",
    "sinRotX": "f23",
    "cosRotX": "f22",
    "cosRotYResult": "f21",
    "cosRotY": "f20",
    "sinRotXSecond": "f19",
    "sinWeight": "f18",
}
DONOR_ROLES = (
    "post_call_scalar_copy_sine",
    "post_call_scalar_copy_cosine",
    "integer_conversion_mask_255",
)
FORBIDDEN_AXES = (
    "typed_u8_alpha_local",
    "block_initializer_permutations",
    "partial_owner_cycle",
    "unknown_owner_chronology",
    "generic_declaration_permutations",
    "dead_or_fake_locals",
    "padding",
    "aliases",
    "register_shaping",
    "tracer_before_static_closure",
    "source_retention",
    "promotion",
)


class SavedFprSemanticOwnerInputError(ValueError):
    """The supplied evidence cannot safely support this diagnosis."""


def _closed(value: Any, *, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SavedFprSemanticOwnerInputError(f"{label} must be a JSON object")
    missing = fields - set(value)
    extra = set(value) - fields
    if missing or extra:
        raise SavedFprSemanticOwnerInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise SavedFprSemanticOwnerInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SavedFprSemanticOwnerInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise SavedFprSemanticOwnerInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise SavedFprSemanticOwnerInputError(f"{label} has invalid characters")
    return result


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise SavedFprSemanticOwnerInputError(f"{label} must be lowercase SHA-256")
    return value


def _boolean(value: Any, label: str, expected: bool) -> bool:
    if type(value) is not bool or value is not expected:
        raise SavedFprSemanticOwnerInputError(f"{label} must be {expected}")
    return expected


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if type(value) is not int or value < minimum or value > maximum:
        raise SavedFprSemanticOwnerInputError(
            f"{label} must be an integer in [{minimum}, {maximum}]"
        )
    return value


def _number(value: Any, label: str, *, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SavedFprSemanticOwnerInputError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise SavedFprSemanticOwnerInputError(f"{label} is outside the accepted range")
    return result


def _exact_sequence(value: Any, expected: Sequence[str], label: str) -> list[str]:
    if not isinstance(value, list):
        raise SavedFprSemanticOwnerInputError(f"{label} must be an array")
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if result != list(expected):
        raise SavedFprSemanticOwnerInputError(f"{label} must equal the sealed sequence")
    return result


def _artifact(value: Any, label: str, *, exact: bool) -> dict[str, str]:
    fields = {
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "candidate_record_sha256",
    }
    if exact:
        fields.add("compile_attestation_sha256")
    item = _closed(value, fields=fields, label=label)
    return {field: _sha(item.get(field), f"{label}.{field}") for field in fields}


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "saved-FPR semantic-owner chronology context"
    root = _closed(
        value,
        fields={
            "schema",
            "owner",
            "function",
            "source_owner_task",
            "authority_advanced",
            "report",
            "provenance",
            "donors",
            "baseline",
            "semantic_owner_stage",
            "chronology",
            "exact_result",
            "negative_controls",
            "telemetry",
            "forbidden_axes",
        },
        label=label,
    )
    if _text(root.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SavedFprSemanticOwnerInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    owner = _owner(root.get("owner"), f"{label}.owner")
    function = _identifier(root.get("function"), f"{label}.function")
    source_owner_task = _owner(root.get("source_owner_task"), f"{label}.source_owner_task")
    _boolean(root.get("authority_advanced"), f"{label}.authority_advanced", False)

    report = _closed(
        root.get("report"),
        fields={"report_sha256", "base_source_sha256", "target_object_sha256"},
        label=f"{label}.report",
    )
    normalized_report = {
        field: _sha(report.get(field), f"{label}.report.{field}")
        for field in ("report_sha256", "base_source_sha256", "target_object_sha256")
    }

    provenance = _closed(
        root.get("provenance"),
        fields={
            "graphify_report_location",
            "narrow_verified_location",
            "graphify_bound",
            "graft_ask_count",
            "graft_status",
            "narrow_named_file_verified",
            "broad_searches",
        },
        label=f"{label}.provenance",
    )
    normalized_provenance = {
        "graphify_report_location": _text(
            provenance.get("graphify_report_location"),
            f"{label}.provenance.graphify_report_location",
        ),
        "narrow_verified_location": _text(
            provenance.get("narrow_verified_location"),
            f"{label}.provenance.narrow_verified_location",
        ),
        "graphify_bound": _boolean(
            provenance.get("graphify_bound"), f"{label}.provenance.graphify_bound", True
        ),
        "graft_ask_count": _uint(
            provenance.get("graft_ask_count"),
            f"{label}.provenance.graft_ask_count",
            minimum=1,
            maximum=1,
        ),
        "graft_status": _text(
            provenance.get("graft_status"), f"{label}.provenance.graft_status"
        ),
        "narrow_named_file_verified": _boolean(
            provenance.get("narrow_named_file_verified"),
            f"{label}.provenance.narrow_named_file_verified",
            True,
        ),
        "broad_searches": _uint(
            provenance.get("broad_searches"),
            f"{label}.provenance.broad_searches",
            maximum=0,
        ),
    }
    if normalized_provenance["graft_status"] != "no_nodes":
        raise SavedFprSemanticOwnerInputError(
            f"{label}.provenance.graft_status must be no_nodes"
        )

    raw_donors = root.get("donors")
    if not isinstance(raw_donors, list) or len(raw_donors) != 3:
        raise SavedFprSemanticOwnerInputError(f"{label}.donors must contain three exact donors")
    donors: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_donors):
        item_label = f"{label}.donors[{index}]"
        item = _closed(
            raw,
            fields={
                "function",
                "role",
                "source_location",
                "same_translation_unit",
                "strict_exact",
                "data_exact",
                "source_shape",
            },
            label=item_label,
        )
        donor_function = _identifier(item.get("function"), f"{item_label}.function")
        if donor_function == function:
            raise SavedFprSemanticOwnerInputError(f"{item_label}.function must differ from focus")
        donors.append(
            {
                "function": donor_function,
                "role": _text(item.get("role"), f"{item_label}.role"),
                "source_location": _text(
                    item.get("source_location"), f"{item_label}.source_location"
                ),
                "same_translation_unit": _boolean(
                    item.get("same_translation_unit"),
                    f"{item_label}.same_translation_unit",
                    True,
                ),
                "strict_exact": _boolean(
                    item.get("strict_exact"), f"{item_label}.strict_exact", True
                ),
                "data_exact": _boolean(
                    item.get("data_exact"), f"{item_label}.data_exact", True
                ),
                "source_shape": _text(
                    item.get("source_shape"), f"{item_label}.source_shape"
                ),
            }
        )
    if [item["role"] for item in donors] != list(DONOR_ROLES):
        raise SavedFprSemanticOwnerInputError(f"{label}.donors roles must equal the sealed sequence")
    if len({item["function"] for item in donors}) != 3:
        raise SavedFprSemanticOwnerInputError(f"{label}.donors functions must be distinct")

    baseline = _closed(
        root.get("baseline"),
        fields={
            "target_size",
            "candidate_size",
            "target_frame",
            "candidate_frame",
            "target_objdiff_relocation_records",
            "candidate_objdiff_relocation_records",
            "match_percent",
            "target_saved_fprs",
            "candidate_saved_fprs",
            "missing_semantic_owner_count",
            "verified_physical_relocations",
            "recommended_components",
        },
        label=f"{label}.baseline",
    )
    baseline_values = {
        field: _uint(baseline.get(field), f"{label}.baseline.{field}", minimum=1)
        for field in (
            "target_size",
            "candidate_size",
            "target_frame",
            "candidate_frame",
            "target_objdiff_relocation_records",
            "candidate_objdiff_relocation_records",
            "missing_semantic_owner_count",
            "verified_physical_relocations",
        )
    }
    baseline_values["match_percent"] = _number(
        baseline.get("match_percent"), f"{label}.baseline.match_percent"
    )
    baseline_values["target_saved_fprs"] = _exact_sequence(
        baseline.get("target_saved_fprs"),
        tuple(f"f{index}" for index in range(18, 32)),
        f"{label}.baseline.target_saved_fprs",
    )
    baseline_values["candidate_saved_fprs"] = _exact_sequence(
        baseline.get("candidate_saved_fprs"),
        tuple(f"f{index}" for index in range(24, 32)),
        f"{label}.baseline.candidate_saved_fprs",
    )
    baseline_values["recommended_components"] = _exact_sequence(
        baseline.get("recommended_components"),
        SEMANTIC_OWNER_COMPONENTS,
        f"{label}.baseline.recommended_components",
    )
    if not (
        baseline_values["candidate_size"] < baseline_values["target_size"]
        and baseline_values["candidate_frame"] < baseline_values["target_frame"]
        and baseline_values["target_objdiff_relocation_records"]
        == baseline_values["candidate_objdiff_relocation_records"]
        and baseline_values["verified_physical_relocations"]
        >= baseline_values["target_objdiff_relocation_records"]
        and baseline_values["missing_semantic_owner_count"] == 6
        and baseline_values["match_percent"] < 100.0
    ):
        raise SavedFprSemanticOwnerInputError(f"{label}.baseline signature drifted")

    stage = _closed(
        root.get("semantic_owner_stage"),
        fields={
            "candidate_id",
            "objdiff_canonical_sha256",
            "target_size",
            "candidate_size",
            "target_frame",
            "candidate_frame",
            "target_objdiff_relocation_records",
            "candidate_objdiff_relocation_records",
            "match_percent",
            "residual_count",
            "diff_kind",
            "operation_order_exact",
            "cfg_calls_exact",
            "stack_homes_exact",
            "data_exact",
            "protected_siblings_preserved",
            "verified_physical_relocations",
            "artifact",
        },
        label=f"{label}.semantic_owner_stage",
    )
    normalized_stage = {
        "candidate_id": _owner(
            stage.get("candidate_id"), f"{label}.semantic_owner_stage.candidate_id"
        ),
        "objdiff_canonical_sha256": _sha(
            stage.get("objdiff_canonical_sha256"),
            f"{label}.semantic_owner_stage.objdiff_canonical_sha256",
        ),
        "artifact": _artifact(
            stage.get("artifact"), f"{label}.semantic_owner_stage.artifact", exact=False
        ),
    }
    for field in (
        "target_size",
        "candidate_size",
        "target_frame",
        "candidate_frame",
        "target_objdiff_relocation_records",
        "candidate_objdiff_relocation_records",
        "verified_physical_relocations",
        "residual_count",
    ):
        normalized_stage[field] = _uint(
            stage.get(field), f"{label}.semantic_owner_stage.{field}", minimum=1
        )
    normalized_stage["match_percent"] = _number(
        stage.get("match_percent"), f"{label}.semantic_owner_stage.match_percent"
    )
    normalized_stage["diff_kind"] = _text(
        stage.get("diff_kind"), f"{label}.semantic_owner_stage.diff_kind"
    )
    for field in (
        "operation_order_exact",
        "cfg_calls_exact",
        "stack_homes_exact",
        "data_exact",
        "protected_siblings_preserved",
    ):
        normalized_stage[field] = _boolean(
            stage.get(field), f"{label}.semantic_owner_stage.{field}", True
        )
    if not (
        normalized_stage["target_size"] == normalized_stage["candidate_size"]
        == baseline_values["target_size"]
        and normalized_stage["target_frame"] == normalized_stage["candidate_frame"]
        == baseline_values["target_frame"]
        and normalized_stage["target_objdiff_relocation_records"]
        == normalized_stage["candidate_objdiff_relocation_records"]
        == baseline_values["target_objdiff_relocation_records"]
        and normalized_stage["verified_physical_relocations"]
        == baseline_values["verified_physical_relocations"]
        and normalized_stage["match_percent"] < 100.0
        and normalized_stage["residual_count"] == 13
        and normalized_stage["diff_kind"] == "DIFF_ARG_MISMATCH"
    ):
        raise SavedFprSemanticOwnerInputError(
            f"{label}.semantic_owner_stage must be the sealed exact-structure 13-row precursor"
        )

    chronology = _closed(
        root.get("chronology"),
        fields={"declaration_order", "target_fpr_map", "all_owners_live", "unknown_owners_present"},
        label=f"{label}.chronology",
    )
    raw_map = _closed(
        chronology.get("target_fpr_map"),
        fields=set(TARGET_FPR_MAP),
        label=f"{label}.chronology.target_fpr_map",
    )
    normalized_map = {
        key: _text(raw_map.get(key), f"{label}.chronology.target_fpr_map.{key}")
        for key in TARGET_FPR_MAP
    }
    if normalized_map != TARGET_FPR_MAP:
        raise SavedFprSemanticOwnerInputError(f"{label}.chronology.target_fpr_map drifted")
    normalized_chronology = {
        "declaration_order": _exact_sequence(
            chronology.get("declaration_order"),
            DECLARATION_ORDER,
            f"{label}.chronology.declaration_order",
        ),
        "target_fpr_map": normalized_map,
        "all_owners_live": _boolean(
            chronology.get("all_owners_live"), f"{label}.chronology.all_owners_live", True
        ),
        "unknown_owners_present": _boolean(
            chronology.get("unknown_owners_present"),
            f"{label}.chronology.unknown_owners_present",
            False,
        ),
    }

    exact = _closed(
        root.get("exact_result"),
        fields={
            "objdiff_canonical_sha256",
            "target_size",
            "candidate_size",
            "physical_relocations",
            "objdiff_relocation_records",
            "match_percent",
            "artifact",
        },
        label=f"{label}.exact_result",
    )
    normalized_exact = {
        "objdiff_canonical_sha256": _sha(
            exact.get("objdiff_canonical_sha256"),
            f"{label}.exact_result.objdiff_canonical_sha256",
        ),
        "target_size": _uint(exact.get("target_size"), f"{label}.exact_result.target_size", minimum=1),
        "candidate_size": _uint(
            exact.get("candidate_size"), f"{label}.exact_result.candidate_size", minimum=1
        ),
        "physical_relocations": _uint(
            exact.get("physical_relocations"),
            f"{label}.exact_result.physical_relocations",
            minimum=1,
        ),
        "objdiff_relocation_records": _uint(
            exact.get("objdiff_relocation_records"),
            f"{label}.exact_result.objdiff_relocation_records",
            minimum=1,
        ),
        "match_percent": _number(
            exact.get("match_percent"), f"{label}.exact_result.match_percent"
        ),
        "artifact": _artifact(
            exact.get("artifact"), f"{label}.exact_result.artifact", exact=True
        ),
    }
    if not (
        normalized_exact["target_size"] == normalized_exact["candidate_size"]
        == baseline_values["target_size"]
        and normalized_exact["physical_relocations"]
        == baseline_values["verified_physical_relocations"]
        and normalized_exact["objdiff_relocation_records"]
        == baseline_values["target_objdiff_relocation_records"]
        and normalized_exact["match_percent"] == 100.0
    ):
        raise SavedFprSemanticOwnerInputError(f"{label}.exact_result drifted")

    raw_controls = root.get("negative_controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != 2:
        raise SavedFprSemanticOwnerInputError(f"{label}.negative_controls must contain two controls")
    controls: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_controls):
        item_label = f"{label}.negative_controls[{index}]"
        item = _closed(
            raw,
            fields={"candidate_id", "axis", "result", "measured", "match_percent"},
            label=item_label,
        )
        controls.append(
            {
                "candidate_id": _owner(item.get("candidate_id"), f"{item_label}.candidate_id"),
                "axis": _text(item.get("axis"), f"{item_label}.axis"),
                "result": _text(item.get("result"), f"{item_label}.result"),
                "measured": _boolean(item.get("measured"), f"{item_label}.measured", True),
                "match_percent": _number(
                    item.get("match_percent"), f"{item_label}.match_percent"
                ),
            }
        )
    if [item["axis"] for item in controls] != [
        "typed_u8_alpha_local",
        "block_scoped_initializers",
    ]:
        raise SavedFprSemanticOwnerInputError(f"{label}.negative_controls axes drifted")
    if any(item["match_percent"] >= normalized_stage["match_percent"] for item in controls):
        raise SavedFprSemanticOwnerInputError(
            f"{label}.negative_controls must precede the exact-structure stage"
        )

    telemetry = _closed(
        root.get("telemetry"),
        fields={
            "parent_active_seconds",
            "helper_active_seconds_sum",
            "team_active_seconds_sum",
            "active_wall_union_seconds",
            "heavy_seconds",
            "candidate_count",
            "tracer_runs",
            "donor_searches",
            "telemetry_complete",
            "eligible_for_measured_crack_per_hour",
            "no_imputation",
            "uncovered_start_utc",
            "uncovered_end_utc",
            "uncovered_seconds",
            "telemetry_sha256",
            "active_interval_log_sha256",
        },
        label=f"{label}.telemetry",
    )
    normalized_telemetry = {
        field: _number(telemetry.get(field), f"{label}.telemetry.{field}")
        for field in (
            "parent_active_seconds",
            "helper_active_seconds_sum",
            "team_active_seconds_sum",
            "active_wall_union_seconds",
            "heavy_seconds",
            "uncovered_seconds",
        )
    }
    for field in ("candidate_count", "tracer_runs", "donor_searches"):
        normalized_telemetry[field] = _uint(
            telemetry.get(field), f"{label}.telemetry.{field}"
        )
    normalized_telemetry.update(
        {
            "telemetry_complete": _boolean(
                telemetry.get("telemetry_complete"),
                f"{label}.telemetry.telemetry_complete",
                False,
            ),
            "eligible_for_measured_crack_per_hour": _boolean(
                telemetry.get("eligible_for_measured_crack_per_hour"),
                f"{label}.telemetry.eligible_for_measured_crack_per_hour",
                False,
            ),
            "no_imputation": _boolean(
                telemetry.get("no_imputation"),
                f"{label}.telemetry.no_imputation",
                True,
            ),
            "uncovered_start_utc": _text(
                telemetry.get("uncovered_start_utc"),
                f"{label}.telemetry.uncovered_start_utc",
            ),
            "uncovered_end_utc": _text(
                telemetry.get("uncovered_end_utc"),
                f"{label}.telemetry.uncovered_end_utc",
            ),
            "telemetry_sha256": _sha(
                telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"
            ),
            "active_interval_log_sha256": _sha(
                telemetry.get("active_interval_log_sha256"),
                f"{label}.telemetry.active_interval_log_sha256",
            ),
        }
    )
    if (
        normalized_telemetry["candidate_count"] != 4
        or normalized_telemetry["tracer_runs"] != 0
        or normalized_telemetry["donor_searches"] != 1
        or normalized_telemetry["uncovered_seconds"] <= 0.0
    ):
        raise SavedFprSemanticOwnerInputError(f"{label}.telemetry campaign counts drifted")

    return {
        "schema": CONTEXT_SCHEMA,
        "owner": owner,
        "function": function,
        "source_owner_task": source_owner_task,
        "authority_advanced": False,
        "report": normalized_report,
        "provenance": normalized_provenance,
        "donors": donors,
        "baseline": baseline_values,
        "semantic_owner_stage": normalized_stage,
        "chronology": normalized_chronology,
        "exact_result": normalized_exact,
        "negative_controls": controls,
        "telemetry": normalized_telemetry,
        "forbidden_axes": _exact_sequence(
            root.get("forbidden_axes"), FORBIDDEN_AXES, f"{label}.forbidden_axes"
        ),
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:16]:
        if instruction.has_instruction:
            match = _FRAME_RE.fullmatch(instruction.formatted)
            if match is not None:
                return int(match.group("size"), 0)
    return None


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _size(symbol: Mapping[str, Any] | None) -> int | None:
    return causal_reducer._parse_number(symbol.get("size")) if symbol else None


def _match(symbol: Mapping[str, Any] | None) -> float | None:
    if symbol is None:
        return None
    value = symbol.get("match_percent")
    return float(value) if isinstance(value, (int, float)) and not isinstance(value, bool) else None


def _signature(
    pair: causal_reducer.FunctionPair,
    target: Sequence[Any],
    candidate: Sequence[Any],
) -> tuple[Any, ...]:
    return (
        _size(pair.target),
        _size(pair.candidate),
        _frame_size(target),
        _frame_size(candidate),
        _physical_relocations(target),
        _physical_relocations(candidate),
        _match(pair.candidate),
    )


def _close(left: Any, right: Any) -> bool:
    if isinstance(left, float) and isinstance(right, float):
        return math.isclose(left, right, rel_tol=0.0, abs_tol=1e-6)
    return left == right


def _mismatch_pairs(target: Sequence[Any], candidate: Sequence[Any]) -> list[tuple[int, Any, Any]]:
    return [
        (index, left, right)
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if causal_reducer._instruction_mismatch(left, right)
    ]


def _sda_owner_only_difference(left: Any, right: Any) -> bool:
    if (
        left is None
        or right is None
        or not left.has_instruction
        or not right.has_instruction
        or left.mnemonic != right.mnemonic
        or not _SDA_OWNER_RE.search(left.formatted)
        or not _SDA_OWNER_RE.search(right.formatted)
    ):
        return False
    return _SDA_OWNER_RE.sub("<owner>@sda21", left.formatted) == _SDA_OWNER_RE.sub(
        "<owner>@sda21", right.formatted
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
            "reason": "no authenticated saved-FPR semantic-owner chronology context was supplied",
        }
    if pair.name != context["function"]:
        return {
            "matched": False,
            "reason": "the saved-FPR semantic-owner context is bound to another function",
        }

    baseline = context["baseline"]
    observed = _signature(pair, target, candidate)
    sealed_baseline = (
        baseline["target_size"],
        baseline["candidate_size"],
        baseline["target_frame"],
        baseline["candidate_frame"],
        baseline["target_objdiff_relocation_records"],
        baseline["candidate_objdiff_relocation_records"],
        baseline["match_percent"],
    )
    if all(_close(left, right) for left, right in zip(observed, sealed_baseline)):
        return {
            "matched": True,
            "reason": "the size and saved-FPR frame deficit matches one authenticated semantic-owner family",
            "evidence": {
                "stage": "baseline_to_semantic_owner_structure",
                "observed_signature": list(observed),
                "missing_semantic_owner_count": baseline["missing_semantic_owner_count"],
                "recommended_cells": [
                    {
                        "kind": "complete_saved_fpr_semantic_owner_family",
                        "components": list(baseline["recommended_components"]),
                        "donors": [item["function"] for item in context["donors"]],
                    }
                ],
                "suppress_tracer": True,
                "negative_controls": context["negative_controls"],
                "telemetry": context["telemetry"],
                "forbidden_axes": context["forbidden_axes"],
                "authority_advanced": False,
            },
        }

    stage = context["semantic_owner_stage"]
    if objdiff_canonical_sha256 == stage["objdiff_canonical_sha256"]:
        sealed_stage = (
            stage["target_size"],
            stage["candidate_size"],
            stage["target_frame"],
            stage["candidate_frame"],
            stage["target_objdiff_relocation_records"],
            stage["candidate_objdiff_relocation_records"],
            stage["match_percent"],
        )
        if not all(_close(left, right) for left, right in zip(observed, sealed_stage)):
            return {
                "matched": False,
                "reason": "the sealed semantic-owner precursor topology drifted",
                "evidence": {"observed_signature": list(observed), "sealed_signature": list(sealed_stage)},
            }
        raw_mismatches = _mismatch_pairs(target, candidate)
        mismatches = [
            row for row in raw_mismatches if not _sda_owner_only_difference(row[1], row[2])
        ]
        owner_only_count = len(raw_mismatches) - len(mismatches)
        if len(mismatches) != stage["residual_count"]:
            return {
                "matched": False,
                "reason": "the residual is not the sealed thirteen-row saved-FPR cycle",
                "evidence": {
                    "observed_count": len(mismatches),
                    "value_equivalent_sda_owner_rows": owner_only_count,
                },
            }
        rows: list[dict[str, Any]] = []
        for index, left, right in mismatches:
            if (
                left is None
                or right is None
                or not left.has_instruction
                or not right.has_instruction
                or left.diff_kind != stage["diff_kind"]
                or right.diff_kind != stage["diff_kind"]
                or left.mnemonic != right.mnemonic
                or _FPR_RE.sub("f#", left.formatted)
                != _FPR_RE.sub("f#", right.formatted)
                or not _FPR_RE.search(left.formatted)
                or not _FPR_RE.search(right.formatted)
            ):
                return {
                    "matched": False,
                    "reason": "the closed residual contains partial, UNKNOWN, or non-FPR topology drift",
                    "evidence": {"row": index},
                }
            rows.append({"row": index, "target": left.formatted, "candidate": right.formatted})
        chronology = context["chronology"]
        return {
            "matched": True,
            "reason": "size, frame, CFG, calls, homes, data, and relocations are exact; the only residual is one complete live saved-FPR permutation",
            "evidence": {
                "stage": "closed_structure_to_live_declaration_chronology",
                "residual_rows": rows,
                "value_equivalent_sda_owner_rows": owner_only_count,
                "recommended_cells": [
                    {
                        "kind": "exact_live_saved_fpr_declaration_chronology",
                        "declaration_order": chronology["declaration_order"],
                        "target_fpr_map": chronology["target_fpr_map"],
                    }
                ],
                "suppress_tracer": True,
                "telemetry": context["telemetry"],
                "forbidden_axes": context["forbidden_axes"],
                "authority_advanced": False,
            },
        }

    exact = context["exact_result"]
    if objdiff_canonical_sha256 == exact["objdiff_canonical_sha256"]:
        raw_mismatches = _mismatch_pairs(target, candidate)
        residuals = [
            row for row in raw_mismatches if not _sda_owner_only_difference(row[1], row[2])
        ]
        if residuals:
            return {
                "matched": False,
                "reason": "the sealed exact result now has code residual rows",
                "evidence": {
                    "observed_count": len(residuals),
                    "value_equivalent_sda_owner_rows": len(raw_mismatches) - len(residuals),
                },
            }
        return {
            "matched": False,
            "reason": "the function is already exact; no candidate is scheduled",
            "evidence": {
                "value_equivalent_sda_owner_rows": len(raw_mismatches),
                "telemetry": context["telemetry"],
            },
        }

    return {
        "matched": False,
        "reason": "the report matches neither the sealed baseline signature nor an authenticated campaign stage",
    }
