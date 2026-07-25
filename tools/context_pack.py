#!/usr/bin/env python3
"""Generate a token-bounded evidence pack for one owner or function."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.recovery_core import load, root_from
from tools.recovery_data import RecoveryError, token_estimate
from tools.recovery_knowledge import context_pack


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root")
    parser.add_argument("--budget", type=int, default=12000)
    parser.add_argument(
        "--knowledge-limit",
        type=int,
        help="maximum automatically selected knowledge cards; 0 disables",
    )
    parser.add_argument("--output")
    sub = parser.add_subparsers(dest="kind", required=True)
    function = sub.add_parser("function")
    function.add_argument("target")
    function.add_argument("--owner")
    owner = sub.add_parser("owner")
    owner.add_argument("target")
    args = parser.parse_args()
    try:
        root = root_from(args.root)
        text = context_pack(
            load(root, validate=False),
            args.kind,
            args.target,
            owner_id=getattr(args, "owner", None),
            budget=args.budget,
            knowledge_limit=args.knowledge_limit,
        )
        if args.output:
            path = root / args.output
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
            print(
                f"wrote {path.relative_to(root)} "
                f"({token_estimate(text)} estimated tokens)"
            )
        else:
            print(text, end="")
        return 0
    except RecoveryError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
