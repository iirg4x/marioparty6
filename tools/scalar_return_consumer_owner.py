#!/usr/bin/env python3
"""Fail-closed scalar helper-return to saved-consumer owner diagnosis."""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "scalar_return_consumer_owner_context/v1"
RULE_ID = "scalar_return_consumer_owner_chain"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FPR_RE = re.compile(r"f(?:[0-9]|[12][0-9]|3[01])")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_FMR_RE = re.compile(
    r"^\s*fmr\s+(?P<destination>f(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<source>f(?:[0-9]|[12][0-9]|3[01]))\s*$",
    re.IGNORECASE,
)
_FPR_MEMORY_RE = re.compile(
    r"^\s*(?P<opcode>stfd|lfd)\s+(?P<register>f(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*(?P<base>r1)\s*\)\s*$",
    re.IGNORECASE,
)

_PROOF_FLAGS = (
    "cfg_calls_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "target_copy_use_save_chain_authenticated",
    "source_aware_trace_authenticated",
    "input_owner_exact",
    "call_result_owner_exact",
    "missing_consumer_owner_isolated",
    "negative_controls_measured",
    "protected_siblings_preserved",
    "exact_result_verified",
)
_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "trace_envelope_sha256",
    "trace_causal_receipt_sha256",
    "exact_source_sha256",
    "exact_object_sha256",
    "exact_strict_report_sha256",
    "exact_data_report_sha256",
    "exact_record_sha256",
    "report_artifact_sha256",
)
_CONTROL_CLASSES = {
    "lexical_scope_or_spelling": "object_identical",
    "call_result_assignment_fusion": "regressed",
    "declaration_chronology": "exhausted_neutral_or_regressed",
    "existing_input_owner_copy": "wrong_owner",
    "complementary_existing_owner_copy": "dominated_wrong_chain",
    "consumer_boundary_existing_owner_assignment": "wrong_owner",
}


class ScalarReturnOwnerInputError(ValueError):
    """The supplied evidence cannot support the rule safely."""


