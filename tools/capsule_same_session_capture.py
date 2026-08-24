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
import time
import uuid
from collections.abc import Callable, Iterable, Mapping, MutableMapping, Sequence
from typing import Any, Protocol

try:  # Package import under unittest/agent tooling.
    from . import capsule_stack_home_native as _stack_home
    from . import donor_cfg_align as _donor_cfg
    from . import mwcc_fe_chronology_native as _frontend_chronology
    from . import pcode_varinfo_correlator as _correlator
except ImportError:  # Direct ``python tools/...`` execution.
    try:  # importlib file loading under the repository test harness.
        from tools import capsule_stack_home_native as _stack_home
        from tools import donor_cfg_align as _donor_cfg
        from tools import mwcc_fe_chronology_native as _frontend_chronology
        from tools import pcode_varinfo_correlator as _correlator
    except ImportError:
        import capsule_stack_home_native as _stack_home
        import donor_cfg_align as _donor_cfg
        import mwcc_fe_chronology_native as _frontend_chronology
        import pcode_varinfo_correlator as _correlator


SCHEMA = "mwcc_capsule_same_session_capture/v1"
REQUEST_SCHEMA = "mwcc_capsule_same_session_capture_request/v1"
EVENT_SCHEMA = "mwcc_capsule_same_session_capture_event/v1"
SOURCE_SPAN_SCHEMA = "mwcc_source_span_bindings/v1"
CAUSAL_MAP_SCHEMA = "mwcc_source_aware_causal_map/v1"
# The pinned hook union changed: old requests/envelopes must not be
# interpreted as captures from this repaired transport.
TOOL_VERSION = "capsule-same-session-capture-2"
DIAGNOSTIC_ONLY = True
BOARD_ADMISSION = False
EXACTNESS_CLAIM = False
AUTHORITY_ADVANCED = False

KNOWN_IMAGE_BASE = _stack_home.KNOWN_IMAGE_BASE
# ``DEBUG_PROCESS`` follows a launcher through its descendant processes.  The
# wrapper-first MWCC command therefore needs this flag; DEBUG_ONLY_THIS_PROCESS
# would attach only to sjiswrap.exe and all compiler hooks would be read from
# the wrong address space.
DEBUG_PROCESS = 0x00000001
# sjiswrap does not create a child process.  It manually maps mwcceppc.exe
# into its own address space through memexec_exe_with_hooks.  The mapped image
# is therefore a MEM_PRIVATE allocation, not a normal loader module.
MEM_COMMIT = 0x00001000
MEM_PRIVATE = 0x00020000
MEMEXEC_STARTUP_TIMEOUT_SECONDS = 30.0
# A first probe is requested when the authenticated wrapper is released.  A
# manually mapped compiler may then create its worker thread before that probe
# is delivered; permit one follow-up observation after that event, but never
# turn a missing map into an unbounded stream of debug breaks.
MEMEXEC_MAX_PROBES = 2
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
HOOK_BY_ID = {str(row["id"]): row for row in HOOKS}
HOOK_BY_ADDRESS = {int(row["address"]): row for row in HOOKS}
WRITE_HOOK_IDS = tuple(row["id"] for row in HOOKS if row["role"] == "object_stack_write")
PCODE_HOOK_IDS = tuple(
    row["id"]
    for row in HOOKS
    if row["lane"] == "pcode" and row["role"] not in {"regalloc", "regalloc_post"}
)

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
    },
    "regalloc_assignment": {"object_token", "status", "reason", "vreg_id", "bank"},
    "physical_reg_assignment": {"object_token", "status", "reason", "physical_reg", "bank"},
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


def _path_descriptor(value: Any, label: str, *, must_exist: bool) -> dict[str, Any]:
    """Read a descriptor while allowing direct fake-auth fixtures."""

    if isinstance(value, Mapping) and set(value) == {"path", "size", "sha256"}:
        raw_path = _text(value["path"], f"{label}.path")
        path = _canonical_path(raw_path, f"{label}.path", must_exist=must_exist)
        size = _integer(value["size"], f"{label}.size", nonnegative=True)
        digest = _text(value["sha256"], f"{label}.sha256").lower()
        if not SHA256_RE.fullmatch(digest):
            raise Rejected(f"{label}.sha256 is malformed")
        if path.exists():
            if path.stat().st_size != size or sha256(path) != digest:
                raise Rejected(f"{label} identity mismatch")
        return {"path": str(path), "size": size, "sha256": digest}
    return _descriptor(value, label, verify=True)


def _digest(value: Any, label: str) -> str:
    digest = _text(value, label).lower()
    if digest != str(value):
        raise Rejected(f"{label} must use lowercase hexadecimal")
    if not SHA256_RE.fullmatch(digest):
        raise Rejected(f"{label} must be a SHA-256 digest")
    return digest


