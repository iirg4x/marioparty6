"""Narrow advisory rewrite checks; no compiler or retail inputs."""
import json
from pathlib import Path
import unittest

from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import report, row


def fragments(count=7):
    before = "\n".join(f"state->rows[{i}] = value;" for i in range(8))
    after = "{ float **slot = state->rows;\n" + "\n".join(
        "*slot++ = value;" for _ in range(count)) + "\n}"
    return before, after


class SourceReviewTests(unittest.TestCase):
    def test_missing_store_warning(self):
        result = groups.review_source_store_counts(*fragments())
        self.assertEqual((result["before_store_count"], result["after_store_count"]), (8, 7))
        self.assertIn("8 indexed stores become 7", result["warnings"][0])
        self.assertFalse(result["authority"])

    def test_complete_stores_no_warning(self):
        result = groups.review_source_store_counts(*fragments(8))
        self.assertEqual(result["status"], "observed")
        self.assertEqual(result["warnings"], [])
        self.assertIn("semantic equivalence UNKNOWN", result["scope"])

    def test_comments_never_count_and_literals_are_unknown(self):
        before, after = fragments()
        result = groups.review_source_store_counts(before, after[:-1] +
            "/* *slot++ = value; */ // *slot++ = value;\n}")
        self.assertEqual(result["after_store_count"], 7)
        for literal in ('"*slot++ = value;"', "'*'"):
            result = groups.review_source_store_counts(before, after[:-1] + f"text = {literal}; }}")
            self.assertEqual(result["status"], "UNKNOWN")
            self.assertEqual(result["warnings"], [])

    def test_unsupported_control_alias_and_noncontiguous_are_unknown(self):
        before, after = fragments()
        for changed in ("{ float **slot = state->rows; for(i=0;i<8;i++) *slot++=value; }",
                        after.replace("*slot++", "if (ready) *slot++", 1),
                        after.replace("*slot++", "slot = other; *slot++", 1),
                        after.replace("*slot++", "state = other; *slot++", 1),
                        after.replace("*slot++", "{ *slot++", 1) + "}"):
            self.assertEqual(groups.review_source_store_counts(before, changed)["status"], "UNKNOWN")
        self.assertEqual(groups.review_source_store_counts(before.replace("[3]", "[8]"), after)["status"], "UNKNOWN")

    def test_validator_preserves_advisory_and_existing_anchor(self):
        before, after = fragments()
        rows = [row("blr", 0)]
        document = report(rows, rows)
        for side in ("left", "right"):
            document[side]["symbols"][0]["name"] = "generic_init"
        packet = groups.decision_packet(document, "generic_init", before, 1, 8,
            "Review destination initialization", 0, 0, decision_mode="source-hypothesis")
        answer = dict(status="hypothesis", function="generic_init", packet_sha256=packet["packet_sha256"],
            answer=dict(cause="A consumed destination cursor.", prediction="Store scheduling may change.",
                        source_change=dict(before=before, after=after)), evidence_rows=[0],
            missing_evidence="Uncompiled hypothesis.")
        receipt = groups.validate_decision_answer(packet, answer)
        self.assertEqual(receipt["status"], "valid_finding")
        self.assertFalse(receipt["authority"])
        self.assertTrue(receipt["source_review"]["warnings"])
        answer["answer"]["source_change"]["before"] = before + " absent = 0;"
        with self.assertRaisesRegex(ValueError, "unique bound"):
            groups.validate_decision_answer(packet, answer)

    def test_unrelated_function_safe(self):
        result = groups.review_source_store_counts("return value;", "return other;")
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["warnings"], [])

    def test_saved_answer_read_only_replay(self):
        folder = Path(__file__).resolve().parents[2] / "build/qwen-exev-comb-small-20260913/combiner-history-initialization"
        paths = [folder / "decision.json", folder / "answer/combiner-history-initialization.answer.txt"]
        if not all(path.is_file() for path in paths):
            self.skipTest("local support artifacts unavailable")
        raw = [path.read_bytes() for path in paths]
        receipt = groups.validate_decision_answer(*map(json.loads, raw))
        self.assertEqual(receipt["status"], "valid_finding")
        self.assertFalse(receipt["authority"])
        self.assertEqual(receipt["source_review"]["before_store_count"], 8)
        self.assertEqual(receipt["source_review"]["after_store_count"], 7)
        self.assertTrue(receipt["source_review"]["warnings"])
        self.assertEqual(raw, [path.read_bytes() for path in paths])


if __name__ == "__main__":
    unittest.main()
