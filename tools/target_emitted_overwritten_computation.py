#!/usr/bin/env python3
"""Fail-closed target-emitted computation diagnosis for MiracleSprUpdate.

The rule is deliberately program-point and owner-record bound.  It is not a
general dead-assignment detector and never grants admissibility or promotion.
"""

from __future__ import annotations

import math
import re
from typing import Any, Mapping, Sequence

from tools import mismatch_cluster_audit as causal_reducer


CONTEXT_SCHEMA = "target_emitted_overwritten_computation_context/v1"
RULE_ID = "target_emitted_overwritten_computation"
FUNCTION = "ev_CapMiracleSprUpdate"
SOURCE_STATEMENT = "scale = cos((M_PI * (90.0f * time)) / 180.0);"
TARGET_CHAIN = ("lfd", "lfs", "fmuls", "fmul", "lfd", "fdiv", "bl", "frsp")
REPORT_SHA256 = "41d8257182fac0f040b6888f5b3845ea05d5cde557d7e9b531d75080a0bf2bcd"
TARGET_OBJECT_SHA256 = "a1799b041c6bb18b9ea60410518007c90887510d9e07288cb9db373525c7679b"
BASELINE_SOURCE_SHA256 = "66bb9cf06e2a3c68b9636320cd018d7bca3802629851b6e14f4cd874f80084c3"
EXACT_HASHES = {
    "source_sha256": "6b46657f9d3556e3c081f46254efcfa47a879abc49defef3d57b551ae4496a9e",
    "object_sha256": "752c4cc0effd6c66952dc0dcc28ad1b6a36e4a64c3271ab70184969a3cdcd1f4",
    "strict_report_sha256": "d338bc37f66ac310210a56be0f6bcfc9e751dc431a8398cfdf8a96ad40790e9c",
    "data_report_sha256": "b24128811a8c83ad2ad3bdf00cd204239513b737e612fdd4b6249957c8a5a8ec",
    "compile_attestation_sha256": "e054bb40c45432720afbc09bc8fbc29094e14221a1a55e1511143aae3ac55e4f",
    "candidate_record_sha256": "e53085282a0deccfd80ccce069a282b517b6cb754e7147f19bf8d358a6f503fa",
    "policy_correction_record_sha256": "90572de02f98b6474c2176f62aa0b6d24224b42e6757d6debd8247cbdea8ab01",
    "parent_memo_sha256": "679925a56c4d36426c3a5524e924c20bacc73bcd0a16cf5c2957fda5e2b10a68",
}
TELEMETRY_SHA256 = "e37e4b38a30e6055cd4d5752a7e5d7f13d3ab83311cfda300f45860f6af48a46"
INTERVAL_PREFIX_SHA256 = "cfce1bf47ab5c13fa5748dbfe28cc60c1d308476b69dffb38595d7bb8e6c47fa"
FORBIDDEN_AXES = (
    "generic_dead_or_unused_assignment_insertion",
    "invented_call_or_constant",
    "invented_local_or_lifetime",
    "synthetic_cfg",
    "inline_assembly",
    "padding",
    "register_shaping",
    "broad_source_search",
    "tracer_or_donor_hunt",
    "automatic_source_retention",
    "promotion",
)

_SHA_RE = re.compile(r"[0-9a-f]{64}")
_OWNER_RE = re.compile(r"[A-Za-z0-9_./:+@#-]{1,192}")


class TargetEmittedOverwrittenInputError(ValueError):
    """The evidence cannot safely support this diagnosis."""