def _trust_root_descriptor(root: ExternalTrustRoot, name: str, *, required: bool = True) -> dict[str, Any] | None:
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
    path = _canonical_path(values["path"], f"external trust root.{name}.path", must_exist=True)
    size = _integer(values["size"], f"external trust root.{name}.size", nonnegative=True)
    digest = _digest(values["sha256"], f"external trust root.{name}.sha256")
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
    return values


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
    )
    if "path" not in wrapper:
        raise Rejected("native launch wrapper identity has no path")
    wrapper_path = _canonical_path(wrapper["path"], "request.wrapper.path", must_exist=False)
    if Path(values[0]).name.casefold() != wrapper_path.name.casefold():
        raise Rejected("native launch argv does not invoke the authenticated wrapper")
    return values


def _validate_external_root_against_request(
    root: ExternalTrustRoot,
    request: Mapping[str, Any],
    *,
    request_path: Path | None,
    allow_outputs: bool = False,
) -> None:
    """Check all independently retained identities against a request."""

    _root_request_binding(root, request)
    for name in ("source", "compiler", *_TOOL_IDENTITY_KEYS, "authority"):
        expected = request.get(name)
        if not isinstance(expected, Mapping):
            raise Rejected(f"request.{name} identity is missing")
        actual = _trust_root_descriptor(root, name, required=True)
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
    if allow_outputs:
        return
    for name in ("event_stream_stack", "event_stream_pcode", "envelope"):
        _trust_root_descriptor(root, name, required=True)


def _validate_hook_rows(value: Any, label: str = "hooks") -> list[dict[str, Any]]:
    if not isinstance(value, list) or len(value) != len(HOOKS):
        raise Rejected(f"{label} must contain the complete pinned hook union")
    result: list[dict[str, Any]] = []
    seen_addresses: set[int] = set()
    for index, (raw, expected) in enumerate(zip(value, HOOKS)):
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
        if normalized != expected:
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
        self.hooks = _validate_hook_rows([dict(row) for row in hooks], "dispatcher hooks")
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
        if accepted and row["role"] == "object_stack_write":
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
        self.dispatcher = SharedBreakpointDispatcher(backend, self)
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

    def _check_process(self, process_id: Any | None) -> None:
        if process_id is None:
            return
        self.bus.check_process(process_id)

    def _unknown(self, reason: str) -> None:
        reason = _text(reason, "unknown reason")
        if reason not in self.unknown:
            self.unknown.append(reason)

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
            self._unknown("incomplete inventory")
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
                self._unknown("incomplete inventory")
                self.inventory_complete = False
                continue
            normalized_rows: list[dict[str, Any]] = []
            seen_tokens: set[str] = set()
            for index, item in enumerate(rows):
                if not isinstance(item, Mapping):
                    self._unknown("null object identity")
                    continue
                pointer = item.get("pointer", item.get("object"))
                token = self.ledger.register(kind, pointer)
                if token is None:
                    self._unknown("null object identity" if pointer in (None, 0) else "duplicate object identity")
                    continue
                if token in seen_tokens:
                    self.ledger.mark_unknown(token, "duplicate object identity")
                    self._unknown("duplicate object identity")
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
                        self._unknown("missing or invalid Object VarInfo pointer")
                    else:
                        if varinfo_pointer == 0:
                            self._unknown("missing or invalid Object VarInfo pointer")
                        else:
                            prior_token = self.varinfo_owners.get(varinfo_pointer)
                            if prior_token not in (None, token):
                                self._unknown("duplicate Object VarInfo identity")
                            else:
                                self.varinfo_by_token[token] = varinfo_pointer
                                self.varinfo_owners[varinfo_pointer] = token
                normalized_rows.append(row)
            self.inventory_rows[key] = normalized_rows
        self.inventory_captured = True
        self.inventory_complete = not self.unknown and self.inventory_structure_ready
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
            # Keep the process-local EBX/EBP evidence until a complete Object
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
            self.function_entered = True
            self.bus.emit("stack", "function_entry", {"hook_id": row["id"]})
            return True
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
        expected_stack = {
            "function_filter", "allocation_pre", "allocation_post",
            *WRITE_HOOK_IDS,
        }
        expected_pcode = set(PCODE_HOOK_IDS)
        missing_stack = expected_stack - stack_hook_ids
        missing_pcode = expected_pcode - pcode_hook_ids
        if "regalloc_assignment" not in pcode_kinds:
            missing_pcode.add("regalloc")
        if "physical_reg_assignment" not in pcode_kinds:
            missing_pcode.add("regalloc_post")
        if missing_stack and "lane_unknown" not in stack_kinds:
            reason = "incomplete stack evidence"
            self._unknown(reason)
            self.bus.emit("stack", "lane_unknown", {"reason": reason})
        if missing_pcode and "lane_unknown" not in pcode_kinds:
            reason = "incomplete PCode evidence"
            self._unknown(reason)
            self.bus.emit("pcode", "lane_unknown", {"reason": reason})

    def on_hook_post(self, row: Mapping[str, Any], thread: int) -> None:
        payload = self.pending_writes.pop(thread, None)
        if payload is None or payload["hook_id"] != row["id"]:
            raise Rejected("single-step write chronology mismatch")
        self.bus.emit("stack", "object_stack_write_post", {**payload, "write_observed": True})
        self._maybe_complete_target()

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
        self._ensure_lane_completion()
        self.function_exited = True
        self.bus.emit("stack", "function_exit", {"exit_code": self.exit_code})

    def on_disconnect(self, reason: Any) -> None:
        raise Rejected(f"native debug transport disconnected: {_text(reason, 'disconnect reason')}")

    def _finalize_inventory(self) -> None:
        self._capture_inventory()
        for token in self.ledger.tokens("local") + self.ledger.tokens("argument"):
            if token not in self.mappings:
                self.ledger.mark_unknown(token, "incomplete regalloc")
                self._unknown("incomplete regalloc")
        if not self.inventory_complete:
            self._unknown("incomplete inventory")

    def _inventory(self) -> dict[str, Any]:
        self._finalize_inventory()
        rows: dict[str, list[dict[str, Any]]] = {"locals": [], "arguments": []}
        for key, kind in (("locals", "local"), ("arguments", "argument")):
            for raw in self.inventory_rows[key]:
                token = raw["token"]
                row = dict(raw)
                mapping = self.mappings.get(token)
                if mapping is None or self.ledger._by_token.get(token, {}).get("status") == "UNKNOWN":
                    row["ownership"] = {"status": "UNKNOWN", "reason": self.ledger._by_token.get(token, {}).get("reason", "incomplete regalloc")}
                else:
                    row["ownership"] = dict(mapping)
                rows[key].append(row)
        return {"status": "COMPLETE" if self.inventory_complete and not self.unknown else "UNKNOWN", **rows}

    def run(self) -> dict[str, Any]:
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
            return self.build_envelope()
        except Rejected:
            raise
        except Exception as exc:
            raise Rejected(f"native transport failure: {type(exc).__name__}: {exc}") from exc
        finally:
            # The dispatcher owns the union's restoration boundary.  The
            # backend's close() then closes handles/process state; no partial
            # breakpoint mutation survives a failed session.
            self.dispatcher.remove_all()

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
            "hooks": [dict(row) for row in HOOKS],
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
    _validate_compile_argv(argv, cwd=cwd, source=source, compiler=compiler, wrapper=tools["wrapper"])
    session_id = _safe_session_id(raw.get("session_id", _new_session_id()))
    hooks = _validate_hook_rows(raw.get("hooks", [dict(row) for row in HOOKS]), "manifest.hooks")
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


