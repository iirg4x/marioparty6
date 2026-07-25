#!/usr/bin/env python3
"""Validate, build, or query the deterministic recovery index."""

from __future__ import annotations

import argparse
import json

from tools.recovery_core import (
    RecoveryError,
    build_index,
    load,
    query_index,
    root_from,
    validate_data,
)


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
        errors = validate_data(data)
        if errors:
            print("recovery metadata invalid:")
            for error in errors:
                print(f"- {error}")
            return 1
        if args.command == "check":
            print(f"recovery metadata OK: {len(data['owners'])} owners")
            return 0
        if args.command == "build":
            output = root / args.output
            counts = build_index(data, output)
            summary = ", ".join(
                f"{key}={value}" for key, value in sorted(counts.items())
            )
            print(f"built {output.relative_to(root)}: {summary}")
            return 0
        database = root / args.database
        if not database.is_file():
            build_index(data, database)
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
