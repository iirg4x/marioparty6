#!/usr/bin/env python3
"""Decode typed literal-pool ownership mismatches from objdiff JSON.

Objdiff relocation symbol indices are local to each object.  Comparing the
indices or labels directly therefore confuses three materially different
cases: different literal bits, a different relocation contract, and identical
bits owned by a differently named/ordered pool symbol.  This read-only tool
resolves both sides, decodes the literal using the consumer instruction, and
groups the paired rows by causal mismatch class.

The output is diagnostic evidence only.  It never authenticates a source name,
advances match authority, or recommends inventing a label merely to steer the
compiler pool.
"""

from __future__ import annotations

import argparse
import base64
import binascii
import hashlib
import json
import math
import struct
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Mapping, Sequence


SCHEMA = "typed_pool_owner_decoder/v1"
DEFAULT_GROUP_LIMIT = 24
DEFAULT_ROW_LIMIT = 12
MAX_LITERAL_BYTES = 64


class PoolDecodeError(ValueError):
    """Malformed or unsupported objdiff pool input."""


def _int(value: Any, default: int | None = None) -> int | None:
    if isinstance(value, bool):
        return default
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


def _sequence(value: Any) -> Sequence[Any]:
    if isinstance(value, Sequence) and not isinstance(value, (str, bytes, bytearray)):
        return value
    return ()


def _symbols(side: Mapping[str, Any]) -> list[Mapping[str, Any]]:
    value = side.get("symbols")
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes, bytearray)):
        raise PoolDecodeError("objdiff side symbols must be an array")
    if any(not isinstance(item, Mapping) for item in value):
        raise PoolDecodeError("objdiff side symbols contains a non-object entry")
    return list(value)


def _symbol_name(symbol: Mapping[str, Any] | None, index: int | None) -> str:
    if symbol is not None and isinstance(symbol.get("name"), str) and symbol["name"]:
        return str(symbol["name"])
    return f"symbol[{index}]" if index is not None else "<unknown>"


def _owner_class(name: str) -> str:
    if name.startswith("@"):
        return "compiler_anonymous"
    if name.startswith("gap_"):
        return "named_gap"
    if name.startswith("lbl_"):
        return "named_label"
    if name.startswith("symbol[") or name == "<unknown>":
        return "unresolved"
    return "named_object"


def _decode_data(symbol: Mapping[str, Any] | None) -> bytes | None:
    if symbol is None:
        return None
    size = _int(symbol.get("size"))
    if size is not None and (size < 0 or size > MAX_LITERAL_BYTES):
        return None
    chunks: list[tuple[int, bytes]] = []
    cursor = 0
    for entry in _sequence(symbol.get("data_diff")):
        if not isinstance(entry, Mapping) or entry.get("kind") == "DIFF_DELETE":
            continue
        encoded = entry.get("data")
        if not isinstance(encoded, str):
            continue
        try:
            raw = base64.b64decode(encoded, validate=False)
        except (binascii.Error, TypeError, ValueError):
            continue
        offset = _int(entry.get("offset", entry.get("address")), cursor)
        if offset is None or offset < 0:
            continue
        chunks.append((offset, raw))
        cursor = offset + len(raw)
    if not chunks:
        return None
    chunks.sort(key=lambda item: item[0])
    base = chunks[0][0]
    end = max(offset + len(raw) for offset, raw in chunks)
    if end - base > MAX_LITERAL_BYTES:
        return None
    merged = bytearray(end - base)
    present = bytearray(end - base)
    for offset, raw in chunks:
        start = offset - base
        for index, byte in enumerate(raw):
            slot = start + index
            if present[slot] and merged[slot] != byte:
                raise PoolDecodeError("conflicting literal bytes in data_diff")
            merged[slot] = byte
            present[slot] = 1
    if not all(present):
        return None
    if size is not None:
        if size < 0 or len(merged) < size:
            return None
        merged = merged[:size]
    return bytes(merged) or None


def _finite(value: float) -> float | str:
    if math.isnan(value):
        return "nan"
    if math.isinf(value):
        return "+inf" if value > 0 else "-inf"
    return value


