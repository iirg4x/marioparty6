#!/usr/bin/env python3
"""Finalize ready worker claims after serialized integration verification."""

from __future__ import annotations

import argparse
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence

from tools.agent_queue import QueueError, locked_queue, queue_path


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _run(root: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode:
        raise QueueError(result.stderr.strip() or "git command failed")
    return result.stdout.strip()


def _repo_path(value: str | None) -> str | None:
    if value is None or not value.strip():
        return None
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise QueueError(f"path must be repository-relative: {value}")
    return path.as_posix()


def _claimed_paths(task: Mapping[str, Any]) -> list[str]:
    result = {
        path
        for value in task.get("shared_files", [])
        if (path := _repo_path(str(value)))
    }
    if source := _repo_path(task.get("source")):
        result.add(source)
    return sorted(result)


def _find_ready(queue: Mapping[str, Any], owner: str) -> dict[str, Any]:
    matches = [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict)
        and task.get("owner") == owner
        and task.get("status") == "ready"
    ]
    if len(matches) != 1:
        raise QueueError(f"{owner} must have exactly one ready task")
    return matches[0]


def _worker_proof_errors(task: Mapping[str, Any]) -> list[str]:
    proof = task.get("verification")
    if not isinstance(proof, Mapping):
        return ["structured worker verification is missing"]
    errors: list[str] = []
    for key in ("verified_commit", "public_gate"):
        if not proof.get(key):
            errors.append(f"{key} is missing")
    if proof.get("public_gate") != "pass":
        errors.append("public_gate must be pass")
    if task.get("change_class") in {
        "private-source",
        "shared-interface",
        "build-configuration",
    }:
        for key in ("object_report", "functions_exact", "relocations"):
            if not proof.get(key):
                errors.append(f"{key} is required")
    return errors


def _consumer_pairs(values: Sequence[str] | None) -> dict[str, str]:
    result: dict[str, str] = {}
    for value in values or []:
        key, separator, status = value.partition("=")
        if not separator or not key.strip() or not status.strip():
            raise QueueError("consumer results must use owner=status")
        result[key.strip()] = status.strip()
    return result


def finalize_task(
    root: Path,
    owner: str,
    *,
    agent: str,
    resource: str = "integration",
    retail_gate: str,
    checksum: str,
    consumers: Mapping[str, str] | None = None,
    toolchain: str | None = None,
    note: str | None = None,
    queue_file: str | Path | None = None,
) -> dict[str, Any]:
    if retail_gate != "pass" or checksum != "pass":
        raise QueueError("finalization requires passing retail and checksum gates")
    root = root.resolve()
    path = queue_path(root, queue_file)
    with locked_queue(path) as queue:
        resources = queue.setdefault("resources", {})
        lock = resources.get(resource)
        if not isinstance(lock, Mapping) or lock.get("agent") != agent:
            raise QueueError(
                f"{agent} must hold the {resource} resource before finalization"
            )
        task = _find_ready(queue, owner)
        proof_errors = _worker_proof_errors(task)
        if proof_errors:
            raise QueueError(
                "worker proof is incomplete:\n- " + "\n- ".join(proof_errors)
            )
        if _run(root, "status", "--porcelain"):
            raise QueueError("integration finalization requires a clean worktree")
        integrated_commit = _run(root, "rev-parse", "HEAD")
        verified_commit = str(task["verification"]["verified_commit"])
        for claimed_path in _claimed_paths(task):
            comparison = subprocess.run(
                [
                    "git",
                    "diff",
                    "--quiet",
                    verified_commit,
                    integrated_commit,
                    "--",
                    claimed_path,
                ],
                cwd=root,
                text=True,
                capture_output=True,
                check=False,
            )
            if comparison.returncode == 1:
                raise QueueError(
                    f"integrated tree differs from verified worker content at {claimed_path}"
                )
            if comparison.returncode not in {0, 1}:
                raise QueueError(
                    comparison.stderr.strip()
                    or f"failed to compare integrated path {claimed_path}"
                )
        proof = dict(task["verification"])
        proof.update(
            {
                "retail_gate": retail_gate,
                "checksum": checksum,
                "integration_commit": integrated_commit,
                "integrated_at": _now(),
                "integration_agent": agent,
                "integration_resource": resource,
                "integration_toolchain": toolchain,
            }
        )
        merged_consumers = dict(proof.get("consumers") or {})
        merged_consumers.update(consumers or {})
        proof["consumers"] = merged_consumers
        task["verification"] = proof
        task["status"] = "done"
        task["updated_at"] = task["released_at"] = _now()
        if note is not None:
            task["note"] = note
        return dict(task)


def add_integration_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "integration", help="finalize ready tasks after serialized retail proof"
    )
    commands = parser.add_subparsers(dest="integration_command", required=True)
    finalize = commands.add_parser("finalize")
    finalize.add_argument("owner")
    finalize.add_argument("--agent", required=True)
    finalize.add_argument("--resource", default="integration")
    finalize.add_argument("--retail-gate", choices=["pass", "fail"], required=True)
    finalize.add_argument("--checksum", choices=["pass", "fail"], required=True)
    finalize.add_argument("--consumer", action="append", default=[])
    finalize.add_argument("--toolchain")
    finalize.add_argument("--note")
    finalize.add_argument("--queue-file")
    return parser


def run_integration_command(args: argparse.Namespace, *, root: Path) -> int:
    value = finalize_task(
        root,
        args.owner,
        agent=args.agent,
        resource=args.resource,
        retail_gate=args.retail_gate,
        checksum=args.checksum,
        consumers=_consumer_pairs(args.consumer),
        toolchain=args.toolchain,
        note=args.note,
        queue_file=args.queue_file,
    )
    print(json.dumps(value, indent=2))
    return 0
