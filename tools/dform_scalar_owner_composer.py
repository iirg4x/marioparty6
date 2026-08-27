#!/usr/bin/env python3
"""Compose a D-form aggregate copy with scalar-owner and typed-pool evidence.

This diagnostic is intentionally fail-closed.  It consumes four immutable
``focus_symbol_report/v1`` artifacts representing a structural precursor, a
D-form-copy precursor, an owner/pool precursor, and an exact result.  It also
requires the independent physical-relocation summary and the completed crack
report.  When every boundary is sealed, it emits the bounded two-cell plan
which would have avoided the partial probes.  It never emits source or grants
retention/promotion authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence


CONTEXT_SCHEMA = "dform_scalar_owner_composer_context/v1"
OUTPUT_SCHEMA = "dform_scalar_owner_composer/v1"
FOCUS_SCHEMA = "focus_symbol_report/v1"
RULE_ID = "dform_copy_plus_scalar_owner_composer"
STAGE_ROLES = ("structural", "dform", "owner_pool", "exact")

_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@-]{1,192}")


class DformScalarOwnerInputError(ValueError):
    """The supplied evidence cannot safely support the composition."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value,
        sort_keys=True,
        separators=(",", ":"),
        ensure_ascii=True,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise DformScalarOwnerInputError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise DformScalarOwnerInputError(f"{label} must be an object")
    return value