def _closed(value: Any, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise TargetEmittedOverwrittenInputError(f"{label} must be a JSON object")
    missing, extra = fields - set(value), set(value) - fields
    if missing or extra:
        raise TargetEmittedOverwrittenInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip() or len(value.strip()) > limit:
        raise TargetEmittedOverwrittenInputError(f"{label} must be bounded non-empty text")
    return value.strip()


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or _SHA_RE.fullmatch(value) is None:
        raise TargetEmittedOverwrittenInputError(f"{label} must be lowercase SHA-256")
    return value


def _owner(value: Any, label: str) -> str:
    result = _text(value, label, 192)
    if _OWNER_RE.fullmatch(result) is None:
        raise TargetEmittedOverwrittenInputError(f"{label} has invalid characters")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if type(value) is not bool or value is not expected:
        raise TargetEmittedOverwrittenInputError(f"{label} must be {expected}")
    return expected


def _uint(value: Any, label: str, minimum: int, maximum: int) -> int:
    if type(value) is not int or not minimum <= value <= maximum:
        raise TargetEmittedOverwrittenInputError(
            f"{label} must be an integer in [{minimum}, {maximum}]"
        )
    return value


def _number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TargetEmittedOverwrittenInputError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result) or result < 0:
        raise TargetEmittedOverwrittenInputError(f"{label} must be finite and non-negative")
    return result


