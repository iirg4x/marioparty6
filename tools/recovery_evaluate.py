"""One-command private recovery measurement; no live source writes or permits.

Consumes the existing current-evidence index, compiles one supplied natural-C
candidate (or measures an existing object), and compares the whole owner. Source
selection and final source fidelity remain the owner's job. This is not a source
generator, an exactness oracle, or a promotion mechanism.
Compiler recipes are trusted owner input, not arbitrary programs sandboxed by
this module. The evaluator itself writes only its private build artifacts.
"""
from __future__ import annotations

from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import sys
import tempfile
import time
from typing import Any, Callable

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import bounded_process
from tools import compile_recovery_candidate as compiler
from tools import owner_campaign
from tools import recovery_frontier as frontier
from tools import recovery_object_inventory as objects
from tools import recovery_causal_groups as causal_groups

SCHEMA = "recovery_candidate_evaluation/v1"
BATCH_SCHEMA = "recovery_evaluation_batch/v1"
BATCH_EVENT_SCHEMA = "recovery_evaluation_event/v1"
PREDICTION_SCHEMA = "recovery_prediction/v1"
LIMIT = 512 * 1024
SEEN_LIMIT = 128
BATCH_MAX_JOBS = 8
BATCH_MAX_WORKERS = 3
_PREDICTION_LIMIT = 64 * 1024
_PREDICTION_SITE_LIMIT = 32
_PREDICTION_FUNCTION_LIMIT = 128
_PREDICTION_SIGNATURE_LIMIT = 1024
_SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
_DIAGNOSTIC_FUNCTION_LIMIT = 3
_DIAGNOSTIC_CONTEXT_LIMIT = 2
_EVALUATE_STDOUT_LIMIT = 12 * 1024
_DIAGNOSTIC_SITE_LIMIT = 8
_PAIRED_MEMORY_MNEMONICS = frozenset({"psq_l", "psq_lx", "psq_st", "psq_stx"})
_SCALAR_ABS_CONVERSION_MNEMONICS = frozenset({"fabs", "frsp"})
_SCALAR_MEMORY_MNEMONICS = frozenset(
    opcode for opcode in frontier._STACK_ACCESS_WIDTHS
    if not opcode.startswith("psq_")
    and opcode not in {"lvx", "lvxl", "stvx", "stvxl"}
)
_SCALAR_UNARY_MNEMONICS = frozenset({
    "fabs", "fctiw", "fctiwz", "fmr", "fneg", "fnabs", "fres", "frsp", "frsqrte",
})
_STACK_BASE_REGISTER_RE = re.compile(r"(?<![A-Za-z0-9_])r1(?![A-Za-z0-9_])", re.IGNORECASE)
_BATCH_ID = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,63}\Z")
_BATCH_DEVICE_IDS = {"CON", "PRN", "AUX", "NUL"} | {
    f"{prefix}{number}" for prefix in ("COM", "LPT") for number in range(1, 10)
}


def _sha(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _descriptor(path: Path) -> dict:
    return {"path": str(path), "sha256": compiler.digest(path), "size_bytes": path.stat().st_size}


def _implementation_binding() -> dict[str, str]:
    # A parser/classifier fix must invalidate cached measurements even when
    # this entry point did not change in the same release.
    paths = {Path(__file__), Path(frontier.__file__), Path(objects.__file__),
             Path(compiler.__file__), Path(bounded_process.__file__),
             Path(frontier.focus.__file__), Path(causal_groups.__file__),
             Path(__file__).with_name("crack_evidence_bundle.py")}
    return {str(path): compiler.digest(path) for path in sorted(paths)}


def _atomic(path: Path, value: dict) -> None:
    data = frontier.canonical(value) + b"\n"
    if len(data) > LIMIT:
        raise ValueError(f"compact evaluation exceeds {LIMIT} bytes: {path}")
    compiler.atomic(path, data)


def _seen(root: Path, path: Path, key: str) -> dict | None:
    if not path.exists():
        return None
    raw, _ = frontier.read_bound(root, path, LIMIT)
    cache = frontier.load_json(raw)
    if cache.get("schema") != "recovery_evaluation_cache/v1":
        raise ValueError(f"invalid evaluation cache: {path}")
    for item in cache.get("entries", []):
        if item.get("key") != key:
            continue
        result_path = frontier.local(root, path.parent / item["result"])
        if result_path.parent != path.parent:
            raise ValueError("evaluation cache result escapes its directory")
        if result_path.is_file() and compiler.digest(result_path) == item.get("sha256"):
            old = frontier.load_json(result_path.read_bytes())
            if old.get("context_key") == key and old.get("status") != "failed":
                return {"path": str(result_path), "sha256": item["sha256"],
                        "status": old["status"], "candidate_sha256": old["candidate_source"]["sha256"]}
    return None


def _remember(root: Path, path: Path, key: str, out: Path) -> None:
    # Compact lookup, not an attempt cap. A different source/context is always
    # eligible; dropping an old lookup never bans a function.
    with owner_campaign._exclusive_lock(path.with_suffix(".lock"), timeout=5):
        if path.exists():
            raw, _ = frontier.read_bound(root, path, LIMIT)
            cache = frontier.load_json(raw)
            if cache.get("schema") != "recovery_evaluation_cache/v1":
                raise ValueError(f"invalid evaluation cache: {path}")
            entries = cache.get("entries", [])
        else:
            entries = []
        entries = [entry for entry in entries if entry.get("key") != key]
        entries.append({"key": key, "result": out.name, "sha256": compiler.digest(out)})
        _atomic(path, {"schema": "recovery_evaluation_cache/v1", "entries": entries[-SEEN_LIMIT:]})


def _prediction_address(value: Any, label: str) -> int:
    if isinstance(value, bool):
        raise ValueError(f"{label} must be a non-negative instruction address")
    if isinstance(value, int):
        address = value
    elif isinstance(value, str):
        text = value.strip()
        try:
            address = int(text, 16 if text.lower().startswith("0x") else 10)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"{label} must be a non-negative instruction address") from exc
    else:
        raise ValueError(f"{label} must be a non-negative instruction address")
    if not 0 <= address < 2 ** 64:
        raise ValueError(f"{label} is outside the uint64 address range")
    return address


