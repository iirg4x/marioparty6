#!/usr/bin/env python3
"""Group observed machine residuals; never infer source causes or promotion proof."""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import re
from pathlib import Path
import sys
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import recovery_frontier as frontier


def ranking_tuple(summary: dict, *, exact_function_count: int = 0,
                  protected_loss_count: int = 0, score: float = 0.0) -> tuple:
    """Lower is better. Diagnostic ordering, never a retention/proof gate."""
    return (protected_loss_count, -exact_function_count,
            summary["structural_hazard_count"], summary["unresolved_row_count"],
            len(summary["groups"]), -score)


def numeric_domain_evidence(left: list[dict], right: list[dict]) -> dict:
    """Observe evaluation-domain differences, not source types or rewrite proof.

    GC1.2.5n FFT exposed why register/row-count triage alone is insufficient:
    signed length arithmetic and promoted double literals changed unrolling and
    a large downstream register schedule in both functions of one owner.
    """
    pairs = {("cmplw", "cmpw"): ("integer", "unsigned"),
             ("cmplwi", "cmpwi"): ("integer", "unsigned"),
             ("srwi", "srawi"): ("integer", "unsigned"),
             ("fmuls", "fmul"): ("arithmetic", "single"),
             ("fadds", "fadd"): ("arithmetic", "single"),
             ("fsubs", "fsub"): ("arithmetic", "single"),
             ("fdivs", "fdiv"): ("arithmetic", "single"),
             ("lfs", "lfd"): ("load", "single")}
    reverse = {"unsigned": "signed", "single": "double"}
    pairs.update({(b, a): (family, reverse[direction])
                  for (a, b), (family, direction) in list(pairs.items())})
    buckets: dict[tuple, list[dict]] = {}
    for index, (target, candidate) in enumerate(zip(left, right)):
        if not isinstance(target, dict) or not isinstance(candidate, dict):
            continue
        if any(not isinstance(row.get("instruction"), dict)
               or not isinstance(row["instruction"].get("formatted"), str)
               for row in (target, candidate)):
            continue
        if any(row.get("diff_kind") in {"DIFF_INSERT", "DIFF_DELETE"}
               for row in (target, candidate)):
            continue
        parts = [frontier._instruction_parts(row) for row in (target, candidate)]
        if not all(parts) or (parts[0][0], parts[1][0]) not in pairs:
            continue
        # Malformed mnemonic-only rows are not evidence; operands may legitimately
        # be different physical registers due to the domain-induced cascade.
        if not all(any(operand.strip() for operand in part[1]) for part in parts):
            continue
        family, direction = pairs[(parts[0][0], parts[1][0])]
        buckets.setdefault((family, direction), []).append({
            "row": index, "target": target["instruction"]["formatted"],
            "candidate": candidate["instruction"]["formatted"]})
    signals = []
    for (family, direction), sites in sorted(buckets.items()):
        category = "integer" if family == "integer" else "floating"
        mixed = any(("integer" if other == "integer" else "floating") == category
                    and other_direction != direction for other, other_direction in buckets)
        signals.append({"family": family, "target_domain": direction,
                        "row_count": len(sites), "sites": sites[:12],
                        "sites_truncated": len(sites) > 12,
                        "mixed_direction": mixed,
                        "review_priority": "support_only" if family == "load" or mixed else "source_domain",
                        "source_review": (
                            "Inspect length/index declarations, shifts/divisions and caller domains."
                            if category == "integer" else
                            "Inspect literal suffixes, casts, operand types and public prototypes; load width alone is insufficient."),
                        "cause_proven": False})
    return {"signals": signals, "source_type_identity": "UNKNOWN",
            "next_action": "Review the indicated expressions and consumers together before local register permutations; compile one coherent candidate, not blanket type changes.",
            "authority_advanced": False}


def _score_census_duplicates(document: dict, sides: list[list[dict]]) -> dict:
    """Classify duplicate names without merging symbols or transferring scores."""
    duplicates = {}
    for side, symbols in zip(("left", "right"), sides):
        code_sections = [s for s in document[side].get("sections", [])
                         if s.get("kind") == "SECTION_CODE"]
        by_name = {}
        for index, symbol in enumerate(symbols):
            if symbol.get("instructions"):
                by_name.setdefault(symbol["name"], []).append((index, symbol))
        for name, entries in by_name.items():
            if len(entries) < 2:
                continue
            extents = []
            for index, symbol in entries:
                instructions = [r["instruction"] for r in symbol["instructions"]
                                if r.get("instruction")]
                address = frontier.focus._integer(symbol.get("address"))
                if address is None and instructions:
                    address = frontier.focus._integer(instructions[0].get("address"))
                size = frontier.focus._integer(symbol.get("size"))
                if address is None or address < 0 or size is None or size <= 0:
                    raise ValueError(f"ambiguous duplicate function extent: {side}.{name}")
                extents.append((address, size))
            same_extent = len(set(extents)) == 1
            disjoint = all(a + size <= b or b + other_size <= a
                           for i, (a, size) in enumerate(extents)
                           for b, other_size in extents[i + 1:])
            # A shared address in an unspecified/multi-code-section object is
            # not sufficient alias evidence. Even confirmed aliases stay as
            # separate symbol entries, including their null scores/bindings.
            same_instructions = all(s["instructions"] == entries[0][1]["instructions"]
                                    for _, s in entries[1:])
            if same_extent and len(code_sections) == 1 and same_instructions:
                classification = "same_extent_aliases"
            elif disjoint:
                classification = "separate_extents"
            else:
                raise ValueError(f"ambiguous duplicate function extents or conflicting alias evidence: {side}.{name}")
            duplicates.setdefault(side, {})[name] = {
                "classification": classification,
                "members": [{"identity": f"{name}@{side}.symbols[{index}]",
                             "symbol_index": index, "address": address, "bytes": size}
                            for (index, _), (address, size) in zip(entries, extents)]}
    return duplicates


def summarize_match_scores(document: dict) -> dict:
    """Function-symbol score census; null/alias scores never imply exactness.

    This lightweight path intentionally requires no disassembly interpretation
    and performs no file writes, builds, or repair of a report's bindings.
    Unique-name keys/rows retain the legacy API. Names duplicated on either side
    use report-local symbol-table identities and reciprocal objdiff pair indices,
    never a name/extent guess. Alias entries are classified, not collapsed.
    ``score_exact`` is only the reported score, not instruction/physical proof.
    """
    sides = [frontier.focus._symbols(document, side, "strict") for side in ("left", "right")]
    duplicates = _score_census_duplicates(document, sides)
    duplicate_names = {name for names in duplicates.values() for name in names}
    for side_index, symbols in enumerate(sides):
        other = sides[1 - side_index]
        for index, symbol in enumerate(symbols):
            pair = symbol.get("target_symbol")
            if not symbol.get("instructions") or symbol["name"] not in duplicate_names or pair is None:
                continue
            if (type(pair) is not int or not 0 <= pair < len(other)
                    or not other[pair].get("instructions")
                    or type(other[pair].get("target_symbol")) is not int
                    or other[pair]["target_symbol"] != index):
                side = ("left", "right")[side_index]
                raise ValueError(f"ambiguous duplicate function pairing: {side}.{symbol['name']} at symbol {index}")
    right = {s["name"]: (i, s) for i, s in enumerate(sides[1])
             if s.get("instructions") and s["name"] not in duplicate_names}
    def identity(side, index, symbol):
        name = symbol["name"]
        return f"{name}@{side}.symbols[{index}]" if name in duplicate_names else name

    scores = {}
    paired = set()
    for index, symbol in enumerate(sides[0]):
        if not symbol.get("instructions"):
            continue
        name = symbol["name"]
        candidate_index, candidate = right.get(name, (None, None))
        if name in duplicate_names and symbol.get("target_symbol") is not None:
            candidate_index = symbol["target_symbol"]
            candidate = sides[1][candidate_index]
        if candidate is not None:
            if candidate_index in paired:
                raise ValueError(f"ambiguous duplicate candidate binding: right symbol {candidate_index}")
            paired.add(candidate_index)
        score = symbol.get("match_percent")
        status = ("unpaired_target" if candidate is None and name in duplicate_names else
                  "missing_candidate" if candidate is None else
                  "unscored" if score is None else "score_exact" if score == 100 else "mismatch")
        key = identity("left", index, symbol)
        if key in scores:
            raise ValueError(f"ambiguous duplicate census identity: {key}")
        scores[key] = {"target_bytes": symbol.get("size"),
                       "candidate_bytes": candidate.get("size") if candidate else None,
                       "score": score, "status": status}
        if name in duplicate_names:
            scores[key].update(function=name, target_symbol_index=index,
                               candidate_symbol_index=candidate_index,
                               candidate_function=candidate["name"] if candidate else None,
                               pairing="reciprocal_report_indices" if candidate else "unpaired")
    candidate_only = {}
    for index, symbol in enumerate(sides[1]):
        if not symbol.get("instructions") or index in paired:
            continue
        key = identity("right", index, symbol)
        if key in candidate_only:
            raise ValueError(f"ambiguous duplicate census identity: {key}")
        candidate_only[key] = {"candidate_bytes": symbol.get("size"),
                               "score": symbol.get("match_percent"), "status": "missing_target"}
        if symbol["name"] in duplicate_names:
            candidate_only[key].update(function=symbol["name"], candidate_symbol_index=index)
    result = {"functions": len(scores),
            "exact": sum(s["status"] == "score_exact" for s in scores.values()),
            "function_scores": scores,
            "residuals": {name: s for name, s in scores.items() if s["status"] != "score_exact"},
            "candidate_only": candidate_only}
    if duplicates:
        result.update(duplicate_names=duplicates, census_unit="function_symbols",
                      identity_scope="indices in the supplied report's unfiltered symbol tables; not cross-report identities")
    return result


