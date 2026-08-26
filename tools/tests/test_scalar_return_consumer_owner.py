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


def _report() -> dict[str, object]:
    target_text: list[tuple[str | None, str | None]] = [
        ("stwu r1, -0x60(r1)", "DIFF_ARG_MISMATCH"),
        ("stfd f31, 0x58(r1)", None),
        ("stfd f30, 0x50(r1)", None),
        ("stfd f29, 0x48(r1)", "DIFF_INSERT"),
        ("fmuls f31, f1, f2", None),
        ("bl mbSinDeg", None),
        ("fmr f30, f1", None),
        ("fmr f29, f30", "DIFF_INSERT"),
        ("fmuls f0, f31, f29", "DIFF_ARG_MISMATCH"),
        ("lfd f29, 0x48(r1)", "DIFF_INSERT"),
        ("lfd f30, 0x50(r1)", None),
        ("lfd f31, 0x58(r1)", None),
        ("blr", None),
    ]
    candidate_text: list[tuple[str | None, str | None]] = [
        ("stwu r1, -0x50(r1)", "DIFF_ARG_MISMATCH"),
        ("stfd f31, 0x48(r1)", None),
        ("stfd f30, 0x40(r1)", None),
        (None, "DIFF_INSERT"),
        ("fmuls f31, f1, f2", None),
        ("bl mbSinDeg", None),
        ("fmr f30, f1", None),
        (None, "DIFF_INSERT"),
        ("fmuls f0, f31, f30", "DIFF_ARG_MISMATCH"),
        (None, "DIFF_INSERT"),
        ("lfd f30, 0x40(r1)", None),
        ("lfd f31, 0x48(r1)", None),
        ("blr", None),
    ]
    return {
        "left": {
            "symbols": [
                {
                    "name": "mbev_CapEffSnowOMExec",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "544",
                    "match_percent": 96.19853,
                    "instructions": [
                        _instruction(100 + 4 * index, formatted, diff_kind=kind)
                        for index, (formatted, kind) in enumerate(target_text)
                    ],
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": "mbev_CapEffSnowOMExec",
                    "kind": "SYMBOL_FUNCTION",
                    "address": "100",
                    "size": "524",
                    "match_percent": 96.19853,
                    "instructions": [
                        _instruction(100 + 4 * index, formatted, diff_kind=kind)
                        for index, (formatted, kind) in enumerate(candidate_text)
                    ],
                }
            ]
        },
    }