def _prediction_signature(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{label} must be non-empty instruction text")
    value = " ".join(value.split())
    if len(value) > _PREDICTION_SIGNATURE_LIMIT:
        raise ValueError(f"{label} exceeds {_PREDICTION_SIGNATURE_LIMIT} characters")
    return value


def _prediction_row_signature(row: Any) -> str | None:
    instruction = row.get("instruction") if isinstance(row, dict) else None
    value = instruction.get("formatted") if isinstance(instruction, dict) else None
    return " ".join(value.split())[:_PREDICTION_SIGNATURE_LIMIT] if isinstance(value, str) and value.strip() else None


def _load_prediction(root: Path, path: Path | None) -> tuple[dict | None, dict | None, str | None]:
    if path is None:
        return None, None, None
    try:
        raw, descriptor = frontier.read_bound(root, path, _PREDICTION_LIMIT)
        document = frontier.load_json(raw)
        if not isinstance(document, dict) or document.get("schema") != PREDICTION_SCHEMA:
            raise ValueError(f"prediction schema must be {PREDICTION_SCHEMA}")
        def sha(field: str) -> str:
            value = document.get(field)
            if not isinstance(value, str) or _SHA256_RE.fullmatch(value) is None:
                raise ValueError(f"prediction {field} must be a lowercase SHA-256")
            return value
        baseline_sha, source_sha = sha("baseline_index_sha256"), sha("candidate_source_sha256")
        functions, expected = document.get("functions"), document.get("expected")
        if (not isinstance(functions, list) or not functions
                or len(functions) > _PREDICTION_FUNCTION_LIMIT
                or any(not isinstance(value, str) or not value.strip() for value in functions)
                or len(set(functions)) != len(functions)):
            raise ValueError("prediction functions must be distinct non-empty names")
        if not isinstance(expected, list) or not expected or len(expected) > _PREDICTION_FUNCTION_LIMIT:
            raise ValueError(f"prediction expected must contain 1..{_PREDICTION_FUNCTION_LIMIT} items")
        functions = [value.strip() for value in functions]
        normalized, seen = [], set()
        for number, item in enumerate(expected, 1):
            if not isinstance(item, dict):
                raise ValueError(f"prediction expected instruction {number} must be an object")
            function = item.get("function")
            if not isinstance(function, str) or not function.strip():
                raise ValueError(f"prediction expected instruction {number} function is required")
            function = function.strip()
            address = _prediction_address(
                item.get("target_address"), f"prediction expected instruction {number} address"
            )
            signature = _prediction_signature(
                item.get("target_signature"), f"prediction expected instruction {number} signature"
            )
            if (function, address) in seen:
                raise ValueError(f"prediction expected instruction {number} duplicates {function}@{address}")
            seen.add((function, address))
            normalized.append({"function": function, "address": address, "signature": signature})
        if not {item["function"] for item in normalized} <= set(functions):
            raise ValueError("prediction expected instruction function is outside functions")
        return {
            "schema": PREDICTION_SCHEMA, "baseline_index_sha256": baseline_sha,
            "candidate_source_sha256": source_sha, "functions": functions, "expected": normalized,
        }, descriptor, None
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as exc:
        return None, None, str(exc)[:800]


def _prediction_report_rows(document: dict | None, function: str) -> tuple[list[dict], list[dict]] | None:
    if not isinstance(document, dict):
        return None
    sides = {side: frontier.focus._symbols(document, side, "prediction feedback")
             for side in ("left", "right")}
    target = frontier._stack_function(sides["left"], function, "target")
    candidate = frontier._stack_function(sides["right"], function, "candidate")
    return (frontier.focus._rows(target, function), frontier.focus._rows(candidate, function)) \
        if target is not None and candidate is not None else None


def _prediction_baseline_report(root: Path, base: dict) -> dict | None:
    descriptor = base.get("inputs", {}).get("strict_report")
    if not isinstance(descriptor, dict):
        return None
    try:
        raw, actual = frontier.read_bound(root, Path(descriptor["path"]), frontier.REPORT_LIMIT)
        document = frontier.load_json(raw)
        return document if actual == descriptor and isinstance(document, dict) else None
    except (OSError, ValueError, RuntimeError, KeyError, TypeError, json.JSONDecodeError):
        return None


def _prediction_feedback(*, spec: dict | None, prediction_descriptor: dict | None,
                         prediction_error: str | None, index_desc: dict, source_desc: dict,
                         functions: list[str], baseline_document: dict | None,
                         after_document: dict | None, evaluation_status: str | None) -> dict:
    """Compare predictions at target addresses, never at retained row numbers."""
    result: dict[str, Any] = {
        "schema": PREDICTION_SCHEMA, "diagnostic_only": True,
        "authority_advanced": False, "prediction": prediction_descriptor,
    }
    empty = {"closed": 0, "still_different": 0, "unmapped": 0}
    if prediction_error is not None:
        result.update(status="unmapped", binding_status="invalid",
                      reason=f"invalid prediction: {prediction_error}", sites=[], counts=empty)
        return result
    if spec is None:
        result.update(status="unmapped", binding_status="invalid",
                      reason="prediction was not loaded", sites=[], counts=empty)
        return result

    expected_index, actual_index = spec["baseline_index_sha256"], index_desc.get("sha256")
    expected_source, actual_source = spec["candidate_source_sha256"], source_desc.get("sha256")
    expected_functions = list(spec["functions"])
    result.update(
        binding={
            "baseline_index_sha256": {"expected": expected_index, "actual": actual_index,
                                      "matched": expected_index == actual_index},
            "candidate_source_sha256": {"expected": expected_source, "actual": actual_source,
                                        "matched": expected_source == actual_source},
            "functions": {"expected": expected_functions, "actual": list(functions),
                          "matched": set(expected_functions) == set(functions)},
        },
        functions=expected_functions, predicted_count=len(spec["expected"]),
    )
    reasons = []
    if expected_index != actual_index:
        reasons.append("baseline index file SHA-256 differs")
    if expected_source != actual_source:
        reasons.append("candidate source SHA-256 differs")
    if set(expected_functions) != set(functions):
        reasons.append("focus functions differ")
    expected = spec["expected"][:_PREDICTION_SITE_LIMIT]
    if reasons:
        sites = [{
            "function": item["function"], "target_address": item["address"],
            "expected_target_signature": item["signature"], "status": "unmapped",
            "reason": "; ".join(reasons)[:240],
        } for item in expected]
        result.update(status="unmapped", binding_status="drifted",
                      reason="prediction binding mismatch: " + "; ".join(reasons),
                      sites=sites, truncated=len(spec["expected"]) > len(sites),
                      counts={"closed": 0, "still_different": 0, "unmapped": len(sites)})
        return result

    report = after_document
    if report is None and evaluation_status in {"duplicate_object", "duplicate_source"}:
        report = baseline_document
    sites = []
    for item in expected:
        site = {
            "function": item["function"], "target_address": item["address"],
            "expected_target_signature": item["signature"], "status": "unmapped",
        }
        try:
            rows = _prediction_report_rows(report, item["function"])
            matches = []
            if rows is not None:
                for index, row in enumerate(rows[0]):
                    instruction = row.get("instruction") if isinstance(row, dict) else None
                    if not isinstance(instruction, dict):
                        continue
                    try:
                        address = _prediction_address(instruction.get("address"), "target instruction address")
                    except ValueError:
                        continue
                    if address == item["address"]:
                        matches.append((index, row))
            if len(matches) != 1:
                site["reason"] = ("target instruction address is missing" if not matches
                                  else "target instruction address is ambiguous")
            else:
                aligned_row, target_row = matches[0]
                candidate_row = rows[1][aligned_row] if aligned_row < len(rows[1]) else None
                target_signature = _prediction_row_signature(target_row)
                candidate_signature = _prediction_row_signature(candidate_row)
                site.update(aligned_row=aligned_row, target_signature=target_signature,
                            candidate_signature=candidate_signature)
                if target_signature is None:
                    site["reason"] = "target instruction signature is missing"
                elif target_signature != item["signature"]:
                    site["reason"] = "target instruction signature changed since prediction"
                elif candidate_signature is None:
                    site["reason"] = "aligned candidate instruction is missing"
                elif candidate_signature == item["signature"]:
                    site["status"] = "closed"
                else:
                    site["status"] = "still_different"
        except (ValueError, KeyError, TypeError, AttributeError) as exc:
            site["reason"] = f"aligned report is not usable: {str(exc)[:160]}"
        sites.append(site)

    counts = {status: sum(site["status"] == status for site in sites)
              for status in ("closed", "still_different", "unmapped")}
    status = "contradiction" if evaluation_status == "duplicate_object" and counts["still_different"] \
        else "unmapped" if counts["unmapped"] else "still_different" if counts["still_different"] else "closed"
    if status == "contradiction":
        result["contradiction"] = "semantic duplicate_object cannot satisfy a promised instruction change"
    result.update(status=status, binding_status="matched", sites=sites,
                  truncated=len(spec["expected"]) > len(sites), counts=counts)
    return result
def _metric_map(rows: list[dict]) -> dict[str, dict]:
    result = {row["function"]: dict(row) for row in rows}
    if len(result) != len(rows):
        raise ValueError("duplicate functions in evidence")
    for row in result.values():
        for field in ("target_bytes", "candidate_bytes", "diff_rows"):
            value = frontier.focus._integer(row[field])
            if value is None or value < 0:
                raise ValueError(f"invalid {field} for {row['function']}")
            row[field] = value
        value = row.get("match_percent")
        if value is not None and (isinstance(value, bool) or not isinstance(value, (int, float))
                                  or not math.isfinite(value) or not 0 <= value <= 100):
            raise ValueError(f"invalid percentage for {row['function']}")
    return result


def _qualified_positional_shift(name: str, row: dict, comparison: dict,
                                channels: list[tuple[list[dict], list[dict]]],
                                focus: set[str]) -> bool:
    """Partial-gain exception; never an exactness proof or a reference waiver."""
    if (name not in focus or row.get("raw_equal_base") is not False
            or row.get("base_raw_exact_target") is not False
            or row.get("ordered_normalized_relocations_equal") is not True
            or comparison.get("function_census_equal") is not True
            or (comparison.get("allocated_nontext_changed") is not False
                and comparison.get("allocated_nontext_relocations_only_code_motion") is not True)):
        return False
    sizes = [frontier.focus._integer(row.get(key)) for key in
             ("target_size", "base_size", "candidate_size")]
    if any(value is None or value < 0 for value in sizes):
        return False
    target, base, candidate = sizes
    if (comparison.get("allocated_nontext_changed") is False
            and comparison.get("function_layout_equal") is not True
            and comparison.get("allocated_nontext_normalized_relocations_equal") is not True):
        return False
    if base == target and candidate != target:
        return False
    size_closer = abs(candidate - target) < abs(base - target)
    strict_gain = True
    for before, after in channels:
        left, right = _metric_map(before), _metric_map(after)
        if name not in left or name not in right:
            return False
        a, b = left[name], right[name]
        if a["instruction_exact"] or b["diff_rows"] > a["diff_rows"]:
            return False
        strict_gain &= b["diff_rows"] < a["diff_rows"]
    canonical = [frontier.focus._integer(row.get(key)) for key in
                 ("normalized_diff_before", "normalized_diff_after")]
    if comparison.get("allocated_nontext_changed") is not False:
        size_closer &= len(channels) == 2 and strict_gain
    if not size_closer and not (len(channels) == 2 and strict_gain
            and all(value is not None and value >= 0 for value in canonical)
            and canonical[1] < canonical[0]):
        return False
    for sibling in comparison.get("functions", {}).values():
        if sibling.get("base_raw_exact_target") and sibling.get("base_normalized_exact"):
            if (sibling.get("raw_equal_base") is not True
                    or sibling.get("candidate_normalized_exact") is not True):
                return False
    return True


def _code_quality(before: dict, after: dict, name: str) -> dict:
    """Measure report-backed closures, never source intent or score alone."""
    result: dict[str, Any] = {"function": name, "status": "unknown",
                              "closed_opcodes": 0, "closed_operands": 0, "losses": 0,
                              "sites": [], "diagnostic_only": True, "authority_advanced": False}
    try:
        old, new = _diagnostic_rows(before, name), _diagnostic_rows(after, name)
        if old is None or new is None:
            raise ValueError("function report rows unavailable")

        def anchors(rows: list[dict]) -> dict[int, tuple[int, dict]]:
            entries = {}
            for index, row in enumerate(rows):
                payload = frontier._diagnose_payload(row, index)
                if payload is None:
                    continue
                address = _prediction_address(row["instruction"].get("address"), "instruction address")
                if address in entries:
                    raise ValueError("ambiguous target address")
                entries[address] = (index, payload)
            return entries

        left, right = anchors(old[0]), anchors(new[0])
        if not left or [(k, v[1]) for k, v in left.items()] != [(k, v[1]) for k, v in right.items()]:
            raise ValueError("target instruction stream changed")
        complete = len(old[0]) == len(old[1]) and len(new[0]) == len(new[1])
        # Existing unchanged gaps are unresolved evidence, not new losses.
        # New/moved gaps remain diagnostic-only, as do changes to unanchored
        # inserted instructions that cannot be ranked against a target site.
        complete = complete and len(old[0]) == len(new[0])
        for side in (0, 1):
            complete = complete and [i for i, r in enumerate(old[side]) if not r.get("instruction")] == [
                i for i, r in enumerate(new[side]) if not r.get("instruction")]
        for index, row in enumerate(old[0]):
            if not row.get("instruction") and index < len(new[1]) and index < len(old[1]):
                complete = complete and frontier._diagnose_payload(old[1][index], index) == frontier._diagnose_payload(new[1][index], index)
        old_candidates, new_candidates = anchors(old[1]), anchors(new[1])
        if not old_candidates or not new_candidates:
            raise ValueError("candidate instruction stream unavailable")
        old_origin, new_origin = min(old_candidates), min(new_candidates)
        for address, (old_index, _) in left.items():
            new_index = right[address][0]
            target = old[0][old_index]
            a = old[1][old_index] if old_index < len(old[1]) else {}
            b = new[1][new_index] if new_index < len(new[1]) else {}
            parsed = [frontier._instruction_parts(row) for row in (target, a, b)]
            if not a.get("instruction") and not b.get("instruction") and a == b:
                continue
            if any(value is None for value in parsed):
                complete = False
                continue
            target_parts, before_parts, after_parts = parsed
            a_address = _prediction_address(a["instruction"].get("address"), "candidate address")
            b_address = _prediction_address(b["instruction"].get("address"), "candidate address")
            if a_address - old_origin != b_address - new_origin:
                complete = False
            if (frontier._diagnose_payload(a, old_index) == frontier._diagnose_payload(b, new_index)
                    and a.get("arg_diff") == b.get("arg_diff")
                    and a.get("diff_kind") == b.get("diff_kind")):
                # An unchanged unresolved site is not a new loss. It need not
                # become interpretable merely to credit a different closure.
                continue
            old_opcode = before_parts[0] != target_parts[0]
            new_opcode = after_parts[0] != target_parts[0]
            closed_opcodes, closed_operands, losses = 0, 0, 0
            if old_opcode and not new_opcode:
                closed_opcodes = 1
            elif not old_opcode and new_opcode:
                losses = 1
            elif not old_opcode and not new_opcode:
                def operands(row: dict, parts: tuple) -> set[int] | None:
                    # Objdiff's argument flags respect its relocation channel;
                    # formatted symbolic aliases alone cannot prove a closure.
                    flags = row.get("arg_diff")
                    if isinstance(flags, list) and all(isinstance(flag, dict) for flag in flags):
                        return {i for i, flag in enumerate(flags) if "diff_index" in flag}
                    if row.get("diff_kind") in (None, "DIFF_NONE") and parts == target_parts:
                        return set()
                    return None
                old_args, new_args = operands(a, before_parts), operands(b, after_parts)
                if old_args is None or new_args is None:
                    complete = False
                    continue
                closed_operands = len(old_args - new_args)
                losses = len(new_args - old_args)
            elif before_parts != after_parts:
                # A changed, still-wrong opcode is an unranked tradeoff.
                complete = False
            result["closed_opcodes"] += closed_opcodes
            result["closed_operands"] += closed_operands
            result["losses"] += losses
            if (closed_opcodes or closed_operands or losses) and len(result["sites"]) < _DIAGNOSTIC_SITE_LIMIT:
                result["sites"].append({"target_address": address, "closed_opcodes": closed_opcodes,
                                        "closed_operands": closed_operands, "losses": losses})
        result.update(status="observed", complete=complete,
                      target_stream_sha256=_sha(frontier.canonical([(k, v[1]) for k, v in left.items()])))
    except (ValueError, KeyError, TypeError, IndexError, AttributeError) as exc:
        result.update(reason=str(exc)[:180], complete=False)
    return result


def _structural_closure(before: dict, after: dict, name: str) -> dict:
    """A narrow partial-gain proof, not semantic equivalence or source fidelity.

    Missing operations may close while allocation coordinates become less exact.
    Require complete target-anchored operation/CFG closure, a global nonvolatile
    register permutation, and a byte-bijective permutation of observed stack homes.
    Unknown address-taking, overlapping homes and ABI register changes fail shut.
    """
    result = {"function": name, "qualified": False, "authority_advanced": False}
    try:
        old = causal_groups.summarize_groups(before, name)
        new = causal_groups.summarize_groups(after, name)
        if old["target_binding"] != new["target_binding"]:
            raise ValueError("target instruction stream changed")
        missing = old["category_rows"].get("deleted_instruction", 0) + old["category_rows"].get("deleted_move", 0)
        if not missing or not set(old["category_rows"]) <= {
                "deleted_instruction", "deleted_move", "cfg_changed",
                "register_relation_unknown", "register_permutation"}:
            raise ValueError("baseline is not a missing-operation/coordinate residual")
        if (not new["size_exact"] or new["unresolved_row_count"]
                or not set(new["category_rows"]) <= {"register_permutation"}):
            raise ValueError("candidate has unresolved noncoordinate residuals")
        mapping = new["register_mapping"]
        if mapping["status"] != "confirmed" or any(
                a[0] != b[0] or int(a[1:]) < 14 or int(b[1:]) < 14
                for a, b in mapping["mapping"].items()):
            raise ValueError("register relation is not a nonvolatile permutation")
        old_target, old_candidate = _diagnostic_rows(before, name)
        target, candidate = _diagnostic_rows(after, name)
        if (len(target) != len(candidate) or len(old_target) != len(old_candidate)
                or any(not r.get("instruction") for r in old_target)
                or sum(not r.get("instruction") for r in old_candidate) != missing):
            raise ValueError("unanchored or ambiguous instruction gaps")
        byte_map, inverse = {}, {}
        pointer_offsets = set()
        for rows in (target, candidate):
            addresses = [_prediction_address(r["instruction"]["address"], "instruction address") for r in rows]
            if any(r["instruction"].get("size") != 4 for r in rows) or addresses != list(range(addresses[0], addresses[0] + 4 * len(rows), 4)):
                raise ValueError("candidate/target instructions are not contiguous PPC words")
        for index, pair in enumerate(zip(target, candidate)):
            texts = [r["instruction"]["formatted"] for r in pair]
            if not any("r1" in frontier._register_tokens(text) for text in texts):
                continue
            accesses = [frontier._stack_instruction(text, index) for text in texts]
            if any(a is None for a in accesses):
                raise ValueError("unresolved stack address use")
            a, b = accesses
            if a["kind"] != b["kind"] or a["opcode"] != b["opcode"]:
                raise ValueError("stack operation changed")
            if a["kind"] == "addi_pointer":
                if texts[0] != texts[1]:
                    raise ValueError("stack address-taking changed")
                pointer_offsets.add(a["offset"])
                continue
            width = frontier._STACK_ACCESS_WIDTHS.get(a["opcode"])
            if width is None:
                raise ValueError("unresolved stack access width")
            if a["opcode"].startswith("psq_"):
                parts = frontier._instruction_parts(pair[0])
                if texts[0] != texts[1] or parts[1][-2:] != ["0", "qr0"]:
                    raise ValueError("unresolved paired stack access width")
            if a["opcode"].endswith("u") and texts[0] != texts[1]:
                raise ValueError("stack update changed")
            for delta in range(width):
                x, y = a["offset"] + delta, b["offset"] + delta
                if byte_map.setdefault(x, y) != y or inverse.setdefault(y, x) != x:
                    raise ValueError("stack byte relation is not bijective")
        if byte_map.keys() != inverse.keys():
            raise ValueError("observed stack byte census changed")
        if any(x != y and (x >= offset or y >= offset) for x, y in byte_map.items()
               for offset in pointer_offsets if offset >= 0):
            raise ValueError("address-taken stack region moved")
        result.update(qualified=True, closed_missing_instructions=missing,
                      target_binding=new["target_binding"], stack_bytes_checked=len(byte_map),
                      register_mapping=mapping["mapping"])
    except (ValueError, KeyError, TypeError, IndexError, AttributeError) as exc:
        result["reason"] = str(exc)[:180]
    return result


def _qualified_relocation_alias(document: dict, name: str, physical: dict,
                                strict: dict, data: dict) -> dict | None:
    """Waive only label/addend spelling rows, never change strict measurements."""
    if (physical.get("raw_exact_target") is not True
            or physical.get("candidate_normalized_exact") is not True
            or physical.get("candidate_physical_exact") is not True
            or physical.get("normalized_diff_after") != 0
            or physical.get("physical_diff_after") != 0
            or data.get("instruction_exact") is not True or data.get("diff_rows") != 0
            or strict.get("instruction_exact") is not False or not strict.get("diff_rows")
            or not strict["candidate_bytes"] == strict["target_bytes"] == data["candidate_bytes"] == data["target_bytes"]):
        return None
    try:
        target, candidate = _diagnostic_rows(document, name)
        symbols = [frontier.focus._symbols(document, side, "strict") for side in ("left", "right")]
        if len(target) != len(candidate) or 4 * len(target) != strict["target_bytes"]:
            return None
        aliases = []
        for index, pair in enumerate(zip(target, candidate)):
            if any(not r.get("instruction") or r["instruction"].get("size") != 4 for r in pair):
                return None
            if all(r.get("diff_kind") in (None, "DIFF_NONE") for r in pair):
                continue
            if any(r.get("diff_kind") != "DIFF_ARG_MISMATCH" for r in pair):
                return None
            parts = [r["instruction"].get("parts") for r in pair]
            if not parts[0] or parts[0] != parts[1]:
                return None
            args = [p["arg"] for p in parts[0] if "arg" in p]
            relocation_args = {i for i, arg in enumerate(args) if arg == {"reloc": True}}
            if not relocation_args:
                return None
            for r in pair:
                flags = r.get("arg_diff")
                if not isinstance(flags, list) or len(flags) != len(args) or not all(isinstance(f, dict) for f in flags):
                    return None
                changed = {i for i, f in enumerate(flags) if "diff_index" in f}
                if not changed or not changed <= relocation_args:
                    return None
            relocs = [frontier.relocation_key(r, table) for r, table in zip(pair, symbols)]
            if not all(relocs) or relocs[0]["type"] != relocs[1]["type"] or relocs[0] == relocs[1]:
                return None
            aliases.append(index)
        if len(aliases) != strict["diff_rows"]:
            return None
        return {"function": name, "rows": aliases, "proof": "raw+canonical+physical+data exact; strict relocation operands only",
                "strict_measurements_unchanged": True, "authority_advanced": False}
    except (ValueError, KeyError, TypeError, IndexError, AttributeError):
        return None


def _classify(before_strict: list[dict], before_data: list[dict],
              after_strict: list[dict], after_data: list[dict],
              object_comparison: dict, functions: list[str], *,
              baseline_documents: dict[str, dict] | None = None,
              after_documents: dict[str, dict] | None = None) -> dict:
    """Conservative measurement gate, independent of percentage-only ranking."""
    regressions, gains, changes, positional_shifts = [], [], [], []
    focus = set(functions)
    quality = []
    structural_closures = []
    # Only waive aggregate row growth after all independent structural channels
    # close. Never waive score, sibling, data, relocation or authenticity gates.
    if baseline_documents and after_documents:
        for name in functions:
            row = object_comparison.get("functions", {}).get(name, {})
            target, base, candidate = [frontier.focus._integer(row.get(k)) for k in
                                       ("target_size", "base_size", "candidate_size")]
            if (None in (target, base, candidate) or not 0 <= base < candidate == target
                    or object_comparison.get("function_census_equal") is not True
                    or object_comparison.get("allocated_nontext_changed") is not False
                    or object_comparison.get("allocated_nontext_relocations_changed") is not False
                    or row.get("candidate_normalized_exact") is not True
                    or row.get("candidate_physical_exact") is not True
                    or row.get("ordered_normalized_relocations_equal") is not True
                    or row.get("normalized_diff_after") != 0 or row.get("physical_diff_after") != 0
                    or not isinstance(row.get("normalized_diff_before"), int) or row["normalized_diff_before"] <= 0
                    or row.get("closed_normalized_row_loss_count") != 0
                    or row.get("closed_normalized_row_losses") != []
                    or row.get("closed_physical_row_loss_count") != 0
                    or row.get("closed_physical_row_losses") != []
                    or any(other.get("raw_equal_base") is not True for sibling, other in
                           object_comparison.get("functions", {}).items() if sibling != name)):
                continue
            metrics = [(_metric_map(before).get(name), _metric_map(after).get(name))
                       for before, after in ((before_strict, after_strict), (before_data, after_data))]
            if any(not a or not b or a["instruction_exact"]
                   or (a["target_bytes"], a["candidate_bytes"], b["target_bytes"], b["candidate_bytes"])
                   != (target, base, target, candidate) for a, b in metrics):
                continue
            findings = [_structural_closure(baseline_documents.get(channel, {}),
                                            after_documents.get(channel, {}), name)
                        for channel in ("strict", "data")]
            for channel, finding in zip(("strict", "data"), findings):
                finding["channel"] = channel
            if all(f["qualified"] and 4 * f["closed_missing_instructions"] == candidate - base for f in findings):
                structural_closures.append({"function": name, "channels": findings})
                gains.append(f"structural:{name}: size, missing operations, CFG and physical relocations closed")
    structural_names = {row["function"] for row in structural_closures}
    relocation_aliases = []
    strict_map, data_map = _metric_map(after_strict), _metric_map(after_data)
    if after_documents:
        for name in functions:
            if name in strict_map and name in data_map:
                alias = _qualified_relocation_alias(after_documents.get("strict", {}), name,
                    object_comparison.get("functions", {}).get(name, {}), strict_map[name], data_map[name])
                if alias:
                    relocation_aliases.append(alias)
    alias_names = {row["function"] for row in relocation_aliases}
    for channel, before, after in (("strict", before_strict, after_strict),
                                    ("data", before_data, after_data)):
        left, right = _metric_map(before), _metric_map(after)
        if left.keys() != right.keys():
            regressions.append(f"{channel}: function census changed")
        for name in sorted(left.keys() & right.keys()):
            a, b = left[name], right[name]
            fields = ("candidate_bytes", "diff_rows", "instruction_exact", "match_percent")
            if any(a[k] != b[k] for k in fields):
                changes.append({"channel": channel, "function": name,
                                "before": {k: a[k] for k in fields},
                                "after": {k: b[k] for k in fields}})
            alias_only = channel == "strict" and name in alias_names
            if a["instruction_exact"] and not b["instruction_exact"] and not alias_only:
                regressions.append(f"{channel}:{name}: exact function lost")
            if a["candidate_bytes"] == a["target_bytes"] and b["candidate_bytes"] != b["target_bytes"]:
                regressions.append(f"{channel}:{name}: exact size lost")
            if b["diff_rows"] > a["diff_rows"] and name not in structural_names and not alias_only:
                regressions.append(f"{channel}:{name}: differing rows increased")
            if a.get("match_percent") is not None and b.get("match_percent") is not None:
                if b["match_percent"] < a["match_percent"] and not alias_only:
                    regressions.append(f"{channel}:{name}: score regressed")
            if name in focus and b["diff_rows"] < a["diff_rows"]:
                gains.append(f"{channel}:{name}: {a['diff_rows']} -> {b['diff_rows']} differing rows")
            if baseline_documents and after_documents and name in focus:
                finding = _code_quality(baseline_documents.get(channel, {}), after_documents.get(channel, {}), name)
                finding["channel"] = channel
                quality.append(finding)
    for name in functions:
        findings = [row for row in quality if row["function"] == name]
        if len(findings) != 2 or not all(row.get("complete") and not row["losses"] for row in findings):
            continue
        for row in findings:
            channel = row["channel"]
            before, after = ((before_strict, after_strict) if channel == "strict" else (before_data, after_data))
            a, b = _metric_map(before)[name], _metric_map(after)[name]
            if (row["closed_opcodes"] or row["closed_operands"]) and b["diff_rows"] == a["diff_rows"] \
                    and a.get("match_percent") is not None and b.get("match_percent") is not None \
                    and b["match_percent"] > a["match_percent"]:
                gains.append(f"{channel}:{name}: closed {row['closed_opcodes']} opcode and "
                             f"{row['closed_operands']} operand mismatches at stable target anchors")
    if not object_comparison.get("function_census_equal", False):
        regressions.append("object function census changed")
    for name, row in object_comparison.get("functions", {}).items():
        # Relocation placement can move when a .text function is slid.  The
        # inventory retains both views: raw physical rows for diagnostics and
        # normalized rows keyed by canonical callee identity.  Use the latter
        # for monotonic gates whenever present, retaining the raw fallback for
        # older inventories that predate normalized relocation fields.
        normalized_before = frontier.focus._integer(row.get("normalized_diff_before"))
        normalized_after = frontier.focus._integer(row.get("normalized_diff_after"))
        normalized_losses = row.get("closed_normalized_row_losses")
        normalized_loss_count = frontier.focus._integer(row.get("closed_normalized_row_loss_count"))
        use_normalized = (
            normalized_before is not None and normalized_before >= 0
            and normalized_after is not None and normalized_after >= 0
            and isinstance(normalized_losses, list)
            and normalized_loss_count is not None and normalized_loss_count >= 0
        )
        if use_normalized:
            qualified_shift = (not regressions and _qualified_positional_shift(
                name, row, object_comparison,
                [(before_strict, after_strict), (before_data, after_data)], focus))
            if normalized_loss_count or normalized_losses:
                if qualified_shift:
                    positional_shifts.append(name)
                else:
                    regressions.append(f"relocation:{name}: canonical relocation rows lost")
            if normalized_after > normalized_before and not qualified_shift:
                regressions.append(f"relocation:{name}: canonical differences increased")
            if name in focus and normalized_after < normalized_before:
                gains.append(f"relocation:{name}: {normalized_before} -> {normalized_after}")
        else:
            if row.get("closed_physical_row_losses"):
                regressions.append(f"physical:{name}: previously exact relocation rows lost")
            physical_before = frontier.focus._integer(row.get("physical_diff_before"))
            physical_after = frontier.focus._integer(row.get("physical_diff_after"))
            physical_before = 0 if physical_before is None else physical_before
            physical_after = 0 if physical_after is None else physical_after
            if physical_after > physical_before:
                regressions.append(f"physical:{name}: differences increased")
            if name in focus and physical_after < physical_before:
                gains.append(f"physical:{name}: {physical_before} -> {physical_after}")
    # Nontext movement can be a legitimate pool repair. Do not call it a safe
    # retained gain without its separate typed-value/consumer proof.
    review = ["allocated nontext changed; typed data/consumer review required"] if object_comparison.get("allocated_nontext_changed") else []
    strict, data = _metric_map(after_strict), _metric_map(after_data)
    exact = all(name in strict and name in data and strict[name]["instruction_exact"]
                and data[name]["instruction_exact"]
                and object_comparison.get("functions", {}).get(name, {}).get("raw_exact_target")
                and object_comparison.get("functions", {}).get(name, {}).get("candidate_physical_exact")
                for name in functions)
    status = "rejected" if regressions else "exact" if exact else "improved" if gains else "no_gain"
    return {"status": status, "exact_scope": "selected_functions_only", "owner_exact": False,
            "gains": gains, "regressions": sorted(set(regressions)),
            "review_required": review, "metric_changes": changes, "code_quality": quality,
            "qualified_positional_relocation_shifts": positional_shifts,
            "qualified_relocation_aliases": relocation_aliases,
            "qualified_structural_closures": structural_closures,
            "retention_ready": status in {"exact", "improved"} and not review}


def _command_context(root: Path, command_json: Path, tools: list[Path]) -> dict:
    command = compiler.load_command_json(command_json)
    if command.count("{source}") != 1 or command.count("{object}") != 1:
        raise ValueError("compiler argv requires exactly one {source} and one {object} argument")
    if any(("{source}" in item and item != "{source}") or
           ("{object}" in item and item != "{object}") for item in command):
        raise ValueError("compiler placeholders must be complete argv cells")
    command = compiler._resolved_command(command, root)
    paths = {Path(command[0]), *(Path(os.path.abspath(root / path)) for path in tools)}
    if not tools:
        raise ValueError("name the compiler executable with --compiler-tool (also when using a wrapper)")
    return {"argv_template": command, "command_json": _descriptor(command_json),
            "tools": {str(path): compiler.digest(path) for path in sorted(paths)},
            "headers": compiler.tree(root / "include"),
            "generated_headers": compiler.tree(root / "build/GP6E01/include"),
            "environment_sha256": _sha(frontier.canonical(dict(os.environ)))}


def _same_baseline_context(root: Path, base: dict, context: dict) -> bool:
    descriptor = base.get("inputs", {}).get("compile_receipt")
    if base.get("compile_binding") != "receipt_hashes_match" or not descriptor:
        return False
    raw, actual = frontier.read_bound(root, Path(descriptor["path"]), frontier.INDEX_LIMIT)
    if actual != descriptor:
        raise ValueError("baseline compiler receipt drifted")
    return frontier.load_json(raw).get("context_sha256") == _sha(frontier.canonical(context))


def _compile_candidate(root: Path, candidate: Path, output: Path, command_json: Path,
                       compiler_tools: list[Path], timeout: float) -> dict:
    context = _command_context(root, command_json, compiler_tools)
    source = _descriptor(candidate)
    argv = [str(candidate) if item == "{source}" else str(output) if item == "{object}" else item
            for item in context["argv_template"]]
    # A fresh invocation-owned path is never allowed to reuse a stale object.
    if output.exists():
        raise ValueError(f"compiler object already exists: {output}")
    started = time.monotonic()
    with owner_campaign._exclusive_lock(root / "build/.compiler-lane.lock", timeout=timeout):
        if context != _command_context(root, command_json, compiler_tools):
            raise ValueError("compiler context drifted before launch")
        remaining = timeout - (time.monotonic() - started)
        result = bounded_process.run(argv, cwd=root, timeout=remaining, max_output=256 * 1024)
    compiler.atomic(output.with_suffix(".stdout.log"), result.stdout)
    compiler.atomic(output.with_suffix(".stderr.log"), result.stderr)
    if result.returncode:
        raise ValueError(f"compiler failed ({result.returncode}): {compiler._diagnostics(result.stdout, result.stderr)}")
    if _descriptor(candidate) != source or context != _command_context(root, command_json, compiler_tools):
        raise ValueError("source or compiler context changed during compilation")
    if not output.is_file() or not 0 < output.stat().st_size <= 16 * 1024 * 1024:
        raise ValueError("compiler did not produce a bounded nonempty object")
    return {"schema": "recovery_candidate_compile/v1", "source_sha256": source["sha256"],
            "object_sha256": compiler.digest(output), "context_sha256": _sha(frontier.canonical(context)),
            "command": argv, "context": context,
            "seconds": time.monotonic() - started, "stdout_sha256": _sha(result.stdout),
            "stderr_sha256": _sha(result.stderr)}


def _report(objdiff: Path, target: Path, candidate: Path, output: Path,
            root: Path, data: bool, timeout: float) -> None:
    args = [str(objdiff), "diff", "-1", str(target), "-2", str(candidate), "-o", str(output), "--format", "json"]
    if data:
        args += ["-c", "functionRelocDiffs=data_value"]
    result = bounded_process.run(args, cwd=root, timeout=timeout, max_output=256 * 1024)
    if result.returncode:
        raise ValueError(f"objdiff failed ({result.returncode}): {compiler._diagnostics(result.stdout, result.stderr)}")
    if not output.is_file() or output.stat().st_size > frontier.REPORT_LIMIT:
        raise ValueError("objdiff report missing or exceeds bounded limit")


def _validate_report_objects(document: dict, target: dict, candidate: dict) -> None:
    """Reject omitted/extra functions and metadata not matching actual objects."""
    for side, obj in (("left", target), ("right", candidate)):
        rows = frontier.focus._symbols(document, side, "candidate report")
        actual = {r["name"]: int(r["size"]) for r in rows if frontier.focus._is_function(r)}
        expected = {name: row["size"] for name, row in obj["functions"].items()}
        if actual != expected:
            raise ValueError(f"{side} report function census/size differs from actual object")
        for row in rows:
            if not frontier.focus._is_function(row):
                continue
            instructions = frontier.focus._rows(row, row["name"])
            if frontier.focus._instruction_count(instructions) * 4 != expected[row["name"]]:
                raise ValueError(f"{side} report instruction coverage differs from object: {row['name']}")


def _focus_evidence(document: dict, functions: list[str]) -> dict:
    result = {}
    sides = {side: frontier.focus._symbols(document, side, "candidate") for side in ("left", "right")}
    for name in functions:
        left = frontier.focus._rows(frontier._stack_function(sides["left"], name, "target"), name)
        right = frontier.focus._rows(frontier._stack_function(sides["right"], name, "candidate"), name)
        summary = frontier._diagnose_rows(left, right)
        differing = []
        for i in range(max(len(left), len(right))):
            a, b = left[i] if i < len(left) else {}, right[i] if i < len(right) else {}
            if frontier._diagnose_kind(a) or frontier._diagnose_kind(b):
                differing.append({"row": i, "target": frontier.instruction(a),
                                  "candidate": frontier.instruction(b),
                                  "kind": a.get("diff_kind") or b.get("diff_kind")})
        summary.update(residuals=differing[:24], residuals_total=len(differing),
                       residuals_truncated=len(differing) > 24)
        result[name] = summary
    return result


def _diagnostic_rows(document: dict, name: str) -> tuple[list[dict], list[dict]] | None:
    sides = {side: frontier.focus._symbols(document, side, "changed-result diagnostic")
             for side in ("left", "right")}
    target = frontier._stack_function(sides["left"], name, "target")
    candidate = frontier._stack_function(sides["right"], name, "candidate")
    if target is None or candidate is None:
        return None
    return (frontier.focus._rows(target, name), frontier.focus._rows(candidate, name))


def _diagnostic_mismatches(target: list[dict], candidate: list[dict]) -> dict[int, tuple[Any, Any]]:
    result: dict[int, tuple[Any, Any]] = {}
    for index in range(max(len(target), len(candidate))):
        target_payload = frontier._diagnose_payload(target[index] if index < len(target) else None, index)
        candidate_payload = frontier._diagnose_payload(candidate[index] if index < len(candidate) else None, index)
        if target_payload != candidate_payload:
            result[index] = (target_payload, candidate_payload)
    return result


def _diagnostic_context(target: list[dict], candidate: list[dict], center: int) -> list[dict]:
    result = []
    for index in (center - 1, center + 1):
        if 0 <= index < max(len(target), len(candidate)):
            result.append({"row": index,
                           "target": frontier._diagnose_row(target[index] if index < len(target) else None, index),
                           "candidate": frontier._diagnose_row(candidate[index] if index < len(candidate) else None, index)})
    return result[:_DIAGNOSTIC_CONTEXT_LIMIT]


def _compact_first_mismatch(summary: dict | None) -> dict | None:
    first = summary.get("first_instruction_mismatch") if isinstance(summary, dict) else None
    if not isinstance(first, dict):
        return None
    return {key: first[key] for key in ("row", "kind", "target", "candidate") if key in first}


def _compact_canonical_mismatch(summary: dict | None) -> dict | None:
    first = summary.get("first_mismatch") if isinstance(summary, dict) else None
    if not isinstance(first, dict):
        return None
    return {key: first[key] for key in ("row", "kind", "target", "candidate") if key in first}


def _compact_site_instruction(row: dict | None, index: int) -> tuple[dict[str, Any], str, bool] | None:
    """Return one bounded instruction site and its exact report opcode."""
    compact = frontier._diagnose_row(row, index)
    instruction = compact.get("instruction")
    if not isinstance(instruction, dict):
        return None
    formatted = instruction.get("formatted")
    if not isinstance(formatted, str):
        return None
    opcode_match = frontier._OPCODE_RE.match(formatted)
    if opcode_match is None:
        return None
    opcode = opcode_match.group("opcode").lower()
    operands = formatted[opcode_match.end():]
    stack_base = bool(_STACK_BASE_REGISTER_RE.search(operands)) if opcode.endswith("x") else any(
        match.group("base").lower() == "r1"
        for match in frontier._MEMORY_OPERAND_RE.finditer(formatted)
    )
    return ({"row": compact["row"], "address": instruction.get("address"),
             "formatted": formatted}, opcode, stack_base)


def _diagnostic_site_census(rows: list[dict], mnemonics: frozenset[str],
                            neighbor_mnemonics: frozenset[str],
                            *, prioritize_non_stack_base: bool = False) -> dict[str, Any]:
    """Collect a bounded exact-opcode site census from one report stream."""
    parsed = [_compact_site_instruction(row, index) for index, row in enumerate(rows)]
    matches = [(index, item) for index, item in enumerate(parsed)
               if item is not None and item[1] in mnemonics]
    stack_base_total = sum(item[2] for _, item in matches)
    if prioritize_non_stack_base:
        selected = [entry for entry in matches if not entry[1][2]][:_DIAGNOSTIC_SITE_LIMIT]
        selected.extend(entry for entry in matches if entry[1][2]
                        and len(selected) < _DIAGNOSTIC_SITE_LIMIT)
        selected.sort(key=lambda entry: entry[0])
        selection_policy = "non_stack_base_first_then_chronological"
    else:
        selected = matches[:_DIAGNOSTIC_SITE_LIMIT]
        selection_policy = "chronological"
    sites = []
    for index, current in selected:
        if current is None:  # pragma: no cover - selected from ``matches`` above
            continue
        site = dict(current[0])
        site["stack_base"] = current[2]
        neighbors = {}
        for side, delta in (("before", -1), ("after", 1)):
            adjacent = index + delta
            if not 0 <= adjacent < len(parsed):
                continue
            neighbor = parsed[adjacent]
            if neighbor is not None and neighbor[1] in neighbor_mnemonics:
                neighbors[side] = dict(neighbor[0])
        if neighbors:
            site["neighbors"] = neighbors
        sites.append(site)
    return {
        "diagnostic_only": True,
        "selection_policy": selection_policy,
        "sites": sites,
        "total": len(matches),
        "non_stack_base_total": len(matches) - stack_base_total,
        "stack_base_total": stack_base_total,
        "truncated": len(matches) > _DIAGNOSTIC_SITE_LIMIT,
    }


def _diagnostic_site_streams(target_rows: list[dict], candidate_rows: list[dict],
                             mnemonics: frozenset[str],
                             neighbor_mnemonics: frozenset[str],
                             *, prioritize_non_stack_base: bool = False) -> dict[str, Any]:
    return {
        "target": _diagnostic_site_census(
            target_rows, mnemonics, neighbor_mnemonics,
            prioritize_non_stack_base=prioritize_non_stack_base,
        ),
        "candidate": _diagnostic_site_census(
            candidate_rows, mnemonics, neighbor_mnemonics,
            prioritize_non_stack_base=prioritize_non_stack_base,
        ),
    }


def _paired_memory_site_census(rows: list[dict]) -> dict[str, Any]:
    return _diagnostic_site_census(
        rows, _PAIRED_MEMORY_MNEMONICS, _SCALAR_MEMORY_MNEMONICS,
        prioritize_non_stack_base=True,
    )


def _scalar_abs_conversion_site_census(rows: list[dict]) -> dict[str, Any]:
    return _diagnostic_site_census(
        rows, _SCALAR_ABS_CONVERSION_MNEMONICS,
        _SCALAR_MEMORY_MNEMONICS | _SCALAR_UNARY_MNEMONICS,
    )


def _target_anchor_context(before_target: list[dict], after_target: list[dict],
                           after_candidate: list[dict], first: dict | None) -> dict:
    """Keep the original problem site visible through inserted prologue rows.

    An anchor is report context, not a causal or retention verdict. Require the
    whole non-gap target stream to agree, not merely a coincident address.
    """
    result: dict[str, Any] = {"diagnostic_only": True, "status": "unknown"}
    if first is None:
        return {**result, "status": "none", "reason": "no baseline instruction mismatch"}
    try:
        def stream(rows: list[dict]) -> tuple[list[tuple[int, dict]], dict[int, int]]:
            identities, positions = [], {}
            for index, row in enumerate(rows):
                payload = frontier._diagnose_payload(row, index)
                if payload is None:
                    continue
                value = row["instruction"].get("address")
                if isinstance(value, bool) or not isinstance(value, (str, int)):
                    raise ValueError("target instruction address missing or invalid")
                if isinstance(value, str):
                    if len(value) > 32:
                        raise ValueError("target instruction address too long")
                    address = int(value, 16 if value.lower().startswith("0x") else 10)
                else:
                    address = value
                if address < 0 or address >= 2 ** 64:
                    raise ValueError("target instruction address out of range")
                if address in positions:
                    raise ValueError("ambiguous target instruction address")
                positions[address] = index
                identities.append((address, payload))
            return identities, positions

        before, before_positions = stream(before_target)
        after, after_positions = stream(after_target)
        if before != after:
            raise ValueError("non-gap target stream changed")
        baseline_row = first.get("row")
        matches = [address for address, index in before_positions.items() if index == baseline_row]
        if len(matches) != 1:
            raise ValueError("baseline mismatch has no unique target instruction")
        address = matches[0]
        row = after_positions[address]
        result.update(status="located", target_address=address, baseline_row=baseline_row,
                      current_row=row, target_stream_sha256=_sha(frontier.canonical(before)),
                      target=frontier._diagnose_row(after_target[row], row),
                      candidate=frontier._diagnose_row(
                          after_candidate[row] if row < len(after_candidate) else None, row),
                      context=_diagnostic_context(after_target, after_candidate, row))
    except (ValueError, KeyError, TypeError, IndexError, AttributeError) as exc:
        result.update(reason=str(exc)[:180])
    return result


def _changed_result_diagnostics(*, root: Path, baseline_documents: dict[str, dict],
                                after_documents: dict[str, dict], strict_path: Path,
                                data_path: Path, metric_changes: list[dict],
                                object_comparison: dict, reports_retained: bool = False) -> dict:
    changes_by_function: dict[str, dict[str, dict]] = {}
    for change in metric_changes:
        if isinstance(change, dict) and isinstance(change.get("function"), str):
            changes_by_function.setdefault(change["function"], {})[change.get("channel", "unknown")] = change
    object_rows = object_comparison.get("functions", {}) if isinstance(object_comparison, dict) else {}
    if not isinstance(object_rows, dict):
        object_rows = {}
    for name, row in object_rows.items():
        if isinstance(row, dict) and row.get("raw_equal_base") is False:
            changes_by_function.setdefault(name, {})
    raw_names = [name for name, row in object_rows.items()
                 if isinstance(row, dict) and row.get("raw_equal_base") is False]
    names = raw_names + [name for name in changes_by_function if name not in raw_names]
    selected = names[:_DIAGNOSTIC_FUNCTION_LIMIT]
    result: dict[str, Any] = {"diagnostic_only": True, "status": "none" if not names else "bounded",
                              "affected_function_count": len(names), "truncated": len(names) > len(selected),
                              "functions": []}
    for name in selected:
        object_row = object_rows.get(name, {}) if isinstance(object_rows, dict) else {}
        if not isinstance(object_row, dict):
            object_row = {}
        if "raw_equal_base" not in object_row:
            object_relation = "unknown"
        elif not object_row["raw_equal_base"]:
            object_relation = "raw_changed"
        else:
            base_relocations = object_row.get("base_relocations")
            candidate_relocations = object_row.get("candidate_relocations")
            if (isinstance(base_relocations, dict) and isinstance(candidate_relocations, dict)
                    and isinstance(base_relocations.get("sha256"), str)
                    and isinstance(candidate_relocations.get("sha256"), str)):
                object_relation = ("relocation_only" if base_relocations["sha256"] != candidate_relocations["sha256"]
                                   else "unchanged")
            else:
                object_relation = "unknown"
        item: dict[str, Any] = {"function": name, "object_relation": object_relation,
                                "metric_changes": {channel: {"before": change.get("before"), "after": change.get("after")}
                                                    for channel, change in changes_by_function[name].items()},
                                "diagnose_argv": (["python", "tools/recovery_frontier.py", "--root", str(root),
                                                   "diagnose", "--strict", str(strict_path), "--data", str(data_path),
                                                   "--function", name] if reports_retained else None),
                                "diagnose_argv_available": reports_retained, "channels": {}}
        for channel, path in (("strict", strict_path), ("data", data_path)):
            after = after_documents.get(channel)
            before = baseline_documents.get(channel)
            channel_item: dict[str, Any] = {}
            try:
                after_rows = _diagnostic_rows(after, name) if after is not None else None
                before_rows = _diagnostic_rows(before, name) if before is not None else None
                if after_rows is None or before_rows is None:
                    raise ValueError("report function missing")
                before_target, before_candidate = before_rows
                after_target, after_candidate = after_rows
                after_summary = frontier._diagnose_rows(after_target, after_candidate)
                before_summary = frontier._diagnose_rows(before_target, before_candidate)
                channel_item["current_first_instruction_mismatch"] = _compact_first_mismatch(after_summary)
                existing_instruction = _compact_first_mismatch(before_summary)
                channel_item["existing_first_instruction_mismatch"] = existing_instruction
                anchor = existing_instruction
                anchor_basis = "instruction"
                if anchor is None:
                    anchor = _compact_canonical_mismatch(before_summary)
                    anchor_basis = "annotation"
                channel_item["baseline_target_context"] = _target_anchor_context(
                    before_target, after_target, after_candidate, anchor)
                if anchor is not None:
                    channel_item["baseline_target_context"]["anchor_basis"] = anchor_basis
                channel_item["paired_memory_sites"] = _diagnostic_site_streams(
                    after_target, after_candidate, _PAIRED_MEMORY_MNEMONICS, _SCALAR_MEMORY_MNEMONICS,
                    prioritize_non_stack_base=True)
                channel_item["scalar_abs_conversion_sites"] = _diagnostic_site_streams(
                    after_target, after_candidate, _SCALAR_ABS_CONVERSION_MNEMONICS,
                    _SCALAR_MEMORY_MNEMONICS | _SCALAR_UNARY_MNEMONICS)
                if len(before_target) != len(after_target) or len(before_candidate) != len(after_candidate):
                    raise ValueError("aligned report row count changed")
                for index in range(len(before_target)):
                    if frontier._diagnose_payload(before_target[index], index) != frontier._diagnose_payload(after_target[index], index):
                        raise ValueError("target code stream changed")
                old = _diagnostic_mismatches(before_target, before_candidate)
                new = _diagnostic_mismatches(after_target, after_candidate)
                introduced = sorted(set(new) - set(old))
                changed_existing = sorted(index for index in set(new) & set(old) if new[index] != old[index])
                if introduced:
                    row = introduced[0]
                    channel_item.update(status="new_code_difference", row=row,
                                        mismatch={"target": frontier._diagnose_row(after_target[row], row),
                                                  "candidate": frontier._diagnose_row(after_candidate[row], row)},
                                        context=_diagnostic_context(after_target, after_candidate, row))
                elif changed_existing:
                    channel_item.update(status="unknown", reason="existing row changed; not_newly_proven")
                elif new:
                    channel_item.update(status="existing", reason="canonical code mismatch predates candidate",
                                        row=min(new))
                else:
                    channel_item.update(status="none")
            except (OSError, ValueError, RuntimeError, KeyError, TypeError, IndexError, AttributeError) as exc:
                channel_item.update(status="unknown", reason=f"{str(exc)[:180]}; not_newly_proven")
            item["channels"][channel] = channel_item
        result["functions"].append(item)
    result["unknown_count"] = sum(
        1 for item in result["functions"] for channel in item["channels"].values()
        if channel.get("status") == "unknown"
    )
    return result


def _causal_group_diagnostics(before: dict[str, dict], after: dict[str, dict],
                              functions: list[str]) -> dict:
    """Keep actionable machine relationships after disposable reports are removed.

    These are observed relation buckets, not a source-causality proof and not a
    new retention gate. Malformed diagnostic data cannot mask the primary proof.
    """
    result = {"schema": "recovery_evaluation_causal_groups/v1",
              "authority_advanced": False, "channels": {},
              "omitted_functions": functions[_DIAGNOSTIC_FUNCTION_LIMIT:]}
    for channel in ("strict", "data"):
        rows = {}
        result["channels"][channel] = rows
        for function in functions[:_DIAGNOSTIC_FUNCTION_LIMIT]:
            try:
                current = causal_groups.summarize_groups(after[channel], function)
                previous = causal_groups.summarize_groups(before[channel], function)
                change = causal_groups.compare_groups(previous, current)
                # Full maps stay in the dedicated diagnostic tool. Per-evaluation
                # memory is bounded independently of whole-TU report size.
                current["group_count"] = len(current["groups"])
                current["groups_omitted"] = max(0, len(current["groups"]) - 64)
                current["groups"] = current["groups"][:64]
                for group in current["groups"]:
                    members = group.get("members", [])
                    group["members_omitted"] = max(0, len(members) - 8)
                    group["members"] = members[:8]
                current["register_mapping"].pop("body_projection", None)
                conflicts = current["register_mapping"].get("conflicts", [])
                current["register_mapping"]["conflicts_omitted"] = max(0, len(conflicts) - 16)
                current["register_mapping"]["conflicts"] = conflicts[:16]
                for field in ("closed_groups", "new_groups", "changed_groups", "reclassified_groups", "disappeared_observation_buckets"):
                    change[field + "_count"] = len(change[field])
                    change[field + "_omitted"] = max(0, len(change[field]) - 64)
                    change[field] = change[field][:64]
                payload = {"status": "observed", "after": current, "change": change}
                if len(frontier.canonical(payload)) > 32 * 1024:
                    payload = {"status": "summary_only", "group_count": current["group_count"],
                               "coverage": current["coverage"],
                               "category_rows": current["category_rows"],
                               "ranking_tuple": current["ranking_tuple"],
                               "structural_hazard_count": current["structural_hazard_count"],
                               "reason": "detailed relation groups exceed compact limit",
                               "closure_status": change["status"],
                               "closed_groups_count": change["closed_groups_count"],
                               "reclassified_groups_count": change["reclassified_groups_count"],
                               "resolved_target_site_count": change["resolved_target_site_count"],
                               "introduced_target_site_count": change["introduced_target_site_count"],
                               "residual_rows_delta": change["residual_rows_delta"]}
                rows[function] = payload
            except (ValueError, KeyError, TypeError, AttributeError, IndexError) as exc:
                rows[function] = {"status": "unknown", "reason": str(exc)[:400]}
    return result


def _physical_progress_summary(object_comparison: dict) -> dict:
    result: dict[str, Any] = {
        "diagnostic_only": True,
        "scope": "raw+normalized-physical only",
        "strict_exact": "not_measured",
        "linked_or_whole_owner_completion": "not_measured",
        "status": "unknown",
        "functions_inspected": None,
        "raw_target_exact_count": None,
        "raw_and_normalized_physical_exact_count": None,
        "normalized_mismatch_rows_before": None,
        "normalized_mismatch_rows_after": None,
        "previously_closed_normalized_row_loss_count": None,
        "raw_changed_function_count": None,
    }
    if not isinstance(object_comparison, dict) or object_comparison.get("function_census_equal") is not True:
        result["reason"] = "function census or comparison fields incomplete"
        return result
    rows = object_comparison.get("functions")
    if not isinstance(rows, dict) or not rows:
        result["reason"] = "function comparison rows missing"
        return result
    result["functions_inspected"] = len(rows)
    required_bool = ("base_raw_exact_target", "raw_exact_target", "base_normalized_exact",
                     "candidate_normalized_exact", "raw_equal_base")
    required_int = ("normalized_diff_before", "normalized_diff_after", "closed_normalized_row_loss_count")
    values = []
    for name, row in rows.items():
        if not isinstance(row, dict):
            result["reason"] = f"incomplete function row: {str(name)[:96]}"
            return result
        if any(not isinstance(row.get(field), bool) for field in required_bool):
            result["reason"] = f"incomplete boolean fields: {str(name)[:96]}"
            return result
        if any(isinstance(row.get(field), bool) or not isinstance(row.get(field), int)
               or row[field] < 0 for field in required_int):
            result["reason"] = f"incomplete numeric fields: {str(name)[:96]}"
            return result
        if (row["base_normalized_exact"] != (row["normalized_diff_before"] == 0)
                or row["candidate_normalized_exact"] != (row["normalized_diff_after"] == 0)):
            result["reason"] = f"contradictory normalized exactness: {str(name)[:96]}"
            return result
        if ((row["raw_equal_base"] and row["base_raw_exact_target"] != row["raw_exact_target"])
                or (row["base_raw_exact_target"] and row["raw_exact_target"] and not row["raw_equal_base"])):
            result["reason"] = f"contradictory raw identity: {str(name)[:96]}"
            return result
        values.append(row)
    result.update(
        status="known",
        raw_target_exact_count={
            "before": sum(row["base_raw_exact_target"] for row in values),
            "after": sum(row["raw_exact_target"] for row in values),
        },
        raw_and_normalized_physical_exact_count={
            "before": sum(row["base_raw_exact_target"] and row["base_normalized_exact"] for row in values),
            "after": sum(row["raw_exact_target"] and row["candidate_normalized_exact"] for row in values),
        },
        normalized_mismatch_rows_before=sum(row["normalized_diff_before"] for row in values),
        normalized_mismatch_rows_after=sum(row["normalized_diff_after"] for row in values),
        previously_closed_normalized_row_loss_count=sum(
            row["closed_normalized_row_loss_count"] for row in values
        ),
        raw_changed_function_count=sum(not row["raw_equal_base"] for row in values),
        definition="raw exact AND canonical normalized relocation exact",
    )
    return result


def _dispatch_summary(result: dict) -> dict:
    summary = {key: result[key] for key in
               ("status", "functions", "compiler_runs", "objdiff_runs", "retention_ready", "retained", "seconds", "cleanup_errors")}
    summary.update(gains=result.get("gains", [])[:8], regressions=result.get("regressions", [])[:8],
                   regression_count=len(result.get("regressions", [])), reason=result.get("reason"),
                   changed_result_diagnostics=result.get("changed_result_diagnostics"),
                   physical_progress=result.get("physical_progress"), code_quality=result.get("code_quality", [])[:6])
    # Process-limit diagnostics otherwise disappear from the CLI even though
    # the durable result kept them. Do not require a second run to learn why
    # no object/report was produced.
    summary.update(stage=result.get("stage"),
                   diagnostics=str(result.get("diagnostics") or "")[-2048:])
    if "prediction_feedback" in result:
        summary["prediction_feedback"] = result["prediction_feedback"]
    encoded = json.dumps(summary, sort_keys=True).encode("utf-8")
    if len(encoded) > _EVALUATE_STDOUT_LIMIT:
        diagnostic = result.get("changed_result_diagnostics") or {}
        summary["changed_result_diagnostics"] = {
            "diagnostic_only": True, "status": diagnostic.get("status", "unknown"),
            "affected_function_count": diagnostic.get("affected_function_count", 0),
            "unknown_count": diagnostic.get("unknown_count", 0), "truncated": True,
        }
    if len(json.dumps(summary, sort_keys=True).encode("utf-8")) > _EVALUATE_STDOUT_LIMIT:
        diagnostic = result.get("changed_result_diagnostics") or {}
        summary = {
            "status": str(result.get("status", "unknown"))[:128],
            "functions": [str(value)[:96] for value in result.get("functions", [])[:8]],
            "function_count": len(result.get("functions", [])),
            "compiler_runs": result.get("compiler_runs", 0), "objdiff_runs": result.get("objdiff_runs", 0),
            "retention_ready": bool(result.get("retention_ready", False)),
            "retained": bool(result.get("retained", False)), "seconds": result.get("seconds"),
            "cleanup_error_count": len(result.get("cleanup_errors", [])),
            "gains_count": len(result.get("gains", [])), "regressions_count": len(result.get("regressions", [])),
            "reason": str(result.get("reason") or "")[:500],
            "stage": str(result.get("stage") or "unknown")[:64],
            "diagnostics": str(result.get("diagnostics") or "")[-2048:],
            "changed_result_diagnostics": {
                "diagnostic_only": True, "status": diagnostic.get("status", "unknown"),
                "affected_function_count": diagnostic.get("affected_function_count", 0),
                "unknown_count": diagnostic.get("unknown_count", 0), "truncated": True,
            },
            "physical_progress": result.get("physical_progress"),
            "prediction_feedback": (
                {"schema": PREDICTION_SCHEMA,
                 "diagnostic_only": True, "status": result.get("prediction_feedback", {}).get("status", "unknown"),
                 "binding_status": result.get("prediction_feedback", {}).get("binding_status", "unknown"),
                 "counts": result.get("prediction_feedback", {}).get("counts", {}), "truncated": True}
                if isinstance(result.get("prediction_feedback"), dict) else None
            ),
            "stdout_truncated": True,
        }
    return summary


def _cleanup(directory: Path, root: Path) -> list[str]:
    """Remove only this invocation's explicit, flat temporary output directory."""
    errors = []
    try:
        frontier.local(root, directory).relative_to(root / "build")
        for path in directory.iterdir():
            frontier.local(root, path)
            if not path.is_file():
                raise ValueError(f"unexpected non-file in private evaluation directory: {path}")
            path.unlink()
        directory.rmdir()
    except (OSError, ValueError) as exc:
        errors.append(str(exc))
    return errors


def working_source_binding(root: Path, index: Path, base: dict, working: dict) -> tuple[bytes, dict[str, str]]:
    """Validate diagnostic reconstruction ancestry; it never replaces the frontier."""
    if (not isinstance(working, dict)
            or working.get("champion_source_sha256") != base["inputs"]["source"]["sha256"]
            or working.get("baseline_index_sha256") != compiler.digest(index)
            or not isinstance(working.get("lineage"), str) or not working["lineage"].strip()
            or len(working["lineage"]) > 2000):
        raise ValueError("working source requires current champion/index binding and bounded lineage")
    evidence = working.get("evidence")
    if not isinstance(evidence, list) or not 1 <= len(evidence) <= 8:
        raise ValueError("working source requires 1..8 hash-bound diagnostic evidence descriptors")
    watched = {}
    raw = None
    for number, desc in enumerate([working, *evidence]):
        if not isinstance(desc, dict) or not isinstance(desc.get("path"), str):
            raise ValueError("working source/evidence descriptor required")
        path = frontier.local(root, Path(desc["path"]))
        data, actual = frontier.read_bound(root, path, 4 * 1024 * 1024 if number == 0 else frontier.REPORT_LIMIT)
        if actual["sha256"] != desc.get("sha256"):
            raise ValueError("working source/evidence binding is stale")
        watched[str(path)] = actual["sha256"]
        if number == 0:
            raw = data
    return raw, watched


def source_lineage(root: Path, index: Path, base: dict, working: dict, candidate: bytes) -> dict:
    """Report both real source deltas, never call a working delta a champion delta."""
    import difflib
    source, _ = working_source_binding(root, index, base, working)
    champion, actual = frontier.read_bound(root, Path(base["inputs"]["source"]["path"]), 4 * 1024 * 1024)
    if actual["sha256"] != base["inputs"]["source"]["sha256"]:
        raise ValueError("protected champion source binding is stale")

    def delta(before: bytes, label: str) -> dict:
        left, right = before.decode("utf-8").splitlines(True), candidate.decode("utf-8").splitlines(True)
        operations = [op for op in difflib.SequenceMatcher(None, left, right, autojunk=False).get_opcodes() if op[0] != "equal"]
        diff = "".join(difflib.unified_diff(left, right, fromfile=label, tofile="candidate")).encode("utf-8")
        # Counts and digest describe the entire delta, not this bounded preview.
        # Ignore only an incomplete UTF-8 character at the preview boundary.
        preview = diff[:16 * 1024].decode("utf-8", errors="ignore")
        return {"source_sha256": _sha(before), "candidate_sha256": _sha(candidate),
                "changed_regions": len(operations), "removed_lines": sum(b-a for _, a, b, _, _ in operations),
                "added_lines": sum(d-c for _, _, _, c, d in operations),
                "diff": preview, "diff_sha256": _sha(diff), "diff_bytes": len(diff),
                "diff_truncated": len(diff) > 16 * 1024}

    return {"schema": "recovery_source_lineage/v1", "working_source": working,
            "protected_champion": base["inputs"]["source"], "baseline_index_sha256": compiler.digest(index),
            "working_to_candidate": delta(source, "canonical-working-source"),
            "champion_to_candidate": delta(champion, "protected-champion"),
            "diagnostic_only": True, "authority_advanced": False}


def evaluate(*, root: Path, index: Path, candidate: Path, functions: list[str], out: Path,
             objdiff: Path, readelf: Path, command_json: Path | None = None,
             compiler_tools: list[Path] | None = None, candidate_object: Path | None = None,
             timeout: float = 120, keep_reports: bool = False,
             prediction: Path | None = None, working_source: dict | None = None) -> dict:
    """Measure a candidate without modifying source, baseline, queue or permits."""
    started = time.monotonic()
    root = Path(os.path.abspath(root))
    index, candidate, out = [frontier.local(root, p) for p in (index, candidate, out)]
    prediction = frontier.local(root, prediction) if prediction is not None else None
    out.relative_to(root / "build")
    if not functions or len(set(functions)) != len(functions):
        raise ValueError("distinct focus function names are required")
    if bool(command_json) == bool(candidate_object):
        raise ValueError("supply exactly one compiler command JSON or existing candidate object")
    if not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("positive finite evaluation deadline required")
    if out.exists():
        raise ValueError(f"evaluation result already exists: {out}")
    if out.name in {"seen.json", "seen.lock"}:
        raise ValueError("evaluation output uses a reserved cache name")
    raw, index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    base = frontier.load_json(raw)
    frontier.verify(root, base)
    if base.get("data_functions") is None:
        raise ValueError("baseline index must include strict and data reports")
    if not set(functions) <= {r["function"] for r in base["functions"]}:
        raise ValueError("focus function absent from current evidence")
    source_bytes, source_desc = frontier.read_bound(root, candidate, 4 * 1024 * 1024)
    source_sha = _sha(source_bytes)
    lineage = source_lineage(root, index, base, working_source, source_bytes) if working_source is not None else None
    prediction_spec, prediction_descriptor, prediction_error = _load_prediction(root, prediction)
    prediction_baseline_document = _prediction_baseline_report(root, base) if prediction is not None else None
    bound_paths = {frontier.local(root, Path(v["path"])) for v in base["inputs"].values()}
    if out in bound_paths | {index, candidate} | ({prediction} if prediction is not None else set()):
        raise ValueError("evaluation output aliases an input")
    target = frontier.local(root, Path(base["inputs"]["target_object"]["path"]))
    baseline = frontier.local(root, Path(base["inputs"]["candidate_object"]["path"]))
    result: dict[str, Any] = {"schema": SCHEMA, "owner": base["owner"], "functions": functions,
        "authority_advanced": False, "retained": False, "retention_ready": False,
        "linked_exact": None, "source_fidelity": "owner_review_required",
        "baseline_index": index_desc, "candidate_source": source_desc,
        "compiler_runs": 0, "objdiff_runs": 0, "cleanup_errors": [], "stage": "preflight"}
    directory = None
    if lineage is not None:
        result["source_lineage"] = lineage
    after_documents: dict[str, dict] = {}
    preserved = []
    cache_path = out.parent / "seen.json"
    try:
        compile_context = (_command_context(root, frontier.local(root, command_json), compiler_tools or [])
                           if command_json else None)
        if (candidate == frontier.local(root, Path(base["inputs"]["source"]["path"]))
                and source_sha == base["inputs"]["source"]["sha256"] and compile_context is not None
                and _same_baseline_context(root, base, compile_context)):
            result.update(status="duplicate_source", reason="candidate is the retained source; nothing to compile")
        else:
            def remaining() -> float:
                if working_source is not None:
                    working_source_binding(root, index, base, working_source)
                    frontier.verify(root, base)
                seconds = timeout - (time.monotonic() - started)
                if seconds <= 0:
                    raise TimeoutError("evaluation deadline exhausted")
                return seconds
            objdiff = Path(os.path.abspath(objdiff))
            readelf = Path(os.path.abspath(readelf))
            proof_tools = {str(p): compiler.digest(p) for p in (objdiff, readelf)}
            result["proof_tools"] = proof_tools
            context = (compile_context if command_json else
                       {"existing_object": _descriptor(frontier.local(root, candidate_object))})
            result["implementation"] = _implementation_binding()
            key = _sha(frontier.canonical({"index": base["index_sha256"], "source": source_desc,
                       "focus": functions, "compiler": context, "proof_tools": proof_tools,
                       "implementation": result["implementation"]}))
            result["context_key"] = key
            cached = _seen(root, cache_path, key)
            if cached is not None:
                result.update(status="duplicate_source", reason="this exact candidate/context was already measured",
                              reused_measurement=cached)
                frontier.verify(root, base)
                if frontier.read_bound(root, candidate, 4 * 1024 * 1024)[1] != source_desc:
                    raise ValueError("candidate source changed during duplicate lookup")
                if working_source is not None:
                    working_source_binding(root, index, base, working_source)
                result["stage"] = "complete"
                return result
            out.parent.mkdir(parents=True, exist_ok=True)
            directory = Path(tempfile.mkdtemp(prefix=".evaluate-", dir=out.parent))
            if candidate_object is None:
                result["stage"] = "compile"
                candidate_obj = directory / "candidate.o"
                result["compiler_runs"] = 1
                receipt = _compile_candidate(root, candidate, candidate_obj,
                    frontier.local(root, command_json), compiler_tools or [], remaining())
                if (receipt.get("schema") != "recovery_candidate_compile/v1"
                        or receipt.get("source_sha256") != source_sha
                        or receipt.get("object_sha256") != compiler.digest(candidate_obj)):
                    raise ValueError("compiler receipt does not bind this candidate source/object")
                result["compile_receipt"] = receipt
                result["compile_binding"] = "compiler_receipt"
            else:
                candidate_obj = frontier.local(root, candidate_object)
                if candidate_obj == out:
                    raise ValueError("output aliases existing candidate object")
                result["compile_binding"] = "caller_supplied_object; not source proof"
            result["stage"] = "object_inventory"
            target_inventory, base_inventory, candidate_inventory = [objects.inventory(p) for p in (target, baseline, candidate_obj)]
            result["candidate_object"] = _descriptor(candidate_obj)
            result["semantic_object_sha256"] = candidate_inventory["semantic_sha256"]
            result["semantic_object_equal_baseline"] = candidate_inventory["semantic_sha256"] == base_inventory["semantic_sha256"]
            comparison = objects.compare(target_inventory, base_inventory, candidate_inventory, functions)
            result["object_comparison"] = comparison
            result["physical_progress"] = _physical_progress_summary(comparison)
            if result["semantic_object_equal_baseline"]:
                result.update(status="duplicate_object", reason="allocated object/link semantics unchanged; skip objdiff")
            else:
                result["stage"] = "proof"
                for path in (target, baseline, candidate_obj):
                    check = bounded_process.run([str(readelf), "-SWsWr", "--", str(path)],
                                                cwd=root, timeout=remaining(), max_output=512 * 1024)
                    if check.returncode:
                        raise ValueError(f"independent readelf rejected {path}: {check.returncode}")
                strict_path, data_path = directory / "strict.json", directory / "data.json"
                with ThreadPoolExecutor(max_workers=2) as pool:
                    tasks = [pool.submit(_report, objdiff, target, candidate_obj, path, root, data, remaining())
                             for path, data in ((strict_path, False), (data_path, True))]
                    result["objdiff_runs"] = 2
                    for task in tasks:
                        task.result()
                summaries = {}
                after_documents = {}
                result["next_mismatch_evidence"] = {}
                for channel, path in (("strict", strict_path), ("data", data_path)):
                    document = frontier.load_json(path.read_bytes())
                    after_documents[channel] = document
                    _validate_report_objects(document, target_inventory, candidate_inventory)
                    summaries[channel] = frontier.summarize(document, channel)
                    result[channel + "_report"] = _descriptor(path)
                    result[channel + "_functions"] = summaries[channel]
                    result["next_mismatch_evidence"][channel] = _focus_evidence(document, functions)
                baseline_documents = {}
                try:
                    for channel in ("strict", "data"):
                        descriptor = base.get("inputs", {}).get(channel + "_report")
                        if not isinstance(descriptor, dict):
                            raise ValueError(f"baseline {channel} report descriptor missing")
                        baseline_raw, actual = frontier.read_bound(root, Path(descriptor["path"]), frontier.REPORT_LIMIT)
                        if actual != descriptor:
                            raise ValueError(f"baseline {channel} report changed")
                        baseline_document = frontier.load_json(baseline_raw)
                        if not isinstance(baseline_document, dict):
                            raise ValueError(f"baseline {channel} report is not an object")
                        baseline_documents[channel] = baseline_document
                except (OSError, ValueError, RuntimeError, KeyError, TypeError):
                    baseline_documents = {}
                result.update(_classify(base["functions"], base["data_functions"],
                                        summaries["strict"], summaries["data"], comparison, functions,
                                        baseline_documents=baseline_documents, after_documents=after_documents))
                result["changed_result_diagnostics"] = _changed_result_diagnostics(
                    root=root, baseline_documents=baseline_documents, after_documents=after_documents,
                    strict_path=strict_path, data_path=data_path, metric_changes=result.get("metric_changes", []),
                    object_comparison=comparison, reports_retained=keep_reports,
                )
                result["causal_groups"] = _causal_group_diagnostics(
                    baseline_documents, after_documents, functions)
                result["focus"] = {channel: [r for r in summaries[channel] if r["function"] in functions]
                                   for channel in ("strict", "data")}
                if candidate_object is not None:
                    result["retention_ready"] = False
                if base.get("compile_binding") != "receipt_hashes_match":
                    result["review_required"].append("baseline source/object compile receipt absent; use existing owner proof")
                    result["retention_ready"] = False
                result["next_action"] = ("owner source-fidelity review then retain verified candidate" if result["retention_ready"]
                    else "use first_mismatch and closed-channel regressions; do not repeat the same candidate")
            if any(compiler.digest(Path(path)) != sha for path, sha in proof_tools.items()):
                raise ValueError("proof executable changed during evaluation")
            if _descriptor(candidate_obj) != result["candidate_object"]:
                raise ValueError("candidate object changed during evaluation")
        frontier.verify(root, base)
        if frontier.read_bound(root, candidate, 4 * 1024 * 1024)[1] != source_desc:
            raise ValueError("candidate source changed during evaluation")
        if result.get("status") in {"exact", "improved"} and candidate_object is None and directory is not None:
            # Preserve only the useful source/object/receipt, not two whole-TU
            # reports per attempt. A measured gain still needs its object when
            # source fidelity or typed data requires review: do not force an
            # otherwise unnecessary rebuild just because readiness is false.
            for suffix, payload in (("candidate.c", source_bytes),
                                    ("candidate.o", candidate_obj.read_bytes()),
                                    ("compile.json", frontier.canonical(result["compile_receipt"]) + b"\n")):
                destination = out.with_name(out.stem + "." + suffix)
                frontier.local(root, destination)
                if destination.exists() or destination in bound_paths | {index, candidate}:
                    raise ValueError(f"verified candidate output already exists or aliases input: {destination}")
                compiler.atomic(destination, payload)
                preserved.append(destination)
            result["measured_candidate"] = {p.suffix if p.suffix != ".json" else "receipt": _descriptor(p) for p in preserved}
            if result.get("retention_ready"):
                result["verified_candidate"] = result["measured_candidate"]
        if working_source is not None:
            working_source_binding(root, index, base, working_source)
        result["stage"] = "complete"
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        result.update(status="failed", reason=str(exc)[:8000], retention_ready=False)
        if isinstance(exc, bounded_process.ProcessLimitError):
            result["diagnostics"] = compiler._diagnostics(exc.stdout, exc.stderr)
    finally:
        if prediction is not None:
            try:
                result["prediction_feedback"] = _prediction_feedback(
                    spec=prediction_spec,
                    prediction_descriptor=prediction_descriptor,
                    prediction_error=prediction_error,
                    index_desc=index_desc,
                    source_desc=source_desc,
                    functions=functions,
                    baseline_document=(prediction_baseline_document
                                       if result.get("status") in {"duplicate_object", "duplicate_source"}
                                       and "reused_measurement" not in result
                                       else None),
                    after_document=after_documents.get("strict"),
                    evaluation_status=("cached" if "reused_measurement" in result
                                       else result.get("status")),
                )
            except (OSError, ValueError, RuntimeError, KeyError, TypeError, AttributeError) as exc:
                result["prediction_feedback"] = {
                    "schema": PREDICTION_SCHEMA,
                    "diagnostic_only": True,
                    "authority_advanced": False,
                    "status": "unmapped",
                    "binding_status": "invalid",
                    "reason": f"prediction feedback unavailable: {str(exc)[:800]}",
                    "sites": [],
                    "counts": {"closed": 0, "still_different": 0, "unmapped": 0},
                }
        if result.get("status") == "failed":
            for path in preserved:
                try:
                    frontier.local(root, path).unlink()
                except (OSError, ValueError) as exc:
                    result["cleanup_errors"].append(str(exc))
        if directory is not None:
            if keep_reports and result.get("stage") == "complete":
                result["artifacts"] = {p.name: _descriptor(p) for p in directory.iterdir() if p.is_file()}
                result["retained_private_directory"] = str(directory)
            else:
                result["cleanup_errors"].extend(_cleanup(directory, root))
        if result["cleanup_errors"]:
            result["retention_ready"] = False
        result["seconds"] = time.monotonic() - started
        result["result_sha256"] = _sha(frontier.canonical(result))
        _atomic(out, result)
        if result.get("context_key") and result.get("status") not in {"failed", "duplicate_source"}:
            try:
                _remember(root, cache_path, result["context_key"], out)
            except (OSError, ValueError, RuntimeError) as exc:
                # Cache maintenance cannot erase a successfully measured gain.
                result["cache_warning"] = str(exc)
                result.pop("result_sha256", None)
                result["result_sha256"] = _sha(frontier.canonical(result))
                _atomic(out, result)
    return result


def _batch_relative(root: Path, value: Any, field: str) -> Path:
    """Resolve a manifest path while requiring an owner-root-relative name."""
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{field} must be a non-empty relative path")
    text = value.replace("\\", "/")
    windows = PureWindowsPath(text)
    parts = PurePosixPath(text).parts
    if (windows.is_absolute() or windows.drive or text.startswith("/")
            or not parts or any(part in {"", ".", ".."} for part in parts)):
        raise ValueError(f"{field} must be owner-root-relative")
    return frontier.local(root, root.joinpath(*parts))


def _batch_file(root: Path, value: Any, field: str, limit: int) -> tuple[Path, dict]:
    path = _batch_relative(root, value, field)
    if not path.is_file():
        raise ValueError(f"{field} is not a file: {value}")
    _, descriptor = frontier.read_bound(root, path, limit)
    return path, descriptor


def _batch_manifest(root: Path, manifest: Path) -> tuple[dict, dict, list[dict]]:
    if not isinstance(manifest, Path):
        manifest = Path(manifest)
    raw, manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
    try:
        document = frontier.load_json(raw)
    except (ValueError, TypeError, json.JSONDecodeError) as exc:
        raise ValueError(f"malformed batch manifest: {manifest}") from exc
    if not isinstance(document, dict) or set(document) != {"schema", "jobs"}:
        raise ValueError("batch manifest must contain only schema and jobs")
    if document.get("schema") != BATCH_SCHEMA:
        raise ValueError(f"unsupported batch manifest schema: {document.get('schema')!r}")
    raw_jobs = document.get("jobs")
    if not isinstance(raw_jobs, list) or not raw_jobs or len(raw_jobs) > BATCH_MAX_JOBS:
        raise ValueError(f"batch jobs must contain 1..{BATCH_MAX_JOBS} entries")
    jobs = []
    ids = set()
    folded_ids = set()
    for number, item in enumerate(raw_jobs, 1):
        if not isinstance(item, dict):
            raise ValueError(f"job {number} must be an object")
        allowed = {"id", "candidate", "functions", "candidate_object", "working_source"}
        if set(item) - allowed or not {"id", "candidate", "functions"} <= set(item):
            raise ValueError(f"job {number} must contain exactly id, candidate, functions and optional candidate_object")
        job_id = item["id"]
        device_name = job_id.upper().rstrip(".").split(".", 1)[0] if isinstance(job_id, str) else ""
        if (not isinstance(job_id, str) or not _BATCH_ID.fullmatch(job_id)
                or job_id.endswith(".") or device_name in _BATCH_DEVICE_IDS
                or job_id in ids or job_id.casefold() in folded_ids):
            raise ValueError(f"job {number} has an unsafe or duplicate id")
        ids.add(job_id)
        folded_ids.add(job_id.casefold())
        functions = item["functions"]
        if (not isinstance(functions, list) or not functions or len(functions) > 128
                or any(not isinstance(name, str) or not name.strip() for name in functions)
                or len(set(functions)) != len(functions)):
            raise ValueError(f"job {job_id} functions must be distinct non-empty names")
        candidate, candidate_desc = _batch_file(root, item["candidate"],
                                                 f"job {job_id} candidate", 4 * 1024 * 1024)
        candidate_object = candidate_object_desc = None
        if "candidate_object" in item:
            candidate_object, candidate_object_desc = _batch_file(
                root, item["candidate_object"], f"job {job_id} candidate_object", 64 * 1024 * 1024)
        jobs.append({"id": job_id, "candidate": candidate, "candidate_desc": candidate_desc,
                     "functions": functions, "candidate_object": candidate_object,
                     "candidate_object_desc": candidate_object_desc})
        if "working_source" in item:
            jobs[-1]["working_source"] = item["working_source"]
    return document, manifest_desc, jobs


def _batch_tools(objdiff: Path, readelf: Path) -> dict[str, str]:
    paths = {"objdiff": Path(os.path.abspath(objdiff)), "readelf": Path(os.path.abspath(readelf))}
    if any(not path.is_file() for path in paths.values()):
        raise ValueError("objdiff and readelf must be existing files")
    return {name: compiler.digest(path) for name, path in paths.items()}


def _batch_snapshot(root: Path, index: Path, manifest: Path, manifest_desc: dict,
                    jobs: list[dict], command_json: Path | None, compiler_tools: list[Path],
                    objdiff: Path, readelf: Path, base: dict) -> dict:
    context = (_command_context(root, command_json, compiler_tools) if command_json else None)
    _, index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    _, actual_manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
    if actual_manifest_desc != manifest_desc:
        raise ValueError("batch manifest changed during preflight")
    for job in jobs:
        if job.get("working_source") is not None:
            working_source_binding(root, index, base, job["working_source"])
    return {"index": index_desc, "manifest": actual_manifest_desc,
            "jobs": {job["id"]: {"candidate": job["candidate_desc"],
                                  "candidate_object": job["candidate_object_desc"]}
                     for job in jobs},
            "command_context": context,
            "proof_tools": _batch_tools(objdiff, readelf),
            "base_index_sha256": base.get("index_sha256"),
            "implementation": _implementation_binding()}


def _batch_drift(root: Path, index: Path, manifest: Path, snapshot: dict, jobs: list[dict],
                 command_json: Path | None, compiler_tools: list[Path],
                 objdiff: Path, readelf: Path, base: dict) -> list[str]:
    reasons = []
    try:
        current_raw, current_index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
        if current_index_desc != snapshot["index"]:
            reasons.append("index changed")
        current_base = frontier.load_json(current_raw)
        frontier.verify(root, current_base)
        if current_base.get("index_sha256") != snapshot["base_index_sha256"]:
            reasons.append("index identity changed")
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        reasons.append(f"index verification failed: {str(exc)[:300]}")
    try:
        _, current_manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
        if current_manifest_desc != snapshot["manifest"]:
            reasons.append("manifest changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"manifest changed: {str(exc)[:300]}")
    for job in jobs:
        expected = snapshot["jobs"][job["id"]]
        try:
            if job.get("working_source") is not None:
                working_source_binding(root, index, base, job["working_source"])
            _, current_candidate_desc = frontier.read_bound(root, job["candidate"], 4 * 1024 * 1024)
            if current_candidate_desc != expected["candidate"]:
                reasons.append(f"candidate changed: {job['id']}")
            if job["candidate_object"] is not None:
                _, current_object_desc = frontier.read_bound(root, job["candidate_object"], 64 * 1024 * 1024)
                if current_object_desc != expected["candidate_object"]:
                    reasons.append(f"candidate object changed: {job['id']}")
        except (OSError, ValueError) as exc:
            reasons.append(f"candidate changed: {job['id']}: {str(exc)[:300]}")
    if command_json:
        try:
            if _command_context(root, command_json, compiler_tools) != snapshot["command_context"]:
                reasons.append("compiler context changed")
        except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
            reasons.append(f"compiler context changed: {str(exc)[:300]}")
    try:
        current_tools = _batch_tools(objdiff, readelf)
        if current_tools != snapshot["proof_tools"]:
            reasons.append("proof tool changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"proof tool changed: {str(exc)[:300]}")
    try:
        if _implementation_binding() != snapshot["implementation"]:
            reasons.append("evaluator implementation changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"evaluator implementation changed: {str(exc)[:300]}")
    return list(dict.fromkeys(reasons))


def _batch_failure(job: dict, reason: str, status: str = "failed") -> dict:
    return {"schema": SCHEMA, "owner": "batch", "functions": job["functions"],
            "status": status, "reason": reason[:8000], "compiler_runs": 0,
            "objdiff_runs": 0, "cleanup_errors": [], "retention_ready": False,
            "retained": False, "authority_advanced": False, "stage": "batch"}


def _batch_run(root: Path, index: Path, job: dict, result_path: Path,
               objdiff: Path, readelf: Path, command_json: Path | None,
               compiler_tools: list[Path], timeout: float) -> dict:
    kwargs = {"root": root, "index": index, "candidate": job["candidate"],
              "functions": job["functions"], "out": result_path, "objdiff": objdiff,
              "readelf": readelf, "timeout": timeout}
    if "working_source" in job:
        kwargs["working_source"] = job["working_source"]
    if job["candidate_object"] is not None:
        # A per-job object is an explicit replay request.  It wins over an
        # optional common recipe in mixed manifests; no compile is performed.
        kwargs["candidate_object"] = job["candidate_object"]
    elif command_json is not None:
        kwargs.update(command_json=command_json, compiler_tools=compiler_tools)
    else:
        raise ValueError("batch job has neither candidate_object nor common command_json")
    return evaluate(**kwargs)


def _batch_result_descriptor(root: Path, path: Path) -> dict:
    raw, descriptor = frontier.read_bound(root, path, LIMIT)
    # Loading here catches a corrupt/mock result before it enters the summary.
    document = frontier.load_json(raw)
    if not isinstance(document, dict):
        raise ValueError(f"batch result is not an object: {path}")
    return descriptor


def _batch_alias_result(job: dict, canonical: dict, canonical_path: Path,
                        canonical_id: str, root: Path) -> dict:
    status = canonical.get("status")
    if status in {"failed", "not_scheduled"}:
        return _batch_failure(job, f"collapsed with {status} job {canonical_id}", status)
    result = {"schema": SCHEMA, "owner": "batch", "functions": job["functions"],
              "status": "duplicate_source", "reason": f"identical candidate/context collapsed with {canonical_id}",
              "reused_batch_job": canonical_id, "reused_result": _batch_result_descriptor(root, canonical_path),
              "compiler_runs": 0, "objdiff_runs": 0, "cleanup_errors": [],
              "retention_ready": False, "retained": False, "authority_advanced": False}
    for key in ("gains", "regressions", "strict", "data", "focus"):
        if key in canonical:
            result[key] = canonical[key]
    return result


def _batch_improvement_rows(result: dict) -> int:
    structured = 0
    has_structured = False
    focus = set(result.get("functions", []))
    for change in result.get("metric_changes", []):
        if not isinstance(change, dict) or (focus and change.get("function") not in focus):
            continue
        before, after = change.get("before"), change.get("after")
        if isinstance(before, dict) and isinstance(after, dict):
            try:
                has_structured = True
                structured += max(0, int(before.get("diff_rows", 0)) - int(after.get("diff_rows", 0)))
            except (TypeError, ValueError):
                pass
    if has_structured:
        for value in result.get("gains", []):
            if str(value).startswith(("relocation:", "physical:")):
                match = re.search(r":\s*(-?\d+)\s*->\s*(-?\d+)", str(value))
                if match:
                    structured += max(0, int(match.group(1)) - int(match.group(2)))
        return structured
    total = 0
    for value in result.get("gains", []):
        match = re.search(r":\s*(-?\d+)\s*->\s*(-?\d+)", str(value))
        if match:
            total += max(0, int(match.group(1)) - int(match.group(2)))
    return total


def _batch_result_event(*, root: Path, job: dict, result: dict, path: Path,
                        baseline: dict, drift_reasons: list[str]) -> dict:
    # Notify only after the per-job result is durable. This is early diagnostic
    # delivery, not a substitute for final drift checks or composition proof.
    defaults = {"status": "unknown", "functions": job["functions"], "compiler_runs": 0,
                "objdiff_runs": 0, "retention_ready": False, "retained": False,
                "seconds": None, "cleanup_errors": []}
    summary = _dispatch_summary(defaults | result)
    event = {"schema": BATCH_EVENT_SCHEMA, "event": "job_completed", "id": job["id"],
             "status": result.get("status", "unknown"), "functions": job["functions"],
             "result": _batch_result_descriptor(root, path), "baseline_index": baseline,
             "summary": summary, "diagnostic_only": True, "authority_advanced": False,
             "adoption_ready": False, "batch_complete": False,
             "drift_detected_so_far": bool(drift_reasons)}
    if len(frontier.canonical(event)) > 16 * 1024:
        event["summary"] = {"status": event["status"], "details_in_result": True,
                            "compiler_runs": result.get("compiler_runs", 0),
                            "objdiff_runs": result.get("objdiff_runs", 0)}
    if len(frontier.canonical(event)) > 16 * 1024:
        raise ValueError("batch result notification exceeds compact event limit; use durable result")
    # A consumer may annotate its event; never expose mutable frozen bindings
    # or evaluator payloads shared with the scheduler.
    return frontier.load_json(frontier.canonical(event))


def evaluate_batch(*, root: Path, index: Path, manifest: Path, out: Path,
                   objdiff: Path, readelf: Path, command_json: Path | None = None,
                   compiler_tools: list[Path] | None = None, workers: int = 2,
                   timeout: float = 120,
                   on_result: Callable[[dict], None] | None = None) -> dict:
    """Measure a bounded batch of independent candidates using ``evaluate``.

    The manifest and every input are frozen before the first worker is launched.
    Batch scheduling is deliberately measurement-only: successful results are
    retained as per-job evaluator files, while composition and adoption remain
    explicit owner decisions.
    """
    started = time.monotonic()
    root = Path(os.path.abspath(root))
    index = frontier.local(root, Path(index))
    manifest = frontier.local(root, Path(manifest))
    out = frontier.local(root, Path(out))
    out.relative_to(root / "build")
    if out.exists():
        raise ValueError(f"batch result already exists: {out}")
    if not isinstance(workers, int) or isinstance(workers, bool) or not 1 <= workers <= BATCH_MAX_WORKERS:
        raise ValueError(f"workers must be between 1 and {BATCH_MAX_WORKERS}")
    if not isinstance(timeout, (int, float)) or isinstance(timeout, bool) or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("positive finite batch deadline required")
    if on_result is not None and not callable(on_result):
        raise ValueError("on_result must be callable")
    _, manifest_desc, jobs = _batch_manifest(root, manifest)
    if bool(command_json) and all(job["candidate_object"] is not None for job in jobs):
        # A supplied common recipe is harmless for replay, but it still becomes
        # part of the frozen context and catches tool drift consistently.
        command_json = frontier.local(root, Path(command_json))
    elif command_json:
        command_json = frontier.local(root, Path(command_json))
    compiler_tools = [Path(os.path.abspath(root / Path(tool))) for tool in (compiler_tools or [])]
    if command_json is None:
        if compiler_tools:
            raise ValueError("compiler_tools require a common command_json")
        if any(job["candidate_object"] is None for job in jobs):
            raise ValueError("common command_json is required unless every job supplies candidate_object")
    elif not command_json.is_file():
        raise ValueError(f"command_json is not a file: {command_json}")
    objdiff = Path(os.path.abspath(objdiff))
    readelf = Path(os.path.abspath(readelf))
    raw, _ = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    base = frontier.load_json(raw)
    frontier.verify(root, base)
    if base.get("data_functions") is None:
        raise ValueError("baseline index must include strict and data reports")
    known_functions = {row["function"] for row in base.get("functions", [])}
    for job in jobs:
        if not set(job["functions"]) <= known_functions:
            raise ValueError(f"job {job['id']} focus function absent from current evidence")
    snapshot = _batch_snapshot(root, index, manifest, manifest_desc, jobs, command_json,
                               compiler_tools, objdiff, readelf, base)
    job_dir = frontier.local(root, out.parent / (out.stem + ".jobs"))
    job_dir.relative_to(root / "build")
    if job_dir.exists():
        raise ValueError(f"batch output directory already exists: {job_dir}")
    job_dir.mkdir(parents=True, exist_ok=False)
    result_paths = {job["id"]: job_dir / f"{job['id']}.json" for job in jobs}
    for path in result_paths.values():
        frontier.local(root, path)
    # Collapse only byte/context/function-equivalent jobs. Paths themselves do
    # not participate, so aliases are measured once and get explicit reuse files.
    groups: dict[str, list[dict]] = {}
    context_key = _sha(frontier.canonical({"command": snapshot["command_context"],
                                           "index": snapshot["index"]["sha256"],
                                           "proof": snapshot["proof_tools"]}))
    for job in jobs:
        key = _sha(frontier.canonical({"candidate": job["candidate_desc"]["sha256"],
                                       "candidate_object": (job["candidate_object_desc"] or {}).get("sha256"),
                                       "functions": sorted(job["functions"]), "context": context_key}))
        groups.setdefault(key, []).append(job)
    canonical = [entries[0] for entries in groups.values()]
    aliases = {job["id"]: entries[0] for entries in groups.values() for job in entries[1:]}
    results: dict[str, dict] = {}
    drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                 compiler_tools, objdiff, readelf, base)
    pending = list(canonical)
    active = {}
    notification_errors: list[str] = []
    notifications_enabled = on_result is not None

    def notify(job: dict) -> None:
        nonlocal notifications_enabled
        if not notifications_enabled:
            return
        try:
            event = _batch_result_event(root=root, job=job, result=results[job["id"]],
                                        path=result_paths[job["id"]], baseline=snapshot["index"],
                                        drift_reasons=drift_reasons)
            on_result(event)
        except Exception as exc:
            # A broken consumer/pipe must not mask a measured outcome or stop
            # siblings. Keep the ordinary durable batch result and stop writes.
            notification_errors.append(f"{type(exc).__name__}: {exc}"[:1000])
            notifications_enabled = False

    def schedule(pool: ThreadPoolExecutor) -> None:
        nonlocal drift_reasons
        while pending and len(active) < workers and not drift_reasons:
            drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                         compiler_tools, objdiff, readelf, base)
            if drift_reasons:
                break
            job = pending.pop(0)
            active[pool.submit(_batch_run, root, index, job, result_paths[job["id"]],
                               objdiff, readelf, command_json, compiler_tools, timeout)] = job
    with ThreadPoolExecutor(max_workers=workers) as pool:
        schedule(pool)
        while active:
            done, _ = wait(tuple(active), return_when=FIRST_COMPLETED)
            completed_jobs = []
            for future in done:
                job = active.pop(future)
                try:
                    result = future.result()
                    if not isinstance(result, dict) or not isinstance(result.get("status"), str):
                        raise ValueError("evaluate returned a non-object result")
                except Exception as exc:
                    result = _batch_failure(job, str(exc))
                if not result_paths[job["id"]].is_file():
                    _atomic(result_paths[job["id"]], result)
                results[job["id"]] = result
                completed_jobs.append(job)
            if not drift_reasons:
                drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                             compiler_tools, objdiff, readelf, base)
            for job in completed_jobs:
                notify(job)
            schedule(pool)
    for job in pending:
        result = _batch_failure(job, "batch input drift stopped scheduling", "not_scheduled")
        _atomic(result_paths[job["id"]], result)
        results[job["id"]] = result
        notify(job)
    # Fan out aliases only after their canonical result is stable.
    for job in jobs:
        if job["id"] in aliases:
            canonical_job = aliases[job["id"]]
            canonical_path = result_paths[canonical_job["id"]]
            result = _batch_alias_result(job, results[canonical_job["id"]], canonical_path,
                                         canonical_job["id"], root)
            _atomic(result_paths[job["id"]], result)
            results[job["id"]] = result
            notify(job)
    final_drift = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                               compiler_tools, objdiff, readelf, base)
    drift_reasons = list(dict.fromkeys(drift_reasons + final_drift))
    records = []
    result_map = {}
    positive = []
    for job in jobs:
        result = results[job["id"]]
        descriptor = _batch_result_descriptor(root, result_paths[job["id"]])
        record = {"id": job["id"], "status": result.get("status"),
                  "canonical_id": aliases.get(job["id"], job)["id"],
                  "candidate": job["candidate_desc"], "functions": job["functions"],
                  "candidate_object": job["candidate_object_desc"],
                  "result": descriptor, "compiler_runs": result.get("compiler_runs", 0),
                  "objdiff_runs": result.get("objdiff_runs", 0),
                  "retention_ready": bool(result.get("retention_ready", False))}
        records.append(record)
        result_map[job["id"]] = {"status": record["status"], "result": descriptor,
                                  "gains": result.get("gains", [])[:16]}
        if record["status"] in {"exact", "improved"} and job["id"] not in aliases:
            gains = result.get("gains", [])
            positive.append({"id": job["id"], "status": record["status"],
                             "gains": gains[:16], "result": descriptor,
                             "improvement_rows": _batch_improvement_rows(result),
                             "retention_ready": record["retention_ready"]})
    positive.sort(key=lambda item: (item["status"] != "exact", -item["improvement_rows"],
                                    -len(item["gains"]), item["id"]))
    statuses = [record["status"] for record in records]
    if drift_reasons:
        status = "drifted"
    elif any(item == "failed" for item in statuses):
        status = "partial" if any(item in {"exact", "improved"} for item in statuses) else "failed"
    else:
        status = "complete"
    summary = {"schema": BATCH_SCHEMA, "status": status, "manifest": manifest_desc,
               "baseline_index": snapshot["index"], "context_sha256": context_key,
               "proof_tools": snapshot["proof_tools"],
               "jobs": records, "results": result_map,
               "measured_gains": positive, "best_positive_candidates": positive[:3],
               "drift_detected": bool(drift_reasons), "drift_reasons": drift_reasons[:16],
               "retention_ready": bool(positive) and not drift_reasons
                   and all(item["retention_ready"] for item in positive),
               "authority_advanced": False, "retained": False, "adoption_ready": False,
               "composition": None, "compiler_runs": sum(r["compiler_runs"] for r in records),
               "objdiff_runs": sum(r["objdiff_runs"] for r in records),
               "seconds": time.monotonic() - started}
    if notification_errors:
        summary["notification_errors"] = notification_errors
    _atomic(out, summary)
    return summary


