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


def summarize_owner(document: dict) -> dict:
    """Small whole-object view: count closures and surface shared domain clues."""
    symbols = frontier.focus._symbols(document, "left", "strict")
    functions = [s for s in symbols if s.get("instructions")]
    residuals, exact = [], []
    for symbol in functions:
        summary = summarize_groups(document, symbol["name"])
        if summary["instruction_exact"]:
            exact.append(symbol["name"])
            continue
        signals = summary["numeric_domain_evidence"]["signals"]
        residuals.append({"function": symbol["name"],
                          "target_bytes": symbol.get("size"),
                          "score": symbol.get("match_percent"),
                          "residual_rows": summary["coverage"]["residual_rows"],
                          "size_exact": summary["size_exact"],
                          "domain_signals": signals,
                          "domain_review_first": any(s["review_priority"] == "source_domain" for s in signals)})
    residuals.sort(key=lambda r: (not r["domain_review_first"], r["residual_rows"], r["function"]))
    return {"schema": "recovery_owner_triage/v1", "functions_total": len(functions),
            "instruction_exact_count": len(exact), "remaining_count": len(residuals),
            "instruction_exact_functions": exact, "residuals": residuals,
            "ordering": "domain-review evidence before opaque register tails, then residual rows; advisory, not expected time or proof",
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
    # The first causal question plus repeated domain examples; never all 1000
    # instructions. Report how much was omitted so support cannot infer absence.
    centers = sites[:1] + [s["sites"][0]["row"] for s in summary["numeric_domain_evidence"]["signals"]]
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
              "residual_row_count": len(sites), "omitted_residual_row_count": len(set(sites)-set(selected)),
              "numeric_domain_evidence": summary["numeric_domain_evidence"],
              "scope": "one source decision; excerpt is incomplete, request a named missing span if needed; no filesystem or compiler access",
              "authority_advanced": False}
    size = len(json.dumps(result, ensure_ascii=False).encode("utf-8"))
    if size > max_bytes:
        raise ValueError(f"support excerpt {size} bytes exceeds {max_bytes}; narrow the source range/question, do not dispatch a full TU")
    return result


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
    arithmetic = {"mr", "fmr", "add", "addi", "addis", "subf", "mullw", "mulli",
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
        supported = op in loads | arithmetic | harmless | {"li", "lis"}
        is_def = op in loads | arithmetic | {"li", "lis"}
        uses = registers[1:] if is_def else registers
        if op in {"li", "lis"}:
            uses = []
        if op in {"addi", "addis"} and len(operands) > 1 and operands[1] == "r0":
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
        if not supported:
            reason = "call boundary" if op in {"bl", "bla", "bctrl", "blrl"} else "CFG boundary" if op.startswith("b") else "unsupported opcode"
            record.update(status="UNKNOWN", reason=reason)
            definitions = {}
            boundary = {"status": "UNKNOWN", "reason": reason, "row": index}
        elif is_def and registers:
            record["defines"] = registers[0]
            definitions[registers[0]] = index
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
    if result["first_machine_divergence"]:
        result["first_machine_divergence"].pop("context", None)
    result["ranking_tuple"] = ranking_tuple(result)
    if producers:
        sites = sorted({m["row"] for group in groups for m in group["members"]})
        result["producer_slice"] = {side: producer_slice(stream, sites, limit=producers)
                                    for side, stream in zip(("target", "candidate"), rows)}
    return result


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


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", type=Path, required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--function")
    mode.add_argument("--owner-summary", action="store_true")
    parser.add_argument("--before", type=Path)
    parser.add_argument("--support-source", type=Path)
    parser.add_argument("--source-lines", help="inclusive START:END for the bounded support question")
    parser.add_argument("--support-max-bytes", type=int, default=24000)
    parser.add_argument("--producers", type=int, default=0, metavar="LIMIT",
                        help="optional block-local producer slice, 1..64 residual sites")
    args = parser.parse_args(argv)
    try:
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
        if args.support_source:
            start, end = map(int, args.source_lines.split(":"))
            with args.support_source.open("rb") as stream:
                source_bytes = stream.read(1024 * 1024 + 1)
            if len(source_bytes) > 1024 * 1024:
                raise ValueError("support source exceeds 1 MiB; select a smaller source input")
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