def _array(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise DformScalarOwnerInputError(f"{label} must be an array")
    return value


def _keys(
    value: Mapping[str, Any],
    label: str,
    required: Sequence[str],
    *,
    optional: Sequence[str] = (),
) -> None:
    expected = set(required) | set(optional)
    missing = sorted(set(required) - set(value))
    extra = sorted(set(value) - expected)
    if missing:
        raise DformScalarOwnerInputError(f"{label} missing fields: {', '.join(missing)}")
    if extra:
        raise DformScalarOwnerInputError(f"{label} has unsupported fields: {', '.join(extra)}")


def _text(value: Any, label: str, *, limit: int = 1024) -> str:
    if not isinstance(value, str) or not value or len(value) > limit:
        raise DformScalarOwnerInputError(f"{label} must be nonempty text <= {limit} bytes")
    return value


def _identifier(value: Any, label: str) -> str:
    text = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(text) is None:
        raise DformScalarOwnerInputError(f"{label} must be a C identifier")
    return text


def _owner(value: Any, label: str) -> str:
    text = _text(value, label, limit=192)
    if _OWNER_RE.fullmatch(text) is None:
        raise DformScalarOwnerInputError(f"{label} is not a bounded owner identifier")
    return text


def _sha256(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise DformScalarOwnerInputError(f"{label} must be lowercase SHA-256")
    return value


def _uint(value: Any, label: str, *, maximum: int = 1 << 24) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= maximum:
        raise DformScalarOwnerInputError(f"{label} must be an unsigned integer")
    return value


def _boolean(value: Any, label: str, *, expected: bool | None = None) -> bool:
    if not isinstance(value, bool):
        raise DformScalarOwnerInputError(f"{label} must be boolean")
    if expected is not None and value is not expected:
        raise DformScalarOwnerInputError(f"{label} must be {str(expected).lower()}")
    return value


def _string_array(value: Any, label: str, *, identifiers: bool = False) -> list[str]:
    rows = _array(value, label)
    if not rows:
        raise DformScalarOwnerInputError(f"{label} must not be empty")
    result: list[str] = []
    for index, row in enumerate(rows):
        result.append(
            _identifier(row, f"{label}[{index}]")
            if identifiers
            else _text(row, f"{label}[{index}]", limit=256)
        )
    if len(set(result)) != len(result):
        raise DformScalarOwnerInputError(f"{label} contains duplicates")
    return result


def _load_json(path: Path, expected_sha256: str, label: str) -> Mapping[str, Any]:
    actual = file_sha256(path)
    if actual != expected_sha256:
        raise DformScalarOwnerInputError(
            f"{label} SHA-256 mismatch: expected {expected_sha256}, got {actual}"
        )
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise DformScalarOwnerInputError(f"cannot parse {label} {path}: {exc}") from exc
    return _mapping(value, label)


def _parse_stage(value: Any, role: str) -> dict[str, Any]:
    stage = _mapping(value, f"context.stages.{role}")
    _keys(
        stage,
        f"context.stages.{role}",
        (
            "file_sha256",
            "artifact_sha256",
            "source_sha256",
            "object_sha256",
            "expected",
        ),
    )
    expected = _mapping(stage["expected"], f"context.stages.{role}.expected")
    _keys(
        expected,
        f"context.stages.{role}.expected",
        (
            "target_size",
            "candidate_size",
            "strict_diff_rows",
            "data_diff_rows",
            "strict_exact",
            "data_exact",
        ),
    )
    return {
        "file_sha256": _sha256(stage["file_sha256"], f"context.stages.{role}.file_sha256"),
        "artifact_sha256": _sha256(
            stage["artifact_sha256"], f"context.stages.{role}.artifact_sha256"
        ),
        "source_sha256": _sha256(
            stage["source_sha256"], f"context.stages.{role}.source_sha256"
        ),
        "object_sha256": _sha256(
            stage["object_sha256"], f"context.stages.{role}.object_sha256"
        ),
        "expected": {
            "target_size": _uint(
                expected["target_size"], f"context.stages.{role}.expected.target_size"
            ),
            "candidate_size": _uint(
                expected["candidate_size"],
                f"context.stages.{role}.expected.candidate_size",
            ),
            "strict_diff_rows": _uint(
                expected["strict_diff_rows"],
                f"context.stages.{role}.expected.strict_diff_rows",
            ),
            "data_diff_rows": _uint(
                expected["data_diff_rows"],
                f"context.stages.{role}.expected.data_diff_rows",
            ),
            "strict_exact": _boolean(
                expected["strict_exact"], f"context.stages.{role}.expected.strict_exact"
            ),
            "data_exact": _boolean(
                expected["data_exact"], f"context.stages.{role}.expected.data_exact"
            ),
        },
    }


def _parse_axis(value: Any, label: str, fields: Sequence[str]) -> dict[str, Any]:
    axis = _mapping(value, label)
    _keys(axis, label, (*fields, "evidence_sha256"))
    result = {field: axis[field] for field in fields}
    result["evidence_sha256"] = _sha256(axis["evidence_sha256"], f"{label}.evidence_sha256")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    _keys(
        value,
        "context",
        (
            "schema",
            "owner",
            "function",
            "report",
            "stages",
            "physical_receipt",
            "semantic_axes",
            "invariants",
        ),
    )
    if value["schema"] != CONTEXT_SCHEMA:
        raise DformScalarOwnerInputError(f"context schema must be {CONTEXT_SCHEMA}")

    report = _mapping(value["report"], "context.report")
    _keys(report, "context.report", ("sha256",))

    stages_raw = _mapping(value["stages"], "context.stages")
    _keys(stages_raw, "context.stages", STAGE_ROLES)
    stages = {role: _parse_stage(stages_raw[role], role) for role in STAGE_ROLES}

    physical = _mapping(value["physical_receipt"], "context.physical_receipt")
    _keys(physical, "context.physical_receipt", ("sha256", "expected_count"))

    axes = _mapping(value["semantic_axes"], "context.semantic_axes")
    _keys(
        axes,
        "context.semantic_axes",
        (
            "stdlib_abs_threshold",
            "dform_copy",
            "scalar_owners",
            "typed_pool",
            "final_operand_order",
        ),
    )

    abs_axis = _parse_axis(
        axes["stdlib_abs_threshold"],
        "context.semantic_axes.stdlib_abs_threshold",
        ("header", "callee", "source_expression"),
    )
    abs_axis["header"] = _text(abs_axis["header"], "stdlib_abs_threshold.header", limit=128)
    abs_axis["callee"] = _identifier(abs_axis["callee"], "stdlib_abs_threshold.callee")
    abs_axis["source_expression"] = _text(
        abs_axis["source_expression"], "stdlib_abs_threshold.source_expression"
    )

    dform_axis = _parse_axis(
        axes["dform_copy"],
        "context.semantic_axes.dform_copy",
        (
            "aggregate_type",
            "helper",
            "target_row_start",
            "opcodes",
            "mwcc_guard",
            "portable_fallback",
        ),
    )
    dform_axis["aggregate_type"] = _identifier(
        dform_axis["aggregate_type"], "dform_copy.aggregate_type"
    )
    dform_axis["helper"] = _identifier(dform_axis["helper"], "dform_copy.helper")
    dform_axis["target_row_start"] = _uint(
        dform_axis["target_row_start"], "dform_copy.target_row_start"
    )
    dform_axis["opcodes"] = _string_array(
        dform_axis["opcodes"], "dform_copy.opcodes", identifiers=True
    )
    if dform_axis["opcodes"] != ["psq_l", "lfs", "psq_st", "stfs"]:
        raise DformScalarOwnerInputError(
            "dform_copy.opcodes must be the sealed psq_l/lfs/psq_st/stfs sequence"
        )
    dform_axis["mwcc_guard"] = _text(dform_axis["mwcc_guard"], "dform_copy.mwcc_guard")
    dform_axis["portable_fallback"] = _boolean(
        dform_axis["portable_fallback"], "dform_copy.portable_fallback", expected=True
    )

    scalar_axis = _parse_axis(
        axes["scalar_owners"],
        "context.semantic_axes.scalar_owners",
        ("reused_parameter", "live_owner", "scaled_role"),
    )
    for key in ("reused_parameter", "live_owner", "scaled_role"):
        scalar_axis[key] = _identifier(scalar_axis[key], f"scalar_owners.{key}")

    pool_axis = _parse_axis(
        axes["typed_pool"],
        "context.semantic_axes.typed_pool",
        ("required_owner", "decoder", "all_nonfinal_rows_closed"),
    )
    pool_axis["required_owner"] = _identifier(
        pool_axis["required_owner"], "typed_pool.required_owner"
    )
    pool_axis["decoder"] = _identifier(pool_axis["decoder"], "typed_pool.decoder")
    pool_axis["all_nonfinal_rows_closed"] = _boolean(
        pool_axis["all_nonfinal_rows_closed"],
        "typed_pool.all_nonfinal_rows_closed",
        expected=True,
    )

    final_axis = _parse_axis(
        axes["final_operand_order"],
        "context.semantic_axes.final_operand_order",
        (
            "target_expression",
            "control_expression",
            "row_indices",
            "target_opcodes",
            "candidate_opcodes",
            "required_opcodes",
        ),
    )
    final_axis["target_expression"] = _text(
        final_axis["target_expression"], "final_operand_order.target_expression"
    )
    final_axis["control_expression"] = _text(
        final_axis["control_expression"], "final_operand_order.control_expression"
    )
    row_indices = _array(final_axis["row_indices"], "final_operand_order.row_indices")
    final_axis["row_indices"] = [
        _uint(row, f"final_operand_order.row_indices[{index}]")
        for index, row in enumerate(row_indices)
    ]
    if len(set(final_axis["row_indices"])) != len(final_axis["row_indices"]):
        raise DformScalarOwnerInputError("final_operand_order.row_indices contains duplicates")
    for key in ("target_opcodes", "candidate_opcodes", "required_opcodes"):
        rows = _array(final_axis[key], f"final_operand_order.{key}")
        final_axis[key] = [
            _text(row, f"final_operand_order.{key}[{index}]", limit=32)
            for index, row in enumerate(rows)
        ]
    if len(final_axis["target_opcodes"]) != len(final_axis["row_indices"]):
        raise DformScalarOwnerInputError("target opcode count must equal final row count")
    if len(final_axis["candidate_opcodes"]) != len(final_axis["row_indices"]):
        raise DformScalarOwnerInputError("candidate opcode count must equal final row count")

    invariants = _mapping(value["invariants"], "context.invariants")
    invariant_names = (
        "cfg_exact_after_owner_pool",
        "calls_exact_after_owner_pool",
        "frame_exact_after_dform",
        "natural_c_only",
        "zero_protected_losses",
        "tracer_required",
        "source_patch_authorized",
        "retention_authorized",
        "promotion_authorized",
    )
    _keys(invariants, "context.invariants", invariant_names)
    parsed_invariants = {
        name: _boolean(
            invariants[name],
            f"context.invariants.{name}",
            expected=(False if name in {
                "tracer_required",
                "source_patch_authorized",
                "retention_authorized",
                "promotion_authorized",
            } else True),
        )
        for name in invariant_names
    }

    report_sha = _sha256(report["sha256"], "context.report.sha256")
    for axis_name, axis in (
        ("stdlib_abs_threshold", abs_axis),
        ("dform_copy", dform_axis),
        ("scalar_owners", scalar_axis),
        ("typed_pool", pool_axis),
        ("final_operand_order", final_axis),
    ):
        if axis["evidence_sha256"] != report_sha:
            raise DformScalarOwnerInputError(
                f"semantic axis {axis_name} is not bound to the report SHA-256"
            )

    return {
        "schema": CONTEXT_SCHEMA,
        "owner": _owner(value["owner"], "context.owner"),
        "function": _identifier(value["function"], "context.function"),
        "report": {"sha256": report_sha},
        "stages": stages,
        "physical_receipt": {
            "sha256": _sha256(physical["sha256"], "context.physical_receipt.sha256"),
            "expected_count": _uint(
                physical["expected_count"], "context.physical_receipt.expected_count"
            ),
        },
        "semantic_axes": {
            "stdlib_abs_threshold": abs_axis,
            "dform_copy": dform_axis,
            "scalar_owners": scalar_axis,
            "typed_pool": pool_axis,
            "final_operand_order": final_axis,
        },
        "invariants": parsed_invariants,
    }


def _verify_focus_artifact(
    value: Mapping[str, Any], role: str, binding: Mapping[str, Any], function: str
) -> None:
    if value.get("schema") != FOCUS_SCHEMA:
        raise DformScalarOwnerInputError(f"{role} artifact schema is not {FOCUS_SCHEMA}")
    expected_internal = _sha256(
        value.get("artifact_sha256"), f"{role}.artifact_sha256"
    )
    unsigned = dict(value)
    unsigned.pop("artifact_sha256", None)
    if canonical_sha256(unsigned) != expected_internal:
        raise DformScalarOwnerInputError(f"{role} artifact internal SHA-256 mismatch")
    if expected_internal != binding["artifact_sha256"]:
        raise DformScalarOwnerInputError(f"{role} artifact identity drifted")
    if value.get("function") != function:
        raise DformScalarOwnerInputError(f"{role} artifact function drifted")
    for field in (
        "authority_advanced",
        "promotion_authorized",
        "retention_authorized",
        "source_patch_emitted",
    ):
        if value.get(field) is not False:
            raise DformScalarOwnerInputError(f"{role} artifact unexpectedly set {field}")


def _channel(value: Mapping[str, Any], role: str, name: str) -> Mapping[str, Any]:
    channels = _mapping(value.get("channels"), f"{role}.channels")
    return _mapping(channels.get(name), f"{role}.channels.{name}")


def _metric(channel: Mapping[str, Any], role: str, name: str) -> Mapping[str, Any]:
    return _mapping(channel.get("metric"), f"{role}.channels.{name}.metric")


def _rows(channel: Mapping[str, Any], role: str, side: str) -> list[Mapping[str, Any]]:
    side_value = _mapping(channel.get(side), f"{role}.{side}")
    rows = _array(side_value.get("rows"), f"{role}.{side}.rows")
    return [_mapping(row, f"{role}.{side}.rows[{index}]") for index, row in enumerate(rows)]


def _formatted(row: Mapping[str, Any]) -> str | None:
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        return None
    value = instruction.get("formatted")
    return value if isinstance(value, str) else None


def _opcode(row: Mapping[str, Any]) -> str:
    formatted = _formatted(row)
    return "<gap>" if not formatted else formatted.split(None, 1)[0]


def _instruction_stream(channel: Mapping[str, Any], role: str, side: str) -> list[str]:
    return [formatted for row in _rows(channel, role, side) if (formatted := _formatted(row))]


def _diff_rows(channel: Mapping[str, Any], role: str, side: str) -> list[Mapping[str, Any]]:
    return [row for row in _rows(channel, role, side) if isinstance(row.get("diff_kind"), str)]


def _row_by_index(
    channel: Mapping[str, Any], role: str, side: str
) -> dict[int, Mapping[str, Any]]:
    result: dict[int, Mapping[str, Any]] = {}
    for row in _rows(channel, role, side):
        index = _uint(row.get("index"), f"{role}.{side}.row.index")
        if index in result:
            raise DformScalarOwnerInputError(f"{role}.{side} has duplicate row index {index}")
        result[index] = row
    return result


def _validate_metrics(
    artifacts: Mapping[str, Mapping[str, Any]], context: Mapping[str, Any]
) -> list[dict[str, Any]]:
    summaries: list[dict[str, Any]] = []
    strict_counts: list[int] = []
    data_counts: list[int] = []
    target_sizes: set[int] = set()
    for role in STAGE_ROLES:
        expected = context["stages"][role]["expected"]
        artifact = artifacts[role]
        strict_metric = _metric(_channel(artifact, role, "strict"), role, "strict")
        data_metric = _metric(_channel(artifact, role, "data"), role, "data")
        observed = {
            "target_size": strict_metric.get("target_size"),
            "candidate_size": strict_metric.get("candidate_size"),
            "strict_diff_rows": strict_metric.get("diff_rows"),
            "data_diff_rows": data_metric.get("diff_rows"),
            "strict_exact": strict_metric.get("exact"),
            "data_exact": data_metric.get("exact"),
        }
        if observed != expected:
            raise DformScalarOwnerInputError(
                f"{role} metric drifted: expected {expected}, got {observed}"
            )
        if data_metric.get("target_size") != expected["target_size"]:
            raise DformScalarOwnerInputError(f"{role} data target size drifted")
        if data_metric.get("candidate_size") != expected["candidate_size"]:
            raise DformScalarOwnerInputError(f"{role} data candidate size drifted")
        for channel_name, channel, expected_rows in (
            ("strict", _channel(artifact, role, "strict"), expected["strict_diff_rows"]),
            ("data", _channel(artifact, role, "data"), expected["data_diff_rows"]),
        ):
            for side in ("target", "candidate"):
                side_value = _mapping(
                    channel.get(side), f"{role}.channels.{channel_name}.{side}"
                )
                observed_rows = _uint(
                    side_value.get("diff_row_count"),
                    f"{role}.channels.{channel_name}.{side}.diff_row_count",
                )
                if observed_rows != expected_rows:
                    raise DformScalarOwnerInputError(
                        f"{role} {channel_name} {side} diff-row count drifted"
                    )
        target_sizes.add(expected["target_size"])
        strict_counts.append(expected["strict_diff_rows"])
        data_counts.append(expected["data_diff_rows"])
        summaries.append({"role": role, **observed})
    if len(target_sizes) != 1:
        raise DformScalarOwnerInputError("target size changed across the stage sequence")
    if not all(left > right for left, right in zip(strict_counts, strict_counts[1:])):
        raise DformScalarOwnerInputError("strict residual did not decrease at every stage")
    if not all(left > right for left, right in zip(data_counts, data_counts[1:])):
        raise DformScalarOwnerInputError("data residual did not decrease at every stage")
    target_size = next(iter(target_sizes))
    if context["stages"]["structural"]["expected"]["candidate_size"] >= target_size:
        raise DformScalarOwnerInputError("structural stage is not the expected size-deficit precursor")
    for role in ("dform", "owner_pool", "exact"):
        if context["stages"][role]["expected"]["candidate_size"] != target_size:
            raise DformScalarOwnerInputError(f"{role} stage is not exact-size")
    if not context["stages"]["exact"]["expected"]["strict_exact"]:
        raise DformScalarOwnerInputError("exact stage is not strict exact")
    if not context["stages"]["exact"]["expected"]["data_exact"]:
        raise DformScalarOwnerInputError("exact stage is not data exact")
    return summaries


def _validate_target_and_siblings(artifacts: Mapping[str, Mapping[str, Any]]) -> dict[str, Any]:
    target_stream_hash: str | None = None
    exact_identity_hash: str | None = None
    exact_sibling_count: int | None = None
    for role in STAGE_ROLES:
        strict = _channel(artifacts[role], role, "strict")
        data = _channel(artifacts[role], role, "data")
        stream_hash = canonical_sha256(_instruction_stream(strict, role, "target"))
        if target_stream_hash is None:
            target_stream_hash = stream_hash
        elif stream_hash != target_stream_hash:
            raise DformScalarOwnerInputError("target instruction stream drifted across stages")
        strict_protected = _mapping(strict.get("protected_siblings"), f"{role}.strict.protected")
        data_protected = _mapping(data.get("protected_siblings"), f"{role}.data.protected")
        current_hash = _sha256(
            strict_protected.get("exact_identity_sha256"),
            f"{role}.strict.protected.exact_identity_sha256",
        )
        if data_protected.get("exact_identity_sha256") != current_hash:
            raise DformScalarOwnerInputError(f"{role} strict/data protected siblings drifted")
        current_count = _uint(
            strict_protected.get("exact_sibling_count"),
            f"{role}.strict.protected.exact_sibling_count",
        )
        if exact_identity_hash is None:
            exact_identity_hash = current_hash
            exact_sibling_count = current_count
        elif current_hash != exact_identity_hash or current_count != exact_sibling_count:
            raise DformScalarOwnerInputError("protected exact sibling identities changed")
    exact_strict = _channel(artifacts["exact"], "exact", "strict")
    target_opcodes = [
        _opcode(row)
        for row in _rows(exact_strict, "exact", "target")
        if _formatted(row)
    ]
    candidate_opcodes = [
        _opcode(row)
        for row in _rows(exact_strict, "exact", "candidate")
        if _formatted(row)
    ]
    if candidate_opcodes != target_opcodes:
        raise DformScalarOwnerInputError("exact candidate opcode stream differs from target")
    return {
        "target_instruction_stream_sha256": target_stream_hash,
        "protected_exact_sibling_count": exact_sibling_count,
        "protected_exact_identity_sha256": exact_identity_hash,
    }


def _validate_dform(
    artifacts: Mapping[str, Mapping[str, Any]], context: Mapping[str, Any]
) -> dict[str, Any]:
    axis = context["semantic_axes"]["dform_copy"]
    indices = [axis["target_row_start"] + offset for offset in range(len(axis["opcodes"]))]
    sealed_rows: list[dict[str, Any]] = []
    for role in STAGE_ROLES:
        strict = _channel(artifacts[role], role, "strict")
        target_rows = _row_by_index(strict, role, "target")
        candidate_rows = _row_by_index(strict, role, "candidate")
        observed_target = [_opcode(target_rows.get(index, {})) for index in indices]
        if observed_target != axis["opcodes"]:
            raise DformScalarOwnerInputError(
                f"{role} target D-form sequence drifted: {observed_target}"
            )
        exact_match = all(
            index in candidate_rows
            and _formatted(candidate_rows[index]) == _formatted(target_rows[index])
            and not candidate_rows[index].get("diff_kind")
            and not target_rows[index].get("diff_kind")
            for index in indices
        )
        if role == "structural" and exact_match:
            raise DformScalarOwnerInputError("structural stage already contains the sealed D-form copy")
        if role in {"dform", "owner_pool", "exact"} and not exact_match:
            raise DformScalarOwnerInputError(f"{role} stage did not preserve the sealed D-form copy")
        if role == "exact":
            sealed_rows = [
                {
                    "row_index": index,
                    "formatted": _formatted(target_rows[index]),
                }
                for index in indices
            ]
    return {
        "aggregate_type": axis["aggregate_type"],
        "helper": axis["helper"],
        "opcodes": axis["opcodes"],
        "sealed_rows": sealed_rows,
        "portable_fallback": axis["portable_fallback"],
    }


def _validate_final_residual(
    artifacts: Mapping[str, Mapping[str, Any]], context: Mapping[str, Any]
) -> dict[str, Any]:
    axis = context["semantic_axes"]["final_operand_order"]
    owner_strict = _channel(artifacts["owner_pool"], "owner_pool", "strict")
    owner_data = _channel(artifacts["owner_pool"], "owner_pool", "data")
    target_rows = _diff_rows(owner_strict, "owner_pool", "target")
    candidate_rows = _diff_rows(owner_strict, "owner_pool", "candidate")
    target_indices = [row["index"] for row in target_rows]
    candidate_indices = [row["index"] for row in candidate_rows]
    if target_indices != axis["row_indices"] or candidate_indices != axis["row_indices"]:
        raise DformScalarOwnerInputError(
            "owner/pool residual row indices do not match the sealed operand-order seam"
        )
    if [_opcode(row) for row in target_rows] != axis["target_opcodes"]:
        raise DformScalarOwnerInputError("target operand-order opcode seam drifted")
    if [_opcode(row) for row in candidate_rows] != axis["candidate_opcodes"]:
        raise DformScalarOwnerInputError("candidate operand-order opcode seam drifted")
    observed_opcodes = {_opcode(row) for row in (*target_rows, *candidate_rows)}
    if not set(axis["required_opcodes"]).issubset(observed_opcodes):
        raise DformScalarOwnerInputError("operand-order seam lacks required opcodes")
    required_owner = context["semantic_axes"]["typed_pool"]["required_owner"]
    if not any(required_owner in (_formatted(row) or "") for row in (*target_rows, *candidate_rows)):
        raise DformScalarOwnerInputError("required typed-pool owner is absent from the final seam")
    if _metric(owner_data, "owner_pool", "data").get("diff_rows") != len(axis["row_indices"]):
        raise DformScalarOwnerInputError("data residual is not the same sealed final seam")

    annotations = _mapping(owner_strict.get("relocation_annotations"), "owner_pool.relocations")
    relocation_rows: set[int] = set()
    for side in ("target", "candidate"):
        side_value = _mapping(annotations.get(side), f"owner_pool.relocations.{side}")
        for entry in _array(side_value.get("entries"), f"owner_pool.relocations.{side}.entries"):
            row = _mapping(entry, f"owner_pool.relocations.{side}.entry")
            if row.get("diff_kind"):
                relocation_rows.add(_uint(row.get("row_index"), "relocation row index"))
    if not relocation_rows or not relocation_rows.issubset(set(axis["row_indices"])):
        raise DformScalarOwnerInputError(
            "nonfinal relocation annotations remain after the owner/pool stage"
        )

    exact_strict = _channel(artifacts["exact"], "exact", "strict")
    exact_data = _channel(artifacts["exact"], "exact", "data")
    for channel_name, channel in (("strict", exact_strict), ("data", exact_data)):
        if _diff_rows(channel, "exact", "target") or _diff_rows(channel, "exact", "candidate"):
            raise DformScalarOwnerInputError(f"exact {channel_name} artifact still has diff rows")

    return {
        "row_indices": axis["row_indices"],
        "target_opcodes": axis["target_opcodes"],
        "candidate_opcodes": axis["candidate_opcodes"],
        "relocation_rows": sorted(relocation_rows),
        "required_pool_owner": required_owner,
    }


def _relocation_count(artifact: Mapping[str, Any], role: str) -> int:
    strict = _channel(artifact, role, "strict")
    annotations = _mapping(strict.get("relocation_annotations"), f"{role}.relocations")
    target = _mapping(annotations.get("target"), f"{role}.relocations.target")
    candidate = _mapping(annotations.get("candidate"), f"{role}.relocations.candidate")
    target_count = _uint(target.get("count"), f"{role}.relocations.target.count")
    candidate_count = _uint(candidate.get("count"), f"{role}.relocations.candidate.count")
    if target_count != candidate_count:
        raise DformScalarOwnerInputError(f"{role} relocation annotation count drifted")
    return target_count


def _validate_physical(
    receipt: Mapping[str, Any], context: Mapping[str, Any], artifacts: Mapping[str, Mapping[str, Any]]
) -> dict[str, Any]:
    _keys(
        receipt,
        "physical receipt",
        ("candidate_count", "difference_count", "differences", "focus", "target_count"),
    )
    if receipt["focus"] != context["function"]:
        raise DformScalarOwnerInputError("physical receipt focus drifted")
    expected = context["physical_receipt"]["expected_count"]
    target_count = _uint(receipt["target_count"], "physical receipt target_count")
    candidate_count = _uint(receipt["candidate_count"], "physical receipt candidate_count")
    difference_count = _uint(receipt["difference_count"], "physical receipt difference_count")
    differences = _array(receipt["differences"], "physical receipt differences")
    if target_count != expected or candidate_count != expected:
        raise DformScalarOwnerInputError("physical relocation count differs from context")
    if difference_count != 0 or differences:
        raise DformScalarOwnerInputError("physical relocation receipt is not exact")
    stage_counts = {role: _relocation_count(artifacts[role], role) for role in STAGE_ROLES}
    if set(stage_counts.values()) != {expected}:
        raise DformScalarOwnerInputError("stage relocation annotation counts drifted")
    return {
        "status": "exact",
        "target_count": target_count,
        "candidate_count": candidate_count,
        "difference_count": difference_count,
        "stage_annotation_counts": stage_counts,
    }


def evaluate(
    context: Mapping[str, Any],
    artifacts: Mapping[str, Mapping[str, Any]],
    physical_receipt: Mapping[str, Any],
    *,
    bindings: Mapping[str, Any],
) -> dict[str, Any]:
    parsed = parse_context(context)
    for role in STAGE_ROLES:
        _verify_focus_artifact(artifacts[role], role, parsed["stages"][role], parsed["function"])

    stage_summary = _validate_metrics(artifacts, parsed)
    stable = _validate_target_and_siblings(artifacts)
    dform = _validate_dform(artifacts, parsed)
    final = _validate_final_residual(artifacts, parsed)
    physical = _validate_physical(physical_receipt, parsed, artifacts)

    axes = parsed["semantic_axes"]
    result: dict[str, Any] = {
        "schema": OUTPUT_SCHEMA,
        "schema_version": 1,
        "rule_id": RULE_ID,
        "owner": parsed["owner"],
        "function": parsed["function"],
        "status": "READY",
        "input_binding": dict(bindings),
        "stage_summary": stage_summary,
        "evidence": {
            "stable_target_and_siblings": stable,
            "dform_copy": dform,
            "final_operand_order": final,
            "physical_relocations": physical,
            "report_bound_invariants": parsed["invariants"],
        },
        "ordered_cells": [
            {
                "rank": 1,
                "id": "compose_abs_dform_scalar_pool",
                "compile_budget": 1,
                "axes": [
                    "stdlib_abs_threshold",
                    "dform_copy",
                    "scalar_owners",
                    "typed_pool",
                ],
                "source_shapes": {
                    "stdlib_abs_threshold": axes["stdlib_abs_threshold"]["source_expression"],
                    "dform_helper": axes["dform_copy"]["helper"],
                    "reused_parameter": axes["scalar_owners"]["reused_parameter"],
                    "live_owner": axes["scalar_owners"]["live_owner"],
                    "typed_pool_owner": axes["typed_pool"]["required_owner"],
                },
                "expected_checkpoint": {
                    "candidate_size": parsed["stages"]["owner_pool"]["expected"]["candidate_size"],
                    "strict_diff_rows": len(final["row_indices"]),
                    "data_diff_rows": len(final["row_indices"]),
                    "remaining_cause": "one sealed operand-order seam",
                },
            },
            {
                "rank": 2,
                "id": "commute_final_multiply_operands",
                "compile_budget": 1,
                "precondition": {
                    "row_indices": final["row_indices"],
                    "required_pool_owner": final["required_pool_owner"],
                    "strict_and_data_rows": len(final["row_indices"]),
                    "physical_relocations": "unchanged",
                },
                "control_expression": axes["final_operand_order"]["control_expression"],
                "target_expression": axes["final_operand_order"]["target_expression"],
                "expected_checkpoint": {
                    "strict_percent": 100.0,
                    "data_percent": 100.0,
                    "physical_relocations": f"{physical['target_count']}/{physical['candidate_count']}",
                    "protected_sibling_losses": 0,
                },
            },
        ],
        "suppressed_axes": [
            "partial_structure_only",
            "dform_copy_only",
            "scalar_owner_only",
            "typed_pool_only",
            "declaration_permutations",
            "tracer_capture",
            "unsealed_operand_permutations",
        ],
        "compile_budget": 2,
        "tracer_required": False,
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["diagnosis_sha256"] = canonical_sha256(result)
    return result


def analyze_paths(
    *,
    context_path: Path,
    expected_context_sha256: str,
    stage_paths: Mapping[str, Path],
    physical_receipt_path: Path,
    report_path: Path,
) -> dict[str, Any]:
    expected_context_sha256 = _sha256(expected_context_sha256, "expected context SHA-256")
    context = _load_json(context_path, expected_context_sha256, "context")
    parsed = parse_context(context)

    artifacts: dict[str, Mapping[str, Any]] = {}
    stage_bindings: dict[str, Any] = {}
    for role in STAGE_ROLES:
        if role not in stage_paths:
            raise DformScalarOwnerInputError(f"missing {role} artifact path")
        expected = parsed["stages"][role]["file_sha256"]
        artifacts[role] = _load_json(stage_paths[role], expected, f"{role} artifact")
        stage_bindings[role] = {
            "path": str(stage_paths[role]),
            "sha256": expected,
            "artifact_sha256": parsed["stages"][role]["artifact_sha256"],
            "source_sha256": parsed["stages"][role]["source_sha256"],
            "object_sha256": parsed["stages"][role]["object_sha256"],
        }

    physical = _load_json(
        physical_receipt_path,
        parsed["physical_receipt"]["sha256"],
        "physical receipt",
    )
    actual_report_sha = file_sha256(report_path)
    if actual_report_sha != parsed["report"]["sha256"]:
        raise DformScalarOwnerInputError(
            f"report SHA-256 mismatch: expected {parsed['report']['sha256']}, got {actual_report_sha}"
        )

    bindings = {
        "context": {"path": str(context_path), "sha256": expected_context_sha256},
        "report": {"path": str(report_path), "sha256": actual_report_sha},
        "stages": stage_bindings,
        "physical_receipt": {
            "path": str(physical_receipt_path),
            "sha256": parsed["physical_receipt"]["sha256"],
        },
    }
    return evaluate(context, artifacts, physical, bindings=bindings)


def _write_result(result: Mapping[str, Any], output: Path | None, *, pretty: bool) -> None:
    text = json.dumps(
        result,
        indent=2 if pretty else None,
        sort_keys=True,
        separators=None if pretty else (",", ":"),
    ) + "\n"
    if output is None:
        sys.stdout.write(text)
        return
    try:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text, encoding="utf-8")
    except OSError as exc:
        raise DformScalarOwnerInputError(f"cannot write {output}: {exc}") from exc


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("context", type=Path)
    parser.add_argument("structural_artifact", type=Path)
    parser.add_argument("dform_artifact", type=Path)
    parser.add_argument("owner_pool_artifact", type=Path)
    parser.add_argument("exact_artifact", type=Path)
    parser.add_argument("physical_receipt", type=Path)
    parser.add_argument("report", type=Path)
    parser.add_argument("--expect-context-sha256", required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--pretty", action="store_true")
    parser.add_argument("--require-ready", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = analyze_paths(
            context_path=args.context,
            expected_context_sha256=args.expect_context_sha256,
            stage_paths={
                "structural": args.structural_artifact,
                "dform": args.dform_artifact,
                "owner_pool": args.owner_pool_artifact,
                "exact": args.exact_artifact,
            },
            physical_receipt_path=args.physical_receipt,
            report_path=args.report,
        )
        _write_result(result, args.output, pretty=args.pretty)
        if args.require_ready and result.get("status") != "READY":
            return 2
        return 0
    except DformScalarOwnerInputError as exc:
        print(f"dform_scalar_owner_composer: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
