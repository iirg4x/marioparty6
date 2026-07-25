#!/usr/bin/env python3
"""Reusable recovery knowledge selection, context injection, and auditing."""

from __future__ import annotations

import json
import sqlite3
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from tools.recovery_core import (
    build_index as _build_index,
    context_pack as _base_context_pack,
    markdown_report as _base_markdown_report,
)
from tools.recovery_data import (
    RecoveryError,
    parse_functions,
    read_json,
    stable_id,
    token_estimate,
)

CARD_CLASSIFICATIONS = {
    "confirmed_rule",
    "contextual_heuristic",
    "owner_constraint",
}
CONFIDENCE_BONUS = {
    "confirmed": 5,
    "high": 4,
    "medium": 3,
    "low": 2,
    "unknown": 0,
}
DEFAULT_CARD_LIMIT = 5


@dataclass(frozen=True)
class KnowledgeMatch:
    card: dict[str, Any]
    score: int
    relevance: str
    reasons: tuple[str, ...]
    counterexample: bool = False

    def as_dict(self) -> dict[str, Any]:
        return {
            "id": self.card.get("id"),
            "title": self.card.get("title"),
            "classification": self.card.get("classification"),
            "category": self.card.get("category"),
            "compiler": self.card.get("compiler"),
            "confidence": self.card.get("confidence"),
            "score": self.score,
            "relevance": self.relevance,
            "reasons": list(self.reasons),
            "counterexample": self.counterexample,
            "rule": self.card.get("rule"),
            "safe_actions": self.card.get("safe_actions", []),
            "evidence": self.card.get("evidence", []),
        }


def _strings(value: Any) -> list[str]:
    if not isinstance(value, list):
        return []
    return [item for item in value if isinstance(item, str) and item]


def _patterns_document(data: dict[str, Any]) -> dict[str, Any]:
    project_files = data.get("project", {}).get("files", {})
    relative = "config/recovery/compiler_patterns.json"
    if isinstance(project_files, dict):
        relative = str(project_files.get("compiler_patterns", relative))
    value = read_json(Path(data["root"]) / relative)
    return value if isinstance(value, dict) else {}


