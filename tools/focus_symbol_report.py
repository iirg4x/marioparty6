#!/usr/bin/env python3
"""Build a small, hash-bound focus-function report from full objdiff reports.

The extractor keeps normalized target/candidate instructions for one strict
focus function, every strict/data diff row, relocation annotations and their
referenced object payloads, plus a compact protected-sibling digest. An
independent physical-relocation receipt can be embedded and cross-checked;
without one, physical relocation authority is explicitly UNKNOWN.

This tool is diagnostic only. It does not emit source, retain candidates, or
advance integration or promotion authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools import match_workbench as workbench


SCHEMA = "focus_symbol_report/v1"
GATE_SCHEMA = "focus_symbol_protected_sibling_gate/v1"
DEFAULT_MAX_OUTPUT_BYTES = 500 * 1024
_SHA256_RE = re.compile(r"[0-9a-f]{64}")


class FocusReportError(ValueError):
    """The supplied evidence cannot safely produce a compact report."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _sha256(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
        raise FocusReportError(f"{label} must be a lowercase SHA-256")
    return value


def _duplicate_checked_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise FocusReportError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _load_bound(path: Path, expected_sha256: str, label: str) -> tuple[Any, dict[str, Any]]:
    expected = _sha256(expected_sha256, f"expected {label} SHA-256")
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise FocusReportError(f"cannot read {label} {path}: {exc}") from exc
    actual = hashlib.sha256(raw).hexdigest()
    if actual != expected:
        raise FocusReportError(f"{label} SHA-256 mismatch: {actual} != {expected}")
    try:
        value = json.loads(raw, object_pairs_hook=_duplicate_checked_object)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise FocusReportError(f"invalid {label} JSON {path}: {exc}") from exc
    return value, {
        "path": str(path.resolve()),
        "sha256": actual,
        "size_bytes": len(raw),
    }


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise FocusReportError(f"{label} must be an object")
    return value


def _symbols(document: Mapping[str, Any], side_name: str, label: str) -> list[Mapping[str, Any]]:
    side = _mapping(document.get(side_name), f"{label}.{side_name}")
    raw = side.get("symbols")
    if not isinstance(raw, list):
        raise FocusReportError(f"{label}.{side_name}.symbols must be an array")
    symbols: list[Mapping[str, Any]] = []
    for index, symbol in enumerate(raw):
        symbols.append(_mapping(symbol, f"{label}.{side_name}.symbols[{index}]"))
    return symbols


def _is_function(symbol: Mapping[str, Any]) -> bool:
    return symbol.get("kind") == "SYMBOL_FUNCTION" or isinstance(symbol.get("instructions"), list)


def _focus_pair(
    document: Mapping[str, Any], function: str, label: str
) -> tuple[
    list[Mapping[str, Any]],
    list[Mapping[str, Any]],
    Mapping[str, Any],
    Mapping[str, Any],
]:
    left = _symbols(document, "left", label)
    right = _symbols(document, "right", label)
    matches = [symbol for symbol in left if _is_function(symbol) and symbol.get("name") == function]
    if len(matches) != 1:
        raise FocusReportError(
            f"{label} must contain exactly one left focus function {function!r}; found {len(matches)}"
        )
    target = matches[0]
    candidate_index = target.get("target_symbol")
    if (
        isinstance(candidate_index, bool)
        or not isinstance(candidate_index, int)
        or not 0 <= candidate_index < len(right)
    ):
        raise FocusReportError(f"{label} focus function is not paired by target_symbol")
    candidate = right[candidate_index]
    if not _is_function(candidate):
        raise FocusReportError(f"{label} paired focus symbol is not a function")
    if candidate.get("name") != function:
        raise FocusReportError(
            f"{label} paired focus symbol name drifted: {candidate.get('name')!r}"
        )
    return left, right, target, candidate


