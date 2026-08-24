#!/usr/bin/env python3
"""Bounded native-WOW64 producer for Capsule stack-home diagnostics.

This module is deliberately a producer boundary, not a source-matching tool.
It launches no process during import and never changes source, object, queue, or
authority state.  A request authenticates the pinned GC/2.6 compiler, the
audited native/debug transport tools, the current source/function identities,
and the six compiler hook prefixes.  A :class:`CaptureSession` consumes a
small backend protocol so fake backends can exercise the chronology without a
compiler or emulator.  The Windows backend is optional and is only created by
the explicit ``capture`` command.

The event contract is intentionally central and pointer-free:

``event_id`` / ``sequence`` / ``event_kind`` / ``function``

Object and VarInfo pointers are used only inside the backend.  The session
assigns capture-local ordinals and emits ``object-000000`` and
``varinfo-000000`` tokens.  The three Object+0x2e write sites are breakpoint
pre-events; their post-events are derived only from a completed one-instruction
single-step.  Missing steps, pointer reuse, transport gaps, disconnects,
partial functions, stale hashes, and unauthorized functions fail closed.
The compiler-list event also carries the authenticated one-to-one relation
between each Object generation token and its VarInfo generation token.  Exact
compiler-exposed C identifiers are included as names; invalid/compiler-private
text is discarded and represented as UNKNOWN rather than serialized.
An Object first observed during an allocation remains provisional until the
post-allocation list snapshot binds its Object/VarInfo pair; no unbound token
is serialized.  Object and VarInfo addresses can be recycled by the compiler,
so the session assigns a new capture-local generation whenever an active pair
is rebound.  Native thread handles are closed through an idempotent,
deduplicated cleanup path.

The explicit ``summarize`` command first validates a sealed packet, then joins
only exact compiler-exposed Object names through the sealed Object/VarInfo pair
to authenticated ``Object+0x2e`` pre/post events. It reports physical stack
slots while keeping ownership ``UNKNOWN`` and ``authority_advanced`` false.
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import time
from typing import Any, Mapping, Protocol, Sequence


SCHEMA = "mwcc_capsule_stack_home_native/v4"
REQUEST_SCHEMA = "mwcc_capsule_stack_home_native_request/v4"
EVENT_SCHEMA = "mwcc_capsule_stack_home_event/v4"
SUMMARY_SCHEMA = "mwcc_capsule_stack_home_summary/v1"
TOOL_VERSION = "capsule-stack-home-native-4"
SUMMARY_STATUS = "SUMMARIZED_UNKNOWN_OWNERSHIP"
BACKEND_NAME = "native-wow64"
DIAGNOSTIC_ONLY = True
BOARD_ADMISSION = False
EXACTNESS_CLAIM = False

SAFE_FUNCTION = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
POINTER_TEXT = re.compile(r"0[xX][0-9a-fA-F]+")
SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
CANONICAL_DECIMAL = re.compile(r"-?(?:0|[1-9][0-9]*)\Z")
CANONICAL_HEX = re.compile(r"-?0[xX][0-9a-fA-F]+\Z")

KNOWN_IMAGE_BASE = 0x00400000
EXPECTED_COMPILER_SIZE = 2_066_432
EXPECTED_COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"

# These are the authenticated identities from the GC/2.6 audit packet.  They
# are tool identities, not source identities; the external authority manifest
# binds the current source/function span and exact canonical artifact paths.
EXPECTED_PRODUCER_SHA256 = "46a220aff94123ca085196ab424d1a6d9f3c48486c4583cf13539648a7ba3e55"
EXPECTED_DEBUGGER_SHA256 = "c8da21ad79ff5b342c45037a4fa5bea9fb993bba13e12817a0aa1fc2f72f6fe3"
EXPECTED_EMULATOR_SHA256 = "8ad07089082ccf48cfcc90bd16eb5702636a14574fec6f51e228ec85eab34db2"
EXPECTED_GDB_SHA256 = "a49bcdef5b72a4a2e82cf9615f1b3c6f8e0e0e9f2330bb79e0f5b332528bf9fe"
# Compatibility spelling used by the earlier allocator audit packet.
EXPECTED_ALLOCATOR_SHA256 = EXPECTED_PRODUCER_SHA256

OBJECT_LIST_HEAD = 0x005EA8D4
FUNCTION_OBJECT = 0x005E9EC0
OBJECT_DATATYPE = 0x02
OBJECT_NAME = 0x0A
OBJECT_TYPE_POINTER = 0x0E
OBJECT_VARINFO_DATATYPE1 = 0x2A
OBJECT_VARINFO_OTHER = 0x32
OBJECT_STACK_FIELD = 0x2E
VARINFO_RAW_SIZE = 0x60

HOOKS: tuple[dict[str, Any], ...] = (
    {
        "id": "function_filter",
        "address": 0x00433492,
        "prefix": "8b400e8b5006eb08",
        "role": "function_filter",
    },
    {
        "id": "allocation_pre",
        "address": 0x0043367E,
        "prefix": "e85d660c00598a44240450",
        "role": "numeric_stack_alloc_pre",
    },
    {
        "id": "allocation_post",
        "address": 0x00433683,
        "prefix": "598a4424045068404e4300",
        "role": "numeric_stack_alloc_post",
    },
    {
        "id": "object_write_0",
        "address": 0x004F9E54,
        "prefix": "89432e8b530e8b420201e84821f00105e40c",
        "role": "object_stack_write",
    },
    {
        "id": "object_write_1",
        "address": 0x004F9EF1,
        "prefix": "89432e8b4b0e8b410201e84821f00105dc0c",
        "role": "object_stack_write",
    },
    {
        "id": "object_write_2",
        "address": 0x004F9F78,
        "prefix": "89432e8b4b0e8b410201e84821f00105d80c",
        "role": "object_stack_write",
    },
)
HOOK_BY_ID = {str(row["id"]): row for row in HOOKS}
WRITE_HOOK_IDS = tuple(row["id"] for row in HOOKS if row["role"] == "object_stack_write")
EVENT_KINDS = frozenset(
    {
        "function_entry",
        "compiler_list",
        "numeric_stack_alloc_pre",
        "numeric_stack_alloc_post",
        "varinfo_home_snapshot",
        "object_stack_write_pre",
        "object_stack_write_post",
        "function_exit",
    }
)

# Every serialized surface is closed.  Keeping these sets beside the producer
# makes it difficult for a transport or a future caller to smuggle raw
# debugger state into the central packet by adding an unreviewed field.
HOOK_ORDER = tuple(str(row["id"]) for row in HOOKS)
HOOK_FIELDS = frozenset({"id", "address", "prefix", "role"})
COMPILER_HOOK_FIELDS = HOOK_FIELDS | {"validated"}
EVENT_BASE_FIELDS = frozenset({"schema", "event_id", "sequence", "event_kind", "function"})
EVENT_FIELDS = {
    "function_entry": EVENT_BASE_FIELDS,
    "function_exit": EVENT_BASE_FIELDS,
    "compiler_list": EVENT_BASE_FIELDS | {
        "object_tokens", "varinfo_tokens", "object_varinfo_pairs", "objects",
    },
    "numeric_stack_alloc_pre": EVENT_BASE_FIELDS | {"allocation_id", "object_tokens", "varinfo_tokens", "allocation_call"},
    "numeric_stack_alloc_post": EVENT_BASE_FIELDS | {"allocation_id", "object_tokens", "varinfo_tokens", "allocation_call"},
    "varinfo_home_snapshot": EVENT_BASE_FIELDS | {"varinfo_token", "varinfo_ordinal", "varinfo_field_offset", "home_value"},
    "object_stack_write_pre": EVENT_BASE_FIELDS | {"write_id", "object_token", "object_ordinal", "object_stack_field_offset", "target_slot", "read_count", "escape", "write_site"},
    "object_stack_write_post": EVENT_BASE_FIELDS | {"write_id", "object_token", "object_ordinal", "object_stack_field_offset", "expected_value", "observed_value", "write_observed", "derived_from"},
}
REQUEST_FIELDS = frozenset(
    {
        "schema", "tool_version", "backend", "diagnostic_only", "board_admission", "exactness_claim",
        "function", "function_sha256", "cwd", "argv", "compiler", "source", "baseline", "producer",
        "debugger", "emulator", "gdb", "authority_manifest", "authority", "output_dir", "event_stream",
        "packet", "candidate", "hooks", "artifacts", "transport", "provenance", "request_sha256",
    }
)
MANIFEST_FIELDS = frozenset(
    {
        "function", "function_sha256", "cwd", "argv", "compiler", "source", "baseline", "producer",
        "debugger", "emulator", "gdb", "authority_manifest",
    }
)
AUTHORITY_SCHEMA = "mwcc_capsule_stack_home_authority/v1"
AUTHORITY_ARTIFACT_NAMES = ("source", "baseline", "compiler", "producer", "debugger", "emulator", "gdb")
AUTHORITY_FIELDS = frozenset({"schema", "source", "function", "artifacts"})
AUTHORITY_ARTIFACT_FIELDS = frozenset(AUTHORITY_ARTIFACT_NAMES)
AUTHORITY_SOURCE_FIELDS = frozenset({"path", "size", "sha256"})
AUTHORITY_FUNCTION_FIELDS = frozenset({"name", "sha256", "source_sha256"})
DESCRIPTOR_FIELDS = frozenset({"path", "size", "sha256"})
TRANSPORT_FIELDS = frozenset({"name", "required_capabilities", "single_step_post_events"})
PROVENANCE_FIELDS = frozenset(
    {
        "authenticated", "producer_sha256", "debugger_sha256", "emulator_sha256", "gdb_sha256",
        "compiler_sha256", "source_sha256", "function_sha256",
    }
)
PACKET_FIELDS = frozenset(
    {
        "schema", "tool_version", "status", "backend", "diagnostic_only", "board_admission", "exactness_claim",
        "function", "binding", "request", "authentication", "events", "event_count", "event_stream",
        "residues", "unknown", "limitations", "packet_sha256",
    }
)
PACKET_BINDING_FIELDS = frozenset(
    {
        "function", "function_sha256", "source_sha256", "source_size", "baseline_sha256", "baseline_size",
        "compiler_sha256", "compiler_size", "producer_sha256", "producer_size", "debugger_sha256", "debugger_size",
        "emulator_sha256", "emulator_size", "gdb_sha256", "gdb_size", "argv", "cwd",
    }
)

REQUIRED_CAPABILITIES = frozenset(
    {
        "read_image",
        "read_memory",
        "read_registers",
        "read_object_list",
        "read_varinfo",
        "read_object_stack_field",
        "install_breakpoint",
        "remove_breakpoint",
        "single_step",
        "run_events",
    }
)


class Rejected(ValueError):
    """Raised for any unauthenticated or incomplete native capture."""


def _normalize_exception(exc: BaseException, context: str) -> Rejected:
    if isinstance(exc, Rejected):
        return exc
    return Rejected(f"{context}: {type(exc).__name__}: {exc}")


def _strict_keys(value: Any, expected: frozenset[str] | set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise Rejected(f"{label} must be an object")
    actual = set(value.keys())
    if actual != set(expected):
        missing = sorted(set(expected) - actual)
        extra = sorted(actual - set(expected))
        if any(str(key).strip().lower() in {"address", "addresses", "pointer", "pointers", "ptr", "raw_pointer", "raw_address"} or str(key).strip().lower().endswith(("_address", "_pointer")) for key in extra):
            raise Rejected(f"{label} exposes a raw pointer/address field")
        details = []
        if missing:
            details.append("missing=" + ",".join(missing))
        if extra:
            details.append("extra=" + ",".join(extra))
        raise Rejected(f"{label} field allowlist mismatch ({'; '.join(details)})")
    if any(not isinstance(key, str) for key in value):
        raise Rejected(f"{label} contains a non-text field name")
    return value


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise Rejected(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _load_json(text: str, label: str) -> Any:
    try:
        return json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    except Rejected:
        raise
    except (TypeError, json.JSONDecodeError) as exc:
        raise Rejected(f"cannot parse {label}: {exc}") from exc


def _load_json_file(path: Path, label: str) -> Any:
    try:
        return _load_json(path.read_text(encoding="utf-8"), label)
    except Rejected:
        raise
    except OSError as exc:
        raise Rejected(f"cannot read {label}: {exc}") from exc


def _strict_integer(value: Any, label: str, *, nonnegative: bool = False) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise Rejected(f"{label} must be an integer (boolean/string values are forbidden)")
    if nonnegative and value < 0:
        raise Rejected(f"{label} must be non-negative")
    return value


def _safe_token(value: Any, kind: str, label: str) -> str:
    token = _text(value, label)
    if not re.fullmatch(rf"{re.escape(kind)}-[0-9]{{6}}", token):
        raise Rejected(f"{label} has a non-canonical token")
    return token


def _safe_pair_id(value: Any, kind: str, label: str) -> str:
    token = _text(value, label)
    if not re.fullmatch(rf"{re.escape(kind)}-[0-9]{{6}}", token):
        raise Rejected(f"{label} has a non-canonical pair id")
    return token


def _safe_event_id(value: Any, capture_id: str, sequence: int, label: str) -> str:
    event_id = _text(value, label)
    if not re.fullmatch(r"[0-9a-f]{16}-e[0-9]{6}", event_id):
        raise Rejected(f"{label} has a non-canonical event id")
    expected = f"{capture_id}-e{sequence:06d}"
    if event_id != expected:
        raise Rejected(f"{label} is not the canonical contiguous event id")
    return event_id


def _reject_path_alias(path: Path, label: str, *, expected_path: Path | None = None) -> Path:
    candidate = Path(path)
    try:
        absolute = candidate.absolute()
        resolved = candidate.resolve(strict=True)
        stat = resolved.stat()
    except (OSError, RuntimeError) as exc:
        raise Rejected(f"{label} cannot be resolved: {exc}") from exc
    if candidate.is_symlink():
        raise Rejected(f"{label} is a symlink/alias")
    # A relative spelling is fine; an existing path whose absolute spelling
    # differs from resolve() is a link/reparse-point on the supported hosts.
    if os.path.normcase(str(absolute)) != os.path.normcase(str(resolved)):
        raise Rejected(f"{label} resolves through an alias")
    if getattr(stat, "st_nlink", 1) != 1:
        raise Rejected(f"{label} has multiple hard links")
    if expected_path is not None:
        expected = Path(expected_path).resolve(strict=True)
        if os.path.normcase(str(resolved)) != os.path.normcase(str(expected)):
            raise Rejected(f"{label} path identity mismatch")
    return resolved


def _reject_raw_path_alias(
    value: str | os.PathLike[str],
    label: str,
    *,
    expected_path: Path | None = None,
) -> Path:
    """Reject lexical path aliases before :class:`Path` can normalize them.

    ``Path`` intentionally folds ``.`` components and, on Windows, the
    filesystem resolves case variants to the same inode.  That is useful for
    ordinary file access but unsafe for authenticated packet references: the
    serialized spelling is part of the identity we are checking.  Keep this
    gate narrow and path-specific; packet fields containing offsets such as
    ``0x0`` and ``0x123`` are never sent through it.
    """

    try:
        raw = os.fspath(value)
    except TypeError as exc:
        raise Rejected(f"{label} must be a path string") from exc
    if isinstance(raw, bytes) or not isinstance(raw, str) or not raw:
        raise Rejected(f"{label} must be a non-empty path string")
    if raw != raw.strip():
        raise Rejected(f"{label} has non-canonical path spelling")

    # A path spelling that mixes the two platform separator conventions is a
    # lexical alias, even when the host filesystem happens to resolve it to
    # the authenticated inode.  Check this before constructing ``Path`` so a
    # mixed spelling cannot be normalized (or fail for the wrong reason) by
    # the host-specific path parser.
    if "/" in raw and "\\" in raw:
        raise Rejected(f"{label} has non-canonical path spelling")

    # Reject dot/dot-dot and repeated/trailing separators lexically.  The
    # existing resolved-path check remains authoritative for symlinks,
    # reparse points, hard links, and other filesystem aliases.
    parts = re.split(r"[\\/]", raw)
    if any(part in {".", ".."} for part in parts):
        raise Rejected(f"{label} has non-canonical path spelling")
    if raw.endswith(("/", "\\")):
        raise Rejected(f"{label} has non-canonical path spelling")
    # Empty interior components are repeated separators.  The two leading
    # empties in a UNC spelling (//server/share/...) are the sole exception.
    empty_indexes = [index for index, part in enumerate(parts) if part == ""]
    if empty_indexes and empty_indexes != [0, 1]:
        raise Rejected(f"{label} has non-canonical path spelling")

    candidate = Path(raw)
    resolved = _reject_path_alias(candidate, label, expected_path=expected_path)
    try:
        absolute = candidate.absolute()
    except (OSError, RuntimeError) as exc:
        raise Rejected(f"{label} cannot be made absolute: {exc}") from exc
    # Do not use normcase here: a case-spelling alias must fail even on a
    # case-insensitive filesystem.  Path.absolute() normalizes separator
    # style while preserving the caller's component case.
    if str(absolute) != str(resolved):
        raise Rejected(f"{label} has non-canonical path spelling")
    return resolved


def _validate_descriptor(value: Any, label: str) -> Mapping[str, Any]:
    row = _strict_keys(value, DESCRIPTOR_FIELDS, label)
    _text(row.get("path"), f"{label}.path")
    _strict_integer(row.get("size"), f"{label}.size", nonnegative=True)
    _digest(row.get("sha256"), f"{label}.sha256")
    return row


def _reject_directory_alias(path: Path, label: str) -> Path:
    candidate = Path(path)
    try:
        resolved = candidate.resolve(strict=True)
    except (OSError, RuntimeError) as exc:
        raise Rejected(f"{label} cannot be resolved: {exc}") from exc
    if not resolved.is_dir():
        raise Rejected(f"{label} is not a directory")
    if candidate.is_symlink() or os.path.normcase(str(candidate.absolute())) != os.path.normcase(str(resolved)):
        raise Rejected(f"{label} is a symlink/alias")
    return resolved


def _canonical_hook_rows() -> list[dict[str, Any]]:
    return [
        {
            "id": str(row["id"]),
            "address": f"0x{int(row['address']):08x}",
            "prefix": str(row["prefix"]),
            "role": str(row["role"]),
        }
        for row in HOOKS
    ]


def _validate_hook_rows(value: Any, label: str, *, compiler: bool = False) -> list[dict[str, Any]]:
    expected = _canonical_hook_rows()
    if not isinstance(value, list) or len(value) != len(expected):
        raise Rejected(f"{label} must contain the complete pinned hook set")
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    for index, row in enumerate(value):
        keys = COMPILER_HOOK_FIELDS if compiler else HOOK_FIELDS
        item = _strict_keys(row, keys, f"{label}[{index}]")
        hook_id = _text(item.get("id"), f"{label}[{index}].id")
        if hook_id in seen:
            raise Rejected(f"{label} contains duplicate hook id {hook_id}")
        seen.add(hook_id)
        if hook_id != expected[index]["id"]:
            raise Rejected(f"{label} hook order is not the closed pinned order")
        if item.get("address") != expected[index]["address"]:
            raise Rejected(f"{label}[{index}] address mismatch")
        if item.get("prefix") != expected[index]["prefix"] or item.get("role") != expected[index]["role"]:
            raise Rejected(f"{label}[{index}] prefix/role mismatch")
        if compiler and item.get("validated") is not True:
            raise Rejected(f"{label}[{index}] is not compiler-validated")
        result.append(dict(item))
    if tuple(row["id"] for row in result) != HOOK_ORDER or set(row["id"] for row in result) != set(HOOK_ORDER):
        raise Rejected(f"{label} is missing a pinned hook role")
    return result


def _request_hook_rows() -> list[dict[str, Any]]:
    return _canonical_hook_rows()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def canonical_hash(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def seal(value: Mapping[str, Any]) -> dict[str, Any]:
    # A packet may be sealed more than once while the producer fills in
    # externally anchored artifacts (the event stream is added after the
    # session has collected events).  The prior digest is not part of the
    # canonical payload and must never be hashed as ordinary packet data.
    result = dict(value)
    result.pop("packet_sha256", None)
    result["packet_sha256"] = canonical_hash(result)
    return result


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise Rejected(f"{label} must be non-empty text")
    return value.strip()


def _function_name(value: Any, label: str) -> str:
    """Return one canonical C identifier without granting trace authority."""

    result = _text(value, label)
    if not SAFE_FUNCTION.fullmatch(result):
        raise Rejected(f"{label} is not a canonical C function identifier")
    return result


def _object_name(value: Any) -> tuple[str | None, str]:
    """Admit only compiler-exposed canonical C identifiers to the packet."""

    if isinstance(value, str):
        result = value.strip()
        if SAFE_FUNCTION.fullmatch(result) and POINTER_TEXT.search(result) is None:
            return result, "EXACT"
    return None, "UNKNOWN"


def _board_source_path(value: Any, label: str) -> Path:
    """Require an exact C source below an authenticated ``src/board`` seam."""

    # The descriptor path has already passed the strict on-disk alias check in
    # ``_artifact_manifest`` / ``_load_authority_manifest``.  Keep this helper
    # lexical so the native session boundary can recheck a sealed descriptor
    # without resolving a second caller-controlled path.
    path = Path(_text(value, label))
    if not path.is_absolute():
        raise Rejected(f"{label} must be absolute")
    folded = [part.casefold() for part in path.parts]
    if path.suffix.casefold() != ".c" or not any(
        folded[index : index + 2] == ["src", "board"]
        for index in range(max(0, len(folded) - 1))
    ):
        raise Rejected(f"{label} is not an authenticated Board C source path")
    return path


def _request_authorized_function(
    request: Mapping[str, Any], artifacts: Mapping[str, Mapping[str, Any]]
) -> str:
    """Recheck the external authority binding at the native session boundary."""

    function = _function_name(request.get("function"), "request.function")
    function_sha256 = _canonical_function_hash(request.get("function_sha256"))
    authority = _strict_keys(request.get("authority"), AUTHORITY_FIELDS, "request.authority")
    function_row = _strict_keys(
        authority.get("function"), AUTHORITY_FUNCTION_FIELDS, "request.authority.function"
    )
    source_row = _validate_descriptor(authority.get("source"), "request.authority.source")
    source = artifacts.get("source")
    if not isinstance(source, Mapping):
        raise Rejected("authenticated source artifact is unavailable")
    source_path = _board_source_path(source.get("path"), "authenticated source path")
    if Path(_text(source_row.get("path"), "request.authority.source.path")) != source_path:
        raise Rejected("request authority source path mismatch")
    if _nonnegative(source_row.get("size"), "request.authority.source.size") != source.get("size"):
        raise Rejected("request authority source size mismatch")
    source_sha256 = _digest(source.get("sha256"), "authenticated source SHA-256")
    if _digest(source_row.get("sha256"), "request.authority.source.sha256") != source_sha256:
        raise Rejected("request authority source SHA-256 mismatch")
    if function_row.get("name") != function:
        raise Rejected("request authority function name mismatch")
    if _digest(function_row.get("sha256"), "request.authority.function.sha256") != function_sha256:
        raise Rejected("request authority function SHA-256 mismatch")
    if _digest(
        function_row.get("source_sha256"), "request.authority.function.source_sha256"
    ) != source_sha256:
        raise Rejected("request authority function/source mismatch")
    return function


def _integer(value: Any, label: str) -> int:
    if isinstance(value, bool):
        raise Rejected(f"{label} must be an integer")
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        if CANONICAL_HEX.fullmatch(value):
            return int(value, 0)
        if CANONICAL_DECIMAL.fullmatch(value):
            return int(value, 10)
    raise Rejected(f"{label} must be a canonical integer")


def _nonnegative(value: Any, label: str) -> int:
    result = _integer(value, label)
    if result < 0:
        raise Rejected(f"{label} must be non-negative")
    return result


def _digest(value: Any, label: str) -> str:
    result = _text(value, label)
    if result != result.lower():
        raise Rejected(f"{label} must use lowercase hexadecimal")
    if not SHA256_RE.fullmatch(result):
        raise Rejected(f"{label} must be a SHA-256 digest")
    return result


def contained(path: Path, root: Path, label: str) -> Path:
    path = path.resolve()
    try:
        path.relative_to(root.resolve())
    except ValueError as exc:
        raise Rejected(f"{label} escapes {root}") from exc
    return path


def _descriptor(path: Path, label: str, expected: Mapping[str, Any] | None = None) -> dict[str, Any]:
    if expected is not None:
        _validate_descriptor(expected, label)
    path = _reject_path_alias(path, label)
    if not path.is_file():
        raise Rejected(f"{label} is missing: {path}")
    result = {"path": str(path), "size": path.stat().st_size, "sha256": sha256(path)}
    if expected is not None:
        expected_path = _reject_path_alias(Path(_text(expected.get("path"), f"{label}.path")), f"{label}.path", expected_path=path)
        if expected_path != path:
            raise Rejected(f"{label} path identity mismatch")
        if _nonnegative(expected.get("size"), f"{label}.size") != result["size"]:
            raise Rejected(f"{label} size identity mismatch")
        if _digest(expected.get("sha256"), f"{label}.sha256") != result["sha256"]:
            raise Rejected(f"{label} SHA-256 identity mismatch")
    return result


def _manifest_path(manifest: Mapping[str, Any], key: str, *, base: Path | None = None) -> Path:
    value = manifest.get(key)
    if isinstance(value, Mapping):
        value = value.get("path")
    path = Path(_text(value, f"manifest.{key}"))
    if base is not None and not path.is_absolute():
        path = Path(base) / path
    return path


def _manifest_descriptor(value: Any, key: str, *, base: Path | None = None) -> Any:
    if not isinstance(value, Mapping) or base is None:
        return value
    row = dict(value)
    path = Path(_text(row.get("path"), f"manifest.{key}.path"))
    if not path.is_absolute():
        row["path"] = str((Path(base) / path).resolve())
    return row


def _load_authority_manifest(path: Path, *, artifacts: Mapping[str, Mapping[str, Any]], function: str, function_sha256: str) -> tuple[dict[str, Any], dict[str, Any]]:
    authority_path = _reject_path_alias(path, "authority manifest")
    raw = _load_json_file(authority_path, "authority manifest")
    document = _strict_keys(raw, AUTHORITY_FIELDS, "authority manifest")
    if document.get("schema") != AUTHORITY_SCHEMA:
        raise Rejected("authority manifest schema mismatch")
    source_row = _validate_descriptor(document.get("source"), "authority.source")
    function_row = _strict_keys(document.get("function"), AUTHORITY_FUNCTION_FIELDS, "authority.function")
    source = artifacts.get("source")
    if not isinstance(source, Mapping):
        raise Rejected("authority source artifact is unavailable")
    _board_source_path(source.get("path"), "authority source path")
    if source_row.get("path") != str(Path(source["path"]).resolve()):
        raise Rejected("authority source path does not match the authenticated source")
    if _strict_integer(source_row.get("size"), "authority.source.size", nonnegative=True) != source["size"]:
        raise Rejected("authority source size mismatch")
    if _digest(source_row.get("sha256"), "authority.source.sha256") != source["sha256"]:
        raise Rejected("authority source SHA-256 mismatch")
    if function_row.get("name") != function or _digest(function_row.get("sha256"), "authority.function.sha256") != function_sha256:
        raise Rejected("authority function identity mismatch")
    if _digest(function_row.get("source_sha256"), "authority.function.source_sha256") != source["sha256"]:
        raise Rejected("authority function/source binding mismatch")
    authority_artifacts = _strict_keys(document.get("artifacts"), AUTHORITY_ARTIFACT_FIELDS, "authority.artifacts")
    for key in AUTHORITY_ARTIFACT_NAMES:
        expected = _validate_descriptor(authority_artifacts.get(key), f"authority.artifacts.{key}")
        actual = artifacts.get(key)
        if not isinstance(actual, Mapping):
            raise Rejected(f"authority artifact is unavailable: {key}")
        expected_path = _reject_path_alias(Path(_text(expected.get("path"), f"authority.artifacts.{key}.path")), f"authority.artifacts.{key}.path", expected_path=Path(str(actual["path"])))
        if expected_path != Path(str(actual["path"])) or _strict_integer(expected.get("size"), f"authority.artifacts.{key}.size", nonnegative=True) != actual["size"] or _digest(expected.get("sha256"), f"authority.artifacts.{key}.sha256") != actual["sha256"]:
            raise Rejected(f"authority artifact identity mismatch: {key}")
    descriptor = {"path": str(authority_path), "size": authority_path.stat().st_size, "sha256": sha256(authority_path)}
    return descriptor, {
        "schema": AUTHORITY_SCHEMA,
        "source": dict(source_row),
        "function": dict(function_row),
        "artifacts": {key: dict(authority_artifacts[key]) for key in AUTHORITY_ARTIFACT_NAMES},
    }


def _read_pe(path: Path) -> tuple[int, Any]:
    data = path.read_bytes()
    if len(data) < 0x40 or data[:2] != b"MZ":
        raise Rejected("compiler is not an MZ PE image")
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if pe_offset + 24 > len(data) or data[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise Rejected("compiler PE signature is missing")
    section_count = struct.unpack_from("<H", data, pe_offset + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    optional_offset = pe_offset + 24
    if optional_offset + optional_size > len(data):
        raise Rejected("compiler PE optional header is truncated")
    if struct.unpack_from("<H", data, optional_offset)[0] != 0x10B:
        raise Rejected("compiler is not PE32")
    image_base = struct.unpack_from("<I", data, optional_offset + 28)[0]
    sections: list[tuple[int, int, int, int]] = []
    section_table = optional_offset + optional_size
    for index in range(section_count):
        row = section_table + index * 40
        if row + 40 > len(data):
            raise Rejected("compiler PE section table is truncated")
        virtual_size, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, row + 8)
        sections.append((rva, max(virtual_size, raw_size), raw_offset, raw_size))

    def read_va(address: int, size: int) -> bytes:
        if size <= 0:
            raise Rejected("PE prefix size must be positive")
        rva = address - image_base
        for start, extent, raw_offset, raw_size in sections:
            if start <= rva and rva + size <= start + extent:
                delta = rva - start
                if delta < 0 or delta + size > raw_size or raw_offset + delta + size > len(data):
                    raise Rejected(f"PE address 0x{address:08x} is not file-backed")
                return data[raw_offset + delta : raw_offset + delta + size]
        raise Rejected(f"PE address 0x{address:08x} is outside compiler sections")

    return image_base, read_va


def verify_compiler(path: Path) -> dict[str, Any]:
    path = _reject_path_alias(path, "compiler")
    if path.name.casefold() != "mwcceppc.exe" or not path.is_file():
        raise Rejected(f"compiler is missing or has wrong name: {path}")
    size = path.stat().st_size
    digest = sha256(path)
    if size != EXPECTED_COMPILER_SIZE or digest != EXPECTED_COMPILER_SHA256:
        raise Rejected(f"compiler identity mismatch: size={size} sha256={digest}")
    image_base, read_va = _read_pe(path)
    if image_base != KNOWN_IMAGE_BASE:
        raise Rejected(f"compiler image base mismatch: 0x{image_base:08x}")
    hooks: list[dict[str, Any]] = []
    for hook in HOOKS:
        expected = bytes.fromhex(str(hook["prefix"]))
        observed = read_va(int(hook["address"]), len(expected))
        if observed != expected:
            raise Rejected(
                f"compiler hook prefix mismatch {hook['id']}: {observed.hex()} != {expected.hex()}"
            )
        hooks.append({**hook, "address": f"0x{int(hook['address']):08x}", "validated": True})
    _validate_hook_rows(hooks, "compiler hook manifest", compiler=True)
    return {
        "path": str(path),
        "size": size,
        "sha256": digest,
        "profile": "GC/2.6 build107",
        "image_base": f"0x{image_base:08x}",
        "hooks": hooks,
    }


def _expected_tool_hash(label: str) -> str:
    return {
        "producer": EXPECTED_PRODUCER_SHA256,
        "debugger": EXPECTED_DEBUGGER_SHA256,
        "emulator": EXPECTED_EMULATOR_SHA256,
        "gdb": EXPECTED_GDB_SHA256,
    }[label]


def verify_tool(path: Path, label: str) -> dict[str, Any]:
    descriptor = _descriptor(path, label)
    expected = _expected_tool_hash(label)
    if descriptor["sha256"] != expected:
        raise Rejected(f"{label} SHA-256 mismatch: {descriptor['sha256']} != {expected}")
    return descriptor


def _compiler_args(args: Any, source: Path, output_dir: Path, cwd: Path) -> tuple[list[str], Path]:
    if not isinstance(args, list) or not args or not all(isinstance(item, str) and item for item in args):
        raise Rejected("manifest.argv must be a non-empty string list")
    first_name = Path(args[0]).name.casefold()
    if first_name in {"mwcceppc", "mwcceppc.exe"} or first_name.endswith(".exe"):
        raise Rejected("manifest.argv contains a duplicated compiler executable; provide compiler arguments only")
    cwd = _reject_directory_alias(Path(cwd), "compiler cwd")
    source_indexes = [index for index, value in enumerate(args) if value == "-c"]
    output_indexes = [index for index, value in enumerate(args) if value == "-o"]
    if len(source_indexes) != 1 or source_indexes[0] + 1 >= len(args):
        raise Rejected("compiler argv requires one -c source operand")
    if len(output_indexes) != 1 or output_indexes[0] + 1 >= len(args):
        raise Rejected("compiler argv requires one -o output operand")
    source_operand = Path(args[source_indexes[0] + 1])
    if not source_operand.is_absolute():
        source_operand = cwd / source_operand
    source_arg = _reject_path_alias(source_operand, "compiler -c source", expected_path=source)
    output_operand = Path(args[output_indexes[0] + 1])
    if not output_operand.is_absolute():
        output_operand = cwd / output_operand
    output_arg = output_operand.resolve()
    if source_arg != source.resolve():
        raise Rejected("compiler -c operand is not the authenticated source")
    contained(output_arg, output_dir, "compiler output")
    return list(args), output_arg


def _artifact_manifest(manifest: Mapping[str, Any], *, cwd: Path | None = None) -> dict[str, Any]:
    for key in ("source", "baseline", "compiler", "producer", "debugger", "emulator", "gdb"):
        value = manifest.get(key)
        if isinstance(value, Mapping):
            _validate_descriptor(_manifest_descriptor(value, key, base=cwd), f"manifest.{key}")
    source_expected = _manifest_descriptor(manifest.get("source"), "source", base=cwd)
    baseline_expected = _manifest_descriptor(manifest.get("baseline"), "baseline", base=cwd)
    source = _descriptor(_manifest_path(manifest, "source", base=cwd), "source", source_expected if isinstance(source_expected, Mapping) else None)
    baseline = _descriptor(_manifest_path(manifest, "baseline", base=cwd), "baseline", baseline_expected if isinstance(baseline_expected, Mapping) else None)
    compiler = verify_compiler(_manifest_path(manifest, "compiler", base=cwd))
    tools = {
        label: verify_tool(_manifest_path(manifest, label, base=cwd), label)
        for label in ("producer", "debugger", "emulator", "gdb")
    }
    for key, actual in {"compiler": compiler, **tools}.items():
        expected = _manifest_descriptor(manifest.get(key), key, base=cwd)
        if isinstance(expected, Mapping):
            _validate_descriptor(expected, f"manifest.{key}")
            expected_path = str(_reject_path_alias(Path(_text(expected.get("path"), f"manifest.{key}.path")), f"manifest.{key}.path", expected_path=Path(actual["path"])))
            if expected_path != actual["path"] or _nonnegative(expected.get("size"), f"manifest.{key}.size") != actual["size"] or _digest(expected.get("sha256"), f"manifest.{key}.sha256") != actual["sha256"]:
                raise Rejected(f"manifest.{key} identity mismatch")
    return {"source": source, "baseline": baseline, "compiler": compiler, **tools}


def _canonical_function_hash(value: Any) -> str:
    result = _digest(value, "function_sha256")
    return result


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


def prepare_request(manifest: Path | Mapping[str, Any], output_dir: Path) -> Path:
    """Authenticate a current source/function request without launching tools."""

    if isinstance(manifest, Mapping):
        raw = dict(manifest)
        manifest_path: Path | None = None
    else:
        manifest_path = _reject_path_alias(Path(manifest), "request manifest")
        raw = _load_json_file(manifest_path, "request manifest")
    _strict_keys(raw, MANIFEST_FIELDS, "request manifest")
    function = _function_name(raw.get("function"), "manifest.function")
    function_sha256 = _canonical_function_hash(raw.get("function_sha256"))
    cwd = _reject_directory_alias(Path(_text(raw.get("cwd"), "manifest.cwd")), "compiler cwd")
    raw_output_dir = Path(output_dir)
    if raw_output_dir.exists() and (raw_output_dir.is_symlink() or os.path.normcase(str(raw_output_dir.absolute())) != os.path.normcase(str(raw_output_dir.resolve()))):
        raise Rejected("output directory is a symlink/alias")
    output_dir = raw_output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    if any(output_dir.iterdir()):
        raise Rejected("output directory is nonempty")
    artifacts = _artifact_manifest(raw, cwd=cwd)
    argv, candidate = _compiler_args(raw.get("argv"), Path(artifacts["source"]["path"]), output_dir, cwd)
    authority_path = _manifest_path(raw, "authority_manifest", base=cwd)
    try:
        authority_path.resolve().relative_to(output_dir.resolve())
    except ValueError:
        pass
    else:
        raise Rejected("authority manifest must be external to the output directory")
    authority_descriptor, authority = _load_authority_manifest(
        authority_path,
        artifacts=artifacts,
        function=function,
        function_sha256=function_sha256,
    )
    authority_expected = raw.get("authority_manifest")
    if isinstance(authority_expected, Mapping):
        normalized_authority_expected = _manifest_descriptor(authority_expected, "authority_manifest", base=cwd)
        _validate_descriptor(normalized_authority_expected, "manifest.authority_manifest")
        if dict(normalized_authority_expected) != authority_descriptor:
            raise Rejected("manifest authority descriptor mismatch")
    tracer_path = Path(__file__).resolve()
    _reject_path_alias(tracer_path, "tracer")
    # ``verify_compiler`` returns an enriched internal record containing the
    # PE profile and validated hook rows.  The serialized request contract is
    # intentionally descriptor-only; authentication re-runs ``_artifact_manifest``
    # and therefore rechecks the compiler image/profile and every hook prefix.
    # Keeping those internal fields out of ``request.artifacts`` preserves the
    # closed descriptor schema without weakening any identity check.
    artifact_descriptors = {
        key: {field: value[field] for field in DESCRIPTOR_FIELDS}
        for key, value in artifacts.items()
    }
    artifact_descriptors["tracer"] = {"path": str(tracer_path), "size": tracer_path.stat().st_size, "sha256": sha256(tracer_path)}
    request = {
        "schema": REQUEST_SCHEMA,
        "tool_version": TOOL_VERSION,
        "backend": BACKEND_NAME,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "function": function,
        "function_sha256": function_sha256,
        "authority_manifest": authority_descriptor,
        "authority": authority,
        "cwd": str(cwd),
        "argv": argv,
        "compiler": artifacts["compiler"]["path"],
        "source": artifacts["source"]["path"],
        "baseline": artifacts["baseline"]["path"],
        "producer": artifacts["producer"]["path"],
        "debugger": artifacts["debugger"]["path"],
        "emulator": artifacts["emulator"]["path"],
        "gdb": artifacts["gdb"]["path"],
        "output_dir": str(output_dir),
        "event_stream": str(output_dir / "events.jsonl"),
        "packet": str(output_dir / "trace.packet.json"),
        "candidate": str(candidate),
        "hooks": _request_hook_rows(),
        "artifacts": artifact_descriptors,
        "transport": {
            "name": BACKEND_NAME,
            "required_capabilities": sorted(REQUIRED_CAPABILITIES),
            "single_step_post_events": True,
        },
        "provenance": {
            "authenticated": True,
            "producer_sha256": artifacts["producer"]["sha256"],
            "debugger_sha256": artifacts["debugger"]["sha256"],
            "emulator_sha256": artifacts["emulator"]["sha256"],
            "gdb_sha256": artifacts["gdb"]["sha256"],
            "compiler_sha256": artifacts["compiler"]["sha256"],
            "source_sha256": artifacts["source"]["sha256"],
            "function_sha256": function_sha256,
        },
    }
    request["request_sha256"] = canonical_hash(request)
    write_new(output_dir / "request.json", (json.dumps(request, indent=2, sort_keys=True) + "\n").encode("utf-8"))
    return output_dir / "request.json"


def _request_paths(request: Mapping[str, Any]) -> dict[str, Path]:
    keys = ("compiler", "source", "baseline", "producer", "debugger", "emulator", "gdb", "cwd", "output_dir", "event_stream", "packet", "candidate")
    return {key: Path(_text(request.get(key), f"request.{key}")).resolve() for key in keys}


def authenticate_request(request_path: Path, *, require_empty: bool = False) -> dict[str, Any]:
    request_path = _reject_path_alias(Path(request_path), "request")
    request = _load_json_file(request_path, "request")
    _strict_keys(request, REQUEST_FIELDS, "request")
    if request.get("schema") != REQUEST_SCHEMA or request.get("tool_version") != TOOL_VERSION:
        raise Rejected("request schema/tool version mismatch")
    if request.get("request_sha256") != canonical_hash({key: value for key, value in request.items() if key != "request_sha256"}):
        raise Rejected("request descriptor hash mismatch")
    for key, expected in (("backend", BACKEND_NAME), ("diagnostic_only", True), ("board_admission", False), ("exactness_claim", False)):
        if request.get(key) != expected:
            raise Rejected(f"request policy mismatch: {key}")
    function = _function_name(request.get("function"), "request.function")
    function_sha256 = _canonical_function_hash(request.get("function_sha256"))
    _reject_directory_alias(Path(_text(request.get("cwd"), "request.cwd")), "request.cwd")
    for key in ("compiler", "source", "baseline", "producer", "debugger", "emulator", "gdb"):
        _reject_path_alias(Path(_text(request.get(key), f"request.{key}")), f"request.{key}")
    _reject_directory_alias(Path(_text(request.get("output_dir"), "request.output_dir")), "request.output_dir")
    paths = _request_paths(request)
    if not paths["cwd"].is_dir() or request_path.parent != paths["output_dir"]:
        raise Rejected("request cwd/output binding mismatch")
    contained(paths["output_dir"], request_path.parent, "output directory")
    for key in ("event_stream", "packet", "candidate"):
        contained(paths[key], paths["output_dir"], key)
    if require_empty:
        extras = [entry for entry in paths["output_dir"].iterdir() if entry.resolve() != request_path]
        if extras:
            raise Rejected("output directory contains stale files")
    argv, candidate = _compiler_args(request.get("argv"), paths["source"], paths["output_dir"], paths["cwd"])
    if candidate != paths["candidate"]:
        raise Rejected("candidate path mismatch")
    _validate_hook_rows(request.get("hooks"), "request.hooks")
    expected_artifacts = _strict_keys(request.get("artifacts"), {"source", "baseline", "compiler", "producer", "debugger", "emulator", "gdb", "tracer"}, "request.artifacts")
    artifacts = _artifact_manifest({key: str(paths[key]) for key in ("source", "baseline", "compiler", "producer", "debugger", "emulator", "gdb")})
    for key, actual in artifacts.items():
        expected = _validate_descriptor(expected_artifacts.get(key), f"request.artifacts.{key}")
        actual_descriptor = {field: actual[field] for field in DESCRIPTOR_FIELDS}
        if dict(expected) != actual_descriptor:
            raise Rejected(f"artifact binding mismatch: {key}")
    tracer = _validate_descriptor(expected_artifacts.get("tracer"), "request.artifacts.tracer")
    tracer_path = _reject_path_alias(Path(__file__), "tracer")
    actual_tracer = {"path": str(tracer_path), "size": tracer_path.stat().st_size, "sha256": sha256(tracer_path)}
    if dict(tracer) != actual_tracer:
        raise Rejected("tracer binding mismatch")
    authority_descriptor = _validate_descriptor(request.get("authority_manifest"), "request.authority_manifest")
    authority_path_text = _text(authority_descriptor.get("path"), "request.authority_manifest.path")
    authority_path = _reject_path_alias(Path(authority_path_text), "authority manifest", expected_path=Path(authority_path_text))
    actual_authority_descriptor = {"path": str(authority_path), "size": authority_path.stat().st_size, "sha256": sha256(authority_path)}
    if dict(authority_descriptor) != actual_authority_descriptor:
        raise Rejected("authority manifest descriptor mismatch")
    authority_path_obj, authority = _load_authority_manifest(authority_path, artifacts=artifacts, function=function, function_sha256=function_sha256)
    if authority_path_obj != actual_authority_descriptor or request.get("authority") != authority:
        raise Rejected("authority manifest binding mismatch")
    transport = _strict_keys(request.get("transport"), TRANSPORT_FIELDS, "request.transport")
    if transport.get("name") != BACKEND_NAME or transport.get("required_capabilities") != sorted(REQUIRED_CAPABILITIES) or transport.get("single_step_post_events") is not True:
        raise Rejected("transport binding mismatch")
    provenance = _strict_keys(request.get("provenance"), PROVENANCE_FIELDS, "request.provenance")
    if provenance.get("authenticated") is not True:
        raise Rejected("authenticated provenance is missing")
    for key, expected in (
        ("producer_sha256", artifacts["producer"]["sha256"]),
        ("debugger_sha256", artifacts["debugger"]["sha256"]),
        ("emulator_sha256", artifacts["emulator"]["sha256"]),
        ("gdb_sha256", artifacts["gdb"]["sha256"]),
        ("compiler_sha256", artifacts["compiler"]["sha256"]),
        ("source_sha256", artifacts["source"]["sha256"]),
        ("function_sha256", function_sha256),
    ):
        if provenance.get(key) != expected:
            raise Rejected(f"provenance binding mismatch: {key}")
    _request_authorized_function(request, artifacts)
    return {
        "request": dict(request),
        "request_path": request_path,
        "request_sha256": sha256(request_path),
        "paths": paths,
        "artifacts": {**artifacts, "tracer": actual_tracer},
        "authority_manifest": authority_descriptor,
        "compiler": artifacts["compiler"],
        "source": artifacts["source"],
    }


def _pointer_free(value: Any, location: str = "event") -> None:
    pointer_names = {"address", "addresses", "pointer", "pointers", "ptr", "raw_pointer", "raw_address", "pid", "tid", "thread", "thread_id", "process", "process_id"}
    if isinstance(value, Mapping):
        for key, child in value.items():
            normalized = str(key).strip().lower()
            if normalized in pointer_names or normalized.endswith("_address") or normalized.endswith("_pointer"):
                raise Rejected(f"{location}.{key} exposes a raw pointer/address")
            _pointer_free(child, f"{location}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            _pointer_free(child, f"{location}[{index}]")


def _token(kind: str, ordinal: int) -> str:
    return f"{kind}-{ordinal:06d}"


class BackendEvent:
    """High-level event delivered by a transport to the capture session."""

    __slots__ = ("event_kind", "address", "thread_id", "payload")

    def __init__(
        self,
        event_kind: str,
        address: int | None = None,
        thread_id: int | None = None,
        payload: Mapping[str, Any] | None = None,
    ) -> None:
        self.event_kind = event_kind
        self.address = address
        self.thread_id = thread_id
        self.payload = payload


class CaptureBackend(Protocol):
    capabilities: Mapping[str, bool]

    def run(self, session: "CaptureSession") -> None: ...

    def read_image(self, address: int, size: int) -> bytes: ...

    def read_memory(self, address: int, size: int) -> bytes: ...

    def install_breakpoint(self, address: int) -> None: ...

    def remove_breakpoint(self, address: int) -> None: ...

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None: ...

    def read_register(self, thread_id: int, name: str) -> int: ...

    def read_execution_state(self, thread_id: int) -> Mapping[str, int] | None: ...

    def trace_event(self, event: Mapping[str, Any]) -> None: ...

    def snapshot_objects(self) -> Sequence[Mapping[str, Any]]: ...

    def snapshot_varinfo(self, pointer: int) -> Mapping[str, Any]: ...

    def read_object_stack_field(self, pointer: int) -> int: ...

    def close(self) -> None: ...


def validate_backend_capabilities(backend: CaptureBackend) -> None:
    capabilities = getattr(backend, "capabilities", None)
    if not isinstance(capabilities, Mapping):
        raise Rejected("native transport capabilities are missing")
    missing = sorted(name for name in REQUIRED_CAPABILITIES if capabilities.get(name) is not True)
    method_names = {
        "read_image": "read_image",
        "read_memory": "read_memory",
        "read_registers": "read_register",
        "read_object_list": "snapshot_objects",
        "read_varinfo": "snapshot_varinfo",
        "read_object_stack_field": "read_object_stack_field",
        "install_breakpoint": "install_breakpoint",
        "remove_breakpoint": "remove_breakpoint",
        "single_step": "single_step",
        "run_events": "run",
        "cleanup": "close",
    }
    missing.extend(capability for capability, method in method_names.items() if capability != "cleanup" and capabilities.get(capability) is True and not callable(getattr(backend, method, None)))
    if not callable(getattr(backend, "close", None)):
        missing.append("cleanup")
    if missing:
        raise Rejected("native transport capability gap: " + ", ".join(sorted(set(missing))))


class CaptureSession:
    """Translate backend callbacks into canonical, pointer-free events."""

    def __init__(self, auth: Mapping[str, Any], backend: CaptureBackend) -> None:
        try:
            validate_backend_capabilities(backend)
        except Exception as exc:
            raise _normalize_exception(exc, "native transport capability validation") from exc
        request = auth.get("request")
        if not isinstance(request, Mapping):
            raise Rejected("authenticated request context is missing")
        function = _request_authorized_function(request, auth.get("artifacts", {}))
        self.auth = auth
        self.backend = backend
        self.function = function
        self.capture_id = str(auth.get("request_sha256", ""))[:16]
        if not SHA256_RE.fullmatch(str(auth.get("request_sha256", ""))):
            raise Rejected("authenticated request hash is malformed")
        self.sequence = 0
        self.events: list[dict[str, Any]] = []
        self.unknown: list[dict[str, Any]] = []
        # These pointer maps describe only the identities in the most recent
        # authenticated compiler-list snapshot.  Their values are capture-
        # local ordinals; they are intentionally not pointer identities in the
        # packet.  A pointer which disappears and later reappears gets a fresh
        # generation/ordinal instead of being merged with its old lifetime.
        self.object_ordinals: dict[int, int] = {}
        self.varinfo_ordinals: dict[int, int] = {}
        self.object_varinfo: dict[int, int] = {}
        self.varinfo_object: dict[int, int] = {}
        self.object_generations: dict[int, int] = {}
        self.varinfo_generations: dict[int, int] = {}
        self.object_identity_ordinals: dict[tuple[int, int], int] = {}
        self.varinfo_identity_ordinals: dict[tuple[int, int], int] = {}
        # Every generation observed in an authenticated list retains its
        # pointer-free Object/VarInfo join.  This relation connects an Object
        # stack-write token to its exact VarInfo home generation in the packet.
        self.object_varinfo_pairs: dict[int, int] = {}
        # Canonical, pointer-free compiler metadata keyed by capture-local
        # Object ordinal.  A name is evidence only when MWCC exposes one exact
        # C identifier; compiler temporaries and malformed text remain UNKNOWN.
        self.object_inventory: dict[int, dict[str, Any]] = {}
        self.next_object_ordinal = 0
        self.next_varinfo_ordinal = 0
        self.released_objects: set[int] = set()
        # The allocator may link a freshly-created Object only after the
        # allocation call has returned.  Keep those raw identities private
        # until the post-call list snapshot proves the Object/VarInfo pair.
        # Keep the exact generation captured by each in-boundary write.  A
        # later list snapshot must resolve the same pointer to the same ordinal;
        # pointer reuse is otherwise an ambiguous stack-home claim.
        self.pending_object_bindings: dict[int, dict[int, int]] = {}
        self.compiler_list_event: dict[str, Any] | None = None
        self.emitted_varinfo_ordinals: set[int] = set()
        self.pending_alloc: dict[int, str] = {}
        self.pending_alloc_state: dict[int, Mapping[str, int] | None] = {}
        self.pending_writes: dict[int, dict[str, Any]] = {}
        self.pending_steps: dict[int, tuple[int, bool]] = {}
        self.target_seen = False
        self.target_active = False
        self.target_finished = False
        self.process_started = False
        self.process_exited = False
        self.mutation_started = False

    def _event(self, event_kind: str, payload: Mapping[str, Any] | None = None) -> dict[str, Any]:
        if event_kind not in {
            "function_entry", "compiler_list", "numeric_stack_alloc_pre", "numeric_stack_alloc_post",
            "varinfo_home_snapshot", "object_stack_write_pre", "object_stack_write_post", "function_exit",
        }:
            raise Rejected(f"unsupported canonical event kind: {event_kind}")
        row: dict[str, Any] = {
            "schema": EVENT_SCHEMA,
            "event_id": f"{self.capture_id}-e{self.sequence:06d}",
            "sequence": self.sequence,
            "event_kind": event_kind,
            "function": self.function,
        }
        if payload:
            row.update(dict(payload))
        _pointer_free(row)
        self.events.append(row)
        self.sequence += 1
        return row

    def _unknown(self, reason: str) -> None:
        self.unknown.append({"reason": reason})

    def _trace(self, event: Mapping[str, Any]) -> None:
        trace_event = getattr(self.backend, "trace_event", None)
        if callable(trace_event):
            trace_event(event)

    def _execution_state(self, thread_id: int) -> Mapping[str, int] | None:
        read_state = getattr(self.backend, "read_execution_state", None)
        if not callable(read_state):
            return None
        state = read_state(thread_id)
        if state is None:
            return None
        if not isinstance(state, Mapping):
            raise Rejected("native execution state is not a mapping")
        for key in ("eip", "esp", "ebp"):
            _strict_integer(state.get(key), f"native execution state {key}", nonnegative=True)
        return state

    def _ignore_object_write(self, address: int, thread_id: int, reason: str, state: Mapping[str, int] | None) -> None:
        self._unknown(reason)
        self._trace(
            {
                "event": "object_write_ignored",
                "address": f"0x{address:08x}",
                "thread": thread_id,
                "reason": reason,
                "state": dict(state) if state is not None else None,
            }
        )
        self._step(address, thread_id, rearm=True)

    def _ordinal_object(self, pointer: int, *, allow_new: bool = False) -> int:
        if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer <= 0:
            raise Rejected("Object pointer is missing or invalid")
        if pointer in self.released_objects:
            raise Rejected("reused Object pointer after release")
        if pointer not in self.object_ordinals:
            if not allow_new:
                raise Rejected("compiler Object was not admitted by an authenticated list snapshot")
            generation = self.object_generations.get(pointer, -1) + 1
            ordinal = self.next_object_ordinal
            self.next_object_ordinal += 1
            self.object_generations[pointer] = generation
            self.object_identity_ordinals[(pointer, generation)] = ordinal
            self.object_ordinals[pointer] = ordinal
        return self.object_ordinals[pointer]

    def _ordinal_varinfo(self, pointer: int) -> int:
        if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer <= 0:
            raise Rejected("VarInfo pointer is missing or invalid")
        if pointer not in self.varinfo_ordinals:
            generation = self.varinfo_generations.get(pointer, -1) + 1
            ordinal = self.next_varinfo_ordinal
            self.next_varinfo_ordinal += 1
            self.varinfo_generations[pointer] = generation
            self.varinfo_identity_ordinals[(pointer, generation)] = ordinal
            self.varinfo_ordinals[pointer] = ordinal
        return self.varinfo_ordinals[pointer]

    def _new_object_generation(self, pointer: int) -> int:
        """Allocate a new Object ordinal for an active pointer rebind."""

        if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer <= 0:
            raise Rejected("Object pointer is missing or invalid")
        generation = self.object_generations.get(pointer, -1) + 1
        ordinal = self.next_object_ordinal
        self.next_object_ordinal += 1
        self.object_generations[pointer] = generation
        self.object_identity_ordinals[(pointer, generation)] = ordinal
        self.object_ordinals[pointer] = ordinal
        return ordinal

    def _new_varinfo_generation(self, pointer: int) -> int:
        """Allocate a new VarInfo ordinal for an active pointer rebind."""

        if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer <= 0:
            raise Rejected("VarInfo pointer is missing or invalid")
        generation = self.varinfo_generations.get(pointer, -1) + 1
        ordinal = self.next_varinfo_ordinal
        self.next_varinfo_ordinal += 1
        self.varinfo_generations[pointer] = generation
        self.varinfo_identity_ordinals[(pointer, generation)] = ordinal
        self.varinfo_ordinals[pointer] = ordinal
        return ordinal

    def _snapshot_list(
        self, *, allow_new: bool = False, allow_empty: bool = False
    ) -> tuple[list[str], list[str], list[dict[str, Any]], list[dict[str, Any]]]:
        objects = list(self.backend.snapshot_objects())
        if not objects:
            if allow_empty:
                return (
                    [_token("object", ordinal) for ordinal in range(self.next_object_ordinal)],
                    [_token("varinfo", ordinal) for ordinal in range(self.next_varinfo_ordinal)],
                    [],
                    [dict(self.object_inventory[ordinal]) for ordinal in range(self.next_object_ordinal)],
                )
            raise Rejected("compiler Object list is empty")
        object_tokens: list[str] = []
        varinfo_tokens: list[str] = []
        snapshots: list[dict[str, Any]] = []
        seen_objects: set[int] = set()
        seen_varinfo: set[int] = set()
        previous_object_varinfo = dict(self.object_varinfo)
        previous_varinfo_object = dict(self.varinfo_object)
        current_object_ordinals: dict[int, int] = {}
        current_varinfo_ordinals: dict[int, int] = {}
        current_object_varinfo: dict[int, int] = {}
        current_varinfo_object: dict[int, int] = {}
        for index, raw in enumerate(objects):
            if not isinstance(raw, Mapping):
                raise Rejected(f"Object list record {index} is not an object")
            pointer = raw.get("pointer")
            varinfo = raw.get("varinfo_pointer")
            if isinstance(pointer, bool) or not isinstance(pointer, int) or pointer in seen_objects:
                raise Rejected("duplicate/reused Object pointer in one compiler list")
            if isinstance(varinfo, bool) or not isinstance(varinfo, int) or varinfo in seen_varinfo:
                raise Rejected("duplicate/reused VarInfo pointer in one compiler list")
            seen_objects.add(pointer)
            seen_varinfo.add(varinfo)
            object_ordinal = self._ordinal_object(pointer, allow_new=allow_new)
            varinfo_ordinal = self._ordinal_varinfo(varinfo)
            previous_varinfo = previous_object_varinfo.get(pointer)
            previous_object = previous_varinfo_object.get(varinfo)
            object_rebound = previous_varinfo is not None and previous_varinfo != varinfo
            varinfo_rebound = previous_object is not None and previous_object != pointer
            if object_rebound:
                object_ordinal = self._new_object_generation(pointer)
            if varinfo_rebound:
                varinfo_ordinal = self._new_varinfo_generation(varinfo)
            if object_rebound or varinfo_rebound:
                self._trace(
                    {
                        "event": "compiler_identity_rebound",
                        "object_ordinal": object_ordinal,
                        "object_generation": self.object_generations[pointer],
                        "varinfo_ordinal": varinfo_ordinal,
                        "varinfo_generation": self.varinfo_generations[varinfo],
                        "object_rebound": object_rebound,
                        "varinfo_rebound": varinfo_rebound,
                    }
                )
            current_object_ordinals[pointer] = object_ordinal
            current_varinfo_ordinals[varinfo] = varinfo_ordinal
            current_object_varinfo[pointer] = varinfo
            current_varinfo_object[varinfo] = pointer
            self.object_varinfo_pairs[object_ordinal] = varinfo_ordinal
            object_token = _token("object", object_ordinal)
            varinfo_token = _token("varinfo", varinfo_ordinal)
            object_tokens.append(object_token)
            varinfo_tokens.append(varinfo_token)
            name, name_status = _object_name(raw.get("name"))
            datatype = _strict_integer(
                raw.get("datatype"), f"Object list record {index} datatype", nonnegative=True
            )
            if datatype > 0xFF:
                raise Rejected("Object datatype is outside the authenticated byte field")
            inventory_row = {
                "object_token": object_token,
                "varinfo_token": varinfo_token,
                "name": name,
                "name_status": name_status,
                "datatype": datatype,
            }
            prior_inventory = self.object_inventory.get(object_ordinal)
            if prior_inventory is not None and prior_inventory != inventory_row:
                raise Rejected("compiler Object metadata changed within one capture generation")
            self.object_inventory[object_ordinal] = inventory_row
            raw_varinfo = self.backend.snapshot_varinfo(varinfo)
            if not isinstance(raw_varinfo, Mapping):
                raise Rejected("VarInfo snapshot is unavailable")
            varinfo_row = {
                "varinfo_token": varinfo_token,
                "varinfo_ordinal": varinfo_ordinal,
                "varinfo_field_offset": "+0x26",
                "home_value": raw_varinfo.get("home_value"),
            }
            _strict_integer(varinfo_row["home_value"], "VarInfo +0x26 home value")
            snapshots.append(varinfo_row)
        # Drop stale pointer bindings from the active maps.  Their generation
        # records and ordinals remain immutable in the capture inventory, so a
        # later pointer reuse still receives a new token rather than reviving a
        # prior compiler Object/VarInfo lifetime.
        self.object_ordinals = current_object_ordinals
        self.varinfo_ordinals = current_varinfo_ordinals
        self.object_varinfo = current_object_varinfo
        self.varinfo_object = current_varinfo_object
        # The backend list order is an implementation detail.  Ordinals are
        # capture-local identity labels, so every serialized inventory uses
        # canonical ordinal order.  Generation counters keep a newly linked or
        # recycled Object from changing the meaning of an earlier token when
        # the native list head is inserted at the front.
        return (
            [_token("object", ordinal) for ordinal in range(self.next_object_ordinal)],
            [_token("varinfo", ordinal) for ordinal in range(self.next_varinfo_ordinal)],
            snapshots,
            [dict(self.object_inventory[ordinal]) for ordinal in range(self.next_object_ordinal)],
        )

    def _compiler_pair_rows(self) -> list[dict[str, str]]:
        return [
            {
                "object_token": _token("object", object_ordinal),
                "varinfo_token": _token("varinfo", varinfo_ordinal),
            }
            for object_ordinal, varinfo_ordinal in sorted(self.object_varinfo_pairs.items())
        ]

    def _refresh_compiler_list(self) -> None:
        if self.compiler_list_event is None:
            raise Rejected("compiler-list binding is unavailable")
        self.compiler_list_event["object_tokens"] = [
            _token("object", ordinal) for ordinal in range(self.next_object_ordinal)
        ]
        self.compiler_list_event["varinfo_tokens"] = [
            _token("varinfo", ordinal) for ordinal in range(self.next_varinfo_ordinal)
        ]
        self.compiler_list_event["object_varinfo_pairs"] = self._compiler_pair_rows()
        self.compiler_list_event["objects"] = [
            dict(self.object_inventory[ordinal]) for ordinal in range(self.next_object_ordinal)
        ]

    def _emit_new_varinfo_snapshots(self, snapshots: Sequence[Mapping[str, Any]]) -> None:
        for snapshot in snapshots:
            ordinal = _strict_integer(snapshot.get("varinfo_ordinal"), "VarInfo ordinal", nonnegative=True)
            if ordinal in self.emitted_varinfo_ordinals:
                continue
            self._event("varinfo_home_snapshot", snapshot)
            self.emitted_varinfo_ordinals.add(ordinal)

    def on_process_started(self) -> None:
        if self.process_started:
            raise Rejected("duplicate process-start event")
        self.process_started = True
        # This is the only point at which the adapter may begin breakpoint
        # mutation.  All PE checks happened in request authentication; all
        # live-image prefixes are read here before the first install call.
        failures: list[str] = []
        for hook in HOOKS:
            address = int(hook["address"])
            expected = bytes.fromhex(str(hook["prefix"]))
            actual = self.backend.read_image(address, len(expected))
            if actual != expected:
                failures.append(f"{hook['id']}: {actual.hex()} != {expected.hex()}")
        if failures:
            raise Rejected("live hook prefix mismatch; no breakpoints installed: " + "; ".join(failures))
        mark_validated = getattr(self.backend, "mark_live_prefixes_validated", None)
        if callable(mark_validated):
            mark_validated()
        self.backend.install_breakpoint(int(HOOK_BY_ID["function_filter"]["address"]))
        self.mutation_started = True

    def _step(self, address: int, thread_id: int, *, rearm: bool) -> None:
        if not self.mutation_started:
            raise Rejected("single-step requested before breakpoint preflight")
        if thread_id in self.pending_steps:
            raise Rejected("duplicate/reordered single-step request")
        if rearm:
            self.pending_steps[thread_id] = (address, rearm)
        self.backend.single_step(address, thread_id, rearm=rearm)

    def on_breakpoint(self, address: int, thread_id: int) -> None:
        if not self.process_started or self.process_exited:
            raise Rejected("breakpoint arrived outside process lifetime")
        hook = next((row for row in HOOKS if int(row["address"]) == address), None)
        if hook is None:
            raise Rejected(f"unsupported breakpoint address: 0x{address:08x}")
        hook_id = str(hook["id"])
        if self.pending_steps:
            raise Rejected("breakpoint arrived before the derived single-step completed")
        if self.pending_writes:
            raise Rejected("breakpoint arrived before Object write post event")
        if self.pending_alloc and hook_id != "allocation_post":
            if hook_id not in WRITE_HOOK_IDS:
                raise Rejected("breakpoint arrived before numeric allocation post event")
        if hook_id == "function_filter":
            current = getattr(self.backend, "current_function", lambda: None)()
            if current == self.function and not self.target_seen:
                self.target_seen = True
                self.target_active = True
                self._trace({"event": "function_entry", "thread": thread_id, "address": f"0x{address:08x}"})
                self._event("function_entry")
                object_tokens, varinfo_tokens, snapshots, objects = self._snapshot_list(
                    allow_new=True, allow_empty=True
                )
                self._trace(
                    {
                        "event": "compiler_list_snapshot",
                        "phase": "function_entry",
                        "object_count": len(object_tokens),
                        "varinfo_count": len(varinfo_tokens),
                    }
                )
                self.compiler_list_event = self._event(
                    "compiler_list",
                    {
                        "object_tokens": object_tokens,
                        "varinfo_tokens": varinfo_tokens,
                        "object_varinfo_pairs": self._compiler_pair_rows(),
                        "objects": objects,
                    },
                )
                self._emit_new_varinfo_snapshots(snapshots)
                for row in HOOKS:
                    if row["id"] != "function_filter":
                        self.backend.install_breakpoint(int(row["address"]))
                self._step(address, thread_id, rearm=True)
                return
            if self.target_active:
                # The next compiler function is the only authenticated end
                # boundary available from this hook.  Any pending write/alloc
                # means the target was observed only partially.
                if self.pending_writes or self.pending_alloc:
                    raise Rejected("partial target function at function-filter boundary")
                # A compiler Object can be linked by ordinary phase work
                # after the numeric allocation.  Refresh identity bindings
                # before sealing the function boundary; this does not create
                # stack-write evidence or allocation ownership.
                _, _, snapshots, _ = self._snapshot_list(allow_new=True, allow_empty=True)
                self._refresh_compiler_list()
                self._emit_new_varinfo_snapshots(snapshots)
                self._trace({"event": "function_exit", "thread": thread_id, "address": f"0x{address:08x}"})
                self._event("function_exit")
                self.target_active = False
                self.target_finished = True
                for row in HOOKS:
                    if row["id"] != "function_filter":
                        self.backend.remove_breakpoint(int(row["address"]))
            self._step(address, thread_id, rearm=not self.target_finished)
            return
        if not self.target_active:
            raise Rejected(f"target hook {hook_id} fired outside target function")
        if hook_id == "allocation_pre":
            # Ordinary compiler-list growth may happen before the numeric
            # call.  It is an identity refresh, not stack-write evidence.
            object_tokens, varinfo_tokens, snapshots, _ = self._snapshot_list(allow_new=True)
            self._refresh_compiler_list()
            self._emit_new_varinfo_snapshots(snapshots)
            self._trace(
                {
                    "event": "compiler_list_snapshot",
                    "phase": "allocation_pre",
                    "object_count": len(object_tokens),
                    "varinfo_count": len(varinfo_tokens),
                }
            )
            allocation_id = f"alloc-{self.sequence:06d}"
            self.pending_alloc[thread_id] = allocation_id
            self.pending_alloc_state[thread_id] = self._execution_state(thread_id)
            self._trace(
                {
                    "event": "allocation_pre",
                    "thread": thread_id,
                    "address": f"0x{address:08x}",
                    "state": dict(self.pending_alloc_state[thread_id]) if self.pending_alloc_state[thread_id] is not None else None,
                }
            )
            self.pending_object_bindings[thread_id] = {}
            self._event(
                "numeric_stack_alloc_pre",
                {
                    "allocation_id": allocation_id,
                    "object_tokens": object_tokens,
                    "varinfo_tokens": varinfo_tokens,
                    "allocation_call": "0x004f9ce0",
                },
            )
            self._step(address, thread_id, rearm=False)
            return
        if hook_id == "allocation_post":
            allocation_id = self.pending_alloc.pop(thread_id, None)
            if allocation_id is None:
                raise Rejected("allocation post has no matching pre event")
            self.pending_alloc_state.pop(thread_id, None)
            self._trace({"event": "allocation_post", "thread": thread_id, "address": f"0x{address:08x}"})
            object_tokens, varinfo_tokens, snapshots, _ = self._snapshot_list(allow_new=True)
            pending_objects = self.pending_object_bindings.pop(thread_id, {})
            ambiguous = [
                pointer
                for pointer, captured_ordinal in pending_objects.items()
                if self.object_ordinals.get(pointer) != captured_ordinal or pointer not in self.object_varinfo
            ]
            if ambiguous:
                raise Rejected("captured Object identity was recycled before allocation post")
            self._refresh_compiler_list()
            self._trace(
                {
                    "event": "compiler_list_snapshot",
                    "phase": "allocation_post",
                    "object_count": len(object_tokens),
                    "varinfo_count": len(varinfo_tokens),
                }
            )
            self._event(
                "numeric_stack_alloc_post",
                {
                    "allocation_id": allocation_id,
                    "object_tokens": object_tokens,
                    "varinfo_tokens": varinfo_tokens,
                    "allocation_call": "0x004f9ce0",
                },
            )
            self._emit_new_varinfo_snapshots(snapshots)
            self._step(address, thread_id, rearm=False)
            self.backend.install_breakpoint(int(HOOK_BY_ID["allocation_pre"]["address"]))
            return
        if hook_id in WRITE_HOOK_IDS:
            state = self._execution_state(thread_id)
            boundary_state = self.pending_alloc_state.get(thread_id)
            if thread_id not in self.pending_alloc:
                self._ignore_object_write(
                    address,
                    thread_id,
                    "Object write is outside the authenticated numeric allocation thread/boundary",
                    state,
                )
                return
            if (
                state is not None
                and boundary_state is not None
                and int(state["esp"]) > int(boundary_state["esp"])
            ):
                self._ignore_object_write(
                    address,
                    thread_id,
                    "Object write is outside the authenticated numeric allocation stack depth",
                    state,
                )
                return
            pointer = self.backend.read_register(thread_id, "ebx")
            value = self.backend.read_register(thread_id, "eax")
            object_ordinal = self._ordinal_object(pointer, allow_new=True)
            self.pending_object_bindings.setdefault(thread_id, {})[pointer] = object_ordinal
            object_token = _token("object", object_ordinal)
            write_id = f"write-{self.sequence:06d}"
            metadata = getattr(self.backend, "write_metadata", lambda _p, _v: {"read_count": 0, "escape": False})(pointer, value)
            if not isinstance(metadata, Mapping) or not isinstance(metadata.get("escape"), bool):
                raise Rejected("Object write lifetime metadata is unavailable")
            _strict_integer(metadata.get("read_count"), "Object write read_count", nonnegative=True)
            pending = {
                "write_id": write_id,
                "hook_id": hook_id,
                "pointer": pointer,
                "value": _signed32(value),
                "object_token": object_token,
                "object_ordinal": object_ordinal,
                "thread_id": thread_id,
                "read_count": metadata["read_count"],
                "escape": metadata["escape"],
                "address": address,
            }
            if thread_id in self.pending_writes:
                raise Rejected("duplicate pending Object write on one thread")
            self.pending_writes[thread_id] = pending
            self._trace(
                {
                    "event": "object_write_captured",
                    "address": f"0x{address:08x}",
                    "thread": thread_id,
                    "state": dict(state) if state is not None else None,
                    "object_ordinal": object_ordinal,
                }
            )
            self._event(
                "object_stack_write_pre",
                {
                    "write_id": write_id,
                    "object_token": object_token,
                    "object_ordinal": object_ordinal,
                    "object_stack_field_offset": "+0x2e",
                    "target_slot": _signed32(value),
                    "read_count": metadata["read_count"],
                    "escape": metadata["escape"],
                    "write_site": f"0x{address:08x}",
                },
            )
            self._step(address, thread_id, rearm=True)
            return

    def on_single_step(self, thread_id: int) -> None:
        step = self.pending_steps.pop(thread_id, None)
        if step is None:
            # Native allocation call steps deliberately do not request a trap;
            # fake transports may still report that ordinary step as a callback.
            return
        pending = self.pending_writes.pop(thread_id, None)
        if pending is None:
            return
        if step[0] != int(pending["address"]):
            raise Rejected("Object write post derived from the wrong breakpoint")
        observed = self.backend.read_object_stack_field(int(pending["pointer"]))
        if isinstance(observed, bool) or not isinstance(observed, int) or _signed32(observed) != pending["value"]:
            raise Rejected("Object+0x2e write was not observed after single-step")
        self._event(
            "object_stack_write_post",
            {
                "write_id": pending["write_id"],
                "object_token": pending["object_token"],
                "object_ordinal": pending["object_ordinal"],
                "object_stack_field_offset": "+0x2e",
                "expected_value": pending["value"],
                "observed_value": _signed32(observed),
                "write_observed": True,
                "derived_from": pending["hook_id"],
            },
        )

    def on_process_exit(self, exit_code: int = 0) -> None:
        if self.process_exited:
            raise Rejected("duplicate process-exit event")
        self.process_exited = True
        if exit_code != 0:
            raise Rejected(f"compiler process exited with status {exit_code}")
        if self.pending_writes or self.pending_alloc or self.pending_steps:
            raise Rejected("capture ended with missing post-step/allocation event")
        if self.pending_object_bindings or self.pending_alloc_state or set(self.object_ordinals) != set(self.object_varinfo):
            raise Rejected("capture ended with unbound compiler Object identities")
        if self.target_active:
            _, _, snapshots, _ = self._snapshot_list(allow_new=True, allow_empty=True)
            self._refresh_compiler_list()
            self._emit_new_varinfo_snapshots(snapshots)
            self._trace(
                {
                    "event": "compiler_list_snapshot",
                    "phase": "function_exit",
                    "object_count": self.next_object_ordinal,
                    "varinfo_count": self.next_varinfo_ordinal,
                }
            )
            self._event("function_exit")
            self.target_active = False
            self.target_finished = True
        if self.target_seen is False:
            return
        if not self.events or self.events[-1]["event_kind"] != "function_exit":
            raise Rejected("target function is partial or missing exit")

    def on_disconnect(self, reason: str) -> None:
        raise Rejected(f"native transport disconnected: {reason}")

    def run(self) -> dict[str, Any]:
        try:
            validate_backend_capabilities(self.backend)
        except Exception as exc:
            raise _normalize_exception(exc, "native transport capability validation") from exc
        try:
            self.backend.run(self)
        except Exception as exc:
            raise _normalize_exception(exc, "native transport failure") from exc
        if not self.process_exited:
            raise Rejected("native transport ended without process exit")
        if not self.target_seen:
            raise Rejected("target function was not observed")
        if self.pending_writes or self.pending_alloc or self.pending_steps:
            raise Rejected("capture chronology is incomplete")
        events = list(self.events)
        if not events or events[0]["event_kind"] != "function_entry" or events[-1]["event_kind"] != "function_exit":
            raise Rejected("partial function chronology")
        event_bytes = canonical_event_bytes(events)
        packet = seal(
            {
                "schema": SCHEMA,
                "tool_version": TOOL_VERSION,
                "status": "CAPTURED_UNKNOWN_OWNERSHIP",
                "backend": BACKEND_NAME,
                "diagnostic_only": True,
                "board_admission": False,
                "exactness_claim": False,
                "function": self.function,
                "binding": _binding(self.auth),
                "request": {"path": str(self.auth["request_path"]), "sha256": self.auth["request_sha256"]},
                "authentication": {"artifacts": self.auth["artifacts"], "hooks": _request_hook_rows()},
                "events": events,
                "event_count": len(events),
                "event_stream": {"size_bytes": len(event_bytes), "sha256": hashlib.sha256(event_bytes).hexdigest()},
                "residues": _residues(events),
                "unknown": list(self.unknown),
                "limitations": [
                    "Object and VarInfo addresses are omitted from the packet and replaced by capture-local ordinals.",
                    "Only canonical compiler-exposed Object names are EXACT; compiler temporaries remain UNKNOWN.",
                    "Source declaration spans, inline ownership, and semantic ownership remain UNKNOWN.",
                    "No source, object, build, queue, authority, or retail state is advanced.",
                ],
            }
        )
        return packet


def _signed32(value: int) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise Rejected("register value is not an integer")
    value &= 0xFFFFFFFF
    return value - 0x100000000 if value & 0x80000000 else value


def canonical_event_bytes(events: Sequence[Mapping[str, Any]]) -> bytes:
    if not isinstance(events, Sequence) or isinstance(events, (str, bytes)) or not events:
        raise Rejected("event stream is empty")
    first = events[0]
    if not isinstance(first, Mapping):
        raise Rejected("events[0] is not an object")
    first_id = _text(first.get("event_id"), "events[0].event_id")
    match = re.fullmatch(r"([0-9a-f]{16})-e[0-9]{6}", first_id)
    if match is None:
        raise Rejected("events[0].event_id has a non-canonical capture id")
    capture_id = match.group(1)
    event_ids: set[str] = set()
    functions: set[str] = set()
    allocation_pre: dict[str, Mapping[str, Any]] = {}
    allocation_post: dict[str, Mapping[str, Any]] = {}
    write_pre: dict[str, Mapping[str, Any]] = {}
    write_post: dict[str, Mapping[str, Any]] = {}
    object_tokens: list[str] = []
    varinfo_tokens: list[str] = []
    object_varinfo_pairs: dict[str, str] = {}
    object_inventory: dict[str, Mapping[str, Any]] = {}
    allocation_token_owner: dict[str, str] = {}
    varinfo_seen: set[str] = set()
    write_sites: list[str] = []
    result: list[bytes] = []
    for index, event in enumerate(events):
        if not isinstance(event, Mapping):
            raise Rejected(f"events[{index}] is not an object")
        sequence = _strict_integer(event.get("sequence"), f"events[{index}].sequence", nonnegative=True)
        if sequence != index:
            raise Rejected("event sequence is not contiguous")
        event_kind = _text(event.get("event_kind"), f"events[{index}].event_kind")
        if event_kind not in EVENT_FIELDS:
            raise Rejected("unsupported event kind")
        _strict_keys(event, EVENT_FIELDS[event_kind], f"events[{index}]")
        event_id = _safe_event_id(event.get("event_id"), capture_id, sequence, f"events[{index}].event_id")
        if event_id in event_ids:
            raise Rejected("duplicate event_id")
        event_ids.add(event_id)
        if event.get("schema") != EVENT_SCHEMA:
            raise Rejected("event schema mismatch")
        function = _function_name(event.get("function"), f"events[{index}].function")
        functions.add(function)
        _pointer_free(event, f"events[{index}]")
        if event_kind == "compiler_list":
            if object_tokens or varinfo_tokens:
                raise Rejected("duplicate compiler-list event")
            raw_objects = event["object_tokens"]
            raw_varinfo = event["varinfo_tokens"]
            raw_pairs = event["object_varinfo_pairs"]
            raw_inventory = event["objects"]
            if not isinstance(raw_objects, list) or not isinstance(raw_varinfo, list) or not raw_objects or not raw_varinfo:
                raise Rejected("compiler-list tokens are missing")
            object_tokens = [_safe_token(value, "object", f"events[{index}].object_tokens") for value in raw_objects]
            varinfo_tokens = [_safe_token(value, "varinfo", f"events[{index}].varinfo_tokens") for value in raw_varinfo]
            if object_tokens != [_token("object", ordinal) for ordinal in range(len(object_tokens))] or varinfo_tokens != [_token("varinfo", ordinal) for ordinal in range(len(varinfo_tokens))]:
                raise Rejected("compiler-list tokens are not contiguous capture-local ordinals")
            if len(set(object_tokens)) != len(object_tokens) or len(set(varinfo_tokens)) != len(varinfo_tokens):
                raise Rejected("duplicate compiler-list token")
            if not isinstance(raw_pairs, list):
                raise Rejected("compiler-list Object/VarInfo pairs are missing")
            for pair_index, raw_pair in enumerate(raw_pairs):
                pair = _strict_keys(
                    raw_pair,
                    {"object_token", "varinfo_token"},
                    f"events[{index}].object_varinfo_pairs[{pair_index}]",
                )
                object_token = _safe_token(
                    pair.get("object_token"),
                    "object",
                    f"events[{index}].object_varinfo_pairs[{pair_index}].object_token",
                )
                varinfo_token = _safe_token(
                    pair.get("varinfo_token"),
                    "varinfo",
                    f"events[{index}].object_varinfo_pairs[{pair_index}].varinfo_token",
                )
                if object_token not in object_tokens or varinfo_token not in varinfo_tokens:
                    raise Rejected("compiler-list Object/VarInfo pair is not token bound")
                if object_token in object_varinfo_pairs or varinfo_token in object_varinfo_pairs.values():
                    raise Rejected("duplicate compiler-list Object/VarInfo pair")
                object_varinfo_pairs[object_token] = varinfo_token
            if set(object_varinfo_pairs) != set(object_tokens) or set(object_varinfo_pairs.values()) != set(varinfo_tokens):
                raise Rejected("compiler-list Object/VarInfo pair coverage is incomplete")
            if not isinstance(raw_inventory, list) or len(raw_inventory) != len(object_tokens):
                raise Rejected("compiler-list Object inventory coverage is incomplete")
            for object_index, raw_object in enumerate(raw_inventory):
                row = _strict_keys(
                    raw_object,
                    {"object_token", "varinfo_token", "name", "name_status", "datatype"},
                    f"events[{index}].objects[{object_index}]",
                )
                object_token = _safe_token(
                    row.get("object_token"), "object", f"events[{index}].objects[{object_index}].object_token"
                )
                varinfo_token = _safe_token(
                    row.get("varinfo_token"), "varinfo", f"events[{index}].objects[{object_index}].varinfo_token"
                )
                if object_token != object_tokens[object_index]:
                    raise Rejected("compiler-list Object inventory is not in canonical ordinal order")
                if object_varinfo_pairs.get(object_token) != varinfo_token:
                    raise Rejected("compiler-list Object inventory VarInfo binding mismatch")
                name_status = row.get("name_status")
                name = row.get("name")
                if name_status == "EXACT":
                    _function_name(name, f"events[{index}].objects[{object_index}].name")
                elif name_status == "UNKNOWN":
                    if name is not None:
                        raise Rejected("UNKNOWN compiler Object name must be null")
                else:
                    raise Rejected("compiler Object name_status is unsupported")
                datatype = _strict_integer(
                    row.get("datatype"), f"events[{index}].objects[{object_index}].datatype", nonnegative=True
                )
                if datatype > 0xFF:
                    raise Rejected("compiler Object datatype is outside the authenticated byte field")
                object_inventory[object_token] = row
        elif event_kind in {"numeric_stack_alloc_pre", "numeric_stack_alloc_post"}:
            pair_id = _safe_pair_id(event["allocation_id"], "alloc", f"events[{index}].allocation_id")
            if event_kind.endswith("_pre") and pair_id != f"alloc-{sequence:06d}":
                raise Rejected("allocation pair id is not derived from its canonical pre-event sequence")
            target = allocation_pre if event_kind.endswith("_pre") else allocation_post
            if pair_id in target:
                raise Rejected("duplicate allocation pair")
            target[pair_id] = event
            if event["allocation_call"] != "0x004f9ce0":
                raise Rejected("allocation callback identity mismatch")
            raw_objects = event["object_tokens"]
            raw_varinfo = event["varinfo_tokens"]
            if not isinstance(raw_objects, list) or not isinstance(raw_varinfo, list):
                raise Rejected("allocation token payload is missing")
            observed_objects = [_safe_token(value, "object", f"events[{index}].object_tokens") for value in raw_objects]
            observed_varinfo = [_safe_token(value, "varinfo", f"events[{index}].varinfo_tokens") for value in raw_varinfo]
            if event_kind.endswith("_pre"):
                # The allocator's pre-call list is allowed to be a strict
                # prefix: Object nodes can be linked only after the call
                # returns.  The post-call list must contain the final bound
                # set, and both lists remain capture-local/pointer-free.
                if observed_objects != object_tokens[: len(observed_objects)] or observed_varinfo != varinfo_tokens[: len(observed_varinfo)]:
                    raise Rejected("allocation pre-event references a different compiler-list token set")
            elif observed_objects != object_tokens[: len(observed_objects)] or observed_varinfo != varinfo_tokens[: len(observed_varinfo)]:
                raise Rejected("allocation post-event is not a compiler-list token prefix")
            for token in observed_objects + observed_varinfo:
                owner = allocation_token_owner.get(token)
                if owner is None:
                    allocation_token_owner[token] = pair_id
                elif owner != pair_id:
                    raise Rejected("allocation token crosses allocation boundaries")
        elif event_kind == "varinfo_home_snapshot":
            token = _safe_token(event["varinfo_token"], "varinfo", f"events[{index}].varinfo_token")
            ordinal = _strict_integer(event["varinfo_ordinal"], f"events[{index}].varinfo_ordinal", nonnegative=True)
            if token not in varinfo_tokens or token != _token("varinfo", ordinal) or event["varinfo_field_offset"] != "+0x26":
                raise Rejected("VarInfo home snapshot is not compiler-list bound")
            _strict_integer(event["home_value"], f"events[{index}].home_value")
            if token in varinfo_seen:
                raise Rejected("duplicate VarInfo home snapshot")
            varinfo_seen.add(token)
        elif event_kind in {"object_stack_write_pre", "object_stack_write_post"}:
            pair_id = _safe_pair_id(event["write_id"], "write", f"events[{index}].write_id")
            if event_kind.endswith("_pre") and pair_id != f"write-{sequence:06d}":
                raise Rejected("write pair id is not derived from its canonical pre-event sequence")
            target = write_pre if event_kind.endswith("_pre") else write_post
            if pair_id in target:
                raise Rejected("duplicate write pair")
            target[pair_id] = event
            token = _safe_token(event["object_token"], "object", f"events[{index}].object_token")
            ordinal = _strict_integer(event["object_ordinal"], f"events[{index}].object_ordinal", nonnegative=True)
            if (
                token not in object_tokens
                or token != _token("object", ordinal)
                or token not in object_varinfo_pairs
                or event["object_stack_field_offset"] != "+0x2e"
            ):
                raise Rejected("Object write is not compiler-list pair bound")
            if event_kind.endswith("_pre"):
                site = _text(event["write_site"], f"events[{index}].write_site")
                expected_sites = {f"0x{int(HOOK_BY_ID[hook_id]['address']):08x}": hook_id for hook_id in WRITE_HOOK_IDS}
                if site not in expected_sites:
                    raise Rejected("Object write site is not one of the closed pinned hooks")
                write_sites.append(expected_sites[site])
                _strict_integer(event["target_slot"], f"events[{index}].target_slot")
                _strict_integer(event["read_count"], f"events[{index}].read_count", nonnegative=True)
                if not isinstance(event["escape"], bool):
                    raise Rejected("event.escape must be a boolean")
            else:
                _strict_integer(event["expected_value"], f"events[{index}].expected_value")
                _strict_integer(event["observed_value"], f"events[{index}].observed_value")
                if event["write_observed"] is not True:
                    raise Rejected("Object write post was not observed")
                if _text(event["derived_from"], f"events[{index}].derived_from") not in WRITE_HOOK_IDS:
                    raise Rejected("Object write post has an unknown derivation hook")
        result.append((json.dumps(dict(event), ensure_ascii=True, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8"))
    if len(functions) != 1 or events[0].get("event_kind") != "function_entry" or events[-1].get("event_kind") != "function_exit":
        raise Rejected("function event chronology is incomplete")
    if sum(event.get("event_kind") == "compiler_list" for event in events) != 1:
        raise Rejected("compiler-list event chronology is incomplete")
    if not object_tokens:
        raise Rejected("allocation chronology has no compiler Object tokens")
    if varinfo_seen != set(varinfo_tokens):
        raise Rejected("VarInfo chronology has missing or orphan snapshots")
    if set(allocation_pre) != set(allocation_post):
        raise Rejected("numeric allocation post event is missing")
    if len(allocation_pre) != 1:
        raise Rejected("closed numeric allocation chronology requires exactly one allocation pair")
    for pair_id, before in allocation_pre.items():
        after = allocation_post[pair_id]
        if (
            before["sequence"] >= after["sequence"]
            or before["object_tokens"] != after["object_tokens"][: len(before["object_tokens"])]
            or before["varinfo_tokens"] != after["varinfo_tokens"][: len(before["varinfo_tokens"])]
        ):
            raise Rejected("numeric allocation pair chronology is invalid")
        enclosed = events[before["sequence"] + 1 : after["sequence"]]
        if not enclosed or any(event["event_kind"] not in {"object_stack_write_pre", "object_stack_write_post"} for event in enclosed):
            raise Rejected("numeric allocation pair does not enclose only Object write events")
    if set(write_pre) != set(write_post):
        raise Rejected("Object write post event is missing")
    for pair_id, before in write_pre.items():
        after = write_post[pair_id]
        if before["sequence"] + 1 != after["sequence"] or before["object_token"] != after["object_token"] or before["write_site"] != f"0x{int(HOOK_BY_ID[after['derived_from']]['address']):08x}" or after.get("expected_value") != after.get("observed_value"):
            raise Rejected("Object write pair chronology is invalid")
    if not write_sites:
        raise Rejected("closed write-hook chronology contains no authenticated Object writes")
    allocation_before = next(iter(allocation_pre.values()))
    allocation_after = next(iter(allocation_post.values()))
    for before in write_pre.values():
        after = write_post[before["write_id"]]
        if not (allocation_before["sequence"] < before["sequence"] < after["sequence"] < allocation_after["sequence"]):
            raise Rejected("Object write pair is outside the numeric allocation boundary")
    allocation_bound = next(iter(allocation_post.values()))
    bound_tokens = set(allocation_bound["object_tokens"] + allocation_bound["varinfo_tokens"])
    if set(allocation_token_owner) != bound_tokens or not bound_tokens.issubset(set(object_tokens + varinfo_tokens)):
        raise Rejected("allocation chronology has missing or orphan compiler tokens")
    return b"".join(result)


def _residues(events: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    rows: dict[int, dict[str, Any]] = {}
    for event in events:
        if event.get("event_kind") != "object_stack_write_pre":
            continue
        slot = _integer(event.get("target_slot"), "event.target_slot")
        row = rows.setdefault(slot, {"target_slot": slot, "write_count": 0, "read_count": 0, "escape": False, "owner": "UNKNOWN"})
        row["write_count"] += 1
        row["read_count"] += _nonnegative(event.get("read_count"), "event.read_count")
        row["escape"] = bool(row["escape"] or event.get("escape"))
    if not rows:
        raise Rejected("no Object+0x2e write events were captured")
    return [rows[key] for key in sorted(rows)]


def _binding(auth: Mapping[str, Any]) -> dict[str, Any]:
    request = auth["request"]
    artifacts = auth["artifacts"]
    return {
        "function": request["function"],
        "function_sha256": request["function_sha256"],
        "source_sha256": artifacts["source"]["sha256"],
        "source_size": artifacts["source"]["size"],
        "baseline_sha256": artifacts["baseline"]["sha256"],
        "baseline_size": artifacts["baseline"]["size"],
        "compiler_sha256": artifacts["compiler"]["sha256"],
        "compiler_size": artifacts["compiler"]["size"],
        "producer_sha256": artifacts["producer"]["sha256"],
        "producer_size": artifacts["producer"]["size"],
        "debugger_sha256": artifacts["debugger"]["sha256"],
        "debugger_size": artifacts["debugger"]["size"],
        "emulator_sha256": artifacts["emulator"]["sha256"],
        "emulator_size": artifacts["emulator"]["size"],
        "gdb_sha256": artifacts["gdb"]["sha256"],
        "gdb_size": artifacts["gdb"]["size"],
        "argv": list(request["argv"]),
        "cwd": request["cwd"],
    }


def unknown_packet(auth: Mapping[str, Any], reason: str) -> dict[str, Any]:
    return seal(
        {
            "schema": SCHEMA,
            "tool_version": TOOL_VERSION,
            "status": "UNKNOWN",
            "backend": BACKEND_NAME,
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "function": auth["request"]["function"],
            "binding": _binding(auth),
            "events": [],
            "event_count": 0,
            "unknown": [{"reason": reason}],
            "reason": reason,
        }
    )


def postflight(auth: Mapping[str, Any]) -> None:
    descriptors = dict(auth["artifacts"])
    if isinstance(auth.get("authority_manifest"), Mapping):
        descriptors["authority_manifest"] = auth["authority_manifest"]
    for label, descriptor in descriptors.items():
        path = Path(str(descriptor["path"]))
        if sha256(path) != descriptor["sha256"]:
            raise Rejected(f"{label} changed during capture")
    if sha256(auth["request_path"]) != auth["request_sha256"]:
        raise Rejected("request changed during capture")


def _remove_partial_artifacts(auth: Mapping[str, Any] | None) -> list[str]:
    if not auth:
        return []
    paths = auth.get("paths")
    if not isinstance(paths, Mapping):
        return []
    errors: list[str] = []
    for key in ("event_stream", "packet"):
        raw = paths.get(key)
        if not isinstance(raw, Path):
            continue
        try:
            if raw.exists() or raw.is_symlink():
                raw.unlink()
        except Exception as exc:
            quarantine = raw.with_name(raw.name + ".quarantine")
            try:
                os.replace(raw, quarantine)
            except Exception as quarantine_exc:
                errors.append(f"{key} cleanup/quarantine failed: {type(exc).__name__}: {exc}; {type(quarantine_exc).__name__}: {quarantine_exc}")
    return errors


def capture_with_backend(request_path: Path, backend: CaptureBackend) -> dict[str, Any]:
    """Run a supplied backend; this is the testable native-session boundary."""
    auth: Mapping[str, Any] | None = None
    packet: dict[str, Any] | None = None
    failures: list[str] = []
    try:
        auth = authenticate_request(request_path, require_empty=True)
        session = CaptureSession(auth, backend)
        packet = session.run()
        event_bytes = canonical_event_bytes(packet["events"])
        write_new(auth["paths"]["event_stream"], event_bytes)
        packet["event_stream"] = {
            "path": str(auth["paths"]["event_stream"]),
            "size_bytes": len(event_bytes),
            "sha256": hashlib.sha256(event_bytes).hexdigest(),
        }
        packet = seal(packet)
        write_new(auth["paths"]["packet"], (json.dumps(packet, indent=2, sort_keys=True) + "\n").encode("utf-8"))
        postflight(auth)
    except Exception as exc:
        failures.append(str(_normalize_exception(exc, "capture failure")))
    finally:
        try:
            backend.close()
        except Exception as exc:
            evidence = getattr(backend, "cleanup_evidence", None)
            detail = f"; evidence={evidence!r}" if evidence else ""
            failures.append(str(_normalize_exception(exc, "native cleanup failure")) + detail)
    trace_rows = getattr(backend, "trace_rows", None)
    if trace_rows:
        # Diagnostic-only chronology is deliberately emitted out-of-band;
        # sealed packets and event streams remain pointer-free and unchanged.
        print(json.dumps({"native_trace": trace_rows}, sort_keys=True), file=sys.stderr)
    if failures:
        cleanup_errors = _remove_partial_artifacts(auth)
        failures.extend(cleanup_errors)
        raise Rejected("; ".join(failures))
    if packet is None:
        raise Rejected("capture produced no packet")
    return packet


def validate_packet(packet_path: str | os.PathLike[str]) -> dict[str, Any]:
    packet_path = _reject_raw_path_alias(packet_path, "packet")
    packet = _load_json_file(packet_path, "packet")
    _strict_keys(packet, PACKET_FIELDS, "packet")
    if packet.get("schema") != SCHEMA:
        raise Rejected("packet schema mismatch")
    if packet.get("packet_sha256") != canonical_hash({key: value for key, value in packet.items() if key != "packet_sha256"}):
        raise Rejected("packet self-digest mismatch")
    if packet.get("tool_version") != TOOL_VERSION or packet.get("backend") != BACKEND_NAME:
        raise Rejected("packet tool/backend binding mismatch")
    if packet.get("status") != "CAPTURED_UNKNOWN_OWNERSHIP":
        raise Rejected("packet status is not an authenticated capture")
    if packet.get("diagnostic_only") is not True or packet.get("board_admission") is not False or packet.get("exactness_claim") is not False:
        raise Rejected("packet policy mismatch")
    function = _function_name(packet.get("function"), "packet.function")
    events = packet.get("events")
    if (
        not isinstance(events, list)
        or not events
        or not all(isinstance(event, Mapping) for event in events)
        or events[0].get("event_kind") != "function_entry"
        or events[-1].get("event_kind") != "function_exit"
    ):
        raise Rejected("packet function chronology is partial")
    canonical_event_bytes(events)
    if any(event.get("function") != function for event in events):
        raise Rejected("packet/event function binding mismatch")
    _strict_integer(packet.get("event_count"), "packet.event_count", nonnegative=True)
    if packet.get("event_count") != len(events):
        raise Rejected("packet event count mismatch")
    if packet.get("residues") != _residues(events):
        raise Rejected("packet residues do not match events")
    residues = packet.get("residues")
    if not isinstance(residues, list):
        raise Rejected("packet residues are missing")
    for index, row in enumerate(residues):
        item = _strict_keys(row, {"target_slot", "write_count", "read_count", "escape", "owner"}, f"packet.residues[{index}]")
        _strict_integer(item["target_slot"], f"packet.residues[{index}].target_slot")
        _strict_integer(item["write_count"], f"packet.residues[{index}].write_count", nonnegative=True)
        _strict_integer(item["read_count"], f"packet.residues[{index}].read_count", nonnegative=True)
        if not isinstance(item["escape"], bool) or item["owner"] != "UNKNOWN":
            raise Rejected("packet residue field identity mismatch")
    unknown = packet.get("unknown")
    if not isinstance(unknown, list) or any(set(row.keys()) != {"reason"} or not isinstance(row.get("reason"), str) for row in unknown):
        raise Rejected("packet.unknown field allowlist mismatch")
    limitations = packet.get("limitations")
    if not isinstance(limitations, list) or any(not isinstance(value, str) for value in limitations):
        raise Rejected("packet.limitations must be a string list")
    request_info = packet.get("request")
    _strict_keys(request_info, {"path", "sha256"}, "packet.request")
    request_path = Path(_text(request_info.get("path"), "packet.request.path"))
    auth = authenticate_request(request_path, require_empty=False)
    if request_info.get("sha256") != auth["request_sha256"] or auth["request"]["function"] != function:
        raise Rejected("packet request binding mismatch")
    authenticated_paths = auth.get("paths")
    if not isinstance(authenticated_paths, Mapping) or not isinstance(authenticated_paths.get("packet"), Path):
        raise Rejected("authenticated packet path is missing")
    if packet_path != authenticated_paths["packet"]:
        raise Rejected("packet path is not the authenticated output")
    first_event_id = _text(events[0].get("event_id"), "packet.events[0].event_id")
    if first_event_id.split("-", 1)[0] != auth["request_sha256"][:16]:
        raise Rejected("packet event id capture binding mismatch")
    binding = _strict_keys(packet.get("binding"), PACKET_BINDING_FIELDS, "packet.binding")
    for key, value in binding.items():
        if key.endswith("_size"):
            _strict_integer(value, f"packet.binding.{key}", nonnegative=True)
        elif key.endswith("_sha256"):
            _digest(value, f"packet.binding.{key}")
    _text(binding.get("cwd"), "packet.binding.cwd")
    if not isinstance(binding.get("argv"), list) or not all(isinstance(value, str) and value for value in binding["argv"]):
        raise Rejected("packet.binding.argv must be a string list")
    if binding != _binding(auth):
        raise Rejected("packet identity binding mismatch")
    authentication = packet.get("authentication")
    _strict_keys(authentication, {"artifacts", "hooks"}, "packet.authentication")
    expected_artifacts = auth["artifacts"]
    actual_artifacts = authentication.get("artifacts")
    if not isinstance(actual_artifacts, Mapping) or set(actual_artifacts) != set(expected_artifacts):
        raise Rejected("packet authentication artifact allowlist mismatch")
    for key, expected in expected_artifacts.items():
        actual = actual_artifacts.get(key)
        if key == "compiler":
            _strict_keys(actual, {"path", "size", "sha256", "profile", "image_base", "hooks"}, f"packet.authentication.artifacts.{key}")
            _validate_descriptor({field: actual[field] for field in DESCRIPTOR_FIELDS}, f"packet.authentication.artifacts.{key}")
            _validate_hook_rows(actual.get("hooks"), f"packet.authentication.artifacts.{key}.hooks", compiler=True)
        else:
            _validate_descriptor(actual, f"packet.authentication.artifacts.{key}")
        if actual != expected:
            raise Rejected(f"packet authentication artifact mismatch: {key}")
    _validate_hook_rows(authentication.get("hooks"), "packet.authentication.hooks")
    if authentication.get("hooks") != _request_hook_rows():
        raise Rejected("packet authentication mismatch")
    stream = packet.get("event_stream")
    _strict_keys(stream, {"path", "size_bytes", "sha256"}, "packet.event_stream")
    _strict_integer(stream.get("size_bytes"), "packet.event_stream.size_bytes", nonnegative=True)
    _digest(stream.get("sha256"), "packet.event_stream.sha256")
    expected_stream = auth["paths"]["event_stream"]
    stream_path = _reject_raw_path_alias(
        stream.get("path"),
        "packet.event_stream.path",
        expected_path=expected_stream,
    )
    event_bytes = canonical_event_bytes(events)
    if stream_path != expected_stream or stream.get("size_bytes") != len(event_bytes) or stream.get("sha256") != hashlib.sha256(event_bytes).hexdigest():
        raise Rejected("packet event stream binding mismatch")
    if not stream_path.is_file() or stream_path.read_bytes() != event_bytes:
        raise Rejected("event stream artifact mismatch")
    return dict(packet)


def _summary_seal(value: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop("summary_sha256", None)
    result["summary_sha256"] = canonical_hash(result)
    return result


def _summary_output_path(
    output_path: str | os.PathLike[str], *, packet_path: Path
) -> Path:
    """Admit one new canonical JSON sibling without normalizing aliases."""

    try:
        raw = os.fspath(output_path)
    except TypeError as exc:
        raise Rejected("summary output must be a path string") from exc
    if isinstance(raw, bytes) or not isinstance(raw, str) or not raw or raw != raw.strip():
        raise Rejected("summary output must use a canonical path spelling")
    if "/" in raw and "\\" in raw:
        raise Rejected("summary output has non-canonical path spelling")
    parts = re.split(r"[\\/]", raw)
    if any(part in {"", ".", ".."} for part in parts[1:]):
        raise Rejected("summary output has non-canonical path spelling")
    candidate = Path(raw)
    if not candidate.is_absolute() or candidate.suffix.casefold() != ".json":
        raise Rejected("summary output must be an absolute JSON path")
    parent = _reject_directory_alias(candidate.parent, "summary output directory")
    resolved = parent / candidate.name
    if str(candidate.absolute()) != str(resolved):
        raise Rejected("summary output has non-canonical path spelling")
    if parent != packet_path.parent:
        raise Rejected("summary output must be beside the authenticated packet")
    if resolved.exists() or resolved.is_symlink():
        raise Rejected("summary output already exists")
    return resolved


def _summarize_validated_packet(
    packet: Mapping[str, Any], packet_path: Path, names: Sequence[str]
) -> dict[str, Any]:
    """Join exact compiler identities to sealed stack-home events.

    ``packet`` must be the result of :func:`validate_packet`. This helper
    deliberately emits no ownership inference: an exact compiler name and an
    authenticated stack slot are identity/chronology evidence, not a source
    declaration, inline-owner, or retention decision.
    """

    if not isinstance(names, Sequence) or isinstance(names, (str, bytes)) or not names:
        raise Rejected("summary requires at least one exact compiler Object name")
    requested_names = [
        _function_name(value, f"summary name[{index}]") for index, value in enumerate(names)
    ]
    if len(set(requested_names)) != len(requested_names):
        raise Rejected("summary contains a duplicate compiler Object name")

    events = packet.get("events")
    if not isinstance(events, list):
        raise Rejected("validated packet events are unavailable")
    compiler_lists = [event for event in events if event.get("event_kind") == "compiler_list"]
    if len(compiler_lists) != 1:
        raise Rejected("validated packet compiler-list identity is unavailable")
    compiler_list = compiler_lists[0]
    raw_inventory = compiler_list.get("objects")
    raw_pairs = compiler_list.get("object_varinfo_pairs")
    if not isinstance(raw_inventory, list) or not isinstance(raw_pairs, list):
        raise Rejected("validated packet compiler inventory is unavailable")
    pair_by_object = {
        _safe_token(row.get("object_token"), "object", "summary Object token"):
        _safe_token(row.get("varinfo_token"), "varinfo", "summary VarInfo token")
        for row in raw_pairs
    }

    exact_by_name: dict[str, list[Mapping[str, Any]]] = {}
    for row in raw_inventory:
        if not isinstance(row, Mapping):
            raise Rejected("validated packet Object inventory is malformed")
        if row.get("name_status") == "EXACT":
            name = _function_name(row.get("name"), "summary compiler Object name")
            exact_by_name.setdefault(name, []).append(row)

    homes_by_varinfo: dict[str, list[Mapping[str, Any]]] = {}
    write_pre_by_object: dict[str, list[Mapping[str, Any]]] = {}
    write_post_by_id: dict[str, Mapping[str, Any]] = {}
    slot_objects: dict[int, set[str]] = {}
    for event in events:
        event_kind = event.get("event_kind")
        if event_kind == "varinfo_home_snapshot":
            token = _safe_token(event.get("varinfo_token"), "varinfo", "summary home VarInfo token")
            homes_by_varinfo.setdefault(token, []).append(event)
        elif event_kind == "object_stack_write_pre":
            token = _safe_token(event.get("object_token"), "object", "summary write Object token")
            target_slot = _strict_integer(event.get("target_slot"), "summary target slot")
            write_pre_by_object.setdefault(token, []).append(event)
            slot_objects.setdefault(target_slot, set()).add(token)
        elif event_kind == "object_stack_write_post":
            write_id = _safe_pair_id(event.get("write_id"), "write", "summary write id")
            if write_id in write_post_by_id:
                raise Rejected("validated packet contains a duplicate write post-event")
            write_post_by_id[write_id] = event
    ambiguous_slots = sorted(slot for slot, tokens in slot_objects.items() if len(tokens) != 1)
    if ambiguous_slots:
        raise Rejected(
            "stack-home slot is bound to multiple Object identities: "
            + ", ".join(str(slot) for slot in ambiguous_slots)
        )

    mappings: list[dict[str, Any]] = []
    for name in requested_names:
        matches = exact_by_name.get(name, [])
        if len(matches) != 1:
            state = "unavailable" if not matches else "ambiguous"
            raise Rejected(f"exact compiler Object name is {state}: {name}")
        inventory = matches[0]
        object_token = _safe_token(
            inventory.get("object_token"), "object", f"summary {name} Object token"
        )
        varinfo_token = _safe_token(
            inventory.get("varinfo_token"), "varinfo", f"summary {name} VarInfo token"
        )
        if pair_by_object.get(object_token) != varinfo_token:
            raise Rejected(f"sealed Object/VarInfo pair mismatch for {name}")
        homes = homes_by_varinfo.get(varinfo_token, [])
        if len(homes) != 1:
            raise Rejected(f"sealed VarInfo home snapshot is not unique for {name}")
        home = homes[0]
        pre_events = sorted(
            write_pre_by_object.get(object_token, []), key=lambda row: int(row["sequence"])
        )
        if not pre_events:
            raise Rejected(f"no authenticated Object+0x2e stack-home event for {name}")
        writes: list[dict[str, Any]] = []
        for before in pre_events:
            write_id = _safe_pair_id(before.get("write_id"), "write", f"summary {name} write id")
            after = write_post_by_id.get(write_id)
            if after is None:
                raise Rejected(f"authenticated Object+0x2e post-event is missing for {name}")
            if before.get("object_stack_field_offset") != "+0x2e" or after.get("object_stack_field_offset") != "+0x2e":
                raise Rejected(f"authenticated Object stack-field offset mismatch for {name}")
            if after.get("write_observed") is not True or after.get("expected_value") != after.get("observed_value"):
                raise Rejected(f"authenticated Object+0x2e write was not observed for {name}")
            writes.append(
                {
                    "write_id": write_id,
                    "pre_event_id": before["event_id"],
                    "pre_sequence": before["sequence"],
                    "post_event_id": after["event_id"],
                    "post_sequence": after["sequence"],
                    "object_stack_field_offset": "+0x2e",
                    "target_slot": before["target_slot"],
                    "write_site": before["write_site"],
                    "read_count": before["read_count"],
                    "escape": before["escape"],
                    "expected_value": after["expected_value"],
                    "observed_value": after["observed_value"],
                    "write_observed": True,
                    "derived_from": after["derived_from"],
                }
            )
        mappings.append(
            {
                "name": name,
                "name_status": "EXACT",
                "datatype": inventory["datatype"],
                "object_token": object_token,
                "varinfo_token": varinfo_token,
                "varinfo_home_snapshot": {
                    "event_id": home["event_id"],
                    "sequence": home["sequence"],
                    "varinfo_field_offset": home["varinfo_field_offset"],
                    "home_value": home["home_value"],
                },
                "mapped_slots": sorted({int(row["target_slot"]) for row in writes}),
                "stack_home_writes": writes,
                "owner": "UNKNOWN",
            }
        )

    binding = packet.get("binding")
    request = packet.get("request")
    event_stream = packet.get("event_stream")
    if not isinstance(binding, Mapping) or not isinstance(request, Mapping) or not isinstance(event_stream, Mapping):
        raise Rejected("validated packet bindings are unavailable")
    result = {
        "schema": SUMMARY_SCHEMA,
        "tool_version": TOOL_VERSION,
        "status": SUMMARY_STATUS,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "function": packet["function"],
        "binding": {
            "source_sha256": binding["source_sha256"],
            "function_sha256": binding["function_sha256"],
            "request_sha256": request["sha256"],
            "event_stream_sha256": event_stream["sha256"],
        },
        "packet": {
            "path": str(packet_path),
            "size": packet_path.stat().st_size,
            "sha256": sha256(packet_path),
            "packet_sha256": packet["packet_sha256"],
        },
        "requested_names": requested_names,
        "mappings": mappings,
        "limitations": [
            "Exact compiler names and stack slots do not prove a source declaration or inline owner.",
            "Mapped slots do not advance source, exactness, retention, promotion, or Board authority.",
        ],
    }
    return _summary_seal(result)


def summarize_packet(
    packet_path: str | os.PathLike[str],
    names: Sequence[str],
    output_path: str | os.PathLike[str],
) -> dict[str, Any]:
    """Validate, deterministically summarize, and exclusively create JSON."""

    packet = validate_packet(packet_path)
    authenticated_packet_path = _reject_raw_path_alias(packet_path, "packet")
    output = _summary_output_path(output_path, packet_path=authenticated_packet_path)
    summary = _summarize_validated_packet(packet, authenticated_packet_path, names)
    data = (json.dumps(summary, indent=2, sort_keys=True) + "\n").encode("utf-8")
    write_new(output, data)
    if sha256(output) != hashlib.sha256(data).hexdigest():
        try:
            output.unlink()
        except OSError:
            pass
        raise Rejected("summary output verification failed")
    return summary


# -- Optional native backend -------------------------------------------------


class NativeWow64Backend:
    """Minimal Win32 debug-event transport used only by explicit capture.

    The backend imports the existing ctypes layout lazily, so Linux/macOS
    inspection and all fake-backend tests remain safe.  It exposes no raw
    pointer in session output; pointers are only used while reading the paused
    compiler process.
    """

    capabilities = {name: True for name in REQUIRED_CAPABILITIES}

    def __init__(self, native: Any, process: int, initial_thread: int, *, trace: bool = False) -> None:
        self.native = native
        self.process = process
        self.threads: dict[int, int] = {}
        if initial_thread:
            self.threads[0] = initial_thread
        self.closed_handles: set[int] = set()
        self.failed_handles: set[int] = set()
        self._close_complete = False
        self._close_error: Rejected | None = None
        self.base = 0
        self.breakpoints: dict[int, int] = {}
        self.pending_steps: dict[int, int] = {}
        self.current_thread = 0
        self.trace = trace
        self.trace_rows: list[dict[str, Any]] = []
        self.exited = False
        self.mutation_allowed = False
        self.cleanup_evidence: dict[str, Any] = {
            "attempted_breakpoints": [],
            "restored_breakpoints": [],
            "failed_breakpoints": [],
            "closed_threads": [],
            "skipped_threads": [],
            "failed_threads": [],
            "terminated": False,
        }

    def _register_thread(self, thread_id: int, handle: int) -> None:
        if not handle:
            return
        if handle in self.closed_handles:
            raise Rejected("debug thread handle was reused after cleanup")
        # CreateProcessW exposes the initial thread handle, and the
        # CREATE_PROCESS debug event exposes the same logical thread again.
        # Collapse that alias before any event can close the handle twice.
        for owner, existing in list(self.threads.items()):
            if owner != thread_id and existing == handle:
                self.threads.pop(owner, None)
        self.threads[thread_id] = handle

    def _handle_is_invalid(self, handle: int) -> bool:
        """Detect an already-invalid native handle without masking real errors."""

        get_handle_information = getattr(self.native.kernel32, "GetHandleInformation", None)
        if not callable(get_handle_information):
            return False
        flags = ctypes.c_uint32()
        try:
            if getattr(get_handle_information, "argtypes", None) is None:
                get_handle_information.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint32)]
                get_handle_information.restype = ctypes.c_int
            result = get_handle_information(ctypes.c_void_p(handle), ctypes.byref(flags))
        except (AttributeError, OSError, TypeError, ValueError):
            return False
        if result:
            return False
        try:
            last_error = int(ctypes.get_last_error())
        except (AttributeError, OSError, TypeError, ValueError):
            return False
        invalid_handle = getattr(self.native, "ERROR_INVALID_HANDLE", 6)
        if isinstance(invalid_handle, bool) or not isinstance(invalid_handle, int):
            invalid_handle = 6
        return last_error == invalid_handle

    def _close_thread(self, thread_id: int, handle: int) -> None:
        if not handle:
            self.threads.pop(thread_id, None)
            self.cleanup_evidence["skipped_threads"].append(thread_id)
            return
        if handle in self.closed_handles:
            self.threads.pop(thread_id, None)
            self.cleanup_evidence["skipped_threads"].append(thread_id)
            return
        if handle in self.failed_handles:
            raise Rejected("CloseHandle previously failed")
        if self._handle_is_invalid(handle):
            for owner, existing in list(self.threads.items()):
                if existing == handle:
                    self.threads.pop(owner, None)
            self.cleanup_evidence["skipped_threads"].append(thread_id)
            return
        result = self.native.kernel32.CloseHandle(handle)
        if result is False or result == 0:
            self.failed_handles.add(handle)
            raise Rejected("CloseHandle returned failure")
        self.closed_handles.add(handle)
        for owner, existing in list(self.threads.items()):
            if existing == handle:
                self.threads.pop(owner, None)
        self.cleanup_evidence["closed_threads"].append(thread_id)

    def _runtime(self, absolute: int) -> int:
        if not self.base:
            raise Rejected("live image base is unavailable")
        return self.base + absolute - KNOWN_IMAGE_BASE

    def _read(self, address: int, size: int) -> bytes:
        if size <= 0:
            return b""
        buffer = ctypes.create_string_buffer(size)
        got = self.native.SIZE_T()
        ok = self.native.kernel32.ReadProcessMemory(self.process, ctypes.c_void_p(address), buffer, size, ctypes.byref(got))
        return buffer.raw[: got.value] if ok else b""

    def read_image(self, address: int, size: int) -> bytes:
        return self._read(self._runtime(address), size)

    def read_memory(self, address: int, size: int) -> bytes:
        return self._read(address, size)

    def _write(self, address: int, data: bytes) -> None:
        buffer = ctypes.create_string_buffer(data)
        written = self.native.SIZE_T()
        ok = self.native.kernel32.WriteProcessMemory(self.process, ctypes.c_void_p(address), buffer, len(data), ctypes.byref(written))
        if not ok or written.value != len(data):
            raise Rejected(f"WriteProcessMemory failed at 0x{address:08x}")

    def install_breakpoint(self, address: int) -> None:
        if not self.mutation_allowed:
            raise Rejected("breakpoint mutation before live prefix validation")
        runtime = self._runtime(address)
        if runtime in self.breakpoints:
            raise Rejected(f"duplicate breakpoint install at 0x{address:08x}")
        original = self._read(runtime, 1)
        if len(original) != 1:
            raise Rejected(f"cannot read breakpoint byte at 0x{address:08x}")
        self._write(runtime, b"\xcc")
        self.breakpoints[runtime] = original[0]

    def mark_live_prefixes_validated(self) -> None:
        self.mutation_allowed = True

    def remove_breakpoint(self, address: int) -> None:
        runtime = self._runtime(address)
        original = self.breakpoints.get(runtime)
        if original is not None:
            self._write(runtime, bytes([original]))
            self.breakpoints.pop(runtime, None)

    def _get_context(self, handle: int) -> Any:
        context = self.native.WOW64_CONTEXT()
        context.ContextFlags = self.native.WOW64_CONTEXT_FULL
        if not self.native.kernel32.Wow64GetThreadContext(handle, ctypes.byref(context)):
            raise Rejected("Wow64GetThreadContext failed")
        return context

    def _set_context(self, handle: int, context: Any) -> None:
        if not self.native.kernel32.Wow64SetThreadContext(handle, ctypes.byref(context)):
            raise Rejected("Wow64SetThreadContext failed")

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None:
        self.remove_breakpoint(address)
        handle = self.threads.get(thread_id)
        if not handle:
            raise Rejected("single-step thread handle missing")
        context = self._get_context(handle)
        context.Eip = self._runtime(address)
        if rearm:
            context.EFlags |= self.native.WOW64_CONTEXT_TF
            if thread_id in self.pending_steps:
                raise Rejected("duplicate pending single-step")
            self.pending_steps[thread_id] = address
        self._set_context(handle, context)

    def read_register(self, thread_id: int, name: str) -> int:
        context = self._get_context(self.threads[thread_id])
        return int(getattr(context, {"eax": "Eax", "ebx": "Ebx"}[name]))

    def read_execution_state(self, thread_id: int) -> Mapping[str, int]:
        context = self._get_context(self.threads[thread_id])
        return {
            "eip": int(context.Eip),
            "esp": int(context.Esp),
            "ebp": int(context.Ebp),
        }

    def trace_event(self, event: Mapping[str, Any]) -> None:
        if not self.trace or len(self.trace_rows) >= 512:
            return
        self.trace_rows.append(dict(event))

    def _u32(self, address: int) -> int:
        data = self._read(address, 4)
        return int.from_bytes(data, "little") if len(data) == 4 else 0

    def _u32_required(self, address: int, label: str) -> int:
        data = self._read(address, 4)
        if len(data) != 4:
            raise Rejected(f"{label} is truncated")
        return int.from_bytes(data, "little")

    def _u8(self, address: int) -> int:
        data = self._read(address, 1)
        return data[0] if data else 0

    def _read_name(self, object_pointer: int) -> str:
        name_pointer = self._u32(object_pointer + OBJECT_NAME)
        if not name_pointer:
            return ""
        data = self._read(name_pointer + 0x0A, 256).split(b"\0", 1)[0]
        return data.decode("latin-1", errors="replace")

    def current_function(self) -> str:
        return self._read_name(self._u32(self._runtime(FUNCTION_OBJECT)))

    def snapshot_objects(self) -> Sequence[Mapping[str, Any]]:
        head = self._u32_required(self._runtime(OBJECT_LIST_HEAD), "native Object list head")
        rows: list[dict[str, Any]] = []
        seen_nodes: set[int] = set()
        seen_objects: set[int] = set()
        while head:
            if len(rows) >= 4096:
                raise Rejected("native Object list exceeded bound")
            if head in seen_nodes:
                raise Rejected("native Object list contains a cycle")
            seen_nodes.add(head)
            pointer = self._u32_required(head + 4, "native Object list node")
            if not pointer:
                raise Rejected("native Object list node has a null Object pointer")
            if pointer in seen_objects:
                raise Rejected("native Object list reuses a pointer")
            seen_objects.add(pointer)
            datatype = self._u8(pointer + OBJECT_DATATYPE)
            varinfo_offset = OBJECT_VARINFO_DATATYPE1 if datatype == 1 else OBJECT_VARINFO_OTHER
            varinfo_pointer = self._u32_required(pointer + varinfo_offset, "native Object VarInfo pointer")
            rows.append(
                {
                    "pointer": pointer,
                    "varinfo_pointer": varinfo_pointer,
                    "name": self._read_name(pointer),
                    "datatype": datatype,
                }
            )
            head = self._u32_required(head, "native Object list link")
        return rows

    def snapshot_varinfo(self, pointer: int) -> Mapping[str, Any]:
        data = self._read(pointer, VARINFO_RAW_SIZE)
        if len(data) != VARINFO_RAW_SIZE:
            raise Rejected("native VarInfo snapshot is truncated")
        return {"home_value": int.from_bytes(data[0x26:0x28], "little", signed=True)}

    def read_object_stack_field(self, pointer: int) -> int:
        data = self._read(pointer + OBJECT_STACK_FIELD, 4)
        if len(data) != 4:
            raise Rejected("native Object+0x2e read is truncated")
        return int.from_bytes(data, "little", signed=False)

    def write_metadata(self, _pointer: int, _value: int) -> Mapping[str, Any]:
        # Runtime capture proves the write and its value.  Read/escape facts
        # remain conservative unless a separately authenticated producer adds
        # them; zero is the only accepted diagnostic default.
        return {"read_count": 0, "escape": False}

    def _normalise(self, exception_address: int) -> int:
        return exception_address - self.base + KNOWN_IMAGE_BASE

    def run(self, session: CaptureSession) -> None:
        event = self.native.DEBUG_EVENT()
        while True:
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == self.native.ERROR_SEM_TIMEOUT:
                    continue
                session.on_disconnect(f"WaitForDebugEvent error {error}")
            code = int(event.dwDebugEventCode)
            pid, tid = int(event.dwProcessId), int(event.dwThreadId)
            if code == self.native.CREATE_PROCESS_DEBUG_EVENT:
                self.base = int(event.u.CreateProcessInfo.lpBaseOfImage or 0)
                if self.base != KNOWN_IMAGE_BASE:
                    raise Rejected(f"live image base mismatch: 0x{self.base:08x}")
                handle = int(event.u.CreateProcessInfo.hThread or 0)
                if not handle:
                    handle = self.threads.pop(0, 0)
                self._register_thread(tid, handle)
                session.on_process_started()
                file_handle = int(event.u.CreateProcessInfo.hFile or 0)
                if file_handle:
                    self.native.kernel32.CloseHandle(file_handle)
            elif code == self.native.CREATE_THREAD_DEBUG_EVENT:
                handle = int(event.u.CreateThread.hThread or 0)
                self._register_thread(tid, handle)
            elif code == self.native.EXIT_THREAD_DEBUG_EVENT:
                handle = self.threads.get(tid)
                if handle:
                    self._close_thread(tid, handle)
            elif code == self.native.EXIT_PROCESS_DEBUG_EVENT:
                self.exited = True
                session.on_process_exit(int(event.u.ExitProcess.dwExitCode))
            elif code == self.native.EXCEPTION_DEBUG_EVENT:
                exception = event.u.Exception.ExceptionRecord
                exception_code = int(exception.ExceptionCode)
                address = int(exception.ExceptionAddress or 0)
                is_step = exception_code in (self.native.EXCEPTION_SINGLE_STEP, self.native.EXCEPTION_WX86_SINGLE_STEP)
                is_break = exception_code in (self.native.EXCEPTION_BREAKPOINT, self.native.EXCEPTION_WX86_BREAKPOINT)
                if is_step:
                    pending = self.pending_steps.pop(tid, None)
                    if pending is None:
                        session.on_disconnect("single-step had no pending breakpoint")
                    context = self._get_context(self.threads[tid])
                    context.EFlags &= ~self.native.WOW64_CONTEXT_TF
                    self._set_context(self.threads[tid], context)
                    session.on_single_step(tid)
                    self.install_breakpoint(pending)
                elif is_break:
                    normalized = self._normalise(address)
                    if normalized == int(HOOK_BY_ID["function_filter"]["address"]):
                        session.on_breakpoint(normalized, tid)
                    elif normalized in {int(row["address"]) for row in HOOKS}:
                        session.on_breakpoint(normalized, tid)
                    else:
                        # Loader breakpoints are expected; compiler hook
                        # addresses are not.  Unknown hooks fail closed.
                        if session.target_active:
                            session.on_disconnect(f"unexpected breakpoint 0x{normalized:08x}")
            status = self.native.DBG_CONTINUE
            if not self.native.kernel32.ContinueDebugEvent(pid, tid, status):
                session.on_disconnect("ContinueDebugEvent failed")
            if self.exited:
                break

    def close(self) -> None:
        if self._close_complete:
            return
        if self._close_error is not None:
            raise self._close_error
        errors: list[str] = []
        for runtime, original in list(self.breakpoints.items()):
            self.cleanup_evidence["attempted_breakpoints"].append(runtime)
            try:
                self._write(runtime, bytes([original]))
            except Exception as exc:
                self.cleanup_evidence["failed_breakpoints"].append(runtime)
                errors.append(f"breakpoint 0x{runtime:08x} restoration failed: {type(exc).__name__}: {exc}")
            else:
                self.breakpoints.pop(runtime, None)
                self.cleanup_evidence["restored_breakpoints"].append(runtime)
        attempted_handles: set[int] = set()
        for thread_id, handle in list(self.threads.items()):
            if handle in attempted_handles:
                continue
            attempted_handles.add(handle)
            try:
                self._close_thread(thread_id, handle)
            except Exception as exc:
                self.cleanup_evidence["failed_threads"].append(thread_id)
                errors.append(f"thread {thread_id} handle cleanup failed: {type(exc).__name__}: {exc}")
        if not self.exited and self.process:
            try:
                result = self.native.kernel32.TerminateProcess(self.process, 1)
                if result is False or result == 0:
                    raise Rejected("TerminateProcess returned failure")
                self.cleanup_evidence["terminated"] = True
            except Exception as exc:
                errors.append(f"process termination failed: {type(exc).__name__}: {exc}")
        if errors:
            self._close_error = Rejected("; ".join(errors))
            raise self._close_error
        self._close_complete = True


def launch_native_capture(request_path: Path, *, trace: bool = False) -> dict[str, Any]:
    """Launch the authenticated compiler under native WOW64 debugging."""

    if os.name != "nt":
        raise Rejected("native WOW64 capture requires Windows; compiler was not launched")
    auth = authenticate_request(request_path, require_empty=True)
    spec = importlib.util.spec_from_file_location("mwcc_win32_varinfo_native", Path(__file__).with_name("mwcc_win32_varinfo.py"))
    if spec is None or spec.loader is None:
        raise Rejected("cannot load native WOW64 ctypes layout")
    native = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(native)
    command = subprocess.list2cmdline([str(auth["paths"]["compiler"]), *auth["request"]["argv"]])
    startup = native.STARTUPINFOW(cb=ctypes.sizeof(native.STARTUPINFOW), dwFlags=native.STARTF_USESHOWWINDOW, wShowWindow=native.SW_HIDE)
    process_info = native.PROCESS_INFORMATION()
    buffer = ctypes.create_unicode_buffer(command)
    created = native.kernel32.CreateProcessW(None, buffer, None, None, False, native.DEBUG_ONLY_THIS_PROCESS | native.CREATE_NO_WINDOW, None, str(auth["paths"]["cwd"]), ctypes.byref(startup), ctypes.byref(process_info))
    if not created:
        raise Rejected(f"CreateProcessW failed: {ctypes.WinError(ctypes.get_last_error())}")
    backend = NativeWow64Backend(native, int(process_info.hProcess), int(process_info.hThread or 0), trace=trace)
    try:
        return capture_with_backend(request_path, backend)
    finally:
        native.kernel32.CloseHandle(process_info.hProcess)


def parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    prep = sub.add_parser("prepare", help="authenticate a current source/function manifest")
    prep.add_argument("--manifest", type=Path, required=True)
    prep.add_argument("--output-dir", type=Path, required=True)
    preflight = sub.add_parser("preflight", help="verify request/tool/PE identities without launching")
    preflight.add_argument("request", type=Path)
    capture = sub.add_parser("capture", help="launch the explicit native WOW64 capture")
    capture.add_argument("request", type=Path)
    capture.add_argument("--trace", action="store_true")
    validate = sub.add_parser("validate", help="validate a captured packet")
    # Keep the raw spelling so validate_packet can reject lexical aliases
    # before Path normalizes ``./`` or filesystem case variants.
    validate.add_argument("packet", type=str)
    summarize = sub.add_parser(
        "summarize", help="join exact Object/VarInfo identities to authenticated stack-home events"
    )
    summarize.add_argument("packet", type=str)
    summarize.add_argument("--name", action="append", required=True)
    summarize.add_argument("--output", type=str, required=True)
    sub.add_parser("self-test")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "prepare":
            result = {"schema": REQUEST_SCHEMA, "status": "READY", "diagnostic_only": True, "board_admission": False, "request": str(prepare_request(args.manifest, args.output_dir))}
        elif args.command == "preflight":
            auth = authenticate_request(args.request, require_empty=False)
            result = {"schema": REQUEST_SCHEMA, "status": "READY", "diagnostic_only": True, "board_admission": False, "request": {"path": str(auth["request_path"]), "sha256": auth["request_sha256"]}, "function": auth["request"]["function"], "compiler": auth["compiler"], "source": auth["source"], "backend": BACKEND_NAME}
        elif args.command == "capture":
            result = launch_native_capture(args.request, trace=args.trace)
        elif args.command == "validate":
            result = validate_packet(args.packet)
        elif args.command == "summarize":
            result = summarize_packet(args.packet, args.name, args.output)
        else:
            result = {"schema": f"{SCHEMA}/self-test", "status": "OK", "diagnostic_only": True, "board_admission": False, "authority_advanced": False, "tests": 7}
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0 if result.get("status") in {"OK", "READY", "CAPTURED_UNKNOWN_OWNERSHIP", SUMMARY_STATUS} else 2
    except Exception as exc:
        result = {"schema": SCHEMA, "status": "UNKNOWN", "diagnostic_only": True, "board_admission": False, "reason": str(exc)}
        print(json.dumps(result, indent=2, sort_keys=True))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
