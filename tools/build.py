#!/usr/bin/env python3
"""Run Ninja and refresh the committed progress data.

After the build, this updates the README badge data in progress/ and the
objdiff progress report in progress/report.json. A workflow uploads that
report on every push to main, and decomp.dev reads it from there.
"""

import subprocess
import sys
from pathlib import Path

from update_progress import ProgressError, update_from_build

VERSION = "GP6E01"
PROGRESS_JSON = Path(f"build/{VERSION}/progress.json")
BUILD_REPORT = Path(f"build/{VERSION}/report.json")
COMMITTED_REPORT = Path("progress/report.json")


def compile_all_sources(resource_options: list[str]) -> None:
    """Compile every source file, including unlinked ones, so the report covers them.

    Work-in-progress sources that do not compile are listed and left out."""
    result = subprocess.run(
        ["ninja", *resource_options, "-k", "0", "all_source"],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    failed = [line.split()[-1] for line in result.stdout.splitlines() if line.startswith("FAILED:")]
    if failed:
        print(f"{len(failed)} unlinked source file(s) do not compile and are reported without source:")
        for path in failed:
            print(f"  {path}")


def refresh_report(resource_options: list[str]) -> int:
    """Build the objdiff report and copy it into progress/ if it changed."""
    compile_all_sources(resource_options)
    BUILD_REPORT.unlink(missing_ok=True)  # always regenerate from the current objects
    result = subprocess.run(["ninja", *resource_options, str(BUILD_REPORT)], check=False)
    if result.returncode != 0:
        print(f"error: could not generate {BUILD_REPORT}")
        return result.returncode

    new_report = BUILD_REPORT.read_bytes()
    if COMMITTED_REPORT.is_file() and COMMITTED_REPORT.read_bytes() == new_report:
        print("decomp.dev progress report is already up to date")
        return 0
    COMMITTED_REPORT.write_bytes(new_report)
    print(f"Updated decomp.dev progress report:\n  {COMMITTED_REPORT}")
    return 0


def resource_options(arguments: list[str]) -> list[str]:
    """Keep the caller's job and load limits for the additional Ninja runs."""
    result = []
    arguments = iter(arguments)
    for argument in arguments:
        if argument in ("-j", "-l"):
            result.extend((argument, next(arguments)))
        elif argument.startswith(("-j", "-l")):
            result.append(argument)
    return result


def main() -> int:
    sys.stdout.reconfigure(line_buffering=True)  # keep messages in order with ninja's output
    command = ["ninja", *sys.argv[1:]]
    result = subprocess.run(command, check=False)
    if result.returncode != 0:
        return result.returncode

    if not PROGRESS_JSON.is_file():
        print(
            f"Build completed, but {PROGRESS_JSON} was not generated. "
            "Run the default target or `ninja progress` to refresh README badges."
        )
        return 0

    try:
        changed = update_from_build(PROGRESS_JSON, Path("progress"), VERSION)
    except ProgressError as exc:
        print(f"error: {exc}")
        return 2

    if changed:
        print("Updated README progress data:")
        for path in changed:
            print(f"  {path}")
    else:
        print("README progress data is already up to date")

    return refresh_report(resource_options(sys.argv[1:]))


if __name__ == "__main__":
    raise SystemExit(main())
