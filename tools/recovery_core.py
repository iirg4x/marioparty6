#!/usr/bin/env python3

"""SQLite indexing, bounded context, source-quality, and report operations."""

from __future__ import annotations

import fnmatch
import json
import re
import sqlite3
import subprocess
import tempfile
from collections import Counter
from pathlib import Path
from typing import Any

from tools.recovery_data import (
    DIMENSIONS, Function, RecoveryError, _mask_c, digest, function_text,
    includes, load, parse_functions, root_from, stable_id, token_estimate,
    validate_data,
)


def build_index(data: dict[str, Any], output: Path) -> dict[str, int]:
    root: Path = data["root"]
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=output.parent, prefix=output.name, suffix=".tmp", delete=False) as handle:
        temporary = Path(handle.name)
    counts = Counter()
    connection: sqlite3.Connection | None = None
    try:
        connection = sqlite3.connect(temporary)
        connection.executescript(
            """
            PRAGMA foreign_keys=ON;
            CREATE TABLE meta(key TEXT PRIMARY KEY, value TEXT NOT NULL);
            CREATE TABLE owners(id TEXT PRIMARY KEY, module TEXT, source TEXT UNIQUE, summary TEXT, compiler TEXT, binary TEXT, source_shape TEXT, semantics TEXT, naming TEXT, data TEXT, manifest TEXT);
            CREATE TABLE functions(stable_id TEXT PRIMARY KEY, owner_id TEXT, symbol TEXT, signature TEXT, start_line INTEGER, end_line INTEGER, FOREIGN KEY(owner_id) REFERENCES owners(id));
            CREATE INDEX functions_symbol ON functions(symbol);
            CREATE TABLE includes(owner_id TEXT, include_path TEXT);
            CREATE TABLE evidence(owner_id TEXT, kind TEXT, confidence TEXT, accepted INTEGER, summary TEXT, reference TEXT);
            CREATE TABLE debt(owner_id TEXT, kind TEXT, priority TEXT, summary TEXT);
            CREATE TABLE constraints(owner_id TEXT, exception_id TEXT);
            CREATE TABLE names(stable_id TEXT PRIMARY KEY, owner_id TEXT, current_symbol TEXT, proposed_name TEXT, status TEXT, confidence TEXT, summary TEXT);
            CREATE TABLE exceptions(id TEXT PRIMARY KEY, classification TEXT, path TEXT, kind TEXT, rules TEXT, rationale TEXT, evidence TEXT);
            CREATE TABLE patterns(id TEXT PRIMARY KEY, compiler TEXT, confidence TEXT, summary TEXT, conditions TEXT, examples TEXT, counterexamples TEXT, evidence TEXT);
            CREATE TABLE search(kind TEXT, key TEXT, owner_id TEXT, text TEXT);
            CREATE INDEX search_key ON search(key);
            """
        )
        for owner in data["owners"]:
            explicit: dict[str, str] = {}
            owner_id = str(owner["id"])
            status = owner["status"]
            connection.execute(
                "INSERT INTO owners VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                (owner_id, owner.get("module"), owner["source"], owner.get("summary", ""), owner.get("compiler"), *(status[item] for item in DIMENSIONS), owner.get("_manifest")),
            )
            counts["owners"] += 1
            text = (root / owner["source"]).read_text(encoding="utf-8")
            for item in owner.get("symbols", []):
                if isinstance(item, dict) and item.get("symbol") and item.get("stable_id"):
                    explicit[str(item["symbol"])] = str(item["stable_id"])
            for function in parse_functions(text):
                function_id = explicit.get(function.symbol, stable_id(owner, function.symbol))
                connection.execute("INSERT INTO functions VALUES(?,?,?,?,?,?)", (function_id, owner_id, function.symbol, function.signature, function.start, function.end))
                connection.execute("INSERT INTO search VALUES(?,?,?,?)", ("function", function_id, owner_id, f"{function.symbol} {function.signature}"))
                counts["functions"] += 1
            for include in includes(text):
                connection.execute("INSERT INTO includes VALUES(?,?)", (owner_id, include))
                counts["includes"] += 1
            for item in owner.get("evidence", []):
                connection.execute("INSERT INTO evidence VALUES(?,?,?,?,?,?)", (owner_id, item.get("kind"), item.get("confidence"), int(item.get("accepted", True)), item.get("summary", ""), item.get("reference")))
                counts["evidence"] += 1
            for item in owner.get("debt", []):
                connection.execute("INSERT INTO debt VALUES(?,?,?,?)", (owner_id, item.get("kind"), item.get("priority", "normal"), item.get("summary", "")))
                counts["debt"] += 1
            for item in owner.get("constraints", []):
                connection.execute("INSERT INTO constraints VALUES(?,?)", (owner_id, item))
            connection.execute("INSERT INTO search VALUES(?,?,?,?)", ("owner", owner_id, owner_id, f"{owner.get('summary', '')} {' '.join(owner.get('tags', []))}"))
        for item in data["names"]:
            connection.execute("INSERT INTO names VALUES(?,?,?,?,?,?,?)", (item.get("stable_id"), item.get("owner"), item.get("current_symbol"), item.get("proposed_name"), item.get("status"), item.get("confidence"), item.get("summary", "")))
            connection.execute("INSERT INTO search VALUES(?,?,?,?)", ("name", item.get("stable_id"), item.get("owner"), f"{item.get('current_symbol', '')} {item.get('proposed_name') or ''} {item.get('summary', '')}"))
            counts["names"] += 1
        for item in data["exceptions"]:
            connection.execute("INSERT INTO exceptions VALUES(?,?,?,?,?,?,?)", (item.get("id"), item.get("classification"), item.get("path"), item.get("kind"), json.dumps(item.get("rules", [])), item.get("rationale", ""), json.dumps(item.get("evidence", []))))
            connection.execute("INSERT INTO search VALUES(?,?,?,?)", ("exception", item.get("id"), None, f"{item.get('kind', '')} {item.get('rationale', '')}"))
            counts["exceptions"] += 1
        for item in data["patterns"]:
            connection.execute("INSERT INTO patterns VALUES(?,?,?,?,?,?,?,?)", (item.get("id"), item.get("compiler"), item.get("confidence"), item.get("summary", ""), item.get("conditions", ""), json.dumps(item.get("examples", [])), json.dumps(item.get("counterexamples", [])), json.dumps(item.get("evidence", []))))
            connection.execute("INSERT INTO search VALUES(?,?,?,?)", ("pattern", item.get("id"), None, f"{item.get('compiler', '')} {item.get('summary', '')} {item.get('conditions', '')}"))
            counts["patterns"] += 1
        source_paths = [root / owner["source"] for owner in data["owners"]]
        connection.execute("INSERT INTO meta VALUES('schema_version','1')")
        connection.execute("INSERT INTO meta VALUES('digest',?)", (digest([*data["metadata_paths"], *source_paths]),))
        connection.commit()
        connection.close()
        connection = None
        temporary.replace(output)
    except Exception:
        if connection is not None:
            connection.close()
        temporary.unlink(missing_ok=True)
        raise
    return dict(counts)