def _prologue_frame(symbol: dict | None) -> dict:
    """Observe one immediate stwu frame in a bounded ABI prologue, not alloca.

    No setup is unknown, not a zero-byte frame. Stop at the first body/control
    instruction; competing stack definitions in the prologue are ambiguous.
    """
    unknown = {"bytes": None, "status": "unknown"}
    if not symbol:
        return dict(unknown, reason="missing symbol")
    setups = []
    for index, row in enumerate(symbol.get("instructions", [])[:32]):
        if (isinstance(row, dict) and row.get("instruction") is None
                and row.get("diff_kind") in {"DIFF_INSERT", "DIFF_DELETE"}):
            continue  # objdiff alignment gap, not a machine instruction
        parts = frontier._instruction_parts(row)
        if parts is None:
            return dict(unknown, reason="unrecognized prologue instruction")
        opcode, operands = parts
        if opcode == "stwu":
            offset = frontier._stack_operand_offset(operands[1]) if len(operands) == 2 else None
            if len(operands) != 2 or operands[0] != "r1" or offset is None or not -32768 <= offset < 0:
                return dict(unknown, reason="unrecognized frame setup")
            setups.append({"bytes": -offset, "row": index,
                           "instruction": f"stwu r1,{offset}(r1)"})
            if len(setups) > 1:
                return dict(unknown, reason="ambiguous multiple frame setups")
        elif (operands and operands[0] == "r1" and opcode not in {"stw", "stmw", "cmpw", "cmplw", "cmpwi", "cmplwi"}) or opcode == "stwux":
            return dict(unknown, reason="unrecognized or competing stack definition")
        elif opcode not in {"mflr", "stw", "stmw", "stfd"}:
            break
    else:
        if len(symbol.get("instructions", [])) > 32:
            return dict(unknown, reason="prologue scan limit reached")
    return dict(setups[0], status="observed") if setups else dict(unknown, reason="no recognized prologue setup")


def compare_match_frontiers(before: dict, after: dict) -> dict:
    """Compare scores, sizes and observed frames; never retention/closure proof.

    Unique function names bridge reports. Report-local duplicate indices cannot.
    Unknown scores never become numerical gains, including a new 100 score.
    Detail lists are bounded; counts always cover the entire target census.
    """
    old, new = (summarize_match_scores(document) for document in (before, after))
    if old.get("duplicate_names") or new.get("duplicate_names"):
        raise ValueError("ambiguous cross-report duplicate function identities; report-local indices cannot bridge reports")
    left, right = old["function_scores"], new["function_scores"]
    if left.keys() != right.keys():
        raise ValueError("incompatible target function set")
    buckets = {key: [] for key in ("new_score_exact", "lost_score_exact", "improved",
                                   "regressed", "changed_size", "unscored_changes", "frame_transitions")}
    unchanged = score_unchanged = unscored = closed_size_regressions = 0
    closed_frame_regressions = unknown_frames = 0
    symbols = [{side: {s["name"]: s for s in frontier.focus._symbols(document, side, "strict")
                       if s.get("instructions")}
                for side in ("left", "right")} for document in (before, after)]
    for name in sorted(left):
        a, b = left[name], right[name]
        sizes = [frontier.focus._integer(row["target_bytes"]) for row in (a, b)]
        if None in sizes or sizes[0] != sizes[1]:
            raise ValueError(f"incompatible or unavailable target size: {name}")
        row = {"function": name, "before_score": a["score"], "after_score": b["score"],
               "target_bytes": sizes[0], "before_bytes": a["candidate_bytes"],
               "after_bytes": b["candidate_bytes"]}
        frames = {label: _prologue_frame(symbols[report][side].get(name))
                  for label, report, side in (("target", 0, "left"), ("after_target", 1, "left"),
                                              ("baseline", 0, "right"), ("candidate", 1, "right"))}
        target, after_target, baseline, candidate = (frames[k]["bytes"] for k in
                                                     ("target", "after_target", "baseline", "candidate"))
        frame_known = None not in (target, after_target, baseline, candidate) and target == after_target
        frame_regression = baseline == target and candidate != target if frame_known else None
        transition = ("closed_frame_regression" if frame_regression else
                      "exact_frame_preserved" if baseline == candidate == target else
                      "frame_closed" if candidate == target else
                      "unchanged_open_frame" if baseline == candidate else "changed_open_frame") if frame_known else "unknown"
        row.update(target_frame_bytes=target, baseline_frame_bytes=baseline,
                   candidate_frame_bytes=candidate, frame_transition=transition,
                   closed_frame_regression=frame_regression)
        # Row positions and decimal/hex spellings are not frame transitions.
        frame_keys = {label: (fact["bytes"], fact["status"], fact.get("reason"))
                      for label, fact in frames.items()}
        frame_changed = (frame_keys["target"] != frame_keys["after_target"]
                         or frame_keys["baseline"] != frame_keys["candidate"])
        if a != b or frame_changed:
            buckets["frame_transitions"].append(dict(row, frame_facts=frames))
        closed_frame_regressions += int(frame_regression is True)
        unknown_frames += int(not frame_known)
        known = all(item["status"] in {"score_exact", "mismatch"}
                    and type(item["score"]) in (int, float)
                    and 0 <= item["score"] <= 100 for item in (a, b))
        if known:
            if b["score"] > a["score"]:
                buckets["improved"].append(row)
                if b["score"] == 100:
                    buckets["new_score_exact"].append(row)
            elif b["score"] < a["score"]:
                buckets["regressed"].append(row)
            else:
                score_unchanged += 1
        else:
            unscored += 1
            if (a["score"], a["status"]) != (b["score"], b["status"]):
                buckets["unscored_changes"].append(dict(row, before_status=a["status"], after_status=b["status"]))
        if a["status"] == "score_exact" and b["status"] != "score_exact":
            buckets["lost_score_exact"].append(row)
        if a["candidate_bytes"] != b["candidate_bytes"]:
            was_closed = frontier.focus._integer(a["candidate_bytes"]) == sizes[0]
            now_closed = frontier.focus._integer(b["candidate_bytes"]) == sizes[0]
            regression = was_closed and not now_closed
            closed_size_regressions += int(regression)
            buckets["changed_size"].append(dict(row, closed_size_regression=regression))
        if a == b and not frame_changed:
            unchanged += 1
    return {"schema": "recovery_match_frontier_transition/v1", "functions": len(left),
            **{key: rows[:24] for key, rows in buckets.items()},
            "counts": {**{key: len(rows) for key, rows in buckets.items()},
                       "unchanged": unchanged, "score_unchanged": score_unchanged,
                       "unscored_comparisons": unscored,
                       "closed_size_regressions": closed_size_regressions,
                       "closed_frame_regressions": closed_frame_regressions,
                       "frame_comparisons": len(left),
                       "unknown_frame_comparisons": unknown_frames},
            "details_limit": 24, "details_truncated": any(len(rows) > 24 for rows in buckets.values()),
            "mixed_gain_regression": bool(buckets["improved"] and (
                buckets["regressed"] or buckets["lost_score_exact"] or closed_size_regressions or closed_frame_regressions)),
            "comparison_scope": "reported function scores, sizes and bounded prologue frame observations; unknowns are not gains; no retention or closure proof",
            "authority_advanced": False}


def summarize_owner(document: dict) -> dict:
    """Small whole-object view: count closures and surface shared domain clues."""
    symbols = frontier.focus._symbols(document, "left", "strict")
    candidate_symbols = frontier.focus._symbols(document, "right", "strict")
    functions = [s for s in symbols if s.get("instructions")]
    residuals, exact = [], []
    for symbol in functions:
        if frontier._stack_function(candidate_symbols, symbol["name"], "candidate") is None:
            residuals.append({"function": symbol["name"], "target_bytes": symbol.get("size"),
                              "score": None, "residual_rows": len(symbol["instructions"]),
                              "size_exact": False, "domain_signals": [], "domain_review_first": False,
                              "status": "candidate_symbol_missing",
                              "next_action": "Reconstruct the missing function; no matched instruction pair exists for domain diagnosis."})
            continue
        summary = summarize_groups(document, symbol["name"])
        if summary["instruction_exact"]:
            exact.append(symbol["name"])
            continue
        signals = summary["numeric_domain_evidence"]["signals"]
        copies = summary["constant_copy_evidence"]["signals"]
        residuals.append({"function": symbol["name"],
                          "target_bytes": symbol.get("size"),
                          "score": symbol.get("match_percent"),
                          "residual_rows": summary["coverage"]["residual_rows"],
                          "size_exact": summary["size_exact"],
                          "domain_signals": signals,
                          "constant_copy_signals": copies,
                          "domain_review_first": any(s["review_priority"] == "source_domain" for s in signals)})
    residuals.sort(key=lambda r: (not (r["domain_review_first"] or r.get("constant_copy_signals")), r["residual_rows"], r["function"]))
    return {"schema": "recovery_owner_triage/v1", "functions_total": len(functions),
            "instruction_exact_count": len(exact), "remaining_count": len(residuals),
            "instruction_exact_functions": exact, "residuals": residuals,
            "ordering": "domain/constant-copy review evidence before opaque register tails, then residual rows; advisory, not expected time or proof",
            "owner_closed": False, "physical_proof": False, "authority_advanced": False}


