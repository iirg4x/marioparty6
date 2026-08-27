from __future__ import annotations

import base64
import copy
import unittest

from tools import complete_stack_home_exchange as exchange
from tools import typed_pool_owner_manifest as owner_manifest


FUNCTION = "mbev_CapHanachan"
MAPPINGS = (
    (0x8C, 0xAC),
    (0x90, 0xB0),
    (0x94, 0xB4),
    (0xC8, 0x8C),
    (0xCC, 0x90),
    (0xD0, 0x94),
)


def _stack_instruction(opcode: str, value: str, offset: int, row: int) -> dict[str, object]:
    if opcode == "addi":
        formatted = f"addi {value}, r1, {offset:#x}"
        arg_diff = [{}, {}, {"diff_index": row % 18}]
    else:
        formatted = f"{opcode} {value}, {offset:#x}(r1)"
        arg_diff = [{}, {"diff_index": row % 18}, {}]
    return {
        "arg_diff": arg_diff,
        "diff_kind": "DIFF_ARG_MISMATCH",
        "instruction": {
            "address": str(0x2500 + row * 4),
            "formatted": formatted,
            "size": 4,
        },
    }


def _pool_symbol(name: str, address: int) -> dict[str, object]:
    return {
        "name": name,
        "kind": "SYMBOL_OBJECT",
        "address": str(address),
        "size": "4",
        "data_diff": [
            {"offset": "0", "data": base64.b64encode(bytes.fromhex("3f800000")).decode()}
        ],
    }


def _pool_instruction(owner_index: int, owner_name: str, destination: str, row: int) -> dict[str, object]:
    return {
        "arg_diff": [{"diff_index": 18}, {}],
        "diff_kind": "DIFF_ARG_MISMATCH",
        "instruction": {
            "address": str(0x2500 + row * 4),
            "formatted": f"lfs {destination}, {owner_name}@sda21",
            "size": 4,
            "relocation": {
                "target_symbol": owner_index,
                "type": 109,
                "type_name": "R_PPC_EMB_SDA21",
                "addend": 0,
            },
        },
    }


def reports() -> tuple[dict[str, object], dict[str, object]]:
    opcodes = (
        ("stw", "r3"),
        ("lwz", "r3"),
        ("lfs", "f0"),
        ("stfs", "f0"),
        ("addi", "r4"),
    )
    target_rows: list[dict[str, object]] = []
    candidate_rows: list[dict[str, object]] = []
    for row in range(46):
        candidate_offset, target_offset = MAPPINGS[row % len(MAPPINGS)]
        opcode, value = opcodes[row % len(opcodes)]
        target_rows.append(_stack_instruction(opcode, value, target_offset, row))
        candidate_rows.append(_stack_instruction(opcode, value, candidate_offset, row))

    target_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "size": "5856",
            "instructions": target_rows,
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "128"},
        _pool_symbol("lbl_802C3D24", 64),
    ]
    candidate_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": FUNCTION,
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 1,
            "size": "5856",
            "match_percent": 99.965164,
            "instructions": candidate_rows,
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "128"},
        _pool_symbol("@283", 64),
    ]
    target_rows.append(_pool_instruction(3, "lbl_802C3D24", "f31", 46))
    candidate_rows.append(_pool_instruction(3, "@283", "f25", 46))
    strict = {"left": {"symbols": target_symbols}, "right": {"symbols": candidate_symbols}}
    data = copy.deepcopy(strict)
    data["right"]["symbols"][1]["match_percent"] = 99.98258
    return strict, data


def binding() -> dict[str, object]:
    return {
        "schema": owner_manifest.BINDING_SCHEMA,
        "strict_report_path": "hanachan075.strict.json",
        "strict_report_sha256": "11" * 32,
        "data_report_path": "hanachan075.data.json",
        "data_report_sha256": "22" * 32,
        "target_object_path": "target-capmove.o",
        "target_object_sha256": "33" * 32,
        "candidate_object_path": "candidate-capmove.o",
        "candidate_object_sha256": "44" * 32,
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }


class CompleteStackHomeExchangeTests(unittest.TestCase):
    def test_hanachan075_shape_matches_46_home_rows_and_one_pool_handoff(self) -> None:
        strict, data = reports()
        result = exchange.build_diagnosis(strict, data, FUNCTION, binding())

        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["route"], exchange.ROUTE)
        self.assertEqual(result["facts"]["strict_residual_row_count"], 47)
        self.assertEqual(result["facts"]["stack_home_row_count"], 46)
        self.assertEqual(result["facts"]["pool_handoff_row_count"], 1)
        self.assertEqual(result["facts"]["mapping_count"], len(MAPPINGS))
        self.assertEqual(len(result["candidate_cells"]), 1)
        self.assertEqual(result["candidate_cells"][0]["compile_candidate_limit"], 1)
        self.assertEqual(result["trace_budget"], 0)
        self.assertFalse(result["source_patch_emitted"])
        self.assertFalse(result["authority_advanced"])
        self.assertIn("declaration_order_permutation", result["suppressed_axes"])
        self.assertIn("scope_permutation", result["suppressed_axes"])

    def test_fails_closed_on_non_stack_instruction(self) -> None:
        strict, data = reports()
        strict["right"]["symbols"][1]["instructions"][0]["instruction"]["formatted"] = "mr r3, r4"
        result = exchange.build_diagnosis(strict, data, FUNCTION, binding())
        self.assertEqual(result["status"], "blocked")
        self.assertIn("row_0_not_supported_r1_stack_instruction", result["blockers"])
        self.assertEqual(result["candidate_cells"], [])

    def test_fails_closed_on_non_bijective_mapping(self) -> None:
        strict, data = reports()
        strict["left"]["symbols"][1]["instructions"][0]["instruction"]["formatted"] = (
            "stw r3, 0xb0(r1)"
        )
        result = exchange.build_diagnosis(strict, data, FUNCTION, binding())
        self.assertEqual(result["status"], "blocked")
        self.assertTrue(
            any(blocker.startswith("candidate_offset_") for blocker in result["blockers"])
        )

    def test_fails_closed_when_function_size_differs(self) -> None:
        strict, data = reports()
        strict["right"]["symbols"][1]["size"] = "5860"
        result = exchange.build_diagnosis(strict, data, FUNCTION, binding())
        self.assertEqual(result["status"], "blocked")
        self.assertIn("function_size_not_exact", result["blockers"])

    def test_diagnosis_hash_is_canonical(self) -> None:
        strict, data = reports()
        result = exchange.build_diagnosis(strict, data, FUNCTION, binding())
        digest = result["diagnosis_sha256"]
        unhashed = copy.deepcopy(result)
        unhashed.pop("diagnosis_sha256")
        self.assertEqual(digest, owner_manifest.canonical_sha256(unhashed))


if __name__ == "__main__":
    unittest.main()
