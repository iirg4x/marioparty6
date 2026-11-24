#!/usr/bin/env python3
"""Generate the objdiff progress report that decomp.dev reads.

objdiff-cli compares every unit in objdiff.json and stops if a unit's compiled
object is missing. This script lists missing objects and reports those units
using their retail objects. The original objdiff.json is preserved.

Usage:
    python tools/generate_report.py OBJDIFF_CLI OUTPUT
"""

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path


def report_config(config: dict, root: Path) -> list:
    """Make unit paths absolute and drop compiled objects that do not exist.

    Returns the names of the units whose compiled object is missing."""
    missing = []
    for unit in config.get("units", []):
        for key in ("target_path", "base_path"):
            if unit.get(key):
                unit[key] = str((root / unit[key]).resolve())
        base = unit.get("base_path")
        if base and not Path(base).is_file():
            missing.append(unit["name"])
            del unit["base_path"]
    return missing


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("objdiff", type=Path, help="objdiff-cli executable")
    parser.add_argument("output", type=Path, help="report file to write")
    args = parser.parse_args()

    root = Path.cwd()
    config = json.loads((root / "objdiff.json").read_text(encoding="utf-8"))
    missing = report_config(config, root)
    if missing:
        print(f"{len(missing)} unit(s) have no compiled object and are reported without source:")
        for name in missing:
            print(f"  {name}")

    with tempfile.TemporaryDirectory() as project:
        (Path(project) / "objdiff.json").write_text(json.dumps(config), encoding="utf-8")
        command = [str(args.objdiff), "report", "generate", "-p", project, "-o", str(args.output.resolve())]
        return subprocess.run(command, check=False).returncode


if __name__ == "__main__":
    sys.exit(main())
