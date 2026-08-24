#!/usr/bin/env python3
"""Plan bounded factorial matching experiments without compiling or editing source.

The planner turns independently justified source axes into a complete Cartesian
batch.  It prevents the common failure mode where two individually neutral
axes are tested serially but their interaction is never compiled.  Cells are
deduplicated only by an explicit normalized topology token or by authenticated
source/object hashes supplied as observations; labels are never treated as
semantic equivalence.

This is a read-only planning tool.  It does not generate source, compile,
record candidates, mutate a workbench, or advance recovery authority.
"""

from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import os
import re
import stat
from collections import defaultdict
from pathlib import Path
from typing import Any, Mapping, Sequence


REQUEST_SCHEMA = "candidate_interaction_request/v1"
PLAN_SCHEMA = "candidate_interaction_plan/v1"
MAX_REQUEST_BYTES = 1024 * 1024
MAX_AXES = 8
MAX_LEVELS_PER_AXIS = 8
DEFAULT_MAX_CELLS = 256
ABSOLUTE_MAX_CELLS = 4096
SAFE_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}")
SHA256_RE = re.compile(r"[0-9a-f]{64}")
ADMISSIBILITY = frozenset({"natural", "diagnostic-only", "blocked"})


class InteractionPlanError(ValueError):
    """Malformed, ambiguous, or unsafe factorial planning input."""


def _fail(message: str) -> None:
    raise InteractionPlanError(message)


