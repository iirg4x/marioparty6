from pathlib import Path
import sys
import tempfile
import unittest
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
