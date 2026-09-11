from __future__ import annotations

import hashlib
import copy
import struct
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from tools import recovery_object_inventory as inventory


def _table_string(names: list[str]) -> tuple[bytes, dict[str, int]]:
    data = bytearray(b"\0")
    offsets: dict[str, int] = {"": 0}
    for name in names:
        offsets[name] = len(data)
        data.extend(name.encode("ascii"))
        data.append(0)
    return bytes(data), offsets


def _symbol(
    name: int,
    value: int,
    size: int,
    info: int,
    section: int,
) -> bytes:
    return struct.pack(">IIIBBH", name, value, size, info, 0, section)


def _write_elf(
    path: Path,
    *,
    file_name: str = "first.c",
    ext_name: str = "external",
    text: bytes | None = None,
    data: bytes = b"DATA",
    local_name: str = "local_label",
    foo_start: int = 0,
    bar_start: int = 8,
    local_value: int = 4,
    rel_text_rows: list[tuple[int, int, int, int]] | None = None,
    rela: bool = True,
    overlap: bool = False,
) -> None:
    if text is None:
        text = b"\x60\x00\x00\x00\x4e\x80\x00\x20\x60\x00\x00\x00"
    if rel_text_rows is None:
        rel_text_rows = [(foo_start, 2, 10, 0), (foo_start + 4, 3, 1, 0)]
    if overlap:
        bar_start = 4

    section_names = ["", ".text", ".data", ".bss", ".shstrtab", ".symtab", ".strtab", ".rela.text", ".rela.data"]
    shstrtab, section_name_offsets = _table_string(section_names[1:])
    symbol_names = ["", "foo", "bar", local_name, ext_name, file_name]
    strtab, symbol_name_offsets = _table_string(symbol_names[1:])
    symbols = b"".join(
        (
            _symbol(0, 0, 0, 0, 0),
            _symbol(symbol_name_offsets["foo"], foo_start, 8, 0x12, 1),
            _symbol(symbol_name_offsets["bar"], bar_start, 4, 0x12, 1),
            _symbol(symbol_name_offsets[local_name], local_value, 0, 0x01, 1),
            _symbol(symbol_name_offsets[ext_name], 0, 0, 0x10, 0),
            _symbol(symbol_name_offsets[file_name], 0, 0, 0x04, 0),
        )
    )
    rela_text = b"".join(
        struct.pack(">IIi", offset, (symbol << 8) | kind, addend)
        for offset, symbol, kind, addend in rel_text_rows
    )
    rela_data = struct.pack(">IIi", 0, (4 << 8) | 1, 0)
    if not rela:
        # The inventory must reject SHT_REL before attempting to interpret its
        # records; keeping the payload empty makes this a minimal fixture.
        rela_text = b""

    sections = [
        {"name": "", "type": 0, "flags": 0, "align": 0, "content": b"", "link": 0, "info": 0, "entsize": 0},
        {"name": ".text", "type": 1, "flags": 0x6, "align": 4, "content": text, "link": 0, "info": 0, "entsize": 0},
        {"name": ".data", "type": 1, "flags": 0x3, "align": 4, "content": data, "link": 0, "info": 0, "entsize": 0},
        {"name": ".bss", "type": 8, "flags": 0x3, "align": 4, "content": b"", "size": 4, "link": 0, "info": 0, "entsize": 0},
        {"name": ".shstrtab", "type": 3, "flags": 0, "align": 1, "content": shstrtab, "link": 0, "info": 0, "entsize": 0},
        {"name": ".symtab", "type": 2, "flags": 0, "align": 4, "content": symbols, "link": 6, "info": 1, "entsize": 16},
        {"name": ".strtab", "type": 3, "flags": 0, "align": 1, "content": strtab, "link": 0, "info": 0, "entsize": 0},
        {"name": ".rela.text", "type": 4 if rela else 9, "flags": 0, "align": 4, "content": rela_text, "link": 5, "info": 1, "entsize": 12 if rela else 8},
        {"name": ".rela.data", "type": 4, "flags": 0, "align": 4, "content": rela_data, "link": 5, "info": 2, "entsize": 12},
    ]
    offset = 0x100
    image = bytearray(offset)
    for section in sections:
        content = bytes(section["content"])
        if section["type"] != 8:
            offset = (offset + 3) & ~3
            section["offset"] = offset
            section["size"] = len(content)
            image.extend(b"\0" * (offset + len(content) - len(image)))
            image[offset:offset + len(content)] = content
            offset += len(content)
        else:
            section["offset"] = offset
            section.setdefault("size", 0)
    shoff = (offset + 3) & ~3
    image.extend(b"\0" * (shoff + len(sections) * 40 - len(image)))
    ident = b"\x7fELF" + bytes((1, 2, 1)) + b"\0" * 9
    header = struct.pack(
        ">16sHHIIIIIHHHHHH",
        ident, 1, 20, 1, 0, 0, shoff, 0, 52, 0, 0, 40, len(sections), 4,
    )
    image[0:52] = header
    for index, section in enumerate(sections):
        entry = shoff + index * 40
        image[entry:entry + 40] = struct.pack(
            ">IIIIIIIIII",
            section_name_offsets[section["name"]],
            int(section["type"]),
            int(section["flags"]),
            0,
            int(section["offset"]),
            int(section["size"]),
            int(section["link"]),
            int(section["info"]),
            int(section["align"]),
            int(section["entsize"]),
        )
    path.write_bytes(bytes(image))


