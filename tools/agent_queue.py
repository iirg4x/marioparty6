#!/usr/bin/env python3
"""Shared task claims for Claude/Codex worktrees on one local repository."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import socket
import subprocess
import tempfile
import time
import uuid
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Iterator, Sequence

SCHEMA_VERSION = 1
QUEUE_ENV = "MP6_AGENT_QUEUE"
ACTIVE = {"claimed", "researching", "coding", "verifying", "blocked", "ready"}
OPEN = {"pending", *ACTIVE}
TERMINAL = {"done", "released", "cancelled"}
ALL = OPEN | TERMINAL
PRIORITY = {"critical": 0, "high": 1, "normal": 2, "low": 3}
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
    return Path(_run(start, "git", "rev-parse", "--show-toplevel")).resolve()


def queue_path(root: Path, override: str | Path | None = None) -> Path:
    raw = override or os.environ.get(QUEUE_ENV)
    if raw:
        path = Path(raw).expanduser()
        path = path if path.is_absolute() else root / path
        path = path.resolve()
        return path if path.suffix.lower() == ".json" else path / "queue.json"
    common = Path(_run(root, "git", "rev-parse", "--git-common-dir"))
    common = common if common.is_absolute() else root / common
    return common.resolve() / "agent-coordination" / "queue.json"


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _empty() -> dict[str, Any]:
    return {
        "schema_version": SCHEMA_VERSION,
        "updated_at": _now(),
        "tasks": [],
    }


def read_queue(path: Path) -> dict[str, Any]:
    if not path.exists():
        return _empty()
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise QueueError(
            f"invalid queue JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    if not isinstance(value, dict):
        raise QueueError(f"{path}: queue root must be an object")
    return value


def _write(path: Path, queue: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    queue["updated_at"] = _now()
    with tempfile.NamedTemporaryFile(
        "w",
        encoding="utf-8",
        dir=path.parent,
        delete=False,
    ) as handle:
        json.dump(queue, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)


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
        queue = read_queue(path)
        yield queue
        _write(path, queue)
    finally:
        shutil.rmtree(lock, ignore_errors=True)


def _repo_path(value: str | None) -> str | None:
    if value is None or not value.strip():
        return None
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise QueueError(f"path must be repository-relative: {value}")
    return None if path.as_posix() == "." else path.as_posix()


def _shared(values: Sequence[str] | None) -> list[str]:
    return sorted(
        {path for value in values or [] if (path := _repo_path(value))}
    )


def _abs(value: str | Path | None, base: Path) -> Path:
    path = Path(value or base).expanduser()
    return (path if path.is_absolute() else base / path).resolve()


def _canon(value: str | None) -> str:
    return os.path.normcase(os.path.realpath(os.path.expanduser(value or "")))


def _overlap(left: str, right: str) -> bool:
    a = PurePosixPath(left).parts
    b = PurePosixPath(right).parts
    return a == b or a == b[: len(a)] or b == a[: len(b)]


def _write_paths(task: dict[str, Any]) -> set[str]:
    result = set(_shared(task.get("shared_files", [])))
    if source := _repo_path(task.get("source")):
        result.add(source)
    return result


def active_tasks(queue: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict) and task.get("status") in ACTIVE
    ]


def _open_tasks(queue: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict) and task.get("status") in OPEN
    ]


def _find(queue: dict[str, Any], owner: str) -> dict[str, Any] | None:
    matches = [task for task in _open_tasks(queue) if task.get("owner") == owner]
    if len(matches) > 1:
        raise QueueError(f"duplicate open tasks for {owner}")
    return matches[0] if matches else None


def validate_queue(queue: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    if queue.get("schema_version") != SCHEMA_VERSION:
        errors.append(f"schema_version must be {SCHEMA_VERSION}")
    tasks = queue.get("tasks")
    if not isinstance(tasks, list):
        return [*errors, "tasks must be a list"]

    ids: set[str] = set()
    owners: set[str] = set()
    for index, task in enumerate(tasks):
        where = f"tasks[{index}]"
        if not isinstance(task, dict):
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
    return sorted(set(errors))


def _source(
    owner: str,
    source: str | None,
    known: Sequence[dict[str, Any]] | None,
) -> str | None:
    if source:
        return _repo_path(source)
    for item in known or []:
        if item.get("id") == owner and isinstance(item.get("source"), str):
            return _repo_path(item["source"])
    if owner.replace("\\", "/").startswith(("src/", "include/", "config/")):
        return _repo_path(owner)
    return None


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
    owners: Sequence[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    owner = owner.strip()
    if not owner or priority not in PRIORITY:
        raise QueueError("valid owner and priority are required")
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        errors = validate_queue(queue)
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        if _find(queue, owner):
            raise QueueError(f"an open task already exists for {owner}")
        now = _now()
        task = {
            "id": uuid.uuid4().hex,
            "owner": owner,
            "target": target or None,
            "source": _source(owner, source, owners),
            "priority": priority,
            "status": "pending",
            "agent": None,
            "worktree": None,
            "branch": None,
            "build_dir": None,
            "shared_files": _shared(shared_files),
            "created_at": now,
            "claimed_at": None,
            "updated_at": now,
            "last_verified_commit": None,
            "note": note or "",
        }
        queue.setdefault("tasks", []).append(task)
        return dict(task)


def _conflicts(queue: dict[str, Any], candidate: dict[str, Any]) -> list[str]:
    conflicts: list[str] = []
    for task in active_tasks(queue):
        if task.get("id") == candidate.get("id"):
            continue
        label = f"{task.get('owner')} ({task.get('agent')})"
        if task.get("owner") == candidate.get("owner"):
            conflicts.append(f"owner already claimed by {label}")
        if task.get("branch") == candidate.get("branch"):
            conflicts.append(f"branch already used by {label}")
        for key in ("worktree", "build_dir"):
            if _canon(task.get(key)) == _canon(candidate.get(key)):
                conflicts.append(f"{key} already used by {label}")
        for old in _write_paths(task):
            for new in _write_paths(candidate):
                if _overlap(old, new):
                    conflicts.append(
                        f"write path {new} overlaps {old} claimed by {label}"
                    )
    return sorted(set(conflicts))


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
    priority: str = "normal",
    shared_files: Sequence[str] | None = None,
    note: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    owner = owner.strip()
    agent = agent.strip()
    if not owner or not agent or priority not in PRIORITY:
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
            task = {
                "id": uuid.uuid4().hex,
                "owner": owner,
                "target": target or None,
                "source": _source(owner, source, owners),
                "priority": priority,
                "status": "pending",
                "shared_files": [],
                "created_at": now,
                "last_verified_commit": None,
                "note": "",
            }
            queue.setdefault("tasks", []).append(task)
        if target:
            task["target"] = target
        if source:
            task["source"] = _repo_path(source)
        task["priority"] = priority
        task["shared_files"] = _shared(
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
            }
        )
        if conflicts := _conflicts(queue, task):
            raise QueueError("claim conflicts:\n- " + "\n- ".join(conflicts))
        return dict(task)


def _commit(root: Path, value: str | None) -> str | None:
    return _run(root, "git", "rev-parse", value) if value else None


def update_task(
    root: Path,
    owner: str,
    *,
    status: str | None = None,
    add_shared: Sequence[str] | None = None,
    remove_shared: Sequence[str] | None = None,
    verified_commit: str | None = None,
    note: str | None = None,
    agent: str | None = None,
    queue_file: str | Path | None = None,
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
        if status:
            task["status"] = status
        shared = set(_shared(task.get("shared_files", []))) | set(
            _shared(add_shared)
        )
        shared -= set(_shared(remove_shared))
        task["shared_files"] = sorted(shared)
        if verified_commit:
            task["last_verified_commit"] = _commit(root, verified_commit)
        if note is not None:
            task["note"] = note
        task["updated_at"] = _now()
        if conflicts := _conflicts(queue, task):
            raise QueueError("update conflicts:\n- " + "\n- ".join(conflicts))
        return dict(task)


def release_task(
    root: Path,
    owner: str,
    *,
    status: str = "done",
    verified_commit: str | None = None,
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
        if verified_commit:
            task["last_verified_commit"] = _commit(root, verified_commit)
        if note is not None:
            task["note"] = note
        task["status"] = status
        task["updated_at"] = task["released_at"] = _now()
        return dict(task)


def _stale(task: dict[str, Any], hours: float) -> bool:
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
    return (
        datetime.now(timezone.utc) - updated
    ).total_seconds() > hours * 3600


def render_status(
    path: Path,
    queue: dict[str, Any],
    *,
    include_terminal: bool = False,
    stale_hours: float = STALE_HOURS,
) -> str:
    allowed = ALL if include_terminal else OPEN
    tasks = sorted(
        (task for task in queue.get("tasks", []) if task.get("status") in allowed),
        key=lambda task: (
            task.get("status") in TERMINAL,
            PRIORITY.get(task.get("priority"), 99),
            task.get("created_at", ""),
        ),
    )
    lines = [f"Queue: {path}"]
    if not tasks:
        return "\n".join([*lines, "No queued or active recovery tasks."])
    for task in tasks:
        mark = "*" if _stale(task, stale_hours) else ""
        verified = str(task.get("last_verified_commit") or "-")[:10]
        lines.append(
            f"{task.get('status')}{mark:1}  {task.get('priority'):8}  "
            f"{task.get('owner')}  agent={task.get('agent') or '-'}  "
            f"branch={task.get('branch') or '-'}  "
            f"build={task.get('build_dir') or '-'}  verified={verified}"
        )
        if task.get("shared_files"):
            lines.append("    shared: " + ", ".join(task["shared_files"]))
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
    if not active:
        return "pass", f"empty shared queue: {path}"
    branch = _run(root, "git", "branch", "--show-current")
    current = _canon(str(root.resolve()))
    matches = [
        task
        for task in active
        if _canon(task.get("worktree")) == current
        or task.get("branch") == branch
    ]
    if len(matches) == 1:
        task = matches[0]
        status = "warn" if _stale(task, STALE_HOURS) else "pass"
        return status, (
            f"{task.get('agent')} owns {task.get('owner')} "
            f"({task.get('status')})"
        )
    if len(matches) > 1:
        return "fail", "current worktree or branch matches multiple claims"
    return "warn", f"{len(active)} active claim(s); current worktree is unclaimed"


def add_queue_parser(subparsers: Any) -> argparse.ArgumentParser:
    queue = subparsers.add_parser(
        "queue",
        help="coordinate Claude/Codex worktrees",
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

    add = actions.add_parser("add")
    add.add_argument("owner")
    add.add_argument("--target")
    add.add_argument("--source")
    add.add_argument("--priority", choices=sorted(PRIORITY), default="normal")
    add.add_argument("--shared", action="append", default=[])
    add.add_argument("--note")

    claim = actions.add_parser("claim")
    claim.add_argument("owner")
    claim.add_argument("--agent", required=True)
    claim.add_argument("--worktree")
    claim.add_argument("--branch")
    claim.add_argument("--build-dir")
    claim.add_argument("--target")
    claim.add_argument("--source")
    claim.add_argument("--priority", choices=sorted(PRIORITY), default="normal")
    claim.add_argument("--shared", action="append", default=[])
    claim.add_argument("--note")

    update = actions.add_parser("update")
    update.add_argument("owner")
    update.add_argument("--agent")
    update.add_argument("--status", choices=sorted(ACTIVE))
    update.add_argument("--add-shared", action="append", default=[])
    update.add_argument("--remove-shared", action="append", default=[])
    update.add_argument("--verified-commit")
    update.add_argument("--note")

    release = actions.add_parser("release")
    release.add_argument("owner")
    release.add_argument("--agent")
    release.add_argument("--status", choices=sorted(TERMINAL), default="done")
    release.add_argument("--verified-commit")
    release.add_argument("--note")
    return queue


def run_queue_command(
    args: argparse.Namespace,
    *,
    root: Path,
    owners: Sequence[dict[str, Any]] | None = None,
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
                        },
                        indent=2,
                    )
                )
            elif errors:
                print("queue invalid:\n- " + "\n- ".join(errors))
            else:
                print(
                    f"queue OK: {len(active_tasks(queue))} "
                    f"active claim(s) at {path}"
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
        )
    elif args.queue_command == "update":
        task = update_task(
            root,
            args.owner,
            status=args.status,
            add_shared=args.add_shared,
            remove_shared=args.remove_shared,
            verified_commit=args.verified_commit,
            note=args.note,
            agent=args.agent,
            queue_file=args.queue_file,
        )
    else:
        task = release_task(
            root,
            args.owner,
            status=args.status,
            verified_commit=args.verified_commit,
            note=args.note,
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
