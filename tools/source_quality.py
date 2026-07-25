#!/usr/bin/env python3
"""Review changed C/C++ lines for unusual constructs requiring evidence."""

from __future__ import annotations

import argparse
import json

from tools.recovery_core import RecoveryError, load, quality_findings, root_from


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--changed", metavar="BASE")
    mode.add_argument("--all", action="store_true")
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--reject-temporary", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        data = load(root_from(args.root))
        findings = quality_findings(
            data,
            base=args.changed,
            full=args.all or not args.changed,
        )
        if args.json:
            print(json.dumps(findings, indent=2))
        elif findings:
            for item in findings:
                exception = (
                    f", {item['exception']}" if item.get("exception") else ""
                )
                print(
                    f"{item['path']}:{item['line']}: {item['rule']}: "
                    f"{item['message']} ({item['classification']}{exception})"
                )
        else:
            print("source review: no findings")
        rejected = [
            item
            for item in findings
            if item["classification"] == "unreviewed"
            or (
                args.reject_temporary
                and item["classification"] == "temporary"
            )
        ]
        return 1 if args.strict and rejected else 0
    except RecoveryError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