def _write_pool_elf(
    path: Path,
    *,
    pool_offset: int = 0,
    pool_flags: int = 2,
    pool_bytes: bytes = bytes.fromhex("3f800000"),
    owner_name: str = "pool_owner",
    function_names: tuple[str, ...] = ("foo", "bar"),
    instruction_opcode: int = 48,
    relocation_bias: int = 0,
    duplicate_owner_name: str | None = None,
    duplicate_owner_offset: int | None = None,
) -> None:
    """Small relocatable fixture with shared pool consumers and a moved owner."""
    section_names = ["", ".text", ".sdata2", ".shstrtab", ".symtab", ".strtab", ".rela.text"]
    shstrtab, section_name_offsets = _table_string(section_names[1:])
    symbol_names = ["", *function_names, owner_name]
    if duplicate_owner_name is not None:
        symbol_names.append(duplicate_owner_name)
    symbol_names.append("fixture.c")
    strtab, symbol_name_offsets = _table_string(symbol_names[1:])
    symbols = [struct.pack(">IIIBBH", 0, 0, 0, 0, 0, 0)]
    for index, name in enumerate(function_names):
        symbols.append(_symbol(symbol_name_offsets[name], index * 4, 4, 0x12, 1))
    owner_index = len(symbols)
    symbols.append(_symbol(symbol_name_offsets[owner_name], pool_offset, len(pool_bytes), 0x11, 2))
    if duplicate_owner_name is not None:
        duplicate_offset = (pool_offset + len(pool_bytes) + 4 if duplicate_owner_offset is None
                            else duplicate_owner_offset)
        symbols.append(_symbol(symbol_name_offsets[duplicate_owner_name], duplicate_offset, len(pool_bytes), 0x11, 2))
    symbols.append(_symbol(symbol_name_offsets["fixture.c"], 0, 0, 0x04, 0))
    symbol_bytes = b"".join(symbols)
    rela = b"".join(
        struct.pack(">IIi", index * 4 + relocation_bias, (owner_index << 8) | 109, 0)
        for index in range(len(function_names))
    )
    text = b"".join(struct.pack(">I", instruction_opcode << 26) for _ in function_names)
    pool = b"\0" * pool_offset + pool_bytes
    if duplicate_owner_name is not None:
        pool += b"\0" * 4 + pool_bytes
    sections = [
        {"name": "", "type": 0, "flags": 0, "align": 0, "content": b"", "link": 0, "info": 0, "entsize": 0},
        {"name": ".text", "type": 1, "flags": 0x6, "align": 4, "content": text, "link": 0, "info": 0, "entsize": 0},
        {"name": ".sdata2", "type": 1, "flags": pool_flags, "align": 4, "content": pool, "link": 0, "info": 0, "entsize": 0},
        {"name": ".shstrtab", "type": 3, "flags": 0, "align": 1, "content": shstrtab, "link": 0, "info": 0, "entsize": 0},
        {"name": ".symtab", "type": 2, "flags": 0, "align": 4, "content": symbol_bytes, "link": 5, "info": 1, "entsize": 16},
        {"name": ".strtab", "type": 3, "flags": 0, "align": 1, "content": strtab, "link": 0, "info": 0, "entsize": 0},
        {"name": ".rela.text", "type": 4, "flags": 0, "align": 4, "content": rela, "link": 4, "info": 1, "entsize": 12},
    ]
    offset = 0x100
    image = bytearray(offset)
    for section in sections:
        content = bytes(section["content"])
        offset = (offset + 3) & ~3
        section["offset"] = offset
        section["size"] = len(content)
        image.extend(b"\0" * (offset + len(content) - len(image)))
        image[offset:offset + len(content)] = content
        offset += len(content)
    shoff = (offset + 3) & ~3
    image.extend(b"\0" * (shoff + len(sections) * 40 - len(image)))
    ident = b"\x7fELF" + bytes((1, 2, 1)) + b"\0" * 9
    image[0:52] = struct.pack(">16sHHIIIIIHHHHHH", ident, 1, 20, 1, 0, 0, shoff, 0, 52, 0, 0, 40, len(sections), 3)
    for index, section in enumerate(sections):
        entry = shoff + index * 40
        image[entry:entry + 40] = struct.pack(
            ">IIIIIIIIII", section_name_offsets[section["name"]], int(section["type"]),
            int(section["flags"]), 0, int(section["offset"]), int(section["size"]),
            int(section["link"]), int(section["info"]), int(section["align"]), int(section["entsize"]),
        )
    path.write_bytes(bytes(image))