def _context(report: dict[str, object]) -> dict[str, object]:
    trace_envelope = "713e8763100b22ff093018d6cbde6dfa0e2d7f41499d02505bc10d03544869c1"
    trace_causal = "6ad0d556368a41f9b546ba40082c1f8d18d71aaa1aa636f9c5e6f3e863d06013"
    source = "c132f1d6424a56c781bd7d5e2989bd07193222d713f11e1d659ec4abe3c98692"
    obj = "5ba11356a1c9f17354abfda210dff6de74b96a954feb647e1cc4d78d81caa515"
    strict = "5b2c005b104318e99fbcb9aab5bc6d7f2dc1b38d0e45edc6b7ee7601b427ddb8"
    data = "ab99beaff4ec35c7fd7c68057c366a253095b6fd55683923e2873997f0b3b6db"
    record = "8b418f44d62e2e332c3364bbf329f48906a41fd9d4744e993f9fa58caa72e67f"
    return {
        "schema": rules.SCALAR_RETURN_CONSUMER_CONTEXT_SCHEMA,
        "proofs": {
            "cfg_calls_exact": True,
            "data_values_exact": True,
            "physical_relocations_exact": True,
            "target_copy_use_save_chain_authenticated": True,
            "source_aware_trace_authenticated": True,
            "input_owner_exact": True,
            "call_result_owner_exact": True,
            "missing_consumer_owner_isolated": True,
            "negative_controls_measured": True,
            "protected_siblings_preserved": True,
            "exact_result_verified": True,
            "objdiff_canonical_sha256": rules._sha256(rules._canonical(report)),
            "strict_report_sha256": "1" * 64,
            "data_report_sha256": "2" * 64,
            "trace_envelope_sha256": trace_envelope,
            "trace_causal_receipt_sha256": trace_causal,
            "exact_source_sha256": source,
            "exact_object_sha256": obj,
            "exact_strict_report_sha256": strict,
            "exact_data_report_sha256": data,
            "exact_record_sha256": record,
            "report_artifact_sha256": "39850692f0bcc99a2c7bdf3fbca0e77cd5433b005c12e00941cdfbe1878a2b92",
        },
        "precursor": {
            "function": "mbev_CapEffSnowOMExec",
            "candidate_id": "snow-two-local-baseline",
            "target_bytes": 544,
            "candidate_bytes": 524,
            "target_frame": 0x60,
            "candidate_frame": 0x50,
            "match_percent": 96.19853,
            "target_physical_relocations": 25,
            "candidate_physical_relocations": 25,
            "residual_rows": [0, 1, 2, 3, 7, 8, 9, 10, 11],
        },
        "target_chain": {
            "call_symbol": "mbSinDeg",
            "input_owner": "angle",
            "input_register": "f31",
            "call_result_owner": "sinAngle",
            "call_result_register": "f30",
            "consumer_owner": "sinValue",
            "consumer_register": "f29",
            "return_register": "f1",
            "copy_opcode": "fmr",
            "use_opcode": "fmuls",
            "save_opcode": "stfd",
            "restore_opcode": "lfd",
            "stack_slot_offset": 0x48,
        },
        "source_trace": {
            "same_session": True,
            "authority_advanced": False,
            "seam_unknown_count": 0,
            "input_owner_status": "EXACT",
            "input_owner_register": "f31",
            "call_result_owner_status": "EXACT",
            "call_result_owner_register": "f30",
            "consumer_owner_status": "TARGET_ONLY_MISSING",
            "consumer_target_register": "f29",
            "event_count": 251,
            "envelope_sha256": trace_envelope,
            "causal_receipt_sha256": trace_causal,
        },
        "controls": [
            {
                "kind": "lexical_scope_or_spelling",
                "result_class": "object_identical",
                "candidate_record_sha256": "3" * 64,
            },
            {
                "kind": "call_result_assignment_fusion",
                "result_class": "regressed",
                "candidate_record_sha256": "4" * 64,
            },
            {
                "kind": "declaration_chronology",
                "result_class": "exhausted_neutral_or_regressed",
                "candidate_record_sha256": "5" * 64,
            },
            {
                "kind": "existing_input_owner_copy",
                "result_class": "wrong_owner",
                "candidate_record_sha256": "3330a98fba6ac1364b1eddd08d15534ff0cce44c42004f7ef78791cf8501c01d",
            },
            {
                "kind": "complementary_existing_owner_copy",
                "result_class": "dominated_wrong_chain",
                "candidate_record_sha256": "c44dc41c4336b5e0668d594df339d9e76f7a788a599ded29736ebf613926ba7a",
            },
            {
                "kind": "consumer_boundary_existing_owner_assignment",
                "result_class": "wrong_owner",
                "candidate_record_sha256": "7ed91584dadacfbe054e76d981644e103432940f116b45eab563e062cc9521a0",
            },
        ],
        "telemetry": {
            "active_seconds": 1294.10743,
            "telemetry_complete": False,
            "exclude_from_measured_crack_hour": True,
            "telemetry_sha256": "d8ab87750f3af14df1b9e7d83cf5017ee0ce37e81c2864bbd5334df4fa1bf4f1",
        },
        "exact_result": {
            "candidate_id": "snow-third-live-sine-owner",
            "target_bytes": 544,
            "candidate_bytes": 544,
            "physical_relocations": 25,
            "source_sha256": source,
            "object_sha256": obj,
            "strict_report_sha256": strict,
            "data_report_sha256": data,
            "candidate_record_sha256": record,
        },
    }


def _evaluation(result: dict[str, object]) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == "scalar_return_consumer_owner_chain"
    )