def query_index(database: Path, term: str, limit: int = 20) -> list[dict[str, Any]]:
    connection = sqlite3.connect(database)
    try:
        connection.row_factory = sqlite3.Row
        exact = connection.execute("SELECT kind,key,owner_id,text FROM search WHERE lower(key)=lower(?) LIMIT ?", (term, limit)).fetchall()
        rows = exact or connection.execute("SELECT kind,key,owner_id,text FROM search WHERE lower(key) LIKE lower(?) OR lower(text) LIKE lower(?) ORDER BY kind,key LIMIT ?", (f"%{term}%", f"%{term}%", limit)).fetchall()
    finally:
        connection.close()
    return [dict(row) for row in rows]


def _owner(data: dict[str, Any], owner_id: str) -> dict[str, Any]:
    matches = [item for item in data["owners"] if item.get("id") == owner_id]
    if len(matches) != 1:
        raise RecoveryError(f"owner {owner_id!r} was not found")
    return matches[0]


def _clip(text: str, budget: int) -> str:
    if token_estimate(text) <= budget:
        return text
    limit = max(0, budget * 4 - 100)
    return text[:limit].rstrip() + "\n\n[clipped to token budget]\n"


def context_pack(data: dict[str, Any], kind: str, target: str, *, owner_id: str | None = None, budget: int = 12000) -> str:
    root: Path = data["root"]
    project = data["project"]
    owner: dict[str, Any]
    function: Function | None = None
    if kind == "owner":
        owner = _owner(data, target)
    elif kind == "function":
        candidates: list[tuple[dict[str, Any], Function]] = []
        for item in data["owners"]:
            if owner_id and item.get("id") != owner_id:
                continue
            text = (root / item["source"]).read_text(encoding="utf-8")
            explicit = {entry.get("symbol"): entry.get("stable_id") for entry in item.get("symbols", []) if isinstance(entry, dict)}
            for parsed in parse_functions(text):
                parsed_id = explicit.get(parsed.symbol, stable_id(item, parsed.symbol))
                if target in {parsed.symbol, parsed_id}:
                    candidates.append((item, parsed))
        if len(candidates) != 1:
            raise RecoveryError(f"function {target!r} resolved to {len(candidates)} owners; pass --owner when ambiguous")
        owner, function = candidates[0]
    else:
        raise RecoveryError("context kind must be owner or function")
    owner_id_value = str(owner["id"])
    status = owner["status"]
    sections: list[str] = [
        "# Recovery context pack",
        f"- Target: `{target}`",
        f"- Owner: `{owner_id_value}`",
        f"- Source: `{owner['source']}`",
        f"- Approximate token budget: `{budget}`",
        "\n## Recovery contract\n" + "\n".join(f"- {item}" for item in project.get("agent_contract", [])),
        "\n## Owner state\n" + "\n".join(f"- {dimension}: `{status[dimension]}`" for dimension in DIMENSIONS) + f"\n\n{owner.get('summary', '')}",
    ]
    source_text = (root / owner["source"]).read_text(encoding="utf-8")
    parsed = parse_functions(source_text)
    if function:
        explicit = {entry.get("symbol"): entry.get("stable_id") for entry in owner.get("symbols", []) if isinstance(entry, dict)}
        function_id = explicit.get(function.symbol, stable_id(owner, function.symbol))
        sections.append(f"\n## Target function\n- Stable identity: `{function_id}`\n- Symbol: `{function.symbol}`\n- Lines: `{function.start}-{function.end}`\n- Signature: `{function.signature}`\n\n```c\n{function_text(source_text, function)}\n```")
        signatures = [f"- `{item.signature}`" for item in parsed if item.symbol != function.symbol][:40]
        sections.append("\n## Bounded owner neighbourhood\n" + ("\n".join(signatures) if signatures else "No other parsed functions."))
    else:
        signatures = [f"- `{item.signature}` — lines {item.start}-{item.end}" for item in parsed[:80]]
        sections.append("\n## Owner functions\n" + ("\n".join(signatures) if signatures else "No file-scope functions parsed."))
    accepted = [item for item in owner.get("evidence", []) if item.get("accepted", True)]
    rejected = [item for item in owner.get("evidence", []) if not item.get("accepted", True)]
    sections.append("\n## Accepted evidence\n" + ("\n".join(f"- [{item.get('kind')}/{item.get('confidence')}] {item.get('summary')} ({item.get('reference', 'no reference')})" for item in accepted) or "- None recorded."))
    sections.append("\n## Rejected evidence and probes\n" + ("\n".join(f"- [{item.get('kind')}/{item.get('confidence')}] {item.get('summary')} ({item.get('reference', 'no reference')})" for item in rejected) or "- None recorded."))
    constraints = {item.get("id"): item for item in data["exceptions"]}
    sections.append("\n## Authenticated constraints\n" + ("\n".join(f"- `{value}`: {constraints.get(value, {}).get('rationale', 'missing exception record')}" for value in owner.get("constraints", [])) or "- None recorded."))
    relevant_names = [item for item in data["names"] if item.get("owner") == owner_id_value and (not function or target in {item.get("stable_id"), item.get("current_symbol")} or item.get("current_symbol") == function.symbol)]
    sections.append("\n## Naming ledger\n" + ("\n".join(f"- `{item.get('stable_id')}` → `{item.get('proposed_name') or item.get('current_symbol')}` ({item.get('status')}, {item.get('confidence')}): {item.get('summary', '')}" for item in relevant_names) or "- No target-specific entry."))
    sections.append("\n## Remaining recovery debt\n" + ("\n".join(f"- [{item.get('priority', 'normal')}] {item.get('kind')}: {item.get('summary')}" for item in owner.get("debt", [])) or "- None recorded."))
    reports = owner.get("context", {}).get("reports", []) if isinstance(owner.get("context"), dict) else []
    sections.append("\n## Local reports\n" + ("\n".join(f"- `{path}` — {'available' if (root / path).is_file() else 'not present in this checkout'}" for path in reports) or "- None declared."))
    sections.append("\n## Acceptance criteria\n" + "\n".join(f"- {item}" for item in project.get("acceptance_criteria", [])))
    text = "\n".join(sections).strip() + "\n"
    return _clip(text, budget)


