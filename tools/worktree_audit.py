#!/usr/bin/env python3
"""Audit every active queue claim against its actual Git worktree assignment."""

from __future__ import annotations

from pathlib import Path
from typing import Any

from tools import agent_queue as queue


def audit_active_worktrees(
    root: Path, queue_file: str | Path | None = None
) -> list[dict[str, Any]]:
    path = queue.queue_path(root, queue_file)
    data = queue.read_queue(path)
    result: list[dict[str, Any]] = []
    for task in queue.active_tasks(data):
        errors = queue._validate_worktree(
            root,
            Path(str(task.get("worktree") or "")),
            str(task.get("branch") or ""),
            Path(str(task.get("build_dir") or "")),
        )
        result.append(
            {
                "owner": task.get("owner"),
                "agent": task.get("agent"),
                "worktree": task.get("worktree"),
                "branch": task.get("branch"),
                "build_dir": task.get("build_dir"),
                "errors": errors,
            }
        )
    return result


def worktree_audit_summary(
    root: Path, queue_file: str | Path | None = None
) -> tuple[str, str]:
    values = audit_active_worktrees(root, queue_file)
    failures = [value for value in values if value["errors"]]
    if failures:
        first = failures[0]
        return (
            "fail",
            f"{first['owner']}: {first['errors'][0]}",
        )
    return (
        "pass",
        f"{len(values)} active worktree assignment(s) valid",
    )
