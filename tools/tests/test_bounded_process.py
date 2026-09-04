import os
from pathlib import Path
import sys
import tempfile
import threading
import time
import unittest
from tools import bounded_process as bp
from tools import owner_campaign as campaign
from tools import owner_campaign_measure as measure


class BoundedProcessTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def run_child(self, code, **kwargs):
        return bp.run([sys.executable, '-c', code], cwd=self.root, timeout=3, **kwargs)

    def test_drains_stdout_and_stderr_over_pipe_capacity(self):
        result = self.run_child('import sys; sys.stdout.write("x"*131072); sys.stderr.write("y"*131072)')
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, b'x'*131072)
        self.assertEqual(result.stderr, b'y'*131072)

    def test_live_shared_output_budget(self):
        start = time.monotonic()
        with self.assertRaises(bp.ProcessLimitError) as caught:
            self.run_child('import sys,time; sys.stdout.write("x"*131072); sys.stdout.flush(); time.sleep(20)', max_output=32768)
        self.assertIn('output exceeded', str(caught.exception))
        self.assertLessEqual(len(caught.exception.stdout)+len(caught.exception.stderr), 32768)
        self.assertLess(time.monotonic()-start, 3)

    def test_nonzero_diagnostic_preserved(self):
        result = self.run_child('import sys; sys.stderr.write("compiler error"); sys.exit(2)')
        self.assertEqual((result.returncode, result.stderr), (2, b'compiler error'))

    def test_deadline_reaps_readers(self):
        with self.assertRaises(bp.ProcessLimitError):
            bp.run([sys.executable, '-c', 'import time; time.sleep(20)'], cwd=self.root, timeout=.2)
        self.assertFalse(any(t.name.startswith('recovery-pipe-') for t in threading.enumerate()))

    def test_launcher_exit_still_kills_pipe_holding_descendant(self):
        code = 'import subprocess,sys; subprocess.Popen([sys.executable,"-c","import time; time.sleep(20)"])'
        with self.assertRaises(bp.ProcessLimitError):
            bp.run([sys.executable, '-c', code], cwd=self.root, timeout=.3)
        self.assertFalse(any(t.name.startswith('recovery-pipe-') for t in threading.enumerate()))

    def test_constraint_exception_survives(self):
        def fail():
            raise RuntimeError('storage sentinel')
        with self.assertRaisesRegex(RuntimeError, 'storage sentinel'):
            self.run_child('import time; time.sleep(20)', check=fail)

    def test_campaign_regression_large_output_does_not_deadlock(self):
        result = campaign._run_bounded_process(
            [sys.executable, '-c', 'import sys; sys.stdout.write("x"*131072)'],
            cwd=self.root, environment=dict(os.environ), timeout=3, scratch=self.root,
            temporary_root=self.root, scratch_hard_bytes=1000000, cell_temporary_bytes=1000000)
        self.assertEqual(len(result.stdout), 131072)

    def test_measurement_uses_live_limit(self):
        from unittest import mock
        with mock.patch.object(measure, 'MAX_OUTPUT', 1024), self.assertRaisesRegex(measure.MeasurementError, 'output exceeded'):
            measure._run_bounded([sys.executable, '-c', 'print("x"*131072)'],
                                 cwd=self.root, deadline=measure.Deadline(3), label='test')
