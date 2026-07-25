import tempfile
import unittest
from pathlib import Path

from tools.owner_catalog import build_catalog, find_owner


class OwnerCatalogTests(unittest.TestCase):
    def test_ast_catalog_and_header_consumers(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "src/game").mkdir(parents=True)
            (root / "src/REL/foo").mkdir(parents=True)
            (root / "include").mkdir()
            (root / "include/common.h").write_text(
                "#pragma once\n", encoding="utf-8"
            )
            (root / "src/game/a.c").write_text(
                '#include "common.h"\nint a;\n', encoding="utf-8"
            )
            (root / "src/REL/foo/b.c").write_text(
                "int b;\n", encoding="utf-8"
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
            owner = find_owner(catalog, "main:game/a")[0]
            self.assertIn("include/common.h", owner["includes"])
            self.assertEqual(
                catalog["header_consumers"]["include/common.h"],
                ["main:game/a"],
            )


if __name__ == "__main__":
    unittest.main()