def _typed_interpretations(raw: bytes | None) -> dict[str, Any] | None:
    if raw is None:
        return None
    result: dict[str, Any] = {"width_bytes": len(raw), "hex": raw.hex()}
    if len(raw) == 2:
        result.update(
            {
                "u16": int.from_bytes(raw, "big", signed=False),
                "s16": int.from_bytes(raw, "big", signed=True),
            }
        )
    elif len(raw) == 4:
        result.update(
            {
                "u32": int.from_bytes(raw, "big", signed=False),
                "s32": int.from_bytes(raw, "big", signed=True),
                "f32": _finite(struct.unpack(">f", raw)[0]),
            }
        )
    elif len(raw) == 8:
        words = [int.from_bytes(raw[index:index + 4], "big") for index in (0, 4)]
        result.update(
            {
                "u64": int.from_bytes(raw, "big", signed=False),
                "s64": int.from_bytes(raw, "big", signed=True),
                "f64": _finite(struct.unpack(">d", raw)[0]),
                "u32_words": words,
            }
        )
        if words == [0x43300000, 0x80000000]:
            result["mwcc_role"] = "signed-int-to-double-bias"
        elif words == [0x43300000, 0x00000000]:
            result["mwcc_role"] = "unsigned-int-to-double-bias"
    return result


def _opcode(formatted: str) -> str:
    return formatted.strip().split(None, 1)[0].lower() if formatted.strip() else ""


def _consumer_type(formatted: str, raw: bytes | None) -> str:
    opcode = _opcode(formatted)
    if opcode in {"lfs", "stfs"}:
        return "f32"
    if opcode in {"lfd", "stfd"}:
        return "f64"
    if opcode in {"lha", "sth"}:
        return "s16"
    if opcode in {"lhz"}:
        return "u16"
    if opcode in {"lbz", "stb"}:
        return "u8"
    if opcode in {"lwz", "stw"}:
        return "u32-or-address"
    if raw is not None:
        return f"bits{len(raw) * 8}"
    return "unknown"


def _instruction(row: Mapping[str, Any]) -> Mapping[str, Any] | None:
    value = row.get("instruction")
    return value if isinstance(value, Mapping) else None


def _relocation(row: Mapping[str, Any]) -> Mapping[str, Any] | None:
    instruction = _instruction(row)
    if instruction is None:
        return None
    value = instruction.get("relocation")
    return value if isinstance(value, Mapping) else None


def _formatted(row: Mapping[str, Any]) -> str:
    instruction = _instruction(row)
    value = instruction.get("formatted") if instruction is not None else None
    return value if isinstance(value, str) else ""


def _section_membership(symbols: Sequence[Mapping[str, Any]]) -> dict[int, str | None]:
    membership: dict[int, str | None] = {}
    section: str | None = None
    for index, symbol in enumerate(symbols):
        kind = str(symbol.get("kind", ""))
        name = str(symbol.get("name", ""))
        if kind == "SYMBOL_SECTION":
            section = name.strip("[]") or None
            membership[index] = section
            continue
        membership[index] = section
        if kind not in {"SYMBOL_OBJECT", "SYMBOL_SECTION"}:
            section = None
    return membership


def _owner_record(
    side: Mapping[str, Any],
    target_index: Any,
    *,
    formatted: str,
) -> dict[str, Any]:
    symbols = _symbols(side)
    index = _int(target_index)
    symbol = symbols[index] if index is not None and 0 <= index < len(symbols) else None
    name = _symbol_name(symbol, index)
    raw = _decode_data(symbol)
    membership = _section_membership(symbols)
    return {
        "symbol_index": index,
        "name": name,
        "owner_class": _owner_class(name),
        "kind": symbol.get("kind") if symbol is not None else None,
        "section": membership.get(index) if index is not None else None,
        "address": _int(symbol.get("address")) if symbol is not None else None,
        "size_bytes": _int(symbol.get("size")) if symbol is not None else None,
        "bytes": raw.hex() if raw is not None else None,
        "typed": _typed_interpretations(raw),
        "consumer_type": _consumer_type(formatted, raw),
    }


