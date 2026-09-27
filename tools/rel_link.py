#!/usr/bin/env python3
"""Place native REL sections and resolve compiler-local readonly exports.

Only existing undefined symbol definitions are completed after native, split and
linked storage/reference checks. Code, data and relocation bytes are unchanged.
This build helper requires only the Python standard library.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import sys
from typing import Any, Mapping


class ELFError(ValueError):
    """The input does not establish the required ELF structure or ownership."""


def _canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")

def _elf_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def _elf_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def _cstring(data: bytes, offset: int) -> str:
    end = data.find(b"\0", offset)
    if end < 0:
        raise ELFError("unterminated ELF string")
    return data[offset:end].decode("utf-8", errors="strict")


def _parse_elf_structure(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    if len(data) < 52 or data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 2:
        raise ELFError(f"{path} is not a big-endian ELF32 object")
    if _elf_u16(data, 18) != 20:
        raise ELFError(f"{path} is not PowerPC ELF")
    shoff, shentsize, shnum, shstrndx = _elf_u32(data, 32), _elf_u16(data, 46), _elf_u16(data, 48), _elf_u16(data, 50)
    if shentsize != 40 or shnum == 0 or shstrndx >= shnum:
        raise ELFError("unsupported ELF section table")
    sections: list[dict[str, Any]] = []
    for index in range(shnum):
        off = shoff + index * shentsize
        if off + 40 > len(data):
            raise ELFError("truncated ELF section table")
        sections.append({
            "name_off": _elf_u32(data, off), "type": _elf_u32(data, off + 4),
            "flags": _elf_u32(data, off + 8),
            "offset": _elf_u32(data, off + 16), "size": _elf_u32(data, off + 20),
            "link": _elf_u32(data, off + 24), "info": _elf_u32(data, off + 28),
            "entsize": _elf_u32(data, off + 36),
        })
    for index, section in enumerate(sections):
        # SHT_NOBITS occupies memory, not file bytes; all other payloads must
        # exist physically before any equality or ownership check can use them.
        if section["type"] not in (0, 8) and (
            section["offset"] > len(data)
            or section["size"] > len(data) - section["offset"]
        ):
            raise ELFError(f"{path}: section {index} payload extends beyond file")
    shstr = sections[shstrndx]
    shnames = data[shstr["offset"]:shstr["offset"] + shstr["size"]]
    for section in sections:
        section["name"] = _cstring(shnames, section["name_off"])
    symtabs = [(i, row) for i, row in enumerate(sections) if row["type"] == 2]
    if len(symtabs) != 1:
        raise ELFError("ELF must contain exactly one symbol table")
    sym_index, symtab = symtabs[0]
    if symtab["entsize"] != 16 or symtab["link"] >= shnum:
        raise ELFError("unsupported ELF symbol table")
    strtab = sections[symtab["link"]]
    strings = data[strtab["offset"]:strtab["offset"] + strtab["size"]]
    symbols: list[dict[str, Any]] = []
    for off in range(symtab["offset"], symtab["offset"] + symtab["size"], 16):
        if off + 16 > len(data):
            raise ELFError("truncated ELF symbol table")
        symbols.append({
            "name": _cstring(strings, _elf_u32(data, off)), "value": _elf_u32(data, off + 4),
            "size": _elf_u32(data, off + 8), "info": data[off + 12], "section": _elf_u16(data, off + 14),
        })
    return {"data": data, "sections": sections, "symbols": symbols,
            "sym_index": sym_index, "shnum": shnum,
            "object": {"path": str(path.resolve()), "sha256": hashlib.sha256(data).hexdigest(), "size_bytes": len(data)}}


def _parse_elf_relocations(path: Path, function: str, *, structure: Mapping[str, Any] | None = None) -> dict[str, Any]:
    parsed = structure if structure is not None else _parse_elf_structure(path)
    data, sections, symbols = parsed["data"], parsed["sections"], parsed["symbols"]
    sym_index, shnum = parsed["sym_index"], parsed["shnum"]
    matches = [row for row in symbols if (row["info"] & 0xF) == 2 and row["name"] == function]
    if len(matches) != 1:
        raise ELFError(f"ELF must contain exactly one function {function!r}; found {len(matches)}")
    focus = matches[0]
    section_index, start, size = focus["section"], focus["value"], focus["size"]
    if section_index == 0 or section_index >= shnum or size <= 0 or size % 4:
        raise ELFError("focus function has uncertain section/extent")
    focus_section = sections[section_index]
    relocation_sections = [row for row in sections if row["type"] in {4, 9} and row["info"] == section_index]
    if len(relocation_sections) > 1:
        raise ELFError("multiple relocation sections target the focus section")
    rows: list[dict[str, Any]] = []
    if relocation_sections:
        relsec = relocation_sections[0]
        if relsec["type"] != 4:
            raise ELFError("SHT_REL addends cannot be proven independently; SHT_RELA is required")
        expected_ent = 12
        if relsec["entsize"] != expected_ent or relsec["link"] != sym_index:
            raise ELFError("unsupported focus relocation section")
        for off in range(relsec["offset"], relsec["offset"] + relsec["size"], expected_ent):
            if off + expected_ent > len(data):
                raise ELFError("truncated ELF relocation section")
            rel_offset, info = _elf_u32(data, off), _elf_u32(data, off + 4)
            if not start <= rel_offset < start + size:
                continue
            symbol_index, rel_type = info >> 8, info & 0xFF
            if symbol_index >= len(symbols):
                raise ELFError("relocation symbol index is invalid")
            addend = struct.unpack_from(">i", data, off + 8)[0]
            symbol = symbols[symbol_index]
            symbol_section = symbol["section"]
            if symbol_section == 0:
                effective = {"kind": "undefined", "name": symbol["name"], "addend": addend}
            elif symbol_section == 0xFFF1:
                effective = {"kind": "absolute", "name": symbol["name"], "value": symbol["value"] + addend}
            elif symbol_section < shnum:
                effective = {
                    "kind": "section", "section": sections[symbol_section]["name"],
                    "offset": symbol["value"] + addend,
                }
            else:
                raise ELFError("relocation targets an unsupported special section")
            rows.append({
                "offset": rel_offset - start, "type": rel_type,
                "symbol": symbol["name"], "symbol_value": symbol["value"],
                "addend": addend, "effective_target": effective,
            })
    rows.sort(key=lambda row: (row["offset"], row["type"], _canonical(row["effective_target"])))
    return {
        "object": dict(parsed["object"]), "section": focus_section["name"], "offset": start,
        "size": size, "instruction_count": size // 4, "physical_relocation_count": len(rows),
        "physical_relocations": rows,
    }


def validate_pool_text_members(exports: list[dict], module: str) -> dict[str, list[dict]]:
    """Validate explicit native sections replacing pure target code splits."""
    prefix = f"REL/{module}/"
    result, claimed = {}, set()
    providers = {item.get('source') for item in exports}
    for export in exports:
        members = export.get('text_members', [])
        if not members:
            continue
        source = export.get('source')
        if (not isinstance(source, str) or not source.startswith(prefix)
                or '..' in Path(source).parts or source in result or not isinstance(members, list)):
            raise ValueError('invalid native pool text member provider')
        sections = set()
        checked = []
        for member in members:
            if not isinstance(member, dict) or set(member) != {'source', 'section'}:
                raise ValueError('text members require source and section only')
            unit, section = member['source'], member['section']
            if (not isinstance(unit, str) or not unit.startswith(prefix)
                    or not re.fullmatch(r'[A-Za-z0-9_./]+', unit)
                    or '..' in Path(unit).parts or unit in providers or unit in claimed
                    or not isinstance(section, str)
                    or not re.fullmatch(r'\.text\.[A-Za-z0-9_]+', section)
                    or section in sections):
                raise ValueError('duplicate, escaping or invalid native pool text member')
            claimed.add(unit)
            sections.add(section)
            checked.append(dict(member))
        result[source] = checked
    return result


def validated_pool_members(binding: dict) -> list[dict]:
    """Return explicit scalar ownership, retaining the legacy singleton form."""
    size = binding.get('size')
    native_section = binding.get('native_section', binding.get('section'))
    if (binding.get('section') != '.rodata' or type(size) is not int or size <= 0
            or not isinstance(native_section, str) or not re.fullmatch(r'\.rodata(?:\.[A-Za-z0-9_]+)?', native_section)
            or type(binding.get('native_offset')) is not int or binding['native_offset'] != 0
            or type(binding.get('linked_offset')) is not int or binding['linked_offset'] < 0):
        raise ValueError('invalid readonly pool extent')
    members = binding.get('members', [{'symbol': binding.get('symbol'), 'offset': 0, 'size': size}])
    if not isinstance(members, list) or not members:
        raise ValueError('pool members must be a nonempty list')
    names, end = set(), 0
    for member in members:
        if not isinstance(member, dict) or set(member) != {'symbol', 'offset', 'size'}:
            raise ValueError('pool member requires symbol, offset and size only')
        name, offset, length = member['symbol'], member['offset'], member['size']
        if (not isinstance(name, str) or not re.fullmatch(r'[A-Za-z_.$][A-Za-z0-9_.$]*', name)
                or name in names or type(offset) is not int or type(length) is not int
                or length <= 0 or offset < end or offset + length > size):
            raise ValueError('invalid, duplicate or overlapping pool member')
        names.add(name)
        end = offset + length
    if members[0]['symbol'] != binding.get('symbol') or members[0]['offset'] != 0:
        raise ValueError('primary pool symbol must be the first offset-zero member')
    return [dict(member) for member in members]


def _pool_member_owners(elf: dict, index: int, members: list[dict], base: int,
                        size: int, *, native: bool = False) -> None:
    owners = [s for s in elf['symbols'] if s['section'] == index
              and s['info'] & 15 != 3 and base <= s['value'] < base + size]
    expected = {(base + m['offset'], m['size']): m['symbol'] for m in members}
    sec = elf['sections'][index]
    cursor, holes = 0, set()
    for member in members:
        offset = member['offset']
        # Scalar compiler owners may have only natural, zero alignment gaps.
        alignment = min(member['size'], 8)
        if offset != cursor:
            if (alignment not in (1, 2, 4, 8) or offset != (cursor + alignment - 1) // alignment * alignment
                    or any(elf['data'][sec['offset'] + base + cursor:sec['offset'] + base + offset])):
                raise ValueError('pool has non-natural or nonzero alignment gap')
            holes.add((base + cursor, offset - cursor))
        cursor = offset + member['size']
    if cursor != size:
        raise ValueError('pool has unowned trailing storage')
    seen, gap_symbols = set(), set()
    for owner in owners:
        key = owner['value'], owner['size']
        split_gap = (not native and key in holes
                     and re.fullmatch(r'gap_[0-9A-Fa-f]+_[0-9A-Fa-f]+_rodata', owner['name']))
        if (key in seen or owner['info'] & 15 != 1
                or owner['info'] >> 4 != (0 if native else 1)
                or (key not in expected and not split_gap)
                or (not native and not split_gap and owner['name'] != expected[key])):
            raise ValueError('pool member ownership is omitted, ambiguous or conflicting')
        seen.add(key)
        if split_gap:
            gap_symbols.add(owner['name'])
    if seen - holes != set(expected):
        raise ValueError('pool member ownership is incomplete')
    # DTK may name zero alignment holes. Such names are not scalar owners and
    # cannot have consumers, even references with an addend escaping the hole.
    for relsec in elf['sections']:
        if relsec['type'] not in (4, 9) or not relsec['size']:
            continue
        entry_size = 12 if relsec['type'] == 4 else 8
        if relsec['link'] != elf['sym_index'] or relsec['size'] % entry_size:
            raise ValueError('unsupported relocation table in pool ownership proof')
        for offset in range(relsec['offset'], relsec['offset'] + relsec['size'], entry_size):
            _, info = struct.unpack_from('>II', elf['data'], offset)
            symbol_index = info >> 8
            if symbol_index >= len(elf['symbols']):
                raise ValueError('invalid relocation symbol in pool ownership proof')
            target = elf['symbols'][symbol_index]
            if target['name'] in gap_symbols:
                raise ValueError('split alignment gap has a relocated use')
            if target['section'] != index or not holes:
                continue
            if relsec['type'] == 9:
                raise ValueError('implicit relocation cannot prove unused alignment gaps')
            addend = struct.unpack_from('>i', elf['data'], offset + 8)[0]
            effective = target['value'] + addend
            if any(start <= effective < start + length for start, length in holes):
                raise ValueError('split alignment gap has a relocated use')


def data_pool_exports(exports: list[dict], module: str) -> list[dict]:
    """Separate real readonly exports from data-free native text groups.

    Text-only groups use exactly source/text_members. They still undergo the
    same split ownership, native section and linked instruction/reference
    checks; they simply have no undefined data symbol to resolve.
    """
    groups = validate_pool_text_members(exports, module)
    pools = []
    for row in exports:
        if 'symbol' in row:
            validated_pool_members(row)
            pools.append(row)
        elif set(row) != {'source', 'text_members'} or row.get('source') not in groups:
            raise ValueError('code-only native group requires source and nonempty text_members only')
    return pools


def _linked_reference(binding: dict, linked: dict, aliases: dict) -> dict:
    """Resolve only a unique actual link symbol; preserve external identities."""
    if binding.get('kind') != 'undefined':
        return dict(binding)
    name = aliases.get(binding['name'], binding['name'])
    symbols = [s for s in linked['symbols'] if s['name'] == name]
    if len(symbols) != 1:
        raise ValueError('missing or ambiguous linked symbol: ' + name)
    symbol = symbols[0]
    if symbol['section'] == 0:
        return {**binding, 'name': name}
    if not 0 < symbol['section'] < len(linked['sections']):
        raise ValueError('unsupported absolute/common linked symbol: ' + name)
    return {'kind': 'section', 'section': linked['sections'][symbol['section']]['name'],
            'offset': symbol['value'] + binding['addend']}


def validate_split_pool(path: Path, binding: dict) -> None:
    """A separately ordered retail pool may replace only a complete data-only input."""
    elf = _parse_elf_structure(path)
    members = validated_pool_members(binding)
    allocated = [(i, s) for i, s in enumerate(elf['sections']) if s['flags'] & 2 and s['size']]
    if len(allocated) != 1:
        raise ValueError('separate pool input must have exactly one allocated section')
    index, sec = allocated[0]
    owners = [s for s in elf['symbols'] if s['name'] == binding['symbol']]
    if (sec['name'] != '.rodata' or sec['flags'] & 5 or sec['type'] != 1
            or sec['size'] != binding['size'] or len(owners) != 1
            or owners[0]['section'] != index or owners[0]['value'] != 0
            or owners[0]['size'] != members[0]['size'] or owners[0]['info'] & 15 != 1
            or any(s['size'] and s['type'] in (4, 9) for s in elf['sections'])):
        raise ValueError('separate pool input is not a complete relocation-free readonly owner')
    if 'members' in binding:
        _pool_member_owners(elf, index, members, 0, binding['size'])


def validate_split_code(path: Path) -> dict:
    """A replaced code member must contain only target instructions, not data."""
    obj = _parse_elf_structure(path)
    allocated = [(i, s) for i, s in enumerate(obj['sections']) if s['flags'] & 2 and s['size']]
    if (len(allocated) != 1 or allocated[0][1]['name'] != '.text'
            or allocated[0][1]['type'] != 1 or allocated[0][1]['flags'] != 6):
        raise ValueError('text member must be a pure executable target text split')
    section_index, section = allocated[0]
    functions = [s for s in obj['symbols'] if s['section'] == section_index and s['info'] & 15 == 2 and s['size']]
    cursor = 0
    for func in sorted(functions, key=lambda s: s['value']):
        if func['value'] != cursor:
            raise ValueError('text member has gaps, overlaps or aliases between functions')
        cursor += func['size']
    if cursor != section['size']:
        raise ValueError('text member is not completely covered by sized functions')
    return obj


def resolve_pool_exports(input_path: Path, output_path: Path, bindings: list[dict]) -> dict:
    """Finish verified data-symbol resolution in a MWLD partial REL link.

    MWLD -r1 does not emit LCF-created symbols in this toolchain. A selected
    compiler object can therefore own the correct bytes while the remaining
    assembler objects still import their target label. Resolve only existing
    undefined labels, from explicit native/target split ownership; an absent
    importer needs no symbol edit after the same proof. No code, data,
    string-table, or relocation bytes are rewritten.
    """

    if input_path.resolve() == output_path.resolve() or not bindings:
        raise ValueError("pool export requires separate input/output and explicit bindings")
    linked = _parse_elf_structure(input_path)
    result = bytearray(linked["data"])
    symtab = linked["sections"][linked["sym_index"]]
    if symtab["flags"] & 2:
        raise ValueError("symbol table is unexpectedly allocated")
    records, names, permitted = [], set(), set()
    pool_placements = []
    for binding in bindings:
        validated_pool_members(binding)
        provider = str(Path(binding['native_object']).resolve())
        native_section = binding.get('native_section', binding['section'])
        start, end = binding['linked_offset'], binding['linked_offset'] + binding['size']
        if any((provider == p and native_section == s) or (start < e and b < end)
               for p, s, b, e in pool_placements):
            raise ValueError('duplicate native section or overlapping linked pool exports')
        pool_placements.append((provider, native_section, start, end))

    def section(elf, name):
        found = [(i, s) for i, s in enumerate(elf["sections"]) if s["name"] == name]
        if len(found) != 1:
            raise ValueError(f"missing or ambiguous section {name}")
        return found[0]

    def symbol(elf, name):
        found = [(i, s) for i, s in enumerate(elf["symbols"]) if s["name"] == name]
        if len(found) != 1:
            raise ValueError(f"missing or ambiguous symbol {name}")
        return found[0]

    def extent(elf, sec, start, size):
        if type(start) is not int or type(size) is not int or start < 0 or size <= 0 or start + size > sec["size"]:
            raise ValueError("pool extent outside section")
        payload = elf["data"][sec["offset"] + start:sec["offset"] + start + size]
        if len(payload) != size:
            raise ValueError("pool extent is not physically present in file")
        return payload

    for binding in bindings:
        members = validated_pool_members(binding)
        name, secname = binding["symbol"], binding["section"]
        if any(m['symbol'] in names for m in members) or secname != ".rodata":
            raise ValueError("duplicate export or non-readonly pool")
        names.update(m['symbol'] for m in members)
        native = _parse_elf_structure(Path(binding["native_object"]))
        target = _parse_elf_structure(Path(binding["target_object"]))
        code_members = {}
        member_sections = set()
        for member in binding.get('target_code_members', []):
            if not isinstance(member, dict) or set(member) != {'target_object', 'native_section'}:
                raise ValueError('invalid target code member binding')
            member_section = member['native_section']
            if (not isinstance(member_section, str) or not re.fullmatch(r'\.text\.[A-Za-z0-9_]+', member_section)
                    or member_section in member_sections):
                raise ValueError('invalid or duplicate member native section')
            member_sections.add(member_section)
            member_path = Path(member['target_object'])
            member_target = validate_split_code(member_path)
            _, native_member_section = section(native, member_section)
            if native_member_section['type'] != 1 or native_member_section['flags'] != 6:
                raise ValueError('native code member section is not executable code')
            native_names = {s['name'] for s in native['symbols'] if s['info'] & 15 == 2
                            and 0 < s['section'] < native['shnum']
                            and native['sections'][s['section']]['name'] == member_section}
            target_names = {s['name'] for s in member_target['symbols'] if s['info'] & 15 == 2 and s['section'] != 0}
            if native_names != target_names or not native_names:
                raise ValueError('native/target code member function sets disagree')
            for function_name in target_names:
                if function_name in code_members:
                    raise ValueError('duplicate target code member function')
                code_members[function_name] = (member_path, member_target)
        pool_target = target
        if binding.get('target_pool_object'):
            validate_split_pool(Path(binding['target_pool_object']), binding)
            pool_target = _parse_elf_structure(Path(binding['target_pool_object']))
        native_secname = binding.get('native_section', secname)
        ni, ns = section(native, native_secname)
        ti, ts = section(pool_target, secname)
        li, ls = section(linked, secname)
        _, owner = symbol(pool_target, name)
        dest, size = binding["linked_offset"], binding["size"]
        start = binding["native_offset"]
        if (owner["section"] != ti or owner["size"] != members[0]['size'] or owner["info"] & 15 != 1
                or ns["flags"] != 2 or ls["flags"] != 2 or ts['flags'] != 2 or ns["type"] != 1
                or ts['type'] != 1 or ls['type'] != 1
                or any(s['type'] in (4, 9) and s['size'] and s['info'] == ni for s in native['sections'])
                or start != 0 or size != ns["size"]):
            raise ValueError("pool must be a complete, target-backed readonly native section")
        payload = extent(native, ns, start, size)
        if payload != extent(pool_target, ts, owner["value"], size) or payload != extent(linked, ls, dest, size):
            raise ValueError("native/target/linked pool bytes disagree")
        if 'members' in binding:
            _pool_member_owners(native, ni, members, start, size, native=True)
            _pool_member_owners(pool_target, ti, members, owner['value'], size)
        providers = [s for s in native["symbols"] if s["section"] == ni
                     and s["value"] == start and s["size"] == size and s["info"] & 15 == 1]
        if 'members' not in binding and (len(providers) != 1 or providers[0]["info"] >> 4 != 0):
            raise ValueError("pool is not a unique compiler-local native object")
        uses = 0
        def normalized_reference(ref, elf, pool_base=None):
            effective = ref['effective_target']
            if effective.get('kind') == 'undefined':
                for other in bindings:
                    for member in validated_pool_members(other):
                        if effective['name'] == member['symbol']:
                            return {'kind': 'section', 'section': other['section'],
                                    'offset': other['linked_offset'] + member['offset'] + effective['addend']}
                return _linked_reference(effective, linked, {})
            if effective.get('kind') != 'section':
                return effective
            if elf is linked:
                return effective
            if elf is native:
                for other in bindings:
                    if (Path(other['native_object']).resolve() == Path(binding['native_object']).resolve()
                            and effective['section'] == other.get('native_section', other['section'])):
                        validated_pool_members(other)
                        if not 0 <= effective['offset'] < other['size']:
                            raise ValueError('native reference escapes the pool')
                        return {'kind': 'section', 'section': other['section'],
                                'offset': other['linked_offset'] + effective['offset']}
            if effective['section'] == secname and pool_base is not None:
                return {**effective, 'offset': dest + effective['offset'] - pool_base}
            containing = [s for s in elf['symbols'] if s['info'] & 15 == 2
                and 0 < s['section'] < elf['shnum']
                and elf['sections'][s['section']]['name'] == effective['section']
                and s['value'] <= effective['offset'] < s['value'] + s['size']]
            if len(containing) != 1:
                raise ValueError('provider reference outside proven function ownership')
            origin = containing[0]
            _, final = symbol(linked, origin['name'])
            return {'kind': 'section', 'section': linked['sections'][final['section']]['name'],
                    'offset': final['value'] + effective['offset'] - origin['value']}

        functions = [s for s in native["symbols"] if s["info"] & 15 == 2 and 0 < s["section"] < native["shnum"]]
        if not functions:
            raise ValueError("native pool has no compiled function provider")
        for func in functions:
            function_target_path, function_target = code_members.get(func['name'], (Path(binding['target_object']), target))
            _, lf = symbol(linked, func["name"])
            _, tf = symbol(function_target, func["name"])
            code = extent(native, native["sections"][func["section"]], func["value"], func["size"])
            if (lf["size"] != func["size"] or tf["size"] != func["size"] or
                    code != extent(linked, linked["sections"][lf["section"]], lf["value"], lf["size"]) or
                    code != extent(function_target, function_target["sections"][tf["section"]], tf["value"], tf["size"])):
                raise ValueError("native provider instructions are not target/linked exact")
            nr = _parse_elf_relocations(Path(binding["native_object"]), func["name"], structure=native)["physical_relocations"]
            lr = _parse_elf_relocations(input_path, func["name"], structure=linked)["physical_relocations"]
            tr = _parse_elf_relocations(function_target_path, func["name"], structure=function_target)["physical_relocations"]
            lm = {(r["offset"], r["type"]): r for r in lr}
            tm = {(r["offset"], r["type"]): r for r in tr}
            for ref in nr:
                effective = ref["effective_target"]
                if effective.get("kind") != "section" or effective.get("section") != native_secname:
                    continue
                offset = effective["offset"] - start
                if not 0 <= offset < size:
                    raise ValueError("native reference escapes the pool")
                key = ref["offset"], ref["type"]
                expected = {"kind": "section", "section": secname, "offset": dest + offset}
                target_expected = {"kind": "section", "section": secname, "offset": owner["value"] + offset}
                target_matches = key in tm and tm[key]['effective_target'] == target_expected
                if (binding.get('target_pool_object') or func['name'] in code_members) and key in tm:
                    # The provider split imports the separate retail data owner.
                    target_matches = any(tm[key]['symbol'] == m['symbol']
                        and tm[key]['addend'] == offset - m['offset']
                        and m['offset'] <= offset < m['offset'] + m['size']
                        and tm[key]['effective_target'].get('kind') == 'undefined' for m in members)
                if key not in lm or not target_matches or lm[key]["effective_target"] != expected:
                    raise ValueError("native pool use lacks exact target/linked ownership")
                uses += 1
            if nr:
                native_refs = [(r['offset'], r['type'], normalized_reference(r, native, start)) for r in nr]
                target_refs = [(r['offset'], r['type'], normalized_reference(r, function_target,
                    owner['value'] if function_target is pool_target else None)) for r in tr]
                linked_refs = [(r['offset'], r['type'], normalized_reference(r, linked)) for r in lr]
                if native_refs != target_refs or native_refs != linked_refs:
                    first = next(i for i in range(max(len(native_refs), len(target_refs), len(linked_refs)))
                        if (native_refs[i:i+1] != target_refs[i:i+1]
                            or native_refs[i:i+1] != linked_refs[i:i+1]))
                    raise ValueError('native provider relocation identities are not target/linked exact: '
                        f'{func["name"]} row {first}; native={native_refs[first:first+1]}; '
                        f'target={target_refs[first:first+1]}; linked={linked_refs[first:first+1]}')
            elif tr or lr:
                raise ValueError('no native relocation matches target/linked provider references')
        if not uses:
            raise ValueError("no native relocation proves this pool owner")
        unimported = []
        for member in members:
            if not any(s['name'] == member['symbol'] for s in linked['symbols']):
                # Once the native/target/linked ownership and provider proofs
                # above pass, an unimported scalar needs no new ELF symbol.
                unimported.append(member['symbol'])
                continue
            index, unresolved = symbol(linked, member['symbol'])
            if unresolved["section"] != 0 or unresolved["info"] >> 4 != 1:
                raise ValueError("export must resolve one existing global undefined symbol")
            off = symtab["offset"] + index * 16 + 4
            struct.pack_into(">IIB", result, off, dest + member['offset'], member['size'], 0x11)
            struct.pack_into(">H", result, off + 10, li)
            permitted.update(range(off, off + 9))
            permitted.update(range(off + 10, off + 12))
        records.append({"symbol": name, "section": secname, "offset": dest, "size": size,
                        "native_object": native["object"], "target_object": target["object"],
                        "target_pool_object": pool_target["object"], "native_pool_uses": uses,
                        **({'members': members} if 'members' in binding else {}),
                        **({'unimported_members': unimported} if unimported else {}),
                        **({'target_code_members': binding['target_code_members']} if code_members else {})})
    if any(a != b and i not in permitted for i, (a, b) in enumerate(zip(linked["data"], result))):
        raise ValueError("resolution attempted to alter bytes outside symbol definitions")
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary = output_path.with_name(output_path.name + ".tmp")
    temporary.write_bytes(result)
    os.replace(temporary, output_path)
    return {"resolved_pool_exports": records, "input_sha256": linked["object"]["sha256"],
            "output_sha256": hashlib.sha256(result).hexdigest(),
            "code_data_relocations_unchanged": True}


def resolve_pool_exports_main(argv):
    parser = argparse.ArgumentParser(description="Resolve proven native constant exports in a partial REL link")
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--bindings", required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        result = resolve_pool_exports(args.input, args.output, json.loads(args.bindings.read_text(encoding="utf-8")))
        print(json.dumps(result))
        return 0
    except (OSError, ValueError, KeyError, IndexError, struct.error) as exc:
        print(str(exc), file=sys.stderr)
        return 2


if __name__ == "__main__":
    args = sys.argv[1:]
    if args and args[0] == "resolve-pool-exports":
        args = args[1:]
    sys.exit(resolve_pool_exports_main(args))
