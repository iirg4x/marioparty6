
#!/usr/bin/env python3
"""Dump GC/2.6 local VarInfo through the native Win32 debug API.

This is intentionally a small, single-process diagnostic helper.  It launches
``mwcceppc.exe`` with DEBUG_ONLY_THIS_PROCESS, breaks at the native GC/2.6
local-FPR allocator, and reads the compiler's object lists while the target
function is paused.  It does not use a debugger executable, inject code, or
modify the reconstruction tree.

The default command mirrors the current ``src/board/telop.c`` Ninja rule.  A
different compiler command can be supplied after ``--``; the helper always
adds the source/output arguments only when no command is supplied.
"""

from __future__ import annotations

import argparse
import ctypes
from ctypes import wintypes
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
from typing import Any, Callable, Iterable, Sequence


# Win32 constants used by the native debugger API.
DEBUG_ONLY_THIS_PROCESS = 0x00000002
CREATE_NO_WINDOW = 0x08000000
DBG_CONTINUE = 0x00010002
DBG_EXCEPTION_NOT_HANDLED = 0x80010001
EXCEPTION_DEBUG_EVENT = 1
CREATE_THREAD_DEBUG_EVENT = 2
CREATE_PROCESS_DEBUG_EVENT = 3
EXIT_THREAD_DEBUG_EVENT = 4
EXIT_PROCESS_DEBUG_EVENT = 5
EXCEPTION_BREAKPOINT = 0x80000003
EXCEPTION_SINGLE_STEP = 0x80000004
# WOW64 reports 32-bit first-chance traps using the compatibility status
# values below rather than the native x64 exception codes.
EXCEPTION_WX86_BREAKPOINT = 0x4000001F
EXCEPTION_WX86_SINGLE_STEP = 0x4000001E
INFINITE = 0xFFFFFFFF
ERROR_SEM_TIMEOUT = 121
WAIT_POLL_MILLISECONDS = 250
DEFAULT_TIMEOUT_SECONDS = 30.0
MAX_TIMEOUT_SECONDS = 300.0
STARTF_USESHOWWINDOW = 0x00000001
SW_HIDE = 0

# x86 CONTEXT flags for a WOW64 target.  The compiler is a 32-bit PE even when
# this Python helper runs as a 64-bit process.
WOW64_CONTEXT_FULL = 0x00010007
WOW64_CONTEXT_CONTROL = 0x00010001
WOW64_CONTEXT_INTEGER = 0x00010002
WOW64_CONTEXT_TRACE = 0x00010010
WOW64_CONTEXT_TF = 0x00000100

KNOWN_IMAGE_BASE = 0x00400000
CODEGEN_START = 0x00433492
# repos/mwcc-debugger uses 0x5089A9 for the later colorgraph pass.  The
# earlier O0 local-FPR allocator is the useful hook here: object lists and
# VarInfo are still named and live at 0x4357D0.
ALLOCATE_LOCAL_FPRS = 0x004357D0
ASSIGN_LOCAL_FPR = 0x0043598B

# These are the bytes in the pinned GC/2.6 compiler before the helper writes
# an INT3.  Every hook is checked before any breakpoint is installed; this
# prevents accidentally patching an unrelated executable at a reused address.
PINNED_COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"
EXPECTED_HOOK_BYTES: dict[int, bytes] = {
    CODEGEN_START: bytes.fromhex("8b400e8b5006eb08"),
    ALLOCATE_LOCAL_FPRS: bytes.fromhex("5356575583ec10e9"),
    ASSIGN_LOCAL_FPR: bytes.fromhex("ff74240ce89ca809"),
}

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_COMPILER = REPO_ROOT / "build" / "compilers" / "GC" / "2.6" / "mwcceppc.exe"
DEFAULT_OUTPUT = (
    REPO_ROOT / "build" / "recovery" / "mwcc-win32-varinfo" / "mbTelopTimeSprRotSet.json"
)

GFUNCTION = 0x005E9EC0
LOCALS_LIST = 0x005EA8D4
ARGUMENTS_LIST = 0x005EAA28

OBJECT_DATATYPE = 0x02
OBJECT_NAME = 0x0A
OBJECT_VARINFO = 0x2A


if os.name == "nt":
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
else:  # Keep imports and --help usable when inspected off-host.
    kernel32 = None


