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


if __name__ == "__main__":
    unittest.main()
