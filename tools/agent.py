#!/usr/bin/env python3
"""Agent-facing entry point for setup, queue, context, knowledge, and checks."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.agent_queue import (
    QueueError,
    add_queue_parser,
    queue_health,
    run_queue_command,
)
from tools.recovery_core import load, quality_findings, root_from
from tools.recovery_data import RecoveryError, token_estimate, validate_data
from tools.recovery_knowledge import (
    build_recovery_index,
    context_pack,
    knowledge_audit,
    recovery_report,
    render_knowledge_audit,
    render_knowledge_cards,
    resolve_context_target,
    select_knowledge_cards,
    validate_knowledge,
)

MIN_PYTHON = (3, 10)
DEFAULT_FORBIDDEN = [
    ".github.example",
    ".vscode.example",
    "README.example.md",
]
REQUIRED_AGENT_FILES = [
    "AGENTS.md",
    "CONTRIBUTING.md",
    "docs/agent_quickstart.md",
    "docs/recovery_standard.md",
    "docs/concurrent_agents.md",
    "config/recovery/project.json",
    "config/recovery/compiler_patterns.json",
    "tools/agent_queue.py",
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
    command: Sequence[str],
    *,
    root: Path,
    capture: bool = True,
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command),
        cwd=root,
        text=True,
        capture_output=capture,
        check=False,
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
    return sorted(set([*validate_data(data), *validate_knowledge(data)]))


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
                f"{len(data['owners'])} governed owners, "
                f"{len(data['patterns'])} knowledge cards"
            )
            if not errors
            else "; ".join(errors[:5]),
        )
    )

    required = _project_list(data, "required_files", REQUIRED_AGENT_FILES)
    missing_required = [path for path in required if not (root / path).is_file()]
    checks.append(
        Check(
            "agent entrypoints",
            "pass" if not missing_required else "fail",
            "all required files present"
            if not missing_required
            else "missing: " + ", ".join(missing_required),
        )
    )

    forbidden = _project_list(data, "forbidden_paths", DEFAULT_FORBIDDEN)
    present_forbidden = [path for path in forbidden if (root / path).exists()]
    checks.append(
        Check(
            "template cleanup",
            "pass" if not present_forbidden else "fail",
            "no inactive template workspaces"
            if not present_forbidden
            else "remove: " + ", ".join(present_forbidden),
        )
    )

    git_path = shutil.which("git")
    checks.append(Check("git", "pass" if git_path else "fail", git_path or "not found"))
    if git_path:
        branch = _git(root, "branch", "--show-current")
        branch_name = branch.stdout.strip() if branch.returncode == 0 else "unknown"
        if branch_name == "main":
            checks.append(
                Check(
                    "branch isolation",
                    "warn",
                    "currently on main; create a task branch before editing",
                )
            )
        elif branch_name:
            checks.append(Check("branch isolation", "pass", branch_name))
        else:
            checks.append(
                Check(
                    "branch isolation",
                    "warn",
                    "detached HEAD or branch unavailable",
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

        tracked_generated: list[str] = []
        for path in GENERATED_PATHS:
            result = _git(root, "ls-files", "--", path)
            tracked_generated.extend(
                line for line in result.stdout.splitlines() if line
            )
        checks.append(
            Check(
                "generated files",
                "pass" if not tracked_generated else "fail",
                "generated outputs are untracked"
                if not tracked_generated
                else "tracked: " + ", ".join(sorted(set(tracked_generated))[:8]),
            )
        )

        queue_status, queue_detail = queue_health(root)
        checks.append(Check("claim queue", queue_status, queue_detail))

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
    return checks


def _print_checks(checks: list[Check], *, as_json: bool = False) -> None:
    if as_json:
        print(json.dumps([asdict(item) for item in checks], indent=2))
        return
    width = max(len(item.name) for item in checks)
    for item in checks:
        print(f"[{item.status.upper():4}] {item.name:<{width}}  {item.detail}")


def _safe_name(value: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_")
    return cleaned or "context"


def _write_context(
    data: dict[str, Any],
    *,
    kind: str,
    target: str,
    owner: str | None,
    budget: int,
    knowledge_limit: int | None,
    output: str | None,
    stdout: bool,
) -> Path | None:
    text = context_pack(
        data,
        kind,
        target,
        owner_id=owner,
        budget=budget,
        knowledge_limit=knowledge_limit,
    )
    if stdout:
        print(text, end="")
        return None
    root: Path = data["root"]
    destination = root / (
        output
        or f"build/context/{kind}-{_safe_name(owner or '')}-{_safe_name(target)}.md"
    )
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(text, encoding="utf-8")
    print(
        f"wrote {destination.relative_to(root)} "
        f"({token_estimate(text)} estimated tokens)"
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
    blocking = [item for item in checks if item.status == "fail"]
    _print_checks(checks)
    if blocking:
        failed = True

    commands = [
        [sys.executable, "-m", "compileall", "-q", "tools"],
        [sys.executable, "-m", "unittest", "discover", "-s", "tools/tests", "-v"],
    ]
    for command in commands:
        print(f"\n$ {' '.join(command)}")
        process = _run(command, root=root, capture=False)
        if process.returncode:
            failed = True

    errors = _metadata_errors(data)
    if errors:
        for error in errors:
            print(f"metadata: {error}")
        failed = True
    else:
        database = root / "build/context/recovery.sqlite"
        counts = build_recovery_index(data, database)
        print(
            "\nindex: "
            + ", ".join(
                f"{key}={value}" for key, value in sorted(counts.items())
            )
        )

        report = root / "build/context/recovery-report.md"
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(recovery_report(data), encoding="utf-8")
        print(f"report: {report.relative_to(root)}")

        model_owner = next(
            (
                item
                for item in data["owners"]
                if "model-owner" in item.get("tags", [])
            ),
            data["owners"][0] if data["owners"] else None,
        )
        if model_owner:
            smoke = root / "build/context/agent-smoke-context.md"
            smoke.write_text(
                context_pack(
                    data,
                    "owner",
                    str(model_owner["id"]),
                    budget=6000,
                ),
                encoding="utf-8",
            )
            print(f"context smoke: {smoke.relative_to(root)}")

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
            suffix = f" ({item['classification']})"
            print(
                f"{item['path']}:{item['line']}: {item['rule']}: "
                f"{item['message']}{suffix}"
            )
        if any(item["classification"] == "unreviewed" for item in findings):
            failed = True
        elif not findings:
            print("source review: no findings")

    print(
        "\npublic agent gate: "
        + ("FAILED" if failed else "PASS")
        + "\nprivate DOL/REL and retail checksum gates are separate"
    )
    return 1 if failed else 0


def _print_knowledge(
    data: dict[str, Any],
    *,
    kind: str,
    target: str | None,
    owner_id: str | None,
    limit: int,
    as_json: bool,
    max_items: int,
) -> int:
    if kind == "audit":
        audit = knowledge_audit(data)
        if as_json:
            print(json.dumps(audit, indent=2))
        else:
            print(render_knowledge_audit(audit, max_items=max_items), end="")
        return 0
    if not target:
        raise RecoveryError(f"knowledge {kind} requires a target")
    owner, stable_identity = resolve_context_target(
        data,
        kind,
        target,
        owner_id,
    )
    matches = select_knowledge_cards(
        data,
        owner,
        stable_identity=stable_identity,
        limit=limit,
    )
    if as_json:
        print(json.dumps([item.as_dict() for item in matches], indent=2))
    else:
        print(render_knowledge_cards(matches))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Unified agent workspace entry point."
    )
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)

    doctor = sub.add_parser("doctor", help="inspect agent workspace readiness")
    doctor.add_argument("--json", action="store_true")
    doctor.add_argument(
        "--strict",
        action="store_true",
        help="treat warnings as failures",
    )

    add_queue_parser(sub)

    context = sub.add_parser("context", help="generate bounded recovery context")
    context.add_argument("kind", choices=["function", "owner"])
    context.add_argument("target")
    context.add_argument("--owner")
    context.add_argument("--budget", type=int, default=12000)
    context.add_argument(
        "--knowledge-limit",
        type=int,
        help="maximum automatically selected knowledge cards; 0 disables",
    )
    context.add_argument("--output")
    context.add_argument("--stdout", action="store_true")

    knowledge = sub.add_parser(
        "knowledge",
        help="inspect applicable rules or the wave-distillation backlog",
    )
    knowledge.add_argument("kind", choices=["function", "owner", "audit"])
    knowledge.add_argument("target", nargs="?")
    knowledge.add_argument("--owner")
    knowledge.add_argument("--limit", type=int, default=5)
    knowledge.add_argument("--max-items", type=int, default=20)
    knowledge.add_argument("--json", action="store_true")

    check = sub.add_parser("check", help="run the public-safe agent branch gate")
    check.add_argument("--base", help="base ref or SHA for changed-line checks")

    report = sub.add_parser("report", help="generate the recovery knowledge report")
    report.add_argument(
        "--output",
        default="build/context/recovery-report.md",
    )

    args = parser.parse_args()
    try:
        root = root_from(args.root)
        data = load(root, validate=False)
        if args.command == "doctor":
            checks = doctor_checks(data)
            _print_checks(checks, as_json=args.json)
            blocking = [
                item
                for item in checks
                if item.status == "fail"
                or (args.strict and item.status == "warn")
            ]
            return 1 if blocking else 0
        if args.command == "queue":
            return run_queue_command(
                args,
                root=root,
                owners=data.get("owners", []),
            )
        if args.command == "context":
            _write_context(
                data,
                kind=args.kind,
                target=args.target,
                owner=args.owner,
                budget=args.budget,
                knowledge_limit=args.knowledge_limit,
                output=args.output,
                stdout=args.stdout,
            )
            return 0
        if args.command == "knowledge":
            return _print_knowledge(
                data,
                kind=args.kind,
                target=args.target,
                owner_id=args.owner,
                limit=args.limit,
                as_json=args.json,
                max_items=args.max_items,
            )
        if args.command == "check":
            return public_check(data, base=args.base)
        errors = _metadata_errors(data)
        if errors:
            raise RecoveryError(
                "recovery metadata invalid:\n- " + "\n- ".join(errors)
            )
        destination = root / args.output
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(recovery_report(data), encoding="utf-8")
        print(f"wrote {destination.relative_to(root)}")
        return 0
    except (OSError, QueueError, RecoveryError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