def support_packet(document: dict, function: str, source: str, start: int, end: int,
                   *, radius: int = 4, max_bytes: int = 24000) -> dict:
    """Bound one support question without reducing local-model reasoning limits.

    Keep the original TU/object for compilation. This is an analysis excerpt,
    with complete omitted-row counts and bindings, never a synthetic compiler TU.
    """
    lines = source.splitlines()
    if not (1 <= start <= end <= len(lines)) or not 0 <= radius <= 16:
        raise ValueError("invalid source range or row radius")
    if not 1000 <= max_bytes <= 64000:
        raise ValueError("support byte budget must be 1000..64000")
    summary = summarize_groups(document, function)
    streams = [frontier.focus._rows(frontier._stack_function(
        frontier.focus._symbols(document, side, "strict"), function, side), function)
        for side in ("left", "right")]
    sites = sorted({m["row"] for g in summary["groups"] for m in g["members"]})
    # An early branch displacement/register cycle can hide the actual extra
    # load much later (Window's first-frame memcpy). Include the first concrete
    # added/deleted operation of each kind, not just the first differing row.
    structural = {}
    for group in summary["groups"]:
        if group["kind"] in {"added_instruction", "deleted_instruction", "added_move", "deleted_move"}:
            structural[group["kind"]] = min(structural.get(group["kind"], group["first"]["row"]), group["first"]["row"])
    centers = sites[:1] + [s["sites"][0]["row"] for s in summary["numeric_domain_evidence"]["signals"]]
    centers += list(structural.values())
    selected = sorted({i for center in centers for i in range(max(0, center-radius),
                      min(max(map(len, streams)), center+radius+1))})
    rows = [{"row": i, **{side: (stream[i].get("instruction") or {}).get("formatted")
                          if i < len(stream) else None
                          for side, stream in zip(("target", "candidate"), streams)}} for i in selected]
    result = {"schema": "recovery_support_excerpt/v1", "function": function,
              "source_sha256": hashlib.sha256(source.encode("utf-8")).hexdigest(),
              "report_sha256": hashlib.sha256(json.dumps(document, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
              "report_hash_scope": "canonical parsed JSON", "source_lines": [start, end],
              "source_excerpt": "\n".join(lines[start-1:end]), "paired_rows": rows,
              "structural_context_anchors": structural,
              "category_rows": summary["category_rows"], "size_exact": summary["size_exact"],
              "residual_row_count": len(sites), "omitted_residual_row_count": len(set(sites)-set(selected)),
              "numeric_domain_evidence": summary["numeric_domain_evidence"],
              "scope": "one source decision; excerpt is incomplete, request a named missing span if needed; no filesystem or compiler access",
              "authority_advanced": False}
    size = len(json.dumps(result, ensure_ascii=False).encode("utf-8"))
    if size > max_bytes:
        raise ValueError(f"support excerpt {size} bytes exceeds {max_bytes}; narrow the source range/question, do not dispatch a full TU")
    return result


def _digest(value: Any) -> str:
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":"),
                                    ensure_ascii=False).encode("utf-8")).hexdigest()


def _target_only_branch_facts(packet: dict) -> list[dict]:
    """Resolve only reported addresses; no case meanings or CFG inference."""
    addresses = packet.get("target_instruction_addresses")
    if (not isinstance(addresses, list) or not addresses
            or any(a is not None and (type(a) is not int or a < 0) for a in addresses)
            or len([a for a in addresses if a is not None]) != len({a for a in addresses if a is not None})):
        raise ValueError("invalid target instruction addresses")
    selected = set()
    for row in packet["paired_rows"]:
        index = row.get("row")
        if (type(index) is not int or not 0 <= index < len(addresses) or index in selected
                or "target_address" not in row or row["target_address"] != addresses[index]):
            raise ValueError("target row/address binding differs")
        selected.add(index)
    by_address = {address: i for i, address in enumerate(addresses) if address is not None}
    included_text = {row["row"]: row["target"] for row in packet["paired_rows"]}
    facts = []
    for site in packet["machine_census"]["target"]["branches_including_return"]:
        if (type(site.get("row")) is not int or not 0 <= site["row"] < len(addresses)
                or (site["row"] in included_text and site["instruction"] != included_text[site["row"]])):
            raise ValueError("target branch census differs from supplied rows")
        branch = frontier._branch_instruction(site["instruction"], site["row"])
        if branch is None:
            continue
        destination = branch["destination"]
        index = by_address.get(destination)
        status = ("unresolved" if destination is None else
                  "included" if index in selected else "omitted" if index is not None else
                  "external" if all(a is not None for a in addresses) else "external_or_unreported")
        facts.append({"row": site["row"], "destination_address": destination,
                      "destination_row": index if status == "included" else None, "status": status})
    return facts


def _known_measurement_context(entries: list[dict], function: str) -> dict:
    """Copy bounded caller-selected observations, never authenticate compiler proof.

    Descriptors identify supplied historical artifacts; this pure packet builder
    neither opens them nor discovers history. Their contents/context still need
    caller verification. In particular, missing compile bindings stay missing.
    """
    if not isinstance(entries, list) or not 1 <= len(entries) <= 8:
        raise ValueError("known measurements require 1..8 selected entries")
    copied = []
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {
                "function", "source_fragment", "observed_result", "evidence"}:
            raise ValueError("invalid known measurement fields")
        if entry["function"] != function:
            raise ValueError("known measurement function differs from decision function")
        for field, limit in (("function", 256), ("source_fragment", 2048), ("observed_result", 1000)):
            value = entry[field]
            if not isinstance(value, str) or not value.strip() or len(value.encode("utf-8")) > limit:
                raise ValueError(f"invalid known measurement {field}: expected 1..{limit} UTF-8 bytes")
        evidence = entry["evidence"]
        if not isinstance(evidence, list) or not 1 <= len(evidence) <= 4:
            raise ValueError("known measurement requires 1..4 supplied evidence descriptors")
        descriptors = []
        for descriptor in evidence:
            if (not isinstance(descriptor, dict) or not {"path", "sha256"} <= set(descriptor)
                    or set(descriptor) - {"path", "sha256", "size_bytes"}
                    or not isinstance(descriptor["path"], str) or not descriptor["path"].strip()
                    or "\x00" in descriptor["path"] or len(descriptor["path"].encode("utf-8")) > 1024
                    or not isinstance(descriptor["sha256"], str)
                    or re.fullmatch(r"[0-9a-f]{64}", descriptor["sha256"]) is None
                    or ("size_bytes" in descriptor and (type(descriptor["size_bytes"]) is not int
                        or not 0 <= descriptor["size_bytes"] <= 2**63 - 1))):
                raise ValueError("invalid known measurement evidence descriptor")
            descriptors.append(dict(descriptor))
        copied.append({**entry, "evidence": descriptors})
    context = {"entries": copied, "diagnostic_only": True,
               "compiler_proof_reusable": False, "suppress_compile": False}
    if len(json.dumps(context, ensure_ascii=False).encode("utf-8")) > 8192:
        raise ValueError("known measurements exceed 8192-byte context budget")
    return context


def _comparison_direction(paired_rows: list[dict]) -> dict:
    """Bounded alignment facts, never source causes or instruction edit commands."""
    facts = []
    differences = 0
    for row in paired_rows:
        if (not isinstance(row, dict) or type(row.get("row")) is not int
                or row["row"] < 0 or any(side not in row or
                    (row[side] is not None and not isinstance(row[side], str))
                    for side in ("target", "candidate"))):
            raise ValueError("invalid comparison direction paired row")
        target, candidate = row["target"], row["candidate"]
        kind = ("missing_from_candidate" if target is not None and candidate is None else
                "extra_in_candidate" if target is None and candidate is not None else
                "empty_alignment" if target is None else
                "paired_same_text" if target == candidate else "paired_difference")
        if kind in {"empty_alignment", "paired_same_text"}:
            continue
        differences += 1
        if len(facts) < 64:
            facts.append({"row": row["row"], "observation": kind})
    return {"target_role": "immutable_reference", "candidate_role": "editable_C",
            "rows": facts, "selected_row_count": len(paired_rows),
            "different_alignment_count": differences,
            "row_limit": 64, "rows_truncated": differences > 64,
            "scope": "alignment observations only; not semantic equivalence, source causes or edit commands"}


