#!/usr/bin/env python3
"""Bounded, read-only evidence for MWCC virtual-register pool reuse.

The reuse analysis is deliberately limited to confirmed GPR operands observed
by the same-session producer.  Capture coverage records other observed banks
explicitly, so an FPR-only session is reported as unsupported evidence rather
than as an empty GPR result.  It does not assign source ownership or propose a
source change.  A candidate objdiff row is used only as the target-role and
machine-row identity for an observed event.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any


SCHEMA = "mwcc_temp_pool_reuse/v1"
MAX_INPUT_BYTES = 64 * 1024 * 1024
MAX_OUTPUT_BYTES = 256 * 1024
MAX_COLLISIONS = 128
MAX_EXEMPLARS = 3
MAX_WRAP_BOUNDARIES = 4
MAX_ALIGNMENT_CELLS = 4096
MAX_ALIGNMENT_BEST = 4
POOL_BASE = 32  # GPR 0..31 are pre-coloured; pool IDs begin at 32.
SUPPORTED_OPERAND_BANKS = ("GPR",)
OFFSET_DELTAS = (-3, -2, -1, 1, 2, 3)
_INTEGER = re.compile(r"[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+)\Z")
_GPR = re.compile(r"(?<![A-Za-z0-9_])r([0-9]|[12][0-9]|3[01])(?![A-Za-z0-9_])", re.I)


def canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def _integer(value: Any, label: str) -> int:
    if isinstance(value, bool):
        raise ValueError(f"{label} must be an integer")
    if isinstance(value, int):
        return value
    if not isinstance(value, str) or _INTEGER.fullmatch(value.strip()) is None:
        raise ValueError(f"{label} must be a decimal or hexadecimal integer")
    text = value.strip()
    sign = -1 if text.startswith("-") else 1
    digits = text[1:] if text[:1] in {"+", "-"} else text
    base = 16 if digits.lower().startswith("0x") else 10
    return sign * int(digits[2:] if base == 16 else digits, base)


def _read_json(path: Path, label: str) -> tuple[Any, dict[str, Any]]:
    path = Path(path)
    with path.open("rb") as stream:
        raw = stream.read(MAX_INPUT_BYTES + 1)
    if len(raw) > MAX_INPUT_BYTES:
        raise ValueError(f"{label} exceeds {MAX_INPUT_BYTES} bytes: {path}")

    def unique(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"duplicate JSON key in {label}: {key}")
            result[key] = value
        return result

    try:
        document = json.loads(raw.decode("utf-8"), object_pairs_hook=unique)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"malformed {label}: {path}") from exc
    return document, {
        "path": str(path.resolve()),
        "size_bytes": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
    }


def _symbols(document: Any, side: str, label: str) -> list[dict[str, Any]]:
    if not isinstance(document, dict) or not isinstance(document.get(side), dict):
        raise ValueError(f"{label} is missing the {side} channel")
    symbols = document[side].get("symbols")
    if not isinstance(symbols, list):
        raise ValueError(f"{label}.{side}.symbols must be a list")
    if not all(isinstance(symbol, dict) for symbol in symbols):
        raise ValueError(f"{label}.{side}.symbols contains a non-object")
    return symbols


def _select_symbol(symbols: list[dict[str, Any]], function: str, side: str) -> dict[str, Any]:
    matches = [symbol for symbol in symbols if symbol.get("name") == function]
    if len(matches) != 1:
        raise ValueError(f"report must contain exactly one {side} function {function!r}; found {len(matches)}")
    symbol = matches[0]
    if not isinstance(symbol.get("instructions"), list):
        raise ValueError(f"report {side} function {function!r} has no instruction list")
    return symbol


def _symbol_position(symbols: list[dict[str, Any]], symbol: dict[str, Any]) -> int:
    """Return the selected symbol's opposite-channel pairing index."""
    for index, candidate in enumerate(symbols):
        if candidate is symbol:
            return index
    raise ValueError("selected function is not present in its report symbol table")