def _sequence(value: Any, expected: Sequence[str], label: str) -> list[str]:
    if not isinstance(value, list):
        raise TargetEmittedOverwrittenInputError(f"{label} must be an array")
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if result != list(expected):
        raise TargetEmittedOverwrittenInputError(f"{label} must equal the sealed sequence")
    return result


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "target-emitted overwritten-computation context"
    root = _closed(
        value,
        {
            "schema", "owner", "function", "source_owner_task", "authority_advanced",
            "report_sha256", "toolchain", "provenance", "program_point",
            "target_chain", "baseline", "exact_result", "admissibility",
            "telemetry", "forbidden_axes",
        },
        label,
    )
    if _text(root["schema"], f"{label}.schema") != CONTEXT_SCHEMA:
        raise TargetEmittedOverwrittenInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    owner = _owner(root["owner"], f"{label}.owner")
    function = _text(root["function"], f"{label}.function", 128)
    if (owner, function) != ("main:board/capspecial", FUNCTION):
        raise TargetEmittedOverwrittenInputError(f"{label} owner/function drifted")
    source_owner_task = _owner(root["source_owner_task"], f"{label}.source_owner_task")
    _bool(root["authority_advanced"], f"{label}.authority_advanced", False)
    if _sha(root["report_sha256"], f"{label}.report_sha256") != REPORT_SHA256:
        raise TargetEmittedOverwrittenInputError(f"{label}.report_sha256 drifted")

    tool_raw = _closed(root["toolchain"], {"target_object_sha256", "compile_context_authenticated"}, f"{label}.toolchain")
    toolchain = {
        "target_object_sha256": _sha(tool_raw["target_object_sha256"], f"{label}.toolchain.target_object_sha256"),
        "compile_context_authenticated": _bool(tool_raw["compile_context_authenticated"], f"{label}.toolchain.compile_context_authenticated", True),
    }
    if toolchain["target_object_sha256"] != TARGET_OBJECT_SHA256:
        raise TargetEmittedOverwrittenInputError(f"{label}.toolchain target drifted")

    provenance_raw = _closed(
        root["provenance"],
        {"graphify_status", "graft_ask_count", "graft_status", "narrow_named_file_verified", "broad_searches"},
        f"{label}.provenance",
    )
    provenance = {
        "graphify_status": _text(provenance_raw["graphify_status"], f"{label}.provenance.graphify_status"),
        "graft_ask_count": _uint(provenance_raw["graft_ask_count"], f"{label}.provenance.graft_ask_count", 1, 1),
        "graft_status": _text(provenance_raw["graft_status"], f"{label}.provenance.graft_status"),
        "narrow_named_file_verified": _bool(provenance_raw["narrow_named_file_verified"], f"{label}.provenance.narrow_named_file_verified", True),
        "broad_searches": _uint(provenance_raw["broad_searches"], f"{label}.provenance.broad_searches", 0, 0),
    }
    if (provenance["graphify_status"], provenance["graft_status"]) != ("no_graph", "no_relevant_nodes"):
        raise TargetEmittedOverwrittenInputError(f"{label}.provenance no-hit boundary drifted")

    point_raw = _closed(
        root["program_point"],
        {
            "switch_case", "source_line", "after_statement", "before_condition",
            "destination", "input", "source_statement", "result_read_before_overwrite",
            "later_source_consumer_exists", "overwritten_before_later_consumer",
        },
        f"{label}.program_point",
    )
    program_point = {
        "switch_case": _uint(point_raw["switch_case"], f"{label}.program_point.switch_case", 64, 64),
        "source_line": _uint(point_raw["source_line"], f"{label}.program_point.source_line", 2910, 2910),
        "after_statement": _text(point_raw["after_statement"], f"{label}.program_point.after_statement"),
        "before_condition": _text(point_raw["before_condition"], f"{label}.program_point.before_condition"),
        "destination": _text(point_raw["destination"], f"{label}.program_point.destination", 32),
        "input": _text(point_raw["input"], f"{label}.program_point.input", 32),
        "source_statement": _text(point_raw["source_statement"], f"{label}.program_point.source_statement"),
        "result_read_before_overwrite": _bool(point_raw["result_read_before_overwrite"], f"{label}.program_point.result_read_before_overwrite", False),
        "later_source_consumer_exists": _bool(point_raw["later_source_consumer_exists"], f"{label}.program_point.later_source_consumer_exists", True),
        "overwritten_before_later_consumer": _bool(point_raw["overwritten_before_later_consumer"], f"{label}.program_point.overwritten_before_later_consumer", True),
    }
    expected_point = {
        "switch_case": 64,
        "source_line": 2910,
        "after_statement": "time = (float)work->time / 60.0f;",
        "before_condition": "if (time < 1.0f)",
        "destination": "scale",
        "input": "time",
        "source_statement": SOURCE_STATEMENT,
        "result_read_before_overwrite": False,
        "later_source_consumer_exists": True,
        "overwritten_before_later_consumer": True,
    }
    if program_point != expected_point:
        raise TargetEmittedOverwrittenInputError(f"{label}.program_point drifted")

    chain_raw = _closed(
        root["target_chain"],
        {"row_start", "row_end", "mnemonics", "call", "instruction_count", "byte_delta", "preceding_instruction", "following_instruction"},
        f"{label}.target_chain",
    )
    target_chain = {
        "row_start": _uint(chain_raw["row_start"], f"{label}.target_chain.row_start", 608, 608),
        "row_end": _uint(chain_raw["row_end"], f"{label}.target_chain.row_end", 615, 615),
        "mnemonics": _sequence(chain_raw["mnemonics"], TARGET_CHAIN, f"{label}.target_chain.mnemonics"),
        "call": _text(chain_raw["call"], f"{label}.target_chain.call", 64),
        "instruction_count": _uint(chain_raw["instruction_count"], f"{label}.target_chain.instruction_count", 8, 8),
        "byte_delta": _uint(chain_raw["byte_delta"], f"{label}.target_chain.byte_delta", 32, 32),
        "preceding_instruction": _text(chain_raw["preceding_instruction"], f"{label}.target_chain.preceding_instruction"),
        "following_instruction": _text(chain_raw["following_instruction"], f"{label}.target_chain.following_instruction"),
    }
    if (
        target_chain["call"] != "cos"
        or target_chain["preceding_instruction"] != "fdivs f30, f1, f0"
        or target_chain["following_instruction"] != "lfs f0, lbl_802C42C8@sda21"
    ):
        raise TargetEmittedOverwrittenInputError(f"{label}.target_chain anchors drifted")

    baseline_raw = _closed(
        root["baseline"],
        {"candidate_id", "objdiff_canonical_sha256", "source_sha256", "object_sha256", "strict_report_sha256", "target_size", "candidate_size", "strict_match_percent"},
        f"{label}.baseline",
    )
    baseline = {
        "candidate_id": _owner(baseline_raw["candidate_id"], f"{label}.baseline.candidate_id"),
        "objdiff_canonical_sha256": _sha(baseline_raw["objdiff_canonical_sha256"], f"{label}.baseline.objdiff_canonical_sha256"),
        "source_sha256": _sha(baseline_raw["source_sha256"], f"{label}.baseline.source_sha256"),
        "object_sha256": _sha(baseline_raw["object_sha256"], f"{label}.baseline.object_sha256"),
        "strict_report_sha256": _sha(baseline_raw["strict_report_sha256"], f"{label}.baseline.strict_report_sha256"),
        "target_size": _uint(baseline_raw["target_size"], f"{label}.baseline.target_size", 2780, 2780),
        "candidate_size": _uint(baseline_raw["candidate_size"], f"{label}.baseline.candidate_size", 2748, 2748),
        "strict_match_percent": _number(baseline_raw["strict_match_percent"], f"{label}.baseline.strict_match_percent"),
    }
    if baseline["source_sha256"] != BASELINE_SOURCE_SHA256 or not math.isclose(baseline["strict_match_percent"], 98.84892, abs_tol=1e-6):
        raise TargetEmittedOverwrittenInputError(f"{label}.baseline contract drifted")

    exact_fields = {
        "candidate_id", "objdiff_canonical_sha256", *EXACT_HASHES,
        "target_size", "candidate_size", "physical_relocations", "protected_siblings", "owner_exact_frontier",
    }
    exact_raw = _closed(root["exact_result"], set(exact_fields), f"{label}.exact_result")
    exact = {
        "candidate_id": _owner(exact_raw["candidate_id"], f"{label}.exact_result.candidate_id"),
        "objdiff_canonical_sha256": _sha(exact_raw["objdiff_canonical_sha256"], f"{label}.exact_result.objdiff_canonical_sha256"),
        **{key: _sha(exact_raw[key], f"{label}.exact_result.{key}") for key in EXACT_HASHES},
        "target_size": _uint(exact_raw["target_size"], f"{label}.exact_result.target_size", 2780, 2780),
        "candidate_size": _uint(exact_raw["candidate_size"], f"{label}.exact_result.candidate_size", 2780, 2780),
        "physical_relocations": _uint(exact_raw["physical_relocations"], f"{label}.exact_result.physical_relocations", 172, 172),
        "protected_siblings": _text(exact_raw["protected_siblings"], f"{label}.exact_result.protected_siblings"),
        "owner_exact_frontier": _text(exact_raw["owner_exact_frontier"], f"{label}.exact_result.owner_exact_frontier"),
    }
    if any(exact[key] != expected for key, expected in EXACT_HASHES.items()):
        raise TargetEmittedOverwrittenInputError(f"{label}.exact_result identity drifted")
    if (exact["protected_siblings"], exact["owner_exact_frontier"]) != ("37/37", "38/44"):
        raise TargetEmittedOverwrittenInputError(f"{label}.exact_result frontier drifted")

    admissibility_raw = _closed(
        root["admissibility"],
        {"owner_decision", "retained_record_bound", "policy_correction_supersedes_quarantine", "independent_test_pass", "exact_bytes_alone_sufficient", "blanket_dead_assignment_waiver", "requires_owner_local_review"},
        f"{label}.admissibility",
    )
    admissibility = {
        "owner_decision": _text(admissibility_raw["owner_decision"], f"{label}.admissibility.owner_decision"),
        "retained_record_bound": _bool(admissibility_raw["retained_record_bound"], f"{label}.admissibility.retained_record_bound", True),
        "policy_correction_supersedes_quarantine": _bool(admissibility_raw["policy_correction_supersedes_quarantine"], f"{label}.admissibility.policy_correction_supersedes_quarantine", True),
        "independent_test_pass": _bool(admissibility_raw["independent_test_pass"], f"{label}.admissibility.independent_test_pass", True),
        "exact_bytes_alone_sufficient": _bool(admissibility_raw["exact_bytes_alone_sufficient"], f"{label}.admissibility.exact_bytes_alone_sufficient", False),
        "blanket_dead_assignment_waiver": _bool(admissibility_raw["blanket_dead_assignment_waiver"], f"{label}.admissibility.blanket_dead_assignment_waiver", False),
        "requires_owner_local_review": _bool(admissibility_raw["requires_owner_local_review"], f"{label}.admissibility.requires_owner_local_review", True),
    }
    if admissibility["owner_decision"] != "retained":
        raise TargetEmittedOverwrittenInputError(f"{label}.admissibility owner decision drifted")

    telemetry_raw = _closed(
        root["telemetry"],
        {"mixed_parent_interval_seconds", "complete_decision_interval_seconds", "compile_heavy_seconds", "telemetry_complete", "crack_hour_eligible", "no_imputation", "telemetry_receipt_sha256", "active_interval_prefix_sha256"},
        f"{label}.telemetry",
    )
    telemetry = {
        "mixed_parent_interval_seconds": _number(telemetry_raw["mixed_parent_interval_seconds"], f"{label}.telemetry.mixed_parent_interval_seconds"),
        "complete_decision_interval_seconds": _number(telemetry_raw["complete_decision_interval_seconds"], f"{label}.telemetry.complete_decision_interval_seconds"),
        "compile_heavy_seconds": _number(telemetry_raw["compile_heavy_seconds"], f"{label}.telemetry.compile_heavy_seconds"),
        "telemetry_complete": _bool(telemetry_raw["telemetry_complete"], f"{label}.telemetry.telemetry_complete", False),
        "crack_hour_eligible": _bool(telemetry_raw["crack_hour_eligible"], f"{label}.telemetry.crack_hour_eligible", False),
        "no_imputation": _bool(telemetry_raw["no_imputation"], f"{label}.telemetry.no_imputation", True),
        "telemetry_receipt_sha256": _sha(telemetry_raw["telemetry_receipt_sha256"], f"{label}.telemetry.telemetry_receipt_sha256"),
        "active_interval_prefix_sha256": _sha(telemetry_raw["active_interval_prefix_sha256"], f"{label}.telemetry.active_interval_prefix_sha256"),
    }
    if (
        not math.isclose(telemetry["mixed_parent_interval_seconds"], 7365.1560854, abs_tol=1e-7)
        or not math.isclose(telemetry["complete_decision_interval_seconds"], 978.2983101, abs_tol=1e-7)
        or not math.isclose(telemetry["compile_heavy_seconds"], 0.2821299, abs_tol=1e-7)
        or telemetry["telemetry_receipt_sha256"] != TELEMETRY_SHA256
        or telemetry["active_interval_prefix_sha256"] != INTERVAL_PREFIX_SHA256
    ):
        raise TargetEmittedOverwrittenInputError(f"{label}.telemetry drifted")

    return {
        "schema": CONTEXT_SCHEMA,
        "owner": owner,
        "function": function,
        "source_owner_task": source_owner_task,
        "authority_advanced": False,
        "report_sha256": REPORT_SHA256,
        "toolchain": toolchain,
        "provenance": provenance,
        "program_point": program_point,
        "target_chain": target_chain,
        "baseline": baseline,
        "exact_result": exact,
        "admissibility": admissibility,
        "telemetry": telemetry,
        "forbidden_axes": _sequence(root["forbidden_axes"], FORBIDDEN_AXES, f"{label}.forbidden_axes"),
    }


