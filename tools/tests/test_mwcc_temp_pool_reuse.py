from __future__ import annotations

import copy
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import mwcc_temp_pool_reuse as pool


FUNCTION = "Focus"
MACHINE_WORD = 0x38600001  # li r3, 1


def _row(address: int, text: str, *, word: int | None = MACHINE_WORD) -> dict[str, object]:
    instruction: dict[str, object] = {
        "address": str(address),
        "size": 4,
        "formatted": text,
    }
    if word is not None:
        instruction["ppc_word"] = word
    return {"instruction": instruction}


def _fixture(
    target_roles: list[int],
    virtual_ids: list[int],
    *,
    candidate_roles: list[int] | None = None,
    ordinals: list[int] | None = None,
    word: int = MACHINE_WORD,
    row_word: int | None = MACHINE_WORD,
) -> tuple[dict[str, object], dict[str, object]]:
    if len(target_roles) != len(virtual_ids):
        raise ValueError("fixture role and ID lengths differ")
    candidate_roles = candidate_roles or [3] * len(target_roles)
    if len(candidate_roles) != len(target_roles):
        raise ValueError("fixture target and candidate lengths differ")
    ordinals = ordinals or list(range(len(target_roles)))
    if len(ordinals) != len(target_roles):
        raise ValueError("fixture role and ordinal lengths differ")

    target_rows = [_row(0x1000 + index * 4, f"li r{role}, 1", word=row_word) for index, role in enumerate(target_roles)]
    candidate_rows = [_row(0x2000 + index * 4, f"li r{role}, 1", word=row_word) for index, role in enumerate(candidate_roles)]
    report: dict[str, object] = {
        "left": {"symbols": [{
            "name": FUNCTION,
            "address": "0x1000",
            "size": len(target_rows) * 4,
            "target_symbol": 0,
            "instructions": target_rows,
        }]},
        "right": {"symbols": [{
            "name": FUNCTION,
            "address": "0x2000",
            "size": len(candidate_rows) * 4,
            "target_symbol": 0,
            "instructions": candidate_rows,
        }]},
    }
    events: list[dict[str, object]] = []
    for index, (virtual_id, ordinal) in enumerate(zip(virtual_ids, ordinals)):
        token = f"pcode-{index}"
        events.append({
            "event_kind": "machine_emission",
            "function": FUNCTION,
            "instruction_index": index,
            "emitted_offset": index * 4,
            "pcode_token": token,
            "ppc_bytes": f"{word:08x}",
            "ppc_word": word,
        })
        events.append({
            "event_kind": "pcode_capture",
            "function": FUNCTION,
            "confirmed": True,
            "operand_bank": "GPR",
            "operand_ordinal": ordinal,
            "operand_index": virtual_id,
            "operand_flags": 2,
            "final_color": candidate_roles[index],
            "pcode_token": token,
        })
    envelope: dict[str, object] = {"events": events}
    return envelope, report


