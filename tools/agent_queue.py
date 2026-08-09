#!/usr/bin/env python3
"""Shared, conflict-aware recovery queue for concurrent local worktrees.

The queue lives in the Git common directory by default, so every worktree from
one clone sees the same claims. Separate clones can share ``MP6_AGENT_QUEUE``.
"""

from __future__ import annotations

import argparse
import copy
import errno
import hashlib
import json
import os
import shutil
import socket
import subprocess
import tempfile
import time
import uuid
import sys
import warnings
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Iterable, Iterator, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.git_paths import native_git_path
from tools.workspace_policy import DEFAULT_WORKER_BASE

SCHEMA_VERSION = 2
QUEUE_ENV = "MP6_AGENT_QUEUE"
QUEUE_BACKUP_SUFFIX = ".bak"
QUEUE_AUDIT_SUFFIX = ".audit.jsonl"
QUEUE_AUDIT_PENDING_SUFFIX = ".audit.pending"
ACTIVE = {"claimed", "researching", "coding", "verifying", "blocked", "ready"}
OPEN = {"pending", *ACTIVE}
TERMINAL = {"done", "released", "cancelled"}
ALL = OPEN | TERMINAL
PRIORITY = {"critical": 0, "high": 1, "normal": 2, "low": 3}
CHANGE_CLASSES = {
    "documentation",
    "tooling",
    "metadata",
    "private-source",
    "shared-interface",
    "build-configuration",
}
DEFAULT_RESOURCES = {
    "retail-build",
    "integration",
    "symbol-regeneration",
    "configure-shared",
}
STALE_HOURS = 24.0


class QueueError(ValueError):
    pass


