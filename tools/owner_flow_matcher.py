#!/usr/bin/env python3
"""Bind residual stack/register flows to semantic source owners.

The matcher consumes a compact ``focus_symbol_report/v1`` artifact plus a
hash-bound ``owner_flow_context/v1`` manifest.  It classifies every strict
argument-only row, builds candidate-to-target stack-home and register-flow
components, and solves a minimum-cost bipartite assignment from those
components to live source owners.

This is a diagnostic and candidate-ranking tool.  It emits no source, retains
no candidate, and advances no recovery or promotion authority.
"""

from __future__ import annotations

import argparse
import itertools
import json
import os
import re
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import typed_pool_owner_manifest as owner_manifest


SCHEMA = "owner_flow_matcher/v1"
CONTEXT_SCHEMA = "owner_flow_context/v1"
ROUTE = "closed_semantic_owner_flow_then_composed_semantic_closure"
_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_MEMORY_RE = re.compile(
    r"^(?P<opcode>lbz|lhz|lha|lwz|lfs|lfd|stb|sth|stw|stfs|stfd)\s+"
    r"(?P<value>[^,]+),\s*(?P<offset>[+-]?(?:0x[0-9a-f]+|[0-9]+))\(r1\)$",
    re.IGNORECASE,
)
_REGISTER_RE = re.compile(r"\b(?:r(?:[12]?[0-9]|3[01])|f(?:[12]?[0-9]|3[01]))\b", re.IGNORECASE)
_INTEGER_RE = re.compile(r"^[+-]?(?:0x[0-9a-f]+|[0-9]+)$", re.IGNORECASE)
_POOL_RE = re.compile(r",\s*(?P<owner>[^,\s]+)@sda21\s*$", re.IGNORECASE)
_SUPPORTED_OWNER_KINDS = {"stack_home", "register_flow"}
_SUPPORTED_SEMANTIC_KINDS = {"pool_owner", "immediate_semantic", "branch_topology"}
_INF = 10**9


class OwnerFlowInputError(ValueError):
    """Raised when evidence is malformed, incomplete, or contradictory."""


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise OwnerFlowInputError(f"{label} must be an object")
    return value


def _sequence(value: Any, label: str) -> list[Any]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)):
        raise OwnerFlowInputError(f"{label} must be an array")
    return list(value)


def _closed(
    value: Any,
    label: str,
    *,
    required: set[str],
    optional: set[str] | None = None,
) -> Mapping[str, Any]:
    row = _mapping(value, label)
    allowed = required | (optional or set())
    missing = sorted(required - set(row))
    extra = sorted(set(row) - allowed)
    if missing:
        raise OwnerFlowInputError(f"{label} missing fields: {', '.join(missing)}")
    if extra:
        raise OwnerFlowInputError(f"{label} has unknown fields: {', '.join(extra)}")
    return row


def _text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip():
        raise OwnerFlowInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise OwnerFlowInputError(f"{label} exceeds {limit} characters")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise OwnerFlowInputError(f"{label} must be a lowercase SHA-256 digest")
    return result


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool):
        raise OwnerFlowInputError(f"{label} must be an integer")
    try:
        result = int(value)
    except (TypeError, ValueError) as exc:
        raise OwnerFlowInputError(f"{label} must be an integer") from exc
    if result < minimum:
        raise OwnerFlowInputError(f"{label} must be >= {minimum}")
    return result


def _integers(value: Any, label: str, *, allow_empty: bool = False) -> list[int]:
    result = [_integer(item, f"{label}[{index}]") for index, item in enumerate(_sequence(value, label))]
    if not result and not allow_empty:
        raise OwnerFlowInputError(f"{label} must not be empty")
    if len(result) != len(set(result)):
        raise OwnerFlowInputError(f"{label} contains duplicates")
    return result