def add_batch_arguments(parser: Any) -> None:
    parser.add_argument("--index", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, required=True)
    parser.add_argument("--readelf", type=Path, required=True)
    parser.add_argument("--command-json", type=Path)
    parser.add_argument("--compiler-tool", type=Path, action="append", dest="compiler_tools")
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--stream", action="store_true",
                        help="emit bounded JSONL results as jobs finish, then the final batch summary")


def dispatch_batch(args: Any) -> int:
    values = vars(args).copy()
    values.pop("action", None)
    stream = values.pop("stream", False)
    if stream:
        values["on_result"] = lambda event: print(json.dumps(event, sort_keys=True), flush=True)
    result = evaluate_batch(**values)
    summary = {key: result[key] for key in
               ("status", "drift_detected", "drift_reasons", "compiler_runs", "objdiff_runs", "retention_ready", "seconds")}
    summary.update(jobs=[{"id": row["id"], "status": row["status"]} for row in result["jobs"]],
                   best_positive_candidates=result["best_positive_candidates"])
    if stream:
        summary.update(schema=BATCH_EVENT_SCHEMA, event="batch_completed", batch_complete=True,
                       result=_descriptor(Path(os.path.abspath(Path(args.root) / args.out))),
                       adoption_ready=False, authority_advanced=False)
    if result.get("notification_errors"):
        summary["notification_errors"] = result["notification_errors"]
    try:
        print(json.dumps(summary, sort_keys=True), flush=stream)
    except OSError:
        if not stream:
            raise
        # Final durable result already exists even when the output reader left.
    return 2 if result["status"] in {"failed", "drifted"} else 0


