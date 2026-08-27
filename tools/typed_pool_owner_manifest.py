#!/usr/bin/env python3
"""Emit one fail-closed candidate manifest for typed SDA21 owner-only residuals.

This tool is intentionally narrower than the installed typed-pool decoder.  It
only converts a residual into one bounded candidate cell when the function is
exact-size, data-exact, and every strict residual row is a value/type/addend/
consumer-identical SDA21 relocation whose target is a named pool owner and
whose candidate is compiler-anonymous.  Anything else is reported as blocked.

The manifest is diagnostic.  It does not edit source, authenticate a C
declaration, retain a candidate, prove physical relocation targets, or advance
recovery authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import pool_reloc_summary as pool_decoder


SCHEMA = "typed_pool_owner_manifest/v1"
BINDING_SCHEMA = "typed_pool_owner_manifest_binding/v1"
ROUTE = "typed_pool_owner_manifest"
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_SDA_LOAD_RE = re.compile(
    r"^(?P<opcode>lfs|lfd|lha|lhz|lbz|lwz)\s+"
    r"(?P<destination>[^,]+),\s*(?P<owner>[^,\s]+)@sda21$",
    re.IGNORECASE,
)
_EXACT_CLASSES = {"exact_pool_contract", "mapped_pool_contract"}


class TypedPoolManifestInputError(ValueError):
    """Malformed or unbound manifest evidence."""


def _canonical(value: Mapping[str, Any]) -> bytes:
    return (
        json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False) + "\n"
    ).encode("utf-8")


def canonical_sha256(value: Mapping[str, Any]) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            while chunk := stream.read(1024 * 1024):
                digest.update(chunk)
    except OSError as exc:
        raise TypedPoolManifestInputError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _sha256(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise TypedPoolManifestInputError(f"{label} must be a lowercase SHA-256")
    return value


def _text(value: Any, label: str, *, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise TypedPoolManifestInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise TypedPoolManifestInputError(f"{label} exceeds {limit} characters")
    return result


def _closed_binding(value: Mapping[str, Any]) -> dict[str, Any]:
    fields = {
        "schema",
        "strict_report_path",
        "strict_report_sha256",
        "data_report_path",
        "data_report_sha256",
        "target_object_path",
        "target_object_sha256",
        "candidate_object_path",
        "candidate_object_sha256",
        "retail_target_authenticated",
        "authority_advanced",
    }
    if not isinstance(value, Mapping):
        raise TypedPoolManifestInputError("binding must be a JSON object")
    missing = fields - set(value)
    extra = set(value) - fields
    if missing or extra:
        raise TypedPoolManifestInputError(
            f"binding fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    if value.get("schema") != BINDING_SCHEMA:
        raise TypedPoolManifestInputError(f"binding.schema must be {BINDING_SCHEMA}")
    if value.get("retail_target_authenticated") is not True:
        raise TypedPoolManifestInputError("binding.retail_target_authenticated must be true")
    if value.get("authority_advanced") is not False:
        raise TypedPoolManifestInputError("binding.authority_advanced must be false")
    result = {
        "schema": BINDING_SCHEMA,
        "strict_report_path": _text(value.get("strict_report_path"), "binding.strict_report_path"),
        "strict_report_sha256": _sha256(
            value.get("strict_report_sha256"), "binding.strict_report_sha256"
        ),
        "data_report_path": _text(value.get("data_report_path"), "binding.data_report_path"),
        "data_report_sha256": _sha256(
            value.get("data_report_sha256"), "binding.data_report_sha256"
        ),
        "target_object_path": _text(value.get("target_object_path"), "binding.target_object_path"),
        "target_object_sha256": _sha256(
            value.get("target_object_sha256"), "binding.target_object_sha256"
        ),
        "candidate_object_path": _text(
            value.get("candidate_object_path"), "binding.candidate_object_path"
        ),
        "candidate_object_sha256": _sha256(
            value.get("candidate_object_sha256"), "binding.candidate_object_sha256"
        ),
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }
    return result


def _has_diff(row: Any) -> bool:
    if not isinstance(row, Mapping):
        return False
    kinds = [row.get("diff_kind")]
    instruction = row.get("instruction")
    if isinstance(instruction, Mapping):
        kinds.append(instruction.get("diff_kind"))
    return any(value not in {None, "", "DIFF_NONE", "NONE"} for value in kinds)


def _strict_residual_rows(
    target_function: Mapping[str, Any], candidate_function: Mapping[str, Any]
) -> list[int]:
    target_rows = list(pool_decoder._sequence(target_function.get("instructions")))
    candidate_rows = list(pool_decoder._sequence(candidate_function.get("instructions")))
    return [
        index
        for index in range(max(len(target_rows), len(candidate_rows)))
        if (index < len(target_rows) and _has_diff(target_rows[index]))
        or (index < len(candidate_rows) and _has_diff(candidate_rows[index]))
    ]


def _float_percent(value: Any) -> float | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, (int, float)):
        return float(value)
    if isinstance(value, str):
        try:
            return float(value)
        except ValueError:
            return None
    return None


def _instruction_contract(formatted: Any) -> dict[str, str] | None:
    if not isinstance(formatted, str):
        return None
    match = _SDA_LOAD_RE.fullmatch(formatted.strip())
    if match is None:
        return None
    return {
        "opcode": match.group("opcode").lower(),
        "destination": match.group("destination").strip().lower(),
        "owner_operand": match.group("owner"),
    }


def _owner(item: Mapping[str, Any]) -> Mapping[str, Any]:
    value = item.get("owner")
    return value if isinstance(value, Mapping) else {}


def _relocation(item: Mapping[str, Any]) -> Mapping[str, Any]:
    value = item.get("relocation")
    return value if isinstance(value, Mapping) else {}


def _manifest_consumer(pair: Mapping[str, Any]) -> dict[str, Any]:
    target = pair.get("target") if isinstance(pair.get("target"), Mapping) else {}
    candidate = pair.get("candidate") if isinstance(pair.get("candidate"), Mapping) else {}
    target_contract = _instruction_contract(target.get("instruction")) or {}
    candidate_contract = _instruction_contract(candidate.get("instruction")) or {}
    return {
        "row": int(pair["row"]),
        "target_instruction_address": target.get("instruction_address"),
        "candidate_instruction_address": candidate.get("instruction_address"),
        "target_instruction": target.get("instruction"),
        "candidate_instruction": candidate.get("instruction"),
        "opcode": target_contract.get("opcode"),
        "destination": target_contract.get("destination"),
        "relocation": dict(_relocation(target)),
        "source_site_status": "instruction_site_bound_source_span_unresolved",
    }


def build_manifest(
    strict_report: Mapping[str, Any],
    data_report: Mapping[str, Any],
    function: str,
    binding: Mapping[str, Any],
) -> dict[str, Any]:
    """Return a matched or blocked deterministic manifest."""

    function = _text(function, "function", limit=256)
    binding_value = _closed_binding(binding)
    try:
        strict_left, strict_right, strict_target, strict_candidate = pool_decoder._function_pair(
            strict_report, function
        )
        _, _, data_target, data_candidate = pool_decoder._function_pair(data_report, function)
    except pool_decoder.PoolDecodeError as exc:
        raise TypedPoolManifestInputError(str(exc)) from exc

    strict_target_size = pool_decoder._int(strict_target.get("size"))
    strict_candidate_size = pool_decoder._int(strict_candidate.get("size"))
    data_target_size = pool_decoder._int(data_target.get("size"))
    data_candidate_size = pool_decoder._int(data_candidate.get("size"))
    data_percent = _float_percent(data_candidate.get("match_percent"))
    residual_rows = _strict_residual_rows(strict_target, strict_candidate)

    target_pool_rows = pool_decoder._pool_rows(strict_left, strict_target)
    candidate_pool_rows = pool_decoder._pool_rows(strict_right, strict_candidate)
    all_pool_pairs = [
        pool_decoder._pair_record(row, target_pool_rows.get(row), candidate_pool_rows.get(row))
        for row in sorted(set(target_pool_rows) | set(candidate_pool_rows))
    ]
    nonexact_pool_pairs = [
        pair for pair in all_pool_pairs if pair.get("classification") not in _EXACT_CLASSES
    ]
    pool_rows = sorted(int(pair["row"]) for pair in nonexact_pool_pairs)
    classification_counts: dict[str, int] = defaultdict(int)
    for pair in nonexact_pool_pairs:
        classification_counts[str(pair.get("classification"))] += 1

    blockers: list[str] = []
    if strict_target_size is None or strict_target_size != strict_candidate_size:
        blockers.append("function_size_not_exact")
    if data_target_size is None or data_target_size != data_candidate_size or data_percent != 100.0:
        blockers.append("data_function_not_exact")
    if not residual_rows:
        blockers.append("no_strict_residual_rows")
    if residual_rows != pool_rows:
        blockers.append("strict_residual_contains_non_pool_or_unpaired_rows")
    if not nonexact_pool_pairs:
        blockers.append("no_nonexact_pool_pairs")

    grouped: dict[str, list[Mapping[str, Any]]] = defaultdict(list)
    for pair in nonexact_pool_pairs:
        target = pair.get("target") if isinstance(pair.get("target"), Mapping) else None
        candidate = pair.get("candidate") if isinstance(pair.get("candidate"), Mapping) else None
        if target is None or candidate is None:
            blockers.append(f"row_{pair.get('row')}_consumer_presence_mismatch")
            continue
        target_owner = _owner(target)
        candidate_owner = _owner(candidate)
        target_relocation = _relocation(target)
        candidate_relocation = _relocation(candidate)
        target_contract = _instruction_contract(target.get("instruction"))
        candidate_contract = _instruction_contract(candidate.get("instruction"))
        row = int(pair["row"])
        if pair.get("classification") != "owner_identity_mismatch":
            blockers.append(f"row_{row}_classification_{pair.get('classification')}")
        if set(pair.get("differences", [])) - {"owner_name", "owner_offset"}:
            blockers.append(f"row_{row}_has_non_owner_difference")
        if target_relocation != candidate_relocation or target_relocation.get("type") != "R_PPC_EMB_SDA21":
            blockers.append(f"row_{row}_relocation_contract_not_exact_sda21")
        if (
            target_owner.get("bytes") is None
            or target_owner.get("bytes") != candidate_owner.get("bytes")
            or target_owner.get("size_bytes") != candidate_owner.get("size_bytes")
            or target_owner.get("consumer_type") != candidate_owner.get("consumer_type")
        ):
            blockers.append(f"row_{row}_typed_value_contract_not_exact")
        if target_owner.get("section") != candidate_owner.get("section") or target_owner.get(
            "section"
        ) != ".sdata2":
            blockers.append(f"row_{row}_section_not_exact_sdata2")
        if target_owner.get("owner_class") not in {"named_label", "named_object"}:
            blockers.append(f"row_{row}_target_owner_not_named")
        if candidate_owner.get("owner_class") != "compiler_anonymous":
            blockers.append(f"row_{row}_candidate_owner_not_compiler_anonymous")
        if target_contract is None or candidate_contract is None:
            blockers.append(f"row_{row}_unsupported_consumer_instruction")
        elif (
            target_contract["opcode"] != candidate_contract["opcode"]
            or target_contract["destination"] != candidate_contract["destination"]
        ):
            blockers.append(f"row_{row}_consumer_instruction_not_exact")
        name = target_owner.get("name")
        if not isinstance(name, str) or not name:
            blockers.append(f"row_{row}_target_owner_name_missing")
        else:
            grouped[name].append(pair)

    owners: list[dict[str, Any]] = []
    for name, pairs in sorted(grouped.items(), key=lambda item: min(int(pair["row"]) for pair in item[1])):
        first_target = pairs[0]["target"]
        first_candidate = pairs[0]["candidate"]
        target_owner = _owner(first_target)
        candidate_owners = {
            str(_owner(pair["candidate"]).get("name")) for pair in pairs
        }
        target_contracts = {
            (
                _owner(pair["target"]).get("bytes"),
                _owner(pair["target"]).get("size_bytes"),
                _owner(pair["target"]).get("consumer_type"),
                _owner(pair["target"]).get("section"),
            )
            for pair in pairs
        }
        if len(candidate_owners) != 1:
            blockers.append(f"target_owner_{name}_maps_multiple_candidate_owners")
        if len(target_contracts) != 1:
            blockers.append(f"target_owner_{name}_has_ambiguous_typed_contract")
        owners.append(
            {
                "target": {
                    "name": name,
                    "owner_class": target_owner.get("owner_class"),
                    "section": target_owner.get("section"),
                    "address": target_owner.get("address"),
                    "size_bytes": target_owner.get("size_bytes"),
                    "bytes": target_owner.get("bytes"),
                    "typed": target_owner.get("typed"),
                    "consumer_type": target_owner.get("consumer_type"),
                },
                "candidate": {
                    "names": sorted(candidate_owners),
                    "owner_class": _owner(first_candidate).get("owner_class"),
                    "section": _owner(first_candidate).get("section"),
                    "address": _owner(first_candidate).get("address"),
                },
                "consumer_count": len(pairs),
                "consumers": [_manifest_consumer(pair) for pair in sorted(pairs, key=lambda item: int(item["row"]))],
            }
        )

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
            "target_size_bytes": strict_target_size,
            "candidate_size_bytes": strict_candidate_size,
            "function_size_exact": strict_target_size is not None
            and strict_target_size == strict_candidate_size,
            "data_target_size_bytes": data_target_size,
            "data_candidate_size_bytes": data_candidate_size,
            "data_match_percent": data_percent,
            "data_values_exact": data_percent == 100.0 and data_target_size == data_candidate_size,
            "strict_residual_row_count": len(residual_rows),
            "strict_residual_rows": residual_rows,
            "nonexact_pool_row_count": len(nonexact_pool_pairs),
            "nonexact_pool_rows": pool_rows,
            "classification_counts": dict(sorted(classification_counts.items())),
            "named_target_owner_count": len(owners),
        },
        "blockers": blockers,
        "owners": owners if matched else [],
        "candidate_cells": (
            [
                {
                    "id": "bind_all_named_typed_pool_owners_at_exact_consumers",
                    "ordinal": 1,
                    "owner_count": len(owners),
                    "consumer_count": len(nonexact_pool_pairs),
                    "action": (
                        "Bind every listed target-proven named typed owner at only the listed semantic "
                        "consumer sites, preserving value, type, addend, opcode, destination, CFG, and calls."
                    ),
                    "source_patch_emitted": False,
                    "owner_source_site_confirmation_required": True,
                    "compile_candidate_limit": 1,
                }
            ]
            if matched
            else []
        ),
        "suppressed_axes": [
            "numeric_literal_search",
            "declaration_order_permutation",
            "pool_seeder",
            "invented_label",
            "cfg_edit",
            "register_shaping",
            "tracing_before_candidate",
        ],
        "next_gate": (
            "owner confirms each instruction row maps to a truthful semantic source consumer; then compile one composed cell"
            if matched
            else "run first-pass triage and inspect the reported non-owner residual class"
        ),
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["manifest_sha256"] = canonical_sha256(result)
    return result


def load_json(path: Path, label: str) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise TypedPoolManifestInputError(f"cannot read {label} {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise TypedPoolManifestInputError(f"invalid JSON in {label} {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise TypedPoolManifestInputError(f"{label} root must be a JSON object")
    return value


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
        "strict_report_sha256": _sha256(
            expected_strict_report_sha256, "expected_strict_report_sha256"
        ),
        "data_report_sha256": _sha256(
            expected_data_report_sha256, "expected_data_report_sha256"
        ),
        "target_object_sha256": _sha256(
            expected_target_object_sha256, "expected_target_object_sha256"
        ),
        "candidate_object_sha256": _sha256(
            expected_candidate_object_sha256, "expected_candidate_object_sha256"
        ),
    }
    actual = {
        "strict_report_sha256": file_sha256(strict_report_path),
        "data_report_sha256": file_sha256(data_report_path),
        "target_object_sha256": file_sha256(target_object_path),
        "candidate_object_sha256": file_sha256(candidate_object_path),
    }
    mismatches = [name for name in expected if expected[name] != actual[name]]
    if mismatches:
        raise TypedPoolManifestInputError(
            "evidence hash mismatch: " + ", ".join(sorted(mismatches))
        )
    binding = {
        "schema": BINDING_SCHEMA,
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
    return build_manifest(
        load_json(strict_report_path, "strict report"),
        load_json(data_report_path, "data report"),
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
    except TypedPoolManifestInputError as exc:
        parser.error(str(exc))
    if args.require_match and result["status"] != "matched":
        parser.error("typed pool owner manifest did not match: " + ", ".join(result["blockers"]))
    rendered = json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.output is not None:
        try:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(rendered, encoding="utf-8")
        except OSError as exc:
            parser.error(f"cannot write {args.output}: {exc}")
    else:
        print(rendered, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
