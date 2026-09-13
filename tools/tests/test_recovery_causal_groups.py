from __future__ import annotations

import copy
import hashlib
import json
import os
from pathlib import Path
import unittest
import tempfile
import subprocess
import sys

from tools import recovery_causal_groups as groups


def row(text, address, **metadata):
    return {"instruction": {"formatted": text, "address": str(address), "size": 4, **metadata}}


def report(left, right):
    return {side: {"symbols": [{"name": "f", "kind": "SYMBOL_FUNCTION", "size": str(4 * sum(bool(r.get('instruction')) for r in rows)),
                                "instructions": rows}]}
            for side, rows in (("left", left), ("right", right))}


class CausalGroupsTests(unittest.TestCase):
    def test_legacy_target_only_prompt_and_answer_compatibility(self):
        root = Path(__file__).resolve().parents[2]
        folder = root / "build/qwen-gsctx-reconstruction-20260913/gcd-chunks"
        if not (folder / "decision.json").is_file():
            self.skipTest("local original gcd packet unavailable")
        packet = json.loads((folder / "decision.json").read_bytes())
        self.assertNotIn("target_instruction_addresses", packet)
        self.assertEqual(groups.render_decision_prompt(packet), (folder / "prompt.txt").read_text())
        answer_path = folder / "answer/gcd-chunks.answer.txt"
        # While the real answer is still running, exercise its unchanged contract
        # with a deterministic factual finding bound to the actual original packet.
        answer = (json.loads(answer_path.read_bytes()) if answer_path.is_file() else
                  dict(status="supported", function=packet["function"], packet_sha256=packet["packet_sha256"],
                       answer="The supplied target row 0 is " + packet["paired_rows"][0]["target"],
                       evidence_rows=[0], missing_evidence=None))
        self.assertIn(groups.validate_decision_answer(packet, answer)["status"],
                      {"valid_finding", "insufficient_evidence"})
        for field, value in (("target_instruction_addresses", []), ("target_branch_destinations", []),
                             ("row_address", 0)):
            changed = copy.deepcopy(packet)
            if field == "row_address":
                changed["paired_rows"][0]["target_address"] = value
            else:
                changed[field] = value
            changed["packet_sha256"] = groups._digest({k: v for k, v in changed.items() if k != "packet_sha256"})
            with self.assertRaises(ValueError):
                groups.validate_decision_packet(changed)

    def test_target_only_branch_address_map_and_omissions(self):
        rows = [row("beq 0x8", 0), row("b 0x20", 4), row("blr", 8)]
        document = report(rows, [])
        document["right"]["symbols"] = []
        packet = groups.decision_packet(document, "f", "int context;", 1, 1,
                                        "Map branch destinations.", 0, 1, allow_missing_candidate=True)
        self.assertEqual(packet["paired_rows"][1]["target_address"], 4)
        facts = packet["target_branch_destinations"]
        self.assertEqual(facts[0], dict(row=0, destination_address=8, destination_row=None, status="omitted"))
        self.assertEqual(facts[1]["status"], "external")
        self.assertEqual(len(facts), 2)  # Return is not a direct branch destination.
        groups.validate_decision_packet(packet)
        changed = copy.deepcopy(packet)
        changed["target_branch_destinations"][0].update(destination_row=1, status="included")
        changed["packet_sha256"] = groups._digest({k: v for k, v in changed.items() if k != "packet_sha256"})
        with self.assertRaisesRegex(ValueError, "mapping differs"):
            groups.validate_decision_packet(changed)
        changed = copy.deepcopy(packet)
        changed["paired_rows"][0]["target_address"] = 4
        changed["packet_sha256"] = groups._digest({k: v for k, v in changed.items() if k != "packet_sha256"})
        with self.assertRaisesRegex(ValueError, "row/address"):
            groups.validate_decision_packet(changed)
        rows[0]["instruction"]["branch_dest"] = "4"
        with self.assertRaisesRegex(ValueError, "branch text/address"):
            groups.decision_packet(document, "f", "int context;", 1, 1,
                                   "Map branch destinations.", 0, 1, allow_missing_candidate=True)

    def test_actual_action_layout_target_address_replay(self):
        root = Path(__file__).resolve().parents[2]
        old_path = root / "build/qwen-gsctx-reconstruction-20260913/action-layout/decision.json"
        report_path = root / "build/small-first-20260913/gsctx-session-params/strict.json"
        if not old_path.is_file() or not report_path.is_file():
            self.skipTest("local action-layout acceptance artifacts unavailable")
        old = json.loads(old_path.read_bytes())
        document = json.loads(report_path.read_bytes())
        self.assertEqual(groups._digest(document), old["report_sha256"])
        packet = groups.decision_packet(document, old["function"], old["source_excerpt"],
                                        1, len(old["source_excerpt"].splitlines()), old["question"],
                                        0, 77, allow_missing_candidate=True, max_bytes=24000)
        facts = {f["row"]: f for f in packet["target_branch_destinations"]}
        self.assertEqual(facts[18], dict(row=18, destination_address=0x4c0, destination_row=35, status="included"))
        self.assertEqual(facts[25], dict(row=25, destination_address=0x510, destination_row=55, status="included"))
        groups.validate_decision_packet(packet)
        packet["target_branch_destinations"][next(i for i, f in enumerate(packet["target_branch_destinations"]) if f["row"] == 18)]["destination_row"] = 55
        packet["packet_sha256"] = groups._digest({k: v for k, v in packet.items() if k != "packet_sha256"})
        with self.assertRaisesRegex(ValueError, "mapping differs"):
            groups.validate_decision_packet(packet)

    def test_target_only_fact_decision(self):
        rows = [row("li r3, 1", 0), row("bl helper", 4), row("blr", 8)]
        document = report(rows, rows)
        document["right"]["symbols"] = []
        def packet(**options):
            return groups.decision_packet(document, "f", "int helper(int value);", 1, 1,
                                          "Which supplied value reaches helper?", 0, 1, **options)
        with self.assertRaisesRegex(ValueError, "function missing"):
            packet()
        result = packet(allow_missing_candidate=True, producer_sites=[1])
        groups.validate_decision_packet(result)
        self.assertTrue(result["candidate_missing"])
        self.assertEqual(result["source_excerpt_role"], "context")
        self.assertEqual(result["source_excerpt"], "int helper(int value);")
        self.assertEqual([r["target"] for r in result["paired_rows"]], ["li r3, 1", "bl helper"])
        self.assertTrue(all(r["candidate"] is None for r in result["paired_rows"]))
        self.assertEqual(result["machine_census"]["candidate"]["status"], "unavailable")
        self.assertIsNone(result["machine_census"]["candidate"]["instruction_count"])
        self.assertEqual(set(result["producer_context"]), {"target"})
        self.assertIn("TARGET-ONLY factual support", groups.render_decision_prompt(result))
        answer = dict(status="supported", function="f", packet_sha256=result["packet_sha256"],
                      answer="The row 0 constant reaches the row 1 call.", evidence_rows=[0, 1], missing_evidence=None)
        groups.validate_decision_answer(result, answer)
        with self.assertRaises(ValueError):
            groups.validate_decision_answer(result, {**answer, "evidence_rows": [3]})
        with self.assertRaises(ValueError):
            groups.validate_decision_answer(result, {**answer, "status": "hypothesis"})
        with self.assertRaisesRegex(ValueError, "fact mode"):
            packet(allow_missing_candidate=True, decision_mode="source-hypothesis")
        for mutate in (lambda p: p.update(decision_mode="source-hypothesis"),
                       lambda p: p["paired_rows"][0].update(candidate="li r3, 1"),
                       lambda p: p["machine_census"]["candidate"].update(instruction_count=0)):
            changed = copy.deepcopy(result)
            mutate(changed)
            changed["packet_sha256"] = groups._digest({k: v for k, v in changed.items() if k != "packet_sha256"})
            with self.assertRaisesRegex(ValueError, "target-only"):
                groups.validate_decision_packet(changed)
        with self.assertRaisesRegex(ValueError, "byte budget"):
            packet(allow_missing_candidate=True, max_bytes=1000)
        document["left"]["symbols"] = []
        with self.assertRaisesRegex(ValueError, "function missing"):
            packet(allow_missing_candidate=True)

    def test_existing_decisions_ignore_missing_candidate_opt_in(self):
        rows = [row("blr", 0)]
        plain = self.decision(rows, 0, 0)
        opted = self.decision(rows, 0, 0, allow_missing_candidate=True)
        self.assertEqual(plain, opted)
        self.assertEqual(groups.render_decision_prompt(plain), groups.render_decision_prompt(opted))

    def test_score_census_missing_null_and_aliases(self):
        def symbol(name, score):
            return {"name": name, "instructions": [row("blr", 0)], "size": "4", "match_percent": score}
        document = {"left": {"symbols": [symbol("exact", 100), symbol("missing", 100),
                                          symbol("null", None), symbol("alias", None)]},
                    "right": {"symbols": [symbol("exact", 100), symbol("null", 100),
                                           symbol("alias", 100), symbol("extra", 100)]}}
        result = groups.summarize_match_scores(document)
        self.assertEqual(result["exact"], 1)
        self.assertEqual(result["functions"], 4)
        self.assertEqual(result["residuals"]["missing"]["status"], "missing_candidate")
        self.assertIsNone(result["residuals"]["missing"]["candidate_bytes"])
        self.assertEqual(result["residuals"]["null"]["status"], "unscored")
        self.assertIsNone(result["residuals"]["alias"]["score"])
        self.assertEqual(result["candidate_only"]["extra"]["status"], "missing_target")
        document["right"]["symbols"].append(symbol("exact", 100))
        with self.assertRaisesRegex(ValueError, "duplicate"):
            groups.summarize_match_scores(document)

    def duplicate_score_report(self):
        # Actual langdata shape: two target extents share a name; the second
        # has no objdiff pair, despite a differently named candidate at 0xEC.
        def symbol(name, address, **metadata):
            return {"name": name, "address": str(address), "size": "12",
                    "kind": "SYMBOL_FUNCTION", "flags": {"local": True},
                    "instructions": [row("lwz r3, 0x0(r3)", address),
                                     row("lwz r3, 0xcc(r3)", address + 4),
                                     row("blr", address + 8)], **metadata}
        section = {"name": "[.text]", "kind": "SYMBOL_SECTION"}
        first = symbol("_langGetNbrSpeechUnit", 224, target_symbol=1, match_percent=100.0)
        second = symbol("_langGetNbrSpeechUnit", 236)
        return {side: {"sections": [{"name": ".text", "kind": "SECTION_CODE"}],
                       "symbols": copy.deepcopy(symbols)}
                for side, symbols in (("left", [section, first, second]),
                                      ("right", [section, first, symbol("_langGetNbrWarpFactors", 236)]))}

    def test_score_census_separate_duplicate_extents_preserves_unpaired(self):
        document = self.duplicate_score_report()
        original = copy.deepcopy(document)
        result = groups.summarize_match_scores(document)
        self.assertEqual(document, original)
        self.assertEqual((result["functions"], result["exact"]), (2, 1))
        first = result["function_scores"]["_langGetNbrSpeechUnit@left.symbols[1]"]
        second = result["residuals"]["_langGetNbrSpeechUnit@left.symbols[2]"]
        self.assertEqual(first["candidate_symbol_index"], 1)  # Unfiltered index, not function ordinal.
        self.assertEqual(first["pairing"], "reciprocal_report_indices")
        self.assertEqual(second["status"], "unpaired_target")
        self.assertIsNone(second["score"])
        self.assertIsNone(second["candidate_bytes"])
        self.assertEqual(result["candidate_only"]["_langGetNbrWarpFactors"]["status"], "missing_target")
        duplicate = result["duplicate_names"]["left"]["_langGetNbrSpeechUnit"]
        self.assertEqual(duplicate["classification"], "separate_extents")
        self.assertEqual([m["address"] for m in duplicate["members"]], [224, 236])
        self.assertEqual(result["census_unit"], "function_symbols")
        # An unbound score must not borrow the first duplicate's valid pair.
        document["left"]["symbols"][2]["match_percent"] = 100
        changed = groups.summarize_match_scores(document)
        self.assertEqual(changed["exact"], 1)
        self.assertEqual(changed["residuals"]["_langGetNbrSpeechUnit@left.symbols[2]"]["score"], 100)

    def test_score_census_same_extent_aliases_never_transfer_null_score(self):
        document = self.duplicate_score_report()
        for side in ("left", "right"):
            document[side]["symbols"][2] = copy.deepcopy(document[side]["symbols"][1])
            document[side]["symbols"][2]["target_symbol"] = 2
        document["left"]["symbols"][2].pop("match_percent")
        result = groups.summarize_match_scores(document)
        self.assertEqual((result["functions"], result["exact"]), (2, 1))
        self.assertEqual(result["candidate_only"], {})
        self.assertEqual(result["residuals"]["_langGetNbrSpeechUnit@left.symbols[2]"]["status"], "unscored")
        for side in ("left", "right"):
            self.assertEqual(result["duplicate_names"][side]["_langGetNbrSpeechUnit"]["classification"],
                             "same_extent_aliases")
        # Same addresses in an unspecified section do not establish aliases.
        document["left"].pop("sections")
        with self.assertRaisesRegex(ValueError, "ambiguous duplicate"):
            groups.summarize_match_scores(document)

    def test_score_census_duplicate_conflicts_refused(self):
        for pair in (True, -1, 100, 0, 2):
            document = self.duplicate_score_report()
            document["left"]["symbols"][1]["target_symbol"] = pair
            with self.subTest(pair=pair), self.assertRaisesRegex(ValueError, "duplicate function pairing"):
                groups.summarize_match_scores(document)
        document = self.duplicate_score_report()
        document["right"]["symbols"][1]["target_symbol"] = 2
        with self.assertRaisesRegex(ValueError, "duplicate function pairing"):
            groups.summarize_match_scores(document)
        # A reverse-only pair is conflicting evidence, not candidate-only data.
        document = self.duplicate_score_report()
        document["left"]["symbols"][1].pop("target_symbol")
        with self.assertRaisesRegex(ValueError, "duplicate function pairing"):
            groups.summarize_match_scores(document)
        document = self.duplicate_score_report()
        second = document["left"]["symbols"][2]
        second.update(address="224", instructions=[row("blr", 224)])
        with self.assertRaisesRegex(ValueError, "conflicting alias evidence"):
            groups.summarize_match_scores(document)
        # Partially overlapping extents are not aliases or separate functions.
        second["address"] = "228"
        with self.assertRaisesRegex(ValueError, "ambiguous duplicate"):
            groups.summarize_match_scores(document)

    def test_score_census_duplicate_candidate_keeps_unpaired_identity(self):
        document = self.duplicate_score_report()
        document["left"]["symbols"][2]["name"] = "_langGetNbrWarpFactors"
        document["right"]["symbols"][2]["name"] = "_langGetNbrSpeechUnit"
        result = groups.summarize_match_scores(document)
        self.assertEqual(result["exact"], 1)
        self.assertEqual(result["candidate_only"]["_langGetNbrSpeechUnit@right.symbols[2]"]["status"],
                         "missing_target")
        self.assertEqual(result["residuals"]["_langGetNbrWarpFactors"]["status"], "missing_candidate")

    def test_actual_langdata_duplicate_score_replay(self):
        root = Path(__file__).resolve().parents[2]
        folder = root / "build/small-remaining-baseline-20260913/gssdk_lib/asrpho/common/ctxdata/langdata/next-small-census"
        report_path = folder / "base.json"
        if not report_path.is_file():
            self.skipTest("local langdata duplicate census artifact unavailable")
        raw = report_path.read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(),
                         "551b03cf33dbdc38a6b5df8ead509a7081ca97a29918ff525cdedbbc40063f42")
        result = groups.summarize_match_scores(json.loads(raw))
        self.assertEqual((result["functions"], result["exact"], len(result["residuals"])), (76, 66, 10))
        self.assertEqual(result["residuals"]["_langGetNbrSpeechUnit@left.symbols[19]"]["status"], "unpaired_target")
        self.assertEqual(set(result["candidate_only"]), {"_langGetNbrWarpFactors"})
        self.assertEqual(report_path.read_bytes(), raw)

    def test_source_hypothesis_opt_in_and_legacy_compatibility(self):
        rows = [row("li r3, 0", 0), row("blr", 4)]
        plain = self.decision(rows, 0, 1)
        explicit = self.decision(rows, 0, 1, decision_mode="fact")
        self.assertEqual(plain, explicit)
        self.assertNotIn("decision_mode", plain)
        self.assertEqual(groups.render_decision_prompt(plain), groups.render_decision_prompt(explicit))
        packet = self.decision(rows, 0, 1, decision_mode="source-hypothesis")
        answer = dict(status="hypothesis", function="f", packet_sha256=packet["packet_sha256"],
                      answer={"cause": "Reuse an existing zero initialization value.",
                              "prediction": "Row 0 may use the shared producer.",
                              "source_change": {"before": "void f(void) {}", "after": "void f(void) { return; }"}},
                      evidence_rows=[0], missing_evidence="Compiler outcome is untested; source identity unknown.")
        receipt = groups.validate_decision_answer(packet, answer)
        self.assertEqual(receipt["status"], "valid_finding")
        self.assertEqual(receipt["finding_status"], "hypothesis")
        self.assertIs(receipt["review_required"], True)
        self.assertIs(receipt["authority"], False)
        with tempfile.TemporaryDirectory() as temporary:
            packet_path = Path(temporary) / "decision.json"
            answer_path = Path(temporary) / "answer.json"
            packet_path.write_text(json.dumps(packet), encoding="utf-8")
            answer_path.write_text(json.dumps(answer), encoding="utf-8")
            output = subprocess.check_output(
                [sys.executable, str(Path(groups.__file__)), "--decision-packet", str(packet_path),
                 "--check-support-answer", str(answer_path)], text=True)
            self.assertEqual(json.loads(output), receipt)
        self.assertIn("at most ONE natural-C", groups.render_decision_prompt(packet))
        legacy_answer = {**answer, "packet_sha256": plain["packet_sha256"]}
        with self.assertRaises(ValueError):
            groups.validate_decision_answer(plain, legacy_answer)
        for status, missing in (("supported", None), ("insufficient", "Need a definition")):
            groups.validate_decision_answer(packet, {**answer, "status": status,
                                                     "answer": "A factual finding.", "missing_evidence": missing})
        for changes in ({"evidence_rows": []}, {"evidence_rows": [90]}, {"evidence_rows": [0, 0]},
                        {"missing_evidence": None}, {"answer": "Unstructured speculation"},
                        {"answer": {"cause": "x", "prediction": ""}},
                        {"packet_sha256": plain["packet_sha256"]}):
            with self.assertRaises(ValueError):
                groups.validate_decision_answer(packet, {**answer, **changes})
        for change in ({"before": "absent", "after": "new"},
                       {"before": "void", "after": "new"},
                       {"before": "void f(void) {}", "after": "void f(void) {}"},
                       {"before": "void f(void) {}", "after": " "},
                       {"before": "", "after": "new"}):
            with self.assertRaisesRegex(ValueError, "unique bound"):
                groups.validate_decision_answer(packet, {**answer, "answer": {
                    **answer["answer"], "source_change": change}})
        tampered = {**plain, "decision_mode": "source-hypothesis"}
        with self.assertRaises(ValueError):
            groups.validate_decision_packet(tampered)
        tampered = {**packet, "decision_mode": "anything"}
        tampered["packet_sha256"] = groups._digest({k: v for k, v in tampered.items() if k != "packet_sha256"})
        with self.assertRaises(ValueError):
            groups.validate_decision_packet(tampered)
        with self.assertRaises(ValueError):
            self.decision(rows, 0, 1, decision_mode="anything")

    def test_hypothesis_guidance_distinguishes_unproved_from_forbidden(self):
        rows = [row("lwz r3, 0(r3)", 0), row("blr", 4)]
        packet = self.decision(rows, 0, 1, decision_mode="source-hypothesis")
        before = copy.deepcopy(packet)
        prompt = groups.render_decision_prompt(packet)
        for guidance in ("absence of proved causality, not an admission requirement",
                         "source-to-compiler ownership proof is not required",
                         "Real used typed locals or aggregate snapshots",
                         "coupled source boundaries", "actual supplied inputs, types and consumers",
                         "fake/dead locals or numeric register shaping",
                         "untested predictions, not guaranteed gains",
                         "If no grounded cause is available, return insufficient"):
            self.assertIn(guidance, prompt)
        self.assertEqual(packet, before)
        self.assertIs(packet["source_causality_proven"], False)
        self.assertIs(packet["authority_advanced"], False)
        fact = self.decision(rows, 0, 1)
        self.assertNotIn("Real used typed locals", groups.render_decision_prompt(fact))
        document = report(rows, [])
        document["right"]["symbols"] = []
        target_only = groups.decision_packet(document, "f", "int context;", 1, 1,
                                            "Describe loads.", 0, 1, allow_missing_candidate=True)
        self.assertNotIn("Real used typed locals", groups.render_decision_prompt(target_only))

    def test_uncertain_used_local_hypothesis_and_insufficient_keep_no_authority(self):
        rows = [row("lwz r4, 0(r3)", 0), row("lwz r3, 4(r3)", 4),
                row("add r3, r4, r3", 8), row("blr", 12)]
        source = "typedef struct Pair { unsigned a, b; } Pair;\nunsigned f(Pair *p) { return p->a + p->b; }"
        packet = groups.decision_packet(report(rows, rows), "f", source, 1, 2,
                                        "Can a real input snapshot affect load scheduling?", 0, 3,
                                        decision_mode="source-hypothesis")
        original = copy.deepcopy(packet)
        answer = dict(status="hypothesis", function="f", packet_sha256=packet["packet_sha256"],
                      answer={"cause": "A typed snapshot of the supplied pair feeds both sum operands.",
                              "prediction": "Rows 0 and 1 may schedule the two input loads together.",
                              "source_change": {"before": "return p->a + p->b;",
                                                "after": "Pair values = *p; return values.a + values.b;"}},
                      evidence_rows=[0, 1],
                      missing_evidence="No source-to-compiler ownership proof; compiler outcome untested.")
        receipt = groups.validate_decision_answer(packet, answer)
        self.assertEqual(receipt["finding_status"], "hypothesis")
        self.assertIs(receipt["authority"], False)
        self.assertIs(receipt["review_required"], True)
        insufficient = {**answer, "status": "insufficient", "answer": "No grounded cause selected.",
                        "evidence_rows": [], "missing_evidence": "Caller alias constraints are unavailable."}
        receipt = groups.validate_decision_answer(packet, insufficient)
        self.assertEqual(receipt["status"], "insufficient_evidence")
        self.assertIs(receipt.get("authority", False), False)
        self.assertEqual(packet, original)
        self.assertIs(packet["source_causality_proven"], False)
        self.assertIs(packet["authority_advanced"], False)

    def test_saved_codebook_insufficient_remains_valid_read_only(self):
        folder = Path(__file__).resolve().parents[2] / "build/qwen-lang-cause-next-20260913/codebook-flags-reload"
        paths = [folder / "decision.json", folder / "answer/codebook-flags-reload.answer.txt"]
        if not all(path.is_file() for path in paths):
            self.skipTest("local codebook support artifacts unavailable")
        raw = [path.read_bytes() for path in paths]
        packet, answer = map(json.loads, raw)
        self.assertEqual(packet["packet_sha256"],
                         "c7575d03735a87b7e19d23a1452f4fd388c0734db65d7f8d22443da9dd92d228")
        self.assertEqual(answer["status"], "insufficient")
        self.assertIn("source_causality_proven is false", answer["missing_evidence"])
        receipt = groups.validate_decision_answer(packet, answer)
        self.assertEqual(receipt["status"], "insufficient_evidence")
        self.assertIs(receipt.get("authority", False), False)
        self.assertIs(packet["authority_advanced"], False)
        self.assertIn("not an admission requirement", groups.render_decision_prompt(packet))
        self.assertEqual([path.read_bytes() for path in paths], raw)

    def test_existing_mel_fact_packet_replay(self):
        folder = Path(__file__).resolve().parents[2] / "build/qwen-mel-decisions-20260913/process-cursor"
        if not folder.is_dir():
            self.skipTest("local bounded replay artifacts unavailable")
        packet = json.loads((folder / "decision.json").read_text())
        prompt = groups.render_decision_prompt(packet)
        self.assertEqual(hashlib.sha256(prompt.encode()).hexdigest(),
                         "85783f9a38c219d7738bc63a042148c9679d72b87aa465e625a1739e7718ddf2")
        answer = json.loads((folder / "answer/process-cursor.answer.txt").read_text())
        self.assertEqual(groups.validate_decision_answer(packet, answer)["status"], "valid_finding")

    def decision(self, rows, start, end, **kwargs):
        return groups.decision_packet(report(rows, rows), "f", "void f(void) {}", 1, 1,
                                      "Where is this value defined?", start, end, **kwargs)

    def test_decision_optional_producers_and_citations(self):
        rows = [row("lwz r3, 12(r29)", 0), row("addi r4, r3, 4", 4)]
        plain = self.decision(rows, 1, 1)
        self.assertNotIn("producer_context", plain)
        self.assertNotIn("Optional producer_context", groups.render_decision_prompt(plain))
        packet = self.decision(rows, 1, 1, producer_sites=[1])
        sliced = packet["producer_context"]["target"]
        self.assertEqual([n["row"] for n in sliced["nodes"]], [0, 1])
        self.assertEqual(sliced["nodes"][1]["uses"], [{"register": "r3", "definition_row": 0}])
        self.assertEqual(sliced["nodes"][0]["uses"][0]["status"], "UNKNOWN")
        self.assertIn("Optional producer_context", groups.render_decision_prompt(packet))
        answer = dict(status="supported", function="f", packet_sha256=packet["packet_sha256"],
                      answer="The add uses the row 0 load.", evidence_rows=[0], missing_evidence=None)
        groups.validate_decision_answer(packet, answer)
        answer["packet_sha256"] = plain["packet_sha256"]
        with self.assertRaisesRegex(ValueError, "missing/duplicate"):
            groups.validate_decision_answer(plain, answer)

    def test_decision_producers_stop_at_boundaries_and_unknowns(self):
        for boundary, reason in (("bl helper", "call boundary"), ("b 0x8", "CFG/function entry"),
                                 ("mystery r3", "unsupported opcode")):
            packet = self.decision([row("li r3, 1", 0), row(boundary, 4), row("mr r4, r3", 8)],
                                   2, 2, producer_sites=[2])
            groups.validate_decision_packet(packet)
            nodes = packet["producer_context"]["target"]["nodes"]
            self.assertEqual([n["row"] for n in nodes], [2])
            self.assertEqual(nodes[0]["uses"][0]["reason"], reason)

    def test_decision_producer_budgets_and_validation(self):
        rows = [row("addi r3, r3, 1", i*4) for i in range(10)]
        packet = self.decision(rows, 9, 9, producer_sites=[9], producer_limit=1)
        sliced = packet["producer_context"]["target"]
        self.assertTrue(sliced["truncated"])
        self.assertEqual(len(sliced["nodes"]), 4)
        groups.validate_decision_packet(packet)
        answer = dict(status="supported", function="f", packet_sha256=packet["packet_sha256"],
                      answer="Missing producer is not citation evidence.", evidence_rows=[5], missing_evidence=None)
        with self.assertRaisesRegex(ValueError, "missing/duplicate"):
            groups.validate_decision_answer(packet, answer)
        plain = self.decision(rows, 9, 9)
        budget = len(json.dumps(plain, ensure_ascii=False).encode()) + 50
        self.decision(rows, 9, 9, max_bytes=max(1000, budget))
        with self.assertRaisesRegex(ValueError, "byte budget"):
            self.decision(rows, 9, 9, producer_sites=[9], max_bytes=max(1000, budget))
        for sites, limit in (([], 16), ([8], 16), ([9, 9], 16), ([True], 16), ([9], 65)):
            with self.assertRaises(ValueError):
                self.decision(rows, 9, 9, producer_sites=sites, producer_limit=limit)
        sliced["node_limit"] = 1000
        packet["packet_sha256"] = groups._digest({k: v for k, v in packet.items() if k != "packet_sha256"})
        with self.assertRaisesRegex(ValueError, "bounds/scope"):
            groups.validate_decision_packet(packet)

    def test_constant_copy_recognizes_a_reaching_value_not_source_identity(self):
        for op in ("addi r4, r5, 0x0", "mr r4, r5"):
            left = [row("li r5, 0", 0), row(op, 4), row("stw r4, 0(r3)", 8)]
            right = [row("li r5, 0", 0), row("li r4, 0", 4), row("stw r4, 0(r3)", 8)]
            result = groups.constant_copy_evidence(left, right)
            self.assertEqual(len(result["signals"]), 1)
            self.assertEqual(result["signals"][0]["producer_row"], 0)
            self.assertEqual(result["signals"][0]["row"], 1)
            self.assertFalse(result["signals"][0]["cause_proven"])
            self.assertFalse(result["authority_advanced"])

    def test_constant_copy_rejects_unknown_redefined_or_different_values(self):
        for middle in ("bl f", "b 0x8", "lwz r5, 0(r3)", "addi r5, r5, 1", "li r5, 1"):
            left = [row("li r5, 0", 0), row(middle, 4), row("addi r4, r5, 0", 8)]
            right = copy.deepcopy(left)
            right[2] = row("li r4, 0", 8)
            self.assertEqual(groups.constant_copy_evidence(left, right)["signals"], [], middle)
        self.assertEqual(groups.constant_copy_evidence([row("mr r4, r5", 0)], [row("li r4, 0", 0)])["signals"], [])

    def test_constant_copy_respects_powerpc_zero_base_and_actual_copy(self):
        for target in ("addi r4, r0, 0", "addi r4, r5, 1", "addi r5, r5, 0", "mr r6, r5"):
            left = [row("li r5, 0", 0), row("li r0, 0", 4), row(target, 8)]
            right = copy.deepcopy(left)
            right[2] = row("li r4, 0", 8)
            self.assertEqual(groups.constant_copy_evidence(left, right)["signals"], [])

    def test_constant_copy_bounded_negative_values_and_owner_routing(self):
        left = [row("li r5, -0x1", 0)] + [row("mr r4, r5", i*4) for i in range(1, 21)]
        right = [row("li r5, -0x1", 0)] + [row("li r4, -1", i*4) for i in range(1, 21)]
        result = groups.constant_copy_evidence(left, right)
        self.assertEqual(len(result["signals"]), 16)
        self.assertTrue(result["sites_truncated"])
        self.assertTrue(all(s["value"] == -1 for s in result["signals"]))
        owner = groups.summarize_owner(report(left[:2], right[:2]))
        self.assertEqual(len(owner["residuals"][0]["constant_copy_signals"]), 1)
        self.assertFalse(owner["owner_closed"])

    def test_numeric_domains_fft_unsigned_and_single(self):
        left = [row("cmplwi r3, 2", 0), row("srwi r4, r3, 1", 4),
                row("fmuls f0, f1, f2", 8), row("lfs f2, 0(r2)", 12)]
        right = [row("cmpwi r3, 2", 0), row("srawi r4, r3, 1", 4),
                 row("fmul f0, f1, f2", 8), row("lfd f2, 0(r2)", 12)]
        evidence = groups.summarize_groups(report(left, right), "f")["numeric_domain_evidence"]
        signals = {s["family"]: s for s in evidence["signals"]}
        self.assertEqual(signals["integer"]["target_domain"], "unsigned")
        self.assertEqual(signals["integer"]["row_count"], 2)
        self.assertEqual(signals["arithmetic"]["target_domain"], "single")
        self.assertEqual(signals["load"]["review_priority"], "support_only")
        self.assertEqual(evidence["source_type_identity"], "UNKNOWN")

    def test_domain_reverse_and_mixed_directions(self):
        left = [row("cmpw r3, r4", 0), row("cmplw r3, r4", 4)]
        right = list(reversed(left))
        signals = groups.numeric_domain_evidence(left, right)["signals"]
        self.assertEqual({s["target_domain"] for s in signals}, {"signed", "unsigned"})
        self.assertTrue(all(s["mixed_direction"] and s["review_priority"] == "support_only" for s in signals))
        double = groups.numeric_domain_evidence([row("fdiv f0, f1, f2", 0)], [row("fdivs f0, f1, f2", 0)])
        self.assertEqual(double["signals"][0]["target_domain"], "double")

    def test_domain_opcode_families_and_malformed_instructions(self):
        for single, double in (("fmuls", "fmul"), ("fadds", "fadd"), ("fsubs", "fsub"), ("fdivs", "fdiv")):
            with self.subTest(op=single):
                result = groups.numeric_domain_evidence([row(single+" f0, f1, f2", 0)],
                                                       [row(double+" f0, f1, f2", 0)])
                self.assertEqual(result["signals"][0]["target_domain"], "single")
        for malformed in (None, {"instruction": None}, {"instruction": {"formatted": 3}},
                          {"instruction": "bad"}, row("cmplwi", 0)):
            self.assertEqual(groups.numeric_domain_evidence([malformed], [row("cmpwi r3, 1", 0)])["signals"], [])

    def test_domain_missing_unchanged_inserted_and_bounded(self):
        left = [row("cmplwi r3, 1", 0), {}, row("cmpwi r3, 1", 8),
                {**row("cmplwi r3, 1", 12), "diff_kind": "DIFF_DELETE"}]
        right = [{}, row("cmpwi r3, 1", 4), row("cmpwi r4, 1", 8), row("cmpwi r3, 1", 12)]
        self.assertEqual(groups.numeric_domain_evidence(left, right)["signals"], [])
        many = groups.numeric_domain_evidence([row("cmplwi r3, 1", i*4) for i in range(20)],
                                             [row("cmpwi r3, 1", i*4) for i in range(20)])["signals"][0]
        self.assertEqual(many["row_count"], 20)
        self.assertEqual(len(many["sites"]), 12)
        self.assertTrue(many["sites_truncated"])

    def test_owner_counts_and_no_physical_closure_claim(self):
        doc = report([row("cmplwi r3, 1", 0)], [row("cmpwi r3, 1", 0)])
        for side in ("left", "right"):
            exact = copy.deepcopy(doc[side]["symbols"][0])
            exact.update(name="already_exact", instructions=[row("blr", 0)])
            doc[side]["symbols"].append(exact)
        result = groups.summarize_owner(doc)
        self.assertEqual((result["instruction_exact_count"], result["remaining_count"]), (1, 1))
        self.assertTrue(result["residuals"][0]["domain_review_first"])
        self.assertFalse(result["owner_closed"])
        self.assertFalse(result["physical_proof"])

    def test_owner_missing_candidate_is_a_residual_not_a_crash_or_match(self):
        doc = report([row("blr", 0)], [row("blr", 0)])
        doc["right"]["symbols"] = []
        result = groups.summarize_owner(doc)
        self.assertEqual((result["instruction_exact_count"], result["remaining_count"]), (0, 1))
        self.assertEqual(result["residuals"][0]["status"], "candidate_symbol_missing")
        self.assertIsNone(result["residuals"][0]["score"])
        self.assertFalse(result["owner_closed"])

    def test_support_excerpt_is_bound_and_does_not_dump_full_stream(self):
        left = [row("cmplwi r3, 1", 0)] + [row("blr", i*4) for i in range(1, 100)]
        right = [row("cmpwi r3, 1", 0)] + copy.deepcopy(left[1:])
        right[90] = row("li r3, 0", 360)
        source = "float unrelated;\r\nvoid f(void) { }\r\nfloat other;\r\n"
        result = groups.support_packet(report(left, right), "f", source, 2, 2)
        self.assertEqual(result["source_excerpt"], "void f(void) { }")
        self.assertEqual(result["source_sha256"], hashlib.sha256(source.encode()).hexdigest())
        self.assertEqual(result["omitted_residual_row_count"], 1)
        self.assertLess(len(result["paired_rows"]), 10)
        self.assertNotIn("unrelated", json.dumps(result))
        with self.assertRaisesRegex(ValueError, "narrow"):
            groups.support_packet(report(left, right), "f", "a"*10000, 1, 1, max_bytes=1000)
        with self.assertRaises(ValueError):
            groups.support_packet(report(left, right), "f", source, 0, 1)

    def test_support_retains_late_extra_load_behind_early_register_cycle(self):
        left = [row("mr r4, r3", 0)] + [row("nop", i*4) for i in range(1, 120)]
        right = copy.deepcopy(left)
        right[0] = row("mr r5, r3", 0)
        left[90] = {}
        right[90] = row("lwz r4, 0x2c(r31)", 360)
        result = groups.support_packet(report(left, right), "f", "void f(void) {}", 1, 1)
        self.assertEqual(result["structural_context_anchors"], {"added_instruction": 90})
        self.assertTrue({0, 89, 90, 91}.issubset({r["row"] for r in result["paired_rows"]}))
        self.assertEqual(result["omitted_residual_row_count"], 0)
        self.assertFalse(result["size_exact"])
        self.assertLess(len(result["paired_rows"]), 20)

    def test_producer_field_order_and_downstream_use(self):
        left = [row("lwz r4, 0x28(r30)", 0), row("lwz r3, 0x2c(r30)", 4), row("add r6, r5, r3", 8)]
        right = [row("lwz r4, 0x2c(r30)", 0), row("lwz r3, 0x28(r30)", 4), row("add r4, r5, r4", 8)]
        result = groups.summarize_groups(report(left, right), "f", producers=8)["producer_slice"]
        self.assertEqual(result["target"]["nodes"][0]["memory_root"]["offset"], 40)
        self.assertEqual(result["candidate"]["nodes"][0]["memory_root"]["offset"], 44)
        self.assertEqual(result["target"]["nodes"][2]["uses"][1]["definition_row"], 1)
        self.assertEqual(result["candidate"]["nodes"][2]["uses"][1]["definition_row"], 0)

    def test_producer_boundaries(self):
        for text, reason in (("bl helper", "call boundary"), ("b 0x8", "CFG/function entry"), ("mystery r8", "unsupported opcode")):
            result = groups.producer_slice([row("li r3, 1", 0), row(text, 4), row("mr r4, r3", 8)], [2])
            self.assertEqual(result["nodes"][-1]["uses"][0]["reason"], reason)
            self.assertNotIn("definition_row", result["nodes"][-1]["uses"][0])

    def test_producer_subi_and_update_load_pre_update_base(self):
        for op, dest in (("lwzu", "r4"), ("lfsu", "f4"), ("lfdu", "f4")):
            rows = [row("li r3, 16", 0), row("subi r3, r3, 4", 4),
                    row(f"{op} {dest}, -0x4(r3)", 8), row("addi r5, r3, 8", 12),
                    row(f"{'mr' if op == 'lwzu' else 'fmr'} { 'r6' if op == 'lwzu' else 'f6'}, {dest}", 16)]
            nodes = groups.producer_slice(rows, [3, 4])["nodes"]
            self.assertEqual(nodes[1]["uses"], [{"register": "r3", "definition_row": 0}])
            self.assertEqual(nodes[2]["uses"], [{"register": "r3", "definition_row": 1}])
            self.assertEqual(nodes[2]["defines"], dest)
            self.assertEqual(nodes[2]["base_update"], {"register": "r3", "operation": "old_base_plus_offset", "offset": -4})
            self.assertEqual(nodes[2]["memory_root"]["alias_identity"], "UNKNOWN")
            self.assertEqual(nodes[3]["uses"], [{"register": "r3", "definition_row": 2}])
            self.assertEqual(nodes[4]["uses"], [{"register": dest, "definition_row": 2}])
            groups.validate_decision_packet(self.decision(rows, 3, 4, producer_sites=[3, 4]))
        self.assertEqual(groups.producer_slice([row("subi r3, r0, 4", 0)], [0])["nodes"][0]["uses"], [])

    def test_producer_update_rejects_illegal_or_unparsed_forms(self):
        for instruction in ("lwzu r3, 4(r3)", "lwzu r4, 4(r0)", "lfsu f4, 4(r0)",
                            "lfdu f4, label(r3)", "lwzu f4, 4(r3)", "lfsu r4, 4(r3)",
                            "lwzu r4, 65536(r3)", "lwzu r4, 4(r32)"):
            nodes = groups.producer_slice([row("li r3, 1", 0), row(instruction, 4),
                                           row("mr r5, r3", 8)], [1, 2])["nodes"]
            self.assertEqual(nodes[0]["status"], "UNKNOWN")
            self.assertNotIn("base_update", nodes[0])
            self.assertEqual(nodes[1]["uses"][0]["reason"], "unsupported opcode")

    def test_producer_update_does_not_cross_calls_or_cfg(self):
        for boundary in ("bl helper", "b 0xc"):
            rows = [row("li r3, 1", 0), row(boundary, 4), row("lfsu f0, 4(r3)", 8),
                    row("addi r4, r3, 4", 12)]
            nodes = groups.producer_slice(rows, [2, 3])["nodes"]
            self.assertEqual(nodes[0]["uses"][0]["status"], "UNKNOWN")
            if boundary.startswith("bl "):
                self.assertEqual(nodes[1]["uses"][0]["definition_row"], 2)
            else:
                self.assertEqual(nodes[1]["uses"][0]["status"], "UNKNOWN")

    def test_producer_branch_entry_alias_and_symbolic_memory(self):
        rows = [row("li r3, 1", 0), row("b 0xc", 4, branch_dest="12"), row("li r3, 2", 8),
                row("mr r4, r3", 12), row("lfs f0, label@sda21(r2)", 12)]
        result = groups.producer_slice(rows, [3, 4])
        self.assertEqual(result["nodes"][0]["uses"][0]["status"], "UNKNOWN")
        self.assertEqual(result["nodes"][1]["memory_root"]["status"], "UNKNOWN")

    def test_producer_bounds_and_optional_default(self):
        rows = [row("addi r3, r3, 1", i * 4) for i in range(100)]
        result = groups.producer_slice(rows, [99, 98], limit=1)
        self.assertEqual(len(result["nodes"]), 4)
        self.assertTrue(result["truncated"])
        self.assertNotIn("producer_slice", groups.summarize_groups(report(rows, rows), "f"))
        with self.assertRaises(ValueError):
            groups.producer_slice(rows, [1], limit=65)

    def test_producer_label_and_zero_base(self):
        rows = [row("li r3, 1", 0), {"label": "alias"}, row("mr r4, r3", 4),
                row("lwz r5, 0x28(r0)", 8), row("addi r6, r0, 4", 12)]
        nodes = groups.producer_slice(rows, [2, 3, 4])["nodes"]
        self.assertEqual(nodes[0]["uses"][0]["reason"], "unsupported/label boundary")
        self.assertEqual(nodes[1]["uses"], [])
        self.assertTrue(nodes[1]["memory_root"]["zero_base"])
        self.assertEqual(nodes[2]["uses"], [])

    def test_rotate_mask_aliases_preserve_unrelated_definitions(self):
        for alias in ("clrlwi r0, r3, 16", "clrlslwi r0, r3, 16, 2"):
            rows = [row("li r5, 1", 0), row("li r6, 2", 4), row("li r3, 3", 8),
                    row(alias, 12), row("add r7, r5, r6", 16)]
            nodes = groups.producer_slice(rows, [3, 4])["nodes"]
            self.assertEqual(nodes[3]["defines"], "r0")
            self.assertEqual(nodes[3]["uses"], [{"register": "r3", "definition_row": 2}])
            self.assertEqual(nodes[4]["uses"], [{"register": "r5", "definition_row": 0},
                                                {"register": "r6", "definition_row": 1}])

    def test_identical(self):
        rows = [row("li r3, 0", 0), row("blr", 4)]
        result = groups.summarize_groups(report(rows, copy.deepcopy(rows)), "f")
        self.assertTrue(result["instruction_exact"])
        self.assertEqual(result["groups"], [])
        self.assertIsNone(result["first_machine_divergence"])
        self.assertFalse(result["physical_proof"])

    def test_global_permutation(self):
        result = groups.summarize_groups(report([row("add r3, r4, r3", 0)], [row("add r4, r3, r4", 0)]), "f")
        self.assertEqual(result["register_mapping"]["status"], "confirmed")
        self.assertEqual(result["category_rows"], {"register_permutation": 1})

    def test_conflict_stays_unknown(self):
        result = groups.summarize_groups(report([row("mr r3, r4", 0), row("mr r3, r4", 4)],
                                                [row("mr r4, r3", 0), row("mr r5, r3", 4)]), "f")
        self.assertEqual(result["register_mapping"]["mapping"], {})
        self.assertEqual(result["unresolved_row_count"], 2)
        self.assertTrue(all(not g["cause_proven"] for g in result["groups"]))

    def test_added_deleted_moves(self):
        result = groups.summarize_groups(report([row("mr r3, r4", 0), {}], [{}, row("mr r4, r3", 0)]), "f")
        self.assertEqual(result["category_rows"], {"added_move": 1, "deleted_move": 1})
        self.assertEqual(result["coverage"]["grouped_rows"], 2)

    def test_cfg_and_opcode(self):
        left = [row("b 0x4", 0, branch_dest="4"), row("li r3, 1", 4), row("li r3, 2", 8)]
        right = [row("b 0x8", 0, branch_dest="8"), row("li r3, 1", 4), row("addi r3, r3, 2", 8)]
        result = groups.summarize_groups(report(left, right), "f")
        self.assertEqual(result["category_rows"], {"cfg_changed": 1, "opcode": 1})
        self.assertEqual(result["structural_hazard_count"], 2)

    def test_relocation_addend(self):
        left = [row("lis r3, f@ha", 0, relocation={"type_name": "R_PPC_ADDR16_HA", "target_symbol": 0, "addend": 0})]
        right = copy.deepcopy(left)
        right[0]["instruction"]["relocation"]["addend"] = 4
        result = groups.summarize_groups(report(left, right), "f")
        self.assertEqual(result["category_rows"], {"relocation": 1})
        self.assertFalse(result["instruction_exact"])

    def test_ranking_protects_exact_before_score(self):
        result = groups.summarize_groups(report([row("blr", 0)], [row("blr", 0)]), "f")
        self.assertLess(groups.ranking_tuple(result, score=0), groups.ranking_tuple(result, protected_loss_count=1, score=100))
        self.assertLess(groups.ranking_tuple(result, exact_function_count=2, score=0), groups.ranking_tuple(result, exact_function_count=1, score=100))

    def summarize_pair(self, candidate):
        target = [row("mr r3, r4", 0), row("mr r3, r4", 4)]
        return groups.summarize_groups(report(target, candidate), "f")

    def test_changed_register_relation_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r0, r4", 0), row("mr r0, r4", 4)])
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])

    def test_changed_category_is_reclassified_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("addi r3, r4, 1", 0), row("addi r3, r4, 1", 4)])
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])
        self.assertTrue(delta["reclassified_groups"])

    def test_split_relation_bucket_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r0, r4", 0), row("mr r6, r4", 4)])
        self.assertGreater(len(after["groups"]), len(before["groups"]))
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["closed_groups"], [])
        self.assertTrue(delta["reclassified_groups"])

    def test_partially_resolved_group_is_not_closed(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r6, r4", 4)])
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [])

    def test_all_prior_sites_resolved_closes_group(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [before["groups"][0]["id"]])

    def test_insertion_alignment_shift_does_not_close_target_sites(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = groups.summarize_groups(report(
            [{}, row("mr r3, r4", 0), row("mr r3, r4", 4)],
            [row("nop", 0), row("mr r6, r4", 4), row("mr r6, r4", 8)]), "f")
        self.assertEqual(groups.compare_groups(before, after)["closed_groups"], [])
        self.assertEqual(before["target_binding"], after["target_binding"])
        self.assertEqual({m["target_index"] for g in after["groups"]
                          for m in g["members"] if m["anchor"] == "target"}, {0, 1})

    def test_different_target_stream_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        target = [row("mr r7, r4", 0), row("mr r7, r4", 4)]
        after = groups.summarize_groups(report(target, copy.deepcopy(target)), "f")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_legacy_missing_members_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        before["groups"][0].pop("members")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_legacy_missing_target_binding_returns_unknown(self):
        before = self.summarize_pair([row("mr r5, r4", 0), row("mr r5, r4", 4)])
        after = self.summarize_pair([row("mr r3, r4", 0), row("mr r3, r4", 4)])
        before.pop("target_binding")
        delta = groups.compare_groups(before, after)
        self.assertEqual(delta["status"], "unknown")
        self.assertEqual(delta["closed_groups"], [])

    def test_actual_fixture_when_explicitly_selected(self):
        # Retail-derived local reports are never read by default public checks.
        directory = os.environ.get("MP6_CAUSAL_GROUP_FIXTURE_ROOT")
        if not directory:
            self.skipTest("explicit local fixture directory not selected")
        root = Path(directory)
        raw = (root / "kettou-guide-field-reconstruction/.evaluate-jpuewz_e/strict.json").read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(), "5de00fed59959dbbea921bb07b67922231a81b969e2e177e2db1773bd8d258c3")
        after = groups.summarize_groups(json.loads(raw), "ev_CapKettouStart")
        before = groups.summarize_groups(json.loads((root / "evaluations/.evaluate-uzle7on4/strict.json").read_bytes()), "ev_CapKettouStart")
        self.assertEqual(before["coverage"]["residual_rows"], 314)
        self.assertEqual(after["coverage"]["residual_rows"], 291)
        self.assertEqual(after["category_rows"], {"added_move": 13, "deleted_move": 13, "register_relation_unknown": 263, "abi_helper_call": 2})
        delta = groups.compare_groups(before, after)
        self.assertEqual(len(delta["disappeared_observation_buckets"]), 5)
        self.assertEqual(len(delta["closed_groups"]), 2)
        self.assertEqual(len(delta["reclassified_groups"]), 3)
        closed = [g for g in before["groups"] if g["id"] in delta["closed_groups"]]
        self.assertTrue(all("r24" in str(g["relation"]) for g in closed))
        residual_sites = {member["target_index"] for group in after["groups"] for member in group["members"]}
        self.assertEqual(sum(group["row_count"] for group in closed), 23)
        self.assertTrue(all(not {member["target_index"] for member in group["members"]} & residual_sites
                            for group in closed))
        reclassified = [group for group in before["groups"] if group["id"] in delta["reclassified_groups"]]
        self.assertTrue(all({member["target_index"] for member in group["members"]} & residual_sites
                            for group in reclassified))
        self.assertEqual(delta["new_groups"], [])
        self.assertEqual(after["register_mapping"]["mapping"], {})


if __name__ == "__main__":
    unittest.main()