QUALITY_RULES: dict[str, tuple[str, re.Pattern[str]]] = {
    "compiler_pragma": ("compiler pragma requires evidence", re.compile(r"#\s*pragma\b")),
    "volatile": ("volatile may be a register-allocation control", re.compile(r"\bvolatile\b")),
    "register": ("register may be a register-allocation control", re.compile(r"\bregister\b")),
    "forced_inline": ("forced inline/no-inline requires evidence", re.compile(r"\b(?:NOINLINE|FORCEINLINE|FORCE_INLINE|never_inline|always_inline)\b", re.I)),
    "inline_asm": ("inline assembly requires original-assembly or target evidence", re.compile(r"\b(?:asm|__asm__)\s*(?:\(|\{)")),
    "include_guard_override": (
        "defining another header guard in C is a source-quality smell",
        re.compile(r"^\s*#\s*define\s+_[A-Z0-9_]+_(?:H|HPP)\b"),
    ),
    "self_assignment": (
        "self-assignment requires source-shape evidence",
        re.compile(r"(?<![A-Za-z0-9_.>])([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\1\s*;"),
    ),
    "synthetic_padding": (
        "named padding should be natural unless target-backed",
        re.compile(
            r"(?<![A-Za-z0-9])(?!(?-i:(?:PAD|PADDING|ALIGN|RESERVED)_[A-Z0-9_]+\b))(?:pad|padding|align|reserved)(?:_|\d)",
            re.I,
        ),
    ),
    "opaque_blob": (
        "opaque raw/tail/blob storage needs consumer analysis",
        re.compile(r"(?<![.>])\b(?:raw|tail|blob|opaque|unk)\w*\s*\[[^\]]+\]", re.I),
    ),
    "dead_branch": ("compile-time dead branches must not be codegen scaffolds", re.compile(r"^\s*#\s*if\s+0\b")),
    "raw_hex_literal": (
        "raw hexadecimal literals are forbidden in recovered C; use a named, evidence-backed constant",
        re.compile(r"(?<![A-Za-z0-9_])0[xX][0-9A-Fa-f]+(?:[uUlL]*)\b"),
    ),
}
C_SUFFIXES = {".c", ".cp", ".cc", ".cpp", ".cxx", ".h", ".hpp"}
HEADER_SUFFIXES = {".h", ".hpp"}
_HEADER_IFNDEF = re.compile(r"^\s*#\s*ifndef\s+([_A-Za-z][_A-Za-z0-9]*)\s*$")
_HEADER_DEFINE = re.compile(r"^\s*#\s*define\s+([_A-Za-z][_A-Za-z0-9]*)\s*$")


