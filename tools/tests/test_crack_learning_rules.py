from __future__ import annotations

import contextlib
import io
import json
import re
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules


def _instruction(
    address: int,
    formatted: str,
    *,
    diff_kind: str | None = None,
    branch_dest: int | None = None,
) -> dict[str, object]:
    nested: dict[str, object] = {
        "address": str(address),
        "size": 4,
        "formatted": formatted,
    }
    if branch_dest is not None:
        nested["branch_dest"] = str(branch_dest)
    row: dict[str, object] = {"instruction": nested}
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _placeholder(diff_kind: str = "DIFF_DELETE") -> dict[str, object]:
    return {"diff_kind": diff_kind}


def _function(
    name: str,
    instructions: list[dict[str, object]],
    *,
    size: int | None = None,
    match_percent: float = 90.0,
) -> dict[str, object]:
    return {
        "name": name,
        "kind": "SYMBOL_FUNCTION",
        "address": "100",
        "size": str(size if size is not None else len(instructions) * 4),
        "match_percent": match_percent,
        "instructions": instructions,
    }


def _report(
    focus: str,
    target: list[dict[str, object]],
    candidate: list[dict[str, object]],
    *,
    target_size: int | None = None,
    candidate_size: int | None = None,
    extra_pairs: tuple[
        tuple[dict[str, object], dict[str, object]], ...
    ] = (),
) -> dict[str, object]:
    return {
        "left": {
            "symbols": [
                _function(focus, target, size=target_size),
                *(pair[0] for pair in extra_pairs),
            ]
        },
        "right": {
            "symbols": [
                _function(focus, candidate, size=candidate_size),
                *(pair[1] for pair in extra_pairs),
            ]
        },
    }


def _evaluation(result: dict[str, object], rule_id: str) -> dict[str, object]:
    return next(
        item
        for item in result["evaluations"]  # type: ignore[union-attr]
        if item["rule_id"] == rule_id
    )


def _assignment_cycle_report() -> dict[str, object]:
    target_text = [
        "stwu r1, -32(r1)",
        "mr r29, r4",
        "mr r30, r5",
        "mr r31, r6",
        "bl get_result",
        "mr r29, r3",
        "cmpwi r29, 0",
        "bne 0x84",
        "mr r3, r30",
        "mr r4, r31",
        "blr",
    ]
    candidate_text = [
        "stwu r1, -32(r1)",
        "mr r30, r4",
        "mr r31, r5",
        "mr r29, r6",
        "bl get_result",
        "mr r30, r3",
        "cmpwi r30, 0",
        "bne 0x84",
        "mr r3, r31",
        "mr r4, r29",
        "blr",
    ]
    target: list[dict[str, object]] = []
    candidate: list[dict[str, object]] = []
    for index, (left, right) in enumerate(zip(target_text, candidate_text)):
        address = 100 + index * 4
        branch_dest = 132 if left.startswith("bne") else None
        kind = "DIFF_ARG_MISMATCH" if left != right else None
        target.append(
            _instruction(address, left, diff_kind=kind, branch_dest=branch_dest)
        )
        candidate.append(
            _instruction(address, right, diff_kind=kind, branch_dest=branch_dest)
        )
    return _report("ev_CapMiracleCoinTrade", target, candidate)


