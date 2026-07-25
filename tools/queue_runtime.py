#!/usr/bin/env python3
"""Safety and scheduling layer over the local shared claim queue.

This module deliberately reuses ``tools.agent_queue`` storage and locking so
existing local queues continue to work while adding diff enforcement,
verification proof, dependencies, resource locks, and strict worktree checks.
"""

from __future__ import annotations

import os
import socket
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence

from tools import agent_queue as legacy

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

QueueError = legacy.QueueError


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _canon(value: str | None) -> str:
    return os.path.normcase(os.path.realpath(os.path.expanduser(value or "")))


def _repo_path(value: str | None) -> str | None:
    if value is None or not value.strip():
        return None
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise QueueError(f"path must be repository-relative: {value}")
    return None if path.as_posix() == "." else path.as_posix()


def _paths(values: Sequence[str] | None) -> list[str]:
    return sorted({path for value in values or [] if (path := _repo_path(value))})


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
    catalog: Sequence[Mapping[str, Any]] | None,
) -> dict[str, Mapping[str, Any]]:
    return {
        str(item.get("id")): item
        for item in catalog or []
        if item.get("id")
    }


def _catalog_source(
    owner: str,
    source: str | None,
    catalog: Sequence[Mapping[str, Any]] | None,
) -> str | None:
    if source:
        return _repo_path(source)
    info = _catalog_map(catalog).get(owner)
    if info and isinstance(info.get("source"), str):
        return _repo_path(str(info["source"]))
    return None


def _git_common(root: Path) -> Path:
    value = Path(legacy._run(root, "git", "rev-parse", "--git-common-dir"))
    return (value if value.is_absolute() else root / value).resolve()


def _within(child: Path, parent: Path) -> bool:
    try:
        return os.path.commonpath([str(child), str(parent)]) == str(parent)
    except ValueError:
        return False


