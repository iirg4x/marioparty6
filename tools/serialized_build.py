#!/usr/bin/env python3
"""Run a strictly serial Ninja build under the cross-orchestrator lock."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys
from typing import Sequence

from tools.recovery_pass import (
    BUILD_LOCK_ENV,
    DEFAULT_BUILD_LOCK,
    serialized_build_lock,
)


def run(
    root: Path,
    targets: Sequence[str],
    *,
    lock_path: Path,
    timeout_seconds: float = 55.0,
    ninja: str = "ninja",
) -> int:
    if not targets:
        raise ValueError("at least one Ninja target is required")
    root = root.resolve()
    if not (root / "build.ninja").is_file():
        raise ValueError(f"build.ninja does not exist under {root}")
    command = [ninja, "-j1", *targets]
    with serialized_build_lock(lock_path.expanduser().resolve(), timeout_seconds):
        return subprocess.run(command, cwd=root, check=False).returncode


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("targets", nargs="+")
    parser.add_argument("--root", default=".")
    parser.add_argument(
        "--build-lock",
        default=os.environ.get(BUILD_LOCK_ENV, str(DEFAULT_BUILD_LOCK)),
    )
    parser.add_argument("--build-lock-timeout", type=float, default=55.0)
    parser.add_argument("--ninja", default="ninja")
    args = parser.parse_args(argv)
    try:
        return run(
            Path(args.root),
            args.targets,
            lock_path=Path(args.build_lock),
            timeout_seconds=args.build_lock_timeout,
            ninja=args.ninja,
        )
    except (OSError, TimeoutError, ValueError) as exc:
        print(f"serialized build: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
