from __future__ import annotations

import base64
import copy
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools import pool_reloc_summary as module


def _symbol(name: str, raw: bytes, address: int) -> dict[str, object]:
    return {
        "name": name,
        "kind": "SYMBOL_OBJECT",
        "address": str(address),
        "size": str(len(raw)),
        "data_diff": [{"offset": "0", "data": base64.b64encode(raw).decode()}],
    }


def _weak_symbol(name: str, raw: bytes, address: int | None) -> dict[str, object]:
    symbol = _symbol(name, raw, 0 if address is None else address)
    if address is None:
        symbol.pop("address")
    symbol["flags"] = {"global": True, "weak": True}
    return symbol


def _instruction(
    formatted: str,
    owner: int | None,
    *,
    type_name: str = "R_PPC_EMB_SDA21",
    addend: int = 0,
) -> dict[str, object]:
    instruction: dict[str, object] = {"formatted": formatted}
    row: dict[str, object] = {"instruction": instruction}
    if owner is not None:
        relocation: dict[str, object] = {
            "target_symbol": owner,
            "type_name": type_name,
        }
        if addend:
            relocation["addend"] = addend
        instruction["relocation"] = relocation
        row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return row


def _report() -> dict[str, object]:
    target_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "PoolFocus",
            "kind": "SYMBOL_FUNCTION",
            "size": "24",
            "instructions": [
                _instruction("lfs f0, lbl_zero@sda21", 3),
                _instruction("lfs f1, lbl_zero@sda21", 3),
                _instruction("lfd f2, lbl_bias@sda21", 4),
                _instruction("lfs f3, lbl_value@sda21", 5),
                _instruction("lfs f4, lbl_addend@sda21", 6, addend=4),
                _instruction("lfs f5, lbl_missing@sda21", 7),
            ],
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "28"},
        _symbol("lbl_zero", bytes.fromhex("00000000"), 0),
        _symbol("lbl_bias", bytes.fromhex("4330000080000000"), 8),
        _symbol("lbl_value", bytes.fromhex("3f800000"), 16),
        _symbol("lbl_addend", bytes.fromhex("40000000"), 20),
        _symbol("lbl_missing", bytes.fromhex("40400000"), 24),
    ]
    candidate_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "PoolFocus",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 1,
            "size": "24",
            "match_percent": 97.5,
            "instructions": [
                _instruction("lfs f0, @10@sda21", 3),
                _instruction("lfs f1, @10@sda21", 3),
                _instruction("lfs f2, @11@sda21", 4),
                _instruction("lfs f3, @12@sda21", 5),
                _instruction("lfs f4, @13@sda21", 6, addend=0),
                _instruction("nop", None),
            ],
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "24"},
        _symbol("@10", bytes.fromhex("00000000"), 4),
        _symbol("@11", bytes.fromhex("4330000080000000"), 8),
        _symbol("@12", bytes.fromhex("40000000"), 16),
        _symbol("@13", bytes.fromhex("40000000"), 20),
    ]
    return {"left": {"symbols": target_symbols}, "right": {"symbols": candidate_symbols}}


