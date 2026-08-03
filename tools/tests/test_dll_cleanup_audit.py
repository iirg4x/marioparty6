import tempfile
import unittest
from pathlib import Path

from tools.dll_cleanup_audit import (
    address_identifiers,
    build_cleanup_report,
    classify_module,
    forbidden_source_name,
    function_pointer_integer_casts,
    reviewed_rel_modules,
)


class DllCleanupAuditTests(unittest.TestCase):
    def test_reviewed_allowlist_never_guesses_uncertain_module_ownership(self) -> None:
        owners = [
            {
                "id": "REL:endingdll:ending",
                "module": "endingdll",
                "source": "src/REL/endingdll/ending.c",
            },
            {
                "id": "REL:w01Dll:world01",
                "module": "w01Dll",
                "source": "src/REL/w01Dll/world01.c",
            },
        ]
        reviewed = reviewed_rel_modules(owners)
        self.assertEqual(classify_module("endingdll", reviewed)[0], "eligible_reviewed")
        self.assertEqual(classify_module("w01Dll", reviewed)[0], "eligible_reviewed")
        self.assertEqual(classify_module("w00Dll", reviewed)[0], "excluded_w0")
        self.assertEqual(classify_module("m401Dll", reviewed)[0], "uncertain_excluded")

    def test_mechanical_source_scan_ignores_comments_and_literals(self) -> None:
        self.assertEqual(
            forbidden_source_name("src/REL/endingdll/ending_pass6_1025c.c"),
            ["placeholder_fragment", "address_shard"],
        )
        self.assertEqual(forbidden_source_name("src/REL/foo/compass.c"), [])
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "owner.c"
            path.write_text(
                'const char *text = "fn_1_DEAD";\n'
                "/* lbl_1_data_BEEF */\n"
                "int fn_1_1234(void) { return 0; }\n",
                encoding="utf-8",
            )
            self.assertEqual(
                address_identifiers(path, "src/REL/foo/owner.c"),
                [
                    {
                        "path": "src/REL/foo/owner.c",
                        "line": 3,
                        "identifier": "fn_1_1234",
                    }
                ],
            )
            path.write_text(
                "typedef float (*CurveEval)();\n"
                "float slope(float a, float b, float c, float t);\n"
                "CurveEval value = (CurveEval)(u32)slope;\n",
                encoding="utf-8",
            )
            casts = function_pointer_integer_casts(
                path, "src/REL/foo/owner.c"
            )
            self.assertEqual(len(casts), 1)
            self.assertEqual(casts[0]["rule"], "function_pointer_integer_cast")

    def test_report_scans_all_sources_in_reviewed_module_only(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ending = root / "src/REL/endingdll/ending_pass1.c"
            runtime = root / "src/REL/endingdll/runtime.c"
            minigame = root / "src/REL/m401Dll/application_pass1.c"
            ending.parent.mkdir(parents=True)
            minigame.parent.mkdir(parents=True)
            ending.write_text("int fn_1_1234(void) { return 0; }\n", encoding="utf-8")
            runtime.write_text(
                "extern int fn_1_1234(void);\n"
                "int runtime(void) { return fn_1_1234(); }\n",
                encoding="utf-8",
            )
            minigame.write_text("int fn_1_5678(void) { return 0; }\n", encoding="utf-8")
            data = {
                "root": root,
                "owners": [
                    {
                        "id": "REL:endingdll:ending",
                        "module": "endingdll",
                        "source": "src/REL/endingdll/ending_pass1.c",
                    }
                ],
                "exceptions": [],
            }
            catalog = {
                "owners": [
                    {"module": "endingdll", "source": "src/REL/endingdll/ending_pass1.c"},
                    {"module": "endingdll", "source": "src/REL/endingdll/runtime.c"},
                    {"module": "m401Dll", "source": "src/REL/m401Dll/application_pass1.c"},
                ]
            }
            report = build_cleanup_report(data, catalog)
            modules = {item["module"]: item for item in report["modules"]}
            self.assertTrue(modules["endingdll"]["actionable"])
            self.assertEqual(modules["endingdll"]["configured_sources"], 2)
            self.assertEqual(len(modules["endingdll"]["filename_findings"]), 1)
            self.assertEqual(len(modules["endingdll"]["address_identifiers"]), 3)
            self.assertEqual(
                len(modules["endingdll"]["unique_address_identifiers"]), 1
            )
            self.assertEqual(modules["m401Dll"]["classification"], "uncertain_excluded")
            self.assertFalse(modules["m401Dll"]["actionable"])

    def test_report_can_audit_a_separate_worker_source_root(self) -> None:
        with tempfile.TemporaryDirectory() as metadata_directory, tempfile.TemporaryDirectory() as source_directory:
            metadata_root = Path(metadata_directory)
            source_root = Path(source_directory)
            source = source_root / "src/REL/w01Dll/world01.c"
            source.parent.mkdir(parents=True)
            source.write_text("int World01Create(void) { return 0; }\n", encoding="utf-8")
            data = {
                "root": metadata_root,
                "owners": [
                    {
                        "id": "REL:w01Dll:world01",
                        "module": "w01Dll",
                        "source": "src/REL/w01Dll/world01.c",
                    }
                ],
                "exceptions": [],
            }
            catalog = {
                "owners": [
                    {"module": "w01Dll", "source": "src/REL/w01Dll/world01.c"}
                ]
            }
            report = build_cleanup_report(data, catalog, source_root=source_root)
            self.assertEqual(report["source_root"], str(source_root.resolve()))
            self.assertEqual(report["summary"]["clean_eligible"], 1)


if __name__ == "__main__":
    unittest.main()
