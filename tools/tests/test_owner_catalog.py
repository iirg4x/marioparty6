import tempfile
import unittest
from pathlib import Path

from tools.owner_catalog import build_catalog, find_owner


class OwnerCatalogTests(unittest.TestCase):
    def test_operational_dependency_edges(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src/game").mkdir(parents=True)
            (root / "src/REL/foo").mkdir(parents=True)
            (root / "include").mkdir()
            (root / "include/common.h").write_text(
                "#pragma once\n", encoding="utf-8"
            )
            (root / "src/game/a.c").write_text(
                '#include "common.h"\n'
                "extern int shared;\n"
                "int helper(void);\n"
                "int caller(void)\n"
                "{\n"
                "    return helper() + shared;\n"
                "}\n",
                encoding="utf-8",
            )
            (root / "src/REL/foo/b.c").write_text(
                "int shared;\n"
                "int helper(void)\n"
                "{\n"
                "    return shared;\n"
                "}\n",
                encoding="utf-8",
            )
            (root / "src/game/canonical_cp.cp").write_text(
                "int CanonicalCp(void)\n{\n    return 1;\n}\n",
                encoding="utf-8",
            )
            (root / "src/game/canonical_cpp.cpp").write_text(
                "int CanonicalCpp(void)\n{\n    return 2;\n}\n",
                encoding="utf-8",
            )
            (root / "configure.py").write_text(
                'main = [Object(Matching, "game/a.c")]\n'
                'rel = Rel("foo", objects=[Object(NonMatching, "REL/foo/b.c")])\n',
                encoding="utf-8",
            )
            catalog = build_catalog(root)
            identifiers = {owner["id"] for owner in catalog["owners"]}
            self.assertIn("main:game/a", identifiers)
            self.assertIn("REL:foo:b", identifiers)
            self.assertIn("unconfigured:game/canonical_cp", identifiers)
            self.assertIn("unconfigured:game/canonical_cpp", identifiers)

            owner = find_owner(catalog, "main:game/a")[0]
            self.assertIn("include/common.h", owner["includes"])
            self.assertEqual(
                catalog["header_consumers"]["include/common.h"],
                ["main:game/a"],
            )
            self.assertEqual(
                catalog["function_owners"]["helper"], ["REL:foo:b"]
            )
            self.assertEqual(
                catalog["function_consumers"]["helper"], ["main:game/a"]
            )
            self.assertEqual(
                catalog["global_owners"]["shared"], ["REL:foo:b"]
            )
            self.assertEqual(
                catalog["data_consumers"]["shared"], ["main:game/a"]
            )
            self.assertEqual(
                catalog["symbol_import_consumers"]["helper"], ["main:game/a"]
            )
            self.assertIn("REL:foo:b", owner["depends_on_owners"])
            self.assertFalse(catalog["analysis_quality"]["semantic_claim"])


if __name__ == "__main__":
    unittest.main()
