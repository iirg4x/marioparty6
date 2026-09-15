"""Focused coverage for hash-bound empty REL evidence and snapshot assembly."""

from __future__ import annotations

import copy
import hashlib
import importlib.util
import json
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
import rel_metadata  # noqa: E402  (the test intentionally imports the shipped helper)


def _load_builder():
    spec = importlib.util.spec_from_file_location("empty_rel_snapshot_builder", TOOLS / "build_snapshot.py")
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


builder = _load_builder()


EMPTY_SPLITS = ".ctors type:rodata align:4\n.dtors type:rodata align:4\n"
EMPTY_SYMBOLS = "# no function symbols\n"
TEXT_SPLITS = ".text type:code align:4\n" + EMPTY_SPLITS
ZERO_ONLY_SPLITS = ".text type:code align:4\n.bss type:bss align:8\n"
MODULE_ID = "probeRel"


def make_empty_rel() -> bytes:
    """Build the nine-slot, 156-byte REL shape accepted by the inspector."""

    raw = bytearray(156)
    struct.pack_into(">I", raw, 0, 0xDEAD)  # arbitrary module id: no name allowlist
    struct.pack_into(">I", raw, 12, 9)
    struct.pack_into(">I", raw, 16, 76)
    struct.pack_into(">I", raw, 28, 3)
    struct.pack_into(">I", raw, 64, 4)
    struct.pack_into(">I", raw, 68, 1)
    struct.pack_into(">I", raw, 72, 156)
    for index in (2, 3):
        struct.pack_into(">II", raw, 76 + index * 8, 148 + (index - 2) * 4, 4)
    return bytes(raw)


EMPTY_RAW = make_empty_rel()
EMPTY_SHA1 = hashlib.sha1(EMPTY_RAW).hexdigest()


def inspected_target(raw: bytes = EMPTY_RAW, splits: str = EMPTY_SPLITS, symbols: str = EMPTY_SYMBOLS) -> dict:
    return rel_metadata.inspect_rel_target(raw, hashlib.sha1(raw).hexdigest(), splits, symbols)


def metadata_entry(
    target: dict | None,
    *,
    original_hash: str = EMPTY_SHA1,
    dtk_version: str = "0.9.2",
    code: int = 0,
    data: int = 0,
) -> dict:
    return {
        "originalHash": original_hash,
        "dtkVersion": dtk_version,
        "code": code,
        "data": data,
        "title": "Synthetic REL",
        "titleVerified": True,
        "category": "system",
        "aliases": [],
        "provenance": {"evidence": "test fixture", "sources": []},
        "target": target,
    }