def authenticate_request(
    request_path: Path | str,
    *,
    require_empty: bool = False,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    if external_trust_root is not None and trust_root is not None:
        raise Rejected("conflicting external trust root arguments")
    root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
    if root is None:
        raise Rejected("external trust root is required for authenticated request")
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
    _validate_hook_rows(parsed["hooks"], "request.hooks")
    function = _text(parsed["function"], "request.function")
    source = _descriptor(parsed["source"], "request.source")
    if not _authorized_board_function(function, source):
        raise Rejected("unsupported target function")
    compiler = _descriptor(parsed["compiler"], "request.compiler")
    tools = {
        key: _descriptor(parsed[key], f"request.{key}")
        for key in _TOOL_IDENTITY_KEYS
    }
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
    )
    paths = _request_paths(parsed)
    output_dir = _canonical_path(parsed["output_dir"], "request.output_dir", directory=True, must_exist=True)
    if request_path.parent != output_dir or request_path.name != "request.json":
        raise Rejected("request/output directory binding mismatch")
    if require_empty:
        extras = [entry for entry in output_dir.iterdir() if entry.name != request_path.name]
        if extras:
            raise Rejected("capture output directory contains stale or partial files")
    _validate_external_root_against_request(root, request, request_path=request_path, allow_outputs=True)
    return {
        "request": request,
        "request_path": request_path,
        "request_sha256": sha256(request_path),
        "paths": paths,
        "hooks": [dict(row) for row in HOOKS],
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


def _remove_partial_outputs(auth: Mapping[str, Any] | None) -> None:
    if not isinstance(auth, Mapping):
        return
    paths = auth.get("paths")
    if not isinstance(paths, Mapping):
        return
    output_dir = auth.get("output_dir")
    candidates: list[Path] = [Path(value) for value in paths.values()]
    if output_dir is not None:
        directory = Path(output_dir)
        if directory.exists() and directory.is_dir():
            candidates.extend(entry for entry in directory.iterdir() if entry.name != "request.json")
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
) -> dict[str, Any]:
    """Run one fake/native backend and atomically publish its complete envelope."""

    auth: dict[str, Any] | None = None
    try:
        if external_trust_root is not None and trust_root is not None:
            raise Rejected("conflicting external trust root arguments")
        root = _coerce_external_trust_root(external_trust_root if external_trust_root is not None else trust_root)
        auth = authenticate_request(request_path, require_empty=True, external_trust_root=root)
        revalidate_request(auth)
        session = CombinedCaptureSession(auth, backend)
        envelope = session.run()
        revalidate_request(auth)
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
        validate_envelope(paths["envelope"], external_trust_root=output_root)
        return envelope
    except Exception as exc:
        _remove_partial_outputs(auth)
        if isinstance(exc, Rejected):
            raise
        raise Rejected(f"capture failure: {type(exc).__name__}: {exc}") from exc
    finally:
        close = getattr(backend, "close", None)
        if callable(close):
            try:
                close()
            except Exception as exc:
                _remove_partial_outputs(auth)
                raise Rejected(f"native cleanup failure: {type(exc).__name__}: {exc}") from exc


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
    if event_kind in {"function_entry", "numeric_stack_alloc_pre", "numeric_stack_alloc_post", "object_stack_write_pre", "object_stack_write_post", "pcode_capture"}:
        hook_id = _text(event["hook_id"], f"event[{index}].hook_id")
        hook = HOOK_BY_ID.get(hook_id)
        if hook is None:
            raise Rejected(f"event[{index}] hook_id is not owned")
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
            if not isinstance(ownership, Mapping) or not set(ownership).issubset({"status", "reason", "vreg_id", "bank"}):
                raise Rejected(f"inventory.{key}[{index}] ownership is invalid")
            if ownership.get("status") == "EXACT":
                if set(ownership) != {"status", "vreg_id", "bank"}:
                    raise Rejected(f"inventory.{key}[{index}] exact ownership is incomplete")
                vreg = _validate_vreg(ownership["vreg_id"], "inventory vreg_id")
                bank = ownership["bank"]
                if bank not in {"GPR", "FPR"} or not vreg.startswith("r" if bank == "GPR" else "f"):
                    raise Rejected(f"inventory.{key}[{index}] bank/vreg mismatch")
            elif ownership.get("status") != "UNKNOWN":
                raise Rejected(f"inventory.{key}[{index}] ownership status is invalid")
            elif "reason" not in ownership or not isinstance(ownership["reason"], str) or not ownership["reason"]:
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
    expected_writes = set(WRITE_HOOK_IDS)
    present_write_order = [
        str(event["hook_id"])
        for event in events
        if event["event_kind"] == "object_stack_write_pre"
    ]
    canonical_write_order = [hook for hook in WRITE_HOOK_IDS if hook in set(present_write_order)]
    present_write_order = list(dict.fromkeys(present_write_order))
    if present_write_order != canonical_write_order:
        raise Rejected("object write hook chronology is reversed")
    present_post_order = [
        str(event["hook_id"])
        for event in events
        if event["event_kind"] == "object_stack_write_post"
    ]
    canonical_post_order = [hook for hook in WRITE_HOOK_IDS if hook in set(present_post_order)]
    present_post_order = list(dict.fromkeys(present_post_order))
    if present_post_order != canonical_post_order:
        raise Rejected("object write post chronology is reversed")
    missing_writes = expected_writes - set(pre_by_hook) - set(post_by_hook)
    stack_unknown = [event for event in events if event["lane"] == "stack" and event["event_kind"] == "lane_unknown"]
    if missing_writes and not stack_unknown:
        raise Rejected("stack write chronology is incomplete without UNKNOWN")
    if stack_unknown and not missing_writes and set(pre_by_hook) == expected_writes and set(post_by_hook) == expected_writes:
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
    pcode_by_hook: dict[str, Mapping[str, Any]] = {}
    pcode_order: list[int] = []
    for event in pcode_rows:
        hook = str(event["hook_id"])
        if hook in pcode_by_hook:
            raise Rejected(f"duplicate PCode edge for {hook}")
        pcode_by_hook[hook] = event
        pcode_order.append(int(event["sequence"]))
    missing_pcode = set(PCODE_HOOK_IDS) - set(pcode_by_hook)
    pcode_unknown = [event for event in events if event["lane"] == "pcode" and event["event_kind"] == "lane_unknown"]
    regalloc_present = any(event["event_kind"] == "regalloc_assignment" for event in events)
    physical_present = any(event["event_kind"] == "physical_reg_assignment" for event in events)
    missing_allocator_edges = not regalloc_present or not physical_present
    if missing_pcode and not pcode_unknown:
        raise Rejected("PCode chronology is incomplete without UNKNOWN")
    if missing_allocator_edges and not pcode_unknown:
        raise Rejected("PCode chronology is incomplete without UNKNOWN")
    if pcode_unknown and not missing_pcode and len(pcode_rows) == len(PCODE_HOOK_IDS) and regalloc_present and physical_present:
        raise Rejected("PCode UNKNOWN marker is not justified by missing edges")
    present_order = [hook for hook in PCODE_HOOK_IDS if hook in pcode_by_hook]
    if [event["hook_id"] for event in pcode_rows] != present_order:
        raise Rejected("PCode hook chronology is reversed")
    if pcode_rows and any(event["status"] == "UNKNOWN" for event in pcode_rows):
        if not any(event["lane"] == "pcode" and event["event_kind"] == "lane_unknown" for event in events):
            # An individual UNKNOWN PCode record carries its own reason and is
            # sufficient; no lane marker is needed for a complete hook set.
            pass

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
    for key in ("locals", "arguments"):
        for row in inventory[key]:
            ownership = row["ownership"]
            if ownership["status"] == "EXACT":
                token = str(row["token"])
                vreg = str(ownership["vreg_id"])
                if exact_objects.get(token) != vreg:
                    raise Rejected("inventory ownership is not backed by a same-session regalloc edge")
            elif row["token"] in exact_objects:
                raise Rejected("UNKNOWN inventory ownership has an exact event claim")

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