def _pairs(items: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in items:
        if key in result:
            _fail(f"duplicate JSON key {key!r}")
        result[key] = value
    return result


def _canonical(value: Any) -> bytes:
    try:
        return (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise InteractionPlanError(f"cannot serialize canonical JSON: {exc}") from exc


def _digest(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _closed(
    value: Any,
    *,
    allowed: set[str] | frozenset[str],
    required: set[str] | frozenset[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        _fail(f"{label} must be an object")
    unknown = sorted(set(value) - set(allowed))
    if unknown:
        _fail(f"{label} contains unknown field {unknown[0]!r}")
    missing = sorted(set(required) - set(value))
    if missing:
        _fail(f"{label} lacks required field {missing[0]!r}")
    return value


def _text(value: Any, label: str, *, limit: int = 2048) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        _fail(f"{label} must be a non-empty string")
    result = value.strip()
    if len(result) > limit:
        _fail(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if SAFE_ID_RE.fullmatch(result) is None:
        _fail(f"{label} must use 1-128 letters, digits, dot, underscore, or dash")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if SHA256_RE.fullmatch(result) is None:
        _fail(f"{label} must be a lowercase SHA-256 digest")
    return result


def _array(value: Any, label: str, *, minimum: int = 0, maximum: int) -> list[Any]:
    if not isinstance(value, list):
        _fail(f"{label} must be an array")
    if len(value) < minimum:
        _fail(f"{label} must contain at least {minimum} entries")
    if len(value) > maximum:
        _fail(f"{label} must contain at most {maximum} entries")
    return value


def _unique(values: Sequence[str], label: str) -> None:
    if len(set(values)) != len(values):
        _fail(f"{label} must be unique")


def _string_array(value: Any, label: str, *, maximum: int = 16) -> list[str]:
    raw = _array(value, label, minimum=1, maximum=maximum)
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(raw)]
    _unique(result, label)
    return result


def _load_request(path: Path) -> tuple[Mapping[str, Any], str]:
    absolute = Path(os.path.abspath(path))
    try:
        info = absolute.stat()
    except FileNotFoundError as exc:
        raise InteractionPlanError(f"interaction request does not exist: {absolute}") from exc
    if not stat.S_ISREG(info.st_mode):
        _fail("interaction request must be a regular file")
    if info.st_size > MAX_REQUEST_BYTES:
        _fail(f"interaction request exceeds {MAX_REQUEST_BYTES} bytes")
    try:
        raw = absolute.read_bytes()
        value = json.loads(raw.decode("utf-8"), object_pairs_hook=_pairs)
    except UnicodeDecodeError as exc:
        raise InteractionPlanError("interaction request is not UTF-8") from exc
    except json.JSONDecodeError as exc:
        raise InteractionPlanError(
            f"invalid interaction request JSON {exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    if not isinstance(value, Mapping):
        _fail("interaction request must be an object")
    return value, hashlib.sha256(raw).hexdigest()


def _selection_key(selection: Mapping[str, str]) -> tuple[tuple[str, str], ...]:
    return tuple(sorted(selection.items()))


def _cell_id(selection: Mapping[str, str]) -> str:
    return f"cell-{_digest(dict(sorted(selection.items())))[:16]}"


def _topology_group_id(topology: Sequence[tuple[str, str]]) -> str:
    return f"topology-{_digest(list(topology))[:16]}"


def _parse_request(value: Mapping[str, Any]) -> dict[str, Any]:
    request = _closed(
        value,
        allowed={
            "schema",
            "planner_id",
            "focus_symbols",
            "axes",
            "constraints",
            "observations",
            "max_cells",
        },
        required={"schema", "planner_id", "focus_symbols", "axes"},
        label="interaction request",
    )
    if _text(request.get("schema"), "interaction request schema") != REQUEST_SCHEMA:
        _fail(f"interaction request schema must be {REQUEST_SCHEMA}")
    planner_id = _identifier(request.get("planner_id"), "planner_id")
    focus_symbols = sorted(_string_array(request.get("focus_symbols"), "focus_symbols"))
    max_cells_value = request.get("max_cells", DEFAULT_MAX_CELLS)
    if isinstance(max_cells_value, bool) or not isinstance(max_cells_value, int):
        _fail("max_cells must be an integer")
    if not 1 <= max_cells_value <= ABSOLUTE_MAX_CELLS:
        _fail(f"max_cells must be between 1 and {ABSOLUTE_MAX_CELLS}")

    raw_axes = _array(request.get("axes"), "axes", minimum=2, maximum=MAX_AXES)
    axes: list[dict[str, Any]] = []
    for axis_index, raw_axis in enumerate(raw_axes):
        axis = _closed(
            raw_axis,
            allowed={"id", "hypothesis", "control_level", "levels"},
            required={"id", "hypothesis", "control_level", "levels"},
            label=f"axes[{axis_index}]",
        )
        axis_id = _identifier(axis.get("id"), f"axes[{axis_index}].id")
        hypothesis = _text(axis.get("hypothesis"), f"axes[{axis_index}].hypothesis")
        control_level = _identifier(
            axis.get("control_level"), f"axes[{axis_index}].control_level"
        )
        raw_levels = _array(
            axis.get("levels"),
            f"axes[{axis_index}].levels",
            minimum=2,
            maximum=MAX_LEVELS_PER_AXIS,
        )
        levels: list[dict[str, Any]] = []
        for level_index, raw_level in enumerate(raw_levels):
            level = _closed(
                raw_level,
                allowed={
                    "id",
                    "topology_token",
                    "source_action",
                    "evidence",
                    "admissibility",
                },
                required={
                    "id",
                    "topology_token",
                    "source_action",
                    "evidence",
                    "admissibility",
                },
                label=f"axes[{axis_index}].levels[{level_index}]",
            )
            admissibility = _text(
                level.get("admissibility"),
                f"axes[{axis_index}].levels[{level_index}].admissibility",
            )
            if admissibility not in ADMISSIBILITY:
                _fail(
                    f"axes[{axis_index}].levels[{level_index}].admissibility must be one of "
                    + ", ".join(sorted(ADMISSIBILITY))
                )
            levels.append(
                {
                    "id": _identifier(
                        level.get("id"), f"axes[{axis_index}].levels[{level_index}].id"
                    ),
                    "topology_token": _identifier(
                        level.get("topology_token"),
                        f"axes[{axis_index}].levels[{level_index}].topology_token",
                    ),
                    "source_action": _text(
                        level.get("source_action"),
                        f"axes[{axis_index}].levels[{level_index}].source_action",
                    ),
                    "evidence": _string_array(
                        level.get("evidence"),
                        f"axes[{axis_index}].levels[{level_index}].evidence",
                    ),
                    "admissibility": admissibility,
                }
            )
        level_ids = [level["id"] for level in levels]
        _unique(level_ids, f"axis {axis_id} level ids")
        if control_level not in level_ids:
            _fail(f"axis {axis_id} control_level does not name one of its levels")
        axes.append(
            {
                "id": axis_id,
                "hypothesis": hypothesis,
                "control_level": control_level,
                "levels": sorted(levels, key=lambda item: item["id"]),
            }
        )
    axis_ids = [axis["id"] for axis in axes]
    _unique(axis_ids, "axis ids")
    axes.sort(key=lambda item: item["id"])
    axis_levels = {
        axis["id"]: {level["id"] for level in axis["levels"]} for axis in axes
    }

    product_size = 1
    for axis in axes:
        product_size *= len(axis["levels"])
    if product_size > max_cells_value:
        _fail(
            f"factorial product has {product_size} cells, exceeding max_cells={max_cells_value}"
        )

    constraints: list[dict[str, Any]] = []
    for index, raw_constraint in enumerate(
        _array(request.get("constraints", []), "constraints", maximum=256)
    ):
        constraint = _closed(
            raw_constraint,
            allowed={"when", "reason"},
            required={"when", "reason"},
            label=f"constraints[{index}]",
        )
        when = constraint.get("when")
        if not isinstance(when, dict) or not when:
            _fail(f"constraints[{index}].when must be a non-empty object")
        normalized_when: dict[str, str] = {}
        for raw_axis_id, raw_level_id in when.items():
            axis_id = _identifier(raw_axis_id, f"constraints[{index}].when axis")
            level_id = _identifier(raw_level_id, f"constraints[{index}].when.{axis_id}")
            if axis_id not in axis_levels or level_id not in axis_levels[axis_id]:
                _fail(f"constraints[{index}] references an unknown axis or level")
            normalized_when[axis_id] = level_id
        constraints.append(
            {
                "when": dict(sorted(normalized_when.items())),
                "reason": _text(constraint.get("reason"), f"constraints[{index}].reason"),
            }
        )
    constraint_keys = [_canonical(item) for item in constraints]
    _unique([item.hex() for item in constraint_keys], "constraints")

    observations: list[dict[str, Any]] = []
    for index, raw_observation in enumerate(
        _array(request.get("observations", []), "observations", maximum=max_cells_value)
    ):
        observation = _closed(
            raw_observation,
            allowed={"selection", "candidate_id", "source_sha256", "object_sha256"},
            required={"selection", "candidate_id", "source_sha256", "object_sha256"},
            label=f"observations[{index}]",
        )
        selection = observation.get("selection")
        if not isinstance(selection, dict) or set(selection) != set(axis_levels):
            _fail(f"observations[{index}].selection must name every axis exactly once")
        normalized_selection: dict[str, str] = {}
        for axis_id in sorted(axis_levels):
            level_id = _identifier(
                selection.get(axis_id), f"observations[{index}].selection.{axis_id}"
            )
            if level_id not in axis_levels[axis_id]:
                _fail(f"observations[{index}] references an unknown level")
            normalized_selection[axis_id] = level_id
        observations.append(
            {
                "selection": normalized_selection,
                "candidate_id": _identifier(
                    observation.get("candidate_id"), f"observations[{index}].candidate_id"
                ),
                "source_sha256": _sha256(
                    observation.get("source_sha256"), f"observations[{index}].source_sha256"
                ),
                "object_sha256": _sha256(
                    observation.get("object_sha256"), f"observations[{index}].object_sha256"
                ),
            }
        )
    _unique([item["candidate_id"] for item in observations], "observation candidate ids")
    _unique(
        [json.dumps(item["selection"], sort_keys=True) for item in observations],
        "observation selections",
    )
    observations.sort(key=lambda item: _selection_key(item["selection"]))
    return {
        "planner_id": planner_id,
        "focus_symbols": focus_symbols,
        "axes": axes,
        "constraints": constraints,
        "observations": observations,
        "max_cells": max_cells_value,
        "raw_cell_count": product_size,
    }


def _constraint_reason(
    selection: Mapping[str, str], constraints: Sequence[Mapping[str, Any]]
) -> str | None:
    reasons = [
        str(constraint["reason"])
        for constraint in constraints
        if all(selection.get(axis) == level for axis, level in constraint["when"].items())
    ]
    return "; ".join(sorted(reasons)) if reasons else None


def build_interaction_plan(request_path: Path | str) -> dict[str, Any]:
    raw_request, request_sha256 = _load_request(Path(request_path))
    request = _parse_request(raw_request)
    axes = request["axes"]
    level_maps = {
        axis["id"]: {level["id"]: level for level in axis["levels"]} for axis in axes
    }
    observation_by_selection = {
        _selection_key(item["selection"]): item for item in request["observations"]
    }

    cells: list[dict[str, Any]] = []
    for chosen in itertools.product(*(axis["levels"] for axis in axes)):
        selection = {axis["id"]: level["id"] for axis, level in zip(axes, chosen)}
        topology = tuple(
            (axis["id"], level["topology_token"]) for axis, level in zip(axes, chosen)
        )
        interaction_order = sum(
            selection[axis["id"]] != axis["control_level"] for axis in axes
        )
        constraint_reason = _constraint_reason(selection, request["constraints"])
        blocked_levels = [
            axis["id"]
            for axis, level in zip(axes, chosen)
            if level["admissibility"] == "blocked"
        ]
        observation = observation_by_selection.get(_selection_key(selection))
        cells.append(
            {
                "cell_id": _cell_id(selection),
                "selection": dict(sorted(selection.items())),
                "interaction_order": interaction_order,
                "topology_key": [list(item) for item in topology],
                "topology_group_id": _topology_group_id(topology),
                "source_actions": {
                    axis["id"]: level["source_action"] for axis, level in zip(axes, chosen)
                },
                "admissibility": {
                    axis["id"]: level["admissibility"] for axis, level in zip(axes, chosen)
                },
                "blocked_reason": (
                    constraint_reason
                    or (
                        "blocked source level on " + ", ".join(sorted(blocked_levels))
                        if blocked_levels
                        else None
                    )
                ),
                "observation": observation,
            }
        )
    cells.sort(
        key=lambda item: (
            item["interaction_order"],
            tuple(item["selection"].items()),
        )
    )

    topology_groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for cell in cells:
        topology_groups[cell["topology_group_id"]].append(cell)
    for members in topology_groups.values():
        members.sort(
            key=lambda item: (
                item["blocked_reason"] is not None,
                item["observation"] is None,
                item["interaction_order"],
                tuple(item["selection"].items()),
            )
        )
        canonical = members[0]
        for cell in members:
            cell["topology_canonical_cell_id"] = canonical["cell_id"]
            cell["topology_duplicate_of"] = (
                None if cell is canonical else canonical["cell_id"]
            )

    source_first: dict[str, str] = {}
    object_first: dict[str, str] = {}
    observed_cells = [cell for cell in cells if cell["observation"] is not None]
    for cell in observed_cells:
        observation = cell["observation"]
        source_hash = str(observation["source_sha256"])
        object_hash = str(observation["object_sha256"])
        cell["duplicate_source_of"] = source_first.get(source_hash)
        cell["duplicate_object_of"] = object_first.get(object_hash)
        source_first.setdefault(source_hash, str(observation["candidate_id"]))
        object_first.setdefault(object_hash, str(observation["candidate_id"]))

    for cell in cells:
        if cell["blocked_reason"] is not None:
            action = "blocked"
        elif cell["topology_duplicate_of"] is not None:
            action = "skip_duplicate_topology"
        elif cell["observation"] is not None:
            action = "reuse_measured_candidate"
        else:
            action = "generate_and_compile"
        cell["action"] = action
        cell.pop("topology_key")

    runnable = [cell["cell_id"] for cell in cells if cell["action"] == "generate_and_compile"]
    batches = []
    for interaction_order in sorted({cell["interaction_order"] for cell in cells}):
        batch_cells = [
            cell["cell_id"]
            for cell in cells
            if cell["interaction_order"] == interaction_order
            and cell["action"] == "generate_and_compile"
        ]
        if batch_cells:
            batches.append(
                {
                    "interaction_order": interaction_order,
                    "kind": (
                        "control"
                        if interaction_order == 0
                        else "single_axis"
                        if interaction_order == 1
                        else "interaction"
                    ),
                    "cell_ids": batch_cells,
                }
            )

    output_cells = []
    for cell in cells:
        observation = cell["observation"]
        output_cells.append(
            {
                "cell_id": cell["cell_id"],
                "selection": cell["selection"],
                "interaction_order": cell["interaction_order"],
                "topology_group_id": cell["topology_group_id"],
                "topology_canonical_cell_id": cell["topology_canonical_cell_id"],
                "topology_duplicate_of": cell["topology_duplicate_of"],
                "source_actions": cell["source_actions"],
                "admissibility": cell["admissibility"],
                "blocked_reason": cell["blocked_reason"],
                "action": cell["action"],
                "observation": (
                    {
                        **observation,
                        "duplicate_source_of": cell.get("duplicate_source_of"),
                        "duplicate_object_of": cell.get("duplicate_object_of"),
                    }
                    if observation is not None
                    else None
                ),
            }
        )

    body = {
        "schema": PLAN_SCHEMA,
        "schema_version": 1,
        "planner_id": request["planner_id"],
        "request_sha256": request_sha256,
        "focus_symbols": request["focus_symbols"],
        "axes": axes,
        "summary": {
            "raw_cell_count": len(cells),
            "unique_topology_count": len(topology_groups),
            "topology_duplicate_count": sum(
                cell["topology_duplicate_of"] is not None for cell in cells
            ),
            "blocked_cell_count": sum(cell["action"] == "blocked" for cell in cells),
            "observed_cell_count": len(observed_cells),
            "generate_and_compile_count": len(runnable),
            "source_duplicate_observation_count": sum(
                cell.get("duplicate_source_of") is not None for cell in observed_cells
            ),
            "object_duplicate_observation_count": sum(
                cell.get("duplicate_object_of") is not None for cell in observed_cells
            ),
        },
        "batches": batches,
        "recommended_execution_order": runnable,
        "cells": output_cells,
        "production_modified": False,
        "authority_advanced": False,
        "limitations": [
            "Topology equivalence is accepted only from explicit topology tokens or authenticated hashes.",
            "Object equality proves code-generation equality, not source provenance or semantic equivalence.",
            "The planner does not generate source, compile, record candidates, or authorize retention.",
        ],
    }
    return {**body, "interaction_plan_sha256": _digest(body)}


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("request")
    parser.add_argument("--json", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    try:
        result = build_interaction_plan(args.request)
    except (InteractionPlanError, OSError) as exc:
        print(f"error: {exc}")
        return 2
    print(json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
