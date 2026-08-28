#!/usr/bin/env python3
"""Fail-closed join of residual register rows to volatile-owner evidence.

The reducer is deliberately diagnostic-only.  It accepts immutable, externally
hash-bound evidence and either proves one closed register permutation with one
allowlisted natural source class, or emits deterministic ``UNKNOWN``.  It never
emits source, retains a candidate, or advances recovery authority.
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


SCHEMA = "volatile_owner_causal_join/v1"
ENRICHED_SCHEMA = "volatile_owner_causal_join/v2"
CONTEXT_SCHEMA = "volatile_owner_causal_join_context/v1"
FOCUS_SCHEMA = "focus_symbol_report/v1"
GRAPH_SCHEMA = "mwcc_capsule_same_session_ownership_failure_graph/v1"
SPAN_SCHEMAS = {"mwcc_source_span_bindings/v1", "mwcc_source_span_bindings/v2"}

_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_SAFE_TOKEN_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_.:+@/-]{0,191}$")
_REGISTER_RE = re.compile(r"\b([rf](?:[0-9]|[12][0-9]|3[01]))\b", re.IGNORECASE)
_VREG_RE = re.compile(r"^[rf][0-9]+$", re.IGNORECASE)
_PCODE_ID_RE = re.compile(r"^pcode-(?P<session>session-[0-9a-f]{16})-[0-9]{6}$")
_IG_ID_RE = re.compile(r"^ig-(?P<session>session-[0-9a-f]{16})-[0-9]{6}$")
_FACT_ID_RE = re.compile(r"^owner-fact-[0-9]{6}$")
_ROW_ID_RE = re.compile(r"^residual-[0-9]{6}$")
_DEF_ID_RE = re.compile(r"^def-[0-9]{6}$")
_USE_ID_RE = re.compile(r"^use-[0-9]{6}$")
_OBJECT_TOKEN_RE = re.compile(r"^(?:local|argument)-(?P<session>session-[0-9a-f]{16})-[0-9]{6}$")
_HIDDEN_OBJECT_TOKEN_RE = re.compile(r"^hidden-ig-(?P<session>session-[0-9a-f]{16})-[0-9]{6}$")
_EVENT_ID_RE = re.compile(r"^(?P<session>session-[0-9a-f]{16})-e[0-9]{6}$")
_POINTER_LIKE_RE = re.compile(r"(?i)(?:0x[0-9a-f]{8,}|\b(?:ptr|pointer|address)\s*[:=])")
_SEMANTIC_ROLES = {
    "index",
    "base",
    "result",
    "left",
    "right",
    "lhs",
    "rhs",
    "arithmetic_operand_0",
    "arithmetic_operand_1",
    "arithmetic_operand_2",
}


class VolatileOwnerJoinInputError(ValueError):
    """An input is malformed, unsafe, or not authenticated by its manifest."""


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
        raise VolatileOwnerJoinInputError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise VolatileOwnerJoinInputError(f"{label} must be an object")
    return value


def _sequence(value: Any, label: str) -> list[Any]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)):
        raise VolatileOwnerJoinInputError(f"{label} must be an array")
    return list(value)


def _closed(value: Any, label: str, required: set[str], optional: set[str] | None = None) -> Mapping[str, Any]:
    row = _mapping(value, label)
    allowed = required | (optional or set())
    missing = sorted(required - set(row))
    extra = sorted(set(row) - allowed)
    if missing:
        raise VolatileOwnerJoinInputError(f"{label} missing fields: {', '.join(missing)}")
    if extra:
        raise VolatileOwnerJoinInputError(f"{label} has unsupported fields: {', '.join(extra)}")
    return row


def _text(value: Any, label: str, *, limit: int = 192) -> str:
    if not isinstance(value, str) or not value or len(value) > limit:
        raise VolatileOwnerJoinInputError(f"{label} must be nonempty text <= {limit} characters")
    return value


def _token(value: Any, label: str) -> str:
    result = _text(value, label)
    if _SAFE_TOKEN_RE.fullmatch(result) is None:
        raise VolatileOwnerJoinInputError(f"{label} is not a bounded pointer-free token")
    if _POINTER_LIKE_RE.search(result):
        raise VolatileOwnerJoinInputError(f"{label} contains a pointer-like token")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if _SHA256_RE.fullmatch(result) is None:
        raise VolatileOwnerJoinInputError(f"{label} must be lowercase SHA-256")
    return result


def _uint(value: Any, label: str, *, maximum: int = 1 << 31) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= maximum:
        raise VolatileOwnerJoinInputError(f"{label} must be an unsigned integer")
    return value


def _json(path: Path, expected_sha256: str, label: str) -> Mapping[str, Any]:
    actual = file_sha256(path)
    if actual != expected_sha256:
        raise VolatileOwnerJoinInputError(
            f"{label} SHA-256 mismatch: expected {expected_sha256}, got {actual}"
        )
    try:
        return _mapping(json.loads(path.read_text(encoding="utf-8")), label)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise VolatileOwnerJoinInputError(f"cannot parse {label} {path}: {exc}") from exc


def _descriptor(value: Any, label: str, *, artifact_field: str | None = None) -> dict[str, Any]:
    required = {"file_sha256"} | ({artifact_field} if artifact_field else set())
    row = _closed(value, label, required)
    result = {"file_sha256": _sha256(row["file_sha256"], f"{label}.file_sha256")}
    if artifact_field:
        result[artifact_field] = _sha256(row[artifact_field], f"{label}.{artifact_field}")
    return result


def _object(value: Any, label: str) -> dict[str, Any]:
    row = _closed(value, label, {"sha256", "size"}, {"path"})
    return {
        "sha256": _sha256(row["sha256"], f"{label}.sha256"),
        "size": _uint(row["size"], f"{label}.size"),
        "path": (_text(row["path"], f"{label}.path", limit=4096) if "path" in row else None),
    }


def _physical_descriptor(value: Any, label: str) -> dict[str, Any]:
    row = _closed(value, label, {"file_sha256", "receipt_payload_sha256"}, {"path"})
    return {
        "file_sha256": _sha256(row["file_sha256"], f"{label}.file_sha256"),
        "receipt_payload_sha256": _sha256(row["receipt_payload_sha256"], f"{label}.receipt_payload_sha256"),
        "path": (_text(row["path"], f"{label}.path", limit=4096) if "path" in row else None),
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    row = _closed(
        value,
        "context",
        {
            "schema", "session_id", "function", "focus", "source_span_manifest",
            "target_object", "candidate_object", "ownership_failure_graph",
            "source_class_allowlist", "residual_row_bindings",
            "source_class_hypotheses", "strict_report_sha256",
        },
        {"context_sha256", "data_report_sha256", "physical_relocation_receipt"},
    )
    if row["schema"] != CONTEXT_SCHEMA:
        raise VolatileOwnerJoinInputError(f"context.schema must be {CONTEXT_SCHEMA}")
    allowlist = [_token(item, f"context.source_class_allowlist[{index}]") for index, item in enumerate(_sequence(row["source_class_allowlist"], "context.source_class_allowlist"))]
    if not allowlist or len(allowlist) != len(set(allowlist)):
        raise VolatileOwnerJoinInputError("context.source_class_allowlist must be nonempty and duplicate-free")
    row_bindings: list[dict[str, Any]] = []
    bound_ids: set[str] = set()
    bound_positions: set[tuple[int, int]] = set()
    for index, raw in enumerate(_sequence(row["residual_row_bindings"], "context.residual_row_bindings")):
        label = f"context.residual_row_bindings[{index}]"
        binding = _closed(raw, label, {"row_id", "focus_row_index", "captured_role", "semantic_role"}, {"candidate_operand_index"})
        parsed = {
            "row_id": _token(binding["row_id"], f"{label}.row_id"),
            "focus_row_index": _uint(binding["focus_row_index"], f"{label}.focus_row_index"),
            "captured_role": _token(binding["captured_role"], f"{label}.captured_role"),
            "semantic_role": _token(binding["semantic_role"], f"{label}.semantic_role"),
            "candidate_operand_index": (_uint(binding["candidate_operand_index"], f"{label}.candidate_operand_index") if "candidate_operand_index" in binding else None),
        }
        if parsed["semantic_role"] not in _SEMANTIC_ROLES:
            raise VolatileOwnerJoinInputError(f"{label}.semantic_role is unsupported")
        position = (parsed["focus_row_index"], parsed["candidate_operand_index"] if parsed["candidate_operand_index"] is not None else -1)
        if parsed["row_id"] in bound_ids or position in bound_positions:
            raise VolatileOwnerJoinInputError("context.residual_row_bindings contain a duplicate row or operand position")
        bound_ids.add(parsed["row_id"])
        bound_positions.add(position)
        row_bindings.append(parsed)
    if not row_bindings:
        raise VolatileOwnerJoinInputError("context.residual_row_bindings must not be empty")
    hypotheses: list[dict[str, Any]] = []
    seen_classes: set[str] = set()
    for index, raw in enumerate(_sequence(row["source_class_hypotheses"], "context.source_class_hypotheses")):
        label = f"context.source_class_hypotheses[{index}]"
        hypothesis = _closed(raw, label, {"source_class", "row_ids"})
        source_class = _token(hypothesis["source_class"], f"{label}.source_class")
        row_ids = [_token(item, f"{label}.row_ids[{item_index}]") for item_index, item in enumerate(_sequence(hypothesis["row_ids"], f"{label}.row_ids"))]
        if source_class in seen_classes or not row_ids or len(row_ids) != len(set(row_ids)):
            raise VolatileOwnerJoinInputError("context source-class hypotheses must be nonempty and duplicate-free")
        seen_classes.add(source_class)
        hypotheses.append({"source_class": source_class, "row_ids": row_ids})
    return {
        "session_id": _token(row["session_id"], "context.session_id"),
        "function": _token(row["function"], "context.function"),
        "strict_report_sha256": _sha256(row["strict_report_sha256"], "context.strict_report_sha256"),
        "data_report_sha256": (_sha256(row["data_report_sha256"], "context.data_report_sha256") if "data_report_sha256" in row else None),
        "context_sha256": (_sha256(row["context_sha256"], "context.context_sha256") if "context_sha256" in row else None),
        "physical_relocation_receipt": (_physical_descriptor(row["physical_relocation_receipt"], "context.physical_relocation_receipt") if "physical_relocation_receipt" in row else None),
        "focus": _descriptor(row["focus"], "context.focus", artifact_field="artifact_sha256"),
        "source_span_manifest": _descriptor(row["source_span_manifest"], "context.source_span_manifest", artifact_field="manifest_sha256"),
        "target_object": _object(row["target_object"], "context.target_object"),
        "candidate_object": _object(row["candidate_object"], "context.candidate_object"),
        "ownership_failure_graph": _descriptor(row["ownership_failure_graph"], "context.ownership_failure_graph", artifact_field="failure_graph_sha256"),
        "source_class_allowlist": allowlist,
        "residual_row_bindings": row_bindings,
        "source_class_hypotheses": hypotheses,
    }


def _self_digest(value: Mapping[str, Any], field: str, label: str) -> None:
    expected = _sha256(value.get(field), f"{label}.{field}")
    unsigned = dict(value)
    unsigned.pop(field, None)
    actual = canonical_sha256(unsigned)
    if actual != expected:
        raise VolatileOwnerJoinInputError(f"{label} self-digest mismatch")


def _verify_context_digest(value: Mapping[str, Any], blockers: list[str]) -> None:
    if "context_sha256" not in value:
        blockers.append("context_self_digest_missing")
        return
    _self_digest(value, "context_sha256", "context")


def _verify_object_file(descriptor: Mapping[str, Any], label: str, blockers: list[str]) -> None:
    raw_path = descriptor.get("path")
    if not isinstance(raw_path, str):
        blockers.append(f"{label}_path_missing")
        return
    path = Path(raw_path)
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        blockers.append(f"{label}_path_not_verified_regular_file")
        return
    try:
        size = path.stat().st_size
    except OSError:
        blockers.append(f"{label}_path_not_verified_regular_file")
        return
    if size != descriptor["size"] or file_sha256(path) != descriptor["sha256"]:
        blockers.append(f"{label}_file_identity_mismatch")


def _verify_physical_receipt_file(descriptor: Mapping[str, Any] | None, strict_report_sha256: str, blockers: list[str]) -> None:
    if descriptor is None or not isinstance(descriptor.get("path"), str):
        blockers.append("physical_relocation_receipt_path_missing")
        return
    path = Path(str(descriptor["path"]))
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        blockers.append("physical_relocation_receipt_path_not_verified")
        return
    if file_sha256(path) != descriptor["file_sha256"]:
        blockers.append("physical_relocation_receipt_file_hash_mismatch")
        return
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError):
        blockers.append("physical_relocation_receipt_file_invalid")
        return
    if canonical_sha256(value) != descriptor["receipt_payload_sha256"]:
        blockers.append("physical_relocation_receipt_payload_hash_mismatch")
        return
    report = value.get("report") if isinstance(value, Mapping) else None
    if (
        not isinstance(value, Mapping)
        or value.get("physical_relocations_exact") is not True
        or value.get("physical_relocation_differences") != []
        or not isinstance(report, Mapping)
        or report.get("sha256") != strict_report_sha256
    ):
        blockers.append("physical_relocation_receipt_payload_not_exact")


def _row_map(side: Any, label: str) -> dict[int, Mapping[str, Any]]:
    side_row = _mapping(side, label)
    rows_kind = side_row.get("rows_kind")
    if rows_kind not in {"all", "diff_only"}:
        raise VolatileOwnerJoinInputError(f"{label}.rows_kind must be all or diff_only")
    result: dict[int, Mapping[str, Any]] = {}
    for ordinal, raw in enumerate(_sequence(side_row.get("rows"), f"{label}.rows")):
        item = _mapping(raw, f"{label}.rows[{ordinal}]")
        if rows_kind == "all" and not item.get("diff_kind"):
            continue
        index = _uint(item.get("index"), f"{label}.rows[{ordinal}].index")
        if index in result:
            raise VolatileOwnerJoinInputError(f"{label} contains duplicate row {index}")
        result[index] = item
    if side_row.get("diff_row_count") != len(result):
        raise VolatileOwnerJoinInputError(f"{label}.diff_row_count does not match its rows")
    return result


def _focus_rows(focus: Mapping[str, Any], context: Mapping[str, Any], blockers: list[str]) -> tuple[dict[int, Mapping[str, Any]], dict[int, Mapping[str, Any]]]:
    if focus.get("schema") != FOCUS_SCHEMA:
        raise VolatileOwnerJoinInputError(f"focus.schema must be {FOCUS_SCHEMA}")
    _self_digest(focus, "artifact_sha256", "focus")
    if focus.get("artifact_sha256") != context["focus"]["artifact_sha256"]:
        raise VolatileOwnerJoinInputError("focus artifact identity differs from context")
    input_binding = _mapping(focus.get("input_binding"), "focus.input_binding")
    strict_report = _mapping(input_binding.get("strict_report"), "focus.input_binding.strict_report")
    if strict_report.get("sha256") != context["strict_report_sha256"]:
        raise VolatileOwnerJoinInputError("focus strict-report identity differs from context")
    data_report = input_binding.get("data_report")
    if context["data_report_sha256"] is None or not isinstance(data_report, Mapping):
        blockers.append("data_report_binding_missing")
    elif data_report.get("sha256") != context["data_report_sha256"]:
        raise VolatileOwnerJoinInputError("focus data-report identity differs from context")
    if input_binding.get("retail_target_authenticated") is not True or input_binding.get("authority_advanced") is not False:
        blockers.append("focus_input_binding_policy_mismatch")
    if focus.get("authority_advanced") is not False:
        blockers.append("focus_authority_not_false")
    if focus.get("function") != context["function"]:
        blockers.append("focus_function_mismatch")
    channels = _mapping(focus.get("channels"), "focus.channels")
    strict = _mapping(channels.get("strict"), "focus.channels.strict")
    target = _row_map(strict.get("target"), "focus.channels.strict.target")
    candidate = _row_map(strict.get("candidate"), "focus.channels.strict.candidate")
    strict_metric = _mapping(strict.get("metric"), "focus.channels.strict.metric")
    if strict_metric.get("diff_rows") != len(target) or strict_metric.get("diff_rows") != len(candidate):
        blockers.append("strict_metric_row_count_mismatch")
    if set(target) != set(candidate):
        blockers.append("strict_target_candidate_row_set_mismatch")
    for index in sorted(set(target) & set(candidate)):
        if target[index].get("diff_kind") != "DIFF_ARG_MISMATCH" or candidate[index].get("diff_kind") != "DIFF_ARG_MISMATCH":
            blockers.append(f"row_{index}_not_argument_mismatch")
    data = channels.get("data")
    if isinstance(data, Mapping):
        data_target = _row_map(data.get("target"), "focus.channels.data.target")
        data_candidate = _row_map(data.get("candidate"), "focus.channels.data.candidate")
        data_metric = _mapping(data.get("metric"), "focus.channels.data.metric")
        if data_metric.get("diff_rows") != len(data_target) or data_metric.get("diff_rows") != len(data_candidate):
            blockers.append("data_metric_row_count_mismatch")
        if set(data_target) != set(target) or set(data_candidate) != set(candidate):
            blockers.append("strict_data_row_set_mismatch")
    else:
        blockers.append("data_channel_missing")

    physical = focus.get("physical_relocations")
    receipt = context["physical_relocation_receipt"]
    if not isinstance(physical, Mapping) or receipt is None:
        blockers.append("exact_physical_relocation_receipt_missing")
    else:
        binding = physical.get("binding")
        if (
            physical.get("status") != "exact"
            or physical.get("authority") != "independent_physical_receipt"
            or physical.get("physical_relocation_differences") != []
            or physical.get("receipt_payload_sha256") != receipt["receipt_payload_sha256"]
            or not isinstance(binding, Mapping)
            or binding.get("sha256") != receipt["file_sha256"]
        ):
            blockers.append("physical_relocation_receipt_not_exact_or_hash_bound")
    return target, candidate


def _validate_span(
    span: Mapping[str, Any], context: Mapping[str, Any], blockers: list[str]
) -> dict[str, dict[str, Any]]:
    if span.get("schema") not in SPAN_SCHEMAS:
        raise VolatileOwnerJoinInputError("source-span manifest schema is unsupported")
    required = {"schema", "function", "function_sha256", "session_id", "source", "spans", "authority_advanced", "manifest_sha256"}
    if span.get("schema") == "mwcc_source_span_bindings/v2":
        required.add("objects")
    _closed(span, "source-span manifest", required)
    _sha256(span.get("function_sha256"), "source-span manifest.function_sha256")
    source = _closed(span.get("source"), "source-span manifest.source", {"path", "size", "sha256"})
    _text(source["path"], "source-span manifest.source.path", limit=4096)
    _uint(source["size"], "source-span manifest.source.size")
    _sha256(source["sha256"], "source-span manifest.source.sha256")
    raw_spans = _sequence(span.get("spans"), "source-span manifest.spans")
    _self_digest(span, "manifest_sha256", "source-span manifest")
    if span.get("manifest_sha256") != context["source_span_manifest"]["manifest_sha256"]:
        raise VolatileOwnerJoinInputError("source-span manifest identity differs from context")
    if span.get("session_id") != context["session_id"]:
        blockers.append("source_span_session_mismatch")
    if span.get("function") != context["function"]:
        blockers.append("source_span_function_mismatch")
    if span.get("authority_advanced") is not False:
        blockers.append("source_span_authority_not_false")
    if span.get("schema") != "mwcc_source_span_bindings/v2":
        return {}

    source_path = Path(str(source["path"]))
    source_bytes: bytes | None = None
    if not source_path.is_absolute() or source_path.is_symlink() or not source_path.is_file():
        blockers.append("source_span_source_path_not_verified_regular_file")
    else:
        try:
            source_bytes = source_path.read_bytes()
        except OSError:
            blockers.append("source_span_source_path_not_verified_regular_file")
        else:
            if len(source_bytes) != source["size"] or hashlib.sha256(source_bytes).hexdigest() != source["sha256"]:
                blockers.append("source_span_source_file_identity_mismatch")

    object_tokens: set[str] = set()
    for index, raw in enumerate(_sequence(span.get("objects"), "source-span manifest.objects")):
        label = f"source-span manifest.objects[{index}]"
        item = _closed(raw, label, {"byte_size", "identity", "object_token", "object_type", "ownership_mode"})
        token = _token(item["object_token"], f"{label}.object_token")
        token_match = _OBJECT_TOKEN_RE.fullmatch(token)
        if token_match is None or token_match.group("session") != context["session_id"]:
            raise VolatileOwnerJoinInputError(f"{label}.object_token is not a canonical same-session token")
        _uint(item["byte_size"], f"{label}.byte_size")
        _token(item["identity"], f"{label}.identity")
        _token(item["object_type"], f"{label}.object_type")
        _token(item["ownership_mode"], f"{label}.ownership_mode")
        if token in object_tokens:
            raise VolatileOwnerJoinInputError("source-span manifest objects contain duplicate tokens")
        object_tokens.add(token)

    declarations: dict[str, dict[str, Any]] = {}
    for index, raw in enumerate(raw_spans):
        label = f"source-span manifest.spans[{index}]"
        item = _closed(
            raw,
            label,
            {
                "byte_end", "byte_start", "dependency_id", "identity", "line_end",
                "line_start", "machine_instruction_indices", "object_token", "role",
                "text_sha256",
            },
        )
        token = _token(item["object_token"], f"{label}.object_token")
        if token not in object_tokens:
            raise VolatileOwnerJoinInputError(f"{label}.object_token has no declared object")
        identity = _token(item["identity"], f"{label}.identity")
        role = _token(item["role"], f"{label}.role")
        byte_start = _uint(item["byte_start"], f"{label}.byte_start")
        byte_end = _uint(item["byte_end"], f"{label}.byte_end")
        line_start = _uint(item["line_start"], f"{label}.line_start")
        line_end = _uint(item["line_end"], f"{label}.line_end")
        text_sha256 = _sha256(item["text_sha256"], f"{label}.text_sha256")
        if byte_end < byte_start or line_end < line_start:
            raise VolatileOwnerJoinInputError(f"{label} has a reversed source range")
        machine_indices = [
            _uint(value, f"{label}.machine_instruction_indices[{ordinal}]")
            for ordinal, value in enumerate(_sequence(item["machine_instruction_indices"], f"{label}.machine_instruction_indices"))
        ]
        if len(machine_indices) != len(set(machine_indices)):
            raise VolatileOwnerJoinInputError(f"{label}.machine_instruction_indices contains duplicates")
        if source_bytes is not None:
            if byte_end > len(source_bytes):
                blockers.append(f"source_span_{index}_range_out_of_bounds")
            elif hashlib.sha256(source_bytes[byte_start:byte_end]).hexdigest() != text_sha256:
                blockers.append(f"source_span_{index}_text_hash_mismatch")
        descriptor = {
            "identity": identity,
            "role": role,
            "object_token": token,
            "byte_start": byte_start,
            "byte_end": byte_end,
            "line_start": line_start,
            "line_end": line_end,
            "text_sha256": text_sha256,
        }
        if role == "declaration":
            if token in declarations:
                blockers.append(f"source_object_{token}_declaration_not_unique")
            else:
                declarations[token] = descriptor
    return declarations


def _capture_binding(
    section: Mapping[str, Any], context: Mapping[str, Any], span: Mapping[str, Any], blockers: list[str]
) -> dict[str, dict[str, str]]:
    source = _mapping(section.get("source"), "graph.volatile_owner_facts.source")
    compiler = _mapping(section.get("compiler"), "graph.volatile_owner_facts.compiler")
    candidate = section.get("candidate_object")
    for descriptor, label in ((source, "source"), (compiler, "compiler")):
        _closed(descriptor, f"graph.volatile_owner_facts.{label}", {"path", "size", "sha256"})
        _text(descriptor["path"], f"graph.volatile_owner_facts.{label}.path", limit=4096)
        _uint(descriptor["size"], f"graph.volatile_owner_facts.{label}.size")
        _sha256(descriptor["sha256"], f"graph.volatile_owner_facts.{label}.sha256")
    if not isinstance(candidate, Mapping):
        blockers.append("graph_candidate_object_missing")
    else:
        _closed(candidate, "graph.volatile_owner_facts.candidate_object", {"path", "size", "sha256"})
        _text(candidate["path"], "graph.volatile_owner_facts.candidate_object.path", limit=4096)
        candidate_sha = _sha256(candidate["sha256"], "graph.volatile_owner_facts.candidate_object.sha256")
        candidate_size = _uint(candidate["size"], "graph.volatile_owner_facts.candidate_object.size")
        if candidate_sha != context["candidate_object"]["sha256"] or candidate_size != context["candidate_object"]["size"]:
            blockers.append("graph_candidate_object_identity_mismatch")
    hashes = _sequence(section.get("raw_event_hashes"), "graph.volatile_owner_facts.raw_event_hashes")
    if not hashes:
        raise VolatileOwnerJoinInputError("graph volatile-owner raw event hashes must not be empty")
    event_ids: set[str] = set()
    for index, raw in enumerate(hashes):
        event = _closed(raw, f"graph.volatile_owner_facts.raw_event_hashes[{index}]", {"event_id", "sha256"})
        event_id = _token(event["event_id"], f"graph.volatile_owner_facts.raw_event_hashes[{index}].event_id")
        event_match = _EVENT_ID_RE.fullmatch(event_id)
        if event_match is None or event_match.group("session") != context["session_id"]:
            raise VolatileOwnerJoinInputError("graph volatile-owner event ID is not session-bound")
        _sha256(event["sha256"], f"graph.volatile_owner_facts.raw_event_hashes[{index}].sha256")
        if event_id in event_ids:
            raise VolatileOwnerJoinInputError("graph volatile-owner event IDs are duplicated")
        event_ids.add(event_id)
    object_bindings = _sequence(section.get("object_identity_bindings"), "graph.volatile_owner_facts.object_identity_bindings")
    bound_fact_ids: set[str] = set()
    binding_statuses: dict[str, dict[str, str]] = {}
    for index, raw in enumerate(object_bindings):
        label = f"graph.volatile_owner_facts.object_identity_bindings[{index}]"
        binding = _mapping(raw, label)
        status = binding.get("status")
        required = {"fact_id", "status", "object_token"} if status == "PRESENT" else {"fact_id", "status", "hidden_owner_token"}
        binding = _closed(binding, label, required)
        fact_id = _token(binding["fact_id"], f"{label}.fact_id")
        if status not in {"PRESENT", "UNKNOWN"}:
            raise VolatileOwnerJoinInputError(f"{label}.status is unsupported")
        token_field = "object_token" if status == "PRESENT" else "hidden_owner_token"
        capture_token = _token(binding[token_field], f"{label}.{token_field}")
        token_match = (_OBJECT_TOKEN_RE if status == "PRESENT" else _HIDDEN_OBJECT_TOKEN_RE).fullmatch(capture_token)
        if token_match is None or token_match.group("session") != context["session_id"]:
            raise VolatileOwnerJoinInputError(f"{label}.{token_field} is not a canonical same-session capture token")
        if fact_id in bound_fact_ids:
            raise VolatileOwnerJoinInputError("graph object identity bindings contain duplicate facts")
        bound_fact_ids.add(fact_id)
        binding_statuses[fact_id] = {"status": str(status), "object_token": capture_token}
    span_source = span.get("source")
    if not isinstance(span_source, Mapping) or span_source.get("sha256") != source.get("sha256"):
        blockers.append("graph_source_identity_mismatch")
    return binding_statuses


def _graph_rows(section: Mapping[str, Any], blockers: list[str], session_id: str) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    raw_rows = section.get("closed_residual_rows")
    raw_facts = section.get("owner_facts")
    if raw_rows is None or raw_facts is None:
        blockers.append("volatile_owner_extension_absent")
        return [], []
    rows: list[dict[str, Any]] = []
    row_ids: set[str] = set()
    for index, raw in enumerate(_sequence(raw_rows, "graph.closed_residual_rows")):
        label = f"graph.closed_residual_rows[{index}]"
        row = _closed(raw, label, {"row_id", "ordinal", "kind", "owner_fact_id", "required_operand_roles"})
        row_id = _token(row["row_id"], f"{label}.row_id")
        ordinal = _uint(row["ordinal"], f"{label}.ordinal")
        kind = _uint(row["kind"], f"{label}.kind")
        owner_fact_id = _token(row["owner_fact_id"], f"{label}.owner_fact_id")
        if _ROW_ID_RE.fullmatch(row_id) is None or _FACT_ID_RE.fullmatch(owner_fact_id) is None:
            raise VolatileOwnerJoinInputError(f"{label} has a noncanonical row/fact token")
        roles = [_token(item, f"{label}.required_operand_roles[{role_index}]") for role_index, item in enumerate(_sequence(row["required_operand_roles"], f"{label}.required_operand_roles"))]
        if len(roles) != len(set(roles)):
            raise VolatileOwnerJoinInputError(f"{label}.required_operand_roles contains duplicates")
        if row_id in row_ids:
            raise VolatileOwnerJoinInputError("graph closed residual rows contain duplicate identities")
        row_ids.add(row_id)
        rows.append({"row_id": row_id, "ordinal": ordinal, "kind": kind, "owner_fact_id": owner_fact_id, "required_operand_roles": roles})
    facts: list[dict[str, Any]] = []
    fact_ids: set[str] = set()
    for index, raw in enumerate(_sequence(raw_facts, "graph.owner_facts")):
        label = f"graph.owner_facts[{index}]"
        fact = _closed(
            raw,
            label,
            {"fact_id", "row_id", "role", "pcode_id", "ig_node_id", "vreg", "final_color", "physical_register", "def_id", "use_ids", "classification"},
            {"interference_neighbors", "missing_edge"},
        )
        parsed = {key: _token(fact[key], f"{label}.{key}") for key in ("fact_id", "row_id", "role", "pcode_id", "ig_node_id", "physical_register", "classification")}
        if parsed["fact_id"] in fact_ids:
            raise VolatileOwnerJoinInputError("graph.owner_facts contains duplicate fact IDs")
        fact_ids.add(parsed["fact_id"])
        if _FACT_ID_RE.fullmatch(parsed["fact_id"]) is None or _ROW_ID_RE.fullmatch(parsed["row_id"]) is None:
            raise VolatileOwnerJoinInputError(f"{label} has a noncanonical fact/row token")
        pcode_match = _PCODE_ID_RE.fullmatch(parsed["pcode_id"])
        ig_match = _IG_ID_RE.fullmatch(parsed["ig_node_id"])
        if pcode_match is None or ig_match is None or pcode_match.group("session") != session_id or ig_match.group("session") != session_id:
            raise VolatileOwnerJoinInputError(f"{label} PCode/IG tokens are not session-bound")
        vreg = fact["vreg"]
        def_id = fact["def_id"]
        parsed["vreg"] = None if vreg is None else _token(vreg, f"{label}.vreg")
        parsed["def_id"] = None if def_id is None else _token(def_id, f"{label}.def_id")
        if parsed["def_id"] is not None and _DEF_ID_RE.fullmatch(parsed["def_id"]) is None:
            raise VolatileOwnerJoinInputError(f"{label}.def_id is noncanonical")
        if parsed["vreg"] is not None and _VREG_RE.fullmatch(parsed["vreg"]) is None:
            raise VolatileOwnerJoinInputError(f"{label}.vreg is invalid")
        if _REGISTER_RE.fullmatch(parsed["physical_register"]) is None:
            raise VolatileOwnerJoinInputError(f"{label} has an invalid register")
        final_color = _uint(fact["final_color"], f"{label}.final_color", maximum=31)
        if int(parsed["physical_register"][1:]) != final_color or (parsed["vreg"] is not None and parsed["physical_register"][0].lower() != parsed["vreg"][0].lower()):
            blockers.append(f"fact_{parsed['fact_id']}_color_physical_mismatch")
        use_ids = [_token(item, f"{label}.use_ids[{use_index}]") for use_index, item in enumerate(_sequence(fact["use_ids"], f"{label}.use_ids"))]
        if any(_USE_ID_RE.fullmatch(item) is None for item in use_ids):
            raise VolatileOwnerJoinInputError(f"{label}.use_ids contains a noncanonical token")
        if len(use_ids) != len(set(use_ids)):
            raise VolatileOwnerJoinInputError(f"{label}.use_ids must be duplicate-free")
        neighbors: list[dict[str, Any]] | None = None
        if "interference_neighbors" in fact:
            neighbors = []
            seen_neighbor_ids: set[str] = set()
            for neighbor_index, raw_neighbor in enumerate(_sequence(fact["interference_neighbors"], f"{label}.interference_neighbors")):
                neighbor_label = f"{label}.interference_neighbors[{neighbor_index}]"
                neighbor = _closed(raw_neighbor, neighbor_label, {"ig_node_id", "vreg", "final_color", "physical_register"})
                ig_node_id = _token(neighbor["ig_node_id"], f"{neighbor_label}.ig_node_id")
                ig_match = _IG_ID_RE.fullmatch(ig_node_id)
                if ig_match is None or ig_match.group("session") != session_id:
                    raise VolatileOwnerJoinInputError(f"{neighbor_label}.ig_node_id is not session-bound")
                neighbor_vreg = _token(neighbor["vreg"], f"{neighbor_label}.vreg")
                neighbor_register = _token(neighbor["physical_register"], f"{neighbor_label}.physical_register").lower()
                neighbor_color = _uint(neighbor["final_color"], f"{neighbor_label}.final_color", maximum=31)
                if _VREG_RE.fullmatch(neighbor_vreg) is None or _REGISTER_RE.fullmatch(neighbor_register) is None:
                    raise VolatileOwnerJoinInputError(f"{neighbor_label} has an invalid vreg/register")
                if int(neighbor_register[1:]) != neighbor_color or neighbor_register[0] != neighbor_vreg[0].lower():
                    blockers.append(f"fact_{parsed['fact_id']}_interference_neighbor_color_mismatch")
                if ig_node_id in seen_neighbor_ids:
                    raise VolatileOwnerJoinInputError(f"{label}.interference_neighbors contains duplicate IG nodes")
                seen_neighbor_ids.add(ig_node_id)
                neighbors.append({
                    "ig_node_id": ig_node_id,
                    "vreg": neighbor_vreg,
                    "final_color": neighbor_color,
                    "physical_register": neighbor_register,
                })
        missing_edge = None
        if "missing_edge" in fact and fact["missing_edge"] is not None:
            missing_edge = _token(fact["missing_edge"], f"{label}.missing_edge")
        parsed.update({
            "final_color": final_color,
            "use_ids": use_ids,
            "interference_neighbors": neighbors,
            "missing_edge": missing_edge,
        })
        facts.append(parsed)
    return rows, facts


def _formatted(row: Mapping[str, Any], label: str) -> str:
    instruction = _mapping(row.get("instruction"), f"{label}.instruction")
    return _text(instruction.get("formatted"), f"{label}.instruction.formatted", limit=512)


def _registers(row: Mapping[str, Any], label: str) -> list[str]:
    return [match.group(1).lower() for match in _REGISTER_RE.finditer(_formatted(row, label))]


def _register_skeleton(row: Mapping[str, Any], label: str) -> str:
    formatted = _formatted(row, label).strip().lower()
    return _REGISTER_RE.sub("<reg>", formatted)


def build_join(
    focus: Mapping[str, Any],
    span: Mapping[str, Any],
    graph: Mapping[str, Any],
    context_value: Mapping[str, Any],
    expected_context_sha256: str,
) -> dict[str, Any]:
    expected_context = _sha256(expected_context_sha256, "expected context SHA-256")
    if context_value.get("context_sha256") != expected_context:
        raise VolatileOwnerJoinInputError("context SHA-256 differs from caller-supplied trust anchor")
    context = parse_context(context_value)
    blockers: list[str] = []
    _verify_context_digest(context_value, blockers)
    _verify_object_file(context["target_object"], "target_object", blockers)
    _verify_object_file(context["candidate_object"], "candidate_object", blockers)
    _verify_physical_receipt_file(context["physical_relocation_receipt"], context["strict_report_sha256"], blockers)
    target_rows, candidate_rows = _focus_rows(focus, context, blockers)
    source_declarations = _validate_span(span, context, blockers)
    if graph.get("schema") != GRAPH_SCHEMA:
        raise VolatileOwnerJoinInputError(f"ownership graph schema must be {GRAPH_SCHEMA}")
    _self_digest(graph, "failure_graph_sha256", "ownership graph")
    if graph.get("failure_graph_sha256") != context["ownership_failure_graph"]["failure_graph_sha256"]:
        raise VolatileOwnerJoinInputError("ownership graph identity differs from context")
    if graph.get("session_id") != context["session_id"]:
        blockers.append("graph_session_mismatch")
    if graph.get("function") != context["function"]:
        blockers.append("graph_function_mismatch")
    if graph.get("diagnostic_only") is not True or graph.get("authority_advanced") is not False:
        blockers.append("graph_policy_mismatch")
    section = graph.get("volatile_owner_facts")
    if not isinstance(section, Mapping):
        blockers.append("volatile_owner_extension_absent")
        section = {}
        object_binding_statuses: dict[str, dict[str, str]] = {}
    else:
        _closed(
            section,
            "graph.volatile_owner_facts",
            {
                "schema", "status", "authority_advanced", "session_id", "function",
                "source", "compiler", "raw_event_hashes", "object_identity_bindings",
                "closed_residual_rows", "owner_facts", "volatile_owner_facts_sha256",
            },
            {"candidate_object"},
        )
        _self_digest(section, "volatile_owner_facts_sha256", "graph.volatile_owner_facts")
        if section.get("schema") != f"{GRAPH_SCHEMA}/volatile-owner-facts/v1" or section.get("status") != "DIAGNOSTIC_ONLY":
            blockers.append("volatile_owner_section_policy_mismatch")
        if section.get("session_id") != context["session_id"] or section.get("function") != context["function"]:
            blockers.append("volatile_owner_section_context_mismatch")
        if section.get("authority_advanced") is not False:
            blockers.append("volatile_owner_section_authority_not_false")
        object_binding_statuses = _capture_binding(section, context, span, blockers)
    graph_rows, facts = _graph_rows(section, blockers, context["session_id"])
    enriched = (
        span.get("schema") == "mwcc_source_span_bindings/v2"
        or any(fact["interference_neighbors"] is not None or fact["missing_edge"] is not None for fact in facts)
    )
    if isinstance(section, Mapping) and section:
        bound_fact_ids = {
            row.get("fact_id") for row in section.get("object_identity_bindings", [])
            if isinstance(row, Mapping)
        }
        if bound_fact_ids != {fact["fact_id"] for fact in facts}:
            blockers.append("object_identity_bindings_do_not_close_owner_facts")

    focus_indices = set(target_rows) & set(candidate_rows)
    graph_row_ids = {row["row_id"] for row in graph_rows}
    binding_by_id = {row["row_id"]: row for row in context["residual_row_bindings"]}
    graph_indices = {binding_by_id[row_id]["focus_row_index"] for row_id in graph_row_ids if row_id in binding_by_id}
    if graph_row_ids != set(binding_by_id) or graph_indices != focus_indices:
        blockers.append("graph_rows_do_not_exactly_close_focus_residual")
    rows_by_id = {row["row_id"]: row for row in graph_rows}
    expected_pairs = {(row_id, binding["captured_role"]) for row_id, binding in binding_by_id.items()}
    observed: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for fact in facts:
        observed.setdefault((fact["row_id"], fact["role"]), []).append(fact)
    if set(observed) - expected_pairs:
        blockers.append("owner_facts_contain_extra_row_or_role")
    bindings: list[dict[str, Any]] = []
    row_diagnostics: list[dict[str, Any]] = []
    permutation: dict[str, str] = {}
    changed_pairs: set[tuple[str, str]] = set()
    for index in sorted(focus_indices):
        candidate_regs = _registers(candidate_rows[index], f"focus.candidate[{index}]")
        target_regs = _registers(target_rows[index], f"focus.target[{index}]")
        if _register_skeleton(candidate_rows[index], f"focus.candidate[{index}]") != _register_skeleton(target_rows[index], f"focus.target[{index}]"):
            blockers.append(f"row_{index}_instruction_skeleton_differs_beyond_registers")
            continue
        if len(candidate_regs) != len(target_regs):
            blockers.append(f"row_{index}_register_arity_mismatch")
            continue
        changed_pairs.update((candidate, target) for candidate, target in zip(candidate_regs, target_regs) if candidate != target)
    for pair in sorted(expected_pairs):
        choices = observed.get(pair, [])
        if len(choices) != 1:
            blockers.append(f"owner_fact_not_unique:{pair[0]}:{pair[1]}")
            if enriched:
                manifest_binding = binding_by_id.get(pair[0])
                row_diagnostics.append({
                    "row_id": pair[0],
                    "row_ordinal": (manifest_binding["focus_row_index"] if manifest_binding is not None else None),
                    "role": pair[1],
                    "semantic_role": (manifest_binding["semantic_role"] if manifest_binding is not None else None),
                    "candidate_operand_index": (manifest_binding["candidate_operand_index"] if manifest_binding is not None else None),
                    "pcode_id": None,
                    "ig_node_id": None,
                    "vreg": None,
                    "final_color": None,
                    "candidate_physical_register": None,
                    "target_physical_register": None,
                    "def_id": None,
                    "use_ids": [],
                    "interference_neighbors": [],
                    "missing_edge": "owner_fact_not_unique",
                    "source_span": None,
                })
            continue
        fact = choices[0]
        row = rows_by_id.get(fact["row_id"])
        manifest_binding = binding_by_id.get(fact["row_id"])
        if row is None or manifest_binding is None or manifest_binding["focus_row_index"] not in focus_indices:
            blockers.append(f"owner_fact_row_unbound:{fact['fact_id']}")
            continue
        object_binding = object_binding_statuses.get(fact["fact_id"])
        source_span = (
            source_declarations.get(object_binding["object_token"])
            if object_binding is not None and object_binding.get("status") == "PRESENT"
            else None
        )
        index = manifest_binding["focus_row_index"]
        candidate_regs = _registers(candidate_rows[index], f"focus.candidate[{index}]")
        target_regs = _registers(target_rows[index], f"focus.target[{index}]")
        operand_index = manifest_binding["candidate_operand_index"]
        target_register = (
            target_regs[operand_index]
            if operand_index is not None and operand_index < len(target_regs)
            else None
        )
        derived_missing_edge = fact["missing_edge"]
        if fact["classification"] != "UNIQUE":
            derived_missing_edge = derived_missing_edge or "owner_chain_not_unique"
        elif fact["vreg"] is None:
            derived_missing_edge = derived_missing_edge or "object_to_vreg"
        elif fact["def_id"] is None or not fact["use_ids"]:
            derived_missing_edge = derived_missing_edge or "pcode_def_use"
        elif object_binding is None or object_binding.get("status") != "PRESENT":
            derived_missing_edge = derived_missing_edge or "source_object_identity"
        elif enriched and source_span is None:
            derived_missing_edge = derived_missing_edge or "source_object_to_declaration_span"
        elif enriched and fact["interference_neighbors"] is None:
            derived_missing_edge = derived_missing_edge or "ig_interference_neighbors"
        if enriched:
            row_diagnostics.append({
                "row_id": fact["row_id"],
                "row_ordinal": index,
                "role": fact["role"],
                "semantic_role": manifest_binding["semantic_role"],
                "candidate_operand_index": operand_index,
                "pcode_id": fact["pcode_id"],
                "ig_node_id": fact["ig_node_id"],
                "vreg": fact["vreg"],
                "final_color": fact["final_color"],
                "candidate_physical_register": fact["physical_register"].lower(),
                "target_physical_register": target_register,
                "def_id": fact["def_id"],
                "use_ids": fact["use_ids"],
                "interference_neighbors": fact["interference_neighbors"] or [],
                "missing_edge": derived_missing_edge,
                "source_span": source_span,
            })
        if row["owner_fact_id"] != fact["fact_id"] or fact["classification"] != "UNIQUE" or fact["vreg"] is None or fact["def_id"] is None or not fact["use_ids"]:
            blockers.append(f"owner_fact_chain_not_unique:{fact['fact_id']}")
            continue
        if object_binding is None or object_binding.get("status") != "PRESENT":
            blockers.append(f"owner_fact_object_identity_not_present:{fact['fact_id']}")
            continue
        if enriched and source_span is None:
            blockers.append(f"owner_fact_source_span_missing:{fact['fact_id']}")
            continue
        if enriched and fact["interference_neighbors"] is None:
            blockers.append(f"owner_fact_interference_evidence_missing:{fact['fact_id']}")
            continue
        if enriched and derived_missing_edge is not None:
            blockers.append(f"owner_fact_has_missing_edge:{fact['fact_id']}:{derived_missing_edge}")
            continue
        if fact["role"] != manifest_binding["captured_role"] or fact["role"] not in row["required_operand_roles"]:
            blockers.append(f"owner_fact_role_not_capture_bound:{fact['fact_id']}")
            continue
        if operand_index is None:
            blockers.append(f"candidate_operand_index_missing:{fact['fact_id']}")
            continue
        if operand_index >= len(candidate_regs) or operand_index >= len(target_regs):
            blockers.append(f"candidate_operand_index_out_of_range:{fact['fact_id']}")
            continue
        if candidate_regs[operand_index] != fact["physical_register"].lower():
            blockers.append(f"candidate_operand_register_mismatch:{fact['fact_id']}")
            continue
        target_register = target_regs[operand_index]
        previous = permutation.get(fact["physical_register"].lower())
        if previous is not None and previous != target_register:
            blockers.append(f"physical_register_has_conflicting_targets:{fact['physical_register'].lower()}")
            continue
        permutation[fact["physical_register"].lower()] = target_register
        binding_result = {
            "row_id": fact["row_id"], "row_ordinal": index, "role": fact["role"],
            "semantic_role": manifest_binding["semantic_role"],
            "candidate_operand_index": operand_index,
            "pcode_id": fact["pcode_id"], "ig_node_id": fact["ig_node_id"],
            "vreg": fact["vreg"], "final_color": fact["final_color"],
            "candidate_physical_register": fact["physical_register"].lower(),
            "target_physical_register": target_register, "def_id": fact["def_id"],
            "use_ids": fact["use_ids"], "classification": fact["classification"],
        }
        if enriched:
            binding_result.update({
                "interference_neighbors": fact["interference_neighbors"],
                "missing_edge": None,
                "source_span": source_span,
            })
        bindings.append(binding_result)
    if not changed_pairs:
        blockers.append("focus_contains_no_register_permutation")
    if not changed_pairs <= set(permutation.items()):
        blockers.append("register_permutation_not_fully_owner_bound")
    changed = {source: target for source, target in permutation.items() if source != target}
    if changed and set(changed) != set(changed.values()):
        blockers.append("register_mapping_is_not_a_closed_permutation")
    eligible_classes = [
        hypothesis["source_class"]
        for hypothesis in context["source_class_hypotheses"]
        if hypothesis["source_class"] in context["source_class_allowlist"]
        and set(hypothesis["row_ids"]) == graph_row_ids
    ]
    if len(eligible_classes) != 1:
        blockers.append("natural_source_class_not_unique")

    status = "PROVEN" if not blockers else "UNKNOWN"
    result: dict[str, Any] = {
        "schema": ENRICHED_SCHEMA if enriched else SCHEMA,
        "status": status,
        "function": context["function"],
        "session_id": context["session_id"],
        "bindings": sorted(bindings, key=lambda item: (item["row_ordinal"], item["role"], item["pcode_id"])),
        "register_permutation": {
            "complete": status == "PROVEN",
            "mapping": [{"candidate": source, "target": target} for source, target in sorted(permutation.items())],
            "changed_mapping": [{"candidate": source, "target": target} for source, target in sorted(changed.items())],
        },
        "ranked_source_classes": ([{"rank": 1, "source_class": eligible_classes[0]}] if status == "PROVEN" else []),
        "closed_residual_rows": sorted(graph_indices),
        "evidence_binding": {
            "context_sha256": context["context_sha256"],
            "focus_artifact_sha256": context["focus"]["artifact_sha256"],
            "strict_report_sha256": context["strict_report_sha256"],
            "data_report_sha256": context["data_report_sha256"],
            "source_span_manifest_sha256": context["source_span_manifest"]["manifest_sha256"],
            "ownership_failure_graph_sha256": context["ownership_failure_graph"]["failure_graph_sha256"],
            "physical_relocation_receipt": (
                {key: context["physical_relocation_receipt"][key] for key in ("file_sha256", "receipt_payload_sha256")}
                if context["physical_relocation_receipt"] is not None else None
            ),
            "target_object": {key: context["target_object"][key] for key in ("sha256", "size")},
            "candidate_object": {key: context["candidate_object"][key] for key in ("sha256", "size")},
        },
        "blockers": sorted(set(blockers)),
        "diagnostic_only": True,
        "source_text_emitted": False,
        "source_patch_emitted": False,
        "retention_authorized": False,
        "authority_advanced": False,
    }
    if enriched:
        result["row_diagnostics"] = sorted(
            row_diagnostics,
            key=lambda item: (
                item["row_ordinal"] if item["row_ordinal"] is not None else 1 << 31,
                item["role"],
                item["pcode_id"] or "",
            ),
        )
    result["join_sha256"] = canonical_sha256(result)
    return result


def build_from_paths(
    context_path: Path,
    focus_path: Path,
    span_path: Path,
    graph_path: Path,
    expected_context_sha256: str,
) -> dict[str, Any]:
    try:
        context_raw = json.loads(context_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise VolatileOwnerJoinInputError(f"cannot parse context {context_path}: {exc}") from exc
    context_mapping = _mapping(context_raw, "context")
    expected_context = _sha256(expected_context_sha256, "expected context SHA-256")
    if context_mapping.get("context_sha256") != expected_context:
        raise VolatileOwnerJoinInputError("context SHA-256 differs from caller-supplied trust anchor")
    context = parse_context(context_mapping)
    focus = _json(focus_path, context["focus"]["file_sha256"], "focus artifact")
    span = _json(span_path, context["source_span_manifest"]["file_sha256"], "source-span manifest")
    graph = _json(graph_path, context["ownership_failure_graph"]["file_sha256"], "ownership graph")
    return build_join(focus, span, graph, context_raw, expected_context)


def schema_path_for_document(document: Mapping[str, Any]) -> Path:
    """Resolve only the two published output contracts; never guess a version."""

    schema = document.get("schema")
    if schema == SCHEMA:
        return Path(__file__).with_name("VOLATILE_OWNER_CAUSAL_JOIN_V1.schema.json")
    if schema == ENRICHED_SCHEMA:
        return Path(__file__).with_name("VOLATILE_OWNER_CAUSAL_JOIN_V2.schema.json")
    raise VolatileOwnerJoinInputError("volatile-owner join output schema is unsupported")


def _atomic_write(path: Path, text: str) -> None:
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
    parser.add_argument("--context-sha256", required=True, help="externally supplied canonical context SHA-256")
    parser.add_argument("--focus", type=Path, required=True)
    parser.add_argument("--source-spans", type=Path, required=True)
    parser.add_argument("--ownership-graph", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = build_from_paths(args.context, args.focus, args.source_spans, args.ownership_graph, args.context_sha256)
    except VolatileOwnerJoinInputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    rendered = json.dumps(result, ensure_ascii=True, sort_keys=True, indent=2) + "\n"
    if args.output:
        _atomic_write(args.output, rendered)
    else:
        sys.stdout.write(rendered)
    return 0 if result["status"] == "PROVEN" else 1


if __name__ == "__main__":
    raise SystemExit(main())
