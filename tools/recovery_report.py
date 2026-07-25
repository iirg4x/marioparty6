#!/usr/bin/env python3
"""Render a human-readable report from recovery metadata."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.recovery_core import load, root_from
from tools.recovery_data import RecoveryError
from tools.recovery_knowledge import recovery_report, validate_knowledge


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root")
    parser.add_argument("--output")
    args = parser.parse_args()
    try:
        root = root_from(args.root)
        data = load(root, validate=False)
        errors = validate_knowledge(data)
        if errors:
            raise RecoveryError("recovery knowledge invalid:\n- " + "\n- ".join(errors))
        text = recovery_report(data)
        if args.output:
            path = root / args.output
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
            print(f"wrote {path.relative_to(root)}")
        else:
            print(text, end="")
        return 0
    except RecoveryError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
