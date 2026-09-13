"""Focused physical producer tests for immediate rotate/mask aliases."""
from __future__ import annotations

import hashlib
import copy
import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import row, report


class RotateProducerTests(unittest.TestCase):
    def test_eabi_call_preservation_is_opt_in_and_clears_volatile_values(self):
        for call in ("bl _tosGetProfileU32", "bla 0x100", "bctrl", "blrl"):
            with self.subTest(call=call):
                rows = [row("li r31, 9", 0), row("li r3, 4", 4),
                        row("fmr f14, f1", 8), row("fmr f13, f1", 12), row(call, 16),
                        row("add r31, r31, r3", 20), row("fadd f2, f14, f13", 24)]
                old = groups.producer_slice(rows, [5, 6])
                self.assertEqual(old, groups.producer_slice(rows, [5, 6], call_model="conservative"))
                self.assertNotIn("call_model", old)
                self.assertTrue(all("definition_row" not in use for n in old["nodes"] for use in n["uses"]))
                result = groups.producer_slice(rows, [5, 6], call_model="ppc-eabi")
                nodes = {n["row"]: n for n in result["nodes"]}
                self.assertEqual(nodes[5]["uses"][0], {"register": "r31", "definition_row": 0})
                self.assertEqual(nodes[6]["uses"][0], {"register": "f14", "definition_row": 2})
                for i in (5, 6):
                    self.assertEqual(nodes[i]["uses"][1]["reason"], "call boundary")
                self.assertFalse(result["call_model"]["callee_conformance_proven"])
                self.assertEqual(result["scope"], groups._EABI_PRODUCER_SCOPE)

    def test_eabi_mode_stops_at_cfg_unsupported_and_malformed_calls(self):
        for boundary in ("b 0xc", "mystery r0", "bl", "bl r3", "bl target, r3",
                         "bctrl r3", "blrl 4", "bl 0x3", "bl 0x100000000", "bl target + 4",
                         "bl. target", "bla -1"):
            with self.subTest(boundary=boundary):
                rows = [row("li r31, 9", 0), row(boundary, 4), row("nop", 8), row("mr r3, r31", 12)]
                nodes = groups.producer_slice(rows, [3], call_model="ppc-eabi")["nodes"]
                self.assertNotIn("definition_row", nodes[-1]["uses"][0])
        # A branch entering after a recognized call still starts a fresh block.
        rows = [row("b 0x10", 0), row("li r31, 4", 4), row("bl helper", 8),
                row("nop", 12), row("mr r3, r31", 16)]
        self.assertNotIn("definition_row", groups.producer_slice(rows, [4], call_model="ppc-eabi")["nodes"][-1]["uses"][0])

    def test_eabi_model_metadata_and_decision_seal(self):
        rows = [row("li r31, 4", 0), row("bl helper", 4), row("mr r3, r31", 8)]
        document = report(rows, rows)
        packet = groups.decision_packet(document, "f", "void f(void) {}", 1, 1,
                                        "Trace the value", 2, 2, producer_sites=[2],
                                        producer_call_model="ppc-eabi")
        groups.validate_decision_packet(packet)
        self.assertIn("recognized calls obey PowerPC EABI", groups.render_decision_prompt(packet))
        summary = groups.summarize_groups(document, "f", producers=4, producer_call_model="ppc-eabi")
        self.assertEqual(summary["producer_slice"]["target"]["call_model"], groups._producer_call_model("ppc-eabi"))
        for change in ("missing", "name", "scope", "preserved_registers", "callee_conformance_proven"):
            broken = copy.deepcopy(packet)
            sliced = broken["producer_context"]["target"]
            if change == "missing":
                del sliced["call_model"]
            elif change == "scope":
                sliced["scope"] = groups._PRODUCER_SCOPE
            else:
                sliced["call_model"][change] = "invalid"
            broken["packet_sha256"] = groups._digest({k: v for k, v in broken.items() if k != "packet_sha256"})
            with self.subTest(change=change), self.assertRaises(ValueError):
                groups.validate_decision_packet(broken)
        for mode in ("unknown", None, True):
            with self.subTest(mode=mode), self.assertRaises(ValueError):
                groups.producer_slice(rows, [2], call_model=mode)
        with self.assertRaises(ValueError):
            groups.summarize_groups(document, "f", producer_call_model="ppc-eabi")
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            groups.main(["--function", "f", "--producer-call-model", "unknown"])

    def test_eabi_existing_cli_summary_and_decision(self):
        rows = [row("li r31, 4", 0), row("bl helper", 4), row("mr r3, r31", 8)]
        with tempfile.TemporaryDirectory() as directory:
            strict = Path(directory) / "strict.json"
            source = Path(directory) / "f.c"
            strict.write_text(json.dumps(report(rows, rows)), encoding="utf-8")
            source.write_text("void f(void) {}", encoding="utf-8")
            base = ["--strict", str(strict), "--function", "f", "--producers", "4"]
            def run(arguments):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    self.assertEqual(groups.main(arguments), 0)
                return json.loads(output.getvalue())
            default = run(base)
            self.assertEqual(default, run(base + ["--producer-call-model", "conservative"]))
            modeled = run(base + ["--producer-call-model", "ppc-eabi"])
            self.assertIn("call_model", modeled["producer_slice"]["target"])
            decision = base + ["--support-source", str(source), "--source-lines", "1:1",
                               "--decision-question", "Trace the value", "--decision-rows", "2:2"]
            self.assertNotIn("producer_context", run(decision))
            packet = run(decision + ["--producer-call-model", "ppc-eabi"])
            groups.validate_decision_packet(packet)
            nodes = packet["producer_context"]["target"]["nodes"]
            self.assertEqual(nodes[-1]["uses"], [{"register": "r31", "definition_row": 0}])

    def test_divide_reads_both_gprs_before_defining_quotient(self):
        for op in ("divw", "divwu", "divwo", "divwuo"):
            for suffix in ("", "."):
                with self.subTest(opcode=op + suffix):
                    rows = [row("li r31, 16", 0), row("li r0, 4", 4),
                            row(f"{op}{suffix} r5, r31, r0", 8), row("mullw r0, r5, r0", 12)]
                    nodes = groups.producer_slice(rows, [3])["nodes"]
                    self.assertEqual(nodes[2]["defines"], "r5")
                    self.assertEqual(nodes[2]["uses"], [{"register": "r31", "definition_row": 0},
                                                       {"register": "r0", "definition_row": 1}])
                    self.assertEqual(nodes[3]["uses"], [{"register": "r5", "definition_row": 2},
                                                       {"register": "r0", "definition_row": 1}])
        nodes = groups.producer_slice([row("li r0, 4", 0), row("divwu r0, r0, r0", 4)], [1])["nodes"]
        self.assertEqual(nodes[1]["uses"], [{"register": "r0", "definition_row": 0}] * 2)

    def test_quotient_does_not_cross_cfg_or_call(self):
        for boundary in ("bl helper", "b 0xc"):
            rows = [row("divw r5, r31, r0", 0), row(boundary, 4),
                    row("nop", 8), row("clrlslwi r0, r5, 18, 2", 12)]
            nodes = groups.producer_slice(rows, [3])["nodes"]
            self.assertEqual(nodes[-1]["uses"][0]["status"], "UNKNOWN")
            self.assertNotIn("definition_row", nodes[-1]["uses"][0])

    def test_qenqueueone_first_new_element_block(self):
        # Hash-bound report rows 18..24, preserving its row-21 alignment gap.
        rows = [row("addi r0, r4, 0x3", 0), row("lwz r6, 0x0(r31)", 4),
                row("clrrwi r3, r0, 2", 8), {}, row("addi r5, r3, 0x68", 12),
                row("add r5, r6, r5", 16), row("srwi r4, r0, 2", 20)]
        nodes = {node["row"]: node for node in groups.producer_slice(rows, [5, 6])["nodes"]}
        self.assertEqual(set(nodes), {0, 1, 2, 4, 5, 6})
        self.assertEqual(nodes[2]["uses"], [{"register": "r0", "definition_row": 0}])
        self.assertEqual(nodes[4]["uses"], [{"register": "r3", "definition_row": 2}])
        self.assertEqual(nodes[5]["uses"], [{"register": "r6", "definition_row": 1},
                                          {"register": "r5", "definition_row": 4}])
        self.assertEqual(nodes[6]["uses"], [{"register": "r0", "definition_row": 0}])

    def test_valid_aliases_read_source_and_preserve_other_definitions(self):
        for opcode, immediates in (("clrrwi", "2"), ("clrlwi", "31"),
                                   ("slwi", "0"), ("srwi", "0x1f"),
                                   ("rlwinm", "2, 30, 1"),
                                   ("clrlslwi", "16, 2")):
            for suffix in ("", "."):
                with self.subTest(opcode=opcode + suffix):
                    rows = [row("li r0, 16", 0), row("li r5, 8", 4),
                            row(f"{opcode}{suffix} r3, r0, {immediates}", 8),
                            row("add r4, r3, r5", 12)]
                    nodes = groups.producer_slice(rows, [3])["nodes"]
                    self.assertEqual(nodes[2]["defines"], "r3")
                    self.assertEqual(nodes[2]["uses"], [{"register": "r0", "definition_row": 0}])
                    self.assertEqual(nodes[3]["uses"], [{"register": "r3", "definition_row": 2},
                                                       {"register": "r5", "definition_row": 1}])

    def test_in_place_reads_prior_definition(self):
        nodes = groups.producer_slice([row("li r0, 16", 0),
                                       row("clrrwi. r0, r0, 2", 4)], [1])["nodes"]
        self.assertEqual(nodes[1]["uses"], [{"register": "r0", "definition_row": 0}])
        self.assertEqual(nodes[1]["defines"], "r0")

    def test_malformed_forms_fail_closed(self):
        malformed = ("clrrwi r3, r0", "clrrwi r3, r0, 2, 1", "clrrwi r3, r0,",
                     "clrrwi r3, r0, -1", "clrrwi r3, r0, 32", "clrrwi r3, r0, label",
                     "clrrwi r3, r0, r2", "clrrwi r32, r0, 2", "clrrwi r3, r32, 2",
                     "clrrwi f3, r0, 2", "clrrwi r3, f0, 2", "clrrwi.. r3, r0, 2",
                     "clrrwi . r3, r0, 2", "clrrwi. r3, r0, 32", "rlwinm r3, r0, 2, 0, 32",
                     "clrlslwi r3, r0, 1, 2", "srwi r3, r0, 32",
                     "slwi r3, r0, -1", "clrlwi r3, r0, unknown",
                     "divw r5, r0", "divwu r5, r0, r1, r2", "divw r5, r0, 2",
                     "divwo r5, r0, f2", "divwuo. r32, r0, r2", "divw. r5, r0, r32",
                     "divw.. r5, r0, r2", "divwu . r5, r0, r2", "divw r5, r0, label")
        for instruction in malformed:
            with self.subTest(instruction=instruction):
                nodes = groups.producer_slice([row("li r0, 16", 0), row(instruction, 4),
                                                row("mr r5, r0", 8)], [1, 2])["nodes"]
                self.assertEqual(nodes[0]["status"], "UNKNOWN")
                self.assertEqual(nodes[0]["uses"], [])
                self.assertNotIn("defines", nodes[0])
                self.assertEqual(nodes[1]["uses"][0]["reason"], "unsupported opcode")
                self.assertNotIn("definition_row", nodes[1]["uses"][0])


