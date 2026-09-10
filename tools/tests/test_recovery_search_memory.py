import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools import recovery_search_memory as memory


class SearchMemoryTests(unittest.TestCase):
    def binding(self, **changes):
        return dict({"family": "initialization", "scope": "ev_CapKoopaCoin",
                     "source_sha256": "e" * 64, "participants": ["loseCount", "teamLose"],
                     "boundary": "creation-phase", "causal_evidence_ids": ["target:0x801BF780"],
                     "producer": "teamLose", "dependencies": ["selector-reload"]}, **changes)

    def test_hypothesis_diagnostics_never_deny_a_family(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
        warning = self.store.lookup_hypothesis(self.context, self.binding())
        self.assertEqual(warning["decision"], "related_hypothesis_warning")
        self.assertFalse(warning["suppress_compile"])
        for changes in ({"producer": "loseCount"}, {"dependencies": ["new-inline-family"]},
                        {"boundary": "nested-row-loop"}, {"source_sha256": "f" * 64},
                        {"scope": "another_function"}):
            result = self.store.lookup_hypothesis(self.context, self.binding(**changes))
            self.assertEqual(result["decision"], "unseen_hypothesis")
            self.assertFalse(result["suppress_compile"])

    def test_hypothesis_renamed_candidate_exact_hash_and_neutral_object(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
        renamed = self.root / "different-filename.c"
        renamed.write_text("one")
        duplicate = self.store.lookup_hypothesis(self.context, self.binding(),
            source_sha256=memory.compiler.digest(renamed))
        self.assertEqual(duplicate["decision"], "exact_measured_duplicate")
        self.assertTrue(duplicate["suppress_compile"])
        obj = self.store.lookup_hypothesis(self.context, self.binding(), object_sha256=o)
        self.assertEqual(obj["decision"], "exact_measured_duplicate")
        self.assertFalse(obj["suppress_compile"])
        self.assertFalse(obj["authority_advanced"])

    def test_hypothesis_context_or_evidence_change_is_not_reusable(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
        for field in ("compiler", "proof_tools", "implementation", "working_source", "champion"):
            changed = dict(self.context, **{field: {"sha256": "f" * 64}})
            self.assertEqual(self.store.lookup_hypothesis(changed, self.binding(),
                source_sha256=s)["decision"], "unseen_hypothesis")
        path.write_text("stale")
        self.assertFalse(self.store.lookup_hypothesis(self.context, self.binding(),
            source_sha256=s)["suppress_compile"])

    def test_flat_and_nested_loop_are_distinct_and_lower_score_is_diagnostic(self):
        s, o, path = self.fixture()
        result = json.loads(path.read_text())
        result["status"] = "rejected"
        path.write_text(json.dumps(result))
        flat = self.binding(family="loop", boundary="flat-indexed-loop")
        self.store.record(self.context, s, o, path, hypothesis_binding=flat,
            causal_summary={"disposition": "rejected", "strict": {
                "f": {"structural_hazard_count": 0, "residual_rows_delta": 3}}})
        diag = self.store.lookup_hypothesis(self.context, flat)
        self.assertTrue(diag["matches"][0]["causal_summary"]["diagnostic_only"])
        self.assertEqual(diag["matches"][0]["disposition"], "rejected")
        self.assertEqual(diag["matches"][0]["causal_summary"]["strict"]["f"]
                         ["structural_hazard_count"], 0)
        self.assertEqual(self.store.lookup_hypothesis(self.context,
            self.binding(family="loop", boundary="nested-row-loop"))["decision"], "unseen_hypothesis")

    def test_hypothesis_binding_validates_hash_and_bounds(self):
        for changes in ({"source_sha256": "bad"}, {"participants": []},
                        {"family": "x" * 257}, {"dependencies": ["x"] * 17}):
            with self.assertRaises(ValueError):
                self.store.lookup_hypothesis(self.context, self.binding(**changes))
        s, o, path = self.fixture(receipt=False)
        self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
        self.assertFalse(self.store.lookup_hypothesis(self.context, self.binding(),
            source_sha256=s)["suppress_compile"])

    def test_unmeasured_terminal_states_never_suppress_compile(self):
        s, o, path = self.fixture()
        entry = self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
        result = json.loads(path.read_text())
        for status in ("compile_failed", "failed", "infrastructure_failed", "interrupted",
                       "cancelled", "unknown", "duplicate_source", "future_terminal_state"):
            with self.subTest(status=status):
                result["status"] = status
                path.write_text(json.dumps(result))
                with self.assertRaises(ValueError):
                    self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
                # Model an older store accepting this outcome, including valid
                # result digest, source/object files and compile receipt.
                entry["result"] = self.store._descriptor(path)
                entry["disposition"] = status
                self.store._write({"schema": memory.SCHEMA, "entries": [entry],
                                   "latest_frontier": None})
                self.assertIsNone(self.store.lookup_source(self.context, s))
                self.assertIsNone(self.store.lookup_object(self.context, o))
                self.assertFalse(self.store.lookup_hypothesis(self.context, self.binding(),
                    source_sha256=s, object_sha256=o)["suppress_compile"])

    def test_measured_rejections_and_no_gain_remain_reusable(self):
        s, o, path = self.fixture()
        result = json.loads(path.read_text())
        for status in ("rejected", "no_gain", "duplicate_object", "duplicate_semantic_object",
                       "exact", "improved"):
            with self.subTest(status=status):
                result["status"] = status
                path.write_text(json.dumps(result))
                self.store.record(self.context, s, o, path, hypothesis_binding=self.binding())
                self.assertIsNotNone(self.store.lookup_source(self.context, s))
                self.assertIsNotNone(self.store.lookup_object(self.context, o))
                self.assertTrue(self.store.lookup_hypothesis(self.context, self.binding(),
                    source_sha256=s)["suppress_compile"])
                self.assertEqual(self.store.lookup_hypothesis(self.context, self.binding())
                                 ["decision"], "related_hypothesis_warning")

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.store = memory.SearchMemory(self.root, Path("build/search.json"))
        self.context = {"baseline_index_sha256": "a" * 64, "compiler": {"headers": "b" * 64},
                        "proof_tools": {"objdiff": "c" * 64}, "implementation": {"evaluate": "d" * 64}}

    def fixture(self, name="one", receipt=True):
        source = self.root / (name + ".c")
        obj = self.root / (name + ".o")
        source.write_text(name, encoding="utf-8")
        obj.write_bytes(name.encode())
        s, o = self.store._descriptor(source), self.store._descriptor(obj)
        result = {"baseline_index": {"sha256": self.context["baseline_index_sha256"]},
                  "candidate_source": s, "candidate_object": o, "status": "neutral"}
        if receipt:
            result["compile_receipt"] = {"schema": "recovery_candidate_compile/v1",
                                         "context_sha256": memory.compiler.context_digest(self.context["compiler"]),
                                         "source_sha256": s["sha256"], "object_sha256": o["sha256"]}
        path = self.root / (name + ".json")
        path.write_text(json.dumps(result), encoding="utf-8")
        return s["sha256"], o["sha256"], path

    def test_source_and_object_reuse(self):
        s, o, path = self.fixture()
        entry = self.store.record(self.context, s, o, path)
        self.assertEqual(self.store.lookup_source(self.context, s), entry)
        self.assertEqual(self.store.lookup_object(self.context, o), entry)

    def test_neutral_identity_survives_proof_changes_only(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path)
        baseline = self.store._descriptor(path.with_suffix('.o'))
        changed = dict(self.context, implementation={"new": "e"*64}, proof_tools={"new": "f"*64})
        self.assertIsNone(self.store.lookup_source(changed, s))
        result = self.store.lookup_neutral_source(changed, s, baseline)
        self.assertEqual(result['status'], 'known_identical_baseline_object')
        self.assertNotIn('disposition', result)
        self.assertIsNone(self.store.lookup_neutral_source(dict(changed, compiler={"headers": "changed"}), s, baseline))
        self.assertIsNone(self.store.lookup_neutral_source(changed, '0'*64, baseline))
        path.with_suffix('.o').write_bytes(b'drift')
        self.assertIsNone(self.store.lookup_neutral_source(changed, s, baseline))

    def test_neutral_identity_requires_valid_source_and_receipt(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path)
        baseline = self.store._descriptor(path.with_suffix('.o'))
        path.with_suffix('.c').write_bytes(b'drift')
        self.assertIsNone(self.store.lookup_neutral_source(self.context, s, baseline))

    def test_root_relative_evaluator_descriptors(self):
        s, o, path = self.fixture()
        result = json.loads(path.read_text())
        for field in ("candidate_source", "candidate_object"):
            result[field]["path"] = Path(result[field]["path"]).relative_to(self.root).as_posix()
        path.write_text(json.dumps(result))
        self.store.record(self.context, s, o, path)
        self.assertIsNotNone(self.store.lookup_source(self.context, s))

    def test_stale_result_source_object_and_receipt(self):
        for changed in ("result", "source", "object", "receipt"):
            with self.subTest(changed=changed):
                s, o, path = self.fixture(changed)
                receipt = self.root / (changed + "-receipt.json")
                receipt.write_text(json.dumps(json.loads(path.read_text())["compile_receipt"]))
                self.store.record(self.context, s, o, path, compile_receipt_path=receipt)
                target = {"result": path, "source": path.with_suffix(".c"),
                          "object": path.with_suffix(".o"), "receipt": receipt}[changed]
                target.write_text("changed")
                self.assertIsNone(self.store.lookup_source(self.context, s))

    def test_receipt_mismatch_rejected(self):
        s, o, path = self.fixture()
        result = json.loads(path.read_text())
        result["compile_receipt"]["object_sha256"] = "f" * 64
        path.write_text(json.dumps(result))
        with self.assertRaises(ValueError):
            self.store.record(self.context, s, o, path)

    def test_wrong_compiler_context_rejected(self):
        s, o, path = self.fixture()
        wrong = dict(self.context, compiler={"headers": "e" * 64})
        with self.assertRaises(ValueError):
            self.store.record(wrong, s, o, path)

    def test_caller_object_does_not_prove_source(self):
        s, o, path = self.fixture(receipt=False)
        self.store.record(self.context, s, o, path)
        self.assertIsNone(self.store.lookup_source(self.context, s))
        self.assertIsNotNone(self.store.lookup_object(self.context, o))

    def test_context_drift_and_neutral_different_candidate(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path)
        for key in self.context:
            context = dict(self.context)
            context[key] = "e" * 64 if key == "baseline_index_sha256" else {"changed": "e" * 64}
            self.assertIsNone(self.store.lookup_source(context, s))
        other, _, _ = self.fixture("other")
        self.assertIsNone(self.store.lookup_source(self.context, other))

    def test_frontier_advance_requires_new_proof(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path)
        index = self.root / "current.json"
        index.write_text("new index")
        desc = self.store.advance_frontier(index, self.context["baseline_index_sha256"])
        self.assertIsNone(self.store.lookup_source(self.context, s))
        self.assertEqual(self.store._read()["entries"], [])
        with self.assertRaises(ValueError):
            self.store.advance_frontier(index, "b" * 64)
        self.assertEqual(self.store._read()["latest_frontier"], desc)

    def test_bounded_compaction(self):
        for number in range(memory.MAX_ENTRIES + 2):
            s, o, path = self.fixture(str(number))
            self.store.record(self.context, s, o, path)
        self.assertEqual(len(self.store._read()["entries"]), memory.MAX_ENTRIES)
        self.assertLessEqual(self.store.path.stat().st_size, memory.MAX_BYTES)

    def test_atomic_failure_preserves_old_state(self):
        s, o, path = self.fixture()
        self.store.record(self.context, s, o, path)
        before = self.store.path.read_bytes()
        with patch.object(memory.compiler, "atomic", side_effect=OSError("write failed")):
            with self.assertRaises(OSError):
                self.store.record(self.context, s, o, path, hypothesis="new")
        self.assertEqual(self.store.path.read_bytes(), before)

    def test_actual_producer_summary_preserves_bound_diagnostics(self):
        s, o, path = self.fixture()
        result = json.loads(path.read_text())
        result["status"] = "rejected"
        result["causal_groups"] = {"channels": {"strict": {"f": {
            "status": "observed", "after": {"unresolved_row_count": 4,
                "first_machine_divergence": {"row": 7, "target": {
                    "instruction": {"address": "100", "formatted": "mulli r4, r22, 264"}},
                    "context": ["must not persist"]}},
            "change": {"closed_groups": ["target:96"], "new_groups": ["target:100"]}}}}}
        path.write_text(json.dumps(result))
        producer = {"diagnostic_only": True, "strict": {"f": {
            "status": "observed", "closed_groups": 1, "new_groups": 1,
            "category_rows": {"register_unknown": 4}}}, "full_detail": self.store._descriptor(path)}
        entry = self.store.record(self.context, s, o, path, causal_summary=producer)
        facts = entry["causal_summary"]["strict"]["f"]
        self.assertEqual(facts["unresolved_row_count"], 4)
        self.assertEqual(facts["closed_groups_ids"], ["target:96"])
        self.assertEqual(facts["first_machine_divergence"]["target"]["address"], "100")
        self.assertNotIn("must not persist", json.dumps(entry))
        self.assertNotIn("full_detail", entry["causal_summary"])
        self.assertEqual(self.store.lookup_source(self.context, s), entry)
        self.assertIsNone(self.store.lookup_source(dict(self.context, functions=["other"]), s))
        path.write_text("stale rejected result")
        self.assertIsNone(self.store.lookup_source(self.context, s))

    def test_summary_is_deterministic_bounded_and_legacy_compatible(self):
        rows = {"f%02d" % i: {"status": "observed", "after": {"unresolved_row_count": i},
                "change": {"new_groups": ["x" * 200 + str(j) for j in range(30)]}}
                for i in range(25)}
        result = {"causal_groups": {"channels": {"strict": rows}}}
        first = memory.compact_causal_summary({"disposition": "rejected"}, result)
        rows = dict(reversed(list(rows.items())))
        second = memory.compact_causal_summary({"disposition": "rejected"},
                    {"causal_groups": {"channels": {"strict": rows}}})
        self.assertEqual(first, second)
        self.assertTrue(first["truncated"])
        self.assertLessEqual(len(memory._canonical(first)), 4096)
        self.assertEqual(first["disposition"], "rejected")
        self.assertEqual(first["strict"]["f00"]["new_groups_count"], 30)


if __name__ == "__main__":
    unittest.main()