def _strings(value: Any, label: str, *, allow_empty: bool = True) -> list[str]:
    result = [_text(item, f"{label}[{index}]", limit=256) for index, item in enumerate(_sequence(value, label))]
    if not result and not allow_empty:
        raise OwnerFlowInputError(f"{label} must not be empty")
    if len(result) != len(set(result)):
        raise OwnerFlowInputError(f"{label} contains duplicates")
    return result


def _parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed(
        value,
        "context",
        required={
            "schema",
            "function",
            "focus_artifact_sha256",
            "report_sha256",
            "source_sha256",
            "candidate_object_sha256",
            "target_object_sha256",
            "protected_siblings_zero_loss",
            "physical_relocations",
            "owners",
            "semantic_groups",
        },
    )
    if context["schema"] != CONTEXT_SCHEMA:
        raise OwnerFlowInputError(f"context.schema must be {CONTEXT_SCHEMA!r}")
    if context["protected_siblings_zero_loss"] is not True:
        raise OwnerFlowInputError("context.protected_siblings_zero_loss must be true")

    physical = _closed(
        context["physical_relocations"],
        "context.physical_relocations",
        required={"status"},
        optional={"target_count", "candidate_count", "difference_count", "receipt_sha256"},
    )
    physical_status = _text(physical["status"], "context.physical_relocations.status", limit=16)
    if physical_status not in {"exact", "unknown"}:
        raise OwnerFlowInputError("context.physical_relocations.status must be exact or unknown")
    parsed_physical: dict[str, Any] = {"status": physical_status}
    for key in ("target_count", "candidate_count", "difference_count"):
        if key in physical:
            parsed_physical[key] = _integer(physical[key], f"context.physical_relocations.{key}")
    if "receipt_sha256" in physical and physical["receipt_sha256"] is not None:
        parsed_physical["receipt_sha256"] = _sha256(
            physical["receipt_sha256"], "context.physical_relocations.receipt_sha256"
        )
    if physical_status == "exact":
        required_counts = {"target_count", "candidate_count", "difference_count"}
        if not required_counts <= set(parsed_physical):
            raise OwnerFlowInputError("exact physical relocation evidence requires all counts")
        if parsed_physical["target_count"] != parsed_physical["candidate_count"]:
            raise OwnerFlowInputError("exact physical relocation counts differ")
        if parsed_physical["difference_count"] != 0:
            raise OwnerFlowInputError("exact physical relocation evidence has differences")

    owners: list[dict[str, Any]] = []
    owner_ids: set[str] = set()
    for index, raw_owner in enumerate(_sequence(context["owners"], "context.owners")):
        label = f"context.owners[{index}]"
        owner = _closed(
            raw_owner,
            label,
            required={
                "id",
                "kind",
                "source_type",
                "candidate_tokens",
                "row_indices",
                "declaration_line",
                "definition_lines",
                "use_lines",
                "call_boundaries",
                "write_only_target_observed",
            },
        )
        owner_id = _text(owner["id"], f"{label}.id", limit=128)
        if owner_id in owner_ids:
            raise OwnerFlowInputError(f"duplicate owner id {owner_id!r}")
        owner_ids.add(owner_id)
        kind = _text(owner["kind"], f"{label}.kind", limit=32)
        if kind not in _SUPPORTED_OWNER_KINDS:
            raise OwnerFlowInputError(f"{label}.kind is unsupported: {kind}")
        write_only = owner["write_only_target_observed"]
        if not isinstance(write_only, bool):
            raise OwnerFlowInputError(f"{label}.write_only_target_observed must be boolean")
        use_lines = _integers(owner["use_lines"], f"{label}.use_lines", allow_empty=True)
        if not use_lines and not write_only:
            raise OwnerFlowInputError(f"{label} has no uses and is not target-observed write-only")
        owners.append(
            {
                "id": owner_id,
                "kind": kind,
                "source_type": _text(owner["source_type"], f"{label}.source_type", limit=128),
                "candidate_tokens": _strings(
                    owner["candidate_tokens"], f"{label}.candidate_tokens"
                ),
                "row_indices": _integers(owner["row_indices"], f"{label}.row_indices"),
                "declaration_line": _integer(
                    owner["declaration_line"], f"{label}.declaration_line", minimum=1
                ),
                "definition_lines": _integers(
                    owner["definition_lines"], f"{label}.definition_lines"
                ),
                "use_lines": use_lines,
                "call_boundaries": _strings(
                    owner["call_boundaries"], f"{label}.call_boundaries"
                ),
                "write_only_target_observed": write_only,
            }
        )
    if not owners or len(owners) > 8:
        raise OwnerFlowInputError("context.owners must contain between one and eight owners")

    semantic_groups: list[dict[str, Any]] = []
    for index, raw_group in enumerate(
        _sequence(context["semantic_groups"], "context.semantic_groups")
    ):
        label = f"context.semantic_groups[{index}]"
        group = _closed(
            raw_group,
            label,
            required={"kind", "row_indices", "action", "evidence"},
        )
        kind = _text(group["kind"], f"{label}.kind", limit=32)
        if kind not in _SUPPORTED_SEMANTIC_KINDS:
            raise OwnerFlowInputError(f"{label}.kind is unsupported: {kind}")
        semantic_groups.append(
            {
                "kind": kind,
                "row_indices": _integers(group["row_indices"], f"{label}.row_indices"),
                "action": _text(group["action"], f"{label}.action", limit=512),
                "evidence": _text(group["evidence"], f"{label}.evidence", limit=512),
            }
        )

    return {
        "schema": CONTEXT_SCHEMA,
        "function": _text(context["function"], "context.function", limit=256),
        "focus_artifact_sha256": _sha256(
            context["focus_artifact_sha256"], "context.focus_artifact_sha256"
        ),
        "report_sha256": _sha256(context["report_sha256"], "context.report_sha256"),
        "source_sha256": _sha256(context["source_sha256"], "context.source_sha256"),
        "candidate_object_sha256": _sha256(
            context["candidate_object_sha256"], "context.candidate_object_sha256"
        ),
        "target_object_sha256": _sha256(
            context["target_object_sha256"], "context.target_object_sha256"
        ),
        "protected_siblings_zero_loss": True,
        "physical_relocations": parsed_physical,
        "owners": owners,
        "semantic_groups": semantic_groups,
    }