class ScalarReturnConsumerOwnerTests(unittest.TestCase):
    def test_snow_report_schedules_only_distinct_consumer_owner(self) -> None:
        report = _report()
        result = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapEffSnowOMExec",
            scalar_return_consumer_context=_context(report),
        )
        diagnosis = _evaluation(result)
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["source_class"],
            "scalar_return_to_distinct_saved_consumer_owner",
        )
        cell = diagnosis["evidence"]["recommended_cells"][0]  # type: ignore[index]
        self.assertEqual(cell["call_assignment"], "sinAngle = mbSinDeg(angle)")
        self.assertEqual(cell["copy_assignment"], "sinValue = sinAngle")
        self.assertEqual(len(diagnosis["evidence"]["recommended_cells"]), 1)  # type: ignore[index]
        self.assertTrue(
            diagnosis["evidence"]["telemetry"][  # type: ignore[index]
                "exclude_from_measured_crack_hour"
            ]
        )
        self.assertFalse(result["authority_advanced"])

    def test_suppresses_measured_controls_and_repeat_trace(self) -> None:
        report = _report()
        diagnosis = _evaluation(
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffSnowOMExec",
                scalar_return_consumer_context=_context(report),
            )
        )
        suppressed = set(diagnosis["evidence"]["suppressed_axes"])  # type: ignore[index]
        self.assertTrue(
            {
                "lexical_scope_or_spelling",
                "call_result_assignment_fusion",
                "declaration_permutations",
                "existing_input_owner_copy",
                "consumer_boundary_existing_owner_assignment",
                "dead_or_fake_consumer_owner",
                "repeat_tracer_capture",
                "automatic_retention",
            }.issubset(suppressed)
        )
        self.assertEqual(len(diagnosis["evidence"]["negative_controls"]), 6)  # type: ignore[index]

    def test_fails_closed_without_context(self) -> None:
        result = rules.diagnose_document(
            _report(), focus_symbol="mbev_CapEffSnowOMExec"
        )
        self.assertFalse(_evaluation(result)["matched"])

    def test_context_rejects_trace_control_and_telemetry_drift(self) -> None:
        report = _report()
        mutations: list[tuple[str, callable]] = [
            (
                "unknown_seam",
                lambda value: value["source_trace"].__setitem__(  # type: ignore[union-attr]
                    "seam_unknown_count", 1
                ),
            ),
            (
                "wrong_owner_register",
                lambda value: value["source_trace"].__setitem__(  # type: ignore[union-attr]
                    "consumer_target_register", "f28"
                ),
            ),
            (
                "wrong_control",
                lambda value: value["controls"][0].__setitem__(  # type: ignore[index,union-attr]
                    "result_class", "regressed"
                ),
            ),
            (
                "unexcluded_telemetry",
                lambda value: value["telemetry"].__setitem__(  # type: ignore[union-attr]
                    "exclude_from_measured_crack_hour", False
                ),
            ),
            (
                "authority_advanced",
                lambda value: value["source_trace"].__setitem__(  # type: ignore[union-attr]
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
                        focus_symbol="mbev_CapEffSnowOMExec",
                        scalar_return_consumer_context=unsafe,
                    )

    def test_report_rejects_copy_use_save_call_and_bind_drift(self) -> None:
        mutations: list[tuple[str, callable]] = [
            (
                "copy_drift",
                lambda value: value["left"]["symbols"][0]["instructions"][7][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "fmr f28, f30"),
            ),
            (
                "use_drift",
                lambda value: value["left"]["symbols"][0]["instructions"][8][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "fmuls f0, f31, f28"),
            ),
            (
                "save_drift",
                lambda value: value["left"]["symbols"][0]["instructions"][3][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "stfd f29, 0x40(r1)"),
            ),
            (
                "call_drift",
                lambda value: value["right"]["symbols"][0]["instructions"][5][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "bl mbCosDeg"),
            ),
            (
                "bind_drift",
                lambda value: value["right"]["symbols"][0]["instructions"][6][  # type: ignore[index]
                    "instruction"
                ].__setitem__("formatted", "fmr f29, f1"),
            ),
        ]
        for name, mutate in mutations:
            report = _report()
            context = _context(report)
            mutate(report)
            result = rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffSnowOMExec",
                scalar_return_consumer_context=context,
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
                            "mbev_CapEffSnowOMExec",
                            "--scalar-return-consumer-context",
                            str(context_path),
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(
                report,
                focus_symbol="mbev_CapEffSnowOMExec",
                scalar_return_consumer_context=context,
            ),
        )


if __name__ == "__main__":
    unittest.main()
