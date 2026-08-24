#!/usr/bin/env python3
"""Deterministic recovery-pass triage from objdiff and source evidence.

This prototype is deliberately read-only with respect to recovered source and
knowledge metadata.  It may generate objdiff reports and writes only beneath
the requested output directory.  It never invents C, edits cards, or launches
parallel builds.
"""

from __future__ import annotations

import argparse
import base64
import contextlib
import difflib
import hashlib
import json
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import time
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence


SCRIPT = Path(__file__).resolve()
DEFAULT_ROOT = SCRIPT.parents[1]
if str(DEFAULT_ROOT) not in sys.path:
    sys.path.insert(0, str(DEFAULT_ROOT))

from tools.knowledge_freshness import card_freshness
from tools.context_engine import collect_rejected_probe_history
from tools.recovery_core import load
from tools.recovery_data import _mask_c, function_text, parse_functions


CALL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
REGISTER_RE = re.compile(r"\b([rf])(\d+)\b")
STACK_RE = re.compile(r"(-?\d+)\(r1\)")
NUMERIC_DEFINE_RE = re.compile(
    r"^\s*#\s*define\s+([A-Z][A-Z0-9_]*)\s+"
    r"\(?\s*([-+]?(?:0[xX][0-9A-Fa-f]+|\d+(?:\.\d*)?|\.\d+)"
    r"(?:[eE][-+]?\d+)?)[fFuUlL]*\s*\)?(?:\s|/|$)",
    re.MULTILINE,
)
COMMUTATIVE = {"add", "add.", "and", "and.", "or", "or.", "xor", "xor.",
               "mullw", "mullw.", "fadd", "fadds", "fmul", "fmuls"}
BRANCH_ONLY = {"b"}
CALL_KEYWORDS = {"if", "for", "while", "switch", "sizeof", "return"}
BUILD_LOCK_ENV = "MP6_BUILD_LOCK"
# Relative to the selected repository root.  Different isolated worktrees must
# not serialize each other; this lock only protects concurrent mutation of one
# worktree's build tree.
DEFAULT_BUILD_LOCK = Path("build") / ".compiler-lane.lock"
TARGET_CALL_CLUSTER_CARD = "target-call-skeleton-before-shared-accessor-abstraction"
PAIRED_SINGLE_QUARANTINE_CARD = "gc26-paired-single-huvecf-copy-spelling-negative"
POOL_OWNERSHIP_CARD = "mwcc-conversion-pool-relocation-ownership-before-text-retention"
GENERIC_CLUSTER_CALLS = {
    "abs", "fabs", "fabsf", "fmod", "memcpy", "memset",
    "HuPrcCurrentGet", "HuPrcSleep", "HuPrcVSleep",
}
MIN_ORDERED_CLUSTER_CALLS = 6
MIN_ORDERED_CLUSTER_RATIO = 0.75
MIN_HOMOLOGOUS_MISSING_FAMILY_BYTES = 512
PREFERRED_PACKET_MIN_FUNCTIONS = 12
PREFERRED_PACKET_MAX_FUNCTIONS = 20
PREFERRED_PACKET_MIN_BYTES = 3072
PREFERRED_PACKET_MAX_BYTES = 6144
PROBE_HISTORY = Path("build/board-autonomy/batch-history.json")
WORKER_PACKET_SCHEMA_VERSION = 1
WORKER_PROBE_BUDGETS = {
    "structural_or_type": 10,
    "declaration_or_include": 6,
    "compiler_reconciliation": 3,
}


@dataclass(frozen=True)
class ElfSection:
    name: str
    index: int
    offset: int
    size: int
    link: int
    entsize: int
    kind: int


def _lock_file_nonblocking(handle: Any) -> bool:
    handle.seek(0)
    if os.name == "nt":
        import msvcrt

        try:
            msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
        except OSError:
            return False
    else:
        import fcntl

        try:
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            return False
    return True


def _unlock_file(handle: Any) -> None:
    handle.seek(0)
    if os.name == "nt":
        import msvcrt

        msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
    else:
        import fcntl

        fcntl.flock(handle.fileno(), fcntl.LOCK_UN)


@contextlib.contextmanager
def serialized_build_lock(path: Path, timeout_seconds: float) -> Iterable[None]:
    """Serialize one mutable build transaction that shares a build tree."""
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a+b") as handle:
        if path.stat().st_size == 0:
            handle.write(b"\0")
            handle.flush()
        deadline = time.monotonic() + timeout_seconds
        while not _lock_file_nonblocking(handle):
            if time.monotonic() >= deadline:
                raise ValueError(
                    f"worktree build lock remained busy for {timeout_seconds:g}s: {path}"
                )
            time.sleep(0.1)
        try:
            yield
        finally:
            _unlock_file(handle)


def atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, prefix=path.name, suffix=".tmp",
        delete=False,
    ) as handle:
        json.dump(value, handle, indent=2, sort_keys=False)
        handle.write("\n")
        temporary = Path(handle.name)
    temporary.replace(path)


def atomic_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, prefix=path.name, suffix=".tmp",
        delete=False,
    ) as handle:
        handle.write(value)
        temporary = Path(handle.name)
    temporary.replace(path)


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ValueError(f"missing JSON report: {path}") from exc
    except json.JSONDecodeError as exc:
        raise ValueError(f"invalid JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}") from exc


def file_hash(path: Path | None) -> str | None:
    if path is None or not path.is_file():
        return None
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def tree_hash(root: Path, pattern: str) -> dict[str, Any]:
    digest = hashlib.sha256()
    count = 0
    for path in sorted(root.rglob(pattern)) if root.is_dir() else []:
        if not path.is_file():
            continue
        digest.update(path.relative_to(root).as_posix().encode("utf-8"))
        digest.update(bytes.fromhex(file_hash(path) or ""))
        count += 1
    return {"sha256": digest.hexdigest(), "files": count}


def recovery_cache_key(
    root: Path,
    unit: Mapping[str, Any],
    source: Path,
    strict_report: Path | None,
    value_report: Path | None,
    baseline_strict: Path | None,
    baseline_value: Path | None,
    graph: Path | None,
    objdiff_executable: Path,
    donor_roots: Sequence[Path],
) -> dict[str, Any]:
    target = root / str(unit["target_path"])
    base = root / str(unit["base_path"])
    manifest = graph.parent / "manifest.json" if graph else None
    inputs = {
        "unit": unit["name"],
        "source": file_hash(source),
        "objects": {"target": file_hash(target), "base": file_hash(base)},
        "reports": {
            "strict": file_hash(strict_report) if strict_report else "generated_from_objects",
            "data_value": file_hash(value_report) if value_report else "generated_from_objects",
            "baseline_strict": file_hash(baseline_strict),
            "baseline_value": file_hash(baseline_value),
        },
        "objdiff": file_hash(objdiff_executable),
        "graphify": {"graph": file_hash(graph), "manifest": file_hash(manifest)},
        "headers": tree_hash(root / "include", "*.h"),
        "knowledge": tree_hash(root / "config" / "recovery", "*.json"),
        "implementation": file_hash(SCRIPT),
        "commit": git_value(root, "rev-parse", "HEAD"),
        "donors": donor_cache_key(donor_roots),
    }
    encoded = json.dumps(inputs, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return {"schema_version": 1, "key": hashlib.sha256(encoded).hexdigest(), "inputs": inputs}


def number(value: Any, default: int = 0) -> int:
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def functions(side: Mapping[str, Any]) -> list[dict[str, Any]]:
    return [dict(item) for item in side.get("symbols", []) if item.get("kind") == "SYMBOL_FUNCTION"]


def changed_rows(symbol: Mapping[str, Any]) -> list[Mapping[str, Any]]:
    return [row for row in symbol.get("instructions", []) if row.get("diff_kind")]


def is_exact(symbol: Mapping[str, Any]) -> bool:
    return symbol.get("target_symbol") is not None and symbol.get("match_percent") == 100.0 and not changed_rows(symbol)


def instruction(row: Mapping[str, Any]) -> str:
    value = row.get("instruction", {}).get("formatted")
    return value if isinstance(value, str) else ""


def split_instruction(value: str) -> tuple[str, list[str]]:
    if not value:
        return "", []
    pieces = value.strip().split(None, 1)
    operands = [part.strip() for part in pieces[1].split(",")] if len(pieces) == 2 else []
    return pieces[0], operands


def paired_symbol(report: Mapping[str, Any], left_symbol: Mapping[str, Any]) -> Mapping[str, Any] | None:
    index = left_symbol.get("target_symbol")
    right = report.get("right", {}).get("symbols", [])
    return right[index] if isinstance(index, int) and 0 <= index < len(right) else None


def paired_changed(report: Mapping[str, Any], symbol: Mapping[str, Any]) -> list[tuple[str, str, str]]:
    other = paired_symbol(report, symbol)
    if other is None:
        return []
    left = changed_rows(symbol)
    right = changed_rows(other)
    if len(left) != len(right):
        return []
    return [(str(a.get("diff_kind")), instruction(a), instruction(b)) for a, b in zip(left, right)]


def commutative_swap_kind(pairs: Sequence[tuple[str, str, str]]) -> str | None:
    if not pairs:
        return None
    opcodes: list[str] = []
    for kind, target, source in pairs:
        top, ta = split_instruction(target)
        sop, sa = split_instruction(source)
        if kind != "DIFF_ARG_MISMATCH" or top != sop or top not in COMMUTATIVE:
            return None
        if len(ta) != 3 or len(sa) != 3 or ta[0] != sa[0] or ta[1:] != list(reversed(sa[1:])):
            return None
        opcodes.append(top)
    return "floating_commutative_swap" if all(op.startswith("f") for op in opcodes) else "integer_commutative_swap"


def branch_destination_only(pairs: Sequence[tuple[str, str, str]]) -> bool:
    if not pairs:
        return False
    for kind, target, source in pairs:
        top, ta = split_instruction(target)
        sop, sa = split_instruction(source)
        if kind != "DIFF_ARG_MISMATCH" or top != sop or top not in BRANCH_ONLY or len(ta) != 1 or len(sa) != 1:
            return False
    return True


def saved_register_cycle(pairs: Sequence[tuple[str, str, str]]) -> dict[str, str] | None:
    if not pairs:
        return None
    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    changed = False
    for kind, target, source in pairs:
        if kind != "DIFF_ARG_MISMATCH":
            return None
        top, _ = split_instruction(target)
        sop, _ = split_instruction(source)
        if top != sop:
            return None
        target_regs = [match.group(0) for match in REGISTER_RE.finditer(target)]
        source_regs = [match.group(0) for match in REGISTER_RE.finditer(source)]
        target_shape = REGISTER_RE.sub("REG", target)
        source_shape = REGISTER_RE.sub("REG", source)
        if target_shape != source_shape or len(target_regs) != len(source_regs):
            return None
        for source_reg, target_reg in zip(source_regs, target_regs):
            if source_reg == target_reg:
                continue
            prefix = source_reg[0]
            if prefix != target_reg[0] or number(source_reg[1:]) < 14 or number(target_reg[1:]) < 14:
                return None
            if source_reg in mapping and mapping[source_reg] != target_reg:
                return None
            if target_reg in reverse and reverse[target_reg] != source_reg:
                return None
            mapping[source_reg] = target_reg
            reverse[target_reg] = source_reg
            changed = True
    return mapping if changed and set(mapping) == set(mapping.values()) else None


def stack_slot_permutation(pairs: Sequence[tuple[str, str, str]]) -> dict[str, str] | None:
    if not pairs:
        return None
    mapping: dict[str, str] = {}
    for kind, target, source in pairs:
        if kind != "DIFF_ARG_MISMATCH":
            return None
        top, _ = split_instruction(target)
        sop, _ = split_instruction(source)
        if top != sop:
            return None
        target_slots = STACK_RE.findall(target)
        source_slots = STACK_RE.findall(source)
        if not target_slots or len(target_slots) != len(source_slots):
            return None
        if STACK_RE.sub("SLOT(r1)", target) != STACK_RE.sub("SLOT(r1)", source):
            return None
        for source_slot, target_slot in zip(source_slots, target_slots):
            if source_slot != target_slot:
                mapping[source_slot] = target_slot
    return mapping or None


def relocation_name(side: Mapping[str, Any], relocation: Mapping[str, Any]) -> str:
    index = relocation.get("target_symbol")
    symbols = side.get("symbols", [])
    if isinstance(index, int) and 0 <= index < len(symbols):
        return str(symbols[index].get("name", f"symbol[{index}]"))
    return f"symbol[{index}]"


def relocation_owner_evidence(
    report: Mapping[str, Any], symbol: Mapping[str, Any]
) -> list[dict[str, str]]:
    other = paired_symbol(report, symbol)
    if other is None:
        return []
    evidence: set[tuple[str, str, str]] = set()
    for target_row, source_row in zip(changed_rows(symbol), changed_rows(other)):
        target_relocation = target_row.get("instruction", {}).get("relocation")
        source_relocation = source_row.get("instruction", {}).get("relocation")
        if not isinstance(target_relocation, Mapping) or not isinstance(source_relocation, Mapping):
            continue
        target_owner = relocation_name(report.get("left", {}), target_relocation)
        source_owner = relocation_name(report.get("right", {}), source_relocation)
        relocation_type = str(target_relocation.get("type_name", "unknown"))
        if target_owner != source_owner:
            evidence.add((target_owner, source_owner, relocation_type))
    return [
        {"target_owner": target, "source_owner": source, "type": kind}
        for target, source, kind in sorted(evidence)
    ]


def call_skeleton(side: Mapping[str, Any], symbol: Mapping[str, Any]) -> list[str]:
    result: list[str] = []
    for row in symbol.get("instructions", []):
        relocation = row.get("instruction", {}).get("relocation")
        if isinstance(relocation, Mapping) and relocation.get("type_name") == "R_PPC_REL24":
            result.append(relocation_name(side, relocation))
    return result


FORENSICS_BYTE_LIMIT = 96
FORENSICS_PROLOGUE_LIMIT = 8
FORENSICS_RELOCATION_LIMIT = 96
FORENSICS_DIRECT_CALL_LIMIT = 64
FORENSICS_CALLBACK_LIMIT = 32
FORENSICS_DATA_SYMBOL_LIMIT = 32
FORENSICS_R1_OFFSET_LIMIT = 64
FORENSICS_REGISTER_LIMIT = 64
FORENSICS_DIAGNOSTIC_LIMIT = 16
FORENSICS_TEXT_LIMIT = 240
STACK_CLUE_RE = re.compile(r"(-?(?:0[xX][0-9A-Fa-f]+|\d+))\(r1\)")
BASE64_ALPHABET = frozenset("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=")


def _objdiff_int(value: Any, default: int | None = None) -> int | None:
    """Parse the decimal/hex strings emitted by objdiff without raising."""
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, int):
        return value
    if isinstance(value, float) and value.is_integer():
        return int(value)
    if isinstance(value, str):
        text = value.strip()
        if not text:
            return default
        try:
            return int(text, 0)
        except ValueError:
            try:
                return int(text, 10)
            except ValueError:
                return default
    return default


