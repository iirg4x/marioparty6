from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


def _instruction(
    address: int,
    formatted: str | None,
    *,
    diff_kind: str | None = None,
) -> dict[str, object]:
    row: dict[str, object] = {}
    if formatted is not None:
        row["instruction"] = {
            "address": str(address),
            "size": 4,
            "formatted": formatted,
        }
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _report(
    function: str = "mbev_CapCoinManCreate",
    *,
    target_bytes: int = 328,
    candidate_bytes: int = 324,
) -> dict[str, object]:
    target_text: list[tuple[str | None, str | None]] = [
        ("stwu r1, -0x30(r1)", "DIFF_ARG_MISMATCH"),
        ("bl HuMemDirectMallocNum", None),
        ("mr r31, r3", None),
        ("stw r31, 0x20(r30)", None),
        ("stw r31, 0x08(r1)", "DIFF_INSERT"),
        ("mr r3, r31", None),
        ("li r4, 0", None),
        ("li r5, 0xe00", None),
        ("bl memset", None),
        ("blr", None),
    ]
    candidate_text: list[tuple[str | None, str | None]] = [
        ("stwu r1, -0x20(r1)", "DIFF_ARG_MISMATCH"),
        ("bl HuMemDirectMallocNum", None),
        ("mr r31, r3", None),
        ("stw r31, 0x20(r30)", None),
        (None, "DIFF_INSERT"),
        ("mr r3, r31", None),
        ("li r4, 0", None),
        ("li r5, 0xe00", None),
        ("bl memset", None),
        ("blr", None),
    ]
    target = [
        _instruction(100 + 4 * index, formatted, diff_kind=kind)
        for index, (formatted, kind) in enumerate(target_text)
    ]
    candidate = [
        _instruction(100 + 4 * index, formatted, diff_kind=kind)
        for index, (formatted, kind) in enumerate(candidate_text)
    ]
    return {
        "left": {
            "symbols": [
                {
                    "name": function,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": str(target_bytes),
                    "match_percent": 98.70731,
                    "instructions": target,
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": function,
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": str(candidate_bytes),
                    "match_percent": 98.70731,
                    "instructions": candidate,
                }
            ]
        },
    }


