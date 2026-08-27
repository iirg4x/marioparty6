#!/usr/bin/env python3
"""Diagnose a source-bound MWCC allocation-score delta without editing source.

The reducer is deliberately narrow.  It accepts an authenticated comparison of
two same-session frontend traces, requires every score increment to be bound to
an exact source span, and emits one diagnostic control cell.  It never treats a
codegen-neutral spelling as original-source provenance or retention authority.
"""

from __future__ import annotations

import hashlib
import json
import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "mwcc_score_delta_context/v1"
RULE_ID = "mwcc_source_bound_score_delta"

_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_REGISTER_RE = {
    "FPR": re.compile(r"f(?:[0-9]|[12][0-9]|3[01])", re.IGNORECASE),
    "GPR": re.compile(r"r(?:[0-9]|[12][0-9]|3[01])", re.IGNORECASE),
}


class ScoreDeltaInputError(ValueError):
    """The supplied trace/source evidence cannot seal a score delta."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    ).encode("utf-8")


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ScoreDeltaInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise ScoreDeltaInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 4096) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise ScoreDeltaInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise ScoreDeltaInputError(f"{label} exceeds {limit} characters")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise ScoreDeltaInputError(f"{label} must be a C identifier")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if result != result.lower() or _SHA256_RE.fullmatch(result) is None:
        raise ScoreDeltaInputError(f"{label} must be a lowercase SHA-256")
    return result


def _uint(
    value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24
) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise ScoreDeltaInputError(
            f"{label} must be an integer from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, maximum: float = 100.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ScoreDeltaInputError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or not 0.0 <= result <= maximum:
        raise ScoreDeltaInputError(f"{label} is outside the supported range")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise ScoreDeltaInputError(f"{label} must be {str(expected).lower()}")
    return expected


def _rows(value: Any, label: str, *, allow_empty: bool) -> list[int]:
    if not isinstance(value, list) or (not value and not allow_empty):
        requirement = "an array" if allow_empty else "a non-empty array"
        raise ScoreDeltaInputError(f"{label} must be {requirement}")
    result = [_uint(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if result != sorted(set(result)):
        raise ScoreDeltaInputError(f"{label} must be sorted and unique")
    return result


def _register(value: Any, bank: str, label: str) -> str:
    result = _text(value, label, limit=3).lower()
    if _REGISTER_RE[bank].fullmatch(result) is None:
        raise ScoreDeltaInputError(f"{label} must be a {bank} register")
    return result


def _hash_fields(
    value: Mapping[str, Any], label: str, fields: Sequence[str]
) -> dict[str, str]:
    return {field: _sha256(value.get(field), f"{label}.{field}") for field in fields}


def _parse_baseline(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "candidate_id",
        "objdiff_canonical_sha256",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "target_bytes",
        "candidate_bytes",
        "target_frame",
        "candidate_frame",
        "match_percent",
        "target_physical_relocations",
        "candidate_physical_relocations",
        "score_residual_rows",
        "other_residual_rows",
        "instruction_count_exact",
        "operation_order_exact",
        "cfg_calls_exact",
        "frame_exact",
        "physical_relocation_topology_exact",
        "protected_siblings_preserved",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    target_bytes = _uint(item.get("target_bytes"), f"{label}.target_bytes", minimum=4)
    candidate_bytes = _uint(
        item.get("candidate_bytes"), f"{label}.candidate_bytes", minimum=4
    )
    target_frame = _uint(item.get("target_frame"), f"{label}.target_frame", minimum=16)
    candidate_frame = _uint(
        item.get("candidate_frame"), f"{label}.candidate_frame", minimum=16
    )
    target_relocs = _uint(
        item.get("target_physical_relocations"),
        f"{label}.target_physical_relocations",
        minimum=1,
    )
    candidate_relocs = _uint(
        item.get("candidate_physical_relocations"),
        f"{label}.candidate_physical_relocations",
        minimum=1,
    )
    match_percent = _number(item.get("match_percent"), f"{label}.match_percent")
    if (
        target_bytes != candidate_bytes
        or target_frame != candidate_frame
        or target_relocs != candidate_relocs
        or match_percent >= 100.0
    ):
        raise ScoreDeltaInputError(
            f"{label} must be nonexact with exact size/frame/physical relocations"
        )
    for field in (
        "instruction_count_exact",
        "operation_order_exact",
        "cfg_calls_exact",
        "frame_exact",
        "physical_relocation_topology_exact",
        "protected_siblings_preserved",
    ):
        _bool(item.get(field), f"{label}.{field}", True)
    score_rows = _rows(item.get("score_residual_rows"), f"{label}.score_residual_rows", allow_empty=False)
    other_rows = _rows(item.get("other_residual_rows"), f"{label}.other_residual_rows", allow_empty=True)
    if set(score_rows) & set(other_rows):
        raise ScoreDeltaInputError(f"{label} score and other residual rows overlap")
    return {
        "candidate_id": _text(item.get("candidate_id"), f"{label}.candidate_id", limit=128),
        **_hash_fields(
            item,
            label,
            (
                "objdiff_canonical_sha256",
                "source_sha256",
                "object_sha256",
                "strict_report_sha256",
                "data_report_sha256",
            ),
        ),
        "target_bytes": target_bytes,
        "candidate_bytes": candidate_bytes,
        "target_frame": target_frame,
        "candidate_frame": candidate_frame,
        "match_percent": match_percent,
        "physical_relocations": target_relocs,
        "score_residual_rows": score_rows,
        "other_residual_rows": other_rows,
        "instruction_count_exact": True,
        "operation_order_exact": True,
        "cfg_calls_exact": True,
        "frame_exact": True,
        "physical_relocation_topology_exact": True,
        "protected_siblings_preserved": True,
    }


def _parse_control(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "candidate_id",
        "objdiff_canonical_sha256",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "target_bytes",
        "candidate_bytes",
        "match_percent",
        "physical_relocations",
        "allocation_rows_removed",
        "remaining_non_score_rows",
        "new_instruction_rows",
        "new_call_rows",
        "new_store_rows",
        "new_branch_rows",
        "codegen_neutral_after_allocation",
        "protected_siblings_preserved",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    result = {
        "candidate_id": _text(item.get("candidate_id"), f"{label}.candidate_id", limit=128),
        **_hash_fields(
            item,
            label,
            (
                "objdiff_canonical_sha256",
                "source_sha256",
                "object_sha256",
                "strict_report_sha256",
                "data_report_sha256",
            ),
        ),
        "target_bytes": _uint(item.get("target_bytes"), f"{label}.target_bytes", minimum=4),
        "candidate_bytes": _uint(
            item.get("candidate_bytes"), f"{label}.candidate_bytes", minimum=4
        ),
        "match_percent": _number(item.get("match_percent"), f"{label}.match_percent"),
        "physical_relocations": _uint(
            item.get("physical_relocations"), f"{label}.physical_relocations", minimum=1
        ),
        "allocation_rows_removed": _uint(
            item.get("allocation_rows_removed"), f"{label}.allocation_rows_removed", minimum=1
        ),
        "remaining_non_score_rows": _uint(
            item.get("remaining_non_score_rows"), f"{label}.remaining_non_score_rows"
        ),
        "new_instruction_rows": _uint(
            item.get("new_instruction_rows"), f"{label}.new_instruction_rows"
        ),
        "new_call_rows": _uint(item.get("new_call_rows"), f"{label}.new_call_rows"),
        "new_store_rows": _uint(item.get("new_store_rows"), f"{label}.new_store_rows"),
        "new_branch_rows": _uint(item.get("new_branch_rows"), f"{label}.new_branch_rows"),
        "codegen_neutral_after_allocation": _bool(
            item.get("codegen_neutral_after_allocation"),
            f"{label}.codegen_neutral_after_allocation",
            True,
        ),
        "protected_siblings_preserved": _bool(
            item.get("protected_siblings_preserved"),
            f"{label}.protected_siblings_preserved",
            True,
        ),
    }
    if (
        result["target_bytes"] != result["candidate_bytes"]
        or result["match_percent"] >= 100.0
        or any(result[field] for field in ("new_instruction_rows", "new_call_rows", "new_store_rows", "new_branch_rows"))
    ):
        raise ScoreDeltaInputError(
            f"{label} must be a nonexact codegen-neutral allocation control"
        )
    return result


def _parse_exact(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "candidate_id",
        "objdiff_canonical_sha256",
        "source_sha256",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "candidate_record_sha256",
        "target_bytes",
        "candidate_bytes",
        "physical_relocations",
        "strict_percent",
        "data_percent",
        "zero_diff_rows",
        "protected_siblings_preserved",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    result = {
        "candidate_id": _text(item.get("candidate_id"), f"{label}.candidate_id", limit=128),
        **_hash_fields(
            item,
            label,
            (
                "objdiff_canonical_sha256",
                "source_sha256",
                "object_sha256",
                "strict_report_sha256",
                "data_report_sha256",
                "candidate_record_sha256",
            ),
        ),
        "target_bytes": _uint(item.get("target_bytes"), f"{label}.target_bytes", minimum=4),
        "candidate_bytes": _uint(
            item.get("candidate_bytes"), f"{label}.candidate_bytes", minimum=4
        ),
        "physical_relocations": _uint(
            item.get("physical_relocations"), f"{label}.physical_relocations", minimum=1
        ),
        "strict_percent": _number(item.get("strict_percent"), f"{label}.strict_percent"),
        "data_percent": _number(item.get("data_percent"), f"{label}.data_percent"),
        "zero_diff_rows": _bool(item.get("zero_diff_rows"), f"{label}.zero_diff_rows", True),
        "protected_siblings_preserved": _bool(
            item.get("protected_siblings_preserved"),
            f"{label}.protected_siblings_preserved",
            True,
        ),
    }
    if (
        result["target_bytes"] != result["candidate_bytes"]
        or result["strict_percent"] != 100.0
        or result["data_percent"] != 100.0
    ):
        raise ScoreDeltaInputError(f"{label} must be strict/data exact")
    return result


def _parse_trace(value: Any, label: str, bank: str) -> dict[str, Any]:
    fields = {
        "label",
        "trace_sha256",
        "envelope_sha256",
        "allocation_score",
        "bank",
        "physical_register",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    observed_bank = _text(item.get("bank"), f"{label}.bank", limit=3).upper()
    if observed_bank != bank:
        raise ScoreDeltaInputError(f"{label}.bank must equal focus bank {bank}")
    return {
        "label": _text(item.get("label"), f"{label}.label", limit=128),
        "trace_sha256": _sha256(item.get("trace_sha256"), f"{label}.trace_sha256"),
        "envelope_sha256": _sha256(
            item.get("envelope_sha256"), f"{label}.envelope_sha256"
        ),
        "allocation_score": _uint(
            item.get("allocation_score"), f"{label}.allocation_score", minimum=1
        ),
        "bank": bank,
        "physical_register": _register(
            item.get("physical_register"), bank, f"{label}.physical_register"
        ),
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "MWCC score-delta context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "focus",
        "baseline",
        "retained_control",
        "exact_result",
        "trace_comparison",
        "def_use_pairs",
        "telemetry",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise ScoreDeltaInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    _bool(context.get("authority_advanced"), f"{label}.authority_advanced", False)
    focus_item = _closed(
        context.get("focus"),
        allowed={"function", "owner", "bank"},
        required={"function", "owner", "bank"},
        label=f"{label}.focus",
    )
    bank = _text(focus_item.get("bank"), f"{label}.focus.bank", limit=3).upper()
    if bank not in _REGISTER_RE:
        raise ScoreDeltaInputError(f"{label}.focus.bank must be FPR or GPR")
    focus = {
        "function": _identifier(focus_item.get("function"), f"{label}.focus.function"),
        "owner": _identifier(focus_item.get("owner"), f"{label}.focus.owner"),
        "bank": bank,
    }
    baseline = _parse_baseline(context.get("baseline"), f"{label}.baseline")
    control = _parse_control(
        context.get("retained_control"), f"{label}.retained_control"
    )
    exact = _parse_exact(context.get("exact_result"), f"{label}.exact_result")
    if (
        baseline["target_bytes"] != control["target_bytes"]
        or baseline["target_bytes"] != exact["target_bytes"]
        or baseline["physical_relocations"] != control["physical_relocations"]
        or baseline["physical_relocations"] != exact["physical_relocations"]
        or control["allocation_rows_removed"] != len(baseline["score_residual_rows"])
        or control["remaining_non_score_rows"] != len(baseline["other_residual_rows"])
        or control["match_percent"] <= baseline["match_percent"]
    ):
        raise ScoreDeltaInputError(
            f"{label} baseline/control/exact proof chain is inconsistent"
        )

    comparison_item = _closed(
        context.get("trace_comparison"),
        allowed={"comparison_sha256", "baseline", "retained"},
        required={"comparison_sha256", "baseline", "retained"},
        label=f"{label}.trace_comparison",
    )
    baseline_trace = _parse_trace(
        comparison_item.get("baseline"), f"{label}.trace_comparison.baseline", bank
    )
    retained_trace = _parse_trace(
        comparison_item.get("retained"), f"{label}.trace_comparison.retained", bank
    )
    if baseline_trace["label"] == retained_trace["label"]:
        raise ScoreDeltaInputError(f"{label} trace labels must be distinct")
    score_delta = retained_trace["allocation_score"] - baseline_trace["allocation_score"]
    if score_delta <= 0 or score_delta % 2:
        raise ScoreDeltaInputError(
            f"{label} retained allocation score must have a positive even delta"
        )
    if baseline_trace["physical_register"] == retained_trace["physical_register"]:
        raise ScoreDeltaInputError(
            f"{label} score delta must change the authenticated physical color"
        )

    raw_pairs = context.get("def_use_pairs")
    if not isinstance(raw_pairs, list) or not raw_pairs:
        raise ScoreDeltaInputError(f"{label}.def_use_pairs must be a non-empty array")
    pairs: list[dict[str, Any]] = []
    seen_ids: set[str] = set()
    seen_lines: set[tuple[int, int]] = set()
    expected_source = f"{focus['owner']} = {focus['owner']};"
    for index, raw in enumerate(raw_pairs):
        pair_label = f"{label}.def_use_pairs[{index}]"
        item = _closed(
            raw,
            allowed={
                "pair_id",
                "source_sha256",
                "start_line",
                "end_line",
                "source_text",
                "source_text_sha256",
                "definition_increment",
                "use_increment",
            },
            required={
                "pair_id",
                "source_sha256",
                "start_line",
                "end_line",
                "source_text",
                "source_text_sha256",
                "definition_increment",
                "use_increment",
            },
            label=pair_label,
        )
        pair_id = _text(item.get("pair_id"), f"{pair_label}.pair_id", limit=128)
        if pair_id in seen_ids:
            raise ScoreDeltaInputError(f"{label}.def_use_pairs contains duplicate pair_id")
        seen_ids.add(pair_id)
        source_sha = _sha256(item.get("source_sha256"), f"{pair_label}.source_sha256")
        if source_sha != control["source_sha256"]:
            raise ScoreDeltaInputError(f"{pair_label} is not bound to the retained control source")
        start_line = _uint(item.get("start_line"), f"{pair_label}.start_line", minimum=1)
        end_line = _uint(item.get("end_line"), f"{pair_label}.end_line", minimum=start_line)
        if start_line != end_line or (start_line, end_line) in seen_lines:
            raise ScoreDeltaInputError(
                f"{pair_label} must bind one unique source line"
            )
        seen_lines.add((start_line, end_line))
        source_text = _text(item.get("source_text"), f"{pair_label}.source_text")
        source_text_sha = _sha256(
            item.get("source_text_sha256"), f"{pair_label}.source_text_sha256"
        )
        if source_text != expected_source or hashlib.sha256(source_text.encode("utf-8")).hexdigest() != source_text_sha:
            raise ScoreDeltaInputError(
                f"{pair_label} must seal the exact owner self-assignment source text"
            )
        definition_increment = _uint(
            item.get("definition_increment"),
            f"{pair_label}.definition_increment",
            minimum=1,
            maximum=1,
        )
        use_increment = _uint(
            item.get("use_increment"),
            f"{pair_label}.use_increment",
            minimum=1,
            maximum=1,
        )
        pairs.append(
            {
                "pair_id": pair_id,
                "source_sha256": source_sha,
                "start_line": start_line,
                "end_line": end_line,
                "source_text": source_text,
                "source_text_sha256": source_text_sha,
                "definition_increment": definition_increment,
                "use_increment": use_increment,
                "score_increment": definition_increment + use_increment,
            }
        )
    if pairs != sorted(pairs, key=lambda item: (item["start_line"], item["pair_id"])):
        raise ScoreDeltaInputError(f"{label}.def_use_pairs must be source ordered")
    if sum(item["score_increment"] for item in pairs) != score_delta or len(pairs) != score_delta // 2:
        raise ScoreDeltaInputError(
            f"{label} source-bound def/use pairs do not minimally explain the score delta"
        )

    telemetry_item = _closed(
        context.get("telemetry"),
        allowed={
            "candidate_count",
            "tracer_runs",
            "donor_searches",
            "telemetry_complete",
            "interval_log_sha256",
        },
        required={
            "candidate_count",
            "tracer_runs",
            "donor_searches",
            "telemetry_complete",
            "interval_log_sha256",
        },
        label=f"{label}.telemetry",
    )
    telemetry_complete = telemetry_item.get("telemetry_complete")
    if not isinstance(telemetry_complete, bool):
        raise ScoreDeltaInputError(f"{label}.telemetry.telemetry_complete must be boolean")
    telemetry = {
        "candidate_count": _uint(
            telemetry_item.get("candidate_count"), f"{label}.telemetry.candidate_count", minimum=1
        ),
        "tracer_runs": _uint(
            telemetry_item.get("tracer_runs"), f"{label}.telemetry.tracer_runs"
        ),
        "donor_searches": _uint(
            telemetry_item.get("donor_searches"), f"{label}.telemetry.donor_searches"
        ),
        "telemetry_complete": telemetry_complete,
        "interval_log_sha256": _sha256(
            telemetry_item.get("interval_log_sha256"),
            f"{label}.telemetry.interval_log_sha256",
        ),
    }
    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": _sha256(
            context.get("report_artifact_sha256"), f"{label}.report_artifact_sha256"
        ),
        "focus": focus,
        "baseline": baseline,
        "retained_control": control,
        "exact_result": exact,
        "trace_comparison": {
            "comparison_sha256": _sha256(
                comparison_item.get("comparison_sha256"),
                f"{label}.trace_comparison.comparison_sha256",
            ),
            "baseline": baseline_trace,
            "retained": retained_trace,
            "score_delta": score_delta,
            "physical_color_change": {
                "from": baseline_trace["physical_register"],
                "to": retained_trace["physical_register"],
            },
        },
        "def_use_pairs": pairs,
        "minimal_def_use_pair_count": len(pairs),
        "telemetry": telemetry,
        "authority_advanced": False,
    }


def _frame_size(instructions: Sequence[causal_reducer.Instruction]) -> int | None:
    for instruction in instructions[:16]:
        if instruction.has_instruction:
            match = _FRAME_RE.fullmatch(instruction.formatted)
            if match is not None:
                return int(match.group("size"), 0)
    return None


def _physical_relocations(instructions: Sequence[causal_reducer.Instruction]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _marked_rows(
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
) -> list[int]:
    return sorted(
        {
            instruction.index
            for instruction in (*target, *candidate)
            if instruction.diff_kind is not None
        }
    )


def evaluate(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {
            "matched": False,
            "reason": "no authenticated MWCC score-delta context was supplied",
        }
    focus = context["focus"]
    if pair.name != focus["function"]:
        return {"matched": False, "reason": "the score-delta context is bound to another function"}
    if objdiff_canonical_sha256 == context["exact_result"]["objdiff_canonical_sha256"]:
        return {
            "matched": False,
            "reason": "the authenticated function result is already exact; no score control is scheduled",
            "evidence": {
                "exact_result": context["exact_result"],
                "authority_advanced": False,
            },
        }
    if objdiff_canonical_sha256 == context["retained_control"]["objdiff_canonical_sha256"]:
        return {
            "matched": False,
            "reason": "the authenticated score delta is already present; hand off only the remaining non-score residual",
            "evidence": {
                "retained_control": context["retained_control"],
                "authority_advanced": False,
            },
        }
    baseline = context["baseline"]
    if objdiff_canonical_sha256 != baseline["objdiff_canonical_sha256"]:
        return {
            "matched": False,
            "reason": "the score-delta context is bound to another objdiff report",
        }
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    observed_signature = (
        target_size,
        candidate_size,
        _frame_size(target),
        _frame_size(candidate),
        _physical_relocations(target),
        _physical_relocations(candidate),
    )
    sealed_signature = (
        baseline["target_bytes"],
        baseline["candidate_bytes"],
        baseline["target_frame"],
        baseline["candidate_frame"],
        baseline["physical_relocations"],
        baseline["physical_relocations"],
    )
    if observed_signature != sealed_signature:
        return {
            "matched": False,
            "reason": "the exact size/frame/physical-relocation signature drifted",
            "evidence": {"observed": list(observed_signature), "sealed": list(sealed_signature)},
        }
    expected_rows = sorted(
        baseline["score_residual_rows"] + baseline["other_residual_rows"]
    )
    observed_rows = _marked_rows(target, candidate)
    if observed_rows != expected_rows:
        return {
            "matched": False,
            "reason": "the marked residual no longer matches the sealed score-plus-other partition",
            "evidence": {"observed_rows": observed_rows, "sealed_rows": expected_rows},
        }
    comparison = context["trace_comparison"]
    cell = {
        "order": 1,
        "kind": "source_bound_mwcc_score_delta_control",
        "owner": focus["owner"],
        "bank": focus["bank"],
        "allocation_score_before": comparison["baseline"]["allocation_score"],
        "allocation_score_after": comparison["retained"]["allocation_score"],
        "score_delta": comparison["score_delta"],
        "physical_register_before": comparison["baseline"]["physical_register"],
        "physical_register_after": comparison["retained"]["physical_register"],
        "minimal_def_use_pair_count": context["minimal_def_use_pair_count"],
        "source_spans": context["def_use_pairs"],
        "compile_as_one_diagnostic_cell": True,
        "retention_authorized": False,
    }
    return {
        "matched": True,
        "reason": (
            "same-session MWCC traces seal one frontend score delta, every increment is joined "
            "to an exact source span, and the bounded control removes only the allocation rows"
        ),
        "confidence": 1.0,
        "source_class": "source_bound_codegen_neutral_def_use_score_control",
        "recommendation": (
            f"Compile one diagnostic control for {focus['owner']}: add the sealed minimal "
            f"+{comparison['score_delta']} score from {context['minimal_def_use_pair_count']} "
            "def/use pairs, preserve every other source axis, and require the authenticated "
            "control result. Treat the spelling as function-scoped source debt, not provenance."
        ),
        "evidence": {
            "focus": focus,
            "baseline": baseline,
            "trace_comparison": comparison,
            "all_score_increments_source_bound": True,
            "def_use_pairs": context["def_use_pairs"],
            "recommended_cells": [cell],
            "suppressed_axes": [
                "declaration_chronology_matrix",
                "scope_permutations",
                "expression_spelling_matrix",
                "producer_staging_with_new_instructions",
                "repeat_tracer_capture",
                "dead_or_fake_local",
                "padding",
                "register_shaping",
                "automatic_source_retention",
                "automatic_promotion",
            ],
            "retained_control": context["retained_control"],
            "exact_result": context["exact_result"],
            "telemetry": context["telemetry"],
            "report_artifact_sha256": context["report_artifact_sha256"],
            "authority_advanced": False,
        },
    }
