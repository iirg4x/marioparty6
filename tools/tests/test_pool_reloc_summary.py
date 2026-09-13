from __future__ import annotations

import base64
import copy
import json
import hashlib
import struct
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


def _chronology_report() -> dict[str, object]:
    target_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "Producer",
            "kind": "SYMBOL_FUNCTION",
            "size": "8",
            "instructions": [
                _instruction("lfs f0, lbl_missing@sda21", 4),
                _instruction("lfs f1, lbl_following@sda21", 5),
            ],
        },
        {
            "name": "Downstream",
            "kind": "SYMBOL_FUNCTION",
            "size": "12",
            "instructions": [
                _instruction("lfs f1, lbl_1800@sda21", 7),
                _instruction("lfs f1, lbl_2100@sda21", 8),
                _instruction("lfs f1, lbl_3200@sda21", 9),
            ],
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "32"},
        _symbol("lbl_missing", bytes.fromhex("409cccce"), 8),
        _symbol("lbl_following", bytes.fromhex("3f7ae148"), 12),
        _symbol("lbl_middle", bytes.fromhex("3f800000"), 16),
        _symbol("lbl_1800", bytes.fromhex("44e10000"), 20),
        _symbol("lbl_2100", bytes.fromhex("45034000"), 24),
        _symbol("lbl_3200", bytes.fromhex("45480000"), 28),
    ]
    for address, symbol in enumerate(target_symbols[1]["instructions"]):
        symbol["instruction"]["address"] = 100 + (address * 4)
    for address, symbol in enumerate(target_symbols[2]["instructions"]):
        symbol["instruction"]["address"] = 200 + (address * 4)

    candidate_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "Producer",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 1,
            "size": "8",
            "instructions": [
                _instruction("lfs f0, @old_shared@sda21", 4),
                _instruction("lfs f1, @following@sda21", 5),
            ],
        },
        {
            "name": "Downstream",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 2,
            "size": "12",
            "instructions": [
                _instruction("lfs f1, @1800@sda21", 7),
                _instruction("lfs f1, @2100@sda21", 8),
                _instruction("lfs f1, @3200@sda21", 9),
            ],
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "28"},
        _symbol("@old_shared", bytes.fromhex("409ccccd"), 0),
        _symbol("@following", bytes.fromhex("3f7ae148"), 8),
        _symbol("@middle", bytes.fromhex("3f800000"), 12),
        _symbol("@1800", bytes.fromhex("44e10000"), 16),
        _symbol("@2100", bytes.fromhex("45034000"), 20),
        _symbol("@3200", bytes.fromhex("45480000"), 24),
    ]
    for address, symbol in enumerate(candidate_symbols[1]["instructions"]):
        symbol["instruction"]["address"] = 80 + (address * 4)
    for address, symbol in enumerate(candidate_symbols[2]["instructions"]):
        symbol["instruction"]["address"] = 180 + (address * 4)
    return {"left": {"symbols": target_symbols}, "right": {"symbols": candidate_symbols}}


