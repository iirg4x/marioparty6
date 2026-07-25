#!/usr/bin/env python3
"""Install lightweight local hooks for draft-phase agent work."""

from __future__ import annotations

import argparse
import stat
import subprocess
from pathlib import Path
from typing import Any

MARKER = "# managed-by-mp6-agent-tools"


class HookError(ValueError):
    pass


def _run(root: Path, *args: str, allow_failure: bool = False) -> str:
    result = subprocess.run(
        args, cwd=root, text=True, capture_output=True, check=False
    )
    if result.returncode and not allow_failure:
        raise HookError(
            result.stderr.strip() or "command failed: " + " ".join(args)
        )
    return result.stdout.strip()


def hooks_dir(root: Path) -> Path:
    configured = _run(
        root,
        "git",
        "config",
        "--get",
        "core.hooksPath",
        allow_failure=True,
    )
    if configured:
        path = Path(configured)
        return (path if path.is_absolute() else root / path).resolve()
    common = Path(_run(root, "git", "rev-parse", "--git-common-dir"))
    common = common if common.is_absolute() else root / common
    return common.resolve() / "hooks"


def _script(kind: str) -> str:
    if kind == "pre-commit":
        body = r'''
ROOT="$(git rev-parse --show-toplevel)"
PYTHON_BIN="${MP6_PYTHON:-python}"
BASE="${MP6_AGENT_BASE:-origin/main}"
cd "$ROOT"
"$PYTHON_BIN" tools/agent.py queue check-diff --base "$BASE"
changed_py="$(git diff --cached --name-only --diff-filter=ACMR | grep -E '\.py$' || true)"
if [ -n "$changed_py" ]; then
  "$PYTHON_BIN" -m py_compile $changed_py
fi
'''
    elif kind == "pre-push":
        body = r'''
ROOT="$(git rev-parse --show-toplevel)"
PYTHON_BIN="${MP6_PYTHON:-python}"
cd "$ROOT"
"$PYTHON_BIN" tools/recovery_index.py check
"$PYTHON_BIN" tools/knowledge_cards.py check
"$PYTHON_BIN" -m unittest discover -s tools/tests -v
'''
    else:
        raise HookError(f"unsupported hook {kind}")
    return f"#!/bin/sh\n{MARKER}\nset -eu\n{body.strip()}\n"


def install_hooks(root: Path, *, force: bool = False) -> list[Path]:
    directory = hooks_dir(root)
    directory.mkdir(parents=True, exist_ok=True)
    installed: list[Path] = []
    for name in ("pre-commit", "pre-push"):
        path = directory / name
        if (
            path.exists()
            and MARKER
            not in path.read_text(encoding="utf-8", errors="replace")
            and not force
        ):
            raise HookError(
                f"refusing to replace unmanaged hook {path}; use --force"
            )
        path.write_text(_script(name), encoding="utf-8", newline="\n")
        path.chmod(
            path.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH
        )
        installed.append(path)
    return installed


def uninstall_hooks(root: Path) -> list[Path]:
    removed: list[Path] = []
    for name in ("pre-commit", "pre-push"):
        path = hooks_dir(root) / name
        if path.is_file() and MARKER in path.read_text(
            encoding="utf-8", errors="replace"
        ):
            path.unlink()
            removed.append(path)
    return removed


def hook_status(root: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for name in ("pre-commit", "pre-push"):
        path = hooks_dir(root) / name
        if not path.exists():
            result[name] = "missing"
        elif MARKER in path.read_text(encoding="utf-8", errors="replace"):
            result[name] = "managed"
        else:
            result[name] = "unmanaged"
    return result


def add_hooks_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "hooks", help="install/status local agent hooks"
    )
    commands = parser.add_subparsers(dest="hooks_command", required=True)
    install = commands.add_parser("install")
    install.add_argument("--force", action="store_true")
    commands.add_parser("status")
    commands.add_parser("uninstall")
    return parser


def run_hooks_command(args: argparse.Namespace, *, root: Path) -> int:
    if args.hooks_command == "install":
        paths = install_hooks(root, force=args.force)
        for path in paths:
            print(f"installed {path}")
    elif args.hooks_command == "uninstall":
        paths = uninstall_hooks(root)
        for path in paths:
            print(f"removed {path}")
    else:
        for name, status in hook_status(root).items():
            print(f"{name}: {status}")
    return 0
