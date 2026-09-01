#!/usr/bin/env python3
"""Capture authenticated Capsule or Player frontend/backend evidence in one native session.

The two older Capsule producers deliberately stop at different boundaries:
one observes the Object/VarInfo stack home and one observes PCode/register
allocation metadata.  This module is the narrow join boundary.  It launches
one compiler process, installs one authenticated union of breakpoints, and
sends every observation through one monotonic event bus.  The stack and PCode
lanes are still kept separate in the output so a consumer can hash and audit
them independently.

This is a diagnostic producer.  It never claims source ownership, never emits
raw addresses, and never changes source, build, queue, lease, or authority
state.  The native backend is intentionally small and protocol driven; fake
backends can exercise all chronology and fail-closed rules without Windows.
"""

from __future__ import annotations

import argparse
import ctypes
from ctypes import wintypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time
import uuid
from collections.abc import Callable, Iterable, Mapping, MutableMapping, Sequence
from typing import Any, Protocol

def _load_sibling_modules(names: Sequence[str]) -> tuple[Any, ...]:
    """Load tool dependencies as one private package rooted beside this file."""

    tool_dir = Path(__file__).resolve().parent
    paths: list[tuple[str, Path]] = []
    for name in names:
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
            raise ImportError(f"invalid same-directory tool dependency name: {name!r}")
        path = (tool_dir / f"{name}.py").resolve()
        if path.parent != tool_dir or not path.is_file():
            raise ImportError(f"missing same-directory tool dependency: {path}")
        paths.append((name, path))

    bundle_key = f"_capsule_same_session_capture_bundle_{uuid.uuid4().hex}"
    if bundle_key in sys.modules:
        raise ImportError("private same-directory tool package alias collision")
    bundle_spec = importlib.util.spec_from_loader(bundle_key, loader=None, is_package=True)
    if bundle_spec is None:
        raise ImportError(f"cannot create same-directory tool package: {tool_dir}")
    bundle = importlib.util.module_from_spec(bundle_spec)
    bundle.__path__ = [str(tool_dir)]
    sys.modules[bundle_key] = bundle
    loaded_keys: list[str] = []
    loaded_modules: list[Any] = []
    try:
        for name, path in paths:
            module_key = f"{bundle_key}.{name}"
            if module_key in sys.modules:
                raise ImportError(f"private tool dependency alias collision: {module_key}")
            spec = importlib.util.spec_from_file_location(module_key, path)
            if spec is None or spec.loader is None:
                raise ImportError(f"cannot load same-directory tool dependency: {path}")
            module = importlib.util.module_from_spec(spec)
            sys.modules[module_key] = module
            loaded_keys.append(module_key)
            spec.loader.exec_module(module)
            setattr(bundle, name, module)
            loaded_modules.append(module)
    except BaseException:
        for module_key in reversed(loaded_keys):
            sys.modules.pop(module_key, None)
        sys.modules.pop(bundle_key, None)
        raise
    return tuple(loaded_modules)


(
    _stack_home,
    _donor_cfg,
    _frontend_chronology,
    _correlator,
) = _load_sibling_modules(
    (
        "capsule_stack_home_native",
        "donor_cfg_align",
        "mwcc_fe_chronology_native",
        "pcode_varinfo_correlator",
    )
)


SCHEMA = "mwcc_capsule_same_session_capture/v1"
REQUEST_SCHEMA = "mwcc_capsule_same_session_capture_request/v1"
EVENT_SCHEMA = "mwcc_capsule_same_session_capture_event/v1"
SOURCE_SPAN_SCHEMA = "mwcc_source_span_bindings/v1"
SOURCE_SPAN_SCHEMA_V2 = "mwcc_source_span_bindings/v2"
SOURCE_SPAN_PLAN_SCHEMA = "mwcc_source_span_binding_plan/v1"
CAUSAL_MAP_SCHEMA = "mwcc_source_aware_causal_map/v1"
PARTIAL_EVIDENCE_SCHEMA = "mwcc_capsule_same_session_partial_evidence/v1"
PARTIAL_FAILURE_GRAPH_SCHEMA = "mwcc_capsule_same_session_ownership_failure_graph/v1"
PARTIAL_HOOK_RECEIPT_SCHEMA = "mwcc_capsule_same_session_hook_validation/v1"
COMPILER_EXECUTION_RECEIPT_SCHEMA = "mwcc_capsule_compiler_to_object_receipt/v1"
COMPILER_TERMINAL_STATES = frozenset({"SUCCESS", "UNKNOWN", "FAILED"})
# The pinned hook union changed: old requests/envelopes must not be
# interpreted as captures from this repaired transport.
TOOL_VERSION = "capsule-same-session-capture-2"
DIAGNOSTIC_ONLY = True
BOARD_ADMISSION = False
EXACTNESS_CLAIM = False
AUTHORITY_ADVANCED = False

_PRESERVABLE_FINAL_JOIN_FAILURES = frozenset(
    {
        "machine owner join lacks an exact Object/vreg edge",
        "machine owner join lacks an exact physical-register edge",
        "machine physical owner join lacks an exact same-session assignment",
    }
)
_PARTIAL_EVIDENCE_FILENAMES = {
    "stack_events": "stack.events.jsonl",
    "pcode_events": "pcode.events.jsonl",
    "machine_events": "machine.events.jsonl",
    "candidate_envelope": "candidate-envelope.json",
    "hook_validation": "hook-validation.json",
    "failure_graph": "ownership-failure-graph.json",
    "manifest": "partial-evidence.json",
}

KNOWN_IMAGE_BASE = _stack_home.KNOWN_IMAGE_BASE
# ``DEBUG_PROCESS`` follows a launcher through its descendant processes.  The
# wrapper-first MWCC command therefore needs this flag; DEBUG_ONLY_THIS_PROCESS
# would attach only to sjiswrap.exe and all compiler hooks would be read from
# the wrong address space.
DEBUG_PROCESS = 0x00000001
STARTF_USESTDHANDLES = 0x00000100
# sjiswrap does not create a child process.  It manually maps mwcceppc.exe
# into its own address space through memexec_exe_with_hooks.  The mapped image
# is therefore a MEM_PRIVATE allocation, not a normal loader module.
MEM_COMMIT = 0x00001000
MEM_PRIVATE = 0x00020000
MEMEXEC_STARTUP_TIMEOUT_SECONDS = 30.0
NATIVE_CAPTURE_TERMINAL_TIMEOUT_SECONDS = 300.0
# A first probe is requested when the authenticated wrapper is released.  A
# manually mapped compiler may then create its worker thread before that probe
# is delivered; permit one follow-up observation after that event, but never
# turn a missing map into an unbounded stream of debug breaks.
MEMEXEC_MAX_PROBES = 2
# sjiswrap v1.1.1 and the pinned GC/2.7 compiler both prefer 0x00400000.
# The wrapper therefore manual-maps a relocated compiler image into its own
# PID.  Direct launch was retained as an authenticated transport for archived
# envelope validation, but sealed compiler diagnostics proved that it is not
# macro-environment-equivalent to sjiswrap.  New captures for this closed pair
# must preserve wrapper semantics even when all request/source bytes are ASCII.
SJISWRAP_V111_SHA256 = "27a3c5d4f263e4eb96e5619cfcda22f45d33ccd121104c7ff6a37e15b3f427cd"
GC27_COMPILER_SHA256 = "04ece8178961bdbaeebe2d4e5922ed542c4d82b2fc3de996c41c9e193bd49eea"
# GC/2.6 is the compiler image used by the stack-home and frontend chronology
# producers.  The frontend hooks below are enabled only for this exact image;
# an arbitrary compiler hash continues to use the legacy stack/PCode profile.
GC26_COMPILER_SHA256 = _stack_home.EXPECTED_COMPILER_SHA256
AUTHENTICATED_DIRECT_COMPILER_PAIRS = frozenset(
    {(SJISWRAP_V111_SHA256, GC27_COMPILER_SHA256)}
)
WRAPPER_SEMANTICS_REQUIRED_PAIRS = frozenset(
    {(SJISWRAP_V111_SHA256, GC27_COMPILER_SHA256)}
)
LOCALS_LIST_HEAD = 0x005EA8D4
ARGUMENTS_LIST_HEAD = 0x005EAA28
# Import the authenticated stack-home producer's compiler globals and
# Object/VarInfo layouts, but keep collection in this process so the combined
# capture has one paused compiler, event bus, and ownership ledger.
FUNCTION_OBJECT = _stack_home.FUNCTION_OBJECT
OBJECT_DATATYPE = _stack_home.OBJECT_DATATYPE
OBJECT_NAME = _stack_home.OBJECT_NAME
OBJECT_VARINFO_DATATYPE1 = _stack_home.OBJECT_VARINFO_DATATYPE1
OBJECT_VARINFO_OTHER = _stack_home.OBJECT_VARINFO_OTHER
VARINFO_RAW_SIZE = _stack_home.VARINFO_RAW_SIZE
# Verified GC/2.6 post-allocation VarInfo layout used by ``regalloc_post``.
VARINFO_NOREGISTER_FIELD = 0x22
VARINFO_FLAGS_FIELD = 0x24
VARINFO_CLASS_FIELD = 0x25
VARINFO_REG_FIELD = 0x26
VARINFO_REG_HI_FIELD = 0x28
VARINFO_PHYSICAL_READ_SIZE = VARINFO_REG_HI_FIELD + 0x2
PHYSICAL_REGISTER_CLASSES = {4: "GPR", 3: "FPR"}
# Compatibility spellings used by the lower-level allocator probes.
LOCALS_LIST = LOCALS_LIST_HEAD
ARGUMENTS_LIST = ARGUMENTS_LIST_HEAD
OBJECT_STACK_FIELD = _stack_home.OBJECT_STACK_FIELD
VARINFO_HOME_FIELD = 0x26

# The historical allowlist remains for compatibility with isolated fixtures.
# New functions are admitted only when the externally bound source is under an
# actual ``src/board`` tree and the request binds an explicit function hash.
CAPSULE_FUNCTIONS = frozenset(
    {
        "mbCapListDebug",
        "CapCheckComPath",
        "mbCapMasuNextGet",
        "CapShopNextGet",
        "CapEffThrowMasu",
        "CapSelectMasuCom",
        "CapSelectMasuPlayer",
    }
)
PLAYER_FUNCTIONS = frozenset(
    {
        "mbPlayerMoveMain",
        "MoveNumOMExec",
        "mbev_PlayerColBall",
        "GetBiriQEffectRadius",
        "MetalEffectCreate",
    }
)
ALLOWED_FUNCTIONS = CAPSULE_FUNCTIONS | PLAYER_FUNCTIONS
SAFE_FUNCTION = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
SESSION_RE = re.compile(r"session-[0-9a-f]{16}\Z")
# Pointer ordinals are capture-local, but a bare ordinal is not a provenance
# identity.  Include the authenticated session in every serialized token so a
# token copied from another capture cannot silently claim ownership here.
TOKEN_RE = re.compile(r"(?P<kind>local|argument)-(?P<session>session-[0-9a-f]{16})-(?P<ordinal>[0-9]{6})\Z")
PCODE_TOKEN_RE = re.compile(r"pcode-(?P<session>session-[0-9a-f]{16})-(?P<ordinal>[0-9]{6})\Z")
IG_TOKEN_RE = re.compile(r"ig-(?P<session>session-[0-9a-f]{16})-(?P<ordinal>[0-9]{6})\Z")
HIDDEN_IG_TOKEN_RE = re.compile(r"hidden-ig-(?P<session>session-[0-9a-f]{16})-(?P<ordinal>[0-9]{6})\Z")
VREG_RE = re.compile(r"[rf][0-9]+\Z")
CANONICAL_DECIMAL = re.compile(r"(?:0|-?[1-9][0-9]*)\Z")
CANONICAL_HEX = re.compile(r"(?:0[xX]0|-?0[xX][1-9a-fA-F][0-9a-fA-F]*)\Z")
WINDOWS_ABSOLUTE = re.compile(r"[A-Za-z]:[\\/][^\\/].*\Z")


def _authorized_board_function(function: Any, source_path: Any) -> bool:
    if not isinstance(function, str) or SAFE_FUNCTION.fullmatch(function) is None:
        return False
    if function in ALLOWED_FUNCTIONS:
        return True
    if isinstance(source_path, Mapping):
        source_path = source_path.get("path")
    try:
        normalized = str(Path(str(source_path)).resolve()).replace("\\", "/").casefold()
    except (OSError, ValueError, TypeError):
        return False
    return "/src/board/" in normalized or "/game/src/board/" in normalized


class _MEMORY_BASIC_INFORMATION(ctypes.Structure):
    """Win64 layout used by VirtualQueryEx for a WOW64 target."""

    _fields_ = [
        ("BaseAddress", ctypes.c_void_p),
        ("AllocationBase", ctypes.c_void_p),
        ("AllocationProtect", wintypes.DWORD),
        ("PartitionId", wintypes.WORD),
        ("_padding", wintypes.WORD),
        ("RegionSize", ctypes.c_size_t),
        ("State", wintypes.DWORD),
        ("Protect", wintypes.DWORD),
        ("Type", wintypes.DWORD),
    ]


def _native_value(value: Any) -> int:
    """Read an integer or ctypes scalar without relying on ``int(c_void_p)``."""

    raw = getattr(value, "value", value)
    return int(raw or 0)

# The first six rows are shared verbatim with the authenticated native
# allocator producer.  The two final rows are the retained GC/2.6 allocator
# sites: ``regalloc`` is the direct Object-to-vreg probe and ``regalloc_post``
# is the verified post-assignment Object/VarInfo probe.  The latter observes
# the compiler's EBX Object* and EBP VarInfo* contract after the allocator has
# written the physical-register fields.
# The former 0x433CF5/0x433EA6/0x433EAB stage rows were from a different
# compiler layout and do not match the authenticated image.  They are omitted
# rather than relabeled as PCode evidence.  Prefixes are bytes, never a caller
# supplied address claim; preflight checks all of them before the first
# breakpoint write.
HOOKS: tuple[dict[str, Any], ...] = tuple(
    {**row, "lane": "stack"} for row in _stack_home.HOOKS
) + (
    {
        "id": "regalloc",
        "address": 0x0043598B,
        "prefix": "ff74240ce89ca809",
        "lane": "pcode",
        "role": "regalloc",
    },
    {
        "id": "regalloc_post",
        "address": 0x004D03E8,
        "prefix": "83c4085d5e5bc300",
        "lane": "pcode",
        "role": "regalloc_post",
    },
)
# Preserve the central legacy authority even when an explicitly imported
# private backend replaces the public ``HOOKS`` compatibility variable.
LEGACY_HOOKS = HOOKS
# ``target_boundary`` is the same instruction as the stack lane's
# ``function_filter``.  It must not be installed twice: the combined session
# observes the single trap as both the target-function filter and the frontend
# target entry.  The other five frontend sites are independent traps and are
# carried in the GC/2.6 profile with an explicit frontend lane.
GC26_FRONTEND_HOOKS: tuple[dict[str, Any], ...] = tuple(
    {**row, "lane": "frontend"}
    for row in _frontend_chronology.HOOKS
    if row["id"] != "target_boundary"
)
GC26_FRONTEND_HOOK_IDS = tuple(str(row["id"]) for row in GC26_FRONTEND_HOOKS)
GC26_HOOKS: tuple[dict[str, Any], ...] = HOOKS + GC26_FRONTEND_HOOKS
# GC/2.7 keeps the same stack-hook meanings, but its allocator helper is
# 0xe0 bytes earlier than the authenticated GC/2.6 image.  Three Object-write
# sites therefore move with that helper, while the call at allocation_pre
# stays put and receives a different relative displacement.  Keep these
# compiler-specific bytes out of ``capsule_stack_home_native`` so its GC/2.6
# authority remains immutable.
GC27_STACK_HOOK_OVERRIDES: Mapping[str, Mapping[str, Any]] = {
    "allocation_pre": {
        "address": 0x0043367E,
        "prefix": "e87d650c00598a44240450",
    },
    "object_write_0": {
        "address": 0x004F9D74,
        "prefix": "89432e8b530e8b420201e84821f00105e40c",
    },
    "object_write_1": {
        "address": 0x004F9E11,
        "prefix": "89432e8b4b0e8b410201e84821f00105dc0c",
    },
    "object_write_2": {
        "address": 0x004F9E98,
        "prefix": "89432e8b4b0e8b410201e84821f00105d80c",
    },
}
GC27_PHYSICAL_HOOKS: tuple[dict[str, Any], ...] = (
    {
        "id": "physical_pair_commit",
        "address": 0x004D0E65,
        "prefix": "5d5f5e5bc3",
        "lane": "pcode",
        "role": "regalloc_post",
    },
    {
        "id": "physical_single_commit",
        "address": 0x004D0F6E,
        "prefix": "5d5f5e5bc3",
        "lane": "pcode",
        "role": "regalloc_post",
    },
    {
        "id": "precolored_commit",
        "address": 0x004D0A7B,
        "prefix": "eb768d4000",
        "lane": "pcode",
        "role": "regalloc_post",
    },
)
GC27_PCODE_COLOR_HOOKS: tuple[dict[str, Any], ...] = (
    {
        "id": "pcode_color_pre",
        "address": 0x005086C4,
        "prefix": "6689420483c20c83",
        "lane": "pcode",
        "role": "pcode_color_diagnostic",
    },
    {
        "id": "pcode_color_post",
        "address": 0x005086C8,
        "prefix": "83c20c83ed0173d3",
        "lane": "pcode",
        "role": "pcode_color_diagnostic",
    },
)
GC27_MACHINE_EMIT_HOOK: dict[str, Any] = {
    "id": "gc27_machine_emit",
    "address": 0x004EB21F,
    "prefix": "8b178b0a030dd00b5e0001e989018b43",
    "lane": "pcode",
    "role": "machine_emit",
}
# GC/2.7 shares the stack-hook contract and direct-allocation meaning, not all
# physical addresses.  Apply the four image-authenticated overrides above,
# then replace the stale GC/2.6 0x4D03E8 post-assignment hook with a closed
# three-site physical-commit profile.  A private backend may implement capture
# behavior, never change the request's authenticated hook authority.
GC27_BASE_HOOKS: tuple[dict[str, Any], ...] = tuple(
    {
        **row,
        **GC27_STACK_HOOK_OVERRIDES.get(str(row["id"]), {}),
    }
    for row in HOOKS
    if row["id"] != "regalloc_post"
)
GC27_HOOKS: tuple[dict[str, Any], ...] = (
    GC27_BASE_HOOKS
    + GC27_PHYSICAL_HOOKS
    + GC27_PCODE_COLOR_HOOKS
    + (GC27_MACHINE_EMIT_HOOK,)
)
GC27_OPCODE_DESCRIPTOR_TABLE = 0x005C0FA8
GC27_OPCODE_DESCRIPTOR_STRIDE = 18
GC27_OPCODE_DESCRIPTOR_BASE_OFFSET = 0x0E
_HOOK_SETS: tuple[tuple[dict[str, Any], ...], ...] = (HOOKS, GC26_HOOKS, GC27_HOOKS)
HOOK_BY_ID = {str(row["id"]): row for rows in _HOOK_SETS for row in rows}
HOOK_BY_ADDRESS = {int(row["address"]): row for rows in _HOOK_SETS for row in rows}
WRITE_HOOK_IDS = tuple(row["id"] for row in HOOKS if row["role"] == "object_stack_write")
PCODE_HOOK_IDS = tuple(
    row["id"]
    for row in HOOKS
    if row["lane"] == "pcode" and row["role"] not in {"regalloc", "regalloc_post", "machine_emit"}
)
MACHINE_HOOK_IDS = (GC27_MACHINE_EMIT_HOOK["id"],)


def _pcode_stage_hook_ids(hooks: Sequence[Mapping[str, Any]]) -> tuple[str, ...]:
    """Return required generic PCode-stage hooks for one closed profile."""

    excluded = {"regalloc", "regalloc_post", "pcode_color_diagnostic", "machine_emit"}
    return tuple(
        str(row["id"])
        for row in hooks
        if row["lane"] == "pcode" and row["role"] not in excluded
    )


def _hooks_for_compiler(compiler_sha256: str) -> tuple[dict[str, Any], ...]:
    """Select only hook sites authenticated for the request compiler."""

    normalized = compiler_sha256.lower()
    if normalized == GC27_COMPILER_SHA256:
        return GC27_HOOKS
    if normalized == GC26_COMPILER_SHA256:
        return GC26_HOOKS
    return LEGACY_HOOKS


def _validate_runtime_hook_patch(compiler_sha256: str) -> None:
    """Reject private hook-table patches that are not one exact profile."""

    # The module's original tuple is an explicit unpatched sentinel.  A
    # backend-provided copy of the legacy rows is not that sentinel and must
    # therefore satisfy the selected compiler profile exactly.
    if HOOKS is LEGACY_HOOKS:
        return
    runtime = tuple(HOOKS)
    expected = _hooks_for_compiler(compiler_sha256)
    if runtime != expected:
        raise Rejected("private backend hook patch does not match the compiler profile")


def _pe32_file_image_bytes(path: Path, absolute_address: int, size: int) -> bytes:
    """Read preferred-base image bytes from one authenticated PE32 file."""

    if size <= 0:
        raise Rejected("hook prefix is empty")
    data = path.read_bytes()
    try:
        pe_offset = int.from_bytes(data[0x3C:0x40], "little")
        if data[:2] != b"MZ" or pe_offset < 0x40 or pe_offset + 24 > len(data):
            raise ValueError
        if data[pe_offset:pe_offset + 4] != b"PE\0\0":
            raise ValueError
        file_header = pe_offset + 4
        section_count = int.from_bytes(data[file_header + 2:file_header + 4], "little")
        optional_size = int.from_bytes(data[file_header + 16:file_header + 18], "little")
        optional = file_header + 20
        if section_count <= 0 or optional_size < 64 or optional + optional_size > len(data):
            raise ValueError
        if int.from_bytes(data[optional:optional + 2], "little") != 0x10B:
            raise ValueError
        image_base = int.from_bytes(data[optional + 28:optional + 32], "little")
        size_of_headers = int.from_bytes(data[optional + 60:optional + 64], "little")
        rva = int(absolute_address) - image_base
        if rva < 0:
            raise ValueError
        if rva + size <= size_of_headers:
            if rva + size > len(data):
                raise ValueError
            return data[rva:rva + size]
        section_table = optional + optional_size
        for index in range(section_count):
            start = section_table + index * 40
            if start + 40 > len(data):
                raise ValueError
            virtual_size = int.from_bytes(data[start + 8:start + 12], "little")
            virtual_address = int.from_bytes(data[start + 12:start + 16], "little")
            raw_size = int.from_bytes(data[start + 16:start + 20], "little")
            raw_pointer = int.from_bytes(data[start + 20:start + 24], "little")
            span = max(virtual_size, raw_size)
            if not (virtual_address <= rva and rva + size <= virtual_address + span):
                continue
            offset = rva - virtual_address
            copied = min(size, max(0, raw_size - offset))
            if raw_pointer + offset + copied > len(data):
                raise ValueError
            return data[raw_pointer + offset:raw_pointer + offset + copied] + b"\0" * (size - copied)
    except (IndexError, OSError, TypeError, ValueError):
        raise Rejected("authenticated compiler image is not a valid PE32 image") from None
    raise Rejected(f"hook address 0x{int(absolute_address):08x} is outside authenticated compiler sections")


def _validate_authenticated_compiler_hook_image(
    compiler: Mapping[str, Any],
    hooks: Sequence[Mapping[str, Any]],
) -> None:
    """Fail before launch when the pinned GC/2.7 profile is stale on disk."""

    if str(compiler["sha256"]).lower() != GC27_COMPILER_SHA256:
        return
    path = Path(str(compiler["path"]))
    mismatches: list[str] = []
    for row in hooks:
        expected = bytes.fromhex(str(row["prefix"]))
        actual = _pe32_file_image_bytes(path, int(row["address"]), len(expected))
        if actual != expected:
            mismatches.append(f"0x{int(row['address']):08x}")
    if mismatches:
        raise Rejected("authenticated compiler hook prefix mismatch on disk: " + ", ".join(mismatches))

# The three compiler write hooks all begin with the authenticated
# ``mov [ebx+0x2e], eax`` sequence.  Register names are kept in a closed
# per-hook table so a future hook cannot accidentally read an arbitrary
# register and turn it into ownership evidence.
STACK_WRITE_REGISTER_LAYOUTS: Mapping[str, Mapping[str, str]] = {
    "object_write_0": {"object": "ebx", "value": "eax"},
    "object_write_1": {"object": "ebx", "value": "eax"},
    "object_write_2": {"object": "ebx", "value": "eax"},
}

_POINTER_KEY_PARTS = {
    "address",
    "addresses",
    "pointer",
    "pointers",
    "ptr",
    "raw_pointer",
    "raw_address",
    "object_pointer",
    "varinfo_pointer",
    "ig_node",
    "ig_pointer",
    "thread_id",
    "handle",
    "handles",
}
_TOOL_IDENTITY_KEYS = ("wrapper", "debugger", "transport")
_REQUEST_KEYS = {
    "schema",
    "tool_version",
    "diagnostic_only",
    "board_admission",
    "exactness_claim",
    "session_id",
    "function",
    "function_sha256",
    "argv",
    "cwd",
    "source",
    "compiler",
    "authority",
    "wrapper",
    "debugger",
    "transport",
    "hooks",
    "paths",
    "output_dir",
    "request_sha256",
}
_REQUEST_PATH_KEYS = {"event_stream_stack", "event_stream_pcode", "envelope"}
_EVENT_BASE_KEYS = {"schema", "event_id", "sequence", "lane", "event_kind", "session_id", "process_id", "function"}
_EVENT_EXTRA_KEYS = {
    "hook_id",
    "locals",
    "arguments",
    "object_token",
    "target_slot",
    "write_observed",
    "status",
    "reason",
    "vreg_id",
    "bank",
    "stage",
    "opcode",
    "instruction",
    "pcode_token",
    "source_offset",
    "block",
    "order",
    "operands",
    "physical_reg",
    "emitted_offset",
    "instruction_index",
    "opcode_enum",
    "ppc_word",
    "ppc_bytes",
    "mnemonic",
    "registers",
    "immediate",
    "memory_op",
    "memory_width",
    "effective_stack_offset",
    "address_definition",
    "arithmetic_op",
    "arithmetic_type",
    "reaching_definitions",
    "known_reaching_definitions",
    "missing_reaching_registers",
    "owner_joins",
    "physical_owner_joins",
    "operand_role_order",
    "ig_token",
    "hidden_owner_token",
    "operand_ordinal",
    "operand_count",
    "operand_kind",
    "operand_class",
    "operand_bank",
    "operand_index",
    "final_color",
    "ig_flags",
    "confirmed",
    "exit_code",
}
_LANES = ("stack", "pcode")
_EVENT_KINDS = {
    "function_entry",
    "compiler_list",
    "numeric_stack_alloc_pre",
    "numeric_stack_alloc_post",
    "object_stack_write_pre",
    "object_stack_write_post",
    "pcode_capture",
    "regalloc_assignment",
    "physical_reg_assignment",
    "machine_emission",
    "lane_unknown",
    "function_exit",
}
_EVENT_ALLOWED_FIELDS = {
    "function_entry": {"hook_id"},
    "compiler_list": {"locals", "arguments"},
    "numeric_stack_alloc_pre": {"hook_id", "locals", "arguments"},
    "numeric_stack_alloc_post": {"hook_id", "locals", "arguments"},
    "object_stack_write_pre": {"hook_id", "object_token", "target_slot"},
    "object_stack_write_post": {"hook_id", "object_token", "target_slot", "write_observed"},
    "pcode_capture": {
        "hook_id", "status", "reason", "stage", "opcode", "instruction",
        "pcode_token", "source_offset", "block", "order", "operands",
        "ig_token", "object_token", "hidden_owner_token", "operand_ordinal",
        "operand_count", "operand_kind", "operand_class", "operand_bank",
        "operand_index", "final_color", "ig_flags", "confirmed",
    },
    "regalloc_assignment": {"object_token", "status", "reason", "vreg_id", "bank"},
    "physical_reg_assignment": {"object_token", "status", "reason", "physical_reg", "bank"},
    "machine_emission": {
        "hook_id", "status", "reason", "pcode_token", "emitted_offset",
        "instruction_index", "opcode_enum", "ppc_word", "ppc_bytes", "mnemonic",
        "registers", "immediate", "memory_op", "memory_width",
        "effective_stack_offset", "address_definition", "reaching_definitions",
        "known_reaching_definitions", "missing_reaching_registers",
        "arithmetic_op", "arithmetic_type", "owner_joins", "physical_owner_joins",
        "operand_role_order",
    },
    "lane_unknown": {"reason"},
    "function_exit": {"exit_code"},
}
_EVENT_REQUIRED_FIELDS = {
    "function_entry": {"hook_id"},
    "compiler_list": {"locals", "arguments"},
    "numeric_stack_alloc_pre": {"hook_id", "locals", "arguments"},
    "numeric_stack_alloc_post": {"hook_id", "locals", "arguments"},
    "object_stack_write_pre": {"hook_id", "object_token", "target_slot"},
    "object_stack_write_post": {"hook_id", "object_token", "target_slot", "write_observed"},
    "pcode_capture": {"hook_id", "status"},
    "regalloc_assignment": {"status"},
    "physical_reg_assignment": {"status"},
    "machine_emission": {"hook_id", "status"},
    "lane_unknown": {"reason"},
    "function_exit": {"exit_code"},
}
_UNKNOWN_REASONS = (
    "incomplete inventory",
    "null object identity",
    "duplicate object identity",
    "reused IG-node identity",
    "one-to-many object-to-vreg claim",
    "one-to-many vreg-to-object claim",
    "missing or invalid vreg identity",
    "missing GPR/FPR bank evidence",
    "request changed during capture",
)
_KNOWN_UNKNOWN_REASONS = frozenset(
    _UNKNOWN_REASONS
    + (
        "incomplete stack evidence",
        "incomplete PCode evidence",
        "incomplete regalloc",
        "incomplete stack write",
        "GPR/FPR bank conflicts with vreg",
        "invalid vreg identity",
        "missing IG-node identity",
        "function filter did not select requested function",
        "missing or invalid Object VarInfo pointer",
        "duplicate Object VarInfo identity",
        "physical register marked no-register",
        "physical register flags missing assignment bit",
        "unsupported physical register class",
        "physical register index out of range",
        "one-to-many object-to-physical-register claim",
        "one-to-many physical-register-to-object claim",
        "duplicate physical-register assignment",
        "incomplete physical register evidence",
        "incomplete machine emission evidence",
        "missing PCode token",
        "ambiguous PCode token",
        "ambiguous reaching definition",
        "indexed base is nonzero",
        "quantized PSQ is unsupported",
        "unsupported machine opcode",
        "unsupported machine operand",
        "descriptor opcode mismatch",
        "machine owner register-bank mismatch",
        "paired physical register assignment unsupported",
        "incomplete frontend chronology",
     )
)


class Rejected(ValueError):
    """Raised when an input or capture cannot be authenticated safely."""


class ExternalTrustRoot:
    """Immutable out-of-band anchors for a same-session capture.

    The request and envelope contain self-digests, but a producer cannot be
    its own authority.  Callers therefore supply the independently retained
    file identities and compile/debug binding through this object.  Nested
    descriptor mappings are accepted by :meth:`from_mapping` for JSON-facing
    callers; all validation remains in the request/envelope boundary.
    """

    FIELDS = (
        "request_path", "request_sha256", "request_size",
        "source_path", "source_sha256", "source_size",
        "compiler_path", "compiler_sha256", "compiler_size",
        "wrapper_path", "wrapper_sha256", "wrapper_size",
        "debugger_path", "debugger_sha256", "debugger_size",
        "transport_path", "transport_sha256", "transport_size",
        "authority_path", "authority_sha256", "authority_size",
        "event_stream_stack_path", "event_stream_stack_sha256", "event_stream_stack_size",
        "event_stream_pcode_path", "event_stream_pcode_sha256", "event_stream_pcode_size",
        "envelope_path", "envelope_sha256", "envelope_size",
        "function", "function_sha256", "cwd", "argv",
    )

    def __init__(self, **values: Any) -> None:
        unknown = set(values) - set(self.FIELDS)
        if unknown:
            raise Rejected(f"external trust root contains unsupported metadata: {sorted(unknown)}")
        object.__setattr__(self, "_initialized", False)
        for field in self.FIELDS:
            value = values.get(field)
            if field == "argv" and isinstance(value, list):
                value = tuple(value)
            object.__setattr__(self, field, value)
        object.__setattr__(self, "_initialized", True)

    def __setattr__(self, name: str, value: Any) -> None:
        if getattr(self, "_initialized", False):
            raise AttributeError("ExternalTrustRoot is immutable")
        object.__setattr__(self, name, value)

    def __repr__(self) -> str:
        values = ", ".join(
            f"{field}={getattr(self, field)!r}"
            for field in self.FIELDS
            if getattr(self, field) is not None
        )
        return f"ExternalTrustRoot({values})"

    def __eq__(self, other: object) -> bool:
        return isinstance(other, ExternalTrustRoot) and all(
            getattr(self, field) == getattr(other, field) for field in self.FIELDS
        )

    @classmethod
    def from_mapping(cls, value: Mapping[str, Any]) -> "ExternalTrustRoot":
        if not isinstance(value, Mapping):
            raise Rejected("external trust root must be an object")
        aliases = {
            "debug_transport": "transport",
            "event_stack": "event_stream_stack",
            "event_pcode": "event_stream_pcode",
        }
        nested_names = (
            "request", "source", "compiler", "wrapper", "debugger", "transport",
            "debug_transport", "authority", "event_stream_stack", "event_stream_pcode",
            "event_stack", "event_pcode", "envelope",
        )
        flat = set(cls.FIELDS)
        allowed = flat | set(nested_names)
        unknown = set(value) - allowed
        if unknown:
            raise Rejected(f"external trust root contains unsupported metadata: {sorted(unknown)}")
        result: dict[str, Any] = {key: value[key] for key in flat if key in value}
        for name in nested_names:
            nested = value.get(name)
            if nested is None:
                continue
            if not isinstance(nested, Mapping) or set(nested) != {"path", "size", "sha256"}:
                raise Rejected(f"external trust root {name} anchor is malformed")
            canonical = aliases.get(name, name)
            for suffix in ("path", "size", "sha256"):
                target = f"{canonical}_{suffix}"
                candidate = nested[suffix]
                if target in result and result[target] != candidate:
                    raise Rejected(f"conflicting external trust root anchor: {canonical}")
                result[target] = candidate
        return cls(**result)


def _coerce_external_trust_root(value: Any) -> ExternalTrustRoot | None:
    if value is None:
        return None
    if isinstance(value, ExternalTrustRoot):
        return value
    if isinstance(value, Mapping):
        return ExternalTrustRoot.from_mapping(value)
    raise Rejected("external trust root must be an object")


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise Rejected(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def strict_json_loads(text: str, label: str = "JSON") -> Any:
    try:
        return json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    except Rejected:
        raise
    except json.JSONDecodeError as exc:
        raise Rejected(f"invalid {label}: {exc}") from exc


def canonical_hash(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or value != value.strip():
        raise Rejected(f"{label} must be non-empty text")
    return value


def _integer(value: Any, label: str, *, nonnegative: bool = False) -> int:
    if isinstance(value, bool):
        raise Rejected(f"{label} must be an integer")
    if isinstance(value, int):
        result = value
    elif isinstance(value, str) and CANONICAL_HEX.fullmatch(value):
        result = int(value, 0)
    elif isinstance(value, str) and CANONICAL_DECIMAL.fullmatch(value):
        result = int(value, 10)
    else:
        raise Rejected(f"{label} must be an integer")
    if nonnegative and result < 0:
        raise Rejected(f"{label} must be non-negative")
    return result


def _canonical_path(value: Any, label: str, *, directory: bool = False, must_exist: bool = True) -> Path:
    raw = _text(str(value) if isinstance(value, Path) else value, label)
    components = re.split(r"[\\/]", raw)
    if "." in components or ".." in components:
        raise Rejected(f"{label} uses a non-canonical path spelling")
    # Windows paths are canonical in the Windows capture environment even
    # when a fixture is inspected on POSIX.  Preserve that spelling here.
    if WINDOWS_ABSOLUTE.fullmatch(raw):
        candidate = Path(raw)
        if candidate.is_symlink():
            raise Rejected(f"{label} is a symlink")
        if must_exist and os.name == "nt" and not candidate.exists():
            raise Rejected(f"{label} is missing: {raw}")
        if candidate.exists() and directory != candidate.is_dir():
            raise Rejected(f"{label} has the wrong file kind")
        return candidate
    candidate = Path(raw)
    if not candidate.is_absolute():
        raise Rejected(f"{label} must be absolute")
    try:
        resolved = candidate.resolve(strict=False)
    except OSError as exc:
        raise Rejected(f"cannot resolve {label}: {exc}") from exc
    if str(resolved) != raw:
        raise Rejected(f"{label} is not the canonical path")
    if candidate.is_symlink():
        raise Rejected(f"{label} is a symlink")
    if must_exist and not candidate.exists():
        raise Rejected(f"{label} is missing: {candidate}")
    if candidate.exists() and directory != candidate.is_dir():
        raise Rejected(f"{label} has the wrong file kind")
    return candidate


def _descriptor(value: Any, label: str, *, verify: bool = True) -> dict[str, Any]:
    if isinstance(value, (str, Path)):
        path = _canonical_path(value, f"{label}.path")
        descriptor = {"path": str(path), "size": path.stat().st_size, "sha256": sha256(path)}
        return descriptor
    if not isinstance(value, Mapping) or set(value) != {"path", "size", "sha256"}:
        raise Rejected(f"{label} must contain only path, size, sha256")
    path = _canonical_path(value["path"], f"{label}.path")
    size = _integer(value["size"], f"{label}.size", nonnegative=True)
    digest = _text(value["sha256"], f"{label}.sha256").lower()
    if not SHA256_RE.fullmatch(digest):
        raise Rejected(f"{label}.sha256 is malformed")
    actual_size = path.stat().st_size
    actual_digest = sha256(path)
    if verify and (size != actual_size or digest != actual_digest):
        raise Rejected(f"{label} identity mismatch")
    return {"path": str(path), "size": actual_size, "sha256": actual_digest}


def _path_descriptor(
    value: Any,
    label: str,
    *,
    must_exist: bool,
    verify_live: bool = True,
) -> dict[str, Any]:
    """Read a descriptor while allowing direct fake-auth fixtures."""

    if isinstance(value, Mapping) and set(value) == {"path", "size", "sha256"}:
        raw_path = _text(value["path"], f"{label}.path")
        path = _canonical_path(
            raw_path,
            f"{label}.path",
            must_exist=must_exist if verify_live else False,
        )
        size = _integer(value["size"], f"{label}.size", nonnegative=True)
        digest = _text(value["sha256"], f"{label}.sha256").lower()
        if not SHA256_RE.fullmatch(digest):
            raise Rejected(f"{label}.sha256 is malformed")
        if verify_live and path.exists():
            if path.stat().st_size != size or sha256(path) != digest:
                raise Rejected(f"{label} identity mismatch")
        return {"path": str(path), "size": size, "sha256": digest}
    if not verify_live:
        raise Rejected(f"{label} requires a sealed descriptor when live verification is disabled")
    return _descriptor(value, label, verify=True)


def _digest(value: Any, label: str) -> str:
    digest = _text(value, label).lower()
    if digest != str(value):
        raise Rejected(f"{label} must use lowercase hexadecimal")
    if not SHA256_RE.fullmatch(digest):
        raise Rejected(f"{label} must be a SHA-256 digest")
    return digest


def _trust_root_descriptor(
    root: ExternalTrustRoot,
    name: str,
    *,
    required: bool = True,
    verify_live: bool = True,
) -> dict[str, Any] | None:
    values = {
        "path": getattr(root, f"{name}_path", None),
        "sha256": getattr(root, f"{name}_sha256", None),
        "size": getattr(root, f"{name}_size", None),
    }
    if all(value is None for value in values.values()):
        if required:
            raise Rejected(f"external trust root.{name} anchor is missing")
        return None
    if any(value is None for value in values.values()):
        raise Rejected(f"external trust root.{name} anchor is incomplete")
    path = _canonical_path(
        values["path"],
        f"external trust root.{name}.path",
        must_exist=verify_live,
    )
    size = _integer(values["size"], f"external trust root.{name}.size", nonnegative=True)
    digest = _digest(values["sha256"], f"external trust root.{name}.sha256")
    if not verify_live:
        return {"path": str(path), "size": size, "sha256": digest}
    actual = {"path": str(path), "size": path.stat().st_size, "sha256": sha256(path)}
    if size != actual["size"] or digest != actual["sha256"]:
        raise Rejected(f"external trust root.{name} bytes do not match")
    return actual


def _root_request_binding(root: ExternalTrustRoot, request: Mapping[str, Any]) -> None:
    """Bind an external root to the exact request compile/debug context."""

    if root.function is None or root.function_sha256 is None or root.cwd is None or root.argv is None:
        raise Rejected("external trust root compile binding is incomplete")
    if root.function != request["function"]:
        raise Rejected("external trust root function does not match request")
    if _digest(root.function_sha256, "external trust root.function_sha256") != request["function_sha256"]:
        raise Rejected("external trust root function hash does not match request")
    root_cwd = _canonical_cwd(root.cwd, must_exist=True)
    if root_cwd != request["cwd"]:
        raise Rejected("external trust root cwd does not match request")
    root_argv = _canonical_argv(list(root.argv), "external trust root.argv")
    if root_argv != request["argv"]:
        raise Rejected("external trust root argv does not match request")


def _validate_compile_argv(
    argv: Sequence[str],
    *,
    cwd: str,
    source: Mapping[str, Any],
    compiler: Mapping[str, Any],
    wrapper: Mapping[str, Any] | None = None,
    require_include_paths: bool = False,
) -> list[str]:
    """Require a single, source-anchored compiler ``-c`` operand."""

    values = _canonical_argv(list(argv), "argv")
    source_path = _canonical_path(source["path"], "source.path", must_exist=False)
    compiler_path = _canonical_path(compiler["path"], "compiler.path", must_exist=False)
    if not values:
        raise Rejected("argv is empty")
    executable_candidates = [Path(values[0])]
    if wrapper is not None:
        wrapper_path = _canonical_path(wrapper["path"], "wrapper.path", must_exist=False)
        wrapper_name = wrapper_path.name.casefold()
        if executable_candidates[0].name.casefold() == wrapper_name:
            wrapper_operand = executable_candidates[0]
            if wrapper_operand.is_absolute() or len(wrapper_operand.parts) > 1:
                if not wrapper_operand.is_absolute():
                    wrapper_operand = Path(cwd) / wrapper_operand
                if str(_canonical_path(wrapper_operand, "argv wrapper", must_exist=False)) != str(wrapper_path):
                    raise Rejected("argv wrapper identity does not match request")
            if len(values) < 2:
                raise Rejected("argv wrapper is missing the compiler")
            executable_candidates.append(Path(values[1]))
        else:
            # A wrapper identity is still bound even when the caller supplied
            # the compiler directly; this is only accepted for direct fake
            # contexts whose wrapper is not an on-disk executable.
            if wrapper_path.exists():
                raise Rejected("argv does not invoke the authenticated wrapper")
    executable = executable_candidates[-1]
    if executable.is_absolute():
        if str(_canonical_path(executable, "argv compiler", must_exist=False)) != str(compiler_path):
            raise Rejected("argv compiler identity does not match request")
    elif len(executable.parts) > 1:
        resolved_executable = _canonical_path(Path(cwd) / executable, "argv compiler", must_exist=False)
        if str(resolved_executable) != str(compiler_path):
            raise Rejected("argv compiler identity does not match request")
    elif executable.name.casefold() != compiler_path.name.casefold():
        raise Rejected("argv compiler identity does not match request")
    source_indexes = [index for index, item in enumerate(values) if item == "-c"]
    if len(source_indexes) != 1 or source_indexes[0] + 1 >= len(values):
        raise Rejected("argv requires exactly one -c source operand")
    source_operand = values[source_indexes[0] + 1]
    if source_operand.startswith("-"):
        raise Rejected("argv -c operand is missing a source path")
    operand = Path(source_operand)
    if not operand.is_absolute():
        operand = Path(cwd) / operand
    operand = _canonical_path(operand, "argv -c source", must_exist=False)
    if str(operand) != str(source_path):
        raise Rejected("argv -c operand is not the authenticated source")
    if require_include_paths:
        _compile_include_paths(values, cwd)
    return values


def _compile_include_paths(argv: Sequence[str], cwd: str) -> list[Path]:
    """Resolve every explicit MWCC include root and reject environment drift."""

    values = list(argv)
    roots: list[Path] = []
    for index, value in enumerate(values):
        if value != "-i":
            continue
        if index + 1 >= len(values) or values[index + 1].startswith("-"):
            raise Rejected("compiler -i operand is missing an include path")
        path = Path(values[index + 1])
        if not path.is_absolute():
            path = Path(cwd) / path
        roots.append(_canonical_path(path, "compiler -i include path", directory=True, must_exist=True))
    return roots


def _directory_tree_descriptor(path: Path) -> dict[str, Any]:
    path = _canonical_path(path, "compiler include tree", directory=True, must_exist=True)
    rows: list[dict[str, Any]] = []
    for entry in sorted(path.rglob("*"), key=lambda value: value.relative_to(path).as_posix()):
        if entry.is_symlink():
            raise Rejected("compiler include tree contains a symlink")
        if entry.is_dir():
            continue
        if not entry.is_file():
            raise Rejected("compiler include tree contains a non-regular entry")
        rows.append({
            "relative_path": entry.relative_to(path).as_posix(),
            "size": entry.stat().st_size,
            "sha256": sha256(entry),
        })
    return {
        "path": str(path),
        "file_count": len(rows),
        "tree_sha256": canonical_hash(rows),
    }


def _native_launch_argv(request: Mapping[str, Any]) -> list[str]:
    """Return the authenticated executable argv used by CreateProcessW.

    Request argv is already canonicalized and bound to the source/compiler,
    but the native launch path must preserve the wrapper executable when one
    is present.  The old path prepended ``mwcceppc.exe`` whenever argv[0] was
    not the compiler, which transformed the authenticated
    ``sjiswrap.exe mwcceppc.exe ...`` shape into an unrelated command.  This
    helper revalidates the complete shape and returns it unchanged.
    """

    if not isinstance(request, Mapping):
        raise Rejected("native launch request is not an object")
    source = request.get("source")
    compiler = request.get("compiler")
    wrapper = request.get("wrapper")
    if not isinstance(source, Mapping) or not isinstance(compiler, Mapping):
        raise Rejected("native launch source/compiler identities are missing")
    if not isinstance(wrapper, Mapping):
        raise Rejected("native launch requires an authenticated wrapper identity")
    values = _validate_compile_argv(
        request.get("argv"),
        cwd=_canonical_cwd(request.get("cwd"), must_exist=False),
        source=source,
        compiler=compiler,
        wrapper=wrapper,
        require_include_paths=True,
    )
    if "path" not in wrapper:
        raise Rejected("native launch wrapper identity has no path")
    wrapper_path = _canonical_path(wrapper["path"], "request.wrapper.path", must_exist=False)
    if Path(values[0]).name.casefold() != wrapper_path.name.casefold():
        raise Rejected("native launch argv does not invoke the authenticated wrapper")
    return values


def _authenticated_direct_compiler_argv(
    request: Mapping[str, Any],
    *,
    observed_wrapper_map: bool = False,
) -> list[str]:
    """Derive one closed direct-compiler transport from wrapper authority.

    This is a transport substitution, not an argv rewrite or compiler
    identity fallback. It is available only for the pinned sjiswrap v1.1.1
    and GC/2.7 pair and only before any wrapper map has been observed. Every
    request/cwd/source byte that the wrapper could transcode must be ASCII, so
    removing argv[0] is byte-equivalent at the process boundary. Hook/image
    authentication still occurs against the compiler selected by its path,
    complete SHA-256, and pinned prefixes.
    """

    if observed_wrapper_map:
        raise Rejected("direct compiler transport cannot follow an observed wrapper map")
    values = _native_launch_argv(request)
    wrapper = request.get("wrapper")
    compiler = request.get("compiler")
    source = request.get("source")
    if not isinstance(wrapper, Mapping) or not isinstance(compiler, Mapping) or not isinstance(source, Mapping):
        raise Rejected("direct compiler transport lacks authenticated identities")
    wrapper_sha = _digest(wrapper.get("sha256"), "request.wrapper.sha256")
    compiler_sha = _digest(compiler.get("sha256"), "request.compiler.sha256")
    if (wrapper_sha, compiler_sha) not in AUTHENTICATED_DIRECT_COMPILER_PAIRS:
        raise Rejected("wrapper/compiler pair is not authorized for direct transport")
    wrapper_path = _canonical_path(wrapper.get("path"), "request.wrapper.path", must_exist=True)
    compiler_path = _canonical_path(compiler.get("path"), "request.compiler.path", must_exist=True)
    if sha256(wrapper_path) != wrapper_sha or sha256(compiler_path) != compiler_sha:
        raise Rejected("direct transport executable bytes do not match authority")

    cwd = _canonical_cwd(request.get("cwd"), must_exist=False)
    try:
        for value in (str(cwd), *values):
            value.encode("ascii", errors="strict")
    except UnicodeEncodeError:
        raise Rejected("direct compiler transport requires ASCII-equivalent cwd and argv") from None

    source_path = _canonical_path(source.get("path"), "request.source.path", must_exist=True)
    try:
        source_path.read_bytes().decode("ascii", errors="strict")
    except UnicodeDecodeError:
        raise Rejected("direct compiler transport requires ASCII-equivalent source bytes") from None
    if sha256(source_path) != _digest(source.get("sha256"), "request.source.sha256"):
        raise Rejected("source bytes changed before direct compiler transport")

    executed = values[1:]
    if not executed:
        raise Rejected("direct compiler transport lost its compiler argv")
    if str(_canonical_path(executed[0], "direct compiler argv[0]", must_exist=True)) != str(compiler_path):
        raise Rejected("direct compiler transport did not preserve compiler identity")
    return executed


def _native_transport_plan(request: Mapping[str, Any]) -> tuple[str, list[str]]:
    """Choose the authenticated native transport before process creation."""

    request_argv = _native_launch_argv(request)
    wrapper = request.get("wrapper")
    compiler = request.get("compiler")
    if not isinstance(wrapper, Mapping) or not isinstance(compiler, Mapping):
        raise Rejected("native transport plan lacks wrapper/compiler identities")
    pair = (
        _digest(wrapper.get("sha256"), "request.wrapper.sha256"),
        _digest(compiler.get("sha256"), "request.compiler.sha256"),
    )
    if pair in WRAPPER_SEMANTICS_REQUIRED_PAIRS:
        return "wrapper_memexec", request_argv
    if pair in AUTHENTICATED_DIRECT_COMPILER_PAIRS:
        return "authenticated_direct_compiler", _authenticated_direct_compiler_argv(request)
    return "wrapper_memexec", request_argv


def _validate_external_root_against_request(
    root: ExternalTrustRoot,
    request: Mapping[str, Any],
    *,
    request_path: Path | None,
    allow_outputs: bool = False,
    post_capture_analysis: bool = False,
) -> None:
    """Check all independently retained identities against a request."""

    _root_request_binding(root, request)
    for name in ("source", "compiler", *_TOOL_IDENTITY_KEYS, "authority"):
        expected = request.get(name)
        if not isinstance(expected, Mapping):
            raise Rejected(f"request.{name} identity is missing")
        actual = _trust_root_descriptor(
            root,
            name,
            required=True,
            verify_live=not (
                post_capture_analysis and name in {"debugger", "transport"}
            ),
        )
        if actual != dict(expected):
            raise Rejected(f"external trust root.{name} does not match request")
    if request_path is not None:
        actual_request = _trust_root_descriptor(root, "request", required=True)
        if actual_request is None or str(_canonical_path(request_path, "request")) != actual_request["path"]:
            raise Rejected("external trust root.request path does not match request")
        if request_path.stat().st_size != actual_request["size"] or sha256(request_path) != actual_request["sha256"]:
            raise Rejected("external trust root.request bytes do not match request")
    elif any(getattr(root, f"request_{suffix}", None) is not None for suffix in ("path", "sha256", "size")):
        # A caller may provide a root before prepare_request, but a partial
        # request anchor there is ambiguous and must not be accepted.
        values = [getattr(root, f"request_{suffix}", None) for suffix in ("path", "sha256", "size")]
        if not all(value is None for value in values):
            raise Rejected("external trust root.request anchor cannot bind before request creation")
    if post_capture_analysis:
        # Historical analysis may outlive the exact debugger source path, but
        # only after all immutable producer outputs remain present and bound.
        for name in ("event_stream_stack", "event_stream_pcode", "envelope"):
            _trust_root_descriptor(root, name, required=True, verify_live=True)
    if allow_outputs:
        return
    for name in ("event_stream_stack", "event_stream_pcode", "envelope"):
        _trust_root_descriptor(root, name, required=True)


def _validate_hook_rows(
    value: Any,
    label: str = "hooks",
    *,
    compiler_sha256: str | None = None,
) -> list[dict[str, Any]]:
    expected_sets = (
        (_hooks_for_compiler(compiler_sha256),)
        if compiler_sha256 is not None
        else _HOOK_SETS
    )
    if not isinstance(value, list):
        raise Rejected(f"{label} must contain the complete pinned hook union")
    expected = next((rows for rows in expected_sets if len(rows) == len(value)), None)
    if expected is None:
        raise Rejected(f"{label} must contain the complete pinned hook union")
    result: list[dict[str, Any]] = []
    seen_addresses: set[int] = set()
    for index, (raw, expected_row) in enumerate(zip(value, expected)):
        if not isinstance(raw, Mapping) or set(raw) != {"id", "address", "prefix", "lane", "role"}:
            raise Rejected(f"{label}[{index}] has unsupported fields")
        if not isinstance(raw["address"], int) or isinstance(raw["address"], bool):
            raise Rejected(f"{label}[{index}].address is not type-canonical")
        normalized = {
            "id": _text(raw["id"], f"{label}[{index}].id"),
            "address": _integer(raw["address"], f"{label}[{index}].address", nonnegative=True),
            "prefix": _text(raw["prefix"], f"{label}[{index}].prefix").lower(),
            "lane": _text(raw["lane"], f"{label}[{index}].lane"),
            "role": _text(raw["role"], f"{label}[{index}].role"),
        }
        if normalized != expected_row:
            raise Rejected(f"{label}[{index}] does not match the pinned hook union")
        prefix = normalized["prefix"]
        if len(prefix) == 0 or len(prefix) % 2 or not re.fullmatch(r"[0-9a-f]+", prefix):
            raise Rejected(f"{label}[{index}].prefix is malformed")
        if normalized["address"] in seen_addresses:
            raise Rejected(f"{label} contains duplicate addresses")
        seen_addresses.add(normalized["address"])
        result.append(normalized)
    return result


def _safe_session_id(value: Any) -> str:
    session_id = _text(value, "session_id")
    if not SESSION_RE.fullmatch(session_id):
        raise Rejected("session_id is not canonical")
    return session_id


def _new_session_id() -> str:
    return f"session-{uuid.uuid4().hex[:16]}"


def _canonical_argv(value: Any, label: str = "argv") -> list[str]:
    if not isinstance(value, list) or not value or not all(isinstance(item, str) and item == item.strip() and item for item in value):
        raise Rejected(f"{label} must be a non-empty canonical string list")
    return list(value)


def _canonical_cwd(value: Any, *, must_exist: bool = True) -> str:
    return str(_canonical_path(value, "cwd", directory=True, must_exist=must_exist))


def _pointer_free(value: Any, location: str = "value") -> None:
    """Reject raw pointer/address spellings recursively before serialization."""

    if isinstance(value, Mapping):
        for key, child in value.items():
            key_text = str(key).lower()
            # Hook instruction addresses are authenticated code-site metadata,
            # not process/object pointers.  They are the one serialized
            # address spelling allowed in the envelope.
            hook_address = key_text == "address" and ".hooks" in location
            if (key_text in _POINTER_KEY_PARTS and not hook_address) or key_text.endswith(("_pointer", "_ptr", "_address")):
                raise Rejected(f"{location}.{key} exposes a raw pointer/address")
            _pointer_free(child, f"{location}.{key}")
    elif isinstance(value, (list, tuple)):
        for index, child in enumerate(value):
            _pointer_free(child, f"{location}[{index}]")


def _validate_vreg(value: Any, label: str) -> str:
    vreg = _text(value, label)
    if not VREG_RE.fullmatch(vreg):
        raise Rejected(f"{label} is not a canonical virtual register")
    # Physical r0..r31/f0..f31 values are not virtual ownership identities.
    if int(vreg[1:]) < 32:
        raise Rejected(f"{label} is a physical register, not a vreg")
    return vreg


def write_new(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
    except BaseException:
        try:
            path.unlink()
        except OSError:
            pass
        raise


def _canonical_json_bytes(value: Any) -> bytes:
    _pointer_free(value)
    return (json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")


def canonical_lane_bytes(events: Sequence[Mapping[str, Any]]) -> bytes:
    return b"".join(_canonical_json_bytes(dict(event)) for event in events)


def canonical_event_bytes(events: Sequence[Mapping[str, Any]]) -> bytes:
    return canonical_lane_bytes(events)


class PointerLedger:
    """Capture-local pointer ledger; raw identities never cross this boundary."""

    def __init__(self, session_id: str = "session-0000000000000000") -> None:
        self.session_id = _safe_session_id(session_id)
        self._by_kind: dict[str, dict[int, str]] = {"local": {}, "argument": {}}
        self._by_token: dict[str, dict[str, Any]] = {}
        self._next: dict[str, int] = {"local": 0, "argument": 0}
        self._collisions: set[str] = set()

    def register(self, kind: str, pointer: Any) -> str | None:
        if kind not in self._by_kind:
            raise Rejected(f"unsupported ledger kind: {kind}")
        try:
            value = _integer(pointer, f"{kind} pointer", nonnegative=True)
        except Rejected:
            self._collisions.add(f"{kind}:null")
            return None
        if value == 0:
            self._collisions.add(f"{kind}:null")
            return None
        existing_other = next((token for other, rows in self._by_kind.items() if other != kind for raw, token in rows.items() if raw == value), None)
        if existing_other is not None:
            self._collisions.add(existing_other)
            self._collisions.add(f"{kind}:{value}")
            return None
        if value in self._by_kind[kind]:
            return self._by_kind[kind][value]
        ordinal = self._next[kind]
        token = f"{kind}-{self.session_id}-{ordinal:06d}"
        self._next[kind] += 1
        self._by_kind[kind][value] = token
        self._by_token[token] = {"kind": kind, "status": "CAPTURED"}
        return token

    def lookup(self, kind: str, pointer: Any) -> str | None:
        try:
            value = _integer(pointer, f"{kind} pointer", nonnegative=True)
        except Rejected:
            return None
        return self._by_kind.get(kind, {}).get(value)

    def kind_for(self, pointer: Any) -> tuple[str, str] | None:
        try:
            value = _integer(pointer, "object identity", nonnegative=True)
        except Rejected:
            return None
        matches = [(kind, token) for kind, rows in self._by_kind.items() if (token := rows.get(value)) is not None]
        return matches[0] if len(matches) == 1 else None

    def mark_unknown(self, token: str | None, reason: str) -> None:
        if token is None:
            return
        row = self._by_token.setdefault(token, {"kind": token.split("-", 1)[0], "status": "UNKNOWN"})
        row["status"] = "UNKNOWN"
        row["reason"] = reason

    def tokens(self, kind: str) -> list[str]:
        return list(self._by_kind[kind].values())

    def inventory(self, kind: str) -> list[dict[str, Any]]:
        return [{"token": token, "kind": kind} for token in self.tokens(kind)]

    @property
    def collisions(self) -> set[str]:
        return set(self._collisions)


class EventBus:
    """Single monotonic sequence shared by stack and PCode lanes."""

    def __init__(self, *, session_id: str, function: str) -> None:
        self.session_id = _safe_session_id(session_id)
        self.function = _text(function, "function")
        self.process_id: int | None = None
        self.events: list[dict[str, Any]] = []

    def bind_process(self, process_id: Any) -> None:
        value = _integer(process_id, "native process id", nonnegative=True)
        if value == 0:
            raise Rejected("native process id must be non-zero")
        if self.process_id is not None and self.process_id != value:
            raise Rejected("native process id changed during capture")
        self.process_id = value

    def check_process(self, process_id: Any) -> int:
        value = _integer(process_id, "native process id", nonnegative=True)
        if value == 0 or self.process_id is None or value != self.process_id:
            raise Rejected("native process id changed during capture")
        return value

    def emit(self, lane: str, event_kind: str, payload: Mapping[str, Any] | None = None) -> dict[str, Any]:
        if lane not in _LANES:
            raise Rejected(f"unsupported event lane: {lane}")
        if self.process_id is None:
            raise Rejected("event emitted before native process start")
        sequence = len(self.events)
        event: dict[str, Any] = {
            "schema": EVENT_SCHEMA,
            "event_id": f"{self.session_id}-e{sequence:06d}",
            "sequence": sequence,
            "lane": lane,
            "event_kind": _text(event_kind, "event_kind"),
            "session_id": self.session_id,
            "process_id": self.process_id,
            "function": self.function,
        }
        if payload:
            for key, value in payload.items():
                if key in event:
                    raise Rejected(f"event payload overwrites {key}")
                event[key] = value
        _pointer_free(event)
        self.events.append(event)
        return event


def _signed_field(value: int, bits: int) -> int:
    sign = 1 << (bits - 1)
    return (value & (sign - 1)) - (value & sign)


class MachineEmissionDecoder:
    """Deterministically reduce emitted PPC words to proven stack effects."""

    _D_MEMORY = {
        32: ("lwz", "load", 4, "GPR"),
        34: ("lbz", "load", 1, "GPR"),
        36: ("stw", "store", 4, "GPR"),
        38: ("stb", "store", 1, "GPR"),
        40: ("lhz", "load", 2, "GPR"),
        42: ("lha", "load", 2, "GPR"),
        44: ("sth", "store", 2, "GPR"),
        48: ("lfs", "load", 4, "FPR"),
        50: ("lfd", "load", 8, "FPR"),
        52: ("stfs", "store", 4, "FPR"),
        54: ("stfd", "store", 8, "FPR"),
    }

    def __init__(self) -> None:
        self.address_defs: dict[int, tuple[int, int]] = {}
        self.value_defs: dict[tuple[str, int], int] = {}
        self.last_offset = -4

    def invalidate(self) -> None:
        self.address_defs.clear()
        self.value_defs.clear()

    def _unknown(
        self,
        reason: str,
        located: Mapping[str, Any] | None = None,
    ) -> dict[str, Any]:
        if located is None or not isinstance(located.get("ppc_word"), int):
            # Without a located instruction there is no safe register-local
            # invalidation boundary.
            self.invalidate()
        else:
            # A located but unsupported instruction must not erase unrelated
            # stack/address chains.  Conservatively invalidate the encoded
            # destination field in both tracked banks; for update-form memory
            # instructions also invalidate the updated base register.  The
            # UNKNOWN event itself remains dependency-local in the causal map.
            word = int(located["ppc_word"])
            primary = word >> 26
            destination = (word >> 21) & 31
            self.address_defs.pop(destination, None)
            self.value_defs.pop(("GPR", destination), None)
            self.value_defs.pop(("FPR", destination), None)
            if primary in {33, 35, 37, 39, 41, 43, 45, 49, 51, 53, 55}:
                updated_base = (word >> 16) & 31
                self.address_defs.pop(updated_base, None)
                self.value_defs.pop(("GPR", updated_base), None)
        result = {"status": "UNKNOWN", "reason": reason}
        if located is not None:
            for key in (
                "pcode_token",
                "emitted_offset",
                "instruction_index",
                "opcode_enum",
                "ppc_word",
                "ppc_bytes",
                "mnemonic",
                "registers",
                "arithmetic_op",
                "arithmetic_type",
                "known_reaching_definitions",
                "missing_reaching_registers",
                "operand_role_order",
            ):
                if key in located:
                    result[key] = located[key]
        return result

    def _clear_call_volatile_definitions(self) -> None:
        """Invalidate only caller-clobbered register definitions at ``bl``.

        The REL24 field in an unlinked object may be zero, so the decoder does
        not invent a branch target.  Stack intervals and nonvolatile register
        definitions survive the call boundary.
        """

        for register in (0, *range(3, 13)):
            self.address_defs.pop(register, None)
            self.value_defs.pop(("GPR", register), None)
        for register in range(14):
            self.value_defs.pop(("FPR", register), None)

    def _stack_base(self, register: int) -> tuple[int, int] | None:
        if register == 1:
            return (0, -1)
        return self.address_defs.get(register)

    def decode(
        self,
        *,
        pcode_token: str,
        emitted_offset: int,
        opcode_enum: int,
        encoded_value: int,
        descriptor_base: int,
    ) -> dict[str, Any]:
        if PCODE_TOKEN_RE.fullmatch(pcode_token) is None:
            return self._unknown("missing PCode token")
        if emitted_offset < 0 or emitted_offset % 4 or emitted_offset <= self.last_offset:
            return self._unknown("ambiguous PCode token")
        self.last_offset = emitted_offset
        instruction_index = emitted_offset // 4
        if not 0 <= opcode_enum <= 0x1D4 or not 0 <= encoded_value <= 0xFFFFFFFF:
            return self._unknown("unsupported machine operand")
        emitted_bytes = encoded_value.to_bytes(4, "little", signed=False)
        word = int.from_bytes(emitted_bytes, "big", signed=False)
        result: dict[str, Any] = {
            "status": "CAPTURED",
            "pcode_token": pcode_token,
            "emitted_offset": emitted_offset,
            "instruction_index": instruction_index,
            "opcode_enum": opcode_enum,
            "ppc_word": word,
            "ppc_bytes": emitted_bytes.hex(),
            "reaching_definitions": [],
        }
        if (word & 0xFC000000) != (descriptor_base & 0xFC000000):
            return self._unknown("descriptor opcode mismatch", result)
        primary = word >> 26
        reaching: set[int] = set()

        if primary == 14:  # addi
            destination = (word >> 21) & 31
            base = (word >> 16) & 31
            immediate = _signed_field(word, 16)
            result.update(
                mnemonic="addi",
                registers={"destination": f"r{destination}", "base": f"r{base}"},
                immediate=immediate,
            )
            base_def = self._stack_base(base)
            self.address_defs.pop(destination, None)
            if base_def is not None:
                stack_offset = base_def[0] + immediate
                result["address_definition"] = {
                    "register": f"r{destination}",
                    "stack_offset": stack_offset,
                }
                self.address_defs[destination] = (stack_offset, instruction_index)
                if base_def[1] >= 0:
                    reaching.add(base_def[1])
            self.value_defs[("GPR", destination)] = instruction_index
        elif primary in self._D_MEMORY:
            mnemonic, memory_op, width, bank = self._D_MEMORY[primary]
            data_register = (word >> 21) & 31
            base = (word >> 16) & 31
            displacement = _signed_field(word, 16)
            base_def = self._stack_base(base)
            if base_def is None:
                return self._unknown("ambiguous reaching definition", result)
            stack_offset = base_def[0] + displacement
            result.update(
                mnemonic=mnemonic,
                registers={"data": f"{'r' if bank == 'GPR' else 'f'}{data_register}", "base": f"r{base}"},
                immediate=displacement,
                memory_op=memory_op,
                memory_width=width,
                effective_stack_offset=stack_offset,
            )
            if base_def[1] >= 0:
                reaching.add(base_def[1])
            key = (bank, data_register)
            if memory_op == "load":
                self.value_defs[key] = instruction_index
                if bank == "GPR":
                    self.address_defs.pop(data_register, None)
            elif key in self.value_defs:
                reaching.add(self.value_defs[key])
        elif primary in {56, 60}:  # psq_l / psq_st
            data_register = (word >> 21) & 31
            base = (word >> 16) & 31
            quantization = (word >> 12) & 7
            single = (word >> 15) & 1
            if quantization != 0:
                return self._unknown("quantized PSQ is unsupported", result)
            base_def = self._stack_base(base)
            if base_def is None:
                return self._unknown("ambiguous reaching definition", result)
            displacement = _signed_field(word, 12)
            memory_op = "load" if primary == 56 else "store"
            result.update(
                mnemonic="psq_l" if memory_op == "load" else "psq_st",
                registers={"data": f"f{data_register}", "base": f"r{base}"},
                immediate=displacement,
                memory_op=memory_op,
                memory_width=4 if single else 8,
                effective_stack_offset=base_def[0] + displacement,
            )
            if base_def[1] >= 0:
                reaching.add(base_def[1])
            key = ("FPR", data_register)
            if memory_op == "load":
                self.value_defs[key] = instruction_index
            elif key in self.value_defs:
                reaching.add(self.value_defs[key])
        elif primary == 4 and (word & 0x7F) in {0x0C, 0x0E}:  # psq_lx / psq_stx
            data_register = (word >> 21) & 31
            base = (word >> 16) & 31
            index = (word >> 11) & 31
            single = (word >> 10) & 1
            quantization = (word >> 7) & 7
            if quantization != 0:
                return self._unknown("quantized PSQ is unsupported", result)
            if base != 0:
                return self._unknown("indexed base is nonzero", result)
            index_def = self.address_defs.get(index)
            if index_def is None:
                return self._unknown("ambiguous reaching definition", result)
            memory_op = "load" if (word & 0x7F) == 0x0C else "store"
            result.update(
                mnemonic="psq_lx" if memory_op == "load" else "psq_stx",
                registers={"data": f"f{data_register}", "base": "r0", "index": f"r{index}"},
                memory_op=memory_op,
                memory_width=4 if single else 8,
                effective_stack_offset=index_def[0],
            )
            reaching.add(index_def[1])
            key = ("FPR", data_register)
            if memory_op == "load":
                self.value_defs[key] = instruction_index
            elif key in self.value_defs:
                reaching.add(self.value_defs[key])
        elif primary == 18 and (word & 1) == 1:  # bl
            result.update(mnemonic="bl", registers={})
            self._clear_call_volatile_definitions()
        elif primary == 63 and ((word >> 1) & 0x3FF) == 40:  # fneg
            destination = (word >> 21) & 31
            source = (word >> 11) & 31
            source_key = ("FPR", source)
            if source_key not in self.value_defs:
                return self._unknown("ambiguous reaching definition", result)
            reaching.add(self.value_defs[source_key])
            result.update(
                mnemonic="fneg",
                registers={"destination": f"f{destination}", "source": f"f{source}"},
                operand_role_order=["destination", "source"],
            )
            self.value_defs[("FPR", destination)] = instruction_index
        elif primary == 59 and ((word >> 1) & 0x1F) == 25:  # fmuls
            destination = (word >> 21) & 31
            source_a = (word >> 16) & 31
            source_b = (word >> 6) & 31
            source_keys = (("FPR", source_a), ("FPR", source_b))
            known_reaching = [
                {
                    "physical_register": f"f{register}",
                    "instruction_index": self.value_defs[("FPR", register)],
                }
                for register in sorted({source_a, source_b})
                if ("FPR", register) in self.value_defs
            ]
            missing_reaching = [
                f"f{register}"
                for register in sorted({source_a, source_b})
                if ("FPR", register) not in self.value_defs
            ]
            result.update(
                mnemonic="fmuls",
                registers={
                    "destination": f"f{destination}",
                    "source_a": f"f{source_a}",
                    "source_b": f"f{source_b}",
                },
                arithmetic_op="multiply",
                arithmetic_type="f32",
                operand_role_order=["destination", "source_a", "source_b"],
                known_reaching_definitions=known_reaching,
                missing_reaching_registers=missing_reaching,
            )
            if missing_reaching:
                return self._unknown("ambiguous reaching definition", result)
            reaching.update(self.value_defs[key] for key in source_keys)
            result.pop("known_reaching_definitions")
            result.pop("missing_reaching_registers")
            self.value_defs[("FPR", destination)] = instruction_index
        elif primary == 4 and ((word >> 1) & 0x1F) == 25:  # ps_mul
            destination = (word >> 21) & 31
            source_a = (word >> 16) & 31
            source_b = (word >> 6) & 31
            source_keys = (("FPR", source_a), ("FPR", source_b))
            if any(key not in self.value_defs for key in source_keys):
                return self._unknown("ambiguous reaching definition", result)
            reaching.update(self.value_defs[key] for key in source_keys)
            result.update(
                mnemonic="ps_mul",
                registers={
                    "destination": f"f{destination}",
                    "source_a": f"f{source_a}",
                    "source_b": f"f{source_b}",
                },
                arithmetic_op="multiply",
                arithmetic_type="paired-single",
                operand_role_order=["destination", "source_a", "source_b"],
            )
            self.value_defs[("FPR", destination)] = instruction_index
        else:
            return self._unknown("unsupported machine opcode", result)

        result["reaching_definitions"] = sorted(reaching)
        return result


class CaptureBackend(Protocol):
    """Protocol for both the native adapter and deterministic fake backends."""

    capabilities: set[str] | frozenset[str]

    def read_image(self, address: int, size: int) -> bytes: ...

    def install_breakpoint(self, address: int) -> None: ...

    def remove_breakpoint(self, address: int) -> None: ...

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None: ...

    def run(self, session: "CombinedCaptureSession") -> None: ...

    def close(self) -> None: ...


class SharedBreakpointDispatcher:
    """One breakpoint table and one step/rearm path for both lanes."""

    def __init__(self, backend: CaptureBackend, session: "CombinedCaptureSession", hooks: Sequence[Mapping[str, Any]] = HOOKS) -> None:
        self.backend = backend
        self.session = session
        session_auth = getattr(session, "auth", None)
        compiler_sha256: str | None = None
        if isinstance(session_auth, Mapping):
            request = session_auth.get("request")
            if isinstance(request, Mapping) and isinstance(request.get("compiler"), Mapping):
                compiler_sha256 = str(request["compiler"]["sha256"])
        self.hooks = _validate_hook_rows(
            [dict(row) for row in hooks],
            "dispatcher hooks",
            compiler_sha256=compiler_sha256,
        )
        self.by_address = {int(row["address"]): row for row in self.hooks}
        if len(self.by_address) != len(self.hooks):
            raise Rejected("dispatcher hook union has conflicting addresses")
        # Every owned INT3 needs the same replay path.  Rewinding only the
        # Object+0x2e writes loses compiler/filter/PCode instructions and lets
        # the captured process continue from the byte after INT3.
        # Keep whether the corresponding breakpoint was accepted by the
        # target-function chronology.  Compiler functions before/after the
        # requested one still execute the same shared hook sites; they must
        # be single-stepped and re-armed, but must not create target events or
        # an object-write post edge.
        self.pending_steps: dict[int, tuple[int, bool]] = {}
        self.installed: set[int] = set()
        self.prefixes_validated = False

    def preflight(self) -> None:
        if self.prefixes_validated:
            return
        read_image = getattr(self.backend, "read_image", None)
        if not callable(read_image):
            raise Rejected("backend lacks live hook-prefix reads")
        expected_prefix = getattr(self.backend, "expected_hook_prefix", None)
        mismatches: list[str] = []
        for row in self.hooks:
            if callable(expected_prefix):
                try:
                    expected = bytes(expected_prefix(row))
                except (TypeError, ValueError, Rejected):
                    raise Rejected(f"backend could not authenticate hook prefix 0x{int(row['address']):08x}") from None
                if len(expected) != len(bytes.fromhex(str(row["prefix"]))):
                    raise Rejected(f"backend returned malformed hook prefix 0x{int(row['address']):08x}")
            else:
                expected = bytes.fromhex(str(row["prefix"]))
            actual = bytes(read_image(int(row["address"]), len(expected)))
            if actual != expected:
                mismatches.append(f"0x{int(row['address']):08x}")
        if mismatches:
            raise Rejected("live hook prefix mismatch: " + ", ".join(mismatches))
        self.prefixes_validated = True

    def install(self) -> None:
        self.preflight()
        install = getattr(self.backend, "install_breakpoint", None)
        if not callable(install):
            raise Rejected("backend lacks breakpoint installation")
        for row in self.hooks:
            address = int(row["address"])
            if address in self.installed:
                raise Rejected(f"duplicate breakpoint install 0x{address:08x}")
            install(address)
            self.installed.add(address)

    def remove_all(self) -> None:
        remove = getattr(self.backend, "remove_breakpoint", None)
        if not callable(remove):
            return
        # A native EXIT_PROCESS event invalidates the compiler address space;
        # the backend records that boundary before session validation.  There
        # is nothing left to restore in that process, and attempting writes
        # here would replace the useful chronology error/result with a
        # misleading cleanup failure.
        if bool(getattr(self.backend, "exited", False)):
            self.installed.clear()
            return
        failures: list[str] = []
        for address in sorted(self.installed):
            try:
                remove(address)
            except Exception as exc:
                failures.append(f"0x{address:08x}: {type(exc).__name__}: {exc}")
        self.installed.clear()
        if failures:
            raise Rejected("breakpoint cleanup failed: " + "; ".join(failures))

    def on_breakpoint(self, address: Any, thread_id: Any) -> None:
        absolute = _integer(address, "breakpoint address", nonnegative=True)
        thread = _integer(thread_id, "thread id", nonnegative=True)
        row = self.by_address.get(absolute)
        if row is None:
            raise Rejected(f"unexpected breakpoint 0x{absolute:08x}")
        if thread in self.pending_steps:
            raise Rejected("conflicting pending single-step on one thread")
        accepted = bool(self.session.on_hook(row, thread))
        self.pending_steps[thread] = (absolute, accepted)
        step = getattr(self.backend, "single_step", None)
        if not callable(step):
            raise Rejected("backend lacks single-step support")
        # Rearm only after the post-event.  This replays every owned INT3,
        # including the function filter and all PCode hooks.
        step(absolute, thread, rearm=False)
        self.installed.discard(absolute)

    def on_single_step(self, thread_id: Any) -> None:
        thread = _integer(thread_id, "thread id", nonnegative=True)
        pending = self.pending_steps.pop(thread, None)
        if pending is None:
            raise Rejected("single-step had no pending write breakpoint")
        address, accepted = pending
        row = self.by_address[address]
        if accepted and (
            row["role"] == "object_stack_write"
            or (
                row.get("lane") == "frontend"
                and row["role"] in {"generic_completed_insertion", "bulk_object_link"}
            )
        ):
            self.session.on_hook_post(row, thread)
        install = getattr(self.backend, "install_breakpoint", None)
        if not callable(install):
            raise Rejected("backend lacks breakpoint rearm")
        install(address)
        self.installed.add(address)


class CombinedCaptureSession:
    """State machine for one process and the shared stack/PCode event bus."""

    def __init__(self, auth: Mapping[str, Any], backend: CaptureBackend, *, session_id: str | None = None) -> None:
        self.auth = _normalize_auth(auth)
        request = self.auth["request"]
        request_session = _safe_session_id(request["session_id"])
        if session_id is not None and _safe_session_id(session_id) != request_session:
            raise Rejected("constructor session ID mismatch")
        self.session_id = request_session
        self.function = _text(request["function"], "request.function")
        if not _authorized_board_function(self.function, request.get("source")):
            raise Rejected("unsupported target function")
        if self.auth.get("explicit_tools"):
            _validate_compile_argv(
                request["argv"],
                cwd=request["cwd"],
                source=request["source"],
                compiler=request["compiler"],
                wrapper=request.get("wrapper") if "wrapper" in self.auth.get("explicit_tools", set()) else None,
            )
        self.backend = backend
        capabilities = set(getattr(backend, "capabilities", ()))
        required = {"read_image", "install_breakpoint", "remove_breakpoint", "single_step", "run", "close"}
        missing = sorted(name for name in required if not hasattr(backend, name) and name not in capabilities)
        if missing:
            raise Rejected("backend capability gap: " + ", ".join(missing))
        self.bus = EventBus(session_id=self.session_id, function=self.function)
        self.ledger = PointerLedger(self.session_id)
        self.dispatcher = SharedBreakpointDispatcher(backend, self, hooks=self.auth["hooks"])
        self.started = False
        self.exited = False
        self.exit_code: int | None = None
        self.function_entered = False
        # Hook sites are shared by every function compiled in the TU.  The
        # requested function may be well after the first one, so keep the
        # chronology window explicit instead of treating an earlier function's
        # allocation as target evidence.
        self.target_complete = False
        self.function_exited = False
        self.inventory_captured = False
        self.inventory_complete = False
        self.inventory_structure_ready = False
        self.inventory_snapshot_reasons: set[str] = set()
        self.stack_allocation_pre_seen = False
        self.inventory_event_emitted = False
        self.inventory_rows: dict[str, list[dict[str, Any]]] = {"locals": [], "arguments": []}
        # Object/VarInfo joins are capture-internal.  The VarInfo pointer is
        # retained only long enough to authenticate the post-allocation hook;
        # it is never serialized or used as an ownership identity.
        self.varinfo_by_token: dict[str, int] = {}
        self.varinfo_owners: dict[int, str] = {}
        self.mappings: dict[str, dict[str, Any]] = {}
        self.vreg_owners: dict[str, str] = {}
        self.ig_nodes: set[Any] = set()
        self.physical_mappings: dict[str, dict[str, Any]] = {}
        self.physical_reg_owners: dict[tuple[str, int], str] = {}
        self.physical_unknown: dict[str, str] = {}
        self.pending_physical_rows: list[dict[str, Any]] = []
        self.unknown: list[str] = []
        self.pending_writes: dict[int, dict[str, Any]] = {}
        self.machine_decoder = MachineEmissionDecoder()
        self.pcode_tokens: dict[int, str] = {}
        self.pcode_offsets: dict[str, int] = {}
        self.pcode_offset_owners: dict[int, str] = {}
        self.ig_tokens: dict[int, str] = {}
        self.hidden_ig_tokens: dict[str, str] = {}
        self.pending_pcode_colors: dict[int, dict[str, Any]] = {}
        self.pcode_color_evidence: set[tuple[str, int]] = set()
        # PCode-color evidence is scoped to one capture-local PCode token.
        # It may authenticate an Object's physical color for that instruction
        # even when the separate Object-to-vreg reader is unavailable.  Never
        # promote the operand index to a virtual-register identity.
        self.pcode_color_owners: dict[tuple[str, str, int], str] = {}
        # GC/2.6 frontend chronology is consumed by this same session.  It is
        # deliberately absent for GC/2.7: that compiler has a different
        # frontend image/profile and must not inherit these hook addresses.
        self.frontend_session: Any | None = None
        self.frontend_packet: dict[str, Any] | None = None
        self.frontend_failure: str | None = None
        if str(request["compiler"]["sha256"]).lower() == GC26_COMPILER_SHA256:
            provenance = {
                "source_sha256": str(request["source"]["sha256"]),
                "compiler_sha256": str(request["compiler"]["sha256"]),
                # The combined event streams and frontend stream are produced
                # by this authenticated request/session.  The request digest
                # is the stable same-session trace anchor available before the
                # envelope is sealed.
                "trace_sha256": self.auth["request_sha256"],
                "session_id": self.session_id,
            }
            try:
                self.frontend_session = _frontend_chronology.FrontendChronologySession(
                    provenance,
                    function=self.function,
                )
            except Exception:
                # Frontend chronology is an optional evidence lane.  A bad or
                # incomplete frontend setup must remain UNKNOWN while the
                # stack/PCode lanes continue to capture in this process.
                self.frontend_failure = "incomplete frontend chronology"

    def _check_process(self, process_id: Any | None) -> None:
        if process_id is None:
            return
        self.bus.check_process(process_id)

    def _unknown(self, reason: str) -> None:
        reason = _text(reason, "unknown reason")
        if reason not in self.unknown:
            self.unknown.append(reason)

    def _pcode_token(self, pointer: Any, emitted_offset: int | None = None) -> str | None:
        try:
            value = _integer(pointer, "PCode node pointer", nonnegative=True)
        except Rejected:
            self._unknown("missing PCode token")
            return None
        if value == 0:
            self._unknown("missing PCode token")
            return None
        token = self.pcode_tokens.get(value)
        if token is None:
            token = f"pcode-{self.session_id}-{len(self.pcode_tokens):06d}"
            self.pcode_tokens[value] = token
        if emitted_offset is None:
            return token
        return self._bind_pcode_offset(token, emitted_offset)

    def _bind_pcode_offset(self, token: str, emitted_offset: int) -> str | None:
        match = PCODE_TOKEN_RE.fullmatch(token)
        if match is None or match.group("session") != self.session_id:
            self._unknown("missing PCode token")
            return None
        prior_offset = self.pcode_offsets.get(token)
        if prior_offset not in (None, emitted_offset):
            self._unknown("ambiguous PCode token")
            return None
        prior_token = self.pcode_offset_owners.get(emitted_offset)
        if prior_token not in (None, token):
            self._unknown("ambiguous PCode token")
            return None
        self.pcode_offsets[token] = emitted_offset
        self.pcode_offset_owners[emitted_offset] = token
        return token

    def _ig_token(self, pointer: Any) -> str | None:
        try:
            value = _integer(pointer, "IG node pointer", nonnegative=True)
        except Rejected:
            self._unknown("missing IG-node identity")
            return None
        if value == 0:
            self._unknown("missing IG-node identity")
            return None
        token = self.ig_tokens.get(value)
        if token is None:
            token = f"ig-{self.session_id}-{len(self.ig_tokens):06d}"
            self.ig_tokens[value] = token
        return token

    def _hidden_ig_token(self, ig_token: str) -> str:
        token = self.hidden_ig_tokens.get(ig_token)
        if token is None:
            token = f"hidden-ig-{self.session_id}-{len(self.hidden_ig_tokens):06d}"
            self.hidden_ig_tokens[ig_token] = token
        return token

    def _capture_pcode_color(self, row: Mapping[str, Any], thread: int) -> dict[str, Any] | None:
        method = getattr(self.backend, "capture_pcode", None)
        raw = method(row["id"], thread) if callable(method) else None
        if not isinstance(raw, Mapping):
            raise Rejected("PCode color backend returned a non-object")
        status = raw.get("status")
        if status == "NOOP":
            if row["id"] != "pcode_color_post" or thread in self.pending_pcode_colors:
                raise Rejected("PCode color NOOP is not a non-register post path")
            return None
        required = {
            "pcode_pointer", "ig_pointer", "operand_ordinal", "operand_count",
            "operand_kind", "register_class", "operand_index", "final_color",
            "ig_flags", "object_pointer",
        }
        if status not in {"PENDING", "CAPTURED"} or not required.issubset(raw):
            raise Rejected("PCode color evidence is incomplete")
        values = {
            key: _integer(raw[key], f"PCode color {key}", nonnegative=key not in {"final_color"})
            for key in required
        }
        if not 0 <= values["operand_ordinal"] < values["operand_count"] <= 256:
            raise Rejected("PCode color operand chronology is invalid")
        if not 0 <= values["operand_kind"] <= 0xFF or not 0 <= values["operand_index"] <= 0x7FFF:
            raise Rejected("PCode color operand identity is invalid")
        if values["register_class"] not in {3, 4}:
            raise Rejected("PCode color operand class is not GPR/FPR")
        if not 0 <= values["final_color"] <= 31 or not 0 <= values["ig_flags"] <= 0xFFFF:
            raise Rejected("PCode color result is invalid")
        pcode_token = self._pcode_token(values["pcode_pointer"])
        ig_token = self._ig_token(values["ig_pointer"])
        if pcode_token is None or ig_token is None:
            raise Rejected(self.unknown[-1])
        bank = "GPR" if values["register_class"] == 4 else "FPR"
        payload: dict[str, Any] = {
            "hook_id": "pcode_color_post",
            "status": "CAPTURED",
            "stage": "pcode_color_post",
            "pcode_token": pcode_token,
            "ig_token": ig_token,
            "operand_ordinal": values["operand_ordinal"],
            "operand_count": values["operand_count"],
            "operand_kind": values["operand_kind"],
            "operand_class": values["register_class"],
            "operand_bank": bank,
            "operand_index": values["operand_index"],
            "final_color": values["final_color"],
            "ig_flags": values["ig_flags"],
            "confirmed": True,
        }
        object_pointer = values["object_pointer"]
        binding = self.ledger.kind_for(object_pointer) if object_pointer else None
        if binding is None:
            payload["hidden_owner_token"] = self._hidden_ig_token(ig_token)
        else:
            _kind, object_token = binding
            payload["object_token"] = object_token
        if row["id"] == "pcode_color_pre":
            if status != "PENDING" or thread in self.pending_pcode_colors:
                raise Rejected("nested PCode color writeback")
            self.pending_pcode_colors[thread] = {"raw": values, "payload": payload}
            return None
        if row["id"] != "pcode_color_post" or status != "CAPTURED":
            raise Rejected("PCode color hook/status mismatch")
        pending = self.pending_pcode_colors.pop(thread, None)
        if pending is None or pending["raw"] != values or pending["payload"] != payload:
            raise Rejected("PCode color pre/post evidence conflicts")
        evidence_key = (pcode_token, values["operand_ordinal"])
        if evidence_key in self.pcode_color_evidence:
            raise Rejected("duplicate PCode color evidence")
        self.pcode_color_evidence.add(evidence_key)
        if "object_token" in payload:
            color_key = (pcode_token, bank, values["final_color"])
            prior_owner = self.pcode_color_owners.get(color_key)
            if prior_owner not in (None, payload["object_token"]):
                raise Rejected("PCode color evidence maps one physical color to multiple Objects")
            self.pcode_color_owners[color_key] = str(payload["object_token"])
        return payload

    def _machine_owner_joins(
        self,
        pcode_token: str,
        registers: Mapping[str, Any],
    ) -> tuple[list[dict[str, str]], list[dict[str, str]], str | None]:
        joins: list[dict[str, str]] = []
        physical_joins: list[dict[str, str]] = []
        seen: set[tuple[str, str]] = set()
        for register in registers.values():
            if not isinstance(register, str) or re.fullmatch(r"[rf](?:[0-9]|[12][0-9]|3[01])", register) is None:
                continue
            bank = "GPR" if register.startswith("r") else "FPR"
            physical_index = int(register[1:])
            assigned_token = self.physical_reg_owners.get((bank, physical_index))
            color_token = self.pcode_color_owners.get((pcode_token, bank, physical_index))
            if assigned_token is not None and color_token is not None and assigned_token != color_token:
                return [], [], "machine owner register-bank mismatch"
            token = assigned_token or color_token
            if token is None:
                continue
            if assigned_token is not None:
                physical = self.physical_mappings.get(assigned_token)
                if (
                    not isinstance(physical, Mapping)
                    or physical.get("status") != "EXACT"
                    or physical.get("bank") != bank
                    or int(physical.get("physical_reg", -1)) != physical_index
                ):
                    return [], [], "machine owner register-bank mismatch"
            key = (register, token)
            if key in seen:
                continue
            seen.add(key)
            physical_joins.append({"physical_register": register, "object_token": token})
            mapping = self.mappings.get(token)
            if not isinstance(mapping, Mapping) or mapping.get("status") != "EXACT":
                continue
            vreg_id = str(mapping.get("vreg_id", ""))
            if mapping.get("bank") != bank or not vreg_id.startswith("r" if bank == "GPR" else "f"):
                return [], [], "machine owner register-bank mismatch"
            joins.append({"physical_register": register, "object_token": token, "vreg_id": vreg_id})
        key = lambda row: (row["physical_register"], row["object_token"])
        return sorted(joins, key=key), sorted(physical_joins, key=key), None

    def _unknown_machine_emission(self, hook_id: str, reason: str) -> dict[str, Any]:
        self._unknown(reason)
        self.machine_decoder.invalidate()
        return {"hook_id": hook_id, "status": "UNKNOWN", "reason": reason}

    def _capture_machine_emission(self, row: Mapping[str, Any], thread: int) -> dict[str, Any]:
        method = getattr(self.backend, "capture_machine_emission", None)
        raw = method(row["id"], thread) if callable(method) else None
        if not isinstance(raw, Mapping):
            return self._unknown_machine_emission(row["id"], "incomplete machine emission evidence")
        if raw.get("status") == "UNKNOWN":
            reason = str(raw.get("reason", "incomplete machine emission evidence"))
            if reason not in _KNOWN_UNKNOWN_REASONS:
                reason = "incomplete machine emission evidence"
            return self._unknown_machine_emission(row["id"], reason)
        required = {"pcode_pointer", "emitted_offset", "opcode_enum", "encoded_value", "descriptor_base"}
        if not required.issubset(raw):
            return self._unknown_machine_emission(row["id"], "incomplete machine emission evidence")
        try:
            emitted_offset = _integer(raw["emitted_offset"], "machine emitted offset", nonnegative=True)
            opcode_enum = _integer(raw["opcode_enum"], "machine opcode enum", nonnegative=True)
            encoded_value = _integer(raw["encoded_value"], "machine encoded value", nonnegative=True)
            descriptor_base = _integer(raw["descriptor_base"], "machine descriptor base", nonnegative=True)
        except Rejected:
            reason = "unsupported machine operand"
            return self._unknown_machine_emission(row["id"], reason)
        token = self._pcode_token(raw["pcode_pointer"], emitted_offset)
        if token is None:
            self.machine_decoder.invalidate()
            return {"hook_id": row["id"], "status": "UNKNOWN", "reason": self.unknown[-1]}
        decoded = self.machine_decoder.decode(
            pcode_token=token,
            emitted_offset=emitted_offset,
            opcode_enum=opcode_enum,
            encoded_value=encoded_value,
            descriptor_base=descriptor_base,
        )
        if decoded["status"] == "UNKNOWN":
            self._unknown(str(decoded["reason"]))
            return {"hook_id": row["id"], **decoded}
        owner_joins, physical_owner_joins, join_reason = self._machine_owner_joins(
            token,
            decoded["registers"],
        )
        if join_reason is not None:
            return self._unknown_machine_emission(row["id"], join_reason)
        decoded["owner_joins"] = owner_joins
        decoded["physical_owner_joins"] = physical_owner_joins
        return {"hook_id": row["id"], **decoded}

    def _frontend_unknown(self) -> None:
        """Disable only the frontend evidence lane after an invalid join.

        Frontend hooks are diagnostic enrichment.  A missing, duplicate, or
        ambiguous frontend event must not discard the already authenticated
        stack/PCode events from this same compiler process.
        """

        self.frontend_failure = "incomplete frontend chronology"
        self.frontend_session = None
        self.frontend_packet = None
        self._unknown(self.frontend_failure)

    def _frontend_snapshot_rows(self) -> list[dict[str, Any]]:
        """Return the pointer-bearing snapshot only to the private frontend lane."""

        method = getattr(self.backend, "snapshot_objects", None)
        if callable(method):
            raw_rows = method()
        else:
            method = getattr(self.backend, "snapshot_inventory", None)
            if not callable(method):
                raise Rejected("frontend Object snapshot is unavailable")
            raw = method()
            if not isinstance(raw, Mapping):
                raise Rejected("frontend Object snapshot is malformed")
            raw_rows = [
                *list(raw.get("locals", ())),
                *list(raw.get("arguments", ())),
            ]
        if not isinstance(raw_rows, Sequence) or isinstance(raw_rows, (str, bytes, bytearray)):
            raise Rejected("frontend Object snapshot is malformed")
        rows: list[dict[str, Any]] = []
        for raw in raw_rows:
            if not isinstance(raw, Mapping):
                raise Rejected("frontend Object snapshot row is malformed")
            if "pointer" not in raw or "varinfo_pointer" not in raw:
                raise Rejected("frontend Object snapshot row lacks VarInfo identity")
            row: dict[str, Any] = {
                "pointer": raw["pointer"],
                "varinfo_pointer": raw["varinfo_pointer"],
            }
            if "home_value" in raw:
                row["home_value"] = raw["home_value"]
            rows.append(row)
        return rows

    def _frontend_maybe_snapshot(self) -> None:
        session = self.frontend_session
        if session is None or not session.target_entry_seen or not session.bulk_seen or session.snapshot_seen:
            return
        session.on_post_allocation_snapshot(self._frontend_snapshot_rows())

    def _frontend_complete_hook(self, row: Mapping[str, Any], thread: int) -> None:
        session = self.frontend_session
        if session is None:
            return
        hook_id = str(row["id"])
        capture = getattr(self.backend, "capture_frontend", None)
        raw: Any
        if callable(capture):
            raw = capture(hook_id, thread)
        elif hook_id in _frontend_chronology.GENERIC_HOOK_IDS:
            register = _frontend_chronology.GENERIC_OBJECT_REGISTERS.get(hook_id)
            if register is None:
                raise Rejected(f"frontend hook has no authenticated Object register: {hook_id}")
            pointer = self._call_backend("read_register", thread, register)
            raw = {"pointer": pointer}
        elif hook_id == "bulk_object_link":
            rows = self._frontend_snapshot_rows()
            raw = {"object_pointers": [item["pointer"] for item in rows]}
        else:
            raise Rejected(f"unsupported frontend hook completion: {hook_id}")
        if not isinstance(raw, Mapping):
            raise Rejected("frontend hook backend returned a non-object")
        if hook_id in _frontend_chronology.GENERIC_HOOK_IDS:
            pointer = raw.get("pointer", raw.get("object_pointer"))
            session.on_hook_complete(hook_id, pointer=pointer)
        elif hook_id == "bulk_object_link":
            pointers = raw.get("object_pointers")
            session.on_hook_complete(hook_id, object_pointers=pointers)
            self._frontend_maybe_snapshot()

    def _frontend_target_entry(self, observed: Any) -> None:
        session = self.frontend_session
        if session is None or observed != self.function:
            return
        session.on_target_boundary(phase="entry", function=self.function)
        self._frontend_maybe_snapshot()

    def on_process_started(self, process_id: Any | None = None) -> None:
        if self.started:
            raise Rejected("native process started twice")
        actual = process_id
        if actual is None:
            actual = getattr(self.backend, "process_id", None)
        if actual is None:
            raise Rejected("actual native PID is missing")
        backend_pid = getattr(self.backend, "process_id", None)
        if backend_pid not in (None, actual):
            raise Rejected("native process id does not match backend process")
        self.bus.bind_process(actual)
        self.started = True
        if self.frontend_session is not None:
            hook_bytes = {
                str(row["id"]): str(row["prefix"])
                for row in _frontend_chronology.HOOKS
            }
            try:
                self.frontend_session.on_process_started(hook_bytes=hook_bytes)
            except Exception:
                self._frontend_unknown()

    def _call_backend(self, name: str, *args: Any) -> Any:
        method = getattr(self.backend, name, None)
        if not callable(method):
            return None
        try:
            return method(*args)
        except TypeError:
            if args:
                return method()
            raise

    def _capture_inventory(self, *, force: bool = False) -> None:
        # The post-allocation hook can precede numeric_stack_alloc_pre.  A
        # first snapshot may therefore see a still-empty/partial list; permit
        # later stack edges to refresh the ledger without changing any already
        # assigned token ordinals.
        if self.inventory_captured and self.inventory_structure_ready and not force:
            return
        raw: Any = None
        snapshot_reasons: set[str] = set()
        method = getattr(self.backend, "snapshot_inventory", None)
        if callable(method):
            raw = method()
        else:
            locals_method = getattr(self.backend, "snapshot_locals", None)
            arguments_method = getattr(self.backend, "snapshot_arguments", None)
            if callable(locals_method) or callable(arguments_method):
                raw = {
                    "locals": locals_method() if callable(locals_method) else [],
                    "arguments": arguments_method() if callable(arguments_method) else [],
                }
            else:
                # Native adapters may expose the two fixed list heads through
                # one method.  The addresses stay internal to this call.
                objects_method = getattr(self.backend, "snapshot_objects", None)
                if callable(objects_method):
                    try:
                        raw = {"locals": objects_method(LOCALS_LIST_HEAD), "arguments": objects_method(ARGUMENTS_LIST_HEAD)}
                    except TypeError:
                        raw = objects_method()
        if not isinstance(raw, Mapping):
            self.inventory_snapshot_reasons = {"incomplete inventory"}
            self.inventory_captured = True
            self.inventory_structure_ready = False
            self.inventory_complete = False
            return
        self.inventory_structure_ready = all(
            key in raw and isinstance(raw[key], Sequence) and not isinstance(raw[key], (str, bytes, bytearray))
            for key in ("locals", "arguments")
        )
        for kind, key in (("local", "locals"), ("argument", "arguments")):
            rows = raw.get(key)
            if not isinstance(rows, Sequence) or isinstance(rows, (str, bytes, bytearray)):
                snapshot_reasons.add("incomplete inventory")
                self.inventory_complete = False
                continue
            normalized_rows: list[dict[str, Any]] = []
            seen_tokens: set[str] = set()
            for index, item in enumerate(rows):
                if not isinstance(item, Mapping):
                    snapshot_reasons.add("null object identity")
                    continue
                pointer = item.get("pointer", item.get("object"))
                token = self.ledger.register(kind, pointer)
                if token is None:
                    snapshot_reasons.add(
                        "null object identity" if pointer in (None, 0) else "duplicate object identity"
                    )
                    continue
                if token in seen_tokens:
                    self.ledger.mark_unknown(token, "duplicate object identity")
                    snapshot_reasons.add("duplicate object identity")
                    continue
                seen_tokens.add(token)
                row: dict[str, Any] = {"token": token, "kind": kind}
                # Names are retained only as direct backend metadata.  They
                # never participate in ownership matching.
                if isinstance(item.get("name"), str) and item["name"]:
                    row["name"] = item["name"]
                if "varinfo_pointer" in item or "varinfo" in item:
                    raw_varinfo = item.get("varinfo_pointer", item.get("varinfo"))
                    try:
                        varinfo_pointer = _integer(raw_varinfo, "Object VarInfo pointer", nonnegative=True)
                    except Rejected:
                        snapshot_reasons.add("missing or invalid Object VarInfo pointer")
                    else:
                        if varinfo_pointer == 0:
                            snapshot_reasons.add("missing or invalid Object VarInfo pointer")
                        else:
                            prior_token = self.varinfo_owners.get(varinfo_pointer)
                            if prior_token not in (None, token):
                                snapshot_reasons.add("duplicate Object VarInfo identity")
                            else:
                                self.varinfo_by_token[token] = varinfo_pointer
                                self.varinfo_owners[varinfo_pointer] = token
                normalized_rows.append(row)
            self.inventory_rows[key] = normalized_rows
        self.inventory_captured = True
        self.inventory_snapshot_reasons = snapshot_reasons
        self.inventory_complete = not snapshot_reasons and self.inventory_structure_ready
        # Keep compiler_list at the allocator phase boundary.  A physical
        # post hook may arrive earlier and can force a provisional snapshot;
        # serializing that provisional list would make the later refreshed
        # token ledger disagree with the immutable event.
        if self.function_entered and self.stack_allocation_pre_seen and not self.inventory_event_emitted:
            self.bus.emit(
                "stack",
                "compiler_list",
                {"locals": self.ledger.tokens("local"), "arguments": self.ledger.tokens("argument")},
            )
            self.inventory_event_emitted = True
        if self.inventory_structure_ready:
            self._flush_pending_physical_rows()

    def _row_for_pointer(self, pointer: Any, kind: str | None = None) -> tuple[str | None, str | None]:
        if kind is not None and kind not in {"local", "argument"}:
            self._unknown("null object identity")
            return None, None
        if kind is not None:
            token = self.ledger.lookup(kind, pointer)
            return token, kind
        result = self.ledger.kind_for(pointer)
        if result is None:
            return None, None
        kind_value, token = result
        return token, kind_value

    def _normalize_register_mapping(self, row: Mapping[str, Any]) -> dict[str, Any] | None:
        if not isinstance(row, Mapping):
            self._unknown("missing or invalid vreg identity")
            return None
        kind_value = row.get("kind", row.get("object_kind"))
        kind = str(kind_value).lower() if isinstance(kind_value, str) else None
        pointer = row.get("object", row.get("object_id", row.get("source_object", row.get("object_pointer"))))
        # A raw object_pointer is intentionally accepted only at the internal
        # backend boundary and is removed before the event is emitted.
        if pointer is None:
            pointer = row.get("pointer")
        token = row.get("object_token")
        if token is not None:
            token = _text(token, "object_token")
            if not TOKEN_RE.fullmatch(token):
                self._unknown("missing or invalid vreg identity")
                return None
            if token not in self.ledger._by_token:
                self._unknown("null object identity")
                return None
            if pointer is not None:
                # Backend-provided tokens are advisory serialization, not an
                # ownership identity.  Re-derive the canonical token from the
                # authenticated Object pointer whenever both are present.  A
                # stale local/argument spelling (for example an argument token
                # attached to local ``@1310``) must not cross into the event
                # stream or disagree with the final inventory row.
                canonical_token, canonical_kind = self._row_for_pointer(pointer)
                if canonical_token is None or canonical_kind is None:
                    self._unknown("null object identity")
                    return None
                token, kind = canonical_token, canonical_kind
            else:
                kind = token_kind
        else:
            token, kind = self._row_for_pointer(pointer, kind)
        if token is None:
            self._unknown("null object identity")
            return None
        ig_node = row.get("ig_node", row.get("ig_node_id"))
        if ig_node in (None, 0, ""):
            self.ledger.mark_unknown(token, "missing IG-node identity")
            self._unknown("missing or invalid vreg identity")
            return None
        try:
            ig_key = (type(ig_node).__name__, int(ig_node)) if isinstance(ig_node, (int, str)) else repr(ig_node)
        except (TypeError, ValueError):
            ig_key = repr(ig_node)
        if ig_key in self.ig_nodes:
            self.ledger.mark_unknown(token, "reused IG-node identity")
            self._unknown("reused IG-node identity")
            return None
        self.ig_nodes.add(ig_key)
        raw_vreg = row.get("vreg_id", row.get("virtual_register"))
        if raw_vreg is None:
            self.ledger.mark_unknown(token, "missing vreg identity")
            self._unknown("missing or invalid vreg identity")
            return None
        try:
            vreg_id = _validate_vreg(raw_vreg, "vreg_id")
        except Rejected:
            self.ledger.mark_unknown(token, "invalid vreg identity")
            self._unknown("missing or invalid vreg identity")
            return None
        bank_raw = row.get("bank", row.get("register_class"))
        if not isinstance(bank_raw, str) or bank_raw.lower() not in {"gpr", "fpr"}:
            self.ledger.mark_unknown(token, "missing GPR/FPR bank evidence")
            self._unknown("missing GPR/FPR bank evidence")
            return None
        bank = bank_raw.lower()
        expected_prefix = "r" if bank == "gpr" else "f"
        if not vreg_id.startswith(expected_prefix):
            self.ledger.mark_unknown(token, "GPR/FPR bank conflicts with vreg")
            self._unknown("missing GPR/FPR bank evidence")
            return None
        if token in self.mappings:
            self.ledger.mark_unknown(token, "one-to-many object-to-vreg claim")
            self._unknown("one-to-many object-to-vreg claim")
            return None
        prior = self.vreg_owners.get(vreg_id)
        if prior is not None:
            self.ledger.mark_unknown(token, "one-to-many vreg-to-object claim")
            self.ledger.mark_unknown(prior, "one-to-many vreg-to-object claim")
            self._unknown("one-to-many vreg-to-object claim")
            return None
        mapping = {"status": "EXACT", "vreg_id": vreg_id, "bank": bank.upper()}
        self.mappings[token] = mapping
        self.vreg_owners[vreg_id] = token
        return {"object_token": token, **mapping}

    def on_regalloc(self, row: Mapping[str, Any]) -> None:
        normalized = self._normalize_register_mapping(row)
        if normalized is None:
            payload = {"status": "UNKNOWN", "reason": self.unknown[-1] if self.unknown else "incomplete regalloc"}
        else:
            payload = normalized
        self.bus.emit("pcode", "regalloc_assignment", payload)

    def _physical_unknown(self, token: str | None, reason: str) -> None:
        """Record a physical-assignment failure without downgrading vreg ownership."""

        reason = _text(reason, "physical assignment reason")
        self._unknown(reason)
        if token is not None:
            self.physical_unknown.setdefault(token, reason)

    def _normalize_physical_mapping(self, row: Mapping[str, Any]) -> dict[str, Any] | None:
        """Authenticate one post-allocation Object/VarInfo physical mapping.

        The backend may pass raw Object/VarInfo pointers here, but this method
        converts them to the already captured Object token before the row can
        reach the event bus.  Physical assignments intentionally do not carry
        a virtual-register field: the verified hook observes VarInfo's physical
        result only.
        """

        if not isinstance(row, Mapping):
            self._physical_unknown(None, "incomplete physical register evidence")
            return None
        kind_value = row.get("kind", row.get("object_kind"))
        kind = str(kind_value).lower() if isinstance(kind_value, str) else None
        pointer = row.get("object", row.get("object_pointer", row.get("pointer")))
        token = row.get("object_token")
        if token is not None:
            try:
                token = _text(token, "object_token")
                token_kind, _ = _token_parts(token, "object_token", self.session_id)
            except Rejected:
                self._physical_unknown(None, "null object identity")
                return None
            if token not in self.ledger._by_token:
                self._physical_unknown(None, "null object identity")
                return None
            if kind is not None and kind != token_kind:
                self._physical_unknown(token, "null object identity")
                return None
            if pointer is not None:
                pointer_token, pointer_kind = self._row_for_pointer(pointer, kind)
                if pointer_token != token or pointer_kind != token_kind:
                    self._physical_unknown(token, "null object identity")
                    return None
        else:
            token, kind = self._row_for_pointer(pointer, kind)
        if token is None:
            self._physical_unknown(None, "null object identity")
            return None

        raw_varinfo = row.get("varinfo_pointer", row.get("varinfo"))
        try:
            varinfo_pointer = _integer(raw_varinfo, "Object VarInfo pointer", nonnegative=True)
        except Rejected:
            self._physical_unknown(token, "missing or invalid Object VarInfo pointer")
            return None
        expected_varinfo = self.varinfo_by_token.get(token)
        if varinfo_pointer == 0 or expected_varinfo is None or expected_varinfo != varinfo_pointer:
            self._physical_unknown(token, "missing or invalid Object VarInfo pointer")
            return None
        if self.varinfo_owners.get(varinfo_pointer) != token:
            self._physical_unknown(token, "missing or invalid Object VarInfo pointer")
            return None
        if row.get("status") == "UNKNOWN":
            reason = row.get("reason")
            if reason not in _KNOWN_UNKNOWN_REASONS:
                reason = "incomplete physical register evidence"
            self._physical_unknown(token, str(reason))
            return None

        def _field_int(name: str, *aliases: str) -> int | None:
            value = next((row[key] for key in (name, *aliases) if key in row), None)
            try:
                return _integer(value, f"physical VarInfo {name}")
            except Rejected:
                self._physical_unknown(token, "incomplete physical register evidence")
                return None

        noregister = _field_int("noregister")
        flags = _field_int("flags")
        rclass = _field_int("rclass", "register_class", "class")
        reg = _field_int("reg", "physical_reg")
        reg_hi = _field_int("reg_hi", "physical_reg_hi")
        if None in {noregister, flags, rclass, reg, reg_hi}:
            return None
        assert noregister is not None and flags is not None and rclass is not None
        assert reg is not None and reg_hi is not None
        if noregister != 0:
            self._physical_unknown(token, "physical register marked no-register")
            return None
        if not (flags & 2):
            self._physical_unknown(token, "physical register flags missing assignment bit")
            return None
        bank = PHYSICAL_REGISTER_CLASSES.get(rclass)
        if bank is None:
            self._physical_unknown(token, "unsupported physical register class")
            return None
        if not 0 <= reg <= 31 or not 0 <= reg_hi <= 31:
            self._physical_unknown(token, "physical register index out of range")
            return None
        # MWCC uses a zero high half for the ordinary single-register case;
        # only a non-zero high half consumes a second physical register.
        physical_indices = {reg}
        if reg_hi != 0:
            physical_indices.add(reg_hi)
        existing = self.physical_mappings.get(token)
        if existing is not None:
            self._physical_unknown(token, "duplicate physical-register assignment")
            return None
        owners = {self.physical_reg_owners.get((bank, index)) for index in physical_indices}
        owners.discard(None)
        if owners:
            self._physical_unknown(token, "one-to-many physical-register-to-object claim")
            for owner in owners:
                if owner != token:
                    self._physical_unknown(owner, "one-to-many physical-register-to-object claim")
            return None
        mapping = {"status": "EXACT", "physical_reg": reg, "bank": bank}
        self.physical_mappings[token] = mapping
        for index in physical_indices:
            self.physical_reg_owners[(bank, index)] = token
        return {"object_token": token, **mapping}

    def _flush_pending_physical_rows(self, *, force_unknown: bool = False) -> None:
        if not self.pending_physical_rows:
            return
        if not force_unknown and not self.stack_allocation_pre_seen:
            return
        pending = self.pending_physical_rows
        self.pending_physical_rows = []
        if force_unknown or not self.inventory_structure_ready:
            for _row in pending:
                self._physical_unknown(None, "incomplete physical register evidence")
                self.bus.emit(
                    "pcode",
                    "physical_reg_assignment",
                    {"status": "UNKNOWN", "reason": "incomplete physical register evidence"},
                )
            return
        for row in pending:
            self.on_physical_regalloc(row)

    def on_physical_regalloc(self, row: Mapping[str, Any]) -> None:
        if not self.inventory_structure_ready or not self.stack_allocation_pre_seen:
            # Keep the process-local hook-specific Object/VarInfo evidence until a complete Object
            # list snapshot binds the pointer to a session token.  Raw
            # pointers never enter the event bus or serialized envelope.
            self.pending_physical_rows.append(dict(row) if isinstance(row, Mapping) else {})
            return
        normalized = self._normalize_physical_mapping(row)
        if normalized is None:
            reason = self.unknown[-1] if self.unknown else "incomplete physical register evidence"
            payload = {"status": "UNKNOWN", "reason": reason}
        else:
            payload = normalized
        self.bus.emit("pcode", "physical_reg_assignment", payload)

    def _maybe_complete_target(self) -> None:
        """Close the target window after every required hook has landed."""

        if not self.function_entered or self.target_complete:
            return
        events = self.bus.events
        stack_hook_ids = {
            str(event.get("hook_id"))
            for event in events
            if event["lane"] == "stack" and event.get("event_kind") in {
                "numeric_stack_alloc_pre",
                "numeric_stack_alloc_post",
                "object_stack_write_post",
            }
        }
        expected_stack = {
            "allocation_pre",
            "allocation_post",
            *(f"{hook_id}" for hook_id in WRITE_HOOK_IDS),
        }
        pcode_complete = any(
            event["lane"] == "pcode" and event.get("event_kind") == "regalloc_assignment"
            for event in events
        )
        physical_complete = any(
            event["lane"] == "pcode" and event.get("event_kind") == "physical_reg_assignment"
            for event in events
        )
        # regalloc_post is the allocator's per-Object epilogue, not a single
        # function-level edge.  Keep the target open after the first physical
        # row so later Object replacements can be authenticated as well.  A
        # subsequent function-filter boundary closes the window above.
        if physical_complete:
            return
        if expected_stack.issubset(stack_hook_ids) and pcode_complete and physical_complete:
            self.target_complete = True

    def _backend_write_payload(self, row: Mapping[str, Any], thread: int) -> dict[str, Any]:
        method = getattr(self.backend, "capture_stack_write", None)
        raw: Any = method(row["id"], thread) if callable(method) else None
        if raw is None:
            object_pointer = self._call_backend("read_register", thread, "ebx")
            value = self._call_backend("read_register", thread, "eax")
            raw = {"object": object_pointer, "value": value}
        if not isinstance(raw, Mapping):
            raise Rejected("stack write backend returned a non-object")
        object_pointer = raw.get("object", raw.get("object_pointer", raw.get("pointer")))
        kind = raw.get("kind", raw.get("object_kind"))
        kind_text = str(kind).lower() if isinstance(kind, str) else None
        token, _ = self._row_for_pointer(object_pointer, kind_text)
        if token is None:
            self._unknown("null object identity")
        value = raw.get("value", raw.get("target_slot"))
        if value is None:
            self._unknown("incomplete stack write")
            value = 0
        slot = _integer(value, "stack write target slot")
        return {"hook_id": row["id"], "object_token": token or "UNKNOWN", "target_slot": slot}

    def on_hook(self, row: Mapping[str, Any], thread: int) -> bool:
        role = row["role"]
        if role == "frontend_reset":
            if self.frontend_session is not None:
                try:
                    self.frontend_session.on_hook("reset")
                except Exception:
                    self._frontend_unknown()
            return True
        if role == "function_filter":
            if self.target_complete:
                return False
            observed = self._call_backend("current_function")
            if observed is not None and observed != self.function:
                # This is ordinary compiler chronology before the requested
                # definition.  The dispatcher still replays/rearms the site,
                # but no target event or UNKNOWN ledger entry is emitted.
                # Once the target has entered, the next function boundary is
                # the only durable end marker for a repeated regalloc_post
                # site.  Mark it complete there; the physical hook itself is
                # intentionally allowed to fire many times per function.
                if self.function_entered:
                    self.target_complete = True
                return False
            if self.function_entered:
                raise Rejected("target function entry observed twice")
            if self.frontend_session is not None:
                try:
                    self._frontend_target_entry(observed)
                except Exception:
                    self._frontend_unknown()
            self.function_entered = True
            self.bus.emit("stack", "function_entry", {"hook_id": row["id"]})
            return True
        if row.get("lane") == "frontend":
            # Frontend Object insertions/linking delimit the target epoch and
            # therefore legitimately occur before the shared function-filter
            # breakpoint selects the requested function.  The standalone
            # chronology consumer uses that same order.  Do not gate these
            # hooks on the stack lane's ``function_entered`` flag; doing so
            # would silently lose the Object generations needed for the later
            # target-boundary join.  Once the stack lane has closed the target
            # epoch, subsequent frontend hooks belong to another function.
            if self.target_complete:
                return False
            if self.frontend_session is None:
                return False
            if role in {"generic_completed_insertion", "bulk_object_link"}:
                # These hooks are sampled after their first instruction has
                # executed; the dispatcher completes them from on_hook_post.
                return True
            raise Rejected(f"unsupported frontend hook role: {role}")
        if self.target_complete or not self.function_entered:
            return False
        if role == "numeric_stack_alloc_pre":
            if any(event.get("event_kind") == "numeric_stack_alloc_pre" for event in self.bus.events):
                return False
            self.stack_allocation_pre_seen = True
            self._capture_inventory(force=True)
            self.bus.emit("stack", "numeric_stack_alloc_pre", {"hook_id": row["id"], "locals": self.ledger.tokens("local"), "arguments": self.ledger.tokens("argument")})
            return True
        if role == "numeric_stack_alloc_post":
            if any(event.get("event_kind") == "numeric_stack_alloc_post" for event in self.bus.events):
                return False
            self._capture_inventory(force=True)
            self.bus.emit("stack", "numeric_stack_alloc_post", {"hook_id": row["id"], "locals": self.ledger.tokens("local"), "arguments": self.ledger.tokens("argument")})
            return True
        if role == "object_stack_write":
            payload = self._backend_write_payload(row, thread)
            self.pending_writes[thread] = payload
            self.bus.emit("stack", "object_stack_write_pre", payload)
            return True
        if role == "machine_emit":
            payload = self._capture_machine_emission(row, thread)
            self.bus.emit("pcode", "machine_emission", payload)
            return True
        if role == "pcode_color_diagnostic":
            payload = self._capture_pcode_color(row, thread)
            if payload is not None:
                self.bus.emit("pcode", "pcode_capture", payload)
            return True
        if row.get("lane") == "pcode" and role not in {"regalloc", "regalloc_post"}:
            method = getattr(self.backend, "capture_pcode", None)
            raw = method(row["id"], thread) if callable(method) else {"hook_id": row["id"], "status": "UNKNOWN"}
            if not isinstance(raw, Mapping):
                raise Rejected("PCode backend returned a non-object")
            clean = {"hook_id": row["id"], **{str(key): value for key, value in raw.items() if key not in {"pointer", "address", "thread_id"}}}
            if clean.get("status") not in {"CAPTURED", "UNKNOWN"}:
                clean["status"] = "UNKNOWN"
                clean["reason"] = "incomplete PCode evidence"
            _pointer_free(clean)
            if clean.get("status") == "UNKNOWN":
                self._unknown("incomplete PCode evidence")
            self.bus.emit("pcode", "pcode_capture", clean)
            return True
        if role == "regalloc":
            method = getattr(self.backend, "capture_regalloc", None)
            raw = method(row["id"], thread) if callable(method) else []
            if not isinstance(raw, Sequence) or isinstance(raw, (str, bytes, bytearray)):
                raw = [raw]
            for item in raw:
                if isinstance(item, Mapping):
                    self.on_regalloc(item)
                else:
                    self._unknown("missing or invalid vreg identity")
                    self.bus.emit("pcode", "regalloc_assignment", {"status": "UNKNOWN", "reason": "missing or invalid vreg identity"})
            if not raw and self.auth["request"]["compiler"]["sha256"] == GC27_COMPILER_SHA256:
                # GC/2.7's authenticated 0x43598B site is chronology-only.  It
                # does not expose a canonical Object-to-vreg identity; direct
                # operand/IG evidence is captured at 0x5086C4/0x5086C8 instead.
                self._maybe_complete_target()
                return True
            if not raw:
                self._unknown("incomplete regalloc")
                self.bus.emit("pcode", "regalloc_assignment", {"status": "UNKNOWN", "reason": "incomplete regalloc"})
            self._maybe_complete_target()
            return True
        if role == "regalloc_post":
            # The verified epilogue may run before numeric_stack_alloc_pre.
            # Snapshot the Object lists first so the backend can bind EBX to
            # its VarInfo when possible; if the lists are not ready yet,
            # on_physical_regalloc retains the raw row internally and the
            # next forced stack snapshot resolves it.
            self._capture_inventory(force=True)
            method = getattr(self.backend, "capture_physical_regalloc", None)
            if not callable(method):
                method = getattr(self.backend, "capture_regalloc_post", None)
            try:
                raw = method(row["id"], thread) if callable(method) else None
            except Rejected as exc:
                message = str(exc).lower()
                if "object" in message and ("uncaptured" in message or "null" in message):
                    reason = "null object identity"
                elif "truncated" in message:
                    reason = "incomplete physical register evidence"
                elif "varinfo" in message:
                    reason = "missing or invalid Object VarInfo pointer"
                else:
                    reason = "incomplete physical register evidence"
                self._physical_unknown(None, reason)
                self.bus.emit(
                    "pcode",
                    "physical_reg_assignment",
                    {"status": "UNKNOWN", "reason": reason},
                )
                self._maybe_complete_target()
                return True
            if isinstance(raw, Mapping):
                self.on_physical_regalloc(raw)
            elif isinstance(raw, Sequence) and not isinstance(raw, (str, bytes, bytearray)):
                if not raw:
                    self._physical_unknown(None, "incomplete physical register evidence")
                    self.bus.emit(
                        "pcode",
                        "physical_reg_assignment",
                        {"status": "UNKNOWN", "reason": "incomplete physical register evidence"},
                    )
                else:
                    for item in raw:
                        self.on_physical_regalloc(item if isinstance(item, Mapping) else {})
            else:
                self._physical_unknown(None, "incomplete physical register evidence")
                self.bus.emit(
                    "pcode",
                    "physical_reg_assignment",
                    {"status": "UNKNOWN", "reason": "incomplete physical register evidence"},
                )
            self._maybe_complete_target()
            return True
        raise Rejected(f"unsupported hook role: {role}")

    def _ensure_lane_completion(self) -> None:
        """Emit explicit UNKNOWN edges for lanes that ended incomplete."""

        # Any deferred rows that never obtained a complete Object-list join
        # must still become explicit physical UNKNOWN events before the lane
        # summary is computed.
        self._flush_pending_physical_rows(force_unknown=True)
        events = self.bus.events
        stack_kinds = {event["event_kind"] for event in events if event["lane"] == "stack"}
        pcode_kinds = {event["event_kind"] for event in events if event["lane"] == "pcode"}
        stack_hook_ids = {
            str(event.get("hook_id"))
            for event in events
            if event["lane"] == "stack" and "hook_id" in event
        }
        pcode_hook_ids = {
            str(event.get("hook_id"))
            for event in events
            if event["lane"] == "pcode" and "hook_id" in event
        }
        profile_hooks = tuple(self.auth["hooks"])
        expected_stack = {
            "function_filter", "allocation_pre", "allocation_post",
        }
        expected_pcode = set(_pcode_stage_hook_ids(profile_hooks))
        missing_stack = expected_stack - stack_hook_ids
        missing_pcode = expected_pcode - pcode_hook_ids
        gc27_profile = (
            str(self.auth["request"]["compiler"]["sha256"]).lower()
            == GC27_COMPILER_SHA256
        )
        # GC/2.7's retained ``regalloc`` site is chronology-only.  The direct
        # Object-to-IG/vreg claim must come from separately authenticated
        # evidence; its absence is therefore not a missing lane edge.  This
        # mirrors ``_validate_chronology`` and, critically, does not synthesize
        # a virtual-register assignment from the physical result.
        if "regalloc_assignment" not in pcode_kinds and not gc27_profile:
            missing_pcode.add("regalloc")
        if "physical_reg_assignment" not in pcode_kinds:
            missing_pcode.add("regalloc_post")
        if any(row.get("role") == "machine_emit" for row in profile_hooks) and "machine_emission" not in pcode_kinds:
            missing_pcode.add("gc27_machine_emit")
        if missing_stack and "lane_unknown" not in stack_kinds:
            reason = "incomplete stack evidence"
            self._unknown(reason)
            self.bus.emit("stack", "lane_unknown", {"reason": reason})
        if missing_pcode and "lane_unknown" not in pcode_kinds:
            reason = "incomplete PCode evidence"
            self._unknown(reason)
            self.bus.emit("pcode", "lane_unknown", {"reason": reason})

    def on_hook_post(self, row: Mapping[str, Any], thread: int) -> None:
        if row.get("lane") == "frontend":
            try:
                self._frontend_complete_hook(row, thread)
            except Exception:
                self._frontend_unknown()
            return
        payload = self.pending_writes.pop(thread, None)
        if payload is None or payload["hook_id"] != row["id"]:
            raise Rejected("single-step write chronology mismatch")
        self.bus.emit("stack", "object_stack_write_post", {**payload, "write_observed": True})
        self._maybe_complete_target()

    def _finalize_frontend(self) -> None:
        session = self.frontend_session
        if session is None:
            if self.frontend_failure is not None:
                self._unknown(self.frontend_failure)
            return
        try:
            packet = session.on_process_exit(0)
            validated = _frontend_chronology.validate_packet(packet)
            self.frontend_packet = dict(validated)
        except Exception:
            self._frontend_unknown()

    def on_single_step(self, thread_id: Any, process_id: Any | None = None) -> None:
        self._check_process(process_id)
        self.dispatcher.on_single_step(thread_id)

    def on_breakpoint(self, address: Any, thread_id: Any, process_id: Any | None = None) -> None:
        self._check_process(process_id)
        self.dispatcher.on_breakpoint(address, thread_id)

    def on_process_exit(self, exit_code: Any = 0, process_id: Any | None = None) -> None:
        self._check_process(process_id)
        if not self.started:
            raise Rejected("process exited before start")
        if self.exited:
            raise Rejected("native process exited twice")
        self.exit_code = _integer(exit_code, "compiler exit code")
        self.exited = True
        if self.exit_code != 0:
            raise Rejected(f"compiler process exited with code {self.exit_code}")
        if not self.function_entered:
            raise Rejected("process exited without target function entry")
        if self.pending_writes or self.dispatcher.pending_steps:
            raise Rejected("process exited with pending single-step")
        self._finalize_frontend()
        self._ensure_lane_completion()
        self.function_exited = True
        self.bus.emit("stack", "function_exit", {"exit_code": self.exit_code})

    def on_disconnect(self, reason: Any) -> None:
        raise Rejected(f"native debug transport disconnected: {_text(reason, 'disconnect reason')}")

    def _exact_stack_home(self, token: str) -> dict[str, Any] | None:
        """Return one authenticated final-epoch stack home for ``token``.

        A VarInfo home write authenticates an offset, not an aggregate width.
        Width and lifetime remain a source-aware causal-map concern.
        """

        allocation_posts = [
            event
            for event in self.bus.events
            if event["event_kind"] == "numeric_stack_alloc_post"
        ]
        if len(allocation_posts) != 1:
            return None
        boundary = int(allocation_posts[0]["sequence"])
        pending: dict[tuple[str, str, int], list[Mapping[str, Any]]] = {}
        pairs: list[tuple[Mapping[str, Any], Mapping[str, Any]]] = []
        for event in self.bus.events:
            if int(event["sequence"]) <= boundary:
                continue
            kind = event["event_kind"]
            if kind not in {"object_stack_write_pre", "object_stack_write_post"}:
                continue
            if event.get("object_token") != token:
                continue
            key = (
                str(event["hook_id"]),
                str(event["object_token"]),
                int(event["target_slot"]),
            )
            if kind == "object_stack_write_pre":
                pending.setdefault(key, []).append(event)
                continue
            before_rows = pending.get(key, [])
            if not before_rows:
                return None
            before = before_rows.pop(0)
            pairs.append((before, event))
        if any(rows for rows in pending.values()) or not pairs:
            return None
        slots = {int(after["target_slot"]) for _before, after in pairs}
        if len(slots) != 1:
            return None
        evidence_ids = [
            str(event["event_id"])
            for before, after in pairs
            for event in (before, after)
        ]
        return {
            "status": "EXACT",
            "mode": "stack_home",
            "evidence_event_ids": evidence_ids,
            "stack_home": {"base": "r1", "offset": slots.pop()},
        }

    def _exact_event_ids(self, event_kind: str, token: str) -> list[str]:
        return [
            str(event["event_id"])
            for event in self.bus.events
            if event["event_kind"] == event_kind
            and event.get("status") == "EXACT"
            and event.get("object_token") == token
        ]

    def _resolved_ownership(self, token: str) -> dict[str, Any] | None:
        virtual = self.mappings.get(token)
        if isinstance(virtual, Mapping) and virtual.get("status") == "EXACT":
            # Preserve the historical schema byte-for-byte.  The same-session
            # physical observation remains separately validated chronology.
            return dict(virtual)
        physical = self.physical_mappings.get(token)
        if isinstance(physical, Mapping) and physical.get("status") == "EXACT":
            evidence_ids = self._exact_event_ids("physical_reg_assignment", token)
            if len(evidence_ids) != 1:
                return None
            return {
                "status": "EXACT",
                "mode": "physical_register",
                "evidence_event_ids": evidence_ids,
                "physical_reg": int(physical["physical_reg"]),
                "bank": str(physical["bank"]),
            }
        return self._exact_stack_home(token)

    def _finalize_inventory(self) -> None:
        self._capture_inventory()
        for token in self.ledger.tokens("local") + self.ledger.tokens("argument"):
            if self._resolved_ownership(token) is None:
                self.ledger.mark_unknown(token, "incomplete regalloc")
                self._unknown("incomplete regalloc")
        if not self.inventory_complete:
            for reason in sorted(self.inventory_snapshot_reasons):
                self._unknown(reason)
            self._unknown("incomplete inventory")

    def _inventory(self) -> dict[str, Any]:
        self._finalize_inventory()
        rows: dict[str, list[dict[str, Any]]] = {"locals": [], "arguments": []}
        complete = self.inventory_complete
        for key, kind in (("locals", "local"), ("arguments", "argument")):
            for raw in self.inventory_rows[key]:
                token = raw["token"]
                row = dict(raw)
                mapping = self._resolved_ownership(token)
                if mapping is None:
                    complete = False
                    row["ownership"] = {"status": "UNKNOWN", "reason": self.ledger._by_token.get(token, {}).get("reason", "incomplete regalloc")}
                else:
                    row["ownership"] = dict(mapping)
                rows[key].append(row)
        return {"status": "COMPLETE" if complete else "UNKNOWN", **rows}

    def run(self) -> dict[str, Any]:
        result: dict[str, Any] | None = None
        failure: Rejected | None = None
        try:
            # A wrapper-first launch (sjiswrap.exe -> mwcceppc.exe) is debugged
            # with DEBUG_PROCESS.  The native backend must consume and
            # authenticate the wrapper/child CREATE_PROCESS events before this
            # preflight reads a single hook byte; fake backends do not expose
            # this optional preparation boundary.
            prepare = getattr(self.backend, "prepare_capture", None)
            if callable(prepare):
                prepare(self)
            self.dispatcher.preflight()
            self.dispatcher.install()
            run = getattr(self.backend, "run", None)
            if not callable(run):
                raise Rejected("backend lacks shared debug loop")
            run(self)
            if not self.exited:
                raise Rejected("debug loop ended without process exit")
            if not self.function_exited:
                raise Rejected("capture ended without function exit")
            result = self.build_envelope()
        except Rejected as exc:
            failure = exc
        except Exception as exc:
            failure = Rejected(f"native transport failure: {type(exc).__name__}: {exc}")
        # The dispatcher owns the union's restoration boundary. The backend's
        # close() then closes handles/process state; no partial breakpoint
        # mutation survives a failed session. Restoration is secondary and
        # must not replace the first compiler/capture failure.
        try:
            self.dispatcher.remove_all()
        except Exception as exc:
            cleanup = f"breakpoint cleanup failure: {type(exc).__name__}: {exc}"
            failure = _combine_terminal_failure(failure, cleanup)
        if failure is not None:
            raise failure
        if result is None:
            raise Rejected("capture session returned no terminal result")
        return result

    def build_envelope(self) -> dict[str, Any]:
        if not self.started or self.bus.process_id is None:
            raise Rejected("capture has no actual native PID")
        if not self.exited or not self.function_exited:
            raise Rejected("capture is incomplete")
        self._finalize_inventory()
        events = list(self.bus.events)
        lanes: dict[str, dict[str, Any]] = {}
        for lane in _LANES:
            lane_events = [event for event in events if event["lane"] == lane]
            lane_bytes = canonical_lane_bytes(lane_events)
            lanes[lane] = {
                "event_count": len(lane_events),
                "event_ids": [event["event_id"] for event in lane_events],
                "size_bytes": len(lane_bytes),
                "sha256": hashlib.sha256(lane_bytes).hexdigest(),
            }
        request = self.auth["request"]
        request_path = Path(self.auth["request_path"])
        request_size = request_path.stat().st_size if request_path.exists() else 0
        context = {
            "session_id": self.session_id,
            "process_id": self.bus.process_id,
            "function": request["function"],
            "function_sha256": request["function_sha256"],
            "argv": list(request["argv"]),
            "cwd": request["cwd"],
            "source": dict(request["source"]),
            "compiler": dict(request["compiler"]),
            "wrapper": dict(request["wrapper"]),
            "debugger": dict(request["debugger"]),
            "transport": dict(request["transport"]),
            "request": {"path": str(request_path), "size": request_size, "sha256": self.auth["request_sha256"]},
            "authority": dict(request["authority"]),
        }
        is_gc26 = str(request["compiler"]["sha256"]).lower() == GC26_COMPILER_SHA256
        if is_gc26:
            context["frontend_trace_sha256"] = self.auth["request_sha256"]
        transport_provenance = getattr(self.backend, "transport_provenance", None)
        if callable(transport_provenance):
            execution = transport_provenance(request["argv"])
            if not isinstance(execution, Mapping):
                raise Rejected("native transport execution provenance is malformed")
            context["execution"] = dict(execution)
        outputs = {
            key: {
                "path": str(self.auth["paths"][key]),
                "size": len(canonical_lane_bytes([event for event in events if event["lane"] == lane])) if key != "envelope" else 0,
                "sha256": hashlib.sha256(canonical_lane_bytes([event for event in events if event["lane"] == lane])).hexdigest() if key != "envelope" else "0" * 64,
            }
            for key, lane in (("event_stream_stack", "stack"), ("event_stream_pcode", "pcode"))
        }
        envelope: dict[str, Any] = {
            "schema": SCHEMA,
            "tool_version": TOOL_VERSION,
            "status": "CAPTURED_UNKNOWN_OWNERSHIP",
            "diagnostic_only": DIAGNOSTIC_ONLY,
            "board_admission": BOARD_ADMISSION,
            "exactness_claim": EXACTNESS_CLAIM,
            "authority_advanced": AUTHORITY_ADVANCED,
            "context": context,
            "authority": dict(request["authority"]),
            "outputs": outputs,
            "hooks": [dict(row) for row in self.auth["hooks"]],
            "events": events,
            "event_count": len(events),
            "lanes": lanes,
            "inventory": self._inventory(),
            "unknown": sorted(set(self.unknown)),
            "limitations": [
                "One native compiler process and one shared breakpoint dispatcher are used.",
                "Raw pointers, IG-node identities, thread handles, and addresses are capture-internal only.",
                "Ownership is EXACT only for direct, unique same-session object-to-vreg evidence.",
                "Missing, duplicate, reused, null, or one-to-many evidence is UNKNOWN.",
            ],
        }
        if is_gc26:
            if self.frontend_packet is not None:
                envelope["frontend_chronology"] = {
                    "status": "CAPTURED",
                    "packet": dict(self.frontend_packet),
                }
            else:
                envelope["frontend_chronology"] = {
                    "status": "UNKNOWN",
                    "reason": self.frontend_failure or "incomplete frontend chronology",
                }
        _pointer_free(envelope)
        envelope["envelope_sha256"] = canonical_hash(envelope)
        return envelope


# Compatibility spelling for callers that use the donor's shorter name.
CaptureSession = CombinedCaptureSession


def _normalize_auth(auth: Mapping[str, Any]) -> dict[str, Any]:
    if not isinstance(auth, Mapping):
        raise Rejected("authenticated request context is missing")
    request = auth.get("request")
    if not isinstance(request, Mapping):
        raise Rejected("request context is missing")
    function = _text(request.get("function"), "request.function")
    function_sha = _text(request.get("function_sha256"), "request.function_sha256").lower()
    if not SHA256_RE.fullmatch(function_sha):
        raise Rejected("request.function_sha256 is malformed")
    argv = _canonical_argv(request.get("argv"))
    cwd_value = request.get("cwd")
    cwd = _canonical_cwd(cwd_value, must_exist=False)
    artifacts = auth.get("artifacts")
    if not isinstance(artifacts, Mapping):
        artifacts = {}
    source_value = request.get("source", artifacts.get("source"))
    compiler_value = request.get("compiler", artifacts.get("compiler"))
    authority_value = request.get("authority", artifacts.get("authority", artifacts.get("producer")))
    source = _path_descriptor(source_value, "request.source", must_exist=False)
    if not _authorized_board_function(function, source):
        raise Rejected("unsupported target function")
    compiler = _path_descriptor(compiler_value, "request.compiler", must_exist=False)
    hooks = _validate_hook_rows(
        auth.get("hooks", [dict(row) for row in _hooks_for_compiler(str(compiler["sha256"]))]),
        "authenticated hooks",
        compiler_sha256=str(compiler["sha256"]),
    )
    authority = _path_descriptor(authority_value, "request.authority", must_exist=False)
    normalized_tools: dict[str, dict[str, Any]] = {}
    explicit_tool_keys: set[str] = set()
    for key in _TOOL_IDENTITY_KEYS:
        value = request.get(key, artifacts.get(key))
        if value is None:
            # Programmatic fake backends predate the explicit tool inventory.
            # They remain useful for chronology tests, but disk-backed request
            # authentication below never accepts this compatibility fallback.
            value = authority
        else:
            explicit_tool_keys.add(key)
        normalized_tools[key] = _path_descriptor(value, f"request.{key}", must_exist=False)
    raw_session = request.get("session_id", auth.get("session_id"))
    if raw_session is None:
        seed = {
            "function": function,
            "argv": argv,
            "cwd": cwd,
            "source": source,
            "compiler": compiler,
            **normalized_tools,
            "authority": authority,
        }
        raw_session = f"session-{canonical_hash(seed)[:16]}"
    session_id = _safe_session_id(raw_session)
    normalized_request = {
        "function": function,
        "function_sha256": function_sha,
        "argv": argv,
        "cwd": cwd,
        "source": source,
        "compiler": compiler,
        **normalized_tools,
        "authority": authority,
        "session_id": session_id,
    }
    if explicit_tool_keys:
        _validate_compile_argv(
            argv,
            cwd=cwd,
            source=source,
            compiler=compiler,
            wrapper=normalized_tools["wrapper"] if "wrapper" in explicit_tool_keys else None,
        )
    request_path = auth.get("request_path")
    if request_path is None:
        request_path = Path(str(auth.get("request", {}).get("path", "request.json")))
    request_path = Path(request_path)
    request_sha = auth.get("request_sha256", auth.get("request", {}).get("sha256", ""))
    request_sha = _text(request_sha, "request_sha256").lower()
    if not SHA256_RE.fullmatch(request_sha):
        # Direct fake-auth tests may not have an on-disk request.  Preserve a
        # deterministic trusted fixture hash while still requiring a digest.
        request_sha = canonical_hash(normalized_request)
    paths_raw = auth.get("paths", {})
    paths: dict[str, Path] = {}
    if isinstance(paths_raw, Mapping):
        for key in _REQUEST_PATH_KEYS:
            if key in paths_raw:
                paths[key] = Path(paths_raw[key])
    if not paths:
        output_dir = request_path.parent
        paths = {
            "event_stream_stack": output_dir / "stack.events.jsonl",
            "event_stream_pcode": output_dir / "pcode.events.jsonl",
            "envelope": output_dir / "same-session.envelope.json",
        }
    return {
        "request": normalized_request,
        "request_path": request_path,
        "request_sha256": request_sha,
        "paths": paths,
        "output_dir": request_path.parent,
        "explicit_tools": explicit_tool_keys,
        "hooks": hooks,
    }


def prepare_request(
    manifest: Path | Mapping[str, Any],
    output_dir: Path,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
) -> Path:
    """Create one immutable request and prelaunch session id.

    The manifest is an input authority descriptor.  It is read and hashed;
    this function never writes or rewrites it.
    """

    if external_trust_root is not None and trust_root is not None:
        raise Rejected("conflicting external trust root arguments")
    root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
    if isinstance(manifest, Mapping):
        raw = dict(manifest)
    else:
        manifest_path = _canonical_path(manifest, "manifest")
        try:
            raw = strict_json_loads(manifest_path.read_text(encoding="utf-8"), "manifest")
        except OSError as exc:
            raise Rejected(f"cannot read manifest: {exc}") from exc
    if not isinstance(raw, Mapping):
        raise Rejected("manifest must be an object")
    function = _text(raw.get("function"), "manifest.function")
    source = _descriptor(raw.get("source"), "manifest.source")
    if not _authorized_board_function(function, source):
        raise Rejected("unsupported target function")
    compiler = _descriptor(raw.get("compiler"), "manifest.compiler")
    tools: dict[str, dict[str, Any]] = {}
    for key in _TOOL_IDENTITY_KEYS:
        raw_value = raw.get(key)
        if raw_value is None and key == "transport":
            raw_value = raw.get("debug_transport")
        if raw_value is None:
            raise Rejected(f"manifest.{key} identity is missing")
        tools[key] = _descriptor(raw_value, f"manifest.{key}")
    authority = _descriptor(raw.get("authority"), "manifest.authority")
    function_sha = _text(raw.get("function_sha256"), "manifest.function_sha256").lower()
    if not SHA256_RE.fullmatch(function_sha):
        raise Rejected("manifest.function_sha256 is malformed")
    argv = _canonical_argv(raw.get("argv"))
    cwd = _canonical_cwd(raw.get("cwd"), must_exist=True)
    _validate_compile_argv(
        argv,
        cwd=cwd,
        source=source,
        compiler=compiler,
        wrapper=tools["wrapper"],
        require_include_paths=True,
    )
    session_id = _safe_session_id(raw.get("session_id", _new_session_id()))
    expected_hooks = _hooks_for_compiler(str(compiler["sha256"]))
    hooks = _validate_hook_rows(
        raw.get("hooks", [dict(row) for row in expected_hooks]),
        "manifest.hooks",
        compiler_sha256=str(compiler["sha256"]),
    )
    output_dir = _canonical_path(output_dir, "output_dir", directory=True, must_exist=False)
    output_dir.mkdir(parents=True, exist_ok=True)
    paths = {
        "event_stream_stack": output_dir / "stack.events.jsonl",
        "event_stream_pcode": output_dir / "pcode.events.jsonl",
        "envelope": output_dir / "same-session.envelope.json",
    }
    request_path = output_dir / "request.json"
    if any(output_dir.iterdir()):
        raise Rejected("capture output directory is not empty")
    request = {
        "schema": REQUEST_SCHEMA,
        "tool_version": TOOL_VERSION,
        "diagnostic_only": DIAGNOSTIC_ONLY,
        "board_admission": BOARD_ADMISSION,
        "exactness_claim": EXACTNESS_CLAIM,
        "session_id": session_id,
        "function": function,
        "function_sha256": function_sha,
        "argv": argv,
        "cwd": cwd,
        "source": source,
        "compiler": compiler,
        **tools,
        "authority": authority,
        "hooks": hooks,
        "output_dir": str(output_dir),
        "paths": {key: str(path) for key, path in paths.items()},
    }
    request["request_sha256"] = canonical_hash(request)
    request_bytes = (json.dumps(request, ensure_ascii=True, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")
    if root is not None:
        # A prelaunch root may omit request/output anchors because the request
        # bytes do not exist yet.  All artifact and compile anchors still bind
        # before the request is written.
        _validate_external_root_against_request(root, request, request_path=None, allow_outputs=True)
    write_new(request_path, request_bytes)
    return request_path


def _request_paths(request: Mapping[str, Any]) -> dict[str, Path]:
    paths = request.get("paths")
    if not isinstance(paths, Mapping) or set(paths) != _REQUEST_PATH_KEYS:
        raise Rejected("request paths are incomplete")
    result: dict[str, Path] = {}
    for key in _REQUEST_PATH_KEYS:
        result[key] = _canonical_path(paths[key], f"request.paths.{key}", must_exist=False)
    output_dir = _canonical_path(request.get("output_dir"), "request.output_dir", directory=True, must_exist=False)
    if str(output_dir) != str(Path(next(iter(result.values()))).parent):
        raise Rejected("request output paths do not share output_dir")
    if any(path.parent != output_dir for path in result.values()):
        raise Rejected("request output path escapes output_dir")
    return result


def _compiler_output_paths(request: Mapping[str, Any]) -> tuple[Path, ...]:
    """Derive only explicit compiler-owned ``-o`` paths from bound argv."""

    argv = _canonical_argv(request.get("argv"), "request.argv")
    cwd = Path(_canonical_cwd(request.get("cwd"), must_exist=True))
    outputs: list[Path] = []
    for index, value in enumerate(argv):
        operand: str | None = None
        if value == "-o":
            if index + 1 >= len(argv) or argv[index + 1].startswith("-"):
                raise Rejected("compiler -o operand is missing")
            operand = argv[index + 1]
        if operand is None:
            continue
        path = Path(operand)
        if not path.is_absolute():
            path = cwd / path
        outputs.append(_canonical_path(path, "compiler -o output", must_exist=False))
    if len(outputs) > 1:
        raise Rejected("compiler argv contains multiple -o outputs")
    return tuple(outputs)


def _compiler_transport_paths(auth: Mapping[str, Any]) -> dict[str, Path]:
    """Derive an isolated, capture-owned diagnostics boundary."""

    request_path = _canonical_path(auth.get("request_path"), "authenticated request path")
    request = auth.get("request")
    if not isinstance(request, Mapping):
        raise Rejected("authenticated request metadata is missing")
    session_id = _safe_session_id(request.get("session_id"))
    output_dir = _canonical_path(auth.get("output_dir"), "authenticated output directory", directory=True)
    if request_path.parent != output_dir:
        raise Rejected("authenticated request/output directory binding changed")
    compiler_outputs = tuple(Path(path) for path in auth.get("compiler_output_paths", ()))
    if len(compiler_outputs) != 1:
        raise Rejected("native compiler receipt requires exactly one -o output")
    diagnostic_dir = output_dir.with_name(f"{output_dir.name}.compiler-transport-{session_id}")
    diagnostic_dir = _canonical_path(
        diagnostic_dir,
        "compiler transport diagnostic directory",
        directory=True,
        must_exist=False,
    )
    if diagnostic_dir.exists() or diagnostic_dir.is_symlink():
        raise Rejected("compiler transport diagnostic directory already exists")
    paths = {
        "directory": diagnostic_dir,
        "stdout": diagnostic_dir / "compiler.stdout.bin",
        "stderr": diagnostic_dir / "compiler.stderr.bin",
        "receipt": diagnostic_dir / "compiler-to-object.json",
    }
    if any(path.exists() or path.is_symlink() for key, path in paths.items() if key != "directory"):
        raise Rejected("compiler transport diagnostic output already exists")
    return paths


def _optional_output_descriptor(path: Path) -> dict[str, Any]:
    if path.is_symlink():
        raise Rejected("compiler output is a symlink")
    if not path.exists():
        return {"path": str(path), "exists": False, "size": None, "sha256": None}
    if not path.is_file():
        raise Rejected("compiler output is not a regular file")
    return {
        "path": str(path),
        "exists": True,
        "size": path.stat().st_size,
        "sha256": sha256(path),
    }


def _compiler_object_descriptor(path: Path) -> dict[str, Any]:
    """Describe and minimally authenticate a complete ELF32/PPC relocatable."""

    descriptor = _optional_output_descriptor(path)
    descriptor["format"] = None
    descriptor["complete"] = False
    descriptor["validation_failure"] = None
    if descriptor["exists"] is not True:
        descriptor["validation_failure"] = "compiler object is absent"
        return descriptor
    try:
        data = path.read_bytes()
        if len(data) < 52:
            raise Rejected("compiler object is shorter than an ELF32 header")
        if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 2:
            raise Rejected("compiler object is not big-endian ELF32")
        e_type = int.from_bytes(data[16:18], "big")
        e_machine = int.from_bytes(data[18:20], "big")
        e_shoff = int.from_bytes(data[32:36], "big")
        e_ehsize = int.from_bytes(data[40:42], "big")
        e_shentsize = int.from_bytes(data[46:48], "big")
        e_shnum = int.from_bytes(data[48:50], "big")
        if e_type != 1 or e_machine != 20 or e_ehsize != 52:
            raise Rejected("compiler object ELF identity is not PPC relocatable")
        if e_shoff < e_ehsize or e_shentsize != 40 or e_shnum <= 0:
            raise Rejected("compiler object section table is incomplete")
        if e_shoff + e_shentsize * e_shnum > len(data):
            raise Rejected("compiler object section table exceeds the emitted file")
    except Exception as exc:
        descriptor["validation_failure"] = f"{type(exc).__name__}: {exc}"
        return descriptor
    descriptor["format"] = "ELF32_PPC_RELOCATABLE"
    descriptor["complete"] = True
    return descriptor


def _compiler_environment_binding(environment: Mapping[str, str] | None = None) -> dict[str, Any]:
    """Bind the inherited process environment without publishing its values."""

    values = os.environ if environment is None else environment
    rows: list[list[str]] = []
    for key, value in values.items():
        if not isinstance(key, str) or not isinstance(value, str) or not key:
            raise Rejected("compiler environment contains a malformed entry")
        rows.append([key.upper(), value])
    rows.sort(key=lambda row: (row[0], row[1]))
    if len({row[0] for row in rows}) != len(rows):
        raise Rejected("compiler environment contains case-ambiguous keys")
    return {
        "mode": "inherited",
        "variable_count": len(rows),
        "variable_names": [row[0] for row in rows],
        "sha256": canonical_hash(rows),
    }


def _sealed_compiler_environment(
    environment: Mapping[str, str] | None = None,
) -> tuple[dict[str, Any], Any]:
    """Snapshot one immutable Unicode environment block and its safe binding."""

    values = dict(os.environ if environment is None else environment)
    binding = _compiler_environment_binding(values)
    ordered = sorted(values.items(), key=lambda row: row[0].upper())
    block = "\0".join(f"{key}={value}" for key, value in ordered) + "\0\0"
    return binding, ctypes.create_unicode_buffer(block)


def _combine_terminal_failure(
    failure: Exception | None, secondary: str
) -> Rejected:
    secondary = _text(secondary, "secondary terminal failure")
    if failure is None:
        result = Rejected(secondary)
        result.primary_failure = secondary
        result.secondary_failures = []
        return result
    primary = str(getattr(failure, "primary_failure", str(failure)))
    existing = list(getattr(failure, "secondary_failures", ()))
    # Finalization can be observed from more than one boundary (for example a
    # failed dispatcher restoration followed by an idempotent backend close).
    # Preserve every distinct failure once, without allowing a repeated close
    # observation to rewrite either the causal primary or secondary order.
    if secondary in existing:
        return failure if isinstance(failure, Rejected) else _structured_terminal_failure(
            primary, existing
        )
    all_secondary = [*existing, secondary]
    combined = _structured_terminal_failure(primary, all_secondary)
    return combined


def _structured_terminal_failure(primary: str, secondary: Sequence[str]) -> Rejected:
    message = "; ".join(
        [primary, *(f"secondary failure: {item}" for item in secondary)]
    )
    combined = Rejected(message)
    combined.primary_failure = primary
    combined.secondary_failures = list(secondary)
    return combined


def _close_capture_streams(
    streams: Sequence[Any], failure: Exception | None
) -> Exception | None:
    result = failure
    for stream in reversed(streams):
        try:
            stream.close()
        except OSError as exc:
            result = _combine_terminal_failure(
                result,
                f"compiler diagnostic stream close failure: {type(exc).__name__}: {exc}",
            )
    return result


def _capture_output_descriptors(
    auth: Mapping[str, Any], capture_result: Mapping[str, Any] | None
) -> tuple[bool, dict[str, Any], str | None]:
    """Return complete canonical capture outputs, or an unadmitted diagnostic set."""

    paths = auth.get("paths")
    if not isinstance(paths, Mapping):
        raise Rejected("compiler receipt lacks authenticated capture paths")
    descriptors: dict[str, Any] = {}
    for name in ("event_stream_stack", "event_stream_pcode", "envelope"):
        path = _canonical_path(paths.get(name), f"compiler receipt capture {name}", must_exist=False)
        descriptors[name] = _optional_output_descriptor(path)
    if not isinstance(capture_result, Mapping) or capture_result.get("schema") != SCHEMA:
        return False, descriptors, "capture result is missing or has the wrong schema"
    if not all(row["exists"] is True and int(row["size"] or 0) > 0 for row in descriptors.values()):
        return False, descriptors, "capture output set is missing or empty"
    try:
        envelope_path = Path(descriptors["envelope"]["path"])
        envelope = strict_json_loads(envelope_path.read_text(encoding="utf-8"), "compiler receipt envelope")
        if not isinstance(envelope, Mapping) or envelope.get("schema") != SCHEMA:
            raise Rejected("capture envelope schema mismatch")
        expected_hash = envelope.get("envelope_sha256")
        unsigned_envelope = {key: value for key, value in envelope.items() if key != "envelope_sha256"}
        if expected_hash != canonical_hash(unsigned_envelope):
            raise Rejected("capture envelope self-digest mismatch")
        if capture_result.get("envelope_sha256") != expected_hash:
            raise Rejected("capture result/envelope identity mismatch")
        context = envelope.get("context")
        request = auth.get("request")
        if not isinstance(context, Mapping) or not isinstance(request, Mapping):
            raise Rejected("capture envelope context is missing")
        if context.get("session_id") != request.get("session_id"):
            raise Rejected("capture envelope session mismatch")
        if context.get("function") != request.get("function"):
            raise Rejected("capture envelope function mismatch")
        if context.get("function_sha256") != request.get("function_sha256"):
            raise Rejected("capture envelope function hash mismatch")
        request_descriptor = context.get("request")
        if (
            not isinstance(request_descriptor, Mapping)
            or request_descriptor.get("sha256") != auth.get("request_sha256")
        ):
            raise Rejected("capture envelope request binding mismatch")
        envelope_outputs = envelope.get("outputs")
        if not isinstance(envelope_outputs, Mapping):
            raise Rejected("capture envelope output bindings are missing")
        for name in ("event_stream_stack", "event_stream_pcode"):
            live_binding = {
                key: descriptors[name][key] for key in ("path", "size", "sha256")
            }
            if envelope_outputs.get(name) != live_binding:
                raise Rejected(f"capture envelope {name} binding mismatch")

        root = auth.get("trust_root")
        if not isinstance(root, ExternalTrustRoot):
            raise Rejected("capture receipt lacks its external trust root")
        output_values = {field: getattr(root, field) for field in ExternalTrustRoot.FIELDS}
        for name in ("event_stream_stack", "event_stream_pcode", "envelope"):
            output_values[f"{name}_path"] = descriptors[name]["path"]
            output_values[f"{name}_size"] = descriptors[name]["size"]
            output_values[f"{name}_sha256"] = descriptors[name]["sha256"]
        validate_envelope(envelope_path, external_trust_root=ExternalTrustRoot(**output_values))
    except Exception as exc:
        return False, descriptors, f"{type(exc).__name__}: {exc}"
    return True, descriptors, None


def _compiler_execution_receipt(
    auth: Mapping[str, Any],
    *,
    transport_mode: str,
    executed_argv: Sequence[str],
    include_search_paths: Sequence[Mapping[str, Any]],
    stdout_path: Path,
    stderr_path: Path,
    process_created: bool,
    process_quiesced: bool,
    terminated_by_capture: bool,
    exit_code: int | None,
    failure: str | None,
    environment_binding: Mapping[str, Any] | None = None,
    capture_result: Mapping[str, Any] | None = None,
    process_summary: Mapping[str, Any] | None = None,
    secondary_failures: Sequence[str] = (),
) -> dict[str, Any]:
    """Seal the compiler invocation and its exact object/diagnostic outcome."""

    request = auth.get("request")
    if not isinstance(request, Mapping):
        raise Rejected("compiler receipt lacks authenticated request metadata")
    compiler_outputs = tuple(Path(path) for path in auth.get("compiler_output_paths", ()))
    if len(compiler_outputs) != 1:
        raise Rejected("compiler receipt requires exactly one -o output")
    argv = [_text(value, f"compiler receipt argv[{index}]") for index, value in enumerate(executed_argv)]
    live_include_paths = [
        _directory_tree_descriptor(Path(str(row.get("path", ""))))
        for row in include_search_paths
    ]
    if live_include_paths != [dict(row) for row in include_search_paths]:
        raise Rejected("compiler include tree changed during capture")
    if not stdout_path.is_file() or stdout_path.is_symlink():
        raise Rejected("compiler stdout capture is missing or not a regular file")
    if not stderr_path.is_file() or stderr_path.is_symlink():
        raise Rejected("compiler stderr capture is missing or not a regular file")
    if exit_code is not None:
        exit_code = _integer(exit_code, "compiler receipt exit code")
    output = _compiler_object_descriptor(compiler_outputs[0])
    capture_complete, capture_outputs, capture_validation_failure = _capture_output_descriptors(
        auth, capture_result
    )
    compiler_observed = (
        process_created
        and process_quiesced
        and not terminated_by_capture
        and exit_code == 0
        and output["complete"] is True
        and failure is None
    )
    terminal_success = compiler_observed and capture_complete
    environment = dict(environment_binding or _compiler_environment_binding())
    if set(environment) != {"mode", "variable_count", "variable_names", "sha256"}:
        raise Rejected("compiler environment binding is malformed")
    secondary = [_text(value, f"compiler receipt secondary failure[{index}]") for index, value in enumerate(secondary_failures)]
    if secondary != list(dict.fromkeys(secondary)):
        raise Rejected("compiler receipt secondary failures are noncanonical")
    process_tree = dict(process_summary or {
        "observed_process_ids": [],
        "exited_process_ids": [],
        "open_process_ids": [],
        "open_thread_handle_count": 0,
        "unclosed_handle_count": 0,
        "active_debug_event": False,
    })
    process_tree_complete = (
        process_tree.get("open_process_ids") == []
        and process_tree.get("open_thread_handle_count") == 0
        and process_tree.get("unclosed_handle_count", 0) == 0
        and process_tree.get("active_debug_event") is False
    )
    terminal_success = terminal_success and process_tree_complete
    terminal_state = "SUCCESS" if terminal_success else ("UNKNOWN" if process_created else "FAILED")
    if terminal_state not in COMPILER_TERMINAL_STATES:
        raise Rejected("compiler terminal state is invalid")
    partial_evidence_admitted = (
        isinstance(capture_result, Mapping)
        and capture_result.get("schema") == f"{PARTIAL_EVIDENCE_SCHEMA}/capture"
        and capture_result.get("status") == "UNKNOWN"
    )
    unsigned = {
        "schema": COMPILER_EXECUTION_RECEIPT_SCHEMA,
        "status": "COMPILER_TO_OBJECT_OBSERVED" if compiler_observed else "UNKNOWN",
        "terminal_state": terminal_state,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "session_id": _safe_session_id(request.get("session_id")),
        "function": _text(request.get("function"), "compiler receipt function"),
        "source_span": {
            "function": _text(request.get("function"), "compiler receipt function"),
            "function_sha256": _text(
                request.get("function_sha256"), "compiler receipt function hash"
            ).lower(),
        },
        "request": _descriptor(auth.get("request_path"), "compiler receipt request"),
        "source": _descriptor(request.get("source"), "compiler receipt source"),
        "compiler": _descriptor(request.get("compiler"), "compiler receipt compiler"),
        "transport_mode": _text(transport_mode, "compiler receipt transport mode"),
        "request_argv_sha256": canonical_hash(list(request.get("argv", ()))),
        "executed_argv": argv,
        "executed_argv_sha256": canonical_hash(argv),
        "include_search_paths": live_include_paths,
        "cwd": _canonical_cwd(request.get("cwd"), must_exist=True),
        "environment": environment,
        "prelaunch_empty_output_proof": dict(auth.get("prelaunch_empty_output_proof") or {}),
        "process_created": bool(process_created),
        "process_quiesced": bool(process_quiesced),
        "terminated_by_capture": bool(terminated_by_capture),
        "exit_code": exit_code,
        "compiler_output": output,
        "object_observation": (
            "ADMITTED_COMPLETE" if terminal_success else
            "PRESENT_UNADMITTED" if output["exists"] else
            "ABSENT"
        ),
        "stdout": _descriptor(stdout_path, "compiler receipt stdout"),
        "stderr": _descriptor(stderr_path, "compiler receipt stderr"),
        "diagnostics_sealed": bool(not process_created or process_quiesced),
        "capture_outputs": capture_outputs,
        "capture_validation_failure": capture_validation_failure,
        "evidence_admission": (
            "COMPLETE" if terminal_success else
            "PARTIAL_DIAGNOSTIC" if partial_evidence_admitted else
            "NONE"
        ),
        "partial_evidence_admitted": partial_evidence_admitted,
        "process_tree": process_tree,
        "primary_failure": failure,
        "secondary_failures": secondary,
        # Compatibility alias retained for existing receipt readers.
        "failure": failure,
    }
    return {**unsigned, "receipt_sha256": canonical_hash(unsigned)}


def _publish_compiler_execution_receipt(path: Path, receipt: Mapping[str, Any]) -> None:
    unsigned = {key: value for key, value in receipt.items() if key != "receipt_sha256"}
    if receipt.get("receipt_sha256") != canonical_hash(unsigned):
        raise Rejected("compiler execution receipt self-digest mismatch")
    if path.exists() or path.is_symlink():
        raise FileExistsError(path)
    staging = path.with_name(f".{path.name}.{receipt['receipt_sha256']}.tmp")
    if staging.exists() or staging.is_symlink():
        raise Rejected("compiler execution receipt staging path already exists")
    try:
        write_new(staging, _canonical_json_bytes(dict(receipt)))
        # Same-directory rename is the single publication point. On Windows it
        # also fails rather than replacing an existing immutable receipt.
        os.rename(staging, path)
    finally:
        try:
            if staging.exists() or staging.is_symlink():
                staging.unlink()
        except OSError:
            pass


def authenticate_request(
    request_path: Path | str,
    *,
    require_empty: bool = False,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    post_capture_analysis: bool = False,
) -> dict[str, Any]:
    if external_trust_root is not None and trust_root is not None:
        raise Rejected("conflicting external trust root arguments")
    root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
    if root is None:
        raise Rejected("external trust root is required for authenticated request")
    if post_capture_analysis and require_empty:
        raise Rejected("post-capture analysis cannot authenticate a prelaunch request")
    request_path = _canonical_path(request_path, "request")
    try:
        parsed = strict_json_loads(request_path.read_text(encoding="utf-8"), "request")
    except OSError as exc:
        raise Rejected(f"cannot read request: {exc}") from exc
    if not isinstance(parsed, Mapping) or set(parsed) != _REQUEST_KEYS:
        raise Rejected("request contains unsupported or missing fields")
    if parsed["schema"] != REQUEST_SCHEMA or parsed["tool_version"] != TOOL_VERSION:
        raise Rejected("request schema/tool version mismatch")
    unsigned = {key: value for key, value in parsed.items() if key != "request_sha256"}
    if parsed["request_sha256"] != canonical_hash(unsigned):
        raise Rejected("request self-digest mismatch")
    if parsed["diagnostic_only"] is not True or parsed["board_admission"] is not False or parsed["exactness_claim"] is not False:
        raise Rejected("request policy mismatch")
    function = _text(parsed["function"], "request.function")
    source = _descriptor(parsed["source"], "request.source")
    if not _authorized_board_function(function, source):
        raise Rejected("unsupported target function")
    compiler = _descriptor(parsed["compiler"], "request.compiler")
    _validate_runtime_hook_patch(str(compiler["sha256"]))
    hooks = _validate_hook_rows(
        parsed["hooks"],
        "request.hooks",
        compiler_sha256=str(compiler["sha256"]),
    )
    _validate_authenticated_compiler_hook_image(compiler, hooks)
    tools = {}
    for key in _TOOL_IDENTITY_KEYS:
        if post_capture_analysis and key in {"debugger", "transport"}:
            tools[key] = _path_descriptor(
                parsed[key],
                f"request.{key}",
                must_exist=False,
                verify_live=False,
            )
        else:
            tools[key] = _descriptor(parsed[key], f"request.{key}")
    authority = _descriptor(parsed["authority"], "request.authority")
    request = {
        "function": function,
        "function_sha256": _text(parsed["function_sha256"], "request.function_sha256").lower(),
        "argv": _canonical_argv(parsed["argv"]),
        "cwd": _canonical_cwd(parsed["cwd"], must_exist=True),
        "source": source,
        "compiler": compiler,
        **tools,
        "authority": authority,
        "session_id": _safe_session_id(parsed["session_id"]),
    }
    if not SHA256_RE.fullmatch(request["function_sha256"]):
        raise Rejected("request.function_sha256 is malformed")
    _validate_compile_argv(
        request["argv"],
        cwd=request["cwd"],
        source=source,
        compiler=compiler,
        wrapper=tools["wrapper"],
        require_include_paths=not post_capture_analysis,
    )
    paths = _request_paths(parsed)
    output_dir = _canonical_path(parsed["output_dir"], "request.output_dir", directory=True, must_exist=True)
    if request_path.parent != output_dir or request_path.name != "request.json":
        raise Rejected("request/output directory binding mismatch")
    compiler_output_paths = _compiler_output_paths(request)
    capture_outputs = set(paths.values())
    for compiler_output in compiler_output_paths:
        if compiler_output == request_path or compiler_output in capture_outputs:
            raise Rejected("compiler -o output collides with request or capture evidence")
    prelaunch_empty_output_proof: dict[str, Any] | None = None
    if require_empty:
        extras = [entry for entry in output_dir.iterdir() if entry.name != request_path.name]
        if extras:
            raise Rejected("capture output directory contains stale or partial files")
        if any(path.exists() or path.is_symlink() for path in compiler_output_paths):
            raise Rejected("compiler -o output existed before authenticated process launch")
        proof_unsigned = {
            "schema": "mwcc_prelaunch_empty_output_proof/v1",
            "output_dir": str(output_dir),
            "request_path": str(request_path),
            "request_sha256": sha256(request_path),
            "verified_entries": [request_path.name],
            "compiler_outputs_absent": [str(path) for path in compiler_output_paths],
        }
        prelaunch_empty_output_proof = {
            **proof_unsigned,
            "proof_sha256": canonical_hash(proof_unsigned),
        }
    _validate_external_root_against_request(
        root,
        request,
        request_path=request_path,
        allow_outputs=True,
        post_capture_analysis=post_capture_analysis,
    )
    return {
        "request": request,
        "request_path": request_path,
        "request_sha256": sha256(request_path),
        "paths": paths,
        "output_dir": output_dir,
        "compiler_output_paths": compiler_output_paths,
        "prelaunch_empty_output_proof": prelaunch_empty_output_proof,
        "hooks": hooks,
        "trust_root": root,
    }


def revalidate_request(auth: Mapping[str, Any]) -> None:
    path = Path(auth["request_path"])
    current = sha256(path)
    if current != auth["request_sha256"]:
        raise Rejected("request changed during capture")
    root = auth.get("trust_root")
    fresh = authenticate_request(path, external_trust_root=root)
    if fresh["request"] != auth["request"] or fresh["request_sha256"] != auth["request_sha256"]:
        raise Rejected("request identity changed during capture")


def _validate_post_launch_output_handoff(auth: Mapping[str, Any]) -> None:
    """Preserve the prelaunch empty proof across compiler process creation.

    The authenticated compiler may create its exact ``-o`` object before the
    debug backend enters ``capture_with_backend``.  No capture stream,
    envelope, unrelated file, symlink, or unowned compiler output is accepted
    across that handoff.
    """

    output_dir = Path(auth["output_dir"])
    request_path = Path(auth["request_path"])
    proof = auth.get("prelaunch_empty_output_proof")
    proof_unsigned = {
        "schema": "mwcc_prelaunch_empty_output_proof/v1",
        "output_dir": str(output_dir),
        "request_path": str(request_path),
        "request_sha256": auth.get("request_sha256"),
        "verified_entries": [request_path.name],
        "compiler_outputs_absent": [str(path) for path in auth.get("compiler_output_paths", ())],
    }
    if not isinstance(proof, Mapping) or dict(proof) != {
        **proof_unsigned,
        "proof_sha256": canonical_hash(proof_unsigned),
    }:
        raise Rejected("preauthenticated handoff lacks the prelaunch empty-output proof")
    capture_paths = {Path(path) for path in auth["paths"].values()}
    compiler_outputs = {Path(path) for path in auth.get("compiler_output_paths", ())}
    if capture_paths & compiler_outputs:
        raise Rejected("compiler -o output collides with capture evidence")
    extra_entries = [entry for entry in output_dir.iterdir() if entry.name != request_path.name]
    for entry in extra_entries:
        if entry.is_symlink() or not entry.is_file():
            raise Rejected("authenticated compiler output handoff is not a regular file")
    extras = {entry.resolve() for entry in extra_entries}
    unexpected = extras - compiler_outputs
    if unexpected:
        raise Rejected("capture output directory contains stale or partial files")
    for compiler_output in compiler_outputs:
        if compiler_output.exists() or compiler_output.is_symlink():
            if compiler_output.is_symlink() or not compiler_output.is_file():
                raise Rejected("authenticated compiler output handoff is not a regular file")


def _partial_trust_binding(root: ExternalTrustRoot) -> dict[str, Any]:
    values = {field: getattr(root, field) for field in ExternalTrustRoot.FIELDS}
    return {
        "schema": f"{PARTIAL_EVIDENCE_SCHEMA}/trust-root-binding",
        "fields": values,
        "binding_sha256": canonical_hash(values),
    }


def _partial_output_dir(value: Path | str, auth: Mapping[str, Any]) -> Path:
    raw = Path(value)
    if not raw.is_absolute():
        raise Rejected("partial evidence output directory must be absolute")
    output = _canonical_path(raw, "partial evidence output directory", directory=True, must_exist=False)
    if output.exists() or output.is_symlink():
        raise Rejected("partial evidence output directory already exists")
    parent = _canonical_path(output.parent, "partial evidence output parent", directory=True)
    if output.parent != parent:
        raise Rejected("partial evidence output parent is not canonical")
    capture_dir = _canonical_path(auth["output_dir"], "capture output directory", directory=True)
    if output == capture_dir or capture_dir in output.parents:
        raise Rejected("partial evidence output must be outside the raw capture directory")
    return output


def _exact_edge_map(
    events: Sequence[Mapping[str, Any]],
    *,
    event_kind: str,
    value_fields: tuple[str, ...],
    label: str,
) -> dict[str, tuple[Any, ...]]:
    result: dict[str, tuple[Any, ...]] = {}
    for event in events:
        if event.get("event_kind") != event_kind or event.get("status") != "EXACT":
            continue
        token = event.get("object_token")
        if not isinstance(token, str):
            continue
        value = tuple(event.get(field) for field in value_fields)
        previous = result.get(token)
        if previous is not None:
            raise Rejected(f"partial evidence contains ambiguous {label} token")
        result[token] = value
    return result


def _build_volatile_owner_facts(
    events: Sequence[Mapping[str, Any]],
    context: Mapping[str, Any],
    candidate_object: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Freeze capture-local PCode/IG facts without promoting them to authority."""

    session_id = _safe_session_id(context.get("session_id"))
    function = _text(context.get("function"), "volatile owner function")
    source = _descriptor(context.get("source"), "volatile owner source", verify=False)
    compiler = _descriptor(context.get("compiler"), "volatile owner compiler", verify=False)
    machine_by_pcode: dict[str, list[dict[str, Any]]] = {}
    machine_role_orders: dict[str, list[tuple[str, list[str]]]] = {}
    exact_vregs: dict[str, list[str]] = {}
    exact_physical: dict[str, list[str]] = {}

    for event in events:
        if not isinstance(event, Mapping):
            raise Rejected("volatile owner event is not an object")
        kind = event.get("event_kind")
        if kind == "regalloc_assignment" and event.get("status") == "EXACT":
            token = _text(event.get("object_token"), "volatile owner Object token")
            _token_parts(token, "volatile owner Object token", session_id)
            exact_vregs.setdefault(token, []).append(
                _validate_vreg(event.get("vreg_id"), "volatile owner vreg")
            )
        if kind == "physical_reg_assignment" and event.get("status") == "EXACT":
            token = _text(event.get("object_token"), "volatile physical Object token")
            _token_parts(token, "volatile physical Object token", session_id)
            bank = event.get("bank")
            color = _integer(event.get("physical_reg"), "volatile physical color", nonnegative=True)
            if bank not in {"GPR", "FPR"} or color > 31:
                raise Rejected("volatile physical assignment is invalid")
            exact_physical.setdefault(token, []).append(
                f"{'r' if bank == 'GPR' else 'f'}{color}"
            )
        if kind != "machine_emission" or event.get("status") not in {"CAPTURED", "UNKNOWN"}:
            continue
        if not isinstance(event.get("pcode_token"), str) or not isinstance(event.get("registers"), Mapping):
            continue
        pcode_id = str(event["pcode_token"])
        pcode_match = PCODE_TOKEN_RE.fullmatch(pcode_id)
        if pcode_match is None or pcode_match.group("session") != session_id:
            raise Rejected("volatile machine PCode provenance changed")
        role_order = event.get("operand_role_order")
        if role_order is not None:
            if (
                not isinstance(role_order, list)
                or not role_order
                or any(not isinstance(role, str) or role not in event["registers"] for role in role_order)
                or len(role_order) != len(set(role_order))
            ):
                raise Rejected("volatile machine operand role order is invalid")
            machine_role_orders.setdefault(pcode_id, []).append(
                (_text(event.get("event_id"), "volatile machine event ID"), list(role_order))
            )
        instruction_index = _integer(
            event.get("instruction_index"), "volatile instruction index", nonnegative=True
        )
        reaching = {
            _integer(value, "volatile reaching definition", nonnegative=True)
            for value in event.get("reaching_definitions", [])
        }
        known_by_register: dict[str, set[int]] = {}
        for row in event.get("known_reaching_definitions", []):
            if not isinstance(row, Mapping):
                raise Rejected("volatile known reaching definition is malformed")
            register = _text(row.get("physical_register"), "volatile reaching register")
            known_by_register.setdefault(register, set()).add(
                _integer(row.get("instruction_index"), "volatile known definition", nonnegative=True)
            )
        effects: list[dict[str, Any]] = []
        for role in sorted(event["registers"]):
            register = event["registers"][role]
            if not isinstance(role, str) or not isinstance(register, str) or re.fullmatch(
                r"[rf](?:[0-9]|[12][0-9]|3[01])", register
            ) is None:
                raise Rejected("volatile machine register effect is invalid")
            is_def = role == "destination" or (
                role == "data" and event.get("memory_op") == "load"
            )
            effects.append({
                "event_id": event.get("event_id"),
                "instruction_index": instruction_index,
                "role": role,
                "physical_register": register,
                "effect": "DEF" if is_def else "USE",
                "reaching": sorted(known_by_register.get(register, reaching)),
            })
        machine_by_pcode.setdefault(pcode_id, []).extend(effects)

    closed_rows: list[dict[str, Any]] = []
    owner_facts: list[dict[str, Any]] = []
    object_bindings: list[dict[str, Any]] = []
    selected_event_ids: set[str] = set()
    selected_object_tokens: set[str] = set()
    machine_effects = [
        effect
        for effects in machine_by_pcode.values()
        for effect in effects
    ]
    for event in events:
        if (
            not isinstance(event, Mapping)
            or event.get("event_kind") != "pcode_capture"
            or event.get("status") != "CAPTURED"
            or "operand_ordinal" not in event
        ):
            continue
        pcode_id = _text(event.get("pcode_token"), "volatile operand PCode ID")
        ig_node_id = _text(event.get("ig_token"), "volatile operand IG ID")
        pcode_match = PCODE_TOKEN_RE.fullmatch(pcode_id)
        ig_match = IG_TOKEN_RE.fullmatch(ig_node_id)
        if (
            pcode_match is None
            or ig_match is None
            or pcode_match.group("session") != session_id
            or ig_match.group("session") != session_id
        ):
            raise Rejected("volatile PCode/IG provenance changed")
        ordinal = _integer(event.get("operand_ordinal"), "volatile operand ordinal", nonnegative=True)
        count = _integer(event.get("operand_count"), "volatile operand count", nonnegative=True)
        kind = _integer(event.get("operand_kind"), "volatile operand kind", nonnegative=True)
        final_color = _integer(event.get("final_color"), "volatile final color", nonnegative=True)
        bank = event.get("operand_bank")
        if count < 1 or ordinal >= count or bank not in {"GPR", "FPR"} or final_color > 31:
            raise Rejected("volatile operand chronology/color is invalid")
        physical_register = f"{'r' if bank == 'GPR' else 'f'}{final_color}"
        object_token: str | None = None
        hidden_token: str | None = None
        if isinstance(event.get("object_token"), str):
            object_token = str(event["object_token"])
            _token_parts(object_token, "volatile operand Object token", session_id)
        elif isinstance(event.get("hidden_owner_token"), str):
            hidden_token = str(event["hidden_owner_token"])
            hidden_match = HIDDEN_IG_TOKEN_RE.fullmatch(hidden_token)
            if hidden_match is None or hidden_match.group("session") != session_id:
                raise Rejected("volatile hidden IG provenance changed")
        else:
            raise Rejected("volatile operand lacks an owner identity")

        role_orders = machine_role_orders.get(pcode_id, [])
        role_order_ambiguous = len(role_orders) > 1
        selected_role: str | None = None
        supporting_machine_events: set[str] = set()
        plausible_roles: list[str] = []
        if len(role_orders) == 1 and len(role_orders[0][1]) == count:
            supporting_machine_event, order = role_orders[0]
            supporting_machine_events.add(supporting_machine_event)
            selected_role = order[ordinal]
            plausible_roles = [selected_role]
        elif role_order_ambiguous:
            role_candidates: set[str] = set()
            for event_id, order in role_orders:
                if len(order) == count:
                    supporting_machine_events.add(event_id)
                    role_candidates.add(order[ordinal])
            plausible_roles = sorted(role_candidates)
        candidates_by_role: dict[str, Mapping[str, Any]] = {}
        for effect in machine_by_pcode.get(pcode_id, []):
            if (
                effect["role"] in plausible_roles
                and effect["physical_register"] == physical_register
            ):
                candidates_by_role.setdefault(str(effect["role"]), effect)
        candidates = [candidates_by_role[role] for role in plausible_roles if role in candidates_by_role]
        # Anonymous IG nodes with no authenticated ordinal-to-role machine
        # edge add bulk but no join capability.  The complete events remain in
        # the separately hash-bound raw streams.
        if object_token is None and not candidates:
            continue
        variants: list[Mapping[str, Any] | None] = candidates or [None]
        required_roles = plausible_roles
        pcode_event_id = _text(event.get("event_id"), "volatile PCode event ID")
        selected_event_ids.add(pcode_event_id)
        selected_event_ids.update(supporting_machine_events)
        if object_token is not None:
            selected_object_tokens.add(object_token)
        for candidate in variants:
            fact_index = len(owner_facts)
            fact_id = f"owner-fact-{fact_index:06d}"
            row_id = f"residual-{fact_index:06d}"
            vreg_candidates = [] if object_token is None else exact_vregs.get(object_token, [])
            all_physical_candidates = (
                [] if object_token is None else exact_physical.get(object_token, [])
            )
            physical_candidates = [] if object_token is None else [
                value for value in all_physical_candidates
                if value == physical_register
            ]
            if object_token is None:
                classification = "UNKNOWN_MISSING_OBJECT_EDGE"
            elif role_order_ambiguous:
                classification = "UNKNOWN_AMBIGUOUS_MACHINE_EDGE"
            elif not candidates:
                classification = "UNKNOWN_MISSING_MACHINE_EDGE"
            elif len(candidates) > 1:
                classification = "UNKNOWN_AMBIGUOUS_MACHINE_EDGE"
            elif len(vreg_candidates) > 1 or len(all_physical_candidates) > 1:
                classification = "UNKNOWN_AMBIGUOUS_ASSIGNMENT_EDGE"
            elif not vreg_candidates:
                classification = "UNKNOWN_MISSING_VREG_EDGE"
            elif not physical_candidates:
                classification = "UNKNOWN_MISSING_ASSIGNMENT_EDGE"
            else:
                classification = "UNIQUE"

            def_id: str | None = None
            use_ids: list[str] = []
            role = "UNKNOWN" if candidate is None else str(candidate["role"])
            if candidate is not None:
                selected_event_ids.add(
                    _text(candidate.get("event_id"), "volatile owner machine event ID")
                )
                if candidate["effect"] == "DEF":
                    definition_index = int(candidate["instruction_index"])
                    def_id = f"def-{definition_index:06d}"
                else:
                    reaching = list(candidate["reaching"])
                    definition_index = reaching[0] if len(reaching) == 1 else None
                    if definition_index is not None:
                        def_id = f"def-{definition_index:06d}"
                    use_ids.append(f"use-{int(candidate['instruction_index']):06d}")
            if def_id is not None:
                definition_index = int(def_id.rsplit("-", 1)[1])
                for effect in machine_effects:
                    same_register = effect["physical_register"] == physical_register
                    supports_definition = (
                        same_register
                        and effect["effect"] == "DEF"
                        and int(effect["instruction_index"]) == definition_index
                    )
                    supports_use = (
                        same_register
                        and effect["effect"] == "USE"
                        and definition_index in effect["reaching"]
                    )
                    if supports_definition or supports_use:
                        selected_event_ids.add(
                            _text(effect.get("event_id"), "volatile owner chronology event ID")
                        )
                    if supports_use:
                        use_ids.append(f"use-{int(effect['instruction_index']):06d}")
            use_ids = sorted(set(use_ids))
            closed_rows.append({
                "row_id": row_id,
                "ordinal": ordinal,
                "kind": kind,
                "owner_fact_id": fact_id,
                "required_operand_roles": required_roles,
            })
            owner_facts.append({
                "fact_id": fact_id,
                "row_id": row_id,
                "role": role,
                "pcode_id": pcode_id,
                "ig_node_id": ig_node_id,
                "vreg": vreg_candidates[0] if len(vreg_candidates) == 1 else None,
                "final_color": final_color,
                "physical_register": physical_register,
                "def_id": def_id,
                "use_ids": use_ids,
                "classification": classification,
            })
            binding: dict[str, Any] = {"fact_id": fact_id, "status": "UNKNOWN"}
            if object_token is not None:
                binding.update(status="PRESENT", object_token=object_token)
            elif hidden_token is not None:
                binding["hidden_owner_token"] = hidden_token
            object_bindings.append(binding)

    result: dict[str, Any] = {
        "schema": f"{PARTIAL_FAILURE_GRAPH_SCHEMA}/volatile-owner-facts/v1",
        "status": "DIAGNOSTIC_ONLY",
        "authority_advanced": False,
        "session_id": session_id,
        "function": function,
        "source": source,
        "compiler": compiler,
        "raw_event_hashes": [
            {"event_id": event.get("event_id"), "sha256": canonical_hash(event)}
            for event in events
            if (
                event.get("event_id") in selected_event_ids
                or (
                    event.get("event_kind") in {"regalloc_assignment", "physical_reg_assignment"}
                    and event.get("object_token") in selected_object_tokens
                )
            )
        ],
        "object_identity_bindings": object_bindings,
        "closed_residual_rows": closed_rows,
        "owner_facts": owner_facts,
    }
    if candidate_object is not None:
        result["candidate_object"] = _descriptor(
            candidate_object, "volatile owner candidate object", verify=False
        )
    _pointer_free(result)
    result["volatile_owner_facts_sha256"] = canonical_hash(result)
    return result


def _build_ownership_failure_graph(
    envelope: Mapping[str, Any],
    validation_failure: str,
    candidate_object: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    events = envelope.get("events")
    context = envelope.get("context")
    inventory = envelope.get("inventory")
    if not isinstance(events, list) or not isinstance(context, Mapping) or not isinstance(inventory, Mapping):
        raise Rejected("partial evidence envelope is structurally incomplete")
    session_id = _safe_session_id(context.get("session_id"))
    function = _text(context.get("function"), "partial evidence function")
    expected_sequences = list(range(len(events)))
    sequences = [event.get("sequence") if isinstance(event, Mapping) else None for event in events]
    if sequences != expected_sequences:
        raise Rejected("partial evidence chronology is noncanonical")
    for index, event in enumerate(events):
        if not isinstance(event, Mapping):
            raise Rejected("partial evidence event is not an object")
        if event.get("event_id") != f"{session_id}-e{index:06d}":
            raise Rejected("partial evidence event identity is noncanonical")
        if event.get("session_id") != session_id or event.get("function") != function:
            raise Rejected("partial evidence event context changed")

    inventory_tokens: set[str] = set()
    for lane in ("locals", "arguments"):
        rows = inventory.get(lane)
        if not isinstance(rows, list):
            raise Rejected("partial evidence inventory is incomplete")
        for row in rows:
            if not isinstance(row, Mapping):
                raise Rejected("partial evidence inventory row is malformed")
            token = _text(row.get("token"), "partial evidence inventory token")
            _token_parts(token, "partial evidence inventory token", session_id)
            if token in inventory_tokens:
                raise Rejected("partial evidence contains a reused inventory token")
            inventory_tokens.add(token)

    object_to_vreg = _exact_edge_map(
        events,
        event_kind="regalloc_assignment",
        value_fields=("vreg_id", "bank"),
        label="Object-to-vreg",
    )
    object_to_physical = _exact_edge_map(
        events,
        event_kind="physical_reg_assignment",
        value_fields=("bank", "physical_reg"),
        label="Object-to-physical",
    )
    volatile_owner_facts = _build_volatile_owner_facts(
        events, context, candidate_object
    )

    pcode_operands: dict[str, dict[int, dict[str, Any]]] = {}
    pcode_operand_counts: dict[str, int] = {}
    for event in events:
        if event.get("event_kind") != "pcode_capture" or event.get("status") != "CAPTURED":
            continue
        pcode_token = event.get("pcode_token")
        if not isinstance(pcode_token, str) or "operand_ordinal" not in event:
            continue
        match = PCODE_TOKEN_RE.fullmatch(pcode_token)
        if match is None or match.group("session") != session_id:
            raise Rejected("partial evidence PCode operand token provenance changed")
        ordinal = _integer(event.get("operand_ordinal"), "partial evidence operand ordinal", nonnegative=True)
        count = _integer(event.get("operand_count"), "partial evidence operand count", nonnegative=True)
        if count < 1 or ordinal >= count:
            raise Rejected("partial evidence PCode operand chronology is invalid")
        previous_count = pcode_operand_counts.setdefault(pcode_token, count)
        if previous_count != count:
            raise Rejected("partial evidence PCode operand count changed")
        operands = pcode_operands.setdefault(pcode_token, {})
        if ordinal in operands:
            raise Rejected("partial evidence contains a reused PCode operand token")
        owner: dict[str, Any]
        if isinstance(event.get("object_token"), str):
            token = str(event["object_token"])
            _token_parts(token, "partial evidence PCode object token", session_id)
            owner = {"status": "PRESENT", "object_token": token}
        elif isinstance(event.get("hidden_owner_token"), str):
            hidden = str(event["hidden_owner_token"])
            hidden_match = HIDDEN_IG_TOKEN_RE.fullmatch(hidden)
            if hidden_match is None or hidden_match.group("session") != session_id:
                raise Rejected("partial evidence hidden PCode owner provenance changed")
            owner = {"status": "UNKNOWN", "hidden_owner_token": hidden}
        else:
            raise Rejected("partial evidence PCode operand lacks an owner identity")
        operands[ordinal] = {
            "event_id": event["event_id"],
            "sequence": event["sequence"],
            "operand_ordinal": ordinal,
            "operand_count": count,
            "operand_bank": event.get("operand_bank"),
            "final_color": event.get("final_color"),
            "ig_token": event.get("ig_token"),
            "owner_identity": owner,
        }

    unresolved_machine_sites: list[dict[str, Any]] = []
    for event in events:
        if (
            event.get("event_kind") != "machine_emission"
            or event.get("status") != "UNKNOWN"
            or "known_reaching_definitions" not in event
        ):
            continue
        pcode_token = _text(event.get("pcode_token"), "partial evidence unresolved PCode token")
        count = pcode_operand_counts.get(pcode_token)
        operands = pcode_operands.get(pcode_token)
        if count is None or operands is None or sorted(operands) != list(range(count)):
            raise Rejected("partial evidence unresolved machine site lacks complete PCode operands")
        unresolved_machine_sites.append(
            {
                "event_id": event["event_id"],
                "sequence": event["sequence"],
                "instruction_index": event["instruction_index"],
                "pcode_token": pcode_token,
                "ppc_bytes": event["ppc_bytes"],
                "mnemonic": event["mnemonic"],
                "registers": dict(event["registers"]),
                "reason": event["reason"],
                "known_reaching_definitions": [
                    dict(row) for row in event["known_reaching_definitions"]
                ],
                "missing_reaching_registers": list(event["missing_reaching_registers"]),
                "pcode_operands": [dict(operands[ordinal]) for ordinal in range(count)],
            }
        )

    attempts: list[dict[str, Any]] = []
    for event in events:
        if event.get("event_kind") != "machine_emission" or event.get("status") != "CAPTURED":
            continue
        registers = event.get("registers")
        machine_registers = set(registers.values()) if isinstance(registers, Mapping) else set()
        raw_joins: list[tuple[str, Mapping[str, Any]]] = []
        owner_joins = event.get("owner_joins", [])
        physical_joins = event.get("physical_owner_joins", [])
        if isinstance(owner_joins, list):
            raw_joins.extend(("object_vreg_physical", row) for row in owner_joins if isinstance(row, Mapping))
        if isinstance(physical_joins, list):
            raw_joins.extend(("object_physical", row) for row in physical_joins if isinstance(row, Mapping))
        seen_joins: set[tuple[str, str]] = set()
        for join_kind, join in raw_joins:
            token = _text(join.get("object_token"), "partial evidence machine object token")
            register = _text(join.get("physical_register"), "partial evidence machine register")
            _token_parts(token, "partial evidence machine object token", session_id)
            join_key = (token, register)
            if join_key in seen_joins:
                if join_kind == "object_physical":
                    continue
                raise Rejected("partial evidence contains an ambiguous machine owner join")
            seen_joins.add(join_key)
            if not re.fullmatch(r"[rf](?:[0-9]|[12][0-9]|3[01])", register):
                raise Rejected("partial evidence machine register is invalid")
            expected_bank = "GPR" if register.startswith("r") else "FPR"
            expected_physical = int(register[1:])
            virtual = object_to_vreg.get(token)
            physical = object_to_physical.get(token)
            inventory_edge = {"status": "PRESENT" if token in inventory_tokens else "MISSING"}
            if virtual is None:
                vreg_edge: dict[str, Any] = {"status": "MISSING"}
            else:
                vreg_status = "PRESENT"
                claimed_vreg = join.get("vreg_id")
                if claimed_vreg is not None and claimed_vreg != virtual[0]:
                    vreg_status = "CONFLICT"
                vreg_edge = {"status": vreg_status, "vreg_id": virtual[0], "bank": virtual[1]}
            if physical is None:
                physical_edge: dict[str, Any] = {
                    "status": "MISSING",
                    "expected_bank": expected_bank,
                    "expected_physical_reg": expected_physical,
                }
            else:
                physical_status = "PRESENT" if physical == (expected_bank, expected_physical) else "CONFLICT"
                physical_edge = {
                    "status": physical_status,
                    "bank": physical[0],
                    "physical_reg": physical[1],
                    "expected_bank": expected_bank,
                    "expected_physical_reg": expected_physical,
                }
            machine_edge = {"status": "PRESENT" if register in machine_registers else "MISSING"}
            attempts.append(
                {
                    "event_id": event["event_id"],
                    "sequence": event["sequence"],
                    "instruction_index": event.get("instruction_index"),
                    "mnemonic": event.get("mnemonic"),
                    "pcode_token": event.get("pcode_token"),
                    "reaching_definitions": list(event.get("reaching_definitions", [])),
                    "join_kind": join_kind,
                    "object_token": token,
                    "physical_register": register,
                    "edges": {
                        "inventory_object": inventory_edge,
                        "object_to_vreg": vreg_edge,
                        "vreg_to_physical": physical_edge,
                        "machine_operand": machine_edge,
                    },
                }
            )

    if not attempts:
        raise Rejected("partial evidence has no machine ownership join attempts")
    first_absent: dict[str, Any] | None = None
    edge_order = ("inventory_object", "object_to_vreg", "vreg_to_physical", "machine_operand")
    for attempt in attempts:
        for edge_name in edge_order:
            edge = attempt["edges"][edge_name]
            if edge["status"] != "PRESENT":
                first_absent = {
                    "event_id": attempt["event_id"],
                    "sequence": attempt["sequence"],
                    "instruction_index": attempt["instruction_index"],
                    "object_token": attempt["object_token"],
                    "physical_register": attempt["physical_register"],
                    "edge": edge_name,
                    "status": edge["status"],
                }
                break
        if first_absent is not None:
            break
    if first_absent is None:
        raise Rejected("partial evidence failure graph did not isolate an absent ownership edge")

    graph: dict[str, Any] = {
        "schema": PARTIAL_FAILURE_GRAPH_SCHEMA,
        "status": "UNKNOWN",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "session_id": session_id,
        "function": function,
        "validation_failure": validation_failure,
        "first_absent_edge": first_absent,
        "join_attempts": attempts,
        "unresolved_machine_sites": unresolved_machine_sites,
        "volatile_owner_facts": volatile_owner_facts,
        "machine_event_ids": [
            event["event_id"]
            for event in events
            if event.get("event_kind") == "machine_emission"
        ],
    }
    _pointer_free(graph)
    graph["failure_graph_sha256"] = canonical_hash(graph)
    return graph


def _validate_partial_capture_boundary(auth: Mapping[str, Any]) -> dict[str, Any]:
    output_dir = _canonical_path(auth["output_dir"], "capture output directory", directory=True)
    request_path = _canonical_path(auth["request_path"], "capture request")
    capture_paths = {_canonical_path(path, f"capture output {name}") for name, path in auth["paths"].items()}
    compiler_outputs = tuple(auth.get("compiler_output_paths", ()))
    if len(compiler_outputs) != 1:
        raise Rejected("partial evidence requires exactly one compiler-owned -o output")
    compiler_output = _canonical_path(compiler_outputs[0], "compiler-owned -o output")
    if compiler_output.is_symlink() or not compiler_output.is_file():
        raise Rejected("compiler-owned -o output is not a regular file")
    allowed = {request_path, *capture_paths}
    if compiler_output.parent == output_dir:
        allowed.add(compiler_output)
    observed: set[Path] = set()
    for entry in output_dir.iterdir():
        if entry.is_symlink() or not entry.is_file():
            raise Rejected("partial evidence capture boundary contains a non-regular entry")
        observed.add(entry.resolve())
    if observed != allowed:
        raise Rejected("partial evidence capture boundary contains stale or unowned output")
    return _descriptor(compiler_output, "compiler-owned object")


def _publish_partial_evidence(
    output_dir: Path | str,
    *,
    auth: Mapping[str, Any],
    envelope: Mapping[str, Any],
    validation_failure: str,
    trust_root: ExternalTrustRoot,
) -> dict[str, Any]:
    if validation_failure not in _PRESERVABLE_FINAL_JOIN_FAILURES:
        raise Rejected("capture failure is not an authenticated final ownership-join UNKNOWN")
    revalidate_request(auth)
    destination = _partial_output_dir(output_dir, auth)
    compiler_object = _validate_partial_capture_boundary(auth)
    proof = auth.get("prelaunch_empty_output_proof")
    if not isinstance(proof, Mapping) or proof.get("proof_sha256") != canonical_hash(
        {key: value for key, value in proof.items() if key != "proof_sha256"}
    ):
        raise Rejected("partial evidence lacks an authenticated prelaunch empty-output proof")
    if envelope.get("envelope_sha256") != canonical_hash(
        {key: value for key, value in envelope.items() if key != "envelope_sha256"}
    ):
        raise Rejected("partial evidence candidate envelope self-digest mismatch")

    events = envelope.get("events")
    if not isinstance(events, list):
        raise Rejected("partial evidence candidate envelope has no event stream")
    stack_events = [event for event in events if event.get("lane") == "stack"]
    pcode_events = [event for event in events if event.get("lane") == "pcode"]
    machine_events = [event for event in pcode_events if event.get("event_kind") == "machine_emission"]
    blobs: dict[str, bytes] = {
        "stack_events": canonical_lane_bytes(stack_events),
        "pcode_events": canonical_lane_bytes(pcode_events),
        "machine_events": canonical_lane_bytes(machine_events),
        "candidate_envelope": (json.dumps(envelope, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8"),
    }
    for name, request_key in (("stack_events", "event_stream_stack"), ("pcode_events", "event_stream_pcode"), ("candidate_envelope", "envelope")):
        source_path = _canonical_path(auth["paths"][request_key], f"partial evidence source {name}")
        if source_path.read_bytes() != blobs[name]:
            raise Rejected(f"partial evidence source {name} changed after capture")

    graph = _build_ownership_failure_graph(
        envelope, validation_failure, candidate_object=compiler_object
    )
    context = envelope.get("context")
    if not isinstance(context, Mapping):
        raise Rejected("partial evidence context is missing")
    hook_receipt: dict[str, Any] = {
        "schema": PARTIAL_HOOK_RECEIPT_SCHEMA,
        "status": "AUTHENTICATED_PARTIAL_CAPTURE",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "session_id": context["session_id"],
        "function": context["function"],
        "function_sha256": context["function_sha256"],
        "request": dict(context["request"]),
        "source": dict(context["source"]),
        "compiler": dict(context["compiler"]),
        "wrapper": dict(context["wrapper"]),
        "debugger": dict(context["debugger"]),
        "transport": dict(context["transport"]),
        "argv": list(context["argv"]),
        "cwd": context["cwd"],
        "execution": dict(context["execution"]) if isinstance(context.get("execution"), Mapping) else None,
        "hooks": [dict(row) for row in envelope["hooks"]],
        "hook_count": len(envelope["hooks"]),
        "prelaunch_empty_output_proof": dict(proof),
        "compiler_owned_object": compiler_object,
        "validation_failure": validation_failure,
    }
    _pointer_free(hook_receipt)
    hook_receipt["receipt_sha256"] = canonical_hash(hook_receipt)
    blobs["hook_validation"] = (json.dumps(hook_receipt, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8")
    blobs["failure_graph"] = (json.dumps(graph, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8")

    artifact_descriptors = {
        name: {
            "path": str(destination / _PARTIAL_EVIDENCE_FILENAMES[name]),
            "size": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
        }
        for name, data in blobs.items()
    }
    manifest: dict[str, Any] = {
        "schema": PARTIAL_EVIDENCE_SCHEMA,
        "status": "UNKNOWN",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "immutable": True,
        "package_dir": str(destination),
        "context": dict(context),
        "request": _descriptor(auth["request_path"], "partial evidence request"),
        "trust_root": _partial_trust_binding(trust_root),
        "compiler_owned_object": compiler_object,
        "validation_failure": validation_failure,
        "first_absent_edge": dict(graph["first_absent_edge"]),
        "artifacts": artifact_descriptors,
    }
    _pointer_free(manifest)
    manifest["manifest_sha256"] = canonical_hash(manifest)
    manifest_bytes = (json.dumps(manifest, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8")

    with tempfile.TemporaryDirectory(prefix=".partial-evidence-", dir=destination.parent) as temporary:
        staging = Path(temporary)
        for name, data in blobs.items():
            write_new(staging / _PARTIAL_EVIDENCE_FILENAMES[name], data)
        write_new(staging / _PARTIAL_EVIDENCE_FILENAMES["manifest"], manifest_bytes)
        for name, descriptor in artifact_descriptors.items():
            staged = staging / _PARTIAL_EVIDENCE_FILENAMES[name]
            if staged.stat().st_size != descriptor["size"] or sha256(staged) != descriptor["sha256"]:
                raise Rejected(f"partial evidence staged {name} identity mismatch")
        if sha256(staging / _PARTIAL_EVIDENCE_FILENAMES["manifest"]) != hashlib.sha256(manifest_bytes).hexdigest():
            raise Rejected("partial evidence staged manifest identity mismatch")
        os.rename(staging, destination)

    return {
        "schema": f"{PARTIAL_EVIDENCE_SCHEMA}/capture",
        "status": "UNKNOWN",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "package": _descriptor(destination / _PARTIAL_EVIDENCE_FILENAMES["manifest"], "partial evidence manifest"),
        "manifest_sha256": manifest["manifest_sha256"],
        "first_absent_edge": manifest["first_absent_edge"],
        "validation_failure": validation_failure,
    }


def _remove_partial_outputs(auth: Mapping[str, Any] | None) -> None:
    if not isinstance(auth, Mapping):
        return
    paths = auth.get("paths")
    if not isinstance(paths, Mapping):
        return
    candidates: list[Path] = [Path(value) for value in paths.values()]
    seen: set[Path] = set()
    for path in candidates:
        if path in seen:
            continue
        seen.add(path)
        try:
            if path.exists() or path.is_symlink():
                path.unlink()
        except OSError:
            # Do not mask the original capture failure.  The next invocation
            # will fail the empty-output gate rather than consuming partial
            # evidence.
            pass


def capture_with_backend(
    request_path: Path | str,
    backend: CaptureBackend,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    preauthenticated_auth: Mapping[str, Any] | None = None,
    partial_evidence_dir: Path | str | None = None,
) -> dict[str, Any]:
    """Run one fake/native backend and atomically publish its complete envelope."""

    auth: dict[str, Any] | None = None
    root: ExternalTrustRoot | None = None
    envelope: dict[str, Any] | None = None
    result: dict[str, Any] | None = None
    validation_failure: str | None = None
    failure: Rejected | None = None
    try:
        if external_trust_root is not None and trust_root is not None:
            raise Rejected("conflicting external trust root arguments")
        root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
        if preauthenticated_auth is None:
            auth = authenticate_request(request_path, require_empty=True, external_trust_root=root)
        else:
            auth = dict(preauthenticated_auth)
            if _canonical_path(request_path, "request") != Path(auth.get("request_path", "")).resolve():
                raise Rejected("preauthenticated request handoff path changed")
            if root != auth.get("trust_root"):
                raise Rejected("preauthenticated request handoff trust root changed")
            fresh = authenticate_request(request_path, external_trust_root=root)
            for key in (
                "request", "request_path", "request_sha256", "paths", "output_dir",
                "compiler_output_paths", "hooks", "trust_root",
            ):
                if fresh.get(key) != auth.get(key):
                    raise Rejected("preauthenticated request handoff metadata changed")
            revalidate_request(auth)
            _validate_post_launch_output_handoff(auth)
        revalidate_request(auth)
        session = CombinedCaptureSession(auth, backend)
        envelope = session.run()
        revalidate_request(auth)
        if preauthenticated_auth is not None:
            _validate_post_launch_output_handoff(auth)
        paths = auth["paths"]
        stack_events = [event for event in envelope["events"] if event["lane"] == "stack"]
        pcode_events = [event for event in envelope["events"] if event["lane"] == "pcode"]
        write_new(paths["event_stream_stack"], canonical_lane_bytes(stack_events))
        write_new(paths["event_stream_pcode"], canonical_lane_bytes(pcode_events))
        write_new(paths["envelope"], (json.dumps(envelope, ensure_ascii=True, sort_keys=True, indent=2) + "\n").encode("utf-8"))
        revalidate_request(auth)
        output_values = {field: getattr(root, field) for field in ExternalTrustRoot.FIELDS} if root is not None else {}
        for name, path in (
            ("event_stream_stack", paths["event_stream_stack"]),
            ("event_stream_pcode", paths["event_stream_pcode"]),
            ("envelope", paths["envelope"]),
        ):
            record = _descriptor(path, f"capture.{name}")
            expected = {
                "path": getattr(root, f"{name}_path"),
                "size": getattr(root, f"{name}_size"),
                "sha256": getattr(root, f"{name}_sha256"),
            }
            if any(value is not None for value in expected.values()) and expected != record:
                raise Rejected(f"external trust root.{name} does not match captured output")
            output_values[f"{name}_path"] = record["path"]
            output_values[f"{name}_size"] = record["size"]
            output_values[f"{name}_sha256"] = record["sha256"]
        output_root = ExternalTrustRoot(**output_values)
        try:
            validate_envelope(paths["envelope"], external_trust_root=output_root)
        except Rejected as exc:
            if partial_evidence_dir is None or str(exc) not in _PRESERVABLE_FINAL_JOIN_FAILURES:
                raise
            if root is None:
                raise Rejected("partial evidence requires an external trust root") from exc
            validation_failure = str(exc)
        else:
            result = envelope
    except Exception as exc:
        if isinstance(exc, Rejected):
            failure = exc
        else:
            failure = Rejected(f"capture failure: {type(exc).__name__}: {exc}")
    finally:
        close = getattr(backend, "close", None)
        if callable(close):
            try:
                close()
            except Exception as exc:
                cleanup_text = f"native cleanup failure: {type(exc).__name__}: {exc}"
                # Preserve the causal failure first. Cleanup is secondary
                # evidence and must never replace the compiler/capture outcome
                # that caused cleanup to run.
                failure = _combine_terminal_failure(failure, cleanup_text)

    if validation_failure is not None and failure is None:
        assert auth is not None and envelope is not None and root is not None and partial_evidence_dir is not None
        try:
            result = _publish_partial_evidence(
                partial_evidence_dir,
                auth=auth,
                envelope=envelope,
                validation_failure=validation_failure,
                trust_root=root,
            )
        except Exception as exc:
            if isinstance(exc, Rejected):
                failure = exc
            else:
                failure = Rejected(f"partial evidence publication failure: {type(exc).__name__}: {exc}")

    if failure is not None or validation_failure is not None:
        _remove_partial_outputs(auth)
    if failure is not None:
        raise failure
    if result is None:
        raise Rejected("capture produced no authenticated result")
    return result


def _token_parts(value: Any, label: str, session_id: str, kind: str | None = None) -> tuple[str, int]:
    token = _text(value, label)
    match = TOKEN_RE.fullmatch(token)
    if match is None:
        raise Rejected(f"{label} is not a canonical session token")
    token_kind = str(match.group("kind"))
    token_session = str(match.group("session"))
    ordinal = str(match.group("ordinal"))
    if token_session != session_id:
        raise Rejected(f"{label} provenance does not match session")
    if kind is not None and token_kind != kind:
        raise Rejected(f"{label} has the wrong token kind")
    return token_kind, int(ordinal)


def _validate_token_list(value: Any, label: str, session_id: str, kind: str) -> list[str]:
    if not isinstance(value, list):
        raise Rejected(f"{label} must be a list")
    result = []
    seen: set[str] = set()
    for index, token in enumerate(value):
        _token_parts(token, f"{label}[{index}]", session_id, kind)
        if token in seen:
            raise Rejected(f"{label} contains duplicate token claims")
        seen.add(token)
        result.append(token)
    return result


def _validate_event(event: Mapping[str, Any], index: int, context: Mapping[str, Any]) -> None:
    if not isinstance(event, Mapping):
        raise Rejected(f"event[{index}] is not an object")
    if set(_EVENT_BASE_KEYS).difference(event):
        raise Rejected(f"event[{index}] is missing base fields")
    unsupported = set(event).difference(_EVENT_BASE_KEYS | _EVENT_EXTRA_KEYS)
    if unsupported:
        raise Rejected(f"event[{index}] has unsupported fields: {sorted(unsupported)}")
    _pointer_free(event, f"event[{index}]")
    if event["schema"] != EVENT_SCHEMA:
        raise Rejected(f"event[{index}] schema mismatch")
    _text(event["event_id"], f"event[{index}].event_id")
    _integer(event["process_id"], f"event[{index}].process_id", nonnegative=True)
    if int(event["process_id"]) == 0:
        raise Rejected(f"event[{index}] process_id is zero")
    if event["session_id"] != context["session_id"] or event["process_id"] != context["process_id"] or event["function"] != context["function"]:
        raise Rejected(f"event[{index}] context mismatch")
    if event["lane"] not in _LANES:
        raise Rejected(f"event[{index}] lane is invalid")
    if not isinstance(event["sequence"], int) or isinstance(event["sequence"], bool):
        raise Rejected(f"event[{index}].sequence is not type-canonical")
    sequence = _integer(event["sequence"], f"event[{index}].sequence", nonnegative=True)
    if sequence != index:
        raise Rejected(f"event[{index}] sequence is not contiguous")
    expected_id = f"{context['session_id']}-e{index:06d}"
    if event["event_id"] != expected_id:
        raise Rejected(f"event[{index}] id is not canonical")
    event_kind = _text(event["event_kind"], f"event[{index}].event_kind")
    if event_kind not in _EVENT_KINDS:
        raise Rejected(f"event[{index}] kind is unsupported")
    fields = set(event) - _EVENT_BASE_KEYS
    allowed = _EVENT_ALLOWED_FIELDS[event_kind]
    required = _EVENT_REQUIRED_FIELDS[event_kind]
    if not required.issubset(fields) or not fields.issubset(allowed):
        missing = sorted(required - fields)
        extra = sorted(fields - allowed)
        raise Rejected(f"event[{index}] {event_kind} payload is not closed (missing={missing}, extra={extra})")
    profile_hooks = _hooks_for_compiler(str(context["compiler"]["sha256"]))
    profile_hook_by_id = {str(row["id"]): row for row in profile_hooks}
    if event_kind in {"function_entry", "numeric_stack_alloc_pre", "numeric_stack_alloc_post", "object_stack_write_pre", "object_stack_write_post", "pcode_capture", "machine_emission"}:
        hook_id = _text(event["hook_id"], f"event[{index}].hook_id")
        hook = profile_hook_by_id.get(hook_id)
        if hook is None:
            raise Rejected(f"event[{index}] hook_id is not owned by the compiler profile")
        if hook["lane"] != event["lane"]:
            raise Rejected(f"event[{index}] hook lane mismatch")
    if event_kind in {"compiler_list", "numeric_stack_alloc_pre", "numeric_stack_alloc_post"}:
        _validate_token_list(event["locals"], f"event[{index}].locals", context["session_id"], "local")
        _validate_token_list(event["arguments"], f"event[{index}].arguments", context["session_id"], "argument")
    if event_kind in {"object_stack_write_pre", "object_stack_write_post"}:
        token = event["object_token"]
        if token != "UNKNOWN":
            _token_parts(token, f"event[{index}].object_token", context["session_id"])
        if not isinstance(event["target_slot"], int) or isinstance(event["target_slot"], bool):
            raise Rejected(f"event[{index}].target_slot is not type-canonical")
        _integer(event["target_slot"], f"event[{index}].target_slot")
        if event_kind == "object_stack_write_post" and event["write_observed"] is not True:
            raise Rejected(f"event[{index}] write_observed must be true")
    if event_kind == "pcode_capture":
        status = _text(event["status"], f"event[{index}].status")
        if status not in {"CAPTURED", "UNKNOWN"}:
            raise Rejected(f"event[{index}] PCode status is invalid")
        for key in ("reason", "stage", "opcode", "instruction", "pcode_token", "block"):
            if key in event and not isinstance(event[key], str):
                raise Rejected(f"event[{index}].{key} must be text")
        for key in ("source_offset", "order"):
            if key in event:
                if not isinstance(event[key], int) or isinstance(event[key], bool):
                    raise Rejected(f"event[{index}].{key} is not type-canonical")
                _integer(event[key], f"event[{index}].{key}")
        if "operands" in event and not isinstance(event["operands"], list):
            raise Rejected(f"event[{index}].operands must be a list")
        if status == "UNKNOWN" and ("reason" not in event or not event["reason"]):
            raise Rejected(f"event[{index}] UNKNOWN PCode evidence lacks a reason")
        color_fields = {
            "ig_token", "operand_ordinal", "operand_count", "operand_kind",
            "operand_class", "operand_bank", "operand_index", "final_color",
            "ig_flags", "confirmed",
        }
        if color_fields & fields:
            required = {"hook_id", "status", "stage", "pcode_token", *color_fields}
            if status != "CAPTURED" or not required.issubset(fields):
                raise Rejected(f"event[{index}] PCode color evidence is incomplete")
            pcode_match = PCODE_TOKEN_RE.fullmatch(str(event["pcode_token"]))
            ig_match = IG_TOKEN_RE.fullmatch(str(event["ig_token"]))
            if pcode_match is None or ig_match is None or pcode_match.group("session") != context["session_id"] or ig_match.group("session") != context["session_id"]:
                raise Rejected(f"event[{index}] PCode/IG token provenance is invalid")
            owner_fields = {"object_token", "hidden_owner_token"} & fields
            if len(owner_fields) != 1:
                raise Rejected(f"event[{index}] PCode color owner is not exclusive")
            if "object_token" in owner_fields:
                _token_parts(event["object_token"], f"event[{index}].object_token", context["session_id"])
            else:
                hidden_match = HIDDEN_IG_TOKEN_RE.fullmatch(str(event["hidden_owner_token"]))
                if hidden_match is None or hidden_match.group("session") != context["session_id"]:
                    raise Rejected(f"event[{index}] hidden IG token provenance is invalid")
            for key in ("operand_ordinal", "operand_count", "operand_kind", "operand_class", "operand_index", "final_color", "ig_flags"):
                if not isinstance(event[key], int) or isinstance(event[key], bool):
                    raise Rejected(f"event[{index}].{key} is not type-canonical")
            if not 0 <= event["operand_ordinal"] < event["operand_count"] <= 256 or event["operand_class"] not in {3, 4}:
                raise Rejected(f"event[{index}] PCode color chronology/class is invalid")
            expected_bank = "GPR" if event["operand_class"] == 4 else "FPR"
            if event["operand_bank"] != expected_bank or not 0 <= event["operand_index"] <= 0x7FFF or not 0 <= event["final_color"] <= 31 or event["confirmed"] is not True:
                raise Rejected(f"event[{index}] PCode color operand/result is invalid")
    if event_kind == "regalloc_assignment":
        status = _text(event["status"], f"event[{index}].status")
        if status not in {"EXACT", "UNKNOWN"}:
            raise Rejected(f"event[{index}] register ownership status is invalid")
        if status == "EXACT":
            if set(fields) != {"object_token", "status", "vreg_id", "bank"}:
                raise Rejected(f"event[{index}] exact register ownership is not closed")
            _token_parts(event["object_token"], f"event[{index}].object_token", context["session_id"])
            vreg = _validate_vreg(event["vreg_id"], f"event[{index}].vreg_id")
            bank = event["bank"]
            if bank not in {"GPR", "FPR"} or not vreg.startswith("r" if bank == "GPR" else "f"):
                raise Rejected(f"event[{index}] register bank/vreg mismatch")
        else:
            if set(fields) != {"status", "reason"}:
                raise Rejected(f"event[{index}] UNKNOWN register ownership is not closed")
            if not isinstance(event["reason"], str) or not event["reason"]:
                raise Rejected(f"event[{index}] UNKNOWN register ownership lacks a reason")
    if event_kind == "physical_reg_assignment":
        if event["lane"] != "pcode":
            raise Rejected(f"event[{index}] physical assignment lane is invalid")
        status = _text(event["status"], f"event[{index}].status")
        if status not in {"EXACT", "UNKNOWN"}:
            raise Rejected(f"event[{index}] physical register status is invalid")
        if status == "EXACT":
            if set(fields) != {"object_token", "status", "physical_reg", "bank"}:
                raise Rejected(f"event[{index}] exact physical ownership is not closed")
            _token_parts(event["object_token"], f"event[{index}].object_token", context["session_id"])
            if not isinstance(event["physical_reg"], int) or isinstance(event["physical_reg"], bool):
                raise Rejected(f"event[{index}].physical_reg is not type-canonical")
            if not 0 <= _integer(event["physical_reg"], f"event[{index}].physical_reg") <= 31:
                raise Rejected(f"event[{index}].physical_reg is outside 0..31")
            if event["bank"] not in {"GPR", "FPR"}:
                raise Rejected(f"event[{index}] physical register bank is invalid")
        else:
            if set(fields) != {"status", "reason"}:
                raise Rejected(f"event[{index}] UNKNOWN physical ownership is not closed")
            if not isinstance(event["reason"], str) or not event["reason"]:
                raise Rejected(f"event[{index}] UNKNOWN physical ownership lacks a reason")
    if event_kind == "machine_emission":
        machine_hook_ids = {
            str(row["id"]) for row in profile_hooks if row["role"] == "machine_emit"
        }
        if event["lane"] != "pcode" or event["hook_id"] not in machine_hook_ids:
            raise Rejected(f"event[{index}] machine-emission hook is invalid")
        status = _text(event["status"], f"event[{index}].status")
        if status == "UNKNOWN":
            minimal_unknown = {"hook_id", "status", "reason"}
            located_unknown = minimal_unknown | {
                "pcode_token", "emitted_offset", "instruction_index",
                "opcode_enum", "ppc_word", "ppc_bytes",
            }
            diagnostic_unknown = located_unknown | {
                "mnemonic", "registers", "arithmetic_op", "arithmetic_type",
                "known_reaching_definitions", "missing_reaching_registers",
            }
            diagnostic_unknown_with_roles = diagnostic_unknown | {"operand_role_order"}
            unknown_fields = frozenset(fields)
            if unknown_fields not in {
                frozenset(minimal_unknown),
                frozenset(located_unknown),
                frozenset(diagnostic_unknown),
                frozenset(diagnostic_unknown_with_roles),
            }:
                raise Rejected(f"event[{index}] UNKNOWN machine emission is not closed")
            if event["reason"] not in _KNOWN_UNKNOWN_REASONS:
                raise Rejected(f"event[{index}] UNKNOWN machine reason is unsupported")
            if unknown_fields in {frozenset(located_unknown), frozenset(diagnostic_unknown)}:
                token = _text(event["pcode_token"], f"event[{index}].pcode_token")
                match = PCODE_TOKEN_RE.fullmatch(token)
                if match is None or match.group("session") != context["session_id"]:
                    raise Rejected(f"event[{index}] located UNKNOWN PCode token provenance is invalid")
                emitted_offset = _integer(event["emitted_offset"], f"event[{index}].emitted_offset", nonnegative=True)
                instruction_index = _integer(event["instruction_index"], f"event[{index}].instruction_index", nonnegative=True)
                if emitted_offset % 4 or instruction_index != emitted_offset // 4:
                    raise Rejected(f"event[{index}] located UNKNOWN instruction index is invalid")
                opcode_enum = _integer(event["opcode_enum"], f"event[{index}].opcode_enum", nonnegative=True)
                word = _integer(event["ppc_word"], f"event[{index}].ppc_word", nonnegative=True)
                if opcode_enum > 0x1D4 or word > 0xFFFFFFFF:
                    raise Rejected(f"event[{index}] located UNKNOWN opcode is outside bounds")
                ppc_bytes = _text(event["ppc_bytes"], f"event[{index}].ppc_bytes")
                if re.fullmatch(r"[0-9a-f]{8}", ppc_bytes) is None or int(ppc_bytes, 16) != word:
                    raise Rejected(f"event[{index}] located UNKNOWN PPC bytes/word mismatch")
            if unknown_fields in {
                frozenset(diagnostic_unknown),
                frozenset(diagnostic_unknown_with_roles),
            }:
                if event["reason"] != "ambiguous reaching definition" or event["mnemonic"] != "fmuls":
                    raise Rejected(f"event[{index}] diagnostic UNKNOWN machine class is invalid")
                registers = event["registers"]
                if not isinstance(registers, Mapping) or set(registers) != {
                    "destination", "source_a", "source_b",
                }:
                    raise Rejected(f"event[{index}] diagnostic UNKNOWN machine operands are malformed")
                for register in registers.values():
                    if not isinstance(register, str) or re.fullmatch(r"f(?:[0-9]|[12][0-9]|3[01])", register) is None:
                        raise Rejected(f"event[{index}] diagnostic UNKNOWN FPR is malformed")
                if event["arithmetic_op"] != "multiply" or event["arithmetic_type"] != "f32":
                    raise Rejected(f"event[{index}] diagnostic UNKNOWN arithmetic effect is invalid")
                if "operand_role_order" in event and event["operand_role_order"] != [
                    "destination", "source_a", "source_b"
                ]:
                    raise Rejected(f"event[{index}] diagnostic UNKNOWN operand role order is invalid")
                source_registers = {registers["source_a"], registers["source_b"]}
                known = event["known_reaching_definitions"]
                if not isinstance(known, list):
                    raise Rejected(f"event[{index}] known reaching definitions are malformed")
                known_registers: list[str] = []
                for row in known:
                    if not isinstance(row, Mapping) or set(row) != {
                        "physical_register", "instruction_index",
                    }:
                        raise Rejected(f"event[{index}] known reaching definition is malformed")
                    register = _text(
                        row["physical_register"],
                        f"event[{index}].known_reaching_definition.physical_register",
                    )
                    definition = _integer(
                        row["instruction_index"],
                        f"event[{index}].known_reaching_definition.instruction_index",
                        nonnegative=True,
                    )
                    if register not in source_registers or definition >= instruction_index:
                        raise Rejected(f"event[{index}] known reaching definition is inconsistent")
                    known_registers.append(register)
                if known_registers != sorted(set(known_registers)):
                    raise Rejected(f"event[{index}] known reaching definitions are noncanonical")
                missing = event["missing_reaching_registers"]
                if (
                    not isinstance(missing, list)
                    or any(not isinstance(register, str) for register in missing)
                    or missing != sorted(set(missing))
                ):
                    raise Rejected(f"event[{index}] missing reaching registers are noncanonical")
                if set(known_registers).intersection(missing) or set(known_registers).union(missing) != source_registers:
                    raise Rejected(f"event[{index}] partial reaching-definition partition is inconsistent")
        elif status == "CAPTURED":
            required_machine = {
                "hook_id", "status", "pcode_token", "emitted_offset",
                "instruction_index", "opcode_enum", "ppc_word", "ppc_bytes",
                "mnemonic", "registers", "reaching_definitions", "owner_joins",
            }
            if not required_machine.issubset(fields) or "reason" in fields:
                raise Rejected(f"event[{index}] captured machine emission is incomplete")
            token = _text(event["pcode_token"], f"event[{index}].pcode_token")
            match = PCODE_TOKEN_RE.fullmatch(token)
            if match is None or match.group("session") != context["session_id"]:
                raise Rejected(f"event[{index}] PCode token provenance is invalid")
            emitted_offset = _integer(event["emitted_offset"], f"event[{index}].emitted_offset", nonnegative=True)
            instruction_index = _integer(event["instruction_index"], f"event[{index}].instruction_index", nonnegative=True)
            if emitted_offset % 4 or instruction_index != emitted_offset // 4:
                raise Rejected(f"event[{index}] machine instruction index is invalid")
            opcode_enum = _integer(event["opcode_enum"], f"event[{index}].opcode_enum", nonnegative=True)
            word = _integer(event["ppc_word"], f"event[{index}].ppc_word", nonnegative=True)
            if opcode_enum > 0x1D4 or word > 0xFFFFFFFF:
                raise Rejected(f"event[{index}] machine opcode is outside bounds")
            ppc_bytes = _text(event["ppc_bytes"], f"event[{index}].ppc_bytes")
            if re.fullmatch(r"[0-9a-f]{8}", ppc_bytes) is None or int(ppc_bytes, 16) != word:
                raise Rejected(f"event[{index}] PPC bytes/word mismatch")
            if event["mnemonic"] not in {"addi", *[row[0] for row in MachineEmissionDecoder._D_MEMORY.values()], "psq_l", "psq_st", "psq_lx", "psq_stx", "bl", "fneg", "fmuls", "ps_mul"}:
                raise Rejected(f"event[{index}] machine mnemonic is unsupported")
            registers = event["registers"]
            if not isinstance(registers, Mapping) or (not registers and event["mnemonic"] != "bl"):
                raise Rejected(f"event[{index}] machine registers are malformed")
            for role, register in registers.items():
                if role not in {"destination", "source", "source_a", "source_b", "data", "base", "index"} or not isinstance(register, str) or re.fullmatch(r"[rf](?:[0-9]|[12][0-9]|3[01])", register) is None:
                    raise Rejected(f"event[{index}] machine register operand is malformed")
            if "operand_role_order" in event:
                expected_role_order = {
                    "fneg": ["destination", "source"],
                    "fmuls": ["destination", "source_a", "source_b"],
                    "ps_mul": ["destination", "source_a", "source_b"],
                }.get(str(event["mnemonic"]))
                if expected_role_order is None or event["operand_role_order"] != expected_role_order:
                    raise Rejected(f"event[{index}] machine operand role order is invalid")
            reaching = event["reaching_definitions"]
            if not isinstance(reaching, list) or any(not isinstance(value, int) or isinstance(value, bool) or value < 0 or value >= instruction_index for value in reaching) or reaching != sorted(set(reaching)):
                raise Rejected(f"event[{index}] reaching definitions are malformed")
            memory_fields = {"memory_op", "memory_width", "effective_stack_offset"} & fields
            if memory_fields and memory_fields != {"memory_op", "memory_width", "effective_stack_offset"}:
                raise Rejected(f"event[{index}] machine memory effect is incomplete")
            if memory_fields:
                if event["memory_op"] not in {"load", "store"} or event["memory_width"] not in {1, 2, 4, 8}:
                    raise Rejected(f"event[{index}] machine memory effect is invalid")
                _integer(event["effective_stack_offset"], f"event[{index}].effective_stack_offset")
            if "immediate" in fields:
                _integer(event["immediate"], f"event[{index}].immediate")
            if "address_definition" in fields:
                definition = event["address_definition"]
                if not isinstance(definition, Mapping) or set(definition) != {"register", "stack_offset"} or definition["register"] not in registers.values():
                    raise Rejected(f"event[{index}] address definition is malformed")
                _integer(definition["stack_offset"], f"event[{index}].address_definition.stack_offset")
            arithmetic_fields = {"arithmetic_op", "arithmetic_type"} & fields
            arithmetic_mnemonics = {"fmuls", "ps_mul"}
            if event["mnemonic"] in arithmetic_mnemonics:
                if arithmetic_fields != {"arithmetic_op", "arithmetic_type"}:
                    raise Rejected(f"event[{index}] machine arithmetic effect is incomplete")
                expected_type = "f32" if event["mnemonic"] == "fmuls" else "paired-single"
                if event["arithmetic_op"] != "multiply" or event["arithmetic_type"] != expected_type:
                    raise Rejected(f"event[{index}] machine arithmetic effect is invalid")
                if set(registers) != {"destination", "source_a", "source_b"}:
                    raise Rejected(f"event[{index}] machine arithmetic operands are malformed")
            elif arithmetic_fields:
                raise Rejected(f"event[{index}] non-arithmetic machine event carries arithmetic fields")
            joins = event["owner_joins"]
            if not isinstance(joins, list):
                raise Rejected(f"event[{index}] owner joins are malformed")
            join_registers: set[str] = set()
            join_tokens: set[str] = set()
            for join in joins:
                if not isinstance(join, Mapping) or set(join) != {"physical_register", "object_token", "vreg_id"}:
                    raise Rejected(f"event[{index}] owner join is malformed")
                _token_parts(join["object_token"], f"event[{index}].owner_join.object_token", context["session_id"])
                vreg_id = _validate_vreg(join["vreg_id"], f"event[{index}].owner_join.vreg_id")
                physical_register = _text(join["physical_register"], f"event[{index}].owner_join.physical_register")
                if physical_register not in registers.values():
                    raise Rejected(f"event[{index}] owner join references an absent machine register")
                if physical_register.startswith("r") != vreg_id.startswith("r"):
                    raise Rejected(f"event[{index}] owner join register bank is inconsistent")
                if physical_register in join_registers or join["object_token"] in join_tokens:
                    raise Rejected(f"event[{index}] owner join is ambiguous")
                join_registers.add(physical_register)
                join_tokens.add(str(join["object_token"]))
            physical_joins = event.get("physical_owner_joins", [])
            if not isinstance(physical_joins, list):
                raise Rejected(f"event[{index}] physical owner joins are malformed")
            physical_join_registers: set[str] = set()
            physical_join_tokens: set[str] = set()
            for join in physical_joins:
                if not isinstance(join, Mapping) or set(join) != {"physical_register", "object_token"}:
                    raise Rejected(f"event[{index}] physical owner join is malformed")
                _token_parts(join["object_token"], f"event[{index}].physical_owner_join.object_token", context["session_id"])
                physical_register = _text(join["physical_register"], f"event[{index}].physical_owner_join.physical_register")
                if physical_register not in registers.values():
                    raise Rejected(f"event[{index}] physical owner join references an absent machine register")
                if physical_register in physical_join_registers or join["object_token"] in physical_join_tokens:
                    raise Rejected(f"event[{index}] physical owner join is ambiguous")
                physical_join_registers.add(physical_register)
                physical_join_tokens.add(str(join["object_token"]))
        else:
            raise Rejected(f"event[{index}] machine status is invalid")
    if event_kind == "lane_unknown":
        reason = _text(event["reason"], f"event[{index}].reason")
        if event["lane"] not in _LANES:
            raise Rejected(f"event[{index}] unknown lane is invalid")
    if event_kind == "function_exit":
        if not isinstance(event["exit_code"], int) or isinstance(event["exit_code"], bool):
            raise Rejected(f"event[{index}].exit_code is not type-canonical")
        code = _integer(event["exit_code"], f"event[{index}].exit_code")
        if code != 0:
            raise Rejected(f"event[{index}] compiler exit code is non-zero")


def _validate_inventory(inventory: Any, session_id: str) -> set[str]:
    if not isinstance(inventory, Mapping) or set(inventory) != {"status", "locals", "arguments"}:
        raise Rejected("inventory shape mismatch")
    if inventory["status"] not in {"COMPLETE", "UNKNOWN"}:
        raise Rejected("inventory status is invalid")
    tokens: set[str] = set()
    ownership_unknown = False
    for key, prefix in (("locals", "local-"), ("arguments", "argument-")):
        rows = inventory[key]
        if not isinstance(rows, list):
            raise Rejected(f"inventory.{key} is not a list")
        for index, row in enumerate(rows):
            if not isinstance(row, Mapping) or not set(row).issubset({"token", "kind", "name", "ownership"}):
                raise Rejected(f"inventory.{key}[{index}] has unsupported fields")
            token = row.get("token")
            if not isinstance(token, str) or not TOKEN_RE.fullmatch(token) or not token.startswith(prefix) or token in tokens:
                raise Rejected(f"inventory.{key}[{index}] token is invalid or duplicated")
            _token_parts(token, f"inventory.{key}[{index}].token", session_id, prefix[:-1])
            if token != f"{prefix}{session_id}-{index:06d}":
                raise Rejected(f"inventory.{key}[{index}] token ordinal is not canonical")
            tokens.add(token)
            if row.get("kind") != key[:-1]:
                raise Rejected(f"inventory.{key}[{index}] kind is invalid")
            if "name" in row and (not isinstance(row["name"], str) or not row["name"]):
                raise Rejected(f"inventory.{key}[{index}] name is invalid")
            ownership = row.get("ownership")
            ownership_fields = {
                "status", "reason", "mode", "evidence_event_ids", "vreg_id",
                "physical_reg", "bank", "stack_home",
            }
            if not isinstance(ownership, Mapping) or not set(ownership).issubset(ownership_fields):
                raise Rejected(f"inventory.{key}[{index}] ownership is invalid")
            if ownership.get("status") == "EXACT":
                mode = ownership.get("mode")
                if mode is None:
                    if set(ownership) != {"status", "vreg_id", "bank"}:
                        raise Rejected(f"inventory.{key}[{index}] legacy exact ownership is incomplete")
                    vreg = _validate_vreg(ownership["vreg_id"], "inventory vreg_id")
                    bank = ownership["bank"]
                    if bank not in {"GPR", "FPR"} or not vreg.startswith("r" if bank == "GPR" else "f"):
                        raise Rejected(f"inventory.{key}[{index}] bank/vreg mismatch")
                else:
                    evidence_ids = ownership.get("evidence_event_ids")
                    if not isinstance(evidence_ids, list) or not evidence_ids or evidence_ids != list(dict.fromkeys(evidence_ids)):
                        raise Rejected(f"inventory.{key}[{index}] ownership evidence ids are invalid")
                    for event_id in evidence_ids:
                        if not isinstance(event_id, str) or re.fullmatch(rf"{re.escape(session_id)}-e[0-9]{{6}}", event_id) is None:
                            raise Rejected(f"inventory.{key}[{index}] ownership evidence id is cross-session or malformed")
                    if mode == "virtual_register":
                        expected = {"status", "mode", "evidence_event_ids", "vreg_id", "bank"}
                        if set(ownership) != expected:
                            raise Rejected(f"inventory.{key}[{index}] virtual ownership is incomplete")
                        vreg = _validate_vreg(ownership["vreg_id"], "inventory vreg_id")
                        bank = ownership["bank"]
                        if bank not in {"GPR", "FPR"} or not vreg.startswith("r" if bank == "GPR" else "f"):
                            raise Rejected(f"inventory.{key}[{index}] virtual bank/vreg mismatch")
                    elif mode == "physical_register":
                        expected = {"status", "mode", "evidence_event_ids", "physical_reg", "bank"}
                        if set(ownership) != expected or len(evidence_ids) != 1:
                            raise Rejected(f"inventory.{key}[{index}] physical ownership is incomplete")
                        if not isinstance(ownership["physical_reg"], int) or isinstance(ownership["physical_reg"], bool) or not 0 <= ownership["physical_reg"] <= 31:
                            raise Rejected(f"inventory.{key}[{index}] physical register is invalid")
                        if ownership["bank"] not in {"GPR", "FPR"}:
                            raise Rejected(f"inventory.{key}[{index}] physical bank is invalid")
                    elif mode == "stack_home":
                        expected = {"status", "mode", "evidence_event_ids", "stack_home"}
                        home = ownership.get("stack_home")
                        if set(ownership) != expected or len(evidence_ids) < 2 or len(evidence_ids) % 2:
                            raise Rejected(f"inventory.{key}[{index}] stack ownership is incomplete")
                        if not isinstance(home, Mapping) or set(home) != {"base", "offset"} or home.get("base") != "r1":
                            raise Rejected(f"inventory.{key}[{index}] stack home is malformed")
                        if not isinstance(home.get("offset"), int) or isinstance(home.get("offset"), bool):
                            raise Rejected(f"inventory.{key}[{index}] stack offset is not type-canonical")
                    else:
                        raise Rejected(f"inventory.{key}[{index}] ownership mode is unsupported")
            elif ownership.get("status") != "UNKNOWN":
                raise Rejected(f"inventory.{key}[{index}] ownership status is invalid")
            elif set(ownership) != {"status", "reason"} or not isinstance(ownership["reason"], str) or not ownership["reason"]:
                raise Rejected(f"inventory.{key}[{index}] UNKNOWN ownership lacks a reason")
            else:
                ownership_unknown = True
    if inventory["status"] == "COMPLETE" and ownership_unknown:
        raise Rejected("COMPLETE inventory contains UNKNOWN ownership")
    return tokens


def _validate_chronology(
    events: Sequence[Mapping[str, Any]],
    context: Mapping[str, Any],
    inventory: Mapping[str, Any],
    inventory_tokens: set[str],
) -> None:
    """Validate semantic event edges after structural event validation."""

    if not events:
        raise Rejected("capture event chronology is empty")
    profile_hooks = _hooks_for_compiler(str(context["compiler"]["sha256"]))
    profile_write_ids = tuple(
        str(row["id"]) for row in profile_hooks if row["role"] == "object_stack_write"
    )
    profile_pcode_ids = _pcode_stage_hook_ids(profile_hooks)
    profile_machine_ids = tuple(
        str(row["id"]) for row in profile_hooks if row["role"] == "machine_emit"
    )
    kinds = [str(event["event_kind"]) for event in events]
    if kinds[0] != "function_entry" or kinds[-1] != "function_exit":
        raise Rejected("capture chronology must begin at function entry and end at function exit")
    if kinds.count("function_entry") != 1 or kinds.count("function_exit") != 1:
        raise Rejected("capture chronology has duplicate function edges")
    positions = {kind: [index for index, value in enumerate(kinds) if value == kind] for kind in _EVENT_KINDS}
    entry = positions["function_entry"][0]
    exit_index = positions["function_exit"][0]
    if entry >= exit_index:
        raise Rejected("function exit precedes function entry")
    for allocator_kind in ("regalloc_assignment", "physical_reg_assignment"):
        if any(index <= entry or index >= exit_index for index in positions[allocator_kind]):
            raise Rejected(f"{allocator_kind} chronology is outside the target function")

    def _one(kind: str) -> Mapping[str, Any] | None:
        rows = positions[kind]
        if len(rows) > 1:
            raise Rejected(f"duplicate {kind} chronology edge")
        return events[rows[0]] if rows else None

    compiler_list = _one("compiler_list")
    allocation_pre = _one("numeric_stack_alloc_pre")
    allocation_post = _one("numeric_stack_alloc_post")
    if compiler_list is None or allocation_pre is None or allocation_post is None:
        raise Rejected("stack allocation chronology is incomplete")
    if not positions["compiler_list"][0] < positions["numeric_stack_alloc_pre"][0] < positions["numeric_stack_alloc_post"][0]:
        raise Rejected("stack allocation chronology is reversed")
    expected_locals = [row["token"] for row in inventory["locals"]]
    expected_arguments = [row["token"] for row in inventory["arguments"]]
    for label, row in (("compiler_list", compiler_list), ("allocation_pre", allocation_pre), ("allocation_post", allocation_post)):
        if row["locals"] != expected_locals or row["arguments"] != expected_arguments:
            raise Rejected(f"{label} token ledger does not match inventory")

    # A compiler hook address is a shared code path, not a per-object event
    # identity.  Larger functions (including CapSelectMasuCom) can execute
    # the same Object-write path repeatedly.  Retain each ordered pair and
    # validate the pair chronology below instead of collapsing by hook ID.
    pre_by_hook: dict[str, list[tuple[int, Mapping[str, Any]]]] = {}
    post_by_hook: dict[str, list[tuple[int, Mapping[str, Any]]]] = {}
    for index, event in enumerate(events):
        kind = event["event_kind"]
        if kind == "object_stack_write_pre":
            hook = str(event["hook_id"])
            pre_by_hook.setdefault(hook, []).append((index, event))
        elif kind == "object_stack_write_post":
            hook = str(event["hook_id"])
            post_by_hook.setdefault(hook, []).append((index, event))
    present_write_order = [
        str(event["hook_id"])
        for event in events
        if event["event_kind"] == "object_stack_write_pre"
    ]
    canonical_write_order = [hook for hook in profile_write_ids if hook in set(present_write_order)]
    present_write_order = list(dict.fromkeys(present_write_order))
    if present_write_order != canonical_write_order:
        raise Rejected("object write hook chronology is reversed")
    present_post_order = [
        str(event["hook_id"])
        for event in events
        if event["event_kind"] == "object_stack_write_post"
    ]
    canonical_post_order = [hook for hook in profile_write_ids if hook in set(present_post_order)]
    present_post_order = list(dict.fromkeys(present_post_order))
    if present_post_order != canonical_post_order:
        raise Rejected("object write post chronology is reversed")
    stack_unknown = [event for event in events if event["lane"] == "stack" and event["event_kind"] == "lane_unknown"]
    # The three authenticated write hooks are alternate compiler paths, not
    # three mandatory per-function events.  A normal target-function exit
    # proves which installed sites executed.  Require every observed write to
    # be balanced and ordered below, but do not relabel an unexecuted path as
    # missing evidence.  An UNKNOWN marker is justified only by an actually
    # unbalanced observed path.
    balanced_counts = all(
        len(pre_by_hook[hook]) == len(post_by_hook[hook])
        for hook in set(pre_by_hook) & set(post_by_hook)
    )
    if stack_unknown and set(pre_by_hook) == set(post_by_hook) and balanced_counts:
        raise Rejected("stack UNKNOWN marker is not justified by missing edges")
    if set(pre_by_hook) != set(post_by_hook):
        if not any(event["lane"] == "stack" and event["event_kind"] == "lane_unknown" for event in events):
            raise Rejected("stack write pre/post edges are unbalanced")
    for hook in set(pre_by_hook) & set(post_by_hook):
        pre_rows = pre_by_hook[hook]
        post_rows = post_by_hook[hook]
        if len(pre_rows) != len(post_rows):
            if not any(event["lane"] == "stack" and event["event_kind"] == "lane_unknown" for event in events):
                raise Rejected(f"stack write pre/post edges are unbalanced for {hook}")
        for pair_index, ((before_index, before), (after_index, after)) in enumerate(zip(pre_rows, post_rows)):
            if before_index >= after_index:
                raise Rejected(f"write edge {hook}[{pair_index}] is reversed")
            if before["object_token"] != after["object_token"] or before["target_slot"] != after["target_slot"]:
                raise Rejected(f"write edge {hook}[{pair_index}] changes token or slot")
            if before["object_token"] != "UNKNOWN" and before["object_token"] not in inventory_tokens:
                raise Rejected(f"write edge {hook}[{pair_index}] references an orphan token")

    pcode_rows = [
        event for event in events
        if event["event_kind"] == "pcode_capture"
    ]
    pcode_color_rows = [event for event in pcode_rows if event.get("confirmed") is True]
    pcode_stage_rows = [event for event in pcode_rows if event.get("confirmed") is not True]
    pcode_by_hook: dict[str, Mapping[str, Any]] = {}
    pcode_order: list[int] = []
    for event in pcode_stage_rows:
        hook = str(event["hook_id"])
        if hook in pcode_by_hook:
            raise Rejected(f"duplicate PCode edge for {hook}")
        pcode_by_hook[hook] = event
        pcode_order.append(int(event["sequence"]))
    color_keys: set[tuple[str, int]] = set()
    for event in pcode_color_rows:
        key = (str(event["pcode_token"]), int(event["operand_ordinal"]))
        if key in color_keys:
            raise Rejected("duplicate PCode color chronology edge")
        color_keys.add(key)
    missing_pcode = set(profile_pcode_ids) - set(pcode_by_hook)
    pcode_unknown = [event for event in events if event["lane"] == "pcode" and event["event_kind"] == "lane_unknown"]
    regalloc_present = any(event["event_kind"] == "regalloc_assignment" for event in events)
    physical_present = any(event["event_kind"] == "physical_reg_assignment" for event in events)
    gc27_profile = str(context["compiler"]["sha256"]) == GC27_COMPILER_SHA256
    missing_allocator_edges = not physical_present or (not gc27_profile and not regalloc_present)
    machine_rows = [event for event in events if event["event_kind"] == "machine_emission"]
    machine_required = bool(profile_machine_ids)
    missing_machine = machine_required and not machine_rows
    if machine_rows and not machine_required:
        raise Rejected("machine-emission chronology is forbidden by the compiler profile")
    if any(str(event["hook_id"]) not in profile_machine_ids for event in machine_rows):
        raise Rejected("machine-emission chronology uses a cross-profile hook")
    if missing_machine:
        raise Rejected("machine-emission chronology is required by the compiler profile")
    if missing_pcode and not pcode_unknown:
        raise Rejected("PCode chronology is incomplete without UNKNOWN")
    if missing_allocator_edges and not pcode_unknown:
        raise Rejected("PCode chronology is incomplete without UNKNOWN")
    if pcode_unknown and not missing_pcode and not missing_machine and len(pcode_stage_rows) == len(profile_pcode_ids) and (gc27_profile or regalloc_present) and physical_present:
        raise Rejected("PCode UNKNOWN marker is not justified by missing edges")
    present_order = [hook for hook in profile_pcode_ids if hook in pcode_by_hook]
    if [event["hook_id"] for event in pcode_stage_rows] != present_order:
        raise Rejected("PCode hook chronology is reversed")
    if pcode_rows and any(event["status"] == "UNKNOWN" for event in pcode_rows):
        if not any(event["lane"] == "pcode" and event["event_kind"] == "lane_unknown" for event in events):
            # An individual UNKNOWN PCode record carries its own reason and is
            # sufficient; no lane marker is needed for a complete hook set.
            pass

    captured_machine = [event for event in machine_rows if event["status"] == "CAPTURED"]
    offsets = [int(event["emitted_offset"]) for event in captured_machine]
    if offsets != sorted(set(offsets)):
        raise Rejected("machine-emission offsets are duplicated or reversed")
    machine_indices = {int(event["instruction_index"]) for event in captured_machine}
    machine_tokens: dict[str, int] = {}
    for event in captured_machine:
        token = str(event["pcode_token"])
        offset = int(event["emitted_offset"])
        if token in machine_tokens and machine_tokens[token] != offset:
            raise Rejected("PCode token maps to multiple machine offsets")
        machine_tokens[token] = offset
        if any(int(definition) not in machine_indices for definition in event["reaching_definitions"]):
            raise Rejected("machine lifetime edge references an absent emission")

    exact_vregs: dict[str, str] = {}
    exact_objects: dict[str, str] = {}
    for event in events:
        if event["event_kind"] != "regalloc_assignment" or event["status"] != "EXACT":
            continue
        token = str(event["object_token"])
        vreg = str(event["vreg_id"])
        if token not in inventory_tokens:
            raise Rejected("regalloc assignment references an orphan token")
        if token in exact_objects or vreg in exact_vregs:
            raise Rejected("duplicate token or virtual-register ownership claim")
        exact_objects[token] = vreg
        exact_vregs[vreg] = token

    physical_exact_objects: dict[str, tuple[str, int]] = {}
    physical_exact_regs: dict[tuple[str, int], str] = {}
    for event in events:
        if event["event_kind"] != "physical_reg_assignment" or event["status"] != "EXACT":
            continue
        token = str(event["object_token"])
        bank = str(event["bank"])
        physical_reg = int(event["physical_reg"])
        if token not in inventory_tokens:
            raise Rejected("physical assignment references an orphan token")
        if token in physical_exact_objects:
            raise Rejected("duplicate physical Object ownership claim")
        physical_exact_objects[token] = (bank, physical_reg)
        key = (bank, physical_reg)
        if key in physical_exact_regs:
            raise Rejected("duplicate physical-register ownership claim")
        physical_exact_regs[key] = token

    events_by_id = {str(event["event_id"]): event for event in events}
    allocation_post_sequence = int(allocation_post["sequence"])
    for container in ("locals", "arguments"):
        for row in inventory[container]:
            token = str(row["token"])
            ownership = row["ownership"]
            if ownership["status"] != "EXACT":
                if token in exact_objects:
                    raise Rejected("UNKNOWN inventory ownership has an exact event claim")
                continue
            mode = ownership.get("mode")
            if mode in {None, "virtual_register"}:
                vreg = str(ownership["vreg_id"])
                if exact_objects.get(token) != vreg:
                    raise Rejected("inventory ownership is not backed by a same-session regalloc edge")
            elif mode == "physical_register":
                physical = (str(ownership["bank"]), int(ownership["physical_reg"]))
                if physical_exact_objects.get(token) != physical:
                    raise Rejected("inventory physical ownership lacks a same-session assignment")
                evidence_ids = list(ownership["evidence_event_ids"])
                if len(evidence_ids) != 1:
                    raise Rejected("inventory physical ownership has ambiguous evidence")
                evidence = events_by_id.get(str(evidence_ids[0]))
                if evidence is None or evidence["event_kind"] != "physical_reg_assignment" or evidence.get("object_token") != token or evidence.get("status") != "EXACT":
                    raise Rejected("inventory physical evidence id is not an exact same-session edge")
            elif mode == "stack_home":
                evidence_ids = list(ownership["evidence_event_ids"])
                evidence = [events_by_id.get(str(event_id)) for event_id in evidence_ids]
                if any(event is None for event in evidence):
                    raise Rejected("inventory stack evidence id is absent")
                assert all(event is not None for event in evidence)
                offsets: set[int] = set()
                for pair_index in range(0, len(evidence), 2):
                    before = evidence[pair_index]
                    after = evidence[pair_index + 1]
                    assert before is not None and after is not None
                    if before["event_kind"] != "object_stack_write_pre" or after["event_kind"] != "object_stack_write_post":
                        raise Rejected("inventory stack evidence is not a pre/post pair")
                    if int(before["sequence"]) <= allocation_post_sequence or int(before["sequence"]) >= int(after["sequence"]):
                        raise Rejected("inventory stack evidence is outside the final allocation epoch")
                    if before.get("object_token") != token or after.get("object_token") != token or before["hook_id"] != after["hook_id"] or before["target_slot"] != after["target_slot"]:
                        raise Rejected("inventory stack evidence changes token, hook, or slot")
                    offsets.add(int(after["target_slot"]))
                if offsets != {int(ownership["stack_home"]["offset"])}:
                    raise Rejected("inventory stack evidence does not match its claimed home")
            else:  # Structural inventory validation should already reject this.
                raise Rejected("inventory ownership mode is unsupported")

    for event in captured_machine:
        machine_registers = set(event["registers"].values())
        for join in event["owner_joins"]:
            register = str(join["physical_register"])
            token = str(join["object_token"])
            vreg = str(join["vreg_id"])
            if register not in machine_registers:
                raise Rejected("machine owner join references an absent operand")
            if token not in inventory_tokens or exact_objects.get(token) != vreg:
                raise Rejected("machine owner join lacks an exact Object/vreg edge")
            bank = "GPR" if register.startswith("r") else "FPR"
            physical = (bank, int(register[1:]))
            if physical_exact_objects.get(token) != physical:
                raise Rejected("machine owner join lacks an exact physical-register edge")
        for join in event.get("physical_owner_joins", []):
            register = str(join["physical_register"])
            token = str(join["object_token"])
            if register not in machine_registers:
                raise Rejected("machine physical owner join references an absent operand")
            bank = "GPR" if register.startswith("r") else "FPR"
            physical = (bank, int(register[1:]))
            if physical_exact_objects.get(token) != physical:
                raise Rejected("machine physical owner join lacks an exact same-session assignment")


def _validate_embedded_frontend_chronology(
    value: Any,
    context: Mapping[str, Any],
) -> dict[str, Any]:
    """Validate frontend chronology emitted by the same combined session."""

    if not isinstance(value, Mapping):
        raise Rejected("embedded frontend chronology must be an object")
    status = value.get("status")
    if status == "CAPTURED":
        if set(value) != {"status", "packet"} or not isinstance(value.get("packet"), Mapping):
            raise Rejected("embedded frontend chronology capture shape mismatch")
        packet = _frontend_chronology.validate_packet(value["packet"])
        provenance = packet["provenance"]
        if packet["function"] != context["function"]:
            raise Rejected("embedded frontend chronology function mismatch")
        if provenance["session_id"] != context["session_id"]:
            raise Rejected("embedded frontend chronology session mismatch")
        if provenance["source_sha256"] != context["source"]["sha256"] or provenance["compiler_sha256"] != context["compiler"]["sha256"]:
            raise Rejected("embedded frontend chronology provenance mismatch")
        trace = context.get("frontend_trace_sha256")
        if trace is None or provenance["trace_sha256"] != trace:
            raise Rejected("embedded frontend chronology trace mismatch")
        return {"status": "CAPTURED", "packet": packet}
    if status == "UNKNOWN":
        if set(value) != {"status", "reason"} or value.get("reason") != "incomplete frontend chronology":
            raise Rejected("embedded frontend chronology UNKNOWN shape mismatch")
        return {"status": "UNKNOWN", "reason": "incomplete frontend chronology"}
    raise Rejected("embedded frontend chronology status is unsupported")


def validate_envelope(
    envelope_path: Path | str,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    request_path: Path | str | None = None,
    post_capture_analysis: bool = False,
) -> dict[str, Any]:
    if external_trust_root is not None and trust_root is not None:
        raise Rejected("conflicting external trust root arguments")
    root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
    if root is None:
        raise Rejected("external trust root is required for envelope authenticity")
    path = _canonical_path(envelope_path, "envelope")
    try:
        envelope = strict_json_loads(path.read_text(encoding="utf-8"), "envelope")
    except OSError as exc:
        raise Rejected(f"cannot read envelope: {exc}") from exc
    if not isinstance(envelope, Mapping):
        raise Rejected("envelope must be an object")
    expected_keys = {
        "schema", "tool_version", "status", "diagnostic_only", "board_admission", "exactness_claim", "authority_advanced", "context", "authority", "outputs", "hooks", "events", "event_count", "lanes", "inventory", "unknown", "limitations", "envelope_sha256"
    }
    envelope_keys = set(envelope)
    if envelope_keys != expected_keys and envelope_keys != expected_keys | {"frontend_chronology"}:
        raise Rejected("envelope contains unsupported or missing fields")
    digest = envelope.get("envelope_sha256")
    unsigned = {key: value for key, value in envelope.items() if key != "envelope_sha256"}
    if digest != canonical_hash(unsigned):
        raise Rejected("envelope self-digest mismatch")
    if envelope["schema"] != SCHEMA or envelope["tool_version"] != TOOL_VERSION or envelope["status"] != "CAPTURED_UNKNOWN_OWNERSHIP":
        raise Rejected("envelope schema/status mismatch")
    if envelope["diagnostic_only"] is not True or envelope["board_admission"] is not False or envelope["exactness_claim"] is not False or envelope["authority_advanced"] is not False:
        raise Rejected("envelope policy mismatch")
    context = envelope["context"]
    base_context_keys = {"session_id", "process_id", "function", "function_sha256", "argv", "cwd", "source", "compiler", "wrapper", "debugger", "transport", "request", "authority"}
    allowed_context_keys = {
        frozenset(base_context_keys),
        frozenset(base_context_keys | {"execution"}),
        frozenset(base_context_keys | {"frontend_trace_sha256"}),
        frozenset(base_context_keys | {"frontend_trace_sha256", "execution"}),
    }
    if not isinstance(context, Mapping) or frozenset(context) not in allowed_context_keys:
        raise Rejected("envelope context shape mismatch")
    session_id = _safe_session_id(context["session_id"])
    if not isinstance(context["process_id"], int) or isinstance(context["process_id"], bool):
        raise Rejected("envelope process_id is not type-canonical")
    process_id = _integer(context["process_id"], "envelope process_id", nonnegative=True)
    if process_id == 0:
        raise Rejected("envelope process_id is zero")
    function = _text(context["function"], "envelope function")
    if not _authorized_board_function(function, context["source"]):
        raise Rejected("envelope function is unsupported")
    _canonical_argv(context["argv"], "envelope argv")
    _canonical_cwd(context["cwd"], must_exist=False)
    for name in ("source", "compiler", *_TOOL_IDENTITY_KEYS, "authority"):
        _path_descriptor(
            context[name],
            f"envelope context {name}",
            must_exist=False,
            verify_live=not (
                post_capture_analysis and name in {"debugger", "transport"}
            ),
        )
    _path_descriptor(envelope["authority"], "envelope authority", must_exist=False)
    if context["authority"] != envelope["authority"]:
        raise Rejected("envelope authority descriptor is not shared with context")
    compiler_sha256 = str(context["compiler"]["sha256"]).lower()
    has_frontend = "frontend_chronology" in envelope
    if compiler_sha256 == GC26_COMPILER_SHA256:
        if not has_frontend or "frontend_trace_sha256" not in context:
            raise Rejected("GC/2.6 envelope is missing same-session frontend chronology")
        _digest(context["frontend_trace_sha256"], "envelope frontend trace")
        _validate_embedded_frontend_chronology(envelope["frontend_chronology"], context)
    elif has_frontend or "frontend_trace_sha256" in context:
        raise Rejected("frontend chronology is not valid for this compiler profile")
    request_descriptor = context["request"]
    if not isinstance(request_descriptor, Mapping) or set(request_descriptor) != {"path", "size", "sha256"}:
        raise Rejected("envelope request descriptor is malformed")
    request_path_value = request_path if request_path is not None else getattr(root, "request_path", None)
    if request_path_value is None:
        raise Rejected("external trust root.request anchor is missing")
    authenticated_request = authenticate_request(
        request_path_value,
        external_trust_root=root,
        post_capture_analysis=post_capture_analysis,
    )
    authenticated_request_path = Path(authenticated_request["request_path"])
    if str(_canonical_path(request_descriptor["path"], "envelope context request.path")) != str(authenticated_request_path):
        raise Rejected("envelope request path is not externally bound")
    if _integer(request_descriptor["size"], "envelope request.size", nonnegative=True) != authenticated_request_path.stat().st_size:
        raise Rejected("envelope request size is not externally bound")
    if _digest(request_descriptor["sha256"], "envelope request.sha256") != authenticated_request["request_sha256"]:
        raise Rejected("envelope request digest is not externally bound")
    expected_context = authenticated_request["request"]
    if context["session_id"] != expected_context["session_id"] or context["function"] != expected_context["function"] or context["function_sha256"] != expected_context["function_sha256"] or context["argv"] != expected_context["argv"] or context["cwd"] != expected_context["cwd"]:
        raise Rejected("envelope context does not match authenticated request")
    for name in ("source", "compiler", *_TOOL_IDENTITY_KEYS, "authority"):
        if context[name] != expected_context[name]:
            raise Rejected(f"envelope context {name} does not match authenticated request")
    execution = context.get("execution")
    if execution is not None:
        if not isinstance(execution, Mapping) or set(execution) != {"mode", "argv", "request_argv_sha256", "wrapper_bypassed"}:
            raise Rejected("envelope execution provenance is malformed")
        mode = _text(execution["mode"], "envelope execution mode")
        executed_argv = _canonical_argv(execution["argv"], "envelope executed argv")
        request_argv_digest = _digest(execution["request_argv_sha256"], "envelope request argv digest")
        if request_argv_digest != canonical_hash({"argv": list(expected_context["argv"])}):
            raise Rejected("envelope execution request argv digest mismatch")
        if mode == "authenticated_direct_compiler":
            if execution["wrapper_bypassed"] is not True:
                raise Rejected("direct compiler execution did not record wrapper bypass")
            if executed_argv != list(expected_context["argv"])[1:]:
                raise Rejected("direct compiler execution argv is not the authenticated derivation")
            _authenticated_direct_compiler_argv(expected_context)
        elif mode == "wrapper_memexec":
            if execution["wrapper_bypassed"] is not False or executed_argv != list(expected_context["argv"]):
                raise Rejected("wrapper execution provenance does not match authenticated argv")
        else:
            raise Rejected("envelope execution mode is unsupported")
    _validate_hook_rows(
        envelope["hooks"],
        "envelope.hooks",
        compiler_sha256=str(context["compiler"]["sha256"]),
    )
    events = envelope["events"]
    if not isinstance(events, list) or envelope["event_count"] != len(events):
        raise Rejected("envelope event count mismatch")
    for index, event in enumerate(events):
        _validate_event(event, index, context)
    lanes = envelope["lanes"]
    if not isinstance(lanes, Mapping) or set(lanes) != set(_LANES):
        raise Rejected("envelope lanes shape mismatch")
    for lane in _LANES:
        lane_events = [event for event in events if event["lane"] == lane]
        descriptor = lanes[lane]
        if not isinstance(descriptor, Mapping) or set(descriptor) != {"event_count", "event_ids", "size_bytes", "sha256"}:
            raise Rejected(f"lane {lane} descriptor shape mismatch")
        data = canonical_lane_bytes(lane_events)
        if descriptor["event_count"] != len(lane_events) or descriptor["event_ids"] != [event["event_id"] for event in lane_events] or descriptor["size_bytes"] != len(data) or descriptor["sha256"] != hashlib.sha256(data).hexdigest():
            raise Rejected(f"lane {lane} hash mismatch")
    inventory_tokens = _validate_inventory(envelope["inventory"], session_id)
    _validate_chronology(events, context, envelope["inventory"], inventory_tokens)
    derived_unknown: set[str] = set()
    for key in ("locals", "arguments"):
        for row in envelope["inventory"][key]:
            ownership = row["ownership"]
            if ownership["status"] == "UNKNOWN":
                derived_unknown.add(str(ownership["reason"]))
    for event in events:
        if event["event_kind"] == "lane_unknown":
            derived_unknown.add(str(event["reason"]))
        elif event["event_kind"] in {"pcode_capture", "regalloc_assignment", "physical_reg_assignment", "machine_emission"} and event.get("status") == "UNKNOWN":
            derived_unknown.add(str(event["reason"]))
    if has_frontend and envelope["frontend_chronology"].get("status") == "UNKNOWN":
        derived_unknown.add(str(envelope["frontend_chronology"]["reason"]))
    if not isinstance(envelope["unknown"], list) or envelope["unknown"] != sorted(set(envelope["unknown"])) or not all(isinstance(item, str) and item in _KNOWN_UNKNOWN_REASONS for item in envelope["unknown"]):
        raise Rejected("envelope unknown list is invalid")
    if not derived_unknown.issubset(set(envelope["unknown"])):
        raise Rejected("envelope UNKNOWN ledger does not match incomplete evidence")
    if not isinstance(envelope["limitations"], list) or not all(isinstance(item, str) and item for item in envelope["limitations"]):
        raise Rejected("envelope limitations are invalid")
    outputs = envelope["outputs"]
    if not isinstance(outputs, Mapping) or set(outputs) != {"event_stream_stack", "event_stream_pcode"}:
        raise Rejected("envelope outputs shape mismatch")
    output_paths = authenticated_request["paths"]
    output_bytes: dict[str, bytes] = {}
    for key in ("event_stream_stack", "event_stream_pcode"):
        descriptor = outputs[key]
        if not isinstance(descriptor, Mapping) or set(descriptor) != {"path", "size", "sha256"}:
            raise Rejected(f"envelope output {key} descriptor is malformed")
        expected_path = output_paths[key]
        actual_path = _canonical_path(descriptor["path"], f"envelope output {key}.path")
        if str(actual_path) != str(expected_path):
            raise Rejected(f"envelope output {key} path is not request-bound")
        data = actual_path.read_bytes()
        if _integer(descriptor["size"], f"envelope output {key}.size", nonnegative=True) != len(data) or _digest(descriptor["sha256"], f"envelope output {key}.sha256") != hashlib.sha256(data).hexdigest():
            raise Rejected(f"envelope output {key} bytes do not match")
        output_bytes[key] = data
        stream_events = [event for event in events if event["lane"] == ("stack" if key.endswith("stack") else "pcode")]
        if data != canonical_lane_bytes(stream_events):
            raise Rejected(f"envelope output {key} stream does not match event ledger")
    envelope_anchor = _trust_root_descriptor(root, "envelope", required=True)
    if envelope_anchor is None or str(_canonical_path(envelope_path, "envelope")) != envelope_anchor["path"]:
        raise Rejected("external trust root.envelope path does not match envelope")
    for name in ("event_stream_stack", "event_stream_pcode"):
        anchor = _trust_root_descriptor(root, name, required=True)
        if anchor is None or outputs[name] != anchor:
            raise Rejected(f"external trust root.{name} does not match envelope output")
    output_dir = authenticated_request["paths"]["envelope"].parent
    expected_names = {"request.json", "stack.events.jsonl", "pcode.events.jsonl", "same-session.envelope.json"}
    extras = [entry.name for entry in output_dir.iterdir() if entry.name not in expected_names]
    if extras:
        raise Rejected(f"capture output directory contains extra or partial files: {sorted(extras)}")
    return dict(envelope)


_SOURCE_SPAN_FIELDS = {
    "object_token", "identity", "role", "byte_start", "byte_end",
    "line_start", "line_end", "text_sha256",
}
_SOURCE_SPAN_V2_FIELDS = _SOURCE_SPAN_FIELDS | {
    "dependency_id", "machine_instruction_indices",
}
_SOURCE_OBJECT_FIELDS = {
    "object_token", "identity", "ownership_mode", "object_type", "byte_size",
}
_SOURCE_SPAN_ROLES = {"declaration", "read", "write", "call_return", "evaluation"}
_SOURCE_OWNERSHIP_MODES = {"scalar_register", "stack_interval"}
_SOURCE_DEPENDENCY_RE = re.compile(r"[A-Za-z][A-Za-z0-9_.-]{0,127}")
_SOURCE_PLAN_OBJECT_FIELDS = {
    "identity", "ownership_mode", "object_type", "byte_size",
}
_SOURCE_PLAN_BINDING_FIELDS = {
    "identity", "role", "byte_start", "byte_end", "dependency_id",
    "machine_instruction_indices",
}
_SOURCE_TEMPLATE_REQUIRED_FIELDS = {
    "schema", "template_schema", "function", "function_sha256", "session_id",
    "source", "spans", "diagnostic_only", "board_admission", "exactness_claim",
    "authority_advanced", "unsealed",
}
_SOURCE_TEMPLATE_ALLOWED_FIELDS = _SOURCE_TEMPLATE_REQUIRED_FIELDS | {
    "capture_placeholders", "notes", "source_aliases", "source_anchor",
    "span_template", "stack_interval_provenance",
}
_SOURCE_TEMPLATE_SPAN_ALLOWED_FIELDS = _SOURCE_SPAN_FIELDS | {
    "ownership_mode", "stack_interval",
}


def seal_source_span_manifest(value: Mapping[str, Any]) -> dict[str, Any]:
    """Seal a source-span binding manifest without granting source authority."""

    result = dict(value)
    if result.get("schema") not in {SOURCE_SPAN_SCHEMA, SOURCE_SPAN_SCHEMA_V2}:
        raise Rejected("unsealed source span manifest schema is unsupported")
    if result.get("authority_advanced") is not False:
        raise Rejected("unsealed source span manifest may not advance authority")
    result.pop("manifest_sha256", None)
    result["manifest_sha256"] = canonical_hash(result)
    return result


def seal_source_span_file(input_path: Path | str, output_path: Path | str) -> dict[str, Any]:
    source = _canonical_path(input_path, "unsealed source span manifest")
    raw = strict_json_loads(source.read_text(encoding="utf-8"), "unsealed source span manifest")
    if not isinstance(raw, Mapping) or "manifest_sha256" in raw:
        raise Rejected("unsealed source span manifest must be an object without manifest_sha256")
    sealed = seal_source_span_manifest(raw)
    output = Path(output_path).resolve()
    if output.exists() or output.is_symlink():
        raise Rejected("sealed source span output already exists")
    write_new(output, (json.dumps(sealed, indent=2, sort_keys=True) + "\n").encode("utf-8"))
    return {
        "schema": f"{sealed['schema']}/seal",
        "status": "READY",
        "input": _path_descriptor(source, "unsealed source span manifest", must_exist=True),
        "output": _path_descriptor(output, "sealed source span manifest", must_exist=True),
        "manifest_sha256": sealed["manifest_sha256"],
        "authority_advanced": False,
    }


def normalize_source_span_template(
    envelope_path: Path | str,
    template_path: Path | str,
    binding_plan_path: Path | str,
    output_path: Path | str,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any],
) -> dict[str, Any]:
    """Normalize reviewed placeholder metadata into one capture-local v2 manifest."""

    envelope = validate_envelope(envelope_path, trust_root=trust_root)
    template_descriptor = _descriptor(template_path, "source span template")
    envelope_descriptor = _descriptor(envelope_path, "source span envelope")
    plan_descriptor = _descriptor(binding_plan_path, "source span binding plan")
    template = strict_json_loads(
        Path(template_descriptor["path"]).read_text(encoding="utf-8"),
        "source span template",
    )
    plan = strict_json_loads(
        Path(plan_descriptor["path"]).read_text(encoding="utf-8"),
        "source span binding plan",
    )
    if (
        not isinstance(template, Mapping)
        or not _SOURCE_TEMPLATE_REQUIRED_FIELDS.issubset(template)
        or not set(template).issubset(_SOURCE_TEMPLATE_ALLOWED_FIELDS)
    ):
        raise Rejected("source span template shape is unsupported")
    if (
        template.get("diagnostic_only") is not True
        or template.get("board_admission") is not False
        or template.get("exactness_claim") is not False
        or template.get("authority_advanced") is not False
        or template.get("unsealed") is not True
    ):
        raise Rejected("source span template policy is not fail-closed")
    expected_plan_fields = {
        "schema", "function", "function_sha256", "session_id", "source",
        "envelope", "template", "objects", "bindings", "authority_advanced",
    }
    if not isinstance(plan, Mapping) or set(plan) != expected_plan_fields:
        raise Rejected("source span binding plan shape is unsupported")
    if plan.get("schema") != SOURCE_SPAN_PLAN_SCHEMA or plan.get("authority_advanced") is not False:
        raise Rejected("source span binding plan schema/policy mismatch")
    context = envelope["context"]
    for key in ("function", "function_sha256", "session_id"):
        template_value = template.get(key)
        template_matches = (
            template_value in {context.get(key), "<CAPTURE_SESSION_ID>"}
            if key == "session_id"
            else template_value == context.get(key)
        )
        if plan.get(key) != context.get(key) or not template_matches:
            raise Rejected(f"source span binding plan/template {key} is not capture-bound")
    if _descriptor(plan["source"], "source span plan source") != dict(context["source"]):
        raise Rejected("source span binding plan source is not capture-bound")
    if _descriptor(template["source"], "source span template source") != dict(context["source"]):
        raise Rejected("source span template source is not capture-bound")
    if _descriptor(plan["envelope"], "source span plan envelope") != envelope_descriptor:
        raise Rejected("source span binding plan envelope identity changed")
    if _descriptor(plan["template"], "source span plan template") != template_descriptor:
        raise Rejected("source span binding plan template identity changed")

    inventory = [
        row
        for container in ("locals", "arguments")
        for row in envelope["inventory"][container]
    ]
    inventory_by_name: dict[str, list[Mapping[str, Any]]] = {}
    for row in inventory:
        if isinstance(row.get("name"), str):
            inventory_by_name.setdefault(str(row["name"]), []).append(row)

    raw_objects = plan.get("objects")
    if not isinstance(raw_objects, list) or not raw_objects:
        raise Rejected("source span binding plan requires objects")
    planned_objects: dict[str, dict[str, Any]] = {}
    for index, raw_object in enumerate(raw_objects):
        if not isinstance(raw_object, Mapping) or set(raw_object) != _SOURCE_PLAN_OBJECT_FIELDS:
            raise Rejected(f"source span binding plan objects[{index}] shape mismatch")
        identity = _text(raw_object["identity"], f"source span binding plan objects[{index}].identity")
        if identity in planned_objects:
            raise Rejected("source span binding plan contains duplicate identities")
        rows = inventory_by_name.get(identity, [])
        if len(rows) != 1:
            raise Rejected("source span identity does not bind one unique compiler inventory row")
        planned_objects[identity] = {
            **dict(raw_object),
            "object_token": str(rows[0]["token"]),
        }

    raw_bindings = plan.get("bindings")
    if not isinstance(raw_bindings, list) or not raw_bindings:
        raise Rejected("source span binding plan requires at least one dependency binding")
    bindings: dict[tuple[str, str, int, int], dict[str, Any]] = {}
    for index, raw_binding in enumerate(raw_bindings):
        if not isinstance(raw_binding, Mapping) or set(raw_binding) != _SOURCE_PLAN_BINDING_FIELDS:
            raise Rejected(f"source span binding plan bindings[{index}] shape mismatch")
        identity = _text(raw_binding["identity"], f"source span binding plan bindings[{index}].identity")
        role = _text(raw_binding["role"], f"source span binding plan bindings[{index}].role")
        start = _integer(raw_binding["byte_start"], "source span binding byte_start", nonnegative=True)
        end = _integer(raw_binding["byte_end"], "source span binding byte_end", nonnegative=True)
        key = (identity, role, start, end)
        if key in bindings:
            raise Rejected("source span binding plan contains duplicate span bindings")
        bindings[key] = dict(raw_binding)

    raw_spans = template.get("spans")
    if not isinstance(raw_spans, list) or not raw_spans:
        raise Rejected("source span template has no spans")
    normalized_spans: list[dict[str, Any]] = []
    matched_bindings: set[tuple[str, str, int, int]] = set()
    template_identities: set[str] = set()
    for index, raw_span in enumerate(raw_spans):
        if not isinstance(raw_span, Mapping) or not _SOURCE_SPAN_FIELDS.issubset(raw_span) or not set(raw_span).issubset(_SOURCE_TEMPLATE_SPAN_ALLOWED_FIELDS):
            raise Rejected(f"source span template spans[{index}] shape mismatch")
        identity = _text(raw_span["identity"], f"source span template spans[{index}].identity")
        object_spec = planned_objects.get(identity)
        if object_spec is None:
            raise Rejected("source span template identity is absent from the binding plan")
        template_identities.add(identity)
        key = (
            identity,
            str(raw_span["role"]),
            int(raw_span["byte_start"]),
            int(raw_span["byte_end"]),
        )
        binding = bindings.get(key)
        if binding is not None:
            matched_bindings.add(key)
        normalized_spans.append({
            **{field: raw_span[field] for field in _SOURCE_SPAN_FIELDS if field != "object_token"},
            "object_token": object_spec["object_token"],
            "dependency_id": binding["dependency_id"] if binding is not None else None,
            "machine_instruction_indices": list(binding["machine_instruction_indices"]) if binding is not None else [],
        })
    if template_identities != set(planned_objects) or matched_bindings != set(bindings):
        raise Rejected("source span template/plan identity or dependency coverage is not closed")

    normalized_objects = [
        {
            "object_token": raw_object["object_token"],
            "identity": identity,
            "ownership_mode": raw_object["ownership_mode"],
            "object_type": raw_object["object_type"],
            "byte_size": raw_object["byte_size"],
        }
        for identity, raw_object in planned_objects.items()
    ]
    normalized_objects.sort(key=lambda row: str(row["object_token"]))
    manifest = seal_source_span_manifest({
        "schema": SOURCE_SPAN_SCHEMA_V2,
        "function": context["function"],
        "function_sha256": context["function_sha256"],
        "session_id": context["session_id"],
        "source": dict(context["source"]),
        "objects": normalized_objects,
        "spans": normalized_spans,
        "authority_advanced": False,
    })
    output = Path(output_path).resolve()
    if output.exists() or output.is_symlink():
        raise Rejected("normalized source span output already exists")
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary_name: str | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="\n",
            dir=output.parent,
            prefix=".source-spans-",
            suffix=".json",
            delete=False,
        ) as temporary:
            json.dump(manifest, temporary, indent=2, sort_keys=True)
            temporary.write("\n")
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_name = temporary.name
        _validate_source_span_manifest(temporary_name, envelope)
        os.rename(temporary_name, output)
        temporary_name = None
    finally:
        if temporary_name is not None:
            try:
                Path(temporary_name).unlink()
            except OSError:
                pass
    return {
        "schema": f"{SOURCE_SPAN_SCHEMA_V2}/normalize",
        "status": "READY",
        "envelope": envelope_descriptor,
        "template": template_descriptor,
        "binding_plan": plan_descriptor,
        "output": _descriptor(output, "normalized source span output"),
        "manifest_sha256": manifest["manifest_sha256"],
        "authority_advanced": False,
    }


def _validate_source_span_manifest(
    manifest_path: Path | str,
    envelope: Mapping[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    path = _canonical_path(manifest_path, "source span manifest")
    try:
        value = strict_json_loads(path.read_text(encoding="utf-8"), "source span manifest")
    except OSError as exc:
        raise Rejected(f"cannot read source span manifest: {exc}") from exc
    if not isinstance(value, Mapping):
        raise Rejected("source span manifest must be an object")
    schema = value.get("schema")
    if schema == SOURCE_SPAN_SCHEMA:
        expected = {
            "schema", "function", "function_sha256", "session_id", "source",
            "spans", "authority_advanced", "manifest_sha256",
        }
    elif schema == SOURCE_SPAN_SCHEMA_V2:
        expected = {
            "schema", "function", "function_sha256", "session_id", "source",
            "objects", "spans", "authority_advanced", "manifest_sha256",
        }
    else:
        raise Rejected("source span manifest schema/policy mismatch")
    if set(value) != expected:
        raise Rejected("source span manifest contains unsupported or missing fields")
    unsigned = {key: item for key, item in value.items() if key != "manifest_sha256"}
    if value.get("manifest_sha256") != canonical_hash(unsigned):
        raise Rejected("source span manifest self-digest mismatch")
    if value.get("authority_advanced") is not False:
        raise Rejected("source span manifest schema/policy mismatch")

    context = envelope["context"]
    for key in ("function", "function_sha256", "session_id"):
        if value.get(key) != context.get(key):
            raise Rejected(f"source span manifest {key} is not capture-bound")
    source = value.get("source")
    if not isinstance(source, Mapping) or set(source) != {"path", "size", "sha256"}:
        raise Rejected("source span manifest source descriptor is malformed")
    if dict(source) != dict(context["source"]):
        raise Rejected("source span manifest source descriptor is not capture-bound")
    source_path = _canonical_path(source["path"], "source span manifest source")
    source_bytes = source_path.read_bytes()
    if len(source_bytes) != source["size"] or hashlib.sha256(source_bytes).hexdigest() != source["sha256"]:
        raise Rejected("source span manifest source bytes changed")
    try:
        source_text = source_bytes.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise Rejected("source span manifest source is not UTF-8") from exc

    inventory = [
        row
        for container in ("locals", "arguments")
        for row in envelope["inventory"][container]
    ]
    inventory_by_token = {str(row["token"]): row for row in inventory}
    objects_by_token: dict[str, dict[str, Any]] = {}
    if schema == SOURCE_SPAN_SCHEMA_V2:
        objects = value.get("objects")
        if not isinstance(objects, list) or not objects:
            raise Rejected("source span v2 manifest requires at least one object")
        for index, raw_object in enumerate(objects):
            if not isinstance(raw_object, Mapping) or set(raw_object) != _SOURCE_OBJECT_FIELDS:
                raise Rejected(f"source span manifest objects[{index}] shape mismatch")
            token = _text(
                raw_object["object_token"],
                f"source span manifest objects[{index}].object_token",
            )
            token_match = TOKEN_RE.fullmatch(token)
            if token_match is None or token_match.group("session") != context["session_id"]:
                raise Rejected("source span manifest object token is not capture-local")
            if token in objects_by_token:
                raise Rejected("source span manifest contains a duplicate object token")
            inventory_row = inventory_by_token.get(token)
            if inventory_row is None:
                raise Rejected("source span manifest object token is not in the authenticated inventory")
            identity = _text(
                raw_object["identity"],
                f"source span manifest objects[{index}].identity",
            )
            if SAFE_FUNCTION.fullmatch(identity) is None or inventory_row.get("name") != identity:
                raise Rejected("source span manifest object identity does not match compiler inventory metadata")
            mode = _text(
                raw_object["ownership_mode"],
                f"source span manifest objects[{index}].ownership_mode",
            )
            if mode not in _SOURCE_OWNERSHIP_MODES:
                raise Rejected("source span manifest object ownership mode is unsupported")
            object_type = raw_object["object_type"]
            byte_size = raw_object["byte_size"]
            ownership = inventory_row.get("ownership")
            if not isinstance(ownership, Mapping) or ownership.get("status") != "EXACT":
                raise Rejected("source span manifest object lacks exact authenticated ownership")
            if mode == "scalar_register":
                if object_type is not None or byte_size is not None:
                    raise Rejected("scalar source span object must not assert aggregate type or size")
                if ownership.get("mode") == "stack_home":
                    raise Rejected("scalar source span object is backed only by a stack home")
            else:
                if object_type != "HuVecF" or byte_size != 12:
                    raise Rejected("stack interval source span object must be an exact 12-byte HuVecF")
                if ownership.get("mode") != "stack_home":
                    raise Rejected("stack interval source span object lacks an authenticated final stack home")
            objects_by_token[token] = dict(raw_object)
    spans = value.get("spans")
    if not isinstance(spans, list) or not spans:
        raise Rejected("source span manifest requires at least one span")
    claimed_ranges: list[tuple[int, int, str]] = []
    seen: set[tuple[str, str, int, int]] = set()
    dependency_claims: dict[int, str] = {}
    stack_declarations: set[str] = set()
    stack_dependencies: set[str] = set()
    dependency_indices_by_token: dict[tuple[str, str], tuple[int, ...]] = {}
    referenced_tokens: set[str] = set()
    expected_span_fields = _SOURCE_SPAN_FIELDS if schema == SOURCE_SPAN_SCHEMA else _SOURCE_SPAN_V2_FIELDS
    for index, raw in enumerate(spans):
        if not isinstance(raw, Mapping) or set(raw) != expected_span_fields:
            raise Rejected(f"source span manifest spans[{index}] shape mismatch")
        token = _text(raw["object_token"], f"source span manifest spans[{index}].object_token")
        token_match = TOKEN_RE.fullmatch(token)
        if token_match is None or token_match.group("session") != context["session_id"]:
            raise Rejected("source span manifest token is not capture-local")
        inventory_row = inventory_by_token.get(token)
        if inventory_row is None:
            raise Rejected("source span manifest token is not in the authenticated inventory")
        if schema == SOURCE_SPAN_SCHEMA_V2 and token not in objects_by_token:
            raise Rejected("source span manifest span lacks a closed object declaration")
        referenced_tokens.add(token)
        identity = _text(raw["identity"], f"source span manifest spans[{index}].identity")
        if SAFE_FUNCTION.fullmatch(identity) is None or inventory_row.get("name") != identity:
            raise Rejected("source span manifest identity does not match compiler inventory metadata")
        role = _text(raw["role"], f"source span manifest spans[{index}].role")
        if role not in _SOURCE_SPAN_ROLES:
            raise Rejected("source span manifest role is unsupported")
        start = _integer(raw["byte_start"], "source span byte_start", nonnegative=True)
        end = _integer(raw["byte_end"], "source span byte_end", nonnegative=True)
        if start >= end or end > len(source_bytes):
            raise Rejected("source span manifest byte range is invalid")
        try:
            snippet = source_bytes[start:end].decode("utf-8")
            prefix = source_bytes[:start].decode("utf-8")
        except UnicodeDecodeError as exc:
            raise Rejected("source span manifest splits a UTF-8 sequence") from exc
        if hashlib.sha256(source_bytes[start:end]).hexdigest() != _digest(raw["text_sha256"], "source span text_sha256"):
            raise Rejected("source span manifest text digest mismatch")
        if re.search(rf"(?<![A-Za-z0-9_]){re.escape(identity)}(?![A-Za-z0-9_])", snippet) is None:
            raise Rejected("source span manifest text does not contain its exact identity")
        actual_line_start = prefix.count("\n") + 1
        actual_line_end = actual_line_start + snippet.count("\n")
        if raw["line_start"] != actual_line_start or raw["line_end"] != actual_line_end:
            raise Rejected("source span manifest line range mismatch")
        if schema == SOURCE_SPAN_SCHEMA_V2:
            dependency_id = raw["dependency_id"]
            instruction_indices = raw["machine_instruction_indices"]
            if dependency_id is None:
                if instruction_indices != []:
                    raise Rejected("source span manifest unbound dependency has machine indices")
            else:
                dependency_id = _text(
                    dependency_id,
                    f"source span manifest spans[{index}].dependency_id",
                )
                if _SOURCE_DEPENDENCY_RE.fullmatch(dependency_id) is None:
                    raise Rejected("source span manifest dependency id is malformed")
                if (
                    not isinstance(instruction_indices, list)
                    or not instruction_indices
                    or any(
                        not isinstance(item, int) or isinstance(item, bool) or item < 0
                        for item in instruction_indices
                    )
                    or instruction_indices != sorted(set(instruction_indices))
                ):
                    raise Rejected("source span manifest dependency instruction indices are invalid")
                key = (token, dependency_id)
                canonical_indices = tuple(instruction_indices)
                prior_indices = dependency_indices_by_token.get(key)
                if prior_indices not in (None, canonical_indices):
                    raise Rejected("one source object dependency has conflicting machine indices")
                dependency_indices_by_token[key] = canonical_indices
                for instruction_index in instruction_indices:
                    prior_dependency = dependency_claims.get(instruction_index)
                    if prior_dependency not in (None, dependency_id):
                        raise Rejected("one machine instruction is claimed by multiple source dependencies")
                    dependency_claims[instruction_index] = dependency_id
                if objects_by_token[token]["ownership_mode"] == "stack_interval":
                    stack_dependencies.add(token)
            if (
                objects_by_token[token]["ownership_mode"] == "stack_interval"
                and role == "declaration"
            ):
                if "HuVecF" not in snippet:
                    raise Rejected("stack interval declaration span does not prove HuVecF source type")
                stack_declarations.add(token)
        for prior_start, prior_end, prior_token in claimed_ranges:
            if start < prior_end and prior_start < end and prior_token != token:
                # One exact source object may legitimately own nested source
                # roles over the same expression (for example, an initialized
                # declaration is both a declaration and a call-return span).
                # Crossing that range into a different Object identity is an
                # ambiguous alias and remains fail-closed.
                raise Rejected("different source object bindings overlap")
        claimed_ranges.append((start, end, token))
        unique = (token, role, start, end)
        if unique in seen:
            raise Rejected("source span manifest contains a duplicate binding")
        seen.add(unique)
    if schema == SOURCE_SPAN_SCHEMA_V2:
        if referenced_tokens != set(objects_by_token):
            raise Rejected("source span manifest object/span coverage is not closed")
        stack_tokens = {
            token
            for token, raw_object in objects_by_token.items()
            if raw_object["ownership_mode"] == "stack_interval"
        }
        if stack_tokens - stack_declarations:
            raise Rejected("stack interval source span object lacks a typed declaration span")
        if stack_tokens - stack_dependencies:
            raise Rejected("stack interval source span object lacks a machine dependency")
    chronology = _donor_cfg.source_chronology(source_path, symbol=str(context["function"]))
    if chronology["source"]["sha256"] != source["sha256"]:
        raise Rejected("source chronology parser did not consume the capture-bound source")
    return dict(value), chronology


def _tool_source_descriptor(module: Any) -> dict[str, Any]:
    path = Path(module.__file__).resolve()
    return _path_descriptor(path, f"tool {module.__name__}", must_exist=True)


def _source_machine_index(events: Sequence[Mapping[str, Any]]) -> dict[int, dict[str, Any]]:
    """Index located machine evidence without inventing missing rows."""

    result: dict[int, dict[str, Any]] = {}
    for event in events:
        if event.get("event_kind") != "machine_emission":
            continue
        instruction_index = event.get("instruction_index")
        if not isinstance(instruction_index, int) or isinstance(instruction_index, bool):
            continue
        if instruction_index in result:
            raise Rejected("machine-emission chronology contains duplicate instruction indices")
        result[instruction_index] = dict(event)
    return result


def _source_pcode_owners(events: Sequence[Mapping[str, Any]]) -> dict[str, str]:
    """Return only unique, confirmed same-session Object-to-PCode owners."""

    claims: dict[str, set[str]] = {}
    for event in events:
        if (
            event.get("event_kind") == "pcode_capture"
            and event.get("status") == "CAPTURED"
            and event.get("confirmed") is True
            and isinstance(event.get("pcode_token"), str)
            and isinstance(event.get("object_token"), str)
        ):
            claims.setdefault(str(event["pcode_token"]), set()).add(str(event["object_token"]))
    return {
        pcode_token: next(iter(tokens))
        for pcode_token, tokens in claims.items()
        if len(tokens) == 1
    }


def _closed_memory_coverage(
    rows: Sequence[Mapping[str, Any]],
    *,
    byte_size: int,
) -> tuple[int, int] | None:
    """Return a whole, nonoverlapping stack interval or ``None``."""

    intervals: list[tuple[int, int]] = []
    for row in rows:
        if "memory_op" not in row:
            continue
        start = row.get("effective_stack_offset")
        width = row.get("memory_width")
        if (
            not isinstance(start, int)
            or isinstance(start, bool)
            or not isinstance(width, int)
            or isinstance(width, bool)
            or width <= 0
        ):
            return None
        intervals.append((start, start + width))
    if not intervals:
        return None
    intervals.sort()
    cursor = intervals[0][0]
    first = cursor
    for start, end in intervals:
        if start != cursor or end <= start:
            return None
        cursor = end
    if cursor - first != byte_size:
        return None
    return first, cursor


def _stack_interval_object_join(
    *,
    token: str,
    object_spec: Mapping[str, Any],
    spans: Sequence[Mapping[str, Any]],
    inventory_row: Mapping[str, Any],
    machine_by_index: Mapping[int, Mapping[str, Any]],
    pcode_owners: Mapping[str, str],
) -> dict[str, Any]:
    """Bind one typed source aggregate to final stack-home dependencies."""

    ownership = inventory_row.get("ownership")
    if (
        not isinstance(ownership, Mapping)
        or ownership.get("status") != "EXACT"
        or ownership.get("mode") != "stack_home"
        or not isinstance(ownership.get("stack_home"), Mapping)
    ):
        return {
            "status": "UNKNOWN",
            "evidence": ["authenticated final stack home is absent"],
            "stack_interval_dependencies": [],
        }
    raw_home = int(ownership["stack_home"]["offset"])
    byte_size = int(object_spec["byte_size"])
    grouped: dict[str, dict[str, Any]] = {}
    for span in spans:
        dependency_id = span.get("dependency_id")
        if not isinstance(dependency_id, str):
            continue
        group = grouped.setdefault(
            dependency_id,
            {"roles": set(), "indices": tuple(span["machine_instruction_indices"])},
        )
        group["roles"].add(str(span["role"]))

    dependency_rows: list[dict[str, Any]] = []
    reasons: list[str] = []
    biases: set[int] = set()
    for dependency_id in sorted(grouped):
        group = grouped[dependency_id]
        indices = list(group["indices"])
        selected: list[dict[str, Any]] = []
        missing = [index for index in indices if index not in machine_by_index]
        if missing:
            reason = f"dependency {dependency_id} lacks located machine rows {missing}"
            reasons.append(reason)
            dependency_rows.append({
                "dependency_id": dependency_id,
                "status": "UNKNOWN",
                "reason": reason,
                "instruction_indices": indices,
            })
            continue
        selected = [dict(machine_by_index[index]) for index in indices]
        unknown = [row for row in selected if row.get("status") != "CAPTURED"]
        if unknown:
            reason = f"dependency {dependency_id} contains UNKNOWN machine evidence"
            reasons.append(reason)
            dependency_rows.append({
                "dependency_id": dependency_id,
                "status": "UNKNOWN",
                "reason": reason,
                "instruction_indices": indices,
                "unknown_instruction_indices": [row["instruction_index"] for row in unknown],
            })
            continue
        memory_rows = [row for row in selected if "memory_op" in row]
        roles = set(group["roles"])
        expected_op = "load" if "read" in roles and "write" not in roles else "store" if "write" in roles and "read" not in roles else None
        if expected_op is None or any(row.get("memory_op") != expected_op for row in memory_rows):
            reason = f"dependency {dependency_id} source role does not match one machine memory direction"
            reasons.append(reason)
            dependency_rows.append({
                "dependency_id": dependency_id,
                "status": "UNKNOWN",
                "reason": reason,
                "instruction_indices": indices,
            })
            continue
        coverage = _closed_memory_coverage(memory_rows, byte_size=byte_size)
        if coverage is None:
            reason = f"dependency {dependency_id} does not wholly cover one {byte_size}-byte aggregate"
            reasons.append(reason)
            dependency_rows.append({
                "dependency_id": dependency_id,
                "status": "UNKNOWN",
                "reason": reason,
                "instruction_indices": indices,
            })
            continue
        start, end = coverage
        bias = start - raw_home
        if bias < 0 or bias % 4:
            reason = f"dependency {dependency_id} has an invalid final-home ABI adjustment"
            reasons.append(reason)
            dependency_rows.append({
                "dependency_id": dependency_id,
                "status": "UNKNOWN",
                "reason": reason,
                "instruction_indices": indices,
            })
            continue
        biases.add(bias)
        pcode_tokens = sorted({
            str(row["pcode_token"])
            for row in memory_rows
            if isinstance(row.get("pcode_token"), str)
        })
        conflicting_pcode_owner = any(
            pcode_owners.get(pcode_token) not in {None, token}
            for pcode_token in pcode_tokens
        )
        exact_direct_pcode = bool(pcode_tokens) and all(
            pcode_owners.get(pcode_token) == token for pcode_token in pcode_tokens
        )
        exact_interval_pcode = (
            bool(pcode_tokens)
            and len(pcode_tokens) == len(memory_rows)
            and not conflicting_pcode_owner
        )
        dependency_rows.append({
            "dependency_id": dependency_id,
            "status": "MATCHED_AUTHENTICATED",
            "instruction_indices": indices,
            "memory_direction": expected_op,
            "raw_stack_home": raw_home,
            "abi_adjustment_bytes": bias,
            "effective_interval": {"start": start, "end": end, "size": byte_size},
            "whole_access": True,
            "machine_events": selected,
            "pcode_crosswalk": {
                "status": "MATCHED_AUTHENTICATED" if exact_interval_pcode else "UNKNOWN",
                "pcode_tokens": pcode_tokens,
                "ownership_edge": (
                    "direct_object_to_pcode"
                    if exact_direct_pcode
                    else "final_stack_home_to_machine_pcode"
                    if exact_interval_pcode
                    else None
                ),
                "reason": (
                    None
                    if exact_interval_pcode
                    else "machine PCode rows are missing or conflict with direct Object ownership"
                ),
            },
        })
    if len(biases) > 1:
        reasons.append("one stack object has inconsistent final-home ABI adjustments")
    exact = bool(dependency_rows) and not reasons and all(
        row["status"] == "MATCHED_AUTHENTICATED" for row in dependency_rows
    )
    return {
        "status": "MATCHED_AUTHENTICATED" if exact else "UNKNOWN",
        "evidence": ["typed whole-access stack interval is authenticated"] if exact else reasons,
        "stack_interval_dependencies": dependency_rows,
        "abi_adjustment_bytes": next(iter(biases)) if exact and len(biases) == 1 else None,
    }


def _stack_copy_dependencies(joined: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    """Prove only exact read-to-write aggregate copies within one dependency."""

    by_dependency: dict[str, list[tuple[Mapping[str, Any], Mapping[str, Any]]]] = {}
    for row in joined:
        for dependency in row.get("stack_interval_dependencies", []):
            if dependency.get("status") == "MATCHED_AUTHENTICATED":
                by_dependency.setdefault(str(dependency["dependency_id"]), []).append((row, dependency))
    result: list[dict[str, Any]] = []
    for dependency_id in sorted(by_dependency):
        rows = by_dependency[dependency_id]
        reads = [(obj, dep) for obj, dep in rows if dep.get("memory_direction") == "load"]
        writes = [(obj, dep) for obj, dep in rows if dep.get("memory_direction") == "store"]
        if len(reads) != 1 or len(writes) != 1:
            continue
        source, source_dep = reads[0]
        destination, destination_dep = writes[0]
        source_interval = source_dep["effective_interval"]
        destination_interval = destination_dep["effective_interval"]
        loads = {
            int(event["instruction_index"]): event
            for event in source_dep["machine_events"]
            if event.get("memory_op") == "load"
        }
        stores = [
            event
            for event in destination_dep["machine_events"]
            if event.get("memory_op") == "store"
        ]
        pairs: list[dict[str, Any]] = []
        reason: str | None = None
        for store in stores:
            candidates = [
                load
                for index, load in loads.items()
                if index in store.get("reaching_definitions", [])
                and load.get("memory_width") == store.get("memory_width")
                and load.get("registers", {}).get("data") == store.get("registers", {}).get("data")
                and int(load["effective_stack_offset"]) - int(source_interval["start"])
                == int(store["effective_stack_offset"]) - int(destination_interval["start"])
            ]
            if len(candidates) != 1:
                reason = "copy store lacks one exact same-width reaching source load"
                break
            load = candidates[0]
            pairs.append({
                "load_instruction_index": load["instruction_index"],
                "store_instruction_index": store["instruction_index"],
                "relative_offset": int(load["effective_stack_offset"]) - int(source_interval["start"]),
                "width": load["memory_width"],
            })
        exact = reason is None and sum(int(pair["width"]) for pair in pairs) == int(source_interval["size"])
        selected = [
            event
            for _obj, dependency in rows
            for event in dependency["machine_events"]
        ]
        def address_producer(interval_start: int) -> Mapping[str, Any] | None:
            candidates = [
                event
                for event in selected
                if event.get("mnemonic") == "addi"
                and isinstance(event.get("address_definition"), Mapping)
                and int(event["address_definition"].get("stack_offset", -1)) == interval_start
                and event.get("registers", {}).get("base") == "r1"
                and event.get("registers", {}).get("destination")
                == event["address_definition"].get("register")
            ]
            return candidates[0] if len(candidates) == 1 else None

        source_producer = address_producer(int(source_interval["start"]))
        destination_producer = address_producer(int(destination_interval["start"]))

        def memory_uses_producer(
            events: Sequence[Mapping[str, Any]],
            producer: Mapping[str, Any] | None,
        ) -> bool:
            if producer is None:
                return False
            produced_register = producer["registers"]["destination"]
            producer_index = int(producer["instruction_index"])
            memory_events = [event for event in events if "memory_op" in event]
            return bool(memory_events) and all(
                event.get("registers", {}).get("base") == produced_register
                and producer_index in event.get("reaching_definitions", [])
                for event in memory_events
            )

        paired_mnemonics = {"psq_l", "lfs", "psq_st", "stfs"}
        paired_proof = (
            exact
            and paired_mnemonics.issubset({str(event.get("mnemonic")) for event in selected})
            and memory_uses_producer(source_dep["machine_events"], source_producer)
            and memory_uses_producer(destination_dep["machine_events"], destination_producer)
        )
        result.append({
            "dependency_id": dependency_id,
            "status": "MATCHED_AUTHENTICATED" if exact else "UNKNOWN",
            "source_object_token": source["object_token"],
            "destination_object_token": destination["object_token"],
            "copy_pairs": pairs,
            "paired_codegen_proof": paired_proof,
            "paired_codegen_reason": (
                None
                if paired_proof
                else "paired proof requires exact r1-derived addi address producers and memory-use edges"
            ),
            "reason": None if exact else reason or "copy coverage is incomplete",
        })
    return result


def build_source_aware_causal_map(
    envelope_path: Path | str,
    source_span_manifest: Path | str,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any],
    frontend_chronology: Path | str | None = None,
    post_capture_analysis: bool = False,
) -> dict[str, Any]:
    """Join source spans to same-session physical/stack evidence fail-closed."""

    envelope = validate_envelope(
        envelope_path,
        trust_root=trust_root,
        post_capture_analysis=post_capture_analysis,
    )
    manifest, source_chronology = _validate_source_span_manifest(source_span_manifest, envelope)
    events = envelope["events"]
    spans_by_token: dict[str, list[dict[str, Any]]] = {}
    for span in manifest["spans"]:
        spans_by_token.setdefault(str(span["object_token"]), []).append(dict(span))

    physical_by_token: dict[str, list[dict[str, Any]]] = {}
    stack_by_token: dict[str, list[dict[str, Any]]] = {}
    machine_by_token: dict[str, list[dict[str, Any]]] = {}
    machine_ambiguous_tokens: set[str] = set()
    machine_by_index = _source_machine_index(events)
    pcode_owners = _source_pcode_owners(events)
    for event in events:
        if event["event_kind"] == "machine_emission":
            if event.get("status") != "CAPTURED":
                continue
            event_claims: set[tuple[str, str]] = set()
            token_registers: dict[str, set[str]] = {}
            for join in [*event.get("owner_joins", []), *event.get("physical_owner_joins", [])]:
                token = str(join["object_token"])
                register = str(join["physical_register"])
                claim = (token, register)
                if claim in event_claims:
                    continue
                event_claims.add(claim)
                token_registers.setdefault(token, set()).add(register)
                if len(token_registers[token]) > 1:
                    machine_ambiguous_tokens.add(token)
                    continue
                machine_by_token.setdefault(token, []).append(dict(event))
            continue
        token = event.get("object_token")
        if not isinstance(token, str):
            continue
        if event["event_kind"] == "physical_reg_assignment":
            physical_by_token.setdefault(token, []).append(dict(event))
        elif event["event_kind"] in {"object_stack_write_pre", "object_stack_write_post"}:
            stack_by_token.setdefault(token, []).append(dict(event))

    inventory_rows = [
        row
        for container in ("locals", "arguments")
        for row in envelope["inventory"][container]
    ]
    name_counts: dict[str, int] = {}
    for row in inventory_rows:
        if isinstance(row.get("name"), str):
            name_counts[row["name"]] = name_counts.get(row["name"], 0) + 1
    inventory_by_token = {str(row["token"]): row for row in inventory_rows}
    object_specs = {
        str(raw["object_token"]): dict(raw)
        for raw in manifest.get("objects", [])
    }

    joined: list[dict[str, Any]] = []
    machine_required = str(envelope["context"]["compiler"]["sha256"]) == GC27_COMPILER_SHA256
    for row in inventory_rows:
        token = str(row["token"])
        name = row.get("name") if isinstance(row.get("name"), str) else ""
        ownership = row.get("ownership") if isinstance(row.get("ownership"), Mapping) else {}
        ownership_mode = ownership.get("mode")
        vreg_id = (
            ownership.get("vreg_id")
            if ownership.get("status") == "EXACT"
            and ownership_mode in {None, "virtual_register"}
            else None
        )
        direct_inventory = {
            "vreg_ids": [vreg_id] if isinstance(vreg_id, str) else [],
            "vreg_status": "AUTHENTICATED" if isinstance(vreg_id, str) else "UNKNOWN",
        }
        status, confidence, score, reasons = _correlator._join_status(
            name,
            {name} if name and name_counts.get(name, 0) > 1 else set(),
            None,
            direct_inventory if name else None,
            {vreg_id: {}} if isinstance(vreg_id, str) else {},
            set(),
            True,
            [],
            direct_v3=True,
        )
        bound_spans = spans_by_token.get(token, [])
        physical = physical_by_token.get(token, [])
        object_spec = object_specs.get(token)
        requested_mode = object_spec.get("ownership_mode") if object_spec is not None else "scalar_register"
        stack_join: dict[str, Any] | None = None
        if not bound_spans:
            status, confidence, score = "UNKNOWN", "none", 0.0
            reasons = ["authenticated source-span binding is absent"]
        elif requested_mode == "stack_interval":
            stack_join = _stack_interval_object_join(
                token=token,
                object_spec=object_spec,
                spans=bound_spans,
                inventory_row=inventory_by_token[token],
                machine_by_index=machine_by_index,
                pcode_owners=pcode_owners,
            )
            status = str(stack_join["status"])
            confidence = "exact" if status == "MATCHED_AUTHENTICATED" else "none"
            score = 1.0 if status == "MATCHED_AUTHENTICATED" else 0.0
            reasons = list(stack_join["evidence"])
        elif len(physical) > 1:
            status, confidence, score = "UNKNOWN", "none", 0.0
            reasons = ["one Object token has multiple physical-register assignments"]
        elif not isinstance(vreg_id, str) and len(physical) == 1:
            if manifest.get("schema") == SOURCE_SPAN_SCHEMA_V2:
                status, confidence, score = "UNKNOWN", "none", 0.0
                reasons = ["scalar source object lacks an authenticated Object-to-vreg ownership edge"]
            else:
                status, confidence, score = "MATCHED_AUTHENTICATED", "exact", 1.0
                reasons = ["source span and unique same-session physical register are authenticated"]
        machine = machine_by_token.get(token, [])
        if machine_required and bound_spans and requested_mode == "scalar_register":
            if token in machine_ambiguous_tokens:
                status, confidence, score = "UNKNOWN", "none", 0.0
                reasons = ["machine-emission owner join is ambiguous"]
            elif len(physical) != 1:
                status, confidence, score = "UNKNOWN", "none", 0.0
                reasons = ["exact physical-register evidence is absent for machine join"]
            elif not machine:
                status, confidence, score = "UNKNOWN", "none", 0.0
                reasons = ["authenticated machine-emission owner join is absent"]
        stack_machine = []
        if stack_join is not None:
            stack_machine = [
                event
                for dependency in stack_join["stack_interval_dependencies"]
                if dependency.get("status") == "MATCHED_AUTHENTICATED"
                for event in dependency["machine_events"]
            ]
            stack_machine = list({int(event["instruction_index"]): event for event in stack_machine}.values())
            stack_machine.sort(key=lambda event: int(event["instruction_index"]))
        related_calls = [
            call
            for call in source_chronology["calls"]
            if name and call.get("assigned_lhs") == name
        ]
        joined.append({
            "object_token": token,
            "identity": name or None,
            "ownership_mode": requested_mode,
            "status": status,
            "confidence": confidence,
            "score": score,
            "evidence": reasons,
            "source_spans": bound_spans,
            "virtual_register": vreg_id,
            "physical_register": physical[0] if len(physical) == 1 else None,
            "stack_chronology": stack_by_token.get(token, []),
            "stack_interval_dependencies": stack_join["stack_interval_dependencies"] if stack_join is not None else [],
            "abi_adjustment_bytes": stack_join.get("abi_adjustment_bytes") if stack_join is not None else None,
            "machine_emission_chronology": stack_machine if stack_join is not None else machine,
            "call_return_chronology": related_calls,
        })

    stack_biases = {
        int(row["abi_adjustment_bytes"])
        for row in joined
        if row["ownership_mode"] == "stack_interval"
        and row["status"] == "MATCHED_AUTHENTICATED"
        and isinstance(row.get("abi_adjustment_bytes"), int)
    }
    if len(stack_biases) > 1:
        for row in joined:
            if row["ownership_mode"] == "stack_interval" and row["status"] == "MATCHED_AUTHENTICATED":
                row["status"] = "UNKNOWN"
                row["confidence"] = "none"
                row["score"] = 0.0
                row["evidence"] = ["stack interval objects disagree on the final-home ABI adjustment"]
                for dependency in row["stack_interval_dependencies"]:
                    dependency["status"] = "UNKNOWN"
                    dependency["reason"] = "stack interval objects disagree on the final-home ABI adjustment"

    interval_claims: list[tuple[int, int, dict[str, Any]]] = []
    for row in joined:
        if row["ownership_mode"] != "stack_interval" or row["status"] != "MATCHED_AUTHENTICATED":
            continue
        for dependency in row["stack_interval_dependencies"]:
            if dependency.get("status") != "MATCHED_AUTHENTICATED":
                continue
            interval = dependency.get("effective_interval")
            if isinstance(interval, Mapping):
                interval_claims.append((int(interval["start"]), int(interval["end"]), row))
    conflicting_stack_rows: set[str] = set()
    for index, (left_start, left_end, left_row) in enumerate(interval_claims):
        for right_start, right_end, right_row in interval_claims[index + 1:]:
            if left_row["object_token"] == right_row["object_token"]:
                continue
            if max(left_start, right_start) < min(left_end, right_end):
                conflicting_stack_rows.update((left_row["object_token"], right_row["object_token"]))
    if conflicting_stack_rows:
        for row in joined:
            if row["object_token"] not in conflicting_stack_rows:
                continue
            row["status"] = "UNKNOWN"
            row["confidence"] = "none"
            row["score"] = 0.0
            row["evidence"] = ["different source objects claim overlapping effective stack intervals"]
            for dependency in row["stack_interval_dependencies"]:
                dependency["status"] = "UNKNOWN"
                dependency["reason"] = "different source objects claim overlapping effective stack intervals"
    stack_copy_dependencies = _stack_copy_dependencies(joined)

    frontend: dict[str, Any]
    if frontend_chronology is None:
        embedded = envelope.get("frontend_chronology")
        if isinstance(embedded, Mapping):
            if embedded.get("status") == "CAPTURED":
                validated = _validate_embedded_frontend_chronology(
                    embedded,
                    envelope["context"],
                )["packet"]
                frontend = {
                    "status": validated["status"],
                    "source": "embedded_same_session",
                    "packet_sha256": validated["packet_sha256"],
                    "events": validated["events"],
                }
            else:
                frontend = {
                    "status": "UNKNOWN",
                    "reason": str(embedded.get("reason", "incomplete frontend chronology")),
                    "source": "embedded_same_session",
                }
        else:
            frontend = {"status": "UNKNOWN", "reason": "frontend chronology packet was not supplied"}
    else:
        chronology_path = _canonical_path(frontend_chronology, "frontend chronology packet")
        chronology_raw = strict_json_loads(chronology_path.read_text(encoding="utf-8"), "frontend chronology packet")
        if not isinstance(chronology_raw, Mapping):
            raise Rejected("frontend chronology packet must be an object")
        validated = _frontend_chronology.validate_packet(chronology_raw)
        provenance = validated["provenance"]
        context = envelope["context"]
        if validated["function"] != context["function"]:
            raise Rejected("frontend chronology function does not match same-session capture")
        if provenance["session_id"] != context["session_id"]:
            raise Rejected("frontend chronology session does not match same-session capture")
        if provenance["source_sha256"] != context["source"]["sha256"] or provenance["compiler_sha256"] != context["compiler"]["sha256"]:
            raise Rejected("frontend chronology provenance does not match same-session capture")
        frontend = {
            "status": validated["status"],
            "path": str(chronology_path),
            "sha256": hashlib.sha256(chronology_path.read_bytes()).hexdigest(),
            "packet_sha256": validated["packet_sha256"],
            "events": validated["events"],
        }

    result = {
        "schema": CAUSAL_MAP_SCHEMA,
        "status": "CAPTURED" if any(row["status"] == "MATCHED_AUTHENTICATED" for row in joined) else "UNKNOWN",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "context": dict(envelope["context"]),
        "capture": {"path": str(_canonical_path(envelope_path, "envelope")), "sha256": envelope["envelope_sha256"]},
        "source_span_manifest": {"path": str(_canonical_path(source_span_manifest, "source span manifest")), "sha256": manifest["manifest_sha256"]},
        "capture_tool_validation": {
            "mode": (
                "sealed_post_capture_descriptor"
                if post_capture_analysis
                else "live_bytes"
            ),
            "debugger": dict(envelope["context"]["debugger"]),
            "transport": dict(envelope["context"]["transport"]),
        },
        "tools": {
            "same_session": _path_descriptor(Path(__file__).resolve(), "tool same_session", must_exist=True),
            "stack_home": _tool_source_descriptor(_stack_home),
            "frontend_chronology": _tool_source_descriptor(_frontend_chronology),
            "correlator": _tool_source_descriptor(_correlator),
            "donor_cfg": _tool_source_descriptor(_donor_cfg),
        },
        "joined_objects": joined,
        "stack_copy_dependencies": stack_copy_dependencies,
        "source_evaluation_chronology": source_chronology,
        "frontend_chronology": frontend,
        "unknown": sorted(
            {
                reason
                for row in joined
                if row["status"] != "MATCHED_AUTHENTICATED"
                for reason in row["evidence"]
            }
            | {
                str(dependency["pcode_crosswalk"]["reason"])
                for row in joined
                for dependency in row.get("stack_interval_dependencies", [])
                if dependency.get("status") == "MATCHED_AUTHENTICATED"
                and dependency.get("pcode_crosswalk", {}).get("status") == "UNKNOWN"
            }
            | ({frontend["reason"]} if frontend["status"] == "UNKNOWN" else set())
        ),
    }
    _correlator._reject_pointer_material(result)
    result["causal_map_sha256"] = canonical_hash(result)
    return result


class NativeWow64Backend:
    """Minimal one-process adapter used by ``launch_native_capture``.

    The breakpoint/event protocol is shared with fake backends.  The native
    debug loop is intentionally kept here rather than invoking two donor
    tools, so one process owns all hooks and one cleanup path restores them.
    """

    # Keep the capability names aligned with the concrete methods below.  The
    # session still performs method-level fail-closed checks, but advertising
    # the complete native surface makes a preflight able to reject an adapter
    # that silently fell back to UNKNOWN-only capture.
    capabilities = frozenset(
        {
            "read_image",
            "install_breakpoint",
            "remove_breakpoint",
            "single_step",
            "run",
            "close",
            "current_function",
            "snapshot_inventory",
            "snapshot_objects",
            "snapshot_varinfo",
            "read_register",
            "capture_stack_write",
            "capture_frontend",
            "capture_pcode",
            "capture_machine_emission",
            "capture_regalloc",
            "capture_physical_regalloc",
            "capture_regalloc_post",
            "prepare_capture",
        }
    )

    def __init__(
        self,
        native: Any,
        process: int,
        initial_thread: int,
        process_id: int,
        *,
        base: int = KNOWN_IMAGE_BASE,
        compiler_path: str | None = None,
        compiler_sha256: str | None = None,
        wrapper_path: str | None = None,
        wrapper_sha256: str | None = None,
        transport_mode: str = "wrapper_memexec",
        executed_argv: Sequence[str] | None = None,
    ) -> None:
        self.native = native
        self.process = int(process)
        # ``process``/``process_id`` begin as the wrapper transport.  A normal
        # launcher replaces them after an authenticated compiler child
        # CREATE_PROCESS event; sjiswrap keeps the same PID and only changes
        # the mapped image base after private-image authentication.  Until
        # either boundary no compiler address is read.
        self.transport_process = int(process)
        self.transport_process_id = int(process_id)
        self.transport_thread = int(initial_thread or 0)
        self.process_id = int(process_id)
        self.compiler_selected = False
        self.compiler_process_id: int | None = None
        self.compiler_path = self._normalize_image_path(compiler_path) if compiler_path else None
        self.compiler_sha256 = compiler_sha256.lower() if isinstance(compiler_sha256, str) else None
        self.wrapper_path = self._normalize_image_path(wrapper_path) if wrapper_path else None
        self.wrapper_sha256 = wrapper_sha256.lower() if isinstance(wrapper_sha256, str) else None
        if transport_mode not in {"wrapper_memexec", "authenticated_direct_compiler"}:
            raise Rejected("native transport mode is unsupported")
        self.transport_mode = transport_mode
        self.executed_argv = [str(value) for value in (executed_argv or ())]
        if transport_mode == "authenticated_direct_compiler" and not self.executed_argv:
            raise Rejected("native transport lacks its executed argv")
        self.direct_compiler_transport = transport_mode == "authenticated_direct_compiler"
        self._owned_handles: set[int] = {value for value in (int(process or 0), int(initial_thread or 0)) if value}
        self._process_handles: dict[int, int] = {
            int(process_id): int(process)
        } if int(process_id) > 0 and int(process or 0) > 0 else {}
        self._observed_process_ids: set[int] = {int(process_id)} if int(process_id) > 0 else set()
        self._exited_process_ids: set[int] = set()
        self._descendant_threads: dict[int, dict[int, int]] = {}
        self.transport_threads: dict[int, int] = {}
        if initial_thread:
            self.transport_threads[0] = int(initial_thread)
        self.threads: dict[int, int] = {}
        self.base = int(base)
        self.breakpoints: dict[int, int] = {}
        self.pending_steps: dict[int, int] = {}
        self.exited = False
        self.compiler_exit_code: int | None = None
        self.process_quiesced = False
        self.compiler_terminated_by_capture = False
        self.loader_breakpoint_pending = False
        self.loader_breakpoints_remaining = 0
        # These maps are process-local and never cross the event boundary.
        # They also make duplicate/cross-kind identities fail closed before
        # CombinedCaptureSession assigns its capture-local tokens.
        self._object_kind: dict[int, str] = {}
        self._object_varinfo: dict[int, int] = {}
        self._varinfo_object: dict[int, int] = {}
        self._direct_vreg_evidence: dict[int, dict[str, Any]] = {}
        self._pcode_events: list[dict[str, Any]] = []
        self._pending_pcode_color: dict[int, dict[str, int]] = {}
        self._transport_image_seen = False
        self._transport_exited = False
        self._memexec_probe_requested = False
        self._memexec_probe_count = 0
        self._pending_create_event: tuple[int, int] | None = None
        # sjiswrap's compiler handoff is a same-process manual map.  These
        # fields remain capture-internal; only the authenticated compiler PID
        # and normalized addresses are used by the debugger loop.
        self._memexec_image_base: int | None = None
        self._compiler_image_size: int | None = None
        self._compiler_preferred_base: int | None = None
        self._compiler_relocation_rvas_cache: tuple[int, ...] | None = None
        self._compiler_relocation_values_cache: dict[int, int] | None = None
        self._pending_debug_event: tuple[int, int] | None = None
        # A debugger event is an execution lease: Windows will not finish
        # terminating a DEBUG_PROCESS debuggee until the event is continued.
        # Keep the currently delivered event explicit so failure cleanup can
        # release that lease before waiting for process quiescence.
        self._active_debug_event: tuple[int, int] | None = None
        self._selection_mode: str | None = None
        self._close_started = False
        self._terminal_deadline: float | None = None

    def _runtime(self, absolute: int) -> int:
        return absolute - KNOWN_IMAGE_BASE + self.base

    def _consume_initial_system_breakpoint(self, address: int) -> bool:
        """Consume one bounded debugger-init breakpoint outside the compiler image."""

        if self.loader_breakpoints_remaining <= 0:
            return False
        if self._compiler_image_size is None:
            self._compiler_pe_shape()
        image_size = int(self._compiler_image_size or 0)
        if image_size <= 0:
            raise Rejected("authenticated compiler image size is unavailable")
        if self.base <= int(address) < self.base + image_size:
            return False
        self.loader_breakpoints_remaining -= 1
        self.loader_breakpoint_pending = self.loader_breakpoints_remaining > 0
        return True

    @staticmethod
    def _normalize_image_path(value: str | Path | None) -> str:
        """Normalize a Win32 image path for strict parent/child matching."""

        if value is None:
            return ""
        raw = str(value).strip().strip('"')
        # QueryFullProcessImageNameW may return a device-prefix spelling while
        # the authenticated request uses the ordinary DOS path.  This is a
        # presentation difference, not permission to accept a different file.
        for prefix in ("\\\\?\\", "\\??\\"):
            if raw.startswith(prefix):
                raw = raw[len(prefix):]
                break
        return os.path.normcase(os.path.normpath(raw))

    @classmethod
    def _same_image_path(cls, actual: str | Path | None, expected: str | Path | None) -> bool:
        return bool(actual and expected and cls._normalize_image_path(actual) == cls._normalize_image_path(expected))

    def _read_from_handle(self, handle: int, address: int, size: int) -> bytes:
        if int(handle) <= 0 or int(address) <= 0 or int(size) <= 0:
            raise Rejected("native remote read has an invalid handle or address")
        buffer = (ctypes.c_ubyte * int(size))()
        read = ctypes.c_size_t()
        ok = self.native.kernel32.ReadProcessMemory(
            ctypes.c_void_p(int(handle)),
            ctypes.c_void_p(int(address)),
            buffer,
            int(size),
            ctypes.byref(read),
        )
        if not ok or int(read.value) != int(size):
            raise Rejected("ReadProcessMemory failed while identifying debug image")
        return bytes(buffer)

    def _query_process_image_path(self, process_handle: int, info: Any) -> str:
        """Resolve a CREATE_PROCESS image without trusting its basename."""

        kernel32 = self.native.kernel32
        handle = ctypes.c_void_p(int(process_handle))
        query = getattr(kernel32, "QueryFullProcessImageNameW", None)
        if callable(query):
            buffer = ctypes.create_unicode_buffer(32768)
            length = wintypes.DWORD(len(buffer))
            try:
                if query(handle, 0, buffer, ctypes.byref(length)):
                    return buffer.value
            except (OSError, TypeError, ValueError):
                pass

        file_handle = _native_value(getattr(info, "hFile", 0))
        get_file_name = getattr(kernel32, "GetFinalPathNameByHandleW", None)
        if file_handle and callable(get_file_name):
            buffer = ctypes.create_unicode_buffer(32768)
            try:
                length = int(get_file_name(ctypes.c_void_p(file_handle), buffer, len(buffer), 0))
                if 0 < length < len(buffer):
                    return buffer.value[:length]
            except (OSError, TypeError, ValueError):
                pass

        # Last-resort DEBUG_EVENT image-name read.  The pointer is owned by the
        # child process and is used only for this path comparison.
        image_name = _native_value(getattr(info, "lpImageName", 0))
        if image_name:
            try:
                raw = self._read_from_handle(int(process_handle), image_name, 4096)
                if int(getattr(info, "fUnicode", 0) or 0):
                    direct = raw.decode("utf-16-le", errors="ignore").split("\0", 1)[0]
                else:
                    direct = raw.split(b"\0", 1)[0].decode("mbcs", errors="ignore")
                if direct:
                    return direct
            except (Rejected, UnicodeError):
                pass
        raise Rejected("debug CREATE_PROCESS image path is unavailable")

    def _authenticate_debug_image(self, process_id: int, image_path: str) -> str:
        """Classify one debugged image, accepting only wrapper or compiler."""

        if int(process_id) == self.transport_process_id:
            if self.direct_compiler_transport:
                if not self._same_image_path(image_path, self.compiler_path):
                    raise Rejected("direct debug transport image is not the authenticated compiler")
                if self.compiler_sha256:
                    compiler_file = Path(str(self.compiler_path))
                    if not compiler_file.exists() or sha256(compiler_file) != self.compiler_sha256:
                        raise Rejected("direct debug compiler bytes do not match authority")
                return "compiler"
            if not self._same_image_path(image_path, self.wrapper_path):
                raise Rejected("debug transport image is not the authenticated wrapper")
            if self.wrapper_sha256:
                wrapper_file = Path(str(self.wrapper_path))
                if not wrapper_file.exists() or sha256(wrapper_file) != self.wrapper_sha256:
                    raise Rejected("debug transport wrapper bytes do not match authority")
            return "wrapper"
        if self._same_image_path(image_path, self.compiler_path):
            if self.compiler_sha256:
                compiler_file = Path(str(self.compiler_path))
                if not compiler_file.exists() or sha256(compiler_file) != self.compiler_sha256:
                    raise Rejected("debug compiler bytes do not match authority")
            return "compiler"
        raise Rejected("unrecognized debugged image; compiler child was not authenticated")

    def classify_debug_image(self, process_id: int, image_path: str) -> str:
        """Testable closed classifier for wrapper-parent/compiler-child events."""

        return self._authenticate_debug_image(int(process_id), str(image_path))

    def _continue_debug_event(self, process_id: int, thread_id: int) -> None:
        if not self.native.kernel32.ContinueDebugEvent(
            int(process_id),
            int(thread_id),
            getattr(self.native, "DBG_CONTINUE", 0x00010002),
        ):
            raise Rejected("ContinueDebugEvent failed")
        if self._active_debug_event == (int(process_id), int(thread_id)):
            self._active_debug_event = None

    def _close_debug_file(self, info: Any) -> None:
        file_handle = _native_value(getattr(info, "hFile", 0))
        if file_handle:
            self.native.kernel32.CloseHandle(file_handle)

    def _record_process_create(self, process_id: int, thread_id: int, info: Any) -> None:
        process_handle = _native_value(getattr(info, "hProcess", 0))
        thread_handle = _native_value(getattr(info, "hThread", 0))
        if process_id <= 0 or process_handle <= 0 or thread_handle <= 0:
            raise Rejected("debug descendant CREATE_PROCESS event lacks handles")
        if process_id in self._observed_process_ids:
            existing = self._process_handles.get(process_id)
            if existing not in {None, process_handle}:
                raise Rejected("debug process identity reused a different handle")
        self._observed_process_ids.add(int(process_id))
        self._process_handles[int(process_id)] = int(process_handle)
        self._descendant_threads.setdefault(int(process_id), {})[int(thread_id)] = int(thread_handle)
        self._owned_handles.update((int(process_handle), int(thread_handle)))
        self._close_debug_file(info)

    def _record_process_exit(self, process_id: int) -> None:
        if process_id not in self._observed_process_ids:
            raise Rejected("debug process exit lacks an observed process")
        self._exited_process_ids.add(int(process_id))

    def terminal_process_summary(self) -> dict[str, Any]:
        open_processes = sorted(self._observed_process_ids.difference(self._exited_process_ids))
        return {
            "observed_process_ids": sorted(self._observed_process_ids),
            "exited_process_ids": sorted(self._exited_process_ids),
            "open_process_ids": open_processes,
            "open_thread_handle_count": len(self.threads) + len(self.transport_threads) + sum(
                len(rows) for rows in self._descendant_threads.values()
            ),
            "unclosed_handle_count": len(self._owned_handles),
            "active_debug_event": self._active_debug_event is not None,
        }

    def _select_compiler_process(
        self,
        process_id: int,
        thread_id: int,
        info: Any,
        image_path: str,
        session: CombinedCaptureSession,
    ) -> None:
        if self.compiler_selected:
            raise Rejected("compiler child was created more than once")
        if not self.direct_compiler_transport and not self._transport_image_seen:
            raise Rejected("compiler child appeared before wrapper authentication")
        if self.direct_compiler_transport and int(process_id) != self.transport_process_id:
            raise Rejected("direct compiler transport changed process identity")
        if self._authenticate_debug_image(process_id, image_path) != "compiler":
            raise Rejected("debug child is not the authenticated compiler")
        process_handle = _native_value(getattr(info, "hProcess", 0))
        thread_handle = _native_value(getattr(info, "hThread", 0))
        image_base = _native_value(getattr(info, "lpBaseOfImage", 0))
        if process_handle <= 0 or thread_handle <= 0 or image_base <= 0:
            raise Rejected("compiler CREATE_PROCESS event lacks authenticated handles")
        self.process = process_handle
        self.process_id = int(process_id)
        self.compiler_process_id = int(process_id)
        self.compiler_selected = True
        self._selection_mode = self.transport_mode
        if self.direct_compiler_transport:
            self._transport_image_seen = True
        self.base = image_base
        self.threads[int(thread_id)] = thread_handle
        self._owned_handles.update((process_handle, thread_handle))
        self._observed_process_ids.add(int(process_id))
        self._process_handles[int(process_id)] = int(process_handle)
        # A native 32-bit process under WOW64 produces both the native and
        # WOW64 debugger-init breakpoints before user code.  A normal child
        # transport has one.  Only same-PID, out-of-compiler-image events may
        # consume this bounded budget; any third or later breakpoint remains
        # a hard failure.
        self.loader_breakpoints_remaining = 2 if self.direct_compiler_transport else 1
        self.loader_breakpoint_pending = True
        self._pending_create_event = (int(process_id), int(thread_id))
        self._close_debug_file(info)
        session.on_process_started(int(process_id))

    def transport_provenance(self, request_argv: Sequence[str]) -> dict[str, Any]:
        """Return pointer-free execution provenance for the sealed context."""

        original = [str(value) for value in request_argv]
        expected = original[1:] if self.direct_compiler_transport else original
        if self.executed_argv != expected:
            raise Rejected("native executed argv diverged from authenticated transport plan")
        return {
            "mode": self.transport_mode,
            "argv": list(self.executed_argv),
            "request_argv_sha256": canonical_hash({"argv": original}),
            "wrapper_bypassed": self.direct_compiler_transport,
        }

    def _compiler_pe_shape(self) -> tuple[int, int]:
        """Authenticate and parse the compiler image metadata from disk."""

        if self.compiler_path is None:
            raise Rejected("native capture lacks an authenticated compiler path")
        path = Path(str(self.compiler_path))
        if not path.exists() or not path.is_file():
            raise Rejected("authenticated compiler image is unavailable")
        if self.compiler_sha256 and sha256(path) != self.compiler_sha256:
            raise Rejected("compiler bytes changed before same-process selection")
        try:
            data = path.read_bytes()
            pe_offset = int.from_bytes(data[0x3C:0x40], "little")
            if data[:2] != b"MZ" or pe_offset < 0x40 or pe_offset + 0x60 > len(data):
                raise ValueError
            if data[pe_offset:pe_offset + 4] != b"PE\0\0":
                raise ValueError
            optional = pe_offset + 4 + 20
            if int.from_bytes(data[optional:optional + 2], "little") != 0x10B:
                raise ValueError
            image_base = int.from_bytes(data[optional + 28:optional + 32], "little")
            image_size = int.from_bytes(data[optional + 56:optional + 60], "little")
        except (IndexError, ValueError, TypeError):
            raise Rejected("authenticated compiler image is not a valid PE32 image") from None
        if image_base <= 0 or image_size <= 0 or image_size > 0x80000000:
            raise Rejected("authenticated compiler PE image shape is invalid")
        self._compiler_preferred_base = image_base
        self._compiler_image_size = image_size
        return image_base, image_size

    def _compiler_relocation_rvas(self) -> tuple[int, ...]:
        """Return authenticated PE32 HIGHLOW relocation RVAs.

        The memexec loader applies the compiler's base-relocation table after
        copying sections.  Hook prefixes containing absolute addresses
        therefore differ from the preferred-base bytes on disk when the
        wrapper's own image occupies the preferred base.  The relocation table
        is read only from the already authenticated compiler file; no mapped
        process bytes are used to synthesize relocation metadata.
        """

        if self._compiler_relocation_rvas_cache is not None:
            return self._compiler_relocation_rvas_cache
        if self.compiler_path is None:
            self._compiler_relocation_rvas_cache = ()
            self._compiler_relocation_values_cache = {}
            return self._compiler_relocation_rvas_cache
        path = Path(str(self.compiler_path))
        if not path.exists() or not path.is_file():
            raise Rejected("authenticated compiler image is unavailable")
        if self.compiler_sha256 and sha256(path) != self.compiler_sha256:
            raise Rejected("compiler bytes changed before relocation authentication")
        data = path.read_bytes()
        try:
            pe_offset = int.from_bytes(data[0x3C:0x40], "little")
            if data[:2] != b"MZ" or pe_offset < 0x40 or pe_offset + 24 > len(data):
                raise ValueError
            if data[pe_offset:pe_offset + 4] != b"PE\0\0":
                raise ValueError
            file_header = pe_offset + 4
            section_count = int.from_bytes(data[file_header + 2:file_header + 4], "little")
            optional_size = int.from_bytes(data[file_header + 16:file_header + 18], "little")
            optional = file_header + 20
            if section_count <= 0 or optional_size < 96 or optional + optional_size > len(data):
                raise ValueError
            if int.from_bytes(data[optional:optional + 2], "little") != 0x10B:
                raise ValueError
            directory = optional + 96 + 8 * 5  # IMAGE_DIRECTORY_ENTRY_BASERELOC
            if directory + 8 > optional + optional_size:
                raise ValueError
            reloc_rva = int.from_bytes(data[directory:directory + 4], "little")
            reloc_size = int.from_bytes(data[directory + 4:directory + 8], "little")
            if reloc_rva == 0 or reloc_size == 0:
                self._compiler_relocation_rvas_cache = ()
                self._compiler_relocation_values_cache = {}
                return self._compiler_relocation_rvas_cache
            section_table = optional + optional_size
            sections: list[tuple[int, int, int, int]] = []
            for index in range(section_count):
                start = section_table + index * 40
                if start + 40 > len(data):
                    raise ValueError
                virtual_size = int.from_bytes(data[start + 8:start + 12], "little")
                virtual_address = int.from_bytes(data[start + 12:start + 16], "little")
                raw_size = int.from_bytes(data[start + 16:start + 20], "little")
                raw_pointer = int.from_bytes(data[start + 20:start + 24], "little")
                sections.append((virtual_address, max(virtual_size, raw_size), raw_pointer, raw_size))

            def raw_image_bytes(rva: int, size: int) -> bytes | None:
                """Read initialized image bytes, zero-filling authenticated BSS."""

                for virtual_address, span, raw_pointer, raw_size in sections:
                    if not (virtual_address <= rva and rva + size <= virtual_address + span):
                        continue
                    offset = rva - virtual_address
                    if offset >= raw_size:
                        return b"\0" * size
                    copied = min(size, raw_size - offset)
                    start = raw_pointer + offset
                    if start < 0 or start + copied > len(data):
                        return None
                    return data[start:start + copied] + b"\0" * (size - copied)
                return None

            raw_offset: int | None = None
            for virtual_address, span, raw_pointer, raw_size in sections:
                if virtual_address <= reloc_rva < virtual_address + span:
                    delta = reloc_rva - virtual_address
                    if delta >= raw_size:
                        raise ValueError
                    raw_offset = raw_pointer + delta
                    break
            if raw_offset is None or raw_offset < 0 or raw_offset + reloc_size > len(data):
                raise ValueError
            result: list[int] = []
            relocation_values: dict[int, int] = {}
            cursor = raw_offset
            end = raw_offset + reloc_size
            while cursor < end:
                if cursor + 8 > end:
                    raise ValueError
                page_rva = int.from_bytes(data[cursor:cursor + 4], "little")
                block_size = int.from_bytes(data[cursor + 4:cursor + 8], "little")
                if block_size < 8 or cursor + block_size > end or (block_size - 8) % 2:
                    raise ValueError
                for item_offset in range(cursor + 8, cursor + block_size, 2):
                    item = int.from_bytes(data[item_offset:item_offset + 2], "little")
                    if item >> 12 == 3:  # IMAGE_REL_BASED_HIGHLOW
                        relocation_rva = page_rva + (item & 0x0FFF)
                        result.append(relocation_rva)
                        raw_value = raw_image_bytes(relocation_rva, 4)
                        if raw_value is not None:
                            relocation_values[relocation_rva] = int.from_bytes(raw_value, "little")
                cursor += block_size
            self._compiler_relocation_rvas_cache = tuple(result)
            self._compiler_relocation_values_cache = relocation_values
            return self._compiler_relocation_rvas_cache
        except (IndexError, TypeError, ValueError):
            raise Rejected("authenticated compiler relocation table is malformed") from None

    def _mapped_hook_prefix(self, row: Mapping[str, Any], base: int) -> bytes:
        """Apply authenticated HIGHLOW deltas to one hook prefix."""

        expected = bytearray(bytes.fromhex(str(row["prefix"])))
        preferred = self._compiler_preferred_base
        relocations = self._compiler_relocation_rvas_cache
        relocation_values = self._compiler_relocation_values_cache
        if preferred is None or relocations is None or base == preferred:
            return bytes(expected)
        hook_rva = int(row["address"]) - preferred
        delta = int(base) - preferred
        for relocation_rva in relocations:
            offset = relocation_rva - hook_rva
            if offset >= len(expected) or offset + 4 <= 0:
                continue
            value = None if relocation_values is None else relocation_values.get(relocation_rva)
            if value is None:
                # The v5/v6 unit shape supplies the complete operand in the
                # pinned prefix.  Production prefixes may end in the middle
                # of an authenticated relocation; those require the raw
                # operand cached from the authenticated PE parser above.
                if offset < 0 or offset + 4 > len(expected):
                    continue
                value = int.from_bytes(expected[offset:offset + 4], "little")
            patched = ((value + delta) & 0xFFFFFFFF).to_bytes(4, "little")
            start = max(0, offset)
            end = min(len(expected), offset + 4)
            expected[start:end] = patched[start - offset:end - offset]
        return bytes(expected)

    @staticmethod
    def _pointer_value(value: Any) -> int:
        return _native_value(value)

    def _memory_regions(self) -> Iterable[tuple[int, int]]:
        """Yield unique committed private allocation bases and sizes."""

        query = getattr(self.native.kernel32, "VirtualQueryEx", None)
        if not callable(query):
            raise Rejected("native backend lacks VirtualQueryEx for same-process compiler discovery")
        address = 0
        limit = 0x100000000
        seen: set[int] = set()
        info_size = ctypes.sizeof(_MEMORY_BASIC_INFORMATION)
        while address < limit:
            info = _MEMORY_BASIC_INFORMATION()
            result = query(
                ctypes.c_void_p(int(self.process)),
                ctypes.c_void_p(address),
                ctypes.byref(info),
                info_size,
            )
            if not result:
                error = ctypes.get_last_error()
                if error in (0, 87, 487):
                    break
                raise Rejected(f"VirtualQueryEx failed during compiler discovery: {error}")
            region_base = self._pointer_value(info.BaseAddress)
            region_size = self._pointer_value(info.RegionSize)
            allocation_base = self._pointer_value(info.AllocationBase)
            if region_size <= 0 or region_base < address:
                raise Rejected("VirtualQueryEx returned an invalid region")
            if int(info.State) == MEM_COMMIT and int(info.Type) == MEM_PRIVATE and allocation_base and allocation_base not in seen:
                seen.add(allocation_base)
                yield allocation_base, region_size
            next_address = region_base + region_size
            if next_address <= address:
                raise Rejected("VirtualQueryEx region walk did not advance")
            address = next_address

    def _memexec_candidate_matches(self, base: int, image_size: int) -> bool:
        """Check every authenticated hook in a section-only memexec image.

        ``sjiswrap`` uses ``memexec_exe_with_hooks``.  That loader allocates
        the image and copies PE sections, but deliberately does not copy the
        DOS/PE headers.  The on-disk compiler descriptor and hash are already
        authenticated by :meth:`_compiler_pe_shape`; the mapped image must
        therefore be identified by its complete pinned hook union rather than
        by looking for an ``MZ`` header that is not present in a valid map.
        """

        if base <= 0 or base % 0x1000 or base + image_size > 0x100000000:
            return False
        for row in _hooks_for_compiler(self.compiler_sha256 or ""):
            address = base + int(row["address"]) - KNOWN_IMAGE_BASE
            expected = self._mapped_hook_prefix(row, base)
            try:
                actual = self._read_from_handle(self.process, address, len(expected))
            except Rejected:
                return False
            if actual != expected:
                return False
        return True

    def _discover_memexec_image_base(self) -> int | None:
        """Find the authenticated compiler's private same-process mapping."""

        if self._memexec_image_base is not None:
            return self._memexec_image_base
        _preferred_base, image_size = self._compiler_pe_shape()
        # Authenticate the relocation table before checking section-only hook
        # bytes.  A valid memexec map is commonly rebased away from the
        # compiler's preferred image base, so absolute HIGHLOW operands in the
        # pinned prefixes must be adjusted before the candidate loop.
        self._compiler_relocation_rvas()
        for allocation_base, _region_size in self._memory_regions():
            if self._memexec_candidate_matches(allocation_base, image_size):
                self._memexec_image_base = allocation_base
                return allocation_base
        return None

    def _request_memexec_probe(self, *, followup: bool = False) -> bool:
        """Interrupt a running wrapper so its private image can be inspected.

        ``sjiswrap`` can map and enter the compiler without producing another
        debug event.  Waiting passively in that state races the wrapper's
        process-exit event: the compiler image is then gone before the
        handoff code gets a chance to authenticate it.  Once the wrapper
        CREATE_PROCESS event has been continued, or when a wait times out,
        the debuggee is running with no pending event; a
        ``DebugBreakProcess`` request is the narrow, authenticated way to
        create an observation point in the same PID.  One request is made at
        wrapper release and one bounded follow-up may be made after a worker
        CREATE_THREAD event.  The latter closes the real wrapper/worker event
        gap without turning an unrecognized map into an unbounded stream of
        debug breaks.  A false return is not promoted to a compiler identity;
        the normal bounded loop remains fail-closed and will report the
        transport failure if the process really has exited.
        """

        debug_break = getattr(self.native.kernel32, "DebugBreakProcess", None)
        if self._memexec_probe_count >= MEMEXEC_MAX_PROBES or not callable(debug_break):
            return False
        # Timeout polling may happen repeatedly before the first probe event;
        # it must not silently consume the one worker-gap follow-up budget.
        if self._memexec_probe_count and not followup:
            return False
        try:
            requested = bool(debug_break(ctypes.c_void_p(int(self.process))))
        except (OSError, TypeError, ValueError):
            return False
        if requested:
            self._memexec_probe_requested = True
            self._memexec_probe_count += 1
        return requested

    def _pause_same_process_target(self, event_type: Any) -> None:
        """Pause the manual-mapped compiler before preflight mutates hooks."""

        debug_break = getattr(self.native.kernel32, "DebugBreakProcess", None)
        if not callable(debug_break) or not debug_break(ctypes.c_void_p(int(self.process))):
            error = ctypes.get_last_error()
            raise Rejected(f"DebugBreakProcess failed before same-process preflight: {error}")
        event = event_type()
        while True:
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == getattr(self.native, "ERROR_SEM_TIMEOUT", 121):
                    raise Rejected("same-process compiler pause timed out")
                raise Rejected(f"WaitForDebugEvent failed while pausing compiler: {error}")
            pid = int(event.dwProcessId)
            tid = int(event.dwThreadId)
            self._active_debug_event = (pid, tid)
            if pid != self.transport_process_id:
                raise Rejected("same-process pause reported an unauthenticated process")
            code = int(event.dwDebugEventCode)
            if code == getattr(self.native, "EXCEPTION_DEBUG_EVENT", 1):
                thread_handle = self.transport_threads.get(tid)
                if thread_handle:
                    self.threads[tid] = thread_handle
                    self._owned_handles.add(thread_handle)
                self._pending_debug_event = (pid, tid)
                return
            if code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                raise Rejected("compiler exited before same-process preflight")
            self._continue_debug_event(pid, tid)

    def _select_memexec_process(self, image_base: int, session: CombinedCaptureSession, event_type: Any) -> None:
        """Bind the authenticated compiler image while retaining wrapper PID."""

        if self.compiler_selected:
            raise Rejected("compiler image was selected more than once")
        if not self._transport_image_seen:
            raise Rejected("compiler image appeared before wrapper authentication")
        if image_base <= 0:
            raise Rejected("same-process compiler image has no valid base")
        # The compiler bytes were hashed above, and all pinned hook prefixes
        # were matched against the private mapping before this binding.
        self.process_id = self.transport_process_id
        self.compiler_process_id = self.transport_process_id
        self.compiler_selected = True
        self._selection_mode = "same_process_memexec"
        self.base = int(image_base)
        self.loader_breakpoint_pending = False
        self.loader_breakpoints_remaining = 0
        session.on_process_started(self.process_id)
        self._pause_same_process_target(event_type)
        # A same-PID compiler can create its worker before the bounded pause.
        # The pause event identifies only the thread that delivered that
        # observation; the first owned hook may execute on another retained
        # worker.  Promote every live transport handle at the selection
        # boundary so single_step/read_register can resolve either thread.
        for thread_id, thread_handle in self.transport_threads.items():
            if thread_id > 0:
                self.threads.setdefault(thread_id, thread_handle)

    def prepare_capture(self, session: CombinedCaptureSession) -> None:
        """Authenticate wrapper and compiler before the first hook read.

        The normal ``DEBUG_PROCESS`` child path is retained for launchers that
        really create a compiler child.  sjiswrap v1.1.1 instead uses
        ``memexec_exe_with_hooks``: after the wrapper CREATE_PROCESS event, the
        authenticated compiler is a private image in the *same* PID.  Polling
        VirtualQueryEx between debug events makes that handoff explicit and
        fail-closed without trusting a basename or an unbound address.
        """

        if self.compiler_selected:
            return
        if not self.compiler_path or (not self.direct_compiler_transport and not self.wrapper_path):
            raise Rejected("native capture lacks authenticated wrapper/compiler paths")
        event_type = getattr(self.native, "DEBUG_EVENT", None)
        if event_type is None:
            raise Rejected("native DEBUG_EVENT layout is unavailable")
        event = event_type()
        deadline = time.monotonic() + MEMEXEC_STARTUP_TIMEOUT_SECONDS
        while not self.compiler_selected:
            if time.monotonic() >= deadline:
                if self._transport_exited:
                    raise Rejected("wrapper transport exited before compiler selection")
                raise Rejected("native debug transport gap before compiler selection")
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == getattr(self.native, "ERROR_SEM_TIMEOUT", 121):
                    if self._transport_image_seen and not self._transport_exited:
                        image_base = self._discover_memexec_image_base()
                        if image_base is not None:
                            self._select_memexec_process(image_base, session, event_type)
                            return
                        # The wrapper may be executing the manually mapped
                        # compiler without generating another debug event.
                        # Force one bounded same-PID observation before the
                        # next wait; otherwise a fast wrapper can reach its
                        # exit event while the handoff is still invisible.
                        self._request_memexec_probe()
                    if time.monotonic() >= deadline:
                        if self._transport_exited:
                            raise Rejected("wrapper transport exited before compiler selection")
                        raise Rejected("native debug transport gap before compiler selection")
                    continue
                raise Rejected(f"WaitForDebugEvent failed before compiler selection: {error}")
            code = int(event.dwDebugEventCode)
            pid = int(event.dwProcessId)
            tid = int(event.dwThreadId)
            self._active_debug_event = (pid, tid)
            if code == getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3):
                info = event.u.CreateProcessInfo
                process_handle = _native_value(getattr(info, "hProcess", 0))
                if process_handle <= 0:
                    raise Rejected("CREATE_PROCESS event lacks a process handle")
                image_path = self._query_process_image_path(process_handle, info)
                role = self._authenticate_debug_image(pid, image_path)
                if role == "wrapper":
                    if pid != self.transport_process_id or self._transport_image_seen:
                        raise Rejected("wrapper transport process identity changed")
                    self._transport_image_seen = True
                    wrapper_thread = _native_value(getattr(info, "hThread", 0))
                    if wrapper_thread:
                        self.transport_threads.pop(0, None)
                        self.transport_threads[tid] = wrapper_thread
                        self._owned_handles.add(wrapper_thread)
                    self._owned_handles.add(process_handle)
                    self._close_debug_file(info)
                    self._continue_debug_event(pid, tid)
                    image_base = self._discover_memexec_image_base()
                    if image_base is not None:
                        self._select_memexec_process(image_base, session, event_type)
                        return
                    # A same-PID launcher can exit immediately after its
                    # CREATE_PROCESS event.  The timeout path below is too
                    # late for that transport race, so request one bounded
                    # observation while the authenticated wrapper is still
                    # known to own the debug transport.
                    self._request_memexec_probe()
                    continue
                self._select_compiler_process(pid, tid, info, image_path, session)
                # Keep this CREATE_PROCESS event pending.  Dispatcher preflight
                # and all INT3 writes must happen while the authenticated child
                # is paused at creation, before its first instruction runs.
                return
            if pid == self.transport_process_id:
                if not self._transport_image_seen:
                    raise Rejected("wrapper event arrived before wrapper authentication")
                if code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2):
                    # A manually mapped compiler can create its worker thread
                    # before the bounded same-PID pause below.  That thread
                    # still belongs to the authenticated debug transport, but
                    # it does not emit a second CREATE_PROCESS event.  Retain
                    # its event-owned handle so the first hook/single-step can
                    # read and update the correct WOW64 context.
                    thread_handle = _native_value(getattr(event.u.CreateThread, "hThread", 0))
                    if thread_handle:
                        self.transport_threads[tid] = thread_handle
                        self._owned_handles.add(thread_handle)
                elif code == getattr(self.native, "EXIT_THREAD_DEBUG_EVENT", 4):
                    thread_handle = self.transport_threads.pop(tid, None)
                    if thread_handle:
                        self.native.kernel32.CloseHandle(thread_handle)
                        self._owned_handles.discard(thread_handle)
                elif code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                    # A real launcher may detach/exit after handing off a
                    # compiler child.  Continue consuming the one debug port
                    # so a queued authenticated child CREATE_PROCESS event is
                    # still selectable.  The bounded startup deadline keeps a
                    # same-process wrapper exit fail-closed.
                    self._transport_exited = True
                    self._record_process_exit(pid)
                self._continue_debug_event(pid, tid)
                if not self._transport_exited:
                    image_base = self._discover_memexec_image_base()
                    if image_base is not None:
                        self._select_memexec_process(image_base, session, event_type)
                        return
                if self._transport_exited and time.monotonic() >= deadline:
                    raise Rejected("wrapper transport exited before compiler selection")
                # The wrapper's first probe can arrive before a worker has
                # finished entering the manually mapped compiler.  Request one
                # additional observation only for that authenticated worker
                # event; the probe budget keeps this fail-closed.
                if code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2) and not self._transport_exited:
                    self._request_memexec_probe(followup=True)
                continue
            # A normal child CREATE_PROCESS event is handled above.  Any
            # other PID before selection remains an unauthenticated process;
            # do not silently reinterpret it as the compiler.
            raise Rejected("unexpected debug process before compiler selection")

    def _read(self, address: int, size: int) -> bytes:
        buffer = (ctypes.c_ubyte * size)()
        read = ctypes.c_size_t()
        ok = self.native.kernel32.ReadProcessMemory(self.process, ctypes.c_void_p(address), buffer, size, ctypes.byref(read))
        if not ok or int(read.value) != size:
            raise Rejected(f"ReadProcessMemory failed at 0x{address:08x}")
        return bytes(buffer)

    def _u32(self, address: int, label: str) -> int:
        data = self._read(address, 4)
        value = int.from_bytes(data, "little", signed=False)
        if value == 0:
            raise Rejected(f"{label} is null")
        return value

    def _u32_optional(self, address: int) -> int:
        data = self._read(address, 4)
        return int.from_bytes(data, "little", signed=False)

    def _u8(self, address: int, label: str) -> int:
        data = self._read(address, 1)
        if len(data) != 1:
            raise Rejected(f"{label} is truncated")
        return data[0]

    def _get_context(self, handle: int) -> Any:
        context_type = getattr(self.native, "WOW64_CONTEXT", None)
        if context_type is None:
            raise Rejected("WOW64 context layout is unavailable")
        context = context_type()
        context.ContextFlags = getattr(self.native, "WOW64_CONTEXT_FULL", 0x00010007)
        if not self.native.kernel32.Wow64GetThreadContext(handle, ctypes.byref(context)):
            raise Rejected("Wow64GetThreadContext failed")
        return context

    def _read_name(self, object_pointer: int) -> str:
        name_pointer = self._u32_optional(object_pointer + OBJECT_NAME)
        if not name_pointer:
            return ""
        data = self._read(name_pointer + 0x0A, 256).split(b"\0", 1)[0]
        return data.decode("latin-1", errors="replace")

    def current_function(self) -> str:
        """Read the compiler's current function name at the filter hook.

        The name is used only for the in-process filter.  It is never emitted
        into the event ledger as an ownership claim; an absent name is an
        empty result and therefore fails the requested-function filter.
        """

        function_object = self._u32_optional(self._runtime(FUNCTION_OBJECT))
        if not function_object:
            return ""
        return self._read_name(function_object)

    def read_register(self, thread_id: int, name: str) -> int:
        """Read one closed-layout x86 register from the paused WOW64 thread."""

        register_names = {
            "eax": "Eax",
            "ebx": "Ebx",
            "ecx": "Ecx",
            "edx": "Edx",
            "esi": "Esi",
            "edi": "Edi",
            "ebp": "Ebp",
            "esp": "Esp",
            "eip": "Eip",
        }
        field = register_names.get(str(name).lower())
        if field is None:
            raise Rejected(f"unsupported WOW64 register layout: {name!r}")
        handle = self.threads.get(int(thread_id))
        if handle is None:
            raise Rejected("WOW64 thread handle missing")
        context = self._get_context(handle)
        value = getattr(context, field, None)
        if value is None:
            raise Rejected(f"WOW64 context lacks {field}")
        return int(value) & 0xFFFFFFFF

    def _snapshot_list(self, list_head: int, kind: str) -> list[dict[str, Any]]:
        if kind not in {"local", "argument"}:
            raise Rejected("unsupported compiler Object-list kind")
        head = self._u32_optional(self._runtime(list_head))
        rows: list[dict[str, Any]] = []
        seen_nodes: set[int] = set()
        seen_objects: set[int] = set()
        while head:
            if len(rows) >= 4096:
                raise Rejected("native compiler Object list exceeded bound")
            if head in seen_nodes:
                raise Rejected("native compiler Object list contains a cycle")
            seen_nodes.add(head)
            node = self._read(head, 8)
            next_node = int.from_bytes(node[0:4], "little", signed=False)
            object_pointer = int.from_bytes(node[4:8], "little", signed=False)
            if not object_pointer:
                raise Rejected("native compiler Object list has a null Object")
            if object_pointer in seen_objects:
                raise Rejected("native compiler Object list reuses an Object")
            if object_pointer in self._object_kind and self._object_kind[object_pointer] != kind:
                raise Rejected("Object appears in both local and argument lists")
            seen_objects.add(object_pointer)
            datatype = self._u8(object_pointer + OBJECT_DATATYPE, "Object datatype")
            varinfo_offset = OBJECT_VARINFO_DATATYPE1 if datatype == 1 else OBJECT_VARINFO_OTHER
            varinfo_pointer = self._u32(object_pointer + varinfo_offset, "Object VarInfo pointer")
            prior_object = self._varinfo_object.get(varinfo_pointer)
            if prior_object not in (None, object_pointer):
                raise Rejected("VarInfo pointer is reused by another Object")
            prior_varinfo = self._object_varinfo.get(object_pointer)
            if prior_varinfo not in (None, varinfo_pointer):
                raise Rejected("Object changed VarInfo identity")
            self._object_kind[object_pointer] = kind
            self._object_varinfo[object_pointer] = varinfo_pointer
            self._varinfo_object[varinfo_pointer] = object_pointer
            rows.append(
                {
                    "pointer": object_pointer,
                    "varinfo_pointer": varinfo_pointer,
                    "name": self._read_name(object_pointer),
                    "datatype": datatype,
                }
            )
            head = next_node
        return rows

    def snapshot_inventory(self) -> Mapping[str, Sequence[Mapping[str, Any]]]:
        """Read both compiler-owned Object lists while the target is paused."""

        locals_rows = self._snapshot_list(LOCALS_LIST_HEAD, "local")
        arguments_rows = self._snapshot_list(ARGUMENTS_LIST_HEAD, "argument")
        return {"locals": locals_rows, "arguments": arguments_rows}

    def snapshot_objects(self, list_head: int | None = None) -> Sequence[Mapping[str, Any]]:
        """Compatibility donor primitive for one authenticated list head."""

        selected = LOCALS_LIST_HEAD if list_head is None else int(list_head)
        kind = "argument" if selected == ARGUMENTS_LIST_HEAD else "local"
        if selected not in {LOCALS_LIST_HEAD, ARGUMENTS_LIST_HEAD}:
            raise Rejected("unowned compiler Object-list head")
        return self._snapshot_list(selected, kind)

    def snapshot_varinfo(self, pointer: int) -> Mapping[str, Any]:
        """Read only the authenticated VarInfo home field used by the donor."""

        if int(pointer) <= 0 or int(pointer) not in self._varinfo_object:
            raise Rejected("VarInfo pointer is not in the captured Object lists")
        data = self._read(int(pointer), VARINFO_RAW_SIZE)
        home = int.from_bytes(data[VARINFO_HOME_FIELD:VARINFO_HOME_FIELD + 2], "little", signed=True)
        return {"home_value": home}

    def capture_stack_write(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        """Capture the authenticated EBX/EAX Object+0x2e write pair."""

        hook = HOOK_BY_ID.get(str(hook_id))
        layout = STACK_WRITE_REGISTER_LAYOUTS.get(str(hook_id))
        if hook is None or hook.get("role") != "object_stack_write" or layout is None:
            raise Rejected("unowned stack-write hook")
        object_pointer = self.read_register(thread_id, layout["object"])
        value = self.read_register(thread_id, layout["value"])
        if object_pointer == 0:
            raise Rejected("Object write has a null Object register")
        result: dict[str, Any] = {"object": object_pointer, "value": value}
        kind = self._object_kind.get(object_pointer)
        if kind is not None:
            result["kind"] = kind
        return result

    def capture_frontend(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        """Capture one GC/2.6 frontend observation in the same paused process."""

        if self.compiler_sha256 != GC26_COMPILER_SHA256:
            raise Rejected("frontend hook is not authenticated for this compiler")
        hook = next((row for row in GC26_FRONTEND_HOOKS if str(row["id"]) == str(hook_id)), None)
        if hook is None:
            raise Rejected("unowned frontend hook")
        if hook_id in _frontend_chronology.GENERIC_HOOK_IDS:
            register = _frontend_chronology.GENERIC_OBJECT_REGISTERS.get(str(hook_id))
            if register is None:
                raise Rejected("frontend generic hook has no Object register")
            pointer = self.read_register(thread_id, register)
            if pointer == 0:
                raise Rejected("frontend generic hook returned a null Object")
            return {"pointer": pointer}
        if hook_id == "bulk_object_link":
            rows = self.snapshot_objects()
            pointers = [row.get("pointer") for row in rows]
            if not pointers or any(pointer is None for pointer in pointers):
                raise Rejected("frontend bulk-link snapshot is incomplete")
            return {"object_pointers": pointers}
        raise Rejected("unsupported frontend hook")

    def _read_direct_vreg_evidence(self, hook_id: str, thread_id: int) -> list[dict[str, Any]]:
        """Read an optional authenticated direct Object-to-vreg table.

        The stock GC/2.6 ctypes transport does not expose compiler-internal
        PCode/IG structures.  A transport that has separately authenticated
        that layout may provide ``read_direct_vreg_evidence`` on the same
        native adapter.  We accept only a one-to-one table tied to Objects
        already present in this session; names, list order, and physical homes
        are deliberately not accepted as substitutes.
        """

        reader = getattr(self.native, "read_direct_vreg_evidence", None)
        if not callable(reader):
            return []
        try:
            raw = reader(str(hook_id), int(thread_id))
        except TypeError:
            raw = reader(str(hook_id))
        if isinstance(raw, Mapping):
            if raw.get("status") in {"UNKNOWN", "INCOMPLETE"}:
                return []
            raw = raw.get("rows")
        if not isinstance(raw, Sequence) or isinstance(raw, (str, bytes, bytearray)):
            raise Rejected("direct Object-to-vreg evidence is not a row sequence")
        result: list[dict[str, Any]] = []
        seen_objects: set[int] = set()
        seen_ig_nodes: set[int] = set()
        seen_vregs: set[str] = set()
        for index, item in enumerate(raw):
            if not isinstance(item, Mapping):
                raise Rejected(f"direct Object-to-vreg row {index} is not an object")
            pointer = item.get("object", item.get("object_pointer"))
            ig_node = item.get("ig_node", item.get("ig_node_id"))
            vreg = item.get("vreg_id", item.get("virtual_register"))
            bank_raw = item.get("bank", item.get("register_class"))
            if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer <= 0:
                raise Rejected("direct Object-to-vreg evidence has an invalid Object")
            if pointer not in self._object_kind:
                raise Rejected("direct Object-to-vreg evidence references an uncaptured Object")
            if isinstance(ig_node, bool) or not isinstance(ig_node, int) or ig_node <= 0:
                raise Rejected("direct Object-to-vreg evidence has an invalid IG node")
            if not isinstance(bank_raw, str) or bank_raw.lower() not in {"gpr", "fpr"}:
                raise Rejected("direct Object-to-vreg evidence lacks a GPR/FPR bank")
            bank = bank_raw.lower()
            vreg_id = _validate_vreg(vreg, "direct vreg_id")
            if not vreg_id.startswith("r" if bank == "gpr" else "f"):
                raise Rejected("direct Object-to-vreg bank conflicts with vreg")
            if pointer in seen_objects:
                raise Rejected("direct Object-to-vreg evidence maps one Object more than once")
            if ig_node in seen_ig_nodes:
                raise Rejected("direct Object-to-vreg evidence reuses an IG node")
            if vreg_id in seen_vregs:
                raise Rejected("direct Object-to-vreg evidence reuses a vreg")
            seen_objects.add(pointer)
            seen_ig_nodes.add(ig_node)
            seen_vregs.add(vreg_id)
            result.append(
                {
                    "object": pointer,
                    "ig_node": ig_node,
                    "vreg_id": vreg_id,
                    "bank": bank.upper(),
                }
            )
        return result

    def capture_pcode(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        """Capture only a hook-bound PCode stage, never guessed ownership."""

        hook = HOOK_BY_ID.get(str(hook_id))
        if hook is None or hook.get("lane") != "pcode" or hook.get("role") == "regalloc":
            raise Rejected("unowned PCode hook")
        if hook.get("role") == "pcode_color_diagnostic":
            return self._capture_pcode_color(str(hook_id), int(thread_id))
        # A stage marker is useful chronology, but it is not direct ownership
        # evidence.  The latter is emitted only by capture_regalloc after a
        # one-to-one Object/IG/vreg table has been validated.
        self._pcode_events.append({"hook_id": str(hook_id), "thread": int(thread_id)})
        reader = getattr(self.native, "read_direct_pcode_stage", None)
        if not callable(reader):
            return {"status": "UNKNOWN", "reason": "incomplete PCode evidence", "stage": str(hook_id)}
        try:
            raw = reader(str(hook_id), int(thread_id))
        except TypeError:
            raw = reader(str(hook_id))
        if not isinstance(raw, Mapping):
            return {"status": "UNKNOWN", "reason": "incomplete PCode evidence", "stage": str(hook_id)}
        status = raw.get("status")
        if status != "CAPTURED":
            return {"status": "UNKNOWN", "reason": "incomplete PCode evidence", "stage": str(hook_id)}
        # Only the closed, pointer-free stage fields may cross into the event
        # bus.  A reader's address/pointer/thread fields are never copied.
        result: dict[str, Any] = {"status": "CAPTURED", "stage": str(hook_id)}
        for key in ("opcode", "instruction", "pcode_token", "source_offset", "block", "order", "operands"):
            if key in raw:
                result[key] = raw[key]
        _pointer_free(result)
        return result

    def _capture_pcode_color(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        if self.compiler_sha256 != GC27_COMPILER_SHA256:
            raise Rejected("PCode color hook is not authenticated for this compiler")
        if hook_id == "pcode_color_pre":
            if thread_id in self._pending_pcode_color:
                raise Rejected("nested PCode color writeback")
            operand_pointer = self.read_register(thread_id, "edx")
            pcode_pointer = self.read_register(thread_id, "esi")
            ig_pointer = self.read_register(thread_id, "ecx")
            remaining = self.read_register(thread_id, "ebp")
            if min(operand_pointer, pcode_pointer, ig_pointer) <= 0:
                raise Rejected("PCode color writeback has a null identity")
            header = self._read(pcode_pointer + 0x22, 2)
            operand = self._read(operand_pointer, 6)
            if len(header) != 2 or len(operand) != 6:
                raise Rejected("PCode color writeback record is truncated")
            operand_count = int.from_bytes(header, "little", signed=True)
            if not 1 <= operand_count <= 256 or not 0 <= remaining < operand_count:
                raise Rejected("PCode color writeback count is invalid")
            ordinal = operand_count - 1 - remaining
            if operand_pointer != pcode_pointer + 0x24 + ordinal * 0xC:
                raise Rejected("PCode color operand chronology is misaligned")
            operand_index = int.from_bytes(operand[4:6], "little", signed=True)
            final_color = self.read_register(thread_id, "eax") & 0xFFFF
            if final_color >= 0x8000:
                final_color -= 0x10000
            node = self._read(ig_pointer, 0x18)
            if len(node) != 0x18:
                raise Rejected("PCode IG node is truncated")
            row = {
                "pcode_pointer": pcode_pointer,
                "ig_pointer": ig_pointer,
                "operand_ordinal": ordinal,
                "operand_count": operand_count,
                "operand_kind": int(operand[0]),
                "register_class": int(operand[1]),
                "operand_index": operand_index,
                "final_color": final_color,
                "ig_flags": int.from_bytes(node[0x16:0x18], "little", signed=False),
                "object_pointer": int.from_bytes(node[0x4:0x8], "little", signed=False),
            }
            ig_color = int.from_bytes(node[0x14:0x16], "little", signed=True)
            if not 0 <= operand_index <= 0x7FFF or not 0 <= final_color <= 31 or ig_color != final_color:
                raise Rejected("PCode color operand/result conflicts with its IG node")
            self._pending_pcode_color[thread_id] = {**row, "operand_pointer": operand_pointer}
            return {"status": "PENDING", **row}
        if hook_id != "pcode_color_post":
            raise Rejected("unowned PCode color hook")
        row = self._pending_pcode_color.pop(thread_id, None)
        if row is None:
            return {"status": "NOOP"}
        stored = self._read(int(row.pop("operand_pointer")) + 4, 2)
        if len(stored) != 2 or int.from_bytes(stored, "little", signed=True) != row["final_color"]:
            raise Rejected("PCode color post-write value mismatch")
        return {"status": "CAPTURED", **row}

    def capture_machine_emission(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        """Read the authenticated GC/2.7 post-encoder machine event."""

        hook = HOOK_BY_ID.get(str(hook_id))
        if hook is None or hook.get("role") != "machine_emit":
            raise Rejected("unowned machine-emission hook")
        if self.compiler_sha256 != GC27_COMPILER_SHA256:
            raise Rejected("machine-emission hook is not authenticated for this compiler")
        pcode_pointer = self.read_register(thread_id, "ebx")
        emitted_offset = self.read_register(thread_id, "ebp")
        encoded_value = self.read_register(thread_id, "eax")
        if pcode_pointer == 0:
            return {"status": "UNKNOWN", "reason": "missing PCode token"}
        opcode_enum = int.from_bytes(self._read(pcode_pointer + 0x20, 2), "little", signed=True)
        if not 0 <= opcode_enum <= 0x1D4:
            return {"status": "UNKNOWN", "reason": "unsupported machine operand"}
        descriptor = self._runtime(
            GC27_OPCODE_DESCRIPTOR_TABLE
            + opcode_enum * GC27_OPCODE_DESCRIPTOR_STRIDE
            + GC27_OPCODE_DESCRIPTOR_BASE_OFFSET
        )
        descriptor_base = int.from_bytes(self._read(descriptor, 4), "little", signed=False)
        return {
            "pcode_pointer": pcode_pointer,
            "emitted_offset": emitted_offset,
            "opcode_enum": opcode_enum,
            "encoded_value": encoded_value,
            "descriptor_base": descriptor_base,
        }

    def capture_regalloc(self, hook_id: str, thread_id: int) -> Sequence[Mapping[str, Any]]:
        """Return direct Object-to-vreg rows only when the table is proven."""

        hook = HOOK_BY_ID.get(str(hook_id))
        if hook is None or hook.get("role") != "regalloc":
            raise Rejected("unowned register-allocation hook")
        rows = self._read_direct_vreg_evidence(str(hook_id), int(thread_id))
        self._direct_vreg_evidence = {int(row["object"]): row for row in rows}
        # The session consumes these internal pointers immediately and emits
        # capture-local tokens.  No source name or physical register is ever
        # synthesized here.
        return rows

    def capture_physical_regalloc(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        """Read one compiler-profile-specific post-allocation Object/VarInfo pair.

        ``regalloc_post`` lands on the GC/2.6 epilogue at 0x004D03E8.  At
        that site EBX still names the compiler Object and EBP still names its
        VarInfo.  GC/2.7's pair/single epilogues instead retain Object* in ESI
        and the just-written VarInfo* in EAX.  Its precolored loop retains a
        list node in EBX (Object* at +0x04) and VarInfo* in ESI.  Only the five
        authenticated VarInfo fields cross this backend boundary; all native
        identities are checked against the captured list ledger first.
        """

        hook = HOOK_BY_ID.get(str(hook_id))
        if hook is None or hook.get("role") != "regalloc_post":
            raise Rejected("unowned physical register-allocation hook")
        commit: dict[str, Any] = {}
        if self.compiler_sha256 == GC27_COMPILER_SHA256:
            if hook_id in {"physical_pair_commit", "physical_single_commit"}:
                object_pointer = self.read_register(thread_id, "esi")
                varinfo_pointer = self.read_register(thread_id, "eax")
                raw_bp = self.read_register(thread_id, "ebp") & 0xFFFF
                live_reg = raw_bp - 0x10000 if raw_bp >= 0x8000 else raw_bp
                if hook_id == "physical_pair_commit":
                    raw_bx = self.read_register(thread_id, "ebx") & 0xFFFF
                    live_reg_hi = raw_bx - 0x10000 if raw_bx >= 0x8000 else raw_bx
                    commit = {
                        "register_class": 4,
                        "reg": live_reg,
                        "reg_hi": live_reg_hi,
                    }
                else:
                    live_class = self.read_register(thread_id, "ebx") & 0xFF
                    commit = {
                        "register_class": live_class,
                        "reg": live_reg,
                        "reg_hi_ignored": True,
                    }
            elif hook_id == "precolored_commit":
                list_node = self.read_register(thread_id, "ebx")
                object_data = self._read(list_node + 4, 4) if list_node else b""
                operand_data = self._read(list_node + 0xC, 2) if list_node else b""
                object_pointer = (
                    int.from_bytes(object_data, "little", signed=False)
                    if len(object_data) == 4
                    else 0
                )
                varinfo_pointer = self.read_register(thread_id, "esi")
                live_class = operand_data[0] if len(operand_data) == 2 else -1
                live_reg = (
                    int.from_bytes(operand_data[1:2], "little", signed=True)
                    if len(operand_data) == 2
                    else -1
                )
                commit = {
                    "register_class": live_class,
                    "reg": live_reg,
                    "reg_hi": 0,
                }
            else:
                object_pointer = 0
                varinfo_pointer = 0
        else:
            object_pointer = self.read_register(thread_id, "ebx")
            varinfo_pointer = self.read_register(thread_id, "ebp")
        if object_pointer == 0:
            return {
                "hook_id": str(hook_id),
                "status": "UNKNOWN",
                "reason": "null object identity",
                "object": 0,
                "varinfo_pointer": varinfo_pointer,
                "commit": commit,
                "noregister": 0,
                "flags": 0,
                "rclass": 0,
                "reg": 0,
                "reg_hi": 0,
            }
        expected_varinfo = self._object_varinfo.get(object_pointer)
        if varinfo_pointer == 0:
            raise Rejected("Object VarInfo pointer is null")
        # The post hook can precede the first Object-list snapshot.  Preserve
        # the raw pair for the session join in that case; once this backend has
        # a list binding, continue enforcing it immediately here as well.
        if expected_varinfo is not None and expected_varinfo != varinfo_pointer:
            raise Rejected("Object VarInfo pointer mismatch")
        reverse_object = self._varinfo_object.get(varinfo_pointer)
        if reverse_object is not None and reverse_object != object_pointer:
            raise Rejected("Object VarInfo pointer reverse binding mismatch")
        data = self._read(varinfo_pointer, VARINFO_PHYSICAL_READ_SIZE)
        if len(data) < VARINFO_PHYSICAL_READ_SIZE:
            raise Rejected("physical assignment VarInfo record is truncated")
        result: dict[str, Any] = {
            "hook_id": str(hook_id),
            "object": object_pointer,
            "varinfo_pointer": varinfo_pointer,
            "noregister": data[VARINFO_NOREGISTER_FIELD],
            "flags": data[VARINFO_FLAGS_FIELD],
            "rclass": data[VARINFO_CLASS_FIELD],
            "reg": int.from_bytes(data[VARINFO_REG_FIELD:VARINFO_REG_FIELD + 0x2], "little", signed=True),
            "reg_hi": int.from_bytes(data[VARINFO_REG_HI_FIELD:VARINFO_REG_HI_FIELD + 0x2], "little", signed=True),
        }
        if commit:
            result["commit"] = commit
        if self.compiler_sha256 != GC27_COMPILER_SHA256:
            return result
        consistent = (
            result["noregister"] == 0
            and result["rclass"] == commit.get("register_class")
            and result["reg"] == commit.get("reg")
        )
        if hook_id == "physical_pair_commit":
            consistent = (
                consistent
                and result["flags"] & 6 == 6
                and result["reg_hi"] == commit.get("reg_hi")
            )
            result.update(
                status="UNKNOWN",
                reason=(
                    "paired physical register assignment unsupported"
                    if consistent
                    else "incomplete physical register evidence"
                ),
            )
            return result
        if hook_id == "physical_single_commit":
            consistent = consistent and result["flags"] & 2 == 2
            # +0x28 is not written by the single-register epilogue.  Never
            # reinterpret a stale high-half value as a second assignment.
            result["reg_hi"] = 0
        elif hook_id == "precolored_commit":
            consistent = (
                consistent
                and result["flags"] & 2 == 2
                and result["reg_hi"] == 0
            )
        if not consistent:
            result.update(status="UNKNOWN", reason="incomplete physical register evidence")
        return result

    # Name the hook-specific operation as well for transports that dispatch
    # by breakpoint ID rather than by the generic physical-regalloc role.
    def capture_regalloc_post(self, hook_id: str, thread_id: int) -> Mapping[str, Any]:
        return self.capture_physical_regalloc(hook_id, thread_id)

    def read_image(self, address: int, size: int) -> bytes:
        return self._read(self._runtime(address), size)

    def expected_hook_prefix(self, row: Mapping[str, Any]) -> bytes:
        """Return the authenticated bytes expected at the live image base.

        A rebased memexec image carries HIGHLOW-adjusted absolute operands,
        while the static hook table stores the preferred-base bytes.  Discovery
        already authenticated the relocation table before binding ``base``;
        reuse that same transformation for the preflight read instead of
        comparing a valid rebased image with stale preferred-base bytes.
        """

        address = int(row["address"])
        prefix = str(row["prefix"])
        return self._mapped_hook_prefix({"address": address, "prefix": prefix}, self.base)

    def install_breakpoint(self, address: int) -> None:
        runtime = self._runtime(address)
        if runtime in self.breakpoints:
            raise Rejected(f"duplicate native breakpoint 0x{address:08x}")
        original = self._read(runtime, 1)
        if original == b"\xcc":
            raise Rejected(f"native breakpoint already installed at 0x{address:08x}")
        written = ctypes.c_ubyte(0xCC)
        count = ctypes.c_size_t()
        if not self.native.kernel32.WriteProcessMemory(self.process, ctypes.c_void_p(runtime), ctypes.byref(written), 1, ctypes.byref(count)) or int(count.value) != 1:
            raise Rejected(f"WriteProcessMemory failed at 0x{address:08x}")
        self.breakpoints[runtime] = original[0]

    def remove_breakpoint(self, address: int) -> None:
        runtime = self._runtime(address)
        original = self.breakpoints.pop(runtime, None)
        if original is None:
            return
        self._write(runtime, bytes([original]))

    def _write(self, address: int, data: bytes) -> None:
        buffer = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
        count = ctypes.c_size_t()
        if not self.native.kernel32.WriteProcessMemory(self.process, ctypes.c_void_p(address), buffer, len(data), ctypes.byref(count)) or int(count.value) != len(data):
            raise Rejected(f"WriteProcessMemory failed at 0x{address:08x}")

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None:
        self.remove_breakpoint(address)
        handle = self.threads.get(thread_id)
        if handle is None:
            raise Rejected("single-step thread handle missing")
        context_type = getattr(self.native, "WOW64_CONTEXT", None)
        if context_type is None:
            raise Rejected("WOW64 context layout is unavailable")
        context = context_type()
        context.ContextFlags = getattr(self.native, "WOW64_CONTEXT_FULL", 0x00010007)
        if not self.native.kernel32.Wow64GetThreadContext(handle, ctypes.byref(context)):
            raise Rejected("Wow64GetThreadContext failed")
        context.Eip = self._runtime(address)
        # The dispatcher owns the logical rearm.  The CPU trap itself is
        # required for every write, including the ``rearm=False`` pre-step.
        context.EFlags |= getattr(self.native, "WOW64_CONTEXT_TF", 0x100)
        if rearm:
            self.pending_steps[thread_id] = address
        if not self.native.kernel32.Wow64SetThreadContext(handle, ctypes.byref(context)):
            raise Rejected("Wow64SetThreadContext failed")

    def _handle_breakpoint_exception(
        self,
        session: CombinedCaptureSession,
        address: int,
        thread_id: int,
        process_id: int,
    ) -> None:
        normalized = int(address) - self.base + KNOWN_IMAGE_BASE
        if normalized in session.dispatcher.by_address:
            self.loader_breakpoint_pending = False
            self.loader_breakpoints_remaining = 0
            session.on_breakpoint(normalized, thread_id, process_id)
        elif self._consume_initial_system_breakpoint(address):
            # Windows emits bounded native/WOW64 initialization breakpoints
            # outside the authenticated compiler image.  They are transport
            # noise and never enter the event ledger.
            return
        else:
            raise Rejected(f"unexpected non-loader breakpoint 0x{normalized:08x}")

    def run(self, session: CombinedCaptureSession) -> None:
        # The native structure definitions are supplied by the existing
        # standard-library-only adapter.  Importing it does not launch a
        # second process or install any breakpoints.
        if not self.compiler_selected or self.compiler_process_id is None:
            raise Rejected("native compiler child was not selected before hook loop")
        event_type = getattr(self.native, "DEBUG_EVENT", None)
        if event_type is None:
            raise Rejected("native DEBUG_EVENT layout is unavailable")
        event = event_type()
        self._terminal_deadline = time.monotonic() + NATIVE_CAPTURE_TERMINAL_TIMEOUT_SECONDS
        if self._pending_create_event is not None:
            self._continue_debug_event(*self._pending_create_event)
            self._pending_create_event = None
        if self._pending_debug_event is not None:
            self._continue_debug_event(*self._pending_debug_event)
            self._pending_debug_event = None
        while not self.exited:
            if self._terminal_deadline is not None and time.monotonic() >= self._terminal_deadline:
                raise Rejected("native compiler capture exceeded its terminal deadline")
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == getattr(self.native, "ERROR_SEM_TIMEOUT", 121):
                    raise Rejected("native debug transport gap or timeout")
                raise Rejected(f"WaitForDebugEvent failed: {error}")
            code = int(event.dwDebugEventCode)
            pid = int(event.dwProcessId)
            tid = int(event.dwThreadId)
            compiler_exit_event = False
            self._active_debug_event = (pid, tid)
            if code == getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3) and pid != self.compiler_process_id:
                # DEBUG_PROCESS reports every descendant.  Descendants never
                # contribute hook evidence, but their handles/events remain
                # part of the terminal process-tree proof.
                self._record_process_create(pid, tid, event.u.CreateProcessInfo)
                self._continue_debug_event(pid, tid)
                continue
            if pid in self._observed_process_ids and pid not in {
                self.transport_process_id, self.compiler_process_id
            }:
                if code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2):
                    handle = _native_value(event.u.CreateThread.hThread)
                    if handle:
                        self._descendant_threads.setdefault(pid, {})[tid] = handle
                        self._owned_handles.add(handle)
                elif code == getattr(self.native, "EXIT_THREAD_DEBUG_EVENT", 4):
                    handle = self._descendant_threads.setdefault(pid, {}).pop(tid, None)
                    if handle:
                        self.native.kernel32.CloseHandle(handle)
                        self._owned_handles.discard(handle)
                elif code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                    self._record_process_exit(pid)
                self._continue_debug_event(pid, tid)
                continue
            # DEBUG_PROCESS also reports the wrapper's loader/transport events.
            # They are continued without inspection; every other process must
            # be the one authenticated compiler child.
            if pid == self.transport_process_id and pid != self.compiler_process_id:
                if code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                    self._record_process_exit(pid)
                    self._continue_debug_event(pid, tid)
                    continue
                if code == getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3):
                    raise Rejected("wrapper transport emitted a second process-create event")
                if code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2):
                    wrapper_thread = _native_value(getattr(event.u.CreateThread, "hThread", 0))
                    if wrapper_thread:
                        self.transport_threads[tid] = wrapper_thread
                        self._owned_handles.add(wrapper_thread)
                elif code == getattr(self.native, "EXIT_THREAD_DEBUG_EVENT", 4):
                    wrapper_thread = self.transport_threads.pop(tid, None)
                    if wrapper_thread:
                        self.native.kernel32.CloseHandle(wrapper_thread)
                        self._owned_handles.discard(wrapper_thread)
                self._continue_debug_event(pid, tid)
                continue
            if pid != self.compiler_process_id:
                raise Rejected("debug event came from an unauthenticated process")
            if code == getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3):
                raise Rejected("compiler CREATE_PROCESS event was not consumed during preparation")
            elif code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2):
                session._check_process(pid)
                handle = _native_value(event.u.CreateThread.hThread)
                if handle:
                    self.threads[tid] = handle
                    self._owned_handles.add(handle)
            elif code == getattr(self.native, "EXIT_THREAD_DEBUG_EVENT", 4):
                session._check_process(pid)
                handle = self.threads.pop(tid, None)
                if handle:
                    self.native.kernel32.CloseHandle(handle)
                    self._owned_handles.discard(handle)
            elif code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                # The process address space is already gone when the exit
                # event is delivered; record that boundary before asking the
                # session to validate chronology so cleanup cannot attempt
                # impossible breakpoint writes if validation rejects.
                self.exited = True
                self.compiler_exit_code = int(event.u.ExitProcess.dwExitCode)
                self._record_process_exit(pid)
                compiler_exit_event = True
                session.on_process_exit(self.compiler_exit_code, pid)
            elif code == getattr(self.native, "EXCEPTION_DEBUG_EVENT", 1):
                session._check_process(pid)
                record = event.u.Exception.ExceptionRecord
                exception_code = int(record.ExceptionCode)
                address = int(record.ExceptionAddress or 0)
                if exception_code in (getattr(self.native, "EXCEPTION_SINGLE_STEP", 0x80000004), getattr(self.native, "EXCEPTION_WX86_SINGLE_STEP", 0x4000001E)):
                    session.on_single_step(tid, pid)
                elif exception_code in (getattr(self.native, "EXCEPTION_BREAKPOINT", 0x80000003), getattr(self.native, "EXCEPTION_WX86_BREAKPOINT", 0x4000001F)):
                    self._handle_breakpoint_exception(session, address, tid, pid)
                else:
                    raise Rejected(f"unsupported native exception 0x{exception_code:08x}")
            else:
                session._check_process(pid)
            self._continue_debug_event(pid, tid)
            if compiler_exit_event:
                # EXIT_PROCESS is not a durable process boundary until its
                # debugger event has been continued and the process handle is
                # signaled.  Prove that boundary before a receipt may consume
                # the compiler exit status.
                self._seal_process_quiescence()

    def _seal_process_quiescence(self) -> None:
        """Continue termination events until every observed process is signaled."""

        if self._active_debug_event is not None:
            self._continue_debug_event(*self._active_debug_event)
        self._pending_create_event = None
        self._pending_debug_event = None

        exit_code = ctypes.c_uint32()
        deadline = time.monotonic() + 5.0
        event_type = getattr(self.native, "DEBUG_EVENT", None)
        event = None if event_type is None else event_type()
        compiler_exit_code: int | None = None
        while True:
            unsignaled: list[int] = []
            for pid, raw_handle in sorted(self._process_handles.items()):
                wait_result = int(
                    self.native.kernel32.WaitForSingleObject(ctypes.c_void_p(int(raw_handle)), 0)
                )
                if wait_result != 0:
                    unsignaled.append(pid)
                else:
                    self._exited_process_ids.add(pid)
            if not unsignaled:
                process_handle = ctypes.c_void_p(int(self.process))
                if not self.native.kernel32.GetExitCodeProcess(process_handle, ctypes.byref(exit_code)):
                    raise Rejected("compiler exit code was unavailable after diagnostic quiescence")
                observed = int(exit_code.value)
                expected = compiler_exit_code
                if expected is None:
                    expected = self.compiler_exit_code
                if expected is not None and observed != expected:
                    raise Rejected("compiler exit event and process exit code diverged")
                self.compiler_exit_code = observed
                self.process_quiesced = True
                self.exited = True
                return

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise Rejected("compiler process did not quiesce before diagnostic sealing")
            if event is None:
                raise Rejected("native DEBUG_EVENT layout is unavailable during process quiescence")
            wait_ms = max(1, min(250, int(remaining * 1000)))
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), wait_ms):
                error = ctypes.get_last_error()
                if error == getattr(self.native, "ERROR_SEM_TIMEOUT", 121):
                    continue
                raise Rejected(f"WaitForDebugEvent failed during process quiescence: {error}")

            code = int(event.dwDebugEventCode)
            pid = int(event.dwProcessId)
            tid = int(event.dwThreadId)
            self._active_debug_event = (pid, tid)
            authenticated_pids = {self.transport_process_id}
            if self.compiler_process_id is not None:
                authenticated_pids.add(self.compiler_process_id)
            if pid not in authenticated_pids:
                if code != getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3) and pid not in self._observed_process_ids:
                    raise Rejected("process quiescence reported an unauthenticated debug process")
            if code == getattr(self.native, "CREATE_PROCESS_DEBUG_EVENT", 3):
                self._record_process_create(pid, tid, event.u.CreateProcessInfo)
            elif code == getattr(self.native, "CREATE_THREAD_DEBUG_EVENT", 2):
                handle = _native_value(event.u.CreateThread.hThread)
                if handle:
                    self._descendant_threads.setdefault(pid, {})[tid] = handle
                    self._owned_handles.add(handle)
            elif code == getattr(self.native, "EXIT_THREAD_DEBUG_EVENT", 4):
                handle = self._descendant_threads.setdefault(pid, {}).pop(tid, None)
                if handle:
                    self.native.kernel32.CloseHandle(handle)
                    self._owned_handles.discard(handle)
            elif code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
                self._record_process_exit(pid)
                compiler_pid = self.compiler_process_id or self.transport_process_id
                if pid == compiler_pid:
                    compiler_exit_code = int(event.u.ExitProcess.dwExitCode)
            self._continue_debug_event(pid, tid)

    def close(self) -> None:
        if self._close_started:
            return
        self._close_started = True
        errors: list[str] = []
        # Once the compiler process has delivered EXIT_PROCESS, its address
        # space is gone and WriteProcessMemory cannot restore INT3 bytes.  The
        # process-exit boundary is already the durable cleanup for those
        # breakpoints; attempting restoration only masks the captured result
        # as a cleanup failure.
        if not self.exited:
            for runtime, original in list(self.breakpoints.items()):
                try:
                    self._write(runtime, bytes([original]))
                except Exception as exc:
                    errors.append(f"breakpoint 0x{runtime:08x}: {type(exc).__name__}: {exc}")
                else:
                    self.breakpoints.pop(runtime, None)
        # Release a delivered debugger event before deciding whether any
        # process needs forced termination. An EXIT_PROCESS event commonly
        # becomes signaled immediately after this continuation.
        if self._active_debug_event is not None:
            try:
                self._continue_debug_event(*self._active_debug_event)
            except Exception as exc:
                errors.append(f"active debug event: {type(exc).__name__}: {exc}")
        exit_code = ctypes.c_uint32()
        for pid, raw_handle in sorted(self._process_handles.items(), reverse=True):
            process_handle = ctypes.c_void_p(int(raw_handle))
            wait_result = int(self.native.kernel32.WaitForSingleObject(process_handle, 0))
            if wait_result == 0:
                self._exited_process_ids.add(pid)
                if pid == (self.compiler_process_id or self.transport_process_id):
                    if not self.native.kernel32.GetExitCodeProcess(process_handle, ctypes.byref(exit_code)):
                        errors.append("compiler exit code was unavailable after its process handle signaled")
                    else:
                        self.compiler_exit_code = int(exit_code.value)
                        self.exited = True
                continue
            if not self.native.kernel32.TerminateProcess(process_handle, 1):
                errors.append(f"debug process {pid} could not be terminated before diagnostic sealing")
            elif pid == (self.compiler_process_id or self.transport_process_id):
                self.compiler_terminated_by_capture = True

        # An EXIT_PROCESS status is not sufficient while a debugger event is
        # still leased.  Always continue/drain that event and prove the process
        # handle signaled before making diagnostics immutable.
        if self._process_handles:
            try:
                self._seal_process_quiescence()
            except Exception as exc:
                errors.append(f"process quiescence: {type(exc).__name__}: {exc}")
        # DEBUG_PROCESS gives us handles for both the wrapper and compiler
        # CREATE_PROCESS events.  Close each raw handle once at the durable
        # cleanup boundary; never let a transport handle remain attached to a
        # later capture.
        unclosed_handles: set[int] = set()
        for handle in sorted(self._owned_handles):
            try:
                closed = self.native.kernel32.CloseHandle(handle)
            except Exception as exc:
                errors.append(f"native handle {handle}: {type(exc).__name__}: {exc}")
            else:
                if not closed:
                    errors.append(f"native handle {handle}: CloseHandle returned FALSE")
                    unclosed_handles.add(handle)
        self._owned_handles = unclosed_handles
        self.threads.clear()
        self.transport_threads.clear()
        self._descendant_threads.clear()
        if errors:
            raise Rejected("native cleanup restoration failed: " + "; ".join(errors))


def launch_native_capture(
    request_path: Path | str,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    partial_evidence_dir: Path | str | None = None,
) -> dict[str, Any]:
    """Launch exactly one authenticated compiler under one WOW64 loop."""

    if os.name != "nt":
        raise Rejected("native WOW64 capture requires Windows; compiler was not launched")
    if external_trust_root is not None and trust_root is not None:
        raise Rejected("conflicting external trust root arguments")
    root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
    auth = authenticate_request(request_path, require_empty=True, external_trust_root=root)
    spec = importlib.util.spec_from_file_location("mwcc_same_session_native_layout", Path(__file__).with_name("mwcc_win32_varinfo.py"))
    if spec is None or spec.loader is None:
        raise Rejected("cannot load native WOW64 ctypes layout")
    native = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(native)
    # mwcc_win32_varinfo configures the core debug APIs, while these two
    # same-process handoff calls are intentionally local to this tool.  Bind
    # their pointer-sized signatures before the first native call so a 64-bit
    # Python host cannot truncate a WOW64 process handle or region address.
    virtual_query = getattr(native.kernel32, "VirtualQueryEx", None)
    debug_break = getattr(native.kernel32, "DebugBreakProcess", None)
    wait_for_process = getattr(native.kernel32, "WaitForSingleObject", None)
    if not callable(virtual_query) or not callable(debug_break) or not callable(wait_for_process):
        raise Rejected("native kernel32 lacks same-process compiler handoff APIs")
    virtual_query.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(_MEMORY_BASIC_INFORMATION), ctypes.c_size_t]
    virtual_query.restype = ctypes.c_size_t
    debug_break.argtypes = [ctypes.c_void_p]
    debug_break.restype = wintypes.BOOL
    wait_for_process.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    wait_for_process.restype = ctypes.c_uint32
    request = auth["request"]
    transport_mode, executed_argv = _native_transport_plan(request)
    environment_binding, environment_block = _sealed_compiler_environment()
    include_search_paths = [
        _directory_tree_descriptor(path)
        for path in _compile_include_paths(executed_argv, request["cwd"])
    ]
    diagnostic_paths = _compiler_transport_paths(auth)
    diagnostic_paths["directory"].mkdir(parents=False, exist_ok=False)
    command = subprocess.list2cmdline(executed_argv)
    startup = native.STARTUPINFOW(
        cb=ctypes.sizeof(native.STARTUPINFOW),
        dwFlags=native.STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES,
        wShowWindow=native.SW_HIDE,
    )
    process_info = native.PROCESS_INFORMATION()
    buffer = ctypes.create_unicode_buffer(command)
    streams: list[Any] = []
    inherited_handles: list[int] = []
    backend: NativeWow64Backend | None = None
    process_created = False
    result: dict[str, Any] | None = None
    failure: Exception | None = None
    try:
        import msvcrt

        stdout_fd = os.open(diagnostic_paths["stdout"], os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        stdout_stream = os.fdopen(stdout_fd, "wb", buffering=0)
        streams.append(stdout_stream)
        stderr_fd = os.open(diagnostic_paths["stderr"], os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        stderr_stream = os.fdopen(stderr_fd, "wb", buffering=0)
        streams.append(stderr_stream)
        stdin_stream = open(os.devnull, "rb", buffering=0)
        streams.append(stdin_stream)
        stdin_handle = int(msvcrt.get_osfhandle(stdin_stream.fileno()))
        stdout_handle = int(msvcrt.get_osfhandle(stdout_stream.fileno()))
        stderr_handle = int(msvcrt.get_osfhandle(stderr_stream.fileno()))
        inherited_handles.extend((stdin_handle, stdout_handle, stderr_handle))
        for handle in inherited_handles:
            os.set_handle_inheritable(handle, True)
        startup.hStdInput = stdin_handle
        startup.hStdOutput = stdout_handle
        startup.hStdError = stderr_handle
        # The immutable request argv remains wrapper-first. For the one closed
        # sjiswrap/GC2.7 pair above, the process command is its authenticated
        # ASCII-equivalent argv[1:] derivation; every other pair remains wrapper
        # based. DEBUG_PROCESS covers both the direct compiler and launchers that
        # create a real compiler child. The backend authenticates the selected
        # image before any hook read. Only the three diagnostic handles above
        # are intentionally inheritable.
        created = native.kernel32.CreateProcessW(
            None,
            buffer,
            None,
            None,
            True,
            DEBUG_PROCESS
            | native.CREATE_NO_WINDOW
            | getattr(native, "CREATE_UNICODE_ENVIRONMENT", 0x00000400),
            environment_block,
            request["cwd"],
            ctypes.byref(startup),
            ctypes.byref(process_info),
        )
        if not created:
            raise Rejected(f"CreateProcessW failed: {ctypes.WinError(ctypes.get_last_error())}")
        process_created = True
        for handle in inherited_handles:
            os.set_handle_inheritable(handle, False)
        pid = _native_value(process_info.dwProcessId)
        if pid <= 0:
            native.kernel32.CloseHandle(process_info.hThread)
            native.kernel32.CloseHandle(process_info.hProcess)
            raise Rejected("CreateProcessW returned no native PID")
        backend = NativeWow64Backend(
            native,
            _native_value(process_info.hProcess),
            _native_value(process_info.hThread),
            pid,
            compiler_path=str(request["compiler"]["path"]),
            compiler_sha256=str(request["compiler"]["sha256"]),
            wrapper_path=str(request["wrapper"]["path"]),
            wrapper_sha256=str(request["wrapper"]["sha256"]),
            transport_mode=transport_mode,
            executed_argv=executed_argv,
        )
        result = capture_with_backend(
            request_path,
            backend,
            external_trust_root=root,
            preauthenticated_auth=auth,
            partial_evidence_dir=partial_evidence_dir,
        )
    except Exception as exc:
        failure = exc
    finally:
        for handle in inherited_handles:
            try:
                os.set_handle_inheritable(handle, False)
            except OSError:
                pass
        failure = _close_capture_streams(streams, failure)

    failure_text = None if failure is None else str(failure)
    primary_failure = (
        getattr(failure, "primary_failure", failure_text) if failure is not None else None
    )
    secondary_failures = (
        list(getattr(failure, "secondary_failures", ())) if failure is not None else []
    )
    process_summary = None if backend is None else backend.terminal_process_summary()
    try:
        receipt = _compiler_execution_receipt(
            auth,
            transport_mode=transport_mode,
            executed_argv=executed_argv,
            include_search_paths=include_search_paths,
            stdout_path=diagnostic_paths["stdout"],
            stderr_path=diagnostic_paths["stderr"],
            process_created=process_created,
            process_quiesced=False if backend is None else backend.process_quiesced,
            terminated_by_capture=False if backend is None else backend.compiler_terminated_by_capture,
            exit_code=None if backend is None else backend.compiler_exit_code,
            failure=primary_failure,
            environment_binding=environment_binding,
            capture_result=result,
            process_summary=process_summary,
            secondary_failures=secondary_failures,
        )
        _publish_compiler_execution_receipt(diagnostic_paths["receipt"], receipt)
    except Exception as exc:
        publication = f"compiler execution receipt publication failed: {type(exc).__name__}: {exc}"
        raise _combine_terminal_failure(failure, publication) from exc
    if failure is not None:
        if isinstance(failure, Rejected):
            raise failure
        raise Rejected(f"capture failure: {type(failure).__name__}: {failure}") from failure
    if result is None:
        raise Rejected("native capture returned no result")
    return result


def unknown_result(reason: str) -> dict[str, Any]:
    return {
        "schema": SCHEMA,
        "status": "UNKNOWN",
        "diagnostic_only": DIAGNOSTIC_ONLY,
        "board_admission": BOARD_ADMISSION,
        "exactness_claim": EXACTNESS_CLAIM,
        "reason": str(reason),
    }


def self_test() -> dict[str, Any]:
    if len(LEGACY_HOOKS) != 8 or len(GC26_HOOKS) != 13 or len(GC27_HOOKS) != 13 or len(HOOK_BY_ADDRESS) != 22:
        raise Rejected("hook union is not closed")
    if tuple(HOOKS) not in (LEGACY_HOOKS, GC26_HOOKS, GC27_HOOKS):
        raise Rejected("private backend hook patch does not match a closed profile")
    if any(row["address"] == 0x004D03E8 for row in GC27_HOOKS):
        raise Rejected("GC/2.7 profile contains stale GC/2.6 regalloc hook")
    if LOCALS_LIST_HEAD == ARGUMENTS_LIST_HEAD:
        raise Rejected("locals and arguments list heads collapsed")
    duplicate = '{"schema":"one","schema":"two"}'
    try:
        strict_json_loads(duplicate, "self-test")
    except Rejected:
        pass
    else:
        raise Rejected("duplicate-key parser accepted a duplicate")
    return {"schema": f"{SCHEMA}/self-test", "status": "OK", "tests": 3, "diagnostic_only": True, "board_admission": False}


def parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    prepare = sub.add_parser("prepare", help="create an immutable prelaunch request")
    prepare.add_argument("--manifest", type=Path, required=True)
    prepare.add_argument("--output-dir", type=Path, required=True)
    prepare.add_argument("--trust-root", type=Path)
    preflight = sub.add_parser("preflight", help="authenticate request and hook union without launching")
    preflight.add_argument("request", type=Path)
    preflight.add_argument("--trust-root", type=Path, required=True)
    capture = sub.add_parser("capture", help="launch one native WOW64 compiler process")
    capture.add_argument("request", type=Path)
    capture.add_argument("--trust-root", type=Path, required=True)
    capture.add_argument(
        "--partial-evidence-dir",
        type=Path,
        help=(
            "atomically preserve pointer-free raw evidence when strict validation "
            "ends at one authenticated final ownership-join UNKNOWN"
        ),
    )
    validate = sub.add_parser("validate", help="validate a completed envelope")
    validate.add_argument("envelope", type=Path)
    validate.add_argument("--trust-root", type=Path, required=True)
    causal = sub.add_parser("causal-map", help="join authenticated source spans to physical/stack chronology")
    causal.add_argument("--envelope", type=Path, required=True)
    causal.add_argument("--trust-root", type=Path, required=True)
    causal.add_argument("--source-spans", type=Path, required=True)
    causal.add_argument("--frontend-chronology", type=Path)
    causal.add_argument(
        "--post-capture-analysis",
        action="store_true",
        help=(
            "accept capture-time debugger/transport descriptors from a sealed "
            "post-capture root while still authenticating every producer output"
        ),
    )
    causal.add_argument("--output", type=Path)
    seal_spans = sub.add_parser("seal-source-spans", help="seal a reviewed capture-local source-span manifest")
    seal_spans.add_argument("--input", type=Path, required=True)
    seal_spans.add_argument("--output", type=Path, required=True)
    normalize_spans = sub.add_parser(
        "normalize-source-spans",
        help="bind a reviewed placeholder template to one authenticated capture",
    )
    normalize_spans.add_argument("--envelope", type=Path, required=True)
    normalize_spans.add_argument("--trust-root", type=Path, required=True)
    normalize_spans.add_argument("--template", type=Path, required=True)
    normalize_spans.add_argument("--binding-plan", type=Path, required=True)
    normalize_spans.add_argument("--output", type=Path, required=True)
    sub.add_parser("self-test")
    return parser


def _load_trust_root(path: Path | None) -> ExternalTrustRoot | None:
    if path is None:
        return None
    try:
        value = strict_json_loads(path.read_text(encoding="utf-8"), "external trust root")
    except OSError as exc:
        raise Rejected(f"cannot read external trust root: {exc}") from exc
    return _coerce_external_trust_root(value)


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "prepare":
            result = {"schema": f"{REQUEST_SCHEMA}/prepare", "status": "READY", "request": str(prepare_request(args.manifest, args.output_dir, external_trust_root=_load_trust_root(args.trust_root))), "diagnostic_only": True, "board_admission": False}
        elif args.command == "preflight":
            auth = authenticate_request(args.request, external_trust_root=_load_trust_root(args.trust_root))
            result = {"schema": f"{REQUEST_SCHEMA}/preflight", "status": "READY", "session_id": auth["request"]["session_id"], "function": auth["request"]["function"], "request_sha256": auth["request_sha256"], "hooks": [dict(row) for row in auth["hooks"]], "diagnostic_only": True, "board_admission": False}
        elif args.command == "capture":
            result = launch_native_capture(
                args.request,
                external_trust_root=_load_trust_root(args.trust_root),
                partial_evidence_dir=args.partial_evidence_dir,
            )
        elif args.command == "validate":
            result = validate_envelope(args.envelope, external_trust_root=_load_trust_root(args.trust_root))
        elif args.command == "causal-map":
            root = _load_trust_root(args.trust_root)
            if root is None:
                raise Rejected("causal-map requires an external trust root")
            result = build_source_aware_causal_map(
                args.envelope,
                args.source_spans,
                trust_root=root,
                frontend_chronology=args.frontend_chronology,
                post_capture_analysis=args.post_capture_analysis,
            )
            if args.output is not None:
                output = args.output.resolve()
                if output.exists() or output.is_symlink():
                    raise Rejected("causal-map output already exists")
                write_new(output, (json.dumps(result, indent=2, sort_keys=True) + "\n").encode("utf-8"))
        elif args.command == "seal-source-spans":
            result = seal_source_span_file(args.input, args.output)
        elif args.command == "normalize-source-spans":
            root = _load_trust_root(args.trust_root)
            if root is None:
                raise Rejected("normalize-source-spans requires an external trust root")
            result = normalize_source_span_template(
                args.envelope,
                args.template,
                args.binding_plan,
                args.output,
                trust_root=root,
            )
        else:
            result = self_test()
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0 if result.get("status") in {"OK", "READY", "CAPTURED", "CAPTURED_UNKNOWN_OWNERSHIP"} else 2
    except (OSError, Rejected, ValueError, json.JSONDecodeError) as exc:
        print(json.dumps(unknown_result(str(exc)), indent=2, sort_keys=True))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
