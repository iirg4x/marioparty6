#!/usr/bin/env python3
"""Normalize paths printed by Git for native Python processes.

The locally selected Git may be an MSYS2 executable even when the recovery
tools run under native Windows Python.  In that configuration Git prints paths
such as ``/d/work/repo``; passing that spelling directly to ``pathlib.Path``
silently resolves it as ``D:\\d\\work\\repo`` instead of ``D:\\work\\repo``.
"""

from __future__ import annotations

import os
import re
from pathlib import Path

_MSYS_DRIVE_PATH = re.compile(r"^/([A-Za-z])(?:/(.*))?$")
_MSYS_HOME_PATH = re.compile(r"^/home/([^/]+)(?:/(.*))?$")


def windows_git_path_text(
    value: str,
    *,
    home: str | Path | None = None,
) -> str:
    """Return a native spelling for an MSYS drive path.

    This text-only helper is intentionally platform-independent so the path
    conversion has deterministic unit coverage on non-Windows CI hosts too.
    """

    text = value.strip()
    match = _MSYS_DRIVE_PATH.fullmatch(text)
    if not match:
        home_match = _MSYS_HOME_PATH.fullmatch(text)
        native_home = Path(home).resolve() if home is not None else Path.home()
        if (
            home_match
            and native_home.name.casefold() == home_match.group(1).casefold()
        ):
            tail = home_match.group(2)
            return str(native_home / tail) if tail else str(native_home)
        return text
    drive, tail = match.groups()
    return f"{drive.upper()}:/{tail or ''}"


def native_git_path(
    value: str | Path,
    *,
    relative_to: str | Path | None = None,
) -> Path:
    """Resolve a path emitted by Git for the current native Python process."""

    text = str(value).strip()
    if os.name == "nt":
        text = windows_git_path_text(text)
    path = Path(text)
    if not path.is_absolute() and relative_to is not None:
        path = Path(relative_to) / path
    return path.resolve()
