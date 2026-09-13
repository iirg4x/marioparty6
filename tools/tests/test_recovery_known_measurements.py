"""Selected historical source constraints are sealed observations, not proof reuse."""
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import report, row


DIALECT_GUIDANCE = (
    "Honor the supplied language dialect. When C99/mixed-declaration support is not "
    "established, put declarations at block entry; a natural nested block may keep a "
    "legitimate used snapshot local to its use. Do not change language flags to make "
    "a hypothesis compile. "
)


class KnownMeasurementsTests(unittest.TestCase):
    def test_source_hypothesis_dialect_guidance_preserves_local_snapshots(self):
        packet = self.packet(decision_mode="source-hypothesis")
        before = copy.deepcopy(packet)
        prompt = groups.render_decision_prompt(packet)
        self.assertEqual(prompt.count(DIALECT_GUIDANCE), 1)
        self.assertIn("Real used typed locals or aggregate snapshots", prompt)
        self.assertEqual(packet, before)
        self.assertNotIn(DIALECT_GUIDANCE, groups.render_decision_prompt(self.packet()))

    def test_actual_enqueue_one_packet_dialect_render_is_read_only(self):
        path = (Path(__file__).resolve().parents[2]
                / "build/qwen-mqueue-enqueueone-known-20260913/queue-single-scheduling/decision.json")
        if not path.is_file():
            self.skipTest("local original enqueue-one packet unavailable")
        original = path.read_bytes()
        packet = json.loads(original)
        before = copy.deepcopy(packet)
        self.assertEqual(packet["function"], "qEnQueueOne")
        prompt = groups.render_decision_prompt(packet)
        self.assertIn(DIALECT_GUIDANCE, prompt)
        self.assertEqual(packet, before)
        self.assertEqual(path.read_bytes(), original)
        self.assertTrue(prompt.endswith(json.dumps(packet, ensure_ascii=False, separators=(",", ":"))))

    def test_comparison_direction_roles_and_alignment_caveat(self):
        left = [row("stw r0, 0xc(r1)", 0), {}, row("lwz r3,0(r1)", 4), row("blr", 8)]
        right = [{}, row("mr r30,r3", 0), row("lwz r4,0(r1)", 4), row("blr", 8)]
        packet = groups.decision_packet(report(left, right), "f", "void f(void) { use(data); }",
                                        1, 1, "Which return boundary differs?", 0, 3,
                                        decision_mode="source-hypothesis")
        facts = packet["comparison_direction"]
        self.assertEqual([r["observation"] for r in facts["rows"]],
                         ["missing_from_candidate", "extra_in_candidate", "paired_difference"])
        self.assertEqual(facts["different_alignment_count"], 3)
        self.assertEqual(facts["target_role"], "immutable_reference")
        prompt = groups.render_decision_prompt(packet)
        for text in ("only candidate C may change", "Do not optimize target to resemble candidate",
                     "not remove the target instruction", "Alignment can pair different operations",
                     "no row alone proves a semantic addition/deletion"):
            self.assertIn(text, prompt)
        for mutate in (lambda d: d.update(target_role="editable_C", candidate_role="immutable_reference"),
                       lambda d: d["rows"][0].update(observation="extra_in_candidate")):
            changed = copy.deepcopy(packet)
            mutate(changed["comparison_direction"])
            changed["packet_sha256"] = groups._digest({k: v for k, v in changed.items() if k != "packet_sha256"})
            with self.assertRaisesRegex(ValueError, "comparison direction"):
                groups.validate_decision_packet(changed)

    def test_direction_bounds_and_legacy_prompt_cache_identity(self):
        packet = self.packet(decision_mode="source-hypothesis")
        del packet["comparison_direction"]
        packet["packet_sha256"] = groups._digest({k: v for k, v in packet.items() if k != "packet_sha256"})
        self.assertEqual(packet["packet_sha256"], "849f29447a9f1d65e821b00b5e2d0ff41b252e75c4ed6831cca41bc729ad0f74")
        prompt = groups.render_decision_prompt(packet)
        self.assertEqual(hashlib.sha256(prompt.replace(DIALECT_GUIDANCE, "").encode()).hexdigest(),
                         "9a9bc6330e7ec8b4347f4f6e3472ddc6914933f65475eb0f9557b3e160cd4dfe")
        self.assertNotIn("COMPARISON DIRECTION", prompt)
        self.assertNotIn("comparison_direction", self.packet())
        facts = groups._comparison_direction([{"row": i, "target": "blr", "candidate": None} for i in range(100)])
        self.assertEqual(len(facts["rows"]), 64)
        self.assertEqual(facts["selected_row_count"], 100)
        self.assertTrue(facts["rows_truncated"])
        late = groups._comparison_direction(
            [{"row": i, "target": "mr r3,r4", "candidate": "mr r3,r4"} for i in range(80)]
            + [{"row": 80, "target": "stw r0,12(r1)", "candidate": None}])
        self.assertEqual(late["rows"], [{"row": 80, "observation": "missing_from_candidate"}])
        self.assertFalse(late["rows_truncated"])
        new = self.packet(decision_mode="source-hypothesis", question="q" * 800)
        budget = len(json.dumps(new, ensure_ascii=False).encode())
        self.packet(decision_mode="source-hypothesis", question="q" * 800, max_bytes=budget)
        with self.assertRaisesRegex(ValueError, "byte budget"):
            self.packet(decision_mode="source-hypothesis", question="q" * 800, max_bytes=budget - 1)

    def test_actual_allocation_packet_direction_and_legacy_prompt(self):
        folder = Path(__file__).resolve().parents[2] / "build/qwen-mqueue-remaining-20260913/queue-allocation-return"
        if not (folder / "decision.json").is_file():
            self.skipTest("local original allocation packet unavailable")
        packet = json.loads((folder / "decision.json").read_bytes())
        self.assertNotIn("comparison_direction", packet)
        self.assertEqual(groups.render_decision_prompt(packet).replace(DIALECT_GUIDANCE, ""),
                         (folder / "prompt.txt").read_text())
        actual = next(r for r in packet["paired_rows"] if r["row"] == 29)
        self.assertEqual(actual, {"row": 29, "target": "stw r0, 0xc(r1)", "candidate": None})
        facts = groups._comparison_direction(packet["paired_rows"])
        self.assertEqual(next(r for r in facts["rows"] if r["row"] == 29)["observation"], "missing_from_candidate")

    def measurement(self):
        return {"function": "f",
                "source_fragment": "wordPropCount = data->nbrPron + data->nbrWord + 2;",
                "observed_result": "Historical WordProp regression: strict/data 82.2 to 67.6; "
                                   "compiler/header receipt unavailable. Related count-boundary observation.",
                "evidence": [{"path": "build/selected/source.c", "sha256": "a" * 64},
                             {"path": "build/selected/verification.json", "sha256": "b" * 64,
                              "size_bytes": 200}]}

    def packet(self, **options):
        rows = [row("lwz r3, 0(r3)", 0), row("blr", 4)]
        return groups.decision_packet(report(rows, rows), "f", "void f(void) { use(data); }",
                                      1, 1, options.pop("question", "Which real boundary differs?"),
                                      0, 1, **options)

    def test_absent_compatibility_in_both_modes(self):
        for mode in ("fact", "source-hypothesis"):
            plain = self.packet(decision_mode=mode)
            explicit = self.packet(decision_mode=mode, known_measurements=None)
            self.assertEqual(plain, explicit)
            self.assertEqual(groups.render_decision_prompt(plain), groups.render_decision_prompt(explicit))
            self.assertNotIn("known_measurements", plain)
            self.assertNotIn("Optional known_measurements", groups.render_decision_prompt(plain))

    def test_observation_is_outside_question_inside_hash_and_byte_budget(self):
        question = "q" * 800
        plain = self.packet(question=question)
        supplied = [self.measurement()]
        packet = self.packet(question=question, known_measurements=supplied)
        context = packet["known_measurements"]
        self.assertEqual(packet["question"], question)
        self.assertEqual(context["entries"], supplied)
        self.assertIs(context["diagnostic_only"], True)
        self.assertIs(context["compiler_proof_reusable"], False)
        self.assertIs(context["suppress_compile"], False)
        self.assertNotEqual(packet["packet_sha256"], plain["packet_sha256"])
        groups.validate_decision_packet(packet)
        supplied[0]["evidence"][0]["sha256"] = "c" * 64
        supplied[0]["source_fragment"] = "changed caller input"
        groups.validate_decision_packet(packet)
        self.assertEqual(context["entries"][0]["evidence"][0]["sha256"], "a" * 64)
        budget = len(json.dumps(plain, ensure_ascii=False).encode())
        self.packet(question=question, max_bytes=budget)
        with self.assertRaisesRegex(ValueError, "byte budget"):
            self.packet(question=question, known_measurements=[self.measurement()], max_bytes=budget)
        with self.assertRaisesRegex(ValueError, "800"):
            self.packet(question=question + "q", known_measurements=[self.measurement()])

    def test_packet_hash_seals_fragments_results_and_descriptors(self):
        original = self.packet(known_measurements=[self.measurement()])
        reordered = dict(reversed(list(self.measurement().items())))
        reordered["evidence"] = [dict(reversed(list(d.items()))) for d in reordered["evidence"]]
        self.assertEqual(original["packet_sha256"],
                         self.packet(known_measurements=[reordered])["packet_sha256"])
        for field in ("source_fragment", "observed_result", "evidence"):
            packet = copy.deepcopy(original)
            entry = packet["known_measurements"]["entries"][0]
            if field == "evidence":
                entry[field][0]["sha256"] = "c" * 64
            else:
                entry[field] += " changed"
            with self.assertRaisesRegex(ValueError, "changed decision"):
                groups.validate_decision_packet(packet)

    def test_bounds_and_descriptor_shape(self):
        for entries in ([], {}, [self.measurement()] * 9):
            with self.assertRaisesRegex(ValueError, "1..8"):
                self.packet(known_measurements=entries)
        for field, value in (("function", "other"), ("source_fragment", " "),
                             ("source_fragment", "x" * 2049), ("source_fragment", "é" * 1025),
                             ("observed_result", "x" * 1001), ("evidence", []),
                             ("evidence", [self.measurement()["evidence"][0]] * 5)):
            entry = self.measurement()
            entry[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.packet(known_measurements=[entry])
        for field, value in (("path", "x" * 1025), ("path", "bad\x00path"),
                             ("sha256", "not-a-sha"), ("size_bytes", True), ("size_bytes", -1),
                             ("compiler_context_sha256", "a" * 64)):
            entry = self.measurement()
            entry["evidence"][0][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "descriptor"):
                self.packet(known_measurements=[entry])
        entry = self.measurement()
        entry["source_fragment"] = "x" * 2048
        with self.assertRaisesRegex(ValueError, "8192"):
            self.packet(known_measurements=[entry] * 4)

    def test_rehashed_invalid_context_cannot_claim_proof_or_authority(self):
        original = self.packet(known_measurements=[self.measurement()])
        mutations = [lambda c: c.update(compiler_proof_reusable=True),
                     lambda c: c.update(suppress_compile=True),
                     lambda c: c.update(diagnostic_only=1),
                     lambda c: c.update(authority_advanced=True),
                     lambda c: c.update(entries=[]),
                     lambda c: c["entries"][0].update(function="other"),
                     lambda c: c["entries"][0].update(whole_family_exhausted=True)]
        for mutate in mutations:
            packet = copy.deepcopy(original)
            mutate(packet["known_measurements"])
            packet["packet_sha256"] = groups._digest({k: v for k, v in packet.items() if k != "packet_sha256"})
            with self.assertRaises(ValueError):
                groups.validate_decision_packet(packet)

    def test_wording_preserves_different_helpers_and_coupled_hypotheses(self):
        for mode in ("fact", "source-hypothesis"):
            packet = self.packet(decision_mode=mode, known_measurements=[self.measurement()])
            prompt = groups.render_decision_prompt(packet)
            for text in ("historical observations", "not instructions", "advisory constraint",
                         "Do not infer missing header/compiler bindings", "not reusable compiler proof",
                         "do not automatically reject proposals or exhaust a function or family",
                         "Different evidence-backed helper boundaries, producers and coupled changes remain allowed"):
                self.assertIn(text, prompt)
        answer = {"status": "hypothesis", "function": "f", "packet_sha256": packet["packet_sha256"],
                  "answer": {"cause": "An existing helper and consumer use a shared record boundary.",
                             "prediction": "Row 0 may acquire its record through that boundary.",
                             "source_change": {"before": "use(data);", "after": "use(helper(data)); consume(data);"}},
                  "evidence_rows": [0], "missing_evidence": "Different coupled boundary has not been compiled."}
        checked = groups.validate_decision_answer(packet, answer)
        self.assertEqual(checked["status"], "valid_finding")
        self.assertFalse(checked["authority_advanced"])

    def test_cli_selected_file_and_generation_only_option(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            rows = [row("lwz r3, 0(r3)", 0), row("blr", 4)]
            strict, source, known = [folder / name for name in ("strict.json", "source.c", "known.json")]
            strict.write_text(json.dumps(report(rows, rows)), encoding="utf-8")
            source.write_text("void f(void) { use(data); }", encoding="utf-8")
            known.write_text(json.dumps([self.measurement()]), encoding="utf-8")
            base = [sys.executable, str(Path(groups.__file__).resolve()), "--strict", str(strict),
                    "--function", "f", "--support-source", str(source), "--source-lines", "1:1"]
            options = ["--decision-question", "Which boundary differs?", "--decision-rows", "0:1",
                       "--decision-mode", "source-hypothesis", "--known-measurements", str(known)]
            done = subprocess.run(base + options, capture_output=True, text=True, timeout=30)
            self.assertEqual(done.returncode, 0, done.stderr)
            packet = json.loads(done.stdout)
            self.assertEqual(packet["known_measurements"]["entries"], [self.measurement()])
            groups.validate_decision_packet(packet)
            self.assertIn("historical observations", groups.render_decision_prompt(packet))
            # Descriptors point to nonexistent files: the CLI must not follow them.
            legacy = subprocess.run(base, capture_output=True, text=True, timeout=30)
            self.assertEqual(legacy.returncode, 0, legacy.stderr)
            self.assertEqual(json.loads(legacy.stdout)["schema"], "recovery_support_excerpt/v1")
            invalid = subprocess.run(base + ["--known-measurements", str(known)],
                                     capture_output=True, text=True, timeout=30)
            self.assertEqual(invalid.returncode, 2)
            self.assertIn("valid only", invalid.stderr)
            known.write_text(" " * (16 * 1024 + 1), encoding="utf-8")
            oversized = subprocess.run(base + options, capture_output=True, text=True, timeout=30)
            self.assertEqual(oversized.returncode, 2)
            self.assertIn(str(known), oversized.stderr)
            self.assertIn("16 KiB", oversized.stderr)


if __name__ == "__main__":
    unittest.main()