def _closed(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ScalarReturnOwnerInputError(f"{label} must be a JSON object")
    fields = set(value)
    missing = required - fields
    extra = fields - allowed
    if missing or extra:
        raise ScalarReturnOwnerInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ScalarReturnOwnerInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise ScalarReturnOwnerInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise ScalarReturnOwnerInputError(f"{label} must be a C source identifier")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise ScalarReturnOwnerInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ScalarReturnOwnerInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise ScalarReturnOwnerInputError(f"{label} must be from {minimum} through {maximum}")
    return value


def _positive_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ScalarReturnOwnerInputError(f"{label} must be a positive finite number")
    result = float(value)
    if not math.isfinite(result) or result <= 0.0:
        raise ScalarReturnOwnerInputError(f"{label} must be a positive finite number")
    return result


def _rows(value: Any, label: str) -> list[int]:
    if not isinstance(value, list) or not value:
        raise ScalarReturnOwnerInputError(f"{label} must be a non-empty row array")
    rows = [_uint(item, label, maximum=1 << 20) for item in value]
    if rows != sorted(set(rows)) or len(rows) > 32:
        raise ScalarReturnOwnerInputError(f"{label} must be sorted, unique, and bounded")
    return rows


def _fpr(value: Any, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if _FPR_RE.fullmatch(result) is None:
        raise ScalarReturnOwnerInputError(f"{label} must be an FPR")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "scalar return consumer-owner context"
    fields = {
        "schema",
        "proofs",
        "precursor",
        "target_chain",
        "source_trace",
        "controls",
        "telemetry",
        "exact_result",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise ScalarReturnOwnerInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")

    proof_fields = set(_PROOF_FLAGS) | set(_PROOF_HASHES)
    proofs = _closed(
        context.get("proofs"), allowed=proof_fields, required=proof_fields, label=f"{label}.proofs"
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise ScalarReturnOwnerInputError(f"{label}.proofs.{field} must be true")
        normalized_proofs[field] = True
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
    target_bytes = _uint(
        precursor.get("target_bytes"), f"{label}.precursor.target_bytes", minimum=24
    )
    candidate_bytes = _uint(
        precursor.get("candidate_bytes"), f"{label}.precursor.candidate_bytes", minimum=4
    )
    target_frame = _uint(
        precursor.get("target_frame"), f"{label}.precursor.target_frame", minimum=16
    )
    candidate_frame = _uint(
        precursor.get("candidate_frame"), f"{label}.precursor.candidate_frame", minimum=16
    )
    match_percent = _positive_number(
        precursor.get("match_percent"), f"{label}.precursor.match_percent"
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
    if target_bytes - candidate_bytes != 20 or target_frame - candidate_frame != 16:
        raise ScalarReturnOwnerInputError(
            f"{label}.precursor must have the sealed +20-byte/+16-frame saved-FPR chain"
        )
    if match_percent >= 100.0:
        raise ScalarReturnOwnerInputError(f"{label}.precursor.match_percent must be nonexact")
    if target_relocations != candidate_relocations:
        raise ScalarReturnOwnerInputError(f"{label}.precursor physical relocations must be exact")
    normalized_precursor = {
        "function": _identifier(precursor.get("function"), f"{label}.precursor.function"),
        "candidate_id": _text(
            precursor.get("candidate_id"), f"{label}.precursor.candidate_id", limit=128
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocations,
        "residual_rows": _rows(
            precursor.get("residual_rows"), f"{label}.precursor.residual_rows"
        ),
    }

    chain_fields = {
        "call_symbol",
        "input_owner",
        "input_register",
        "call_result_owner",
        "call_result_register",
        "consumer_owner",
        "consumer_register",
        "return_register",
        "copy_opcode",
        "use_opcode",
        "save_opcode",
        "restore_opcode",
        "stack_slot_offset",
    }
    chain = _closed(
        context.get("target_chain"),
        allowed=chain_fields,
        required=chain_fields,
        label=f"{label}.target_chain",
    )
    normalized_chain = {
        "call_symbol": _identifier(chain.get("call_symbol"), f"{label}.target_chain.call_symbol"),
        "input_owner": _identifier(chain.get("input_owner"), f"{label}.target_chain.input_owner"),
        "input_register": _fpr(chain.get("input_register"), f"{label}.target_chain.input_register"),
        "call_result_owner": _identifier(
            chain.get("call_result_owner"), f"{label}.target_chain.call_result_owner"
        ),
        "call_result_register": _fpr(
            chain.get("call_result_register"), f"{label}.target_chain.call_result_register"
        ),
        "consumer_owner": _identifier(
            chain.get("consumer_owner"), f"{label}.target_chain.consumer_owner"
        ),
        "consumer_register": _fpr(
            chain.get("consumer_register"), f"{label}.target_chain.consumer_register"
        ),
        "return_register": _fpr(
            chain.get("return_register"), f"{label}.target_chain.return_register"
        ),
        "copy_opcode": _text(chain.get("copy_opcode"), f"{label}.target_chain.copy_opcode").lower(),
        "use_opcode": _text(chain.get("use_opcode"), f"{label}.target_chain.use_opcode").lower(),
        "save_opcode": _text(chain.get("save_opcode"), f"{label}.target_chain.save_opcode").lower(),
        "restore_opcode": _text(
            chain.get("restore_opcode"), f"{label}.target_chain.restore_opcode"
        ).lower(),
        "stack_slot_offset": _uint(
            chain.get("stack_slot_offset"), f"{label}.target_chain.stack_slot_offset", minimum=8
        ),
    }
    registers = {
        normalized_chain["input_register"],
        normalized_chain["call_result_register"],
        normalized_chain["consumer_register"],
        normalized_chain["return_register"],
    }
    if len(registers) != 4:
        raise ScalarReturnOwnerInputError(f"{label}.target_chain FPR roles must be distinct")
    if normalized_chain["copy_opcode"] != "fmr":
        raise ScalarReturnOwnerInputError(f"{label}.target_chain.copy_opcode must be fmr")
    if normalized_chain["save_opcode"] != "stfd" or normalized_chain["restore_opcode"] != "lfd":
        raise ScalarReturnOwnerInputError(f"{label}.target_chain must use stfd/lfd saved-FPR ownership")
    if normalized_chain["use_opcode"] not in {"fmuls", "fmadds", "fadds", "fsubs", "fdivs"}:
        raise ScalarReturnOwnerInputError(f"{label}.target_chain.use_opcode is unsupported")

    trace_fields = {
        "same_session",
        "authority_advanced",
        "seam_unknown_count",
        "input_owner_status",
        "input_owner_register",
        "call_result_owner_status",
        "call_result_owner_register",
        "consumer_owner_status",
        "consumer_target_register",
        "event_count",
        "envelope_sha256",
        "causal_receipt_sha256",
    }
    trace = _closed(
        context.get("source_trace"),
        allowed=trace_fields,
        required=trace_fields,
        label=f"{label}.source_trace",
    )
    if trace.get("same_session") is not True or trace.get("authority_advanced") is not False:
        raise ScalarReturnOwnerInputError(f"{label}.source_trace must be same-session and authority-free")
    if _uint(trace.get("seam_unknown_count"), f"{label}.source_trace.seam_unknown_count") != 0:
        raise ScalarReturnOwnerInputError(f"{label}.source_trace seam must contain zero UNKNOWN")
    if _text(trace.get("input_owner_status"), f"{label}.source_trace.input_owner_status") != "EXACT":
        raise ScalarReturnOwnerInputError(f"{label}.source_trace input owner must be EXACT")
    if _text(
        trace.get("call_result_owner_status"), f"{label}.source_trace.call_result_owner_status"
    ) != "EXACT":
        raise ScalarReturnOwnerInputError(f"{label}.source_trace call-result owner must be EXACT")
    if _text(
        trace.get("consumer_owner_status"), f"{label}.source_trace.consumer_owner_status"
    ) != "TARGET_ONLY_MISSING":
        raise ScalarReturnOwnerInputError(
            f"{label}.source_trace consumer owner must be TARGET_ONLY_MISSING"
        )
    normalized_trace = {
        "same_session": True,
        "authority_advanced": False,
        "seam_unknown_count": 0,
        "input_owner_status": "EXACT",
        "input_owner_register": _fpr(
            trace.get("input_owner_register"), f"{label}.source_trace.input_owner_register"
        ),
        "call_result_owner_status": "EXACT",
        "call_result_owner_register": _fpr(
            trace.get("call_result_owner_register"),
            f"{label}.source_trace.call_result_owner_register",
        ),
        "consumer_owner_status": "TARGET_ONLY_MISSING",
        "consumer_target_register": _fpr(
            trace.get("consumer_target_register"),
            f"{label}.source_trace.consumer_target_register",
        ),
        "event_count": _uint(
            trace.get("event_count"), f"{label}.source_trace.event_count", minimum=1
        ),
        "envelope_sha256": _sha256(
            trace.get("envelope_sha256"), f"{label}.source_trace.envelope_sha256"
        ),
        "causal_receipt_sha256": _sha256(
            trace.get("causal_receipt_sha256"), f"{label}.source_trace.causal_receipt_sha256"
        ),
    }
    if (
        normalized_trace["input_owner_register"] != normalized_chain["input_register"]
        or normalized_trace["call_result_owner_register"]
        != normalized_chain["call_result_register"]
        or normalized_trace["consumer_target_register"] != normalized_chain["consumer_register"]
    ):
        raise ScalarReturnOwnerInputError(f"{label}.source_trace owner registers drift from target chain")
    if (
        normalized_trace["envelope_sha256"] != normalized_proofs["trace_envelope_sha256"]
        or normalized_trace["causal_receipt_sha256"]
        != normalized_proofs["trace_causal_receipt_sha256"]
    ):
        raise ScalarReturnOwnerInputError(f"{label}.source_trace hashes drift from proofs")

    raw_controls = context.get("controls")
    if not isinstance(raw_controls, list) or len(raw_controls) != len(_CONTROL_CLASSES):
        raise ScalarReturnOwnerInputError(f"{label}.controls must contain six sealed controls")
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
            raise ScalarReturnOwnerInputError(
                f"{label}.controls do not match the sealed negative-control matrix"
            )
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
        raise ScalarReturnOwnerInputError(f"{label}.controls must cover every sealed control")
    controls.sort(key=lambda item: item["kind"])

    telemetry_fields = {
        "active_seconds",
        "telemetry_complete",
        "exclude_from_measured_crack_hour",
        "telemetry_sha256",
    }
    telemetry = _closed(
        context.get("telemetry"),
        allowed=telemetry_fields,
        required=telemetry_fields,
        label=f"{label}.telemetry",
    )
    telemetry_complete = telemetry.get("telemetry_complete")
    excluded = telemetry.get("exclude_from_measured_crack_hour")
    if not isinstance(telemetry_complete, bool) or not isinstance(excluded, bool):
        raise ScalarReturnOwnerInputError(f"{label}.telemetry completeness fields must be booleans")
    if not telemetry_complete and not excluded:
        raise ScalarReturnOwnerInputError(f"{label}.telemetry incomplete evidence must be excluded")
    normalized_telemetry = {
        "active_seconds": _positive_number(
            telemetry.get("active_seconds"), f"{label}.telemetry.active_seconds"
        ),
        "telemetry_complete": telemetry_complete,
        "exclude_from_measured_crack_hour": excluded,
        "telemetry_sha256": _sha256(
            telemetry.get("telemetry_sha256"), f"{label}.telemetry.telemetry_sha256"
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
        exact.get("physical_relocations"),
        f"{label}.exact_result.physical_relocations",
        minimum=1,
    )
    if (
        exact_target != exact_candidate
        or exact_target != target_bytes
        or exact_relocations != target_relocations
    ):
        raise ScalarReturnOwnerInputError(
            f"{label}.exact_result must close target size and preserve relocations"
        )
    normalized_exact: dict[str, Any] = {
        "candidate_id": _text(
            exact.get("candidate_id"), f"{label}.exact_result.candidate_id", limit=128
        ),
        "target_bytes": exact_target,
        "candidate_bytes": exact_candidate,
        "physical_relocations": exact_relocations,
    }
    for field in (
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "candidate_record_sha256",
    ):
        normalized_exact[field] = _sha256(exact.get(field), f"{label}.exact_result.{field}")
    if (
        normalized_exact["source_sha256"] != normalized_proofs["exact_source_sha256"]
        or normalized_exact["object_sha256"] != normalized_proofs["exact_object_sha256"]
        or normalized_exact["strict_report_sha256"]
        != normalized_proofs["exact_strict_report_sha256"]
        or normalized_exact["data_report_sha256"]
        != normalized_proofs["exact_data_report_sha256"]
        or normalized_exact["candidate_record_sha256"] != normalized_proofs["exact_record_sha256"]
    ):
        raise ScalarReturnOwnerInputError(f"{label}.exact_result hashes drift from proofs")

    return {
        "schema": CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "precursor": normalized_precursor,
        "target_chain": normalized_chain,
        "source_trace": normalized_trace,
        "controls": controls,
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


def _call_indices(instructions: Sequence[Any], symbol: str) -> list[int]:
    return [
        index
        for index, instruction in enumerate(instructions)
        if instruction.has_instruction
        and instruction.mnemonic in {"bl", "bla"}
        and re.search(rf"\b{re.escape(symbol)}\b", instruction.formatted) is not None
    ]


def _fmr_indices(instructions: Sequence[Any], destination: str, source: str) -> list[int]:
    result: list[int] = []
    for index, instruction in enumerate(instructions):
        if not instruction.has_instruction:
            continue
        match = _FMR_RE.fullmatch(instruction.formatted)
        if match is not None and (
            match.group("destination").lower(), match.group("source").lower()
        ) == (destination, source):
            result.append(index)
    return result


def _mismatch_rows(
    target: Sequence[Any], candidate: Sequence[Any]
) -> list[tuple[int, Any | None, Any | None]]:
    result: list[tuple[int, Any | None, Any | None]] = []
    for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate)):
        if causal_reducer._instruction_mismatch(left, right):
            result.append((index, left, right))
    return result


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
            "reason": "no authenticated scalar return consumer-owner context was supplied",
        }
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return {
            "matched": False,
            "reason": "the scalar return consumer-owner context is bound to another objdiff report",
        }

    precursor = context["precursor"]
    if pair.name != precursor["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = (
        causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    )
    observed = (
        target_size,
        candidate_size,
        _frame_size(target),
        _frame_size(candidate),
    )
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
            "reason": "the physical residual rows differ from the sealed precursor",
            "evidence": {
                "report_residual_rows": mismatch_indices,
                "context_residual_rows": precursor["residual_rows"],
            },
        }

    chain = context["target_chain"]
    target_only = [
        item
        for item in mismatches
        if item[1] is not None
        and item[1].has_instruction
        and (item[2] is None or not item[2].has_instruction)
    ]
    paired = [
        item
        for item in mismatches
        if item[1] is not None
        and item[1].has_instruction
        and item[2] is not None
        and item[2].has_instruction
    ]
    if len(target_only) != 3 or len(paired) < 2:
        return {
            "matched": False,
            "reason": "the report must contain three target-only saved-owner rows plus frame/use/cascade pairs",
        }

    save: tuple[int, Any] | None = None
    copy: tuple[int, Any] | None = None
    restore: tuple[int, Any] | None = None
    for row, instruction, _ in target_only:
        fmr = _FMR_RE.fullmatch(instruction.formatted)
        memory = _FPR_MEMORY_RE.fullmatch(instruction.formatted)
        if fmr is not None and (
            fmr.group("destination").lower(), fmr.group("source").lower()
        ) == (chain["consumer_register"], chain["call_result_register"]):
            copy = (row, instruction)
        elif memory is not None and (
            memory.group("opcode").lower(),
            memory.group("register").lower(),
            int(memory.group("offset"), 0),
        ) == (
            chain["save_opcode"],
            chain["consumer_register"],
            chain["stack_slot_offset"],
        ):
            save = (row, instruction)
        elif memory is not None and (
            memory.group("opcode").lower(),
            memory.group("register").lower(),
            int(memory.group("offset"), 0),
        ) == (
            chain["restore_opcode"],
            chain["consumer_register"],
            chain["stack_slot_offset"],
        ):
            restore = (row, instruction)
    if save is None or copy is None or restore is None:
        return {
            "matched": False,
            "reason": "the target-only rows do not form the sealed save/copy/restore consumer chain",
        }

    frame_rows = [
        item
        for item in paired
        if item[1].mnemonic == "stwu" and item[2].mnemonic == "stwu"
    ]
    use_rows = [
        item
        for item in paired
        if item[1].mnemonic == chain["use_opcode"]
        and item[2].mnemonic == chain["use_opcode"]
        and re.search(rf"\b{re.escape(chain['consumer_register'])}\b", item[1].formatted)
        and re.search(rf"\b{re.escape(chain['call_result_register'])}\b", item[2].formatted)
        and re.search(rf"\b{re.escape(chain['input_register'])}\b", item[1].formatted)
        and re.search(rf"\b{re.escape(chain['input_register'])}\b", item[2].formatted)
    ]
    if len(frame_rows) != 1 or len(use_rows) != 1:
        return {
            "matched": False,
            "reason": "the paired rows do not isolate the sealed frame and consumer-register substitution",
        }
    frozen_registers = {chain["input_register"], chain["call_result_register"]}
    cascade_rows = [item for item in paired if item not in frame_rows and item not in use_rows]
    cascade_roles: set[tuple[str, str]] = set()
    for row, left, right in cascade_rows:
        left_memory = _FPR_MEMORY_RE.fullmatch(left.formatted)
        right_memory = _FPR_MEMORY_RE.fullmatch(right.formatted)
        if left_memory is None or right_memory is None:
            return {
                "matched": False,
                "reason": "a paired residual outside frame/use is not a frozen saved-FPR offset cascade",
                "evidence": {"row": row},
            }
        left_tuple = (
            left_memory.group("opcode").lower(),
            left_memory.group("register").lower(),
            left_memory.group("base").lower(),
        )
        right_tuple = (
            right_memory.group("opcode").lower(),
            right_memory.group("register").lower(),
            right_memory.group("base").lower(),
        )
        if (
            left_tuple != right_tuple
            or left_tuple[1] not in frozen_registers
            or int(left_memory.group("offset"), 0) - int(right_memory.group("offset"), 0) != 16
        ):
            return {
                "matched": False,
                "reason": "a saved-FPR cascade row changes more than the sealed +16 frame offset",
                "evidence": {"row": row},
            }
        cascade_roles.add((left_tuple[0], left_tuple[1]))
    expected_cascade_roles = {
        (opcode, register)
        for opcode in (chain["save_opcode"], chain["restore_opcode"])
        for register in frozen_registers
    }
    if cascade_roles != expected_cascade_roles or len(cascade_rows) != 4:
        return {
            "matched": False,
            "reason": "the frozen input/call-result save and restore cascade is incomplete or ambiguous",
        }

    target_calls = _call_indices(target, chain["call_symbol"])
    candidate_calls = _call_indices(candidate, chain["call_symbol"])
    target_bind = _fmr_indices(
        target, chain["call_result_register"], chain["return_register"]
    )
    candidate_bind = _fmr_indices(
        candidate, chain["call_result_register"], chain["return_register"]
    )
    if any(len(items) != 1 for items in (target_calls, candidate_calls, target_bind, candidate_bind)):
        return {
            "matched": False,
            "reason": "the scalar helper call and return-owner bind must occur exactly once in both lanes",
        }
    use_row = use_rows[0][0]
    if not (
        save[0] < target_calls[0] < target_bind[0] < copy[0] < use_row < restore[0]
        and candidate_calls[0] < candidate_bind[0] < use_row
    ):
        return {
            "matched": False,
            "reason": "the target save/call/bind/copy/use/restore order is not the sealed chain",
        }

    input_owner = chain["input_owner"]
    result_owner = chain["call_result_owner"]
    consumer_owner = chain["consumer_owner"]
    evidence = {
        "precursor": precursor,
        "target_chain": chain,
        "source_trace": context["source_trace"],
        "target_rows": {
            "save": save[0],
            "copy": copy[0],
            "use": use_row,
            "restore": restore[0],
            "frozen_save_restore_cascade": [item[0] for item in cascade_rows],
        },
        "negative_controls": context["controls"],
        "recommended_cells": [
            {
                "kind": "separate_typed_post_call_consumer_owner",
                "call_assignment": f"{result_owner} = {chain['call_symbol']}({input_owner})",
                "copy_assignment": f"{consumer_owner} = {result_owner}",
                "consumer_owner": consumer_owner,
                "preserve_all_other_source_axes": True,
            }
        ],
        "suppressed_axes": [
            "lexical_scope_or_spelling",
            "call_result_assignment_fusion",
            "declaration_permutations",
            "existing_input_owner_copy",
            "complementary_existing_owner_copy",
            "consumer_boundary_existing_owner_assignment",
            "dead_or_fake_consumer_owner",
            "padding",
            "register_shaping",
            "repeat_tracer_capture",
            "automatic_retention",
        ],
        "telemetry": context["telemetry"],
        "combined_exact_result": context["exact_result"],
        "proofs": context["proofs"],
        "authority_advanced": False,
    }
    return {
        "matched": True,
        "reason": "exact static structure plus a complete target-only saved-FPR copy/use chain and same-session owner proof isolate one missing typed post-call consumer identity",
        "confidence": 0.997,
        "source_class": "scalar_return_to_distinct_saved_consumer_owner",
        "recommendation": (
            f"Preserve {input_owner} and {result_owner}; compile only the distinct live "
            f"{consumer_owner} = {result_owner} consumer-owner cell."
        ),
        "evidence": evidence,
    }