def _row_map(side: Mapping[str, Any], label: str) -> dict[int, Mapping[str, Any]]:
    rows = _sequence(side.get("rows"), f"{label}.rows")
    result: dict[int, Mapping[str, Any]] = {}
    for position, raw_row in enumerate(rows):
        row = _mapping(raw_row, f"{label}.rows[{position}]")
        if not row.get("diff_kind"):
            continue
        index = _integer(row.get("index"), f"{label}.rows[{position}].index")
        if index in result:
            raise OwnerFlowInputError(f"{label} has duplicate diff row index {index}")
        result[index] = row
    return result


def _formatted(row: Mapping[str, Any], label: str) -> str:
    instruction = _mapping(row.get("instruction"), f"{label}.instruction")
    return _text(instruction.get("formatted"), f"{label}.instruction.formatted", limit=512)


def _opcode(text: str) -> str:
    return text.split(None, 1)[0].lower()


def _memory(text: str) -> dict[str, Any] | None:
    match = _MEMORY_RE.fullmatch(text.strip())
    if match is None:
        return None
    return {
        "opcode": match.group("opcode").lower(),
        "value": match.group("value").strip().lower(),
        "offset": int(match.group("offset"), 0),
    }


def _register_shape(text: str) -> str:
    return _REGISTER_RE.sub("<reg>", text.lower())


def _numeric_token_delta(target: str, candidate: str) -> bool:
    target_tokens = [token for token in re.split(r"([,()\s]+)", target.lower()) if token]
    candidate_tokens = [token for token in re.split(r"([,()\s]+)", candidate.lower()) if token]
    if len(target_tokens) != len(candidate_tokens):
        return False
    differences = [
        (left, right)
        for left, right in zip(target_tokens, candidate_tokens)
        if left != right
    ]
    return len(differences) == 1 and all(_INTEGER_RE.fullmatch(item) for item in differences[0])


