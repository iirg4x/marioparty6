#!/usr/bin/env python3
"""Validate and evaluate freshness metadata for reusable recovery cards."""

from __future__ import annotations

import json
import re
import subprocess
from pathlib import Path
from typing import Any, Mapping

COMMIT_RE = re.compile(r"^[0-9a-fA-F]{40}$")
STATUSES = {"active", "stale", "superseded", "experimental"}


class FreshnessError(ValueError):
    pass


def freshness_path(data: Mapping[str, Any]) -> Path:
    root = Path(data["root"])
    files = data.get("project", {}).get("files", {})
    relative = (
        files.get(
            "knowledge_freshness",
            "config/recovery/knowledge_freshness.json",
        )
        if isinstance(files, Mapping)
        else "config/recovery/knowledge_freshness.json"
    )
    return root / str(relative)


def load_freshness(data: Mapping[str, Any]) -> dict[str, Any]:
    path = freshness_path(data)
    if not path.is_file():
        return {"schema_version": 1, "cards": {}}
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise FreshnessError(
            f"invalid freshness JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    if not isinstance(value, dict):
        raise FreshnessError(f"{path}: expected object")
    return value


def validate_freshness(data: Mapping[str, Any]) -> list[str]:
    value = load_freshness(data)
    errors: list[str] = []
    if value.get("schema_version") != 1:
        errors.append("knowledge_freshness.schema_version must be 1")
    cards = value.get("cards")
    if not isinstance(cards, Mapping):
        return [*errors, "knowledge_freshness.cards must be an object"]
    known = {str(card.get("id")) for card in data.get("patterns", [])}
    missing = known - set(cards)
    unknown = set(cards) - known
    if missing:
        errors.append(
            "knowledge cards missing freshness records: "
            + ", ".join(sorted(missing))
        )
    if unknown:
        errors.append(
            "freshness records reference unknown cards: "
            + ", ".join(sorted(unknown))
        )
    for card_id, record in cards.items():
        where = f"knowledge_freshness.cards.{card_id}"
        if not isinstance(record, Mapping):
            errors.append(f"{where} must be an object")
            continue
        if record.get("status") not in STATUSES:
            errors.append(f"{where}.status is invalid")
        commit = record.get("validated_commit")
        if not isinstance(commit, str) or not COMMIT_RE.fullmatch(commit):
            errors.append(f"{where}.validated_commit must be a 40-hex SHA")
        if not isinstance(record.get("validated_at"), str) or not record.get(
            "validated_at"
        ):
            errors.append(f"{where}.validated_at is required")
        for key in ("watch_paths", "supersedes"):
            values = record.get(key, [])
            if not isinstance(values, list) or not all(
                isinstance(item, str) and item for item in values
            ):
                errors.append(f"{where}.{key} must be a string list")
        superseded_by = record.get("superseded_by")
        if superseded_by is not None and superseded_by not in known:
            errors.append(f"{where}.superseded_by references unknown card")
    return sorted(set(errors))


def _git(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args], cwd=root, text=True, capture_output=True, check=False
    )


def _compiler_patterns_path(data: Mapping[str, Any]) -> str:
    files = data.get("project", {}).get("files", {})
    if isinstance(files, Mapping):
        return str(
            files.get(
                "compiler_patterns",
                "config/recovery/compiler_patterns.json",
            )
        )
    return "config/recovery/compiler_patterns.json"


def _card_payload(value: Mapping[str, Any], card_id: str) -> dict[str, Any] | None:
    cards = value.get("patterns", [])
    if not isinstance(cards, list):
        return None
    card = next(
        (
            item
            for item in cards
            if isinstance(item, Mapping) and str(item.get("id")) == card_id
        ),
        None,
    )
    if card is None:
        return None
    return {
        "schema_version": value.get("schema_version"),
        "card": dict(card),
    }


def _card_payload_at_commit(
    root: Path,
    commit: str,
    relative: str,
    card_id: str,
) -> dict[str, Any] | None:
    result = _git(root, "show", f"{commit}:{relative}")
    if result.returncode:
        return None
    try:
        value = json.loads(result.stdout)
    except json.JSONDecodeError:
        return None
    return _card_payload(value, card_id) if isinstance(value, Mapping) else None


def _current_card_payload(
    root: Path,
    relative: str,
    card_id: str,
) -> dict[str, Any] | None:
    try:
        value = json.loads((root / relative).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    return _card_payload(value, card_id) if isinstance(value, Mapping) else None


def card_freshness(data: Mapping[str, Any], card_id: str) -> dict[str, Any]:
    value = load_freshness(data)
    record = dict(value.get("cards", {}).get(card_id, {}))
    if not record:
        return {
            "status": "unknown",
            "effective_status": "unknown",
            "reason": "no freshness record",
        }
    root = Path(data["root"])
    configured = record.get("status")
    if configured in {"superseded", "stale"}:
        return {
            **record,
            "effective_status": configured,
            "reason": "configured status",
        }
    commit = str(record.get("validated_commit", ""))
    exists = _git(root, "cat-file", "-t", commit)
    if exists.returncode or exists.stdout.strip() != "commit":
        return {
            **record,
            "effective_status": "stale",
            "reason": "validated commit unavailable",
        }
    watch = list(record.get("watch_paths", []))
    patterns_path = _compiler_patterns_path(data)
    if patterns_path in watch:
        validated_card = _card_payload_at_commit(
            root,
            commit,
            patterns_path,
            card_id,
        )
        current_card = _current_card_payload(root, patterns_path, card_id)
        if validated_card is not None and validated_card == current_card:
            watch.remove(patterns_path)
    if watch:
        diff = _git(
            root,
            "diff",
            "--name-only",
            commit,
            "--",
            *watch,
        )
        if diff.returncode:
            return {
                **record,
                "effective_status": "stale",
                "reason": diff.stderr.strip() or "freshness diff failed",
            }
        changed = [line for line in diff.stdout.splitlines() if line.strip()]
        if changed:
            return {
                **record,
                "effective_status": "stale",
                "reason": "watched evidence/source changed",
                "changed_paths": changed,
            }
    return {
        **record,
        "effective_status": "active",
        "reason": "validated inputs unchanged",
    }


def all_freshness(data: Mapping[str, Any]) -> dict[str, dict[str, Any]]:
    return {
        str(card.get("id")): card_freshness(data, str(card.get("id")))
        for card in data.get("patterns", [])
    }


def render_freshness_report(data: Mapping[str, Any]) -> str:
    records = all_freshness(data)
    lines = ["## Knowledge freshness", ""]
    if not records:
        return "\n".join([*lines, "- No knowledge cards."])
    for card, record in sorted(records.items()):
        status = record.get("effective_status", record.get("status", "unknown"))
        line = f"- `{status}` `{card}` — {record.get('reason', 'unknown')}"
        changed = record.get("changed_paths", [])
        if changed:
            line += "; changed: " + ", ".join(f"`{path}`" for path in changed[:5])
        lines.append(line)
    return "\n".join(lines)