def _function_pair(
    report: Mapping[str, Any], name: str
) -> tuple[Mapping[str, Any], Mapping[str, Any], Mapping[str, Any], Mapping[str, Any]]:
    left = report.get("left")
    right = report.get("right")
    if not isinstance(left, Mapping) or not isinstance(right, Mapping):
        raise PoolDecodeError("objdiff report must contain mapping-valued left and right sides")
    left_symbols = _symbols(left)
    right_symbols = _symbols(right)
    candidates = [item for item in right_symbols if item.get("name") == name]
    if not candidates:
        raise PoolDecodeError(f"candidate function not found: {name}")
    if len(candidates) != 1:
        raise PoolDecodeError(f"candidate function identity is ambiguous: {name}")
    candidate = candidates[0]
    target_index = _int(candidate.get("target_symbol"))
    if target_index is None or not 0 <= target_index < len(left_symbols):
        raise PoolDecodeError(f"candidate function has no valid target pairing: {name}")
    target = left_symbols[target_index]
    if target.get("kind") != "SYMBOL_FUNCTION" or candidate.get("kind") != "SYMBOL_FUNCTION":
        raise PoolDecodeError(f"paired symbol is not a function: {name}")
    if target.get("name") != name:
        raise PoolDecodeError(f"target/candidate function identity mismatch: {name}")
    return left, right, target, candidate


def _pool_rows(side: Mapping[str, Any], function: Mapping[str, Any]) -> dict[int, dict[str, Any]]:
    rows: dict[int, dict[str, Any]] = {}
    for row_index, row in enumerate(_sequence(function.get("instructions"))):
        if not isinstance(row, Mapping):
            continue
        relocation = _relocation(row)
        if relocation is None:
            continue
        formatted = _formatted(row)
        type_name = str(relocation.get("type_name", "unknown"))
        owner = _owner_record(side, relocation.get("target_symbol"), formatted=formatted)
        name = str(owner["name"])
        section = str(owner.get("section") or "").lower()
        typed = owner.get("typed") if isinstance(owner.get("typed"), Mapping) else {}
        typed_width = typed.get("width_bytes")
        literal_name = name.startswith(("lbl_", "@", "gap_"))
        is_pool = (
            (
                owner.get("bytes") is not None
                and ("@sda" in formatted.lower() or "sdata2" in section or literal_name)
            )
            or (
                literal_name
                and "@sda" in formatted.lower()
                and (typed_width in {2, 4, 8} or owner.get("bytes") is None)
            )
        )
        if not is_pool:
            continue
        instruction = _instruction(row) or {}
        rows[row_index] = {
            "row": row_index,
            "instruction_address": _int(instruction.get("address")),
            "instruction": formatted,
            "diff_kind": row.get("diff_kind", instruction.get("diff_kind")),
            "relocation": {
                "type": type_name,
                "addend": _int(relocation.get("addend"), 0),
            },
            "owner": owner,
        }
    return rows