def decision_packet(document: dict, function: str, source: str, start: int, end: int,
                    question: str, row_start: int, row_end: int, *, max_bytes: int = 18000,
                    producer_sites: list[int] | None = None, producer_limit: int = 16,
                    decision_mode: str = "fact", allow_missing_candidate: bool = False,
                    known_measurements: list[dict] | None = None) -> dict:
    """One bounded support decision, not an open-ended function rewrite.

    Include the complete function's call/branch census even when arithmetic is
    excerpted. Matrix's unchanged straight-line memcpy was previously replaced
    by an invented loop after a worker saw only selected mismatching rows.
    Reasoning/output limits are deliberately not part of this interface.
    Optional producer_sites are explicit selected-row questions for the existing
    block-local slicer; their definitions may precede row_start. The shared byte
    cap includes this context, and omitted context leaves legacy packets intact.
    decision_mode='source-hypothesis' explicitly permits a single uncertain
    natural-C cause proposal. Default fact packets and prompts remain unchanged.
    allow_missing_candidate permits target-only factual evidence with supplied
    source context, never a fabricated candidate body or hypothesis replacement.
    Optional known_measurements supplies at most eight selected historical
    observations outside the question limit but inside the packet hash/budget.
    They are advisory source constraints, not reusable compiler proof or bans.
    """
    if decision_mode not in {"fact", "source-hypothesis"}:
        raise ValueError("unknown decision mode")
    if type(allow_missing_candidate) is not bool:
        raise ValueError("allow_missing_candidate must be boolean")
    lines = source.splitlines()
    if not isinstance(question, str) or not question.strip() or len(question) > 800:
        raise ValueError("one concrete decision question is required (1..800 characters)")
    if not 1 <= start <= end <= len(lines) or not 1000 <= max_bytes <= 64000:
        raise ValueError("invalid decision source range or byte budget")
    symbols = [frontier._stack_function(frontier.focus._symbols(document, side, "strict"),
                                       function, side) for side in ("left", "right")]
    candidate_missing = symbols[1] is None
    if symbols[0] is None or (candidate_missing and not allow_missing_candidate):
        raise ValueError("decision function missing")
    if candidate_missing and decision_mode != "fact":
        raise ValueError("target-only decisions require fact mode; no candidate body exists")
    streams = [frontier.focus._rows(symbol, function) if symbol is not None else [] for symbol in symbols]
    if not 0 <= row_start <= row_end < max(map(len, streams)):
        raise ValueError("invalid inclusive decision machine range")
    if producer_sites is not None and (
            not isinstance(producer_sites, list) or not producer_sites
            or any(type(i) is not int or not row_start <= i <= row_end for i in producer_sites)
            or len(set(producer_sites)) != len(producer_sites)
            or type(producer_limit) is not int or not 1 <= producer_limit <= 64):
        raise ValueError("producer sites must be unique selected row IDs; limit must be 1..64")
    census = {}
    for side, symbol, stream in zip(("target", "candidate"), symbols, streams):
        if symbol is None:
            census[side] = {"bytes": None, "instruction_count": None, "calls": [],
                            "branches_including_return": [], "status": "unavailable"}
            continue
        calls, branches = [], []
        for index, row in enumerate(stream):
            parts = frontier._instruction_parts(row)
            if not parts:
                continue
            op = parts[0]
            site = {"row": index, "instruction": row["instruction"]["formatted"]}
            if op in {"bl", "bla", "bctrl", "blrl"}:
                calls.append(site)
            elif op.startswith("b"):
                branches.append(site)
        census[side] = {"bytes": symbol.get("size"),
                        "instruction_count": sum(bool(row.get("instruction")) for row in stream),
                        "calls": calls, "branches_including_return": branches}
    result = {"schema": "recovery_support_decision/v1", "function": function,
              "question": question.strip(), "source_sha256": hashlib.sha256(source.encode()).hexdigest(),
              "report_sha256": _digest(document), "report_hash_scope": "canonical parsed JSON",
              "source_lines": [start, end], "source_excerpt": "\n".join(lines[start-1:end]),
              "machine_census": census,
              "paired_rows": [{"row": i, **{side: (stream[i].get("instruction") or {}).get("formatted")
                                            if i < len(stream) else None
                                            for side, stream in zip(("target", "candidate"), streams)}}
                              for i in range(row_start, row_end+1)],
              "omitted_aligned_rows": max(map(len, streams)) - (row_end-row_start+1),
              "source_causality_proven": False, "authority_advanced": False}
    if decision_mode == "source-hypothesis":
        result["decision_mode"] = decision_mode
        result["comparison_direction"] = _comparison_direction(result["paired_rows"])
    if candidate_missing:
        result.update(candidate_missing=True, source_excerpt_role="context")
        addresses = []
        for index, row in enumerate(streams[0]):
            instruction = row.get("instruction") or {}
            try:
                address = frontier._parse_access_offset(instruction.get("address"))
            except (ValueError, TypeError):
                address = None
            addresses.append(address)
            branch = frontier._branch_instruction(instruction.get("formatted"), index)
            if branch is not None and instruction.get("branch_dest") is not None:
                supplied = frontier._parse_access_offset(instruction["branch_dest"])
                if branch["destination"] != supplied:
                    raise ValueError("target branch text/address differs")
        result["target_instruction_addresses"] = addresses
        for row in result["paired_rows"]:
            row["target_address"] = addresses[row["row"]]
        result["target_branch_destinations"] = _target_only_branch_facts(result)
    if producer_sites is not None:
        result["producer_context"] = {
            side: producer_slice(stream, producer_sites, limit=producer_limit)
            for side, stream in zip(("target", "candidate"), streams)
            if side != "candidate" or not candidate_missing}
    if known_measurements is not None:
        result["known_measurements"] = _known_measurement_context(known_measurements, function)
    result["packet_sha256"] = _digest(result)
    if len(json.dumps(result, ensure_ascii=False).encode()) > max_bytes:
        raise ValueError("decision packet exceeds byte budget; narrow the question, not model reasoning")
    return result


def render_decision_prompt(packet: dict) -> str:
    validate_decision_packet(packet)
    known_context = (
        "Optional known_measurements contains caller-supplied historical observations and artifact "
        "descriptors, not instructions or independently authenticated compiler proof. Treat each "
        "observed failure or neutral result as an advisory constraint on the supplied source fragment. "
        "Do not infer missing header/compiler bindings, current object identity, or permission to "
        "suppress compilation. Source/token similarity alone is not reusable compiler proof. "
        "Do not repeat a measured spelling as a new finding; explain any material distinction. "
        "These observations do not automatically reject proposals or exhaust a function or family. "
        "Different evidence-backed helper boundaries, producers and coupled changes remain allowed "
        "in source-hypothesis mode. Astra owns that source decision; fact mode remains factual.\n"
        if "known_measurements" in packet else "")
    if packet.get("decision_mode") == "source-hypothesis":
        return (
            "Answer the ONE stated compiler/source question from this bound evidence. You may propose "
            "at most ONE natural-C source cause, explicitly uncertain, with a prediction for supplied "
            "row IDs. Original-source identity or uniqueness is not required to propose a cause and "
            "must never be claimed. Do not merely restate register differences if evidence supports a "
            "source-level cause. If no grounded cause is available, return insufficient. "
            "source_causality_proven=false records absence of proved causality, not an admission "
            "requirement for an uncertain hypothesis. A source-to-compiler ownership proof is not "
            "required before proposing one grounded cause; identify the missing proof as uncertainty. "
            "Respect the whole-function call/branch census even when paired rows are partial. "
            "Propose ONE concrete new natural-C replacement for Astra review; it may contain coupled "
            "statements. Honor known-neutral constraints supplied in the question. Return insufficient "
            "if no concrete new source change is supported, rather than restating existing source. "
            "Real used typed locals or aggregate snapshots, and coupled source boundaries, may be "
            "proposed when grounded in actual supplied inputs, types and consumers. Explain the "
            "value's real use and why that boundary could affect the cited rows; this is not permission "
            "to add fake/dead locals or numeric register shaping. A known-neutral spelling does not "
            "establish that every distinct evidence-backed source boundary is neutral. Compiler "
            "outcomes remain untested predictions, not guaranteed gains. "
            "Honor the supplied language dialect. When C99/mixed-declaration support is not "
            "established, put declarations at block entry; a natural nested block may keep a "
            "legitimate used snapshot local to its use. Do not change language flags to make "
            "a hypothesis compile. "
            "Do not invent loops, fake locals/storage/operations, ABI/signatures, syntax "
            "matrices, or alternative proposals. PowerPC subf d,a,b computes b-a. "
            "Respect volatile clobbers and actual return values at calls; an argument-register value "
            "alone does not prove an extra argument. UNKNOWN/truncated producer context is not proof "
            "of source ownership or cross-CFG/call dataflow. "
            "Return only JSON with exactly these fields: "
            '{"status":"hypothesis or supported or insufficient","function":"name",'
            '"packet_sha256":"copied hash","answer":"concise finding",'
            '"evidence_rows":[0],"missing_evidence":null}. '
            "For hypothesis, answer must be one object instead of a string, with exactly "
            '{"cause":"one natural-C cause","prediction":"expected effect on cited rows",'
            '"source_change":{"before":"exact unique substring of source_excerpt",'
            '"after":"proposed replacement"}}; '
            "before must occur exactly once in the sealed source_excerpt; after must be nonempty "
            "and different. This is only a proposal, never an applied patch or authority to act. "
            "evidence_rows must be nonempty supplied row IDs predicted to change, and missing_evidence "
            "must be a nonempty uncertainty/limitation string. For supported use a factual string, "
            "nonempty citations and null missing_evidence. For insufficient use a string and nonempty "
            "missing_evidence. Cite only supplied paired/census/producer-node row IDs. "
            "Astra alone chooses and tests any cause; this answer grants no patch, compile, retention, "
            "or promotion authority. Semantic plausibility and the single-cause rule require primary "
            "review, not merely schema validation.\n"
            + ("COMPARISON DIRECTION: target is the immutable desired reference; only candidate C "
               "may change. Do not optimize target to resemble candidate. A target-only aligned row "
               "is missing_from_candidate: the candidate must reproduce the desired target behavior, "
               "not remove the target instruction. A candidate-only row is extra_in_candidate: "
               "extra current behavior at that alignment, not desired target behavior. Paired "
               "differences are observations, not rewrite commands. Alignment can pair different "
               "operations or split moved instructions into gaps; no row alone proves a semantic "
               "addition/deletion, source cause, or required C statement. Review the supplied "
               "context and preserve uncertainty.\n" if "comparison_direction" in packet else "")
            + known_context + json.dumps(packet, ensure_ascii=False, separators=(",", ":")))
    return (("TARGET-ONLY factual support: the candidate function is missing. Candidate rows are null "
             "and its census is unavailable, not an empty implementation or exact comparison. "
             "source_excerpt is context declarations, not the missing function's source body. "
             "Answer only supplied ABI/layout/control-flow evidence questions; do not propose source "
             "changes or treat context as an implementation. Real target rows/calls/branches are the "
             "only machine evidence; preserve uncertainty about unsupplied callee signatures.\n"
             if packet.get("candidate_missing") else "")
            + ("target_address and target_branch_destinations bind branch addresses to supplied row IDs; "
             "use this mapping rather than guessing case labels from row order. Destination rows marked "
             "omitted/external/unresolved are not supplied instruction evidence. These are address facts, "
             "not inferred switch-case meanings or new authority.\n"
             if "target_branch_destinations" in packet else "")
            + "Answer the ONE stated compiler/source question using only this bound evidence. "
            "Do not solve the entire function, invent a new algorithm, write a patch, or list experiments. "
            "The call/branch census covers the whole function; the paired arithmetic rows may be partial. "
            "PowerPC dataflow: subf d,a,b computes b-a. At a call, track volatile-register clobbers "
            "and the actual return value; r3 after a pointer-returning call is not the old r3 input. "
            "A live value in an argument register does not by itself prove an extra call argument. "
            "Use supplied source/callee signatures; if a signature or reaching definition is absent, "
            "state the uncertainty instead of inventing an ABI. "
            "Machine facts do not prove unique original C. If the question needs missing evidence, return "
            "insufficient and name that evidence; do not speculate until a rewrite seems plausible. "
            "Finish once the question is answered. Return only JSON with exactly these fields: "
            '{"status":"supported or insufficient","function":"name","packet_sha256":"copied hash",'
            '"answer":"concise finding, distinguish fact from hypothesis",'
            '"evidence_rows":[0],"missing_evidence":null}. '
            "Cite only row IDs present below. For insufficient use a nonempty missing_evidence string. "
            "Astra reviews the finding and owns any source decision/compile; this answer grants no authority.\n"
            + ("Optional producer_context contains bounded same-block physical definitions, not source "
               "ownership or cross-CFG/call inference. UNKNOWN and truncated dependencies remain missing "
               "evidence; cite only supplied nodes, not dangling definition_row references.\n"
               if "producer_context" in packet else "")
            + known_context + json.dumps(packet, ensure_ascii=False, separators=(",", ":")))


