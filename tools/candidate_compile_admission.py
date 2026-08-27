#!/usr/bin/env python3
"""Mandatory central lookup/record front door for Board candidate compiles.

Prefer ``tools/agent.py match lookup`` when using a match workbench.  This
standalone front door covers bounded compiler scripts that do not use one.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Any, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.agent_queue import QueueError
from tools.recovery_memory import (
    RecoveryMemoryError,
    run_memory_command,
)


def _common(parser: Any) -> None:
    parser.add_argument("--owner", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--base-commit", required=True)
    parser.add_argument("--toolchain-key", required=True)
    parser.add_argument("--target-sha256", required=True)
    parser.add_argument("--compiler-sha256")
    parser.add_argument("--context-key")
    parser.add_argument("--source-sha256", required=True)
    parser.add_argument("--shape-key")
    parser.add_argument("--hypothesis")
    parser.add_argument("--axis")
    parser.add_argument("--requester")
    parser.add_argument("--source-path")
    parser.add_argument("--json", action="store_true")


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--root", default=".")
    commands = result.add_subparsers(dest="memory_command", required=True)
    admit = commands.add_parser("admit")
    _common(admit)
    record = commands.add_parser("record")
    _common(record)
    record.add_argument("--object-sha256", required=True)
    record.add_argument("--status", required=True)
    record.add_argument("--reason", required=True)
    record.add_argument("--admission-token")
    record.add_argument("--candidate-id")
    record.add_argument("--candidate-record-sha256")
    record.add_argument("--strict-report-sha256")
    record.add_argument("--data-report-sha256")
    record.add_argument("--report-sha256")
    record.add_argument("--workspace")
    return result


def main(argv: Sequence[str] | None = None) -> int:
    selected = parser()
    args = selected.parse_args(argv)
    try:
        return run_memory_command(args, root=Path(args.root))
    except (OSError, QueueError, RecoveryMemoryError) as exc:
        selected.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
