#!/usr/bin/env python3
"""Reduce authenticated MWCC stack homes to bounded source-lifetime axes.

The reducer is read-only.  ``bind`` reads one objdiff report, candidate source,
and optional legacy VarInfo report and prints a compact, self-hashed bound.
``reduce`` composes those bounds with the packet and summary produced by
``tools.capsule_stack_home_native``.  It never launches a compiler, writes an
artifact, edits source, records a candidate, or advances recovery authority.

Physical homes are not source ownership.  The only source facts emitted here
are exact identifier intervals from the bound source bytes.  Natural-C axes
remain diagnostic recommendations; dead locals, padding, and register shaping
are explicitly forbidden.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import math
import os
from pathlib import Path
import re
import stat
import sys
from typing import Any, Mapping, Sequence


BOUND_SCHEMA = "mwcc_stack_object_lifetime_bound/v1"
REQUEST_SCHEMA = "mwcc_stack_object_lifetime_request/v1"
REPORT_SCHEMA = "mwcc_stack_object_lifetime_reducer/v1"
TOOL_VERSION = "stack-object-lifetime-reducer-1"
STATUS = "REDUCED_DIAGNOSTIC_ONLY"
MAX_JSON_BYTES = 64 * 1024 * 1024
MAX_SOURCE_BYTES = 4 * 1024 * 1024
MAX_OBSERVATIONS = 32
SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
SAFE_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}\Z")
TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
ARRAY_EXPR_RE = re.compile(r"\[\s*([A-Za-z_][A-Za-z0-9_]*|[1-9][0-9]*)\s*\]")
ABI_TYPE_SIZES = {
    "char": 1,
    "s8": 1,
    "u8": 1,
    "short": 2,
    "s16": 2,
    "u16": 2,
    "int": 4,
    "s32": 4,
    "u32": 4,
    "float": 4,
    "double": 8,
    "HuVecF": 12,
    "Vec": 12,
    "Mtx": 48,
}
ABI_ARRAY_CONSTANTS = {
    # game/gamework.h ABI: the source spelling is retained alongside the
    # resolved value so a four-player-capacity declaration is not flattened
    # into an unexplained literal extent.
    "GW_PLAYER_MAX": 4,
}


class ReducerError(ValueError):
    """An input cannot support a deterministic lifetime reduction."""


def _fail(message: str) -> None:
    raise ReducerError(message)


def _pairs(items: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in items:
        if key in result:
            _fail(f"duplicate JSON key {key!r}")
        result[key] = value
    return result


def _canonical(value: Any) -> bytes:
    try:
        return (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise ReducerError(f"cannot serialize canonical JSON: {exc}") from exc


def _digest(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _seal(value: Mapping[str, Any], field: str) -> dict[str, Any]:
    result = dict(value)
    result.pop(field, None)
    result[field] = _digest(result)
    return result


def _closed(
    value: Any,
    *,
    allowed: set[str] | frozenset[str],
    required: set[str] | frozenset[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        _fail(f"{label} must be an object")
    unknown = sorted(set(value) - set(allowed))
    if unknown:
        _fail(f"{label} contains unknown field {unknown[0]!r}")
    missing = sorted(set(required) - set(value))
    if missing:
        _fail(f"{label} lacks required field {missing[0]!r}")
    return value


def _text(value: Any, label: str, *, limit: int = 4096) -> str:
    if not isinstance(value, str) or not value or value != value.strip() or "\x00" in value:
        _fail(f"{label} must be non-empty canonical text")
    if len(value) > limit:
        _fail(f"{label} exceeds {limit} characters")
    return value


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if IDENTIFIER_RE.fullmatch(result) is None:
        _fail(f"{label} must be a C identifier")
    return result


def _safe_id(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if SAFE_ID_RE.fullmatch(result) is None:
        _fail(f"{label} must use 1-128 letters, digits, dot, underscore, or dash")
    return result


def _sha(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if SHA256_RE.fullmatch(result) is None:
        _fail(f"{label} must be a lowercase SHA-256 digest")
    return result


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        _fail(f"{label} must be an integer >= {minimum}")
    return value


def _json_file(path: Path, label: str, *, limit: int = MAX_JSON_BYTES) -> tuple[Mapping[str, Any], bytes]:
    absolute = Path(os.path.abspath(path))
    try:
        info = absolute.stat()
    except FileNotFoundError as exc:
        raise ReducerError(f"{label} does not exist: {absolute}") from exc
    if not stat.S_ISREG(info.st_mode) or absolute.is_symlink():
        _fail(f"{label} must be a non-symlink regular file: {absolute}")
    if info.st_size > limit:
        _fail(f"{label} exceeds {limit} bytes: {absolute}")
    raw = absolute.read_bytes()
    try:
        value = json.loads(raw.decode("utf-8"), object_pairs_hook=_pairs)
    except UnicodeDecodeError as exc:
        raise ReducerError(f"{label} is not UTF-8: {absolute}") from exc
    except json.JSONDecodeError as exc:
        raise ReducerError(
            f"invalid {label} JSON {exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    if not isinstance(value, Mapping):
        _fail(f"{label} must contain one JSON object")
    return value, raw


def _descriptor(path: Path, label: str, *, limit: int = MAX_JSON_BYTES) -> dict[str, Any]:
    absolute = Path(os.path.abspath(path))
    try:
        info = absolute.stat()
    except FileNotFoundError as exc:
        raise ReducerError(f"{label} does not exist: {absolute}") from exc
    if not stat.S_ISREG(info.st_mode) or absolute.is_symlink():
        _fail(f"{label} must be a non-symlink regular file: {absolute}")
    if info.st_size > limit:
        _fail(f"{label} exceeds {limit} bytes: {absolute}")
    digest = hashlib.sha256(absolute.read_bytes()).hexdigest()
    return {"path": str(absolute), "size": info.st_size, "sha256": digest}


def _validate_descriptor(value: Any, label: str, *, limit: int = MAX_JSON_BYTES) -> tuple[dict[str, Any], Path]:
    row = _closed(
        value,
        allowed={"path", "size", "sha256"},
        required={"path", "size", "sha256"},
        label=label,
    )
    raw_path = _text(row.get("path"), f"{label}.path")
    path = Path(raw_path)
    if not path.is_absolute() or str(path.absolute()) != raw_path:
        _fail(f"{label}.path must be absolute and canonically spelled")
    expected_size = _integer(row.get("size"), f"{label}.size")
    if expected_size > limit:
        _fail(f"{label}.size exceeds {limit} bytes")
    expected_sha = _sha(row.get("sha256"), f"{label}.sha256")
    actual = _descriptor(path, label, limit=limit)
    if actual["size"] != expected_size or actual["sha256"] != expected_sha:
        _fail(f"{label} descriptor does not match live bytes: {path}")
    return actual, path


def _instruction_mnemonic(row: Mapping[str, Any]) -> str:
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        return "<none>"
    parts = instruction.get("parts")
    if isinstance(parts, list):
        for part in parts:
            if isinstance(part, Mapping) and isinstance(part.get("opcode"), Mapping):
                mnemonic = part["opcode"].get("mnemonic")
                if isinstance(mnemonic, str) and mnemonic:
                    return mnemonic.lower()
    formatted = instruction.get("formatted")
    if isinstance(formatted, str) and formatted.strip():
        return formatted.split(None, 1)[0].lower()
    return "<unknown>"


def _instruction_shape(
    row: Mapping[str, Any], symbol_names: Sequence[str | None]
) -> dict[str, Any]:
    instruction = row.get("instruction")
    relocation = instruction.get("relocation") if isinstance(instruction, Mapping) else None
    reloc_shape: dict[str, Any] | None = None
    if isinstance(relocation, Mapping):
        # Symbol-table indexes are side-local. Resolve them to semantic names
        # before comparing sides so a changed callee/global is not mislabeled
        # as a stack-home-only argument difference.
        reloc_shape = {
            key: relocation[key]
            for key in ("type", "type_name", "addend")
            if key in relocation
        }
        target_index = relocation.get("target_symbol")
        if isinstance(target_index, int) and not isinstance(target_index, bool):
            if not 0 <= target_index < len(symbol_names):
                _fail("strict relocation target symbol is out of range")
            target_name = symbol_names[target_index]
            # Retail address labels and compiler-generated @N pool labels are
            # side-specific spellings of local storage. External/static C
            # names remain exact so a changed callee/global stays semantic.
            if isinstance(target_name, str) and (
                target_name.startswith(("lbl_", "@", "[."))
            ):
                target_name = "<local-relocation>"
            reloc_shape["target_name"] = target_name
    return {"mnemonic": _instruction_mnemonic(row), "relocation": reloc_shape}


def _frame_size(rows: Sequence[Any], label: str) -> int:
    for index, raw in enumerate(rows[:16]):
        if not isinstance(raw, Mapping) or _instruction_mnemonic(raw) not in {"stwu", "stdu"}:
            continue
        instruction = raw.get("instruction")
        parts = instruction.get("parts") if isinstance(instruction, Mapping) else None
        if not isinstance(parts, list):
            continue
        signed: list[int] = []
        for part in parts:
            arg = part.get("arg") if isinstance(part, Mapping) else None
            if isinstance(arg, Mapping) and "signed" in arg:
                try:
                    signed.append(int(arg["signed"]))
                except (TypeError, ValueError):
                    _fail(f"{label} frame displacement is not an integer")
        if len(signed) == 1 and signed[0] < 0:
            return -signed[0]
        _fail(f"{label} frame update at instruction {index} is ambiguous")
    _fail(f"{label} lacks a bounded negative stack-frame update")


def _focus_symbol(report: Mapping[str, Any], function: str) -> tuple[Mapping[str, Any], Mapping[str, Any]]:
    left = report.get("left")
    right = report.get("right")
    if not isinstance(left, Mapping) or not isinstance(right, Mapping):
        _fail("strict report lacks left/right sides")
    left_symbols = left.get("symbols")
    right_symbols = right.get("symbols")
    if not isinstance(left_symbols, list) or not isinstance(right_symbols, list):
        _fail("strict report lacks symbol arrays")
    matches = [row for row in left_symbols if isinstance(row, Mapping) and row.get("name") == function]
    if len(matches) != 1:
        _fail(f"strict report must contain exactly one left symbol {function!r}")
    target = matches[0]
    pair_index = target.get("target_symbol")
    if isinstance(pair_index, bool) or not isinstance(pair_index, int) or not 0 <= pair_index < len(right_symbols):
        _fail(f"strict report symbol {function!r} is not paired")
    source = right_symbols[pair_index]
    if not isinstance(source, Mapping) or source.get("name") != function:
        _fail(f"strict report paired symbol is not {function!r}")
    return target, source


def _strict_evidence(report: Mapping[str, Any], function: str) -> dict[str, Any]:
    target, source = _focus_symbol(report, function)
    target_rows = target.get("instructions")
    source_rows = source.get("instructions")
    if not isinstance(target_rows, list) or not isinstance(source_rows, list):
        _fail("strict report focus instructions must be arrays")
    if not all(isinstance(row, Mapping) for row in target_rows + source_rows):
        _fail("strict report focus instruction row is malformed")
    left_symbols = report["left"]["symbols"]
    right_symbols = report["right"]["symbols"]
    target_names = [
        row.get("name") if isinstance(row, Mapping) and isinstance(row.get("name"), str) else None
        for row in left_symbols
    ]
    source_names = [
        row.get("name") if isinstance(row, Mapping) and isinstance(row.get("name"), str) else None
        for row in right_symbols
    ]
    counts = Counter(str(row.get("diff_kind", "MATCH")) for row in target_rows)
    target_shape = [_instruction_shape(row, target_names) for row in target_rows]
    source_shape = [_instruction_shape(row, source_names) for row in source_rows]
    mismatches = sum(value for key, value in counts.items() if key != "MATCH")
    if mismatches == 0:
        difference_class = "exact"
    elif set(counts) <= {"MATCH", "DIFF_ARG_MISMATCH"} and target_shape == source_shape:
        difference_class = "home_only"
    else:
        difference_class = "semantic_or_topology"
    match_percent = target.get("match_percent")
    if isinstance(match_percent, bool) or not isinstance(match_percent, (int, float)) or not math.isfinite(match_percent):
        _fail("strict report focus match_percent is invalid")
    return {
        "target_size": int(target.get("size")),
        "source_size": int(source.get("size")),
        "target_frame_size": _frame_size(target_rows, "strict target"),
        "source_frame_size": _frame_size(source_rows, "strict source"),
        "frame_size_delta_target_minus_source": _frame_size(target_rows, "strict target")
        - _frame_size(source_rows, "strict source"),
        "target_instruction_count": len(target_rows),
        "source_instruction_count": len(source_rows),
        "match_percent": float(match_percent),
        "diff_counts": dict(sorted(counts.items())),
        "target_instruction_shape_sha256": _digest(target_shape),
        "source_instruction_shape_sha256": _digest(source_shape),
        "difference_class": difference_class,
    }


def _source_text(path: Path) -> str:
    info = path.stat()
    if info.st_size > MAX_SOURCE_BYTES:
        _fail(f"source exceeds {MAX_SOURCE_BYTES} bytes: {path}")
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError as exc:
        raise ReducerError(f"source is not UTF-8: {path}") from exc


def _line_column(source: str, index: int) -> tuple[int, int]:
    line = source.count("\n", 0, index) + 1
    previous = source.rfind("\n", 0, index)
    return line, index - previous


def _byte_offset(source: str, index: int) -> int:
    return len(source[:index].encode("utf-8"))


def _brace_depth(masked: str, start: int, index: int) -> int:
    depth = 0
    for char in masked[start:index]:
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
    return depth


def _declaration_statement(masked: str, token_start: int, body_start: int) -> tuple[int, int]:
    boundary = max(
        masked.rfind(";", body_start, token_start),
        masked.rfind("{", body_start, token_start),
        masked.rfind("}", body_start, token_start),
    )
    statement_start = boundary + 1
    statement_end = masked.find(";", token_start)
    if statement_end < 0:
        _fail("source identifier declaration has no terminating semicolon")
    return statement_start, statement_end + 1


def _source_objects(source: str, function: str, names: Sequence[str]) -> tuple[dict[str, Any], dict[str, Any]]:
    try:
        from tools import match_workbench
    except ImportError as exc:
        try:
            import match_workbench  # type: ignore[no-redef]
        except ImportError:
            raise ReducerError(
                "tools.match_workbench is required for bounded C extraction"
            ) from exc
    extracted = match_workbench._c_extract_function(source, function, "bound source")
    cleaned = match_workbench._c_clean_comments(source, "bound source")
    masked = match_workbench._c_mask_literals(cleaned, "bound source")
    body_start = int(extracted["body_start"])
    body_end = int(extracted["body_end"])
    objects: dict[str, Any] = {}
    for name in names:
        occurrences = [
            match
            for match in TOKEN_RE.finditer(masked, body_start, body_end)
            if match.group(0) == name
        ]
        if not occurrences:
            _fail(f"focus source object {name!r} does not occur in {function!r}")
        declaration = occurrences[0]
        statement_start, statement_end = _declaration_statement(
            masked, declaration.start(), body_start
        )
        prefix = masked[statement_start : declaration.start()]
        type_tokens = TOKEN_RE.findall(prefix)
        if not type_tokens:
            _fail(f"cannot identify declaration type for {name!r}")
        base_type = type_tokens[-1]
        declaration_text = masked[declaration.end() : statement_end]
        dimension_expressions = ARRAY_EXPR_RE.findall(declaration_text)
        resolved_dimensions = [
            int(value) if value.isdigit() else ABI_ARRAY_CONSTANTS.get(value)
            for value in dimension_expressions
        ]
        dimensions = [int(value) for value in resolved_dimensions if value is not None]
        dimensions_resolved = len(dimensions) == len(dimension_expressions)
        base_size = ABI_TYPE_SIZES.get(base_type)
        extent = (
            None
            if base_size is None or not dimensions_resolved
            else base_size * math.prod(dimensions or [1])
        )
        outer_stride = (
            None
            if base_size is None or not dimensions_resolved
            else base_size * math.prod(dimensions[1:] or [1])
        )
        symbolic_capacity = [
            {
                "symbol": expression,
                "elements": ABI_ARRAY_CONSTANTS[expression],
                "status": "SOURCE_DECLARATION_TIE",
            }
            for expression in dimension_expressions
            if expression in ABI_ARRAY_CONSTANTS
        ]
        declaration_line, declaration_column = _line_column(source, declaration.start())
        intervals: list[dict[str, Any]] = []
        previous_end = statement_end
        for use_index, use in enumerate(occurrences[1:]):
            start_byte = _byte_offset(source, previous_end)
            end_byte = _byte_offset(source, use.end())
            start_line, start_column = _line_column(source, previous_end)
            end_line, end_column = _line_column(source, use.end())
            interval_bytes = source[previous_end : use.end()].encode("utf-8")
            intervals.append(
                {
                    "use_index": use_index,
                    "start_byte": start_byte,
                    "end_byte": end_byte,
                    "start_line": start_line,
                    "start_column": start_column,
                    "end_line": end_line,
                    "end_column": end_column,
                    "interval_sha256": hashlib.sha256(interval_bytes).hexdigest(),
                }
            )
            previous_end = use.end()
        declaration_bytes = source[statement_start:statement_end].encode("utf-8")
        objects[name] = {
            "name": name,
            "base_type": base_type,
            "base_type_size": base_size,
            "dimensions": dimensions,
            "dimension_expressions": dimension_expressions,
            "symbolic_capacity_ties": symbolic_capacity,
            "extent_bytes": extent,
            "outer_stride_bytes": outer_stride,
            "declaration": {
                "line": declaration_line,
                "column": declaration_column,
                "scope_depth": _brace_depth(masked, body_start, declaration.start()),
                "statement_sha256": hashlib.sha256(declaration_bytes).hexdigest(),
            },
            "semantic_use_count": max(0, len(occurrences) - 1),
            "source_use_intervals": intervals,
            "earliest_source_use_interval": intervals[0] if intervals else None,
        }
    function_evidence = {
        key: extracted[key]
        for key in (
            "start_line",
            "end_line",
            "body_sha256",
            "normalized_body_sha256",
            "function_sha256",
        )
    }
    return objects, function_evidence


def _varinfo_evidence(value: Mapping[str, Any], function: str, source: Path, names: Sequence[str]) -> dict[str, Any]:
    try:
        from tools import mwcc_win32_varinfo
    except ImportError as exc:
        try:
            import mwcc_win32_varinfo  # type: ignore[no-redef]
        except ImportError:
            raise ReducerError(
                "tools.mwcc_win32_varinfo is required for VarInfo validation"
            ) from exc
    try:
        mwcc_win32_varinfo.validate_result_schema(dict(value))
    except (TypeError, ValueError) as exc:
        raise ReducerError(f"VarInfo schema rejected: {exc}") from exc
    if value.get("function") != function or value.get("target") != function:
        _fail("VarInfo function/target does not match the bound function")
    command = value.get("command")
    if not isinstance(command, str) or str(source) not in command:
        _fail("VarInfo command is not bound to the candidate source path")
    locals_rows = value.get("locals")
    snapshots = value.get("assignment_snapshots", [])
    if not isinstance(locals_rows, list) or not isinstance(snapshots, list):
        _fail("VarInfo locals/snapshots must be arrays")
    allowed_state = ("usage", "noregister", "used", "flags", "rclass", "reg", "reg_hi")
    objects: dict[str, Any] = {}
    for name in names:
        final = [row for row in locals_rows if isinstance(row, Mapping) and row.get("name") == name]
        if len(final) != 1:
            _fail(f"VarInfo must contain exactly one final local {name!r}")
        trajectory: list[dict[str, Any]] = []
        for snapshot_index, snapshot in enumerate(snapshots):
            if not isinstance(snapshot, Mapping) or not isinstance(snapshot.get("locals"), list):
                _fail(f"VarInfo snapshot {snapshot_index} is malformed")
            matches = [
                row
                for row in snapshot["locals"]
                if isinstance(row, Mapping) and row.get("name") == name
            ]
            if len(matches) != 1:
                _fail(f"VarInfo snapshot {snapshot_index} lacks unique local {name!r}")
            trajectory.append(
                {
                    "snapshot_index": snapshot_index,
                    **{key: matches[0].get(key) for key in allowed_state},
                }
            )
        objects[name] = {
            "final": {key: final[0].get(key) for key in allowed_state},
            "assignment_trajectory": trajectory,
        }
    return {
        "schema_version": value.get("schema_version"),
        "tool": value.get("tool"),
        "compiler_sha256": value.get("compiler_sha256"),
        "capture_assignments": value.get("capture_assignments"),
        "objects": objects,
    }


def bind_case(
    *,
    observation_id: str,
    function: str,
    strict_report: Path,
    source: Path,
    names: Sequence[str],
    varinfo_report: Path | None = None,
) -> dict[str, Any]:
    """Bind one real report/source/VarInfo tuple without writing anything."""

    observation_id = _safe_id(observation_id, "observation_id")
    function = _identifier(function, "function")
    selected_names = [_identifier(name, f"names[{index}]") for index, name in enumerate(names)]
    if not selected_names or len(set(selected_names)) != len(selected_names):
        _fail("names must be a non-empty unique identifier list")
    strict_descriptor = _descriptor(strict_report, "strict report")
    source_descriptor = _descriptor(source, "source", limit=MAX_SOURCE_BYTES)
    strict_value, _ = _json_file(strict_report, "strict report")
    source_value = _source_text(source)
    objects, function_evidence = _source_objects(source_value, function, selected_names)
    varinfo_descriptor: dict[str, Any] | None = None
    varinfo: dict[str, Any] | None = None
    if varinfo_report is not None:
        varinfo_descriptor = _descriptor(varinfo_report, "VarInfo report")
        varinfo_value, _ = _json_file(varinfo_report, "VarInfo report")
        varinfo = _varinfo_evidence(varinfo_value, function, Path(os.path.abspath(source)), selected_names)
    result = {
        "schema": BOUND_SCHEMA,
        "tool_version": TOOL_VERSION,
        "diagnostic_only": True,
        "authority_advanced": False,
        "observation_id": observation_id,
        "function": function,
        "artifacts": {
            "strict_report": strict_descriptor,
            "source": source_descriptor,
            "varinfo_report": varinfo_descriptor,
        },
        "function_evidence": function_evidence,
        "strict": _strict_evidence(strict_value, function),
        "objects": objects,
        "varinfo": varinfo,
        "limitations": [
            "Bound report hashes identify observed bytes but do not authenticate retail authority.",
            "Identifier intervals prove source chronology, not semantic ownership of physical homes.",
        ],
    }
    return _seal(result, "bound_sha256")


def _validate_bound(value: Any, label: str) -> dict[str, Any]:
    row = _closed(
        value,
        allowed={
            "schema",
            "tool_version",
            "diagnostic_only",
            "authority_advanced",
            "observation_id",
            "function",
            "artifacts",
            "function_evidence",
            "strict",
            "objects",
            "varinfo",
            "limitations",
            "bound_sha256",
        },
        required={
            "schema",
            "tool_version",
            "diagnostic_only",
            "authority_advanced",
            "observation_id",
            "function",
            "artifacts",
            "function_evidence",
            "strict",
            "objects",
            "varinfo",
            "limitations",
            "bound_sha256",
        },
        label=label,
    )
    if row.get("schema") != BOUND_SCHEMA or row.get("tool_version") != TOOL_VERSION:
        _fail(f"{label} schema/tool mismatch")
    if row.get("diagnostic_only") is not True or row.get("authority_advanced") is not False:
        _fail(f"{label} policy mismatch")
    expected = _digest({key: value for key, value in row.items() if key != "bound_sha256"})
    if row.get("bound_sha256") != expected:
        _fail(f"{label} self-hash mismatch")
    _safe_id(row.get("observation_id"), f"{label}.observation_id")
    _identifier(row.get("function"), f"{label}.function")
    artifacts = _closed(
        row.get("artifacts"),
        allowed={"strict_report", "source", "varinfo_report"},
        required={"strict_report", "source", "varinfo_report"},
        label=f"{label}.artifacts",
    )
    _validate_descriptor(artifacts.get("strict_report"), f"{label}.artifacts.strict_report")
    _validate_descriptor(
        artifacts.get("source"),
        f"{label}.artifacts.source",
        limit=MAX_SOURCE_BYTES,
    )
    if artifacts.get("varinfo_report") is not None:
        _validate_descriptor(
            artifacts.get("varinfo_report"), f"{label}.artifacts.varinfo_report"
        )
    if not isinstance(row.get("objects"), Mapping):
        _fail(f"{label}.objects must be an object")
    return dict(row)


def _load_bound_descriptor(value: Any, label: str) -> tuple[dict[str, Any], dict[str, Any]]:
    descriptor, path = _validate_descriptor(value, label)
    report, _ = _json_file(path, label)
    return descriptor, _validate_bound(report, label)


def _stack_home_module() -> Any:
    try:
        from tools import capsule_stack_home_native
    except ImportError as exc:
        try:
            import capsule_stack_home_native  # type: ignore[no-redef]
        except ImportError:
            raise ReducerError(
                "tools.capsule_stack_home_native is required; install the generic "
                "stack-home producer first"
            ) from exc
    return capsule_stack_home_native


def _compose_stack_home(
    value: Any,
    *,
    bound: Mapping[str, Any],
    matrix_object: str | None,
    module: Any | None = None,
) -> dict[str, Any]:
    row = _closed(
        value,
        allowed={"packet", "summary"},
        required={"packet", "summary"},
        label="stack_home",
    )
    packet_descriptor, packet_path = _validate_descriptor(row.get("packet"), "stack_home.packet")
    summary_descriptor, summary_path = _validate_descriptor(row.get("summary"), "stack_home.summary")
    producer = module or _stack_home_module()
    try:
        packet = producer.validate_packet(packet_path)
    except Exception as exc:
        raise ReducerError(f"generic stack-home packet rejected: {exc}") from exc
    summary, _ = _json_file(summary_path, "stack_home.summary")
    summary_hash = summary.get("summary_sha256")
    expected_summary_hash = producer.canonical_hash(
        {key: item for key, item in summary.items() if key != "summary_sha256"}
    )
    if summary_hash != expected_summary_hash:
        _fail("generic stack-home summary self-hash mismatch")
    names = summary.get("requested_names")
    if not isinstance(names, list) or not all(isinstance(name, str) for name in names):
        _fail("generic stack-home summary requested_names is malformed")
    try:
        expected_summary = producer._summarize_validated_packet(packet, packet_path, names)
    except Exception as exc:
        raise ReducerError(f"generic stack-home summary composition failed: {exc}") from exc
    if summary != expected_summary:
        _fail("generic stack-home summary is not the deterministic summary of its packet")
    if summary.get("authority_advanced") is not False:
        _fail("generic stack-home summary advanced authority")
    source_sha = bound["artifacts"]["source"]["sha256"]
    binding = summary.get("binding")
    if not isinstance(binding, Mapping) or binding.get("source_sha256") != source_sha:
        _fail("generic stack-home summary source hash is not bound to the observation")
    mappings = summary.get("mappings")
    if not isinstance(mappings, list):
        _fail("generic stack-home summary mappings are malformed")
    projection = [
        {
            "name": mapping.get("name"),
            "object_token": mapping.get("object_token"),
            "varinfo_token": mapping.get("varinfo_token"),
            "home_value": mapping.get("varinfo_home_snapshot", {}).get("home_value")
            if isinstance(mapping.get("varinfo_home_snapshot"), Mapping)
            else None,
            "mapped_slots": mapping.get("mapped_slots"),
            "owner": mapping.get("owner"),
        }
        for mapping in mappings
        if isinstance(mapping, Mapping)
    ]
    writes = packet.get("events")
    if not isinstance(writes, list):
        _fail("generic packet events are malformed")
    pre_writes = sorted(
        [
            event
            for event in writes
            if isinstance(event, Mapping)
            and event.get("event_kind") == "object_stack_write_pre"
        ],
        key=lambda event: int(event["sequence"]),
    )
    names_by_token = {
        mapping.get("object_token"): mapping.get("name")
        for mapping in mappings
        if isinstance(mapping, Mapping)
    }

    def frontend_event(event: Mapping[str, Any] | None) -> dict[str, Any] | None:
        if event is None:
            return None
        return {
            "sequence": event.get("sequence"),
            "object_token": event.get("object_token"),
            "name": names_by_token.get(event.get("object_token"), "UNKNOWN"),
            "target_slot": event.get("target_slot"),
            "status": "AUTHENTIC_FRONTEND_STACK_WRITE",
        }

    earliest_frontend = frontend_event(pre_writes[0] if pre_writes else None)
    lowest_home = frontend_event(
        min(
            pre_writes,
            key=lambda event: (int(event["target_slot"]), int(event["sequence"])),
        )
        if pre_writes
        else None
    )
    matrix_event: dict[str, Any] | None = None
    if matrix_object is not None:
        matrix_matches = [mapping for mapping in mappings if isinstance(mapping, Mapping) and mapping.get("name") == matrix_object]
        if len(matrix_matches) != 1:
            _fail(f"generic summary lacks unique matrix object {matrix_object!r}")
        matrix = matrix_matches[0]
        matrix_token = matrix.get("object_token")
        matrix_writes = [event for event in pre_writes if event.get("object_token") == matrix_token]
        if not matrix_writes:
            _fail(f"generic packet lacks matrix stack write for {matrix_object!r}")
        first = matrix_writes[0]
        predecessors = [event for event in pre_writes if int(event["sequence"]) < int(first["sequence"])]
        predecessor = predecessors[-1] if predecessors else None
        home_snapshot = matrix.get("varinfo_home_snapshot")
        slots = matrix.get("mapped_slots")
        if not isinstance(home_snapshot, Mapping) or not isinstance(slots, list) or not slots:
            _fail("generic matrix mapping lacks home/slot evidence")
        home_value = home_snapshot.get("home_value")
        if isinstance(home_value, bool) or not isinstance(home_value, int) or not all(isinstance(slot, int) and not isinstance(slot, bool) for slot in slots):
            _fail("generic matrix home/slots are not integers")
        base_shift = min(slots) - home_value
        reservation = predecessor.get("target_slot") if predecessor is not None else None
        matrix_event = {
            "matrix_object": matrix_object,
            "matrix_pre_sequence": first["sequence"],
            "pre_matrix_reservation": reservation,
            "pre_matrix_object_token": predecessor.get("object_token") if predecessor is not None else None,
            "varinfo_home_value": home_value,
            "mapped_base": min(slots),
            "base_shift": base_shift,
            "target_pattern_0x34_plus_0x8": reservation == 0x34 and base_shift == 0x8,
        }
    return {
        "packet": packet_descriptor,
        "summary": summary_descriptor,
        "summary_sha256": summary_hash,
        "mappings": projection,
        "earliest_frontend_stack_object_event": earliest_frontend,
        "lowest_home_stack_object_event": lowest_home,
        "matrix_lifetime_event": matrix_event,
        "authority_advanced": False,
    }


def _comparison(before: Mapping[str, Any], after: Mapping[str, Any]) -> dict[str, Any]:
    before_strict = before["strict"]
    after_strict = after["strict"]
    before_source = before["artifacts"]["source"]["sha256"]
    after_source = after["artifacts"]["source"]["sha256"]
    before_report = before["artifacts"]["strict_report"]["sha256"]
    after_report = after["artifacts"]["strict_report"]["sha256"]
    if before_report == after_report:
        difference_class = "source_only_no_object_effect"
        home_only = True
        semantic = False
    elif before_strict["source_instruction_shape_sha256"] == after_strict["source_instruction_shape_sha256"]:
        difference_class = "home_only"
        home_only = True
        semantic = False
    else:
        difference_class = "semantic_or_topology"
        home_only = False
        semantic = True
    return {
        "before": before["observation_id"],
        "after": after["observation_id"],
        "source_changed": before_source != after_source,
        "strict_report_changed": before_report != after_report,
        "source_frame_delta": after_strict["source_frame_size"] - before_strict["source_frame_size"],
        "target_gap_before": abs(before_strict["frame_size_delta_target_minus_source"]),
        "target_gap_after": abs(after_strict["frame_size_delta_target_minus_source"]),
        "difference_class": difference_class,
        "home_only": home_only,
        "semantic_difference": semantic,
    }


def _first_use(bound: Mapping[str, Any], object_name: str | None) -> dict[str, Any] | None:
    if object_name is None:
        return None
    objects = bound.get("objects")
    row = objects.get(object_name) if isinstance(objects, Mapping) else None
    interval = row.get("earliest_source_use_interval") if isinstance(row, Mapping) else None
    if not isinstance(interval, Mapping):
        return None
    return {
        "observation_id": bound["observation_id"],
        "object": object_name,
        **dict(interval),
    }


def _lexical_scope_experiments(
    bounds: Sequence[Mapping[str, Any]], object_names: Sequence[str | None]
) -> list[dict[str, Any]]:
    experiments: list[dict[str, Any]] = []
    for index in range(1, len(bounds)):
        before = bounds[index - 1]
        after = bounds[index]
        comparison = _comparison(before, after)
        for object_name in object_names:
            if object_name is None:
                continue
            before_object = before["objects"].get(object_name)
            after_object = after["objects"].get(object_name)
            if not isinstance(before_object, Mapping) or not isinstance(after_object, Mapping):
                continue
            before_depth = before_object.get("declaration", {}).get("scope_depth")
            after_depth = after_object.get("declaration", {}).get("scope_depth")
            if before_depth == after_depth:
                continue
            no_object_effect = comparison["difference_class"] == "source_only_no_object_effect"
            experiments.append(
                {
                    "object": object_name,
                    "before": before["observation_id"],
                    "after": after["observation_id"],
                    "scope_depth_before": before_depth,
                    "scope_depth_after": after_depth,
                    "difference_class": comparison["difference_class"],
                    "status": (
                        "NO_GO_NO_OBJECT_EFFECT"
                        if no_object_effect
                        else "OBSERVED_NOT_DISPOSITIVE"
                    ),
                    "no_go": no_object_effect,
                }
            )
    return experiments


def _derive(
    bounds: Sequence[Mapping[str, Any]],
    *,
    motion_object: str | None,
    allocation_object: str | None,
    matrix_object: str | None,
    stack_homes: Mapping[str, Mapping[str, Any]],
) -> tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]:
    final = bounds[-1]
    comparisons = [_comparison(bounds[index - 1], bounds[index]) for index in range(1, len(bounds))]
    final_objects = final["objects"]
    scope_experiments = _lexical_scope_experiments(
        bounds, (motion_object, allocation_object, matrix_object)
    )
    scope_no_go_objects = {
        row["object"] for row in scope_experiments if row["no_go"] is True
    }
    motion = final_objects.get(motion_object) if motion_object is not None else None
    motion_stride = None
    if isinstance(motion, Mapping) and isinstance(motion.get("outer_stride_bytes"), int):
        motion_stride = {
            "object": motion_object,
            "bytes": motion["outer_stride_bytes"],
            "status": "PROVEN_SOURCE_DECLARATION",
            "observation_id": final["observation_id"],
            "declaration_sha256": motion["declaration"]["statement_sha256"],
        }
    allocation = final_objects.get(allocation_object) if allocation_object is not None else None
    gap = abs(int(final["strict"]["frame_size_delta_target_minus_source"]))
    allocation_class = None
    if isinstance(allocation, Mapping) and allocation.get("extent_bytes") == gap and gap > 0:
        allocation_class = {
            "object": allocation_object,
            "bytes": gap,
            "status": "REMAINING_BOUND_CLASS",
            "observation_id": final["observation_id"],
            "declaration_sha256": allocation["declaration"]["statement_sha256"],
        }
    selected_use_object = matrix_object or allocation_object or motion_object
    earliest = _first_use(final, selected_use_object)
    matrix_projection = stack_homes.get(final["observation_id"], {}).get("matrix_lifetime_event")
    frontend_projection = stack_homes.get(final["observation_id"], {}).get(
        "earliest_frontend_stack_object_event"
    )
    lowest_home_projection = stack_homes.get(final["observation_id"], {}).get(
        "lowest_home_stack_object_event"
    )
    capacity_ties: list[dict[str, Any]] = []
    for object_name in (motion_object, allocation_object, matrix_object):
        obj = final_objects.get(object_name) if object_name is not None else None
        ties = obj.get("symbolic_capacity_ties") if isinstance(obj, Mapping) else None
        if not isinstance(ties, list):
            continue
        capacity_ties.extend(
            {"object": object_name, **dict(tie)}
            for tie in ties
            if isinstance(tie, Mapping)
        )
    authentic_event = None
    if isinstance(matrix_projection, Mapping) and matrix_projection.get("target_pattern_0x34_plus_0x8") is True:
        authentic_event = {
            "classification": "scoped_aggregate_coalescing_with_outgoing_call_area",
            "object": matrix_object,
            "pre_matrix_reservation": 0x34,
            "base_shift": 0x8,
            "earliest_source_use_interval": earliest,
            "strict_difference_class": final["strict"]["difference_class"],
            "status": "AUTHENTIC_SOURCE_LIFETIME_EVENT",
        }
    conclusions = {
        "proven_motion_stride": motion_stride,
        "remaining_allocation_class": allocation_class,
        "earliest_source_use_interval": earliest,
        "difference_classes": {
            "observations": [bound["strict"]["difference_class"] for bound in bounds],
            "comparisons": [row["difference_class"] for row in comparisons],
        },
        "lexical_scope_experiments": scope_experiments,
        "lexical_scope_no_go": sorted(scope_no_go_objects),
        "symbolic_capacity_ties": capacity_ties,
        "earliest_frontend_stack_object_event": frontend_projection,
        "lowest_home_stack_object_event": lowest_home_projection,
        "authentic_lifetime_event": authentic_event,
    }
    axes: list[dict[str, Any]] = []
    if authentic_event is not None and earliest is not None:
        if matrix_object not in scope_no_go_objects:
            axes.append(
                {
                    "id": "scope-existing-aggregate-at-first-use",
                    "priority": 10,
                    "admissibility": "natural",
                    "source_action": f"Move the existing {matrix_object} declaration to the narrowest scope containing its first and last semantic uses.",
                    "evidence": [
                        f"source-use:{earliest['interval_sha256']}",
                        "stack-home:pre-matrix-0x34",
                        "stack-home:base-shift-0x8",
                    ],
                }
            )
        axes.append(
            {
                "id": "test-existing-aggregate-outgoing-area-coalescing",
                "priority": 20,
                "admissibility": "natural",
                "source_action": "Test the existing aggregate's lifetime against the already present outgoing call area without adding storage.",
                "evidence": ["stack-home:pre-matrix-0x34", "strict:home-only"],
            }
        )
    for tie in capacity_ties:
        axes.append(
            {
                "id": f"preserve-{tie['object']}-four-player-capacity",
                "priority": 15,
                "admissibility": "natural",
                "source_action": f"Preserve {tie['object']} as {tie['symbol']}; its four-player capacity is source-authenticated.",
                "evidence": [
                    f"capacity-symbol:{tie['symbol']}",
                    f"capacity-elements:{tie['elements']}",
                ],
            }
        )
    if (
        allocation_class is not None
        and earliest is not None
        and allocation_object not in scope_no_go_objects
    ):
        axes.append(
            {
                "id": "narrow-existing-allocation-class-scope",
                "priority": 30,
                "admissibility": "natural",
                "source_action": f"Scope the existing {allocation_object} aggregate at its earliest real use; retain its proven {allocation_class['bytes']}-byte type.",
                "evidence": [
                    f"source-use:{earliest['interval_sha256']}",
                    f"allocation-class:{allocation_class['bytes']}",
                ],
            }
        )
    elif allocation_class is not None and earliest is not None:
        axes.append(
            {
                "id": "preserve-existing-allocation-scope-test-chronology",
                "priority": 30,
                "admissibility": "natural",
                "source_action": f"Preserve the tested {allocation_object} lexical scope and {allocation_class['bytes']}-byte type; test only real declaration/use chronology.",
                "evidence": [
                    f"source-use:{earliest['interval_sha256']}",
                    "scope:NO_GO_NO_OBJECT_EFFECT",
                ],
            }
        )
    axes.sort(key=lambda row: (int(row["priority"]), str(row["id"])))
    forbidden = [
        {
            "id": "dead-local-or-padding",
            "reason": "Unauthenticated dead storage or padding is register/stack shaping, not recovered source.",
        },
        {
            "id": "register-volatile-shaping",
            "reason": "register/volatile qualifiers used only to steer code generation are forbidden.",
        },
        {
            "id": "fake-use-or-dead-branch",
            "reason": "Fake reads, writes, or unreachable branches cannot authenticate a lifetime.",
        },
    ]
    return conclusions, axes, forbidden


def reduce_request(path: Path, *, module: Any | None = None) -> dict[str, Any]:
    request, raw = _json_file(path, "reducer request", limit=1024 * 1024)
    request = _closed(
        request,
        allowed={"schema", "case_id", "function", "focus", "bound_reports", "stack_homes"},
        required={"schema", "case_id", "function", "focus", "bound_reports", "stack_homes"},
        label="reducer request",
    )
    if request.get("schema") != REQUEST_SCHEMA:
        _fail(f"reducer request schema must be {REQUEST_SCHEMA}")
    case_id = _safe_id(request.get("case_id"), "case_id")
    function = _identifier(request.get("function"), "function")
    focus = _closed(
        request.get("focus"),
        allowed={"motion_object", "allocation_object", "matrix_object"},
        required={"motion_object", "allocation_object", "matrix_object"},
        label="focus",
    )
    parsed_focus: dict[str, str | None] = {}
    for key in ("motion_object", "allocation_object", "matrix_object"):
        value = focus.get(key)
        parsed_focus[key] = None if value is None else _identifier(value, f"focus.{key}")
    raw_bounds = request.get("bound_reports")
    if not isinstance(raw_bounds, list) or not 1 <= len(raw_bounds) <= MAX_OBSERVATIONS:
        _fail(f"bound_reports must contain 1-{MAX_OBSERVATIONS} descriptors")
    bounds: list[dict[str, Any]] = []
    bound_descriptors: list[dict[str, Any]] = []
    for index, value in enumerate(raw_bounds):
        descriptor, bound = _load_bound_descriptor(value, f"bound_reports[{index}]")
        if bound["function"] != function:
            _fail(f"bound_reports[{index}] function mismatch")
        bounds.append(bound)
        bound_descriptors.append(descriptor)
    observation_ids = [bound["observation_id"] for bound in bounds]
    if len(set(observation_ids)) != len(observation_ids):
        _fail("bound report observation ids must be unique")
    raw_homes = request.get("stack_homes")
    if not isinstance(raw_homes, list) or not raw_homes:
        _fail("stack_homes must contain at least one generic packet/summary composition")
    by_id = {bound["observation_id"]: bound for bound in bounds}
    stack_homes: dict[str, dict[str, Any]] = {}
    for index, value in enumerate(raw_homes):
        home = _closed(
            value,
            allowed={"observation_id", "packet", "summary"},
            required={"observation_id", "packet", "summary"},
            label=f"stack_homes[{index}]",
        )
        observation_id = _safe_id(home.get("observation_id"), f"stack_homes[{index}].observation_id")
        if observation_id not in by_id or observation_id in stack_homes:
            _fail(f"stack_homes[{index}] has unknown or duplicate observation_id")
        stack_homes[observation_id] = _compose_stack_home(
            {"packet": home["packet"], "summary": home["summary"]},
            bound=by_id[observation_id],
            matrix_object=parsed_focus["matrix_object"],
            module=module,
        )
    conclusions, axes, forbidden = _derive(
        bounds,
        motion_object=parsed_focus["motion_object"],
        allocation_object=parsed_focus["allocation_object"],
        matrix_object=parsed_focus["matrix_object"],
        stack_homes=stack_homes,
    )
    result = {
        "schema": REPORT_SCHEMA,
        "tool_version": TOOL_VERSION,
        "status": STATUS,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "case_id": case_id,
        "function": function,
        "request": {
            "path": str(Path(os.path.abspath(path))),
            "size": len(raw),
            "sha256": hashlib.sha256(raw).hexdigest(),
        },
        "focus": parsed_focus,
        "bound_reports": [
            {
                "observation_id": bound["observation_id"],
                "descriptor": descriptor,
                "bound_sha256": bound["bound_sha256"],
                "artifacts": bound["artifacts"],
            }
            for bound, descriptor in zip(bounds, bound_descriptors)
        ],
        "stack_homes": stack_homes,
        "observations": [
            {
                "observation_id": bound["observation_id"],
                "strict": bound["strict"],
                "objects": bound["objects"],
                "varinfo": bound["varinfo"],
            }
            for bound in bounds
        ],
        "comparisons": [_comparison(bounds[index - 1], bounds[index]) for index in range(1, len(bounds))],
        "conclusions": conclusions,
        "ranked_natural_c_axes": axes,
        "forbidden_axes": forbidden,
        "limitations": [
            "Stack homes and source intervals are diagnostic chronology, not proof of original ownership.",
            "Ranked axes require an independent compile/object/relocation/linked-retail proof transaction.",
        ],
    }
    return _seal(result, "report_sha256")


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    commands = result.add_subparsers(dest="command", required=True)
    bind = commands.add_parser("bind", help="print one compact bound report")
    bind.add_argument("--observation-id", required=True)
    bind.add_argument("--function", required=True)
    bind.add_argument("--strict-report", type=Path, required=True)
    bind.add_argument("--source", type=Path, required=True)
    bind.add_argument("--varinfo-report", type=Path)
    bind.add_argument("--name", action="append", required=True)
    reduce = commands.add_parser("reduce", help="compose bounds with generic stack-home evidence")
    reduce.add_argument("request", type=Path)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "bind":
            result = bind_case(
                observation_id=args.observation_id,
                function=args.function,
                strict_report=args.strict_report,
                source=args.source,
                names=args.name,
                varinfo_report=args.varinfo_report,
            )
        else:
            result = reduce_request(args.request)
    except (OSError, ReducerError) as exc:
        print(f"stack object/lifetime reducer: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
