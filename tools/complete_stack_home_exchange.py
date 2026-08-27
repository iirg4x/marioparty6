#!/usr/bin/env python3
"""Detect a complete r1-relative stack-home exchange before source probing.

The classifier is deliberately fail-closed.  It matches only exact-size
functions whose entire non-pool strict residual consists of same-address,
same-opcode instructions with one changed r1-relative stack displacement.
The observed candidate-to-target home mapping must be consistent and
bijective, and the residual must contain both stores and later consumers.

A match is diagnostic only.  It ranks one authenticated aggregate/capacity
donor composition before declaration or scope permutations, then hands any
remaining pool rows to the typed-pool decoder.  It never emits C, retains a
candidate, or advances recovery authority.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import pool_reloc_summary as pool_decoder
from tools import typed_pool_owner_manifest as owner_manifest


SCHEMA = "complete_stack_home_exchange/v1"
ROUTE = "authenticated_aggregate_donor_home_exchange_then_typed_pool"
MIN_STACK_ROWS = 8
_EXACT_POOL_CLASSES = {"exact_pool_contract", "mapped_pool_contract"}
_OFFSET = r"(?P<offset>[+-]?(?:0x[0-9a-f]+|[0-9]+))"
_STACK_MEMORY_RE = re.compile(
    rf"^(?P<opcode>lbz|lhz|lha|lwz|lfs|lfd|stb|sth|stw|stfs|stfd)\s+"
    rf"(?P<value>[^,]+),\s*{_OFFSET}\(r1\)$",
    re.IGNORECASE,
)
_STACK_ADDRESS_RE = re.compile(
    rf"^(?P<opcode>addi)\s+(?P<value>[^,]+),\s*r1,\s*{_OFFSET}$",
    re.IGNORECASE,
)


def _sequence(value: Any) -> list[Any]:
    return list(value) if isinstance(value, Sequence) and not isinstance(value, (str, bytes)) else []


def _diff_kind(row: Any) -> str | None:
    if not isinstance(row, Mapping):
        return None
    value = row.get("diff_kind")
    if isinstance(value, str) and value:
        return value
    instruction = row.get("instruction")
    if isinstance(instruction, Mapping):
        nested = instruction.get("diff_kind")
        if isinstance(nested, str) and nested:
            return nested
    return None


def _diff_indexes(row: Any) -> tuple[int, ...]:
    if not isinstance(row, Mapping):
        return ()
    result: list[int] = []
    for item in _sequence(row.get("arg_diff")):
        if not isinstance(item, Mapping) or "diff_index" not in item:
            continue
        value = item.get("diff_index")
        if isinstance(value, bool):
            continue
        try:
            result.append(int(value))
        except (TypeError, ValueError):
            continue
    return tuple(result)


def _instruction(row: Any) -> Mapping[str, Any]:
    if not isinstance(row, Mapping):
        return {}
    value = row.get("instruction")
    return value if isinstance(value, Mapping) else {}


def _stack_contract(row: Any) -> dict[str, Any] | None:
    instruction = _instruction(row)
    formatted = instruction.get("formatted")
    if not isinstance(formatted, str):
        return None
    text = formatted.strip()
    match = _STACK_MEMORY_RE.fullmatch(text)
    kind = "memory"
    if match is None:
        match = _STACK_ADDRESS_RE.fullmatch(text)
        kind = "address"
    if match is None:
        return None
    try:
        offset = int(match.group("offset"), 0)
    except ValueError:
        return None
    opcode = match.group("opcode").lower()
    return {
        "kind": kind,
        "opcode": opcode,
        "value_operand": match.group("value").strip().lower(),
        "offset": offset,
        "instruction_address": instruction.get("address"),
        "instruction_size": instruction.get("size", row.get("size") if isinstance(row, Mapping) else None),
        "formatted": text,
    }


def _float_percent(value: Any) -> float | None:
    if isinstance(value, bool):
        return None
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _nonexact_pool_pairs(
    left: Mapping[str, Any],
    right: Mapping[str, Any],
    target_function: Mapping[str, Any],
    candidate_function: Mapping[str, Any],
) -> list[dict[str, Any]]:
    target_rows = pool_decoder._pool_rows(left, target_function)
    candidate_rows = pool_decoder._pool_rows(right, candidate_function)
    pairs = [
        pool_decoder._pair_record(row, target_rows.get(row), candidate_rows.get(row))
        for row in sorted(set(target_rows) | set(candidate_rows))
    ]
    return [pair for pair in pairs if pair.get("classification") not in _EXACT_POOL_CLASSES]


def _extents(offsets: set[int]) -> list[dict[str, Any]]:
    if not offsets:
        return []
    ordered = sorted(offsets)
    groups: list[list[int]] = [[ordered[0]]]
    for offset in ordered[1:]:
        if offset == groups[-1][-1] + 4:
            groups[-1].append(offset)
        else:
            groups.append([offset])
    return [
        {
            "start": group[0],
            "start_hex": hex(group[0]),
            "end_inclusive": group[-1],
            "end_inclusive_hex": hex(group[-1]),
            "observed_word_count": len(group),
            "observed_span_bytes": group[-1] - group[0] + 4,
            "offsets": group,
        }
        for group in groups
    ]


def build_diagnosis(
    strict_report: Mapping[str, Any],
    data_report: Mapping[str, Any],
    function: str,
    binding: Mapping[str, Any],
) -> dict[str, Any]:
    function = owner_manifest._text(function, "function", limit=256)
    binding_value = owner_manifest._closed_binding(binding)
    try:
        strict_left, strict_right, strict_target, strict_candidate = pool_decoder._function_pair(
            strict_report, function
        )
        _, _, data_target, data_candidate = pool_decoder._function_pair(data_report, function)
    except pool_decoder.PoolDecodeError as exc:
        raise owner_manifest.TypedPoolManifestInputError(str(exc)) from exc

    target_rows = _sequence(strict_target.get("instructions"))
    candidate_rows = _sequence(strict_candidate.get("instructions"))
    residual_rows = owner_manifest._strict_residual_rows(strict_target, strict_candidate)
    pool_pairs = _nonexact_pool_pairs(strict_left, strict_right, strict_target, strict_candidate)
    pool_rows = sorted(int(pair["row"]) for pair in pool_pairs)
    pool_row_set = set(pool_rows)
    stack_rows = [row for row in residual_rows if row not in pool_row_set]

    target_size = pool_decoder._int(strict_target.get("size"))
    candidate_size = pool_decoder._int(strict_candidate.get("size"))
    data_target_size = pool_decoder._int(data_target.get("size"))
    data_candidate_size = pool_decoder._int(data_candidate.get("size"))
    data_percent = _float_percent(data_candidate.get("match_percent"))

    blockers: list[str] = []
    if target_size is None or target_size != candidate_size:
        blockers.append("function_size_not_exact")
    if len(target_rows) != len(candidate_rows):
        blockers.append("instruction_count_not_exact")
    if data_target_size is None or data_target_size != data_candidate_size:
        blockers.append("data_function_size_not_exact")
    if not stack_rows:
        blockers.append("no_non_pool_residual_rows")
    if 0 < len(stack_rows) < MIN_STACK_ROWS:
        blockers.append("stack_home_row_count_below_minimum")

    mapping: dict[int, int] = {}
    inverse: dict[int, int] = {}
    pair_rows: dict[tuple[int, int], list[int]] = defaultdict(list)
    row_records: list[dict[str, Any]] = []
    opcodes: set[str] = set()
    has_store = False
    has_load = False
    has_address = False

    for row in stack_rows:
        if row >= len(target_rows) or row >= len(candidate_rows):
            blockers.append(f"row_{row}_instruction_presence_mismatch")
            continue
        target_row = target_rows[row]
        candidate_row = candidate_rows[row]
        target = _stack_contract(target_row)
        candidate = _stack_contract(candidate_row)
        if target is None or candidate is None:
            blockers.append(f"row_{row}_not_supported_r1_stack_instruction")
            continue
        if _diff_kind(target_row) != "DIFF_ARG_MISMATCH" or _diff_kind(candidate_row) != "DIFF_ARG_MISMATCH":
            blockers.append(f"row_{row}_not_arg_only_mismatch")
        target_indexes = _diff_indexes(target_row)
        candidate_indexes = _diff_indexes(candidate_row)
        if len(target_indexes) != 1 or target_indexes != candidate_indexes:
            blockers.append(f"row_{row}_arg_diff_not_single_and_paired")
        if (
            target["kind"],
            target["opcode"],
            target["value_operand"],
        ) != (
            candidate["kind"],
            candidate["opcode"],
            candidate["value_operand"],
        ):
            blockers.append(f"row_{row}_instruction_contract_not_equal_except_stack_offset")
        if target["instruction_address"] != candidate["instruction_address"]:
            blockers.append(f"row_{row}_instruction_address_not_exact")
        if target["instruction_size"] != candidate["instruction_size"]:
            blockers.append(f"row_{row}_instruction_size_not_exact")
        target_offset = int(target["offset"])
        candidate_offset = int(candidate["offset"])
        if target_offset == candidate_offset:
            blockers.append(f"row_{row}_stack_offset_not_changed")
        if target_offset % 4 or candidate_offset % 4:
            blockers.append(f"row_{row}_stack_offset_not_word_aligned")
        previous_target = mapping.get(candidate_offset)
        if previous_target is not None and previous_target != target_offset:
            blockers.append(f"candidate_offset_{candidate_offset}_maps_multiple_targets")
        previous_candidate = inverse.get(target_offset)
        if previous_candidate is not None and previous_candidate != candidate_offset:
            blockers.append(f"target_offset_{target_offset}_maps_multiple_candidates")
        mapping[candidate_offset] = target_offset
        inverse[target_offset] = candidate_offset
        pair_rows[(candidate_offset, target_offset)].append(row)
        opcode = str(target["opcode"])
        opcodes.add(opcode)
        has_store = has_store or opcode.startswith("st")
        has_load = has_load or opcode.startswith("l")
        has_address = has_address or target["kind"] == "address"
        row_records.append(
            {
                "row": row,
                "opcode": opcode,
                "value_operand": target["value_operand"],
                "target_offset": target_offset,
                "candidate_offset": candidate_offset,
                "target_instruction": target["formatted"],
                "candidate_instruction": candidate["formatted"],
            }
        )

    if len(mapping) < 2:
        blockers.append("fewer_than_two_stack_home_mappings")
    if not has_store:
        blockers.append("stack_home_exchange_has_no_store")
    if not has_load:
        blockers.append("stack_home_exchange_has_no_load")
    if not has_address:
        blockers.append("stack_home_exchange_has_no_address_consumer")

    mapping_records = [
        {
            "candidate_offset": candidate_offset,
            "candidate_offset_hex": hex(candidate_offset),
            "target_offset": target_offset,
            "target_offset_hex": hex(target_offset),
            "delta": target_offset - candidate_offset,
            "rows": sorted(pair_rows[(candidate_offset, target_offset)]),
            "row_count": len(pair_rows[(candidate_offset, target_offset)]),
        }
        for candidate_offset, target_offset in sorted(mapping.items())
    ]
    delta_groups: dict[int, list[dict[str, Any]]] = defaultdict(list)
    for item in mapping_records:
        delta_groups[int(item["delta"])].append(item)
    displacement_groups = [
        {
            "delta": delta,
            "mapping_count": len(items),
            "candidate_offsets": [int(item["candidate_offset"]) for item in items],
            "target_offsets": [int(item["target_offset"]) for item in items],
            "rows": sorted(row for item in items for row in item["rows"]),
        }
        for delta, items in sorted(delta_groups.items())
    ]

    target_offsets = set(inverse)
    candidate_offsets = set(mapping)
    blockers = sorted(set(blockers))
    matched = not blockers
    result: dict[str, Any] = {
        "schema": SCHEMA,
        "schema_version": 1,
        "status": "matched" if matched else "blocked",
        "route": ROUTE if matched else None,
        "function": function,
        "binding": binding_value,
        "facts": {
            "target_size_bytes": target_size,
            "candidate_size_bytes": candidate_size,
            "function_size_exact": target_size is not None and target_size == candidate_size,
            "instruction_count_exact": len(target_rows) == len(candidate_rows),
            "data_target_size_bytes": data_target_size,
            "data_candidate_size_bytes": data_candidate_size,
            "data_match_percent": data_percent,
            "strict_residual_row_count": len(residual_rows),
            "stack_home_row_count": len(stack_rows),
            "pool_handoff_row_count": len(pool_rows),
            "pool_handoff_rows": pool_rows,
            "mapping_count": len(mapping),
            "opcode_set": sorted(opcodes),
            "all_non_pool_rows_accounted": len(row_records) == len(stack_rows),
        },
        "blockers": blockers,
        "stack_home_rows": row_records if matched else [],
        "home_mapping": mapping_records if matched else [],
        "displacement_groups": displacement_groups if matched else [],
        "observed_extents": (
            {
                "target": _extents(target_offsets),
                "candidate": _extents(candidate_offsets),
                "target_only_offsets": sorted(target_offsets - candidate_offsets),
                "candidate_only_offsets": sorted(candidate_offsets - target_offsets),
            }
            if matched
            else {}
        ),
        "candidate_cells": (
            [
                {
                    "id": "compose_authenticated_aggregate_and_capacity_home_class",
                    "ordinal": 1,
                    "action": (
                        "Enumerate the observed target/candidate extents, bind each extent to live typed "
                        "source owners using Graphify plus one exact same-game donor, and compile the single "
                        "composed aggregate/capacity cell before any lexical permutation."
                    ),
                    "compile_candidate_limit": 1,
                    "exact_donor_required": True,
                    "source_patch_emitted": False,
                }
            ]
            if matched
            else []
        ),
        "pool_handoff": (
            "after the composed stack-home cell, decode only the listed remaining pool rows"
            if matched and pool_rows
            else None
        ),
        "suppressed_axes": [
            "declaration_order_permutation",
            "scope_permutation",
            "dead_or_fake_local",
            "padding",
            "register_shaping",
            "tracing_before_donor_composition",
        ],
        "analysis_deadline_minutes": 5,
        "candidate_budget": 3,
        "trace_budget": 0 if matched else 1,
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["diagnosis_sha256"] = owner_manifest.canonical_sha256(result)
    return result


def build_from_paths(
    *,
    strict_report_path: Path,
    data_report_path: Path,
    target_object_path: Path,
    candidate_object_path: Path,
    function: str,
    expected_strict_report_sha256: str,
    expected_data_report_sha256: str,
    expected_target_object_sha256: str,
    expected_candidate_object_sha256: str,
) -> dict[str, Any]:
    expected = {
        "strict_report_sha256": owner_manifest._sha256(
            expected_strict_report_sha256, "expected_strict_report_sha256"
        ),
        "data_report_sha256": owner_manifest._sha256(
            expected_data_report_sha256, "expected_data_report_sha256"
        ),
        "target_object_sha256": owner_manifest._sha256(
            expected_target_object_sha256, "expected_target_object_sha256"
        ),
        "candidate_object_sha256": owner_manifest._sha256(
            expected_candidate_object_sha256, "expected_candidate_object_sha256"
        ),
    }
    actual = {
        "strict_report_sha256": owner_manifest.file_sha256(strict_report_path),
        "data_report_sha256": owner_manifest.file_sha256(data_report_path),
        "target_object_sha256": owner_manifest.file_sha256(target_object_path),
        "candidate_object_sha256": owner_manifest.file_sha256(candidate_object_path),
    }
    mismatches = [name for name in expected if expected[name] != actual[name]]
    if mismatches:
        raise owner_manifest.TypedPoolManifestInputError(
            "evidence hash mismatch: " + ", ".join(sorted(mismatches))
        )
    binding = {
        "schema": owner_manifest.BINDING_SCHEMA,
        "strict_report_path": str(strict_report_path),
        "strict_report_sha256": actual["strict_report_sha256"],
        "data_report_path": str(data_report_path),
        "data_report_sha256": actual["data_report_sha256"],
        "target_object_path": str(target_object_path),
        "target_object_sha256": actual["target_object_sha256"],
        "candidate_object_path": str(candidate_object_path),
        "candidate_object_sha256": actual["candidate_object_sha256"],
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }
    return build_diagnosis(
        owner_manifest.load_json(strict_report_path, "strict report"),
        owner_manifest.load_json(data_report_path, "data report"),
        function,
        binding,
    )


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("strict_report", type=Path)
    parser.add_argument("data_report", type=Path)
    parser.add_argument("function")
    parser.add_argument("--target-object", type=Path, required=True)
    parser.add_argument("--candidate-object", type=Path, required=True)
    parser.add_argument("--expect-strict-report-sha256", required=True)
    parser.add_argument("--expect-data-report-sha256", required=True)
    parser.add_argument("--expect-target-object-sha256", required=True)
    parser.add_argument("--expect-candidate-object-sha256", required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--require-match", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _parser()
    args = parser.parse_args(argv)
    try:
        result = build_from_paths(
            strict_report_path=args.strict_report,
            data_report_path=args.data_report,
            target_object_path=args.target_object,
            candidate_object_path=args.candidate_object,
            function=args.function,
            expected_strict_report_sha256=args.expect_strict_report_sha256,
            expected_data_report_sha256=args.expect_data_report_sha256,
            expected_target_object_sha256=args.expect_target_object_sha256,
            expected_candidate_object_sha256=args.expect_candidate_object_sha256,
        )
    except owner_manifest.TypedPoolManifestInputError as exc:
        parser.error(str(exc))
    rendered = json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.output is not None:
        try:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(rendered, encoding="utf-8")
        except OSError as exc:
            parser.error(f"cannot write {args.output}: {exc}")
    else:
        print(rendered, end="")
    if args.require_match and result["status"] != "matched":
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
