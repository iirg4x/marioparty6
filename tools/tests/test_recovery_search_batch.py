from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import recovery_search_batch as batch


class SearchBatchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "build").mkdir()
        (self.root / "scratch").mkdir()
        self.write("live.c", b"int x;")
        self.write("evidence.txt", b"target evidence")
        self.write("compiler.ps1", b"trusted recipe")
        self.write("objdiff.exe", b"test")
        self.write("readelf.exe", b"test")
        self.write("baseline.o", b"baseline")
        self.write("index.json", json.dumps({"functions": [{"function": f"f{i}"} for i in range(44)],
                                                "inputs": {"candidate_object": batch.evaluator._descriptor(self.root / "baseline.o")},
                                                "data_functions": []}).encode())
        self.doc = {"schema": batch.SCHEMA, "root_reviewed": True,
                    "causal_family": "authenticated return type",
                    "baseline_index_sha256": self.sha("index.json"),
                    "live_source_sha256": self.sha("live.c"), "candidates": []}
        self.add_candidate("one", b"int x=1;")
        self.args = dict(root=self.root, index=Path("index.json"), scratch=Path("scratch"),
                         source_relpath="live.c", object_relpath="build/test.o",
                         compiler_script=Path("compiler.ps1"), manifest=Path("manifest.json"),
                         out=Path("build/result.json"), objdiff=self.root / "objdiff.exe",
                         readelf=self.root / "readelf.exe")
        self.context = {"headers": {}, "tools": {}, "command": ["test"]}
        self.compiles = []
        self.real_memory = False
        self.semantic_inventory = lambda path: {"semantic_sha256": batch.compiler.digest(path)}
        self.measurement_status = "no_gain"

    def write(self, name, raw):
        (self.root / name).write_bytes(raw)

    def sha(self, name):
        return batch.compiler.digest(self.root / name)

    def add_candidate(self, name, data):
        self.write(name + ".c", data)
        self.doc["candidates"].append({"id": name, "source": name + ".c", "sha256": self.sha(name + ".c"),
            "functions": ["f0"], "evidence": [{"path": "evidence.txt", "sha256": self.sha("evidence.txt")}]})

    def compile(self, **kwargs):
        self.compiles.append(kwargs["source"].name)
        kwargs["output"].write_bytes(kwargs["source"].read_bytes())
        receipt = {"schema": "recovery_candidate_compile/v1", "source_sha256": batch.compiler.digest(kwargs["source"]),
                   "context_sha256": batch.compiler.context_digest(self.context),
                   "object_sha256": batch.compiler.digest(kwargs["output"])}
        kwargs["output"].with_suffix(".o.receipt.json").write_text(json.dumps(receipt))
        return receipt

    def evaluate(self, **kwargs):
        # Validate the real evaluator manifest API, including owner-relative paths.
        _, _, jobs = batch.evaluator._batch_manifest(self.root, kwargs["manifest"])
        results = {}
        for job in jobs:
            path = kwargs["out"].with_name(job["id"] + ".result.json")
            path.write_text(json.dumps({"status": self.measurement_status,
                "baseline_index": batch.evaluator._descriptor(self.root / "index.json"),
                "candidate_source": job["candidate_desc"], "candidate_object": job["candidate_object_desc"]}))
            results[job["id"]] = {"status": self.measurement_status, "result": batch.evaluator._batch_result_descriptor(self.root, path)}
        return {"status": "complete", "results": results, "measured_gains": []}

    def test_neutral_identity_skips_compile_after_proof_change(self):
        self.real_memory = True
        self.write("baseline.o", (self.root / "one.c").read_bytes())
        baseline = json.loads((self.root / "index.json").read_text())
        baseline["inputs"]["candidate_object"] = batch.evaluator._descriptor(self.root / "baseline.o")
        self.write("index.json", json.dumps(baseline).encode())
        self.doc["baseline_index_sha256"] = self.sha("index.json")
        self.execute()
        self.compiles.clear()
        self.write("objdiff.exe", b"new proof implementation")
        self.args["out"] = Path("build/new-proof.json")
        result = self.execute()
        self.assertEqual(self.compiles, [])
        self.assertEqual(result["candidates"][0]["status"], "known_identical_baseline_object")
        self.assertFalse(result["retained"])
        self.assertEqual(result["ranked_admissible_gains"], [])
        self.assertFalse((self.root / "build/new-proof.search/one.c").exists())

    def execute(self):
        self.write("manifest.json", json.dumps(self.doc).encode())
        memory = mock.Mock()
        memory.lookup_source.return_value = memory.lookup_object.return_value = None
        memory.lookup_neutral_source.return_value = None
        if self.real_memory:
            memory = batch.recovery_search_memory.SearchMemory(self.root, self.root / "build/recovery-search-memory.json")
        with mock.patch.object(batch.frontier, "verify"), \
             mock.patch.object(batch.compiler, "preflight_context", return_value=self.context), \
             mock.patch.object(batch.compiler, "compile_candidate", side_effect=self.compile), \
             mock.patch.object(batch.evaluator.objects, "inventory", side_effect=self.semantic_inventory), \
             mock.patch.object(batch.evaluator, "evaluate_batch", side_effect=self.evaluate), \
             mock.patch.object(batch.recovery_search_memory, "SearchMemory", return_value=memory):
            return batch.run_batch(**self.args)

    def test_serial_compile_measurement_manifest_and_cleanup(self):
        self.add_candidate("two", b"int x=2;")
        result = self.execute()
        self.assertEqual(result["status"], "complete")
        self.assertEqual(result["owner_function_count"], 44)
        self.assertEqual(self.compiles, ["one.c", "two.c"])
        self.assertEqual((self.root / "live.c").read_bytes(), b"int x;")
        self.assertFalse(list((self.root / "build/result.search").glob("*.o")))

    def working_fixture(self):
        self.write('working.c', b'int reconstructed;\n')
        base = json.loads((self.root / 'index.json').read_text())
        base['inputs']['source'] = batch.evaluator._descriptor(self.root / 'live.c')
        self.write('index.json', json.dumps(base).encode())
        self.doc['baseline_index_sha256'] = self.sha('index.json')
        self.doc['working_source'] = dict(batch.evaluator._descriptor(self.root / 'working.c'),
            champion_source_sha256=self.sha('live.c'), baseline_index_sha256=self.sha('index.json'),
            lineage='measured reconstruction, not retained',
            evidence=[{'path': 'evidence.txt', 'sha256': self.sha('evidence.txt')}])

    def test_working_lineage_passes_through_evaluator_manifest_and_neutral_preserves_champion(self):
        self.working_fixture()
        original = self.evaluate
        def evaluate(**kwargs):
            _, _, jobs = batch.evaluator._batch_manifest(self.root, kwargs['manifest'])
            self.assertEqual(jobs[0]['working_source'], self.doc['working_source'])
            return original(**kwargs)
        self.evaluate = evaluate
        result = self.execute()
        lineage = result['candidates'][0]['source_lineage']
        self.assertEqual(lineage['champion_to_candidate']['source_sha256'], self.sha('live.c'))
        self.assertEqual(lineage['working_to_candidate']['source_sha256'], self.sha('working.c'))
        self.assertEqual(result['candidates'][0]['status'], 'no_gain')
        self.assertEqual((self.root / 'live.c').read_bytes(), b'int x;')
        self.assertFalse(result['retained'])

    def test_working_evidence_drift_fails_before_compile(self):
        self.working_fixture()
        self.write('working.c', b'drift')
        with self.assertRaises(ValueError):
            self.execute()
        self.assertEqual(self.compiles, [])

    def test_structured_hypothesis_is_advisory_and_exact_source_still_suppresses(self):
        self.real_memory = True
        self.working_fixture()
        self.doc['candidates'][0]['hypothesis_binding'] = {
            'family': 'shared_initializer_producer', 'scope': 'f0',
            'source_sha256': self.sha('working.c'), 'participants': ['a', 'b'],
            'boundary': 'initialization', 'causal_evidence_ids': ['target-evidence'], 'producer': 'a'}
        first = self.execute()
        self.assertEqual(first['candidates'][0]['hypothesis_memory']['decision'], 'unseen_hypothesis')
        self.assertNotIn('memory_warning', first['candidates'][0])
        self.args['out'] = Path('build/second.json')
        second = self.execute()
        self.assertEqual(second['candidates'][0]['status'], 'duplicate_source')
        self.assertEqual(len(self.compiles), 1)

    def test_duplicate_source_does_not_compile(self):
        self.add_candidate("two", b"int x=1;")
        result = self.execute()
        self.assertEqual(len(self.compiles), 1)
        self.assertEqual(result["candidates"][1]["status"], "duplicate_source")

    def test_semantic_duplicate_preserves_receipts_but_measures_once(self):
        self.add_candidate("two", b"int x=2;")
        self.semantic_inventory = lambda path: {"semantic_sha256": "a" * 64}
        result = self.execute()
        self.assertEqual(len(self.compiles), 2)
        self.assertEqual(result["candidates"][1]["status"], "duplicate_semantic_object")
        self.assertNotEqual(result["candidates"][0]["compile_receipt"]["object_sha256"],
                            result["candidates"][1]["compile_receipt"]["object_sha256"])
        self.assertEqual(len(result["measurement"]["results"]), 1)

    def test_semantic_duplicates_with_different_focus_are_measured(self):
        self.add_candidate("two", b"int x=2;")
        self.doc["candidates"][1]["functions"] = ["f1"]
        self.semantic_inventory = lambda path: {"semantic_sha256": "a" * 64}
        result = self.execute()
        self.assertEqual(len(result["measurement"]["results"]), 2)

    def test_semantic_aliases_reuse_both_sources_next_batch(self):
        self.real_memory = True
        self.add_candidate("two", b"int x=2;")
        self.semantic_inventory = lambda path: {"semantic_sha256": "a" * 64}
        first = self.execute()
        self.assertNotIn("memory_warning", first["candidates"][1])
        self.args["out"] = Path("build/second.json")
        second = self.execute()
        self.assertEqual([r["status"] for r in second["candidates"]], ["duplicate_source", "duplicate_source"])
        self.assertEqual(len(self.compiles), 2)

    def test_failed_canonical_does_not_cache_alias(self):
        self.real_memory = True
        self.measurement_status = "failed"
        self.add_candidate("two", b"int x=2;")
        self.semantic_inventory = lambda path: {"semantic_sha256": "a" * 64}
        first = self.execute()
        self.assertNotIn("evaluation", first["candidates"][1])
        self.args["out"] = Path("build/second.json")
        self.execute()
        self.assertEqual(len(self.compiles), 4)

    def test_changed_canonical_invalidates_alias_reuse(self):
        self.real_memory = True
        self.add_candidate("two", b"int x=2;")
        self.semantic_inventory = lambda path: {"semantic_sha256": "a" * 64}
        first = self.execute()
        path = self.root / first["candidates"][0]["evaluation"]["path"]
        path.write_text("{}")
        self.args["out"] = Path("build/second.json")
        self.execute()
        self.assertEqual(len(self.compiles), 4)

    def test_real_memory_suppresses_repeat_across_batches(self):
        self.real_memory = True
        first = self.execute()
        self.assertNotIn("memory_warning", first["candidates"][0])
        self.args["out"] = Path("build/second.json")
        second = self.execute()
        self.assertEqual(second["candidates"][0]["status"], "duplicate_source")
        self.assertEqual(len(self.compiles), 1)
        self.assertFalse(list((self.root / "build/second.search").glob("*.c")))

    def test_duplicate_object_does_not_measure_twice(self):
        self.add_candidate("two", b"int x=2;")
        original = self.compile
        def same_object(**kwargs):
            receipt = original(**kwargs)
            kwargs["output"].write_bytes(b"same object")
            receipt["object_sha256"] = batch.compiler.digest(kwargs["output"])
            return receipt
        self.compile = same_object
        result = self.execute()
        self.assertEqual(result["candidates"][1]["status"], "duplicate_object")

    def test_real_memory_does_not_reuse_different_focus(self):
        self.real_memory = True
        self.execute()
        self.args["out"] = Path("build/second.json")
        self.doc["candidates"][0]["functions"] = ["f1"]
        second = self.execute()
        self.assertEqual(second["candidates"][0]["status"], "no_gain")
        self.assertEqual(len(self.compiles), 2)

    def test_stale_source_rejected_before_compile(self):
        self.write("one.c", b"changed")
        with self.assertRaisesRegex(ValueError, "hash mismatch"):
            self.execute()
        self.assertEqual(self.compiles, [])

    def test_bounded_count(self):
        for i in range(8):
            self.add_candidate(f"extra{i}", b"int x;")
        with self.assertRaisesRegex(ValueError, "1..8"):
            self.execute()

    def test_compiler_failure_contained(self):
        self.add_candidate("two", b"int x=2;")
        original = self.compile
        def failure(**kwargs):
            if kwargs["source"].name == "one.c":
                raise RuntimeError("compiler failed")
            return original(**kwargs)
        self.compile = failure
        result = self.execute()
        self.assertEqual(result["status"], "partial")
        self.assertEqual(result["candidates"][0]["status"], "compile_failed")
        self.assertEqual(result["candidates"][1]["status"], "no_gain")

    def test_drift_stops_next_compile(self):
        self.add_candidate("two", b"int x=2;")
        original = self.compile
        def drift(**kwargs):
            result = original(**kwargs)
            self.write("evidence.txt", b"drift")
            return result
        self.compile = drift
        result = self.execute()
        self.assertEqual(result["status"], "drifted")
        self.assertEqual(len(self.compiles), 1)
        self.assertFalse(result["ranked_admissible_gains"])

    def test_compile_and_evaluation_share_remaining_deadline(self):
        clock = [0.0]
        original_compile, original_evaluate = self.compile, self.evaluate
        budgets = []
        def compile(**kwargs):
            budgets.append(kwargs['timeout'])
            value = original_compile(**kwargs)
            clock[0] = 7.0
            return value
        def evaluate(**kwargs):
            budgets.append(kwargs['timeout'])
            return original_evaluate(**kwargs)
        self.compile, self.evaluate = compile, evaluate
        self.args['timeout'] = 10
        with mock.patch.object(batch.time, 'monotonic', side_effect=lambda: clock[0]):
            result = self.execute()
        self.assertEqual(result['status'], 'complete')
        self.assertEqual(budgets, [10, 3])

    def test_expired_compile_budget_never_starts_evaluation(self):
        clock = [0.0]
        original = self.compile
        def compile(**kwargs):
            value = original(**kwargs)
            clock[0] = 10.0
            return value
        self.compile = compile
        self.evaluate = mock.Mock(side_effect=AssertionError('expired evaluation'))
        self.args['timeout'] = 10
        with mock.patch.object(batch.time, 'monotonic', side_effect=lambda: clock[0]):
            result = self.execute()
        self.assertEqual(result['status'], 'drifted')
        self.assertIn('deadline', result['reason'])
        self.evaluate.assert_not_called()
        self.assertFalse(list((self.root / 'build/result.search').glob('*.o')))

    def test_timeout_must_be_finite_positive(self):
        for timeout in (0, -1, True, float('nan'), float('inf')):
            with self.subTest(timeout=timeout), self.assertRaisesRegex(ValueError, 'timeout'):
                self.args['timeout'] = timeout
                self.execute()
        self.assertEqual(self.compiles, [])


if __name__ == "__main__":
    unittest.main()
