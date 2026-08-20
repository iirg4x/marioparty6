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
import gzip
import hashlib
import json
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
INDEX_SCHEMA = "match_workbench_index/v1"
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

    def __init__(self, target: Path) -> None:
        self.target = Path(os.path.abspath(target))
        self.parent = self.target.parent
        self.fd: int | None = None
        self._handles: list[Any] = []
        self._kernel32: Any = None
        self._parent_identity: tuple[int, int, int] | None = None

    def __enter__(self) -> "_PinnedTargetParent":
        try:
            _safe_parent(self.target)
            _assert_no_indirection(self.parent)
            if os.name == "nt":
                self._open_windows()
            else:
                self._open_posix()
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
        self._parent_identity = (info.st_dev, info.st_ino, info.st_nlink)

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
        self._parent_identity = (info.st_dev, info.st_ino, info.st_nlink)

    def verify(self) -> None:
        """Verify that the named parent still denotes the pinned directory."""
        _assert_no_indirection(self.parent)
        try:
            info = self.parent.lstat()
        except OSError as exc:
            raise MatchError(f"session target parent changed during repair: {self.parent}") from exc
        if not stat.S_ISDIR(info.st_mode):
            _fail(f"session target parent is not a directory: {self.parent}")
        identity = (info.st_dev, info.st_ino, info.st_nlink)
        if self._parent_identity is not None and identity != self._parent_identity:
            _fail(f"session target parent changed during repair: {self.parent}")
        if self.fd is not None:
            current = os.fstat(self.fd)
            if (current.st_dev, current.st_ino, current.st_nlink) != identity:
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
        "identity": {
            "device": before.st_dev,
            "inode": before.st_ino,
            "nlink": before.st_nlink,
            "mtime_ns": before.st_mtime_ns,
        },
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


def _descriptor(value: Any, *, base: Path, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256"},
        required={"path", "size_bytes", "sha256"},
        label=label,
    )
    path = _resolve(_text(item["path"], f"{label}.path"), base)
    expected_size = _integer(item["size_bytes"], f"{label}.size_bytes")
    expected_sha = _sha256(item["sha256"], f"{label}.sha256")
    actual = _snapshot(path, label)
    if actual["size_bytes"] != expected_size or actual["sha256"] != expected_sha:
        _fail(f"descriptor mismatch for {label}: {path}")
    return {key: actual[key] for key in ("path", "size_bytes", "sha256")}


def descriptor(path: Path | str) -> dict[str, Any]:
    """Return an authenticated descriptor for a regular single-link file."""
    actual = _snapshot(Path(path).expanduser(), "artifact")
    return {key: actual[key] for key in ("path", "size_bytes", "sha256")}


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
        allowed={"path", "size_bytes", "sha256"},
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
    return {"path": os.fspath(path), "size_bytes": size, "sha256": sha}


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
    target: Path, cas_path: Path, expected: Mapping[str, Any]
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
    with _PinnedTargetParent(target) as parent:
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
    current = _snapshot(Path(str(value["path"])), label)
    if current["size_bytes"] != value["size_bytes"] or current["sha256"] != value["sha256"]:
        _fail(f"{label} changed from its authenticated descriptor")