def _validate_pairing(
    left_symbols: list[dict[str, Any]],
    right_symbols: list[dict[str, Any]],
    left_symbol: dict[str, Any],
    right_symbol: dict[str, Any],
    function: str,
) -> None:
    """Validate reciprocal objdiff indices without comparing raw index values."""
    left_index = _symbol_position(left_symbols, left_symbol)
    right_index = _symbol_position(right_symbols, right_symbol)
    for symbol, expected, side in (
        (left_symbol, right_index, "target"),
        (right_symbol, left_index, "candidate"),
    ):
        if symbol.get("target_symbol") is None:
            continue
        actual = _integer(symbol.get("target_symbol"), f"{side} {function}.target_symbol")
        if actual != expected:
            raise ValueError(
                f"{side} function {function!r} does not reciprocally pair with the selected opposite symbol"
            )


def _instruction_rows(symbol: dict[str, Any], side: str, function: str) -> list[dict[str, Any]]:
    rows = symbol["instructions"]
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            raise ValueError(f"{side}.{function}.instructions[{index}] must be an object")
        instruction = row.get("instruction")
        if instruction is not None and not isinstance(instruction, dict):
            raise ValueError(f"{side}.{function}.instructions[{index}].instruction must be an object or null")
        if isinstance(instruction, dict):
            formatted = instruction.get("formatted")
            if formatted is not None and not isinstance(formatted, str):
                raise ValueError(f"{side}.{function}.instructions[{index}].formatted must be text")
    return rows


def _formatted(row: dict[str, Any] | None) -> str | None:
    instruction = row.get("instruction") if isinstance(row, dict) else None
    if instruction is None:
        return None
    text = instruction.get("formatted")
    return text if isinstance(text, str) else None


def _gpr_roles(text: str | None) -> list[int]:
    return [int(match.group(1)) for match in _GPR.finditer(text or "")]


def _word(value: Any, label: str) -> int:
    result = _integer(value, label)
    if not 0 <= result <= 0xFFFFFFFF:
        raise ValueError(f"{label} is outside the 32-bit machine-word range")
    return result


def _bytes_word(value: Any, label: str) -> int:
    if isinstance(value, str):
        text = value.strip().replace(" ", "").replace("_", "")
        if text.lower().startswith("0x"):
            text = text[2:]
        if len(text) != 8 or re.fullmatch(r"[0-9a-fA-F]{8}", text) is None:
            raise ValueError(f"{label} must contain exactly four machine bytes")
        return int(text, 16)
    if isinstance(value, list) and len(value) == 4 and all(isinstance(part, int) and 0 <= part <= 255 for part in value):
        return int.from_bytes(bytes(value), "big")
    raise ValueError(f"{label} must contain exactly four machine bytes")


def _optional_row_word(instruction: dict[str, Any], row_label: str) -> int | None:
    for key in ("ppc_word", "machine_word", "word"):
        if key in instruction:
            return _word(instruction[key], f"{row_label}.{key}")
    for key in ("ppc_bytes", "machine_bytes", "bytes"):
        if key in instruction:
            return _bytes_word(instruction[key], f"{row_label}.{key}")
    return None


