import json
import sqlite3
import tempfile
import unittest
from contextlib import closing
from pathlib import Path

from tools.recovery_core import (
    Function,
    build_index,
    context_pack,
    load,
    parse_functions,
    quality_findings,
    validate_data,
)


class RecoveryWorkflowTests(unittest.TestCase):
    def fixture(self, root: Path) -> None:
        files = {
            "src/a.c": (
                '#include "a.h"\n'
                "// volatile ignored\n"
                "int fn_1_20(int x)\n"
                "{\n"
                "    return x + 1;\n"
                "}\n"
            ),
            "include/a.h": "int fn_1_20(int x);\n",
            "docs/evidence.md": "# Evidence\n",
            "config/recovery/project.json": json.dumps(
                {
                    "schema_version": 1,
                    "project": "test",
                    "owner_globs": ["config/recovery/owners/*.json"],
                    "files": {
                        "names": "config/recovery/names.json",
                        "exceptions": "config/recovery/exceptions.json",
                        "compiler_patterns": "config/recovery/compiler_patterns.json",
                    },
                    "dimensions": {
                        "binary": ["partial", "exact"],
                        "source_shape": ["plausible", "evidence_backed"],
                        "semantics": ["partial", "recovered"],
                        "naming": ["address_only", "evidence_backed"],
                        "data": ["typed_partial", "typed"],
                    },
                    "confidence_levels": ["unknown", "high", "confirmed"],
                    "evidence_hierarchy": [
                        {"id": "target_binary", "rank": 1}
                    ],
                    "agent_contract": ["Recover source, not only bytes."],
                    "acceptance_criteria": ["No exact regression."],
                }
            ),
            "config/recovery/owners/a.json": json.dumps(
                {
                    "schema_version": 1,
                    "id": "REL:test:a",
                    "module": "test",
                    "source": "src/a.c",
                    "summary": "fixture owner",
                    "status": {
                        "binary": "exact",
                        "source_shape": "evidence_backed",
                        "semantics": "partial",
                        "naming": "address_only",
                        "data": "typed",
                    },
                    "symbols": [
                        {"symbol": "fn_1_20", "stable_id": "test:0x20"}
                    ],
                    "evidence": [
                        {
                            "kind": "target_binary",
                            "confidence": "confirmed",
                            "accepted": True,
                            "summary": "exact",
                            "reference": "docs/evidence.md",
                        }
                    ],
                    "constraints": [],
                    "debt": [{"kind": "name", "summary": "address name"}],
                    "context": {"reports": []},
                }
            ),
            "config/recovery/names.json": json.dumps(
                {
                    "schema_version": 1,
                    "names": [
                        {
                            "stable_id": "test:0x20",
                            "owner": "REL:test:a",
                            "current_symbol": "fn_1_20",
                            "proposed_name": None,
                            "status": "unresolved",
                            "confidence": "unknown",
                            "summary": "unknown",
                        }
                    ],
                }
            ),
            "config/recovery/exceptions.json": json.dumps(
                {"schema_version": 1, "exceptions": []}
            ),
            "config/recovery/compiler_patterns.json": json.dumps(
                {"schema_version": 1, "patterns": []}
            ),
        }
        for relative, content in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")

    def test_parser_finds_file_scope_function(self):
        value = parse_functions(
            "int table[] = {1};\n"
            "void f(int x)\n"
            "{\n"
            " if (x) {}\n"
            "}\n"
        )
        self.assertEqual(value, [Function("f", "void f(int x)", 2, 5)])

    def test_metadata_index_and_context(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            data = load(root)
            database = root / "build/context/index.sqlite"
            counts = build_index(data, database)
            self.assertEqual(counts["functions"], 1)
            with closing(sqlite3.connect(database)) as connection:
                stable_id = connection.execute(
                    "SELECT stable_id FROM functions"
                ).fetchone()[0]
            self.assertEqual(stable_id, "test:0x20")
            packet = context_pack(
                data,
                "function",
                "fn_1_20",
                owner_id="REL:test:a",
                budget=2000,
            )
            self.assertIn("Recover source, not only bytes", packet)
            self.assertIn("test:0x20", packet)

    def test_invalid_dimension_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            owner = root / "config/recovery/owners/a.json"
            value = json.loads(owner.read_text())
            value["status"]["semantics"] = "pretend"
            owner.write_text(json.dumps(value))
            data = load(root, validate=False)
            self.assertTrue(
                any("invalid semantics" in item for item in validate_data(data))
            )

    def test_quality_ignores_comments_and_honours_exception(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text("// volatile fake;\nvolatile int real;\n")
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(
                [(item["line"], item["rule"]) for item in findings],
                [(2, "volatile")],
            )
            data["exceptions"] = [
                {
                    "id": "ok",
                    "classification": "authenticated",
                    "path": "src/new.c",
                    "rules": ["volatile"],
                    "rationale": "test",
                }
            ]
            self.assertEqual(quality_findings(data, full=True), [])

    def test_quality_preserves_preprocessor_directives(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text("#pragma options align=power\nint value;\n")
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(findings[0]["rule"], "compiler_pragma")

    def test_quality_ignores_sdk_pad_macros_but_flags_padding_names(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text(
                "int input = PAD_BUTTON_A | PAD_TRIGGER_R;\n"
                "int align = HUWIN_ATTR_ALIGN_CENTER;\n"
                "int pad_0;\n"
                "int filesel_bss_pad_338;\n",
                encoding="utf-8",
            )
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(
                [(item["line"], item["rule"]) for item in findings],
                [
                    (3, "synthetic_padding"),
                    (4, "synthetic_padding"),
                ],
            )

    def test_quality_flags_self_assignment(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text(
                "int i;\n"
                "i = i;\n"
                "point->x = x;\n",
                encoding="utf-8",
            )
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(
                [(item["line"], item["rule"]) for item in findings],
                [(2, "self_assignment")],
            )

    def test_empty_exception_rules_do_not_blanket_suppress(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text("volatile int real;\n")
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            data["exceptions"] = [
                {
                    "id": "unrelated",
                    "classification": "authenticated",
                    "path": "src/new.c",
                    "rules": [],
                    "rationale": "unrelated source-shape evidence",
                }
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(findings[0]["classification"], "unreviewed")

    def test_quality_flags_raw_hex_literals_but_ignores_comments_and_strings(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/new.c"
            source.write_text(
                "enum { NAMED_VALUE = 42 };\n"
                "int raw = 0x2A;\n"
                "const char *text = \"0x2A\";\n"
                "// 0x2A\n",
                encoding="utf-8",
            )
            data = load(root)
            data["owners"] = [
                {**data["owners"][0], "source": "src/new.c"}
            ]
            findings = quality_findings(data, full=True)
            self.assertEqual(
                [(item["line"], item["rule"]) for item in findings],
                [(2, "raw_hex_literal")],
            )


if __name__ == "__main__":
    unittest.main()
