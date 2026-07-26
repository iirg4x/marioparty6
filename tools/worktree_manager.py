#!/usr/bin/env python3
"""Create and retire isolated, queue-backed agent worktrees."""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
from pathlib import Path
from typing import Any, Mapping, Sequence

from tools.agent_queue import QueueError, claim_task, queue_path, read_queue
from tools.workspace_policy import DEFAULT_WORKER_BASE


class WorktreeError(ValueError):
    pass


def _run(cwd: Path, *args: str) -> str:
    result = subprocess.run(
        args, cwd=cwd, text=True, capture_output=True, check=False
    )
    if result.returncode:
        raise WorktreeError(
            result.stderr.strip() or "command failed: " + " ".join(args)
        )
    return result.stdout.strip()


def _slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-") or "task"


def _link_directory(source: Path, destination: Path) -> None:
    if destination.is_symlink():
        return
    if destination.is_dir():
        contents = {item.name for item in destination.iterdir()}
        if contents <= {".gitkeep"}:
            shutil.rmtree(destination)
        else:
            raise WorktreeError(
                f"retail destination is not empty: {destination}"
            )
    elif destination.exists():
        raise WorktreeError(f"retail destination already exists: {destination}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    try:
        destination.symlink_to(source, target_is_directory=True)
        return
    except OSError:
        if os.name != "nt":
            raise
    result = subprocess.run(
        ["cmd", "/c", "mklink", "/J", str(destination), str(source)],
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        raise WorktreeError(result.stderr.strip() or result.stdout.strip())


def create_worktree(
    root: Path,
    *,
    agent: str,
    owner: str,
    base: str = DEFAULT_WORKER_BASE,
    branch: str | None = None,
    path: str | Path | None = None,
    build_dir: str | Path | None = None,
    retail: str | Path | None = None,
    target: str | None = None,
    source: str | None = None,
    shared_files: Sequence[str] | None = None,
    change_class: str | None = None,
    queue_file: str | Path | None = None,
    owners: Sequence[Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    slug = _slug(owner)
    assigned_branch = branch or f"agent/{_slug(agent)}-{slug}"
    assigned_path = (
        Path(path).expanduser().resolve()
        if path
        else root.parent / f"{root.name}-{_slug(agent)}-{slug}"
    )
    if assigned_path.exists():
        raise WorktreeError(f"worktree path already exists: {assigned_path}")
    _run(
        root,
        "git",
        "worktree",
        "add",
        "-b",
        assigned_branch,
        str(assigned_path),
        base,
    )
    try:
        assigned_build = (
            Path(build_dir).expanduser().resolve()
            if build_dir
            else assigned_path / "build"
        )
        assigned_build.mkdir(parents=True, exist_ok=True)
        if retail:
            retail_path = Path(retail).expanduser().resolve()
            if not retail_path.is_dir():
                raise WorktreeError(
                    f"retail directory does not exist: {retail_path}"
                )
            _link_directory(retail_path, assigned_path / "orig" / "GP6E01")
        task = claim_task(
            root,
            owner,
            agent=agent,
            worktree=assigned_path,
            branch=assigned_branch,
            build_dir=assigned_build,
            target=target,
            source=source,
            shared_files=shared_files,
            change_class=change_class,
            base_ref=base,
            queue_file=queue_file,
            owners=owners,
        )
        return {
            "task": task,
            "worktree": str(assigned_path),
            "branch": assigned_branch,
            "build_dir": str(assigned_build),
            "next": [
                f"cd {assigned_path}",
                f"python tools/agent.py context owner {owner}",
            ],
        }
    except Exception:
        subprocess.run(
            ["git", "worktree", "remove", "--force", str(assigned_path)],
            cwd=root,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        subprocess.run(
            ["git", "branch", "-D", assigned_branch],
            cwd=root,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        raise


def close_worktree(
    root: Path,
    *,
    owner: str,
    force: bool = False,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    queue = read_queue(queue_path(root, queue_file))
    matches = [
        task for task in queue.get("tasks", []) if task.get("owner") == owner
    ]
    if not matches:
        raise WorktreeError(f"no queue history exists for {owner}")
    task = matches[-1]
    if not force and task.get("status") not in {
        "done",
        "released",
        "cancelled",
    }:
        raise WorktreeError(
            f"{owner} is still {task.get('status')}; release or complete it first"
        )
    path = Path(str(task.get("worktree") or ""))
    branch = str(task.get("branch") or "")
    if path.is_dir():
        args = ["git", "worktree", "remove"]
        if force:
            args.append("--force")
        args.append(str(path))
        _run(root, *args)
    return {
        "owner": owner,
        "worktree": str(path),
        "branch": branch,
        "removed": True,
    }


def add_worktree_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "worktree", help="create/retire isolated task worktrees"
    )
    commands = parser.add_subparsers(dest="worktree_command", required=True)
    create = commands.add_parser("create")
    create.add_argument("owner")
    create.add_argument("--agent", required=True)
    create.add_argument("--base", default=DEFAULT_WORKER_BASE)
    create.add_argument("--branch")
    create.add_argument("--path")
    create.add_argument("--build-dir")
    create.add_argument("--retail")
    create.add_argument("--target")
    create.add_argument("--source")
    create.add_argument("--shared", action="append", default=[])
    create.add_argument("--change-class")
    create.add_argument("--queue-file")
    close = commands.add_parser("close")
    close.add_argument("owner")
    close.add_argument("--force", action="store_true")
    close.add_argument("--queue-file")
    return parser


def run_worktree_command(
    args: argparse.Namespace,
    *,
    root: Path,
    owners: Sequence[Mapping[str, Any]] | None = None,
) -> int:
    import json

    if args.worktree_command == "create":
        value = create_worktree(
            root,
            agent=args.agent,
            owner=args.owner,
            base=args.base,
            branch=args.branch,
            path=args.path,
            build_dir=args.build_dir,
            retail=args.retail,
            target=args.target,
            source=args.source,
            shared_files=args.shared,
            change_class=args.change_class,
            queue_file=args.queue_file,
            owners=owners,
        )
    else:
        value = close_worktree(
            root,
            owner=args.owner,
            force=args.force,
            queue_file=args.queue_file,
        )
    print(json.dumps(value, indent=2))
    return 0