def _run(cwd: Path, *command: str) -> str:
    result = subprocess.run(
        command,
        cwd=cwd,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        raise QueueError(
            result.stderr.strip() or "command failed: " + " ".join(command)
        )
    return result.stdout.strip()


def git_root(cwd: str | Path | None = None) -> Path:
    start = Path(cwd or Path.cwd()).resolve()
    relative = _run(start, "git", "rev-parse", "--show-cdup")
    return (start / relative).resolve()


def git_common_dir(root: Path) -> Path:
    return native_git_path(
        _run(
            root,
            "git",
            "rev-parse",
            "--path-format=relative",
            "--git-common-dir",
        ),
        relative_to=root,
    )


def queue_path(root: Path, override: str | Path | None = None) -> Path:
    raw = override or os.environ.get(QUEUE_ENV)
    if raw:
        path = Path(raw).expanduser()
        path = path if path.is_absolute() else root / path
        path = path.resolve()
        return path if path.suffix.lower() == ".json" else path / "queue.json"
    return git_common_dir(root) / "agent-coordination" / "queue.json"


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _empty() -> dict[str, Any]:
    return {
        "schema_version": SCHEMA_VERSION,
        "updated_at": _now(),
        "tasks": [],
        "resources": {},
    }


def _normalise_task(task: Mapping[str, Any]) -> dict[str, Any]:
    # Migration is deliberately the only place where legacy defaults are
    # introduced.  The raw queue is shape-checked before this function runs so
    # malformed records can never disappear through a permissive comprehension.
    value = copy.deepcopy(dict(task))
    value.setdefault("target", None)
    value.setdefault("source", None)
    value.setdefault("priority", "normal")
    value.setdefault("status", "pending")
    value.setdefault("agent", None)
    value.setdefault("worktree", None)
    value.setdefault("branch", None)
    value.setdefault("build_dir", None)
    value.setdefault("shared_files", [])
    value.setdefault("depends_on", [])
    value.setdefault("batch", None)
    value.setdefault("capabilities", [])
    value.setdefault("change_class", "private-source")
    value.setdefault("estimated_cost", 1)
    value.setdefault("verification_cost", 1)
    value.setdefault("base_ref", DEFAULT_WORKER_BASE)
    value.setdefault("created_at", _now())
    value.setdefault("claimed_at", None)
    value.setdefault("updated_at", value.get("created_at") or _now())
    value.setdefault("released_at", None)
    value.setdefault("last_verified_commit", None)
    value.setdefault("verification", None)
    value.setdefault("note", "")
    return value


def _migrate(value: dict[str, Any]) -> dict[str, Any]:
    version = value.get("schema_version", 1)
    result = copy.deepcopy(value)
    if version not in {1, 2}:
        return result
    result["schema_version"] = SCHEMA_VERSION
    result["tasks"] = [_normalise_task(item) for item in result["tasks"]]
    resources = result.get("resources")
    result["resources"] = dict(resources) if resources is not None else {}
    result.setdefault("updated_at", _now())
    return result


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _queue_json_bytes(queue: Mapping[str, Any]) -> bytes:
    try:
        return (
            json.dumps(
                queue,
                ensure_ascii=False,
                allow_nan=False,
                indent=2,
                sort_keys=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, UnicodeError, ValueError) as exc:
        raise QueueError(f"queue cannot be serialized as strict JSON: {exc}") from exc


def _audit_json_bytes(record: Mapping[str, Any]) -> bytes:
    try:
        return (
            json.dumps(
                record,
                ensure_ascii=False,
                allow_nan=False,
                sort_keys=True,
                separators=(",", ":"),
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, UnicodeError, ValueError) as exc:
        raise QueueError(f"audit record cannot be serialized as strict JSON: {exc}") from exc


def _reject_json_constant(value: str) -> None:
    raise ValueError(f"non-finite JSON constant is not allowed: {value}")


def _parse_json_bytes(data: bytes, path: Path) -> Any:
    if not data:
        raise QueueError(f"empty queue file: {path}")
    if all(byte == 0 for byte in data):
        raise QueueError(f"all-NUL queue file: {path}")
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise QueueError(f"invalid UTF-8 queue {path}: {exc}") from exc
    try:
        return json.loads(
            text,
            parse_constant=_reject_json_constant,
        )
    except json.JSONDecodeError as exc:
        raise QueueError(
            f"invalid queue JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    except (TypeError, ValueError) as exc:
        raise QueueError(f"invalid queue JSON {path}: {exc}") from exc


def _raw_task_errors(task: Any, where: str) -> list[str]:
    if not isinstance(task, Mapping):
        return [f"{where} must be an object"]
    errors: list[str] = []
    for key in ("id", "owner"):
        if key in task and (not isinstance(task[key], str) or not task[key]):
            errors.append(f"{where}.{key} must be a non-empty string")
    for key in (
        "target", "source", "agent", "worktree", "branch", "build_dir",
        "created_at", "claimed_at", "updated_at", "released_at", "base_ref", "note",
    ):
        if key in task and task[key] is not None and not isinstance(task[key], str):
            errors.append(f"{where}.{key} must be a string or null")
    if "status" in task and task["status"] not in ALL:
        errors.append(f"{where}.status is invalid")
    if "priority" in task and task["priority"] not in PRIORITY:
        errors.append(f"{where}.priority is invalid")
    if "change_class" in task and task["change_class"] not in CHANGE_CLASSES:
        errors.append(f"{where}.change_class is invalid")
    for key in ("shared_files", "depends_on", "capabilities"):
        if key in task and (
            not isinstance(task[key], list)
            or not all(isinstance(item, str) and item for item in task[key])
        ):
            errors.append(f"{where}.{key} must be a string list")
    for key in ("estimated_cost", "verification_cost"):
        if key in task and (
            isinstance(task[key], bool)
            or not isinstance(task[key], int)
            or task[key] < 1
        ):
            errors.append(f"{where}.{key} must be a positive integer")
    if "verification" in task and task["verification"] is not None and not isinstance(task["verification"], Mapping):
        errors.append(f"{where}.verification must be an object or null")
    return errors


def _raw_resource_errors(name: Any, record: Any, where: str) -> list[str]:
    errors: list[str] = []
    if not isinstance(name, str) or not name:
        errors.append(f"{where} resource name must be a non-empty string")
    if not isinstance(record, Mapping):
        return [*errors, f"{where} must be an object"]
    required = ("name", "agent", "worktree", "branch", "acquired_at", "updated_at", "host")
    for key in required:
        if key not in record:
            errors.append(f"{where}.{key} is required")
        elif not isinstance(record[key], str) or not record[key]:
            errors.append(f"{where}.{key} must be a non-empty string")
    if isinstance(name, str) and record.get("name") is not None and record.get("name") != name:
        errors.append(f"{where}.name must match resource key {name!r}")
    if "owner" in record and record["owner"] is not None and (not isinstance(record["owner"], str) or not record["owner"]):
        errors.append(f"{where}.owner must be a string or null")
    return errors


def _raw_queue_errors(value: Any) -> list[str]:
    if not isinstance(value, Mapping):
        return ["queue root must be an object"]
    errors: list[str] = []
    version = value.get("schema_version", 1)
    if isinstance(version, bool) or not isinstance(version, int) or version not in {1, 2}:
        errors.append("schema_version must be 1 or 2")
    tasks = value.get("tasks")
    if not isinstance(tasks, list):
        errors.append("tasks must be a list")
    else:
        errors.extend(
            error
            for index, task in enumerate(tasks)
            for error in _raw_task_errors(task, f"tasks[{index}]")
        )
    if "resources" in value:
        resources = value["resources"]
        if not isinstance(resources, Mapping):
            errors.append("resources must be an object")
        else:
            errors.extend(
                error
                for name, record in resources.items()
                for error in _raw_resource_errors(name, record, f"resources[{name!r}]")
            )
    elif version == 2:
        errors.append("resources is required for schema_version 2")
    if "updated_at" in value and (not isinstance(value["updated_at"], str) or not value["updated_at"]):
        errors.append("updated_at must be a non-empty string")
    return sorted(set(errors))


def _preserve_evidence(path: Path, data: bytes, kind: str = "corrupt") -> Path | None:
    """Preserve bytes without ever overwriting an existing evidence file."""
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        digest = _sha256(data)
        evidence = path.with_name(f"{path.name}.{kind}.{digest}")
        if evidence.exists() and evidence.read_bytes() == data:
            return evidence
        if evidence.exists():
            return None
        _install_bytes(evidence, data, label="evidence")
        return evidence
    except (OSError, QueueError):
        return None


def _fsync_directory(directory: Path) -> bool:
    """Best-effort directory durability; unsupported directory fsync is normal."""
    unsupported = {errno.EINVAL, errno.ENOTSUP}
    if os.name == "nt":
        unsupported.update({errno.EPERM, errno.EACCES})
    try:
        descriptor = os.open(str(directory), os.O_RDONLY)
    except OSError as exc:
        if exc.errno in unsupported:
            return False
        raise QueueError(f"cannot open queue directory for durability {directory}: {exc}") from exc
    try:
        os.fsync(descriptor)
    except OSError as exc:
        # Windows and several network filesystems reject fsync on directories.
        if exc.errno in unsupported:
            return False
        raise QueueError(f"cannot fsync queue directory {directory}: {exc}") from exc
    finally:
        os.close(descriptor)
    return True


def _parse_queue_bytes(
    data: bytes, path: Path, *, strict: bool = False
) -> dict[str, Any]:
    raw = _parse_json_bytes(data, path)
    raw_errors = _raw_queue_errors(raw)
    if raw_errors:
        raise QueueError("invalid queue schema:\n- " + "\n- ".join(raw_errors))
    migrated = _migrate(dict(raw))
    if strict:
        validation_errors = validate_queue(migrated)
        if validation_errors:
            raise QueueError("invalid queue schema:\n- " + "\n- ".join(validation_errors))
    return migrated


def read_queue(path: Path) -> dict[str, Any]:
    path = Path(path)
    if not path.exists():
        return _empty()
    try:
        data = path.read_bytes()
    except OSError as exc:
        raise QueueError(f"cannot read queue {path}: {exc}") from exc
    try:
        return _parse_queue_bytes(data, path)
    except QueueError as exc:
        evidence = _preserve_evidence(path, data, "corrupt")
        detail = f"{exc}; preserved evidence at {evidence}" if evidence else str(exc)
        raise QueueError(detail) from exc


def _durable_temp(path: Path, data: bytes, *, label: str = "candidate") -> Path:
    temporary: Path | None = None
    keep_temp = False
    try:
        with tempfile.NamedTemporaryFile(
            "wb",
            dir=path.parent,
            prefix=f".{path.name}.{label}-",
            suffix=".json",
            delete=False,
        ) as handle:
            temporary = Path(handle.name)
            handle.write(data)
            handle.flush()
            os.fsync(handle.fileno())
        readback = temporary.read_bytes()
        if _sha256(readback) != _sha256(data):
            keep_temp = True
            raise QueueError(
                f"{label} temp hash mismatch for {path}"
                f"; preserved temp evidence at {temporary}"
            )
        return temporary
    except (OSError, QueueError, UnicodeError) as exc:
        if temporary is not None and not keep_temp:
            try:
                temporary.unlink(missing_ok=True)
            except OSError:
                pass
        raise QueueError(f"cannot durably write {label} for {path}: {exc}") from exc


def _install_bytes(path: Path, data: bytes, *, label: str) -> None:
    temporary = _durable_temp(path, data, label=label)
    try:
        os.replace(temporary, path)
        temporary = None
        _fsync_directory(path.parent)
        if _sha256(path.read_bytes()) != _sha256(data):
            raise QueueError(f"{label} install hash mismatch for {path}")
    except (OSError, QueueError) as exc:
        raise QueueError(f"cannot durably replace {path}: {exc}") from exc
    finally:
        if temporary is not None:
            try:
                temporary.unlink(missing_ok=True)
            except OSError:
                pass


def _backup_path(path: Path) -> Path:
    return path.with_name(path.name + QUEUE_BACKUP_SUFFIX)


def _audit_path(path: Path) -> Path:
    return path.with_name(path.name + QUEUE_AUDIT_SUFFIX)


def _pending_audit_path(path: Path) -> Path:
    return path.with_name(path.name + QUEUE_AUDIT_PENDING_SUFFIX)


def _task_changes(before: Mapping[str, Any] | None, after: Mapping[str, Any]) -> list[dict[str, str | None]]:
    old_tasks = {
        str(task.get("id")): task
        for task in (before or {}).get("tasks", [])
        if isinstance(task, Mapping) and task.get("id")
    }
    new_tasks = {
        str(task.get("id")): task
        for task in after.get("tasks", [])
        if isinstance(task, Mapping) and task.get("id")
    }
    changed: list[dict[str, str | None]] = []
    for task_id in sorted(set(old_tasks) | set(new_tasks)):
        if old_tasks.get(task_id) != new_tasks.get(task_id):
            task = new_tasks.get(task_id) or old_tasks.get(task_id) or {}
            changed.append({"id": task_id, "owner": task.get("owner")})
    return changed


def _append_audit_record(path: Path, record: Mapping[str, Any]) -> None:
    payload = _audit_json_bytes(record)
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("ab") as handle:
            handle.write(payload)
            handle.flush()
            os.fsync(handle.fileno())
        _fsync_directory(path.parent)
    except OSError as exc:
        raise QueueError(f"cannot append queue audit {path}: {exc}") from exc


def _pending_records(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    try:
        data = path.read_bytes()
    except OSError as exc:
        raise QueueError(f"cannot read pending queue audit {path}: {exc}") from exc
    records: list[dict[str, Any]] = []
    for index, line in enumerate(data.splitlines(), start=1):
        if not line.strip():
            continue
        try:
            value = json.loads(
                line.decode("utf-8"),
                parse_constant=_reject_json_constant,
            )
        except (UnicodeDecodeError, json.JSONDecodeError, TypeError, ValueError) as exc:
            raise QueueError(f"invalid pending queue audit {path}:{index}: {exc}") from exc
        if not isinstance(value, dict):
            raise QueueError(f"invalid pending queue audit {path}:{index}: record must be an object")
        records.append(value)
    return records


def _replay_pending_audit(path: Path) -> None:
    pending_path = _pending_audit_path(path)
    pending = _pending_records(pending_path)
    if not pending:
        return
    audit_path = _audit_path(path)
    try:
        live_hash = _sha256(path.read_bytes()) if path.exists() else None
        pending_data = pending_path.read_bytes()
        existing_audit = set(audit_path.read_bytes().splitlines()) if audit_path.exists() else set()
    except OSError as exc:
        raise QueueError(f"cannot inspect pending queue audit {pending_path}: {exc}") from exc
    try:
        for record in pending:
            old_hash = record.get("old_sha256")
            new_hash = record.get("new_sha256")
            if live_hash == old_hash:
                _preserve_evidence(pending_path, pending_data, "aborted")
                continue
            if live_hash != new_hash:
                raise QueueError(
                    f"pending queue audit conflicts with live queue {path}; "
                    f"pending new hash {new_hash}, live hash {live_hash}"
                )
            payload = _audit_json_bytes(record).rstrip(b"\n")
            if payload not in existing_audit:
                _append_audit_record(audit_path, record)
                existing_audit.add(payload)
        pending_path.unlink()
        _fsync_directory(path.parent)
    except (OSError, QueueError) as exc:
        raise QueueError(f"pending queue audit remains at {pending_path}: {exc}") from exc


def _make_audit_record(
    *,
    before: Mapping[str, Any] | None,
    after: Mapping[str, Any],
    old_sha256: str | None,
    new_sha256: str,
    command: str | None,
) -> dict[str, Any]:
    changed = _task_changes(before, after)
    record: dict[str, Any] = {
        "timestamp": _now(),
        "pid": os.getpid(),
        "host": socket.gethostname(),
        "command": command or " ".join(sys.argv),
        "changed_tasks": changed,
        "old_sha256": old_sha256,
        "new_sha256": new_sha256,
        "event_id": uuid.uuid4().hex,
    }
    return record


def _stage_audit(
    path: Path,
    *,
    before: Mapping[str, Any] | None,
    after: Mapping[str, Any],
    old_sha256: str | None,
    new_sha256: str,
    command: str | None = None,
) -> dict[str, Any]:
    pending_path = _pending_audit_path(path)
    _replay_pending_audit(path)
    record = _make_audit_record(
        before=before,
        after=after,
        old_sha256=old_sha256,
        new_sha256=new_sha256,
        command=command,
    )
    try:
        with pending_path.open("ab") as handle:
            handle.write(_audit_json_bytes(record))
            handle.flush()
            os.fsync(handle.fileno())
        _fsync_directory(path.parent)
    except (OSError, QueueError) as exc:
        raise QueueError(f"cannot stage queue audit {pending_path}: {exc}") from exc
    return record


def _finalize_audit(path: Path, record: Mapping[str, Any]) -> None:
    pending_path = _pending_audit_path(path)
    try:
        _append_audit_record(_audit_path(path), record)
        pending_path.unlink()
        _fsync_directory(path.parent)
    except (OSError, QueueError) as exc:
        warnings.warn(
            f"queue committed and validated; audit pending at {pending_path}: {exc}",
            RuntimeWarning,
            stacklevel=2,
        )


def _discard_staged_audit(path: Path) -> None:
    pending_path = _pending_audit_path(path)
    try:
        pending_path.unlink(missing_ok=True)
        _fsync_directory(path.parent)
    except (OSError, QueueError):
        # Preserve pending evidence if cleanup cannot be made durable.
        return


def _rollback_queue(path: Path, old_data: bytes | None) -> None:
    if old_data is None:
        try:
            path.unlink(missing_ok=True)
            _fsync_directory(path.parent)
        except OSError as exc:
            raise QueueError(f"rollback could not remove {path}: {exc}") from exc
        return
    try:
        _install_bytes(path, old_data, label="rollback")
        restored = path.read_bytes()
        if _sha256(restored) != _sha256(old_data):
            raise QueueError(f"rollback hash mismatch for {path}")
        _parse_queue_bytes(restored, path, strict=True)
    except (OSError, QueueError) as exc:
        raise QueueError(f"rollback failed for {path}: {exc}") from exc


def _write(
    path: Path,
    queue: dict[str, Any],
    *,
    before: Mapping[str, Any] | None = None,
    command: str | None = None,
) -> None:
    """Persist a validated queue with atomic replacement and an audit record."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    old_data: bytes | None = None
    old_queue: Mapping[str, Any] | None = before
    if path.exists():
        try:
            old_data = path.read_bytes()
            if old_queue is None:
                old_queue = _parse_queue_bytes(old_data, path, strict=True)
        except (OSError, QueueError) as exc:
            evidence = (
                _preserve_evidence(path, old_data, "corrupt")
                if old_data is not None
                else None
            )
            detail = f"cannot read existing queue before write {path}: {exc}"
            if evidence:
                detail += f"; preserved evidence at {evidence}"
            raise QueueError(detail) from exc

    queue["schema_version"] = SCHEMA_VERSION
    queue["updated_at"] = _now()
    validation_errors = validate_queue(queue)
    if validation_errors:
        raise QueueError("queue invalid before write:\n- " + "\n- ".join(validation_errors))
    candidate_data = _queue_json_bytes(queue)
    candidate_hash = _sha256(candidate_data)
    temporary: Path | None = None
    staged_audit: dict[str, Any] | None = None
    try:
        temporary = _durable_temp(path, candidate_data, label="candidate")
        readback = temporary.read_bytes()
        if _sha256(readback) != candidate_hash:
            evidence = _preserve_evidence(temporary, readback, "candidate")
            raise QueueError(
                f"candidate queue hash mismatch for {temporary}"
                + (f"; preserved evidence at {evidence}" if evidence else "")
            )
        _parse_queue_bytes(readback, temporary, strict=True)
        staged_audit = _stage_audit(
            path,
            before=old_queue,
            after=queue,
            old_sha256=_sha256(old_data) if old_data is not None else None,
            new_sha256=candidate_hash,
            command=command,
        )
        if old_data is not None:
            try:
                _install_bytes(_backup_path(path), old_data, label="backup")
            except QueueError:
                _discard_staged_audit(path)
                raise
        try:
            os.replace(temporary, path)
        except OSError as exc:
            _discard_staged_audit(path)
            evidence = _preserve_evidence(temporary, readback, "candidate")
            raise QueueError(
                f"atomic queue replace failed for {path}: {exc}"
                + (f"; preserved evidence at {evidence}" if evidence else "")
            ) from exc
        temporary = None
        try:
            _fsync_directory(path.parent)
            installed = path.read_bytes()
            if _sha256(installed) != candidate_hash:
                raise QueueError(f"post-replace queue hash mismatch for {path}")
            post_queue = read_queue(path)
            if validate_queue(post_queue):
                raise QueueError("post-replace queue validation failed")
        except (OSError, QueueError) as exc:
            corrupt: bytes | None = None
            try:
                corrupt = path.read_bytes()
            except OSError:
                pass
            evidence = (
                _preserve_evidence(path, corrupt, "corrupt")
                if corrupt is not None
                else None
            )
            try:
                _rollback_queue(path, old_data)
            except QueueError as rollback_exc:
                _discard_staged_audit(path)
                raise QueueError(
                    f"post-replace queue validation failed for {path}: {exc}; "
                    f"rollback failed: {rollback_exc}"
                ) from exc
            _discard_staged_audit(path)
            raise QueueError(
                f"post-replace queue validation failed for {path}: {exc}"
                + (f"; preserved evidence at {evidence}" if evidence else "")
            ) from exc
        if staged_audit is None:
            raise QueueError("queue audit was not staged before replacement")
        _finalize_audit(path, staged_audit)
    finally:
        if temporary is not None:
            try:
                temporary.unlink(missing_ok=True)
            except OSError:
                pass


@contextmanager
def locked_queue(path: Path, timeout: float = 8.0) -> Iterator[dict[str, Any]]:
    lock = Path(str(path) + ".lock")
    lock.parent.mkdir(parents=True, exist_ok=True)
    deadline = time.monotonic() + timeout
    while True:
        try:
            lock.mkdir()
            (lock / "owner.json").write_text(
                json.dumps(
                    {
                        "pid": os.getpid(),
                        "host": socket.gethostname(),
                        "time": _now(),
                    }
                )
                + "\n",
                encoding="utf-8",
            )
            break
        except FileExistsError:
            try:
                if time.time() - lock.stat().st_mtime > 120:
                    shutil.rmtree(lock, ignore_errors=True)
                    continue
            except FileNotFoundError:
                continue
            if time.monotonic() >= deadline:
                raise QueueError(f"timed out waiting for queue lock {lock}")
            time.sleep(0.05)
    try:
        queue = copy.deepcopy(read_queue(path))
        original = copy.deepcopy(queue)
        yield queue
        if queue != original:
            _write(path, queue, before=original)
    finally:
        shutil.rmtree(lock, ignore_errors=True)


def _repo_path(value: str | None) -> str | None:
    if value is None or not value.strip():
        return None
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise QueueError(f"path must be repository-relative: {value}")
    return None if path.as_posix() == "." else path.as_posix()


def _paths(values: Sequence[str] | None) -> list[str]:
    return sorted({path for value in values or [] if (path := _repo_path(value))})


def _abs(value: str | Path | None, base: Path) -> Path:
    path = Path(value or base).expanduser()
    return (path if path.is_absolute() else base / path).resolve()


def _canon(value: str | None) -> str:
    return os.path.normcase(os.path.realpath(os.path.expanduser(value or "")))


def _overlap(left: str, right: str) -> bool:
    a = PurePosixPath(left).parts
    b = PurePosixPath(right).parts
    return a == b or a == b[: len(a)] or b == a[: len(b)]


def _write_paths(task: Mapping[str, Any]) -> set[str]:
    result = set(_paths(task.get("shared_files", [])))
    if source := _repo_path(task.get("source")):
        result.add(source)
    return result


def _catalog_map(
    owners: Sequence[Mapping[str, Any]] | None,
) -> dict[str, Mapping[str, Any]]:
    return {
        str(item.get("id")): item
        for item in owners or []
        if item.get("id")
    }


def _source(
    owner: str,
    source: str | None,
    known: Sequence[Mapping[str, Any]] | None,
) -> str | None:
    if source:
        return _repo_path(source)
    for item in known or []:
        if item.get("id") == owner and isinstance(item.get("source"), str):
            return _repo_path(str(item["source"]))
    if owner.replace("\\", "/").startswith(("src/", "include/", "config/")):
        return _repo_path(owner)
    return None


def active_tasks(queue: Mapping[str, Any]) -> list[dict[str, Any]]:
    return [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict) and task.get("status") in ACTIVE
    ]


def _open_tasks(queue: Mapping[str, Any]) -> list[dict[str, Any]]:
    return [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict) and task.get("status") in OPEN
    ]


def _find(queue: Mapping[str, Any], owner: str) -> dict[str, Any] | None:
    matches = [task for task in _open_tasks(queue) if task.get("owner") == owner]
    if len(matches) > 1:
        raise QueueError(f"duplicate open tasks for {owner}")
    return matches[0] if matches else None


def _within(child: Path, parent: Path) -> bool:
    try:
        return os.path.commonpath([str(child), str(parent)]) == str(parent)
    except ValueError:
        return False


def _validate_worktree(
    repository_root: Path,
    worktree: Path,
    branch: str,
    build_dir: Path,
) -> list[str]:
    errors: list[str] = []
    if not worktree.is_dir():
        return [f"worktree does not exist: {worktree}"]
    try:
        actual_root = git_root(worktree)
        if actual_root != worktree.resolve():
            errors.append(f"worktree root is {actual_root}, not {worktree}")
        if git_common_dir(repository_root) != git_common_dir(worktree):
            errors.append("worktree belongs to a different Git common directory")
        actual_branch = _run(worktree, "git", "branch", "--show-current")
        if actual_branch != branch:
            errors.append(
                f"worktree branch is {actual_branch or 'detached'}, not {branch}"
            )
        listing = _run(
            repository_root, "git", "worktree", "list", "--porcelain"
        )
        registered = {
            _canon(
                str(
                    native_git_path(
                        line.removeprefix("worktree "),
                        relative_to=repository_root,
                    )
                )
            )
            for line in listing.splitlines()
            if line.startswith("worktree ")
        }
        if _canon(str(worktree)) not in registered:
            errors.append("path is not registered by git worktree list")
    except QueueError as exc:
        errors.append(str(exc))
    if not _within(build_dir, worktree.resolve()):
        errors.append("build directory must be inside the claimed worktree")
    return errors


def _dependency_conflicts(
    left: Mapping[str, Any],
    right: Mapping[str, Any],
    owners: Sequence[Mapping[str, Any]] | None,
) -> list[str]:
    by_id = _catalog_map(owners)
    left_includes = set(
        _paths(by_id.get(str(left.get("owner")), {}).get("includes", []))
    )
    right_includes = set(
        _paths(by_id.get(str(right.get("owner")), {}).get("includes", []))
    )
    conflicts: list[str] = []
    for path in _paths(left.get("shared_files", [])):
        if any(_overlap(path, include) for include in right_includes):
            conflicts.append(
                f"{right.get('owner')} consumes {path}, edited by {left.get('owner')}"
            )
    for path in _paths(right.get("shared_files", [])):
        if any(_overlap(path, include) for include in left_includes):
            conflicts.append(
                f"{left.get('owner')} consumes {path}, edited by {right.get('owner')}"
            )
    return conflicts


def _conflicts(
    queue: Mapping[str, Any],
    candidate: Mapping[str, Any],
    owners: Sequence[Mapping[str, Any]] | None,
) -> list[str]:
    conflicts: list[str] = []
    for task in _open_tasks(queue):
        if task.get("id") == candidate.get("id"):
            continue
        label = f"{task.get('owner')} ({task.get('agent')})"
        if task.get("owner") == candidate.get("owner"):
            conflicts.append(f"owner already claimed by {label}")
        if candidate.get("branch") and task.get("branch") == candidate.get("branch"):
            conflicts.append(f"branch already used by {label}")
        for key in ("worktree", "build_dir"):
            if (
                candidate.get(key)
                and task.get(key)
                and _canon(task.get(key)) == _canon(candidate.get(key))
            ):
                conflicts.append(f"{key} already used by {label}")
        for old in _write_paths(task):
            for new in _write_paths(candidate):
                if _overlap(old, new):
                    conflicts.append(
                        f"write path {new} overlaps {old} claimed by {label}"
                    )
        conflicts.extend(_dependency_conflicts(task, candidate, owners))
    return sorted(set(conflicts))


def _verification_errors(task: Mapping[str, Any], *, terminal: bool) -> list[str]:
    proof = task.get("verification")
    if not isinstance(proof, Mapping):
        return ["structured verification proof is missing"]
    errors: list[str] = []
    if proof.get("public_gate") != "pass":
        errors.append("public_gate must be pass")
    if not proof.get("verified_commit"):
        errors.append("verified_commit is missing")
    change_class = str(task.get("change_class", "private-source"))
    source_like = change_class in {
        "private-source",
        "shared-interface",
        "build-configuration",
    }
    if source_like:
        for key in ("object_report", "functions_exact", "relocations"):
            if not proof.get(key):
                errors.append(f"{key} is required for {change_class}")
    if change_class in {"shared-interface", "build-configuration"} and not proof.get(
        "consumers"
    ):
        errors.append(f"consumer results are required for {change_class}")
    if terminal and source_like:
        if proof.get("retail_gate") != "pass":
            errors.append("retail_gate must be pass before done")
        if proof.get("checksum") != "pass":
            errors.append("checksum must be pass before done")
    return errors


def validate_queue(queue: Mapping[str, Any]) -> list[str]:
    errors: list[str] = []
    if not isinstance(queue, Mapping):
        return ["queue root must be an object"]
    if (
        isinstance(queue.get("schema_version"), bool)
        or not isinstance(queue.get("schema_version"), int)
        or queue.get("schema_version") != SCHEMA_VERSION
    ):
        errors.append(f"schema_version must be {SCHEMA_VERSION}")
    if not isinstance(queue.get("updated_at"), str) or not queue.get("updated_at"):
        errors.append("updated_at must be a non-empty string")
    resources = queue.get("resources")
    if not isinstance(resources, Mapping):
        errors.append("resources must be an object")
    else:
        for name, record in resources.items():
            errors.extend(_raw_resource_errors(name, record, f"resources[{name!r}]"))
    tasks = queue.get("tasks")
    if not isinstance(tasks, list):
        return [*errors, "tasks must be a list"]

    ids: set[str] = set()
    owners: set[str] = set()
    task_owners = {
        str(task.get("owner"))
        for task in tasks
        if isinstance(task, Mapping) and task.get("owner")
    }
    for index, task in enumerate(tasks):
        where = f"tasks[{index}]"
        if not isinstance(task, Mapping):
            errors.append(f"{where} must be an object")
            continue
        task_id = task.get("id")
        owner = task.get("owner")
        status = task.get("status")
        if not isinstance(task_id, str) or not task_id:
            errors.append(f"{where}.id is required")
        elif task_id in ids:
            errors.append(f"duplicate task id: {task_id}")
        ids.add(str(task_id))
        if not isinstance(owner, str) or not owner:
            errors.append(f"{where}.owner is required")
        if status not in ALL:
            errors.append(f"{where}.status is invalid")
        if status in OPEN and isinstance(owner, str):
            if owner in owners:
                errors.append(f"duplicate open owner: {owner}")
            owners.add(owner)
        if task.get("priority") not in PRIORITY:
            errors.append(f"{where}.priority is invalid")
        if task.get("change_class") not in CHANGE_CLASSES:
            errors.append(f"{where}.change_class is invalid")
        for key in (
            "target",
            "source",
            "agent",
            "worktree",
            "branch",
            "build_dir",
            "created_at",
            "claimed_at",
            "updated_at",
            "released_at",
            "base_ref",
            "note",
        ):
            value = task.get(key)
            if value is not None and not isinstance(value, str):
                errors.append(f"{where}.{key} must be a string or null")
        for key in ("shared_files", "depends_on", "capabilities"):
            values = task.get(key, [])
            if not isinstance(values, list) or not all(
                isinstance(item, str) and item for item in values
            ):
                errors.append(f"{where}.{key} must be a string list")
            elif key == "shared_files":
                for item in values:
                    try:
                        _repo_path(item)
                    except QueueError as exc:
                        errors.append(f"{where}.shared_files: {exc}")
        if isinstance(task.get("source"), str):
            try:
                _repo_path(task["source"])
            except QueueError as exc:
                errors.append(f"{where}.source: {exc}")
        for key in ("estimated_cost", "verification_cost"):
            value = task.get(key)
            if isinstance(value, bool) or not isinstance(value, int) or value < 1:
                errors.append(f"{where}.{key} must be a positive integer")
        if task.get("verification") is not None and not isinstance(
            task.get("verification"), Mapping
        ):
            errors.append(f"{where}.verification must be an object or null")
        for dependency in task.get("depends_on", []):
            if dependency == owner:
                errors.append(f"{where} depends on itself")
            elif dependency not in task_owners:
                errors.append(f"{where} depends on unknown owner {dependency}")
        if status in ACTIVE:
            for key in ("agent", "worktree", "branch", "build_dir", "claimed_at"):
                if not isinstance(task.get(key), str) or not task.get(key):
                    errors.append(f"{where}.{key} is required")

    active = active_tasks(queue)
    for index, left in enumerate(active):
        for right in active[index + 1 :]:
            if left.get("branch") == right.get("branch"):
                errors.append(
                    f"{left.get('owner')} and {right.get('owner')} share branch"
                )
            for key in ("worktree", "build_dir"):
                if _canon(left.get(key)) == _canon(right.get(key)):
                    errors.append(
                        f"{left.get('owner')} and {right.get('owner')} share {key}"
                    )
            for first in _write_paths(left):
                for second in _write_paths(right):
                    if _overlap(first, second):
                        errors.append(
                            f"{left.get('owner')} and {right.get('owner')} "
                            f"overlap {first} / {second}"
                        )

    opened = _open_tasks(queue)
    for index, left in enumerate(opened):
        for right in opened[index + 1 :]:
            for first in _write_paths(left):
                for second in _write_paths(right):
                    if _overlap(first, second):
                        errors.append(
                            f"open tasks {left.get('owner')} and "
                            f"{right.get('owner')} overlap {first} / {second}"
                        )
    return sorted(set(errors))


def add_task(
    root: Path,
    owner: str,
    *,
    target: str | None = None,
    source: str | None = None,
    priority: str = "normal",
    shared_files: Sequence[str] | None = None,
    note: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[Mapping[str, Any]] | None = None,
    depends_on: Sequence[str] | None = None,
    batch: str | None = None,
    capabilities: Sequence[str] | None = None,
    change_class: str = "private-source",
    estimated_cost: int = 1,
    verification_cost: int = 1,
    base_ref: str = DEFAULT_WORKER_BASE,
) -> dict[str, Any]:
    owner = owner.strip()
    if not owner or priority not in PRIORITY:
        raise QueueError("valid owner and priority are required")
    if change_class not in CHANGE_CLASSES:
        raise QueueError("invalid change class")
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        errors = validate_queue(queue)
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        if _find(queue, owner):
            raise QueueError(f"an open task already exists for {owner}")
        now = _now()
        task = _normalise_task(
            {
                "id": uuid.uuid4().hex,
                "owner": owner,
                "target": target or None,
                "source": _source(owner, source, owners),
                "priority": priority,
                "status": "pending",
                "shared_files": _paths(shared_files),
                "depends_on": sorted(set(depends_on or [])),
                "batch": batch or None,
                "capabilities": sorted(set(capabilities or [])),
                "change_class": change_class,
                "estimated_cost": max(1, int(estimated_cost)),
                "verification_cost": max(1, int(verification_cost)),
                "base_ref": base_ref,
                "created_at": now,
                "updated_at": now,
                "note": note or "",
            }
        )
        if conflicts := _conflicts(queue, task, owners):
            raise QueueError("task conflicts:\n- " + "\n- ".join(conflicts))
        queue.setdefault("tasks", []).append(task)
        return dict(task)


def claim_task(
    root: Path,
    owner: str,
    *,
    agent: str,
    worktree: str | Path | None = None,
    branch: str | None = None,
    build_dir: str | Path | None = None,
    target: str | None = None,
    source: str | None = None,
    priority: str | None = None,
    shared_files: Sequence[str] | None = None,
    note: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[Mapping[str, Any]] | None = None,
    capabilities: Sequence[str] | None = None,
    change_class: str | None = None,
    base_ref: str | None = None,
) -> dict[str, Any]:
    owner = owner.strip()
    agent = agent.strip()
    if not owner or not agent or (priority is not None and priority not in PRIORITY):
        raise QueueError("valid owner, agent, and priority are required")

    current_branch = _run(root, "git", "branch", "--show-current")
    assigned_worktree = _abs(worktree, root)
    if worktree is not None and branch is None and assigned_worktree != root.resolve():
        raise QueueError("--branch is required when claiming a different worktree")
    assigned_branch = (branch if branch is not None else current_branch).strip()
    if not assigned_branch or assigned_branch in {"main", "master"}:
        raise QueueError("claim requires a non-main task branch")
    assigned_build = (
        (assigned_worktree / "build").resolve()
        if build_dir is None
        else _abs(build_dir, assigned_worktree)
    )
    worktree_errors = _validate_worktree(
        root, assigned_worktree, assigned_branch, assigned_build
    )
    if worktree_errors:
        raise QueueError("invalid worktree claim:\n- " + "\n- ".join(worktree_errors))

    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        errors = validate_queue(queue)
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        task = _find(queue, owner)
        if task and task.get("status") != "pending":
            raise QueueError(
                f"{owner} is already {task.get('status')} by {task.get('agent')}"
            )
        now = _now()
        if task is None:
            task = _normalise_task(
                {
                    "id": uuid.uuid4().hex,
                    "owner": owner,
                    "target": target or None,
                    "source": _source(owner, source, owners),
                    "priority": priority or "normal",
                    "status": "pending",
                    "created_at": now,
                    "updated_at": now,
                }
            )
            queue.setdefault("tasks", []).append(task)
        if target:
            task["target"] = target
        if source:
            task["source"] = _repo_path(source)
        if priority is not None:
            task["priority"] = priority
        if change_class is not None:
            if change_class not in CHANGE_CLASSES:
                raise QueueError("invalid change class")
            task["change_class"] = change_class
        if base_ref:
            task["base_ref"] = base_ref
        if capabilities:
            task["capabilities"] = sorted(
                set(task.get("capabilities", [])) | set(capabilities)
            )
        task["shared_files"] = _paths(
            [*task.get("shared_files", []), *(shared_files or [])]
        )
        if note:
            task["note"] = note
        task.update(
            {
                "status": "claimed",
                "agent": agent,
                "worktree": str(assigned_worktree),
                "branch": assigned_branch,
                "build_dir": str(assigned_build),
                "claimed_at": now,
                "updated_at": now,
                "host": socket.gethostname(),
                "verification": None,
                "last_verified_commit": None,
            }
        )
        if conflicts := _conflicts(queue, task, owners):
            raise QueueError("claim conflicts:\n- " + "\n- ".join(conflicts))
        return dict(task)


def _dependency_done(queue: Mapping[str, Any], owner: str) -> bool:
    matching = [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, Mapping) and task.get("owner") == owner
    ]
    return bool(matching) and matching[-1].get("status") == "done"


def claim_next(
    root: Path,
    *,
    agent: str,
    capabilities: Sequence[str] | None = None,
    batch: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    path = queue_path(root, queue_file)
    requested = set(capabilities or [])
    with locked_queue(path) as queue:
        errors = validate_queue(queue)
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        candidates = []
        for task in queue.get("tasks", []):
            if not isinstance(task, dict) or task.get("status") != "pending":
                continue
            if batch and task.get("batch") != batch:
                continue
            required = set(task.get("capabilities", []))
            if not required.issubset(requested):
                continue
            if not all(_dependency_done(queue, dep) for dep in task.get("depends_on", [])):
                continue
            candidates.append(task)
        candidates.sort(
            key=lambda task: (
                PRIORITY.get(task.get("priority"), 99),
                int(task.get("estimated_cost", 1))
                + int(task.get("verification_cost", 1)),
                task.get("created_at", ""),
            )
        )
        owners_to_try = [str(task["owner"]) for task in candidates]
    for owner in owners_to_try:
        try:
            return claim_task(
                root,
                owner,
                agent=agent,
                queue_file=queue_file,
                owners=owners,
                capabilities=capabilities,
            )
        except QueueError as exc:
            if "conflict" not in str(exc):
                raise
    raise QueueError("no dependency-ready, conflict-free task is available")


def _commit(root: Path, value: str | None) -> str | None:
    return _run(root, "git", "rev-parse", value) if value else None


def _current_claim(
    queue: Mapping[str, Any], root: Path, agent: str | None = None
) -> dict[str, Any] | None:
    branch = _run(root, "git", "branch", "--show-current")
    current = _canon(str(root.resolve()))
    matches = [
        task
        for task in active_tasks(queue)
        if (
            _canon(task.get("worktree")) == current
            or task.get("branch") == branch
        )
        and (agent is None or task.get("agent") == agent)
    ]
    if len(matches) > 1:
        raise QueueError("current worktree or branch matches multiple claims")
    return matches[0] if matches else None


def changed_paths(root: Path, base: str, *, include_worktree: bool = True) -> set[str]:
    commands = [("git", "diff", "--name-only", f"{base}...HEAD")]
    if include_worktree:
        commands += [
            ("git", "diff", "--cached", "--name-only"),
            ("git", "diff", "--name-only"),
            ("git", "ls-files", "--others", "--exclude-standard"),
        ]
    result: set[str] = set()
    for command in commands:
        output = _run(root, *command)
        result.update(
            path
            for line in output.splitlines()
            if (path := _repo_path(line.strip()))
        )
    return result


def check_diff_claim(
    root: Path,
    *,
    base: str | None = None,
    agent: str | None = None,
    queue_file: str | Path | None = None,
    require_claim: bool = True,
) -> dict[str, Any]:
    path = queue_path(root, queue_file)
    queue = read_queue(path)
    errors = validate_queue(queue)
    if errors:
        raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
    task = _current_claim(queue, root, agent)
    if task is None:
        if require_claim:
            raise QueueError("current worktree has no active claim")
        return {"task": None, "changed": [], "errors": []}
    effective_base = base or str(task.get("base_ref") or DEFAULT_WORKER_BASE)
    changed = sorted(changed_paths(root, effective_base))
    allowed = _write_paths(task)
    diff_errors: list[str] = []
    for changed_path in changed:
        if not any(_overlap(changed_path, allowed_path) for allowed_path in allowed):
            diff_errors.append(
                f"undeclared changed path {changed_path}; claim it with --shared"
            )
        for other in active_tasks(queue):
            if other.get("id") == task.get("id"):
                continue
            for protected in _write_paths(other):
                if _overlap(changed_path, protected):
                    diff_errors.append(
                        f"changed path {changed_path} overlaps {protected} owned by "
                        f"{other.get('owner')} ({other.get('agent')})"
                    )
    return {
        "task": dict(task),
        "base": effective_base,
        "changed": changed,
        "allowed": sorted(allowed),
        "errors": sorted(set(diff_errors)),
    }


def record_verification(
    root: Path,
    owner: str,
    *,
    agent: str | None = None,
    public_gate: str = "not-run",
    object_report: str | None = None,
    functions_exact: str | None = None,
    relocations: str | None = None,
    consumers: Mapping[str, str] | None = None,
    retail_gate: str = "not-run",
    checksum: str = "not-run",
    toolchain: str | None = None,
    base: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        task = _find(queue, owner)
        if not task or task.get("status") == "pending":
            raise QueueError(f"{owner} is not actively claimed")
        if agent and task.get("agent") != agent:
            raise QueueError(
                f"{owner} is assigned to {task.get('agent')}, not {agent}"
            )
        current = _current_claim(queue, root, agent)
        if not current or current.get("id") != task.get("id"):
            raise QueueError(
                "verification must run from the claimed worktree and branch"
            )
        worktree_errors = _validate_worktree(
            root,
            Path(str(task["worktree"])),
            str(task["branch"]),
            Path(str(task["build_dir"])),
        )
        if worktree_errors:
            raise QueueError(
                "worktree validation failed:\n- " + "\n- ".join(worktree_errors)
            )
        if _run(root, "git", "status", "--porcelain"):
            raise QueueError("verification requires a clean working tree")
        diff = check_diff_claim(
            root,
            base=base,
            agent=agent,
            queue_file=queue_file,
            require_claim=True,
        )
        if diff["errors"]:
            raise QueueError(
                "claim diff invalid:\n- " + "\n- ".join(diff["errors"])
            )
        head = _commit(root, "HEAD")
        proof = {
            "verified_commit": head,
            "verified_at": _now(),
            "clean": True,
            "claim_diff": "pass",
            "base": diff["base"],
            "public_gate": public_gate,
            "object_report": _repo_path(object_report) if object_report else None,
            "functions_exact": functions_exact,
            "relocations": relocations,
            "consumers": dict(consumers or {}),
            "retail_gate": retail_gate,
            "checksum": checksum,
            "toolchain": toolchain,
        }
        task["verification"] = proof
        task["last_verified_commit"] = head
        task["updated_at"] = _now()
        return dict(task)


def update_task(
    root: Path,
    owner: str,
    *,
    status: str | None = None,
    add_shared: Sequence[str] | None = None,
    remove_shared: Sequence[str] | None = None,
    note: str | None = None,
    agent: str | None = None,
    base_ref: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    if status and status not in ACTIVE:
        raise QueueError("invalid active status")
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        task = _find(queue, owner)
        if not task or task.get("status") == "pending":
            raise QueueError(f"{owner} is not actively claimed")
        if agent and task.get("agent") != agent:
            raise QueueError(
                f"{owner} is assigned to {task.get('agent')}, not {agent}"
            )
        if base_ref is not None:
            if (
                status == "ready"
                or task.get("status") == "ready"
                or task.get("verification")
            ):
                raise QueueError(
                    "base_ref is frozen once verification is recorded"
                )
            branch = task.get("branch")
            if not branch:
                raise QueueError(
                    "task has no claimed branch to validate base_ref against"
                )
            pinned = _commit(root, base_ref)
            try:
                tip = _commit(root, branch)
            except QueueError as error:
                raise QueueError(
                    f"cannot resolve claimed branch {branch}: {error}"
                ) from error
            try:
                _run(root, "git", "merge-base", "--is-ancestor", pinned, tip)
            except QueueError as error:
                raise QueueError(
                    f"{base_ref} is not an ancestor of claimed branch {branch}"
                ) from error
            task["base_ref"] = pinned
        if status == "ready":
            proof_errors = _verification_errors(task, terminal=False)
            if proof_errors:
                raise QueueError("task is not ready:\n- " + "\n- ".join(proof_errors))
        if status:
            task["status"] = status
        shared = set(_paths(task.get("shared_files", []))) | set(
            _paths(add_shared)
        )
        shared -= set(_paths(remove_shared))
        task["shared_files"] = sorted(shared)
        if note is not None:
            task["note"] = note
        task["updated_at"] = _now()
        if conflicts := _conflicts(queue, task, owners):
            raise QueueError("update conflicts:\n- " + "\n- ".join(conflicts))
        return dict(task)


def release_task(
    root: Path,
    owner: str,
    *,
    status: str = "done",
    note: str | None = None,
    agent: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    if status not in TERMINAL:
        raise QueueError("invalid terminal status")
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        task = _find(queue, owner)
        if not task:
            raise QueueError(f"no open task exists for {owner}")
        if agent and task.get("agent") not in {None, agent}:
            raise QueueError(
                f"{owner} is assigned to {task.get('agent')}, not {agent}"
            )
        if status == "done":
            current = _current_claim(queue, root, agent)
            if not current or current.get("id") != task.get("id"):
                raise QueueError("done must be recorded from the claimed worktree")
            head = _commit(root, "HEAD")
            proof = task.get("verification") or {}
            if proof.get("verified_commit") != head:
                raise QueueError("HEAD differs from the last verified commit")
            if _run(root, "git", "status", "--porcelain"):
                raise QueueError("done requires a clean working tree")
            proof_errors = _verification_errors(task, terminal=True)
            if proof_errors:
                raise QueueError(
                    "task cannot be completed:\n- " + "\n- ".join(proof_errors)
                )
        if note is not None:
            task["note"] = note
        task["status"] = status
        task["updated_at"] = task["released_at"] = _now()
        return dict(task)


def acquire_resource(
    root: Path,
    name: str,
    *,
    agent: str,
    owner: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        resources = queue.setdefault("resources", {})
        existing = resources.get(name)
        if isinstance(existing, Mapping):
            raise QueueError(
                f"resource {name} is held by {existing.get('agent')} "
                f"for {existing.get('owner') or 'unspecified owner'}"
            )
        record = {
            "name": name,
            "agent": agent,
            "owner": owner,
            "worktree": str(root.resolve()),
            "branch": _run(root, "git", "branch", "--show-current"),
            "acquired_at": _now(),
            "updated_at": _now(),
            "host": socket.gethostname(),
        }
        resources[name] = record
        return dict(record)


def release_resource(
    root: Path,
    name: str,
    *,
    agent: str,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        resources = queue.setdefault("resources", {})
        existing = resources.get(name)
        if not isinstance(existing, Mapping):
            raise QueueError(f"resource {name} is not held")
        if existing.get("agent") != agent:
            raise QueueError(f"resource {name} is held by {existing.get('agent')}")
        record = dict(existing)
        del resources[name]
        return record


def _stale(task: Mapping[str, Any], hours: float) -> bool:
    if task.get("status") not in ACTIVE:
        return False
    try:
        updated = datetime.fromisoformat(
            str(task.get("updated_at", "")).replace("Z", "+00:00")
        )
    except ValueError:
        return True
    if updated.tzinfo is None:
        updated = updated.replace(tzinfo=timezone.utc)
    return (datetime.now(timezone.utc) - updated).total_seconds() > hours * 3600


def render_status(
    path: Path,
    queue: Mapping[str, Any],
    *,
    include_terminal: bool = False,
    stale_hours: float = STALE_HOURS,
) -> str:
    allowed = ALL if include_terminal else OPEN
    tasks = sorted(
        (
            task
            for task in queue.get("tasks", [])
            if isinstance(task, Mapping) and task.get("status") in allowed
        ),
        key=lambda task: (
            task.get("status") in TERMINAL,
            PRIORITY.get(task.get("priority"), 99),
            task.get("created_at", ""),
        ),
    )
    lines = [f"Queue: {path}"]
    if not tasks:
        lines.append("No queued or active recovery tasks.")
    for task in tasks:
        mark = "*" if _stale(task, stale_hours) else ""
        verified = str(task.get("last_verified_commit") or "-")[:10]
        lines.append(
            f"{task.get('status')}{mark:1}  {task.get('priority'):8}  "
            f"{task.get('owner')}  agent={task.get('agent') or '-'}  "
            f"branch={task.get('branch') or '-'}  "
            f"build={task.get('build_dir') or '-'}  verified={verified}"
        )
        if task.get("depends_on"):
            lines.append("    depends: " + ", ".join(task["depends_on"]))
        if task.get("shared_files"):
            lines.append("    shared: " + ", ".join(task["shared_files"]))
    resources = queue.get("resources", {})
    if resources:
        lines.append("Resources:")
        for name, record in sorted(resources.items()):
            lines.append(
                f"    {name}: {record.get('agent')} "
                f"owner={record.get('owner') or '-'}"
            )
    if any(_stale(task, stale_hours) for task in tasks):
        lines.append(f"* no update for more than {stale_hours:g} hours")
    return "\n".join(lines)


def queue_health(
    root: Path,
    queue_file: str | Path | None = None,
) -> tuple[str, str]:
    path = queue_path(root, queue_file)
    try:
        queue = read_queue(path)
        errors = validate_queue(queue)
    except QueueError as exc:
        return "fail", str(exc)
    if errors:
        return "fail", "; ".join(errors[:4])
    active = active_tasks(queue)
    resources = queue.get("resources", {})
    if not active:
        detail = f"empty shared queue: {path}"
        if resources:
            detail += f"; {len(resources)} resource lock(s)"
        return "pass", detail
    try:
        task = _current_claim(queue, root)
    except QueueError as exc:
        return "fail", str(exc)
    if task:
        status = "warn" if _stale(task, STALE_HOURS) else "pass"
        return status, (
            f"{task.get('agent')} owns {task.get('owner')} "
            f"({task.get('status')})"
        )
    return "warn", f"{len(active)} active claim(s); current worktree is unclaimed"


def _consumer_pairs(values: Sequence[str] | None) -> dict[str, str]:
    result: dict[str, str] = {}
    for value in values or []:
        key, separator, status = value.partition("=")
        if not separator or not key.strip() or not status.strip():
            raise QueueError("consumer results must use owner=status")
        result[key.strip()] = status.strip()
    return result


def add_queue_parser(subparsers: Any) -> argparse.ArgumentParser:
    queue = subparsers.add_parser(
        "queue", help="coordinate Claude/Codex worktrees"
    )
    queue.add_argument(
        "--queue-file",
        help=f"shared JSON path; defaults to Git common dir or ${QUEUE_ENV}",
    )
    actions = queue.add_subparsers(dest="queue_command", required=True)

    status = actions.add_parser("status")
    status.add_argument("--all", action="store_true")
    status.add_argument("--json", action="store_true")
    status.add_argument("--stale-hours", type=float, default=STALE_HOURS)

    actions.add_parser("check").add_argument("--json", action="store_true")

    diff = actions.add_parser("check-diff")
    diff.add_argument("--base")
    diff.add_argument("--agent")
    diff.add_argument("--json", action="store_true")

    add = actions.add_parser("add")
    add.add_argument("owner")
    add.add_argument("--target")
    add.add_argument("--source")
    add.add_argument("--priority", choices=sorted(PRIORITY), default="normal")
    add.add_argument("--shared", action="append", default=[])
    add.add_argument("--depends-on", action="append", default=[])
    add.add_argument("--batch")
    add.add_argument("--capability", action="append", default=[])
    add.add_argument(
        "--change-class",
        choices=sorted(CHANGE_CLASSES),
        default="private-source",
    )
    add.add_argument("--estimated-cost", type=int, default=1)
    add.add_argument("--verification-cost", type=int, default=1)
    add.add_argument("--base-ref", default=DEFAULT_WORKER_BASE)
    add.add_argument("--note")

    claim = actions.add_parser("claim")
    claim.add_argument("owner")
    claim.add_argument("--agent", required=True)
    claim.add_argument("--worktree")
    claim.add_argument("--branch")
    claim.add_argument("--build-dir")
    claim.add_argument("--target")
    claim.add_argument("--source")
    claim.add_argument("--priority", choices=sorted(PRIORITY), default=None)
    claim.add_argument("--shared", action="append", default=[])
    claim.add_argument("--capability", action="append", default=[])
    claim.add_argument("--change-class", choices=sorted(CHANGE_CLASSES))
    claim.add_argument("--base-ref")
    claim.add_argument("--note")

    next_parser = actions.add_parser("claim-next")
    next_parser.add_argument("--agent", required=True)
    next_parser.add_argument("--capability", action="append", default=[])
    next_parser.add_argument("--batch")

    update = actions.add_parser("update")
    update.add_argument("owner")
    update.add_argument("--agent")
    update.add_argument("--status", choices=sorted(ACTIVE))
    update.add_argument("--add-shared", action="append", default=[])
    update.add_argument("--remove-shared", action="append", default=[])
    update.add_argument("--base-ref")
    update.add_argument("--note")

    verify = actions.add_parser("verify")
    verify.add_argument("owner")
    verify.add_argument("--agent")
    verify.add_argument("--base")
    verify.add_argument(
        "--public-gate",
        choices=["pass", "fail", "not-run"],
        default="not-run",
    )
    verify.add_argument("--object-report")
    verify.add_argument("--functions-exact")
    verify.add_argument("--relocations")
    verify.add_argument("--consumer", action="append", default=[])
    verify.add_argument(
        "--retail-gate",
        choices=["pass", "fail", "not-run"],
        default="not-run",
    )
    verify.add_argument(
        "--checksum",
        choices=["pass", "fail", "not-run"],
        default="not-run",
    )
    verify.add_argument("--toolchain")

    release = actions.add_parser("release")
    release.add_argument("owner")
    release.add_argument("--agent")
    release.add_argument("--status", choices=sorted(TERMINAL), default="done")
    release.add_argument("--note")

    acquire = actions.add_parser("acquire-resource")
    acquire.add_argument("name")
    acquire.add_argument("--agent", required=True)
    acquire.add_argument("--owner")

    release_resource_parser = actions.add_parser("release-resource")
    release_resource_parser.add_argument("name")
    release_resource_parser.add_argument("--agent", required=True)
    return queue


def run_queue_command(
    args: argparse.Namespace,
    *,
    root: Path,
    owners: Sequence[Mapping[str, Any]] | None = None,
) -> int:
    path = queue_path(root, args.queue_file)
    if args.queue_command in {"status", "check"}:
        queue = read_queue(path)
        errors = validate_queue(queue)
        if args.queue_command == "check":
            if args.json:
                print(
                    json.dumps(
                        {
                            "path": str(path),
                            "errors": errors,
                            "active": len(active_tasks(queue)),
                            "resources": len(queue.get("resources", {})),
                        },
                        indent=2,
                    )
                )
            elif errors:
                print("queue invalid:\n- " + "\n- ".join(errors))
            else:
                print(
                    f"queue OK: {len(active_tasks(queue))} active claim(s), "
                    f"{len(queue.get('resources', {}))} resource lock(s) at {path}"
                )
            return 1 if errors else 0
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        if args.json:
            print(json.dumps({"path": str(path), "queue": queue}, indent=2))
        else:
            print(
                render_status(
                    path,
                    queue,
                    include_terminal=args.all,
                    stale_hours=args.stale_hours,
                )
            )
        return 0

    if args.queue_command == "check-diff":
        result = check_diff_claim(
            root,
            base=args.base,
            agent=args.agent,
            queue_file=args.queue_file,
        )
        if args.json:
            print(json.dumps(result, indent=2))
        elif result["errors"]:
            print("claim diff invalid:\n- " + "\n- ".join(result["errors"]))
        else:
            print(
                f"claim diff OK: {len(result['changed'])} changed path(s) "
                f"within {result['task']['owner']}"
            )
        return 1 if result["errors"] else 0

    if args.queue_command == "add":
        task = add_task(
            root,
            args.owner,
            target=args.target,
            source=args.source,
            priority=args.priority,
            shared_files=args.shared,
            note=args.note,
            queue_file=args.queue_file,
            owners=owners,
            depends_on=args.depends_on,
            batch=args.batch,
            capabilities=args.capability,
            change_class=args.change_class,
            estimated_cost=args.estimated_cost,
            verification_cost=args.verification_cost,
            base_ref=args.base_ref,
        )
    elif args.queue_command == "claim":
        task = claim_task(
            root,
            args.owner,
            agent=args.agent,
            worktree=args.worktree,
            branch=args.branch,
            build_dir=args.build_dir,
            target=args.target,
            source=args.source,
            priority=args.priority,
            shared_files=args.shared,
            note=args.note,
            queue_file=args.queue_file,
            owners=owners,
            capabilities=args.capability,
            change_class=args.change_class,
            base_ref=args.base_ref,
        )
    elif args.queue_command == "claim-next":
        task = claim_next(
            root,
            agent=args.agent,
            capabilities=args.capability,
            batch=args.batch,
            queue_file=args.queue_file,
            owners=owners,
        )
    elif args.queue_command == "update":
        task = update_task(
            root,
            args.owner,
            status=args.status,
            add_shared=args.add_shared,
            remove_shared=args.remove_shared,
            note=args.note,
            agent=args.agent,
            base_ref=args.base_ref,
            queue_file=args.queue_file,
            owners=owners,
        )
    elif args.queue_command == "verify":
        task = record_verification(
            root,
            args.owner,
            agent=args.agent,
            public_gate=args.public_gate,
            object_report=args.object_report,
            functions_exact=args.functions_exact,
            relocations=args.relocations,
            consumers=_consumer_pairs(args.consumer),
            retail_gate=args.retail_gate,
            checksum=args.checksum,
            toolchain=args.toolchain,
            base=args.base,
            queue_file=args.queue_file,
        )
    elif args.queue_command == "release":
        task = release_task(
            root,
            args.owner,
            status=args.status,
            note=args.note,
            agent=args.agent,
            queue_file=args.queue_file,
        )
    elif args.queue_command == "acquire-resource":
        task = acquire_resource(
            root,
            args.name,
            agent=args.agent,
            owner=args.owner,
            queue_file=args.queue_file,
        )
    else:
        task = release_resource(
            root,
            args.name,
            agent=args.agent,
            queue_file=args.queue_file,
        )
    print(json.dumps(task, indent=2))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)
    add_queue_parser(sub)
    args = parser.parse_args()
    try:
        return run_queue_command(args, root=git_root(args.root))
    except (OSError, QueueError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