def _external_pool_family_report() -> dict[str, object]:
    target_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "Producer",
            "kind": "SYMBOL_FUNCTION",
            "size": "4",
            "instructions": [_instruction("lfs f2, lbl_missing@sda21", 6)],
        },
        {
            "name": "DownstreamA",
            "kind": "SYMBOL_FUNCTION",
            "size": "4",
            "instructions": [_instruction("lfs f0, lbl_later_a@sda21", 8)],
        },
        {
            "name": "DownstreamB",
            "kind": "SYMBOL_FUNCTION",
            "size": "4",
            "instructions": [_instruction("lfs f1, lbl_later_b@sda21", 9)],
        },
        {
            "name": "DownstreamC",
            "kind": "SYMBOL_FUNCTION",
            "size": "8",
            "instructions": [
                _instruction("lfd f0, lbl_successor@sda21", 7),
                _instruction("lfs f1, lbl_later_a@sda21", 8),
            ],
        },
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "32"},
        _symbol("lbl_missing", bytes.fromhex("c1f00000"), 8),
        _symbol("lbl_successor", bytes.fromhex("3ff0000000000000"), 16),
        _symbol("lbl_later_a", bytes.fromhex("42a00000"), 24),
        _symbol("lbl_later_b", bytes.fromhex("42c80000"), 28),
    ]
    candidate_symbols: list[dict[str, object]] = [
        {"name": "[.text]", "kind": "SYMBOL_SECTION"},
        {
            "name": "Producer",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 1,
            "size": "4",
            "instructions": [_instruction("lfs f2, lbl_missing@sda21", 6)],
        },
        {
            "name": "DownstreamA",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 2,
            "size": "4",
            "instructions": [_instruction("lfs f0, @later_a@sda21", 9)],
        },
        {
            "name": "DownstreamB",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 3,
            "size": "4",
            "instructions": [_instruction("lfs f1, @later_b@sda21", 10)],
        },
        {
            "name": "DownstreamC",
            "kind": "SYMBOL_FUNCTION",
            "target_symbol": 4,
            "size": "8",
            "instructions": [
                _instruction("lfd f0, @successor@sda21", 8),
                _instruction("lfs f1, @later_a@sda21", 9),
            ],
        },
        {"name": "unused_external", "flags": {"global": True}},
        {"name": "lbl_missing", "flags": {"global": True}},
        {"name": "[.sdata2]", "kind": "SYMBOL_SECTION", "size": "24"},
        _symbol("@successor", bytes.fromhex("3ff0000000000000"), 8),
        _symbol("@later_a", bytes.fromhex("42a00000"), 16),
        _symbol("@later_b", bytes.fromhex("42c80000"), 20),
    ]
    for side in (target_symbols, candidate_symbols):
        for function_index in range(1, 5):
            for row, instruction in enumerate(side[function_index]["instructions"]):
                instruction["instruction"]["address"] = (function_index * 100) + (row * 4)
    target_symbols[1]["instructions"][0].pop("diff_kind")
    candidate_symbols[1]["instructions"][0].pop("diff_kind")
    return {"left": {"symbols": target_symbols}, "right": {"symbols": candidate_symbols}}