def _load_session(
    workspace: Path, root: Path, *, skip_live_target_check: bool = False
) -> Mapping[str, Any]:
    session = _load_json(workspace / "session.json", "session")
    _verify_self_hash(session, "session_sha256", "session")
    _closed(
        session,
        allowed={
            "schema", "schema_version", "session_id", "root", "workspace", "request",
            "request_manifest", "target_blob", "authority_advanced", "session_sha256",
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
    _closed(target, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label="session target")
    _text(target.get("path"), "session target.path")
    _integer(target.get("size_bytes"), "session target.size_bytes")
    _sha256(target.get("sha256"), "session target.sha256")
    if skip_live_target_check:
        _validate_target_path(
            _resolve(str(target["path"]), root),
            allow_missing_leaf=True,
            label="session target",
        )
    else:
        _recheck_descriptor(target, "session target")
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
        allowed={"base_commit", "toolchain_key", "compiler", "compile_argv", "compile_inputs", "context_complete"},
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
        _closed(compiler, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label="session compiler")
        _recheck_descriptor(compiler, "session compiler")
    elif compiler is not None:
        _fail("session compiler must be a descriptor or null")
    compile_inputs = context.get("compile_inputs", []) if isinstance(context, Mapping) else []
    if not isinstance(compile_inputs, list):
        _fail("session compile_inputs is invalid")
    for index, item in enumerate(compile_inputs):
        if not isinstance(item, Mapping):
            _fail(f"session compile input {index} is invalid")
        _closed(item, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label=f"session compile input {index}")
        _recheck_descriptor(item, f"session compile input {index}")
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
        allowed={"base_commit", "toolchain_key", "compiler", "compile_argv", "compile_inputs", "context_complete"},
        required={"base_commit", "toolchain_key", "compiler", "compile_argv"},
        label="request.context",
    )
    compiler = None
    if context["compiler"] is not None:
        compiler = _descriptor(context["compiler"], base=root, label="request.context.compiler")
    argv = context["compile_argv"]
    if not isinstance(argv, list) or not all(isinstance(arg, str) and "\x00" not in arg for arg in argv):
        _fail("request.context.compile_argv must be a string array")
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
    if claimed_complete and (compiler is not None or argv) and not compile_inputs:
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
    return {
        "schema": REQUEST_SCHEMA,
        "schema_version": 1,
        "session_id": _identifier(item["session_id"], "request.session_id"),
        "owner": _text(item["owner"], "request.owner"),
        "unit": _text(item["unit"], "request.unit"),
        "function": _text(item["function"], "request.function"),
        "target": target,
        "context": {
            "base_commit": _text(context["base_commit"], "request.context.base_commit"),
            "toolchain_key": _text(context["toolchain_key"], "request.context.toolchain_key"),
            "compiler": compiler,
            "compile_argv": list(argv),
            "compile_inputs": compile_inputs,
            "context_complete": context_complete,
        },
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

        restored = _restore_target_from_cas(target_path, target_cas, target)
        return {
            "status": "restored",
            "workspace": os.fspath(destination),
            "session_id": session["session_id"],
            "target": restored,
            "authority_advanced": False,
        }


def _context_key(session: Mapping[str, Any], source_sha: str) -> str:
    value = {
        "session_sha256": session["session_sha256"],
        "source_sha256": source_sha,
        "target_sha256": session["request"]["target"]["sha256"],
        "context": session["request"]["context"],
    }
    return _sha256_bytes(_canonical(value))


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
    if source is not None:
        source_snapshot = _snapshot(_resolve(source, root), "candidate source")
        source_key = _context_key(session, source_snapshot["sha256"])
    source_match = index["source_context_index"].get(source_key) if source_key else None
    object_match = None
    object_snapshot = None
    if object_path is not None:
        object_snapshot = _snapshot(_resolve(object_path, root), "candidate object")
        object_match = index["object_index"].get(_object_key(session, object_snapshot["sha256"]))
    loaded_matches: dict[str, Mapping[str, Any]] = {}
    for matched_id in {item for item in (source_match, object_match) if item is not None}:
        loaded_matches[matched_id] = _load_candidate(destination, matched_id, session)
    if source_match and loaded_matches[source_match].get("source_context_key") != source_key:
        _fail("source/context index does not match its immutable candidate record")
    if object_match and loaded_matches[object_match].get("object_result_key") != _object_key(
        session, str(object_snapshot["sha256"])
    ):
        _fail("object index does not match its immutable candidate record")
    conflict = bool(
        source_match
        and object_snapshot is not None
        and session["request"]["context"].get("context_complete", False)
        and object_match != source_match
    )
    status = "new"
    if conflict:
        status = "conflict"
    elif source_match:
        status = "known_source"
    elif object_match:
        status = "known_object"
    return {
        "status": status,
        "source_sha256": source_snapshot["sha256"] if source_snapshot else None,
        "object_sha256": object_snapshot["sha256"] if object_snapshot else None,
        "source_candidate_id": source_match,
        "object_candidate_id": object_match,
        "skip_compile": bool(source_match) and bool(session["request"]["context"].get("context_complete", False)) and not conflict,
        # A known object alone does not prove that the requested diagnostic
        # manifest has run.  Diagnose remains cheap because exact fingerprints
        # are content-addressed and return cached results.
        "skip_diagnostics": False,
        "diagnostic_reuse_candidate_id": object_match,
        "reason": (
            "the same frozen source/context produced a different object; compiler inputs must be re-authenticated"
            if conflict
            else "source/context is known but the frozen compile context is incomplete"
            if source_match and not session["request"]["context"].get("context_complete", False)
            else "object bytes are known; run diagnose to reuse matching fingerprints"
            if object_match
            else None
        ),
        "authority_advanced": False,
    }


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
        with tempfile.NamedTemporaryFile(
            "wb", dir=output.parent, prefix=f".{sha}.", suffix=".tmp", delete=False
        ) as target:
            temporary = Path(target.name)
            with source.open("rb") as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b""):
                    target.write(block)
            target.flush()
            os.fsync(target.fileno())
        if _sha256_file(temporary) != sha or temporary.stat().st_size != snapshot["size_bytes"]:
            temporary.unlink(missing_ok=True)
            _fail(f"{kind} changed while copying to content-addressed storage")
        os.replace(temporary, output)
    after = _snapshot(source, f"candidate {kind}")
    if after["sha256"] != sha or after["size_bytes"] != snapshot["size_bytes"]:
        _fail(f"candidate {kind} changed during content-addressed copy")
    cached = _snapshot(output, f"cached {kind} blob")
    return {
        "kind": kind,
        "sha256": sha,
        "size_bytes": cached["size_bytes"],
        "cas_path": output.relative_to(workspace).as_posix(),
        "dedup_hit": dedup_hit,
    }


def _report_summary(path: Path, session: Mapping[str, Any], label: str) -> dict[str, Any]:
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
            "focus": None,
        }
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
    focus = str(session["request"]["function"])
    total = 0
    exact = 0
    focus_row: dict[str, Any] | None = None
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
            if symbol.get("name") == focus:
                other_size = None
                if isinstance(paired, int) and isinstance(right_symbols, list) and 0 <= paired < len(right_symbols):
                    other = right_symbols[paired]
                    if isinstance(other, Mapping):
                        other_size = other.get("size")
                focus_row = {
                    "name": focus,
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
        "focus": focus_row,
        "diff_kinds": dict(sorted(diff_kinds.items())),
    }
    # Preserve high-level fields when a schema variant does not expose the
    # canonical left/right symbol arrays.
    for key in ("status", "matched", "match_percent", "percent"):
        if key in value and isinstance(value[key], (str, int, float, bool)):
            result[key] = value[key]
    if len(_canonical(result)) > int(session["request"]["policy"]["max_compact_bytes"]):
        _fail(f"{label} compact summary exceeds max_compact_bytes")
    return result


