#!/usr/bin/env python3
"""Unified entry point for recovery context, queueing, worktrees, and checks."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.agent_queue import (
    QueueError,
    add_queue_parser,
    check_diff_claim,
    queue_health,
    read_queue,
    queue_path,
    run_queue_command,
)
from tools.context_engine import (
    build_context,
    context_token_estimate,
    render_compact_knowledge,
    select_context_knowledge,
)
from tools.hooks import HookError, add_hooks_parser, hook_status, run_hooks_command
from tools.integration_finalize import add_integration_parser, run_integration_command
from tools.knowledge_freshness import (
    FreshnessError,
    all_freshness,
    render_freshness_report,
    validate_freshness,
)
from tools.owner_catalog import CatalogError, build_catalog, find_owner, write_catalog
from tools.recovery_core import load, quality_findings, root_from
from tools.recovery_data import RecoveryError, validate_data
from tools.recovery_knowledge import (
    build_recovery_index,
    knowledge_audit,
    recovery_report,
    render_knowledge_audit,
    resolve_context_target,
    validate_knowledge,
)
from tools.worktree_manager import (
    WorktreeError,
    add_worktree_parser,
    run_worktree_command,
)

MIN_PYTHON = (3, 10)
DEFAULT_FORBIDDEN = [".github.example", ".vscode.example", "README.example.md"]
REQUIRED_AGENT_FILES = [
    "AGENTS.md",
    "CONTRIBUTING.md",
    "docs/agent_quickstart.md",
    "docs/recovery_standard.md",
    "docs/concurrent_agents.md",
    "config/recovery/project.json",
    "config/recovery/compiler_patterns.json",
    "config/recovery/knowledge_freshness.json",
    "tools/agent_queue.py",
    "tools/owner_catalog.py",
    "tools/context_engine.py",
    "tools/local_evidence.py",
    "tools/knowledge_freshness.py",
    "tools/integration_finalize.py",
    "tools/worktree_manager.py",
    "tools/hooks.py",
    "tools/knowledge_cards.py",
    ".github/PULL_REQUEST_TEMPLATE.md",
]
GENERATED_PATHS = ["build", "build.ninja", "objdiff.json", "ctx.c"]


@dataclass(frozen=True)
class Check:
    name: str
    status: str
    detail: str


def _run(
    command: Sequence[str], *, root: Path, capture: bool = True
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command), cwd=root, text=True, capture_output=capture, check=False
    )


def _git(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return _run(["git", *args], root=root)


def _project_list(data: dict[str, Any], key: str, fallback: list[str]) -> list[str]:
    readiness = data["project"].get("agent_readiness", {})
    if not isinstance(readiness, dict):
        return fallback
    value = readiness.get(key, fallback)
    if not isinstance(value, list) or not all(isinstance(item, str) for item in value):
        return fallback
    return value


def _metadata_errors(data: dict[str, Any]) -> list[str]:
    return sorted(
        set(
            [
                *validate_data(data),
                *validate_knowledge(data),
                *validate_freshness(data),
            ]
        )
    )


def _catalog(data: dict[str, Any]) -> dict[str, Any]:
    return build_catalog(data["root"], reviewed=data.get("owners", []))


def doctor_checks(data: dict[str, Any]) -> list[Check]:
    root: Path = data["root"]
    checks: list[Check] = []
    python_ok = sys.version_info >= MIN_PYTHON
    checks.append(
        Check(
            "python",
            "pass" if python_ok else "fail",
            f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro} "
            f"(requires >= {MIN_PYTHON[0]}.{MIN_PYTHON[1]})",
        )
    )

    errors = _metadata_errors(data)
    checks.append(
        Check(
            "recovery metadata",
            "pass" if not errors else "fail",
            (
                f"{len(data['owners'])} reviewed owners, "
                f"{len(data['patterns'])} knowledge cards"
            )
            if not errors
            else "; ".join(errors[:5]),
        )
    )
    try:
        catalog = _catalog(data)
        checks.append(
            Check(
                "owner catalog",
                "pass",
                f"{len(catalog['owners'])} operational owners",
            )
        )
    except CatalogError as exc:
        checks.append(Check("owner catalog", "fail", str(exc)))

    required = _project_list(data, "required_files", REQUIRED_AGENT_FILES)
    missing = [path for path in required if not (root / path).is_file()]
    checks.append(
        Check(
            "agent entrypoints",
            "pass" if not missing else "fail",
            "all required files present"
            if not missing
            else "missing: " + ", ".join(missing),
        )
    )
    forbidden = _project_list(data, "forbidden_paths", DEFAULT_FORBIDDEN)
    present = [path for path in forbidden if (root / path).exists()]
    checks.append(
        Check(
            "template cleanup",
            "pass" if not present else "fail",
            "no inactive template workspaces"
            if not present
            else "remove: " + ", ".join(present),
        )
    )

    git_path = shutil.which("git")
    checks.append(Check("git", "pass" if git_path else "fail", git_path or "not found"))
    if git_path:
        branch = _git(root, "branch", "--show-current")
        branch_name = branch.stdout.strip() if branch.returncode == 0 else "unknown"
        checks.append(
            Check(
                "branch isolation",
                "warn" if branch_name in {"main", "master", ""} else "pass",
                branch_name or "detached HEAD",
            )
        )
        status = _git(root, "status", "--porcelain")
        dirty = [line for line in status.stdout.splitlines() if line.strip()]
        checks.append(
            Check(
                "working tree",
                "pass" if not dirty else "warn",
                "clean" if not dirty else f"{len(dirty)} changed paths",
            )
        )
        tracked: list[str] = []
        for path in GENERATED_PATHS:
            result = _git(root, "ls-files", "--", path)
            tracked.extend(line for line in result.stdout.splitlines() if line)
        checks.append(
            Check(
                "generated files",
                "pass" if not tracked else "fail",
                "generated outputs are untracked"
                if not tracked
                else "tracked: " + ", ".join(sorted(set(tracked))[:8]),
            )
        )
        queue_status, queue_detail = queue_health(root)
        checks.append(Check("claim queue", queue_status, queue_detail))
        hook_values = hook_status(root)
        managed = sum(value == "managed" for value in hook_values.values())
        checks.append(
            Check(
                "local hooks",
                "pass" if managed == len(hook_values) else "warn",
                f"{managed}/{len(hook_values)} managed; run `python tools/agent.py hooks install`",
            )
        )

    ninja_path = shutil.which("ninja")
    checks.append(
        Check(
            "ninja",
            "pass" if ninja_path else "warn",
            ninja_path or "not found; public metadata tools still work",
        )
    )
    main_dol = root / "orig/GP6E01/sys/main.dol"
    checks.append(
        Check(
            "private retail inputs",
            "pass" if main_dol.is_file() else "warn",
            "GP6E01 main.dol present"
            if main_dol.is_file()
            else "not present; retail build and checksum gates are unavailable",
        )
    )
    freshness = all_freshness(data)
    stale = [
        card
        for card, value in freshness.items()
        if value.get("effective_status") != "active"
    ]
    checks.append(
        Check(
            "knowledge freshness",
            "pass" if not stale else "warn",
            "all cards active" if not stale else "review: " + ", ".join(stale[:5]),
        )
    )
    return checks


def _print_checks(checks: list[Check], *, as_json: bool = False) -> None:
    if as_json:
        print(json.dumps([asdict(item) for item in checks], indent=2))
        return
    width = max(len(item.name) for item in checks)
    for item in checks:
        print(f"[{item.status.upper():4}] {item.name:<{width}}  {item.detail}")


def _safe_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_") or "context"


def _write_context(data: dict[str, Any], args: argparse.Namespace) -> Path | None:
    text = build_context(
        data,
        args.kind,
        args.target,
        owner_id=args.owner,
        budget=args.budget,
        knowledge_limit=args.knowledge_limit,
        symptoms=args.symptom,
        local_evidence=args.local_evidence,
        reports=args.report,
    )
    if args.stdout:
        print(text, end="")
        return None
    root: Path = data["root"]
    destination = root / (
        args.output
        or f"build/context/{args.kind}-{_safe_name(args.owner or '')}-{_safe_name(args.target)}.md"
    )
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(text, encoding="utf-8")
    print(
        f"wrote {destination.relative_to(root)} "
        f"({context_token_estimate(text)} estimated tokens)"
    )
    return destination


def _changed_forbidden(root: Path, base: str) -> list[str]:
    process = _git(root, "diff", "--name-only", f"{base}...HEAD")
    if process.returncode:
        raise RecoveryError(process.stderr.strip() or f"git diff failed for {base}")
    disallowed = ("orig/", "build/")
    return [
        path
        for path in process.stdout.splitlines()
        if path in {"build.ninja", "objdiff.json", "ctx.c"}
        or path.startswith(disallowed)
    ]


def public_check(data: dict[str, Any], *, base: str | None) -> int:
    root: Path = data["root"]
    failed = False
    checks = doctor_checks(data)
    _print_checks(checks)
    if any(item.status == "fail" for item in checks):
        failed = True

    for command in (
        [sys.executable, "-m", "compileall", "-q", "tools"],
        [sys.executable, "-m", "unittest", "discover", "-s", "tools/tests", "-v"],
    ):
        print(f"\n$ {' '.join(command)}")
        if _run(command, root=root, capture=False).returncode:
            failed = True

    errors = _metadata_errors(data)
    if errors:
        for error in errors:
            print(f"metadata: {error}")
        failed = True
    else:
        catalog = _catalog(data)
        catalog_path = root / "build/context/owner-catalog.json"
        write_catalog(catalog, catalog_path)
        print(f"catalog: {len(catalog['owners'])} owners")
        database = root / "build/context/recovery.sqlite"
        counts = build_recovery_index(data, database)
        print(
            "index: "
            + ", ".join(
                f"{key}={value}" for key, value in sorted(counts.items())
            )
        )
        report = root / "build/context/recovery-report.md"
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(
            recovery_report(data).rstrip()
            + "\n\n"
            + render_freshness_report(data)
            + "\n",
            encoding="utf-8",
        )
        model = next(
            (
                item
                for item in data["owners"]
                if "model-owner" in item.get("tags", [])
            ),
            None,
        )
        if model:
            smoke = root / "build/context/agent-smoke-context.md"
            smoke.write_text(
                build_context(data, "owner", str(model["id"]), budget=6000),
                encoding="utf-8",
            )

    if base:
        print(f"\nchanged-line review against {base}")
        diff_check = _git(root, "diff", "--check", f"{base}...HEAD")
        if diff_check.returncode:
            print(diff_check.stdout or diff_check.stderr)
            failed = True
        forbidden = _changed_forbidden(root, base)
        if forbidden:
            print("generated/private paths changed: " + ", ".join(forbidden))
            failed = True
        findings = quality_findings(data, base=base)
        for item in findings:
            print(
                f"{item['path']}:{item['line']}: {item['rule']}: "
                f"{item['message']} ({item['classification']})"
            )
        if any(item["classification"] == "unreviewed" for item in findings):
            failed = True
        try:
            queue = read_queue(queue_path(root))
            if queue.get("tasks"):
                claim = check_diff_claim(root, base=base, require_claim=False)
                if claim.get("task") and claim.get("errors"):
                    print("claim diff invalid:\n- " + "\n- ".join(claim["errors"]))
                    failed = True
        except QueueError as exc:
            print(f"queue: {exc}")
            failed = True

    print(
        "\npublic agent gate: "
        + ("FAILED" if failed else "PASS")
        + "\nprivate DOL/REL and retail checksum gates are separate"
    )
    return 1 if failed else 0


def _print_knowledge(data: dict[str, Any], args: argparse.Namespace) -> int:
    if args.kind == "audit":
        audit = knowledge_audit(data)
        print(
            json.dumps(audit, indent=2)
            if args.json
            else render_knowledge_audit(audit, max_items=args.max_items),
            end="" if not args.json else "\n",
        )
        return 0
    if not args.target:
        raise RecoveryError(f"knowledge {args.kind} requires a target")
    owner, stable = resolve_context_target(
        data, args.kind, args.target, args.owner
    )
    matches = select_context_knowledge(
        data,
        owner,
        stable_identity=stable,
        symptoms=args.symptom,
        limit=args.limit,
    )
    if args.json:
        print(json.dumps([match.as_dict() for match in matches], indent=2))
    else:
        print(render_compact_knowledge(data, matches))
    return 0


def _add_catalog_parser(sub: Any) -> None:
    parser = sub.add_parser(
        "catalog", help="build/query operational owner inventory"
    )
    commands = parser.add_subparsers(dest="catalog_command", required=True)
    build = commands.add_parser("build")
    build.add_argument("--output", default="build/context/owner-catalog.json")
    query = commands.add_parser("query")
    query.add_argument("value")
    query.add_argument("--json", action="store_true")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)
    doctor = sub.add_parser("doctor")
    doctor.add_argument("--json", action="store_true")
    doctor.add_argument("--strict", action="store_true")
    add_queue_parser(sub)
    add_worktree_parser(sub)
    add_hooks_parser(sub)
    add_integration_parser(sub)
    _add_catalog_parser(sub)

    context = sub.add_parser("context")
    context.add_argument("kind", choices=["function", "owner"])
    context.add_argument("target")
    context.add_argument("--owner")
    context.add_argument("--budget", type=int, default=12000)
    context.add_argument("--knowledge-limit", type=int)
    context.add_argument("--symptom", action="append", default=[])
    context.add_argument("--local-evidence", action="store_true")
    context.add_argument("--report", action="append", default=[])
    context.add_argument("--output")
    context.add_argument("--stdout", action="store_true")

    knowledge = sub.add_parser("knowledge")
    knowledge.add_argument("kind", choices=["function", "owner", "audit"])
    knowledge.add_argument("target", nargs="?")
    knowledge.add_argument("--owner")
    knowledge.add_argument("--symptom", action="append", default=[])
    knowledge.add_argument("--limit", type=int, default=5)
    knowledge.add_argument("--max-items", type=int, default=20)
    knowledge.add_argument("--json", action="store_true")

    check = sub.add_parser("check")
    check.add_argument("--base")
    report = sub.add_parser("report")
    report.add_argument("--output", default="build/context/recovery-report.md")

    args = parser.parse_args()
    try:
        root = root_from(args.root)
        data = load(root, validate=False)
        catalog = (
            _catalog(data)
            if args.command in {"queue", "worktree", "catalog"}
            else None
        )
        if args.command == "doctor":
            checks = doctor_checks(data)
            _print_checks(checks, as_json=args.json)
            return (
                1
                if any(
                    item.status == "fail"
                    or (args.strict and item.status == "warn")
                    for item in checks
                )
                else 0
            )
        if args.command == "queue":
            return run_queue_command(
                args,
                root=root,
                owners=catalog["owners"] if catalog else [],
            )
        if args.command == "worktree":
            return run_worktree_command(
                args,
                root=root,
                owners=catalog["owners"] if catalog else [],
            )
        if args.command == "hooks":
            return run_hooks_command(args, root=root)
        if args.command == "integration":
            return run_integration_command(args, root=root)
        if args.command == "catalog":
            if args.catalog_command == "build":
                destination = root / args.output
                write_catalog(catalog, destination)
                print(
                    f"wrote {destination.relative_to(root)}: "
                    f"{len(catalog['owners'])} owners"
                )
                return 0
            matches = find_owner(catalog, args.value)
            if args.json:
                print(json.dumps(matches, indent=2))
            else:
                for item in matches:
                    print(
                        f"{item['id']}  {item['configured_status']}  "
                        f"{item['source']}  {item['size_bytes']} bytes"
                    )
            return 0 if matches else 1
        if args.command == "context":
            _write_context(data, args)
            return 0
        if args.command == "knowledge":
            return _print_knowledge(data, args)
        if args.command == "check":
            return public_check(data, base=args.base)
        errors = _metadata_errors(data)
        if errors:
            raise RecoveryError(
                "recovery metadata invalid:\n- " + "\n- ".join(errors)
            )
        destination = root / args.output
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(
            recovery_report(data).rstrip()
            + "\n\n"
            + render_freshness_report(data)
            + "\n",
            encoding="utf-8",
        )
        print(f"wrote {destination.relative_to(root)}")
        return 0
    except (
        CatalogError,
        FreshnessError,
        HookError,
        OSError,
        QueueError,
        RecoveryError,
        WorktreeError,
    ) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