def validate_envelope(
    envelope_path: Path | str,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    request_path: Path | str | None = None,
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
    if set(envelope) != expected_keys:
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
    if not isinstance(context, Mapping) or set(context) != {"session_id", "process_id", "function", "function_sha256", "argv", "cwd", "source", "compiler", "wrapper", "debugger", "transport", "request", "authority"}:
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
        _path_descriptor(context[name], f"envelope context {name}", must_exist=False)
    _path_descriptor(envelope["authority"], "envelope authority", must_exist=False)
    if context["authority"] != envelope["authority"]:
        raise Rejected("envelope authority descriptor is not shared with context")
    request_descriptor = context["request"]
    if not isinstance(request_descriptor, Mapping) or set(request_descriptor) != {"path", "size", "sha256"}:
        raise Rejected("envelope request descriptor is malformed")
    request_path_value = request_path if request_path is not None else getattr(root, "request_path", None)
    if request_path_value is None:
        raise Rejected("external trust root.request anchor is missing")
    authenticated_request = authenticate_request(
        request_path_value,
        external_trust_root=root,
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
    _validate_hook_rows(envelope["hooks"], "envelope.hooks")
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
        elif event["event_kind"] in {"pcode_capture", "regalloc_assignment", "physical_reg_assignment"} and event.get("status") == "UNKNOWN":
            derived_unknown.add(str(event["reason"]))
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
_SOURCE_SPAN_ROLES = {"declaration", "read", "write", "call_return", "evaluation"}


def seal_source_span_manifest(value: Mapping[str, Any]) -> dict[str, Any]:
    """Seal a source-span binding manifest without granting source authority."""

    result = dict(value)
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
        "schema": f"{SOURCE_SPAN_SCHEMA}/seal",
        "status": "READY",
        "input": _path_descriptor(source, "unsealed source span manifest", must_exist=True),
        "output": _path_descriptor(output, "sealed source span manifest", must_exist=True),
        "manifest_sha256": sealed["manifest_sha256"],
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
    expected = {
        "schema", "function", "function_sha256", "session_id", "source",
        "spans", "authority_advanced", "manifest_sha256",
    }
    if set(value) != expected:
        raise Rejected("source span manifest contains unsupported or missing fields")
    unsigned = {key: item for key, item in value.items() if key != "manifest_sha256"}
    if value.get("manifest_sha256") != canonical_hash(unsigned):
        raise Rejected("source span manifest self-digest mismatch")
    if value.get("schema") != SOURCE_SPAN_SCHEMA or value.get("authority_advanced") is not False:
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
    spans = value.get("spans")
    if not isinstance(spans, list) or not spans:
        raise Rejected("source span manifest requires at least one span")
    claimed_ranges: dict[tuple[int, int], str] = {}
    seen: set[tuple[str, str, int, int]] = set()
    for index, raw in enumerate(spans):
        if not isinstance(raw, Mapping) or set(raw) != _SOURCE_SPAN_FIELDS:
            raise Rejected(f"source span manifest spans[{index}] shape mismatch")
        token = _text(raw["object_token"], f"source span manifest spans[{index}].object_token")
        token_match = TOKEN_RE.fullmatch(token)
        if token_match is None or token_match.group("session") != context["session_id"]:
            raise Rejected("source span manifest token is not capture-local")
        inventory_row = inventory_by_token.get(token)
        if inventory_row is None:
            raise Rejected("source span manifest token is not in the authenticated inventory")
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
        range_key = (start, end)
        prior = claimed_ranges.get(range_key)
        if prior not in (None, token):
            raise Rejected("one source span is claimed by multiple Object tokens")
        claimed_ranges[range_key] = token
        unique = (token, role, start, end)
        if unique in seen:
            raise Rejected("source span manifest contains a duplicate binding")
        seen.add(unique)
    chronology = _donor_cfg.source_chronology(source_path, symbol=str(context["function"]))
    if chronology["source"]["sha256"] != source["sha256"]:
        raise Rejected("source chronology parser did not consume the capture-bound source")
    return dict(value), chronology


def _tool_source_descriptor(module: Any) -> dict[str, Any]:
    path = Path(module.__file__).resolve()
    return _path_descriptor(path, f"tool {module.__name__}", must_exist=True)


def build_source_aware_causal_map(
    envelope_path: Path | str,
    source_span_manifest: Path | str,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any],
    frontend_chronology: Path | str | None = None,
) -> dict[str, Any]:
    """Join source spans to same-session physical/stack evidence fail-closed."""

    envelope = validate_envelope(envelope_path, trust_root=trust_root)
    manifest, source_chronology = _validate_source_span_manifest(source_span_manifest, envelope)
    events = envelope["events"]
    spans_by_token: dict[str, list[dict[str, Any]]] = {}
    for span in manifest["spans"]:
        spans_by_token.setdefault(str(span["object_token"]), []).append(dict(span))

    physical_by_token: dict[str, list[dict[str, Any]]] = {}
    stack_by_token: dict[str, list[dict[str, Any]]] = {}
    for event in events:
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

    joined: list[dict[str, Any]] = []
    for row in inventory_rows:
        token = str(row["token"])
        name = row.get("name") if isinstance(row.get("name"), str) else ""
        ownership = row.get("ownership") if isinstance(row.get("ownership"), Mapping) else {}
        vreg_id = ownership.get("vreg_id") if ownership.get("status") == "EXACT" else None
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
        if not bound_spans:
            status, confidence, score = "UNKNOWN", "none", 0.0
            reasons = ["authenticated source-span binding is absent"]
        elif len(physical) > 1:
            status, confidence, score = "UNKNOWN", "none", 0.0
            reasons = ["one Object token has multiple physical-register assignments"]
        related_calls = [
            call
            for call in source_chronology["calls"]
            if name and call.get("assigned_lhs") == name
        ]
        joined.append({
            "object_token": token,
            "identity": name or None,
            "status": status,
            "confidence": confidence,
            "score": score,
            "evidence": reasons,
            "source_spans": bound_spans,
            "virtual_register": vreg_id,
            "physical_register": physical[0] if len(physical) == 1 else None,
            "stack_chronology": stack_by_token.get(token, []),
            "call_return_chronology": related_calls,
        })

    frontend: dict[str, Any]
    if frontend_chronology is None:
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
        "tools": {
            "same_session": _path_descriptor(Path(__file__).resolve(), "tool same_session", must_exist=True),
            "stack_home": _tool_source_descriptor(_stack_home),
            "frontend_chronology": _tool_source_descriptor(_frontend_chronology),
            "correlator": _tool_source_descriptor(_correlator),
            "donor_cfg": _tool_source_descriptor(_donor_cfg),
        },
        "joined_objects": joined,
        "source_evaluation_chronology": source_chronology,
        "frontend_chronology": frontend,
        "unknown": sorted({reason for row in joined if row["status"] != "MATCHED_AUTHENTICATED" for reason in row["evidence"]} | ({frontend["reason"]} if frontend["status"] == "UNKNOWN" else set())),
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
            "capture_pcode",
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
        self._owned_handles: set[int] = {value for value in (int(process or 0), int(initial_thread or 0)) if value}
        self.transport_threads: dict[int, int] = {}
        if initial_thread:
            self.transport_threads[0] = int(initial_thread)
        self.threads: dict[int, int] = {}
        self.base = int(base)
        self.breakpoints: dict[int, int] = {}
        self.pending_steps: dict[int, int] = {}
        self.exited = False
        self.loader_breakpoint_pending = False
        # These maps are process-local and never cross the event boundary.
        # They also make duplicate/cross-kind identities fail closed before
        # CombinedCaptureSession assigns its capture-local tokens.
        self._object_kind: dict[int, str] = {}
        self._object_varinfo: dict[int, int] = {}
        self._varinfo_object: dict[int, int] = {}
        self._direct_vreg_evidence: dict[int, dict[str, Any]] = {}
        self._pcode_events: list[dict[str, Any]] = []
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
        self._selection_mode: str | None = None

    def _runtime(self, absolute: int) -> int:
        return absolute - KNOWN_IMAGE_BASE + self.base

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

    def _close_debug_file(self, info: Any) -> None:
        file_handle = _native_value(getattr(info, "hFile", 0))
        if file_handle:
            self.native.kernel32.CloseHandle(file_handle)

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
        if not self._transport_image_seen:
            raise Rejected("compiler child appeared before wrapper authentication")
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
        self.base = image_base
        self.threads[int(thread_id)] = thread_handle
        self._owned_handles.update((process_handle, thread_handle))
        self.loader_breakpoint_pending = True
        self._pending_create_event = (int(process_id), int(thread_id))
        self._close_debug_file(info)
        session.on_process_started(int(process_id))

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
        for row in HOOKS:
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
        if not self.compiler_path or not self.wrapper_path:
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
        """Read the verified post-allocation EBX Object*/EBP VarInfo* pair.

        ``regalloc_post`` lands on the GC/2.6 epilogue at 0x004D03E8.  At
        that site EBX still names the compiler Object and EBP still names its
        VarInfo.  Only the five authenticated VarInfo fields cross this
        backend boundary; the Object/VarInfo addresses remain process-local
        and are checked against the captured list ledger first.
        """

        hook = HOOK_BY_ID.get(str(hook_id))
        if hook is None or hook.get("role") != "regalloc_post":
            raise Rejected("unowned physical register-allocation hook")
        object_pointer = self.read_register(thread_id, "ebx")
        varinfo_pointer = self.read_register(thread_id, "ebp")
        if object_pointer == 0:
            raise Rejected("physical assignment has a null Object register")
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
        return {
            "object": object_pointer,
            "varinfo_pointer": varinfo_pointer,
            "noregister": data[VARINFO_NOREGISTER_FIELD],
            "flags": data[VARINFO_FLAGS_FIELD],
            "rclass": data[VARINFO_CLASS_FIELD],
            "reg": int.from_bytes(data[VARINFO_REG_FIELD:VARINFO_REG_FIELD + 0x2], "little", signed=True),
            "reg_hi": int.from_bytes(data[VARINFO_REG_HI_FIELD:VARINFO_REG_HI_FIELD + 0x2], "little", signed=True),
        }

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
        if self._pending_create_event is not None:
            self._continue_debug_event(*self._pending_create_event)
            self._pending_create_event = None
        if self._pending_debug_event is not None:
            self._continue_debug_event(*self._pending_debug_event)
            self._pending_debug_event = None
        while not self.exited:
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == getattr(self.native, "ERROR_SEM_TIMEOUT", 121):
                    raise Rejected("native debug transport gap or timeout")
                raise Rejected(f"WaitForDebugEvent failed: {error}")
            code = int(event.dwDebugEventCode)
            pid = int(event.dwProcessId)
            tid = int(event.dwThreadId)
            # DEBUG_PROCESS also reports the wrapper's loader/transport events.
            # They are continued without inspection; every other process must
            # be the one authenticated compiler child.
            if pid == self.transport_process_id and pid != self.compiler_process_id:
                if code == getattr(self.native, "EXIT_PROCESS_DEBUG_EVENT", 5):
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
                session.on_process_exit(int(event.u.ExitProcess.dwExitCode), pid)
            elif code == getattr(self.native, "EXCEPTION_DEBUG_EVENT", 1):
                session._check_process(pid)
                record = event.u.Exception.ExceptionRecord
                exception_code = int(record.ExceptionCode)
                address = int(record.ExceptionAddress or 0)
                normalized = address - self.base + KNOWN_IMAGE_BASE
                if exception_code in (getattr(self.native, "EXCEPTION_SINGLE_STEP", 0x80000004), getattr(self.native, "EXCEPTION_WX86_SINGLE_STEP", 0x4000001E)):
                    session.on_single_step(tid, pid)
                elif exception_code in (getattr(self.native, "EXCEPTION_BREAKPOINT", 0x80000003), getattr(self.native, "EXCEPTION_WX86_BREAKPOINT", 0x4000001F)):
                    if normalized in session.dispatcher.by_address:
                        self.loader_breakpoint_pending = False
                        session.on_breakpoint(normalized, tid, pid)
                    elif self.loader_breakpoint_pending:
                        # Windows emits an initial loader breakpoint before
                        # user code.  It is transport noise, not an owned
                        # compiler hook, and must not enter the event ledger.
                        self.loader_breakpoint_pending = False
                    else:
                        raise Rejected(f"unexpected non-loader breakpoint 0x{normalized:08x}")
                else:
                    raise Rejected(f"unsupported native exception 0x{exception_code:08x}")
            else:
                session._check_process(pid)
            self._continue_debug_event(pid, tid)

    def close(self) -> None:
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
        # DEBUG_PROCESS gives us handles for both the wrapper and compiler
        # CREATE_PROCESS events.  Close each raw handle once at the durable
        # cleanup boundary; never let a transport handle remain attached to a
        # later capture.
        for handle in sorted(self._owned_handles):
            try:
                self.native.kernel32.CloseHandle(handle)
            except Exception as exc:
                errors.append(f"native handle {handle}: {type(exc).__name__}: {exc}")
        self._owned_handles.clear()
        self.threads.clear()
        if errors:
            raise Rejected("native cleanup restoration failed: " + "; ".join(errors))


def launch_native_capture(
    request_path: Path | str,
    external_trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
    *,
    trust_root: ExternalTrustRoot | Mapping[str, Any] | None = None,
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
    if not callable(virtual_query) or not callable(debug_break):
        raise Rejected("native kernel32 lacks same-process compiler handoff APIs")
    virtual_query.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(_MEMORY_BASIC_INFORMATION), ctypes.c_size_t]
    virtual_query.restype = ctypes.c_size_t
    debug_break.argtypes = [ctypes.c_void_p]
    debug_break.restype = wintypes.BOOL
    request = auth["request"]
    argv = _native_launch_argv(request)
    command = subprocess.list2cmdline(argv)
    startup = native.STARTUPINFOW(cb=ctypes.sizeof(native.STARTUPINFOW), dwFlags=native.STARTF_USESHOWWINDOW, wShowWindow=native.SW_HIDE)
    process_info = native.PROCESS_INFORMATION()
    buffer = ctypes.create_unicode_buffer(command)
    # The authenticated argv starts with sjiswrap.exe.  DEBUG_ONLY_THIS_PROCESS
    # would still be sufficient for sjiswrap's manual-map handoff, but
    # DEBUG_PROCESS also covers launchers that create a real compiler child;
    # the backend authenticates either the child image or sjiswrap's private
    # mwcceppc mapping before any hook read.
    created = native.kernel32.CreateProcessW(None, buffer, None, None, False, DEBUG_PROCESS | native.CREATE_NO_WINDOW, None, request["cwd"], ctypes.byref(startup), ctypes.byref(process_info))
    if not created:
        raise Rejected(f"CreateProcessW failed: {ctypes.WinError(ctypes.get_last_error())}")
    pid = _native_value(process_info.dwProcessId)
    if pid <= 0:
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
    )
    return capture_with_backend(request_path, backend, external_trust_root=root)


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
    if len(HOOKS) != 8 or len(HOOK_BY_ADDRESS) != 8:
        raise Rejected("hook union is not closed")
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
    validate = sub.add_parser("validate", help="validate a completed envelope")
    validate.add_argument("envelope", type=Path)
    validate.add_argument("--trust-root", type=Path, required=True)
    causal = sub.add_parser("causal-map", help="join authenticated source spans to physical/stack chronology")
    causal.add_argument("--envelope", type=Path, required=True)
    causal.add_argument("--trust-root", type=Path, required=True)
    causal.add_argument("--source-spans", type=Path, required=True)
    causal.add_argument("--frontend-chronology", type=Path)
    causal.add_argument("--output", type=Path)
    seal_spans = sub.add_parser("seal-source-spans", help="seal a reviewed capture-local source-span manifest")
    seal_spans.add_argument("--input", type=Path, required=True)
    seal_spans.add_argument("--output", type=Path, required=True)
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
            result = {"schema": f"{REQUEST_SCHEMA}/preflight", "status": "READY", "session_id": auth["request"]["session_id"], "function": auth["request"]["function"], "request_sha256": auth["request_sha256"], "hooks": [dict(row) for row in HOOKS], "diagnostic_only": True, "board_admission": False}
        elif args.command == "capture":
            result = launch_native_capture(args.request, external_trust_root=_load_trust_root(args.trust_root))
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
            )
            if args.output is not None:
                output = args.output.resolve()
                if output.exists() or output.is_symlink():
                    raise Rejected("causal-map output already exists")
                write_new(output, (json.dumps(result, indent=2, sort_keys=True) + "\n").encode("utf-8"))
        elif args.command == "seal-source-spans":
            result = seal_source_span_file(args.input, args.output)
        else:
            result = self_test()
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0 if result.get("status") in {"OK", "READY", "CAPTURED", "CAPTURED_UNKNOWN_OWNERSHIP"} else 2
    except (OSError, Rejected, ValueError, json.JSONDecodeError) as exc:
        print(json.dumps(unknown_result(str(exc)), indent=2, sort_keys=True))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