def _store_report(workspace: Path, path: Path, session: Mapping[str, Any], label: str) -> dict[str, Any]:
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
        "compact": _report_summary(path, session, label),
    }


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
            "source", "object", "source_context_key", "object_result_key",
            "source_blob", "object_blob", "reports", "hypothesis", "outcome",
            "report_binding", "telemetry", "duplicate_of", "previous_record_sha256", "authority_advanced",
            "record_sha256",
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
    if not isinstance(value.get("source"), Mapping) or not isinstance(value.get("object"), Mapping):
        _fail("candidate source/object descriptors are missing")
    _sha256(value["source"].get("sha256"), "candidate source.sha256")
    _sha256(value["object"].get("sha256"), "candidate object.sha256")
    for label in ("source", "object"):
        descriptor_value = value[label]
        _closed(descriptor_value, allowed={"path", "size_bytes", "sha256"}, required={"path", "size_bytes", "sha256"}, label=f"candidate {label}")
        _text(descriptor_value["path"], f"candidate {label}.path")
        _integer(descriptor_value["size_bytes"], f"candidate {label}.size_bytes")
    if value.get("source_context_key") != _context_key(session, value["source"]["sha256"]):
        _fail("candidate source context key mismatch")
    if value.get("object_result_key") != _object_key(session, value["object"]["sha256"]):
        _fail("candidate object result key mismatch")
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
    strict_report: Path | str,
    data_report: Path | str | None,
    hypothesis: str,
    axis: str,
    residual: str | None = None,
    status: str = "measured",
    reason: str = "candidate measured",
    heavy_seconds: float | None = None,
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
    object_snapshot = _snapshot(object_file, "candidate object")
    _snapshot(strict_path, "strict report")
    if data_path is not None:
        _snapshot(data_path, "data report")
    lock_path = destination / ".workbench.lock"
    with _workbench_lock(lock_path, 8.0):
        session = _load_session(destination, root)
        index = _load_index(destination, session)
        source_key = _context_key(session, source_snapshot["sha256"])
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
                and existing["object"]["sha256"] == object_snapshot["sha256"]
                and existing["hypothesis"]["name"] == _text(hypothesis, "hypothesis")
                and existing["hypothesis"]["axis"] == _text(axis, "axis")
                and existing["hypothesis"].get("residual_fingerprint")
                == residual_digest
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
                return {"status": "unchanged", "record": existing}
            _fail(f"candidate_id already records different evidence: {candidate_id}")
        source_blob = _copy_blob(destination, source_path, "source", source_snapshot)
        object_blob = _copy_blob(destination, object_file, "object", object_snapshot)
        strict = _store_report(destination, strict_path, session, "strict report")
        data = _store_report(destination, data_path, session, "data report") if data_path else None
        object_duplicate = index["object_index"].get(object_key)
        source_duplicate = index["source_context_index"].get(source_key)
        if source_duplicate and source_duplicate != object_duplicate:
            _fail(
                "the same frozen source/context produced a different object; "
                "recording it would hide a nondeterministic or incomplete cache key"
            )
        duplicate_id = object_duplicate or source_duplicate
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
                "source_context_key": source_key,
                "object_result_key": object_key,
                "source_blob": source_blob,
                "object_blob": object_blob,
                "reports": {"strict": strict, "data": data},
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