def _forensics_text(value: Any, limit: int = FORENSICS_TEXT_LIMIT) -> str:
    """Render bounded free-form objdiff text without inflating worker packets."""
    text = value if isinstance(value, str) else str(value) if value is not None else ""
    if len(text) <= limit:
        return text
    return text[: max(0, limit - 14)] + "...[clipped]"


def _symbol_target(side: Mapping[str, Any], relocation: Mapping[str, Any]) -> tuple[str, Mapping[str, Any] | None, int | None]:
    if not isinstance(side, Mapping):
        side = {}
    index = relocation.get("target_symbol")
    symbols = side.get("symbols", [])
    if not isinstance(symbols, Sequence) or isinstance(symbols, (str, bytes, bytearray)):
        symbols = []
    target: Mapping[str, Any] | None = None
    if isinstance(index, int) and 0 <= index < len(symbols) and isinstance(symbols[index], Mapping):
        target = symbols[index]
    name = str(target.get("name")) if target and target.get("name") else (
        f"symbol[{index}]" if index is not None else "<unknown>"
    )
    return name, target, index if isinstance(index, int) else None


def _target_data_bytes(symbol: Mapping[str, Any]) -> tuple[str | None, dict[str, int]]:
    """Return bounded target data bytes and explicit emission/truncation counts."""
    if not isinstance(symbol, Mapping):
        return None, {"emitted": 0, "total": 0, "truncated": 0}
    chunks: list[bytes] = []
    entries = symbol.get("data_diff", [])
    if not isinstance(entries, Sequence) or isinstance(entries, (str, bytes, bytearray)):
        return None, {"emitted": 0, "total": 0, "truncated": 0}
    valid_entries = [entry for entry in entries if isinstance(entry, Mapping)]
    addressed = [
        _objdiff_int(entry.get("offset", entry.get("address")))
        for entry in valid_entries
    ]
    if any(value is not None for value in addressed):
        valid_entries = sorted(
            valid_entries,
            key=lambda entry: (
                _objdiff_int(entry.get("offset", entry.get("address"))) is None,
                _objdiff_int(entry.get("offset", entry.get("address")), 0) or 0,
                str(entry.get("kind", "")),
                str(entry.get("data", "")),
                _objdiff_int(entry.get("size"), 0) or 0,
            ),
        )
    declared_size = _objdiff_int(symbol.get("size"))
    byte_limit = FORENSICS_BYTE_LIMIT
    if declared_size is not None and declared_size >= 0:
        byte_limit = min(byte_limit, declared_size)
    remaining = byte_limit
    total = 0
    for entry in valid_entries:
        if not isinstance(entry.get("data"), str):
            continue
        encoded = entry["data"]
        encoded_count = 0
        trailing_padding = 0
        for character in encoded:
            if character not in BASE64_ALPHABET:
                continue
            encoded_count += 1
            if character == "=":
                trailing_padding += 1
            else:
                trailing_padding = 0
        if encoded_count:
            total += max(0, ((encoded_count + 3) // 4) * 3 - min(2, trailing_padding))
        if remaining <= 0:
            continue
        desired_chars = max(4, ((remaining + 2) // 3) * 4)
        prefix_chars: list[str] = []
        for character in encoded:
            if character not in BASE64_ALPHABET:
                continue
            prefix_chars.append(character)
            if len(prefix_chars) >= desired_chars:
                break
        if not prefix_chars:
            continue
        prefix = "".join(prefix_chars)
        if len(prefix) % 4:
            prefix += "=" * (4 - len(prefix) % 4)
        try:
            raw = base64.b64decode(prefix, validate=False)
        except (ValueError, TypeError):
            continue
        if raw:
            chunk = raw[:remaining]
            chunks.append(chunk)
            remaining -= len(chunk)
    emitted_bytes = b"".join(chunks)[:byte_limit]
    counts = {
        "emitted": len(emitted_bytes),
        "total": total,
        "truncated": max(0, total - len(emitted_bytes)),
    }
    if not emitted_bytes:
        return None, counts
    return base64.b64encode(emitted_bytes).decode("ascii"), counts


def function_forensics(
    side: Mapping[str, Any],
    symbol: Mapping[str, Any],
    diagnostics: Sequence[str] = (),
) -> dict[str, Any]:
    """Extract compact target-backed relocation and stack clues for one function.

    Objdiff has historically emitted partially populated rows (notably for
    synthetic/missing symbols), so this deliberately treats every field as
    optional and never lets malformed evidence abort packet generation.
    """
    if not isinstance(side, Mapping):
        side = {}
    if not isinstance(symbol, Mapping):
        symbol = {}
    size = _objdiff_int(symbol.get("size"), 0) or 0
    instructions = symbol.get("instructions", [])
    if not isinstance(instructions, Sequence) or isinstance(instructions, (str, bytes, bytearray)):
        instructions = []
    def instruction_sort_key(row: Any, row_index: int) -> tuple[Any, ...]:
        raw = row.get("instruction") if isinstance(row, Mapping) else {}
        inst = raw if isinstance(raw, Mapping) else {}
        address = _objdiff_int(inst.get("address"))
        relocation = inst.get("relocation")
        relocation = relocation if isinstance(relocation, Mapping) else {}
        target_name, _, target_index = _symbol_target(side, relocation)
        type_name = _forensics_text(relocation.get("type_name") or relocation.get("type") or "unknown")
        target_name = _forensics_text(target_name)
        addend = _objdiff_int(relocation.get("addend"), 0) or 0
        return (
            address is None,
            address if address is not None else 0,
            type_name,
            target_name,
            addend,
            target_index if target_index is not None else -1,
            row_index if address is None else -1,
            str(inst.get("formatted", "")),
            _objdiff_int(inst.get("size"), 4) or 4,
        )
    ordered_instructions = [
        row
        for _, row in sorted(
            enumerate(instructions),
            key=lambda pair: instruction_sort_key(pair[1], pair[0]),
        )
    ]
    symbol_start = _objdiff_int(symbol.get("address"))
    first_address: int | None = None
    cursor = 0
    relocations: list[dict[str, Any]] = []
    direct_calls: list[str] = []
    direct_call_edges: list[dict[str, Any]] = []
    callback_edges: list[dict[str, Any]] = []
    data_symbols: dict[int | str, dict[str, Any]] = {}
    prologue: list[str] = []
    r1_offsets: set[int] = set()
    registers: set[str] = set()
    frame_size: int | None = None
    for row in ordered_instructions:
        if not isinstance(row, Mapping):
            continue
        raw_instruction = row.get("instruction")
        inst = raw_instruction if isinstance(raw_instruction, Mapping) else {}
        formatted = inst.get("formatted")
        formatted_text = _forensics_text(formatted)
        if len(prologue) < FORENSICS_PROLOGUE_LIMIT and formatted_text:
            prologue.append(formatted_text)
        address = _objdiff_int(inst.get("address"), cursor)
        if first_address is None and address is not None:
            first_address = address
        instruction_size = _objdiff_int(inst.get("size"), 4) or 4
        cursor = (address if address is not None else cursor) + instruction_size
        for match in STACK_CLUE_RE.finditer(formatted_text):
            offset = _objdiff_int(match.group(1))
            if offset is not None:
                r1_offsets.add(offset)
        for match in REGISTER_RE.finditer(formatted_text):
            registers.add(match.group(0))
        if frame_size is None:
            opcode, operands = split_instruction(formatted_text)
            if opcode in {"stwu", "stdu"} and len(operands) >= 2:
                immediate = _objdiff_int(operands[1].split("(", 1)[0])
                if immediate is not None:
                    frame_size = abs(immediate)
        relocation = inst.get("relocation")
        if not isinstance(relocation, Mapping):
            continue
        target_name, target_symbol, target_index = _symbol_target(side, relocation)
        type_name = _forensics_text(relocation.get("type_name") or relocation.get("type") or "unknown")
        addend = _objdiff_int(relocation.get("addend"), 0) or 0
        target_kind = str(target_symbol.get("kind", "")) if target_symbol else ""
        target_name = _forensics_text(target_name)
        if type_name == "R_PPC_REL24":
            kind = "call"
            direct_calls.append(target_name)
            edge = {"offset": address, "target": target_name}
            direct_call_edges.append(edge)
        elif target_kind == "SYMBOL_FUNCTION":
            kind = "callback"
            callback_edges.append(
                {"offset": address, "target": target_name, "type": type_name}
            )
        elif target_kind == "SYMBOL_OBJECT":
            kind = "data"
        else:
            kind = "symbol"
        relocations.append(
            {
                "offset": address,
                "type": type_name,
                "addend": addend,
                "target": target_name,
                "kind": kind,
            }
        )
        if target_kind == "SYMBOL_OBJECT":
            key: int | str = target_index if target_index is not None else target_name
            if key not in data_symbols:
                data_item: dict[str, Any] = {
                    "name": _forensics_text(target_name),
                    "address": _objdiff_int(target_symbol.get("address")),
                    "size": _objdiff_int(target_symbol.get("size"), 0) or 0,
                    "order": len(data_symbols),
                    "symbol_index": target_index,
                    "first_offset": address,
                }
                compact, byte_counts = _target_data_bytes(target_symbol)
                if compact is not None:
                    data_item["bytes"] = compact
                data_item["bytes_emitted"] = byte_counts["emitted"]
                data_item["bytes_total"] = byte_counts["total"]
                data_item["bytes_truncated"] = byte_counts["truncated"]
                data_symbols[key] = data_item
    start = symbol_start if symbol_start is not None else (first_address if first_address is not None else 0)
    relocations.sort(
        key=lambda item: (
            item["offset"] is None,
            item["offset"] if item["offset"] is not None else 0,
            str(item.get("type", "")),
            str(item.get("target", "")),
            _objdiff_int(item.get("addend"), 0) or 0,
            str(item.get("kind", "")),
        )
    )
    direct_call_edges.sort(
        key=lambda item: (
            item["offset"] is None,
            item["offset"] if item["offset"] is not None else 0,
            str(item.get("target", "")),
        )
    )
    callback_edges.sort(
        key=lambda item: (
            item["offset"] is None,
            item["offset"] if item["offset"] is not None else 0,
            str(item.get("type", "")),
            str(item.get("target", "")),
        )
    )
    direct_call_names = list(dict.fromkeys(item["target"] for item in direct_call_edges))
    ordered_data_symbols = sorted(
        data_symbols.values(),
        key=lambda item: (
            item["first_offset"] is None,
            item["first_offset"] if item["first_offset"] is not None else 0,
            item["address"] is None,
            item["address"] if item["address"] is not None else 0,
            str(item.get("name", "")),
            item.get("symbol_index") if item.get("symbol_index") is not None else -1,
        ),
    )
    for order, item in enumerate(ordered_data_symbols):
        item["order"] = order
        item.pop("symbol_index", None)
        item.pop("first_offset", None)
    diagnostic_values = (
        [_forensics_text(item) for item in diagnostics if isinstance(item, str)]
        if isinstance(diagnostics, (list, tuple))
        else []
    )
    counts = {
        "relocations": len(relocations),
        "direct_rel24_calls": len(direct_call_names),
        "direct_calls": len(direct_call_names),
        "direct_call_edges": len(direct_call_edges),
        "callback_edges": len(callback_edges),
        "data_symbols": len(ordered_data_symbols),
        "r1_offsets": len(r1_offsets),
        "registers": len(registers),
        "diagnostics": len(diagnostic_values),
    }
    limits = {
        "relocations": FORENSICS_RELOCATION_LIMIT,
        "direct_rel24_calls": FORENSICS_DIRECT_CALL_LIMIT,
        "direct_calls": FORENSICS_DIRECT_CALL_LIMIT,
        "direct_call_edges": FORENSICS_DIRECT_CALL_LIMIT,
        "callback_edges": FORENSICS_CALLBACK_LIMIT,
        "data_symbols": FORENSICS_DATA_SYMBOL_LIMIT,
        "r1_offsets": FORENSICS_R1_OFFSET_LIMIT,
        "registers": FORENSICS_REGISTER_LIMIT,
        "diagnostics": FORENSICS_DIAGNOSTIC_LIMIT,
    }
    truncated = {
        name: max(0, counts[name] - limit)
        for name, limit in limits.items()
    }
    emitted_data_symbols = ordered_data_symbols[:FORENSICS_DATA_SYMBOL_LIMIT]
    emitted_r1_offsets = sorted(r1_offsets)[:FORENSICS_R1_OFFSET_LIMIT]
    emitted_registers = sorted(registers)[:FORENSICS_REGISTER_LIMIT]
    target_range = {"start": start, "end": start + size, "size": size}
    return {
        "target_range": target_range,
        "relocations": relocations[:FORENSICS_RELOCATION_LIMIT],
        "direct_rel24_calls": direct_call_names[:FORENSICS_DIRECT_CALL_LIMIT],
        "direct_calls": direct_call_names[:FORENSICS_DIRECT_CALL_LIMIT],
        "direct_call_edges": direct_call_edges[:FORENSICS_DIRECT_CALL_LIMIT],
        "callback_edges": callback_edges[:FORENSICS_CALLBACK_LIMIT],
        "data_symbols": emitted_data_symbols,
        "counts": counts,
        "truncated": truncated,
        "stack_clues": {
            "prologue": prologue,
            "frame_size": frame_size,
            "r1_offsets": emitted_r1_offsets,
            "registers": emitted_registers,
            "diagnostics": diagnostic_values[:FORENSICS_DIAGNOSTIC_LIMIT],
        },
    }


def source_calls(text: str, symbol: str, known: set[str]) -> list[str]:
    for item in parse_functions(text):
        if item.symbol == symbol:
            masked = _mask_c(function_text(text, item))
            values = CALL_RE.findall(masked)
            if values and values[0] == symbol:
                values = values[1:]
            return [value for value in values if value not in CALL_KEYWORDS and value not in known]
    return []


def report_state(report: Mapping[str, Any]) -> dict[str, Any]:
    left = functions(report.get("left", {}))
    exact = {item["name"] for item in left if is_exact(item)}
    sizes = {item["name"]: number(item.get("size")) for item in left}
    paired = {item["name"] for item in left if item.get("target_symbol") is not None}
    return {"exact": exact, "paired": paired, "sizes": sizes, "total": len(left)}


def delta(current: Mapping[str, Any], baseline: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if baseline is None:
        return None
    current_state = report_state(current)
    base_state = report_state(baseline)
    gained = sorted(current_state["exact"] - base_state["exact"])
    regressed = sorted(base_state["exact"] - current_state["exact"])
    return {
        "newly_exact": gained,
        "newly_exact_bytes": sum(current_state["sizes"].get(name, 0) for name in gained),
        "regressed_exact": regressed,
        "regressed_bytes": sum(base_state["sizes"].get(name, 0) for name in regressed),
        "paired_gain": len(current_state["paired"]) - len(base_state["paired"]),
    }


def order_diagnostics(target_order: Sequence[str], source_order: Sequence[str]) -> dict[str, Any]:
    common = [name for name in target_order if name in set(source_order)]
    source_common = [name for name in source_order if name in set(target_order)]
    source_rank = {name: index for index, name in enumerate(source_common)}
    sequence = [source_rank[name] for name in common]
    inversions = 0
    for index, value in enumerate(sequence):
        inversions += sum(other < value for other in sequence[index + 1 :])
    displaced = [
        {
            "function": name,
            "target_rank": index,
            "source_rank": source_rank[name],
            "delta": source_rank[name] - index,
        }
        for index, name in enumerate(common)
        if source_rank[name] != index
    ]
    displaced.sort(key=lambda item: (-abs(item["delta"]), item["target_rank"], item["function"]))
    return {
        "common_functions": len(common),
        "order_matches": common == source_common,
        "inversions": inversions,
        "displaced": displaced,
        "target_order": list(target_order),
        "source_definition_order": list(source_order),
    }


def normalize_tokens(name: str) -> list[str]:
    value = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name)
    tokens = [part.lower() for part in re.split(r"[^A-Za-z0-9]+", value) if part]
    result: list[str] = []
    aliases = {"capsule": "cap", "destroy": "kill", "object": "obj"}
    for token in tokens:
        if token in {"mb", "mbev", "ev", "exec", "main", "void"}:
            continue
        result.append(aliases.get(token, token))
    return result


def name_score(target: str, donor: str) -> float:
    left = normalize_tokens(target)
    right = normalize_tokens(donor)
    if not left or not right:
        return 0.0
    union = set(left) | set(right)
    jaccard = len(set(left) & set(right)) / len(union)
    sequence = difflib.SequenceMatcher(None, "_".join(left), "_".join(right)).ratio()
    return max(jaccard, sequence)


def lcs_ratio(left: Sequence[str], right: Sequence[str]) -> float:
    if not left or not right:
        return 0.0
    a = ["_".join(normalize_tokens(item)) for item in left]
    b = ["_".join(normalize_tokens(item)) for item in right]
    row = [0] * (len(b) + 1)
    for item in a:
        previous = row[:]
        for index, other in enumerate(b, 1):
            row[index] = previous[index - 1] + 1 if item == other else max(previous[index], row[index - 1])
    return (2.0 * row[-1]) / (len(a) + len(b))


def ordered_lcs(left: Sequence[str], right: Sequence[str]) -> list[str]:
    rows: list[list[list[str]]] = [[[] for _ in range(len(right) + 1)] for _ in range(len(left) + 1)]
    for left_index, left_item in enumerate(left, 1):
        for right_index, right_item in enumerate(right, 1):
            if left_item == right_item:
                rows[left_index][right_index] = [*rows[left_index - 1][right_index - 1], left_item]
            else:
                above = rows[left_index - 1][right_index]
                before = rows[left_index][right_index - 1]
                rows[left_index][right_index] = above if len(above) >= len(before) else before
    return rows[-1][-1]


def filtered_cluster_calls(calls: Sequence[Any]) -> list[str]:
    return [
        name for name in (str(call) for call in calls)
        if name not in GENERIC_CLUSTER_CALLS
        and not name.startswith(("_savegpr_", "_restgpr_", "_savefpr_", "_restfpr_", "__"))
    ]


def strong_ordered_call_overlap(left: Sequence[str], right: Sequence[str]) -> bool:
    if min(len(left), len(right)) < MIN_ORDERED_CLUSTER_CALLS:
        return False
    common = ordered_lcs(left, right)
    ratio = (2.0 * len(common)) / (len(left) + len(right))
    return len(common) >= MIN_ORDERED_CLUSTER_CALLS and ratio >= MIN_ORDERED_CLUSTER_RATIO


def git_value(repo: Path, *args: str) -> str | None:
    result = subprocess.run(["git", "-C", str(repo), *args], text=True, capture_output=True, check=False)
    return result.stdout.strip() if result.returncode == 0 else None


def git_path_clean(repo: Path, relative: str) -> bool:
    result = subprocess.run(
        ["git", "-C", str(repo), "diff", "--quiet", "HEAD", "--", relative],
        capture_output=True,
        check=False,
    )
    return result.returncode == 0


def donor_cache_key(roots: Sequence[Path]) -> dict[str, Any]:
    values = []
    for root in roots:
        repo = root.resolve()
        values.append(
            {
                "root": str(repo),
                "head": git_value(repo, "rev-parse", "HEAD"),
                "board_status": git_value(repo, "status", "--porcelain", "--", "src/board", "src/game/board"),
            }
        )
    return {"schema_version": 2, "roots": values}


def donor_catalog(
    roots: Sequence[Path], cache_path: Path | None = None
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], bool]:
    key = donor_cache_key(roots)
    if cache_path and cache_path.is_file():
        cached = read_json(cache_path)
        if cached.get("key") == key:
            return list(cached.get("functions", [])), list(cached.get("provenance", [])), True
    functions_out: list[dict[str, Any]] = []
    provenance: list[dict[str, Any]] = []
    for root in roots:
        repo = root.resolve()
        head = git_value(repo, "rev-parse", "HEAD")
        remote = git_value(repo, "remote", "get-url", "origin")
        root_record = {"root": str(repo), "head": head, "remote": remote, "files": 0, "functions": 0}
        candidates = sorted({*repo.glob("src/board/*.c"), *repo.glob("src/**/board/*.c")})
        for path in candidates:
            try:
                text = path.read_text(encoding="utf-8")
            except (OSError, UnicodeDecodeError):
                continue
            parsed = parse_functions(text)
            known = {item.symbol for item in parsed}
            relative = path.relative_to(repo).as_posix()
            committed_blob = git_value(repo, "rev-parse", f"HEAD:{relative}") if head else None
            authenticated = bool(committed_blob and git_path_clean(repo, relative))
            root_record["files"] += 1
            root_record["functions"] += len(parsed)
            for item in parsed:
                body = function_text(text, item)
                masked = _mask_c(body)
                calls = CALL_RE.findall(masked)
                if calls and calls[0] == item.symbol:
                    calls = calls[1:]
                functions_out.append(
                    {
                        "symbol": item.symbol,
                        "calls": [call for call in calls if call not in CALL_KEYWORDS and call not in known],
                        "source": relative,
                        "line": item.start,
                        "repository": str(repo),
                        "remote": remote,
                        "commit": head,
                        "blob": committed_blob,
                        "provenance_authenticated": authenticated,
                    }
                )
        provenance.append(root_record)
    if cache_path:
        atomic_json(cache_path, {"key": key, "functions": functions_out, "provenance": provenance})
    return functions_out, provenance, False


def rank_donors(target: str, calls: Sequence[str], catalog: Sequence[Mapping[str, Any]], limit: int = 5) -> list[dict[str, Any]]:
    ranked: list[dict[str, Any]] = []
    for item in catalog:
        lexical = name_score(target, str(item["symbol"]))
        skeleton = lcs_ratio(calls, list(item.get("calls", [])))
        if lexical < 0.35 and skeleton < 0.35:
            continue
        score = round(100.0 * (0.55 * skeleton + 0.45 * lexical), 3)
        ranked.append({**dict(item), "name_score": round(lexical, 4), "call_lcs_score": round(skeleton, 4), "score": score})
    ranked.sort(key=lambda item: (-item["provenance_authenticated"], -item["score"], item["source"], item["line"], item["symbol"]))
    return ranked[:limit]


def graph_context(
    path: Path | None,
    source: Path,
    source_functions: Sequence[str],
    head: str | None,
) -> dict[str, Any] | None:
    if path is None or not path.is_file():
        return None
    graph = read_json(path)
    nodes = {str(node.get("id")): node for node in graph.get("nodes", [])}
    basename = source.name
    file_ids = {node_id for node_id, node in nodes.items() if node.get("label") == basename or node.get("source_file") == basename and str(node.get("label", "")).endswith(".c")}
    contained = {link.get("target") for link in graph.get("links", []) if link.get("relation") == "contains" and link.get("source") in file_ids}
    neighbors: Counter[str] = Counter()
    for link in graph.get("links", []):
        if link.get("relation") != "calls" or link.get("source") not in contained:
            continue
        node = nodes.get(str(link.get("target")), {})
        neighbor = str(node.get("source_file", ""))
        if neighbor and neighbor != basename:
            neighbors[neighbor] += 1
    built = graph.get("built_at_commit")
    graph_functions = {
        str(node.get("label", ""))[:-2]
        for node in nodes.values()
        if node.get("source_file") == basename and str(node.get("label", "")).endswith("()")
    }
    current_functions = set(source_functions)
    manifest_path = path.parent / "manifest.json"
    manifest = read_json(manifest_path) if manifest_path.is_file() else {}
    manifest_record = manifest.get(basename, {}) if isinstance(manifest, Mapping) else {}
    source_md5 = hashlib.md5(source.read_bytes()).hexdigest()
    manifest_hash = manifest_record.get("ast_hash") if isinstance(manifest_record, Mapping) else None
    reasons: list[str] = []
    if built != head:
        reasons.append("built commit differs from current commit")
    if manifest_hash != source_md5:
        reasons.append("source content hash differs from Graphify manifest")
    missing_functions = sorted(current_functions - graph_functions)
    extra_functions = sorted(graph_functions - current_functions)
    if missing_functions:
        reasons.append(f"graph misses {len(missing_functions)} current source functions")
    if extra_functions:
        reasons.append(f"graph retains {len(extra_functions)} removed source functions")
    return {
        "path": str(path),
        "built_at_commit": built,
        "current_commit": head,
        "fresh": not reasons,
        "stale_reasons": reasons,
        "source_md5": source_md5,
        "manifest_ast_hash": manifest_hash,
        "source_function_count": len(current_functions),
        "graph_function_count": len(graph_functions),
        "missing_functions": missing_functions,
        "extra_functions": extra_functions,
        "refresh_needed": bool(reasons),
        "refresh_working_directory": str(path.parent),
        "refresh_command": f'graphify update "{source.parent}" --no-cluster',
        "refresh_constraint": "run serially between builds, then rerun this report before dependency batching",
        "cross_file_call_neighbors": [{"source": name, "edges": count} for name, count in sorted(neighbors.items(), key=lambda item: (-item[1], item[0]))[:12]],
    }


def cstring(blob: bytes, offset: int) -> str:
    end = blob.find(b"\0", offset)
    if offset < 0 or offset >= len(blob):
        return ""
    return blob[offset : len(blob) if end < 0 else end].decode("utf-8", errors="replace")


def elf_sections(data: bytes) -> list[ElfSection]:
    if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 2:
        raise ValueError("expected ELF32 big-endian object")
    shoff = struct.unpack_from(">I", data, 32)[0]
    shentsize, shnum, shstrndx = struct.unpack_from(">HHH", data, 46)
    raw = [struct.unpack_from(">IIIIIIIIII", data, shoff + index * shentsize) for index in range(shnum)]
    string_row = raw[shstrndx]
    names = data[string_row[4] : string_row[4] + string_row[5]]
    return [ElfSection(cstring(names, row[0]), index, row[4], row[5], row[6], row[9], row[1]) for index, row in enumerate(raw)]


def elf_literals(path: Path) -> list[dict[str, Any]]:
    try:
        data = path.read_bytes()
        sections = elf_sections(data)
    except (OSError, ValueError, struct.error):
        return []
    by_index = {section.index: section for section in sections}
    wanted = {section.index: section for section in sections if section.name in {".sdata2", ".rodata"}}
    symbols: list[dict[str, Any]] = []
    for table in sections:
        if table.kind != 2 or table.entsize < 16 or table.link not in by_index:
            continue
        strings_section = by_index[table.link]
        strings = data[strings_section.offset : strings_section.offset + strings_section.size]
        for offset in range(table.offset, table.offset + table.size, table.entsize):
            name_at, value, size = struct.unpack_from(">III", data, offset)
            info = data[offset + 12]
            shndx = struct.unpack_from(">H", data, offset + 14)[0]
            if shndx not in wanted or (info & 15) not in {0, 1}:
                continue
            symbols.append({"name": cstring(strings, name_at), "offset": value, "size": size, "section_index": shndx})
    result: list[dict[str, Any]] = []
    for section_index, section in wanted.items():
        members = sorted((item for item in symbols if item["section_index"] == section_index), key=lambda item: (item["offset"], item["name"]))
        for index, item in enumerate(members):
            inferred = item["size"]
            if inferred == 0:
                next_offset = members[index + 1]["offset"] if index + 1 < len(members) else section.size
                inferred = next_offset - item["offset"]
            if inferred not in {4, 8} or item["offset"] + inferred > section.size:
                continue
            start = section.offset + item["offset"]
            raw = data[start : start + inferred]
            value = struct.unpack(">f" if inferred == 4 else ">d", raw)[0]
            if not math.isfinite(value):
                continue
            result.append(
                {
                    "section": section.name,
                    "symbol": item["name"] or f"section_offset_{item['offset']}",
                    "offset": item["offset"],
                    "width": inferred,
                    "kind": "float32" if inferred == 4 else "float64",
                    "decimal": fixed_decimal(value),
                    "value": value,
                }
            )
    return result


def fixed_decimal(value: float) -> str:
    raw = repr(value)
    if "e" not in raw.lower():
        return raw
    return format(value, ".24f").rstrip("0").rstrip(".") or "0"


def macro_index(root: Path) -> dict[str, Any]:
    integers: dict[int, list[str]] = defaultdict(list)
    floats: dict[bytes, list[str]] = defaultdict(list)
    for path in sorted((root / "include").rglob("*.h")):
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        for match in NUMERIC_DEFINE_RE.finditer(text):
            name, token = match.groups()
            try:
                if token.lower().startswith("0x"):
                    integers[int(token, 16)].append(name)
                elif any(mark in token.lower() for mark in (".", "e")):
                    value = float(token)
                    floats[struct.pack(">f", value)].append(name)
                    floats[struct.pack(">d", value)].append(name)
                else:
                    integers[int(token, 10)].append(name)
            except (OverflowError, ValueError, struct.error):
                continue
    return {"integers": integers, "floats": floats}


def literal_report(target_object: Path, macros: Mapping[str, Any]) -> list[dict[str, Any]]:
    values = elf_literals(target_object)
    for item in values:
        try:
            packed = struct.pack(">f" if item["width"] == 4 else ">d", item.pop("value"))
        except (OverflowError, struct.error):
            packed = b""
        item["macro_candidates"] = sorted(set(macros["floats"].get(packed, [])))[:12]
    return values


def immediate_macro_matches(
    symbol: Mapping[str, Any], macros: Mapping[str, Any], source_body: str = ""
) -> list[dict[str, Any]]:
    matches: list[dict[str, Any]] = []
    pending_high: dict[str, int] = {}
    for row in symbol.get("instructions", []):
        formatted = instruction(row)
        opcode, operands = split_instruction(formatted)
        signed_args = []
        for part in row.get("instruction", {}).get("parts", []):
            if not isinstance(part, Mapping) or not isinstance(part.get("arg"), Mapping):
                continue
            arg = part["arg"]
            if arg.get("signed") is not None:
                signed_args.append(number(arg.get("signed"), 2**63))
            elif arg.get("unsigned") is not None:
                signed_args.append(number(arg.get("unsigned"), 2**63))
        if opcode == "lis" and len(operands) >= 2 and signed_args:
            pending_high[operands[0]] = (signed_args[-1] & 65535) << 16
            value = pending_high[operands[0]]
        elif opcode == "ori" and len(operands) >= 3 and operands[1] in pending_high and signed_args:
            value = pending_high[operands[1]] | (signed_args[-1] & 65535)
            pending_high[operands[0]] = value
        elif opcode == "li" and signed_args:
            value = signed_args[-1]
        else:
            continue
        names = sorted(
            set(macros["integers"].get(value, [])),
            key=lambda name: (name not in source_body, not name.startswith(("HU_", "MB_")), name),
        )
        if names and value not in {-1, 0, 1} and (abs(value) >= 16 or len(names) <= 4):
            matches.append(
                {
                    "instruction": formatted,
                    "decimal": value,
                    "macro_candidates": names[:12],
                    "source_macro_matches": [name for name in names if name in source_body][:6],
                }
            )
    unique: dict[tuple[int, tuple[str, ...]], dict[str, Any]] = {}
    for item in matches:
        unique[(item["decimal"], tuple(item["macro_candidates"]))] = item
    return list(unique.values())


def select_cards(root: Path, card_ids: Iterable[str]) -> list[dict[str, Any]]:
    data = load(root, validate=False)
    cards = {str(card.get("id")): card for card in data.get("patterns", [])}
    selected: list[dict[str, Any]] = []
    for card_id in sorted(set(card_ids)):
        card = cards.get(card_id)
        if not card:
            selected.append({"id": card_id, "missing": True})
            continue
        freshness = card_freshness(data, card_id)
        selected.append(
            {
                "id": card_id,
                "title": card.get("title"),
                "classification": card.get("classification"),
                "confidence": card.get("confidence"),
                "freshness": freshness.get("effective_status", freshness.get("status", "unknown")),
                "freshness_reason": freshness.get("reason"),
                "rule": card.get("rule"),
                "safe_actions": list(card.get("safe_actions", []))[:4],
                "counterexamples": list(card.get("counterexamples", []))[:3],
            }
        )
    return selected


def paired_single_quarantine(
    root: Path,
) -> tuple[dict[str, Any] | None, set[str], str, str | None]:
    """Return the explicitly-scoped exhausted paired-single card, if present."""
    data = load(root, validate=False)
    card = next(
        (item for item in data.get("patterns", []) if item.get("id") == PAIRED_SINGLE_QUARANTINE_CARD),
        None,
    )
    if not isinstance(card, dict):
        return None, set(), "missing", None
    applicability = card.get("applicability", {})
    stable_ids = applicability.get("stable_ids", []) if isinstance(applicability, Mapping) else []
    freshness = card_freshness(data, PAIRED_SINGLE_QUARANTINE_CARD)
    return (
        card,
        {str(item) for item in stable_ids if isinstance(item, str)},
        str(freshness.get("effective_status", freshness.get("status", "unknown"))),
        freshness.get("reason"),
    )


def source_local_identifiers(body: str) -> dict[str, list[str]]:
    """Return recovered local type/work names without inferring new source facts."""
    types: set[str] = set()
    work_identifiers: set[str] = set()
    for match in re.finditer(
        r"\b(?:const\s+)?([A-Za-z_]\w*)\s*(?:\*+\s*)?([A-Za-z_]\w*)\s*(?:[=;,\[])",
        _mask_c(body),
    ):
        type_name, local_name = match.groups()
        if type_name.startswith("Hu") or "work" in type_name.lower():
            types.add(type_name)
        if "work" in local_name.lower() or "work" in type_name.lower():
            work_identifiers.add(local_name)
    return {
        "types": sorted(types),
        "work_identifiers": sorted(work_identifiers),
    }


def relocation_identity_pattern(
    target: Mapping[str, Any], *, strict_exact: bool, value_exact: bool
) -> str:
    if target.get("target_symbol") is None:
        return "unpaired"
    if strict_exact:
        return "strict_exact"
    if value_exact:
        return "data_value_exact_only"
    return "paired_instruction_residual"


def cluster_cause(item: Mapping[str, Any]) -> str | None:
    diagnostics = [str(value) for value in item.get("diagnostics", [])]
    concrete = sorted(
        value for value in diagnostics
        if value in {
            "branch_destination_only",
            "floating_commutative_swap",
            "integer_commutative_swap",
            "local_declaration_or_first_use_cycle",
        }
    )
    if concrete:
        return "+".join(concrete)
    if item.get("category") == "relocation_identity_only":
        return "relocation_identity_only"
    return None


def cluster_feature_key(item: Mapping[str, Any]) -> tuple[str, str, tuple[tuple[str, int], ...], str, str] | None:
    cause = cluster_cause(item)
    if cause is None:
        return None
    delta = item.get("target_source_size_delta")
    delta_shape = "unknown" if delta is None else "equal" if delta == 0 else "source_larger" if delta > 0 else "source_smaller"
    return (
        cause,
        str(item.get("category")),
        tuple(sorted((str(kind), number(count)) for kind, count in item.get("diff_kinds", {}).items())),
        str(item.get("relocation_identity_pattern")),
        delta_shape,
    )


def generic_call_cluster_key(item: Mapping[str, Any]) -> tuple[str, int, tuple[str, ...]] | None:
    if item.get("category") != "paired_residual":
        return None
    if item.get("relocation_identity_pattern") != "paired_instruction_residual":
        return None
    delta = item.get("target_source_size_delta")
    calls = filtered_cluster_calls(item.get("target_call_skeleton", []))
    diff_kinds = tuple(sorted(str(kind) for kind in item.get("diff_kinds", {})))
    if delta is None or len(calls) < MIN_ORDERED_CLUSTER_CALLS or len(set(calls)) < 2 or not diff_kinds:
        return None
    return "paired_instruction_residual", number(delta), diff_kinds


def repeated_target_call_clusters(ranked: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    buckets: dict[tuple[str, int, tuple[str, ...]], list[Mapping[str, Any]]] = defaultdict(list)
    for item in ranked:
        key = generic_call_cluster_key(item)
        if not item.get("strict_exact") and not item.get("quarantined_by_card") and key is not None:
            buckets[key].append(item)
    clusters: list[dict[str, Any]] = []
    for key, bucket in sorted(buckets.items()):
        remaining = sorted(bucket, key=lambda item: str(item["function"]))
        while remaining:
            seed, *candidates = remaining
            members = [seed]
            rejected: list[Mapping[str, Any]] = []
            for candidate in candidates:
                candidate_calls = filtered_cluster_calls(candidate.get("target_call_skeleton", []))
                if all(
                    strong_ordered_call_overlap(
                        candidate_calls,
                        filtered_cluster_calls(member.get("target_call_skeleton", [])),
                    )
                    for member in members
                ):
                    members.append(candidate)
                else:
                    rejected.append(candidate)
            remaining = rejected
            if len(members) < 2:
                continue
            common_calls = filtered_cluster_calls(members[0].get("target_call_skeleton", []))
            for member in members[1:]:
                common_calls = ordered_lcs(
                    common_calls, filtered_cluster_calls(member.get("target_call_skeleton", []))
                )
            if len(common_calls) < MIN_ORDERED_CLUSTER_CALLS or len(set(common_calls)) < 2:
                continue
            target_bytes = sum(number(item.get("target_bytes")) for item in members)
            actionable = len(members) >= 3 or target_bytes >= 1024
            diff_ranges = {
                kind: {
                    "min": min(number(item.get("diff_kinds", {}).get(kind)) for item in members),
                    "max": max(number(item.get("diff_kinds", {}).get(kind)) for item in members),
                }
                for kind in key[2]
            }
            clusters.append(
                {
                    "cluster_id": "",
                    "cause": "repeated_target_call_skeleton",
                    "category": "paired_residual",
                    "functions": [str(item["function"]) for item in members],
                    "function_count": len(members),
                    "target_bytes": target_bytes,
                    "diff_kind_shape": {kind: values["min"] for kind, values in diff_ranges.items()},
                    "diff_kind_count_ranges": diff_ranges,
                    "relocation_identity_pattern": key[0],
                    "target_source_size_delta_shape": (
                        "equal" if key[1] == 0 else "source_larger" if key[1] > 0 else "source_smaller"
                    ),
                    "target_source_size_deltas": [key[1]],
                    "shared_evidence": {"ordered_target_call_skeleton": common_calls},
                    "knowledge_card_id": TARGET_CALL_CLUSTER_CARD,
                    "concrete_shared_cause": True,
                    "actionable": actionable,
                    "expected_compiler_probes": 1,
                    "expected_exact_bytes": target_bytes,
                    "expected_exact_bytes_per_compiler_probe": target_bytes,
                }
            )
    return clusters


def plan_ready_missing_definition_families(ranked: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    """Group only dependency-closed, named missing-definition implementation families."""
    missing = [
        item for item in ranked
        if item.get("category") == "missing_definition" and not item.get("quarantined_by_card")
    ]
    missing_names = {str(item["function"]) for item in missing}
    buckets: dict[tuple[str, ...], list[Mapping[str, Any]]] = defaultdict(list)
    for item in missing:
        tokens = normalize_tokens(str(item["function"]))
        if len(tokens) >= 4:
            buckets[tuple(tokens[:3])].append(item)
    families: list[dict[str, Any]] = []
    for prefix, members in sorted(buckets.items()):
        if len(members) < 2:
            continue
        names = {str(item["function"]) for item in members}
        edges = sorted(
            (str(item["function"]), str(call))
            for item in members
            for call in item.get("target_call_skeleton", [])
            if str(call) in names
        )
        if not edges:
            continue
        if any(
            str(call) not in names
            for item in members
            for call in item.get("target_call_skeleton", [])
            if str(call) in missing_names
        ):
            continue
        target_bytes = sum(number(item.get("target_bytes")) for item in members)
        if target_bytes < 1024:
            continue
        families.append(
            {
                "cluster_id": "",
                "cause": "dependency_closed_missing_family",
                "category": "missing_definition",
                "functions": sorted(names),
                "function_count": len(members),
                "target_bytes": target_bytes,
                "implementation_ready": True,
                "actionable": True,
                "actionability_reason": "named_dependency_closed_missing_family",
                "shared_evidence": {
                    "normalized_prefix": list(prefix),
                    "internal_target_call_edges": [
                        {"caller": caller, "callee": callee} for caller, callee in edges
                    ],
                },
                "implementation_target_bytes": target_bytes,
                "potential_exact_bytes": target_bytes,
                "expected_compiler_probes": 0,
            }
        )
    sibling_buckets: dict[tuple[int, tuple[str, ...], tuple[str, ...]], list[Mapping[str, Any]]] = defaultdict(list)
    for item in missing:
        tokens = normalize_tokens(str(item["function"]))
        calls = tuple(filtered_cluster_calls(item.get("target_call_skeleton", [])))
        prefix = tuple(tokens[:-1])
        if len(prefix) >= 2 and calls:
            sibling_buckets[(number(item.get("target_bytes")), calls, prefix)].append(item)
    for (target_size, calls, prefix), members in sorted(sibling_buckets.items()):
        if len(members) < 2:
            continue
        target_bytes = target_size * len(members)
        if target_bytes < MIN_HOMOLOGOUS_MISSING_FAMILY_BYTES:
            continue
        families.append(
            {
                "cluster_id": "",
                "cause": "homologous_missing_definition_siblings",
                "category": "missing_definition",
                "functions": sorted(str(item["function"]) for item in members),
                "function_count": len(members),
                "target_bytes": target_bytes,
                "implementation_ready": True,
                "actionable": True,
                "actionability_reason": "exact_structural_missing_definition_siblings",
                "shared_evidence": {
                    "normalized_prefix": list(prefix),
                    "target_byte_size": target_size,
                    "ordered_nonruntime_target_calls": list(calls),
                },
                "implementation_target_bytes": target_bytes,
                "potential_exact_bytes": target_bytes,
                "expected_compiler_probes": 0,
            }
        )
    return families


def preferred_missing_definition_packet(
    ranked: Sequence[Mapping[str, Any]],
) -> dict[str, Any] | None:
    """Choose the largest dependency-closed missing-definition packet in pass bounds."""
    missing = {
        str(item["function"]): item
        for item in ranked
        if item.get("category") == "missing_definition"
        and not item.get("quarantined_by_card")
    }
    if not missing:
        return None

    adjacency = {name: set() for name in missing}
    for name, item in missing.items():
        for call in item.get("target_call_skeleton", []):
            callee = str(call)
            if callee in missing:
                adjacency[name].add(callee)
                adjacency[callee].add(name)

    components: list[dict[str, Any]] = []
    seen: set[str] = set()
    ordered_names = sorted(
        missing,
        key=lambda name: (number(missing[name].get("target_rank")), name),
    )
    for start in ordered_names:
        if start in seen:
            continue
        stack = [start]
        seen.add(start)
        members: list[str] = []
        while stack:
            name = stack.pop()
            members.append(name)
            for neighbor in adjacency[name]:
                if neighbor not in seen:
                    seen.add(neighbor)
                    stack.append(neighbor)
        members.sort(key=lambda name: (number(missing[name].get("target_rank")), name))
        target_bytes = sum(number(missing[name].get("target_bytes")) for name in members)
        if (
            len(members) <= PREFERRED_PACKET_MAX_FUNCTIONS
            and target_bytes <= PREFERRED_PACKET_MAX_BYTES
        ):
            components.append(
                {
                    "functions": members,
                    "function_count": len(members),
                    "target_bytes": target_bytes,
                    "first_target_rank": min(
                        number(missing[name].get("target_rank")) for name in members
                    ),
                }
            )

    states: dict[tuple[int, int], tuple[int, ...]] = {(0, 0): ()}
    for component_index, component in enumerate(components):
        additions: dict[tuple[int, int], tuple[int, ...]] = {}
        for (function_count, target_bytes), chosen in list(states.items()):
            next_count = function_count + number(component["function_count"])
            next_bytes = target_bytes + number(component["target_bytes"])
            if (
                next_count > PREFERRED_PACKET_MAX_FUNCTIONS
                or next_bytes > PREFERRED_PACKET_MAX_BYTES
            ):
                continue
            state = (next_count, next_bytes)
            candidate = (*chosen, component_index)
            current = states.get(state) or additions.get(state)
            if current is None or candidate < current:
                additions[state] = candidate
        states.update(additions)

    ready_states = [
        state
        for state in states
        if state[0] >= PREFERRED_PACKET_MIN_FUNCTIONS
        and state[1] >= PREFERRED_PACKET_MIN_BYTES
    ]
    candidate_states = ready_states or [state for state in states if state != (0, 0)]
    if not candidate_states:
        return None
    selected_state = max(
        candidate_states,
        key=lambda state: (
            state[1],
            state[0],
            -sum(components[index]["first_target_rank"] for index in states[state]),
        ),
    )
    selected_components = [components[index] for index in states[selected_state]]
    selected_functions = sorted(
        (
            name
            for component in selected_components
            for name in component["functions"]
        ),
        key=lambda name: (number(missing[name].get("target_rank")), name),
    )
    return {
        "ready": selected_state in ready_states,
        "dependency_closed": True,
        "function_count": selected_state[0],
        "target_bytes": selected_state[1],
        "functions": selected_functions,
        "component_count": len(selected_components),
        "constraints": {
            "min_functions": PREFERRED_PACKET_MIN_FUNCTIONS,
            "max_functions": PREFERRED_PACKET_MAX_FUNCTIONS,
            "min_bytes": PREFERRED_PACKET_MIN_BYTES,
            "max_bytes": PREFERRED_PACKET_MAX_BYTES,
        },
    }


def worker_dispatch(report: Mapping[str, Any]) -> dict[str, Any]:
    """Choose work that can produce a measured result without reopening dead ends."""
    strict_delta = report.get("strict_delta") or {}
    value_delta = report.get("data_value_delta") or {}
    regressions = sorted(
        set(strict_delta.get("regressed_exact", []))
        | set(value_delta.get("regressed_exact", []))
    )
    ranked = list(report.get("ranked_functions", []))
    by_name = {str(item.get("function")): item for item in ranked}
    if regressions:
        return {
            "mode": "regression_reconciliation",
            "ready": True,
            "reason": "repair or revert exact regressions before opening new work",
            "functions": regressions,
            "function_count": len(regressions),
            "target_bytes": sum(number(by_name.get(name, {}).get("target_bytes")) for name in regressions),
        }

    pending = [item for item in ranked if item.get("source_pending_build")]
    if pending:
        pending.sort(key=lambda item: (number(item.get("target_rank")), str(item.get("function"))))
        return {
            "mode": "verification_first",
            "ready": True,
            "reason": "source definitions changed after the compiled object; establish a clean baseline before more edits",
            "functions": [str(item["function"]) for item in pending],
            "function_count": len(pending),
            "target_bytes": sum(number(item.get("target_bytes")) for item in pending),
        }

    preferred = report.get("preferred_implementation_packet")
    if isinstance(preferred, Mapping) and preferred.get("functions"):
        selected = set(str(name) for name in preferred["functions"])
        missing = {
            str(item.get("function"))
            for item in ranked
            if item.get("category") == "missing_definition"
        }
        bulk_bounds_met = bool(preferred.get("ready")) or (
            number(preferred.get("function_count")) >= PREFERRED_PACKET_MIN_FUNCTIONS
            or number(preferred.get("target_bytes")) >= PREFERRED_PACKET_MIN_BYTES
        )
        if not preferred.get("ready") and missing - selected:
            return {
                "mode": "evidence_preflight",
                "ready": False,
                "bulk_bounds_met": bulk_bounds_met,
                "reason": (
                    "call-closed packet excludes other missing owner functions; validate literal/static-data "
                    "ownership and source provenance before assigning an implementation lane"
                ),
                "functions": list(preferred["functions"]),
                "function_count": number(preferred.get("function_count")),
                "target_bytes": number(preferred.get("target_bytes")),
                "excluded_missing_functions": sorted(missing - selected),
            }
        return {
            "mode": "implementation",
            "ready": True,
            "bulk_bounds_met": bulk_bounds_met,
            "reason": "dependency-closed missing-definition packet in target order",
            "functions": list(preferred["functions"]),
            "function_count": number(preferred.get("function_count")),
            "target_bytes": number(preferred.get("target_bytes")),
        }

    clusters = [
        item for item in report.get("shared_cause_clusters", [])
        if item.get("actionable") and not item.get("owner_audit_only")
    ]
    if clusters:
        cluster = clusters[0]
        return {
            "mode": (
                "implementation" if cluster.get("implementation_ready")
                else "bounded_shared_cause_probe"
            ),
            "ready": True,
            "bulk_bounds_met": (
                number(cluster.get("function_count")) >= PREFERRED_PACKET_MIN_FUNCTIONS
                or number(cluster.get("target_bytes")) >= PREFERRED_PACKET_MIN_BYTES
            ),
            "reason": str(cluster.get("actionability_reason") or cluster.get("cause")),
            "cluster_id": cluster.get("cluster_id"),
            "functions": list(cluster.get("functions", [])),
            "function_count": number(cluster.get("function_count")),
            "target_bytes": number(
                cluster.get("implementation_target_bytes", cluster.get("target_bytes"))
            ),
        }

    return {
        "mode": "rotate_owner",
        "ready": False,
        "reason": "no dependency-closed implementation packet or evidence-linked shared-cause cluster",
        "functions": [],
        "function_count": 0,
        "target_bytes": 0,
    }


def probe_history_summary(
    root: Path,
    unit_name: str,
    functions: Sequence[str],
    *,
    history: Path = PROBE_HISTORY,
    source: str | None = None,
) -> dict[str, Any]:
    """Return compact owner-local negative evidence across canonical ledgers."""
    path = history if history.is_absolute() else root / history
    legacy_records: list[dict[str, Any]] = []
    legacy_status = "missing"
    warning: str | None = None
    if path.is_file():
        try:
            payload = read_json(path)
            probes = payload.get("probes", {}) if isinstance(payload, Mapping) else {}
            if not isinstance(probes, Mapping):
                legacy_status = "invalid"
                warning = "probes is not an object"
                probes = {}
            else:
                legacy_status = "available"
        except ValueError as exc:
            legacy_status = "invalid"
            warning = str(exc)
            probes = {}
    else:
        probes = {}
    owner_id = (
        "main:" + unit_name[len("main/"):]
        if unit_name.startswith("main/")
        else unit_name
    )
    aliases = {unit_name}
    if unit_name.startswith("main/"):
        aliases.add("main:" + unit_name[len("main/"):])
    selected = set(functions)
    for record in probes.values():
        if not isinstance(record, Mapping) or record.get("owner") not in aliases:
            continue
        symbol = str(record.get("symbol", ""))
        if selected and symbol not in selected:
            continue
        legacy_records.append(
            {
                "owner": owner_id,
                "symbol": symbol,
                "symbols": [symbol] if symbol else [],
                "probe_key": record.get("probe_key"),
                "probe": record.get("probe_key"),
                "input_key": record.get("input_key", ""),
                "status": record.get("status"),
                "reason": record.get("reason"),
                "duplicate_of": record.get("duplicate_of"),
                "commit": record.get("commit"),
                "source": "batch-history.json:legacy",
            }
        )
    owner_source = source
    if not owner_source:
        owner_path = owner_id[5:] if owner_id.startswith("main:") else owner_id
        owner_source = owner_path if owner_path.endswith(".c") else f"src/{owner_path}.c"
    canonical_records: list[dict[str, Any]] = []
    try:
        for record in collect_rejected_probe_history(
            root,
            {"id": owner_id, "source": owner_source},
            target_symbols=list(functions),
            limit=12,
        ):
            symbols = record.get("symbols")
            symbol_values = [str(item) for item in symbols if isinstance(item, str)] if isinstance(symbols, Sequence) and not isinstance(symbols, (str, bytes, bytearray)) else []
            symbol = symbol_values[0] if symbol_values else ""
            record_source = str(record.get("source", ""))
            record_status = record.get("status") or (
                "blocked" if record_source.startswith("blocked.json:") else ""
            )
            canonical_records.append(
                {
                    "owner": record.get("owner"),
                    "symbol": symbol,
                    "symbols": symbol_values,
                    "probe_key": record.get("probe", ""),
                    "probe": record.get("probe", ""),
                    "input_key": record.get("input_key", ""),
                    "status": record_status,
                    "reason": record.get("reason"),
                    "duplicate_of": record.get("duplicate_of"),
                    "commit": record.get("commit"),
                    "profile": record.get("profile", ""),
                    "toolchain": record.get("toolchain", ""),
                    "target_sha256": record.get("target_sha256", ""),
                    "candidate_sha256": record.get("candidate_sha256", ""),
                    "source": record_source,
                    "artifact": record.get("artifact", ""),
                }
            )
    except Exception:
        # Context history is deliberately best effort; malformed queue/ledgers
        # must never prevent a worker packet from being generated.
        canonical_records = []

    def history_identity(record: Mapping[str, Any]) -> tuple[str, ...]:
        owner_text = str(record.get("owner") or owner_id).replace("\\", "/").strip()
        owner_text = owner_text.split("#", 1)[0].strip("/")
        folded_owner = owner_text.casefold()
        if folded_owner.startswith("main:") or folded_owner.startswith("main/"):
            owner_text = owner_text[5:]
        if owner_text.casefold().startswith("src/"):
            owner_text = owner_text[4:]
        if owner_text.casefold().startswith("board/"):
            owner_text = owner_text[6:]
        if ":" in owner_text:
            owner_text = owner_text.split(":", 1)[0]
        if owner_text.casefold().endswith(".c"):
            owner_text = owner_text[:-2]
        owner = owner_text.strip("/").casefold()
        symbols = record.get("symbols")
        if isinstance(symbols, Sequence) and not isinstance(symbols, (str, bytes, bytearray)):
            symbol_values = sorted({str(item).casefold() for item in symbols if isinstance(item, str)})
        else:
            symbol_values = [str(record.get("symbol", "")).casefold()] if record.get("symbol") else []
        probe = str(record.get("probe_key") or record.get("probe") or "").casefold()
        input_key = str(record.get("input_key", "")).casefold()
        fallback = str(record.get("artifact") or record.get("source") or "").casefold()
        return (owner, ",".join(symbol_values), probe, input_key or fallback)

    def history_quality(record: Mapping[str, Any]) -> tuple[Any, ...]:
        source_name = str(record.get("source", "")).casefold()
        source_rank = (
            3 if source_name.startswith("queue:")
            else 2 if source_name.startswith("batch-history.json")
            else 1 if source_name.startswith("blocked.json")
            else 0
        )
        return (
            bool(record.get("input_key")),
            bool(record.get("status")),
            bool(record.get("artifact") or record.get("report")),
            source_rank,
            source_name,
            str(record.get("reason", "")).casefold(),
        )

    canonical_batch_probe_keys = {
        (
            str(record.get("owner") or owner_id).casefold(),
            str(record.get("symbol", "")).casefold(),
            str(record.get("probe_key") or record.get("probe") or "").casefold(),
        )
        for record in canonical_records
        if not record.get("input_key")
        and str(record.get("source", "")).startswith("batch-history.json")
        and (record.get("probe_key") or record.get("probe"))
    }
    by_identity: dict[tuple[str, ...], dict[str, Any]] = {}
    for source_records, legacy in ((canonical_records, False), (legacy_records, True)):
        for record in source_records:
            if legacy and not record.get("input_key"):
                legacy_key = (
                    str(record.get("owner") or owner_id).casefold(),
                    str(record.get("symbol", "")).casefold(),
                    str(record.get("probe_key") or record.get("probe") or "").casefold(),
                )
                if legacy_key in canonical_batch_probe_keys:
                    continue
            identity = history_identity(record)
            current = by_identity.get(identity)
            if current is None or history_quality(record) > history_quality(current):
                by_identity[identity] = record
    records = list(by_identity.values())
    records.sort(key=lambda item: (str(item.get("symbol", "")), str(item.get("probe_key", "")), str(item.get("source", ""))))
    status = "available" if records else legacy_status
    result = {
        "path": _repo_relative(root, path),
        "status": status,
        "records": records,
    }
    if warning and not records:
        result["warning"] = warning
    return result


def _repo_relative(root: Path, path: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return str(path.resolve())


def build_worker_packet(
    root: Path,
    unit: Mapping[str, Any],
    report: Mapping[str, Any],
    output: Path,
    strict_baseline: Path,
    value_baseline: Path,
) -> dict[str, Any]:
    """Build a concise, executor-ready contract for one Luna lane."""
    dispatch = worker_dispatch(report)
    wanted = set(dispatch["functions"])
    ranked_by_name = {
        str(item.get("function")): item
        for item in report.get("ranked_functions", [])
    }
    evidence = []
    for name in dispatch["functions"]:
        item = ranked_by_name.get(str(name))
        if item is None:
            continue
        evidence.append(
            {
                "function": item.get("function"),
                "target_rank": item.get("target_rank"),
                "target_bytes": item.get("target_bytes"),
                "category": item.get("category"),
                "strict_match_percent": item.get("strict_match_percent"),
                "strict_diff_rows": item.get("strict_diff_rows"),
                "diff_kind_shape": item.get("diff_kind_shape"),
                "target_calls": item.get("target_call_skeleton", []),
                "diagnostics": item.get("diagnostics", []),
                "function_forensics": item.get("function_forensics", {
                    "target_range": {
                        "start": 0,
                        "end": number(item.get("target_bytes")),
                        "size": number(item.get("target_bytes")),
                    },
                    "relocations": [],
                    "direct_rel24_calls": [],
                    "direct_calls": [],
                    "direct_call_edges": [],
                    "callback_edges": [],
                    "data_symbols": [],
                    "stack_clues": {
                        "prologue": [],
                        "frame_size": None,
                        "r1_offsets": [],
                        "registers": [],
                        "diagnostics": list(item.get("diagnostics", [])),
                    },
                }),
                "safe_actions": item.get("safe_actions", [])[:3],
            }
        )
    source = str(report["source"])
    unit_name = str(report["unit"])
    owner = "main:" + unit_name[len("main/"):] if unit_name.startswith("main/") else unit_name
    for item in evidence:
        item["internal_dependencies"] = [
            call for call in item["target_calls"] if call in wanted
        ]
    identity = {
        "unit": unit_name,
        "source": source,
        "target_object": str(unit.get("target_path", "")),
        "candidate_object": str(unit.get("base_path", "")),
        "target_sha256": file_hash(root / str(unit.get("target_path", ""))),
        "candidate_sha256": file_hash(root / str(unit.get("base_path", ""))),
        "source_sha256": file_hash(root / source),
        "functions": [
            {
                "function": item["function"],
                "target_rank": item["target_rank"],
                "target_bytes": item["target_bytes"],
                "target_calls": item["target_calls"],
                "function_forensics": item.get("function_forensics", {}),
            }
            for item in evidence
        ],
    }
    input_key = hashlib.sha256(
        json.dumps(identity, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()
    packet_id = input_key
    for item in evidence:
        item["probe"] = {
            "probe_key": (
                f"{dispatch['mode']}/{packet_id[:16]}/{item['function']}"
            ),
            "input_key": input_key,
            "history": PROBE_HISTORY.as_posix(),
        }
    verification = output / "worker-verification"
    report_command = [
        "rtk", "python", "tools/recovery_pass.py", unit_name,
        "--no-build", "--force",
        "--output-dir", _repo_relative(root, verification),
        "--baseline-strict", _repo_relative(root, strict_baseline),
        "--baseline-value", _repo_relative(root, value_baseline),
    ]
    knowledge_cards = list(report.get("selected_knowledge_cards", []))
    if (
        dispatch["mode"] == "evidence_preflight"
        and not any(card.get("id") == POOL_OWNERSHIP_CARD for card in knowledge_cards)
    ):
        knowledge_cards.extend(select_cards(root, [POOL_OWNERSHIP_CARD]))
    graph_context = report.get("graphify")
    if isinstance(graph_context, Mapping):
        graph_context = dict(graph_context)
        if graph_context.get("refresh_needed"):
            graph_context.pop("refresh_command", None)
            graph_context.pop("refresh_working_directory", None)
            graph_context["refresh_owner"] = "root-orchestrator-only"
    return {
        "schema_version": WORKER_PACKET_SCHEMA_VERSION,
        "kind": "same-owner-recovery",
        "packet_id": packet_id,
        "input_key": input_key,
        "owner": owner,
        "unit": unit_name,
        "source": source,
        "target_object": str(unit.get("target_path", "")),
        "candidate_object": str(unit.get("base_path", "")),
        "commit": report.get("commit"),
        "dispatch": dispatch,
        "baseline": dict(report.get("summary", {})),
        "function_evidence": evidence,
        "knowledge_cards": knowledge_cards,
        "probe_history": probe_history_summary(
            root,
            unit_name,
            dispatch["functions"],
            source=source,
        ),
        "graphify": graph_context,
        "budgets": {
            "preferred_functions": [PREFERRED_PACKET_MIN_FUNCTIONS, PREFERRED_PACKET_MAX_FUNCTIONS],
            "preferred_target_bytes": [PREFERRED_PACKET_MIN_BYTES, PREFERRED_PACKET_MAX_BYTES],
            "max_probes": dict(WORKER_PROBE_BUDGETS),
        },
        "commands": {
            "build": [
                "rtk", "python", "-m", "tools.serialized_build", "--root", ".",
                str(unit["base_path"]),
            ],
            "verify": report_command,
            "knowledge": [
                "rtk", "python", "tools/agent.py", "knowledge", "owner", unit_name,
                "--limit", "8",
            ],
            "organicity": [
                "rtk", "python", "tools/blind_recovery.py", "organicity", source,
                "--json",
            ],
            "context": [
                "rtk", "python", "tools/agent.py", "context", "owner", owner,
                "--budget", "12000", "--knowledge-limit", "5", "--local-evidence",
            ],
            "probe_lookups": [
                [
                    "rtk", "python", "tools/agent.py", "probe", "lookup",
                    "--owner", owner,
                    "--symbol", str(item["function"]),
                    "--probe-key", str(item["probe"]["probe_key"]),
                    "--input-key", input_key,
                    "--history", PROBE_HISTORY.as_posix(),
                    "--json",
                ]
                for item in evidence
            ],
        },
        "acceptance_gates": [
            "Use the machine-wide serialized build only; never launch a parallel retail build.",
            "Keep edits inside the assigned source and do not create branches, worktrees, commits, or pushes.",
            "Require zero strict-exact and data-value-exact regressions against this packet baseline.",
            "Retain a pass only when it gains strict-exact work or closes a necessary dependency of a gained exact caller.",
            "For literal, macro, include, or static-data edits, compare affected .sdata2/.rodata bytes and relocations; function text alone is not proof.",
            "Use natural target-backed C; no raw hexadecimal literals, scientific notation, invented names/types/padding, inline assembly, or match-only cast tricks.",
            "Run the repository organicity review on the complete source; resolve each finding with target/provenance evidence instead of treating the numeric score as authenticity proof.",
            "Record a rejected probe in the ledger and never repeat a listed probe without new target or provenance evidence.",
        ],
        "continuation": (
            "After each accepted build, rerun the pass report and take the next dependency-closed packet in this same source "
            "until 12-20 functions or 3072-6144 target bytes are measured, the owner closes, or the report says rotate_owner."
        ),
        "handoff": {
            "keep_one_owner_scope": True,
            "status_until_owner_closure": "coding",
            "report_retained_exact_functions_and_bytes": True,
            "report_rejected_probe_keys": True,
            "report_organicity_findings": True,
        },
    }


def render_worker_prompt(packet: Mapping[str, Any]) -> str:
    """Render the packet as a low-context handoff that Luna can execute directly."""
    dispatch = packet["dispatch"]
    lines = [
        f"# Luna recovery packet: `{packet['unit']}`",
        "",
        f"Packet ID: `{packet['packet_id']}`.",
        f"Work only in `{packet['source']}`. You share this worktree with other workers; preserve their edits.",
        f"Dispatch mode: **{dispatch['mode']}** - {dispatch['reason']}.",
    ]
    if dispatch["functions"]:
        lines += [
            f"Measured scope: **{dispatch['function_count']} functions / {dispatch['target_bytes']} target bytes**.",
            "Target-order functions: " + ", ".join(f"`{name}`" for name in dispatch["functions"]),
        ]
        if dispatch["mode"] == "evidence_preflight":
            lines.append(
                "Do not edit or build yet. Prove that literal pools, static data, headers, and semantics are closed over this packet; return implementation-ready or rotate_owner."
            )
    else:
        lines.append("Do not spend an implementation lane here; report `rotate_owner` immediately.")
    graph = packet.get("graphify")
    if isinstance(graph, Mapping) and not graph.get("fresh"):
        lines.append(
            "Graphify is stale for this source; use the target-call evidence in this packet and let root refresh the graph serially between builds."
        )
    lines += ["", "## Evidence", ""]
    for item in packet["function_evidence"]:
        calls = " -> ".join(item["target_calls"][:10]) or "none"
        diagnostics = ", ".join(item["diagnostics"]) or "none"
        lines.append(
            f"- `{item['function']}`: {item['target_bytes']}B, {item['category']}, "
            f"strict={item['strict_match_percent']}%, rows={item['strict_diff_rows']}; "
            f"calls: {calls}; diagnostics: {diagnostics}."
        )
    lines += ["", "## Knowledge and deduplication", ""]
    if packet["knowledge_cards"]:
        for card in packet["knowledge_cards"]:
            lines.append(
                f"- `{card['id']}` ({card.get('freshness', 'unknown')}): {card.get('rule') or card.get('title')}"
            )
            for counterexample in card.get("counterexamples", [])[:2]:
                lines.append(f"  - Do not repeat: {counterexample}")
    else:
        lines.append("- No card was selected; target evidence still outranks source-shape guesses.")
    records = packet["probe_history"]["records"]
    if records:
        for record in records:
            lines.append(
                f"- Prior probe `{record['symbol']}::{record['probe_key']}` = {record['status']}: {record['reason']}"
            )
    else:
        lines.append("- No owner-local probe-ledger entry intersects this packet.")
    lines += ["", "## Execute", ""]
    execute = (
        ("context", "knowledge")
        if dispatch["mode"] == "evidence_preflight"
        else ("context", "knowledge", "build", "verify", "organicity")
    )
    for label in execute:
        lines.append(
            f"- {label}: `{subprocess.list2cmdline(packet['commands'][label])}`"
        )
    if packet["commands"]["probe_lookups"]:
        lines.append("- Before a compiler probe, run its exact lookup command from `worker-packet.json`; `conflict` means stop or re-key before compiling.")
    lines += ["", "## Hard gates", ""]
    lines.extend(f"- {gate}" for gate in packet["acceptance_gates"])
    lines += ["", "## Continue", ""]
    lines.append(
        "Return the ownership/provenance closure result to root; do not convert this into an implementation task yourself."
        if dispatch["mode"] == "evidence_preflight"
        else packet["continuation"]
    )
    return "\n".join(lines).rstrip() + "\n"


def plan_shared_cause_clusters(ranked: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    """Plan only evidence-linked, repeatable recovery probes from ranked residuals."""
    grouped: dict[tuple[str, str, tuple[tuple[str, int], ...], str, str], list[Mapping[str, Any]]] = defaultdict(list)
    for item in ranked:
        if item.get("strict_exact") or item.get("quarantined_by_card"):
            continue
        key = cluster_feature_key(item)
        if key is not None:
            grouped[key].append(item)
    clusters: list[dict[str, Any]] = []
    for key, members in sorted(grouped.items()):
        if len(members) < 2:
            continue
        cause, category, diff_shape, relocation_pattern, delta_shape = key
        call_sets = [set(str(call) for call in item.get("target_call_skeleton", [])) for item in members]
        shared_calls = sorted(set.intersection(*call_sets)) if call_sets else []
        identifier_sets = [
            set(item.get("source_local_identifiers", {}).get("types", []))
            | set(item.get("source_local_identifiers", {}).get("work_identifiers", []))
            for item in members
        ]
        shared_identifiers = sorted(set.intersection(*identifier_sets)) if identifier_sets else []
        owner_sets = [
            {
                (
                    str(value.get("target_owner")),
                    str(value.get("source_owner")),
                    str(value.get("type")),
                )
                for value in item.get("relocation_owner_evidence", [])
            }
            for item in members
        ]
        shared_owners = sorted(set.intersection(*owner_sets)) if owner_sets else []
        owner_audit_only = cause == "relocation_identity_only" and not shared_owners
        if cause != "relocation_identity_only" and not shared_calls and not shared_identifiers:
            continue
        target_bytes = sum(number(item.get("target_bytes")) for item in members)
        threshold_met = len(members) >= 3 or target_bytes >= 1024
        actionable = threshold_met and not owner_audit_only
        linkage: dict[str, Any] = {}
        if cause == "relocation_identity_only" and shared_owners:
            linkage["relocation_owner_pairs"] = [
                {"target_owner": target, "source_owner": source, "type": kind}
                for target, source, kind in shared_owners
            ]
        elif shared_calls:
            linkage["target_calls"] = shared_calls
        if cause != "relocation_identity_only" and shared_identifiers:
            linkage["source_local_identifiers"] = shared_identifiers
        clusters.append(
            {
                "cluster_id": "",
                "cause": cause,
                "category": category,
                "functions": [str(item["function"]) for item in members],
                "function_count": len(members),
                "target_bytes": target_bytes,
                "diff_kind_shape": {kind: count for kind, count in diff_shape},
                "relocation_identity_pattern": relocation_pattern,
                "target_source_size_delta_shape": delta_shape,
                "shared_evidence": linkage,
                "concrete_shared_cause": not owner_audit_only,
                "owner_audit_only": owner_audit_only,
                "actionability_reason": (
                    "missing_shared_relocation_owner" if owner_audit_only
                    else "shared_relocation_owner" if cause == "relocation_identity_only"
                    else "shared_diagnostic_and_context"
                ),
                "actionable": actionable,
                "expected_compiler_probes": 0 if owner_audit_only else 1,
                "expected_exact_bytes": 0 if owner_audit_only else target_bytes,
                "expected_exact_bytes_per_compiler_probe": 0 if owner_audit_only else target_bytes,
            }
        )
    clusters.extend(repeated_target_call_clusters(ranked))
    clusters.extend(plan_ready_missing_definition_families(ranked))
    clusters.sort(
        key=lambda item: (
            not item.get("implementation_ready"),
            not item["actionable"],
            -number(item.get("implementation_target_bytes", item.get("expected_exact_bytes_per_compiler_probe"))),
            item["cause"],
            item["functions"],
        )
    )
    for index, cluster in enumerate(clusters, 1):
        cluster["cluster_id"] = f"shared-cause-{index:02d}"
    return clusters


def analyze(
    root: Path,
    unit: Mapping[str, Any],
    source: Path,
    strict: Mapping[str, Any],
    value: Mapping[str, Any],
    baseline_strict: Mapping[str, Any] | None,
    baseline_value: Mapping[str, Any] | None,
    donors: Sequence[Mapping[str, Any]],
    donor_provenance: Sequence[Mapping[str, Any]],
    donor_cache_hit: bool,
    graph_path: Path | None,
) -> dict[str, Any]:
    source_text = source.read_text(encoding="utf-8")
    parsed = parse_functions(source_text)
    source_order = [item.symbol for item in parsed]
    known_source = set(source_order)
    strict_functions = functions(strict["left"])
    compiled_by_name = {
        str(item["name"]): item
        for item in functions(strict.get("right", {}))
        if item.get("name")
    }
    value_by_name = {item["name"]: item for item in functions(value["left"])}
    target_order = [item["name"] for item in strict_functions]
    target_rank = {name: index for index, name in enumerate(target_order)}
    order = order_diagnostics(target_order, source_order)
    compiled_order = order_diagnostics(
        target_order, [item["name"] for item in functions(strict.get("right", {}))]
    )
    baseline_compiled_order = (
        order_diagnostics(
            target_order,
            [item["name"] for item in functions(baseline_strict.get("right", {}))],
        )
        if baseline_strict
        else None
    )
    macros = macro_index(root)
    literals = literal_report(root / str(unit["target_path"]), macros)
    source_uses_named_mem_domain = "HU_MEMNUM_OVL" in source_text
    quarantine_card, quarantined_stable_ids, quarantine_status, quarantine_reason = paired_single_quarantine(root)
    quarantine_reopen = None
    if quarantine_card:
        quarantine_reopen = next(
            (
                str(action)
                for action in quarantine_card.get("safe_actions", [])
                if "reopen" in str(action).lower()
            ),
            str(quarantine_card.get("rule", "")),
        )
    card_ids: set[str] = set()
    candidates: list[dict[str, Any]] = []
    refinements: list[dict[str, Any]] = []
    ranked: list[dict[str, Any]] = []
    missing_count = 0
    missing_bytes = 0
    object_missing_count = 0
    object_missing_bytes = 0
    source_pending_build_count = 0
    source_pending_build_bytes = 0
    compiled_unpaired_count = 0
    compiled_unpaired_bytes = 0
    strict_exact_count = 0
    value_exact_count = 0
    for target in strict_functions:
        name = str(target["name"])
        quarantined_by_card = (
            PAIRED_SINGLE_QUARANTINE_CARD if name in quarantined_stable_ids else None
        )
        size = number(target.get("size"))
        strict_exact = is_exact(target)
        value_exact = is_exact(value_by_name.get(name, {}))
        strict_exact_count += strict_exact
        value_exact_count += value_exact
        object_missing = target.get("target_symbol") is None
        source_definition_present = name in known_source
        compiled_unpaired = (
            object_missing
            and source_definition_present
            and name in compiled_by_name
        )
        source_pending_build = (
            object_missing
            and source_definition_present
            and not compiled_unpaired
        )
        missing_definition = object_missing and not source_definition_present
        object_missing_count += object_missing
        object_missing_bytes += size if object_missing else 0
        missing_count += missing_definition
        missing_bytes += size if missing_definition else 0
        source_pending_build_count += source_pending_build
        source_pending_build_bytes += size if source_pending_build else 0
        compiled_unpaired_count += compiled_unpaired
        compiled_unpaired_bytes += size if compiled_unpaired else 0
        calls = call_skeleton(strict["left"], target)
        body = next((function_text(source_text, item) for item in parsed if item.symbol == name), "")
        pairs = paired_changed(strict, target)
        paired = paired_symbol(strict, target)
        source_size = number(paired.get("size")) if paired is not None else None
        diagnostics: list[str] = []
        safe_actions: list[str] = []
        commutative = commutative_swap_kind(pairs)
        if commutative:
            diagnostics.append(commutative)
            if commutative == "floating_commutative_swap":
                card_ids.add("mwcc-commutative-fmuls-source-swap-canonicalizes-neutral")
                safe_actions.append("Run at most one direct operand-reversal control; then inspect producer lifetime or chronology.")
                baseline_symbol = next(
                    (
                        item
                        for item in functions(baseline_strict.get("left", {}))
                        if item.get("name") == name
                    ),
                    None,
                ) if baseline_strict else None
                unchanged = bool(
                    baseline_symbol
                    and commutative_swap_kind(paired_changed(baseline_strict, baseline_symbol))
                    == "floating_commutative_swap"
                    and paired_changed(baseline_strict, baseline_symbol) == pairs
                )
                refinements.append(
                    {
                        "card_id": "mwcc-commutative-fmuls-source-swap-canonicalizes-neutral",
                        "function": name,
                        "status": "byte_neutral_across_pass" if unchanged else "isolated_swap_residual",
                        "evidence": f"{len(pairs)} same-size fmuls operand swaps and no other instruction differences",
                    }
                )
            else:
                candidates.append(
                    {
                        "suggested_id": "mwcc-commutative-integer-add-source-swap",
                        "function": name,
                        "fingerprint": "exact-sized body with only commutative integer operand reversals",
                        "evidence": [{"target": left, "source": right} for _, left, right in pairs],
                        "action": "Distill only after one direct reversal control proves whether MWCC canonicalizes the spelling neutrally.",
                    }
                )
                safe_actions.append("Test one direct operand reversal; do not add a register-shaping local.")
        if branch_destination_only(pairs):
            diagnostics.append("branch_destination_only")
            nested = bool(re.search(r"\bdo\b", body) and re.search(r"\bfor\s*\(", body))
            candidates.append(
                {
                    "suggested_id": "gc26-o0-single-for-or-condition-backedge" if nested else "ppc-loop-backedge-source-shape",
                    "function": name,
                    "fingerprint": "same-size body whose only residual is an unconditional branch destination",
                    "nested_do_for_source": nested,
                    "evidence": [{"target": left, "source": right} for _, left, right in pairs],
                    "action": "If target has one init-to-condition edge and both true arms return to one body, test one for(init; inner || outer; increment) source shape.",
                }
            )
            safe_actions.append("Inspect the loop CFG and test one bounded natural source shape.")
        register_cycle = saved_register_cycle(pairs)
        stack_cycle = stack_slot_permutation(pairs)
        if register_cycle or stack_cycle:
            diagnostics.append("local_declaration_or_first_use_cycle")
            card_ids.add("mwcc-local-declaration-order-and-first-use-rotate-saved-registers")
            safe_actions.append("Test declaration order, initialization point, and first use as separate bounded variables.")
        uses_rand_macro = "MBCapsuleEffRandF" in body
        if uses_rand_macro and "mbRandMod" in calls:
            diagnostics.append(
                "authenticated_capsule_random_macro_consumer"
                if strict_exact
                else "capsule_random_macro_consumer_pending_exact_body"
            )
        huvec_locals = re.findall(r"\bHuVecF\s+([A-Za-z_]\w*)\s*(?:[;=\[])", body)
        if strict_exact and len(huvec_locals) >= 2:
            card_ids.add("mwcc-local-declaration-order-and-first-use-rotate-saved-registers")
            refinements.append(
                {
                    "card_id": "mwcc-local-declaration-order-and-first-use-rotate-saved-registers",
                    "function": name,
                    "status": "strict_exact_source_shape_candidate",
                    "evidence": "sequential HuVecF locals in exact body: " + ", ".join(huvec_locals[:6]),
                }
            )
        if strict_exact and re.search(r"\bfor\s*\([^;]*;[^;]*\|\|[^;]*;[^)]*\)", body, re.DOTALL):
            candidates.append(
                {
                    "suggested_id": "gc26-o0-single-for-or-condition-backedge",
                    "function": name,
                    "fingerprint": "strict-exact GC/2.6 O0 loop uses one for-condition with logical OR",
                    "evidence": "target/source branch CFG is exact under the single-for OR-condition spelling",
                    "action": "Distill as bounded control-flow evidence; do not generalize beyond matching init/condition/body/backedge topology.",
                }
            )
            card_ids.add("mp6-board-capsule-eff-rand-macro")
            refinements.append(
                {
                    "card_id": "mp6-board-capsule-eff-rand-macro",
                    "function": name,
                    "status": "strict_exact" if strict_exact else "pending_full_function_exactness",
                    "evidence": "source macro call corresponds to target mbRandMod relocation skeleton",
                }
            )
        if source_uses_named_mem_domain:
            card_ids.add("recovered-c-numeric-identifiers-require-named-domains")
        donor_matches = (
            rank_donors(name, calls, donors)
            if (
                missing_definition
                or (
                    not strict_exact
                    and not source_pending_build
                    and not compiled_unpaired
                )
            )
            else []
        )
        if donor_matches:
            card_ids.add("same-game-cross-owner-relocation-skeleton-harvesting")
        if strict_exact:
            category = "strict_exact"
            priority = 999
            quarantined_by_card = None
        elif quarantined_by_card:
            category = "knowledge_quarantined" if quarantine_status == "active" else "knowledge_stale_hold"
            priority = 998
            card_ids.add(quarantined_by_card)
            if quarantine_status == "active":
                safe_actions = [quarantine_reopen] if quarantine_reopen else []
            else:
                safe_actions = [
                    "Revalidate stale knowledge card before any reopen decision"
                    + (f": {quarantine_reason}" if quarantine_reason else ".")
                ]
        elif source_pending_build:
            category = "source_pending_build"
            priority = 45
            safe_actions = [
                "Rebuild the source object and rerun objdiff before scheduling implementation work."
            ]
        elif compiled_unpaired:
            category = "compiled_unpaired"
            priority = 46
            safe_actions = [
                "Inspect source csects/section mappings, then prove the linker-folded owner with a serialized linked-binary replay."
            ]
        elif missing_definition:
            category = "missing_definition"
            priority = 40 if donor_matches else 50
            safe_actions.append("Start from target calls/relocations and the highest-provenance sibling candidate; rebind every target-owned contract.")
        elif value_exact:
            category = "relocation_identity_only"
            priority = 70
            safe_actions.append("Treat the C body as value-exact; investigate literal or symbol ownership without changing exact instructions.")
        elif commutative:
            category = commutative
            priority = 10
        elif branch_destination_only(pairs):
            category = "branch_destination_only"
            priority = 15
        elif register_cycle or stack_cycle:
            category = "local_order_cycle"
            priority = 20
        else:
            category = "paired_residual"
            diff_count = len(changed_rows(target))
            priority = 25 if target.get("match_percent", 0) >= 99.0 and diff_count <= 12 else 60
        immediate_matches = immediate_macro_matches(target, macros, body)
        for macro_match in immediate_matches:
            for macro_name in macro_match.get("source_macro_matches", []):
                refinements.append(
                    {
                        "card_id": "recovered-c-numeric-identifiers-require-named-domains",
                        "function": name,
                        "status": "strict_exact" if strict_exact else "pending_full_function_exactness",
                        "evidence": (
                            f"target immediate {macro_match['decimal']} is spelled with existing "
                            f"source-domain macro {macro_name}"
                        ),
                    }
                )
        ranked.append(
            {
                "function": name,
                "target_rank": target_rank[name],
                "target_bytes": size,
                "category": category,
                "priority": priority,
                "strict_exact": strict_exact,
                "data_value_exact": value_exact,
                "object_missing": object_missing,
                "source_definition_present": source_definition_present,
                "source_pending_build": source_pending_build,
                "compiled_unpaired": compiled_unpaired,
                "strict_match_percent": target.get("match_percent"),
                "strict_diff_rows": len(changed_rows(target)),
                "diff_kinds": dict(Counter(str(row.get("diff_kind")) for row in changed_rows(target))),
                "diff_kind_shape": dict(Counter(str(row.get("diff_kind")) for row in changed_rows(target))),
                "source_bytes": source_size,
                "target_source_size_delta": source_size - size if source_size is not None else None,
                "relocation_identity_pattern": relocation_identity_pattern(
                    target, strict_exact=strict_exact, value_exact=value_exact
                ),
                "diagnostics": diagnostics,
                "function_forensics": function_forensics(strict.get("left", {}), target, diagnostics),
                "register_cycle": register_cycle,
                "stack_slot_cycle": stack_cycle,
                "target_call_skeleton": calls,
                "source_call_skeleton": source_calls(source_text, name, known_source) if name in known_source else [],
                "source_local_identifiers": source_local_identifiers(body),
                "relocation_owner_evidence": relocation_owner_evidence(strict, target),
                "immediate_macro_matches": immediate_matches,
                "donor_candidates": donor_matches,
                "safe_actions": safe_actions,
                "quarantined_by_card": quarantined_by_card,
                "quarantine_freshness": {
                    "effective_status": quarantine_status,
                    "reason": quarantine_reason,
                } if quarantined_by_card else None,
            }
        )
    if not order["order_matches"] or (
        baseline_compiled_order
        and baseline_compiled_order["inversions"] > compiled_order["inversions"]
    ):
        card_ids.add("gc26-o0-definition-order-and-call-evaluation")
    if baseline_compiled_order and baseline_compiled_order["inversions"] > compiled_order["inversions"]:
        refinements.append(
            {
                "card_id": "gc26-o0-definition-order-and-call-evaluation",
                "function": None,
                "status": "strict_gain_with_order_restoration",
                "evidence": (
                    f"compiled target-order inversions fell from {baseline_compiled_order['inversions']} "
                    f"to {compiled_order['inversions']} while the pass gained "
                    f"{len((delta(strict, baseline_strict) or {}).get('newly_exact', []))} strict functions"
                ),
            }
        )
    unique_refinements: dict[tuple[str, str | None], dict[str, Any]] = {}
    for item in refinements:
        unique_refinements[(item["card_id"], item.get("function"))] = item
    ranked.sort(key=lambda item: (item["priority"], item["strict_diff_rows"], item["target_bytes"], item["function"]))
    clusters = plan_shared_cause_clusters(ranked)
    preferred_packet = preferred_missing_definition_packet(ranked)
    if any(item.get("knowledge_card_id") == TARGET_CALL_CLUSTER_CARD for item in clusters):
        card_ids.add(TARGET_CALL_CLUSTER_CARD)
    head = git_value(root, "rev-parse", "HEAD")
    return {
        "schema_version": 1,
        "unit": unit["name"],
        "source": source.relative_to(root).as_posix(),
        "commit": head,
        "method": {
            "strict_exact": "paired target function, match_percent 100, and zero diff_kind rows",
            "data_value_exact": "same gate under functionRelocDiffs=data_value",
            "donors": "committed sibling source ranked by normalized symbol similarity and ordered call LCS; candidates remain hypotheses",
            "knowledge": "existing cards selected from observed fingerprints; candidate records are never written automatically",
            "shared_cause_clusters": "clusters require one concrete diagnostic plus shared target-call or local-identifier evidence; actionable clusters meet a three-function or 1024-byte threshold",
        },
        "summary": {
            "functions_total": len(strict_functions),
            "strict_exact": strict_exact_count,
            "data_value_exact": value_exact_count,
            "missing_definitions": missing_count,
            "missing_definition_bytes": missing_bytes,
            "object_missing": object_missing_count,
            "object_missing_bytes": object_missing_bytes,
            "source_pending_build": source_pending_build_count,
            "source_pending_build_bytes": source_pending_build_bytes,
            "compiled_unpaired": compiled_unpaired_count,
            "compiled_unpaired_bytes": compiled_unpaired_bytes,
            "source_definitions": len(source_order),
            "order_inversions": order["inversions"],
        },
        "strict_delta": delta(strict, baseline_strict),
        "data_value_delta": delta(value, baseline_value),
        "definition_order": order,
        "compiled_object_order": compiled_order,
        "baseline_compiled_object_order": baseline_compiled_order,
        "ranked_functions": ranked,
        "shared_cause_clusters": clusters,
        "preferred_implementation_packet": preferred_packet,
        "target_literals": literals,
        "selected_knowledge_cards": select_cards(root, card_ids),
        "knowledge_card_candidates": candidates,
        "knowledge_card_refinements": list(unique_refinements.values()),
        "donor_provenance": list(donor_provenance),
        "donor_cache_hit": donor_cache_hit,
        "graphify": graph_context(graph_path, source, source_order, head),
    }


def render_markdown(report: Mapping[str, Any]) -> str:
    summary = report["summary"]
    lines = [
        f"# Recovery pass: `{report['unit']}`",
        "",
        f"- Strict exact: **{summary['strict_exact']} / {summary['functions_total']}**",
        f"- Data-value exact: **{summary['data_value_exact']} / {summary['functions_total']}**",
        f"- Missing definitions: **{summary['missing_definitions']} functions / {summary['missing_definition_bytes']} target bytes**",
        f"- Source-present/object-missing (pending build): **{summary['source_pending_build']} functions / {summary['source_pending_build_bytes']} target bytes**",
        f"- Compiled but unpaired (section/csect mapping): **{summary.get('compiled_unpaired', 0)} functions / {summary.get('compiled_unpaired_bytes', 0)} target bytes**",
        f"- Target/source definition-order inversions: **{summary['order_inversions']}**",
    ]
    for label, key in (("Strict", "strict_delta"), ("Data-value", "data_value_delta")):
        value = report.get(key)
        if value:
            lines.append(f"- {label} gain: **{len(value['newly_exact'])} functions / {value['newly_exact_bytes']} bytes**; regressions: **{len(value['regressed_exact'])}**")
    packet = report.get("preferred_implementation_packet")
    lines += ["", "## Preferred implementation packet", ""]
    if not packet:
        lines.append("- No missing-definition packet is available.")
    else:
        members = ", ".join(f"`{name}`" for name in packet["functions"])
        readiness = "ready" if packet["ready"] else "below preferred pass bounds"
        lines.append(
            f"- **{packet['function_count']} functions / {packet['target_bytes']} target bytes**; "
            f"dependency-closed; {readiness}."
        )
        lines.append(f"  - Members in target order: {members}")
    lines += ["", "## Shared-cause clusters", ""]
    clusters = list(report.get("shared_cause_clusters", []))
    implementation_families = [item for item in clusters if item.get("implementation_ready")]
    actionable_clusters = [item for item in clusters if item.get("actionable") and not item.get("implementation_ready")]
    owner_audits = [item for item in clusters if item.get("owner_audit_only")]
    if not actionable_clusters and not implementation_families:
        lines.append("- No actionable shared-cause clusters.")
    if owner_audits:
        lines.append(
            f"- Owner-audit only: {len(owner_audits)} cluster(s) / "
            f"{sum(number(item.get('target_bytes')) for item in owner_audits)} target bytes; "
            "no compiler probe is recommended without a shared relocation owner."
        )
    for item in implementation_families:
        members = ", ".join(f"`{name}`" for name in item["functions"])
        evidence = item.get("shared_evidence", {})
        edges = evidence.get("internal_target_call_edges", [])
        edge_text = ", ".join(f"`{edge['caller']}` â†’ `{edge['callee']}`" for edge in edges)
        lines.append(
            f"- `{item['cluster_id']}` - ready implementation family; {item['function_count']} functions / "
            f"{item['implementation_target_bytes']} target bytes (potential exact bytes, not a compiler-probe yield)"
        )
        lines.append(f"  - Members: {members}")
        if edge_text:
            lines.append(f"  - Closed internal calls: {edge_text}")
        elif evidence.get("ordered_nonruntime_target_calls"):
            calls = " -> ".join(f"`{name}`" for name in evidence["ordered_nonruntime_target_calls"])
            lines.append(f"  - Shared target call contract: {calls}")
    for item in actionable_clusters:
        members = ", ".join(f"`{name}`" for name in item["functions"])
        evidence = item.get("shared_evidence", {})
        links: list[str] = []
        if evidence.get("target_calls"):
            links.append("calls " + ", ".join(f"`{name}`" for name in evidence["target_calls"][:6]))
        if evidence.get("source_local_identifiers"):
            links.append("locals " + ", ".join(f"`{name}`" for name in evidence["source_local_identifiers"][:6]))
        if evidence.get("ordered_target_call_skeleton"):
            links.append(
                "ordered calls "
                + " → ".join(f"`{name}`" for name in evidence["ordered_target_call_skeleton"][:8])
            )
        linkage = "; ".join(links)
        lines.append(
            f"- `{item['cluster_id']}` - {item['cause']}; {item['function_count']} functions / "
            f"{item['target_bytes']} target bytes; score **{item['expected_exact_bytes_per_compiler_probe']} bytes/probe**"
        )
        lines.append(f"  - Members: {members}")
        lines.append(f"  - Shared evidence: {linkage}")
    lines += ["", "## Ranked recovery actions", ""]
    actionable = [item for item in report["ranked_functions"] if not item["strict_exact"]]
    if not actionable:
        lines.append("- No non-exact target functions.")
    for index, item in enumerate(actionable[:30], 1):
        lines.append(f"{index}. `{item['function']}` — {item['category']}; {item['target_bytes']} bytes; strict rows {item['strict_diff_rows']}")
        if item["diagnostics"]:
            lines.append("   - Diagnostics: " + ", ".join(item["diagnostics"]))
        if item["target_call_skeleton"]:
            lines.append("   - Target calls: " + " → ".join(f"`{name}`" for name in item["target_call_skeleton"][:16]))
        if item["donor_candidates"]:
            donor = item["donor_candidates"][0]
            lines.append(f"   - Donor lead: `{donor['symbol']}` in `{donor['source']}:{donor['line']}` at `{str(donor['commit'])[:12]}` (score {donor['score']}, provenance={donor['provenance_authenticated']})")
        if item["safe_actions"]:
            lines.append("   - Next: " + item["safe_actions"][0])
    lines += ["", "## Definition chronology", ""]
    order = report["definition_order"]
    lines.append(f"- Common functions: {order['common_functions']}; inversions: {order['inversions']}; exact sequence: {order['order_matches']}")
    compiled = report["compiled_object_order"]
    baseline_compiled = report.get("baseline_compiled_object_order")
    lines.append(f"- Current compiled-object inversions: {compiled['inversions']}")
    if baseline_compiled:
        lines.append(f"- Baseline compiled-object inversions: {baseline_compiled['inversions']}")
    for item in order["displaced"][:12]:
        lines.append(f"- `{item['function']}` target rank {item['target_rank']}, C rank {item['source_rank']} (delta {item['delta']:+d})")
    lines += ["", "## Selected knowledge", ""]
    if not report["selected_knowledge_cards"]:
        lines.append("- No cards selected.")
    for card in report["selected_knowledge_cards"]:
        lines.append(f"- `{card['id']}` — {card.get('freshness', 'missing')}: {card.get('title', 'missing card')}")
    lines += ["", "## Candidate distillation (review only)", ""]
    if not report["knowledge_card_candidates"]:
        lines.append("- No new candidate fingerprint; existing cards cover the detected diagnostics.")
    for item in report["knowledge_card_candidates"]:
        lines.append(f"- `{item['suggested_id']}` from `{item['function']}` — {item['fingerprint']}")
    lines += ["", "## Existing-card refinements", ""]
    if not report["knowledge_card_refinements"]:
        lines.append("- No new owner/example evidence for an existing card.")
    for item in report["knowledge_card_refinements"]:
        target = f" from `{item['function']}`" if item.get("function") else ""
        lines.append(f"- `{item['card_id']}`{target} — {item['status']}: {item['evidence']}")
    graph = report.get("graphify")
    if graph:
        lines += ["", "## Graphify context", "", f"- Graph fresh at current commit: `{graph['fresh']}` (built `{graph.get('built_at_commit')}`)"]
        if graph.get("stale_reasons"):
            lines.append("- Stale because: " + "; ".join(graph["stale_reasons"]))
        if graph.get("missing_functions"):
            lines.append("- Missing current functions: " + ", ".join(f"`{name}`" for name in graph["missing_functions"][:20]))
        if graph.get("refresh_needed"):
            lines.append("- Refresh is root-orchestrator-only and must not create `graphify-out` beneath a source directory.")
        for item in graph["cross_file_call_neighbors"][:8]:
            lines.append(f"- `{item['source']}`: {item['edges']} call edges")
    lines += ["", "## Literal decoding", ""]
    if not report["target_literals"]:
        lines.append("- No sized float/double symbols decoded from target `.sdata2` or `.rodata`.")
    for item in report["target_literals"][:40]:
        macros = ", ".join(f"`{name}`" for name in item["macro_candidates"]) or "no authenticated simple macro match"
        lines.append(f"- `{item['symbol']}` ({item['kind']}) = **{item['decimal']}**; {macros}")
    return "\n".join(lines).rstrip() + "\n"


def objdiff(
    executable: Path,
    root: Path,
    unit: Mapping[str, Any],
    output: Path,
    *,
    data_value: bool,
) -> None:
    command = [
        str(executable),
        "diff",
        "-1",
        str(root / str(unit["target_path"])),
        "-2",
        str(root / str(unit["base_path"])),
        "-o", str(output), "--format", "json-pretty",
    ]
    if data_value:
        command += ["-c", "functionRelocDiffs=data_value"]
    result = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
    if result.returncode:
        raise ValueError(result.stderr.strip() or f"objdiff failed with {result.returncode}")


def infer_source(root: Path, unit: Mapping[str, Any]) -> Path:
    base = Path(str(unit["base_path"]))
    parts = list(base.parts)
    try:
        start = parts.index("src")
    except ValueError as exc:
        raise ValueError("cannot infer source path; pass --source") from exc
    relative = Path(*parts[start:]).with_suffix(".c")
    return root / relative


def _add_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("unit", help="objdiff unit name, for example main/board/capmove")
    parser.add_argument("--source")
    parser.add_argument("--output-dir")
    parser.add_argument("--strict-report")
    parser.add_argument("--value-report")
    parser.add_argument("--baseline-strict")
    parser.add_argument("--baseline-value")
    parser.add_argument("--objdiff", default="build/tools/objdiff-cli.exe")
    build_group = parser.add_mutually_exclusive_group()
    build_group.add_argument("--build", action="store_true", help="run exactly one worktree-local ninja object build before reports")
    build_group.add_argument(
        "--no-build",
        action="store_true",
        help="explicit report-only mode; skip mutation of this worktree's build tree",
    )
    parser.add_argument(
        "--build-lock",
        default=os.environ.get(BUILD_LOCK_ENV),
        help=(
            "same-worktree build lock path (default: <root>/build/.compiler-lane.lock; "
            f"override with {BUILD_LOCK_ENV})"
        ),
    )
    parser.add_argument(
        "--build-lock-timeout",
        type=float,
        default=55.0,
        help="seconds to wait for this worktree's build lock (default: 55)",
    )
    parser.add_argument("--donor-root", action="append", default=[])
    parser.add_argument("--graph", default="build/board-autonomy/graphify-board/graph.json")
    parser.add_argument("--force", action="store_true", help="ignore an unchanged-input report cache")
    parser.add_argument("--json", action="store_true", help="print the concise summary as JSON")


def add_recovery_pass_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "pass-report",
        help="generate strict/value recovery triage, gains, chronology, donors, and card diagnostics",
    )
    _add_arguments(parser)
    return parser


def run_recovery_pass(args: argparse.Namespace, *, root: Path) -> int:
    try:
        root = root.resolve()
        config = read_json(root / "objdiff.json")
        matches = [item for item in config.get("units", []) if item.get("name") == args.unit]
        if len(matches) != 1:
            raise ValueError(f"unit {args.unit!r} resolved to {len(matches)} objdiff entries")
        unit = matches[0]
        source = (root / args.source).resolve() if args.source else infer_source(root, unit)
        if not source.is_file():
            raise ValueError(f"source does not exist: {source}")
        output_dir = args.output_dir or f"build/recovery-pass/{re.sub(r'[^A-Za-z0-9_.-]+', '_', args.unit)}"
        output = (root / output_dir).resolve()
        output.mkdir(parents=True, exist_ok=True)
        if args.build:
            lock_path = (
                Path(args.build_lock).expanduser().resolve()
                if args.build_lock
                else (root / DEFAULT_BUILD_LOCK).resolve()
            )
            with serialized_build_lock(lock_path, args.build_lock_timeout):
                result = subprocess.run(["ninja", "-j1", str(unit["base_path"])], cwd=root, text=True, capture_output=True, check=False)
            if result.returncode:
                raise ValueError(result.stderr.strip() or result.stdout.strip() or "serialized object build failed")
        strict_path = (root / args.strict_report).resolve() if args.strict_report else output / "strict.json"
        value_path = (root / args.value_report).resolve() if args.value_report else output / "data-value.json"
        baseline_strict_path = (root / args.baseline_strict).resolve() if args.baseline_strict else None
        baseline_value_path = (root / args.baseline_value).resolve() if args.baseline_value else None
        donor_roots = [(root / path).resolve() if not Path(path).is_absolute() else Path(path).resolve() for path in args.donor_root]
        graph_path = (root / args.graph).resolve() if args.graph else None
        objdiff_path = (root / args.objdiff).resolve()
        cache = recovery_cache_key(
            root, unit, source,
            strict_path if args.strict_report else None,
            value_path if args.value_report else None,
            baseline_strict_path, baseline_value_path, graph_path, objdiff_path, donor_roots,
        )
        report_path = output / "report.json"
        markdown_path = output / "report.md"
        cached_report: Mapping[str, Any] | None = None
        if report_path.is_file():
            try:
                candidate = read_json(report_path)
                if isinstance(candidate, Mapping):
                    cached_report = candidate
            except ValueError:
                pass
        report_cache_hit = bool(
            not args.force
            and cached_report
            and cached_report.get("input_cache", {}).get("key") == cache["key"]
        )
        if report_cache_hit:
            report = dict(cached_report or {})
            if not markdown_path.is_file():
                atomic_text(markdown_path, render_markdown(report))
        else:
            previous_inputs = (cached_report or {}).get("input_cache", {}).get("inputs", {})
            objects_unchanged = previous_inputs.get("objects") == cache["inputs"]["objects"]
            objdiff_unchanged = previous_inputs.get("objdiff") == cache["inputs"]["objdiff"]
            reuse_generated = not args.force and objects_unchanged and objdiff_unchanged
            if not args.strict_report and (not reuse_generated or not strict_path.is_file()):
                objdiff(objdiff_path, root, unit, strict_path, data_value=False)
            if not args.value_report and (not reuse_generated or not value_path.is_file()):
                objdiff(objdiff_path, root, unit, value_path, data_value=True)
            strict = read_json(strict_path)
            value = read_json(value_path)
            baseline_strict = read_json(baseline_strict_path) if baseline_strict_path else None
            baseline_value = read_json(baseline_value_path) if baseline_value_path else None
            donor_cache = root / "build" / "board-autonomy" / "recovery-cache" / "donors.json"
            donors, donor_provenance, donor_cache_hit = donor_catalog(donor_roots, donor_cache if donor_roots else None)
            report = analyze(
                root, unit, source, strict, value, baseline_strict, baseline_value,
                donors, donor_provenance, donor_cache_hit, graph_path,
            )
            report["input_cache"] = cache
            atomic_json(report_path, report)
            atomic_text(markdown_path, render_markdown(report))
        summary = report["summary"]
        preferred_packet = report.get("preferred_implementation_packet")
        worker_packet = build_worker_packet(
            root, unit, report, output, strict_path, value_path,
        )
        worker_packet_path = output / "worker-packet.json"
        worker_prompt_path = output / "worker-prompt.md"
        atomic_json(worker_packet_path, worker_packet)
        atomic_text(worker_prompt_path, render_worker_prompt(worker_packet))
        concise = {
            "unit": args.unit,
            "strict_exact": f"{summary['strict_exact']}/{summary['functions_total']}",
            "data_value_exact": f"{summary['data_value_exact']}/{summary['functions_total']}",
            "missing_definitions": summary["missing_definitions"],
            "missing_definition_bytes": summary["missing_definition_bytes"],
            "source_pending_build": summary["source_pending_build"],
            "source_pending_build_bytes": summary["source_pending_build_bytes"],
            "compiled_unpaired": summary.get("compiled_unpaired", 0),
            "compiled_unpaired_bytes": summary.get("compiled_unpaired_bytes", 0),
            "preferred_packet": (
                {
                    "ready": preferred_packet["ready"],
                    "functions": preferred_packet["function_count"],
                    "target_bytes": preferred_packet["target_bytes"],
                    "members": preferred_packet["functions"],
                }
                if preferred_packet else None
            ),
            "order_inversions": summary["order_inversions"],
            "strict_delta": report.get("strict_delta"),
            "worker_dispatch": worker_packet["dispatch"],
            "report_cache_hit": report_cache_hit,
            "report_json": str(report_path),
            "report_markdown": str(markdown_path),
            "worker_packet_json": str(worker_packet_path),
            "worker_prompt_markdown": str(worker_prompt_path),
        }
        if args.json:
            print(json.dumps(concise, indent=2))
        else:
            print(
                f"{args.unit}: strict={concise['strict_exact']} "
                f"value={concise['data_value_exact']} "
                f"missing={summary['missing_definitions']}/{summary['missing_definition_bytes']}B "
                f"pending_build={summary['source_pending_build']}/{summary['source_pending_build_bytes']}B "
                f"compiled_unpaired={summary.get('compiled_unpaired', 0)}/{summary.get('compiled_unpaired_bytes', 0)}B "
                f"order_inversions={summary['order_inversions']}"
            )
            print(f"report cache: {'hit' if report_cache_hit else 'miss'}")
            print(f"wrote {report_path}")
            print(f"wrote {markdown_path}")
            print(
                f"worker dispatch: {worker_packet['dispatch']['mode']} "
                f"({worker_packet['dispatch']['function_count']} functions / "
                f"{worker_packet['dispatch']['target_bytes']}B)"
            )
            print(f"wrote {worker_packet_path}")
            print(f"wrote {worker_prompt_path}")
        return 0
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=str(DEFAULT_ROOT))
    _add_arguments(parser)
    args = parser.parse_args()
    return run_recovery_pass(args, root=Path(args.root))


if __name__ == "__main__":
    raise SystemExit(main())
