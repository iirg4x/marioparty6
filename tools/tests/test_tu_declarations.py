import json
import tempfile
import unittest
from pathlib import Path

from tools.check_tu_declarations import (
    _declarator_symbol,
    check_policy,
    explicit_externs,
)


class DeclarationLexerTests(unittest.TestCase):
    def test_ignores_comments_strings_and_function_bodies(self):
        source = r'''
            // extern int comment_only;
            const char *text = "extern int string_only;";
            void f(void) { extern int block_local; }
            extern int real_object[4];
        '''
        self.assertEqual(
            [(item["symbol"], item["kind"]) for item in explicit_externs(source)],
            [("real_object", "object")],
        )

    def test_multiline_declaration_cannot_evade_gate(self):
        source = "extern\nconst u32\nbogus_table[];\n"
        declaration = explicit_externs(source)[0]
        self.assertEqual(declaration["symbol"], "bogus_table")
        self.assertEqual(declaration["line"], 1)

    def test_function_pointer_is_an_object(self):
        symbol, kind = _declarator_symbol("extern void (*callback)(void);")
        self.assertEqual((symbol, kind), ("callback", "object"))


class OwnershipGateTests(unittest.TestCase):
    def _fixture(self, root: Path) -> Path:
        files = {
            "include/REL/module.h": "void shared(void);\n",
            "include/REL/globals.h": "extern int table[1];\n",
            "src/a.c": (
                '#include "REL/module.h"\n'
                '#include "REL/globals.h"\n'
                "typedef void (*VoidFunc)(void);\n"
                "extern const VoidFunc _ctors[];\n"
                "int table[1];\n"
            ),
            "src/b.c": '#include "REL/module.h"\nvoid shared(void) {}\n',
            "config/V/main_symbols.txt": "dol_only = .text:0; // type:function\n",
            "config/V/rel_symbols.txt": (
                "table = .bss:0; // type:object\n"
                "shared = .text:0; // type:function\n"
                "_ctors = .ctors:0; // type:label\n"
            ),
        }
        for relative, content in files.items():
            path = root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        policy = {
            "module_header": "include/REL/module.h",
            "globals_header": "include/REL/globals.h",
            "header_includes": {
                "include/REL/module.h": [],
                "include/REL/globals.h": [],
            },
            "dol_symbol_files": ["config/V/main_symbols.txt"],
            "rel_symbol_files": ["config/V/rel_symbols.txt"],
            "sources": [
                {
                    "path": "src/a.c",
                    "required_includes": ["REL/module.h", "REL/globals.h"],
                    "allowed_includes": ["REL/module.h", "REL/globals.h"],
                    "allowed_externs": {
                        "_ctors": {
                            "declaration": "extern const VoidFunc _ctors[];",
                            "authority": "rel",
                        }
                    },
                },
                {
                    "path": "src/b.c",
                    "required_includes": ["REL/module.h"],
                    "allowed_includes": ["REL/module.h"],
                    "allowed_externs": {},
                },
            ],
            "module_functions": ["shared"],
            "owned_functions": [],
            "mirrored_prototypes": [],
        }
        policy_path = root / "config/V/tu_declarations.json"
        policy_path.write_text(json.dumps(policy), encoding="utf-8")
        return policy_path

    def test_valid_contract_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            policy = self._fixture(Path(directory))
            self.assertEqual(check_policy(policy), [])

    def test_unowned_extern_fails_even_when_target_backed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8") + "extern void bogus(void);\n",
                encoding="utf-8",
            )
            symbols = root / "config/V/main_symbols.txt"
            symbols.write_text(
                symbols.read_text(encoding="utf-8")
                + "bogus = .text:4; // type:function\n",
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("unowned extern bogus" in error for error in errors))

    def test_header_extern_requires_one_owner_definition(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8").replace("int table[1];\n", ""),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("table has 0 owner definitions" in error for error in errors))

    def test_initializer_reference_is_not_an_owner_definition(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8").replace(
                    "int table[1];\n", "int *alias = table;\n"
                ),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("table has 0 owner definitions" in error for error in errors))

    def test_plain_unowned_prototype_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8") + "void bogus(void);\n",
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("unowned prototype bogus" in error for error in errors))

    def test_module_function_requires_one_definition(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/b.c"
            source.write_text('#include "REL/module.h"\n', encoding="utf-8")
            errors = check_policy(policy)
            self.assertTrue(any("module function shared has 0 definitions" in error for error in errors))

    def test_comma_packed_extern_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8").replace(
                    "extern const VoidFunc _ctors[];",
                    "extern const VoidFunc bogus, _ctors[];",
                ),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("comma-packed extern" in error for error in errors))

    def test_public_header_cannot_add_import_prototype(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            header = root / "include/REL/module.h"
            header.write_text(
                header.read_text(encoding="utf-8") + "void bogus(void);\n",
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("extra=['bogus']" in error for error in errors))

    def test_source_cannot_add_an_unreviewed_header(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                '#include "fake_imports.h"\n' + source.read_text(encoding="utf-8"),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("extra=['fake_imports.h']" in error for error in errors))

    def test_tag_forward_declaration_is_not_an_object_definition(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8").replace(
                    "int table[1];\n", "struct table;\n"
                ),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("table has 0 owner definitions" in error for error in errors))

    def test_public_header_cannot_smuggle_a_nested_include(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            header = root / "include/REL/module.h"
            header.write_text(
                '#include "fake_imports.h"\n' + header.read_text(encoding="utf-8"),
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("extra=['fake_imports.h']" in error for error in errors))

    def test_duplicate_authenticated_extern_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8")
                + "extern const VoidFunc _ctors[];\n",
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("duplicate extern _ctors" in error for error in errors))

    def test_phase_two_line_splice_cannot_hide_extern(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            policy = self._fixture(root)
            source = root / "src/a.c"
            source.write_text(
                source.read_text(encoding="utf-8") + "ext\\\nern int bogus;\n",
                encoding="utf-8",
            )
            errors = check_policy(policy)
            self.assertTrue(any("unowned extern bogus" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