def validate_knowledge(data: dict[str, Any]) -> list[str]:
    """Validate rich recovery-knowledge cards and their cross-references."""

    root: Path = data["root"]
    document = _patterns_document(data)
    patterns = data.get("patterns", [])
    errors: list[str] = []
    version = document.get("schema_version")
    if patterns and version != 2:
        errors.append("compiler_patterns.schema_version must be 2 for knowledge cards")
    elif not patterns and version not in {1, 2}:
        errors.append("compiler_patterns.schema_version must be 1 or 2")

    confidence = set(data.get("project", {}).get("confidence_levels", []))
    owner_ids = {str(item.get("id")) for item in data.get("owners", [])}
    exception_ids = {str(item.get("id")) for item in data.get("exceptions", [])}
    seen: set[str] = set()

    for index, card in enumerate(patterns):
        where = f"compiler_patterns[{index}]"
        card_id = card.get("id")
        if not isinstance(card_id, str) or not card_id:
            errors.append(f"{where}: missing id")
        elif card_id in seen:
            errors.append(f"{where}: duplicate id {card_id}")
        else:
            seen.add(card_id)

        for key in ("title", "category", "summary", "rule", "conditions"):
            if not isinstance(card.get(key), str) or not card.get(key):
                errors.append(f"{where}: {key} must be a non-empty string")

        classification = card.get("classification")
        if classification not in CARD_CLASSIFICATIONS:
            errors.append(f"{where}: invalid classification {classification!r}")
        if card.get("confidence") not in confidence:
            errors.append(f"{where}: invalid confidence {card.get('confidence')!r}")
        compiler = card.get("compiler")
        if compiler is not None and (not isinstance(compiler, str) or not compiler):
            errors.append(f"{where}: compiler must be a non-empty string or null")

        source_condition = card.get("source_condition")
        if not isinstance(source_condition, dict):
            errors.append(f"{where}: source_condition must be an object")
            source_condition = {}
        if not isinstance(source_condition.get("change"), str) or not source_condition.get("change"):
            errors.append(f"{where}: source_condition.change is required")
        requires = source_condition.get("requires", [])
        if not isinstance(requires, list) or not all(isinstance(item, str) and item for item in requires):
            errors.append(f"{where}: source_condition.requires must be a string list")

        emitted = card.get("emitted_effect")
        if not isinstance(emitted, dict):
            errors.append(f"{where}: emitted_effect must be an object")
            emitted = {}
        for key in ("possible_changes", "known_signatures"):
            values = emitted.get(key, [])
            if not isinstance(values, list) or not all(isinstance(item, str) and item for item in values):
                errors.append(f"{where}: emitted_effect.{key} must be a string list")

        safe_actions = card.get("safe_actions")
        if not isinstance(safe_actions, list) or not safe_actions or not all(
            isinstance(item, str) and item for item in safe_actions
        ):
            errors.append(f"{where}: safe_actions must be a non-empty string list")

        applicability = card.get("applicability")
        if not isinstance(applicability, dict):
            errors.append(f"{where}: applicability must be an object")
            applicability = {}
        for key in ("owners", "stable_ids", "modules", "owner_tags"):
            values = applicability.get(key, [])
            if not isinstance(values, list) or not all(isinstance(item, str) and item for item in values):
                errors.append(f"{where}: applicability.{key} must be a string list")
        compiler_wide = applicability.get("compiler_wide", False)
        project_wide = applicability.get("project_wide", False)
        if not isinstance(compiler_wide, bool):
            errors.append(f"{where}: applicability.compiler_wide must be boolean")
        if not isinstance(project_wide, bool):
            errors.append(f"{where}: applicability.project_wide must be boolean")
        if compiler_wide and not compiler:
            errors.append(f"{where}: compiler_wide cards require compiler")

        scoped_owners = set(_strings(applicability.get("owners")))
        unknown_owners = scoped_owners - owner_ids
        if unknown_owners:
            errors.append(
                f"{where}: unknown applicability owners: {', '.join(sorted(unknown_owners))}"
            )
        if classification == "owner_constraint":
            if compiler_wide or project_wide:
                errors.append(f"{where}: owner_constraint cannot be compiler/project wide")
            if not scoped_owners and not _strings(applicability.get("stable_ids")):
                errors.append(f"{where}: owner_constraint needs owners or stable_ids")

        for key in ("examples", "counterexamples", "related_exceptions", "evidence"):
            values = card.get(key, [])
            if not isinstance(values, list) or not all(isinstance(item, str) and item for item in values):
                errors.append(f"{where}: {key} must be a string list")

        unknown_exceptions = set(_strings(card.get("related_exceptions"))) - exception_ids
        if unknown_exceptions:
            errors.append(
                f"{where}: unknown related exceptions: {', '.join(sorted(unknown_exceptions))}"
            )
        for reference in _strings(card.get("evidence")):
            if reference.startswith("docs/") and not (root / reference).is_file():
                errors.append(f"{where}: missing evidence {reference}")

    return sorted(set(errors))


def _owner_by_id(data: dict[str, Any], owner_id: str) -> dict[str, Any]:
    matches = [item for item in data.get("owners", []) if item.get("id") == owner_id]
    if len(matches) != 1:
        raise RecoveryError(f"owner {owner_id!r} was not found")
    return matches[0]


def resolve_context_target(
    data: dict[str, Any],
    kind: str,
    target: str,
    owner_id: str | None = None,
) -> tuple[dict[str, Any], str | None]:
    """Resolve the owner and stable identity used for knowledge selection."""

    root: Path = data["root"]
    if kind == "owner":
        return _owner_by_id(data, target), None
    if kind != "function":
        raise RecoveryError("context kind must be owner or function")

    candidates: list[tuple[dict[str, Any], str]] = []
    for owner in data.get("owners", []):
        if owner_id and owner.get("id") != owner_id:
            continue
        explicit = {
            item.get("symbol"): item.get("stable_id")
            for item in owner.get("symbols", [])
            if isinstance(item, dict)
        }
        text = (root / owner["source"]).read_text(encoding="utf-8")
        for function in parse_functions(text):
            identity = str(explicit.get(function.symbol) or stable_id(owner, function.symbol))
            if target in {function.symbol, identity}:
                candidates.append((owner, identity))
    if len(candidates) != 1:
        raise RecoveryError(
            f"function {target!r} resolved to {len(candidates)} owners; "
            "pass --owner when ambiguous"
        )
    return candidates[0]