class MwccTempPoolReuseTests(unittest.TestCase):
    def test_hypothetical_wrap_fit_is_bounded_nonmutating_and_not_reset_authority(self):
        definitions = [{"machine_index": i, "virtual_id": vid, "source_offset": 100 + i}
                       for i, vid in enumerate((40, 100, 96, 160, 34, 40, 160, 34, 40))]
        observations = [{"machine_index": i, "virtual_id": 40, "target_color": color,
                         "candidate_color": 3, "source_offset": 100 + i, "partition": 0}
                        for i, color in ((0, 3), (5, 4), (8, 3))]
        original = copy.deepcopy((observations, definitions))
        result = pool._hypothetical_wrap_fits(observations, definitions, 1)
        self.assertEqual(result["boundary_count"], 2)  # 100->96 is not a wrap hypothesis.
        self.assertEqual(result["reset_authority"], "UNKNOWN")
        self.assertEqual(result["target_virtual_ids"], "NOT_RECOVERED")
        self.assertEqual(len(result["independent_wrap_fits"]), 2)
        self.assertEqual(result["epoch_fits"][1]["best_conflicts"], 0)
        self.assertEqual(result["epoch_fits"][1]["hypotheses"][0]["source_offset"], 105)
        self.assertEqual((observations, definitions), original)
        with mock.patch.object(pool, "MAX_ALIGNMENT_CELLS", 1):
            limited = pool._hypothetical_wrap_fits(observations, definitions, 1)
        self.assertEqual(limited["cells_evaluated"], 1)
        self.assertEqual(limited["status"], "UNKNOWN")
        self.assertFalse(limited["epoch_fits"][1]["search_complete"])

    def test_hypothetical_wrap_fit_suppresses_small_drops_and_caps_boundaries(self):
        definitions = [{"machine_index": i, "virtual_id": vid}
                       for i, vid in enumerate((100, 96, 101))]
        self.assertEqual(pool._hypothetical_wrap_fits([], definitions, 1)["status"], "suppressed")
        definitions = [{"machine_index": i, "virtual_id": vid}
                       for i, vid in enumerate((160, 34) * 5)]
        result = pool._hypothetical_wrap_fits([], definitions, 1)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["cells_evaluated"], 0)

    def _native_fixture(self):
        envelope, report = _fixture([3, 4], [58, 58], ordinals=[0, 0])
        native = {"tool": "mwcc_win32_varinfo", "schema_version": 1,
                  "target": FUNCTION, "machine_emissions": [], "regalloc_pcode": []}
        for index in range(2):
            machine = dict(envelope["events"][index * 2])
            raw = "00040200" + (58).to_bytes(2, "little").hex() + "000000000000"
            colored = "00040200" + (3).to_bytes(2, "little").hex() + "000000000000"
            machine.update(opcode=137, operand_count=1, source_offset=100 + index,
                           pcode=hex(0x3000 + index * 64))
            capture = {key: machine[key] for key in
                       ("pcode", "pcode_token", "opcode", "operand_count", "source_offset")}
            capture.update(observation_index=index, operand_ordinal=0, operand_index=58,
                           operand_flags=2, operand_raw=raw, color=3, **{"class": 4, "pass": 1})
            machine["operands"] = [{"ordinal": 0, "kind": 0, "class": 4, "color": 3,
                                    "join_status": "observed", "vreg": 58,
                                    "color_observation": index, "allocation_pass": 1,
                                    "pre_color_flags": 2, "raw": colored}]
            native["machine_emissions"].append(machine)
            native["regalloc_pcode"].append(capture)
        return native, report

    def test_native_join_keeps_source_offsets_and_target_role_conflicts(self):
        native, report = self._native_fixture()
        result = self._analyze(native, report)
        self.assertEqual(result["input_format"], "native")
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(result["collisions"]["count"], 1)
        examples = result["collisions"]["items"][0]["exemplars"]
        self.assertEqual([x["source_offset"] for x in examples], [100, 101])
        self.assertEqual([x["operand_ordinal"] for x in examples], [0, 0])

    def test_native_rejects_stale_join_fields_and_raw_operands(self):
        for key in ("opcode", "source_offset", "operand_ordinal", "operand_count",
                    "operand_index", "color", "observation_index", "operand_raw"):
            native, report = self._native_fixture()
            native["regalloc_pcode"][0][key] = "bad" if key == "operand_raw" else -1
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, "native"):
                self._analyze(native, report)

    def test_native_unknown_operand_never_gets_a_virtual_id(self):
        native, report = self._native_fixture()
        for emission in native["machine_emissions"]:
            emission["operands"][0]["join_status"] = "UNKNOWN"
        result = self._analyze(native, report)
        self.assertEqual(result["observations"]["pooled_roles"], 0)
        self.assertEqual(result["status"], "no_confirmed_gpr_evidence")

    def test_native_candidate_word_and_source_offset_drift_rejected(self):
        for key, value in (("ppc_word", 0), ("source_offset", 999)):
            native, report = self._native_fixture()
            report["right"]["symbols"][0]["instructions"][0]["instruction"][key] = value
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, "drift"):
                self._analyze(native, report)

    def test_native_id_drops_do_not_claim_allocator_resets(self):
        result = pool._reset([{"virtual_id": x, "machine_index": i}
                              for i, x in enumerate((58, 59, 35, 60, 58))], native=True)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertFalse(result["detected"])
        self.assertEqual(result["nonmonotonic_transitions"], 2)
        self.assertEqual(result["repeated_definition_ids"], [58])

    def _analyze(
        self,
        envelope: dict[str, object],
        report: dict[str, object],
        *,
        function: str = FUNCTION,
    ) -> dict[str, object]:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            envelope_path = root / "envelope.json"
            report_path = root / "report.json"
            envelope_path.write_text(json.dumps(envelope), encoding="utf-8")
            report_path.write_text(json.dumps(report), encoding="utf-8")
            return pool.analyze(envelope_path, report_path, function)

    def test_clean_monotonic_reset_is_reported(self) -> None:
        envelope, report = _fixture([3] * 6, [32, 33, 34, 35, 32, 33])
        result = self._analyze(envelope, report)
        self.assertEqual(result["reset"]["status"], "detected")
        self.assertEqual(result["reset"]["machine_index"], 4)
        self.assertEqual(result["reset"]["from_virtual_id"], 35)
        self.assertEqual(result["reset"]["to_virtual_id"], 32)
        self.assertEqual(result["collisions"]["count"], 0)

    def test_fpr_only_capture_is_explicitly_unsupported(self) -> None:
        envelope, report = _fixture([3, 4], [32, 33])
        for event in envelope["events"]:
            if event.get("event_kind") == "pcode_capture":
                event["operand_bank"] = "FPR"
        result = self._analyze(envelope, report)
        coverage = result["coverage"]
        self.assertEqual(result["status"], "unsupported_fpr_evidence")
        self.assertEqual(result["decision"], "unsupported_fpr_evidence")
        self.assertEqual(result["suffix_fit"]["status"], "unsupported_fpr_evidence")
        self.assertEqual(coverage["supported_operand_banks"], ["GPR"])
        self.assertEqual(coverage["observed_banks"], ["FPR"])
        self.assertEqual(coverage["discarded_unsupported_bank_counts"], {"FPR": 2})

    def test_empty_operand_capture_is_not_reported_as_pool_absence(self) -> None:
        envelope, report = _fixture([3], [32])
        envelope["events"] = [
            event for event in envelope["events"] if event.get("event_kind") == "machine_emission"
        ]
        result = self._analyze(envelope, report)
        self.assertEqual(result["status"], "no_operand_evidence")
        self.assertEqual(result["decision"], "no_operand_evidence")
        self.assertEqual(result["coverage"]["observed_banks"], [])
        self.assertEqual(result["coverage"]["discarded_unsupported_bank_counts"], {})
        self.assertEqual(result["suffix_fit"]["status"], "no_operand_evidence")

    def test_noncanonical_lowercase_bank_cannot_claim_gpr_analysis(self) -> None:
        envelope, report = _fixture([3], [32])
        for event in envelope["events"]:
            if event.get("event_kind") == "pcode_capture":
                event["operand_bank"] = "gpr"
        result = self._analyze(envelope, report)
        self.assertEqual(result["status"], "no_operand_evidence")
        self.assertEqual(result["decision"], "no_operand_evidence")
        self.assertEqual(result["coverage"]["observed_banks"], [])
        self.assertEqual(result["observations"]["confirmed_gpr_roles"], 0)
        self.assertEqual(result["suffix_fit"]["status"], "no_operand_evidence")

    def test_gpr_aliases_remain_supported_evidence(self) -> None:
        envelope, report = _fixture([3, 4], [32, 32])
        result = self._analyze(envelope, report)
        self.assertEqual(result["status"], "supported_gpr_evidence")
        self.assertEqual(result["decision"], "analyze_gpr")
        self.assertEqual(result["coverage"]["supported_operand_banks"], ["GPR"])
        self.assertEqual(result["coverage"]["observed_banks"], ["GPR"])
        self.assertEqual(result["coverage"]["discarded_unsupported_bank_counts"], {})
        self.assertEqual(result["collisions"]["count"], 1)

    def test_ambiguous_target_roles_remain_unknown(self) -> None:
        envelope, report = _fixture([3, 4], [32, 32])
        result = self._analyze(envelope, report)
        self.assertEqual(result["collisions"]["count"], 1)
        self.assertEqual(result["suffix_fit"]["status"], "UNKNOWN")
        self.assertEqual(result["suffix_fit"]["zero_conflict_candidate_count"], 0)

    def test_drifted_emitted_word_fails_closed(self) -> None:
        envelope, report = _fixture([3], [32])
        envelope = copy.deepcopy(envelope)
        envelope["events"][0]["ppc_word"] = MACHINE_WORD + 1
        with self.assertRaisesRegex(ValueError, "machine word drift"):
            self._analyze(envelope, report)

    def test_exact_pool_suppresses_fit(self) -> None:
        envelope, report = _fixture([3, 4, 3], [32, 33, 34], candidate_roles=[3, 4, 3])
        result = self._analyze(envelope, report)
        self.assertEqual(result["collisions"]["count"], 0)
        self.assertEqual(result["suffix_fit"]["status"], "exact")
        self.assertTrue(result["suffix_fit"]["suppressed"])

    def test_unique_first_partition_suffix_offset(self) -> None:
        envelope, report = _fixture(
            [3, 4, 5, 6, 7, 8, 9, 10, 11, 3, 9, 4, 5, 6, 7, 8, 9, 10],
            list(range(32, 41)) + list(range(32, 41)),
        )
        result = self._analyze(envelope, report)
        self.assertEqual(result["collisions"]["count"], 8)
        self.assertEqual(result["suffix_fit"]["status"], "unique")
        self.assertEqual(result["suffix_fit"]["partition"], 0)
        self.assertEqual(result["suffix_fit"]["start_virtual_id"], 33)
        self.assertEqual(result["suffix_fit"]["delta"], 1)

    def test_noncontiguous_gpr_operand_ordinals_are_accepted(self) -> None:
        envelope, report = _fixture([3], [32], ordinals=[2])
        result = self._analyze(envelope, report)
        self.assertEqual(result["observations"]["pooled_roles"], 1)

    def test_machine_token_expansion_is_not_rejected(self) -> None:
        envelope, report = _fixture([3, 3], [32, 33])
        envelope = copy.deepcopy(envelope)
        envelope["events"][2]["pcode_token"] = envelope["events"][0]["pcode_token"]
        result = self._analyze(envelope, report)
        self.assertGreaterEqual(result["observations"]["confirmed_gpr_roles"], 1)

    def test_unverified_canonical_rows_are_labeled(self) -> None:
        envelope, report = _fixture([3], [32], row_word=None)
        result = self._analyze(envelope, report)
        verification = result["machine_word_verification"]
        self.assertEqual(verification["status"], "unverified")
        self.assertEqual(verification["word_pairs_checked"], 0)
        self.assertEqual(verification["emission_self_pairs_checked"], 1)

    def test_cli_emits_bounded_canonical_json(self) -> None:
        envelope, report = _fixture([3], [32])
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            envelope_path = root / "envelope.json"
            report_path = root / "report.json"
            envelope_path.write_text(json.dumps(envelope), encoding="utf-8")
            report_path.write_text(json.dumps(report), encoding="utf-8")
            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                status = pool.main([
                    "--envelope", str(envelope_path),
                    "--report", str(report_path),
                    "--function", FUNCTION,
                ])
            self.assertEqual(status, 0)
            parsed = json.loads(stdout.getvalue())
            self.assertEqual(parsed["schema"], pool.SCHEMA)
            self.assertLessEqual(len(stdout.getvalue().encode("utf-8")), pool.MAX_OUTPUT_BYTES)

    def test_reciprocal_symbol_pairing_is_checked(self) -> None:
        envelope, report = _fixture([3], [32])
        report["right"]["symbols"][0]["target_symbol"] = 7
        with self.assertRaisesRegex(ValueError, "reciprocally pair"):
            self._analyze(envelope, report)

    def test_missing_function_fails_closed(self) -> None:
        envelope, report = _fixture([3], [32])
        with self.assertRaisesRegex(ValueError, "exactly one target function"):
            self._analyze(envelope, report, function="Missing")

    def test_malformed_envelope_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            envelope_path = root / "envelope.json"
            report_path = root / "report.json"
            envelope_path.write_text("{not-json", encoding="utf-8")
            report_path.write_text(json.dumps(_fixture([3], [32])[1]), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "malformed envelope"):
                pool.analyze(envelope_path, report_path, FUNCTION)


if __name__ == "__main__":
    unittest.main()
