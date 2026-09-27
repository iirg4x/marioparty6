"""Public REL-link tooling stays independent of private recovery modules."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import struct
import sys
import tempfile
import unittest

from tools import rel_link


PROJECT_ROOT = Path(__file__).resolve().parents[2]
LABEL = "lbl_1_rodata_2F0"
FUNCTION = "fn_provider"
POOL = bytes.fromhex("0000000042b40000")
CODE = bytes.fromhex("3c60000038630000c0230000")
DEST = 0x2F0


def _isolated_tools(root: Path) -> Path:
    """Install only the three ordinary production modules under a temp package."""
    tools = root / "tools"
    tools.mkdir()
    source = PROJECT_ROOT / "tools"
    for name in ("__init__.py", "project.py", "ninja_syntax.py", "rel_link.py"):
        shutil.copyfile(source / name, tools / name)
    return tools


def _strings(names: list[str]) -> tuple[bytes, dict[str, int]]:
    data = bytearray(b"\0")
    offsets = {"": 0}
    for name in names:
        offsets[name] = len(data)
        data.extend(name.encode("ascii") + b"\0")
    return bytes(data), offsets


def _write_elf(path: Path, *, role: str, pool: bytes = POOL,
               duplicate_target_label: bool = False, native_uses: bool = True) -> Path:
    """A small synthetic big-endian PPC ELF; no retail or recovery fixture input."""
    if len(pool) != len(POOL):
        raise ValueError("synthetic pool must remain eight bytes")
    section_names = ["", ".text", ".rodata", ".shstrtab", ".symtab", ".strtab", ".rela.text"]
    shstr, sh_offsets = _strings(section_names[1:])
    # Local symbols precede globals. The linked relocation targets the section
    # symbol, while the native object uses a compiler-local pool owner.
    symbols = [("", 0, 0, 0, 0), ("", 0, 0, 3, 2)]
    if role == "native":
        symbols += [("@local_pool", 0, len(pool), 1, 2), (FUNCTION, 0, len(CODE), 2, 1)]
        relocation_index, relocation_addend = 2, 0
        rodata = pool
    elif role in ("target", "member", "linked"):
        symbols.append((FUNCTION, 0, len(CODE), 2, 1))
        symbols.append((LABEL, 0, len(pool) if role == "target" else 0,
                        0x11, 2 if role == "target" else 0))
        if role == "target" and duplicate_target_label:
            symbols.append(symbols[-1])
        relocation_index, relocation_addend = (1, DEST) if role == "linked" else (3, 0)
        if role == "target":
            rodata = pool
        elif role == "member":
            rodata = b""
        else:
            content = bytearray([0xCC]) * (DEST + 2 * len(pool))
            content[DEST:DEST + len(pool)] = pool
            content[DEST + len(pool):DEST + 2 * len(pool)] = pool
            rodata = bytes(content)
    else:
        raise ValueError(role)

    strtab, str_offsets = _strings([row[0] for row in symbols if row[0]])
    symtab = b"".join(struct.pack(">IIIBBH", str_offsets.get(name, 0), value, size, info, 0, section)
                      for name, value, size, info, section in symbols)
    relocations = b""
    if role != "native" or native_uses:
        relocations = b"".join(struct.pack(">IIi", offset, (relocation_index << 8) | kind,
                                            relocation_addend)
                               for offset, kind in ((2, 6), (6, 4)))
    sections = [
        ("", 0, 0, 0, b"", 0, 0, 0),
        (".text", 1, 6, 4, CODE, 0, 0, 0),
        (".rodata", 1, 2, 4, rodata, 0, 0, 0),
        (".shstrtab", 3, 0, 1, shstr, 0, 0, 0),
        (".symtab", 2, 0, 4, symtab, 5, 3, 16),
        (".strtab", 3, 0, 1, strtab, 0, 0, 0),
        (".rela.text", 4, 0, 4, relocations, 4, 1, 12),
    ]
    image = bytearray(52)
    section_headers = []
    for name, kind, flags, alignment, content, link, info, entry_size in sections:
        offset = (len(image) + max(1, alignment) - 1) & ~(max(1, alignment) - 1)
        image.extend(b"\0" * (offset - len(image)))
        image.extend(content)
        section_headers.append((sh_offsets.get(name, 0), kind, flags, 0, offset,
                                len(content), link, info, alignment, entry_size))
    shoff = (len(image) + 3) & ~3
    image.extend(b"\0" * (shoff - len(image) + len(sections) * 40))
    ident = b"\x7fELF" + bytes((1, 2, 1)) + b"\0" * 9
    image[:52] = struct.pack(">16sHHIIIIIHHHHHH", ident, 1, 20, 1, 0, 0, shoff,
                             0, 52, 0, 0, 40, len(sections), 3)
    for index, header in enumerate(section_headers):
        struct.pack_into(">IIIIIIIIII", image, shoff + index * 40, *header)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(image)
    return path


def _scalar_fixture(root: Path, **options: object) -> tuple[Path, Path, Path, dict]:
    native = _write_elf(root / "native.o", role="native", **options)
    target = _write_elf(root / "target.plf", role="target", **options)
    linked = _write_elf(root / "owner.preplf", role="linked", **options)
    binding = {
        "symbol": LABEL,
        "section": ".rodata",
        "native_object": str(native),
        "target_object": str(target),
        "native_offset": 0,
        "linked_offset": DEST,
        "size": len(POOL),
    }
    return native, target, linked, binding


_PROJECT_PROBE = r'''
import importlib.util
from pathlib import Path
import sys

sys.path.insert(0, sys.argv[1])
for name in ("tools.rel_owner_batch", "tools.crack_evidence_bundle", "tools.crack_contract"):
    assert importlib.util.find_spec(name) is None, name

from tools.project import Object, ProjectConfig, generate_build_ninja

kind = sys.argv[2]
provider_target = Path(sys.argv[3])
member_target = Path(sys.argv[4])
source = "REL/sample/provider.c"
member = "REL/sample/member.c"
config = ProjectConfig()
config.build_dir = Path("build")
config.src_dir = Path("src")
config.tools_dir = Path("tools")
config.check_sha_path = Path("version.sha1")
config.config_path = Path("configure.py")
config.ldflags = []
config.linker_version = "GC/2.6"
config.version = "TEST"
config.dtk_tag = "test"
config.compilers_tag = "test"
config.binutils_tag = "test"
config.sjiswrap_tag = "test"
config.libs = [{"lib": "sample", "mw_version": "GC/1.3.2", "cflags": ["-O0,p"],
                "src_dir": Path("src"), "objects": [Object(True, source, source="provider.c")]}]
if kind != "plain":
    group = {"source": source, "text_members": [{"source": member,
             "section": ".text.transition"}]}
    if kind == "pool_group":
        group.update(symbol="lbl_1_rodata_2F0", section=".rodata", native_offset=0,
                     linked_offset=0x2F0, size=8)
    config.rel_pool_exports = {"sample": [group]}
units = [{"name": source, "object": str(provider_target), "autogenerated": False}]
if kind != "plain":
    units.append({"name": member, "object": str(member_target), "autogenerated": True})
module = {"name": "sample", "module_id": 1, "ldscript": "ldscript.lcf",
          "entry": None, "units": units}
build = {"name": "main", "module_id": 0, "ldscript": "ldscript.lcf",
         "entry": None, "units": [], "modules": [module],
         "links": [{"modules": ["sample"]}]}
generate_build_ninja(config, build)
for name in ("tools.rel_owner_batch", "tools.crack_evidence_bundle", "tools.crack_contract"):
    assert name not in sys.modules, name
'''


class RelLinkExtractionTests(unittest.TestCase):
    def test_isolated_project_configuration_preserves_pool_group_and_plain_edges(self):
        for kind in ("pool_group", "group_only", "plain"):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory(prefix="public rel link ") as tmp:
                root = Path(tmp)
                _isolated_tools(root)
                (root / "src").mkdir()
                (root / "src/provider.c").write_text("void fn_provider(void) {}\n", encoding="utf-8")
                (root / "ldscript.lcf").write_text(
                    "SECTIONS\n{\n    GROUP:\n    {\n"
                    "        .text ALIGN(0x4):{}\n        .rodata ALIGN(0x8):{}\n"
                    "    }\n}\n", encoding="utf-8")
                provider_target = _write_elf(root / "fixture/provider-target.o", role="target")
                member_target = _write_elf(root / "fixture/member-target.o", role="member")
                proc = subprocess.run(
                    [sys.executable, "-I", "-c", _PROJECT_PROBE, str(root), kind,
                     str(provider_target), str(member_target)],
                    cwd=root, capture_output=True, text=True, timeout=30,
                )
                self.assertEqual(proc.returncode, 0, proc.stderr)
                ninja = (root / "build.ninja").read_text(encoding="utf-8").replace("\\", "/")
                self.assertNotIn("rel_owner_batch.py", ninja)
                self.assertNotIn("crack_evidence_bundle.py", ninja)
                self.assertNotIn("crack_contract.py", ninja)
                group_lcf = root / "ldscript.source-groups.lcf"
                bindings = root / "build/TEST/sample/sample.pool-exports.json"
                if kind == "pool_group":
                    self.assertIn("tools/rel_link.py", ninja)
                    self.assertIn("sample.plf: resolve_pool_exports", ninja)
                    self.assertTrue(group_lcf.is_file())
                    self.assertEqual(json.loads(bindings.read_text(encoding="utf-8"))[0]["symbol"], LABEL)
                elif kind == "group_only":
                    self.assertTrue(group_lcf.is_file())
                    self.assertNotIn("sample.plf: resolve_pool_exports", ninja)
                    self.assertFalse(bindings.exists())
                else:
                    self.assertFalse(group_lcf.exists())
                    self.assertNotIn("sample.plf: resolve_pool_exports", ninja)
                    self.assertFalse(bindings.exists())

    def test_isolated_direct_cli_only_edits_expected_symbol_definition(self):
        with tempfile.TemporaryDirectory(prefix="public rel link cli ") as tmp:
            root = Path(tmp)
            tools = _isolated_tools(root)
            _, _, linked, binding = _scalar_fixture(root)
            bindings_path = root / "pool bindings.json"
            bindings_path.write_text(json.dumps([binding]), encoding="utf-8")
            output = root / "owner.bound.plf"
            original = linked.read_bytes()
            proc = subprocess.run(
                [sys.executable, "-I", str(tools / "rel_link.py"), "resolve-pool-exports",
                 "--input", str(linked), "--output", str(output), "--bindings", str(bindings_path)],
                cwd=root, capture_output=True, text=True, timeout=30,
            )
            self.assertEqual(proc.returncode, 0, proc.stderr)
            result = json.loads(proc.stdout)
            self.assertEqual(linked.read_bytes(), original)
            record = result["resolved_pool_exports"][0]
            self.assertEqual((record["symbol"], record["offset"], record["size"],
                              record["native_pool_uses"]), (LABEL, DEST, len(POOL), 2))
            self.assertTrue(result["code_data_relocations_unchanged"])
            # Derive the sole permitted ELF edit from the fixture's fixed layout,
            # independently of the production parser and compatibility exports.
            shoff = struct.unpack_from(">I", original, 32)[0]
            symtab_offset = struct.unpack_from(">I", original, shoff + 4 * 40 + 16)[0]
            label_offset = symtab_offset + 3 * 16
            self.assertEqual(struct.unpack_from(">II", original, label_offset + 4), (0, 0))
            self.assertEqual(struct.unpack_from(">H", original, label_offset + 14)[0], 0)
            expected = bytearray(original)
            struct.pack_into(">II", expected, label_offset + 4, DEST, len(POOL))
            struct.pack_into(">H", expected, label_offset + 14, 2)  # .rodata section index
            self.assertEqual(output.read_bytes(), bytes(expected))
            self.assertEqual(result["input_sha256"], hashlib.sha256(original).hexdigest())
            self.assertEqual(result["output_sha256"], hashlib.sha256(expected).hexdigest())

    def test_truncated_physical_sections_cannot_prove_equal_empty_payloads(self):
        for offset_delta in (-4, 256):
            with self.subTest(offset_delta=offset_delta), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                native, target, linked, binding = _scalar_fixture(root)
                for path in (native, target, linked):
                    data = bytearray(path.read_bytes())
                    shoff = struct.unpack_from(">I", data, 32)[0]
                    struct.pack_into(">I", data, shoff + 2 * 40 + 16,
                                     len(data) + offset_delta)
                    path.write_bytes(data)
                original = linked.read_bytes()
                output = root / "rejected.plf"
                with self.assertRaisesRegex(ValueError, "payload extends beyond file"):
                    rel_link.resolve_pool_exports(linked, output, [binding])
                self.assertFalse(output.exists())
                self.assertEqual(linked.read_bytes(), original)

    def test_nobits_section_does_not_require_a_file_payload(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = _write_elf(Path(tmp) / "bss.o", role="native")
            data = bytearray(path.read_bytes())
            shoff = struct.unpack_from(">I", data, 32)[0]
            struct.pack_into(">I", data, shoff + 2 * 40 + 4, 8)
            struct.pack_into(">I", data, shoff + 2 * 40 + 16, len(data) + 256)
            path.write_bytes(data)
            parsed = rel_link._parse_elf_structure(path)
            self.assertEqual(parsed["sections"][2]["type"], 8)
            self.assertEqual(parsed["sections"][2]["size"], len(POOL))

    def test_rejections_preserve_input_and_never_publish_output(self):
        cases = {
            "malformed_symbol": {"symbol": "invalid!symbol"},
            "ambiguous": {"options": {"duplicate_target_label": True}},
            "overlap": {"bindings": "duplicate"},
            "changed_native_bytes": {"native_pool": b"badbytes"},
            "missing_native_references": {"options": {"native_uses": False}},
            "wrong_linked_reference": {"linked_offset": DEST + len(POOL)},
        }
        for name, change in cases.items():
            with self.subTest(name=name), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                native, _, linked, binding = _scalar_fixture(root, **change.get("options", {}))
                if "native_pool" in change:
                    _write_elf(native, role="native", pool=change["native_pool"])
                if "symbol" in change:
                    binding["symbol"] = change["symbol"]
                if "linked_offset" in change:
                    binding["linked_offset"] = change["linked_offset"]
                bindings = [binding, binding] if change.get("bindings") == "duplicate" else [binding]
                original = linked.read_bytes()
                output = root / "rejected.plf"
                with self.assertRaises(ValueError):
                    rel_link.resolve_pool_exports(linked, output, bindings)
                self.assertFalse(output.exists())
                self.assertEqual(linked.read_bytes(), original)


if __name__ == "__main__":
    unittest.main()
