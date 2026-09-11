
#!/usr/bin/env python3
"""Dump GC/2.6 local VarInfo through the native Win32 debug API.

This is intentionally a small, single-process diagnostic helper.  It launches
``mwcceppc.exe`` with DEBUG_ONLY_THIS_PROCESS, breaks at the native GC/2.6
local-FPR allocator, and reads the compiler's object lists while the target
function is paused.  It does not use a debugger executable, inject code, or
modify the reconstruction tree.

Opt-in ``--regalloc`` observes optimized FPR coloring (or GPR coloring with
``--regalloc-class gpr``).  Add ``--frontend`` for AST stages and temporary
creation/range-split provenance, or ``--cse`` for GPR LI/LIS CSE observations.
``--machine-emit`` adds authenticated emitted offsets/words and GPR operand
color joins. Missing pre-color ancestry remains explicitly UNKNOWN.
These are facts about the supplied compilation, not recovered retail virtual
register identities. Unsupported frontend nodes remain explicitly incomplete.

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

# GC/2.6 optimized allocator observations, enabled only by --regalloc.
# These sites are authenticated against the pinned executable before INT3 writes.
REGALLOC_HOOK_BYTES = {
    0x508890: bytes.fromhex("5356575583ec08"),
    0x5088C6: bytes.fromhex("8b14248d731a31ed"),
    0x50892E: bytes.fromhex("66894b14"),
    0x508932: bytes.fromhex("eb60"),
    0x508954: bytes.fromhex("66894314"),
    0x508958: bytes.fromhex("0fbf4b14"),
    0x508987: bytes.fromhex("66834b1601"),
    0x508994: bytes.fromhex("8b1b85db"),
    0x5087A4: bytes.fromhex("66894204"),
}
# LI/LIS common-subexpression observations are intentionally opt-in.  These
# hooks are inside the optimized GPR path and are authenticated against the
# pinned executable before any INT3 writes.
CSE_LI_ELIGIBILITY_HOOK = 0x509356
CSE_LI_REUSE_HOOK = 0x509376
CSE_HOOK_BYTES = {
    CSE_LI_ELIGIBILITY_HOOK: bytes.fromhex("5985c00f84a1000000"),
    CSE_LI_REUSE_HOOK: bytes.fromhex("85c074760fbf442418"),
}
CSE_LI_OPCODES = frozenset((0x89, 0x8A))
CSE_PCODE_HEADER_SIZE = 0x24
CSE_PCODE_OPERAND_SIZE = 12
CSE_PCODE_SIZE = CSE_PCODE_HEADER_SIZE + 2 * CSE_PCODE_OPERAND_SIZE
CSE_MODE = 0x5E18A8
CSE_LOWER_BOUND = 0x5EB218
CSE_UPPER_BOUND = 0x5EA650
MAX_CSE_OBSERVATIONS = 8192
MAX_MACHINE_EMISSIONS = 16384
MAX_MACHINE_COLOR_OBSERVATIONS = 131072
IG_TABLE = 0x5EA768
# The allocator keeps one graph count and one register limit per coloring
# class.  Pinned GC/2.6 static evidence at function 0x4D06E0 stores class-name
# pointers at 0x5EA81C + 4 * class: class 2 is ``VR`` (0x5B9AC8), class 3 is
# ``FPR`` (0x5B9AD4), and class 4 is ``GPR`` (0x5B9AE0).  The adjacent format
# pointers are ``vr%ld``, ``f%ld``, and ``r%ld`` respectively.  This observer
# therefore supports only classes 3 (FPR) and 4 (GPR).
IG_COUNT_BASE = 0x5EAA2C
REGISTER_LIMIT_BASE = 0x5EA710
REGALLOC_CLASS_IDS = {"fpr": 3, "gpr": 4}
REGALLOC_CLASS_NAMES = {3: "FPR", 4: "GPR"}
GPR_COUNT = IG_COUNT_BASE + 4 * 4
FPR_COUNT = IG_COUNT_BASE + 4 * 3
GPR_LIMIT = REGISTER_LIMIT_BASE + 4 * 4
FPR_LIMIT = REGISTER_LIMIT_BASE + 4 * 3
COLORING_CLASS = 0x5EB2CF
FRONTEND_HOOK_BYTES = {
    0x433566: bytes.fromhex("ffb424d4000000"),
    0x433599: bytes.fromhex("e8429f0a00"),
    0x43359E: bytes.fromhex("59803d98b05e0000"),
}
FRONTEND_STAGES = dict(zip(FRONTEND_HOOK_BYTES, ("initial", "optimized", "final")))
# The temporary factory returns its completed Object in EBX.  This hook is at
# the function epilogue immediately before ``mov eax, ebx; pop ebx; ret``.
# Keep it separate from the three AST-stage hooks: temporary creation is a
# provenance lane, not another frontend stage.
TEMP_ORIGIN_HOOK = 0x004F5BA2
TEMP_ORIGIN_FACTORY = 0x004F5B30
TEMP_ORIGIN_HOOK_BYTES = {
    TEMP_ORIGIN_HOOK: bytes.fromhex("89d85bc3"),
}
MAX_TEMPORARY_ORIGINS = 2048
TEMPORARY_OBJECT_HEADER_SIZE = 0x2E
TEMPORARY_STACK_SIZE = 8
TEMPORARY_NAME_LIMIT = 256
RANGE_SPLIT_CALLER_RETURN = 0x0045DEDF
RANGE_SPLIT_CALLER_PREFIX_ADDRESS = 0x0045DED3
RANGE_SPLIT_CALLER_PREFIX = bytes.fromhex("8b6b028b450e50e8517c0900")
RANGE_SPLIT_VARIABLE_HEADER_SIZE = 0x22
RANGE_SPLIT_VARIABLE_OBJECT = 0x02
RANGE_SPLIT_MEMBER_HEADER_SIZE = 0x18
RANGE_SPLIT_USE_HEADER_SIZE = 0x1C
RANGE_SPLIT_MEMBER_EXPRESSION = 0x08
RANGE_SPLIT_MEMBER_PARENT = 0x0C
RANGE_SPLIT_MEMBER_NEXT = 0x14
RANGE_SPLIT_USE_REACHING_DEFS = 0x18
RANGE_SPLIT_LIST_LIMIT = 4096
RANGE_SPLIT_BITSET_WORD_LIMIT = 4096
RANGE_SPLIT_EXPRESSION_LIMIT = 4096
RANGE_SPLIT_IRO_COMMON_HEADER_SIZE = 0x20
RANGE_SPLIT_IRO_LEAF_SIZE = 0x24
RANGE_SPLIT_IRO_UNARY_SIZE = 0x24
RANGE_SPLIT_IRO_BINARY_SIZE = 0x28
RANGE_SPLIT_FRONTEND_EXPRESSION_SIZE = 0x1A
RANGE_SPLIT_BITSETS = {
    "remaining_defs": 0x005E00AC,
    "remaining_uses": 0x005E00A8,
    "selected_defs": 0x005E00A4,
    "selected_uses": 0x005E00A0,
}
NODE_NAMES = 0x5BC980
NODE_NAME_COUNT = 77


def remaining_color_mask(initial: int, colors: Iterable[int], limit: int) -> int:
    """Replay the pinned selector's mask operation, including signed colors."""
    if not 1 <= limit <= 32 or not 0 <= initial <= 0xFFFFFFFF:
        raise ValueError("invalid register limit or initial color mask")
    mask = initial
    for color in colors:
        if not -32768 <= color <= 32767:
            raise ValueError("neighbor color is not signed16")
        if color != -1 and color < limit:
            mask &= ~(1 << (color & 31)) & 0xFFFFFFFF
    return mask