def validate_worktree(
    repository_root: Path,
    worktree: Path,
    branch: str,
    build_dir: Path,
) -> list[str]:
    errors: list[str] = []
    if not worktree.is_dir():
        return [f"worktree does not exist: {worktree}"]
    try:
        actual_root = Path(
            legacy._run(worktree, "git", "rev-parse", "--show-toplevel")
        ).resolve()
        if actual_root != worktree.resolve():
            errors.append(f"worktree root is {actual_root}, not {worktree}")
        if _git_common(repository_root) != _git_common(worktree):
            errors.append("worktree belongs to a different Git common directory")
        actual_branch = legacy._run(
            worktree, "git", "branch", "--show-current"
        )
        if actual_branch != branch:
            errors.append(
                f"worktree branch is {actual_branch or 'detached'}, not {branch}"
            )
        listing = legacy._run(
            repository_root, "git", "worktree", "list", "--porcelain"
        )
        registered = {
            _canon(line.removeprefix("worktree "))
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


def _current_task(
    queue: Mapping[str, Any], root: Path, agent: str | None = None
) -> dict[str, Any] | None:
    branch = legacy._run(root, "git", "branch", "--show-current")
    current = _canon(str(root.resolve()))
    matches = [
        task
        for task in legacy.active_tasks(queue)
        if (
            _canon(task.get("worktree")) == current
            or task.get("branch") == branch
        )
        and (agent is None or task.get("agent") == agent)
    ]
    if len(matches) > 1:
        raise QueueError("current worktree or branch matches multiple claims")
    return matches[0] if matches else None


def _dependency_conflicts(
    left: Mapping[str, Any],
    right: Mapping[str, Any],
    catalog: Sequence[Mapping[str, Any]] | None,
) -> list[str]:
    by_id = _catalog_map(catalog)
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


def advanced_conflicts(
    queue: Mapping[str, Any],
    candidate: Mapping[str, Any],
    catalog: Sequence[Mapping[str, Any]] | None,
) -> list[str]:
    conflicts: list[str] = []
    for task in legacy.active_tasks(queue):
        if task.get("id") == candidate.get("id"):
            continue
        label = f"{task.get('owner')} ({task.get('agent')})"
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
        conflicts.extend(_dependency_conflicts(task, candidate, catalog))
    return sorted(set(conflicts))


def _task_by_id(queue: Mapping[str, Any], task_id: str) -> dict[str, Any]:
    for task in queue.get("tasks", []):
        if isinstance(task, dict) and task.get("id") == task_id:
            return task
    raise QueueError(f"queue task disappeared: {task_id}")


def add_task(
    root: Path,
    owner: str,
    *,
    catalog: Sequence[Mapping[str, Any]] | None = None,
    target: str | None = None,
    source: str | None = None,
    priority: str = "normal",
    shared_files: Sequence[str] | None = None,
    depends_on: Sequence[str] | None = None,
    batch: str | None = None,
    capabilities: Sequence[str] | None = None,
    change_class: str = "private-source",
    estimated_cost: int = 1,
    verification_cost: int = 1,
    base_ref: str = "origin/main",
    note: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    if change_class not in CHANGE_CLASSES:
        raise QueueError("invalid change class")
    task = legacy.add_task(
        root,
        owner,
        target=target,
        source=_catalog_source(owner, source, catalog),
        priority=priority,
        shared_files=shared_files,
        note=note,
        queue_file=queue_file,
        owners=catalog,
    )
    path = legacy.queue_path(root, queue_file)
    with legacy.locked_queue(path) as queue:
        stored = _task_by_id(queue, str(task["id"]))
        stored.update(
            {
                "depends_on": sorted(set(depends_on or [])),
                "batch": batch or None,
                "capabilities": sorted(set(capabilities or [])),
                "change_class": change_class,
                "estimated_cost": max(1, int(estimated_cost)),
                "verification_cost": max(1, int(verification_cost)),
                "base_ref": base_ref,
                "verification": None,
            }
        )
        return dict(stored)


def claim_task(
    root: Path,
    owner: str,
    *,
    agent: str,
    catalog: Sequence[Mapping[str, Any]] | None = None,
    worktree: str | Path | None = None,
    branch: str | None = None,
    build_dir: str | Path | None = None,
    target: str | None = None,
    source: str | None = None,
    priority: str | None = None,
    shared_files: Sequence[str] | None = None,
    capabilities: Sequence[str] | None = None,
    change_class: str | None = None,
    base_ref: str | None = None,
    note: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    assigned_worktree = (
        Path(worktree).expanduser().resolve() if worktree else root.resolve()
    )
    assigned_branch = branch or legacy._run(
        assigned_worktree, "git", "branch", "--show-current"
    )
    assigned_build = (
        Path(build_dir).expanduser().resolve()
        if build_dir
        else assigned_worktree / "build"
    )
    errors = validate_worktree(
        root, assigned_worktree, assigned_branch, assigned_build
    )
    if errors:
        raise QueueError("invalid worktree claim:\n- " + "\n- ".join(errors))

    queue = legacy.read_queue(legacy.queue_path(root, queue_file))
    pending = legacy._find(queue, owner)
    inherited_priority = (
        str(pending.get("priority"))
        if pending and pending.get("status") == "pending"
        else "normal"
    )
    selected_priority = priority or inherited_priority
    task = legacy.claim_task(
        root,
        owner,
        agent=agent,
        worktree=assigned_worktree,
        branch=assigned_branch,
        build_dir=assigned_build,
        target=target,
        source=_catalog_source(owner, source, catalog),
        priority=selected_priority,
        shared_files=shared_files,
        note=note,
        queue_file=queue_file,
        owners=catalog,
    )
    path = legacy.queue_path(root, queue_file)
    with legacy.locked_queue(path) as queue:
        stored = _task_by_id(queue, str(task["id"]))
        stored.setdefault("depends_on", [])
        stored.setdefault("batch", None)
        stored.setdefault("capabilities", [])
        stored.setdefault("change_class", "private-source")
        stored.setdefault("estimated_cost", 1)
        stored.setdefault("verification_cost", 1)
        stored.setdefault("base_ref", "origin/main")
        if capabilities:
            stored["capabilities"] = sorted(
                set(stored["capabilities"]) | set(capabilities)
            )
        if change_class:
            if change_class not in CHANGE_CLASSES:
                raise QueueError("invalid change class")
            stored["change_class"] = change_class
        if base_ref:
            stored["base_ref"] = base_ref
        stored["verification"] = None
        stored["last_verified_commit"] = None
        conflicts = advanced_conflicts(queue, stored, catalog)
        if conflicts:
            stored.update(
                {
                    "status": "pending",
                    "agent": None,
                    "worktree": None,
                    "branch": None,
                    "build_dir": None,
                    "claimed_at": None,
                }
            )
            raise QueueError("claim conflicts:\n- " + "\n- ".join(conflicts))
        return dict(stored)


def _dependency_done(queue: Mapping[str, Any], owner: str) -> bool:
    matches = [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, Mapping) and task.get("owner") == owner
    ]
    return bool(matches) and matches[-1].get("status") == "done"


def claim_next(
    root: Path,
    *,
    agent: str,
    catalog: Sequence[Mapping[str, Any]] | None = None,
    capabilities: Sequence[str] | None = None,
    batch: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    queue = legacy.read_queue(legacy.queue_path(root, queue_file))
    requested = set(capabilities or [])
    candidates = []
    for task in queue.get("tasks", []):
        if not isinstance(task, Mapping) or task.get("status") != "pending":
            continue
        if batch and task.get("batch") != batch:
            continue
        required = set(task.get("capabilities", []))
        if not required.issubset(requested):
            continue
        if not all(
            _dependency_done(queue, dependency)
            for dependency in task.get("depends_on", [])
        ):
            continue
        candidates.append(task)
    candidates.sort(
        key=lambda task: (
            legacy.PRIORITY.get(task.get("priority"), 99),
            int(task.get("estimated_cost", 1))
            + int(task.get("verification_cost", 1)),
            task.get("created_at", ""),
        )
    )
    for task in candidates:
        try:
            return claim_task(
                root,
                str(task["owner"]),
                agent=agent,
                catalog=catalog,
                capabilities=capabilities,
                queue_file=queue_file,
            )
        except QueueError as exc:
            if "conflict" not in str(exc):
                raise
    raise QueueError("no dependency-ready, conflict-free task is available")


def changed_paths(
    root: Path, base: str, *, include_worktree: bool = True
) -> set[str]:
    commands = [["git", "diff", "--name-only", f"{base}...HEAD"]]
    if include_worktree:
        commands.extend(
            [
                ["git", "diff", "--cached", "--name-only"],
                ["git", "diff", "--name-only"],
                ["git", "ls-files", "--others", "--exclude-standard"],
            ]
        )
    result: set[str] = set()
    for command in commands:
        output = legacy._run(root, *command)
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
    queue = legacy.read_queue(legacy.queue_path(root, queue_file))
    errors = legacy.validate_queue(queue)
    if errors:
        raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
    task = _current_task(queue, root, agent)
    if task is None:
        if require_claim:
            raise QueueError("current worktree has no active claim")
        return {"task": None, "changed": [], "errors": []}
    effective_base = base or str(task.get("base_ref") or "origin/main")
    changed = sorted(changed_paths(root, effective_base))
    allowed = _write_paths(task)
    diff_errors: list[str] = []
    for changed_path in changed:
        if not any(_overlap(changed_path, path) for path in allowed):
            diff_errors.append(
                f"undeclared changed path {changed_path}; claim it with --shared"
            )
        for other in legacy.active_tasks(queue):
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


def _proof_errors(task: Mapping[str, Any], *, terminal: bool) -> list[str]:
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
    path = legacy.queue_path(root, queue_file)
    with legacy.locked_queue(path) as queue:
        task = legacy._find(queue, owner)
        if not task or task.get("status") == "pending":
            raise QueueError(f"{owner} is not actively claimed")
        if agent and task.get("agent") != agent:
            raise QueueError(
                f"{owner} is assigned to {task.get('agent')}, not {agent}"
            )
        current = _current_task(queue, root, agent)
        if not current or current.get("id") != task.get("id"):
            raise QueueError(
                "verification must run from the claimed worktree and branch"
            )
        worktree_errors = validate_worktree(
            root,
            Path(str(task["worktree"])),
            str(task["branch"]),
            Path(str(task["build_dir"])),
        )
        if worktree_errors:
            raise QueueError(
                "worktree validation failed:\n- " + "\n- ".join(worktree_errors)
            )
        if legacy._run(root, "git", "status", "--porcelain"):
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
        head = legacy._run(root, "git", "rev-parse", "HEAD")
        proof = {
            "verified_commit": head,
            "verified_at": _now(),
            "clean": True,
            "claim_diff": "pass",
            "base": diff["base"],
            "public_gate": public_gate,
            "object_report": object_report,
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
    catalog: Sequence[Mapping[str, Any]] | None = None,
    status: str | None = None,
    add_shared: Sequence[str] | None = None,
    remove_shared: Sequence[str] | None = None,
    note: str | None = None,
    agent: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = legacy.queue_path(root, queue_file)
    if status == "ready":
        queue = legacy.read_queue(path)
        task = legacy._find(queue, owner)
        errors = _proof_errors(task or {}, terminal=False)
        if errors:
            raise QueueError("task is not ready:\n- " + "\n- ".join(errors))
    task = legacy.update_task(
        root,
        owner,
        status=status,
        add_shared=add_shared,
        remove_shared=remove_shared,
        note=note,
        agent=agent,
        queue_file=queue_file,
    )
    with legacy.locked_queue(path) as queue:
        stored = _task_by_id(queue, str(task["id"]))
        conflicts = advanced_conflicts(queue, stored, catalog)
        if conflicts:
            raise QueueError("update conflicts:\n- " + "\n- ".join(conflicts))
        return dict(stored)


def release_task(
    root: Path,
    owner: str,
    *,
    status: str = "done",
    note: str | None = None,
    agent: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = legacy.queue_path(root, queue_file)
    queue = legacy.read_queue(path)
    task = legacy._find(queue, owner)
    if not task:
        raise QueueError(f"no open task exists for {owner}")
    if status == "done":
        current = _current_task(queue, root, agent)
        if not current or current.get("id") != task.get("id"):
            raise QueueError("done must be recorded from the claimed worktree")
        head = legacy._run(root, "git", "rev-parse", "HEAD")
        proof = task.get("verification") or {}
        if proof.get("verified_commit") != head:
            raise QueueError("HEAD differs from the last verified commit")
        if legacy._run(root, "git", "status", "--porcelain"):
            raise QueueError("done requires a clean working tree")
        errors = _proof_errors(task, terminal=True)
        if errors:
            raise QueueError(
                "task cannot be completed:\n- " + "\n- ".join(errors)
            )
    return legacy.release_task(
        root,
        owner,
        status=status,
        note=note,
        agent=agent,
        queue_file=queue_file,
    )


def acquire_resource(
    root: Path,
    name: str,
    *,
    agent: str,
    owner: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    path = legacy.queue_path(root, queue_file)
    with legacy.locked_queue(path) as queue:
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
            "branch": legacy._run(root, "git", "branch", "--show-current"),
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
    path = legacy.queue_path(root, queue_file)
    with legacy.locked_queue(path) as queue:
        resources = queue.setdefault("resources", {})
        existing = resources.get(name)
        if not isinstance(existing, Mapping):
            raise QueueError(f"resource {name} is not held")
        if existing.get("agent") != agent:
            raise QueueError(f"resource {name} is held by {existing.get('agent')}")
        record = dict(existing)
        del resources[name]
        return record


def queue_health(
    root: Path, queue_file: str | Path | None = None
) -> tuple[str, str]:
    path = legacy.queue_path(root, queue_file)
    try:
        queue = legacy.read_queue(path)
        errors = legacy.validate_queue(queue)
    except QueueError as exc:
        return "fail", str(exc)
    if errors:
        return "fail", "; ".join(errors[:4])
    for task in legacy.active_tasks(queue):
        worktree_errors = validate_worktree(
            root,
            Path(str(task.get("worktree"))),
            str(task.get("branch")),
            Path(str(task.get("build_dir"))),
        )
        if worktree_errors:
            return "fail", f"{task.get('owner')}: {worktree_errors[0]}"
    active = legacy.active_tasks(queue)
    resources = queue.get("resources", {})
    if not active:
        detail = f"empty shared queue: {path}"
        if resources:
            detail += f"; {len(resources)} resource lock(s)"
        return "pass", detail
    task = _current_task(queue, root)
    if task:
        return (
            "pass",
            f"{task.get('agent')} owns {task.get('owner')} ({task.get('status')})",
        )
    return "warn", f"{len(active)} active claim(s); current worktree is unclaimed"


def consumer_pairs(values: Sequence[str] | None) -> dict[str, str]:
    result: dict[str, str] = {}
    for value in values or []:
        key, separator, status = value.partition("=")
        if not separator or not key.strip() or not status.strip():
            raise QueueError("consumer results must use owner=status")
        result[key.strip()] = status.strip()
    return result