def bound_report_check(path: Path) -> None:
    """Optional read-only acceptance against the supplied active residual."""
    raw = path.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != "c53b9a106794d508d9d21ea3f5e73332690bc5eaeb1b6e02c83a01e9050c79eb":
        raise ValueError(f"{path}: report SHA-256 differs from acceptance binding: {digest}")
    document = groups.frontier.load_json(raw)
    symbol = groups.frontier._stack_function(
        groups.frontier.focus._symbols(document, "left", "strict"), "qEnQueueOne", "left")
    rows = groups.frontier.focus._rows(symbol, "qEnQueueOne")
    sliced = groups.producer_slice(rows, list(range(18, 25)))
    nodes = {node["row"]: node for node in sliced["nodes"]}
    assert nodes[20]["instruction"].split()[0] == "clrrwi"
    assert nodes[20]["defines"] == "r3"
    assert any(use["register"] == "r0" and "definition_row" in use for use in nodes[20]["uses"])
    print(json.dumps({"report_sha256": digest, "function": "qEnQueueOne", "slice": sliced}))


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--bound-report":
        bound_report_check(Path(sys.argv[2]))
    elif len(sys.argv) in {5, 6} and sys.argv[1] == "--slice-report":
        raw = Path(sys.argv[2]).read_bytes()
        document = groups.frontier.load_json(raw)
        symbol = groups.frontier._stack_function(
            groups.frontier.focus._symbols(document, "left", "strict"), sys.argv[3], "left")
        rows = groups.frontier.focus._rows(symbol, sys.argv[3])
        print(json.dumps({"report_sha256": hashlib.sha256(raw).hexdigest(),
                          "slice": groups.producer_slice(rows, list(map(int, sys.argv[4].split(","))),
                                                         call_model=sys.argv[5] if len(sys.argv) == 6 else "conservative")}))
    else:
        unittest.main()
