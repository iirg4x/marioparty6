#!/usr/bin/env python3
"""Bounded, relocation-aware inventory for one PowerPC ELF object.

This module is deliberately a read-only structural diagnostic.  It does not
run a compiler, invoke readelf, or make a retain/promotion decision.  The
existing evidence-bundle parser remains the single ELF-header/symbol-table
reader; this module adds the section flags/alignment and all-section
relocation views needed by object-level comparison.
"""

from __future__ import annotations

import hashlib
import argparse
import json
import struct
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.crack_evidence_bundle import (  # noqa: E402
    EvidenceError,
    _parse_elf_relocations,
    _parse_elf_structure,
)


SHT_PROGBITS = 1
SHT_RELA = 4
SHT_REL = 9
SHT_NOBITS = 8
SHF_ALLOC = 0x2
SHF_EXECINSTR = 0x4
SHN_UNDEF = 0
SHN_ABS = 0xFFF1
SHN_COMMON = 0xFFF2
STT_SECTION = 3
STT_FILE = 4
STT_FUNC = 2

_RELA_ENTSIZE = 12
_SECTION_ENTSIZE = 40
_SYMBOL_ENTSIZE = 16
_MAX_CLOSED_DETAILS = 64
_DEBUG_SECTION_PREFIXES = (".debug", ".zdebug")
_DEBUG_SECTIONS = {".line", ".comment", ".note", ".gnu.attributes"}
SCHEMA = "recovery_object_inventory/v1"


class ObjectInventoryError(EvidenceError):
    """Object structure or comparison input cannot be interpreted safely."""


def _canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def _sha(value: Any) -> str:
    if isinstance(value, bytes):
        return hashlib.sha256(value).hexdigest()
    return hashlib.sha256(_canonical(value)).hexdigest()


