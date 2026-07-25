#!/usr/bin/env python3
"""Validate, build, or query the deterministic recovery index."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.recovery_core import (
    load,
    query_index,
    root_from,
    validate_data,
)
from tools.recovery_data import RecoveryError
from tools.recovery_knowledge import build_recovery_index, validate_knowledge


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("check")
    build = sub.add_parser("build")
    build.add_argument("--output", default="build/context/recovery.sqlite")
    query = sub.add_parser("query")
    query.add_argument("term")
    query.add_argument("--database", default="build/context/recovery.sqlite")
    query.add_argument("--limit", type=int, default=20)
    query.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        root = root_from(args.root)
        data = load(root, validate=False)
        errors = sorted(set([*validate_data(data), *validate_knowledge(data)]))
        if errors:
            print("recovery metadata invalid:")
            for error in errors:
                print(f"- {error}")
            return 1
        if args.command == "check":
            print(
                f"recovery metadata OK: {len(data['owners'])} owners, "
                f"{len(data['patterns'])} knowledge cards"
            )
            return 0
        if args.command == "build":
            output = root / args.output
            counts = build_recovery_index(data, output)
            summary = ", ".join(
                f"{key}={value}" for key, value in sorted(counts.items())
            )
            print(f"built {output.relative_to(root)}: {summary}")
            return 0
        database = root / args.database
        if not database.is_file():
            build_recovery_index(data, database)
        rows = query_index(database, args.term, args.limit)
        if args.json:
            print(json.dumps(rows, indent=2))
        else:
            for row in rows:
                owner = f" [{row['owner_id']}]" if row.get("owner_id") else ""
                print(f"{row['kind']}: {row['key']}{owner}\n  {row['text']}")
        return 0
    except RecoveryError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