def _pool_owner(row: Mapping[str, Any], text: str) -> str | None:
    instruction = _mapping(row.get("instruction"), "row.instruction")
    relocation = instruction.get("relocation")
    if not isinstance(relocation, Mapping):
        return None
    if relocation.get("type_name") != "R_PPC_EMB_SDA21" and relocation.get("type") != 109:
        return None
    match = _POOL_RE.search(text)
    return match.group("owner") if match is not None else None


def _classify_pair(
    index: int,
    target_row: Mapping[str, Any],
    candidate_row: Mapping[str, Any],
) -> dict[str, Any]:
    target_text = _formatted(target_row, f"target[{index}]")
    candidate_text = _formatted(candidate_row, f"candidate[{index}]")
    target_opcode = _opcode(target_text)
    candidate_opcode = _opcode(candidate_text)
    record: dict[str, Any] = {
        "row_index": index,
        "target": target_text,
        "candidate": candidate_text,
        "target_opcode": target_opcode,
        "candidate_opcode": candidate_opcode,
    }
    if target_opcode != candidate_opcode:
        return {**record, "kind": "unclassified", "reason": "opcode_differs"}

    target_pool = _pool_owner(target_row, target_text)
    candidate_pool = _pool_owner(candidate_row, candidate_text)
    if target_pool is not None and candidate_pool is not None:
        return {
            **record,
            "kind": "pool_owner",
            "target_owner": target_pool,
            "candidate_owner": candidate_pool,
        }

    target_memory = _memory(target_text)
    candidate_memory = _memory(candidate_text)
    if target_memory is not None and candidate_memory is not None:
        if target_memory["opcode"] != candidate_memory["opcode"]:
            return {**record, "kind": "unclassified", "reason": "memory_opcode_differs"}
        if target_memory["offset"] != candidate_memory["offset"]:
            if target_memory["value"] != candidate_memory["value"]:
                return {**record, "kind": "unclassified", "reason": "home_and_value_both_differ"}
            return {
                **record,
                "kind": "stack_home",
                "candidate_token": f"stack:{candidate_memory['offset']:#x}",
                "target_token": f"stack:{target_memory['offset']:#x}",
                "access": "store" if candidate_opcode.startswith("st") else "load",
            }
        if target_memory["value"] != candidate_memory["value"]:
            return {**record, "kind": "register_flow", "access": "store" if candidate_opcode.startswith("st") else "load"}

    target_instruction = _mapping(target_row.get("instruction"), f"target[{index}].instruction")
    candidate_instruction = _mapping(candidate_row.get("instruction"), f"candidate[{index}].instruction")
    if target_instruction.get("branch_dest") != candidate_instruction.get("branch_dest") and (
        target_instruction.get("branch_dest") is not None
        or candidate_instruction.get("branch_dest") is not None
    ):
        return {**record, "kind": "branch_topology"}
    if _numeric_token_delta(target_text, candidate_text):
        return {**record, "kind": "immediate_semantic"}
    if _register_shape(target_text) == _register_shape(candidate_text):
        return {**record, "kind": "register_flow", "access": "register"}
    return {**record, "kind": "unclassified", "reason": "unsupported_argument_delta"}


