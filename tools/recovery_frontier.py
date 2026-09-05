#!/usr/bin/env python3
"""Compact current-source evidence index. No compile, history scan or retention gate.

Publish explicitly selected reports once, then verify the small index on resume.
Instruction exactness is not physical/link exactness. A supplied report is a
diagnostic input, not proof that its object was built from the supplied source.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import focus_symbol_report as focus

SCHEMA = "recovery_current_evidence/v1"
INDEX_LIMIT = 256 * 1024
REPORT_LIMIT = 32 * 1024 * 1024


def canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def local(root: Path, path: Path) -> Path:
    path = Path(os.path.abspath(root / path))
    path.relative_to(root)
    for part in (path, *path.parents):
        if part.is_symlink() or (hasattr(part, "is_junction") and part.is_junction()):
            raise ValueError(f"indirected evidence path: {part}")
        if part == root:
            break
    return path


def read_bound(root: Path, path: Path, limit: int) -> tuple[bytes, dict]:
    path = local(root, path)
    with path.open("rb") as stream:
        raw = stream.read(limit + 1)
    if len(raw) > limit:
        raise ValueError(f"evidence exceeds {limit} bytes: {path}")
    return raw, {"path": path.relative_to(root).as_posix(), "size_bytes": len(raw),
                 "sha256": hashlib.sha256(raw).hexdigest()}


def load_json(raw: bytes) -> Any:
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"duplicate JSON key: {key}")
            result[key] = value
        return result
    return json.loads(raw, object_pairs_hook=unique)


def instruction(row: dict | None) -> dict | None:
    if row is None:
        return None
    insn = row.get("instruction")
    if not isinstance(insn, dict):
        return None
    # Parts carry relocations and register text; retain one bounded row, not the
    # thousands of repeated instruction/relocation objects in the full report.
    text = "".join(str(p.get("text", "")) for p in insn.get("parts", []) if isinstance(p, dict))
    result = {key: insn[key] for key in ("address", "size", "opcode", "formatted") if key in insn}
    if text:
        result["text"] = text[:512]
    return result


def relocation_key(row: dict, symbols: list) -> dict | None:
    insn = row.get("instruction") or {}
    reloc = insn.get("relocation")
    if not isinstance(reloc, dict) or reloc.get("type_name") == "R_PPC_NONE":
        return None
    index = reloc.get("target_symbol")
    if not isinstance(index, int) or isinstance(index, bool) or not 0 <= index < len(symbols):
        raise ValueError("invalid relocation target in current report")
    return {"symbol": symbols[index].get("name"), "type": reloc.get("type_name", reloc.get("type")),
            "addend": reloc.get("addend", 0)}


def summarize(document: dict, label: str) -> list[dict]:
    if not isinstance(document, dict):
        raise ValueError(f"{label} report must be a JSON object")
    left = focus._symbols(document, "left", label)
    right = focus._symbols(document, "right", label)
    if not any(focus._is_function(symbol) for symbol in left):
        raise ValueError(f"{label} report has no target functions")
    result = []
    seen = set()
    for symbol in left:
        if not focus._is_function(symbol):
            continue
        name = symbol.get("name")
        if not isinstance(name, str) or name in seen:
            raise ValueError(f"ambiguous {label} function: {name!r}")
        seen.add(name)
        index = symbol.get("target_symbol")
        candidate = right[index] if isinstance(index, int) and not isinstance(index, bool) and 0 <= index < len(right) else None
        if candidate is not None and candidate.get("name") != name:
            raise ValueError(f"{label} paired name differs for {name}")
        if candidate is not None and not focus._is_function(candidate):
            raise ValueError(f"{label} candidate is not a function: {name}")
        rows = focus._rows(symbol, name)
        other_rows = focus._rows(candidate, name) if candidate else []
        differences = [(i, rows[i] if i < len(rows) else {}) for i in range(max(len(rows), len(other_rows)))
                       if ((rows[i].get("diff_kind") if i < len(rows) else None)
                           or (other_rows[i].get("diff_kind") if i < len(other_rows) else None))
                       not in (None, "", "DIFF_NONE", "NONE")]
        first = None
        if differences:
            i, row = differences[0]
            first = {"row": i, "kind": row.get("diff_kind") or other_rows[i].get("diff_kind"), "target": instruction(row),
                     "candidate": instruction(other_rows[i]) if i < len(other_rows) else None}
        relocations = []
        for i, row in differences:
            target_key = relocation_key(row, left)
            candidate_key = relocation_key(other_rows[i], right) if i < len(other_rows) else None
            if target_key is not None and candidate_key is not None and target_key != candidate_key:
                entry = {"target": target_key, "candidate": candidate_key}
                if entry not in relocations:
                    relocations.append(entry)
        percent = symbol.get("match_percent")
        exact = (candidate is not None and focus._is_function(candidate) and percent == 100 and not differences
                 and focus._integer(symbol.get("size")) is not None
                 and focus._integer(symbol.get("size")) == focus._integer(candidate.get("size"))
                 and focus._instruction_count(rows) == focus._instruction_count(other_rows))
        result.append({"function": name, "target_bytes": symbol.get("size"),
                       "candidate_bytes": candidate.get("size") if candidate else None,
                       "pair_index": index, "target_instruction_count": focus._instruction_count(rows),
                       "candidate_instruction_count": focus._instruction_count(other_rows),
                       "match_percent": percent, "diff_rows": len(differences),
                       "instruction_exact": bool(exact), "relocation_keys": relocations,
                       "unlocated_mismatch": None if exact or first else (
                           "unpaired target function" if candidate is None else
                           "score/size/count mismatch without a differing instruction row"),
                       "first_mismatch": first})
    return result


def candidate_only(document: dict, metrics: list[dict]) -> list[dict]:
    paired = {item["pair_index"] for item in metrics}
    return [{"function": symbol.get("name"), "bytes": symbol.get("size"), "index": i}
            for i, symbol in enumerate(focus._symbols(document, "right", "report"))
            if focus._is_function(symbol) and i not in paired]


def snapshot(*, root: Path, owner: str, source: Path, target: Path, candidate: Path,
             strict: Path, data: Path | None, toolchain_key: str,
             compile_receipt: Path | None = None) -> dict:
    root = Path(os.path.abspath(root))
    if not owner or not toolchain_key:
        raise ValueError("owner and toolchain key are required")
    inputs = {}
    for role, path, limit in (("source", source, 4*1024*1024), ("target_object", target, 16*1024*1024),
                              ("candidate_object", candidate, 16*1024*1024)):
        _, inputs[role] = read_bound(root, path, limit)
    raw, inputs["strict_report"] = read_bound(root, strict, REPORT_LIMIT)
    document = load_json(raw)
    strict_rows = summarize(document, "strict")
    extra_functions = candidate_only(document, strict_rows)
    del raw, document
    data_rows = None
    if data is not None:
        if local(root, data) == local(root, strict):
            inputs["data_report"] = dict(inputs["strict_report"])
            data_rows = strict_rows
            data_extra = extra_functions
        else:
            raw, inputs["data_report"] = read_bound(root, data, REPORT_LIMIT)
            document = load_json(raw)
            data_rows = summarize(document, "data")
            data_extra = candidate_only(document, data_rows)
            del raw, document
        fields = ("function", "target_bytes", "candidate_bytes", "pair_index", "target_instruction_count", "candidate_instruction_count")
        if ([tuple(r[k] for k in fields) for r in strict_rows] != [tuple(r[k] for k in fields) for r in data_rows]
                or extra_functions != data_extra):
            raise ValueError("strict/data function census or object layout differs")
    binding = "not_supplied"
    if compile_receipt is not None:
        raw, inputs["compile_receipt"] = read_bound(root, compile_receipt, INDEX_LIMIT)
        receipt = load_json(raw)
        if not isinstance(receipt, dict):
            raise ValueError("compiler receipt must be a JSON object")
        if (receipt.get("schema") != "recovery_candidate_compile/v1"
                or receipt.get("source_sha256") != inputs["source"]["sha256"]
                or receipt.get("object_sha256") != inputs["candidate_object"]["sha256"]):
            raise ValueError("compiler receipt does not bind selected source/object")
        binding = "receipt_hashes_match"
    result = {"schema": SCHEMA, "owner": owner, "toolchain_key": toolchain_key,
              "authority_advanced": False, "report_binding": "caller_selected_diagnostic",
              "compile_binding": binding, "physical_exact": None, "linked_exact": None,
              "function_census_binding": "report_only_not_independently_verified",
              "candidate_only_functions": extra_functions,
              "inputs": inputs, "functions": strict_rows, "data_functions": data_rows,
              "summary": {"functions": len(strict_rows),
                          "strict_instruction_exact": sum(r["instruction_exact"] for r in strict_rows),
                          "data_instruction_exact": None if data_rows is None else sum(r["instruction_exact"] for r in data_rows)}}
    groups = {}
    for item in strict_rows:
        for pair in item["relocation_keys"]:
            key = canonical(pair).decode("utf-8")
            groups.setdefault(key, []).append(item["function"])
    result["shared_relocation_diagnostics"] = [
        {**json.loads(key), "functions": names, "cause_proven": False}
        for key, names in sorted(groups.items()) if len(names) > 1]
    result["index_sha256"] = hashlib.sha256(canonical(result)).hexdigest()
    if len(canonical(result)) + 1 > INDEX_LIMIT:
        raise ValueError("compact index exceeds 256 KiB; no output published")
    verify(root, result)
    return result


def verify(root: Path, index: dict) -> None:
    root = Path(os.path.abspath(root))
    if index.get("schema") != SCHEMA:
        raise ValueError("unsupported current evidence index")
    payload = {key: value for key, value in index.items() if key != "index_sha256"}
    if hashlib.sha256(canonical(payload)).hexdigest() != index.get("index_sha256"):
        raise ValueError("current evidence index digest differs")
    for role, desc in index["inputs"].items():
        _, actual = read_bound(root, Path(desc["path"]), REPORT_LIMIT)
        if actual != desc:
            raise ValueError(f"stale current evidence: {role} {desc['path']}; refresh from live source")


def publish(root: Path, path: Path, value: dict) -> None:
    root = Path(os.path.abspath(root))
    path = local(root, path)
    path.relative_to(root / "build")
    if any(local(root, Path(d["path"])) == path for d in value["inputs"].values()):
        raise ValueError("index output aliases evidence input")
    raw = canonical(value) + b"\n"
    if len(raw) > INDEX_LIMIT:
        raise ValueError("compact index exceeds 256 KiB")
    verify(root, value)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(raw)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp, path)
    finally:
        Path(temp).unlink(missing_ok=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    sub = parser.add_subparsers(dest="action", required=True)
    create = sub.add_parser("snapshot")
    for name in ("owner", "toolchain-key"):
        create.add_argument("--" + name, required=True)
    for name in ("source", "target-object", "candidate-object", "strict", "out"):
        create.add_argument("--" + name, type=Path, required=True)
    create.add_argument("--data", type=Path)
    create.add_argument("--compile-receipt", type=Path)
    check = sub.add_parser("verify")
    check.add_argument("index", type=Path)
    args = parser.parse_args(argv)
    try:
        root = Path(os.path.abspath(args.root))
        if args.action == "snapshot":
            value = snapshot(root=root, owner=args.owner, source=args.source, target=args.target_object,
                             candidate=args.candidate_object, strict=args.strict, data=args.data,
                             toolchain_key=args.toolchain_key, compile_receipt=args.compile_receipt)
            publish(root, args.out, value)
        else:
            raw, _ = read_bound(root, args.index, INDEX_LIMIT)
            value = load_json(raw)
            verify(root, value)
        print(json.dumps({"status": "current", "owner": value["owner"], **value["summary"],
                          "compile_binding": value["compile_binding"], "authority_advanced": False}, sort_keys=True))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"current evidence: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