DWORD = ctypes.c_uint32
WORD = ctypes.c_uint16
LONG = ctypes.c_int32
ULONG_PTR = ctypes.c_size_t
SIZE_T = ctypes.c_size_t
HANDLE = wintypes.HANDLE
LPVOID = ctypes.c_void_p


class STARTUPINFOW(ctypes.Structure):
    _fields_ = [
        ("cb", DWORD),
        ("lpReserved", ctypes.c_wchar_p),
        ("lpDesktop", ctypes.c_wchar_p),
        ("lpTitle", ctypes.c_wchar_p),
        ("dwX", DWORD),
        ("dwY", DWORD),
        ("dwXSize", DWORD),
        ("dwYSize", DWORD),
        ("dwXCountChars", DWORD),
        ("dwYCountChars", DWORD),
        ("dwFillAttribute", DWORD),
        ("dwFlags", DWORD),
        ("wShowWindow", WORD),
        ("cbReserved2", WORD),
        ("lpReserved2", ctypes.POINTER(ctypes.c_ubyte)),
        ("hStdInput", HANDLE),
        ("hStdOutput", HANDLE),
        ("hStdError", HANDLE),
    ]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ("hProcess", HANDLE),
        ("hThread", HANDLE),
        ("dwProcessId", DWORD),
        ("dwThreadId", DWORD),
    ]


class EXCEPTION_RECORD(ctypes.Structure):
    _fields_ = [
        ("ExceptionCode", DWORD),
        ("ExceptionFlags", DWORD),
        ("ExceptionRecord", LPVOID),
        ("ExceptionAddress", LPVOID),
        ("NumberParameters", DWORD),
        ("__unused", DWORD),
        ("ExceptionInformation", ULONG_PTR * 15),
    ]


class EXCEPTION_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("ExceptionRecord", EXCEPTION_RECORD),
        ("dwFirstChance", DWORD),
    ]


class CREATE_THREAD_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hThread", HANDLE),
        ("lpThreadLocalBase", LPVOID),
        ("lpStartAddress", LPVOID),
    ]


class CREATE_PROCESS_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hFile", HANDLE),
        ("hProcess", HANDLE),
        ("hThread", HANDLE),
        ("lpBaseOfImage", LPVOID),
        ("dwDebugInfoFileOffset", DWORD),
        ("nDebugInfoSize", DWORD),
        ("lpThreadLocalBase", LPVOID),
        ("lpStartAddress", LPVOID),
        ("lpImageName", LPVOID),
        ("fUnicode", WORD),
    ]


class EXIT_THREAD_DEBUG_INFO(ctypes.Structure):
    _fields_ = [("dwExitCode", DWORD)]


class EXIT_PROCESS_DEBUG_INFO(ctypes.Structure):
    _fields_ = [("dwExitCode", DWORD)]


class LOAD_DLL_DEBUG_INFO(ctypes.Structure):
    _fields_ = [
        ("hFile", HANDLE),
        ("lpBaseOfDll", LPVOID),
        ("dwDebugInfoFileOffset", DWORD),
        ("nDebugInfoSize", DWORD),
        ("lpImageName", LPVOID),
        ("fUnicode", WORD),
    ]


class UNLOAD_DLL_DEBUG_INFO(ctypes.Structure):
    _fields_ = [("lpBaseOfDll", LPVOID)]


class OUTPUT_DEBUG_STRING_INFO(ctypes.Structure):
    _fields_ = [
        ("lpDebugStringData", LPVOID),
        ("fUnicode", WORD),
        ("nDebugStringLength", WORD),
    ]


class RIP_INFO(ctypes.Structure):
    _fields_ = [("dwError", DWORD), ("dwType", DWORD)]


class DEBUG_EVENT_UNION(ctypes.Union):
    _fields_ = [
        ("Exception", EXCEPTION_DEBUG_INFO),
        ("CreateThread", CREATE_THREAD_DEBUG_INFO),
        ("CreateProcessInfo", CREATE_PROCESS_DEBUG_INFO),
        ("ExitThread", EXIT_THREAD_DEBUG_INFO),
        ("ExitProcess", EXIT_PROCESS_DEBUG_INFO),
        ("LoadDll", LOAD_DLL_DEBUG_INFO),
        ("UnloadDll", UNLOAD_DLL_DEBUG_INFO),
        ("DebugString", OUTPUT_DEBUG_STRING_INFO),
        ("RipInfo", RIP_INFO),
    ]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [
        ("dwDebugEventCode", DWORD),
        ("dwProcessId", DWORD),
        ("dwThreadId", DWORD),
        ("u", DEBUG_EVENT_UNION),
    ]


