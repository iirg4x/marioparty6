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
from tools import recovery_object_inventory as objects

SCHEMA = "recovery_current_evidence/v1"
INDEX_LIMIT = 256 * 1024
REPORT_LIMIT = 32 * 1024 * 1024
ACCESS_SCHEMA = "recovery_accesses/v1"
STACK_MAP_SCHEMA = "recovery_stack_map/v1"
BRANCH_MAP_SCHEMA = "recovery_branch_map/v1"
DIAGNOSE_SCHEMA = "recovery_source_diagnosis/v1"
POOL_PLAN_SCHEMA = "recovery_pool_plan/v1"
DIAGNOSE_OUTPUT_LIMIT = 32 * 1024
DIAGNOSE_FOCUS_LIMIT = 8 * 1024
POOL_PLAN_OUTPUT_LIMIT = 256 * 1024
POOL_PLAN_MAX_FAMILIES = 128
POOL_PLAN_MAX_CONSUMERS = 512
VARINFO_LIMIT = INDEX_LIMIT
VARINFO_PRIORITY_COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"
VARINFO_INLINE_ASM_FLAG = 0x40
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
_FPR_RE = re.compile(r"f(?:[0-9]|[12][0-9]|3[01])\Z", re.IGNORECASE)
_SHA256_RE = re.compile(r"[0-9a-fA-F]{64}\Z")
_NUMBER_TOKEN_RE = re.compile(
    r"(?<![A-Za-z0-9_.])(?P<number>[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+))(?![A-Za-z0-9_.])"
)

_ABI_PROLOGUE_OPS = frozenset({"mflr", "stwu", "stw", "stmw"})
_ABI_EPILOGUE_OPS = frozenset({"lwz", "mtlr", "addi", "lmw", "mr"})
_STACK_ACCESS_WIDTHS = {
    "lbz": 1, "lbzu": 1, "lbzx": 1, "lbzux": 1,
    "lha": 2, "lhau": 2, "lhax": 2, "lhaux": 2,
    "lhz": 2, "lhzu": 2, "lhzx": 2, "lhzux": 2,
    "stb": 1, "stbu": 1, "stbx": 1, "stbux": 1,
    "sth": 2, "sthu": 2, "sthx": 2, "sthux": 2,
    "lwa": 4, "lwaux": 4, "lwax": 4, "lwz": 4, "lwzu": 4,
    "lwzx": 4, "lwzux": 4, "stw": 4, "stwu": 4, "stwx": 4,
    "stwux": 4, "lfs": 4, "lfsu": 4, "lfsx": 4, "lfsux": 4,
    "stfs": 4, "stfsu": 4, "stfsx": 4, "stfsux": 4, "stfiwx": 4,
    "lfd": 8, "lfdu": 8, "lfdx": 8, "lfdux": 8,
    "stfd": 8, "stfdu": 8, "stfdx": 8, "stfdux": 8,
    "psq_l": 8, "psq_lu": 8, "psq_lx": 8, "psq_lux": 8,
    "psq_st": 8, "psq_stu": 8, "psq_stx": 8, "psq_stux": 8,
    "lvx": 16, "lvxl": 16, "stvx": 16, "stvxl": 16,
}
_STACK_MEMORY_OPS = frozenset(_STACK_ACCESS_WIDTHS) | {"lmw", "stmw"}
_STACK_POINTER_OPS = frozenset({"addi", "addic", "addic.", "addis", "addis."})
_FPR_DEFINITION_OPS = frozenset({
    "fadd", "fadds", "fsub", "fsubs", "fmul", "fmuls", "fdiv", "fdivs",
    "fmadd", "fmadds", "fmsub", "fmsubs", "fnmadd", "fnmadds", "fnmsub",
    "fnmsubs", "fsel", "fmr", "fneg", "fabs", "fnabs", "frsp", "fres",
    "frsqrte", "fctiw", "fctiwz", "lfs", "lfsu", "lfsux", "lfsx", "lfd",
    "lfdu", "lfdux", "lfdx", "ps_add", "ps_sub", "ps_mul", "ps_div",
    "ps_madd", "ps_madds0", "ps_madds1", "ps_msub", "ps_nmadd",
    "ps_nmsub", "ps_mr", "ps_neg", "ps_abs", "ps_sum0", "ps_sum1", "ps_sum2",
    "ps_sum3", "ps_sel", "psq_l", "psq_lu", "psq_lux", "psq_lx",
})


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


def _branch_destination_window(rows: list[dict], start: int, *, window: int = 4) -> tuple[bytes, ...] | None:
    """Return two nearby instruction identities for a shifted branch target.

    A row index can move when a candidate inserts an instruction, while the
    branch still lands on the same operation.  One opcode is not enough to
    establish that relationship; require a second nearby instruction and keep
    the window small so an unrelated repeated opcode cannot be selected.
    Empty report rows are skipped because objdiff may leave an alignment hole
    at a target that is otherwise present on both sides.
    """
    if start < 0 or start >= len(rows) or _diagnose_payload(rows[start], start) is None:
        return None
    identities: list[bytes] = []
    for index in range(start, min(len(rows), start + window)):
        payload = _diagnose_payload(rows[index], index)
        if payload is None:
            continue
        identities.append(canonical(payload))
        if len(identities) == 2:
            return tuple(identities)
    return None


def _branch_destination_window_occurrences(
    rows: list[dict],
    expected: tuple[bytes, ...],
    *,
    window: int = 4,
) -> list[int]:
    """Return row starts whose canonical two-instruction window is ``expected``."""
    return [
        start
        for start in range(len(rows))
        if _branch_destination_window(rows, start, window=window) == expected
    ]


