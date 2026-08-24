import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from tools import donor_cfg_align as module


def report(symbol: str = "CapGuideRotYSet", match: float = 76.0) -> dict:
    return {
        "left": {
            "symbols": [
                {
                    "name": symbol,
                    "kind": "SYMBOL_FUNCTION",
                    "size": "32",
                    "target_symbol": 0,
                    "match_percent": match,
                    "instructions": [
                        {
                            "diff_kind": "REG_SWAP",
                            "instruction": {"formatted": "mr r3,r4"},
                        }
                    ],
                }
            ]
        },
        "right": {
            "symbols": [
                {"name": symbol, "kind": "SYMBOL_FUNCTION", "size": "32"}
            ]
        },
    }


class DonorCfgAlignTests(unittest.TestCase):
    def run_alignment(
        self,
        current: str,
        donor: str,
        *,
        symbol: str = "CapGuideRotYSet",
        report_value: dict | None = None,
    ) -> dict:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            current_path = root / "current.c"
            donor_path = root / "donor.c"
            report_path = root / "objdiff.json"
            current_path.write_text(current, encoding="utf-8")
            donor_path.write_text(donor, encoding="utf-8")
            report_path.write_text(
                json.dumps(report_value or report(symbol)), encoding="utf-8"
            )
            return module.align_donor_cfg(
                report_path,
                focus_symbol=symbol,
                current_source=current_path,
                donor_source=donor_path,
            )

    def test_mp5_call_argument_identity_is_ranked_with_spans(self) -> None:
        result = self.run_alignment(
            """
void CapGuideRotYSet(int masuId, int value) {
    CapGuideRotYSet(masuId, candidateRot[j]);
}
""",
            """
void CapGuideRotYSet(int masuId, int value) {
    /* MP5 keeps the indexed candidate rotation at the direct call. */
    CapGuideRotYSet(masuId, candidateRot[i]);
}
""",
        )
        self.assertEqual(result["authentication"]["status"], "authenticated")
        kinds = {item["kind"] for item in result["hypotheses"]}
        self.assertTrue(
            {"call_argument_identity", "call_argument_topology"} & kinds,
            result["hypotheses"],
        )
        hypothesis = next(
            item
            for item in result["hypotheses"]
            if item["kind"].startswith("call_argument")
        )
        snippets = " ".join(
            str(span.get("snippet", "")) for span in hypothesis["evidence"]
        )
        self.assertIn("candidateRot[i]", snippets)
        self.assertIn("candidateRot[j]", snippets)
        self.assertTrue(all("line_start" in span for span in hypothesis["evidence"] if span["side"] != "report"))

    def test_result_lifetime_materialization_is_ranked(self) -> None:
        result = self.run_alignment(
            """
float CapGuideRotYSet(Vec *v) {
    return VECMag(v);
}
""",
            """
float CapGuideRotYSet(Vec *v) {
    float t;
    t = VECMag(v);
    return t;
}
""",
        )
        lifetime = [
            item for item in result["hypotheses"] if item["kind"] == "result_lifetime"
        ]
        self.assertEqual(len(lifetime), 1, result["hypotheses"])
        self.assertIn("t = VECMag", " ".join(span.get("snippet", "") for span in lifetime[0]["evidence"]))

    def test_direct_call_inlining_is_ranked(self) -> None:
        result = self.run_alignment(
            """
int CapGuideRotYSet(int value) {
    value += 1;
    return value;
}
""",
            """
int CapGuideRotYSet(int value) {
    capGuideStep(value);
    return value;
}
""",
        )
        self.assertTrue(
            any(item["kind"] == "direct_call_inlining" for item in result["hypotheses"]),
            result["hypotheses"],
        )

    def test_comments_do_not_create_calls_or_definitions(self) -> None:
        result = self.run_alignment(
            """
/* void CapGuideRotYSet(void) { fake_call(); } */
void CapGuideRotYSet(int value) {
    return value;
}
""",
            """
void CapGuideRotYSet(int value) {
    // fake_call() must not be considered donor evidence.
    return value;
}
""",
        )
        self.assertEqual(result["hypotheses"], [])
        self.assertEqual(result["verdict"], "no_safe_hypotheses")

    def test_ambiguous_and_missing_donor_fail_closed(self) -> None:
        current = "int CapGuideRotYSet(void) { return 0; }"
        with self.assertRaisesRegex(module.DonorCfgError, "ambiguous"):
            self.run_alignment(
                current,
                "int CapGuideRotYSet(void) { return 0; }\nint CapGuideRotYSet(void) { return 1; }",
            )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            current_path = root / "current.c"
            report_path = root / "report.json"
            current_path.write_text(current, encoding="utf-8")
            report_path.write_text(json.dumps(report()), encoding="utf-8")
            with self.assertRaisesRegex(module.DonorCfgError, "does not exist"):
                module.align_donor_cfg(
                    report_path,
                    focus_symbol="CapGuideRotYSet",
                    current_source=current_path,
                    donor_source=root / "no-donor.c",
                )
            unrelated = root / "unrelated.c"
            unrelated.write_text("int Other(void) { return 1; }", encoding="utf-8")
            with self.assertRaisesRegex(module.DonorCfgError, "was not found"):
                module.align_donor_cfg(
                    report_path,
                    focus_symbol="CapGuideRotYSet",
                    current_source=current_path,
                    donor_source=unrelated,
                )
            with self.assertRaisesRegex(module.DonorCfgError, "was not found"):
                module.align_donor_cfg(
                    report_path,
                    focus_symbol="CapGuideRotYSet",
                    current_source=current_path,
                    donor_source=unrelated,
                    donor_symbol="Wrong",
                )

    def test_report_pointer_conflicts_and_ranges_fail_closed(self) -> None:
        current = "int CapGuideRotYSet(void) { return 0; }"
        donor = "int CapGuideRotYSet(void) { return 1; }"
        base = report()

        def assert_rejected(value: dict, message: str) -> None:
            with tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                current_path = root / "current.c"
                donor_path = root / "donor.c"
                report_path = root / "report.json"
                current_path.write_text(current, encoding="utf-8")
                donor_path.write_text(donor, encoding="utf-8")
                report_path.write_text(json.dumps(value), encoding="utf-8")
                with self.assertRaisesRegex(module.DonorCfgError, message):
                    module.align_donor_cfg(
                        report_path,
                        focus_symbol="CapGuideRotYSet",
                        current_source=current_path,
                        donor_source=donor_path,
                    )

        candidate_other = json.loads(json.dumps(base))
        candidate_other["left"]["symbols"] = [
            {"name": "CapGuideRotYSet", "kind": "SYMBOL_FUNCTION", "target_symbol": 1},
            {"name": "Other", "kind": "SYMBOL_FUNCTION", "target_symbol": 0},
        ]
        candidate_other["right"]["symbols"] = [
            {"name": "CapGuideRotYSet", "kind": "SYMBOL_FUNCTION"},
            {"name": "Other", "kind": "SYMBOL_FUNCTION"},
        ]
        assert_rejected(candidate_other, "conflicts")

        out_of_range = json.loads(json.dumps(base))
        out_of_range["left"]["symbols"][0]["target_symbol"] = 9
        assert_rejected(out_of_range, "out of range")

        target_conflict = json.loads(json.dumps(base))
        target_conflict["left"]["symbols"].append(
            {"name": "Other", "kind": "SYMBOL_FUNCTION"}
        )
        target_conflict["right"]["symbols"] = [
            {"name": "CapGuideRotYSet", "kind": "SYMBOL_FUNCTION", "candidate_symbol": 1},
            {"name": "Other", "kind": "SYMBOL_FUNCTION"},
        ]
        assert_rejected(target_conflict, "conflicts")

    def test_deterministic_order_and_json_cli(self) -> None:
        current = """
int CapGuideRotYSet(int a, int b) {
    first(a, currentArg);
    second(b, 2);
    return a;
}
"""
        donor = """
int CapGuideRotYSet(int a, int b) {
    second(b, donorArg);
    first(a, donorArg);
    return a;
}
"""
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "current.c").write_text(current, encoding="utf-8")
            (root / "donor.c").write_text(donor, encoding="utf-8")
            (root / "report.json").write_text(json.dumps(report()), encoding="utf-8")
            first = module.align_donor_cfg(
                root / "report.json",
                focus_symbol="CapGuideRotYSet",
                current_source=root / "current.c",
                donor_source=root / "donor.c",
            )
            second = module.align_donor_cfg(
                root / "report.json",
                focus_symbol="CapGuideRotYSet",
                current_source=root / "current.c",
                donor_source=root / "donor.c",
            )
            self.assertEqual(first, second)
            process = subprocess.run(
                [
                    sys.executable,
                    "tools/donor_cfg_align.py",
                    "--report",
                    str(root / "report.json"),
                    "--focus-symbol",
                    "CapGuideRotYSet",
                    "--current-source",
                    str(root / "current.c"),
                    "--donor-source",
                    str(root / "donor.c"),
                    "--json",
                ],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(process.returncode, 0, process.stderr)
            self.assertEqual(json.loads(process.stdout), module.align_donor_cfg(
                root / "report.json",
                focus_symbol="CapGuideRotYSet",
                current_source=root / "current.c",
                donor_source=root / "donor.c",
            ))

    def test_constant_and_dead_local_shaping_is_not_proposed(self) -> None:
        result = self.run_alignment(
            "int CapGuideRotYSet(void) { int dead = 1; return 0; }",
            "int CapGuideRotYSet(void) { int dead = 2; int unused = 3; return 0; }",
        )
        self.assertFalse(
            any(item["kind"] in {"missing_assignment", "extra_assignment"} for item in result["hypotheses"])
        )
        self.assertFalse(any("register" in item["kind"] for item in result["hypotheses"]))
        self.assertTrue(result["safe_to_apply"] is False)

    def test_register_storage_difference_is_closed(self) -> None:
        result = self.run_alignment(
            "float CapGuideRotYSet(Vec *v) { return VECMag(v); }",
            "float CapGuideRotYSet(Vec *v) { register float t; t = VECMag(v); return t; }",
        )
        self.assertFalse(result["hypotheses"])
        self.assertIn("register storage shaping is closed", result["closed"])

    def test_identifier_digits_are_not_constant_only(self) -> None:
        result = self.run_alignment(
            "int CapGuideRotYSet(int value) { return helper(value, foo1); }",
            "int CapGuideRotYSet(int value) { return helper(value, foo2); }",
        )
        self.assertTrue(
            any(item["kind"] == "call_argument_identity" for item in result["hypotheses"]),
            result["hypotheses"],
        )

    def test_chained_dead_writes_are_not_reads_or_hypotheses(self) -> None:
        result = self.run_alignment(
            "int CapGuideRotYSet(int x) { return 0; }",
            """
int CapGuideRotYSet(int x) {
    int t;
    t = h(x);
    t = g(x);
    return 0;
}
""",
        )
        self.assertFalse(result["hypotheses"], result["hypotheses"])
        self.assertTrue(
            any("dead-local" in item for item in result["closed"]),
            result["closed"],
        )

    def test_pointer_output_closes_aggregate_and_assignment_shaping(self) -> None:
        result = self.run_alignment(
            "Vec *CapGuideRotYSet(int x) { return 0; }",
            """
Vec *CapGuideRotYSet(int x) {
    Vec temp;
    temp.x = x;
    temp.y = x;
    return &temp;
}
""",
        )
        kinds = {item["kind"] for item in result["hypotheses"]}
        self.assertNotIn("missing_assignment", kinds)
        self.assertNotIn("extra_assignment", kinds)
        self.assertNotIn("aggregate_temporary", kinds)
        self.assertIn("pointer-output aggregate/assignment shaping is closed", result["closed"])

    def test_non_explicit_pair_closes_aggregate_and_assignment_shaping(self) -> None:
        report_value = report()
        del report_value["left"]["symbols"][0]["target_symbol"]
        result = self.run_alignment(
            "int CapGuideRotYSet(int x) { return x; }",
            """
int CapGuideRotYSet(int x) {
    Vec temp;
    temp.x = x;
    temp.y = x;
    return x;
}
""",
            report_value=report_value,
        )
        kinds = {item["kind"] for item in result["hypotheses"]}
        self.assertNotIn("missing_assignment", kinds)
        self.assertNotIn("extra_assignment", kinds)
        self.assertNotIn("aggregate_temporary", kinds)
        self.assertFalse(result["authentication"]["pairing"]["explicit"])
        self.assertIn(
            "non-explicit target/candidate pairing closes assignment/aggregate shaping",
            result["closed"],
        )

    def test_branch_aggregate_and_prototype_cast_topics_are_ranked(self) -> None:
        result = self.run_alignment(
            """
int CapGuideRotYSet(int value) {
    return helper(value);
}
""",
            """
int CapGuideRotYSet(int value) {
    Vec temp;
    temp.x = value;
    temp.y = value;
    if (value) {
        return helper((f32)value);
    }
    return 0;
}
""",
        )
        kinds = {item["kind"] for item in result["hypotheses"]}
        self.assertIn("branch_loop_shape", kinds)
        self.assertIn("aggregate_temporary", kinds)
        self.assertIn("prototype_cast", kinds)

    def test_source_chronology_reports_call_return_and_evaluation_order(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "source.c"
            path.write_text(
                "void Focus(void) { int value; value = First(); Second(value); }\n",
                encoding="utf-8",
            )
            result = module.source_chronology(path, symbol="Focus")
        self.assertEqual(result["schema"], "donor_cfg_source_chronology/v1")
        self.assertFalse(result["authority_advanced"])
        self.assertEqual([row["callee"] for row in result["calls"]], ["First", "Second"])
        self.assertEqual(result["calls"][0]["assigned_lhs"], "value")
        self.assertEqual([row["evaluation_ordinal"] for row in result["calls"]], [1, 2])
        self.assertRegex(result["chronology_sha256"], r"^[0-9a-f]{64}$")


if __name__ == "__main__":
    unittest.main()