def select_knowledge_cards(
    data: dict[str, Any],
    owner: dict[str, Any],
    *,
    stable_identity: str | None = None,
    limit: int | None = None,
) -> list[KnowledgeMatch]:
    """Rank reusable findings for one task without overgeneralizing them."""

    errors = validate_knowledge(data)
    if errors:
        raise RecoveryError("recovery knowledge invalid:\n- " + "\n- ".join(errors))

    if limit is None:
        configured = data.get("project", {}).get("knowledge_card_limit", DEFAULT_CARD_LIMIT)
        limit = configured if isinstance(configured, int) else DEFAULT_CARD_LIMIT
    if limit <= 0:
        return []

    owner_id = str(owner.get("id", ""))
    module = str(owner.get("module", ""))
    compiler = owner.get("compiler")
    tags = set(_strings(owner.get("tags")))
    matches: list[KnowledgeMatch] = []

    for card in data.get("patterns", []):
        applicability = card.get("applicability", {})
        scoped_stable = set(_strings(applicability.get("stable_ids")))
        scoped_owners = set(_strings(applicability.get("owners")))
        scoped_modules = set(_strings(applicability.get("modules")))
        scoped_tags = set(_strings(applicability.get("owner_tags")))
        examples = set(_strings(card.get("examples")))
        counterexamples = set(_strings(card.get("counterexamples")))
        references = {owner_id}
        if stable_identity:
            references.add(stable_identity)

        is_counterexample = bool(references & counterexamples)
        exact_stable = bool(stable_identity and stable_identity in scoped_stable)
        example_stable = bool(stable_identity and stable_identity in examples)
        exact_owner = owner_id in scoped_owners
        example_owner = owner_id in examples
        module_match = bool(module and module in scoped_modules)
        tag_matches = sorted(tags & scoped_tags)
        compiler_match = bool(compiler and card.get("compiler") == compiler)
        project_wide = applicability.get("project_wide") is True
        compiler_wide = applicability.get("compiler_wide") is True and compiler_match

        if card.get("compiler") and compiler and card.get("compiler") != compiler:
            continue

        classification = card.get("classification")
        if classification == "owner_constraint":
            applicable = exact_stable or exact_owner or is_counterexample
        else:
            applicable = any(
                (
                    exact_stable,
                    example_stable,
                    exact_owner,
                    example_owner,
                    module_match,
                    bool(tag_matches),
                    compiler_wide,
                    project_wide,
                    is_counterexample,
                )
            )
        if not applicable:
            continue

        reasons: list[str] = []
        if is_counterexample:
            score = 120
            relevance = "counterexample"
            reasons.append("this target is a recorded counterexample; do not apply the rule blindly")
        elif exact_stable:
            score = 110
            relevance = "exact target"
            reasons.append("stable identity is explicitly in scope")
        elif example_stable:
            score = 104
            relevance = "confirmed example"
            reasons.append("stable identity is a confirmed example")
        elif exact_owner:
            score = 90
            relevance = "owner-specific"
            reasons.append("owner is explicitly in scope")
        elif example_owner:
            score = 84
            relevance = "confirmed owner example"
            reasons.append("owner is a confirmed example")
        elif module_match:
            score = 68
            relevance = "module-related"
            reasons.append(f"module `{module}` is in scope")
        elif tag_matches:
            score = 58
            relevance = "owner-tag related"
            reasons.append("matching owner tags: " + ", ".join(tag_matches))
        elif compiler_wide:
            score = 35
            relevance = "compiler-wide"
            reasons.append(f"owner uses `{compiler}`")
        else:
            score = 20
            relevance = "project-wide"
            reasons.append("project-wide recovery rule")

        score += CONFIDENCE_BONUS.get(str(card.get("confidence")), 0)
        if classification == "owner_constraint":
            score += 3
        elif classification == "confirmed_rule":
            score += 2
        matches.append(
            KnowledgeMatch(
                card=card,
                score=score,
                relevance=relevance,
                reasons=tuple(reasons),
                counterexample=is_counterexample,
            )
        )

    matches.sort(key=lambda item: (-item.score, str(item.card.get("id", ""))))
    return matches[:limit]


