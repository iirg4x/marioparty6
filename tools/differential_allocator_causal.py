#!/usr/bin/env python3
"""Fail-closed differential allocator causal solver.

The solver consumes hash-bound focus, physical-stream, relocation, source-span,
and same-session ownership evidence.  It ranks at most one caller-declared
natural source interaction when every residual row belongs to one maximal
closed register permutation.  It never emits source or authorizes retention.
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


SCHEMA = "differential_allocator_causal/v1"
CONTEXT_SCHEMA = "differential_allocator_causal_context/v1"
STREAM_SCHEMA = "differential_allocator_physical_streams/v1"
TRACE_SCHEMA = "differential_allocator_same_session_trace/v1"
SPAN_SCHEMA = "mwcc_source_span_bindings/v2"
FOCUS_SCHEMA = "focus_symbol_report/v1"
REQUEST_SCHEMA = "differential_allocator_one_cell_request/v1"

_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_ID_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_.:+@/-]{0,191}$")
_REGISTER_RE = re.compile(r"\b([rf](?:[0-9]|[12][0-9]|3[01]))\b", re.IGNORECASE)
_VREG_RE = re.compile(r"^[rf][0-9]+$", re.IGNORECASE)
_FORBIDDEN_RE = re.compile(
    r"(?i)(?:(?:^|[^A-Za-z0-9])(?:matrix|dead|fake|padding)(?:$|[^A-Za-z0-9])|"
    r"register[-_ ]?shap(?:e|ing)|inline[-_ ]?asm)"
)


class DifferentialAllocatorInputError(ValueError):
    """Malformed, unauthenticated, or policy-incompatible evidence."""


def _canonical(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise DifferentialAllocatorInputError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise DifferentialAllocatorInputError(f"{label} must be an object")
    return value


def _sequence(value: Any, label: str) -> list[Any]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)):
        raise DifferentialAllocatorInputError(f"{label} must be an array")
    return list(value)


def _closed(value: Any, label: str, required: set[str], optional: set[str] | None = None) -> Mapping[str, Any]:
    row = _mapping(value, label)
    allowed = required | (optional or set())
    missing = sorted(required - set(row))
    extra = sorted(set(row) - allowed)
    if missing:
        raise DifferentialAllocatorInputError(f"{label} missing fields: {', '.join(missing)}")
    if extra:
        raise DifferentialAllocatorInputError(f"{label} has unsupported fields: {', '.join(extra)}")
    return row


def _text(value: Any, label: str, limit: int = 512) -> str:
    if not isinstance(value, str) or not value or len(value) > limit:
        raise DifferentialAllocatorInputError(f"{label} must be nonempty text <= {limit} characters")
    return value


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, 192)
    if _ID_RE.fullmatch(result) is None:
        raise DifferentialAllocatorInputError(f"{label} is not a canonical identifier")
    return result


def _optional_identifier(value: Any, label: str) -> str | None:
    return None if value is None else _identifier(value, label)


def _natural_text(value: Any, label: str) -> str:
    result = _text(value, label)
    if _FORBIDDEN_RE.search(result):
        raise DifferentialAllocatorInputError(f"{label} contains a prohibited source class")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, 64)
    if _SHA256_RE.fullmatch(result) is None:
        raise DifferentialAllocatorInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, maximum: int = 1 << 31) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= maximum:
        raise DifferentialAllocatorInputError(f"{label} must be an unsigned integer")
    return value


def _self_digest(value: Mapping[str, Any], field: str, label: str) -> None:
    expected = _sha256(value.get(field), f"{label}.{field}")
    unsigned = dict(value)
    unsigned.pop(field, None)
    if canonical_sha256(unsigned) != expected:
        raise DifferentialAllocatorInputError(f"{label} self-digest mismatch")


def _descriptor(value: Any, label: str) -> dict[str, str]:
    row = _closed(value, label, {"path", "file_sha256", "payload_sha256"})
    return {
        "path": _text(row["path"], f"{label}.path", 4096),
        "file_sha256": _sha256(row["file_sha256"], f"{label}.file_sha256"),
        "payload_sha256": _sha256(row["payload_sha256"], f"{label}.payload_sha256"),
    }


def _binary_descriptor(value: Any, label: str) -> dict[str, Any]:
    row = _closed(value, label, {"path", "size", "sha256"})
    return {
        "path": _text(row["path"], f"{label}.path", 4096),
        "size": _uint(row["size"], f"{label}.size"),
        "sha256": _sha256(row["sha256"], f"{label}.sha256"),
    }


def _read_bound(descriptor: Mapping[str, str], label: str) -> Mapping[str, Any]:
    path = Path(descriptor["path"])
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        raise DifferentialAllocatorInputError(f"{label} is not an absolute regular file")
    if file_sha256(path) != descriptor["file_sha256"]:
        raise DifferentialAllocatorInputError(f"{label} file SHA-256 mismatch")
    try:
        return _mapping(json.loads(path.read_text(encoding="utf-8")), label)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise DifferentialAllocatorInputError(f"cannot parse {label}: {exc}") from exc


def _verify_binary(descriptor: Mapping[str, Any], label: str) -> None:
    path = Path(str(descriptor["path"]))
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        raise DifferentialAllocatorInputError(f"{label} is not an absolute regular file")
    if path.stat().st_size != descriptor["size"] or file_sha256(path) != descriptor["sha256"]:
        raise DifferentialAllocatorInputError(f"{label} file identity mismatch")


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    row = _closed(
        value,
        "context",
        {
            "schema", "function", "focus", "physical_streams", "physical_relocation_receipt",
            "source_spans", "trace", "compiler", "tool", "source_class_allowlist",
            "source_class_hypotheses", "rejected_controls", "session_id", "trust_anchor_sha256",
            "context_sha256",
        },
    )
    if row["schema"] != CONTEXT_SCHEMA:
        raise DifferentialAllocatorInputError(f"context.schema must be {CONTEXT_SCHEMA}")
    _self_digest(row, "context_sha256", "context")
    allowlist = [_identifier(item, f"context.source_class_allowlist[{index}]") for index, item in enumerate(_sequence(row["source_class_allowlist"], "context.source_class_allowlist"))]
    if not allowlist or len(allowlist) != len(set(allowlist)):
        raise DifferentialAllocatorInputError("source_class_allowlist must be nonempty and duplicate-free")
    controls: list[dict[str, Any]] = []
    control_ids: set[str] = set()
    for index, raw in enumerate(_sequence(row["rejected_controls"], "context.rejected_controls")):
        label = f"context.rejected_controls[{index}]"
        item = _closed(raw, label, {"control_id", "source_sha256", "object_sha256", "outcome"})
        control_id = _identifier(item["control_id"], f"{label}.control_id")
        if control_id in control_ids:
            raise DifferentialAllocatorInputError("rejected controls contain duplicate IDs")
        control_ids.add(control_id)
        controls.append({
            "control_id": control_id,
            "source_sha256": _sha256(item["source_sha256"], f"{label}.source_sha256"),
            "object_sha256": _sha256(item["object_sha256"], f"{label}.object_sha256"),
            "outcome": _text(item["outcome"], f"{label}.outcome"),
        })
    hypotheses: list[dict[str, Any]] = []
    seen_classes: set[str] = set()
    for index, raw in enumerate(_sequence(row["source_class_hypotheses"], "context.source_class_hypotheses")):
        label = f"context.source_class_hypotheses[{index}]"
        item = _closed(raw, label, {"source_class", "owner_ids", "row_indices", "axes", "suppresses_control_ids"})
        source_class = _identifier(item["source_class"], f"{label}.source_class")
        if _FORBIDDEN_RE.search(source_class):
            raise DifferentialAllocatorInputError(f"{label}.source_class contains a prohibited source class")
        if source_class not in allowlist or source_class in seen_classes:
            raise DifferentialAllocatorInputError(f"{label}.source_class is not uniquely allowlisted")
        seen_classes.add(source_class)
        owner_ids = [_identifier(value, f"{label}.owner_ids[{ordinal}]") for ordinal, value in enumerate(_sequence(item["owner_ids"], f"{label}.owner_ids"))]
        row_indices = [_uint(value, f"{label}.row_indices[{ordinal}]") for ordinal, value in enumerate(_sequence(item["row_indices"], f"{label}.row_indices"))]
        if not owner_ids or not row_indices or len(owner_ids) != len(set(owner_ids)) or len(row_indices) != len(set(row_indices)):
            raise DifferentialAllocatorInputError(f"{label} owner/row sets must be nonempty and duplicate-free")
        axes: list[dict[str, str]] = []
        for axis_index, raw_axis in enumerate(_sequence(item["axes"], f"{label}.axes")):
            axis_label = f"{label}.axes[{axis_index}]"
            axis = _closed(raw_axis, axis_label, {"id", "hypothesis", "source_action", "topology_token"})
            axis_id = _identifier(axis["id"], f"{axis_label}.id")
            topology_token = _identifier(axis["topology_token"], f"{axis_label}.topology_token")
            if _FORBIDDEN_RE.search(axis_id) or _FORBIDDEN_RE.search(topology_token):
                raise DifferentialAllocatorInputError(f"{axis_label} contains a prohibited source class")
            axes.append({
                "id": axis_id,
                "hypothesis": _natural_text(axis["hypothesis"], f"{axis_label}.hypothesis"),
                "source_action": _natural_text(axis["source_action"], f"{axis_label}.source_action"),
                "topology_token": topology_token,
            })
        if not 2 <= len(axes) <= 3 or len({axis["id"] for axis in axes}) != len(axes):
            raise DifferentialAllocatorInputError(f"{label}.axes must contain two or three unique natural axes")
        suppresses = [
            _identifier(value, f"{label}.suppresses_control_ids[{ordinal}]")
            for ordinal, value in enumerate(_sequence(item["suppresses_control_ids"], f"{label}.suppresses_control_ids"))
        ]
        if len(suppresses) != len(set(suppresses)) or not set(suppresses) <= control_ids:
            raise DifferentialAllocatorInputError(f"{label}.suppresses_control_ids must uniquely reference rejected controls")
        hypotheses.append({
            "source_class": source_class,
            "owner_ids": owner_ids,
            "row_indices": row_indices,
            "axes": axes,
            "suppresses_control_ids": suppresses,
        })
    return {
        "function": _identifier(row["function"], "context.function"),
        "session_id": _identifier(row["session_id"], "context.session_id"),
        "trust_anchor_sha256": _sha256(row["trust_anchor_sha256"], "context.trust_anchor_sha256"),
        "focus": _descriptor(row["focus"], "context.focus"),
        "physical_streams": _descriptor(row["physical_streams"], "context.physical_streams"),
        "physical_relocation_receipt": _descriptor(row["physical_relocation_receipt"], "context.physical_relocation_receipt"),
        "source_spans": _descriptor(row["source_spans"], "context.source_spans"),
        "trace": _descriptor(row["trace"], "context.trace"),
        "compiler": _binary_descriptor(row["compiler"], "context.compiler"),
        "tool": _binary_descriptor(row["tool"], "context.tool"),
        "allowlist": allowlist,
        "hypotheses": hypotheses,
        "rejected_controls": controls,
        "context_sha256": row["context_sha256"],
    }


def _rows(side: Any, label: str) -> dict[int, Mapping[str, Any]]:
    side_row = _mapping(side, label)
    raw_rows = _sequence(side_row.get("rows"), f"{label}.rows")
    result: dict[int, Mapping[str, Any]] = {}
    for ordinal, raw in enumerate(raw_rows):
        item = _mapping(raw, f"{label}.rows[{ordinal}]")
        index = _uint(item.get("index"), f"{label}.rows[{ordinal}].index")
        if index in result:
            raise DifferentialAllocatorInputError(f"{label} has duplicate row {index}")
        result[index] = item
    return result


def _formatted(row: Mapping[str, Any], label: str) -> str:
    instruction = _mapping(row.get("instruction"), f"{label}.instruction")
    return _text(instruction.get("formatted"), f"{label}.instruction.formatted")


def _registers(row: Mapping[str, Any], label: str) -> list[str]:
    return [match.group(1).lower() for match in _REGISTER_RE.finditer(_formatted(row, label))]


def _skeleton(row: Mapping[str, Any], label: str) -> str:
    return _REGISTER_RE.sub("<reg>", _formatted(row, label).strip().lower())


def _focus_evidence(value: Mapping[str, Any], function: str, blockers: list[str]) -> tuple[dict[int, Mapping[str, Any]], dict[int, Mapping[str, Any]]]:
    if value.get("schema") != FOCUS_SCHEMA or value.get("function") != function:
        raise DifferentialAllocatorInputError("focus schema/function mismatch")
    _self_digest(value, "artifact_sha256", "focus")
    if value.get("authority_advanced") is not False:
        blockers.append("focus_authority_not_false")
    channels = _mapping(value.get("channels"), "focus.channels")
    strict = _mapping(channels.get("strict"), "focus.channels.strict")
    data = _mapping(channels.get("data"), "focus.channels.data")
    strict_target = _rows(strict.get("target"), "focus.strict.target")
    strict_candidate = _rows(strict.get("candidate"), "focus.strict.candidate")
    data_target = _rows(data.get("target"), "focus.data.target")
    data_candidate = _rows(data.get("candidate"), "focus.data.candidate")
    strict_diff = {index for index, row in strict_target.items() if row.get("diff_kind")}
    candidate_diff = {index for index, row in strict_candidate.items() if row.get("diff_kind")}
    data_diff = {index for index, row in data_target.items() if row.get("diff_kind")}
    data_candidate_diff = {index for index, row in data_candidate.items() if row.get("diff_kind")}
    if strict_diff != candidate_diff or strict_diff != data_diff or strict_diff != data_candidate_diff:
        blockers.append("strict_data_residual_row_sets_differ")
    for index in sorted(strict_diff):
        if strict_target[index].get("diff_kind") != "DIFF_ARG_MISMATCH" or strict_candidate[index].get("diff_kind") != "DIFF_ARG_MISMATCH":
            blockers.append(f"row_{index}_not_argument_only")
    return ({index: strict_target[index] for index in strict_diff}, {index: strict_candidate[index] for index in strict_diff})


def _physical_rows(side: Any, label: str) -> dict[int, Mapping[str, Any]]:
    side_row = _closed(side, label, {"rows"})
    result: dict[int, Mapping[str, Any]] = {}
    for ordinal, raw in enumerate(_sequence(side_row["rows"], f"{label}.rows")):
        row_label = f"{label}.rows[{ordinal}]"
        item = _closed(raw, row_label, {"index", "diff_kind", "instruction"})
        index = _uint(item["index"], f"{row_label}.index")
        if index in result:
            raise DifferentialAllocatorInputError(f"{label} has duplicate row {index}")
        if item["diff_kind"] is not None and not isinstance(item["diff_kind"], str):
            raise DifferentialAllocatorInputError(f"{row_label}.diff_kind must be text or null")
        instruction = _closed(item["instruction"], f"{row_label}.instruction", {"formatted"})
        _text(instruction["formatted"], f"{row_label}.instruction.formatted")
        result[index] = item
    return result


def _stream_evidence(
    value: Mapping[str, Any],
    function: str,
    target_rows: Mapping[int, Mapping[str, Any]],
    candidate_rows: Mapping[int, Mapping[str, Any]],
    blockers: list[str],
) -> tuple[dict[int, Mapping[str, Any]], dict[int, Mapping[str, Any]]]:
    row = _closed(
        value,
        "physical streams",
        {
            "schema", "function", "authority_advanced", "target_cfg_sha256", "candidate_cfg_sha256",
            "target_relocation_sha256", "candidate_relocation_sha256", "target", "candidate", "stream_sha256",
        },
    )
    if row["schema"] != STREAM_SCHEMA or row["function"] != function:
        raise DifferentialAllocatorInputError("physical stream schema/function mismatch")
    _self_digest(row, "stream_sha256", "physical streams")
    target_cfg = _sha256(row["target_cfg_sha256"], "physical_streams.target_cfg_sha256")
    candidate_cfg = _sha256(row["candidate_cfg_sha256"], "physical_streams.candidate_cfg_sha256")
    target_relocation = _sha256(row["target_relocation_sha256"], "physical_streams.target_relocation_sha256")
    candidate_relocation = _sha256(row["candidate_relocation_sha256"], "physical_streams.candidate_relocation_sha256")
    if row["authority_advanced"] is not False:
        blockers.append("physical_stream_authority_not_false")
    if target_cfg != candidate_cfg:
        blockers.append("cfg_fingerprint_differs")
    if target_relocation != candidate_relocation:
        blockers.append("stream_relocation_fingerprint_differs")
    stream_target = _physical_rows(row["target"], "physical_streams.target")
    stream_candidate = _physical_rows(row["candidate"], "physical_streams.candidate")
    if set(stream_target) != set(stream_candidate):
        blockers.append("physical_stream_row_sets_differ")
    focus_residuals = set(target_rows)
    target_residuals = {index for index in stream_target if stream_target[index].get("diff_kind") is not None}
    candidate_residuals = {index for index in stream_candidate if stream_candidate[index].get("diff_kind") is not None}
    if focus_residuals != target_residuals:
        blockers.append("target_physical_stream_focus_residual_set_mismatch")
    if focus_residuals != candidate_residuals:
        blockers.append("candidate_physical_stream_focus_residual_set_mismatch")
    for index in sorted(set(stream_target) & set(stream_candidate)):
        target_kind = stream_target[index].get("diff_kind")
        candidate_kind = stream_candidate[index].get("diff_kind")
        if target_kind != candidate_kind:
            blockers.append(f"row_{index}_physical_stream_diff_kind_mismatch")
        if index not in focus_residuals:
            if target_kind is not None or candidate_kind is not None:
                blockers.append(f"row_{index}_nonfocus_row_marked_residual")
            if _formatted(stream_target[index], f"physical_streams.target[{index}]") != _formatted(stream_candidate[index], f"physical_streams.candidate[{index}]"):
                blockers.append(f"row_{index}_nonfocus_physical_text_or_operand_mismatch")
    for index in sorted(set(target_rows) & set(stream_target) & set(stream_candidate)):
        if stream_target[index].get("diff_kind") != target_rows[index].get("diff_kind"):
            blockers.append(f"row_{index}_target_stream_focus_diff_kind_mismatch")
        if stream_candidate[index].get("diff_kind") != candidate_rows[index].get("diff_kind"):
            blockers.append(f"row_{index}_candidate_stream_focus_diff_kind_mismatch")
        if _formatted(stream_target[index], f"physical_streams.target[{index}]") != _formatted(target_rows[index], f"focus.target[{index}]"):
            blockers.append(f"row_{index}_target_stream_focus_mismatch")
        if _formatted(stream_candidate[index], f"physical_streams.candidate[{index}]") != _formatted(candidate_rows[index], f"focus.candidate[{index}]"):
            blockers.append(f"row_{index}_candidate_stream_focus_mismatch")
    return stream_target, stream_candidate


def _relocation_evidence(value: Mapping[str, Any], blockers: list[str]) -> None:
    _self_digest(value, "receipt_sha256", "physical relocation receipt")
    if value.get("physical_relocations_exact") is not True or value.get("physical_relocation_differences") != []:
        blockers.append("physical_relocations_not_exact")


def _span_evidence(
    value: Mapping[str, Any],
    function: str,
    context: Mapping[str, Any],
    blockers: list[str],
) -> dict[str, Mapping[str, Any]]:
    if value.get("schema") != SPAN_SCHEMA or value.get("function") != function:
        raise DifferentialAllocatorInputError("source-span schema/function mismatch")
    _self_digest(value, "manifest_sha256", "source spans")
    if _identifier(value.get("session_id"), "source_spans.session_id") != context["session_id"]:
        blockers.append("source_span_session_id_mismatch")
    if _sha256(value.get("trust_anchor_sha256"), "source_spans.trust_anchor_sha256") != context["trust_anchor_sha256"]:
        blockers.append("source_span_trust_anchor_mismatch")
    if value.get("authority_advanced") is not False:
        blockers.append("source_span_authority_not_false")
    source = _binary_descriptor(value.get("source"), "source_spans.source")
    _verify_binary(source, "source spans source")
    source_bytes = Path(source["path"]).read_bytes()
    spans: dict[str, Mapping[str, Any]] = {}
    for index, raw in enumerate(_sequence(value.get("spans"), "source_spans.spans")):
        item = _mapping(raw, f"source_spans.spans[{index}]")
        span_id = _identifier(item.get("span_id"), f"source_spans.spans[{index}].span_id")
        if span_id in spans:
            raise DifferentialAllocatorInputError("source spans contain duplicate IDs")
        if item.get("natural") is not True:
            blockers.append(f"source_span_{span_id}_not_natural")
        byte_start = _uint(item.get("byte_start"), f"source_spans.spans[{index}].byte_start", len(source_bytes))
        byte_end = _uint(item.get("byte_end"), f"source_spans.spans[{index}].byte_end", len(source_bytes))
        if byte_start >= byte_end or hashlib.sha256(source_bytes[byte_start:byte_end]).hexdigest() != _sha256(item.get("text_sha256"), f"source_spans.spans[{index}].text_sha256"):
            raise DifferentialAllocatorInputError(f"source span {span_id} byte binding mismatch")
        spans[span_id] = item
    return spans


def _trace_evidence(value: Mapping[str, Any], context: Mapping[str, Any], spans: Mapping[str, Mapping[str, Any]], blockers: list[str]) -> list[dict[str, Any]]:
    row = _closed(
        value,
        "trace",
        {
            "schema", "function", "session_id", "trust_anchor_sha256", "compiler_sha256",
            "tool_sha256", "authority_advanced", "owner_facts", "trace_sha256",
        },
    )
    if row["schema"] != TRACE_SCHEMA or row["function"] != context["function"]:
        raise DifferentialAllocatorInputError("trace schema/function mismatch")
    _self_digest(row, "trace_sha256", "trace")
    trace_session = _identifier(row["session_id"], "trace.session_id")
    trace_trust = _sha256(row["trust_anchor_sha256"], "trace.trust_anchor_sha256")
    if trace_session != context["session_id"]:
        blockers.append("trace_session_id_mismatch")
    if trace_trust != context["trust_anchor_sha256"]:
        blockers.append("trace_trust_anchor_mismatch")
    if _sha256(row["compiler_sha256"], "trace.compiler_sha256") != context["compiler"]["sha256"]:
        blockers.append("trace_compiler_identity_mismatch")
    if _sha256(row["tool_sha256"], "trace.tool_sha256") != context["tool"]["sha256"]:
        blockers.append("trace_tool_identity_mismatch")
    if row["authority_advanced"] is not False:
        blockers.append("trace_authority_not_false")
    facts: list[dict[str, Any]] = []
    for index, raw in enumerate(_sequence(row["owner_facts"], "trace.owner_facts")):
        label = f"trace.owner_facts[{index}]"
        item = _closed(
            raw,
            label,
            {
                "owner_id", "source_span_id", "row_indices", "pcode_def_token", "pcode_use_tokens",
                "object_token", "varinfo_id", "ig_node_id", "vreg", "candidate_physical_register",
                "target_physical_register", "interference_neighbors", "classification", "missing_edge",
                "lifetime", "session_id",
            },
        )
        owner_id = _identifier(item["owner_id"], f"{label}.owner_id")
        fact_session = _identifier(item["session_id"], f"{label}.session_id")
        if fact_session != context["session_id"] or fact_session != trace_session:
            blockers.append(f"owner_{owner_id}_session_id_mismatch")
        span_id = _identifier(item["source_span_id"], f"{label}.source_span_id")
        if span_id not in spans:
            blockers.append(f"owner_{owner_id}_source_span_missing")
        row_indices = [_uint(row, f"{label}.row_indices[{ordinal}]") for ordinal, row in enumerate(_sequence(item["row_indices"], f"{label}.row_indices"))]
        if not row_indices or len(row_indices) != len(set(row_indices)):
            raise DifferentialAllocatorInputError(f"{label}.row_indices must be nonempty and duplicate-free")
        candidate_register = _identifier(item["candidate_physical_register"], f"{label}.candidate_physical_register").lower()
        target_register = _identifier(item["target_physical_register"], f"{label}.target_physical_register").lower()
        vreg = _optional_identifier(item["vreg"], f"{label}.vreg")
        if _REGISTER_RE.fullmatch(candidate_register) is None or _REGISTER_RE.fullmatch(target_register) is None or (vreg is not None and _VREG_RE.fullmatch(vreg) is None):
            raise DifferentialAllocatorInputError(f"{label} has an invalid register/vreg")
        uses = [_identifier(use, f"{label}.pcode_use_tokens[{ordinal}]") for ordinal, use in enumerate(_sequence(item["pcode_use_tokens"], f"{label}.pcode_use_tokens"))]
        neighbors = [_identifier(neighbor, f"{label}.interference_neighbors[{ordinal}]") for ordinal, neighbor in enumerate(_sequence(item["interference_neighbors"], f"{label}.interference_neighbors"))]
        missing = item["missing_edge"]
        missing_edge = None if missing is None else _identifier(missing, f"{label}.missing_edge")
        lifetime = _closed(
            item["lifetime"],
            f"{label}.lifetime",
            {"birth", "assignment", "first_use", "last_use"},
        )
        ordered_lifetime = {
            key: _uint(lifetime[key], f"{label}.lifetime.{key}")
            for key in ("birth", "assignment", "first_use", "last_use")
        }
        if list(ordered_lifetime.values()) != sorted(ordered_lifetime.values()):
            missing_edge = missing_edge or "noncanonical_lifetime_chronology"
        classification = item["classification"]
        if classification not in {"UNIQUE", "UNKNOWN"}:
            raise DifferentialAllocatorInputError(f"{label}.classification must be UNIQUE or UNKNOWN")
        object_token = _optional_identifier(item["object_token"], f"{label}.object_token")
        varinfo_id = _optional_identifier(item["varinfo_id"], f"{label}.varinfo_id")
        pcode_def = _optional_identifier(item["pcode_def_token"], f"{label}.pcode_def_token")
        ig_node = _optional_identifier(item["ig_node_id"], f"{label}.ig_node_id")
        if classification != "UNIQUE" or not uses or not neighbors or None in {object_token, varinfo_id, pcode_def, ig_node, vreg}:
            missing_edge = missing_edge or "incomplete_object_varinfo_pcode_ig_vreg_chain"
        facts.append({
            "owner_id": owner_id,
            "session_id": fact_session,
            "source_span_id": span_id,
            "row_indices": row_indices,
            "object_token": object_token,
            "varinfo_id": varinfo_id,
            "pcode_def_token": pcode_def,
            "pcode_use_tokens": uses,
            "ig_node_id": ig_node,
            "vreg": vreg,
            "candidate": candidate_register,
            "target_claim": target_register,
            "target": None,
            "target_residual_interval": None,
            "interference_neighbors": neighbors,
            "lifetime": ordered_lifetime,
            "missing_edge": missing_edge,
        })
    owner_ids = [fact["owner_id"] for fact in facts]
    object_tokens = [fact["object_token"] for fact in facts if fact["object_token"] is not None]
    pcode_defs = [fact["pcode_def_token"] for fact in facts if fact["pcode_def_token"] is not None]
    if len(owner_ids) != len(set(owner_ids)) or len(object_tokens) != len(set(object_tokens)) or len(pcode_defs) != len(set(pcode_defs)):
        raise DifferentialAllocatorInputError("trace owner/object/PCode-def identities must be unique")
    return facts


def _interaction_request(function: str, source_class: str, axes: Sequence[Mapping[str, str]], evidence_sha256: str) -> dict[str, Any]:
    request = {
        "schema": REQUEST_SCHEMA,
        "function": function,
        "source_class": source_class,
        "cell_id": f"composed_{source_class}",
        "composed_axes": [
            {
                "id": axis["id"],
                "hypothesis": axis["hypothesis"],
                "source_action": axis["source_action"],
                "topology_token": axis["topology_token"],
            }
            for axis in axes
        ],
        "evidence_sha256": evidence_sha256,
        "max_cells": 1,
        "matrix_expansion": False,
        "compile_authorized": False,
        "authority_advanced": False,
    }
    request["request_sha256"] = canonical_sha256(request)
    return request


def solve(
    context_value: Mapping[str, Any],
    focus: Mapping[str, Any],
    physical_streams: Mapping[str, Any],
    relocation_receipt: Mapping[str, Any],
    source_spans: Mapping[str, Any],
    trace: Mapping[str, Any],
    expected_context_sha256: str,
) -> dict[str, Any]:
    expected = _sha256(expected_context_sha256, "expected context SHA-256")
    if context_value.get("context_sha256") != expected:
        raise DifferentialAllocatorInputError("context differs from caller-supplied trust anchor")
    context = parse_context(context_value)
    supplied_payloads = {
        "focus": focus.get("artifact_sha256"),
        "physical_streams": physical_streams.get("stream_sha256"),
        "physical_relocation_receipt": relocation_receipt.get("receipt_sha256"),
        "source_spans": source_spans.get("manifest_sha256"),
        "trace": trace.get("trace_sha256"),
    }
    for label, supplied in supplied_payloads.items():
        if supplied != context[label]["payload_sha256"]:
            raise DifferentialAllocatorInputError(f"{label} payload differs from context binding")
    blockers: list[str] = []
    target_rows, candidate_rows = _focus_evidence(focus, context["function"], blockers)
    stream_target_rows, stream_candidate_rows = _stream_evidence(
        physical_streams, context["function"], target_rows, candidate_rows, blockers
    )
    _relocation_evidence(relocation_receipt, blockers)
    pre_allocator_blockers = sorted(set(blockers))
    facts: list[dict[str, Any]] = []
    spans: dict[str, Mapping[str, Any]] = {}
    inference_started = not pre_allocator_blockers and bool(target_rows)
    if not target_rows:
        blockers.append("focus_has_zero_residual_groups")
        inference_started = False
    if inference_started:
        spans = _span_evidence(source_spans, context["function"], context, blockers)
        facts = _trace_evidence(trace, context, spans, blockers)

    focus_rows = set(target_rows)
    observed_rows = {row for fact in facts for row in fact["row_indices"]}
    row_owner_counts = {
        row: sum(row in fact["row_indices"] for fact in facts)
        for row in observed_rows
    }
    owner_ids = {fact["owner_id"] for fact in facts}
    mapping: dict[str, str] = {}
    if inference_started:
        if observed_rows != focus_rows:
            blockers.append("trace_does_not_cover_exact_focus_residual")
        for row, count in sorted(row_owner_counts.items()):
            if count != 1:
                blockers.append(f"row_{row}_has_{count}_owner_facts")
        for index in sorted(focus_rows):
            if _skeleton(target_rows[index], f"focus.target[{index}]") != _skeleton(candidate_rows[index], f"focus.candidate[{index}]"):
                blockers.append(f"row_{index}_is_not_register_only")
        for fact in facts:
            if fact["missing_edge"] is not None:
                blockers.append(f"owner_{fact['owner_id']}_missing_edge:{fact['missing_edge']}")
            anchors: list[dict[str, Any]] = []
            derived_targets: set[str] = set()
            for row_index in fact["row_indices"]:
                if row_index not in target_rows:
                    continue
                if fact["candidate"] not in _registers(candidate_rows[row_index], f"focus.candidate[{row_index}]"):
                    blockers.append(f"owner_{fact['owner_id']}_candidate_register_not_in_row_{row_index}")
                candidate_registers = _registers(stream_candidate_rows[row_index], f"physical_streams.candidate[{row_index}]")
                target_registers = _registers(stream_target_rows[row_index], f"physical_streams.target[{row_index}]")
                if len(candidate_registers) != len(target_registers):
                    blockers.append(f"row_{row_index}_register_arity_differs")
                    continue
                positions = [position for position, register in enumerate(candidate_registers) if register == fact["candidate"]]
                if not positions:
                    blockers.append(f"owner_{fact['owner_id']}_candidate_register_not_in_stream_row_{row_index}")
                    continue
                row_targets = {target_registers[position] for position in positions}
                if len(row_targets) != 1:
                    blockers.append(f"owner_{fact['owner_id']}_target_pseudo_owner_ambiguous_at_row_{row_index}")
                    continue
                derived_target = next(iter(row_targets))
                derived_targets.add(derived_target)
                anchors.append({
                    "row_index": row_index,
                    "operand_positions": positions,
                    "candidate_register": fact["candidate"],
                    "target_register": derived_target,
                })
            if len(derived_targets) != 1:
                blockers.append(f"owner_{fact['owner_id']}_target_pseudo_owner_not_unique")
                continue
            derived_target = next(iter(derived_targets))
            fact["target"] = derived_target
            fact["target_residual_interval"] = {
                "first_row": min(fact["row_indices"]),
                "last_row": max(fact["row_indices"]),
                "anchors": anchors,
            }
            if fact["target_claim"] != derived_target:
                blockers.append(f"owner_{fact['owner_id']}_target_register_claim_mismatch")
            previous = mapping.get(fact["candidate"])
            if previous is not None and previous != derived_target:
                blockers.append(f"candidate_register_{fact['candidate']}_has_conflicting_targets")
            mapping[fact["candidate"]] = derived_target
        changed = {source: target for source, target in mapping.items() if source != target}
        if not changed:
            blockers.append("no_changed_allocator_mapping")
        elif set(changed) != set(changed.values()):
            blockers.append("allocator_mapping_is_not_a_closed_permutation")

    eligible = [
        hypothesis
        for hypothesis in context["hypotheses"]
        if set(hypothesis["owner_ids"]) == owner_ids and set(hypothesis["row_indices"]) == focus_rows
        and set(hypothesis["suppresses_control_ids"])
        == {control["control_id"] for control in context["rejected_controls"]}
    ]
    permutation_complete = inference_started and not blockers
    frontier_size = min((len(item["axes"]) for item in eligible), default=None)
    minimum_frontier = sorted(
        (item for item in eligible if len(item["axes"]) == frontier_size),
        key=lambda item: item["source_class"],
    )[:3]
    if inference_started and len(minimum_frontier) != 1:
        blockers.append("minimum_causal_frontier_not_unique")

    blockers = sorted(set(blockers))
    status = "RANKED_SOURCE_CLASS" if not blockers else "UNKNOWN"
    selected = minimum_frontier[0] if status == "RANKED_SOURCE_CLASS" else None
    missing_edges = sorted(blocker for blocker in blockers if "missing_edge:" in blocker)
    first_missing_edge = missing_edges[0] if missing_edges else (blockers[0] if blockers else None)
    evidence_sha256 = canonical_sha256({
        "context": context["context_sha256"],
        "focus": focus.get("artifact_sha256"),
        "streams": physical_streams.get("stream_sha256"),
        "trace": trace.get("trace_sha256"),
    })
    result: dict[str, Any] = {
        "schema": SCHEMA,
        "status": status,
        "function": context["function"],
        "inference_started": inference_started,
        "failure_stage": (None if status == "RANKED_SOURCE_CLASS" else ("pre_allocator_gate" if pre_allocator_blockers or not inference_started else "allocator_join")),
        "first_missing_edge": first_missing_edge,
        "maximal_closed_permutation": {
            "complete": permutation_complete,
            "mapping": [{"candidate": source, "target": target} for source, target in sorted(mapping.items())],
            "row_indices": sorted(observed_rows),
            "owner_ids": sorted(owner_ids),
        },
        "minimum_causal_frontier": {
            "unique": status == "RANKED_SOURCE_CLASS",
            "axis_count": (len(selected["axes"]) if selected is not None else frontier_size),
            "source_class": (selected["source_class"] if selected is not None else None),
        },
        "ranked_source_class": (
            {"rank": 1, "source_class": selected["source_class"]} if selected is not None else None
        ),
        "ranked_source_classes": [
            {
                "rank": rank,
                "source_class": hypothesis["source_class"],
                "axis_count": len(hypothesis["axes"]),
                "suppresses_control_ids": hypothesis["suppresses_control_ids"],
            }
            for rank, hypothesis in enumerate(minimum_frontier, 1)
        ],
        "candidate_interaction_request": (
            _interaction_request(context["function"], selected["source_class"], selected["axes"], evidence_sha256)
            if selected is not None else None
        ),
        "owner_facts": facts,
        "rejected_controls": context["rejected_controls"],
        "evidence_binding": {
            "session_id": context["session_id"],
            "trust_anchor_sha256": context["trust_anchor_sha256"],
            "context_sha256": context["context_sha256"],
            "focus_artifact_sha256": focus.get("artifact_sha256"),
            "physical_stream_sha256": physical_streams.get("stream_sha256"),
            "physical_relocation_receipt_sha256": relocation_receipt.get("receipt_sha256"),
            "source_span_manifest_sha256": source_spans.get("manifest_sha256"),
            "trace_sha256": trace.get("trace_sha256"),
            "compiler_sha256": context["compiler"]["sha256"],
            "tool_sha256": context["tool"]["sha256"],
        },
        "blockers": blockers,
        "diagnostic_only": True,
        "source_text_emitted": False,
        "source_patch_emitted": False,
        "compile_authorized": False,
        "retention_authorized": False,
        "authority_advanced": False,
    }
    result["result_sha256"] = canonical_sha256(result)
    return result


def solve_from_paths(context_path: Path, expected_context_sha256: str) -> dict[str, Any]:
    if not context_path.is_absolute() or context_path.is_symlink() or not context_path.is_file():
        raise DifferentialAllocatorInputError("context must be an absolute regular file")
    try:
        context_value = _mapping(json.loads(context_path.read_text(encoding="utf-8")), "context")
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise DifferentialAllocatorInputError(f"cannot parse context: {exc}") from exc
    context = parse_context(context_value)
    for label in ("compiler", "tool"):
        _verify_binary(context[label], label)
    return solve(
        context_value,
        _read_bound(context["focus"], "focus"),
        _read_bound(context["physical_streams"], "physical streams"),
        _read_bound(context["physical_relocation_receipt"], "physical relocation receipt"),
        _read_bound(context["source_spans"], "source spans"),
        _read_bound(context["trace"], "trace"),
        expected_context_sha256,
    )


def _atomic_write(path: Path, text: str) -> None:
    if path.exists() or path.is_symlink():
        raise DifferentialAllocatorInputError("output path already exists; refusing to overwrite immutable evidence")
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary = tempfile.mkstemp(prefix=f".{path.name}.", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    except BaseException:
        try:
            os.unlink(temporary)
        except OSError:
            pass
        raise


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", type=Path, required=True)
    parser.add_argument("--context-sha256", required=True)
    parser.add_argument("--output", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = solve_from_paths(args.context, args.context_sha256)
    except DifferentialAllocatorInputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    rendered = json.dumps(result, ensure_ascii=True, sort_keys=True, indent=2) + "\n"
    if args.output:
        try:
            _atomic_write(args.output, rendered)
        except DifferentialAllocatorInputError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
    else:
        sys.stdout.write(rendered)
    return 0 if result["status"] == "RANKED_SOURCE_CLASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
