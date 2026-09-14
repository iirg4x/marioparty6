import os
from pathlib import Path
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock
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

    def test_windows_job_waits_for_asynchronous_active_count(self):
        job = object.__new__(bp._WindowsJob)
        job.handle = 123
        with mock.patch.object(job, 'active_processes', side_effect=[2, 1, 0]) as active, \
                mock.patch.object(bp.time, 'monotonic', side_effect=[10, 10.01, 10.02]), \
                mock.patch.object(bp.time, 'sleep') as sleep:
            job.wait_empty(timeout=1)
        self.assertEqual(active.call_count, 3)
        self.assertEqual(sleep.call_args_list, [mock.call(.01), mock.call(.01)])
        self.assertEqual(job.handle, 123)

    def test_windows_job_drain_timeout_is_bounded_and_keeps_handle(self):
        job = object.__new__(bp._WindowsJob)
        job.handle = 123
        with mock.patch.object(job, 'active_processes', return_value=2), \
                mock.patch.object(bp.time, 'monotonic', side_effect=[10, 10.5, 11]), \
                mock.patch.object(bp.time, 'sleep') as sleep:
            with self.assertRaisesRegex(bp.ProcessLimitError, '2 active processes'):
                job.wait_empty(timeout=1)
        sleep.assert_called_once_with(.01)
        self.assertEqual(job.handle, 123)

    @unittest.skipUnless(os.name == 'nt', 'Windows job cleanup')
    def test_windows_job_drain_failure_preserves_primary_exception(self):
        for diagnostic in (OSError('query sentinel'), bp.ProcessLimitError('drain timeout sentinel')):
            with self.subTest(diagnostic=diagnostic):
                primary = RuntimeError('storage sentinel')
                def fail():
                    raise primary
                close = bp._WindowsJob.close
                with mock.patch.object(bp._WindowsJob, 'wait_empty', side_effect=diagnostic), \
                        mock.patch.object(bp._WindowsJob, 'close', autospec=True, side_effect=close) as closed:
                    with self.assertRaises(RuntimeError) as caught:
                        self.run_child('import time; time.sleep(20)', check=fail)
                self.assertIs(caught.exception, primary)
                self.assertTrue(any(str(diagnostic) in note for note in primary.__notes__))
                closed.assert_called_once()

    @unittest.skipUnless(os.name == 'nt', 'Windows job cleanup')
    def test_windows_job_drain_failure_on_success_is_reported(self):
        with mock.patch.object(bp._WindowsJob, 'wait_empty', side_effect=OSError('query sentinel')):
            with self.assertRaisesRegex(bp.ProcessLimitError, 'Windows job drain failed: query sentinel'):
                self.run_child('pass')

    @unittest.skipUnless(os.name == 'nt', 'Windows descendant cwd release')
    def test_windows_descendant_cwd_released_repeatedly(self):
        child = 'from pathlib import Path; import time; Path("ready").touch(); time.sleep(20)'
        code = f'import subprocess,sys; subprocess.Popen([sys.executable,"-c",{child!r}])'
        for iteration in range(10):
            with self.subTest(iteration=iteration), tempfile.TemporaryDirectory(dir=self.root) as directory:
                cwd = Path(directory)
                def child_ready():
                    if (cwd/'ready').exists():
                        raise RuntimeError('descendant is ready')
                with self.assertRaisesRegex(RuntimeError, 'descendant is ready'):
                    bp.run([sys.executable, '-c', code], cwd=cwd, timeout=3, check=child_ready)
                self.assertFalse(any(t.name.startswith('recovery-pipe-') for t in threading.enumerate()))
            # Each context above removes the child's actual cwd immediately,
            # without retries, sleeps, or deferred tempfile cleanup.

    @unittest.skipUnless(os.name == 'nt', 'Windows descendant cwd release')
    def test_windows_success_drains_non_pipe_descendant(self):
        child = 'from pathlib import Path; import time; Path("ready").touch(); time.sleep(20)'
        code = ('import subprocess,sys,time; from pathlib import Path; '
                f'subprocess.Popen([sys.executable,"-c",{child!r}], '
                'stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)\n'
                'while not Path("ready").exists(): time.sleep(.01)')
        with tempfile.TemporaryDirectory(dir=self.root) as directory:
            result = bp.run([sys.executable, '-c', code], cwd=Path(directory), timeout=3)
            self.assertEqual(result.returncode, 0)

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