def _components(classified: Sequence[Mapping[str, Any]]) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    stack_groups: dict[tuple[str, str], list[Mapping[str, Any]]] = defaultdict(list)
    register_rows: list[Mapping[str, Any]] = []
    semantic_rows: dict[str, list[Mapping[str, Any]]] = defaultdict(list)
    for row in classified:
        kind = row["kind"]
        if kind == "stack_home":
            stack_groups[(str(row["candidate_token"]), str(row["target_token"]))].append(row)
        elif kind == "register_flow":
            register_rows.append(row)
        elif kind in _SUPPORTED_SEMANTIC_KINDS:
            semantic_rows[str(kind)].append(row)

    flow_components: list[dict[str, Any]] = []
    for (candidate_token, target_token), rows in sorted(stack_groups.items()):
        row_indices = sorted(int(row["row_index"]) for row in rows)
        flow_components.append(
            {
                "id": f"stack-{candidate_token[6:]}-to-{target_token[6:]}",
                "kind": "stack_home",
                "candidate_token": candidate_token,
                "target_token": target_token,
                "row_indices": row_indices,
                "access_counts": dict(sorted(Counter(str(row["access"]) for row in rows).items())),
                "opcodes": sorted({str(row["candidate_opcode"]) for row in rows}),
            }
        )

    register_rows = sorted(register_rows, key=lambda row: int(row["row_index"]))
    contiguous: list[list[Mapping[str, Any]]] = []
    for row in register_rows:
        if not contiguous or int(row["row_index"]) != int(contiguous[-1][-1]["row_index"]) + 1:
            contiguous.append([row])
        else:
            contiguous[-1].append(row)
    for rows in contiguous:
        row_indices = [int(row["row_index"]) for row in rows]
        token = f"register-flow:{row_indices[0]}-{row_indices[-1]}"
        flow_components.append(
            {
                "id": token,
                "kind": "register_flow",
                "candidate_token": token,
                "target_token": token,
                "row_indices": row_indices,
                "access_counts": dict(sorted(Counter(str(row["access"]) for row in rows).items())),
                "opcodes": sorted({str(row["candidate_opcode"]) for row in rows}),
            }
        )

    semantic_components: list[dict[str, Any]] = []
    for kind, rows in sorted(semantic_rows.items()):
        record: dict[str, Any] = {
            "kind": kind,
            "row_indices": sorted(int(row["row_index"]) for row in rows),
        }
        if kind == "pool_owner":
            record["target_owners"] = sorted({str(row["target_owner"]) for row in rows})
            record["candidate_owners"] = sorted({str(row["candidate_owner"]) for row in rows})
        semantic_components.append(record)
    return flow_components, semantic_components


def _assignment_cost(owner: Mapping[str, Any], component: Mapping[str, Any]) -> int:
    if owner["kind"] != component["kind"]:
        return _INF
    candidate_tokens = set(owner["candidate_tokens"])
    if candidate_tokens and component["candidate_token"] not in candidate_tokens:
        return _INF
    owner_rows = set(owner["row_indices"])
    component_rows = set(component["row_indices"])
    if not owner_rows & component_rows:
        return _INF
    return 10 * len(owner_rows ^ component_rows)


def _solve_assignment(
    owners: Sequence[Mapping[str, Any]], components: Sequence[Mapping[str, Any]]
) -> tuple[list[dict[str, Any]], list[str]]:
    blockers: list[str] = []
    if len(owners) != len(components):
        return [], [f"owner_component_count_mismatch:{len(owners)}:{len(components)}"]
    solutions: list[tuple[int, tuple[int, ...]]] = []
    for permutation in itertools.permutations(range(len(components))):
        costs = [_assignment_cost(owner, components[index]) for owner, index in zip(owners, permutation)]
        if any(cost >= _INF for cost in costs):
            continue
        solutions.append((sum(costs), permutation))
    if not solutions:
        return [], ["owner_flow_assignment_missing"]
    solutions.sort(key=lambda item: (item[0], item[1]))
    best_cost = solutions[0][0]
    best = [item for item in solutions if item[0] == best_cost]
    if len(best) != 1:
        return [], [f"owner_flow_assignment_ambiguous:{len(best)}"]
    mappings: list[dict[str, Any]] = []
    for owner, component_index in zip(owners, best[0][1]):
        component = components[component_index]
        mappings.append(
            {
                "owner_id": owner["id"],
                "owner_kind": owner["kind"],
                "source_type": owner["source_type"],
                "candidate_token": component["candidate_token"],
                "target_token": component["target_token"],
                "row_indices": component["row_indices"],
                "access_counts": component["access_counts"],
                "opcodes": component["opcodes"],
                "assignment_cost": _assignment_cost(owner, component),
                "source_evidence": {
                    "declaration_line": owner["declaration_line"],
                    "definition_lines": owner["definition_lines"],
                    "use_lines": owner["use_lines"],
                    "call_boundaries": owner["call_boundaries"],
                    "write_only_target_observed": owner["write_only_target_observed"],
                },
            }
        )
    return sorted(mappings, key=lambda item: (item["owner_kind"], item["owner_id"])), blockers