def _classification(target: Mapping[str, Any] | None, candidate: Mapping[str, Any] | None) -> tuple[str, list[str], str, str]:
    if target is None:
        return (
            "candidate_only_pool_consumer",
            ["consumer_presence"],
            "missing_or_extra_pool_consumer",
            "Inspect the earliest source/CFG or expression-shape divergence that created the extra candidate consumer.",
        )
    if candidate is None:
        return (
            "target_only_pool_consumer",
            ["consumer_presence"],
            "missing_or_extra_pool_consumer",
            "Inspect the earliest source/CFG or expression-shape divergence that omitted the target consumer.",
        )
    t_reloc = target["relocation"]
    c_reloc = candidate["relocation"]
    t_owner = target["owner"]
    c_owner = candidate["owner"]
    differences: list[str] = []
    if t_reloc.get("type") != c_reloc.get("type"):
        differences.append("relocation_type")
    if t_reloc.get("addend") != c_reloc.get("addend"):
        differences.append("relocation_addend")
    if t_owner.get("size_bytes") != c_owner.get("size_bytes"):
        differences.append("literal_width")
    if t_owner.get("bytes") is None or c_owner.get("bytes") is None:
        differences.append("literal_bytes_unresolved")
    elif t_owner.get("bytes") != c_owner.get("bytes"):
        differences.append("literal_value")
    if t_owner.get("consumer_type") != c_owner.get("consumer_type"):
        differences.append("consumer_type")
    if t_owner.get("name") != c_owner.get("name"):
        differences.append("owner_name")
    if t_owner.get("section") != c_owner.get("section"):
        differences.append("owner_section")
    if t_owner.get("address") != c_owner.get("address"):
        differences.append("owner_offset")
    if not differences:
        return (
            "exact_pool_contract",
            [],
            "exact",
            "No pool source axis is indicated by this consumer.",
        )
    observed_diff = bool(target.get("diff_kind") or candidate.get("diff_kind"))
    exact_external_contract_differences = {
        "literal_width",
        "literal_bytes_unresolved",
        "owner_section",
        "owner_offset",
    }
    if (
        not observed_diff
        and t_reloc.get("type") == c_reloc.get("type")
        and t_reloc.get("addend") == c_reloc.get("addend")
        and t_owner.get("name") == c_owner.get("name")
        and t_owner.get("bytes") is not None
        and c_owner.get("bytes") is None
        and t_owner.get("consumer_type") == c_owner.get("consumer_type")
        and t_owner.get("kind") == "SYMBOL_OBJECT"
        and c_owner.get("kind") is None
        and set(differences).issubset(exact_external_contract_differences)
    ):
        return (
            "mapped_pool_contract",
            differences,
            "exact_relocation_mapping_with_external_owner_contract",
            "The candidate references the exact named external pool owner; no source axis is indicated by absent local bytes.",
        )
    if not observed_diff and set(differences).issubset(
        {"owner_name", "owner_section", "owner_offset"}
    ):
        return (
            "mapped_pool_contract",
            differences,
            "exact_relocation_mapping_with_object_local_owner_identity",
            "Objdiff mapped this owner transition exactly; no source axis is indicated.",
        )
    if "relocation_type" in differences:
        return (
            "relocation_type_mismatch",
            differences,
            "abi_or_storage_class_mismatch",
            "Inspect declaration linkage, small-data placement, prototype, and expression shape; do not relabel the pool.",
        )
    if "relocation_addend" in differences:
        return (
            "relocation_addend_mismatch",
            differences,
            "subobject_or_index_mismatch",
            "Inspect field, subobject, array-index, or folded-address source shape.",
        )
    if "literal_bytes_unresolved" in differences:
        return (
            "unresolved_pool_bytes",
            differences,
            "insufficient_value_evidence",
            "Recover authenticated object bytes before deciding whether this is value, type, or ownership.",
        )
    if "literal_width" in differences or "consumer_type" in differences:
        return (
            "literal_type_mismatch",
            differences,
            "literal_or_prototype_type_mismatch",
            "Inspect literal suffixes, casts, parameter types, and f32/f64 promotion boundaries.",
        )
    if "literal_value" in differences:
        return (
            "literal_value_mismatch",
            differences,
            "semantic_literal_mismatch",
            "Verify the target bit pattern and its semantic consumer before changing the source constant.",
        )
    if "owner_name" in differences:
        t_class = t_owner.get("owner_class")
        c_class = c_owner.get("owner_class")
        source_axis = (
            "Restore authenticated named constant binding or TU first-use chronology; never invent a label to steer codegen."
            if t_class == "named_label" and c_class == "compiler_anonymous"
            else "Inspect authenticated constant ownership, definition visibility, and first-use chronology."
        )
        return (
            "owner_identity_mismatch",
            differences,
            "body_value_equivalent_owner_identity_only",
            source_axis,
        )
    return (
        "owner_chronology_mismatch",
        differences,
        "body_value_equivalent_pool_chronology_only",
        "Inspect preceding literal consumers, definition order, and first-use chronology without changing literal bits.",
    )