class PoolRelocSummaryTests(unittest.TestCase):
    def test_decodes_owner_only_groups_and_mwcc_bias(self) -> None:
        result = module.decode_function(_report(), "PoolFocus")
        self.assertEqual(result["schema"], module.SCHEMA)
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(result["summary"]["classification_counts"]["owner_identity_mismatch"], 2)
        owner_group = next(
            item for item in result["groups"]
            if item["classification"] == "owner_identity_mismatch" and item["count"] == 2
        )
        self.assertEqual(owner_group["rows"], [0, 1])
        self.assertEqual(owner_group["target"]["owner"]["typed"]["f32"], 0.0)
        self.assertEqual(owner_group["candidate"]["owner"]["owner_class"], "compiler_anonymous")
        type_group = next(item for item in result["groups"] if item["classification"] == "literal_type_mismatch")
        self.assertEqual(type_group["target"]["owner"]["typed"]["mwcc_role"], "signed-int-to-double-bias")
        self.assertIn("consumer_type", type_group["differences"])

    def test_separates_value_addend_and_unpaired_consumers(self) -> None:
        result = module.decode_function(_report(), "PoolFocus")
        counts = result["summary"]["classification_counts"]
        self.assertEqual(counts["literal_value_mismatch"], 1)
        self.assertEqual(counts["relocation_addend_mismatch"], 1)
        self.assertEqual(counts["target_only_pool_consumer"], 1)
        value = next(item for item in result["groups"] if item["classification"] == "literal_value_mismatch")
        self.assertEqual(value["target"]["owner"]["typed"]["f32"], 1.0)
        self.assertEqual(value["candidate"]["owner"]["typed"]["f32"], 2.0)
        self.assertLess(
            next(index for index, item in enumerate(result["groups"]) if item["classification"] == "literal_value_mismatch"),
            next(index for index, item in enumerate(result["groups"]) if item["classification"] == "owner_identity_mismatch"),
        )

    def test_relocation_type_precedes_owner_identity(self) -> None:
        report = _report()
        candidate = report["right"]["symbols"][1]
        candidate["instructions"][0]["instruction"]["relocation"]["type_name"] = "R_PPC_ADDR32"
        result = module.decode_function(report, "PoolFocus")
        group = next(item for item in result["groups"] if 0 in item["rows"])
        self.assertEqual(group["classification"], "relocation_type_mismatch")
        self.assertEqual(group["interpretation"], "abi_or_storage_class_mismatch")

    def test_include_exact_reports_exact_contract(self) -> None:
        report = _report()
        report["right"]["symbols"][3] = copy.deepcopy(report["left"]["symbols"][3])
        result = module.decode_function(report, "PoolFocus", include_exact=True)
        self.assertEqual(result["summary"]["classification_counts"]["exact_pool_contract"], 2)

    def test_objdiff_mapped_owner_transition_is_not_a_default_mismatch(self) -> None:
        report = _report()
        report["left"]["symbols"][1]["instructions"][0].pop("diff_kind")
        report["right"]["symbols"][1]["instructions"][0].pop("diff_kind")
        default = module.decode_function(report, "PoolFocus")
        self.assertNotIn(0, [row for group in default["groups"] for row in group["rows"]])
        full = module.decode_function(report, "PoolFocus", include_exact=True)
        mapped = next(item for item in full["groups"] if 0 in item["rows"])
        self.assertEqual(mapped["classification"], "mapped_pool_contract")
        self.assertEqual(mapped["interpretation"], "exact_relocation_mapping_with_object_local_owner_identity")

    def test_exact_external_pool_contract_is_not_unresolved(self) -> None:
        report = _report()
        target_owner = report["left"]["symbols"][3]
        target_owner["name"] = "lbl_external"
        candidate_owner = report["right"]["symbols"][3]
        candidate_owner.clear()
        candidate_owner.update({"name": "lbl_external", "flags": {"global": True}})
        report["left"]["symbols"][1]["instructions"][0].pop("diff_kind")
        report["right"]["symbols"][1]["instructions"][0].pop("diff_kind")

        default = module.decode_function(report, "PoolFocus")
        self.assertNotIn(0, [row for group in default["groups"] for row in group["rows"]])
        full = module.decode_function(report, "PoolFocus", include_exact=True)
        mapped = next(item for item in full["groups"] if 0 in item["rows"])
        self.assertEqual(mapped["classification"], "mapped_pool_contract")
        self.assertEqual(mapped["interpretation"], "exact_relocation_mapping_with_external_owner_contract")

    def test_detects_exact_weak_sqrtf_prefix_and_predicts_section_shift(self) -> None:
        report = _report()
        target = report["left"]
        candidate = report["right"]
        target["sections"] = [{"name": ".sdata2", "size": "560"}]
        candidate["sections"] = [{"name": ".sdata2", "size": "480"}]
        section_index = next(
            index for index, symbol in enumerate(candidate["symbols"])
            if symbol.get("name") == "[.sdata2]"
        )
        candidate["symbols"][section_index + 1:section_index + 1] = [
            _weak_symbol("_half$localstatic3$sqrtf__Ff", bytes.fromhex("3fe0000000000000"), None),
            _weak_symbol("_three$localstatic4$sqrtf__Ff", bytes.fromhex("4008000000000000"), 8),
        ]

        diagnosis = module.decode_function(report, "PoolFocus")["section_prefix_diagnosis"]
        self.assertEqual(diagnosis["status"], "matched")
        self.assertEqual(diagnosis["classification"], "candidate_only_weak_sqrtf_prefix")
        self.assertEqual(diagnosis["removable_prefix_bytes"], 16)
        self.assertEqual(diagnosis["predicted_candidate_section_size_bytes"], 464)
        self.assertEqual(diagnosis["predicted_downstream_owner_offset_delta_bytes"], -16)
        self.assertFalse(diagnosis["authority_advanced"])

    def test_sqrtf_prefix_diagnosis_fails_closed_on_inexact_evidence(self) -> None:
        base = _report()
        base["left"]["sections"] = [{"name": ".sdata2", "size": "560"}]
        base["right"]["sections"] = [{"name": ".sdata2", "size": "480"}]
        section_index = next(
            index for index, symbol in enumerate(base["right"]["symbols"])
            if symbol.get("name") == "[.sdata2]"
        )
        base["right"]["symbols"][section_index + 1:section_index + 1] = [
            _weak_symbol("_half$localstatic3$sqrtf__Ff", bytes.fromhex("3fe0000000000000"), None),
            _weak_symbol("_three$localstatic4$sqrtf__Ff", bytes.fromhex("4008000000000000"), 8),
        ]

        mutations = {
            "wrong_bits": lambda report: report["right"]["symbols"][section_index + 1].update(
                _weak_symbol("_half$localstatic3$sqrtf__Ff", bytes.fromhex("3ff0000000000000"), None)
            ),
            "wrong_name": lambda report: report["right"]["symbols"][section_index + 1].update(
                {"name": "_half$localstatic3$other__Ff"}
            ),
            "wrong_order": lambda report: report["right"]["symbols"].__setitem__(
                slice(section_index + 1, section_index + 3),
                list(reversed(report["right"]["symbols"][section_index + 1:section_index + 3])),
            ),
            "not_weak": lambda report: report["right"]["symbols"][section_index + 1].update(
                {"flags": {"global": True}}
            ),
            "target_also_owns": lambda report: report["left"]["symbols"].append(
                copy.deepcopy(report["right"]["symbols"][section_index + 1])
            ),
        }
        for name, mutate in mutations.items():
            with self.subTest(name=name):
                report = copy.deepcopy(base)
                mutate(report)
                diagnosis = module.decode_function(report, "PoolFocus")["section_prefix_diagnosis"]
                self.assertEqual(diagnosis["status"], "none")
                self.assertIsNone(diagnosis["classification"])

    def test_nonpool_addr_relocations_are_excluded(self) -> None:
        report = _report()
        for side_name in ("left", "right"):
            symbols = report[side_name]["symbols"]
            symbols.append({"name": "GwPlayer", "kind": "SYMBOL_OBJECT"})
            owner = len(symbols) - 1
            symbols[1]["instructions"].append(
                _instruction("lis r3, GwPlayer@ha", owner, type_name="R_PPC_ADDR16_HA")
            )
        result = module.decode_function(report, "PoolFocus", include_exact=True)
        self.assertEqual(result["target"]["pool_consumer_count"], 6)
        self.assertEqual(result["candidate"]["pool_consumer_count"], 5)

    def test_invalid_pairing_fails_closed(self) -> None:
        report = _report()
        report["right"]["symbols"][1].pop("target_symbol")
        with self.assertRaisesRegex(module.PoolDecodeError, "target pairing"):
            module.decode_function(report, "PoolFocus")

    def test_ambiguous_or_malformed_symbol_tables_fail_closed(self) -> None:
        duplicate = _report()
        duplicate["right"]["symbols"].append(copy.deepcopy(duplicate["right"]["symbols"][1]))
        with self.assertRaisesRegex(module.PoolDecodeError, "identity is ambiguous"):
            module.decode_function(duplicate, "PoolFocus")

        malformed = _report()
        malformed["left"]["symbols"].insert(0, None)
        with self.assertRaisesRegex(module.PoolDecodeError, "non-object entry"):
            module.decode_function(malformed, "PoolFocus")

    def test_oversized_data_diff_is_not_allocated(self) -> None:
        report = _report()
        report["left"]["symbols"][3]["size"] = "1048576"
        result = module.decode_function(report, "PoolFocus")
        group = next(item for item in result["groups"] if 0 in item["rows"])
        self.assertEqual(group["classification"], "unresolved_pool_bytes")

    def test_cli_is_deterministic(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(json.dumps(_report()), encoding="utf-8")
            command = [sys.executable, str(Path(module.__file__)), str(path), "PoolFocus"]
            first = subprocess.run(command, check=True, capture_output=True, text=True).stdout
            second = subprocess.run(command, check=True, capture_output=True, text=True).stdout
        self.assertEqual(first, second)
        self.assertEqual(len(json.loads(first)["decoder_sha256"]), 64)


if __name__ == "__main__":
    unittest.main()