def _verify_machine_rows(
    candidate: dict[str, Any],
    rows: list[dict[str, Any]],
    machine: list[dict[str, Any]],
    function: str,
) -> dict[str, int | str]:
    candidate_start = _integer(candidate.get("address"), f"candidate {function}.address")
    real_rows: list[tuple[int, dict[str, Any]]] = []
    for report_index, row in enumerate(rows):
        instruction = row.get("instruction")
        if instruction is None:
            continue
        if not isinstance(instruction, dict) or not isinstance(instruction.get("formatted"), str):
            raise ValueError(f"candidate {function}.instructions[{report_index}] is not canonical")
        size = _integer(instruction.get("size", 4), f"candidate {function}.instructions[{report_index}].size")
        if size != 4:
            raise ValueError(f"candidate {function}.instructions[{report_index}] is not a four-byte instruction")
        address = _integer(instruction.get("address"), f"candidate {function}.instructions[{report_index}].address")
        expected = candidate_start + len(real_rows) * 4
        if address != expected:
            raise ValueError(f"candidate row address drift at row {report_index}: expected {expected}, got {address}")
        real_rows.append((report_index, instruction))
    if len(real_rows) != len(machine):
        raise ValueError(f"machine emission count {len(machine)} does not match candidate rows {len(real_rows)} for {function}")

    row_word_pairs = 0
    for machine_index, (event, (report_index, instruction)) in enumerate(zip(machine, real_rows)):
        actual_index = _integer(event.get("instruction_index"), f"machine event {machine_index}.instruction_index")
        emitted_offset = _integer(event.get("emitted_offset"), f"machine event {machine_index}.emitted_offset")
        if actual_index != machine_index or emitted_offset != machine_index * 4:
            raise ValueError(f"machine/candidate row identity drift at machine row {machine_index}")
        bytes_word = _bytes_word(event.get("ppc_bytes"), f"machine event {machine_index}.ppc_bytes")
        event_word = _word(event.get("ppc_word"), f"machine event {machine_index}.ppc_word")
        if bytes_word != event_word:
            raise ValueError(f"machine word drift at machine row {machine_index}: ppc_bytes != ppc_word")
        row_word = _optional_row_word(instruction, f"candidate row {report_index}.instruction")
        if row_word is not None and row_word != event_word:
            raise ValueError(f"machine word drift at candidate row {report_index}")
        if row_word is not None:
            row_word_pairs += 1
        if "source_offset" in instruction and instruction["source_offset"] != event.get("source_offset"):
            raise ValueError(f"source offset drift at candidate row {report_index}")
    verification: dict[str, int | str] = {
        "status": "verified" if row_word_pairs == len(machine) else "unverified",
        "machine_event_count": len(machine),
        "candidate_instruction_count": len(real_rows),
        "word_pairs_checked": row_word_pairs,
        "emission_self_pairs_checked": len(machine),
    }
    if row_word_pairs != len(machine):
        verification["reason"] = "canonical candidate rows do not carry machine-word fields"
    return verification


def _captures(events: list[dict[str, Any]], function: str) -> dict[str, list[dict[str, int]]]:
    grouped: dict[str, list[dict[str, int]]] = defaultdict(list)
    for event in events:
        if event.get("event_kind") != "pcode_capture" or event.get("function") != function:
            continue
        if event.get("confirmed") is not True or event.get("operand_bank") != "GPR":
            continue
        token = event.get("pcode_token")
        if not isinstance(token, str) or not token:
            raise ValueError("confirmed GPR capture has no pcode_token")
        grouped[token].append({
            "operand_ordinal": _integer(event.get("operand_ordinal"), "capture.operand_ordinal"),
            "virtual_id": _integer(event.get("operand_index"), "capture.operand_index"),
            "final_color": _integer(event.get("final_color"), "capture.final_color"),
            "operand_flags": _integer(event.get("operand_flags", 0), "capture.operand_flags"),
        })
    for token, rows in grouped.items():
        ordinals = [row["operand_ordinal"] for row in rows]
        if len(set(ordinals)) != len(ordinals):
            raise ValueError(f"duplicate confirmed operand ordinal for pcode token {token}")
        if any(
            row["operand_ordinal"] < 0
            or row["virtual_id"] < 0
            or row["final_color"] < 0
            or row["final_color"] > 31
            for row in rows
        ):
            raise ValueError(f"invalid confirmed GPR identity for pcode token {token}")
    return grouped


