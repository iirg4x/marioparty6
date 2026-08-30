#!/usr/bin/env python3
"""Run one explicitly approved, bounded natural-C crack cell."""

from __future__ import annotations

import argparse
import difflib
import errno
import hashlib
import hmac
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import secrets
import stat
import subprocess
import sys
import tempfile
import threading
import time
from datetime import datetime, timedelta, timezone
from typing import Any, Callable, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.crack_contract import is_closed_objdiff_unit_name
from tools.recovery_pass import serialized_build_lock


APPROVAL_SCHEMA = "crack_harness_approval/v1"
WINNING_CELL_EVIDENCE_SCHEMA = "crack_winning_cell_evidence/v1"
LUNA5_AUDIT_SCHEMA = "crack_luna5_audit/v1"
RETRY_SCHEMA = "crack_harness_legacy_reconciliation/v1"
HISTORICAL_EXACT_EVIDENCE_SCHEMA = "crack_harness_historical_exact_evidence/v1"
RESULT_SCHEMA = "crack_harness_result/v1"
REPORT_SCHEMA = "CRACK_REPORT/v1"
FRONTIER_SCHEMA = "crack_harness_frontier/v1"
PERMIT_SCHEMA = "crack_harness_resume_permit/v1"
DEFAULT_ACTIVE_SECONDS = 30 * 60
DEFAULT_STORAGE_BYTES = 512 * 1024 * 1024
DEFAULT_STATE_ROOT = Path("build/crack-harness")
MANAGER_PERMIT_KEY = Path(r"C:\Users\Anony\.codex\manager-secrets\mp6-crack-harness.key")
MANAGER_ISSUER = "mp6-crack-manager"
MANAGER_KEY_ID = "7a40e303f395fcfb25894819a19ad75488430e16a7d590aa1bc6370738b8591f"
MAX_CLOCK_SKEW_SECONDS = 30
TOOLCHAIN_MANIFEST_KEY = "b6764a1e5883ea1a096bfe4f8b888b93f1740f0f4046eb6149e0fe1d64cc6d90"
MAX_RETAINED_OWNER_BYTES = 16 * 1024 * 1024
MAX_RETAINED_GLOBAL_BYTES = 64 * 1024 * 1024
MAX_COMPACT_TERMINAL_BYTES = 1024 * 1024
MAX_PERMIT_ATTEMPTS_PER_FUNCTION = 32
MAX_CONSUMED_CELLS = 4096
RETRY_USED_SCHEMA = "crack_harness_retry_used/v1"
LUNA5_ROLES = {
    "exact_candidate_recovery", "source_provenance", "retry_safety",
    "permit_pipeline", "adversarial_security",
}
LUNA5_ARTIFACT_SCHEMA = "crack_luna_audit_receipt/v1"
LUNA5_ROLE_CHECKS = {
    "exact_candidate_recovery": {
        "current_source_bound", "candidate_hash_reproduced",
        "predicted_terminal_bound",
    },
    "source_provenance": {
        "natural_c_reviewed", "source_provenance_bound",
        "no_opaque_or_dead_source",
    },
    "retry_safety": {
        "candidate_identity_one_shot", "frontier_base_bound",
        "one_shot_marker_bound",
    },
    "permit_pipeline": {
        "manager_hmac_required", "stop_nonce_bound", "expected_terminal_bound",
        "dry_run_only",
    },
    "adversarial_security": {
        "path_containment_reviewed", "primary_error_preserved",
        "no_compile_or_mutation",
    },
}
EXACT_REPORT_FIELDS = {
    "schema", "status", "completed", "authority_advanced", "owner", "function",
    "task_id", "base_commit", "approval_sha256", "source_sha256",
    "target_object_sha256", "candidate_object_sha256", "result", "proof_receipts",
    "predicted_rows", "completed_at", "report_sha256",
}
EXACT_REPORT_RESULT_FIELDS = {
    "strict_percent", "data_percent", "target_bytes", "candidate_bytes", "owner_gain",
}
EXACT_REPORT_PROOF_RECEIPTS = {
    "precompile", "strict", "data", "focus", "siblings", "physical", "assess", "record",
}
EXACT_RESULT_RECEIPTS = EXACT_REPORT_PROOF_RECEIPTS | {"compile"}
COMMAND_RECEIPT_FIELDS = {
    "argv_sha256", "returncode", "active_seconds", "stdout_sha256", "stderr_sha256",
    "cleanup_errors",
}
TRANSACTION_SCHEMA = "crack_harness_transaction/v1"
TRANSACTION_FIELDS = {
    "schema", "approval_path", "approval_id", "approval_identity_sha256",
    "approval_sha256", "owner", "function", "source_relpath", "source_sha256",
    "base_relpath", "base_sha256", "base_commit", "candidate_relpath",
    "candidate_sha256", "baseline_snapshot", "baseline_sha256",
    "target_object_sha256", "result_path", "report_path", "worktree",
    "record_commit_path", "central_record_binding",
}
# A journal is written before the central record call.  Once that call has
# returned, the journal is advanced atomically with the central row digest so
# startup can distinguish a pre-record failure from a post-record ambiguity.
# Keep the original field set accepted for journals written by older workers.
TRANSACTION_RECORDED_FIELDS = TRANSACTION_FIELDS | {
    "record_succeeded", "record_sha256",
}
LEGACY_TRANSACTION_FIELDS = {
    "schema", "source_relpath", "baseline_snapshot", "baseline_sha256",
    "approval_sha256", "target_object_sha256", "candidate_sha256",
    "result_path", "worktree", "record_commit_path", "central_record_binding",
}
RECOVERY_REQUIRED_SCHEMA = "crack_harness_recovery_required/v2"
ROOT_CLEANUP_RECEIPT_SCHEMA = "crack_harness_root_cleanup_receipt/v1"
ATTEMPT_SCHEMA = "crack_harness_attempt/v2"
PACKET_ROLLBACK_REQUIRED_SCHEMA = "crack_harness_packet_rollback_required/v1"
OBJDIFF_PIN = {
    "path": r"C:\Users\Anony\.codex\tools\objdiff\v3.8.0\objdiff-cli.exe",
    "version": "3.8.0", "size": 7161344,
    "sha256": "3023818f7fdd2f2dc6ade16e68d2c37f5f5754f96881d18d68ddfce77ced15e1",
}
HOOKS = ("strict", "data", "focus", "siblings", "physical")
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
_TEST_STATE_TOKEN = object()
REPARSE_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
CANONICAL_SCRIPTS = {
    "canonical_admission": "tools/candidate_compile_admission.py",
    "canonical_record": "tools/candidate_compile_admission.py",
    "compile": "tools/crack_evidence_bundle.py",
    "proof_strict": "tools/crack_harness.py",
    "proof_data": "tools/crack_harness.py",
    "proof_focus": "tools/crack_harness.py",
    "proof_siblings": "tools/crack_harness.py",
    "proof_physical": "tools/crack_harness.py",
    "assessment": "tools/crack_harness.py",
}


class CrackHarnessError(ValueError):
    """An approval, path, command, proof, or state violated the harness contract."""


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _digest_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _verify_objdiff_pin() -> None:
    path = Path(OBJDIFF_PIN["path"])
    if (
        not path.is_file() or path.stat().st_size != OBJDIFF_PIN["size"]
        or _digest_file(path) != OBJDIFF_PIN["sha256"]
    ):
        raise CrackHarnessError("central objdiff executable does not match the pinned manifest")


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _digest_json(value: Any) -> str:
    return _digest_bytes(_canonical(value))


def _approval_permit_identity(value: Mapping[str, Any]) -> str:
    """Cycle-free identity signed by the manager before permit_sha256 exists."""

    unsigned = dict(value)
    unsigned.pop("permit_sha256", None)
    return _digest_json(unsigned)


def _read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise CrackHarnessError(f"JSON file does not exist: {path}") from exc
    except json.JSONDecodeError as exc:
        raise CrackHarnessError(
            f"invalid JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc


_UNSUPPORTED_DIRECTORY_FSYNC_ERRNOS = frozenset(
    value for value in (
        getattr(errno, "EACCES", None),
        getattr(errno, "EINVAL", None),
        getattr(errno, "ENOSYS", None),
        getattr(errno, "ENOTSUP", None),
        getattr(errno, "EOPNOTSUPP", None),
        getattr(errno, "EPERM", None),
    ) if value is not None
)


def _directory_fsync(path: Path) -> None:
    """Flush a directory entry, tolerating Windows' unsupported operation.

    POSIX filesystems expose directory descriptors for the post-rename flush.
    Windows generally rejects opening a directory with ``os.open`` and does
    not offer the same portable Python primitive; those specific unsupported
    errors are therefore treated as the platform boundary.  Unexpected I/O
    errors still fail the write instead of claiming durability.
    """

    _assert_no_indirection(path)
    flags = os.O_RDONLY | getattr(os, "O_DIRECTORY", 0)
    try:
        descriptor = os.open(os.fspath(path), flags)
    except OSError as exc:
        if os.name == "nt" and exc.errno in _UNSUPPORTED_DIRECTORY_FSYNC_ERRNOS:
            return
        raise
    fsync_error: BaseException | None = None
    try:
        try:
            os.fsync(descriptor)
        except OSError as exc:
            if not (
                os.name == "nt"
                and exc.errno in _UNSUPPORTED_DIRECTORY_FSYNC_ERRNOS
            ):
                fsync_error = exc
        except BaseException as exc:
            fsync_error = exc
    finally:
        try:
            os.close(descriptor)
        except BaseException as exc:
            if fsync_error is None:
                fsync_error = exc
    if fsync_error is not None:
        raise fsync_error


def _note_secondary(primary: BaseException, label: str, error: BaseException) -> None:
    """Attach cleanup diagnostics without replacing the operation's failure."""

    note = f"{label}: {error}"[:1000]
    add_note = getattr(primary, "add_note", None)
    if callable(add_note):
        add_note(note)


def _atomic_bytes(path: Path, value: bytes) -> None:
    """Atomically publish bytes with file and containing-directory flushes."""

    _safe_mkdir(path.parent)
    _assert_no_indirection(path, missing_leaf=True)
    if path.exists() and not path.is_file():
        raise CrackHarnessError(f"atomic output is not a regular file: {path}")
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "wb", dir=path.parent, prefix=path.name, suffix=".tmp", delete=False
        ) as handle:
            temporary = Path(handle.name)
            handle.write(value)
            handle.flush()
            os.fsync(handle.fileno())
        _assert_no_indirection(path, missing_leaf=True)
        os.replace(temporary, path)
        temporary = None
        _directory_fsync(path.parent)
    except BaseException as primary:
        if temporary is not None:
            try:
                _assert_no_indirection(temporary)
                temporary.unlink(missing_ok=True)
            except BaseException as cleanup_error:
                _note_secondary(primary, "atomic temporary cleanup", cleanup_error)
        raise


def _atomic_json(path: Path, value: Any) -> None:
    """Atomically publish JSON and make the rename crash-durable."""

    _safe_mkdir(path.parent)
    _assert_no_indirection(path, missing_leaf=True)
    if path.exists() and not path.is_file():
        raise CrackHarnessError(f"atomic JSON output is not a regular file: {path}")
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, prefix=path.name,
            suffix=".tmp", delete=False
        ) as handle:
            temporary = Path(handle.name)
            json.dump(value, handle, indent=2, sort_keys=True)
            handle.write("\n")
            handle.flush()
            os.fsync(handle.fileno())
        _assert_no_indirection(path, missing_leaf=True)
        os.replace(temporary, path)
        temporary = None
        _directory_fsync(path.parent)
    except BaseException as primary:
        if temporary is not None:
            try:
                _assert_no_indirection(temporary)
                temporary.unlink(missing_ok=True)
            except BaseException as cleanup_error:
                _note_secondary(primary, "atomic JSON temporary cleanup", cleanup_error)
        raise


def _safe_unlink(path: Path) -> None:
    """Remove one regular file and flush its parent, rejecting aliases."""

    if not path.exists() and not path.is_symlink():
        return
    _assert_no_indirection(path)
    if not path.is_file():
        raise CrackHarnessError(f"rollback target is not a regular file: {path}")
    path.unlink()
    _directory_fsync(path.parent)


def _atomic_copy(source: Path, target: Path) -> None:
    _assert_no_indirection(source)
    _safe_mkdir(target.parent)
    _assert_no_indirection(target.parent)
    with source.open("rb") as incoming, tempfile.NamedTemporaryFile(
        "wb", dir=target.parent, prefix=target.name, suffix=".tmp", delete=False
    ) as outgoing:
        shutil.copyfileobj(incoming, outgoing, 1024 * 1024)
        outgoing.flush()
        os.fsync(outgoing.fileno())
        temporary = Path(outgoing.name)
    os.replace(temporary, target)


