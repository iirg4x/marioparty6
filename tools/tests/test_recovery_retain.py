import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools import recovery_retain as retain


class RetainTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.live = self.write("src/owner.c", b"old")
        self.candidate = self.write("build/candidate.c", b"new")
        self.obj = self.write("build/measured.o", b"object")
        self.target = self.write("build/target.o", b"target")
        self.index = self.write("build/current.json", b"{}")
        self.base = {"owner": "test", "toolchain_key": "GC/2.6", "inputs": {
            "source": retain._desc(self.live), "target_object": retain._desc(self.target)}}
        self.index.write_text(json.dumps(self.base))
        self.context = {"tools": {}, "headers": {}, "generated_headers": {}}
        self.measurement = {"schema": retain.evaluator.SCHEMA, "status": "improved", "stage": "complete",
            "regressions": [], "review_required": [], "cleanup_errors": [], "functions": ["f"],
            "baseline_index": retain._desc(self.index), "candidate_source": retain._desc(self.candidate),
            "candidate_object": retain._desc(self.obj)}
        self.measured_path = self.write("build/measured.json", json.dumps(self.measurement).encode())
        self.script = self.write("build/compiler.ps1", b"script")
        self.proof = self.write("build/proof.exe", b"proof")
        self.directory = self.root / "build/retain"
        self.args = dict(root=self.root, index=self.index, candidate=self.candidate, functions=["f"],
            measured_result=self.measured_path, scratch=self.root / "build/scratch", source_relpath="src/owner.c",
            object_relpath="build/owner.o", compiler_script=self.script, objdiff=self.proof, readelf=self.proof,
            tools=[], out_dir=self.directory, source_reviewed=True)
        for target, fake in (("compiler.preflight_context", lambda **kw: copy.deepcopy(self.context)),
                             ("compiler.compile_candidate", self.compile), ("evaluator.evaluate", self.evaluate),
                             ("evaluator._implementation_binding", lambda: {}),
                             ("frontier.verify", lambda *a: None), ("frontier.snapshot", self.snapshot),
                             ("frontier.publish", lambda root, path, value: retain._write(path, value))):
            p = patch("tools.recovery_retain." + target, side_effect=fake)
            p.start()
            self.addCleanup(p.stop)

    def write(self, name, payload):
        p = self.root / name
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(payload)
        return p

    def compile(self, **kw):
        kw["output"].write_bytes(b"object")
        result = {"schema": "recovery_candidate_compile/v1", "source_sha256": retain.compiler.digest(kw["source"]),
            "object_sha256": retain.compiler.digest(kw["output"]),
            "tools": self.context["tools"],
            "header_set_sha256": hashlib.sha256(json.dumps(self.context["headers"], sort_keys=True).encode()).hexdigest(),
            "generated_header_set_sha256": hashlib.sha256(json.dumps(self.context["generated_headers"], sort_keys=True).encode()).hexdigest(),
            "context_sha256": retain.compiler.context_digest(self.context)}
        result.update(command=kw["command"], command_descriptor=None, object_size=6,
                      seconds=.01, stdout_sha256=hashlib.sha256(b"").hexdigest(),
                      stderr_sha256=hashlib.sha256(b"").hexdigest(),
                      stdout_path=str(kw["output"]) + ".stdout.log",
                      stderr_path=str(kw["output"]) + ".stderr.log")
        kw["output"].with_suffix(".o.receipt.json").write_text(json.dumps(result))
        return result

    def evaluate(self, **kw):
        result = copy.deepcopy(self.measurement)
        result["candidate_source"] = retain._desc(kw["candidate"])
        result["candidate_object"] = retain._desc(kw["candidate_object"])
        strict = self.write("build/retain/strict.json", b"{}")
        data = self.write("build/retain/data.json", b"{}")
        result["artifacts"] = {"strict.json": retain._desc(strict), "data.json": retain._desc(data)}
        kw["out"].write_text(json.dumps(result))
        return result

    def snapshot(self, **kw):
        return {"owner": "test", "inputs": {role: retain._desc(kw[arg]) for role, arg in
                (("source", "source"), ("target_object", "target"), ("candidate_object", "candidate"),
                 ("compile_receipt", "compile_receipt"), ("strict_report", "strict"), ("data_report", "data"))}}

    def test_improved_retained(self):
        result = retain.retain_gain(**self.args)
        self.assertEqual(result["status"], "retained")
        self.assertIs(result["retained"], True)
        self.assertEqual(self.live.read_bytes(), b"new")
        self.assertEqual(json.loads(self.index.read_text())["inputs"]["source"]["path"], "src/owner.c")
        self.assertFalse((self.directory / "pending.json").exists())

    def test_explicit_data_review_is_bound_and_preserves_measurement(self):
        self.measurement["review_required"] = [retain.DATA_REVIEW]
        self.measured_path.write_text(json.dumps(self.measurement))
        with self.assertRaisesRegex(ValueError, "reviewed"):
            retain.retain_gain(**self.args)
        result = retain.retain_gain(**self.args, data_reviewed=True)
        review = json.loads(Path(result["data_review"]["path"]).read_text())
        self.assertTrue(review["explicitly_reviewed"])
        self.assertEqual(review["source"]["sha256"], retain.compiler.digest(self.candidate))
        self.assertEqual(review["candidate_object"]["sha256"], retain.compiler.digest(self.obj))
        self.assertEqual(review["measured_evaluation"], retain._desc(self.measured_path))
        fresh = json.loads(Path(review["reproduced_evaluation"]["path"]).read_text())
        self.assertEqual(fresh["review_required"], [retain.DATA_REVIEW])
        self.assertEqual(json.loads(self.measured_path.read_text())["review_required"], [retain.DATA_REVIEW])

    def test_data_review_never_waives_regressions_other_reviews_or_binding(self):
        for change in ({"regressions": ["lost sibling"]}, {"review_required": [retain.DATA_REVIEW, "other"]},
                       {"review_required": ["other"]}, {"candidate_source": {"sha256": "wrong"}},
                       {"baseline_index": {"sha256": "wrong"}}, {"status": "rejected"}):
            doc = {**self.measurement, "review_required": [retain.DATA_REVIEW], **change}
            with self.assertRaises(ValueError):
                retain._gate(doc, retain.compiler.digest(self.index), retain.compiler.digest(self.candidate), ["f"], True)
        for truthy in (1, "yes"):
            with self.assertRaises(ValueError):
                retain._gate({**self.measurement, "review_required": [retain.DATA_REVIEW]},
                    retain.compiler.digest(self.index), retain.compiler.digest(self.candidate), ["f"], truthy)

    def test_exact_retained(self):
        self.measurement["status"] = "exact"
        self.measured_path.write_text(json.dumps(self.measurement))
        self.assertEqual(retain.retain_gain(**self.args)["status"], "retained")

    def working_lineage_fixture(self):
        working = self.write('build/working.c', b'reconstruction\nunchanged\nproducer\n')
        self.candidate.write_bytes(b'reconstruction\nunchanged\nshared producer\n')
        evidence = self.write('build/constraint.txt', b'measured rejected reconstruction; useful constraint')
        binding = dict(retain._desc(working), champion_source_sha256=retain.compiler.digest(self.live),
            baseline_index_sha256=retain.compiler.digest(self.index), lineage='rejected working reconstruction',
            evidence=[retain._desc(evidence)])
        lineage = retain.evaluator.source_lineage(self.root, self.index, self.base, binding, self.candidate.read_bytes())
        self.measurement.update(candidate_source=retain._desc(self.candidate), source_lineage=lineage)
        self.measured_path.write_text(json.dumps(self.measurement))
        return working, lineage

    def test_working_gain_journal_keeps_both_true_deltas_and_resumes(self):
        working, lineage = self.working_lineage_fixture()
        with patch.object(retain.frontier, 'publish', side_effect=OSError('publication failure')):
            with self.assertRaises(OSError):
                retain.retain_gain(**self.args)
        pending = json.loads((self.directory / 'pending.json').read_text())
        self.assertEqual(pending['source_lineage'], lineage)
        self.assertEqual(lineage['working_to_candidate']['removed_lines'], 1)
        self.assertEqual(lineage['champion_to_candidate']['added_lines'], 3)
        result = retain.resume_pending(self.root, self.directory)
        self.assertEqual(result['source_lineage'], lineage)
        self.assertEqual(self.live.read_bytes(), self.candidate.read_bytes())
        self.assertEqual(working.read_bytes(), b'reconstruction\nunchanged\nproducer\n')

    def test_stale_working_rejected_before_independent_compile(self):
        working, _ = self.working_lineage_fixture()
        working.write_bytes(b'drift')
        with patch.object(retain.compiler, 'compile_candidate') as compile:
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        compile.assert_not_called()
        self.assertEqual(self.live.read_bytes(), b'old')

    def test_working_neutral_never_replaces_champion(self):
        self.working_lineage_fixture()
        self.measurement['status'] = 'no_gain'
        self.measured_path.write_text(json.dumps(self.measurement))
        with patch.object(retain.compiler, 'compile_candidate') as compile:
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        compile.assert_not_called()
        self.assertEqual(self.live.read_bytes(), b'old')

    def test_published_gain_advances_old_cache_frontier(self):
        memory = retain.SearchMemory(self.root, self.root / 'build/recovery-search-memory.json')
        old_sha = retain.compiler.digest(self.index)
        memory.advance_frontier(self.index, old_sha)
        result = retain.retain_gain(**self.args)
        self.assertEqual(result['search_memory']['status'], 'advanced')
        self.assertEqual(memory._read()['latest_frontier']['sha256'], retain.compiler.digest(self.index))
        # Replay after the index has already advanced is harmless.
        self.assertEqual(retain._advance_memory(self.root, self.index, old_sha)['status'], 'advanced')

    def test_cache_failure_does_not_lose_published_gain(self):
        with patch.object(retain.SearchMemory, 'advance_frontier', side_effect=OSError('cache unavailable')):
            result = retain.retain_gain(**self.args)
        self.assertTrue(result['retained'])
        self.assertEqual(result['search_memory']['status'], 'warning')
        self.assertEqual(self.live.read_bytes(), b'new')
        self.assertFalse((self.directory / 'pending.json').exists())
        self.assertEqual(json.loads(self.index.read_text())['inputs']['source']['path'], 'src/owner.c')

    def test_unknown_cache_frontier_is_preserved_as_advisory_warning(self):
        memory = retain.SearchMemory(self.root, self.root / 'build/recovery-search-memory.json')
        unrelated = self.write('build/unrelated.json', b'unrelated')
        memory.advance_frontier(unrelated, retain.compiler.digest(self.index))
        before = memory.path.read_bytes()
        result = retain.retain_gain(**self.args)
        self.assertTrue(result['retained'])
        self.assertEqual(result['search_memory']['status'], 'warning')
        self.assertEqual(memory.path.read_bytes(), before)

    def test_nullable_tools_and_larger_evaluation(self):
        self.measurement["diagnostic"] = "x" * (retain.LIMIT + 1)
        self.measured_path.write_text(json.dumps(self.measurement))
        self.assertTrue(retain.retain_gain(**dict(self.args, tools=None))["retained"])

    def test_invalid_deadlines(self):
        for timeout in (0, -1, float("nan"), float("inf"), True):
            with self.subTest(timeout=timeout), self.assertRaises(ValueError):
                retain.retain_gain(**dict(self.args, timeout=timeout))
        self.assertEqual(self.live.read_bytes(), b"old")

    def test_compile_and_evaluate_share_deadline(self):
        clock = [0.0]
        limits = []
        def compile(**kw):
            limits.append(kw["timeout"])
            clock[0] = 7.0
            return self.compile(**kw)
        def evaluate(**kw):
            limits.append(kw["timeout"])
            return self.evaluate(**kw)
        with patch.object(retain.time, "monotonic", side_effect=lambda: clock[0]), \
                patch.object(retain.compiler, "compile_candidate", side_effect=compile), \
                patch.object(retain.evaluator, "evaluate", side_effect=evaluate):
            retain.retain_gain(**dict(self.args, timeout=10))
        self.assertEqual(limits, [10, 3])

    def test_missing_compiler_receipt_is_rejected(self):
        def compile(**kw):
            result = self.compile(**kw)
            kw["output"].with_suffix(".o.receipt.json").unlink()
            return result
        with patch.object(retain.compiler, "compile_candidate", side_effect=compile):
            with self.assertRaises(OSError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"old")
        self.assertFalse((self.directory / "candidate.o").exists())
        self.assertTrue((self.directory / "failure.json").exists())

    def test_snapshot_failure_cleans_raw_reports(self):
        with patch.object(retain.frontier, "snapshot", side_effect=ValueError("bad proof")):
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"old")
        self.assertFalse((self.directory / "strict.json").exists())
        self.assertFalse((self.directory / "candidate.o").exists())

    def test_rejected_gates_leave_source(self):
        for field, value in (("status", "no_gain"), ("review_required", ["review"]), ("regressions", ["loss"])):
            with self.subTest(field=field):
                doc = dict(self.measurement, **{field: value})
                self.measured_path.write_text(json.dumps(doc))
                with self.assertRaises(ValueError):
                    retain.retain_gain(**self.args)
                self.assertEqual(self.live.read_bytes(), b"old")

    def test_wrong_object_rejected(self):
        self.obj.write_bytes(b"different")
        self.measurement["candidate_object"] = retain._desc(self.obj)
        self.measured_path.write_text(json.dumps(self.measurement))
        with self.assertRaises(ValueError):
            retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"old")

    def test_independent_no_gain_leaves_source_and_discards_raw_reports(self):
        def evaluate(**kw):
            result = self.evaluate(**kw)
            result["status"] = "no_gain"
            return result
        with patch.object(retain.evaluator, "evaluate", side_effect=evaluate):
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"old")
        self.assertFalse((self.directory / "strict.json").exists())
        self.assertFalse((self.directory / "candidate.o").exists())

    def test_source_drift_before_write(self):
        def evaluate(**kw):
            result = self.evaluate(**kw)
            self.live.write_bytes(b"concurrent")
            return result
        with patch.object(retain.evaluator, "evaluate", side_effect=evaluate):
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"concurrent")

    def test_index_drift_before_write(self):
        def evaluate(**kw):
            result = self.evaluate(**kw)
            self.index.write_bytes(b"concurrent")
            return result
        with patch.object(retain.evaluator, "evaluate", side_effect=evaluate):
            with self.assertRaises(ValueError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"old")

    def test_pending_after_source_write_resumes(self):
        with patch.object(retain.frontier, "publish", side_effect=OSError("publication failure")):
            with self.assertRaises(OSError):
                retain.retain_gain(**self.args)
        self.assertEqual(self.live.read_bytes(), b"new")
        self.assertTrue((self.directory / "pending.json").exists())
        self.assertEqual(retain.resume_pending(self.root, self.directory)["status"], "retained")

    def test_pending_never_overwrites_unknown_source(self):
        with patch.object(retain.frontier, "publish", side_effect=OSError("failure")):
            with self.assertRaises(OSError):
                retain.retain_gain(**self.args)
        self.live.write_bytes(b"concurrent")
        with self.assertRaises(ValueError):
            retain.resume_pending(self.root, self.directory)
        self.assertEqual(self.live.read_bytes(), b"concurrent")

    def test_paths_escape_and_unreviewed_rejected(self):
        for changes in ({"candidate": self.root.parent / "outside.c"}, {"source_reviewed": False},
                        {"out_dir": self.root.parent / "outside"}):
            with self.assertRaises((ValueError, OSError)):
                retain.retain_gain(**dict(self.args, **changes))


if __name__ == "__main__":
    unittest.main()