def _u32(data: bytes, offset: int, path: Path, label: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ObjectInventoryError(f"{path}: truncated {label}")
    return struct.unpack_from(">I", data, offset)[0]


def _i32(data: bytes, offset: int, path: Path, label: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise ObjectInventoryError(f"{path}: truncated {label}")
    return struct.unpack_from(">i", data, offset)[0]


def _section_name(sections: Sequence[Mapping[str, Any]], index: int, path: Path) -> str:
    if index == SHN_UNDEF:
        return "UNDEF"
    if index == SHN_ABS:
        return "ABS"
    if index == SHN_COMMON:
        raise ObjectInventoryError(f"{path}: unsupported SHN_COMMON symbol")
    if index < 0 or index >= len(sections):
        raise ObjectInventoryError(f"{path}: symbol section index {index} is invalid")
    return str(sections[index]["name"])


def _section_views(parsed: Mapping[str, Any], path: Path) -> list[dict[str, Any]]:
    """Add flags/alignment and validate every section payload once."""

    data = parsed.get("data")
    sections = parsed.get("sections")
    if not isinstance(data, bytes) or not isinstance(sections, list):
        raise ObjectInventoryError(f"{path}: parser returned an invalid ELF structure")
    if len(data) < 52:
        raise ObjectInventoryError(f"{path}: truncated ELF header")
    shoff = _u32(data, 32, path, "section table offset")
    shentsize = struct.unpack_from(">H", data, 46)[0]
    shnum = struct.unpack_from(">H", data, 48)[0]
    if shentsize != _SECTION_ENTSIZE or shnum != len(sections):
        raise ObjectInventoryError(f"{path}: section table shape changed after parser validation")
    if shoff + shnum * _SECTION_ENTSIZE > len(data):
        raise ObjectInventoryError(f"{path}: section table extends past object")

    views: list[dict[str, Any]] = []
    for index, section in enumerate(sections):
        entry = shoff + index * _SECTION_ENTSIZE
        flags = _u32(data, entry + 8, path, "section flags")
        align = _u32(data, entry + 32, path, "section alignment")
        section_type = int(section["type"])
        offset = int(section["offset"])
        size = int(section["size"])
        if offset < 0 or size < 0:
            raise ObjectInventoryError(f"{path}: section {section.get('name', index)!r} has negative extent")
        if section_type == SHT_NOBITS:
            content = b""
        else:
            if offset + size > len(data):
                raise ObjectInventoryError(f"{path}: section {section.get('name', index)!r} is truncated")
            content = data[offset:offset + size]
        views.append({
            "index": index,
            "name": str(section["name"]),
            "type": section_type,
            "flags": flags,
            "offset": offset,
            "size": size,
            "align": align,
            "link": int(section["link"]),
            "info": int(section["info"]),
            "entsize": int(section["entsize"]),
            "content": content,
        })
    return views


def _symbol_descriptor(symbol: Mapping[str, Any], sections: Sequence[Mapping[str, Any]], path: Path) -> dict[str, Any]:
    info = int(symbol["info"])
    section_index = int(symbol["section"])
    return {
        "name": str(symbol["name"]),
        "binding": info >> 4,
        "type": info & 0xF,
        "section": _section_name(sections, section_index, path),
        "value": int(symbol["value"]),
        "size": int(symbol["size"]),
    }


def _semantic_symbol(symbol: Mapping[str, Any], sections: Sequence[Mapping[str, Any]], path: Path) -> bool:
    info = int(symbol["info"])
    symbol_type = info & 0xF
    if symbol_type == STT_FILE:
        return False
    section_index = int(symbol["section"])
    if section_index in {SHN_UNDEF, SHN_ABS}:
        return True
    section_name = _section_name(sections, section_index, path)
    section = sections[section_index]
    if section_name.startswith(_DEBUG_SECTION_PREFIXES) or section_name in _DEBUG_SECTIONS:
        return False
    # Anonymous section markers carry no identity beyond the section descriptor.
    if symbol_type == STT_SECTION and not str(symbol["name"]):
        return False
    return bool(int(section["flags"]) & SHF_ALLOC)


def _function_metadata(parsed: Mapping[str, Any], sections: Sequence[Mapping[str, Any]], path: Path) -> list[dict[str, Any]]:
    symbols = parsed.get("symbols")
    if not isinstance(symbols, list):
        raise ObjectInventoryError(f"{path}: parser returned no symbol list")
    functions: list[dict[str, Any]] = []
    by_name: dict[str, dict[str, Any]] = {}
    for symbol in symbols:
        if int(symbol["info"]) & 0xF != STT_FUNC or not str(symbol["name"]):
            continue
        section_index = int(symbol["section"])
        if section_index in {SHN_UNDEF, SHN_ABS}:
            continue
        if section_index < 0 or section_index >= len(sections):
            raise ObjectInventoryError(f"{path}: function {symbol['name']!r} has invalid section")
        section = sections[section_index]
        start = int(symbol["value"])
        size = int(symbol["size"])
        if size <= 0:
            # Zero-sized aliases do not define a byte range and cannot safely
            # participate in .text target normalization.
            continue
        if section["type"] == SHT_NOBITS or not int(section["flags"]) & SHF_EXECINSTR:
            raise ObjectInventoryError(f"{path}: function {symbol['name']!r} is not in executable data")
        if start < 0 or start + size > int(section["size"]):
            raise ObjectInventoryError(f"{path}: function {symbol['name']!r} exceeds section extent")
        name = str(symbol["name"])
        if name in by_name:
            raise ObjectInventoryError(f"{path}: duplicate sized function {name!r}")
        row = {
            "name": name,
            "binding": int(symbol["info"]) >> 4,
            "section_index": section_index,
            "section": str(section["name"]),
            "start": start,
            "size": size,
        }
        by_name[name] = row
        functions.append(row)

    by_section: dict[int, list[dict[str, Any]]] = {}
    for row in functions:
        by_section.setdefault(int(row["section_index"]), []).append(row)
    for section_index, rows in by_section.items():
        rows.sort(key=lambda item: (int(item["start"]), int(item["size"]), str(item["name"])))
        for previous, current in zip(rows, rows[1:]):
            previous_end = int(previous["start"]) + int(previous["size"])
            if previous_end > int(current["start"]):
                raise ObjectInventoryError(
                    f"{path}: overlapping function ranges in section {sections[section_index]['name']!r}"
                )
    return sorted(functions, key=lambda item: str(item["name"]))


def _normalize_effective_target(
    effective: Mapping[str, Any],
    functions_by_section: Mapping[str, Sequence[Mapping[str, Any]]],
    path: Path,
) -> dict[str, Any]:
    if effective.get("kind") != "section" or effective.get("section") != ".text":
        return dict(effective)
    try:
        target_offset = int(effective["offset"])
    except (KeyError, TypeError, ValueError) as exc:
        raise ObjectInventoryError(f"{path}: malformed .text relocation target") from exc
    matches = [
        row for row in functions_by_section.get(".text", ())
        if int(row["start"]) <= target_offset < int(row["start"]) + int(row["size"])
    ]
    if len(matches) > 1:
        raise ObjectInventoryError(f"{path}: ambiguous .text function mapping at offset {target_offset}")
    if len(matches) == 1:
        row = matches[0]
        return {
            "kind": "function",
            "name": str(row["name"]),
            "offset": target_offset - int(row["start"]),
        }
    # A section-local label outside a sized function is retained structurally;
    # guessing a function owner would be worse than leaving it unnormalized.
    return dict(effective)


def _effective_target(symbol: Mapping[str, Any], sections: Sequence[Mapping[str, Any]], addend: int, path: Path) -> dict[str, Any]:
    section_index = int(symbol["section"])
    if section_index == SHN_UNDEF:
        return {"kind": "undefined", "name": str(symbol["name"]), "addend": addend}
    if section_index == SHN_ABS:
        return {"kind": "absolute", "name": str(symbol["name"]), "value": int(symbol["value"]) + addend}
    if section_index == SHN_COMMON:
        raise ObjectInventoryError(f"{path}: relocation uses unsupported SHN_COMMON symbol")
    if section_index < 0 or section_index >= len(sections):
        raise ObjectInventoryError(f"{path}: relocation symbol section index {section_index} is invalid")
    return {
        "kind": "section",
        "section": str(sections[section_index]["name"]),
        "offset": int(symbol["value"]) + addend,
    }


def _all_relocations(
    parsed: Mapping[str, Any],
    sections: Sequence[Mapping[str, Any]],
    functions_by_section: Mapping[str, Sequence[Mapping[str, Any]]],
    path: Path,
) -> dict[tuple[int, int, int], dict[str, Any]]:
    data = parsed["data"]
    symbols = parsed["symbols"]
    sym_index = int(parsed["sym_index"])
    rows: dict[tuple[int, int, int], dict[str, Any]] = {}
    target_rela_count: dict[int, int] = {}
    for relsec in sections:
        rel_type = int(relsec["type"])
        if rel_type == SHT_REL:
            raise ObjectInventoryError(f"{path}: SHT_REL relocation section is unsupported")
        if rel_type != SHT_RELA:
            continue
        target_index = int(relsec["info"])
        if target_index <= 0 or target_index >= len(sections):
            raise ObjectInventoryError(f"{path}: relocation section {relsec['name']!r} has invalid target")
        if int(relsec["link"]) != sym_index:
            raise ObjectInventoryError(f"{path}: relocation section {relsec['name']!r} has invalid symbol link")
        if int(relsec["entsize"]) != _RELA_ENTSIZE or int(relsec["size"]) % _RELA_ENTSIZE:
            raise ObjectInventoryError(f"{path}: relocation section {relsec['name']!r} has unsupported entry size")
        if int(relsec["offset"]) + int(relsec["size"]) > len(data):
            raise ObjectInventoryError(f"{path}: relocation section {relsec['name']!r} is truncated")
        target_rela_count[target_index] = target_rela_count.get(target_index, 0) + 1
        if target_rela_count[target_index] > 1:
            raise ObjectInventoryError(f"{path}: multiple relocation sections target {sections[target_index]['name']!r}")
        target = sections[target_index]
        for record_offset in range(int(relsec["offset"]), int(relsec["offset"]) + int(relsec["size"]), _RELA_ENTSIZE):
            rel_offset = _u32(data, record_offset, path, "relocation offset")
            if rel_offset >= int(target["size"]):
                raise ObjectInventoryError(f"{path}: relocation offset escapes section {target['name']!r}")
            info = _u32(data, record_offset + 4, path, "relocation info")
            symbol_index, relocation_kind = info >> 8, info & 0xFF
            if symbol_index >= len(symbols):
                raise ObjectInventoryError(f"{path}: relocation symbol index {symbol_index} is invalid")
            addend = _i32(data, record_offset + 8, path, "relocation addend")
            symbol = symbols[symbol_index]
            raw_effective = _effective_target(symbol, sections, addend, path)
            normalized = _normalize_effective_target(raw_effective, functions_by_section, path)
            identity = (target_index, int(rel_offset), int(relocation_kind))
            if identity in rows:
                raise ObjectInventoryError(
                    f"{path}: duplicate relocation identity {target['name']}+0x{rel_offset:x}/type{relocation_kind}"
                )
            rows[identity] = {
                "target_section": str(target["name"]),
                "offset": int(rel_offset),
                "type": int(relocation_kind),
                "symbol": _symbol_descriptor(symbol, sections, path),
                "addend": int(addend),
                "effective_target": raw_effective,
                "normalized_effective_target": normalized,
            }
    return rows


def _allocated_sections(sections: Sequence[Mapping[str, Any]]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for section in sections:
        if not int(section["flags"]) & SHF_ALLOC:
            continue
        name = str(section["name"])
        if name in result:
            raise ObjectInventoryError(f"duplicate allocated section name {name!r}")
        result[name] = {
            "type": int(section["type"]),
            "flags": int(section["flags"]),
            "size": int(section["size"]),
            "align": int(section["align"]),
            "content_sha256": _sha(bytes(section["content"])),
        }
    return {name: result[name] for name in sorted(result)}


def _function_physical_rows(
    path: Path,
    parsed: Mapping[str, Any],
    function: Mapping[str, Any],
    all_rows: Mapping[tuple[int, int, int], Mapping[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    """Use the existing focus parser, then join its rows to the all-section view."""

    helper = _parse_elf_relocations(path, str(function["name"]), structure=parsed)
    if helper.get("section") != function["section"] or int(helper.get("size", -1)) != int(function["size"]):
        raise ObjectInventoryError(f"{path}: focus relocation parser disagrees about {function['name']!r}")
    normalized_rows: list[dict[str, Any]] = []
    physical_rows: list[dict[str, Any]] = []
    section_index = int(function["section_index"])
    for helper_row in helper.get("physical_relocations", []):
        offset = int(helper_row["offset"])
        relocation_kind = int(helper_row["type"])
        identity = (section_index, int(function["start"]) + offset, relocation_kind)
        raw = all_rows.get(identity)
        if raw is None:
            raise ObjectInventoryError(f"{path}: focus relocation row is absent from object relocation index")
        if raw["effective_target"] != helper_row["effective_target"]:
            raise ObjectInventoryError(f"{path}: focus relocation target parser disagreement")
        normalized_rows.append({
            "offset": offset,
            "type": relocation_kind,
            "effective_target": dict(raw["normalized_effective_target"]),
        })
        physical_rows.append({
            "offset": offset,
            "type": relocation_kind,
            "symbol": dict(raw["symbol"]),
            "addend": int(raw["addend"]),
            # Physical identity follows the canonical raw effective target.
            # Symbol spelling/value are retained as attribution data, but an
            # equivalent local alias must not manufacture a regression.
            "effective_target": dict(raw["effective_target"]),
            "normalized_effective_target": dict(raw["normalized_effective_target"]),
        })
    normalized_rows.sort(key=lambda row: (row["offset"], row["type"], _canonical(row["effective_target"])))
    physical_rows.sort(key=lambda row: (row["offset"], row["type"], _canonical(row)))
    return normalized_rows, physical_rows


def inventory(path: Path) -> dict[str, Any]:
    """Return a deterministic structural inventory for one ELF object.

    The parser is intentionally invoked once for the object.  Focus relocation
    calls receive that parsed structure, so they do not reread or reparse the
    object.  Unsupported REL relocations, malformed extents, and ambiguous
    function ranges/targets raise :class:`ObjectInventoryError`.
    """

    path = Path(path)
    try:
        parsed = _parse_elf_structure(path)
    except ObjectInventoryError:
        raise
    except (EvidenceError, OSError, struct.error, TypeError, ValueError) as exc:
        raise ObjectInventoryError(f"{path}: ELF parse failed: {exc}") from exc
    try:
        sections = _section_views(parsed, path)
        functions = _function_metadata(parsed, sections, path)
        by_section: dict[str, list[dict[str, Any]]] = {}
        for function in functions:
            by_section.setdefault(str(function["section"]), []).append(function)
        all_rows = _all_relocations(parsed, sections, by_section, path)
        allocated = _allocated_sections(sections)
        data = parsed["data"]
        function_output: dict[str, dict[str, Any]] = {}
        for function in functions:
            section = sections[int(function["section_index"])]
            start = int(function["start"])
            size = int(function["size"])
            raw = bytes(section["content"])[start:start + size]
            if len(raw) != size:
                raise ObjectInventoryError(f"{path}: function {function['name']!r} raw extent is truncated")
            normalized_rows, physical_rows = _function_physical_rows(path, parsed, function, all_rows)
            function_output[str(function["name"])] = {
                "size": size,
                "section": str(function["section"]),
                "offset": start,
                "binding": int(function["binding"]),
                "raw_sha256": _sha(raw),
                "relocations": normalized_rows,
                "relocations_sha256": _sha(normalized_rows),
                "physical_relocations": physical_rows,
                "physical_sha256": _sha([
                    {key: row[key] for key in ("offset", "type", "effective_target")}
                    for row in physical_rows
                ]),
            }

        semantic_symbols = [
            _symbol_descriptor(symbol, sections, path)
            for symbol in parsed["symbols"]
            if _semantic_symbol(symbol, sections, path)
        ]
        semantic_symbols.sort(key=_canonical)
        semantic_relocations = [
            {
                "section": row["target_section"],
                "offset": row["offset"],
                "type": row["type"],
                "symbol": row["symbol"],
                "addend": row["addend"],
                "effective_target": row["effective_target"],
            }
            for (target_index, _, _), row in all_rows.items()
            if int(sections[target_index]["flags"]) & SHF_ALLOC
        ]
        semantic_relocations.sort(key=_canonical)
        semantic_payload = {
            "allocated_sections": allocated,
            "symbols": semantic_symbols,
            "relocations": semantic_relocations,
        }
        return {
            "schema": SCHEMA,
            "object": dict(parsed["object"]),
            "functions": {name: function_output[name] for name in sorted(function_output)},
            "allocated_sections": allocated,
            "allocated_relocations": semantic_relocations,
            "semantic_sha256": _sha(semantic_payload),
        }
    except ObjectInventoryError:
        raise
    except (EvidenceError, OSError, struct.error, TypeError, ValueError, KeyError) as exc:
        raise ObjectInventoryError(f"{path}: inventory failed: {exc}") from exc


def _inventory_map(value: Mapping[str, Any], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ObjectInventoryError(f"{label}: inventory is not an object")
    functions = value.get("functions")
    sections = value.get("allocated_sections")
    if not isinstance(functions, Mapping) or not isinstance(sections, Mapping):
        raise ObjectInventoryError(f"{label}: inventory lacks functions/allocated_sections")
    if not isinstance(value.get("semantic_sha256"), str):
        raise ObjectInventoryError(f"{label}: inventory lacks semantic_sha256")
    if not isinstance(value.get("allocated_relocations"), list):
        raise ObjectInventoryError(f"{label}: inventory lacks allocated relocation semantics")
    for name, row in functions.items():
        if not isinstance(name, str) or not isinstance(row, Mapping):
            raise ObjectInventoryError(f"{label}: malformed function inventory row")
        for field in ("binding", "size", "section", "offset", "raw_sha256", "relocations", "physical_relocations"):
            if field not in row:
                raise ObjectInventoryError(f"{label}: function {name!r} lacks {field}")
        if not isinstance(row["relocations"], list) or not isinstance(row["physical_relocations"], list):
            raise ObjectInventoryError(f"{label}: function {name!r} has malformed relocation lists")
        for field in ("binding", "size", "offset"):
            try:
                int(row[field])
            except (TypeError, ValueError) as exc:
                raise ObjectInventoryError(f"{label}: function {name!r} has malformed {field}") from exc
    for name, row in sections.items():
        if not isinstance(name, str) or not isinstance(row, Mapping):
            raise ObjectInventoryError(f"{label}: malformed allocated section row")
        for field in ("type", "flags", "size", "align", "content_sha256"):
            if field not in row:
                raise ObjectInventoryError(f"{label}: allocated section {name!r} lacks {field}")
        for field in ("type", "flags", "size", "align"):
            try:
                int(row[field])
            except (TypeError, ValueError) as exc:
                raise ObjectInventoryError(f"{label}: allocated section {name!r} has malformed {field}") from exc
    return value


def _function_census(inventory_value: Mapping[str, Any]) -> dict[str, tuple[Any, Any, Any]]:
    functions = inventory_value["functions"]
    return {
        str(name): (row.get("binding"),)
        for name, row in functions.items()
        if isinstance(row, Mapping)
    }


def _function_layout(inventory_value: Mapping[str, Any]) -> dict[str, tuple[Any, Any, Any]]:
    functions = inventory_value["functions"]
    return {
        str(name): (row.get("section"), row.get("offset"), row.get("size"))
        for name, row in functions.items()
        if isinstance(row, Mapping)
    }


def _relocation_summary(row: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if row is None:
        return None
    relocations = row.get("relocations")
    if not isinstance(relocations, list):
        raise ObjectInventoryError("function inventory has malformed normalized relocations")
    return {"count": len(relocations), "sha256": _sha(relocations)}


def _physical_map(
    row: Mapping[str, Any] | None,
    label: str,
    *,
    normalized: bool = False,
) -> dict[tuple[int, int], dict[str, Any]]:
    if row is None:
        return {}
    field = "relocations" if normalized else "physical_relocations"
    physical = row.get(field)
    if not isinstance(physical, list):
        raise ObjectInventoryError(f"{label}: function inventory lacks {field}")
    result: dict[tuple[int, int], dict[str, Any]] = {}
    for item in physical:
        if not isinstance(item, Mapping):
            raise ObjectInventoryError(f"{label}: malformed physical relocation")
        try:
            key = (int(item["offset"]), int(item["type"]))
        except (KeyError, TypeError, ValueError) as exc:
            raise ObjectInventoryError(f"{label}: malformed physical relocation identity") from exc
        if key in result:
            raise ObjectInventoryError(f"{label}: duplicate physical relocation identity {key}")
        if "effective_target" not in item or not isinstance(item["effective_target"], Mapping):
            raise ObjectInventoryError(f"{label}: physical relocation lacks effective target")
        result[key] = {
            "offset": key[0],
            "type": key[1],
            "effective_target": dict(item["effective_target"]),
        }
    return result


def _physical_diff(left: Mapping[tuple[int, int], Any], right: Mapping[tuple[int, int], Any]) -> int:
    return sum(left.get(key) != right.get(key) for key in set(left) | set(right))


def _closed_losses(
    target: Mapping[tuple[int, int], Any],
    base: Mapping[tuple[int, int], Any],
    candidate: Mapping[tuple[int, int], Any],
) -> tuple[list[dict[str, Any]], int]:
    losses: list[dict[str, Any]] = []
    for key in sorted(set(target) & set(base)):
        if base[key] != target[key] or candidate.get(key) == target[key]:
            continue
        detail: dict[str, Any] = {"offset": key[0], "type": key[1]}
        if key not in candidate:
            detail["candidate"] = "missing"
        else:
            detail["candidate"] = "changed"
        losses.append(detail)
    return losses[:_MAX_CLOSED_DETAILS], len(losses)


def _nontext_sections(value: Mapping[str, Any]) -> Mapping[str, Any]:
    sections = value["allocated_sections"]
    return {
        str(name): dict(row)
        for name, row in sections.items()
        if isinstance(row, Mapping) and not int(row.get("flags", 0)) & SHF_EXECINSTR
    }


def _nontext_relocations(value: Mapping[str, Any]) -> list[dict[str, Any]]:
    rows = value.get("allocated_relocations")
    if not isinstance(rows, list):
        raise ObjectInventoryError("inventory lacks allocated relocation semantics")
    executable = {
        str(name)
        for name, section in value["allocated_sections"].items()
        if isinstance(section, Mapping) and int(section.get("flags", 0)) & SHF_EXECINSTR
    }
    result: list[dict[str, Any]] = []
    for row in rows:
        if not isinstance(row, Mapping) or not isinstance(row.get("section"), str):
            raise ObjectInventoryError("inventory has malformed allocated relocation semantics")
        if row["section"] not in executable:
            result.append(dict(row))
    result.sort(key=_canonical)
    return result


def compare(
    target: Mapping[str, Any],
    base: Mapping[str, Any],
    candidate: Mapping[str, Any],
    focus: Sequence[str],
) -> dict[str, Any]:
    """Compare three inventories without making an authority decision.

    ``functions`` contains every function so sibling regressions remain visible;
    ``focus`` is echoed and validated for callers that need a narrowed report.
    Relocation lists are represented by count/hash summaries here.  Physical
    row-loss details are keyed by exact ``(offset, type)`` identities and capped
    to keep the result bounded.
    """

    target = _inventory_map(target, "target")
    base = _inventory_map(base, "base")
    candidate = _inventory_map(candidate, "candidate")
    if isinstance(focus, (str, bytes)):
        raise ObjectInventoryError("focus must be a sequence of function names")
    focus_names = [str(name) for name in focus]
    if len(set(focus_names)) != len(focus_names):
        raise ObjectInventoryError("focus contains duplicate function names")
    for name in focus_names:
        if name not in target["functions"] or name not in base["functions"] or name not in candidate["functions"]:
            raise ObjectInventoryError(f"focus function {name!r} is absent from one inventory")

    census_target = _function_census(target)
    census_base = _function_census(base)
    census_candidate = _function_census(candidate)
    layout_target = _function_layout(target)
    layout_base = _function_layout(base)
    layout_candidate = _function_layout(candidate)
    names = sorted(set(census_target) | set(census_base) | set(census_candidate))
    rows: dict[str, dict[str, Any]] = {}
    for name in names:
        target_row = target["functions"].get(name)
        base_row = base["functions"].get(name)
        candidate_row = candidate["functions"].get(name)
        target_physical = _physical_map(target_row, f"target:{name}")
        base_physical = _physical_map(base_row, f"base:{name}")
        candidate_physical = _physical_map(candidate_row, f"candidate:{name}")
        losses, loss_count = _closed_losses(target_physical, base_physical, candidate_physical)
        target_normalized = _physical_map(target_row, f"target:{name}", normalized=True)
        base_normalized = _physical_map(base_row, f"base:{name}", normalized=True)
        candidate_normalized = _physical_map(candidate_row, f"candidate:{name}", normalized=True)
        normalized_losses, normalized_loss_count = _closed_losses(
            target_normalized, base_normalized, candidate_normalized
        )
        rows[name] = {
            "target_size": target_row.get("size") if isinstance(target_row, Mapping) else None,
            "base_size": base_row.get("size") if isinstance(base_row, Mapping) else None,
            "candidate_size": candidate_row.get("size") if isinstance(candidate_row, Mapping) else None,
            "raw_equal_base": bool(base_row and candidate_row and base_row.get("raw_sha256") == candidate_row.get("raw_sha256")),
            "raw_exact_target": bool(target_row and candidate_row and candidate_row.get("raw_sha256") == target_row.get("raw_sha256")),
            "base_relocations": _relocation_summary(base_row),
            "candidate_relocations": _relocation_summary(candidate_row),
            "target_relocations": _relocation_summary(target_row),
            "base_normalized_exact": bool(target_row and base_row and target_row.get("relocations_sha256") == base_row.get("relocations_sha256")),
            "candidate_normalized_exact": bool(target_row and candidate_row and target_row.get("relocations_sha256") == candidate_row.get("relocations_sha256")),
            "base_raw_exact_target": bool(target_row and base_row and base_row.get("raw_sha256") == target_row.get("raw_sha256")),
            "base_physical_exact": bool(target_row and base_row and base_physical == target_physical),
            "candidate_physical_exact": bool(target_row and candidate_row and candidate_physical == target_physical),
            "closed_physical_row_losses": losses,
            "closed_physical_row_loss_count": loss_count,
            "physical_diff_before": _physical_diff(target_physical, base_physical),
            "physical_diff_after": _physical_diff(target_physical, candidate_physical),
            "closed_normalized_row_losses": normalized_losses,
            "closed_normalized_row_loss_count": normalized_loss_count,
            "normalized_diff_before": _physical_diff(target_normalized, base_normalized),
            "normalized_diff_after": _physical_diff(target_normalized, candidate_normalized),
        }
    base_nontext = _nontext_sections(base)
    candidate_nontext = _nontext_sections(candidate)
    nontext_changes = sorted(
        name for name in set(base_nontext) | set(candidate_nontext)
        if base_nontext.get(name) != candidate_nontext.get(name)
    )
    nontext_relocations_changed = _nontext_relocations(base) != _nontext_relocations(candidate)
    if nontext_relocations_changed:
        nontext_changes.append("<allocated-relocations>")
    return {
        "authority_advanced": False,
        "focus": focus_names,
        "function_census_equal": census_base == census_candidate,
        "function_census": {
            "target_count": len(census_target),
            "base_count": len(census_base),
            "candidate_count": len(census_candidate),
            "target_base_equal": census_target == census_base,
            "target_candidate_equal": census_target == census_candidate,
        },
        "function_layout_equal": layout_base == layout_candidate,
        "functions": rows,
        "allocated_nontext_changed": bool(nontext_changes),
        "allocated_nontext_relocations_changed": nontext_relocations_changed,
        "allocated_nontext_change_count": len(nontext_changes),
        "allocated_nontext_changes": nontext_changes[:_MAX_CLOSED_DETAILS],
        "semantic_object_equal_baseline": base["semantic_sha256"] == candidate["semantic_sha256"],
        "semantic_object_equal_target": target["semantic_sha256"] == candidate["semantic_sha256"],
        "semantic_equal": {
            "target_base": target["semantic_sha256"] == base["semantic_sha256"],
            "target_candidate": target["semantic_sha256"] == candidate["semantic_sha256"],
            "base_candidate": base["semantic_sha256"] == candidate["semantic_sha256"],
        },
    }


__all__ = ["ObjectInventoryError", "compare", "inventory"]


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("object", type=Path, help="big-endian PowerPC ELF object")
    args = parser.parse_args(argv)
    try:
        print(json.dumps(inventory(args.object), sort_keys=True, separators=(",", ":")))
    except ObjectInventoryError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