def canonical_header_guard_lines(path: str, text: str) -> set[int]:
    """Return the one matching leading guard define line allowed in a header."""
    suffix = Path(path).suffix.lower()
    if suffix not in HEADER_SUFFIXES:
        return set()
    stem = re.sub(r"[^A-Za-z0-9]+", "_", Path(path).stem).strip("_").upper()
    if not stem:
        return set()
    guard_suffixes = (f"{stem}_H", f"{stem}_HPP") if suffix == ".hpp" else (f"{stem}_H",)
    lines = _mask_c(text, preserve_preprocessor=True).splitlines()
    meaningful = [
        (line_number, value.strip())
        for line_number, value in enumerate(lines, 1)
        if value.strip()
    ]
    if len(meaningful) < 2:
        return set()
    ifndef = _HEADER_IFNDEF.fullmatch(meaningful[0][1])
    define = _HEADER_DEFINE.fullmatch(meaningful[1][1])
    if (
        not ifndef
        or not define
        or ifndef.group(1) != define.group(1)
        or not any(
            define.group(1).upper() == value
            or define.group(1).upper().endswith(f"_{value}")
            for value in guard_suffixes
        )
    ):
        return set()
    return {meaningful[1][0]}


def added_lines(root: Path, base: str) -> list[tuple[str, int, str]]:
    process = subprocess.run(["git", "diff", "--unified=0", f"{base}...HEAD", "--", "*.c", "*.cp", "*.h", "*.cc", "*.cpp", "*.cxx", "*.hpp"], cwd=root, text=True, capture_output=True, check=False)
    if process.returncode:
        raise RecoveryError(process.stderr.strip() or "git diff failed")
    path = ""
    line = 0
    result: list[tuple[str, int, str]] = []
    for raw in process.stdout.splitlines():
        if raw.startswith("+++ b/"):
            path = raw[6:]
        elif raw.startswith("@@"):
            match = re.search(r"\+(\d+)(?:,(\d+))?", raw)
            line = int(match.group(1)) if match else 0
        elif raw.startswith("+") and not raw.startswith("+++"):
            result.append((path, line, raw[1:]))
            line += 1
        elif raw.startswith(" "):
            line += 1
    return result