def _integer(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    try:
        return int(str(value), 0)
    except (TypeError, ValueError):
        return None


def _symbol_descriptor(symbol: Mapping[str, Any], index: int) -> dict[str, Any]:
    result: dict[str, Any] = {"index": index}
    for key in (
        "name",
        "kind",
        "address",
        "size",
        "flags",
        "target_symbol",
        "match_percent",
        "data_diff",
    ):
        if key in symbol:
            result[key] = symbol[key]
    return result


def _symbol_metadata(symbol: Mapping[str, Any]) -> dict[str, Any]:
    return {
        key: value
        for key, value in symbol.items()
        if key not in {"instructions", "data_diff"}
    }


def _rows(symbol: Mapping[str, Any], label: str) -> list[Mapping[str, Any]]:
    raw = symbol.get("instructions")
    if not isinstance(raw, list):
        raise FocusReportError(f"{label}.instructions must be an array")
    rows: list[Mapping[str, Any]] = []
    for index, row in enumerate(raw):
        rows.append(_mapping(row, f"{label}.instructions[{index}]"))
    return rows


def _normalized_row(row: Mapping[str, Any], index: int) -> dict[str, Any]:
    result: dict[str, Any] = {"index": index}
    for key in ("diff_kind", "arg_diff"):
        if key in row:
            result[key] = row[key]
    instruction = row.get("instruction")
    if isinstance(instruction, Mapping):
        compact_instruction = {
            key: value for key, value in instruction.items() if key != "parts"
        }
        result["instruction"] = compact_instruction
        if "parts" in instruction:
            result["parts_sha256"] = canonical_sha256(instruction["parts"])
    return result


def _normalized_rows(rows: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    return [_normalized_row(row, index) for index, row in enumerate(rows)]


def _instruction_payload_sha256(rows: Sequence[Mapping[str, Any]]) -> str:
    """Hash instruction payloads without strict/data-only diff annotations."""
    return canonical_sha256([row.get("instruction") for row in rows])


def _diff_rows(rows: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    return [
        _normalized_row(row, index)
        for index, row in enumerate(rows)
        if isinstance(row.get("diff_kind"), str) and row.get("diff_kind")
    ]


def _relocation_annotations(
    rows: Sequence[Mapping[str, Any]], symbols: Sequence[Mapping[str, Any]]
) -> dict[str, Any]:
    entries: list[dict[str, Any]] = []
    referenced: dict[int, dict[str, Any]] = {}
    for index, row in enumerate(rows):
        instruction = row.get("instruction")
        if not isinstance(instruction, Mapping):
            continue
        relocation = instruction.get("relocation")
        if not isinstance(relocation, Mapping):
            continue
        if relocation.get("type_name") == "R_PPC_NONE":
            continue
        target_index = relocation.get("target_symbol")
        if isinstance(target_index, bool) or not isinstance(target_index, int):
            raise FocusReportError(f"relocation row {index} has invalid target_symbol")
        if not 0 <= target_index < len(symbols):
            raise FocusReportError(f"relocation row {index} target_symbol is out of range")
        target = _symbol_descriptor(symbols[target_index], target_index)
        referenced[target_index] = target
        entries.append(
            {
                "row_index": index,
                "diff_kind": row.get("diff_kind"),
                "instruction_address": instruction.get("address"),
                "instruction_formatted": instruction.get("formatted"),
                "instruction_size": instruction.get("size"),
                "relocation": dict(relocation),
                "target_symbol_index": target_index,
            }
        )
    targets = [referenced[index] for index in sorted(referenced)]
    pool_dependencies = [
        target for target in targets if target.get("kind") == "SYMBOL_OBJECT"
    ]
    return {
        "count": len(entries),
        "entries": entries,
        "entries_sha256": canonical_sha256(entries),
        "targets": targets,
        "pool_dependencies": pool_dependencies,
        "pool_dependency_sha256": canonical_sha256(pool_dependencies),
    }


def _section_summaries(document: Mapping[str, Any], side_name: str) -> list[dict[str, Any]]:
    side = _mapping(document.get(side_name), f"report.{side_name}")
    raw = side.get("sections", [])
    if not isinstance(raw, list):
        raise FocusReportError(f"report.{side_name}.sections must be an array")
    result: list[dict[str, Any]] = []
    for index, section in enumerate(raw):
        value = _mapping(section, f"report.{side_name}.sections[{index}]")
        result.append(
            {
                key: item
                for key, item in value.items()
                if key in {"name", "kind", "size", "match_percent"}
            }
        )
    return result


def _protected_siblings(
    document: Mapping[str, Any], function: str, label: str
) -> dict[str, Any]:
    try:
        records, counts = workbench._assessment_records(document, label)
    except workbench.MatchError as exc:
        raise FocusReportError(str(exc)) from exc
    focuses = [record for record in records if record["name"] == function]
    if len(focuses) != 1:
        raise FocusReportError(f"{label} focus record count is {len(focuses)}")
    focus_identity = focuses[0]["identity"]
    siblings = [
        {
            "identity": record["identity"],
            "metric": workbench._residual_metric(record["metric"]),
        }
        for record in records
        if record["identity"] != focus_identity
    ]
    exact_identities = sorted(
        row["identity"] for row in siblings if row["metric"]["exact"]
    )
    return {
        "focus_identity_excluded": focus_identity,
        "function_counts": counts,
        "sibling_count": len(siblings),
        "exact_sibling_count": len(exact_identities),
        "exact_identities": exact_identities,
        "exact_identity_sha256": canonical_sha256(exact_identities),
        "all_sibling_metric_sha256": canonical_sha256(siblings),
    }


def _focus_metric(document: Mapping[str, Any], function: str, label: str) -> dict[str, Any]:
    try:
        records, _ = workbench._assessment_records(document, label)
    except workbench.MatchError as exc:
        raise FocusReportError(str(exc)) from exc
    metrics = [record["metric"] for record in records if record["name"] == function]
    if len(metrics) != 1:
        raise FocusReportError(f"{label} focus metric count is {len(metrics)}")
    return workbench._residual_metric(metrics[0])


def _channel(
    document: Mapping[str, Any],
    function: str,
    label: str,
    *,
    full_rows: bool,
    full_relocations: bool,
) -> dict[str, Any]:
    left_symbols, right_symbols, target, candidate = _focus_pair(document, function, label)
    target_rows = _rows(target, f"{label}.target")
    candidate_rows = _rows(candidate, f"{label}.candidate")
    target_normalized = _normalized_rows(target_rows) if full_rows else _diff_rows(target_rows)
    candidate_normalized = _normalized_rows(candidate_rows) if full_rows else _diff_rows(candidate_rows)
    return {
        "metric": _focus_metric(document, function, label),
        "target": {
            "symbol": _symbol_metadata(target),
            "instruction_count": len(target_rows),
            "raw_instruction_sha256": canonical_sha256(target_rows),
            "instruction_payload_sha256": _instruction_payload_sha256(target_rows),
            "rows_kind": "all" if full_rows else "diff_only",
            "rows": target_normalized,
            "diff_row_count": len(_diff_rows(target_rows)),
        },
        "candidate": {
            "symbol": _symbol_metadata(candidate),
            "instruction_count": len(candidate_rows),
            "raw_instruction_sha256": canonical_sha256(candidate_rows),
            "instruction_payload_sha256": _instruction_payload_sha256(candidate_rows),
            "rows_kind": "all" if full_rows else "diff_only",
            "rows": candidate_normalized,
            "diff_row_count": len(_diff_rows(candidate_rows)),
        },
        "relocation_annotations": (
            {
                "authority": "report_annotation_not_physical_proof",
                "storage": "full",
                "target": _relocation_annotations(target_rows, left_symbols),
                "candidate": _relocation_annotations(candidate_rows, right_symbols),
            }
            if full_relocations
            else {
                "authority": "report_annotation_not_physical_proof",
                "storage": "strict_channel_only",
                "strict_channel_reference": True,
            }
        ),
        "sections": {
            "target": _section_summaries(document, "left"),
            "candidate": _section_summaries(document, "right"),
        },
        "protected_siblings": _protected_siblings(document, function, label),
    }


def _validate_cross_channel(strict: Mapping[str, Any], data: Mapping[str, Any]) -> None:
    for side in ("target", "candidate"):
        for key in ("instruction_count", "instruction_payload_sha256"):
            if strict[side][key] != data[side][key]:
                raise FocusReportError(f"strict/data {side} {key} drifted")
        strict_size = strict["metric"].get(f"{side}_size")
        data_size = data["metric"].get(f"{side}_size")
        if strict_size != data_size:
            raise FocusReportError(f"strict/data {side} size drifted")


def _physical_receipt(
    receipt: Mapping[str, Any],
    binding: Mapping[str, Any],
    strict_binding: Mapping[str, Any],
    strict_channel: Mapping[str, Any],
    *,
    require_exact: bool,
) -> dict[str, Any]:
    target = _mapping(receipt.get("target"), "physical receipt target")
    candidate = _mapping(receipt.get("candidate"), "physical receipt candidate")
    differences = receipt.get("physical_relocation_differences")
    if not isinstance(differences, list):
        raise FocusReportError("physical receipt differences must be an array")
    exact = receipt.get("physical_relocations_exact")
    if not isinstance(exact, bool):
        raise FocusReportError("physical receipt exact flag must be boolean")
    report_descriptor = receipt.get("report")
    if isinstance(report_descriptor, Mapping) and "sha256" in report_descriptor:
        if report_descriptor.get("sha256") != strict_binding.get("sha256"):
            raise FocusReportError("physical receipt is not bound to the strict report")
    normalized: dict[str, Any] = {}
    for side_name, side in (("target", target), ("candidate", candidate)):
        rows = side.get("physical_relocations")
        count = _integer(side.get("physical_relocation_count"))
        if not isinstance(rows, list) or count is None or count != len(rows):
            raise FocusReportError(f"physical receipt {side_name} relocation count is invalid")
        expected_size = strict_channel["metric"].get(f"{side_name}_size")
        if _integer(side.get("size")) != expected_size:
            raise FocusReportError(f"physical receipt {side_name} size drifted")
        if _integer(side.get("instruction_count")) != strict_channel[side_name]["instruction_count"]:
            raise FocusReportError(f"physical receipt {side_name} instruction count drifted")
        normalized[side_name] = dict(side)
    if exact and differences:
        raise FocusReportError("physical receipt claims exactness with nonempty differences")
    if require_exact and not exact:
        raise FocusReportError("physical relocation receipt is not exact")
    return {
        "status": "exact" if exact else "mismatch",
        "authority": "independent_physical_receipt",
        "binding": dict(binding),
        "receipt_schema": receipt.get("schema"),
        "target": normalized["target"],
        "candidate": normalized["candidate"],
        "physical_relocation_differences": differences,
        "symbol_attribution_aliases": receipt.get("symbol_attribution_aliases", []),
        "receipt_payload_sha256": canonical_sha256(receipt),
    }


def build_artifact(
    strict_document: Mapping[str, Any],
    data_document: Mapping[str, Any],
    function: str,
    binding: Mapping[str, Any],
    *,
    physical_receipt: Mapping[str, Any] | None = None,
    physical_binding: Mapping[str, Any] | None = None,
    require_physical: bool = False,
) -> dict[str, Any]:
    if not isinstance(function, str) or not function.strip():
        raise FocusReportError("function must be nonempty text")
    focus = function.strip()
    strict = _channel(
        strict_document,
        focus,
        "strict report",
        full_rows=True,
        full_relocations=True,
    )
    data = _channel(
        data_document,
        focus,
        "data report",
        full_rows=False,
        full_relocations=False,
    )
    _validate_cross_channel(strict, data)
    strict_binding = _mapping(binding.get("strict_report"), "binding.strict_report")
    if physical_receipt is None:
        if require_physical:
            raise FocusReportError("an independent physical relocation receipt is required")
        physical: dict[str, Any] = {
            "status": "UNKNOWN",
            "authority": "none",
            "reason": "no independent physical relocation receipt was supplied",
        }
    else:
        if physical_binding is None:
            raise FocusReportError("physical receipt binding is required")
        physical = _physical_receipt(
            physical_receipt,
            physical_binding,
            strict_binding,
            strict,
            require_exact=require_physical,
        )
    result: dict[str, Any] = {
        "schema": SCHEMA,
        "schema_version": 1,
        "function": focus,
        "input_binding": dict(binding),
        "channels": {"strict": strict, "data": data},
        "physical_relocations": physical,
        "policies": {
            "strict_rows": "all_normalized_rows",
            "data_rows": "diff_only_with_full_raw_digest",
            "instruction_parts": "omitted_but_sha256_bound_per_row",
            "protected_sibling_gate": "baseline_exact_identity_subset",
            "physical_without_receipt": "UNKNOWN",
        },
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    result["artifact_sha256"] = canonical_sha256(result)
    return result


def build_from_paths(
    *,
    strict_report_path: Path,
    data_report_path: Path,
    function: str,
    expected_strict_report_sha256: str,
    expected_data_report_sha256: str,
    physical_receipt_path: Path | None = None,
    expected_physical_receipt_sha256: str | None = None,
    require_physical: bool = False,
) -> dict[str, Any]:
    strict_document, strict_binding = _load_bound(
        strict_report_path, expected_strict_report_sha256, "strict report"
    )
    data_document, data_binding = _load_bound(
        data_report_path, expected_data_report_sha256, "data report"
    )
    physical_receipt = None
    physical_binding = None
    if (physical_receipt_path is None) != (expected_physical_receipt_sha256 is None):
        raise FocusReportError("physical receipt path and expected SHA-256 must be supplied together")
    if physical_receipt_path is not None and expected_physical_receipt_sha256 is not None:
        physical_receipt, physical_binding = _load_bound(
            physical_receipt_path,
            expected_physical_receipt_sha256,
            "physical relocation receipt",
        )
    binding = {
        "strict_report": strict_binding,
        "data_report": data_binding,
        "retail_target_authenticated": True,
        "authority_advanced": False,
    }
    if physical_binding is not None:
        binding["physical_relocation_receipt"] = physical_binding
    return build_artifact(
        _mapping(strict_document, "strict report"),
        _mapping(data_document, "data report"),
        function,
        binding,
        physical_receipt=(
            _mapping(physical_receipt, "physical relocation receipt")
            if physical_receipt is not None
            else None
        ),
        physical_binding=physical_binding,
        require_physical=require_physical,
    )


def _verify_artifact(value: Mapping[str, Any], label: str) -> None:
    if value.get("schema") != SCHEMA:
        raise FocusReportError(f"{label} schema is not {SCHEMA}")
    expected = value.get("artifact_sha256")
    _sha256(expected, f"{label}.artifact_sha256")
    unsigned = dict(value)
    unsigned.pop("artifact_sha256", None)
    actual = canonical_sha256(unsigned)
    if actual != expected:
        raise FocusReportError(f"{label} internal artifact SHA-256 mismatch")
    if value.get("authority_advanced") is not False:
        raise FocusReportError(f"{label} advanced authority")


def gate_artifacts(
    baseline: Mapping[str, Any],
    candidate: Mapping[str, Any],
    binding: Mapping[str, Any],
) -> dict[str, Any]:
    _verify_artifact(baseline, "baseline")
    _verify_artifact(candidate, "candidate")
    if baseline.get("function") != candidate.get("function"):
        raise FocusReportError("baseline and candidate focus functions differ")
    channel_results: dict[str, Any] = {}
    for channel in ("strict", "data"):
        baseline_material = _mapping(
            _mapping(baseline.get("channels"), "baseline.channels").get(channel),
            f"baseline.channels.{channel}",
        ).get("protected_siblings")
        candidate_material = _mapping(
            _mapping(candidate.get("channels"), "candidate.channels").get(channel),
            f"candidate.channels.{channel}",
        ).get("protected_siblings")
        before = _mapping(baseline_material, f"baseline {channel} protected siblings")
        after = _mapping(candidate_material, f"candidate {channel} protected siblings")
        before_exact = before.get("exact_identities")
        after_exact = after.get("exact_identities")
        if not isinstance(before_exact, list) or not all(isinstance(item, str) for item in before_exact):
            raise FocusReportError(f"baseline {channel} exact identities are invalid")
        if not isinstance(after_exact, list) or not all(isinstance(item, str) for item in after_exact):
            raise FocusReportError(f"candidate {channel} exact identities are invalid")
        missing = sorted(set(before_exact) - set(after_exact))
        gained = sorted(set(after_exact) - set(before_exact))
        channel_results[channel] = {
            "passed": not missing,
            "protected_exact_before": len(before_exact),
            "protected_exact_after": len(after_exact),
            "missing_exact_siblings": missing,
            "gained_exact_siblings": gained,
            "baseline_exact_identity_sha256": before.get("exact_identity_sha256"),
            "candidate_exact_identity_sha256": after.get("exact_identity_sha256"),
        }
    passed = all(result["passed"] for result in channel_results.values())
    result: dict[str, Any] = {
        "schema": GATE_SCHEMA,
        "schema_version": 1,
        "function": baseline.get("function"),
        "binding": dict(binding),
        "status": "passed" if passed else "regressed",
        "passed": passed,
        "channels": channel_results,
        "authority_advanced": False,
    }
    result["gate_sha256"] = canonical_sha256(result)
    return result


def gate_from_paths(
    *,
    baseline_path: Path,
    candidate_path: Path,
    expected_baseline_sha256: str,
    expected_candidate_sha256: str,
) -> dict[str, Any]:
    baseline, baseline_binding = _load_bound(
        baseline_path, expected_baseline_sha256, "baseline compact artifact"
    )
    candidate, candidate_binding = _load_bound(
        candidate_path, expected_candidate_sha256, "candidate compact artifact"
    )
    return gate_artifacts(
        _mapping(baseline, "baseline compact artifact"),
        _mapping(candidate, "candidate compact artifact"),
        {
            "baseline_artifact": baseline_binding,
            "candidate_artifact": candidate_binding,
            "authority_advanced": False,
        },
    )


def _write_result(
    value: Mapping[str, Any], output: Path | None, max_output_bytes: int, *, pretty: bool
) -> None:
    if max_output_bytes <= 0:
        raise FocusReportError("max_output_bytes must be positive")
    rendered = (
        json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2)
        if pretty
        else _canonical(value).decode("utf-8")
    ) + "\n"
    size = len(rendered.encode("utf-8"))
    if size > max_output_bytes:
        raise FocusReportError(
            f"compact artifact exceeds max_output_bytes ({size} > {max_output_bytes})"
        )
    if output is None:
        print(rendered, end="")
        return
    try:
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(rendered, encoding="utf-8")
    except OSError as exc:
        raise FocusReportError(f"cannot write {output}: {exc}") from exc


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract = subparsers.add_parser("extract", help="extract one compact focus artifact")
    extract.add_argument("strict_report", type=Path)
    extract.add_argument("data_report", type=Path)
    extract.add_argument("function")
    extract.add_argument("--expect-strict-report-sha256", required=True)
    extract.add_argument("--expect-data-report-sha256", required=True)
    extract.add_argument("--physical-relocation-receipt", type=Path)
    extract.add_argument("--expect-physical-relocation-receipt-sha256")
    extract.add_argument("--require-physical", action="store_true")
    extract.add_argument("--output", type=Path)
    extract.add_argument("--max-output-bytes", type=int, default=DEFAULT_MAX_OUTPUT_BYTES)
    extract.add_argument("--pretty", action="store_true")

    gate = subparsers.add_parser("gate", help="compare protected sibling sets")
    gate.add_argument("baseline_artifact", type=Path)
    gate.add_argument("candidate_artifact", type=Path)
    gate.add_argument("--expect-baseline-artifact-sha256", required=True)
    gate.add_argument("--expect-candidate-artifact-sha256", required=True)
    gate.add_argument("--output", type=Path)
    gate.add_argument("--max-output-bytes", type=int, default=DEFAULT_MAX_OUTPUT_BYTES)
    gate.add_argument("--pretty", action="store_true")
    gate.add_argument("--require-pass", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = _parser()
    args = parser.parse_args(argv)
    try:
        if args.command == "extract":
            result = build_from_paths(
                strict_report_path=args.strict_report,
                data_report_path=args.data_report,
                function=args.function,
                expected_strict_report_sha256=args.expect_strict_report_sha256,
                expected_data_report_sha256=args.expect_data_report_sha256,
                physical_receipt_path=args.physical_relocation_receipt,
                expected_physical_receipt_sha256=(
                    args.expect_physical_relocation_receipt_sha256
                ),
                require_physical=args.require_physical,
            )
            _write_result(result, args.output, args.max_output_bytes, pretty=args.pretty)
            return 0
        result = gate_from_paths(
            baseline_path=args.baseline_artifact,
            candidate_path=args.candidate_artifact,
            expected_baseline_sha256=args.expect_baseline_artifact_sha256,
            expected_candidate_sha256=args.expect_candidate_artifact_sha256,
        )
        _write_result(result, args.output, args.max_output_bytes, pretty=args.pretty)
        if args.require_pass and not result["passed"]:
            return 2
        return 0
    except FocusReportError as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