def validate_decision_packet(packet: dict) -> None:
    if (not isinstance(packet, dict) or packet.get("schema") != "recovery_support_decision/v1"
            or packet.get("authority_advanced") is not False
            or packet.get("source_causality_proven") is not False
            or packet.get("packet_sha256") != _digest({k: v for k, v in packet.items() if k != "packet_sha256"})):
        raise ValueError("invalid or changed decision packet")
    if "decision_mode" in packet and packet["decision_mode"] != "source-hypothesis":
        raise ValueError("invalid decision mode")
    if (not isinstance(packet.get("function"), str) or not packet["function"]
            or not isinstance(packet.get("question"), str) or not packet["question"].strip()
            or not isinstance(packet.get("paired_rows"), list) or not packet["paired_rows"]
            or set(packet.get("machine_census", {})) != {"target", "candidate"}):
        raise ValueError("incomplete decision packet")
    if "known_measurements" in packet:
        context = packet["known_measurements"]
        if (not isinstance(context, dict) or set(context) != {
                "entries", "diagnostic_only", "compiler_proof_reusable", "suppress_compile"}
                or context.get("diagnostic_only") is not True
                or context.get("compiler_proof_reusable") is not False
                or context.get("suppress_compile") is not False):
            raise ValueError("invalid advisory known measurement context")
        _known_measurement_context(context["entries"], packet["function"])
    if "comparison_direction" in packet:
        if (packet.get("decision_mode") != "source-hypothesis"
                or packet["comparison_direction"] != _comparison_direction(packet["paired_rows"])):
            raise ValueError("comparison direction differs from supplied paired rows or mode")
    if "candidate_missing" in packet or "source_excerpt_role" in packet:
        if (packet.get("candidate_missing") is not True or packet.get("source_excerpt_role") != "context"
                or "decision_mode" in packet
                or packet["machine_census"]["candidate"] != {
                    "bytes": None, "instruction_count": None, "calls": [],
                    "branches_including_return": [], "status": "unavailable"}
                or any(not isinstance(row, dict) or row.get("candidate", "absent") is not None
                       for row in packet["paired_rows"])):
            raise ValueError("invalid target-only decision evidence/mode")
        has_mapping = ("target_instruction_addresses" in packet or "target_branch_destinations" in packet
                       or any("target_address" in row for row in packet["paired_rows"]))
        if has_mapping:
            if packet.get("target_branch_destinations") != _target_only_branch_facts(packet):
                raise ValueError("target branch destination mapping differs from supplied rows/addresses")
    if "producer_context" in packet:
        context = packet["producer_context"]
        expected_sides = {"target"} if packet.get("candidate_missing") else {"target", "candidate"}
        if not isinstance(context, dict) or set(context) != expected_sides:
            raise ValueError("invalid decision producer context")
        selected = {row["row"] for row in packet["paired_rows"]}
        for sliced in context.values():
            if (not isinstance(sliced, dict) or type(sliced.get("node_limit")) is not int
                    or not 4 <= sliced["node_limit"] <= 256 or sliced["node_limit"] % 4
                    or type(sliced.get("truncated")) is not bool
                    or not isinstance(sliced.get("sites"), list) or not sliced["sites"]
                    or any(type(i) is not int or i not in selected for i in sliced["sites"])
                    or len(set(sliced["sites"])) != len(sliced["sites"])
                    or len(sliced["sites"]) > sliced["node_limit"] // 4
                    or not isinstance(sliced.get("nodes"), list)
                    or len(sliced["nodes"]) > sliced["node_limit"]
                    or sliced.get("scope") != "block-local physical definitions; memory identity and source causality UNKNOWN"):
                raise ValueError("invalid decision producer context bounds/scope")
            ids = []
            for node in sliced["nodes"]:
                if (not isinstance(node, dict) or type(node.get("row")) is not int
                        or not 0 <= node["row"] <= max(sliced["sites"])
                        or not isinstance(node.get("instruction"), str)
                        or not isinstance(node.get("uses"), list)):
                    raise ValueError("invalid decision producer node")
                ids.append(node["row"])
                for use in node["uses"]:
                    if (not isinstance(use, dict) or not isinstance(use.get("register"), str)
                            or ("definition_row" in use and (type(use["definition_row"]) is not int
                                or not 0 <= use["definition_row"] < node["row"]))
                            or ("definition_row" not in use and (use.get("status") != "UNKNOWN"
                                or not isinstance(use.get("reason"), str)))):
                        raise ValueError("invalid decision producer dependency")
            if ids != sorted(set(ids)):
                raise ValueError("duplicate/unordered decision producer nodes")


def validate_decision_answer(packet: dict, answer: dict) -> dict:
    """Mechanical citation/identity checks, never automatic truth/retention."""
    validate_decision_packet(packet)
    keys = {"status", "function", "packet_sha256", "answer", "evidence_rows", "missing_evidence"}
    if not isinstance(answer, dict) or set(answer) != keys:
        raise ValueError("decision answer must use the exact JSON contract; patches are not findings")
    if answer["function"] != packet["function"] or answer["packet_sha256"] != packet["packet_sha256"]:
        raise ValueError("decision answer function/packet binding differs")
    hypothesis = answer["status"] == "hypothesis" and packet.get("decision_mode") == "source-hypothesis"
    if hypothesis:
        proposal = answer["answer"]
        if (not isinstance(proposal, dict) or set(proposal) != {"cause", "prediction", "source_change"}
                or any(not isinstance(proposal[key], str) or not proposal[key].strip()
                       for key in ("cause", "prediction"))):
            raise ValueError("hypothesis needs one cause and row-effect prediction")
        change = proposal["source_change"]
        if (not isinstance(change, dict) or set(change) != {"before", "after"}
                or any(not isinstance(value, str) or not value.strip() for value in change.values())
                or change["before"] == change["after"]
                or not isinstance(packet.get("source_excerpt"), str)
                or packet["source_excerpt"].count(change["before"]) != 1):
            raise ValueError("hypothesis needs one unique bound before and different nonempty after")
    elif answer["status"] not in {"supported", "insufficient"} or not isinstance(answer["answer"], str) or not answer["answer"].strip():
        raise ValueError("decision answer lacks status/finding")
    cited = answer["evidence_rows"]
    available = {row["row"] for row in packet["paired_rows"]}
    for sliced in packet.get("producer_context", {}).values():
        available.update(node["row"] for node in sliced["nodes"])
    for stream in packet["machine_census"].values():
        available.update(site["row"] for kind in ("calls", "branches_including_return") for site in stream[kind])
    if (not isinstance(cited, list) or any(type(i) is not int or i not in available for i in cited)
            or len(set(cited)) != len(cited)):
        raise ValueError("decision answer cites missing/duplicate rows")
    if answer["status"] == "supported" and (not cited or answer["missing_evidence"] is not None):
        raise ValueError("supported finding needs citations and no missing evidence")
    if answer["status"] == "insufficient" and (not isinstance(answer["missing_evidence"], str) or not answer["missing_evidence"].strip()):
        raise ValueError("insufficient finding must identify missing evidence")
    if hypothesis and (not cited or not isinstance(answer["missing_evidence"], str)
                       or not answer["missing_evidence"].strip()):
        raise ValueError("hypothesis needs predicted rows and explicit uncertainty")
    return {"status": "valid_finding" if hypothesis or answer["status"] == "supported" else "insufficient_evidence",
            "packet_sha256": packet["packet_sha256"], "answer_sha256": _digest(answer),
            "factual_correctness": "requires primary review", "authority_advanced": False,
            **({"finding_status": "hypothesis", "review_required": True, "authority": False}
               if hypothesis else {})}