def _external_data_report() -> dict[str, object]:
    instruction = _instruction("lis r3, lbl_data@ha", 3, type_name="R_PPC_ADDR16_HA")
    instruction["instruction"].update({
        "size": 4,
        "parts": [{"opcode": {"mnemonic": "lis", "opcode": 264}},
                  {"arg": {"opaque": "r3"}}, {"arg": {"reloc": True}}],
    })
    instruction["instruction"]["relocation"]["type"] = 6
    function = {"name": "ExternalFocus", "kind": "SYMBOL_FUNCTION", "size": "4",
                "target_symbol": 1, "instructions": [instruction]}
    return {
        "left": {"symbols": [
            {"name": "[.text]", "kind": "SYMBOL_SECTION"}, copy.deepcopy(function),
            {"name": "[.data]", "kind": "SYMBOL_SECTION"},
            _symbol("lbl_data", bytes.fromhex("43fa000044fa000043c80000"), 0),
        ]},
        "right": {"symbols": [
            {"name": "[.text]", "kind": "SYMBOL_SECTION"}, copy.deepcopy(function),
            {"name": "unused_external", "flags": {"global": True}},
            {"name": "lbl_data", "flags": {"global": True}},
        ]},
    }


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
        self.assertNotIn(
            0,
            [
                row
                for owner in full["tu_owner_consumer_census"]["owners"]
                for row in owner["focus_rows"]
            ],
        )

    def test_tu_owner_census_detects_named_subset_of_anonymous_pool(self) -> None:
        report = _report()
        for side_name, names in (
            ("left", ["ev_CapKettouStart"]),
            ("right", ["ev_CapKettouStart", "ev_CapDonkeyStart", "ev_CapKoopaStart"]),
        ):
            for name in names:
                report[side_name]["symbols"].append(
                    {
                        "name": name,
                        "kind": "SYMBOL_FUNCTION",
                        "instructions": [_instruction("lfs f0, pool@sda21", 3)],
                    }
                )

        census = module.decode_function(report, "PoolFocus")["tu_owner_consumer_census"]
        owner = next(item for item in census["owners"] if 0 in item["focus_rows"])
        self.assertEqual(
            owner["interpretation"],
            "target_named_owner_is_strict_consumer_subset_of_candidate_anonymous_pool",
        )
        self.assertEqual(owner["target"]["consumer_function_count"], 2)
        self.assertEqual(owner["candidate"]["consumer_function_count"], 4)
        self.assertEqual(
            [item["function"] for item in owner["target"]["consumers"]],
            ["PoolFocus", "ev_CapKettouStart"],
        )
        self.assertFalse(census["authority_advanced"])

    def partial_report(self):
        report = _report()
        for side in ('left', 'right'):
            report[side]['symbols'][1]['instructions'] = report[side]['symbols'][1]['instructions'][:2]
        for name in ('OtherA', 'OtherB'):
            report['left']['symbols'].append({'name': name, 'kind': 'SYMBOL_FUNCTION',
                'size': '4', 'instructions': [_instruction('lfs f1, lbl_zero@sda21', 3)]})
        # An undefined reference is not a candidate function definition.
        report['right']['symbols'].append({'name': 'OtherA', 'kind': 'SYMBOL_UNKNOWN'})
        return report

    def partial_summary(self, report, **limits):
        return module.decode_function(report, 'PoolFocus', **limits)['tu_owner_consumer_census']['partial_unit_dependencies']

    def test_partial_unit_shared_literal_and_definition_presence(self):
        report = self.partial_report()
        summary = self.partial_summary(report)
        self.assertEqual(summary['status'], 'partial_object_dependencies_observed')
        self.assertEqual(summary['absent_candidate_definitions'], ['OtherA', 'OtherB'])
        self.assertEqual(summary['dependency_owner_count'], 1)
        owner = summary['dependency_owners'][0]
        self.assertTrue(owner['owner_bytes_equal'])
        self.assertEqual(owner['target_owner']['bytes'], '00000000')
        self.assertEqual(owner['represented_candidate_definitions'], ['PoolFocus'])
        self.assertIn('source-selected split/link', summary['review'])
        self.assertIn('not unrecovered-source evidence', summary['review'])
        self.assertFalse(summary['authority_advanced'])
        report['right']['symbols'].pop()
        for index in (8, 9):
            report['right']['symbols'].append({**copy.deepcopy(report['left']['symbols'][index]), 'target_symbol': index})
        self.assertEqual(self.partial_summary(report)['status'], 'no_absent_definitions_observed')
        self.assertEqual(self.partial_summary(report)['absent_candidate_definitions'], [])

    def test_partial_unit_unknown_mapping_bytes_and_changed_bits(self):
        report = self.partial_report()
        report['right']['symbols'][3] = _symbol('@10', bytes.fromhex('3f800000'), 4)
        summary = self.partial_summary(report)
        self.assertFalse(summary['dependency_owners'][0]['owner_bytes_equal'])
        self.assertEqual(summary['equal_owner_bytes_count'], 0)
        self.assertEqual(summary['different_owner_bytes_count'], 1)
        report['right']['symbols'][3].pop('data_diff')
        self.assertEqual(self.partial_summary(report)['status'], 'unknown')
        self.assertIsNone(self.partial_summary(report)['dependency_owners'][0]['owner_bytes_equal'])
        report = self.partial_report()
        report['right']['symbols'].append(copy.deepcopy(report['right']['symbols'][3]))
        summary = self.partial_summary(report)
        self.assertEqual(summary['status'], 'unknown')
        self.assertIn('ambiguous_owner_identity', summary['dependency_owners'][0]['unknown_reasons'])
        report = self.partial_report()
        report['right']['symbols'][-1] = {**copy.deepcopy(report['left']['symbols'][8])}
        self.assertEqual(self.partial_summary(report)['unknown_candidate_definitions'], ['OtherA'])

    def test_partial_unit_totals_survive_truncation_and_exact_focus_filter(self):
        report = self.partial_report()
        full = self.partial_summary(report)
        small = self.partial_summary(report, group_limit=1, row_limit=1)
        self.assertEqual(small['absent_candidate_definition_count'], 2)
        self.assertEqual(small['absent_candidate_definitions_omitted'], 1)
        self.assertEqual(small['dependency_owner_count'], full['dependency_owner_count'])
        self.assertEqual(small['dependency_owners'][0]['focus_row_count'], 2)
        zero = self.partial_summary(report, group_limit=0)
        self.assertEqual(zero['dependency_owners'], [])
        self.assertEqual(zero['dependency_owners_omitted'], 1)
        self.assertEqual(zero['absent_candidate_definition_count'], 2)
        report['right']['symbols'][3] = copy.deepcopy(report['left']['symbols'][3])
        report['right']['symbols'][3]['target_symbol'] = 3
        exact = self.partial_summary(report)
        self.assertEqual(exact['absent_candidate_definitions'], ['OtherA', 'OtherB'])
        self.assertEqual(exact, self.partial_summary(report))

    def test_external_data_counterpart_retains_unknown_bytes_not_exactness(self) -> None:
        for reverse in (False, True):
            with self.subTest(reverse=reverse):
                report = _external_data_report()
                if reverse:
                    report["left"], report["right"] = report["right"], report["left"]
                result = module.decode_function(report, "ExternalFocus")
                group = result["groups"][0]
                self.assertEqual(group["classification"], "unresolved_external_data_reference")
                self.assertIn("literal_bytes_unresolved", group["differences"])
                self.assertEqual(result["summary"]["unresolved_value_or_contract_count"], 1)
                self.assertEqual(result["summary"]["semantic_or_contract_mismatch_count"], 0)
                self.assertEqual(result["summary"]["value_equivalent_owner_only_count"], 0)
                undefined, defined = ("target", "candidate") if reverse else ("candidate", "target")
                self.assertIsNone(group[undefined]["owner"]["bytes"])
                self.assertIsNone(group[undefined]["owner"]["typed"])
                self.assertTrue(group[undefined]["owner"]["external"])
                self.assertEqual(group[defined]["owner"]["bytes"], "43fa000044fa000043c80000")
                self.assertEqual(result["target"]["pool_consumer_count"], 1)
                self.assertEqual(result["candidate"]["pool_consumer_count"], 1)
                self.assertFalse(result["authority_advanced"])

    def test_external_data_changed_consumer_contract_is_not_suppressed(self) -> None:
        mutations = {
            "register": (lambda row, owner: row.update({"formatted": "lis r4, lbl_data@ha"}),
                         "external_data_reference_mismatch", "instruction_identity"),
            "opcode_parts": (lambda row, owner: row["parts"][0]["opcode"].update({"opcode": 999}),
                             "external_data_reference_mismatch", "instruction_identity"),
            "instruction_size": (lambda row, owner: row.update({"size": 8}),
                                 "external_data_reference_mismatch", "instruction_identity"),
            "reloc_type": (lambda row, owner: row["relocation"].update({"type_name": "R_PPC_ADDR32"}),
                           "relocation_type_mismatch", "relocation_type"),
            "reloc_type_code": (lambda row, owner: row["relocation"].update({"type": 4}),
                                "relocation_type_mismatch", "relocation_type"),
            "reloc_addend": (lambda row, owner: row["relocation"].update({"addend": 4}),
                             "relocation_addend_mismatch", "relocation_addend"),
            "owner_name": (lambda row, owner: owner.update({"name": "lbl_other_data"}),
                           "external_data_reference_mismatch", "owner_name"),
        }
        for name, (mutate, classification, difference) in mutations.items():
            with self.subTest(name=name):
                report = _external_data_report()
                symbols = report["right"]["symbols"]
                mutate(symbols[1]["instructions"][0]["instruction"], symbols[3])
                result = module.decode_function(report, "ExternalFocus")
                self.assertEqual(result["groups"][0]["classification"], classification)
                self.assertIn(difference, result["groups"][0]["differences"])
                self.assertEqual(result["summary"]["semantic_or_contract_mismatch_count"], 1)

    def test_external_data_missing_consumer_and_invalid_symbol_remain_errors(self) -> None:
        for mutation in ("missing_row", "missing_reloc", "invalid_owner", "not_external"):
            with self.subTest(mutation=mutation):
                report = _external_data_report()
                symbols = report["right"]["symbols"]
                instruction = symbols[1]["instructions"][0]["instruction"]
                if mutation == "missing_row":
                    symbols[1]["instructions"].clear()
                elif mutation == "missing_reloc":
                    instruction.pop("relocation")
                elif mutation == "invalid_owner":
                    instruction["relocation"]["target_symbol"] = 999
                else:
                    symbols[3]["flags"]["global"] = False
                result = module.decode_function(report, "ExternalFocus")
                self.assertEqual(result["groups"][0]["classification"], "target_only_pool_consumer")
                self.assertEqual(result["summary"]["semantic_or_contract_mismatch_count"], 1)

    def test_external_data_missing_instruction_evidence_does_not_pair_by_label(self) -> None:
        report = _external_data_report()
        for side in ("left", "right"):
            report[side]["symbols"][1]["instructions"][0]["instruction"]["formatted"] = ""
        result = module.decode_function(report, "ExternalFocus")
        self.assertEqual(result["groups"][0]["classification"], "unresolved_pool_bytes")
        self.assertIn("consumer_contract_unresolved", result["groups"][0]["differences"])

    def test_diagnostic_limits_preserve_census_totals_and_interpretation(self) -> None:
        report = _report()
        for side in ("left", "right"):
            symbols = report[side]["symbols"]
            symbols[1]["instructions"] *= 5
            for index in range(6):
                function = copy.deepcopy(symbols[1])
                function["name"] = f"Other{index}"
                symbols.append(function)
        full = module.decode_function(report, "PoolFocus", group_limit=100, row_limit=100)
        small = module.decode_function(report, "PoolFocus", group_limit=2, row_limit=2)
        self.assertEqual(small["summary"], full["summary"])
        census = small["tu_owner_consumer_census"]
        self.assertEqual(census["owner_count"], 5)
        self.assertEqual(census["owners_omitted"], 3)
        self.assertEqual(len(census["owners"]), 2)
        for owner, unbounded in zip(census["owners"], full["tu_owner_consumer_census"]["owners"]):
            self.assertEqual(owner["interpretation"], unbounded["interpretation"])
            self.assertEqual(owner["focus_row_count"], len(unbounded["focus_rows"]))
            self.assertEqual(owner["focus_rows_omitted"], owner["focus_row_count"] - 2)
            self.assertEqual(len(owner["focus_rows"]), 2)
            for side in ("target", "candidate"):
                detail = owner[side]
                self.assertEqual(detail["consumer_function_count"], 7)
                self.assertEqual(detail["consumer_relocation_count"], unbounded[side]["consumer_relocation_count"])
                self.assertEqual(detail["consumers_omitted"], 5)
                self.assertEqual(len(detail["consumers"]), 2)
                for consumer in detail["consumers"]:
                    self.assertEqual(len(consumer["rows"]), 2)
                    self.assertEqual(consumer["rows_omitted"], consumer["count"] - 2)
        zero = module.decode_function(report, "PoolFocus", group_limit=0, row_limit=1)
        self.assertEqual(zero["summary"], full["summary"])
        self.assertEqual(zero["tu_owner_consumer_census"]["owners"], [])
        self.assertEqual(zero["tu_owner_consumer_census"]["owners_omitted"], 5)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(json.dumps(report), encoding="utf-8")
            command = [sys.executable, str(Path(module.__file__)), str(path), "PoolFocus",
                       "--group-limit", "2", "--row-limit", "2"]
            first = subprocess.run(command, check=True, capture_output=True, text=True).stdout
            second = subprocess.run(command, check=True, capture_output=True, text=True).stdout
        self.assertEqual(first, second)
        self.assertLess(len(first), 22000)  # Includes bounded partial-unit dependency observations.

    def test_chronology_and_family_census_details_are_also_bounded(self) -> None:
        result = module.decode_function(_external_pool_family_report(), "DownstreamC", group_limit=1, row_limit=1)
        diagnosis = result["tu_pool_chronology_diagnosis"]
        self.assertEqual(len(diagnosis["affected_consumer"]["rows"]), 1)
        self.assertEqual(diagnosis["affected_consumer"]["rows_omitted"], 1)
        family = result["tu_pool_chronology_family"]
        self.assertEqual(family["affected_function_count"], 3)
        self.assertEqual(len(family["affected_functions"]), 1)
        self.assertEqual(family["affected_functions_omitted"], 2)
        self.assertEqual(len(family["downstream_body_edit_suppressed_functions"]), 1)
        self.assertEqual(family["downstream_body_edit_suppressed_functions_omitted"], 2)

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

    def test_tu_chronology_attributes_uniform_shift_to_missing_predecessor(self) -> None:
        report = _chronology_report()
        downstream = module.decode_function(report, "Downstream")["tu_pool_chronology_diagnosis"]
        self.assertEqual(downstream["status"], "matched")
        self.assertEqual(
            downstream["classification"],
            "missing_predecessor_pool_owner_causes_uniform_downstream_shift",
        )
        self.assertEqual(downstream["downstream_offset_delta_bytes"], 4)
        self.assertEqual(downstream["producer"]["function"], "Producer")
        self.assertEqual(downstream["producer"]["target"]["owner"]["bytes"], "409cccce")
        self.assertEqual(
            [item["row"] for item in downstream["affected_consumer"]["rows"]],
            [0, 1, 2],
        )
        self.assertTrue(downstream["affected_consumer"]["body_edit_suppressed"])
        self.assertIn("Do not edit Downstream", downstream["recommended_source_axis"])
        self.assertFalse(downstream["authority_advanced"])

        producer = module.decode_function(report, "Producer")["tu_pool_chronology_diagnosis"]
        self.assertEqual(producer["status"], "matched")
        self.assertEqual(producer["producer"]["function"], "Producer")
        self.assertEqual([item["row"] for item in producer["affected_consumer"]["rows"]], [1])
        self.assertFalse(producer["affected_consumer"]["body_edit_suppressed"])

    def test_tu_chronology_fails_closed_on_inexact_or_ambiguous_evidence(self) -> None:
        mutations = {
            "nonuniform_delta": lambda report: report["right"]["symbols"][8].update({"address": "21"}),
            "target_value_already_present": lambda report: report["right"]["symbols"].append(
                _symbol("@duplicate", bytes.fromhex("409cccce"), 28)
            ),
            "downstream_relocation_mismatch": lambda report: report["right"]["symbols"][2][
                "instructions"
            ][0]["instruction"]["relocation"].update({"type_name": "R_PPC_ADDR32"}),
        }
        for name, mutate in mutations.items():
            with self.subTest(name=name):
                report = _chronology_report()
                mutate(report)
                diagnosis = module.decode_function(report, "Downstream")["tu_pool_chronology_diagnosis"]
                self.assertEqual(diagnosis["status"], "none")
                self.assertIsNone(diagnosis["classification"])

    def test_external_contract_missing_physical_owner_groups_downstream_family(self) -> None:
        report = _external_pool_family_report()
        result = module.decode_function(report, "DownstreamA")
        diagnosis = result["tu_pool_chronology_diagnosis"]
        self.assertEqual(diagnosis["status"], "matched")
        self.assertEqual(diagnosis["producer"]["function"], "Producer")
        self.assertEqual(
            diagnosis["producer"]["source_contract"],
            "exact_external_contract_missing_physical_owner",
        )
        self.assertEqual(
            diagnosis["producer"]["physical_extent_contract"],
            "owner_size_plus_natural_successor_alignment",
        )
        self.assertEqual(diagnosis["downstream_offset_delta_bytes"], 8)
        self.assertTrue(diagnosis["affected_consumer"]["body_edit_suppressed"])

        family = result["tu_pool_chronology_family"]
        self.assertEqual(family["status"], "matched")
        self.assertEqual(
            family["classification"],
            "one_missing_pool_producer_explains_multiple_downstream_functions",
        )
        self.assertEqual(family["producer_edit_functions"], ["Producer"])
        self.assertEqual(
            [item["function"] for item in family["affected_functions"]],
            ["DownstreamA", "DownstreamB", "DownstreamC"],
        )
        self.assertEqual(
            family["downstream_body_edit_suppressed_functions"],
            ["DownstreamA", "DownstreamB", "DownstreamC"],
        )
        self.assertEqual(family["report_deduplication"], "treat_as_one_tu_pool_producer_family")
        self.assertFalse(family["authority_advanced"])

    def test_external_contract_family_fails_closed_without_physical_uniqueness(self) -> None:
        mutations = {
            "target_bytes_already_present": lambda report: report["right"]["symbols"].append(
                _symbol("@duplicate_missing", bytes.fromhex("c1f00000"), 24)
            ),
            "producer_has_multiple_consumers": lambda report: (
                report["left"]["symbols"][1]["instructions"].append(
                    _instruction("lfs f3, lbl_missing@sda21", 6)
                ),
                report["right"]["symbols"][1]["instructions"].append(
                    _instruction("lfs f3, lbl_missing@sda21", 6)
                ),
            ),
            "no_natural_alignment_witness": lambda report: report["right"]["symbols"][8].update(
                {"address": "4"}
            ),
        }
        for name, mutate in mutations.items():
            with self.subTest(name=name):
                report = _external_pool_family_report()
                mutate(report)
                diagnosis = module.decode_function(report, "DownstreamA")[
                    "tu_pool_chronology_diagnosis"
                ]
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