def _size(symbol: Mapping[str, Any] | None) -> int | None:
    return causal_reducer._parse_number(symbol.get("size")) if symbol else None


def _match(symbol: Mapping[str, Any] | None) -> float | None:
    if symbol is None:
        return None
    value = symbol.get("match_percent")
    return float(value) if isinstance(value, (int, float)) and not isinstance(value, bool) else None


def _relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1 for instruction in instructions
        if instruction.relocation and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def _mismatches(target: Sequence[Any], candidate: Sequence[Any]) -> list[tuple[int, Any, Any]]:
    return [
        (index, left, right)
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if causal_reducer._instruction_mismatch(left, right)
    ]


def _gap(instruction: Any) -> bool:
    return instruction is None or not instruction.has_instruction


def _reported_residuals(
    target: Sequence[Any], candidate: Sequence[Any]
) -> list[tuple[int, Any, Any]]:
    return [
        (index, left, right)
        for index, (left, right) in enumerate(causal_reducer._paired_records(target, candidate))
        if left is None
        or right is None
        or left.has_instruction != right.has_instruction
        or causal_reducer._is_diff_kind(left.diff_kind)
        or causal_reducer._is_diff_kind(right.diff_kind)
    ]


def _chain_matches(target: Sequence[Any], candidate: Sequence[Any], context: Mapping[str, Any]) -> bool:
    pairs = list(causal_reducer._paired_records(target, candidate))
    chain = context["target_chain"]
    start, end = chain["row_start"], chain["row_end"]
    if start <= 0 or end + 1 >= len(pairs):
        return False
    observed: list[str] = []
    for row in range(start, end + 1):
        left, right = pairs[row]
        if left is None or not left.has_instruction or not _gap(right):
            return False
        observed.append(left.mnemonic)
    if observed != chain["mnemonics"]:
        return False
    before = pairs[start - 1]
    after = pairs[end + 1]
    if any(item is None or not item.has_instruction for item in (*before, *after)):
        return False
    if before[0].formatted != chain["preceding_instruction"] or before[1].formatted != chain["preceding_instruction"]:
        return False
    if (
        after[0].formatted != chain["following_instruction"]
        or after[1].mnemonic != "lfs"
    ):
        return False
    call = pairs[start + 6][0]
    return call.mnemonic == "bl" and re.search(r"\bcos\b", call.formatted) is not None


