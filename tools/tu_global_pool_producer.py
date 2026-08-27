#!/usr/bin/env python3
"""Fail-closed global TU pool-producer visibility and chronology diagnosis."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "tu_global_pool_producer_context/v1"
RULE_ID = "tu_global_pool_producer_linkage"

FUNCTIONS = ("mbev_CapMiracle", "ev_CapMiracleMasu")
REPORT_CONTRACTS = {
    "mbev_CapMiracle": (1256, 96),
    "ev_CapMiracleMasu": (6136, 397),
}
CONSUMER_CENSUS = (
    ("ev_CapKettouStart", 3570, "greater_equal_guard"),
    ("ev_CapKettouStart", 3573, "less_equal_guard"),
    ("ev_CapDonkeyStart", 4121, "angle_lerp_step"),
    ("ev_CapDonkeyStart", 4299, "first_phase_guard"),
    ("ev_CapDonkeyStart", 4300, "first_phase_subtract"),
    ("ev_CapDonkeyStart", 4318, "second_phase_guard"),
    ("ev_CapDonkeyStart", 4319, "second_phase_subtract"),
)
FORBIDDEN_AXES = (
    "local_static_owner",
    "synthetic_target_label_or_extern",
    "duplicate_literal_seeder",
    "literal_spelling_probe",
    "downstream_exact_body_edit",
    "storage_duration_permutation",
    "dead_or_fake_local",
    "padding",
    "register_shaping",
    "tracer_before_static_closure",
    "source_retention",
    "promotion",
)

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@#-]{1,192}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_SHA1_RE = re.compile(r"[0-9a-f]{40}")
_SDA_OWNER_RE = re.compile(r"[^,\s]+@sda21\b")


class TuGlobalPoolProducerInputError(ValueError):
    """The supplied evidence cannot safely support this diagnosis."""


def _closed(value: Any, *, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise TuGlobalPoolProducerInputError(f"{label} must be a JSON object")
    missing = fields - set(value)
    extra = set(value) - fields
    if missing or extra:
        raise TuGlobalPoolProducerInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise TuGlobalPoolProducerInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise TuGlobalPoolProducerInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise TuGlobalPoolProducerInputError(f"{label} must be a C identifier")
    return result


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(result) is None:
        raise TuGlobalPoolProducerInputError(f"{label} has invalid characters")
    return result


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise TuGlobalPoolProducerInputError(f"{label} must be lowercase SHA-256")
    return value


def _commit(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA1_RE.fullmatch(value) is None:
        raise TuGlobalPoolProducerInputError(f"{label} must be a lowercase 40-hex commit")
    return value


def _boolean(value: Any, label: str, expected: bool) -> bool:
    if type(value) is not bool or value is not expected:
        raise TuGlobalPoolProducerInputError(f"{label} must be {expected}")
    return expected


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if type(value) is not int or value < minimum or value > maximum:
        raise TuGlobalPoolProducerInputError(
            f"{label} must be an integer in [{minimum}, {maximum}]"
        )
    return value


def _number(value: Any, label: str, *, minimum: float = 0.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TuGlobalPoolProducerInputError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result) or result < minimum:
        raise TuGlobalPoolProducerInputError(f"{label} is outside the accepted range")
    return result


def _exact_sequence(value: Any, expected: Sequence[str], label: str) -> list[str]:
    if not isinstance(value, list):
        raise TuGlobalPoolProducerInputError(f"{label} must be an array")
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if result != list(expected):
        raise TuGlobalPoolProducerInputError(f"{label} must equal the sealed sequence")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "TU-global pool-producer context"
    root = _closed(
        value,
        fields={
            "schema", "owner", "functions", "source_owner_task",
            "authority_advanced", "reports", "toolchain", "provenance",
            "producer", "consumers", "static_control", "exact_result",
            "telemetry", "forbidden_axes",
        },
        label=label,
    )
    if _text(root.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise TuGlobalPoolProducerInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    owner = _owner(root.get("owner"), f"{label}.owner")
    functions = _exact_sequence(root.get("functions"), FUNCTIONS, f"{label}.functions")
    source_owner_task = _owner(root.get("source_owner_task"), f"{label}.source_owner_task")
    _boolean(root.get("authority_advanced"), f"{label}.authority_advanced", False)

    reports_raw = root.get("reports")
    if not isinstance(reports_raw, list) or len(reports_raw) != len(FUNCTIONS):
        raise TuGlobalPoolProducerInputError(f"{label}.reports must bind both focus reports")
    reports: list[dict[str, Any]] = []
    for index, expected_function in enumerate(FUNCTIONS):
        item_label = f"{label}.reports[{index}]"
        item = _closed(
            reports_raw[index],
            fields={
                "function", "report_sha256", "target_size", "candidate_size",
                "physical_relocations", "strict_exact", "data_exact",
            },
            label=item_label,
        )
        function = _identifier(item.get("function"), f"{item_label}.function")
        if function != expected_function:
            raise TuGlobalPoolProducerInputError(f"{item_label}.function order drifted")
        expected_size, expected_relocations = REPORT_CONTRACTS[function]
        target_size = _uint(item.get("target_size"), f"{item_label}.target_size", minimum=1)
        candidate_size = _uint(item.get("candidate_size"), f"{item_label}.candidate_size", minimum=1)
        relocations = _uint(
            item.get("physical_relocations"), f"{item_label}.physical_relocations", minimum=1
        )
        if (target_size, candidate_size, relocations) != (
            expected_size, expected_size, expected_relocations
        ):
            raise TuGlobalPoolProducerInputError(f"{item_label} exact proof contract drifted")
        reports.append(
            {
                "function": function,
                "report_sha256": _sha(item.get("report_sha256"), f"{item_label}.report_sha256"),
                "target_size": target_size,
                "candidate_size": candidate_size,
                "physical_relocations": relocations,
                "strict_exact": _boolean(item.get("strict_exact"), f"{item_label}.strict_exact", True),
                "data_exact": _boolean(item.get("data_exact"), f"{item_label}.data_exact", True),
            }
        )

    toolchain_raw = _closed(
        root.get("toolchain"),
        fields={"base_commit", "compiler_sha256", "wrapper_sha256", "target_object_sha256"},
        label=f"{label}.toolchain",
    )
    toolchain = {
        "base_commit": _commit(toolchain_raw.get("base_commit"), f"{label}.toolchain.base_commit"),
        "compiler_sha256": _sha(toolchain_raw.get("compiler_sha256"), f"{label}.toolchain.compiler_sha256"),
        "wrapper_sha256": _sha(toolchain_raw.get("wrapper_sha256"), f"{label}.toolchain.wrapper_sha256"),
        "target_object_sha256": _sha(toolchain_raw.get("target_object_sha256"), f"{label}.toolchain.target_object_sha256"),
    }

    provenance_raw = _closed(
        root.get("provenance"),
        fields={
            "graphify_status", "graphify_bound", "graft_ask_count", "graft_status",
            "narrow_named_file_verified", "broad_searches",
        },
        label=f"{label}.provenance",
    )
    provenance = {
        "graphify_status": _text(provenance_raw.get("graphify_status"), f"{label}.provenance.graphify_status"),
        "graphify_bound": _boolean(provenance_raw.get("graphify_bound"), f"{label}.provenance.graphify_bound", False),
        "graft_ask_count": _uint(provenance_raw.get("graft_ask_count"), f"{label}.provenance.graft_ask_count", minimum=1, maximum=1),
        "graft_status": _text(provenance_raw.get("graft_status"), f"{label}.provenance.graft_status"),
        "narrow_named_file_verified": _boolean(provenance_raw.get("narrow_named_file_verified"), f"{label}.provenance.narrow_named_file_verified", True),
        "broad_searches": _uint(provenance_raw.get("broad_searches"), f"{label}.provenance.broad_searches", maximum=0),
    }
    if provenance["graphify_status"] != "no_usable_graph" or provenance["graft_status"] != "no_nodes":
        raise TuGlobalPoolProducerInputError(f"{label}.provenance must preserve the fail-closed no-hit results")

    producer_raw = _closed(
        root.get("producer"),
        fields={
            "source_name", "c_type", "value", "target_symbol", "target_global",
            "source_linkage", "source_line", "before_function",
        },
        label=f"{label}.producer",
    )
    producer = {
        "source_name": _identifier(producer_raw.get("source_name"), f"{label}.producer.source_name"),
        "c_type": _text(producer_raw.get("c_type"), f"{label}.producer.c_type"),
        "value": _number(producer_raw.get("value"), f"{label}.producer.value"),
        "target_symbol": _owner(producer_raw.get("target_symbol"), f"{label}.producer.target_symbol"),
        "target_global": _boolean(producer_raw.get("target_global"), f"{label}.producer.target_global", True),
        "source_linkage": _text(producer_raw.get("source_linkage"), f"{label}.producer.source_linkage"),
        "source_line": _uint(producer_raw.get("source_line"), f"{label}.producer.source_line", minimum=1),
        "before_function": _identifier(producer_raw.get("before_function"), f"{label}.producer.before_function"),
    }
    if producer != {
        "source_name": "capspecialTen", "c_type": "const float", "value": 10.0,
        "target_symbol": "lbl_802C4370", "target_global": True,
        "source_linkage": "external", "source_line": 1511,
        "before_function": "mbev_CapMiracle",
    }:
        raise TuGlobalPoolProducerInputError(f"{label}.producer drifted from the sealed producer")

    consumers_raw = root.get("consumers")
    if not isinstance(consumers_raw, list) or len(consumers_raw) != len(CONSUMER_CENSUS):
        raise TuGlobalPoolProducerInputError(f"{label}.consumers must contain exactly seven sites")
    consumers: list[dict[str, Any]] = []
    for index, expected in enumerate(CONSUMER_CENSUS):
        item_label = f"{label}.consumers[{index}]"
        item = _closed(consumers_raw[index], fields={"function", "source_line", "role"}, label=item_label)
        normalized = {
            "function": _identifier(item.get("function"), f"{item_label}.function"),
            "source_line": _uint(item.get("source_line"), f"{item_label}.source_line", minimum=1),
            "role": _text(item.get("role"), f"{item_label}.role"),
        }
        if tuple(normalized[field] for field in ("function", "source_line", "role")) != expected:
            raise TuGlobalPoolProducerInputError(f"{item_label} drifted from the sealed census")
        consumers.append(normalized)

    control_raw = _closed(
        root.get("static_control"),
        fields={
            "candidate_id", "objdiff_canonical_sha256", "source_sha256", "object_sha256",
            "baseline_object_sha256", "object_identical_to_baseline", "consumer_count",
            "linkage", "strict_match_percent", "strict_diff_rows", "data_exact",
            "outcome", "compile_attestation_sha256", "candidate_record_sha256",
        },
        label=f"{label}.static_control",
    )
    control = {
        "candidate_id": _owner(control_raw.get("candidate_id"), f"{label}.static_control.candidate_id"),
        "objdiff_canonical_sha256": _sha(control_raw.get("objdiff_canonical_sha256"), f"{label}.static_control.objdiff_canonical_sha256"),
        "source_sha256": _sha(control_raw.get("source_sha256"), f"{label}.static_control.source_sha256"),
        "object_sha256": _sha(control_raw.get("object_sha256"), f"{label}.static_control.object_sha256"),
        "baseline_object_sha256": _sha(control_raw.get("baseline_object_sha256"), f"{label}.static_control.baseline_object_sha256"),
        "object_identical_to_baseline": _boolean(control_raw.get("object_identical_to_baseline"), f"{label}.static_control.object_identical_to_baseline", True),
        "consumer_count": _uint(control_raw.get("consumer_count"), f"{label}.static_control.consumer_count", minimum=7, maximum=7),
        "linkage": _text(control_raw.get("linkage"), f"{label}.static_control.linkage"),
        "strict_match_percent": _number(control_raw.get("strict_match_percent"), f"{label}.static_control.strict_match_percent"),
        "strict_diff_rows": _uint(control_raw.get("strict_diff_rows"), f"{label}.static_control.strict_diff_rows", minimum=1, maximum=1),
        "data_exact": _boolean(control_raw.get("data_exact"), f"{label}.static_control.data_exact", True),
        "outcome": _text(control_raw.get("outcome"), f"{label}.static_control.outcome"),
        "compile_attestation_sha256": _sha(control_raw.get("compile_attestation_sha256"), f"{label}.static_control.compile_attestation_sha256"),
        "candidate_record_sha256": _sha(control_raw.get("candidate_record_sha256"), f"{label}.static_control.candidate_record_sha256"),
    }
    if (
        control["object_sha256"] != control["baseline_object_sha256"]
        or control["linkage"] != "internal"
        or control["outcome"] != "rejected_object_neutral"
        or not math.isclose(control["strict_match_percent"], 99.98408, abs_tol=1e-6)
    ):
        raise TuGlobalPoolProducerInputError(f"{label}.static_control no longer proves local-static neutrality")

    exact_raw = _closed(
        root.get("exact_result"),
        fields={
            "candidate_id", "objdiff_canonical_sha256", "source_sha256", "object_sha256",
            "strict_report_sha256", "data_report_sha256", "compile_attestation_sha256",
            "candidate_record_sha256", "strict_data_exact", "protected_siblings",
        },
        label=f"{label}.exact_result",
    )
    exact = {
        "candidate_id": _owner(exact_raw.get("candidate_id"), f"{label}.exact_result.candidate_id"),
        "objdiff_canonical_sha256": _sha(exact_raw.get("objdiff_canonical_sha256"), f"{label}.exact_result.objdiff_canonical_sha256"),
        "source_sha256": _sha(exact_raw.get("source_sha256"), f"{label}.exact_result.source_sha256"),
        "object_sha256": _sha(exact_raw.get("object_sha256"), f"{label}.exact_result.object_sha256"),
        "strict_report_sha256": _sha(exact_raw.get("strict_report_sha256"), f"{label}.exact_result.strict_report_sha256"),
        "data_report_sha256": _sha(exact_raw.get("data_report_sha256"), f"{label}.exact_result.data_report_sha256"),
        "compile_attestation_sha256": _sha(exact_raw.get("compile_attestation_sha256"), f"{label}.exact_result.compile_attestation_sha256"),
        "candidate_record_sha256": _sha(exact_raw.get("candidate_record_sha256"), f"{label}.exact_result.candidate_record_sha256"),
        "strict_data_exact": _boolean(exact_raw.get("strict_data_exact"), f"{label}.exact_result.strict_data_exact", True),
        "protected_siblings": _text(exact_raw.get("protected_siblings"), f"{label}.exact_result.protected_siblings"),
    }
    if exact["protected_siblings"] != "31/31":
        raise TuGlobalPoolProducerInputError(f"{label}.exact_result protected siblings drifted")

    telemetry_raw = _closed(
        root.get("telemetry"),
        fields={
            "parent_active_seconds", "parent_heavy_seconds", "wait_seconds",
            "telemetry_complete", "current_campaign_eligible", "historical_denominator_allowed",
            "no_historical_imputation", "historical_gap_seconds", "telemetry_receipt_sha256",
            "paired_telemetry_receipt_sha256", "active_interval_log_sha256",
        },
        label=f"{label}.telemetry",
    )
    gap_raw = telemetry_raw.get("historical_gap_seconds")
    if not isinstance(gap_raw, list) or len(gap_raw) != 3:
        raise TuGlobalPoolProducerInputError(f"{label}.telemetry.historical_gap_seconds must bind three gaps")
    telemetry = {
        "parent_active_seconds": _number(telemetry_raw.get("parent_active_seconds"), f"{label}.telemetry.parent_active_seconds"),
        "parent_heavy_seconds": _number(telemetry_raw.get("parent_heavy_seconds"), f"{label}.telemetry.parent_heavy_seconds"),
        "wait_seconds": _number(telemetry_raw.get("wait_seconds"), f"{label}.telemetry.wait_seconds"),
        "telemetry_complete": _boolean(telemetry_raw.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete", True),
        "current_campaign_eligible": _boolean(telemetry_raw.get("current_campaign_eligible"), f"{label}.telemetry.current_campaign_eligible", True),
        "historical_denominator_allowed": _boolean(telemetry_raw.get("historical_denominator_allowed"), f"{label}.telemetry.historical_denominator_allowed", False),
        "no_historical_imputation": _boolean(telemetry_raw.get("no_historical_imputation"), f"{label}.telemetry.no_historical_imputation", True),
        "historical_gap_seconds": [_number(item, f"{label}.telemetry.historical_gap_seconds[{index}]", minimum=0.000001) for index, item in enumerate(gap_raw)],
        "telemetry_receipt_sha256": _sha(telemetry_raw.get("telemetry_receipt_sha256"), f"{label}.telemetry.telemetry_receipt_sha256"),
        "paired_telemetry_receipt_sha256": _sha(telemetry_raw.get("paired_telemetry_receipt_sha256"), f"{label}.telemetry.paired_telemetry_receipt_sha256"),
        "active_interval_log_sha256": _sha(telemetry_raw.get("active_interval_log_sha256"), f"{label}.telemetry.active_interval_log_sha256"),
    }
    if (
        not math.isclose(telemetry["parent_active_seconds"], 893.2932908, abs_tol=1e-7)
        or not math.isclose(telemetry["parent_heavy_seconds"], 0.2495924, abs_tol=1e-7)
        or telemetry["wait_seconds"] != 0.0
    ):
        raise TuGlobalPoolProducerInputError(f"{label}.telemetry bounded campaign timing drifted")

    return {
        "schema": CONTEXT_SCHEMA,
        "owner": owner,
        "functions": functions,
        "source_owner_task": source_owner_task,
        "authority_advanced": False,
        "reports": reports,
        "toolchain": toolchain,
        "provenance": provenance,
        "producer": producer,
        "consumers": consumers,
        "static_control": control,
        "exact_result": exact,
        "telemetry": telemetry,
        "forbidden_axes": _exact_sequence(root.get("forbidden_axes"), FORBIDDEN_AXES, f"{label}.forbidden_axes"),
    }


def _size(symbol: Mapping[str, Any] | None) -> int | None:
    return causal_reducer._parse_number(symbol.get("size")) if symbol else None


def _match(symbol: Mapping[str, Any] | None) -> float | None:
    if symbol is None:
        return None
    value = symbol.get("match_percent")
    return float(value) if isinstance(value, (int, float)) and not isinstance(value, bool) else None


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1 for instruction in instructions
        if instruction.relocation and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _mismatches(target: Sequence[Any], candidate: Sequence[Any]) -> list[tuple[int, Any, Any]]:
    return [
        (index, left, right)
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if causal_reducer._instruction_mismatch(left, right)
    ]


def _pool_owner_only(left: Any, right: Any) -> bool:
    if (
        left is None or right is None or not left.has_instruction or not right.has_instruction
        or left.mnemonic != right.mnemonic or left.mnemonic != "lfs"
        or not _SDA_OWNER_RE.search(left.formatted) or not _SDA_OWNER_RE.search(right.formatted)
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
        return {"matched": False, "reason": "no authenticated TU-global pool-producer context was supplied"}
    if pair.name not in context["functions"]:
        return {"matched": False, "reason": "the TU-global pool-producer context is bound to another function family"}

    report = next(item for item in context["reports"] if item["function"] == pair.name)
    observed = (_size(pair.target), _size(pair.candidate), _physical_relocations(target), _physical_relocations(candidate), _match(pair.candidate))
    exact = context["exact_result"]
    if objdiff_canonical_sha256 == exact["objdiff_canonical_sha256"]:
        mismatches = _mismatches(target, candidate)
        if observed[:4] != (
            report["target_size"], report["candidate_size"],
            report["physical_relocations"], report["physical_relocations"],
        ) or not math.isclose(observed[4] or -1.0, 100.0, abs_tol=1e-9) or mismatches:
            return {"matched": False, "reason": "the sealed global-producer exact result drifted", "evidence": {"observed_signature": list(observed), "residual_count": len(mismatches)}}
        return {
            "matched": False,
            "reason": "the function is already exact under the authenticated global producer; no candidate is scheduled",
            "evidence": {"producer": context["producer"], "consumer_count": len(context["consumers"]), "telemetry": context["telemetry"], "authority_advanced": False},
        }

    control = context["static_control"]
    if objdiff_canonical_sha256 != control["objdiff_canonical_sha256"]:
        return {"matched": False, "reason": "the report matches neither the sealed local-static control nor the global exact result"}
    if pair.name != "mbev_CapMiracle":
        return {"matched": False, "reason": "the object-neutral local-static control is focus-bound to mbev_CapMiracle"}
    mismatches = _mismatches(target, candidate)
    if observed[:4] != (1256, 1256, 96, 96) or not math.isclose(observed[4] or -1.0, 99.98408, abs_tol=1e-6):
        return {"matched": False, "reason": "the sealed local-static control signature drifted", "evidence": {"observed_signature": list(observed)}}
    if len(mismatches) != 1 or not _pool_owner_only(mismatches[0][1], mismatches[0][2]):
        return {"matched": False, "reason": "the control residual is not the sealed single pool-owner row", "evidence": {"residual_count": len(mismatches)}}
    return {
        "matched": True,
        "reason": "the seven-consumer local-static control is object-neutral while target metadata proves a global f32 owner at the authenticated TU boundary",
        "evidence": {
            "stage": "local_static_control_to_global_tu_producer",
            "residual_row": {"row": mismatches[0][0], "target": mismatches[0][1].formatted, "candidate": mismatches[0][2].formatted},
            "negative_control": control,
            "recommended_cells": [{
                "kind": "restore_authenticated_global_tu_pool_producer",
                "declaration": "const float capspecialTen = 10.0f;",
                "source_line": context["producer"]["source_line"],
                "before_function": context["producer"]["before_function"],
                "target_symbol": context["producer"]["target_symbol"],
                "consumers": context["consumers"],
            }],
            "suppress_downstream_body_edits": True,
            "suppress_tracer": True,
            "provenance": context["provenance"],
            "telemetry": context["telemetry"],
            "forbidden_axes": context["forbidden_axes"],
            "authority_advanced": False,
        },
    }
