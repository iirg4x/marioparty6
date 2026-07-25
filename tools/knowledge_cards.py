#!/usr/bin/env python3
"""Inspect, validate, select, and audit reusable recovery knowledge cards."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.context_engine import render_compact_knowledge, select_context_knowledge
from tools.knowledge_freshness import all_freshness, validate_freshness
from tools.recovery_core import load, root_from
from tools.recovery_data import RecoveryError
from tools.recovery_knowledge import (
    knowledge_audit,
    render_knowledge_audit,
    resolve_context_target,
    validate_knowledge,
)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Query structured source-to-output recovery knowledge."
    )
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("check", help="validate card schema, references, and freshness")

    audit = sub.add_parser("audit", help="audit wave-to-card distillation coverage")
    audit.add_argument("--json", action="store_true")
    audit.add_argument("--max-items", type=int, default=20)

    freshness = sub.add_parser("freshness", help="show active/stale card inputs")
    freshness.add_argument("--json", action="store_true")

    for kind in ("function", "owner"):
        command = sub.add_parser(kind, help=f"select cards for one {kind}")
        command.add_argument("target")
        if kind == "function":
            command.add_argument("--owner")
        command.add_argument("--symptom", action="append", default=[])
        command.add_argument("--limit", type=int, default=5)
        command.add_argument("--json", action="store_true")

    args = parser.parse_args()
    try:
        data = load(root_from(args.root), validate=False)
        errors = sorted(set([*validate_knowledge(data), *validate_freshness(data)]))
        if errors:
            print("recovery knowledge invalid:")
            for error in errors:
                print(f"- {error}")
            return 1
        if args.command == "check":
            print(
                f"recovery knowledge OK: {len(data['patterns'])} cards, "
                "freshness records complete"
            )
            return 0
        if args.command == "audit":
            value = knowledge_audit(data)
            if args.json:
                print(json.dumps(value, indent=2))
            else:
                print(
                    render_knowledge_audit(value, max_items=args.max_items),
                    end="",
                )
            return 0
        if args.command == "freshness":
            value = all_freshness(data)
            if args.json:
                print(json.dumps(value, indent=2))
            else:
                for card, record in sorted(value.items()):
                    print(
                        f"{record.get('effective_status', record.get('status')):10} "
                        f"{card}: {record.get('reason')}"
                    )
            return 0

        owner, stable_identity = resolve_context_target(
            data,
            args.command,
            args.target,
            getattr(args, "owner", None),
        )
        matches = select_context_knowledge(
            data,
            owner,
            stable_identity=stable_identity,
            symptoms=args.symptom,
            limit=args.limit,
        )
        if args.json:
            print(json.dumps([item.as_dict() for item in matches], indent=2))
        else:
            print(render_compact_knowledge(data, matches))
        return 0
    except (OSError, RecoveryError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