def evaluate(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {"matched": False, "reason": "no authenticated target-emitted overwritten-computation context was supplied"}
    if pair.name != context["function"]:
        return {"matched": False, "reason": "the context is bound to another function"}
    residuals = _mismatches(target, candidate)
    reported_residuals = _reported_residuals(target, candidate)
    observed = (_size(pair.target), _size(pair.candidate), _relocations(target), _relocations(candidate), _match(pair.candidate))

    exact = context["exact_result"]
    if objdiff_canonical_sha256 == exact["objdiff_canonical_sha256"]:
        if observed[:4] != (2780, 2780, 172, 172) or not math.isclose(observed[4] or -1.0, 100.0, abs_tol=1e-9) or reported_residuals:
            return {"matched": False, "reason": "the sealed exact result drifted", "evidence": {"observed_signature": list(observed), "residual_count": len(reported_residuals)}}
        return {
            "matched": False,
            "reason": "the owner-retained program point is already exact; no candidate is scheduled",
            "evidence": {"owner_admissibility_required": True, "blanket_dead_assignment_waiver": False, "telemetry": context["telemetry"], "authority_advanced": False},
        }

    baseline = context["baseline"]
    if objdiff_canonical_sha256 != baseline["objdiff_canonical_sha256"]:
        return {"matched": False, "reason": "the report matches neither the sealed baseline nor exact result"}
    if observed[:2] != (2780, 2748) or not math.isclose(observed[4] or -1.0, 98.84892, abs_tol=1e-6) or not _chain_matches(target, candidate, context):
        return {"matched": False, "reason": "the baseline size, score, target-only chain, call, or anchors drifted", "evidence": {"observed_signature": list(observed), "residual_count": len(residuals)}}
    return {
        "matched": True,
        "reason": "the authenticated baseline omits the sealed target-only cosine chain at the owner-retained program point",
        "evidence": {
            "stage": RULE_ID,
            "program_point": context["program_point"],
            "target_chain": context["target_chain"],
            "recommended_cells": [{"kind": "restore_owner_retained_target_emitted_computation", "source_statement": SOURCE_STATEMENT, "after_statement": context["program_point"]["after_statement"], "before_condition": context["program_point"]["before_condition"]}],
            "requires_owner_admissibility_record": True,
            "blanket_dead_assignment_waiver": False,
            "suppress_tracer": True,
            "provenance": context["provenance"],
            "admissibility": context["admissibility"],
            "telemetry": context["telemetry"],
            "forbidden_axes": context["forbidden_axes"],
            "authority_advanced": False,
        },
    }
