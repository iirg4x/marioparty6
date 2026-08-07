#!/usr/bin/env python3
"""Require public progress updates for recovery pushes to main."""

from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path
from typing import Iterable


STATUS_PATH = "STATUS.md"
PROGRESS_PATHS = {
    "progress/GP6E01.json",
    "progress/all.json",
    "progress/dlls.json",
    "progress/dol.json",
}
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
OBJECT_RE = re.compile(
    r'Object\(\s*(Matching|NonMatching),\s*"([^"]+)"', re.MULTILINE
)


class ProgressGateError(ValueError):
    pass


def _git(root: Path, *args: str, allow_failure: bool = False) -> str:
    result = subprocess.run(
        ("git", *args),
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode and not allow_failure:
        raise ProgressGateError(
            result.stderr.strip() or "git command failed: " + " ".join(args)
        )
    return result.stdout


def _object_statuses(root: Path, ref: str) -> dict[str, str]:
    text = _git(root, "show", f"{ref}:configure.py", allow_failure=True)
    return {path: status for status, path in OBJECT_RE.findall(text)}


def progress_errors(
    changed_paths: Iterable[str], *, matching_status_changed: bool
) -> list[str]:
    paths = {path.replace("\\", "/") for path in changed_paths}
    source_changed = any(
        path.startswith("src/") and Path(path).suffix.lower() in SOURCE_SUFFIXES
        for path in paths
    )
    errors: list[str] = []
    if (source_changed or matching_status_changed) and STATUS_PATH not in paths:
        errors.append(
            "recovery source or Matching status changed, but STATUS.md was not updated"
        )
    if matching_status_changed:
        missing = sorted(PROGRESS_PATHS - paths)
        if missing:
            errors.append(
                "Matching status changed, but generated progress files were not "
                "all refreshed: " + ", ".join(missing)
            )
    return errors


def check_range(root: Path, base: str, head: str) -> list[str]:
    if not head or set(head) == {"0"}:
        return []
    if not base or set(base) == {"0"}:
        raise ProgressGateError("cannot validate a new main branch without a base")
    changed = _git(
        root,
        "diff",
        "--name-only",
        "--diff-filter=ACMRD",
        f"{base}..{head}",
    ).splitlines()
    matching_changed = _object_statuses(root, base) != _object_statuses(root, head)
    return progress_errors(changed, matching_status_changed=matching_changed)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--base", required=True)
    parser.add_argument("--head", required=True)
    args = parser.parse_args()
    errors = check_range(args.root.resolve(), args.base, args.head)
    if errors:
        print("public progress gate failed:")
        for error in errors:
            print(f"- {error}")
        print("Run the verified build, refresh progress/**, and reconcile STATUS.md.")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