def _native_events(document: dict[str, Any], function: str) -> list[dict[str, Any]]:
    """Adapt only independently cross-checked native observed color joins.

    A native UNKNOWN operand is never promoted from its physical register.
    source_offset is a compiler location, not authenticated source ownership.
    """
    if document.get("schema_version") != 1 or document.get("target") != function:
        raise ValueError("native schema/target mismatch")
    machine, captures = document.get("machine_emissions"), document.get("regalloc_pcode")
    if not isinstance(machine, list) or not isinstance(captures, list):
        raise ValueError("native capture requires machine_emissions and regalloc_pcode lists")
    events: list[dict[str, Any]] = []
    seen_captures: set[tuple[str, int, int]] = set()
    for emission in machine:
        if not isinstance(emission, dict) or emission.get("function") != function:
            raise ValueError("native machine function mismatch")
        operands = emission.get("operands")
        if not isinstance(operands, list) or len(operands) != emission.get("operand_count"):
            raise ValueError("native operand count mismatch")
        event = dict(emission, event_kind="machine_emission", native=True)
        event["native_gpr_operands"] = []
        events.append(event)
        for ordinal, operand in enumerate(operands):
            if not isinstance(operand, dict) or operand.get("ordinal") != ordinal:
                raise ValueError("native operand ordinal mismatch")
            if operand.get("kind") != 0 or operand.get("class") != 4:
                continue
            item = {"operand_ordinal": ordinal, "final_color": operand.get("color"),
                    "virtual_id": None}
            event["native_gpr_operands"].append(item)
            if operand.get("join_status") != "observed":
                events.append({"event_kind": "pcode_capture", "function": function,
                               "operand_bank": "GPR", "confirmed": False})
                continue
            index = _integer(operand.get("color_observation"), "native color_observation")
            if not 0 <= index < len(captures) or not isinstance(captures[index], dict):
                raise ValueError("native color observation out of range")
            capture = captures[index]
            for key in ("pcode", "pcode_token", "opcode", "operand_count", "source_offset"):
                if capture.get(key) != emission.get(key):
                    raise ValueError(f"native {key} join mismatch")
            for key, value in (("operand_ordinal", ordinal), ("operand_index", operand.get("vreg")),
                               ("color", operand.get("color")), ("class", 4),
                               ("observation_index", index), ("pass", operand.get("allocation_pass")),
                               ("operand_flags", operand.get("pre_color_flags"))):
                if capture.get(key) != value:
                    raise ValueError(f"native {key} join mismatch")
            try:
                before = bytes.fromhex(capture["operand_raw"])
                after = bytes.fromhex(operand["raw"])
            except (KeyError, TypeError, ValueError) as exc:
                raise ValueError("native operand raw encoding invalid") from exc
            if (len(before) != 12 or len(after) != 12 or before[:4] != after[:4]
                    or before[6:] != after[6:]
                    or int.from_bytes(before[4:6], "little", signed=True) != operand["vreg"]
                    or int.from_bytes(after[4:6], "little", signed=True) != operand["color"]):
                raise ValueError("native operand raw join mismatch")
            item["virtual_id"] = operand["vreg"]
            key = (capture["pcode_token"], ordinal, index)
            if key not in seen_captures:
                events.append(dict(capture, event_kind="pcode_capture", function=function,
                                   confirmed=True, operand_bank="GPR", final_color=capture["color"]))
                seen_captures.add(key)
    return events


def _operand_coverage(events: list[dict[str, Any]], function: str) -> dict[str, Any]:
    """Summarize operand-bank coverage before the GPR-only analysis runs.

    The capture producer can emit both canonical GPR and FPR operands, but this
    utility authenticates only canonical GPR virtual-register identities.
    Counting recognized bank labels independently of confirmation makes
    unsupported FPR evidence visible without treating malformed or missing
    labels as a supported bank.
    """
    observed_counts: dict[str, int] = defaultdict(int)
    confirmed_supported_counts: dict[str, int] = defaultdict(int)
    for event in events:
        if event.get("event_kind") != "pcode_capture" or event.get("function") != function:
            continue
        bank = event.get("operand_bank")
        if bank not in {"GPR", "FPR"}:
            continue
        observed_counts[bank] += 1
        if bank in SUPPORTED_OPERAND_BANKS and event.get("confirmed") is True:
            confirmed_supported_counts[bank] += 1

    observed_banks = sorted(observed_counts)
    discarded = {
        bank: count
        for bank, count in sorted(observed_counts.items())
        if bank not in SUPPORTED_OPERAND_BANKS
    }
    if not observed_banks:
        status = "no_operand_evidence"
        decision = "no_operand_evidence"
    elif confirmed_supported_counts.get("GPR", 0) == 0:
        if observed_counts.get("FPR", 0):
            status = "unsupported_fpr_evidence"
            decision = "unsupported_fpr_evidence"
        else:
            status = "no_confirmed_gpr_evidence"
            decision = "no_confirmed_gpr_evidence"
    else:
        status = "supported_gpr_evidence"
        decision = "analyze_gpr"
    return {
        "supported_operand_banks": list(SUPPORTED_OPERAND_BANKS),
        "observed_banks": observed_banks,
        "discarded_unsupported_bank_counts": discarded,
        "status": status,
        "decision": decision,
    }


