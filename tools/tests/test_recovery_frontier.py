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


if __name__ == "__main__":
    unittest.main()
