#!/usr/bin/env python3
"""Compact, schema-tolerant summaries of local objdiff/build JSON evidence."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Iterable, Mapping


class EvidenceError(ValueError):
    pass


INTERESTING = {
    "matchpercent",
    "match_percent",
    "functions",
    "functionsdiff",
    "functiondiffs",
    "functionrelocdiffs",
    "relocations",
    "sections",
    "units",
    "symbols",
    "targetsize",
    "basesize",
    "currentsize",
    "stacksize",
    "registers",
    "diffs",
}


def _key(value: str) -> str:
    return "".join(ch for ch in value.lower() if ch.isalnum() or ch == "_")


def _flatten(value: Any, prefix: str = "", depth: int = 0) -> list[tuple[str, Any]]:
    if depth > 6:
        return []
    result: list[tuple[str, Any]] = []
    if isinstance(value, Mapping):
        for key, child in value.items():
            path = f"{prefix}.{key}" if prefix else str(key)
            normal = _key(str(key))
            if normal in INTERESTING and isinstance(child, (str, int, float, bool)):
                result.append((path, child))
            elif normal in INTERESTING and isinstance(child, list):
                result.append((path, f"{len(child)} items"))
            result.extend(_flatten(child, path, depth + 1))
    elif isinstance(value, list):
        for index, child in enumerate(value[:200]):
            result.extend(_flatten(child, f"{prefix}[{index}]", depth + 1))
    return result


def _function_counts(value: Any) -> tuple[int, int] | None:
    candidates: list[Mapping[str, Any]] = []

    def visit(node: Any) -> None:
        if isinstance(node, Mapping):
            lowered = {_key(str(k)): v for k, v in node.items()}
            if any(k in lowered for k in ("matchpercent", "match_percent", "similarity")):
                candidates.append(node)
            for child in node.values():
                visit(child)
        elif isinstance(node, list):
            for child in node:
                visit(child)

    visit(value)
    if not candidates:
        return None
    exact = 0
    total = 0
    for item in candidates:
        lowered = {_key(str(k)): v for k, v in item.items()}
        percent = lowered.get("matchpercent", lowered.get("match_percent", lowered.get("similarity")))
        try:
            number = float(percent)
        except (TypeError, ValueError):
            continue
        total += 1
        if number >= 99.999999:
            exact += 1
    return (exact, total) if total else None


def summarize_report(path: str | Path) -> dict[str, Any]:
    report = Path(path)
    if not report.is_file():
        raise EvidenceError(f"report does not exist: {report}")
    try:
        value = json.loads(report.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise EvidenceError(
            f"invalid JSON {report}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    facts = _flatten(value)
    counts = _function_counts(value)
    return {
        "path": str(report),
        "size_bytes": report.stat().st_size,
        "function_counts": {
            "exact": counts[0],
            "total": counts[1],
        }
        if counts
        else None,
        "facts": [{"path": key, "value": val} for key, val in facts[:30]],
    }


def render_summary(summaries: Iterable[Mapping[str, Any]]) -> str:
    values = list(summaries)
    if not values:
        return "## Local object-diff evidence\n\n- No local reports were requested or available."
    lines = ["## Local object-diff evidence", ""]
    for item in values:
        lines.append(f"### `{item.get('path')}`")
        counts = item.get("function_counts")
        if isinstance(counts, Mapping):
            lines.append(
                f"- Parsed exact functions: `{counts.get('exact')}/{counts.get('total')}`"
            )
        facts = item.get("facts", [])
        if facts:
            for fact in facts[:12]:
                lines.append(f"- `{fact.get('path')}`: `{fact.get('value')}`")
        else:
            lines.append(
                "- No recognized compact fields; inspect the report manually if needed."
            )
    return "\n".join(lines)
