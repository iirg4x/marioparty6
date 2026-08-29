"""Shared, side-effect-free contracts for the bounded crack harness."""

from __future__ import annotations

from pathlib import Path
import re
from typing import Any


SAFE_UNIT_SEGMENT_RE = re.compile(r"[A-Za-z0-9_.-]+\Z")


def is_closed_objdiff_unit_name(value: Any) -> bool:
    """Return whether *value* is a closed slash-form objdiff unit name."""

    if not isinstance(value, str):
        return False
    parts = value.split("/")
    return (
        len(parts) >= 2
        and all(
            part not in {".", ".."}
            and SAFE_UNIT_SEGMENT_RE.fullmatch(part) is not None
            for part in parts
        )
        and ".." not in Path(value).parts
    )
