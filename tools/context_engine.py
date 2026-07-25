#!/usr/bin/env python3
"""Symptom-aware, section-budgeted recovery context with local evidence."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any, Mapping, Sequence

from tools.knowledge_freshness import card_freshness
from tools.local_evidence import EvidenceError, render_summary, summarize_report
from tools.owner_catalog import CatalogError, build_catalog
from tools.recovery_core import context_pack as base_context_pack
from tools.recovery_data import token_estimate
from tools.recovery_knowledge import (
    KnowledgeMatch,
    resolve_context_target,
    select_knowledge_cards,
)

DEFAULT_SECTION_WEIGHTS = {
    "Recovery contract": 7,
    "Owner state": 7,
    "Relevant recovered knowledge": 18,
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
    data: dict[str, Any], matches: Sequence[KnowledgeMatch]
) -> str:
    lines = [
        "## Relevant recovered knowledge",
        "",
        "Selected deterministically. Compiler-wide cards are diagnostics; owner constraints never transfer to unrelated owners.",
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
        if name in {"Relevant recovered knowledge", "Acceptance criteria"}:
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
    knowledge = render_compact_knowledge(data, matches)
    base = base_context_pack(
        data,
        kind,
        target,
        owner_id=owner_id,
        budget=max(20000, budget * 2),
    )
    preamble, sections = _split_sections(base)
    insertion = 2
    sections.insert(insertion, ("Relevant recovered knowledge", knowledge))
    sections.insert(
        insertion + 1,
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
    root = Path(data["root"])
    for path in dict.fromkeys(requested):
        candidate = Path(path)
        candidate = candidate if candidate.is_absolute() else root / candidate
        if not candidate.is_file():
            continue
        try:
            summaries.append(summarize_report(candidate))
        except EvidenceError:
            continue
    if local_evidence or reports:
        sections.insert(
            insertion + 2,
            ("Local object-diff evidence", render_summary(summaries)),
        )
    return _budget_sections(preamble, sections, budget=budget)


def context_token_estimate(text: str) -> int:
    return token_estimate(text)