def quality_findings(data: dict[str, Any], *, base: str | None = None, full: bool = False) -> list[dict[str, Any]]:
    root: Path = data["root"]
    candidates: list[tuple[str, int, str]] = []
    allowed_guard_lines: dict[str, set[int]] = {}
    if base:
        changed = added_lines(root, base)
        by_path: dict[str, set[int]] = {}
        for path, line, _ in changed:
            by_path.setdefault(path, set()).add(line)
        for path, lines in by_path.items():
            file_path = root / path
            if file_path.suffix.lower() not in C_SUFFIXES or not file_path.is_file():
                continue
            original = file_path.read_text(encoding="utf-8", errors="replace")
            allowed_guard_lines[path] = canonical_header_guard_lines(path, original)
            masked = _mask_c(original, preserve_preprocessor=True).splitlines()
            for line in sorted(lines):
                value = masked[line - 1] if 0 < line <= len(masked) else ""
                candidates.append((path, line, value))
    elif full:
        roots = ((root / "src", C_SUFFIXES), (root / "include", HEADER_SUFFIXES))
        for source_root, suffixes in roots:
            if not source_root.is_dir():
                continue
            for path in sorted(source_root.rglob("*")):
                if path.suffix.lower() not in suffixes or not path.is_file():
                    continue
                relative = path.relative_to(root).as_posix()
                allowed_guard_lines[relative] = canonical_header_guard_lines(
                    relative,
                    path.read_text(encoding="utf-8", errors="replace"),
                )
                masked = _mask_c(
                    path.read_text(encoding="utf-8", errors="replace"),
                    preserve_preprocessor=True,
                )
                for line, value in enumerate(masked.splitlines(), 1):
                    candidates.append((relative, line, value))
    else:
        raise RecoveryError("quality scan requires a diff base or full=True")
    findings: list[dict[str, Any]] = []
    for path, line, text in candidates:
        for rule, (message, pattern) in QUALITY_RULES.items():
            if not pattern.search(text):
                continue
            if rule == "include_guard_override" and line in allowed_guard_lines.get(path, set()):
                continue
            matching = [item for item in data["exceptions"] if fnmatch.fnmatch(path, str(item.get("path", ""))) and rule in item.get("rules", [])]
            authenticated = next((item for item in matching if item.get("classification") == "authenticated"), None)
            if authenticated:
                continue
            temporary = next((item for item in matching if item.get("classification") == "temporary"), None)
            findings.append({"path": path, "line": line, "rule": rule, "message": message, "classification": "temporary" if temporary else "unreviewed", "exception": temporary.get("id") if temporary else None})
    return findings


def markdown_report(data: dict[str, Any]) -> str:
    owners = data["owners"]
    lines = ["# Recovery knowledge report", "", "This report tracks source quality independently from binary progress.", "", "## Owner matrix", "", "| Owner | Binary | Shape | Semantics | Naming | Data | Debt |", "| --- | --- | --- | --- | --- | --- | ---: |"]
    for owner in owners:
        status = owner["status"]
        lines.append(f"| `{owner['id']}` | {status['binary']} | {status['source_shape']} | {status['semantics']} | {status['naming']} | {status['data']} | {len(owner.get('debt', []))} |")
    lines += ["", "## Dimension counts", ""]
    for dimension in DIMENSIONS:
        counts = Counter(owner["status"][dimension] for owner in owners)
        lines.append(f"### {dimension.replace('_', ' ').title()}")
        lines.extend(f"- `{name}`: {count}" for name, count in sorted(counts.items()))
        lines.append("")
    lines += ["## Recovery debt", ""]
    debt_count = 0
    for owner in owners:
        for item in owner.get("debt", []):
            debt_count += 1
            lines.append(f"- `{owner['id']}` [{item.get('priority', 'normal')}] **{item.get('kind')}**: {item.get('summary')}")
    if not debt_count:
        lines.append("- None recorded.")
    lines += ["", "## Naming ledger", ""]
    lines.extend(f"- `{item.get('stable_id')}`: `{item.get('current_symbol')}` → `{item.get('proposed_name') or 'unresolved'}` ({item.get('status')}, {item.get('confidence')})" for item in data["names"])
    lines += ["", "## Authenticated and temporary source shapes", ""]
    lines.extend(f"- `{item.get('id')}` [{item.get('classification')}] `{item.get('path')}`: {item.get('rationale')}" for item in data["exceptions"])
    lines += ["", "## Compiler knowledge", ""]
    lines.extend(f"- `{item.get('id')}` [{item.get('compiler')}, {item.get('confidence')}]: {item.get('summary')} Conditions: {item.get('conditions', 'not recorded')}" for item in data["patterns"])
    return "\n".join(lines).rstrip() + "\n"
