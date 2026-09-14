"""Concurrent pipe draining with a live shared byte budget and deadline."""
from __future__ import annotations

import math
import ctypes
import os
from pathlib import Path
import signal
import subprocess
import threading
import time
from typing import Callable, Mapping, Sequence


class ProcessLimitError(RuntimeError):
    def __init__(self, reason: str, stdout: bytes = b'', stderr: bytes = b''):
        super().__init__(reason)
        self.stdout, self.stderr = stdout, stderr


class _WindowsJob:
    """Assign a suspended child before it can spawn, then resume it.

    Drain this owned job before closing it, even after the launcher exits.
    """
    def __init__(self, process):
        from ctypes import wintypes as w
        class Basic(ctypes.Structure):
            _fields_ = [('process_time', ctypes.c_int64), ('job_time', ctypes.c_int64),
                        ('flags', w.DWORD), ('min_ws', ctypes.c_size_t), ('max_ws', ctypes.c_size_t),
                        ('active', w.DWORD), ('affinity', ctypes.c_size_t),
                        ('priority', w.DWORD), ('scheduling', w.DWORD)]
        class Extended(ctypes.Structure):
            _fields_ = [('basic', Basic), ('io', ctypes.c_uint64*6),
                        ('process_memory', ctypes.c_size_t), ('job_memory', ctypes.c_size_t),
                        ('peak_process', ctypes.c_size_t), ('peak_job', ctypes.c_size_t)]
        class Accounting(ctypes.Structure):
            _fields_ = [('user_time', ctypes.c_int64), ('kernel_time', ctypes.c_int64),
                        ('period_user_time', ctypes.c_int64), ('period_kernel_time', ctypes.c_int64),
                        ('page_faults', w.DWORD), ('total_processes', w.DWORD),
                        ('active_processes', w.DWORD), ('terminated_processes', w.DWORD)]
        self.accounting_type = Accounting
        self.api = ctypes.WinDLL('kernel32', use_last_error=True)
        self.api.CreateJobObjectW.argtypes = [ctypes.c_void_p, w.LPCWSTR]
        self.api.CreateJobObjectW.restype = w.HANDLE
        self.api.SetInformationJobObject.argtypes = [w.HANDLE, ctypes.c_int, ctypes.c_void_p, w.DWORD]
        self.api.AssignProcessToJobObject.argtypes = [w.HANDLE, w.HANDLE]
        self.api.TerminateJobObject.argtypes = [w.HANDLE, w.UINT]
        self.api.QueryInformationJobObject.argtypes = [w.HANDLE, ctypes.c_int, ctypes.c_void_p,
                                                      w.DWORD, ctypes.POINTER(w.DWORD)]
        self.api.QueryInformationJobObject.restype = w.BOOL
        self.api.OpenProcess.argtypes = [w.DWORD, w.BOOL, w.DWORD]
        self.api.OpenProcess.restype = w.HANDLE
        self.api.IsProcessInJob.argtypes = [w.HANDLE, w.HANDLE, ctypes.POINTER(w.BOOL)]
        self.api.IsProcessInJob.restype = w.BOOL
        self.api.WaitForSingleObject.argtypes = [w.HANDLE, w.DWORD]
        self.api.WaitForSingleObject.restype = w.DWORD
        self.api.CloseHandle.argtypes = [w.HANDLE]
        self.process_handles: list[int] = []
        self.handle = self.api.CreateJobObjectW(None, None)
        if not self.handle:
            raise ctypes.WinError(ctypes.get_last_error())
        try:
            limits = Extended()
            limits.basic.flags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
            if not self.api.SetInformationJobObject(self.handle, 9, ctypes.byref(limits), ctypes.sizeof(limits)):
                raise ctypes.WinError(ctypes.get_last_error())
            if not self.api.AssignProcessToJobObject(self.handle, int(process._handle)):
                raise ctypes.WinError(ctypes.get_last_error())
            resume = ctypes.WinDLL('ntdll').NtResumeProcess
            resume.argtypes = [w.HANDLE]
            resume.restype = ctypes.c_long
            if resume(int(process._handle)) < 0:
                raise OSError('could not resume bounded child')
        except BaseException:
            self.close()
            raise

    def terminate(self):
        # A job's ActiveProcesses can reach zero before process teardown has
        # released its cwd. Keep verified process objects alive for a real wait.
        snapshot_error = None
        try:
            self.capture_process_handles()
        except Exception as exc:
            snapshot_error = exc
        if not self.api.TerminateJobObject(self.handle, 1):
            error = ctypes.WinError(ctypes.get_last_error())
            if snapshot_error is not None:
                error.add_note(f'Windows job process snapshot: {snapshot_error}')
            raise error
        if snapshot_error is not None:
            raise snapshot_error

    def process_ids(self) -> list[int]:
        if not self.handle:
            raise RuntimeError('cannot query a closed owned Windows job')
        capacity = 16
        deadline = time.monotonic() + 3
        while capacity <= 4096:
            class ProcessIds(ctypes.Structure):
                _fields_ = [('assigned', ctypes.c_uint32), ('count', ctypes.c_uint32),
                            ('ids', ctypes.c_size_t * capacity)]
            info = ProcessIds()
            ok = self.api.QueryInformationJobObject(
                self.handle, 3, ctypes.byref(info), ctypes.sizeof(info), None)
            if ok and info.count == info.assigned and info.count <= capacity:
                return list(info.ids[:info.count])
            if not ok and ctypes.get_last_error() != 234:  # ERROR_MORE_DATA
                raise ctypes.WinError(ctypes.get_last_error())
            if time.monotonic() >= deadline:
                raise ProcessLimitError('Windows job process snapshot timed out')
            capacity = max(capacity * 2, info.assigned)
        raise ProcessLimitError('Windows job process snapshot exceeded 4096 processes')

    def capture_process_handles(self) -> None:
        from ctypes import wintypes as w
        for pid in self.process_ids():
            # SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION. No kill access
            # is requested, and reused PIDs must still belong to this exact job.
            handle = self.api.OpenProcess(0x100000 | 0x1000, False, pid)
            if not handle:
                if ctypes.get_last_error() == 87:  # Process already gone.
                    continue
                raise ctypes.WinError(ctypes.get_last_error())
            try:
                owned = w.BOOL()
                if not self.api.IsProcessInJob(handle, self.handle, ctypes.byref(owned)):
                    raise ctypes.WinError(ctypes.get_last_error())
                if owned.value:
                    self.process_handles.append(handle)
                    handle = None
            finally:
                if handle:
                    self.api.CloseHandle(handle)

    def active_processes(self) -> int:
        if not self.handle:
            raise RuntimeError('cannot query a closed owned Windows job')
        accounting = self.accounting_type()
        # JobObjectBasicAccountingInformation, not the calling process's job.
        if not self.api.QueryInformationJobObject(
                self.handle, 1, ctypes.byref(accounting), ctypes.sizeof(accounting), None):
            raise ctypes.WinError(ctypes.get_last_error())
        return accounting.active_processes

    def wait_empty(self, timeout: float = 3) -> None:
        """Termination is asynchronous; pipe EOF does not release descendant cwd."""
        deadline = time.monotonic() + timeout
        for handle in getattr(self, 'process_handles', ()):
            milliseconds = max(0, math.ceil((deadline - time.monotonic()) * 1000))
            status = self.api.WaitForSingleObject(handle, milliseconds)
            if status == 0xFFFFFFFF:  # WAIT_FAILED
                raise ctypes.WinError(ctypes.get_last_error())
            if status != 0:  # WAIT_OBJECT_0 signals completed process teardown.
                raise ProcessLimitError(f'Windows job process did not signal within {timeout:g} seconds')
        while True:
            active = self.active_processes()
            if not active:
                return
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise ProcessLimitError(f'Windows job did not drain after {timeout:g} seconds '
                                        f'({active} active processes)')
            time.sleep(min(0.01, remaining))

    def close(self):
        for handle in self.process_handles:
            self.api.CloseHandle(handle)
        self.process_handles.clear()
        if self.handle:
            self.api.CloseHandle(self.handle)
            self.handle = None


