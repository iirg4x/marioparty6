from __future__ import annotations

import base64
import copy
import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools import typed_pool_owner_manifest as manifest


FUNCTION = "ConfigPadClose"
OWNER_ROWS = (
    ("lbl_802C4CB4", "@358", "00000000", (52, 68, 75)),
    ("lbl_802C4CC8", "@537", "3e19999a", (66, 73)),
    ("lbl_802C4CCC", "@297", "3f800000", (108, 111)),
    ("lbl_802C4CD0", "@535", "3f99999a", (62,)),
    ("lbl_802C4CD4", "@536", "be666666", (64,)),
    ("lbl_802C4CF4", "@838", "3eca3d71", (71,)),
    ("lbl_802C4CF8", "@839", "3eaaaaab", (77,)),
)


def _symbol(name: str, bits: str, address: int) -> dict[str, object]:
    raw = bytes.fromhex(bits)
    return {
        "name": name,
        "kind": "SYMBOL_OBJECT",
        "address": str(address),
        "size": str(len(raw)),
        "data_diff": [{"offset": "0", "data": base64.b64encode(raw).decode()}],
    }


def _instruction(
    owner_index: int | None,
    owner_name: str | None,
    row: int,
    *,
    destination: str = "f0",
    diff: bool = True,
    relocation_type: str = "R_PPC_EMB_SDA21",
) -> dict[str, object]:
    nested: dict[str, object] = {
        "address": str(0x2000 + row * 4),
        "formatted": "nop" if owner_name is None else f"lfs {destination}, {owner_name}@sda21",
    }
    result: dict[str, object] = {"instruction": nested}
    if owner_index is not None:
        nested["relocation"] = {
            "target_symbol": owner_index,
            "type_name": relocation_type,
            "addend": 0,
        }
    if diff:
        result["diff_kind"] = "DIFF_ARG_MISMATCH"
    return result


def reports() -> tuple[dict[str, object], dict[str, object]]:
    max_row = max(row for _, _, _, rows in OWNER_ROWS for row in rows)
    target_instructions = [_instruction(None, None, row, diff=False) for row in range(max_row + 1)]
    candidate_instructions = copy.deepcopy(target_instructions)
    target_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "size": "1192",
            "instructions": target_instructions,
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "96"},
    ]
    candidate_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 1,
            "size": "1192",
            "match_percent": 99.81544,
            "instructions": candidate_instructions,
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "84"},
    ]
    for index, (target_name, candidate_name, bits, rows) in enumerate(OWNER_ROWS):
        target_index = len(target_symbols)
        candidate_index = len(candidate_symbols)
        target_symbols.append(_symbol(target_name, bits, 20 + index * 12))
        candidate_symbols.append(_symbol(candidate_name, bits, 20 + index * 8))
        for row in rows:
            destination = "f1" if row in {62, 77} else "f0"
            target_instructions[row] = _instruction(
                target_index, target_name, row, destination=destination
            )
            candidate_instructions[row] = _instruction(
                candidate_index, candidate_name, row, destination=destination
            )
    strict = {"left": {"symbols": target_symbols}, "right": {"symbols": candidate_symbols}}
    data = copy.deepcopy(strict)
    data["right"]["symbols"][1]["match_percent"] = 100.0
    return strict, data


def binding() -> dict[str, object]:
    return {
        "schema": manifest.BINDING_SCHEMA,
        "strict_report_path": "strict.json",
        "strict_report_sha256": "11" * 32,
        "data_report_path": "data.json",
        "data_report_sha256": "22" * 32,
        "target_object_path": "target.o",
        "target_object_sha256": "33" * 32,
        "candidate_object_path": "candidate.o",
        "candidate_object_sha256": "44" * 32,
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }


def _hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class TypedPoolOwnerManifestTests(unittest.TestCase):
    def test_configpadclose_shape_emits_one_seven_owner_cell(self) -> None:
        strict, data = reports()
        result = manifest.build_manifest(strict, data, FUNCTION, binding())

        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["route"], manifest.ROUTE)
        self.assertEqual(result["facts"]["strict_residual_row_count"], 11)
        self.assertEqual(result["facts"]["nonexact_pool_row_count"], 11)
        self.assertEqual(result["facts"]["classification_counts"], {"owner_identity_mismatch": 11})
        self.assertEqual(len(result["owners"]), 7)
        self.assertEqual(
            [item["target"]["name"] for item in result["owners"]],
            [item[0] for item in sorted(OWNER_ROWS, key=lambda item: min(item[3]))],
        )
        self.assertEqual(len(result["candidate_cells"]), 1)
        self.assertEqual(result["candidate_cells"][0]["compile_candidate_limit"], 1)
        self.assertFalse(result["candidate_cells"][0]["source_patch_emitted"])
        self.assertFalse(result["authority_advanced"])
        self.assertFalse(result["retention_authorized"])
        self.assertFalse(result["promotion_authorized"])

        digest = result["manifest_sha256"]
        unhashed = copy.deepcopy(result)
        unhashed.pop("manifest_sha256")
        self.assertEqual(digest, manifest.canonical_sha256(unhashed))

    def test_fails_closed_on_non_pool_residual(self) -> None:
        strict, data = reports()
        for side in (strict["left"], strict["right"]):
            side["symbols"][1]["instructions"][10] = _instruction(None, None, 10, diff=True)
        result = manifest.build_manifest(strict, data, FUNCTION, binding())
        self.assertEqual(result["status"], "blocked")
        self.assertIn("strict_residual_contains_non_pool_or_unpaired_rows", result["blockers"])
        self.assertEqual(result["owners"], [])
        self.assertEqual(result["candidate_cells"], [])

    def test_fails_closed_when_data_is_not_exact(self) -> None:
        strict, data = reports()
        data["right"]["symbols"][1]["match_percent"] = 99.9
        result = manifest.build_manifest(strict, data, FUNCTION, binding())
        self.assertEqual(result["status"], "blocked")
        self.assertIn("data_function_not_exact", result["blockers"])

    def test_fails_closed_on_value_type_or_relocation_drift(self) -> None:
        mutations = {
            "value": lambda report: report["right"]["symbols"][3].update(
                _symbol("@358", "00000001", 20)
            ),
            "relocation": lambda report: report["right"]["symbols"][1]["instructions"][52][
                "instruction"
            ]["relocation"].update({"type_name": "R_PPC_ADDR32"}),
            "target_unnamed": lambda report: report["left"]["symbols"][3].update(
                {"name": "@target_zero"}
            ),
            "destination": lambda report: report["right"]["symbols"][1]["instructions"][52][
                "instruction"
            ].update({"formatted": "lfs f2, @358@sda21"}),
        }
        for name, mutate in mutations.items():
            with self.subTest(name=name):
                strict, data = reports()
                mutate(strict)
                result = manifest.build_manifest(strict, data, FUNCTION, binding())
                self.assertEqual(result["status"], "blocked")
                self.assertTrue(result["blockers"])

    def test_binding_is_closed_and_authority_false(self) -> None:
        strict, data = reports()
        for mutation in (
            lambda value: value.update({"extra": True}),
            lambda value: value.update({"retail_target_authenticated": False}),
            lambda value: value.update({"authority_advanced": True}),
            lambda value: value.update({"target_object_sha256": "BAD"}),
        ):
            with self.subTest(mutation=mutation):
                value = binding()
                mutation(value)
                with self.assertRaises(manifest.TypedPoolManifestInputError):
                    manifest.build_manifest(strict, data, FUNCTION, value)

    def test_cli_hash_binds_files_and_writes_manifest(self) -> None:
        strict, data = reports()
        root = Path(__file__).resolve().parents[2]
        script = root / "tools" / "typed_pool_owner_manifest.py"
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            strict_path = temp / "strict.json"
            data_path = temp / "data.json"
            target_path = temp / "target.o"
            candidate_path = temp / "candidate.o"
            output_path = temp / "manifest.json"
            strict_path.write_text(json.dumps(strict), encoding="utf-8")
            data_path.write_text(json.dumps(data), encoding="utf-8")
            target_path.write_bytes(b"target-object")
            candidate_path.write_bytes(b"candidate-object")
            command = [
                sys.executable,
                str(script),
                str(strict_path),
                str(data_path),
                FUNCTION,
                "--target-object",
                str(target_path),
                "--candidate-object",
                str(candidate_path),
                "--expect-strict-report-sha256",
                _hash(strict_path),
                "--expect-data-report-sha256",
                _hash(data_path),
                "--expect-target-object-sha256",
                _hash(target_path),
                "--expect-candidate-object-sha256",
                _hash(candidate_path),
                "--output",
                str(output_path),
                "--require-match",
            ]
            completed = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            result = json.loads(output_path.read_text(encoding="utf-8"))
            self.assertEqual(result["status"], "matched")

            command[command.index(_hash(target_path))] = "ff" * 32
            failed = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
            self.assertNotEqual(failed.returncode, 0)
            self.assertIn("evidence hash mismatch", failed.stderr)


if __name__ == "__main__":
    unittest.main()