def _owner_cycles(mappings: Sequence[Mapping[str, Any]]) -> tuple[list[list[str]], list[str]]:
    stack = [mapping for mapping in mappings if mapping["owner_kind"] == "stack_home"]
    by_candidate = {str(mapping["candidate_token"]): str(mapping["owner_id"]) for mapping in stack}
    successor: dict[str, str] = {}
    blockers: list[str] = []
    for mapping in stack:
        owner_id = str(mapping["owner_id"])
        target_token = str(mapping["target_token"])
        target_owner = by_candidate.get(target_token)
        if target_owner is None:
            blockers.append(f"target_home_unbound:{owner_id}:{target_token}")
            continue
        successor[owner_id] = target_owner

    cycles: list[list[str]] = []
    visited: set[str] = set()
    for start in sorted(successor):
        if start in visited:
            continue
        trail: list[str] = []
        positions: dict[str, int] = {}
        current = start
        while current not in visited and current in successor:
            positions[current] = len(trail)
            trail.append(current)
            current = successor[current]
            if current in positions:
                cycle = trail[positions[current] :]
                if len(cycle) > 1:
                    smallest = min(range(len(cycle)), key=lambda index: cycle[index])
                    cycles.append(cycle[smallest:] + cycle[:smallest])
                break
        visited.update(trail)
    return sorted(cycles), blockers