def validate_color_selection(initial: int, colors: Iterable[int], limit: int,
                             observed_mask: int, selected: int) -> None:
    expected = remaining_color_mask(initial, colors, limit)
    if expected != observed_mask:
        raise ValueError("allocator available-mask mismatch")
    first = next((i for i in range(limit) if expected & (1 << i)), None)
    if first is None or selected != first:
        raise ValueError("allocator did not select the first available color")

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_COMPILER = REPO_ROOT / "build" / "compilers" / "GC" / "2.6" / "mwcceppc.exe"
DEFAULT_OUTPUT = (
    REPO_ROOT / "build" / "recovery" / "mwcc-win32-varinfo" / "mbTelopTimeSprRotSet.json"
)

GFUNCTION = 0x005E9EC0
LOCALS_LIST = 0x005EA8D4
ARGUMENTS_LIST = 0x005EAA28
# Pinned O0 GPR allocator 0x435C39 scans this third ObjectList after
# arguments/locals. Its identity must come from object names, not PPC colors.
IMPLICIT_GPR_LIST = 0x005EA69C

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
    read_memory: Callable[[int, int], bytes], runtime_address: Callable[[int], int],
    expected_hooks: dict[int, bytes] | None = None,
) -> None:
    """Check all native hook sites before the first INT3 write."""
    mismatches: list[str] = []
    for absolute, expected in (EXPECTED_HOOK_BYTES if expected_hooks is None else expected_hooks).items():
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


