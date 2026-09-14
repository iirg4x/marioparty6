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
                        '--valid-syntax', '--force-decimal', '--context', str(inp), '-f', 'f', str(inp)]
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

if __name__ == '__main__':
    unittest.main()