def build_diagnosis(focus: Mapping[str, Any], context_value: Mapping[str, Any]) -> dict[str, Any]:
    context = _parse_context(context_value)
    blockers: list[str] = []
    warnings: list[str] = []
    if focus.get("schema") != "focus_symbol_report/v1":
        raise OwnerFlowInputError("focus artifact schema must be focus_symbol_report/v1")
    if focus.get("function") != context["function"]:
        raise OwnerFlowInputError("focus function does not match context.function")
    if focus.get("artifact_sha256") != context["focus_artifact_sha256"]:
        raise OwnerFlowInputError("focus artifact identity does not match context binding")
    if focus.get("authority_advanced") is not False:
        raise OwnerFlowInputError("focus artifact must remain authority-free")

    channels = _mapping(focus.get("channels"), "focus.channels")
    strict = _mapping(channels.get("strict"), "focus.channels.strict")
    data = _mapping(channels.get("data"), "focus.channels.data")
    strict_target_side = _mapping(strict.get("target"), "focus.channels.strict.target")
    strict_candidate_side = _mapping(strict.get("candidate"), "focus.channels.strict.candidate")
    data_target_side = _mapping(data.get("target"), "focus.channels.data.target")
    data_candidate_side = _mapping(data.get("candidate"), "focus.channels.data.candidate")
    strict_target = _row_map(strict_target_side, "strict.target")
    strict_candidate = _row_map(strict_candidate_side, "strict.candidate")
    data_target = _row_map(data_target_side, "data.target")
    data_candidate = _row_map(data_candidate_side, "data.candidate")
    if set(strict_target) != set(strict_candidate):
        blockers.append("strict_target_candidate_diff_rows_differ")
    if set(data_target) != set(data_candidate):
        blockers.append("data_target_candidate_diff_rows_differ")

    metric = _mapping(strict.get("metric"), "focus.channels.strict.metric")
    target_size = _integer(metric.get("target_size"), "strict.metric.target_size")
    candidate_size = _integer(metric.get("candidate_size"), "strict.metric.candidate_size")
    if target_size != candidate_size:
        blockers.append("function_size_not_exact")
    if len(_sequence(strict_target_side.get("rows"), "strict.target.rows")) != len(
        _sequence(strict_candidate_side.get("rows"), "strict.candidate.rows")
    ):
        blockers.append("instruction_count_not_exact")
    if any(row.get("diff_kind") != "DIFF_ARG_MISMATCH" for row in strict_target.values()):
        blockers.append("strict_residual_not_argument_only")

    classified: list[dict[str, Any]] = []
    for index in sorted(set(strict_target) & set(strict_candidate)):
        classified.append(_classify_pair(index, strict_target[index], strict_candidate[index]))
    unclassified = [row for row in classified if row["kind"] == "unclassified"]
    if unclassified:
        blockers.extend(f"row_{row['row_index']}_unclassified:{row['reason']}" for row in unclassified)

    flow_components, semantic_components = _components(classified)
    mappings, assignment_blockers = _solve_assignment(context["owners"], flow_components)
    blockers.extend(assignment_blockers)
    cycles, cycle_blockers = _owner_cycles(mappings)
    blockers.extend(cycle_blockers)

    extracted_by_kind = {component["kind"]: component for component in semantic_components}
    context_by_kind: dict[str, Mapping[str, Any]] = {}
    for group in context["semantic_groups"]:
        if group["kind"] in context_by_kind:
            blockers.append(f"duplicate_context_semantic_group:{group['kind']}")
        context_by_kind[group["kind"]] = group
    for kind in sorted(set(extracted_by_kind) | set(context_by_kind)):
        extracted = extracted_by_kind.get(kind)
        expected = context_by_kind.get(kind)
        if extracted is None:
            blockers.append(f"semantic_group_not_observed:{kind}")
        elif expected is None:
            blockers.append(f"semantic_group_not_bound:{kind}")
        elif extracted["row_indices"] != expected["row_indices"]:
            blockers.append(f"semantic_group_rows_differ:{kind}")

    covered_rows = {
        row
        for mapping in mappings
        for row in mapping["row_indices"]
    } | {
        row
        for group in context["semantic_groups"]
        for row in group["row_indices"]
    }
    strict_rows = set(strict_target)
    missing_rows = sorted(strict_rows - covered_rows)
    extra_rows = sorted(covered_rows - strict_rows)
    if missing_rows:
        blockers.append("strict_rows_uncovered:" + ",".join(map(str, missing_rows)))
    if extra_rows:
        blockers.append("context_rows_not_in_strict_residual:" + ",".join(map(str, extra_rows)))
    if context["physical_relocations"]["status"] == "unknown":
        warnings.append("physical_relocation_authority_unknown")

    matched = not blockers
    bound_semantic_groups: list[dict[str, Any]] = []
    if matched:
        for component in semantic_components:
            expected = context_by_kind[component["kind"]]
            bound_semantic_groups.append(
                {
                    **component,
                    "action": expected["action"],
                    "evidence": expected["evidence"],
                }
            )

    candidate_cells: list[dict[str, Any]] = []
    if matched and cycles:
        candidate_cells.append(
            {
                "id": "close_owner_flow_cycles",
                "ordinal": 1,
                "action": (
                    "Compile one natural declaration/lifetime-boundary cell covering only the bound "
                    "owner cycles; do not probe unrelated declarations independently."
                ),
                "owner_cycles": cycles,
                "compile_candidate_limit": 1,
                "source_patch_emitted": False,
            }
        )
    if matched and (bound_semantic_groups or any(mapping["owner_kind"] == "register_flow" for mapping in mappings)):
        candidate_cells.append(
            {
                "id": "compose_remaining_semantic_closure",
                "ordinal": len(candidate_cells) + 1,
                "action": (
                    "After the home cycle is exact, compile one composed cell containing only the bound "
                    "pool, immediate, branch, and live result-flow actions."
                ),
                "semantic_actions": [group["action"] for group in bound_semantic_groups],
                "register_flow_owners": [
                    mapping["owner_id"] for mapping in mappings if mapping["owner_kind"] == "register_flow"
                ],
                "compile_candidate_limit": 1,
                "source_patch_emitted": False,
            }
        )

    result: dict[str, Any] = {
        "schema": SCHEMA,
        "status": "matched" if matched else "blocked",
        "route": ROUTE if matched else None,
        "function": context["function"],
        "input_binding": {
            "focus_artifact_sha256": context["focus_artifact_sha256"],
            "report_sha256": context["report_sha256"],
            "source_sha256": context["source_sha256"],
            "candidate_object_sha256": context["candidate_object_sha256"],
            "target_object_sha256": context["target_object_sha256"],
            "physical_relocations": context["physical_relocations"],
            "protected_siblings_zero_loss": True,
        },
        "facts": {
            "target_size": target_size,
            "candidate_size": candidate_size,
            "strict_diff_row_count": len(strict_target),
            "data_diff_row_count": len(data_target),
            "classification_counts": dict(sorted(Counter(row["kind"] for row in classified).items())),
            "flow_component_count": len(flow_components),
            "semantic_component_count": len(semantic_components),
            "owner_mapping_count": len(mappings),
            "owner_cycle_count": len(cycles),
            "all_strict_rows_accounted": not missing_rows and not extra_rows,
        },
        "classified_rows": classified if matched else [],
        "flow_components": flow_components if matched else [],
        "owner_mappings": mappings if matched else [],
        "owner_cycles": cycles if matched else [],
        "semantic_groups": bound_semantic_groups if matched else [],
        "candidate_cells": candidate_cells,
        "compile_candidate_budget": sum(cell["compile_candidate_limit"] for cell in candidate_cells),
        "analysis_deadline_minutes": 5,
        "trace_budget": 0 if matched else 1,
        "warnings": warnings,
        "blockers": blockers,
        "suppressed_axes": [
            "unbound_declaration_permutation",
            "scope_permutation",
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "tracing_without_a_changed_proof_obligation",
        ],
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["diagnosis_sha256"] = owner_manifest.canonical_sha256(result)
    return result


def build_from_paths(
    *,
    focus_path: Path,
    context_path: Path,
    expected_focus_file_sha256: str,
    expected_context_file_sha256: str,
) -> dict[str, Any]:
    expected_focus = _sha256(expected_focus_file_sha256, "expected_focus_file_sha256")
    expected_context = _sha256(expected_context_file_sha256, "expected_context_file_sha256")
    actual_focus = owner_manifest.file_sha256(focus_path)
    actual_context = owner_manifest.file_sha256(context_path)
    mismatches = []
    if expected_focus != actual_focus:
        mismatches.append("focus")
    if expected_context != actual_context:
        mismatches.append("context")
    if mismatches:
        raise OwnerFlowInputError("evidence hash mismatch: " + ", ".join(mismatches))
    focus = owner_manifest.load_json(focus_path, "focus artifact")
    context = owner_manifest.load_json(context_path, "owner-flow context")
    return build_diagnosis(focus, context)


def _atomic_write(path: Path, text: str) -> None:
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        descriptor, temporary_name = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
        temporary = Path(temporary_name)
        try:
            with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
                stream.write(text)
            temporary.replace(path)
        except BaseException:
            temporary.unlink(missing_ok=True)
            raise
    except OSError as exc:
        raise OwnerFlowInputError(f"cannot write {path}: {exc}") from exc


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("focus_artifact", type=Path)
    parser.add_argument("context", type=Path)
    parser.add_argument("--expect-focus-file-sha256", required=True)
    parser.add_argument("--expect-context-file-sha256", required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--require-match", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _parser()
    args = parser.parse_args(argv)
    try:
        result = build_from_paths(
            focus_path=args.focus_artifact,
            context_path=args.context,
            expected_focus_file_sha256=args.expect_focus_file_sha256,
            expected_context_file_sha256=args.expect_context_file_sha256,
        )
        rendered = json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
        if args.output is None:
            print(rendered, end="")
        else:
            _atomic_write(args.output, rendered)
    except (OwnerFlowInputError, owner_manifest.TypedPoolManifestInputError) as exc:
        parser.error(str(exc))
    if args.require_match and result["status"] != "matched":
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