def producer_slice(rows: list[dict], sites: list[int], *, limit: int = 16) -> dict:
    """Bounded block-local physical-register DAG; no source/alias inference.

    Native trace slicing needs a compiler capture; frontier's definition state
    only answers must-defined FPRs. Reuse its objdiff/branch parsers here.
    """
    if not 1 <= limit <= 64:
        raise ValueError("producer limit must be between 1 and 64")
    branches, addresses = frontier._branch_rows(rows)
    entries = {0}
    for branch in branches:
        entries.update(addresses.get(branch.get("destination"), []))
    # Duplicate addresses/label aliases are not unique block identities.
    for indices in addresses.values():
        if len(indices) > 1:
            entries.update(indices)
    definitions = {}
    records = {}
    boundary = {"status": "UNKNOWN", "reason": "CFG/function entry"}
    loads = {"lwz", "lhz", "lha", "lbz", "lfs", "lfd"}
    update_loads = {"lwzu", "lfsu", "lfdu"}
    arithmetic = {"mr", "fmr", "add", "addi", "addis", "subi", "subf", "mullw", "mulli",
                  "slwi", "srwi", "srawi", "rlwinm", "clrlwi", "clrlslwi", "neg", "extsh", "extsb",
                  "and", "andi.", "or", "xor", "fadd", "fadds", "fsub", "fsubs",
                  "fmul", "fmuls", "fdiv", "fdivs", "fneg", "fabs", "frsp"}
    harmless = {"nop", "cmpw", "cmplw", "cmpwi", "cmplwi", "fcmpo", "fcmpu",
                "stw", "sth", "stb", "stfs", "stfd", "mtctr"}
    for index, row in enumerate(rows):
        if index in entries:
            definitions = {}
            boundary = {"status": "UNKNOWN", "reason": "CFG/function entry", "row": index}
        parts = frontier._instruction_parts(row)
        if parts is None:
            if row.get("instruction") or row.get("label"):
                definitions = {}
                boundary = {"status": "UNKNOWN", "reason": "unsupported/label boundary", "row": index}
            continue
        op, operands = parts
        registers = frontier._register_tokens(",".join(operands))
        record = {"row": index, "instruction": row["instruction"]["formatted"], "uses": []}
        # Update forms define two values: loaded RT/FRT and EA in RA. Capture
        # the old RA use before publishing either definition. Only numeric
        # D-form addresses are understood here; symbolic/illegal forms stop.
        update = None
        if op in update_loads and len(operands) == 2:
            update = re.fullmatch(r"(-?(?:0x[0-9a-f]+|\d+))\((r(?:[12]?\d|3[01]))\)", operands[1])
            destination = r"r(?:[12]?\d|3[01])" if op == "lwzu" else r"f(?:[12]?\d|3[01])"
            if (not update or not re.fullmatch(destination, operands[0])
                    or update[2] == "r0" or (op == "lwzu" and operands[0] == update[2])
                    or not -32768 <= int(update[1], 0) <= 32767):
                update = None
        supported = op in loads | arithmetic | harmless | {"li", "lis"} or update is not None
        is_def = op in loads | arithmetic | {"li", "lis"} or update is not None
        uses = registers[1:] if is_def else registers
        if op in {"li", "lis"}:
            uses = []
        if op in {"addi", "addis", "subi"} and len(operands) > 1 and operands[1] == "r0":
            uses = []  # PPC RA=0 is a literal zero here, not the r0 value.
        if op in loads and len(operands) > 1 and operands[1].endswith("(r0)"):
            uses = []
        for register in uses if supported else []:
            record["uses"].append({"register": register, **(
                {"definition_row": definitions[register]} if register in definitions else boundary)})
        if op in loads and len(operands) == 2:
            match = re.fullmatch(r"(-?(?:0x[0-9a-f]+|\d+))\((r\d+)\)", operands[1])
            record["memory_root"] = ({"base": match[2], "offset": int(match[1], 0),
                                       "zero_base": match[2] == "r0", "alias_identity": "UNKNOWN"} if match else
                                      {"status": "UNKNOWN", "reason": "symbolic/unsupported address"})
        if update is not None:
            record["memory_root"] = {"base": update[2], "offset": int(update[1], 0),
                                     "zero_base": False, "alias_identity": "UNKNOWN"}
            record["base_update"] = {"register": update[2], "operation": "old_base_plus_offset",
                                     "offset": int(update[1], 0)}
        if not supported:
            reason = "call boundary" if op in {"bl", "bla", "bctrl", "blrl"} else "CFG boundary" if op.startswith("b") else "unsupported opcode"
            record.update(status="UNKNOWN", reason=reason)
            definitions = {}
            boundary = {"status": "UNKNOWN", "reason": reason, "row": index}
        elif is_def and registers:
            record["defines"] = registers[0]
            definitions[registers[0]] = index
            if update is not None:
                definitions[update[2]] = index
        records[index] = record
    selected = sites[:limit]
    pending = list(selected)
    included = {}
    while pending and len(included) < limit * 4:
        index = pending.pop(0)
        if index in included or index not in records:
            continue
        included[index] = records[index]
        pending.extend(use["definition_row"] for use in records[index]["uses"] if "definition_row" in use)
    return {"sites": selected, "nodes": [included[i] for i in sorted(included)],
            "truncated": len(sites) > limit or bool(pending), "node_limit": limit * 4,
            "scope": "block-local physical definitions; memory identity and source causality UNKNOWN"}


