#!/usr/bin/env python3
"""Symptom-aware, section-budgeted recovery context with local evidence."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence

from tools.agent_queue import QueueError, queue_path, read_queue
from tools.knowledge_freshness import card_freshness
from tools.local_evidence import EvidenceError, render_summary, summarize_report
from tools.owner_catalog import CatalogError, build_catalog
from tools.recovery_core import context_pack as base_context_pack
from tools.recovery_data import parse_functions, stable_id, token_estimate
from tools.recovery_knowledge import (
    KnowledgeMatch,
    resolve_context_target,
    select_knowledge_cards,
)
from tools.recovery_memory import (
    RecoveryMemory,
    RecoveryMemoryError,
    recovery_memory_available,
    render_context_memory,
)

DEFAULT_SECTION_WEIGHTS = {
    "Recovery contract": 7,
    "Owner state": 7,
    "Current target": 14,
    "Durable rejected-probe/blocker history": 8,
    "Central recovery memory": 16,
    "Relevant recovered knowledge": 18,
    "Broader recovery diagnostics": 8,
    "Operational dependency context": 12,
    "Local object-diff evidence": 10,
    "Target function": 34,
    "Owner functions": 30,
    "Bounded owner neighbourhood": 8,
    "Accepted evidence": 7,
    "Rejected evidence and probes": 5,
    "Authenticated constraints": 5,
    "Naming ledger": 4,
    "Remaining recovery debt": 5,
    "Local reports": 3,
    "Acceptance criteria": 7,
}
MANDATORY = {
    "Recovery contract",
    "Owner state",
    "Current target",
    "Durable rejected-probe/blocker history",
    "Central recovery memory",
    "Relevant recovered knowledge",
    "Authenticated constraints",
    "Acceptance criteria",
}


def _terms(values: Sequence[str] | None) -> set[str]:
    result: set[str] = set()
    for value in values or []:
        result.update(re.findall(r"[a-z0-9_]+", value.lower()))
    return {term for term in result if len(term) > 2}


def _card_text(card: Mapping[str, Any]) -> str:
    source = card.get("source_condition", {})
    emitted = card.get("emitted_effect", {})
    values: list[Any] = [
        card.get("title"),
        card.get("category"),
        card.get("summary"),
        card.get("conditions"),
        card.get("rule"),
        source.get("change") if isinstance(source, Mapping) else None,
    ]
    if isinstance(emitted, Mapping):
        values += list(emitted.get("possible_changes", []))
        values += list(emitted.get("known_signatures", []))
    return " ".join(str(value) for value in values if value)


def select_context_knowledge(
    data: dict[str, Any],
    owner: dict[str, Any],
    *,
    stable_identity: str | None,
    symptoms: Sequence[str] | None,
    limit: int | None,
) -> list[KnowledgeMatch]:
    configured = data.get("project", {}).get("knowledge_card_limit", 5)
    desired = limit if limit is not None else int(configured or 5)
    candidates = select_knowledge_cards(
        data,
        owner,
        stable_identity=stable_identity,
        limit=max(30, desired * 4),
    )
    symptom_terms = _terms(symptoms)
    if symptom_terms:
        selected: list[KnowledgeMatch] = []
        for match in candidates:
            if match.counterexample or match.relevance in {
                "exact target",
                "confirmed example",
                "owner-specific",
                "confirmed owner example",
                "module-related",
                "owner-tag related",
            }:
                selected.append(match)
                continue
            card_terms = _terms([_card_text(match.card)])
            if symptom_terms & card_terms:
                selected.append(match)
        candidates = selected
    return candidates[:desired]


def render_compact_knowledge(
    data: dict[str, Any], matches: Sequence[KnowledgeMatch],
    *, heading: str = "Relevant recovered knowledge",
) -> str:
    lines = [
        f"## {heading}",
        "",
    ]
    if not matches:
        lines.append("- No applicable cards after scope and symptom filtering.")
        return "\n".join(lines)
    for match in matches:
        card = match.card
        freshness = card_freshness(data, str(card.get("id")))
        status = freshness.get("effective_status", freshness.get("status", "unknown"))
        source = card.get("source_condition", {})
        actions = list(card.get("safe_actions", []))[:3]
        counterexamples = list(card.get("counterexamples", []))
        lines += [
            "",
            f"### {card.get('title')} (`{card.get('id')}`)",
            f"- **Relevance:** {match.relevance}; {'; '.join(match.reasons)}",
            f"- **Freshness:** `{status}` — {freshness.get('reason', 'not evaluated')}",
            f"- **Trigger:** {source.get('change') if isinstance(source, Mapping) else ''}",
            f"- **Rule:** {card.get('rule')}",
        ]
        if actions:
            lines.append("- **Safe actions:** " + "; ".join(actions))
        if counterexamples:
            lines.append(
                "- **Counterexamples:** "
                + ", ".join(f"`{item}`" for item in counterexamples)
            )
    return "\n".join(lines)


def _edge_text(edges: Sequence[Mapping[str, Any]], limit: int = 12) -> str:
    values = []
    for edge in edges[:limit]:
        targets = ", ".join(f"`{item}`" for item in edge.get("owners", [])) or "unresolved"
        values.append(f"`{edge.get('symbol')}` → {targets}")
    return "; ".join(values) or "none"


def render_operational_dependencies(
    data: dict[str, Any], owner: Mapping[str, Any]
) -> str:
    lines = [
        "## Operational dependency context",
        "",
        "Generated from source/configuration. Call, import, and data edges are conservative operational approximations, not semantic proof.",
    ]
    try:
        catalog = build_catalog(data["root"], reviewed=data.get("owners", []))
    except (CatalogError, OSError) as exc:
        return "\n".join([*lines, f"- Catalog unavailable: {exc}"])
    source = owner.get("source")
    owner_id = owner.get("id")
    matches = [
        item
        for item in catalog.get("owners", [])
        if source == item.get("source")
        or owner_id in {item.get("id"), item.get("reviewed_owner_id")}
    ]
    if len(matches) != 1:
        return "\n".join([*lines, "- No unique operational owner record."])
    record = matches[0]
    dependencies = record.get("depends_on_owners", [])
    lines += [
        f"- **Owner:** `{record.get('id')}` · `{record.get('source')}` · `{record.get('configured_status')}`",
        "- **Outgoing owner dependencies:** "
        + (", ".join(f"`{item}`" for item in dependencies[:20]) or "none"),
        "- **Direct call edges:** " + _edge_text(record.get("call_edges", [])),
        "- **Global-data edges:** " + _edge_text(record.get("data_edges", [])),
        "- **Declared imports:** " + _edge_text(record.get("import_edges", [])),
    ]
    incoming_functions: list[str] = []
    for symbol in record.get("functions_defined", []):
        consumers = catalog.get("function_consumers", {}).get(symbol, [])
        if consumers:
            incoming_functions.append(
                f"`{symbol}` ← "
                + ", ".join(f"`{item}`" for item in consumers[:8])
            )
    incoming_data: list[str] = []
    for symbol in record.get("globals_defined", []):
        consumers = catalog.get("data_consumers", {}).get(symbol, [])
        if consumers:
            incoming_data.append(
                f"`{symbol}` ← "
                + ", ".join(f"`{item}`" for item in consumers[:8])
            )
    lines.append(
        "- **Incoming function consumers:** "
        + ("; ".join(incoming_functions[:12]) or "none resolved")
    )
    lines.append(
        "- **Incoming data consumers:** "
        + ("; ".join(incoming_data[:12]) or "none resolved")
    )
    return "\n".join(lines)


def _split_sections(text: str) -> tuple[str, list[tuple[str, str]]]:
    matches = list(re.finditer(r"(?m)^## ([^\n]+)\n", text))
    if not matches:
        return text, []
    preamble = text[: matches[0].start()].rstrip()
    sections: list[tuple[str, str]] = []
    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else len(text)
        sections.append((match.group(1).strip(), text[match.start() : end].rstrip()))
    return preamble, sections


def _clip_plain(text: str, limit: int) -> str:
    if len(text) <= limit:
        return text
    if limit < 120:
        return text[:limit]
    return text[: limit - 40].rstrip() + "\n\n[section clipped]\n"


def _clip_target(text: str, limit: int) -> str:
    if len(text) <= limit:
        return text
    marker = "```c\n"
    start = text.find(marker)
    if start < 0:
        return _clip_plain(text, limit)
    end = text.rfind("```")
    metadata = text[: start + len(marker)]
    suffix = "\n[function source clipped]\n```"
    if len(metadata) + len(suffix) >= limit:
        return _clip_plain(text, limit)
    code = text[start + len(marker) : end if end > start else len(text)]
    room = limit - len(metadata) - len(suffix)
    return metadata + code[:room].rstrip() + suffix


HISTORY_SECTION = "Durable rejected-probe/blocker history"
BLOCKED_HISTORY_PATH = Path("build/board-autonomy/blocked.json")
BATCH_HISTORY_PATH = Path("build/board-autonomy/batch-history.json")
HISTORY_LIMIT = 12
TERMINAL_QUEUE_STATUSES = {"done", "released", "cancelled"}
QUEUE_NOTE_STATUSES = {"blocked", "released", "cancelled"}
REJECTED_PROBE_STATUSES = {
    "blocked",
    "failed",
    "no-gain",
    "no_gain",
    "regressed",
    "rejected",
    "rejected-neutral",
    "rejected-regression",
    "reverted",
}


def _owner_aliases(value: Any) -> set[str]:
    """Return a stable owner key for the common queue/history spellings.

    The durable ledgers were written by several generations of tooling.  Keep
    the normalization deliberately narrow: only board owner prefixes and the
    optional task suffix are removed.  Other owner families (REL, game, ...)
    retain their full identity and therefore cannot accidentally collide.
    """

    if not isinstance(value, str):
        return set()
    text = value.strip().replace("\\", "/")
    if not text:
        return set()
    # Queue claims sometimes carry a task label after the owner.  It is not
    # part of the semantic owner identity used by the ledgers.
    text = text.split("#", 1)[0].strip().strip("/")
    if not text:
        return set()
    folded = text.casefold()
    board_owner = False
    if folded.startswith("main:"):
        text = text[5:]
    elif folded.startswith("main/"):
        text = text[5:]
    folded = text.casefold()
    if folded.startswith("src/"):
        text = text[4:]
    folded = text.casefold()
    if folded.startswith("board/"):
        text = text[6:]
        board_owner = True
    if board_owner and ":" in text:
        # Queue claims append a task label as ``board/X:task``.  The colon in
        # ``main:board/X`` was already consumed above; only strip this suffix
        # after the semantic board prefix has been recognized.
        text = text.split(":", 1)[0]
    if text.casefold().endswith(".c"):
        text = text[:-2]
    text = text.strip("/")
    return {text.casefold()} if text else set()


def _symbol_aliases(value: Any) -> set[str]:
    if not isinstance(value, str):
        return set()
    text = value.strip()
    if not text:
        return set()
    values = {text.casefold()}
    # Stable identities are owner:symbol (or module:0xADDR); the final
    # component is the useful target symbol when a ledger stores either form.
    if ":" in text:
        tail = text.rsplit(":", 1)[-1].strip()
        if tail:
            values.add(tail.casefold())
    if "/" in text:
        tail = text.rsplit("/", 1)[-1].strip()
        if tail:
            values.add(tail.casefold())
    return values


def _text_values(value: Any) -> list[str]:
    """Flatten strings from a ledger field without stringifying structures."""

    if isinstance(value, str):
        return [value.strip()] if value.strip() else []
    if isinstance(value, Mapping):
        values: list[str] = []
        for key in ("name", "symbol", "function", "target", "value", "key"):
            if key in value:
                values.extend(_text_values(value[key]))
        return values
    if isinstance(value, Sequence) and not isinstance(value, (str, bytes, bytearray)):
        values: list[str] = []
        for item in value:
            values.extend(_text_values(item))
        return values
    return []


def _compact_history_text(value: Any, limit: int = 360) -> str:
    # Ledger text is untrusted prompt input.  Keep one line, remove control
    # bytes, and neutralize Markdown code delimiters before rendering it.
    text = re.sub(r"[\x00-\x1f\x7f]", " ", str(value or ""))
    text = " ".join(text.split()).replace("`", "'")
    if len(text) <= limit:
        return text
    return text[: limit - 3].rstrip() + "..."


def _looks_like_record(value: Mapping[str, Any]) -> bool:
    return bool(
        set(value)
        & {
            "owner",
            "function",
            "functions",
            "symbol",
            "symbols",
            "target",
            "targets",
            "probe",
            "probe_key",
            "input_key",
            "reason",
            "result",
            "summary",
            "status",
            "action",
            "note",
            "resume_when",
        }
    )


def _iter_record_entries(
    value: Any,
    *,
    fallback_owner: str | None = None,
) -> Iterable[tuple[Mapping[str, Any], str | None, str | None]]:
    """Yield ``(record, owner fallback, key)`` from list or keyed ledgers."""

    if isinstance(value, Mapping):
        if _looks_like_record(value):
            yield value, fallback_owner, None
            return
        for key, item in value.items():
            if isinstance(item, Mapping):
                if _looks_like_record(item):
                    yield item, fallback_owner or str(key), str(key)
                else:
                    yield from _iter_record_entries(
                        item,
                        fallback_owner=fallback_owner or str(key),
                    )
            elif isinstance(item, Sequence) and not isinstance(
                item, (str, bytes, bytearray)
            ):
                yield from _iter_record_entries(
                    item,
                    fallback_owner=fallback_owner or str(key),
                )
        return
    if isinstance(value, Sequence) and not isinstance(value, (str, bytes, bytearray)):
        for item in value:
            if isinstance(item, Mapping):
                yield from _iter_record_entries(
                    item,
                    fallback_owner=fallback_owner,
                )


def _record_field(record: Mapping[str, Any], *names: str) -> Any:
    for name in names:
        if name in record and record[name] not in (None, "", [], {}):
            return record[name]
    return None


def _history_record(
    value: Mapping[str, Any],
    *,
    source: str,
    fallback_owner: str | None = None,
    key: str | None = None,
    force_rejected: bool = False,
) -> dict[str, Any] | None:
    owner_values = _text_values(
        _record_field(value, "owner", "task_owner", "owner_id")
    )
    owner = owner_values[0] if owner_values else fallback_owner
    if owner and "|" in owner:
        owner = owner.split("|", 1)[0].strip()
    if not owner and key and "|" in key:
        owner = key.split("|", 1)[0]
    if not owner:
        return None

    status = _compact_history_text(_record_field(value, "status", "state"), 80)
    action = _compact_history_text(_record_field(value, "action"), 100)
    status_folded = status.casefold().replace(" ", "_")
    action_folded = action.casefold().replace(" ", "_")
    if not force_rejected and not (
        status_folded in REJECTED_PROBE_STATUSES
        or action_folded in REJECTED_PROBE_STATUSES
        or "revert" in action_folded
    ):
        return None

    symbols = _text_values(
        _record_field(
            value,
            "symbol",
            "symbols",
            "function",
            "functions",
            "target",
            "targets",
        )
    )
    # Human summaries such as ``"30/32 strict at isolated checkpoint"`` may
    # use the plural field without naming functions.  Keep only identifier-
    # shaped values so those records remain owner-level blockers rather than
    # being accidentally filtered out of a target-function context.
    symbols = [
        symbol
        for symbol in symbols
        if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_:$./-]*", symbol)
    ]
    if not symbols and key and "|" in key:
        parts = key.split("|")
        if len(parts) > 1 and parts[1].strip():
            symbols = [parts[1].strip()]
    probe = _text_values(
        _record_field(value, "probe_key", "probe", "experiment", "probe_name")
    )
    input_key = _text_values(_record_field(value, "input_key", "input"))
    profile = _text_values(_record_field(value, "profile", "compiler_profile"))
    toolchain = _text_values(
        _record_field(value, "toolchain_key", "toolchain", "compiler")
    )
    target_sha = _text_values(_record_field(value, "target_sha256", "target_hash"))
    candidate_sha = _text_values(
        _record_field(value, "candidate_sha256", "candidate_hash")
    )
    reason = _text_values(
        _record_field(
            value,
            "reason",
            "result",
            "summary",
            "message",
            "outcome",
            "resume_when",
            "note",
        )
    )
    if not reason and action:
        reason = [action]
    if not reason:
        reason = ["rejected or blocked; no durable reason recorded"]
    artifact = _text_values(_record_field(value, "report", "reference", "source"))
    return {
        "owner": str(owner).strip(),
        "symbols": list(dict.fromkeys(symbols)),
        "probe": probe[0] if probe else "",
        "input_key": input_key[0] if input_key else "",
        "profile": _compact_history_text(profile[0], 100) if profile else "",
        "toolchain": _compact_history_text(toolchain[0], 100) if toolchain else "",
        "target_sha256": _compact_history_text(target_sha[0], 100) if target_sha else "",
        "candidate_sha256": _compact_history_text(candidate_sha[0], 100)
        if candidate_sha
        else "",
        "reason": _compact_history_text(reason[0]),
        "status": status or action,
        "source": source,
        "artifact": _compact_history_text(artifact[0], 160) if artifact else "",
    }


def _safe_json_object(path: Path) -> Mapping[str, Any] | None:
    try:
        if not path.is_file():
            return None
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, ValueError, TypeError):
        return None
    return value if isinstance(value, Mapping) else None


def _blocked_history_records(path: Path) -> list[dict[str, Any]]:
    value = _safe_json_object(path)
    if value is None:
        return []
    records: list[dict[str, Any]] = []
    for field in (
        "blocked",
        "exhausted_recent_probes",
        "owner_checkpoint",
        "missing_definition_deadlock",
    ):
        container = value.get(field)
        for item, fallback_owner, key in _iter_record_entries(container):
            record = _history_record(
                item,
                source=f"blocked.json:{field}",
                fallback_owner=fallback_owner,
                key=key,
                force_rejected=True,
            )
            if record is not None:
                records.append(record)
    return records


def _batch_history_records(path: Path) -> list[dict[str, Any]]:
    value = _safe_json_object(path)
    if value is None:
        return []
    records: list[dict[str, Any]] = []
    # Both schema 1 and schema 2 keep probe records at the top level.  A
    # mapping keyed by owner|symbol|probe and a plain list are both accepted.
    for item, fallback_owner, key in _iter_record_entries(value.get("probes")):
        record = _history_record(
            item,
            source="batch-history.json:probes",
            fallback_owner=fallback_owner,
            key=key,
        )
        if record is not None:
            records.append(record)
    batches = value.get("batches")
    if isinstance(batches, Mapping):
        batch_values: Iterable[Any] = batches.values()
    elif isinstance(batches, Sequence) and not isinstance(
        batches, (str, bytes, bytearray)
    ):
        batch_values = batches
    else:
        batch_values = ()
    for batch in batch_values:
        if not isinstance(batch, Mapping):
            continue
        fallback_owner = _text_values(
            _record_field(batch, "owner", "task_owner", "owner_id")
        )
        owner = fallback_owner[0] if fallback_owner else None
        batch_id = _text_values(_record_field(batch, "id", "batch_id"))
        source = "batch-history.json:batches"
        if batch_id:
            source += f"/{_compact_history_text(batch_id[0], 80)}"
        for item, item_owner, key in _iter_record_entries(
            batch.get("rejected"), fallback_owner=owner
        ):
            record = _history_record(
                item,
                source=source + ":rejected",
                fallback_owner=item_owner or owner,
                key=key,
                force_rejected=True,
            )
            if record is not None:
                records.append(record)
    return records


def _queue_history_records(root: Path) -> list[dict[str, Any]]:
    try:
        queue = read_queue(queue_path(root))
    except QueueError:
        return []
    except OSError:
        return []
    except (UnicodeError, ValueError, TypeError):
        return []
    except Exception:
        return []
    if not isinstance(queue, Mapping):
        return []
    records: list[dict[str, Any]] = []

    def semantic_owner(task: Mapping[str, Any]) -> str | None:
        source = task.get("source")
        source_norm = source.replace("\\", "/").casefold() if isinstance(source, str) else ""
        if isinstance(source, str) and source_norm.startswith(("src/board/", "main/board/", "main:board/", "board/")) and _owner_aliases(source):
            return source
        target = task.get("target")
        if isinstance(target, str) and owner_shaped_target(target) and _owner_aliases(target):
            return target
        owner = task.get("owner")
        return owner if isinstance(owner, str) else None

    def owner_shaped_target(value: Any) -> bool:
        if not isinstance(value, str):
            return False
        normalized = value.replace("\\", "/").casefold()
        return normalized.startswith(("main/board/", "main:board/", "board/", "src/board/")) or normalized.endswith(".o")

    tasks = queue.get("tasks")
    if not isinstance(tasks, Sequence) or isinstance(tasks, (str, bytes, bytearray)):
        return records
    for task in tasks:
        if not isinstance(task, Mapping):
            continue
        status_value = task.get("status")
        if not isinstance(status_value, str) or status_value not in QUEUE_NOTE_STATUSES:
            continue
        note = _text_values(task.get("note"))
        if not note:
            continue
        status = status_value.casefold()
        note_text = " ".join(note[0].casefold().split())
        negative_terms = (
            "reject",
            "revert",
            "regress",
            "no gain",
            "no-gain",
            "no_gain",
            "no exact gain",
            "neutral",
            "exhaust",
            "fail",
            "block",
        )
        if not any(term in note_text for term in negative_terms):
            continue
        queue_record = dict(task)
        queue_record["owner"] = semantic_owner(task)
        # Queue targets often name the owner object (for example
        # ``main/board/mgcall``), not a function.  Such a note is an
        # owner-level negative checkpoint and should survive target filtering.
        if not any(
            isinstance(task.get(key), str) and task.get(key).strip()
            for key in ("function", "symbol", "symbols", "functions")
        ) and owner_shaped_target(task.get("target")):
            queue_record.pop("target", None)
        elif not any(
            isinstance(task.get(key), str) and task.get(key).strip()
            for key in ("function", "symbol", "symbols", "functions")
        ) and isinstance(task.get("target"), str):
            # Queue claims commonly pack several function targets into one
            # ``+``/comma-separated string.  Split those before the generic
            # identifier filter; otherwise the whole string is discarded and
            # the note incorrectly becomes owner-wide.
            targets = [
                item.strip()
                for item in re.split(r"[+,;|]", str(task["target"]))
                if item.strip()
            ]
            if targets:
                queue_record["symbols"] = targets
                queue_record.pop("target", None)
        record = _history_record(
            {**queue_record, "reason": note[0]},
            source="queue:terminal-note",
            force_rejected=True,
        )
        if record is not None:
            records.append(record)
    return records


def _history_matches(
    record: Mapping[str, Any],
    *,
    owner_keys: set[str],
    target_keys: set[str],
) -> bool:
    if not owner_keys or not (_owner_aliases(record.get("owner")) & owner_keys):
        return False
    symbols = record.get("symbols")
    record_keys: set[str] = set()
    symbol_values = symbols if isinstance(symbols, Sequence) else []
    for symbol in symbol_values:
        record_keys.update(_symbol_aliases(symbol))
    # Owner-level checkpoints/deadlocks have no symbol and apply to every
    # function in that owner.  Symbol-bearing records must match the target.
    return not target_keys or not record_keys or bool(record_keys & target_keys)


def _history_dedupe_key(record: Mapping[str, Any]) -> tuple[str, ...]:
    owner = next(iter(_owner_aliases(record.get("owner"))), "")
    symbols = record.get("symbols")
    symbol_values = symbols if isinstance(symbols, Sequence) else []
    symbol_key = ",".join(
        sorted(
            alias
            for symbol in symbol_values
            for alias in _symbol_aliases(symbol)
        )
    )
    return (
        owner,
        symbol_key,
        str(record.get("probe", "")).casefold(),
        str(record.get("input_key", "")).casefold(),
        str(record.get("reason", "")).casefold(),
    )


def collect_rejected_probe_history(
    root: Path,
    owner: Mapping[str, Any],
    *,
    target_symbols: Sequence[str] | None = None,
    limit: int = HISTORY_LIMIT,
) -> list[dict[str, Any]]:
    """Collect best-effort durable no-go evidence for one owner/target."""

    root = Path(root)
    owner_keys: set[str] = set()
    for value in (owner.get("id"), owner.get("source")):
        owner_keys.update(_owner_aliases(value))
    target_keys = {
        alias
        for symbol in target_symbols or []
        for alias in _symbol_aliases(symbol)
    }
    requested_limit = max(0, min(limit, HISTORY_LIMIT))
    if requested_limit == 0:
        return []
    records: list[dict[str, Any]] = []
    records.extend(_blocked_history_records(root / BLOCKED_HISTORY_PATH))
    records.extend(_batch_history_records(root / BATCH_HISTORY_PATH))
    records.extend(_queue_history_records(root))
    selected: list[dict[str, Any]] = []
    seen: set[tuple[str, ...]] = set()
    for record in records:
        if not _history_matches(
            record, owner_keys=owner_keys, target_keys=target_keys
        ):
            continue
        identity = _history_dedupe_key(record)
        if identity in seen:
            continue
        seen.add(identity)
        selected.append(record)
        if len(selected) >= requested_limit:
            break
    return selected


def render_rejected_probe_history(
    root: Path,
    owner: Mapping[str, Any],
    *,
    target_symbols: Sequence[str] | None = None,
    limit: int = HISTORY_LIMIT,
) -> str:
    records = collect_rejected_probe_history(
        root,
        owner,
        target_symbols=target_symbols,
        limit=limit,
    )
    lines = [
        f"## {HISTORY_SECTION}",
        "",
    ]
    if records:
        lines.append(
            "- **DO NOT REPEAT:** Do not repeat a listed probe while its "
            "input/evidence key is unchanged; run `rtk python tools/agent.py probe "
            "lookup` before compiling; reopen only with a concretely changed "
            "`input_key` from target/source/toolchain/provenance."
        )
    else:
        lines.append(
            "- **DO NOT REPEAT:** No matching durable no-go record was found; "
            "run `rtk python tools/agent.py probe lookup` before compiling and "
            "require a concretely changed `input_key` from "
            "target/source/toolchain/provenance before trying a probe."
        )
    for record in records:
        owner_text = _compact_history_text(record.get("owner"), 100)
        symbols = record.get("symbols")
        symbol_values = symbols if isinstance(symbols, Sequence) else []
        symbol_text = ", ".join(
            f"`{_compact_history_text(symbol, 100)}`"
            for symbol in symbol_values
            if isinstance(symbol, str)
        )
        label = f"`{owner_text}`"
        if symbol_text:
            label += f" / {symbol_text}"
        details: list[str] = []
        if record.get("probe"):
            details.append(f"probe=`{_compact_history_text(record['probe'], 100)}`")
        if record.get("input_key"):
            details.append(
                f"input_key=`{_compact_history_text(record['input_key'], 120)}`"
            )
        for key in ("profile", "toolchain", "target_sha256", "candidate_sha256"):
            if record.get(key):
                details.append(
                    f"{key}=`{_compact_history_text(record[key], 120)}`"
                )
        if record.get("status"):
            details.append(f"status=`{_compact_history_text(record['status'], 60)}`")
        if record.get("source"):
            details.append(f"source=`{_compact_history_text(record['source'], 120)}`")
        if record.get("artifact"):
            details.append(
                f"evidence=`{_compact_history_text(record['artifact'], 120)}`"
            )
        suffix = f" ({'; '.join(details)})" if details else ""
        lines.append(
            f"- {label}: {_compact_history_text(record.get('reason'))}{suffix}"
        )
    return "\n".join(lines)


def _context_target_symbols(
    data: Mapping[str, Any],
    owner: Mapping[str, Any],
    kind: str,
    target: str,
    stable_identity: str | None,
) -> list[str]:
    if kind != "function":
        return []
    values = [target]
    if stable_identity:
        values.append(stable_identity)
        for item in owner.get("symbols", []):
            if isinstance(item, Mapping) and item.get("stable_id") == stable_identity:
                values.append(str(item.get("symbol", "")))
        # Explicit symbol maps are not present for every owner.  Derive the
        # symbol from the stable id against the current source when possible.
        try:
            source = Path(data["root"]) / str(owner["source"])
            source_text = source.read_text(encoding="utf-8")
            for function in parse_functions(source_text):
                if stable_id(dict(owner), function.symbol) == stable_identity:
                    values.append(function.symbol)
        except (OSError, UnicodeError, TypeError, ValueError):
            pass
    return list(dict.fromkeys(value for value in values if isinstance(value, str) and value.strip()))


def _first_local_mismatch(path: Path, symbols: Sequence[str]) -> str | None:
    """Read one bounded report, selecting the requested symbol before any row.

    A local report is evidence, not proof that the current source produced it.
    Keep that distinction in the capsule rather than inferring freshness from
    a report filename or a percentage.
    """
    if not symbols:
        return None
    try:
        if path.stat().st_size > 32 * 1024 * 1024:
            return None
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, ValueError):
        return None
    if not isinstance(value, Mapping):
        return None

    pairs: list[tuple[str, list[Any], list[Any]]] = []
    left, right = value.get("left"), value.get("right")
    if isinstance(left, Mapping) and isinstance(right, Mapping):
        left_symbols, right_symbols = left.get("symbols"), right.get("symbols")
        if isinstance(left_symbols, list) and isinstance(right_symbols, list):
            candidates = {
                item.get("name"): item for item in right_symbols
                if isinstance(item, Mapping) and isinstance(item.get("name"), str)
            }
            for item in left_symbols:
                if not isinstance(item, Mapping):
                    continue
                name = item.get("name")
                if not isinstance(name, str) or (symbols and name not in symbols):
                    continue
                rows = item.get("instructions")
                other = candidates.get(name, {}).get("instructions", [])
                if isinstance(rows, list) and isinstance(other, list):
                    pairs.append((name, rows, other))
    if value.get("schema") == "focus_symbol_report/v1":
        name = value.get("function")
        channels = value.get("channels")
        strict = channels.get("strict") if isinstance(channels, Mapping) else None
        if isinstance(name, str) and (not symbols or name in symbols) and isinstance(strict, Mapping):
            target_side, candidate_side = strict.get("target"), strict.get("candidate")
            if isinstance(target_side, Mapping) and isinstance(candidate_side, Mapping):
                rows, other = target_side.get("rows"), candidate_side.get("rows")
                if isinstance(rows, list) and isinstance(other, list):
                    pairs.append((name, rows, other))
    for name, target_rows, candidate_rows in pairs:
        for index in range(max(len(target_rows), len(candidate_rows))):
            target_row = target_rows[index] if index < len(target_rows) else {}
            candidate_row = candidate_rows[index] if index < len(candidate_rows) else {}
            if not isinstance(target_row, Mapping) or not isinstance(candidate_row, Mapping):
                continue
            kind = next((value for value in (
                target_row.get("diff_kind"), candidate_row.get("diff_kind"),
            ) if isinstance(value, str) and value not in {"DIFF_NONE", "NONE"}), None)
            if kind is None:
                continue
            target_ins = target_row.get("instruction") or {}
            candidate_ins = candidate_row.get("instruction") or {}
            if not isinstance(target_ins, Mapping) or not isinstance(candidate_ins, Mapping):
                continue
            target_text = _compact_history_text(target_ins.get("formatted") or "<absent>", 160)
            candidate_text = _compact_history_text(candidate_ins.get("formatted") or "<absent>", 160)
            row_index = target_row.get("index", candidate_row.get("index", index))
            if not isinstance(row_index, int) or isinstance(row_index, bool):
                row_index = index
            return f"`{name}` row {row_index} ({kind}): target `{target_text}`; candidate `{candidate_text}`"
    return None


def _render_current_target(
    data: Mapping[str, Any], owner: Mapping[str, Any], kind: str,
    target: str, stable_identity: str | None, symbols: Sequence[str],
    mismatch: tuple[Path, str] | None,
) -> str:
    source = str(owner.get("source") or "unavailable")
    location = source
    symbol = target
    if kind == "function":
        try:
            parsed = parse_functions((Path(data["root"]) / source).read_text(encoding="utf-8"))
            function = next((item for item in parsed if item.symbol in symbols), None)
            if function is not None:
                location = f"{source}:{function.start}-{function.end}"
                symbol = function.symbol
        except (OSError, UnicodeError, TypeError, ValueError):
            pass
    lines = [
        "## Current target", "",
        f"- Owner: `{owner.get('id')}`; target: `{symbol}`.",
        f"- Stable identity: `{stable_identity or owner.get('id')}`.",
        f"- Current source: `{location}`.",
    ]
    if mismatch is not None:
        path, detail = mismatch
        try:
            report = path.relative_to(Path(data["root"]))
        except ValueError:
            report = path
        lines.extend([
            f"- First local mismatch: {detail}.",
            f"- Report: `{report}` (current-source binding not verified).",
        ])
    return "\n".join(lines)


def _budget_sections(
    preamble: str,
    sections: Sequence[tuple[str, str]],
    *,
    budget: int,
) -> str:
    char_budget = max(1200, budget * 4)
    preamble_limit = min(max(350, int(char_budget * 0.04)), len(preamble))
    preamble_text = _clip_plain(preamble, preamble_limit)
    remaining = max(800, char_budget - len(preamble_text) - 2)
    weights = [DEFAULT_SECTION_WEIGHTS.get(name, 3) for name, _ in sections]
    total_weight = max(1, sum(weights))
    rendered: list[str] = []
    for (name, text), weight in zip(sections, weights):
        quota = max(
            250 if name in MANDATORY else 120,
            int(remaining * weight / total_weight),
        )
        if name in {"Current target", "Relevant recovered knowledge", "Authenticated constraints", "Acceptance criteria"}:
            clipped = text
        elif name == "Target function":
            clipped = _clip_target(text, quota)
        else:
            clipped = _clip_plain(text, quota)
        rendered.append(clipped)
    result = preamble_text + "\n\n" + "\n\n".join(rendered) + "\n"
    if len(result) <= char_budget:
        return result
    low_priority = {
        "Broader recovery diagnostics",
        "Local reports",
        "Bounded owner neighbourhood",
        "Owner functions",
        "Accepted evidence",
        "Rejected evidence and probes",
        "Naming ledger",
        "Remaining recovery debt",
        "Operational dependency context",
    }
    mutable = list(zip([name for name, _ in sections], rendered))
    for index, (name, text) in enumerate(mutable):
        if len(preamble_text) + sum(len(value) + 2 for _, value in mutable) <= char_budget:
            break
        if name in low_priority:
            mutable[index] = (name, _clip_plain(text, max(100, len(text) // 3)))
    result = preamble_text + "\n\n" + "\n\n".join(value for _, value in mutable) + "\n"
    if len(result) <= char_budget:
        return result
    return result[:char_budget].rstrip() + "\n[context clipped]\n"


def build_context(
    data: dict[str, Any],
    kind: str,
    target: str,
    *,
    owner_id: str | None = None,
    budget: int = 12000,
    knowledge_limit: int | None = None,
    symptoms: Sequence[str] | None = None,
    local_evidence: bool = False,
    reports: Sequence[str] | None = None,
) -> str:
    owner, stable_identity = resolve_context_target(data, kind, target, owner_id)
    matches = select_context_knowledge(
        data,
        owner,
        stable_identity=stable_identity,
        symptoms=symptoms,
        limit=knowledge_limit,
    )
    base = base_context_pack(
        data,
        kind,
        target,
        owner_id=owner_id,
        budget=max(20000, budget * 2),
    )
    preamble, sections = _split_sections(base)
    # The base packet is intentionally oversized for section-level allocation;
    # its internal construction budget is not the user's requested budget.
    budget_line = f"- Approximate token budget: `{budget}`"
    if re.search(r"(?m)^- Approximate token budget:", preamble):
        preamble = re.sub(r"(?m)^- Approximate token budget:.*$", budget_line, preamble)
    else:
        preamble = preamble.rstrip() + "\n" + budget_line
    target_symbols = _context_target_symbols(data, owner, kind, target, stable_identity)
    insertion = 2
    sections.insert(
        insertion,
        (
            HISTORY_SECTION,
            render_rejected_probe_history(
                Path(data["root"]),
                owner,
                target_symbols=target_symbols,
            ),
        ),
    )
    insertion += 1
    root = Path(data["root"])
    try:
        central = (
            RecoveryMemory.for_root(root).context_memory(
                str(owner.get("id") or owner.get("source")),
                None,
                limit=12,
            )
            if recovery_memory_available(root)
            else {
                "experiments": [],
                "reports": [],
            }
        )
        central_text = render_context_memory(central)
    except (OSError, RecoveryMemoryError, QueueError) as exc:
        central_text = (
            "## Central recovery memory\n\n"
            f"- Unavailable: {exc}. Do not compile until lane startup passes."
        )
    sections.insert(insertion, ("Central recovery memory", central_text))
    insertion += 1
    scoped = [match for match in matches if match.counterexample or match.relevance in {
        "exact target", "confirmed example", "owner-specific", "confirmed owner example",
    }]
    diagnostics = [match for match in matches if match not in scoped]
    knowledge = render_compact_knowledge(data, scoped)
    sections.insert(insertion, ("Relevant recovered knowledge", knowledge))
    insertion += 1
    sections.insert(
        insertion,
        ("Operational dependency context", render_operational_dependencies(data, owner)),
    )

    requested = list(reports or [])
    if local_evidence:
        context = owner.get("context", {})
        if isinstance(context, Mapping):
            requested.extend(
                str(path)
                for path in context.get("reports", [])
                if isinstance(path, str)
            )
    summaries = []
    mismatch = None
    evidence_symbols = target_symbols
    if kind == "owner":
        try:
            evidence_symbols = [function.symbol for function in parse_functions(
                (root / str(owner["source"])).read_text(encoding="utf-8")
            )]
        except (OSError, UnicodeError, KeyError, TypeError, ValueError):
            evidence_symbols = []
    for path in dict.fromkeys(requested):
        candidate = Path(path)
        candidate = candidate if candidate.is_absolute() else root / candidate
        if not candidate.is_file():
            continue
        try:
            summaries.append(summarize_report(candidate))
        except EvidenceError:
            continue
        if mismatch is None:
            detail = _first_local_mismatch(candidate, evidence_symbols)
            if detail is not None:
                mismatch = (candidate, detail)
    if local_evidence or reports:
        sections.insert(
            insertion + 1,
            ("Local object-diff evidence", render_summary(summaries)),
        )
    sections.insert(2, ("Current target", _render_current_target(
        data, owner, kind, target, stable_identity, target_symbols, mismatch,
    )))
    if diagnostics:
        sections.append(("Broader recovery diagnostics", render_compact_knowledge(
            data, diagnostics, heading="Broader recovery diagnostics",
        )))
    return _budget_sections(preamble, sections, budget=budget)


def context_token_estimate(text: str) -> int:
    return token_estimate(text)
