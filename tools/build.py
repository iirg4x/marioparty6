#!/usr/bin/env python3
"""Run Ninja and refresh the committed progress badge data."""

import subprocess
import sys
from pathlib import Path

from update_progress import ProgressError, update_from_build


def main() -> int:
    command = ["ninja", *sys.argv[1:]]
    result = subprocess.run(command, check=False)
    if result.returncode != 0:
        return result.returncode

    progress_path = Path("build/GP6E01/progress.json")
    if not progress_path.is_file():
        print(
            "Build completed, but build/GP6E01/progress.json was not generated. "
            "Run the default target or `ninja progress` to refresh README badges."
        )
        return 0

    try:
        changed = update_from_build(progress_path, Path("progress"), "GP6E01")
    except ProgressError as exc:
        print(f"error: {exc}")
        return 2

    if changed:
        print("Updated README progress data:")
        for path in changed:
            print(f"  {path}")
    else:
        print("README progress data is already up to date")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
