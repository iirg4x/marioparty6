#!/usr/bin/env python3
"""Fail-closed single-use scalar direct-consumption diagnosis at a final call."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "single_use_final_call_consumer_context/v1"
RULE_ID = "single_use_final_call_direct_consumption"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_GPR_RE = re.compile(r"\br(?:[0-9]|[12][0-9]|3[01])\b", re.IGNORECASE)
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_CONTROL_OUTCOMES = {
    "declaration_chronology": "object_identical",
    "unrelated_pointer_birth": "regressed_topology",
    "unrelated_pointer_final_consumer": "regressed_topology",
    "assignment_expression": "grew_function",
}


class SingleUseFinalCallInputError(ValueError):
    """The evidence cannot safely support direct producer consumption."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SingleUseFinalCallInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise SingleUseFinalCallInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise SingleUseFinalCallInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SingleUseFinalCallInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise SingleUseFinalCallInputError(f"{label} must be a C identifier")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if result != result.lower() or _SHA256_RE.fullmatch(result) is None:
        raise SingleUseFinalCallInputError(f"{label} must be a lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise SingleUseFinalCallInputError(
            f"{label} must be an integer from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, maximum: float = 100.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise SingleUseFinalCallInputError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or not 0.0 <= result <= maximum:
        raise SingleUseFinalCallInputError(f"{label} is outside the supported range")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise SingleUseFinalCallInputError(
            f"{label} must be {str(expected).lower()}"
        )
    return expected


def _gpr(value: Any, label: str, *, saved: bool | None = None) -> str:
    result = _text(value, label, limit=3).lower()
    if _GPR_RE.fullmatch(result) is None:
        raise SingleUseFinalCallInputError(f"{label} must be a GPR")
    number = int(result[1:])
    if saved is True and number < 14:
        raise SingleUseFinalCallInputError(f"{label} must be a nonvolatile GPR")
    if saved is False and number >= 14:
        raise SingleUseFinalCallInputError(f"{label} must be a volatile GPR")
    return result


def _hash_fields(value: Mapping[str, Any], label: str, fields: Sequence[str]) -> dict[str, str]:
    return {field: _sha256(value.get(field), f"{label}.{field}") for field in fields}


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "single-use final-call context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "precursor",
        "owners",
        "final_call",
        "negative_controls",
        "exact_result",
        "telemetry",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SingleUseFinalCallInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
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
        "physical_relocations_exact",
        "protected_siblings_preserved",
    }
    precursor = _closed(
        context.get("precursor"),
        allowed=precursor_fields,
        required=precursor_fields,
        label=f"{label}.precursor",
    )
    target_bytes = _uint(precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=32)
    candidate_bytes = _uint(
        precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=32
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
    match_percent = _number(
        precursor.get("match_percent"), f"{label}.precursor.match_percent"
    )
    if (
        target_bytes != candidate_bytes
        or target_frame != candidate_frame
        or target_relocations != candidate_relocations
        or match_percent >= 100.0
    ):
        raise SingleUseFinalCallInputError(
            f"{label}.precursor must be nonexact with exact size/frame/relocations"
        )
    for field in (
        "operation_order_exact",
        "cfg_calls_exact",
        "data_exact",
        "stack_homes_exact",
        "physical_relocations_exact",
        "protected_siblings_preserved",
    ):
        _bool(precursor.get(field), f"{label}.precursor.{field}", True)
    raw_pairs = precursor.get("residual_pairs")
    if not isinstance(raw_pairs, list) or len(raw_pairs) != 4:
        raise SingleUseFinalCallInputError(
            f"{label}.precursor.residual_pairs must contain exactly four rows"
        )
    residual_pairs: list[dict[str, Any]] = []
    expected_mnemonics = ("mr", "lwz", "lwz", "mr")
    for index, raw in enumerate(raw_pairs):
        item = _closed(
            raw,
            allowed={"row", "target", "candidate"},
            required={"row", "target", "candidate"},
            label=f"{label}.precursor.residual_pairs[{index}]",
        )
        target_form = _text(item.get("target"), f"{label}.precursor.residual_pairs[{index}].target")
        candidate_form = _text(
            item.get("candidate"), f"{label}.precursor.residual_pairs[{index}].candidate"
        )
        target_mnemonic = target_form.split()[0].lower()
        candidate_mnemonic = candidate_form.split()[0].lower()
        if target_mnemonic != candidate_mnemonic or target_mnemonic != expected_mnemonics[index]:
            raise SingleUseFinalCallInputError(
                f"{label}.precursor residual opcode sequence must be mr/lwz/lwz/mr"
            )
        residual_pairs.append(
            {
                "row": _uint(item.get("row"), f"{label}.precursor.residual_pairs[{index}].row"),
                "target": target_form,
                "candidate": candidate_form,
            }
        )
    residual_rows = [item["row"] for item in residual_pairs]
    if residual_rows != sorted(set(residual_rows)):
        raise SingleUseFinalCallInputError(f"{label}.precursor residual rows must be sorted and unique")
    precursor_hashes = _hash_fields(
        precursor,
        f"{label}.precursor",
        (
            "objdiff_canonical_sha256",
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
        ),
    )
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _text(precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128),
        **precursor_hashes,
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
        "physical_relocations_exact": True,
        "protected_siblings_preserved": True,
    }

    owners = _closed(
        context.get("owners"),
        allowed={"long_lived", "single_use", "unaffected_final_arguments"},
        required={"long_lived", "single_use", "unaffected_final_arguments"},
        label=f"{label}.owners",
    )
    long_raw = _closed(
        owners.get("long_lived"),
        allowed={
            "name",
            "source_role",
            "target_register",
            "candidate_register",
            "capture_row",
            "final_use_row",
            "evidence_sha256",
        },
        required={
            "name",
            "source_role",
            "target_register",
            "candidate_register",
            "capture_row",
            "final_use_row",
            "evidence_sha256",
        },
        label=f"{label}.owners.long_lived",
    )
    if _text(long_raw.get("source_role"), f"{label}.owners.long_lived.source_role") != "function_parameter":
        raise SingleUseFinalCallInputError(
            f"{label}.owners.long_lived.source_role must be function_parameter"
        )
    long_lived = {
        "name": _identifier(long_raw.get("name"), f"{label}.owners.long_lived.name"),
        "source_role": "function_parameter",
        "target_register": _gpr(
            long_raw.get("target_register"), f"{label}.owners.long_lived.target_register", saved=True
        ),
        "candidate_register": _gpr(
            long_raw.get("candidate_register"), f"{label}.owners.long_lived.candidate_register", saved=True
        ),
        "capture_row": _uint(long_raw.get("capture_row"), f"{label}.owners.long_lived.capture_row"),
        "final_use_row": _uint(long_raw.get("final_use_row"), f"{label}.owners.long_lived.final_use_row"),
        "evidence_sha256": _sha256(
            long_raw.get("evidence_sha256"), f"{label}.owners.long_lived.evidence_sha256"
        ),
    }
    single_raw = _closed(
        owners.get("single_use"),
        allowed={
            "name",
            "source_role",
            "target_register",
            "candidate_register",
            "conversion_load_row",
            "final_argument_row",
            "assignment_count",
            "consumer_count",
            "source_expression",
            "evidence_sha256",
        },
        required={
            "name",
            "source_role",
            "target_register",
            "candidate_register",
            "conversion_load_row",
            "final_argument_row",
            "assignment_count",
            "consumer_count",
            "source_expression",
            "evidence_sha256",
        },
        label=f"{label}.owners.single_use",
    )
    if _text(single_raw.get("source_role"), f"{label}.owners.single_use.source_role") != "single_use_scalar_conversion":
        raise SingleUseFinalCallInputError(
            f"{label}.owners.single_use.source_role must be single_use_scalar_conversion"
        )
    if (
        _uint(single_raw.get("assignment_count"), f"{label}.owners.single_use.assignment_count", maximum=8) != 1
        or _uint(single_raw.get("consumer_count"), f"{label}.owners.single_use.consumer_count", maximum=8) != 1
    ):
        raise SingleUseFinalCallInputError(
            f"{label}.owners.single_use must have exactly one assignment and one consumer"
        )
    single_use = {
        "name": _identifier(single_raw.get("name"), f"{label}.owners.single_use.name"),
        "source_role": "single_use_scalar_conversion",
        "target_register": _gpr(
            single_raw.get("target_register"), f"{label}.owners.single_use.target_register", saved=True
        ),
        "candidate_register": _gpr(
            single_raw.get("candidate_register"), f"{label}.owners.single_use.candidate_register", saved=True
        ),
        "conversion_load_row": _uint(
            single_raw.get("conversion_load_row"), f"{label}.owners.single_use.conversion_load_row"
        ),
        "final_argument_row": _uint(
            single_raw.get("final_argument_row"), f"{label}.owners.single_use.final_argument_row"
        ),
        "assignment_count": 1,
        "consumer_count": 1,
        "source_expression": _text(
            single_raw.get("source_expression"), f"{label}.owners.single_use.source_expression"
        ),
        "evidence_sha256": _sha256(
            single_raw.get("evidence_sha256"), f"{label}.owners.single_use.evidence_sha256"
        ),
    }
    if long_lived["name"] == single_use["name"]:
        raise SingleUseFinalCallInputError(f"{label}.owners names must be distinct")
    if (
        long_lived["target_register"] != single_use["candidate_register"]
        or long_lived["candidate_register"] != single_use["target_register"]
        or long_lived["target_register"] == long_lived["candidate_register"]
    ):
        raise SingleUseFinalCallInputError(
            f"{label}.owners must describe one complete two-register swap"
        )
    raw_unaffected = owners.get("unaffected_final_arguments")
    if not isinstance(raw_unaffected, list) or not 1 <= len(raw_unaffected) <= 4:
        raise SingleUseFinalCallInputError(
            f"{label}.owners.unaffected_final_arguments must contain 1-4 entries"
        )
    unaffected: list[dict[str, Any]] = []
    unaffected_rows: set[int] = set()
    for index, raw in enumerate(raw_unaffected):
        item = _closed(
            raw,
            allowed={"name", "register", "abi_register", "argument_row", "evidence_sha256"},
            required={"name", "register", "abi_register", "argument_row", "evidence_sha256"},
            label=f"{label}.owners.unaffected_final_arguments[{index}]",
        )
        row = _uint(item.get("argument_row"), f"{label}.owners.unaffected_final_arguments[{index}].argument_row")
        if row in unaffected_rows:
            raise SingleUseFinalCallInputError(f"{label}.owners unaffected argument rows must be unique")
        unaffected_rows.add(row)
        unaffected.append(
            {
                "name": _identifier(item.get("name"), f"{label}.owners.unaffected_final_arguments[{index}].name"),
                "register": _gpr(item.get("register"), f"{label}.owners.unaffected_final_arguments[{index}].register", saved=True),
                "abi_register": _gpr(item.get("abi_register"), f"{label}.owners.unaffected_final_arguments[{index}].abi_register", saved=False),
                "argument_row": row,
                "evidence_sha256": _sha256(
                    item.get("evidence_sha256"), f"{label}.owners.unaffected_final_arguments[{index}].evidence_sha256"
                ),
            }
        )

    final_raw = _closed(
        context.get("final_call"),
        allowed={
            "symbol",
            "call_row",
            "integer_argument_index",
            "abi_register",
            "conversion_rows",
            "source_template",
            "typed_consumer_proven",
            "call_count_exact",
            "call_order_exact",
            "evidence_sha256",
        },
        required={
            "symbol",
            "call_row",
            "integer_argument_index",
            "abi_register",
            "conversion_rows",
            "source_template",
            "typed_consumer_proven",
            "call_count_exact",
            "call_order_exact",
            "evidence_sha256",
        },
        label=f"{label}.final_call",
    )
    for field in ("typed_consumer_proven", "call_count_exact", "call_order_exact"):
        _bool(final_raw.get(field), f"{label}.final_call.{field}", True)
    conversion_rows_raw = final_raw.get("conversion_rows")
    if not isinstance(conversion_rows_raw, list) or len(conversion_rows_raw) != 3:
        raise SingleUseFinalCallInputError(f"{label}.final_call.conversion_rows must contain three rows")
    conversion_rows = [
        _uint(item, f"{label}.final_call.conversion_rows[{index}]")
        for index, item in enumerate(conversion_rows_raw)
    ]
    if conversion_rows != list(range(conversion_rows[0], conversion_rows[0] + 3)):
        raise SingleUseFinalCallInputError(f"{label}.final_call.conversion_rows must be contiguous")
    call_row = _uint(final_raw.get("call_row"), f"{label}.final_call.call_row")
    if conversion_rows[-1] != single_use["conversion_load_row"]:
        raise SingleUseFinalCallInputError(
            f"{label}.final_call conversion must end at the single-use load row"
        )
    final_call = {
        "symbol": _identifier(final_raw.get("symbol"), f"{label}.final_call.symbol"),
        "call_row": call_row,
        "integer_argument_index": _uint(
            final_raw.get("integer_argument_index"), f"{label}.final_call.integer_argument_index", minimum=0, maximum=31
        ),
        "abi_register": _gpr(final_raw.get("abi_register"), f"{label}.final_call.abi_register", saved=False),
        "conversion_rows": conversion_rows,
        "source_template": _text(final_raw.get("source_template"), f"{label}.final_call.source_template", limit=1024),
        "typed_consumer_proven": True,
        "call_count_exact": True,
        "call_order_exact": True,
        "evidence_sha256": _sha256(final_raw.get("evidence_sha256"), f"{label}.final_call.evidence_sha256"),
    }
    if final_call["abi_register"] == "r3":
        raise SingleUseFinalCallInputError(f"{label}.final_call ABI register is invalid")
    last_argument_row = max(
        [single_use["final_argument_row"], *(item["argument_row"] for item in unaffected)]
    )
    if call_row != last_argument_row + 1:
        raise SingleUseFinalCallInputError(
            f"{label}.final_call must immediately follow the sealed final argument rows"
        )

    controls_raw = context.get("negative_controls")
    if not isinstance(controls_raw, list) or len(controls_raw) != len(_CONTROL_OUTCOMES):
        raise SingleUseFinalCallInputError(
            f"{label}.negative_controls must contain exactly {len(_CONTROL_OUTCOMES)} entries"
        )
    controls: list[dict[str, Any]] = []
    seen_axes: set[str] = set()
    for index, raw in enumerate(controls_raw):
        item = _closed(
            raw,
            allowed={
                "axis",
                "outcome",
                "candidate_id",
                "target_bytes",
                "candidate_bytes",
                "match_percent",
                "evidence_sha256",
            },
            required={
                "axis",
                "outcome",
                "candidate_id",
                "target_bytes",
                "candidate_bytes",
                "match_percent",
                "evidence_sha256",
            },
            label=f"{label}.negative_controls[{index}]",
        )
        axis = _text(item.get("axis"), f"{label}.negative_controls[{index}].axis", limit=128)
        outcome = _text(item.get("outcome"), f"{label}.negative_controls[{index}].outcome", limit=128)
        if axis not in _CONTROL_OUTCOMES or _CONTROL_OUTCOMES[axis] != outcome or axis in seen_axes:
            raise SingleUseFinalCallInputError(f"{label}.negative_controls has an unsupported or duplicate axis/outcome")
        seen_axes.add(axis)
        control_target = _uint(item.get("target_bytes"), f"{label}.negative_controls[{index}].target_bytes", minimum=32)
        control_candidate = _uint(item.get("candidate_bytes"), f"{label}.negative_controls[{index}].candidate_bytes", minimum=32)
        control_match = _number(item.get("match_percent"), f"{label}.negative_controls[{index}].match_percent")
        if control_target != target_bytes or control_match >= 100.0:
            raise SingleUseFinalCallInputError(f"{label}.negative_controls must remain target-bound and nonexact")
        if outcome == "grew_function" and control_candidate <= control_target:
            raise SingleUseFinalCallInputError(f"{label}.negative_controls assignment expression must grow the function")
        if outcome != "grew_function" and control_candidate != control_target:
            raise SingleUseFinalCallInputError(f"{label}.negative_controls non-growth controls must preserve size")
        controls.append(
            {
                "axis": axis,
                "outcome": outcome,
                "candidate_id": _text(item.get("candidate_id"), f"{label}.negative_controls[{index}].candidate_id", limit=128),
                "target_bytes": control_target,
                "candidate_bytes": control_candidate,
                "match_percent": control_match,
                "evidence_sha256": _sha256(item.get("evidence_sha256"), f"{label}.negative_controls[{index}].evidence_sha256"),
            }
        )
    if seen_axes != set(_CONTROL_OUTCOMES):
        raise SingleUseFinalCallInputError(f"{label}.negative_controls are incomplete")

    exact_raw = _closed(
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
    exact_target = _uint(exact_raw.get("target_bytes"), f"{label}.exact_result.target_bytes", minimum=32)
    exact_candidate = _uint(exact_raw.get("candidate_bytes"), f"{label}.exact_result.candidate_bytes", minimum=32)
    exact_relocations = _uint(
        exact_raw.get("physical_relocations"), f"{label}.exact_result.physical_relocations", minimum=1
    )
    if exact_target != exact_candidate or exact_target != target_bytes or exact_relocations != target_relocations:
        raise SingleUseFinalCallInputError(
            f"{label}.exact_result must close target size and preserve physical relocations"
        )
    exact_hashes = _hash_fields(
        exact_raw,
        f"{label}.exact_result",
        (
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "candidate_record_sha256",
        ),
    )
    exact_result = {
        "candidate_id": _text(exact_raw.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
        **exact_hashes,
    }

    telemetry_raw = _closed(
        context.get("telemetry"),
        allowed={"candidate_count", "tracer_runs", "donor_searches", "telemetry_complete", "interval_log_sha256"},
        required={"candidate_count", "tracer_runs", "donor_searches", "telemetry_complete", "interval_log_sha256"},
        label=f"{label}.telemetry",
    )
    telemetry = {
        "candidate_count": _uint(telemetry_raw.get("candidate_count"), f"{label}.telemetry.candidate_count", minimum=1),
        "tracer_runs": _uint(telemetry_raw.get("tracer_runs"), f"{label}.telemetry.tracer_runs", maximum=1000),
        "donor_searches": _uint(telemetry_raw.get("donor_searches"), f"{label}.telemetry.donor_searches", maximum=1000),
        "telemetry_complete": _bool(telemetry_raw.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete", False),
        "interval_log_sha256": _sha256(telemetry_raw.get("interval_log_sha256"), f"{label}.telemetry.interval_log_sha256"),
    }

    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": report_artifact_sha256,
        "precursor": normalized_precursor,
        "owners": {
            "long_lived": long_lived,
            "single_use": single_use,
            "unaffected_final_arguments": unaffected,
        },
        "final_call": final_call,
        "negative_controls": controls,
        "exact_result": exact_result,
        "telemetry": telemetry,
        "authority_advanced": False,
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:12]:
        if not instruction.has_instruction:
            continue
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
        if (left is not None and left.diff_kind is not None)
        or (right is not None and right.diff_kind is not None)
    ]


def _registers(formatted: str) -> list[str]:
    return [match.group(0).lower() for match in _GPR_RE.finditer(formatted)]


def _saved(register: str) -> bool:
    return register.startswith("r") and int(register[1:]) >= 14


def _call_indices(instructions: Sequence[Any], symbol: str) -> list[int]:
    return [
        index
        for index, instruction in enumerate(instructions)
        if instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and re.search(rf"\b{re.escape(symbol)}\b", instruction.formatted) is not None
    ]


def _is_restore_thunk(instruction: Any) -> bool:
    return bool(
        instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and re.search(r"\b_rest(?:gpr|fpr)_\d+\b", instruction.formatted)
    )


def _mismatch_mapping(
    rows: Sequence[tuple[Any | None, Any | None]], residual_rows: Sequence[int]
) -> tuple[dict[str, str], str | None]:
    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    for row in residual_rows:
        left, right = rows[row]
        if left is None or right is None or not left.has_instruction or not right.has_instruction:
            return {}, f"residual row {row} is not paired"
        left_registers = _registers(left.formatted)
        right_registers = _registers(right.formatted)
        if len(left_registers) != len(right_registers):
            return {}, f"residual row {row} changes register arity"
        for target_register, candidate_register in zip(left_registers, right_registers):
            if target_register == candidate_register:
                continue
            if not (_saved(target_register) and _saved(candidate_register)):
                return {}, f"residual row {row} escapes nonvolatile GPR ownership"
            if mapping.get(target_register, candidate_register) != candidate_register:
                return {}, f"residual row {row} has an inconsistent target mapping"
            if reverse.get(candidate_register, target_register) != target_register:
                return {}, f"residual row {row} has a non-bijective mapping"
            mapping[target_register] = candidate_register
            reverse[candidate_register] = target_register
    return mapping, None


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
            "reason": "no authenticated single-use final-call context was supplied",
        }
    precursor = context["precursor"]
    if precursor["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {
            "matched": False,
            "reason": "the single-use final-call context is bound to another objdiff report",
        }
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
            "reason": "the exact size/frame/physical-relocation signature drifted",
            "evidence": {"observed": list(observed_signature), "sealed": list(sealed_signature)},
        }
    rows = causal_reducer._paired_records(target, candidate)
    sealed_rows = [item["row"] for item in precursor["residual_pairs"]]
    observed_rows = _marked_rows(target, candidate)
    if observed_rows != sealed_rows:
        return {
            "matched": False,
            "reason": "the marked residual rows differ from the sealed four-row cycle",
            "evidence": {"observed_rows": observed_rows, "sealed_rows": sealed_rows},
        }
    if not all(row < len(rows) for row in sealed_rows):
        return {"matched": False, "reason": "a sealed residual row lies outside the function"}
    for item in precursor["residual_pairs"]:
        left, right = rows[item["row"]]
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
    mapping, mapping_error = _mismatch_mapping(rows, sealed_rows)
    if mapping_error is not None:
        return {"matched": False, "reason": mapping_error}
    long_lived = context["owners"]["long_lived"]
    single_use = context["owners"]["single_use"]
    expected_mapping = {
        long_lived["target_register"]: long_lived["candidate_register"],
        single_use["target_register"]: single_use["candidate_register"],
    }
    if mapping != expected_mapping:
        return {
            "matched": False,
            "reason": "the authenticated owner identities do not match the physical two-register swap",
            "evidence": {"observed": mapping, "expected": expected_mapping},
        }

    def paired(row: int) -> tuple[Any, Any] | None:
        if not 0 <= row < len(rows):
            return None
        left, right = rows[row]
        if left is None or right is None or not left.has_instruction or not right.has_instruction:
            return None
        return left, right

    capture = paired(long_lived["capture_row"])
    expected_capture = (
        f"mr {long_lived['target_register']}, r3",
        f"mr {long_lived['candidate_register']}, r3",
    )
    if capture is None or (capture[0].formatted, capture[1].formatted) != expected_capture:
        return {"matched": False, "reason": "the long-lived parameter capture drifted"}
    conversion_rows = context["final_call"]["conversion_rows"]
    conversion = [paired(row) for row in conversion_rows]
    if any(item is None for item in conversion):
        return {"matched": False, "reason": "the scalar conversion window is incomplete"}
    assert all(item is not None for item in conversion)
    conversion_pairs = [item for item in conversion if item is not None]
    if (
        conversion_pairs[0][0].mnemonic != "fctiwz"
        or conversion_pairs[0][0].formatted != conversion_pairs[0][1].formatted
        or conversion_pairs[1][0].mnemonic != "stfd"
        or conversion_pairs[1][0].formatted != conversion_pairs[1][1].formatted
    ):
        return {"matched": False, "reason": "the fctiwz/stfd producer window drifted"}
    conversion_load = conversion_pairs[2]
    expected_load_prefix = (
        f"lwz {single_use['target_register']}, ",
        f"lwz {single_use['candidate_register']}, ",
    )
    if (
        not conversion_load[0].formatted.startswith(expected_load_prefix[0])
        or not conversion_load[1].formatted.startswith(expected_load_prefix[1])
        or conversion_load[0].formatted[len(expected_load_prefix[0]) :]
        != conversion_load[1].formatted[len(expected_load_prefix[1]) :]
    ):
        return {"matched": False, "reason": "the converted integer result no longer loads into the sealed owner pair"}
    final_use = paired(long_lived["final_use_row"])
    if (
        final_use is None
        or final_use[0].mnemonic != "lwz"
        or final_use[1].mnemonic != "lwz"
        or long_lived["target_register"] not in _registers(final_use[0].formatted)
        or long_lived["candidate_register"] not in _registers(final_use[1].formatted)
    ):
        return {"matched": False, "reason": "the final long-lived parameter consumer drifted"}
    final_argument = paired(single_use["final_argument_row"])
    expected_argument = (
        f"mr {context['final_call']['abi_register']}, {single_use['target_register']}",
        f"mr {context['final_call']['abi_register']}, {single_use['candidate_register']}",
    )
    if final_argument is None or (
        final_argument[0].formatted,
        final_argument[1].formatted,
    ) != expected_argument:
        return {"matched": False, "reason": "the single-use result no longer feeds the sealed ABI argument"}
    unaffected_evidence: list[dict[str, Any]] = []
    for owner in context["owners"]["unaffected_final_arguments"]:
        argument = paired(owner["argument_row"])
        expected = f"mr {owner['abi_register']}, {owner['register']}"
        if argument is None or argument[0].formatted != expected or argument[1].formatted != expected:
            return {
                "matched": False,
                "reason": "an authenticated unaffected final-call owner drifted",
                "evidence": {"owner": owner["name"], "row": owner["argument_row"]},
            }
        unaffected_evidence.append({**owner, "formatted": expected})
    final_call = context["final_call"]
    target_calls = _call_indices(target, final_call["symbol"])
    candidate_calls = _call_indices(candidate, final_call["symbol"])
    if target_calls != [final_call["call_row"]] or candidate_calls != [final_call["call_row"]]:
        return {
            "matched": False,
            "reason": "the typed final call is missing, duplicated, or moved",
            "evidence": {"target_calls": target_calls, "candidate_calls": candidate_calls},
        }
    call_pair = paired(final_call["call_row"])
    expected_call = f"bl {final_call['symbol']}"
    if call_pair is None or call_pair[0].formatted != expected_call or call_pair[1].formatted != expected_call:
        return {"matched": False, "reason": "the typed final call instruction drifted"}
    if any(
        instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and not _is_restore_thunk(instruction)
        for instruction in target[final_call["call_row"] + 1 :]
    ):
        return {"matched": False, "reason": "the sealed consumer is no longer the final ordinary call"}

    recommended_cell = {
        "order": 1,
        "kind": "direct_single_use_scalar_consumption",
        "remove_local_identity": single_use["name"],
        "consumer_call": final_call["symbol"],
        "argument_index": final_call["integer_argument_index"],
        "source_expression": single_use["source_expression"],
        "source_template": final_call["source_template"],
        "compile_as_one_cell": True,
        "requires_trace": False,
        "preserve_all_other_source_axes": True,
    }
    return {
        "matched": True,
        "reason": (
            "an otherwise exact final-call function has one complete long-lived-parameter/"
            "single-use-conversion saved-GPR swap, one typed consumer, and sealed controls "
            "proving that assignment-expression and unrelated pointer cells are not equivalent"
        ),
        "confidence": 0.995,
        "source_class": "direct_single_use_scalar_to_typed_final_call",
        "recommendation": (
            f"Compile exactly one cell: remove the single-use {single_use['name']} identity "
            f"and pass {single_use['source_expression']} directly to {final_call['symbol']}; "
            "do not test assignment-expression, pointer-lifetime, or declaration-order variants."
        ),
        "evidence": {
            "precursor": precursor,
            "register_mapping": mapping,
            "owners": context["owners"],
            "conversion_rows": conversion_rows,
            "final_call": final_call,
            "unaffected_final_arguments": unaffected_evidence,
            "recommended_cells": [recommended_cell],
            "suppressed_axes": [
                "named_local_assignment_expression",
                "declaration_chronology",
                "unrelated_pointer_birth",
                "unrelated_pointer_final_consumer",
                "global_source_permutations",
                "repeat_tracer_capture",
                "dead_or_fake_local",
                "padding",
                "register_shaping",
                "automatic_retention_or_promotion",
            ],
            "negative_controls": context["negative_controls"],
            "exact_result": context["exact_result"],
            "telemetry": context["telemetry"],
            "report_artifact_sha256": context["report_artifact_sha256"],
            "authority_advanced": False,
        },
    }
