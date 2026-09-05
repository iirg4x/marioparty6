#!/usr/bin/env python3
"""Compact current-source evidence index. No compile, history scan or retention gate.

Publish explicitly selected reports once, then verify the small index on resume.
Instruction exactness is not physical/link exactness. A supplied report is a
diagnostic input, not proof that its object was built from the supplied source.
"""
from __future__ import annotations

import argparse
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
        else:
            value = stack_map(root=root, strict=args.strict, function=args.function)
            print(json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"current evidence: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