def _pair_record(row: int, target: Mapping[str, Any] | None, candidate: Mapping[str, Any] | None) -> dict[str, Any]:
    classification, differences, interpretation, axis = _classification(target, candidate)
    return {
        "row": row,
        "classification": classification,
        "differences": differences,
        "interpretation": interpretation,
        "recommended_source_axis": axis,
        "target": target,
        "candidate": candidate,
    }


def _group_key(pair: Mapping[str, Any]) -> tuple[Any, ...]:
    target = pair.get("target") if isinstance(pair.get("target"), Mapping) else {}
    candidate = pair.get("candidate") if isinstance(pair.get("candidate"), Mapping) else {}
    t_owner = target.get("owner") if isinstance(target.get("owner"), Mapping) else {}
    c_owner = candidate.get("owner") if isinstance(candidate.get("owner"), Mapping) else {}
    t_reloc = target.get("relocation") if isinstance(target.get("relocation"), Mapping) else {}
    c_reloc = candidate.get("relocation") if isinstance(candidate.get("relocation"), Mapping) else {}
    return (
        pair.get("classification"),
        tuple(pair.get("differences", [])),
        t_owner.get("bytes"),
        c_owner.get("bytes"),
        t_owner.get("consumer_type"),
        c_owner.get("consumer_type"),
        t_owner.get("owner_class"),
        c_owner.get("owner_class"),
        t_reloc.get("type"),
        c_reloc.get("type"),
        t_reloc.get("addend"),
        c_reloc.get("addend"),
    )


