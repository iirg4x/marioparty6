#!/usr/bin/env python3
"""Compact current-source evidence index. No compile, history scan or retention gate.

Publish explicitly selected reports once, then verify the small index on resume.
Instruction exactness is not physical/link exactness. A supplied report is a
diagnostic input, not proof that its object was built from the supplied source.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import tempfile
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import focus_symbol_report as focus

SCHEMA = "recovery_current_evidence/v1"
INDEX_LIMIT = 256 * 1024
REPORT_LIMIT = 32 * 1024 * 1024
ACCESS_SCHEMA = "recovery_accesses/v1"
STACK_MAP_SCHEMA = "recovery_stack_map/v1"
BRANCH_MAP_SCHEMA = "recovery_branch_map/v1"
DIAGNOSE_SCHEMA = "recovery_source_diagnosis/v1"
DIAGNOSE_OUTPUT_LIMIT = 32 * 1024
DIAGNOSE_FOCUS_LIMIT = 8 * 1024
VARINFO_LIMIT = INDEX_LIMIT
DEFAULT_ACCESS_MATCH_LIMIT = 64
MAX_ACCESS_MATCH_LIMIT = 256
MAX_ACCESS_CONTEXT = 3
ACCESS_TEXT_LIMIT = 512

_OFFSET_RE = re.compile(r"[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+)\Z")
_REGISTER_RE = re.compile(r"r(?:[0-9]|[12][0-9]|3[01])\Z", re.IGNORECASE)
# This intentionally recognizes only the canonical numeric displacement(base)
# spelling.  In particular, the boundaries prevent finding 0x88 inside 0x188
# or inside a symbolic expression such as foo+0x88(r1).
_MEMORY_OPERAND_RE = re.compile(
    r"(?<![A-Za-z0-9_.+-])"
    r"(?P<displacement>[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+))"
    r"[ \t]*\([ \t]*(?P<base>r(?:[0-9]|[12][0-9]|3[01]))[ \t]*\)"
    r"(?![A-Za-z0-9_.+-])",
    re.IGNORECASE,
)
_OPCODE_RE = re.compile(r"^\s*(?P<opcode>[A-Za-z][A-Za-z0-9_.]*)\b", re.IGNORECASE)
_ADDI_POINTER_RE = re.compile(
    r"^\s*(?P<opcode>addi)\s+"
    r"(?P<destination>r(?:[0-9]|[12][0-9]|3[01]))\s*,\s*"
    r"r1\s*,\s*(?P<displacement>[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+))\s*\Z",
    re.IGNORECASE,
)
_LINK_BRANCHES = frozenset({"bl", "bla", "blr", "blrl", "bcl", "bcla", "bclr",
                            "bcctr", "bctrl"})
_REGISTER_TOKEN_RE = re.compile(
    r"(?<![A-Za-z0-9_])(?P<register>[rf](?:[0-9]|[12][0-9]|3[01]))(?![A-Za-z0-9_])",
    re.IGNORECASE,
)
_SHA256_RE = re.compile(r"[0-9a-fA-F]{64}\Z")
_NUMBER_TOKEN_RE = re.compile(
    r"(?<![A-Za-z0-9_.])(?P<number>[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+))(?![A-Za-z0-9_.])"
)


def canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def local(root: Path, path: Path) -> Path:
    path = Path(os.path.abspath(root / path))
    path.relative_to(root)
    for part in (path, *path.parents):
        if part.is_symlink() or (hasattr(part, "is_junction") and part.is_junction()):
            raise ValueError(f"indirected evidence path: {part}")
        if part == root:
            break
    return path


def read_bound(root: Path, path: Path, limit: int) -> tuple[bytes, dict]:
    path = local(root, path)
    with path.open("rb") as stream:
        raw = stream.read(limit + 1)
    if len(raw) > limit:
        raise ValueError(f"evidence exceeds {limit} bytes: {path}")
    return raw, {"path": path.relative_to(root).as_posix(), "size_bytes": len(raw),
                 "sha256": hashlib.sha256(raw).hexdigest()}


def load_json(raw: bytes) -> Any:
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"duplicate JSON key: {key}")
            result[key] = value
        return result
    return json.loads(raw, object_pairs_hook=unique)


def _parse_access_offset(value: Any) -> int:
    if isinstance(value, bool):
        raise ValueError("offset must be a decimal or hexadecimal integer")
    if isinstance(value, int):
        return value
    if not isinstance(value, str):
        raise ValueError("offset must be a decimal or hexadecimal integer")
    text = value.strip()
    if _OFFSET_RE.fullmatch(text) is None:
        raise ValueError("offset must be a decimal or hexadecimal integer")
    sign = -1 if text.startswith("-") else 1
    digits = text[1:] if text[:1] in {"+", "-"} else text
    base = 16 if digits.lower().startswith("0x") else 10
    try:
        return sign * int(digits[2:] if base == 16 else digits, base)
    except ValueError as exc:
        raise ValueError("offset must be a decimal or hexadecimal integer") from exc


def _parse_access_register(value: Any) -> str:
    if not isinstance(value, str):
        raise ValueError("base register must be a PowerPC GPR r0-r31")
    register = value.strip().lower()
    if _REGISTER_RE.fullmatch(register) is None:
        raise ValueError("base register must be a PowerPC GPR r0-r31")
    return register


def _validate_access_context(value: Any) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= MAX_ACCESS_CONTEXT:
        raise ValueError(f"context must be between 0 and {MAX_ACCESS_CONTEXT}")
    return value


def _validate_access_limit(value: Any) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not 1 <= value <= MAX_ACCESS_MATCH_LIMIT:
        raise ValueError(f"max_matches must be between 1 and {MAX_ACCESS_MATCH_LIMIT}")
    return value


def _access_row(row: dict, index: int) -> dict[str, Any] | None:
    instruction_value = row.get("instruction")
    if instruction_value is None:
        # Alignment placeholders in objdiff rows have no formatted instruction
        # and cannot be a memory access or useful textual context.
        return None
    if not isinstance(instruction_value, dict):
        raise ValueError(f"strict.instructions[{index}].instruction must be an object")
    formatted = instruction_value.get("formatted")
    if formatted is None:
        return None
    if not isinstance(formatted, str):
        raise ValueError(f"strict.instructions[{index}].instruction.formatted must be text")
    if len(formatted) > ACCESS_TEXT_LIMIT:
        raise ValueError(f"strict.instructions[{index}] formatted text exceeds {ACCESS_TEXT_LIMIT} characters")
    diff_kind = row.get("diff_kind")
    if diff_kind is not None and not isinstance(diff_kind, str):
        raise ValueError(f"strict.instructions[{index}].diff_kind must be text or null")
    address = instruction_value.get("address")
    if address is not None and (isinstance(address, bool) or not isinstance(address, (int, float, str))):
        raise ValueError(f"strict.instructions[{index}].instruction.address must be scalar")
    return {
        "row_index": index,
        "instruction_address": address,
        "text": formatted,
        "diff_kind": diff_kind,
    }


def _access_matches(text: str, base_register: str, offset: int) -> bool:
    for match in _MEMORY_OPERAND_RE.finditer(text):
        if match.group("base").lower() != base_register:
            continue
        if _parse_access_offset(match.group("displacement")) == offset:
            return True
    return False


def accesses(
    *,
    root: Path,
    strict: Path,
    function: str,
    side: str,
    base_register: str,
    offset: Any,
    context: int,
    max_matches: int = DEFAULT_ACCESS_MATCH_LIMIT,
) -> dict[str, Any]:
    """Return bounded formatted-instruction accesses from one objdiff side."""
    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function.strip():
        raise ValueError("function must be nonempty text")
    function = function.strip()
    if side not in {"target", "candidate"}:
        raise ValueError("side must be target or candidate")
    base_register = _parse_access_register(base_register)
    offset = _parse_access_offset(offset)
    context = _validate_access_context(context)
    max_matches = _validate_access_limit(max_matches)

    raw, report_binding = read_bound(root, Path(strict), REPORT_LIMIT)
    document = load_json(raw)
    if not isinstance(document, dict):
        raise ValueError("strict report must be a JSON object")
    # Validate both canonical channels, while only retaining the requested
    # function's rows.  No relocation, symbol-owner or semantic identity is
    # inferred from the numeric displacement.
    left = focus._symbols(document, "left", "strict")
    right = focus._symbols(document, "right", "strict")
    symbols = left if side == "target" else right
    functions = [symbol for symbol in symbols if focus._is_function(symbol)]
    selected = [symbol for symbol in functions if symbol.get("name") == function]
    if len(selected) != 1:
        raise ValueError(
            f"strict report must contain exactly one {side} function {function!r}; found {len(selected)}"
        )
    rows = focus._rows(selected[0], f"strict.{side}.{function}")
    compact_rows: dict[int, dict[str, Any]] = {}
    matching_indices: list[int] = []
    for index, row in enumerate(rows):
        compact = _access_row(row, index)
        if compact is None:
            continue
        compact_rows[index] = compact
        if _access_matches(compact["text"], base_register, offset):
            matching_indices.append(index)
    del raw, document, left, right

    selected_indices = matching_indices[:max_matches]
    selected_set = set(selected_indices)
    context_rows: dict[int, dict[str, Any]] = {}
    for index in selected_indices:
        start = max(0, index - context)
        stop = min(len(rows), index + context + 1)
        for adjacent in range(start, stop):
            if adjacent == index or adjacent in selected_set:
                continue
            compact = compact_rows.get(adjacent)
            if compact is not None:
                context_rows.setdefault(adjacent, compact)

    result: dict[str, Any] = {
        "schema": ACCESS_SCHEMA,
        "schema_version": 1,
        "report": dict(report_binding),
        "report_sha256": report_binding["sha256"],
        "function": function,
        "side": side,
        "base_register": base_register,
        "offset": offset,
        "context": context,
        "max_matches": max_matches,
        "match_count": len(matching_indices),
        "matches": [compact_rows[index] for index in selected_indices],
        "context_rows": [context_rows[index] for index in sorted(context_rows)],
        "truncated": len(selected_indices) < len(matching_indices),
        "diagnostic_only": True,
        "authority_advanced": False,
    }

    # A caller may request 256 matches, but a diagnostic response must remain
    # below the compact-index budget.  Drop trailing context first, then later
    # matches, and make the bounded result explicit via truncated.
    while len(canonical(result)) + 1 > INDEX_LIMIT and result["context_rows"]:
        result["context_rows"].pop()
        result["truncated"] = True
    while len(canonical(result)) + 1 > INDEX_LIMIT and result["matches"]:
        result["matches"].pop()
        result["truncated"] = True
    if len(canonical(result)) + 1 > INDEX_LIMIT:
        raise ValueError("accesses result exceeds 256 KiB")
    return result


def _stack_instruction(text: Any, index: int) -> dict[str, Any] | None:
    if text is None:
        return None
    if not isinstance(text, str):
        raise ValueError(f"strict.instructions[{index}] formatted text must be text")
    if len(text) > ACCESS_TEXT_LIMIT:
        raise ValueError(f"strict.instructions[{index}] formatted text exceeds {ACCESS_TEXT_LIMIT} characters")
    opcode_match = _OPCODE_RE.match(text)
    if opcode_match is None:
        return None
    opcode = opcode_match.group("opcode").lower()
    pointer = _ADDI_POINTER_RE.fullmatch(text)
    if pointer is not None:
        return {
            "kind": "addi_pointer",
            "opcode": opcode,
            "operands": f"{pointer.group('destination').lower()}, r1",
            "offset": _parse_access_offset(pointer.group("displacement")),
        }
    memory = [match for match in _MEMORY_OPERAND_RE.finditer(text)
              if match.group("base").lower() == "r1"]
    if len(memory) != 1:
        return None
    match = memory[0]
    masked = text[:match.start()] + "<disp>(r1)" + text[match.end():]
    return {
        "kind": "d_form",
        "opcode": opcode,
        "operands": " ".join(masked[opcode_match.end():].split()).lower(),
        "offset": _parse_access_offset(match.group("displacement")),
    }


def _stack_rows(rows: list[dict], kind: str) -> tuple[list[dict[str, Any]], int]:
    parsed: list[dict[str, Any]] = []
    for index, row in enumerate(rows):
        compact = _access_row(row, index)
        if compact is None:
            continue
        item = _stack_instruction(compact["text"], index)
        if item is not None and item["kind"] == kind:
            parsed.append({**item, "row_index": index})
    return parsed, len(parsed)


def _stack_offset_hex(offset: int) -> str:
    return f"-0x{abs(offset):x}" if offset < 0 else f"0x{offset:x}"


def _stack_category(target_rows: list[dict], candidate_rows: list[dict], kind: str) -> dict[str, Any]:
    target, _ = _stack_rows(target_rows, kind)
    candidate, _ = _stack_rows(candidate_rows, kind)
    by_index = {item["row_index"]: item for item in candidate}
    observations: dict[tuple[str, str, int, int], dict[str, Any]] = {}
    target_links: dict[tuple[str, str, int], set[int]] = {}
    candidate_links: dict[tuple[str, str, int], set[int]] = {}
    for item in target:
        other = by_index.get(item["row_index"])
        if other is None or (item["opcode"], item["operands"]) != (other["opcode"], other["operands"]):
            continue
        signature = (item["opcode"], item["operands"])
        key = (*signature, item["offset"], other["offset"])
        entry = observations.setdefault(key, {"count": 0, "exemplars": []})
        entry["count"] += 1
        if len(entry["exemplars"]) < 3:
            entry["exemplars"].append({"row_index": item["row_index"], "opcode": item["opcode"]})
        target_links.setdefault((*signature, item["offset"]), set()).add(other["offset"])
        candidate_links.setdefault((*signature, other["offset"]), set()).add(item["offset"])
    pairs = []
    for (opcode, operands, target_offset, candidate_offset), entry in observations.items():
        ambiguous = (len(target_links[(opcode, operands, target_offset)]) > 1
                     or len(candidate_links[(opcode, operands, candidate_offset)]) > 1)
        pairs.append({
            "opcode": opcode,
            "operands": operands,
            "target_offset": target_offset,
            "target_offset_hex": _stack_offset_hex(target_offset),
            "candidate_offset": candidate_offset,
            "candidate_offset_hex": _stack_offset_hex(candidate_offset),
            "status": "ambiguous_one_to_many" if ambiguous else (
                "equal" if target_offset == candidate_offset else "changed"
            ),
            "count": entry["count"],
            "exemplars": entry["exemplars"],
        })
    pairs.sort(key=lambda pair: (pair["opcode"], pair["operands"],
                                 pair["target_offset"], pair["candidate_offset"]))
    paired_rows = sum(entry["count"] for entry in observations.values())
    return {
        "target_access_count": len(target),
        "candidate_access_count": len(candidate),
        "paired_rows": paired_rows,
        "unpaired_target_access_count": len(target) - paired_rows,
        "unpaired_candidate_access_count": len(candidate) - paired_rows,
        "pair_count": len(pairs),
        "pairs": pairs,
    }


def _stack_function(symbols: list, name: str, side: str) -> dict | None:
    matches = [symbol for symbol in symbols
               if focus._is_function(symbol) and symbol.get("name") == name]
    if len(matches) > 1:
        raise ValueError(f"ambiguous {side} function {name!r}")
    return matches[0] if matches else None


def stack_map(*, root: Path, strict: Path, function: str) -> dict[str, Any]:
    """Summarize aligned r1 stack accesses without assigning semantic owners."""
    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function.strip():
        raise ValueError("function must be nonempty text")
    function = function.strip()
    raw, report_binding = read_bound(root, Path(strict), REPORT_LIMIT)
    document = load_json(raw)
    if not isinstance(document, dict):
        raise ValueError("strict report must be a JSON object")
    left = focus._symbols(document, "left", "strict")
    right = focus._symbols(document, "right", "strict")
    target_symbol = _stack_function(left, function, "target")
    candidate_symbol = _stack_function(right, function, "candidate")
    result: dict[str, Any] = {
        "schema": STACK_MAP_SCHEMA,
        "schema_version": 1,
        "report": dict(report_binding),
        "report_sha256": report_binding["sha256"],
        "function": function,
        "status": "ok" if target_symbol is not None and candidate_symbol is not None else "missing_symbol",
        "missing_sides": ([side for side, symbol in (("target", target_symbol), ("candidate", candidate_symbol))
                           if symbol is None]),
        "base_register": "r1",
        "d_form": _stack_category(
            focus._rows(target_symbol, f"strict.target.{function}") if target_symbol is not None else [],
            focus._rows(candidate_symbol, f"strict.candidate.{function}") if candidate_symbol is not None else [],
            "d_form",
        ),
        "addi_pointer": _stack_category(
            focus._rows(target_symbol, f"strict.target.{function}") if target_symbol is not None else [],
            focus._rows(candidate_symbol, f"strict.candidate.{function}") if candidate_symbol is not None else [],
            "addi_pointer",
        ),
        "truncated": False,
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    while len(canonical(result)) + 1 > INDEX_LIMIT:
        category = max((result["d_form"], result["addi_pointer"]), key=lambda item: len(item["pairs"]))
        if not category["pairs"]:
            raise ValueError("stack-map result exceeds 256 KiB")
        category.setdefault("returned_pair_count", len(category["pairs"]))
        drop = max(1, len(category["pairs"]) // 2)
        del category["pairs"][-drop:]
        category["returned_pair_count"] = len(category["pairs"])
        result["truncated"] = True
    return result


def _branch_instruction(text: Any, index: int, branch_dest: Any = None) -> dict[str, Any] | None:
    if text is None:
        return None
    if not isinstance(text, str):
        raise ValueError(f"strict.instructions[{index}] formatted text must be text")
    if len(text) > ACCESS_TEXT_LIMIT:
        raise ValueError(f"strict.instructions[{index}] formatted text exceeds {ACCESS_TEXT_LIMIT} characters")
    opcode_match = _OPCODE_RE.match(text)
    if opcode_match is None:
        return None
    opcode = opcode_match.group("opcode").lower()
    if not opcode.startswith("b") or opcode in _LINK_BRANCHES:
        return None
    operands = text[opcode_match.end():].strip()
    if opcode not in {"b", "ba"} and operands[:1] in {"+", "-"}:
        opcode += operands[0]
        operands = operands[1:].lstrip()
    destination_text = operands.rsplit(",", 1)[-1].strip() if operands else None
    destination = None
    if branch_dest is not None:
        try:
            destination = _parse_access_offset(branch_dest)
        except ValueError:
            pass
    elif destination_text:
        try:
            destination = _parse_access_offset(destination_text)
        except ValueError:
            pass
    return {"opcode": opcode, "destination_text": destination_text, "destination": destination}


def _branch_rows(rows: list[dict]) -> tuple[list[dict[str, Any]], dict[int, list[int]]]:
    branches: list[dict[str, Any]] = []
    addresses: dict[int, list[int]] = {}
    for index, row in enumerate(rows):
        compact = _access_row(row, index)
        if compact is None:
            continue
        try:
            address = _parse_access_offset(compact["instruction_address"])
        except ValueError:
            address = None
        if address is not None:
            addresses.setdefault(address, []).append(index)
        instruction_value = row.get("instruction")
        branch_dest = instruction_value.get("branch_dest") if isinstance(instruction_value, dict) else None
        branch = _branch_instruction(compact["text"], index, branch_dest)
        if branch is None:
            continue
        branches.append({**branch, "row_index": index})
    return branches, addresses


def _branch_side(item: dict[str, Any] | None, destination_row: int | None) -> dict[str, Any] | None:
    if item is None:
        return None
    return {
        "opcode": item["opcode"],
        "destination_text": item["destination_text"],
        "destination_address": item["destination"],
        "destination_row": destination_row,
    }


def _branch_category(target_rows: list[dict], candidate_rows: list[dict]) -> dict[str, Any]:
    target, target_addresses = _branch_rows(target_rows)
    candidate, candidate_addresses = _branch_rows(candidate_rows)
    target_by_index = {item["row_index"]: item for item in target}
    candidate_by_index = {item["row_index"]: item for item in candidate}

    def resolve(item: dict[str, Any] | None, addresses: dict[int, list[int]]) -> int | None:
        if item is None or item["destination"] is None:
            return None
        rows = addresses.get(item["destination"], [])
        return rows[0] if len(rows) == 1 else None

    findings: list[dict[str, Any]] = []
    same = changed = unresolved = paired = 0
    for index in sorted(set(target_by_index) | set(candidate_by_index)):
        left = target_by_index.get(index)
        right = candidate_by_index.get(index)
        if left is None or right is None:
            unresolved += 1
            target_row = resolve(left, target_addresses)
            candidate_row = resolve(right, candidate_addresses)
            status = "unresolved"
        else:
            paired += 1
            target_row = resolve(left, target_addresses)
            candidate_row = resolve(right, candidate_addresses)
            if left["opcode"] != right["opcode"]:
                unresolved += 1
                status = "unresolved"
            elif target_row is None or candidate_row is None:
                unresolved += 1
                status = "unresolved"
            elif target_row == candidate_row:
                same += 1
                continue
            else:
                changed += 1
                status = "changed"
        findings.append({
            "row_index": index,
            "status": status,
            "target_destination_row": target_row,
            "candidate_destination_row": candidate_row,
            "target": _branch_side(left, target_row),
            "candidate": _branch_side(right, candidate_row),
        })
    return {
        "target_branch_count": len(target),
        "candidate_branch_count": len(candidate),
        "paired_branch_count": paired,
        "same_destination_count": same,
        "changed_destination_count": changed,
        "unresolved_count": unresolved,
        "finding_count": len(findings),
        "findings": findings,
    }


def branch_map(*, root: Path, strict: Path, function: str) -> dict[str, Any]:
    """Compare branch destination row identities without source/owner inference."""
    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function.strip():
        raise ValueError("function must be nonempty text")
    function = function.strip()
    raw, report_binding = read_bound(root, Path(strict), REPORT_LIMIT)
    document = load_json(raw)
    if not isinstance(document, dict):
        raise ValueError("strict report must be a JSON object")
    left = focus._symbols(document, "left", "strict")
    right = focus._symbols(document, "right", "strict")
    target_symbol = _stack_function(left, function, "target")
    candidate_symbol = _stack_function(right, function, "candidate")
    missing = [side for side, symbol in (("target", target_symbol), ("candidate", candidate_symbol))
               if symbol is None]
    result: dict[str, Any] = {
        "schema": BRANCH_MAP_SCHEMA,
        "schema_version": 1,
        "report": dict(report_binding),
        "report_sha256": report_binding["sha256"],
        "function": function,
        "status": "ok" if not missing else "missing_symbol",
        "missing_sides": missing,
        "branches": _branch_category(
            focus._rows(target_symbol, f"strict.target.{function}") if target_symbol is not None else [],
            focus._rows(candidate_symbol, f"strict.candidate.{function}") if candidate_symbol is not None else [],
        ),
        "truncated": False,
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    while len(canonical(result)) + 1 > INDEX_LIMIT:
        findings = result["branches"]["findings"]
        if not findings:
            raise ValueError("branch-map result exceeds 256 KiB")
        result["branches"].setdefault("returned_finding_count", len(findings))
        drop = max(1, len(findings) // 2)
        del findings[-drop:]
        result["branches"]["returned_finding_count"] = len(findings)
        result["truncated"] = True
    return result


def _diagnose_text(value: Any, index: int) -> str | None:
    """Return one bounded normalized instruction spelling."""
    if value is None:
        return None
    if not isinstance(value, str):
        raise ValueError(f"strict.instructions[{index}] formatted text must be text")
    if len(value) > ACCESS_TEXT_LIMIT:
        raise ValueError(f"strict.instructions[{index}] formatted text exceeds {ACCESS_TEXT_LIMIT} characters")
    return " ".join(value.strip().split()).lower()


def _diagnose_identity_text(value: str, index: int) -> str:
    """Normalize an instruction for equality, abstracting branch row targets."""
    normalized = _diagnose_text(value, index) or ""
    # Relocation spellings are report-local symbol annotations (for example
    # lbl_802c32d8@sda21 versus @598@sda21), not source operands.  Their
    # attribution remains visible in the compact context/strict-data residual.
    normalized = re.sub(
        r"(?<![A-Za-z0-9_.])([A-Za-z_.$@][A-Za-z0-9_.$@]*)@(sda21|ha|h|l)\b",
        "<reloc>",
        normalized,
        flags=re.IGNORECASE,
    )
    opcode_match = _OPCODE_RE.match(normalized)
    if opcode_match is None:
        return normalized
    opcode = opcode_match.group("opcode").lower()
    if not opcode.startswith("b") or opcode in _LINK_BRANCHES:
        return normalized
    operands = normalized[opcode_match.end():].strip()
    if not operands:
        return normalized
    prefix, separator, last = operands.rpartition(",")
    destination = last.strip() if separator else operands
    try:
        _parse_access_offset(destination)
    except ValueError:
        return normalized
    operands = f"{prefix},{'<branch-target>'}" if separator else "<branch-target>"
    return " ".join(f"{opcode} {operands}".split()).lower()


def _diagnose_scalar(value: Any, limit: int = 128) -> Any:
    if value is None or isinstance(value, (bool, int, float)):
        return value
    if isinstance(value, str):
        return value[:limit]
    return str(value)[:limit]


def _diagnose_payload(row: dict | None, index: int) -> dict[str, Any] | None:
    """Compact instruction identity used for aligned source diagnostics.

    Addresses, parts and relocation target indexes are deliberately omitted from
    the identity.  Addresses may move between objects and relocation indexes are
    report-local annotations; neither is an instruction/source difference.
    """
    if row is None:
        return None
    instruction_value = row.get("instruction")
    if instruction_value is None:
        return None
    if not isinstance(instruction_value, dict):
        raise ValueError(f"strict.instructions[{index}].instruction must be an object")
    formatted = instruction_value.get("formatted")
    if formatted is not None and not isinstance(formatted, str):
        raise ValueError(f"strict.instructions[{index}].instruction.formatted must be text")
    if isinstance(formatted, str) and len(formatted) > ACCESS_TEXT_LIMIT:
        raise ValueError(f"strict.instructions[{index}] formatted text exceeds {ACCESS_TEXT_LIMIT} characters")
    result: dict[str, Any] = {}
    # branch_dest is checked separately by _branch_category using row identity;
    # absolute branch addresses are expected to move with an object.
    for key in ("size", "opcode"):
        if key in instruction_value:
            result[key] = instruction_value[key]
    if formatted is not None:
        result["formatted"] = _diagnose_identity_text(formatted, index)
    return result


def _diagnose_row(row: dict | None, index: int) -> dict[str, Any]:
    """Return one compact row for first-mismatch/context output."""
    result: dict[str, Any] = {"row": index}
    if row is None:
        result["instruction"] = None
        return result
    if not isinstance(row, dict):
        raise ValueError(f"strict.instructions[{index}] must be an object")
    instruction_value = row.get("instruction")
    if instruction_value is None:
        result["instruction"] = None
    elif not isinstance(instruction_value, dict):
        raise ValueError(f"strict.instructions[{index}].instruction must be an object")
    else:
        compact: dict[str, Any] = {}
        for key in ("address", "size", "opcode", "branch_dest"):
            if key in instruction_value:
                compact[key] = _diagnose_scalar(instruction_value[key])
        formatted = instruction_value.get("formatted")
        if formatted is not None:
            compact["formatted"] = _diagnose_text(formatted, index)
        relocation = instruction_value.get("relocation")
        if isinstance(relocation, dict):
            compact["relocation"] = {
                key: _diagnose_scalar(relocation[key])
                for key in ("type", "type_name", "target_symbol", "addend")
                if key in relocation
            }
        result["instruction"] = compact
    diff_kind = row.get("diff_kind")
    if diff_kind is not None:
        if not isinstance(diff_kind, str):
            raise ValueError(f"strict.instructions[{index}].diff_kind must be text or null")
        if diff_kind:
            result["diff_kind"] = diff_kind[:128]
    return result


def _diagnose_kind(row: dict | None) -> str | None:
    if not isinstance(row, dict):
        return None
    value = row.get("diff_kind")
    if isinstance(value, str) and value and value not in {"DIFF_NONE", "NONE"}:
        return value[:128]
    return None


def _diagnose_rows(target_rows: list[dict], candidate_rows: list[dict]) -> dict[str, Any]:
    """Compare aligned rows and retain only a bounded first mismatch window."""
    target_count = focus._instruction_count(target_rows)
    candidate_count = focus._instruction_count(candidate_rows)
    target_kinds: Counter[str] = Counter()
    candidate_kinds: Counter[str] = Counter()
    first: dict[str, Any] | None = None
    mismatch_count = 0
    instruction_mismatch_count = 0
    annotation_only_count = 0
    total = max(len(target_rows), len(candidate_rows))
    for index in range(total):
        target_row = target_rows[index] if index < len(target_rows) else None
        candidate_row = candidate_rows[index] if index < len(candidate_rows) else None
        target_payload = _diagnose_payload(target_row, index)
        candidate_payload = _diagnose_payload(candidate_row, index)
        target_kind = _diagnose_kind(target_row)
        candidate_kind = _diagnose_kind(candidate_row)
        if target_kind:
            target_kinds[target_kind] += 1
        if candidate_kind:
            candidate_kinds[candidate_kind] += 1
        instruction_changed = target_payload != candidate_payload
        annotation_changed = target_kind != candidate_kind
        if not instruction_changed and not annotation_changed:
            continue
        mismatch_count += 1
        if instruction_changed:
            instruction_mismatch_count += 1
        else:
            annotation_only_count += 1
        if first is None or (instruction_changed and first["kind"] == "annotation"):
            first = {
                "row": index,
                "kind": "instruction" if instruction_changed else "annotation",
                "target": _diagnose_row(target_row, index),
                "candidate": _diagnose_row(candidate_row, index),
            }

    if first is not None:
        center = first["row"]
        context: list[dict[str, Any]] = []
        start = max(0, center - 2)
        stop = min(total, center + 3)
        for index in range(start, stop):
            target_row = target_rows[index] if index < len(target_rows) else None
            candidate_row = candidate_rows[index] if index < len(candidate_rows) else None
            target_payload = _diagnose_payload(target_row, index)
            candidate_payload = _diagnose_payload(candidate_row, index)
            context.append({
                "row": index,
                "relative": index - center,
                "different": target_payload != candidate_payload,
                "target": _diagnose_row(target_row, index),
                "candidate": _diagnose_row(candidate_row, index),
            })
        first["context"] = context

    return {
        "target_row_count": len(target_rows),
        "candidate_row_count": len(candidate_rows),
        "target_instruction_count": target_count,
        "candidate_instruction_count": candidate_count,
        "diff_row_count": mismatch_count,
        "instruction_mismatch_count": instruction_mismatch_count,
        "annotation_only_count": annotation_only_count,
        "diff_kinds": {
            "target": dict(sorted(target_kinds.items())),
            "candidate": dict(sorted(candidate_kinds.items())),
            "combined": dict(sorted((target_kinds + candidate_kinds).items())),
        },
        "first_mismatch": first,
    }


def _diagnose_branch_status(summary: dict[str, Any]) -> str:
    target_count = summary["target_branch_count"]
    candidate_count = summary["candidate_branch_count"]
    if target_count == candidate_count == 0:
        return "none"
    if summary["unresolved_count"] or target_count != candidate_count:
        return "unresolved"
    if summary["changed_destination_count"]:
        return "changed"
    return "exact"


def _bounded_stack_category(category: dict[str, Any], limit: int = 48) -> dict[str, Any]:
    result = dict(category)
    pairs = list(category.get("pairs", []))
    interesting = [pair for pair in pairs if pair.get("status") != "equal"]
    result["pair_count_total"] = len(pairs)
    result["equal_pair_count"] = sum(pair.get("status") == "equal" for pair in pairs)
    result["changed_pair_count"] = len(interesting)
    result["pairs"] = interesting[:limit]
    result["returned_pair_count"] = len(result["pairs"])
    result["truncated"] = len(result["pairs"]) < len(interesting)
    return result


def _bounded_branch_category(category: dict[str, Any], limit: int = 48) -> dict[str, Any]:
    result = dict(category)
    findings = list(category.get("findings", []))
    result["finding_count_total"] = len(findings)
    result["findings"] = findings[:limit]
    result["truncated"] = len(result["findings"]) < len(findings)
    return result


def _operand_order_diagnostics(target_rows: list[dict], candidate_rows: list[dict]) -> dict[str, Any]:
    """Report aligned two-input order swaps without assigning source meaning."""
    findings: list[dict[str, Any]] = []
    checked_rows = max(len(target_rows), len(candidate_rows))
    for index in range(checked_rows):
        target_row = target_rows[index] if index < len(target_rows) else None
        candidate_row = candidate_rows[index] if index < len(candidate_rows) else None
        target_instruction = target_row.get("instruction") if isinstance(target_row, dict) else None
        candidate_instruction = candidate_row.get("instruction") if isinstance(candidate_row, dict) else None
        if not isinstance(target_instruction, dict) or not isinstance(candidate_instruction, dict):
            continue
        target_text = target_instruction.get("formatted")
        candidate_text = candidate_instruction.get("formatted")
        if not isinstance(target_text, str) or not isinstance(candidate_text, str):
            continue
        target_match = _OPCODE_RE.match(target_text)
        candidate_match = _OPCODE_RE.match(candidate_text)
        if target_match is None or candidate_match is None:
            continue
        target_opcode = target_match.group("opcode").lower()
        candidate_opcode = candidate_match.group("opcode").lower()
        if target_opcode != candidate_opcode:
            continue
        target_operands = [part.strip().lower() for part in target_text[target_match.end():].split(",")]
        candidate_operands = [part.strip().lower() for part in candidate_text[candidate_match.end():].split(",")]
        if len(target_operands) != 3 or len(candidate_operands) != 3:
            continue
        if target_operands[0] != candidate_operands[0]:
            continue
        if target_operands[1] == candidate_operands[1] and target_operands[2] == candidate_operands[2]:
            continue
        if sorted(target_operands[1:]) != sorted(candidate_operands[1:]):
            continue
        if len(findings) < 8:
            findings.append({
                "row": index,
                "opcode": target_opcode,
                "destination": target_operands[0],
                "target_inputs": target_operands[1:],
                "candidate_inputs": candidate_operands[1:],
                "classification": "same_opcode_destination_two_input_swap",
                "source_commutation_proven": False,
            })
    return {
        "status": "observed" if findings else "none",
        "classification": "same_opcode_destination_two_input_swap" if findings else None,
        "finding_count": len(findings),
        "findings": findings,
        "diagnostic_only": True,
        "authority_advanced": False,
    }


def _register_tokens(text: str) -> list[str]:
    return [match.group("register").lower() for match in _REGISTER_TOKEN_RE.finditer(text)]


def _without_registers(text: str) -> str:
    # Mask stack-home displacements before replacing register tokens so the
    # base-register test can still distinguish r1 from ordinary operands.
    masked = _diagnose_identity_text(text, 0)
    def stack_home(match: re.Match[str]) -> str:
        return f"<stack-disp>({match.group('base').lower()})" if match.group("base").lower() == "r1" else match.group(0)
    masked = _MEMORY_OPERAND_RE.sub(stack_home, masked)
    masked = _REGISTER_TOKEN_RE.sub("<reg>", masked)
    def normalize_number(match: re.Match[str]) -> str:
        token = match.group("number")
        sign = -1 if token.startswith("-") else 1
        digits = token[1:] if token[:1] in {"+", "-"} else token
        base = 16 if digits.lower().startswith("0x") else 10
        return str(sign * int(digits[2:] if base == 16 else digits, base))
    masked = _NUMBER_TOKEN_RE.sub(normalize_number, masked)
    return " ".join(masked.strip().split())


def _register_permutation(target_rows: list[dict], candidate_rows: list[dict]) -> dict[str, Any]:
    """Find a closed register-only rename without assigning source names.

    Every aligned instruction must retain its operation and immediates.  A
    conflicting register relation is reported as ambiguous, never promoted to
    a permutation hypothesis.
    """
    target_to_candidate: dict[str, set[str]] = {}
    candidate_to_target: dict[str, set[str]] = {}
    target_seen: set[str] = set()
    candidate_seen: set[str] = set()
    nonregister_differences: list[dict[str, Any]] = []
    register_conflicts: list[dict[str, Any]] = []
    checked_rows = max(len(target_rows), len(candidate_rows))
    for index in range(checked_rows):
        target_row = target_rows[index] if index < len(target_rows) else None
        candidate_row = candidate_rows[index] if index < len(candidate_rows) else None
        target_instruction = target_row.get("instruction") if isinstance(target_row, dict) else None
        candidate_instruction = candidate_row.get("instruction") if isinstance(candidate_row, dict) else None
        target_text = target_instruction.get("formatted") if isinstance(target_instruction, dict) else None
        candidate_text = candidate_instruction.get("formatted") if isinstance(candidate_instruction, dict) else None
        if target_text is not None and not isinstance(target_text, str):
            raise ValueError(f"strict.instructions[{index}].instruction.formatted must be text")
        if candidate_text is not None and not isinstance(candidate_text, str):
            raise ValueError(f"strict.instructions[{index}].instruction.formatted must be text")
        target_metadata = target_instruction if isinstance(target_instruction, dict) else {}
        candidate_metadata = candidate_instruction if isinstance(candidate_instruction, dict) else {}
        if target_text is None or candidate_text is None:
            if (target_text != candidate_text
                    or (target_metadata.get("size"), target_metadata.get("opcode")) != (
                        candidate_metadata.get("size"), candidate_metadata.get("opcode")
                    )):
                nonregister_differences.append({
                    "row": index,
                    "reason": "missing_instruction" if target_text != candidate_text else "opcode_or_immediate_changed",
                })
            continue
        target_registers = _register_tokens(target_text)
        candidate_registers = _register_tokens(candidate_text)
        target_seen.update(target_registers)
        candidate_seen.update(candidate_registers)
        if (target_metadata.get("size"), target_metadata.get("opcode")) != (
            candidate_metadata.get("size"), candidate_metadata.get("opcode")
        ):
            if len(nonregister_differences) < 8:
                nonregister_differences.append({
                    "row": index,
                    "reason": "opcode_or_immediate_changed",
                    "target": _diagnose_text(target_text, index),
                    "candidate": _diagnose_text(candidate_text, index),
                })
            continue
        if _without_registers(target_text) != _without_registers(candidate_text):
            if len(nonregister_differences) < 8:
                nonregister_differences.append({
                    "row": index,
                    "reason": "opcode_or_immediate_changed",
                    "target": _diagnose_text(target_text, index),
                    "candidate": _diagnose_text(candidate_text, index),
                })
            continue
        if len(target_registers) != len(candidate_registers):
            if len(nonregister_differences) < 8:
                nonregister_differences.append({
                    "row": index,
                    "reason": "register_operand_count_changed",
                })
            continue
        for target_register, candidate_register in zip(target_registers, candidate_registers):
            if target_register[0] != candidate_register[0]:
                if len(register_conflicts) < 8:
                    register_conflicts.append({
                        "row": index,
                        "target": target_register,
                        "candidate": candidate_register,
                        "reason": "register_class_changed",
                    })
                continue
            target_to_candidate.setdefault(target_register, set()).add(candidate_register)
            candidate_to_target.setdefault(candidate_register, set()).add(target_register)
    # A branch's textual displacement is an object-local address, but a changed
    # destination row is a real control-flow residual.  Keep that separate from
    # the register-only comparison so a moved-but-equivalent branch does not
    # block a useful permutation hypothesis while a changed branch cannot pass.
    branch_summary = _branch_category(target_rows, candidate_rows)
    if _diagnose_branch_status(branch_summary) not in {"exact", "none"}:
        for finding in branch_summary.get("findings", [])[:4]:
            if len(nonregister_differences) >= 8:
                break
            nonregister_differences.append({
                "row": finding.get("row_index"),
                "reason": "branch_destination_changed",
            })
    mapping = {
        target: next(iter(candidates))
        for target, candidates in sorted(target_to_candidate.items())
        if len(candidates) == 1
    }
    mapping_conflicts = [
        {"target": target, "candidates": sorted(candidates)}
        for target, candidates in sorted(target_to_candidate.items())
        if len(candidates) > 1
    ]
    mapping_conflicts.extend(
        {"candidate": candidate, "targets": sorted(targets)}
        for candidate, targets in sorted(candidate_to_target.items())
        if len(targets) > 1
    )
    target_banks = {bank: {reg for reg in target_seen if reg.startswith(bank)} for bank in "rf"}
    candidate_banks = {bank: {reg for reg in candidate_seen if reg.startswith(bank)} for bank in "rf"}
    register_set_cardinality_equal = all(
        len(target_banks[bank]) == len(candidate_banks[bank]) for bank in "rf"
    )
    changed_mapping = {target: candidate for target, candidate in mapping.items() if target != candidate}
    mapping_complete = (
        register_set_cardinality_equal
        and not mapping_conflicts
        and not register_conflicts
        and all(target in mapping for target in target_seen)
        and len({candidate for candidate in mapping.values()}) == len(mapping)
    )
    closed = bool(mapping_complete and not nonregister_differences)
    if nonregister_differences:
        status = "rejected"
        reason = nonregister_differences[0]["reason"]
    elif register_conflicts or mapping_conflicts:
        status = "ambiguous"
        reason = "conflicting_register_relations"
    elif not closed:
        status = "not_closed"
        reason = "register_set_cardinality_changed"
    elif not changed_mapping:
        status = "none"
        reason = "no_register_operand_change"
    else:
        status = "confirmed"
        reason = "closed_register_only_permutation"
    return {
        "status": status,
        "reason": reason,
        "closed": closed,
        "mapping_complete": mapping_complete,
        "register_set_cardinality_equal": register_set_cardinality_equal,
        "operations_immediates_agree": not bool(nonregister_differences),
        "checked_rows": checked_rows,
        "target_register_count": len(target_seen),
        "candidate_register_count": len(candidate_seen),
        "target_register_classes": {bank: len(target_banks[bank]) for bank in "rf"},
        "candidate_register_classes": {bank: len(candidate_banks[bank]) for bank in "rf"},
        "mapping": changed_mapping,
        "changed_mapping": changed_mapping,
        "mapping_conflicts": mapping_conflicts[:4],
        "register_conflicts": register_conflicts[:4],
        "nonregister_differences": nonregister_differences,
        "diagnostic_only": True,
        "authority_advanced": False,
    }


def _diagnose_sha(value: Any) -> str | None:
    if isinstance(value, str) and _SHA256_RE.fullmatch(value):
        return value.lower()
    if isinstance(value, dict):
        for key in ("sha256", "sha", "digest", "object_sha256", "output_sha256"):
            result = _diagnose_sha(value.get(key))
            if result is not None:
                return result
    return None


def _top_level_output_sha(document: dict[str, Any]) -> str | None:
    for key in (
        "compiler_output_sha256",
        "compiler_output_sha",
        "candidate_object_sha256",
        "candidate_output_sha256",
        "candidate_sha256",
        "object_sha256",
        "output_sha256",
        "index_sha256",
        "index",
        "candidate_object",
        "candidate_output",
        "compiler_output",
    ):
        result = _diagnose_sha(document.get(key))
        if result is not None:
            return result
    return None


def _varinfo_variable(value: Any, index: int, collection: str) -> dict[str, Any] | None:
    if not isinstance(value, dict):
        raise ValueError(f"varinfo {collection}[{index}] must be an object")
    name = value.get("name")
    if not isinstance(name, str) or not name.strip():
        return None
    if len(name) > 128:
        raise ValueError(f"varinfo {collection}[{index}] name exceeds 128 characters")
    result: dict[str, Any] = {"name": name}
    for key in ("usage", "rclass", "reg", "reg_hi", "noregister", "used", "datatype", "flags"):
        if key in value:
            result[key] = _diagnose_scalar(value[key], 64)
    return result


def _varinfo_diagnosis(
    *,
    root: Path,
    path: Path | None,
    function: str,
    strict_document: dict[str, Any],
) -> dict[str, Any]:
    base: dict[str, Any] = {
        "status": "not_supplied" if path is None else "unproven",
        "path": None,
        "named_arguments": [],
        "named_locals": [],
        "arguments": [],
        "locals": [],
        "usage_groups": [],
        "score_relation": {"status": "UNKNOWN", "known_scores": [], "unknown_count": 0},
        "compiler_output_binding": {
            "status": "unproven",
            "physical_proof": False,
            "authority_advanced": False,
        },
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    if path is None:
        return base
    try:
        raw, binding = read_bound(root, path, VARINFO_LIMIT)
        document = load_json(raw)
    except (OSError, ValueError, TypeError) as exc:
        base["status"] = "malformed"
        base["reason"] = str(exc)[:256]
        return base
    base["path"] = dict(binding)
    if not isinstance(document, dict):
        base["status"] = "malformed"
        base["reason"] = "varinfo must be a JSON object"
        return base
    varinfo_function = document.get("function") or document.get("target")
    if varinfo_function is not None and varinfo_function != function:
        base["status"] = "function_mismatch"
        base["reason"] = f"varinfo target {varinfo_function!r} differs from {function!r}"
    malformed_rows: list[str] = []
    duplicate_names: set[str] = set()
    seen_names: set[str] = set()
    for collection, output_key in (("arguments", "named_arguments"), ("locals", "named_locals")):
        raw_rows = document.get(collection, [])
        if raw_rows is None:
            raw_rows = []
        if not isinstance(raw_rows, list):
            malformed_rows.append(collection)
            continue
        rows: list[dict[str, Any]] = []
        for index, value in enumerate(raw_rows):
            try:
                item = _varinfo_variable(value, index, collection)
            except ValueError as exc:
                malformed_rows.append(str(exc)[:192])
                continue
            if item is None:
                continue
            name = item["name"]
            if name in seen_names:
                duplicate_names.add(name)
            seen_names.add(name)
            rows.append(item)
        rows = rows[:128]
        base[output_key] = rows
        base[collection] = rows
    all_variables = base["named_arguments"] + base["named_locals"]
    groups: dict[int, list[dict[str, Any]]] = {}
    unknown_count = 0
    for variable in all_variables:
        usage = variable.get("usage")
        if isinstance(usage, bool) or not isinstance(usage, int):
            unknown_count += 1
            continue
        groups.setdefault(usage, []).append(variable)
    usage_groups = []
    for usage, variables in sorted(groups.items()):
        usage_groups.append({
            "usage": usage,
            "names": [item["name"] for item in variables[:16]],
            "registers": [item.get("reg") for item in variables[:16]],
            "tie": len(variables) > 1,
        })
    base["usage_groups"] = usage_groups[:64]
    known_scores = sorted(groups)
    tie_names = sorted({item["name"] for variables in groups.values() if len(variables) > 1
                        for item in variables})[:16]
    if unknown_count or not known_scores:
        score_status = "UNKNOWN"
    elif any(len(variables) > 1 for variables in groups.values()):
        score_status = "TIED"
    elif len(known_scores) > 1:
        score_status = "UNEQUAL"
    else:
        score_status = "EQUAL"
    base["score_relation"] = {
        "status": score_status,
        "known_scores": known_scores[:64],
        "unknown_count": unknown_count,
        "tie_names": tie_names,
        "duplicate_names": sorted(duplicate_names)[:16],
    }
    if malformed_rows:
        base["status"] = "malformed" if base["status"] == "unproven" else base["status"]
        base["malformed_rows"] = malformed_rows[:8]
    elif duplicate_names and base["status"] == "unproven":
        base["status"] = "ambiguous"
        base["reason"] = "duplicate named variable"
    elif base["status"] == "unproven":
        base["status"] = "ok"

    compiler_output_sha = _top_level_output_sha(document)
    index_output_sha = _top_level_output_sha(strict_document)
    binding_result = base["compiler_output_binding"]
    binding_result["compiler_output_sha256"] = compiler_output_sha
    binding_result["index_output_sha256"] = index_output_sha
    if compiler_output_sha is not None and index_output_sha is not None:
        binding_result["status"] = "bound" if compiler_output_sha == index_output_sha else "mismatch"
        binding_result["reason"] = "sha256_equal" if compiler_output_sha == index_output_sha else "sha256_differs"
    else:
        binding_result["reason"] = "compiler_output_or_index_sha256_not_supplied"
    return base


def _strict_data_residuals(
    strict_target: list[dict],
    strict_candidate: list[dict],
    data_target: list[dict],
    data_candidate: list[dict],
) -> dict[str, Any]:
    residuals: list[dict[str, Any]] = []
    counts: Counter[str] = Counter()
    for side, strict_rows, data_rows in (
        ("target", strict_target, data_target),
        ("candidate", strict_candidate, data_candidate),
    ):
        for index in range(max(len(strict_rows), len(data_rows))):
            strict_row = strict_rows[index] if index < len(strict_rows) else None
            data_row = data_rows[index] if index < len(data_rows) else None
            strict_payload = _diagnose_payload(strict_row, index)
            data_payload = _diagnose_payload(data_row, index)
            if strict_payload != data_payload:
                kind = "instruction"
            else:
                strict_kind = _diagnose_kind(strict_row)
                data_kind = _diagnose_kind(data_row)
                strict_reloc = strict_row.get("instruction", {}).get("relocation") if isinstance(strict_row, dict) and isinstance(strict_row.get("instruction"), dict) else None
                data_reloc = data_row.get("instruction", {}).get("relocation") if isinstance(data_row, dict) and isinstance(data_row.get("instruction"), dict) else None
                if strict_reloc != data_reloc:
                    kind = "relocation_annotation"
                elif strict_kind != data_kind:
                    kind = "diff_annotation"
                else:
                    continue
            counts[kind] += 1
            if len(residuals) < 16:
                residuals.append({
                    "side": side,
                    "row": index,
                    "kind": kind,
                    "strict": _diagnose_row(strict_row, index),
                    "data": _diagnose_row(data_row, index),
                })
    if not residuals and not counts:
        status = "exact"
    elif set(counts) == {"relocation_annotation"}:
        status = "relocation_only"
    elif set(counts) <= {"diff_annotation", "relocation_annotation"}:
        status = "annotations_only"
    else:
        status = "residual"
    return {
        "status": status,
        "residual_count": sum(counts.values()),
        "residual_kinds": dict(sorted(counts.items())),
        "residuals": residuals,
        "relocation_attribution": {
            "status": "diagnostic_only",
            "authority": "strict_data_report_annotation",
            "physical_proof": False,
            "authority_advanced": False,
        },
    }


def _diagnose_channel(document: dict[str, Any], label: str, function: str) -> dict[str, Any]:
    left = focus._symbols(document, "left", label)
    right = focus._symbols(document, "right", label)
    target = _stack_function(left, function, "target")
    candidate = _stack_function(right, function, "candidate")
    missing = [side for side, symbol in (("target", target), ("candidate", candidate)) if symbol is None]
    if missing:
        return {
            "status": "missing_symbol",
            "missing_sides": missing,
            "gates": {
                "size_exact": None,
                "instruction_count_exact": None,
                "instruction_stream_exact": None,
                "exact": False,
            },
            "first_mismatch": None,
            "diffs": None,
            "branch_destinations": None,
            "stack_home": None,
            "input_operand_order": None,
            "register_permutation": None,
        }
    target_rows = focus._rows(target, f"{label}.target.{function}")
    candidate_rows = focus._rows(candidate, f"{label}.candidate.{function}")
    differences = _diagnose_rows(target_rows, candidate_rows)
    first_mismatch = differences.pop("first_mismatch")
    target_size = focus._integer(target.get("size"))
    candidate_size = focus._integer(candidate.get("size"))
    size_exact = target_size is not None and candidate_size is not None and target_size == candidate_size
    count_exact = differences["target_instruction_count"] == differences["candidate_instruction_count"]
    stream_exact = differences["diff_row_count"] == 0
    branch = _bounded_branch_category(_branch_category(target_rows, candidate_rows))
    branch["status"] = _diagnose_branch_status(branch)
    branch["exact"] = branch["status"] in {"exact", "none"}
    stack = {
        "d_form": _bounded_stack_category(_stack_category(target_rows, candidate_rows, "d_form")),
        "addi_pointer": _bounded_stack_category(_stack_category(target_rows, candidate_rows, "addi_pointer")),
    }
    stack["diagnostic_only"] = True
    stack["authority_advanced"] = False
    operand_order = _operand_order_diagnostics(target_rows, candidate_rows)
    permutation = _register_permutation(target_rows, candidate_rows)
    exact = bool(size_exact and count_exact and stream_exact and branch["exact"])
    return {
        "status": "ok",
        "missing_sides": [],
        "target": {
            "name": target.get("name"),
            "size": target.get("size"),
            "size_integer": target_size,
            "match_percent": target.get("match_percent"),
        },
        "candidate": {
            "name": candidate.get("name"),
            "size": candidate.get("size"),
            "size_integer": candidate_size,
            "match_percent": candidate.get("match_percent"),
        },
        "gates": {
            "size_exact": size_exact,
            "instruction_count_exact": count_exact,
            "instruction_stream_exact": stream_exact,
            "branch_destination_exact": branch["exact"],
            "exact": exact,
        },
        "size_exact": size_exact,
        "instruction_count_exact": count_exact,
        "first_mismatch": first_mismatch,
        "diffs": differences,
        "branch_destinations": branch,
        "stack_home": stack,
        "input_operand_order": operand_order,
        "register_permutation": permutation,
    }


def _diagnose_channel_summary(channel: dict[str, Any], comparison: str) -> dict[str, Any]:
    """Keep the data channel as counts; residual rows live in strict_vs_data."""
    diffs = channel.get("diffs") if isinstance(channel.get("diffs"), dict) else {}
    branches = channel.get("branch_destinations")
    branch_summary = {}
    if isinstance(branches, dict):
        branch_summary = {
            key: branches.get(key)
            for key in (
                "status", "target_branch_count", "candidate_branch_count",
                "same_destination_count", "changed_destination_count", "unresolved_count",
            )
            if key in branches
        }
    return {
        "status": channel.get("status"),
        "summary": comparison,
        "gates": {
            key: channel.get("gates", {}).get(key)
            for key in ("size_exact", "instruction_count_exact", "instruction_stream_exact",
                        "branch_destination_exact", "exact")
            if key in channel.get("gates", {})
        },
        "diffs": {
            key: diffs.get(key)
            for key in (
                "target_row_count", "candidate_row_count", "target_instruction_count",
                "candidate_instruction_count", "diff_row_count", "instruction_mismatch_count",
                "annotation_only_count", "diff_kinds",
            )
            if key in diffs
        },
        "branch_destinations": branch_summary,
        "diagnostic_only": True,
        "authority_advanced": False,
    }


def _fit_diagnosis(result: dict[str, Any]) -> dict[str, Any]:
    """Keep the default diagnosis compact while retaining earliest evidence."""
    result["output_limit_bytes"] = DIAGNOSE_OUTPUT_LIMIT
    result["focused_output_limit_bytes"] = DIAGNOSE_FOCUS_LIMIT
    fit_limit = DIAGNOSE_FOCUS_LIMIT - 256
    paths = [
        ("strict_vs_data", "residuals"),
        ("varinfo", "named_locals"),
        ("varinfo", "named_arguments"),
        ("varinfo", "usage_groups"),
    ]
    for owner, key in paths:
        value = result.get(owner)
        if isinstance(value, dict) and isinstance(value.get(key), list):
            limit = 32 if key == "usage_groups" else 64
            value[key] = value[key][:limit]
            if owner == "varinfo" and key == "named_locals":
                value["locals"] = value[key]
            elif owner == "varinfo" and key == "named_arguments":
                value["arguments"] = value[key]
            value["truncated"] = len(value[key]) == limit
    for channel in (result.get("strict"), result.get("data")):
        if not isinstance(channel, dict):
            continue
        branch = channel.get("branch_destinations")
        if isinstance(branch, dict) and isinstance(branch.get("findings"), list):
            branch["findings"] = branch["findings"][:32]
        stack = channel.get("stack_home")
        if isinstance(stack, dict):
            for category in (stack.get("d_form"), stack.get("addi_pointer")):
                if isinstance(category, dict) and isinstance(category.get("pairs"), list):
                    category["pairs"] = category["pairs"][:32]
        permutation = channel.get("register_permutation")
        if isinstance(permutation, dict):
            permutation["nonregister_differences"] = permutation.get("nonregister_differences", [])[:4]
    while len(canonical(result)) + 1 > fit_limit:
        changed = False
        for owner, key in paths:
            value = result.get(owner)
            if isinstance(value, dict) and isinstance(value.get(key), list) and value[key]:
                value[key].pop()
                if owner == "varinfo" and key == "named_locals":
                    value["locals"] = value[key]
                elif owner == "varinfo" and key == "named_arguments":
                    value["arguments"] = value[key]
                value["truncated"] = True
                changed = True
                break
        if changed:
            continue
        for channel in (result.get("strict"), result.get("data")):
            if not isinstance(channel, dict):
                continue
            branch = channel.get("branch_destinations")
            if isinstance(branch, dict) and branch.get("findings"):
                branch["findings"].pop()
                branch["truncated"] = True
                changed = True
                break
            stack = channel.get("stack_home")
            if isinstance(stack, dict):
                for category in (stack.get("d_form"), stack.get("addi_pointer")):
                    if isinstance(category, dict) and category.get("pairs"):
                        category["pairs"].pop()
                        category["truncated"] = True
                        changed = True
                        break
                if changed:
                    break
        if changed:
            continue
        raise ValueError("diagnosis result exceeds focused 8 KiB budget")
    # The focused fit may trim emitted findings after the bounded builders have
    # populated their returned-count fields.  Keep those fields truthful while
    # retaining the separate total/equal/changed counts for the full scan.
    for channel in (result.get("strict"), result.get("data")):
        if not isinstance(channel, dict):
            continue
        branch = channel.get("branch_destinations")
        if isinstance(branch, dict) and isinstance(branch.get("findings"), list):
            branch["returned_finding_count"] = len(branch["findings"])
        stack = channel.get("stack_home")
        if isinstance(stack, dict):
            for category in (stack.get("d_form"), stack.get("addi_pointer")):
                if isinstance(category, dict) and isinstance(category.get("pairs"), list):
                    category["returned_pair_count"] = len(category["pairs"])
    result["output_bytes"] = len(canonical(result)) + 1
    # output_bytes itself changes the payload by a few bytes; recalculate once.
    result["output_bytes"] = len(canonical(result)) + 1
    if result["output_bytes"] > DIAGNOSE_FOCUS_LIMIT:
        raise ValueError("diagnosis result exceeds focused 8 KiB budget")
    return result


def diagnose(
    *,
    root: Path,
    strict: Path,
    data: Path | None,
    function: str,
    varinfo: Path | None = None,
) -> dict[str, Any]:
    """Produce a read-only, bounded source-inference diagnosis."""
    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function.strip():
        raise ValueError("function must be nonempty text")
    function = function.strip()
    strict_raw, strict_binding = read_bound(root, Path(strict), REPORT_LIMIT)
    strict_document = load_json(strict_raw)
    if not isinstance(strict_document, dict):
        raise ValueError("strict report must be a JSON object")
    data_document = None
    data_binding = None
    if data is not None:
        if local(root, Path(data)) == local(root, Path(strict)):
            data_document = strict_document
            data_binding = dict(strict_binding)
        else:
            data_raw, data_binding = read_bound(root, Path(data), REPORT_LIMIT)
            data_document = load_json(data_raw)
            if not isinstance(data_document, dict):
                raise ValueError("data report must be a JSON object")
    strict_channel = _diagnose_channel(strict_document, "strict", function)
    data_channel = None if data_document is None else _diagnose_channel(data_document, "data", function)
    strict_target: list[dict] = []
    strict_candidate: list[dict] = []
    data_target: list[dict] = []
    data_candidate: list[dict] = []
    if strict_channel.get("status") == "ok":
        left = focus._symbols(strict_document, "left", "strict")
        right = focus._symbols(strict_document, "right", "strict")
        strict_target = focus._rows(_stack_function(left, function, "target"), f"strict.target.{function}")
        strict_candidate = focus._rows(_stack_function(right, function, "candidate"), f"strict.candidate.{function}")
    strict_data = None
    if data_document is not None:
        if data_channel is not None and data_channel.get("status") == "ok":
            left = focus._symbols(data_document, "left", "data")
            right = focus._symbols(data_document, "right", "data")
            data_target = focus._rows(_stack_function(left, function, "target"), f"data.target.{function}")
            data_candidate = focus._rows(_stack_function(right, function, "candidate"), f"data.candidate.{function}")
        if strict_channel.get("status") == "ok" and data_channel is not None and data_channel.get("status") == "ok":
            strict_data = _strict_data_residuals(strict_target, strict_candidate, data_target, data_candidate)
        else:
            strict_data = {
                "status": "unproven",
                "residual_count": None,
                "residual_kinds": {},
                "residuals": [],
                "relocation_attribution": {
                    "status": "diagnostic_only",
                    "authority": "strict_data_report_annotation",
                    "physical_proof": False,
                    "authority_advanced": False,
                },
            }
    data_output_channel = data_channel
    if data_channel is not None:
        comparison = (
            "same_as_strict"
            if strict_data is not None and strict_data.get("status") == "exact"
            else "residuals_in_strict_vs_data"
        )
        data_output_channel = _diagnose_channel_summary(data_channel, comparison)
    result: dict[str, Any] = {
        "schema": DIAGNOSE_SCHEMA,
        "schema_version": 1,
        "function": function,
        "inputs": {
            "strict": dict(strict_binding),
            "data": None if data_binding is None else dict(data_binding),
            "varinfo": None,
        },
        "strict": strict_channel,
        "data": data_output_channel,
        "strict_vs_data": strict_data,
        "varinfo": _varinfo_diagnosis(
            root=root,
            path=varinfo,
            function=function,
            strict_document=strict_document,
        ),
        "physical_proof": False,
        "source_emission_authorized": False,
        "promotion_authorized": False,
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    if result["varinfo"].get("path") is not None:
        result["inputs"]["varinfo"] = dict(result["varinfo"]["path"])
    return _fit_diagnosis(result)


def instruction(row: dict | None) -> dict | None:
    if row is None:
        return None
    insn = row.get("instruction")
    if not isinstance(insn, dict):
        return None
    # Parts carry relocations and register text; retain one bounded row, not the
    # thousands of repeated instruction/relocation objects in the full report.
    text = "".join(str(p.get("text", "")) for p in insn.get("parts", []) if isinstance(p, dict))
    result = {key: insn[key] for key in ("address", "size", "opcode", "formatted") if key in insn}
    if text:
        result["text"] = text[:512]
    return result


def relocation_key(row: dict, symbols: list) -> dict | None:
    insn = row.get("instruction") or {}
    reloc = insn.get("relocation")
    if not isinstance(reloc, dict) or reloc.get("type_name") == "R_PPC_NONE":
        return None
    index = reloc.get("target_symbol")
    if not isinstance(index, int) or isinstance(index, bool) or not 0 <= index < len(symbols):
        raise ValueError("invalid relocation target in current report")
    return {"symbol": symbols[index].get("name"), "type": reloc.get("type_name", reloc.get("type")),
            "addend": reloc.get("addend", 0)}


def summarize(document: dict, label: str) -> list[dict]:
    if not isinstance(document, dict):
        raise ValueError(f"{label} report must be a JSON object")
    left = focus._symbols(document, "left", label)
    right = focus._symbols(document, "right", label)
    if not any(focus._is_function(symbol) for symbol in left):
        raise ValueError(f"{label} report has no target functions")
    result = []
    seen = set()
    for symbol in left:
        if not focus._is_function(symbol):
            continue
        name = symbol.get("name")
        if not isinstance(name, str) or name in seen:
            raise ValueError(f"ambiguous {label} function: {name!r}")
        seen.add(name)
        index = symbol.get("target_symbol")
        candidate = right[index] if isinstance(index, int) and not isinstance(index, bool) and 0 <= index < len(right) else None
        if candidate is not None and candidate.get("name") != name:
            raise ValueError(f"{label} paired name differs for {name}")
        if candidate is not None and not focus._is_function(candidate):
            raise ValueError(f"{label} candidate is not a function: {name}")
        rows = focus._rows(symbol, name)
        other_rows = focus._rows(candidate, name) if candidate else []
        differences = [(i, rows[i] if i < len(rows) else {}) for i in range(max(len(rows), len(other_rows)))
                       if ((rows[i].get("diff_kind") if i < len(rows) else None)
                           or (other_rows[i].get("diff_kind") if i < len(other_rows) else None))
                       not in (None, "", "DIFF_NONE", "NONE")]
        first = None
        if differences:
            i, row = differences[0]
            first = {"row": i, "kind": row.get("diff_kind") or other_rows[i].get("diff_kind"), "target": instruction(row),
                     "candidate": instruction(other_rows[i]) if i < len(other_rows) else None}
        relocations = []
        for i, row in differences:
            target_key = relocation_key(row, left)
            candidate_key = relocation_key(other_rows[i], right) if i < len(other_rows) else None
            if target_key is not None and candidate_key is not None and target_key != candidate_key:
                entry = {"target": target_key, "candidate": candidate_key}
                if entry not in relocations:
                    relocations.append(entry)
        percent = symbol.get("match_percent")
        exact = (candidate is not None and focus._is_function(candidate) and percent == 100 and not differences
                 and focus._integer(symbol.get("size")) is not None
                 and focus._integer(symbol.get("size")) == focus._integer(candidate.get("size"))
                 and focus._instruction_count(rows) == focus._instruction_count(other_rows))
        result.append({"function": name, "target_bytes": symbol.get("size"),
                       "candidate_bytes": candidate.get("size") if candidate else None,
                       "pair_index": index, "target_instruction_count": focus._instruction_count(rows),
                       "candidate_instruction_count": focus._instruction_count(other_rows),
                       "match_percent": percent, "diff_rows": len(differences),
                       "instruction_exact": bool(exact), "relocation_keys": relocations,
                       "unlocated_mismatch": None if exact or first else (
                           "unpaired target function" if candidate is None else
                           "score/size/count mismatch without a differing instruction row"),
                       "first_mismatch": first})
    return result


def candidate_only(document: dict, metrics: list[dict]) -> list[dict]:
    paired = {item["pair_index"] for item in metrics}
    return [{"function": symbol.get("name"), "bytes": symbol.get("size"), "index": i}
            for i, symbol in enumerate(focus._symbols(document, "right", "report"))
            if focus._is_function(symbol) and i not in paired]


def snapshot(*, root: Path, owner: str, source: Path, target: Path, candidate: Path,
             strict: Path, data: Path | None, toolchain_key: str,
             compile_receipt: Path | None = None) -> dict:
    root = Path(os.path.abspath(root))
    if not owner or not toolchain_key:
        raise ValueError("owner and toolchain key are required")
    inputs = {}
    for role, path, limit in (("source", source, 4*1024*1024), ("target_object", target, 16*1024*1024),
                              ("candidate_object", candidate, 16*1024*1024)):
        _, inputs[role] = read_bound(root, path, limit)
    raw, inputs["strict_report"] = read_bound(root, strict, REPORT_LIMIT)
    document = load_json(raw)
    strict_rows = summarize(document, "strict")
    extra_functions = candidate_only(document, strict_rows)
    del raw, document
    data_rows = None
    if data is not None:
        if local(root, data) == local(root, strict):
            inputs["data_report"] = dict(inputs["strict_report"])
            data_rows = strict_rows
            data_extra = extra_functions
        else:
            raw, inputs["data_report"] = read_bound(root, data, REPORT_LIMIT)
            document = load_json(raw)
            data_rows = summarize(document, "data")
            data_extra = candidate_only(document, data_rows)
            del raw, document
        fields = ("function", "target_bytes", "candidate_bytes", "pair_index", "target_instruction_count", "candidate_instruction_count")
        if ([tuple(r[k] for k in fields) for r in strict_rows] != [tuple(r[k] for k in fields) for r in data_rows]
                or extra_functions != data_extra):
            raise ValueError("strict/data function census or object layout differs")
    binding = "not_supplied"
    if compile_receipt is not None:
        raw, inputs["compile_receipt"] = read_bound(root, compile_receipt, INDEX_LIMIT)
        receipt = load_json(raw)
        if not isinstance(receipt, dict):
            raise ValueError("compiler receipt must be a JSON object")
        if (receipt.get("schema") != "recovery_candidate_compile/v1"
                or receipt.get("source_sha256") != inputs["source"]["sha256"]
                or receipt.get("object_sha256") != inputs["candidate_object"]["sha256"]):
            raise ValueError("compiler receipt does not bind selected source/object")
        binding = "receipt_hashes_match"
    result = {"schema": SCHEMA, "owner": owner, "toolchain_key": toolchain_key,
              "authority_advanced": False, "report_binding": "caller_selected_diagnostic",
              "compile_binding": binding, "physical_exact": None, "linked_exact": None,
              "function_census_binding": "report_only_not_independently_verified",
              "candidate_only_functions": extra_functions,
              "inputs": inputs, "functions": strict_rows, "data_functions": data_rows,
              "summary": {"functions": len(strict_rows),
                          "strict_instruction_exact": sum(r["instruction_exact"] for r in strict_rows),
                          "data_instruction_exact": None if data_rows is None else sum(r["instruction_exact"] for r in data_rows)}}
    groups = {}
    for item in strict_rows:
        for pair in item["relocation_keys"]:
            key = canonical(pair).decode("utf-8")
            groups.setdefault(key, []).append(item["function"])
    result["shared_relocation_diagnostics"] = [
        {**json.loads(key), "functions": names, "cause_proven": False}
        for key, names in sorted(groups.items()) if len(names) > 1]
    result["index_sha256"] = hashlib.sha256(canonical(result)).hexdigest()
    if len(canonical(result)) + 1 > INDEX_LIMIT:
        raise ValueError("compact index exceeds 256 KiB; no output published")
    verify(root, result)
    return result


def verify(root: Path, index: dict) -> None:
    root = Path(os.path.abspath(root))
    if index.get("schema") != SCHEMA:
        raise ValueError("unsupported current evidence index")
    payload = {key: value for key, value in index.items() if key != "index_sha256"}
    if hashlib.sha256(canonical(payload)).hexdigest() != index.get("index_sha256"):
        raise ValueError("current evidence index digest differs")
    for role, desc in index["inputs"].items():
        _, actual = read_bound(root, Path(desc["path"]), REPORT_LIMIT)
        if actual != desc:
            raise ValueError(f"stale current evidence: {role} {desc['path']}; refresh from live source")


def publish(root: Path, path: Path, value: dict) -> None:
    root = Path(os.path.abspath(root))
    path = local(root, path)
    path.relative_to(root / "build")
    if any(local(root, Path(d["path"])) == path for d in value["inputs"].values()):
        raise ValueError("index output aliases evidence input")
    raw = canonical(value) + b"\n"
    if len(raw) > INDEX_LIMIT:
        raise ValueError("compact index exceeds 256 KiB")
    verify(root, value)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(raw)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp, path)
    finally:
        Path(temp).unlink(missing_ok=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    sub = parser.add_subparsers(dest="action", required=True)
    create = sub.add_parser("snapshot")
    for name in ("owner", "toolchain-key"):
        create.add_argument("--" + name, required=True)
    for name in ("source", "target-object", "candidate-object", "strict", "out"):
        create.add_argument("--" + name, type=Path, required=True)
    create.add_argument("--data", type=Path)
    create.add_argument("--compile-receipt", type=Path)
    check = sub.add_parser("verify")
    check.add_argument("index", type=Path)
    access = sub.add_parser("accesses", help="find exact formatted displacement(base) accesses")
    access.add_argument("--strict", type=Path, required=True)
    access.add_argument("--function", required=True)
    access.add_argument("--side", choices=("target", "candidate"), required=True)
    access.add_argument("--base-register", required=True)
    access.add_argument("--offset", required=True)
    access.add_argument("--context", type=int, default=0)
    access.add_argument(
        "--max-matches",
        "--limit",
        dest="max_matches",
        type=int,
        default=DEFAULT_ACCESS_MATCH_LIMIT,
    )
    stack = sub.add_parser("stack-map", help="summarize aligned r1 stack displacement pairs")
    stack.add_argument("--strict", type=Path, required=True)
    stack.add_argument("--function", required=True)
    branch = sub.add_parser("branch-map", help="compare aligned branch destination row identities")
    branch.add_argument("--strict", type=Path, required=True)
    branch.add_argument("--function", required=True)
    diagnose_parser = sub.add_parser(
        "diagnose",
        help="bounded read-only source-inference diagnosis",
    )
    diagnose_parser.add_argument("--strict", type=Path, required=True)
    diagnose_parser.add_argument("--data", type=Path)
    diagnose_parser.add_argument("--function", required=True)
    diagnose_parser.add_argument("--varinfo", type=Path)
    args = parser.parse_args(argv)
    try:
        root = Path(os.path.abspath(args.root))
        if args.action == "snapshot":
            value = snapshot(root=root, owner=args.owner, source=args.source, target=args.target_object,
                             candidate=args.candidate_object, strict=args.strict, data=args.data,
                             toolchain_key=args.toolchain_key, compile_receipt=args.compile_receipt)
            publish(root, args.out, value)
            print(json.dumps({"status": "current", "owner": value["owner"], **value["summary"],
                              "compile_binding": value["compile_binding"], "authority_advanced": False}, sort_keys=True))
        elif args.action == "verify":
            raw, _ = read_bound(root, args.index, INDEX_LIMIT)
            value = load_json(raw)
            verify(root, value)
            print(json.dumps({"status": "current", "owner": value["owner"], **value["summary"],
                              "compile_binding": value["compile_binding"], "authority_advanced": False}, sort_keys=True))
        elif args.action == "accesses":
            value = accesses(
                root=root,
                strict=args.strict,
                function=args.function,
                side=args.side,
                base_register=args.base_register,
                offset=args.offset,
                context=args.context,
                max_matches=args.max_matches,
            )
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        elif args.action == "stack-map":
            value = stack_map(root=root, strict=args.strict, function=args.function)
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        elif args.action == "diagnose":
            value = diagnose(
                root=root,
                strict=args.strict,
                data=args.data,
                function=args.function,
                varinfo=args.varinfo,
            )
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        else:
            value = branch_map(root=root, strict=args.strict, function=args.function)
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"current evidence: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
