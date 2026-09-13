import copy
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest

from tools import recovery_causal_groups as groups


def report(scores):
    def symbol(name, score):
        return {"name": name, "size": "4", "match_percent": score,
                "instructions": [{"instruction": {"formatted": "blr", "address": "0"}}]}
    return {side: {"symbols": [symbol(name, score) for name, score in scores.items()]}
            for side in ("left", "right")}


class FrontierTransitionTests(unittest.TestCase):
    def frame_report(self, target, candidate, score=90):
        document = report(dict(f=score))
        for side, stream in (("left", target), ("right", candidate)):
            document[side]["symbols"][0]["instructions"] = [
                {"instruction": {"formatted": text, "address": str(i * 4)}}
                for i, text in enumerate(stream)]
        return document

    def test_closed_frame_regression_despite_score_gain_and_same_size(self):
        prologue = ["mflr r0", "stw r0,4(r1)", "STWU R1, -0x28(R1)", "li r3,0", "blr"]
        changed = ["mflr r0", "stw r0,4(r1)", "stwu r1,-48(r1)", "li r3,0", "blr"]
        delta = groups.compare_match_frontiers(self.frame_report(prologue, prologue),
                                               self.frame_report(prologue, changed, 98))
        row = delta["improved"][0]
        self.assertEqual((row["target_frame_bytes"], row["baseline_frame_bytes"],
                          row["candidate_frame_bytes"]), (40, 40, 48))
        self.assertTrue(row["closed_frame_regression"])
        self.assertEqual(delta["counts"]["closed_frame_regressions"], 1)
        self.assertTrue(delta["mixed_gain_regression"])
        self.assertEqual(delta["changed_size"], [])

    def test_exact_frame_preserved_with_alignment_gap(self):
        stream = ["stwu r1,-0x28(r1)", "li r3,0", "blr"]
        a, b = self.frame_report(stream, stream), self.frame_report(stream, stream, 98)
        b["right"]["symbols"][0]["instructions"].insert(0, {"diff_kind": "DIFF_DELETE"})
        delta = groups.compare_match_frontiers(a, b)
        self.assertEqual(delta["improved"][0]["frame_transition"], "exact_frame_preserved")
        self.assertFalse(delta["improved"][0]["closed_frame_regression"])

    def test_unknown_and_ambiguous_frames_are_not_regressions(self):
        stream = ["stwu r1,-40(r1)", "li r3,0", "blr"]
        for candidate in (["blr"], ["stwu"], ["stwux r1,r1,r0", "blr"],
                          ["stwu r1,-40(r1)", "stwu r1,-8(r1)", "blr"],
                          ["addi r1,r1,-40", "blr"], ["stwu r1,unknown(r1)", "blr"],
                          ["stwu r1,-40(r1)", "mr r1,r3", "blr"],
                          ["b somewhere", "stwu r1,-40(r1)"],
                          ["mflr r0"] * 32 + stream):
            with self.subTest(candidate=candidate[:3]):
                delta = groups.compare_match_frontiers(self.frame_report(stream, stream),
                                                       self.frame_report(stream, candidate))
                row = delta["frame_transitions"][0]
                self.assertIsNone(row["candidate_frame_bytes"])
                self.assertIsNone(row["closed_frame_regression"])
                self.assertEqual(row["frame_transition"], "unknown")
                self.assertEqual(delta["counts"]["closed_frame_regressions"], 0)

    def test_different_target_frame_is_unknown(self):
        a = self.frame_report(["stwu r1,-40(r1)", "blr"], ["stwu r1,-40(r1)", "blr"])
        b = self.frame_report(["stwu r1,-48(r1)", "blr"], ["stwu r1,-48(r1)", "blr"])
        row = groups.compare_match_frontiers(a, b)["frame_transitions"][0]
        self.assertEqual(row["frame_transition"], "unknown")
        self.assertIsNone(row["closed_frame_regression"])

    def test_unchanged_frames_do_not_hide_late_regression(self):
        a = report({f"a{i:02}": 100 for i in range(30)})
        tail = self.frame_report(["stwu r1,-40(r1)", "blr"], ["stwu r1,-40(r1)", "blr"])
        for side in ("left", "right"):
            tail[side]["symbols"][0]["name"] = "z_late"
            a[side]["symbols"].extend(tail[side]["symbols"])
        b = copy.deepcopy(a)
        b["right"]["symbols"][-1]["instructions"][0]["instruction"]["formatted"] = "stwu r1,-48(r1)"
        delta = groups.compare_match_frontiers(a, b)
        self.assertEqual(delta["counts"]["frame_comparisons"], 31)
        self.assertEqual(delta["counts"]["frame_transitions"], 1)
        self.assertEqual(delta["counts"]["unchanged"], 30)
        self.assertFalse(delta["details_truncated"])
        self.assertTrue(delta["frame_transitions"][0]["closed_frame_regression"])
        self.assertEqual(groups.compare_match_frontiers(a, a)["frame_transitions"], [])

    def test_unknown_knowledge_change_is_reported(self):
        a = self.frame_report(["blr"], ["blr"])
        b = self.frame_report(["blr"], ["stwu"])
        delta = groups.compare_match_frontiers(a, b)
        self.assertEqual(len(delta["frame_transitions"]), 1)
        self.assertEqual(delta["counts"]["unchanged"], 0)

    def test_mixed_and_lost_exact(self):
        a, b = report(dict(f=80, g=100, h=90)), report(dict(f=100, g=99, h=80))
        delta = groups.compare_match_frontiers(a, b)
        self.assertEqual([r["function"] for r in delta["new_score_exact"]], ["f"])
        self.assertEqual([r["function"] for r in delta["lost_score_exact"]], ["g"])
        self.assertEqual(delta["counts"]["regressed"], 2)
        self.assertTrue(delta["mixed_gain_regression"])
        self.assertFalse(delta["authority_advanced"])

    def test_no_gain_and_read_only(self):
        a = report(dict(f=90, g=100))
        saved = copy.deepcopy(a)
        delta = groups.compare_match_frontiers(a, a)
        self.assertEqual(delta["counts"]["unchanged"], 2)
        self.assertFalse(delta["mixed_gain_regression"])
        self.assertEqual(delta["improved"], [])
        self.assertEqual(a, saved)

    def test_unknown_or_missing_is_not_a_gain(self):
        for missing in (False, True):
            a, b = report(dict(f=None)), report(dict(f=100))
            if missing:
                a["right"]["symbols"] = []
                a["left"]["symbols"][0]["match_percent"] = 100
            delta = groups.compare_match_frontiers(a, b)
            self.assertEqual(delta["new_score_exact"], [])
            self.assertEqual(delta["improved"], [])
            self.assertEqual(delta["counts"]["unscored_comparisons"], 1)
            reverse = groups.compare_match_frontiers(b, a)
            self.assertEqual(len(reverse["lost_score_exact"]), 1)
            self.assertEqual(reverse["regressed"], [])

    def test_incompatible_targets(self):
        a = report(dict(f=90))
        with self.assertRaisesRegex(ValueError, "target function set"):
            groups.compare_match_frontiers(a, report(dict(g=90)))
        b = copy.deepcopy(a)
        b["left"]["symbols"][0]["size"] = "8"
        with self.assertRaisesRegex(ValueError, "target size: f"):
            groups.compare_match_frontiers(a, b)

    def test_duplicate_report_indices_cannot_bridge(self):
        a = report(dict(f=90))
        for side in ("left", "right"):
            a[side]["sections"] = [{"kind": "SECTION_CODE"}]
            a[side]["symbols"].append(copy.deepcopy(a[side]["symbols"][0]))
            for i, symbol in enumerate(a[side]["symbols"]):
                symbol["target_symbol"] = i
        self.assertEqual(groups.summarize_match_scores(a)["functions"], 2)
        with self.assertRaisesRegex(ValueError, "cross-report duplicate"):
            groups.compare_match_frontiers(a, copy.deepcopy(a))

    def test_size_only_and_closed_size_regression(self):
        a, b = report(dict(f=90)), report(dict(f=90))
        b["right"]["symbols"][0]["size"] = "8"
        delta = groups.compare_match_frontiers(a, b)
        self.assertEqual(delta["counts"]["score_unchanged"], 1)
        self.assertEqual(delta["counts"]["unchanged"], 0)
        self.assertEqual(delta["counts"]["closed_size_regressions"], 1)
        self.assertTrue(delta["changed_size"][0]["closed_size_regression"])
        self.assertEqual(delta["regressed"], [])

    def test_bounded_details(self):
        delta = groups.compare_match_frontiers(report({str(i): 90 for i in range(30)}),
                                               report({str(i): 100 for i in range(30)}))
        self.assertEqual(delta["counts"]["improved"], 30)
        self.assertEqual(len(delta["improved"]), 24)
        self.assertTrue(delta["details_truncated"])

    def test_cli_and_legacy_owner_mode(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "strict.json"
            document = report(dict(f=100))
            path.write_text(json.dumps(document), encoding="utf-8")
            for extra, expected in (([], groups.summarize_owner(document)),
                                    (["--baseline-strict", str(path)], groups.compare_match_frontiers(document, document))):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    code = groups.main(["--strict", str(path), "--owner-summary", *extra])
                self.assertEqual(code, 0)
                self.assertEqual(json.loads(output.getvalue()), expected)
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(groups.main(["--strict", str(path), "--function", "f",
                                              "--baseline-strict", str(path)]), 0)
            self.assertEqual(json.loads(output.getvalue()),
                             groups.compare_function_constraints(document, document, "f"))
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(groups.main(["--strict", str(path), "--function", "f",
                                              "--baseline-strict", str(path),
                                              "--before", str(path)]), 2)


if __name__ == "__main__":
    unittest.main()