def machine_emission_layout() -> tuple[dict[str, Any], int, int, int]:
    """Reuse the existing authenticated GC/2.6 hook; no new native sites."""
    if __package__:
        from . import capsule_same_session_capture as capture
    else:
        import capsule_same_session_capture as capture
    if capture.GC26_COMPILER_SHA256 != PINNED_COMPILER_SHA256:
        raise ValueError("machine-emission compiler profile mismatch")
    return (dict(capture.GC26_MACHINE_EMIT_HOOK), capture.PCODE_OPCODE_DESCRIPTOR_TABLE,
            capture.PCODE_OPCODE_DESCRIPTOR_STRIDE, capture.PCODE_OPCODE_DESCRIPTOR_BASE_OFFSET)


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
        capture_regalloc: bool = False,
        capture_frontend: bool = False,
        regalloc_class: int = 3,
        capture_cse: bool = False,
        capture_machine_emit: bool = False,
    ) -> None:
        if not isinstance(target_name, str) or not target_name.strip():
            raise ValueError("target_name must be a nonempty function name")
        if (not isinstance(regalloc_class, int) or isinstance(regalloc_class, bool)
                or regalloc_class not in REGALLOC_CLASS_NAMES):
            raise ValueError("regalloc_class must be 3 (FPR) or 4 (GPR)")
        if capture_frontend and not capture_regalloc:
            raise ValueError("capture_frontend requires capture_regalloc")
        if capture_cse and not capture_regalloc:
            raise ValueError("capture_cse requires capture_regalloc")
        if capture_cse and regalloc_class != REGALLOC_CLASS_IDS["gpr"]:
            raise ValueError("capture_cse requires GPR regalloc_class")
        if capture_machine_emit and (not capture_regalloc or regalloc_class != REGALLOC_CLASS_IDS["gpr"]):
            raise ValueError("capture_machine_emit requires GPR capture_regalloc")
        self.process = process
        self.output = output
        self.target_name = target_name
        self.trace = trace
        self.capture_assignments = capture_assignments
        self.capture_regalloc = capture_regalloc
        self.capture_frontend = capture_frontend
        self.regalloc_class = regalloc_class
        self.capture_cse = capture_cse
        self.capture_machine_emit = capture_machine_emit
        self.observation_hooks = dict(REGALLOC_HOOK_BYTES)
        if capture_frontend:
            self.observation_hooks.update(FRONTEND_HOOK_BYTES)
            self.observation_hooks.update(TEMP_ORIGIN_HOOK_BYTES)
        if capture_cse:
            self.observation_hooks.update(CSE_HOOK_BYTES)
        self.machine_hook = None
        self.machine_colors: dict[tuple[int, int], dict[str, Any]] = {}
        self.machine_tokens: dict[int, str] = {}
        if capture_machine_emit:
            if compiler_sha256 != PINNED_COMPILER_SHA256:
                raise ValueError("machine emission requires pinned GC/2.6 compiler")
            self.machine_hook, self.machine_descriptor_table, self.machine_descriptor_stride, self.machine_descriptor_offset = machine_emission_layout()
            self.observation_hooks[self.machine_hook["address"]] = bytes.fromhex(self.machine_hook["prefix"])
        self.regalloc_active = False
        self.regalloc_pass = 0
        self.regalloc_pending: dict[str, Any] | None = None
        self.cse_pending_eligibility: tuple[int, bytes, int] | None = None
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
            "capture_regalloc": capture_regalloc,
            "capture_frontend": capture_frontend,
            "known_image_base": KNOWN_IMAGE_BASE,
            "compiler_sha256": compiler_sha256,
            "timeout_seconds": timeout_seconds,
            "breakpoints": {
                "codegen_start": hex(CODEGEN_START),
                "allocate_local_FPRs": hex(ALLOCATE_LOCAL_FPRS),
                "assign_local_FPR": hex(ASSIGN_LOCAL_FPR),
            },
        }
        if capture_frontend:
            self.result["temporary_origins"] = []
        if capture_cse:
            self.result["capture_cse"] = True
            self.result["cse_li_eligibility"] = []
            self.result["cse_li_reuse"] = []
        if capture_machine_emit:
            self.result.update(capture_machine_emit=True, machine_emissions=[],
                               machine_emission_scope="target-function; same-session PCode identity only",
                               authority_advanced=False)

    @property
    def regalloc_class_label(self) -> str:
        return REGALLOC_CLASS_NAMES[self.regalloc_class]

    @property
    def regalloc_count_address(self) -> int:
        return IG_COUNT_BASE + 4 * self.regalloc_class

    @property
    def regalloc_limit_address(self) -> int:
        return REGISTER_LIMIT_BASE + 4 * self.regalloc_class

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
        hooks = dict(EXPECTED_HOOK_BYTES)
        if self.capture_regalloc:
            hooks.update(self.observation_hooks)
        validate_hook_bytes(self.read, self.runtime, hooks)
        self.result["validated_hooks"] = {hex(k): v.hex() for k, v in hooks.items()}
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
        if self.capture_regalloc:
            if self.regalloc_active:
                if self.regalloc_pending is not None:
                    raise RuntimeError("function boundary with incomplete allocation observation")
                self.cse_pending_eligibility = None
                for site in self.observation_hooks:
                    self.remove_breakpoint(self.runtime(site))
                self.regalloc_active = False
                self.result["regalloc_end_function"] = function_name
                self.step_over(event, address, rearm=False)
                return
            if function_name == self.target_name:
                self.target_seen = True
                self.regalloc_active = True
                self.cse_pending_eligibility = None
                for site in self.observation_hooks:
                    self.install_breakpoint(site)
            self.step_over(event, address, rearm=True)
            return
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

    def read_exact(self, address: int, size: int) -> bytes:
        data = self.read(address, size)
        if len(data) != size:
            raise ValueError(f"truncated allocator read at {address:#x}, wanted {size} bytes")
        return data

    def frontend_snapshot(self, head: int) -> dict[str, Any]:
        """Read the GC/2.6 statement/expression graph without evaluating it.

        Layout reference: cadmic/mwcc-debugger's GC/2.6 AST reader. Preserve
        addresses and raw fields so object identity, not a printed name, joins
        snapshots to allocation. Unsupported nodes are explicitly incomplete.
        """
        names_raw = self.read_exact(self.runtime(NODE_NAMES), NODE_NAME_COUNT * 4)
        names = [self.read_string(u32(names_raw, i * 4)) for i in range(NODE_NAME_COUNT)]
        unary = set(("EPOSTINC EPOSTDEC EPREINC EPREDEC EINDIRECT EMONMIN EBINNOT "
                     "ELOGNOT EFORCELOAD ETYPCON EBITFIELD").split())
        binary = set(("EMUL EMULV EDIV EMODULO EADDV ESUBV EADD ESUB ESHL ESHR "
                      "ELESS EGREATER ELESSEQU EGREATEREQU EEQU ENOTEQU EAND EXOR EOR "
                      "ELAND ELOR EASS EMULASS EDIVASS EMODASS EADDASS ESUBASS "
                      "ESHLASS ESHRASS EANDASS EXORASS EORASS EBCLR EBSET ECOMMA "
                      "EPMODULO EROTL EROTR EBTST ENULLCHECK").split())
        statements, expressions, pending, seen_statements = [], {}, [], set()
        while head:
            if head in seen_statements or len(seen_statements) >= 16384:
                raise ValueError("cyclic or excessive frontend statement list")
            seen_statements.add(head)
            raw = self.read_exact(head, 0x1A)
            kind, expr = raw[4], u32(raw, 0xA)
            statements.append({"address": hex(head), "kind": kind, "line": s32(raw, 0x16),
                               "expression": hex(expr), "raw": raw.hex()})
            if kind in (4, 5, 6, 7, 8, 12, 13, 14, 15) and expr:
                pending.append(expr)
            head = u32(raw)
        while pending:
            address = pending.pop()
            if address in expressions:
                continue
            if len(expressions) >= 65536:
                raise ValueError("excessive frontend expression graph")
            raw = self.read_exact(address, 0x1A)
            if raw[0] >= len(names):
                raise ValueError("frontend expression kind outside name table")
            kind, type_address = names[raw[0]], u32(raw, 6)
            row = {"address": hex(address), "kind": kind, "type": hex(type_address),
                   "raw": raw.hex(), "children": []}
            if type_address:
                row["type_raw"] = self.read_exact(type_address, 7).hex()
            children = []
            if kind in unary:
                children = [u32(raw, 0xE)]
            elif kind in binary:
                children = [u32(raw, 0xE), u32(raw, 0x12)]
            elif kind in ("ECOND", "ECONDASS"):
                children = [u32(raw, offset) for offset in (0xE, 0x12, 0x16)]
            elif kind in ("EFUNCCALL", "EFUNCCALLP"):
                children = [u32(raw, 0xE)]
                arg, seen_args = u32(raw, 0x12), set()
                while arg:
                    if arg in seen_args or len(seen_args) >= 4096:
                        raise ValueError("cyclic or excessive frontend argument list")
                    seen_args.add(arg)
                    value = self.read_exact(arg, 8)
                    children.append(u32(value, 4))
                    arg = u32(value)
            elif kind == "EOBJREF":
                obj = u32(raw, 0xE)
                row.update(object=hex(obj), object_name=self.read_object_name(obj))
            elif kind not in ("EINTCONST", "EFLOATCONST", "ESTRINGCONST", "ELABEL"):
                row["incomplete"] = True
            if any(not child for child in children):
                raise ValueError("null frontend expression child")
            row["children"] = [hex(child) for child in children]
            expressions[address] = row
            pending.extend(children)
        incomplete = (not statements
                      or any(row["kind"] not in (1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15)
                             for row in statements)
                      or any(row.get("incomplete") for row in expressions.values()))
        return {"statements": statements, "expressions": list(expressions.values()),
                "incomplete": incomplete}

    def ig_node(self, pointer: int, *, neighbors: bool = False) -> dict[str, Any]:
        class_label = self.regalloc_class_label
        count = u32(self.read_exact(self.runtime(self.regalloc_count_address), 4))
        table = u32(self.read_exact(self.runtime(IG_TABLE), 4))
        if not table or not 1 <= count <= 4096:
            raise ValueError(f"invalid {class_label} graph table or count")
        raw = self.read_exact(pointer, 0x1A)
        index, color, degree = s16(raw, 0x10), s16(raw, 0x14), s16(raw, 0x18)
        if not 0 <= index < count or not 0 <= degree <= count:
            raise ValueError(f"invalid {class_label} node index or neighbor count")
        if u32(self.read_exact(table + index * 4, 4)) != pointer:
            raise ValueError(f"{class_label} node/table identity mismatch")
        obj = u32(raw, 4)
        row: dict[str, Any] = {
            "node": hex(pointer), "vreg": index, "color": color,
            "next": hex(u32(raw)), "object": hex(obj),
            "object_name": self.read_object_name(obj) if obj else None,
            "flags": int.from_bytes(raw[0x16:0x18], "little"),
            "cost": s32(raw, 0x0C), "remaining_degree": s16(raw, 0x12),
            "neighbor_count": degree, "graph": hex(table), "graph_count": count,
        }
        if neighbors:
            raw_neighbors = self.read_exact(pointer + 0x1A, degree * 2) if degree else b""
            adjacent = []
            for offset in range(0, degree * 2, 2):
                neighbor = s16(raw_neighbors, offset)
                if not 0 <= neighbor < count:
                    raise ValueError(f"{class_label} neighbor index outside graph")
                address = u32(self.read_exact(table + neighbor * 4, 4))
                adjacent.append(self.ig_node(address))
            row["neighbors"] = adjacent
        return row

    def cse_read_exact(self, address: int, size: int) -> bytes:
        data = self.read(address, size)
        if len(data) != size:
            raise ValueError(
                f"truncated CSE read at {address:#x}, wanted {size} bytes"
            )
        return data

    def cse_pcode_snapshot(
        self, pcode_address: int
    ) -> tuple[bytes, int, int] | None:
        """Read and validate the LI/LIS PCode shape used by the CSE hooks."""
        if not pcode_address:
            raise ValueError("CSE hook has a null PCode pointer")
        header = self.cse_read_exact(pcode_address, CSE_PCODE_HEADER_SIZE)
        opcode = s16(header, 0x20)
        if opcode not in CSE_LI_OPCODES:
            return None
        operand_count = s16(header, 0x22)
        if operand_count != 2:
            raise ValueError("LI/LIS PCode must have exactly two operands")
        operands = self.cse_read_exact(
            pcode_address + CSE_PCODE_HEADER_SIZE,
            operand_count * CSE_PCODE_OPERAND_SIZE,
        )
        raw = header + operands
        first = raw[CSE_PCODE_HEADER_SIZE : CSE_PCODE_HEADER_SIZE + CSE_PCODE_OPERAND_SIZE]
        if first[0] != 0 or first[1] != REGALLOC_CLASS_IDS["gpr"]:
            raise ValueError("LI/LIS destination operand kind/class mismatch")
        return raw, opcode, s16(first, 4)

    def observe_cse_eligibility(self, event: DEBUG_EVENT) -> None:
        """Capture the authenticated result of the LI/LIS eligibility gate."""
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError("CSE eligibility breakpoint without thread handle")
        context = self.get_context(thread)
        pcode_address = u32(self.cse_read_exact(context.Esp, 4))
        snapshot = self.cse_pcode_snapshot(pcode_address)
        if snapshot is None:
            self.cse_pending_eligibility = None
            return
        raw, opcode, destination_vreg = snapshot
        result_eax = int(context.Eax)
        if result_eax not in (0, 1):
            raise ValueError("LI/LIS eligibility returned non-boolean EAX")
        mode = s32(self.cse_read_exact(self.runtime(CSE_MODE), 4))
        lower_bound = s16(self.cse_read_exact(self.runtime(CSE_LOWER_BOUND), 2))
        upper_bound = s32(self.cse_read_exact(self.runtime(CSE_UPPER_BOUND), 4))
        in_generated_interval = lower_bound <= destination_vreg <= upper_bound
        if result_eax and not in_generated_interval:
            raise ValueError("LI/LIS eligibility succeeded outside generated interval")
        if result_eax and mode == 0:
            raise ValueError("LI/LIS eligibility succeeded with zero CSE mode")

        rows = self.result.setdefault("cse_li_eligibility", [])
        if len(rows) >= MAX_CSE_OBSERVATIONS:
            raise ValueError("excessive CSE LI eligibility observations")
        index = len(rows)
        rows.append(
            {
                "pcode": hex(pcode_address),
                "raw": raw.hex(),
                "opcode": opcode,
                "destination_vreg": destination_vreg,
                "mode": mode,
                "lower_bound": lower_bound,
                "upper_bound": upper_bound,
                "result_eax": result_eax,
                "in_generated_interval": in_generated_interval,
            }
        )
        self.cse_pending_eligibility = (
            pcode_address,
            raw,
            index,
        ) if result_eax else None

    def observe_cse_lookup(self, event: DEBUG_EVENT) -> None:
        """Capture an LI/LIS CSE lookup and its initialized output operand."""
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError("CSE lookup breakpoint without thread handle")
        context = self.get_context(thread)
        pcode_address = u32(self.cse_read_exact(context.Esp + 0xC, 4))
        snapshot = self.cse_pcode_snapshot(pcode_address)
        if snapshot is None:
            self.cse_pending_eligibility = None
            return
        raw, opcode, destination_vreg = snapshot
        found_eax = int(context.Eax)
        if found_eax not in (0, 1):
            raise ValueError("LI/LIS CSE lookup returned non-boolean EAX")
        pending = self.cse_pending_eligibility
        if (
            pending is None
            or pending[0] != pcode_address
            or pending[1] != raw
        ):
            self.cse_pending_eligibility = None
            raise ValueError("LI/LIS CSE lookup has no matching successful eligibility")
        rows = self.result.setdefault("cse_li_reuse", [])
        if len(rows) >= MAX_CSE_OBSERVATIONS:
            raise ValueError("excessive CSE LI reuse observations")

        row: dict[str, Any] = {
            "pcode": hex(pcode_address),
            "raw": raw.hex(),
            "opcode": opcode,
            "destination_vreg": destination_vreg,
            "found_eax": found_eax,
            "eligibility_index": pending[2],
        }
        if found_eax:
            output_operand = self.cse_read_exact(context.Esp + 0x14, CSE_PCODE_OPERAND_SIZE)
            if (
                output_operand[0] != 0
                or output_operand[1] != REGALLOC_CLASS_IDS["gpr"]
            ):
                raise ValueError("LI/LIS CSE output operand kind/class mismatch")
            row["output_operand"] = {
                "raw": output_operand.hex(),
                "kind": output_operand[0],
                "class": output_operand[1],
                "source_vreg": s16(output_operand, 4),
            }
        rows.append(row)
        self.cse_pending_eligibility = None

    def machine_pcode_token(self, address: int) -> str:
        if not address:
            raise ValueError("machine emission has null PCode identity")
        if address not in self.machine_tokens:
            if len(self.machine_tokens) >= MAX_MACHINE_EMISSIONS:
                raise ValueError("excessive machine PCode identities")
            self.machine_tokens[address] = f"pcode-{len(self.machine_tokens):06d}"
        return self.machine_tokens[address]

    def observe_machine_emission(self, event: DEBUG_EVENT) -> None:
        """GC/2.6 post-encoder: EBX=PCode, EBP=offset, EAX=encoded bytes.

        All operands are serialized, including raw flags and nonregister forms.
        Only a matching observed pre-color write supplies an original GPR vreg;
        physical-only operands or later rewrites never acquire invented ancestry.
        """
        if not self.capture_machine_emit or not self.regalloc_active:
            raise RuntimeError("machine emission outside requested target capture")
        emissions = self.result["machine_emissions"]
        if len(emissions) >= MAX_MACHINE_EMISSIONS:
            raise ValueError("excessive machine emission observations")
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError("machine emission without thread handle")
        context = self.get_context(thread)
        pointer, offset, encoded = int(context.Ebx), int(context.Ebp), int(context.Eax)
        token = self.machine_pcode_token(pointer)
        if offset < 0 or offset % 4 or (emissions and offset <= emissions[-1]["emitted_offset"]):
            raise ValueError("machine emitted offsets are unaligned or nonmonotonic")
        if not 0 <= encoded <= 0xFFFFFFFF:
            raise ValueError("invalid machine encoded word")
        header = self.read_exact(pointer, 0x24)
        opcode, count = s16(header, 0x20), s16(header, 0x22)
        if not 0 <= opcode <= 0x1D4 or not 0 <= count <= 256:
            raise ValueError("unsupported machine PCode opcode/count")
        raw = self.read_exact(pointer + 0x24, count * 12) if count else b""
        descriptor = self.runtime(self.machine_descriptor_table + opcode * self.machine_descriptor_stride
                                  + self.machine_descriptor_offset)
        descriptor_base = u32(self.read_exact(descriptor, 4))
        encoded_bytes = encoded.to_bytes(4, "little")
        word = int.from_bytes(encoded_bytes, "big")
        if (word & 0xFC000000) != (descriptor_base & 0xFC000000):
            raise ValueError("machine descriptor opcode mismatch")
        operands = []
        for ordinal in range(count):
            operand = raw[ordinal * 12:(ordinal + 1) * 12]
            kind, cls, flags, index = operand[0], operand[1], int.from_bytes(operand[2:4], "little"), s16(operand, 4)
            item = {"ordinal": ordinal, "kind": kind, "class": cls, "flags": flags,
                    "index": index, "raw": operand.hex(), "vreg": None, "color": None,
                    "join_status": "not_gpr"}
            if kind == 0 and cls == REGALLOC_CLASS_IDS["gpr"]:
                if not 0 <= index <= 31:
                    raise ValueError("emitted GPR operand is not a physical color")
                item.update(color=index, join_status="UNKNOWN", reason="no observed pre-color operand")
                prior = self.machine_colors.get((pointer, ordinal))
                if prior:
                    prior_raw = bytes.fromhex(prior["operand_raw"])
                    if (prior["opcode"] == opcode and prior["operand_count"] == count
                            and prior["source_offset"] == u32(header, 0x1C)
                            and prior["color"] == index
                            and prior_raw[:4] == operand[:4] and prior_raw[6:] == operand[6:]):
                        item.update(vreg=prior["operand_index"], join_status="observed",
                                    color_observation=prior["observation_index"],
                                    allocation_pass=prior["pass"], pre_color_flags=prior["operand_flags"])
                        item.pop("reason")
                    else:
                        item["reason"] = "PCode operand changed since observed color write"
            operands.append(item)
        emissions.append({"function": self.target_name, "pcode": hex(pointer), "pcode_token": token, "emitted_offset": offset,
                          "instruction_index": offset // 4, "opcode": opcode, "operand_count": count,
                          "source_offset": u32(header, 0x1C), "ppc_word": word,
                          "ppc_bytes": encoded_bytes.hex(), "operands": operands,
                          "gpr_joins_complete": all(x["join_status"] != "UNKNOWN" for x in operands),
                          "authority_advanced": False})

    def observe_regalloc(self, event: DEBUG_EVENT, address: int) -> None:
        if not self.regalloc_active:
            raise RuntimeError("optimized allocator observation outside target function")
        site = address - self.base + KNOWN_IMAGE_BASE
        if self.machine_hook and site == self.machine_hook["address"]:
            self.observe_machine_emission(event)
            return
        if site in FRONTEND_HOOK_BYTES:
            if not self.capture_frontend:
                raise RuntimeError("frontend observation was not requested")
            context = self.get_context(self.threads[event.dwThreadId])
            snapshot = self.frontend_snapshot(u32(self.read_exact(context.Esp, 4)))
            snapshot["stage"] = FRONTEND_STAGES[site]
            self.result.setdefault("frontend", []).append(snapshot)
            return
        if site in TEMP_ORIGIN_HOOK_BYTES:
            if not self.capture_frontend:
                raise RuntimeError("temporary-origin observation was not requested")
            self.observe_temporary_origin(event)
            return
        if site in CSE_HOOK_BYTES:
            if not self.capture_cse:
                raise RuntimeError("CSE observation was not requested")
            if site == CSE_LI_ELIGIBILITY_HOOK:
                self.observe_cse_eligibility(event)
            else:
                self.observe_cse_lookup(event)
            return
        cls = self.read_exact(self.runtime(COLORING_CLASS), 1)[0]
        if site == 0x508890:
            self.log(
                f"REGALLOC_SELECTOR target={self.target_name} "
                f"COLORING_CLASS={cls} requested_class={self.regalloc_class}"
            )
        if cls != self.regalloc_class:
            return
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError("allocator breakpoint without thread handle")
        context = self.get_context(thread)
        site = address - self.base + KNOWN_IMAGE_BASE
        if site == 0x508890:
            if self.regalloc_pending is not None:
                raise RuntimeError("new selector pass with unfinished node")
            self.regalloc_pass += 1
            self.result.setdefault("regalloc_passes", []).append({
                "pass": self.regalloc_pass, "class": cls,
                "head": hex(u32(self.read_exact(context.Esp + 4, 4))),
            })
            return
        if site == 0x5087A4:
            header = self.read_exact(context.Esi, 0x24)
            count = s16(header, 0x22)
            offset = context.Edx - context.Esi - 0x24
            if not 1 <= count <= 256 or offset < 0 or offset % 12 or offset // 12 >= count:
                raise ValueError("invalid PCode operand location")
            operand = self.read_exact(context.Edx, 12)
            node = self.ig_node(context.Ecx)
            index = s16(operand, 4)
            table = int(node["graph"], 16)
            if (operand[0] != 0 or operand[1] != cls or not 0 <= index < node["graph_count"]
                    or u32(self.read_exact(table + index * 4, 4)) != context.Ecx
                    or node["color"] != (context.Eax & 0xFFFF)):
                raise ValueError("PCode operand/IG/color join mismatch")
            observations = self.result.setdefault("regalloc_pcode", [])
            if self.capture_machine_emit and len(observations) >= MAX_MACHINE_COLOR_OBSERVATIONS:
                raise ValueError("excessive machine color observations")
            row = {
                "pass": self.regalloc_pass, "class": cls, "node": node["node"],
                "node_vreg": node["vreg"], "pcode": hex(context.Esi),
                "opcode": s16(header, 0x20), "operand_count": count,
                "operand_ordinal": offset // 12, "operand_index": index,
                "operand_flags": int.from_bytes(operand[2:4], "little"),
                "color": node["color"],
            }
            if self.capture_machine_emit:
                row.update(pcode_token=self.machine_pcode_token(context.Esi),
                           source_offset=u32(header, 0x1C), operand_raw=operand.hex(),
                           observation_index=len(observations))
                self.machine_colors[(int(context.Esi), offset // 12)] = row
            observations.append(row)
            return
        if site == 0x5088C6:
            if self.regalloc_pending is not None:
                raise RuntimeError("new node before previous allocation completed")
            limit = u32(self.read_exact(self.runtime(self.regalloc_limit_address), 4))
            if not 1 <= limit <= 32:
                raise ValueError(f"invalid {self.regalloc_class_label} register limit")
            node = self.ig_node(context.Ebx, neighbors=True)
            row = {
                "pass": self.regalloc_pass, "class": cls, "thread": event.dwThreadId,
                "order": len(self.result.setdefault("regalloc_selections", [])),
                "node": node, "initial_mask": u32(self.read_exact(context.Esp, 4)),
                "register_limit": limit, "status": "pending",
            }
            self.result["regalloc_selections"].append(row)
            self.regalloc_pending = row
            return
        pending = self.regalloc_pending
        if pending is None or pending["thread"] != event.dwThreadId or pending["node"]["node"] != hex(context.Ebx):
            raise ValueError("allocator node/thread pre/post identity mismatch")
        if site in (0x50892E, 0x508954, 0x508987):
            if "outcome" in pending:
                raise ValueError("duplicate allocator outcome")
            colors = [x["color"] for x in pending["node"]["neighbors"]]
            expected = remaining_color_mask(pending["initial_mask"], colors, pending["register_limit"])
            if site == 0x50892E:
                selected = context.Ecx & 0xFFFF
                validate_color_selection(pending["initial_mask"], colors,
                                         pending["register_limit"], context.Edx, selected)
                pending.update(outcome="first_available", selected=selected, final_mask=context.Edx)
            else:
                if expected != 0:
                    raise ValueError("allocator fallback with nonempty available mask")
                if site == 0x508954:
                    selected = context.Eax & 0xFFFF
                    if not 0 <= selected < pending["register_limit"]:
                        raise ValueError("allocator fallback selected invalid color")
                    pending.update(outcome="new_register", selected=selected, final_mask=0)
                else:
                    pending.update(outcome="spill", final_mask=0)
            return
        if site in (0x508932, 0x508958):
            expected_outcome = "first_available" if site == 0x508932 else "new_register"
            if pending.get("outcome") != expected_outcome:
                raise ValueError("allocator store has no matching pre-observation")
            node = self.ig_node(context.Ebx)
            if node["color"] != pending["selected"]:
                raise ValueError("allocator stored color mismatch")
            pending["store_verified"] = True
            return
        if site == 0x508994:
            if pending.get("outcome") == "spill":
                if not self.ig_node(context.Ebx)["flags"] & 1:
                    raise ValueError("allocator spill flag was not stored")
            elif not pending.get("store_verified"):
                raise ValueError("allocator node finished without verified outcome")
            pending["status"] = "observed"
            self.regalloc_pending = None
            self.dumped = True
            return
        raise ValueError("unsupported optimized allocator hook")

    def read_bounded_string(self, address: int, limit: int = TEMPORARY_NAME_LIMIT) -> str:
        """Read a compiler string only when its terminator is in the bound."""
        data = self.read_exact(address, limit)
        terminator = data.find(b"\0")
        if terminator < 0:
            raise ValueError(
                f"temporary name is not NUL-terminated within {limit} bytes"
            )
        return data[:terminator].decode("latin-1", errors="replace")

    def range_split_bitset(
        self, pointer: int, label: str, pointer_raw: bytes | None = None
    ) -> dict[str, Any]:
        if not pointer:
            raise ValueError(f"range-split {label} bitset pointer is null")
        count_raw = self.read_exact(pointer, 4)
        word_count = u32(count_raw)
        if word_count > RANGE_SPLIT_BITSET_WORD_LIMIT:
            raise ValueError(
                f"range-split {label} bitset word count exceeds "
                f"{RANGE_SPLIT_BITSET_WORD_LIMIT}"
            )
        words_raw = self.read_exact(pointer + 4, word_count * 4) if word_count else b""
        words = [u32(words_raw, offset) for offset in range(0, len(words_raw), 4)]
        set_indices = [
            word_index * 32 + bit
            for word_index, word in enumerate(words)
            for bit in range(32)
            if word & (1 << bit)
        ]
        record: dict[str, Any] = {
            "pointer": hex(pointer),
            "word_count": word_count,
            "words": words,
            "raw": (count_raw + words_raw).hex(),
            "set_indices": set_indices,
        }
        if pointer_raw is not None:
            record["pointer_raw"] = pointer_raw.hex()
        return record

    def range_split_global_bitset(self, absolute: int, label: str) -> dict[str, Any]:
        pointer_raw = self.read_exact(self.runtime(absolute), 4)
        record = self.range_split_bitset(u32(pointer_raw), label, pointer_raw)
        record["global"] = hex(absolute)
        return record

    def range_split_frontend_leaf(self, address: int) -> dict[str, Any]:
        raw = self.read_exact(address, RANGE_SPLIT_FRONTEND_EXPRESSION_SIZE)
        rawkind = raw[0]
        record: dict[str, Any] = {
            "address": hex(address),
            "rawkind": rawkind,
            "raw": raw.hex(),
            "object": None,
            "object_name": None,
        }
        if rawkind != 0x38:
            # Only EOBJREF is decoded here; preserving raw bytes does not
            # establish the child structure of other frontend node kinds.
            record["incomplete"] = True
            return record
        object_address = u32(raw, 0x0E)
        record["object"] = hex(object_address) if object_address else None
        if not object_address:
            record["incomplete"] = True
            return record
        name_hash_raw = self.read_exact(object_address + OBJECT_NAME, 4)
        name_hash = u32(name_hash_raw)
        record["object_name_hash"] = hex(name_hash) if name_hash else None
        if name_hash:
            record["object_name"] = self.read_bounded_string(name_hash + OBJECT_NAME)
        return record

    def range_split_expression_graph(self, root: int) -> dict[str, Any]:
        nodes: dict[int, dict[str, Any]] = {}
        discovered: set[int] = set()
        active: set[int] = set()
        incomplete = False
        # Keep traversal iterative: a compiler-generated expression chain can
        # be deeper than Python's recursion limit, while the discovered-node
        # bound still keeps the snapshot finite.
        pending: list[tuple[int, bool]] = [(root, False)] if root else []
        while pending:
            address, exiting = pending.pop()
            if exiting:
                active.remove(address)
                continue
            if not address:
                continue
            if address in active:
                raise ValueError("cyclic range-split IRO expression graph")
            if address in discovered:
                continue
            if len(discovered) >= RANGE_SPLIT_EXPRESSION_LIMIT:
                raise ValueError("excessive range-split IRO expression graph")
            discovered.add(address)
            active.add(address)
            common = self.read_exact(address, RANGE_SPLIT_IRO_COMMON_HEADER_SIZE)
            kind = common[0]
            children: list[int] = []
            if kind == 1:
                tail = self.read_exact(address + 0x20, 4)
                frontend_address = u32(tail)
                row: dict[str, Any] = {
                    "address": hex(address),
                    "kind": 1,
                    "raw": (common + tail).hex(),
                    "children": [],
                    "frontend_expression": (
                        hex(frontend_address) if frontend_address else None
                    ),
                }
                if frontend_address:
                    row["frontend_ast"] = self.range_split_frontend_leaf(frontend_address)
                    if row["frontend_ast"].get("incomplete"):
                        row["incomplete"] = True
                        incomplete = True
                else:
                    row["frontend_ast"] = None
                    row["incomplete"] = True
                    incomplete = True
            elif kind == 2:
                tail = self.read_exact(address + 0x20, 4)
                child = u32(tail)
                children = [child]
                row = {
                    "address": hex(address),
                    "kind": 2,
                    "raw": (common + tail).hex(),
                    "children": [hex(child) if child else None],
                }
                if not child:
                    row["incomplete"] = True
                    incomplete = True
            elif kind == 3:
                tail = self.read_exact(address + 0x20, 8)
                children = [u32(tail, 0), u32(tail, 4)]
                row = {
                    "address": hex(address),
                    "kind": 3,
                    "raw": (common + tail).hex(),
                    "children": [hex(child) if child else None for child in children],
                }
                if any(not child for child in children):
                    row["incomplete"] = True
                    incomplete = True
            else:
                row = {
                    "address": hex(address),
                    "kind": kind,
                    "raw_header": common.hex(),
                    "raw": common.hex(),
                    "children": [],
                    "incomplete": True,
                }
                incomplete = True
            nodes[address] = row
            pending.append((address, True))
            for child in reversed(children):
                if child:
                    pending.append((child, False))
        return {
            "root": hex(root) if root else None,
            "nodes": list(nodes.values()),
            "incomplete": incomplete,
        }

    def range_split_member_list(
        self, variable_address: int, head: int, *, uses: bool
    ) -> list[dict[str, Any]]:
        records: list[dict[str, Any]] = []
        seen_nodes: set[int] = set()
        seen_indices: set[int] = set()
        header_size = RANGE_SPLIT_USE_HEADER_SIZE if uses else RANGE_SPLIT_MEMBER_HEADER_SIZE
        label = "uses" if uses else "definitions"
        while head:
            if len(records) >= RANGE_SPLIT_LIST_LIMIT:
                raise ValueError(f"excessive range-split {label} list")
            if head in seen_nodes:
                raise ValueError(f"cyclic range-split {label} list")
            seen_nodes.add(head)
            raw = self.read_exact(head, header_size)
            index = u32(raw)
            if index in seen_indices:
                raise ValueError(f"duplicate range-split {label} index {index}")
            seen_indices.add(index)
            parent = u32(raw, RANGE_SPLIT_MEMBER_PARENT)
            if parent != variable_address:
                raise ValueError(f"range-split {label} member parent mismatch")
            expression = u32(raw, RANGE_SPLIT_MEMBER_EXPRESSION)
            next_address = u32(raw, RANGE_SPLIT_MEMBER_NEXT)
            record: dict[str, Any] = {
                "address": hex(head),
                "index": index,
                "expression": hex(expression) if expression else None,
                "parent": hex(parent),
                "next": hex(next_address) if next_address else None,
                "raw": raw.hex(),
            }
            if uses:
                reaching_pointer = u32(raw, RANGE_SPLIT_USE_REACHING_DEFS)
                record["reaching_defs_pointer"] = (
                    hex(reaching_pointer) if reaching_pointer else None
                )
                record["reaching_defs"] = (
                    self.range_split_bitset(reaching_pointer, f"use {index} reaching defs")
                    if reaching_pointer
                    else None
                )
            if expression:
                record["expression_graph"] = self.range_split_expression_graph(expression)
            elif uses:
                raise ValueError(f"null range-split use expression at index {index}")
            else:
                record["expression_null"] = True
                record["expression_graph"] = None
            records.append(record)
            head = next_address
        return records

    def range_split_snapshot(
        self, variable_address: int, caller_prefix_address: int, caller_prefix: bytes
    ) -> dict[str, Any]:
        variable_header = self.read_exact(
            variable_address, RANGE_SPLIT_VARIABLE_HEADER_SIZE
        )
        old_object_address = u32(variable_header, RANGE_SPLIT_VARIABLE_OBJECT)
        if not old_object_address:
            raise ValueError("range-split variable has no old Object pointer")
        old_object_header = self.read_exact(
            old_object_address, TEMPORARY_OBJECT_HEADER_SIZE
        )
        old_name_hash = u32(old_object_header, OBJECT_NAME)
        old_name = (
            self.read_bounded_string(old_name_hash + OBJECT_NAME)
            if old_name_hash
            else None
        )
        old_type_address = u32(old_object_header, 0x0E)
        old_type_raw = (
            self.read_exact(old_type_address, 7) if old_type_address else None
        )
        old_varinfo_address = u32(old_object_header, OBJECT_VARINFO)
        old_object = {
            "object": hex(old_object_address),
            "header": old_object_header.hex(),
            "name": old_name,
            "name_hash": hex(old_name_hash) if old_name_hash else None,
            "type": hex(old_type_address) if old_type_address else None,
            "type_raw": old_type_raw.hex() if old_type_raw is not None else None,
            "varinfo": hex(old_varinfo_address) if old_varinfo_address else None,
        }
        bitsets = {
            label: self.range_split_global_bitset(absolute, label)
            for label, absolute in RANGE_SPLIT_BITSETS.items()
        }
        definitions_head = u32(variable_header, 0x12)
        uses_head = u32(variable_header, 0x16)
        definitions = self.range_split_member_list(
            variable_address, definitions_head, uses=False
        )
        uses = self.range_split_member_list(variable_address, uses_head, uses=True)
        return {
            "caller_prefix_address": hex(caller_prefix_address),
            "caller_prefix": caller_prefix.hex(),
            "variable": hex(variable_address),
            "variable_header": variable_header.hex(),
            "old_object": old_object,
            "bitsets": bitsets,
            "definitions_head": hex(definitions_head) if definitions_head else None,
            "uses_head": hex(uses_head) if uses_head else None,
            "definitions": definitions,
            "uses": uses,
        }

    def observe_temporary_origin(self, event: DEBUG_EVENT) -> None:
        """Record one completed local Object and its verified caller shape."""
        origins = self.result.setdefault("temporary_origins", [])
        if len(origins) >= MAX_TEMPORARY_ORIGINS:
            raise ValueError("excessive temporary origin observations")
        thread = self.threads.get(event.dwThreadId)
        if not thread:
            raise RuntimeError("temporary-origin breakpoint without thread handle")
        context = self.get_context(thread)
        object_address = int(context.Ebx)
        stack_address = int(context.Esp)
        object_header = self.read_exact(object_address, TEMPORARY_OBJECT_HEADER_SIZE)
        stack = self.read_exact(stack_address, TEMPORARY_STACK_SIZE)

        name_hash = u32(object_header, OBJECT_NAME)
        if not name_hash:
            raise ValueError("temporary Object has no generated name pointer")
        generated_name = self.read_bounded_string(name_hash + OBJECT_NAME)
        type_address = u32(object_header, 0x0E)
        type_raw = self.read_exact(type_address, 7) if type_address else None
        varinfo_address = u32(object_header, OBJECT_VARINFO)
        saved_ebx = u32(stack)
        return_address = u32(stack, 4)
        if return_address < 5:
            raise ValueError("temporary caller return address precedes call bytes")
        caller_preceding5 = self.read_exact(return_address - 5, 5)

        caller_call_target: int | None = None
        caller_call_shape = "unknown"
        direct_factory_call = False
        if caller_preceding5[0] == 0xE8:
            caller_call_target = (return_address + s32(caller_preceding5, 1)) & 0xFFFFFFFF
            caller_call_shape = "direct_e8"
            direct_factory_call = caller_call_target == self.runtime(TEMP_ORIGIN_FACTORY)

        normalized_return_address = (
            return_address - self.base + KNOWN_IMAGE_BASE
        ) & 0xFFFFFFFF
        range_split: dict[str, Any] | None = None
        if (
            direct_factory_call
            and normalized_return_address == RANGE_SPLIT_CALLER_RETURN
        ):
            caller_prefix_address = self.runtime(RANGE_SPLIT_CALLER_PREFIX_ADDRESS)
            caller_prefix = self.read_exact(
                caller_prefix_address, len(RANGE_SPLIT_CALLER_PREFIX)
            )
            if caller_prefix != RANGE_SPLIT_CALLER_PREFIX:
                raise ValueError(
                    "range-split caller prefix mismatch: "
                    f"expected {RANGE_SPLIT_CALLER_PREFIX.hex()}, got {caller_prefix.hex()}"
                )
            range_split = self.range_split_snapshot(
                saved_ebx, caller_prefix_address, caller_prefix
            )

        frontend = self.result.get("frontend", [])
        prior_frontend_stage = (
            frontend[-1].get("stage", "pre-initial") if frontend else "pre-initial"
        )
        origins.append(
            {
                "object": hex(object_address),
                "name": generated_name,
                "name_hash": hex(name_hash),
                "type": hex(type_address) if type_address else None,
                "type_raw": type_raw.hex() if type_raw is not None else None,
                "varinfo": hex(varinfo_address) if varinfo_address else None,
                "saved_ebx": hex(saved_ebx),
                "caller_return_address": hex(return_address),
                "normalized_caller_return_address": hex(normalized_return_address),
                "caller_preceding5": caller_preceding5.hex(),
                "caller_call_target": (
                    hex(caller_call_target) if caller_call_target is not None else None
                ),
                "caller_call_shape": caller_call_shape,
                "direct_factory_call": direct_factory_call,
                "prior_frontend_stage": prior_frontend_stage,
                "object_header": object_header.hex(),
                "stack": stack.hex(),
            }
        )
        if range_split is not None:
            origins[-1]["range_split"] = range_split

    def object_record(self, address: int) -> dict[str, Any]:
        datatype_data = self.read(address + OBJECT_DATATYPE, 1)
        datatype = datatype_data[0] if datatype_data else None
        # Pinned get_varinfo (0x4CFFE0): datatype 1 stores +0x2A;
        # datatype 0/2 (including compiler section objects) store +0x32.
        info_offset = {0: 0x32, 1: OBJECT_VARINFO, 2: 0x32}.get(datatype)
        info_address = self.read_u32(address + info_offset) if info_offset is not None else 0
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
                "implicit_gpr_objects": self.object_list(IMPLICIT_GPR_LIST),
                "implicit_gpr_evidence": {
                    "list_global": hex(IMPLICIT_GPR_LIST),
                    "scan_address": "0x435c39",
                    "eligibility": "reg == 0 and used != 0 and usage >= 3; noregister not tested for this list",
                    "usage_override": "flags & 0x40 forces usage to 100000 before ranking",
                    "ranking": "scanned after arguments/locals; usage >= current best wins",
                    "stage": "FPR entry, after O0 GPR allocation",
                    "missing_edges": [
                        "contributing source/data uses not captured",
                        "section-object to individual storage-symbol edges not captured",
                    ],
                },
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
                    if self.capture_regalloc:
                        if self.regalloc_pending is not None:
                            raise RuntimeError("compiler exited with unfinished allocator observation")
                        if not self.result.get("regalloc_selections") or not self.result.get("regalloc_pcode"):
                            raise RuntimeError(
                                f"compiler exited without target optimized "
                                f"{self.regalloc_class_label} evidence"
                            )
                        stages = [x["stage"] for x in self.result.get("frontend", [])]
                        if self.capture_frontend and stages != list(FRONTEND_STAGES.values()):
                            raise RuntimeError("compiler exited without all requested frontend stages")
                        if self.capture_machine_emit and not self.result.get("machine_emissions"):
                            raise RuntimeError("compiler exited without requested machine emissions")
                        self.result["status"] = "regalloc_observed"
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
                            elif self.capture_regalloc and exception_address - self.base + KNOWN_IMAGE_BASE in self.observation_hooks:
                                self.observe_regalloc(event, exception_address)
                                self.step_over(event, exception_address, rearm=True)
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
    parser.add_argument(
        "--regalloc", action="store_true",
        help="observe optimized GC/2.6 FPR/GPR graph selection and PCode color joins",
    )
    parser.add_argument(
        "--regalloc-class",
        choices=tuple(REGALLOC_CLASS_IDS),
        default="fpr",
        help="allocator register class to observe (gpr requires --regalloc)",
    )
    parser.add_argument(
        "--cse", action="store_true",
        help="with --regalloc --regalloc-class gpr, capture LI/LIS CSE observations",
    )
    parser.add_argument("--frontend", action="store_true",
                        help="with --regalloc, capture AST stages and temporary/range-split origins")
    parser.add_argument("--machine-emit", action="store_true",
                        help="with GPR --regalloc, capture emitted PPC words/offsets and operand color joins")
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
    if not args.target.strip():
        print("--target must be a nonempty function name", file=sys.stderr)
        return 2
    if args.regalloc and args.assign:
        print("--regalloc and --assign select different allocator stages", file=sys.stderr)
        return 2
    if args.regalloc_class == "gpr" and not args.regalloc:
        print("--regalloc-class gpr requires --regalloc", file=sys.stderr)
        return 2
    if args.cse and not args.regalloc:
        print("--cse requires --regalloc", file=sys.stderr)
        return 2
    if args.cse and args.regalloc_class != "gpr":
        print("--cse requires --regalloc-class gpr", file=sys.stderr)
        return 2
    if args.frontend and not args.regalloc:
        print("--frontend requires --regalloc", file=sys.stderr)
        return 2
    if args.machine_emit and (not args.regalloc or args.regalloc_class != "gpr"):
        print("--machine-emit requires --regalloc --regalloc-class gpr", file=sys.stderr)
        return 2
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
        capture_regalloc=args.regalloc,
        capture_frontend=args.frontend,
        regalloc_class=REGALLOC_CLASS_IDS[args.regalloc_class],
        capture_cse=args.cse,
        capture_machine_emit=args.machine_emit,
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
