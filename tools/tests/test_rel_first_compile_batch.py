"""Deterministic synthetic-only tests; never invokes a real compiler."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import argparse
import contextlib
import io
import sys
import threading
import time
from unittest.mock import patch

if __package__ in {None, ''}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools import rel_first_compile_batch as batch

class BatchTests(unittest.TestCase):
    def tool_files(self, root):
        for name in ('build/tools/sjiswrap.exe', 'build/compilers/GC/1.3.2/mwcceppc.exe'):
            path = root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'fake executable')

    def test_worker_arg_bounds(self):
        for value in ('0', '9', '-1', 'abc', '1.5'):
            with self.assertRaises(argparse.ArgumentTypeError): batch.worker_count(value)
        self.assertEqual(batch.worker_count('1'), 1)
        self.assertEqual(batch.worker_count('8'), 8)

    def test_abi_profile_reaches_translator_and_legacy_is_unchanged(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inp = root/'input'; inp.write_text('input')
            module = dict(name='m', target_path=str(inp), assembly_paths=[str(inp)])
            manifest = dict(output_root=str(root), prefix_path=str(inp), context_path=str(inp),
                            m2c=str(inp), proof_root=str(root))
            binding = {'files': {str(inp): batch.sha(inp)}}
            def fake(argv, cwd, scratch, *args):
                (scratch/'stdout.log').write_text('synthetic failure')
                return 1, '', ''
            expected = [sys.executable, str(inp), '-t', 'ppc-mwcc-c', '--knr',
                        '--valid-syntax', '--force-decimal', '--stacktrace', '--context', str(inp), '-f', 'f', str(inp)]
            for profile in (None, 'legacy', 'gekko-eabi'):
                selected = manifest if profile is None else {**manifest, 'ppc_abi': profile}
                with patch.object(batch, 'run_process', side_effect=fake) as run:
                    batch.attempt(selected, module, 'f', 0, binding)
                argv = run.call_args.args[0]
                self.assertEqual(argv, expected if profile != 'gekko-eabi'
                                 else expected[:4]+['--ppc-abi', profile]+expected[4:])
            with patch.object(batch, 'run_process') as run:
                row, _, _ = batch.attempt({**manifest, 'ppc_abi': 'unknown'}, module, 'f', 0, binding)
                run.assert_not_called()
                self.assertIn('unsupported PPC argument ABI', row['diagnostic'])

    def test_resume_rejects_changed_tool_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            self.tool_files(root)
            inp = root/'input'; inp.write_text('input')
            m2c, objdiff = root/'m2c.py', root/'objdiff.exe'
            m2c.write_text('translator'); objdiff.write_text('diff tool')
            package_files = [root/'m2c/arch_ppc.py', root/'m2c/translate.py',
                             root/'m2c_pycparser/ply/yacc.py', root/'m2c_macros.h']
            for path in package_files:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('original dependency')
            m = dict(schema='rel_first_compile_batch/v1', root=str(root), proof_root=str(root),
                     m2c=str(m2c), objdiff=str(objdiff), context_path=str(inp), prefix_path=str(inp),
                     output_root=str(root/'build/run'), modules=[])
            mp = root/'manifest.json'; mp.write_text(json.dumps(m))
            with contextlib.redirect_stdout(io.StringIO()):
                with patch.object(sys, 'argv', ['batch', str(mp)]): batch.main()
                for tool in (m2c, objdiff, root/'build/tools/sjiswrap.exe',
                             root/'build/compilers/GC/1.3.2/mwcceppc.exe', *package_files):
                    original = tool.read_bytes()
                    tool.write_bytes(original+b' changed')
                    with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), \
                            self.assertRaisesRegex(ValueError, 'resume input binding mismatch'):
                        batch.main()
                    tool.write_bytes(original)
                added = root/'m2c/new_module.py'
                added.write_text('new dependency')
                with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), \
                        self.assertRaisesRegex(ValueError, 'resume input binding mismatch'):
                    batch.main()
                added.unlink()
                m['ppc_abi'] = 'unknown'; mp.write_text(json.dumps(m))
                with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), \
                        self.assertRaisesRegex(ValueError, 'unsupported PPC argument ABI'):
                        batch.main()

    def test_translator_dependency_scope_limits_and_final_membership(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            launcher = root/'standalone.py'
            launcher.write_text('standalone')
            self.assertEqual(batch.translator_dependencies(launcher), [str(launcher)])
            module, parser, macro = root/'m2c/main.py', root/'m2c_pycparser/ply/lex.py', root/'m2c_macros.h'
            for path in (module, parser, macro):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('dependency')
            (root/'unrelated.py').write_text('not a translator dependency')
            (module.parent/'README.md').write_text('not Python')
            paths = batch.translator_dependencies(launcher)
            self.assertEqual(paths, sorted(map(str, (launcher, module, parser, macro))))
            binding = {'translator_dependencies': {'launcher': str(launcher), 'paths': paths},
                       'files': {p: batch.sha(p) for p in paths}}
            self.assertTrue(batch.unchanged(binding))
            with patch.object(batch, 'TRANSLATOR_FILE_LIMIT', 3):
                with self.assertRaisesRegex(ValueError, 'translator dependency limit exceeded:'):
                    batch.translator_dependencies(launcher)
                self.assertFalse(batch.unchanged(binding))
            with patch.object(batch, 'TRANSLATOR_BYTE_LIMIT', 1):
                with self.assertRaisesRegex(ValueError, 'translator dependency limit exceeded:'):
                    batch.translator_dependencies(launcher)
            added = module.parent/'new.py'
            added.write_text('new module')
            self.assertFalse(batch.unchanged(binding))
            added.unlink()
            self.assertTrue(batch.unchanged(binding))
            module.unlink()
            self.assertFalse(batch.unchanged(binding))

    def test_worker_caps_resume_and_census_rejection(self):
        for count in (1, 4):
            with self.subTest(workers=count), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp); out = root / 'build/run'
                inp = root / 'input'; inp.write_text('input')
                self.tool_files(root)
                m = dict(schema='rel_first_compile_batch/v1', root=str(root), proof_root=str(root), m2c=str(inp), objdiff=str(inp), context_path=str(inp), prefix_path=str(inp), flags=[], output_root=str(out), modules=[dict(name='m', target_path=str(inp), assembly_paths=[str(inp)], functions=[str(i) for i in range(8)])])
                mp = root / 'manifest.json'; mp.write_text(json.dumps(m))
                lock = threading.Lock(); active = 0; peak = 0; calls = []; slots = set()
                def fake(manifest, module, name, worker, binding):
                    nonlocal active, peak
                    with lock:
                        active += 1; peak = max(peak, active); calls.append(name); slots.add(worker)
                    time.sleep(.02)
                    with lock: active -= 1
                    scratch = out / ('worker-' + str(worker))
                    return dict(module='m', function=name, compile_exitcode=1), scratch/'candidate.c', scratch/'candidate.o'
                with patch.object(batch, 'attempt', side_effect=fake), contextlib.redirect_stdout(io.StringIO()):
                    with patch.object(sys, 'argv', ['batch', str(mp), '--workers', str(count)]): batch.main()
                    self.assertEqual(peak, count)
                    self.assertEqual(slots, set(range(count)))
                    self.assertEqual(sorted(calls), [str(i) for i in range(8)])
                    calls.clear()
                    with patch.object(sys, 'argv', ['batch', str(mp), '--resume', '--workers', str(4 if count == 1 else 1)]): batch.main()
                    self.assertEqual(calls, [])
                    census = out / 'census.jsonl'; original = census.read_text()
                    for bad in (json.loads(original.splitlines()[0]), dict(module='unknown', function='x')):
                        census.write_text(original + json.dumps(bad) + '\n')
                        with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), self.assertRaises(ValueError): batch.main()
                        self.assertEqual(calls, [])
                    self.assertNotIn('workers', json.loads((out / 'bindings.json').read_text()))

    def test_frozen_job_watch_excludes_other_modules(self):
        m = dict(context_path='ctx', prefix_path='prefix', source_watch_sha256={'header': 'h', 'other.plf': 'p'})
        mod = dict(target_path='mine.plf', assembly_paths=['mine.s'], header_watch_paths=['header'])
        self.assertIn('other.plf', batch.job_watch_paths(m, mod))
        m['frozen_input_schema'] = 'rel_frozen_inputs/v1'
        self.assertEqual(batch.job_watch_paths(m, mod), ['ctx', 'header', 'mine.plf', 'mine.s', 'prefix'])
        mod['header_watch_paths'] = ['unbound']
        with self.assertRaises(ValueError): batch.job_watch_paths(m, mod)

    def test_parser_exact_neutral_missing(self):
        report = {'left': {'symbols': [{'name': 'f', 'target_symbol': 0, 'size': 4, 'match_percent': 100, 'instructions': [{'diff_kind': 'DIFF_NONE'}]}]}, 'right': {'symbols': [{'size': 4}]}}
        row = batch.parse_report(report, 'f')
        self.assertEqual((row['score'], row['size'], row['diffcount']), (100, 4, 0))
        report['left']['symbols'][0].update(match_percent=50, instructions=[{'diff_kind': 'DIFF_REPLACE'}])
        self.assertEqual(batch.parse_report(report, 'f')['diffcount'], 1)
        with self.assertRaises(ValueError):
            batch.parse_report({}, 'f')

    def test_placeholder_exclusions(self):
        exact = dict(score=100, compile_exitcode=0, m2c_exitcode=0, objdiff_exitcode=0, syntax_placeholders=[], decompile_errors=False)
        self.assertTrue(batch.eligible(exact))
        for token in ('M2C_ERROR', 'M2C_custom', 'GLUE_F64', 'CLZ', 'MULT_HI'):
            self.assertFalse(batch.eligible({**exact, 'syntax_placeholders': batch.placeholders(token)}))
        self.assertFalse(batch.eligible({**exact, 'decompile_errors': True}))
        self.assertFalse(batch.eligible({**exact, 'compile_exitcode': 1}))

    def test_binding_drift_and_cleanup(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            p = root / 'input'; p.write_text('a')
            self.tool_files(root)
            manifest = dict(context_path=str(p), prefix_path=str(p), modules=[],
                            proof_root=str(root), m2c=str(p), objdiff=str(p))
            mp = root / 'manifest'; mp.write_text(json.dumps(manifest))
            b = batch.bindings(manifest, mp)
            self.assertTrue(batch.unchanged(b))
            p.write_text('b')
            self.assertFalse(batch.unchanged(b))
            self.assertNotEqual(b, batch.bindings(manifest, mp))
            scratch = root / 'worker-0'; scratch.mkdir()
            (scratch / 'old.o').write_bytes(b'old')
            batch.clean_scratch(scratch)
            self.assertEqual(list(scratch.iterdir()), [])

    def test_failed_m2c_records_without_compile(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inp = root / 'input'; inp.write_text('prefix')
            module = dict(name='m', target_path=str(inp), assembly_paths=[str(inp)])
            manifest = dict(output_root=str(root), prefix_path=str(inp), context_path=str(inp), m2c=str(inp), proof_root=str(root))
            binding = {'files': {str(inp): batch.sha(inp)}}
            def fake(argv, cwd, scratch, *args):
                (scratch / 'stdout.log').write_text('Error occurred M2C_ERROR')
                return 1, '', 'synthetic failure'
            with patch.object(batch, 'run_process', side_effect=fake) as run:
                row, src, obj = batch.attempt(manifest, module, 'f', 0, binding)
            self.assertEqual(run.call_count, 1)
            self.assertEqual(row['m2c_exitcode'], 1)
            self.assertIsNone(row['object_sha256'])
            self.assertTrue(row['decompile_errors'])
            self.assertEqual(row['source_sha256'], batch.sha(src))
            self.assertFalse((root / 'worker-0/report.json').exists())

    def test_missing_object_is_recorded(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inp = root / 'input'; inp.write_text('')
            module = dict(name='m', target_path=str(inp), assembly_paths=[str(inp)])
            manifest = dict(output_root=str(root), prefix_path=str(inp), context_path=str(inp), m2c=str(inp), proof_root=str(root), flags=[])
            def fake(argv, cwd, scratch, *args):
                (scratch / 'stdout.log').write_text('void f(void) {}')
                return 0, '', ''
            with patch.object(batch, 'run_process', side_effect=fake) as run:
                row, _, _ = batch.attempt(manifest, module, 'f', 0, {'files': {str(inp): batch.sha(inp)}})
            self.assertEqual(run.call_count, 2)
            self.assertIn('object missing', row['diagnostic'])
            self.assertIsNone(row['objdiff_exitcode'])
            self.assertFalse(batch.eligible(row))

    def test_group_attempt_one_compile_shared_artifacts_and_order(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inp = root / 'input'
            inp.write_text('prefix')
            m2c = root / 'm2c.py'
            m2c.write_text('translator')
            objdiff = root / 'objdiff.exe'
            objdiff.write_text('diff')
            module = dict(name='m', target_path=str(inp), assembly_paths=[str(inp)], functions=['a', 'b'])
            manifest = dict(output_root=str(root), prefix_path=str(inp), context_path=str(inp),
                            m2c=str(m2c), objdiff=str(objdiff), proof_root=str(root), flags=[])
            binding = {'files': {str(inp): batch.sha(inp)}}
            calls = []

            def fake(argv, cwd, scratch, *args):
                argv = list(map(str, argv))
                calls.append(argv)
                if '-f' in argv:
                    (scratch / 'stdout.log').write_text('void a(void) {} void b(void) {}')
                    return 0, '', ''
                if 'mwcceppc' in ' '.join(argv):
                    (scratch / 'candidate.o').write_bytes(b'obj')
                    return 0, '', ''
                if 'objdiff' in argv[0]:
                    report = {'left': {'symbols': [
                        {'name': 'a', 'target_symbol': 0, 'size': 4, 'match_percent': 100,
                         'instructions': [{'diff_kind': 'DIFF_NONE'}]},
                        {'name': 'b', 'target_symbol': 1, 'size': 8, 'match_percent': 50,
                         'instructions': [{'diff_kind': 'DIFF_REPLACE'}]}]},
                       'right': {'symbols': [{'size': 4}, {'size': 8}]}}
                    (scratch / 'report.json').write_text(json.dumps(report))
                    return 0, '', ''
                raise AssertionError('unexpected process: ' + repr(argv))

            def f_options(argv):
                found = []
                i = 0
                while i < len(argv) - 1:
                    if argv[i] == '-f':
                        found.append(argv[i + 1])
                        i += 2
                    else:
                        i += 1
                return found

            with patch.object(batch, 'run_process', side_effect=fake) as run:
                rows, src, obj = batch.attempt_group(manifest, module, ['a', 'b'], 'g', 0, binding)
            self.assertEqual(run.call_count, 3)
            self.assertEqual(f_options(calls[0]), ['a', 'b'])
            self.assertEqual([row['function'] for row in rows], ['a', 'b'])
            self.assertEqual([row['score'] for row in rows], [100, 50])
            self.assertEqual([row['diffcount'] for row in rows], [0, 1])
            self.assertTrue(batch.eligible(rows[0]))
            self.assertFalse(batch.eligible(rows[1]))
            self.assertEqual(rows[0]['source_sha256'], rows[1]['source_sha256'])
            self.assertEqual(rows[0]['object_sha256'], rows[1]['object_sha256'])
            self.assertNotIn(None, (rows[0]['source_sha256'], rows[0]['object_sha256']))
            self.assertEqual(rows[0]['translation_group'], 'g')
            self.assertEqual(rows[1]['translation_group'], 'g')

            calls.clear()
            with patch.object(batch, 'run_process', side_effect=fake) as run:
                singleton_rows, _, _ = batch.attempt_group(manifest, module, ['a'], 's', 0, binding)
            self.assertEqual(run.call_count, 3)
            self.assertEqual(f_options(calls[0]), ['a'])
            self.assertEqual(singleton_rows[0]['translation_group'], 's')

            calls.clear()
            with patch.object(batch, 'run_process', side_effect=fake) as run:
                ungrouped_row, _, _ = batch.attempt(manifest, module, 'a', 0, binding)
            self.assertEqual(run.call_count, 3)
            self.assertIsNone(ungrouped_row['translation_group'])

    def test_group_validation_rejects_before_processes(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inp = root / 'input'
            inp.write_text('x')
            cases = [
                [{'name': 'g', 'functions': ['a']}, {'name': 'g', 'functions': ['b']}],
                [{'name': 'g1', 'functions': ['a']}, {'name': 'g2', 'functions': ['a']}],
                [{'name': 'g', 'functions': ['x']}],
                [{'name': '1bad', 'functions': ['a']}],
                [{'name': 'g', 'functions': []}],
                [{'name': 'g', 'functions': ['a', 'a']}],
            ]
            with patch.object(batch, 'bindings', side_effect=AssertionError('binding ran')) as bind, \
                 patch.object(batch, 'run_process') as run, \
                 patch.object(batch, 'attempt') as attempt, \
                 patch.object(batch, 'attempt_group') as attempt_group:
                for groups in cases:
                    with self.subTest(groups=groups):
                        manifest = dict(schema='rel_first_compile_batch/v1', root=str(root), proof_root=str(root),
                                       m2c=str(inp), objdiff=str(inp), context_path=str(inp), prefix_path=str(inp),
                                       output_root=str(root / 'build' / 'run'),
                                       modules=[dict(name='m', target_path=str(inp), assembly_paths=[str(inp)],
                                                     functions=['a', 'b'], translation_groups=groups)])
                        mp = root / 'manifest.json'
                        mp.write_text(json.dumps(manifest))
                        with patch.object(sys, 'argv', ['batch', str(mp)]), \
                                contextlib.redirect_stdout(io.StringIO()), \
                                self.assertRaises(ValueError):
                            batch.main()
                        bind.assert_not_called()
                        run.assert_not_called()
                        attempt.assert_not_called()
                        attempt_group.assert_not_called()
                        bind.reset_mock()
                        run.reset_mock()
                        attempt.reset_mock()
                        attempt_group.reset_mock()

    def test_group_resume_all_and_partial(self):
        for mode in ('all', 'partial'):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                inp = root / 'input'
                inp.write_text('prefix')
                m2c = root / 'm2c.py'
                m2c.write_text('t')
                objdiff = root / 'objdiff.exe'
                objdiff.write_text('d')
                self.tool_files(root)
                out = root / 'build' / 'run'
                out.mkdir(parents=True, exist_ok=True)
                module = dict(name='m', target_path=str(inp), assembly_paths=[str(inp)], functions=['a', 'b'],
                              translation_groups=[{'name': 'g', 'functions': ['a', 'b']}])
                manifest = dict(schema='rel_first_compile_batch/v1', root=str(root), proof_root=str(root),
                                m2c=str(m2c), objdiff=str(objdiff), context_path=str(inp), prefix_path=str(inp),
                                output_root=str(out), modules=[module])
                mp = root / 'manifest.json'
                mp.write_text(json.dumps(manifest))
                calls = []

                def fake_group(manifest_arg, module_arg, functions_arg, group_arg, worker_arg, binding_arg):
                    calls.append((group_arg, list(functions_arg), worker_arg))
                    rows = []
                    for fn in functions_arg:
                        rows.append(dict(module='m', function=fn, translation_group=group_arg,
                                         m2c_exitcode=0, compile_exitcode=1, objdiff_exitcode=None,
                                         score=None, size=None, diffcount=None, diagnostic='synthetic',
                                         syntax_placeholders=[], decompile_errors=False,
                                         source_sha256=None, object_sha256=None))
                    return rows, out / 'worker-0' / 'candidate.c', out / 'worker-0' / 'candidate.o'

                census = out / 'census.jsonl'
                if mode == 'all':
                    with patch.object(batch, 'attempt_group', side_effect=fake_group):
                        with patch.object(sys, 'argv', ['batch', str(mp)]), contextlib.redirect_stdout(io.StringIO()):
                            batch.main()
                        self.assertEqual(calls, [('g', ['a', 'b'], 0)])
                        self.assertEqual(len(census.read_text(encoding='utf-8').splitlines()), 2)
                        calls.clear()
                        with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), contextlib.redirect_stdout(io.StringIO()):
                            batch.main()
                    self.assertEqual(calls, [])
                    self.assertEqual(len(census.read_text(encoding='utf-8').splitlines()), 2)
                else:
                    binding = batch.bindings(manifest, mp)
                    batch.atomic(out / 'bindings.json', binding)
                    row_a = dict(module='m', function='a', translation_group='g',
                                 m2c_exitcode=0, compile_exitcode=0, objdiff_exitcode=0,
                                 score=100, size=4, diffcount=0, diagnostic='',
                                 syntax_placeholders=[], decompile_errors=False,
                                 source_sha256=None, object_sha256=None)
                    census.write_text(json.dumps(row_a) + '\n', encoding='utf-8')
                    with patch.object(batch, 'attempt_group', side_effect=fake_group):
                        with patch.object(sys, 'argv', ['batch', str(mp), '--resume']), contextlib.redirect_stdout(io.StringIO()):
                            batch.main()
                    self.assertEqual(calls, [('g', ['a', 'b'], 0)])
                    lines = [json.loads(line) for line in census.read_text(encoding='utf-8').splitlines()]
                    self.assertEqual(len(lines), 2)
                    self.assertEqual(sum(1 for row in lines if row['function'] == 'a'), 1)
                    self.assertEqual(sum(1 for row in lines if row['function'] == 'b'), 1)

    def test_partial_group_retention_reuses_storage_at_the_limit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            retained = root / 'retained'; retained.mkdir()
            source, obj = root / 'input.c', root / 'input.o'
            source.write_bytes(b'source'); obj.write_bytes(b'object')
            def row(name):
                return dict(module='m', function=name, translation_group='g',
                            m2c_exitcode=0, compile_exitcode=0, objdiff_exitcode=0,
                            score=100, syntax_placeholders=[], decompile_errors=False)
            a, b = row('a'), row('b')
            with patch.object(batch, 'LIMIT', 12):
                count = batch.retain_rows([a], source, obj, retained, root, 0)
                self.assertEqual(count, 12)
                self.assertEqual(batch.retain_rows([b], source, obj, retained, root, count), 12)
            self.assertEqual(a['retained_source'], b['retained_source'])
            self.assertEqual(a['retained_object'], b['retained_object'])
            self.assertEqual(len(list(retained.iterdir())), 2)
            obj.write_bytes(b'drift!')
            with self.assertRaisesRegex(ValueError, 'immutable retained artifact collision'):
                batch.retain_rows([row('b')], source, obj, retained, root, 12)

if __name__ == '__main__':
    unittest.main()