def _aligned_branch_destination(
    target_rows: list[dict],
    candidate_rows: list[dict],
    target_row: int | None,
    candidate_row: int | None,
) -> bool:
    """Confirm a shifted destination from a uniquely paired window anchor.

    A repeated window at another shifted row is ambiguous and must not turn a
    changed branch into an aligned one.  Windows already present at the same
    row on both sides are exact row matches, so they are removed before the
    shifted-anchor uniqueness check; this preserves independent exact matches
    in a function that happens to reuse the same two instructions.
    """
    if target_row is None or candidate_row is None or target_row == candidate_row:
        return False
    target_window = _branch_destination_window(target_rows, target_row)
    candidate_window = _branch_destination_window(candidate_rows, candidate_row)
    if target_window is None or target_window != candidate_window:
        return False
    target_occurrences = _branch_destination_window_occurrences(target_rows, target_window)
    candidate_occurrences = _branch_destination_window_occurrences(candidate_rows, candidate_window)
    exact_rows = set(target_occurrences).intersection(candidate_occurrences)
    target_shifted = [row for row in target_occurrences if row not in exact_rows]
    candidate_shifted = [row for row in candidate_occurrences if row not in exact_rows]
    return target_shifted == [target_row] and candidate_shifted == [candidate_row]


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
    same = aligned = changed = unresolved = paired = 0
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
            elif _aligned_branch_destination(target_rows, candidate_rows, target_row, candidate_row):
                # The destination row shifted, but its local instruction
                # window uniquely agrees.  This is alignment evidence, not a
                # claim that an arbitrary same-opcode target is equivalent.
                same += 1
                aligned += 1
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
        "aligned_destination_count": aligned,
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
    # Objdiff may spell the same relocation as a symbol, a numeric pool
    # identity, or a numeric identity plus addend (for example
    # ``@1131+0x4@sda21``).  Treat only that complete relocation operand as
    # report-local identity; ordinary immediates remain part of code shape.
    relocation_atom = r"(?:[A-Za-z_.$@][A-Za-z0-9_.$@]*|(?:0[xX][0-9a-fA-F]+|[0-9]+))"
    normalized = re.sub(
        rf"(?<![A-Za-z0-9_.]){relocation_atom}(?:[ \t]*[+-][ \t]*{relocation_atom})*@(sda21|ha|h|l)\b",
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
    first_instruction: dict[str, Any] | None = None
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
        # A diff annotation is itself a canonical-report residual even when
        # both sides carry the same annotation.  Previously comparing the two
        # kinds dropped these rows after relocation identity normalization,
        # making a non-100% strict report appear exact.  Keep this separate
        # from the normalized code-shape comparison above.
        annotation_present = target_kind is not None or candidate_kind is not None
        if not instruction_changed and not annotation_present:
            continue
        mismatch_count += 1
        if instruction_changed:
            instruction_mismatch_count += 1
        else:
            annotation_only_count += 1
        if first is None:
            first = {
                "row": index,
                "kind": "instruction" if instruction_changed else "annotation",
                "target": _diagnose_row(target_row, index),
                "candidate": _diagnose_row(candidate_row, index),
            }
        if instruction_changed and first_instruction is None:
            first_instruction = {
                "row": index,
                "kind": "instruction",
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
                "different": (
                    target_payload != candidate_payload
                    or _diagnose_kind(target_row) is not None
                    or _diagnose_kind(candidate_row) is not None
                ),
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
        "code_shape_exact": instruction_mismatch_count == 0,
        "canonical_report_exact": mismatch_count == 0,
        "diff_kinds": {
            "target": dict(sorted(target_kinds.items())),
            "candidate": dict(sorted(candidate_kinds.items())),
            "combined": dict(sorted((target_kinds + candidate_kinds).items())),
        },
        "first_mismatch": first,
        "first_instruction_mismatch": first_instruction,
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


def _instruction_parts(row: dict | None) -> tuple[str, list[str]] | None:
    if not isinstance(row, dict):
        return None
    instruction = row.get("instruction")
    if not isinstance(instruction, dict):
        return None
    text = instruction.get("formatted")
    if not isinstance(text, str):
        return None
    match = _OPCODE_RE.match(text)
    if match is None:
        return None
    operands = [part.strip().lower() for part in text[match.end():].split(",")]
    return match.group("opcode").lower(), operands


def _fpr_operand(value: str) -> str | None:
    value = value.strip().lower()
    return value if _FPR_RE.fullmatch(value) else None


def _stack_operand_offset(value: str) -> int | None:
    match = _MEMORY_OPERAND_RE.fullmatch(value.strip())
    if match is None or match.group("base").lower() != "r1":
        return None
    try:
        return _parse_access_offset(match.group("displacement"))
    except ValueError:
        return None


def _abi_pair(rows: list[dict], index: int, kind: str) -> dict[str, Any] | None:
    first = _instruction_parts(rows[index]) if index < len(rows) else None
    second = _instruction_parts(rows[index + 1]) if index + 1 < len(rows) else None
    if first is None or second is None:
        return None
    first_opcode, first_operands = first
    second_opcode, second_operands = second
    if kind == "save":
        if first_opcode != "stfd" or second_opcode != "psq_st":
            return None
    elif kind == "restore":
        if first_opcode != "psq_l" or second_opcode != "lfd":
            return None
    else:
        raise ValueError(f"unknown ABI pair kind: {kind}")
    if kind == "save" and (len(first_operands) != 2 or len(second_operands) != 4):
        return None
    if kind == "restore" and (len(first_operands) != 4 or len(second_operands) != 2):
        return None
    register = _fpr_operand(first_operands[0])
    second_register = _fpr_operand(second_operands[0])
    if register is None or register != second_register:
        return None
    # Only f14-f31 are callee-saved FPRs in the PowerPC ABI.  A matching
    # low-register store/load shape is ordinary body traffic, not an
    # authenticated ABI backup that may be projected away.
    register_number = int(register[1:])
    if not 14 <= register_number <= 31:
        return None
    mode_operands = second_operands if kind == "save" else first_operands
    if mode_operands[2] not in {"0", "+0", "0x0", "+0x0"} or mode_operands[3] != "qr0":
        return None
    if kind == "save":
        double_offset = _stack_operand_offset(first_operands[1])
        paired_offset = _stack_operand_offset(second_operands[1])
    else:
        paired_offset = _stack_operand_offset(first_operands[1])
        double_offset = _stack_operand_offset(second_operands[1])
    if double_offset is None or paired_offset is None or paired_offset != double_offset + 8:
        return None
    if kind == "restore":
        # The load pair reverses the store pair's width order.
        if (_stack_operand_offset(first_operands[1]) != paired_offset
                or _stack_operand_offset(second_operands[1]) != double_offset):
            return None
    return {
        "register": register,
        "rows": [index, index + 1],
        "double_offset": double_offset,
        "paired_offset": paired_offset,
    }


def _abi_pair_run(rows: list[dict], kind: str) -> tuple[list[dict[str, Any]], str | None]:
    starts = [index for index in range(max(0, len(rows) - 1))
              if _abi_pair(rows, index, kind) is not None]
    if not starts:
        return [], None
    runs: list[list[int]] = []
    run = [starts[0]]
    for start in starts[1:]:
        if start == run[-1] + 2:
            run.append(start)
        else:
            runs.append(run)
            run = [start]
    runs.append(run)
    if len(runs) != 1:
        return [], "multiple_abi_pair_runs"
    run = runs[0]
    first = run[0]
    after = run[-1] + 2
    if kind == "save":
        prefix = [_instruction_parts(row) for row in rows[:first]]
        if any(parts is None or parts[0] not in _ABI_PROLOGUE_OPS for parts in prefix):
            return [], "save_pair_not_in_prologue"
    else:
        suffix = [_instruction_parts(row) for row in rows[after:]]
        for parts in suffix:
            if parts is None:
                return [], "restore_pair_not_in_epilogue"
            opcode, operands = parts
            if opcode == "blr":
                continue
            if opcode == "bl" and any("_restgpr" in operand for operand in operands):
                continue
            if opcode not in _ABI_EPILOGUE_OPS:
                return [], "restore_pair_not_in_epilogue"
    pairs = [_abi_pair(rows, start, kind) for start in run]
    if any(pair is None for pair in pairs):
        return [], "incomplete_abi_pair_run"
    pairs = [pair for pair in pairs if pair is not None]
    offsets = [pair["double_offset"] for pair in pairs]
    if len(offsets) > 1:
        stride = offsets[1] - offsets[0]
        if stride not in {-16, 16} or any(
            offsets[index + 1] - offsets[index] != stride
            for index in range(len(offsets) - 1)
        ):
            return [], "ABI_stack_slots_not_sequential"
    return pairs, None


def _fpr_is_definition(opcode: str, operands: list[str], register: str) -> bool:
    return bool(operands and operands[0] == register and opcode in _FPR_DEFINITION_OPS)


def _stack_body_accesses(
    opcode: str,
    operands: list[str],
) -> tuple[list[tuple[int, int]], str | None]:
    """Return known r1-relative byte ranges and reject opaque stack forms."""
    direct: list[int] = []
    for operand in operands:
        match = _MEMORY_OPERAND_RE.fullmatch(operand.strip())
        if match is None or match.group("base").lower() != "r1":
            continue
        try:
            direct.append(_parse_access_offset(match.group("displacement")))
        except ValueError:
            return [], "unsupported_stack_access_form"
    if direct:
        if len(direct) != 1:
            return [], "unsupported_stack_access_form"
        if opcode in {"lmw", "stmw"}:
            if not operands or _REGISTER_RE.fullmatch(operands[0].strip()) is None:
                return [], "unsupported_stack_access_form"
            first_register = operands[0].strip().lower()
            width = (32 - int(first_register[1:])) * 4
            if width <= 0:
                return [], "unsupported_stack_access_form"
        else:
            width = _STACK_ACCESS_WIDTHS.get(opcode)
            if width is None:
                return [], "unsupported_stack_access_form"
        return [(direct[0], width)], None

    # Indexed stack traffic has an r1 operand but no statically known
    # displacement.  It cannot be proven disjoint from an ABI backup region.
    if opcode in _STACK_MEMORY_OPS and any(
        _REGISTER_RE.fullmatch(operand.strip()) is not None
        and operand.strip().lower() == "r1"
        for operand in operands
    ):
        return [], "unsupported_stack_access_form"

    # An addi based stack pointer is an address into the frame even though it
    # is not itself a load/store.  Only the canonical immediate form is safe
    # to classify; addis/addic and symbolic forms remain opaque.
    if opcode in _STACK_POINTER_OPS and len(operands) >= 2 and operands[1].strip().lower() == "r1":
        if opcode != "addi" or len(operands) != 3:
            return [], "unsupported_stack_access_form"
        try:
            offset = _parse_access_offset(operands[2])
        except ValueError:
            return [], "unsupported_stack_access_form"
        return [(offset, 1)], None
    return [], None


def _body_definition_states(
    rows: list[dict],
    excluded_rows: set[int],
    registers: set[str],
) -> tuple[dict[int, set[str]] | None, int | None]:
    """Compute must-defined selected FPRs at each reachable row.

    A linear scan can incorrectly accept a use after a conditional jump over
    its first definition.  This small direct-branch CFG keeps projection
    conservative: unresolved/indirect branches and malformed rows invalidate
    the diagnostic rather than being treated as fallthrough.
    """
    if not rows:
        return {}, None
    branches, addresses = _branch_rows(rows)
    branch_by_index = {branch["row_index"]: branch for branch in branches}
    successors: dict[int, list[int]] = {}
    terminators = {"blr", "bclr", "rfi", "rfid", "sc"}
    for index, row in enumerate(rows):
        parts = _instruction_parts(row)
        if parts is None and index not in excluded_rows:
            return None, index
        opcode = parts[0] if parts is not None else None
        # Count-register branches are indirect control flow.  A bctr/bcctr
        # may be a switch into the body, so treating it as a return would make
        # later uses look unreachable and unsafely confirm the projection.
        # bctrl remains a call with a fallthrough edge; f14-f31 are callee-save.
        if opcode in {"bctr", "bcctr"}:
            return None, index
        branch = branch_by_index.get(index)
        if branch is not None:
            destination = branch.get("destination")
            destination_rows = addresses.get(destination, []) if destination is not None else []
            if len(destination_rows) != 1:
                return None, index
            next_rows = [destination_rows[0]]
            if branch["opcode"] not in {"b", "ba"} and index + 1 < len(rows):
                next_rows.append(index + 1)
            successors[index] = sorted(set(next_rows))
        elif opcode in terminators:
            successors[index] = []
        elif index + 1 < len(rows):
            successors[index] = [index + 1]
        else:
            successors[index] = []

    reachable: set[int] = set()
    pending = [0]
    while pending:
        index = pending.pop()
        if index in reachable:
            continue
        reachable.add(index)
        pending.extend(successors[index])
    predecessors: dict[int, set[int]] = {index: set() for index in reachable}
    for index in reachable:
        for successor in successors[index]:
            if successor in reachable:
                predecessors[successor].add(index)

    def defined_after(index: int, before: set[str]) -> set[str]:
        parts = _instruction_parts(rows[index])
        if parts is None:
            return set(before)
        opcode, operands = parts
        after = set(before)
        for register in registers:
            if _fpr_is_definition(opcode, operands, register):
                after.add(register)
        return after

    in_states = {index: set(registers) for index in reachable}
    in_states[0] = set()
    # Each pass can remove at least one register from one row's incoming set;
    # bound the fixed-point work by the finite row/register lattice.
    for _ in range(max(1, len(reachable) * (len(registers) + 1) + 1)):
        changed = False
        out_states = {index: defined_after(index, in_states[index]) for index in reachable}
        for index in sorted(reachable):
            if index == 0:
                incoming = set()
            elif predecessors[index]:
                incoming = set.intersection(*(out_states[pred] for pred in predecessors[index]))
            else:
                continue
            if incoming != in_states[index]:
                in_states[index] = incoming
                changed = True
        if not changed:
            break
    else:
        return None, 0
    return in_states, None


def _body_conflicts(
    rows: list[dict],
    excluded_rows: set[int],
    registers: set[str],
    backup_regions: list[tuple[int, int]],
    side: str,
) -> list[dict[str, Any]]:
    conflicts: list[dict[str, Any]] = []
    in_states, cfg_error_row = _body_definition_states(rows, excluded_rows, registers)
    if in_states is None:
        return [{"side": side, "row": cfg_error_row, "reason": "body_control_flow_unresolved"}]
    for index, row in enumerate(rows):
        if index in excluded_rows or index not in in_states:
            continue
        parts = _instruction_parts(row)
        if parts is None:
            return [{"side": side, "row": index, "reason": "body_control_flow_unresolved"}]
        opcode, operands = parts
        accesses, access_error = _stack_body_accesses(opcode, operands)
        if access_error is not None:
            return [{"side": side, "row": index, "reason": access_error}]
        for offset, width in accesses:
            access_end = offset + width
            if any(offset < region_end and access_end > region_start
                   for region_start, region_end in backup_regions):
                conflicts.append({"side": side, "row": index, "reason": "abi_save_slot_used_by_body"})
                if len(conflicts) >= 4:
                    return conflicts
        before = in_states[index]
        _, operands = parts
        for register in sorted(registers):
            positions = [position for position, operand in enumerate(operands)
                         if operand == register]
            if not positions:
                continue
            definition = _fpr_is_definition(opcode, operands, register)
            if register not in before and not (definition and positions == [0]):
                conflicts.append({"side": side, "row": index,
                                  "register": register,
                                  "reason": "incoming_register_value_used"})
                if len(conflicts) >= 4:
                    return conflicts
    return conflicts


def _body_stack_conflicts(
    target_rows: list[dict],
    candidate_rows: list[dict],
    target_excluded: set[int],
    candidate_excluded: set[int],
) -> list[dict[str, Any]]:
    conflicts: list[dict[str, Any]] = []
    for index in range(max(len(target_rows), len(candidate_rows))):
        if index in target_excluded or index in candidate_excluded:
            continue
        target_parts = _instruction_parts(target_rows[index]) if index < len(target_rows) else None
        candidate_parts = _instruction_parts(candidate_rows[index]) if index < len(candidate_rows) else None
        target_offsets = [] if target_parts is None else [
            offset for operand in target_parts[1]
            for offset in [_stack_operand_offset(operand)]
            if offset is not None
        ]
        candidate_offsets = [] if candidate_parts is None else [
            offset for operand in candidate_parts[1]
            for offset in [_stack_operand_offset(operand)]
            if offset is not None
        ]
        if target_offsets != candidate_offsets:
            conflicts.append({"row": index, "reason": "stack_home_changed"})
            if len(conflicts) >= 4:
                break
    return conflicts


def _register_cycles(mapping: dict[str, str]) -> list[list[str]]:
    cycles: list[list[str]] = []
    visited: set[str] = set()
    for start in sorted(mapping):
        if start in visited:
            continue
        chain: list[str] = []
        current = start
        while current in mapping and current not in visited:
            visited.add(current)
            chain.append(current)
            current = mapping[current]
        if current == start and len(chain) > 1:
            cycles.append(chain + [start])
    return cycles


def _body_register_projection(target_rows: list[dict], candidate_rows: list[dict]) -> dict[str, Any]:
    empty: dict[str, Any] = {
        "status": "none",
        "reason": "no_authenticated_abi_pairs",
        "projected_cycle": [],
        "projected_mapping": {},
        "excluded_paired_rows": {"target": [], "candidate": []},
        "excluded_pair_count": 0,
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    target_saves, target_save_reason = _abi_pair_run(target_rows, "save")
    candidate_saves, candidate_save_reason = _abi_pair_run(candidate_rows, "save")
    target_restores, target_restore_reason = _abi_pair_run(target_rows, "restore")
    candidate_restores, candidate_restore_reason = _abi_pair_run(candidate_rows, "restore")
    reasons = (target_save_reason, candidate_save_reason,
               target_restore_reason, candidate_restore_reason)
    if any(reason is not None for reason in reasons):
        empty["status"] = "rejected"
        empty["reason"] = next(reason for reason in reasons if reason is not None)
        return empty
    if not target_saves or not candidate_saves or not target_restores or not candidate_restores:
        return empty
    if len(target_saves) != len(candidate_saves) or len(target_restores) != len(candidate_restores):
        empty["reason"] = "no_common_authenticated_abi_pairs"
        return empty
    for target_pair, candidate_pair in zip(target_saves, candidate_saves):
        if target_pair != candidate_pair:
            empty["status"] = "rejected"
            empty["reason"] = "save_pair_alignment_changed"
            return empty
    for target_pair, candidate_pair in zip(target_restores, candidate_restores):
        if target_pair != candidate_pair:
            empty["status"] = "rejected"
            empty["reason"] = "restore_pair_alignment_changed"
            return empty
    save_by_register = {pair["register"]: pair for pair in target_saves}
    restore_by_register = {pair["register"]: pair for pair in target_restores}
    if (len(save_by_register) != len(target_saves)
            or len(restore_by_register) != len(target_restores)
            or set(save_by_register) != set(restore_by_register)):
        empty["status"] = "rejected"
        empty["reason"] = "ABI_register_pairing_changed"
        return empty
    for register, save_pair in save_by_register.items():
        restore_pair = restore_by_register[register]
        if (save_pair["double_offset"] != restore_pair["double_offset"]
                or save_pair["paired_offset"] != restore_pair["paired_offset"]):
            empty["status"] = "rejected"
            empty["reason"] = "ABI_stack_slot_changed"
            return empty
    target_excluded = {row for pair in target_saves + target_restores for row in pair["rows"]}
    candidate_excluded = {row for pair in candidate_saves + candidate_restores for row in pair["rows"]}
    body_target = [row for index, row in enumerate(target_rows) if index not in target_excluded]
    body_candidate = [row for index, row in enumerate(candidate_rows) if index not in candidate_excluded]
    # Removing ABI save/restore rows can also remove the row that a branch
    # targets.  Re-running the branch resolver on the filtered lists would
    # therefore turn an unchanged branch into an unresolved one (and, worse,
    # report it as a changed destination).  Keep branch identity from the full
    # authenticated rows while projecting only the register body.
    branch_summary = _branch_category(target_rows, candidate_rows)
    body_comparison = _register_permutation(
        body_target,
        body_candidate,
        _include_body_projection=False,
        _branch_summary=branch_summary,
    )
    body_mapping_registers = set(body_comparison.get("changed_mapping", {}))
    body_mapping_registers.update(body_comparison.get("changed_mapping", {}).values())
    for conflict in body_comparison.get("mapping_conflicts", [])[:8]:
        for key in ("target", "candidate"):
            register = conflict.get(key)
            if isinstance(register, str):
                body_mapping_registers.add(register)
        for key in ("candidates", "targets"):
            registers = conflict.get(key)
            if isinstance(registers, list):
                body_mapping_registers.update(
                    register for register in registers if isinstance(register, str)
                )
    selected_registers = set(save_by_register) & body_mapping_registers
    if not selected_registers:
        empty["reason"] = "no_paired_registers_in_body"
        return empty
    selected_saves = [save_by_register[register] for register in sorted(selected_registers)]
    selected_restores = [restore_by_register[register] for register in sorted(selected_registers)]
    selected_rows_target = {row for pair in selected_saves + selected_restores for row in pair["rows"]}
    selected_rows_candidate = {row for pair in selected_saves + selected_restores for row in pair["rows"]}
    backup_regions = []
    for register in sorted(selected_registers):
        save_pair = save_by_register[register]
        start = min(save_pair["double_offset"], save_pair["paired_offset"])
        end = max(save_pair["double_offset"], save_pair["paired_offset"]) + 8
        backup_regions.append((start, end))
    conflicts = _body_stack_conflicts(
        target_rows, candidate_rows, selected_rows_target, selected_rows_candidate
    )
    if conflicts:
        empty["status"] = "rejected"
        empty["reason"] = conflicts[0]["reason"]
        empty["conflicts"] = conflicts[:4]
        return empty
    conflicts = _body_conflicts(
        target_rows, selected_rows_target, selected_registers, backup_regions, "target"
    )
    conflicts.extend(_body_conflicts(
        candidate_rows, selected_rows_candidate, selected_registers, backup_regions, "candidate"
    ))
    if conflicts:
        empty["status"] = "rejected"
        empty["reason"] = conflicts[0]["reason"]
        empty["conflicts"] = conflicts[:4]
        return empty
    projected_target = [row for index, row in enumerate(target_rows) if index not in selected_rows_target]
    projected_candidate = [row for index, row in enumerate(candidate_rows) if index not in selected_rows_candidate]
    projected = _register_permutation(
        projected_target,
        projected_candidate,
        _include_body_projection=False,
        _branch_summary=branch_summary,
    )
    cycles = _register_cycles(projected.get("changed_mapping", {}))
    result = dict(empty)
    result["excluded_paired_rows"] = {
        "target": sorted(selected_rows_target),
        "candidate": sorted(selected_rows_candidate),
    }
    result["excluded_pair_count"] = len(selected_registers)
    result["excluded_row_count"] = len(selected_rows_target) + len(selected_rows_candidate)
    result["status"] = projected.get("status", "rejected")
    result["reason"] = projected.get("reason", "projected_register_comparison_failed")
    if result["status"] == "confirmed":
        result["projected_mapping"] = projected.get("changed_mapping", {})
        result["projected_cycle"] = cycles[0] if cycles else []
        result["projected_cycle_count"] = len(cycles)
        if cycles:
            result["reason"] = "closed_body_register_cycle"
    return result


def _register_permutation(
    target_rows: list[dict],
    candidate_rows: list[dict],
    *,
    _include_body_projection: bool = True,
    _branch_summary: dict[str, Any] | None = None,
) -> dict[str, Any]:
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
    branch_summary = (_branch_summary if _branch_summary is not None
                      else _branch_category(target_rows, candidate_rows))
    if _diagnose_branch_status(branch_summary) not in {"exact", "none"}:
        for finding in branch_summary.get("findings", [])[:4]:
            if len(nonregister_differences) >= 8:
                break
            nonregister_differences.append({
                "row": finding.get("row_index"),
                "reason": ("branch_destination_changed"
                           if finding.get("status") == "changed"
                           else "branch_destination_unresolved"),
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
    result = {
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
    if _include_body_projection:
        result["body_projection"] = _body_register_projection(target_rows, candidate_rows)
    return result


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


def _varinfo_priority_hint(
    document: dict[str, Any],
    variables: list[dict[str, Any]],
    *,
    function_matches: bool,
    malformed_rows: bool = False,
    duplicate_names: bool = False,
) -> dict[str, Any]:
    """Explain the known GC/2.6 VarInfo flag without granting source authority."""
    compiler_sha = None
    for key in ("compiler_sha256", "compiler_sha", "compiler_hash"):
        compiler_sha = _diagnose_sha(document.get(key))
        if compiler_sha is not None:
            break
    compiler_matches = compiler_sha == VARINFO_PRIORITY_COMPILER_SHA256
    flagged: list[str] = []
    ordinary: list[str] = []
    invalid: list[str] = []
    for variable in variables[:256]:
        name = variable.get("name")
        flags = variable.get("flags")
        if (isinstance(flags, bool) or not isinstance(flags, int)
                or not 0 <= flags <= 0xFF):
            if isinstance(name, str):
                invalid.append(name)
            continue
        if flags & VARINFO_INLINE_ASM_FLAG:
            if isinstance(name, str):
                flagged.append(name)
        elif isinstance(name, str):
            ordinary.append(name)
    valid = (compiler_matches and function_matches and not invalid
             and not malformed_rows and not duplicate_names)
    if valid:
        status = "known"
        origin = "inline_assembly_operand_priority" if flagged else "ordinary_observed_usage"
        allocator_effect = "conditional_o0_usage_100000" if flagged else "not_applicable"
        declaration_reordering = (
            "conditional_o0_priority_barrier" if flagged else "ordinary_ties_only"
        )
        caution = (
            "O0 allocator effect remains conditional; no register or inline-asm recommendation."
            if flagged else "Only ordinary observed-use ties are order-sensitive."
        )
    else:
        status = "UNKNOWN"
        origin = "UNKNOWN"
        allocator_effect = "UNKNOWN"
        declaration_reordering = "UNKNOWN"
        caution = "Unknown compiler/flags/function binding; no source conclusion."
    trace_present = any(
        key in document for key in ("raw_trace", "trace", "assignment_snapshots", "capture_assignments")
    )
    return {
        "status": status,
        "compiler_sha256": compiler_sha,
        "compiler_hash_known": compiler_matches,
        "flagged_count": len(flagged),
        "flagged_names": flagged[:8],
        "ordinary_count": len(ordinary),
        "invalid_count": len(invalid),
        "origin": origin,
        "conditional_o0_usage": 100000 if valid and flagged else None,
        "optimization_unverified": True,
        "allocator_effect": allocator_effect,
        "declaration_reordering": declaration_reordering,
        "raw_trace": "unbound" if trace_present else "absent",
        "source_binding": "not_advanced",
        "caution": caution,
        "diagnostic_only": True,
        "authority_advanced": False,
    }


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
        "priority_hint": None,
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
        base["priority_hint"] = _varinfo_priority_hint({}, [], function_matches=False)
        return base
    base["path"] = dict(binding)
    if not isinstance(document, dict):
        base["status"] = "malformed"
        base["reason"] = "varinfo must be a JSON object"
        base["priority_hint"] = _varinfo_priority_hint({}, [], function_matches=False)
        return base
    varinfo_function = document.get("function") or document.get("target")
    function_matches = varinfo_function is None or varinfo_function == function
    if not function_matches:
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
    base["priority_hint"] = _varinfo_priority_hint(
        document, all_variables, function_matches=varinfo_function == function,
        malformed_rows=bool(malformed_rows), duplicate_names=bool(duplicate_names),
    )
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
            "first_instruction_mismatch": None,
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
    first_instruction_mismatch = differences.pop("first_instruction_mismatch")
    target_size = focus._integer(target.get("size"))
    candidate_size = focus._integer(candidate.get("size"))
    size_exact = target_size is not None and candidate_size is not None and target_size == candidate_size
    count_exact = differences["target_instruction_count"] == differences["candidate_instruction_count"]
    code_shape_exact = differences["code_shape_exact"]
    canonical_report_exact = differences["canonical_report_exact"]
    # Preserve the historical gate name for callers: the stream gate includes
    # report annotations, while code_shape_exact is the relocation-normalized
    # comparison used to distinguish a real instruction change.
    stream_exact = canonical_report_exact
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
            "code_shape_exact": code_shape_exact,
            "canonical_report_exact": canonical_report_exact,
            "branch_destination_exact": branch["exact"],
            "exact": exact,
        },
        "size_exact": size_exact,
        "instruction_count_exact": count_exact,
        "code_shape_exact": code_shape_exact,
        "canonical_report_exact": canonical_report_exact,
        "first_mismatch": first_mismatch,
        "first_instruction_mismatch": first_instruction_mismatch,
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
                        "code_shape_exact", "canonical_report_exact",
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


def _pool_plan_owner(census: dict[str, Any], section: str, offset: int) -> tuple[dict[str, Any] | None, str | None]:
    owners = [
        owner for owner in census.get("owners", [])
        if isinstance(owner, dict) and owner.get("section") == section
        and isinstance(owner.get("offset"), int)
        and int(owner["offset"]) <= offset < int(owner["offset"]) + int(owner.get("size_bytes", 0))
    ]
    if len(owners) > 1:
        return None, "ambiguous_duplicate_value_owner"
    if not owners:
        return None, "unknown_pool_owner"
    return owners[0], None


def _pool_plan_uses(census: dict[str, Any]) -> dict[tuple[str, int, int], list[dict[str, Any]]]:
    result: dict[tuple[str, int, int], list[dict[str, Any]]] = {}
    for owner in census.get("owners", []):
        if not isinstance(owner, dict):
            continue
        for use in owner.get("uses", []):
            if not isinstance(use, dict):
                continue
            try:
                key = (str(use["function"]), int(use["function_offset"]), int(use["relocation_type"]))
            except (KeyError, TypeError, ValueError):
                continue
            result.setdefault(key, []).append({"owner": owner, "use": use})
    return result


def _pool_plan_compact_owner(owner: dict[str, Any] | None) -> dict[str, Any] | None:
    if owner is None:
        return None
    return {
        key: owner.get(key)
        for key in ("name", "section", "offset", "size_bytes", "bytes", "bytes_sha256",
                    "bytes_complete", "writable", "consumer_count", "consumer_relocation_count")
    } | {"issues": list(owner.get("issues", []))}


def _pool_plan_issue_flags(owner: dict[str, Any] | None, owner_error: str | None, *, side: str) -> set[str]:
    flags: set[str] = set()
    if owner_error:
        flags.add(owner_error)
    if owner is None:
        flags.add("unknown_typed_use")
        return flags
    if owner.get("writable"):
        flags.add("candidate_section_flag_drift" if side == "candidate" else "writable_pool_owner")
    for issue in owner.get("issues", []):
        if not isinstance(issue, dict):
            continue
        name = str(issue.get("issue", ""))
        if name == "writable_pool_owner":
            flags.add("candidate_section_flag_drift" if side == "candidate" else "writable_pool_owner")
        elif name == "writable_use":
            flags.add("writable_pool_owner")
        elif name in {"unknown_typed_use", "typed_extent_mismatch"}:
            flags.add("unknown_typed_use")
        elif name in {"address_escaping_or_unknown_consumer", "ambiguous_consumer_function"}:
            flags.add("address_escaping")
        elif name == "ambiguous_duplicate_value_owner":
            flags.add(name)
        elif name == "use_census_limit":
            flags.add("unknown_typed_use")
    if owner.get("uses_complete") is False:
        flags.add("unknown_typed_use")
    return flags


def _pool_plan_duplicate_values(census: dict[str, Any]) -> dict[tuple[Any, ...], list[int]]:
    values: dict[tuple[Any, ...], set[int]] = {}
    for owner in census.get("owners", []):
        if not isinstance(owner, dict):
            continue
        for use in owner.get("uses", []):
            if not isinstance(use, dict):
                continue
            key = (owner.get("section"), use.get("type"), use.get("width_bytes"), use.get("bytes"))
            if None in key:
                continue
            values.setdefault(key, set()).add(int(owner.get("offset", -1)))
    return {key: sorted(offsets) for key, offsets in values.items() if len(offsets) > 1}


def _pool_plan_owner_value_key(owner: dict[str, Any]) -> tuple[int, str] | None:
    try:
        size = int(owner["size_bytes"])
    except (KeyError, TypeError, ValueError):
        return None
    value = owner.get("bytes")
    if size <= 0 or not isinstance(value, str) or not owner.get("bytes_complete", False):
        return None
    return size, value


def _pool_plan_owner_types(owner: dict[str, Any]) -> tuple[list[dict[str, Any]], set[str]]:
    typed: set[tuple[str, int]] = set()
    for use in owner.get("uses", []):
        if not isinstance(use, dict):
            continue
        value_type = use.get("type")
        width = use.get("width_bytes")
        if not isinstance(value_type, str) or not isinstance(width, int):
            continue
        typed.add((value_type, width))
    typed_rows = [
        {"type": value_type, "width_bytes": width}
        for value_type, width in sorted(typed)
    ]
    flags: set[str] = set()
    if owner.get("uses_complete") is False or not typed:
        flags.add("unknown_typed_use")
    if len(typed) > 1:
        flags.add("unknown_typed_use")
    return typed_rows, flags


def _pool_plan_lis(sequence: list[int]) -> tuple[list[int], list[list[int]]]:
    """Return one unique LIS, or bounded alternatives when tied.

    Only lengths, capped path counts, and a few predecessor indices are kept
    for each owner.  Full paths are materialized only for the at-most-three
    alternatives returned to the caller.
    """
    lengths: list[int] = []
    counts: list[int] = []
    predecessors: list[list[int]] = []
    for index, value in enumerate(sequence):
        best_length = 0
        best_predecessors: list[int] = []
        for previous, previous_value in enumerate(sequence[:index]):
            if previous_value >= value:
                continue
            candidate_length = lengths[previous]
            if candidate_length > best_length:
                best_length = candidate_length
                best_predecessors = [previous]
            elif candidate_length == best_length:
                best_predecessors.append(previous)
        if best_length == 0:
            lengths.append(1)
            counts.append(1)
            predecessors.append([])
            continue
        lengths.append(best_length + 1)
        counts.append(min(3, sum(counts[previous] for previous in best_predecessors)))
        predecessors.append(best_predecessors[:3])
    if not lengths:
        return [], []
    maximum = max(lengths)
    ends = [index for index, length in enumerate(lengths) if length == maximum]
    alternatives: list[list[int]] = []
    for end in ends:
        path = [end]
        stack = [iter(predecessors[end])]
        while path and len(alternatives) < 3:
            if not predecessors[path[-1]]:
                alternatives.append(list(reversed(path)))
                path.pop()
                stack.pop()
                continue
            previous = next(stack[-1], None)
            if previous is None:
                path.pop()
                stack.pop()
            else:
                path.append(previous)
                stack.append(iter(predecessors[previous]))
        if len(alternatives) == 3:
            break
    alternatives.sort()
    return alternatives[0] if len(alternatives) == 1 else [], alternatives


def _pool_plan_frontier_section(
    target_census: dict[str, Any],
    candidate_census: dict[str, Any],
    section: str,
) -> dict[str, Any] | None:
    target_section = target_census.get("sections", {}).get(section)
    candidate_section = candidate_census.get("sections", {}).get(section)
    if not isinstance(target_section, dict) or not isinstance(candidate_section, dict):
        return None
    if not bool(target_section.get("readonly")):
        return None
    target_owners = [
        owner for owner in target_census.get("owners", [])
        if isinstance(owner, dict) and owner.get("section") == section
    ]
    candidate_owners = [
        owner for owner in candidate_census.get("owners", [])
        if isinstance(owner, dict) and owner.get("section") == section
    ]
    target_by_key: dict[tuple[int, str], list[dict[str, Any]]] = {}
    candidate_by_key: dict[tuple[int, str], list[dict[str, Any]]] = {}
    for owner, by_key in ((target_owners, target_by_key), (candidate_owners, candidate_by_key)):
        for row in owner:
            key = _pool_plan_owner_value_key(row)
            if key is not None:
                by_key.setdefault(key, []).append(row)
    ambiguous_keys = sorted(
        repr(key) for key in set(target_by_key) & set(candidate_by_key)
        if len(target_by_key.get(key, [])) > 1 or len(candidate_by_key.get(key, [])) > 1
    )
    ambiguous_duplicate_key_count = len(ambiguous_keys)
    matched: list[tuple[dict[str, Any], dict[str, Any]]] = []
    for key in sorted(set(target_by_key) & set(candidate_by_key)):
        if len(target_by_key[key]) == 1 and len(candidate_by_key[key]) == 1:
            matched.append((target_by_key[key][0], candidate_by_key[key][0]))
    matched.sort(key=lambda pair: (
        int(pair[0].get("offset", -1)), int(pair[1].get("offset", -1)), str(pair[0].get("name", ""))
    ))
    target_offsets = [int(target.get("offset", -1)) for target, _ in matched]
    current_offsets = [int(current.get("offset", -1)) for _, current in matched]
    offset_collision = len(set(target_offsets)) != len(target_offsets) or len(set(current_offsets)) != len(current_offsets)
    if offset_collision:
        ambiguous_keys.append("offset-collision")
    selected_indices, alternatives = _pool_plan_lis(current_offsets)
    selected = set(selected_indices)
    excluded = [pair for index, pair in enumerate(matched) if index not in selected]
    if alternatives and len(alternatives) > 1:
        excluded = []
    if not matched:
        return None
    frontier_rows: list[dict[str, Any]] = []
    frontier_flags: set[str] = set()
    for target_owner, candidate_owner in excluded:
        target_types, target_type_flags = _pool_plan_owner_types(target_owner)
        candidate_types, candidate_type_flags = _pool_plan_owner_types(candidate_owner)
        flags = (_pool_plan_issue_flags(target_owner, None, side="target")
                 | _pool_plan_issue_flags(candidate_owner, None, side="candidate")
                 | target_type_flags | candidate_type_flags)
        if target_types != candidate_types:
            flags.add("unknown_typed_use")
        frontier_flags.update(flags)
        target_functions = sorted({str(use.get("function")) for use in target_owner.get("uses", [])
                                   if isinstance(use, dict) and use.get("function") is not None})
        candidate_functions = sorted({str(use.get("function")) for use in candidate_owner.get("uses", [])
                                      if isinstance(use, dict) and use.get("function") is not None})
        frontier_rows.append({
            "target_owner": _pool_plan_compact_owner(target_owner),
            "current_owner": _pool_plan_compact_owner(candidate_owner),
            "target_types": target_types,
            "current_types": candidate_types,
            "target_consumer_count": target_owner.get("consumer_count", 0),
            "current_consumer_count": candidate_owner.get("consumer_count", 0),
            "affected_functions": sorted(set(target_functions) | set(candidate_functions)),
            "flags": sorted(flags),
        })
    frontier_rows.sort(key=lambda row: (
        int((row.get("target_owner") or {}).get("offset", 1 << 60)),
        int((row.get("current_owner") or {}).get("offset", 1 << 60)),
    ))
    if ambiguous_keys:
        frontier_flags.add("ambiguous_duplicate_value_owner")
    if not target_census.get("complete") or not candidate_census.get("complete"):
        frontier_flags.add("unknown_typed_use")
    blocking_frontier_flags = {
        flag for flag in frontier_flags
        if flag not in {"candidate_section_flag_drift", "ambiguous_duplicate_value_owner"}
    }
    status = "actionable"
    if offset_collision or len(alternatives) != 1:
        status = "ambiguous"
    elif not frontier_rows:
        status = "exact_order"
    elif blocking_frontier_flags:
        status = "unresolved"
    return {
        "section": section,
        "status": status,
        "reason": "minimum_source_order_reconstruction",
        "target_section_readonly": True,
        "candidate_section_readonly": bool(candidate_section.get("readonly")),
        "candidate_section_flag_drift": bool(target_section.get("readonly")) and not bool(candidate_section.get("readonly")),
        "target_owner_count": len(target_owners),
        "current_owner_count": len(candidate_owners),
        "matched_unique_owner_count": len(matched),
        "common_order_owner_count": len(selected_indices),
        "excluded_moved_owner_count": len(frontier_rows),
        "ambiguous_key_count": ambiguous_duplicate_key_count,
        "ambiguous_duplicate_value_keys": ambiguous_keys[:16],
        "ambiguity_requires_review": bool(ambiguous_keys),
        "ambiguous_alternatives": [
            [{"target_offset": target_offsets[index], "current_offset": current_offsets[index]}
             for index in alternative]
            for alternative in alternatives[1:3]
        ],
        "flags": sorted(frontier_flags),
        "blocking_flags": sorted(blocking_frontier_flags),
        "candidate_owners": frontier_rows,
        "consumers_complete": bool(target_census.get("complete")) and bool(candidate_census.get("complete")),
    }


def _pool_plan_owner_frontier(
    target_census: dict[str, Any],
    candidate_census: dict[str, Any],
    changed_sections: set[str] | None = None,
) -> dict[str, Any] | None:
    sections = sorted(set(target_census.get("sections", {})) & set(candidate_census.get("sections", {})))
    if changed_sections is not None:
        sections = [section for section in sections if section in changed_sections]
    candidates = [
        result for section in sections
        if (result := _pool_plan_frontier_section(target_census, candidate_census, section)) is not None
        and (result.get("excluded_moved_owner_count", 0) > 0 or result.get("status") == "ambiguous")
    ]
    if not candidates:
        return None
    candidates.sort(key=lambda result: (-int(result.get("excluded_moved_owner_count", 0)), str(result.get("section"))))
    return candidates[0]


def _pool_plan_observation(
    census: dict[str, Any],
    uses: dict[tuple[str, int, int], list[dict[str, Any]]],
    function: str,
    row: dict[str, Any],
    *,
    side: str,
) -> tuple[dict[str, Any], set[str]]:
    effective = row.get("effective_target")
    if not isinstance(effective, dict) or effective.get("kind") != "section":
        return {"section": None, "offset": None, "type": None, "width_bytes": None, "bytes": None,
                "owner": None}, {"unknown_typed_use"}
    section = str(effective.get("section"))
    try:
        offset = int(effective["offset"])
    except (KeyError, TypeError, ValueError):
        return {"section": section, "offset": None, "type": None, "width_bytes": None, "bytes": None,
                "owner": None}, {"unknown_typed_use"}
    try:
        key = (function, int(row["offset"]), int(row["type"]))
    except (KeyError, TypeError, ValueError):
        key = (function, -1, -1)
    candidates = uses.get(key, [])
    if len(candidates) == 1:
        entry = candidates[0]
        use = entry["use"]
        owner = entry["owner"]
        return {
            "section": section, "offset": offset, "type": use.get("type"),
            "width_bytes": use.get("width_bytes"), "bytes": use.get("bytes"),
            "owner": owner,
        }, _pool_plan_issue_flags(owner, None, side=side)
    owner, owner_error = _pool_plan_owner(census, section, offset)
    flags = _pool_plan_issue_flags(owner, owner_error, side=side)
    flags.add("unknown_typed_use")
    return {
        "section": section, "offset": offset, "type": None, "width_bytes": None, "bytes": None,
        "owner": owner,
    }, flags


def _pool_plan_strict_scores(root: Path, strict: Path | None) -> dict[str, Any]:
    if strict is None:
        return {}
    raw, _ = read_bound(root, Path(strict), REPORT_LIMIT)
    document = load_json(raw)
    if not isinstance(document, dict):
        raise ValueError("strict report must be a JSON object")
    return {
        str(row["function"]): row.get("match_percent")
        for row in summarize(document, "strict")
        if isinstance(row.get("function"), str)
    }


def _pool_plan_bindings(
    root: Path,
    *,
    index: Path | None,
    target_object: Path | None,
    candidate_object: Path | None,
    source: Path | None,
    strict: Path | None,
) -> tuple[Path, Path, Path, dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    if index is not None:
        if any(value is not None for value in (target_object, candidate_object, source)):
            raise ValueError("pool-plan accepts either --index or explicit object/source paths")
        raw, index_binding = read_bound(root, Path(index), INDEX_LIMIT)
        index_value = load_json(raw)
        if not isinstance(index_value, dict):
            raise ValueError("current index must be a JSON object")
        verify(root, index_value)
        inputs = index_value.get("inputs")
        if not isinstance(inputs, dict):
            raise ValueError("current index lacks bound inputs")
        paths: list[Path] = []
        descs: list[dict[str, Any]] = []
        for role in ("target_object", "candidate_object", "source"):
            desc = inputs.get(role)
            if not isinstance(desc, dict) or not isinstance(desc.get("path"), str):
                raise ValueError(f"current index lacks bound {role}")
            paths.append(Path(desc["path"]))
            descs.append(dict(desc))
        scores = {str(row.get("function")): row.get("match_percent")
                  for row in index_value.get("functions", []) if isinstance(row, dict)}
        return paths[0], paths[1], paths[2], {"path": str(Path(index)), **index_binding,
            "verified": True}, descs[0], descs[1], {"source": descs[2], "strict_scores": scores}
    if target_object is None or candidate_object is None or source is None:
        raise ValueError("pool-plan requires --index or --target-object, --candidate-object and --source")
    if strict is not None:
        strict_scores = _pool_plan_strict_scores(root, strict)
    else:
        strict_scores = {}
    source_raw, source_binding = read_bound(root, Path(source), 4 * 1024 * 1024)
    _, target_binding = read_bound(root, Path(target_object), 16 * 1024 * 1024)
    _, candidate_binding = read_bound(root, Path(candidate_object), 16 * 1024 * 1024)
    return (Path(target_object), Path(candidate_object), Path(source),
            {"mode": "explicit"}, target_binding, candidate_binding,
            {"source": source_binding, "strict_scores": strict_scores})


def pool_plan(
    *,
    root: Path,
    index: Path | None = None,
    target_object: Path | None = None,
    candidate_object: Path | None = None,
    source: Path | None = None,
    strict: Path | None = None,
    max_families: int = POOL_PLAN_MAX_FAMILIES,
) -> dict[str, Any]:
    """Build a bounded, read-only shared readonly-pool action plan."""
    root = Path(os.path.abspath(root))
    if isinstance(max_families, bool) or not isinstance(max_families, int) or not 1 <= max_families <= POOL_PLAN_MAX_FAMILIES:
        raise ValueError(f"max_families must be between 1 and {POOL_PLAN_MAX_FAMILIES}")
    target_path, candidate_path, source_path, index_binding, target_binding, candidate_binding, extra = _pool_plan_bindings(
        root, index=index, target_object=target_object, candidate_object=candidate_object,
        source=source, strict=strict,
    )
    target_inventory = objects.inventory(local(root, target_path))
    candidate_inventory = objects.inventory(local(root, candidate_path))
    target_census = objects.pool_census(local(root, target_path))
    candidate_census = objects.pool_census(local(root, candidate_path))
    target_functions = target_inventory["functions"]
    candidate_functions = candidate_inventory["functions"]
    target_uses = _pool_plan_uses(target_census)
    candidate_uses = _pool_plan_uses(candidate_census)
    target_duplicate_values = _pool_plan_duplicate_values(target_census)
    candidate_duplicate_values = _pool_plan_duplicate_values(candidate_census)
    target_census_complete = bool(target_census.get("complete")) and not bool(target_census.get("issues_truncated"))
    candidate_census_complete = bool(candidate_census.get("complete")) and not bool(candidate_census.get("issues_truncated"))
    strict_scores = extra.get("strict_scores", {})
    groups: dict[tuple[Any, ...], dict[str, Any]] = {}
    label_only = 0
    raw_exact_count = 0
    physical_pool_functions: set[str] = set()
    changed_pool_sections: set[str] = set()
    strict100_physical: set[str] = set()
    for function in sorted(set(target_functions) & set(candidate_functions)):
        target_row = target_functions[function]
        candidate_row = candidate_functions[function]
        if target_row.get("raw_sha256") != candidate_row.get("raw_sha256"):
            continue
        raw_exact_count += 1
        t_rows = {(int(row["offset"]), int(row["type"])): row for row in target_row.get("physical_relocations", [])}
        c_rows = {(int(row["offset"]), int(row["type"])): row for row in candidate_row.get("physical_relocations", [])}
        for key in sorted(set(t_rows) | set(c_rows)):
            tr, cr = t_rows.get(key), c_rows.get(key)
            if tr is None or cr is None:
                continue
            if tr.get("effective_target") == cr.get("effective_target"):
                if ((tr.get("symbol") or {}).get("name") != (cr.get("symbol") or {}).get("name")):
                    label_only += 1
                continue
            tobs, tflags = _pool_plan_observation(target_census, target_uses, function, tr, side="target")
            cobs, cflags = _pool_plan_observation(candidate_census, candidate_uses, function, cr, side="candidate")
            if tobs.get("section") is None or cobs.get("section") is None:
                continue
            if tobs.get("section") != cobs.get("section"):
                continue
            physical_pool_functions.add(function)
            changed_pool_sections.add(str(tobs.get("section")))
            score = strict_scores.get(function)
            if score == 100 or score == 100.0:
                strict100_physical.add(function)
            target_owner = tobs.get("owner")
            candidate_owner = cobs.get("owner")
            flags = set(tflags) | set(cflags)
            target_bytes, candidate_bytes = tobs.get("bytes"), cobs.get("bytes")
            value_equal = target_bytes is not None and target_bytes == candidate_bytes
            type_equal = tobs.get("type") is not None and tobs.get("type") == cobs.get("type")
            width_equal = tobs.get("width_bytes") is not None and tobs.get("width_bytes") == cobs.get("width_bytes")
            if not value_equal:
                flags.add("wrong_value")
            if not type_equal or not width_equal:
                flags.add("unknown_typed_use")
            # A decoded word load is a known four-byte scalar/address-width
            # consumer.  Keep its conservative ``u32_or_address`` type in the
            # evidence, but do not call it an unknown use merely because the
            # source-level signedness is not recoverable from the instruction.
            # Unsupported opcodes, missing consumers, and address-escaping
            # owners are already reported by the actual census as blockers.
            if tobs.get("type") in {None, "unknown"} or cobs.get("type") in {None, "unknown"}:
                flags.add("unknown_typed_use")
            if target_owner is None or candidate_owner is None:
                flags.add("unknown_pool_owner")
            if not target_census_complete or not candidate_census_complete:
                flags.add("unknown_typed_use")
            group_key = (tobs.get("section"), tobs.get("type"), tobs.get("width_bytes"), target_bytes,
                         cobs.get("type"), cobs.get("width_bytes"), candidate_bytes,
                         (target_owner or {}).get("offset"), (candidate_owner or {}).get("offset"))
            group = groups.setdefault(group_key, {
                "target_observation": tobs, "candidate_observation": cobs,
                "flags": set(), "consumers": {}, "pairs": [],
            })
            group["flags"].update(flags)
            group["pairs"].append({"function": function, "target_row": key[0], "relocation_type": key[1],
                                   "target_offset": tobs.get("offset"), "candidate_offset": cobs.get("offset")})
            group["consumers"].setdefault(function, 0)
            group["consumers"][function] += 1
    ordered_groups = sorted(groups.values(), key=lambda item: (-len(item["consumers"]), -len(item["pairs"]),
                                                                str(item["target_observation"].get("bytes"))))
    families: list[dict[str, Any]] = []
    for index_number, group in enumerate(ordered_groups[:max_families], 1):
        tobs, cobs = group["target_observation"], group["candidate_observation"]
        target_owner, candidate_owner = tobs.get("owner"), cobs.get("owner")
        all_consumers = [
            {"function": name, "relocation_count": count, "nominal_strict_score": strict_scores.get(name),
             "raw_byte_exact": True}
            for name, count in sorted(group["consumers"].items())
        ]
        consumers = all_consumers[:POOL_PLAN_MAX_CONSUMERS]
        flags = sorted(group["flags"])
        consumers_truncated = len(all_consumers) > len(consumers)
        pairs_truncated = len(group["pairs"]) > POOL_PLAN_MAX_CONSUMERS
        if consumers_truncated or pairs_truncated:
            flags = sorted(set(flags) | {"consumer_census_truncated"})
        blocking_flags = [flag for flag in flags if flag != "candidate_section_flag_drift"]
        status = "actionable" if not blocking_flags and tobs.get("offset") != cobs.get("offset") else "unresolved"
        observations: list[dict[str, Any]] = []
        if (tobs.get("bytes") is not None and tobs.get("bytes") == cobs.get("bytes")
                and target_owner and candidate_owner
                and not str(target_owner.get("name", "")).startswith("@")
                and str(candidate_owner.get("name", "")).startswith("@")):
            observations.append({
                "kind": "possible_scalar_const_folding",
                "evidence": "target_named_current_compiler_anonymous_same_typed_bytes",
            })
        duplicate_key_target = (tobs.get("section"), tobs.get("type"), tobs.get("width_bytes"), tobs.get("bytes"))
        duplicate_key_candidate = (cobs.get("section"), cobs.get("type"), cobs.get("width_bytes"), cobs.get("bytes"))
        if duplicate_key_target in target_duplicate_values or duplicate_key_candidate in candidate_duplicate_values:
            observations.append({
                "kind": "late_duplicate_pool_value",
                "evidence": "same_typed_bytes_have_multiple_physical_owner_offsets",
                "target_owner_offsets": target_duplicate_values.get(duplicate_key_target, []),
                "current_owner_offsets": candidate_duplicate_values.get(duplicate_key_candidate, []),
            })
        representation = {"one_element_const_array": False, "reason": "owner_extent_or_typed_use_not_proven"}
        if (not blocking_flags and target_owner and candidate_owner and target_owner.get("size_bytes") == 4
                and candidate_owner.get("size_bytes") == 4 and tobs.get("width_bytes") == 4
                and cobs.get("width_bytes") == 4 and tobs.get("type") in {"f32", "s32", "u16", "s16"}):
            representation = {"one_element_const_array": True,
                              "reason": "real_4B_readonly_owner_and_4B_typed_consumers"}
        family = {
            "id": f"pool-family-{index_number:03d}", "status": status,
            "type": tobs.get("type"), "width_bytes": tobs.get("width_bytes"),
            "target_bytes": tobs.get("bytes"), "candidate_bytes": cobs.get("bytes"),
            "value_equal": tobs.get("bytes") is not None and tobs.get("bytes") == cobs.get("bytes"),
            "target_owner": _pool_plan_compact_owner(target_owner),
            "current_owner": _pool_plan_compact_owner(candidate_owner),
            "target_owner_offset": (target_owner or {}).get("offset"),
            "current_owner_offset": (candidate_owner or {}).get("offset"),
            "target_use_offset": tobs.get("offset"), "current_use_offset": cobs.get("offset"),
            "binding": {
                "target": {"owner_offset": (target_owner or {}).get("offset"), "use_offset": tobs.get("offset"),
                            "type": tobs.get("type"), "width_bytes": tobs.get("width_bytes"), "bytes": tobs.get("bytes")},
                "current": {"owner_offset": (candidate_owner or {}).get("offset"), "use_offset": cobs.get("offset"),
                             "type": cobs.get("type"), "width_bytes": cobs.get("width_bytes"), "bytes": cobs.get("bytes")},
            },
            "owner_offset_delta": ((target_owner or {}).get("offset") - (candidate_owner or {}).get("offset")
                                   if isinstance((target_owner or {}).get("offset"), int)
                                   and isinstance((candidate_owner or {}).get("offset"), int) else None),
            "consumer_count": len(all_consumers), "consumer_relocation_count": len(group["pairs"]),
            "consumers": consumers, "consumers_returned_count": len(consumers),
            "consumers_truncated": consumers_truncated,
            "consumer_census_complete": not consumers_truncated and not pairs_truncated,
            "affected_functions": [item["function"] for item in consumers],
            "affected_function_count": len(all_consumers),
            "pairs": group["pairs"][:POOL_PLAN_MAX_CONSUMERS],
            "pairs_returned_count": min(len(group["pairs"]), POOL_PLAN_MAX_CONSUMERS),
            "pairs_truncated": pairs_truncated, "flags": flags, "blocking_flags": blocking_flags,
            "observations": observations,
            "representation_review": representation,
            "observed_owner_classes": {
                "target": "named_or_local" if target_owner else "unknown",
                "current": "named_or_local" if candidate_owner else "unknown",
            },
            "next_action": (
                "Freeze code and restore the actual typed storage producers/consumers as one family; recompile once."
                if status == "actionable" else
                "Freeze code; resolve the flagged physical owner/type/value issue before restoring the actual typed storage producers/consumers as one family."
            ),
        }
        families.append(family)
    omitted_families = max(0, len(ordered_groups) - len(families))
    actionable = [family for family in families if family["status"] == "actionable"]
    owner_frontier = _pool_plan_owner_frontier(target_census, candidate_census, changed_pool_sections)
    highest = max(actionable or families,
                  key=lambda item: (item["consumer_count"], item["consumer_relocation_count"]),
                  default=None)
    owner_level_guidance = None
    if owner_frontier is not None:
        frontier_rows = owner_frontier.get("candidate_owners", [])
        if owner_frontier.get("status") == "ambiguous":
            owner_level_guidance = {
                "section": owner_frontier.get("section"),
                "source": "owner_frontier",
                "status": "ambiguous",
                "reason": owner_frontier.get("reason"),
                "ambiguous_alternatives": owner_frontier.get("ambiguous_alternatives", []),
                "next_action": (
                    "Freeze code; resolve the ambiguous same-typed owner ordering before restoring "
                    "any producer/consumer family."
                ),
            }
        elif frontier_rows:
            earliest_row = frontier_rows[0]
            target_owner = earliest_row.get("target_owner") or {}
            current_owner = earliest_row.get("current_owner") or {}
            section = str(owner_frontier.get("section"))
            target_offset = int(target_owner.get("offset", 0))
            same_section = [
                family for family in families
                if isinstance(family.get("target_owner"), dict)
                and str(family["target_owner"].get("section")) == section
            ]
            owner_level_guidance = {
                "section": section,
                "source": "owner_frontier",
                "status": owner_frontier.get("status"),
                "reason": owner_frontier.get("reason"),
                "earliest_displaced_target_owner": {
                    "name": target_owner.get("name"),
                    "offset": target_offset,
                    "offset_hex": f"0x{target_offset:X}",
                    "current_name": current_owner.get("name"),
                    "current_offset": current_owner.get("offset"),
                    "type": (earliest_row.get("target_types") or [{}])[0].get("type"),
                    "width_bytes": (earliest_row.get("target_types") or [{}])[0].get("width_bytes"),
                    "bytes": target_owner.get("bytes"),
                    "flags": earliest_row.get("flags", []),
                },
                "same_section_family_ids": [family["id"] for family in same_section],
                "same_section_family_count": len(same_section),
                "source_ordering": "earliest_displaced_target_owner_first",
                "consumer_leverage_is_not_source_edit_order": True,
                "next_action": (
                    f"Start with the earliest displaced target owner at {section}+0x{target_offset:X}; "
                    "batch its same-section typed storage producers/consumers as one owner family, "
                    "then freeze downstream displaced consumers and unchanged raw code. "
                    "Do not edit the highest-consumer family independently."
                ) if owner_frontier.get("status") == "actionable" else (
                    "Freeze code; resolve the owner-frontier flags before restoring the actual typed "
                    "storage producers/consumers as one family."
                ),
            }
    if owner_level_guidance is not None:
        global_next_action = owner_level_guidance["next_action"]
    elif actionable:
        global_next_action = "Freeze code and restore the actual typed storage producers/consumers as one family; recompile once."
    elif families:
        global_next_action = "Freeze code; resolve the flagged physical owner/type/value issue before restoring the actual typed storage producers/consumers as one family."
    else:
        global_next_action = "No actionable unresolved readonly-pool family; keep code frozen and refresh after the next physical change."
    result = {
        "schema": POOL_PLAN_SCHEMA, "schema_version": 1,
        "mode": "index" if index is not None else "explicit",
        "inputs": {"index": index_binding if index is not None else None,
                   "source": {"path": str(source_path), "binding": extra["source"]},
                   "target_object": target_binding, "candidate_object": candidate_binding},
        "summary": {
            "target_function_count": len(target_functions), "candidate_function_count": len(candidate_functions),
            "raw_byte_exact_function_count": raw_exact_count,
            "physical_pool_difference_function_count": len(physical_pool_functions),
            "raw_byte_exact_pool_function_count": len(physical_pool_functions),
            "physical_pool_difference_count": sum(len(group["pairs"]) for group in ordered_groups),
            "nominal_strict100_physical_difference_function_count": len(strict100_physical),
            "strict100_hidden_physical_pool_case_count": len(strict100_physical),
            "pool_family_count": len(ordered_groups), "returned_family_count": len(families),
            "omitted_family_count": omitted_families, "actionable_family_count": len(actionable),
            "unresolved_family_count": len(families) - len(actionable),
            "label_only_excluded_count": label_only,
            "target_pool_census_complete": target_census_complete,
            "candidate_pool_census_complete": candidate_census_complete,
            "target_object_sha256": target_inventory["object"]["sha256"],
            "candidate_object_sha256": candidate_inventory["object"]["sha256"],
        },
        "families": families,
        "owner_frontier": owner_frontier,
        "pool_census": {
            "target": {
                "complete": target_census_complete,
                "owner_count": len(target_census.get("owners", [])),
                "issue_count": target_census.get("issue_count", 0),
                "issues_returned_count": target_census.get("issues_returned_count", 0),
                "issues_truncated": bool(target_census.get("issues_truncated")),
            },
            "current": {
                "complete": candidate_census_complete,
                "owner_count": len(candidate_census.get("owners", [])),
                "issue_count": candidate_census.get("issue_count", 0),
                "issues_returned_count": candidate_census.get("issues_returned_count", 0),
                "issues_truncated": bool(candidate_census.get("issues_truncated")),
            },
        },
        "highest_leverage_shared_owner_family": None if highest is None else {
            "id": highest["id"], "status": highest["status"],
            "consumer_count": highest["consumer_count"],
            "consumer_relocation_count": highest["consumer_relocation_count"],
            "type": highest["type"], "target_bytes": highest["target_bytes"],
            "candidate_bytes": highest["candidate_bytes"],
            "target_owner_offset": highest["target_owner_offset"],
            "current_owner_offset": highest["current_owner_offset"],
            "affected_functions": highest["affected_functions"],
            "selection_scope": "actionable" if actionable else "unresolved",
            "source_edit_order": "owner_offset_ascending",
            "consumer_leverage_is_not_source_edit_order": True,
        },
        "owner_level_guidance": owner_level_guidance,
        "next_action": global_next_action,
        "diagnostic_only": True, "physical_proof": False, "source_emission_authorized": False,
        "promotion_authorized": False, "authority_advanced": False,
    }
    if len(canonical(result)) + 1 > POOL_PLAN_OUTPUT_LIMIT:
        raise ValueError("pool-plan result exceeds 256 KiB; reduce the selected family scope")
    return result


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
    from tools import recovery_evaluate
    evaluate_parser = sub.add_parser("evaluate", help="automate private candidate compile, duplicate detection and whole-owner checks")
    recovery_evaluate.add_arguments(evaluate_parser)
    batch_parser = sub.add_parser("evaluate-batch", help="measure independent candidates concurrently against one current owner frontier")
    recovery_evaluate.add_batch_arguments(batch_parser)
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
    pool_parser = sub.add_parser("pool-plan", help="plan one shared readonly-pool owner family from ELF facts")
    pool_parser.add_argument("--index", type=Path)
    pool_parser.add_argument("--source", type=Path)
    pool_parser.add_argument("--target-object", "--target", dest="target_object", type=Path)
    pool_parser.add_argument("--candidate-object", "--candidate", dest="candidate_object", type=Path)
    pool_parser.add_argument("--strict", type=Path, help="optional strict report for explicit nominal scores")
    pool_parser.add_argument("--max-families", type=int, default=POOL_PLAN_MAX_FAMILIES)
    args = parser.parse_args(argv)
    try:
        root = Path(os.path.abspath(args.root))
        if args.action == "evaluate":
            return recovery_evaluate.dispatch(args)
        elif args.action == "evaluate-batch":
            return recovery_evaluate.dispatch_batch(args)
        elif args.action == "snapshot":
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
        elif args.action == "pool-plan":
            value = pool_plan(
                root=root,
                index=args.index,
                target_object=args.target_object,
                candidate_object=args.candidate_object,
                source=args.source,
                strict=args.strict,
                max_families=args.max_families,
            )
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        else:
            value = branch_map(root=root, strict=args.strict, function=args.function)
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        return 0
    except (OSError, ValueError, KeyError, TypeError, objects.ObjectInventoryError) as exc:
        print(f"current evidence: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
