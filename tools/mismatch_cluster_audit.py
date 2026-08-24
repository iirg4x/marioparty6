#!/usr/bin/env python3
"""Cluster objdiff residuals into bounded source-shape hypotheses.

This tool is deliberately conservative.  It consumes objdiff's JSON report
and, optionally, the two disassemblies used to produce it.  It reports
structural signals only: stack offsets, opcodes, branch shape, aggregate
loads/stores, and relocation metadata.  It never assigns a C variable name or
claims source provenance from assembly.

The primary input produced by objdiff has ``left`` and ``right`` sides, each
containing a ``symbols`` array.  A small ``functions`` comparison form is also
accepted for fixtures and for callers that have already paired symbols.

``--focus-symbol`` restricts the audit to one unambiguously paired function;
missing, duplicate, or one-sided focus symbols fail closed.  Function residuals
from pairs whose target and candidate both report 100% are skipped by default;
``--include-exact-residuals`` retains the historical compatibility behavior.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence


SCHEMA_VERSION = 2
DEFAULT_MAX_HYPOTHESES = 20
MAX_HYPOTHESES = 1000
_DIFF_PREFIX = "DIFF_"
_NUMBER = r"[+-]?(?:0[xX][0-9a-fA-F]+|\d+)"
_STACK_RE = re.compile(rf"(?P<offset>{_NUMBER})\s*\(\s*r1\s*\)", re.IGNORECASE)
_MNEMONIC_RE = re.compile(r"^\s*([A-Za-z][A-Za-z0-9_.]*)")
_ASM_COMMENT_RE = re.compile(r"^\s*/\*\s*(?P<address>0[xX][0-9a-fA-F]+|[0-9A-Fa-f]{6,16})\s+[^*]*\*/\s*(?P<text>.*)$")
_ASM_COLON_RE = re.compile(r"^\s*(?P<address>0[xX][0-9a-fA-F]+|[0-9A-Fa-f]{6,16})\s*:\s*(?P<text>.*)$")
_ASM_FN_RE = re.compile(r"^\s*\.fn\s+(?P<name>[A-Za-z_.$][\w.$@]*)")
_ASM_ENDFN_RE = re.compile(r"^\s*\.endfn\b")

_BRANCH_MNEMONICS = frozenset(
    {
        "b",
        "ba",
        "bc",
        "bca",
        "bcl",
        "bcla",
        "beq",
        "beqa",
        "bge",
        "bgea",
        "bgt",
        "bgta",
        "ble",
        "blea",
        "blt",
        "blta",
        "bne",
        "bnea",
        "bso",
        "bns",
        "bdnz",
        "bdz",
        "bflr",
    }
)
_UNCONDITIONAL_BRANCH_MNEMONICS = frozenset({"b", "ba"})
_CONDITIONAL_BRANCH_MNEMONICS = _BRANCH_MNEMONICS - _UNCONDITIONAL_BRANCH_MNEMONICS
_SIGN_EXT_MNEMONICS = frozenset({"extsb", "extsh", "extsb.", "extsh."})
_LOAD_MNEMONICS = frozenset(
    {"lbz", "lbzx", "lhz", "lhzx", "lha", "lhax", "lwz", "lwzx", "lha", "lhax"}
)
_AGGREGATE_MNEMONICS = frozenset(
    {
        "stfs",
        "stfd",
        "psq_st",
        "lfs",
        "lfd",
        "psq_l",
        "lwz",
        "lwzx",
        "stw",
        "stwx",
        "lhz",
        "lhzx",
        "lha",
        "lhax",
        "sth",
        "sthx",
        "lbz",
        "lbzx",
        "stb",
        "stbx",
    }
)
_STORE_MNEMONICS = frozenset(
    {
        "stfs",
        "stfd",
        "psq_st",
        "stw",
        "stwx",
        "sth",
        "sthx",
        "stb",
        "stbx",
    }
)
_INTEGER_LOAD_MNEMONICS = frozenset(
    {"lbz", "lbzx", "lhz", "lhzx", "lha", "lhax", "lwz", "lwzx"}
)


class AuditInputError(ValueError):
    """An input cannot be safely interpreted."""

    def __init__(self, code: str, message: str) -> None:
        super().__init__(message)
        self.code = code
        self.message = message


@dataclass(frozen=True)
class Instruction:
    index: int
    diff_kind: str | None
    address: int | None
    size: int | None
    formatted: str
    mnemonic: str
    relocation: Mapping[str, Any] | None
    branch_dest: int | None
    has_instruction: bool
    raw: Mapping[str, Any]


@dataclass(frozen=True)
class FunctionPair:
    name: str
    target: Mapping[str, Any] | None
    candidate: Mapping[str, Any] | None
    target_index: int | None = None
    candidate_index: int | None = None


@dataclass(frozen=True)
class AsmInstruction:
    address: int
    text: str
    mnemonic: str
    function: str | None
    line: int


@dataclass
class AssemblyIndex:
    path: str
    instructions: list[AsmInstruction]
    functions: dict[str, list[AsmInstruction]]

    def context_for(self, function_name: str, function_address: int | None) -> dict[str, Any]:
        """Return bounded, structural assembly context for one function."""

        lines = self.functions.get(function_name, [])
        result: dict[str, Any] = {
            "path": self.path,
            "parsed_instruction_count": len(self.instructions),
            "function_label_found": bool(self.functions.get(function_name)),
        }
        if lines:
            result["function_instruction_count"] = len(lines)
            result["function_address"] = lines[0].address
        return result

    def snippets(
        self,
        function_name: str,
        function_address: int | None,
        offsets: Iterable[int],
        *,
        limit: int = 16,
    ) -> list[dict[str, Any]]:
        """Find snippets by function-relative offset, never by C meaning."""

        lines = self.functions.get(function_name, [])
        if not lines:
            return []
        base = lines[0].address
        wanted = set(offsets)
        found: list[dict[str, Any]] = []
        for item in lines:
            if item.address - base in wanted:
                found.append(
                    {
                        "address": item.address,
                        "offset": item.address - base,
                        "line": item.line,
                        "text": item.text,
                    }
                )
                if len(found) >= limit:
                    break
        return found


def _parse_number(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        if math.isfinite(value) and value.is_integer():
            return int(value)
        return None
    if not isinstance(value, str):
        return None
    text = value.strip()
    if not text:
        return None
    try:
        if text.lower().startswith(("+0x", "-0x")):
            sign = -1 if text.startswith("-") else 1
            return sign * int(text[3:], 16)
        if text.lower().startswith("0x"):
            return int(text, 16)
        return int(text, 10)
    except ValueError:
        return None


def _parse_asm_address(value: str) -> int | None:
    """Assembly addresses without a 0x prefix are conventionally hexadecimal."""

    text = value.strip()
    if text.lower().startswith("0x"):
        return _parse_number(text)
    if re.fullmatch(r"[0-9A-Fa-f]{6,16}", text):
        try:
            return int(text, 16)
        except ValueError:
            return None
    return _parse_number(text)


def _mnemonic(formatted: str) -> str:
    match = _MNEMONIC_RE.match(formatted)
    return match.group(1).lower() if match else ""


def _normalize_relocation(value: Any) -> Mapping[str, Any] | None:
    if not isinstance(value, Mapping):
        return None
    result: dict[str, Any] = {}
    for key in ("type", "type_name", "addend", "offset"):
        if key in value:
            result[key] = value[key]
    # Symbol-table indexes differ between target and candidate objects.  A
    # textual target is useful evidence; numeric target_symbol values are not.
    target = value.get("target_name", value.get("symbol"))
    if isinstance(target, str):
        result["target_name"] = target
    return result or None


def _normalize_instruction(raw: Any, index: int) -> Instruction:
    if not isinstance(raw, Mapping):
        raise AuditInputError("invalid_instruction", f"instruction entry {index} is not an object")
    diff_kind = raw.get("diff_kind")
    if diff_kind is not None and not isinstance(diff_kind, str):
        raise AuditInputError("invalid_diff_kind", f"instruction entry {index} has a non-string diff_kind")
    nested = raw.get("instruction")
    if nested is None and any(key in raw for key in ("formatted", "address", "parts")):
        nested = raw
    if nested is None:
        return Instruction(index, diff_kind, None, None, "", "", None, None, False, raw)
    if not isinstance(nested, Mapping):
        raise AuditInputError("invalid_instruction", f"instruction entry {index} has a non-object instruction")
    formatted_value = nested.get("formatted", "")
    if formatted_value is None:
        formatted_value = ""
    if not isinstance(formatted_value, str):
        raise AuditInputError("invalid_instruction_text", f"instruction entry {index} has non-string formatted text")
    address = _parse_number(nested.get("address"))
    size = _parse_number(nested.get("size"))
    relocation = _normalize_relocation(nested.get("relocation", raw.get("relocation")))
    branch_dest = _parse_number(nested.get("branch_dest", nested.get("branch_target")))
    return Instruction(
        index=index,
        diff_kind=diff_kind,
        address=address,
        size=size,
        formatted=formatted_value,
        mnemonic=_mnemonic(formatted_value),
        relocation=relocation,
        branch_dest=branch_dest,
        has_instruction=True,
        raw=raw,
    )


def _function_symbols(side: Mapping[str, Any], side_name: str) -> list[Mapping[str, Any]]:
    symbols = side.get("symbols")
    if isinstance(symbols, Mapping):
        symbols = list(symbols.values())
    if not isinstance(symbols, list):
        raise AuditInputError("missing_symbols", f"{side_name} side has no symbols array")
    result: list[Mapping[str, Any]] = []
    for index, symbol in enumerate(symbols):
        if not isinstance(symbol, Mapping):
            raise AuditInputError("invalid_symbol", f"{side_name} symbol {index} is not an object")
        instructions = symbol.get("instructions")
        kind = str(symbol.get("kind", ""))
        if instructions is None and "FUNCTION" not in kind.upper():
            continue
        if not isinstance(instructions, list):
            raise AuditInputError("missing_instructions", f"{side_name} function {index} has no instructions array")
        result.append(symbol)
    return result


def _symbol_name(symbol: Mapping[str, Any], fallback: str) -> str:
    for key in ("name", "symbol", "formatted"):
        value = symbol.get(key)
        if isinstance(value, str) and value.strip():
            return value.strip()
    return fallback


def _direct_function_pairs(document: Mapping[str, Any]) -> list[FunctionPair] | None:
    functions = document.get("functions", document.get("comparisons"))
    if not isinstance(functions, list):
        return None
    pairs: list[FunctionPair] = []
    for index, item in enumerate(functions):
        if not isinstance(item, Mapping):
            raise AuditInputError("invalid_function", f"function comparison {index} is not an object")
        name = item.get("name", item.get("function"))
        if not isinstance(name, str) or not name.strip():
            raise AuditInputError("missing_function_name", f"function comparison {index} has no name")
        target = item.get("target", item.get("left"))
        candidate = item.get("candidate", item.get("right"))
        if target is None and "instructions" in item:
            target = item
            candidate = item.get("candidate")
        for side_name, value in (("target", target), ("candidate", candidate)):
            if value is not None:
                if not isinstance(value, Mapping):
                    raise AuditInputError("invalid_function", f"{name} {side_name} side is not an object")
                if not isinstance(value.get("instructions"), list):
                    raise AuditInputError("missing_instructions", f"{name} {side_name} side has no instructions array")
        pairs.append(FunctionPair(name.strip(), target, candidate, index if target is not None else None, index if candidate is not None else None))
    return pairs


def _paired_functions(document: Mapping[str, Any]) -> list[FunctionPair]:
    direct = _direct_function_pairs(document)
    if direct is not None:
        return sorted(direct, key=lambda pair: (pair.name, pair.target_index or -1, pair.candidate_index or -1))
    left = document.get("left")
    right = document.get("right")
    if not isinstance(left, Mapping) or not isinstance(right, Mapping):
        raise AuditInputError("unsupported_shape", "expected objdiff left/right sides or functions array")
    left_symbols = _function_symbols(left, "left")
    right_symbols = _function_symbols(right, "right")
    grouped_left: dict[str, list[tuple[int, Mapping[str, Any]]]] = defaultdict(list)
    grouped_right: dict[str, list[tuple[int, Mapping[str, Any]]]] = defaultdict(list)
    for index, symbol in enumerate(left_symbols):
        grouped_left[_symbol_name(symbol, f"<unnamed:left:{index}>")].append((index, symbol))
    for index, symbol in enumerate(right_symbols):
        grouped_right[_symbol_name(symbol, f"<unnamed:right:{index}>")].append((index, symbol))
    pairs: list[FunctionPair] = []
    for name in sorted(set(grouped_left) | set(grouped_right)):
        left_group = grouped_left.get(name, [])
        right_group = grouped_right.get(name, [])
        for occurrence in range(max(len(left_group), len(right_group))):
            left_item = left_group[occurrence] if occurrence < len(left_group) else (None, None)
            right_item = right_group[occurrence] if occurrence < len(right_group) else (None, None)
            pairs.append(FunctionPair(name, left_item[1], right_item[1], left_item[0], right_item[0]))
    return sorted(
        pairs,
        key=lambda pair: (
            pair.name,
            (_parse_number(pair.target.get("address")) or -1) if pair.target else -1,
            (_parse_number(pair.candidate.get("address")) or -1) if pair.candidate else -1,
        ),
    )


def _focus_text(value: Any) -> str:
    if not isinstance(value, str) or not value.strip():
        raise AuditInputError("invalid_focus_symbol", "focus symbol must be non-empty text")
    return value.strip()


def _focus_pairs(
    pairs: Sequence[FunctionPair], focus_symbol: str | None
) -> list[FunctionPair]:
    """Select one paired function, failing closed for unsafe focus requests."""

    if focus_symbol is None:
        return list(pairs)
    focus = _focus_text(focus_symbol)
    matches = [pair for pair in pairs if pair.name == focus]
    if not matches:
        raise AuditInputError(
            "focus_not_found", f"focus symbol {focus!r} was not found in the report"
        )
    if len(matches) != 1:
        raise AuditInputError(
            "focus_ambiguous",
            f"focus symbol {focus!r} has {len(matches)} report pairings; exactly one is required",
        )
    pair = matches[0]
    if pair.target is None or pair.candidate is None:
        side = "target" if pair.target is None else "candidate"
        raise AuditInputError(
            "focus_one_sided",
            f"focus symbol {focus!r} is present only on the {side} side",
        )
    return [pair]


def _match_percent(value: Any) -> float | None:
    if isinstance(value, bool) or value is None:
        return None
    if isinstance(value, (int, float)):
        result = float(value)
        return result if math.isfinite(result) else None
    if isinstance(value, str):
        try:
            result = float(value.strip())
        except ValueError:
            return None
        return result if math.isfinite(result) else None
    return None


def _is_exact_pair(pair: FunctionPair) -> bool:
    """Return true only when both authenticated sides explicitly report 100%."""

    if pair.target is None or pair.candidate is None:
        return False
    return (
        _match_percent(pair.target.get("match_percent")) == 100.0
        and _match_percent(pair.candidate.get("match_percent")) == 100.0
    )


def _entries(symbol: Mapping[str, Any] | None, side: str, function_name: str) -> list[Instruction]:
    if symbol is None:
        return []
    raw_entries = symbol.get("instructions")
    if not isinstance(raw_entries, list):
        raise AuditInputError("missing_instructions", f"{side} {function_name} has no instructions array")
    return [_normalize_instruction(item, index) for index, item in enumerate(raw_entries)]


def _is_diff_kind(value: str | None) -> bool:
    return bool(value and value.startswith(_DIFF_PREFIX) and value not in {"DIFF_MATCH", "DIFF_NONE"})


def _relocation_diff(left: Instruction, right: Instruction) -> bool:
    if left.relocation is None and right.relocation is None:
        return False
    return left.relocation != right.relocation


def _branch_relative(item: Instruction) -> int | None:
    if item.branch_dest is None or item.address is None:
        return None
    return item.branch_dest - item.address


def _instruction_mismatch(left: Instruction | None, right: Instruction | None) -> bool:
    if left is None or right is None:
        return True
    if _is_diff_kind(left.diff_kind) or _is_diff_kind(right.diff_kind):
        return True
    if left.has_instruction != right.has_instruction:
        return True
    if not left.has_instruction and not right.has_instruction:
        return False
    if left.formatted.strip() != right.formatted.strip():
        return True
    if left.mnemonic in _BRANCH_MNEMONICS or right.mnemonic in _BRANCH_MNEMONICS:
        if _branch_relative(left) != _branch_relative(right):
            return True
    return _relocation_diff(left, right)


def _paired_records(target: Sequence[Instruction], candidate: Sequence[Instruction]) -> list[tuple[Instruction | None, Instruction | None]]:
    return [
        (target[index] if index < len(target) else None, candidate[index] if index < len(candidate) else None)
        for index in range(max(len(target), len(candidate)))
    ]


def _cluster_records(
    target: Sequence[Instruction], candidate: Sequence[Instruction]
) -> list[list[tuple[Instruction | None, Instruction | None]]]:
    clusters: list[list[tuple[Instruction | None, Instruction | None]]] = []
    current: list[tuple[Instruction | None, Instruction | None]] = []
    for left, right in _paired_records(target, candidate):
        mismatch = _instruction_mismatch(left, right)
        if mismatch:
            current.append((left, right))
        elif current:
            clusters.append(current)
            current = []
    if current:
        clusters.append(current)
    return clusters


def _stack_offset(item: Instruction | None) -> int | None:
    if item is None:
        return None
    match = _STACK_RE.search(item.formatted)
    return _parse_number(match.group("offset")) if match else None


def _arg_registers(item: Instruction | None) -> set[str]:
    if item is None:
        return set()
    return set(re.findall(r"\b[rRfFqQ](?:[0-9]|[12][0-9]|3[01])\b", item.formatted))


def _instruction_summary(item: Instruction | None) -> dict[str, Any]:
    if item is None:
        return {"present": False}
    result: dict[str, Any] = {
        "present": item.has_instruction,
        "index": item.index,
        "address": item.address,
        "formatted": item.formatted,
        "mnemonic": item.mnemonic,
        "diff_kind": item.diff_kind,
    }
    offset = _stack_offset(item)
    if offset is not None:
        result["stack_offset"] = offset
    if item.relocation is not None:
        result["relocation"] = dict(item.relocation)
    relative = _branch_relative(item)
    if relative is not None:
        result["branch_relative_target"] = relative
    return result


def _counts(items: Iterable[str]) -> dict[str, int]:
    return {key: count for key, count in sorted(Counter(items).items())}


def _validate_max_hypotheses(value: Any) -> int:
    """Validate the bounded compact-output hypothesis limit."""

    if isinstance(value, bool) or not isinstance(value, int):
        raise AuditInputError(
            "invalid_max_hypotheses",
            f"max_hypotheses must be an integer from 1 through {MAX_HYPOTHESES}",
        )
    if value < 1 or value > MAX_HYPOTHESES:
        raise AuditInputError(
            "invalid_max_hypotheses",
            f"max_hypotheses must be from 1 through {MAX_HYPOTHESES}; got {value}",
        )
    return value


def _compact_hypothesis(hypothesis: Mapping[str, Any]) -> dict[str, Any]:
    """Keep ranked structural evidence while dropping instruction-level bulk."""

    return {
        key: value
        for key, value in hypothesis.items()
        if key not in {"instruction_pairs", "instruction_pairs_truncated"}
    }


def _classification_counts(hypotheses: Iterable[Mapping[str, Any]]) -> dict[str, int]:
    return _counts(
        classification
        for hypothesis in hypotheses
        for classification in [hypothesis.get("classification")]
        if isinstance(classification, str)
    )


def _candidate_hypotheses(
    records: Sequence[tuple[Instruction | None, Instruction | None]],
) -> tuple[str, float, list[dict[str, Any]], dict[str, Any]]:
    target_items = [left for left, _ in records if left is not None]
    candidate_items = [right for _, right in records if right is not None]
    all_items = target_items + candidate_items
    target_ops = [item.mnemonic for item in target_items if item.has_instruction]
    candidate_ops = [item.mnemonic for item in candidate_items if item.has_instruction]
    all_ops = set(target_ops) | set(candidate_ops)
    diff_kinds = [item.diff_kind for item in all_items if _is_diff_kind(item.diff_kind)]
    target_offsets = [_stack_offset(item) for item in target_items]
    candidate_offsets = [_stack_offset(item) for item in candidate_items]
    offset_pairs = [
        (left, right)
        for left, right in zip(target_offsets, candidate_offsets)
        if left is not None and right is not None
    ]
    deltas = [left - right for left, right in offset_pairs]
    nonzero_deltas = [delta for delta in deltas if delta != 0]
    uniform_delta = bool(nonzero_deltas) and len(set(nonzero_deltas)) == 1 and len(nonzero_deltas) >= 2
    target_ext = [item.mnemonic for item in target_items if item.mnemonic in _SIGN_EXT_MNEMONICS]
    candidate_ext = [item.mnemonic for item in candidate_items if item.mnemonic in _SIGN_EXT_MNEMONICS]
    ext_asymmetry = bool(target_ext) != bool(candidate_ext) or target_ext != candidate_ext
    branch_items = [item for item in all_items if item.mnemonic in _BRANCH_MNEMONICS]
    branch_asymmetry = any(
        left is None
        or right is None
        or left.mnemonic != right.mnemonic
        or _branch_relative(left) != _branch_relative(right)
        for left, right in records
        if (left is not None and left.mnemonic in _BRANCH_MNEMONICS)
        or (right is not None and right.mnemonic in _BRANCH_MNEMONICS)
    )
    integer_load = bool(all_ops & _INTEGER_LOAD_MNEMONICS)
    aggregate_ops = all_ops & _AGGREGATE_MNEMONICS
    stores = all_ops & _STORE_MNEMONICS
    loads = aggregate_ops - stores
    reloc_asymmetry = any(_relocation_diff(left, right) for left, right in records if left and right)
    reloc_asymmetry = reloc_asymmetry or any(
        (left is None and right is not None and right.relocation is not None)
        or (right is None and left is not None and left.relocation is not None)
        for left, right in records
    )

    candidates: list[dict[str, Any]] = []
    if uniform_delta:
        candidates.append(
            {
                "classification": "stack_home_uniform_delta",
                "confidence": 0.9 if len(nonzero_deltas) >= 3 else 0.84,
                "basis": f"{len(nonzero_deltas)} paired r1-relative accesses share target-minus-candidate delta {nonzero_deltas[0]}",
            }
        )
    if ext_asymmetry and (target_ext or candidate_ext or integer_load):
        candidates.append(
            {
                "classification": "missing_sign_extension_or_prototype",
                "confidence": 0.91 if target_ext or candidate_ext else 0.68,
                "basis": "asymmetric integer sign-extension opcode near a load/argument mismatch",
            }
        )
    if branch_asymmetry:
        candidates.append(
            {
                "classification": "branch_shape",
                "confidence": 0.9,
                "basis": "conditional branch opcode or function-relative branch target differs",
            }
        )
    if len(aggregate_ops) >= 2 and (stores and loads or len(records) >= 2):
        candidates.append(
            {
                "classification": "aggregate_copy_or_lifetime",
                "confidence": 0.82 if stores and loads else 0.7,
                "basis": "multiple aggregate-sized stack load/store opcodes occur in the residual cluster",
            }
        )
    if reloc_asymmetry:
        candidates.append(
            {
                "classification": "relocation_or_data_mismatch",
                "confidence": 0.88,
                "basis": "relocation presence or typed relocation metadata differs",
            }
        )
    if not candidates:
        candidates.append(
            {
                "classification": "unknown",
                "confidence": 0.2,
                "basis": "residual does not meet a supported structural signal",
            }
        )
    order = {
        "relocation_or_data_mismatch": 0,
        "branch_shape": 1,
        "missing_sign_extension_or_prototype": 2,
        "stack_home_uniform_delta": 3,
        "aggregate_copy_or_lifetime": 4,
        "unknown": 5,
    }
    candidates.sort(key=lambda item: (-float(item["confidence"]), order[item["classification"]]))
    primary = candidates[0]
    evidence = {
        "diff_kinds": _counts(diff_kinds),
        "target_mnemonics": _counts(target_ops),
        "candidate_mnemonics": _counts(candidate_ops),
        "target_stack_offsets": sorted({offset for offset in target_offsets if offset is not None}),
        "candidate_stack_offsets": sorted({offset for offset in candidate_offsets if offset is not None}),
        "stack_deltas": deltas,
        "asymmetric_sign_extension": {"target": sorted(set(target_ext)), "candidate": sorted(set(candidate_ext))},
        "branch_mnemonics": sorted({item.mnemonic for item in branch_items}),
        "relocation_signal": reloc_asymmetry,
    }
    return str(primary["classification"]), float(primary["confidence"]), candidates, evidence


def _range_value(items: Sequence[Instruction | None], attribute: str) -> tuple[int | None, int | None]:
    values = [getattr(item, attribute) for item in items if item is not None and getattr(item, attribute) is not None]
    return (min(values), max(values)) if values else (None, None)


def _cluster_result(
    name: str,
    cluster: Sequence[tuple[Instruction | None, Instruction | None]],
    target_symbol: Mapping[str, Any] | None,
    candidate_symbol: Mapping[str, Any] | None,
    target_asm: AssemblyIndex | None,
    candidate_asm: AssemblyIndex | None,
) -> dict[str, Any]:
    target_items = [left for left, _ in cluster]
    candidate_items = [right for _, right in cluster]
    classification, confidence, alternatives, evidence = _candidate_hypotheses(cluster)
    target_range = _range_value(target_items, "address")
    candidate_range = _range_value(candidate_items, "address")
    indices = [item.index for item in target_items + candidate_items if item is not None]
    result: dict[str, Any] = {
        "function": name,
        "index_start": min(indices) if indices else None,
        "index_end": max(indices) if indices else None,
        "target_address_start": target_range[0],
        "target_address_end": target_range[1],
        "candidate_address_start": candidate_range[0],
        "candidate_address_end": candidate_range[1],
        "classification": classification,
        "confidence": confidence,
        "diff_pair_count": len(cluster),
        "alternatives": alternatives,
        "evidence": evidence,
        "instruction_pairs": [
            {"target": _instruction_summary(left), "candidate": _instruction_summary(right)}
            for left, right in cluster[:16]
        ],
        "instruction_pairs_truncated": len(cluster) > 16,
        "limitations": [
            "Classification is a structural heuristic, not a source reconstruction.",
            "Semantic variable names, declaration scope, and provenance cannot be established from assembly.",
        ],
    }
    if target_symbol is not None:
        result["target_function_size"] = _parse_number(target_symbol.get("size"))
        result["target_match_percent"] = target_symbol.get("match_percent")
    if candidate_symbol is not None:
        result["candidate_function_size"] = _parse_number(candidate_symbol.get("size"))
        result["candidate_match_percent"] = candidate_symbol.get("match_percent")
    offsets = [
        item.address - (_parse_number(target_symbol.get("address")) or item.address)
        for item in target_items
        if item is not None and item.address is not None
    ]
    if target_asm is not None:
        result.setdefault("evidence", {})["target_assembly"] = target_asm.context_for(name, _parse_number(target_symbol.get("address")) if target_symbol else None)
        result["evidence"]["target_assembly_snippets"] = target_asm.snippets(name, _parse_number(target_symbol.get("address")) if target_symbol else None, offsets)
    if candidate_asm is not None:
        candidate_offsets = [
            item.address - (_parse_number(candidate_symbol.get("address")) or item.address)
            for item in candidate_items
            if item is not None and item.address is not None
        ]
        result.setdefault("evidence", {})["candidate_assembly"] = candidate_asm.context_for(name, _parse_number(candidate_symbol.get("address")) if candidate_symbol else None)
        result["evidence"]["candidate_assembly_snippets"] = candidate_asm.snippets(name, _parse_number(candidate_symbol.get("address")) if candidate_symbol else None, candidate_offsets)
    return result


def _explicit_else_return_patterns(
    name: str,
    target: Sequence[Instruction],
    candidate: Sequence[Instruction],
) -> list[dict[str, Any]]:
    """Recognize MWCC's explicit ``else { return; }`` epilogue topology.

    The supported signature is intentionally narrow: the target has two
    adjacent unconditional branches to the same following epilogue, an
    earlier conditional branch targets the second branch, and the candidate
    omits both unconditional branches while its corresponding conditional
    branch targets the epilogue directly.  This is the exact topology seen in
    ``ev_CapTeresaFadeMatHook`` c17.  It is source-axis evidence, not proof that
    an explicit else was present in original source.
    """

    patterns: list[dict[str, Any]] = []
    limit = min(len(target), len(candidate))
    for first_index in range(max(0, limit - 2)):
        first = target[first_index]
        second = target[first_index + 1]
        epilogue = target[first_index + 2]
        candidate_first = candidate[first_index]
        candidate_second = candidate[first_index + 1]
        candidate_epilogue = candidate[first_index + 2]
        if (
            first.mnemonic not in _UNCONDITIONAL_BRANCH_MNEMONICS
            or second.mnemonic not in _UNCONDITIONAL_BRANCH_MNEMONICS
            or first.branch_dest is None
            or second.branch_dest is None
            or epilogue.address is None
            or first.branch_dest != epilogue.address
            or second.branch_dest != epilogue.address
            or candidate_first.has_instruction
            or candidate_second.has_instruction
            or candidate_epilogue.address is None
        ):
            continue
        conditional_index = next(
            (
                index
                for index in range(first_index - 1, -1, -1)
                if target[index].mnemonic in _CONDITIONAL_BRANCH_MNEMONICS
                and target[index].branch_dest == second.address
                and candidate[index].mnemonic in _CONDITIONAL_BRANCH_MNEMONICS
                and candidate[index].branch_dest == candidate_epilogue.address
            ),
            None,
        )
        if conditional_index is None:
            continue
        conditional = target[conditional_index]
        candidate_conditional = candidate[conditional_index]
        patterns.append(
            {
                "function": name,
                "index_start": conditional_index,
                "index_end": first_index + 1,
                "target_address_start": conditional.address,
                "target_address_end": second.address,
                "candidate_address_start": candidate_conditional.address,
                "candidate_address_end": candidate_epilogue.address,
                "classification": "explicit_else_return_epilogue",
                "confidence": 0.98,
                "diff_pair_count": 3,
                "alternatives": [
                    {
                        "classification": "explicit_else_return_epilogue",
                        "confidence": 0.98,
                        "basis": "target conditional enters the second of two adjacent branches to one epilogue while candidate branches directly to that epilogue",
                    }
                ],
                "recommended_source_axis": "Test a positive guarded body with an explicit else-return: if (condition) { body } else { return; }.",
                "evidence": {
                    "conditional_index": conditional_index,
                    "conditional_target_address": conditional.address,
                    "conditional_target_destination": conditional.branch_dest,
                    "conditional_candidate_address": candidate_conditional.address,
                    "conditional_candidate_destination": candidate_conditional.branch_dest,
                    "target_exit_branch_indices": [first_index, first_index + 1],
                    "target_exit_branch_addresses": [first.address, second.address],
                    "shared_target_epilogue": epilogue.address,
                    "candidate_direct_epilogue": candidate_epilogue.address,
                    "diff_kinds": {"DIFF_DELETE": 2, "DIFF_ARG_MISMATCH": 1},
                },
                "limitations": [
                    "The topology ranks an explicit else-return as the smallest natural-C diagnostic; it does not prove original source provenance.",
                    "Retention still requires strict/data/relocation and sibling gates.",
                ],
            }
        )
    return patterns


def _causal_signature(item: Mapping[str, Any]) -> tuple[Any, ...]:
    classification = str(item.get("classification", "unknown"))
    evidence = item.get("evidence", {})
    if not isinstance(evidence, Mapping):
        evidence = {}
    if classification == "stack_home_uniform_delta":
        deltas = tuple(sorted({int(value) for value in evidence.get("stack_deltas", []) if isinstance(value, int) and value != 0}))
        return (classification, deltas)
    if classification == "missing_sign_extension_or_prototype":
        asymmetry = evidence.get("asymmetric_sign_extension", {})
        if not isinstance(asymmetry, Mapping):
            asymmetry = {}
        return (
            classification,
            tuple(asymmetry.get("target", [])),
            tuple(asymmetry.get("candidate", [])),
        )
    if classification == "explicit_else_return_epilogue":
        return (classification,)
    return (classification,)


def _causal_groups(
    name: str,
    clusters: Sequence[Mapping[str, Any]],
    patterns: Sequence[Mapping[str, Any]],
) -> list[dict[str, Any]]:
    """Collapse repeated residual clusters into bounded causal families."""

    grouped: dict[tuple[Any, ...], list[Mapping[str, Any]]] = {}
    for item in [*patterns, *clusters]:
        grouped.setdefault(_causal_signature(item), []).append(item)
    result: list[dict[str, Any]] = []
    for signature, members in grouped.items():
        ordered = sorted(
            members,
            key=lambda item: (
                int(item.get("index_start", -1) or -1),
                int(item.get("index_end", -1) or -1),
            ),
        )
        primary = max(
            ordered,
            key=lambda item: (float(item.get("confidence", 0.0)), -int(item.get("index_start", -1) or -1)),
        )
        ranges = [
            {"index_start": item.get("index_start"), "index_end": item.get("index_end")}
            for item in ordered
        ]
        group = {
            "function": name,
            "classification": primary.get("classification"),
            "confidence": primary.get("confidence"),
            "signature": list(signature),
            "root_index": primary.get("index_start"),
            "cluster_count": len(ordered),
            "diff_pair_count": sum(int(item.get("diff_pair_count", 0) or 0) for item in ordered),
            "affected_ranges": ranges,
            "probable_cascade_pair_count": sum(int(item.get("diff_pair_count", 0) or 0) for item in ordered if item is not primary),
            "limitations": [
                "A causal group collapses repeated structural signatures; it does not prove that one source edit causes every member.",
            ],
        }
        if primary.get("recommended_source_axis"):
            group["recommended_source_axis"] = primary["recommended_source_axis"]
        result.append(group)
    result.sort(
        key=lambda item: (
            -float(item.get("confidence", 0.0)),
            int(item.get("root_index", -1) or -1),
            str(item.get("classification", "")),
        )
    )
    return result


def _section_hypotheses(document: Mapping[str, Any]) -> list[dict[str, Any]]:
    left = document.get("left")
    right = document.get("right")
    if not isinstance(left, Mapping) or not isinstance(right, Mapping):
        return []
    left_sections = left.get("sections", [])
    right_sections = right.get("sections", [])
    if not isinstance(left_sections, list) or not isinstance(right_sections, list):
        return []
    left_map = {item.get("name"): item for item in left_sections if isinstance(item, Mapping) and isinstance(item.get("name"), str)}
    right_map = {item.get("name"): item for item in right_sections if isinstance(item, Mapping) and isinstance(item.get("name"), str)}
    result: list[dict[str, Any]] = []
    for name in sorted(set(left_map) | set(right_map)):
        target = left_map.get(name, {})
        candidate = right_map.get(name, {})
        target_data = target.get("data_diff", [])
        candidate_data = candidate.get("data_diff", [])
        target_reloc = target.get("reloc_diff", [])
        candidate_reloc = candidate.get("reloc_diff", [])
        if not isinstance(target_data, list) or not isinstance(candidate_data, list):
            target_data = []
            candidate_data = []
        if not isinstance(target_reloc, list) or not isinstance(candidate_reloc, list):
            target_reloc = []
            candidate_reloc = []
        if not target_data and not candidate_data and not target_reloc and not candidate_reloc:
            continue
        result.append(
            {
                "function": "<sections>",
                "section": name,
                "classification": "relocation_or_data_mismatch",
                "confidence": 0.95,
                "evidence": {
                    "target_data_diff_count": len(target_data),
                    "candidate_data_diff_count": len(candidate_data),
                    "target_reloc_diff_count": len(target_reloc),
                    "candidate_reloc_diff_count": len(candidate_reloc),
                    "target_match_percent": target.get("match_percent"),
                    "candidate_match_percent": candidate.get("match_percent"),
                },
                "limitations": [
                    "Section evidence identifies bytes/relocations only; it does not identify a source declaration or provenance."
                ],
            }
        )
    return result


def _parse_assembly(path: str | Path) -> AssemblyIndex:
    path_obj = Path(path)
    try:
        text = path_obj.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        raise AuditInputError("assembly_read_error", f"cannot read assembly {path_obj}: {exc}") from exc
    instructions: list[AsmInstruction] = []
    functions: dict[str, list[AsmInstruction]] = defaultdict(list)
    current: str | None = None
    for line_number, line in enumerate(text.splitlines(), 1):
        function_match = _ASM_FN_RE.match(line)
        if function_match:
            current = function_match.group("name")
            functions.setdefault(current, [])
            continue
        if _ASM_ENDFN_RE.match(line):
            current = None
            continue
        match = _ASM_COMMENT_RE.match(line) or _ASM_COLON_RE.match(line)
        if not match:
            continue
        address = _parse_asm_address(match.group("address"))
        text_value = match.group("text").strip()
        if address is None or not text_value or text_value.startswith((".", "#")):
            continue
        item = AsmInstruction(address, text_value, _mnemonic(text_value), current, line_number)
        instructions.append(item)
        if current is not None:
            functions[current].append(item)
    if not instructions:
        raise AuditInputError("assembly_parse_error", f"assembly {path_obj} contains no parseable instruction lines")
    return AssemblyIndex(str(path_obj), instructions, dict(functions))


def _read_document(path: str) -> Mapping[str, Any]:
    try:
        if path == "-":
            text = sys.stdin.read()
        else:
            text = Path(path).read_text(encoding="utf-8")
    except OSError as exc:
        raise AuditInputError("input_read_error", f"cannot read JSON input {path}: {exc}") from exc
    try:
        value = json.loads(text)
    except json.JSONDecodeError as exc:
        raise AuditInputError("invalid_json", f"invalid JSON at line {exc.lineno}, column {exc.colno}") from exc
    if not isinstance(value, Mapping):
        raise AuditInputError("invalid_root", "JSON root must be an object")
    return value


def _fail_closed(error: AuditInputError) -> dict[str, Any]:
    return {
        "schema_version": SCHEMA_VERSION,
        "status": "fail_closed",
        "fail_closed": True,
        "error": {"code": error.code, "message": error.message},
        "functions": [],
        "hypotheses": [],
        "causal_groups": [],
        "section_hypotheses": [],
        "unmatched_functions": [],
    }


def _summary_result(
    function_results: Sequence[Mapping[str, Any]],
    hypotheses: Sequence[Mapping[str, Any]],
    causal_groups: Sequence[Mapping[str, Any]],
    section_hypotheses: Sequence[Mapping[str, Any]],
    unmatched: Sequence[Mapping[str, Any]],
    limitations: Sequence[str],
    *,
    max_hypotheses: int,
) -> dict[str, Any]:
    """Return a bounded report that retains metrics and ranked evidence.

    ``hypotheses`` has already been assigned its deterministic global ranks.
    Cluster identity is therefore retained while selecting the top-ranked
    entries, allowing each function to report only the same bounded set that
    appears in the flat summary.  Section evidence remains complete because
    it is already count-based and bounded by the source report's section set.
    """

    selected = list(hypotheses[:max_hypotheses])
    selected_cluster_ranks = {
        item.get("rank")
        for item in selected
        if item.get("function") != "<sections>"
    }
    compact_functions: list[dict[str, Any]] = []
    for function in function_results:
        all_clusters = [
            cluster
            for cluster in function.get("clusters", [])
            if isinstance(cluster, Mapping)
        ]
        compact_clusters = [
            _compact_hypothesis(cluster)
            for cluster in all_clusters
            if cluster.get("rank") in selected_cluster_ranks
        ]
        compact_function = dict(function)
        compact_function["clusters"] = compact_clusters
        compact_function["patterns"] = [
            _compact_hypothesis(pattern)
            for pattern in function.get("patterns", [])
            if isinstance(pattern, Mapping) and pattern.get("rank") in selected_cluster_ranks
        ]
        compact_function["classification_counts"] = _classification_counts(all_clusters)
        compact_function["retained_cluster_count"] = len(compact_clusters)
        compact_functions.append(compact_function)

    compact_hypotheses = [_compact_hypothesis(item) for item in selected]
    return {
        "schema_version": SCHEMA_VERSION,
        "status": "ok",
        "fail_closed": False,
        "functions": compact_functions,
        "hypotheses": compact_hypotheses,
        "causal_groups": [dict(item) for item in causal_groups],
        "section_hypotheses": [dict(item) for item in section_hypotheses],
        "unmatched_functions": [dict(item) for item in unmatched],
        "limitations": list(limitations),
        "summary_only": True,
        "max_hypotheses": max_hypotheses,
        "hypothesis_count": len(hypotheses),
        "returned_hypothesis_count": len(compact_hypotheses),
        "classification_counts": _classification_counts(hypotheses),
    }


def audit_document(
    document: Mapping[str, Any],
    *,
    target_assembly: str | Path | None = None,
    candidate_assembly: str | Path | None = None,
    focus_symbol: str | None = None,
    include_exact_residuals: bool = False,
    summary_only: bool = False,
    max_hypotheses: int = DEFAULT_MAX_HYPOTHESES,
) -> dict[str, Any]:
    """Audit a parsed objdiff document and return deterministic JSON data."""

    max_hypotheses = _validate_max_hypotheses(max_hypotheses)
    if not isinstance(summary_only, bool):
        raise AuditInputError("invalid_summary_only", "summary_only must be a boolean")
    target_asm = _parse_assembly(target_assembly) if target_assembly is not None else None
    candidate_asm = _parse_assembly(candidate_assembly) if candidate_assembly is not None else None
    pairs = _focus_pairs(_paired_functions(document), focus_symbol)
    function_results: list[dict[str, Any]] = []
    hypotheses: list[dict[str, Any]] = []
    causal_groups: list[dict[str, Any]] = []
    unmatched: list[dict[str, Any]] = []
    for pair in pairs:
        # Validate every side before deciding that a function is unmatched.
        # Otherwise a target-only/candidate-only symbol could carry malformed
        # instruction entries and be silently discarded from the audit.
        target_entries = _entries(pair.target, "target", pair.name)
        candidate_entries = _entries(pair.candidate, "candidate", pair.name)
        if pair.target is None or pair.candidate is None:
            unmatched.append(
                {
                    "function": pair.name,
                    "side": "target" if pair.target is None else "candidate",
                    "classification": "unknown",
                    "reason": "function is present on only one side; no structural source-shape inference is attempted",
                }
            )
            continue
        exact_pair = _is_exact_pair(pair)
        clusters = (
            _cluster_records(target_entries, candidate_entries)
            if include_exact_residuals or not exact_pair
            else []
        )
        function_result = {
            "function": pair.name,
            "target_address": _parse_number(pair.target.get("address")),
            "candidate_address": _parse_number(pair.candidate.get("address")),
            "target_size": _parse_number(pair.target.get("size")),
            "candidate_size": _parse_number(pair.candidate.get("size")),
            "target_match_percent": pair.target.get("match_percent"),
            "candidate_match_percent": pair.candidate.get("match_percent"),
            "residual_cluster_count": len(clusters),
            "clusters": [],
            "patterns": [],
            "causal_groups": [],
        }
        for cluster in clusters:
            result = _cluster_result(pair.name, cluster, pair.target, pair.candidate, target_asm, candidate_asm)
            function_result["clusters"].append(result)
            hypotheses.append(result)
        patterns = _explicit_else_return_patterns(pair.name, target_entries, candidate_entries)
        function_result["patterns"].extend(patterns)
        hypotheses.extend(patterns)
        groups = _causal_groups(pair.name, function_result["clusters"], patterns)
        function_result["causal_groups"].extend(groups)
        causal_groups.extend(groups)
        function_results.append(function_result)
    section_hypotheses = _section_hypotheses(document)
    hypotheses.extend(section_hypotheses)
    # Assign a global rank after all structural evidence has been collected.
    hypotheses.sort(
        key=lambda item: (
            -float(item.get("confidence", 0.0)),
            str(item.get("function", "")),
            _parse_number(item.get("target_address_start")) or -1,
            _parse_number(item.get("candidate_address_start")) or -1,
            int(item.get("index_start", -1) or -1),
            str(item.get("section", "")),
        )
    )
    for rank, item in enumerate(hypotheses, 1):
        item["rank"] = rank
    # Function clusters preserve address order, while the flat hypotheses are
    # confidence-ranked for queue consumers.
    for item in function_results:
        item["clusters"].sort(
            key=lambda cluster: (
                _parse_number(cluster.get("target_address_start")) or -1,
                _parse_number(cluster.get("candidate_address_start")) or -1,
                int(cluster.get("index_start", -1) or -1),
            )
        )
    function_results.sort(key=lambda item: (item["function"], item.get("target_address") or -1, item.get("candidate_address") or -1))
    result = {
        "schema_version": SCHEMA_VERSION,
        "status": "ok",
        "fail_closed": False,
        "functions": function_results,
        "hypotheses": hypotheses,
        "causal_groups": causal_groups,
        "section_hypotheses": section_hypotheses,
        "unmatched_functions": unmatched,
        "limitations": [
            "Confidence and ranking are deterministic structural heuristics, not proof of source shape.",
            "Semantic variable names and provenance are intentionally not inferred from assembly.",
        ],
    }
    if summary_only:
        return _summary_result(
            result["functions"],
            result["hypotheses"],
            result["causal_groups"],
            result["section_hypotheses"],
            result["unmatched_functions"],
            result["limitations"],
            max_hypotheses=max_hypotheses,
        )
    return result


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Cluster objdiff residuals and rank conservative source-shape hypotheses."
    )
    parser.add_argument("report", help="objdiff JSON report path, or - for stdin")
    parser.add_argument("--target-asm", help="optional target assembly listing")
    parser.add_argument("--candidate-asm", help="optional candidate assembly listing")
    parser.add_argument("--focus-symbol", help="audit exactly one paired function symbol")
    parser.add_argument(
        "--include-exact-residuals",
        action="store_true",
        help="include residual clusters from pairs whose two sides report 100%%",
    )
    parser.add_argument(
        "--summary-only",
        action="store_true",
        help="emit bounded ranked evidence without instruction-level pairs",
    )
    parser.add_argument(
        "--max-hypotheses",
        default=str(DEFAULT_MAX_HYPOTHESES),
        help=f"summary hypothesis limit (1-{MAX_HYPOTHESES}, default: {DEFAULT_MAX_HYPOTHESES})",
    )
    parser.add_argument("-o", "--output", help="write JSON to this path instead of stdout")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)
    try:
        document = _read_document(args.report)
        try:
            max_hypotheses = int(args.max_hypotheses, 10)
        except (TypeError, ValueError) as exc:
            raise AuditInputError(
                "invalid_max_hypotheses",
                f"max_hypotheses must be an integer from 1 through {MAX_HYPOTHESES}",
            ) from exc
        result = audit_document(
            document,
            target_assembly=args.target_asm,
            candidate_assembly=args.candidate_asm,
            focus_symbol=args.focus_symbol,
            include_exact_residuals=args.include_exact_residuals,
            summary_only=args.summary_only,
            max_hypotheses=max_hypotheses,
        )
    except AuditInputError as error:
        result = _fail_closed(error)
    output = json.dumps(result, indent=2, sort_keys=True) + "\n"
    try:
        if args.output:
            Path(args.output).write_text(output, encoding="utf-8")
        else:
            sys.stdout.write(output)
    except OSError as exc:
        print(f"mismatch_cluster_audit: output error: {exc}", file=sys.stderr)
        return 2
    return 2 if result.get("fail_closed") else 0


if __name__ == "__main__":
    raise SystemExit(main())

