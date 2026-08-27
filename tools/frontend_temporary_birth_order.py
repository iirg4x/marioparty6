#!/usr/bin/env python3
"""Diagnose closed MWCC frontend-temporary stack-home permutations.

The reducer consumes a compact ``focus_symbol_report/v1`` plus a hash-bound
``frontend_temporary_birth_order_context/v1`` manifest. It verifies that every
residual row is a stack-home-only argument mismatch, proves the observed and
proposed home orders against reverse frontend creation order, and ranks one
bounded consumer-boundary source class.

This tool is diagnostic only. It emits no source, retains no candidate, and
advances no recovery, relocation, or promotion authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
import tempfile
from pathlib import Path
from typing import Any, Mapping, Sequence


SCHEMA = "frontend_temporary_birth_order/v1"
CONTEXT_SCHEMA = "frontend_temporary_birth_order_context/v1"
ROUTE = "closed_reverse_frontend_temporary_birth_order"
_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_MEMORY_RE = re.compile(
    r"^(?P<opcode>lbz|lhz|lha|lwz|lfs|lfd|stb|sth|stw|stfs|stfd)\s+"
    r"(?P<value>[^,]+),\s*(?P<offset>[+-]?(?:0x[0-9a-f]+|[0-9]+))"
    r"\((?P<base>r(?:[12]?[0-9]|3[01]))\)$",
    re.IGNORECASE,
)
_STORE_OPCODES = {"stb", "sth", "stw", "stfs", "stfd"}
_LOAD_OPCODES = {"lbz", "lhz", "lha", "lwz", "lfs", "lfd"}
_TEMPORARY_KINDS = {"compiler_call_temporary", "live_typed_address"}
_PHYSICAL_STATUSES = {"exact", "unknown"}
_CONTROL_OUTCOMES = {
    "neutral",
    "object_identical",
    "regressed",
    "size_drift",
    "topology_drift",
}


class TemporaryBirthInputError(ValueError):
    """Raised when an evidence artifact is malformed or not hash-bound."""


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise TemporaryBirthInputError(f"{label} must be an object")
    return value


def _sequence(value: Any, label: str) -> list[Any]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)):
        raise TemporaryBirthInputError(f"{label} must be an array")
    return list(value)


def _closed(
    value: Any,
    label: str,
    *,
    required: set[str],
    optional: set[str] | None = None,
) -> Mapping[str, Any]:
    row = _mapping(value, label)
    allowed = required | (optional or set())
    missing = sorted(required - set(row))
    extra = sorted(set(row) - allowed)
    if missing:
        raise TemporaryBirthInputError(
            f"{label} missing fields: {', '.join(missing)}"
        )
    if extra:
        raise TemporaryBirthInputError(
            f"{label} has unknown fields: {', '.join(extra)}"
        )
    return row


def _text(value: Any, label: str, *, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip():
        raise TemporaryBirthInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise TemporaryBirthInputError(f"{label} exceeds {limit} characters")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise TemporaryBirthInputError(
            f"{label} must be a lowercase SHA-256 digest"
        )
    return result


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool):
        raise TemporaryBirthInputError(f"{label} must be an integer")
    try:
        result = int(value)
    except (TypeError, ValueError) as exc:
        raise TemporaryBirthInputError(f"{label} must be an integer") from exc
    if result < minimum:
        raise TemporaryBirthInputError(f"{label} must be >= {minimum}")
    return result


def _boolean(value: Any, label: str) -> bool:
    if not isinstance(value, bool):
        raise TemporaryBirthInputError(f"{label} must be boolean")
    return value


def _home(value: Any, label: str) -> int:
    if isinstance(value, bool):
        raise TemporaryBirthInputError(f"{label} must be a stack offset")
    try:
        result = int(value, 0) if isinstance(value, str) else int(value)
    except (TypeError, ValueError) as exc:
        raise TemporaryBirthInputError(
            f"{label} must be a decimal or hexadecimal stack offset"
        ) from exc
    if result < 0 or result % 4:
        raise TemporaryBirthInputError(
            f"{label} must be a non-negative four-byte-aligned stack offset"
        )
    return result


def _indices(value: Any, label: str) -> list[int]:
    result = [
        _integer(item, f"{label}[{index}]")
        for index, item in enumerate(_sequence(value, label))
    ]
    if len(result) != 2:
        raise TemporaryBirthInputError(
            f"{label} must contain exactly one store/load row pair"
        )
    if len(set(result)) != len(result):
        raise TemporaryBirthInputError(f"{label} contains duplicate rows")
    return result


def _parse_physical(value: Any) -> dict[str, Any]:
    physical = _closed(
        value,
        "context.preconditions.physical_relocations",
        required={"status"},
        optional={
            "target_count",
            "candidate_count",
            "difference_count",
            "receipt_sha256",
        },
    )
    status = _text(
        physical["status"],
        "context.preconditions.physical_relocations.status",
        limit=16,
    )
    if status not in _PHYSICAL_STATUSES:
        raise TemporaryBirthInputError(
            "physical relocation status must be exact or unknown"
        )
    result: dict[str, Any] = {"status": status}
    for key in ("target_count", "candidate_count", "difference_count"):
        if key in physical:
            result[key] = _integer(
                physical[key],
                f"context.preconditions.physical_relocations.{key}",
            )
    if physical.get("receipt_sha256") is not None:
        result["receipt_sha256"] = _sha256(
            physical["receipt_sha256"],
            "context.preconditions.physical_relocations.receipt_sha256",
        )
    if status == "exact":
        required_counts = {"target_count", "candidate_count", "difference_count"}
        if not required_counts <= set(result):
            raise TemporaryBirthInputError(
                "exact physical relocation evidence requires all counts"
            )
        if result["target_count"] != result["candidate_count"]:
            raise TemporaryBirthInputError(
                "exact physical relocation target/candidate counts differ"
            )
        if result["difference_count"] != 0:
            raise TemporaryBirthInputError(
                "exact physical relocation evidence has differences"
            )
    return result


def _parse_temporaries(value: Any) -> list[dict[str, Any]]:
    temporaries: list[dict[str, Any]] = []
    temporary_ids: set[str] = set()
    all_rows: set[int] = set()
    for index, raw_temporary in enumerate(_sequence(value, "context.temporaries")):
        label = f"context.temporaries[{index}]"
        temporary = _closed(
            raw_temporary,
            label,
            required={
                "id",
                "kind",
                "source_type",
                "row_indices",
                "candidate_home",
                "target_home",
                "current_birth_rank",
                "proposed_birth_rank",
                "producer",
                "consumer",
                "evaluation_order_sealed",
                "use_count",
            },
        )
        temporary_id = _text(temporary["id"], f"{label}.id", limit=128)
        if temporary_id in temporary_ids:
            raise TemporaryBirthInputError(
                f"duplicate temporary id {temporary_id!r}"
            )
        temporary_ids.add(temporary_id)
        kind = _text(temporary["kind"], f"{label}.kind", limit=64)
        if kind not in _TEMPORARY_KINDS:
            raise TemporaryBirthInputError(
                f"{label}.kind is unsupported: {kind}"
            )
        row_indices = _indices(temporary["row_indices"], f"{label}.row_indices")
        overlap = all_rows.intersection(row_indices)
        if overlap:
            raise TemporaryBirthInputError(
                f"{label}.row_indices overlap prior rows: {sorted(overlap)}"
            )
        all_rows.update(row_indices)
        temporaries.append(
            {
                "id": temporary_id,
                "kind": kind,
                "source_type": _text(
                    temporary["source_type"], f"{label}.source_type", limit=128
                ),
                "row_indices": row_indices,
                "candidate_home": _home(
                    temporary["candidate_home"], f"{label}.candidate_home"
                ),
                "target_home": _home(
                    temporary["target_home"], f"{label}.target_home"
                ),
                "current_birth_rank": _integer(
                    temporary["current_birth_rank"],
                    f"{label}.current_birth_rank",
                    minimum=1,
                ),
                "proposed_birth_rank": _integer(
                    temporary["proposed_birth_rank"],
                    f"{label}.proposed_birth_rank",
                    minimum=1,
                ),
                "producer": _text(
                    temporary["producer"], f"{label}.producer", limit=256
                ),
                "consumer": _text(
                    temporary["consumer"], f"{label}.consumer", limit=256
                ),
                "evaluation_order_sealed": _boolean(
                    temporary["evaluation_order_sealed"],
                    f"{label}.evaluation_order_sealed",
                ),
                "use_count": _integer(
                    temporary["use_count"], f"{label}.use_count", minimum=1
                ),
            }
        )
    if len(temporaries) < 3 or len(temporaries) > 8:
        raise TemporaryBirthInputError(
            "context.temporaries must contain between three and eight entries"
        )
    return temporaries


def _parse_boundary(value: Any, temporary_ids: set[str]) -> dict[str, Any]:
    boundary = _closed(
        value,
        "context.consumer_boundary",
        required={
            "temporary_id",
            "aggregate_type",
            "copy_expression",
            "typed_consumer",
            "aggregate_copy_required",
            "use_count",
            "current_source_class",
            "proposed_source_class",
        },
    )
    result = {
        "temporary_id": _text(
            boundary["temporary_id"],
            "context.consumer_boundary.temporary_id",
            limit=128,
        ),
        "aggregate_type": _text(
            boundary["aggregate_type"],
            "context.consumer_boundary.aggregate_type",
            limit=128,
        ),
        "copy_expression": _text(
            boundary["copy_expression"],
            "context.consumer_boundary.copy_expression",
            limit=256,
        ),
        "typed_consumer": _text(
            boundary["typed_consumer"],
            "context.consumer_boundary.typed_consumer",
            limit=256,
        ),
        "aggregate_copy_required": _boolean(
            boundary["aggregate_copy_required"],
            "context.consumer_boundary.aggregate_copy_required",
        ),
        "use_count": _integer(
            boundary["use_count"],
            "context.consumer_boundary.use_count",
            minimum=1,
        ),
        "current_source_class": _text(
            boundary["current_source_class"],
            "context.consumer_boundary.current_source_class",
            limit=128,
        ),
        "proposed_source_class": _text(
            boundary["proposed_source_class"],
            "context.consumer_boundary.proposed_source_class",
            limit=128,
        ),
    }
    if result["temporary_id"] not in temporary_ids:
        raise TemporaryBirthInputError(
            "context.consumer_boundary.temporary_id is not declared"
        )
    return result


def _parse_controls(value: Any) -> list[dict[str, str]]:
    controls: list[dict[str, str]] = []
    control_ids: set[str] = set()
    control_classes: set[str] = set()
    for index, raw_control in enumerate(
        _sequence(value, "context.negative_controls")
    ):
        label = f"context.negative_controls[{index}]"
        control = _closed(
            raw_control,
            label,
            required={"id", "source_class", "outcome", "evidence"},
        )
        control_id = _text(control["id"], f"{label}.id", limit=128)
        source_class = _text(
            control["source_class"], f"{label}.source_class", limit=128
        )
        outcome = _text(control["outcome"], f"{label}.outcome", limit=32)
        if control_id in control_ids:
            raise TemporaryBirthInputError(f"duplicate control id {control_id!r}")
        if source_class in control_classes:
            raise TemporaryBirthInputError(
                f"duplicate negative-control source class {source_class!r}"
            )
        if outcome not in _CONTROL_OUTCOMES:
            raise TemporaryBirthInputError(
                f"{label}.outcome is unsupported: {outcome}"
            )
        control_ids.add(control_id)
        control_classes.add(source_class)
        controls.append(
            {
                "id": control_id,
                "source_class": source_class,
                "outcome": outcome,
                "evidence": _text(
                    control["evidence"], f"{label}.evidence", limit=512
                ),
            }
        )
    return controls


def _parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed(
        value,
        "context",
        required={
            "schema",
            "function",
            "focus_file_sha256",
            "focus_artifact_sha256",
            "report_sha256",
            "candidate_source_sha256",
            "candidate_object_sha256",
            "target_object_sha256",
            "protected_siblings_zero_loss",
            "preconditions",
            "temporaries",
            "consumer_boundary",
            "negative_controls",
        },
    )
    if context["schema"] != CONTEXT_SCHEMA:
        raise TemporaryBirthInputError(
            f"context.schema must be {CONTEXT_SCHEMA!r}"
        )
    preconditions = _closed(
        context["preconditions"],
        "context.preconditions",
        required={
            "function_size_exact",
            "cfg_exact",
            "calls_exact",
            "data_values_exact",
            "physical_relocations",
        },
    )
    temporaries = _parse_temporaries(context["temporaries"])
    boundary = _parse_boundary(
        context["consumer_boundary"], {item["id"] for item in temporaries}
    )
    return {
        "schema": CONTEXT_SCHEMA,
        "function": _text(context["function"], "context.function", limit=256),
        "focus_file_sha256": _sha256(
            context["focus_file_sha256"], "context.focus_file_sha256"
        ),
        "focus_artifact_sha256": _sha256(
            context["focus_artifact_sha256"],
            "context.focus_artifact_sha256",
        ),
        "report_sha256": _sha256(
            context["report_sha256"], "context.report_sha256"
        ),
        "candidate_source_sha256": _sha256(
            context["candidate_source_sha256"],
            "context.candidate_source_sha256",
        ),
        "candidate_object_sha256": _sha256(
            context["candidate_object_sha256"],
            "context.candidate_object_sha256",
        ),
        "target_object_sha256": _sha256(
            context["target_object_sha256"],
            "context.target_object_sha256",
        ),
        "protected_siblings_zero_loss": _boolean(
            context["protected_siblings_zero_loss"],
            "context.protected_siblings_zero_loss",
        ),
        "preconditions": {
            "function_size_exact": _boolean(
                preconditions["function_size_exact"],
                "context.preconditions.function_size_exact",
            ),
            "cfg_exact": _boolean(
                preconditions["cfg_exact"], "context.preconditions.cfg_exact"
            ),
            "calls_exact": _boolean(
                preconditions["calls_exact"],
                "context.preconditions.calls_exact",
            ),
            "data_values_exact": _boolean(
                preconditions["data_values_exact"],
                "context.preconditions.data_values_exact",
            ),
            "physical_relocations": _parse_physical(
                preconditions["physical_relocations"]
            ),
        },
        "temporaries": temporaries,
        "consumer_boundary": boundary,
        "negative_controls": _parse_controls(context["negative_controls"]),
    }


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def _canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical_bytes(value)).hexdigest()


def _file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise TemporaryBirthInputError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _json_file(path: Path, label: str) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise TemporaryBirthInputError(f"cannot read {label} {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise TemporaryBirthInputError(
            f"{label} {path} is not valid JSON: {exc}"
        ) from exc
    return _mapping(value, label)


def _rows(side: Mapping[str, Any], label: str) -> dict[int, Mapping[str, Any]]:
    result: dict[int, Mapping[str, Any]] = {}
    for position, raw_row in enumerate(_sequence(side.get("rows"), f"{label}.rows")):
        row = _mapping(raw_row, f"{label}.rows[{position}]")
        index = _integer(row.get("index"), f"{label}.rows[{position}].index")
        if index in result:
            raise TemporaryBirthInputError(f"{label}.rows has duplicate index {index}")
        result[index] = row
    return result


def _candidate_diff_rows(
    side: Mapping[str, Any], label: str
) -> dict[int, Mapping[str, Any]]:
    rows = _rows(side, label)
    if side.get("rows_kind") == "diff_only":
        return rows
    return {
        index: row
        for index, row in rows.items()
        if row.get("diff_kind") not in {None, "DIFF_NONE"}
    }


def _formatted(row: Mapping[str, Any], label: str) -> str:
    instruction = _mapping(row.get("instruction"), f"{label}.instruction")
    return _text(instruction.get("formatted"), f"{label}.instruction.formatted")


def _memory(text: str) -> dict[str, Any] | None:
    match = _MEMORY_RE.fullmatch(text.strip())
    if match is None:
        return None
    return {
        "opcode": match.group("opcode").lower(),
        "value": match.group("value").strip().lower(),
        "offset": int(match.group("offset"), 0),
        "base": match.group("base").lower(),
    }


def _channel(
    focus: Mapping[str, Any], channel_name: str
) -> tuple[Mapping[str, Any], dict[int, Mapping[str, Any]], dict[int, Mapping[str, Any]]]:
    channels = _mapping(focus.get("channels"), "focus.channels")
    channel = _mapping(channels.get(channel_name), f"focus.channels.{channel_name}")
    metric = _mapping(channel.get("metric"), f"focus.channels.{channel_name}.metric")
    target = _mapping(channel.get("target"), f"focus.channels.{channel_name}.target")
    candidate = _mapping(
        channel.get("candidate"), f"focus.channels.{channel_name}.candidate"
    )
    return (
        metric,
        _rows(target, f"focus.channels.{channel_name}.target"),
        _candidate_diff_rows(
            candidate, f"focus.channels.{channel_name}.candidate"
        ),
    )


def _analyze_channels(
    focus: Mapping[str, Any], blockers: list[str]
) -> tuple[dict[str, dict[str, Any]], set[int], dict[int, dict[str, Any]]]:
    details: dict[str, dict[str, Any]] = {}
    strict_indices: set[int] = set()
    row_pairs: dict[int, dict[str, Any]] = {}
    for channel_name in ("strict", "data"):
        metric, target_rows, candidate_rows = _channel(focus, channel_name)
        target_size = _integer(
            metric.get("target_size"),
            f"focus.channels.{channel_name}.metric.target_size",
        )
        candidate_size = _integer(
            metric.get("candidate_size"),
            f"focus.channels.{channel_name}.metric.candidate_size",
        )
        if target_size != candidate_size:
            blockers.append(f"{channel_name}_function_size_differs")
        candidate_indices = set(candidate_rows)
        metric_rows = _integer(
            metric.get("diff_rows"),
            f"focus.channels.{channel_name}.metric.diff_rows",
        )
        if metric_rows != len(candidate_indices):
            blockers.append(f"{channel_name}_metric_row_count_differs")
        if channel_name == "strict":
            strict_indices = candidate_indices
        elif candidate_indices != strict_indices:
            blockers.append("strict_and_data_residual_rows_differ")
        details[channel_name] = {
            "target_size": target_size,
            "candidate_size": candidate_size,
            "diff_row_count": len(candidate_indices),
        }
        for row_index in sorted(candidate_indices):
            candidate_row = candidate_rows[row_index]
            target_row = target_rows.get(row_index)
            if target_row is None:
                blockers.append(f"row_{row_index}_missing_target_instruction")
                continue
            if candidate_row.get("diff_kind") != "DIFF_ARG_MISMATCH":
                blockers.append(f"row_{row_index}_candidate_not_arg_mismatch")
            candidate_memory = _memory(
                _formatted(candidate_row, f"{channel_name}.candidate[{row_index}]")
            )
            target_memory = _memory(
                _formatted(target_row, f"{channel_name}.target[{row_index}]")
            )
            if candidate_memory is None or target_memory is None:
                blockers.append(f"row_{row_index}_not_supported_stack_memory")
                continue
            for field in ("opcode", "value", "base"):
                if candidate_memory[field] != target_memory[field]:
                    blockers.append(f"row_{row_index}_{field}_differs")
            if candidate_memory["base"] != "r1":
                blockers.append(f"row_{row_index}_base_not_r1")
            if candidate_memory["offset"] == target_memory["offset"]:
                blockers.append(f"row_{row_index}_stack_home_not_different")
            if channel_name == "strict":
                row_pairs[row_index] = {
                    "index": row_index,
                    "opcode": candidate_memory["opcode"],
                    "value": candidate_memory["value"],
                    "candidate_home": candidate_memory["offset"],
                    "target_home": target_memory["offset"],
                }
    return details, strict_indices, row_pairs


def build_diagnosis(
    focus_value: Mapping[str, Any], context_value: Mapping[str, Any]
) -> dict[str, Any]:
    """Build one deterministic fail-closed temporary-birth diagnosis."""

    focus = _mapping(focus_value, "focus")
    context = _parse_context(context_value)
    blockers: list[str] = []
    warnings: list[str] = ["exact_source_spelling_is_family_specific_and_not_emitted"]

    if focus.get("schema") != "focus_symbol_report/v1":
        raise TemporaryBirthInputError(
            "focus.schema must be 'focus_symbol_report/v1'"
        )
    if focus.get("authority_advanced") is not False:
        blockers.append("focus_authority_must_remain_false")
    if focus.get("function") != context["function"]:
        blockers.append("focus_function_differs_from_context")
    if focus.get("artifact_sha256") != context["focus_artifact_sha256"]:
        blockers.append("focus_artifact_identity_differs_from_context")
    if not context["protected_siblings_zero_loss"]:
        blockers.append("protected_siblings_not_zero_loss")
    for key in ("function_size_exact", "cfg_exact", "calls_exact", "data_values_exact"):
        if not context["preconditions"][key]:
            blockers.append(f"precondition_not_closed:{key}")
    physical = context["preconditions"]["physical_relocations"]
    if physical["status"] == "unknown":
        warnings.append("physical_relocation_authority_unknown")

    channel_details, strict_indices, row_pairs = _analyze_channels(focus, blockers)
    context_rows = {
        row_index
        for temporary in context["temporaries"]
        for row_index in temporary["row_indices"]
    }
    if strict_indices != context_rows:
        blockers.append("context_rows_do_not_cover_strict_residual")

    temporary_records: list[dict[str, Any]] = []
    for temporary in context["temporaries"]:
        pairs = [row_pairs.get(index) for index in temporary["row_indices"]]
        if any(pair is None for pair in pairs):
            blockers.append(f"temporary_{temporary['id']}_has_unparsed_row")
            continue
        parsed_pairs = [pair for pair in pairs if pair is not None]
        candidate_homes = {pair["candidate_home"] for pair in parsed_pairs}
        target_homes = {pair["target_home"] for pair in parsed_pairs}
        opcodes = {pair["opcode"] for pair in parsed_pairs}
        if candidate_homes != {temporary["candidate_home"]}:
            blockers.append(f"temporary_{temporary['id']}_candidate_home_differs")
        if target_homes != {temporary["target_home"]}:
            blockers.append(f"temporary_{temporary['id']}_target_home_differs")
        if not (opcodes & _STORE_OPCODES) or not (opcodes & _LOAD_OPCODES):
            blockers.append(f"temporary_{temporary['id']}_not_store_load_pair")
        if not temporary["evaluation_order_sealed"]:
            blockers.append(f"temporary_{temporary['id']}_evaluation_order_unsealed")
        temporary_records.append(
            {
                **temporary,
                "candidate_home_hex": f"0x{temporary['candidate_home']:x}",
                "target_home_hex": f"0x{temporary['target_home']:x}",
            }
        )

    count = len(context["temporaries"])
    required_ranks = set(range(1, count + 1))
    current_ranks = {item["current_birth_rank"] for item in context["temporaries"]}
    proposed_ranks = {item["proposed_birth_rank"] for item in context["temporaries"]}
    if current_ranks != required_ranks:
        blockers.append("current_birth_ranks_not_closed")
    if proposed_ranks != required_ranks:
        blockers.append("proposed_birth_ranks_not_closed")

    candidate_homes = {item["candidate_home"] for item in context["temporaries"]}
    target_homes = {item["target_home"] for item in context["temporaries"]}
    if candidate_homes != target_homes:
        blockers.append("candidate_target_homes_not_a_closed_permutation")
    if len(candidate_homes) != count:
        blockers.append("temporary_homes_not_unique")
    reverse_homes = sorted(candidate_homes, reverse=True)
    if len(reverse_homes) == count:
        for temporary in context["temporaries"]:
            expected_candidate = reverse_homes[temporary["current_birth_rank"] - 1]
            expected_target = reverse_homes[temporary["proposed_birth_rank"] - 1]
            if temporary["candidate_home"] != expected_candidate:
                blockers.append(
                    f"temporary_{temporary['id']}_candidate_not_reverse_birth_order"
                )
            if temporary["target_home"] != expected_target:
                blockers.append(
                    f"temporary_{temporary['id']}_target_not_reverse_birth_order"
                )

    typed_addresses = [
        item for item in context["temporaries"] if item["kind"] == "live_typed_address"
    ]
    call_temporaries = [
        item
        for item in context["temporaries"]
        if item["kind"] == "compiler_call_temporary"
    ]
    if len(typed_addresses) != 1:
        blockers.append("requires_exactly_one_live_typed_address")
    if len(call_temporaries) < 2:
        blockers.append("requires_at_least_two_compiler_call_temporaries")

    boundary = context["consumer_boundary"]
    boundary_temporary = next(
        (item for item in context["temporaries"] if item["id"] == boundary["temporary_id"]),
        None,
    )
    if boundary_temporary is None or boundary_temporary["kind"] != "live_typed_address":
        blockers.append("consumer_boundary_not_bound_to_live_typed_address")
    else:
        if boundary_temporary["use_count"] != 1 or boundary["use_count"] != 1:
            blockers.append("consumer_boundary_address_not_single_use")
        if boundary_temporary["proposed_birth_rank"] != count:
            blockers.append("consumer_boundary_address_not_latest_proposed_birth")
        if boundary_temporary["current_birth_rank"] == count:
            blockers.append("consumer_boundary_address_already_latest_birth")
    if not boundary["aggregate_copy_required"]:
        blockers.append("consumer_boundary_aggregate_copy_not_required")
    if boundary["current_source_class"] != "explicit_pointer_local":
        blockers.append("consumer_boundary_current_class_not_explicit_pointer_local")
    if boundary["proposed_source_class"] != "live_aggregate_copy_right_argument_temporary":
        blockers.append("consumer_boundary_proposed_class_not_supported")

    matched = not blockers
    predicted_homes = [
        {
            "id": item["id"],
            "kind": item["kind"],
            "proposed_birth_rank": item["proposed_birth_rank"],
            "target_home": f"0x{item['target_home']:x}",
            "producer": item["producer"],
            "consumer": item["consumer"],
        }
        for item in sorted(
            context["temporaries"], key=lambda item: item["proposed_birth_rank"]
        )
    ]
    candidate_cells = []
    if matched:
        candidate_cells.append(
            {
                "id": "compose_live_address_at_consumer_boundary",
                "source_class": boundary["proposed_source_class"],
                "temporary_id": boundary["temporary_id"],
                "action": (
                    "Preserve the live aggregate copy and materialize its typed "
                    "address inside the existing final call argument at the "
                    "consumer boundary."
                ),
                "typed_consumer": boundary["typed_consumer"],
                "aggregate_type": boundary["aggregate_type"],
                "evidence_row_indices": sorted(context_rows),
                "predicted_target_homes": predicted_homes,
                "requires_compile_proof": True,
            }
        )

    suppressed_axes = sorted(
        {
            "dead_or_fake_local",
            "declaration_order_permutation",
            "padding",
            "register_shaping",
            "source_link_closure_inside_this_reducer",
            *(control["source_class"] for control in context["negative_controls"]),
        }
    )
    result: dict[str, Any] = {
        "schema": SCHEMA,
        "status": "matched" if matched else "blocked",
        "route": ROUTE if matched else None,
        "function": context["function"],
        "binding": {
            "focus_file_sha256": context["focus_file_sha256"],
            "focus_artifact_sha256": context["focus_artifact_sha256"],
            "report_sha256": context["report_sha256"],
            "candidate_source_sha256": context["candidate_source_sha256"],
            "candidate_object_sha256": context["candidate_object_sha256"],
            "target_object_sha256": context["target_object_sha256"],
            "physical_relocations": physical,
            "protected_siblings_zero_loss": context["protected_siblings_zero_loss"],
        },
        "facts": {
            "strict_target_size": channel_details.get("strict", {}).get("target_size"),
            "strict_candidate_size": channel_details.get("strict", {}).get(
                "candidate_size"
            ),
            "strict_diff_row_count": channel_details.get("strict", {}).get(
                "diff_row_count"
            ),
            "data_diff_row_count": channel_details.get("data", {}).get("diff_row_count"),
            "temporary_count": count,
            "call_temporary_count": len(call_temporaries),
            "live_typed_address_count": len(typed_addresses),
            "closed_home_permutation": candidate_homes == target_homes,
            "all_rows_accounted": strict_indices == context_rows,
            "reverse_allocation_model": "latest_birth_receives_lowest_home",
        },
        "temporaries": temporary_records,
        "predicted_target_homes": predicted_homes if matched else [],
        "candidate_cells": candidate_cells,
        "compile_candidate_budget": 1 if matched else 0,
        "trace_budget": 0,
        "negative_controls": context["negative_controls"],
        "suppressed_axes": suppressed_axes,
        "source_link_handoff": "tools/source_linked_owner_closure.py",
        "warnings": warnings,
        "blockers": sorted(set(blockers)),
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["diagnosis_sha256"] = _canonical_sha256(result)
    return result


def build_from_paths(focus_path: Path, context_path: Path) -> dict[str, Any]:
    focus = _json_file(focus_path, "focus artifact")
    context = _parse_context(_json_file(context_path, "context"))
    observed_focus_sha256 = _file_sha256(focus_path)
    if observed_focus_sha256 != context["focus_file_sha256"]:
        raise TemporaryBirthInputError(
            "focus artifact file SHA-256 differs from context binding: "
            f"expected {context['focus_file_sha256']}, observed {observed_focus_sha256}"
        )
    return build_diagnosis(focus, context)


def _atomic_write(path: Path, text: str) -> None:
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        descriptor, temporary_name = tempfile.mkstemp(
            prefix=path.name + ".", suffix=".tmp", dir=path.parent
        )
        temporary = Path(temporary_name)
        try:
            with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
                stream.write(text)
            temporary.replace(path)
        except BaseException:
            temporary.unlink(missing_ok=True)
            raise
    except OSError as exc:
        raise TemporaryBirthInputError(f"cannot write {path}: {exc}") from exc


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Diagnose a closed MWCC frontend-temporary stack-home permutation "
            "without emitting source or authority."
        )
    )
    parser.add_argument("focus", type=Path, help="focus_symbol_report/v1 JSON")
    parser.add_argument("context", type=Path, help=f"{CONTEXT_SCHEMA} JSON")
    parser.add_argument("--output", type=Path, help="atomic JSON output path")
    parser.add_argument(
        "--require-match",
        action="store_true",
        help="return exit status 2 when the valid diagnosis is blocked",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = build_from_paths(args.focus, args.context)
        rendered = json.dumps(result, indent=2, sort_keys=True) + "\n"
        if args.output is not None:
            _atomic_write(args.output, rendered)
        else:
            sys.stdout.write(rendered)
    except TemporaryBirthInputError as exc:
        print(f"frontend temporary-birth input rejected: {exc}", file=sys.stderr)
        return 2
    if args.require_match and result["status"] != "matched":
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
