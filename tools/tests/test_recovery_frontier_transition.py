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
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(groups.main(["--strict", str(path), "--function", "f",
                                              "--baseline-strict", str(path)]), 2)


if __name__ == "__main__":
    unittest.main()
