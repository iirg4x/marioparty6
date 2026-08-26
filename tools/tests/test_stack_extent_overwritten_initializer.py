from __future__ import annotations

import contextlib
import copy
import io
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


def _instruction(address: int, formatted: str, *, diff_kind: str | None = None) -> dict[str, object]:
    row: dict[str, object] = {
        "instruction": {"address": str(address), "size": 4, "formatted": formatted}
    }
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _report() -> dict[str, object]:
    target_text = [
        "stwu r1, -0xb0(r1)",
        "li r0, -1",
        "stw r0, 0x3c(r1)",
        "stw r0, 0x3c(r1)",
        "li r0, 0",
        "stw r0, 0x38(r1)",
        "stw r0, 0x34(r1)",
        "blr",
    ]
    candidate_text = list(target_text)
    candidate_text[3] = "stw r0, 0x40(r1)"
    return {
        "left": {
            "symbols": [
                {
                    "name": "MetalEffectCreate",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "1216",
                    "match_percent": 99.996710,
                    "instructions": [
                        _instruction(
                            100 + 4 * index,
                            formatted,
                            diff_kind="DIFF_ARG_MISMATCH" if index == 3 else None,
                        )
                        for index, formatted in enumerate(target_text)
                    ],
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": "MetalEffectCreate",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "1216",
                    "match_percent": 99.996710,
                    "instructions": [
                        _instruction(
                            100 + 4 * index,
                            formatted,
                            diff_kind="DIFF_ARG_MISMATCH" if index == 3 else None,
                        )
                        for index, formatted in enumerate(candidate_text)
                    ],
                }
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    source = "73b2637df0b9b1541ef3224b17ced76a807828c939f745457320f5bdc09ecd55"
    obj = "66bf2a468bbf9c1d44eea7fd45ee9d5345a93bb50cec3a373315c66e441bcb6a"
    strict = "407c8d38505d11aef9aa96b5b26ac29e0663ef4a24b95a944b9c6228d72fdebb"
    record = "c7abee29b345f93bb5e706c4cf1849799991d1f63adfb987f28d3aca7e5a432d"
    controls = [
        ("max_vertex_overwrite", "regressed"),
        ("self_chain", "regressed"),
        ("address_sizeof_visibility", "object_identical"),
        ("constant_bound_visibility", "object_identical"),
        ("two_word_array", "exact"),
        ("two_field_struct", "exact_same_object"),
        ("one_word_aggregate", "regressed"),
        ("particle_data_aliases", "regressed"),
        ("three_word_semantic_state", "regressed"),
        ("tracer_capture", "failed_closed_no_retry"),
    ]
    return {
        "schema": rules.STACK_EXTENT_OVERWRITTEN_INITIALIZER_CONTEXT_SCHEMA,
        "proofs": {
            "function_size_exact": True,
            "stack_frame_exact": True,
            "cfg_calls_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "stack_residue_authenticated": True,
            "target_home_chronology_authenticated": True,
            "overwritten_slot_one_write_zero_read": True,
            "duplicate_same_home_initializer_authenticated": True,
            "negative_controls_measured": True,
            "protected_siblings_preserved": True,
            "exact_result_verified": True,
            "authority_advanced": False,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "stack_residue_receipt_sha256": "3" * 64,
            "target_chronology_receipt_sha256": "4" * 64,
            "precursor_source_sha256": "5" * 64,
            "precursor_object_sha256": "6" * 64,
            "precursor_record_sha256": "7" * 64,
            "exact_source_sha256": source,
            "exact_object_sha256": obj,
            "exact_strict_report_sha256": strict,
            "exact_data_report_sha256": strict,
            "exact_record_sha256": record,
            "report_artifact_sha256": "053357edfe627839765f0f61b87a4afed23eb0e0a2fc17362ca1ab839d6a4db2",
        },
        "precursor": {
            "function": "MetalEffectCreate",
            "candidate_id": "player-metal-v813",
            "target_bytes": 1216,
            "candidate_bytes": 1216,
            "target_frame": 0xB0,
            "candidate_frame": 0xB0,
            "match_percent": 99.996710,
            "target_physical_relocations": 29,
            "candidate_physical_relocations": 29,
            "residual_rows": [3],
        },
        "stack_seam": {
            "base_register": "r1",
            "value_register": "r0",
            "access_width": 4,
            "selected_home_offset": 0x3C,
            "candidate_selected_home_offset": 0x40,
            "overwritten_slot_offset": 0x38,
            "adjacent_zero_offset": 0x34,
            "negative_initializer": -1,
            "zero_initializer": 0,
            "missing_extent_bytes": 4,
            "current_extent_bytes": 4,
            "target_extent_bytes": 8,
            "element_size": 4,
            "current_capacity": 1,
            "predicted_capacity": 2,
            "overwritten_write_count": 1,
            "overwritten_read_count": 0,
        },
        "controls": [
            {
                "kind": kind,
                "result_class": result,
                "candidate_record_sha256": f"{index + 10:064x}",
            }
            for index, (kind, result) in enumerate(controls)
        ],
        "provenance_boundary": {
            "owner": "main:board/player",
            "function": "MetalEffectCreate",
            "source_provenance_authenticated": False,
            "residue_reconstruction_only": True,
            "unused_second_element": True,
            "general_dead_storage_waiver": False,
            "promotion_authority": False,
        },
        "telemetry": {
            "parent_active_seconds": 10636.3622159,
            "active_seconds_measured": True,
            "parent_intervals_complete": True,
            "helper_coverage_complete": False,
            "candidate_heavy_coverage_complete": False,
            "throughput_complete": False,
            "exclude_from_measured_crack_hour": True,
            "no_imputation": True,
            "telemetry_sha256": "7019ffd46822fa466bdd2f2d7935518f3b9d13389535b3953f2c2ac733915cf4",
            "active_interval_log_sha256": "e8b6d12f73fc231542cefe3b2a975237b6cc40ed0989e7317aa2bf0abd046eea",
        },
        "exact_result": {
            "candidate_id": "player-metal-v828-two-word-array",
            "target_bytes": 1216,
            "candidate_bytes": 1216,
            "physical_relocations": 29,
            "source_sha256": source,
            "object_sha256": obj,
            "strict_report_sha256": strict,
            "data_report_sha256": strict,
            "candidate_record_sha256": record,
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "stack_extent_overwritten_initializer"
    )


class StackExtentOverwrittenInitializerTests(unittest.TestCase):
    def test_metal_report_schedules_only_two_word_extent(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol="MetalEffectCreate",
            stack_extent_overwritten_initializer_context=_context(report),
        )
        diagnosis = _evaluation(result)
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["source_class"],
            "two_word_stack_extent_with_duplicate_first_initializer",
        )
        cells = diagnosis["evidence"]["recommended_cells"]  # type: ignore[index]
        self.assertEqual(len(cells), 1)
        self.assertEqual(cells[0]["declaration"], "int objectNo[2]")
        self.assertEqual(cells[0]["initializer"], "objectNo[0] = objectNo[0] = -1")
        self.assertEqual(cells[0]["target_extent_bytes"], 8)
        provenance = diagnosis["evidence"]["provenance_boundary"]  # type: ignore[index]
        self.assertTrue(provenance["residue_reconstruction_only"])
        self.assertFalse(provenance["source_provenance_authenticated"])
        self.assertFalse(result["authority_advanced"])

    def test_suppresses_unsafe_and_measured_negative_axes(self) -> None:
        report = _report()
        diagnosis = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol="MetalEffectCreate",
                stack_extent_overwritten_initializer_context=_context(report),
            )
        )
        suppressed = set(diagnosis["evidence"]["suppressed_axes"])  # type: ignore[index]
        self.assertTrue(
            {
                "repeat_one_word_aggregate",
                "three_word_or_larger_state",
                "particle_data_aliases",
                "declaration_or_scope_permutations",
                "dead_or_fake_local",
                "padding",
                "register_shaping",
                "repeat_failed_closed_tracer",
                "claim_original_source_provenance",
                "general_dead_storage_waiver",
                "automatic_retention_or_promotion",
            }.issubset(suppressed)
        )
        self.assertEqual(len(diagnosis["evidence"]["negative_controls"]), 10)  # type: ignore[index]

    def test_fails_closed_without_context(self) -> None:
        result = rules.diagnose_document(_report(), focus_symbol="MetalEffectCreate")
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_provenance_control_and_telemetry_drift(self) -> None:
        report = _report()
        mutations: list[tuple[str, callable]] = [
            (
                "source_provenance",
                lambda value: value["provenance_boundary"].__setitem__(  # type: ignore[union-attr]
                    "source_provenance_authenticated", True
                ),
            ),
            (
                "dead_storage_waiver",
                lambda value: value["provenance_boundary"].__setitem__(  # type: ignore[union-attr]
                    "general_dead_storage_waiver", True
                ),
            ),
            (
                "wrong_control",
                lambda value: value["controls"][0].__setitem__(  # type: ignore[index,union-attr]
                    "result_class", "exact"
                ),
            ),
            (
                "throughput_claim",
                lambda value: value["telemetry"].__setitem__(  # type: ignore[union-attr]
                    "exclude_from_measured_crack_hour", False
                ),
            ),
            (
                "authority",
                lambda value: value["proofs"].__setitem__(  # type: ignore[union-attr]
                    "authority_advanced", True
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
                        focus_symbol="MetalEffectCreate",
                        stack_extent_overwritten_initializer_context=unsafe,
                    )

    def test_report_rejects_home_sequence_and_slot_access_drift(self) -> None:
        mutations: list[tuple[str, callable]] = [
            (
                "wrong_target_home",
                lambda value: value["left"]["symbols"][0]["instructions"][3][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "stw r0, 0x38(r1)"),
            ),
            (
                "wrong_value_register",
                lambda value: value["left"]["symbols"][0]["instructions"][2][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "stw r3, 0x3c(r1)"),
            ),
            (
                "slot_read",
                lambda value: value["left"]["symbols"][0]["instructions"].insert(  # type: ignore[index]
                    7, _instruction(128, "lfs f1, 0x38(r1)")
                ),
            ),
        ]
        for name, mutate in mutations:
            report = _report()
            mutate(report)
            context = _context(report)
            result = rules.diagnose_document(
                report,
                focus_symbol="MetalEffectCreate",
                stack_extent_overwritten_initializer_context=context,
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
                            "MetalEffectCreate",
                            "--stack-extent-overwritten-initializer-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="MetalEffectCreate",
                stack_extent_overwritten_initializer_context=context,
            ),
        )


if __name__ == "__main__":
    unittest.main()