def render_knowledge_cards(matches: list[KnowledgeMatch]) -> str:
    if not matches:
        return "## Relevant recovered knowledge\n\n- No applicable knowledge cards were selected."

    lines = [
        "## Relevant recovered knowledge",
        "",
        "These cards are selected automatically. A compiler-wide card is a diagnostic rule, not permission to copy an owner-specific source shape.",
    ]
    for match in matches:
        card = match.card
        source_condition = card.get("source_condition", {})
        emitted = card.get("emitted_effect", {})
        requires = _strings(source_condition.get("requires"))
        changes = _strings(emitted.get("possible_changes"))
        signatures = _strings(emitted.get("known_signatures"))
        safe_actions = _strings(card.get("safe_actions"))
        counterexamples = _strings(card.get("counterexamples"))
        evidence = _strings(card.get("evidence"))
        lines += [
            "",
            f"### {card.get('title')} (`{card.get('id')}`)",
            f"- **Type:** `{card.get('classification')}` · `{card.get('category')}` · confidence `{card.get('confidence')}`",
            f"- **Relevance:** {match.relevance} — {'; '.join(match.reasons)}",
            f"- **Trigger:** {source_condition.get('change')}",
        ]
        if requires:
            lines.append(f"- **Requires:** {'; '.join(requires)}")
        if changes:
            lines.append(f"- **Possible output changes:** {'; '.join(changes)}")
        if signatures:
            lines.append(f"- **Known signatures:** {'; '.join(signatures)}")
        lines.append(f"- **Rule:** {card.get('rule')}")
        if safe_actions:
            lines.append(f"- **Safe actions:** {'; '.join(safe_actions)}")
        if counterexamples:
            lines.append(f"- **Counterexamples:** {', '.join(f'`{item}`' for item in counterexamples)}")
        if evidence:
            lines.append(f"- **Evidence:** {', '.join(f'`{item}`' for item in evidence)}")
    return "\n".join(lines)


def context_pack(
    data: dict[str, Any],
    kind: str,
    target: str,
    *,
    owner_id: str | None = None,
    budget: int = 12000,
    knowledge_limit: int | None = None,
) -> str:
    """Generate base context with high-priority reusable knowledge injected."""

    errors = validate_knowledge(data)
    if errors:
        raise RecoveryError("recovery knowledge invalid:\n- " + "\n- ".join(errors))

    if knowledge_limit == 0:
        return _base_context_pack(
            data,
            kind,
            target,
            owner_id=owner_id,
            budget=budget,
        )

    owner, stable_identity = resolve_context_target(data, kind, target, owner_id)
    matches = select_knowledge_cards(
        data,
        owner,
        stable_identity=stable_identity,
        limit=knowledge_limit,
    )
    knowledge = render_knowledge_cards(matches)
    knowledge_tokens = token_estimate(knowledge)
    base_budget = max(1000, budget - knowledge_tokens - 50)
    base = _base_context_pack(
        data,
        kind,
        target,
        owner_id=owner_id,
        budget=base_budget,
    )
    marker = "\n## Target function" if kind == "function" else "\n## Owner functions"
    if marker in base:
        combined = base.replace(marker, f"\n\n{knowledge}{marker}", 1)
    else:
        combined = base.rstrip() + "\n\n" + knowledge + "\n"
    if token_estimate(combined) <= budget:
        return combined
    limit = max(0, budget * 4 - 100)
    return combined[:limit].rstrip() + "\n\n[clipped to token budget]\n"


def _knowledge_search_text(card: dict[str, Any]) -> str:
    source_condition = card.get("source_condition", {})
    emitted = card.get("emitted_effect", {})
    values: list[str] = [
        str(card.get("title", "")),
        str(card.get("classification", "")),
        str(card.get("category", "")),
        str(card.get("compiler", "")),
        str(card.get("summary", "")),
        str(card.get("conditions", "")),
        str(source_condition.get("change", "")),
        str(card.get("rule", "")),
    ]
    for value in (
        source_condition.get("requires"),
        emitted.get("possible_changes"),
        emitted.get("known_signatures"),
        card.get("safe_actions"),
        card.get("examples"),
        card.get("counterexamples"),
    ):
        values.extend(_strings(value))
    return " ".join(item for item in values if item)


def build_recovery_index(data: dict[str, Any], output: Path) -> dict[str, int]:
    """Build the existing index, then enrich pattern search with full cards."""

    errors = validate_knowledge(data)
    if errors:
        raise RecoveryError("recovery knowledge invalid:\n- " + "\n- ".join(errors))
    counts = _build_index(data, output)
    with sqlite3.connect(output) as connection:
        for card in data.get("patterns", []):
            connection.execute(
                "UPDATE search SET text=? WHERE kind='pattern' AND key=?",
                (_knowledge_search_text(card), card.get("id")),
            )
        connection.commit()
    return counts


