from __future__ import annotations

import copy
import unittest

from tools import crack_first_pass as first_pass
from tools import complete_stack_home_exchange as stack_home
from tools import typed_pool_owner_manifest as manifest
from tools.tests.test_complete_stack_home_exchange import (
    FUNCTION as HANACHAN_FUNCTION,
    binding as hanachan_binding,
    reports as hanachan_reports,
)
from tools.tests.test_typed_pool_owner_manifest import FUNCTION, binding, reports


class CrackFirstPassTests(unittest.TestCase):
    def assert_first_mismatch_policy(self, result: dict) -> None:
        self.assertEqual(result["candidate_budget"], 1)
        actions = " ".join(result["actions"]).lower()
        self.assertIn("earliest mismatch", actions)
        self.assertIn("one highest-ranked evidence-backed cell", actions)
        self.assertIn("pivot/recompute", actions)
        self.assertIn("optional", actions)
        self.assertNotIn("at most three", actions)

    def test_routes_owner_only_residual_to_one_candidate(self) -> None:
        strict, data = reports()
        owner_result = manifest.build_manifest(strict, data, FUNCTION, binding())
        result = first_pass.route_manifest(owner_result)
        self.assertEqual(result["route"], "typed_pool_owner_manifest")
        self.assertEqual(result["trace_budget"], 0)
        self.assertEqual(len(result["actions"]), 3)
        self.assert_first_mismatch_policy(result)
        self.assertFalse(result["authority_advanced"])
        self.assertFalse(result["source_patch_emitted"])

    def test_routes_value_mismatch_to_typed_pool_decoder(self) -> None:
        strict, data = reports()
        strict["right"]["symbols"][3]["data_diff"] = copy.deepcopy(
            strict["right"]["symbols"][4]["data_diff"]
        )
        owner_result = manifest.build_manifest(strict, data, FUNCTION, binding())
        result = first_pass.route_manifest(owner_result)
        self.assertEqual(owner_result["status"], "blocked")
        self.assertEqual(result["route"], "typed_pool_decoder")
        self.assert_first_mismatch_policy(result)
        self.assertEqual(result["trace_budget"], 0)

    def test_routes_non_pool_residual_to_causal_reducer(self) -> None:
        strict, data = reports()
        for side in (strict["left"], strict["right"]):
            function = side["symbols"][1]
            for row in function["instructions"]:
                row.pop("diff_kind", None)
            function["instructions"][10]["diff_kind"] = "DIFF_ARG_MISMATCH"
        owner_result = manifest.build_manifest(strict, data, FUNCTION, binding())
        result = first_pass.route_manifest(owner_result)
        self.assertEqual(result["route"], "causal_reducer")
        self.assert_first_mismatch_policy(result)
        self.assertEqual(result["trace_budget"], 1)

    def test_routes_mixed_residual_in_causal_then_pool_order(self) -> None:
        strict, data = reports()
        strict["left"]["symbols"][1]["instructions"][10]["diff_kind"] = "DIFF_ARG_MISMATCH"
        strict["right"]["symbols"][1]["instructions"][10]["diff_kind"] = "DIFF_ARG_MISMATCH"
        owner_result = manifest.build_manifest(strict, data, FUNCTION, binding())
        result = first_pass.route_manifest(owner_result)
        self.assertEqual(result["route"], "causal_reducer_then_typed_pool_decoder")
        self.assert_first_mismatch_policy(result)
        self.assertEqual(result["trace_budget"], 1)

    def test_routes_complete_stack_home_exchange_before_generic_causal_reducer(self) -> None:
        strict, data = hanachan_reports()
        owner_result = manifest.build_manifest(strict, data, HANACHAN_FUNCTION, hanachan_binding())
        diagnosis = stack_home.build_diagnosis(
            strict, data, HANACHAN_FUNCTION, hanachan_binding()
        )
        result = first_pass.route_manifest(owner_result, diagnosis)

        self.assertEqual(owner_result["status"], "blocked")
        self.assertEqual(diagnosis["status"], "matched")
        self.assertEqual(result["route"], stack_home.ROUTE)
        self.assert_first_mismatch_policy(result)
        self.assertEqual(result["trace_budget"], 0)
        self.assertEqual(result["facts"]["stack_home_row_count"], 46)
        self.assertEqual(result["facts"]["stack_home_pool_handoff_row_count"], 1)

    def test_triage_hash_is_canonical(self) -> None:
        strict, data = reports()
        result = first_pass.route_manifest(manifest.build_manifest(strict, data, FUNCTION, binding()))
        digest = result["triage_sha256"]
        unhashed = copy.deepcopy(result)
        unhashed.pop("triage_sha256")
        self.assertEqual(digest, manifest.canonical_sha256(unhashed))


if __name__ == "__main__":
    unittest.main()
