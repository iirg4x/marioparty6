from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import recovery_frontier as frontier
from tools.tests.test_focus_symbol_report import _report


def _access_row(address: int, text: str, diff_kind: str | None = None) -> dict[str, object]:
    row: dict[str, object] = {
        "instruction": {"address": str(address), "formatted": text, "size": 4}
    }
    if diff_kind is not None:
        row["diff_kind"] = diff_kind
    return row


def _branch_row(address: int, text: str, destination: object | None = None) -> dict[str, object]:
    row = _access_row(address, text)
    if destination is not None:
        row["instruction"]["branch_dest"] = destination
    return row


def _abi_cycle_rows(*, unpaired=False, conflicting_use=False,
                    changed_operation=False, body_data=False):
    target_body = [
        _access_row(24, "lfs f14, 0x0(r4)"),
        _access_row(28, "lfs f15, 0x4(r4)"),
        _access_row(32, "fadd f14, f15, f0"),
        _access_row(36, "fadd f15, f14, f1"),
    ]
    candidate_body = [
        _access_row(24, "lfs f15, 0x0(r4)"),
        _access_row(28, "lfs f14, 0x4(r4)"),
        _access_row(32, "fadd f15, f14, f0"),
        _access_row(36, "fadd f14, f15, f1"),
    ]
    if conflicting_use:
        target_body[:2] = [
            _access_row(24, "fadd f14, f15, f0"),
            _access_row(28, "fadd f15, f14, f1"),
        ]
        candidate_body[:2] = [
            _access_row(24, "fadd f15, f14, f0"),
            _access_row(28, "fadd f14, f15, f1"),
        ]
    if changed_operation:
        candidate_body[2] = _access_row(32, "fsub f15, f14, f0")
    if body_data:
        target_body.append(_access_row(40, "stfd f14, 0x10(r1)"))
        candidate_body.append(_access_row(40, "stfd f15, 0x10(r1)"))
    target = [
        _access_row(0, "stwu r1, -0x40(r1)"),
        _access_row(4, "mflr r0"),
        _access_row(8, "stw r0, 0x44(r1)"),
        _access_row(12, "stfd f15, 0x20(r1)"),
        _access_row(16, "psq_st f15, 0x28(r1), 0, qr0"),
        _access_row(20, "stfd f14, 0x10(r1)"),
        _access_row(24, "psq_st f14, 0x18(r1), 0, qr0"),
        *target_body,
        _access_row(48, "psq_l f15, 0x28(r1), 0, qr0"),
        _access_row(52, "lfd f15, 0x20(r1)"),
        _access_row(56, "psq_l f14, 0x18(r1), 0, qr0"),
        _access_row(60, "lfd f14, 0x10(r1)"),
        _access_row(64, "lwz r0, 0x44(r1)"),
        _access_row(68, "mtlr r0"),
        _access_row(72, "addi r1, r1, 0x40"),
        _access_row(76, "blr"),
    ]
    candidate = [
        _access_row(0, "stwu r1, -0x40(r1)"),
        _access_row(4, "mflr r0"),
        _access_row(8, "stw r0, 0x44(r1)"),
        _access_row(12, "stfd f15, 0x20(r1)"),
        _access_row(16, "psq_st f15, 0x28(r1), 0, qr0"),
        _access_row(20, "stfd f14, 0x10(r1)"),
        _access_row(24, "psq_st f14, 0x18(r1), 0, qr0"),
        *candidate_body,
        _access_row(48, "psq_l f15, 0x28(r1), 0, qr0"),
        _access_row(52, "lfd f15, 0x20(r1)"),
        _access_row(56, "psq_l f14, 0x18(r1), 0, qr0"),
        _access_row(60, "lfd f14, 0x10(r1)"),
        _access_row(64, "lwz r0, 0x44(r1)"),
        _access_row(68, "mtlr r0"),
        _access_row(72, "addi r1, r1, 0x40"),
        _access_row(76, "blr"),
    ]
    if unpaired:
        for rows in (target, candidate):
            rows[4] = _access_row(16, "stw r3, 0x28(r1)")
            rows[6] = _access_row(24, "stw r3, 0x18(r1)")
    return target, candidate


class RecoveryFrontierTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "build").mkdir()
        for name, data in (("source.c", b"int f(void) {return 1;}\n"), ("target.o", b"target"), ("candidate.o", b"candidate")):
            (self.root / name).write_bytes(data)
        self.report = _report(focus_exact=False, sibling_exact=True)
        (self.root / "strict.json").write_text(json.dumps(self.report), encoding="utf-8")

    def snapshot(self, **options):
        args = dict(root=self.root, owner="main:board/snpc", source=Path("source.c"),
                    target=Path("target.o"), candidate=Path("candidate.o"),
                    strict=Path("strict.json"), data=None, toolchain_key="GC/2.6/test")
        args.update(options)
        return frontier.snapshot(**args)

    def access_report(self, target_rows, candidate_rows=None):
        report = copy.deepcopy(self.report)
        target = report["left"]["symbols"][1]
        candidate = report["right"]["symbols"][1]
        target["instructions"] = target_rows
        candidate["instructions"] = copy.deepcopy(
            target_rows if candidate_rows is None else candidate_rows
        )
        (self.root / "strict.json").write_text(json.dumps(report), encoding="utf-8")

    def access(self, **options):
        args = dict(
            root=self.root,
            strict=Path("strict.json"),
            function="FocusFunction",
            side="target",
            base_register="r1",
            offset="0x88",
            context=0,
        )
        args.update(options)
        return frontier.accesses(**args)

    def test_first_mismatch_and_counts_are_retained(self):
        value = self.snapshot()
        self.assertEqual(value["summary"], {"functions": 2, "strict_instruction_exact": 1, "data_instruction_exact": None})
        row = value["functions"][0]["first_mismatch"]
        self.assertEqual(row["row"], 0)
        self.assertEqual(row["target"]["formatted"], "lfs f1, pool@sda21")
        self.assertEqual(row["candidate"]["formatted"], "lfs f2, @1@sda21")
        self.assertIsNone(value["physical_exact"])
        self.assertFalse(value["authority_advanced"])
        self.assertEqual(value["compile_binding"], "not_supplied")

    def test_report_is_parsed_once_when_both_channels_share_it(self):
        with mock.patch.object(frontier, "load_json", wraps=frontier.load_json) as parse:
            value = self.snapshot(data=Path("strict.json"))
        self.assertEqual(parse.call_count, 1)
        self.assertEqual(value["summary"]["data_instruction_exact"], 1)

    def test_resume_rejects_source_drift(self):
        value = self.snapshot()
        (self.root / "source.c").write_text("changed", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "stale current evidence: source"):
            frontier.verify(self.root, value)

    def test_resume_rejects_object_and_report_drift(self):
        for filename in ("candidate.o", "target.o", "strict.json"):
            with self.subTest(filename=filename):
                value = self.snapshot()
                path = self.root / filename
                original = path.read_bytes()
                path.write_bytes(original + b" ")
                with self.assertRaisesRegex(ValueError, "stale current evidence"):
                    frontier.verify(self.root, value)
                path.write_bytes(original)

    def test_index_digest_is_checked(self):
        value = self.snapshot()
        value["summary"]["strict_instruction_exact"] = 2
        with self.assertRaisesRegex(ValueError, "digest differs"):
            frontier.verify(self.root, value)

    def test_bad_compiler_receipt_rejected(self):
        path = self.root / "receipt.json"
        path.write_text(json.dumps({"schema": "recovery_candidate_compile/v1", "source_sha256": "wrong"}), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "receipt does not bind"):
            self.snapshot(compile_receipt=Path("receipt.json"))

    def test_bound_compiler_receipt_has_narrow_claim(self):
        receipt = {"schema": "recovery_candidate_compile/v1",
                   "source_sha256": hashlib.sha256((self.root / "source.c").read_bytes()).hexdigest(),
                   "object_sha256": hashlib.sha256((self.root / "candidate.o").read_bytes()).hexdigest()}
        (self.root / "receipt.json").write_text(json.dumps(receipt), encoding="utf-8")
        value = self.snapshot(compile_receipt=Path("receipt.json"))
        self.assertEqual(value["compile_binding"], "receipt_hashes_match")
        self.assertEqual(value["report_binding"], "caller_selected_diagnostic")
        self.assertIsNone(value["linked_exact"])

    def test_duplicate_function_and_pair_mismatch_rejected(self):
        report = copy.deepcopy(self.report)
        report["left"]["symbols"].append(report["left"]["symbols"][1])
        with self.assertRaisesRegex(ValueError, "ambiguous"):
            frontier.summarize(report, "strict")

    def test_empty_diff_kind_and_inconsistent_size(self):
        report = _report(focus_exact=True, sibling_exact=True)
        report['left']['symbols'][1]['instructions'][0]['diff_kind'] = ''
        self.assertTrue(frontier.summarize(report, 'strict')[0]['instruction_exact'])
        report['right']['symbols'][1]['size'] = '12'
        self.assertFalse(frontier.summarize(report, 'strict')[0]['instruction_exact'])

    def test_shared_pool_key_diagnostic_is_not_a_causal_proof(self):
        report = copy.deepcopy(self.report)
        target = copy.deepcopy(report['left']['symbols'][1])
        candidate = copy.deepcopy(report['right']['symbols'][1])
        target['name'] = candidate['name'] = 'SecondPoolConsumer'
        target['target_symbol'] = len(report['right']['symbols'])
        report['left']['symbols'].append(target)
        report['right']['symbols'].append(candidate)
        (self.root/'strict.json').write_text(json.dumps(report), encoding='utf-8')
        value = self.snapshot()
        group = value['shared_relocation_diagnostics'][0]
        self.assertEqual(group['target']['symbol'], 'pool')
        self.assertEqual(group['candidate']['symbol'], '@1')
        self.assertEqual(group['functions'], ['FocusFunction', 'SecondPoolConsumer'])
        self.assertFalse(group['cause_proven'])
        report = copy.deepcopy(self.report)
        report["right"]["symbols"][1]["name"] = "Wrong"
        with self.assertRaisesRegex(ValueError, "paired name differs"):
            frontier.summarize(report, "strict")

    def test_path_escape_and_output_source_rejected(self):
        with self.assertRaises(ValueError):
            self.snapshot(source=Path("../outside.c"))
        value = self.snapshot()
        with self.assertRaises(ValueError):
            frontier.publish(self.root, Path("source.c"), value)

    def test_publish_failure_preserves_previous_small_index(self):
        value = self.snapshot()
        path = self.root / "build/current.json"
        path.write_bytes(b"old")
        with mock.patch.object(frontier.os, "replace", side_effect=OSError("sentinel")):
            with self.assertRaisesRegex(OSError, "sentinel"):
                frontier.publish(self.root, path, value)
        self.assertEqual(path.read_bytes(), b"old")
        self.assertEqual(list(path.parent.glob("*.tmp")), [])

    def test_no_recursive_history_scan_and_small_output(self):
        with mock.patch.object(Path, "rglob", side_effect=AssertionError("history scan")):
            value = self.snapshot()
            frontier.publish(self.root, Path("build/current.json"), value)
        self.assertLess((self.root / "build/current.json").stat().st_size, 8192)
        self.assertEqual(frontier.main(["--root", str(self.root), "verify", "build/current.json"]), 0)

    def test_input_limit_and_duplicate_json_rejected(self):
        with self.assertRaisesRegex(ValueError, "exceeds 2 bytes"):
            frontier.read_bound(self.root, Path("strict.json"), 2)
        with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
            frontier.load_json(b'{"x":1,"x":2}')

    def test_none_relocations_not_grouped(self):
        for side in ('left', 'right'):
            self.report[side]['symbols'][1]['instructions'][0]['instruction']['relocation']['type_name'] = 'R_PPC_NONE'
        self.assertEqual(frontier.summarize(self.report, 'strict')[0]['relocation_keys'], [])

    def test_extra_candidate_function_is_explicit(self):
        extra = copy.deepcopy(self.report['right']['symbols'][2])
        extra['name'] = 'CandidateOnly'
        self.report['right']['symbols'].append(extra)
        (self.root/'strict.json').write_text(json.dumps(self.report), encoding='utf-8')
        value = self.snapshot()
        self.assertEqual(value['candidate_only_functions'][0]['function'], 'CandidateOnly')
        self.assertEqual(value['function_census_binding'], 'report_only_not_independently_verified')

    def test_cross_channel_size_drift_rejected(self):
        report = copy.deepcopy(self.report)
        report['right']['symbols'][1]['size'] = '12'
        (self.root/'data.json').write_text(json.dumps(report), encoding='utf-8')
        with self.assertRaisesRegex(ValueError, 'object layout differs'):
            self.snapshot(data=Path('data.json'))

    def test_nonexact_without_rows_is_explicitly_unlocated(self):
        report = _report(focus_exact=True, sibling_exact=True)
        report['left']['symbols'][1]['match_percent'] = 99.0
        row = frontier.summarize(report, 'strict')[0]
        self.assertFalse(row['instruction_exact'])
        self.assertIsNotNone(row['unlocated_mismatch'])

    def test_malformed_and_empty_reports_rejected_cleanly(self):
        for report in ([], {'left': {'symbols': []}, 'right': {'symbols': []}}):
            with self.assertRaises(ValueError):
                frontier.summarize(report, 'strict')
        report = copy.deepcopy(self.report)
        report['right']['symbols'][1]['instructions'] = [42]
        with self.assertRaises(ValueError):
            frontier.summarize(report, 'strict')

    def test_accesses_match_exact_displacement_and_decimal_hex_equivalence(self):
        self.access_report(
            [
                _access_row(0, "lwz r3, 0x88(r1)", "DIFF_ARG_MISMATCH"),
                _access_row(4, "lwz r3, 0x188(r1)"),
                _access_row(8, "lwz r3, 136(r1)"),
                _access_row(12, "lwz r3, -0x88(r1)"),
                _access_row(16, "lwz r3, 0x88(r10)"),
                _access_row(20, "addi r3, r1, 0x88"),
                _access_row(24, "lwz r3, 0(r1)"),
            ]
        )
        hexadecimal = self.access(offset="0x88")
        decimal = self.access(offset="136")
        self.assertEqual([row["row_index"] for row in hexadecimal["matches"]], [0, 2])
        self.assertEqual(
            [row["row_index"] for row in decimal["matches"]],
            [0, 2],
        )
        self.assertEqual(hexadecimal["match_count"], 2)
        self.assertEqual(hexadecimal["matches"][0]["instruction_address"], "0")
        self.assertEqual(hexadecimal["matches"][0]["text"], "lwz r3, 0x88(r1)")
        self.assertEqual(hexadecimal["matches"][0]["diff_kind"], "DIFF_ARG_MISMATCH")

    def test_accesses_separate_base_registers_and_sides(self):
        target_rows = [
            _access_row(0, "lwz r3, 0x88(r1)"),
            _access_row(4, "lwz r3, 0x88(r10)"),
        ]
        candidate_rows = [
            _access_row(0, "lwz r3, 0x88(r10)"),
            _access_row(4, "lwz r3, 0x88(r1)"),
        ]
        self.access_report(target_rows, candidate_rows)
        target = self.access(side="target")
        candidate = self.access(side="candidate")
        self.assertEqual([row["row_index"] for row in target["matches"]], [0])
        self.assertEqual([row["row_index"] for row in candidate["matches"]], [1])
        self.assertEqual(candidate["side"], "candidate")
        self.assertEqual(candidate["report_sha256"], candidate["report"]["sha256"])

    def test_accesses_context_is_bounded_and_deduplicated(self):
        rows = [_access_row(index, f"addi r3, r3, {index}") for index in range(8)]
        rows[2] = _access_row(8, "lwz r3, 0x88(r1)")
        rows[4] = _access_row(16, "lwz r3, 0x88(r1)")
        self.access_report(rows)
        result = self.access(context=2)
        context_indices = [row["row_index"] for row in result["context_rows"]]
        self.assertEqual([row["row_index"] for row in result["matches"]], [2, 4])
        self.assertEqual(context_indices, [0, 1, 3, 5, 6])
        self.assertEqual(len(context_indices), len(set(context_indices)))
        self.assertFalse(result["truncated"])

        many = [_access_row(index, "lwz r3, 0x88(r1)") for index in range(70)]
        self.access_report(many)
        result = self.access(context=3, max_matches=2)
        self.assertEqual(result["match_count"], 70)
        self.assertEqual(len(result["matches"]), 2)
        self.assertTrue(result["truncated"])
        self.assertLess(len(json.dumps(result).encode("utf-8")), 256 * 1024)

    def test_accesses_reject_invalid_queries_and_malformed_or_missing_reports(self):
        self.access_report([_access_row(0, "lwz r3, 0x88(r1)")])
        for options in (
            {"function": "MissingFunction"},
            {"side": "other"},
            {"base_register": "r32"},
            {"offset": "0xGG"},
            {"context": 4},
        ):
            with self.subTest(options=options):
                with self.assertRaises(ValueError):
                    self.access(**options)
        with self.assertRaises((OSError, ValueError)):
            self.access(strict=Path("missing.json"))

        (self.root / "strict.json").write_text("[]", encoding="utf-8")
        with self.assertRaises(ValueError):
            self.access()
        malformed = copy.deepcopy(self.report)
        malformed["left"]["symbols"][1]["instructions"] = [42]
        (self.root / "strict.json").write_text(json.dumps(malformed), encoding="utf-8")
        with self.assertRaises(ValueError):
            self.access()


    def stack(self, target_rows, candidate_rows=None, function="FocusFunction"):
        self.access_report(target_rows, candidate_rows)
        return frontier.stack_map(root=self.root, strict=Path("strict.json"), function=function)

    def branch(self, target_rows, candidate_rows=None, function="FocusFunction"):
        self.access_report(target_rows, candidate_rows)
        return frontier.branch_map(root=self.root, strict=Path("strict.json"), function=function)

    def test_stack_map_pairs_masked_d_form_and_addi_offsets(self):
        target = [
            _access_row(0, "stw r0, 0x68(r1)"),
            _access_row(4, "stw r0, 0x6c(r1)"),
            _access_row(8, "lwz r3, -0x4(r1)"),
            _access_row(12, "addi r5, r1, 0x68"),
            _access_row(16, "addi r5, r1, 136"),
        ]
        candidate = [
            _access_row(0, "stw r0, 0x6c(r1)"),
            _access_row(4, "stw r0, 0x68(r1)"),
            _access_row(8, "lwz r3, -4(r1)"),
            _access_row(12, "addi r5, r1, 104"),
            _access_row(16, "addi r5, r1, 0x68"),
        ]
        result = self.stack(target, candidate)
        d_pairs = {(row["target_offset"], row["candidate_offset"]): row
                   for row in result["d_form"]["pairs"]}
        self.assertEqual(d_pairs[(0x68, 0x6c)]["status"], "changed")
        self.assertEqual(d_pairs[(0x6c, 0x68)]["status"], "changed")
        self.assertEqual(d_pairs[(-4, -4)]["status"], "equal")
        pointer = result["addi_pointer"]["pairs"]
        pointer_pairs = {(row["target_offset"], row["candidate_offset"]): row for row in pointer}
        self.assertEqual(pointer_pairs[(0x68, 0x68)]["status"], "ambiguous_one_to_many")
        self.assertEqual(pointer_pairs[(136, 104)]["status"], "ambiguous_one_to_many")
        self.assertEqual(pointer_pairs[(0x68, 0x68)]["count"], 1)
        self.assertLessEqual(len(pointer_pairs[(0x68, 0x68)]["exemplars"]), 3)
        self.assertEqual(result["report_sha256"], result["report"]["sha256"])
        self.assertFalse(result["authority_advanced"])

    def test_stack_map_requires_same_non_displacement_operands_and_marks_ambiguity(self):
        target = [_access_row(0, "lwz r3, 0x68(r1)"),
                  _access_row(4, "lwz r3, 0x68(r1)")]
        candidate = [_access_row(0, "lwz r3, 0x6c(r1)"),
                     _access_row(4, "lwz r3, 0x70(r1)")]
        result = self.stack(target, candidate)
        self.assertEqual(result["d_form"]["paired_rows"], 2)
        self.assertEqual(result["d_form"]["pair_count"], 2)
        self.assertTrue(all(row["status"] == "ambiguous_one_to_many"
                            for row in result["d_form"]["pairs"]))
        result = self.stack(
            [_access_row(0, "lwz r3, 0x68(r1)")],
            [_access_row(0, "lwz r4, 0x6c(r1)")],
        )
        self.assertEqual(result["d_form"]["paired_rows"], 0)
        self.assertEqual(result["d_form"]["target_access_count"], 1)

    def test_stack_map_placeholders_missing_symbol_and_malformed_json(self):
        result = self.stack([{"instruction": None}, _access_row(4, "lwz r3, 0(r1)")])
        self.assertEqual(result["d_form"]["target_access_count"], 1)
        self.assertEqual(result["d_form"]["paired_rows"], 1)
        report = copy.deepcopy(self.report)
        report["right"]["symbols"][1]["name"] = "OtherFunction"
        (self.root / "strict.json").write_text(json.dumps(report), encoding="utf-8")
        result = frontier.stack_map(root=self.root, strict=Path("strict.json"), function="FocusFunction")
        self.assertEqual(result["status"], "missing_symbol")
        self.assertEqual(result["missing_sides"], ["candidate"])
        self.assertEqual(result["d_form"]["candidate_access_count"], 0)
        (self.root / "strict.json").write_text("[", encoding="utf-8")
        with self.assertRaises(ValueError):
            frontier.stack_map(root=self.root, strict=Path("strict.json"), function="FocusFunction")

    def test_stack_map_output_is_bounded_and_cli_is_wired(self):
        target = [_access_row(index, f"lwz r3, 0x{0x100 + index * 4:x}(r1)") for index in range(2000)]
        candidate = [_access_row(index, f"lwz r3, 0x{0x200 + index * 4:x}(r1)") for index in range(2000)]
        result = self.stack(target, candidate)
        self.assertTrue(result["truncated"])
        self.assertLess(len(frontier.canonical(result)), 256 * 1024)
        self.access_report([_access_row(0, "lwz r3, 0x68(r1)")])
        with unittest.mock.patch("builtins.print") as output:
            self.assertEqual(frontier.main(["--root", str(self.root), "stack-map",
                                            "--strict", "strict.json", "--function", "FocusFunction"]), 0)
        self.assertTrue(output.called)


    def test_branch_map_uses_destination_row_identity_and_excludes_calls(self):
        target = [
            _branch_row(100, "b 108", "108"),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "blr"),
        ]
        candidate = [
            _branch_row(200, "b 208", "0xd0"),
            _access_row(204, "li r3, 0x0"),
            _access_row(208, "blr"),
        ]
        result = self.branch(target, candidate)
        summary = result["branches"]
        self.assertEqual(summary["paired_branch_count"], 1)
        self.assertEqual(summary["same_destination_count"], 1)
        self.assertEqual(summary["changed_destination_count"], 0)
        self.assertEqual(summary["findings"], [])
        calls = self.branch([_access_row(100, "bl helper"), _access_row(104, "blr")])
        self.assertEqual(calls["branches"]["target_branch_count"], 0)
        self.assertEqual(calls["branches"]["candidate_branch_count"], 0)

        result = self.branch(
            [_branch_row(100, "b 108", 108), _access_row(104, "li r3, 0x0"), _access_row(108, "blr")],
            [_branch_row(200, "b 204", 204), _access_row(204, "li r3, 0x0"), _access_row(208, "blr")],
        )
        finding = result["branches"]["findings"][0]
        self.assertEqual(finding["status"], "changed")
        self.assertEqual((finding["target_destination_row"], finding["candidate_destination_row"]), (2, 1))

    def test_branch_map_accepts_unique_destination_window_after_insertion(self):
        target = [
            _branch_row(100, "b 108", 108),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r3, r29"),
            _access_row(112, "bl mbComChoiceListDownSet"),
            _access_row(116, "blr"),
        ]
        candidate = [
            _branch_row(100, "b 112", 112),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r4, r3"),
            _access_row(112, "mr r3, r29"),
            _access_row(116, "bl mbComChoiceListDownSet"),
            _access_row(120, "blr"),
        ]
        result = self.branch(target, candidate)["branches"]
        self.assertEqual(result["same_destination_count"], 1)
        self.assertEqual(result["aligned_destination_count"], 1)
        self.assertEqual(result["changed_destination_count"], 0)
        self.assertEqual(result["findings"], [])

    def test_branch_map_rejects_same_destination_opcode_with_changed_followup(self):
        target = [
            _branch_row(100, "b 108", 108),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r3, r29"),
            _access_row(112, "bl mbComChoiceListDownSet"),
            _access_row(116, "blr"),
        ]
        candidate = [
            _branch_row(100, "b 112", 112),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r4, r3"),
            _access_row(112, "mr r3, r29"),
            _access_row(116, "bl mbComChoiceListUpSet"),
            _access_row(120, "blr"),
        ]
        result = self.branch(target, candidate)["branches"]
        self.assertEqual(result["same_destination_count"], 0)
        self.assertEqual(result["aligned_destination_count"], 0)
        self.assertEqual(result["changed_destination_count"], 1)
        self.assertEqual(result["findings"][0]["status"], "changed")

    def test_branch_map_rejects_ambiguous_repeated_destination_window(self):
        target = [
            _branch_row(100, "b 108", 108),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r3, r29"),
            _access_row(112, "bl mbComChoiceListDownSet"),
            _access_row(116, "blr"),
        ]
        candidate = [
            _branch_row(100, "b 112", 112),
            _access_row(104, "li r3, 0x0"),
            _access_row(108, "mr r4, r3"),
            _access_row(112, "mr r3, r29"),
            _access_row(116, "bl mbComChoiceListDownSet"),
            _access_row(120, "blr"),
            _access_row(124, "mr r3, r29"),
            _access_row(128, "bl mbComChoiceListDownSet"),
        ]
        result = self.branch(target, candidate)["branches"]
        self.assertEqual(result["same_destination_count"], 0)
        self.assertEqual(result["aligned_destination_count"], 0)
        self.assertEqual(result["changed_destination_count"], 1)
        self.assertEqual(result["findings"][0]["status"], "changed")

    def test_branch_map_unresolved_missing_destination_and_malformed_report(self):
        result = self.branch(
            [_branch_row(100, "b 0x999", 0x999), _access_row(104, "blr")],
            [_branch_row(200, "b 204", 204), _access_row(204, "blr")],
        )
        finding = result["branches"]["findings"][0]
        self.assertEqual(finding["status"], "unresolved")
        self.assertIsNone(finding["target_destination_row"])
        self.assertEqual(finding["candidate_destination_row"], 1)
        report = copy.deepcopy(self.report)
        report["right"]["symbols"][1]["name"] = "OtherFunction"
        (self.root / "strict.json").write_text(json.dumps(report), encoding="utf-8")
        result = frontier.branch_map(root=self.root, strict=Path("strict.json"), function="FocusFunction")
        self.assertEqual(result["status"], "missing_symbol")
        self.assertEqual(result["missing_sides"], ["candidate"])
        (self.root / "strict.json").write_text("[", encoding="utf-8")
        with self.assertRaises(ValueError):
            frontier.branch_map(root=self.root, strict=Path("strict.json"), function="FocusFunction")

    def test_branch_map_output_is_bounded_and_cli_is_wired(self):
        target = [_branch_row(index * 4, f"b 0x{0x100000 + index * 4:x}") for index in range(2000)]
        candidate = [_branch_row(index * 4, f"b 0x{0x200000 + index * 4:x}") for index in range(2000)]
        result = self.branch(target, candidate)
        self.assertTrue(result["truncated"])
        self.assertLess(len(frontier.canonical(result)), 256 * 1024)
        self.branch([_branch_row(100, "b 104"), _access_row(104, "blr")])
        with unittest.mock.patch("builtins.print") as output:
            self.assertEqual(frontier.main(["--root", str(self.root), "branch-map",
                                            "--strict", "strict.json", "--function", "FocusFunction"]), 0)
        self.assertTrue(output.called)

    def diagnose(self, target_rows, candidate_rows=None, *, data_report=None, varinfo=None):
        self.access_report(target_rows, candidate_rows)
        data_path = None
        if data_report is not None:
            (self.root / "data.json").write_text(json.dumps(data_report), encoding="utf-8")
            data_path = Path("data.json")
        return frontier.diagnose(
            root=self.root,
            strict=Path("strict.json"),
            data=data_path,
            function="FocusFunction",
            varinfo=varinfo,
        )

    def test_diagnose_first_mismatch_gates_and_context_are_bounded(self):
        result = self.diagnose(
            [_access_row(0, "li r3, 1"), _access_row(4, "mr r3, r4"), _access_row(8, "blr")],
            [_access_row(0, "li r3, 1"), _access_row(4, "mr r4, r3"), _access_row(8, "blr")],
        )
        self.assertEqual(result["schema"], frontier.DIAGNOSE_SCHEMA)
        mismatch = result["strict"]["first_mismatch"]
        self.assertEqual(mismatch["row"], 1)
        self.assertEqual(mismatch["kind"], "instruction")
        self.assertEqual([row["relative"] for row in mismatch["context"]], [-1, 0, 1])
        self.assertEqual(result["strict"]["diffs"]["diff_row_count"], 1)
        self.assertFalse(result["strict"]["gates"]["exact"])
        self.assertEqual(result["strict"]["branch_destinations"]["status"], "none")
        self.assertFalse(result["physical_proof"])
        self.assertFalse(result["authority_advanced"])

    def test_diagnose_closed_register_two_cycle_is_confirmed(self):
        target = [
            _access_row(0, "mr r3, r4"),
            _access_row(4, "addi r3, r3, 1"),
            _access_row(8, "mr r4, r3"),
            _access_row(12, "addi r4, r4, 2"),
        ]
        candidate = [
            _access_row(0, "mr r4, r3"),
            _access_row(4, "addi r4, r4, 0x1"),
            _access_row(8, "mr r3, r4"),
            _access_row(12, "addi r3, r3, 2"),
        ]
        permutation = self.diagnose(target, candidate)["strict"]["register_permutation"]
        self.assertEqual(permutation["status"], "confirmed")
        self.assertEqual(permutation["changed_mapping"], {"r3": "r4", "r4": "r3"})
        self.assertTrue(permutation["closed"])
        self.assertTrue(permutation["operations_immediates_agree"])

    def test_diagnose_body_projection_excludes_authenticated_abi_cycle_pairs(self):
        target, candidate = _abi_cycle_rows()
        permutation = self.diagnose(target, candidate)["strict"]["register_permutation"]
        projection = permutation["body_projection"]
        self.assertEqual(permutation["status"], "ambiguous")
        self.assertEqual(projection["status"], "confirmed")
        self.assertEqual(projection["reason"], "closed_body_register_cycle")
        self.assertEqual(projection["projected_cycle"], ["f14", "f15", "f14"])
        self.assertEqual(projection["excluded_pair_count"], 2)
        self.assertEqual(projection["excluded_paired_rows"]["target"],
                         [3, 4, 5, 6, 11, 12, 13, 14])
        self.assertEqual(projection["excluded_paired_rows"]["candidate"],
                         [3, 4, 5, 6, 11, 12, 13, 14])
        self.assertFalse(projection["authority_advanced"])

    def test_diagnose_body_projection_keeps_branch_target_in_excluded_abi_rows(self):
        target, candidate = _abi_cycle_rows()
        # The branch targets the first restore instruction.  That row is
        # intentionally excluded from the projected body, but the branch is
        # still identical in both full instruction streams.
        target.insert(11, _branch_row(44, "b 48", 48))
        candidate.insert(11, _branch_row(44, "b 48", 48))
        strict = self.diagnose(target, candidate)["strict"]
        self.assertEqual(strict["branch_destinations"]["status"], "exact")
        projection = strict["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "confirmed")
        self.assertEqual(projection["reason"], "closed_body_register_cycle")

    def test_diagnose_body_projection_rejects_low_fpr_pseudo_pairs(self):
        target, candidate = _abi_cycle_rows()
        for rows in (target, candidate):
            for row in rows:
                instruction = row.get("instruction")
                if isinstance(instruction, dict):
                    instruction["formatted"] = instruction["formatted"].replace("f14", "f2").replace("f15", "f3")
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "none")
        self.assertEqual(projection["reason"], "no_authenticated_abi_pairs")
        self.assertEqual(projection["excluded_pair_count"], 0)

    def test_diagnose_body_projection_rejects_unpaired_stack_home_pattern(self):
        target, candidate = _abi_cycle_rows(unpaired=True)
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "none")
        self.assertEqual(projection["reason"], "no_authenticated_abi_pairs")
        self.assertEqual(projection["projected_cycle"], [])

    def test_diagnose_body_projection_rejects_incoming_and_changed_body_use(self):
        target, candidate = _abi_cycle_rows(conflicting_use=True)
        incoming = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(incoming["status"], "rejected")
        self.assertEqual(incoming["reason"], "incoming_register_value_used")
        self.assertEqual(incoming["excluded_paired_rows"]["target"], [])

        target, candidate = _abi_cycle_rows(changed_operation=True)
        changed = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(changed["status"], "rejected")
        self.assertEqual(changed["reason"], "opcode_or_immediate_changed")
        self.assertEqual(changed["projected_cycle"], [])

    def test_diagnose_body_projection_rejects_real_body_use_of_abi_slot(self):
        target, candidate = _abi_cycle_rows(body_data=True)
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "rejected")
        self.assertEqual(projection["reason"], "abi_save_slot_used_by_body")
        self.assertEqual(projection["conflicts"][0]["row"], 11)

    def test_diagnose_body_projection_rejects_overlapping_stack_access(self):
        target, candidate = _abi_cycle_rows(body_data=True)
        target[11] = _access_row(40, "stfs f14, 0x14(r1)")
        candidate[11] = _access_row(40, "stfs f15, 0x14(r1)")
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "rejected")
        self.assertEqual(projection["reason"], "abi_save_slot_used_by_body")
        self.assertEqual(projection["conflicts"][0]["row"], 11)

    def test_diagnose_body_projection_rejects_stack_pointer_into_backup(self):
        target, candidate = _abi_cycle_rows(body_data=True)
        target[11] = _access_row(40, "addi r3, r1, 0x14")
        candidate[11] = _access_row(40, "addi r3, r1, 0x14")
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "rejected")
        self.assertEqual(projection["reason"], "abi_save_slot_used_by_body")
        self.assertEqual(projection["conflicts"][0]["row"], 11)

    def test_diagnose_body_projection_rejects_indexed_stack_access(self):
        target, candidate = _abi_cycle_rows(body_data=True)
        target[11] = _access_row(40, "stfdx f14, r3, r1")
        candidate[11] = _access_row(40, "stfdx f15, r3, r1")
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "rejected")
        self.assertEqual(projection["reason"], "unsupported_stack_access_form")
        self.assertEqual(projection["conflicts"][0]["row"], 11)

    def test_diagnose_body_projection_rejects_indirect_count_branch(self):
        for opcode in ("bctr", "bcctr"):
            with self.subTest(opcode=opcode):
                target, candidate = _abi_cycle_rows(body_data=True)
                target[11] = _access_row(40, f"{opcode} ")
                candidate[11] = _access_row(40, f"{opcode} ")
                projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
                self.assertEqual(projection["status"], "rejected")
                self.assertEqual(projection["reason"], "body_control_flow_unresolved")
                self.assertEqual(projection["conflicts"][0]["row"], 11)

    def test_diagnose_body_projection_rejects_conditional_incoming_register(self):
        target, candidate = _abi_cycle_rows()
        target[7] = _branch_row(24, "bne 32", 32)
        candidate[7] = _branch_row(24, "bne 32", 32)
        projection = self.diagnose(target, candidate)["strict"]["register_permutation"]["body_projection"]
        self.assertEqual(projection["status"], "rejected")
        self.assertEqual(projection["reason"], "incoming_register_value_used")
        self.assertEqual(projection["conflicts"][0]["row"], 9)

    def test_diagnose_opcode_or_immediate_change_is_rejected(self):
        permutation = self.diagnose(
            [_access_row(0, "addi r3, r3, 1")],
            [_access_row(0, "addi r4, r4, 2")],
        )["strict"]["register_permutation"]
        self.assertEqual(permutation["status"], "rejected")
        self.assertEqual(permutation["reason"], "opcode_or_immediate_changed")
        self.assertFalse(permutation["operations_immediates_agree"])

    def test_diagnose_register_relation_ambiguity_is_not_permutation(self):
        permutation = self.diagnose(
            [_access_row(0, "mr r3, r4"), _access_row(4, "mr r3, r5")],
            [_access_row(0, "mr r6, r7"), _access_row(4, "mr r8, r7")],
        )["strict"]["register_permutation"]
        self.assertEqual(permutation["status"], "ambiguous")
        self.assertEqual(permutation["reason"], "conflicting_register_relations")
        self.assertFalse(permutation["closed"])
        self.assertTrue(permutation["register_set_cardinality_equal"])
        self.assertTrue(permutation["mapping_conflicts"])

    def test_diagnose_observes_two_input_operand_order_without_source_claim(self):
        order = self.diagnose(
            [_access_row(0, "or r0, r3, r0")],
            [_access_row(0, "or r0, r0, r3")],
        )["strict"]["input_operand_order"]
        self.assertEqual(order["status"], "observed")
        self.assertEqual(order["finding_count"], 1)
        self.assertEqual(order["findings"][0]["classification"],
                         "same_opcode_destination_two_input_swap")
        self.assertFalse(order["findings"][0]["source_commutation_proven"])

    def test_diagnose_stack_home_swap_is_separate_from_register_permutation(self):
        result = self.diagnose(
            [_access_row(0, "stw r3, 0x68(r1)"), _access_row(4, "stw r4, 0x6c(r1)")],
            [_access_row(0, "stw r3, 0x6c(r1)"), _access_row(4, "stw r4, 0x68(r1)")],
        )
        pairs = result["strict"]["stack_home"]["d_form"]["pairs"]
        self.assertEqual({(pair["target_offset"], pair["candidate_offset"])
                          for pair in pairs}, {(0x68, 0x6c), (0x6c, 0x68)})
        self.assertTrue(all(pair["status"] == "changed" for pair in pairs))
        self.assertEqual(result["strict"]["register_permutation"]["status"], "none")

    def test_diagnose_branch_destination_change_blocks_exact_and_permutation(self):
        result = self.diagnose(
            [_branch_row(0, "b 8", 8), _access_row(4, "blr"), _access_row(8, "blr")],
            [_branch_row(0, "b 4", 4), _access_row(4, "blr"), _access_row(8, "blr")],
        )
        self.assertEqual(result["strict"]["branch_destinations"]["status"], "changed")
        self.assertFalse(result["strict"]["gates"]["branch_destination_exact"])
        self.assertEqual(result["strict"]["register_permutation"]["status"], "rejected")
        self.assertEqual(result["strict"]["register_permutation"]["reason"],
                         "branch_destination_changed")

    def test_diagnose_varinfo_missing_usage_is_unknown_not_zero(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "locals": [{"name": "pathStack", "rclass": 4, "reg": 15}],
        }), encoding="utf-8")
        result = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))
        varinfo = result["varinfo"]
        self.assertEqual(varinfo["status"], "ok")
        self.assertEqual(varinfo["named_locals"][0]["name"], "pathStack")
        self.assertEqual(varinfo["named_locals"][0]["rclass"], 4)
        self.assertEqual(varinfo["score_relation"]["status"], "UNKNOWN")
        self.assertEqual(varinfo["score_relation"]["unknown_count"], 1)
        self.assertNotIn(0, varinfo["score_relation"]["known_scores"])
        self.assertFalse(varinfo["compiler_output_binding"]["physical_proof"])

    def test_diagnose_varinfo_usage_ties_are_named_without_register_mapping(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "locals": [
                {"name": "uselessdecl", "usage": 14, "rclass": 4, "reg": 14},
                {"name": "pathStack", "usage": 15, "rclass": 4, "reg": 15},
                {"name": "nextMasu", "usage": 15, "rclass": 4, "reg": 16},
            ],
        }), encoding="utf-8")
        varinfo = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]
        self.assertEqual(varinfo["score_relation"]["status"], "TIED")
        self.assertEqual(varinfo["score_relation"]["known_scores"], [14, 15])
        self.assertEqual(varinfo["score_relation"]["tie_names"], ["nextMasu", "pathStack"])
        self.assertFalse(varinfo["compiler_output_binding"]["authority_advanced"])

    def test_diagnose_varinfo_priority_known_40_and_42(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
            "locals": [
                {"name": "xy", "flags": 0x40, "usage": 100000},
                {"name": "z", "flags": 0x42, "usage": 100000},
            ],
        }), encoding="utf-8")
        hint = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]["priority_hint"]
        self.assertEqual(hint["status"], "known")
        self.assertEqual(hint["origin"], "inline_assembly_operand_priority")
        self.assertEqual(hint["flagged_names"], ["xy", "z"])
        self.assertEqual(hint["flagged_count"], 2)
        self.assertEqual(hint["conditional_o0_usage"], 100000)
        self.assertEqual(hint["allocator_effect"], "conditional_o0_usage_100000")
        self.assertTrue(hint["optimization_unverified"])
        self.assertEqual(hint["declaration_reordering"], "conditional_o0_priority_barrier")
        self.assertFalse(hint["authority_advanced"])

    def test_diagnose_varinfo_priority_unflagged_and_wrong_compiler(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
            "locals": [{"name": "ordinary", "flags": 0, "usage": 9}],
        }), encoding="utf-8")
        hint = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]["priority_hint"]
        self.assertEqual(hint["status"], "known")
        self.assertEqual(hint["origin"], "ordinary_observed_usage")
        self.assertEqual(hint["ordinary_count"], 1)
        self.assertEqual(hint["allocator_effect"], "not_applicable")

        path.write_text(json.dumps({
            "function": "FocusFunction",
            "compiler_sha256": "0" * 64,
            "locals": [{"name": "asm", "flags": 0x40, "usage": 100000}],
        }), encoding="utf-8")
        unknown = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]["priority_hint"]
        self.assertEqual(unknown["status"], "UNKNOWN")
        self.assertEqual(unknown["origin"], "UNKNOWN")
        self.assertEqual(unknown["allocator_effect"], "UNKNOWN")
        self.assertFalse(unknown["authority_advanced"])

    def test_diagnose_varinfo_priority_rejects_malformed_flags_and_function(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
            "locals": [{"name": "bad", "flags": "0x40", "usage": 100000}],
        }), encoding="utf-8")
        malformed = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]["priority_hint"]
        self.assertEqual(malformed["status"], "UNKNOWN")
        self.assertEqual(malformed["invalid_count"], 1)
        self.assertEqual(malformed["declaration_reordering"], "UNKNOWN")

        path.write_text(json.dumps({
            "function": "OtherFunction",
            "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
            "locals": [{"name": "asm", "flags": 0x40, "usage": 100000}],
        }), encoding="utf-8")
        mismatch = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))["varinfo"]
        self.assertEqual(mismatch["status"], "function_mismatch")
        self.assertEqual(mismatch["priority_hint"]["status"], "UNKNOWN")
        self.assertFalse(mismatch["priority_hint"]["authority_advanced"])

        for fields in (
            {"locals": [{"name": "x", "flags": 64}]},
            {"function": "FocusFunction", "locals": [False]},
            {"function": "FocusFunction", "locals": [{"name": "x", "flags": True}]},
            {"function": "FocusFunction", "locals": [{"name": "x", "flags": 256}]},
            {"function": "FocusFunction", "locals": [{"name": "x", "flags": 64}] * 2},
        ):
            with self.subTest(fields=fields):
                path.write_text(json.dumps({
                    "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
                    **fields,
                }), encoding="utf-8")
                result = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))
                self.assertEqual(result["varinfo"]["priority_hint"]["status"], "UNKNOWN")

    def test_diagnose_varinfo_priority_raw_trace_is_unbound(self):
        path = self.root / "varinfo.json"
        path.write_text(json.dumps({
            "function": "FocusFunction",
            "compiler_sha256": frontier.VARINFO_PRIORITY_COMPILER_SHA256,
            "locals": [{"name": "asm", "flags": 0x40, "usage": 100000}],
            "assignment_snapshots": [{"index": 0, "locals": [{"name": "asm", "flags": 0x40}]}],
        }), encoding="utf-8")
        result = self.diagnose([_access_row(0, "blr")], varinfo=Path("varinfo.json"))
        hint = result["varinfo"]["priority_hint"]
        self.assertEqual(hint["raw_trace"], "unbound")
        self.assertEqual(hint["source_binding"], "not_advanced")
        self.assertFalse(hint["authority_advanced"])
        self.assertEqual(result["varinfo"]["compiler_output_binding"]["status"], "unproven")
        self.assertFalse(result["authority_advanced"])

    def test_diagnose_strict_data_relocation_only_is_not_physical_proof(self):
        target = [_access_row(0, "lfs f1, pool@sda21"), _access_row(4, "blr")]
        data_report = copy.deepcopy(self.report)
        data_target = data_report["left"]["symbols"][1]
        data_candidate = data_report["right"]["symbols"][1]
        data_target["instructions"] = copy.deepcopy(target)
        data_candidate["instructions"] = copy.deepcopy(target)
        for row in (data_target["instructions"][0], data_candidate["instructions"][0]):
            row["instruction"]["relocation"] = {"target_symbol": 99, "type_name": "R_PPC_EMB_SDA21"}
        result = self.diagnose(target, target, data_report=data_report)
        residual = result["strict_vs_data"]
        self.assertEqual(residual["status"], "relocation_only")
        self.assertEqual(residual["residual_kinds"], {"relocation_annotation": 2})
        self.assertEqual(residual["relocation_attribution"]["status"], "diagnostic_only")
        self.assertFalse(residual["relocation_attribution"]["physical_proof"])
        self.assertEqual(result["data"]["summary"], "residuals_in_strict_vs_data")
        self.assertFalse(result["physical_proof"])

    def test_diagnose_large_input_stays_within_32k_output_cap_and_cli(self):
        rows = [_access_row(index * 4, f"lwz r3, 0x{index * 4:x}(r1)") for index in range(2200)]
        result = self.diagnose(rows, list(reversed(rows)))
        self.assertLessEqual(result["output_bytes"], frontier.DIAGNOSE_OUTPUT_LIMIT)
        self.assertLessEqual(result["output_bytes"], frontier.DIAGNOSE_FOCUS_LIMIT)
        self.assertLessEqual(len(frontier.canonical(result)) + 1, frontier.DIAGNOSE_OUTPUT_LIMIT)
        stack = result["strict"].get("stack_home")
        self.assertIsInstance(stack, dict)
        for category in (stack.get("d_form"), stack.get("addi_pointer")):
            if isinstance(category, dict):
                self.assertEqual(category["returned_pair_count"], len(category["pairs"]))
        with unittest.mock.patch("builtins.print") as output:
            self.assertEqual(frontier.main([
                "--root", str(self.root), "diagnose", "--strict", "strict.json",
                "--function", "FocusFunction",
            ]), 0)
        self.assertTrue(output.called)


class BatchCliTests(unittest.TestCase):
    def test_batch_cli_dispatches_without_running_measurement(self):
        with mock.patch("tools.recovery_evaluate.dispatch_batch", return_value=0) as dispatch:
            result = frontier.main([
                "--root", "owner-root", "evaluate-batch",
                "--index", "build/current.json", "--manifest", "build/jobs.json",
                "--out", "build/batch.json", "--objdiff", "objdiff.exe",
                "--readelf", "readelf.exe", "--command-json", "build/argv.json",
                "--workers", "2", "--timeout", "120",
            ])
        self.assertEqual(result, 0)
        args = dispatch.call_args.args[0]
        self.assertEqual(args.action, "evaluate-batch")
        self.assertEqual(args.manifest, Path("build/jobs.json"))
        self.assertEqual(args.workers, 2)
        self.assertEqual(args.timeout, 120)


if __name__ == "__main__":
    unittest.main()