def summarize_groups(document: dict, function: str, *, producers: int = 0) -> dict[str, Any]:
    """Summarize canonical objdiff aligned rows using existing frontier parsers.

    Group IDs use target addresses (insertions use the next target address).
    Register relation buckets are observations, not independently proven causes.
    """
    symbols = [frontier.focus._symbols(document, side, "strict") for side in ("left", "right")]
    selected = [frontier._stack_function(items, function, side)
                for items, side in zip(symbols, ("target", "candidate"))]
    if any(item is None for item in selected):
        raise ValueError(f"function missing from report: {function}")
    rows = [frontier.focus._rows(item, function) for item in selected]
    left, right = rows
    # Aligned row numbers move when objdiff inserts a candidate-only row. The
    # target instruction ordinal does not. Keep a complete, target-bound site
    # census so a changed relation/category cannot masquerade as a repair.
    target_sites = []
    target_stream = []
    for index, row in enumerate(left):
        target_sites.append(len(target_stream))
        if row.get("instruction"):
            target_stream.append({
                "instruction": frontier._diagnose_payload(row, index),
                "branch_dest": row["instruction"].get("branch_dest"),
                "relocation": frontier.relocation_key(row, symbols[0]),
            })
    target_binding = {
        "instruction_count": len(target_stream),
        "sha256": hashlib.sha256(json.dumps({"size": selected[0].get("size"), "instructions": target_stream}, sort_keys=True,
            separators=(",", ":")).encode("utf-8")).hexdigest(),
    }
    branches = frontier._branch_category(left, right)
    branch_findings = {item["row_index"]: item["status"] for item in branches["findings"]}
    permutation = frontier._register_permutation(left, right)
    differences = frontier._diagnose_rows(left, right)
    buckets: dict[tuple, list[dict]] = {}
    counts: Counter = Counter()
    unresolved = 0
    label_only_rows = 0
    total = max(map(len, rows))
    for index in range(total):
        pair = [side[index] if index < len(side) else {} for side in rows]
        ins = [item.get("instruction") or {} for item in pair]
        payload = [frontier._diagnose_payload(item, index) for item in pair]
        relocs = [frontier.relocation_key(item, table) for item, table in zip(pair, symbols)]
        kinds = [frontier._diagnose_kind(item) for item in pair]
        branch = branch_findings.get(index)
        label_only = (all(relocs) and relocs[0] != relocs[1]
                      and all(relocs[0][key] == relocs[1][key] for key in ("type", "addend")))
        if payload[0] == payload[1] and label_only and not any(kinds) and not branch:
            label_only_rows += 1
            continue
        if payload[0] == payload[1] and relocs[0] == relocs[1] and not any(kinds) and not branch:
            continue
        texts = [item.get("formatted", "") for item in ins]
        ops = [text.split()[0] if text else "" for text in texts]
        signature: tuple = ()
        if relocs[0] != relocs[1]:
            if label_only and any(op in {"bl", "bla"} for op in ops) and texts[0] != texts[1]:
                helper = all(str(item.get("symbol", "")).startswith(("_savegpr_", "_restgpr_", "_savefpr_", "_restfpr_")) for item in relocs)
                category = "abi_helper_call" if helper else "call_relocation"
            elif label_only:
                # Symbol labels are not physical owner identities. In particular
                # lbl_* versus @pool labels are not proven relocation changes.
                category = "relocation_identity_unknown"
            else:
                category = "call_relocation" if any(op in {"bl", "bla"} for op in ops) else "relocation"
            signature = tuple(json.dumps(item, sort_keys=True) for item in relocs)
        elif branch:
            category = "cfg_changed" if branch == "changed" else "cfg_unknown"
        elif not all(ins):
            category = ("added_" if not ins[0] else "deleted_") + ("move" if next((op for op in ops if op), "") in {"mr", "fmr"} else "instruction")
        elif ops[0] != ops[1] or ins[0].get("opcode") != ins[1].get("opcode"):
            category = "opcode"
            signature = tuple(ops)
        elif texts[0] != texts[1] and frontier._without_registers(texts[0]) == frontier._without_registers(texts[1]):
            signature = tuple((a, b) for a, b in zip(frontier._register_tokens(texts[0]), frontier._register_tokens(texts[1])) if a != b)
            category = "register_permutation" if permutation["status"] == "confirmed" else "register_relation_unknown"
        elif payload[0] != payload[1]:
            category = "operand"
        else:
            category = "annotation_unknown"
        if category.endswith("unknown"):
            unresolved += 1
        counts[category] += 1
        address = ins[0].get("address")
        anchor = "target"
        if address is None:
            address = next(((row.get("instruction") or {}).get("address") for row in left[index + 1:] if (row.get("instruction") or {}).get("address") is not None), "end")
            anchor = "before_target"
        target_index = target_sites[index] if index < len(target_sites) else len(target_stream)
        buckets.setdefault((category, signature), []).append({"row": index, "target_address": address,
            "target_index": target_index, "anchor": anchor})
    groups = []
    for (category, signature), members in buckets.items():
        first = members[0]
        groups.append({"id": f"{function}:{first['anchor']}:{first['target_address']}:{category}",
                       "kind": category, "row_count": len(members), "first": first,
                       "last": members[-1], "members": members, "relation": signature,
                       "cause_proven": False})
    hazard_kinds = {"cfg_changed", "cfg_unknown", "opcode", "operand", "relocation", "call_relocation", "abi_helper_call", "added_instruction", "deleted_instruction"}
    sizes = [frontier.focus._integer(item.get("size")) for item in selected]
    size_exact = None not in sizes and sizes[0] == sizes[1]
    hazards = sum(value for key, value in counts.items() if key in hazard_kinds) + int(not size_exact)
    result = {"schema": "recovery_causal_groups/v1", "function": function,
              "groups": groups, "category_rows": dict(sorted(counts.items())),
              "coverage": {"residual_rows": sum(counts.values()), "grouped_rows": sum(counts.values()), "aligned_rows": total},
              "label_only_rows_excluded": label_only_rows,
              "unresolved_row_count": unresolved, "structural_hazard_count": hazards,
              "size_exact": size_exact, "instruction_exact": not groups and size_exact,
              "target_binding": target_binding,
              "first_machine_divergence": differences["first_instruction_mismatch"],
              "earliest_source_cause": {"status": "unknown", "reason": "aligned rows do not establish source causality"},
              "register_mapping": {"status": permutation["status"], "reason": permutation["reason"],
                                   "mapping": permutation["mapping"] if permutation["status"] == "confirmed" else {},
                                   "conflicts": permutation["mapping_conflicts"],
                                   "body_projection": permutation.get("body_projection")},
              "diagnostic_only": True, "physical_proof": False, "authority_advanced": False,
              "source_emission_authorized": False}
    result["group_semantics"] = "observational relation buckets, not independent defects or proven causes"
    result["numeric_domain_evidence"] = numeric_domain_evidence(left, right)
    result["constant_copy_evidence"] = constant_copy_evidence(left, right)
    if result["first_machine_divergence"]:
        result["first_machine_divergence"].pop("context", None)
    result["ranking_tuple"] = ranking_tuple(result)
    if producers:
        sites = sorted({m["row"] for group in groups for m in group["members"]})
        result["producer_slice"] = {side: producer_slice(stream, sites, limit=producers)
                                    for side, stream in zip(("target", "candidate"), rows)}
    return result


def constant_copy_evidence(left: list[dict], right: list[dict]) -> dict:
    """Recognize a proven block-local constant copy, not its original C cause.

    DelayBlock and PitchWindow used the same live fill-value parameter. Generic
    opcode triage had obscured that clue. Reuse the existing definition slicer;
    never propagate constants through calls, joins, or unsupported operations.
    """
    candidates = []
    for index, (target, candidate) in enumerate(zip(left, right)):
        if not isinstance(target, dict) or not isinstance(candidate, dict):
            continue
        tp, cp = frontier._instruction_parts(target), frontier._instruction_parts(candidate)
        if not tp or not cp or cp[0] != "li" or len(cp[1]) != 2:
            continue
        if tp[0] == "mr" and len(tp[1]) == 2:
            destination, source = tp[1]
        elif tp[0] == "addi" and len(tp[1]) == 3:
            destination, source, offset = tp[1]
            try:
                if int(offset, 0) != 0 or source == "r0":
                    continue
            except ValueError:
                continue
        else:
            continue
        if (destination != cp[1][0] or destination == source
                or not re.fullmatch(r"r(?:[0-9]|[12][0-9]|3[01])", source)):
            continue
        try:
            immediate = int(cp[1][1], 0)
        except ValueError:
            continue
        candidates.append((index, destination, source, immediate))
    signals = []
    selected = candidates[:16]
    if selected and all(isinstance(row, dict) for row in left):
        sliced = producer_slice(left, [item[0] for item in selected], limit=16)
        nodes = {node["row"]: node for node in sliced["nodes"]}
        for index, destination, source, immediate in selected:
            use = next((u for u in nodes.get(index, {}).get("uses", [])
                        if u["register"] == source), {})
            definition = use.get("definition_row")
            if definition not in nodes:
                continue
            producer = frontier._instruction_parts(left[definition])
            if not producer or producer[0] != "li" or len(producer[1]) != 2:
                continue
            try:
                if int(producer[1][1], 0) != immediate:
                    continue
            except ValueError:
                continue
            signals.append({"row": index, "producer_row": definition,
                            "value": immediate, "source_register": source,
                            "destination_register": destination,
                            "target": left[index]["instruction"]["formatted"],
                            "candidate": right[index]["instruction"]["formatted"],
                            "kind": "constant_copy_vs_literal", "cause_proven": False})
    return {"signals": signals, "sites_truncated": len(candidates) > 16,
            "scope": "same-block reaching li definition; no source identity or retention proof",
            "source_review": "Inspect a real consumed argument/automatic-inline boundary, such as a fill-value operation, before constant or register spellings. The copy alone does not prove a helper; do not add dead aliases or force registers.",
            "authority_advanced": False}


def compare_groups(before: dict, after: dict) -> dict:
    """A bucket disappeared is not the same fact as its instructions matching.

    Closure requires the same target stream and every former target site to be
    absent from the new mismatch census. Insertions conservatively share their
    following target site. No compiler/source causality is inferred here.
    """
    if before["function"] != after["function"]:
        raise ValueError("cannot compare different functions")
    old = {item["id"]: item for item in before["groups"]}
    new = {item["id"]: item for item in after["groups"]}
    binding = before.get("target_binding")
    comparable = (isinstance(binding, dict) and isinstance(binding.get("sha256"), str)
                  and binding == after.get("target_binding"))
    locations = []
    for summary in (before, after):
        by_group = {}
        for group in summary["groups"]:
            members = group.get("members")
            if (not isinstance(members, list) or len(members) != group["row_count"]
                    or not members or any(not isinstance(m, dict)
                        or type(m.get("target_index")) is not int or m["target_index"] < 0
                        for m in members)):
                comparable = False
                continue
            by_group[group["id"]] = {m["target_index"] for m in members}
        locations.append(by_group)
    after_sites = set().union(*locations[1].values()) if locations[1] else set()
    before_sites = set().union(*locations[0].values()) if locations[0] else set()
    closed = sorted(key for key, sites in locations[0].items()
                    if comparable and sites.isdisjoint(after_sites))
    reclassified = sorted((old.keys() - new.keys()) - set(closed)) if comparable else []
    return {"function": after["function"], "closed_groups": closed,
            "status": "observed" if comparable else "unknown",
            "closure_unknown_reason": None if comparable else "target binding or complete target-site census unavailable/different",
            "resolved_target_site_count": len(before_sites - after_sites) if comparable else None,
            "introduced_target_site_count": len(after_sites - before_sites) if comparable else None,
            "reclassified_groups": reclassified,
            "disappeared_observation_buckets": sorted(old.keys() - new.keys()),
            "new_groups": sorted(new.keys() - old.keys()),
            "changed_groups": [{"id": key, "before_rows": old[key]["row_count"], "after_rows": new[key]["row_count"]}
                               for key in sorted(old.keys() & new.keys()) if old[key] != new[key]],
            "structural_hazards_before": before["structural_hazard_count"],
            "structural_hazards_after": after["structural_hazard_count"],
            "residual_rows_delta": after["coverage"]["residual_rows"] - before["coverage"]["residual_rows"],
            "comparison_scope": "closed requires all previous group target sites to match; changed buckets are not repairs; no source cause is inferred",
            "cause_proven": False, "authority_advanced": False}