def _collision_items(observations: list[dict[str, Any]], adjustments: dict[int, tuple[int, int]] | None = None) -> tuple[int, list[dict[str, Any]]]:
    groups: dict[int, dict[str, Any]] = {}
    for observation in observations:
        virtual_id = int(observation["virtual_id"])
        if adjustments:
            partition, start, delta = adjustments.get(observation["partition"], (-1, 0, 0))
            if partition == observation["partition"] and virtual_id >= start:
                virtual_id += delta
        group = groups.setdefault(
            virtual_id,
            {"target_colors": set(), "candidate_colors": set(), "observations": [], "observation_count": 0},
        )
        group["target_colors"].add(int(observation["target_color"]))
        group["candidate_colors"].add(int(observation["candidate_color"]))
        group["observation_count"] += 1
        if len(group["observations"]) < MAX_EXEMPLARS:
            group["observations"].append(observation)
        elif observation["target_color"] not in {x["target_color"] for x in group["observations"]}:
            group["observations"][-1] = observation
    collisions = []
    for virtual_id, group in groups.items():
        if len(group["target_colors"]) <= 1:
            continue
        collisions.append({
            "virtual_id": virtual_id,
            "target_colors": sorted(group["target_colors"]),
            "candidate_colors": sorted(group["candidate_colors"]),
            "observation_count": group["observation_count"],
            "exemplars": [
                {key: item.get(key) for key in ("machine_index", "row_index", "target_color", "candidate_color", "operand_ordinal", "opcode", "source_offset", "pcode_token")}
                for item in group["observations"]
            ],
        })
    collisions.sort(key=lambda item: item["virtual_id"])
    return len(collisions), collisions[:MAX_COLLISIONS]


def _reset(definitions: list[dict[str, int]], *, native: bool = False) -> dict[str, Any]:
    pool = [item for item in definitions if item["virtual_id"] >= POOL_BASE]
    drops = []
    for before, after in zip(pool, pool[1:]):
        if after["virtual_id"] < before["virtual_id"]:
            drops.append((before, after))
    if not drops:
        return {"status": "none", "detected": False, "pool_definition_count": len(pool)}
    if native:
        seen: set[int] = set()
        repeated = set()
        for item in pool:
            if item["virtual_id"] in seen:
                repeated.add(item["virtual_id"])
            seen.add(item["virtual_id"])
        return {
            "status": "UNKNOWN", "detected": False,
            "pool_definition_count": len(pool), "nonmonotonic_transitions": len(drops),
            "repeated_definition_ids": sorted(repeated)[:MAX_COLLISIONS],
            "repeated_definition_id_count": len(repeated),
            "transition_examples": [
                {"before": {key: before.get(key) for key in ("machine_index", "virtual_id", "source_offset")},
                 "after": {key: after.get(key) for key in ("machine_index", "virtual_id", "source_offset")}}
                for before, after in drops[:8]
            ],
            "reason": "ID drops/redefinitions do not prove allocator reset or distinct lifetimes",
            "alternatives": ["pool reuse", "existing local redefinition or scheduling"],
        }
    if len(drops) != 1:
        return {
            "status": "UNKNOWN",
            "detected": False,
            "pool_definition_count": len(pool),
            "candidate_count": len(drops),
        }
    before, after = drops[0]
    return {
        "status": "detected",
        "detected": True,
        "pool_definition_count": len(pool),
        "machine_index": after["machine_index"],
        "from_virtual_id": before["virtual_id"],
        "to_virtual_id": after["virtual_id"],
    }