def knowledge_audit(data: dict[str, Any]) -> dict[str, Any]:
    """Show how much historical wave evidence has reusable structured cards."""

    root: Path = data["root"]
    wave_paths = sorted(
        path.relative_to(root).as_posix()
        for path in (root / "docs").glob("native_matching_wave*.md")
    )
    card_refs = {
        reference
        for card in data.get("patterns", [])
        for reference in _strings(card.get("evidence"))
        if reference.startswith("docs/native_matching_wave")
    }
    owner_refs = {
        evidence.get("reference")
        for owner in data.get("owners", [])
        for evidence in owner.get("evidence", [])
        if isinstance(evidence, dict)
        and isinstance(evidence.get("reference"), str)
        and evidence.get("reference", "").startswith("docs/native_matching_wave")
    }
    exception_refs = {
        reference
        for item in data.get("exceptions", [])
        for reference in _strings(item.get("evidence"))
        if reference.startswith("docs/native_matching_wave")
    }
    by_classification = Counter(
        str(card.get("classification", "unknown"))
        for card in data.get("patterns", [])
    )
    undistilled = sorted(set(wave_paths) - card_refs)
    return {
        "cards": len(data.get("patterns", [])),
        "by_classification": dict(sorted(by_classification.items())),
        "wave_documents": len(wave_paths),
        "waves_referenced_by_cards": len(set(wave_paths) & card_refs),
        "waves_referenced_by_owner_evidence": len(set(wave_paths) & owner_refs),
        "waves_referenced_by_exceptions": len(set(wave_paths) & exception_refs),
        "waves_without_knowledge_card": undistilled,
    }


def render_knowledge_audit(audit: dict[str, Any], *, max_items: int = 20) -> str:
    lines = [
        "# Recovery knowledge audit",
        "",
        f"- Knowledge cards: `{audit.get('cards', 0)}`",
        f"- Historical wave documents: `{audit.get('wave_documents', 0)}`",
        f"- Waves referenced by reusable cards: `{audit.get('waves_referenced_by_cards', 0)}`",
        f"- Waves referenced only by owner evidence: `{audit.get('waves_referenced_by_owner_evidence', 0)}`",
        f"- Waves referenced by source-shape exceptions: `{audit.get('waves_referenced_by_exceptions', 0)}`",
        "",
        "## Card classifications",
    ]
    classifications = audit.get("by_classification", {})
    if classifications:
        lines.extend(
            f"- `{name}`: {count}"
            for name, count in sorted(classifications.items())
        )
    else:
        lines.append("- None.")
    undistilled = list(audit.get("waves_without_knowledge_card", []))
    lines += [
        "",
        "## Wave reports without reusable knowledge cards",
        "",
        "These files remain historical evidence. Their presence does not load them into context, but reusable findings should be distilled before agents repeat the investigation.",
    ]
    if not undistilled:
        lines.append("- None.")
    else:
        lines.extend(f"- `{path}`" for path in undistilled[:max_items])
        if len(undistilled) > max_items:
            lines.append(f"- … and {len(undistilled) - max_items} more")
    return "\n".join(lines).rstrip() + "\n"


def recovery_report(data: dict[str, Any]) -> str:
    """Extend the source-quality report with actionable knowledge cards."""

    base = _base_markdown_report(data).rstrip()
    lines = [base, "", "## Actionable recovery knowledge", ""]
    for card in data.get("patterns", []):
        lines += [
            f"### {card.get('title')} (`{card.get('id')}`)",
            f"- Type: `{card.get('classification')}` · category `{card.get('category')}` · compiler `{card.get('compiler') or 'project'}` · confidence `{card.get('confidence')}`",
            f"- Trigger: {card.get('source_condition', {}).get('change')}",
            f"- Rule: {card.get('rule')}",
            f"- Safe actions: {'; '.join(_strings(card.get('safe_actions')))}",
            f"- Examples: {', '.join(f'`{item}`' for item in _strings(card.get('examples'))) or 'none'}",
            f"- Counterexamples: {', '.join(f'`{item}`' for item in _strings(card.get('counterexamples'))) or 'none'}",
            "",
        ]
    audit = knowledge_audit(data)
    lines += [
        "## Historical evidence distillation",
        "",
        f"- Wave documents: `{audit['wave_documents']}`",
        f"- Referenced by reusable cards: `{audit['waves_referenced_by_cards']}`",
        f"- Still without a knowledge card: `{len(audit['waves_without_knowledge_card'])}`",
        "",
        "Run `python tools/knowledge_cards.py audit` for the bounded backlog. Historical wave text is not injected automatically into task context.",
    ]
    return "\n".join(lines).rstrip() + "\n"