def add_arguments(parser: Any) -> None:
    parser.add_argument("--index", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--prediction", type=Path,
                        help="optional diagnostic prediction JSON (recovery_prediction/v1)")
    parser.add_argument("--function", action="append", required=True, dest="functions")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, required=True)
    parser.add_argument("--readelf", type=Path, required=True)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--command-json", type=Path)
    group.add_argument("--candidate-object", type=Path)
    parser.add_argument("--compiler-tool", type=Path, action="append", dest="compiler_tools")
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--keep-reports", action="store_true")


def dispatch(args: Any) -> int:
    values = vars(args).copy()
    values.pop("action", None)
    try:
        result = evaluate(**values)
        summary = _dispatch_summary(result)
        # Only describe the durable file after evaluate publishes it. A missing
        # or unreadable receipt is an entry/publication error, not a new probe.
        output = Path(os.path.abspath(Path(args.root) / args.out))
        summary["result"] = _descriptor(output)
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        # Preflight and publication errors may occur before/after evaluate's
        # private transaction. Never write to an unvalidated --out or replace
        # an earlier receipt just to manufacture a failed result.
        failure = {"schema": "recovery_evaluation_cli/v1", "status": "failed",
                   "stage": "entry_or_publication", "error_type": type(exc).__name__,
                   "reason": str(exc)[:2048], "result_published": None,
                   "retained": False, "authority_advanced": False}
        print(json.dumps(failure, sort_keys=True), flush=True)
        return 2
    print(json.dumps(summary, sort_keys=True), flush=True)
    return 2 if result["status"] == "failed" else 0


def main(argv: list[str] | None = None) -> int:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    add_arguments(parser)
    return dispatch(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