class WOW64_FLOATING_SAVE_AREA(ctypes.Structure):
    _fields_ = [
        ("ControlWord", DWORD),
        ("StatusWord", DWORD),
        ("TagWord", DWORD),
        ("ErrorOffset", DWORD),
        ("ErrorSelector", DWORD),
        ("DataOffset", DWORD),
        ("DataSelector", DWORD),
        ("RegisterArea", ctypes.c_ubyte * 80),
        ("Cr0NpxState", DWORD),
    ]


class WOW64_CONTEXT(ctypes.Structure):
    _fields_ = [
        ("ContextFlags", DWORD),
        ("Dr0", DWORD),
        ("Dr1", DWORD),
        ("Dr2", DWORD),
        ("Dr3", DWORD),
        ("Dr6", DWORD),
        ("Dr7", DWORD),
        ("FloatSave", WOW64_FLOATING_SAVE_AREA),
        ("SegGs", DWORD),
        ("SegFs", DWORD),
        ("SegEs", DWORD),
        ("SegDs", DWORD),
        ("Edi", DWORD),
        ("Esi", DWORD),
        ("Ebx", DWORD),
        ("Edx", DWORD),
        ("Ecx", DWORD),
        ("Eax", DWORD),
        ("Ebp", DWORD),
        ("Eip", DWORD),
        ("SegCs", DWORD),
        ("EFlags", DWORD),
        ("Esp", DWORD),
        ("SegSs", DWORD),
        ("ExtendedRegisters", ctypes.c_ubyte * 512),
    ]


def _configure_api() -> None:
    if kernel32 is None:
        return
    kernel32.CreateProcessW.argtypes = [
        wintypes.LPCWSTR,
        wintypes.LPWSTR,
        LPVOID,
        LPVOID,
        wintypes.BOOL,
        DWORD,
        LPVOID,
        wintypes.LPCWSTR,
        ctypes.POINTER(STARTUPINFOW),
        ctypes.POINTER(PROCESS_INFORMATION),
    ]
    kernel32.CreateProcessW.restype = wintypes.BOOL
    kernel32.WaitForDebugEvent.argtypes = [ctypes.POINTER(DEBUG_EVENT), DWORD]
    kernel32.WaitForDebugEvent.restype = wintypes.BOOL
    kernel32.ContinueDebugEvent.argtypes = [DWORD, DWORD, DWORD]
    kernel32.ContinueDebugEvent.restype = wintypes.BOOL
    kernel32.ReadProcessMemory.argtypes = [HANDLE, LPVOID, LPVOID, SIZE_T, ctypes.POINTER(SIZE_T)]
    kernel32.ReadProcessMemory.restype = wintypes.BOOL
    kernel32.WriteProcessMemory.argtypes = [HANDLE, LPVOID, LPVOID, SIZE_T, ctypes.POINTER(SIZE_T)]
    kernel32.WriteProcessMemory.restype = wintypes.BOOL
    kernel32.Wow64GetThreadContext.argtypes = [HANDLE, ctypes.POINTER(WOW64_CONTEXT)]
    kernel32.Wow64GetThreadContext.restype = wintypes.BOOL
    kernel32.Wow64SetThreadContext.argtypes = [HANDLE, ctypes.POINTER(WOW64_CONTEXT)]
    kernel32.Wow64SetThreadContext.restype = wintypes.BOOL
    kernel32.OpenThread.argtypes = [DWORD, wintypes.BOOL, DWORD]
    kernel32.OpenThread.restype = HANDLE
    kernel32.CloseHandle.argtypes = [HANDLE]
    kernel32.CloseHandle.restype = wintypes.BOOL
    kernel32.GetExitCodeProcess.argtypes = [HANDLE, ctypes.POINTER(DWORD)]
    kernel32.GetExitCodeProcess.restype = wintypes.BOOL
    kernel32.TerminateProcess.argtypes = [HANDLE, wintypes.UINT]
    kernel32.TerminateProcess.restype = wintypes.BOOL


_configure_api()


def winerr(prefix: str) -> OSError:
    return ctypes.WinError(ctypes.get_last_error(), prefix)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_compiler_path(path: Path) -> None:
    """Reject wrappers and missing binaries before CreateProcessW."""
    if path.name.casefold() != "mwcceppc.exe":
        raise ValueError(f"compiler must be named mwcceppc.exe: {path}")
    if not path.is_file():
        raise FileNotFoundError(f"compiler not found: {path}")


