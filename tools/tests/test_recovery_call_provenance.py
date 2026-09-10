from __future__ import annotations

import unittest

from tools import recovery_call_provenance as provenance


def _row(address: int, text: str, *, branch_dest: int | None = None) -> dict[str, object]:
    instruction: dict[str, object] = {
        "address": address,
        "formatted": text,
        "size": 4,
    }
    if branch_dest is not None:
        instruction["branch_dest"] = branch_dest
    return {"instruction": instruction}


def _contract(*arguments: str, return_r3: bool) -> dict[str, object]:
    return {"arguments": list(arguments), "return_r3": return_r3}


CONTRACTS = {
    "make": _contract(return_r3=True),
    "use": _contract("r3", return_r3=False),
}


class RecoveryCallProvenanceTests(unittest.TestCase):
    def test_register_permutation_with_same_origin_is_equal(self) -> None:
        target = [
            _row(0, "bl make"),
            _row(4, "mr r4, r3"),
            _row(8, "mr r3, r4"),
            _row(12, "bl use"),
        ]
        candidate = [
            _row(0, "bl make"),
            _row(4, "mr r5, r3"),
            _row(8, "mr r3, r5"),
            _row(12, "bl use"),
        ]
        result = provenance.compare(target, candidate, CONTRACTS)
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["finding_count"], 0)
        self.assertEqual(result["unknown_count"], 0)
        self.assertEqual(result["equal_argument_count"], 1)
        self.assertFalse(result["authority_advanced"])

    def test_entry_and_derived_producers_survive_register_permutation(self) -> None:
        target = [
            _row(0, "addi r7, r3, 4"),
            _row(4, "mr r4, r7"),
            _row(8, "lwz r8, 0(r6)"),
            _row(12, "mr r5, r8"),
            _row(16, "bl use"),
        ]
        candidate = [
            _row(0, "addi r8, r3, 4"),
            _row(4, "mr r4, r8"),
            _row(8, "lwz r7, 0(r6)"),
            _row(12, "mr r5, r7"),
            _row(16, "bl use"),
        ]
        contracts = {"use": _contract("r4", "r5", return_r3=False)}
        result = provenance.compare(target, candidate, contracts, entry_arguments=("r3", "r6"))
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["equal_argument_count"], 2)

    def test_derived_immediate_change_is_a_value_origin_mismatch(self) -> None:
        target = [_row(0, "addi r4, r3, 4"), _row(4, "bl use")]
        candidate = [_row(0, "addi r4, r3, 8"), _row(4, "bl use")]
        contracts = {"use": _contract("r4", return_r3=False)}
        result = provenance.compare(target, candidate, contracts, entry_arguments=("r3",))
        self.assertEqual(result["status"], "mismatch")
        self.assertEqual(result["findings"][0]["classification"], "different_value_origin")

    def test_li_and_lis_producers_are_explicitly_compared(self) -> None:
        target = [_row(0, "li r4, 1"), _row(4, "lis r5, 1"), _row(8, "bl use")]
        candidate = [_row(0, "li r4, 1"), _row(4, "lis r5, 2"), _row(8, "bl use")]
        contracts = {"use": _contract("r4", "r5", return_r3=False)}
        result = provenance.compare(target, candidate, contracts)
        self.assertEqual(result["status"], "mismatch")
        self.assertEqual(result["equal_argument_count"], 1)
        self.assertEqual(result["findings"][0]["classification"], "different_value_origin")

    def test_equal_consumer_assembly_can_have_wrong_origin(self) -> None:
        # The branch metadata selects different aligned paths while the
        # formatted branch and consumer text remain equal.
        target = [
            _row(0, "bl make"),
            _row(4, "b 0x10", branch_dest=12),
            _row(8, "bl make"),
            _row(12, "bl use"),
        ]
        candidate = [
            _row(0, "bl make"),
            _row(4, "b 0x10", branch_dest=8),
            _row(8, "bl make"),
            _row(12, "bl use"),
        ]
        result = provenance.compare(target, candidate, CONTRACTS)
        self.assertEqual(result["status"], "mismatch")
        self.assertEqual(result["finding_count"], 1)
        finding = result["findings"][0]
        self.assertEqual(finding["classification"], "different_call_return_origin")
        self.assertEqual(finding["target_origin"], {"row": 0, "callee": "make"})
        self.assertEqual(finding["candidate_origin"], {"row": 2, "callee": "make"})

    def test_call_clobber_is_unknown_not_a_mismatch(self) -> None:
        target = [_row(0, "bl make"), _row(4, "bl helper"), _row(8, "bl use")]
        candidate = [_row(0, "bl make"), _row(4, "nop"), _row(8, "bl use")]
        result = provenance.compare(target, candidate, CONTRACTS)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertIn("call_clobber", result["unknowns"][0]["target_causes"])

    def test_missing_entry_declaration_is_unknown(self) -> None:
        rows = [_row(0, "bl use")]
        result = provenance.compare(rows, rows, {"use": _contract("r3", return_r3=False)})
        self.assertEqual(result["status"], "unknown")
        self.assertEqual(result["unknowns"][0]["reason"], "entry_undefined")

    def test_entry_arguments_must_be_distinct_declared_parameters(self) -> None:
        rows = [_row(0, "bl use")]
        contract = {"use": _contract("r3", return_r3=False)}
        with self.assertRaises(ValueError):
            provenance.compare(rows, rows, contract, entry_arguments=("r3", "r3"))
        with self.assertRaises(ValueError):
            provenance.compare(rows, rows, contract, entry_arguments=("r2",))

    def test_ambiguous_entry_producers_at_join_are_unknown(self) -> None:
        rows = [
            _row(0, "beq 0xc", branch_dest=12),
            _row(4, "mr r5, r3"),
            _row(8, "b 0x10", branch_dest=16),
            _row(12, "mr r5, r4"),
            _row(16, "bl use"),
        ]
        result = provenance.compare(
            rows,
            rows,
            {"use": _contract("r5", return_r3=False)},
            entry_arguments=("r3", "r4"),
        )
        self.assertEqual(result["status"], "unknown")
        self.assertEqual(result["unknowns"][0]["reason"], "producer_path_ambiguous")

    def test_ambiguous_branch_destination_is_unknown(self) -> None:
        target = [
            _row(0, "bl make"),
            _row(4, "b 0x8", branch_dest=8),
            _row(8, "nop"),
            _row(8, "bl use"),
        ]
        result = provenance.compare(target, target, CONTRACTS)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertTrue(result["cfg_unknowns"])

    def test_loop_join_is_conservative_unknown(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "bne 0xc", branch_dest=12),
            _row(8, "bl make"),
            _row(12, "bne 0x4", branch_dest=4),
            _row(16, "bl use"),
        ]
        result = provenance.compare(rows, rows, CONTRACTS)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertEqual(result["unknowns"][0]["reason"], "producer_path_ambiguous")

    def test_unsupported_definition_invalidates_destination(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "add r4, r3, r5"),
            _row(8, "bl use"),
        ]
        result = provenance.compare(rows, rows, {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)})
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertIn("unsupported_definition", result["unknowns"][0]["target_causes"])

    def test_overlapping_stack_store_invalidates_spill(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "stw r3, 0(r1)"),
            _row(8, "stfd f0, 2(r1)"),
            _row(12, "lwz r4, 0(r1)"),
            _row(16, "bl use"),
        ]
        result = provenance.compare(rows, rows, {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)})
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)

    def test_call_memory_side_effect_invalidates_stack_reload(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "stw r3, 0(r1)"),
            _row(8, "bl helper"),
            _row(12, "lwz r4, 0(r1)"),
            _row(16, "bl use"),
        ]
        result = provenance.compare(rows, rows, {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)})
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertIn("call_memory_side_effect", result["unknowns"][0]["target_causes"])

    def test_update_address_load_does_not_preserve_stack_origin(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "stw r3, 0(r1)"),
            _row(8, "lwzu r4, 0(r1)"),
            _row(12, "bl use"),
        ]
        contracts = {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)}
        result = provenance.compare(rows, rows, contracts)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertEqual(result["unknowns"][0]["reason"], "unsupported_definition")

    def test_lmw_invalidates_all_loaded_register_origins(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "lmw r4, 0(r1)"),
            _row(8, "bl use"),
        ]
        contracts = {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)}
        result = provenance.compare(rows, rows, contracts)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertEqual(result["unknowns"][0]["reason"], "unsupported_definition")

    def test_stack_width_join_does_not_recover_narrow_reload(self) -> None:
        rows = [
            _row(0, "bl make"),
            _row(4, "beq 0x10", branch_dest=16),
            _row(8, "stw r3, 0(r1)"),
            _row(12, "b 0x14", branch_dest=20),
            _row(16, "stfd f0, 0(r1)"),
            _row(20, "lwz r4, 0(r1)"),
            _row(24, "bl use"),
        ]
        contracts = {"make": CONTRACTS["make"], "use": _contract("r4", return_r3=False)}
        result = provenance.compare(rows, rows, contracts)
        self.assertEqual(result["finding_count"], 0)
        self.assertGreaterEqual(result["unknown_count"], 1)
        # One branch carries the narrow spill while the other invalidates the
        # overlapping wide slot; the join must remain path-ambiguous.
        self.assertEqual(result["unknowns"][0]["reason"], "producer_path_ambiguous")

    def test_stack_width_join_fact_is_explicitly_unknown(self) -> None:
        origin = provenance._Origin(3, "make")
        left = provenance._initial_state()
        right = provenance._initial_state()
        left.memory[0] = provenance._MemCell(0, 4, provenance._Fact.known(origin))
        right.memory[0] = provenance._MemCell(0, 8, provenance._Fact.known(origin))
        joined = provenance._join_states(left, right)
        self.assertIsNone(joined.memory[0].fact.origin)
        self.assertEqual(joined.memory[0].fact.causes, frozenset({"stack_width_join"}))

    def test_unused_argument_is_not_assumed_or_compared(self) -> None:
        target = [_row(0, "bl make"), _row(4, "li r4, 1"), _row(8, "bl use")]
        candidate = [_row(0, "bl make"), _row(4, "li r4, 2"), _row(8, "bl use")]
        result = provenance.compare(target, candidate, CONTRACTS)
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["checked_argument_count"], 1)

    def test_derived_origin_depth_bound_fails_closed(self) -> None:
        rows = [
            _row(0, "addi r4, r3, 1"),
            _row(4, "addi r5, r4, 2"),
            _row(8, "addi r6, r5, 3"),
            _row(12, "addi r7, r6, 4"),
            _row(16, "addi r8, r7, 5"),
            _row(20, "bl use"),
        ]
        result = provenance.compare(
            rows,
            rows,
            {"use": _contract("r8", return_r3=False)},
            entry_arguments=("r3",),
        )
        self.assertEqual(result["status"], "unknown")
        self.assertEqual(result["unknowns"][0]["reason"], "derived_origin_unresolved")

    def test_derived_origin_depth_limit_is_inclusive(self) -> None:
        rows = [
            _row(0, "addi r4, r3, 1"),
            _row(4, "addi r5, r4, 2"),
            _row(8, "addi r6, r5, 3"),
            _row(12, "addi r7, r6, 4"),
            _row(16, "bl use"),
        ]
        result = provenance.compare(
            rows,
            rows,
            {"use": _contract("r7", return_r3=False)},
            entry_arguments=("r3",),
        )
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["equal_argument_count"], 1)

    def test_output_is_bounded_and_contracts_are_explicit(self) -> None:
        contracts = {"use": _contract("r3", return_r3=False)}
        rows = [_row(index * 4, "bl use") for index in range(40)]
        result = provenance.compare(rows, rows, contracts)
        self.assertLessEqual(len(result["findings"]), 16)
        self.assertLessEqual(len(result["unknowns"]), 16)
        self.assertFalse(result["authority_advanced"])
        with self.assertRaises(ValueError):
            provenance.compare(rows, rows, {"use": {"arguments": ["r3"]}})


if __name__ == "__main__":
    unittest.main()