def _switch_fpr_report(*, include_switch: bool = True) -> dict[str, object]:
    target = [
        _instruction(100, "stwu r1, -160(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "mflr r0"),
        _instruction(108, "stfd f26, 96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "stfd f25, 104(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bctr" if include_switch else "nop"),
        _instruction(120, "bl sin"),
        _instruction(124, "fmr f26, f1", diff_kind="DIFF_DELETE"),
        _instruction(128, "nop"),
        _instruction(132, "bl sin"),
        _instruction(136, "fmr f25, f1", diff_kind="DIFF_DELETE"),
        _instruction(140, "nop"),
        _instruction(144, "bl cos"),
        _instruction(148, "fmr f24, f1", diff_kind="DIFF_DELETE"),
        _instruction(152, "blr"),
    ]
    candidate = [
        _instruction(100, "stwu r1, -96(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(104, "mflr r0"),
        _instruction(108, "stfd f26, 32(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(112, "stfd f25, 40(r1)", diff_kind="DIFF_ARG_MISMATCH"),
        _instruction(116, "bctr" if include_switch else "nop"),
        _instruction(120, "bl sin"),
        _placeholder(),
        _instruction(128, "nop"),
        _instruction(132, "bl sin"),
        _placeholder(),
        _instruction(140, "nop"),
        _instruction(144, "bl cos"),
        _placeholder(),
        _instruction(152, "blr"),
    ]
    return _report(
        "ev_CapBobleOMExec",
        target,
        candidate,
        target_size=2000,
        candidate_size=1928,
    )


def _aggregate_report(*, donor_exact: bool = True) -> dict[str, object]:
    prefix = [_instruction(100 + index * 4, "nop") for index in range(12)]
    copy = [
        _instruction(148, "lfs f0, 32(r1)", diff_kind="DIFF_DELETE"),
        _instruction(152, "lfs f1, 36(r1)", diff_kind="DIFF_DELETE"),
        _instruction(156, "lfs f2, 40(r1)", diff_kind="DIFF_DELETE"),
        _instruction(160, "stfs f0, 32(r1)", diff_kind="DIFF_DELETE"),
        _instruction(164, "stfs f1, 36(r1)", diff_kind="DIFF_DELETE"),
        _instruction(168, "stfs f2, 40(r1)", diff_kind="DIFF_DELETE"),
    ]
    consumers = [
        _instruction(172, "bl mbPlayerPosSetV"),
        _instruction(176, "bl mbPlayerRotSetV"),
        _instruction(180, "blr"),
    ]
    focus_target = [*prefix, *copy, *consumers]
    focus_candidate = [
        *prefix,
        *(_placeholder() for _ in copy),
        *consumers,
    ]
    donor_copy = [
        _instruction(148, "lfs f0, 48(r1)"),
        _instruction(152, "lfs f1, 52(r1)"),
        _instruction(156, "lfs f2, 56(r1)"),
        _instruction(160, "stfs f0, 48(r1)"),
        _instruction(164, "stfs f1, 52(r1)"),
        _instruction(168, "stfs f2, 56(r1)"),
    ]
    donor_instructions = [*prefix, *donor_copy, *consumers]
    percent = 100.0 if donor_exact else 99.0
    donor_pair = (
        _function("mbev_CapBobleMove", donor_instructions, match_percent=percent),
        _function("mbev_CapBobleMove", donor_instructions, match_percent=percent),
    )
    return _report(
        "mbev_CapBomheiMove",
        focus_target,
        focus_candidate,
        extra_pairs=(donor_pair,),
    )


class CrackLearningRulesTest(unittest.TestCase):
    def test_explicit_else_return_reuses_causal_reducer_signature(self) -> None:
        target = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(
                104,
                "bne 0x74",
                diff_kind="DIFF_ARG_MISMATCH",
                branch_dest=116,
            ),
            _instruction(108, "bl body"),
            _instruction(112, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            _instruction(116, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            _instruction(120, "blr"),
        ]
        candidate = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(
                104,
                "bne 0x78",
                diff_kind="DIFF_ARG_MISMATCH",
                branch_dest=120,
            ),
            _instruction(108, "bl body"),
            _placeholder(),
            _placeholder(),
            _instruction(120, "blr"),
        ]
        result = rules.diagnose_document(
            _report("ev_CapTeresaFadeMatHook", target, candidate),
            focus_symbol="ev_CapTeresaFadeMatHook",
        )
        diagnosis = _evaluation(result, "explicit_else_return_cfg")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["confidence"], 0.98)
        self.assertEqual(
            diagnosis["evidence"]["causal_classification"],  # type: ignore[index]
            "explicit_else_return_epilogue",
        )

    def test_assignment_in_condition_requires_closed_saved_gpr_cycle(self) -> None:
        result = rules.diagnose_document(
            _assignment_cycle_report(), focus_symbol="ev_CapMiracleCoinTrade"
        )
        diagnosis = _evaluation(result, "assignment_condition_saved_gpr_cycle")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["register_mapping"],  # type: ignore[index]
            {"r29": "r30", "r30": "r31", "r31": "r29"},
        )
        self.assertEqual(
            len(diagnosis["evidence"]["call_result_consumers"]),  # type: ignore[index]
            1,
        )

    def test_switch_case_scoped_fpr_lifetimes_require_frame_join(self) -> None:
        result = rules.diagnose_document(
            _switch_fpr_report(), focus_symbol="ev_CapBobleOMExec"
        )
        diagnosis = _evaluation(result, "switch_case_scoped_fpr_lifetimes")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(diagnosis["evidence"]["frame_delta"], 64)  # type: ignore[index]
        self.assertEqual(len(diagnosis["evidence"]["result_captures"]), 3)  # type: ignore[index]

    def test_final_consumer_self_copy_requires_exact_same_tu_donor(self) -> None:
        result = rules.diagnose_document(
            _aggregate_report(),
            focus_symbol="mbev_CapBomheiMove",
            same_tu_donor_symbols=("mbev_CapBobleMove",),
        )
        diagnosis = _evaluation(result, "aggregate_self_copy_final_consumer")
        self.assertTrue(diagnosis["matched"])
        self.assertEqual(
            diagnosis["evidence"]["focus_copy"]["component_count"],  # type: ignore[index]
            3,
        )
        self.assertEqual(
            diagnosis["evidence"]["exact_donors"][0]["symbol"],  # type: ignore[index]
            "mbev_CapBobleMove",
        )

    def test_adversarial_partial_signatures_are_rejected(self) -> None:
        cycle_report = _assignment_cycle_report()
        cycle_report["left"]["symbols"][0]["instructions"][4] = _instruction(116, "nop")  # type: ignore[index]
        cycle_report["right"]["symbols"][0]["instructions"][4] = _instruction(116, "nop")  # type: ignore[index]
        cycle = rules.diagnose_document(
            cycle_report, focus_symbol="ev_CapMiracleCoinTrade"
        )
        self.assertFalse(
            _evaluation(cycle, "assignment_condition_saved_gpr_cycle")["matched"]
        )

        no_switch = rules.diagnose_document(
            _switch_fpr_report(include_switch=False), focus_symbol="ev_CapBobleOMExec"
        )
        self.assertFalse(
            _evaluation(no_switch, "switch_case_scoped_fpr_lifetimes")["matched"]
        )

        inexact_donor = rules.diagnose_document(
            _aggregate_report(donor_exact=False),
            focus_symbol="mbev_CapBomheiMove",
            same_tu_donor_symbols=("mbev_CapBobleMove",),
        )
        self.assertFalse(
            _evaluation(inexact_donor, "aggregate_self_copy_final_consumer")["matched"]
        )

        else_target = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(104, "bne 0x74", branch_dest=116),
            _instruction(108, "bl body"),
            _instruction(112, "b 0x78", branch_dest=120),
            _instruction(116, "b 0x7c", branch_dest=124),
            _instruction(120, "blr"),
        ]
        else_candidate = [
            _instruction(100, "cmpwi r3, 1"),
            _instruction(104, "bne 0x78", branch_dest=120),
            _instruction(108, "bl body"),
            _placeholder(),
            _placeholder(),
            _instruction(120, "blr"),
        ]
        near_else = rules.diagnose_document(
            _report("near_else", else_target, else_candidate), focus_symbol="near_else"
        )
        self.assertFalse(_evaluation(near_else, "explicit_else_return_cfg")["matched"])

    def test_output_is_deterministic_self_hashed_and_authority_free(self) -> None:
        report = _assignment_cycle_report()
        first = rules.diagnose_document(
            report, focus_symbol="ev_CapMiracleCoinTrade"
        )
        second = rules.diagnose_document(
            report, focus_symbol="ev_CapMiracleCoinTrade"
        )
        self.assertEqual(first, second)
        self.assertEqual(first["schema"], rules.SCHEMA)
        self.assertFalse(first["authority_advanced"])
        self.assertRegex(first["diagnosis_sha256"], re.compile(r"^[0-9a-f]{64}$"))
        body = dict(first)
        claimed = body.pop("diagnosis_sha256")
        self.assertEqual(claimed, rules._sha256(rules._canonical(body)))
        for diagnosis in first["diagnoses"]:
            self.assertIn("confidence", diagnosis)
            self.assertTrue(diagnosis["evidence"])
            self.assertNotIn("retain", diagnosis["recommendation"].lower())

    def test_direct_cli_emits_the_same_closed_document(self) -> None:
        report = _assignment_cycle_report()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(json.dumps(report), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(
                    rules.main(
                        [
                            "--report",
                            str(path),
                            "--function",
                            "ev_CapMiracleCoinTrade",
                        ]
                    ),
                    0,
                )
        self.assertEqual(
            json.loads(output.getvalue()),
            rules.diagnose_document(report, focus_symbol="ev_CapMiracleCoinTrade"),
        )


if __name__ == "__main__":
    unittest.main()