def _suffix_fit(observations: list[dict[str, Any]], reset: dict[str, Any], baseline: int) -> dict[str, Any]:
    if baseline == 0:
        return {"status": "exact", "suppressed": True, "reason": "zero target-role collisions"}
    if reset.get("status") == "UNKNOWN":
        return {"status": "UNKNOWN", "reason": "pool reset boundary is unproved"}
    boundary = int(reset["machine_index"]) if reset.get("detected") else None
    partitions = sorted({0 if boundary is None or item["machine_index"] < boundary else 1 for item in observations})
    candidates: list[dict[str, int]] = []
    for partition in partitions:
        ids = sorted({item["virtual_id"] for item in observations if item["partition"] == partition and item["virtual_id"] >= POOL_BASE})
        for start in ids:
            for delta in OFFSET_DELTAS:
                count, _ = _collision_items(observations, {partition: (partition, start, delta)})
                if count == 0:
                    candidates.append({"partition": partition, "start_virtual_id": start, "delta": delta})
    if len(candidates) == 1:
        return {"status": "unique", "baseline_conflicts": baseline, "conflicts": 0, **candidates[0]}
    result: dict[str, Any] = {
        "status": "UNKNOWN",
        "baseline_conflicts": baseline,
        "zero_conflict_candidate_count": len(candidates),
    }
    if candidates:
        result["zero_conflict_candidates"] = candidates[:32]
        result["truncated"] = len(candidates) > 32
    return result


def _hypothetical_wrap_fits(observations: list[dict[str, Any]],
                           definitions: list[dict[str, Any]], baseline: int) -> dict[str, Any]:
    """Bounded +/-1 alignment diagnostics, never evidence of reset/target IDs.

    A drop of at least one physical-bank width and a fourfold ID decrease
    selects a *hypothetical* wrap boundary. Small local redefinitions are not
    boundaries. Independent binary cuts and separated epochs are scored without
    combining adjustments or applying anything to source/capture data.
    """
    pool = [x for x in definitions if x["virtual_id"] >= POOL_BASE]
    boundaries = [{"machine_index": after["machine_index"],
                   "source_offset": after.get("source_offset"),
                   "from_virtual_id": before["virtual_id"], "to_virtual_id": after["virtual_id"]}
                  for before, after in zip(pool, pool[1:])
                  if before["virtual_id"] - after["virtual_id"] >= POOL_BASE
                  and after["virtual_id"] * 4 <= before["virtual_id"]]
    result: dict[str, Any] = {
        "status": "hypothetical", "baseline_conflicts": baseline,
        "boundary_rule": "ID drop >= physical-bank width and at least fourfold decrease; heuristic only",
        "boundary_count": len(boundaries), "boundaries": boundaries[:MAX_WRAP_BOUNDARIES],
        "reset_authority": "UNKNOWN", "target_virtual_ids": "NOT_RECOVERED",
        "source_authority": "NOT_ESTABLISHED", "cells_evaluated": 0,
        "independent_wrap_fits": [], "epoch_fits": [],
    }
    if not boundaries or baseline == 0:
        result.update(status="suppressed", reason="no selected boundaries or no baseline conflicts")
        return result
    if len(boundaries) > MAX_WRAP_BOUNDARIES:
        result.update(status="UNKNOWN", reason="too many hypothetical boundaries; no fit attempted")
        return result

    def fit(selected: list[dict[str, Any]], partition: int,
            lower: int, upper: int | None) -> dict[str, Any]:
        ids = sorted({x["virtual_id"] for x in selected if x["partition"] == partition})
        best, matches, count = baseline, [], 0
        exhausted = False
        for start in ids:
            for delta in (-1, 1):
                if start + delta < POOL_BASE:
                    continue
                if result["cells_evaluated"] >= MAX_ALIGNMENT_CELLS:
                    exhausted = True
                    break
                result["cells_evaluated"] += 1
                score, _ = _collision_items(selected, {partition: (partition, start, delta)})
                if score >= baseline or score > best:
                    continue
                if score < best:
                    best, matches, count = score, [], 0
                count += 1
                if len(matches) < MAX_ALIGNMENT_BEST:
                    uses = [x for x in definitions if x["virtual_id"] == start
                            and x["machine_index"] >= lower
                            and (upper is None or x["machine_index"] < upper)]
                    basis = "observed_definition"
                    if not uses:
                        uses = [x for x in selected if x["virtual_id"] == start and x["partition"] == partition]
                        basis = "observed_operand_only"
                    first = min(uses, key=lambda x: x["machine_index"])
                    matches.append({"start_virtual_id": start, "delta": delta,
                                    "boundary_machine_index": first["machine_index"],
                                    "source_offset": first.get("source_offset"), "boundary_basis": basis})
            if exhausted:
                break
        return {"status": "UNKNOWN" if exhausted else "hypothetical",
                "machine_start": lower, "machine_end_exclusive": upper,
                "best_conflicts": best, "net_conflicts_removed": baseline - best,
                "best_hypothesis_count": count, "hypotheses": matches,
                "hypotheses_truncated": count > len(matches), "search_complete": not exhausted}

    cuts = [x["machine_index"] for x in boundaries]
    for cut in cuts:
        selected = [dict(x, partition=int(x["machine_index"] >= cut)) for x in observations]
        result["independent_wrap_fits"].append({
            "boundary_machine_index": cut,
            "before": fit(selected, 0, 0, cut), "after": fit(selected, 1, cut, None),
        })
    selected = [dict(x, partition=sum(x["machine_index"] >= cut for cut in cuts)) for x in observations]
    for epoch, lower in enumerate([0, *cuts]):
        upper = cuts[epoch] if epoch < len(cuts) else None
        result["epoch_fits"].append(fit(selected, epoch, lower, upper))
    if result["cells_evaluated"] >= MAX_ALIGNMENT_CELLS:
        result["status"] = "UNKNOWN"
    return result


