#!/usr/bin/env python3
"""Content-addressed matching experiments and bounded read-only diagnostics.

The workbench intentionally does not compile, edit recovered source, mutate the
agent queue, or advance proof authority.  It records already-produced candidate
artifacts, prevents duplicate experiments, stores large reports once, and runs
only explicitly registered read-only diagnostic jobs in parallel.  Compiler,
native-debug, proof, authority, integration, and retail-link work remains in the
existing serialized tools.

The frozen request is a trusted policy document, not an operating-system
sandbox.  Register only independently reviewed diagnostics, and declare every
file dependency so the workbench can authenticate it before and after a job.
"""

from __future__ import annotations

import argparse
import contextlib
import copy
import difflib
import gzip
import hashlib
import json
import locale
import math
import os
import re
import stat
import subprocess
import sys
import tempfile
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

REQUEST_SCHEMA = "match_workbench_request/v1"
SESSION_SCHEMA = "match_workbench_session/v1"
CANDIDATE_SCHEMA = "match_workbench_candidate/v1"
JOBS_SCHEMA = "match_workbench_jobs/v1"
DIAGNOSTIC_SCHEMA = "match_workbench_diagnostic/v1"
MATRIX_SCHEMA = "match_workbench_matrix/v1"
FUNCTION_TELEMETRY_SCHEMA = "match_workbench_function_telemetry/v1"
CAUSAL_REDUCER_SCHEMA = "match_workbench_causal_reducer/v1"
POOL_DECODER_SCHEMA = "match_workbench_pool_decoder/v1"
INTERACTION_PLANNER_SCHEMA = "match_workbench_interaction_plan/v1"
COMPILE_ATTESTATION_SCHEMA = "match_workbench_compile_attestation/v1"
PROVENANCE_MANIFEST_SCHEMA = "match_workbench_provenance_manifest/v1"
PROVENANCE_AUDIT_SCHEMA = "match_workbench_provenance_audit/v1"
PROVENANCE_MIGRATION_SCHEMA = "match_workbench_provenance_migration/v1"
INDEX_SCHEMA = "match_workbench_index/v1"
COMPILE_INPUT_SCHEMA = "match_workbench_compile_input/v1"
ASSESSMENT_SCHEMA = "match_workbench_assessment/v1"
PREPARATION_SCHEMA = "match_workbench_preparation/v1"
RESIDUALS_SCHEMA = "match_workbench_residuals/v1"
STACK_RESIDUE_SCHEMA = "match_workbench_stack_residue/v1"
DONOR_SHAPES_SCHEMA = "match_workbench_donor_shapes/v1"
DONOR_REGISTRY_SCHEMA = "match_workbench_donor_registry/v1"
DONOR_REGISTRY_LIST_SCHEMA = "match_workbench_donor_registry_list/v1"
DONOR_REJECTION_SCHEMA = "match_workbench_donor_rejection/v1"
DEPENDENCY_PROVENANCE_SCHEMA = "match_workbench_dependency_provenance/v1"
INCLUDE_ROOT_SCHEMA = "match_workbench_include_root/v1"
ENVIRONMENT_SCHEMA = "match_workbench_environment/v1"
OUTPUT_BINDING_SCHEMA = "match_workbench_output_binding/v1"
SOURCE_TREE_SCHEMA = "match_workbench_source_tree/v1"
ARGV_BINDING_SCHEMA = "match_workbench_argv_binding/v1"
SCHEMA_VERSION = 1

SAFE_RESOURCE_CLASSES = frozenset(
    {"read_only", "read_only_cpu", "read_only_io", "read_only_subprocess"}
)
SERIAL_RESOURCE_CLASSES = frozenset(
    {
        "compiler",
        "compiler_heavy",
        "native_debug",
        "proof",
        "proof_serial",
        "authority",
        "authority_mutator",
        "integration",
        "retail_link",
        "native",
        "link",
        "retail",
    }
)
SHA256_RE = re.compile(r"[0-9a-f]{64}")
SAFE_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}")
REPARSE_ATTRIBUTE = 0x400

DONOR_SOURCE_KINDS = frozenset(
    {
        "same-tu",
        "same-game-history",
        "cross-game-lineage",
        "target-derived",
        "diagnostic-only",
    }
)
DONOR_STATUSES = frozenset({"accepted", "rejected"})
DONOR_ADMISSIBILITY = frozenset({"admissible", "conditional", "inadmissible"})


class MatchError(ValueError):
    """Invalid, stale, or unsafe matching-workbench input."""


def _fail(message: str) -> None:
    raise MatchError(message)


def _canonical(value: Any) -> bytes:
    try:
        return (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise MatchError(f"cannot serialize canonical JSON: {exc}") from exc


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            _fail(f"duplicate JSON key {key!r}")
        result[key] = value
    return result


def _load_json(path: Path, label: str) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=_pairs)
    except FileNotFoundError as exc:
        raise MatchError(f"{label} does not exist: {path}") from exc
    except UnicodeDecodeError as exc:
        raise MatchError(f"{label} is not UTF-8: {path}") from exc
    except json.JSONDecodeError as exc:
        raise MatchError(
            f"invalid {label} JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc


def _closed(
    value: Any,
    *,
    allowed: set[str] | frozenset[str],
    required: set[str] | frozenset[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        _fail(f"{label} must be an object")
    unknown = sorted(set(value) - set(allowed))
    if unknown:
        _fail(f"{label} contains unknown field {unknown[0]!r}")
    missing = sorted(set(required) - set(value))
    if missing:
        _fail(f"{label} lacks required field {missing[0]!r}")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        _fail(f"{label} must be a non-empty string")
    return value.strip()


def _focus_symbols(
    value: Any,
    label: str,
    *,
    default: str | None = None,
) -> tuple[str, ...]:
    """Normalize one or more focus symbols to a stable, duplicate-free tuple."""
    if value is None:
        if default is None:
            _fail(f"{label} must contain at least one symbol")
        raw_values: list[Any] = [default]
    elif isinstance(value, str):
        raw_values = [value]
    elif isinstance(value, Sequence) and not isinstance(value, (bytes, bytearray)):
        raw_values = list(value)
    else:
        _fail(f"{label} must be a symbol or a sequence of symbols")
    if not raw_values:
        _fail(f"{label} must contain at least one symbol")
    normalized = [_text(item, f"{label}[{index}]") for index, item in enumerate(raw_values)]
    if len(set(normalized)) != len(normalized):
        _fail(f"{label} contains duplicate symbols")
    return tuple(sorted(normalized))


def _stored_focus_symbols(
    value: Mapping[str, Any],
    *,
    default: str,
    label: str = "candidate focus symbols",
) -> tuple[str, ...]:
    """Read singular and plural candidate focus fields without ambiguity."""
    has_singular = "focus_symbol" in value
    has_plural = "focus_symbols" in value
    if has_singular and has_plural:
        _fail(f"{label} cannot contain both focus_symbol and focus_symbols")
    if has_plural:
        return _focus_symbols(value.get("focus_symbols"), f"{label}.focus_symbols")
    if has_singular:
        return _focus_symbols(value.get("focus_symbol"), f"{label}.focus_symbol")
    return _focus_symbols(None, label, default=default)


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label)
    if SAFE_ID_RE.fullmatch(result) is None:
        _fail(f"{label} must use 1-128 letters, digits, dot, underscore, or dash")
    return result


def _integer(value: Any, label: str, *, minimum: int = 0, maximum: int | None = None) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        _fail(f"{label} must be an integer >= {minimum}")
    if maximum is not None and value > maximum:
        _fail(f"{label} must be <= {maximum}")
    return value


def _seconds(value: Any, label: str) -> float | None:
    if value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        _fail(f"{label} must be numeric")
    try:
        result = float(value)
    except (OverflowError, ValueError) as exc:
        raise MatchError(f"{label} must be finite and non-negative") from exc
    if not math.isfinite(result) or result < 0:
        _fail(f"{label} must be finite and non-negative")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label).lower()
    if SHA256_RE.fullmatch(result) is None:
        _fail(f"{label} must be a lowercase SHA-256 digest")
    return result


def _resolve(value: str | os.PathLike[str], base: Path) -> Path:
    raw = os.fspath(value)
    if not raw or "\x00" in raw:
        _fail("path must be non-empty and contain no NUL")
    path = Path(raw).expanduser()
    return Path(os.path.abspath(path if path.is_absolute() else base / path))


def _normalized_compile_input_path(value: str | os.PathLike[str]) -> str:
    """Return the stable lexical identity used for a compiler source input."""
    absolute = os.path.abspath(os.fspath(value))
    normalized = os.path.normcase(os.path.normpath(absolute))
    return Path(normalized).as_posix()


_ARTIFACT_ROLE_CANDIDATE = "candidate"
_ARTIFACT_ROLE_TARGET = "target"
_ARTIFACT_ROLE_UNCLASSIFIED = "unclassified"


def _artifact_role(path: Path) -> tuple[str, str]:
    """Classify a matching artifact from its canonical source/build layout."""
    absolute = Path(os.path.abspath(path))
    parts = tuple(part.casefold() for part in absolute.parts)
    for build_index, component in enumerate(parts):
        if component != "build" or build_index + 2 >= len(parts):
            continue
        variant = parts[build_index + 1]
        obj_index = next(
            (
                index
                for index in range(build_index + 2, len(parts))
                if parts[index] == "obj"
            ),
            None,
        )
        if obj_index is not None:
            return (
                _ARTIFACT_ROLE_TARGET,
                f"build/{variant}/.../obj/... (component {obj_index - build_index} after build)",
            )
    if "src" in parts:
        return (_ARTIFACT_ROLE_CANDIDATE, "path contains a src component")
    return (_ARTIFACT_ROLE_UNCLASSIFIED, "no canonical src or build/.../obj marker")


def _validate_candidate_artifact_path(path: Path, label: str) -> str:
    """Reject extracted-target paths at every candidate/donor ingestion point."""
    role, evidence = _artifact_role(path)
    if role == _ARTIFACT_ROLE_TARGET:
        _fail(
            f"{label} has target role ({evidence}; extracted-target path {path}); "
            "candidate/donor role is forbidden here. Use the compiled candidate "
            "artifact under a .../src/... path instead."
        )
    return role


def _validate_candidate_artifact(
    path: Path,
    snapshot: Mapping[str, Any],
    session: Mapping[str, Any],
    label: str,
) -> str:
    """Reject target-layout paths and target-role CAS bytes as candidates."""
    role = _validate_candidate_artifact_path(path, label)
    target_hashes = {
        str(session["request"]["target"]["sha256"]),
        str(session["target_blob"]["sha256"]),
    }
    if str(snapshot["sha256"]) in target_hashes:
        _fail(
            f"{label} has candidate/donor path role {role}, but SHA-256 "
            f"{snapshot['sha256']} is already registered with target role in "
            "the session CAS; candidate/donor recording requires a distinct "
            "compiled artifact."
        )
    return role


def _assert_no_indirection(path: Path, *, allow_missing_leaf: bool = False) -> None:
    absolute = Path(os.path.abspath(path))
    parts = absolute.parts
    current = Path(parts[0])
    for index, part in enumerate(parts):
        if index:
            current = current / part
        try:
            info = current.lstat()
        except FileNotFoundError:
            if allow_missing_leaf and index == len(parts) - 1:
                return
            raise MatchError(f"path component does not exist: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            _fail(f"path indirection is forbidden: {current}")


def _safe_mkdir(path: Path) -> None:
    """Create a directory one component at a time, rejecting reparse parents."""
    absolute = Path(os.path.abspath(path))
    parts = absolute.parts
    current = Path(parts[0])
    for index, part in enumerate(parts):
        if index:
            current = current / part
        try:
            info = current.lstat()
        except FileNotFoundError:
            try:
                current.mkdir()
            except FileExistsError:
                info = current.lstat()
            else:
                continue
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            _fail(f"path indirection is forbidden: {current}")
        if not stat.S_ISDIR(info.st_mode):
            _fail(f"directory path is not a directory: {current}")


def _safe_parent(path: Path) -> None:
    _safe_mkdir(path.parent)


class _PinnedTargetParent:
    """Hold the target parent against path replacement for a repair.

    A final ``lstat`` immediately before ``os.replace`` is not sufficient on
    Windows: another process can replace the parent with a junction between
    that check and the path-based rename.  Windows directory handles opened
    without ``FILE_SHARE_DELETE`` prevent that replacement.  POSIX hosts use
    a directory descriptor and perform the rename relative to that descriptor
    so a path swap cannot redirect the write.
    """

    def __init__(
        self,
        target: Path,
        *,
        expected_parent_identity: tuple[int, int] | None = None,
    ) -> None:
        self.target = Path(os.path.abspath(target))
        self.parent = self.target.parent
        self.fd: int | None = None
        self._handles: list[Any] = []
        self._kernel32: Any = None
        self._parent_identity: tuple[int, int] | None = None
        self._expected_parent_identity = expected_parent_identity

    def __enter__(self) -> "_PinnedTargetParent":
        try:
            _safe_parent(self.target)
            _assert_no_indirection(self.parent)
            current_identity = _directory_identity(
                self.parent, "session target parent"
            )
            if (
                self._expected_parent_identity is not None
                and current_identity != self._expected_parent_identity
            ):
                _fail(f"session target parent changed before pin: {self.parent}")
            if os.name == "nt":
                self._open_windows()
            else:
                self._open_posix()
            if (
                self._expected_parent_identity is not None
                and self._parent_identity != self._expected_parent_identity
            ):
                _fail(f"session target parent changed before pin: {self.parent}")
            self.verify()
            return self
        except BaseException:
            self.close()
            raise

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> None:
        self.close()

    def _open_posix(self) -> None:
        flags = os.O_RDONLY
        if hasattr(os, "O_DIRECTORY"):
            flags |= os.O_DIRECTORY
        if hasattr(os, "O_NOFOLLOW"):
            flags |= os.O_NOFOLLOW
        try:
            self.fd = os.open(self.parent, flags)
        except OSError as exc:
            raise MatchError(f"cannot pin session target parent {self.parent}: {exc}") from exc
        info = os.fstat(self.fd)
        if not stat.S_ISDIR(info.st_mode):
            _fail(f"session target parent is not a directory: {self.parent}")
        self._parent_identity = (info.st_dev, info.st_ino)

    def _open_windows(self) -> None:
        import ctypes
        from ctypes import wintypes

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel32.CreateFileW.argtypes = [
            wintypes.LPCWSTR,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.HANDLE,
        ]
        kernel32.CreateFileW.restype = wintypes.HANDLE
        kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
        kernel32.CloseHandle.restype = wintypes.BOOL
        self._kernel32 = kernel32

        # GENERIC_READ is required for Windows to enforce the no-delete share
        # on directory rename.  FILE_READ_ATTRIBUTES alone does not pin a
        # directory against MoveFileEx/ junction replacement.
        generic_read = 0x80000000
        share_read = 0x00000001
        share_write = 0x00000002
        open_existing = 3
        backup_semantics = 0x02000000
        open_reparse_point = 0x00200000
        invalid_handle = ctypes.c_void_p(-1).value

        parts = self.parent.parts
        current = Path(parts[0])
        try:
            for index, part in enumerate(parts):
                if index:
                    current = current / part
                handle = kernel32.CreateFileW(
                    os.fspath(current),
                    generic_read,
                    share_read | share_write,
                    None,
                    open_existing,
                    backup_semantics | open_reparse_point,
                    None,
                )
                if handle in (None, invalid_handle):
                    error = ctypes.get_last_error()
                    raise MatchError(
                        f"cannot pin session target parent component {current}: "
                        f"WinError {error}"
                    )
                self._handles.append(handle)
                try:
                    info = current.lstat()
                except OSError as exc:
                    raise MatchError(
                        f"cannot verify session target parent component {current}: {exc}"
                    ) from exc
                if not stat.S_ISDIR(info.st_mode):
                    _fail(f"session target parent component is not a directory: {current}")
                if bool(getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE):
                    _fail(f"path indirection is forbidden: {current}")
        except BaseException:
            self.close()
            raise
        info = self.parent.lstat()
        self._parent_identity = (info.st_dev, info.st_ino)

    def verify(self) -> None:
        """Verify that the named parent still denotes the pinned directory."""
        _assert_no_indirection(self.parent)
        try:
            info = self.parent.lstat()
        except OSError as exc:
            raise MatchError(f"session target parent changed during repair: {self.parent}") from exc
        if not stat.S_ISDIR(info.st_mode):
            _fail(f"session target parent is not a directory: {self.parent}")
        identity = (info.st_dev, info.st_ino)
        if self._parent_identity is not None and identity != self._parent_identity:
            _fail(f"session target parent changed during repair: {self.parent}")
        if self.fd is not None:
            current = os.fstat(self.fd)
            if (current.st_dev, current.st_ino) != identity:
                _fail(f"session target parent changed during repair: {self.parent}")

    def close(self) -> None:
        if self.fd is not None:
            try:
                os.close(self.fd)
            finally:
                self.fd = None
        if self._handles and self._kernel32 is not None:
            for handle in reversed(self._handles):
                self._kernel32.CloseHandle(handle)
            self._handles.clear()


@contextlib.contextmanager
def _temporary_file_in_pinned_parent(
    parent: _PinnedTargetParent, target_name: str
) -> Any:
    """Create a repair temporary in the pinned directory.

    POSIX uses ``openat`` semantics through the pinned descriptor.  Windows
    uses the ordinary path only while all parent components are held open with
    delete sharing denied, so a junction/path swap cannot redirect creation.
    """
    temporary: str | Path | None = None
    stream: Any = None
    if parent.fd is not None:
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        if hasattr(os, "O_BINARY"):
            flags |= os.O_BINARY
        names = tempfile._get_candidate_names()
        for _ in range(100):
            name = f".{target_name}.{next(names)}.tmp"
            try:
                fd = os.open(name, flags, 0o600, dir_fd=parent.fd)
            except FileExistsError:
                continue
            stream = os.fdopen(fd, "wb")
            temporary = name
            break
        if stream is None or temporary is None:
            _fail(f"cannot create repair temporary in {parent.parent}")
    else:
        stream = tempfile.NamedTemporaryFile(
            "wb", dir=parent.parent, prefix=f".{target_name}.", suffix=".tmp", delete=False
        )
        temporary = Path(stream.name)
    try:
        yield temporary, stream
    finally:
        if stream is not None and not stream.closed:
            stream.close()
        if temporary is not None:
            try:
                if parent.fd is not None:
                    os.unlink(temporary, dir_fd=parent.fd)
                else:
                    temporary.unlink()
            except FileNotFoundError:
                pass
            except OSError:
                # Keep the original repair failure.  The temporary remains
                # recoverable for a caller to clean up after the parent is
                # released, and no authority is advanced by this path.
                pass


def _snapshot(path: Path, label: str) -> dict[str, Any]:
    _assert_no_indirection(path)
    try:
        before_link = path.lstat()
        flags = os.O_RDONLY
        if hasattr(os, "O_BINARY"):
            flags |= os.O_BINARY
        handle = os.open(os.fspath(path), flags)
    except OSError as exc:
        raise MatchError(f"cannot open {label}: {path}: {exc}") from exc
    try:
        before = os.fstat(handle)
        if not stat.S_ISREG(before.st_mode):
            _fail(f"{label} is not a regular file: {path}")
        if before.st_nlink != 1:
            _fail(f"{label} must have exactly one hard link: {path}")
        if (before.st_dev, before.st_ino, before.st_nlink) != (
            before_link.st_dev,
            before_link.st_ino,
            before_link.st_nlink,
        ):
            _fail(f"{label} changed before it was read: {path}")
        digest = hashlib.sha256()
        size = 0
        with os.fdopen(handle, "rb", closefd=False) as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
                size += len(block)
        after = os.fstat(handle)
        after_link = path.lstat()
    finally:
        os.close(handle)
    identity = (before.st_dev, before.st_ino, before.st_nlink, before.st_size, before.st_mtime_ns)
    current = (after.st_dev, after.st_ino, after.st_nlink, after.st_size, after.st_mtime_ns)
    if identity != current:
        _fail(f"{label} changed while it was read: {path}")
    if (after_link.st_dev, after_link.st_ino, after_link.st_nlink) != (
        before.st_dev,
        before.st_ino,
        before.st_nlink,
    ):
        _fail(f"{label} changed after it was read: {path}")
    return {
        "path": os.fspath(path),
        "size_bytes": size,
        "sha256": digest.hexdigest(),
        "identity": _file_identity_payload(before),
    }


def _stat_int(info: os.stat_result, name: str, default: int = 0) -> int:
    """Read optional platform stat fields as stable JSON integers."""
    value = getattr(info, name, default)
    if value is None:
        value = default
    try:
        return int(value)
    except (TypeError, ValueError, OverflowError) as exc:
        raise MatchError(f"stat field {name} is not an integer") from exc


def _file_identity_payload(info: os.stat_result) -> dict[str, int | str]:
    """Serialize persistent file/link/reparse identity without platform loss.

    ``st_ino`` is the persistent file id on POSIX and the file-index component
    on Windows.  Windows also exposes volume/file attributes and reparse tags
    through ``stat_result`` on supported Python versions; retaining those
    fields prevents a same-byte replacement or reparse-point substitution from
    being treated as the original compiler input.
    """
    device = _stat_int(info, "st_dev")
    inode = _stat_int(info, "st_ino")
    return {
        "device": device,
        "inode": inode,
        "file_id": f"{device}:{inode}",
        "nlink": _stat_int(info, "st_nlink"),
        "mode": _stat_int(info, "st_mode"),
        "mtime_ns": _stat_int(info, "st_mtime_ns"),
        "ctime_ns": _stat_int(info, "st_ctime_ns"),
        "file_attributes": _stat_int(info, "st_file_attributes"),
        "reparse_tag": _stat_int(info, "st_reparse_tag"),
    }


_FILE_IDENTITY_FIELDS = frozenset(
    {
        "device", "inode", "file_id", "nlink", "mode", "mtime_ns", "ctime_ns",
        "file_attributes", "reparse_tag",
    }
)


def _parse_file_identity(value: Any, label: str, *, complete: bool = False) -> dict[str, int | str]:
    """Validate a serialized file identity, accepting old two-field records."""
    if not isinstance(value, Mapping):
        _fail(f"{label} must be an object")
    required = {"device", "inode"}
    if complete:
        required = set(_FILE_IDENTITY_FIELDS)
    item = _closed(value, allowed=_FILE_IDENTITY_FIELDS, required=required, label=label)
    parsed: dict[str, int | str] = {
        "device": _integer(item["device"], f"{label}.device"),
        "inode": _integer(item["inode"], f"{label}.inode"),
    }
    if "file_id" in item:
        file_id = _text(item["file_id"], f"{label}.file_id")
        if file_id != f"{parsed['device']}:{parsed['inode']}":
            _fail(f"{label}.file_id is not bound to device/inode")
        parsed["file_id"] = file_id
    elif complete:
        _fail(f"{label}.file_id is missing")
    for key in _FILE_IDENTITY_FIELDS - {"device", "inode", "file_id"}:
        if key in item:
            parsed[key] = _integer(item[key], f"{label}.{key}")
        elif complete:
            _fail(f"{label}.{key} is missing")
    return parsed


def _file_identity_matches(actual: Mapping[str, Any], expected: Mapping[str, Any], label: str) -> None:
    parsed = _parse_file_identity(expected, label, complete=False)
    for key, value in parsed.items():
        if actual.get(key) != value:
            _fail(f"{label} identity changed from its authenticated identity")


def _recheck_live_snapshot(path: Path, expected: Mapping[str, Any], label: str) -> None:
    """Reject content changes and same-byte file replacement within one operation."""
    current = _snapshot(path, label)
    if current["size_bytes"] != expected.get("size_bytes") or current["sha256"] != expected.get("sha256"):
        _fail(f"{label} changed from its authenticated snapshot")
    expected_identity = expected.get("identity")
    if isinstance(expected_identity, Mapping) and current.get("identity") != expected_identity:
        _fail(f"{label} identity changed from its authenticated snapshot")


def _compile_input_identity(source_snapshot: Mapping[str, Any]) -> dict[str, str]:
    """Bind a context key only to a path authenticated by a live source snapshot."""
    if not isinstance(source_snapshot.get("identity"), Mapping):
        _fail("candidate source path lacks an authenticated file identity")
    source_path = _text(source_snapshot.get("path"), "candidate source.path")
    if not Path(source_path).is_absolute():
        _fail("candidate source path must be absolute before context binding")
    return {
        "schema": COMPILE_INPUT_SCHEMA,
        "normalized_path": _normalized_compile_input_path(source_path),
        "source_sha256": _sha256(source_snapshot.get("sha256"), "candidate source.sha256"),
    }


def _validate_compile_input_identity(
    value: Any, source: Mapping[str, Any], label: str = "candidate compile_input_identity"
) -> dict[str, str]:
    item = _closed(
        value,
        allowed={"schema", "normalized_path", "source_sha256"},
        required={"schema", "normalized_path", "source_sha256"},
        label=label,
    )
    if item["schema"] != COMPILE_INPUT_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    normalized_path = _text(item["normalized_path"], f"{label}.normalized_path")
    if normalized_path != _normalized_compile_input_path(normalized_path):
        _fail(f"{label}.normalized_path is not canonical")
    expected_path = _normalized_compile_input_path(
        _text(source.get("path"), "candidate source.path")
    )
    if normalized_path != expected_path:
        _fail(f"{label} is not bound to the candidate source path")
    source_sha = _sha256(item["source_sha256"], f"{label}.source_sha256")
    if source_sha != _sha256(source.get("sha256"), "candidate source.sha256"):
        _fail(f"{label} is not bound to the candidate source bytes")
    return {
        "schema": COMPILE_INPUT_SCHEMA,
        "normalized_path": normalized_path,
        "source_sha256": source_sha,
    }


def _validate_target_path(
    path: Path, *, allow_missing_leaf: bool = False, label: str = "target"
) -> None:
    """Validate target path identity without reading its content.

    Repair is allowed to observe a missing target, but it must never treat a
    symlink/reparse point, directory, or hard-link alias as a repair target.
    Normal artifact validation continues to use :func:`_snapshot`, which also
    authenticates the bytes and detects read-time changes.
    """
    _assert_no_indirection(path, allow_missing_leaf=allow_missing_leaf)
    try:
        info = path.lstat()
    except FileNotFoundError:
        if allow_missing_leaf:
            return
        raise MatchError(f"{label} does not exist: {path}") from None
    if not stat.S_ISREG(info.st_mode):
        _fail(f"{label} is not a regular file: {path}")
    if info.st_nlink != 1:
        _fail(f"{label} must have exactly one hard link: {path}")


def _directory_identity(path: Path, label: str) -> tuple[int, int]:
    """Return the authenticated identity of a non-indirected directory."""
    _assert_no_indirection(path)
    try:
        info = path.lstat()
    except OSError as exc:
        raise MatchError(f"cannot inspect {label}: {path}: {exc}") from exc
    if not stat.S_ISDIR(info.st_mode):
        _fail(f"{label} is not a directory: {path}")
    # Directory link counts are structural metadata, not a safe identity:
    # normal POSIX directories commonly have nlink >= 2 and the count changes
    # when child directories are added or removed.  Regular-file hard-link
    # checks remain enforced by _snapshot/_validate_target_path.
    return (info.st_dev, info.st_ino)


def _directory_identity_payload(identity: tuple[int, int]) -> dict[str, int]:
    return {"device": identity[0], "inode": identity[1]}


def _directory_full_identity(path: Path, label: str) -> dict[str, int | str]:
    """Return the full persistent identity for an authenticated directory."""
    _assert_no_indirection(path)
    try:
        info = path.lstat()
    except OSError as exc:
        raise MatchError(f"cannot inspect {label}: {path}: {exc}") from exc
    if not stat.S_ISDIR(info.st_mode):
        _fail(f"{label} is not a directory: {path}")
    return _file_identity_payload(info)


def _directory_full_identity_payload(path: Path, label: str) -> dict[str, int | str]:
    return _directory_full_identity(path, label)


def _directory_tree_name_fingerprint(path: Path, label: str) -> str:
    """Fingerprint the relative names and node kinds beneath an include root.

    This intentionally excludes file bytes (those are separately authenticated
    by ``compile_inputs``) but binds the compiler's name-resolution universe.
    Every visited component is checked for symlink/reparse indirection and the
    root is rescanned after enumeration to catch a concurrent replacement.
    """
    _assert_no_indirection(path)
    before = _directory_full_identity(path, f"{label} root")
    names: list[tuple[str, str]] = []
    pending = [path]
    while pending:
        current = pending.pop()
        _assert_no_indirection(current)
        try:
            entries = list(current.iterdir())
        except OSError as exc:
            raise MatchError(f"cannot enumerate {label}: {current}: {exc}") from exc
        for entry in entries:
            _assert_no_indirection(entry)
            try:
                info = entry.lstat()
            except OSError as exc:
                raise MatchError(f"cannot inspect {label} entry: {entry}: {exc}") from exc
            relative = entry.relative_to(path).as_posix()
            if stat.S_ISDIR(info.st_mode):
                names.append((relative, "dir"))
                pending.append(entry)
            elif stat.S_ISREG(info.st_mode):
                names.append((relative, "file"))
            else:
                _fail(f"{label} contains unsupported node: {entry}")
    names.sort()
    fingerprint = _sha256_bytes(_canonical(names))
    after = _directory_full_identity(path, f"{label} root")
    if before != after:
        _fail(f"{label} changed while its name tree was read")
    return fingerprint


def _canonical_path(path: Path, label: str) -> str:
    value = os.fspath(path)
    if not path.is_absolute() or os.path.abspath(value) != value:
        _fail(f"{label} must be an absolute canonical path")
    if os.path.normcase(os.path.normpath(value)) != os.path.normcase(value):
        _fail(f"{label} must be normalized")
    return value


def _directory_descriptor(
    value: Any,
    *,
    root: Path,
    label: str,
    tree_names: bool = False,
) -> dict[str, Any]:
    """Normalize a complete directory descriptor and its name-tree seal."""
    if isinstance(value, str):
        path = _resolve(value, root)
        supplied: Mapping[str, Any] = {}
    else:
        supplied = _closed(
            value,
            allowed={"path", "identity", "tree_name_fingerprint"},
            required={"path"},
            label=label,
        )
        path = _resolve(_text(supplied["path"], f"{label}.path"), root)
    path_text = _canonical_path(path, f"{label}.path")
    identity = _directory_full_identity_payload(path, label)
    supplied_identity: dict[str, int | str] | None = None
    if "identity" in supplied:
        expected = _parse_file_identity(supplied["identity"], f"{label}.identity", complete=False)
        _file_identity_matches(identity, expected, label)
        supplied_identity = expected
    fingerprint = _directory_tree_name_fingerprint(path, label) if tree_names else None
    if "tree_name_fingerprint" in supplied:
        supplied_fp = _sha256(supplied["tree_name_fingerprint"], f"{label}.tree_name_fingerprint")
        if fingerprint != supplied_fp:
            _fail(f"{label} name tree changed from its authenticated fingerprint")
    # Preserve a legacy two-field directory identity exactly when one was
    # supplied. The strict reuse predicate separately requires every full
    # identity field, so this compatibility path cannot authorize execution.
    result: dict[str, Any] = {
        "path": path_text,
        "identity": supplied_identity if supplied_identity is not None else identity,
    }
    if tree_names:
        result["tree_name_fingerprint"] = fingerprint
    return result


def _parse_directory_identity(value: Any, label: str) -> tuple[int, int]:
    item = _closed(
        value,
        allowed={"device", "inode"},
        required={"device", "inode"},
        label=label,
    )
    return (
        _integer(item["device"], f"{label}.device"),
        _integer(item["inode"], f"{label}.inode"),
    )


def _descriptor(value: Any, *, base: Path, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256", "identity"},
        required={"path", "size_bytes", "sha256"},
        label=label,
    )
    path = _resolve(_text(item["path"], f"{label}.path"), base)
    expected_size = _integer(item["size_bytes"], f"{label}.size_bytes")
    expected_sha = _sha256(item["sha256"], f"{label}.sha256")
    actual = _snapshot(path, label)
    if actual["size_bytes"] != expected_size or actual["sha256"] != expected_sha:
        _fail(f"descriptor mismatch for {label}: {path}")
    if "identity" in item:
        expected_identity = _parse_file_identity(item["identity"], f"{label}.identity", complete=False)
        _file_identity_matches(actual["identity"], expected_identity, label)
    result = {key: actual[key] for key in ("path", "size_bytes", "sha256")}
    # Preserve legacy descriptor shape when the caller supplied no identity;
    # complete contexts must explicitly provide it and are rejected by the
    # strict reuse predicate otherwise.
    if "identity" in item:
        result["identity"] = actual["identity"]
    return result


def descriptor(path: Path | str) -> dict[str, Any]:
    """Return an authenticated descriptor for a regular single-link file."""
    actual = _snapshot(Path(path).expanduser(), "artifact")
    return {
        key: actual[key] for key in ("path", "size_bytes", "sha256", "identity")
    }


def _frozen_target_descriptor(
    value: Any, *, base: Path, expected: Mapping[str, Any], label: str
) -> dict[str, Any]:
    """Validate a request target against a frozen descriptor without reading it.

    This narrowly scoped path is used only while repairing the live target.
    The request manifest, path shape, claimed size/SHA, and target identity are
    still authenticated; only the live target-content snapshot is deferred to
    the CAS-backed restore/check performed by ``repair_target``.
    """
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256", "identity"},
        required={"path", "size_bytes", "sha256"},
        label=label,
    )
    path = _resolve(_text(item["path"], f"{label}.path"), base)
    size = _integer(item["size_bytes"], f"{label}.size_bytes")
    sha = _sha256(item["sha256"], f"{label}.sha256")
    expected_path = _resolve(_text(expected["path"], f"{label}.expected_path"), base)
    expected_size = _integer(expected["size_bytes"], f"{label}.expected_size_bytes")
    expected_sha = _sha256(expected["sha256"], f"{label}.expected_sha256")
    if path != expected_path or size != expected_size or sha != expected_sha:
        _fail(f"frozen request target does not match {label}")
    _validate_target_path(path, allow_missing_leaf=True, label=label)
    result: dict[str, Any] = {"path": os.fspath(path), "size_bytes": size, "sha256": sha}
    if "identity" in item:
        expected_identity = _parse_file_identity(item["identity"], f"{label}.identity", complete=False)
        expected_identity_value = expected.get("identity")
        if isinstance(expected_identity_value, Mapping):
            _file_identity_matches(expected_identity_value, expected_identity, label)
        result["identity"] = dict(expected_identity)
    return result


def _with_self_hash(value: Mapping[str, Any], field: str) -> dict[str, Any]:
    result = copy.deepcopy(dict(value))
    result.pop(field, None)
    result[field] = _sha256_bytes(_canonical(result))
    return result


def _verify_self_hash(value: Any, field: str, label: str) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        _fail(f"{label} must be an object")
    claimed = _sha256(value.get(field), f"{label}.{field}")
    body = copy.deepcopy(value)
    body.pop(field, None)
    if _sha256_bytes(_canonical(body)) != claimed:
        _fail(f"{label} self-hash mismatch")
    return value


def _write_new(path: Path, payload: bytes) -> None:
    _safe_parent(path)
    if os.path.lexists(path):
        _assert_no_indirection(path)
    try:
        handle = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    except FileExistsError as exc:
        raise MatchError(f"immutable output already exists: {path}") from exc
    try:
        with os.fdopen(handle, "wb") as stream:
            handle = -1
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
    except BaseException:
        try:
            path.unlink()
        except OSError:
            pass
        raise
    finally:
        if handle >= 0:
            os.close(handle)


def _atomic_replace(path: Path, payload: bytes) -> None:
    _safe_parent(path)
    if os.path.lexists(path):
        _assert_no_indirection(path)
    with tempfile.NamedTemporaryFile(
        "wb", dir=path.parent, prefix=f".{path.name}.", suffix=".tmp", delete=False
    ) as stream:
        temporary = Path(stream.name)
        stream.write(payload)
        stream.flush()
        os.fsync(stream.fileno())
    try:
        os.replace(temporary, path)
    finally:
        if temporary.exists():
            temporary.unlink()


def _restore_target_from_cas(
    target: Path,
    cas_path: Path,
    expected: Mapping[str, Any],
    expected_parent_identity: tuple[int, int],
) -> dict[str, Any]:
    """Atomically restore a target from an authenticated CAS blob."""
    expected_sha = _sha256(expected.get("sha256"), "target restore.sha256")
    expected_size = _integer(expected.get("size_bytes"), "target restore.size_bytes")
    _validate_target_path(target, allow_missing_leaf=True, label="session target")

    cas_before = _snapshot(cas_path, "session target CAS")
    if (
        cas_before["sha256"] != expected_sha
        or cas_before["size_bytes"] != expected_size
    ):
        _fail("session target CAS does not match its frozen descriptor")

    digest = hashlib.sha256()
    size = 0
    with _PinnedTargetParent(
        target, expected_parent_identity=expected_parent_identity
    ) as parent:
        _validate_target_path(target, allow_missing_leaf=True, label="session target")
        with _temporary_file_in_pinned_parent(parent, target.name) as (temporary, stream):
            with cas_path.open("rb") as source:
                for block in iter(lambda: source.read(1024 * 1024), b""):
                    stream.write(block)
                    digest.update(block)
                    size += len(block)
            stream.flush()
            os.fsync(stream.fileno())
            stream.close()

            cas_after = _snapshot(cas_path, "session target CAS")
            if (
                cas_after["sha256"] != expected_sha
                or cas_after["size_bytes"] != expected_size
                or cas_after["sha256"] != cas_before["sha256"]
                or cas_after["size_bytes"] != cas_before["size_bytes"]
            ):
                _fail("session target CAS changed during repair")
            if digest.hexdigest() != expected_sha or size != expected_size:
                _fail("session target CAS bytes failed repair verification")

            # The parent is pinned for the entire copy and rename.  This
            # verification is useful on POSIX when a directory was renamed
            # concurrently; Windows' no-delete directory handles prevent the
            # replacement itself.
            parent.verify()
            _validate_target_path(target, allow_missing_leaf=True, label="session target")
            try:
                if parent.fd is not None:
                    os.replace(
                        temporary,
                        target.name,
                        src_dir_fd=parent.fd,
                        dst_dir_fd=parent.fd,
                    )
                else:
                    os.replace(temporary, target)
            except (NotImplementedError, TypeError) as exc:
                _fail(f"cannot perform pinned session target replacement: {exc}")
            _validate_target_path(target, label="restored session target")
            restored = _snapshot(target, "restored session target")
            if (
                restored["sha256"] != expected_sha
                or restored["size_bytes"] != expected_size
            ):
                _fail("restored session target does not match its frozen descriptor")
            return {
                key: restored[key] for key in ("path", "size_bytes", "sha256")
            }


@contextlib.contextmanager
def _workbench_lock(path: Path, timeout_seconds: float) -> Any:
    _safe_parent(path)
    if os.path.lexists(path):
        _assert_no_indirection(path)
    flags = os.O_RDWR | os.O_CREAT
    if hasattr(os, "O_BINARY"):
        flags |= os.O_BINARY
    try:
        handle = os.open(path, flags, 0o600)
    except OSError as exc:
        raise MatchError(f"cannot open workbench lock {path}: {exc}") from exc
    stream = os.fdopen(handle, "a+b", closefd=True)
    deadline = time.monotonic() + timeout_seconds
    locked = False
    try:
        while not locked:
            try:
                if os.name == "nt":
                    import msvcrt

                    stream.seek(0)
                    mode = msvcrt.LK_NBLCK if time.monotonic() < deadline else msvcrt.LK_LOCK
                    msvcrt.locking(stream.fileno(), mode, 1)
                else:
                    import fcntl

                    fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                locked = True
            except (BlockingIOError, OSError):
                if time.monotonic() >= deadline:
                    _fail(f"timed out waiting for workbench lock: {path}")
                time.sleep(0.02)
        before = os.fstat(stream.fileno())
        before_path = path.lstat()
        if (before.st_dev, before.st_ino) != (before_path.st_dev, before_path.st_ino):
            _fail(f"workbench lock changed before acquisition: {path}")
        if before.st_nlink != 1 or before_path.st_nlink != 1:
            _fail(f"workbench lock must have exactly one hard link: {path}")
        yield
    finally:
        if locked:
            try:
                if os.name == "nt":
                    import msvcrt

                    stream.seek(0)
                    msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(stream.fileno(), fcntl.LOCK_UN)
            finally:
                stream.close()
        else:
            stream.close()


def _workspace(value: Path | str, root: Path) -> Path:
    path = _resolve(value, root)
    try:
        path.relative_to(root.resolve())
    except ValueError:
        _fail(f"workspace must stay beneath the selected repository: {path}")
    return path


def _empty_index(session_sha256: str) -> dict[str, Any]:
    return _with_self_hash({
        "schema": INDEX_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "session_sha256": session_sha256,
        "sequence": 0,
        "candidates": {},
        "source_context_index": {},
        "object_index": {},
        "diagnostic_index": {},
        "last_record_sha256": None,
    }, "index_sha256")


def _recheck_descriptor(value: Mapping[str, Any], label: str) -> None:
    path = Path(str(value["path"]))
    _canonical_path(path, f"{label}.path")
    current = _snapshot(path, label)
    if current["size_bytes"] != value["size_bytes"] or current["sha256"] != value["sha256"]:
        _fail(f"{label} changed from its authenticated descriptor")
    expected_identity = value.get("identity")
    if expected_identity is not None:
        _file_identity_matches(current["identity"], expected_identity, label)


def _compile_cwd_descriptor(value: Any, *, root: Path, label: str) -> dict[str, Any] | None:
    """Resolve and bind the directory identity used by the compiler process.

    Relative include and output arguments make the working directory part of
    the compiler input. A byte-complete header list is therefore not a
    complete compile context unless the real cwd is frozen too.
    """

    if value is None:
        return None
    if not isinstance(value, (str, Mapping)) or not value:
        _fail(f"{label} must be a non-empty path string/object or null")
    if isinstance(value, str):
        # The v3 extension accepted a bare cwd path and persisted only the
        # legacy device/inode pair. Keep that shape readable; a complete
        # executable context must use the object form with full identity.
        path = _resolve(value, root)
        identity = _directory_identity(path, label)
        return {"path": os.fspath(path), "identity": _directory_identity_payload(identity)}
    return _directory_descriptor(value, root=root, label=label, tree_names=False)


def _recheck_compile_cwd(value: Any, label: str) -> None:
    item = _closed(
        value,
        allowed={"path", "identity", "tree_name_fingerprint"},
        required={"path", "identity"},
        label=label,
    )
    path_value = _text(item["path"], f"{label}.path")
    path = Path(path_value)
    _canonical_path(path, f"{label}.path")
    expected = _parse_file_identity(item["identity"], f"{label}.identity", complete=False)
    actual = _directory_full_identity(path, label)
    try:
        _file_identity_matches(actual, expected, label)
    except MatchError as exc:
        _fail(f"{label} changed from its authenticated identity: {exc}")


def _descriptor_has_complete_identity(value: Any, label: str) -> bool:
    if not isinstance(value, Mapping) or not isinstance(value.get("identity"), Mapping):
        return False
    try:
        _parse_file_identity(value["identity"], f"{label}.identity", complete=True)
    except (MatchError, KeyError, TypeError):
        return False
    return True


def _normalize_build_rule(value: Any, *, root: Path, label: str) -> dict[str, Any]:
    if isinstance(value, str):
        _fail(f"{label} must include an authenticated descriptor")
    return _descriptor(value, base=root, label=label)


def _normalize_runtime_dlls(value: Any, *, root: Path, label: str) -> list[dict[str, Any]]:
    if not isinstance(value, list) or len(value) == 0:
        _fail(f"{label} must be a non-empty descriptor array")
    result = [
        _descriptor(item, base=root, label=f"{label}[{index}]")
        for index, item in enumerate(value)
    ]
    paths = [item["path"] for item in result]
    if len(set(paths)) != len(paths):
        _fail(f"{label} contains duplicate paths")
    return result


def _normalize_include_roots(value: Any, *, root: Path, label: str) -> list[dict[str, Any]]:
    if not isinstance(value, list) or len(value) == 0:
        _fail(f"{label} must be a non-empty directory array")
    result = [
        _directory_descriptor(
            item,
            root=root,
            label=f"{label}[{index}]",
            tree_names=True,
        )
        for index, item in enumerate(value)
    ]
    paths = [item["path"] for item in result]
    if len(set(paths)) != len(paths):
        _fail(f"{label} contains duplicate paths")
    return result


def _normalize_output_binding(value: Any, *, root: Path, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"schema", "output", "depfile"},
        required={"output", "depfile"},
        label=label,
    )
    if item.get("schema", OUTPUT_BINDING_SCHEMA) != OUTPUT_BINDING_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    output = _descriptor(item["output"], base=root, label=f"{label}.output")
    depfile = _descriptor(item["depfile"], base=root, label=f"{label}.depfile")
    if output["path"] == depfile["path"]:
        _fail(f"{label} output and depfile must be distinct")
    return {
        "schema": OUTPUT_BINDING_SCHEMA,
        "output": output,
        "depfile": depfile,
    }


def _normalize_environment(value: Any, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"schema", "variables", "codepage", "locale"},
        required={"variables", "codepage", "locale"},
        label=label,
    )
    if item.get("schema", ENVIRONMENT_SCHEMA) != ENVIRONMENT_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    variables = item["variables"]
    if not isinstance(variables, Mapping):
        _fail(f"{label}.variables must be an object")
    normalized_variables: dict[str, str] = {}
    for key, raw_value in variables.items():
        name = _text(key, f"{label}.variables key")
        if "\x00" in name or "=" in name:
            _fail(f"{label}.variables contains an invalid name")
        normalized_variables[name] = _text(raw_value, f"{label}.variables[{name}]")
    codepage = item["codepage"]
    if isinstance(codepage, bool) or not isinstance(codepage, (str, int)):
        _fail(f"{label}.codepage must be a string or integer")
    codepage_text = str(codepage)
    locale_name = _text(item["locale"], f"{label}.locale")
    if "\x00" in locale_name:
        _fail(f"{label}.locale contains NUL")
    return {
        "schema": ENVIRONMENT_SCHEMA,
        "variables": dict(sorted(normalized_variables.items())),
        "codepage": codepage_text,
        "locale": locale_name,
    }


def _current_environment() -> dict[str, Any]:
    try:
        current_locale = locale.setlocale(locale.LC_CTYPE)
    except locale.Error:
        current_locale = ""
    return {
        "variables": dict(os.environ),
        "codepage": locale.getpreferredencoding(False),
        "locale": current_locale,
    }


def _recheck_environment(value: Any, label: str) -> None:
    item = _normalize_environment(value, label)
    current = _current_environment()
    for name, expected in item["variables"].items():
        if os.environ.get(name) != expected:
            _fail(f"{label}.variables[{name}] changed from its authenticated value")
    if str(current["codepage"]) != item["codepage"]:
        _fail(f"{label}.codepage changed from its authenticated value")
    if current["locale"] != item["locale"]:
        _fail(f"{label}.locale changed from its authenticated value")


def _normalize_dependency_provenance(
    value: Any,
    *,
    root: Path,
    compile_inputs: Sequence[Mapping[str, Any]],
    label: str,
) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"schema", "fresh", "depfile", "input_paths", "path_set_sha256"},
        required={"fresh", "depfile", "input_paths", "path_set_sha256"},
        label=label,
    )
    if item.get("schema", DEPENDENCY_PROVENANCE_SCHEMA) != DEPENDENCY_PROVENANCE_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    if item["fresh"] is not True:
        _fail(f"{label}.fresh must be true for executable reuse")
    depfile = _descriptor(item["depfile"], base=root, label=f"{label}.depfile")
    raw_paths = item["input_paths"]
    if not isinstance(raw_paths, list) or not raw_paths:
        _fail(f"{label}.input_paths must be a non-empty array")
    normalized_paths: list[str] = []
    for index, raw in enumerate(raw_paths):
        if isinstance(raw, Mapping):
            descriptor_value = _descriptor(raw, base=root, label=f"{label}.input_paths[{index}]")
            normalized_paths.append(descriptor_value["path"])
        else:
            path = _resolve(_text(raw, f"{label}.input_paths[{index}]"), root)
            normalized_paths.append(_canonical_path(path, f"{label}.input_paths[{index}]") )
    normalized_paths = sorted(normalized_paths)
    if len(set(normalized_paths)) != len(normalized_paths):
        _fail(f"{label}.input_paths contains duplicate paths")
    compile_paths = sorted(str(item["path"]) for item in compile_inputs)
    if normalized_paths != compile_paths:
        _fail(f"{label}.input_paths is not the exact compile input path set")
    path_set_sha = _sha256_bytes(_canonical(normalized_paths))
    supplied_sha = _sha256(item["path_set_sha256"], f"{label}.path_set_sha256")
    if supplied_sha != path_set_sha:
        _fail(f"{label}.path_set_sha256 does not match input_paths")
    return {
        "schema": DEPENDENCY_PROVENANCE_SCHEMA,
        "fresh": True,
        "depfile": depfile,
        "input_paths": normalized_paths,
        "path_set_sha256": path_set_sha,
    }


def _normalize_source_tree(value: Any, *, root: Path, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"schema", "root", "state", "dirty_patch"},
        required={"root", "state", "dirty_patch"},
        label=label,
    )
    if item.get("schema", SOURCE_TREE_SCHEMA) != SOURCE_TREE_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    tree_root = _directory_descriptor(item["root"], root=root, label=f"{label}.root", tree_names=True)
    state = _text(item["state"], f"{label}.state")
    if state not in {"clean", "dirty"}:
        _fail(f"{label}.state must be clean or dirty")
    dirty_patch = item["dirty_patch"]
    if state == "clean":
        if dirty_patch is not None:
            _fail(f"{label}.dirty_patch must be null for a clean tree")
    else:
        if dirty_patch is None:
            _fail(f"{label}.dirty_patch is required for a dirty tree")
        dirty_patch = _descriptor(dirty_patch, base=root, label=f"{label}.dirty_patch")
    return {
        "schema": SOURCE_TREE_SCHEMA,
        "root": tree_root,
        "state": state,
        "dirty_patch": dirty_patch,
    }


def _normalize_argv_binding(
    value: Any,
    *,
    argv: Sequence[str],
    root: Path,
    compile_cwd: Mapping[str, Any] | None,
    label: str,
) -> dict[str, Any] | None:
    response_args = [arg for arg in argv if arg.startswith("@")]
    if not response_args and value is None:
        return None
    if value is None:
        _fail(f"{label} is required when compile_argv contains a response-file argument")
    item = _closed(
        value,
        allowed={"schema", "expanded", "expanded_argv", "response_files"},
        required={"expanded", "expanded_argv", "response_files"},
        label=label,
    )
    if item.get("schema", ARGV_BINDING_SCHEMA) != ARGV_BINDING_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    if item["expanded"] is not True:
        _fail(f"{label}.expanded must be true")
    expanded = item["expanded_argv"]
    if not isinstance(expanded, list) or not all(isinstance(arg, str) and "\x00" not in arg for arg in expanded):
        _fail(f"{label}.expanded_argv must be a string array")
    if any(arg.startswith("@") for arg in expanded):
        _fail(f"{label}.expanded_argv still contains a response-file argument")
    response_files = item["response_files"]
    if not isinstance(response_files, list):
        _fail(f"{label}.response_files must be an array")
    normalized_files = [
        _descriptor(item, base=root, label=f"{label}.response_files[{index}]")
        for index, item in enumerate(response_files)
    ]
    if len({item["path"] for item in normalized_files}) != len(normalized_files):
        _fail(f"{label}.response_files contains duplicate paths")
    base = root
    if isinstance(compile_cwd, Mapping):
        base = Path(str(compile_cwd["path"]))
    expected_files: list[str] = []
    for arg in response_args:
        raw_path = arg[1:]
        expected_files.append(_canonical_path(_resolve(raw_path, base), f"{label} response path"))
    if sorted(expected_files) != sorted(item["path"] for item in normalized_files):
        _fail(f"{label}.response_files is not bound to compile_argv")
    return {
        "schema": ARGV_BINDING_SCHEMA,
        "expanded": True,
        "expanded_argv": list(expanded),
        "response_files": normalized_files,
    }


def _recheck_include_roots(value: Any, label: str) -> None:
    roots = value
    if not isinstance(roots, list) or not roots:
        _fail(f"{label} must be a non-empty directory array")
    seen: set[str] = set()
    for index, raw in enumerate(roots):
        item = _closed(
            raw,
            allowed={"path", "identity", "tree_name_fingerprint"},
            required={"path", "identity", "tree_name_fingerprint"},
            label=f"{label}[{index}]",
        )
        path = Path(_text(item["path"], f"{label}[{index}].path"))
        _canonical_path(path, f"{label}[{index}].path")
        if os.fspath(path) in seen:
            _fail(f"{label} contains duplicate paths")
        seen.add(os.fspath(path))
        expected = _parse_file_identity(item["identity"], f"{label}[{index}].identity", complete=False)
        actual = _directory_full_identity(path, f"{label}[{index}]")
        _file_identity_matches(actual, expected, f"{label}[{index}]")
        fingerprint = _sha256(item["tree_name_fingerprint"], f"{label}[{index}].tree_name_fingerprint")
        if _directory_tree_name_fingerprint(path, f"{label}[{index}]") != fingerprint:
            _fail(f"{label}[{index}] name tree changed from its authenticated fingerprint")


def _recheck_source_tree(value: Any, label: str) -> None:
    item = _closed(
        value,
        allowed={"schema", "root", "state", "dirty_patch"},
        required={"root", "state", "dirty_patch"},
        label=label,
    )
    if item.get("schema", SOURCE_TREE_SCHEMA) != SOURCE_TREE_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    root_item = item["root"]
    _recheck_include_roots([root_item], f"{label}.root")
    state = _text(item["state"], f"{label}.state")
    if state not in {"clean", "dirty"}:
        _fail(f"{label}.state must be clean or dirty")
    patch = item["dirty_patch"]
    if state == "clean":
        if patch is not None:
            _fail(f"{label}.dirty_patch must be null for a clean tree")
    else:
        if not isinstance(patch, Mapping):
            _fail(f"{label}.dirty_patch is required for a dirty tree")
        _closed(
            patch,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}.dirty_patch",
        )
        _recheck_descriptor(patch, f"{label}.dirty_patch")


def _recheck_output_binding(value: Any, label: str) -> None:
    item = _closed(
        value,
        allowed={"schema", "output", "depfile"},
        required={"output", "depfile"},
        label=label,
    )
    if item.get("schema", OUTPUT_BINDING_SCHEMA) != OUTPUT_BINDING_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    for key in ("output", "depfile"):
        descriptor_value = item[key]
        _closed(
            descriptor_value,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}.{key}",
        )
        _canonical_path(Path(str(descriptor_value["path"])), f"{label}.{key}.path")
        _recheck_descriptor(descriptor_value, f"{label}.{key}")
    if item["output"]["path"] == item["depfile"]["path"]:
        _fail(f"{label} output and depfile must be distinct")


def _recheck_dependency_provenance(value: Any, compile_inputs: Sequence[Mapping[str, Any]], label: str) -> None:
    item = _closed(
        value,
        allowed={"schema", "fresh", "depfile", "input_paths", "path_set_sha256"},
        required={"fresh", "depfile", "input_paths", "path_set_sha256"},
        label=label,
    )
    if item.get("schema", DEPENDENCY_PROVENANCE_SCHEMA) != DEPENDENCY_PROVENANCE_SCHEMA:
        _fail(f"{label}.schema is unsupported")
    if item["fresh"] is not True:
        _fail(f"{label}.fresh must be true")
    depfile = item["depfile"]
    _closed(
        depfile,
        allowed={"path", "size_bytes", "sha256", "identity"},
        required={"path", "size_bytes", "sha256"},
        label=f"{label}.depfile",
    )
    _recheck_descriptor(depfile, f"{label}.depfile")
    paths = item["input_paths"]
    if not isinstance(paths, list) or not all(isinstance(path, str) for path in paths):
        _fail(f"{label}.input_paths must be canonical string paths")
    canonical_paths = sorted(_canonical_path(Path(path), f"{label}.input_paths") for path in paths)
    if len(set(canonical_paths)) != len(canonical_paths):
        _fail(f"{label}.input_paths contains duplicate paths")
    compile_paths = sorted(str(item["path"]) for item in compile_inputs)
    if canonical_paths != compile_paths:
        _fail(f"{label}.input_paths is not the exact compile input path set")
    supplied = _sha256(item["path_set_sha256"], f"{label}.path_set_sha256")
    if supplied != _sha256_bytes(_canonical(canonical_paths)):
        _fail(f"{label}.path_set_sha256 does not match input_paths")


def _recheck_argv_binding(
    value: Any,
    argv: Sequence[str],
    label: str,
    compile_cwd: Mapping[str, Any] | None = None,
) -> None:
    item = _closed(
        value,
        allowed={"schema", "expanded", "expanded_argv", "response_files"},
        required={"expanded", "expanded_argv", "response_files"},
        label=label,
    )
    if item.get("schema", ARGV_BINDING_SCHEMA) != ARGV_BINDING_SCHEMA or item["expanded"] is not True:
        _fail(f"{label} is not an authenticated expanded argv binding")
    expanded = item["expanded_argv"]
    if not isinstance(expanded, list) or any(not isinstance(arg, str) or arg.startswith("@") for arg in expanded):
        _fail(f"{label}.expanded_argv is invalid")
    response_files = item["response_files"]
    if not isinstance(response_files, list):
        _fail(f"{label}.response_files is invalid")
    for index, descriptor_value in enumerate(response_files):
        _closed(
            descriptor_value,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}.response_files[{index}]",
        )
        _recheck_descriptor(descriptor_value, f"{label}.response_files[{index}]")
    if not any(arg.startswith("@") for arg in argv):
        _fail(f"{label} is present but compile_argv has no response-file argument")
    base = Path(str(compile_cwd["path"])) if isinstance(compile_cwd, Mapping) else Path.cwd()
    expected_paths = sorted(
        _canonical_path(_resolve(arg[1:], base), f"{label} response path")
        for arg in argv
        if arg.startswith("@")
    )
    actual_paths = sorted(str(item["path"]) for item in response_files)
    if expected_paths != actual_paths:
        _fail(f"{label}.response_files is not bound to compile_argv")


def _compile_context_complete(context: Mapping[str, Any]) -> bool:
    """Return whether a session has every field needed for compile reuse."""

    if context.get("context_complete") is not True:
        return False
    compiler = context.get("compiler")
    argv = context.get("compile_argv")
    if not (
        isinstance(compiler, Mapping)
        and isinstance(argv, list)
        and isinstance(context.get("compile_cwd"), Mapping)
        and isinstance(context.get("compile_tools"), list)
        and context.get("compile_tools")
        and isinstance(context.get("compile_inputs"), list)
        and context.get("compile_inputs")
        and isinstance(context.get("dependency_provenance"), Mapping)
        and isinstance(context.get("build_rule"), Mapping)
        and isinstance(context.get("include_roots"), list)
        and context.get("include_roots")
        and isinstance(context.get("environment"), Mapping)
        and isinstance(context.get("runtime_dlls"), list)
        and context.get("runtime_dlls")
        and isinstance(context.get("compile_outputs"), Mapping)
        and isinstance(context.get("source_tree"), Mapping)
    ):
        return False
    # Every file descriptor participating in compiler reuse must carry the
    # complete persistent identity; old v1/v2/v3 records intentionally fail
    # here even when their byte hashes still happen to match.
    if not _descriptor_has_complete_identity(compiler, "compile compiler"):
        return False
    try:
        _recheck_descriptor(compiler, "compile compiler")
    except (MatchError, KeyError, TypeError):
        return False
    for index, item in enumerate(context["compile_tools"]):
        if not _descriptor_has_complete_identity(item, f"compile tool {index}"):
            return False
        try:
            _recheck_descriptor(item, f"compile tool {index}")
        except (MatchError, KeyError, TypeError):
            return False
    for index, item in enumerate(context["compile_inputs"]):
        if not _descriptor_has_complete_identity(item, f"compile input {index}"):
            return False
        try:
            _recheck_descriptor(item, f"compile input {index}")
        except (MatchError, KeyError, TypeError):
            return False
    if not _descriptor_has_complete_identity(context["build_rule"], "compile build rule"):
        return False
    try:
        _recheck_descriptor(context["build_rule"], "compile build rule")
    except (MatchError, KeyError, TypeError):
        return False
    for index, item in enumerate(context["runtime_dlls"]):
        if not _descriptor_has_complete_identity(item, f"runtime DLL {index}"):
            return False
        try:
            _recheck_descriptor(item, f"runtime DLL {index}")
        except (MatchError, KeyError, TypeError):
            return False
    dependency = context["dependency_provenance"]
    if not isinstance(dependency.get("depfile"), Mapping) or not _descriptor_has_complete_identity(
        dependency["depfile"], "dependency depfile"
    ):
        return False
    outputs = context["compile_outputs"]
    if not all(
        _descriptor_has_complete_identity(outputs.get(key), f"compile output {key}")
        for key in ("output", "depfile")
    ):
        return False
    try:
        _recheck_descriptor(outputs["output"], "compile output output")
        _recheck_descriptor(outputs["depfile"], "compile output depfile")
    except (MatchError, KeyError, TypeError):
        return False
    source_tree = context["source_tree"]
    try:
        root_identity = source_tree["root"]["identity"]
        _parse_file_identity(root_identity, "source tree root.identity", complete=True)
        if source_tree.get("state") == "dirty":
            if not _descriptor_has_complete_identity(
                source_tree.get("dirty_patch"), "source tree dirty patch"
            ):
                return False
    except (KeyError, TypeError, MatchError):
        return False
    try:
        _parse_file_identity(context["compile_cwd"]["identity"], "compile cwd.identity", complete=True)
        for index, item in enumerate(context["include_roots"]):
            _parse_file_identity(
                item["identity"], f"compile include root {index}.identity", complete=True
            )
        _recheck_include_roots(context["include_roots"], "compile include roots")
        _recheck_dependency_provenance(context["dependency_provenance"], context["compile_inputs"], "compile dependency provenance")
        _recheck_output_binding(context["compile_outputs"], "compile outputs")
        _recheck_source_tree(context["source_tree"], "compile source tree")
        _recheck_environment(context["environment"], "compile environment")
    except (MatchError, KeyError, TypeError):
        return False
    response_args = [arg for arg in argv if isinstance(arg, str) and arg.startswith("@")]
    if response_args:
        binding = context.get("argv_binding")
        if not isinstance(binding, Mapping):
            return False
        try:
            _recheck_argv_binding(
                binding,
                argv,
                "compile argv binding",
                context.get("compile_cwd") if isinstance(context.get("compile_cwd"), Mapping) else None,
            )
            for index, item in enumerate(binding.get("response_files", [])):
                if not _descriptor_has_complete_identity(item, f"argv response file {index}"):
                    return False
        except (MatchError, KeyError, TypeError):
            return False
    elif context.get("argv_binding") is not None:
        return False
    return True


def _load_session(
    workspace: Path, root: Path, *, skip_live_target_check: bool = False
) -> Mapping[str, Any]:
    session = _load_json(workspace / "session.json", "session")
    _verify_self_hash(session, "session_sha256", "session")
    _closed(
        session,
        allowed={
            "schema", "schema_version", "session_id", "root", "workspace", "request",
            "request_manifest", "target_blob", "target_parent_identity",
            "authority_advanced", "session_sha256",
        },
        required={
            "schema", "schema_version", "session_id", "root", "workspace", "request",
            "request_manifest", "target_blob", "authority_advanced", "session_sha256",
        },
        label="session",
    )
    if session.get("schema") != SESSION_SCHEMA or session.get("schema_version") != 1:
        _fail("unsupported session schema")
    _identifier(session.get("session_id"), "session.session_id")
    if session.get("authority_advanced") is not False:
        _fail("session must not advance authority")
    if Path(str(session.get("root", ""))).resolve() != root.resolve():
        _fail("session belongs to a different repository root")
    if Path(str(session.get("workspace", ""))).resolve() != workspace.resolve():
        _fail("session is bound to a different workspace")
    request = session.get("request")
    if not isinstance(request, Mapping):
        _fail("session request is missing")
    _closed(
        request,
        allowed={
            "schema", "schema_version", "session_id", "owner", "unit", "function",
            "target", "context", "policy",
        },
        required={
            "schema", "schema_version", "session_id", "owner", "unit", "function",
            "target", "context", "policy",
        },
        label="session request",
    )
    if request.get("schema") != REQUEST_SCHEMA or request.get("schema_version") != 1:
        _fail("unsupported session request schema")
    if request.get("session_id") != session.get("session_id"):
        _fail("session request identity mismatch")
    for key in ("owner", "unit", "function"):
        _text(request.get(key), f"session request.{key}")
    target = request.get("target")
    if not isinstance(target, Mapping):
        _fail("session request target is missing")
    _closed(target, allowed={"path", "size_bytes", "sha256", "identity"}, required={"path", "size_bytes", "sha256"}, label="session target")
    _text(target.get("path"), "session target.path")
    _integer(target.get("size_bytes"), "session target.size_bytes")
    _sha256(target.get("sha256"), "session target.sha256")
    target_parent_identity_value = session.get("target_parent_identity")
    target_parent_identity = None
    if target_parent_identity_value is not None:
        target_parent_identity = _parse_directory_identity(
            target_parent_identity_value, "session target parent identity"
        )
    elif skip_live_target_check:
        _fail("session target parent identity is missing")
    target_path = _resolve(str(target["path"]), root)
    if skip_live_target_check:
        _validate_target_path(
            target_path,
            allow_missing_leaf=True,
            label="session target",
        )
    else:
        _recheck_descriptor(target, "session target")
    if target_parent_identity is not None:
        current_parent_identity = _directory_identity(
            target_path.parent, "session target parent"
        )
        if current_parent_identity != target_parent_identity:
            _fail(
                "session target parent changed from its authenticated identity: "
                f"{target_path.parent}"
            )
    target_blob = session.get("target_blob")
    if not isinstance(target_blob, Mapping):
        _fail("session target CAS is missing")
    _closed(
        target_blob,
        allowed={"kind", "sha256", "size_bytes", "cas_path", "dedup_hit"},
        required={"kind", "sha256", "size_bytes", "cas_path", "dedup_hit"},
        label="session target CAS",
    )
    if target_blob.get("kind") != "target":
        _fail("session target CAS kind is invalid")
    _sha256(target_blob.get("sha256"), "session target CAS.sha256")
    _integer(target_blob.get("size_bytes"), "session target CAS.size_bytes")
    _text(target_blob.get("cas_path"), "session target CAS.cas_path")
    if not isinstance(target_blob.get("dedup_hit"), bool):
        _fail("session target CAS.dedup_hit must be boolean")
    if target_blob.get("sha256") != target.get("sha256") or target_blob.get("size_bytes") != target.get("size_bytes"):
        _fail("session target CAS is not bound to the request target")
    target_cas = _contained(workspace, target_blob.get("cas_path"), "session target CAS path")
    target_current = _snapshot(target_cas, "session target CAS")
    if target_current["sha256"] != target_blob.get("sha256") or target_current["size_bytes"] != target_blob.get("size_bytes"):
        _fail("session target CAS does not match its frozen descriptor")
    context = request.get("context")
    if not isinstance(context, Mapping):
        _fail("session context is missing")
    _closed(
        context,
        allowed={
            "base_commit", "toolchain_key", "compiler", "compile_argv",
            "compile_cwd", "compile_tools", "compile_inputs", "context_complete",
            "dependency_provenance", "build_rule", "include_roots", "environment",
            "runtime_dlls", "compile_outputs", "source_tree", "argv_binding",
        },
        required={"base_commit", "toolchain_key", "compiler", "compile_argv", "compile_inputs", "context_complete"},
        label="session context",
    )
    _text(context.get("base_commit"), "session context.base_commit")
    _text(context.get("toolchain_key"), "session context.toolchain_key")
    if not isinstance(context.get("compile_argv"), list) or not all(isinstance(arg, str) and "\x00" not in arg for arg in context["compile_argv"]):
        _fail("session context.compile_argv must be a string array")
    if not isinstance(context.get("context_complete"), bool):
        _fail("session context.context_complete must be boolean")
    compiler = context.get("compiler")
    if isinstance(compiler, Mapping):
        _closed(compiler, allowed={"path", "size_bytes", "sha256", "identity"}, required={"path", "size_bytes", "sha256"}, label="session compiler")
        _recheck_descriptor(compiler, "session compiler")
    elif compiler is not None:
        _fail("session compiler must be a descriptor or null")
    compile_cwd = context.get("compile_cwd")
    if compile_cwd is not None:
        _recheck_compile_cwd(compile_cwd, "session compile cwd")
    compile_tools = context.get("compile_tools", [])
    if not isinstance(compile_tools, list):
        _fail("session compile_tools is invalid")
    for index, item in enumerate(compile_tools):
        if not isinstance(item, Mapping):
            _fail(f"session compile tool {index} is invalid")
        _closed(
            item,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"session compile tool {index}",
        )
        _recheck_descriptor(item, f"session compile tool {index}")
    compile_inputs = context.get("compile_inputs", []) if isinstance(context, Mapping) else []
    if not isinstance(compile_inputs, list):
        _fail("session compile_inputs is invalid")
    for index, item in enumerate(compile_inputs):
        if not isinstance(item, Mapping):
            _fail(f"session compile input {index} is invalid")
        _closed(item, allowed={"path", "size_bytes", "sha256", "identity"}, required={"path", "size_bytes", "sha256"}, label=f"session compile input {index}")
        _recheck_descriptor(item, f"session compile input {index}")
    if "build_rule" in context:
        build_rule = context["build_rule"]
        _closed(
            build_rule,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label="session build rule",
        )
        _recheck_descriptor(build_rule, "session build rule")
    if "include_roots" in context:
        _recheck_include_roots(context["include_roots"], "session include roots")
    if "environment" in context:
        _recheck_environment(context["environment"], "session environment")
    if "runtime_dlls" in context:
        runtime_dlls = context["runtime_dlls"]
        if not isinstance(runtime_dlls, list):
            _fail("session runtime_dlls is invalid")
        for index, item in enumerate(runtime_dlls):
            _closed(
                item,
                allowed={"path", "size_bytes", "sha256", "identity"},
                required={"path", "size_bytes", "sha256"},
                label=f"session runtime DLL {index}",
            )
            _recheck_descriptor(item, f"session runtime DLL {index}")
    if "compile_outputs" in context:
        _recheck_output_binding(context["compile_outputs"], "session compile outputs")
    if "source_tree" in context:
        _recheck_source_tree(context["source_tree"], "session source tree")
    if "dependency_provenance" in context:
        _recheck_dependency_provenance(
            context["dependency_provenance"], compile_inputs, "session dependency provenance"
        )
    if "argv_binding" in context:
        _recheck_argv_binding(
            context["argv_binding"],
            context["compile_argv"],
            "session argv binding",
            context.get("compile_cwd") if isinstance(context.get("compile_cwd"), Mapping) else None,
        )
    policy = request.get("policy")
    if not isinstance(policy, Mapping):
        _fail("session policy is missing")
    _closed(
        policy,
        allowed={"max_workers", "max_report_bytes", "max_compact_bytes", "allowed_job_kinds"},
        required={"max_workers", "max_report_bytes", "max_compact_bytes", "allowed_job_kinds"},
        label="session policy",
    )
    _integer(policy.get("max_workers"), "session policy.max_workers", minimum=1, maximum=32)
    _integer(policy.get("max_report_bytes"), "session policy.max_report_bytes", minimum=1024)
    _integer(policy.get("max_compact_bytes"), "session policy.max_compact_bytes", minimum=256)
    kinds = policy.get("allowed_job_kinds")
    if not isinstance(kinds, list) or not kinds or not all(isinstance(kind, str) for kind in kinds) or len(set(kinds)) != len(kinds):
        _fail("session policy.allowed_job_kinds is invalid")
    manifest = session.get("request_manifest")
    if not isinstance(manifest, Mapping):
        _fail("session request manifest descriptor is missing")
    _closed(manifest, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label="session request manifest")
    _text(manifest.get("path"), "session request manifest.path")
    _integer(manifest.get("size_bytes"), "session request manifest.size_bytes")
    _sha256(manifest.get("sha256"), "session request manifest.sha256")
    _recheck_descriptor(manifest, "session request manifest")
    manifest_request = _request(
        _load_json(Path(str(manifest["path"])), "session request manifest"),
        root=root,
        frozen_target=target if skip_live_target_check else None,
    )
    if manifest_request != request:
        _fail("session request no longer matches its authenticated request manifest")
    return session


def _load_index(workspace: Path, session: Mapping[str, Any]) -> dict[str, Any]:
    path = workspace / "index.json"
    if not path.is_file():
        _fail("initialized workbench index is missing")
    value = _load_json(path, "workbench index")
    _verify_self_hash(value, "index_sha256", "workbench index")
    item = _closed(
        value,
        allowed={
            "schema", "schema_version", "session_sha256", "sequence", "candidates",
            "source_context_index", "object_index", "diagnostic_index",
            "last_record_sha256", "index_sha256",
        },
        required={
            "schema", "schema_version", "session_sha256", "sequence", "candidates",
            "source_context_index", "object_index", "diagnostic_index",
            "last_record_sha256", "index_sha256",
        },
        label="workbench index",
    )
    if item["schema"] != INDEX_SCHEMA or item["schema_version"] != 1:
        _fail("unsupported workbench index schema")
    if item["session_sha256"] != session["session_sha256"]:
        _fail("workbench index belongs to a different session")
    for key in ("candidates", "source_context_index", "object_index", "diagnostic_index"):
        if not isinstance(item[key], dict):
            _fail(f"workbench index.{key} must be an object")
    for candidate_id, relative in item["candidates"].items():
        _identifier(candidate_id, "workbench index candidate id")
        if relative != f"candidates/{candidate_id}.json":
            _fail("workbench index candidate path is invalid")
    known_candidates = set(item["candidates"])
    for key in ("source_context_index", "object_index"):
        for digest, candidate_id in item[key].items():
            _sha256(digest, f"workbench index {key} key")
            _identifier(candidate_id, f"workbench index {key} candidate id")
            if candidate_id not in known_candidates:
                _fail(f"workbench index {key} points to an unknown candidate")
    for digest, relative in item["diagnostic_index"].items():
        _sha256(digest, "workbench index diagnostic key")
        if relative != f"diagnostics/{digest}.json":
            _fail("workbench index diagnostic path is invalid")
    _integer(item["sequence"], "workbench index.sequence")
    if item["last_record_sha256"] is not None:
        _sha256(item["last_record_sha256"], "workbench index.last_record_sha256")
    return dict(item)


def _persist_diagnostic_index(
    workspace: Path, session: Mapping[str, Any], fingerprint: str, relative_path: str
) -> None:
    """Join immutable diagnostic events to the candidate index atomically."""
    with _workbench_lock(workspace / ".workbench.lock", 8.0):
        index = _load_index(workspace, session)
        existing = index["diagnostic_index"].get(fingerprint)
        if existing is not None and existing != relative_path:
            _fail(f"diagnostic index collision for {fingerprint}")
        if existing == relative_path:
            return
        index["diagnostic_index"][fingerprint] = relative_path
        _atomic_replace(workspace / "index.json", _canonical(_with_self_hash(index, "index_sha256")))


def _request(
    value: Any, *, root: Path, frozen_target: Mapping[str, Any] | None = None
) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={
            "schema", "schema_version", "session_id", "owner", "unit", "function",
            "target", "context", "policy",
        },
        required={
            "schema", "schema_version", "session_id", "owner", "unit", "function",
            "target", "context", "policy",
        },
        label="request",
    )
    if item["schema"] != REQUEST_SCHEMA or item["schema_version"] != 1:
        _fail(f"request must use {REQUEST_SCHEMA}")
    context = _closed(
        item["context"],
        allowed={
            "base_commit", "toolchain_key", "compiler", "compile_argv",
            "compile_cwd", "compile_tools", "compile_inputs", "context_complete",
            "dependency_provenance", "build_rule", "include_roots", "environment",
            "runtime_dlls", "compile_outputs", "source_tree", "argv_binding",
        },
        required={"base_commit", "toolchain_key", "compiler", "compile_argv"},
        label="request.context",
    )
    compiler = None
    if context["compiler"] is not None:
        compiler = _descriptor(context["compiler"], base=root, label="request.context.compiler")
    argv = context["compile_argv"]
    if not isinstance(argv, list) or not all(isinstance(arg, str) and "\x00" not in arg for arg in argv):
        _fail("request.context.compile_argv must be a string array")
    compile_cwd = _compile_cwd_descriptor(
        context.get("compile_cwd"), root=root, label="request.context.compile_cwd"
    )
    compile_tools_value = context.get("compile_tools", [])
    if not isinstance(compile_tools_value, list):
        _fail("request.context.compile_tools must be an array")
    compile_tools = [
        _descriptor(item, base=root, label=f"request.context.compile_tools[{index}]")
        for index, item in enumerate(compile_tools_value)
    ]
    compile_inputs_value = context.get("compile_inputs", [])
    if not isinstance(compile_inputs_value, list):
        _fail("request.context.compile_inputs must be an array")
    compile_inputs = [
        _descriptor(item, base=root, label=f"request.context.compile_inputs[{index}]")
        for index, item in enumerate(compile_inputs_value)
    ]
    claimed_complete = context.get("context_complete", False)
    if not isinstance(claimed_complete, bool):
        _fail("request.context.context_complete must be boolean")
    if claimed_complete and (compiler is not None or argv):
        if compiler is None:
            _fail("a claimed complete compile context requires an authenticated compiler")
        if compile_cwd is None:
            _fail("a claimed complete compile context requires an authenticated compile_cwd")
        if not compile_inputs:
            _fail("a claimed complete compile context requires authenticated compile_inputs")
    context_complete = bool(claimed_complete) or (compiler is None and not argv)
    policy = _closed(
        item["policy"],
        allowed={"max_workers", "max_report_bytes", "max_compact_bytes", "allowed_job_kinds"},
        required={"max_workers", "max_report_bytes", "max_compact_bytes", "allowed_job_kinds"},
        label="request.policy",
    )
    kinds = policy["allowed_job_kinds"]
    if (
        not isinstance(kinds, list)
        or not kinds
        or not all(isinstance(kind, str) for kind in kinds)
        or len(set(kinds)) != len(kinds)
    ):
        _fail("request.policy.allowed_job_kinds must be a non-empty unique array")
    kinds = [_identifier(kind, "request.policy.allowed_job_kinds item") for kind in kinds]
    target = (
        _descriptor(item["target"], base=root, label="request.target")
        if frozen_target is None
        else _frozen_target_descriptor(
            item["target"],
            base=root,
            expected=frozen_target,
            label="request.target",
        )
    )
    normalized_context: dict[str, Any] = {
        "base_commit": _text(context["base_commit"], "request.context.base_commit"),
        "toolchain_key": _text(context["toolchain_key"], "request.context.toolchain_key"),
        "compiler": compiler,
        "compile_argv": list(argv),
        "compile_inputs": compile_inputs,
        "context_complete": context_complete,
    }
    # Preserve the canonical shape of v1 manifests created before cwd/tool
    # binding existed. This keeps their immutable session comparison readable,
    # while _compile_context_complete deliberately refuses compile reuse for
    # executable legacy contexts that lack an authenticated cwd.
    if "compile_cwd" in context:
        normalized_context["compile_cwd"] = compile_cwd
    if "compile_tools" in context:
        normalized_context["compile_tools"] = compile_tools

    if "build_rule" in context:
        normalized_context["build_rule"] = _normalize_build_rule(
            context["build_rule"], root=root, label="request.context.build_rule"
        )
    if "include_roots" in context:
        normalized_context["include_roots"] = _normalize_include_roots(
            context["include_roots"], root=root, label="request.context.include_roots"
        )
    if "environment" in context:
        normalized_context["environment"] = _normalize_environment(
            context["environment"], "request.context.environment"
        )
        _recheck_environment(
            normalized_context["environment"], "request.context.environment"
        )
    if "runtime_dlls" in context:
        normalized_context["runtime_dlls"] = _normalize_runtime_dlls(
            context["runtime_dlls"], root=root, label="request.context.runtime_dlls"
        )
    if "compile_outputs" in context:
        normalized_context["compile_outputs"] = _normalize_output_binding(
            context["compile_outputs"], root=root, label="request.context.compile_outputs"
        )
    if "source_tree" in context:
        normalized_context["source_tree"] = _normalize_source_tree(
            context["source_tree"], root=root, label="request.context.source_tree"
        )
    if "dependency_provenance" in context:
        normalized_context["dependency_provenance"] = _normalize_dependency_provenance(
            context["dependency_provenance"],
            root=root,
            compile_inputs=compile_inputs,
            label="request.context.dependency_provenance",
        )
    if "argv_binding" in context or any(arg.startswith("@") for arg in argv):
        normalized_context["argv_binding"] = _normalize_argv_binding(
            context.get("argv_binding"),
            argv=argv,
            root=root,
            compile_cwd=normalized_context.get("compile_cwd"),
            label="request.context.argv_binding",
        )

    return {
        "schema": REQUEST_SCHEMA,
        "schema_version": 1,
        "session_id": _identifier(item["session_id"], "request.session_id"),
        "owner": _text(item["owner"], "request.owner"),
        "unit": _text(item["unit"], "request.unit"),
        "function": _text(item["function"], "request.function"),
        "target": target,
        "context": normalized_context,
        "policy": {
            "max_workers": _integer(policy["max_workers"], "request.policy.max_workers", minimum=1, maximum=32),
            "max_report_bytes": _integer(policy["max_report_bytes"], "request.policy.max_report_bytes", minimum=1024),
            "max_compact_bytes": _integer(policy["max_compact_bytes"], "request.policy.max_compact_bytes", minimum=256),
            "allowed_job_kinds": kinds,
        },
    }


def init_workspace(root: Path, manifest: Path | str, workspace: Path | str) -> dict[str, Any]:
    root = root.resolve()
    manifest_path = _resolve(manifest, root)
    manifest_snapshot = _snapshot(manifest_path, "request manifest")
    request = _request(_load_json(manifest_path, "request manifest"), root=root)
    destination = _workspace(workspace, root)
    _safe_mkdir(destination)
    session_path = destination / "session.json"
    lock_path = destination / ".workbench.lock"
    with _workbench_lock(lock_path, 8.0):
        if session_path.is_file():
            existing = _load_session(destination, root)
            _load_index(destination, existing)
            if existing.get("request_manifest", {}).get("sha256") != manifest_snapshot["sha256"]:
                _fail("workspace already contains a different immutable session")
            return {"status": "unchanged", "workspace": os.fspath(destination), "session": existing}
        leftovers = [item for item in destination.iterdir() if item.name != ".workbench.lock"]
        if leftovers:
            _fail("workspace is non-empty but does not contain an immutable session")
        for relative in ("candidates", "diagnostics", "cas/blobs", "cas/reports", "job-output"):
            _safe_mkdir(destination / relative)
        target_parent_identity = _directory_identity(
            Path(request["target"]["path"]).parent, "session target parent"
        )
        target_blob = _copy_blob(destination, Path(request["target"]["path"]), "target", request["target"])
        body = {
            "schema": SESSION_SCHEMA,
            "schema_version": 1,
            "session_id": request["session_id"],
            "root": os.fspath(root),
            "workspace": os.fspath(destination),
            "request": request,
            "request_manifest": {key: manifest_snapshot[key] for key in ("path", "size_bytes", "sha256")},
            "target_blob": target_blob,
            "target_parent_identity": _directory_identity_payload(target_parent_identity),
            "authority_advanced": False,
        }
        session = _with_self_hash(body, "session_sha256")
        _write_new(session_path, _canonical(session))
        _write_new(destination / "index.json", _canonical(_empty_index(session["session_sha256"])))
    return {"status": "initialized", "workspace": os.fspath(destination), "session": session}


def repair_target(root: Path, workspace: Path | str) -> dict[str, Any]:
    """Restore a missing or mutated original target from the session CAS.

    All session, manifest, compiler-input, request-binding, and CAS checks are
    still performed.  The only relaxed check is the live original target
    content snapshot, which is precisely the state this operation repairs.
    """
    root = root.resolve()
    destination = _workspace(workspace, root)
    with _workbench_lock(destination / ".workbench.lock", 8.0):
        session = _load_session(destination, root, skip_live_target_check=True)
        target = session["request"]["target"]
        target_path = _resolve(str(target["path"]), root)
        target_cas = _contained(
            destination,
            session["target_blob"]["cas_path"],
            "session target CAS path",
        )
        cas = _snapshot(target_cas, "session target CAS")
        if cas["sha256"] != target["sha256"] or cas["size_bytes"] != target["size_bytes"]:
            _fail("session target CAS does not match its frozen descriptor")

        _validate_target_path(
            target_path, allow_missing_leaf=True, label="session target"
        )
        if os.path.lexists(target_path):
            current = _snapshot(target_path, "session target")
            if (
                current["sha256"] == target["sha256"]
                and current["size_bytes"] == target["size_bytes"]
            ):
                return {
                    "status": "unchanged",
                    "workspace": os.fspath(destination),
                    "session_id": session["session_id"],
                    "target": {
                        key: current[key] for key in ("path", "size_bytes", "sha256")
                    },
                    "authority_advanced": False,
                }

        expected_parent_identity = _parse_directory_identity(
            session.get("target_parent_identity"),
            "session target parent identity",
        )
        restored = _restore_target_from_cas(
            target_path,
            target_cas,
            target,
            expected_parent_identity,
        )
        return {
            "status": "restored",
            "workspace": os.fspath(destination),
            "session_id": session["session_id"],
            "target": restored,
            "authority_advanced": False,
        }


def _legacy_context_key(session: Mapping[str, Any], source_sha: str) -> str:
    value = {
        "session_sha256": session["session_sha256"],
        "source_sha256": source_sha,
        "target_sha256": session["request"]["target"]["sha256"],
        "context": session["request"]["context"],
    }
    return _sha256_bytes(_canonical(value))


def _context_key(
    session: Mapping[str, Any], source_sha: str, compile_input_identity: Mapping[str, Any]
) -> str:
    value = {
        "session_sha256": session["session_sha256"],
        "source_sha256": source_sha,
        "target_sha256": session["request"]["target"]["sha256"],
        "context": session["request"]["context"],
        "compile_input_identity": dict(compile_input_identity),
    }
    return _sha256_bytes(_canonical(value))


def _compile_context_projection(context: Mapping[str, Any], label: str) -> dict[str, Any]:
    """Return the compile-producer identity that candidate records must attest.

    The full session context contains dependency and output bindings used for
    compile reuse.  Candidate provenance has a narrower purpose: prove which
    compiler/tool wrapper/argv/cwd produced the supplied object.  Keeping this
    projection explicit lets old incomplete sessions remain auditable while
    preventing an object made by another compiler from inheriting the session
    identity merely because it was recorded there.
    """

    toolchain_key = _text(context.get("toolchain_key"), f"{label}.toolchain_key")
    compiler = context.get("compiler")
    if compiler is not None:
        _closed(
            compiler,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}.compiler",
        )
        _text(compiler.get("path"), f"{label}.compiler.path")
        _integer(compiler.get("size_bytes"), f"{label}.compiler.size_bytes")
        _sha256(compiler.get("sha256"), f"{label}.compiler.sha256")
    argv = context.get("compile_argv", [])
    if not isinstance(argv, list) or any(not isinstance(arg, str) for arg in argv):
        _fail(f"{label}.compile_argv must be an array of strings")
    tools = context.get("compile_tools", [])
    if tools is None:
        tools = []
    if not isinstance(tools, list):
        _fail(f"{label}.compile_tools must be an array")
    normalized_tools: list[dict[str, Any]] = []
    for index, tool in enumerate(tools):
        _closed(
            tool,
            allowed={"path", "size_bytes", "sha256", "identity"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}.compile_tools[{index}]",
        )
        _text(tool.get("path"), f"{label}.compile_tools[{index}].path")
        _integer(tool.get("size_bytes"), f"{label}.compile_tools[{index}].size_bytes")
        _sha256(tool.get("sha256"), f"{label}.compile_tools[{index}].sha256")
        normalized_tools.append(copy.deepcopy(dict(tool)))
    cwd = context.get("compile_cwd")
    if cwd is not None and not isinstance(cwd, Mapping):
        _fail(f"{label}.compile_cwd must be an object or null")
    argv_binding = context.get("argv_binding")
    if argv_binding is not None and not isinstance(argv_binding, Mapping):
        _fail(f"{label}.argv_binding must be an object or null")
    return {
        "toolchain_key": toolchain_key,
        "compiler": copy.deepcopy(dict(compiler)) if isinstance(compiler, Mapping) else None,
        "compile_tools": normalized_tools,
        "compile_argv": list(argv),
        "compile_cwd": copy.deepcopy(dict(cwd)) if isinstance(cwd, Mapping) else None,
        "argv_binding": (
            copy.deepcopy(dict(argv_binding))
            if isinstance(argv_binding, Mapping)
            else None
        ),
    }


def _compile_context_sha256(context: Mapping[str, Any], label: str) -> str:
    return _sha256_bytes(_canonical(_compile_context_projection(context, label)))


def _artifact_descriptor_matches(
    descriptor_value: Mapping[str, Any], snapshot: Mapping[str, Any], label: str
) -> None:
    _closed(
        descriptor_value,
        allowed={"path", "size_bytes", "sha256"},
        required={"path", "size_bytes", "sha256"},
        label=label,
    )
    if (
        _normalized_compile_input_path(_text(descriptor_value.get("path"), f"{label}.path"))
        != _normalized_compile_input_path(_text(snapshot.get("path"), f"{label} snapshot.path"))
        or _integer(descriptor_value.get("size_bytes"), f"{label}.size_bytes")
        != _integer(snapshot.get("size_bytes"), f"{label} snapshot.size_bytes")
        or _sha256(descriptor_value.get("sha256"), f"{label}.sha256")
        != _sha256(snapshot.get("sha256"), f"{label} snapshot.sha256")
    ):
        _fail(f"{label} is not bound to the supplied artifact")


def _validate_compile_attestation(
    value: Any,
    *,
    source_snapshot: Mapping[str, Any],
    object_snapshot: Mapping[str, Any],
    expected_context: Mapping[str, Any] | None,
    label: str,
) -> dict[str, Any]:
    _verify_self_hash(value, "attestation_sha256", label)
    item = _closed(
        value,
        allowed={
            "schema", "schema_version", "context", "context_sha256",
            "source", "object", "producer", "authority_advanced",
            "attestation_sha256",
        },
        required={
            "schema", "schema_version", "context", "context_sha256",
            "source", "object", "producer", "authority_advanced",
            "attestation_sha256",
        },
        label=label,
    )
    if item.get("schema") != COMPILE_ATTESTATION_SCHEMA or item.get("schema_version") != 1:
        _fail(f"{label} schema is unsupported")
    if item.get("authority_advanced") is not False:
        _fail(f"{label} must not advance authority")
    context = _compile_context_projection(
        _closed(
            item.get("context"),
            allowed={
                "toolchain_key", "compiler", "compile_tools", "compile_argv",
                "compile_cwd", "argv_binding",
            },
            required={
                "toolchain_key", "compiler", "compile_tools", "compile_argv",
                "compile_cwd", "argv_binding",
            },
            label=f"{label}.context",
        ),
        f"{label}.context",
    )
    context_sha = _sha256(item.get("context_sha256"), f"{label}.context_sha256")
    if context_sha != _sha256_bytes(_canonical(context)):
        _fail(f"{label}.context_sha256 does not match context")
    if expected_context is not None:
        expected = _compile_context_projection(expected_context, "session compile context")
        if context != expected:
            _fail(
                f"{label} compiler/wrapper/argv context does not match the immutable session"
            )
    source_value = item.get("source")
    object_value = item.get("object")
    if not isinstance(source_value, Mapping) or not isinstance(object_value, Mapping):
        _fail(f"{label} source/object descriptors are required")
    _artifact_descriptor_matches(source_value, source_snapshot, f"{label}.source")
    _artifact_descriptor_matches(object_value, object_snapshot, f"{label}.object")
    producer = _closed(
        item.get("producer"),
        allowed={"kind", "command", "command_sha256", "notes"},
        required={"kind", "command", "command_sha256", "notes"},
        label=f"{label}.producer",
    )
    kind = _text(producer.get("kind"), f"{label}.producer.kind")
    if kind not in {"serialized-build", "external-compile-attestation", "test-fixture"}:
        _fail(f"{label}.producer.kind is unsupported")
    if kind == "test-fixture" and context.get("compiler") is not None:
        _fail(f"{label} test-fixture producer cannot attest a real compiler")
    command = producer.get("command")
    if not isinstance(command, list) or any(not isinstance(arg, str) for arg in command):
        _fail(f"{label}.producer.command must be an array of strings")
    if context.get("compiler") is not None and not command:
        _fail(f"{label}.producer.command cannot be empty for a compiled object")
    if context.get("compiler") is not None and command != context["compile_argv"]:
        _fail(
            f"{label}.producer.command must exactly equal the attested compile_argv"
        )
    if _sha256(producer.get("command_sha256"), f"{label}.producer.command_sha256") != _sha256_bytes(
        _canonical(command)
    ):
        _fail(f"{label}.producer.command_sha256 does not match command")
    notes = producer.get("notes")
    if notes is not None and not isinstance(notes, str):
        _fail(f"{label}.producer.notes must be a string or null")
    return copy.deepcopy(dict(item))


def _load_compile_attestation(
    root: Path,
    path: Path | str,
    *,
    source_snapshot: Mapping[str, Any],
    object_snapshot: Mapping[str, Any],
    expected_context: Mapping[str, Any] | None,
    label: str = "compile attestation",
) -> dict[str, Any]:
    attestation_path = _resolve(path, root)
    snapshot = _snapshot(attestation_path, label)
    result = _validate_compile_attestation(
        _load_json(attestation_path, label),
        source_snapshot=source_snapshot,
        object_snapshot=object_snapshot,
        expected_context=expected_context,
        label=label,
    )
    _recheck_live_snapshot(attestation_path, snapshot, label)
    return result


def _require_candidate_compile_attestation(
    candidate: Mapping[str, Any], session: Mapping[str, Any]
) -> Mapping[str, Any]:
    attestation = candidate.get("compile_attestation")
    if not isinstance(attestation, Mapping):
        _fail(
            "candidate compile provenance is unavailable; run provenance-audit "
            "and migrate authenticated records before lookup/reuse"
        )
    return _validate_compile_attestation(
        attestation,
        source_snapshot=candidate["source"],
        object_snapshot=candidate["object"],
        expected_context=session["request"]["context"],
        label="candidate compile attestation",
    )


def create_compile_attestation(
    root: Path,
    workspace: Path | str,
    *,
    source: Path | str,
    object_path: Path | str,
    output: Path | str,
    producer_kind: str,
    producer_command: Sequence[str] | None,
    notes: str | None = None,
) -> dict[str, Any]:
    """Seal the actual producer context before a candidate can be recorded.

    This is an attestation boundary, not a compiler launcher.  The caller must
    name the workspace that describes the compiler invocation actually used.
    Recording into any other session then fails on the context fingerprint.
    Real compiler attestations require the wrapper/tool chain and cwd to be
    explicit; the incomplete legacy session shape cannot mint new evidence.
    """

    root = root.resolve()
    destination = _workspace(workspace, root)
    session = _load_session(destination, root)
    source_path = _resolve(source, root)
    object_file = _resolve(object_path, root)
    source_snapshot = _snapshot(source_path, "attested candidate source")
    object_snapshot = _snapshot(object_file, "attested candidate object")
    _validate_candidate_artifact(
        source_path, source_snapshot, session, "attested candidate source"
    )
    _validate_candidate_artifact(
        object_file, object_snapshot, session, "attested candidate object"
    )
    context = _compile_context_projection(
        session["request"]["context"], "session compile context"
    )
    kind = _text(producer_kind, "producer_kind")
    if kind not in {"serialized-build", "external-compile-attestation", "test-fixture"}:
        _fail("producer_kind is unsupported")
    if context.get("compiler") is not None:
        if not context.get("compile_tools"):
            _fail(
                "real compiler attestation requires an authenticated wrapper/tool chain"
            )
        if context.get("compile_cwd") is None:
            _fail("real compiler attestation requires an authenticated compile cwd")
    command = list(
        context["compile_argv"]
        if producer_command is None
        else producer_command
    )
    if any(not isinstance(arg, str) for arg in command):
        _fail("producer_command must contain only strings")
    if context.get("compiler") is not None and not command:
        _fail("producer_command cannot be empty for a compiled object")
    if context.get("compiler") is not None and command != context["compile_argv"]:
        _fail("producer_command must exactly equal the immutable session compile_argv")
    if notes is not None and not isinstance(notes, str):
        _fail("attestation notes must be a string or null")
    body = {
        "schema": COMPILE_ATTESTATION_SCHEMA,
        "schema_version": 1,
        "context": context,
        "context_sha256": _sha256_bytes(_canonical(context)),
        "source": {
            key: source_snapshot[key] for key in ("path", "size_bytes", "sha256")
        },
        "object": {
            key: object_snapshot[key] for key in ("path", "size_bytes", "sha256")
        },
        "producer": {
            "kind": kind,
            "command": command,
            "command_sha256": _sha256_bytes(_canonical(command)),
            "notes": notes,
        },
        "authority_advanced": False,
    }
    attestation = _with_self_hash(body, "attestation_sha256")
    output_path = _resolve(output, root)
    _validate_target_path(
        output_path, allow_missing_leaf=True, label="compile attestation output"
    )
    if output_path.exists():
        existing = _load_json(output_path, "compile attestation output")
        if existing == attestation:
            return {
                "status": "unchanged",
                "path": os.fspath(output_path),
                "attestation": attestation,
                "authority_advanced": False,
            }
        _fail("compile attestation output already records different evidence")
    _write_new(output_path, _canonical(attestation))
    _recheck_live_snapshot(source_path, source_snapshot, "attested candidate source")
    _recheck_live_snapshot(object_file, object_snapshot, "attested candidate object")
    return {
        "status": "attested",
        "path": os.fspath(output_path),
        "attestation": attestation,
        "authority_advanced": False,
    }


def _provenance_manifest(root: Path, path: Path | str) -> tuple[dict[str, Path], dict[str, Any]]:
    manifest_path = _resolve(path, root)
    snapshot = _snapshot(manifest_path, "provenance manifest")
    value = _load_json(manifest_path, "provenance manifest")
    _verify_self_hash(value, "manifest_sha256", "provenance manifest")
    item = _closed(
        value,
        allowed={"schema", "schema_version", "candidates", "manifest_sha256"},
        required={"schema", "schema_version", "candidates", "manifest_sha256"},
        label="provenance manifest",
    )
    if item.get("schema") != PROVENANCE_MANIFEST_SCHEMA or item.get("schema_version") != 1:
        _fail("provenance manifest schema is unsupported")
    candidates = item.get("candidates")
    if not isinstance(candidates, list):
        _fail("provenance manifest candidates must be an array")
    result: dict[str, Path] = {}
    for index, row in enumerate(candidates):
        entry = _closed(
            row,
            allowed={"candidate_id", "attestation"},
            required={"candidate_id", "attestation"},
            label=f"provenance manifest candidates[{index}]",
        )
        candidate_id = _identifier(entry.get("candidate_id"), f"provenance manifest candidates[{index}].candidate_id")
        if candidate_id in result:
            _fail(f"provenance manifest repeats candidate_id {candidate_id}")
        result[candidate_id] = _resolve(
            _text(entry.get("attestation"), f"provenance manifest candidates[{index}].attestation"),
            root,
        )
    _recheck_live_snapshot(manifest_path, snapshot, "provenance manifest")
    return result, copy.deepcopy(dict(item))


def audit_candidate_provenance(
    root: Path,
    workspace: Path | str,
    *,
    manifest: Path | str | None = None,
) -> dict[str, Any]:
    """Classify immutable records by their attested producer context."""

    root = root.resolve()
    destination = _workspace(workspace, root)
    session = _load_session(destination, root)
    index = _load_index(destination, session)
    attestations: dict[str, Path] = {}
    manifest_value = None
    if manifest is not None:
        attestations, manifest_value = _provenance_manifest(root, manifest)
    unknown = sorted(set(attestations) - set(index["candidates"]))
    if unknown:
        _fail(f"provenance manifest names unknown candidate {unknown[0]}")
    expected_context = _compile_context_projection(
        session["request"]["context"], "session compile context"
    )
    expected_sha = _sha256_bytes(_canonical(expected_context))
    rows: list[dict[str, Any]] = []
    counts = {"context_match": 0, "cross_context": 0, "unattested": 0}
    candidates = sorted(
        (
            _load_candidate(destination, candidate_id, session)
            for candidate_id in index["candidates"]
        ),
        key=lambda row: (int(row["ordinal"]), str(row["candidate_id"])),
    )
    for candidate in candidates:
        candidate_id = str(candidate["candidate_id"])
        attestation: Mapping[str, Any] | None = None
        evidence = "none"
        if candidate_id in attestations:
            attestation = _load_compile_attestation(
                root,
                attestations[candidate_id],
                source_snapshot=candidate["source"],
                object_snapshot=candidate["object"],
                expected_context=None,
                label=f"compile attestation for {candidate_id}",
            )
            evidence = "external_manifest"
        elif isinstance(candidate.get("compile_attestation"), Mapping):
            attestation = _validate_compile_attestation(
                candidate["compile_attestation"],
                source_snapshot=candidate["source"],
                object_snapshot=candidate["object"],
                expected_context=None,
                label=f"embedded compile attestation for {candidate_id}",
            )
            evidence = "embedded_record"
        if attestation is None:
            status = "unattested"
            actual_sha = None
            actual_toolchain = None
            actual_compiler_sha = None
        else:
            actual_sha = str(attestation["context_sha256"])
            status = "context_match" if actual_sha == expected_sha else "cross_context"
            actual_context = attestation["context"]
            actual_toolchain = actual_context.get("toolchain_key")
            compiler = actual_context.get("compiler")
            actual_compiler_sha = (
                compiler.get("sha256") if isinstance(compiler, Mapping) else None
            )
        counts[status] += 1
        rows.append(
            {
                "candidate_id": candidate_id,
                "ordinal": candidate["ordinal"],
                "status": status,
                "evidence": evidence,
                "source_sha256": candidate["source"]["sha256"],
                "object_sha256": candidate["object"]["sha256"],
                "duplicate_of": candidate.get("duplicate_of"),
                "session_context_sha256": expected_sha,
                "actual_context_sha256": actual_sha,
                "session_toolchain_key": expected_context["toolchain_key"],
                "actual_toolchain_key": actual_toolchain,
                "actual_compiler_sha256": actual_compiler_sha,
            }
        )
    result = _with_self_hash(
        {
            "schema": PROVENANCE_AUDIT_SCHEMA,
            "schema_version": 1,
            "session_id": session["session_id"],
            "session_sha256": session["session_sha256"],
            "workspace": os.fspath(destination),
            "manifest_sha256": (
                manifest_value.get("manifest_sha256")
                if isinstance(manifest_value, Mapping)
                else None
            ),
            "counts": counts,
            "rows": rows,
            "status": (
                "clean"
                if counts["cross_context"] == 0 and counts["unattested"] == 0
                else "requires_migration"
            ),
            "authority_advanced": False,
        },
        "audit_sha256",
    )
    return result


def _persist_provenance_result(
    root: Path,
    output: Path | str | None,
    result: Mapping[str, Any],
    *,
    label: str,
) -> None:
    """Optionally persist one self-hashed provenance receipt fail-closed."""

    if output is None:
        return
    output_path = _resolve(output, root)
    _validate_target_path(output_path, allow_missing_leaf=True, label=label)
    if output_path.exists():
        existing = _load_json(output_path, label)
        if existing == result:
            return
        _fail(f"{label} already records different evidence")
    _write_new(output_path, _canonical(result))


def _migrate_report_binding(
    source_workspace: Path,
    destination_workspace: Path,
    report: Mapping[str, Any] | None,
    destination_session: Mapping[str, Any],
    *,
    label: str,
    focus_symbols: Sequence[str],
) -> dict[str, Any] | None:
    if report is None:
        return None
    compressed_path = _contained(
        source_workspace, report.get("cas_path"), f"source {label} report CAS"
    )
    # Decompress through a private temporary file and let _store_report apply
    # the destination session's size/compact-summary gates.  The source gzip
    # descriptor was already verified by _load_candidate.
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "wb",
            dir=destination_workspace / "job-output",
            prefix=f".migration-{label}.",
            suffix=".json",
            delete=False,
        ) as stream:
            temporary = Path(stream.name)
            try:
                with gzip.open(compressed_path, "rb") as source_stream:
                    for block in iter(lambda: source_stream.read(1024 * 1024), b""):
                        stream.write(block)
            except (OSError, EOFError) as exc:
                raise MatchError(f"source {label} report CAS is invalid gzip: {exc}") from exc
            stream.flush()
            os.fsync(stream.fileno())
        raw_snapshot = _snapshot(temporary, f"migrated {label} report")
        if (
            raw_snapshot["sha256"] != report.get("raw_sha256")
            or raw_snapshot["size_bytes"] != report.get("raw_size_bytes")
        ):
            _fail(f"source {label} report raw binding mismatch")
        return _store_report(
            destination_workspace,
            temporary,
            destination_session,
            f"migrated {label} report",
            focus_symbol=focus_symbols,
        )
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def migrate_candidate_provenance(
    root: Path,
    source_workspace: Path | str,
    destination_workspace: Path | str,
    *,
    manifest: Path | str,
) -> dict[str, Any]:
    """Import only records attested for the destination compiler context.

    Source history is immutable and remains untouched.  Imported records are
    re-keyed to the destination session, retain attempt metadata and report
    CAS evidence, and carry an explicit receipt back to the source record.
    Duplicate relations are re-derived in source ordinal order and checked
    against imported predecessors.
    """

    root = root.resolve()
    source_path = _workspace(source_workspace, root)
    destination_path = _workspace(destination_workspace, root)
    if source_path == destination_path:
        _fail("provenance migration requires distinct source and destination workspaces")
    source_session = _load_session(source_path, root)
    source_index = _load_index(source_path, source_session)
    destination_session = _load_session(destination_path, root)
    attestations, manifest_value = _provenance_manifest(root, manifest)
    unknown = sorted(set(attestations) - set(source_index["candidates"]))
    if unknown:
        _fail(f"provenance manifest names unknown candidate {unknown[0]}")
    if not attestations:
        _fail("provenance migration manifest contains no candidates")
    expected_context = _compile_context_projection(
        destination_session["request"]["context"], "destination compile context"
    )
    expected_sha = _sha256_bytes(_canonical(expected_context))
    source_candidates = sorted(
        (
            _load_candidate(source_path, candidate_id, source_session)
            for candidate_id in attestations
        ),
        key=lambda row: (int(row["ordinal"]), str(row["candidate_id"])),
    )
    rows: list[dict[str, Any]] = []
    counts = {"imported": 0, "skipped_cross_context": 0}
    with _workbench_lock(destination_path / ".workbench.lock", 8.0):
        destination_index = _load_index(destination_path, destination_session)
        for source_candidate in source_candidates:
            candidate_id = str(source_candidate["candidate_id"])
            attestation = _load_compile_attestation(
                root,
                attestations[candidate_id],
                source_snapshot=source_candidate["source"],
                object_snapshot=source_candidate["object"],
                expected_context=None,
                label=f"compile attestation for {candidate_id}",
            )
            if attestation["context_sha256"] != expected_sha:
                counts["skipped_cross_context"] += 1
                rows.append(
                    {
                        "candidate_id": candidate_id,
                        "source_ordinal": source_candidate["ordinal"],
                        "status": "skipped_cross_context",
                        "actual_context_sha256": attestation["context_sha256"],
                        "destination_context_sha256": expected_sha,
                    }
                )
                continue
            existing_path = _candidate_path(destination_path, candidate_id)
            if existing_path.is_file():
                existing = _load_candidate(
                    destination_path, candidate_id, destination_session
                )
                if candidate_id not in destination_index["candidates"]:
                    if (
                        existing.get("ordinal") != int(destination_index["sequence"]) + 1
                        or existing.get("previous_record_sha256")
                        != destination_index.get("last_record_sha256")
                    ):
                        _fail(
                            "unindexed migrated candidate is not a recoverable final append"
                        )
                    for key, mapped_candidate in (
                        (
                            existing["source_context_key"],
                            destination_index["source_context_index"].get(
                                existing["source_context_key"]
                            ),
                        ),
                        (
                            existing["object_result_key"],
                            destination_index["object_index"].get(
                                existing["object_result_key"]
                            ),
                        ),
                    ):
                        if mapped_candidate is not None and mapped_candidate != candidate_id:
                            _fail(f"unindexed migrated candidate collides with {key}")
                    destination_index["sequence"] = existing["ordinal"]
                    destination_index["candidates"][candidate_id] = (
                        existing_path.relative_to(destination_path).as_posix()
                    )
                    destination_index["source_context_index"].setdefault(
                        existing["source_context_key"], candidate_id
                    )
                    destination_index["object_index"].setdefault(
                        existing["object_result_key"], candidate_id
                    )
                    destination_index["last_record_sha256"] = existing[
                        "record_sha256"
                    ]
                    _atomic_replace(
                        destination_path / "index.json",
                        _canonical(
                            _with_self_hash(destination_index, "index_sha256")
                        ),
                    )
                migration = existing.get("migration")
                if not isinstance(migration, Mapping) or (
                    migration.get("source_session_sha256")
                    != source_session["session_sha256"]
                    or migration.get("source_record_sha256")
                    != source_candidate["record_sha256"]
                    or existing.get("compile_attestation", {}).get("attestation_sha256")
                    != attestation["attestation_sha256"]
                ):
                    _fail(
                        f"destination candidate_id already records different evidence: {candidate_id}"
                    )
                counts["imported"] += 1
                rows.append(
                    {
                        "candidate_id": candidate_id,
                        "source_ordinal": source_candidate["ordinal"],
                        "destination_ordinal": existing["ordinal"],
                        "status": "imported",
                        "duplicate_of": existing.get("duplicate_of"),
                        "source_duplicate_of": source_candidate.get("duplicate_of"),
                        "actual_context_sha256": expected_sha,
                        "destination_context_sha256": expected_sha,
                    }
                )
                continue
            compile_input = _validate_compile_input_identity(
                source_candidate.get("compile_input_identity"),
                source_candidate["source"],
            )
            source_key = _context_key(
                destination_session,
                source_candidate["source"]["sha256"],
                compile_input,
            )
            object_key = _object_key(
                destination_session, source_candidate["object"]["sha256"]
            )
            source_duplicate = destination_index["source_context_index"].get(source_key)
            object_duplicate = destination_index["object_index"].get(object_key)
            if source_duplicate is not None:
                producer = _load_candidate(
                    destination_path, source_duplicate, destination_session
                )
                _require_candidate_compile_attestation(producer, destination_session)
                if producer.get("object_result_key") != object_key:
                    _fail(
                        "migration found one source/context producing different objects"
                    )
            duplicate_id = source_duplicate or object_duplicate
            source_cas = _contained(
                source_path,
                source_candidate["source_blob"]["cas_path"],
                f"source candidate {candidate_id} source CAS",
            )
            object_cas = _contained(
                source_path,
                source_candidate["object_blob"]["cas_path"],
                f"source candidate {candidate_id} object CAS",
            )
            source_blob = _copy_blob(
                destination_path,
                source_cas,
                "source",
                source_candidate["source"],
            )
            object_blob = _copy_blob(
                destination_path,
                object_cas,
                "object",
                source_candidate["object"],
            )
            focuses = _stored_focus_symbols(
                source_candidate,
                default=str(source_session["request"]["function"]),
            )
            strict = _migrate_report_binding(
                source_path,
                destination_path,
                source_candidate["reports"]["strict"],
                destination_session,
                label=f"{candidate_id}-strict",
                focus_symbols=focuses,
            )
            data = _migrate_report_binding(
                source_path,
                destination_path,
                source_candidate["reports"].get("data"),
                destination_session,
                label=f"{candidate_id}-data",
                focus_symbols=focuses,
            )
            ordinal = int(destination_index["sequence"]) + 1
            migration = {
                "source_session_sha256": source_session["session_sha256"],
                "source_candidate_id": candidate_id,
                "source_record_sha256": source_candidate["record_sha256"],
                "source_ordinal": source_candidate["ordinal"],
                "source_duplicate_of": source_candidate.get("duplicate_of"),
            }
            record_body = {
                "schema": CANDIDATE_SCHEMA,
                "schema_version": 1,
                "session_sha256": destination_session["session_sha256"],
                "candidate_id": candidate_id,
                "ordinal": ordinal,
                "source": copy.deepcopy(source_candidate["source"]),
                "object": copy.deepcopy(source_candidate["object"]),
                "compile_input_identity": compile_input,
                "compile_attestation": attestation,
                "migration": migration,
                "source_context_key": source_key,
                "object_result_key": object_key,
                "source_blob": source_blob,
                "object_blob": object_blob,
                "reports": {"strict": strict, "data": data},
                "report_binding": source_candidate["report_binding"],
                "hypothesis": copy.deepcopy(source_candidate["hypothesis"]),
                "outcome": copy.deepcopy(source_candidate["outcome"]),
                "telemetry": copy.deepcopy(source_candidate["telemetry"]),
                "duplicate_of": duplicate_id,
                "previous_record_sha256": destination_index.get("last_record_sha256"),
                "authority_advanced": False,
            }
            if "focus_symbol" in source_candidate:
                record_body["focus_symbol"] = source_candidate["focus_symbol"]
            if "focus_symbols" in source_candidate:
                record_body["focus_symbols"] = copy.deepcopy(
                    source_candidate["focus_symbols"]
                )
            record = _with_self_hash(record_body, "record_sha256")
            _write_new(existing_path, _canonical(record))
            destination_index["sequence"] = ordinal
            destination_index["candidates"][candidate_id] = existing_path.relative_to(
                destination_path
            ).as_posix()
            destination_index["source_context_index"].setdefault(
                source_key, candidate_id
            )
            destination_index["object_index"].setdefault(object_key, candidate_id)
            destination_index["last_record_sha256"] = record["record_sha256"]
            _atomic_replace(
                destination_path / "index.json",
                _canonical(_with_self_hash(destination_index, "index_sha256")),
            )
            counts["imported"] += 1
            rows.append(
                {
                    "candidate_id": candidate_id,
                    "source_ordinal": source_candidate["ordinal"],
                    "destination_ordinal": ordinal,
                    "status": "imported",
                    "duplicate_of": duplicate_id,
                    "source_duplicate_of": source_candidate.get("duplicate_of"),
                    "actual_context_sha256": expected_sha,
                    "destination_context_sha256": expected_sha,
                }
            )
    return _with_self_hash(
        {
            "schema": PROVENANCE_MIGRATION_SCHEMA,
            "schema_version": 1,
            "source_session_sha256": source_session["session_sha256"],
            "destination_session_sha256": destination_session["session_sha256"],
            "manifest_sha256": manifest_value["manifest_sha256"],
            "counts": counts,
            "rows": rows,
            "status": "migrated" if counts["imported"] else "unchanged",
            "production_modified": False,
            "authority_advanced": False,
        },
        "migration_sha256",
    )


def _source_index_match(
    workspace: Path,
    index: Mapping[str, Any],
    session: Mapping[str, Any],
    source_snapshot: Mapping[str, Any],
    compile_input_identity: Mapping[str, Any],
) -> tuple[str, str | None, Mapping[str, Any] | None]:
    """Find a path-bound source record, with a narrow legacy-record fallback."""
    source_sha = str(source_snapshot["sha256"])
    source_key = _context_key(session, source_sha, compile_input_identity)
    candidate_id = index["source_context_index"].get(source_key)
    if candidate_id is not None:
        candidate = _load_candidate(workspace, candidate_id, session)
        if candidate.get("source_context_key") != source_key:
            _fail("source/context index does not match its immutable candidate record")
        _require_candidate_compile_attestation(candidate, session)
        return source_key, candidate_id, candidate

    # Records written before compile-input path binding remain readable and
    # reusable only at the exact normalized path they originally recorded.
    legacy_key = _legacy_context_key(session, source_sha)
    legacy_id = index["source_context_index"].get(legacy_key)
    if legacy_id is None:
        return source_key, None, None
    legacy = _load_candidate(workspace, legacy_id, session)
    if legacy.get("source_context_key") != legacy_key:
        _fail("legacy source/context index does not match its immutable candidate record")
    if legacy.get("compile_input_identity") is not None:
        _fail("path-bound candidate is indexed by a legacy source/context key")
    _require_candidate_compile_attestation(legacy, session)
    legacy_path = _normalized_compile_input_path(legacy["source"]["path"])
    if legacy_path == compile_input_identity["normalized_path"]:
        return legacy_key, legacy_id, legacy
    return source_key, None, None


def _object_key(session: Mapping[str, Any], object_sha: str) -> str:
    value = {
        "session_sha256": session["session_sha256"],
        "target_sha256": session["request"]["target"]["sha256"],
        "object_sha256": object_sha,
        "toolchain_key": session["request"]["context"]["toolchain_key"],
    }
    return _sha256_bytes(_canonical(value))


def lookup_matches(
    root: Path, workspace: Path | str, source: Path | str | None = None, object_path: Path | str | None = None
) -> dict[str, Any]:
    if source is None and object_path is None:
        _fail("lookup requires a source or object artifact")
    destination = _workspace(workspace, root.resolve())
    session = _load_session(destination, root.resolve())
    index = _load_index(destination, session)
    source_snapshot = None
    source_key = None
    source_candidate = None
    if source is not None:
        source_path = _resolve(source, root)
        source_snapshot = _snapshot(source_path, "candidate source")
        _validate_candidate_artifact(
            source_path, source_snapshot, session, "candidate source"
        )
        compile_input_identity = _compile_input_identity(source_snapshot)
        source_key, source_match, source_candidate = _source_index_match(
            destination, index, session, source_snapshot, compile_input_identity
        )
    else:
        source_path = None
        source_match = None
    object_match = None
    object_snapshot = None
    if object_path is not None:
        object_path_value = _resolve(object_path, root)
        object_snapshot = _snapshot(object_path_value, "candidate object")
        _validate_candidate_artifact(
            object_path_value, object_snapshot, session, "candidate object"
        )
        object_match = index["object_index"].get(_object_key(session, object_snapshot["sha256"]))
    loaded_matches: dict[str, Mapping[str, Any]] = {}
    if source_match is not None and source_candidate is not None:
        loaded_matches[source_match] = source_candidate
    for matched_id in {item for item in (source_match, object_match) if item is not None}:
        if matched_id not in loaded_matches:
            loaded_matches[matched_id] = _load_candidate(destination, matched_id, session)
        _require_candidate_compile_attestation(loaded_matches[matched_id], session)
    if source_match and loaded_matches[source_match].get("source_context_key") != source_key:
        _fail("source/context index does not match its immutable candidate record")
    if object_match and loaded_matches[object_match].get("object_result_key") != _object_key(
        session, str(object_snapshot["sha256"])
    ):
        _fail("object index does not match its immutable candidate record")
    conflict = bool(
        source_match
        and object_snapshot is not None
        and _compile_context_complete(session["request"]["context"])
        and object_match != source_match
    )
    status = "new"
    if conflict:
        status = "conflict"
    elif source_match:
        status = "known_source"
    elif object_match:
        status = "known_object"
    if source_snapshot is not None and source_path is not None:
        _recheck_live_snapshot(source_path, source_snapshot, "candidate source")
    if object_snapshot is not None and object_path is not None:
        _recheck_live_snapshot(object_path_value, object_snapshot, "candidate object")
    object_reuse_candidate_id = (
        source_match
        if source_match is not None and not conflict
        else None
    )
    return {
        "status": status,
        "source_sha256": source_snapshot["sha256"] if source_snapshot else None,
        "object_sha256": object_snapshot["sha256"] if object_snapshot else None,
        "source_candidate_id": source_match,
        "object_candidate_id": object_match,
        "source_context_key": source_key,
        "skip_compile": bool(source_match) and _compile_context_complete(session["request"]["context"]) and not conflict,
        # A known object alone does not prove that the requested diagnostic
        # manifest has run.  Diagnose remains cheap because exact fingerprints
        # are content-addressed and return cached results.
        "skip_diagnostics": False,
        "diagnostic_reuse_candidate_id": object_match,
        # A source/context hit has an authenticated object CAS even when the
        # request omitted compiler dependency descriptors.  Keep compilation
        # conservative, but advertise the explicit materialization command so
        # callers do not pay for a redundant compile merely to recover bytes
        # that are already bound to this exact source/context key.
        "object_reuse_candidate_id": object_reuse_candidate_id,
        "object_reuse_available": object_reuse_candidate_id is not None,
        "reason": (
            "the same frozen source/context produced a different object; compiler inputs must be re-authenticated"
            if conflict
            else "source/context is known but the frozen compile context is incomplete; exact object is available for explicit materialize"
            if source_match and not _compile_context_complete(session["request"]["context"])
            else "object bytes are known; run diagnose to reuse matching fingerprints"
            if object_match
            else None
        ),
        "authority_advanced": False,
    }


def materialize_candidate_object(
    root: Path,
    workspace: Path | str,
    candidate_id: str,
    source: Path | str,
    object_path: Path | str,
) -> dict[str, Any]:
    """Materialize an authenticated candidate object without compiling.

    This operation is deliberately separate from ``lookup`` and never changes
    ``skip_compile``.  It is safe with an incomplete compiler context because
    it copies only the candidate's already-recorded object CAS, after binding
    the live source path and bytes back to the immutable source/context key.
    """
    root = root.resolve()
    destination = _workspace(workspace, root)
    session = _load_session(destination, root)
    candidate_id = _identifier(candidate_id, "candidate_id")
    candidate = _load_candidate(destination, candidate_id, session)
    _require_candidate_compile_attestation(candidate, session)

    source_path = _resolve(source, root)
    source_snapshot = _snapshot(source_path, "materialize source")
    compile_input_identity = _compile_input_identity(source_snapshot)
    source_key = _context_key(
        session, source_snapshot["sha256"], compile_input_identity
    )
    if (
        candidate.get("source", {}).get("sha256") != source_snapshot["sha256"]
        or candidate.get("source_context_key") != source_key
    ):
        _fail(
            "candidate source/context does not match the requested source; "
            "materialization is refused"
        )
    _recheck_live_snapshot(source_path, source_snapshot, "materialize source")

    target = _resolve(object_path, root)
    frozen_target = _resolve(
        str(session["request"]["target"]["path"]), root
    )
    if target == source_path:
        _fail("materialization destination must differ from the source")
    if target == frozen_target:
        _fail("materialization destination must differ from the frozen target")
    try:
        target.relative_to(destination)
    except ValueError:
        pass
    else:
        _fail("materialization destination must be outside the workbench")
    _validate_target_path(target, allow_missing_leaf=True, label="materialization destination")
    expected_parent_identity = _directory_identity(
        target.parent, "materialization destination parent"
    )
    object_blob = candidate.get("object_blob")
    if not isinstance(object_blob, Mapping):
        _fail("candidate object CAS descriptor is missing")
    cas_path = _contained(
        destination, object_blob.get("cas_path"), "candidate object CAS"
    )

    if target.exists():
        current = _snapshot(target, "materialization destination")
        if (
            current["sha256"] == candidate["object"]["sha256"]
            and current["size_bytes"] == candidate["object"]["size_bytes"]
        ):
            _recheck_live_snapshot(source_path, source_snapshot, "materialize source")
            return {
                "status": "unchanged",
                "workspace": os.fspath(destination),
                "candidate_id": candidate_id,
                "source": {
                    key: source_snapshot[key]
                    for key in ("path", "size_bytes", "sha256")
                },
                "object": {
                    key: current[key] for key in ("path", "size_bytes", "sha256")
                },
                "source_context_key": source_key,
                "authority_advanced": False,
            }

    restored = _restore_target_from_cas(
        target,
        cas_path,
        candidate["object"],
        expected_parent_identity,
    )
    _recheck_live_snapshot(source_path, source_snapshot, "materialize source")
    return {
        "status": "materialized",
        "workspace": os.fspath(destination),
        "candidate_id": candidate_id,
        "source": {
            key: source_snapshot[key] for key in ("path", "size_bytes", "sha256")
        },
        "object": restored,
        "source_context_key": source_key,
        "authority_advanced": False,
    }


def _copy_authenticated_file(
    source: Path, target: Any, expected: Mapping[str, Any], label: str
) -> None:
    """Copy one authenticated file through a stable handle and path identity."""
    _assert_no_indirection(source)
    try:
        before_link = source.lstat()
        flags = os.O_RDONLY
        if hasattr(os, "O_BINARY"):
            flags |= os.O_BINARY
        handle = os.open(os.fspath(source), flags)
    except OSError as exc:
        raise MatchError(f"cannot open {label}: {source}: {exc}") from exc
    try:
        before = os.fstat(handle)
        if not stat.S_ISREG(before.st_mode):
            _fail(f"{label} is not a regular file: {source}")
        if before.st_nlink != 1:
            _fail(f"{label} must have exactly one hard link: {source}")
        path_identity = (before_link.st_dev, before_link.st_ino, before_link.st_nlink)
        handle_identity = (before.st_dev, before.st_ino, before.st_nlink)
        if path_identity != handle_identity:
            _fail(f"{label} changed before it was copied: {source}")
        expected_identity = expected.get("identity")
        if isinstance(expected_identity, Mapping):
            _file_identity_matches(_file_identity_payload(before), expected_identity, label)
        digest = hashlib.sha256()
        size = 0
        with os.fdopen(handle, "rb", closefd=False) as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
                size += len(block)
                target.write(block)
        after = os.fstat(handle)
        after_link = source.lstat()
    finally:
        os.close(handle)
    stable_before = (
        before.st_dev,
        before.st_ino,
        before.st_nlink,
        before.st_size,
        before.st_mtime_ns,
    )
    stable_after = (
        after.st_dev,
        after.st_ino,
        after.st_nlink,
        after.st_size,
        after.st_mtime_ns,
    )
    if stable_before != stable_after:
        _fail(f"{label} changed while it was copied: {source}")
    if (after_link.st_dev, after_link.st_ino, after_link.st_nlink) != (
        after.st_dev,
        after.st_ino,
        after.st_nlink,
    ):
        _fail(f"{label} path changed while it was copied: {source}")
    if size != expected.get("size_bytes") or digest.hexdigest() != expected.get("sha256"):
        _fail(f"{label} changed from its authenticated snapshot")


def _copy_blob(workspace: Path, source: Path, kind: str, snapshot: Mapping[str, Any]) -> dict[str, Any]:
    sha = str(snapshot["sha256"])
    output = workspace / "cas" / "blobs" / kind / sha[:2] / f"{sha}.bin"
    _safe_mkdir(output.parent)
    dedup_hit = output.is_file()
    if dedup_hit:
        current = _snapshot(output, f"cached {kind} blob")
        if current["sha256"] != sha or current["size_bytes"] != snapshot["size_bytes"]:
            _fail(f"content-addressed {kind} blob collision: {output}")
    else:
        temporary: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                "wb", dir=output.parent, prefix=f".{sha}.", suffix=".tmp", delete=False
            ) as target:
                temporary = Path(target.name)
                _copy_authenticated_file(source, target, snapshot, f"candidate {kind}")
                target.flush()
                os.fsync(target.fileno())
            if _sha256_file(temporary) != sha or temporary.stat().st_size != snapshot["size_bytes"]:
                _fail(f"{kind} changed while copying to content-addressed storage")
            os.replace(temporary, output)
        finally:
            if temporary is not None and temporary.exists():
                temporary.unlink()
    _recheck_live_snapshot(source, snapshot, f"candidate {kind}")
    cached = _snapshot(output, f"cached {kind} blob")
    return {
        "kind": kind,
        "sha256": sha,
        "size_bytes": cached["size_bytes"],
        "cas_path": output.relative_to(workspace).as_posix(),
        "dedup_hit": dedup_hit,
    }


def _assessment_number(value: Any) -> int | float | None:
    """Normalize the numeric fields emitted by the objdiff JSON variants."""
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return value if math.isfinite(value) else None
    if not isinstance(value, str):
        return None
    text = value.strip().replace(",", "")
    if text.endswith("%"):
        text = text[:-1].strip()
    if not text:
        return None
    try:
        if re.fullmatch(r"[+-]?0[xX][0-9a-fA-F]+", text):
            return int(text, 0)
        number = float(text)
    except ValueError:
        return None
    if not math.isfinite(number):
        return None
    return int(number) if number.is_integer() else number


def _assessment_field(value: Mapping[str, Any], *names: str) -> Any:
    for name in names:
        if name in value:
            return value[name]
    return None


def _assessment_report_sides(value: Any, label: str) -> tuple[list[Any], list[Any]]:
    """Return target/source symbol arrays, rejecting unknown report shapes."""
    if not isinstance(value, Mapping):
        _fail(f"{label} root must be an object with paired left/right sides")

    nested = value.get("report")
    if "left" not in value and "right" not in value and isinstance(nested, Mapping):
        return _assessment_report_sides(nested, label)
    if "left" not in value or "right" not in value:
        _fail(f"{label} lacks paired left/right symbol sides")

    def symbols(side: Any, side_label: str) -> list[Any]:
        if not isinstance(side, Mapping):
            _fail(f"{label} {side_label} side must be an object")
        if "symbols" not in side or not isinstance(side["symbols"], list):
            _fail(f"{label} {side_label} side lacks a symbol array")
        rows = side["symbols"]
        if any(not isinstance(row, Mapping) for row in rows):
            _fail(f"{label} {side_label} symbol array contains a non-object row")
        return rows

    return symbols(value["left"], "left"), symbols(value["right"], "right")


def _assessment_is_function(value: Any) -> bool:
    if not isinstance(value, Mapping):
        return False
    kind = value.get("kind")
    if isinstance(kind, str):
        normalized = kind.upper().replace("-", "_")
        if normalized in {"SYMBOL_FUNCTION", "FUNCTION"}:
            return True
        if normalized and normalized != "SYMBOL":
            return False
    # Flat ``functions`` report variants commonly omit ``kind``.
    return any(
        name in value
        for name in ("match_percent", "matchPercent", "similarity", "instructions", "target_symbol")
    )


def _assessment_name(value: Mapping[str, Any], index: int) -> str:
    name = _assessment_field(value, "name", "symbol", "function")
    if isinstance(name, str) and name.strip():
        return name.strip()
    return f"<unnamed:{index}>"


def _assessment_pair_right(left: Mapping[str, Any], right: list[Any]) -> tuple[int | None, Mapping[str, Any] | None]:
    """Pair a canonical objdiff left symbol by its target-side index.

    Canonical reports carry an explicit ``target_symbol`` relation.  Never
    infer that relation from a matching name: a stale, missing, null, or
    out-of-range index must remain unpaired and be rejected by focus checks.
    Flat report adapters, if added later, must opt into their own pairing
    policy before reaching this canonical helper.
    """
    target_index = left.get("target_symbol")
    if isinstance(target_index, int) and not isinstance(target_index, bool):
        if (
            0 <= target_index < len(right)
            and isinstance(right[target_index], Mapping)
            and _assessment_is_function(right[target_index])
        ):
            return target_index, right[target_index]
    return None, None


def _assessment_diff_kinds(value: Mapping[str, Any]) -> dict[str, int]:
    counts: dict[str, int] = {}
    rows = value.get("instructions")
    if not isinstance(rows, list):
        rows = value.get("diffs")
    if isinstance(rows, list):
        for row in rows:
            if not isinstance(row, Mapping):
                continue
            kind = row.get("diff_kind")
            if isinstance(kind, str) and kind:
                counts[kind] = counts.get(kind, 0) + 1
    raw = _assessment_field(value, "diff_kinds", "diffKinds")
    if isinstance(raw, Mapping):
        for kind, count in raw.items():
            if not isinstance(kind, str):
                continue
            number = _assessment_number(count)
            if number is not None:
                counts[kind] = int(number)
    return dict(sorted(counts.items()))


def _assessment_metric(
    left: Mapping[str, Any],
    right: Mapping[str, Any] | None,
    name: str,
) -> dict[str, Any]:
    right_value = right if isinstance(right, Mapping) else {}
    diff_kinds = _assessment_diff_kinds(left)
    target_size = _assessment_number(_assessment_field(left, "size", "target_size", "size_bytes"))
    candidate_size = _assessment_number(
        _assessment_field(right_value, "size", "candidate_size", "size_bytes")
    ) if right is not None else None
    paired_symbol = (
        _assessment_field(right_value, "name", "symbol", "function")
        if right is not None
        else None
    )
    if not isinstance(paired_symbol, str) or not paired_symbol.strip():
        paired_symbol = None
    else:
        paired_symbol = paired_symbol.strip()
    match_percent = _assessment_number(
        _assessment_field(left, "match_percent", "matchPercent", "similarity")
    )
    diff_rows = sum(diff_kinds.values())
    paired = right is not None
    exact = paired and match_percent == 100.0 and diff_rows == 0
    return {
        "symbol": name,
        # ``size`` is the target-side size, while both sides are retained for
        # callers that need to see source expansion/shrinkage explicitly.
        "size": target_size,
        "target_size": target_size,
        "candidate_size": candidate_size,
        "paired_symbol": paired_symbol,
        "match_percent": match_percent,
        "match": match_percent,
        "diff_rows": diff_rows,
        "diff_kinds": diff_kinds,
        "diff_kind": diff_kinds,
        "exact": bool(exact),
        "paired": paired,
    }


def _assessment_records(value: Any, label: str) -> tuple[list[dict[str, Any]], dict[str, int]]:
    left, right = _assessment_report_sides(value, label)
    records: list[dict[str, Any]] = []
    occurrences: dict[str, int] = {}
    exact_count = 0
    for index, item in enumerate(left):
        if not isinstance(item, Mapping):
            _fail(f"{label} left symbol array contains a non-object row")
        if not _assessment_is_function(item):
            continue
        left_value = item
        name = _assessment_name(left_value, index)
        occurrence = occurrences.get(name, 0) + 1
        occurrences[name] = occurrence
        identity = name if occurrence == 1 else f"{name}#{occurrence}"
        _, right_value = _assessment_pair_right(left_value, right)
        metric = _assessment_metric(left_value, right_value, name)
        exact_count += int(metric["exact"])
        records.append(
            {
                "identity": identity,
                "name": name,
                "occurrence": occurrence,
                "metric": metric,
            }
        )
    if not records:
        _fail(f"{label} contains no function symbols")
    return records, {"exact": exact_count, "total": len(records)}


def _residual_metric(metric: Mapping[str, Any]) -> dict[str, Any]:
    """Keep the stable, per-report facts needed by residual consumers."""
    return {
        "target_size": metric.get("target_size"),
        "candidate_size": metric.get("candidate_size"),
        "match_percent": metric.get("match_percent"),
        "diff_rows": metric.get("diff_rows"),
        "diff_kinds": dict(metric.get("diff_kinds", {})),
        "exact": bool(metric.get("exact")),
        "paired": bool(metric.get("paired")),
        "paired_symbol": metric.get("paired_symbol"),
    }


def _residual_symbols(value: Any, label: str) -> tuple[str, ...]:
    """Normalize optional exclusion names without changing caller ordering."""
    if value is None or (
        isinstance(value, Sequence)
        and not isinstance(value, (str, bytes, bytearray))
        and not value
    ):
        return ()
    return _focus_symbols(value, label)


def rank_residuals(
    root: Path,
    *,
    strict_report: Path | str,
    data_report: Path | str,
    exclude_symbols: Sequence[str] | str | None = None,
) -> dict[str, Any]:
    """Rank nonexact functions from one strict/data report pair.

    The two reports must describe the same target-side function identities and
    each target function must have an explicit objdiff target-index pairing in
    both reports.  This keeps a missing or stale report from silently becoming
    a residual classification.  Exact functions are omitted automatically;
    callers may additionally exclude named functions whose known status is
    tracked outside the report pair.
    """
    root = root.resolve()
    excluded = _residual_symbols(exclude_symbols, "residual exclusion symbols")
    _, strict_descriptor, strict_value = _assessment_file(
        strict_report, root, "strict residual report"
    )
    _, data_descriptor, data_value = _assessment_file(
        data_report, root, "data residual report"
    )
    strict_records, strict_counts = _assessment_records(
        strict_value, "strict residual report"
    )
    data_records, data_counts = _assessment_records(
        data_value, "data residual report"
    )
    strict_by_id = {record["identity"]: record for record in strict_records}
    data_by_id = {record["identity"]: record for record in data_records}
    if set(strict_by_id) != set(data_by_id):
        strict_only = sorted(set(strict_by_id) - set(data_by_id))
        data_only = sorted(set(data_by_id) - set(strict_by_id))
        detail: list[str] = []
        if strict_only:
            detail.append("strict-only identities: " + ", ".join(strict_only))
        if data_only:
            detail.append("data-only identities: " + ", ".join(data_only))
        _fail("strict/data function identity mismatch (" + "; ".join(detail) + ")")

    for identity in sorted(strict_by_id):
        strict_metric = strict_by_id[identity]["metric"]
        data_metric = data_by_id[identity]["metric"]
        if not strict_metric.get("paired") or not data_metric.get("paired"):
            _fail(f"strict/data residual function {identity!r} is not paired")
        if strict_metric.get("paired_symbol") != data_metric.get("paired_symbol"):
            _fail(f"strict/data residual pairing mismatch for {identity!r}")

    residuals: list[dict[str, Any]] = []
    excluded_residual_count = 0
    excluded_function_count = 0
    classification_counts = {
        "strict_only": 0,
        "data_only": 0,
        "both": 0,
    }
    excluded_set = set(excluded)
    for identity in sorted(strict_by_id):
        strict_record = strict_by_id[identity]
        data_record = data_by_id[identity]
        symbol = str(strict_record["name"])
        is_excluded = symbol in excluded_set
        strict_metric = strict_record["metric"]
        data_metric = data_record["metric"]
        strict_nonexact = not bool(strict_metric.get("exact"))
        data_nonexact = not bool(data_metric.get("exact"))
        if is_excluded:
            excluded_function_count += 1
        if not strict_nonexact and not data_nonexact:
            continue
        if is_excluded:
            excluded_residual_count += 1
            continue
        if strict_nonexact and data_nonexact:
            classification = "both"
        elif strict_nonexact:
            classification = "strict_only"
        else:
            classification = "data_only"
        classification_counts[classification] += 1
        residuals.append(
            {
                "symbol": symbol,
                "occurrence": int(strict_record["occurrence"]),
                "classification": classification,
                "strict": _residual_metric(strict_metric),
                "data": _residual_metric(data_metric),
            }
        )

    def ranking_key(row: Mapping[str, Any]) -> tuple[Any, ...]:
        metrics = [row["strict"], row["data"]]
        percentages = [
            float(metric["match_percent"])
            for metric in metrics
            if isinstance(metric.get("match_percent"), (int, float))
            and not isinstance(metric.get("match_percent"), bool)
        ]
        diff_rows = [
            int(metric["diff_rows"])
            for metric in metrics
            if isinstance(metric.get("diff_rows"), int)
            and not isinstance(metric.get("diff_rows"), bool)
        ]
        target_sizes = [
            int(metric["target_size"])
            for metric in metrics
            if isinstance(metric.get("target_size"), int)
            and not isinstance(metric.get("target_size"), bool)
        ]
        candidate_sizes = [
            int(metric["candidate_size"])
            for metric in metrics
            if isinstance(metric.get("candidate_size"), int)
            and not isinstance(metric.get("candidate_size"), bool)
        ]
        return (
            min(percentages) if percentages else -math.inf,
            -(max(diff_rows) if diff_rows else 0),
            -(sum(diff_rows) if diff_rows else 0),
            -(max(target_sizes) if target_sizes else 0),
            -(max(candidate_sizes) if candidate_sizes else 0),
            str(row["symbol"]),
            int(row["occurrence"]),
        )

    residuals.sort(key=ranking_key)
    for rank, row in enumerate(residuals, 1):
        row["rank"] = rank

    return {
        "schema": RESIDUALS_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "reports": {
            "strict": strict_descriptor,
            "data": data_descriptor,
        },
        "excluded_symbols": list(excluded),
        "function_counts": {
            "strict": {
                "exact": strict_counts["exact"],
                "total": strict_counts["total"],
                "nonexact": strict_counts["total"] - strict_counts["exact"],
            },
            "data": {
                "exact": data_counts["exact"],
                "total": data_counts["total"],
                "nonexact": data_counts["total"] - data_counts["exact"],
            },
        },
        "classification_counts": classification_counts,
        "residual_count": len(residuals),
        "excluded_function_count": excluded_function_count,
        "excluded_residual_count": excluded_residual_count,
        "ranking": [
            "worst_match_percent_ascending",
            "max_diff_rows_descending",
            "total_diff_rows_descending",
            "max_target_size_descending",
            "max_candidate_size_descending",
            "symbol_ascending",
            "occurrence_ascending",
        ],
        "residuals": residuals,
        "authority_advanced": False,
    }


_STACK_DFORM_LOADS = frozenset(
    {
        "lbz",
        "lbzu",
        "lha",
        "lhau",
        "lhz",
        "lhzu",
        "lwz",
        "lwzu",
        "ld",
        "ldu",
        "lfs",
        "lfsu",
        "lfd",
        "lfdu",
    }
)
_STACK_DFORM_STORES = frozenset(
    {
        "stb",
        "stbu",
        "sth",
        "sthu",
        "stw",
        "stwu",
        "std",
        "stdu",
        "stfs",
        "stfsu",
        "stfd",
        "stfdu",
    }
)
_STACK_DFORM_UPDATE_MNEMONICS = frozenset(
    {
        "lbzu",
        "lhau",
        "lhzu",
        "lwzu",
        "ldu",
        "lfsu",
        "lfdu",
        "stbu",
        "sthu",
        "stwu",
        "stdu",
        "stfsu",
        "stfdu",
    }
)
_STACK_DFORM_WIDTHS = {
    "lbz": 1,
    "lbzu": 1,
    "lha": 2,
    "lhau": 2,
    "lhz": 2,
    "lhzu": 2,
    "lwz": 4,
    "lwzu": 4,
    "ld": 8,
    "ldu": 8,
    "lfs": 4,
    "lfsu": 4,
    "lfd": 8,
    "lfdu": 8,
    "stb": 1,
    "stbu": 1,
    "sth": 2,
    "sthu": 2,
    "stw": 4,
    "stwu": 4,
    "std": 8,
    "stdu": 8,
    "stfs": 4,
    "stfsu": 4,
    "stfd": 8,
    "stfdu": 8,
}
_STACK_NON_MEMORY_MNEMONICS = frozenset({"la", "li", "lis", "lnop", "lwsync"})
_STACK_FRAME_POINTER_UPDATE_MNEMONICS = frozenset({"stwu", "stdu"})
_STACK_PAIRED_SINGLE_MNEMONICS = frozenset({"psq_l", "psq_st"})
_STACK_OUTGOING_CALL_WINDOW = 24
_STACK_OUTGOING_ARGUMENT_REGISTERS = frozenset(
    {f"r{register}" for register in range(3, 11)}
)


def _stack_integer(value: Any, label: str) -> int:
    number = _assessment_number(value)
    if number is None or isinstance(number, bool):
        _fail(f"{label} must be an integer")
    if isinstance(number, float) and not number.is_integer():
        _fail(f"{label} must be an integer")
    return int(number)


def _stack_register_is_base(value: Any) -> bool:
    return isinstance(value, str) and value.strip().lower() in {"r1", "sp"}


def _stack_canonical_base(value: str) -> str:
    """Normalize the two accepted spellings of the stack/frame register."""
    normalized = value.strip().lower()
    return "r1" if normalized in {"r1", "sp"} else normalized


def _stack_byte_range(offset: int, width: int) -> dict[str, int]:
    """Return the deterministic half-open byte range for one stack access."""
    return {"start": offset, "end": offset + width}


def _stack_ranges_overlap(
    left_offset: int,
    left_width: int,
    right_offset: int,
    right_width: int,
) -> bool:
    """Return whether two half-open stack byte ranges share a byte."""
    return (
        left_offset < right_offset + right_width
        and right_offset < left_offset + left_width
    )


def _stack_operand_mentions_base(value: Any) -> bool:
    return isinstance(value, str) and re.search(
        r"(?<![A-Za-z0-9_])(r1|sp)(?![A-Za-z0-9_])", value.lower()
    ) is not None


def _stack_argument_destination(arguments: Sequence[tuple[str, Any]]) -> str | None:
    """Return a canonical GPR argument destination, if one is explicit."""
    if not arguments or arguments[0][0] != "opaque":
        return None
    value = arguments[0][1]
    if not isinstance(value, str):
        return None
    register = value.strip().lower()
    if register in _STACK_OUTGOING_ARGUMENT_REGISTERS:
        return register
    return None


def _stack_direct_call(
    mnemonic: str, arguments: Sequence[tuple[str, Any]]
) -> bool:
    """Recognize only a direct relocatable ``bl`` as call-context proof.

    Indirect calls and branch-and-link variants are intentionally not guessed:
    their target/call identity is not carried by the canonical operand shape
    used here.  A direct ``bl`` with a relocation operand is the narrow form
    emitted by the objdiff reports used by this workbench.
    """
    return mnemonic.lower() == "bl" and list(arguments) == [("reloc", True)]


def _stack_control_flow_barrier(mnemonic: str) -> bool:
    """Recognize branches that end a straight-line call-argument setup."""
    return mnemonic.lower().startswith("b")


def _stack_d_form_stack_access(
    mnemonic: str, arguments: Sequence[tuple[str, Any]]
) -> tuple[str, str, int] | None:
    """Return ``(read/write, base, offset)`` for a canonical stack D-form."""
    memory_kind = _stack_memory_kind(mnemonic)
    if memory_kind not in {"read", "write"} or len(arguments) != 3:
        return None
    if (
        arguments[0][0] != "opaque"
        or arguments[1][0] not in {"signed", "unsigned"}
        or arguments[2][0] != "opaque"
    ):
        return None
    base_register = str(arguments[2][1])
    if not _stack_register_is_base(base_register):
        return None
    return memory_kind, base_register, int(arguments[1][1])


def _stack_paired_single_selector(
    value: Any,
    label: str,
    *,
    selector: str,
) -> int:
    """Decode one canonical paired-single selector without guessing.

    Objdiff represents the PowerPC ``psq_*`` W and I operands as opaque text
    (for example ``"0"`` and ``"qr0"``).  Accept only decimal selectors or
    the explicit ``qrN`` spelling; all other forms are ambiguous and must fail
    closed instead of being treated as a stack access with an invented width.
    """
    if not isinstance(value, str):
        _fail(f"{label} {selector} selector has unsupported operand shape")
    normalized = value.strip().lower()
    if selector == "W" and normalized.startswith("qr"):
        _fail(f"{label} W selector has unsupported operand shape")
    if normalized.startswith("qr"):
        digits = normalized[2:]
    else:
        digits = normalized
    if not digits or not digits.isdecimal():
        _fail(f"{label} {selector} selector has unsupported operand shape")
    number = int(digits)
    return number


def _stack_paired_single_stack_access(
    mnemonic: str,
    arguments: Sequence[tuple[str, Any]],
    label: str,
) -> tuple[str, str, int, int, int, str] | None:
    """Decode a canonical PowerPC quantized paired-single stack access.

    The canonical objdiff shape is ``fN, D(r1), W, qrI``: five parsed
    arguments consisting of an FPR, displacement, stack base, W selector, and
    quantization-register selector.  W=0 transfers two 32-bit elements (8
    bytes); W=1 transfers one element (4 bytes).  The return value includes
    the normalized W/I evidence used by the report.

    A malformed shape returns ``None`` so callers can preserve the existing
    non-stack skip behavior.  Once a stack base is present, invalid selectors
    are rejected by ``_fail``; silently assuming a width would make residue
    evidence unsound.
    """
    normalized = mnemonic.lower()
    if normalized not in _STACK_PAIRED_SINGLE_MNEMONICS:
        return None
    if len(arguments) != 5:
        return None
    if (
        arguments[0][0] != "opaque"
        or arguments[1][0] not in {"signed", "unsigned"}
        or arguments[2][0] != "opaque"
        or arguments[3][0] != "opaque"
        or arguments[4][0] != "opaque"
    ):
        return None
    if not isinstance(arguments[0][1], str) or re.fullmatch(
        r"f(?:[0-9]|[12][0-9]|3[01])", arguments[0][1].strip().lower()
    ) is None:
        return None
    base_register = str(arguments[2][1])
    if not _stack_register_is_base(base_register):
        return None
    w_value = _stack_paired_single_selector(
        arguments[3][1], label, selector="W"
    )
    if w_value not in {0, 1}:
        _fail(f"{label} W selector must be 0 or 1")
    i_value = _stack_paired_single_selector(
        arguments[4][1], label, selector="I"
    )
    if i_value > 7:
        _fail(f"{label} I selector must identify qr0 through qr7")
    width = 8 if w_value == 0 else 4
    memory_kind = "read" if normalized == "psq_l" else "write"
    return (
        memory_kind,
        base_register,
        int(arguments[1][1]),
        width,
        w_value,
        f"qr{i_value}",
    )


def _stack_outgoing_call_contexts(
    instructions: Sequence[Any],
) -> dict[int, dict[str, Any]]:
    """Find stores proven to stage outgoing arguments for a direct call.

    The classifier is deliberately conservative.  A store must be a positive,
    ABI-aligned stack offset (the first outgoing word is ``8(r1)``), the next
    target instruction must begin explicit ``r3``-``r10`` argument setup, and a
    relocatable direct ``bl`` must follow within a short straight-line window.
    No intervening stack access or branch is accepted.  This keeps a dead local
    such as ``stw r0,8(r1)`` in CapEffCrackOMExec a residue candidate even when
    an unrelated call follows shortly afterward.
    """
    parsed: dict[int, tuple[str, list[tuple[str, Any]], str | None]] = {}
    for instruction_index, row in enumerate(instructions):
        if not isinstance(row, Mapping):
            _fail(
                f"stack residue instruction row {instruction_index} is not an object"
            )
        instruction = row.get("instruction")
        if instruction is None:
            continue
        if not isinstance(instruction, Mapping):
            _fail(f"stack residue instruction row {instruction_index} is malformed")
        mnemonic, arguments = _stack_instruction_parts(
            instruction, f"stack residue instruction {instruction_index}"
        )
        parsed[instruction_index] = (
            mnemonic,
            arguments,
            _stack_memory_kind(mnemonic),
        )

    contexts: dict[int, dict[str, Any]] = {}
    parsed_indices = sorted(parsed)
    for position, source_index in enumerate(parsed_indices):
        mnemonic, arguments, _ = parsed[source_index]
        stack_access = _stack_d_form_stack_access(mnemonic, arguments)
        if stack_access is None or stack_access[0] != "write":
            continue
        _, base_register, offset = stack_access
        if offset < 8 or offset % 4 != 0:
            continue
        if _stack_is_frame_pointer_update(mnemonic, arguments):
            continue

        if position + 1 >= len(parsed_indices):
            continue
        first_setup_index = parsed_indices[position + 1]
        if first_setup_index - source_index > _STACK_OUTGOING_CALL_WINDOW:
            continue
        first_setup_destination = _stack_argument_destination(
            parsed[first_setup_index][1]
        )
        if first_setup_destination is None:
            continue
        # The next target instruction must begin explicit argument setup.  This
        # rejects a nearby unrelated call after a dead/local stack write.
        argument_destinations = {first_setup_destination}
        call_index: int | None = None
        for later_index in parsed_indices[position + 1 :]:
            if later_index - source_index > _STACK_OUTGOING_CALL_WINDOW:
                break
            next_mnemonic, next_arguments, next_memory_kind = parsed[later_index]
            if _stack_direct_call(next_mnemonic, next_arguments):
                if len(argument_destinations) >= 2:
                    call_index = later_index
                break
            if _stack_control_flow_barrier(next_mnemonic):
                break
            next_stack_access = _stack_d_form_stack_access(
                next_mnemonic, next_arguments
            )
            if next_stack_access is not None:
                break
            if next_memory_kind == "unsupported" and any(
                kind == "opaque" and _stack_operand_mentions_base(value)
                for kind, value in next_arguments[1:]
            ):
                break
            destination = _stack_argument_destination(next_arguments)
            if destination is not None:
                argument_destinations.add(destination)
        if call_index is None:
            continue
        call_instruction = next(
            instruction.get("instruction")
            for index, instruction in enumerate(instructions)
            if index == call_index and isinstance(instruction, Mapping)
        )
        if not isinstance(call_instruction, Mapping):
            continue
        call_address = call_instruction.get("address")
        parsed_call_address = (
            None
            if call_address is None
            else _stack_integer(call_address, f"stack residue call {call_index}.address")
        )
        call_formatted = call_instruction.get("formatted")
        if call_formatted is not None and not isinstance(call_formatted, str):
            _fail(f"stack residue call {call_index}.formatted must be text")
        contexts[source_index] = {
            "call_instruction_index": call_index,
            "call_address": parsed_call_address,
            "call_formatted": call_formatted,
            "call_mnemonic": parsed[call_index][0],
            "instruction_distance": call_index - source_index,
            "argument_destinations": sorted(
                argument_destinations,
                key=lambda value: int(value[1:]),
            ),
            "proof": "direct_bl_reloc_after_contiguous_argument_setup",
            "stack_base_register": base_register,
            "stack_offset": offset,
            "stack_width": _STACK_DFORM_WIDTHS[mnemonic.lower()],
            "stack_byte_range": _stack_byte_range(
                offset, _STACK_DFORM_WIDTHS[mnemonic.lower()]
            ),
        }
    return contexts


def _stack_instruction_parts(
    instruction: Mapping[str, Any], label: str
) -> tuple[str, list[tuple[str, Any]]]:
    """Decode canonical objdiff instruction parts without parsing asm text."""
    _closed(
        instruction,
        allowed={"address", "size", "formatted", "parts", "relocation", "branch_dest", "line_number"},
        required={"parts"},
        label=label,
    )
    parts = instruction.get("parts")
    if not isinstance(parts, list):
        _fail(f"{label}.parts has unsupported operand shape")
    mnemonic: str | None = None
    arguments: list[tuple[str, Any]] = []
    for part_index, part in enumerate(parts):
        if not isinstance(part, Mapping) or len(part) != 1:
            _fail(f"{label}.parts[{part_index}] has unsupported operand shape")
        key = next(iter(part))
        if key == "basic":
            if not isinstance(part[key], str):
                _fail(f"{label}.parts[{part_index}].basic must be text")
            continue
        if key == "separator":
            if not isinstance(part[key], bool):
                _fail(f"{label}.parts[{part_index}].separator must be boolean")
            continue
        if key == "opcode":
            if mnemonic is not None or not isinstance(part[key], Mapping):
                _fail(f"{label}.parts[{part_index}] has unsupported opcode shape")
            opcode = _closed(
                part[key],
                allowed={"mnemonic", "opcode"},
                required={"mnemonic"},
                label=f"{label}.parts[{part_index}].opcode",
            )
            mnemonic = _text(opcode.get("mnemonic"), f"{label}.opcode.mnemonic").lower()
            if "opcode" in opcode:
                _stack_integer(opcode.get("opcode"), f"{label}.opcode.opcode")
            continue
        if key == "arg":
            if not isinstance(part[key], Mapping) or len(part[key]) != 1:
                _fail(f"{label}.parts[{part_index}] has unsupported argument shape")
            arg = part[key]
            arg_key = next(iter(arg))
            if arg_key in {"signed", "unsigned", "branch_dest"}:
                number = _stack_integer(arg[arg_key], f"{label}.arg.{arg_key}")
                if arg_key == "unsigned" and number < 0:
                    _fail(f"{label}.arg.unsigned must be non-negative")
                arguments.append((arg_key, number))
            elif arg_key == "opaque":
                arguments.append((arg_key, _text(arg[arg_key], f"{label}.arg.opaque")))
            elif arg_key == "reloc":
                if not isinstance(arg[arg_key], bool):
                    _fail(f"{label}.arg.reloc must be boolean")
                arguments.append((arg_key, arg[arg_key]))
            else:
                _fail(f"{label}.parts[{part_index}] has unsupported argument shape")
            continue
        _fail(f"{label}.parts[{part_index}] has unsupported operand shape")
    if mnemonic is None:
        _fail(f"{label} lacks a canonical opcode part")
    return mnemonic, arguments


def _stack_memory_kind(mnemonic: str) -> str | None:
    normalized = mnemonic.lower()
    # Update-base forms are not ordinary direct D-form accesses.  The one
    # recognized exception (negative stwu/stdu frame allocation) is handled
    # explicitly by ``_stack_is_frame_pointer_update`` in the inventory pass.
    if normalized in _STACK_DFORM_UPDATE_MNEMONICS:
        return "unsupported"
    if normalized in _STACK_DFORM_LOADS:
        return "read"
    if normalized in _STACK_DFORM_STORES:
        return "write"
    if normalized in _STACK_NON_MEMORY_MNEMONICS:
        return None
    # Unknown load/store forms are deliberately not guessed.  They may be an
    # indexed, string, vector, reservation, or architecture-specific access.
    # Quantized paired-single forms do not use the l/st prefix, but their
    # operand widths/base semantics are not represented by a direct D-form.
    if normalized.startswith("l") or normalized.startswith("st") or normalized.startswith("psq_"):
        return "unsupported"
    return None


def _stack_is_frame_pointer_update(
    mnemonic: str, arguments: Sequence[tuple[str, Any]]
) -> bool:
    """Recognize only a negative frame-allocation store-update instruction.

    A prologue such as ``stwu r1,-frame(r1)`` writes the caller's stack pointer
    while updating the frame pointer.  It is a real stack access for the full
    inventory, but not a candidate for an unused local/outgoing stack slot.
    Restricting this to the two common word/doubleword forms, matching source
    and base registers, and a negative displacement keeps positive outgoing
    argument offsets eligible for residue review.
    """
    if mnemonic.lower() not in _STACK_FRAME_POINTER_UPDATE_MNEMONICS:
        return False
    if len(arguments) != 3:
        return False
    source_kind, source = arguments[0]
    offset_kind, offset = arguments[1]
    base_kind, base = arguments[2]
    if (
        source_kind != "opaque"
        or offset_kind != "signed"
        or base_kind != "opaque"
        or not _stack_register_is_base(source)
        or not _stack_register_is_base(base)
    ):
        return False
    if str(source).strip().lower() != str(base).strip().lower():
        return False
    return int(offset) < 0


def inspect_stack_residue(
    root: Path,
    *,
    report: Path | str,
    focus_symbol: str | Sequence[str],
) -> dict[str, Any]:
    """Report target stack slots written but never read in one focus function.

    Only canonical objdiff ``instruction.parts`` are interpreted.  Formatted
    assembly strings are retained as evidence but are never parsed, so a new
    or unsupported operand encoding fails closed instead of becoming a false
    zero-read result.  Negative ``stwu``/``stdu`` frame-pointer updates and
    narrowly proven outgoing-call argument stores remain in ``stack_slots`` but
    are explicitly excluded from residue candidates.  The latter require
    canonical direct ``bl`` call context; a positive offset alone is never
    sufficient.
    """
    root = root.resolve()
    focuses = _focus_symbols(focus_symbol, "stack residue focus_symbols")
    if len(focuses) != 1:
        _fail("stack residue requires exactly one focus symbol")
    selected = focuses[0]
    _, report_descriptor, report_value = _assessment_file(
        report, root, "stack residue report"
    )
    left, right = _assessment_report_sides(report_value, "stack residue report")
    matches: list[tuple[int, Mapping[str, Any]]] = []
    for index, value in enumerate(left):
        if not isinstance(value, Mapping):
            _fail(f"stack residue report left symbol {index} is not an object")
        if _assessment_is_function(value) and _assessment_name(value, index) == selected:
            matches.append((index, value))
    if not matches:
        _fail(f"stack residue report lacks requested focus symbol {selected!r}")
    if len(matches) != 1:
        _fail(f"stack residue report contains duplicate focus symbol {selected!r}")
    left_index, focus = matches[0]
    target_index, paired = _assessment_pair_right(focus, right)
    if paired is None or target_index is None:
        _fail(f"stack residue focus symbol {selected!r} is not paired")
    instructions = focus.get("instructions")
    if not isinstance(instructions, list):
        _fail("stack residue focus instructions must be an array")
    outgoing_call_contexts = _stack_outgoing_call_contexts(instructions)

    slots: dict[tuple[str, int, int], dict[str, Any]] = {}
    accesses: list[dict[str, Any]] = []
    target_instruction_count = 0
    target_stack_access_count = 0
    for instruction_index, row in enumerate(instructions):
        if not isinstance(row, Mapping):
            _fail(f"stack residue instruction row {instruction_index} is not an object")
        instruction = row.get("instruction")
        if instruction is None:
            # Insert/delete rows can legitimately have no target instruction.
            continue
        if not isinstance(instruction, Mapping):
            _fail(f"stack residue instruction row {instruction_index} is malformed")
        target_instruction_count += 1
        mnemonic, arguments = _stack_instruction_parts(
            instruction, f"stack residue instruction {instruction_index}"
        )
        memory_kind = _stack_memory_kind(mnemonic)
        if memory_kind is None:
            continue
        mentions_base = any(
            kind == "opaque" and _stack_operand_mentions_base(value)
            for kind, value in arguments[1:]
        )
        frame_pointer_update = _stack_is_frame_pointer_update(mnemonic, arguments)
        paired_single_access: tuple[str, str, int, int, int, str] | None = None
        if memory_kind == "unsupported":
            if frame_pointer_update:
                # The only update-base form admitted into the inventory is the
                # existing negative stwu/stdu prologue exclusion.  Treat it as
                # a write after the narrow proof above so it retains width and
                # byte-range metadata like every other supported access.
                memory_kind = "write"
            elif mnemonic.lower() in _STACK_PAIRED_SINGLE_MNEMONICS:
                paired_single_access = _stack_paired_single_stack_access(
                    mnemonic,
                    arguments,
                    f"stack residue instruction {instruction_index}",
                )
                if paired_single_access is None:
                    if mentions_base:
                        _fail(
                            f"stack residue instruction {instruction_index} has unsupported "
                            f"operand shape for {mnemonic!r}"
                        )
                    continue
                memory_kind = paired_single_access[0]
            elif mentions_base:
                _fail(
                    f"stack residue instruction {instruction_index} has unsupported "
                    f"operand shape for {mnemonic!r}"
                )
            else:
                continue

        if frame_pointer_update:
            # _stack_is_frame_pointer_update validates the canonical three
            # operand shape and matching r1/sp source/base registers.
            base_register = str(arguments[2][1])
            offset = int(arguments[1][1])
            width = _STACK_DFORM_WIDTHS[mnemonic.lower()]
        elif paired_single_access is not None:
            _, base_register, offset, width, _, _ = paired_single_access
        else:
            stack_access = _stack_d_form_stack_access(mnemonic, arguments)
            if stack_access is None:
                if mentions_base:
                    _fail(
                        f"stack residue instruction {instruction_index} has unsupported "
                        f"operand shape for {mnemonic!r}"
                    )
                continue
            _, base_register, offset = stack_access
            width = _STACK_DFORM_WIDTHS[mnemonic.lower()]
        canonical_base = _stack_canonical_base(base_register)
        key = (canonical_base, offset, width)
        slot = slots.get(key)
        if slot is None:
            slot = {
                "offset": offset,
                "width": width,
                "byte_range": _stack_byte_range(offset, width),
                "base_register": canonical_base,
                "write_count": 0,
                "read_count": 0,
                "overlap_read_count": 0,
                "residue_write_count": 0,
                "excluded_write_count": 0,
                "outgoing_call_argument_write_count": 0,
                "residue_candidate": False,
                "excluded_from_residue": False,
                "zero_read_write_count": 0,
                "overlap_evidence": [],
                "overlapping_reads": [],
                "writes": [],
                "reads": [],
            }
            slots[key] = slot
        formatted = instruction.get("formatted")
        if formatted is not None and not isinstance(formatted, str):
            _fail(
                f"stack residue instruction {instruction_index}.formatted must be text"
            )
        address = instruction.get("address")
        parsed_address = (
            None
            if address is None
            else _stack_integer(address, f"stack residue instruction {instruction_index}.address")
        )
        evidence: dict[str, Any] = {
            "instruction_index": instruction_index,
            "address": parsed_address,
            "formatted": formatted,
            "mnemonic": mnemonic,
            "base_register": base_register,
            "canonical_base_register": canonical_base,
            "offset": offset,
            "width": width,
            "byte_range": _stack_byte_range(offset, width),
            "overlap_evidence": [],
        }
        if paired_single_access is not None:
            evidence["operand_form"] = "paired_single"
            evidence["paired_single_w"] = paired_single_access[4]
            evidence["quantization_register"] = paired_single_access[5]
        outgoing_call_argument = (
            memory_kind == "write"
            and instruction_index in outgoing_call_contexts
        )
        evidence["classification"] = (
            "frame_pointer_update"
            if frame_pointer_update
            else "outgoing_call_argument"
            if outgoing_call_argument
            else f"stack_{memory_kind}"
        )
        evidence["excluded_from_residue"] = frame_pointer_update or outgoing_call_argument
        if outgoing_call_argument:
            evidence["call_context"] = outgoing_call_contexts[instruction_index]
        if "diff_kind" in row:
            evidence["diff_kind"] = row.get("diff_kind")
        if memory_kind == "write":
            slot["write_count"] += 1
            if frame_pointer_update:
                slot["excluded_write_count"] += 1
            elif outgoing_call_argument:
                slot["excluded_write_count"] += 1
                slot["outgoing_call_argument_write_count"] += 1
            else:
                slot["residue_write_count"] += 1
            slot["writes"].append(evidence)
        else:
            slot["read_count"] += 1
            slot["reads"].append(evidence)
        accesses.append(
            {
                "kind": memory_kind,
                "base": canonical_base,
                "offset": offset,
                "width": width,
                "slot": slot,
                "evidence": evidence,
            }
        )
        target_stack_access_count += 1

    def _access_sort_key(access: Mapping[str, Any]) -> tuple[int, int, str]:
        evidence = access["evidence"]
        address = evidence.get("address")
        return (
            int(evidence["instruction_index"]),
            -1 if address is None else int(address),
            str(evidence["mnemonic"]),
        )

    def _overlap_reference(access: Mapping[str, Any]) -> dict[str, Any]:
        evidence = access["evidence"]
        return {
            "instruction_index": evidence["instruction_index"],
            "address": evidence["address"],
            "formatted": evidence["formatted"],
            "mnemonic": evidence["mnemonic"],
            "base_register": access["base"],
            "offset": access["offset"],
            "width": access["width"],
            "byte_range": _stack_byte_range(access["offset"], access["width"]),
        }

    read_accesses = [access for access in accesses if access["kind"] == "read"]
    write_accesses = [access for access in accesses if access["kind"] == "write"]
    for access in sorted(accesses, key=_access_sort_key):
        evidence = access["evidence"]
        if access["kind"] == "write":
            counterparts = read_accesses
            evidence_key = "overlapping_reads"
        else:
            counterparts = write_accesses
            evidence_key = "overlapping_writes"
        overlaps = [
            candidate
            for candidate in counterparts
            if candidate["base"] == access["base"]
            and _stack_ranges_overlap(
                access["offset"],
                access["width"],
                candidate["offset"],
                candidate["width"],
            )
        ]
        overlaps.sort(key=_access_sort_key)
        references = [_overlap_reference(candidate) for candidate in overlaps]
        evidence["overlap_evidence"] = references
        evidence[evidence_key] = references
        if access["kind"] == "write":
            evidence["zero_read"] = not overlaps
            evidence["zero_read_reason"] = (
                "no_read_byte_range_overlap"
                if not overlaps
                else "read_byte_range_overlap"
            )

    all_slots = [
        slots[key]
        for key in sorted(slots, key=lambda item: (item[1], item[2], item[0]))
    ]
    for slot in all_slots:
        slot_accesses = [access for access in accesses if access["slot"] is slot]
        slot_reads = [
            access
            for access in read_accesses
            if access["base"] == slot["base_register"]
            and _stack_ranges_overlap(
                slot["offset"],
                slot["width"],
                access["offset"],
                access["width"],
            )
        ]
        slot_reads.sort(key=_access_sort_key)
        slot["overlap_read_count"] = len(slot_reads)
        slot["overlap_evidence"] = [
            _overlap_reference(access) for access in slot_reads
        ]
        slot["overlapping_reads"] = slot["overlap_evidence"]
        residue_writes = [
            access
            for access in slot_accesses
            if access["kind"] == "write"
            and not access["evidence"]["excluded_from_residue"]
        ]
        slot["zero_read_write_count"] = sum(
            bool(access["evidence"]["zero_read"]) for access in residue_writes
        )
        slot["residue_candidate"] = bool(residue_writes) and all(
            access["evidence"]["zero_read"] for access in residue_writes
        )
        slot["excluded_from_residue"] = (
            slot["excluded_write_count"] > 0 and slot["residue_write_count"] == 0
        )
    zero_read_slots = [
        slot
        for slot in all_slots
        if slot["residue_candidate"]
    ]
    excluded_stack_access_count = sum(
        slot["excluded_write_count"] for slot in all_slots
    )
    outgoing_call_argument_access_count = sum(
        slot["outgoing_call_argument_write_count"] for slot in all_slots
    )
    outgoing_call_argument_slot_count = sum(
        1
        for slot in all_slots
        if slot["outgoing_call_argument_write_count"] > 0
    )
    excluded_stack_slot_count = sum(
        1 for slot in all_slots if slot["excluded_from_residue"]
    )
    paired_name = _assessment_name(paired, target_index)
    return {
        "schema": STACK_RESIDUE_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "report": report_descriptor,
        "focus_symbol": selected,
        "target_symbol_index": left_index,
        "paired_symbol_index": target_index,
        "paired_symbol": paired_name,
        "target_instruction_count": target_instruction_count,
        "target_stack_access_count": target_stack_access_count,
        "stack_slot_count": len(all_slots),
        "zero_read_slot_count": len(zero_read_slots),
        "excluded_stack_access_count": excluded_stack_access_count,
        "excluded_stack_slot_count": excluded_stack_slot_count,
        "outgoing_call_argument_access_count": outgoing_call_argument_access_count,
        "outgoing_call_argument_slot_count": outgoing_call_argument_slot_count,
        # ``slots`` is the decision-facing result; ``stack_slots`` preserves
        # the complete access inventory for callers that need context.
        "slots": zero_read_slots,
        "zero_read_slots": zero_read_slots,
        "stack_slots": all_slots,
        "authority_advanced": False,
    }


_DONOR_SOURCE_SUFFIXES = frozenset({".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp"})
_DONOR_MAX_FILES = 4096
_C_DONOR_OPERATORS = tuple(
    sorted(
        {
            "<<=", ">>=", "...", "->*", "::", "++", "--", "->", "+=", "-=",
            "*=", "/=", "%=", "&=", "|=", "^=", "==", "!=", "<=", ">=", "&&",
            "||", "<<", ">>", "##", "->", "+=", "-=", "*=", "/=", "%=", "&=", "|=",
            "^=", "=>",
        },
        key=len,
        reverse=True,
    )
)


def _c_clean_comments(source: str, label: str) -> str:
    """Replace comments with spaces while preserving literals and newlines."""
    result = list(source)
    state = "normal"
    quote = ""
    index = 0
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "normal":
            if char == "/" and next_char == "/":
                result[index] = " "
                result[index + 1] = " "
                state = "line_comment"
                index += 2
                continue
            if char == "/" and next_char == "*":
                result[index] = " "
                result[index + 1] = " "
                state = "block_comment"
                index += 2
                continue
            if char in {"\"", "'"}:
                quote = char
                state = "literal"
            index += 1
            continue
        if state == "line_comment":
            if char == "\n":
                state = "normal"
            else:
                result[index] = " "
            index += 1
            continue
        if state == "block_comment":
            if char == "*" and next_char == "/":
                result[index] = " "
                result[index + 1] = " "
                state = "normal"
                index += 2
                continue
            if char != "\n":
                result[index] = " "
            index += 1
            continue
        # C and C++ literals may contain escaped quotes and escaped newlines.
        if char == "\\":
            if index + 1 >= len(source):
                raise MatchError(f"unterminated literal in {label}")
            if next_char == "\n":
                index += 2
            else:
                index += 2
            continue
        if char == quote:
            state = "normal"
            quote = ""
            index += 1
            continue
        if char == "\n":
            raise MatchError(f"unterminated literal in {label}")
        index += 1
    if state == "block_comment":
        raise MatchError(f"unterminated block comment in {label}")
    if state == "literal":
        raise MatchError(f"unterminated literal in {label}")
    return "".join(result)


def _c_mask_literals(source: str, label: str) -> str:
    """Mask literal contents so braces/parens inside strings cannot parse code."""
    result = list(source)
    state = "normal"
    quote = ""
    index = 0
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "normal":
            if char in {"\"", "'"}:
                quote = char
                state = "literal"
                result[index] = " "
            index += 1
            continue
        if char == "\\":
            result[index] = " "
            if index + 1 >= len(source):
                raise MatchError(f"unterminated literal in {label}")
            if next_char != "\n":
                result[index + 1] = " "
            index += 2
            continue
        if char == quote:
            result[index] = " "
            state = "normal"
            quote = ""
            index += 1
            continue
        if char != "\n":
            result[index] = " "
        index += 1
    if state == "literal":
        raise MatchError(f"unterminated literal in {label}")
    return "".join(result)


def _c_matching_delimiter(masked: str, opening: int, open_char: str, close_char: str, label: str) -> int:
    depth = 0
    for index in range(opening, len(masked)):
        char = masked[index]
        if char == open_char:
            depth += 1
        elif char == close_char:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                break
    raise MatchError(f"unmatched {open_char!r} in {label}")


def _c_function_start(masked: str, identifier: int) -> int:
    boundaries = [masked.rfind(token, 0, identifier) for token in (";", "}", "{")]
    start = max(boundaries, default=-1) + 1
    while start < identifier and masked[start].isspace():
        start += 1
    return start


def _c_extract_function(source: str, focus_symbol: str, label: str) -> dict[str, Any]:
    """Extract exactly one function definition, rejecting uncertain matches."""
    focus = _text(focus_symbol, "focus symbol")
    cleaned = _c_clean_comments(source, label)
    masked = _c_mask_literals(cleaned, label)
    pattern = re.compile(
        rf"(?<![A-Za-z0-9_]){re.escape(focus)}(?![A-Za-z0-9_])"
    )
    candidates: list[dict[str, int]] = []
    for match in pattern.finditer(masked):
        identifier = match.start()
        prefix_start = max(
            masked.rfind(";", 0, identifier),
            masked.rfind("}", 0, identifier),
            masked.rfind("{", 0, identifier),
        ) + 1
        prefix = masked[prefix_start:identifier]
        if re.search(r"\b(?:if|for|while|switch|return)\s*\([^)]*$", prefix):
            continue
        if re.search(r"(?:->|\.)\s*$", prefix):
            continue
        if re.search(r"(?:=|,|\?|:|\+|-|\*|/|%|!|&|\|)\s*$", prefix):
            continue
        after_name = identifier + len(focus)
        while after_name < len(masked) and masked[after_name].isspace():
            after_name += 1
        if after_name >= len(masked) or masked[after_name] != "(":
            continue
        parameter_end = _c_matching_delimiter(
            masked, after_name, "(", ")", f"function {focus_symbol!r} in {label}"
        )
        after_parameters = parameter_end + 1
        body_start: int | None = None
        while after_parameters < len(masked):
            char = masked[after_parameters]
            if char == "{":
                body_start = after_parameters
                break
            if char in {";", "=", "}"} or char == "#":
                break
            after_parameters += 1
        if body_start is None:
            continue
        body_end = _c_matching_delimiter(
            masked, body_start, "{", "}", f"function {focus_symbol!r} in {label}"
        )
        candidates.append(
            {
                "identifier": identifier,
                "start": _c_function_start(masked, identifier),
                "body_start": body_start,
                "body_end": body_end,
            }
        )
    if not candidates:
        raise MatchError(f"focus function {focus_symbol!r} not found in {label}")
    if len(candidates) != 1:
        raise MatchError(
            f"focus function {focus_symbol!r} is ambiguous in {label} "
            f"({len(candidates)} definitions)"
        )
    candidate = candidates[0]
    start = candidate["start"]
    body_start = candidate["body_start"]
    body_end = candidate["body_end"]
    body = source[body_start : body_end + 1]
    function_source = source[start : body_end + 1]
    normalized_lines = _c_normalized_shape_lines(body, f"function {focus_symbol!r} in {label}")
    tokens = _c_tokens(body, f"function {focus_symbol!r} in {label}")
    normalized_tokens = " ".join(tokens)
    return {
        "source_start": start,
        "source_end": body_end + 1,
        "body_start": body_start,
        "body_end": body_end + 1,
        "start_line": source.count("\n", 0, start) + 1,
        "end_line": source.count("\n", 0, body_end) + 1,
        "body_start_line": source.count("\n", 0, body_start) + 1,
        "body_end_line": source.count("\n", 0, body_end) + 1,
        "body_sha256": _sha256_bytes(body.encode("utf-8")),
        "normalized_body_sha256": _sha256_bytes(normalized_tokens.encode("utf-8")),
        "normalized_body_line_count": len(normalized_lines),
        "normalized_body_lines": normalized_lines,
        "function_sha256": _sha256_bytes(function_source.encode("utf-8")),
    }


def _c_normalized_shape_lines(source: str, label: str) -> list[str]:
    cleaned = _c_clean_comments(source, label)
    result: list[str] = []
    line: list[str] = []
    pending_space = False
    state = "normal"
    quote = ""
    index = 0
    while index < len(cleaned):
        char = cleaned[index]
        if state == "normal":
            if char in {"\"", "'"}:
                if pending_space and line:
                    line.append(" ")
                pending_space = False
                line.append(char)
                state = "literal"
                quote = char
            elif char in " \t\f\v\r":
                pending_space = bool(line)
            elif char == "\n":
                value = "".join(line).strip()
                if value:
                    result.append(value)
                line = []
                pending_space = False
            else:
                if pending_space and line:
                    line.append(" ")
                pending_space = False
                line.append(char)
            index += 1
            continue
        line.append(char)
        if char == "\\" and index + 1 < len(cleaned):
            index += 1
            line.append(cleaned[index])
        elif char == quote:
            state = "normal"
            quote = ""
        index += 1
    if state == "literal":
        raise MatchError(f"unterminated literal in {label}")
    value = "".join(line).strip()
    if value:
        result.append(value)
    return result


def _c_tokens(source: str, label: str) -> list[str]:
    cleaned = _c_clean_comments(source, label)
    tokens: list[str] = []
    index = 0
    while index < len(cleaned):
        char = cleaned[index]
        if char.isspace():
            index += 1
            continue
        if char in {"\"", "'"}:
            quote = char
            start = index
            index += 1
            while index < len(cleaned):
                if cleaned[index] == "\\":
                    index += 2
                    continue
                if cleaned[index] == quote:
                    index += 1
                    break
                index += 1
            else:
                raise MatchError(f"unterminated literal in {label}")
            tokens.append(cleaned[start:index])
            continue
        if char.isalpha() or char == "_":
            start = index
            index += 1
            while index < len(cleaned) and (cleaned[index].isalnum() or cleaned[index] == "_"):
                index += 1
            tokens.append(cleaned[start:index])
            continue
        if char.isdigit() or (
            char == "." and index + 1 < len(cleaned) and cleaned[index + 1].isdigit()
        ):
            start = index
            index += 1
            while index < len(cleaned) and (cleaned[index].isalnum() or cleaned[index] in "._"):
                index += 1
            tokens.append(cleaned[start:index])
            continue
        operator = next(
            (item for item in _C_DONOR_OPERATORS if cleaned.startswith(item, index)),
            None,
        )
        if operator is not None:
            tokens.append(operator)
            index += len(operator)
        else:
            tokens.append(char)
            index += 1
    return tokens


def _donor_display_path(path: Path, root: Path) -> str:
    absolute = Path(os.path.abspath(path))
    try:
        relative = absolute.relative_to(root.resolve())
    except ValueError:
        return absolute.as_posix()
    return relative.as_posix()


def _donor_path_key(path: Path) -> str:
    return os.path.normcase(os.path.normpath(os.path.abspath(path)))


def _donor_file_descriptor(path: Path, root: Path, data: bytes) -> dict[str, Any]:
    return {
        "path": _donor_display_path(path, root),
        "size_bytes": len(data),
        "sha256": _sha256_bytes(data),
    }


def _donor_read_utf8(path: Path, label: str) -> tuple[str, bytes]:
    try:
        data = path.read_bytes()
    except FileNotFoundError as exc:
        raise MatchError(f"{label} does not exist: {path}") from exc
    except OSError as exc:
        raise MatchError(f"cannot read {label} {path}: {exc}") from exc
    try:
        return data.decode("utf-8"), data
    except UnicodeDecodeError as exc:
        raise MatchError(f"{label} is not UTF-8: {path}") from exc


def _donor_candidate_paths(
    root: Path,
    donor_files: Sequence[str] | None,
    search_roots: Sequence[str] | None,
    *,
    max_files: int,
) -> tuple[list[tuple[Path, bool]], dict[str, list[str]]]:
    if max_files < 1 or max_files > 100_000:
        raise MatchError("donor max-files must be between 1 and 100000")
    explicit = [_resolve(value, root) for value in (donor_files or [])]
    roots = [_resolve(value, root) for value in (search_roots or [])]
    if not explicit and not roots:
        raise MatchError("donor-shapes requires at least one donor-file or search-root")
    selected: dict[str, tuple[Path, bool]] = {}
    for path in explicit:
        _assert_no_indirection(path)
        _validate_candidate_artifact_path(path, "donor-file")
        if not path.is_file():
            raise MatchError(f"donor-file is not a regular file: {path}")
        selected[_donor_path_key(path)] = (path, True)
        if len(selected) > max_files:
            raise MatchError(
                f"donor scope exceeds max-files={max_files}; narrow the explicit donor files"
            )
    for search_root in roots:
        _assert_no_indirection(search_root)
        _validate_candidate_artifact_path(search_root, "donor search-root")
        if not search_root.is_dir():
            raise MatchError(f"donor search-root is not a directory: {search_root}")
        for current, directories, files in os.walk(search_root, topdown=True, followlinks=False):
            directories[:] = sorted(
                name
                for name in directories
                if not (Path(current) / name).is_symlink()
            )
            for name in sorted(files):
                path = Path(current) / name
                if path.suffix.lower() not in _DONOR_SOURCE_SUFFIXES or path.is_symlink():
                    continue
                _assert_no_indirection(path)
                _validate_candidate_artifact_path(path, "donor source")
                selected.setdefault(_donor_path_key(path), (path, False))
                if len(selected) > max_files:
                    raise MatchError(
                        f"donor search scope exceeds max-files={max_files}; narrow the explicit roots"
                    )
    paths = sorted(selected.values(), key=lambda item: _donor_display_path(item[0], root))
    scope = {
        "donor_files": sorted(_donor_display_path(path, root) for path in explicit),
        "search_roots": sorted(_donor_display_path(path, root) for path in roots),
    }
    return paths, scope


def donor_shapes(
    root: Path,
    *,
    source: Path | str,
    focus_symbol: str | Sequence[str],
    donor_files: Sequence[str] | None = None,
    search_roots: Sequence[str] | None = None,
    max_files: int = _DONOR_MAX_FILES,
) -> dict[str, Any]:
    """Mine donor source shapes without compiling, editing, or claiming proof."""
    root = root.resolve()
    focuses = _focus_symbols(focus_symbol, "donor focus symbols")
    if len(focuses) != 1:
        raise MatchError("donor-shapes requires exactly one focus symbol")
    focus = focuses[0]
    source_path = _resolve(source, root)
    _assert_no_indirection(source_path)
    _validate_candidate_artifact_path(source_path, "current source")
    if not source_path.is_file():
        raise MatchError(f"current source is not a regular file: {source_path}")
    source_text, source_bytes = _donor_read_utf8(source_path, "current source")
    current_function = _c_extract_function(source_text, focus, f"current source {source_path}")
    current_descriptor = _donor_file_descriptor(source_path, root, source_bytes)
    paths, scope = _donor_candidate_paths(
        root, donor_files, search_roots, max_files=max_files
    )
    donor_records: list[dict[str, Any]] = []
    skipped_missing = 0
    for path, explicit in paths:
        text, data = _donor_read_utf8(path, "donor source")
        if not explicit and focus not in text:
            skipped_missing += 1
            continue
        try:
            function = _c_extract_function(text, focus, f"donor source {path}")
        except MatchError as exc:
            if not explicit and "not found" in str(exc):
                skipped_missing += 1
                continue
            raise
        descriptor = _donor_file_descriptor(path, root, data)
        donor_records.append(
            {
                "source": descriptor,
                "function_sha256": function["function_sha256"],
                "body_sha256": function["body_sha256"],
                "normalized_body_sha256": function["normalized_body_sha256"],
                "source_span": {
                    "start_line": function["start_line"],
                    "end_line": function["end_line"],
                    "body_start_line": function["body_start_line"],
                    "body_end_line": function["body_end_line"],
                },
                "normalized_body_line_count": function["normalized_body_line_count"],
                "normalized_body_lines": function["normalized_body_lines"],
            }
        )
    if not donor_records:
        raise MatchError(
            f"no donor definition of focus function {focus!r} found in the explicit scope"
        )

    groups: dict[str, list[dict[str, Any]]] = {}
    for record in donor_records:
        groups.setdefault(str(record["normalized_body_sha256"]), []).append(record)
    current_lines = current_function["normalized_body_lines"]
    variants: list[dict[str, Any]] = []
    for body_hash, records in groups.items():
        records.sort(key=lambda value: str(value["source"]["path"]))
        representative = records[0]
        diff = list(
            difflib.unified_diff(
                current_lines,
                representative["normalized_body_lines"],
                fromfile=f"current:{current_descriptor['path']}",
                tofile=f"donor:{representative['source']['path']}",
                n=1,
                lineterm="",
            )
        )
        variants.append(
            {
                "normalized_body_sha256": body_hash,
                "donor_count": len(records),
                "representative": {
                    "path": representative["source"]["path"],
                    "sha256": representative["source"]["sha256"],
                    "body_sha256": representative["body_sha256"],
                },
                "source_shape_diff": diff,
                "source_shape_diff_line_count": len(diff),
                "donors": [
                    {
                        key: value
                        for key, value in record.items()
                        if key != "normalized_body_lines"
                    }
                    for record in records
                ],
            }
        )
    variants.sort(
        key=lambda value: (
            -int(value["donor_count"]),
            str(value["representative"]["path"]),
            str(value["normalized_body_sha256"]),
        )
    )
    for rank, variant in enumerate(variants, 1):
        variant["rank"] = rank
    return {
        "schema": DONOR_SHAPES_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbol": focus,
        "current": {
            "source": current_descriptor,
            "function_sha256": current_function["function_sha256"],
            "body_sha256": current_function["body_sha256"],
            "normalized_body_sha256": current_function["normalized_body_sha256"],
            "source_span": {
                "start_line": current_function["start_line"],
                "end_line": current_function["end_line"],
                "body_start_line": current_function["body_start_line"],
                "body_end_line": current_function["body_end_line"],
            },
            "normalized_body_line_count": current_function["normalized_body_line_count"],
        },
        "scope": {
            **scope,
            "source_suffixes": sorted(_DONOR_SOURCE_SUFFIXES),
            "max_files": max_files,
            "scanned_file_count": len(paths),
            "matched_file_count": len(donor_records),
            "skipped_missing_focus_count": skipped_missing,
        },
        "variant_count": len(variants),
        "donor_definition_count": len(donor_records),
        "ranking": [
            "donor_count_descending",
            "representative_path_ascending",
            "normalized_body_sha256_ascending",
        ],
        "variants": variants,
        "evidence_class": "donor_source_shape_only",
        "target_proof": False,
        "auto_edit": False,
        "authority_advanced": False,
    }


def _donor_source_kind(value: Any, label: str = "donor source_kind") -> str:
    """Normalize the bounded provenance vocabulary used by the registry."""
    result = _text(value, label).casefold().replace("_", "-").replace(" ", "-")
    aliases = {
        "same-tu": "same-tu",
        "same-game-history": "same-game-history",
        "cross-game-lineage": "cross-game-lineage",
        "target-derived": "target-derived",
        "diagnostic-only": "diagnostic-only",
    }
    try:
        return aliases[result]
    except KeyError as exc:
        _fail(
            f"{label} must be one of "
            + ", ".join(sorted(DONOR_SOURCE_KINDS))
        )
        raise AssertionError from exc


def _donor_status(value: Any, label: str = "donor status") -> str:
    result = _text(value, label).casefold()
    if result not in DONOR_STATUSES:
        _fail(f"{label} must be accepted or rejected")
    return result


def _donor_admissibility(
    value: Any,
    *,
    status: str,
    label: str = "donor admissibility",
) -> dict[str, str]:
    if isinstance(value, str):
        decision = _text(value, label).casefold()
        reason = "registered provenance decision"
    elif isinstance(value, Mapping):
        item = _closed(
            value,
            allowed={"decision", "reason"},
            required={"decision", "reason"},
            label=label,
        )
        decision = _text(item["decision"], f"{label}.decision").casefold()
        reason = _text(item["reason"], f"{label}.reason")
    else:
        _fail(f"{label} must be a decision string or object")
    if decision not in DONOR_ADMISSIBILITY:
        _fail(
            f"{label}.decision must be one of "
            + ", ".join(sorted(DONOR_ADMISSIBILITY))
        )
    if status == "accepted" and decision != "admissible":
        _fail("accepted donor shapes require admissibility=admissible")
    if status == "rejected" and decision == "admissible":
        _fail("rejected donor shapes cannot retain admissibility=admissible")
    return {"decision": decision, "reason": reason}


def _donor_id_list(value: Any, label: str) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        values: list[Any] = [value]
    elif isinstance(value, Sequence) and not isinstance(value, (bytes, bytearray)):
        values = list(value)
    else:
        _fail(f"{label} must be a string or sequence of strings")
    result = sorted({_identifier(item, f"{label}[{index}]") for index, item in enumerate(values)})
    return result


def _donor_alias_key(value: str) -> str:
    return value.casefold()


def _donor_registry_path(root: Path, value: Path | str) -> Path:
    path = _resolve(value, root)
    try:
        path.relative_to(root.resolve())
    except ValueError:
        _fail(f"donor registry must stay beneath the selected root: {path}")
    if path.suffix.casefold() != ".json":
        _fail(f"donor registry must be a JSON file: {path}")
    _safe_parent(path)
    _assert_no_indirection(path, allow_missing_leaf=True)
    if path.exists():
        _validate_target_path(path, label="donor registry")
    return path


def _donor_source_descriptor(path: Path, label: str = "donor source") -> tuple[dict[str, Any], str]:
    """Read and authenticate a source file used by a durable donor record."""
    _assert_no_indirection(path)
    _validate_candidate_artifact_path(path, label)
    if path.suffix.casefold() not in _DONOR_SOURCE_SUFFIXES:
        _fail(
            f"{label} must be a C/C++ source or header file; object artifacts "
            "cannot be donor sources"
        )
    if not path.is_file():
        _fail(f"{label} is not a regular file: {path}")
    text, data = _donor_read_utf8(path, label)
    descriptor = {
        "path": os.fspath(Path(os.path.abspath(path))),
        "size_bytes": len(data),
        "sha256": _sha256_bytes(data),
        "blob_sha256": _sha256_bytes(data),
    }
    return descriptor, text


def _donor_function_scope(
    source_text: str,
    focus_symbol: str,
    label: str,
) -> dict[str, Any]:
    function = _c_extract_function(source_text, focus_symbol, label)
    return {
        "kind": "function",
        "function": focus_symbol,
        "start_line": function["start_line"],
        "end_line": function["end_line"],
        "body_start_line": function["body_start_line"],
        "body_end_line": function["body_end_line"],
        "function_sha256": function["function_sha256"],
        "body_sha256": function["body_sha256"],
        "normalized_body_sha256": function["normalized_body_sha256"],
        "normalized_body_line_count": function["normalized_body_line_count"],
    }


def _donor_evidence_descriptors(
    root: Path,
    paths: Sequence[str] | None,
    source: Mapping[str, Any],
) -> list[dict[str, Any]]:
    values = list(paths or [])
    if not values:
        values = [str(source["path"])]
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    for index, value in enumerate(values):
        path = _resolve(value, root)
        _assert_no_indirection(path)
        if not path.is_file():
            _fail(f"donor evidence[{index}] is not a regular file: {path}")
        descriptor = _snapshot(path, f"donor evidence[{index}]")
        key = _donor_path_key(path)
        if key in seen:
            continue
        seen.add(key)
        result.append(
            {
                "path": descriptor["path"],
                "size_bytes": descriptor["size_bytes"],
                "sha256": descriptor["sha256"],
            }
        )
    result.sort(key=lambda item: str(item["path"]).casefold())
    return result


def _donor_canonical_id(
    *,
    source_kind: str,
    focus_symbol: str,
    source: Mapping[str, Any],
    scope: Mapping[str, Any],
) -> str:
    identity = {
        "source_kind": source_kind,
        "focus_symbol": focus_symbol,
        "source_sha256": source["sha256"],
        "blob_sha256": source["blob_sha256"],
        "function_sha256": scope["function_sha256"],
        "body_sha256": scope["body_sha256"],
        "normalized_body_sha256": scope["normalized_body_sha256"],
        "start_line": scope["start_line"],
        "end_line": scope["end_line"],
    }
    return f"donor-{_sha256_bytes(_canonical(identity))}"


def _donor_record_payload(
    *,
    donor_id: str | None,
    aliases: Sequence[str] | None,
    source_kind: str,
    status: str,
    admissibility: Any,
    focus_symbol: str,
    source: Mapping[str, Any],
    scope: Mapping[str, Any],
    evidence: Sequence[Mapping[str, Any]],
    supersedes: Sequence[str] | None,
    duplicate_of: str | None,
    queried_by: Sequence[str] | None,
    used_by: Sequence[str] | None,
    notes: str | None,
) -> dict[str, Any]:
    canonical_id = _donor_canonical_id(
        source_kind=source_kind,
        focus_symbol=focus_symbol,
        source=source,
        scope=scope,
    )
    requested_aliases = _donor_id_list(aliases, "donor aliases")
    if donor_id is not None:
        requested_aliases.extend(_donor_id_list(donor_id, "donor id"))
    requested_aliases = sorted({item for item in requested_aliases if item != canonical_id})
    record: dict[str, Any] = {
        "canonical_id": canonical_id,
        "aliases": requested_aliases,
        "focus_symbol": focus_symbol,
        "source_kind": source_kind,
        "source": dict(source),
        "scope": dict(scope),
        "admissibility": _donor_admissibility(admissibility, status=status),
        "status": status,
        "supersedes": sorted(set(supersedes or [])),
        "duplicate_of": None if duplicate_of is None else _identifier(duplicate_of, "donor duplicate_of"),
        "duplicates": [],
        "evidence": [dict(item) for item in evidence],
        "queried_by_candidate_ids": sorted(set(queried_by or [])),
        "used_by_candidate_ids": sorted(set(used_by or [])),
        "notes": None if notes is None else _text(notes, "donor notes"),
    }
    return _with_self_hash(record, "record_sha256")


_DONOR_RECORD_FIELDS = frozenset(
    {
        "canonical_id",
        "aliases",
        "focus_symbol",
        "source_kind",
        "source",
        "scope",
        "admissibility",
        "status",
        "supersedes",
        "duplicate_of",
        "duplicates",
        "evidence",
        "queried_by_candidate_ids",
        "used_by_candidate_ids",
        "notes",
        "record_sha256",
    }
)


def _validate_donor_source_descriptor(value: Any, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256", "blob_sha256"},
        required={"path", "size_bytes", "sha256", "blob_sha256"},
        label=label,
    )
    path = _text(item["path"], f"{label}.path")
    if not Path(path).is_absolute():
        _fail(f"{label}.path must be absolute")
    return {
        "path": path,
        "size_bytes": _integer(item["size_bytes"], f"{label}.size_bytes"),
        "sha256": _sha256(item["sha256"], f"{label}.sha256"),
        "blob_sha256": _sha256(item["blob_sha256"], f"{label}.blob_sha256"),
    }


def _validate_donor_scope(value: Any, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={
            "kind",
            "function",
            "start_line",
            "end_line",
            "body_start_line",
            "body_end_line",
            "function_sha256",
            "body_sha256",
            "normalized_body_sha256",
            "normalized_body_line_count",
        },
        required={
            "kind",
            "function",
            "start_line",
            "end_line",
            "body_start_line",
            "body_end_line",
            "function_sha256",
            "body_sha256",
            "normalized_body_sha256",
            "normalized_body_line_count",
        },
        label=label,
    )
    if item["kind"] != "function":
        _fail(f"{label}.kind must be function")
    start = _integer(item["start_line"], f"{label}.start_line", minimum=1)
    end = _integer(item["end_line"], f"{label}.end_line", minimum=start)
    body_start = _integer(item["body_start_line"], f"{label}.body_start_line", minimum=start)
    body_end = _integer(item["body_end_line"], f"{label}.body_end_line", minimum=body_start)
    if body_end > end:
        _fail(f"{label}.body_end_line must not exceed end_line")
    return {
        "kind": "function",
        "function": _text(item["function"], f"{label}.function"),
        "start_line": start,
        "end_line": end,
        "body_start_line": body_start,
        "body_end_line": body_end,
        "function_sha256": _sha256(item["function_sha256"], f"{label}.function_sha256"),
        "body_sha256": _sha256(item["body_sha256"], f"{label}.body_sha256"),
        "normalized_body_sha256": _sha256(
            item["normalized_body_sha256"], f"{label}.normalized_body_sha256"
        ),
        "normalized_body_line_count": _integer(
            item["normalized_body_line_count"], f"{label}.normalized_body_line_count", minimum=1
        ),
    }


def _validate_donor_evidence(value: Any, label: str) -> list[dict[str, Any]]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes, bytearray)) or not value:
        _fail(f"{label} must be a non-empty sequence")
    result: list[dict[str, Any]] = []
    paths: set[str] = set()
    for index, entry in enumerate(value):
        item = _closed(
            entry,
            allowed={"path", "size_bytes", "sha256"},
            required={"path", "size_bytes", "sha256"},
            label=f"{label}[{index}]",
        )
        path = _text(item["path"], f"{label}[{index}].path")
        if not Path(path).is_absolute():
            _fail(f"{label}[{index}].path must be absolute")
        key = _donor_path_key(Path(path))
        if key in paths:
            _fail(f"{label} contains duplicate paths")
        paths.add(key)
        result.append(
            {
                "path": path,
                "size_bytes": _integer(item["size_bytes"], f"{label}[{index}].size_bytes"),
                "sha256": _sha256(item["sha256"], f"{label}[{index}].sha256"),
            }
        )
    return sorted(result, key=lambda item: str(item["path"]).casefold())


def _validate_donor_record(value: Any, label: str) -> dict[str, Any]:
    _verify_self_hash(value, "record_sha256", label)
    item = _closed(
        value,
        allowed=_DONOR_RECORD_FIELDS,
        required=_DONOR_RECORD_FIELDS - {"duplicate_of"},
        label=label,
    )
    canonical_id = _identifier(item["canonical_id"], f"{label}.canonical_id")
    source_kind = _donor_source_kind(item["source_kind"], f"{label}.source_kind")
    if source_kind == "target-derived":
        _fail(f"{label} target-derived source cannot be stored as a donor record")
    status = _donor_status(item["status"], f"{label}.status")
    source = _validate_donor_source_descriptor(item["source"], f"{label}.source")
    scope = _validate_donor_scope(item["scope"], f"{label}.scope")
    focus = _text(item["focus_symbol"], f"{label}.focus_symbol")
    if focus != scope["function"]:
        _fail(f"{label}.focus_symbol is not bound to scope.function")
    expected_id = _donor_canonical_id(
        source_kind=source_kind,
        focus_symbol=focus,
        source=source,
        scope=scope,
    )
    if canonical_id != expected_id:
        _fail(f"{label}.canonical_id is not bound to source, scope, and source kind")
    aliases = _donor_id_list(item["aliases"], f"{label}.aliases")
    if canonical_id in aliases:
        _fail(f"{label}.aliases must not repeat canonical_id")
    links = _donor_id_list(item["supersedes"], f"{label}.supersedes")
    duplicate_of = item.get("duplicate_of")
    if duplicate_of is not None:
        duplicate_of = _identifier(duplicate_of, f"{label}.duplicate_of")
    duplicates = _donor_id_list(item["duplicates"], f"{label}.duplicates")
    queried = _donor_id_list(item["queried_by_candidate_ids"], f"{label}.queried_by_candidate_ids")
    used = _donor_id_list(item["used_by_candidate_ids"], f"{label}.used_by_candidate_ids")
    admissibility = _donor_admissibility(
        item["admissibility"], status=status, label=f"{label}.admissibility"
    )
    evidence = _validate_donor_evidence(item["evidence"], f"{label}.evidence")
    notes = item["notes"]
    if notes is not None:
        notes = _text(notes, f"{label}.notes")
    return {
        "canonical_id": canonical_id,
        "aliases": aliases,
        "focus_symbol": focus,
        "source_kind": source_kind,
        "source": source,
        "scope": scope,
        "admissibility": admissibility,
        "status": status,
        "supersedes": links,
        "duplicate_of": duplicate_of,
        "duplicates": duplicates,
        "evidence": evidence,
        "queried_by_candidate_ids": queried,
        "used_by_candidate_ids": used,
        "notes": notes,
        "record_sha256": item["record_sha256"],
    }


def _empty_donor_registry() -> dict[str, Any]:
    return _with_self_hash(
        {
            "schema": DONOR_REGISTRY_SCHEMA,
            "schema_version": SCHEMA_VERSION,
            "records": {},
            "aliases": {},
            "rejections": [],
        },
        "registry_sha256",
    )


_DONOR_REGISTRY_FIELDS = frozenset(
    {"schema", "schema_version", "records", "aliases", "rejections", "registry_sha256"}
)


def _validate_donor_rejection(value: Any, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"rejection_id", "source_kind", "source", "reason", "evidence"},
        required={"rejection_id", "source_kind", "source", "reason", "evidence"},
        label=label,
    )
    rejection_id = _identifier(item["rejection_id"], f"{label}.rejection_id")
    source_kind = _donor_source_kind(item["source_kind"], f"{label}.source_kind")
    if source_kind != "target-derived":
        _fail(f"{label}.source_kind must be target-derived")
    source = _validate_donor_source_descriptor(item["source"], f"{label}.source")
    reason = _text(item["reason"], f"{label}.reason")
    evidence = _validate_donor_evidence(item["evidence"], f"{label}.evidence")
    return {
        "rejection_id": rejection_id,
        "source_kind": source_kind,
        "source": source,
        "reason": reason,
        "evidence": evidence,
    }


def _load_donor_registry(path: Path) -> dict[str, Any]:
    if not path.exists():
        return _empty_donor_registry()
    value = _load_json(path, "donor registry")
    _verify_self_hash(value, "registry_sha256", "donor registry")
    item = _closed(
        value,
        allowed=_DONOR_REGISTRY_FIELDS,
        required=_DONOR_REGISTRY_FIELDS,
        label="donor registry",
    )
    if item["schema"] != DONOR_REGISTRY_SCHEMA or item["schema_version"] != SCHEMA_VERSION:
        _fail("donor registry schema is unsupported")
    records_value = item["records"]
    if not isinstance(records_value, Mapping):
        _fail("donor registry.records must be an object")
    records: dict[str, dict[str, Any]] = {}
    for key, raw in records_value.items():
        canonical_id = _identifier(key, "donor registry record key")
        record = _validate_donor_record(raw, f"donor registry.records[{canonical_id!r}]")
        if record["canonical_id"] != canonical_id:
            _fail("donor registry record key is not its canonical_id")
        records[canonical_id] = record
    for canonical_id, record in records.items():
        duplicate_of = record.get("duplicate_of")
        if duplicate_of is not None and duplicate_of not in records:
            _fail(
                f"donor registry record {canonical_id!r} duplicate_of points to "
                f"unknown record: {duplicate_of}"
            )
    aliases_value = item["aliases"]
    if not isinstance(aliases_value, Mapping):
        _fail("donor registry.aliases must be an object")
    aliases: dict[str, str] = {}
    for alias, canonical in aliases_value.items():
        alias_key = _text(alias, "donor registry alias")
        canonical_value = _identifier(canonical, f"donor registry alias {alias_key!r}")
        if canonical_value not in records:
            _fail(f"donor registry alias {alias_key!r} points to an unknown record")
        if _donor_alias_key(alias_key) != alias_key:
            _fail(f"donor registry alias {alias_key!r} is not casefolded")
        aliases[alias_key] = canonical_value
    expected_aliases: dict[str, str] = {}
    for canonical, record in records.items():
        expected_aliases[_donor_alias_key(canonical)] = canonical
        for alias in record["aliases"]:
            key = _donor_alias_key(alias)
            prior = expected_aliases.get(key)
            if prior is not None and prior != canonical:
                _fail(f"donor registry aliases collide: {alias}")
            expected_aliases[key] = canonical
    if aliases != expected_aliases:
        _fail("donor registry aliases are not bound to record aliases")
    rejection_values = item["rejections"]
    if not isinstance(rejection_values, Sequence) or isinstance(rejection_values, (str, bytes, bytearray)):
        _fail("donor registry.rejections must be a sequence")
    rejections: list[dict[str, Any]] = []
    seen_rejection_ids: set[str] = set()
    for index, raw in enumerate(rejection_values):
        rejection = _validate_donor_rejection(raw, f"donor registry.rejections[{index}]")
        if rejection["rejection_id"] in seen_rejection_ids:
            _fail("donor registry contains duplicate rejection ids")
        seen_rejection_ids.add(rejection["rejection_id"])
        rejections.append(rejection)
    return {
        "schema": DONOR_REGISTRY_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "records": records,
        "aliases": aliases,
        "rejections": rejections,
        "registry_sha256": item["registry_sha256"],
    }


def _write_donor_registry(path: Path, registry: Mapping[str, Any]) -> dict[str, Any]:
    payload = _with_self_hash(
        {
            "schema": DONOR_REGISTRY_SCHEMA,
            "schema_version": SCHEMA_VERSION,
            "records": {
                key: registry["records"][key]
                for key in sorted(registry["records"])
            },
            "aliases": {
                key: registry["aliases"][key]
                for key in sorted(registry["aliases"])
            },
            "rejections": sorted(
                registry["rejections"], key=lambda item: str(item["rejection_id"])
            ),
        },
        "registry_sha256",
    )
    _atomic_replace(path, _canonical(payload))
    return payload


def _donor_registry_lock_path(path: Path) -> Path:
    return path.with_name(f".{path.name}.lock")


def _donor_registry_identity_from_source(
    root: Path,
    source: Path | str,
    focus_symbol: str,
) -> tuple[dict[str, Any], dict[str, Any], str]:
    source_path = _resolve(source, root)
    source_descriptor, source_text = _donor_source_descriptor(source_path)
    focus = _identifier(focus_symbol, "donor focus symbol")
    scope = _donor_function_scope(source_text, focus, f"donor source {source_path}")
    return source_descriptor, scope, focus


def register_donor_shape(
    root: Path,
    registry: Path | str,
    *,
    source: Path | str,
    focus_symbol: str,
    source_kind: str,
    donor_id: str | None = None,
    aliases: Sequence[str] | None = None,
    status: str = "accepted",
    admissibility: Any = "admissible",
    evidence_paths: Sequence[str] | None = None,
    supersedes: Sequence[str] | None = None,
    duplicate_of: str | None = None,
    queried_by_candidate_ids: Sequence[str] | None = None,
    used_by_candidate_ids: Sequence[str] | None = None,
    notes: str | None = None,
) -> dict[str, Any]:
    """Register one authenticated source shape, folding aliases by content."""
    root = root.resolve()
    registry_path = _donor_registry_path(root, registry)
    kind = _donor_source_kind(source_kind)
    if kind == "target-derived":
        _fail("target-derived source is rejected and cannot enter the donor registry")
    normalized_status = _donor_status(status)
    focus = _identifier(focus_symbol, "donor focus symbol")
    source_descriptor, scope, focus = _donor_registry_identity_from_source(root, source, focus)
    evidence = _donor_evidence_descriptors(root, evidence_paths, source_descriptor)
    alias_values = _donor_id_list(aliases, "donor aliases")
    if donor_id is not None:
        alias_values.extend(_donor_id_list(donor_id, "donor id"))
    alias_values = sorted(set(alias_values))
    supersede_values = _donor_id_list(supersedes, "donor supersedes")
    duplicate_of_value = None if duplicate_of is None else _identifier(duplicate_of, "donor duplicate_of")
    queried_values = _donor_id_list(
        queried_by_candidate_ids, "donor queried_by_candidate_ids"
    )
    used_values = _donor_id_list(used_by_candidate_ids, "donor used_by_candidate_ids")
    record = _donor_record_payload(
        donor_id=None,
        aliases=alias_values,
        source_kind=kind,
        status=normalized_status,
        admissibility=admissibility,
        focus_symbol=focus,
        source=source_descriptor,
        scope=scope,
        evidence=evidence,
        supersedes=supersede_values,
        duplicate_of=duplicate_of_value,
        queried_by=queried_values,
        used_by=used_values,
        notes=notes,
    )
    canonical_id = str(record["canonical_id"])
    requested_aliases = sorted({canonical_id, *alias_values})
    with _workbench_lock(_donor_registry_lock_path(registry_path), 10.0):
        current = _load_donor_registry(registry_path)
        for link in supersede_values:
            link_key = _donor_alias_key(link)
            if link_key not in current["aliases"]:
                _fail(f"donor supersedes unknown record or alias: {link}")
        if duplicate_of_value is not None:
            duplicate_key = _donor_alias_key(duplicate_of_value)
            if duplicate_key not in current["aliases"]:
                _fail(f"donor duplicate_of unknown record or alias: {duplicate_of_value}")
            duplicate_target = current["aliases"][duplicate_key]
            if duplicate_target == canonical_id:
                _fail("donor duplicate_of cannot point to itself")
            record["duplicate_of"] = duplicate_target
        collisions: list[tuple[str, str]] = []
        for alias in requested_aliases:
            alias_key = _donor_alias_key(alias)
            existing = current["aliases"].get(alias_key)
            if existing is not None and existing != canonical_id:
                collisions.append((alias, existing))
        if collisions:
            _fail(
                f"donor alias {collisions[0][0]!r} already belongs to "
                f"{collisions[0][1]}; choose a new alias"
            )
        existing_record = current["records"].get(canonical_id)
        if existing_record is not None:
            if (
                existing_record["status"] != normalized_status
                or existing_record["admissibility"] != record["admissibility"]
                or existing_record.get("duplicate_of") != record.get("duplicate_of")
            ):
                _fail(
                    "duplicate donor identity has a conflicting status, admissibility, or duplicate link; "
                    "use donor-reject for a deliberate status transition"
                )
            merged = copy.deepcopy(existing_record)
            new_aliases = sorted(set(alias_values) - set(merged["aliases"]))
            merged["aliases"] = sorted(set(merged["aliases"]) | set(alias_values))
            merged_evidence: dict[str, Mapping[str, Any]] = {
                _donor_path_key(Path(str(item["path"]))): item
                for item in merged["evidence"]
            }
            merged_evidence.update(
                {
                    _donor_path_key(Path(str(item["path"]))): item
                    for item in evidence
                }
            )
            merged["evidence"] = _validate_donor_evidence(
                list(merged_evidence.values()), "merged donor evidence"
            )
            merged["queried_by_candidate_ids"] = sorted(
                set(merged["queried_by_candidate_ids"]) | set(queried_values)
            )
            merged["used_by_candidate_ids"] = sorted(
                set(merged["used_by_candidate_ids"]) | set(used_values)
            )
            merged["duplicates"] = sorted(
                set(merged["duplicates"]) | set(new_aliases)
            )
            merged = _with_self_hash(merged, "record_sha256")
            current["records"][canonical_id] = merged
            current["aliases"][_donor_alias_key(canonical_id)] = canonical_id
            for alias in merged["aliases"]:
                current["aliases"][_donor_alias_key(alias)] = canonical_id
            persisted = _write_donor_registry(registry_path, current)
            return {
                "schema": DONOR_REGISTRY_SCHEMA,
                "schema_version": SCHEMA_VERSION,
                "status": "duplicate",
                "canonical_id": canonical_id,
                "duplicate_of": canonical_id,
                "record": merged,
                "registry_sha256": persisted["registry_sha256"],
            }
        for link in supersede_values:
            resolved = current["aliases"][_donor_alias_key(link)]
            record["supersedes"] = sorted(set(record["supersedes"]) | {resolved})
        record = _with_self_hash(record, "record_sha256")
        current["records"][canonical_id] = record
        current["aliases"][_donor_alias_key(canonical_id)] = canonical_id
        for alias in record["aliases"]:
            current["aliases"][_donor_alias_key(alias)] = canonical_id
        persisted = _write_donor_registry(registry_path, current)
    return {
        "schema": DONOR_REGISTRY_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "status": "registered",
        "canonical_id": canonical_id,
        "duplicate_of": record.get("duplicate_of"),
        "record": record,
        "registry_sha256": persisted["registry_sha256"],
    }


def list_donor_shapes(
    root: Path,
    registry: Path | str,
    *,
    source_kind: str | None = None,
    status: str | None = None,
    focus_symbol: str | None = None,
    include_rejections: bool = False,
) -> dict[str, Any]:
    root = root.resolve()
    registry_path = _donor_registry_path(root, registry)
    with _workbench_lock(_donor_registry_lock_path(registry_path), 10.0):
        current = _load_donor_registry(registry_path)
    normalized_kind = None if source_kind is None else _donor_source_kind(source_kind)
    normalized_status = None if status is None else _donor_status(status)
    normalized_focus = None if focus_symbol is None else _identifier(focus_symbol, "donor focus symbol")
    rows = [
        record
        for record in current["records"].values()
        if (normalized_kind is None or record["source_kind"] == normalized_kind)
        and (normalized_status is None or record["status"] == normalized_status)
        and (normalized_focus is None or record["focus_symbol"] == normalized_focus)
    ]
    rows.sort(key=lambda item: str(item["canonical_id"]))
    result: dict[str, Any] = {
        "schema": DONOR_REGISTRY_LIST_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "records": rows,
        "record_count": len(rows),
        "registry_sha256": current["registry_sha256"],
        "authority_advanced": False,
    }
    if include_rejections:
        result["rejections"] = current["rejections"]
    return result


def lookup_donor_shapes(
    root: Path,
    registry: Path | str,
    *,
    donor_id: str | None = None,
    source_sha256: str | None = None,
    focus_symbol: str | None = None,
    candidate_id: str | None = None,
) -> dict[str, Any]:
    if donor_id is None and source_sha256 is None and focus_symbol is None:
        _fail("donor lookup requires donor-id, source-sha256, or focus-symbol")
    root = root.resolve()
    registry_path = _donor_registry_path(root, registry)
    alias = None if donor_id is None else _text(donor_id, "donor lookup id")
    source_hash = None if source_sha256 is None else _sha256(source_sha256, "donor lookup source-sha256")
    focus = None if focus_symbol is None else _identifier(focus_symbol, "donor lookup focus symbol")
    candidate = None if candidate_id is None else _identifier(candidate_id, "donor lookup candidate-id")
    with _workbench_lock(_donor_registry_lock_path(registry_path), 10.0):
        current = _load_donor_registry(registry_path)
        canonical = None
        if alias is not None:
            canonical = current["aliases"].get(_donor_alias_key(alias))
            if canonical is None:
                _fail(f"donor id or alias not found: {alias}")
        rows = []
        for record in current["records"].values():
            if canonical is not None and record["canonical_id"] != canonical:
                continue
            if source_hash is not None and record["source"]["sha256"] != source_hash:
                continue
            if focus is not None and record["focus_symbol"] != focus:
                continue
            rows.append(record)
        rows.sort(key=lambda item: str(item["canonical_id"]))
        if candidate is not None:
            for record in rows:
                values = set(record["queried_by_candidate_ids"])
                values.add(candidate)
                record["queried_by_candidate_ids"] = sorted(values)
                current["records"][record["canonical_id"]] = _with_self_hash(
                    record, "record_sha256"
                )
            persisted = _write_donor_registry(registry_path, current)
            registry_hash = persisted["registry_sha256"]
        else:
            registry_hash = current["registry_sha256"]
    return {
        "schema": DONOR_REGISTRY_LIST_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "records": rows,
        "record_count": len(rows),
        "registry_sha256": registry_hash,
        "authority_advanced": False,
    }


def _donor_rejection_payload(
    *,
    source: Mapping[str, Any],
    reason: str,
    evidence: Sequence[Mapping[str, Any]],
) -> dict[str, Any]:
    identity = {
        "source_kind": "target-derived",
        "source_sha256": source["sha256"],
        "blob_sha256": source["blob_sha256"],
        "reason": reason,
    }
    return {
        "rejection_id": f"reject-{_sha256_bytes(_canonical(identity))}",
        "source_kind": "target-derived",
        "source": dict(source),
        "reason": reason,
        "evidence": [dict(item) for item in evidence],
    }


def reject_donor_shape(
    root: Path,
    registry: Path | str,
    *,
    donor_id: str | None = None,
    source: Path | str | None = None,
    focus_symbol: str | None = None,
    reason: str,
    evidence_paths: Sequence[str] | None = None,
) -> dict[str, Any]:
    """Record a deliberate rejection or a target-object rejection receipt."""
    if donor_id is None and source is None:
        _fail("donor reject requires donor-id or source")
    rejection_reason = _text(reason, "donor rejection reason")
    root = root.resolve()
    registry_path = _donor_registry_path(root, registry)
    with _workbench_lock(_donor_registry_lock_path(registry_path), 10.0):
        current = _load_donor_registry(registry_path)
        if donor_id is not None:
            alias = _text(donor_id, "donor reject id")
            canonical = current["aliases"].get(_donor_alias_key(alias))
            if canonical is None:
                _fail(f"donor id or alias not found: {alias}")
            record = copy.deepcopy(current["records"][canonical])
            if record["status"] == "accepted":
                record["status"] = "rejected"
                record["admissibility"] = {
                    "decision": "inadmissible",
                    "reason": rejection_reason,
                }
                record["notes"] = rejection_reason
                current["records"][canonical] = _with_self_hash(record, "record_sha256")
                persisted = _write_donor_registry(registry_path, current)
                return {
                    "schema": DONOR_REJECTION_SCHEMA,
                    "schema_version": SCHEMA_VERSION,
                    "status": "rejected",
                    "canonical_id": canonical,
                    "record": current["records"][canonical],
                    "registry_sha256": persisted["registry_sha256"],
                }
            return {
                "schema": DONOR_REJECTION_SCHEMA,
                "schema_version": SCHEMA_VERSION,
                "status": "already-rejected",
                "canonical_id": canonical,
                "record": record,
                "registry_sha256": current["registry_sha256"],
            }
        source_path = _resolve(source, root)
        _assert_no_indirection(source_path)
        role, role_evidence = _artifact_role(source_path)
        is_target = role == _ARTIFACT_ROLE_TARGET or source_path.suffix.casefold() not in _DONOR_SOURCE_SUFFIXES
        if is_target:
            if not source_path.is_file():
                _fail(f"rejected target artifact is not a regular file: {source_path}")
            snapshot = _snapshot(source_path, "rejected target artifact")
            source_descriptor = {
                "path": snapshot["path"],
                "size_bytes": snapshot["size_bytes"],
                "sha256": snapshot["sha256"],
                "blob_sha256": snapshot["sha256"],
            }
            evidence = _donor_evidence_descriptors(root, evidence_paths, source_descriptor)
            rejection = _donor_rejection_payload(
                source=source_descriptor,
                reason=f"{rejection_reason}; {role_evidence}",
                evidence=evidence,
            )
            existing = {item["rejection_id"] for item in current["rejections"]}
            if rejection["rejection_id"] not in existing:
                current["rejections"].append(rejection)
                persisted = _write_donor_registry(registry_path, current)
            else:
                persisted = current
            return {
                "schema": DONOR_REJECTION_SCHEMA,
                "schema_version": SCHEMA_VERSION,
                "status": "target-rejected",
                "rejection": rejection,
                "record_count": len(current["records"]),
                "registry_sha256": persisted["registry_sha256"],
            }
        if focus_symbol is None:
            _fail("donor reject by source requires focus-symbol for a source shape")
        source_descriptor, scope, focus = _donor_registry_identity_from_source(
            root, source_path, focus_symbol
        )
        canonical_candidates = [
            record
            for record in current["records"].values()
            if record["source"]["sha256"] == source_descriptor["sha256"]
            and record["scope"]["function"] == scope["function"]
            and record["scope"]["function_sha256"] == scope["function_sha256"]
        ]
        if canonical_candidates:
            record = copy.deepcopy(sorted(canonical_candidates, key=lambda item: item["canonical_id"])[0])
            canonical = record["canonical_id"]
            if record["status"] == "accepted":
                record["status"] = "rejected"
                record["admissibility"] = {
                    "decision": "inadmissible",
                    "reason": rejection_reason,
                }
                record["notes"] = rejection_reason
                current["records"][canonical] = _with_self_hash(record, "record_sha256")
                persisted = _write_donor_registry(registry_path, current)
            else:
                persisted = current
            return {
                "schema": DONOR_REJECTION_SCHEMA,
                "schema_version": SCHEMA_VERSION,
                "status": "rejected" if record["status"] == "rejected" else "already-rejected",
                "canonical_id": canonical,
                "record": current["records"][canonical],
                "registry_sha256": persisted["registry_sha256"],
            }
        evidence = _donor_evidence_descriptors(root, evidence_paths, source_descriptor)
        rejected = _donor_record_payload(
            donor_id=None,
            aliases=[],
            source_kind="diagnostic-only",
            status="rejected",
            admissibility={"decision": "inadmissible", "reason": rejection_reason},
            focus_symbol=focus,
            source=source_descriptor,
            scope=scope,
            evidence=evidence,
            supersedes=[],
            duplicate_of=None,
            queried_by=[],
            used_by=[],
            notes=rejection_reason,
        )
        canonical = str(rejected["canonical_id"])
        current["records"][canonical] = rejected
        current["aliases"][_donor_alias_key(canonical)] = canonical
        persisted = _write_donor_registry(registry_path, current)
        return {
            "schema": DONOR_REJECTION_SCHEMA,
            "schema_version": SCHEMA_VERSION,
            "status": "rejected",
            "canonical_id": canonical,
            "record": rejected,
            "registry_sha256": persisted["registry_sha256"],
        }


def _assessment_signature(value: Mapping[str, Any] | None) -> tuple[Any, ...] | None:
    if value is None:
        return None
    return (
        value.get("size"),
        value.get("target_size"),
        value.get("candidate_size"),
        value.get("paired_symbol"),
        value.get("match_percent"),
        value.get("diff_rows"),
        tuple(sorted(dict(value.get("diff_kinds", {})).items())),
        value.get("exact"),
        value.get("paired"),
    )


def _assessment_numeric_delta(before: Any, after: Any) -> int | float | None:
    if isinstance(before, bool) or isinstance(after, bool):
        return None
    if not isinstance(before, (int, float)) or not isinstance(after, (int, float)):
        return None
    if not math.isfinite(float(before)) or not math.isfinite(float(after)):
        return None
    result = after - before
    return int(result) if isinstance(result, float) and result.is_integer() else result


def _assessment_diff_kind_delta(
    before: Mapping[str, Any] | None, after: Mapping[str, Any] | None
) -> dict[str, int]:
    before_kinds = before.get("diff_kinds", {}) if isinstance(before, Mapping) else {}
    after_kinds = after.get("diff_kinds", {}) if isinstance(after, Mapping) else {}
    keys = set(before_kinds) | set(after_kinds)
    result = {
        str(key): int(after_kinds.get(key, 0)) - int(before_kinds.get(key, 0))
        for key in keys
        if int(after_kinds.get(key, 0)) - int(before_kinds.get(key, 0))
    }
    return dict(sorted(result.items()))


def _assessment_metric_delta(
    before: Mapping[str, Any] | None, after: Mapping[str, Any] | None
) -> dict[str, Any]:
    before_value = before or {}
    after_value = after or {}
    diff_kind_delta = _assessment_diff_kind_delta(before, after)
    return {
        "size": _assessment_numeric_delta(before_value.get("size"), after_value.get("size")),
        "target_size": _assessment_numeric_delta(
            before_value.get("target_size"), after_value.get("target_size")
        ),
        "candidate_size": _assessment_numeric_delta(
            before_value.get("candidate_size"), after_value.get("candidate_size")
        ),
        "match_percent": _assessment_numeric_delta(
            before_value.get("match_percent"), after_value.get("match_percent")
        ),
        "match": _assessment_numeric_delta(
            before_value.get("match_percent"), after_value.get("match_percent")
        ),
        "diff_rows": _assessment_numeric_delta(
            before_value.get("diff_rows"), after_value.get("diff_rows")
        ),
        "diff_kinds": diff_kind_delta,
        # Keep a singular spelling for consumers that describe a single
        # instruction's kind transition.
        "diff_kind_delta": diff_kind_delta,
        "exact": _assessment_numeric_delta(
            int(bool(before_value.get("exact"))), int(bool(after_value.get("exact")))
        ),
    }


def _assessment_focus_regression(
    symbol: str,
    before: Mapping[str, Any],
    after: Mapping[str, Any],
) -> dict[str, Any] | None:
    """Return a fail-closed regression row when the requested focus worsens."""
    if before.get("exact") and not after.get("exact"):
        reason = "previously_exact_focus_regressed"
    else:
        before_match = _assessment_number(before.get("match_percent"))
        after_match = _assessment_number(after.get("match_percent"))
        if (
            before_match is not None
            and after_match is not None
            and after_match < before_match
        ):
            reason = "focus_match_percent_regressed"
        else:
            before_rows = _assessment_number(before.get("diff_rows"))
            after_rows = _assessment_number(after.get("diff_rows"))
            if (
                (before_match is None or after_match is None or after_match == before_match)
                and before_rows is not None
                and after_rows is not None
                and after_rows > before_rows
            ):
                reason = "focus_diff_rows_regressed"
            else:
                return None
    return {
        "symbol": symbol,
        "before": before,
        "after": after,
        "reason": reason,
    }


def _assessment_file(path: Path | str, root: Path, label: str) -> tuple[Path, dict[str, Any], Any]:
    resolved = _resolve(path, root)
    snapshot = _snapshot(resolved, label)
    value = _load_json(resolved, label)
    _recheck_live_snapshot(resolved, snapshot, label)
    descriptor = {
        "path": snapshot["path"],
        "size_bytes": snapshot["size_bytes"],
        "sha256": snapshot["sha256"],
    }
    return resolved, descriptor, value


def _assessment_report_pair(
    root: Path,
    label: str,
    baseline_path: Path | str,
    candidate_path: Path | str,
    focus_symbol: str | Sequence[str],
) -> dict[str, Any]:
    selected_focuses = _focus_symbols(focus_symbol, "assessment focus_symbols")
    _, baseline_descriptor, baseline_value = _assessment_file(
        baseline_path, root, f"baseline {label} report"
    )
    _, candidate_descriptor, candidate_value = _assessment_file(
        candidate_path, root, f"candidate {label} report"
    )
    baseline_records, baseline_counts = _assessment_records(
        baseline_value, f"baseline {label} report"
    )
    candidate_records, candidate_counts = _assessment_records(
        candidate_value, f"candidate {label} report"
    )
    baseline_by_id = {record["identity"]: record for record in baseline_records}
    candidate_by_id = {record["identity"]: record for record in candidate_records}

    def focus_metric(
        records: list[dict[str, Any]], report_path: str, phase: str, symbol: str
    ) -> Mapping[str, Any]:
        matches = [record for record in records if record["name"] == symbol]
        if not matches:
            _fail(
                f"{report_path} lacks requested focus symbol {symbol!r} in {phase} report"
            )
        if len(matches) != 1:
            _fail(
                f"{report_path} contains duplicate requested focus symbol {symbol!r}"
            )
        metric = matches[0]["metric"]
        if not metric.get("paired"):
            _fail(
                f"{report_path} focus symbol {symbol!r} is not paired in {phase} report"
            )
        return metric

    before_focuses = {
        symbol: focus_metric(
            baseline_records, str(baseline_descriptor["path"]), "baseline", symbol
        )
        for symbol in selected_focuses
    }
    after_focuses = {
        symbol: focus_metric(
            candidate_records, str(candidate_descriptor["path"]), "candidate", symbol
        )
        for symbol in selected_focuses
    }
    changed_siblings: list[dict[str, Any]] = []
    regressions: list[dict[str, Any]] = []
    focus_set = set(selected_focuses)
    for identity in sorted(set(baseline_by_id) | set(candidate_by_id)):
        before_record = baseline_by_id.get(identity)
        after_record = candidate_by_id.get(identity)
        name = (before_record or after_record or {}).get("name")
        if name in focus_set:
            continue
        before = before_record.get("metric") if before_record else None
        after = after_record.get("metric") if after_record else None
        if _assessment_signature(before) == _assessment_signature(after):
            continue
        row: dict[str, Any] = {
            "symbol": name,
            "before": before,
            "after": after,
            "delta": _assessment_metric_delta(before, after),
        }
        occurrence = (before_record or after_record or {}).get("occurrence", 1)
        if occurrence != 1:
            row["occurrence"] = occurrence
        changed_siblings.append(row)
        if before is not None and before.get("exact") and not (after is not None and after.get("exact")):
            regressions.append(
                {
                    "symbol": name,
                    "before": before,
                    "after": after,
                    "reason": "previously_exact_sibling_regressed",
                }
            )
    changed_siblings.sort(key=lambda row: (str(row["symbol"]), int(row.get("occurrence", 1))))
    regressions.sort(key=lambda row: str(row["symbol"]))
    focus_rows = [
        {
            "symbol": symbol,
            "before": before_focuses[symbol],
            "after": after_focuses[symbol],
            "delta": _assessment_metric_delta(
                before_focuses[symbol], after_focuses[symbol]
            ),
        }
        for symbol in selected_focuses
    ]
    result = {
        "baseline": {"report": baseline_descriptor, "function_counts": baseline_counts},
        "candidate": {"report": candidate_descriptor, "function_counts": candidate_counts},
        "exact_function_counts": {"before": baseline_counts, "after": candidate_counts},
        "function_counts": {"before": baseline_counts, "after": candidate_counts},
        "focuses": focus_rows,
        "changed_siblings": changed_siblings,
        "regressions": regressions,
    }
    if len(focus_rows) == 1:
        result["focus"] = focus_rows[0]
    return result


def assess_reports(
    root: Path,
    *,
    baseline_strict: Path | str,
    candidate_strict: Path | str,
    baseline_data: Path | str | None = None,
    candidate_data: Path | str | None = None,
    focus_symbol: str | Sequence[str],
) -> dict[str, Any]:
    """Compare two strict/data report pairs without changing any workbench state."""
    root = root.resolve()
    focuses = _focus_symbols(focus_symbol, "assessment focus_symbols")
    if (baseline_data is None) != (candidate_data is None):
        _fail("baseline and candidate data reports must be supplied together")
    strict = _assessment_report_pair(
        root, "strict", baseline_strict, candidate_strict, focuses
    )
    data = (
        _assessment_report_pair(root, "data", baseline_data, candidate_data, focuses)
        if baseline_data is not None and candidate_data is not None
        else None
    )
    if data is not None:
        strict_focuses = {
            row["symbol"]: row
            for row in strict["focuses"]
        }
        data_focuses = {
            row["symbol"]: row
            for row in data["focuses"]
        }
        for symbol in focuses:
            for phase in ("before", "after"):
                strict_focus = strict_focuses[symbol][phase]
                data_focus = data_focuses[symbol][phase]
                if (
                    not strict_focus
                    or not data_focus
                    or not strict_focus.get("paired")
                    or not data_focus.get("paired")
                    or strict_focus.get("symbol") != symbol
                    or data_focus.get("symbol") != symbol
                    or strict_focus.get("paired_symbol")
                    != data_focus.get("paired_symbol")
                ):
                    _fail(
                        f"strict/data focus pairing mismatch for {symbol!r} in {phase} report"
                    )
    reports = {"strict": strict, "data": data}
    changed_siblings: list[dict[str, Any]] = []
    regressions: list[dict[str, Any]] = []
    function_counts: dict[str, Any] = {}
    for label, report in reports.items():
        if report is None:
            function_counts[label] = None
            continue
        function_counts[label] = report["exact_function_counts"]
        changed_siblings.extend(
            {"report": label, **row} for row in report["changed_siblings"]
        )
        regressions.extend({"report": label, **row} for row in report["regressions"])
        for focus_row in report["focuses"]:
            focus_regression = _assessment_focus_regression(
                focus_row["symbol"],
                focus_row["before"],
                focus_row["after"],
            )
            if focus_regression is not None:
                regressions.append({"report": label, **focus_regression})
    changed_siblings.sort(
        key=lambda row: (str(row["report"]), str(row["symbol"]), int(row.get("occurrence", 1)))
    )
    regressions.sort(key=lambda row: (str(row["report"]), str(row["symbol"])))
    verdict = "rejected" if regressions else "accepted"
    result = {
        "schema": ASSESSMENT_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbols": list(focuses),
        "reports": reports,
        "focuses": [
            {
                "symbol": symbol,
                "strict": next(
                    row for row in strict["focuses"] if row["symbol"] == symbol
                ),
                "data": (
                    next(row for row in data["focuses"] if row["symbol"] == symbol)
                    if data is not None
                    else None
                ),
            }
            for symbol in focuses
        ],
        # Keep report names at the top level as well as under ``reports`` so
        # small shell consumers do not need a special-case traversal.
        "strict": strict,
        "data": data,
        "exact_function_counts": function_counts,
        "changed_siblings": changed_siblings,
        "regressions": regressions,
        "verdict": verdict,
        "status": verdict,
        "authority_advanced": False,
    }
    if len(focuses) == 1:
        result["focus_symbol"] = focuses[0]
        result["focus"] = {
            "symbol": focuses[0],
            "strict": strict["focus"],
            "data": data["focus"] if data is not None else None,
        }
    return result


def prepare_candidate_record(
    root: Path,
    *,
    baseline_strict: Path | str,
    candidate_strict: Path | str,
    baseline_data: Path | str | None = None,
    candidate_data: Path | str | None = None,
    focus_symbol: str | Sequence[str],
    workspace: Path | str,
    candidate_id: str,
    source: Path | str,
    object_path: Path | str,
    compile_attestation: Path | str,
    hypothesis: str,
    axis: str,
    residual: str | None = None,
    status: str = "measured",
    reason: str = "candidate measured",
    heavy_seconds: float | None = None,
) -> dict[str, Any]:
    """Prepare a guarded ``record`` request without mutating the workbench.

    The strict/data assessment is the same gate exposed by ``assess``.  A
    rejected assessment deliberately omits the record request so callers
    cannot accidentally record a candidate after a sibling or focus
    regression.  The final ``record`` command remains the authority that
    writes candidate/CAS state.
    """
    root = root.resolve()
    focuses = _focus_symbols(focus_symbol, "preparation focus_symbols")
    workspace_path = _workspace(workspace, root)
    candidate_value = _identifier(candidate_id, "candidate_id")
    source_path = _resolve(source, root)
    object_file = _resolve(object_path, root)
    _validate_candidate_artifact_path(source_path, "candidate source")
    _validate_candidate_artifact_path(object_file, "candidate object")
    source_snapshot = _snapshot(source_path, "candidate source")
    object_snapshot = _snapshot(object_file, "candidate object")
    session_path = workspace_path / "session.json"
    if session_path.is_file():
        session = _load_session(workspace_path, root)
        _validate_candidate_artifact(
            source_path, source_snapshot, session, "candidate source"
        )
        _validate_candidate_artifact(
            object_file, object_snapshot, session, "candidate object"
        )
        _load_compile_attestation(
            root,
            compile_attestation,
            source_snapshot=source_snapshot,
            object_snapshot=object_snapshot,
            expected_context=session["request"]["context"],
        )
    else:
        _fail("prepare requires an initialized immutable workspace")
    assessment = assess_reports(
        root,
        baseline_strict=baseline_strict,
        candidate_strict=candidate_strict,
        baseline_data=baseline_data,
        candidate_data=candidate_data,
        focus_symbol=focuses,
    )

    strict_path = _resolve(candidate_strict, root)
    data_path = _resolve(candidate_data, root) if candidate_data is not None else None
    hypothesis_value = _text(hypothesis, "hypothesis")
    axis_value = _text(axis, "axis")
    residual_value = _text(residual, "residual") if residual is not None else None
    status_value = _text(status, "status")
    reason_value = _text(reason, "reason")
    heavy_value = _seconds(heavy_seconds, "heavy_seconds")

    ready = assessment["verdict"] == "accepted"
    record_request = None
    if ready:
        record_request = {
            "workspace": os.fspath(workspace_path),
            "candidate_id": candidate_value,
            "source": os.fspath(source_path),
            "object": os.fspath(object_file),
            "compile_attestation": os.fspath(_resolve(compile_attestation, root)),
            "strict_report": os.fspath(strict_path),
            "data_report": os.fspath(data_path) if data_path is not None else None,
            "hypothesis": hypothesis_value,
            "axis": axis_value,
            "residual": residual_value,
            "status": status_value,
            "reason": reason_value,
            "heavy_seconds": heavy_value,
            **(
                {"focus_symbol": focuses[0]}
                if len(focuses) == 1
                else {"focus_symbols": list(focuses)}
            ),
        }

    result = {
        "schema": PREPARATION_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbols": list(focuses),
        "assessment": assessment,
        "artifacts": {
            "source": {
                key: source_snapshot[key]
                for key in ("path", "size_bytes", "sha256")
            },
            "object": {
                key: object_snapshot[key]
                for key in ("path", "size_bytes", "sha256")
            },
        },
        "record_request": record_request,
        "status": "ready" if ready else "rejected",
        "reason": None
        if ready
        else "assessment rejected; record request withheld",
        "authority_advanced": False,
    }
    if len(focuses) == 1:
        result["focus_symbol"] = focuses[0]
    return result


def _report_summary(
    path: Path,
    session: Mapping[str, Any],
    label: str,
    *,
    focus_symbol: str | Sequence[str] | None = None,
) -> dict[str, Any]:
    limit = int(session["request"]["policy"]["max_report_bytes"])
    if path.stat().st_size > limit:
        _fail(f"{label} exceeds max_report_bytes ({path.stat().st_size} > {limit})")
    try:
        value = _load_json(path, label)
    except MatchError as exc:
        # Objdiff wrappers in the wild include bounded human-readable output.
        # Keep strict duplicate-key rejection for JSON-shaped input, but make
        # the compact evidence parser tolerant of plain text diagnostics.
        raw = path.read_bytes()
        if raw.lstrip().startswith((b"{", b"[")) or "duplicate JSON key" in str(exc):
            raise
        text = raw.decode("utf-8", errors="replace")
        percentages = [float(match.group(1)) for match in re.finditer(r"(?<![0-9])([0-9]{1,3}(?:\.[0-9]+)?)\s*%", text)]
        diff_kinds: dict[str, int] = {}
        for kind in re.findall(r"\b(REG_[A-Z0-9_]+|STACK_[A-Z0-9_]+|BRANCH_[A-Z0-9_]+|NOP)\b", text):
            diff_kinds[kind] = diff_kinds.get(kind, 0) + 1
        result = {
            "diagnostic_only": True,
            "summary_kind": "objdiff_compact",
            "format": "text",
            "status": "exact" if re.search(r"\b(exact|matched|match)\b", text, re.I) else "mismatch",
            "percent": max(percentages) if percentages else None,
            "diff_kinds": dict(sorted(diff_kinds.items())),
            "text_bytes": len(raw),
            "function_count": 0,
            "exact_function_count": 0,
            "focuses": [None for _ in _focus_symbols(
                focus_symbol,
                f"{label} focus_symbols",
                default=str(session["request"]["function"]),
            )],
        }
        if len(result["focuses"]) == 1:
            result["focus"] = None
        if len(_canonical(result)) > int(session["request"]["policy"]["max_compact_bytes"]):
            _fail(f"{label} compact summary exceeds max_compact_bytes")
        return result
    if isinstance(value, list):
        value = {"functions": value}
    elif not isinstance(value, dict):
        _fail(f"{label} root must be an object")
    left = value.get("left", {})
    right = value.get("right", {})
    left_symbols = left.get("symbols", []) if isinstance(left, Mapping) else []
    right_symbols = right.get("symbols", []) if isinstance(right, Mapping) else []
    focuses = _focus_symbols(
        focus_symbol,
        f"{label} focus_symbols",
        default=str(session["request"]["function"]),
    )
    total = 0
    exact = 0
    focus_rows: dict[str, dict[str, Any] | None] = {symbol: None for symbol in focuses}
    diff_kinds: dict[str, int] = {}
    if isinstance(left_symbols, list):
        for symbol in left_symbols:
            if not isinstance(symbol, Mapping) or symbol.get("kind") != "SYMBOL_FUNCTION":
                continue
            total += 1
            rows = symbol.get("instructions", [])
            changed = [row for row in rows if isinstance(row, Mapping) and row.get("diff_kind")] if isinstance(rows, list) else []
            paired = symbol.get("target_symbol")
            is_exact = paired is not None and symbol.get("match_percent") == 100.0 and not changed
            exact += int(is_exact)
            for row in changed:
                kind = str(row.get("diff_kind"))
                diff_kinds[kind] = diff_kinds.get(kind, 0) + 1
            name = symbol.get("name")
            if isinstance(name, str) and name in focus_rows:
                other_size = None
                if isinstance(paired, int) and isinstance(right_symbols, list) and 0 <= paired < len(right_symbols):
                    other = right_symbols[paired]
                    if isinstance(other, Mapping):
                        other_size = other.get("size")
                focus_rows[name] = {
                    "name": name,
                    "match_percent": symbol.get("match_percent"),
                    "target_size": symbol.get("size"),
                    "candidate_size": other_size,
                    "diff_rows": len(changed),
                    "exact": is_exact,
                }
    result = {
        "diagnostic_only": True,
        "summary_kind": "objdiff_compact",
        "function_count": total,
        "exact_function_count": exact,
        "focuses": [focus_rows[symbol] for symbol in focuses],
        "diff_kinds": dict(sorted(diff_kinds.items())),
    }
    if len(focuses) == 1:
        result["focus"] = focus_rows[focuses[0]]
    # Preserve high-level fields when a schema variant does not expose the
    # canonical left/right symbol arrays.
    for key in ("status", "matched", "match_percent", "percent"):
        if key in value and isinstance(value[key], (str, int, float, bool)):
            result[key] = value[key]
    if len(_canonical(result)) > int(session["request"]["policy"]["max_compact_bytes"]):
        _fail(f"{label} compact summary exceeds max_compact_bytes")
    return result


def _store_report(
    workspace: Path,
    path: Path,
    session: Mapping[str, Any],
    label: str,
    *,
    focus_symbol: str | Sequence[str] | None = None,
) -> dict[str, Any]:
    source = _snapshot(path, label)
    sha = source["sha256"]
    output = workspace / "cas" / "reports" / sha[:2] / f"{sha}.json.gz"
    _safe_mkdir(output.parent)
    dedup_hit = output.is_file()
    if not dedup_hit:
        with tempfile.NamedTemporaryFile(
            "wb", dir=output.parent, prefix=f".{sha}.", suffix=".tmp", delete=False
        ) as raw:
            temporary = Path(raw.name)
            with gzip.GzipFile(filename="", mode="wb", fileobj=raw, compresslevel=9, mtime=0) as compressed:
                with path.open("rb") as stream:
                    for block in iter(lambda: stream.read(1024 * 1024), b""):
                        compressed.write(block)
            raw.flush()
            os.fsync(raw.fileno())
        os.replace(temporary, output)
    compressed = _snapshot(output, f"cached {label}")
    digest = hashlib.sha256()
    size = 0
    try:
        with gzip.open(output, "rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
                size += len(block)
    except (OSError, EOFError) as exc:
        raise MatchError(f"invalid cached gzip report: {output}: {exc}") from exc
    if digest.hexdigest() != sha or size != source["size_bytes"]:
        _fail(f"content-addressed report collision: {output}")
    after = _snapshot(path, label)
    if after["sha256"] != sha or after["size_bytes"] != source["size_bytes"]:
        _fail(f"{label} changed while it was compressed")
    return {
        "raw_sha256": sha,
        "raw_size_bytes": size,
        "codec": "gzip",
        "compressed_sha256": compressed["sha256"],
        "compressed_size_bytes": compressed["size_bytes"],
        "cas_path": output.relative_to(workspace).as_posix(),
        "dedup_hit": dedup_hit,
        "compact": _report_summary(path, session, label, focus_symbol=focus_symbol),
    }


def _compact_focus_rows(compact: Mapping[str, Any], label: str) -> list[Mapping[str, Any] | None]:
    """Read the plural compact focus field, falling back to legacy ``focus``."""
    focuses = compact.get("focuses")
    if focuses is None:
        focus = compact.get("focus")
        return [focus] if isinstance(focus, Mapping) else []
    if not isinstance(focuses, list):
        _fail(f"{label}.focuses must be an array")
    rows: list[Mapping[str, Any] | None] = []
    for index, row in enumerate(focuses):
        if row is not None and not isinstance(row, Mapping):
            _fail(f"{label}.focuses[{index}] must be an object or null")
        rows.append(row)
    return rows


def _candidate_path(workspace: Path, candidate_id: str) -> Path:
    return workspace / "candidates" / f"{candidate_id}.json"


def _contained(workspace: Path, relative: Any, label: str) -> Path:
    value = Path(_text(relative, label))
    if value.is_absolute() or ".." in value.parts:
        _fail(f"{label} must be a contained relative path")
    path = _resolve(value, workspace)
    try:
        path.relative_to(workspace.resolve())
    except ValueError:
        _fail(f"{label} escapes the workspace")
    return path


def _verify_candidate_cas(workspace: Path, value: Mapping[str, Any]) -> None:
    for key in ("source_blob", "object_blob"):
        blob = value.get(key)
        if not isinstance(blob, Mapping):
            _fail(f"candidate record lacks {key}")
        _closed(
            blob,
            allowed={"kind", "sha256", "size_bytes", "cas_path", "dedup_hit"},
            required={"kind", "sha256", "size_bytes", "cas_path", "dedup_hit"},
            label=f"candidate {key}",
        )
        expected_kind = key.removesuffix("_blob")
        if blob.get("kind") != expected_kind:
            _fail(f"candidate {key} kind is invalid")
        _sha256(blob.get("sha256"), f"candidate {key}.sha256")
        _integer(blob.get("size_bytes"), f"candidate {key}.size_bytes")
        if not isinstance(blob.get("dedup_hit"), bool):
            _fail(f"candidate {key}.dedup_hit must be boolean")
        try:
            path = _contained(workspace, blob.get("cas_path"), f"candidate {key}.cas_path")
            current = _snapshot(path, f"candidate {key} CAS")
        except MatchError as exc:
            raise MatchError(f"candidate {key} CAS is unavailable: {exc}") from exc
        if current["size_bytes"] != blob.get("size_bytes") or current["sha256"] != blob.get("sha256"):
            _fail(f"candidate {key} CAS mismatch")
    reports = value.get("reports")
    if not isinstance(reports, Mapping):
        _fail("candidate record reports must be an object")
    for name, report in reports.items():
        if report is None:
            continue
        if not isinstance(report, Mapping):
            _fail(f"candidate report {name} must be an object")
        _closed(
            report,
            allowed={
                "raw_sha256", "raw_size_bytes", "codec", "compressed_sha256",
                "compressed_size_bytes", "cas_path", "dedup_hit", "compact",
            },
            required={
                "raw_sha256", "raw_size_bytes", "codec", "compressed_sha256",
                "compressed_size_bytes", "cas_path", "dedup_hit", "compact",
            },
            label=f"candidate report {name}",
        )
        if report.get("codec") != "gzip" or not isinstance(report.get("dedup_hit"), bool):
            _fail(f"candidate report {name} codec/dedup metadata is invalid")
        _sha256(report.get("raw_sha256"), f"candidate report {name}.raw_sha256")
        _sha256(report.get("compressed_sha256"), f"candidate report {name}.compressed_sha256")
        _integer(report.get("raw_size_bytes"), f"candidate report {name}.raw_size_bytes")
        _integer(report.get("compressed_size_bytes"), f"candidate report {name}.compressed_size_bytes")
        path = _contained(workspace, report.get("cas_path"), f"candidate report {name}.cas_path")
        current = _snapshot(path, f"candidate report {name} CAS")
        if (
            current["size_bytes"] != report.get("compressed_size_bytes")
            or current["sha256"] != report.get("compressed_sha256")
        ):
            _fail(f"candidate report {name} compressed CAS mismatch")
        digest = hashlib.sha256()
        size = 0
        try:
            with gzip.open(path, "rb") as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b""):
                    digest.update(block)
                    size += len(block)
        except (OSError, EOFError) as exc:
            raise MatchError(f"candidate report {name} CAS is invalid gzip: {exc}") from exc
        if digest.hexdigest() != report.get("raw_sha256") or size != report.get("raw_size_bytes"):
            _fail(f"candidate report {name} raw CAS mismatch")


def _load_candidate(
    workspace: Path, candidate_id: str, session: Mapping[str, Any]
) -> Mapping[str, Any]:
    value = _load_json(_candidate_path(workspace, candidate_id), "candidate record")
    _verify_self_hash(value, "record_sha256", "candidate record")
    _closed(
        value,
        allowed={
            "schema", "schema_version", "session_sha256", "candidate_id", "ordinal",
            "source", "object", "compile_input_identity", "source_context_key", "object_result_key",
            "compile_attestation", "migration",
            "source_blob", "object_blob", "reports", "hypothesis", "outcome",
            "report_binding", "telemetry", "duplicate_of", "previous_record_sha256", "authority_advanced",
            "focus_symbol", "focus_symbols", "record_sha256",
        },
        required={
            "schema", "schema_version", "session_sha256", "candidate_id", "ordinal",
            "source", "object", "source_context_key", "object_result_key",
            "source_blob", "object_blob", "reports", "hypothesis", "outcome",
            "report_binding", "telemetry", "duplicate_of", "previous_record_sha256", "authority_advanced",
            "record_sha256",
        },
        label="candidate record",
    )
    if value.get("schema") != CANDIDATE_SCHEMA or value.get("schema_version") != 1 or value.get("candidate_id") != candidate_id:
        _fail("candidate record identity mismatch")
    if value.get("session_sha256") != session.get("session_sha256"):
        _fail("candidate record belongs to a different session")
    _integer(value.get("ordinal"), "candidate ordinal", minimum=1)
    if value.get("authority_advanced") is not False:
        _fail("candidate record must not advance authority")
    if value.get("report_binding") != "caller_supplied_diagnostic_unverified":
        _fail("candidate report binding classification is invalid")
    _stored_focus_symbols(
        value,
        default=str(session["request"]["function"]),
    )
    if not isinstance(value.get("source"), Mapping) or not isinstance(value.get("object"), Mapping):
        _fail("candidate source/object descriptors are missing")
    _sha256(value["source"].get("sha256"), "candidate source.sha256")
    _sha256(value["object"].get("sha256"), "candidate object.sha256")
    for label in ("source", "object"):
        descriptor_value = value[label]
        _closed(descriptor_value, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label=f"candidate {label}")
        _text(descriptor_value["path"], f"candidate {label}.path")
        _integer(descriptor_value["size_bytes"], f"candidate {label}.size_bytes")
    compile_input_value = value.get("compile_input_identity")
    if compile_input_value is None:
        expected_source_key = _legacy_context_key(session, value["source"]["sha256"])
    else:
        compile_input = _validate_compile_input_identity(compile_input_value, value["source"])
        expected_source_key = _context_key(
            session, value["source"]["sha256"], compile_input
        )
    if value.get("source_context_key") != expected_source_key:
        _fail("candidate source context key mismatch")
    if value.get("object_result_key") != _object_key(session, value["object"]["sha256"]):
        _fail("candidate object result key mismatch")
    attestation = value.get("compile_attestation")
    if attestation is not None:
        _validate_compile_attestation(
            attestation,
            source_snapshot=value["source"],
            object_snapshot=value["object"],
            expected_context=session["request"]["context"],
            label="candidate compile attestation",
        )
    migration = value.get("migration")
    if migration is not None:
        migration_value = _closed(
            migration,
            allowed={
                "source_session_sha256", "source_candidate_id",
                "source_record_sha256", "source_ordinal", "source_duplicate_of",
            },
            required={
                "source_session_sha256", "source_candidate_id",
                "source_record_sha256", "source_ordinal", "source_duplicate_of",
            },
            label="candidate migration",
        )
        _sha256(migration_value.get("source_session_sha256"), "candidate migration.source_session_sha256")
        _identifier(migration_value.get("source_candidate_id"), "candidate migration.source_candidate_id")
        _sha256(migration_value.get("source_record_sha256"), "candidate migration.source_record_sha256")
        _integer(migration_value.get("source_ordinal"), "candidate migration.source_ordinal", minimum=1)
        if migration_value.get("source_duplicate_of") is not None:
            _identifier(migration_value.get("source_duplicate_of"), "candidate migration.source_duplicate_of")
    reports = value.get("reports")
    if not isinstance(reports, Mapping) or set(reports) != {"strict", "data"}:
        _fail("candidate reports must contain only strict and data")
    for label in ("strict", "data"):
        report = reports[label]
        if report is None:
            if label == "strict":
                _fail("candidate strict report is missing")
            continue
        if not isinstance(report, Mapping):
            _fail(f"candidate report {label} must be an object")
        for key in ("raw_sha256", "compressed_sha256", "cas_path"):
            if key not in report:
                _fail(f"candidate report {label}.{key} is missing")
        _sha256(report["raw_sha256"], f"candidate report {label}.raw_sha256")
        _sha256(report["compressed_sha256"], f"candidate report {label}.compressed_sha256")
        _text(report["cas_path"], f"candidate report {label}.cas_path")
        compact = report.get("compact")
        if not isinstance(compact, Mapping):
            _fail(f"candidate report {label}.compact is missing")
        focus = compact.get("focus")
        if focus is not None and not isinstance(focus, Mapping):
            _fail(f"candidate report {label}.compact.focus must be an object or null")
        if isinstance(focus, Mapping):
            if "exact" in focus and not isinstance(focus.get("exact"), bool):
                _fail(f"candidate report {label}.compact.focus.exact must be boolean")
        focuses = compact.get("focuses")
        if focuses is not None:
            if not isinstance(focuses, list):
                _fail(f"candidate report {label}.compact.focuses must be an array")
            for index, row in enumerate(focuses):
                if row is not None and not isinstance(row, Mapping):
                    _fail(
                        f"candidate report {label}.compact.focuses[{index}] "
                        "must be an object or null"
                    )
                if isinstance(row, Mapping) and "exact" in row and not isinstance(row.get("exact"), bool):
                    _fail(
                        f"candidate report {label}.compact.focuses[{index}].exact "
                        "must be boolean"
                    )
    for label in ("hypothesis", "outcome", "telemetry"):
        if not isinstance(value.get(label), Mapping):
            _fail(f"candidate {label} is missing")
    hypothesis = _closed(
        value["hypothesis"],
        allowed={"name", "axis", "axis_fingerprint", "residual", "residual_fingerprint"},
        required={"name", "axis", "axis_fingerprint", "residual", "residual_fingerprint"},
        label="candidate hypothesis",
    )
    _text(hypothesis.get("name"), "candidate hypothesis.name")
    axis = _text(hypothesis.get("axis"), "candidate hypothesis.axis")
    if hypothesis.get("axis_fingerprint") != _sha256_bytes(_canonical({"axis": axis})):
        _fail("candidate hypothesis axis fingerprint mismatch")
    residual = hypothesis.get("residual")
    if residual is None:
        if hypothesis.get("residual_fingerprint") is not None:
            _fail("candidate residual fingerprint must be null without a residual")
    else:
        residual_text = _text(residual, "candidate hypothesis.residual")
        if hypothesis.get("residual_fingerprint") != _sha256_bytes(_canonical({"residual": residual_text})):
            _fail("candidate residual fingerprint mismatch")
    outcome = _closed(
        value["outcome"],
        allowed={"status", "reason"},
        required={"status", "reason"},
        label="candidate outcome",
    )
    telemetry = _closed(
        value["telemetry"],
        allowed={"heavy_seconds"},
        required={"heavy_seconds"},
        label="candidate telemetry",
    )
    _text(outcome.get("status"), "candidate outcome.status")
    _text(outcome.get("reason"), "candidate outcome.reason")
    _seconds(telemetry.get("heavy_seconds"), "candidate telemetry.heavy_seconds")
    duplicate_of = value.get("duplicate_of")
    if duplicate_of is not None:
        duplicate_id = _identifier(duplicate_of, "candidate duplicate_of")
        if duplicate_id == candidate_id:
            _fail("candidate cannot duplicate itself")
    previous = value.get("previous_record_sha256")
    if previous is not None:
        _sha256(previous, "candidate previous_record_sha256")
    for label, descriptor_value in (("source", value["source"]), ("object", value["object"])):
        blob = value.get(f"{label}_blob")
        if not isinstance(blob, Mapping):
            _fail(f"candidate {label} CAS descriptor is missing")
        if blob.get("sha256") != descriptor_value.get("sha256") or blob.get("size_bytes") != descriptor_value.get("size_bytes"):
            _fail(f"candidate {label} CAS is not bound to its descriptor")
    _verify_candidate_cas(workspace, value)
    return value


def record_candidate(
    root: Path,
    workspace: Path | str,
    *,
    candidate_id: str,
    source: Path | str,
    object_path: Path | str,
    compile_attestation: Path | str,
    strict_report: Path | str,
    data_report: Path | str | None,
    hypothesis: str,
    axis: str,
    residual: str | None = None,
    status: str = "measured",
    reason: str = "candidate measured",
    heavy_seconds: float | None = None,
    focus_symbol: str | Sequence[str] | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    destination = _workspace(workspace, root)
    candidate_id = _identifier(candidate_id, "candidate_id")
    residual_value = _text(residual, "residual") if residual else None
    residual_digest = (
        _sha256_bytes(_canonical({"residual": residual_value}))
        if residual_value else None
    )
    source_path = _resolve(source, root)
    object_file = _resolve(object_path, root)
    strict_path = _resolve(strict_report, root)
    data_path = _resolve(data_report, root) if data_report is not None else None
    source_snapshot = _snapshot(source_path, "candidate source")
    compile_input_identity = _compile_input_identity(source_snapshot)
    object_snapshot = _snapshot(object_file, "candidate object")
    _snapshot(strict_path, "strict report")
    if data_path is not None:
        _snapshot(data_path, "data report")
    lock_path = destination / ".workbench.lock"
    with _workbench_lock(lock_path, 8.0):
        session = _load_session(destination, root)
        _validate_candidate_artifact(
            source_path, source_snapshot, session, "candidate source"
        )
        _validate_candidate_artifact(
            object_file, object_snapshot, session, "candidate object"
        )
        attestation = _load_compile_attestation(
            root,
            compile_attestation,
            source_snapshot=source_snapshot,
            object_snapshot=object_snapshot,
            expected_context=session["request"]["context"],
        )
        selected_focuses = _focus_symbols(
            focus_symbol,
            "focus_symbols",
            default=str(session["request"]["function"]),
        )
        index = _load_index(destination, session)
        source_key = _context_key(
            session, source_snapshot["sha256"], compile_input_identity
        )
        object_key = _object_key(session, object_snapshot["sha256"])
        existing_path = _candidate_path(destination, candidate_id)
        if candidate_id in index["candidates"] and not existing_path.is_file():
            _fail("immutable candidate index entry has no candidate record")
        if existing_path.is_file():
            existing = _load_candidate(destination, candidate_id, session)
            if candidate_id not in index["candidates"]:
                if (
                    existing.get("ordinal") != int(index["sequence"]) + 1
                    or existing.get("previous_record_sha256") != index.get("last_record_sha256")
                ):
                    _fail("unindexed candidate record is not a recoverable final append")
                for key, mapped_candidate in (
                    (existing["source_context_key"], index["source_context_index"].get(existing["source_context_key"])),
                    (existing["object_result_key"], index["object_index"].get(existing["object_result_key"])),
                ):
                    if mapped_candidate is not None and mapped_candidate != candidate_id:
                        _fail(f"unindexed candidate record collides with {key}")
                index["sequence"] = existing["ordinal"]
                index["candidates"][candidate_id] = existing_path.relative_to(destination).as_posix()
                index["source_context_index"].setdefault(existing["source_context_key"], candidate_id)
                index["object_index"].setdefault(existing["object_result_key"], candidate_id)
                index["last_record_sha256"] = existing["record_sha256"]
                _atomic_replace(destination / "index.json", _canonical(_with_self_hash(index, "index_sha256")))
            same = (
                existing["source"]["sha256"] == source_snapshot["sha256"]
                and _normalized_compile_input_path(existing["source"]["path"])
                == compile_input_identity["normalized_path"]
                and existing["object"]["sha256"] == object_snapshot["sha256"]
                and existing.get("compile_attestation", {}).get("attestation_sha256")
                == attestation["attestation_sha256"]
                and existing["hypothesis"]["name"] == _text(hypothesis, "hypothesis")
                and existing["hypothesis"]["axis"] == _text(axis, "axis")
                and existing["hypothesis"].get("residual_fingerprint")
                == residual_digest
                and _stored_focus_symbols(
                    existing,
                    default=str(session["request"]["function"]),
                )
                == selected_focuses
                and existing["reports"]["strict"]["raw_sha256"]
                == _snapshot(strict_path, "strict report")["sha256"]
                and (
                    (existing["reports"].get("data") is None and data_path is None)
                    or (
                        data_path is not None
                        and isinstance(existing["reports"].get("data"), Mapping)
                        and existing["reports"]["data"]["raw_sha256"]
                        == _snapshot(data_path, "data report")["sha256"]
                    )
                )
                and existing["outcome"]["status"] == _text(status, "status")
                and existing["outcome"]["reason"] == _text(reason, "reason")
                and existing["telemetry"].get("heavy_seconds")
                == _seconds(heavy_seconds, "heavy_seconds")
            )
            if same:
                _recheck_live_snapshot(source_path, source_snapshot, "candidate source")
                _recheck_live_snapshot(object_file, object_snapshot, "candidate object")
                return {"status": "unchanged", "record": existing}
            _fail(f"candidate_id already records different evidence: {candidate_id}")
        source_blob = _copy_blob(destination, source_path, "source", source_snapshot)
        object_blob = _copy_blob(destination, object_file, "object", object_snapshot)
        strict = _store_report(
            destination,
            strict_path,
            session,
            "strict report",
            focus_symbol=selected_focuses,
        )
        data = (
            _store_report(
                destination,
                data_path,
                session,
                "data report",
                focus_symbol=selected_focuses,
            )
            if data_path
            else None
        )
        object_duplicate = index["object_index"].get(object_key)
        _, source_duplicate, _ = _source_index_match(
            destination, index, session, source_snapshot, compile_input_identity
        )
        if source_duplicate:
            source_record = _load_candidate(destination, source_duplicate, session)
            if source_record.get("object_result_key") != object_key:
                _fail(
                    "the same frozen source/context produced a different object; "
                    "recording it would hide a nondeterministic or incomplete cache key"
                )
        # Prefer the same source/context producer when both indexes legitimately
        # name different earlier records for one object.  The object index is
        # intentionally canonical-first, while the source index identifies the
        # evidence that is actually eligible for same-context reuse.
        duplicate_id = source_duplicate or object_duplicate
        ordinal = int(index["sequence"]) + 1
        record = _with_self_hash(
            {
                "schema": CANDIDATE_SCHEMA,
                "schema_version": 1,
                "session_sha256": session["session_sha256"],
                "candidate_id": candidate_id,
                "ordinal": ordinal,
                "source": {key: source_snapshot[key] for key in ("path", "size_bytes", "sha256")},
                "object": {key: object_snapshot[key] for key in ("path", "size_bytes", "sha256")},
                "compile_input_identity": compile_input_identity,
                "compile_attestation": attestation,
                "source_context_key": source_key,
                "object_result_key": object_key,
                "source_blob": source_blob,
                "object_blob": object_blob,
                "reports": {"strict": strict, "data": data},
                **(
                    (
                        {"focus_symbol": selected_focuses[0]}
                        if len(selected_focuses) == 1
                        else {"focus_symbols": list(selected_focuses)}
                    )
                    if focus_symbol is not None
                    else {}
                ),
                "report_binding": "caller_supplied_diagnostic_unverified",
                "hypothesis": {
                    "name": _text(hypothesis, "hypothesis"),
                    "axis": _text(axis, "axis"),
                    "axis_fingerprint": _sha256_bytes(_canonical({"axis": _text(axis, "axis")})),
                    "residual": residual_value,
                    "residual_fingerprint": residual_digest,
                },
                "outcome": {"status": _text(status, "status"), "reason": _text(reason, "reason")},
                "telemetry": {"heavy_seconds": _seconds(heavy_seconds, "heavy_seconds")},
                "duplicate_of": duplicate_id,
                "previous_record_sha256": index.get("last_record_sha256"),
                "authority_advanced": False,
            },
            "record_sha256",
        )
        _write_new(existing_path, _canonical(record))
        index["sequence"] = ordinal
        index["candidates"][candidate_id] = existing_path.relative_to(destination).as_posix()
        index["source_context_index"].setdefault(source_key, candidate_id)
        index["object_index"].setdefault(object_key, candidate_id)
        index["last_record_sha256"] = record["record_sha256"]
        _atomic_replace(destination / "index.json", _canonical(_with_self_hash(index, "index_sha256")))
    return {"status": "duplicate" if duplicate_id else "recorded", "record": record}


def _validate_argv_dependencies(argv: Sequence[str], *, root: Path, base: Path, executable: Mapping[str, Any], inputs: Sequence[Mapping[str, Any]]) -> None:
    """Require path-like argv values to exist and be authenticated descriptors."""
    declared = {
        os.path.normcase(os.path.abspath(str(executable["path"]))),
        *(os.path.normcase(os.path.abspath(str(item["path"]))) for item in inputs),
    }
    placeholders = {"{workspace}", "{output_root}", "{candidate_object}", "{target_object}"}
    separate_path_flags = {
        "-I", "-L", "-include", "-isystem", "--config", "--config-file",
        "--input", "--script", "--response-file",
    }
    attached_path_prefixes = (
        "--response-file", "--config-file", "-isystem", "-include",
        "--config", "--input", "--script",
    )
    expect_path_value = False
    for raw in argv:
        if not raw:
            continue
        forced_path = expect_path_value
        expect_path_value = False
        if raw in placeholders:
            continue
        if raw in separate_path_flags:
            expect_path_value = True
            continue
        if forced_path:
            token = raw
        elif "=" in raw:
            token = raw.split("=", 1)[1]
            forced_path = raw.split("=", 1)[0] in separate_path_flags
        elif raw.startswith(("-I", "-L")) and len(raw) > 2:
            token = raw[2:]
            forced_path = True
        elif raw.startswith("@") and len(raw) > 1:
            token = raw[1:]
            forced_path = True
        else:
            prefix = next(
                (item for item in attached_path_prefixes if raw.startswith(item) and len(raw) > len(item)),
                None,
            )
            if prefix is None:
                token = raw
            else:
                token = raw[len(prefix):].lstrip("=")
                forced_path = True
        if token in placeholders:
            continue
        if token.startswith("{") or any(placeholder in token for placeholder in placeholders):
            _fail("diagnostic argv placeholders must occupy the entire dependency argument")
        pathish = forced_path or os.path.isabs(token) or "/" in token or "\\" in token or Path(token).suffix.lower() in {
            ".c", ".cc", ".cpp", ".h", ".o", ".a", ".json", ".py", ".dol", ".elf", ".bin", ".txt"
        }
        if not pathish:
            continue
        candidate = _resolve(token, base)
        if not os.path.lexists(candidate):
            _fail(f"path-like diagnostic argv dependency does not exist: {token}")
        normalized = os.path.normcase(os.path.abspath(os.fspath(candidate)))
        if normalized not in declared:
            _fail(f"path-like diagnostic argv dependency is undeclared: {token}")
    if expect_path_value:
        _fail("path-bearing diagnostic argv flag lacks its dependency value")


def _job_value(value: Any, *, root: Path, workspace: Path, session: Mapping[str, Any]) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"job_id", "kind", "resource_class", "executable", "argv", "cwd", "inputs", "outputs", "timeout_seconds", "max_output_bytes", "env"},
        required={"job_id", "kind", "resource_class", "executable", "argv", "cwd", "inputs", "outputs", "timeout_seconds"},
        label="diagnostic job",
    )
    resource = _text(item["resource_class"], "diagnostic job.resource_class")
    if resource in SERIAL_RESOURCE_CLASSES:
        _fail(f"serial resource class is forbidden in parallel diagnostics: {resource}")
    if resource not in SAFE_RESOURCE_CLASSES:
        _fail(f"unknown diagnostic resource class: {resource}")
    kind = _identifier(item["kind"], "diagnostic job.kind")
    if kind not in session["request"]["policy"]["allowed_job_kinds"]:
        _fail(f"diagnostic job kind is not registered by the frozen session: {kind}")
    argv = item["argv"]
    if not isinstance(argv, list) or not all(isinstance(arg, str) and "\x00" not in arg for arg in argv):
        _fail("diagnostic job.argv must be a string array")
    inputs = item["inputs"]
    if not isinstance(inputs, list):
        _fail("diagnostic job.inputs must be an array")
    input_descriptors = [
        _descriptor(value, base=root, label=f"diagnostic job.inputs[{index}]")
        for index, value in enumerate(inputs)
    ]
    executable_descriptor = _descriptor(item["executable"], base=root, label="diagnostic job.executable")
    outputs = item["outputs"]
    if (
        not isinstance(outputs, list)
        or not all(isinstance(output, str) for output in outputs)
        or len(set(outputs)) != len(outputs)
    ):
        _fail("diagnostic job.outputs must be a unique string array")
    output_values = []
    for output in outputs:
        text = _text(output, "diagnostic job output")
        relative = Path(text)
        if relative.is_absolute() or ".." in relative.parts:
            _fail(f"diagnostic output must be a relative contained path: {text}")
        output_values.append(relative.as_posix())
    cwd = _resolve(_text(item["cwd"], "diagnostic job.cwd"), root)
    _assert_no_indirection(cwd)
    if not cwd.is_dir():
        _fail(f"diagnostic job.cwd is not a directory: {cwd}")
    try:
        cwd.relative_to(root.resolve())
    except ValueError:
        _fail("diagnostic job.cwd must stay beneath the repository root")
    _validate_argv_dependencies(argv, root=root, base=cwd, executable=executable_descriptor, inputs=input_descriptors)
    raw_env = item.get("env")
    if raw_env is None:
        inherited = os.environ
        env = {
            key: inherited[key]
            for key in ("PATH", "SystemRoot", "TEMP", "TMP", "PYTHONHOME", "PYTHONPATH")
            if key in inherited
        }
    else:
        if not isinstance(raw_env, Mapping):
            _fail("diagnostic job.env must be an object")
        env = {}
        for key, value in raw_env.items():
            if not isinstance(key, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key):
                _fail("diagnostic job.env keys must be shell-safe names")
            env[key] = _text(value, f"diagnostic job.env[{key}]")
    return {
        "job_id": _identifier(item["job_id"], "diagnostic job.job_id"),
        "kind": kind,
        "resource_class": resource,
        "executable": executable_descriptor,
        "argv": list(argv),
        "cwd": os.fspath(cwd),
        "inputs": input_descriptors,
        "outputs": output_values,
        "timeout_seconds": _integer(item["timeout_seconds"], "diagnostic job.timeout_seconds", minimum=1, maximum=3600),
        "max_output_bytes": _integer(
            item.get("max_output_bytes", 1024 * 1024),
            "diagnostic job.max_output_bytes",
            minimum=1024,
            maximum=16 * 1024 * 1024,
        ),
        "env": dict(sorted(env.items())),
    }


def _jobs(path: Path, *, root: Path, workspace: Path, session: Mapping[str, Any]) -> list[dict[str, Any]]:
    value = _closed(
        _load_json(path, "diagnostic jobs"),
        allowed={"schema", "schema_version", "jobs"},
        required={"schema", "schema_version", "jobs"},
        label="diagnostic jobs",
    )
    if value["schema"] != JOBS_SCHEMA or value["schema_version"] != 1:
        _fail(f"diagnostic jobs must use {JOBS_SCHEMA}")
    if not isinstance(value["jobs"], list) or not value["jobs"]:
        _fail("diagnostic jobs.jobs must be a non-empty array")
    jobs = [_job_value(item, root=root, workspace=workspace, session=session) for item in value["jobs"]]
    ids = [job["job_id"] for job in jobs]
    if len(ids) != len(set(ids)):
        _fail("diagnostic job_id values must be unique")
    return jobs


def _job_fingerprint(session: Mapping[str, Any], candidate: Mapping[str, Any], job: Mapping[str, Any]) -> str:
    value = {
        "session_sha256": session["session_sha256"],
        "target_sha256": session["request"]["target"]["sha256"],
        "source_sha256": candidate["source"]["sha256"],
        "source_context_key": candidate["source_context_key"],
        "candidate_object_sha256": candidate["object"]["sha256"],
        "kind": job["kind"],
        "resource_class": job["resource_class"],
        "executable": (
            job["executable"]["path"],
            job["executable"]["size_bytes"],
            job["executable"]["sha256"],
        ),
        "argv": job["argv"],
        "cwd": job["cwd"],
        "inputs": [
            (item["path"], item["size_bytes"], item["sha256"])
            for item in job["inputs"]
        ],
        "outputs": job["outputs"],
        "env": job.get("env", {}),
        "timeout_seconds": job["timeout_seconds"],
        "max_output_bytes": job["max_output_bytes"],
    }
    return _sha256_bytes(_canonical(value))


def _expand_arg(arg: str, *, workspace: Path, output_root: Path, session: Mapping[str, Any], candidate: Mapping[str, Any]) -> str:
    target_blob = session.get("target_blob") or {}
    candidate_blob = candidate.get("object_blob") or {}
    values = {
        "{workspace}": os.fspath(workspace),
        "{output_root}": os.fspath(output_root),
        "{candidate_object}": os.fspath(_contained(workspace, candidate_blob.get("cas_path"), "candidate object CAS")),
        "{target_object}": os.fspath(_contained(workspace, target_blob.get("cas_path"), "session target CAS")),
    }
    result = arg
    for token, replacement in values.items():
        result = result.replace(token, replacement)
    return result


def _verify_job_inputs(job: Mapping[str, Any]) -> None:
    for label, item in [("executable", job["executable"]), *[(f"input[{i}]", value) for i, value in enumerate(job["inputs"])]]:
        current = _snapshot(Path(str(item["path"])), f"diagnostic {label}")
        if current["size_bytes"] != item["size_bytes"] or current["sha256"] != item["sha256"]:
            _fail(f"diagnostic {label} changed from its authenticated descriptor")


def _verify_frozen_artifacts(workspace: Path, session: Mapping[str, Any], candidate: Mapping[str, Any]) -> None:
    for label, item in (
        ("target", session.get("target_blob")),
        ("candidate object", candidate.get("object_blob")),
    ):
        if not isinstance(item, Mapping):
            _fail(f"diagnostic frozen {label} descriptor is missing")
        path = _contained(workspace, item.get("cas_path"), f"diagnostic frozen {label} path")
        current = _snapshot(path, f"diagnostic frozen {label}")
        if current["sha256"] != item.get("sha256") or current["size_bytes"] != item.get("size_bytes"):
            _fail(f"diagnostic frozen {label} changed")


def _validate_diagnostic_result(
    workspace: Path,
    session: Mapping[str, Any],
    candidate: Mapping[str, Any],
    job: Mapping[str, Any],
    value: Any,
    fingerprint: str,
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        _fail("diagnostic cached result must be an object")
    _verify_self_hash(value, "result_sha256", "diagnostic result")
    _closed(
        value,
        allowed={
            "schema", "schema_version", "session_sha256", "candidate_id",
            "candidate_object_sha256", "candidate_source_sha256", "source_context_key",
            "fingerprint", "job_id", "job_spec", "kind", "resource_class", "status", "returncode",
            "stdout", "stderr", "stdout_truncated", "stderr_truncated", "output_limit_exceeded", "duration_seconds",
            "outputs", "output_bytes", "cache_status", "authority_advanced", "error", "result_sha256",
        },
        required={
            "schema", "schema_version", "session_sha256", "candidate_id", "candidate_object_sha256",
            "candidate_source_sha256", "source_context_key", "fingerprint", "job_id", "job_spec",
            "kind", "resource_class", "status", "outputs", "cache_status", "authority_advanced",
            "result_sha256",
        },
        label="diagnostic result",
    )
    if value.get("schema") != DIAGNOSTIC_SCHEMA or value.get("schema_version") != 1:
        _fail("unsupported diagnostic result schema")
    if value.get("session_sha256") != session.get("session_sha256"):
        _fail("diagnostic result belongs to a different session")
    # candidate_id records the producing row. Exact source/object/context
    # fingerprints intentionally allow another duplicate row to reuse it, but
    # the producer must itself remain an authenticated compatible candidate.
    producer_id = _identifier(value.get("candidate_id"), "diagnostic producer candidate_id")
    if producer_id != candidate.get("candidate_id"):
        producer = _load_candidate(workspace, producer_id, session)
        if (
            producer.get("object", {}).get("sha256") != candidate.get("object", {}).get("sha256")
            or producer.get("source", {}).get("sha256") != candidate.get("source", {}).get("sha256")
            or producer.get("source_context_key") != candidate.get("source_context_key")
        ):
            _fail("diagnostic producer candidate is incompatible with cache reuse")
    if value.get("candidate_object_sha256") != candidate.get("object", {}).get("sha256"):
        _fail("diagnostic result candidate object mismatch")
    if value.get("candidate_source_sha256") != candidate.get("source", {}).get("sha256"):
        _fail("diagnostic result candidate source mismatch")
    if value.get("source_context_key") != candidate.get("source_context_key"):
        _fail("diagnostic result source context mismatch")
    if value.get("fingerprint") != fingerprint:
        _fail("diagnostic result fingerprint mismatch")
    if value.get("kind") != job.get("kind") or value.get("resource_class") != job.get("resource_class"):
        _fail("diagnostic cached job identity mismatch")
    _validate_indexed_diagnostic(workspace, session, value, fingerprint)
    return value


def _run_bounded_process(
    command: Sequence[str],
    *,
    cwd: str,
    env: Mapping[str, str],
    timeout_seconds: float,
    max_output_bytes: int,
    output_root: Path | None = None,
) -> tuple[int | None, bytes, bytes, bool, bool, bool, bool, float]:
    """Run a subprocess with bounded stdout/stderr readers and hard timeout."""
    process = subprocess.Popen(
        list(command),
        cwd=cwd,
        env=dict(env),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=False,
        stdin=subprocess.DEVNULL,
    )
    captured: dict[str, bytearray] = {"stdout": bytearray(), "stderr": bytearray()}
    overflow = {"stdout": False, "stderr": False}
    captured_budget = {"bytes": 0}
    captured_lock = threading.Lock()

    def _reader(name: str, stream: Any) -> None:
        try:
            while True:
                block = stream.read(65536)
                if not block:
                    return
                with captured_lock:
                    remaining = max_output_bytes - captured_budget["bytes"]
                    accepted = min(len(block), max(remaining, 0))
                    if accepted:
                        captured[name].extend(block[:accepted])
                        captured_budget["bytes"] += accepted
                    if accepted != len(block):
                        overflow[name] = True
                        return
        finally:
            stream.close()

    threads = [
        threading.Thread(target=_reader, args=(name, getattr(process, name)), daemon=True)
        for name in ("stdout", "stderr")
    ]
    for thread in threads:
        thread.start()
    started = time.monotonic()
    timed_out = False
    output_limited = False
    while process.poll() is None:
        if any(overflow.values()):
            output_limited = True
            process.kill()
            break
        if output_root is not None:
            try:
                output_total = 0
                for output_path in output_root.rglob("*"):
                    if output_path.is_file():
                        _assert_no_indirection(output_path)
                        output_total += output_path.stat().st_size
                        with captured_lock:
                            combined_output = output_total + captured_budget["bytes"]
                        if combined_output > max_output_bytes:
                            output_limited = True
                            process.kill()
                            break
                if output_limited:
                    break
            except (OSError, MatchError):
                output_limited = True
                process.kill()
                break
        if time.monotonic() - started >= timeout_seconds:
            timed_out = True
            process.kill()
            break
        time.sleep(0.01)
    try:
        process.wait(timeout=2.0)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=2.0)
    for thread in threads:
        thread.join(timeout=2.0)
    duration = time.monotonic() - started
    return (
        process.returncode,
        bytes(captured["stdout"]),
        bytes(captured["stderr"]),
        timed_out,
        output_limited or any(overflow.values()),
        bool(overflow["stdout"]),
        bool(overflow["stderr"]),
        duration,
    )


def _validate_indexed_diagnostic(
    workspace: Path, session: Mapping[str, Any], value: Any, fingerprint: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        _fail("diagnostic indexed event must be an object")
    _verify_self_hash(value, "result_sha256", "diagnostic indexed event")
    _closed(
        value,
        allowed={
            "schema", "schema_version", "session_sha256", "candidate_id",
            "candidate_object_sha256", "candidate_source_sha256", "source_context_key",
            "fingerprint", "job_id", "job_spec", "kind", "resource_class", "status", "returncode",
            "stdout", "stderr", "stdout_truncated", "stderr_truncated", "output_limit_exceeded",
            "duration_seconds", "outputs", "output_bytes", "cache_status", "authority_advanced",
            "error", "result_sha256",
        },
        required={
            "schema", "schema_version", "session_sha256", "candidate_id", "candidate_object_sha256",
            "candidate_source_sha256", "source_context_key", "fingerprint", "job_id", "job_spec", "kind",
            "resource_class", "status", "outputs", "output_bytes", "cache_status",
            "authority_advanced", "result_sha256",
        },
        label="diagnostic indexed event",
    )
    if value.get("schema") != DIAGNOSTIC_SCHEMA or value.get("schema_version") != 1:
        _fail("unsupported diagnostic indexed event schema")
    if value.get("session_sha256") != session.get("session_sha256"):
        _fail("diagnostic indexed event belongs to a different session")
    if value.get("fingerprint") != fingerprint:
        _fail("diagnostic indexed event fingerprint mismatch")
    producer_id = _identifier(value.get("candidate_id"), "diagnostic producer candidate_id")
    _sha256(value.get("candidate_object_sha256"), "diagnostic candidate object SHA-256")
    _sha256(value.get("candidate_source_sha256"), "diagnostic candidate source SHA-256")
    _sha256(value.get("source_context_key"), "diagnostic source context key")
    producer = _load_candidate(workspace, producer_id, session)
    if (
        value.get("candidate_object_sha256") != producer.get("object", {}).get("sha256")
        or value.get("candidate_source_sha256") != producer.get("source", {}).get("sha256")
        or value.get("source_context_key") != producer.get("source_context_key")
    ):
        _fail("diagnostic indexed producer binding does not match its candidate record")
    job_spec = value.get("job_spec")
    if not isinstance(job_spec, Mapping):
        _fail("diagnostic indexed job_spec is invalid")
    normalized_job = _job_value(
        job_spec,
        root=Path(str(session["root"])),
        workspace=workspace,
        session=session,
    )
    if normalized_job != job_spec:
        _fail("diagnostic indexed job_spec is not canonical")
    if value.get("job_id") != normalized_job.get("job_id"):
        _fail("diagnostic indexed producer job_id does not match job_spec")
    if (
        value.get("kind") != normalized_job.get("kind")
        or value.get("resource_class") != normalized_job.get("resource_class")
    ):
        _fail("diagnostic indexed job labels do not match job_spec")
    candidate_identity = {
        "source": {"sha256": value["candidate_source_sha256"]},
        "object": {"sha256": value["candidate_object_sha256"]},
        "source_context_key": value["source_context_key"],
    }
    if _job_fingerprint(session, candidate_identity, normalized_job) != fingerprint:
        _fail("diagnostic indexed job_spec does not produce its fingerprint")
    if not SAFE_ID_RE.fullmatch(str(value.get("job_id", ""))):
        _fail("diagnostic indexed event job_id is invalid")
    if value.get("resource_class") not in SAFE_RESOURCE_CLASSES:
        _fail("diagnostic indexed event uses a non-read-only resource class")
    if value.get("status") not in {"passed", "failed", "timeout"}:
        _fail("diagnostic indexed event status is invalid")
    if value.get("authority_advanced") is not False:
        _fail("diagnostic indexed event must not advance authority")
    if not isinstance(value.get("output_limit_exceeded", False), bool):
        _fail("diagnostic indexed output limit flag is invalid")
    for flag in ("stdout_truncated", "stderr_truncated"):
        if flag in value and not isinstance(value.get(flag), bool):
            _fail(f"diagnostic indexed {flag} flag is invalid")
    returncode = value.get("returncode")
    if returncode is not None and (isinstance(returncode, bool) or not isinstance(returncode, int)):
        _fail("diagnostic indexed returncode is invalid")
    if value.get("cache_status") != "ran":
        _fail("immutable diagnostic indexed event must record cache_status=ran")
    if "duration_seconds" in value:
        _seconds(value.get("duration_seconds"), "diagnostic indexed duration_seconds")
    for stream in ("stdout", "stderr"):
        if stream in value and not isinstance(value.get(stream), str):
            _fail(f"diagnostic indexed {stream} must be text")
    outputs = value.get("outputs")
    if (
        not isinstance(outputs, list)
        or isinstance(value.get("output_bytes"), bool)
        or not isinstance(value.get("output_bytes"), int)
        or value.get("output_bytes", -1) < 0
    ):
        _fail("diagnostic indexed event outputs are invalid")
    output_root = _contained(workspace, f"job-output/{fingerprint}", "diagnostic indexed output root")
    if not all(isinstance(output, Mapping) for output in outputs):
        _fail("diagnostic indexed output is invalid")
    observed_output_paths = [
        os.path.normcase(os.path.abspath(_text(output.get("path"), "diagnostic output.path")))
        for output in outputs
    ]
    expected_output_paths = [
        os.path.normcase(os.path.abspath(os.fspath(output_root / relative)))
        for relative in normalized_job["outputs"]
    ]
    if observed_output_paths != expected_output_paths:
        _fail("diagnostic indexed outputs do not match the authenticated job_spec")
    for output in outputs:
        if output.get("missing") is True:
            _closed(output, allowed={"path", "missing"}, required={"path", "missing"}, label="diagnostic missing output")
        elif output.get("too_large") is True:
            _closed(output, allowed={"path", "size_bytes", "too_large"}, required={"path", "size_bytes", "too_large"}, label="diagnostic oversized output")
            _integer(output.get("size_bytes"), "diagnostic oversized output.size_bytes")
        else:
            _closed(output, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label="diagnostic output descriptor")
            _integer(output.get("size_bytes"), "diagnostic output.size_bytes")
            _sha256(output.get("sha256"), "diagnostic output.sha256")
        output_path = Path(str(output.get("path", "")))
        if not output_path.is_absolute() or ".." in output_path.parts:
            _fail("diagnostic indexed output path is not canonical absolute")
        try:
            output_path.relative_to(output_root)
        except ValueError:
            _fail("diagnostic indexed output escapes its private root")
        if output.get("missing"):
            if output_path.exists():
                _fail("diagnostic indexed missing output is present")
        elif output.get("too_large"):
            _assert_no_indirection(output_path)
            if not output_path.is_file() or output_path.stat().st_size != output.get("size_bytes"):
                _fail("diagnostic indexed oversized output changed")
        else:
            _recheck_descriptor(output, "diagnostic indexed output")
    recomputed_output_bytes = len(str(value.get("stdout", "")).encode("utf-8")) + len(
        str(value.get("stderr", "")).encode("utf-8")
    )
    recomputed_output_bytes += sum(
        int(output.get("size_bytes", 0))
        for output in outputs
    )
    if value.get("output_bytes") != recomputed_output_bytes:
        _fail("diagnostic indexed output byte accounting mismatch")
    if value.get("status") == "passed":
        if (
            returncode != 0
            or value.get("output_limit_exceeded")
            or value.get("stdout_truncated")
            or value.get("stderr_truncated")
            or value.get("output_bytes", 0) > normalized_job["max_output_bytes"]
        ):
            _fail("passed diagnostic indexed event has failed process semantics")
        if any(output.get("missing") or output.get("too_large") for output in outputs):
            _fail("passed diagnostic indexed event has incomplete outputs")
    return value


def _run_job_unlocked(workspace: Path, session: Mapping[str, Any], candidate: Mapping[str, Any], job: Mapping[str, Any]) -> dict[str, Any]:
    fingerprint = _job_fingerprint(session, candidate, job)
    result_path = workspace / "diagnostics" / f"{fingerprint}.json"
    index = _load_index(workspace, session)
    indexed_path = index["diagnostic_index"].get(fingerprint)
    expected_path = result_path.relative_to(workspace).as_posix()
    if indexed_path is not None and indexed_path != expected_path:
        _fail("diagnostic index path does not match its immutable fingerprint")
    if indexed_path is not None and not result_path.is_file():
        _fail("immutable diagnostic index entry has no result event")
    if result_path.is_file():
        existing = _load_json(result_path, "diagnostic result")
        _validate_diagnostic_result(workspace, session, candidate, job, existing, fingerprint)
        if indexed_path is None:
            _persist_diagnostic_index(workspace, session, fingerprint, expected_path)
        return _with_self_hash(
            {**dict(existing), "cache_status": "cached", "requested_job_id": job["job_id"]},
            "result_sha256",
        )
    output_root = workspace / "job-output" / fingerprint
    if output_root.exists():
        _fail(f"uncached diagnostic output root already exists: {output_root}")
    _safe_mkdir(output_root)
    _verify_job_inputs(job)
    _verify_frozen_artifacts(workspace, session, candidate)
    command = [str(job["executable"]["path"])] + [
        _expand_arg(arg, workspace=workspace, output_root=output_root, session=session, candidate=candidate)
        for arg in job["argv"]
    ]
    environment = dict(job.get("env", {}))
    environment["MATCH_WORKBENCH_OUTPUT_ROOT"] = os.fspath(output_root)
    environment["MATCH_WORKBENCH_READ_ONLY"] = "1"
    process_returncode, stdout_raw, stderr_raw, timed_out, output_limited, stdout_overflow, stderr_overflow, duration_seconds = _run_bounded_process(
        command,
        cwd=job["cwd"],
        env=environment,
        timeout_seconds=float(job["timeout_seconds"]),
        max_output_bytes=int(job["max_output_bytes"]),
        output_root=output_root,
    )
    limit = int(job["max_output_bytes"])
    stdout_truncated = stdout_overflow
    stderr_truncated = stderr_overflow
    stdout = stdout_raw[:limit].decode("utf-8", errors="replace")
    stderr = stderr_raw[:limit].decode("utf-8", errors="replace")
    _verify_job_inputs(job)
    _verify_frozen_artifacts(workspace, session, candidate)
    declared_output_paths = {
        os.path.normcase(os.path.abspath(os.fspath(output_root / relative)))
        for relative in job["outputs"]
    }
    actual_output_paths: set[str] = set()
    for item in output_root.rglob("*"):
        _assert_no_indirection(item)
        if item.is_file():
            actual_output_paths.add(os.path.normcase(os.path.abspath(os.fspath(item))))
    undeclared_output_paths = sorted(actual_output_paths - declared_output_paths)
    if undeclared_output_paths:
        _fail(
            "diagnostic wrote undeclared private outputs: "
            + ", ".join(undeclared_output_paths)
        )
    outputs = []
    # Persist byte accounting in terms of the canonical UTF-8 text that is
    # actually stored in the event, so validators can recompute it exactly.
    output_bytes = len(stdout.encode("utf-8")) + len(stderr.encode("utf-8"))
    output_limit_exceeded = output_limited or output_bytes > limit
    for relative in job["outputs"]:
        output = _resolve(relative, output_root)
        try:
            output.relative_to(output_root)
        except ValueError:
            _fail(f"diagnostic output escaped its private root: {output}")
        if os.path.lexists(output) and not output.is_file():
            _assert_no_indirection(output)
            _fail(f"diagnostic output is not a regular file: {output}")
        if output.is_file():
            try:
                declared_size = output.stat().st_size
            except OSError as exc:
                raise MatchError(f"cannot stat diagnostic output: {output}") from exc
            if output_bytes + declared_size > limit:
                output_limit_exceeded = True
                output_bytes += declared_size
                outputs.append({"path": os.fspath(output), "size_bytes": declared_size, "too_large": True})
                continue
            current = _snapshot(output, "diagnostic output")
            current_size = int(current["size_bytes"])
            if output_bytes + current_size > limit:
                output_limit_exceeded = True
                output_bytes += current_size
                outputs.append({"path": os.fspath(output), "size_bytes": current_size, "too_large": True})
                continue
            outputs.append({key: current[key] for key in ("path", "size_bytes", "sha256")})
            output_bytes += current_size
        else:
            outputs.append({"path": os.fspath(output), "missing": True})
    status = "timeout" if timed_out else ("failed" if output_limit_exceeded else ("passed" if process_returncode == 0 and not any(item.get("missing") for item in outputs) else "failed"))
    body = {
        "schema": DIAGNOSTIC_SCHEMA,
        "schema_version": 1,
        "session_sha256": session["session_sha256"],
        "candidate_id": candidate["candidate_id"],
        "candidate_object_sha256": candidate["object"]["sha256"],
        "candidate_source_sha256": candidate["source"]["sha256"],
        "source_context_key": candidate["source_context_key"],
        "fingerprint": fingerprint,
        "job_id": job["job_id"],
        "job_spec": dict(job),
        "kind": job["kind"],
        "resource_class": job["resource_class"],
        "status": status,
        "returncode": process_returncode,
        "stdout": stdout,
        "stderr": stderr,
        "stdout_truncated": stdout_truncated,
        "stderr_truncated": stderr_truncated,
        "output_limit_exceeded": output_limit_exceeded,
        "duration_seconds": round(duration_seconds, 6),
        "outputs": outputs,
        "output_bytes": output_bytes,
        "cache_status": "ran",
        "authority_advanced": False,
    }
    if output_limit_exceeded:
        body["error"] = "diagnostic stdout/stderr or declared outputs exceeded max_output_bytes"
    result = _with_self_hash(body, "result_sha256")
    _write_new(result_path, _canonical(result))
    return result


def _run_job(workspace: Path, session: Mapping[str, Any], candidate: Mapping[str, Any], job: Mapping[str, Any]) -> dict[str, Any]:
    fingerprint = _job_fingerprint(session, candidate, job)
    lock = workspace / "diagnostics" / f".{fingerprint}.lock"
    try:
        with _workbench_lock(lock, float(job["timeout_seconds"]) + 8.0):
            result = _run_job_unlocked(workspace, session, candidate, job)
            _persist_diagnostic_index(
                workspace,
                session,
                fingerprint,
                (workspace / "diagnostics" / f"{fingerprint}.json").relative_to(workspace).as_posix(),
            )
            return result
    except ValueError as exc:
        raise MatchError(str(exc)) from exc


def _cleanup_attempt(workspace: Path, fingerprint: str) -> None:
    """Remove only a failed private attempt directory, never caller inputs."""
    attempt = _contained(workspace, f"job-output/{fingerprint}", "diagnostic attempt")
    if not attempt.exists():
        return
    _assert_no_indirection(attempt)
    for child in sorted(attempt.rglob("*"), key=lambda item: len(item.parts), reverse=True):
        _assert_no_indirection(child)
        if child.is_file():
            child.unlink()
        elif child.is_dir():
            child.rmdir()
    attempt.rmdir()


def diagnose_candidate(
    root: Path,
    workspace: Path | str,
    candidate_id: str,
    jobs_path: Path | str,
    *,
    max_workers: int | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    destination = _workspace(workspace, root)
    session = _load_session(destination, root)
    candidate_id = _identifier(candidate_id, "candidate_id")
    candidate = _load_candidate(destination, candidate_id, session)
    _require_candidate_compile_attestation(candidate, session)
    jobs = _jobs(_resolve(jobs_path, root), root=root, workspace=destination, session=session)
    workers = max_workers if max_workers is not None else session["request"]["policy"]["max_workers"]
    workers = _integer(workers, "max_workers", minimum=1, maximum=session["request"]["policy"]["max_workers"])
    by_fingerprint: dict[str, dict[str, Any]] = {}
    aliases: dict[str, list[str]] = {}
    for job in jobs:
        fingerprint = _job_fingerprint(session, candidate, job)
        aliases.setdefault(fingerprint, []).append(job["job_id"])
        by_fingerprint.setdefault(fingerprint, job)
    results: dict[str, Mapping[str, Any]] = {}
    with ThreadPoolExecutor(max_workers=min(workers, len(by_fingerprint))) as pool:
        futures = {
            pool.submit(_run_job, destination, session, candidate, job): fingerprint
            for fingerprint, job in by_fingerprint.items()
        }
        for future in as_completed(futures):
            fingerprint = futures[future]
            try:
                results[fingerprint] = future.result()
            except (MatchError, OSError, subprocess.SubprocessError) as exc:
                result_path = destination / "diagnostics" / f"{fingerprint}.json"
                if result_path.exists():
                    try:
                        recovered = _load_json(result_path, "diagnostic recovery result")
                        _validate_diagnostic_result(
                            destination,
                            session,
                            candidate,
                            by_fingerprint[fingerprint],
                            recovered,
                            fingerprint,
                        )
                        _persist_diagnostic_index(
                            destination,
                            session,
                            fingerprint,
                            result_path.relative_to(destination).as_posix(),
                        )
                    except MatchError:
                        # Keep the original exception below; a corrupt immutable
                        # event remains fail-closed and is never replaced.
                        pass
                    else:
                        results[fingerprint] = _with_self_hash(
                            {**dict(recovered), "cache_status": "recovered"},
                            "result_sha256",
                        )
                        continue
                failed_job = by_fingerprint[fingerprint]
                failed_output_root = destination / "job-output" / fingerprint
                failed_outputs = [
                    {"path": os.fspath(failed_output_root / relative), "missing": True}
                    for relative in failed_job["outputs"]
                ]
                failed = _with_self_hash({
                    "schema": DIAGNOSTIC_SCHEMA,
                    "schema_version": 1,
                    "session_sha256": session["session_sha256"],
                    "candidate_id": candidate["candidate_id"],
                    "candidate_object_sha256": candidate["object"]["sha256"],
                    "candidate_source_sha256": candidate["source"]["sha256"],
                    "source_context_key": candidate["source_context_key"],
                    "fingerprint": fingerprint,
                    "job_id": failed_job["job_id"],
                    "job_spec": dict(failed_job),
                    "kind": failed_job["kind"],
                    "resource_class": failed_job["resource_class"],
                    "status": "failed",
                    "error": str(exc),
                    "outputs": failed_outputs,
                    "output_bytes": 0,
                    "cache_status": "ran",
                    "authority_advanced": False,
                }, "result_sha256")
                try:
                    _cleanup_attempt(destination, fingerprint)
                except MatchError:
                    # Preserve the primary diagnostic failure; a suspicious
                    # attempt path remains fail-closed for matrix validation.
                    pass
                if not result_path.exists():
                    _write_new(result_path, _canonical(failed))
                else:
                    # An invalid immutable event cannot be safely overwritten
                    # or indexed as this synthesized failure.
                    results[fingerprint] = failed
                    continue
                _persist_diagnostic_index(
                    destination,
                    session,
                    fingerprint,
                    result_path.relative_to(destination).as_posix(),
                )
                results[fingerprint] = failed
    rows = []
    for fingerprint in sorted(aliases):
        result = dict(results[fingerprint])
        for offset, job_id in enumerate(sorted(aliases[fingerprint])):
            row = dict(result)
            row["requested_job_id"] = job_id
            if offset:
                row["cache_status"] = "deduplicated_in_run"
            rows.append(_with_self_hash(row, "result_sha256"))
    summary = {
        "ran": sum(row.get("cache_status") == "ran" for row in rows),
        "cached": sum(row.get("cache_status") in {"cached", "recovered", "deduplicated_in_run"} for row in rows),
        "failed": sum(row.get("status") != "passed" for row in rows),
    }
    return {
        "schema": "match_workbench_diagnostic_batch/v1",
        "schema_version": 1,
        "candidate_id": candidate_id,
        "jobs": sorted(rows, key=lambda row: (str(row.get("kind")), str(row.get("requested_job_id")))),
        "summary": summary,
        "authority_advanced": False,
    }


def _candidate_is_no_go(candidate: Mapping[str, Any]) -> bool:
    """Return whether an outcome is explicitly closed against advancement.

    Older ledgers used both a plain ``rejected`` status and free-form no-go
    reasons.  Keep those records readable, but make the matrix conservative:
    neither spelling may turn into a proof/closure recommendation.
    """

    outcome = candidate.get("outcome")
    if not isinstance(outcome, Mapping):
        return False
    status = str(outcome.get("status", "")).strip().casefold()
    reason = str(outcome.get("reason", "")).strip().casefold()
    normalized_status = re.sub(r"[_/ ]+", "-", status)
    normalized_reason = re.sub(r"[_/ ]+", "-", reason)
    return (
        normalized_status.startswith("reject")
        or "no-go" in normalized_status
        or "no-go" in normalized_reason
    )


def _matrix_focus_row(
    row: Mapping[str, Any],
    symbol: str,
    field: str,
) -> Mapping[str, Any] | None:
    """Select one focus metric from a full matrix row.

    Candidate records may retain one focus under ``*_focus`` or several under
    ``*_focuses``.  The compact history view must not make callers load the
    entire report or guess which paired metric belongs to the requested symbol.
    """
    names = row.get("focus_symbols")
    plural_name = f"{field}es" if field.endswith("focus") else f"{field}s"
    plural = row.get(plural_name)
    if isinstance(names, list) and isinstance(plural, list):
        for index, name in enumerate(names):
            if name == symbol and index < len(plural):
                value = plural[index]
                return value if isinstance(value, Mapping) else None
    if row.get("focus_symbol") == symbol:
        value = row.get(field)
        return value if isinstance(value, Mapping) else None
    return None


def _matrix_row_focus_name(
    row: Mapping[str, Any],
    requested: Sequence[str] | None,
    default: str,
) -> str:
    """Return the stable focus name represented by a selected matrix row."""
    if requested:
        names = row.get("focus_symbols")
        if isinstance(names, list):
            for name in requested:
                if name in names:
                    return name
        if row.get("focus_symbol") in requested:
            return str(row["focus_symbol"])
    value = row.get("focus_symbol")
    if isinstance(value, str) and value:
        return value
    names = row.get("focus_symbols")
    if isinstance(names, list) and names and isinstance(names[0], str):
        return names[0]
    return default


def _matrix_history_metric(value: Mapping[str, Any] | None) -> dict[str, Any] | None:
    """Keep only the bounded focus evidence useful for candidate history."""
    if not isinstance(value, Mapping):
        return None
    return {
        "match_percent": value.get("match_percent"),
        "diff_rows": value.get("diff_rows"),
        "target_size": _assessment_number(value.get("target_size")),
        "candidate_size": _assessment_number(value.get("candidate_size")),
        "exact": value.get("exact"),
    }


def _matrix_history_row(
    row: Mapping[str, Any],
    candidate: Mapping[str, Any],
    focus_symbol: str,
) -> dict[str, Any]:
    """Build one bounded, report-free history row for a focus symbol."""
    strict = _matrix_history_metric(_matrix_focus_row(row, focus_symbol, "strict_focus"))
    data = _matrix_history_metric(_matrix_focus_row(row, focus_symbol, "data_focus"))
    hypothesis = candidate.get("hypothesis")
    if not isinstance(hypothesis, Mapping):
        hypothesis = {}
    outcome = candidate.get("outcome")
    if not isinstance(outcome, Mapping):
        outcome = {}
    strict_percent = strict.get("match_percent") if strict else None
    data_percent = data.get("match_percent") if data else None
    strict_diff_rows = strict.get("diff_rows") if strict else None
    data_diff_rows = data.get("diff_rows") if data else None
    return {
        "ordinal": row["ordinal"],
        "candidate_id": row["candidate_id"],
        "focus_symbol": focus_symbol,
        "name": hypothesis.get("name"),
        "axis": hypothesis.get("axis"),
        "hypothesis_name": hypothesis.get("name"),
        "hypothesis_axis": hypothesis.get("axis"),
        "source_sha256": candidate["source"]["sha256"],
        "object_sha256": candidate["object"]["sha256"],
        "strict": strict,
        "data": data,
        "strict_percent": strict_percent,
        "data_percent": data_percent,
        "strict_diff_rows": strict_diff_rows,
        "data_diff_rows": data_diff_rows,
        "diff_rows": {"strict": strict_diff_rows, "data": data_diff_rows},
        "target_size": strict.get("target_size") if strict else None,
        "candidate_size": strict.get("candidate_size") if strict else None,
        "outcome": {
            "status": outcome.get("status"),
            "reason": outcome.get("reason"),
        },
    }


def build_matrix(
    root: Path,
    workspace: Path | str,
    *,
    focus_symbol: str | Sequence[str] | None = None,
    limit: int | None = None,
    compact: bool = False,
    order: str = "oldest",
    latest: bool = False,
) -> dict[str, Any]:
    """Build the full matrix or a bounded focus-history projection.

    With no optional arguments this is byte-for-byte compatible with the
    original full matrix shape.  ``focus_symbol``/``limit`` filter rows, and
    ``compact`` removes report bindings and aggregates so a caller can inspect
    one owner's candidate history without loading every immutable report.
    """
    if order not in {"oldest", "newest"}:
        _fail("matrix order must be 'oldest' or 'newest'")
    if not isinstance(latest, bool):
        _fail("matrix latest must be boolean")
    if latest:
        order = "newest"
    if limit is not None:
        limit = _integer(limit, "matrix limit", minimum=0)
    query_focuses = (
        None
        if focus_symbol is None
        else _focus_symbols(focus_symbol, "matrix focus_symbols")
    )
    destination = _workspace(workspace, root.resolve())
    session = _load_session(destination, root.resolve())
    index = _load_index(destination, session)
    rows = []
    diagnostics_by_pair: dict[tuple[str, str], list[Mapping[str, Any]]] = {}
    indexed_diagnostic_paths: set[str] = set()
    for fingerprint, relative in sorted(index["diagnostic_index"].items()):
        _sha256(fingerprint, "diagnostic index fingerprint")
        path = _contained(destination, relative, "diagnostic index path")
        if path.name != f"{fingerprint}.json" or path.parent != destination / "diagnostics":
            _fail("diagnostic index path is not an immutable diagnostic event")
        value = _load_json(path, "diagnostic result")
        _validate_indexed_diagnostic(destination, session, value, fingerprint)
        pair = (str(value.get("candidate_object_sha256")), str(value.get("source_context_key")))
        diagnostics_by_pair.setdefault(pair, []).append(value)
        indexed_diagnostic_paths.add(path.name)
    actual_diagnostic_paths = {path.name for path in (destination / "diagnostics").glob("*.json")}
    if actual_diagnostic_paths != indexed_diagnostic_paths:
        _fail("diagnostic index does not cover every immutable diagnostic event")
    candidates = [
        _load_candidate(destination, candidate_id, session)
        for candidate_id in sorted(index["candidates"])
    ]
    indexed_candidate_paths = {Path(relative).name for relative in index["candidates"].values()}
    candidate_directory = destination / "candidates"
    actual_candidate_paths = {
        path.name
        for path in candidate_directory.glob("*.json")
        if path.name != "manifest.json"
    }
    candidate_manifest = candidate_directory / "manifest.json"
    if candidate_manifest.exists():
        manifest_value = _load_json(candidate_manifest, "candidate generation manifest")
        if not isinstance(manifest_value, Mapping):
            _fail("candidate generation manifest must be an object")
        manifest_schema = _text(
            manifest_value.get("schema"), "candidate generation manifest schema"
        )
        if manifest_schema == CANDIDATE_SCHEMA:
            _fail("candidate generation manifest cannot contain an immutable candidate record")
    if actual_candidate_paths != indexed_candidate_paths:
        _fail("candidate index does not cover every immutable candidate record")
    if any(candidate.get("session_sha256") != session.get("session_sha256") for candidate in candidates):
        _fail("candidate index contains a record from a different session")
    candidates.sort(key=lambda value: (int(value["ordinal"]), str(value["candidate_id"])))
    previous = None
    for candidate in candidates:
        if candidate.get("previous_record_sha256") != previous:
            _fail(f"candidate record chain mismatch at {candidate['candidate_id']}")
        previous = candidate["record_sha256"]
    if len(candidates) != int(index["sequence"]) or previous != index.get("last_record_sha256"):
        _fail("candidate index sequence does not match the immutable record chain")
    candidate_by_id = {str(candidate["candidate_id"]): candidate for candidate in candidates}
    for source_key, candidate_id in index["source_context_index"].items():
        if candidate_by_id[candidate_id].get("source_context_key") != source_key:
            _fail("source/context index does not match its immutable candidate record")
    for object_key, candidate_id in index["object_index"].items():
        if candidate_by_id[candidate_id].get("object_result_key") != object_key:
            _fail("object index does not match its immutable candidate record")
    known_pairs = {
        (str(candidate["object"]["sha256"]), str(candidate["source_context_key"]))
        for candidate in candidates
    }
    orphan_pairs = set(diagnostics_by_pair) - known_pairs
    if orphan_pairs:
        _fail("diagnostic index contains evidence for an unknown candidate context")
    for pair, events in diagnostics_by_pair.items():
        for event in events:
            producer = candidate_by_id.get(str(event.get("candidate_id")))
            if producer is None:
                _fail("diagnostic indexed event names an unknown producer candidate")
            producer_pair = (
                str(producer["object"]["sha256"]),
                str(producer["source_context_key"]),
            )
            if producer_pair != pair:
                _fail("diagnostic indexed producer does not match its evidence context")
    for expected_ordinal, candidate in enumerate(candidates, 1):
        if candidate.get("ordinal") != expected_ordinal:
            _fail("candidate record ordinals are not contiguous")
        candidate_id = str(candidate["candidate_id"])
        duplicate_id = candidate.get("duplicate_of")
        if duplicate_id is not None:
            duplicate = candidate_by_id.get(str(duplicate_id))
            if duplicate is None or duplicate.get("object", {}).get("sha256") != candidate.get("object", {}).get("sha256"):
                _fail("candidate duplicate_of does not reference identical object evidence")
            if int(duplicate.get("ordinal", 0)) >= int(candidate["ordinal"]):
                _fail("candidate duplicate_of must reference an earlier immutable record")
        strict_compact = candidate["reports"]["strict"]["compact"]
        strict_focus_rows = _compact_focus_rows(
            strict_compact, f"candidate {candidate_id} strict compact"
        )
        strict_focus = strict_focus_rows[0] if len(strict_focus_rows) == 1 else None
        data_value = candidate["reports"].get("data")
        data_focus_rows = (
            _compact_focus_rows(
                data_value["compact"], f"candidate {candidate_id} data compact"
            )
            if isinstance(data_value, Mapping)
            else []
        )
        data_focus = data_focus_rows[0] if len(data_focus_rows) == 1 else None
        jobs = diagnostics_by_pair.get(
            (str(candidate["object"]["sha256"]), str(candidate["source_context_key"])), []
        )
        diagnostic_status = "not_run" if not jobs else ("available" if all(job.get("status") == "passed" for job in jobs) else "failed")
        focus_exact = (
            bool(strict_focus_rows)
            and all(bool(row and row.get("exact")) for row in strict_focus_rows)
            and all(bool(row and row.get("exact")) for row in data_focus_rows)
        )
        record_focuses = _stored_focus_symbols(
            candidate,
            default=str(session["request"]["function"]),
        )
        duplicate = candidate_by_id.get(str(candidate.get("duplicate_of"))) if candidate.get("duplicate_of") else None
        duplicate_same_context = bool(
            duplicate is not None
            and duplicate.get("source_context_key") == candidate.get("source_context_key")
        )
        duplicate_same_evidence = bool(
            duplicate_same_context
            and {
                name: report.get("raw_sha256") if isinstance(report, Mapping) else None
                for name, report in duplicate.get("reports", {}).items()
            }
            == {
                name: report.get("raw_sha256") if isinstance(report, Mapping) else None
                for name, report in candidate.get("reports", {}).items()
            }
        )
        if _candidate_is_no_go(candidate):
            next_action = "do_not_advance_rejected_candidate"
        elif duplicate_same_evidence:
            next_action = "reuse_existing_evidence"
        elif candidate.get("duplicate_of") and diagnostic_status == "not_run":
            next_action = "run_read_only_diagnostics_for_source_context"
        elif focus_exact:
            next_action = "authenticate_report_binding_then_run_serial_proof_and_closure"
        elif diagnostic_status == "failed":
            next_action = "repair_read_only_diagnostic"
        else:
            next_action = "continue_one_axis_matching"
        row = {
            "ordinal": candidate["ordinal"],
            "candidate_id": candidate_id,
            "source_sha256": candidate["source"]["sha256"],
            "object_sha256": candidate["object"]["sha256"],
            "duplicate_of": candidate.get("duplicate_of"),
            "hypothesis_axis": candidate["hypothesis"]["axis"],
            "axis_fingerprint": candidate["hypothesis"]["axis_fingerprint"],
            "residual_fingerprint": candidate["hypothesis"].get("residual_fingerprint"),
            "outcome": candidate["outcome"],
            "focus_symbol": record_focuses[0] if len(record_focuses) == 1 else None,
            "strict_focus": strict_focus,
            "data_focus": data_focus,
            "report_binding": candidate["report_binding"],
            "diagnostic_status": diagnostic_status,
            "available_read_only_evidence": diagnostic_status,
            "heavy_seconds": candidate.get("telemetry", {}).get("heavy_seconds"),
            "next_action": next_action,
        }
        if len(record_focuses) > 1:
            row["focus_symbols"] = list(record_focuses)
            row["strict_focuses"] = strict_focus_rows
            row["data_focuses"] = data_focus_rows
        rows.append(row)
    rows.sort(key=lambda row: (int(row["ordinal"]), str(row["candidate_id"])))
    raw_report_bytes = sum(
        int(report["raw_size_bytes"])
        for candidate in candidates
        for report in candidate["reports"].values()
        if isinstance(report, Mapping)
    )
    unique_reports = {
        str(report["raw_sha256"]): int(report["compressed_size_bytes"])
        for candidate in candidates
        for report in candidate["reports"].values()
        if isinstance(report, Mapping)
    }
    heavy_seconds = sum(
        float(candidate.get("telemetry", {}).get("heavy_seconds") or 0.0)
        for candidate in candidates
    )
    diagnostic_seconds = sum(
        float(value.get("duration_seconds") or 0.0)
        for values in diagnostics_by_pair.values()
        for value in values
    )
    exact_focus_bytes = 0
    for row in rows:
        focus_rows = row.get("strict_focuses")
        if not isinstance(focus_rows, list):
            focus_rows = [row.get("strict_focus")]
        exact_focus_bytes += sum(
            int(focus["target_size"])
            for focus in focus_rows
            if isinstance(focus, Mapping)
            and focus.get("exact")
            and str(focus.get("target_size", "")).isdigit()
        )
    aggregate = {
        "candidate_count": len(rows),
        "unique_object_count": len({row["object_sha256"] for row in rows}),
        "duplicate_candidate_count": sum(bool(row["duplicate_of"]) for row in rows),
        "diagnosed_candidate_count": sum(row["diagnostic_status"] != "not_run" for row in rows),
        "raw_report_bytes": raw_report_bytes,
        "unique_compressed_report_bytes": sum(unique_reports.values()),
        "report_storage_reduction_ratio": (
            round(1.0 - (sum(unique_reports.values()) / raw_report_bytes), 6)
            if raw_report_bytes else 0.0
        ),
        "heavy_seconds": round(heavy_seconds, 6),
        "diagnostic_seconds": round(diagnostic_seconds, 6),
        "exact_focus_bytes": exact_focus_bytes,
        "exact_focus_bytes_per_heavy_minute": (
            round(exact_focus_bytes / (heavy_seconds / 60.0), 6)
            if heavy_seconds else None
        ),
    }
    body = {
        "schema": MATRIX_SCHEMA,
        "schema_version": 1,
        "session_id": session["session_id"],
        "columns": [
            "ordinal", "candidate_id", "source_sha256", "object_sha256", "duplicate_of",
            "hypothesis_axis", "axis_fingerprint", "residual_fingerprint", "strict_focus",
            "focus_symbol", "focus_symbols", "data_focus", "strict_focuses", "data_focuses",
            "outcome", "report_binding", "diagnostic_status",
            "available_read_only_evidence", "next_action",
            "heavy_seconds",
        ],
        "rows": rows,
        "aggregate": aggregate,
        "authority_advanced": False,
    }
    if query_focuses is None and limit is None and not compact and order == "oldest":
        return _with_self_hash(body, "matrix_sha256")

    selected_rows = rows
    if query_focuses is not None:
        selected_rows = [
            row
            for row in rows
            if any(
                _matrix_focus_row(row, symbol, "strict_focus") is not None
                or _matrix_focus_row(row, symbol, "data_focus") is not None
                for symbol in query_focuses
            )
        ]
    if order == "newest":
        selected_rows = list(reversed(selected_rows))
    if limit is not None:
        selected_rows = selected_rows[:limit]
    query = {
        "focus_symbols": list(query_focuses) if query_focuses is not None else None,
        "limit": limit,
        "order": order,
        "total_candidate_count": len(rows),
        "selected_candidate_count": len(selected_rows),
    }
    if not compact:
        body["view"] = "filtered"
        body["query"] = query
        body["rows"] = selected_rows
        body["aggregate"] = {
            **body["aggregate"],
            "selected_candidate_count": len(selected_rows),
        }
        return _with_self_hash(body, "matrix_sha256")

    candidate_lookup = {str(candidate["candidate_id"]): candidate for candidate in candidates}
    compact_rows = []
    for selected in selected_rows:
        candidate = candidate_lookup[str(selected["candidate_id"])]
        selected_focus = _matrix_row_focus_name(
            selected,
            query_focuses,
            default=str(session["request"]["function"]),
        )
        compact_rows.append(_matrix_history_row(selected, candidate, selected_focus))
    compact_body = {
        "schema": MATRIX_SCHEMA,
        "schema_version": 1,
        "session_id": session["session_id"],
        "view": "compact",
        "query": query,
        "columns": [
            "ordinal", "candidate_id", "focus_symbol", "name", "axis",
            "source_sha256", "object_sha256", "strict", "data", "outcome",
        ],
        "rows": compact_rows,
        "aggregate": {
            "total_candidate_count": len(rows),
            "selected_candidate_count": len(compact_rows),
            "exact_strict_count": sum(
                bool(row.get("strict", {}).get("exact"))
                for row in compact_rows
                if isinstance(row.get("strict"), Mapping)
            ),
            "exact_data_count": sum(
                bool(row.get("data", {}).get("exact"))
                for row in compact_rows
                if isinstance(row.get("data"), Mapping)
            ),
        },
        "authority_advanced": False,
    }
    return _with_self_hash(compact_body, "matrix_sha256")


def reduce_objdiff_cascades(
    root: Path,
    *,
    report: Path | str,
    focus_symbol: str,
    target_assembly: Path | str | None = None,
    candidate_assembly: Path | str | None = None,
    summary_only: bool = True,
    max_hypotheses: int = 20,
    include_exact_residuals: bool = False,
) -> dict[str, Any]:
    """Bind and reduce one authenticated objdiff function residual.

    The reducer is deliberately read-only and authority-free.  It groups
    repeated structural mismatch signatures and may rank bounded natural-C
    diagnostics, but it never claims source provenance or candidate retention.
    """

    from tools.mismatch_cluster_audit import AuditInputError, audit_document

    root = root.resolve()
    symbol = _text(focus_symbol, "focus_symbol")
    if isinstance(max_hypotheses, bool) or not isinstance(max_hypotheses, int):
        _fail("max_hypotheses must be an integer")
    resolved_report, report_descriptor, report_value = _assessment_file(
        report, root, "causal reducer objdiff report"
    )

    assembly_inputs: dict[str, dict[str, Any] | None] = {
        "target": None,
        "candidate": None,
    }
    assembly_paths: dict[str, Path | None] = {"target": None, "candidate": None}
    assembly_snapshots: dict[str, Mapping[str, Any]] = {}
    for side, value in (("target", target_assembly), ("candidate", candidate_assembly)):
        if value is None:
            continue
        path = _resolve(value, root)
        snapshot = _snapshot(path, f"causal reducer {side} assembly")
        assembly_paths[side] = path
        assembly_snapshots[side] = snapshot
        assembly_inputs[side] = {
            "path": snapshot["path"],
            "size_bytes": snapshot["size_bytes"],
            "sha256": snapshot["sha256"],
        }

    try:
        audit = audit_document(
            report_value,
            target_assembly=assembly_paths["target"],
            candidate_assembly=assembly_paths["candidate"],
            focus_symbol=symbol,
            include_exact_residuals=include_exact_residuals,
            summary_only=summary_only,
            max_hypotheses=max_hypotheses,
        )
    except AuditInputError as exc:
        _fail(f"causal reducer rejected objdiff report ({exc.code}): {exc.message}")
    for side, snapshot in assembly_snapshots.items():
        path = assembly_paths[side]
        assert path is not None
        _recheck_live_snapshot(path, snapshot, f"causal reducer {side} assembly")
    if audit.get("fail_closed") or audit.get("status") != "ok":
        _fail("causal reducer did not produce a closed successful audit")

    tool_path = Path(__file__).with_name("mismatch_cluster_audit.py").resolve()
    tool_snapshot = _snapshot(tool_path, "causal reducer implementation")
    body = {
        "schema": CAUSAL_REDUCER_SCHEMA,
        "schema_version": 1,
        "focus_symbol": symbol,
        "inputs": {
            "report": report_descriptor,
            "target_assembly": assembly_inputs["target"],
            "candidate_assembly": assembly_inputs["candidate"],
        },
        "tool": {
            "path": tool_snapshot["path"],
            "size_bytes": tool_snapshot["size_bytes"],
            "sha256": tool_snapshot["sha256"],
        },
        "audit": audit,
        "limitations": [
            "Structural grouping and recommended axes are diagnostic evidence, not original-source provenance.",
            "Retention requires independent strict/data/physical-relocation/section and sibling gates.",
        ],
        "authority_advanced": False,
    }
    _recheck_live_snapshot(tool_path, tool_snapshot, "causal reducer implementation")
    return _with_self_hash(body, "causal_reducer_sha256")


def decode_pool_ownership(
    root: Path,
    *,
    report: Path | str,
    focus_symbol: str,
    include_exact: bool = False,
    group_limit: int = 24,
    row_limit: int = 12,
) -> dict[str, Any]:
    """Bind and type-decode one function's literal-pool relocations.

    Symbol indices and anonymous labels are object-local.  This command binds
    the report and decoder implementation, then separates literal bit/type or
    relocation-contract differences from value-equivalent owner identity and
    pool chronology.  It is read-only and never authenticates a source label.
    """

    from tools.pool_reloc_summary import PoolDecodeError, decode_function

    root = root.resolve()
    symbol = _text(focus_symbol, "focus_symbol")
    if isinstance(group_limit, bool) or not isinstance(group_limit, int):
        _fail("group_limit must be an integer")
    if isinstance(row_limit, bool) or not isinstance(row_limit, int):
        _fail("row_limit must be an integer")
    _, report_descriptor, report_value = _assessment_file(
        report, root, "typed pool decoder objdiff report"
    )
    try:
        decoded = decode_function(
            report_value,
            symbol,
            include_exact=include_exact,
            group_limit=group_limit,
            row_limit=row_limit,
        )
    except PoolDecodeError as exc:
        _fail(f"typed pool decoder rejected objdiff report: {exc}")
    if decoded.get("schema") != "typed_pool_owner_decoder/v1":
        _fail("typed pool decoder returned an unsupported schema")
    if decoded.get("authority_advanced") is not False:
        _fail("typed pool decoder attempted to advance authority")

    tool_path = Path(__file__).with_name("pool_reloc_summary.py").resolve()
    tool_snapshot = _snapshot(tool_path, "typed pool decoder implementation")
    body = {
        "schema": POOL_DECODER_SCHEMA,
        "schema_version": 1,
        "focus_symbol": symbol,
        "inputs": {"report": report_descriptor},
        "tool": {
            "path": tool_snapshot["path"],
            "size_bytes": tool_snapshot["size_bytes"],
            "sha256": tool_snapshot["sha256"],
        },
        "decode": decoded,
        "limitations": [
            "Literal bits, consumer types, relocations, and object-local owner facts are report-derived; source names are not recovered.",
            "A value-equivalent owner-only mismatch is not permission to add an extern label or reorder unrelated source.",
            "Retention still requires strict/data/physical-relocation/section and protected-sibling proof.",
        ],
        "authority_advanced": False,
    }
    _recheck_live_snapshot(tool_path, tool_snapshot, "typed pool decoder implementation")
    return _with_self_hash(body, "pool_decoder_sha256")


def plan_candidate_interactions(
    root: Path,
    *,
    request: Path | str,
) -> dict[str, Any]:
    """Bind and expand a bounded, evidence-declared factorial candidate plan.

    The standalone planner owns the closed request schema and topology rules;
    this wrapper attests the request/tool bytes and exposes it through the
    central match CLI.  Neither layer generates source or invokes a compiler.
    """

    from tools.candidate_interaction_planner import (
        InteractionPlanError,
        PLAN_SCHEMA,
        build_interaction_plan,
    )

    root = root.resolve()
    request_path = _resolve(request, root)
    request_snapshot = _snapshot(request_path, "candidate interaction request")
    tool_path = Path(__file__).with_name("candidate_interaction_planner.py").resolve()
    tool_snapshot = _snapshot(tool_path, "candidate interaction planner implementation")
    try:
        plan = build_interaction_plan(request_path)
    except InteractionPlanError as exc:
        _fail(f"candidate interaction planner rejected request: {exc}")
    if plan.get("schema") != PLAN_SCHEMA:
        _fail("candidate interaction planner returned an unsupported schema")
    if plan.get("request_sha256") != request_snapshot["sha256"]:
        _fail("candidate interaction planner request binding mismatch")
    if plan.get("authority_advanced") is not False:
        _fail("candidate interaction planner attempted to advance authority")
    if plan.get("production_modified") is not False:
        _fail("candidate interaction planner attempted to modify production")

    body = {
        "schema": INTERACTION_PLANNER_SCHEMA,
        "schema_version": 1,
        "input": request_snapshot,
        "tool": {
            "path": tool_snapshot["path"],
            "size_bytes": tool_snapshot["size_bytes"],
            "sha256": tool_snapshot["sha256"],
        },
        "plan": plan,
        "limitations": [
            "Explicit topology tokens and authenticated hashes are the only deduplication authorities.",
            "A generated cell still requires natural-source review and strict/data/relocation/sibling gates.",
            "The command is read-only and never invokes candidate generation or compilation.",
        ],
        "production_modified": False,
        "authority_advanced": False,
    }
    _recheck_live_snapshot(request_path, request_snapshot, "candidate interaction request")
    _recheck_live_snapshot(tool_path, tool_snapshot, "candidate interaction planner implementation")
    return _with_self_hash(body, "interaction_planner_sha256")


def build_function_telemetry(
    root: Path,
    workspace: Path | str,
    *,
    focus_symbol: str,
    elapsed_seconds: float | None = None,
    active_seconds: float | None = None,
    tracer_runs: int | None = None,
    donor_searches: int | None = None,
) -> dict[str, Any]:
    """Summarize one function's recovery campaign without inventing time.

    Candidate counts, convergence, objects, and heavy-process time come from
    immutable workbench records.  Human wall/active time and activity counts
    are caller-attested and remain explicitly separated from derived facts.
    Missing candidate ``heavy_seconds`` values make heavy-throughput unknown;
    they are never silently treated as zero.
    """

    symbol = _text(focus_symbol, "focus_symbol")
    elapsed = _seconds(elapsed_seconds, "elapsed_seconds")
    active = _seconds(active_seconds, "active_seconds")
    if elapsed is not None and elapsed <= 0:
        _fail("elapsed_seconds must be greater than zero when provided")
    if active is not None and active <= 0:
        _fail("active_seconds must be greater than zero when provided")
    tracer_count = (
        _integer(tracer_runs, "tracer_runs") if tracer_runs is not None else None
    )
    donor_count = (
        _integer(donor_searches, "donor_searches")
        if donor_searches is not None
        else None
    )

    matrix = build_matrix(
        root,
        workspace,
        focus_symbol=[symbol],
        compact=False,
        order="oldest",
    )
    rows = list(matrix.get("rows", []))
    if not rows:
        _fail(f"no candidate history is recorded for focus symbol {symbol}")

    exact_rows: list[Mapping[str, Any]] = []
    target_sizes: set[int] = set()
    heavy_values: list[float] = []
    missing_heavy: list[str] = []
    outcome_counts: dict[str, int] = {}
    for row in rows:
        strict = _matrix_focus_row(row, symbol, "strict_focus")
        data = _matrix_focus_row(row, symbol, "data_focus")
        if strict is None and data is None:
            _fail(f"matrix returned a row without focus evidence for {symbol}")
        if isinstance(strict, Mapping):
            target_size = _assessment_number(strict.get("target_size"))
            if target_size is not None:
                target_sizes.add(target_size)
        if (
            isinstance(strict, Mapping)
            and strict.get("exact") is True
            and isinstance(data, Mapping)
            and data.get("exact") is True
        ):
            exact_rows.append(row)
        heavy = row.get("heavy_seconds")
        if heavy is None:
            missing_heavy.append(str(row.get("candidate_id")))
        else:
            heavy_values.append(float(_seconds(heavy, "matrix row heavy_seconds")))
        outcome = row.get("outcome")
        status = (
            str(outcome.get("status"))
            if isinstance(outcome, Mapping) and outcome.get("status") is not None
            else "unknown"
        )
        outcome_counts[status] = outcome_counts.get(status, 0) + 1

    if len(target_sizes) > 1:
        _fail(f"focus symbol {symbol} has inconsistent target sizes")
    target_size = next(iter(target_sizes), None)
    first_exact = exact_rows[0] if exact_rows else None
    first_exact_index = rows.index(first_exact) if first_exact is not None else None
    crack_count = 1 if first_exact is not None else 0
    heavy_total = round(sum(heavy_values), 6)
    heavy_complete = not missing_heavy

    def rate(seconds: float | None, unit: int = 1) -> float | None:
        if crack_count == 0 or seconds is None or seconds <= 0:
            return None
        return round((crack_count * unit * 3600.0) / seconds, 6)

    elapsed_rate = rate(elapsed)
    active_rate = rate(active)
    heavy_rate = rate(heavy_total) if heavy_complete else None
    exact_bytes = target_size if crack_count and target_size is not None else 0

    if crack_count == 0:
        status = "not_exact"
    elif elapsed is not None and active is not None and heavy_complete:
        status = "exact_with_complete_time_coverage"
    else:
        status = "exact_with_partial_time_coverage"

    body = {
        "schema": FUNCTION_TELEMETRY_SCHEMA,
        "schema_version": 1,
        "session_id": matrix["session_id"],
        "focus_symbol": symbol,
        "status": status,
        "campaign": {
            "candidate_count": len(rows),
            "unique_source_count": len({str(row.get("source_sha256")) for row in rows}),
            "unique_object_count": len({str(row.get("object_sha256")) for row in rows}),
            "duplicate_candidate_count": sum(bool(row.get("duplicate_of")) for row in rows),
            "outcome_counts": dict(sorted(outcome_counts.items())),
            "strict_data_exact_candidate_count": len(exact_rows),
            "first_exact_candidate_id": (
                str(first_exact.get("candidate_id")) if first_exact is not None else None
            ),
            "first_exact_ordinal": (
                int(first_exact.get("ordinal")) if first_exact is not None else None
            ),
            "candidates_through_first_exact": (
                first_exact_index + 1 if first_exact_index is not None else None
            ),
            "nonexact_candidates_before_first_exact": first_exact_index,
            "target_size_bytes": target_size,
            "exact_focus_bytes": exact_bytes,
        },
        "time": {
            "heavy_seconds": heavy_total,
            "heavy_seconds_known_candidate_count": len(heavy_values),
            "heavy_seconds_missing_candidate_ids": missing_heavy,
            "heavy_seconds_complete": heavy_complete,
            "elapsed_seconds": elapsed,
            "active_seconds": active,
            "elapsed_active_source": "caller_attested" if elapsed is not None or active is not None else None,
        },
        "activity": {
            "tracer_runs": tracer_count,
            "donor_searches": donor_count,
            "source": "caller_attested" if tracer_count is not None or donor_count is not None else None,
        },
        "throughput": {
            "exact_functions": crack_count,
            "exact_functions_per_elapsed_hour": elapsed_rate,
            "exact_functions_per_active_hour": active_rate,
            "exact_functions_per_heavy_process_hour": heavy_rate,
            "exact_bytes_per_elapsed_hour": (
                round(exact_bytes * elapsed_rate, 6) if elapsed_rate is not None else None
            ),
            "exact_bytes_per_active_hour": (
                round(exact_bytes * active_rate, 6) if active_rate is not None else None
            ),
            "exact_bytes_per_heavy_process_hour": (
                round(exact_bytes * heavy_rate, 6) if heavy_rate is not None else None
            ),
        },
        "coverage": {
            "candidate_history": "complete_indexed_workbench_history",
            "heavy_process_time": "complete" if heavy_complete else "partial",
            "elapsed_time": "caller_attested" if elapsed is not None else "missing",
            "active_time": "caller_attested" if active is not None else "missing",
            "physical_relocations": "not_authenticated_by_candidate_telemetry",
            "consumer_closure": "not_authenticated_by_candidate_telemetry",
        },
        "matrix_sha256": matrix["matrix_sha256"],
        "authority_advanced": False,
    }
    return _with_self_hash(body, "telemetry_sha256")


def _print(value: Mapping[str, Any], *, as_json: bool) -> None:
    if as_json or value.get("schema") in {
        ASSESSMENT_SCHEMA,
        RESIDUALS_SCHEMA,
        STACK_RESIDUE_SCHEMA,
        FUNCTION_TELEMETRY_SCHEMA,
        CAUSAL_REDUCER_SCHEMA,
        POOL_DECODER_SCHEMA,
        INTERACTION_PLANNER_SCHEMA,
        DONOR_SHAPES_SCHEMA,
        DONOR_REGISTRY_SCHEMA,
        DONOR_REGISTRY_LIST_SCHEMA,
        DONOR_REJECTION_SCHEMA,
    }:
        print(json.dumps(value, indent=2, ensure_ascii=False, sort_keys=True))
        return
    if value.get("schema") == PREPARATION_SCHEMA:
        focus = value.get("focus_symbol", value.get("focus_symbols"))
        print(
            f"prepare status={value.get('status')} "
            f"focus={focus} "
            f"record_request={'ready' if value.get('record_request') else 'withheld'}"
        )
        return
    if value.get("schema") == "match_workbench_diagnostic_batch/v1":
        summary = value.get("summary", {})
        print(
            f"diagnose candidate={value.get('candidate_id')} jobs={len(value.get('jobs', []))} "
            f"ran={summary.get('ran', 0)} cached={summary.get('cached', 0)} failed={summary.get('failed', 0)}"
        )
        return
    if value.get("schema") == MATRIX_SCHEMA:
        aggregate = value.get("aggregate", {})
        if value.get("view") == "compact":
            query = value.get("query", {})
            print(
                f"matrix-focus session={value.get('session_id')} "
                f"focus={','.join(query.get('focus_symbols') or ['*'])} "
                f"order={query.get('order', 'oldest')} "
                f"rows={aggregate.get('selected_candidate_count', 0)}/"
                f"{aggregate.get('total_candidate_count', 0)}"
            )
            for row in value.get("rows", []):
                strict = row.get("strict") or {}
                data = row.get("data") or {}
                print(
                    f"{row.get('candidate_id')} focus={row.get('focus_symbol')} "
                    f"strict={strict.get('match_percent', '-')}%/"
                    f"{strict.get('diff_rows', '-')}diff "
                    f"data={data.get('match_percent', '-')}%/"
                    f"{data.get('diff_rows', '-')}diff "
                    f"size={strict.get('target_size', '-')}/"
                    f"{strict.get('candidate_size', '-')} "
                    f"outcome={row.get('outcome', {}).get('status', '-')} "
                    f"axis={row.get('axis', '-') }"
                )
            return
        if value.get("view") == "filtered":
            query = value.get("query", {})
            print(
                f"matrix-filter session={value.get('session_id')} "
                f"focus={','.join(query.get('focus_symbols') or ['*'])} "
                f"order={query.get('order', 'oldest')} "
                f"rows={query.get('selected_candidate_count', 0)}/"
                f"{query.get('total_candidate_count', 0)}"
            )
            for row in value.get("rows", []):
                focus = row.get("strict_focus") or row.get("data_focus") or {}
                print(
                    f"{row.get('candidate_id')} focus={row.get('focus_symbol')} "
                    f"match={focus.get('match_percent', '-')}%/"
                    f"{focus.get('diff_rows', '-')}diff "
                    f"axis={row.get('hypothesis_axis', '-')}"
                )
            return
        print(
            f"matrix session={value.get('session_id')} candidates={aggregate.get('candidate_count', 0)} "
            f"duplicates={aggregate.get('duplicate_candidate_count', 0)} "
            f"read_only_diagnosed={aggregate.get('diagnosed_candidate_count', 0)} "
            f"next={(value.get('rows') or [{}])[0].get('next_action', 'none')}"
        )
        return
    if "skip_compile" in value:
        print(
            f"lookup status={value.get('status')} source={str(value.get('source_sha256', ''))[:12]} "
            f"object={str(value.get('object_sha256') or '')[:12]} "
            f"skip_compile={value.get('skip_compile')} skip_diagnostics={value.get('skip_diagnostics')}"
        )
        return
    if "record" in value:
        record = value["record"]
        print(
            f"{value.get('status')} candidate={record.get('candidate_id')} "
            f"object={str(record.get('object', {}).get('sha256', ''))[:12]}"
        )
        return
    if value.get("session"):
        session = value["session"]
        print(f"{value.get('status')} workspace={value.get('workspace')} session={session.get('session_id')}")
        return
    print(value.get("status") or value.get("schema") or "ok")


def _add_commands(commands: Any) -> None:
    init = commands.add_parser("init", help="freeze an authenticated matching session")
    init.add_argument("manifest")
    init.add_argument("--workspace", required=True)
    init.add_argument("--json", action="store_true")

    repair_target_parser = commands.add_parser(
        "repair-target", help="restore a missing or mutated target from session CAS"
    )
    repair_target_parser.add_argument("--workspace", required=True)
    repair_target_parser.add_argument("--json", action="store_true")

    lookup = commands.add_parser("lookup", help="reject a duplicate source/context before compile")
    lookup.add_argument("--workspace", required=True)
    lookup.add_argument("--source")
    lookup.add_argument("--object")
    lookup.add_argument("--json", action="store_true")

    materialize = commands.add_parser(
        "materialize",
        help="copy an authenticated candidate object CAS to an explicit output path",
    )
    materialize.add_argument("--workspace", required=True)
    materialize.add_argument("--candidate-id", required=True)
    materialize.add_argument("--source", required=True)
    materialize.add_argument("--object", required=True)
    materialize.add_argument("--json", action="store_true")

    attest_compile = commands.add_parser(
        "attest-compile",
        help="seal source/object bytes with the actual compiler, wrapper, argv, and cwd context",
    )
    attest_compile.add_argument("--workspace", required=True)
    attest_compile.add_argument("--source", required=True)
    attest_compile.add_argument("--object", required=True)
    attest_compile.add_argument("--output", required=True)
    attest_compile.add_argument(
        "--producer-kind",
        choices=("serialized-build", "external-compile-attestation"),
        required=True,
    )
    producer_argv = attest_compile.add_mutually_exclusive_group(required=True)
    producer_argv.add_argument(
        "--producer-arg",
        action="append",
        help="one actual producer command argument; repeat in exact order",
    )
    producer_argv.add_argument(
        "--producer-argv-from-session",
        action="store_true",
        help="copy the immutable session compile_argv into the attestation",
    )
    attest_compile.add_argument("--notes")
    attest_compile.add_argument("--json", action="store_true")

    provenance_audit = commands.add_parser(
        "provenance-audit",
        help="classify candidate records as context-matched, cross-context, or unattested",
    )
    provenance_audit.add_argument("--workspace", required=True)
    provenance_audit.add_argument("--manifest")
    provenance_audit.add_argument(
        "--output", help="optional immutable self-hashed audit receipt"
    )
    provenance_audit.add_argument("--json", action="store_true")

    provenance_migrate = commands.add_parser(
        "provenance-migrate",
        help="import only compiler-context-matched candidate history into a clean session",
    )
    provenance_migrate.add_argument("--source-workspace", required=True)
    provenance_migrate.add_argument("--destination-workspace", required=True)
    provenance_migrate.add_argument("--manifest", required=True)
    provenance_migrate.add_argument(
        "--output", help="optional immutable self-hashed migration receipt"
    )
    provenance_migrate.add_argument("--json", action="store_true")

    record = commands.add_parser("record", help="record and content-address one measured candidate")
    record.add_argument("--workspace", required=True)
    record.add_argument("--candidate-id", required=True)
    record.add_argument("--source", required=True)
    record.add_argument("--object", required=True)
    record.add_argument("--compile-attestation", required=True)
    record.add_argument("--strict-report", required=True)
    record.add_argument("--data-report")
    record.add_argument("--hypothesis", required=True)
    record.add_argument("--axis", required=True)
    record.add_argument("--residual")
    record.add_argument("--status", default="measured")
    record.add_argument("--reason", default="candidate measured")
    record.add_argument("--heavy-seconds", type=float)
    record.add_argument(
        "--focus-symbol",
        action="append",
        help="focus symbol to retain; repeat for coupled targets",
    )
    record.add_argument("--json", action="store_true")

    diagnose = commands.add_parser("diagnose", help="run bounded authenticated read-only diagnostics")
    diagnose.add_argument("--workspace", required=True)
    diagnose.add_argument("--candidate-id", required=True)
    diagnose.add_argument("--jobs", required=True)
    diagnose.add_argument("--max-workers", type=int)
    diagnose.add_argument("--json", action="store_true")

    matrix = commands.add_parser("matrix", help="render the deterministic candidate matrix")
    matrix.add_argument("--workspace", required=True)
    matrix.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        dest="focus_symbol",
        action="append",
        help="filter candidate history to one focus symbol; repeat for coupled symbols",
    )
    matrix.add_argument(
        "--limit",
        type=int,
        help="return at most this many candidates after deterministic ordering",
    )
    matrix_order = matrix.add_mutually_exclusive_group()
    matrix_order.add_argument(
        "--latest",
        action="store_true",
        help="select newest candidates first (equivalent to --order newest)",
    )
    matrix_order.add_argument(
        "--order",
        choices=("oldest", "newest"),
        default="oldest",
        help="candidate ordering before applying --limit (default: oldest)",
    )
    matrix.add_argument(
        "--compact",
        action="store_true",
        help="emit bounded focus-history rows instead of report bindings",
    )
    matrix.add_argument(
        "--compact-json",
        action="store_true",
        help="emit the bounded focus-history view as JSON",
    )
    matrix.add_argument("--json", action="store_true")

    telemetry = commands.add_parser(
        "telemetry",
        help="summarize one function's recovery attempts and crack/hour coverage",
    )
    telemetry.add_argument("--workspace", required=True)
    telemetry.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        dest="focus_symbol",
        required=True,
    )
    telemetry.add_argument("--elapsed-seconds", type=float)
    telemetry.add_argument("--active-seconds", type=float)
    telemetry.add_argument("--tracer-runs", type=int)
    telemetry.add_argument("--donor-searches", type=int)
    telemetry.add_argument("--json", action="store_true")

    cascade = commands.add_parser(
        "cascade",
        aliases=("causal-reduce",),
        help="reduce one function's objdiff rows into bounded causal groups",
    )
    cascade.add_argument("--report", "--objdiff-report", dest="report", required=True)
    cascade.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        dest="focus_symbol",
        required=True,
    )
    cascade.add_argument("--target-asm")
    cascade.add_argument("--candidate-asm")
    cascade.add_argument("--max-hypotheses", type=int, default=20)
    cascade.add_argument(
        "--full",
        action="store_true",
        help="include bounded instruction-pair evidence instead of compact summary output",
    )
    cascade.add_argument("--include-exact-residuals", action="store_true")
    cascade.add_argument("--json", action="store_true")

    pools = commands.add_parser(
        "pools",
        aliases=("pool-decode", "pool-owners"),
        help="decode typed literal-pool value, relocation, and owner mismatches",
    )
    pools.add_argument("--report", "--objdiff-report", dest="report", required=True)
    pools.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        dest="focus_symbol",
        required=True,
    )
    pools.add_argument("--include-exact", action="store_true")
    pools.add_argument("--group-limit", type=int, default=24)
    pools.add_argument("--row-limit", type=int, default=12)
    pools.add_argument("--json", action="store_true")

    interactions = commands.add_parser(
        "interactions",
        aliases=("factorial-plan", "interaction-plan"),
        help="expand evidence-backed source axes into a deduplicated factorial batch",
    )
    interactions.add_argument("--request", required=True)
    interactions.add_argument("--json", action="store_true")

    assess = commands.add_parser(
        "assess",
        help="compare baseline/candidate strict and data objdiff reports",
    )
    assess.add_argument(
        "--baseline-strict",
        "--strict-baseline",
        "--baseline-strict-report",
        "--baseline",
        dest="baseline_strict",
        required=True,
    )
    assess.add_argument(
        "--candidate-strict",
        "--strict-candidate",
        "--candidate-strict-report",
        "--candidate",
        dest="candidate_strict",
        required=True,
    )
    assess.add_argument(
        "--baseline-data",
        "--data-baseline",
        "--baseline-data-report",
        dest="baseline_data",
    )
    assess.add_argument(
        "--candidate-data",
        "--data-candidate",
        "--candidate-data-report",
        dest="candidate_data",
    )
    assess.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        "--focus",
        dest="focus_symbol",
        action="append",
        required=True,
    )
    # Assessment is intentionally JSON-only.  Accept the common workbench
    # switch so scripts can share argument builders with the other commands.
    assess.add_argument("--json", action="store_true", help=argparse.SUPPRESS)

    residuals = commands.add_parser(
        "residuals",
        help="rank nonexact functions from paired strict/data objdiff reports",
    )
    residuals.add_argument(
        "--strict-report",
        "--strict",
        dest="strict_report",
        required=True,
    )
    residuals.add_argument(
        "--data-report",
        "--data",
        dest="data_report",
        required=True,
    )
    residuals.add_argument(
        "--exclude-symbol",
        "--exclude-function",
        "--exclude-known-exact",
        dest="exclude_symbol",
        action="append",
        default=[],
        help="known-exact function to omit; repeat for multiple symbols",
    )
    residuals.add_argument("--json", action="store_true")

    stack_residue = commands.add_parser(
        "stack-residue",
        aliases=("stack-slots",),
        help="find target stack slots with writes and zero reads",
    )
    stack_residue.add_argument(
        "--report",
        "--objdiff-report",
        dest="report",
        required=True,
    )
    stack_residue.add_argument(
        "--focus-symbol",
        "--symbol",
        "--focus",
        dest="focus_symbol",
        action="append",
        required=True,
    )
    stack_residue.add_argument("--json", action="store_true")

    donor_shapes_parser = commands.add_parser(
        "donor-shapes",
        help="mine deterministic donor C source shapes without compiling or editing",
    )
    donor_shapes_parser.add_argument("--source", required=True)
    donor_shapes_parser.add_argument(
        "--focus-symbol",
        "--function",
        "--focus",
        dest="focus_symbol",
        action="append",
        required=True,
    )
    donor_shapes_parser.add_argument(
        "--donor-file",
        action="append",
        default=[],
        help="explicit donor C file; repeat for multiple files",
    )
    donor_shapes_parser.add_argument(
        "--search-root",
        action="append",
        default=[],
        help="explicit recursive donor search root; repeat for multiple roots",
    )
    donor_shapes_parser.add_argument(
        "--max-files",
        type=int,
        default=_DONOR_MAX_FILES,
        help="maximum deduplicated source files scanned across explicit roots",
    )
    donor_shapes_parser.add_argument("--json", action="store_true")

    donor_register = commands.add_parser(
        "donor-register",
        help="register one authenticated source shape in the durable donor registry",
    )
    donor_register.add_argument("--registry", required=True)
    donor_register.add_argument("--source", required=True)
    donor_register.add_argument(
        "--focus-symbol", "--function", dest="focus_symbol", required=True
    )
    donor_register.add_argument(
        "--source-kind",
        required=True,
        choices=tuple(sorted(DONOR_SOURCE_KINDS)),
    )
    donor_register.add_argument("--donor-id")
    donor_register.add_argument("--alias", action="append", default=[])
    donor_register.add_argument("--status", choices=tuple(sorted(DONOR_STATUSES)), default="accepted")
    donor_register.add_argument(
        "--admissibility",
        choices=tuple(sorted(DONOR_ADMISSIBILITY)),
        default="admissible",
    )
    donor_register.add_argument("--evidence", action="append", default=[])
    donor_register.add_argument("--supersedes", action="append", default=[])
    donor_register.add_argument("--duplicate-of")
    donor_register.add_argument("--queried-by", action="append", default=[])
    donor_register.add_argument("--used-by", action="append", default=[])
    donor_register.add_argument("--notes")
    donor_register.add_argument("--json", action="store_true")

    donor_list = commands.add_parser(
        "donor-list", help="list durable donor/source-shape registry entries"
    )
    donor_list.add_argument("--registry", required=True)
    donor_list.add_argument("--source-kind", choices=tuple(sorted(DONOR_SOURCE_KINDS)))
    donor_list.add_argument("--status", choices=tuple(sorted(DONOR_STATUSES)))
    donor_list.add_argument("--focus-symbol", "--function", dest="focus_symbol")
    donor_list.add_argument("--include-rejections", action="store_true")
    donor_list.add_argument("--json", action="store_true")

    donor_lookup = commands.add_parser(
        "donor-lookup", help="look up durable donor/source-shape registry entries"
    )
    donor_lookup.add_argument("--registry", required=True)
    donor_lookup.add_argument("--donor-id", "--alias", dest="donor_id")
    donor_lookup.add_argument("--source-sha256")
    donor_lookup.add_argument("--focus-symbol", "--function", dest="focus_symbol")
    donor_lookup.add_argument("--candidate-id")
    donor_lookup.add_argument("--json", action="store_true")

    donor_reject = commands.add_parser(
        "donor-reject",
        help="reject a donor record or write a target-object rejection receipt",
    )
    donor_reject.add_argument("--registry", required=True)
    donor_reject_group = donor_reject.add_mutually_exclusive_group(required=True)
    donor_reject_group.add_argument("--donor-id", "--alias", dest="donor_id")
    donor_reject_group.add_argument("--source")
    donor_reject.add_argument("--focus-symbol", "--function", dest="focus_symbol")
    donor_reject.add_argument("--reason", required=True)
    donor_reject.add_argument("--evidence", action="append", default=[])
    donor_reject.add_argument("--json", action="store_true")

    prepare = commands.add_parser(
        "prepare",
        help="assess reports and compose a guarded record request without mutation",
    )
    prepare.add_argument(
        "--baseline-strict",
        "--strict-baseline",
        "--baseline-strict-report",
        "--baseline",
        dest="baseline_strict",
        required=True,
    )
    prepare.add_argument(
        "--candidate-strict",
        "--strict-candidate",
        "--candidate-strict-report",
        "--candidate",
        dest="candidate_strict",
        required=True,
    )
    prepare.add_argument(
        "--baseline-data",
        "--data-baseline",
        "--baseline-data-report",
        dest="baseline_data",
    )
    prepare.add_argument(
        "--candidate-data",
        "--data-candidate",
        "--candidate-data-report",
        dest="candidate_data",
    )
    prepare.add_argument(
        "--focus-symbol",
        "--symbol",
        "--function",
        "--focus",
        dest="focus_symbol",
        action="append",
        required=True,
    )
    prepare.add_argument("--workspace", required=True)
    prepare.add_argument("--candidate-id", required=True)
    prepare.add_argument("--source", required=True)
    prepare.add_argument("--object", required=True)
    prepare.add_argument("--compile-attestation", required=True)
    prepare.add_argument("--hypothesis", required=True)
    prepare.add_argument("--axis", required=True)
    prepare.add_argument("--residual")
    prepare.add_argument("--status", default="measured")
    prepare.add_argument("--reason", default="candidate measured")
    prepare.add_argument("--heavy-seconds", type=float)
    prepare.add_argument("--json", action="store_true")


def add_match_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser("match", help="deduplicate candidates and parallelize read-only diagnosis")
    commands = parser.add_subparsers(dest="match_command", required=True)
    _add_commands(commands)
    return parser


def run_match_command(args: argparse.Namespace, *, root: Path) -> int:
    if args.match_command == "init":
        result = init_workspace(root, args.manifest, args.workspace)
    elif args.match_command == "repair-target":
        result = repair_target(root, args.workspace)
    elif args.match_command == "lookup":
        result = lookup_matches(root, args.workspace, args.source, args.object)
    elif args.match_command == "materialize":
        result = materialize_candidate_object(
            root,
            args.workspace,
            args.candidate_id,
            args.source,
            args.object,
        )
    elif args.match_command == "attest-compile":
        result = create_compile_attestation(
            root,
            args.workspace,
            source=args.source,
            object_path=args.object,
            output=args.output,
            producer_kind=args.producer_kind,
            producer_command=(
                None if args.producer_argv_from_session else args.producer_arg
            ),
            notes=args.notes,
        )
    elif args.match_command == "provenance-audit":
        result = audit_candidate_provenance(
            root,
            args.workspace,
            manifest=args.manifest,
        )
        _persist_provenance_result(
            root, args.output, result, label="provenance audit output"
        )
    elif args.match_command == "provenance-migrate":
        result = migrate_candidate_provenance(
            root,
            args.source_workspace,
            args.destination_workspace,
            manifest=args.manifest,
        )
        _persist_provenance_result(
            root, args.output, result, label="provenance migration output"
        )
    elif args.match_command == "record":
        result = record_candidate(
            root,
            args.workspace,
            candidate_id=args.candidate_id,
            source=args.source,
            object_path=args.object,
            compile_attestation=args.compile_attestation,
            strict_report=args.strict_report,
            data_report=args.data_report,
            hypothesis=args.hypothesis,
            axis=args.axis,
            residual=args.residual,
            status=args.status,
            reason=args.reason,
            heavy_seconds=args.heavy_seconds,
            focus_symbol=args.focus_symbol,
        )
    elif args.match_command == "diagnose":
        result = diagnose_candidate(
            root, args.workspace, args.candidate_id, args.jobs, max_workers=args.max_workers
        )
    elif args.match_command == "telemetry":
        result = build_function_telemetry(
            root,
            args.workspace,
            focus_symbol=args.focus_symbol,
            elapsed_seconds=args.elapsed_seconds,
            active_seconds=args.active_seconds,
            tracer_runs=args.tracer_runs,
            donor_searches=args.donor_searches,
        )
    elif args.match_command in {"cascade", "causal-reduce"}:
        result = reduce_objdiff_cascades(
            root,
            report=args.report,
            focus_symbol=args.focus_symbol,
            target_assembly=args.target_asm,
            candidate_assembly=args.candidate_asm,
            summary_only=not args.full,
            max_hypotheses=args.max_hypotheses,
            include_exact_residuals=args.include_exact_residuals,
        )
    elif args.match_command in {"pools", "pool-decode", "pool-owners"}:
        result = decode_pool_ownership(
            root,
            report=args.report,
            focus_symbol=args.focus_symbol,
            include_exact=args.include_exact,
            group_limit=args.group_limit,
            row_limit=args.row_limit,
        )
    elif args.match_command in {"interactions", "factorial-plan", "interaction-plan"}:
        result = plan_candidate_interactions(
            root,
            request=args.request,
        )
    elif args.match_command == "assess":
        result = assess_reports(
            root,
            baseline_strict=args.baseline_strict,
            candidate_strict=args.candidate_strict,
            baseline_data=args.baseline_data,
            candidate_data=args.candidate_data,
            focus_symbol=args.focus_symbol,
        )
    elif args.match_command == "residuals":
        result = rank_residuals(
            root,
            strict_report=args.strict_report,
            data_report=args.data_report,
            exclude_symbols=args.exclude_symbol,
        )
    elif args.match_command in {"stack-residue", "stack-slots"}:
        result = inspect_stack_residue(
            root,
            report=args.report,
            focus_symbol=args.focus_symbol,
        )
    elif args.match_command == "donor-shapes":
        result = donor_shapes(
            root,
            source=args.source,
            focus_symbol=args.focus_symbol,
            donor_files=args.donor_file,
            search_roots=args.search_root,
            max_files=args.max_files,
        )
    elif args.match_command == "donor-register":
        result = register_donor_shape(
            root,
            args.registry,
            source=args.source,
            focus_symbol=args.focus_symbol,
            source_kind=args.source_kind,
            donor_id=args.donor_id,
            aliases=args.alias,
            status=args.status,
            admissibility=args.admissibility,
            evidence_paths=args.evidence,
            supersedes=args.supersedes,
            duplicate_of=args.duplicate_of,
            queried_by_candidate_ids=args.queried_by,
            used_by_candidate_ids=args.used_by,
            notes=args.notes,
        )
    elif args.match_command == "donor-list":
        result = list_donor_shapes(
            root,
            args.registry,
            source_kind=args.source_kind,
            status=args.status,
            focus_symbol=args.focus_symbol,
            include_rejections=args.include_rejections,
        )
    elif args.match_command == "donor-lookup":
        result = lookup_donor_shapes(
            root,
            args.registry,
            donor_id=args.donor_id,
            source_sha256=args.source_sha256,
            focus_symbol=args.focus_symbol,
            candidate_id=args.candidate_id,
        )
    elif args.match_command == "donor-reject":
        result = reject_donor_shape(
            root,
            args.registry,
            donor_id=args.donor_id,
            source=args.source,
            focus_symbol=args.focus_symbol,
            reason=args.reason,
            evidence_paths=args.evidence,
        )
    elif args.match_command == "prepare":
        result = prepare_candidate_record(
            root,
            baseline_strict=args.baseline_strict,
            candidate_strict=args.candidate_strict,
            baseline_data=args.baseline_data,
            candidate_data=args.candidate_data,
            focus_symbol=args.focus_symbol,
            workspace=args.workspace,
            candidate_id=args.candidate_id,
            source=args.source,
            object_path=args.object,
            compile_attestation=args.compile_attestation,
            hypothesis=args.hypothesis,
            axis=args.axis,
            residual=args.residual,
            status=args.status,
            reason=args.reason,
            heavy_seconds=args.heavy_seconds,
        )
    else:
        result = build_matrix(
            root,
            args.workspace,
            focus_symbol=getattr(args, "focus_symbol", None),
            limit=getattr(args, "limit", None),
            compact=getattr(args, "compact", False) or getattr(args, "compact_json", False),
            order="newest" if getattr(args, "latest", False) else getattr(args, "order", "oldest"),
        )
    _print(result, as_json=args.json or getattr(args, "compact_json", False))
    if args.match_command in {"assess", "prepare"} and (
        result.get("verdict") == "rejected"
        or result.get("status") == "rejected"
    ):
        return 1
    return 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    commands = parser.add_subparsers(dest="match_command", required=True)
    _add_commands(commands)
    args = parser.parse_args(argv)
    try:
        return run_match_command(args, root=Path(args.root).expanduser().resolve())
    except (MatchError, OSError, subprocess.SubprocessError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