def validate_compiler_fingerprint(path: Path, expected_sha256: str = PINNED_COMPILER_SHA256) -> str:
    expected = expected_sha256.strip().casefold()
    if len(expected) != 64 or any(char not in "0123456789abcdef" for char in expected):
        raise ValueError("expected compiler SHA-256 must be 64 hexadecimal characters")
    validate_compiler_path(path)
    actual = sha256_file(path)
    if actual != expected:
        raise ValueError(
            f"compiler SHA-256 mismatch for {path}: expected {expected}, got {actual}"
        )
    return actual


def atomic_write_json(path: Path, value: dict[str, Any]) -> None:
    """Replace a JSON report atomically, including on a failed probe."""
    path = path.resolve()
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        descriptor, temporary_name = tempfile.mkstemp(
            prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
        )
        temporary = Path(temporary_name)
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            json.dump(value, stream, indent=2, sort_keys=True)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            try:
                temporary.unlink()
            except FileNotFoundError:
                pass


def validate_hook_bytes(
    read_memory: Callable[[int, int], bytes], runtime_address: Callable[[int], int]
) -> None:
    """Check all native hook sites before the first INT3 write."""
    mismatches: list[str] = []
    for absolute, expected in EXPECTED_HOOK_BYTES.items():
        address = runtime_address(absolute)
        actual = read_memory(address, len(expected))
        if actual != expected:
            mismatches.append(
                f"0x{absolute:08x}: expected {expected.hex()}, got {actual.hex() or '<unreadable>'}"
            )
    if mismatches:
        raise RuntimeError("compiler hook byte validation failed: " + "; ".join(mismatches))


def validate_result_schema(value: dict[str, Any]) -> None:
    required = {
        "schema_version",
        "tool",
        "target",
        "capture_assignments",
        "known_image_base",
        "breakpoints",
    }
    missing = sorted(required.difference(value))
    if missing:
        raise ValueError(f"VarInfo report missing schema fields: {', '.join(missing)}")
    if value["schema_version"] != 1 or value["tool"] != "mwcc_win32_varinfo":
        raise ValueError("unsupported VarInfo report schema")


def u32(data: bytes, offset: int = 0) -> int:
    return int.from_bytes(data[offset : offset + 4], "little", signed=False)


def s16(data: bytes, offset: int = 0) -> int:
    return int.from_bytes(data[offset : offset + 2], "little", signed=True)


def s32(data: bytes, offset: int = 0) -> int:
    return int.from_bytes(data[offset : offset + 4], "little", signed=True)


