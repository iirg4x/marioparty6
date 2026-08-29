#!/usr/bin/env python3
"""Fail-closed replay of archived MWCC hidden arithmetic ownership evidence.

This tool never launches a compiler.  It consumes an immutable partial-evidence
package produced by ``capsule_same_session_capture.py`` and closes only the
directly authenticated PCode Object/hidden-owner -> IG -> final-color ->
machine-operand edges.  ``operand_index`` is deliberately ignored: it is an
MWCC operand-table index, not a virtual register identifier.
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


CONTEXT_SCHEMA = "hidden_arithmetic_owner_join_context/v1"
OUTPUT_SCHEMA = "hidden_arithmetic_owner_join/v1"
PARTIAL_SCHEMA = "mwcc_capsule_same_session_partial_evidence/v1"
EVENT_SCHEMA = "mwcc_capsule_same_session_capture_event/v1"
SHA_RE = re.compile(r"^[0-9a-f]{64}$")
REGISTER_RE = re.compile(r"^[rf](?:[0-9]|[12][0-9]|3[01])$")
SESSION_RE = re.compile(r"^session-[A-Za-z0-9]+$")


class JoinInputError(ValueError):
    """Evidence is malformed, stale, ambiguous, or outside the contract."""


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode()


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_bytes(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def implementation_binding() -> dict[str, Any]:
    path = Path(__file__).resolve()
    return {"path": str(path), "size": path.stat().st_size, "sha256": file_sha256(path)}


def mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise JoinInputError(f"{label} must be an object")
    return value


def sequence(value: Any, label: str) -> list[Any]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)):
        raise JoinInputError(f"{label} must be an array")
    return list(value)


def text(value: Any, label: str, limit: int = 4096) -> str:
    if not isinstance(value, str) or not value or len(value) > limit:
        raise JoinInputError(f"{label} must be nonempty text")
    return value


def sha256(value: Any, label: str) -> str:
    result = text(value, label, 64)
    if SHA_RE.fullmatch(result) is None:
        raise JoinInputError(f"{label} must be lowercase SHA-256")
    return result


def uint(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise JoinInputError(f"{label} must be an unsigned integer")
    return value


def decimal_uint(value: Any, label: str) -> int:
    if isinstance(value, str) and re.fullmatch(r"[0-9]+", value):
        return int(value)
    return uint(value, label)


def boolean(value: Any, label: str) -> bool:
    if not isinstance(value, bool):
        raise JoinInputError(f"{label} must be Boolean")
    return value


def bound_file(value: Any, label: str, *, payload_field: str | None = None) -> dict[str, Any]:
    row = mapping(value, label)
    required = {"path", "size", "sha256"}
    if payload_field is not None:
        required.add(payload_field)
    if set(row) != required:
        raise JoinInputError(f"{label} fields are not canonical")
    path = Path(text(row["path"], f"{label}.path"))
    size = uint(row["size"], f"{label}.size")
    digest = sha256(row["sha256"], f"{label}.sha256")
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        raise JoinInputError(f"{label} must be an absolute regular file")
    if path.stat().st_size != size or file_sha256(path) != digest:
        raise JoinInputError(f"{label} identity mismatch")
    result: dict[str, Any] = {"path": path, "size": size, "sha256": digest}
    if payload_field is not None:
        result[payload_field] = sha256(row[payload_field], f"{label}.{payload_field}")
    return result


def residual_projection(value: Any, label: str) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    seen: set[int] = set()
    for ordinal, raw in enumerate(sequence(value, label)):
        row_label = f"{label}[{ordinal}]"
        row = mapping(raw, row_label)
        if set(row) != {"index", "target_diff_kind", "target_formatted", "candidate_diff_kind", "candidate_formatted"}:
            raise JoinInputError(f"{row_label} fields are not canonical")
        index = uint(row["index"], f"{row_label}.index")
        if index in seen:
            raise JoinInputError(f"{label} contains duplicate row {index}")
        seen.add(index)
        parsed: dict[str, Any] = {"index": index}
        for side in ("target", "candidate"):
            kind = row[f"{side}_diff_kind"]
            formatted = row[f"{side}_formatted"]
            if not isinstance(kind, str) or not kind.startswith("DIFF_"):
                raise JoinInputError(f"{row_label}.{side}_diff_kind is invalid")
            if formatted is not None and (not isinstance(formatted, str) or not formatted):
                raise JoinInputError(f"{row_label}.{side}_formatted is invalid")
            parsed[f"{side}_diff_kind"] = kind
            parsed[f"{side}_formatted"] = formatted
        rows.append(parsed)
    if not rows:
        raise JoinInputError(f"{label} must not be empty")
    return rows


def read_json(path: Path, label: str) -> Mapping[str, Any]:
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        raise JoinInputError(f"{label} must be an absolute regular file")
    try:
        return mapping(json.loads(path.read_text(encoding="utf-8")), label)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise JoinInputError(f"cannot parse {label}: {exc}") from exc


def read_jsonl(path: Path, expected_sha: str, label: str) -> list[Mapping[str, Any]]:
    if path.is_symlink() or not path.is_file() or file_sha256(path) != expected_sha:
        raise JoinInputError(f"{label} identity mismatch")
    result: list[Mapping[str, Any]] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if not line.strip():
            continue
        try:
            result.append(mapping(json.loads(line), f"{label}:{line_number}"))
        except json.JSONDecodeError as exc:
            raise JoinInputError(f"cannot parse {label}:{line_number}: {exc}") from exc
    return result


def verify_self_hash(value: Mapping[str, Any], field: str, label: str) -> None:
    expected = sha256(value.get(field), f"{label}.{field}")
    unsigned = dict(value)
    unsigned.pop(field, None)
    if canonical_sha256(unsigned) != expected:
        raise JoinInputError(f"{label} self-hash mismatch")


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    required = {
        "schema", "partial_evidence_path", "partial_evidence_sha256", "function",
        "session_id", "source_sha256", "compiler_sha256", "trace_candidate_object_sha256",
        "failure_graph_sha256", "machine_sites", "production_row_groups", "source_owners", "source_class_allowlist",
        "rejected_controls", "target_object", "production_candidate_object", "focus_artifact",
        "strict_report_sha256", "data_report_sha256", "physical_relocation_receipt",
        "physical_relocation_count", "residual_rows", "context_sha256",
    }
    if set(value) != required:
        raise JoinInputError("context fields are not canonical")
    if value["schema"] != CONTEXT_SCHEMA:
        raise JoinInputError(f"context.schema must be {CONTEXT_SCHEMA}")
    verify_self_hash(value, "context_sha256", "context")
    session = text(value["session_id"], "context.session_id", 96)
    if SESSION_RE.fullmatch(session) is None:
        raise JoinInputError("context.session_id is invalid")
    sites = [uint(item, f"context.machine_sites[{i}]") for i, item in enumerate(sequence(value["machine_sites"], "context.machine_sites"))]
    row_groups: list[dict[str, Any]] = []
    for index, raw in enumerate(sequence(value["production_row_groups"], "context.production_row_groups")):
        label = f"context.production_row_groups[{index}]"
        row = mapping(raw, label)
        if set(row) != {"trace_machine_site", "production_fmuls_index", "residual_indices"}:
            raise JoinInputError(f"{label} fields are not canonical")
        residual_indices = [uint(item, f"{label}.residual_indices") for item in sequence(row["residual_indices"], f"{label}.residual_indices")]
        if not residual_indices or len(residual_indices) != len(set(residual_indices)):
            raise JoinInputError(f"{label}.residual_indices must be nonempty and unique")
        row_groups.append({
            "trace_machine_site": uint(row["trace_machine_site"], f"{label}.trace_machine_site"),
            "production_fmuls_index": uint(row["production_fmuls_index"], f"{label}.production_fmuls_index"),
            "residual_indices": residual_indices,
        })
    owners: list[dict[str, Any]] = []
    for index, raw in enumerate(sequence(value["source_owners"], "context.source_owners")):
        row = mapping(raw, f"context.source_owners[{index}]")
        if set(row) != {"name", "object_token", "byte_start", "byte_end", "span_sha256"}:
            raise JoinInputError("source owner binding fields are not canonical")
        owners.append({
            "name": text(row["name"], "source owner name", 128),
            "object_token": text(row["object_token"], "source owner object token", 192),
            "byte_start": uint(row["byte_start"], "source owner byte_start"),
            "byte_end": uint(row["byte_end"], "source owner byte_end"),
            "span_sha256": sha256(row["span_sha256"], "source owner span_sha256"),
        })
    if len(sites) < 2 or len(sites) != len(set(sites)) or not owners or len({row["name"] for row in owners}) != len(owners) or len({row["object_token"] for row in owners}) != len(owners):
        raise JoinInputError("machine sites and source owners must be nonempty and unique")
    if len(row_groups) != len(sites) or {row["trace_machine_site"] for row in row_groups} != set(sites):
        raise JoinInputError("production row groups must cover each trace machine site exactly once")
    class_allowlist = [text(item, "context.source_class_allowlist entry", 192) for item in sequence(value["source_class_allowlist"], "context.source_class_allowlist")]
    supported_classes = {
        "named_owner_chronology_x_reloaded_value_boundary",
        "named_owner_chronology_x_direct_expression_boundary",
    }
    if not class_allowlist or len(class_allowlist) != len(set(class_allowlist)) or not set(class_allowlist) <= supported_classes:
        raise JoinInputError("source class allowlist is empty, duplicate, or unsupported")
    controls: list[dict[str, str]] = []
    control_ids: set[str] = set()
    control_classes: set[str] = set()
    for index, raw in enumerate(sequence(value["rejected_controls"], "context.rejected_controls")):
        row = mapping(raw, f"context.rejected_controls[{index}]")
        if set(row) != {"control_id", "source_sha256", "object_sha256", "source_class", "boundary_kind", "outcome"}:
            raise JoinInputError("rejected control fields are not canonical")
        control_id = text(row["control_id"], "control_id", 128)
        control_class = text(row["source_class"], "control.source_class", 192)
        if control_id in control_ids or control_class in control_classes:
            raise JoinInputError("rejected controls are duplicate")
        control_ids.add(control_id)
        control_classes.add(control_class)
        controls.append({
            "control_id": control_id,
            "source_sha256": sha256(row["source_sha256"], "control.source_sha256"),
            "object_sha256": sha256(row["object_sha256"], "control.object_sha256"),
            "source_class": control_class,
            "boundary_kind": text(row["boundary_kind"], "control.boundary_kind", 64),
            "outcome": text(row["outcome"], "control.outcome", 512),
        })
        if controls[-1]["boundary_kind"] not in {"DIRECT_EXPRESSION_ARITHMETIC_BOUNDARY", "RELOADED_VALUE_BOUNDARY", "SPILL_RELOAD_BOUNDARY", "OTHER_AUTHENTICATED"}:
            raise JoinInputError("rejected control boundary kind is unsupported")
    fmuls_rows = [row["production_fmuls_index"] for row in row_groups]
    if len(fmuls_rows) != len(set(fmuls_rows)) or any(row["production_fmuls_index"] not in row["residual_indices"] for row in row_groups):
        raise JoinInputError("production fmuls mappings must be unique members of their residual groups")
    return {
        "partial_path": Path(text(value["partial_evidence_path"], "context.partial_evidence_path")),
        "partial_sha": sha256(value["partial_evidence_sha256"], "context.partial_evidence_sha256"),
        "function": text(value["function"], "context.function", 192),
        "session": session,
        "source_sha": sha256(value["source_sha256"], "context.source_sha256"),
        "compiler_sha": sha256(value["compiler_sha256"], "context.compiler_sha256"),
        "trace_object_sha": sha256(value["trace_candidate_object_sha256"], "context.trace_candidate_object_sha256"),
        "graph_sha": sha256(value["failure_graph_sha256"], "context.failure_graph_sha256"),
        "target_object": bound_file(value["target_object"], "context.target_object"),
        "production_object": bound_file(value["production_candidate_object"], "context.production_candidate_object"),
        "focus": bound_file(value["focus_artifact"], "context.focus_artifact", payload_field="artifact_sha256"),
        "strict_report_sha": sha256(value["strict_report_sha256"], "context.strict_report_sha256"),
        "data_report_sha": sha256(value["data_report_sha256"], "context.data_report_sha256"),
        "physical_receipt": bound_file(value["physical_relocation_receipt"], "context.physical_relocation_receipt", payload_field="receipt_sha256"),
        "physical_relocation_count": uint(value["physical_relocation_count"], "context.physical_relocation_count"),
        "residual_rows": residual_projection(value["residual_rows"], "context.residual_rows"),
        "sites": sites,
        "row_groups": row_groups,
        "owners": owners,
        "class_allowlist": class_allowlist,
        "controls": controls,
        "context_sha": value["context_sha256"],
    }


def artifact_descriptor(partial: Mapping[str, Any], key: str) -> dict[str, Any]:
    row = mapping(mapping(partial.get("artifacts"), "partial.artifacts").get(key), f"partial.artifacts.{key}")
    if set(row) != {"path", "sha256", "size"}:
        raise JoinInputError(f"partial.artifacts.{key} fields are not canonical")
    path = Path(text(row["path"], f"partial.artifacts.{key}.path"))
    size = uint(row["size"], f"partial.artifacts.{key}.size")
    digest = sha256(row["sha256"], f"partial.artifacts.{key}.sha256")
    if not path.is_absolute() or path.is_symlink() or not path.is_file():
        raise JoinInputError(f"partial artifact {key} is not a regular file")
    if path.stat().st_size != size or file_sha256(path) != digest:
        raise JoinInputError(f"partial artifact {key} identity mismatch")
    return {"path": path, "size": size, "sha256": digest}


def validate_event(event: Mapping[str, Any], session: str, function: str, label: str) -> None:
    if event.get("schema") != EVENT_SCHEMA or event.get("session_id") != session or event.get("function") != function:
        raise JoinInputError(f"{label} session/function provenance mismatch")


def verify_source_owners(source_path: Path, source_sha: str, session: str, owners: Sequence[Mapping[str, Any]]) -> dict[str, dict[str, Any]]:
    if not source_path.is_absolute() or source_path.is_symlink() or not source_path.is_file() or file_sha256(source_path) != source_sha:
        raise JoinInputError("source file identity mismatch")
    payload = source_path.read_bytes()
    result: dict[str, dict[str, Any]] = {}
    for row in owners:
        start = int(row["byte_start"])
        end = int(row["byte_end"])
        token = str(row["object_token"])
        if session not in token or start >= end or end > len(payload):
            raise JoinInputError("source owner span/token is invalid")
        span = payload[start:end]
        if hashlib.sha256(span).hexdigest() != row["span_sha256"]:
            raise JoinInputError("source owner span SHA-256 mismatch")
        try:
            decoded = span.decode("ascii")
        except UnicodeDecodeError as exc:
            raise JoinInputError("source owner span is not ASCII") from exc
        if re.search(rf"\b{re.escape(str(row['name']))}\b", decoded) is None:
            raise JoinInputError("source owner name is absent from sealed span")
        result[token] = {**dict(row), "text": decoded}
    return result


def owner_inventory(envelope: Mapping[str, Any], session: str) -> dict[str, str]:
    inventory = mapping(envelope.get("inventory"), "envelope.inventory")
    result: dict[str, str] = {}
    for group in ("arguments", "locals"):
        for index, raw in enumerate(sequence(inventory.get(group, []), f"inventory.{group}")):
            row = mapping(raw, f"inventory.{group}[{index}]")
            token = text(row.get("token"), "inventory token", 192)
            if session not in token or token in result:
                raise JoinInputError("inventory token session drift or duplicate")
            result[token] = text(row.get("name"), "inventory owner name", 128)
    return result


def physical_for(bank: str, color: int) -> str:
    if bank not in {"GPR", "FPR"} or color > 31:
        raise JoinInputError("PCode bank/color is invalid")
    return f"{'r' if bank == 'GPR' else 'f'}{color}"


def decode_fmuls(ppc_bytes: Any, label: str) -> dict[str, str]:
    raw = text(ppc_bytes, f"{label}.ppc_bytes", 8)
    if re.fullmatch(r"[0-9a-f]{8}", raw) is None:
        raise JoinInputError(f"{label} PPC bytes are not canonical")
    word = int(raw, 16)
    if word >> 26 != 59 or (word >> 1) & 31 != 25 or word & 1:
        raise JoinInputError(f"{label} PPC bytes do not decode as fmuls")
    return {
        "destination": f"f{(word >> 21) & 31}",
        "source_a": f"f{(word >> 16) & 31}",
        "source_b": f"f{(word >> 6) & 31}",
    }


def decode_fpr_destination(machine: Mapping[str, Any], label: str) -> str:
    mnemonic = text(machine.get("mnemonic"), f"{label}.mnemonic", 32)
    raw = text(machine.get("ppc_bytes"), f"{label}.ppc_bytes", 8)
    if re.fullmatch(r"[0-9a-f]{8}", raw) is None:
        raise JoinInputError(f"{label} PPC bytes are not canonical")
    word = int(raw, 16)
    if uint(machine.get("ppc_word"), f"{label}.ppc_word") != word:
        raise JoinInputError(f"{label} ppc_word conflicts with ppc_bytes")
    if mnemonic == "fmuls":
        return decode_fmuls(raw, label)["destination"]
    if mnemonic == "lfs" and word >> 26 == 48:
        return f"f{(word >> 21) & 31}"
    raise JoinInputError(f"{label} producer instruction is not a supported FPR definition")


def event_process(event: Mapping[str, Any], process_id: int, label: str) -> None:
    if uint(event.get("process_id"), f"{label}.process_id") != process_id:
        raise JoinInputError(f"{label} process provenance mismatch")


def diff_projection(channel: Mapping[str, Any], label: str) -> list[dict[str, Any]]:
    sides: dict[str, dict[int, Mapping[str, Any]]] = {}
    for side in ("target", "candidate"):
        side_row = mapping(channel.get(side), f"{label}.{side}")
        rows: dict[int, Mapping[str, Any]] = {}
        for ordinal, raw in enumerate(sequence(side_row.get("rows"), f"{label}.{side}.rows")):
            row = mapping(raw, f"{label}.{side}.rows[{ordinal}]")
            if row.get("diff_kind") is None:
                continue
            index = uint(row.get("index"), f"{label}.{side}.rows[{ordinal}].index")
            if index in rows:
                raise JoinInputError(f"{label}.{side} has duplicate residual row {index}")
            rows[index] = row
        sides[side] = rows
    if set(sides["target"]) != set(sides["candidate"]):
        raise JoinInputError(f"{label} target/candidate residual sets differ")
    result: list[dict[str, Any]] = []
    for index in sorted(sides["target"]):
        item: dict[str, Any] = {"index": index}
        for side in ("target", "candidate"):
            row = sides[side][index]
            instruction = row.get("instruction")
            formatted = None
            if instruction is not None:
                formatted = text(mapping(instruction, f"{label}.{side}[{index}].instruction").get("formatted"), f"{label}.{side}[{index}].formatted")
            item[f"{side}_diff_kind"] = text(row.get("diff_kind"), f"{label}.{side}[{index}].diff_kind", 64)
            item[f"{side}_formatted"] = formatted
        result.append(item)
    return result


def verify_production_evidence(context: Mapping[str, Any]) -> dict[str, Any]:
    focus = read_json(context["focus"]["path"], "focus artifact")
    if focus.get("schema") != "focus_symbol_report/v1":
        raise JoinInputError("focus artifact schema mismatch")
    if focus.get("artifact_sha256") != context["focus"]["artifact_sha256"]:
        raise JoinInputError("focus payload binding mismatch")
    verify_self_hash(focus, "artifact_sha256", "focus artifact")
    if focus.get("function") != context["function"] or focus.get("authority_advanced") is not False:
        raise JoinInputError("focus function/authority mismatch")
    binding = mapping(focus.get("input_binding"), "focus.input_binding")
    for key, expected in (("strict_report", context["strict_report_sha"]), ("data_report", context["data_report_sha"])):
        report = mapping(binding.get(key), f"focus.input_binding.{key}")
        path = Path(text(report.get("path"), f"focus.input_binding.{key}.path"))
        if report.get("sha256") != expected or not path.is_absolute() or path.is_symlink() or not path.is_file() or file_sha256(path) != expected or uint(report.get("size_bytes"), f"focus.input_binding.{key}.size_bytes") != path.stat().st_size:
            raise JoinInputError(f"focus {key} identity mismatch")
    channels = mapping(focus.get("channels"), "focus.channels")
    projections: list[list[dict[str, Any]]] = []
    for name in ("strict", "data"):
        channel = mapping(channels.get(name), f"focus.channels.{name}")
        metric = mapping(channel.get("metric"), f"focus.channels.{name}.metric")
        if uint(metric.get("target_size"), f"focus.{name}.target_size") != uint(metric.get("candidate_size"), f"focus.{name}.candidate_size"):
            raise JoinInputError(f"focus {name} size gate is not exact")
        if uint(metric.get("diff_rows"), f"focus.{name}.diff_rows") != len(context["residual_rows"]):
            raise JoinInputError(f"focus {name} residual row count drift")
        projections.append(diff_projection(channel, f"focus.channels.{name}"))
    if projections[0] != context["residual_rows"] or projections[1] != context["residual_rows"]:
        raise JoinInputError("focus residual row descriptors drifted")
    for row in context["residual_rows"]:
        formatted = [row[f"{side}_formatted"] for side in ("target", "candidate") if row[f"{side}_formatted"] is not None]
        if not formatted or any(not value.startswith(("lfs ", "fmuls ")) for value in formatted):
            raise JoinInputError("production residual escaped the sealed lfs/fmuls family")
    strict = mapping(channels.get("strict"), "focus.channels.strict")
    sibling_projections = []
    for channel_name in ("strict", "data"):
        sibling_row = mapping(mapping(channels.get(channel_name), f"focus.channels.{channel_name}").get("protected_siblings"), f"focus.channels.{channel_name}.protected_siblings")
        identities = [text(item, "protected sibling identity", 192) for item in sequence(sibling_row.get("exact_identities"), "protected sibling identities")]
        digest = sha256(sibling_row.get("exact_identity_sha256"), "protected sibling digest")
        if uint(sibling_row.get("exact_sibling_count"), "protected exact sibling count") != len(identities) or len(identities) != len(set(identities)) or canonical_sha256(identities) != digest:
            raise JoinInputError("protected sibling digest/count drift")
        sibling_projections.append((identities, digest, sha256(sibling_row.get("all_sibling_metric_sha256"), "all sibling metric digest")))
    if sibling_projections[0] != sibling_projections[1]:
        raise JoinInputError("strict/data protected sibling gates disagree")
    section_sets = mapping(strict.get("sections"), "focus.channels.strict.sections")
    target_sections = {text(mapping(row, "target section").get("name"), "target section name", 64): mapping(row, "target section") for row in sequence(section_sets.get("target"), "focus target sections")}
    candidate_sections = {text(mapping(row, "candidate section").get("name"), "candidate section name", 64): mapping(row, "candidate section") for row in sequence(section_sets.get("candidate"), "focus candidate sections")}
    for section_name in (".data", ".bss", ".sdata2"):
        target_section = target_sections.get(section_name)
        candidate_section = candidate_sections.get(section_name)
        if target_section is None or candidate_section is None or float(target_section.get("match_percent", -1)) != 100.0 or decimal_uint(target_section.get("size"), f"target {section_name} size") != decimal_uint(candidate_section.get("size"), f"candidate {section_name} size"):
            raise JoinInputError(f"production data section {section_name} is not exact")
    target_rows = sequence(mapping(strict.get("target"), "focus.strict.target").get("rows"), "focus.strict.target.rows")
    candidate_rows = sequence(mapping(strict.get("candidate"), "focus.strict.candidate").get("rows"), "focus.strict.candidate.rows")
    residual_indices = {row["index"] for row in context["residual_rows"]}
    target_exact = {uint(mapping(row, "target row").get("index"), "target row index"): mapping(row, "target row") for row in target_rows if mapping(row, "target row").get("diff_kind") is None}
    candidate_exact = {uint(mapping(row, "candidate row").get("index"), "candidate row index"): mapping(row, "candidate row") for row in candidate_rows if mapping(row, "candidate row").get("diff_kind") is None}
    if set(target_exact) != set(candidate_exact) or any(index in residual_indices for index in target_exact):
        raise JoinInputError("focus nonresidual row alignment drift")
    for index in target_exact:
        left = mapping(target_exact[index].get("instruction"), f"focus target row {index}.instruction")
        right = mapping(candidate_exact[index].get("instruction"), f"focus candidate row {index}.instruction")
        left_relocation = left.get("relocation")
        right_relocation = right.get("relocation")
        if (left_relocation is None) != (right_relocation is None):
            raise JoinInputError(f"focus relocation presence drift at row {index}")
        if left_relocation is not None:
            left_relocation = mapping(left_relocation, f"focus target row {index}.relocation")
            right_relocation = mapping(right_relocation, f"focus candidate row {index}.relocation")
            for key in ("type", "type_name"):
                if left_relocation.get(key) != right_relocation.get(key):
                    raise JoinInputError(f"focus relocation type drift at row {index}")
            left_parts = sha256(target_exact[index].get("parts_sha256"), f"focus target row {index}.parts_sha256")
            right_parts = sha256(candidate_exact[index].get("parts_sha256"), f"focus candidate row {index}.parts_sha256")
            if left_parts != right_parts or decimal_uint(left.get("size"), f"focus target row {index}.size") != decimal_uint(right.get("size"), f"focus candidate row {index}.size"):
                raise JoinInputError(f"focus relocation-backed structural parts drift at row {index}")
        elif left.get("formatted") != right.get("formatted") or left.get("branch_dest") != right.get("branch_dest"):
            raise JoinInputError(f"focus has unexplained structural drift at row {index}")

    receipt = read_json(context["physical_receipt"]["path"], "physical relocation receipt")
    if receipt.get("receipt_sha256") != context["physical_receipt"]["receipt_sha256"]:
        raise JoinInputError("physical relocation receipt payload binding mismatch")
    verify_self_hash(receipt, "receipt_sha256", "physical relocation receipt")
    focus_receipt = mapping(receipt.get("focus"), "physical receipt.focus")
    if receipt.get("schema") != "mp6_physical_relocation_receipt/v1" or focus_receipt.get("function") != context["function"]:
        raise JoinInputError("physical relocation receipt schema/function mismatch")
    if receipt.get("authority_advanced") is not False or receipt.get("source_patch_emitted") is not False or receipt.get("physical_relocations_exact") is not True or receipt.get("physical_relocation_differences") != []:
        raise JoinInputError("physical relocations are not exact")
    if mapping(receipt.get("strict_report"), "physical receipt.strict_report").get("sha256") != context["strict_report_sha"]:
        raise JoinInputError("physical receipt report identity mismatch")
    if mapping(receipt.get("focus_artifact"), "physical receipt.focus_artifact").get("sha256") != context["focus"]["sha256"]:
        raise JoinInputError("physical receipt focus identity mismatch")
    target = mapping(receipt.get("target"), "physical receipt.target")
    candidate = mapping(receipt.get("candidate"), "physical receipt.candidate")
    if mapping(target.get("object"), "physical receipt.target.object").get("sha256") != context["target_object"]["sha256"] or mapping(candidate.get("object"), "physical receipt.candidate.object").get("sha256") != context["production_object"]["sha256"]:
        raise JoinInputError("physical receipt object identities mismatch")
    expected_count = context["physical_relocation_count"]
    target_rows = sequence(target.get("physical_relocations"), "physical receipt.target.physical_relocations")
    candidate_rows = sequence(candidate.get("physical_relocations"), "physical receipt.candidate.physical_relocations")
    if target.get("physical_relocation_count") != expected_count or candidate.get("physical_relocation_count") != expected_count or len(target_rows) != expected_count or len(candidate_rows) != expected_count:
        raise JoinInputError("physical relocation count drift")
    def normalized_relocation(raw: Any, label: str) -> dict[str, Any]:
        row = mapping(raw, label)
        return {
            "offset": uint(row.get("offset"), f"{label}.offset"),
            "section_offset": uint(row.get("section_offset"), f"{label}.section_offset"),
            "type": uint(row.get("type"), f"{label}.type"),
            "type_name": text(row.get("type_name"), f"{label}.type_name", 64),
            "addend": row.get("addend"),
            "effective_target": mapping(row.get("effective_target"), f"{label}.effective_target"),
        }
    normalized_target = [normalized_relocation(row, f"physical target row {i}") for i, row in enumerate(target_rows)]
    normalized_candidate = [normalized_relocation(row, f"physical candidate row {i}") for i, row in enumerate(candidate_rows)]
    def relocation_key(row: Mapping[str, Any]) -> tuple[int, int, int]:
        return (int(row["offset"]), int(row["section_offset"]), int(row["type"]))
    if normalized_target != sorted(normalized_target, key=relocation_key) or normalized_candidate != sorted(normalized_candidate, key=relocation_key) or len({relocation_key(row) for row in normalized_target}) != expected_count or len({relocation_key(row) for row in normalized_candidate}) != expected_count:
        raise JoinInputError("physical relocation rows are unsorted or duplicate")
    if normalized_target != normalized_candidate:
        raise JoinInputError("physical relocation normalized rows differ")
    return {
        "focus_artifact_sha256": context["focus"]["sha256"],
        "focus_payload_sha256": context["focus"]["artifact_sha256"],
        "target_object_sha256": context["target_object"]["sha256"],
        "production_candidate_object_sha256": context["production_object"]["sha256"],
        "strict_report_sha256": context["strict_report_sha"],
        "data_report_sha256": context["data_report_sha"],
        "physical_relocation_receipt_sha256": context["physical_receipt"]["sha256"],
        "physical_relocation_receipt_payload_sha256": context["physical_receipt"]["receipt_sha256"],
        "residual_rows": context["residual_rows"],
    }


def analyze_site(
    site: int,
    session: str,
    function: str,
    pcode_events: Sequence[Mapping[str, Any]],
    machine_events: Sequence[Mapping[str, Any]],
    inventory: Mapping[str, str],
    source_bindings: Mapping[str, Mapping[str, Any]],
    graph_site: Mapping[str, Any],
    process_id: int,
) -> dict[str, Any]:
    machines = [event for event in machine_events if event.get("instruction_index") == site]
    if len(machines) != 1:
        raise JoinInputError(f"site {site} does not have one machine event")
    machine = machines[0]
    validate_event(machine, session, function, f"machine site {site}")
    event_process(machine, process_id, f"machine site {site}")
    if machine.get("event_kind") != "machine_emission" or machine.get("hook_id") != "gc27_machine_emit" or machine.get("lane") != "pcode":
        raise JoinInputError(f"site {site} machine hook/lane is not authenticated")
    if machine.get("status") != "UNKNOWN" or machine.get("reason") != "ambiguous reaching definition":
        raise JoinInputError(f"site {site} machine terminal status is not the expected partial-evidence UNKNOWN")
    if machine.get("mnemonic") != "fmuls" or machine.get("arithmetic_op") != "multiply":
        raise JoinInputError(f"site {site} is not a sealed fmuls")
    registers = mapping(machine.get("registers"), f"site {site}.registers")
    if set(registers) != {"destination", "source_a", "source_b"}:
        raise JoinInputError(f"site {site} has an unsupported machine role set")
    for value in registers.values():
        if not isinstance(value, str) or REGISTER_RE.fullmatch(value) is None:
            raise JoinInputError(f"site {site} has an invalid physical register")
    decoded = decode_fmuls(machine.get("ppc_bytes"), f"site {site}")
    if uint(machine.get("ppc_word"), f"site {site}.ppc_word") != int(text(machine.get("ppc_bytes"), f"site {site}.ppc_bytes", 8), 16):
        raise JoinInputError(f"site {site} ppc_word conflicts with ppc_bytes")
    if dict(registers) != decoded:
        raise JoinInputError(f"site {site} registers conflict with independently decoded PPC bytes")
    pcode_token = text(machine.get("pcode_token"), f"site {site}.pcode_token", 192)
    operands = [
        event for event in pcode_events
        if event.get("event_kind") == "pcode_capture" and event.get("pcode_token") == pcode_token
    ]
    if len(operands) != 3 or {event.get("operand_ordinal") for event in operands} != {0, 1, 2}:
        raise JoinInputError(f"site {site} lacks one complete three-operand PCode row")
    operands.sort(key=lambda event: int(event["operand_ordinal"]))
    roles = ("destination", "source_a", "source_b")
    parsed: list[dict[str, Any]] = []
    for role, event in zip(roles, operands):
        validate_event(event, session, function, f"site {site} PCode {role}")
        event_process(event, process_id, f"site {site} PCode {role}")
        if event.get("event_kind") != "pcode_capture" or event.get("status") != "CAPTURED" or event.get("confirmed") is not True or event.get("hook_id") != "pcode_color_post" or event.get("stage") != "pcode_color_post" or event.get("lane") != "pcode" or event.get("operand_count") != 3:
            raise JoinInputError(f"site {site} PCode {role} is not authenticated")
        color = uint(event.get("final_color"), f"site {site}.{role}.final_color")
        register = physical_for(text(event.get("operand_bank"), "operand bank", 3), color)
        if register != registers[role]:
            raise JoinInputError(f"site {site} PCode color conflicts with machine {role}")
        owner_fields = [field for field in ("object_token", "hidden_owner_token") if isinstance(event.get(field), str)]
        if len(owner_fields) != 1:
            raise JoinInputError(f"site {site} PCode {role} owner is absent or ambiguous")
        owner_field = owner_fields[0]
        owner_token = text(event[owner_field], f"site {site}.{role}.{owner_field}", 192)
        if session not in owner_token:
            raise JoinInputError(f"site {site} owner token session drift")
        parsed.append({
            "role": role,
            "event_id": text(event.get("event_id"), "PCode event ID", 192),
            "pcode_token": pcode_token,
            "owner_kind": "OBJECT" if owner_field == "object_token" else "HIDDEN",
            "owner_token": owner_token,
            "allocator_node": text(event.get("ig_token"), "IG token", 192),
            "allocator_identity": {"kind": "IG_NODE", "id": text(event.get("ig_token"), "IG token", 192)},
            "edge_mode": "DIRECT_PCODE_OBJECT_TO_IG_COLOR" if owner_field == "object_token" else "DIRECT_PCODE_HIDDEN_OWNER_TO_IG_COLOR",
            "legacy_vreg_id": None,
            "final_color": color,
            "physical_register": register,
        })
    named = [row for row in parsed[1:] if row["owner_kind"] == "OBJECT"]
    hidden_inputs = [row for row in parsed[1:] if row["owner_kind"] == "HIDDEN"]
    if len(named) != 1 or len(hidden_inputs) != 1:
        raise JoinInputError(f"site {site} is not one named-owner x one hidden-owner chain")
    named_owner = named[0]
    name = inventory.get(named_owner["owner_token"])
    source_binding = source_bindings.get(named_owner["owner_token"])
    if source_binding is None or source_binding["name"] != name:
        raise JoinInputError(f"site {site} named owner is not sealed by context")
    hidden = hidden_inputs[0]
    producer_rows = [
        event for event in pcode_events
        if event.get("event_kind") == "pcode_capture"
        and event.get("hidden_owner_token") == hidden["owner_token"]
        and event.get("operand_ordinal") == 0
        and event.get("operand_bank") == "FPR"
        and event.get("ig_token") == hidden["allocator_node"]
    ]
    if len(producer_rows) != 1:
        raise JoinInputError(f"site {site} hidden input has no unique PCode definition")
    producer = producer_rows[0]
    validate_event(producer, session, function, f"site {site} hidden producer")
    event_process(producer, process_id, f"site {site} hidden producer")
    if producer.get("status") != "CAPTURED" or producer.get("confirmed") is not True or producer.get("hook_id") != "pcode_color_post" or producer.get("stage") != "pcode_color_post" or producer.get("lane") != "pcode" or producer.get("operand_count") != 3:
        raise JoinInputError(f"site {site} hidden producer PCode event is not authenticated")
    if producer.get("ig_token") != hidden["allocator_node"]:
        raise JoinInputError(f"site {site} hidden producer IG does not reach the selected hidden input")
    producer_token = text(producer.get("pcode_token"), "producer PCode token", 192)
    producer_machines = [event for event in machine_events if event.get("pcode_token") == producer_token]
    if len(producer_machines) != 1:
        raise JoinInputError(f"site {site} hidden producer has no unique machine emission")
    producer_machine = producer_machines[0]
    validate_event(producer_machine, session, function, f"site {site} producer machine")
    event_process(producer_machine, process_id, f"site {site} producer machine")
    producer_machine_status = producer_machine.get("status")
    producer_output_sealed = producer_machine_status == "CAPTURED" or (
        producer_machine_status == "UNKNOWN" and producer_machine.get("reason") == "ambiguous reaching definition"
    )
    if producer_machine.get("event_kind") != "machine_emission" or producer_machine.get("hook_id") != "gc27_machine_emit" or producer_machine.get("lane") != "pcode" or not producer_output_sealed:
        raise JoinInputError(f"site {site} hidden producer machine event is not authenticated")
    producer_index = uint(producer_machine.get("instruction_index"), "producer instruction index")
    if producer_index >= site:
        raise JoinInputError(f"site {site} hidden producer chronology is invalid")
    producer_sequence = uint(producer.get("sequence"), f"site {site} producer sequence")
    producer_machine_sequence = uint(producer_machine.get("sequence"), f"site {site} producer machine sequence")
    consumer_sequences = [uint(event.get("sequence"), f"site {site} consumer PCode sequence") for event in operands]
    consumer_machine_sequence = uint(machine.get("sequence"), f"site {site} consumer machine sequence")
    if producer_sequence >= min(consumer_sequences) or producer_machine_sequence >= consumer_machine_sequence:
        raise JoinInputError(f"site {site} producer event chronology is invalid")
    producer_destination = decode_fpr_destination(producer_machine, f"site {site} hidden producer machine")
    producer_bank = text(producer.get("operand_bank"), f"site {site} producer bank", 3)
    producer_color = uint(producer.get("final_color"), f"site {site} producer final color")
    if producer_bank != "FPR" or physical_for(producer_bank, producer_color) != producer_destination or producer_color != hidden["final_color"] or producer_destination != hidden["physical_register"]:
        raise JoinInputError(f"site {site} hidden producer color conflicts with decoded destination")
    memory_op = producer_machine.get("memory_op")
    if memory_op is None and producer_machine.get("arithmetic_op") is not None:
        boundary = "DIRECT_EXPRESSION_ARITHMETIC_BOUNDARY"
    elif memory_op == "load":
        boundary = "RELOADED_VALUE_BOUNDARY"
    else:
        boundary = "UNSUPPORTED_BOUNDARY"
    if boundary == "UNSUPPORTED_BOUNDARY":
        raise JoinInputError(f"site {site} hidden producer boundary is unsupported")
    named_owner = dict(named_owner)
    named_owner["source_name"] = name
    named_owner["source_span"] = source_binding
    named_owner["vreg_absence_reason"] = "legacy capture has no regalloc_assignment; direct authenticated Object-to-IG color edge is used"
    result = {
        "machine_site": site,
        "machine_event_id": text(machine.get("event_id"), "machine event ID", 192),
        "machine_pcode_token": pcode_token,
        "named_owner": named_owner,
        "hidden_input": hidden,
        "hidden_definition": {
            "event_id": text(producer.get("event_id"), "producer event ID", 192),
            "pcode_token": producer_token,
            "owner_token": hidden["owner_token"],
            "allocator_node": text(producer.get("ig_token"), "producer IG token", 192),
            "allocator_identity": {"kind": "IG_NODE", "id": text(producer.get("ig_token"), "producer IG token", 192)},
            "edge_mode": "DIRECT_PCODE_HIDDEN_OWNER_TO_IG_COLOR",
            "legacy_vreg_id": None,
            "final_color": producer_color,
            "physical_register": producer_destination,
            "machine_event_id": text(producer_machine.get("event_id"), "producer machine event ID", 192),
            "instruction_index": producer_index,
            "mnemonic": text(producer_machine.get("mnemonic"), "producer mnemonic", 32),
            "boundary_kind": boundary,
            "machine_status": producer_machine_status,
            "producer_input_ownership_claimed": False,
        },
        "def_use_path": {
            "instruction_indices": [producer_index, site],
            "edges": [
                f"{hidden['owner_token']}->{hidden['allocator_node']}",
                f"{hidden['allocator_node']}->{hidden['physical_register']}",
                f"machine:{producer_index}->machine:{site}",
            ],
        },
        "topology_signature": {
            "named_owner_role": named_owner["role"],
            "named_owner_bank": named_owner["physical_register"][0],
            "hidden_input_role": hidden["role"],
            "hidden_boundary_kind": boundary,
            "machine_mnemonic": "fmuls",
        },
    }
    graph_operands = sequence(graph_site.get("pcode_operands"), f"graph site {site}.pcode_operands")
    graph_projection = [
        {
            "event_id": row["event_id"],
            "ig_token": row["allocator_node"],
            "final_color": row["final_color"],
            "operand_ordinal": ordinal,
            "owner_identity": ({"status": "PRESENT", "object_token": row["owner_token"]}
                               if row["owner_kind"] == "OBJECT"
                               else {"status": "UNKNOWN", "hidden_owner_token": row["owner_token"]}),
        }
        for ordinal, row in enumerate(parsed)
    ]
    normalized_graph = [
        {key: row.get(key) for key in ("event_id", "ig_token", "final_color", "operand_ordinal", "owner_identity")}
        for row in graph_operands
    ]
    if normalized_graph != graph_projection or graph_site.get("pcode_token") != pcode_token or graph_site.get("ppc_bytes") != machine.get("ppc_bytes"):
        raise JoinInputError(f"site {site} raw events conflict with bound failure graph")
    return result


def analyze(context_value: Mapping[str, Any]) -> dict[str, Any]:
    context = parse_context(context_value)
    production_bindings = verify_production_evidence(context)
    partial_path = context["partial_path"]
    if not partial_path.is_absolute() or partial_path.is_symlink() or not partial_path.is_file():
        raise JoinInputError("partial evidence path must be an absolute regular file")
    if file_sha256(partial_path) != context["partial_sha"]:
        raise JoinInputError("partial evidence SHA-256 mismatch")
    partial = read_json(partial_path, "partial evidence")
    if partial.get("schema") != PARTIAL_SCHEMA or partial.get("status") != "UNKNOWN" or partial.get("diagnostic_only") is not True:
        raise JoinInputError("partial evidence is not immutable diagnostic UNKNOWN evidence")
    if partial.get("authority_advanced") is not False or partial.get("board_admission") is not False:
        raise JoinInputError("partial evidence advanced authority")
    pctx = mapping(partial.get("context"), "partial.context")
    if pctx.get("function") != context["function"] or pctx.get("session_id") != context["session"]:
        raise JoinInputError("partial function/session mismatch")
    if mapping(pctx.get("source"), "partial.context.source").get("sha256") != context["source_sha"]:
        raise JoinInputError("partial source mismatch")
    if mapping(pctx.get("compiler"), "partial.context.compiler").get("sha256") != context["compiler_sha"]:
        raise JoinInputError("partial compiler mismatch")
    trust = mapping(partial.get("trust_root"), "partial.trust_root")
    trust_fields = mapping(trust.get("fields"), "partial.trust_root.fields")
    if canonical_sha256(trust_fields) != trust.get("binding_sha256"):
        raise JoinInputError("partial trust-root binding mismatch")
    if trust_fields.get("function") != context["function"] or trust_fields.get("source_sha256") != context["source_sha"] or trust_fields.get("compiler_sha256") != context["compiler_sha"]:
        raise JoinInputError("partial trust-root identities mismatch")
    candidate = mapping(partial.get("compiler_owned_object"), "partial.compiler_owned_object")
    if candidate.get("sha256") != context["trace_object_sha"]:
        raise JoinInputError("partial candidate object mismatch")
    candidate_path = Path(text(candidate.get("path"), "candidate object path"))
    if not candidate_path.is_file() or candidate_path.is_symlink() or file_sha256(candidate_path) != context["trace_object_sha"]:
        raise JoinInputError("candidate object identity mismatch")
    descriptors = {key: artifact_descriptor(partial, key) for key in ("candidate_envelope", "failure_graph", "machine_events", "pcode_events", "hook_validation")}
    if descriptors["failure_graph"]["sha256"] != context["graph_sha"]:
        raise JoinInputError("failure graph context mismatch")
    envelope = read_json(descriptors["candidate_envelope"]["path"], "candidate envelope")
    graph = read_json(descriptors["failure_graph"]["path"], "failure graph")
    hook = read_json(descriptors["hook_validation"]["path"], "hook validation")
    verify_self_hash(partial, "manifest_sha256", "partial evidence")
    verify_self_hash(envelope, "envelope_sha256", "candidate envelope")
    hook_unsigned = dict(hook)
    hook_receipt = hook_unsigned.pop("receipt_sha256", None)
    if hook.get("status") != "AUTHENTICATED_PARTIAL_CAPTURE" or hook.get("authority_advanced") is not False or hook.get("board_admission") is not False or canonical_sha256(hook_unsigned) != hook_receipt:
        raise JoinInputError("hook validation receipt is not authenticated")
    if hook.get("session_id") != context["session"] or hook.get("function") != context["function"]:
        raise JoinInputError("hook validation session/function mismatch")
    if mapping(hook.get("source"), "hook.source").get("sha256") != context["source_sha"] or mapping(hook.get("compiler"), "hook.compiler").get("sha256") != context["compiler_sha"] or mapping(hook.get("compiler_owned_object"), "hook.compiler_owned_object").get("sha256") != context["trace_object_sha"]:
        raise JoinInputError("hook validation source/compiler/object mismatch")
    hooks = sequence(hook.get("hooks"), "hook.hooks")
    hook_pairs = {(mapping(row, "hook row").get("id"), mapping(row, "hook row").get("lane")) for row in hooks}
    if not {("pcode_color_post", "pcode"), ("gc27_machine_emit", "pcode")} <= hook_pairs:
        raise JoinInputError("required PCode/machine hooks are absent")
    if envelope.get("context", {}).get("session_id") != context["session"] or graph.get("session_id") != context["session"]:
        raise JoinInputError("envelope/graph session mismatch")
    if graph.get("function") != context["function"] or graph.get("authority_advanced") is not False:
        raise JoinInputError("failure graph function/authority mismatch")
    graph_unsigned = dict(graph)
    graph_digest = graph_unsigned.pop("failure_graph_sha256", None)
    if canonical_sha256(graph_unsigned) != graph_digest:
        raise JoinInputError("failure graph internal self-hash mismatch")
    inventory = owner_inventory(envelope, context["session"])
    process_id = uint(pctx.get("process_id"), "partial.context.process_id")
    source_descriptor = mapping(pctx.get("source"), "partial.context.source")
    source_bindings = verify_source_owners(Path(text(source_descriptor.get("path"), "source path")), context["source_sha"], context["session"], context["owners"])
    pcode_events = read_jsonl(descriptors["pcode_events"]["path"], descriptors["pcode_events"]["sha256"], "PCode events")
    machine_events = read_jsonl(descriptors["machine_events"]["path"], descriptors["machine_events"]["sha256"], "machine events")
    graph_sites_by_index: dict[int, list[Mapping[str, Any]]] = {}
    for raw in sequence(graph.get("unresolved_machine_sites"), "graph.unresolved_machine_sites"):
        row = mapping(raw, "graph unresolved site")
        graph_sites_by_index.setdefault(uint(row.get("instruction_index"), "graph instruction index"), []).append(row)
    chains = []
    for site in context["sites"]:
        graph_sites = graph_sites_by_index.get(site, [])
        if len(graph_sites) != 1:
            raise JoinInputError(f"site {site} is absent or ambiguous in failure graph")
        chains.append(analyze_site(site, context["session"], context["function"], pcode_events, machine_events, inventory, source_bindings, graph_sites[0], process_id))
    signatures = {canonical_sha256(chain["topology_signature"]) for chain in chains}
    if len(signatures) != 1:
        raise JoinInputError("selected arithmetic sites do not share one repeated ownership topology")
    seen_names = {chain["named_owner"]["source_name"] for chain in chains}
    if not seen_names <= {row["name"] for row in context["owners"]}:
        raise JoinInputError("selected arithmetic sites escaped the owner allowlist")
    boundary = chains[0]["hidden_definition"]["boundary_kind"]
    derived_class = {
        "RELOADED_VALUE_BOUNDARY": "named_owner_chronology_x_reloaded_value_boundary",
        "DIRECT_EXPRESSION_ARITHMETIC_BOUNDARY": "named_owner_chronology_x_direct_expression_boundary",
    }.get(boundary)
    if derived_class is None or derived_class not in context["class_allowlist"]:
        raise JoinInputError("derived natural source class is not permitted by the bounded allowlist")
    control_classes = {control["source_class"] for control in context["controls"]}
    if derived_class in control_classes:
        raise JoinInputError("derived source class is already measured and suppressed")
    opposing_kinds = {"DIRECT_EXPRESSION_ARITHMETIC_BOUNDARY"} if boundary == "RELOADED_VALUE_BOUNDARY" else {"RELOADED_VALUE_BOUNDARY", "SPILL_RELOAD_BOUNDARY"}
    if len(control_classes) < 2 or not ({control["boundary_kind"] for control in context["controls"]} & opposing_kinds):
        raise JoinInputError("measured controls do not cover a competing boundary class")
    residual_indices = {row["index"] for row in context["residual_rows"]}
    grouped_indices = {item for group in context["row_groups"] for item in group["residual_indices"]}
    if grouped_indices != residual_indices or sum(len(group["residual_indices"]) for group in context["row_groups"]) != len(grouped_indices):
        raise JoinInputError("production row groups do not partition the exact residual rows")
    group_by_site = {group["trace_machine_site"]: group for group in context["row_groups"]}
    residual_by_index = {row["index"]: row for row in context["residual_rows"]}
    for chain in chains:
        group = group_by_site[chain["machine_site"]]
        fmuls_row = residual_by_index.get(group["production_fmuls_index"])
        if fmuls_row is None or not all(isinstance(fmuls_row[f"{side}_formatted"], str) and fmuls_row[f"{side}_formatted"].startswith("fmuls ") for side in ("target", "candidate")):
            raise JoinInputError("production row group does not bind one target/candidate fmuls residual")
        chain["production_residual_group"] = dict(group)
    result: dict[str, Any] = {
        "schema": OUTPUT_SCHEMA,
        "status": "RANKED_SOURCE_CLASS",
        "authority_advanced": False,
        "diagnostic_only": True,
        "function": context["function"],
        "implementation": implementation_binding(),
        "session_id": context["session"],
        "bindings": {
            "context_sha256": context["context_sha"],
            "partial_evidence_sha256": context["partial_sha"],
            "failure_graph_sha256": context["graph_sha"],
            "source_sha256": context["source_sha"],
            "compiler_sha256": context["compiler_sha"],
            "trace_candidate_object_sha256": context["trace_object_sha"],
            "candidate_envelope_sha256": descriptors["candidate_envelope"]["sha256"],
            "pcode_events_sha256": descriptors["pcode_events"]["sha256"],
            "machine_events_sha256": descriptors["machine_events"]["sha256"],
            "hook_validation_sha256": descriptors["hook_validation"]["sha256"],
            **production_bindings,
        },
        "chains": chains,
        "repeated_topology_sha256": next(iter(signatures)),
        "ranked_source_classes": [{
            "rank": 1,
            "source_class": derived_class,
            "boundary_kind": boundary,
            "source_owners": sorted(seen_names),
            "predicted_scope": {
                "trace_machine_sites": context["sites"],
                "production_residual_groups": context["row_groups"],
                "residual_rows": context["residual_rows"],
                "row_count": len(context["residual_rows"]),
            },
            "suppressed_controls": [control["control_id"] for control in context["controls"]],
        }],
        "rejected_controls": context["controls"],
        "limitations": [
            "No source retention, candidate admission, compile, or promotion is authorized.",
            "Legacy evidence omits regalloc vreg IDs; each named owner instead uses the direct authenticated Object-to-IG PCode edge.",
            "operand_index was not read or interpreted as a vreg.",
        ],
    }
    result["result_sha256"] = canonical_sha256(result)
    return result


def unknown(context_value: Mapping[str, Any], reason: str) -> dict[str, Any]:
    result = {
        "schema": OUTPUT_SCHEMA,
        "status": "UNKNOWN",
        "authority_advanced": False,
        "diagnostic_only": True,
        "function": context_value.get("function") if isinstance(context_value, Mapping) else None,
        "implementation": implementation_binding(),
        "first_missing_edge": reason,
        "ranked_source_classes": [],
    }
    result["result_sha256"] = canonical_sha256(result)
    return result


def validated_output_path(path: Path) -> Path:
    if not path.is_absolute():
        raise JoinInputError("--output must be absolute")
    repository_root = Path(__file__).resolve().parent.parent
    build_root = repository_root / "build"
    lexical = Path(os.path.abspath(path))
    try:
        relative = lexical.relative_to(build_root)
    except ValueError as exc:
        raise JoinInputError("--output must be contained under repository build/") from exc
    if not relative.parts:
        raise JoinInputError("--output must name a file under repository build/")
    current = build_root
    for part in relative.parts[:-1]:
        if current.is_symlink():
            raise JoinInputError("--output build/parent path must not contain symlinks")
        current /= part
    if current.is_symlink() or lexical.is_symlink():
        raise JoinInputError("--output build/parent path must not contain symlinks")
    current.mkdir(parents=True, exist_ok=True)
    current = build_root
    for part in relative.parts[:-1]:
        if current.is_symlink():
            raise JoinInputError("--output build/parent path must not contain symlinks")
        current /= part
    if current.is_symlink() or lexical.is_symlink():
        raise JoinInputError("--output build/parent path must not contain symlinks")
    return lexical


def atomic_write_json(output: Path, result: Mapping[str, Any]) -> None:
    payload = json.dumps(result, indent=2, sort_keys=True).encode("utf-8") + b"\n"
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(mode="wb", delete=False, dir=output.parent, prefix=f".{output.name}.", suffix=".tmp") as stream:
            temporary = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, output)
        temporary = None
    finally:
        if temporary is not None:
            try:
                temporary.unlink()
            except FileNotFoundError:
                pass


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", required=True, type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    output: Path | None = None
    if args.output is not None:
        try:
            output = validated_output_path(args.output)
        except (JoinInputError, OSError) as exc:
            sys.stderr.write(f"hidden_arithmetic_owner_join: {exc}\n")
            return 2
    try:
        context = read_json(args.context.resolve(), "context")
        result = analyze(context)
    except (JoinInputError, OSError) as exc:
        try:
            context = read_json(args.context.resolve(), "context")
        except Exception:
            context = {}
        result = unknown(context, str(exc))
    payload = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if output is not None:
        try:
            atomic_write_json(output, result)
        except OSError as exc:
            sys.stderr.write(f"hidden_arithmetic_owner_join: atomic output failed: {exc}\n")
            return 2
    else:
        sys.stdout.write(payload)
    return 0 if result["status"] == "RANKED_SOURCE_CLASS" else 2


if __name__ == "__main__":
    raise SystemExit(main())