def terminate_tree(process: subprocess.Popen) -> None:
    if os.name == 'nt':
        subprocess.run(['taskkill', '/PID', str(process.pid), '/T', '/F'],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                       timeout=3, check=False)
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    if process.poll() is None:
        process.kill()


def run(argv: Sequence[str], *, cwd: Path, timeout: float,
        env: Mapping[str, str] | None = None, max_output: int = 1024 * 1024,
        check: Callable[[], None] | None = None) -> subprocess.CompletedProcess[bytes]:
    """Drain both pipes immediately; keep at most max_output bytes combined.

    ``check`` may enforce storage/cancellation constraints. It is run every 50ms
    outside reader threads. Its original exception survives process cleanup.
    """
    if not math.isfinite(timeout) or timeout <= 0 or max_output <= 0:
        raise ValueError('positive finite timeout and output budget required')
    flags = (getattr(subprocess, 'CREATE_NEW_PROCESS_GROUP', 0) | 0x4) if os.name == 'nt' else 0
    started = time.monotonic()
    process = subprocess.Popen(list(argv), cwd=cwd, env=env, stdin=subprocess.DEVNULL,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               creationflags=flags, start_new_session=os.name != 'nt')
    job = None
    try:
        if os.name == 'nt':
            job = _WindowsJob(process)
    except BaseException:
        process.kill()
        process.wait(timeout=3)
        process.stdout.close()
        process.stderr.close()
        raise
    buffers = [bytearray(), bytearray()]
    lock = threading.Lock()
    overflow = threading.Event()
    read_errors: list[BaseException] = []
    used = 0

    def drain(stream, index):
        nonlocal used
        try:
            while True:
                data = stream.read1(16384)
                if not data:
                    break
                with lock:
                    available = max(0, max_output - used)
                    buffers[index].extend(data[:available])
                    used += min(len(data), available)
                    if len(data) > available:
                        overflow.set()
        except (OSError, ValueError) as exc:
            read_errors.append(exc)
        finally:
            stream.close()

    threads = [threading.Thread(target=drain, args=(stream, i), daemon=True,
                                name=f'recovery-pipe-{process.pid}-{i}')
               for i, stream in enumerate((process.stdout, process.stderr))]
    for thread in threads:
        thread.start()
    failure: BaseException | None = None
    try:
        while True:
            if overflow.is_set():
                raise ProcessLimitError(f'command output exceeded {max_output} bytes')
            if read_errors:
                raise ProcessLimitError(f'command output read failed: {read_errors[0]}')
            if process.poll() is not None and not any(t.is_alive() for t in threads):
                break
            if time.monotonic() - started >= timeout:
                raise ProcessLimitError(f'command timed out after {timeout:g} seconds')
            if check is not None:
                check()
            time.sleep(min(0.05, max(0, timeout - (time.monotonic() - started))))
    except BaseException as exc:
        failure = exc
    finally:
        # Closing a Windows job already killed any surviving descendants on the
        # successful path. Start that cleanup explicitly so it can be awaited.
        if failure is not None or job is not None:
            try:
                if job is not None:
                    job.terminate()
                else:
                    terminate_tree(process)
            except Exception as exc:
                if failure is None:
                    failure = ProcessLimitError(f'process-tree cleanup failed: {exc}')
                else:
                    failure.add_note(f'process-tree cleanup: {exc}')
                if process.poll() is None:
                    process.kill()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired as exc:
            if failure is None:
                failure = ProcessLimitError('process did not terminate')
            failure.add_note(str(exc))
        if job is not None:
            try:
                job.wait_empty()
            except Exception as exc:
                if failure is None:
                    failure = ProcessLimitError(f'Windows job drain failed: {exc}')
                else:
                    failure.add_note(f'Windows job drain: {exc}')
        for thread in threads:
            thread.join(timeout=1)
        if any(t.is_alive() for t in threads):
            if failure is None:
                failure = ProcessLimitError('pipe descendants did not terminate')
            else:
                failure.add_note('pipe descendants did not terminate')
        if job is not None:
            job.close()
    stdout, stderr = (bytes(data) for data in buffers)
    if failure is not None:
        if isinstance(failure, ProcessLimitError):
            failure.stdout, failure.stderr = stdout, stderr
        raise failure
    return subprocess.CompletedProcess(list(argv), process.returncode, stdout, stderr)