class RecoveryObjectInventoryTests(unittest.TestCase):
    def test_nontext_motion_requires_unchanged_function_and_relative_reference(self):
        def value(offset):
            return dict(allocated_sections={".data": dict(flags=2, sha256="same"),
                                           ".text": dict(flags=6)},
                functions={"f": dict(size=16, section=".text", offset=offset,
                    raw_sha256="body", relocations_sha256="refs")},
                allocated_relocations=[dict(section=".data", offset=0, type=1,
                    symbol=dict(name="f", binding=1, type=2, section=".text", value=offset, size=16),
                    addend=4, effective_target=dict(kind="section", section=".text", offset=offset+4))])
        base, candidate = value(100), value(104)
        self.assertTrue(inventory._nontext_code_motion(base, candidate))
        mixed_base, mixed_candidate = copy.deepcopy(base), copy.deepcopy(candidate)
        unchanged = copy.deepcopy(base["allocated_relocations"][0])
        unchanged["offset"] = 8
        for item in (mixed_base, mixed_candidate):
            item["allocated_relocations"].append(copy.deepcopy(unchanged))
        # One entry follows f; the raw-unchanged second entry now selects a
        # different offset within f. The entire bundle must fail qualification.
        self.assertFalse(inventory._nontext_code_motion(mixed_base, mixed_candidate))
        for fault in ("payload", "body", "refs", "missing", "addend", "site", "type", "target", "size", "symbol"):
            with self.subTest(fault=fault):
                other = copy.deepcopy(candidate)
                r = other["allocated_relocations"][0]
                if fault == "payload": other["allocated_sections"][".data"]["sha256"] = "new"
                elif fault in ("body", "refs"):
                    other["functions"]["f"]["raw_sha256" if fault == "body" else "relocations_sha256"] = "new"
                elif fault == "missing": other["allocated_relocations"] = []
                elif fault == "target": r["effective_target"]["offset"] += 4
                elif fault == "size": r["symbol"]["size"] += 4
                elif fault == "symbol": r["symbol"]["name"] = "unknown"
                else: r[{"site": "offset"}.get(fault, fault)] += 4
                self.assertFalse(inventory._nontext_code_motion(base, other))

    def test_named_storage_reverse_and_paired_exact_instruction_reference(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            target_path, candidate_path = Path(directory) / "t.o", Path(directory) / "c.o"
            _write_pool_elf(target_path, duplicate_owner_name="second", duplicate_owner_offset=8)
            _write_pool_elf(candidate_path, pool_offset=8, duplicate_owner_name="second", duplicate_owner_offset=0)
            target, candidate = inventory.inventory(target_path), inventory.inventory(candidate_path)
            report = inventory.compare(target, target, candidate, ["foo"])
        storage = report["storage_layout"]["candidate"]
        self.assertEqual(storage["sections"][0]["classification"], "reverse-permutation")
        self.assertEqual(storage["changed_owners_count"], 2)
        self.assertEqual(storage["reference_evidence_count"], 2)
        self.assertEqual(storage["reference_evidence"][0]["source_cause_class"], "upstream-object-storage-layout")
        self.assertEqual(storage["reference_evidence"][0]["target"]["offset"], 0)
        self.assertEqual(storage["reference_evidence"][0]["candidate"]["offset"], 8)
        self.assertFalse(storage["authority_advanced"])

    def test_storage_overlap_and_unpaired_names_never_infer_order(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            target_path, candidate_path = Path(directory) / "t.o", Path(directory) / "c.o"
            _write_pool_elf(target_path, duplicate_owner_name="second", duplicate_owner_offset=8)
            target = inventory.inventory(target_path)
            for extra in ({"duplicate_owner_name": "alias", "duplicate_owner_offset": 0},
                          {"duplicate_owner_name": "renamed", "duplicate_owner_offset": 8},
                          {"duplicate_owner_name": "pool_owner", "duplicate_owner_offset": 8}):
                _write_pool_elf(candidate_path, **extra)
                report = inventory.compare(target, target, inventory.inventory(candidate_path), [])
                storage = report["storage_layout"]["candidate"]
                self.assertEqual(storage["sections"][0]["classification"], "unknown")
                self.assertGreater(storage["unknowns_count"], 0)

    def test_storage_old_inventory_compatibility_and_semantic_hash_unchanged(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "t.o"
            _write_pool_elf(path)
            value = inventory.inventory(path)
        old = dict(value)
        del old["storage_layout"]
        report = inventory.compare(old, old, value, [])
        self.assertTrue(report["semantic_object_equal_target"])
        self.assertEqual(report["storage_layout"]["candidate"]["status"], "unknown")

    def test_storage_details_are_bounded_and_counted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "t.o"
            _write_pool_elf(path)
            value = inventory.inventory(path)
        candidate = dict(value)
        candidate["storage_layout"] = dict(value["storage_layout"])
        candidate["storage_layout"]["owners"] = [
            {**value["storage_layout"]["owners"][0], "name": f"renamed{i}", "value": i*4}
            for i in range(100)]
        report = inventory.compare(value, value, candidate, [])["storage_layout"]["candidate"]
        self.assertEqual(report["unknowns_count"], 101)
        self.assertEqual(len(report["unknowns"]), 64)
        self.assertEqual(report["unknowns_omitted"], 37)

    def test_unreferenced_alignment_tail_is_not_elf_equality(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "t.o"
            _write_pool_elf(path)
            value = inventory.inventory(path)
        symbols = [{"name": "scalar", "type": 1, "binding": 0, "section": ".bss", "value": 0, "size": 4}]
        section = {"name": ".bss", "type": 8, "flags": 3, "align": 8, "size": 8, "content": b""}
        left, right = dict(value), dict(value)
        left["allocated_sections"] = {".bss": {k: section[k] for k in ("type", "flags", "align", "size")}}
        left["allocated_sections"][".bss"]["content_sha256"] = hashlib.sha256(b"").hexdigest()
        right["allocated_sections"] = {".bss": {**left["allocated_sections"][".bss"], "size": 4}}
        left["storage_layout"] = inventory._storage_inventory(symbols, [section], [])
        right["storage_layout"] = inventory._storage_inventory(symbols, [{**section, "size": 4}], [])
        report = inventory._storage_compare(left, right)
        self.assertEqual(report["alignment_tail_candidates_count"], 1)
        self.assertFalse(report["alignment_tail_candidates"][0]["linker_equivalence_proven"])
        reference = {"section": ".text", "offset": 0,
                     "effective_target": {"section": ".bss", "offset": 4}}
        left["storage_layout"] = inventory._storage_inventory(symbols, [section], [reference])
        self.assertEqual(inventory._storage_compare(left, right)["alignment_tail_candidates_count"], 0)

    def test_pool_census_uses_actual_bytes_and_typed_consumers(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "pool.o"
            _write_pool_elf(path)
            result = inventory.pool_census(path)

        owner = next(item for item in result["owners"] if item["name"] == "pool_owner")
        self.assertEqual(owner["bytes"], "3f800000")
        self.assertTrue(owner["safe"])
        self.assertEqual(owner["consumer_count"], 2)
        self.assertEqual({use["type"] for use in owner["uses"]}, {"f32"})
        self.assertEqual({use["bytes"] for use in owner["uses"]}, {"3f800000"})

    def test_pool_census_flags_writable_owner(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "writable.o"
            _write_pool_elf(path, pool_flags=3)
            result = inventory.pool_census(path)

        owner = next(item for item in result["owners"] if item["name"] == "pool_owner")
        self.assertTrue(owner["writable"])
        self.assertFalse(owner["safe"])
        self.assertIn("writable_pool_owner", {item["issue"] for item in owner["issues"]})

    def test_pool_census_decodes_instruction_when_relocation_has_intra_word_bias(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "biased.o"
            _write_pool_elf(path, function_names=("foo",), relocation_bias=2)
            result = inventory.pool_census(path)

        owner = next(item for item in result["owners"] if item["name"] == "pool_owner")
        self.assertTrue(owner["safe"])
        self.assertEqual(owner["uses"][0]["instruction_offset"], 0)
        self.assertEqual(owner["uses"][0]["function_offset"], 2)
        self.assertEqual(owner["uses"][0]["type"], "f32")

    def test_pool_census_flags_actual_store_and_unknown_consumer(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store_path = Path(directory) / "store.o"
            unknown_path = Path(directory) / "unknown.o"
            _write_pool_elf(store_path, instruction_opcode=52)
            _write_pool_elf(unknown_path, instruction_opcode=14)
            store = inventory.pool_census(store_path)
            unknown = inventory.pool_census(unknown_path)

        store_owner = next(item for item in store["owners"] if item["name"] == "pool_owner")
        unknown_owner = next(item for item in unknown["owners"] if item["name"] == "pool_owner")
        self.assertIn("writable_use", {item["issue"] for item in store_owner["issues"]})
        self.assertIn("unknown_typed_use", {item["issue"] for item in unknown_owner["issues"]})

    def test_inventory_has_allocated_sections_and_function_relative_targets(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.o"
            _write_elf(path)
            result = inventory.inventory(path)

        self.assertGreater(result["object"]["size_bytes"], 0)
        self.assertEqual(result["allocated_sections"][".text"]["align"], 4)
        self.assertEqual(result["allocated_sections"][".bss"]["size"], 4)
        self.assertEqual(result["allocated_sections"][".bss"]["content_sha256"], hashlib.sha256(b"").hexdigest())
        self.assertEqual(
            result["functions"]["foo"]["relocations"][0]["effective_target"],
            {"kind": "function", "name": "bar", "offset": 0},
        )
        self.assertEqual(
            result["functions"]["foo"]["relocations"][1]["effective_target"],
            {"kind": "function", "name": "foo", "offset": 4},
        )
        self.assertEqual(len(result["functions"]["foo"]["physical_relocations"]), 2)

    def test_structure_is_parsed_once_per_object(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.o"
            _write_elf(path)
            with mock.patch.object(inventory, "_parse_elf_structure", wraps=inventory._parse_elf_structure) as parser:
                inventory.inventory(path)
            self.assertEqual(parser.call_count, 1)

    def test_stt_file_only_change_is_semantically_neutral(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            first = Path(directory) / "first.o"
            second = Path(directory) / "second.o"
            _write_elf(first, file_name="first.c")
            _write_elf(second, file_name="second.c")
            left, right = inventory.inventory(first), inventory.inventory(second)
        self.assertNotEqual(left["object"]["sha256"], right["object"]["sha256"])
        self.assertEqual(left["semantic_sha256"], right["semantic_sha256"])

    def test_allocated_bytes_and_undefined_relocation_are_semantic(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            first = Path(directory) / "first.o"
            data_changed = Path(directory) / "data.o"
            undefined_changed = Path(directory) / "undefined.o"
            _write_elf(first)
            _write_elf(data_changed, data=b"DIFF")
            _write_elf(undefined_changed, ext_name="other_external")
            baseline = inventory.inventory(first)
            changed_data = inventory.inventory(data_changed)
            changed_undefined = inventory.inventory(undefined_changed)
        self.assertNotEqual(baseline["semantic_sha256"], changed_data["semantic_sha256"])
        self.assertNotEqual(baseline["semantic_sha256"], changed_undefined["semantic_sha256"])
        comparison = inventory.compare(baseline, baseline, changed_undefined, ["foo"])
        self.assertTrue(comparison["allocated_nontext_changed"])
        self.assertTrue(comparison["allocated_nontext_relocations_changed"])

    def test_shifted_local_targets_stay_function_relative(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            target_path = Path(directory) / "target.o"
            shifted_path = Path(directory) / "shifted.o"
            _write_elf(target_path)
            shifted_text = b"\0\0\0\0" + b"\x60\x00\x00\x00\x4e\x80\x00\x20\x60\x00\x00\x00"
            _write_elf(
                shifted_path,
                text=shifted_text,
                foo_start=4,
                bar_start=12,
                local_value=8,
                rel_text_rows=[(4, 2, 10, 0), (8, 3, 1, 0)],
            )
            target = inventory.inventory(target_path)
            shifted = inventory.inventory(shifted_path)
        self.assertEqual(target["functions"]["foo"]["relocations"], shifted["functions"]["foo"]["relocations"])
        self.assertEqual(target["functions"]["foo"]["raw_sha256"], shifted["functions"]["foo"]["raw_sha256"])
        self.assertNotEqual(target["functions"]["foo"]["physical_sha256"], shifted["functions"]["foo"]["physical_sha256"])
        comparison = inventory.compare(target, target, shifted, ["foo"])
        row = comparison["functions"]["foo"]
        self.assertEqual(row["physical_diff_before"], 0)
        self.assertEqual(row["physical_diff_after"], 2)
        self.assertEqual(row["normalized_diff_before"], 0)
        self.assertEqual(row["normalized_diff_after"], 0)

    def test_local_symbol_alias_does_not_change_canonical_physical_identity(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            target_path = Path(directory) / "target.o"
            alias_path = Path(directory) / "alias.o"
            _write_elf(target_path, local_name="local_label")
            _write_elf(alias_path, local_name="equivalent_label")
            target = inventory.inventory(target_path)
            alias = inventory.inventory(alias_path)
        self.assertNotEqual(target["semantic_sha256"], alias["semantic_sha256"])
        self.assertEqual(target["functions"]["foo"]["physical_sha256"], alias["functions"]["foo"]["physical_sha256"])
        comparison = inventory.compare(target, target, alias, ["foo"])
        self.assertTrue(comparison["functions"]["foo"]["candidate_physical_exact"])

    def test_compare_reports_closed_physical_row_loss(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            target_path = Path(directory) / "target.o"
            base_path = Path(directory) / "base.o"
            candidate_path = Path(directory) / "candidate.o"
            for path in (target_path, base_path):
                _write_elf(path)
            _write_elf(candidate_path, rel_text_rows=[(4, 3, 1, 0)])
            target = inventory.inventory(target_path)
            base = inventory.inventory(base_path)
            candidate = inventory.inventory(candidate_path)
            comparison = inventory.compare(target, base, candidate, ["foo"])
        row = comparison["functions"]["foo"]
        self.assertTrue(row["base_physical_exact"])
        self.assertFalse(row["candidate_physical_exact"])
        self.assertEqual(row["physical_diff_before"], 0)
        self.assertEqual(row["physical_diff_after"], 1)
        self.assertEqual(row["closed_physical_row_losses"], [{"offset": 0, "type": 10, "candidate": "missing"}])
        self.assertEqual(row["closed_physical_row_loss_count"], 1)
        self.assertEqual(row["normalized_diff_after"], 1)
        self.assertEqual(row["closed_normalized_row_loss_count"], 1)

    def test_unsupported_rel_and_ambiguous_functions_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            rel_path = Path(directory) / "rel.o"
            overlap_path = Path(directory) / "overlap.o"
            _write_elf(rel_path, rela=False)
            _write_elf(overlap_path, overlap=True)
            with self.assertRaises(inventory.ObjectInventoryError):
                inventory.inventory(rel_path)
            with self.assertRaisesRegex(inventory.ObjectInventoryError, "overlapping function"):
                inventory.inventory(overlap_path)

    def test_malformed_object_and_invalid_focus_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            malformed = Path(directory) / "malformed.o"
            valid = Path(directory) / "valid.o"
            malformed.write_bytes(b"not an ELF")
            _write_elf(valid)
            with self.assertRaises(inventory.ObjectInventoryError):
                inventory.inventory(malformed)
            value = inventory.inventory(valid)
            with self.assertRaises(inventory.ObjectInventoryError):
                inventory.compare(value, value, value, ["missing"])


if __name__ == "__main__":
    unittest.main()
