#!/usr/bin/env python3
"""Fail-closed same-TU exact constructor-family transfer diagnosis."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "same_tu_constructor_family_transfer_context/v1"
RULE_ID = "same_tu_constructor_family_transfer"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FRAME_RE = re.compile(r"stwu\s+r1,\s*-(?P<size>0x[0-9A-Fa-f]+|[0-9]+)\(r1\)")
_FPR_RE = re.compile(r"\bf(?:[0-9]|[12][0-9]|3[01])\b")
_SDA_OWNER_RE = re.compile(r"[^,\s]+@sda21\b")

FAMILY_COMPONENTS = (
    "named_scalar_trig_results",
    "per_call_typed_aggregate_snapshots",
    "typed_pointer_address_consumers",
    "live_integer_result_stores",
)
SEMANTIC_ORDER_COMPONENTS = (
    "distinct_shared_rand_owner",
    "aggregate_assignment_vel_before_pos",
    "direction_store_x_z_y",
)
DECLARATION_ORDER = (
    "distance",
    "posCos",
    "posSin",
    "velCos",
    "velSin",
    "value",
    "active",
    "fadeStep",
    "halfDistance",
    "randF",
)
TARGET_FPR_MAP = {
    "distance": "f25",
    "posCos": "f24",
    "posSin": "f23",
    "velCos": "f22",
    "velSin": "f21",
    "active": "f27",
    "fadeStep": "f26",
}
FORBIDDEN_AXES = (
    "isolated_declaration_permutations_before_structure",
    "scope_permutations",
    "dead_or_fake_locals",
    "padding",
    "register_shaping",
    "unauthenticated_donor",
    "tracer_before_static_closure",
    "source_retention",
    "promotion",
)


class ConstructorFamilyInputError(ValueError):
    """The supplied evidence cannot safely support this diagnosis."""


def _closed(value: Any, *, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ConstructorFamilyInputError(f"{label} must be a JSON object")
    missing = fields - set(value)
    extra = set(value) - fields
    if missing or extra:
        raise ConstructorFamilyInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ConstructorFamilyInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise ConstructorFamilyInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise ConstructorFamilyInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise ConstructorFamilyInputError(f"{label} has invalid characters")
    return result


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise ConstructorFamilyInputError(f"{label} must be lowercase SHA-256")
    return value


def _boolean(value: Any, label: str, expected: bool) -> bool:
    if type(value) is not bool or value is not expected:
        raise ConstructorFamilyInputError(f"{label} must be {expected}")
    return expected


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if type(value) is not int or value < minimum or value > maximum:
        raise ConstructorFamilyInputError(
            f"{label} must be an integer in [{minimum}, {maximum}]"
        )
    return value


def _number(value: Any, label: str, *, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ConstructorFamilyInputError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise ConstructorFamilyInputError(f"{label} is outside the accepted range")
    return result


def _exact_sequence(value: Any, expected: Sequence[str], label: str) -> list[str]:
    if not isinstance(value, list):
        raise ConstructorFamilyInputError(f"{label} must be an array")
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if result != list(expected):
        raise ConstructorFamilyInputError(f"{label} must equal the sealed sequence")
    return result


def _artifact(value: Any, label: str, *, include_report: bool = False) -> dict[str, Any]:
    fields = {"source_sha256", "object_sha256", "attestation_sha256"}
    if include_report:
        fields |= {"strict_report_sha256", "data_report_sha256"}
    item = _closed(value, fields=fields, label=label)
    result = {field: _sha(item.get(field), f"{label}.{field}") for field in fields}
    return result


def _stage(value: Any, label: str, *, cell: Sequence[str]) -> dict[str, Any]:
    item = _closed(
        value,
        fields={
            "objdiff_canonical_sha256",
            "candidate_size",
            "candidate_frame",
            "physical_relocations",
            "match_percent",
            "artifact",
            "cell",
        },
        label=label,
    )
    return {
        "objdiff_canonical_sha256": _sha(
            item.get("objdiff_canonical_sha256"), f"{label}.objdiff_canonical_sha256"
        ),
        "candidate_size": _uint(item.get("candidate_size"), f"{label}.candidate_size", minimum=4),
        "candidate_frame": _uint(item.get("candidate_frame"), f"{label}.candidate_frame", minimum=16),
        "physical_relocations": _uint(
            item.get("physical_relocations"), f"{label}.physical_relocations", minimum=1
        ),
        "match_percent": _number(item.get("match_percent"), f"{label}.match_percent"),
        "artifact": _artifact(item.get("artifact"), f"{label}.artifact"),
        "cell": _exact_sequence(item.get("cell"), cell, f"{label}.cell"),
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "same_tu_constructor_family_context"
    root = _closed(
        value,
        fields={
            "schema",
            "owner",
            "function",
            "authority_advanced",
            "report",
            "provenance",
            "donor",
            "baseline",
            "stages",
            "telemetry",
            "forbidden_axes",
        },
        label=label,
    )
    if _text(root.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise ConstructorFamilyInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    owner = _owner(root.get("owner"), f"{label}.owner")
    function = _identifier(root.get("function"), f"{label}.function")
    _boolean(root.get("authority_advanced"), f"{label}.authority_advanced", False)

    report = _closed(
        root.get("report"),
        fields={"report_sha256", "base_commit", "target_object_sha256"},
        label=f"{label}.report",
    )
    normalized_report = {
        field: _sha(report.get(field), f"{label}.report.{field}")
        for field in ("report_sha256", "base_commit", "target_object_sha256")
    }

    provenance = _closed(
        root.get("provenance"),
        fields={
            "graphify_location",
            "graphify_bound",
            "graft_ask_count",
            "graft_status",
            "narrow_named_file_verified",
        },
        label=f"{label}.provenance",
    )
    normalized_provenance = {
        "graphify_location": _text(
            provenance.get("graphify_location"), f"{label}.provenance.graphify_location"
        ),
        "graphify_bound": _boolean(
            provenance.get("graphify_bound"), f"{label}.provenance.graphify_bound", True
        ),
        "graft_ask_count": _uint(
            provenance.get("graft_ask_count"), f"{label}.provenance.graft_ask_count", minimum=1, maximum=1
        ),
        "graft_status": _text(provenance.get("graft_status"), f"{label}.provenance.graft_status"),
        "narrow_named_file_verified": _boolean(
            provenance.get("narrow_named_file_verified"),
            f"{label}.provenance.narrow_named_file_verified",
            True,
        ),
    }
    if normalized_provenance["graft_status"] != "no_nodes":
        raise ConstructorFamilyInputError(f"{label}.provenance.graft_status must be no_nodes")

    donor = _closed(
        root.get("donor"),
        fields={
            "function",
            "same_translation_unit",
            "strict_exact",
            "data_exact",
            "target_size",
            "candidate_size",
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "family_components",
        },
        label=f"{label}.donor",
    )
    donor_function = _identifier(donor.get("function"), f"{label}.donor.function")
    if donor_function == function:
        raise ConstructorFamilyInputError(f"{label}.donor.function must differ from focus")
    donor_target = _uint(donor.get("target_size"), f"{label}.donor.target_size", minimum=4)
    donor_candidate = _uint(donor.get("candidate_size"), f"{label}.donor.candidate_size", minimum=4)
    if donor_target != donor_candidate:
        raise ConstructorFamilyInputError(f"{label}.donor must be exact-size")
    normalized_donor = {
        "function": donor_function,
        "same_translation_unit": _boolean(
            donor.get("same_translation_unit"), f"{label}.donor.same_translation_unit", True
        ),
        "strict_exact": _boolean(donor.get("strict_exact"), f"{label}.donor.strict_exact", True),
        "data_exact": _boolean(donor.get("data_exact"), f"{label}.donor.data_exact", True),
        "target_size": donor_target,
        "candidate_size": donor_candidate,
        "source_sha256": _sha(donor.get("source_sha256"), f"{label}.donor.source_sha256"),
        "object_sha256": _sha(donor.get("object_sha256"), f"{label}.donor.object_sha256"),
        "strict_report_sha256": _sha(
            donor.get("strict_report_sha256"), f"{label}.donor.strict_report_sha256"
        ),
        "family_components": _exact_sequence(
            donor.get("family_components"), FAMILY_COMPONENTS, f"{label}.donor.family_components"
        ),
    }

    baseline = _closed(
        root.get("baseline"),
        fields={
            "target_size",
            "candidate_size",
            "target_frame",
            "candidate_frame",
            "target_relocations",
            "candidate_relocations",
            "match_percent",
        },
        label=f"{label}.baseline",
    )
    normalized_baseline = {
        field: _uint(baseline.get(field), f"{label}.baseline.{field}", minimum=1)
        for field in (
            "target_size",
            "candidate_size",
            "target_frame",
            "candidate_frame",
            "target_relocations",
            "candidate_relocations",
        )
    }
    normalized_baseline["match_percent"] = _number(
        baseline.get("match_percent"), f"{label}.baseline.match_percent"
    )
    if not (
        normalized_baseline["candidate_size"] < normalized_baseline["target_size"]
        and normalized_baseline["candidate_frame"] < normalized_baseline["target_frame"]
        and normalized_baseline["candidate_relocations"] < normalized_baseline["target_relocations"]
        and normalized_baseline["match_percent"] < 100.0
    ):
        raise ConstructorFamilyInputError(f"{label}.baseline must preserve the sealed deficit")

    stages = _closed(
        root.get("stages"),
        fields={"donor_family", "semantic_order", "donor_chronology", "exact"},
        label=f"{label}.stages",
    )
    donor_family = _stage(
        stages.get("donor_family"),
        f"{label}.stages.donor_family",
        cell=FAMILY_COMPONENTS,
    )
    semantic_order = _stage(
        stages.get("semantic_order"),
        f"{label}.stages.semantic_order",
        cell=SEMANTIC_ORDER_COMPONENTS,
    )
    chronology = _closed(
        stages.get("donor_chronology"),
        fields={"objdiff_canonical_sha256", "residual_count", "diff_kind", "declaration_order", "target_fpr_map"},
        label=f"{label}.stages.donor_chronology",
    )
    fpr_map = _closed(
        chronology.get("target_fpr_map"),
        fields=set(TARGET_FPR_MAP),
        label=f"{label}.stages.donor_chronology.target_fpr_map",
    )
    normalized_fpr = {
        name: _text(fpr_map.get(name), f"{label}.stages.donor_chronology.target_fpr_map.{name}")
        for name in TARGET_FPR_MAP
    }
    if normalized_fpr != TARGET_FPR_MAP:
        raise ConstructorFamilyInputError(f"{label}.stages.donor_chronology.target_fpr_map drifted")
    normalized_chronology = {
        "objdiff_canonical_sha256": _sha(
            chronology.get("objdiff_canonical_sha256"),
            f"{label}.stages.donor_chronology.objdiff_canonical_sha256",
        ),
        "residual_count": _uint(
            chronology.get("residual_count"), f"{label}.stages.donor_chronology.residual_count", minimum=1
        ),
        "diff_kind": _text(chronology.get("diff_kind"), f"{label}.stages.donor_chronology.diff_kind"),
        "declaration_order": _exact_sequence(
            chronology.get("declaration_order"), DECLARATION_ORDER,
            f"{label}.stages.donor_chronology.declaration_order",
        ),
        "target_fpr_map": normalized_fpr,
    }
    if normalized_chronology["diff_kind"] != "DIFF_ARG_MISMATCH":
        raise ConstructorFamilyInputError(f"{label}.stages.donor_chronology.diff_kind drifted")

    exact = _closed(
        stages.get("exact"),
        fields={
            "objdiff_canonical_sha256",
            "target_size",
            "candidate_size",
            "physical_relocations",
            "match_percent",
            "artifact",
            "candidate_record_sha256",
        },
        label=f"{label}.stages.exact",
    )
    exact_target = _uint(exact.get("target_size"), f"{label}.stages.exact.target_size", minimum=4)
    exact_candidate = _uint(exact.get("candidate_size"), f"{label}.stages.exact.candidate_size", minimum=4)
    exact_reloc = _uint(
        exact.get("physical_relocations"), f"{label}.stages.exact.physical_relocations", minimum=1
    )
    if (
        exact_target != exact_candidate
        or exact_target != normalized_baseline["target_size"]
        or exact_reloc != normalized_baseline["target_relocations"]
        or _number(exact.get("match_percent"), f"{label}.stages.exact.match_percent") != 100.0
    ):
        raise ConstructorFamilyInputError(f"{label}.stages.exact must close size, score, and relocations")
    normalized_exact = {
        "objdiff_canonical_sha256": _sha(
            exact.get("objdiff_canonical_sha256"), f"{label}.stages.exact.objdiff_canonical_sha256"
        ),
        "target_size": exact_target,
        "candidate_size": exact_candidate,
        "physical_relocations": exact_reloc,
        "match_percent": 100.0,
        "artifact": _artifact(exact.get("artifact"), f"{label}.stages.exact.artifact", include_report=True),
        "candidate_record_sha256": _sha(
            exact.get("candidate_record_sha256"), f"{label}.stages.exact.candidate_record_sha256"
        ),
    }

    if donor_family["candidate_size"] >= semantic_order["candidate_size"]:
        raise ConstructorFamilyInputError(
            f"{label}.stages.donor_family must remain smaller than semantic_order"
        )
    if semantic_order["candidate_size"] != normalized_exact["candidate_size"]:
        raise ConstructorFamilyInputError(f"{label}.stages.semantic_order must close function size")
    if semantic_order["physical_relocations"] != normalized_exact["physical_relocations"]:
        raise ConstructorFamilyInputError(f"{label}.stages.semantic_order must close relocations")

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
        )
    }
    for field in ("candidate_count", "tracer_runs", "donor_searches"):
        normalized_telemetry[field] = _uint(telemetry.get(field), f"{label}.telemetry.{field}")
    normalized_telemetry.update(
        {
            "telemetry_complete": _boolean(
                telemetry.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete", True
            ),
            "eligible_for_measured_crack_per_hour": _boolean(
                telemetry.get("eligible_for_measured_crack_per_hour"),
                f"{label}.telemetry.eligible_for_measured_crack_per_hour",
                True,
            ),
            "no_imputation": _boolean(
                telemetry.get("no_imputation"), f"{label}.telemetry.no_imputation", True
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
    if normalized_telemetry["tracer_runs"] != 0 or normalized_telemetry["candidate_count"] != 3:
        raise ConstructorFamilyInputError(f"{label}.telemetry must preserve the measured three-cell static path")

    return {
        "schema": CONTEXT_SCHEMA,
        "owner": owner,
        "function": function,
        "authority_advanced": False,
        "report": normalized_report,
        "provenance": normalized_provenance,
        "donor": normalized_donor,
        "baseline": normalized_baseline,
        "stages": {
            "donor_family": donor_family,
            "semantic_order": semantic_order,
            "donor_chronology": normalized_chronology,
            "exact": normalized_exact,
        },
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


def _size(symbol: Mapping[str, Any] | None) -> int | None:
    return causal_reducer._parse_number(symbol.get("size")) if symbol else None


def _match(symbol: Mapping[str, Any] | None) -> float | None:
    if symbol is None:
        return None
    value = symbol.get("match_percent")
    return float(value) if isinstance(value, (int, float)) and not isinstance(value, bool) else None


def _signature(pair: causal_reducer.FunctionPair, target: Sequence[Any], candidate: Sequence[Any]) -> tuple[Any, ...]:
    return (
        _size(pair.target),
        _size(pair.candidate),
        _frame_size(target),
        _frame_size(candidate),
        _physical_relocations(target),
        _physical_relocations(candidate),
        _match(pair.candidate),
    )


def _stage_signature(target_size: int, stage: Mapping[str, Any]) -> tuple[Any, ...]:
    return (
        target_size,
        stage["candidate_size"],
        None,
        stage["candidate_frame"],
        stage["physical_relocations"],
        stage["physical_relocations"],
        stage["match_percent"],
    )


def _close_float(left: Any, right: Any) -> bool:
    if isinstance(left, float) and isinstance(right, float):
        return math.isclose(left, right, rel_tol=0.0, abs_tol=1e-6)
    return left == right


def _signature_matches(observed: tuple[Any, ...], sealed: tuple[Any, ...], *, target_frame: int) -> bool:
    expected = list(sealed)
    expected[2] = target_frame
    return all(_close_float(left, right) for left, right in zip(observed, expected))


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
            "reason": "no authenticated same-TU constructor-family context was supplied",
        }
    if pair.name != context["function"]:
        return {"matched": False, "reason": "the constructor-family context is bound to another function"}

    baseline = context["baseline"]
    observed = _signature(pair, target, candidate)
    baseline_signature = (
        baseline["target_size"],
        baseline["candidate_size"],
        baseline["target_frame"],
        baseline["candidate_frame"],
        baseline["target_relocations"],
        baseline["candidate_relocations"],
        baseline["match_percent"],
    )
    if all(_close_float(left, right) for left, right in zip(observed, baseline_signature)):
        cell = {
            "kind": "complete_same_tu_constructor_family_transfer",
            "components": list(context["donor"]["family_components"]),
            "donor": context["donor"]["function"],
        }
        return {
            "matched": True,
            "reason": "the baseline frame and relocation deficit matches one authenticated exact same-TU constructor-family transfer",
            "evidence": {
                "stage": "baseline_to_donor_family",
                "observed_signature": list(observed),
                "recommended_cells": [cell],
                "suppress_tracer": True,
                "forbidden_axes": context["forbidden_axes"],
                "telemetry": context["telemetry"],
                "authority_advanced": False,
            },
        }

    stages = context["stages"]
    donor_stage = stages["donor_family"]
    if objdiff_canonical_sha256 == donor_stage["objdiff_canonical_sha256"]:
        sealed = _stage_signature(baseline["target_size"], donor_stage)
        if not _signature_matches(observed, sealed, target_frame=baseline["target_frame"]):
            return {
                "matched": False,
                "reason": "the donor-family precursor topology drifted",
                "evidence": {"observed_signature": list(observed), "sealed_signature": list(sealed)},
            }
        cell = {
            "kind": "constructor_family_semantic_order_closure",
            "components": list(stages["semantic_order"]["cell"]),
        }
        return {
            "matched": True,
            "reason": "the complete donor family is present; close only the measured shared-random and aggregate-order seams",
            "evidence": {
                "stage": "donor_family_to_closed_structure",
                "recommended_cells": [cell],
                "suppress_tracer": True,
                "forbidden_axes": context["forbidden_axes"],
                "authority_advanced": False,
            },
        }

    semantic_stage = stages["semantic_order"]
    if objdiff_canonical_sha256 == semantic_stage["objdiff_canonical_sha256"]:
        sealed = _stage_signature(baseline["target_size"], semantic_stage)
        if not _signature_matches(observed, sealed, target_frame=baseline["target_frame"]):
            return {
                "matched": False,
                "reason": "the closed-structure precursor topology drifted",
                "evidence": {"observed_signature": list(observed), "sealed_signature": list(sealed)},
            }
        raw_mismatches = _mismatch_pairs(target, candidate)
        mismatches = [
            row
            for row in raw_mismatches
            if not _sda_owner_only_difference(row[1], row[2])
        ]
        owner_only_count = len(raw_mismatches) - len(mismatches)
        chronology = stages["donor_chronology"]
        if len(mismatches) != chronology["residual_count"]:
            return {
                "matched": False,
                "reason": "the residual is not the sealed sixteen-row owner cycle",
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
                or left.mnemonic != right.mnemonic
                or _FPR_RE.sub("f#", left.formatted) != _FPR_RE.sub("f#", right.formatted)
                or not _FPR_RE.search(left.formatted)
                or not _FPR_RE.search(right.formatted)
            ):
                return {
                    "matched": False,
                    "reason": "the closed residual contains non-FPR or non-register topology drift",
                    "evidence": {"row": index},
                }
            rows.append({"row": index, "target": left.formatted, "candidate": right.formatted})
        cell = {
            "kind": "exact_donor_scalar_declaration_chronology",
            "declaration_order": chronology["declaration_order"],
            "target_fpr_map": chronology["target_fpr_map"],
        }
        return {
            "matched": True,
            "reason": "size, frame, CFG, data, and relocations are closed; the only residual is one authenticated donor-order FPR cycle",
            "evidence": {
                "stage": "closed_structure_to_donor_chronology",
                "residual_rows": rows,
                "value_equivalent_sda_owner_rows": owner_only_count,
                "recommended_cells": [cell],
                "suppress_tracer": True,
                "forbidden_axes": context["forbidden_axes"],
                "authority_advanced": False,
            },
        }

    exact = stages["exact"]
    if objdiff_canonical_sha256 == exact["objdiff_canonical_sha256"]:
        raw_mismatches = _mismatch_pairs(target, candidate)
        residuals = [
            row
            for row in raw_mismatches
            if not _sda_owner_only_difference(row[1], row[2])
        ]
        if residuals:
            return {
                "matched": False,
                "reason": "the sealed exact result now has residual rows",
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
            },
        }

    return {
        "matched": False,
        "reason": "the report matches neither the sealed baseline signature nor an authenticated campaign stage",
    }
