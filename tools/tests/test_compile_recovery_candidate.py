from contextlib import redirect_stderr, redirect_stdout
import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock
from tools import compile_recovery_candidate as cc


class CandidateCompileTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.scratch = self.root / 'build/scratch'
        for path in ('src', 'include', 'build/GP6E01/include'):
            (self.root/path).mkdir(parents=True, exist_ok=True)
            (self.scratch/path).mkdir(parents=True, exist_ok=True)
        self.source = self.root/'build/candidate.c'
        self.source.write_bytes(b'candidate')
        (self.root/'src/test.c').write_bytes(b'live')
        (self.scratch/'src/test.c').write_bytes(b'original scratch')
        self.output = self.root/'build/candidate.o'
        self.command_json = self.root/'build/compiler-command.json'
        self.compile_options = ['--source', 'build/candidate.c', '--output', 'build/candidate.o']

    def cli(self, *options):
        argv = ['compile_recovery_candidate.py', '--root', str(self.root),
                '--scratch', 'build/scratch', '--source-relpath', 'src/test.c',
                '--object-relpath', 'build/test.o',
                '--mutex-name', 'Local\\RecoveryFixture'+self.root.name, *options]
        stdout, stderr = io.StringIO(), io.StringIO()
        with mock.patch.object(sys, 'argv', argv), redirect_stdout(stdout), redirect_stderr(stderr):
            result = cc.main()
        return result, stdout.getvalue(), stderr.getvalue()

    def snapshot(self):
        return {p.relative_to(self.root).as_posix(): p.read_bytes() if p.is_file() else None
                for p in self.root.rglob('*')}

    def seed_products(self):
        for path in (self.output, self.output.with_suffix('.o.receipt.json'),
                     self.output.with_suffix('.o.stdout.log'),
                     self.output.with_suffix('.o.stderr.log'), self.scratch/'build/test.o'):
            path.write_bytes(b'previous '+path.name.encode())

    def write_command(self, command):
        self.command_json.write_text(json.dumps(command), encoding='utf-8')

    def mocked_compile(self, command, *, cwd, timeout):
        self.assertIsInstance(command, list)
        self.assertEqual(cwd, self.scratch)
        self.assertEqual((cwd/'src/test.c').read_bytes(), b'candidate')
        (cwd/'build/test.o').write_bytes(b'mocked object')
        return subprocess.CompletedProcess(command, 0, stdout=b'compiler output', stderr=b'')

    def compile(self, code, **extra):
        return cc.compile_candidate(root=self.root, scratch=self.scratch, source=self.source,
            output=self.output, source_relpath='src/test.c', object_relpath='build/test.o',
            command=[sys.executable, '-c', code], tools=[Path(sys.executable)],
            mutex_name='Local\\RecoveryFixture'+self.root.name, **extra)

    def test_success_binds_object_and_restores_scratch(self):
        result = self.compile('from pathlib import Path; Path("build/test.o").write_bytes(Path("src/test.c").read_bytes())')
        self.assertEqual(result['object_sha256'], cc.digest(self.output))
        self.assertEqual(result['source_sha256'], cc.digest(self.source))
        self.assertEqual(self.output.read_bytes(), b'candidate')
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')
        self.assertEqual((self.root/'src/test.c').read_bytes(), b'live')
        self.assertTrue(self.output.with_suffix('.o.receipt.json').is_file())

    def test_failure_never_reuses_previous_output(self):
        self.output.write_bytes(b'stale')
        with self.assertRaisesRegex(RuntimeError, 'bad compile'):
            self.compile('import sys; sys.stderr.write("bad compile"); sys.exit(2)')
        self.assertFalse(self.output.exists())
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')

    def test_timeout_restores_scratch(self):
        with self.assertRaisesRegex(RuntimeError, 'timed out'):
            self.compile('import time; time.sleep(20)', timeout=.2)
        self.assertFalse(self.output.exists())
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')

    def test_stale_headers_reject_before_compile(self):
        (self.root/'include/test.h').write_bytes(b'new interface')
        with self.assertRaisesRegex(ValueError, 'scratch headers differ'):
            self.compile('raise Exception("must not run")')

    def test_missing_object_is_failure(self):
        with self.assertRaisesRegex(RuntimeError, 'nonempty object'):
            self.compile('pass')

    def test_output_cannot_escape_build_or_replace_source(self):
        self.output = self.root/'src/test.c'
        with self.assertRaises(ValueError):
            self.compile('pass')

    def test_command_json_malformed_is_mutation_free(self):
        self.command_json.write_text('[not valid JSON', encoding='utf-8')
        self.seed_products()
        before = self.snapshot()
        with mock.patch.object(cc.bounded_process, 'run') as run:
            result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                         *self.compile_options)
        self.assertEqual(result, 2)
        self.assertIn(str(self.command_json), stderr)
        run.assert_not_called()
        self.assertEqual(self.snapshot(), before)

    def test_command_json_invalid_shape_is_mutation_free(self):
        invalid_commands = [None, True, 7, 'compiler', {'argv': [sys.executable]}, [],
                            ['', '-c', 'pass'], [sys.executable, ''], [sys.executable, '   '],
                            [sys.executable, None], [sys.executable, 7],
                            [sys.executable, False], [sys.executable, ['nested']],
                            [sys.executable, 'bad\x00argument'],
                            [sys.executable, 'bad\nargument'],
                            [sys.executable, 'bad\rargument']]
        self.seed_products()
        for command in invalid_commands:
            with self.subTest(command=command):
                self.write_command(command)
                before = self.snapshot()
                with mock.patch.object(cc.bounded_process, 'run') as run:
                    result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                                 *self.compile_options)
                self.assertEqual(result, 2)
                self.assertIn(str(self.command_json), stderr)
                run.assert_not_called()
                self.assertEqual(self.snapshot(), before)

    def test_command_json_missing_executable_is_mutation_free(self):
        executable = self.scratch/'missing-compiler.exe'
        self.write_command([str(executable), '-pragma', 'cats off'])
        self.seed_products()
        before = self.snapshot()
        with mock.patch.object(cc.bounded_process, 'run') as run:
            result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                         *self.compile_options)
        self.assertEqual(result, 2)
        self.assertIn(str(executable), stderr)
        run.assert_not_called()
        self.assertEqual(self.snapshot(), before)

    def test_command_json_missing_declared_tool_is_mutation_free(self):
        self.write_command([sys.executable, '-pragma', 'cats off'])
        self.seed_products()
        before = self.snapshot()
        with mock.patch.object(cc.bounded_process, 'run') as run:
            result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                         '--tool', 'build/missing-tool.exe', *self.compile_options)
        self.assertEqual(result, 2)
        self.assertIn(str(self.root/'build/missing-tool.exe'), stderr)
        run.assert_not_called()
        self.assertEqual(self.snapshot(), before)

    def test_command_json_preserves_pragma_argument_and_binds_descriptor(self):
        command = [sys.executable, '-pragma', 'cats off', '-c', 'src/test.c',
                   '-o', 'build/test.o']
        self.write_command(command)
        with mock.patch.object(cc.bounded_process, 'run', side_effect=self.mocked_compile) as run:
            result, stdout, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                             *self.compile_options)
        self.assertEqual(result, 0, stderr)
        receipt = json.loads(stdout)
        resolved_command = [str(Path(sys.executable).resolve()), *command[1:]]
        run.assert_called_once_with(resolved_command, cwd=self.scratch, timeout=120)
        self.assertEqual(receipt['command'], resolved_command)
        self.assertEqual(receipt['command_descriptor'],
                         {'path': str(self.command_json.resolve()), 'sha256': cc.digest(self.command_json)})
        self.assertEqual(receipt['tools'][resolved_command[0]], cc.digest(Path(sys.executable)))
        self.assertEqual(self.output.read_bytes(), b'mocked object')
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')
        self.assertEqual((self.root/'src/test.c').read_bytes(), b'live')
        self.assertEqual(json.loads(self.output.with_suffix('.o.receipt.json').read_text(encoding='utf-8')),
                         receipt)

    def test_command_json_preflight_matches_compile_context_without_mutation(self):
        self.write_command([sys.executable, '-pragma', 'cats off'])
        declared_tool = self.root/'build/declared-tool.bin'
        declared_tool.write_bytes(b'compiler dependency')
        self.seed_products()
        options = ['--command-json', 'build/compiler-command.json', '--tool', 'build/declared-tool.bin']
        before = self.snapshot()
        with mock.patch.object(cc.bounded_process, 'run') as run:
            result, stdout, stderr = self.cli(*options, '--preflight')
        self.assertEqual(result, 0, stderr)
        preflight = json.loads(stdout)
        run.assert_not_called()
        self.assertEqual(self.snapshot(), before)
        self.assertEqual(preflight['context']['command_descriptor'],
                         {'path': str(self.command_json.resolve()), 'sha256': cc.digest(self.command_json)})
        self.assertEqual(preflight['context']['tools'][str(declared_tool.resolve())], cc.digest(declared_tool))
        expected_hash = hashlib.sha256(json.dumps(preflight['context'], sort_keys=True).encode()).hexdigest()
        self.assertEqual(preflight['context_sha256'], expected_hash)
        with mock.patch.object(cc.bounded_process, 'run', side_effect=self.mocked_compile):
            result, stdout, stderr = self.cli(*options, *self.compile_options)
        self.assertEqual(result, 0, stderr)
        receipt = json.loads(stdout)
        self.assertEqual(receipt['context_sha256'], preflight['context_sha256'])
        self.assertEqual(receipt['command'], preflight['context']['command'])
        self.assertEqual(receipt['tools'], preflight['context']['tools'])
        self.assertEqual(receipt['command_descriptor'], preflight['context']['command_descriptor'])

    def test_compiler_script_cli_remains_supported(self):
        script = self.root/'build/compiler.ps1'
        script.write_text('# mocked compiler script\n', encoding='utf-8')
        with mock.patch.object(cc.shutil, 'which', return_value=sys.executable), \
                mock.patch.object(cc.bounded_process, 'run', side_effect=self.mocked_compile) as run:
            result, stdout, stderr = self.cli('--compiler-script', 'build/compiler.ps1',
                                             *self.compile_options)
        self.assertEqual(result, 0, stderr)
        receipt = json.loads(stdout)
        command = [str(Path(sys.executable).resolve()), '-NoProfile', '-ExecutionPolicy',
                   'Bypass', '-File', str(script)]
        run.assert_called_once_with(command, cwd=self.scratch, timeout=120)
        self.assertEqual(receipt['command'], command)
        self.assertEqual(receipt['tools'][str(script.resolve())], cc.digest(script))
        self.assertIsNone(receipt['command_descriptor'])

    def test_compiler_script_and_command_json_are_mutually_exclusive(self):
        self.write_command([sys.executable])
        self.seed_products()
        before = self.snapshot()
        with mock.patch.object(cc.bounded_process, 'run') as run:
            with self.assertRaises(SystemExit) as error:
                self.cli('--compiler-script', 'build/compiler.ps1',
                         '--command-json', 'build/compiler-command.json', *self.compile_options)
        self.assertEqual(error.exception.code, 2)
        run.assert_not_called()
        self.assertEqual(self.snapshot(), before)

    def test_command_json_rejects_batch_executable_without_mutation(self):
        self.seed_products()
        for suffix in ('.cmd', '.bat'):
            with self.subTest(suffix=suffix):
                executable = self.scratch/('compiler'+suffix)
                executable.write_text('@echo off\n', encoding='utf-8')
                self.write_command([str(executable), '-pragma', 'cats off'])
                before = self.snapshot()
                with mock.patch.object(cc.bounded_process, 'run') as run:
                    result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                                 *self.compile_options)
                self.assertEqual(result, 2)
                self.assertIn(str(executable), stderr)
                run.assert_not_called()
                self.assertEqual(self.snapshot(), before)

    def test_raw_api_invalid_argv_rejects_before_lock_or_mutation(self):
        self.seed_products()
        for command in (None, 'compiler -pragma cats off', [], [sys.executable, ''],
                        [sys.executable, 7], [sys.executable, 'bad\x00argument']):
            with self.subTest(command=command):
                before = self.snapshot()
                with mock.patch.object(cc, 'compiler_lock') as lock, \
                        mock.patch.object(cc.bounded_process, 'run') as run:
                    with self.assertRaises(ValueError):
                        cc.compile_candidate(root=self.root, scratch=self.scratch, source=self.source,
                            output=self.output, source_relpath='src/test.c', object_relpath='build/test.o',
                            command=command, tools=[])
                lock.assert_not_called()
                run.assert_not_called()
                self.assertEqual(self.snapshot(), before)

    def test_command_json_change_during_compile_rejects_receipt_and_restores_scratch(self):
        self.write_command([sys.executable, '-pragma', 'cats off'])
        self.seed_products()

        def changing_compile(command, *, cwd, timeout):
            result = self.mocked_compile(command, cwd=cwd, timeout=timeout)
            self.write_command([sys.executable, '-pragma', 'cats on'])
            return result

        with mock.patch.object(cc.bounded_process, 'run', side_effect=changing_compile) as run:
            result, _, stderr = self.cli('--command-json', 'build/compiler-command.json',
                                         *self.compile_options)
        self.assertEqual(result, 2)
        self.assertIn('command JSON changed during compilation', stderr)
        self.assertIn(str(self.command_json), stderr)
        run.assert_called_once()
        self.assertFalse(self.output.exists())
        self.assertFalse(self.output.with_suffix('.o.receipt.json').exists())
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')
        self.assertEqual(self.source.read_bytes(), b'candidate')
        self.assertEqual((self.root/'src/test.c').read_bytes(), b'live')

    def test_failed_compile_preserves_both_diagnostic_streams(self):
        self.seed_products()
        with self.assertRaises(RuntimeError) as error:
            self.compile('import sys; sys.stdout.write("compile stdout detail"); '
                         'sys.stderr.write("compile stderr detail"); sys.exit(2)')
        self.assertIn('stdout:\ncompile stdout detail', str(error.exception))
        self.assertIn('stderr:\ncompile stderr detail', str(error.exception))
        self.assertEqual(self.output.with_suffix('.o.stdout.log').read_bytes(), b'compile stdout detail')
        self.assertEqual(self.output.with_suffix('.o.stderr.log').read_bytes(), b'compile stderr detail')
        self.assertFalse(self.output.exists())
        self.assertFalse(self.output.with_suffix('.o.receipt.json').exists())
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')
        self.assertEqual((self.root/'src/test.c').read_bytes(), b'live')

    def test_timed_out_compile_preserves_both_diagnostic_streams(self):
        self.seed_products()
        failure = cc.bounded_process.ProcessLimitError('command timed out after 1 seconds',
                                                       b'partial stdout', b'partial stderr')
        with mock.patch.object(cc.bounded_process, 'run', side_effect=failure) as run:
            with self.assertRaisesRegex(RuntimeError, 'timed out') as error:
                self.compile('pass')
        run.assert_called_once()
        self.assertIn('stdout:\npartial stdout', str(error.exception))
        self.assertIn('stderr:\npartial stderr', str(error.exception))
        self.assertEqual(self.output.with_suffix('.o.stdout.log').read_bytes(), b'partial stdout')
        self.assertEqual(self.output.with_suffix('.o.stderr.log').read_bytes(), b'partial stderr')
        self.assertFalse(self.output.exists())
        self.assertFalse(self.output.with_suffix('.o.receipt.json').exists())
        self.assertEqual((self.scratch/'src/test.c').read_bytes(), b'original scratch')
        self.assertEqual((self.root/'src/test.c').read_bytes(), b'live')