def _compact_side(item: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if item is None:
        return None
    owner = item.get("owner") if isinstance(item.get("owner"), Mapping) else {}
    relocation = item.get("relocation") if isinstance(item.get("relocation"), Mapping) else {}
    return {
        "instruction": item.get("instruction"),
        "diff_kind": item.get("diff_kind"),
        "relocation": dict(relocation),
        "owner": {
            "name": owner.get("name"),
            "owner_class": owner.get("owner_class"),
            "section": owner.get("section"),
            "address": owner.get("address"),
            "size_bytes": owner.get("size_bytes"),
            "bytes": owner.get("bytes"),
            "typed": owner.get("typed"),
            "consumer_type": owner.get("consumer_type"),
        },
    }


def _section_size(side: Mapping[str, Any], name: str) -> int | None:
    for section in _sequence(side.get("sections")):
        if isinstance(section, Mapping) and section.get("name") == name:
            return _int(section.get("size"))
    for symbol in _symbols(side):
        if symbol.get("kind") == "SYMBOL_SECTION" and symbol.get("name") == f"[{name}]":
            return _int(symbol.get("size"))
    return None


def _sdata2_objects(side: Mapping[str, Any]) -> list[Mapping[str, Any]]:
    symbols = _symbols(side)
    membership = _section_membership(symbols)
    return [
        symbol
        for index, symbol in enumerate(symbols)
        if membership.get(index) == ".sdata2" and symbol.get("kind") == "SYMBOL_OBJECT"
    ]


def _weak_flag(symbol: Mapping[str, Any]) -> bool:
    flags = symbol.get("flags")
    return isinstance(flags, Mapping) and flags.get("weak") is True


def _sqrtf_prefix_role(symbol: Mapping[str, Any], role: str) -> bool:
    name = symbol.get("name")
    return (
        isinstance(name, str)
        and name.startswith(f"_{role}$localstatic")
        and name.endswith("$sqrtf__Ff")
    )


def _section_prefix_diagnosis(
    target_side: Mapping[str, Any], candidate_side: Mapping[str, Any]
) -> dict[str, Any]:
    """Diagnose only the authenticated GC/2.6 weak sqrtf two-object prefix."""

    target_objects = _sdata2_objects(target_side)
    candidate_objects = _sdata2_objects(candidate_side)
    no_match = {
        "status": "none",
        "classification": None,
        "authority_advanced": False,
    }
    if len(candidate_objects) < 2:
        return no_match
    half, three = candidate_objects[:2]
    half_address = _int(half.get("address"), 0)
    three_address = _int(three.get("address"))
    if not (
        half_address == 0
        and three_address == 8
        and _int(half.get("size")) == 8
        and _int(three.get("size")) == 8
        and _weak_flag(half)
        and _weak_flag(three)
        and _sqrtf_prefix_role(half, "half")
        and _sqrtf_prefix_role(three, "three")
        and _decode_data(half) == bytes.fromhex("3fe0000000000000")
        and _decode_data(three) == bytes.fromhex("4008000000000000")
    ):
        return no_match
    candidate_names = {str(half.get("name")), str(three.get("name"))}
    target_names = {str(symbol.get("name")) for symbol in target_objects}
    if candidate_names & target_names:
        return no_match
    candidate_size = _section_size(candidate_side, ".sdata2")
    target_size = _section_size(target_side, ".sdata2")
    if candidate_size is None or candidate_size < 16:
        return no_match
    return {
        "status": "matched",
        "classification": "candidate_only_weak_sqrtf_prefix",
        "section": ".sdata2",
        "target_section_size_bytes": target_size,
        "candidate_section_size_bytes": candidate_size,
        "removable_prefix_bytes": 16,
        "predicted_candidate_section_size_bytes": candidate_size - 16,
        "predicted_downstream_owner_offset_delta_bytes": -16,
        "objects": [
            {
                "name": half.get("name"),
                "address": half_address,
                "size_bytes": 8,
                "typed": _typed_interpretations(_decode_data(half)),
            },
            {
                "name": three.get("name"),
                "address": three_address,
                "size_bytes": 8,
                "typed": _typed_interpretations(_decode_data(three)),
            },
        ],
        "interpretation": "candidate_only_weak_sqrtf_prefix_shifts_all_downstream_sdata2_owners",
        "recommended_source_axis": (
            "Audit the authenticated include/dependency closure that instantiated sqrtf; preserve truthful prototypes and ABI. "
            "Do not add recovered-C header guards or edit literal ownership merely to move the pool."
        ),
        "authority_advanced": False,
    }


def _owner_consumers(side: Mapping[str, Any], owner_index: int | None) -> list[dict[str, Any]]:
    if owner_index is None:
        return []
    consumers: list[dict[str, Any]] = []
    for symbol in _symbols(side):
        if symbol.get("kind") != "SYMBOL_FUNCTION":
            continue
        function_name = symbol.get("name")
        if not isinstance(function_name, str) or not function_name:
            continue
        rows: list[dict[str, Any]] = []
        for row_index, row in enumerate(_sequence(symbol.get("instructions"))):
            if not isinstance(row, Mapping):
                continue
            relocation = _relocation(row)
            if relocation is None or _int(relocation.get("target_symbol")) != owner_index:
                continue
            instruction = _instruction(row) or {}
            rows.append(
                {
                    "row": row_index,
                    "instruction_address": _int(instruction.get("address")),
                    "instruction": _formatted(row),
                    "relocation_type": relocation.get("type_name"),
                    "relocation_addend": _int(relocation.get("addend"), 0),
                }
            )
        if rows:
            consumers.append(
                {
                    "function": function_name,
                    "count": len(rows),
                    "rows": rows,
                }
            )
    return sorted(consumers, key=lambda item: str(item["function"]))


def _census_side(
    side: Mapping[str, Any], row: Mapping[str, Any] | None
) -> dict[str, Any] | None:
    if row is None:
        return None
    owner = row.get("owner") if isinstance(row.get("owner"), Mapping) else {}
    owner_index = _int(owner.get("symbol_index"))
    consumers = _owner_consumers(side, owner_index)
    return {
        "symbol_index": owner_index,
        "name": owner.get("name"),
        "owner_class": owner.get("owner_class"),
        "section": owner.get("section"),
        "address": owner.get("address"),
        "size_bytes": owner.get("size_bytes"),
        "bytes": owner.get("bytes"),
        "typed": owner.get("typed"),
        "consumer_function_count": len(consumers),
        "consumer_relocation_count": sum(int(item["count"]) for item in consumers),
        "consumers": consumers,
    }


def _tu_owner_consumer_census(
    target_side: Mapping[str, Any],
    candidate_side: Mapping[str, Any],
    pairs: Sequence[Mapping[str, Any]],
) -> dict[str, Any]:
    grouped: dict[tuple[int | None, int | None], list[Mapping[str, Any]]] = defaultdict(list)
    for pair in pairs:
        target = pair.get("target") if isinstance(pair.get("target"), Mapping) else None
        candidate = pair.get("candidate") if isinstance(pair.get("candidate"), Mapping) else None
        target_owner = target.get("owner") if target and isinstance(target.get("owner"), Mapping) else {}
        candidate_owner = candidate.get("owner") if candidate and isinstance(candidate.get("owner"), Mapping) else {}
        grouped[(_int(target_owner.get("symbol_index")), _int(candidate_owner.get("symbol_index")))].append(pair)
    owners: list[dict[str, Any]] = []
    for (target_index, candidate_index), owner_pairs in sorted(
        grouped.items(), key=lambda item: (item[0][0] is None, item[0][0] or -1, item[0][1] is None, item[0][1] or -1)
    ):
        first = owner_pairs[0]
        target = first.get("target") if isinstance(first.get("target"), Mapping) else None
        candidate = first.get("candidate") if isinstance(first.get("candidate"), Mapping) else None
        target_census = _census_side(target_side, target)
        candidate_census = _census_side(candidate_side, candidate)
        target_functions = {
            str(item["function"])
            for item in (target_census or {}).get("consumers", [])
        }
        candidate_functions = {
            str(item["function"])
            for item in (candidate_census or {}).get("consumers", [])
        }
        interpretation = "consumer_sets_equal"
        if (
            target_census is not None
            and candidate_census is not None
            and target_census.get("owner_class") == "named_label"
            and candidate_census.get("owner_class") == "compiler_anonymous"
            and target_functions < candidate_functions
        ):
            interpretation = "target_named_owner_is_strict_consumer_subset_of_candidate_anonymous_pool"
        elif target_functions != candidate_functions:
            interpretation = "owner_consumer_sets_differ"
        owners.append(
            {
                "focus_rows": sorted(int(pair["row"]) for pair in owner_pairs),
                "focus_classifications": sorted({str(pair["classification"]) for pair in owner_pairs}),
                "interpretation": interpretation,
                "target": target_census,
                "candidate": candidate_census,
            }
        )
    return {
        "status": "available" if owners else "none",
        "owners": owners,
        "authority_advanced": False,
    }


def decode_function(
    report: Mapping[str, Any],
    function: str,
    *,
    include_exact: bool = False,
    group_limit: int = DEFAULT_GROUP_LIMIT,
    row_limit: int = DEFAULT_ROW_LIMIT,
) -> dict[str, Any]:
    if not isinstance(report, Mapping):
        raise PoolDecodeError("objdiff report root must be an object")
    if not isinstance(function, str) or not function.strip():
        raise PoolDecodeError("function must be a non-empty string")
    if isinstance(group_limit, bool) or not isinstance(group_limit, int) or group_limit < 0:
        raise PoolDecodeError("group_limit must be a non-negative integer")
    if isinstance(row_limit, bool) or not isinstance(row_limit, int) or row_limit < 1:
        raise PoolDecodeError("row_limit must be a positive integer")
    left, right, target_function, candidate_function = _function_pair(report, function.strip())
    target_rows = _pool_rows(left, target_function)
    candidate_rows = _pool_rows(right, candidate_function)
    all_pairs = [
        _pair_record(row, target_rows.get(row), candidate_rows.get(row))
        for row in sorted(set(target_rows) | set(candidate_rows))
    ]
    pairs = list(all_pairs)
    if not include_exact:
        pairs = [
            item
            for item in pairs
            if item["classification"] not in {"exact_pool_contract", "mapped_pool_contract"}
        ]
    grouped: dict[tuple[Any, ...], list[Mapping[str, Any]]] = defaultdict(list)
    for pair in pairs:
        grouped[_group_key(pair)].append(pair)
    priority = {
        "relocation_type_mismatch": 0,
        "relocation_addend_mismatch": 1,
        "literal_type_mismatch": 2,
        "literal_value_mismatch": 3,
        "target_only_pool_consumer": 4,
        "candidate_only_pool_consumer": 4,
        "unresolved_pool_bytes": 5,
        "owner_identity_mismatch": 6,
        "owner_chronology_mismatch": 7,
        "exact_pool_contract": 8,
        "mapped_pool_contract": 8,
    }
    ordered = sorted(
        grouped.values(),
        key=lambda items: (
            priority.get(str(items[0]["classification"]), 99),
            -len(items),
            int(items[0]["row"]),
        ),
    )
    groups: list[dict[str, Any]] = []
    for items in ordered[:group_limit]:
        first = items[0]
        groups.append(
            {
                "classification": first["classification"],
                "differences": first["differences"],
                "interpretation": first["interpretation"],
                "recommended_source_axis": first["recommended_source_axis"],
                "count": len(items),
                "rows": [item["row"] for item in items[:row_limit]],
                "rows_omitted": max(0, len(items) - row_limit),
                "target": _compact_side(first.get("target")),
                "candidate": _compact_side(first.get("candidate")),
            }
        )
    classifications = Counter(str(item["classification"]) for item in pairs)
    target_size = _int(target_function.get("size"))
    candidate_size = _int(candidate_function.get("size"))
    return {
        "schema": SCHEMA,
        "schema_version": 1,
        "function": function.strip(),
        "target": {
            "size_bytes": target_size,
            "pool_consumer_count": len(target_rows),
        },
        "candidate": {
            "size_bytes": candidate_size,
            "match_percent": candidate_function.get("match_percent"),
            "pool_consumer_count": len(candidate_rows),
        },
        "summary": {
            "paired_or_unpaired_rows": len(pairs),
            "classification_counts": dict(sorted(classifications.items())),
            "value_equivalent_owner_only_count": sum(
                1
                for item in pairs
                if item["interpretation"] in {
                    "body_value_equivalent_owner_identity_only",
                    "body_value_equivalent_pool_chronology_only",
                }
            ),
            "semantic_or_contract_mismatch_count": sum(
                1
                for item in pairs
                if item["interpretation"] not in {
                    "exact",
                    "exact_relocation_mapping_with_external_owner_contract",
                    "exact_relocation_mapping_with_object_local_owner_identity",
                    "body_value_equivalent_owner_identity_only",
                    "body_value_equivalent_pool_chronology_only",
                }
            ),
        },
        "groups": groups,
        "groups_omitted": max(0, len(ordered) - group_limit),
        "section_prefix_diagnosis": _section_prefix_diagnosis(left, right),
        "tu_owner_consumer_census": _tu_owner_consumer_census(
            left,
            right,
            [
                pair
                for pair in all_pairs
                if pair["classification"] not in {"exact_pool_contract", "mapped_pool_contract"}
            ],
        ),
        "include_exact": include_exact,
        "authority_advanced": False,
    }


def load_report(path: Path) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise PoolDecodeError(f"cannot read report {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise PoolDecodeError(f"invalid JSON report {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise PoolDecodeError("objdiff report root must be an object")
    return value


def _canonical_sha256(value: Mapping[str, Any]) -> str:
    encoded = (json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False) + "\n").encode()
    return hashlib.sha256(encoded).hexdigest()


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    parser.add_argument("function")
    parser.add_argument("--include-exact", action="store_true")
    parser.add_argument("--group-limit", type=int, default=DEFAULT_GROUP_LIMIT)
    parser.add_argument("--row-limit", type=int, default=DEFAULT_ROW_LIMIT)
    args = parser.parse_args(argv)
    try:
        result = decode_function(
            load_report(args.report),
            args.function,
            include_exact=args.include_exact,
            group_limit=args.group_limit,
            row_limit=args.row_limit,
        )
    except PoolDecodeError as exc:
        parser.error(str(exc))
    result["decoder_sha256"] = _canonical_sha256(result)
    print(json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
