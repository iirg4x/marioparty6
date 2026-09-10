import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch, Mock

from tools import recovery_guided_search as guided


class GuidedSearchTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / 'build').mkdir()
        self.source = b'int original;\n'
        self.live = self.root / 'live.c'
        self.live.write_bytes(self.source)
        self.index = self.root / 'index.json'
        self.index.write_text('{}')
        self.plan = self.root / 'plan.json'
        self.plan.write_text('{}')
        self.base = {'owner': 'test', 'inputs': {'source': guided.evaluator._descriptor(self.live)}}
        self.spec = {'allow_retention': True, 'requests': [self.request('later', 20), self.request('first', 7)]}
        self.args = dict(root=self.root, index=self.index, plan=self.plan,
                         out_dir=self.root / 'build/run', scratch=self.root / 'scratch',
                         source_relpath='live.c', object_relpath='build/live.o',
                         compiler_script=self.root / 'compile.ps1', objdiff=self.root / 'objdiff',
                         readelf=self.root / 'readelf')
        self.calls = []
        self.mode = 'no_gain'

    def request(self, name, row):
        return {'id': name, 'function': 'f', 'families': ['approved'],
                'constraints': {}, 'target_rows': [row], 'evidence': []}

    def shape(self, value=b'int changed;\n'):
        return {'candidate_source': value, 'candidate_sha256': guided._sha(value),
                'source_sha256': guided._sha(self.source), 'family': 'approved', 'rationale': 'real consumer'}

    def measure(self, **kwargs):
        manifest = json.loads(kwargs['manifest'].read_text())
        candidate = self.root / manifest['candidates'][0]['source']
        self.assertTrue(candidate.exists())
        self.assertEqual(candidate.parent / 'manifest.json', kwargs['manifest'])
        selection = json.loads((candidate.parent / 'selection.json').read_text())
        self.calls.append(selection['request'])
        result = {'status': 'complete', 'candidates': [{'id': 'generated', 'status': 'no_gain'}],
                  'ranked_admissible_gains': []}
        if self.mode == 'infrastructure':
            result['status'] = 'partial'
        if self.mode in {'gain', 'review'}:
            measurement = candidate.parent / 'measurement.json'
            measurement.write_text(json.dumps({'status': 'improved', 'review_required': self.mode == 'review'}))
            result['candidates'][0].update(status='improved', evaluation=guided.evaluator._descriptor(measurement))
            result['ranked_admissible_gains'] = [{'id': 'generated'}]
        kwargs['out'].write_text(json.dumps(result))
        return result

    def execute(self, generator=None, **overrides):
        frozen = {str(self.live): guided._sha(self.source)}
        with patch.object(guided, '_load', return_value=(self.spec, self.base, self.source, frozen)), \
             patch.object(guided.frontier, 'verify'), \
             patch.object(guided.shapes, 'enumerate_shapes', side_effect=generator or (lambda *a: [self.shape()])), \
             patch.object(guided.batch, 'run_batch', side_effect=self.measure):
            return guided.run(**dict(self.args, **overrides))

    def test_first_actual_row_priority_duplicate_suppression_and_no_live_write(self):
        result = self.execute()
        self.assertEqual(self.calls, ['first'])
        self.assertEqual(result['dispatched'], 1)
        self.assertEqual(result['skipped'][0]['status'], 'duplicate_generated_source')
        self.assertEqual(self.live.read_bytes(), self.source)
        self.assertFalse(result['retained'])

    def test_stops_first_gain_without_automatic_retention(self):
        self.mode = 'gain'
        result = self.execute()
        self.assertEqual(result['status'], 'measured_gain')
        self.assertEqual(self.calls, ['first'])
        self.assertFalse(result['retained'])
        self.assertEqual(self.live.read_bytes(), self.source)

    def test_retention_only_clean_gain_uses_existing_api(self):
        from tools import recovery_retain
        self.mode = 'gain'
        with patch.object(recovery_retain, 'retain_gain', return_value={'retained': True}) as retain:
            result = self.execute(retain=True)
        self.assertEqual(result['status'], 'retained_gain')
        retain.assert_called_once()
        self.assertTrue(retain.call_args.kwargs['source_reviewed'])
        self.assertTrue(retain.call_args.kwargs['measured_result'].exists())

    def test_review_gain_never_retains(self):
        from tools import recovery_retain
        self.mode = 'review'
        with patch.object(recovery_retain, 'retain_gain') as retain:
            result = self.execute(retain=True)
        retain.assert_not_called()
        self.assertFalse(result['retained'])

    def test_context_bound_cached_gain_still_requires_independent_retention(self):
        from tools import recovery_retain
        previous = self.root / 'build/previous'
        previous.mkdir()
        frozen = previous / 'candidate.c'
        frozen.write_bytes(self.shape()['candidate_source'])
        obj = previous / 'candidate.o'
        obj.write_bytes(b'measured object')
        measurement = previous / 'evaluation.json'
        measurement.write_text(json.dumps({
            'schema': guided.evaluator.SCHEMA, 'status': 'improved', 'stage': 'complete',
            'baseline_index': guided.evaluator._descriptor(self.index),
            'candidate_source': guided.evaluator._descriptor(frozen),
            'candidate_object': guided.evaluator._descriptor(obj), 'functions': ['f'],
            'regressions': [], 'review_required': [], 'cleanup_errors': []}))
        entry = {'context_sha256': 'a' * 64, 'compiler_context_sha256': 'b' * 64,
                 'baseline_index_sha256': guided.compiler.digest(self.index),
                 'source_sha256': guided.compiler.digest(frozen),
                 'object_sha256': guided.compiler.digest(obj),
                 'result': guided.evaluator._descriptor(measurement),
                 'hypothesis': 'reviewed shape', 'disposition': 'improved', 'causal_summary': {}}
        def cached(**kwargs):
            result = {'status': 'complete', 'ranked_admissible_gains': [],
                      'candidates': [{'id': 'generated', 'status': 'duplicate_source',
                                      'source_sha256': entry['source_sha256'],
                                      'reused_measurement': entry}]}
            kwargs['out'].write_text(json.dumps(result))
            return result
        self.measure = cached
        with patch.object(recovery_retain, 'retain_gain', return_value={'retained': True}) as retain:
            result = self.execute(retain=True)
        self.assertEqual(result['status'], 'retained_gain')
        self.assertEqual(result['compiler_runs'], 0)
        retain.assert_called_once()
        self.assertEqual(retain.call_args.kwargs['measured_result'], measurement)
        self.assertTrue(retain.call_args.kwargs['source_reviewed'])
        self.assertEqual(retain.call_args.kwargs['candidate'].read_bytes(), frozen.read_bytes())

    def test_infrastructure_failure_stops_without_pivot(self):
        self.mode = 'infrastructure'
        result = self.execute()
        self.assertEqual(result['status'], 'infrastructure_failed')
        self.assertEqual(self.calls, ['first'])
        self.assertEqual(self.live.read_bytes(), self.source)

    def test_budget_and_immutable_output(self):
        result = self.execute(lambda *a: [self.shape(('int v%d;' % i).encode()) for i in range(10)], max_candidates=2)
        self.assertEqual(result['dispatched'], 2)
        self.assertEqual(result['status'], 'candidate_budget_exhausted')
        before = (self.args['out_dir'] / 'result.json').read_bytes()
        with self.assertRaises(ValueError):
            self.execute()
        self.assertEqual((self.args['out_dir'] / 'result.json').read_bytes(), before)

    def test_invalid_budget_and_deadline(self):
        for value in (0, 9, True):
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.execute(max_candidates=value)
        for value in (0, float('nan'), 1801):
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.execute(timeout=value)

    def test_deadline_expiry_before_generation(self):
        with patch.object(guided.time, 'monotonic', side_effect=[0, 400]):
            with self.assertRaises(TimeoutError):
                self.execute(timeout=300)
        self.assertFalse(self.args['out_dir'].exists())

    def test_invalid_generator_hash_refused(self):
        shape = self.shape()
        shape['source_sha256'] = '0' * 64
        result = self.execute(lambda *a: [shape])
        self.assertEqual(result['status'], 'failed')
        self.assertEqual(self.calls, [])
        self.assertFalse((self.args['out_dir'] / 'cell-00').exists())

    def test_unapproved_generator_family_refused(self):
        shape = self.shape()
        shape['family'] = 'unapproved'
        result = self.execute(lambda *a: [shape])
        self.assertEqual(result['status'], 'failed')
        self.assertEqual(self.calls, [])

    def test_retention_requires_explicit_plan_permission(self):
        self.spec['allow_retention'] = False
        with self.assertRaises(ValueError):
            self.execute(retain=True)
        self.assertFalse(self.args['out_dir'].exists())

    def test_generator_input_drift_emits_no_cell_evidence(self):
        def generator(*args):
            self.live.write_bytes(b'external drift')
            return [self.shape()]
        result = self.execute(generator)
        self.assertEqual(result['status'], 'failed')
        self.assertEqual(self.calls, [])
        self.assertFalse((self.args['out_dir'] / 'cell-00').exists())

    def load_fixture(self):
        evidence = self.root / 'evidence.txt'
        evidence.write_text('real consumer')
        report = self.root / 'report.json'
        report.write_text('{}')
        base = dict(self.base, compile_binding='receipt_hashes_match', data_functions=[], functions=[{'function': 'f'}])
        base['inputs'] = dict(base['inputs'],
            source=guided.frontier.read_bound(self.root, self.live, 4096)[1],
            strict_report=guided.frontier.read_bound(self.root, report, 4096)[1])
        self.index.write_text(json.dumps(base))
        req = self.request('one', 7)
        req['families'] = [next(iter(guided.shapes.FAMILIES))]
        req['constraints']['source_sha256'] = guided._sha(self.source)
        req['evidence'] = [guided.evaluator._descriptor(evidence)]
        spec = {'schema': guided.SCHEMA, 'root_reviewed': True, 'source_sha256': guided._sha(self.source),
                'baseline_index_sha256': guided.compiler.digest(self.index), 'requests': [req]}
        self.plan.write_text(json.dumps(spec))
        return spec

    def test_load_actual_rows_source_and_unknown_family(self):
        spec = self.load_fixture()
        with patch.object(guided.frontier, 'verify'), patch.object(guided, '_changed_rows', return_value={7}):
            guided._load(self.root, self.index, self.plan)
            for field, value in [('target_rows', [8]), ('families', ['unknown'])]:
                changed = json.loads(json.dumps(spec))
                changed['requests'][0][field] = value
                self.plan.write_text(json.dumps(changed))
                with self.assertRaises(ValueError):
                    guided._load(self.root, self.index, self.plan)

            self.plan.write_text(json.dumps(spec))
            self.live.write_bytes(b'drift')
            with self.assertRaises(ValueError):
                guided._load(self.root, self.index, self.plan)

    def working_fixture(self):
        spec = self.load_fixture()
        working = self.root / 'build/working.c'
        working.write_bytes(b'int reconstructed;\nint unchanged;\nint producer;\n')
        spec['working_source'] = dict(guided.evaluator._descriptor(working),
            champion_source_sha256=guided._sha(self.source),
            baseline_index_sha256=guided.compiler.digest(self.index),
            lineage='Measured reconstruction; rejected overall, useful lifetime constraint only',
            evidence=spec['requests'][0]['evidence'])
        spec['requests'][0]['constraints']['source_sha256'] = guided.compiler.digest(working)
        self.plan.write_text(json.dumps(spec))
        return spec, working

    def test_working_source_generates_but_champion_is_measured_and_unchanged(self):
        spec, working = self.working_fixture()
        champion = self.live.read_bytes()
        winner = working.read_bytes().replace(b'int producer;', b'int shared_producer;')
        def generate(source, function, families, constraints):
            self.assertEqual(source, working.read_bytes())
            return [dict(self.shape(winner), source_sha256=guided._sha(source), family=families[0])]
        original_measure = self.measure
        def measure(**kwargs):
            manifest = json.loads(kwargs['manifest'].read_text())
            self.assertEqual(manifest['live_source_sha256'], guided._sha(champion))
            self.assertEqual(manifest['working_source'], spec['working_source'])
            return original_measure(**kwargs)
        self.mode = 'gain'
        with patch.object(guided.frontier, 'verify'), patch.object(guided, '_changed_rows', return_value={7}), \
             patch.object(guided.shapes, 'enumerate_shapes', side_effect=generate), \
             patch.object(guided.batch, 'run_batch', side_effect=measure):
            result = guided.run(**self.args)
        self.assertEqual(result['status'], 'measured_gain')
        lineage = result['source_lineage']
        self.assertEqual(lineage['working_to_candidate']['removed_lines'], 1)
        self.assertEqual(lineage['champion_to_candidate']['added_lines'], 3)
        self.assertNotEqual(lineage['working_to_candidate']['source_sha256'], lineage['champion_to_candidate']['source_sha256'])
        self.assertEqual(self.live.read_bytes(), champion)
        self.assertFalse(result['retained'])

    def test_stale_working_or_champion_binding_fails_before_generation(self):
        spec, working = self.working_fixture()
        with patch.object(guided.frontier, 'verify'), patch.object(guided, '_changed_rows', return_value={7}), \
             patch.object(guided.shapes, 'enumerate_shapes') as generate:
            for field in ('sha256', 'champion_source_sha256', 'baseline_index_sha256'):
                changed = json.loads(json.dumps(spec))
                changed['working_source'][field] = '0' * 64
                self.plan.write_text(json.dumps(changed))
                with self.assertRaises(ValueError):
                    guided.run(**self.args)
            generate.assert_not_called()

    def test_large_lineage_diffs_keep_full_counts_hash_and_bounded_utf8_preview(self):
        import difflib
        spec, working = self.working_fixture()
        candidate = ''.join(f'int new_{i}; /* é */\n' for i in range(3000)).encode('utf-8')
        base = json.loads(self.index.read_text())
        lineage = guided.evaluator.source_lineage(self.root, self.index, base, spec['working_source'], candidate)
        for field, source, label in (
                ('working_to_candidate', working.read_bytes(), 'canonical-working-source'),
                ('champion_to_candidate', self.live.read_bytes(), 'protected-champion')):
            delta = lineage[field]
            full = ''.join(difflib.unified_diff(source.decode().splitlines(True),
                candidate.decode().splitlines(True), fromfile=label, tofile='candidate')).encode('utf-8')
            self.assertTrue(delta['diff_truncated'])
            self.assertLessEqual(len(delta['diff'].encode('utf-8')), 16 * 1024)
            self.assertEqual(delta['diff_sha256'], guided._sha(full))
            self.assertEqual(delta['diff_bytes'], len(full))
            self.assertEqual(delta['added_lines'], 3000)
            self.assertEqual(delta['removed_lines'], len(source.splitlines()))
            self.assertEqual(delta['candidate_sha256'], guided._sha(candidate))
        self.assertEqual(working.read_bytes(), b'int reconstructed;\nint unchanged;\nint producer;\n')

    def test_working_source_without_candidate_cannot_advance_champion(self):
        spec, working = self.working_fixture()
        with patch.object(guided.frontier, 'verify'), patch.object(guided, '_changed_rows', return_value={7}), \
             patch.object(guided.shapes, 'enumerate_shapes', return_value=[]), \
             patch.object(guided.batch, 'run_batch') as measure:
            result = guided.run(**self.args)
        self.assertEqual(result['status'], 'no_generated_gain')
        self.assertFalse(result['retained'])
        self.assertEqual(self.live.read_bytes(), self.source)
        measure.assert_not_called()


if __name__ == '__main__':
    unittest.main()