def _assert_no_indirection(path: Path, *, missing_leaf: bool = False) -> None:
    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for index, part in enumerate(absolute.parts[1:], 1):
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            # Once an ancestor is absent, no descendant can already be an
            # indirection.  Callers that permit a not-yet-created leaf use
            # _safe_mkdir/_atomic_json to validate each component again while
            # materializing the tail.
            if missing_leaf:
                return
            raise CrackHarnessError(f"path component does not exist: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise CrackHarnessError(f"path indirection is forbidden: {current}")


def _safe_mkdir(path: Path) -> None:
    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for part in absolute.parts[1:]:
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            try:
                current.mkdir()
                continue
            except FileExistsError:
                info = current.lstat()
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise CrackHarnessError(f"path indirection is forbidden: {current}")
        if not stat.S_ISDIR(info.st_mode):
            raise CrackHarnessError(f"directory path is not a directory: {current}")


def _manager_key_file(root: Path, manager_key_path: Path) -> Path:
    """Validate the manager key lexically before resolving its final path."""

    root = Path(os.path.abspath(root))
    raw = Path(os.fspath(manager_key_path)).expanduser()
    lexical = Path(os.path.abspath(raw))
    # Resolving first would hide a link/reparse component and turn an
    # out-of-tree alias into an apparently safe regular file.
    _assert_no_indirection(lexical)
    if _inside(root, lexical):
        raise CrackHarnessError(
            "manager permit key must be a plain file outside the repository"
        )
    resolved = lexical.resolve(strict=True)
    if _inside(root, resolved):
        raise CrackHarnessError(
            "manager permit key must be a plain file outside the repository"
        )
    info = lexical.lstat()
    if not stat.S_ISREG(info.st_mode) or bool(
        getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
    ):
        raise CrackHarnessError(
            "manager permit key must be a plain file outside the repository"
        )
    return lexical


def _inside(root: Path, path: Path) -> bool:
    try:
        return os.path.normcase(os.path.commonpath((root, path))) == os.path.normcase(
            os.fspath(root)
        )
    except ValueError:
        return False


def _bound_path(root: Path, value: Any, label: str, *, exists: bool = True) -> Path:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise CrackHarnessError(f"{label} must be a non-empty path")
    raw = Path(value).expanduser()
    path = Path(os.path.abspath(raw if raw.is_absolute() else root / raw))
    if not _inside(root, path):
        raise CrackHarnessError(f"{label} escapes repository root: {path}")
    _assert_no_indirection(path, missing_leaf=not exists)
    if exists and not path.is_file():
        raise CrackHarnessError(f"{label} is not a regular file: {path}")
    return path


def _state_path(
    state: Path, value: Path | str, label: str, *, exists: bool | None = None,
) -> Path:
    """Bind a durable harness path to the canonical state tree.

    State metadata is local and self-hashed, not manager-signed.  Every caller
    that is about to read or mutate a path recovered from that metadata must
    therefore check both containment and every path component.  ``exists`` is
    tri-state so callers can require a regular file, require an absent leaf, or
    merely validate a path whose presence is handled separately.
    """

    state_path = Path(os.path.abspath(state))
    path = Path(os.path.abspath(value))
    if not _inside(state_path, path):
        raise CrackHarnessError(f"{label} escapes harness state: {path}")
    _assert_no_indirection(state_path)
    if exists is False:
        _assert_no_indirection(path, missing_leaf=True)
        if path.exists():
            raise CrackHarnessError(f"{label} unexpectedly exists: {path}")
    elif exists is True:
        _assert_no_indirection(path)
        if not path.is_file():
            raise CrackHarnessError(f"{label} is not a regular file: {path}")
    else:
        _assert_no_indirection(path, missing_leaf=not path.exists())
    return path


def _state_glob(state: Path, pattern: str, label: str) -> list[Path]:
    """Enumerate state paths only after validating the complete path chain."""

    state_path = Path(os.path.abspath(state))
    _assert_no_indirection(state_path)
    paths: list[Path] = []
    for candidate in state_path.glob(pattern):
        path = Path(os.path.abspath(candidate))
        if not _inside(state_path, path):
            raise CrackHarnessError(f"{label} escapes harness state: {path}")
        _assert_no_indirection(path)
        paths.append(path)
    return paths


def _state_run_dir(state: Path, run_dir: Path, label: str = "run directory") -> Path:
    """Validate the fixed owners/<owner>/<function>/latest layout."""

    path = _state_path(state, run_dir, label)
    try:
        relative = path.relative_to(Path(os.path.abspath(state))).parts
    except ValueError as exc:
        raise CrackHarnessError(f"{label} escapes harness state: {path}") from exc
    if (
        len(relative) != 4
        or relative[0] != "owners"
        or relative[-1] != "latest"
        or any(not part or part in {".", ".."} for part in relative[1:])
    ):
        raise CrackHarnessError(f"{label} does not use the exact latest state layout: {path}")
    return path


def _state_result_path(
    state: Path, path: Path, label: str = "terminal result",
) -> Path:
    result = _state_path(state, path, label, exists=True)
    run_dir = _state_run_dir(state, result.parent)
    if result.name != "result.json":
        raise CrackHarnessError(f"{label} is not the canonical result path: {result}")
    if result.parent != run_dir:
        raise CrackHarnessError(f"{label} is not under latest state: {result}")
    return result


def _state_sibling_path(
    state: Path, path: Path, name: str, label: str, *, exists: bool | None = None,
) -> Path:
    item = _state_path(state, path, label, exists=exists)
    run_dir = _state_run_dir(state, item.parent)
    if item.name != name or item.parent != run_dir:
        raise CrackHarnessError(f"{label} is not the canonical {name} path: {item}")
    return item


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise CrackHarnessError(f"{label} must be non-empty text")
    return value.strip()


def _sha(value: Any, label: str) -> str:
    result = _text(value, label)
    if result != result.lower() or SHA_RE.fullmatch(result) is None:
        raise CrackHarnessError(f"{label} must be a lowercase SHA-256 digest")
    return result


def _retry_file_descriptor(
    root: Path, value: Any, label: str,
) -> tuple[Path, str]:
    if not isinstance(value, Mapping) or set(value) != {"path", "sha256"}:
        raise CrackHarnessError(f"{label} must bind one path and sha256")
    path = _bound_path(root, value.get("path"), f"{label}.path")
    digest = _sha(value.get("sha256"), f"{label}.sha256")
    if _digest_file(path) != digest:
        raise CrackHarnessError(f"{label} hash mismatch: {path}")
    return path, digest


def _historical_exact_values(
    value: Any, owner: str, function: str, candidate_sha256: str,
    label: str = "historical exact evidence",
) -> dict[str, Any]:
    """Validate the small, immutable proof contract used by legacy retry.

    A historical report is evidence for choosing the old cell again, not a
    replacement for the current proof pipeline.  Accept only the dedicated
    compact schema and require both strict and data channels to carry equal
    byte counts and zero differences.  A historical CRACK_REPORT is too broad
    to serve as retry authority and must first be reduced to this contract.
    """

    if not isinstance(value, Mapping):
        raise CrackHarnessError(f"{label} must be a JSON object")
    schema = value.get("schema")
    if schema == HISTORICAL_EXACT_EVIDENCE_SCHEMA:
        required = {
            "schema", "owner", "function", "candidate_sha256",
            "legacy_controller_commit", "target_bytes", "candidate_bytes",
            "strict_percent", "data_percent", "strict_diff_rows",
            "data_diff_rows",
        }
        if set(value) != required:
            raise CrackHarnessError(f"{label} compact schema is incomplete")
        strict_rows = value.get("strict_diff_rows")
        data_rows = value.get("data_diff_rows")
        target_bytes = value.get("target_bytes")
        candidate_bytes = value.get("candidate_bytes")
        strict_percent = value.get("strict_percent")
        data_percent = value.get("data_percent")
        legacy_commit = value.get("legacy_controller_commit")
        if (
            value.get("owner") != owner
            or value.get("function") != function
            or value.get("candidate_sha256") != candidate_sha256
        ):
            raise CrackHarnessError(f"{label} is not bound to this owner/candidate")
    else:
        raise CrackHarnessError(
            f"{label} must use {HISTORICAL_EXACT_EVIDENCE_SCHEMA}"
        )

    for metric, metric_label in (
        (target_bytes, f"{label}.target_bytes"),
        (candidate_bytes, f"{label}.candidate_bytes"),
        (strict_rows, f"{label}.strict_diff_rows"),
        (data_rows, f"{label}.data_diff_rows"),
    ):
        if type(metric) is not int or metric < 0:
            raise CrackHarnessError(f"{metric_label} must be a non-negative integer")
    if target_bytes != candidate_bytes:
        raise CrackHarnessError(f"{label} target/candidate byte counts differ")
    if strict_percent != 100 and strict_percent != 100.0:
        raise CrackHarnessError(f"{label} strict proof is not 100 percent")
    if data_percent != 100 and data_percent != 100.0:
        raise CrackHarnessError(f"{label} data proof is not 100 percent")
    if strict_rows != 0 or data_rows != 0:
        raise CrackHarnessError(f"{label} strict/data proof has residual rows")
    legacy_commit = _text(legacy_commit, f"{label}.legacy_controller_commit")
    return {
        "owner": owner, "function": function,
        "candidate_sha256": candidate_sha256,
        "legacy_controller_commit": legacy_commit,
        "target_bytes": target_bytes, "candidate_bytes": candidate_bytes,
        "strict_percent": strict_percent, "data_percent": data_percent,
        "strict_diff_rows": strict_rows, "data_diff_rows": data_rows,
    }


def _validate_retry_descriptor(
    root: Path, retry: Any, owner: str, function: str,
    candidate_sha256: str,
) -> dict[str, Any] | None:
    """Validate authority-free legacy reconciliation metadata.

    The descriptor is deliberately insufficient to authorize execution.  It is
    part of the approval identity, which the external manager must HMAC-sign;
    the actual run still requires the normal manager permit.
    """

    if retry is None:
        return None
    required = {
        "schema", "tombstone", "failure", "prior_approval_sha256",
        "candidate_sha256", "legacy_controller_commit",
        "historical_exact_evidence",
    }
    if not isinstance(retry, Mapping) or set(retry) != required:
        raise CrackHarnessError(
            "retry must be a strict crack_harness_legacy_reconciliation/v1 object"
        )
    if retry.get("schema") != RETRY_SCHEMA:
        raise CrackHarnessError(f"retry schema must be {RETRY_SCHEMA}")
    if retry.get("candidate_sha256") != candidate_sha256:
        raise CrackHarnessError("retry candidate_sha256 does not match approval candidate")
    prior = _sha(retry.get("prior_approval_sha256"), "retry.prior_approval_sha256")
    legacy_commit = _text(
        retry.get("legacy_controller_commit"), "retry.legacy_controller_commit"
    )
    tombstone_path, tombstone_sha = _retry_file_descriptor(
        root, retry.get("tombstone"), "retry.tombstone"
    )
    tombstone = _read_json(tombstone_path)
    if (
        not isinstance(tombstone, Mapping)
        or set(tombstone) != {
            "schema", "function_key", "owner", "function",
            "first_campaign_id", "consumed",
        }
        or tombstone.get("schema") != "crack_harness_function_tombstone/v1"
        or tombstone.get("function_key") != _digest_json({"owner": owner, "function": function})
        or tombstone.get("owner") != owner
        or tombstone.get("function") != function
        or not isinstance(tombstone.get("first_campaign_id"), str)
        or not tombstone.get("first_campaign_id")
        or tombstone.get("consumed") is not True
    ):
        raise CrackHarnessError("retry tombstone must be an exact consumed legacy v1 tombstone")
    failure_path, failure_sha = _retry_file_descriptor(
        root, retry.get("failure"), "retry.failure"
    )
    failure = _read_json(failure_path)
    if not isinstance(failure, Mapping):
        raise CrackHarnessError("retry failure diagnostic must be a JSON object")
    failure_body = dict(failure)
    failure_digest = failure_body.pop("diagnostic_sha256", None)
    failure_required = {
        "schema", "approval_sha256", "owner", "function", "primary_reason",
        "cleanup_errors", "finished_at",
    }
    if (
        set(failure_body) != failure_required
        or failure.get("schema") != "crack_harness_failure_diagnostic/v1"
        or failure_digest != _digest_json(failure_body)
        or failure.get("approval_sha256") != prior
        or failure.get("owner") != owner
        or failure.get("function") != function
        or not isinstance(failure.get("primary_reason"), str)
        or not failure.get("primary_reason")
        or not isinstance(failure.get("cleanup_errors"), list)
        or len(failure.get("cleanup_errors")) > 8
        or any(not isinstance(item, str) or len(item) > 1000 for item in failure.get("cleanup_errors"))
    ):
        raise CrackHarnessError("retry failure diagnostic is not the exact prior failure")
    _timestamp(failure.get("finished_at"), "retry.failure.finished_at")
    evidence_path, evidence_sha = _retry_file_descriptor(
        root, retry.get("historical_exact_evidence"),
        "retry.historical_exact_evidence",
    )
    evidence = _historical_exact_values(
        _read_json(evidence_path), owner, function, candidate_sha256
    )
    if evidence["legacy_controller_commit"] != legacy_commit:
        raise CrackHarnessError(
            "retry legacy_controller_commit does not match historical exact evidence"
        )
    return {
        "schema": RETRY_SCHEMA,
        "tombstone_path": tombstone_path,
        "tombstone_sha256": tombstone_sha,
        "failure_path": failure_path,
        "failure_sha256": failure_sha,
        "prior_approval_sha256": prior,
        "candidate_sha256": candidate_sha256,
        "legacy_controller_commit": legacy_commit,
        "historical_exact_evidence_path": evidence_path,
        "historical_exact_evidence_sha256": evidence_sha,
        "historical_exact_evidence": evidence,
    }


def _validate_luna5_audit(
    root: Path, descriptor: Any, owner: str, function: str,
    candidate_sha256: str, controller_commit: str,
) -> None:
    if not isinstance(descriptor, Mapping) or set(descriptor) != {"path", "sha256"}:
        raise CrackHarnessError("selection.luna5_audit must bind one path and sha256")
    path = _bound_path(root, descriptor.get("path"), "selection.luna5_audit.path")
    expected_sha256 = _sha(
        descriptor.get("sha256"), "selection.luna5_audit.sha256"
    )
    if _digest_file(path) != expected_sha256:
        raise CrackHarnessError(f"five-Luna audit hash mismatch: {path}")
    value = _read_json(path)
    required = {
        "schema", "owner", "function", "controller_commit",
        "candidate_sha256", "receipts", "authority_advanced",
    }
    if not isinstance(value, Mapping) or set(value) != required:
        raise CrackHarnessError("five-Luna audit must be a strict closed object")
    if (
        value.get("schema") != LUNA5_AUDIT_SCHEMA
        or value.get("owner") != owner
        or value.get("function") != function
        or value.get("controller_commit") != controller_commit
        or value.get("candidate_sha256") != candidate_sha256
        or value.get("authority_advanced") is not False
    ):
        raise CrackHarnessError("five-Luna audit identity binding is invalid")
    receipts = value.get("receipts")
    if not isinstance(receipts, list) or len(receipts) != len(LUNA5_ROLES):
        raise CrackHarnessError("five-Luna audit must contain exactly five receipts")
    roles: set[str] = set()
    agents: set[str] = set()
    artifacts: set[str] = set()
    for index, receipt in enumerate(receipts):
        fields = {
            "role", "agent_id", "model", "reasoning_effort", "status",
            "controller_commit", "artifact", "mutations", "compiled",
        }
        if not isinstance(receipt, Mapping) or set(receipt) != fields:
            raise CrackHarnessError(f"five-Luna receipt {index} is not strict")
        role = _text(receipt.get("role"), f"five-Luna receipt {index}.role")
        agent_id = _text(
            receipt.get("agent_id"), f"five-Luna receipt {index}.agent_id"
        )
        artifact = receipt.get("artifact")
        if not isinstance(artifact, Mapping) or set(artifact) != {"path", "sha256"}:
            raise CrackHarnessError(f"five-Luna receipt {index} artifact is invalid")
        artifact_path = _bound_path(
            root, artifact.get("path"), f"five-Luna receipt {index}.artifact.path"
        )
        artifact_sha256 = _sha(
            artifact.get("sha256"),
            f"five-Luna receipt {index}.artifact.sha256",
        )
        relative = artifact_path.relative_to(root).as_posix()
        if _digest_file(artifact_path) != artifact_sha256:
            raise CrackHarnessError(
                f"five-Luna receipt artifact hash mismatch: {artifact_path}"
            )
        artifact_value = _read_json(artifact_path)
        artifact_fields = {
            "schema", "role", "agent_id", "model", "reasoning_effort",
            "status", "owner", "function", "controller_commit",
            "candidate_sha256", "checks", "findings", "mutations",
            "compiled", "authority_advanced",
        }
        if (
            not isinstance(artifact_value, Mapping)
            or set(artifact_value) != artifact_fields
            or artifact_value.get("schema") != LUNA5_ARTIFACT_SCHEMA
            or artifact_value.get("role") != role
            or artifact_value.get("agent_id") != agent_id
            or artifact_value.get("model") != "gpt-5.6-luna"
            or artifact_value.get("reasoning_effort") != "max"
            or artifact_value.get("status") != "PASS"
            or artifact_value.get("owner") != owner
            or artifact_value.get("function") != function
            or artifact_value.get("controller_commit") != controller_commit
            or artifact_value.get("candidate_sha256") != candidate_sha256
            or artifact_value.get("mutations") is not False
            or artifact_value.get("compiled") is not False
            or artifact_value.get("authority_advanced") is not False
        ):
            raise CrackHarnessError(
                f"five-Luna receipt {index} artifact content is unbound"
            )
        checks = artifact_value.get("checks")
        required_checks = LUNA5_ROLE_CHECKS.get(role)
        if (
            required_checks is None
            or not isinstance(checks, Mapping)
            or set(checks) != required_checks
            or any(value is not True for value in checks.values())
        ):
            raise CrackHarnessError(
                f"five-Luna receipt {index} artifact checks are incomplete"
            )
        findings = artifact_value.get("findings")
        if (
            not isinstance(findings, list)
            or len(findings) > 16
            or any(
                not isinstance(item, str) or not item.strip() or len(item) > 1000
                for item in findings
            )
        ):
            raise CrackHarnessError(
                f"five-Luna receipt {index} artifact findings are invalid"
            )
        if (
            role not in LUNA5_ROLES
            or agent_id in agents
            or role in roles
            or relative in artifacts
            or receipt.get("model") != "gpt-5.6-luna"
            or receipt.get("reasoning_effort") != "max"
            or receipt.get("status") != "PASS"
            or receipt.get("controller_commit") != controller_commit
            or receipt.get("mutations") is not False
            or receipt.get("compiled") is not False
        ):
            raise CrackHarnessError(
                f"five-Luna receipt {index} is duplicated, drifted, or non-PASS"
            )
        roles.add(role)
        agents.add(agent_id)
        artifacts.add(relative)
    if roles != LUNA5_ROLES:
        raise CrackHarnessError("five-Luna audit does not cover the fixed roles")


def _validate_predicted_rows(value: Any, label: str = "predicted_rows") -> list[str]:
    if (
        not isinstance(value, list)
        or not value
        or any(not isinstance(row, str) or not row.strip() for row in value)
    ):
        raise CrackHarnessError(f"{label} must be a non-empty string array")
    if len(set(value)) != len(value):
        raise CrackHarnessError(f"{label} must contain unique rows")
    return list(value)


def _validate_winning_cell_selection(
    root: Path, selection: Any, candidate_sha256: str,
    predicted_rows: Sequence[str], owner: str, function: str,
    controller_commit: str, *, mutable_source_path: Path,
) -> None:
    """Require one evidence-backed winning cell before any compile is legal."""

    predicted_rows = _validate_predicted_rows(list(predicted_rows))
    mutable_source = Path(os.path.abspath(mutable_source_path))
    required = {
        "strategy", "rank", "expected_terminal", "evidence", "candidate_sha256",
        "predicted_rows_sha256", "alternatives_compiled", "negative_controls",
        "pivot_if_unranked", "source_class", "luna5_audit",
    }
    if not isinstance(selection, Mapping) or set(selection) != required:
        raise CrackHarnessError(
            "selection must be a strict closed winning-cell-first object"
        )
    if selection.get("strategy") != "winning_cell_first":
        raise CrackHarnessError("selection.strategy must be winning_cell_first")
    rank = selection.get("rank")
    if type(rank) is not int or rank != 1:
        raise CrackHarnessError("selection.rank must be exactly 1")
    expected_terminal = selection.get("expected_terminal")
    if expected_terminal not in {"exact", "improved"}:
        raise CrackHarnessError(
            "selection.expected_terminal must be exact or improved"
        )
    evidence = selection.get("evidence")
    if not isinstance(evidence, Mapping) or set(evidence) != {"path", "sha256"}:
        raise CrackHarnessError("selection.evidence must bind one path and sha256")
    evidence_path = _bound_path(
        root, evidence.get("path"), "selection.evidence.path", exists=True
    )
    expected_evidence_sha256 = _sha(
        evidence.get("sha256"), "selection.evidence.sha256"
    )
    if _digest_file(evidence_path) != expected_evidence_sha256:
        raise CrackHarnessError(
            f"selection evidence hash mismatch: {evidence_path}"
        )
    selected_candidate = _sha(
        selection.get("candidate_sha256"), "selection.candidate_sha256"
    )
    if selected_candidate != candidate_sha256:
        raise CrackHarnessError(
            "selection.candidate_sha256 does not match approval candidate"
        )
    selected_rows = _sha(
        selection.get("predicted_rows_sha256"),
        "selection.predicted_rows_sha256",
    )
    if selected_rows != _digest_json(list(predicted_rows)):
        raise CrackHarnessError(
            "selection.predicted_rows_sha256 does not match predicted_rows"
        )
    for key in ("alternatives_compiled", "negative_controls"):
        if type(selection.get(key)) is not int or selection.get(key) != 0:
            raise CrackHarnessError(f"selection.{key} must be exactly 0")
    if selection.get("pivot_if_unranked") is not True:
        raise CrackHarnessError("selection.pivot_if_unranked must be true")
    source_class = _text(selection.get("source_class"), "selection.source_class")
    _validate_luna5_audit(
        root, selection.get("luna5_audit"), owner, function,
        candidate_sha256, controller_commit,
    )

    evidence_value = _read_json(evidence_path)
    evidence_required = {
        "schema", "owner", "function", "strategy", "rank", "expected_terminal",
        "candidate_sha256", "predicted_rows_sha256",
        "alternatives_compiled", "negative_controls", "pivot_if_unranked",
        "source_class", "inputs", "causal_prediction",
    }
    if not isinstance(evidence_value, Mapping) or set(evidence_value) != evidence_required:
        raise CrackHarnessError(
            "selection evidence must be a strict crack_winning_cell_evidence/v1 object"
        )
    evidence_bindings = {
        "schema": WINNING_CELL_EVIDENCE_SCHEMA,
        "owner": owner,
        "function": function,
        "strategy": "winning_cell_first",
        "rank": 1,
        "expected_terminal": expected_terminal,
        "candidate_sha256": candidate_sha256,
        "predicted_rows_sha256": _digest_json(list(predicted_rows)),
        "alternatives_compiled": 0,
        "negative_controls": 0,
        "pivot_if_unranked": True,
        "source_class": source_class,
    }
    for key, expected in evidence_bindings.items():
        if evidence_value.get(key) != expected:
            raise CrackHarnessError(
                f"selection evidence does not bind approval {key}"
            )
    inputs = evidence_value.get("inputs")
    if (
        not isinstance(inputs, list) or not 1 <= len(inputs) <= 16
        or any(not isinstance(item, Mapping) for item in inputs)
    ):
        raise CrackHarnessError("selection evidence inputs must contain 1-16 files")
    seen_inputs: set[tuple[str, str]] = set()
    for index, item in enumerate(inputs):
        if set(item) != {"path", "sha256", "role"}:
            raise CrackHarnessError(
                f"selection evidence input {index} must bind path, sha256, and role"
            )
        input_path = _bound_path(
            root, item.get("path"), f"selection evidence input {index}.path"
        )
        input_sha256 = _sha(
            item.get("sha256"), f"selection evidence input {index}.sha256"
        )
        role = _text(item.get("role"), f"selection evidence input {index}.role")
        identity = (input_path.relative_to(root).as_posix(), role)
        if identity in seen_inputs:
            raise CrackHarnessError("selection evidence inputs contain a duplicate")
        seen_inputs.add(identity)
        if input_path == mutable_source or os.path.samefile(
            input_path, mutable_source
        ):
            raise CrackHarnessError(
                "selection evidence cannot bind the mutable live source"
            )
        if _digest_file(input_path) != input_sha256:
            raise CrackHarnessError(
                f"selection evidence input hash mismatch: {input_path}"
            )
    causal = evidence_value.get("causal_prediction")
    if (
        not isinstance(causal, Mapping)
        or set(causal) != {"earliest_divergence", "predicted_effect", "predicted_rows"}
    ):
        raise CrackHarnessError("selection evidence causal_prediction is not strict")
    _text(causal.get("earliest_divergence"), "causal_prediction.earliest_divergence")
    _text(causal.get("predicted_effect"), "causal_prediction.predicted_effect")
    if causal.get("predicted_rows") != list(predicted_rows):
        raise CrackHarnessError(
            "selection evidence causal_prediction does not bind predicted_rows"
        )


def _timestamp(value: Any, label: str) -> datetime:
    try:
        result = datetime.fromisoformat(_text(value, label).replace("Z", "+00:00"))
    except ValueError as exc:
        raise CrackHarnessError(f"{label} must be an ISO-8601 timestamp") from exc
    if result.tzinfo is None:
        raise CrackHarnessError(f"{label} must include a timezone")
    return result.astimezone(timezone.utc)


def _argv(value: Any, label: str) -> list[str]:
    if not isinstance(value, list) or not value or not all(
        isinstance(item, str) and item and "\x00" not in item for item in value
    ):
        raise CrackHarnessError(f"{label} must be a non-empty argv string array")
    return list(value)


def _command(root: Path, value: Any, label: str, kind: str) -> dict[str, Any]:
    if not isinstance(value, Mapping) or value.get("kind") != kind:
        raise CrackHarnessError(f"{label} must be a typed {kind} command descriptor")
    if set(value) != {"kind", "argv", "executable", "script"}:
        raise CrackHarnessError(f"{label} has an open or incomplete command descriptor")
    argv = _argv(value.get("argv"), f"{label}.argv")
    executable = value.get("executable")
    if not isinstance(executable, Mapping) or set(executable) != {"path", "sha256"}:
        raise CrackHarnessError(f"{label}.executable must bind path and sha256")
    controller_python = Path(sys.executable).resolve()
    controller_python_text = str(controller_python)
    if executable.get("path") != controller_python_text or argv[0] != controller_python_text:
        raise CrackHarnessError(
            f"{label} executable must be the exact controller interpreter {controller_python_text}"
        )
    controller_python_sha256 = _digest_file(controller_python)
    if executable.get("sha256") != controller_python_sha256:
        raise CrackHarnessError(f"{label} executable SHA-256 is not the controller interpreter pin")
    executable_path = Path(os.path.abspath(Path(str(executable["path"])).expanduser()))
    if not executable_path.is_file() or _digest_file(executable_path) != _sha(executable["sha256"], f"{label}.executable.sha256"):
        raise CrackHarnessError(f"{label} executable hash mismatch: {executable_path}")
    if Path(argv[0]).resolve() != executable_path.resolve():
        raise CrackHarnessError(f"{label}.argv[0] is not the pinned executable")
    script = value.get("script")
    script_path = None
    if script is not None:
        if not isinstance(script, Mapping) or set(script) != {"path", "sha256"}:
            raise CrackHarnessError(f"{label}.script must bind path and sha256")
        script_path = _bound_path(root, script.get("path"), f"{label}.script.path")
        if _digest_file(script_path) != _sha(script.get("sha256"), f"{label}.script.sha256"):
            raise CrackHarnessError(f"{label} script hash mismatch: {script_path}")
        argv_script = argv[1].replace("{CONTROLLER_ROOT}", str(root)) if len(argv) > 1 else ""
        if len(argv) < 2 or Path(argv_script).resolve() != script_path.resolve():
            raise CrackHarnessError(f"{label}.argv[1] is not the pinned script")
    canonical_script = (root / CANONICAL_SCRIPTS[kind]).resolve()
    if script_path is None or script_path.resolve() != canonical_script:
        raise CrackHarnessError(
            f"{label} must use canonical registered script {canonical_script}"
        )
    return {
        "kind": kind, "argv": argv, "executable": executable_path,
        "executable_sha256": _sha(executable["sha256"], f"{label}.executable.sha256"),
        "script": script_path,
        "script_sha256": (
            _sha(script["sha256"], f"{label}.script.sha256")
            if isinstance(script, Mapping) else None
        ),
    }


def _validate_command_receipt(value: Any, label: str) -> dict[str, Any]:
    """Validate the sealed result of one reviewed command invocation."""

    if not isinstance(value, Mapping) or set(value) != COMMAND_RECEIPT_FIELDS:
        raise CrackHarnessError(f"{label} is not a complete command receipt")
    for key in ("argv_sha256", "stdout_sha256", "stderr_sha256"):
        _sha(value.get(key), f"{label}.{key}")
    returncode = value.get("returncode")
    if type(returncode) is not int or returncode != 0:
        raise CrackHarnessError(f"{label}.returncode is not a successful exit")
    active_seconds = value.get("active_seconds")
    if isinstance(active_seconds, bool) or not isinstance(active_seconds, (int, float)):
        raise CrackHarnessError(f"{label}.active_seconds is not numeric")
    try:
        active_value = float(active_seconds)
    except OverflowError as exc:
        raise CrackHarnessError(f"{label}.active_seconds is not finite") from exc
    if not math.isfinite(active_value) or active_value < 0:
        raise CrackHarnessError(f"{label}.active_seconds is not finite")
    cleanup_errors = value.get("cleanup_errors")
    if (
        not isinstance(cleanup_errors, list)
        or len(cleanup_errors) > 8
        or any(not isinstance(item, str) or len(item) > 1000 for item in cleanup_errors)
    ):
        raise CrackHarnessError(f"{label}.cleanup_errors is invalid")
    return dict(value)


def _limits(value: Any) -> dict[str, int]:
    if value is None:
        value = {}
    if not isinstance(value, Mapping):
        raise CrackHarnessError("limits must be an object")
    if set(value) - {"active_seconds", "temporary_bytes", "candidates"}:
        raise CrackHarnessError("limits is a strict closed object")
    active = value.get("active_seconds", DEFAULT_ACTIVE_SECONDS)
    storage = value.get("temporary_bytes", DEFAULT_STORAGE_BYTES)
    candidates = value.get("candidates", 1)
    for item, label in (
        (active, "limits.active_seconds"),
        (storage, "limits.temporary_bytes"),
        (candidates, "limits.candidates"),
    ):
        if isinstance(item, bool) or not isinstance(item, int) or item < 1:
            raise CrackHarnessError(f"{label} must be a positive integer")
    if candidates != 1:
        raise CrackHarnessError("one approval must authorize exactly one candidate")
    if active > DEFAULT_ACTIVE_SECONDS or storage > DEFAULT_STORAGE_BYTES:
        raise CrackHarnessError("hard time/storage limits are non-elevatable")
    return {"active_seconds": active, "temporary_bytes": storage, "candidates": 1}


def _validate_natural_cell(
    base: Path, candidate: Path, start: int, end: int,
    base_span_sha256: str | None = None,
) -> None:
    try:
        before = base.read_bytes()
        after = candidate.read_bytes()
        if b"\0" in before or b"\0" in after:
            raise CrackHarnessError("natural-C cell contains NUL")
        old_lines = before.decode("utf-8").splitlines(keepends=True)
        new_lines = after.decode("utf-8").splitlines(keepends=True)
    except UnicodeDecodeError as exc:
        raise CrackHarnessError("natural-C cell must be UTF-8") from exc
    if end > len(old_lines):
        raise CrackHarnessError("approved function span exceeds the sealed base")
    span_bytes = "".join(old_lines[start - 1:end]).encode("utf-8")
    if base_span_sha256 is not None and _digest_bytes(span_bytes) != base_span_sha256:
        raise CrackHarnessError("approved function span hash does not match the sealed base")
    changes = [opcode for opcode in difflib.SequenceMatcher(a=old_lines, b=new_lines, autojunk=False).get_opcodes() if opcode[0] != "equal"]
    if not changes or len(changes) > 3:
        raise CrackHarnessError("candidate must contain one bounded semantic cell of at most three hunks")
    changed_lines = 0
    changed_text_parts = []
    for _, old_start, old_end, new_start, new_end in changes:
        # SequenceMatcher represents an insertion at a zero-width boundary.
        # The first and last boundaries of the approved span are outside the
        # function, so accepting them would let a supposedly function-scoped
        # cell inject a declaration immediately before/after the function.
        outside_span = old_start < start - 1 or old_end > end
        insertion_at_boundary = (
            old_start == old_end and not (start <= old_start < end)
        )
        if outside_span or insertion_at_boundary:
            raise CrackHarnessError("candidate changes lines outside the approved function span")
        changed_lines += (old_end - old_start) + (new_end - new_start)
        changed_text_parts.extend(new_lines[new_start:new_end])
    if changed_lines > 80:
        raise CrackHarnessError("natural-C cell exceeds the 80 changed-line budget")
    changed_text = "".join(changed_text_parts)
    forbidden = (
        r"\b(?:asm|__asm|volatile|register)\b",
        r"\b(?:padding|pad_bytes|dead_code)\b|\b_pad\w*",
        r"\bif\s*\(\s*(?:0|false)\s*\)",
        r"__attribute__|#\s*pragma\s+(?:inline|optimization)",
    )
    if any(re.search(pattern, changed_text, re.I) for pattern in forbidden):
        raise CrackHarnessError("candidate contains a forbidden shaping marker")

    if _contains_preprocessor_directive(changed_text):
        raise CrackHarnessError("candidate contains a preprocessor directive")

    _validate_function_span_structure(
        before.decode("utf-8"), after.decode("utf-8"), start, end
    )


def _mask_c_noncode(text: str) -> str:
    """Blank comments and literals while retaining source offsets and newlines."""

    chars = list(text)
    state = "code"
    quote = ""
    index = 0
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                chars[index] = " "
                chars[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if char == "/" and next_char == "*":
                chars[index] = " "
                chars[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if char in {"\"", "'"}:
                chars[index] = " "
                quote = char
                state = "literal"
            index += 1
            continue
        if state == "line_comment":
            if char in "\r\n":
                state = "code"
            else:
                chars[index] = " "
            index += 1
            continue
        if state == "block_comment":
            if char == "*" and next_char == "/":
                chars[index] = " "
                chars[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char not in "\r\n":
                chars[index] = " "
            index += 1
            continue

        # C literals may contain braces and preprocessor-looking text. Keep only
        # line endings visible to the structural scanner.
        if char == "\\":
            chars[index] = " "
            index += 1
            if index < len(text) and text[index] not in "\r\n":
                chars[index] = " "
                index += 1
            continue
        if char == quote:
            chars[index] = " "
            state = "code"
        elif char in "\r\n":
            state = "code"
        else:
            chars[index] = " "
        index += 1
    return "".join(chars)


def _contains_preprocessor_directive(text: str) -> bool:
    masked = _mask_c_noncode(text)
    continued = False
    for line in masked.splitlines(keepends=True):
        content = line.rstrip("\r\n")
        directive = continued or content.lstrip(" \t").startswith("#")
        if directive:
            return True
        continued = directive and content.rstrip(" \t").endswith("\\")
    return False


def _mask_preprocessor_directives(masked: str) -> str:
    chars = list(masked)
    offset = 0
    continued = False
    for line in masked.splitlines(keepends=True):
        content = line.rstrip("\r\n")
        directive = continued or content.lstrip(" \t").startswith("#")
        if directive:
            for index in range(offset, offset + len(line)):
                if chars[index] not in "\r\n":
                    chars[index] = " "
        continued = directive and content.rstrip(" \t").endswith("\\")
        offset += len(line)
    return "".join(chars)


def _line_offsets(text: str) -> list[int]:
    offsets = [0]
    for line in text.splitlines(keepends=True):
        offsets.append(offsets[-1] + len(line))
    if offsets[-1] != len(text):
        offsets.append(len(text))
    return offsets


def _matching_brace(masked: str, opening: int) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                return None
    return None


_FUNCTION_BLOCK_RE = re.compile(r"(?P<name>[A-Za-z_]\w*)\s*\([^{};]*\)\s*\{")
_CONTROL_BLOCK_KEYWORDS = {"if", "for", "while", "switch", "catch"}


def _function_block_signatures(masked: str, opening: int, closing: int) -> list[str]:
    body = masked[opening + 1 : closing]
    signatures: list[str] = []
    for match in _FUNCTION_BLOCK_RE.finditer(body):
        if match.group("name") in _CONTROL_BLOCK_KEYWORDS:
            continue
        signature = re.sub(r"\s+", " ", body[match.start() : match.end() - 1]).strip()
        signatures.append(signature)
    return signatures


def _validate_function_span_structure(base_text: str, candidate_text: str, start: int, end: int) -> None:
    """Keep the approved function boundary fixed while permitting body edits."""

    base_masked = _mask_preprocessor_directives(_mask_c_noncode(base_text))
    candidate_masked = _mask_preprocessor_directives(_mask_c_noncode(candidate_text))
    base_offsets = _line_offsets(base_text)
    candidate_offsets = _line_offsets(candidate_text)
    if end >= len(base_offsets) or start < 1 or start >= len(base_offsets):
        raise CrackHarnessError("approved function span is outside the source")
    if start >= len(candidate_offsets):
        raise CrackHarnessError("candidate removes the approved function boundary")

    base_span_start = base_offsets[start - 1]
    base_span_end = base_offsets[end]
    candidate_span_start = candidate_offsets[start - 1]
    base_open = base_masked.find("{", base_span_start, base_span_end)
    candidate_open = candidate_masked.find("{", candidate_span_start)
    if base_open < 0 or candidate_open < 0:
        raise CrackHarnessError("approved function span does not contain a function body")
    base_close = _matching_brace(base_masked, base_open)
    candidate_close = _matching_brace(candidate_masked, candidate_open)
    if base_close is None or base_close >= base_span_end or candidate_close is None:
        raise CrackHarnessError("candidate changes the approved function boundary")
    if _contains_preprocessor_directive(
        candidate_text[candidate_span_start : candidate_close + 1]
    ):
        raise CrackHarnessError("candidate contains a preprocessor directive")

    base_prefix = "".join(base_masked[base_span_start:base_open].split())
    candidate_prefix = "".join(candidate_masked[candidate_span_start:candidate_open].split())
    if base_prefix != candidate_prefix:
        raise CrackHarnessError("candidate changes the approved function boundary")
    if candidate_text[candidate_close + 1 :] != base_text[base_close + 1 :]:
        raise CrackHarnessError("candidate changes the approved function boundary")

    base_signatures = _function_block_signatures(base_masked, base_open, base_close)
    remaining = list(base_signatures)
    for signature in _function_block_signatures(candidate_masked, candidate_open, candidate_close):
        if signature in remaining:
            remaining.remove(signature)
        else:
            raise CrackHarnessError("candidate injects a nested function boundary")


def _validate_admission_argv(root: Path, approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["precompile"]
    canonical_script = (root / "tools/candidate_compile_admission.py").resolve()
    if descriptor["script"] is None or descriptor["script"].resolve() != canonical_script:
        raise CrackHarnessError("canonical admission must use tools/candidate_compile_admission.py")
    expected = [
        str(descriptor["executable"]), str(canonical_script),
        "--root", str(root), "admit",
        "--owner", approval["owner"],
        "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--json",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("canonical admission argv is not the exact supported front-door shape")


def _validate_proof_adapter_argv(
    root: Path, approval: Mapping[str, Any], name: str,
) -> None:
    descriptor = approval["commands"][name]
    source_relative = approval["_paths"]["source"].relative_to(root).as_posix()
    expected = [
        str(descriptor["executable"]), "{CONTROLLER_ROOT}/tools/crack_harness.py",
        "proof-adapter", "--kind", name,
        "--owner", approval["owner"], "--function", approval["function"],
        "--candidate-source", f"{{RUN_ROOT}}/{source_relative}",
        "--candidate-source-sha256", approval["candidate"]["sha256"],
        "--approved-target-object-sha256", approval["target_sha256"],
        "--target-object", "{OUT_ROOT}/target.o",
        "--candidate-object", "{OUT_ROOT}/candidate.o",
        "--baseline-strict-report", "{OUT_ROOT}/baseline-strict.json",
        "--baseline-data-report", "{OUT_ROOT}/baseline-data.json",
        "--candidate-strict-report", "{OUT_ROOT}/candidate-strict.json",
        "--candidate-data-report", "{OUT_ROOT}/candidate-data.json",
        "--baseline-physical-receipt", "{OUT_ROOT}/baseline-physical.json",
        "--physical-receipt", "{OUT_ROOT}/physical.json",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError(
            f"commands.{name} argv is not the exact canonical proof-adapter shape"
        )


def _validate_record_descriptor(root: Path, approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["record"]
    expected = [
        str(descriptor["executable"]),
        str((root / "tools/candidate_compile_admission.py").resolve()),
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("commands.record must be the canonical record front door")


def _validate_compile_argv(approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["compile"]
    expected = [
        str(descriptor["executable"]), "{CONTROLLER_ROOT}/tools/crack_evidence_bundle.py",
        "--root", "{RUN_ROOT}", "--context", "{OUT_ROOT}/approval-context.json",
        "--out", "{OUT_ROOT}",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("commands.compile argv is not the exact two-phase evidence-bundle front door")


def _expand_argv(
    argv: Sequence[str], run_root: Path, out_root: Path, controller_root: Path,
) -> list[str]:
    expanded = [
        item.replace("{RUN_ROOT}", str(run_root)).replace("{OUT_ROOT}", str(out_root)).replace(
            "{CONTROLLER_ROOT}", str(controller_root)
        )
        for item in argv
    ]
    if any("{" in item or "}" in item for item in expanded):
        raise CrackHarnessError("reviewed argv contains an unsupported placeholder")
    for index, item in enumerate(expanded[2:], start=2):
        candidate = Path(item)
        path_like = (
            candidate.is_absolute() or item.startswith(("./", "../", ".\\", "..\\"))
            or "/../" in item or "\\..\\" in item
        )
        if not path_like:
            continue
        if not candidate.is_absolute() or not (
            _inside(run_root, candidate) or _inside(out_root, candidate)
        ):
            raise CrackHarnessError(
                f"reviewed argv[{index}] path escapes the disposable writable roots"
            )
    return expanded


def _proof_adapter_payload(
    *, kind: str, owner: str, function: str, candidate_source: Path,
    candidate_source_sha256: str, approved_target_object_sha256: str,
    target_object: Path, candidate_object: Path,
    baseline_strict_report: Path, baseline_data_report: Path,
    candidate_strict_report: Path, candidate_data_report: Path,
    baseline_physical_receipt: Path, physical_receipt: Path,
) -> dict[str, Any]:
    """Derive one closed proof from canonical, hash-bound objdiff artifacts."""
    from tools import focus_symbol_report as focus_report

    source_sha = _digest_file(candidate_source)
    if source_sha != _sha(candidate_source_sha256, "candidate_source_sha256"):
        raise CrackHarnessError("proof adapter candidate source hash mismatch")
    target_sha = _digest_file(target_object)
    if target_sha != _sha(approved_target_object_sha256, "approved_target_object_sha256"):
        raise CrackHarnessError("proof adapter target object is not the approved target")
    candidate_sha = _digest_file(candidate_object)
    baseline = focus_report.build_from_paths(
        strict_report_path=baseline_strict_report,
        data_report_path=baseline_data_report,
        function=function,
        expected_strict_report_sha256=_digest_file(baseline_strict_report),
        expected_data_report_sha256=_digest_file(baseline_data_report),
        physical_receipt_path=baseline_physical_receipt,
        expected_physical_receipt_sha256=_digest_file(baseline_physical_receipt),
        require_physical=False,
    )
    candidate = focus_report.build_from_paths(
        strict_report_path=candidate_strict_report,
        data_report_path=candidate_data_report,
        function=function,
        expected_strict_report_sha256=_digest_file(candidate_strict_report),
        expected_data_report_sha256=_digest_file(candidate_data_report),
        physical_receipt_path=physical_receipt,
        expected_physical_receipt_sha256=_digest_file(physical_receipt),
        require_physical=False,
    )
    report_sha = _sha(candidate.get("artifact_sha256"), "focus artifact_sha256")
    common = {
        "owner": owner, "function": function,
        "candidate_source_sha256": source_sha,
        "target_object_sha256": target_sha,
        "candidate_object_sha256": candidate_sha,
        "report_sha256": report_sha,
    }
    channels = candidate["channels"]
    if kind in {"strict", "data"}:
        metric = channels[kind]["metric"]
        prefix = "strict" if kind == "strict" else "data"
        return {
            **common, "schema": f"crack_proof_{kind}/v1",
            f"{prefix}_percent": metric["match_percent"],
            "target_bytes": metric["target_size"],
            "candidate_bytes": metric["candidate_size"],
            "differences": metric["diff_rows"],
        }
    if kind == "focus":
        return {
            **common, "schema": "crack_proof_focus/v1",
            "differing_rows": channels["strict"]["metric"]["diff_rows"],
        }
    if kind == "siblings":
        before = {
            (channel, _digest_json(identity))
            for channel in ("strict", "data")
            for identity in baseline["channels"][channel]["protected_siblings"]["exact_identities"]
        }
        after = {
            (channel, _digest_json(identity))
            for channel in ("strict", "data")
            for identity in channels[channel]["protected_siblings"]["exact_identities"]
        }
        return {
            **common, "schema": "crack_proof_siblings/v1",
            "protected_total": len(before), "protected_losses": len(before - after),
        }
    if kind == "physical":
        physical = candidate["physical_relocations"]
        differences = physical.get("physical_relocation_differences", [])
        return {
            **common, "schema": "crack_proof_physical/v1",
            "target_count": physical.get("target", {}).get("physical_relocation_count", 0),
            "candidate_count": physical.get("candidate", {}).get("physical_relocation_count", 0),
            "differences": len(differences),
        }
    if kind == "assess":
        before = baseline["channels"]["strict"]["metric"]["match_percent"]
        after = channels["strict"]["metric"]["match_percent"]
        data_before = baseline["channels"]["data"]["metric"]["match_percent"]
        data_after = channels["data"]["metric"]["match_percent"]
        data_diff_before = baseline["channels"]["data"]["metric"]["diff_rows"]
        data_diff_after = channels["data"]["metric"]["diff_rows"]
        baseline_physical = baseline["physical_relocations"]
        candidate_physical = candidate["physical_relocations"]

        def physical_distance(value: Mapping[str, Any]) -> int:
            target_count = int(
                value.get("target", {}).get("physical_relocation_count", 0)
            )
            candidate_count = int(
                value.get("candidate", {}).get("physical_relocation_count", 0)
            )
            differences = value.get("physical_relocation_differences", [])
            if not isinstance(differences, list):
                raise CrackHarnessError(
                    "focus artifact physical differences are invalid"
                )
            return abs(target_count - candidate_count) + len(differences)

        return {
            "schema": "crack_assessment/v1", "owner": owner,
            "function": function, "candidate_source_sha256": source_sha,
            "target_object_sha256": target_sha,
            "candidate_object_sha256": candidate_sha,
            "owner_gain": float(after) - float(before),
            "data_gain": float(data_after) - float(data_before),
            "data_diff_delta": int(data_diff_after) - int(data_diff_before),
            "physical_diff_delta": (
                physical_distance(candidate_physical)
                - physical_distance(baseline_physical)
            ),
        }
    raise CrackHarnessError(f"unsupported proof adapter kind: {kind}")


EVIDENCE_BASELINE_FILES = (
    "target.o", "baseline-candidate.o", "baseline-strict.json", "baseline-data.json",
    "baseline-physical.json", "baseline-receipt.json",
)
EVIDENCE_CANDIDATE_FILES = (
    "candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json",
    "candidate-receipt.json",
)


def _clear_evidence(out_root: Path, names: Sequence[str]) -> None:
    for name in names:
        path = out_root / name
        if path.exists():
            if not path.is_file() or path.is_symlink():
                raise CrackHarnessError(f"evidence output is not a plain file: {path}")
            path.unlink()


def _evidence_hashes(out_root: Path, names: Sequence[str]) -> dict[str, str]:
    result = {}
    for name in names:
        path = out_root / name
        if not path.is_file() or path.is_symlink():
            raise CrackHarnessError(f"evidence bundle omitted plain output: {name}")
        result[name] = _digest_file(path)
    return result


def _validate_evidence_descriptor(
    descriptor: Any, path: Path, label: str,
) -> None:
    if not isinstance(descriptor, Mapping) or set(descriptor) != {"sha256", "size_bytes"}:
        raise CrackHarnessError(f"{label} descriptor is invalid")
    if (
        _sha(descriptor.get("sha256"), f"{label}.sha256") != _digest_file(path)
        or not isinstance(descriptor.get("size_bytes"), int)
        or isinstance(descriptor.get("size_bytes"), bool)
        or descriptor["size_bytes"] != path.stat().st_size
    ):
        raise CrackHarnessError(f"{label} descriptor does not bind its artifact")


def _validate_evidence_receipt(
    out_root: Path, approval: Mapping[str, Any], context: Mapping[str, Any], phase: str,
) -> dict[str, Any]:
    path = out_root / f"{phase}-receipt.json"
    receipt = _read_json(path)
    if not isinstance(receipt, Mapping):
        raise CrackHarnessError(f"{phase} evidence receipt is invalid")
    unsigned = dict(receipt)
    digest = unsigned.pop("receipt_sha256", None)
    if digest != _digest_json(unsigned):
        raise CrackHarnessError(f"{phase} evidence receipt digest is invalid")
    expected = {
        "schema": "crack_evidence_phase_receipt/v1", "phase": phase,
        "owner": approval["owner"], "function": approval["function"],
        "unit": approval["unit"],
        "source_relpath": approval["_paths"]["source"].relative_to(approval["_root"]).as_posix(),
        "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "approval_context_sha256": context["context_sha256"],
        "phase_nonce": _digest_bytes((context["context_sha256"] + ":" + phase).encode()),
        "issued_at": approval["issued_at"], "authority_advanced": False,
    }
    for key, value in expected.items():
        if receipt.get(key) != value:
            raise CrackHarnessError(f"{phase} evidence receipt {key} is not approval-bound")
    artifacts = receipt.get("artifacts")
    required = (
        {
            "target.o", "baseline-candidate.o", "baseline-strict.json",
            "baseline-data.json", "baseline-physical.json",
        }
        if phase == "baseline" else
        {"target.o", "candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json"}
    )
    if not isinstance(artifacts, Mapping) or set(artifacts) != required:
        raise CrackHarnessError(f"{phase} evidence receipt artifact set is invalid")
    for name, descriptor in artifacts.items():
        _validate_evidence_descriptor(descriptor, out_root / name, f"{phase}.{name}")
    if not isinstance(receipt.get("tools"), Mapping) or not receipt["tools"]:
        raise CrackHarnessError(f"{phase} evidence receipt lacks pinned tool descriptors")
    return dict(receipt)


def _validate_evidence_context(
    out_root: Path, approval: Mapping[str, Any], context: Mapping[str, Any], *, completed: bool,
) -> dict[str, Any]:
    value = _read_json(out_root / "evidence-context.json")
    if not isinstance(value, Mapping):
        raise CrackHarnessError("evidence context is invalid")
    unsigned = dict(value)
    digest = unsigned.pop("evidence_context_sha256", None)
    if digest != _digest_json(unsigned):
        raise CrackHarnessError("evidence context digest is invalid")
    expected = {
        "schema": "crack_evidence_bundle_context/v1", "owner": approval["owner"],
        "function": approval["function"], "unit": approval["unit"],
        "source_relpath": approval["_paths"]["source"].relative_to(approval["_root"]).as_posix(),
        "target_sha256": approval["target_sha256"], "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "approval_context_sha256": context["context_sha256"], "authority_advanced": False,
        "phase_nonces": {
            phase: _digest_bytes((context["context_sha256"] + ":" + phase).encode())
            for phase in ("baseline", "candidate")
        },
    }
    for key, expected_value in expected.items():
        if value.get(key) != expected_value:
            raise CrackHarnessError(f"evidence context {key} is not approval-bound")
    _validate_evidence_descriptor(value.get("baseline_receipt"), out_root / "baseline-receipt.json", "baseline receipt")
    if completed:
        if value.get("completed") is not True:
            raise CrackHarnessError("candidate evidence context is not complete")
        _validate_evidence_descriptor(value.get("candidate_receipt"), out_root / "candidate-receipt.json", "candidate receipt")
    elif "candidate_receipt" in value or "completed" in value:
        raise CrackHarnessError("baseline evidence context claims candidate completion")
    return dict(value)


def _create_disposable_worktree(root: Path, destination: Path, base_commit: str) -> None:
    root = Path(os.path.abspath(root))
    destination = Path(os.path.abspath(destination))
    _assert_no_indirection(root)
    if not _inside(root, destination):
        raise CrackHarnessError(f"disposable worktree escapes repository root: {destination}")
    try:
        run_dir = destination.parent.parent
        state = _state_from_run_dir(run_dir)
        if not _inside(root, state):
            raise CrackHarnessError(f"disposable worktree state escapes repository root: {state}")
        _state_run_dir(state, run_dir)
    except (IndexError, ValueError) as exc:
        raise CrackHarnessError(
            f"disposable worktree does not use the exact latest state layout: {destination}"
        ) from exc
    if destination.name != "worktree" or destination.parent.name != "temp":
        raise CrackHarnessError(f"disposable worktree path is not under latest/temp: {destination}")
    _assert_no_indirection(destination.parent)
    _assert_no_indirection(destination, missing_leaf=True)
    if destination.exists():
        raise CrackHarnessError(f"disposable worktree destination already exists: {destination}")
    _safe_mkdir(destination.parent)
    completed = subprocess.run(
        ["git", "worktree", "add", "--detach", str(destination), base_commit],
        cwd=root, text=True, capture_output=True, check=False,
    )
    if completed.returncode != 0:
        raise CrackHarnessError(f"cannot create disposable worktree: {completed.stderr.strip()}")


def _remove_disposable_worktree(root: Path, destination: Path) -> None:
    root = Path(os.path.abspath(root))
    destination = Path(os.path.abspath(destination))
    _assert_no_indirection(root)
    if not _inside(root, destination):
        raise CrackHarnessError(f"disposable worktree escapes repository root: {destination}")
    if destination.name != "worktree" or destination.parent.name != "temp":
        raise CrackHarnessError(f"disposable worktree path is not under latest/temp: {destination}")
    run_dir = destination.parent.parent
    state = _state_from_run_dir(run_dir)
    if not _inside(root, state):
        raise CrackHarnessError(f"disposable worktree state escapes repository root: {state}")
    _state_run_dir(state, run_dir)
    _assert_no_indirection(destination, missing_leaf=True)
    completed = subprocess.run(
        ["git", "worktree", "remove", "--force", str(destination)],
        cwd=root, text=True, capture_output=True, check=False,
    )
    subprocess.run(
        ["git", "worktree", "prune"], cwd=root,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
    )
    if completed.returncode != 0 or destination.exists():
        raise CrackHarnessError(
            f"disposable worktree cleanup failed; global STOP remains: {destination}"
        )


def _sign_attempt_receipt(
    root: Path, body: Mapping[str, Any], *, manager_key_path: Path,
    expected_key_id: str,
) -> dict[str, Any]:
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("attempt manager key is invalid")
    signed_body = {**dict(body), "key_id": expected_key_id}
    signed = {
        **signed_body,
        "signature": hmac.new(
            secret, _canonical(signed_body), hashlib.sha256
        ).hexdigest(),
    }
    return {**signed, "attempt_sha256": _digest_json(signed)}


def _validate_attempt_receipt(
    root: Path, value: Any, *, manager_key_path: Path,
    expected_key_id: str,
) -> dict[str, Any]:
    required = {
        "schema", "run_dir", "source_path", "approval_path",
        "approval_sha256", "disposable_paths", "key_id", "signature",
        "attempt_sha256",
    }
    if not isinstance(value, Mapping) or set(value) != required:
        raise CrackHarnessError("attempt receipt is not a strict closed object")
    receipt = dict(value)
    digest = receipt.pop("attempt_sha256", None)
    if digest != _digest_json(receipt):
        raise CrackHarnessError("attempt receipt integrity failed")
    signature = receipt.pop("signature", None)
    if receipt.get("schema") != ATTEMPT_SCHEMA:
        raise CrackHarnessError("attempt receipt schema is invalid")
    if receipt.get("key_id") != expected_key_id:
        raise CrackHarnessError("attempt receipt key_id is invalid")
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("attempt manager key is invalid")
    expected_signature = hmac.new(
        secret, _canonical(receipt), hashlib.sha256
    ).hexdigest()
    if not isinstance(signature, str) or not hmac.compare_digest(
        signature, expected_signature
    ):
        raise CrackHarnessError("attempt receipt signature is invalid")
    return dict(value)


FRONTIER_BODY_FIELDS = {
    "schema", "owner", "task_id", "function", "campaign_id", "base_commit",
    "source_relpath", "base_sha256", "candidate_sha256",
    "target_object_sha256", "candidate_object_sha256", "approval_sha256",
    "expected_terminal", "predicted_rows_sha256", "strict_percent",
    "strict_target_bytes", "strict_candidate_bytes", "strict_differences",
    "data_percent", "data_target_bytes", "data_candidate_bytes",
    "data_differences", "focus_differing_rows", "protected_total",
    "protected_losses", "physical_target_count", "physical_candidate_count",
    "physical_differences", "owner_gain", "data_gain", "data_diff_delta",
    "physical_diff_delta", "parent_frontier_sha256", "authority_advanced",
    "retained_at", "key_id",
}


def _frontier_file(run_dir: Path) -> Path:
    return run_dir.parent / "latest-frontier.json"


def _frontier_pending_file(run_dir: Path) -> Path:
    return run_dir.parent / "frontier.pending.json"


def _validate_frontier(
    root: Path, path: Path, *, manager_key_path: Path,
    expected_key_id: str, require_live_source: bool = True,
    approval: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Validate one compact manager-signed partial frontier."""

    root = Path(os.path.abspath(root))
    path = Path(os.path.abspath(path))
    _assert_no_indirection(root)
    _assert_no_indirection(path)
    if not path.is_file() or not _inside(root, path):
        raise CrackHarnessError("partial frontier path is not a contained file")
    value = _read_json(path)
    if not isinstance(value, Mapping):
        raise CrackHarnessError("partial frontier is not an object")
    unsigned = dict(value)
    frontier_sha256 = unsigned.pop("frontier_sha256", None)
    if frontier_sha256 != _digest_json(unsigned):
        raise CrackHarnessError("partial frontier digest is invalid")
    signature = unsigned.pop("signature", None)
    if set(unsigned) != FRONTIER_BODY_FIELDS:
        raise CrackHarnessError("partial frontier is not a strict closed object")
    if (
        unsigned.get("schema") != FRONTIER_SCHEMA
        or unsigned.get("authority_advanced") is not False
        or unsigned.get("expected_terminal") not in {"exact", "improved"}
        or unsigned.get("key_id") != expected_key_id
    ):
        raise CrackHarnessError("partial frontier identity is invalid")
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("partial frontier manager key is invalid")
    expected_signature = hmac.new(
        secret, _canonical(unsigned), hashlib.sha256
    ).hexdigest()
    if not isinstance(signature, str) or not hmac.compare_digest(
        signature, expected_signature
    ):
        raise CrackHarnessError("partial frontier signature is invalid")
    for key in (
        "base_sha256", "candidate_sha256", "target_object_sha256",
        "candidate_object_sha256", "approval_sha256", "predicted_rows_sha256",
    ):
        _sha(unsigned.get(key), f"partial frontier {key}")
    parent = unsigned.get("parent_frontier_sha256")
    if parent is not None:
        _sha(parent, "partial frontier parent_frontier_sha256")
    for key in (
        "strict_percent", "data_percent", "owner_gain", "data_gain",
    ):
        value_number = unsigned.get(key)
        if (
            isinstance(value_number, bool)
            or not isinstance(value_number, (int, float))
            or not math.isfinite(float(value_number))
            or float(value_number) < 0
        ):
            raise CrackHarnessError(f"partial frontier {key} is invalid")
    nonnegative_integer_fields = (
        "strict_target_bytes", "strict_candidate_bytes", "strict_differences",
        "data_target_bytes", "data_candidate_bytes", "data_differences",
        "focus_differing_rows", "protected_total", "protected_losses",
        "physical_target_count", "physical_candidate_count",
        "physical_differences",
    )
    for key in nonnegative_integer_fields:
        number = unsigned.get(key)
        if (
            isinstance(number, bool)
            or not isinstance(number, int)
            or number < 0
        ):
            raise CrackHarnessError(f"partial frontier {key} is invalid")
    for key in ("data_diff_delta", "physical_diff_delta"):
        number = unsigned.get(key)
        if isinstance(number, bool) or not isinstance(number, int):
            raise CrackHarnessError(f"partial frontier {key} is invalid")
    if (
        float(unsigned["owner_gain"]) <= 0
        or float(unsigned["data_gain"]) < 0
        or unsigned["data_diff_delta"] > 0
        or unsigned["physical_diff_delta"] > 0
        or unsigned["protected_losses"] != 0
        or unsigned["data_target_bytes"] != unsigned["data_candidate_bytes"]
    ):
        raise CrackHarnessError("partial frontier does not prove a safe gain")
    source = _bound_path(
        root, unsigned.get("source_relpath"), "partial frontier source"
    )
    if not _is_tracked(root, source):
        raise CrackHarnessError("partial frontier source is not tracked")
    try:
        state = path.parents[3]
    except IndexError as exc:
        raise CrackHarnessError("partial frontier directory binding is invalid") from exc
    expected_function_dir = _run_dir(state, unsigned).parent
    if path.parent != expected_function_dir:
        raise CrackHarnessError("partial frontier is stored under the wrong function")
    if require_live_source and _digest_file(source) != unsigned["candidate_sha256"]:
        raise CrackHarnessError("partial frontier does not match the live source")
    if approval is not None:
        expected = {
            "owner": approval["owner"], "task_id": approval["task_id"],
            "function": approval["function"],
            "base_commit": approval["base_commit"],
            "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
            "base_sha256": approval["base"]["sha256"],
            "candidate_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": approval["target_sha256"],
            "approval_sha256": approval["_approval_sha256"],
            "expected_terminal": approval["selection"]["expected_terminal"],
            "predicted_rows_sha256": _digest_json(approval["predicted_rows"]),
        }
        if any(unsigned.get(key) != expected_value for key, expected_value in expected.items()):
            raise CrackHarnessError("partial frontier is not approval-bound")
    _timestamp(unsigned.get("retained_at"), "partial frontier retained_at")
    return dict(value)


def _validate_frontier_continuation(
    root: Path, run_dir: Path, frontier: Mapping[str, Any],
    approval: Mapping[str, Any],
) -> None:
    expected = {
        "owner": approval["owner"],
        "task_id": approval["task_id"],
        "function": approval["function"],
        "base_commit": approval["base_commit"],
        "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
        "target_object_sha256": approval["target_sha256"],
        "candidate_sha256": approval["base"]["sha256"],
    }
    if any(frontier.get(key) != value for key, value in expected.items()):
        raise CrackHarnessError(
            "approved base is stale relative to the retained partial frontier"
        )


def _sign_frontier(
    root: Path, run_dir: Path, approval: Mapping[str, Any],
    object_pair: tuple[str, str], proof_payloads: Mapping[str, Mapping[str, Any]],
    assessment: Mapping[str, Any], *, manager_key_path: Path,
    expected_key_id: str,
) -> dict[str, Any]:
    previous_path = _frontier_file(run_dir)
    parent_sha256 = None
    if previous_path.is_file():
        previous = _validate_frontier(
            root, previous_path, manager_key_path=manager_key_path,
            expected_key_id=expected_key_id, require_live_source=True,
        )
        parent_sha256 = previous["frontier_sha256"]
        _validate_frontier_continuation(root, run_dir, previous, approval)
    strict = proof_payloads["strict"]
    data = proof_payloads["data"]
    focus = proof_payloads["focus"]
    siblings = proof_payloads["siblings"]
    physical = proof_payloads["physical"]
    body = {
        "schema": FRONTIER_SCHEMA,
        "owner": approval["owner"], "task_id": approval["task_id"],
        "function": approval["function"],
        "campaign_id": approval["campaign"]["id"],
        "base_commit": approval["base_commit"],
        "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
        "base_sha256": approval["base"]["sha256"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "target_object_sha256": object_pair[0],
        "candidate_object_sha256": object_pair[1],
        "approval_sha256": approval["_approval_sha256"],
        "expected_terminal": approval["selection"]["expected_terminal"],
        "predicted_rows_sha256": _digest_json(approval["predicted_rows"]),
        "strict_percent": strict["strict_percent"],
        "strict_target_bytes": strict["target_bytes"],
        "strict_candidate_bytes": strict["candidate_bytes"],
        "strict_differences": strict["differences"],
        "data_percent": data["data_percent"],
        "data_target_bytes": data["target_bytes"],
        "data_candidate_bytes": data["candidate_bytes"],
        "data_differences": data["differences"],
        "focus_differing_rows": focus["differing_rows"],
        "protected_total": siblings["protected_total"],
        "protected_losses": siblings["protected_losses"],
        "physical_target_count": physical["target_count"],
        "physical_candidate_count": physical["candidate_count"],
        "physical_differences": physical["differences"],
        "owner_gain": assessment["owner_gain"],
        "data_gain": assessment["data_gain"],
        "data_diff_delta": assessment["data_diff_delta"],
        "physical_diff_delta": assessment["physical_diff_delta"],
        "parent_frontier_sha256": parent_sha256,
        "authority_advanced": False, "retained_at": _now(),
        "key_id": expected_key_id,
    }
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("partial frontier manager key is invalid")
    signed = {
        **body,
        "signature": hmac.new(secret, _canonical(body), hashlib.sha256).hexdigest(),
    }
    return {**signed, "frontier_sha256": _digest_json(signed)}


def _recover_pending_frontiers(
    root: Path, state: Path, *, manager_key_path: Path,
    expected_key_id: str,
) -> None:
    if not state.exists():
        return
    for pending in _state_glob(
        state, "owners/*/*/frontier.pending.json", "pending partial frontier"
    ):
        frontier = _validate_frontier(
            root, pending, manager_key_path=manager_key_path,
            expected_key_id=expected_key_id, require_live_source=False,
        )
        source = _bound_path(
            root, frontier["source_relpath"], "pending partial frontier source"
        )
        source_sha256 = _digest_file(source)
        if source_sha256 == frontier["base_sha256"]:
            pending.unlink()
            continue
        if source_sha256 != frontier["candidate_sha256"]:
            raise CrackHarnessError(
                "pending partial frontier source is neither base nor candidate"
            )
        destination = pending.parent / "latest-frontier.json"
        os.replace(pending, destination)
        _directory_fsync(destination.parent)
        _validate_frontier(
            root, destination, manager_key_path=manager_key_path,
            expected_key_id=expected_key_id, require_live_source=True,
        )


def _seal_root_cleanup_receipt(
    root: Path, state: Path, attempt: Mapping[str, Any],
    approval: Mapping[str, Any], *, manager_key_path: Path,
    expected_key_id: str, required_exact: bool = False,
) -> bool:
    """Sign the approved root-disposable manifest before approval deletion."""

    root = Path(os.path.abspath(root))
    state = Path(os.path.abspath(state))
    attempt = _validate_attempt_receipt(
        root, attempt, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    attempt_sha256 = attempt["attempt_sha256"]
    run_dir = _state_run_dir(
        state, Path(os.path.abspath(str(attempt.get("run_dir")))),
        "root cleanup run path",
    )
    raw_paths = attempt.get("disposable_paths")
    if not isinstance(raw_paths, list) or len(raw_paths) != 4:
        raise CrackHarnessError("root cleanup disposable set is invalid")
    paths = [
        _bound_path(root, str(raw), f"root cleanup disposable {index}", exists=False)
        for index, raw in enumerate(raw_paths)
    ]
    if len(set(paths)) != 4:
        raise CrackHarnessError("root cleanup disposable set is invalid")
    if (
        paths[0] != approval["_paths"]["base"]
        or paths[1] != approval["_paths"]["candidate"]
        or paths[3] != approval["_approval_path"]
    ):
        raise CrackHarnessError("root cleanup paths do not match the approval")
    if (
        attempt.get("approval_sha256") != approval["_approval_sha256"]
        or _digest_file(approval["_approval_path"]) != approval["_approval_sha256"]
    ):
        raise CrackHarnessError("root cleanup pre-approval state is invalid")
    result_path = _state_path(
        state, run_dir / "result.json", "root cleanup terminal result", exists=None,
    )
    if not result_path.exists():
        if required_exact:
            raise CrackHarnessError("root cleanup exact result is missing")
        return False
    if not result_path.is_file():
        raise CrackHarnessError("root cleanup terminal result is not a file")
    result = _read_json(result_path)
    if (
        not isinstance(result, Mapping)
        or result.get("status") != "exact"
        or result.get("approval_sha256") != attempt.get("approval_sha256")
    ):
        raise CrackHarnessError("root cleanup receipt lacks a bound exact result")
    terminal_binding = _terminal_binding_from_result(run_dir, result)
    if (
        terminal_binding is None
        or not _valid_terminal_result(
            root, result_path, run_dir / "record.commit.json",
            terminal_binding, approval, central_required=False,
        )
    ):
        raise CrackHarnessError(
            "root cleanup receipt lacks a complete locally authenticated exact terminal"
        )
    receipt_path = run_dir / "root-cleanup.receipt.json"
    if receipt_path.exists():
        if not _valid_root_cleanup_receipt(
            root, state, run_dir, result,
            manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
            require_absent=False,
        ):
            raise CrackHarnessError(
                "existing root cleanup manifest is invalid"
            )
        receipt = _read_json(receipt_path)
        binding = receipt.get("approval_binding") if isinstance(receipt, Mapping) else None
        if (
            not isinstance(binding, Mapping)
            or binding.get("permit_path") != str(paths[2])
        ):
            raise CrackHarnessError(
                "root cleanup permit path is not manager-authenticated"
            )
        return True
    expected_hashes = (
        approval["base"]["sha256"], approval["candidate"]["sha256"],
        approval["permit_sha256"], approval["_approval_sha256"],
    )
    for path_item, expected_hash in zip(paths, expected_hashes, strict=True):
        if (
            not path_item.is_file()
            or _is_tracked(root, path_item)
            or _digest_file(path_item) != expected_hash
        ):
            raise CrackHarnessError(
                "root cleanup manifest must be sealed before disposable deletion"
            )
    disposables = [
        {
            "role": "base", "path": str(paths[0]),
            "sha256": approval["base"]["sha256"],
        },
        {
            "role": "candidate", "path": str(paths[1]),
            "sha256": approval["candidate"]["sha256"],
        },
        {
            "role": "permit", "path": str(paths[2]),
            "sha256": approval["permit_sha256"],
        },
        {
            "role": "approval", "path": str(paths[3]),
            "sha256": approval["_approval_sha256"],
        },
    ]
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("root cleanup manager key is invalid")
    receipt_body = {
        "schema": ROOT_CLEANUP_RECEIPT_SCHEMA,
        "run_dir": str(run_dir),
        "approval_sha256": _sha(
            attempt.get("approval_sha256"), "root cleanup approval_sha256"
        ),
        "attempt_sha256": _sha(attempt_sha256, "root cleanup attempt_sha256"),
        "approval_binding": {
            "approval_id": approval["approval_id"],
            "approval_identity_sha256": approval["_permit_identity_sha256"],
            "approval_sha256": approval["_approval_sha256"],
            "approval_path": str(approval["_approval_path"]),
            "owner": approval["owner"],
            "task_id": approval["task_id"],
            "function": approval["function"],
            "campaign_id": approval["campaign"]["id"],
            "predicted_rows_sha256": _digest_json(approval["predicted_rows"]),
            "base_commit": approval["base_commit"],
            "source_path": str(approval["_paths"]["source"]),
            "source_sha256": approval["candidate"]["sha256"],
            "base_path": str(approval["_paths"]["base"]),
            "base_sha256": approval["base"]["sha256"],
            "candidate_path": str(approval["_paths"]["candidate"]),
            "candidate_sha256": approval["candidate"]["sha256"],
            "permit_path": str(paths[2]),
            "permit_sha256": approval["permit_sha256"],
        },
        "disposables": disposables,
        "root_disposable_manifest_authenticated": True,
        "sealed_at": _now(),
        "key_id": expected_key_id,
    }
    signed = {
        **receipt_body,
        "signature": hmac.new(
            secret, _canonical(receipt_body), hashlib.sha256
        ).hexdigest(),
    }
    _atomic_json(
        run_dir / "root-cleanup.receipt.json",
        {**signed, "cleanup_sha256": _digest_json(signed)},
    )
    return True


def _valid_root_cleanup_receipt(
    root: Path, state: Path, run_dir: Path, result: Mapping[str, Any], *,
    manager_key_path: Path, expected_key_id: str, require_absent: bool = True,
) -> bool:
    path = _state_path(
        state, run_dir / "root-cleanup.receipt.json",
        "root cleanup receipt", exists=None,
    )
    if path.exists() and not path.is_file():
        return False
    if not path.is_file():
        return False
    value = _read_json(path)
    if not isinstance(value, Mapping):
        return False
    body = dict(value)
    digest = body.pop("cleanup_sha256", None)
    signed = dict(body)
    signature = signed.pop("signature", None)
    if (
        set(signed) != {
            "schema", "run_dir", "approval_sha256", "attempt_sha256",
            "approval_binding", "disposables",
            "root_disposable_manifest_authenticated", "sealed_at", "key_id",
        }
        or signed.get("schema") != ROOT_CLEANUP_RECEIPT_SCHEMA
        or digest != _digest_json(body)
        or signed.get("run_dir") != str(run_dir)
        or signed.get("approval_sha256") != result.get("approval_sha256")
        or signed.get("attempt_sha256") != result.get("attempt_sha256")
        or signed.get("root_disposable_manifest_authenticated") is not True
        or signed.get("key_id") != expected_key_id
    ):
        return False
    try:
        _sha(signed.get("attempt_sha256"), "cleanup attempt_sha256")
        _timestamp(signed.get("sealed_at"), "cleanup sealed_at")
        key_path = _manager_key_file(root, manager_key_path)
        secret = key_path.read_bytes()
        if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
            return False
        expected_signature = hmac.new(
            secret, _canonical(signed), hashlib.sha256
        ).hexdigest()
        if not isinstance(signature, str) or not hmac.compare_digest(
            signature, expected_signature
        ):
            return False
        disposables = signed.get("disposables")
        if not isinstance(disposables, list) or len(disposables) != 4:
            return False
        approval_binding = signed.get("approval_binding")
        required_binding = {
            "approval_id", "approval_identity_sha256", "approval_sha256",
            "approval_path", "owner", "task_id", "function", "campaign_id",
            "predicted_rows_sha256", "base_commit", "source_path",
            "source_sha256", "base_path", "base_sha256", "candidate_path",
            "candidate_sha256", "permit_path", "permit_sha256",
        }
        if not isinstance(approval_binding, Mapping) or set(approval_binding) != required_binding:
            return False
        for key in (
            "approval_identity_sha256", "approval_sha256", "source_sha256",
            "base_sha256", "candidate_sha256", "permit_sha256",
            "predicted_rows_sha256",
        ):
            _sha(approval_binding.get(key), f"cleanup approval binding {key}")
        expected_result = {
            "approval_id": approval_binding["approval_id"],
            "approval_sha256": approval_binding["approval_sha256"],
            "owner": approval_binding["owner"],
            "task_id": approval_binding["task_id"],
            "function": approval_binding["function"],
            "campaign_id": approval_binding["campaign_id"],
            "base_commit": approval_binding["base_commit"],
            "base_sha256": approval_binding["base_sha256"],
            "candidate_sha256": approval_binding["candidate_sha256"],
        }
        if any(result.get(key) != expected for key, expected in expected_result.items()):
            return False
        if _digest_json(result.get("predicted_rows")) != approval_binding[
            "predicted_rows_sha256"
        ]:
            return False
        if run_dir != _run_dir(state, result):
            return False
        source_path = _bound_path(
            root, approval_binding["source_path"], "cleanup retained source",
            exists=True,
        )
        if (
            not source_path.is_file()
            or not _is_tracked(root, source_path)
            or _digest_file(source_path) != approval_binding["source_sha256"]
            or approval_binding["source_sha256"]
            != approval_binding["candidate_sha256"]
        ):
            return False
        roles = ("base", "candidate", "permit", "approval")
        paths: list[Path] = []
        hashes: dict[str, str] = {}
        for item, role in zip(disposables, roles, strict=True):
            if not isinstance(item, Mapping) or set(item) != {"role", "path", "sha256"}:
                return False
            if item.get("role") != role:
                return False
            path_item = _bound_path(
                root, item.get("path"), f"cleanup {role} path", exists=False,
            )
            if require_absent:
                if path_item.exists():
                    return False
            elif path_item.exists() and (
                not path_item.is_file()
                or _digest_file(path_item) != item.get("sha256")
            ):
                return False
            paths.append(path_item)
            hashes[role] = _sha(item.get("sha256"), f"cleanup {role} sha256")
        if len(set(paths)) != 4:
            return False
        expected_paths = (
            Path(os.path.abspath(str(approval_binding["base_path"]))),
            Path(os.path.abspath(str(approval_binding["candidate_path"]))),
            Path(os.path.abspath(str(approval_binding["permit_path"]))),
            Path(os.path.abspath(str(approval_binding["approval_path"]))),
        )
        if tuple(paths) != expected_paths:
            return False
        if (
            hashes["approval"] != result.get("approval_sha256")
            or hashes["base"] != result.get("base_sha256")
            or hashes["candidate"] != result.get("candidate_sha256")
            or hashes["permit"] != approval_binding["permit_sha256"]
        ):
            return False
    except (CrackHarnessError, ValueError):
        return False
    return True


def _scavenge_disposable_worktrees(
    root: Path, state: Path, *, manager_key_path: Path,
    expected_key_id: str,
) -> None:
    root = Path(os.path.abspath(root))
    state = Path(os.path.abspath(state))
    _assert_no_indirection(root)
    _assert_no_indirection(state)
    for path in _state_glob(
        state, "owners/*/*/latest/temp/worktree", "abandoned worktree"
    ):
        if path.exists():
            _remove_disposable_worktree(root, path)
    for name in ("temp", "raw", "logs", "out"):
        for path in _state_glob(
            state, f"owners/*/*/latest/{name}", f"abandoned {name} path"
        ):
            if path.exists():
                _assert_no_indirection(path)
                shutil.rmtree(path)
    attempt = _state_path(state, state / "attempt.json", "abandoned attempt", exists=None)
    if attempt.is_file():
        _assert_no_indirection(attempt)
        value = _read_json(attempt)
        value = _validate_attempt_receipt(
            root, value, manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
        )
        run_dir = _state_run_dir(
            state, Path(os.path.abspath(str(value["run_dir"]))),
            "abandoned attempt run path",
        )
        source = _bound_path(root, value["source_path"], "abandoned approved source")
        if not _is_tracked(root, source):
            raise CrackHarnessError("abandoned approved source is not tracked")
        raw_paths = value["disposable_paths"]
        if not isinstance(raw_paths, list) or len(raw_paths) != 4:
            raise CrackHarnessError("abandoned disposable set is invalid")
        paths = {Path(os.path.abspath(str(raw))) for raw in raw_paths}
        if len(paths) != 4 or source in paths:
            raise CrackHarnessError("abandoned disposable set is invalid")
        for path in paths:
            if not _inside(root, path):
                raise CrackHarnessError("abandoned disposable path escapes repository")
            _assert_no_indirection(path, missing_leaf=not path.exists())
        approval_path = _bound_path(
            root, value["approval_path"], "abandoned approval", exists=False
        )
        if approval_path not in paths:
            raise CrackHarnessError("abandoned approval is outside the disposable set")
        if not approval_path.exists():
            # Approval is deliberately deleted last.  Its absence therefore
            # proves that every earlier root disposable was already removed;
            # only a failed attempt-receipt unlink may remain.
            if any(path.exists() for path in paths):
                raise CrackHarnessError(
                    "abandoned approval is missing while another disposable remains"
                )
            result_path = run_dir / "result.json"
            if result_path.is_file():
                result = _read_json(result_path)
                if (
                    isinstance(result, Mapping)
                    and result.get("status") == "exact"
                    and not _valid_root_cleanup_receipt(
                        root, state, run_dir, result,
                        manager_key_path=manager_key_path,
                        expected_key_id=expected_key_id,
                    )
                ):
                    raise CrackHarnessError(
                        "abandoned exact cleanup lacks manager-authenticated path proof"
                    )
            _safe_unlink(attempt)
            return
        if _digest_file(approval_path) != _sha(
            value["approval_sha256"], "abandoned approval_sha256"
        ):
            raise CrackHarnessError("abandoned approval hash drifted")
        approval_value = load_approval(
            root, approval_path, allow_applied_source=True,
            recovery_cleanup=True,
        )
        expected_paths = {
            _bound_path(
                root, approval_value[name]["path"], f"abandoned {name}",
                exists=False,
            )
            for name in ("base", "candidate")
        } | {approval_path}
        permit_paths = paths - expected_paths
        if len(permit_paths) != 1 or paths != expected_paths | permit_paths:
            raise CrackHarnessError("abandoned disposables do not match the approved exact set")
        permit_path = next(iter(permit_paths))
        if permit_path.exists() and _digest_file(permit_path) != approval_value.get(
            "permit_sha256"
        ):
            raise CrackHarnessError("abandoned permit hash drifted")
        # Exact retention additionally seals a durable cleanup manifest.  For
        # failed/no-gain attempts the manager-signed attempt receipt already
        # authenticates the permit path and all other disposables.
        terminal_path = run_dir / "result.json"
        terminal_value = _read_json(terminal_path) if terminal_path.is_file() else None
        if isinstance(terminal_value, Mapping) and terminal_value.get("status") == "exact":
            _seal_root_cleanup_receipt(
                root, state, value, approval_value,
                manager_key_path=manager_key_path,
                expected_key_id=expected_key_id,
            )
        for path in (
            *(
                _bound_path(
                    root, approval_value[name]["path"], f"abandoned {name}",
                    exists=False,
                )
                for name in ("base", "candidate")
            ),
            permit_path,
        ):
            if path.exists():
                if _is_tracked(root, path):
                    raise CrackHarnessError(f"abandoned disposable is tracked: {path}")
                _safe_unlink(path)
        _safe_unlink(approval_path)
        _safe_unlink(attempt)


def _finalize_cleanup_results(
    state: Path, root: Path | None = None, *, manager_key_path: Path,
    expected_key_id: str,
) -> None:
    """Seal successful startup cleanup without changing crack proof status."""

    state = Path(os.path.abspath(state))
    _assert_no_indirection(state)
    root = Path(root or state.parent).resolve()
    # Root-level approvals/base/candidates/permits are represented by the
    # single attempt receipt.  Never seal cleanup complete while that receipt,
    # the transaction journal, or a recovery marker still exists.
    if any(
        (state / name).exists()
        for name in ("attempt.json", "transaction.json", "RECOVERY_REQUIRED.json")
    ):
        return
    for path in _state_glob(state, "owners/*/*/latest/result.json", "terminal result"):
        value = _read_json(path)
        if (
            isinstance(value, Mapping)
            and value.get("status") == "improved"
            and value.get("cleanup_status") in {"pending", "cleanup_incomplete"}
        ):
            run_dir = path.parent
            if any(
                (run_dir / name).exists()
                for name in ("temp", "raw", "logs", "out")
            ):
                continue
            frontier_path = _frontier_file(run_dir)
            try:
                frontier = _validate_frontier(
                    root, frontier_path,
                    manager_key_path=manager_key_path,
                    expected_key_id=expected_key_id,
                    require_live_source=True,
                )
            except (CrackHarnessError, OSError, TypeError, ValueError):
                continue
            if (
                frontier.get("frontier_sha256") != value.get("frontier_sha256")
                or frontier.get("candidate_sha256")
                != value.get("candidate_sha256")
            ):
                continue
            _assert_no_indirection(run_dir)
            shutil.rmtree(run_dir)
            continue
        if (
            not isinstance(value, Mapping)
            or value.get("status") != "exact"
            or value.get("cleanup_status") not in {"pending", "cleanup_incomplete"}
        ):
            continue
        run_dir = path.parent
        if any((run_dir / name).exists() for name in ("temp", "raw", "logs", "out")):
            continue
        if not _valid_root_cleanup_receipt(
            root, state, run_dir, value,
            manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
        ):
            continue
        binding = _terminal_binding_from_result(run_dir, value)
        if (
            binding is None
            or not _valid_terminal_result(
                root, path, run_dir / "record.commit.json", binding
            )
        ):
            # A self-hashed or otherwise incomplete result is not a retained
            # terminal.  In particular, do not let forged exact JSON block a
            # future dry-run while startup maintenance is running.
            continue
        body = dict(value)
        body.pop("result_sha256", None)
        body["cleanup_status"] = "complete"
        errors = body.get("cleanup_errors")
        body["cleanup_errors"] = (
            [str(item)[:1000] for item in errors[:8]]
            if isinstance(errors, list) else []
        )
        # Recheck the signed manifest at the publication boundary.  Status
        # maintenance holds the transaction lock, and any external recreation
        # observed between the first check and this write leaves the result
        # cleanup-incomplete.
        if not _valid_root_cleanup_receipt(
            root, state, run_dir, value,
            manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
        ):
            continue
        _safe_unlink(_frontier_file(run_dir))
        _atomic_json(path, {**body, "result_sha256": _digest_json(body)})


def _retry_retention_maintenance(
    state: Path, root: Path | None = None, *, manager_key_path: Path,
    expected_key_id: str,
) -> list[str]:
    state = Path(os.path.abspath(state))
    _assert_no_indirection(state)
    result_paths = _state_glob(
        state, "owners/*/*/latest/result.json", "retained terminal result"
    )
    protected = {path.parent.parent for path in result_paths}
    errors: list[str] = []
    for path in result_paths:
        try:
            value = _read_json(path)
            if (
                isinstance(value, Mapping)
                and value.get("status") in {"exact", "improved"}
                and value.get("cleanup_status") in {"pending", "cleanup_incomplete"}
            ):
                run_dir = path.parent
                worktree = run_dir / "temp" / "worktree"
                if worktree.exists():
                    _assert_no_indirection(worktree)
                    _remove_disposable_worktree(root, worktree)
                _cleanup_raw(run_dir)
        except BaseException as exc:
            errors.append(f"terminal disposable cleanup: {exc}"[:1000])
        try:
            _gc_owner(
                path.parent, MAX_RETAINED_OWNER_BYTES,
                protected={path.parent.parent},
            )
        except BaseException as exc:
            errors.append(f"owner retention maintenance: {exc}"[:1000])
    try:
        _gc_global(state, MAX_RETAINED_GLOBAL_BYTES, protected=protected)
    except BaseException as exc:
        errors.append(f"global retention maintenance: {exc}"[:1000])
    if errors:
        for path in result_paths:
            value = _read_json(path)
            if not isinstance(value, Mapping) or value.get("cleanup_status") not in {
                "pending", "cleanup_incomplete",
            }:
                continue
            body = dict(value)
            body.pop("result_sha256", None)
            prior = body.get("cleanup_errors")
            combined = list(prior) if isinstance(prior, list) else []
            body["cleanup_status"] = "cleanup_incomplete"
            body["cleanup_errors"] = [str(item)[:1000] for item in (combined + errors)[-8:]]
            _atomic_json(path, {**body, "result_sha256": _digest_json(body)})
    else:
        _finalize_cleanup_results(
            state, root, manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
        )
    return errors


def load_approval(
    root: Path, path: Path, *, allow_applied_source: bool = False,
    recovery_cleanup: bool = False,
) -> dict[str, Any]:
    root = root.resolve()
    approval_path = _bound_path(root, os.fspath(path), "approval")
    value = _read_json(approval_path)
    if not isinstance(value, Mapping) or value.get("schema") != APPROVAL_SCHEMA:
        raise CrackHarnessError(f"approval schema must be {APPROVAL_SCHEMA}")
    approval = dict(value)
    permit_identity_sha256 = _approval_permit_identity(value)
    commands_sha256 = _digest_json(value.get("commands"))
    allowed = {
        "schema", "approval_id", "owner", "task_id", "function",
        "unit", "base_commit", "toolchain_key", "target_sha256", "permit_sha256", "issued_at", "expires_at",
        "source", "base", "candidate", "function_span", "predicted_rows", "selection",
        "commands", "campaign", "limits", "retry",
    }
    unknown = sorted(set(approval) - allowed)
    if unknown:
        raise CrackHarnessError("approval contains unknown fields: " + ", ".join(unknown))
    approval["_permit_identity_sha256"] = permit_identity_sha256
    approval["_commands_sha256"] = commands_sha256
    approval_id = _text(approval.get("approval_id"), "approval_id")
    if re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", approval_id) is None:
        raise CrackHarnessError("approval_id contains unsafe characters")
    _text(approval.get("owner"), "owner")
    _text(approval.get("task_id"), "task_id")
    unit = _text(approval.get("unit"), "unit")
    if not is_closed_objdiff_unit_name(unit):
        raise CrackHarnessError("unit is not a closed objdiff unit name")
    _text(approval.get("base_commit"), "base_commit")
    if _sha(approval.get("toolchain_key"), "toolchain_key") != TOOLCHAIN_MANIFEST_KEY:
        raise CrackHarnessError("toolchain_key is not the canonical Ninja-inclusive manifest")
    _sha(approval.get("target_sha256"), "target_sha256")
    _sha(approval.get("permit_sha256"), "permit_sha256")
    function = _text(approval.get("function"), "function")
    if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", function) is None:
        raise CrackHarnessError("function must be a C identifier")
    issued_at = _timestamp(approval.get("issued_at"), "issued_at")
    expires_at = _timestamp(approval.get("expires_at"), "expires_at")
    now = datetime.now(timezone.utc)
    if issued_at > now + timedelta(seconds=MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("approval issued_at is unacceptably in the future")
    if expires_at > now + timedelta(seconds=DEFAULT_ACTIVE_SECONDS + MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("approval expires_at exceeds the current hard attempt horizon")
    if expires_at <= now and not recovery_cleanup:
        raise CrackHarnessError("approval has expired")
    if (expires_at - issued_at).total_seconds() > DEFAULT_ACTIVE_SECONDS:
        raise CrackHarnessError("approval lifetime exceeds the hard 1800-second attempt window")
    paths: dict[str, Path] = {}
    for name in ("source", "base", "candidate"):
        if not isinstance(approval.get(name), Mapping) or set(approval[name]) != {"path", "sha256"}:
            raise CrackHarnessError(f"{name} must be a path/hash object")
    for name in ("source", "base", "candidate"):
        descriptor = approval.get(name)
        assert isinstance(descriptor, Mapping)
        cell_may_be_gone = recovery_cleanup and name in {"base", "candidate"}
        item = _bound_path(
            root, descriptor.get("path"), f"{name}.path",
            exists=not cell_may_be_gone,
        )
        expected = _sha(descriptor.get("sha256"), f"{name}.sha256")
        actual = _digest_file(item) if item.is_file() else None
        applied_source = (
            name == "source"
            and allow_applied_source
            and actual == _sha(approval["candidate"].get("sha256"), "candidate.sha256")
        )
        if actual is None and not cell_may_be_gone:
            raise CrackHarnessError(f"{name}.path is missing: {item}")
        if actual is not None and actual != expected and not applied_source:
            raise CrackHarnessError(f"{name} SHA-256 mismatch: {item}")
        paths[name] = item
        if name == "source":
            approval["_source_applied"] = applied_source
    if approval["source"]["sha256"] != approval["base"]["sha256"]:
        raise CrackHarnessError("source and sealed base must start byte-identical")
    if paths["source"] == paths["base"]:
        raise CrackHarnessError("live source and sealed base must use separate paths")
    if paths["source"] == paths["candidate"] or paths["base"] == paths["candidate"]:
        raise CrackHarnessError("candidate must be a separate sealed natural-C cell")
    if approval["candidate"]["sha256"] == approval["base"]["sha256"]:
        raise CrackHarnessError("candidate does not differ from the sealed base")
    approval["_paths"] = paths
    approval["_root"] = root
    if "retry" in approval and approval["retry"] is None:
        raise CrackHarnessError("retry must not be null")
    approval["_retry"] = (
        None
        if recovery_cleanup
        else _validate_retry_descriptor(
            root, approval["retry"] if "retry" in approval else None,
            approval["owner"], approval["function"],
            approval["candidate"]["sha256"],
        )
    )
    span = approval.get("function_span")
    if not isinstance(span, Mapping) or set(span) != {"start_line", "end_line", "base_span_sha256"}:
        raise CrackHarnessError("function_span must bind lines and base_span_sha256")
    start, end = span.get("start_line"), span.get("end_line")
    if not all(isinstance(item, int) and not isinstance(item, bool) and item > 0 for item in (start, end)) or start > end:
        raise CrackHarnessError("function_span is invalid")
    span_sha = _sha(span.get("base_span_sha256"), "function_span.base_span_sha256")
    if paths["base"].is_file() and paths["candidate"].is_file():
        _validate_natural_cell(paths["base"], paths["candidate"], start, end, span_sha)
    elif not recovery_cleanup:
        raise CrackHarnessError("sealed base/candidate cell inputs are missing")
    rows = _validate_predicted_rows(approval.get("predicted_rows"))
    if not recovery_cleanup:
        _validate_winning_cell_selection(
            root, approval.get("selection"), approval["candidate"]["sha256"], rows,
            approval["owner"], approval["function"], approval["base_commit"],
            mutable_source_path=paths["source"],
        )
    commands = approval.get("commands")
    if not isinstance(commands, Mapping):
        raise CrackHarnessError("commands must be an object")
    required = ("precompile", "compile", *HOOKS, "assess", "record")
    kinds = {"precompile": "canonical_admission", "compile": "compile", "assess": "assessment", "record": "canonical_record"}
    kinds.update({name: f"proof_{name}" for name in HOOKS})
    approval["commands"] = {name: _command(root, commands.get(name), f"commands.{name}", kinds[name]) for name in required}
    _validate_admission_argv(root, approval)
    _validate_compile_argv(approval)
    for name in (*HOOKS, "assess"):
        _validate_proof_adapter_argv(root, approval, name)
    _validate_record_descriptor(root, approval)
    for name in required:
        if name in {"precompile", "record"}:
            continue
        joined = "\0".join(approval["commands"][name]["argv"])
        if "{RUN_ROOT}" not in joined or "{OUT_ROOT}" not in joined:
            raise CrackHarnessError(f"commands.{name} must use RUN_ROOT and OUT_ROOT placeholders")
        if str(root) in joined:
            raise CrackHarnessError(f"commands.{name} embeds a production writable path")
    approval["limits"] = _limits(approval.get("limits"))
    campaign = approval.get("campaign")
    if not isinstance(campaign, Mapping) or set(campaign) != {"id", "quota"}:
        raise CrackHarnessError("campaign must be a closed id/quota object")
    campaign_id = _text(campaign.get("id"), "campaign.id")
    if re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", campaign_id) is None:
        raise CrackHarnessError("campaign.id contains unsafe characters")
    quota = campaign.get("quota", 1)
    if quota != 1:
        raise CrackHarnessError("function campaign quota is hard-fixed at one")
    approval["_approval_path"] = approval_path
    approval["_approval_sha256"] = _digest_file(approval_path)
    approval["_paths"] = paths
    return approval


def _state_root(
    root: Path, value: str | Path | None = None, *, _test_token: object | None = None,
) -> Path:
    if value is not None and _test_token is not _TEST_STATE_TOKEN:
        raise CrackHarnessError("production harness state root is fixed and cannot be overridden")
    raw = Path(value or DEFAULT_STATE_ROOT).expanduser()
    path = Path(os.path.abspath(raw if raw.is_absolute() else root / raw))
    if not _inside(root, path):
        raise CrackHarnessError(f"state root escapes repository root: {path}")
    _assert_no_indirection(root)
    _assert_no_indirection(path, missing_leaf=not path.exists())
    return path


def _load_permit(
    root: Path, approval: Mapping[str, Any], permit_path: Path, state: Path,
    *, manager_key_path: Path = MANAGER_PERMIT_KEY,
    expected_key_id: str = MANAGER_KEY_ID,
) -> tuple[dict[str, Any], Path]:
    path = _bound_path(root, os.fspath(permit_path), "manager resume permit")
    value = _read_json(path)
    required = {
        "schema", "permit_id", "issuer", "resume",
        "owner", "task_id", "function", "campaign_id", "stop_nonce",
        "approval_id", "approval_identity_sha256", "commands_sha256",
        "source_relpath", "source_sha256", "base_sha256", "candidate_sha256",
        "base_commit", "toolchain_key", "target_sha256",
        "issued_at", "deadline", "key_id", "signature",
    }
    if not isinstance(value, Mapping) or set(value) != required:
        raise CrackHarnessError("resume permit is not a strict closed object")
    permit = dict(value)
    if permit.get("schema") != PERMIT_SCHEMA or permit.get("resume") is not True:
        raise CrackHarnessError("resume permit is not manager-issued resume authority")
    _text(permit.get("permit_id"), "permit_id")
    if permit.get("issuer") != MANAGER_ISSUER:
        raise CrackHarnessError("resume permit issuer is not the fixed manager authority")
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32:
        raise CrackHarnessError("manager permit key must contain exactly 32 raw bytes")
    key_id = _digest_bytes(secret)
    if key_id != expected_key_id:
        raise CrackHarnessError("manager permit key file does not match the fixed key ID")
    if permit.get("key_id") != key_id:
        raise CrackHarnessError("resume permit key_id does not bind the manager key")
    unsigned = dict(permit)
    signature = unsigned.pop("signature", None)
    expected_signature = hmac.new(secret, _canonical(unsigned), hashlib.sha256).hexdigest()
    if not isinstance(signature, str) or not hmac.compare_digest(signature, expected_signature):
        raise CrackHarnessError("resume permit signature is invalid")
    if _digest_file(path) != approval["permit_sha256"]:
        raise CrackHarnessError("cell approval does not bind this exact assignment permit")
    for key, expected in (("owner", approval["owner"]), ("task_id", approval["task_id"]), ("function", approval["function"]), ("campaign_id", approval["campaign"]["id"])):
        if permit.get(key) != expected:
            raise CrackHarnessError(f"assignment permit does not bind {key}")
    exact_bindings = {
        "approval_id": approval["approval_id"],
        "approval_identity_sha256": approval["_permit_identity_sha256"],
        "commands_sha256": approval["_commands_sha256"],
        "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
        "source_sha256": approval["source"]["sha256"],
        "base_sha256": approval["base"]["sha256"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "base_commit": approval["base_commit"],
        "toolchain_key": approval["toolchain_key"],
        "target_sha256": approval["target_sha256"],
    }
    for key, expected in exact_bindings.items():
        if permit.get(key) != expected:
            raise CrackHarnessError(f"assignment permit does not bind exact {key}")
    permit_issued = _timestamp(permit.get("issued_at"), "permit issued_at")
    permit_expires = _timestamp(permit.get("deadline"), "permit deadline")
    now = datetime.now(timezone.utc)
    if permit_issued > now + timedelta(seconds=MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("resume permit issued_at is unacceptably in the future")
    if permit_expires > now + timedelta(seconds=DEFAULT_ACTIVE_SECONDS + MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("resume permit deadline exceeds the current hard attempt horizon")
    if permit_expires <= now:
        raise CrackHarnessError("resume permit has expired")
    if (permit_expires - permit_issued).total_seconds() > DEFAULT_ACTIVE_SECONDS:
        raise CrackHarnessError("resume permit exceeds the hard 1800-second attempt window")
    approval_expires = _timestamp(approval.get("expires_at"), "approval expires_at")
    if permit_expires > approval_expires:
        raise CrackHarnessError("resume permit deadline outlives the bound approval")
    _verify_stop(state, path, permit)
    permit["_permit_sha256"] = _digest_file(path)
    return permit, path


def _revoked_stop_value() -> dict[str, Any]:
    """Return a STOP record that cannot authenticate any permit."""

    return {
        "schema": "crack_harness_stop/v1",
        "stopped": True,
        "authorized_permit_sha256": "0" * 64,
        "stop_nonce": "0" * 64,
    }


def _rollback_issued_packet(
    state: Path, stop_path: Path, prior_stop: bytes | None,
    outputs: Sequence[Path], primary: BaseException,
) -> None:
    """Undo an incomplete packet while retaining the primary issuer error.

    STOP is restored/revoked before disposable artifacts are removed.  If any
    cleanup step is uncertain, leave a deliberately unauthenticated STOP so a
    leftover permit can never authorize execution.  All rollback failures are
    attached as notes; none may replace the original exception.
    """

    failures: list[tuple[str, BaseException]] = []
    stop_restored = False
    try:
        if prior_stop is None:
            _safe_unlink(stop_path)
        else:
            _atomic_bytes(stop_path, prior_stop)
        stop_restored = True
    except BaseException as exc:
        failures.append(("STOP rollback", exc))

    for path in outputs:
        try:
            _safe_unlink(path)
        except BaseException as exc:
            failures.append((f"packet artifact rollback ({path.name})", exc))

    if not stop_restored or failures:
        try:
            _atomic_json(stop_path, _revoked_stop_value())
        except BaseException as exc:
            failures.append(("STOP revocation", exc))

    if failures:
        marker_body = {
            "schema": PACKET_ROLLBACK_REQUIRED_SCHEMA,
            "primary_reason": str(primary)[:1000],
            "stop_path": str(stop_path),
            "prior_stop_sha256": (
                _digest_bytes(prior_stop) if prior_stop is not None else None
            ),
            "current_stop_sha256": (
                _digest_file(stop_path) if stop_path.is_file() else None
            ),
            "artifacts": [
                {
                    "path": str(path),
                    "sha256": _digest_file(path) if path.is_file() else None,
                }
                for path in outputs
            ],
            "failures": [
                {"label": label, "reason": str(exc)[:1000]}
                for label, exc in failures[:8]
            ],
            "created_at": _now(),
        }
        try:
            _atomic_json(
                state / "PACKET_ROLLBACK_REQUIRED.json",
                {
                    **marker_body,
                    "rollback_sha256": _digest_json(marker_body),
                },
            )
        except BaseException as exc:
            failures.append(("packet rollback marker", exc))

    for label, exc in failures:
        _note_secondary(primary, label, exc)


def _issue_manager_packet_core(
    root: Path, draft_path: Path, approval_out: Path, permit_out: Path,
    state: Path, *, manager_key_path: Path, expected_key_id: str,
) -> dict[str, Any]:
    """Atomically materialize approval, signed permit, and matching STOP.

    STOP is published last, so a partial write can never authorize a run.  On
    any validation failure the prior STOP bytes are restored and newly-created
    disposable files are removed.
    """

    root = Path(os.path.abspath(root))
    draft_file = _bound_path(root, os.fspath(draft_path), "approval draft")
    approval_file = _bound_path(
        root, os.fspath(approval_out), "issued approval", exists=False
    )
    permit_file = _bound_path(
        root, os.fspath(permit_out), "issued permit", exists=False
    )
    if len({draft_file, approval_file, permit_file}) != 3:
        raise CrackHarnessError("draft, approval output, and permit output must differ")
    stop_path = state / "STOP"
    _assert_no_indirection(state)
    if not state.is_dir():
        raise CrackHarnessError(f"manager state root is not a directory: {state}")
    _assert_no_indirection(stop_path, missing_leaf=True)
    if stop_path.exists() and not stop_path.is_file():
        raise CrackHarnessError(f"global STOP is not a regular file: {stop_path}")
    if approval_file == stop_path or permit_file == stop_path:
        raise CrackHarnessError("manager packet outputs must not replace global STOP")
    if _inside(state, approval_file) or _inside(state, permit_file):
        raise CrackHarnessError(
            "manager packet outputs must remain outside the harness state tree"
        )
    for output in (approval_file, permit_file):
        if output.exists():
            raise CrackHarnessError(f"manager issuer refuses to overwrite: {output}")
        if _is_tracked(root, output):
            raise CrackHarnessError(f"manager issuer output is tracked: {output}")
    if (
        (state / "transaction.json").exists()
        or (state / "RECOVERY_REQUIRED.json").exists()
        or (state / "PACKET_ROLLBACK_REQUIRED.json").exists()
    ):
        raise CrackHarnessError("manager issuer refuses while recovery state is active")
    draft_value = _read_json(draft_file)
    if not isinstance(draft_value, Mapping):
        raise CrackHarnessError("approval draft must be an object")
    if draft_value.get("permit_sha256") != "0" * 64:
        raise CrackHarnessError("approval draft permit_sha256 must be the zero placeholder")
    draft = load_approval(root, draft_file)
    _verify_repository(root, draft, allow_source=False)
    key_path = _manager_key_file(root, manager_key_path)
    secret = key_path.read_bytes()
    if len(secret) != 32 or _digest_bytes(secret) != expected_key_id:
        raise CrackHarnessError("manager permit key does not match the fixed key ID")
    issued_at = _timestamp(draft["issued_at"], "issued_at")
    deadline = _timestamp(draft["expires_at"], "expires_at")
    stop_nonce = secrets.token_hex(32)
    permit_body = {
        "schema": PERMIT_SCHEMA,
        "permit_id": "permit-" + secrets.token_hex(12),
        "issuer": MANAGER_ISSUER,
        "resume": True,
        "owner": draft["owner"], "task_id": draft["task_id"],
        "function": draft["function"], "campaign_id": draft["campaign"]["id"],
        "stop_nonce": stop_nonce,
        "approval_id": draft["approval_id"],
        "approval_identity_sha256": _approval_permit_identity(draft_value),
        "commands_sha256": _digest_json(draft_value["commands"]),
        "source_relpath": draft["_paths"]["source"].relative_to(root).as_posix(),
        "source_sha256": draft["source"]["sha256"],
        "base_sha256": draft["base"]["sha256"],
        "candidate_sha256": draft["candidate"]["sha256"],
        "base_commit": draft["base_commit"],
        "toolchain_key": draft["toolchain_key"],
        "target_sha256": draft["target_sha256"],
        "issued_at": issued_at.isoformat(), "deadline": deadline.isoformat(),
        "key_id": expected_key_id,
    }
    permit = {
        **permit_body,
        "signature": hmac.new(
            secret, _canonical(permit_body), hashlib.sha256
        ).hexdigest(),
    }
    prior_stop = stop_path.read_bytes() if stop_path.is_file() else None
    try:
        _atomic_json(permit_file, permit)
        permit_sha256 = _digest_file(permit_file)
        final_approval = dict(draft_value)
        final_approval["permit_sha256"] = permit_sha256
        _atomic_json(approval_file, final_approval)
        _atomic_json(
            stop_path,
            {
                "schema": "crack_harness_stop/v1", "stopped": True,
                "authorized_permit_sha256": permit_sha256,
                "stop_nonce": stop_nonce,
            },
        )
        loaded = load_approval(root, approval_file)
        _load_permit(
            root, loaded, permit_file, state,
            manager_key_path=manager_key_path, expected_key_id=expected_key_id,
        )
        readiness = _dry_run_core(root, approval_file, state)
        if readiness["status"] != "ready":
            raise CrackHarnessError(
                "issued packet is not dry-run ready: " + "; ".join(readiness["blockers"])
            )
    except BaseException as primary:
        _rollback_issued_packet(
            state, stop_path, prior_stop, (approval_file, permit_file), primary
        )
        raise
    return {
        "schema": "crack_harness_issued_packet/v1", "status": "ready",
        "approval": {"path": str(approval_file), "sha256": _digest_file(approval_file)},
        "permit": {"path": str(permit_file), "sha256": _digest_file(permit_file)},
        "stop": {"path": str(stop_path), "sha256": _digest_file(stop_path)},
        "owner": draft["owner"], "function": draft["function"],
        "candidate_sha256": draft["candidate"]["sha256"],
        "deadline": deadline.isoformat(), "authority_advanced": False,
        "run_argv": [
            sys.executable, "tools/agent.py", "--root", str(root), "crack", "run",
            "--approval", str(approval_file), "--permit", str(permit_file),
        ],
    }


def issue_manager_packet(
    root: Path, draft_path: Path, approval_out: Path, permit_out: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    state = _state_root(root)
    _safe_mkdir(state)
    with serialized_build_lock(state / ".transaction.lock", 55.0):
        return _issue_manager_packet_core(
            root, draft_path, approval_out, permit_out, state,
            manager_key_path=MANAGER_PERMIT_KEY, expected_key_id=MANAGER_KEY_ID,
        )


def _issue_manager_packet_for_test(
    root: Path, draft_path: Path, approval_out: Path, permit_out: Path,
    *, state_root: Path, manager_key_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    state = _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN)
    _safe_mkdir(state)
    with serialized_build_lock(state / ".transaction.lock", 55.0):
        validated_key = _manager_key_file(root, manager_key_path)
        return _issue_manager_packet_core(
            root, draft_path, approval_out, permit_out, state,
            manager_key_path=manager_key_path,
            expected_key_id=_digest_file(validated_key),
        )


def _verify_stop(state: Path, permit_path: Path, permit: Mapping[str, Any]) -> None:
    stop = state / "STOP"
    _assert_no_indirection(stop)
    if not stop.is_file():
        raise CrackHarnessError("global STOP is not a regular file")
    stop_value = _read_json(stop)
    expected_stop = {
        "schema": "crack_harness_stop/v1", "stopped": True,
        "authorized_permit_sha256": _digest_file(permit_path),
        "stop_nonce": _sha(permit.get("stop_nonce"), "stop_nonce"),
    }
    if not isinstance(stop_value, Mapping) or dict(stop_value) != expected_stop:
        raise CrackHarnessError("global STOP does not authenticate this exact manager assignment permit")


def _tree_size(path: Path) -> int:
    total = 0
    if not path.exists():
        return 0
    for parent, dirs, files in os.walk(path, followlinks=False):
        base = Path(parent)
        for name in dirs:
            item = base / name
            info = item.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE):
                raise CrackHarnessError(f"temporary path indirection is forbidden: {item}")
        for name in files:
            item = base / name
            info = item.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE):
                raise CrackHarnessError(f"temporary path indirection is forbidden: {item}")
            total += info.st_size
    return total


def _git(root: Path, *args: str) -> str:
    completed = subprocess.run(
        ["git", *args], cwd=root, text=True, capture_output=True, check=False
    )
    if completed.returncode != 0:
        raise CrackHarnessError(f"git {' '.join(args)} failed: {completed.stderr.strip()}")
    return completed.stdout.strip()


def _is_tracked(root: Path, path: Path) -> bool:
    completed = subprocess.run(
        ["git", "ls-files", "--error-unmatch", "--", str(path.relative_to(root))],
        cwd=root, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
    )
    return completed.returncode == 0


def _verify_repository(root: Path, approval: Mapping[str, Any], *, allow_source: bool) -> None:
    _assert_no_indirection(root)
    _assert_no_indirection(root / ".git")
    common_raw = Path(_git(root, "rev-parse", "--git-common-dir"))
    common = Path(os.path.abspath(common_raw if common_raw.is_absolute() else root / common_raw))
    _assert_no_indirection(common)
    if _git(root, "rev-parse", "HEAD") != approval["base_commit"]:
        raise CrackHarnessError("Git HEAD does not equal approved base_commit")
    if not _is_tracked(root, approval["_paths"]["source"]):
        raise CrackHarnessError("approved live source is not tracked at base_commit")
    dirty = []
    for line in _git(root, "status", "--porcelain=v1", "--untracked-files=no").splitlines():
        relative = (line[3:] if len(line) > 2 and line[2] == " " else line[2:]).strip().replace("\\", "/")
        source_relative = approval["_paths"]["source"].relative_to(root).as_posix()
        # A retained partial frontier intentionally leaves exactly this tracked
        # source dirty.  Its bytes are independently hash-checked by
        # ``load_approval`` and every ``_checkpoint``; all other tracked writes
        # remain forbidden.  ``allow_source`` selects base-vs-candidate hash,
        # not whether the one approved source path may appear in Git status.
        if relative != source_relative:
            dirty.append(relative)
    if dirty:
        raise CrackHarnessError("unapproved tracked writes: " + ", ".join(dirty))


def _checkpoint(
    root: Path, approval_path: Path, approval: Mapping[str, Any],
    permit_path: Path, permit: Mapping[str, Any], state: Path, *, allow_source: bool,
) -> None:
    _validate_winning_cell_selection(
        root, approval.get("selection"), approval["candidate"]["sha256"],
        approval["predicted_rows"], approval["owner"], approval["function"],
        approval["base_commit"],
        mutable_source_path=approval["_paths"]["source"],
    )
    now = datetime.now(timezone.utc)
    approval_expires = _timestamp(approval.get("expires_at"), "approval expires_at")
    permit_expires = _timestamp(permit.get("deadline"), "permit deadline")
    if permit_expires > approval_expires:
        raise CrackHarnessError("resume permit deadline outlives the bound approval")
    if now >= approval_expires or now >= permit_expires:
        raise CrackHarnessError("bound approval/permit window expired during transaction")
    if _digest_file(approval_path) != approval["_approval_sha256"]:
        raise CrackHarnessError("approval changed during transaction")
    if _digest_file(permit_path) != permit["_permit_sha256"]:
        raise CrackHarnessError("resume permit changed during transaction")
    _verify_stop(state, permit_path, permit)
    _verify_objdiff_pin()
    for name in ("base", "candidate"):
        if _digest_file(approval["_paths"][name]) != approval[name]["sha256"]:
            raise CrackHarnessError(f"{name} changed during transaction")
    for name, descriptor in approval["commands"].items():
        if _digest_file(descriptor["executable"]) != descriptor["executable_sha256"]:
            raise CrackHarnessError(f"{name} executable changed during transaction")
        if descriptor["script"] is not None and _digest_file(descriptor["script"]) != descriptor["script_sha256"]:
            raise CrackHarnessError(f"{name} script changed during transaction")
    expected_source = approval["candidate" if allow_source else "source"]["sha256"]
    if _digest_file(approval["_paths"]["source"]) != expected_source:
        raise CrackHarnessError("live source changed outside the approved cell")
    _verify_repository(root, approval, allow_source=allow_source)


def _record_commit_value(path: Path) -> Mapping[str, Any] | None:
    try:
        _assert_no_indirection(path, missing_leaf=not path.exists())
        if not path.is_file():
            return None
        return _validate_record_commit(_read_json(path), "record commit")
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return None


def _recovery_record_commit_value(path: Path) -> Mapping[str, Any] | None:
    """Read a journal's local record commit, including legacy outcomes.

    The public terminal surface is exact-only, but recovery must still be able
    to inspect a pre-v2 journal that recorded an older ``improved`` outcome.
    This parser is deliberately private to rollback: callers must never use
    its result as a valid terminal or retained result.  An invalid commit is
    returned as ``None`` so recovery can preserve the central row rather than
    deleting a row it cannot authenticate.
    """

    try:
        _assert_no_indirection(path, missing_leaf=not path.exists())
        if not path.is_file():
            return None
        value = _read_json(path)
        if not isinstance(value, Mapping):
            return None
        body = dict(value)
        digest = body.pop("commit_sha256", None)
        required = {
            "schema", "outcome", "candidate_sha256", "record_payload_sha256",
            "record_sha256",
        }
        if (
            set(body) != required
            or body.get("schema") != "crack_harness_record_commit/v1"
            or body.get("outcome") not in {"exact", "improved"}
            or digest != _digest_json(body)
        ):
            return None
        _sha(body.get("candidate_sha256"), "recovery record candidate_sha256")
        _sha(body.get("record_payload_sha256"), "recovery record payload_sha256")
        _sha(body.get("record_sha256"), "recovery record record_sha256")
        return dict(value)
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return None


def _transaction_central_binding(
    value: Mapping[str, Any], approval: Mapping[str, Any] | None = None,
) -> dict[str, Any] | None:
    """Return a strictly shaped exact binding from a transaction journal.

    The journal digest protects accidental corruption, while the approval
    comparison prevents a journal writer from redirecting recovery to another
    owner/function.  This helper never authorizes central deletion; it only
    identifies the row that must remain retained when local finalization is
    incomplete.
    """

    raw = value.get("central_record_binding")
    required = {
        "input_key", "owner", "function", "source_sha256",
        "target_object_sha256", "object_sha256", "candidate_record_sha256",
        "status",
    }
    if not isinstance(raw, Mapping) or set(raw) != required:
        return None
    if raw.get("status") != "exact":
        return None
    try:
        binding = {
            "input_key": _sha(raw.get("input_key"), "transaction input_key"),
            "owner": _text(raw.get("owner"), "transaction owner"),
            "function": _text(raw.get("function"), "transaction function"),
            "source_sha256": _sha(
                raw.get("source_sha256"), "transaction source_sha256"
            ),
            "target_object_sha256": _sha(
                raw.get("target_object_sha256"),
                "transaction target_object_sha256",
            ),
            "object_sha256": _sha(
                raw.get("object_sha256"), "transaction object_sha256"
            ),
            "candidate_record_sha256": _sha(
                raw.get("candidate_record_sha256"),
                "transaction candidate_record_sha256",
            ),
            "status": "exact",
        }
    except CrackHarnessError:
        return None
    if approval is not None and (
        binding["owner"] != approval.get("owner")
        or binding["function"] != approval.get("function")
        or binding["source_sha256"] != approval.get("candidate", {}).get("sha256")
        or binding["target_object_sha256"] != approval.get("target_sha256")
    ):
        return None
    return binding


def _write_recovery_required(
    root: Path, state: Path, transaction: Mapping[str, Any], reason: str,
    *, record_sha256: str | None = None,
) -> None:
    """Durably bind an ambiguous post-record failure to its journal.

    The journal remains the recovery authority.  This marker is deliberately
    compact and overwrite-only: it tells startup to preserve the candidate and
    central exact row until the complete local report can be authenticated or
    a manager resolves the state.  It is never used to invalidate a central
    row.
    """

    root = Path(os.path.abspath(root))
    state = Path(os.path.abspath(state))
    _assert_no_indirection(root)
    _assert_no_indirection(state)
    raw_transaction = dict(transaction)
    transaction_sha256 = raw_transaction.get("transaction_sha256")
    if transaction_sha256 is None:
        transaction_sha256 = _digest_json(raw_transaction)
    else:
        _sha(transaction_sha256, "transaction_sha256")
    binding = _transaction_central_binding(raw_transaction)
    if binding is None:
        raise CrackHarnessError(
            "cannot write recovery marker without a bound exact transaction"
        )
    body: dict[str, Any] = {
        "schema": RECOVERY_REQUIRED_SCHEMA,
        "transaction_path": "transaction.json",
        "transaction_sha256": transaction_sha256,
        "approval_path": str(raw_transaction.get("approval_path", "")),
        "approval_sha256": raw_transaction.get("approval_sha256"),
        "owner": raw_transaction.get("owner"),
        "function": raw_transaction.get("function"),
        "source_relpath": raw_transaction.get("source_relpath"),
        "base_sha256": raw_transaction.get("base_sha256"),
        "candidate_sha256": raw_transaction.get("candidate_sha256"),
        "target_object_sha256": raw_transaction.get("target_object_sha256"),
        "central_record_binding": binding,
        "record_sha256": record_sha256,
        "reason": str(reason)[:1000] or "post-record recovery is required",
        "created_at": _now(),
    }
    # Keep all marker values bounded and hash the exact body that was written.
    if len(_canonical(body)) > 64 * 1024:
        raise CrackHarnessError("recovery marker exceeds the compact state cap")
    _atomic_json(state / "RECOVERY_REQUIRED.json", {
        **body, "recovery_sha256": _digest_json(body),
    })


def _terminal_binding_from_result(
    run_dir: Path, result: Any,
) -> dict[str, Any] | None:
    """Derive the central binding needed to authenticate a retained result.

    Startup cleanup runs after the local record commit is normally removed, so
    the result's compact record/assessment receipts are the remaining source
    of the binding.  This helper is intentionally only a shape extractor;
    ``_valid_terminal_result`` performs the complete semantic validation.
    """

    _assert_no_indirection(run_dir)
    if not isinstance(result, Mapping) or result.get("status") != "exact":
        return None
    receipts = result.get("receipts")
    if not isinstance(receipts, Mapping):
        return None
    record_receipt = receipts.get("record")
    assess_receipt = receipts.get("assess")
    if not isinstance(record_receipt, Mapping) or not isinstance(assess_receipt, Mapping):
        return None
    record = record_receipt.get("summary")
    if not isinstance(record, Mapping):
        return None
    fields = (
        ("owner", result.get("owner")),
        ("function", result.get("function")),
        ("source_sha256", result.get("candidate_sha256")),
        ("target_object_sha256", record.get("target_object_sha256")),
        ("object_sha256", record.get("candidate_object_sha256")),
        ("input_key", record.get("admission_input_key")),
        ("candidate_record_sha256", assess_receipt.get("payload_sha256")),
    )
    binding: dict[str, Any] = {key: value for key, value in fields}
    binding["status"] = "exact"
    try:
        _sha(binding["source_sha256"], "terminal source_sha256")
        _sha(binding["target_object_sha256"], "terminal target_object_sha256")
        _sha(binding["object_sha256"], "terminal object_sha256")
        _sha(binding["input_key"], "terminal input_key")
        _sha(binding["candidate_record_sha256"], "terminal candidate_record_sha256")
    except CrackHarnessError:
        return None
    return binding


def _validate_record_commit(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, Mapping):
        raise CrackHarnessError(f"{label} is not a typed object")
    body = dict(value)
    digest = body.pop("commit_sha256", None)
    required = {
        "schema", "outcome", "candidate_sha256", "record_payload_sha256",
        "record_sha256",
    }
    if (
        set(body) != required
        or body.get("schema") != "crack_harness_record_commit/v1"
        # The harness retains only a completed exact crack.  RecoveryMemory
        # still supports historical ``improved`` rows, but they are never a
        # valid terminal or record-commit for this exact-only front door.
        or body.get("outcome") != "exact"
        or digest != _digest_json(body)
    ):
        raise CrackHarnessError(f"{label} digest or schema is invalid")
    _sha(body.get("candidate_sha256"), f"{label}.candidate_sha256")
    _sha(body.get("record_payload_sha256"), f"{label}.record_payload_sha256")
    _sha(body.get("record_sha256"), f"{label}.record_sha256")
    return dict(value)


def _report_int(value: Any, label: str) -> int:
    if type(value) is not int or value < 0:
        raise CrackHarnessError(f"{label} must be a non-negative integer")
    return value


def _report_signed_int(value: Any, label: str) -> int:
    if type(value) is not int:
        raise CrackHarnessError(f"{label} must be an integer")
    return value


def _report_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise CrackHarnessError(f"{label} must be numeric")
    try:
        result = float(value)
    except OverflowError as exc:
        raise CrackHarnessError(f"{label} must be finite") from exc
    if not math.isfinite(result):
        raise CrackHarnessError(f"{label} must be finite")
    return result


def _validate_exact_report(
    report: Mapping[str, Any], result: Mapping[str, Any], binding: Mapping[str, Any],
    record_commit: Mapping[str, Any] | None = None,
) -> None:
    """Validate the complete compact CRACK_REPORT used for exact recovery.

    This is deliberately shared by report sealing and interrupted-transaction
    recovery.  A self-digest authenticates bytes, not their meaning, so every
    field that makes an exact result retainable is checked here as well.
    """

    if set(report) != EXACT_REPORT_FIELDS or report.get("schema") != REPORT_SCHEMA:
        raise CrackHarnessError("exact report is not the canonical CRACK_REPORT/v1 schema")
    report_digest = _sha(report.get("report_sha256"), "report_sha256")
    report_body = dict(report)
    report_body.pop("report_sha256")
    if report_digest != _digest_json(report_body):
        raise CrackHarnessError("exact report digest is invalid")
    if report.get("status") != "exact" or report.get("completed") is not True:
        raise CrackHarnessError("exact report is not a completed exact result")
    if report.get("authority_advanced") is not False:
        raise CrackHarnessError("exact report must preserve authority_advanced=false")

    owner = _text(report.get("owner"), "report owner")
    function = _text(report.get("function"), "report function")
    task_id = _text(report.get("task_id"), "report task_id")
    base_commit = _text(report.get("base_commit"), "report base_commit")
    approval_sha256 = _sha(report.get("approval_sha256"), "report approval_sha256")
    source_sha256 = _sha(report.get("source_sha256"), "report source_sha256")
    target_object_sha256 = _sha(
        report.get("target_object_sha256"), "report target_object_sha256"
    )
    candidate_object_sha256 = _sha(
        report.get("candidate_object_sha256"), "report candidate_object_sha256"
    )
    completed_at = _timestamp(report.get("completed_at"), "report completed_at")

    if not isinstance(binding, Mapping) or binding.get("status") != "exact":
        raise CrackHarnessError("exact report central binding is missing or non-exact")
    bound_target_object_sha256 = _sha(
        binding.get("target_object_sha256"), "binding.target_object_sha256"
    )
    if (
        owner != binding.get("owner")
        or function != binding.get("function")
        or source_sha256 != binding.get("source_sha256")
        or target_object_sha256 != bound_target_object_sha256
        or candidate_object_sha256 != binding.get("object_sha256")
    ):
        raise CrackHarnessError("exact report does not bind the central owner/function/source/object")

    result_fields = {
        "schema", "approval_id", "approval_sha256", "owner", "task_id", "function",
        "base_commit", "campaign_id", "attempt_sha256", "candidate_sha256",
        "base_sha256", "status", "expected_terminal", "terminal_expectation_met",
        "reason", "owner_gain", "predicted_rows", "receipts", "finished_at",
        "source_restored", "cleanup_status", "cleanup_errors", "authority_advanced",
        "result_sha256", "report_sha256",
    }
    if set(result) != result_fields or result.get("schema") != RESULT_SCHEMA:
        raise CrackHarnessError("exact terminal result is not the canonical result schema")
    result_digest = _sha(result.get("result_sha256"), "result_sha256")
    _sha(result.get("attempt_sha256"), "result attempt_sha256")
    result_body = dict(result)
    result_body.pop("result_sha256")
    if result_digest != _digest_json(result_body):
        raise CrackHarnessError("exact terminal result digest is invalid")
    if (
        result.get("status") != "exact"
        or result.get("expected_terminal") not in {"exact", "improved"}
        or result.get("terminal_expectation_met") is not True
        or result.get("authority_advanced") is not False
        or result.get("source_restored") is not False
        or result.get("report_sha256") != report_digest
        or result.get("owner") != owner
        or result.get("function") != function
        or result.get("task_id") != task_id
        or result.get("base_commit") != base_commit
        or result.get("approval_sha256") != approval_sha256
        or result.get("candidate_sha256") != source_sha256
    ):
        raise CrackHarnessError("exact terminal result does not bind the exact report")
    if _timestamp(result.get("finished_at"), "result finished_at") != completed_at:
        raise CrackHarnessError("exact report completed_at does not match result finished_at")
    if not isinstance(result.get("reason"), str) or not result["reason"]:
        raise CrackHarnessError("exact terminal result reason is missing")
    if result.get("cleanup_status") not in {"pending", "complete", "cleanup_incomplete"}:
        raise CrackHarnessError("exact terminal cleanup status is invalid")
    cleanup_errors = result.get("cleanup_errors")
    if (
        not isinstance(cleanup_errors, list) or len(cleanup_errors) > 8
        or any(not isinstance(item, str) or len(item) > 1000 for item in cleanup_errors)
    ):
        raise CrackHarnessError("exact terminal cleanup errors are invalid")

    predicted_rows = _validate_predicted_rows(
        report.get("predicted_rows"), "exact report predicted_rows"
    )
    result_rows = _validate_predicted_rows(
        result.get("predicted_rows"), "exact result predicted_rows"
    )
    if predicted_rows != result_rows:
        raise CrackHarnessError("exact report predicted_rows are not result-bound")

    report_result = report.get("result")
    if (
        not isinstance(report_result, Mapping)
        or set(report_result) != EXACT_REPORT_RESULT_FIELDS
    ):
        raise CrackHarnessError("exact report result is incomplete")
    report_strict = _report_number(report_result.get("strict_percent"), "report strict_percent")
    report_data = _report_number(report_result.get("data_percent"), "report data_percent")
    report_target_bytes = _report_int(report_result.get("target_bytes"), "report target_bytes")
    report_candidate_bytes = _report_int(report_result.get("candidate_bytes"), "report candidate_bytes")
    report_gain = _report_number(report_result.get("owner_gain"), "report owner_gain")
    if report_strict != 100 or report_data != 100 or report_target_bytes != report_candidate_bytes or report_gain <= 0:
        raise CrackHarnessError("exact report result does not prove exactness")

    receipts = result.get("receipts")
    if not isinstance(receipts, Mapping):
        raise CrackHarnessError("exact terminal receipts are missing")
    receipt_names = set(receipts)
    if not EXACT_RESULT_RECEIPTS <= receipt_names or receipt_names - (EXACT_RESULT_RECEIPTS | {"secondary_failures"}):
        raise CrackHarnessError("exact terminal is missing a required receipt")
    proof_receipts = report.get("proof_receipts")
    if not isinstance(proof_receipts, Mapping) or set(proof_receipts) != EXACT_REPORT_PROOF_RECEIPTS:
        raise CrackHarnessError("exact report is missing a required proof receipt")

    compact_summaries: dict[str, Mapping[str, Any]] = {}
    for name in EXACT_REPORT_PROOF_RECEIPTS:
        receipt = receipts.get(name)
        if not isinstance(receipt, Mapping) or set(receipt) != {
            "schema", "hook", "ok", "summary", "payload_sha256", "command",
        }:
            raise CrackHarnessError(f"exact {name} receipt is not a compact typed receipt")
        if (
            receipt.get("schema") != "crack_harness_receipt/v1"
            or receipt.get("hook") != name
            or receipt.get("ok") is not True
        ):
            raise CrackHarnessError(f"exact {name} receipt has the wrong schema or hook")
        payload_sha256 = _sha(receipt.get("payload_sha256"), f"{name}.payload_sha256")
        summary = receipt.get("summary")
        _validate_command_receipt(receipt.get("command"), f"{name}.command")
        if not isinstance(summary, Mapping):
            raise CrackHarnessError(f"exact {name} receipt summary is invalid")
        entry = proof_receipts.get(name)
        if (
            not isinstance(entry, Mapping) or set(entry) != {"sha256", "summary"}
            or _sha(entry.get("sha256"), f"report proof {name}.sha256") != payload_sha256
            or entry.get("summary") != summary
        ):
            raise CrackHarnessError(f"report proof receipt {name} does not bind terminal receipt")
        compact_summaries[name] = summary

    compile_receipt = receipts.get("compile")
    if (
        not isinstance(compile_receipt, Mapping)
        or set(compile_receipt) != {"schema", "hook", "baseline_command", "candidate_command"}
        or compile_receipt.get("schema") != "crack_harness_receipt/v1"
        or compile_receipt.get("hook") != "compile"
        or not isinstance(compile_receipt.get("baseline_command"), Mapping)
        or not isinstance(compile_receipt.get("candidate_command"), Mapping)
    ):
        raise CrackHarnessError("exact compile receipt is incomplete")
    _validate_command_receipt(
        compile_receipt["baseline_command"], "compile.baseline_command"
    )
    _validate_command_receipt(
        compile_receipt["candidate_command"], "compile.candidate_command"
    )

    precompile = compact_summaries["precompile"]
    if set(precompile) != {
        "status", "reused", "skip_compile", "input_key", "admission_token",
        "expires_at", "authority_advanced",
    }:
        raise CrackHarnessError("exact precompile receipt is incomplete")
    if (
        precompile.get("status") != "admitted"
        or type(precompile.get("reused")) is not bool
        or precompile.get("skip_compile") is not False
        or precompile.get("authority_advanced") is not False
    ):
        raise CrackHarnessError("exact precompile receipt is not an admitted non-authoritative result")
    admission_input_key = _sha(precompile.get("input_key"), "precompile input_key")
    admission_token = _text(precompile.get("admission_token"), "precompile admission_token")
    _timestamp(precompile.get("expires_at"), "precompile expires_at")

    proof_common = {
        "schema", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "report_sha256",
    }
    object_pair: tuple[str, str] | None = None
    proof_artifact_sha: str | None = None
    for name in ("strict", "data", "focus", "siblings", "physical"):
        summary = compact_summaries[name]
        extras = {
            "strict": {"strict_percent", "target_bytes", "candidate_bytes", "differences"},
            "data": {"data_percent", "target_bytes", "candidate_bytes", "differences"},
            "focus": {"differing_rows"},
            "siblings": {"protected_total", "protected_losses"},
            "physical": {"target_count", "candidate_count", "differences"},
        }[name]
        if set(summary) != proof_common | extras:
            raise CrackHarnessError(f"exact {name} proof summary is incomplete")
        if (
            summary.get("owner") != owner
            or summary.get("function") != function
            or summary.get("candidate_source_sha256") != source_sha256
        ):
            raise CrackHarnessError(f"exact {name} proof summary is not source-bound")
        target = _sha(summary.get("target_object_sha256"), f"{name}.target_object_sha256")
        candidate = _sha(summary.get("candidate_object_sha256"), f"{name}.candidate_object_sha256")
        artifact = _sha(summary.get("report_sha256"), f"{name}.report_sha256")
        if object_pair is None:
            object_pair = (target, candidate)
            proof_artifact_sha = artifact
        elif object_pair != (target, candidate) or proof_artifact_sha != artifact:
            raise CrackHarnessError("exact proof summaries disagree on object/artifact identity")
        if name == "strict":
            if (
                _report_number(summary.get("strict_percent"), "strict_percent") != 100
                or _report_int(summary.get("target_bytes"), "strict.target_bytes")
                != _report_int(summary.get("candidate_bytes"), "strict.candidate_bytes")
                or _report_int(summary.get("differences"), "strict.differences") != 0
            ):
                raise CrackHarnessError("strict proof summary is not exact")
        elif name == "data":
            if (
                _report_number(summary.get("data_percent"), "data_percent") != 100
                or _report_int(summary.get("target_bytes"), "data.target_bytes")
                != _report_int(summary.get("candidate_bytes"), "data.candidate_bytes")
                or _report_int(summary.get("differences"), "data.differences") != 0
            ):
                raise CrackHarnessError("data proof summary is not exact")
        elif name == "focus":
            if _report_int(summary.get("differing_rows"), "focus.differing_rows") != 0:
                raise CrackHarnessError("focus proof summary is not exact")
        elif name == "siblings":
            _report_int(summary.get("protected_total"), "siblings.protected_total")
            if _report_int(summary.get("protected_losses"), "siblings.protected_losses") != 0:
                raise CrackHarnessError("siblings proof summary is not exact")
        else:
            if (
                _report_int(summary.get("target_count"), "physical.target_count")
                != _report_int(summary.get("candidate_count"), "physical.candidate_count")
                or _report_int(summary.get("differences"), "physical.differences") != 0
            ):
                raise CrackHarnessError("physical proof summary is not exact")

    assert object_pair is not None
    if target_object_sha256 != object_pair[0] or candidate_object_sha256 != object_pair[1]:
        raise CrackHarnessError("exact report object identity does not bind proof summaries")
    if candidate_object_sha256 != _sha(binding.get("object_sha256"), "binding.object_sha256"):
        raise CrackHarnessError("exact report candidate object does not bind central record")
    if (
        report_result.get("strict_percent") != compact_summaries["strict"].get("strict_percent")
        or report_result.get("data_percent") != compact_summaries["data"].get("data_percent")
        or report_result.get("target_bytes") != compact_summaries["strict"].get("target_bytes")
        or report_result.get("candidate_bytes") != compact_summaries["strict"].get("candidate_bytes")
    ):
        raise CrackHarnessError("exact report result does not bind strict/data proof summaries")

    assessment = compact_summaries["assess"]
    if set(assessment) != {
        "schema", "owner", "function", "candidate_source_sha256", "target_object_sha256",
        "candidate_object_sha256", "owner_gain", "data_gain", "data_diff_delta",
        "physical_diff_delta",
    } or assessment.get("schema") != "crack_assessment/v1":
        raise CrackHarnessError("exact assessment receipt is incomplete")
    if (
        assessment.get("owner") != owner
        or assessment.get("function") != function
        or assessment.get("candidate_source_sha256") != source_sha256
        or assessment.get("target_object_sha256") != object_pair[0]
        or assessment.get("candidate_object_sha256") != object_pair[1]
    ):
        raise CrackHarnessError("exact assessment is not object/source-bound")
    assessment_gain = _report_number(assessment.get("owner_gain"), "assessment.owner_gain")
    if (
        assessment_gain <= 0
        or _report_number(assessment.get("data_gain"), "assessment.data_gain") < 0
        or _report_signed_int(assessment.get("data_diff_delta"), "assessment.data_diff_delta") > 0
        or _report_signed_int(
            assessment.get("physical_diff_delta"),
            "assessment.physical_diff_delta",
        ) > 0
    ):
        raise CrackHarnessError("exact assessment does not prove a non-regressing gain")
    if report_gain != assessment_gain or _report_number(result.get("owner_gain"), "result.owner_gain") != assessment_gain:
        raise CrackHarnessError("exact owner gain is not assessment-bound")

    record = compact_summaries["record"]
    if set(record) != {
        "schema", "recorded", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "outcome",
        "admission_token_sha256", "admission_input_key", "record_sha256",
    } or record.get("schema") != "crack_central_record_receipt/v1":
        raise CrackHarnessError("exact central record summary is incomplete")
    if (
        record.get("recorded") is not True
        or record.get("owner") != owner
        or record.get("function") != function
        or record.get("candidate_source_sha256") != source_sha256
        or record.get("target_object_sha256") != object_pair[0]
        or record.get("candidate_object_sha256") != object_pair[1]
        or record.get("outcome") != "exact"
        or record.get("admission_input_key") != admission_input_key
        or record.get("admission_token_sha256") != _digest_bytes(admission_token.encode("utf-8"))
    ):
        raise CrackHarnessError("exact central record summary is not admission-bound")
    record_sha256 = _sha(record.get("record_sha256"), "record.record_sha256")
    if record_commit is not None:
        record_commit = _validate_record_commit(record_commit, "exact record commit")
        if record_commit.get("record_sha256") != record_sha256:
            raise CrackHarnessError("exact record SHA does not bind the central record commit")


def _validate_terminal_paths(
    root: Path, path: Path, record_commit_path: Path,
) -> tuple[Path, Path, Path]:
    """Validate result, record-commit, and report paths before reading them."""

    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    result_path = Path(os.path.abspath(path))
    if not result_path.exists():
        raise CrackHarnessError(f"terminal result does not exist: {result_path}")
    run_dir = result_path.parent
    state = _state_from_run_dir(run_dir)
    if not _inside(root, state):
        raise CrackHarnessError(f"terminal state escapes repository root: {state}")
    _state_result_path(state, result_path)
    commit_path = Path(os.path.abspath(record_commit_path))
    _state_sibling_path(
        state, commit_path, "record.commit.json", "record commit",
    )
    report_path = run_dir / "CRACK_REPORT_v1.json"
    _state_sibling_path(
        state, report_path, "CRACK_REPORT_v1.json", "exact report",
    )
    return result_path, commit_path, report_path


def _valid_terminal_result(
    root: Path, path: Path, record_commit_path: Path, binding: Any,
    approval: Mapping[str, Any] | None = None,
    *, central_required: bool = True,
) -> bool:
    _validate_terminal_paths(root, path, record_commit_path)
    try:
        path = Path(os.path.abspath(path))
        record_commit_path = Path(os.path.abspath(record_commit_path))
        report_path = path.parent / "CRACK_REPORT_v1.json"
        if not path.is_file() or not isinstance(binding, Mapping):
            return False
        value = _read_json(path)
        if not isinstance(value, Mapping) or value.get("status") != "exact":
            return False
        if approval is not None:
            expected_approval = {
                "approval_id": approval["approval_id"],
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"],
                "task_id": approval["task_id"],
                "function": approval["function"],
                "base_commit": approval["base_commit"],
                "campaign_id": approval["campaign"]["id"],
                "base_sha256": approval["base"]["sha256"],
                "candidate_sha256": approval["candidate"]["sha256"],
                "predicted_rows": approval["predicted_rows"],
                "expected_terminal": approval["selection"]["expected_terminal"],
            }
            if any(value.get(key) != expected for key, expected in expected_approval.items()):
                return False
            if (
                binding.get("owner") != approval["owner"]
                or binding.get("function") != approval["function"]
                or binding.get("source_sha256") != approval["candidate"]["sha256"]
                or binding.get("target_object_sha256") != approval["target_sha256"]
            ):
                return False
            if value.get("terminal_expectation_met") is not True:
                return False
            expected_run_dir = _run_dir(
                Path(os.path.abspath(_state_from_run_dir(path.parent))), approval,
            )
            if expected_run_dir != path.parent:
                return False
        body = dict(value)
        digest = body.pop("result_sha256", None)
        record = value.get("receipts", {}).get("record", {}) if isinstance(value.get("receipts"), Mapping) else {}
        if record_commit_path.exists() and not record_commit_path.is_file():
            return False
        commit_exists = record_commit_path.is_file()
        commit = _record_commit_value(record_commit_path)
        if commit_exists and commit is None:
            return False
        summary = record.get("summary") if isinstance(record, Mapping) else None
        if (
            digest != _digest_json(body)
            or not isinstance(record, Mapping)
            or not isinstance(summary, Mapping)
            or _digest_json(summary) != record.get("payload_sha256")
            or commit is not None and (
                commit.get("record_payload_sha256") != record.get("payload_sha256")
                or commit.get("outcome") != "exact"
                or commit.get("candidate_sha256") != value.get("candidate_sha256")
            )
        ):
            return False
        bound_target_object_sha256 = _sha(
            binding.get("target_object_sha256"),
            "binding.target_object_sha256",
        )
        if commit is not None and commit.get("record_sha256") != summary.get("record_sha256"):
            return False
        expected = {
            "schema": "crack_central_record_receipt/v1", "recorded": True,
            "owner": binding.get("owner"), "function": binding.get("function"),
            "candidate_source_sha256": binding.get("source_sha256"),
            "candidate_object_sha256": binding.get("object_sha256"),
            "outcome": binding.get("status"),
            "admission_input_key": binding.get("input_key"),
        }
        expected["target_object_sha256"] = bound_target_object_sha256
        if any(summary.get(key) != expected_value for key, expected_value in expected.items()):
            return False
        if (
            value.get("owner") != binding.get("owner")
            or value.get("function") != binding.get("function")
            or value.get("candidate_sha256") != binding.get("source_sha256")
            or value.get("status") != binding.get("status")
        ):
            return False
        terminal_record_sha256 = _sha(
            summary.get("record_sha256"),
            "terminal record.record_sha256",
        )
        if central_required and not _central_record_matches(
            root, binding, record_sha256=terminal_record_sha256,
        ):
            return False
        if not report_path.is_file():
            return False
        report = _read_json(report_path)
        if not isinstance(report, Mapping):
            return False
        _validate_exact_report(report, value, binding, record_commit=commit)
        return True
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return False
def _valid_exact_record_commit(path: Path, candidate_sha256: str) -> bool:
    value = _record_commit_value(path)
    return bool(
        value is not None and value.get("outcome") == "exact"
        and value.get("candidate_sha256") == candidate_sha256
    )


def _invalidate_central_record(
    root: Path, binding: Any, *, record_sha256: str | None = None,
    authenticated: bool = False,
) -> None:
    if binding is None:
        return
    if not authenticated:
        raise CrackHarnessError(
            "central record invalidation requires authenticated terminal evidence"
        )
    memory_required = {
        "input_key", "owner", "function", "source_sha256", "object_sha256",
        "candidate_record_sha256", "status",
    }
    required = memory_required | {"target_object_sha256"}
    if not isinstance(binding, Mapping) or set(binding) != required:
        raise CrackHarnessError("transaction central record binding is invalid")
    try:
        from tools.recovery_memory import RecoveryMemory, RecoveryMemoryError

        memory_binding = {
            "input_key": binding["input_key"],
            "owner": binding["owner"],
            "function": binding["function"],
            "target_sha256": binding["target_object_sha256"],
            "source_sha256": binding["source_sha256"],
            "object_sha256": binding["object_sha256"],
            "candidate_record_sha256": binding["candidate_record_sha256"],
            "status": binding["status"],
        }
        if record_sha256 is not None:
            memory_binding["record_sha256"] = record_sha256
        result = RecoveryMemory.for_root(root).invalidate_retained(**memory_binding)
    except (OSError, RecoveryMemoryError) as exc:
        raise CrackHarnessError(
            "cannot reconcile the exact central retained experiment before rollback"
        ) from exc
    if result.get("status") not in {"missing", "invalidated"}:
        raise CrackHarnessError("central retained experiment reconciliation failed")


def _central_record_matches(
    root: Path, binding: Any, *, record_sha256: str | None = None,
) -> bool:
    memory_required = {
        "input_key", "owner", "function", "source_sha256", "object_sha256",
        "candidate_record_sha256", "status",
    }
    required = memory_required | {"target_object_sha256"}
    if not isinstance(binding, Mapping) or set(binding) != required:
        return False
    try:
        from tools.recovery_memory import RecoveryMemory, RecoveryMemoryError

        memory_binding = {
            "input_key": binding["input_key"],
            "owner": binding["owner"],
            "function": binding["function"],
            "target_sha256": binding["target_object_sha256"],
            "source_sha256": binding["source_sha256"],
            "object_sha256": binding["object_sha256"],
            "candidate_record_sha256": binding["candidate_record_sha256"],
            "status": binding["status"],
        }
        if record_sha256 is not None:
            memory_binding["record_sha256"] = record_sha256
        RecoveryMemory.for_root(root).verify_retained(**memory_binding)
        return True
    except RecoveryMemoryError as exc:
        # ``verify_retained`` distinguishes a verified absent input key from a
        # mismatched/corrupt row.  Only the former is destructive-path evidence.
        if str(exc) == "no retained experiment matches the authenticated input key":
            return False
        raise CrackHarnessError(
            "central retained-record query is inconclusive; recovery remains pending"
        ) from exc
    except (OSError, TypeError, ValueError) as exc:
        # Treat database/validation failure as UNKNOWN, never as "row absent".
        raise CrackHarnessError(
            "central retained-record query is unavailable; recovery remains pending"
        ) from exc


def _authenticated_record_binding(
    root: Path, approval: Mapping[str, Any], result_path: Path,
    record_commit_path: Path,
) -> tuple[dict[str, Any], str] | None:
    """Derive an invalidation binding from local evidence and central memory.

    ``transaction.json`` is an integrity-protected journal, but its digest is
    not an authority: a process that can write the journal can recompute that
    digest.  Consequently, central deletion is allowed only when the actual
    approval file, the compact record/assessment receipts, the record commit,
    and the canonical central row all agree.  In particular, callers must not
    pass a journal-provided ``central_record_binding`` here.
    """

    if not result_path.is_file() or not record_commit_path.is_file():
        return None
    _validate_terminal_paths(root, result_path, record_commit_path)
    try:
        result = _read_json(result_path)
        if not isinstance(result, Mapping) or result.get("status") != "exact":
            return None
        result_body = dict(result)
        result_digest = result_body.pop("result_sha256", None)
        if result_digest != _digest_json(result_body):
            return None
        for key, expected in {
            "approval_sha256": approval["_approval_sha256"],
            "owner": approval["owner"],
            "function": approval["function"],
            "base_commit": approval["base_commit"],
            "base_sha256": approval["base"]["sha256"],
            "candidate_sha256": approval["candidate"]["sha256"],
        }.items():
            if result.get(key) != expected:
                return None

        receipts = result.get("receipts")
        if not isinstance(receipts, Mapping):
            return None
        record_receipt = receipts.get("record")
        assess_receipt = receipts.get("assess")
        if not isinstance(record_receipt, Mapping) or not isinstance(assess_receipt, Mapping):
            return None
        receipt_fields = {"schema", "hook", "ok", "summary", "payload_sha256", "command"}
        if (
            set(record_receipt) != receipt_fields
            or set(assess_receipt) != receipt_fields
            or record_receipt.get("schema") != "crack_harness_receipt/v1"
            or assess_receipt.get("schema") != "crack_harness_receipt/v1"
            or record_receipt.get("hook") != "record"
            or assess_receipt.get("hook") != "assess"
            or record_receipt.get("ok") is not True
            or assess_receipt.get("ok") is not True
        ):
            return None
        record_summary = record_receipt.get("summary")
        assess_summary = assess_receipt.get("summary")
        if not isinstance(record_summary, Mapping) or not isinstance(assess_summary, Mapping):
            return None
        record_payload_sha256 = _sha(
            record_receipt.get("payload_sha256"), "record receipt payload_sha256"
        )
        assessment_payload_sha256 = _sha(
            assess_receipt.get("payload_sha256"), "assessment receipt payload_sha256"
        )
        if (
            _digest_json(record_summary) != record_payload_sha256
            or _digest_json(assess_summary) != assessment_payload_sha256
        ):
            return None
        _validate_command_receipt(record_receipt.get("command"), "record receipt command")
        _validate_command_receipt(assess_receipt.get("command"), "assessment receipt command")

        record_required = {
            "schema", "recorded", "owner", "function", "candidate_source_sha256",
            "target_object_sha256", "candidate_object_sha256", "outcome",
            "admission_token_sha256", "admission_input_key", "record_sha256",
        }
        assess_required = {
            "schema", "owner", "function", "candidate_source_sha256",
            "target_object_sha256", "candidate_object_sha256", "owner_gain",
            "data_gain", "data_diff_delta", "physical_diff_delta",
        }
        if (
            set(record_summary) != record_required
            or set(assess_summary) != assess_required
            or record_summary.get("schema") != "crack_central_record_receipt/v1"
            or assess_summary.get("schema") != "crack_assessment/v1"
        ):
            return None
        expected_record = {
            "recorded": True,
            "owner": approval["owner"],
            "function": approval["function"],
            "candidate_source_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": approval["target_sha256"],
            "outcome": "exact",
        }
        if any(record_summary.get(key) != expected for key, expected in expected_record.items()):
            return None
        expected_assessment = {
            "owner": approval["owner"],
            "function": approval["function"],
            "candidate_source_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": record_summary.get("target_object_sha256"),
            "candidate_object_sha256": record_summary.get("candidate_object_sha256"),
        }
        if any(assess_summary.get(key) != expected for key, expected in expected_assessment.items()):
            return None
        object_sha256 = _sha(
            record_summary.get("candidate_object_sha256"),
            "record candidate_object_sha256",
        )
        target_object_sha256 = _sha(
            record_summary.get("target_object_sha256"),
            "record target_object_sha256",
        )
        input_key = _sha(record_summary.get("admission_input_key"), "record admission_input_key")
        record_sha256 = _sha(record_summary.get("record_sha256"), "record record_sha256")
        _sha(record_summary.get("admission_token_sha256"), "record admission_token_sha256")
        commit = _recovery_record_commit_value(record_commit_path)
        if (
            commit is None
            or commit.get("outcome") != "exact"
            or commit.get("candidate_sha256") != approval["candidate"]["sha256"]
            or commit.get("record_payload_sha256") != record_payload_sha256
            or commit.get("record_sha256") != record_sha256
        ):
            return None
        binding = {
            "input_key": input_key,
            "owner": approval["owner"],
            "function": approval["function"],
            "source_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": target_object_sha256,
            "object_sha256": object_sha256,
            "candidate_record_sha256": assessment_payload_sha256,
            "status": "exact",
        }
        if not _central_record_matches(root, binding, record_sha256=record_sha256):
            return None
        return binding, record_sha256
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return None


def _recover_interrupted(root: Path, state: Path) -> None:
    root = Path(os.path.abspath(root))
    state = Path(os.path.abspath(state))
    _assert_no_indirection(root)
    _assert_no_indirection(state)
    journal = _state_path(state, state / "transaction.json", "transaction journal", exists=None)
    if not journal.is_file():
        return
    _assert_no_indirection(journal)
    value = _read_json(journal)
    if not isinstance(value, Mapping):
        raise CrackHarnessError("interrupted transaction journal is invalid")
    unsigned = dict(value)
    digest = unsigned.pop("transaction_sha256", None)
    if (
        unsigned.get("schema") != TRANSACTION_SCHEMA
        or digest != _digest_json(unsigned)
    ):
        raise CrackHarnessError("interrupted transaction journal integrity failed")
    fields = set(unsigned)
    recorded_journal = fields == TRANSACTION_RECORDED_FIELDS
    legacy_journal = fields == LEGACY_TRANSACTION_FIELDS
    if fields not in (TRANSACTION_FIELDS, TRANSACTION_RECORDED_FIELDS) and not legacy_journal:
        raise CrackHarnessError("interrupted transaction journal identity is incomplete")

    approval: dict[str, Any] | None = None
    if not legacy_journal:
        approval_path_value = value.get("approval_path")
        approval_path = _bound_path(
            root, approval_path_value, "transaction approval", exists=False
        )
        if not approval_path.exists():
            # New exact cleanup removes the transaction journal before the
            # approval.  Therefore a live journal without its authenticated
            # approval is never a normal cleanup state and cannot authorize
            # either retention or rollback.  A post-record journal can still
            # be converted into a durable recovery lock so status remains
            # observable without granting cleanup authority.  Pre-record
            # journals have no authenticated central binding and continue to
            # fail closed below.
            try:
                _write_recovery_required(
                    root, state, value,
                    "interrupted transaction approval is missing; manager "
                    "recovery required",
                    record_sha256=value.get("record_sha256"),
                )
            except CrackHarnessError:
                pass
            else:
                return
            raise CrackHarnessError(
                "interrupted transaction approval is missing; manager recovery required"
            )
        else:
            if _digest_file(approval_path) != _sha(
                value.get("approval_sha256"), "transaction approval_sha256"
            ):
                raise CrackHarnessError("transaction approval file hash drifted")
            approval = load_approval(
                root, approval_path, allow_applied_source=True,
                recovery_cleanup=True,
            )
            expected_identity = {
                "approval_path": approval["_approval_path"],
                "approval_id": approval["approval_id"],
                "approval_identity_sha256": approval["_permit_identity_sha256"],
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"],
                "function": approval["function"],
                "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
                "source_sha256": approval["source"]["sha256"],
                "base_relpath": approval["_paths"]["base"].relative_to(root).as_posix(),
                "base_sha256": approval["base"]["sha256"],
                "base_commit": approval["base_commit"],
                "candidate_relpath": approval["_paths"]["candidate"].relative_to(root).as_posix(),
                "candidate_sha256": approval["candidate"]["sha256"],
                "target_object_sha256": approval["target_sha256"],
            }
            for key, expected in expected_identity.items():
                actual = value.get(key)
                if key == "approval_path":
                    actual = Path(os.path.abspath(str(actual)))
                if actual != expected:
                    raise CrackHarnessError(f"transaction {key} is not bound to the approval")
    else:
        # Legacy journals can still be rolled back, but they carry no
        # authenticated approval identity and therefore can never authorize a
        # central-record invalidation.
        value = dict(value)
        value["base_sha256"] = value["baseline_sha256"]
        value["base_commit"] = ""
        value["report_path"] = str(
            Path(os.path.abspath(str(value["result_path"]))).parent
            / "CRACK_REPORT_v1.json"
        )

    source_relpath = _text(value.get("source_relpath"), "transaction source_relpath")
    source = _bound_path(root, source_relpath, "transaction source")
    baseline = Path(os.path.abspath(str(value.get("baseline_snapshot"))))
    result_path = Path(os.path.abspath(str(value.get("result_path", ""))))
    report_path = Path(os.path.abspath(str(value.get("report_path", ""))))
    record_commit_path = Path(os.path.abspath(str(value.get("record_commit_path", ""))))
    worktree = Path(os.path.abspath(str(value.get("worktree", ""))))
    run_dir = result_path.parent
    _state_run_dir(state, run_dir, "interrupted transaction run")
    if approval is not None and run_dir != _run_dir(state, approval):
        raise CrackHarnessError("transaction run is not bound to the approved owner/function")
    expected_temp = run_dir / "temp"
    _state_path(state, expected_temp, "transaction temp directory")
    if baseline != expected_temp / "baseline.snapshot":
        raise CrackHarnessError("interrupted transaction baseline escapes the exact run temp")
    if worktree != expected_temp / "worktree":
        raise CrackHarnessError("interrupted transaction worktree escapes the exact run temp")
    if record_commit_path != run_dir / "record.commit.json":
        raise CrackHarnessError("interrupted transaction record path escapes latest state")
    if report_path != run_dir / "CRACK_REPORT_v1.json":
        raise CrackHarnessError("interrupted transaction report path escapes latest state")
    _state_path(state, result_path, "interrupted transaction result", exists=None)
    _state_path(state, report_path, "interrupted transaction report", exists=None)
    _state_path(state, record_commit_path, "interrupted transaction record commit", exists=None)
    _state_path(state, worktree, "interrupted transaction worktree", exists=None)
    _state_path(state, baseline, "interrupted transaction baseline", exists=None)
    for path in (source, run_dir):
        _assert_no_indirection(path)
    if expected_temp.exists():
        _assert_no_indirection(expected_temp)
    if not _is_tracked(root, source):
        raise CrackHarnessError("interrupted transaction source is not the approved tracked source")
    expected = _sha(value.get("baseline_sha256"), "transaction baseline_sha256")
    base_sha256 = _sha(value.get("base_sha256"), "transaction base_sha256")
    if base_sha256 != expected:
        raise CrackHarnessError("interrupted transaction base and baseline hashes differ")
    base_commit = _text(value.get("base_commit"), "transaction base_commit") if not legacy_journal else ""
    candidate_sha = _sha(value.get("candidate_sha256"), "transaction candidate_sha256")
    target_object_sha256 = _sha(
        value.get("target_object_sha256"), "transaction target_object_sha256"
    )
    if legacy_journal and value.get("central_record_binding") is not None:
        # A legacy journal has no authenticated approval file.  Its central
        # binding is consequently untrusted even when its self-hash is valid;
        # fail closed instead of attempting to reconcile any row from it.
        raise CrackHarnessError(
            "cannot reconcile interrupted transaction central record; central record retained"
        )
    if approval is not None and (
        expected != approval["base"]["sha256"]
        or candidate_sha != approval["candidate"]["sha256"]
        or target_object_sha256 != approval["target_sha256"]
    ):
        raise CrackHarnessError("transaction source/base/candidate identity drifted")
    baseline_available = baseline.is_file()
    if baseline_available and _digest_file(baseline) != expected:
        raise CrackHarnessError("interrupted transaction baseline hash drifted")

    record_commit_exists = record_commit_path.is_file()
    record_commit = _record_commit_value(record_commit_path)
    if record_commit is None and record_commit_exists:
        record_commit = _recovery_record_commit_value(record_commit_path)
    # A malformed commit is itself a post-record failure when the recorded
    # journal bit is present.  Defer the decision until the central binding is
    # checked so recovery can retain the row and journal instead of deleting
    # them under an invalid local marker.
    if record_commit is not None and record_commit.get("candidate_sha256") != candidate_sha:
        raise CrackHarnessError("interrupted transaction record commit is not candidate-bound")
    if legacy_journal and record_commit is not None and record_commit.get("outcome") != "exact":
        raise CrackHarnessError(
            "cannot reconcile interrupted transaction record commit; central record retained"
        )
    recorded_sha256 = None
    if recorded_journal:
        if value.get("record_succeeded") is not True:
            raise CrackHarnessError("interrupted transaction recorded state is invalid")
        recorded_sha256 = _sha(
            value.get("record_sha256"), "transaction record_sha256"
        )

    # A completed exact run may already have deleted its disposable approval
    # before a later cleanup retry.  The transaction binding may still be used
    # to *preserve* that row; it never authorizes deletion without the loaded
    # approval and independently authenticated terminal receipts below.
    transaction_binding = _transaction_central_binding(value, approval)
    if recorded_journal and transaction_binding is None:
        raise CrackHarnessError(
            "cannot reconcile interrupted transaction central record; binding is invalid"
        )
    recovery_marker = state / "RECOVERY_REQUIRED.json"
    marker_bound = False
    if recovery_marker.is_file() and not legacy_journal:
        _assert_no_indirection(recovery_marker)
        marker_value = _read_json(recovery_marker)
        if not isinstance(marker_value, Mapping):
            raise CrackHarnessError("recovery marker is invalid")
        marker_body = dict(marker_value)
        marker_digest = marker_body.pop("recovery_sha256", None)
        marker_bound = (
            marker_body.get("schema") == RECOVERY_REQUIRED_SCHEMA
            and marker_digest == _digest_json(marker_body)
            and marker_body.get("transaction_sha256")
            == value.get("transaction_sha256")
            and marker_body.get("central_record_binding") == transaction_binding
        )
        if not marker_bound:
            raise CrackHarnessError(
                "recovery marker is not bound to the interrupted transaction"
            )
    exact_commit = (
        record_commit is not None and record_commit.get("outcome") == "exact"
    )
    central_record_matches = False
    if transaction_binding is not None:
        central_record_matches = _central_record_matches(
            root, transaction_binding,
            record_sha256=recorded_sha256
            or (record_commit.get("record_sha256") if exact_commit else None),
        )
    central_record_persisted = bool(
        not legacy_journal and transaction_binding is not None
        and (recorded_journal or central_record_matches)
    )

    terminal_metadata_valid = not legacy_journal
    for metadata_path in (result_path, report_path):
        if not metadata_path.is_file():
            terminal_metadata_valid = False
            break
        try:
            metadata = _read_json(metadata_path)
        except CrackHarnessError:
            terminal_metadata_valid = False
            break
        if not isinstance(metadata, Mapping):
            terminal_metadata_valid = False
            break
        if approval is not None:
            expected_metadata = {
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"], "function": approval["function"],
                "base_commit": approval["base_commit"],
                (
                    "candidate_sha256"
                    if metadata_path == result_path else "source_sha256"
                ): approval["candidate"]["sha256"],
            }
            if any(
                metadata.get(key) != expected_value
                for key, expected_value in expected_metadata.items()
            ):
                terminal_metadata_valid = False
                break
    terminal_binding = (
        _terminal_binding_from_result(run_dir, _read_json(result_path))
        if result_path.is_file() else None
    )
    terminal_valid = (
        terminal_metadata_valid and terminal_binding is not None
        and _valid_terminal_result(
            root, result_path, record_commit_path, terminal_binding, approval
        )
    )
    retained = (
        _digest_file(source) == candidate_sha and terminal_valid
    )
    if not retained and central_record_persisted:
        # The exact central row is authoritative once record returned (or its
        # journal/commit proves that boundary).  A missing/invalid report,
        # commit, or result must not roll the source back and erase the only
        # journal that can reconcile the row.  Leave both durable markers for
        # startup recovery; it will finalize only after the complete exact
        # report and terminal result authenticate.
        _write_recovery_required(
            root, state, value,
            "central exact record committed before local exact terminal finalized",
            record_sha256=recorded_sha256
            or (record_commit.get("record_sha256") if exact_commit else None),
        )
        return
    if not retained:
        authenticated = (
            _authenticated_record_binding(
                root, approval, result_path, record_commit_path
            )
            if approval is not None else None
        )
        if authenticated is not None:
            binding, record_sha256 = authenticated
            _invalidate_central_record(
                root, binding, record_sha256=record_sha256, authenticated=True,
            )
        _atomic_copy(baseline, source)
        if record_commit is not None and record_commit.get("outcome") == "exact":
            # No authenticated central row survived, so this is a completed
            # rollback rather than an ambiguous recovery state.  Seal the
            # primary reason in the ordinary bounded diagnostic and remove the
            # journal below; never publish an unbound RECOVERY_REQUIRED marker.
            diagnostic_body = {
                "schema": "crack_harness_failure_diagnostic/v1",
                "approval_sha256": _sha(
                    value.get("approval_sha256"),
                    "transaction approval_sha256",
                ),
                "owner": (
                    approval["owner"] if approval is not None
                    else str(value.get("owner") or "legacy-unbound")
                ),
                "function": (
                    approval["function"] if approval is not None
                    else str(value.get("function") or run_dir.parent.name)
                ),
                "primary_reason": (
                    "local exact commit lacked a complete hash-bound "
                    "CRACK_REPORT; source and any authenticated central row "
                    "were rolled back"
                ),
                "cleanup_errors": [],
                "finished_at": _now(),
            }
            _atomic_json(
                run_dir.parent / "latest-failure.json",
                {
                    **diagnostic_body,
                    "diagnostic_sha256": _digest_json(diagnostic_body),
                },
            )
        result_path.unlink(missing_ok=True)
        report_path.unlink(missing_ok=True)
        record_commit_path.unlink(missing_ok=True)
    if retained and not (state / "attempt.json").is_file():
        _write_recovery_required(
            root, state, value,
            "exact cleanup authority is missing; manager recovery required",
            record_sha256=recorded_sha256
            or (record_commit.get("record_sha256") if exact_commit else None),
        )
        return
    recovery_marker = state / "RECOVERY_REQUIRED.json"
    # A bound recovery marker is a lock, not permanent history.  Once either
    # retained-terminal validation or authenticated rollback has completed,
    # clear it before removing the journal so startup can never inherit an
    # orphan marker with no reconciliation authority.
    if recovery_marker.exists():
        _safe_unlink(recovery_marker)
    _state_path(state, journal, "transaction journal", exists=True)
    journal.unlink()
    if terminal_valid and record_commit_path.is_file():
        record_commit_path.unlink(missing_ok=True)
    if worktree.exists():
        _remove_disposable_worktree(root, worktree)
    if baseline.parent.exists():
        _assert_no_indirection(baseline.parent)
        shutil.rmtree(baseline.parent, ignore_errors=False)


def _run_command(
    argv: Sequence[str], *, root: Path, run_temp: Path, deadline: float,
    storage_limit: int, expect_json: bool, extra_env: Mapping[str, str] | None = None,
    production_root: Path | None = None, state_root: Path | None = None,
    on_started: Callable[[], Callable[[], None] | None] | None = None,
    on_started_state_paths: Sequence[Path] = (),
) -> tuple[dict[str, Any] | None, dict[str, Any]]:
    remaining = deadline - time.monotonic()
    if remaining <= 0:
        raise CrackHarnessError("active-time limit exceeded")
    env = os.environ.copy()
    for secret_name in tuple(env):
        if secret_name.upper().startswith("CRACK_HARNESS_MANAGER_"):
            env.pop(secret_name, None)
    env.update({
        "CRACK_HARNESS_TEMP": os.fspath(run_temp),
        "CRACK_HARNESS_ROOT": os.fspath(root),
    })
    env.update(extra_env or {})
    # Reviewed Python front doors may import controller-owned modules.  Never
    # let those imports materialize __pycache__ beside the immutable tooling;
    # all durable command output must remain in the disposable roots.
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    env.pop("PYTHONPYCACHEPREFIX", None)
    for redirected_name in (
        "MP6_RECOVERY_MEMORY", "MP6_AGENT_QUEUE", "GIT_DIR", "GIT_COMMON_DIR",
        "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_OBJECT_DIRECTORY",
        "GIT_ALTERNATE_OBJECT_DIRECTORIES",
    ):
        env.pop(redirected_name, None)
    started = time.monotonic()
    production_manifest = (
        _repo_manifest(production_root, state_root) if production_root and state_root else None
    )
    state_manifest = (
        _tree_manifest(state_root, (run_temp,)) if state_root else None
    )
    allowed_state_keys: set[str] = set()
    state_root_path = Path(os.path.abspath(state_root)) if state_root else None
    if on_started_state_paths:
        if on_started is None or state_root_path is None:
            raise CrackHarnessError(
                "on_started state paths require an on_started callback and state root"
            )
        run_temp_path = Path(os.path.abspath(run_temp))
        for path in on_started_state_paths:
            allowed_path = Path(os.path.abspath(path))
            if not _inside(state_root_path, allowed_path):
                raise CrackHarnessError(
                    f"on_started state path escapes monitored state: {allowed_path}"
                )
            if _inside(run_temp_path, allowed_path):
                raise CrackHarnessError(
                    f"on_started state path is inside disposable run temp: {allowed_path}"
                )
            allowed_state_keys.add(
                allowed_path.relative_to(state_root_path).as_posix()
            )
    next_manifest_check = time.monotonic()
    process: subprocess.Popen[str] | None = None
    job_handle: int | None = None
    launch_rollback: Callable[[], None] | None = None
    captured = {"stdout": bytearray(), "stderr": bytearray()}
    overflow = threading.Event()
    capture_lock = threading.Lock()
    readers: list[threading.Thread] = []

    def drain(name: str, stream: Any) -> None:
        while True:
            chunk = stream.read(65536)
            if not chunk:
                return
            with capture_lock:
                if sum(len(value) for value in captured.values()) + len(chunk) > 1024 * 1024:
                    overflow.set()
                    return
                captured[name].extend(chunk)

    def cleanup_resources(*, terminate: bool) -> list[str]:
        errors: list[str] = []
        active_process = process
        active_job = job_handle
        if active_process is not None and terminate:
            try:
                _terminate_process(active_process)
            except BaseException as exc:
                errors.append(f"terminate: {exc}"[:1000])
        if active_job is None and active_process is not None:
            active_job = getattr(active_process, "_crack_harness_job", None)
        if active_job is not None:
            try:
                _quiesce_windows_job(active_job, terminate=terminate)
            except BaseException as exc:
                errors.append(f"quiesce: {exc}"[:1000])
            try:
                _close_windows_job(active_job)
            except BaseException as exc:
                errors.append(f"close job: {exc}"[:1000])
        for reader in readers:
            try:
                reader.join(timeout=2.0)
                if reader.is_alive():
                    errors.append(
                        "reader join: output stream did not quiesce"[:1000]
                    )
            except BaseException as exc:
                errors.append(f"reader join: {exc}"[:1000])
        for stream in (
            getattr(active_process, "stdout", None),
            getattr(active_process, "stderr", None),
        ):
            if stream:
                try:
                    stream.close()
                except BaseException as exc:
                    errors.append(f"stream close: {exc}"[:1000])
        return errors[:8]

    def object_observed() -> bool:
        """Report only whether a compiler object exists in the disposable tree."""

        try:
            for directory, names, files in os.walk(run_temp, followlinks=False):
                directory_path = Path(directory)
                names[:] = [
                    name for name in names
                    if not (directory_path / name).is_symlink()
                ]
                for name in files:
                    path = directory_path / name
                    if name.lower().endswith(".o") and not path.is_symlink():
                        return True
        except OSError:
            return False
        return False

    def failure_command_receipt(cleanup_errors: Sequence[str]) -> dict[str, Any]:
        stdout_bytes = bytes(captured["stdout"])
        stderr_bytes = bytes(captured["stderr"])
        return {
            "argv_sha256": _digest_json(list(argv)),
            "returncode": (
                process.returncode
                if process is not None and type(process.returncode) is int
                else None
            ),
            "active_seconds": round(time.monotonic() - started, 6),
            "stdout_sha256": _digest_bytes(stdout_bytes),
            "stderr_sha256": _digest_bytes(stderr_bytes),
            "cleanup_errors": [str(item)[:1000] for item in cleanup_errors[:8]],
            "object_observed": object_observed(),
        }

    def attach_failure_receipt(
        primary: BaseException, cleanup_errors: Sequence[str],
    ) -> None:
        try:
            setattr(
                primary, "_crack_command_receipt",
                failure_command_receipt(cleanup_errors),
            )
        except BaseException as receipt_error:
            _note_secondary(primary, "failure command receipt", receipt_error)

    try:
        # Keep process creation, containment assignment, resumption, and pipe
        # reader startup in the same cleanup envelope.  A failure at any setup
        # boundary must not leave a suspended child or open pipe behind.
        process = subprocess.Popen(
            list(argv), cwd=root, env=env,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            start_new_session=os.name != "nt",
            creationflags=(
                getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
                | getattr(subprocess, "CREATE_SUSPENDED", 0x00000004)
                if os.name == "nt" else 0
            ),
        )
        job_handle = _assign_windows_job(process)
        # The callback reserves the one-shot cell only after Popen has returned
        # and containment has been assigned.  It must finish before resumption,
        # so a setup/resume failure can still roll the reservation back without
        # ever allowing candidate code to run against a consumed cell.
        if on_started is not None:
            launch_rollback = on_started()
            if launch_rollback is not None and not callable(launch_rollback):
                raise CrackHarnessError(
                    "launch callback must return a callable rollback or None"
                )
            if state_root_path is not None and state_manifest is not None:
                callback_manifest = _tree_manifest(state_root_path, (run_temp,))
                changed = {
                    key for key in set(state_manifest) | set(callback_manifest)
                    if state_manifest.get(key) != callback_manifest.get(key)
                }
                unexpected = changed - allowed_state_keys
                if unexpected:
                    summary = ", ".join(sorted(unexpected)[:8])
                    if len(unexpected) > 8:
                        summary += f" (+{len(unexpected) - 8} more)"
                    raise CrackHarnessError(
                        "reviewed command wrote outside its monitored run root: "
                        + summary
                    )
                # Preserve the pre-callback manifest and advance only the
                # explicitly allowed marker/ledger entries.  Concurrent child
                # writes to any other state path remain visible to monitoring.
                state_manifest = dict(state_manifest)
                for key in changed & allowed_state_keys:
                    if key in callback_manifest:
                        state_manifest[key] = callback_manifest[key]
                    else:
                        state_manifest.pop(key, None)
        readers = [
            threading.Thread(target=drain, args=("stdout", process.stdout), daemon=True),
            threading.Thread(target=drain, args=("stderr", process.stderr), daemon=True),
        ]
        for reader in readers:
            reader.start()
        _resume_windows_process(process, job_handle)
        # From this point onward the child has crossed the execution boundary;
        # retain the durable marker even if command/proof monitoring fails.
        launch_rollback = None
        while process.poll() is None:
            if time.monotonic() >= deadline:
                raise CrackHarnessError("active-time limit exceeded")
            if _tree_size(run_temp) > storage_limit:
                raise CrackHarnessError("temporary-storage limit exceeded")
            if overflow.is_set():
                raise CrackHarnessError("command output exceeded 1 MiB compact-output limit")
            if time.monotonic() >= next_manifest_check:
                if production_manifest is not None:
                    current_production = _repo_manifest(production_root, state_root)
                    if current_production != production_manifest:
                        raise CrackHarnessError(
                            "reviewed command wrote outside the disposable worktree: "
                            + _manifest_delta(production_manifest, current_production)
                        )
                if state_manifest is not None:
                    if _tree_manifest(state_root, (run_temp,)) != state_manifest:
                        raise CrackHarnessError("reviewed command wrote outside its monitored run root")
                next_manifest_check = time.monotonic() + 0.25
            time.sleep(0.02)
    except BaseException as primary:
        if launch_rollback is not None:
            try:
                launch_rollback()
            except BaseException as rollback_error:
                _note_secondary(primary, "launch reservation rollback", rollback_error)
        secondary = cleanup_resources(terminate=True)
        attach_failure_receipt(primary, secondary)
        if secondary and hasattr(primary, "add_note"):
            primary.add_note("secondary command cleanup: " + "; ".join(secondary))
        raise primary.with_traceback(primary.__traceback__)
    assert process is not None
    wait_error: BaseException | None = None
    try:
        process.wait(timeout=5.0)
    except BaseException as exc:
        wait_error = exc
    # A successfully exited parent still needs the containment job quiesced
    # with termination enabled so descendants cannot outlive the command.
    # _terminate_process itself is a no-op once the parent has exited.
    cleanup_errors = cleanup_resources(terminate=True)
    stdout = bytes(captured["stdout"]).decode("utf-8", errors="replace")
    stderr = bytes(captured["stderr"]).decode("utf-8", errors="replace")
    elapsed = round(time.monotonic() - started, 6)
    if production_manifest is not None:
        current_production = _repo_manifest(production_root, state_root)
        if current_production != production_manifest:
            primary = CrackHarnessError(
                "reviewed command wrote outside the disposable worktree: "
                + _manifest_delta(production_manifest, current_production)
            )
            attach_failure_receipt(primary, cleanup_errors)
            raise primary
    if state_manifest is not None and _tree_manifest(state_root, (run_temp,)) != state_manifest:
        primary = CrackHarnessError(
            "reviewed command wrote outside its monitored run root"
        )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    if wait_error is not None:
        attach_failure_receipt(wait_error, cleanup_errors)
        if cleanup_errors and hasattr(wait_error, "add_note"):
            wait_error.add_note(
                "secondary command cleanup: " + "; ".join(cleanup_errors)
            )
        raise wait_error.with_traceback(wait_error.__traceback__)
    if overflow.is_set() or len(captured["stdout"]) + len(captured["stderr"]) > 1024 * 1024:
        primary = CrackHarnessError(
            "command output exceeded 1 MiB compact-output limit"
        )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    receipt = {
        "argv_sha256": _digest_json(list(argv)),
        "returncode": process.returncode,
        "active_seconds": elapsed,
        "stdout_sha256": _digest_bytes(stdout.encode("utf-8")),
        "stderr_sha256": _digest_bytes(stderr.encode("utf-8")),
        "cleanup_errors": cleanup_errors[:8],
    }
    if process.returncode != 0:
        detail = stderr.strip() or stdout.strip() or "no diagnostic"
        primary = CrackHarnessError(
            f"reviewed command failed ({process.returncode}): {detail[:500]}"
        )
        if cleanup_errors and hasattr(primary, "add_note"):
            primary.add_note(
                "secondary command cleanup: " + "; ".join(cleanup_errors)
            )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    if cleanup_errors:
        primary = CrackHarnessError(
            "reviewed command exited successfully but process/output cleanup "
            "did not quiesce: " + "; ".join(cleanup_errors[:8])
        )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    if _tree_size(run_temp) > storage_limit:
        primary = CrackHarnessError("temporary-storage limit exceeded")
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    if not expect_json:
        return None, receipt
    try:
        value = json.loads(stdout)
    except json.JSONDecodeError as exc:
        primary = CrackHarnessError(
            "reviewed proof command did not emit one JSON object"
        )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary from exc
    if not isinstance(value, Mapping):
        primary = CrackHarnessError(
            "reviewed proof command output must be a JSON object"
        )
        attach_failure_receipt(primary, cleanup_errors)
        raise primary
    return dict(value), receipt


def _terminate_process(process: subprocess.Popen[str]) -> None:
    """Terminate the complete reviewed-command process group when possible."""
    if process.poll() is not None:
        return
    if os.name == "nt":
        job_handle = getattr(process, "_crack_harness_job", None)
        if job_handle:
            import ctypes
            ctypes.windll.kernel32.TerminateJobObject(job_handle, 1)
        completed = subprocess.run(
            ["taskkill", "/PID", str(process.pid), "/T", "/F"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
        )
        if completed.returncode != 0 and process.poll() is None:
            process.kill()
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    try:
        process.wait(timeout=5.0)
    except subprocess.TimeoutExpired:
        process.kill()
        try:
            process.wait(timeout=5.0)
        except subprocess.TimeoutExpired as exc:
            raise CrackHarnessError("reviewed process did not quiesce after termination") from exc


def _quiesce_windows_job(
    handle: int | None, *, terminate: bool, timeout: float = 5.0,
) -> None:
    if not handle or os.name != "nt":
        return
    import ctypes
    from ctypes import wintypes

    class BASIC_ACCOUNTING(ctypes.Structure):
        _fields_ = [
            ("TotalUserTime", ctypes.c_longlong),
            ("TotalKernelTime", ctypes.c_longlong),
            ("ThisPeriodTotalUserTime", ctypes.c_longlong),
            ("ThisPeriodTotalKernelTime", ctypes.c_longlong),
            ("TotalPageFaultCount", wintypes.DWORD),
            ("TotalProcesses", wintypes.DWORD),
            ("ActiveProcesses", wintypes.DWORD),
            ("TotalTerminatedProcesses", wintypes.DWORD),
        ]

    kernel = ctypes.windll.kernel32
    if terminate and not kernel.TerminateJobObject(handle, 1):
        raise CrackHarnessError("cannot terminate reviewed Windows Job")
    deadline = time.monotonic() + timeout
    accounting = BASIC_ACCOUNTING()
    while True:
        if not kernel.QueryInformationJobObject(
            handle, 1, ctypes.byref(accounting), ctypes.sizeof(accounting), None
        ):
            raise CrackHarnessError("cannot query reviewed Windows Job quiescence")
        if accounting.ActiveProcesses == 0:
            return
        if time.monotonic() >= deadline:
            raise CrackHarnessError("reviewed Windows Job retained live descendants")
        time.sleep(0.02)


def _assign_windows_job(process: subprocess.Popen[Any]) -> int | None:
    """Put the command in a kill-on-close Job so exited parents cannot orphan children."""
    if os.name != "nt":
        return None
    import ctypes
    from ctypes import wintypes

    class BASIC_LIMITS(ctypes.Structure):
        _fields_ = [
            ("PerProcessUserTimeLimit", ctypes.c_longlong),
            ("PerJobUserTimeLimit", ctypes.c_longlong),
            ("LimitFlags", wintypes.DWORD),
            ("MinimumWorkingSetSize", ctypes.c_size_t),
            ("MaximumWorkingSetSize", ctypes.c_size_t),
            ("ActiveProcessLimit", wintypes.DWORD),
            ("Affinity", ctypes.c_size_t),
            ("PriorityClass", wintypes.DWORD),
            ("SchedulingClass", wintypes.DWORD),
        ]

    class IO_COUNTERS(ctypes.Structure):
        _fields_ = [(name, ctypes.c_ulonglong) for name in (
            "ReadOperationCount", "WriteOperationCount", "OtherOperationCount",
            "ReadTransferCount", "WriteTransferCount", "OtherTransferCount",
        )]

    class EXTENDED_LIMITS(ctypes.Structure):
        _fields_ = [
            ("BasicLimitInformation", BASIC_LIMITS),
            ("IoInfo", IO_COUNTERS),
            ("ProcessMemoryLimit", ctypes.c_size_t),
            ("JobMemoryLimit", ctypes.c_size_t),
            ("PeakProcessMemoryUsed", ctypes.c_size_t),
            ("PeakJobMemoryUsed", ctypes.c_size_t),
        ]

    kernel = ctypes.windll.kernel32
    kernel.CreateJobObjectW.restype = wintypes.HANDLE
    handle = kernel.CreateJobObjectW(None, None)
    if not handle:
        process.kill()
        raise CrackHarnessError("cannot create Windows containment job")
    limits = EXTENDED_LIMITS()
    limits.BasicLimitInformation.LimitFlags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    if not kernel.SetInformationJobObject(handle, 9, ctypes.byref(limits), ctypes.sizeof(limits)):
        kernel.CloseHandle(handle); process.kill()
        raise CrackHarnessError("cannot configure Windows containment job")
    if not kernel.AssignProcessToJobObject(handle, wintypes.HANDLE(process._handle)):
        kernel.CloseHandle(handle); process.kill()
        raise CrackHarnessError("cannot assign reviewed command to Windows containment job")
    process._crack_harness_job = handle
    return handle


def _resume_windows_process(process: subprocess.Popen[Any], job_handle: int | None) -> None:
    """Resume only after Job assignment, closing the Windows child-spawn race."""
    if os.name != "nt":
        return
    import ctypes
    from ctypes import wintypes

    # subprocess closes the primary-thread handle; NtResumeProcess safely resumes
    # every thread while the process handle remains owned by Popen.
    result = ctypes.windll.ntdll.NtResumeProcess(wintypes.HANDLE(process._handle))
    if result != 0:
        if job_handle:
            ctypes.windll.kernel32.TerminateJobObject(job_handle, 1)
        process.kill()
        _close_windows_job(job_handle)
        raise CrackHarnessError("cannot resume contained Windows reviewed command")


def _close_windows_job(handle: int | None) -> None:
    if handle and os.name == "nt":
        import ctypes
        ctypes.windll.kernel32.CloseHandle(handle)


def _repo_manifest(root: Path, state: Path) -> dict[str, tuple[int, int]]:
    return _tree_manifest(root, (state,))


def _manifest_delta(
    before: Mapping[str, tuple[int, int]], after: Mapping[str, tuple[int, int]],
) -> str:
    changed = sorted(
        path for path in set(before) | set(after) if before.get(path) != after.get(path)
    )
    if not changed:
        return "unknown path"
    summary = ", ".join(changed[:8])
    return summary + (f" (+{len(changed) - 8} more)" if len(changed) > 8 else "")


def _tree_manifest(
    root: Path, excluded: Sequence[Path] = (),
) -> dict[str, tuple[int, int]]:
    result: dict[str, tuple[int, int]] = {}
    for parent, dirs, files in os.walk(root, followlinks=False):
        base = Path(parent)
        retained_dirs = []
        for name in dirs:
            candidate = base / name
            info = candidate.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(
                getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
            ):
                raise CrackHarnessError(f"tree path indirection is forbidden: {candidate}")
            if not any(_inside(item, candidate) for item in excluded):
                retained_dirs.append(name)
        dirs[:] = retained_dirs
        for name in files:
            path = base / name
            if any(_inside(item, path) for item in excluded):
                continue
            info = path.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(
                getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
            ):
                raise CrackHarnessError(f"tree path indirection is forbidden: {path}")
            result[path.relative_to(root).as_posix()] = (info.st_size, info.st_mtime_ns)
    return result


def _compact_receipt(name: str, payload: Mapping[str, Any], command: Mapping[str, Any]) -> dict[str, Any]:
    if name == "record":
        if len(_canonical(payload)) > 64 * 1024 or not all(
            isinstance(value, (str, int, float, bool)) or value is None
            for value in payload.values()
        ):
            raise CrackHarnessError("central record receipt exceeds its compact closed form")
        summary = dict(payload)
    else:
        summary = {}
    for key in (() if name == "record" else sorted(payload, key=str)[:32]):
        value = payload[key]
        if isinstance(value, (str, int, float, bool)) or value is None:
            summary[str(key)] = value if not isinstance(value, str) else value[:500]
        elif isinstance(value, list) and len(value) <= 64 and all(
            isinstance(item, (str, int, float, bool)) or item is None for item in value
        ):
            summary[str(key)] = [item[:200] if isinstance(item, str) else item for item in value]
    return {
        "schema": "crack_harness_receipt/v1",
        "hook": name,
        "ok": payload.get("ok", True),
        "summary": summary,
        "payload_sha256": _digest_json(payload),
        "command": dict(command),
    }


def _validate_proof(
    name: str, payload: Mapping[str, Any], approval: Mapping[str, Any],
    object_pair: tuple[str, str] | None,
) -> tuple[dict[str, Any], tuple[str, str], bool]:
    common = {
        "schema", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "report_sha256",
    }
    extra = {
        "strict": {"strict_percent", "target_bytes", "candidate_bytes", "differences"},
        "data": {"data_percent", "target_bytes", "candidate_bytes", "differences"},
        "focus": {"differing_rows"},
        "siblings": {"protected_total", "protected_losses"},
        "physical": {"target_count", "candidate_count", "differences"},
    }[name]
    if set(payload) != common | extra or payload.get("schema") != f"crack_proof_{name}/v1":
        raise CrackHarnessError(f"{name} proof payload is not the strict typed schema")
    for key, expected in (
        ("owner", approval["owner"]), ("function", approval["function"]),
        ("candidate_source_sha256", approval["candidate"]["sha256"]),
    ):
        if payload.get(key) != expected:
            raise CrackHarnessError(f"{name} proof does not bind {key}")
    target = _sha(payload.get("target_object_sha256"), f"{name}.target_object_sha256")
    candidate = _sha(payload.get("candidate_object_sha256"), f"{name}.candidate_object_sha256")
    _sha(payload.get("report_sha256"), f"{name}.report_sha256")
    pair = (target, candidate)
    if target != approval["target_sha256"]:
        raise CrackHarnessError(f"{name} proof target object is not the approved target")
    if object_pair is not None and pair != object_pair:
        raise CrackHarnessError("proof hooks disagree on target/candidate object identity")
    for key in extra:
        value = payload.get(key)
        if (
            isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(float(value))
            or value < 0
        ):
            raise CrackHarnessError(f"{name}.{key} must be non-negative numeric evidence")
    if name == "strict":
        exact = payload["strict_percent"] == 100 and payload["differences"] == 0 and payload["target_bytes"] == payload["candidate_bytes"]
    elif name == "data":
        exact = payload["data_percent"] == 100 and payload["differences"] == 0 and payload["target_bytes"] == payload["candidate_bytes"]
    elif name == "focus":
        exact = payload["differing_rows"] == 0
    elif name == "siblings":
        exact = payload["protected_losses"] == 0
    else:
        exact = payload["differences"] == 0 and payload["target_count"] == payload["candidate_count"]
    return dict(payload), pair, exact


def _validate_assessment(
    payload: Mapping[str, Any], approval: Mapping[str, Any],
    object_pair: tuple[str, str],
) -> float:
    if set(payload) != {"schema", "owner", "function", "candidate_source_sha256", "target_object_sha256", "candidate_object_sha256", "owner_gain", "data_gain", "data_diff_delta", "physical_diff_delta"} or payload.get("schema") != "crack_assessment/v1":
        raise CrackHarnessError("assessment payload is not the strict typed schema")
    if (
        payload.get("owner") != approval["owner"]
        or payload.get("function") != approval["function"]
        or payload.get("candidate_source_sha256") != approval["candidate"]["sha256"]
        or payload.get("target_object_sha256") != object_pair[0]
        or payload.get("candidate_object_sha256") != object_pair[1]
    ):
        raise CrackHarnessError("assessment does not bind this candidate")
    gain = payload.get("owner_gain")
    data_gain = payload.get("data_gain")
    if any(
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(float(value))
        for value in (gain, data_gain)
    ):
        raise CrackHarnessError("assessment gains must be finite numeric values")
    for field in ("data_diff_delta", "physical_diff_delta"):
        if isinstance(payload.get(field), bool) or not isinstance(payload.get(field), int):
            raise CrackHarnessError(f"assessment.{field} must be an integer")
    return float(gain)


def _validate_record(
    payload: Mapping[str, Any], approval: Mapping[str, Any], outcome: str,
    object_pair: tuple[str, str], admission_token: str, admission_input_key: str,
) -> None:
    required = {
        "schema", "recorded", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "outcome",
        "admission_token_sha256", "admission_input_key", "record_sha256",
    }
    if set(payload) != required or payload.get("schema") != "crack_central_record_receipt/v1" or payload.get("recorded") is not True:
        raise CrackHarnessError("central outcome was not recorded with a strict typed receipt")
    expected = {
        "owner": approval["owner"], "function": approval["function"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "target_object_sha256": object_pair[0],
        "candidate_object_sha256": object_pair[1], "outcome": outcome,
        "admission_token_sha256": _digest_bytes(admission_token.encode("utf-8")),
        "admission_input_key": admission_input_key,
    }
    if any(payload.get(key) != value for key, value in expected.items()):
        raise CrackHarnessError("central record receipt does not bind the measured outcome")
    _sha(payload.get("record_sha256"), "record_sha256")


def _run_canonical_record(
    root: Path, approval: Mapping[str, Any], outcome: str,
    object_pair: tuple[str, str], admission_token: str, admission_input_key: str,
    proof_payloads: Mapping[str, Mapping[str, Any]], assessment: Mapping[str, Any],
    *, run_temp: Path, deadline: float, storage_limit: int,
) -> tuple[dict[str, Any], dict[str, Any]]:
    descriptor = approval["commands"]["record"]
    report_sha = proof_payloads["strict"]["report_sha256"]
    argv = [
        str(descriptor["executable"]), str(descriptor["script"]),
        "--root", str(root), "record",
        "--owner", approval["owner"], "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--object-sha256", object_pair[1], "--status", outcome,
        "--reason", f"bounded harness outcome: {outcome}",
        "--admission-token", admission_token,
        "--candidate-id", approval["approval_id"],
        "--candidate-record-sha256", _digest_json(assessment),
        "--strict-report-sha256", proof_payloads["strict"]["report_sha256"],
        "--data-report-sha256", proof_payloads["data"]["report_sha256"],
        "--report-sha256", report_sha, "--workspace", "crack-harness",
        "--json",
    ]
    canonical, command = _run_command(
        argv, root=root, run_temp=run_temp, deadline=deadline,
        storage_limit=storage_limit, expect_json=True,
    )
    if not isinstance(canonical, Mapping):
        raise CrackHarnessError(
            "canonical recovery memory did not return an authenticated experiment row"
        )
    if canonical.get("status") != "recorded" or canonical.get("authority_advanced") is not False:
        raise CrackHarnessError("canonical recovery memory did not record exactly one measured outcome")
    experiment = canonical.get("experiment")
    if not isinstance(experiment, Mapping):
        raise CrackHarnessError(
            "canonical recovery memory did not return its authenticated experiment row"
        )
    required_experiment = {
        "input_key", "owner", "function_name", "target_sha256",
        "source_sha256", "object_sha256", "status", "record_sha256",
    }
    if not required_experiment <= set(experiment):
        raise CrackHarnessError(
            "canonical recovery memory experiment row is incomplete"
        )
    expected_experiment = {
        "input_key": admission_input_key,
        "owner": approval["owner"],
        "function_name": approval["function"],
        "target_sha256": approval["target_sha256"],
        "source_sha256": approval["candidate"]["sha256"],
        "object_sha256": object_pair[1],
        "status": outcome,
    }
    if any(
        experiment.get(key) != expected
        for key, expected in expected_experiment.items()
    ):
        raise CrackHarnessError(
            "canonical recovery memory experiment row does not bind the measured outcome"
        )
    central_record_sha256 = _sha(
        experiment.get("record_sha256"),
        "central experiment.record_sha256",
    )
    payload = {
        "schema": "crack_central_record_receipt/v1", "recorded": True,
        "owner": approval["owner"], "function": approval["function"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "target_object_sha256": object_pair[0],
        "candidate_object_sha256": object_pair[1], "outcome": outcome,
        "admission_token_sha256": _digest_bytes(admission_token.encode("utf-8")),
        "admission_input_key": admission_input_key,
        # This is the central DB experiment digest, not a digest of the CLI
        # response envelope. Recovery must authenticate against this value.
        "record_sha256": central_record_sha256,
    }
    return payload, command


def _run_canonical_discard(
    root: Path, approval: Mapping[str, Any], admission_token: str,
    admission_input_key: str,
    *, run_temp: Path, deadline: float, storage_limit: int,
) -> tuple[dict[str, Any], dict[str, Any]]:
    descriptor = approval["commands"]["record"]
    argv = [
        str(descriptor["executable"]), str(descriptor["script"]),
        "--root", str(root), "discard",
        "--owner", approval["owner"], "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--admission-token", admission_token, "--json",
    ]
    payload, command = _run_command(
        argv, root=root, run_temp=run_temp, deadline=deadline,
        storage_limit=storage_limit, expect_json=True,
    )
    assert payload is not None
    if (
        set(payload) != {"status", "input_key", "authority_advanced"}
        or payload.get("status") not in {"discarded", "missing"}
        or payload.get("input_key") != admission_input_key
    ):
        raise CrackHarnessError("canonical recovery memory did not discard the pending admission")
    _sha(payload.get("input_key"), "discard input_key")
    if payload.get("authority_advanced") is not False:
        raise CrackHarnessError("discard unexpectedly advanced central authority")
    return dict(payload), command


def _run_dir(state_root: Path, approval: Mapping[str, Any]) -> Path:
    owner = re.sub(r"[^A-Za-z0-9_.-]+", "_", approval["owner"]).strip("_") or "owner"
    owner += "-" + _digest_bytes(str(approval["owner"]).encode("utf-8"))[:12]
    function = re.sub(r"[^A-Za-z0-9_.-]+", "_", approval["function"]).strip("_")
    return state_root / "owners" / owner / function / "latest"


def _result_path(state_root: Path, approval: Mapping[str, Any]) -> Path:
    return _run_dir(state_root, approval) / "result.json"


def _function_results(state_root: Path, approval: Mapping[str, Any]) -> list[dict[str, Any]]:
    path = _result_path(state_root, approval)
    _state_run_dir(state_root, path.parent, "function result run")
    _state_path(state_root, path, "function result", exists=None)
    if not path.is_file():
        return []
    value = _read_json(path)
    if (
        isinstance(value, Mapping)
        and value.get("owner") == approval["owner"]
        and value.get("function") == approval["function"]
    ):
        binding = _terminal_binding_from_result(path.parent, value)
        root = approval.get("_root")
        if not isinstance(root, Path):
            root = state_root.parent
        if binding is not None and _valid_terminal_result(
            root, path, path.parent / "record.commit.json", binding, approval
        ):
            return [dict(value)]
    return []


def _function_key(approval: Mapping[str, Any]) -> str:
    return _digest_json({
        "owner": approval["owner"], "function": approval["function"],
    })


def _permit_attempts(run_dir: Path, approval: Mapping[str, Any]) -> list[str]:
    """Load the bounded one-shot permit ledger for one owner/function."""

    path = run_dir.parent / "permit-attempts.json"
    if not path.is_file():
        return []
    _assert_no_indirection(path)
    value = _read_json(path)
    required = {
        "schema", "function_key", "owner", "function", "permit_sha256s",
    }
    permits = value.get("permit_sha256s") if isinstance(value, Mapping) else None
    valid = (
        isinstance(value, Mapping)
        and set(value) == required
        and value.get("schema") == "crack_harness_permit_attempts/v1"
        and value.get("function_key") == _function_key(approval)
        and value.get("owner") == approval["owner"]
        and value.get("function") == approval["function"]
        and isinstance(permits, list)
        and len(permits) <= MAX_PERMIT_ATTEMPTS_PER_FUNCTION
        and len(set(permits)) == len(permits)
        and all(isinstance(item, str) and SHA_RE.fullmatch(item) for item in permits)
    )
    if not valid:
        raise CrackHarnessError("permit-attempt ledger is malformed or unbound")
    return list(permits)


def _permit_attempted(run_dir: Path, approval: Mapping[str, Any]) -> bool:
    return approval["permit_sha256"] in _permit_attempts(run_dir, approval)


def _consume_permit(run_dir: Path, approval: Mapping[str, Any]) -> None:
    """Consume exactly one signed permit without consuming the function cell."""

    path = run_dir.parent / "permit-attempts.json"
    permits = _permit_attempts(run_dir, approval)
    permit_sha256 = approval["permit_sha256"]
    if permit_sha256 in permits:
        raise CrackHarnessError("signed permit has already been attempted")
    if len(permits) >= MAX_PERMIT_ATTEMPTS_PER_FUNCTION:
        raise CrackHarnessError("function permit-attempt cap is exhausted")
    permits.append(permit_sha256)
    _safe_mkdir(path.parent)
    _atomic_json(path, {
        "schema": "crack_harness_permit_attempts/v1",
        "function_key": _function_key(approval),
        "owner": approval["owner"], "function": approval["function"],
        "permit_sha256s": permits,
    })


def _state_from_run_dir(run_dir: Path) -> Path:
    run_dir = Path(os.path.abspath(run_dir))
    # A fresh approved cell has no ``latest`` directory yet.  Validate every
    # existing parent and permit only that final leaf to be absent.
    _assert_no_indirection(run_dir, missing_leaf=True)
    try:
        state = run_dir.parents[3]
    except IndexError as exc:
        raise CrackHarnessError("function run directory is outside harness state") from exc
    state = Path(os.path.abspath(state))
    if not _inside(state, run_dir):
        raise CrackHarnessError("function run directory escapes its state root")
    _assert_no_indirection(state)
    if state.name != "crack-harness" and state.name != "state":
        # Unit tests intentionally use a temporary directory named state.
        if not (state / "owners").exists():
            raise CrackHarnessError("function run directory has invalid state ancestry")
    _state_run_dir(state, run_dir)
    return state


def _consumed_cell_records(state: Path) -> list[dict[str, Any]]:
    path = state / "consumed-cells.json"
    if not path.is_file():
        return []
    _assert_no_indirection(path)
    value = _read_json(path)
    if (
        not isinstance(value, Mapping)
        or set(value) != {"schema", "records"}
        or value.get("schema") != "crack_harness_consumed_cells/v1"
        or not isinstance(value.get("records"), list)
        or len(value["records"]) > MAX_CONSUMED_CELLS
    ):
        raise CrackHarnessError("central consumed-cell ledger is malformed")
    required = {
        "function_key", "owner", "function", "candidate_sha256",
        "first_campaign_id", "consumed_at",
    }
    records: list[dict[str, Any]] = []
    identities: set[tuple[str, str]] = set()
    for item in value["records"]:
        if not isinstance(item, Mapping) or set(item) != required:
            raise CrackHarnessError("central consumed-cell record is malformed")
        function_key = _sha(item.get("function_key"), "consumed function_key")
        candidate_sha256 = _sha(
            item.get("candidate_sha256"), "consumed candidate_sha256"
        )
        _text(item.get("owner"), "consumed owner")
        _text(item.get("function"), "consumed function")
        _text(item.get("first_campaign_id"), "consumed first_campaign_id")
        _timestamp(item.get("consumed_at"), "consumed_at")
        identity = (function_key, candidate_sha256)
        if identity in identities:
            raise CrackHarnessError("central consumed-cell ledger contains a duplicate")
        identities.add(identity)
        records.append(dict(item))
    return records


def _append_consumed_cell(
    state: Path, approval: Mapping[str, Any], *,
    first_campaign_id: str | None = None,
) -> None:
    records = _consumed_cell_records(state)
    identity = (_function_key(approval), approval["candidate"]["sha256"])
    if any(
        (item["function_key"], item["candidate_sha256"]) == identity
        for item in records
    ):
        return
    if len(records) >= MAX_CONSUMED_CELLS:
        raise CrackHarnessError("central consumed-cell ledger reached its hard cap")
    records.append({
        "function_key": identity[0], "owner": approval["owner"],
        "function": approval["function"], "candidate_sha256": identity[1],
        "first_campaign_id": first_campaign_id or approval["campaign"]["id"],
        "consumed_at": _now(),
    })
    _atomic_json(state / "consumed-cells.json", {
        "schema": "crack_harness_consumed_cells/v1", "records": records,
    })


def _consumed_cell_ledger_snapshot(state: Path) -> tuple[Path, bytes | None]:
    """Capture the central ledger before reserving a launch cell."""

    path = state / "consumed-cells.json"
    _assert_no_indirection(state)
    _assert_no_indirection(path, missing_leaf=True)
    if not path.exists():
        return path, None
    if not path.is_file():
        raise CrackHarnessError(f"central consumed-cell ledger is not a regular file: {path}")
    return path, path.read_bytes()


def _read_function_tombstone(
    run_dir: Path, approval: Mapping[str, Any],
) -> tuple[Path | None, Mapping[str, Any] | None]:
    path = run_dir.parent / "latest-function.json"
    if path.is_file():
        _assert_no_indirection(path)
        value = _read_json(path)
        v1 = {
            "schema", "function_key", "owner", "function",
            "first_campaign_id", "consumed",
        }
        v2 = v1 | {
            "approval_sha256", "base_sha256", "candidate_sha256",
            "candidate_execution_started", "consumed_at",
        }
        if not isinstance(value, Mapping) or frozenset(value) not in {
            frozenset(v1), frozenset(v2),
        }:
            raise CrackHarnessError("function tombstone is malformed")
        valid = (
            value.get("schema") in {
                "crack_harness_function_tombstone/v1",
                "crack_harness_function_tombstone/v2",
            }
            and value.get("function_key") == _function_key(approval)
            and value.get("owner") == approval["owner"]
            and value.get("function") == approval["function"]
            and isinstance(value.get("first_campaign_id"), str)
            and bool(value.get("first_campaign_id"))
            and value.get("consumed") is True
        )
        if value.get("schema") == "crack_harness_function_tombstone/v2":
            valid = (
                valid
                and SHA_RE.fullmatch(str(value.get("approval_sha256"))) is not None
                and SHA_RE.fullmatch(str(value.get("base_sha256"))) is not None
                and SHA_RE.fullmatch(str(value.get("candidate_sha256"))) is not None
                and value.get("candidate_execution_started") is True
            )
            _timestamp(value.get("consumed_at"), "function tombstone consumed_at")
        if not valid:
            raise CrackHarnessError("function tombstone binding is invalid")
        return path, value
    legacy = run_dir.parent / "latest-campaign.json"
    if not legacy.is_file():
        return None, None
    _assert_no_indirection(legacy)
    value = _read_json(legacy)
    if (
        not isinstance(value, Mapping)
        or set(value) != {"schema", "campaign_key", "campaign_id", "consumed"}
        or value.get("schema") != "crack_harness_campaign_tombstone/v1"
        or SHA_RE.fullmatch(str(value.get("campaign_key"))) is None
        or not isinstance(value.get("campaign_id"), str)
        or not value.get("campaign_id")
        or value.get("consumed") is not True
    ):
        raise CrackHarnessError("legacy campaign tombstone is malformed")
    return legacy, value


def _retry_used_record(
    run_dir: Path, approval: Mapping[str, Any],
) -> Mapping[str, Any] | None:
    """Read the durable one-shot marker for a legacy reconciliation.

    The marker is intentionally independent of the current approval: after a
    retry has run, a newly issued approval must still see the consumed marker
    and remain blocked.  Its identity and all referenced artifacts are still
    checked against the owner/function and the repository root.
    """

    path = run_dir.parent / "retry-used.json"
    if not path.is_file():
        return None
    _assert_no_indirection(path)
    value = _read_json(path)
    if not isinstance(value, Mapping):
        raise CrackHarnessError("legacy retry-used marker is malformed")
    body = dict(value)
    digest = body.pop("retry_sha256", None)
    required = {
        "schema", "function_key", "owner", "function", "approval_sha256",
        "prior_approval_sha256", "candidate_sha256", "legacy_controller_commit",
        "tombstone_path", "tombstone_sha256", "failure_path", "failure_sha256",
        "historical_exact_evidence_path", "historical_exact_evidence_sha256",
        "used_at",
    }
    if (
        set(body) != required
        or body.get("schema") != RETRY_USED_SCHEMA
        or digest != _digest_json(body)
        or body.get("function_key") != _function_key(approval)
        or body.get("owner") != approval["owner"]
        or body.get("function") != approval["function"]
    ):
        raise CrackHarnessError("legacy retry-used marker is unbound or invalid")
    for key in (
        "approval_sha256", "prior_approval_sha256", "candidate_sha256",
        "tombstone_sha256", "failure_sha256",
        "historical_exact_evidence_sha256",
    ):
        _sha(body.get(key), f"retry-used.{key}")
    _text(body.get("legacy_controller_commit"), "retry-used.legacy_controller_commit")
    _timestamp(body.get("used_at"), "retry-used.used_at")
    root = approval.get("_root")
    if not isinstance(root, Path):
        raise CrackHarnessError("legacy retry-used marker lacks approval root")
    for key in ("tombstone_path", "failure_path", "historical_exact_evidence_path"):
        path_value = body.get(key)
        if not isinstance(path_value, str) or not path_value:
            raise CrackHarnessError(f"retry-used.{key} must be a path")
        _bound_path(root, path_value, f"retry-used.{key}")
    return dict(value)


def _revalidate_legacy_historical_evidence(
    approval: Mapping[str, Any],
) -> dict[str, Any]:
    """Revalidate legacy exact evidence immediately before retry consumption.

    ``load_approval`` validates this evidence while the approval is being
    loaded, but the candidate launch can happen later.  Keep the original
    approval descriptor as the binding authority and repeat the path/hash and
    compact-content checks at the execution boundary.  The parsed value cached
    in ``_retry`` is compared as well so an in-memory descriptor drift cannot
    silently change which historical proof is consumed.
    """

    root = approval.get("_root")
    retry = approval.get("_retry")
    raw_retry = approval.get("retry")
    if not isinstance(root, Path) or not isinstance(retry, Mapping):
        raise CrackHarnessError(
            "legacy retry lacks bound historical exact evidence"
        )
    if not isinstance(raw_retry, Mapping):
        raise CrackHarnessError(
            "legacy retry approval descriptor lacks historical exact evidence"
        )
    descriptor = raw_retry.get("historical_exact_evidence")
    evidence_path, evidence_sha = _retry_file_descriptor(
        root, descriptor, "retry.historical_exact_evidence"
    )
    if (
        evidence_path != retry.get("historical_exact_evidence_path")
        or evidence_sha != retry.get("historical_exact_evidence_sha256")
    ):
        raise CrackHarnessError(
            "legacy historical exact evidence binding changed after approval"
        )
    evidence = _historical_exact_values(
        _read_json(evidence_path),
        approval["owner"],
        approval["function"],
        approval["candidate"]["sha256"],
        label="retry.historical_exact_evidence",
    )
    if evidence != retry.get("historical_exact_evidence"):
        raise CrackHarnessError(
            "legacy historical exact evidence content changed after approval"
        )
    if evidence["legacy_controller_commit"] != retry.get(
        "legacy_controller_commit"
    ):
        raise CrackHarnessError(
            "retry legacy_controller_commit does not match historical exact evidence"
        )
    return evidence


def _legacy_reconciliation_eligible(
    run_dir: Path, approval: Mapping[str, Any],
) -> bool:
    """Return whether this approval may consume one legacy v1 retry.

    A v1 tombstone is normally a permanent guard.  The only exception is a
    manager-signed approval carrying the validated retry descriptor, an exact
    historical proof, and a matching prior failure.  v2 tombstones never enter
    this branch.
    """

    retry = approval.get("_retry")
    if retry is None:
        return False
    if _retry_used_record(run_dir, approval) is not None:
        return False
    _revalidate_legacy_historical_evidence(approval)
    tombstone_path, tombstone = _read_function_tombstone(run_dir, approval)
    if (
        tombstone_path is None
        or not isinstance(tombstone, Mapping)
        or tombstone.get("schema") != "crack_harness_function_tombstone/v1"
    ):
        return False
    expected_tombstone = run_dir.parent / "latest-function.json"
    if tombstone_path != expected_tombstone:
        raise CrackHarnessError("legacy retry must bind the current v1 function tombstone")
    if _digest_file(tombstone_path) != retry["tombstone_sha256"]:
        raise CrackHarnessError("legacy v1 tombstone changed after approval")

    failure_path = run_dir.parent / "latest-failure.json"
    if retry["failure_path"] != failure_path:
        raise CrackHarnessError("legacy retry must bind the current failure diagnostic")
    if _digest_file(failure_path) != retry["failure_sha256"]:
        raise CrackHarnessError("legacy failure diagnostic changed after approval")
    if retry["prior_approval_sha256"] == approval["_approval_sha256"]:
        raise CrackHarnessError("legacy retry prior approval must differ from current approval")

    state = _state_from_run_dir(run_dir)
    key = _function_key(approval)
    old_campaign = tombstone.get("first_campaign_id")
    records = _consumed_cell_records(state)
    for item in records:
        if item["function_key"] != key:
            continue
        if (
            item["candidate_sha256"] != retry["candidate_sha256"]
            or item["first_campaign_id"] != old_campaign
        ):
            raise CrackHarnessError(
                "central consumed-cell ledger conflicts with legacy retry"
            )
        # A matching central marker means this exact one-shot reconciliation
        # already crossed its execution boundary.  This remains consumed even
        # if a crash prevented the local retry-used marker from being sealed.
        return False
    return True


def _rollback_marker(
    marker_path: Path, marker_value: Mapping[str, Any], primary: BaseException,
) -> None:
    """Best-effort marker rollback that never masks the primary failure.

    A local marker may be removed only when central publication failed before
    candidate execution.  If removal itself is uncertain, restore the marker
    so a retry fails closed rather than allowing a second lifetime cell.
    """

    try:
        _safe_unlink(marker_path)
        return
    except BaseException as exc:
        _note_secondary(primary, "marker rollback", exc)
    try:
        _atomic_json(marker_path, dict(marker_value))
    except BaseException as exc:
        _note_secondary(primary, "marker rollback restoration", exc)


def _retain_marker(
    marker_path: Path, marker_value: Mapping[str, Any], primary: BaseException,
) -> None:
    """Keep a marker after the candidate launch boundary has been crossed."""

    try:
        _assert_no_indirection(marker_path)
        if not marker_path.is_file():
            raise CrackHarnessError("execution marker is not a regular file")
    except BaseException as exc:
        # A marker that disappeared or became unreadable must be restored when
        # possible.  Keeping the marker is the conservative outcome because
        # the child has already been launched and may have executed.
        _note_secondary(primary, "marker retention", exc)
        try:
            _atomic_json(marker_path, dict(marker_value))
        except BaseException as restore_error:
            _note_secondary(primary, "marker retention restoration", restore_error)


def _marker_reservation_rollback(
    marker_path: Path, marker_value: Mapping[str, Any],
    ledger_path: Path, ledger_before: bytes | None, ledger_after: bytes,
) -> Callable[[], None]:
    """Return an exact rollback for a fully published, pre-resume reservation.

    The rollback is only valid while both marker files still contain the bytes
    published by the reservation.  Refusing to overwrite a changed ledger or
    marker preserves the fail-closed one-shot guard if another writer raced the
    setup boundary.
    """

    marker_after = _digest_json(marker_value)

    def rollback() -> None:
        _assert_no_indirection(marker_path)
        if (
            not marker_path.is_file()
            or _digest_json(_read_json(marker_path)) != marker_after
        ):
            raise CrackHarnessError("launch marker changed before reservation rollback")
        _assert_no_indirection(ledger_path, missing_leaf=True)
        if not ledger_path.is_file() or ledger_path.read_bytes() != ledger_after:
            raise CrackHarnessError("central consumed-cell ledger changed before reservation rollback")
        if ledger_before is None:
            _safe_unlink(ledger_path)
        else:
            _atomic_bytes(ledger_path, ledger_before)
        _safe_unlink(marker_path)

    return rollback


def _consume_legacy_retry(
    run_dir: Path, approval: Mapping[str, Any], *, execution_started: bool = False,
) -> Callable[[], None] | None:
    """Atomically consume a validated legacy retry at its execution boundary."""

    if not _legacy_reconciliation_eligible(run_dir, approval):
        raise CrackHarnessError("legacy retry is not eligible for this v1 tombstone")
    retry = approval["_retry"]
    root = approval["_root"]
    marker_body = {
        "schema": RETRY_USED_SCHEMA,
        "function_key": _function_key(approval),
        "owner": approval["owner"], "function": approval["function"],
        "approval_sha256": approval["_approval_sha256"],
        "prior_approval_sha256": retry["prior_approval_sha256"],
        "candidate_sha256": retry["candidate_sha256"],
        "legacy_controller_commit": retry["legacy_controller_commit"],
        "tombstone_path": retry["tombstone_path"].relative_to(root).as_posix(),
        "tombstone_sha256": retry["tombstone_sha256"],
        "failure_path": retry["failure_path"].relative_to(root).as_posix(),
        "failure_sha256": retry["failure_sha256"],
        "historical_exact_evidence_path": retry[
            "historical_exact_evidence_path"
        ].relative_to(root).as_posix(),
        "historical_exact_evidence_sha256": retry[
            "historical_exact_evidence_sha256"
        ],
        "used_at": _now(),
    }
    marker_path = run_dir.parent / "retry-used.json"
    marker_value = {
        **marker_body, "retry_sha256": _digest_json(marker_body),
    }
    ledger_path, ledger_before = _consumed_cell_ledger_snapshot(
        _state_from_run_dir(run_dir)
    )
    _atomic_json(marker_path, marker_value)
    try:
        tombstone = _read_json(retry["tombstone_path"])
        assert isinstance(tombstone, Mapping)
        _append_consumed_cell(
            _state_from_run_dir(run_dir), approval,
            first_campaign_id=_text(
                tombstone.get("first_campaign_id"),
                "legacy tombstone first_campaign_id",
            ),
        )
        ledger_after = ledger_path.read_bytes()
    except BaseException as primary:
        # The central ledger is the second half of the one-shot transaction.
        # Once the child has crossed its launch boundary, retain the local
        # marker even if central publication fails: candidate execution may
        # already have begun.  Direct pre-launch callers may safely roll it
        # back, preserving the historical retry behavior.
        if execution_started:
            _retain_marker(marker_path, marker_value, primary)
        else:
            _rollback_marker(marker_path, marker_value, primary)
        raise
    return _marker_reservation_rollback(
        marker_path, marker_value, ledger_path, ledger_before, ledger_after
    )


def _function_consumed(run_dir: Path, approval: Mapping[str, Any]) -> bool:
    if _retry_used_record(run_dir, approval) is not None:
        return True
    _, tombstone = _read_function_tombstone(run_dir, approval)
    if tombstone is not None:
        if (
            tombstone.get("schema") == "crack_harness_function_tombstone/v1"
            and _legacy_reconciliation_eligible(run_dir, approval)
        ):
            return False
        if tombstone.get("schema") == "crack_harness_function_tombstone/v1":
            return True
        if tombstone.get("candidate_sha256") == approval["candidate"]["sha256"]:
            return True
    key = _function_key(approval)
    return any(
        item["function_key"] == key
        and item["candidate_sha256"] == approval["candidate"]["sha256"]
        for item in _consumed_cell_records(_state_from_run_dir(run_dir))
    )


def _consume_function(
    run_dir: Path, approval: Mapping[str, Any], *, execution_started: bool = False,
) -> Callable[[], None] | None:
    path = run_dir.parent / "latest-function.json"
    _safe_mkdir(path.parent)
    if _function_consumed(run_dir, approval):
        raise CrackHarnessError("function already consumed its one lifetime cell")
    if _legacy_reconciliation_eligible(run_dir, approval):
        return _consume_legacy_retry(
            run_dir, approval, execution_started=execution_started
        )
    tombstone = {
        "schema": "crack_harness_function_tombstone/v2",
        "function_key": _function_key(approval),
        "owner": approval["owner"], "function": approval["function"],
        "first_campaign_id": approval["campaign"]["id"], "consumed": True,
        "approval_sha256": approval["_approval_sha256"],
        "base_sha256": approval["base"]["sha256"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "candidate_execution_started": True,
        "consumed_at": _now(),
    }
    ledger_path, ledger_before = _consumed_cell_ledger_snapshot(
        _state_from_run_dir(run_dir)
    )
    _atomic_json(path, tombstone)
    try:
        _append_consumed_cell(_state_from_run_dir(run_dir), approval)
        ledger_after = ledger_path.read_bytes()
    except BaseException as primary:
        # Central publication is part of consuming the v2 cell.  Once the
        # child has crossed its launch boundary, retain the local guard because
        # execution may have begun; pre-launch direct callers may roll it back.
        if execution_started:
            _retain_marker(path, tombstone, primary)
        else:
            _rollback_marker(path, tombstone, primary)
        raise
    return _marker_reservation_rollback(
        path, tombstone, ledger_path, ledger_before, ledger_after
    )


def _dry_run_core(root: Path, approval_path: Path, state: Path) -> dict[str, Any]:
    _assert_no_indirection(Path(os.path.abspath(root)))
    approval = load_approval(root, approval_path, allow_applied_source=True)
    existing = _result_path(state, approval)
    results = _function_results(state, approval)
    blockers = []
    if (state / "PACKET_ROLLBACK_REQUIRED.json").exists():
        blockers.append("manager packet rollback requires repair")
    if results:
        blockers.append("function already has a terminal result")
    if _function_consumed(_run_dir(state, approval), approval):
        blockers.append("function tombstone forbids another lifetime cell")
    if _permit_attempted(_run_dir(state, approval), approval):
        blockers.append("signed permit already attempted; issue a fresh permit")
    if approval.get("_source_applied") and not existing.exists():
        blockers.append("source is already the candidate without a terminal result")
    if any(item.get("status") in {"failed", "no_gain"} for item in results):
        blockers.append("function is terminal after a failed or no-gain candidate")
    if len(results) >= approval["campaign"].get("quota", 1):
        blockers.append("function lifetime quota exhausted")
    return {
        "schema": "crack_harness_dry_run/v1",
        "status": "ready" if not blockers else "blocked",
        "approval_id": approval["approval_id"],
        "approval_sha256": approval["_approval_sha256"],
        "owner": approval["owner"],
        "task_id": approval["task_id"],
        "base_commit": approval["base_commit"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "predicted_rows": approval["predicted_rows"],
        "limits": approval["limits"],
        "blockers": blockers,
        "authority_advanced": False,
    }


def dry_run(root: Path, approval_path: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _dry_run_core(root, approval_path, _state_root(root))


def _dry_run_for_test(root: Path, approval_path: Path, *, state_root: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _dry_run_core(root, approval_path, _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN))


def _cleanup_raw(run_dir: Path) -> None:
    for name in ("temp", "raw", "logs"):
        path = run_dir / name
        if path.exists():
            _assert_no_indirection(path)
            shutil.rmtree(path)


FUNCTION_GUARD_FILES = {
    "latest-function.json", "latest-campaign.json", "permit-attempts.json",
    "retry-used.json", "latest-frontier.json", "frontier.pending.json",
}


def _assert_retention_directory(state: Path, path: Path) -> bool:
    """Validate one retention entry before metadata or deletion is touched."""

    path = Path(os.path.abspath(path))
    if not _inside(Path(os.path.abspath(state)), path):
        raise CrackHarnessError(f"retention path escapes harness state: {path}")
    _assert_no_indirection(path)
    return True


def _prune_function_state(function_dir: Path) -> None:
    """Drop bulky/history state while retaining compact one-shot guards."""

    _assert_no_indirection(function_dir)
    guards = {
        child.name for child in function_dir.iterdir()
        if child.name in FUNCTION_GUARD_FILES and child.is_file()
    }
    if not guards:
        shutil.rmtree(function_dir)
        return
    for child in tuple(function_dir.iterdir()):
        if child.name in guards:
            _assert_no_indirection(child)
            continue
        _assert_no_indirection(child)
        if child.is_dir():
            shutil.rmtree(child)
        else:
            child.unlink()


def _gc_owner(
    run_dir: Path, byte_limit: int, *, protected: set[Path] | None = None,
) -> None:
    run_dir = Path(os.path.abspath(run_dir))
    _assert_no_indirection(run_dir)
    state = _state_from_run_dir(run_dir)
    _state_run_dir(state, run_dir)
    owner_dir = run_dir.parents[1]
    if not _inside(state, owner_dir):
        raise CrackHarnessError(f"owner retention path escapes harness state: {owner_dir}")
    _assert_no_indirection(owner_dir)
    protected_paths = {Path(os.path.abspath(item)) for item in (protected or set())}
    for item in protected_paths:
        if not _inside(state, item):
            raise CrackHarnessError(f"protected retention path escapes harness state: {item}")
        _assert_no_indirection(item, missing_leaf=True)
    entries = sorted(
        (
            item for item in owner_dir.iterdir()
            if (
                _assert_retention_directory(state, item)
                and item.is_dir()
                and Path(os.path.abspath(item)) not in protected_paths
            )
        ),
        key=lambda item: item.stat().st_mtime,
    )
    while _tree_size(owner_dir) > byte_limit and entries:
        victim = entries.pop(0)
        _prune_function_state(victim)
    if _tree_size(owner_dir) > byte_limit:
        raise CrackHarnessError("owner retained state exceeds the hard 16 MiB cap")


def _gc_global(
    state: Path, byte_limit: int, *, protected: set[Path] | None = None,
) -> None:
    state = Path(os.path.abspath(state))
    _assert_no_indirection(state)
    owners = state / "owners"
    if not owners.exists():
        return
    _assert_no_indirection(owners)
    protected_paths = {Path(os.path.abspath(item)) for item in (protected or set())}
    for item in protected_paths:
        if not _inside(state, item):
            raise CrackHarnessError(f"protected retention path escapes harness state: {item}")
        _assert_no_indirection(item, missing_leaf=True)
    entries = sorted(
        (
            function_dir
            for owner_dir in owners.iterdir()
            if _assert_retention_directory(state, owner_dir) and owner_dir.is_dir()
            for function_dir in owner_dir.iterdir()
            if (
                _assert_retention_directory(state, function_dir)
                and function_dir.is_dir()
                and Path(os.path.abspath(function_dir)) not in protected_paths
            )
        ),
        key=lambda item: item.stat().st_mtime,
    )
    while _tree_size(state) > byte_limit and entries:
        victim = entries.pop(0)
        _prune_function_state(victim)
    if _tree_size(state) > byte_limit:
        raise CrackHarnessError("global retained state exceeds the hard 64 MiB cap")


def _run_approved_core(
    root: Path, approval_path: Path, *, permit_path: Path, state: Path,
    manager_key_path: Path, expected_key_id: str,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    approval = load_approval(root, approval_path, allow_applied_source=True)
    _safe_mkdir(state)
    permit, permit_file = _load_permit(
        root, approval, permit_path, state, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    run_dir = _run_dir(state, approval)
    _state_run_dir(state, run_dir, "approved function run")
    transaction_lock = _state_path(
        state, state / ".transaction.lock", "transaction lock", exists=None
    )
    with serialized_build_lock(transaction_lock, 55.0):
        try:
            return _run_locked(
                root, approval_path, approval, permit, permit_file, state,
                run_dir, manager_key_path=manager_key_path,
                expected_key_id=expected_key_id,
            )
        except BaseException as primary:
            cleanup_errors: list[str] = []
            preserve_recovery = bool(
                getattr(primary, "_crack_recovery_required", False)
            )
            try:
                if (state / "transaction.json").exists():
                    _recover_interrupted(root, state)
            except BaseException as exc:
                cleanup_errors.append(f"recovery: {exc}"[:1000])
            preserve_recovery = preserve_recovery or (
                state / "RECOVERY_REQUIRED.json"
            ).is_file()
            if preserve_recovery:
                # Recovery owns all source/approval/temp artifacts once an
                # exact central row may exist.  In particular, do not let the
                # generic failure cleanup erase the journal that binds the row
                # or roll the candidate back underneath it.
                if cleanup_errors and hasattr(primary, "add_note"):
                    primary.add_note(
                        "recovery diagnostics: " + "; ".join(cleanup_errors)
                    )
                raise primary
            try:
                _cleanup_raw(run_dir)
            except BaseException as exc:
                cleanup_errors.append(f"raw cleanup: {exc}"[:1000])
            for disposable in (
                approval["_paths"]["base"], approval["_paths"]["candidate"],
                permit_file, approval["_approval_path"],
            ):
                if cleanup_errors:
                    break
                try:
                    if disposable.exists() and not _is_tracked(root, disposable):
                        disposable.unlink()
                except BaseException as exc:
                    cleanup_errors.append(f"delete {disposable.name}: {exc}"[:1000])
            command_receipt = getattr(primary, "_crack_command_receipt", None)
            failure_receipt = (
                {
                    "schema": "crack_harness_failure_receipt/v1",
                    "hook": "infrastructure",
                    "reason": str(primary)[:1000],
                    "argv_sha256": command_receipt.get("argv_sha256"),
                    "command": dict(command_receipt),
                }
                if isinstance(command_receipt, Mapping) else None
            )
            diagnostic_body = {
                "schema": "crack_harness_failure_diagnostic/v2",
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"], "function": approval["function"],
                "primary_reason": str(primary)[:1000],
                "failure_receipt": failure_receipt,
                "cleanup_errors": cleanup_errors[:8], "finished_at": _now(),
            }
            try:
                _atomic_json(run_dir.parent / "latest-failure.json", {
                    **diagnostic_body,
                    "diagnostic_sha256": _digest_json(diagnostic_body),
                })
            except BaseException as exc:
                cleanup_errors.append(f"diagnostic seal: {exc}"[:1000])
            if not cleanup_errors:
                try:
                    (state / "attempt.json").unlink(missing_ok=True)
                except BaseException as exc:
                    cleanup_errors.append(f"delete attempt: {exc}"[:1000])
            if cleanup_errors and hasattr(primary, "add_note"):
                primary.add_note("cleanup diagnostics: " + "; ".join(cleanup_errors))
            raise


def run_approved(
    root: Path, approval_path: Path, *, permit_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _run_approved_core(
        root, approval_path, permit_path=permit_path, state=_state_root(root),
        manager_key_path=MANAGER_PERMIT_KEY, expected_key_id=MANAGER_KEY_ID,
    )


def _run_approved_for_test(
    root: Path, approval_path: Path, *, permit_path: Path, state_root: Path,
    manager_key_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    state = _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN)
    validated_key = _manager_key_file(root, manager_key_path)
    return _run_approved_core(
        root, approval_path, permit_path=permit_path, state=state,
        manager_key_path=manager_key_path, expected_key_id=_digest_file(validated_key),
    )


def _run_locked(
    root: Path, approval_path: Path, approval: dict[str, Any],
    permit: Mapping[str, Any], permit_file: Path, state: Path, run_dir: Path, *,
    manager_key_path: Path, expected_key_id: str,
) -> dict[str, Any]:
    _state_run_dir(state, run_dir, "approved function run")
    if (state / "PACKET_ROLLBACK_REQUIRED.json").exists():
        raise CrackHarnessError("manager packet rollback requires repair")
    _recover_interrupted(root, state)
    _recover_pending_frontiers(
        root, state, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    if (
        (state / "RECOVERY_REQUIRED.json").is_file()
        or (state / "transaction.json").is_file()
    ):
        raise CrackHarnessError(
            "recorded interrupted winner requires manager recovery review"
        )
    _scavenge_disposable_worktrees(
        root, state, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    maintenance_errors = _retry_retention_maintenance(
        state, root, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    if maintenance_errors:
        raise CrackHarnessError(
            "retained cleanup/maintenance remains incomplete: "
            + "; ".join(maintenance_errors)
        )
    if (state / "RECOVERY_REQUIRED.json").exists():
        raise CrackHarnessError("recorded interrupted winner requires manager recovery review")
    current_frontier = _frontier_file(run_dir)
    if current_frontier.exists() and not current_frontier.is_file():
        raise CrackHarnessError("partial frontier path is not a regular file")
    if current_frontier.is_file():
        retained = _validate_frontier(
            root, current_frontier,
            manager_key_path=manager_key_path,
            expected_key_id=expected_key_id,
            require_live_source=True,
        )
        _validate_frontier_continuation(root, run_dir, retained, approval)
    readiness = _dry_run_core(root, approval_path, state)
    if readiness["status"] != "ready":
        raise CrackHarnessError("; ".join(readiness["blockers"]))
    _verify_repository(root, approval, allow_source=False)
    for disposable in (approval["_paths"]["base"], approval["_paths"]["candidate"], approval["_approval_path"], permit_file):
        if _is_tracked(root, disposable):
            raise CrackHarnessError(f"disposable approval artifact is tracked and cannot be deleted: {disposable}")
    if _function_consumed(run_dir, approval):
        raise CrackHarnessError("function already consumed its one lifetime cell")
    if run_dir.exists():
        _assert_no_indirection(run_dir)
        shutil.rmtree(run_dir)
    attempt_body = {
        "schema": ATTEMPT_SCHEMA,
        "run_dir": str(run_dir),
        "source_path": str(approval["_paths"]["source"]),
        "approval_path": str(approval["_approval_path"]),
        "approval_sha256": approval["_approval_sha256"],
        "disposable_paths": [
            str(approval["_paths"]["base"]), str(approval["_paths"]["candidate"]),
            str(permit_file), str(approval["_approval_path"]),
        ],
    }
    _consume_permit(run_dir, approval)
    attempt_value = _sign_attempt_receipt(
        root, attempt_body, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    _atomic_json(state / "attempt.json", attempt_value)
    _safe_mkdir(run_dir)
    temp = run_dir / "temp"
    _safe_mkdir(temp)
    worktree = temp / "worktree"
    out_root = temp / "out"
    _safe_mkdir(out_root)
    paths = approval["_paths"]
    context_body = {
        "schema": "crack_evidence_context/v1", "owner": approval["owner"],
        "function": approval["function"], "unit": approval["unit"],
        "source_relpath": paths["source"].relative_to(root).as_posix(),
        "target_sha256": approval["target_sha256"],
        "base_source_sha256": approval["base"]["sha256"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "toolchain_key": approval["toolchain_key"], "issued_at": approval["issued_at"],
        "objdiff": OBJDIFF_PIN,
    }
    context = {**context_body, "context_sha256": _digest_json(context_body)}
    context_path = out_root / "approval-context.json"
    _atomic_json(context_path, context)
    remaining_assignment = (
        _timestamp(permit["deadline"], "permit deadline") - datetime.now(timezone.utc)
    ).total_seconds()
    if remaining_assignment <= 0:
        raise CrackHarnessError("assignment permit deadline elapsed before compile")
    deadline = time.monotonic() + min(
        approval["limits"]["active_seconds"], remaining_assignment
    )
    receipts: dict[str, Any] = {}
    proof_payloads: dict[str, Any] = {}
    status = "failed"
    reason = "unclassified failure"
    expected_terminal = approval["selection"]["expected_terminal"]
    terminal_expectation_met = False
    assessment: dict[str, Any] = {}
    proof_exact: dict[str, bool] = {}
    object_pair: tuple[str, str] | None = None
    admission_token = ""
    admission_input_key = ""
    admission_closed = False
    source_replaced = False
    central_record_succeeded = False
    transaction_value: dict[str, Any] | None = None
    frontier_value: dict[str, Any] | None = None
    frontier_commit_crossed = False
    recovery_required = False
    active_hook = "precompile"
    secondary_failures: list[str] = []
    primary_exception: BaseException | None = None
    try:
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        admission, receipt = _run_command(
            approval["commands"]["precompile"]["argv"], root=root, run_temp=temp,
            deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
            expect_json=True
        )
        assert admission is not None
        receipts["precompile"] = _compact_receipt("precompile", admission, receipt)
        admission_fields = {
            "status", "reused", "skip_compile", "input_key", "admission_token",
            "expires_at", "authority_advanced",
        }
        if set(admission) != admission_fields or admission.get("status") != "admitted" or admission.get("skip_compile") is not False or admission.get("authority_advanced") is not False:
            raise CrackHarnessError(f"canonical precompile admission denied: {admission.get('status')}")
        admission_token = _text(admission.get("admission_token"), "precompile admission_token")
        admission_input_key = _sha(admission.get("input_key"), "precompile input_key")
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        _create_disposable_worktree(root, worktree, approval["base_commit"])
        if _tree_size(temp) > approval["limits"]["temporary_bytes"]:
            raise CrackHarnessError("disposable worktree exceeds the hard ephemeral-storage limit")
        worktree_source = worktree / paths["source"].relative_to(root)
        # The authoritative working source may be a retained dirty frontier and
        # therefore need not exist at ``base_commit``.  Overlay the separately
        # sealed, hash-bound base before the baseline build.
        _atomic_copy(paths["base"], worktree_source)
        if _digest_file(worktree_source) != approval["base"]["sha256"]:
            raise CrackHarnessError(
                "disposable worktree source does not equal sealed baseline"
            )
        lock = state / ".serialized-compile.lock"
        with serialized_build_lock(lock, min(55.0, max(0.1, deadline - time.monotonic()))):
            active_hook = "compile"
            compile_argv = _expand_argv(
                approval["commands"]["compile"]["argv"], worktree, out_root, root
            )
            evidence_env = {
                "CRACK_HARNESS_OUT_ROOT": str(out_root),
                "CRACK_HARNESS_OWNER": approval["owner"],
                "CRACK_HARNESS_FUNCTION": approval["function"],
                "CRACK_HARNESS_UNIT": approval["unit"],
                "CRACK_HARNESS_SOURCE_PATH": paths["source"].relative_to(root).as_posix(),
                "CRACK_HARNESS_TARGET_SHA256": approval["target_sha256"],
                "CRACK_HARNESS_BASE_COMMIT": approval["base_commit"],
                "CRACK_HARNESS_APPROVAL_SHA256": approval["_approval_sha256"],
                "CRACK_HARNESS_CONTEXT_PATH": str(context_path),
                "CRACK_HARNESS_CONTEXT_SHA256": context["context_sha256"],
                "CRACK_HARNESS_ISSUED_AT": approval["issued_at"],
            }
            _clear_evidence(out_root, (*EVIDENCE_BASELINE_FILES, *EVIDENCE_CANDIDATE_FILES, "evidence-context.json"))
            baseline_env = {
                **evidence_env, "CRACK_HARNESS_PHASE": "baseline",
                "CRACK_HARNESS_PHASE_NONCE": _digest_bytes((context["context_sha256"] + ":baseline").encode()),
            }
            _, baseline_receipt = _run_command(
                compile_argv, root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=False, production_root=root, state_root=state,
                extra_env=baseline_env,
            )
            baseline_hashes = _evidence_hashes(out_root, EVIDENCE_BASELINE_FILES)
            sealed_baseline_receipt = _validate_evidence_receipt(
                out_root, approval, context, "baseline"
            )
            _validate_evidence_context(out_root, approval, context, completed=False)
            if _digest_file(out_root / "target.o") != approval["target_sha256"]:
                raise CrackHarnessError("baseline evidence target.o is not the approved target")
            if any((out_root / name).exists() for name in EVIDENCE_CANDIDATE_FILES):
                raise CrackHarnessError("baseline evidence phase emitted candidate outputs")
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            _atomic_copy(paths["candidate"], worktree_source)
            _clear_evidence(out_root, EVIDENCE_CANDIDATE_FILES)
            candidate_env = {
                **evidence_env, "CRACK_HARNESS_PHASE": "candidate",
                "CRACK_HARNESS_PHASE_NONCE": _digest_bytes((context["context_sha256"] + ":candidate").encode()),
            }
            _, candidate_receipt = _run_command(
                compile_argv, root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=False, production_root=root, state_root=state,
                extra_env=candidate_env,
                # _run_command invokes this after Popen/containment and before
                # resume.  Its returned rollback is used if setup/resume fails.
                on_started=lambda: _consume_function(run_dir, approval),
                on_started_state_paths=(
                    run_dir.parent / "latest-function.json",
                    run_dir.parent / "retry-used.json",
                    state / "consumed-cells.json",
                ),
            )
            if _evidence_hashes(out_root, EVIDENCE_BASELINE_FILES) != baseline_hashes:
                raise CrackHarnessError("candidate evidence phase changed sealed baseline outputs")
            _evidence_hashes(out_root, EVIDENCE_CANDIDATE_FILES)
            if _validate_evidence_receipt(out_root, approval, context, "baseline") != sealed_baseline_receipt:
                raise CrackHarnessError("candidate evidence phase changed the baseline receipt")
            _validate_evidence_receipt(out_root, approval, context, "candidate")
            _validate_evidence_context(out_root, approval, context, completed=True)
            receipts["compile"] = {
                "schema": "crack_harness_receipt/v1", "hook": "compile",
                "baseline_command": baseline_receipt, "candidate_command": candidate_receipt,
            }
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            for name in HOOKS:
                active_hook = name
                payload, command_receipt = _run_command(
                    _expand_argv(approval["commands"][name]["argv"], worktree, out_root, root), root=worktree, run_temp=temp,
                    deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                    expect_json=True, production_root=root, state_root=state
                )
                assert payload is not None
                typed, object_pair, is_exact = _validate_proof(name, payload, approval, object_pair)
                receipts[name] = _compact_receipt(name, payload, command_receipt)
                proof_payloads[name] = typed
                proof_exact[name] = is_exact
                _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            active_hook = "assess"
            payload, command_receipt = _run_command(
                _expand_argv(approval["commands"]["assess"]["argv"], worktree, out_root, root), root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=True, production_root=root, state_root=state
            )
            assert payload is not None
            assert object_pair is not None
            gain = _validate_assessment(payload, approval, object_pair)
            assessment = dict(payload)
            receipts["assess"] = _compact_receipt("assess", payload, command_receipt)
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        assert object_pair is not None
        nonregression = (
            proof_exact.get("siblings") is True
            and float(assessment["data_gain"]) >= 0
            and assessment["data_diff_delta"] <= 0
            and assessment["physical_diff_delta"] <= 0
            and proof_payloads["data"]["target_bytes"] == proof_payloads["data"]["candidate_bytes"]
        )
        exact = gain > 0 and nonregression and all(proof_exact.values())
        if exact:
            status = "exact"
            reason = "measurable owner gain and every approved proof passed"
        elif gain <= 0:
            status = "no_gain"
            reason = "candidate produced no measurable owner gain"
        elif not nonregression:
            status = "no_gain"
            reason = "positive focus gain rejected because a closed proof channel regressed"
        else:
            status = "improved"
            reason = "measurable non-regressing partial frontier retained"
        terminal_expectation_met = (
            status == "exact"
            or (status == "improved" and expected_terminal == "improved")
        )
        if status == "improved" and expected_terminal == "exact":
            # ``expected_terminal`` is the sealed winning-cell prediction, not
            # permission to destroy a measured gain. Keep the monotonic
            # frontier, but explicitly record that the exact prediction missed.
            reason = (
                "exact terminal expectation unmet; measurable non-regressing "
                "partial frontier retained"
            )
        if status == "exact":
            baseline_snapshot = temp / "baseline.snapshot"
            _atomic_copy(paths["base"], baseline_snapshot)
            transaction_body = {
                "schema": TRANSACTION_SCHEMA,
                "approval_path": str(approval["_approval_path"]),
                "approval_id": approval["approval_id"],
                "approval_identity_sha256": approval["_permit_identity_sha256"],
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"], "function": approval["function"],
                "source_relpath": paths["source"].relative_to(root).as_posix(),
                "source_sha256": approval["source"]["sha256"],
                "base_relpath": paths["base"].relative_to(root).as_posix(),
                "baseline_snapshot": str(baseline_snapshot), "baseline_sha256": approval["base"]["sha256"],
                "base_sha256": approval["base"]["sha256"],
                "base_commit": approval["base_commit"],
                "candidate_relpath": paths["candidate"].relative_to(root).as_posix(),
                "target_object_sha256": approval["target_sha256"],
                "candidate_sha256": approval["candidate"]["sha256"],
                "result_path": str(run_dir / "result.json"),
                "report_path": str(run_dir / "CRACK_REPORT_v1.json"),
                "worktree": str(worktree),
                "record_commit_path": str(run_dir / "record.commit.json"),
                "central_record_binding": {
                    "input_key": admission_input_key,
                    "owner": approval["owner"],
                    "function": approval["function"],
                    "source_sha256": approval["candidate"]["sha256"],
                    "target_object_sha256": approval["target_sha256"],
                    "object_sha256": object_pair[1],
                    "candidate_record_sha256": _digest_json(assessment),
                    "status": status,
                },
            }
            transaction_value = {
                **transaction_body,
                "transaction_sha256": _digest_json(transaction_body),
            }
            _atomic_json(state / "transaction.json", transaction_value)
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            _atomic_copy(worktree_source, paths["source"])
            source_replaced = True
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=True)
        if status == "exact":
            active_hook = "record"
            record_payload, record_command = _run_canonical_record(
                root, approval, status, object_pair, admission_token, admission_input_key,
                proof_payloads, assessment, run_temp=temp, deadline=deadline,
                storage_limit=approval["limits"]["temporary_bytes"],
            )
            # The canonical command may already have committed its central row
            # even if response validation or a later local write fails.
            central_record_succeeded = True
            _validate_record(record_payload, approval, status, object_pair, admission_token, admission_input_key)
            receipts["record"] = _compact_receipt("record", record_payload, record_command)
            admission_closed = True
            commit_body = {
                "schema": "crack_harness_record_commit/v1", "outcome": status,
                "candidate_sha256": approval["candidate"]["sha256"],
                "record_payload_sha256": receipts["record"]["payload_sha256"],
                "record_sha256": receipts["record"]["summary"]["record_sha256"],
            }
            if transaction_value is None:
                raise CrackHarnessError(
                    "exact central record has no bound transaction journal"
                )
            recorded_transaction = {
                **transaction_value,
                "record_succeeded": True,
                "record_sha256": commit_body["record_sha256"],
            }
            recorded_transaction_body = dict(recorded_transaction)
            recorded_transaction_body.pop("transaction_sha256", None)
            new_transaction_value = {
                **recorded_transaction_body,
                "transaction_sha256": _digest_json(recorded_transaction_body),
            }
            _atomic_json(state / "transaction.json", new_transaction_value)
            transaction_value = new_transaction_value
            _atomic_json(run_dir / "record.commit.json", {**commit_body, "commit_sha256": _digest_json(commit_body)})
        else:
            active_hook = "discard"
            _checkpoint(
                root, approval["_approval_path"], approval, permit_file,
                permit, state, allow_source=source_replaced,
            )
            discard_payload, discard_command = _run_canonical_discard(
                root, approval, admission_token, admission_input_key,
                run_temp=temp, deadline=deadline,
                storage_limit=approval["limits"]["temporary_bytes"],
            )
            receipts["discard"] = _compact_receipt("discard", discard_payload, discard_command)
            admission_closed = True
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=source_replaced)
        if status == "improved":
            # Seal the compact frontier before changing the tracked source.  A
            # crash can therefore leave only (base + pending) or
            # (candidate + pending); startup deterministically deletes or
            # finalizes that single pending receipt without candidate history.
            frontier_value = _sign_frontier(
                root, run_dir, approval, object_pair, proof_payloads, assessment,
                manager_key_path=manager_key_path,
                expected_key_id=expected_key_id,
            )
            pending_frontier = _frontier_pending_file(run_dir)
            _atomic_json(pending_frontier, frontier_value)
            _validate_frontier(
                root, pending_frontier, manager_key_path=manager_key_path,
                expected_key_id=expected_key_id, require_live_source=False,
                approval=approval,
            )
            _atomic_copy(paths["candidate"], paths["source"])
            source_replaced = True
            frontier_commit_crossed = True
            try:
                os.replace(pending_frontier, _frontier_file(run_dir))
                _directory_fsync(_frontier_file(run_dir).parent)
            except BaseException as frontier_exc:
                # The signed pending file plus atomically replaced source are a
                # recoverable retained outcome.  Preserve them and attach the
                # finalization issue instead of rolling back a measured gain.
                secondary_failures.append(
                    f"frontier publication: {frontier_exc}"[:1000]
                )
            frontier_path = (
                pending_frontier
                if pending_frontier.is_file()
                else _frontier_file(run_dir)
            )
            _validate_frontier(
                root, frontier_path, manager_key_path=manager_key_path,
                expected_key_id=expected_key_id, require_live_source=True,
                approval=approval,
            )
    except (Exception, KeyboardInterrupt) as exc:
        primary_exception = exc
        reason = str(exc)
        status = "failed"
        descriptor = approval["commands"].get(active_hook, {})
        receipts["failure"] = {
            "schema": "crack_harness_failure_receipt/v1",
            "hook": active_hook,
            "reason": reason[:1000],
            "argv_sha256": _digest_json(descriptor.get("argv", [])),
            "command": (
                dict(getattr(exc, "_crack_command_receipt"))
                if isinstance(
                    getattr(exc, "_crack_command_receipt", None), Mapping
                ) else None
            ),
        }
        # A canonical record command can commit the central exact row before
        # its response, local receipt, commit marker, or checkpoint fails.  If
        # that boundary is known (or the authenticated central row is visible
        # after an ambiguous command failure), preserve the candidate and the
        # journal and switch to startup recovery.  Rolling back here would
        # create the exact central/local split-brain this journal prevents.
        central_record_uncertain = False
        if not central_record_succeeded and transaction_value is not None:
            transaction_binding = _transaction_central_binding(
                transaction_value, approval
            )
            if transaction_binding is not None:
                try:
                    central_record_succeeded = _central_record_matches(
                        root, transaction_binding
                    )
                except CrackHarnessError as query_exc:
                    # The record command may have committed before its response
                    # became locally visible.  An unavailable central query is
                    # an ambiguity boundary, never evidence that the row is
                    # absent.  Preserve source+journal for deterministic retry.
                    central_record_uncertain = True
                    secondary_failures.append(
                        f"central record query: {query_exc}"[:1000]
                    )
        if central_record_succeeded or central_record_uncertain:
            recovery_required = True
            try:
                journal_value = _read_json(state / "transaction.json")
                if not isinstance(journal_value, Mapping):
                    raise CrackHarnessError(
                        "post-record transaction journal is unavailable"
                    )
                _write_recovery_required(
                    root, state, journal_value, reason,
                    record_sha256=(
                        journal_value.get("record_sha256")
                        if journal_value.get("record_succeeded") is True
                        else None
                    ),
                )
            except BaseException as marker_exc:
                secondary_failures.append(
                    f"recovery marker: {marker_exc}"[:1000]
                )
            try:
                setattr(primary_exception, "_crack_recovery_required", True)
            except BaseException:
                pass
        elif frontier_commit_crossed:
            # A validated signed pending frontier existed before the atomic
            # source replacement.  Once both are present, rollback would erase
            # a measured gain.  Preserve candidate+pending; startup recovery
            # will either publish that exact frontier or fail closed on drift.
            recovery_required = True
            try:
                setattr(primary_exception, "_crack_recovery_required", True)
            except BaseException:
                pass
        elif source_replaced:
            try:
                _atomic_copy(paths["base"], paths["source"])
                source_replaced = False
            except BaseException as rollback_exc:
                secondary_failures.append(f"source rollback: {rollback_exc}")
                recovery_required = True
                try:
                    journal_value = _read_json(state / "transaction.json")
                    if not isinstance(journal_value, Mapping):
                        raise CrackHarnessError(
                            "rollback recovery transaction journal is unavailable"
                        )
                    _write_recovery_required(
                        root, state, journal_value,
                        "candidate source rollback failed; local recovery required",
                    )
                    setattr(primary_exception, "_crack_recovery_required", True)
                except BaseException as marker_exc:
                    secondary_failures.append(
                        f"rollback recovery marker: {marker_exc}"[:1000]
                    )
                    try:
                        setattr(primary_exception, "_crack_recovery_required", True)
                    except BaseException:
                        pass
        if admission_token and not admission_closed and not recovery_required:
            try:
                _checkpoint(
                    root, approval["_approval_path"], approval, permit_file,
                    permit, state, allow_source=source_replaced,
                )
                discard_payload, discard_command = _run_canonical_discard(
                    root, approval, admission_token, admission_input_key,
                    run_temp=temp, deadline=deadline,
                    storage_limit=approval["limits"]["temporary_bytes"],
                )
                receipts["discard"] = _compact_receipt("discard", discard_payload, discard_command)
                admission_closed = True
            except BaseException as discard_exc:
                secondary_failures.append(f"admission discard: {discard_exc}")
                receipts["discard_failure"] = {
                    "schema": "crack_harness_failure_receipt/v1", "hook": "discard",
                    "reason": str(discard_exc)[:1000],
                    "command": (
                        dict(getattr(discard_exc, "_crack_command_receipt"))
                        if isinstance(
                            getattr(discard_exc, "_crack_command_receipt", None),
                            Mapping,
                        ) else None
                    ),
                }
    if recovery_required:
        assert primary_exception is not None
        if secondary_failures and hasattr(primary_exception, "add_note"):
            primary_exception.add_note("; ".join(secondary_failures))
        raise primary_exception
    try:
        _checkpoint(
            root, approval["_approval_path"], approval, permit_file, permit, state,
            allow_source=source_replaced,
        )
    except BaseException as checkpoint_exc:
        if status != "failed":
            raise
        secondary_failures.append(f"terminal checkpoint: {checkpoint_exc}")
    if secondary_failures:
        receipts["secondary_failures"] = {
            "schema": "crack_harness_secondary_failures/v1",
            "items": [item[:1000] for item in secondary_failures[:8]],
        }
    if status == "failed" and source_replaced:
        assert primary_exception is not None
        if hasattr(primary_exception, "add_note"):
            primary_exception.add_note("; ".join(secondary_failures))
        raise primary_exception
    finished = _now()
    result_body = {
        "schema": RESULT_SCHEMA,
        "approval_id": approval["approval_id"],
        "approval_sha256": approval["_approval_sha256"],
        "owner": approval["owner"],
        "task_id": approval["task_id"],
        "function": approval["function"],
        "base_commit": approval["base_commit"],
        "campaign_id": approval["campaign"]["id"],
        "attempt_sha256": attempt_value["attempt_sha256"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "base_sha256": approval["base"]["sha256"],
        "status": status,
        "expected_terminal": expected_terminal,
        "terminal_expectation_met": terminal_expectation_met,
        "reason": reason,
        "owner_gain": assessment.get("owner_gain"),
        "predicted_rows": approval["predicted_rows"],
        "receipts": receipts,
        "finished_at": finished,
        "source_restored": status != "exact" and not source_replaced,
        "cleanup_status": "pending",
        "cleanup_errors": [],
        "authority_advanced": False,
        **(
            {"frontier_sha256": frontier_value["frontier_sha256"]}
            if status == "improved" and frontier_value is not None else {}
        ),
    }
    result = {**result_body, "result_sha256": _digest_json(result_body)}
    if len(_canonical(result)) > MAX_COMPACT_TERMINAL_BYTES:
        raise CrackHarnessError("terminal result exceeds the hard 1 MiB compact cap")
    if status == "exact":
        report_body = {
            "schema": REPORT_SCHEMA,
            "status": "exact",
            "completed": True,
            "authority_advanced": False,
            "owner": approval["owner"],
            "function": approval["function"],
            "task_id": approval["task_id"],
            "base_commit": approval["base_commit"],
            "approval_sha256": approval["_approval_sha256"],
            "source_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": object_pair[0],
            "candidate_object_sha256": object_pair[1],
            "result": {
                "strict_percent": proof_payloads["strict"]["strict_percent"],
                "data_percent": proof_payloads["data"]["data_percent"],
                "target_bytes": proof_payloads["strict"]["target_bytes"],
                "candidate_bytes": proof_payloads["strict"]["candidate_bytes"],
                "owner_gain": assessment["owner_gain"],
            },
            "proof_receipts": {
                name: {
                    "sha256": receipt["payload_sha256"],
                    "summary": receipt["summary"],
                }
                for name, receipt in receipts.items()
                if "payload_sha256" in receipt
            },
            "predicted_rows": approval["predicted_rows"],
            "completed_at": finished,
        }
        report = {**report_body, "report_sha256": _digest_json(report_body)}
        if len(_canonical(report)) > MAX_COMPACT_TERMINAL_BYTES:
            raise CrackHarnessError("exact report exceeds the hard 1 MiB compact cap")
        result["report_sha256"] = report["report_sha256"]
        unhashed = dict(result)
        unhashed.pop("result_sha256", None)
        result["result_sha256"] = _digest_json(unhashed)
        record_commit = _record_commit_value(run_dir / "record.commit.json")
        if record_commit is None:
            raise CrackHarnessError("exact central record commit is missing or invalid")
        _validate_exact_report(
            report,
            result,
            {
                "owner": approval["owner"],
                "function": approval["function"],
                "source_sha256": approval["candidate"]["sha256"],
                "target_object_sha256": approval["target_sha256"],
                "object_sha256": object_pair[1],
                "status": "exact",
            },
            record_commit=record_commit,
        )
        _atomic_json(run_dir / "CRACK_REPORT_v1.json", report)
        _atomic_json(run_dir / "result.json", result)
    journal = state / "transaction.json"
    cleanup_errors: list[str] = []

    def secondary(label: str, callback: Any) -> None:
        try:
            callback()
        except BaseException as exc:
            cleanup_errors.append(f"{label}: {exc}"[:1000])

    if status == "exact":
        # Keep the transaction journal until the exact result, root cleanup
        # receipt, and manager-bound disposable cleanup have succeeded.  The
        # retired partial frontier is secondary: remove it only after that
        # authenticated boundary so an unlink failure is retryable by status
        # instead of pinning the exact transaction journal forever.
        secondary(
            "record commit cleanup",
            lambda: (run_dir / "record.commit.json").unlink(missing_ok=True),
        )
        if worktree.exists():
            secondary(
                "disposable worktree cleanup",
                lambda: _remove_disposable_worktree(root, worktree),
            )
        secondary("raw/temp cleanup", lambda: _cleanup_raw(run_dir))
        if not cleanup_errors:
            # Authenticate the exact four root paths while every manager-bound
            # input still exists.  Only then may deletion begin.
            secondary(
                "seal root cleanup receipt",
                lambda: _seal_root_cleanup_receipt(
                    root, state, _read_json(state / "attempt.json"),
                    approval, manager_key_path=manager_key_path,
                    expected_key_id=expected_key_id,
                    required_exact=True,
                ),
            )
        if not cleanup_errors:
            for disposable in (paths["base"], paths["candidate"], permit_file):
                secondary(
                    f"delete {disposable.name}",
                    lambda item=disposable: _safe_unlink(item),
                )
                if cleanup_errors:
                    break
            if not cleanup_errors:
                # The journal is deleted before the approval.  Thus a crash can
                # leave either (journal + authenticated approval) for recovery,
                # or (attempt + approval) for scavenging, but never a live
                # journal whose approval authority has disappeared.
                secondary("transaction journal cleanup", journal.unlink)
            if not cleanup_errors:
                secondary(
                    f"delete {approval['_approval_path'].name}",
                    lambda: _safe_unlink(approval["_approval_path"]),
                )
            if not cleanup_errors:
                secondary(
                    "delete attempt receipt",
                    lambda: _safe_unlink(state / "attempt.json"),
                )
        if not cleanup_errors:
            secondary(
                "retired partial frontier cleanup",
                lambda: _safe_unlink(_frontier_file(run_dir)),
            )
        secondary(
            "owner retention maintenance",
            lambda: _gc_owner(
                run_dir, MAX_RETAINED_OWNER_BYTES,
                protected={run_dir.parent},
            ),
        )
        secondary(
            "global retention maintenance",
            lambda: _gc_global(
                state, MAX_RETAINED_GLOBAL_BYTES,
                protected={run_dir.parent},
            ),
        )
        terminal_body = dict(result)
        terminal_body.pop("result_sha256", None)
        terminal_body["cleanup_status"] = (
            "cleanup_incomplete" if cleanup_errors else "complete"
        )
        terminal_body["cleanup_errors"] = cleanup_errors[:8]
        result = {**terminal_body, "result_sha256": _digest_json(terminal_body)}
        try:
            _atomic_json(run_dir / "result.json", result)
        except BaseException as exc:
            cleanup_errors.append(f"seal cleanup metadata: {exc}"[:1000])
            terminal_body["cleanup_status"] = "cleanup_incomplete"
            terminal_body["cleanup_errors"] = cleanup_errors[:8]
            result = {
                **terminal_body, "result_sha256": _digest_json(terminal_body)
            }
        if cleanup_errors and journal.exists():
            try:
                journal_value = _read_json(journal)
                if not isinstance(journal_value, Mapping):
                    raise CrackHarnessError(
                        "exact cleanup transaction journal is invalid"
                    )
                _write_recovery_required(
                    root, state, journal_value,
                    "exact local finalization is incomplete",
                    record_sha256=(
                        journal_value.get("record_sha256")
                        if journal_value.get("record_succeeded") is True
                        else None
                    ),
                )
            except BaseException as exc:
                cleanup_errors.append(f"recovery marker: {exc}"[:1000])
                terminal_body = dict(result)
                terminal_body.pop("result_sha256", None)
                terminal_body["cleanup_status"] = "cleanup_incomplete"
                terminal_body["cleanup_errors"] = cleanup_errors[:8]
                result = {
                    **terminal_body, "result_sha256": _digest_json(terminal_body)
                }
        return result

    if status == "improved":
        if frontier_value is None:
            raise CrackHarnessError("improved outcome lacks a signed frontier")
        _atomic_json(run_dir / "result.json", result)
        if worktree.exists():
            secondary(
                "improved disposable worktree cleanup",
                lambda: _remove_disposable_worktree(root, worktree),
            )
        if not cleanup_errors:
            secondary("improved raw/temp cleanup", lambda: _cleanup_raw(run_dir))
        for disposable in (
            paths["base"], paths["candidate"], permit_file,
            approval["_approval_path"],
        ):
            if cleanup_errors:
                break
            secondary(
                f"improved delete {disposable.name}",
                lambda item=disposable: _safe_unlink(item),
            )
        if not cleanup_errors:
            secondary(
                "improved delete attempt receipt",
                lambda: _safe_unlink(state / "attempt.json"),
            )
        secondary(
            "improved owner retention maintenance",
            lambda: _gc_owner(
                run_dir, MAX_RETAINED_OWNER_BYTES,
                protected={run_dir.parent},
            ),
        )
        secondary(
            "improved global retention maintenance",
            lambda: _gc_global(
                state, MAX_RETAINED_GLOBAL_BYTES,
                protected={run_dir.parent},
            ),
        )
        terminal_body = dict(result)
        terminal_body.pop("result_sha256", None)
        terminal_body["cleanup_status"] = (
            "cleanup_incomplete" if cleanup_errors else "complete"
        )
        terminal_body["cleanup_errors"] = cleanup_errors[:8]
        result = {
            **terminal_body, "result_sha256": _digest_json(terminal_body)
        }
        if cleanup_errors:
            # Keep only the compact result beside the already signed frontier;
            # a later maintenance pass retries deletion without changing the
            # retained source or primary improved outcome.
            try:
                _atomic_json(run_dir / "result.json", result)
            except BaseException as exc:
                terminal_body["cleanup_errors"] = (
                    cleanup_errors + [f"seal cleanup metadata: {exc}"[:1000]]
                )[:8]
                result = {
                    **terminal_body,
                    "result_sha256": _digest_json(terminal_body),
                }
        elif run_dir.exists():
            secondary(
                "improved run directory cleanup",
                lambda: (_assert_no_indirection(run_dir), shutil.rmtree(run_dir)),
            )
            if cleanup_errors:
                terminal_body["cleanup_status"] = "cleanup_incomplete"
                terminal_body["cleanup_errors"] = cleanup_errors[:8]
                result = {
                    **terminal_body,
                    "result_sha256": _digest_json(terminal_body),
                }
        return result

    if status == "failed":
        if journal.exists():
            secondary("transaction journal cleanup", journal.unlink)
        secondary(
            "record commit cleanup",
            lambda: (run_dir / "record.commit.json").unlink(missing_ok=True),
        )
        if worktree.exists():
            secondary(
                "failed disposable worktree cleanup",
                lambda: _remove_disposable_worktree(root, worktree),
            )
        secondary("failed raw/temp cleanup", lambda: _cleanup_raw(run_dir))
        for disposable in (
            paths["base"], paths["candidate"], permit_file,
            approval["_approval_path"],
        ):
            if cleanup_errors:
                break
            secondary(
                f"failed delete {disposable.name}",
                lambda item=disposable: item.unlink(missing_ok=True),
            )
        if run_dir.exists():
            secondary(
                "failed run directory cleanup",
                lambda: (_assert_no_indirection(run_dir), shutil.rmtree(run_dir)),
            )
        secondary(
            "failed owner retention maintenance",
            lambda: _gc_owner(run_dir, MAX_RETAINED_OWNER_BYTES),
        )
        secondary(
            "failed global retention maintenance",
            lambda: _gc_global(state, MAX_RETAINED_GLOBAL_BYTES),
        )
        diagnostic_body = {
            "schema": "crack_harness_failure_diagnostic/v2",
            "approval_sha256": approval["_approval_sha256"],
            "owner": approval["owner"], "function": approval["function"],
            "primary_reason": reason[:1000],
            "failure_receipt": receipts.get("failure"),
            "cleanup_errors": cleanup_errors[:8], "finished_at": finished,
        }
        try:
            _atomic_json(run_dir.parent / "latest-failure.json", {
                **diagnostic_body,
                "diagnostic_sha256": _digest_json(diagnostic_body),
            })
        except BaseException as exc:
            # Never replace the primary failure even if durable diagnostics are
            # temporarily unwritable; the returned sealed failure remains primary.
            cleanup_errors.append(f"failure diagnostic seal: {exc}"[:1000])
        if not cleanup_errors:
            secondary(
                "failed delete attempt receipt",
                lambda: (state / "attempt.json").unlink(missing_ok=True),
            )
        if cleanup_errors:
            # Refresh the diagnostic after a late attempt-unlink failure so the
            # durable record and the returned terminal agree.
            diagnostic_body["cleanup_errors"] = cleanup_errors[:8]
            try:
                _atomic_json(run_dir.parent / "latest-failure.json", {
                    **diagnostic_body,
                    "diagnostic_sha256": _digest_json(diagnostic_body),
                })
            except BaseException:
                pass
        terminal_body = dict(result)
        terminal_body.pop("result_sha256", None)
        terminal_body["cleanup_status"] = (
            "cleanup_incomplete" if cleanup_errors else "complete"
        )
        terminal_body["cleanup_errors"] = cleanup_errors[:8]
        result = {
            **terminal_body, "result_sha256": _digest_json(terminal_body)
        }
        return result

    secondary(
        "transaction journal cleanup",
        lambda: journal.unlink(missing_ok=True),
    )
    secondary(
        "record commit cleanup",
        lambda: (run_dir / "record.commit.json").unlink(missing_ok=True),
    )
    if worktree.exists():
        secondary(
            "no-gain disposable worktree cleanup",
            lambda: _remove_disposable_worktree(root, worktree),
        )
    secondary("no-gain raw/temp cleanup", lambda: _cleanup_raw(run_dir))
    for disposable in (
        paths["base"], paths["candidate"], permit_file,
        approval["_approval_path"],
    ):
        if cleanup_errors:
            break
        secondary(
            f"no-gain delete {disposable.name}",
            lambda item=disposable: item.unlink(missing_ok=True),
        )
    secondary(
        "no-gain owner retention maintenance",
        lambda: _gc_owner(run_dir, MAX_RETAINED_OWNER_BYTES),
    )
    secondary(
        "no-gain global retention maintenance",
        lambda: _gc_global(state, MAX_RETAINED_GLOBAL_BYTES),
    )
    if run_dir.exists():
        secondary(
            "no-gain run directory cleanup",
            lambda: (_assert_no_indirection(run_dir), shutil.rmtree(run_dir)),
        )
    if not cleanup_errors:
        secondary(
            "no-gain delete attempt receipt",
            lambda: (state / "attempt.json").unlink(missing_ok=True),
        )
    terminal_body = dict(result)
    terminal_body.pop("result_sha256", None)
    terminal_body["cleanup_status"] = (
        "cleanup_incomplete" if cleanup_errors else "complete"
    )
    terminal_body["cleanup_errors"] = cleanup_errors[:8]
    return {**terminal_body, "result_sha256": _digest_json(terminal_body)}


def _status_core(
    root: Path, state: Path, *, manager_key_path: Path,
    expected_key_id: str,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    state = Path(os.path.abspath(state))
    _assert_no_indirection(root)
    _assert_no_indirection(state, missing_leaf=not state.exists())
    results = []
    if state.exists():
        _assert_no_indirection(state)
        _state_path(state, state / "STOP", "global STOP", exists=None)
        _state_path(
            state, state / "transaction.json", "transaction journal", exists=None
        )
        lock_path = _state_path(
            state, state / ".transaction.lock", "transaction lock", exists=None
        )
        with serialized_build_lock(lock_path, 55.0):
            _recover_interrupted(root, state)
            _recover_pending_frontiers(
                root, state, manager_key_path=manager_key_path,
                expected_key_id=expected_key_id,
            )
            # An unresolved post-record transaction owns its temp/worktree
            # until a complete terminal report is available.  Scavenging it
            # here would destroy the baseline needed for recovery.
            if not (
                (state / "RECOVERY_REQUIRED.json").is_file()
                or (state / "transaction.json").is_file()
            ):
                _scavenge_disposable_worktrees(
                    root, state, manager_key_path=manager_key_path,
                    expected_key_id=expected_key_id,
                )
                _retry_retention_maintenance(
                    state, root, manager_key_path=manager_key_path,
                    expected_key_id=expected_key_id,
                )
        for path in _state_glob(state, "owners/*/*/latest/result.json", "status result"):
            try:
                value = _read_json(path)
            except CrackHarnessError:
                continue
            if isinstance(value, Mapping):
                results.append(dict(value))
    return {
        "schema": "crack_harness_status/v1",
        "state_root": str(state),
        "results": sorted(results, key=lambda item: str(item.get("finished_at", ""))),
        "temporary_bytes": _tree_size(state) if state.exists() else 0,
        "global_stop": (state / "STOP").is_file(),
        "interrupted_transaction": (state / "transaction.json").is_file(),
        "packet_rollback_required": (
            state / "PACKET_ROLLBACK_REQUIRED.json"
        ).is_file(),
        "authority_advanced": False,
    }


def status(root: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _status_core(
        root, _state_root(root), manager_key_path=MANAGER_PERMIT_KEY,
        expected_key_id=MANAGER_KEY_ID,
    )


def _status_for_test(
    root: Path, *, state_root: Path, manager_key_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    key = _manager_key_file(root, manager_key_path)
    return _status_core(
        root, _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN),
        manager_key_path=key, expected_key_id=_digest_file(key),
    )


def add_crack_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser("crack", help="run one approved bounded crack cell")
    commands = parser.add_subparsers(dest="crack_command", required=True)
    for name in ("dry-run", "run"):
        command = commands.add_parser(name)
        command.add_argument("--approval", required=True)
        if name != "dry-run":
            command.add_argument("--permit", required=True)
    issue = commands.add_parser(
        "issue", help="manager-only atomic approval/permit/STOP materialization"
    )
    issue.add_argument("--draft", required=True)
    issue.add_argument("--approval-out", required=True)
    issue.add_argument("--permit-out", required=True)
    status_parser = commands.add_parser("status")
    return parser


def _add_proof_adapter_parser(subparsers: Any) -> None:
    parser = subparsers.add_parser("proof-adapter", help=argparse.SUPPRESS)
    parser.add_argument("--kind", required=True, choices=[*HOOKS, "assess"])
    parser.add_argument("--owner", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--candidate-source", required=True, type=Path)
    parser.add_argument("--candidate-source-sha256", required=True)
    parser.add_argument("--approved-target-object-sha256", required=True)
    parser.add_argument("--target-object", required=True, type=Path)
    parser.add_argument("--candidate-object", required=True, type=Path)
    parser.add_argument("--baseline-strict-report", required=True, type=Path)
    parser.add_argument("--baseline-data-report", required=True, type=Path)
    parser.add_argument("--candidate-strict-report", required=True, type=Path)
    parser.add_argument("--candidate-data-report", required=True, type=Path)
    parser.add_argument("--baseline-physical-receipt", required=True, type=Path)
    parser.add_argument("--physical-receipt", required=True, type=Path)


def run_crack_command(args: argparse.Namespace, *, root: Path) -> int:
    if args.crack_command == "status":
        value = status(root)
    elif args.crack_command == "dry-run":
        value = dry_run(root, Path(args.approval))
    elif args.crack_command == "issue":
        value = issue_manager_packet(
            root, Path(args.draft), Path(args.approval_out), Path(args.permit_out)
        )
    else:
        value = run_approved(
            root, Path(args.approval),
            permit_path=Path(args.permit)
        )
    print(json.dumps(value, indent=2, sort_keys=True))
    if value.get("status") == "no_gain":
        print("PIVOT_REQUIRED", file=sys.stderr)
    return 2 if value.get("status") in {"failed", "blocked", "no_gain"} else 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    subparsers = parser.add_subparsers(dest="command", required=True)
    add_crack_parser(subparsers)
    _add_proof_adapter_parser(subparsers)
    args = parser.parse_args(argv)
    try:
        if args.command == "proof-adapter":
            value = _proof_adapter_payload(
                kind=args.kind, owner=args.owner, function=args.function,
                candidate_source=args.candidate_source,
                candidate_source_sha256=args.candidate_source_sha256,
                approved_target_object_sha256=args.approved_target_object_sha256,
                target_object=args.target_object, candidate_object=args.candidate_object,
                baseline_strict_report=args.baseline_strict_report,
                baseline_data_report=args.baseline_data_report,
                candidate_strict_report=args.candidate_strict_report,
                candidate_data_report=args.candidate_data_report,
                baseline_physical_receipt=args.baseline_physical_receipt,
                physical_receipt=args.physical_receipt,
            )
            print(_canonical(value).decode("utf-8"))
            return 0
        return run_crack_command(args, root=Path(args.root).resolve())
    except CrackHarnessError as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
