#!/usr/bin/env python3
"""Check changed paths against a worker claim or integration resource."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any, Mapping

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import agent_queue as queue


def _canon(value: str | None) -> str:
    return os.path.normcase(os.path.realpath(os.path.expanduser(value or "")))


def _branch(root: Path) -> str:
    result = subprocess.run(
        ["git", "branch", "--show-current"],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    return result.stdout.strip()


def _integration_result(
    root: Path, base: str | None, queue_file: str | None
) -> dict[str, Any]:
    data = queue.read_queue(queue.queue_path(root, queue_file))
    errors = queue.validate_queue(data)
    if errors:
        raise queue.QueueError("queue invalid:\n- " + "\n- ".join(errors))
    current = _canon(str(root.resolve()))
    branch = _branch(root)
    resources = data.get("resources", {})
    locks = [
        value
        for name, value in resources.items()
        if name == "integration"
        and isinstance(value, Mapping)
        and (_canon(str(value.get("worktree"))) == current or value.get("branch") == branch)
    ]
    if len(locks) != 1:
        raise queue.QueueError("current worktree has no unique integration resource")
    owner = locks[0].get("owner")
    tasks = [
        task
        for task in data.get("tasks", [])
        if isinstance(task, dict)
        and task.get("owner") == owner
        and task.get("status") == "ready"
    ]
    if len(tasks) != 1:
        raise queue.QueueError("integration resource must name one ready task")
    task = tasks[0]
    effective_base = base or str(
        task.get("base_ref") or queue.DEFAULT_WORKER_BASE
    )
    changed = sorted(queue.changed_paths(root, effective_base))
    allowed = queue._write_paths(task)
    failures = [
        f"integration path {path} is outside ready task {owner}"
        for path in changed
        if not any(queue._overlap(path, claimed) for claimed in allowed)
    ]
    for path in changed:
        for other in queue.active_tasks(data):
            if other.get("id") == task.get("id"):
                continue
            for protected in queue._write_paths(other):
                if queue._overlap(path, protected):
                    failures.append(
                        f"{path} overlaps {protected} owned by {other.get('owner')}"
                    )
    return {
        "mode": "integration",
        "task": task,
        "base": effective_base,
        "changed": changed,
        "allowed": sorted(allowed),
        "errors": sorted(set(failures)),
    }


def check(
    root: Path, base: str | None = None, queue_file: str | None = None
) -> dict[str, Any]:
    try:
        return {
            "mode": "worker",
            **queue.check_diff_claim(root, base=base, queue_file=queue_file),
        }
    except queue.QueueError as exc:
        if "no active claim" not in str(exc):
            raise
    return _integration_result(root, base, queue_file)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    parser.add_argument("--base")
    parser.add_argument("--queue-file")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        result = check(queue.git_root(args.root), args.base, args.queue_file)
        if args.json:
            print(json.dumps(result, indent=2))
        elif result["errors"]:
            print("claim diff invalid:\n- " + "\n- ".join(result["errors"]))
        else:
            print(f"{result['mode']} diff OK: {len(result['changed'])} path(s)")
        return 1 if result["errors"] else 0
    except (OSError, queue.QueueError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
