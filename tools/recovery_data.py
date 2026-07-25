#!/usr/bin/env python3
"""Recovery metadata validation and lightweight C source parsing.

Only the Python standard library is used. Human-authored JSON under
``config/recovery`` is authoritative; generated SQLite and Markdown live under
``build/context`` and are disposable.
"""

from __future__ import annotations

import hashlib
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

PROJECT = Path("config/recovery/project.json")
REL_FN = re.compile(r"^fn_\d+_([0-9A-Fa-f]+)$")
DOL_FN = re.compile(r"^fn_(8[0-9A-Fa-f]{7})$")
IDENT = re.compile(r"[A-Za-z_]\w*")
DIMENSIONS = ("binary", "source_shape", "semantics", "naming", "data")


class RecoveryError(ValueError):
    pass


@dataclass(frozen=True)
class Function:
    symbol: str
    signature: str
    start: int
    end: int


def root_from(value: str | Path | None = None) -> Path:
    path = Path(value or Path.cwd()).resolve()
    if path.is_file():
        path = path.parent
    for candidate in (path, *path.parents):
        if (candidate / PROJECT).is_file():
            return candidate
    raise RecoveryError(f"could not find {PROJECT}")


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise RecoveryError(f"missing metadata: {path}") from exc
    except json.JSONDecodeError as exc:
        raise RecoveryError(
            f"invalid JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc


def _collection(root: Path, relative: str, key: str) -> list[dict[str, Any]]:
    value = read_json(root / relative)
    if isinstance(value, dict):
        value = value.get(key, [])
    if not isinstance(value, list) or not all(isinstance(item, dict) for item in value):
        raise RecoveryError(f"{relative}: expected a list of objects under {key!r}")
    return value


def load(root: str | Path | None = None, *, validate: bool = True) -> dict[str, Any]:
    repo = root_from(root)
    project = read_json(repo / PROJECT)
    if not isinstance(project, dict):
        raise RecoveryError(f"{PROJECT}: expected an object")
    globs = project.get("owner_globs", ["config/recovery/owners/*.json"])
    if not isinstance(globs, list) or not all(isinstance(item, str) for item in globs):
        raise RecoveryError("project.owner_globs must be a list of strings")
    owner_paths = sorted({path for pattern in globs for path in repo.glob(pattern) if path.is_file()})
    owners: list[dict[str, Any]] = []
    for path in owner_paths:
        item = read_json(path)
        if not isinstance(item, dict):
            raise RecoveryError(f"{path.relative_to(repo)}: expected an object")
        item = dict(item)
        item["_manifest"] = path.relative_to(repo).as_posix()
        owners.append(item)
    files = project.get("files", {})
    if not isinstance(files, dict):
        raise RecoveryError("project.files must be an object")
    names_path = str(files.get("names", "config/recovery/names.json"))
    exceptions_path = str(files.get("exceptions", "config/recovery/exceptions.json"))
    patterns_path = str(files.get("compiler_patterns", "config/recovery/compiler_patterns.json"))
    data = {
        "root": repo,
        "project": project,
        "owners": sorted(owners, key=lambda item: str(item.get("id", ""))),
        "names": _collection(repo, names_path, "names"),
        "exceptions": _collection(repo, exceptions_path, "exceptions"),
        "patterns": _collection(repo, patterns_path, "patterns"),
        "metadata_paths": [repo / PROJECT, repo / names_path, repo / exceptions_path, repo / patterns_path, *owner_paths],
    }
    errors = validate_data(data)
    if validate and errors:
        raise RecoveryError("recovery metadata validation failed:\n- " + "\n- ".join(errors))
    data["validation_errors"] = errors
    return data


def validate_data(data: dict[str, Any]) -> list[str]:
    root: Path = data["root"]
    project = data["project"]
    owners = data["owners"]
    names = data["names"]
    exceptions = data["exceptions"]
    patterns = data["patterns"]
    errors: list[str] = []
    if project.get("schema_version") != 1:
        errors.append("project.schema_version must be 1")
    dimensions = project.get("dimensions")
    if not isinstance(dimensions, dict):
        dimensions = {}
        errors.append("project.dimensions must be an object")
    allowed: dict[str, set[str]] = {}
    for dimension in DIMENSIONS:
        values = dimensions.get(dimension)
        if not isinstance(values, list) or not all(isinstance(item, str) for item in values):
            errors.append(f"project.dimensions.{dimension} must be a list of strings")
            values = []
        allowed[dimension] = set(values)
    hierarchy = project.get("evidence_hierarchy", [])
    if not isinstance(hierarchy, list):
        hierarchy = []
        errors.append("project.evidence_hierarchy must be a list")
    evidence_kinds = {item.get("id") for item in hierarchy if isinstance(item, dict)}
    confidence = set(project.get("confidence_levels", []))
    exception_ids = {item.get("id") for item in exceptions}
    owner_ids: set[str] = set()
    owner_sources: set[str] = set()
    for owner in owners:
        where = str(owner.get("_manifest", "owner"))
        owner_id = owner.get("id")
        source = owner.get("source")
        if owner.get("schema_version") != 1:
            errors.append(f"{where}: schema_version must be 1")
        if not isinstance(owner_id, str) or not owner_id:
            errors.append(f"{where}: missing id")
            continue
        if owner_id in owner_ids:
            errors.append(f"{where}: duplicate owner id {owner_id}")
        owner_ids.add(owner_id)
        if not isinstance(source, str) or not source:
            errors.append(f"{where}: missing source")
        elif source in owner_sources:
            errors.append(f"{where}: source owned twice: {source}")
        else:
            owner_sources.add(source)
            if not (root / source).is_file():
                errors.append(f"{where}: source does not exist: {source}")
        status = owner.get("status", {})
        if not isinstance(status, dict):
            errors.append(f"{where}: status must be an object")
            status = {}
        for dimension in DIMENSIONS:
            if status.get(dimension) not in allowed[dimension]:
                errors.append(f"{where}: invalid {dimension} status {status.get(dimension)!r}")
        for index, evidence in enumerate(owner.get("evidence", [])):
            if not isinstance(evidence, dict):
                errors.append(f"{where}.evidence[{index}] must be an object")
                continue
            if evidence.get("kind") not in evidence_kinds:
                errors.append(f"{where}.evidence[{index}]: unknown kind {evidence.get('kind')!r}")
            if evidence.get("confidence") not in confidence:
                errors.append(f"{where}.evidence[{index}]: unknown confidence")
            if not isinstance(evidence.get("summary"), str) or not evidence.get("summary"):
                errors.append(f"{where}.evidence[{index}]: missing summary")
            reference = evidence.get("reference")
            if isinstance(reference, str) and reference.startswith("docs/") and not (root / reference).is_file():
                errors.append(f"{where}.evidence[{index}]: missing {reference}")
        for constraint in owner.get("constraints", []):
            if constraint not in exception_ids:
                errors.append(f"{where}: unknown exception {constraint!r}")
        for index, debt in enumerate(owner.get("debt", [])):
            if not isinstance(debt, dict) or not debt.get("kind") or not debt.get("summary"):
                errors.append(f"{where}.debt[{index}]: kind and summary are required")
    stable_ids: set[str] = set()
    for index, entry in enumerate(names):
        where = f"names[{index}]"
        stable = entry.get("stable_id")
        if not isinstance(stable, str) or not stable:
            errors.append(f"{where}: missing stable_id")
        elif stable in stable_ids:
            errors.append(f"{where}: duplicate stable_id {stable}")
        else:
            stable_ids.add(stable)
        if entry.get("owner") not in owner_ids:
            errors.append(f"{where}: unknown owner {entry.get('owner')!r}")
        if entry.get("status") not in {"unresolved", "proposed", "accepted", "rejected"}:
            errors.append(f"{where}: invalid status")
        if entry.get("confidence") not in confidence:
            errors.append(f"{where}: invalid confidence")
    seen_exceptions: set[str] = set()
    for index, entry in enumerate(exceptions):
        where = f"exceptions[{index}]"
        value = entry.get("id")
        if not isinstance(value, str) or not value:
            errors.append(f"{where}: missing id")
        elif value in seen_exceptions:
            errors.append(f"{where}: duplicate id {value}")
        else:
            seen_exceptions.add(value)
        if entry.get("classification") not in {"authenticated", "temporary", "forbidden"}:
            errors.append(f"{where}: invalid classification")
        if not isinstance(entry.get("path"), str) or not entry.get("path"):
            errors.append(f"{where}: missing path")
        rules = entry.get("rules", [])
        if not isinstance(rules, list) or not all(isinstance(rule, str) for rule in rules):
            errors.append(f"{where}: rules must be a list of strings")
    seen_patterns: set[str] = set()
    for index, entry in enumerate(patterns):
        where = f"compiler_patterns[{index}]"
        value = entry.get("id")
        if not isinstance(value, str) or not value:
            errors.append(f"{where}: missing id")
        elif value in seen_patterns:
            errors.append(f"{where}: duplicate id {value}")
        else:
            seen_patterns.add(value)
        if entry.get("confidence") not in confidence:
            errors.append(f"{where}: invalid confidence")
        if not isinstance(entry.get("summary"), str) or not entry.get("summary"):
            errors.append(f"{where}: missing summary")
    return sorted(set(errors))


def _mask_c(text: str) -> str:
    out = list(text)
    i = 0
    state = "code"
    line_start = True
    directive = False
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if line_start:
                cursor = i
                while cursor < len(text) and text[cursor] in " \t\r":
                    cursor += 1
                directive = cursor < len(text) and text[cursor] == "#"
                line_start = False
            if directive:
                if ch == "\n":
                    directive = i > 0 and text[i - 1] == "\\"
                    line_start = not directive
                else:
                    out[i] = " "
                i += 1
                continue
            if ch == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                state = "line"
                i += 2
                continue
            if ch == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                state = "block"
                i += 2
                continue
            if ch in {'"', "'"}:
                out[i] = " "
                state = "string" if ch == '"' else "char"
                i += 1
                continue
            if ch == "\n":
                line_start = True
            i += 1
            continue
        if state == "line":
            if ch == "\n":
                state = "code"
                line_start = True
            else:
                out[i] = " "
            i += 1
            continue
        if state == "block":
            if ch == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                state = "code"
                i += 2
            else:
                if ch != "\n":
                    out[i] = " "
                else:
                    line_start = True
                i += 1
            continue
        if ch == "\\":
            out[i] = " "
            if i + 1 < len(text):
                if text[i + 1] != "\n":
                    out[i + 1] = " "
                i += 2
            else:
                i += 1
            continue
        if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
            out[i] = " "
            state = "code"
        elif ch != "\n":
            out[i] = " "
        else:
            line_start = True
        i += 1
    return "".join(out)


def parse_functions(text: str) -> list[Function]:
    masked = _mask_c(text)
    result: list[Function] = []
    start = 0
    i = 0
    while i < len(masked):
        if masked[i] == ";":
            start = i + 1
            i += 1
            continue
        if masked[i] != "{":
            i += 1
            continue
        prefix = masked[start:i]
        symbol: str | None = None
        if "=" not in prefix and not re.search(r"\btypedef\b", prefix):
            depth = 0
            opens: list[int] = []
            for position, char in enumerate(prefix):
                if char == "(":
                    if depth == 0:
                        opens.append(position)
                    depth += 1
                elif char == ")" and depth:
                    depth -= 1
            if opens and depth == 0:
                names = IDENT.findall(prefix[: opens[-1]])
                if names and names[-1] not in {"if", "for", "while", "switch", "sizeof"}:
                    symbol = names[-1]
        depth = 1
        end = i + 1
        while end < len(masked) and depth:
            depth += masked[end] == "{"
            depth -= masked[end] == "}"
            end += 1
        if depth:
            break
        if symbol:
            leading = len(prefix) - len(prefix.lstrip())
            source_start = start + leading
            signature = re.sub(r"\s+", " ", text[source_start:i].strip())
            result.append(Function(symbol, signature, text.count("\n", 0, source_start) + 1, text.count("\n", 0, end) + 1))
        start = end
        i = end
    return result


def includes(text: str) -> list[str]:
    return re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', text, re.MULTILINE)


def stable_id(owner: dict[str, Any], symbol: str) -> str:
    match = REL_FN.fullmatch(symbol)
    if match:
        return f"{owner.get('module', owner.get('id'))}:0x{match.group(1).upper()}"
    match = DOL_FN.fullmatch(symbol)
    if match:
        return f"main:0x{match.group(1).upper()}"
    return f"{owner.get('id')}:{symbol}"


def function_text(text: str, function: Function) -> str:
    return "\n".join(text.splitlines()[function.start - 1 : function.end])


def digest(paths: Iterable[Path]) -> str:
    value = hashlib.sha256()
    for path in sorted({item.resolve() for item in paths}, key=lambda item: item.as_posix()):
        value.update(path.as_posix().encode())
        value.update(b"\0")
        if path.is_file():
            value.update(path.read_bytes())
        value.update(b"\0")
    return value.hexdigest()


def token_estimate(text: str) -> int:
    return max(1, (len(text) + 3) // 4) if text else 0