def compare_function_constraints(before: dict, after: dict, function: str) -> dict:
    """Target-address anchored observations; fewer rows do not authorize retention."""
    summaries = [summarize_groups(doc, function) for doc in (before, after)]
    streams, censuses = [], []
    for document, summary in zip((before, after), summaries):
        selected = [frontier._stack_function(frontier.focus._symbols(document, side, "strict"),
                    function, side) for side in ("left", "right")]
        left, right = [frontier.focus._rows(symbol, function) for symbol in selected]
        if len(left) != len(right):
            raise ValueError(f"ambiguous aligned deletions: {function}: unequal row counts")
        identities = []
        for row in left:
            ins = row.get("instruction")
            if not ins:
                continue
            address = frontier.focus._integer(ins.get("address"))
            text = ins.get("formatted")
            if address is None or not isinstance(text, str) or not text.strip():
                raise ValueError(f"unavailable target address/opcode identity: {function}")
            identities.append((address, text, json.dumps(ins.get("parts", []), sort_keys=True)))
        if len({key[0] for key in identities}) != len(identities):
            raise ValueError(f"ambiguous target addresses: {function}")
        streams.append(identities)
        mismatch = {m["row"]: group for group in summary["groups"] for m in group["members"]}
        census, ordinal, inserted = {}, 0, set()
        for index, (target, candidate) in enumerate(zip(left, right)):
            ti, ci = target.get("instruction"), candidate.get("instruction")
            if not ti and not ci:
                raise ValueError(f"ambiguous aligned deletions: {function}: empty pair {index}")
            if ti:
                key = ("target", identities[ordinal])
                ordinal += 1
            else:
                # Multiple insertions at one boundary cannot be paired across reports.
                boundary = identities[ordinal] if ordinal < len(identities) else ("end",)
                key = ("before_target", boundary)
                if key in inserted:
                    raise ValueError(f"ambiguous aligned insertions: {function}: shared boundary")
                inserted.add(key)
            if index in mismatch:
                group = mismatch[index]
                census[key] = {"kind": group["kind"], "relation": group["relation"],
                    "target_address": ti.get("address") if ti else None,
                    "anchor": key[0], "boundary": key[1][0],
                    "target": ti.get("formatted") if ti else None,
                    "candidate": ci.get("formatted") if ci else None}
        censuses.append(census)
    if streams[0] != streams[1] or summaries[0]["target_binding"] != summaries[1]["target_binding"]:
        raise ValueError(f"incompatible target/function instruction identity: {function}")
    old, new = censuses
    buckets = {"resolved": [], "persisting": [], "introduced": []}
    for key in sorted(old.keys() | new.keys(), key=repr):
        state = "persisting" if key in old and key in new else "resolved" if key in old else "introduced"
        buckets[state].append({"before": old.get(key), "after": new.get(key)})
    grouped, details_truncated = {}, False
    for state, sites in buckets.items():
        groups = {}
        for site in sites:
            signature = tuple(json.dumps((site[side]["kind"], site[side]["relation"]))
                              if site[side] else None for side in ("before", "after"))
            groups.setdefault(signature, []).append(site)
        details_truncated |= len(groups) > 24 or any(len(members) > 12 for members in groups.values())
        grouped[state] = [{"row_count": len(members), "sites": members[:12],
                           "sites_truncated": len(members) > 12} for members in groups.values()][:24]
    return {"function": function, "target_binding": summaries[0]["target_binding"],
            "report_sha256": {label: hashlib.sha256(json.dumps(doc, sort_keys=True,
                separators=(",", ":")).encode()).hexdigest()
                for label, doc in zip(("baseline_canonical_json", "candidate_canonical_json"), (before, after))},
            "constraint_groups": grouped,
            "counts": {state: len(sites) for state, sites in buckets.items()},
            "details_truncated": details_truncated,
            "score_size_frame_gates": compare_match_frontiers(before, after),
            "structural_hazards": [s["structural_hazard_count"] for s in summaries],
            "comparison_scope": "same target address/opcode sites; persisting may change category/expression; insertions are boundary observations, not target repairs",
            "cause_proven": False, "retention_authorized": False, "authority_advanced": False}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", type=Path)
    parser.add_argument("--baseline-strict", type=Path,
                        help="compare score frontiers (--owner-summary) or target constraints (--function)")
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--function")
    mode.add_argument("--owner-summary", action="store_true")
    mode.add_argument("--check-support-answer", type=Path)
    mode.add_argument("--check-support-prompt", type=Path)
    parser.add_argument("--decision-packet", type=Path)
    parser.add_argument("--before", type=Path)
    parser.add_argument("--support-source", type=Path)
    parser.add_argument("--source-lines", help="inclusive START:END for the bounded support question")
    parser.add_argument("--support-max-bytes", type=int, default=24000)
    parser.add_argument("--decision-question", help="generate a decision packet on the existing support-source path")
    parser.add_argument("--decision-rows", help="inclusive START:END aligned machine rows for that decision")
    parser.add_argument("--decision-mode", choices=("fact", "source-hypothesis"))
    parser.add_argument("--known-measurements", type=Path,
                        help="explicit selected observation entries JSON; decision generation only, no artifact discovery")
    parser.add_argument("--producers", type=int, default=0, metavar="LIMIT",
                        help="optional block-local producer slice, 1..64 residual sites")
    args = parser.parse_args(argv)
    try:
        if args.baseline_strict and (not args.strict or not (args.owner_summary or args.function) or args.before
                or args.support_source or args.source_lines or args.decision_packet or args.producers
                or args.decision_question or args.decision_rows or args.decision_mode or args.known_measurements):
            raise ValueError("--baseline-strict requires only --strict and --owner-summary or --function")
        decision_options = any(value is not None for value in (
            args.decision_question, args.decision_rows, args.decision_mode, args.known_measurements))
        if decision_options and not (args.function and args.support_source and args.source_lines
                and args.decision_question and args.decision_rows and not args.before
                and not args.decision_packet):
            raise ValueError("decision generation requires --function, --support-source, --source-lines, "
                             "--decision-question and --decision-rows; --known-measurements is valid only "
                             "there, not with --before or --decision-packet checks")
        if args.check_support_answer or args.check_support_prompt:
            if not args.decision_packet:
                raise ValueError("support check requires --decision-packet")
            packet = frontier.load_json(args.decision_packet.read_bytes())
            if args.check_support_answer:
                result = validate_decision_answer(packet, frontier.load_json(args.check_support_answer.read_bytes()))
            else:
                if args.check_support_prompt.read_text(encoding="utf-8") != render_decision_prompt(packet):
                    raise ValueError("prompt differs from its decision packet")
                result = {"status": "valid_prompt", "packet_sha256": packet["packet_sha256"]}
            print(json.dumps(result, separators=(",", ":")))
            return 0
        if not args.strict:
            raise ValueError("report operation requires --strict")
        def read_document(path: Path) -> dict:
            with path.open("rb") as stream:
                raw = stream.read(frontier.REPORT_LIMIT + 1)
            if len(raw) > frontier.REPORT_LIMIT:
                raise ValueError(f"report exceeds limit: {path}")
            if not 0 <= args.producers <= 64:
                raise ValueError("producers must be between 0 and 64")
            return frontier.load_json(raw)
        if (args.support_source or args.source_lines) and (not args.function or not args.support_source or not args.source_lines or args.before):
            raise ValueError("support excerpt requires --function, --support-source and --source-lines; no --before")
        if args.owner_summary and args.before:
            raise ValueError("--before requires --function")
        document = read_document(args.strict)
        if args.baseline_strict:
            baseline = read_document(args.baseline_strict)
            result = (compare_function_constraints(baseline, document, args.function) if args.function
                      else compare_match_frontiers(baseline, document))
        elif args.support_source:
            start, end = map(int, args.source_lines.split(":"))
            with args.support_source.open("rb") as stream:
                source_bytes = stream.read(1024 * 1024 + 1)
            if len(source_bytes) > 1024 * 1024:
                raise ValueError("support source exceeds 1 MiB; select a smaller source input")
            if decision_options:
                row_start, row_end = map(int, args.decision_rows.split(":"))
                known = None
                if args.known_measurements is not None:
                    with args.known_measurements.open("rb") as stream:
                        raw = stream.read(16 * 1024 + 1)
                    if len(raw) > 16 * 1024:
                        raise ValueError(f"{args.known_measurements}: selected measurements file exceeds 16 KiB")
                    try:
                        known = frontier.load_json(raw)
                        _known_measurement_context(known, args.function)
                    except (ValueError, TypeError) as exc:
                        raise ValueError(f"{args.known_measurements}: {exc}") from exc
                result = decision_packet(document, args.function, source_bytes.decode("utf-8"),
                    start, end, args.decision_question, row_start, row_end,
                    max_bytes=args.support_max_bytes, decision_mode=args.decision_mode or "fact",
                    known_measurements=known)
            else:
                result = support_packet(document, args.function, source_bytes.decode("utf-8"),
                                        start, end, max_bytes=args.support_max_bytes)
        elif args.owner_summary:
            result = summarize_owner(document)
        else:
            result = summarize_groups(document, args.function, producers=args.producers)
        if args.before:
            result["comparison"] = compare_groups(summarize_groups(read_document(args.before), args.function), result)
        print(json.dumps(result, sort_keys=True, separators=(",", ":")))
        return 0
    except (ValueError, OSError, TypeError, KeyError) as exc:
        print(f"causal groups: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