def _context(
    report: dict[str, object],
    *,
    function: str = "mbev_CapCoinManCreate",
    target_bytes: int = 328,
    candidate_bytes: int = 324,
    relocations: int = 19,
    element_type: str = "CAPCOINMANWORK",
    element_count: int = 64,
    report_sha256: str = "a" * 64,
) -> dict[str, object]:
    return {
        "schema": rules.LIVE_ALIAS_MEMSET_CONTEXT_SCHEMA,
        "proofs": {
            "cfg_calls_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "allocation_contract_authenticated": True,
            "target_store_forward_order_authenticated": True,
            "historical_alias_authenticated": True,
            "negative_controls_measured": True,
            "protected_siblings_preserved": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "history_receipt_sha256": "3" * 64,
            "exact_source_sha256": "2665129bbd7f5aeaf5671e4b4eb4bc07de6d0535a297456a53005b3e1b55fc3a",
            "exact_object_sha256": "0a963393e274f2c1f0e1db349b27a17c621c3c614f5377ae996a155ee2500d61",
            "exact_strict_report_sha256": "ef87fc6985a123a1f37e63c18ea34b6b8050ec356e7188633c054909ff00047e",
            "exact_data_report_sha256": "7c76eaa9b1aac98b85a1a451338778bc83e5a1247e98cb728bb703ec6bd6c52e",
            "exact_record_sha256": "45eb96bd49ecbb0e9dcd94002e641c1310f2d2dd3f6653d4fa9e7a909ab27cda",
            "report_artifact_sha256": report_sha256,
        },
        "precursor": {
            "function": function,
            "candidate_id": f"{function}-baseline",
            "target_bytes": target_bytes,
            "candidate_bytes": candidate_bytes,
            "target_frame": 0x30,
            "candidate_frame": 0x20,
            "match_percent": 98.70731,
            "target_physical_relocations": relocations,
            "candidate_physical_relocations": relocations,
            "residual_rows": [0, 4],
            "target_home_store": {
                "opcode": "stw",
                "register": "r31",
                "base_register": "r1",
                "offset": 8,
                "candidate_absent": True,
            },
        },
        "producer_consumer": {
            "allocation_symbol": "HuMemDirectMallocNum",
            "consumer_symbol": "memset",
            "allocation_owner": "workData",
            "field_owner": "obj",
            "field_name": "data",
            "live_owner": "workP",
            "element_type": element_type,
            "element_count": element_count,
            "zero_value": 0,
            "return_register": "r3",
            "live_register": "r31",
            "destination_order": ["workBase", "workP", "obj.data", "workData"],
        },
        "historical_alias": {
            "name": "workBase",
            "type": element_type,
            "commit": "1a19f8b5",
            "declaration_authenticated": True,
            "live_at_consumer_boundary": True,
            "outer_assignment": True,
            "stack_home_offset": 8,
        },
        "controls": [
            {
                "kind": "fused_without_alias",
                "result_class": "object_identical",
                "candidate_record_sha256": "ea330c19685deb3d306c70f1893dd661c219544b5847d9d4a7c9c9052c91a959",
            },
            {
                "kind": "direct_allocator_chain",
                "result_class": "regressed",
                "candidate_record_sha256": "5543348c170891c3ac9d9b61d95574fdefa629d3f10fe7993e6f2d66dc7d054b",
            },
            {
                "kind": "typed_allocation_owner",
                "result_class": "object_identical",
                "candidate_record_sha256": "de1aeb6eef1a23064cc5e842f3e9f42b5da8a3ae6cb98764bae6a7226e4b77c2",
            },
            {
                "kind": "sizeof_owner",
                "result_class": "object_identical",
                "candidate_record_sha256": "9f640d63d9bcc40c33bc8c46ccc99faa0ad61198de801cf8d8658bc7ac3b4fcc",
            },
            {
                "kind": "separate_historical_alias",
                "result_class": "size_exact_saved_gpr_regression",
                "candidate_record_sha256": "b31263e524592ca997dbcc5035c979091cdbcde89265ee70d7d55c1bf2aaf67b",
            },
        ],
        "telemetry": {
            "active_seconds": 310.9609309,
            "telemetry_complete": False,
            "exclude_from_measured_crack_hour": True,
            "telemetry_sha256": "d1d2d61f849834bff4052d8b0cb69b1d585b533b39bbc017f7debeaba6d6e37d",
        },
        "exact_result": {
            "candidate_id": "creators-workbase-memset-fusion001",
            "target_bytes": target_bytes,
            "candidate_bytes": target_bytes,
            "physical_relocations": relocations,
            "source_sha256": "2665129bbd7f5aeaf5671e4b4eb4bc07de6d0535a297456a53005b3e1b55fc3a",
            "object_sha256": "0a963393e274f2c1f0e1db349b27a17c621c3c614f5377ae996a155ee2500d61",
            "strict_report_sha256": "ef87fc6985a123a1f37e63c18ea34b6b8050ec356e7188633c054909ff00047e",
            "data_report_sha256": "7c76eaa9b1aac98b85a1a451338778bc83e5a1247e98cb728bb703ec6bd6c52e",
            "candidate_record_sha256": "45eb96bd49ecbb0e9dcd94002e641c1310f2d2dd3f6653d4fa9e7a909ab27cda",
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "historical_live_alias_memset_fusion"
    )


class LiveAliasMemsetFusionTests(unittest.TestCase):
    def test_coin_and_star_creators_share_one_exact_rule(self) -> None:
        cases = [
            (
                "mbev_CapCoinManCreate",
                328,
                324,
                19,
                "CAPCOINMANWORK",
                64,
                "c65d924e68f00a8c50d791f6e855541a5173c5d95740f43a16c365bdfbce2593",
            ),
            (
                "mbev_CapStarManCreate",
                340,
                336,
                21,
                "CAPSTARMANWORK",
                8,
                "b5a8c8ebd0d14bef860795de38989ecb9ee23884c7d07b63e2d000e578088801",
            ),
        ]
        for function, target_bytes, candidate_bytes, relocs, work_type, count, report_hash in cases:
            report = _report(
                function, target_bytes=target_bytes, candidate_bytes=candidate_bytes
            )
            context = _context(
                report,
                function=function,
                target_bytes=target_bytes,
                candidate_bytes=candidate_bytes,
                relocations=relocs,
                element_type=work_type,
                element_count=count,
                report_sha256=report_hash,
            )
            with self.subTest(function=function):
                result = rules.diagnose_document(
                    report,
                    focus_symbol=function,
                    live_alias_memset_context=context,
                )
                diagnosis = _evaluation(result)
                self.assertTrue(diagnosis["matched"])
                self.assertEqual(
                    diagnosis["source_class"],
                    "historical_live_alias_outer_assignment_at_memset_boundary",
                )
                cell = diagnosis["evidence"]["recommended_cells"][0]  # type: ignore[index]
                self.assertEqual(
                    cell["destination_expression"],
                    "workBase = workP = obj.data = workData",
                )
                self.assertEqual(cell["element_count"], count)
                self.assertTrue(
                    diagnosis["evidence"]["telemetry"][  # type: ignore[index]
                        "exclude_from_measured_crack_hour"
                    ]
                )
                self.assertFalse(result["authority_advanced"])

    def test_suppresses_every_measured_negative_control(self) -> None:
        report = _report()
        diagnosis = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapCoinManCreate",
                live_alias_memset_context=_context(report),
            )
        )
        suppressed = set(diagnosis["evidence"]["suppressed_axes"])  # type: ignore[index]
        self.assertTrue(
            {
                "fusion_without_authenticated_alias",
                "direct_allocator_result_chain",
                "type_only_alias",
                "sizeof_owner_changes",
                "separate_alias_statement",
                "dead_or_fake_aliases",
                "automatic_retention",
            }.issubset(suppressed)
        )
        self.assertEqual(len(diagnosis["evidence"]["negative_controls"]), 5)  # type: ignore[index]

    def test_fails_closed_without_context(self) -> None:
        result = rules.diagnose_document(
            _report(), focus_symbol="mbev_CapCoinManCreate"
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_unsafe_or_incomplete_evidence(self) -> None:
        report = _report()
        mutations: list[tuple[str, callable]] = [
            (
                "unauthenticated_alias",
                lambda value: value["historical_alias"].__setitem__(  # type: ignore[union-attr]
                    "declaration_authenticated", False
                ),
            ),
            (
                "wrong_control",
                lambda value: value["controls"][0].__setitem__(  # type: ignore[index,union-attr]
                    "result_class", "regressed"
                ),
            ),
            (
                "missing_control",
                lambda value: value["controls"].pop(),  # type: ignore[union-attr]
            ),
            (
                "not_minus_four",
                lambda value: value["precursor"].__setitem__(  # type: ignore[union-attr]
                    "candidate_bytes", 320
                ),
            ),
            (
                "unexcluded_telemetry",
                lambda value: value["telemetry"].__setitem__(  # type: ignore[union-attr]
                    "exclude_from_measured_crack_hour", False
                ),
            ),
            (
                "dead_alias",
                lambda value: value["historical_alias"].__setitem__(  # type: ignore[union-attr]
                    "live_at_consumer_boundary", False
                ),
            ),
        ]
        for name, mutate in mutations:
            unsafe = _context(report)
            mutate(unsafe)
            with self.subTest(name=name):
                with self.assertRaises(rules.LearningInputError):
                    rules.diagnose_document(
                        report,
                        focus_symbol="mbev_CapCoinManCreate",
                        live_alias_memset_context=unsafe,
                    )

    def test_report_rejects_call_bind_and_home_drift(self) -> None:
        mutations: list[tuple[str, callable]] = [
            (
                "wrong_home",
                lambda value: value["left"]["symbols"][0]["instructions"][4][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "stw r31, 0x0c(r1)"),
            ),
            (
                "candidate_home",
                lambda value: value["right"]["symbols"][0]["instructions"][4].__setitem__(  # type: ignore[index]
                    "instruction",
                    {"address": "116", "size": 4, "formatted": "stw r31, 0x08(r1)"},
                ),
            ),
            (
                "call_drift",
                lambda value: value["right"]["symbols"][0]["instructions"][8][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "bl memset_alt"),
            ),
            (
                "bind_drift",
                lambda value: value["right"]["symbols"][0]["instructions"][2][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "mr r30, r3"),
            ),
        ]
        for name, mutate in mutations:
            report = _report()
            mutate(report)
            result = rules.diagnose_document(
                report,
                focus_symbol="mbev_CapCoinManCreate",
                live_alias_memset_context=_context(report),
            )
            with self.subTest(name=name):
                self.assertFalse(_evaluation(result)["matched"])

    def test_cli_emits_same_document(self) -> None:
        report = _report()
        context = _context(report)
        with tempfile.TemporaryDirectory() as directory:
            report_path = Path(directory) / "report.json"
            context_path = Path(directory) / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(report_path),
                            "--function",
                            "mbev_CapCoinManCreate",
                            "--live-alias-memset-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapCoinManCreate",
                live_alias_memset_context=context,
            ),
        )


if __name__ == "__main__":
    unittest.main()