def analyze(envelope_path: Path | str, report_path: Path | str, function: str) -> dict[str, Any]:
    """Analyze one function's confirmed GPR observations without mutation."""
    if not isinstance(function, str) or not function.strip():
        raise ValueError("function must be nonempty text")
    function = function.strip()
    envelope, envelope_binding = _read_json(Path(envelope_path), "envelope")
    report, report_binding = _read_json(Path(report_path), "report")
    native = isinstance(envelope, dict) and envelope.get("tool") == "mwcc_win32_varinfo"
    if native:
        events = _native_events(envelope, function)
    elif not isinstance(envelope, dict) or not isinstance(envelope.get("events"), list):
        raise ValueError("envelope.events must be a list")
    else:
        events = envelope["events"]
    if not all(isinstance(event, dict) for event in events):
        raise ValueError("envelope.events contains a non-object")
    left_symbols = _symbols(report, "left", "report")
    right_symbols = _symbols(report, "right", "report")
    left_symbol = _select_symbol(left_symbols, function, "target")
    right_symbol = _select_symbol(right_symbols, function, "candidate")
    _validate_pairing(left_symbols, right_symbols, left_symbol, right_symbol, function)
    left_rows = _instruction_rows(left_symbol, "target", function)
    right_rows = _instruction_rows(right_symbol, "candidate", function)
    machine = [event for event in events if event.get("event_kind") == "machine_emission" and event.get("function") == function]
    if not machine:
        raise ValueError(f"envelope has no machine emissions for {function!r}")
    coverage = _operand_coverage(events, function)
    word_check = _verify_machine_rows(right_symbol, right_rows, machine, function)
    captures = _captures(events, function)
    machine_by_token: dict[str, int] = {}
    for machine_index, event in enumerate(machine):
        token = event.get("pcode_token")
        if isinstance(token, str):
            # One PCode can expand to more than one machine word.  Definitions
            # belong to its first emitted word; later words still participate
            # in role mapping below, but must not make a false reset.
            machine_by_token.setdefault(token, machine_index)

    definitions: list[dict[str, int]] = []
    for token, machine_index in machine_by_token.items():
        for capture in captures.get(token, []):
            if capture["operand_flags"] & 2 and capture["virtual_id"] >= POOL_BASE:
                definitions.append({"machine_index": machine_index,
                                    "source_offset": machine[machine_index].get("source_offset"), **capture})
    definitions.sort(key=lambda item: (item["machine_index"], item["operand_ordinal"]))
    reset = _reset(definitions, native=native)
    reset_index = int(reset["machine_index"]) if reset.get("detected") else None
    observations: list[dict[str, Any]] = []
    candidate_index = 0
    for report_index, candidate_row in enumerate(right_rows):
        if candidate_row.get("instruction") is None:
            continue
        target_row = left_rows[report_index] if report_index < len(left_rows) else None
        target_text = _formatted(target_row)
        candidate_text = _formatted(candidate_row)
        machine_event = machine[candidate_index]
        candidate_index += 1
        token = machine_event.get("pcode_token")
        observed = (machine_event["native_gpr_operands"] if native else
                    sorted(captures.get(token, []), key=lambda item: item["operand_ordinal"]) if isinstance(token, str) else [])
        target_roles = _gpr_roles(target_text)
        candidate_roles = _gpr_roles(candidate_text)
        if (
            not observed
            or len(observed) != len(target_roles)
            or len(observed) != len(candidate_roles)
        ):
            continue
        if any(item["final_color"] != candidate_role for item, candidate_role in zip(observed, candidate_roles)):
            continue
        if native and (not target_text or target_text.split(None, 1)[0].lower() != candidate_text.split(None, 1)[0].lower()):
            continue
        partition = 0 if reset_index is None or int(machine_event["instruction_index"]) < reset_index else 1
        opcode = candidate_text.split(None, 1)[0].lower() if candidate_text else ""
        for item, target_role, candidate_role in zip(observed, target_roles, candidate_roles):
            if item["virtual_id"] is None:
                continue
            observations.append({
                "machine_index": int(machine_event["instruction_index"]),
                "row_index": report_index,
                "virtual_id": item["virtual_id"],
                "target_color": target_role,
                "candidate_color": candidate_role,
                "operand_ordinal": item["operand_ordinal"],
                "opcode": opcode,
                "source_offset": machine_event.get("source_offset"),
                "pcode_token": token,
                "partition": partition,
            })
    pooled = [item for item in observations if item["virtual_id"] >= POOL_BASE]
    baseline_count, collision_rows = _collision_items(pooled)
    if coverage["status"] == "supported_gpr_evidence":
        fit = _suffix_fit(pooled, reset, baseline_count)
    else:
        fit = {
            "status": coverage["status"],
            "decision": coverage["decision"],
            "reason": "GPR pool-fit analysis requires confirmed GPR operand evidence",
        }
    result: dict[str, Any] = {
        "schema": SCHEMA,
        "schema_version": 1,
        "function": function,
        "status": coverage["status"],
        "decision": coverage["decision"],
        "inputs": {"envelope": envelope_binding, "report": report_binding},
        "envelope_sha256": envelope_binding["sha256"],
        "report_sha256": report_binding["sha256"],
        "machine_word_verification": word_check,
        "coverage": coverage,
        "observations": {
            "confirmed_gpr_roles": len(observations),
            "pooled_roles": len(pooled),
            "pooled_definitions": len(definitions),
        },
        "reset": reset,
        "collisions": {
            "count": baseline_count,
            "items": collision_rows,
            "truncated": baseline_count > MAX_COLLISIONS,
        },
        "suffix_fit": fit,
        "diagnostic_only": True,
        "input_format": "native" if native else "capsule_events",
        "authority_advanced": False,
        "target_virtual_ids": "NOT_RECOVERED",
        "source_authority": "NOT_ESTABLISHED; compiler source offsets are location evidence only",
    }
    if native:
        result["hypothetical_alignment"] = _hypothetical_wrap_fits(pooled, definitions, baseline_count)
    encoded = canonical(result)
    if len(encoded) > MAX_OUTPUT_BYTES:
        raise ValueError(f"diagnostic result exceeds {MAX_OUTPUT_BYTES} bytes")
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--envelope", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--function", required=True)
    args = parser.parse_args(argv)
    try:
        result = analyze(args.envelope, args.report, args.function)
        print(canonical(result).decode("utf-8"))
    except (OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
