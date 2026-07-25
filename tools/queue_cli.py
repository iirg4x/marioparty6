#!/usr/bin/env python3
"""CLI adapter for the advanced recovery queue runtime."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Mapping, Sequence

from tools import agent_queue as legacy
from tools.queue_runtime import (
    CHANGE_CLASSES,
    QueueError,
    acquire_resource,
    add_task,
    check_diff_claim,
    claim_next,
    claim_task,
    consumer_pairs,
    record_verification,
    release_resource,
    release_task,
    update_task,
)


def add_queue_parser(subparsers: Any) -> argparse.ArgumentParser:
    queue = subparsers.add_parser(
        "queue", help="coordinate Claude/Codex worktrees"
    )
    queue.add_argument(
        "--queue-file",
        help="shared JSON path; defaults to Git common dir or $MP6_AGENT_QUEUE",
    )
    actions = queue.add_subparsers(dest="queue_command", required=True)
    status = actions.add_parser("status")
    status.add_argument("--all", action="store_true")
    status.add_argument("--json", action="store_true")
    status.add_argument("--stale-hours", type=float, default=legacy.STALE_HOURS)
    actions.add_parser("check").add_argument("--json", action="store_true")
    diff = actions.add_parser("check-diff")
    diff.add_argument("--base")
    diff.add_argument("--agent")
    diff.add_argument("--json", action="store_true")

    add = actions.add_parser("add")
    add.add_argument("owner")
    add.add_argument("--target")
    add.add_argument("--source")
    add.add_argument(
        "--priority", choices=sorted(legacy.PRIORITY), default="normal"
    )
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
    add.add_argument("--base-ref", default="origin/main")
    add.add_argument("--note")

    claim = actions.add_parser("claim")
    claim.add_argument("owner")
    claim.add_argument("--agent", required=True)
    claim.add_argument("--worktree")
    claim.add_argument("--branch")
    claim.add_argument("--build-dir")
    claim.add_argument("--target")
    claim.add_argument("--source")
    claim.add_argument(
        "--priority", choices=sorted(legacy.PRIORITY), default=None
    )
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
    update.add_argument("--status", choices=sorted(legacy.ACTIVE))
    update.add_argument("--add-shared", action="append", default=[])
    update.add_argument("--remove-shared", action="append", default=[])
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
    release.add_argument(
        "--status", choices=sorted(legacy.TERMINAL), default="done"
    )
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
    catalog: Sequence[Mapping[str, Any]] | None = None,
) -> int:
    path = legacy.queue_path(root, args.queue_file)
    if args.queue_command in {"status", "check"}:
        queue = legacy.read_queue(path)
        errors = legacy.validate_queue(queue)
        if args.queue_command == "check":
            payload = {
                "path": str(path),
                "errors": errors,
                "active": len(legacy.active_tasks(queue)),
                "resources": len(queue.get("resources", {})),
            }
            if args.json:
                print(json.dumps(payload, indent=2))
            elif errors:
                print("queue invalid:\n- " + "\n- ".join(errors))
            else:
                print(
                    f"queue OK: {payload['active']} active claim(s), "
                    f"{payload['resources']} resource lock(s) at {path}"
                )
            return 1 if errors else 0
        if errors:
            raise QueueError("queue invalid:\n- " + "\n- ".join(errors))
        if args.json:
            print(json.dumps({"path": str(path), "queue": queue}, indent=2))
        else:
            print(
                legacy.render_status(
                    path,
                    queue,
                    include_terminal=args.all,
                    stale_hours=args.stale_hours,
                )
            )
            resources = queue.get("resources", {})
            if resources:
                print("Resources:")
                for name, record in sorted(resources.items()):
                    print(
                        f"  {name}: {record.get('agent')} "
                        f"owner={record.get('owner') or '-'}"
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
            catalog=catalog,
            target=args.target,
            source=args.source,
            priority=args.priority,
            shared_files=args.shared,
            depends_on=args.depends_on,
            batch=args.batch,
            capabilities=args.capability,
            change_class=args.change_class,
            estimated_cost=args.estimated_cost,
            verification_cost=args.verification_cost,
            base_ref=args.base_ref,
            note=args.note,
            queue_file=args.queue_file,
        )
    elif args.queue_command == "claim":
        task = claim_task(
            root,
            args.owner,
            agent=args.agent,
            catalog=catalog,
            worktree=args.worktree,
            branch=args.branch,
            build_dir=args.build_dir,
            target=args.target,
            source=args.source,
            priority=args.priority,
            shared_files=args.shared,
            capabilities=args.capability,
            change_class=args.change_class,
            base_ref=args.base_ref,
            note=args.note,
            queue_file=args.queue_file,
        )
    elif args.queue_command == "claim-next":
        task = claim_next(
            root,
            agent=args.agent,
            catalog=catalog,
            capabilities=args.capability,
            batch=args.batch,
            queue_file=args.queue_file,
        )
    elif args.queue_command == "update":
        task = update_task(
            root,
            args.owner,
            catalog=catalog,
            status=args.status,
            add_shared=args.add_shared,
            remove_shared=args.remove_shared,
            note=args.note,
            agent=args.agent,
            queue_file=args.queue_file,
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
            consumers=consumer_pairs(args.consumer),
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
