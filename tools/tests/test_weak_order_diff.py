import json
import io
import os
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from tools import weak_order_diff as module


READELF_OUTPUT = """
Symbol table '.symtab' contains 12 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 00000020    16 FUNC    LOCAL  DEFAULT    1 beta$2
     2: 00000010    12 FUNC    GLOBAL DEFAULT    1 alpha
     3: 00000008     4 OBJECT  LOCAL  DEFAULT    2 @12
     4: 00000000     4 OBJECT  LOCAL  DEFAULT    2 table$4
     5: 00000004     4 OBJECT  LOCAL  DEFAULT    2 @3
     6: 0000000c     4 OBJECT  LOCAL  DEFAULT    2 lbl_1_data_000c
     7: 00000010     4 OBJECT  LOCAL  HIDDEN     2 hidden
     8: 00000014     4 OBJECT  LOCAL  DEFAULT  UND missing
     9: 00000018     4 OBJECT  LOCAL  DEFAULT    2 .LC0
    10: 0000001c     4 NOTYPE  LOCAL  DEFAULT    2 label
"""


class WeakOrderDiffTests(unittest.TestCase):
    def test_parse_and_normalize_orders(self) -> None:
        symbols = module.parse_readelf_symbols(READELF_OUTPUT)
        orders = module.orders_from_symbols(symbols)
        self.assertEqual(orders["functions"], ["alpha", "beta$"])
        self.assertEqual(orders["data"], ["table$"])
        self.assertEqual(orders["local_pools"], ["@", "@", ".LC0"])

    def test_parser_accepts_bytes_and_ignores_readelf_headings(self) -> None:
        symbols = module.parse_readelf_symbols(READELF_OUTPUT.encode("utf-8"))
        self.assertEqual(len(symbols), 11)
        self.assertEqual(symbols[1].value, 0x20)
        self.assertEqual(symbols[1].size, 16)
        self.assertEqual(symbols[1].name, "beta$2")

    def test_parser_accepts_hexadecimal_symbol_size(self) -> None:
        output = """
Symbol table '.symtab' contains 1 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     1: 00000010  0x10 FUNC    LOCAL  DEFAULT    1 hex_sized
"""
        symbols = module.parse_readelf_symbols(output)
        self.assertEqual(len(symbols), 1)
        self.assertEqual(symbols[0].size, 0x10)

    def test_parser_skips_unprefixed_alpha_size_without_crashing(self) -> None:
        output = """
Symbol table '.symtab' contains 2 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     1: 00000010   ABC FUNC    LOCAL  DEFAULT    1 malformed
     2: 00000020    16 FUNC    LOCAL  DEFAULT    1 valid
"""
        symbols = module.parse_readelf_symbols(output)
        self.assertEqual([symbol.name for symbol in symbols], ["valid"])

    def test_symtab_is_preferred_when_readelf_prints_dynsym_too(self) -> None:
        output = """
Symbol table '.dynsym' contains 1 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     1: 00000000     0 FUNC    GLOBAL DEFAULT  UND duplicate
Symbol table '.symtab' contains 1 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     1: 00000010     4 FUNC    GLOBAL DEFAULT    1 duplicate
"""
        symbols = module.parse_readelf_symbols(output)
        self.assertEqual(len(symbols), 1)
        self.assertEqual(symbols[0].value, 0x10)

    def test_compare_reports_missing_extra_and_inversion(self) -> None:
        target = {"functions": ["a", "b", "c"], "data": [], "local_pools": []}
        source = {"functions": ["b", "a", "d"], "data": [], "local_pools": []}
        differences = module.compare_orders(target, source)
        function_diff = differences["functions"]
        self.assertEqual(function_diff["missing_in_source"], ["c"])
        self.assertEqual(function_diff["extra_in_source"], ["d"])
        self.assertEqual(function_diff["missing"], ["c"])
        self.assertEqual(function_diff["extra"], ["d"])
        self.assertEqual(
            [(item["first"], item["second"]) for item in function_diff["order_inversions"]],
            [("a", "b")],
        )
        self.assertEqual(function_diff["inversions"], 1)
        self.assertFalse(function_diff["order_match"])

    def test_identical_interleaved_duplicate_names_have_no_inversion(self) -> None:
        target = {"functions": [], "data": [], "local_pools": ["@", ".LC0", "@"]}
        source = {"functions": [], "data": [], "local_pools": ["@", ".LC0", "@"]}
        local_pool_diff = module.compare_orders(target, source)["local_pools"]
        self.assertEqual(local_pool_diff["order_inversions"], [])
        self.assertEqual(local_pool_diff["inversions"], 0)
        self.assertTrue(local_pool_diff["order_match"])

    def test_duplicate_names_can_still_have_a_real_inversion(self) -> None:
        target = {"functions": [], "data": [], "local_pools": ["@", ".LC0", "@"]}
        source = {"functions": [], "data": [], "local_pools": [".LC0", "@", "@"]}
        local_pool_diff = module.compare_orders(target, source)["local_pools"]
        self.assertEqual(local_pool_diff["inversions"], 1)
        self.assertEqual(
            local_pool_diff["order_inversions"],
            [
                {
                    "first": "@",
                    "second": ".LC0",
                    "target_indices": [0, 1],
                    "source_indices": [1, 0],
                }
            ],
        )

    def test_readelf_runner_is_explicit_and_read_only(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            object_path = root / "target.o"
            readelf_path = root / "readelf"
            object_path.write_bytes(b"object")
            readelf_path.write_bytes(b"mock executable")
            calls = []

            def runner(command, **kwargs):
                calls.append((command, kwargs))
                return SimpleNamespace(returncode=0, stdout=READELF_OUTPUT, stderr="")

            symbols = module.readelf_symbols(
                object_path,
                readelf_path=readelf_path,
                runner=runner,
            )
            self.assertTrue(symbols)
            self.assertEqual(
                calls[0][0], [str(readelf_path), "-Ws", "--", str(object_path)]
            )
            self.assertEqual(calls[0][1], {"capture_output": True, "text": True, "check": False})
            self.assertEqual(object_path.read_bytes(), b"object")

    def test_leading_dash_relative_object_path_is_after_end_of_options(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            previous_directory = os.getcwd()
            try:
                os.chdir(directory)
                object_path = Path("-target.o")
                readelf_path = Path("readelf")
                object_path.write_bytes(b"object")
                readelf_path.write_bytes(b"mock executable")
                calls = []

                def runner(command, **kwargs):
                    calls.append(command)
                    return SimpleNamespace(returncode=0, stdout=READELF_OUTPUT, stderr="")

                module.readelf_symbols(
                    object_path,
                    readelf_path=readelf_path,
                    runner=runner,
                )
            finally:
                os.chdir(previous_directory)
            self.assertEqual(calls[0], ["readelf", "-Ws", "--", "-target.o"])

    def test_readelf_failure_names_input_path(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            object_path = root / "bad.o"
            readelf_path = root / "readelf"
            object_path.write_bytes(b"object")
            readelf_path.write_bytes(b"mock executable")

            def runner(command, **kwargs):
                return SimpleNamespace(returncode=1, stdout="", stderr="bad object")

            with self.assertRaisesRegex(module.ReadelfError, r"bad\.o.*bad object"):
                module.readelf_symbols(
                    object_path,
                    readelf_path=readelf_path,
                    runner=runner,
                )

    def test_json_cli_uses_mocked_parser_and_does_not_build(self) -> None:
        target_orders = {"functions": ["a"], "data": [], "local_pools": ["@"]}
        source_orders = {"functions": ["a"], "data": [], "local_pools": ["@"]}
        target_path = Path("target.o")
        source_path = Path("source.o")
        with patch.object(
            module,
            "readelf_symbols",
            side_effect=[
                [module.Symbol(0, 0, 1, "FUNC", "GLOBAL", "DEFAULT", "1", "a")],
                [module.Symbol(0, 0, 1, "FUNC", "GLOBAL", "DEFAULT", "1", "a")],
            ],
        ) as readelf_symbols, patch.object(
            module, "orders_from_symbols", side_effect=[target_orders, source_orders]
        ), patch.object(
            module, "resolve_readelf", return_value="resolved-readelf"
        ) as resolve_readelf, patch("sys.stdout", new_callable=io.StringIO) as stdout:
            result = module.main(
                [
                    "--target",
                    str(target_path),
                    "--source",
                    str(source_path),
                    "--readelf",
                    "pinned-readelf",
                    "--json",
                ]
            )
        self.assertEqual(result, 0)
        report = json.loads(stdout.getvalue())
        self.assertEqual(report["readelf"], "resolved-readelf")
        self.assertEqual(report["target"]["orders"], target_orders)
        self.assertEqual(report["differences"]["functions"]["order_match"], True)
        resolve_readelf.assert_called_once_with("pinned-readelf")
        self.assertEqual(
            [call.kwargs["readelf_path"] for call in readelf_symbols.call_args_list],
            ["resolved-readelf", "resolved-readelf"],
        )

    def test_human_report_includes_readelf_provenance(self) -> None:
        report = module.build_report(
            "target.o",
            "source.o",
            {"functions": [], "data": [], "local_pools": []},
            {"functions": [], "data": [], "local_pools": []},
            readelf_path="pinned-readelf",
        )
        rendered = module.render_human(report)
        self.assertIn("Readelf: pinned-readelf", rendered)


if __name__ == "__main__":
    unittest.main()