class SectionTailTests(unittest.TestCase):
    def elf(self, path: Path, raw: bytes, *, name: str = "real_provider", relocation: bool = False) -> Path:
        names = b"\0.rodata\0.shstrtab\0.strtab\0.symtab\0.rela.rodata\0"
        strings = b"\0" + name.encode() + b"\0"
        symbols = bytes(16) + struct.pack(">IIIBBH", 1, 0, len(raw), 0x11, 0, 1)
        chunks = [b"", raw, names, strings, symbols, bytes(12) if relocation else b""]
        image = bytearray(52 + 40 * len(chunks))
        image[:7] = b"\x7fELF\x01\x02\x01"
        struct.pack_into(">HHI", image, 16, 1, 20, 1)
        struct.pack_into(">I", image, 32, 52)
        struct.pack_into(">HHH", image, 46, 40, len(chunks), 2)
        for i, chunk in enumerate(chunks):
            label = [b"", b".rodata", b".shstrtab", b".strtab", b".symtab", b".rela.rodata"][i]
            struct.pack_into(">IIIIIIIIII", image, 52 + 40 * i,
                             names.index(label) if label else 0, [0, 1, 3, 3, 2, 4][i],
                             0, 0, len(image), len(chunk), 3 if i == 4 else 4 if i == 5 else 0,
                             1 if i == 5 else 0, 4 if i == 1 else 1, 16 if i == 4 else 12 if i == 5 else 0)
            image.extend(chunk)
        path.write_bytes(image)
        return path

    def test_provider_fit_and_hash_binding(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            target = self.elf(root / "target.o", b"ab" + bytes(22))
            candidate = self.elf(root / "candidate.o", b"ab")
            provider = self.elf(root / "provider.o", bytes(12))
            result = module.diagnose_section_tail(target, candidate, [provider])
            self.assertEqual(result["status"], "inspect_split_boundary")
            self.assertEqual(result["leading_alignment_gap"], 6)
            self.assertEqual(result["matches"][0]["trailing_alignment_gap"], 4)
            self.assertEqual(result["matches"][0]["symbol_size"], 12)
            for entry, path in zip(result["objects"], [target, candidate, provider]):
                self.assertEqual(entry["sha256"], hashlib.sha256(path.read_bytes()).hexdigest())
            self.assertFalse(result["ownership_authenticated"])
            self.assertFalse(result["authority_advanced"])
            self.assertEqual(module.diagnose_section_tail(candidate, candidate, [])["status"], "identical")
            command = [sys.executable, str(Path(module.__file__)), "section-tail", str(target), str(candidate), "--provider", str(provider)]
            cli = json.loads(subprocess.run(command, check=True, capture_output=True, text=True).stdout)
            self.assertEqual(cli["status"], result["status"])
            self.assertNotIn("decoder_sha256", cli)
            self.assertEqual(cli["decoder_code_sha256"], hashlib.sha256(Path(module.__file__).read_bytes()).hexdigest())
            result_digest = cli.pop("result_sha256")
            self.assertEqual(result_digest, module._canonical_sha256(cli))
            payload = b"actual value"
            self.elf(provider, payload)
            self.elf(target, b"ab" + bytes(6) + payload + bytes(4))
            self.assertEqual(module.diagnose_section_tail(target, candidate, [provider])["status"], "inspect_split_boundary")

    def test_alignment_and_rejections(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            candidate = self.elf(root / "candidate.o", b"ab")
            provider = self.elf(root / "provider.o", bytes(12))
            target = self.elf(root / "target.o", b"ab" + bytes(6))
            self.assertEqual(module.diagnose_section_tail(target, candidate, [])["status"], "alignment_only")
            target = self.elf(target, b"ab" + bytes(22))
            other = self.elf(root / "other.o", bytes(12), name="other")
            self.assertEqual(module.diagnose_section_tail(target, candidate, [provider, other])["status"], "ambiguous")
            for label, options in [("wrong_bytes", {"raw": b"wrong bytes!"}),
                                   ("missing_symbol", {"raw": bytes(12), "name": ""}),
                                   ("relocation", {"raw": bytes(12), "relocation": True})]:
                with self.subTest(label=label):
                    self.elf(provider, **options)
                    self.assertEqual(module.diagnose_section_tail(target, candidate, [provider])["status"], "unknown")
            self.assertEqual(module.diagnose_section_tail(target, candidate, [], alignment=3)["reason"], "unsafe_alignment")
            self.assertEqual(module.diagnose_section_tail(target, candidate, [], section=".missing")["reason"], "missing_or_ambiguous_section")
            self.elf(target, b"ab" + bytes(22), relocation=True)
            self.assertEqual(module.diagnose_section_tail(target, candidate, [])["reason"], "relocations_in_compared_section")
            self.elf(target, b"wrong")
            self.assertEqual(module.diagnose_section_tail(target, candidate, [])["reason"], "target_prefix_mismatch")

    def test_malformed_object_cli_has_clean_deterministic_error(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "malformed.o"
            path.write_bytes(b"not ELF")
            command = [sys.executable, str(Path(module.__file__)), "section-tail", str(path), str(path)]
            first = subprocess.run(command, capture_output=True, text=True)
            second = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(first.returncode, 2)
            self.assertEqual(first.stderr, second.stderr)
            self.assertIn("cannot parse object", first.stderr)
            self.assertIn(str(path), first.stderr)
            self.assertNotIn("Traceback", first.stderr)
            self.assertEqual(first.stdout, "")


if __name__ == "__main__":
    unittest.main()