class Debugger:
    def __init__(
        self,
        process: int,
        output: Path,
        target_name: str,
        trace: bool = False,
        capture_assignments: bool = False,
        timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS,
        compiler_sha256: str = PINNED_COMPILER_SHA256,
    ) -> None:
        self.process = process
        self.output = output
        self.target_name = target_name
        self.trace = trace
        self.capture_assignments = capture_assignments
        self.timeout_seconds = timeout_seconds
        self.compiler_sha256 = compiler_sha256
        self.pid = 0
        self.base = 0
        self.threads: dict[int, int] = {}
        self.breakpoints: dict[int, int] = {}
        self.pending_step: tuple[int, int] | None = None
        self.target_seen = False
        self.dumped = False
        self.exited = False
        self.hooks_validated = False
        self.deadline = time.monotonic() + timeout_seconds
        self.result: dict[str, Any] = {
            "schema_version": 1,
            "tool": "mwcc_win32_varinfo",
            "target": target_name,
            "capture_assignments": capture_assignments,
            "known_image_base": KNOWN_IMAGE_BASE,
            "compiler_sha256": compiler_sha256,
            "timeout_seconds": timeout_seconds,
            "breakpoints": {
                "codegen_start": hex(CODEGEN_START),
                "allocate_local_FPRs": hex(ALLOCATE_LOCAL_FPRS),
                "assign_local_FPR": hex(ASSIGN_LOCAL_FPR),
            },
        }

    def log(self, message: str) -> None:
        if self.trace:
            print(message, file=sys.stderr, flush=True)

    def runtime(self, absolute: int) -> int:
        return self.base + (absolute - KNOWN_IMAGE_BASE)

    def read(self, address: int, size: int) -> bytes:
        if not address or size <= 0:
            return b""
        buf = ctypes.create_string_buffer(size)
        got = SIZE_T()
        ok = kernel32.ReadProcessMemory(
            self.process,
            ctypes.c_void_p(address),
            buf,
            size,
            ctypes.byref(got),
        )
        if not ok:
            return b""
        return buf.raw[: got.value]

    def read_u32(self, address: int) -> int:
        data = self.read(address, 4)
        return u32(data) if len(data) == 4 else 0

    def read_string(self, address: int, limit: int = 256) -> str:
        data = self.read(address, limit)
        if not data:
            return ""
        data = data.split(b"\0", 1)[0]
        return data.decode("latin-1", errors="replace")

    def read_object_name(self, object_address: int) -> str:
        name_hash = self.read_u32(object_address + OBJECT_NAME)
        if not name_hash:
            return ""
        return self.read_string(name_hash + 0x0A)

    def write(self, address: int, data: bytes) -> None:
        buf = ctypes.create_string_buffer(data)
        written = SIZE_T()
        ok = kernel32.WriteProcessMemory(
            self.process,
            ctypes.c_void_p(address),
            buf,
            len(data),
            ctypes.byref(written),
        )
        if not ok or written.value != len(data):
            raise winerr(f"WriteProcessMemory(0x{address:08x})")

    def install_breakpoint(self, absolute: int) -> None:
        if not self.hooks_validated:
            raise RuntimeError("refusing INT3 write before compiler hook validation")
        address = self.runtime(absolute)
        if address in self.breakpoints:
            return
        original = self.read(address, 1)
        if len(original) != 1:
            raise RuntimeError(f"cannot read breakpoint byte at 0x{address:08x}")
        self.write(address, b"\xCC")
        self.breakpoints[address] = original[0]

    def validate_hooks(self) -> None:
        validate_hook_bytes(self.read, self.runtime)
        self.hooks_validated = True

    def remove_breakpoint(self, address: int) -> None:
        original = self.breakpoints.pop(address, None)
        if original is not None:
            self.write(address, bytes([original]))

    def get_context(self, thread: int) -> WOW64_CONTEXT:
        context = WOW64_CONTEXT()
        context.ContextFlags = WOW64_CONTEXT_FULL
        if not kernel32.Wow64GetThreadContext(thread, ctypes.byref(context)):
            raise winerr("Wow64GetThreadContext")
        return context

    def set_context(self, thread: int, context: WOW64_CONTEXT) -> None:
        if not kernel32.Wow64SetThreadContext(thread, ctypes.byref(context)):
            raise winerr("Wow64SetThreadContext")

    def step_over(self, event: DEBUG_EVENT, address: int, rearm: bool) -> None:
        """Restore an INT3, execute one instruction, optionally re-arm it."""
        self.remove_breakpoint(address)
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError(f"no thread handle for {event.dwThreadId}")
        context = self.get_context(thread)
        # Windows reports EIP immediately after the INT3 trap.
        context.Eip = address
        if rearm:
            context.EFlags |= WOW64_CONTEXT_TF
            self.pending_step = (event.dwThreadId, address)
        self.set_context(thread, context)

    def handle_codegen_breakpoint(self, event: DEBUG_EVENT, address: int) -> None:
        function_object = self.read_u32(self.runtime(GFUNCTION))
        function_name = self.read_object_name(function_object)
        self.result.setdefault("functions_seen", []).append(
            {"object": hex(function_object), "name": function_name}
        )
        if self.target_seen and self.dumped:
            # With --assign, keep the codegen hook alive just long enough to
            # detect the next function and retire the assignment hook.  The
            # allocator address is shared by every compiler function.
            if self.capture_assignments:
                self.remove_breakpoint(self.runtime(ASSIGN_LOCAL_FPR))
            self.step_over(event, address, rearm=False)
            self.remove_breakpoint(address)
            return
        if function_name == self.target_name:
            self.target_seen = True
            self.install_breakpoint(ALLOCATE_LOCAL_FPRS)
            if self.capture_assignments:
                self.install_breakpoint(ASSIGN_LOCAL_FPR)
            self.step_over(event, address, rearm=self.capture_assignments)
            return
        self.step_over(event, address, rearm=True)

    def object_record(self, address: int) -> dict[str, Any]:
        datatype_data = self.read(address + OBJECT_DATATYPE, 1)
        datatype = datatype_data[0] if datatype_data else None
        info_address = self.read_u32(address + OBJECT_VARINFO) if datatype == 1 else 0
        record: dict[str, Any] = {
            "object": hex(address),
            "name": self.read_object_name(address),
            "datatype": datatype,
            "varinfo": hex(info_address) if info_address else None,
        }
        if info_address:
            data = self.read(info_address, 0x2A)
            if len(data) >= 0x2A:
                record.update(
                    {
                        "usage": s32(data, 0x04),
                        "noregister": data[0x22],
                        "used": data[0x23],
                        "flags": data[0x24],
                        "rclass": data[0x25],
                        "reg": s16(data, 0x26),
                        "reg_hi": s16(data, 0x28),
                    }
                )
        return record

    def object_list(self, list_address: int) -> list[dict[str, Any]]:
        head = self.read_u32(self.runtime(list_address))
        records: list[dict[str, Any]] = []
        seen: set[int] = set()
        while head and head not in seen and len(records) < 1024:
            seen.add(head)
            node = self.read(head, 8)
            if len(node) != 8:
                break
            object_address = u32(node, 4)
            if object_address:
                records.append(self.object_record(object_address))
            head = u32(node, 0)
        return records

    def write_result(self) -> None:
        validate_result_schema(self.result)
        atomic_write_json(self.output, self.result)

    def dump_locals(self, event: DEBUG_EVENT) -> None:
        locals_records = self.object_list(LOCALS_LIST)
        arguments_records = self.object_list(ARGUMENTS_LIST)
        self.result.update(
            {
                "pid": self.pid,
                "image_base": hex(self.base),
                "function": self.target_name,
                "breakpoint_address": hex(self.runtime(ALLOCATE_LOCAL_FPRS)),
                "locals": locals_records,
                "arguments": arguments_records,
                "varinfo_layout": {
                    "usage": "+0x04 s32",
                    "noregister": "+0x22 u8",
                    "used": "+0x23 u8",
                    "flags": "+0x24 u8",
                    "rclass": "+0x25 u8",
                    "reg": "+0x26 s16",
                    "reg_hi": "+0x28 s16",
                },
            }
        )
        self.write_result()
        self.dumped = True

    def dump_assignment_snapshot(self, event: DEBUG_EVENT) -> None:
        """Record the allocator's pre-call VarInfo state and x86 registers."""
        thread = self.threads.get(event.dwThreadId)
        context = self.get_context(thread) if thread else None
        snapshot: dict[str, Any] = {
            "index": len(self.result.setdefault("assignment_snapshots", [])),
            "eip": hex(context.Eip) if context else None,
            "eax": hex(context.Eax) if context else None,
            "ecx": hex(context.Ecx) if context else None,
            "edx": hex(context.Edx) if context else None,
            "esp": hex(context.Esp) if context else None,
            "locals": self.object_list(LOCALS_LIST),
        }
        self.result["assignment_snapshots"].append(snapshot)
        self.write_result()

    def close_thread(self, thread_id: int) -> None:
        handle = self.threads.pop(thread_id, None)
        if handle:
            kernel32.CloseHandle(handle)

    def run(self) -> int:
        event = DEBUG_EVENT()
        while True:
            remaining = self.deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError(
                    f"debug-event timeout after {self.timeout_seconds:.1f}s"
                )
            wait_milliseconds = min(
                WAIT_POLL_MILLISECONDS, max(1, int(remaining * 1000))
            )
            ok = kernel32.WaitForDebugEvent(ctypes.byref(event), wait_milliseconds)
            if not ok:
                error = ctypes.get_last_error()
                if error == ERROR_SEM_TIMEOUT:
                    code = DWORD()
                    if kernel32.GetExitCodeProcess(self.process, ctypes.byref(code)) and code.value != 259:
                        return int(code.value)
                    continue
                raise winerr("WaitForDebugEvent")

            code = event.dwDebugEventCode
            pid = event.dwProcessId
            tid = event.dwThreadId
            status = DBG_CONTINUE
            try:
                if code == CREATE_PROCESS_DEBUG_EVENT:
                    self.pid = pid
                    self.base = int(event.u.CreateProcessInfo.lpBaseOfImage or 0)
                    thread = int(event.u.CreateProcessInfo.hThread or 0)
                    if thread:
                        self.threads[tid] = thread
                    image_base = self.base
                    self.result.update({"pid": pid, "image_base": hex(image_base)})
                    self.log(f"CREATE_PROCESS pid={pid} tid={tid} base=0x{self.base:08x}")
                    self.validate_hooks()
                    self.install_breakpoint(CODEGEN_START)
                    file_handle = int(event.u.CreateProcessInfo.hFile or 0)
                    if file_handle:
                        kernel32.CloseHandle(file_handle)
                elif code == CREATE_THREAD_DEBUG_EVENT:
                    thread = int(event.u.CreateThread.hThread or 0)
                    if thread:
                        self.threads[tid] = thread
                    self.log(f"CREATE_THREAD tid={tid}")
                elif code == EXIT_THREAD_DEBUG_EVENT:
                    self.log(f"EXIT_THREAD tid={tid} code={event.u.ExitThread.dwExitCode}")
                    self.close_thread(tid)
                elif code == EXIT_PROCESS_DEBUG_EVENT:
                    self.result["exit_code"] = int(event.u.ExitProcess.dwExitCode)
                    self.exited = True
                    self.log(f"EXIT_PROCESS code=0x{int(event.u.ExitProcess.dwExitCode):08x} target_seen={self.target_seen} dumped={self.dumped}")
                    self.write_result()
                    break
                elif code == EXCEPTION_DEBUG_EVENT:
                    exception = event.u.Exception.ExceptionRecord
                    exception_code = int(exception.ExceptionCode)
                    exception_address = int(exception.ExceptionAddress or 0)
                    self.log(
                        f"EXCEPTION tid={tid} code=0x{exception_code:08x} "
                        f"addr=0x{exception_address:08x} eip_bp={[hex(x) for x in self.breakpoints]}"
                    )
                    is_single_step = exception_code in (
                        EXCEPTION_SINGLE_STEP,
                        EXCEPTION_WX86_SINGLE_STEP,
                    )
                    is_breakpoint = exception_code in (
                        EXCEPTION_BREAKPOINT,
                        EXCEPTION_WX86_BREAKPOINT,
                    )
                    if is_single_step and self.pending_step:
                        pending_tid, pending_address = self.pending_step
                        if pending_tid == tid:
                            thread = self.threads.get(tid)
                            if thread:
                                context = self.get_context(thread)
                                context.EFlags &= ~WOW64_CONTEXT_TF
                                self.set_context(thread, context)
                            self.install_breakpoint(pending_address)
                            self.pending_step = None
                    elif is_breakpoint:
                        if exception_address in self.breakpoints:
                            if exception_address == self.runtime(CODEGEN_START):
                                self.handle_codegen_breakpoint(event, exception_address)
                            elif exception_address == self.runtime(ALLOCATE_LOCAL_FPRS):
                                if not self.target_seen:
                                    self.step_over(event, exception_address, rearm=False)
                                else:
                                    self.dump_locals(event)
                                    self.step_over(event, exception_address, rearm=False)
                                    if not self.capture_assignments:
                                        self.remove_breakpoint(self.runtime(CODEGEN_START))
                                    self.result["status"] = "dumped"
                            elif exception_address == self.runtime(ASSIGN_LOCAL_FPR):
                                if self.capture_assignments:
                                    self.dump_assignment_snapshot(event)
                                self.step_over(event, exception_address, rearm=True)
                        else:
                            # The loader's first breakpoint is expected and does
                            # not need to be passed through to the compiler.
                            pass
                    elif exception_code not in (
                        EXCEPTION_BREAKPOINT,
                        EXCEPTION_WX86_BREAKPOINT,
                        EXCEPTION_SINGLE_STEP,
                        EXCEPTION_WX86_SINGLE_STEP,
                    ):
                        status = DBG_EXCEPTION_NOT_HANDLED
                # All debug events must be continued, including handled traps.
                if not kernel32.ContinueDebugEvent(pid, tid, status):
                    raise winerr("ContinueDebugEvent")
            finally:
                event = DEBUG_EVENT()
        return int(self.result.get("exit_code", 0))

    def close(self) -> None:
        if not self.exited and self.process:
            # An exception in the inspection path must not leave the compiler
            # running outside the debugger.  This is only reached on failure;
            # normal runs observe EXIT_PROCESS_DEBUG_EVENT above.
            kernel32.TerminateProcess(self.process, 1)
        for thread_id in list(self.threads):
            self.close_thread(thread_id)


