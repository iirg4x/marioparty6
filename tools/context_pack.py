#!/usr/bin/env python3
"""Generate a token-bounded evidence pack for one owner or function."""

from __future__ import annotations

import argparse

from tools.recovery_core import (
    RecoveryError,
    context_pack,
    load,
    root_from,
    token_estimate,
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root")
    parser.add_argument("--budget", type=int, default=12000)
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
            load(root),
            args.kind,
            args.target,
            owner_id=getattr(args, "owner", None),
            budget=args.budget,
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
