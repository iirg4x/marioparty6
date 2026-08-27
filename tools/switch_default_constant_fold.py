#!/usr/bin/env python3
"""Fail-closed switch/default topology plus typed constant-fold diagnosis."""

from __future__ import annotations

import math
import re
import struct
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "switch_default_constant_fold_context/v1"
RULE_ID = "switch_default_typed_constant_fold"

_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_BRANCH_RE = re.compile(r"^b\s+0[xX][0-9a-fA-F]+$")
_CONTROL_AXES = (
    "outer_zero_if_else",
    "explicit_else_return",
    "terminal_if_chain",
    "explicit_two_arm_returns",
    "inner_else_return",
)
_CONTROL_OUTCOMES = {
    "outer_zero_if_else": "wrong_size_topology",
    "explicit_else_return": "wrong_size_topology",
    "terminal_if_chain": "baseline_equivalent",
    "explicit_two_arm_returns": "wrong_size_topology",
    "inner_else_return": "wrong_size_topology",
}


class SwitchDefaultFoldInputError(ValueError):
    """The evidence cannot safely support the ordered topology/fold cells."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SwitchDefaultFoldInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise SwitchDefaultFoldInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise SwitchDefaultFoldInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SwitchDefaultFoldInputError(f"{label} exceeds {limit} characters")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if result != result.lower() or _SHA256_RE.fullmatch(result) is None:
        raise SwitchDefaultFoldInputError(f"{label} must be a lowercase SHA-256")
    return result


def _uint(
    value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24
) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise SwitchDefaultFoldInputError(
            f"{label} must be an integer from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, maximum: float = 100.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SwitchDefaultFoldInputError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or not 0.0 <= result <= maximum:
        raise SwitchDefaultFoldInputError(f"{label} is outside the supported range")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise SwitchDefaultFoldInputError(
            f"{label} must be {str(expected).lower()}"
        )
    return expected


def _bits(value: Any, label: str) -> str:
    result = _text(value, label, limit=10).lower()
    if result.startswith("0x"):
        result = result[2:]
    if re.fullmatch(r"[0-9a-f]{8}", result) is None:
        raise SwitchDefaultFoldInputError(f"{label} must contain one f32 bit pattern")
    return result


def _f32(value: float) -> float:
    return struct.unpack(">f", struct.pack(">f", value))[0]


def _f32_bits(value: float) -> str:
    return struct.pack(">f", _f32(value)).hex()


def _hashes(value: Mapping[str, Any], label: str, names: Sequence[str]) -> dict[str, str]:
    return {name: _sha256(value.get(name), f"{label}.{name}") for name in names}


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "switch/default constant-fold context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "precursor",
        "topology",
        "negative_controls",
        "topology_result",
        "pool_residual",
        "typed_fold",
        "exact_result",
        "telemetry",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SwitchDefaultFoldInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    report_artifact_sha256 = _sha256(
        context.get("report_artifact_sha256"), f"{label}.report_artifact_sha256"
    )
    _bool(context.get("authority_advanced"), f"{label}.authority_advanced", False)

    precursor_fields = {
        "function",
        "candidate_id",
        "objdiff_canonical_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "object_sha256",
        "target_bytes",
        "candidate_bytes",
        "target_frame",
        "candidate_frame",
        "match_percent",
        "target_physical_relocations",
        "candidate_physical_relocations",
        "residual_pairs",
        "operation_order_exact",
        "stack_homes_exact",
        "physical_relocations_exact",
        "protected_siblings_preserved",
    }
    precursor_raw = _closed(
        context.get("precursor"),
        allowed=precursor_fields,
        required=precursor_fields,
        label=f"{label}.precursor",
    )
    target_bytes = _uint(precursor_raw.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=32)
    candidate_bytes = _uint(
        precursor_raw.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=32
    )
    target_frame = _uint(precursor_raw.get("target_frame"), f"{label}.precursor.target_frame", minimum=16)
    candidate_frame = _uint(
        precursor_raw.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16
    )
    target_relocations = _uint(
        precursor_raw.get("target_physical_relocations"),
        f"{label}.precursor.target_physical_relocations",
        minimum=1,
    )
    candidate_relocations = _uint(
        precursor_raw.get("candidate_physical_relocations"),
        f"{label}.precursor.candidate_physical_relocations",
        minimum=1,
    )
    match_percent = _number(
        precursor_raw.get("match_percent"), f"{label}.precursor.match_percent"
    )
    if (
        target_bytes - candidate_bytes != 4
        or target_frame != candidate_frame
        or target_relocations != candidate_relocations
        or match_percent >= 100.0
    ):
        raise SwitchDefaultFoldInputError(
            f"{label}.precursor must be four bytes short with exact frame/relocations"
        )
    for name in (
        "operation_order_exact",
        "stack_homes_exact",
        "physical_relocations_exact",
        "protected_siblings_preserved",
    ):
        _bool(precursor_raw.get(name), f"{label}.precursor.{name}", True)

    residuals_raw = precursor_raw.get("residual_pairs")
    if not isinstance(residuals_raw, list) or len(residuals_raw) != 2:
        raise SwitchDefaultFoldInputError(
            f"{label}.precursor.residual_pairs must contain exactly two rows"
        )
    residuals: list[dict[str, Any]] = []
    expected_kinds = ("pool_operand", "target_only_terminal_branch")
    for index, raw in enumerate(residuals_raw):
        item = _closed(
            raw,
            allowed={"row", "kind", "target", "candidate"},
            required={"row", "kind", "target", "candidate"},
            label=f"{label}.precursor.residual_pairs[{index}]",
        )
        kind = _text(item.get("kind"), f"{label}.precursor.residual_pairs[{index}].kind")
        if kind != expected_kinds[index]:
            raise SwitchDefaultFoldInputError(
                f"{label}.precursor.residual_pairs must be pool then terminal branch"
            )
        target_form = _text(item.get("target"), f"{label}.precursor.residual_pairs[{index}].target")
        candidate_value = item.get("candidate")
        if kind == "pool_operand":
            candidate_form: str | None = _text(
                candidate_value, f"{label}.precursor.residual_pairs[{index}].candidate"
            )
            if not target_form.startswith("lfs f3, ") or not candidate_form.startswith("lfs f3, "):
                raise SwitchDefaultFoldInputError(f"{label}.precursor pool row must be an lfs f3 operand seam")
        else:
            if candidate_value is not None or _BRANCH_RE.fullmatch(target_form) is None:
                raise SwitchDefaultFoldInputError(
                    f"{label}.precursor terminal row must be a target-only unconditional branch"
                )
            candidate_form = None
        residuals.append(
            {
                "row": _uint(item.get("row"), f"{label}.precursor.residual_pairs[{index}].row"),
                "kind": kind,
                "target": target_form,
                "candidate": candidate_form,
            }
        )
    if [item["row"] for item in residuals] != sorted({item["row"] for item in residuals}):
        raise SwitchDefaultFoldInputError(f"{label}.precursor residual rows must be sorted and unique")
    precursor = {
        "function": _text(precursor_raw.get("function"), f"{label}.precursor.function", limit=128),
        "candidate_id": _text(precursor_raw.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128),
        **_hashes(
            precursor_raw,
            f"{label}.precursor",
            ("objdiff_canonical_sha256", "strict_report_sha256", "data_report_sha256", "object_sha256"),
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocations,
        "residual_pairs": residuals,
        "operation_order_exact": True,
        "stack_homes_exact": True,
        "physical_relocations_exact": True,
        "protected_siblings_preserved": True,
    }

    topology_fields = {
        "source_shape",
        "state_load_row",
        "state_load_form",
        "state_compare_row",
        "state_compare_form",
        "zero_branch_row",
        "target_zero_branch",
        "candidate_zero_branch",
        "body_exit_row",
        "target_body_exit",
        "candidate_body_exit",
        "cleanup_start_row",
        "cleanup_window",
        "terminal_branch_row",
        "target_terminal_branch",
        "epilogue_row",
        "epilogue_form",
        "pool_consumer_row",
        "pool_consumer_form",
    }
    topology_raw = _closed(
        context.get("topology"),
        allowed=topology_fields,
        required=topology_fields,
        label=f"{label}.topology",
    )
    source_shape = _text(topology_raw.get("source_shape"), f"{label}.topology.source_shape")
    if source_shape != "switch_terminal_default_cleanup_return":
        raise SwitchDefaultFoldInputError(
            f"{label}.topology.source_shape must be switch_terminal_default_cleanup_return"
        )
    cleanup_raw = topology_raw.get("cleanup_window")
    if cleanup_raw != ["lwz r3, mbObjMan@sda21", "mr r4, r30", "bl omDelObjEx"]:
        raise SwitchDefaultFoldInputError(f"{label}.topology.cleanup_window is not the sealed cleanup call")
    topology = {
        "source_shape": source_shape,
        "state_load_row": _uint(topology_raw.get("state_load_row"), f"{label}.topology.state_load_row"),
        "state_load_form": _text(topology_raw.get("state_load_form"), f"{label}.topology.state_load_form"),
        "state_compare_row": _uint(topology_raw.get("state_compare_row"), f"{label}.topology.state_compare_row"),
        "state_compare_form": _text(topology_raw.get("state_compare_form"), f"{label}.topology.state_compare_form"),
        "zero_branch_row": _uint(topology_raw.get("zero_branch_row"), f"{label}.topology.zero_branch_row"),
        "target_zero_branch": _text(topology_raw.get("target_zero_branch"), f"{label}.topology.target_zero_branch"),
        "candidate_zero_branch": _text(topology_raw.get("candidate_zero_branch"), f"{label}.topology.candidate_zero_branch"),
        "body_exit_row": _uint(topology_raw.get("body_exit_row"), f"{label}.topology.body_exit_row"),
        "target_body_exit": _text(topology_raw.get("target_body_exit"), f"{label}.topology.target_body_exit"),
        "candidate_body_exit": _text(topology_raw.get("candidate_body_exit"), f"{label}.topology.candidate_body_exit"),
        "cleanup_start_row": _uint(topology_raw.get("cleanup_start_row"), f"{label}.topology.cleanup_start_row"),
        "cleanup_window": list(cleanup_raw),
        "terminal_branch_row": _uint(topology_raw.get("terminal_branch_row"), f"{label}.topology.terminal_branch_row"),
        "target_terminal_branch": _text(topology_raw.get("target_terminal_branch"), f"{label}.topology.target_terminal_branch"),
        "epilogue_row": _uint(topology_raw.get("epilogue_row"), f"{label}.topology.epilogue_row"),
        "epilogue_form": _text(topology_raw.get("epilogue_form"), f"{label}.topology.epilogue_form"),
        "pool_consumer_row": _uint(topology_raw.get("pool_consumer_row"), f"{label}.topology.pool_consumer_row"),
        "pool_consumer_form": _text(topology_raw.get("pool_consumer_form"), f"{label}.topology.pool_consumer_form"),
    }
    if (
        topology["state_load_form"] != "lwz r0, 0x4c(r30)"
        or topology["state_compare_form"] != "cmpwi r0, 0x0"
        or not topology["target_zero_branch"].startswith("beq ")
        or not topology["candidate_zero_branch"].startswith("beq ")
        or _BRANCH_RE.fullmatch(topology["target_body_exit"]) is None
        or _BRANCH_RE.fullmatch(topology["candidate_body_exit"]) is None
        or topology["target_terminal_branch"] != topology["target_body_exit"]
        or topology["pool_consumer_form"] != "bl mbev_CapEffGlowAdd"
    ):
        raise SwitchDefaultFoldInputError(f"{label}.topology does not describe the sealed Hanachan window")
    if not (
        topology["state_load_row"] < topology["state_compare_row"] < topology["zero_branch_row"]
        < topology["body_exit_row"] < topology["cleanup_start_row"]
        < topology["terminal_branch_row"] < topology["epilogue_row"]
    ):
        raise SwitchDefaultFoldInputError(f"{label}.topology rows are not in execution order")
    if topology["cleanup_start_row"] + 3 != topology["terminal_branch_row"]:
        raise SwitchDefaultFoldInputError(f"{label}.topology cleanup must immediately precede the terminal branch")

    controls_raw = context.get("negative_controls")
    if not isinstance(controls_raw, list) or len(controls_raw) != len(_CONTROL_AXES):
        raise SwitchDefaultFoldInputError(f"{label}.negative_controls must contain five sealed controls")
    controls: list[dict[str, Any]] = []
    for index, raw in enumerate(controls_raw):
        item = _closed(
            raw,
            allowed={"axis", "outcome", "candidate_id", "strict_report_sha256", "target_bytes", "candidate_bytes", "match_percent"},
            required={"axis", "outcome", "candidate_id", "strict_report_sha256", "target_bytes", "candidate_bytes", "match_percent"},
            label=f"{label}.negative_controls[{index}]",
        )
        axis = _text(item.get("axis"), f"{label}.negative_controls[{index}].axis")
        outcome = _text(item.get("outcome"), f"{label}.negative_controls[{index}].outcome")
        if axis != _CONTROL_AXES[index] or outcome != _CONTROL_OUTCOMES[axis]:
            raise SwitchDefaultFoldInputError(f"{label}.negative_controls are not in the sealed axis/outcome order")
        target_size = _uint(item.get("target_bytes"), f"{label}.negative_controls[{index}].target_bytes", minimum=32)
        candidate_size = _uint(item.get("candidate_bytes"), f"{label}.negative_controls[{index}].candidate_bytes", minimum=32)
        match = _number(item.get("match_percent"), f"{label}.negative_controls[{index}].match_percent")
        if target_size != target_bytes or match >= 100.0:
            raise SwitchDefaultFoldInputError(f"{label}.negative_controls[{index}] is not a nonexact same-target control")
        controls.append(
            {
                "axis": axis,
                "outcome": outcome,
                "candidate_id": _text(item.get("candidate_id"), f"{label}.negative_controls[{index}].candidate_id", limit=128),
                "strict_report_sha256": _sha256(item.get("strict_report_sha256"), f"{label}.negative_controls[{index}].strict_report_sha256"),
                "target_bytes": target_size,
                "candidate_bytes": candidate_size,
                "match_percent": match,
            }
        )

    result_fields = {
        "candidate_id", "objdiff_canonical_sha256", "strict_report_sha256", "data_report_sha256",
        "object_sha256", "target_bytes", "candidate_bytes", "match_percent", "physical_relocations",
        "residual_row", "target_form", "candidate_form",
    }
    topology_result_raw = _closed(
        context.get("topology_result"), allowed=result_fields, required=result_fields, label=f"{label}.topology_result"
    )
    topology_result_target = _uint(topology_result_raw.get("target_bytes"), f"{label}.topology_result.target_bytes", minimum=32)
    topology_result_candidate = _uint(topology_result_raw.get("candidate_bytes"), f"{label}.topology_result.candidate_bytes", minimum=32)
    topology_result_match = _number(topology_result_raw.get("match_percent"), f"{label}.topology_result.match_percent")
    topology_result_relocations = _uint(topology_result_raw.get("physical_relocations"), f"{label}.topology_result.physical_relocations", minimum=1)
    topology_result_row = _uint(topology_result_raw.get("residual_row"), f"{label}.topology_result.residual_row")
    topology_result_target_form = _text(topology_result_raw.get("target_form"), f"{label}.topology_result.target_form")
    topology_result_candidate_form = _text(topology_result_raw.get("candidate_form"), f"{label}.topology_result.candidate_form")
    if (
        topology_result_target != topology_result_candidate
        or topology_result_target != target_bytes
        or topology_result_match >= 100.0
        or topology_result_relocations != target_relocations
        or topology_result_row != residuals[0]["row"]
        or topology_result_target_form != residuals[0]["target"]
        or topology_result_candidate_form != residuals[0]["candidate"]
    ):
        raise SwitchDefaultFoldInputError(f"{label}.topology_result must close only the terminal branch row")
    topology_result = {
        "candidate_id": _text(topology_result_raw.get("candidate_id"), f"{label}.topology_result.candidate_id", limit=128),
        **_hashes(topology_result_raw, f"{label}.topology_result", ("objdiff_canonical_sha256", "strict_report_sha256", "data_report_sha256", "object_sha256")),
        "target_bytes": topology_result_target,
        "candidate_bytes": topology_result_candidate,
        "match_percent": topology_result_match,
        "physical_relocations": topology_result_relocations,
        "residual_row": topology_result_row,
        "target_form": topology_result_target_form,
        "candidate_form": topology_result_candidate_form,
    }

    pool_fields = {
        "decoder_receipt_sha256", "decoder_schema", "row", "register", "target_operand", "candidate_operand",
        "target_bits", "candidate_bits", "value_type", "consumer_symbol", "consumer_row", "consumer_count",
        "relocation_topology_exact", "owner_chronology_exact",
    }
    pool_raw = _closed(context.get("pool_residual"), allowed=pool_fields, required=pool_fields, label=f"{label}.pool_residual")
    if _text(pool_raw.get("decoder_schema"), f"{label}.pool_residual.decoder_schema") != "match_workbench_pool_decoder/v1":
        raise SwitchDefaultFoldInputError(f"{label}.pool_residual.decoder_schema is unsupported")
    pool = {
        "decoder_receipt_sha256": _sha256(pool_raw.get("decoder_receipt_sha256"), f"{label}.pool_residual.decoder_receipt_sha256"),
        "decoder_schema": "match_workbench_pool_decoder/v1",
        "row": _uint(pool_raw.get("row"), f"{label}.pool_residual.row"),
        "register": _text(pool_raw.get("register"), f"{label}.pool_residual.register", limit=3).lower(),
        "target_operand": _text(pool_raw.get("target_operand"), f"{label}.pool_residual.target_operand"),
        "candidate_operand": _text(pool_raw.get("candidate_operand"), f"{label}.pool_residual.candidate_operand"),
        "target_bits": _bits(pool_raw.get("target_bits"), f"{label}.pool_residual.target_bits"),
        "candidate_bits": _bits(pool_raw.get("candidate_bits"), f"{label}.pool_residual.candidate_bits"),
        "value_type": _text(pool_raw.get("value_type"), f"{label}.pool_residual.value_type"),
        "consumer_symbol": _text(pool_raw.get("consumer_symbol"), f"{label}.pool_residual.consumer_symbol", limit=128),
        "consumer_row": _uint(pool_raw.get("consumer_row"), f"{label}.pool_residual.consumer_row"),
        "consumer_count": _uint(pool_raw.get("consumer_count"), f"{label}.pool_residual.consumer_count", minimum=1, maximum=8),
        "relocation_topology_exact": _bool(pool_raw.get("relocation_topology_exact"), f"{label}.pool_residual.relocation_topology_exact", True),
        "owner_chronology_exact": _bool(pool_raw.get("owner_chronology_exact"), f"{label}.pool_residual.owner_chronology_exact", True),
    }
    if (
        pool["row"] != residuals[0]["row"]
        or pool["register"] != "f3"
        or f"lfs f3, {pool['target_operand']}" != residuals[0]["target"]
        or f"lfs f3, {pool['candidate_operand']}" != residuals[0]["candidate"]
        or pool["value_type"] != "f32"
        or pool["consumer_symbol"] != "mbev_CapEffGlowAdd"
        or pool["consumer_row"] != topology["pool_consumer_row"]
        or pool["consumer_count"] != 1
        or pool["target_bits"] == pool["candidate_bits"]
    ):
        raise SwitchDefaultFoldInputError(f"{label}.pool_residual is not the sealed one-row f32 consumer seam")

    fold_fields = {
        "candidate_source", "exact_source", "numerator", "denominator", "candidate_domain", "exact_domain",
        "destination_type", "candidate_bits", "exact_bits", "opaque_bit_literals_forbidden", "arbitrary_numeric_search_forbidden",
    }
    fold_raw = _closed(context.get("typed_fold"), allowed=fold_fields, required=fold_fields, label=f"{label}.typed_fold")
    numerator = _number(fold_raw.get("numerator"), f"{label}.typed_fold.numerator")
    denominator = _number(fold_raw.get("denominator"), f"{label}.typed_fold.denominator")
    if denominator == 0.0:
        raise SwitchDefaultFoldInputError(f"{label}.typed_fold.denominator must be nonzero")
    candidate_bits = _bits(fold_raw.get("candidate_bits"), f"{label}.typed_fold.candidate_bits")
    exact_bits = _bits(fold_raw.get("exact_bits"), f"{label}.typed_fold.exact_bits")
    computed_candidate_bits = _f32_bits(_f32(numerator) / _f32(denominator))
    computed_exact_bits = _f32_bits(numerator / denominator)
    if (
        _text(fold_raw.get("candidate_domain"), f"{label}.typed_fold.candidate_domain") != "f32"
        or _text(fold_raw.get("exact_domain"), f"{label}.typed_fold.exact_domain") != "f64"
        or _text(fold_raw.get("destination_type"), f"{label}.typed_fold.destination_type") != "f32"
        or candidate_bits != computed_candidate_bits
        or exact_bits != computed_exact_bits
        or candidate_bits != pool["candidate_bits"]
        or exact_bits != pool["target_bits"]
        or candidate_bits == exact_bits
    ):
        raise SwitchDefaultFoldInputError(f"{label}.typed_fold does not reproduce the sealed one-ULP pair")
    fold = {
        "candidate_source": _text(fold_raw.get("candidate_source"), f"{label}.typed_fold.candidate_source"),
        "exact_source": _text(fold_raw.get("exact_source"), f"{label}.typed_fold.exact_source"),
        "numerator": numerator,
        "denominator": denominator,
        "candidate_domain": "f32",
        "exact_domain": "f64",
        "destination_type": "f32",
        "candidate_bits": candidate_bits,
        "exact_bits": exact_bits,
        "opaque_bit_literals_forbidden": _bool(fold_raw.get("opaque_bit_literals_forbidden"), f"{label}.typed_fold.opaque_bit_literals_forbidden", True),
        "arbitrary_numeric_search_forbidden": _bool(fold_raw.get("arbitrary_numeric_search_forbidden"), f"{label}.typed_fold.arbitrary_numeric_search_forbidden", True),
    }
    if fold["candidate_source"] != "4.9f / 60.0f" or fold["exact_source"] != "4.9 / 60.0":
        raise SwitchDefaultFoldInputError(f"{label}.typed_fold source spellings are not the authenticated natural pair")

    exact_fields = {
        "candidate_id", "source_sha256", "object_sha256", "strict_report_sha256", "data_report_sha256",
        "candidate_record_sha256", "target_bytes", "candidate_bytes", "physical_relocations",
    }
    exact_raw = _closed(context.get("exact_result"), allowed=exact_fields, required=exact_fields, label=f"{label}.exact_result")
    exact_target = _uint(exact_raw.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=32)
    exact_candidate = _uint(exact_raw.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=32)
    exact_relocations = _uint(exact_raw.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1)
    if exact_target != target_bytes or exact_candidate != target_bytes or exact_relocations != target_relocations:
        raise SwitchDefaultFoldInputError(f"{label}.exact_result does not seal exact size/relocations")
    exact_result = {
        "candidate_id": _text(exact_raw.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128),
        **_hashes(exact_raw, f"{label}.exact_result", ("source_sha256", "object_sha256", "strict_report_sha256", "data_report_sha256", "candidate_record_sha256")),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
    }

    telemetry_fields = {"candidate_count", "tracer_runs", "donor_searches", "telemetry_complete", "interval_log_sha256"}
    telemetry_raw = _closed(context.get("telemetry"), allowed=telemetry_fields, required=telemetry_fields, label=f"{label}.telemetry")
    telemetry = {
        "candidate_count": _uint(telemetry_raw.get("candidate_count"), f"{label}.telemetry.candidate_count", minimum=1),
        "tracer_runs": _uint(telemetry_raw.get("tracer_runs"), f"{label}.telemetry.tracer_runs", maximum=128),
        "donor_searches": _uint(telemetry_raw.get("donor_searches"), f"{label}.telemetry.donor_searches", maximum=128),
        "telemetry_complete": _bool(telemetry_raw.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete", False),
        "interval_log_sha256": _sha256(telemetry_raw.get("interval_log_sha256"), f"{label}.telemetry.interval_log_sha256"),
    }
    if telemetry["candidate_count"] != 9 or telemetry["tracer_runs"] != 0 or telemetry["donor_searches"] != 0:
        raise SwitchDefaultFoldInputError(f"{label}.telemetry does not bind the nine-cell no-trace campaign")

    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": report_artifact_sha256,
        "precursor": precursor,
        "topology": topology,
        "negative_controls": controls,
        "topology_result": topology_result,
        "pool_residual": pool,
        "typed_fold": fold,
        "exact_result": exact_result,
        "telemetry": telemetry,
        "authority_advanced": False,
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:12]:
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


def _marked_rows(target: Sequence[Any], candidate: Sequence[Any]) -> list[int]:
    return [
        index
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if (left is None or right is None)
        or (left.diff_kind is not None and left.diff_kind not in {"DIFF_MATCH", "DIFF_NONE"})
        or (right.diff_kind is not None and right.diff_kind not in {"DIFF_MATCH", "DIFF_NONE"})
    ]


def evaluate(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {"matched": False, "reason": "no authenticated switch/default fold context was supplied"}
    precursor = context["precursor"]
    if precursor["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {"matched": False, "reason": "the switch/default fold context is bound to another objdiff report"}
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
        precursor["physical_relocations"],
        precursor["physical_relocations"],
    )
    if observed_signature != sealed_signature:
        return {
            "matched": False,
            "reason": "the sealed size/frame/physical-relocation signature drifted",
            "evidence": {"observed": list(observed_signature), "sealed": list(sealed_signature)},
        }
    rows = causal_reducer._paired_records(target, candidate)
    sealed_rows = [item["row"] for item in precursor["residual_pairs"]]
    observed_rows = _marked_rows(target, candidate)
    if observed_rows != sealed_rows:
        return {
            "matched": False,
            "reason": "the marked rows differ from the sealed pool-plus-terminal-branch residual",
            "evidence": {"observed_rows": observed_rows, "sealed_rows": sealed_rows},
        }
    for item in precursor["residual_pairs"]:
        row = item["row"]
        if not 0 <= row < len(rows):
            return {"matched": False, "reason": "a sealed residual row lies outside the function"}
        left, right = rows[row]
        if item["kind"] == "pool_operand":
            if (
                left is None or right is None or not left.has_instruction or not right.has_instruction
                or left.formatted != item["target"] or right.formatted != item["candidate"]
            ):
                return {"matched": False, "reason": "the sealed lfs pool-operand row drifted"}
        elif (
            left is None or not left.has_instruction or left.formatted != item["target"]
            or right is None or right.has_instruction
        ):
            return {"matched": False, "reason": "the sealed target-only terminal branch row drifted"}

    topology = context["topology"]

    def paired_forms(row: int) -> tuple[str | None, str | None]:
        if not 0 <= row < len(rows):
            return None, None
        left, right = rows[row]
        return (
            left.formatted if left is not None and left.has_instruction else None,
            right.formatted if right is not None and right.has_instruction else None,
        )

    exact_pairs = (
        (topology["state_load_row"], topology["state_load_form"], topology["state_load_form"]),
        (topology["state_compare_row"], topology["state_compare_form"], topology["state_compare_form"]),
        (topology["zero_branch_row"], topology["target_zero_branch"], topology["candidate_zero_branch"]),
        (topology["body_exit_row"], topology["target_body_exit"], topology["candidate_body_exit"]),
        (topology["epilogue_row"], topology["epilogue_form"], topology["epilogue_form"]),
        (topology["pool_consumer_row"], topology["pool_consumer_form"], topology["pool_consumer_form"]),
    )
    for row, expected_target, expected_candidate in exact_pairs:
        if paired_forms(row) != (expected_target, expected_candidate):
            return {
                "matched": False,
                "reason": "the sealed switch, body-exit, epilogue, or pool-consumer window drifted",
                "evidence": {"row": row, "observed": list(paired_forms(row))},
            }
    for offset, form in enumerate(topology["cleanup_window"]):
        row = topology["cleanup_start_row"] + offset
        if paired_forms(row) != (form, form):
            return {
                "matched": False,
                "reason": "the sealed default cleanup call window drifted",
                "evidence": {"row": row, "observed": list(paired_forms(row))},
            }
    terminal_forms = paired_forms(topology["terminal_branch_row"])
    if terminal_forms != (topology["target_terminal_branch"], None):
        return {
            "matched": False,
            "reason": "the default arm no longer lacks exactly the target terminal return branch",
            "evidence": {"observed": list(terminal_forms)},
        }
    pool = context["pool_residual"]
    fold = context["typed_fold"]
    computed_candidate_bits = _f32_bits(_f32(fold["numerator"]) / _f32(fold["denominator"]))
    computed_exact_bits = _f32_bits(fold["numerator"] / fold["denominator"])
    if (
        computed_candidate_bits != pool["candidate_bits"]
        or computed_exact_bits != pool["target_bits"]
        or computed_candidate_bits == computed_exact_bits
    ):
        return {"matched": False, "reason": "the natural f32/f64 fold no longer reproduces the sealed pool bits"}

    recommended_cells = [
        {
            "order": 1,
            "kind": "switch_terminal_default_cleanup_return",
            "source_shape": topology["source_shape"],
            "prerequisite": "two-row residual with exact switch body, cleanup call, frame, homes, and relocations",
            "acceptance": {
                "candidate_id": context["topology_result"]["candidate_id"],
                "objdiff_canonical_sha256": context["topology_result"]["objdiff_canonical_sha256"],
                "target_bytes": context["topology_result"]["target_bytes"],
                "candidate_bytes": context["topology_result"]["candidate_bytes"],
                "remaining_rows": [context["topology_result"]["residual_row"]],
            },
        },
        {
            "order": 2,
            "kind": "typed_f32_vs_f64_constant_fold",
            "prerequisite": "topology cell closes size/CFG and leaves exactly one typed pool-operand row",
            "candidate_source": fold["candidate_source"],
            "exact_source": fold["exact_source"],
            "candidate_bits": fold["candidate_bits"],
            "exact_bits": fold["exact_bits"],
            "consumer": {"symbol": pool["consumer_symbol"], "row": pool["consumer_row"]},
            "acceptance": {
                "candidate_id": context["exact_result"]["candidate_id"],
                "object_sha256": context["exact_result"]["object_sha256"],
                "strict_report_sha256": context["exact_result"]["strict_report_sha256"],
                "data_report_sha256": context["exact_result"]["data_report_sha256"],
            },
        },
    ]
    return {
        "matched": True,
        "reason": (
            "an exact-frame four-byte-short switch has one target-only branch after a sealed default cleanup; "
            "the authenticated topology result leaves one f32 pool operand whose one-ULP value is reproduced "
            "only by the natural f64-folded source expression"
        ),
        "confidence": 1.0,
        "source_class": "terminal_default_return_then_typed_constant_fold",
        "recommendation": (
            "Compile the terminal default cleanup/return switch cell first; only if it leaves the sealed one-row "
            "pool residual, compile the authenticated unsuffixed double-fold expression."
        ),
        "evidence": {
            "precursor": precursor,
            "topology": topology,
            "negative_controls": context["negative_controls"],
            "topology_result": context["topology_result"],
            "pool_residual": pool,
            "typed_fold": fold,
            "exact_result": context["exact_result"],
            "recommended_cells": recommended_cells,
            "suppressed_axes": [
                "guard_and_if_else_permutations",
                "opaque_bit_literals",
                "arbitrary_numeric_search",
                "pool_owner_reordering",
                "tracer_capture",
                "dead_or_fake_locals",
                "padding",
                "register_shaping",
            ],
            "telemetry": context["telemetry"],
            "authority_advanced": False,
        },
    }