def default_command(repo: Path, output_dir: Path) -> list[str]:
    return [
        "-nodefaults",
        "-proc",
        "gekko",
        "-align",
        "powerpc",
        "-enum",
        "int",
        "-fp",
        "hardware",
        "-Cpp_exceptions",
        "off",
        "-O4,p",
        "-inline",
        "auto",
        "-pragma",
        "cats off",
        "-pragma",
        "warn_notinlined off",
        "-maxerrors",
        "1",
        "-nosyspath",
        "-RTTI",
        "off",
        "-fp_contract",
        "on",
        "-str",
        "reuse",
        "-multibyte",
        "-i",
        "include",
        "-i",
        "build/GP6E01/include",
        "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
        "-DVERSION=0",
        "-DNDEBUG=1",
        "-O0,p",
        "-char",
        "unsigned",
        "-fp_contract",
        "off",
        "-c",
        "src/board/telop.c",
        "-o",
        str(output_dir),
    ]


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--compiler",
        default=str(DEFAULT_COMPILER),
        help="GC/2.6 mwcceppc.exe path",
    )
    parser.add_argument(
        "--cwd",
        default=str(REPO_ROOT),
        help="compiler working directory",
    )
    parser.add_argument("--target", default="mbTelopTimeSprRotSet")
    parser.add_argument("--trace", action="store_true", help="log debug events to stderr")
    parser.add_argument(
        "--assign",
        action="store_true",
        help="also snapshot locals at the allocator's pre-call FPR assignment breakpoint",
    )
    parser.add_argument(
        "--output",
        default=str(DEFAULT_OUTPUT),
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT_SECONDS,
        help=f"maximum debug-event time in seconds (1-{MAX_TIMEOUT_SECONDS:g})",
    )
    parser.add_argument(
        "compiler_args",
        nargs=argparse.REMAINDER,
        help="optional compiler arguments after --; defaults to the Telop command",
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv if argv is not None else sys.argv[1:])
    if os.name != "nt":
        print(
            "mwcc_win32_varinfo.py requires Windows (native WOW64 debug API); "
            "compiler was not launched",
            file=sys.stderr,
        )
        return 2
    compiler = Path(args.compiler).resolve()
    cwd = Path(args.cwd).resolve()
    output = Path(args.output).resolve()
    if not 1.0 <= args.timeout <= MAX_TIMEOUT_SECONDS:
        print(
            f"timeout must be between 1 and {MAX_TIMEOUT_SECONDS:g} seconds",
            file=sys.stderr,
        )
        return 2
    try:
        compiler_sha256 = validate_compiler_fingerprint(compiler)
    except (FileNotFoundError, ValueError, OSError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    if not cwd.is_dir():
        print(f"compiler cwd not found: {cwd}; compiler was not launched", file=sys.stderr)
        return 2

    if args.compiler_args and args.compiler_args[0] == "--":
        compiler_args = args.compiler_args[1:]
    else:
        compiler_args = args.compiler_args
    if not compiler_args:
        compiler_args = default_command(cwd, output.parent)

    command = subprocess.list2cmdline([str(compiler), *compiler_args])
    command_buffer = ctypes.create_unicode_buffer(command)
    startup = STARTUPINFOW(
        cb=ctypes.sizeof(STARTUPINFOW),
        dwFlags=STARTF_USESHOWWINDOW,
        wShowWindow=SW_HIDE,
    )
    process_info = PROCESS_INFORMATION()
    flags = DEBUG_ONLY_THIS_PROCESS | CREATE_NO_WINDOW
    created = kernel32.CreateProcessW(
        None,
        command_buffer,
        None,
        None,
        False,
        flags,
        None,
        str(cwd),
        ctypes.byref(startup),
        ctypes.byref(process_info),
    )
    if not created:
        raise winerr("CreateProcessW")

    debugger = Debugger(
        int(process_info.hProcess),
        output,
        args.target,
        trace=args.trace,
        capture_assignments=args.assign,
        timeout_seconds=args.timeout,
        compiler_sha256=compiler_sha256,
    )
    debugger.result["command"] = command
    debugger.result["cwd"] = str(cwd)
    try:
        return debugger.run()
    except Exception as exc:
        debugger.result["status"] = "error"
        debugger.result["error"] = str(exc)
        atomic_write_json(output, debugger.result)
        print(f"mwcc debugger: {exc}", file=sys.stderr)
        return 1
    finally:
        debugger.close()
        kernel32.CloseHandle(process_info.hThread)
        kernel32.CloseHandle(process_info.hProcess)


if __name__ == "__main__":
    raise SystemExit(main())