class EmptyRelRepository:
    """Tiny committed repository sufficient for build_snapshot()."""

    def __init__(self, directory: Path) -> None:
        self.root = directory / "repo"
        for path in (
            "config/GP6E01",
            "config/dll/rels/m621dll",
            "config/dll/rels/probeRel",
            "src/game",
            "src/REL/m621dll",
            "include",
            "progress",
        ):
            (self.root / path).mkdir(parents=True, exist_ok=True)
        self._git("init", "-q")
        self._git("config", "user.email", "rel-test@example.invalid")
        self._git("config", "user.name", "REL Test")
        self.main_hash = "1" * 40
        self.m621_hash = "2" * 40
        (self.root / "configure.py").write_text(
            """Matching = True
NonMatching = False
class Config: pass
config = Config()
config.dtk_tag = "v0.9.2"
config.libs = [
    Object(Matching, "game/main.c"),
    Object(Matching, "REL/m621dll/m621.c"),
]
""",
            encoding="utf-8",
        )
        (self.root / "include/mgdata.inc").write_text("DLL_m621dll, MG_TYPE_4P\n", encoding="utf-8")
        (self.root / "progress/STATUS.md").write_text("fixture status\n", encoding="utf-8")
        (self.root / "src/game/main.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
        (self.root / "src/REL/m621dll/m621.c").write_text("int complete(void) { return 1; }\n", encoding="utf-8")
        (self.root / "config/GP6E01/symbols.txt").write_text(
            "main = .text:0x1000; // type:function size:0x4\n", encoding="utf-8"
        )
        (self.root / "config/GP6E01/splits.txt").write_text(
            ".text type:code align:4\n.bss type:bss align:8\n\n"
            "game/main.c:\n  .text start:0x1000 end:0x1004\n  .bss start:0x3000 end:0x3004\n",
            encoding="utf-8",
        )
        (self.root / "config/dll/rels/m621dll/symbols.txt").write_text(
            "complete = .text:0x2000; // type:function size:0x4\n", encoding="utf-8"
        )
        (self.root / "config/dll/rels/m621dll/splits.txt").write_text(
            ".text type:code align:4\n.bss type:bss align:8\n\n"
            "REL/m621dll/m621.c:\n  .text start:0x2000 end:0x2004\n  .bss start:0x4000 end:0x4004\n",
            encoding="utf-8",
        )
        self._write_config(include_probe=False)
        (self.root / "progress/GP6E01.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "version": "GP6E01",
                    "categories": {
                        "all": {"label": "Code", "code": {"matched": 8, "total": 8}, "data": {"matched": 8, "total": 8}},
                        "dol": {"label": "DOL", "code": {"matched": 4, "total": 4}, "data": {"matched": 4, "total": 4}},
                        "modules": {"label": "DLLs", "code": {"matched": 4, "total": 4}, "data": {"matched": 4, "total": 4}},
                    },
                }
            ),
            encoding="utf-8",
        )
        self._git("add", ".")
        self._git("commit", "-q", "-m", "base")
        self.base_commit = self._git("rev-parse", "HEAD")

    def _write_config(self, *, include_probe: bool, probe_hash: str = EMPTY_SHA1) -> None:
        content = (
            f"object: orig/GP6E01/sys/main.dol\nhash: {self.main_hash}\n"
            "symbols: config/GP6E01/symbols.txt\nsplits: config/GP6E01/splits.txt\n\n"
            "modules:\n"
            f"- object: orig/GP6E01/files/dll/m621dll.rel\nhash: {self.m621_hash}\n"
            "  symbols: config/dll/rels/m621dll/symbols.txt\n"
            "  splits: config/dll/rels/m621dll/splits.txt\n"
        )
        if include_probe:
            content += (
                f"- object: orig/GP6E01/files/dll/{MODULE_ID}.rel\nhash: {probe_hash}\n"
                f"  symbols: config/dll/rels/{MODULE_ID}/symbols.txt\n"
                f"  splits: config/dll/rels/{MODULE_ID}/splits.txt\n"
            )
        (self.root / "config/GP6E01/config.yml").write_text(content, encoding="utf-8")

    def add_probe(self, splits: str = EMPTY_SPLITS, symbols: str = EMPTY_SYMBOLS, raw: bytes = EMPTY_RAW) -> str:
        self._write_config(include_probe=True, probe_hash=hashlib.sha1(raw).hexdigest())
        (self.root / f"config/dll/rels/{MODULE_ID}/splits.txt").write_text(splits, encoding="utf-8")
        (self.root / f"config/dll/rels/{MODULE_ID}/symbols.txt").write_text(symbols, encoding="utf-8")
        self._git("add", ".")
        self._git("commit", "-q", "-m", "add probe REL")
        return self._git("rev-parse", "HEAD")

    def _git(self, *arguments: str) -> str:
        result = subprocess.run(
            ["git", "-C", str(self.root), *arguments],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
        )
        return result.stdout.strip()

    def metadata(self, *, target: dict | None, **entry_overrides: object) -> Path:
        document = {
            "schemaVersion": 1,
            "baselineCommit": self.base_commit,
            "modules": {
                "main.dol": metadata_entry(None, original_hash=self.main_hash, code=4, data=4),
                "m621dll": metadata_entry(None, original_hash=self.m621_hash, code=4, data=4),
                MODULE_ID: metadata_entry(target, **entry_overrides),
            },
        }
        path = self.root.parent / "metadata.json"
        path.write_text(json.dumps(document), encoding="utf-8")
        return path


