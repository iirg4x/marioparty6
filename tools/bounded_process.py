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

    Closing this owned job kills descendants even after the launcher exits.
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
        self.api = ctypes.WinDLL('kernel32', use_last_error=True)
        self.api.CreateJobObjectW.argtypes = [ctypes.c_void_p, w.LPCWSTR]
        self.api.CreateJobObjectW.restype = w.HANDLE
        self.api.SetInformationJobObject.argtypes = [w.HANDLE, ctypes.c_int, ctypes.c_void_p, w.DWORD]
        self.api.AssignProcessToJobObject.argtypes = [w.HANDLE, w.HANDLE]
        self.api.TerminateJobObject.argtypes = [w.HANDLE, w.UINT]
        self.api.CloseHandle.argtypes = [w.HANDLE]
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
        if not self.api.TerminateJobObject(self.handle, 1):
            raise ctypes.WinError(ctypes.get_last_error())

    def close(self):
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
        if failure is not None:
            try:
                if job is not None:
                    job.terminate()
                else:
                    terminate_tree(process)
            except Exception as exc:
                failure.add_note(f'process-tree cleanup: {exc}')
                if process.poll() is None:
                    process.kill()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired as exc:
            if failure is None:
                failure = ProcessLimitError('process did not terminate')
            failure.add_note(str(exc))
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