def build_matrix(root: Path, workspace: Path | str) -> dict[str, Any]:
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
    actual_candidate_paths = {path.name for path in (destination / "candidates").glob("*.json")}
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
        strict_focus = candidate["reports"]["strict"]["compact"].get("focus")
        data_value = candidate["reports"].get("data")
        data_focus = data_value["compact"].get("focus") if isinstance(data_value, Mapping) else None
        jobs = diagnostics_by_pair.get(
            (str(candidate["object"]["sha256"]), str(candidate["source_context_key"])), []
        )
        diagnostic_status = "not_run" if not jobs else ("available" if all(job.get("status") == "passed" for job in jobs) else "failed")
        focus_exact = bool(strict_focus and strict_focus.get("exact")) and bool(data_focus is None or data_focus.get("exact"))
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
        if duplicate_same_evidence:
            next_action = "reuse_existing_evidence"
        elif candidate.get("duplicate_of") and diagnostic_status == "not_run":
            next_action = "run_read_only_diagnostics_for_source_context"
        elif focus_exact:
            next_action = "authenticate_report_binding_then_run_serial_proof_and_closure"
        elif diagnostic_status == "failed":
            next_action = "repair_read_only_diagnostic"
        else:
            next_action = "continue_one_axis_matching"
        rows.append(
            {
                "ordinal": candidate["ordinal"],
                "candidate_id": candidate_id,
                "source_sha256": candidate["source"]["sha256"],
                "object_sha256": candidate["object"]["sha256"],
                "duplicate_of": candidate.get("duplicate_of"),
                "hypothesis_axis": candidate["hypothesis"]["axis"],
                "axis_fingerprint": candidate["hypothesis"]["axis_fingerprint"],
                "residual_fingerprint": candidate["hypothesis"].get("residual_fingerprint"),
                "strict_focus": strict_focus,
                "data_focus": data_focus,
                "report_binding": candidate["report_binding"],
                "diagnostic_status": diagnostic_status,
                "available_read_only_evidence": diagnostic_status,
                "heavy_seconds": candidate.get("telemetry", {}).get("heavy_seconds"),
                "next_action": next_action,
            }
        )
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
    exact_focus_bytes = sum(
        int(row["strict_focus"]["target_size"])
        for row in rows
        if isinstance(row.get("strict_focus"), Mapping)
        and row["strict_focus"].get("exact")
        and str(row["strict_focus"].get("target_size", "")).isdigit()
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
            "data_focus", "report_binding", "diagnostic_status", "available_read_only_evidence", "next_action",
            "heavy_seconds",
        ],
        "rows": rows,
        "aggregate": aggregate,
        "authority_advanced": False,
    }
    return _with_self_hash(body, "matrix_sha256")


def _print(value: Mapping[str, Any], *, as_json: bool) -> None:
    if as_json:
        print(json.dumps(value, indent=2, ensure_ascii=False, sort_keys=True))
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

    record = commands.add_parser("record", help="record and content-address one measured candidate")
    record.add_argument("--workspace", required=True)
    record.add_argument("--candidate-id", required=True)
    record.add_argument("--source", required=True)
    record.add_argument("--object", required=True)
    record.add_argument("--strict-report", required=True)
    record.add_argument("--data-report")
    record.add_argument("--hypothesis", required=True)
    record.add_argument("--axis", required=True)
    record.add_argument("--residual")
    record.add_argument("--status", default="measured")
    record.add_argument("--reason", default="candidate measured")
    record.add_argument("--heavy-seconds", type=float)
    record.add_argument("--json", action="store_true")

    diagnose = commands.add_parser("diagnose", help="run bounded authenticated read-only diagnostics")
    diagnose.add_argument("--workspace", required=True)
    diagnose.add_argument("--candidate-id", required=True)
    diagnose.add_argument("--jobs", required=True)
    diagnose.add_argument("--max-workers", type=int)
    diagnose.add_argument("--json", action="store_true")

    matrix = commands.add_parser("matrix", help="render the deterministic candidate matrix")
    matrix.add_argument("--workspace", required=True)
    matrix.add_argument("--json", action="store_true")


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
    elif args.match_command == "record":
        result = record_candidate(
            root,
            args.workspace,
            candidate_id=args.candidate_id,
            source=args.source,
            object_path=args.object,
            strict_report=args.strict_report,
            data_report=args.data_report,
            hypothesis=args.hypothesis,
            axis=args.axis,
            residual=args.residual,
            status=args.status,
            reason=args.reason,
            heavy_seconds=args.heavy_seconds,
        )
    elif args.match_command == "diagnose":
        result = diagnose_candidate(
            root, args.workspace, args.candidate_id, args.jobs, max_workers=args.max_workers
        )
    else:
        result = build_matrix(root, args.workspace)
    _print(result, as_json=args.json)
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