class RelMetadataTests(unittest.TestCase):
    def test_inspector_and_helper_accept_arbitrary_empty_rel(self) -> None:
        target = inspected_target()
        self.assertEqual(target["fileBytes"], 156)
        self.assertEqual(target["header"]["moduleId"], 0xDEAD)
        self.assertEqual(target["sections"][2]["size"], 4)
        evidence = rel_metadata.verified_empty_evidence(target, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)
        self.assertEqual(
            evidence,
            {
                "classification": "verified-empty-rel",
                "targetSha1": EMPTY_SHA1,
                "fileBytes": 156,
                "linkerBytes": 8,
                "recoverableCodeBytes": 0,
                "recoverableDataBytes": 0,
                "functionCount": 0,
                "imports": 0,
                "entrypoints": 0,
            },
        )

    def test_structural_evidence_is_fail_closed(self) -> None:
        target = inspected_target()
        cases = {}

        cases["missing target"] = None
        cases["wrong hash"] = (target, "f" * 40, EMPTY_SPLITS, EMPTY_SYMBOLS)
        stale = copy.deepcopy(target)
        stale["fingerprint"]["splits"] = "0" * 64
        cases["stale fingerprint"] = (stale, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)

        nonzero_ctor = copy.deepcopy(target)
        nonzero_ctor["sections"][2]["allZero"] = False
        cases["nonzero ctor payload"] = (nonzero_ctor, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)
        imports = copy.deepcopy(target)
        imports["header"]["importOffset"] = 148
        imports["header"]["importBytes"] = 4
        cases["imports"] = (imports, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)
        entrypoints = copy.deepcopy(target)
        entrypoints["header"]["prologSection"] = 1
        entrypoints["header"]["prologOffset"] = 148
        cases["entrypoints"] = (entrypoints, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)
        bss = copy.deepcopy(target)
        bss["header"]["bssSection"] = 1
        bss["header"]["bssBytes"] = 4
        cases["BSS"] = (bss, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)

        text_target = inspected_target(splits=TEXT_SPLITS)
        cases["text section"] = (text_target, EMPTY_SHA1, TEXT_SPLITS, EMPTY_SYMBOLS)
        truncated = copy.deepcopy(target)
        truncated["sections"] = truncated["sections"][:-1]
        cases["truncated metadata"] = (truncated, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)

        for label, value in cases.items():
            with self.subTest(label=label):
                if value is None:
                    result = rel_metadata.verified_empty_evidence(None, EMPTY_SHA1, EMPTY_SPLITS, EMPTY_SYMBOLS)
                else:
                    result = rel_metadata.verified_empty_evidence(*value)
                self.assertIsNone(result)

    def test_snapshot_adds_empty_count_without_changing_existing_completion_or_percent(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            fixture = EmptyRelRepository(Path(directory))
            base_metadata = {
                "schemaVersion": 1,
                "baselineCommit": fixture.base_commit,
                "modules": {
                    "main.dol": metadata_entry(None, original_hash=fixture.main_hash, code=4, data=4),
                    "m621dll": metadata_entry(None, original_hash=fixture.m621_hash, code=4, data=4),
                },
            }
            base_path = Path(directory) / "base-metadata.json"
            base_path.write_text(json.dumps(base_metadata), encoding="utf-8")
            before = builder.build_snapshot(fixture.root, fixture.base_commit, base_path, generated_at="2026-01-01T00:00:00+00:00")

            commit = fixture.add_probe()
            target = inspected_target()
            after = builder.build_snapshot(
                fixture.root,
                commit,
                fixture.metadata(target=target),
                generated_at="2026-01-01T00:00:00+00:00",
            )
            probe = next(module for module in after["modules"] if module["id"] == MODULE_ID)
            m621_before = next(module for module in before["modules"] if module["id"] == "m621dll")
            m621_after = next(module for module in after["modules"] if module["id"] == "m621dll")
            self.assertEqual(probe["state"], "empty")
            self.assertEqual(probe["code"], {"matched": 0, "total": 0, "percent": None})
            self.assertEqual(probe["data"], {"matched": 0, "total": 0, "percent": None})
            self.assertEqual(probe["bss"], {"matched": 0, "total": 0, "percent": None})
            self.assertEqual(probe["owners"], [])
            self.assertEqual(probe["emptyEvidence"]["functionCount"], 0)
            self.assertEqual(after["summary"]["overall"]["counts"]["empty"], 1)
            self.assertEqual(after["summary"]["overall"]["counts"]["complete"], before["summary"]["overall"]["counts"]["complete"])
            self.assertEqual(after["summary"]["overall"]["code"], before["summary"]["overall"]["code"])
            self.assertEqual(after["summary"]["overall"]["data"], before["summary"]["overall"]["data"])
            self.assertEqual(m621_after["state"], m621_before["state"])
            self.assertEqual(m621_after["code"], m621_before["code"])
            self.assertEqual(m621_after["data"], m621_before["data"])
            self.assertEqual(after["summary"]["rel"]["counts"]["empty"], 1)
            self.assertEqual(after["summary"]["rel"]["counts"]["complete"], before["summary"]["rel"]["counts"]["complete"])

    def test_zero_only_and_metadata_mismatches_remain_unavailable(self) -> None:
        target = inspected_target()
        base_record = {
            "id": MODULE_ID,
            "hash": EMPTY_SHA1,
            "symbols": "symbols",
            "splits": "splits",
        }
        for label, entry, splits, symbols in (
            ("missing target", metadata_entry(None), EMPTY_SPLITS, EMPTY_SYMBOLS),
            ("zero-only", metadata_entry(None), ZERO_ONLY_SPLITS, EMPTY_SYMBOLS),
            ("wrong hash", metadata_entry(target, original_hash="f" * 40), EMPTY_SPLITS, EMPTY_SYMBOLS),
            ("wrong DTK", metadata_entry(target, dtk_version="0.9.1"), EMPTY_SPLITS, EMPTY_SYMBOLS),
            ("nonzero recoverable budget", metadata_entry(target, code=4), EMPTY_SPLITS, EMPTY_SYMBOLS),
        ):
            with self.subTest(label=label):
                record = dict(base_record)
                files = {"splits": splits, "symbols": symbols}
                entry_key = MODULE_ID.lower()
                result = builder._assemble_module(
                    record,
                    files,
                    set(),
                    {},
                    {entry_key: entry},
                    set(),
                    "0.9.2",
                )
                if label == "nonzero recoverable budget":
                    self.assertEqual(result["state"], "notRecovered")
                    self.assertEqual(result["code"], {"matched": 0, "total": 4, "percent": 0.0})
                else:
                    self.assertEqual(result["state"], "unavailable")
                self.assertIsNone(result["emptyEvidence"])

    def test_source_owners_or_function_records_block_empty_classification(self) -> None:
        target = inspected_target()
        record = {"id": MODULE_ID, "hash": EMPTY_SHA1, "symbols": "symbols", "splits": "splits"}
        owner_result = builder._assemble_module(
            record,
            {"splits": EMPTY_SPLITS + "probe.c:\n", "symbols": EMPTY_SYMBOLS},
            set(),
            {},
            {MODULE_ID.lower(): metadata_entry(inspected_target(splits=EMPTY_SPLITS + "probe.c:\n"))},
            set(),
            "0.9.2",
        )
        self.assertEqual(owner_result["state"], "unavailable")
        self.assertIsNone(owner_result["emptyEvidence"])
        self.assertTrue(owner_result["owners"])

        function_splits = EMPTY_SPLITS
        function_symbols = "fn = .text:0x0000; // type:function size:0x4\n"
        function_target = inspected_target(symbols=function_symbols)
        function_result = builder._assemble_module(
            record,
            {"splits": function_splits, "symbols": function_symbols},
            set(),
            {},
            {MODULE_ID.lower(): metadata_entry(function_target)},
            set(),
            "0.9.2",
        )
        self.assertEqual(function_result["state"], "unavailable")
        self.assertIsNone(function_result["emptyEvidence"])
        self.assertTrue(function_result["owners"])


if __name__ == "__main__":
    unittest.main()
