#!/usr/bin/env python3
"""Build a commit-pinned MP6 recovery snapshot for static hosting.

The builder is intentionally dependency-free and read-only with respect to the
source checkout.  It reads the committed configuration, split/symbol metadata,
progress publication, and source paths through Git object commands.  It never
executes ``configure.py`` or a game build and never needs a retail binary.

Usage::

    python tools/build_snapshot.py --repo /path/to/marioparty6 \
        --commit <full-40-character-commit> --metadata source-metadata.json \
        --output build

The output directory receives ``snapshot.json`` and ``snapshot-version.json``
only after every input and consistency check has passed.  The parser routines
are the pure, source-independent portions of ``outputs/viewer/snapshot.py``
adapted for a portable GitHub Actions invocation.
"""

from __future__ import annotations

import argparse
import ast
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List, Mapping, Optional, Sequence, Set, Tuple


_rel_spec = importlib.util.spec_from_file_location("mp6_rel_metadata", Path(__file__).with_name("rel_metadata.py"))
assert _rel_spec and _rel_spec.loader
_rel_metadata = importlib.util.module_from_spec(_rel_spec)
_rel_spec.loader.exec_module(_rel_metadata)
verified_empty_evidence = _rel_metadata.verified_empty_evidence


COMMIT_RE = re.compile(r"^[0-9a-fA-F]{40}$")
HASH_RE = re.compile(r"^[0-9a-fA-F]{40}$")
DTK_RE = re.compile(r"^v?([0-9]+(?:\.[0-9]+)*)$", re.IGNORECASE)


class SnapshotBuildError(RuntimeError):
    """Raised when a snapshot cannot be proven safe to publish."""


class MetadataError(SnapshotBuildError):
    """Raised when the reusable binary metadata is invalid."""


class ValidationError(SnapshotBuildError):
    """Raised when committed ownership cannot fit its known budgets."""


def _git(repo: Path, *arguments: str, input_bytes: Optional[bytes] = None) -> bytes:
    """Run a read-only Git query and return its stdout."""

    command = ["git", "-C", str(repo), *arguments]
    try:
        result = subprocess.run(
            command,
            input=input_bytes,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        detail = getattr(exc, "stderr", b"") or b""
        if isinstance(detail, bytes):
            detail = detail.decode("utf-8", "replace")
        raise SnapshotBuildError(
            "git query failed ({}): {}".format(" ".join(command), str(detail).strip())
        ) from exc
    return result.stdout


def _validate_commit(repo: Path, commit: str) -> str:
    if not COMMIT_RE.fullmatch(commit):
        raise SnapshotBuildError(
            "--commit must be a full 40-character commit id; got {!r}".format(commit)
        )
    normalized = commit.lower()
    object_type = _git(repo, "cat-file", "-t", normalized).decode("ascii", "replace").strip()
    if object_type != "commit":
        raise SnapshotBuildError("--commit does not name a commit: {}".format(commit))
    return normalized


def _show_text(repo: Path, commit: str, path: str, *, required: bool = True) -> Optional[str]:
    try:
        raw = _git(repo, "show", "{}:{}".format(commit, path))
    except SnapshotBuildError:
        if required:
            raise
        return None
    return raw.decode("utf-8", "replace")


def _read_committed(repo: Path, commit: str) -> Tuple[Dict[str, str], Set[str], Optional[str]]:
    """Read the configuration and all source paths from one commit only."""

    files: Dict[str, str] = {}
    for path in (
        "configure.py",
        "config/GP6E01/config.yml",
        "include/mgdata.inc",
        "progress/GP6E01.json",
    ):
        value = _show_text(repo, commit, path)
        assert value is not None
        files[path] = value

    # STATUS moved between repository revisions.  Reading whichever committed
    # form exists keeps this audit useful without consulting a working tree.
    status = None
    for path in ("progress/STATUS.md", "STATUS.md", "progress/STATUS"):
        status = _show_text(repo, commit, path, required=False)
        if status is not None:
            files[path] = status
            break

    raw_paths = _git(repo, "ls-tree", "-r", "--name-only", commit, "--", "src")
    source_paths = {
        path.strip()
        for path in raw_paths.decode("utf-8", "replace").splitlines()
        if path.strip().startswith("src/")
    }
    return files, source_paths, status


def _text_hash(text: str) -> str:
    return hashlib.sha256(text.replace("\r\n", "\n").encode("utf-8")).hexdigest()


def _metric(matched: int, total: Optional[int]) -> Optional[dict]:
    if total is None:
        return None
    if not isinstance(matched, int) or isinstance(matched, bool):
        raise ValidationError("metric matched value is not an integer")
    if not isinstance(total, int) or isinstance(total, bool) or total < 0:
        raise ValidationError("metric total value is not a non-negative integer")
    if not 0 <= matched <= total:
        raise ValidationError("invalid byte count {}/{}".format(matched, total))
    return {
        "matched": matched,
        "total": total,
        "percent": round(matched * 100 / total, 6) if total else None,
    }


def configured_objects(source: str) -> Dict[str, str]:
    """Read literal Object entries under config.libs without executing Python."""

    try:
        tree = ast.parse(source)
    except SyntaxError as exc:
        raise SnapshotBuildError("configure.py is not valid Python: {}".format(exc)) from exc

    flags = {
        target.id: node.value
        for node in tree.body
        if isinstance(node, ast.Assign)
        for target in node.targets
        if isinstance(target, ast.Name) and target.id in ("Matching", "NonMatching")
    }
    if any(
        not isinstance(flags.get(key), ast.Constant) or flags[key].value is not value
        for key, value in (("Matching", True), ("NonMatching", False))
    ):
        raise SnapshotBuildError("source-selection flag definitions changed")

    assignments = [
        node
        for node in tree.body
        if isinstance(node, ast.Assign)
        and any(
            isinstance(target, ast.Attribute)
            and isinstance(target.value, ast.Name)
            and target.value.id == "config"
            and target.attr == "libs"
            for target in node.targets
        )
    ]
    if len(assignments) != 1 or not isinstance(assignments[0].value, ast.List):
        raise SnapshotBuildError("unsupported configure.py config.libs shape")

    result: Dict[str, str] = {}
    for branch in ast.walk(assignments[0].value):
        if isinstance(branch, (ast.IfExp, ast.ListComp, ast.SetComp, ast.GeneratorExp)) and any(
            isinstance(node, ast.Call)
            and isinstance(node.func, ast.Name)
            and node.func.id == "Object"
            for node in ast.walk(branch)
        ):
            raise SnapshotBuildError("conditional source entries require explicit support")

    for node in ast.walk(assignments[0].value):
        if not isinstance(node, ast.Call) or not isinstance(node.func, ast.Name) or node.func.id != "Object":
            continue
        if (
            len(node.args) < 2
            or not isinstance(node.args[1], ast.Constant)
            or not isinstance(node.args[1].value, str)
        ):
            raise SnapshotBuildError("unsupported Object path in committed configure.py")
        status = node.args[0].id if isinstance(node.args[0], ast.Name) else None
        if status not in ("Matching", "NonMatching", "Equivalent"):
            raise SnapshotBuildError("unsupported source-selection expression")
        path = node.args[1].value
        if path in result:
            raise SnapshotBuildError("duplicate configured source owner: {}".format(path))
        result[path] = "matching" if status == "Matching" else "remaining"

    # A mutation after config.libs would make the static AST census incomplete.
    start = tree.body.index(assignments[0])
    for node in tree.body[start + 1 :]:
        if isinstance(node, (ast.Assign, ast.AugAssign)):
            raise SnapshotBuildError("unexpected configuration mutation after config.libs")
        if any(
            isinstance(value, ast.Attribute)
            and value.attr == "libs"
            and isinstance(value.value, ast.Name)
            and value.value.id == "config"
            for value in ast.walk(node)
        ):
            raise SnapshotBuildError("unsupported access or mutation after config.libs")
    return result


def parse_modules(source: str) -> List[dict]:
    records: List[dict] = []
    current: Optional[dict] = None
    for line in source.splitlines():
        match = re.match(r"^(?:- )?object:\s*(\S+)\s*$", line)
        if match:
            current = {"object": match[1]}
            records.append(current)
        elif current is not None:
            match = re.match(r"^\s*(hash|symbols|splits):\s*(\S+)\s*$", line)
            if match:
                current[match[1]] = match[2]
    required = {"object", "hash", "symbols", "splits"}
    if not records or any(not required <= set(record) for record in records):
        raise SnapshotBuildError("incomplete committed module configuration")
    for index, record in enumerate(records):
        record["id"] = "main.dol" if index == 0 else Path(record["object"]).stem
    ids = [record["id"].lower() for record in records]
    if len(set(ids)) != len(ids):
        raise SnapshotBuildError("duplicate configured module")
    for record in records:
        if not HASH_RE.fullmatch(record["hash"]):
            raise SnapshotBuildError("invalid original object hash for {}".format(record["id"]))
    return records


def minigame_registry(source: str) -> Set[str]:
    cleaned = re.sub(
        r'"(?:\\.|[^"\\])*"|/\*.*?\*/|//[^\n]*',
        lambda match: match[0] if match[0].startswith('"') else " ",
        source,
        flags=re.S,
    )
    return {
        match.lower()
        for match in re.findall(r"\bDLL_(m\d+dll)\s*,\s*MG_TYPE_\w+", cleaned, re.I)
    }


def parse_splits(source: str) -> Dict[str, dict]:
    """Parse split ownership and reject overlapping committed ranges."""

    kinds: Dict[str, str] = {}
    owners: Dict[str, dict] = {}
    owner: Optional[str] = None
    all_ranges: Dict[str, List[Tuple[int, int]]] = {}
    for line in source.splitlines():
        match = re.match(r"\s*(\S+)\s+type:(\w+)", line)
        if match:
            section, kind = match[1], match[2]
            if section in kinds:
                raise ValidationError("duplicate split section {}".format(section))
            if kind not in {"code", "data", "bss", "rodata"}:
                raise ValidationError("unsupported split section type {} for {}".format(kind, section))
            # These two sections anchor the code/data interpretation used by
            # the budget checks.  A future config that silently relabels one
            # would otherwise turn a malformed split into plausible totals.
            if section == ".text" and kind != "code":
                raise ValidationError("canonical .text section must be type:code")
            if section == ".bss" and kind != "bss":
                raise ValidationError("canonical .bss section must be type:bss")
            kinds[section] = kind
            continue
        match = re.match(r"([^\s#].*):\s*$", line)
        if match:
            owner = match[1]
            if owner == "Sections":
                owner = None
                continue
            if owner in owners:
                raise ValidationError("duplicate split owner {}".format(owner))
            owners[owner] = {"code": 0, "data": 0, "bss": 0, "ranges": []}
            continue
        match = re.match(r"\s*(\S+)\s+start:(0x[\da-fA-F]+)\s+end:(0x[\da-fA-F]+)", line)
        if not match:
            continue
        section, start_text, end_text = match[1], match[2], match[3]
        start, end = int(start_text, 16), int(end_text, 16)
        if owner is None or section not in kinds or end < start:
            raise ValidationError("unsupported ownership range")
        kind = kinds[section]
        if kind == "code":
            owners[owner]["code"] += end - start
        elif kind == "bss":
            # The published data budget includes initialized data and BSS;
            # retain a separate BSS subtotal for the owner/module detail.
            owners[owner]["data"] += end - start
            owners[owner]["bss"] += end - start
        else:
            owners[owner]["data"] += end - start
        owners[owner]["ranges"].append((section, start, end))
        all_ranges.setdefault(section, []).append((start, end))

    if not kinds:
        raise ValidationError("split file contains no section definitions")
    for section, ranges in all_ranges.items():
        previous: Optional[int] = None
        for start, end in sorted(ranges):
            if previous is not None and start < previous:
                raise ValidationError("overlapping committed ownership ranges in {}".format(section))
            previous = end
    return owners


def parse_functions(source: str) -> List[dict]:
    result: List[dict] = []
    for line in source.splitlines():
        match = re.match(r"(\S+)\s*=\s*(\S+):(0x[\da-fA-F]+);\s*//\s*(.*)", line)
        if not match or not re.search(r"\btype:function\b", match[4]):
            continue
        size = re.search(r"\bsize:(0x[\da-fA-F]+|\d+)", match[4])
        result.append(
            {
                "name": match[1],
                "section": match[2],
                "offset": int(match[3], 16),
                "address": match[2] + ":" + match[3],
                "size": int(size[1], 0) if size else None,
            }
        )
    return result


def bind_functions(functions: Iterable[dict], owners: Mapping[str, dict], ranges: Mapping[str, dict]) -> List[dict]:
    unresolved: List[dict] = []
    for function in functions:
        cleaned = {key: value for key, value in function.items() if key not in ("section", "offset")}
        matches = [
            path
            for path, detail in ranges.items()
            for section, start, end in detail["ranges"]
            if section == function["section"]
            and start <= function["offset"] < end
            and function["size"] is not None
            and function["offset"] + function["size"] <= end
        ]
        if len(matches) == 1:
            selection = owners[matches[0]]["selection"]
            cleaned["state"] = selection if selection in ("matching", "remaining") else "unknown"
            owners[matches[0]]["functions"].append(cleaned)
        else:
            cleaned["state"] = "unknown"
            unresolved.append(cleaned)
    return unresolved


def _normalize_dtk(value: Any, label: str) -> str:
    if not isinstance(value, str) or not DTK_RE.fullmatch(value.strip()):
        raise MetadataError("{} must be a version such as 0.9.2".format(label))
    return value.strip().lstrip("vV")


def _load_metadata(path: Path) -> dict:
    try:
        document = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        raise MetadataError("cannot read metadata JSON {}: {}".format(path, exc)) from exc
    if not isinstance(document, dict) or document.get("schemaVersion") != 1:
        raise MetadataError("metadata schemaVersion must be 1")
    baseline = document.get("baselineCommit")
    if not isinstance(baseline, str) or not COMMIT_RE.fullmatch(baseline):
        raise MetadataError("metadata baselineCommit must be a full 40-character commit id")
    modules = document.get("modules")
    if not isinstance(modules, dict) or not modules:
        raise MetadataError("metadata modules must be a non-empty object")

    normalized: Dict[str, dict] = {}
    for module_id, value in modules.items():
        if not isinstance(module_id, str) or not module_id:
            raise MetadataError("metadata module id must be a non-empty string")
        key = module_id.lower()
        if key in normalized:
            raise MetadataError("metadata contains duplicate case-insensitive module id {}".format(module_id))
        if not isinstance(value, dict):
            raise MetadataError("metadata entry {} must be an object".format(module_id))
        original_hash = value.get("originalHash")
        if not isinstance(original_hash, str) or not HASH_RE.fullmatch(original_hash):
            raise MetadataError("metadata {} originalHash must be a 40-character SHA-1".format(module_id))
        dtk_version = _normalize_dtk(value.get("dtkVersion"), "metadata {} dtkVersion".format(module_id))
        budgets: Dict[str, int] = {}
        for field in ("code", "data"):
            budget = value.get(field)
            if not isinstance(budget, int) or isinstance(budget, bool) or budget < 0:
                raise MetadataError("metadata {} {} must be a non-negative integer".format(module_id, field))
            budgets[field] = budget
        aliases = value.get("aliases", [])
        if not isinstance(aliases, list) or any(not isinstance(alias, str) for alias in aliases):
            raise MetadataError("metadata {} aliases must be a string list".format(module_id))
        title = value.get("title", module_id)
        if not isinstance(title, str) or not title:
            raise MetadataError("metadata {} title must be a non-empty string".format(module_id))
        title_verified = value.get("titleVerified", False)
        if not isinstance(title_verified, bool):
            raise MetadataError("metadata {} titleVerified must be boolean".format(module_id))
        category = value.get("category", "system")
        if not isinstance(category, str) or not category:
            raise MetadataError("metadata {} category must be a non-empty string".format(module_id))
        provenance = value.get("provenance", {"evidence": "", "sources": []})
        if not isinstance(provenance, dict):
            raise MetadataError("metadata {} provenance must be an object".format(module_id))
        normalized[key] = {
            "id": module_id,
            "originalHash": original_hash.lower(),
            "dtkVersion": dtk_version,
            **budgets,
            "title": title,
            "titleVerified": title_verified,
            "category": category,
            "aliases": list(aliases),
            # Metadata is a reviewed public input.  JSON round-tripping here
            # prevents callers from mutating the source document in place.
            "provenance": json.loads(json.dumps(provenance, ensure_ascii=False)),
            "target": json.loads(json.dumps(value.get("target"))),
        }
    return {"schemaVersion": 1, "baselineCommit": baseline.lower(), "modules": normalized}


def _pinned_dtk_version(configure_source: str) -> str:
    matches = re.findall(
        r"^\s*config\.dtk_tag\s*=\s*([\"'])([^\"']+)\1\s*$",
        configure_source,
        flags=re.MULTILINE,
    )
    if len(matches) != 1:
        raise SnapshotBuildError("could not identify exactly one pinned config.dtk_tag")
    return _normalize_dtk(matches[0][1], "committed config.dtk_tag")


def _published_metric(value: Any, label: str) -> dict:
    if not isinstance(value, dict):
        raise SnapshotBuildError("published {} metric is missing".format(label))
    matched, total = value.get("matched"), value.get("total")
    if (
        not isinstance(matched, int)
        or isinstance(matched, bool)
        or not isinstance(total, int)
        or isinstance(total, bool)
        or total < 0
        or not 0 <= matched <= total
    ):
        raise SnapshotBuildError("published {} metric is invalid".format(label))
    # Preserve the repository's published percentage while validating the
    # authoritative integer pair.
    return {
        "matched": matched,
        "total": total,
        "percent": value.get("percent", round(matched * 100 / total, 6) if total else None),
    }


def _published_categories(progress_source: str) -> Dict[str, dict]:
    try:
        document = json.loads(progress_source)
    except ValueError as exc:
        raise SnapshotBuildError("committed progress/GP6E01.json is not valid JSON") from exc
    if not isinstance(document, dict) or document.get("schema_version") != 1:
        raise SnapshotBuildError("committed progress publication schema is unsupported")
    categories = document.get("categories")
    if not isinstance(categories, dict):
        raise SnapshotBuildError("committed progress publication has no categories")
    result: Dict[str, dict] = {}
    for key in ("all", "dol", "modules"):
        value = categories.get(key)
        if not isinstance(value, dict):
            raise SnapshotBuildError("committed progress publication is missing {}".format(key))
        result[key] = {
            "label": value.get("label", key),
            "code": _published_metric(value.get("code"), key + " code"),
            "data": _published_metric(value.get("data"), key + " data"),
        }
    return result


def _same_metric(left: Optional[dict], right: Optional[dict]) -> bool:
    if left is None or right is None:
        return False
    return left.get("matched") == right.get("matched") and left.get("total") == right.get("total")


def _aggregate(modules: Sequence[dict], label: str) -> dict:
    counts = {
        "total": len(modules),
        **{
            state: sum(module.get("state") == state for module in modules)
            for state in ("complete", "partial", "notRecovered", "empty", "unavailable")
        },
    }
    metrics = {
        field: _metric(
            sum(module[field]["matched"] for module in modules),
            sum(module[field]["total"] for module in modules),
        )
        if all(module.get(field) is not None for module in modules)
        else None
        for field in ("code", "data")
    }
    return {"label": label, **metrics, "counts": counts}


def _module_selection_paths(module_id: str, selections: Mapping[str, str]) -> Dict[str, str]:
    return {
        path: selection
        for path, selection in selections.items()
        if (module_id == "main.dol" and not path.startswith("REL/"))
        or path.startswith("REL/" + module_id + "/")
    }


def _source_name_exists(path: str, source_paths: Set[str]) -> bool:
    return path in source_paths or (not path.startswith("src/") and "src/" + path in source_paths)


def _assemble_module(
    record: Mapping[str, str],
    files: Mapping[str, str],
    source_paths: Set[str],
    selections: Mapping[str, str],
    metadata: Mapping[str, dict],
    registered: Set[str],
    pinned_dtk: str,
) -> dict:
    module_id = record["id"]
    lookup = module_id.lower()
    entry = metadata.get(lookup)
    hash_matches = entry is not None and entry["originalHash"] == record["hash"].lower()
    version_matches = entry is not None and entry["dtkVersion"] == pinned_dtk
    usable = bool(entry and hash_matches and version_matches)

    ranges = parse_splits(files[record["splits"]])
    selected_paths = _module_selection_paths(module_id, selections)
    owners: Dict[str, dict] = {}
    for path in dict.fromkeys([*ranges, *selected_paths]):
        selection = selections.get(path, "unowned")
        detail = ranges.get(path)
        if selection == "matching" and (not _source_name_exists(path, source_paths) or detail is None):
            selection = "unknown"
        owners[path] = {
            "path": path,
            "selection": selection,
            "code": _metric(detail["code"] if selection == "matching" else 0, detail["code"] if usable and detail else None)
            if usable and detail
            else None,
            "data": _metric(detail["data"] if selection == "matching" else 0, detail["data"] if usable and detail else None)
            if usable and detail
            else None,
            "bss": _metric(detail["bss"] if selection == "matching" else 0, detail["bss"] if usable and detail else None)
            if usable and detail
            else None,
            "functions": [],
        }

    unresolved = bind_functions(parse_functions(files[record["symbols"]]), owners, ranges)
    totals = {
        field: sum(detail[field] for detail in ranges.values())
        for field in ("code", "data", "bss")
    }
    if usable:
        for field in ("code", "data"):
            if totals[field] > entry[field]:
                raise ValidationError(
                    "{} split ranges exceed metadata {} budget ({}/{})".format(
                        module_id, field, totals[field], entry[field]
                    )
                )

    selected_bytes = {
        field: sum(
            detail[field]
            for path, detail in ranges.items()
            if owners[path]["selection"] == "matching"
        )
        for field in ("code", "data", "bss")
    }

    if usable:
        code = _metric(selected_bytes["code"], entry["code"])
        data = _metric(selected_bytes["data"], entry["data"])
        unowned = {
            field: entry[field] - totals[field]
            for field in ("code", "data")
        }
    else:
        code = data = None
        unowned = None

    if unresolved or (unowned and any(unowned.values())):
        owners["Unassigned ranges"] = {
            "path": "Unassigned ranges / original-object fallback",
            "selection": "unowned",
            "code": _metric(0, unowned["code"]) if unowned else None,
            "data": _metric(0, unowned["data"]) if unowned else None,
            "bss": None,
            "functions": unresolved,
        }

    selected_owner_count = sum(owner["selection"] == "matching" for owner in owners.values())
    all_owners_selected = all(owner["selection"] == "matching" for owner in owners.values())
    empty_evidence = None
    if usable and module_id != "main.dol" and not entry["code"] and not entry["data"] and not owners and not unresolved:
        empty_evidence = verified_empty_evidence(
            entry.get("target"), record["hash"], files[record["splits"]], files[record["symbols"]]
        )
    complete = bool(
        usable
        and code is not None
        and data is not None
        and (entry["code"] or entry["data"])
        and code["matched"] == code["total"]
        and data["matched"] == data["total"]
        and all_owners_selected
        and not unresolved
    )
    if not usable:
        state = "unavailable"
    elif empty_evidence:
        state = "empty"
    elif not entry["code"] and not entry["data"]:
        state = "unavailable"
    elif complete:
        state = "complete"
    elif selected_owner_count or (code and (code["matched"] or data["matched"])):
        state = "partial"
    else:
        state = "notRecovered"

    notes: List[str] = []
    if entry is None:
        notes.append("No reviewed binary-size metadata is available for this committed module.")
    elif not hash_matches:
        notes.append("Committed original-object hash does not match reviewed metadata; totals are withheld.")
    elif not version_matches:
        notes.append("Pinned DTK version does not match reviewed metadata; totals are withheld.")
    if not usable:
        notes.append("Owner ranges remain visible, but percentage and whole-module completion evidence is unavailable.")
    elif empty_evidence:
        notes.append("Hash-verified retail REL contains only null .ctors and .dtors linker tables: 0 functions and 0 recoverable code/data bytes. File and linker-table bytes are separate from recovery totals; percentages do not apply.")
    elif not entry["code"] and not entry["data"]:
        notes.append("Zero measured code/data alone does not establish an empty target. Verified target-structure evidence is unavailable.")

    normalized_registered = {value.lower() for value in registered}
    title_entry = entry or {
        "title": module_id,
        "titleVerified": False,
        "category": "system",
        "aliases": [],
        "provenance": {"evidence": "No reviewed human title is available; the module ID is retained.", "sources": []},
    }
    category = "minigame" if lookup in normalized_registered else title_entry.get("category", "system")
    return {
        "id": module_id,
        "title": title_entry.get("title", module_id),
        "titleVerified": bool(title_entry.get("titleVerified", False)),
        "category": category,
        "kind": "DOL" if module_id == "main.dol" else "REL",
        "aliases": list(title_entry.get("aliases", [])),
        "provenance": json.loads(json.dumps(title_entry.get("provenance", {}), ensure_ascii=False)),
        "code": code,
        "data": data,
        "bss": _metric(selected_bytes["bss"], totals["bss"]) if usable and all_owners_selected and not unresolved else None,
        "state": state,
        "emptyEvidence": empty_evidence,
        "owners": list(owners.values()),
        "notes": notes,
        "sizeEvidenceAvailable": usable,
    }


def _cleanup_builder() -> Any:
    path = Path(__file__).resolve().with_name("cleanup_index.py")
    spec = importlib.util.spec_from_file_location("mp6_cleanup_index", path)
    if spec is None or spec.loader is None:
        raise SnapshotBuildError("cannot load tools/cleanup_index.py")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module.build_cleanup


def _commit_stamp(repo: Path, commit: str) -> Tuple[str, str]:
    raw = _git(repo, "show", "-s", "--format=%cI%n%s", commit).decode("utf-8", "replace").split("\n", 1)
    if len(raw) != 2 or not raw[0] or not raw[1]:
        raise SnapshotBuildError("could not read committed timestamp and subject")
    return raw[0], raw[1]


def build_snapshot(
    repo: Path,
    commit: str,
    metadata_path: Path,
    generated_at: Optional[str] = None,
) -> dict:
    """Build an in-memory schemaVersion 1 snapshot from one commit."""

    commit = _validate_commit(repo, commit)
    metadata_document = _load_metadata(metadata_path)
    files, source_paths, _status = _read_committed(repo, commit)
    pinned_dtk = _pinned_dtk_version(files["configure.py"])
    records = parse_modules(files["config/GP6E01/config.yml"])
    for record in records:
        for field in ("symbols", "splits"):
            path = record[field]
            if path not in files:
                value = _show_text(repo, commit, path)
                assert value is not None
                files[path] = value
    selections = configured_objects(files["configure.py"])
    registered = minigame_registry(files["include/mgdata.inc"])
    if not registered:
        raise SnapshotBuildError("registered minigame census could not be read")

    modules = [
        _assemble_module(
            record,
            files,
            source_paths,
            selections,
            metadata_document["modules"],
            registered,
            pinned_dtk,
        )
        for record in records
    ]
    summary_computed = {
        key: _aggregate(
            [module for module in modules if condition(module)],
            label,
        )
        for key, label, condition in (
            ("overall", "Entire project", lambda module: True),
            ("dol", "Main executable · DOL", lambda module: module["kind"] == "DOL"),
            ("rel", "Loadable modules · REL", lambda module: module["kind"] == "REL"),
            ("minigames", "Registered minigames", lambda module: module["category"] == "minigame"),
            ("modes", "Modes", lambda module: module["category"] == "mode"),
            ("boards", "Boards", lambda module: module["category"] == "board"),
        )
    }

    published = _published_categories(files["progress/GP6E01.json"])
    summary = dict(summary_computed)
    reconciliation: Dict[str, bool] = {}
    for key, published_key in (("overall", "all"), ("dol", "dol"), ("rel", "modules")):
        computed = summary_computed[key]
        published_value = published[published_key]
        reconciliation[key] = _same_metric(computed.get("code"), published_value["code"]) and _same_metric(
            computed.get("data"), published_value["data"]
        )
        summary[key] = {
            **computed,
            "code": published_value["code"],
            "data": published_value["data"],
            "published": True,
        }

    stamp_time, subject = _commit_stamp(repo, commit)
    publication_commit = _git(
        repo,
        "log",
        "-1",
        "--format=%H",
        commit,
        "--",
        "progress/GP6E01.json",
    ).decode("ascii", "replace").strip()
    if not COMMIT_RE.fullmatch(publication_commit):
        raise SnapshotBuildError("could not identify the committed progress publication revision")
    generated_at = generated_at or datetime.now(timezone.utc).isoformat(timespec="seconds")
    cleanup = _cleanup_builder()(repo, commit, generated_at=generated_at)
    if cleanup.get("commit") != commit:
        raise SnapshotBuildError("cleanup index commit does not match snapshot commit")

    notes = [
        "Headlines reuse progress/GP6E01.json from this exact commit.",
        "Source-selected means the committed normal configuration marks an owner Matching. Original-object fallback earns no recovery credit.",
        "Complete requires every reviewed code/data byte and every source owner selected. Compiler translation units and split objects are never counted as whole modules.",
        "Verified empty RELs are counted separately: authenticated null linker tables contain zero functions and zero recoverable bytes. Their percentages are not applicable, and they add no completed implementation or minigame.",
        "Functions inherit committed source-owner selection. Unresolved symbol ranges remain unknown; this is not independent per-function objdiff proof.",
        "The builder reads committed Git objects and never executes configure.py, builds the game, or consults a developer checkout or retail binary.",
        "Reviewed module titles and provenance are supplied by the public metadata file; unresolved containers retain their IDs.",
        "Code/data budgets are reused only when the committed original-object hash and pinned DTK version match reviewed metadata. Split ownership may change without changing the original budget.",
    ]
    if not all(reconciliation.values()):
        notes.insert(0, "Computed detail totals do not fully reconcile with the committed progress publication; inspect unavailable or changed ownership details.")

    return {
        "schemaVersion": 1,
        "version": "GP6E01",
        "commit": commit,
        "commitSubject": subject,
        "committedAt": stamp_time,
        "refreshedAt": generated_at,
        "remote": "Committed main · GP6E01",
        "publicationCommit": publication_commit,
        "summary": summary,
        "modules": modules,
        "cleanup": cleanup,
        "notes": notes,
        "validation": {
            "reconciliation": reconciliation,
            "metadataSchemaVersion": metadata_document["schemaVersion"],
            "metadataBaselineCommit": metadata_document["baselineCommit"],
            "dtkVersion": pinned_dtk,
            "registeredMinigames": len(registered),
            "configuredModules": len(records),
            "unresolvedNames": sum(not module["titleVerified"] for module in modules),
            "missingSizeEvidence": sum(not module["sizeEvidenceAvailable"] for module in modules),
        },
    }


def _json_bytes(value: Any) -> bytes:
    return json.dumps(value, ensure_ascii=False, separators=(",", ":")).encode("utf-8")


def _write_artifacts(output: Path, snapshot_bytes: bytes, manifest_bytes: bytes) -> None:
    """Atomically replace the two generated files after validation."""

    try:
        output.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        raise SnapshotBuildError("cannot create output directory {}: {}".format(output, exc)) from exc
    pid = os.getpid()
    snapshot_temp = output / ".snapshot.json.{}.tmp".format(pid)
    manifest_temp = output / ".snapshot-version.json.{}.tmp".format(pid)
    try:
        snapshot_temp.write_bytes(snapshot_bytes)
        manifest_temp.write_bytes(manifest_bytes)
        os.replace(str(snapshot_temp), str(output / "snapshot.json"))
        os.replace(str(manifest_temp), str(output / "snapshot-version.json"))
    except OSError as exc:
        for temporary in (snapshot_temp, manifest_temp):
            try:
                temporary.unlink()
            except OSError:
                pass
        raise SnapshotBuildError("cannot replace generated snapshot artifacts: {}".format(exc)) from exc


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path, help="read-only repository containing the pinned source")
    parser.add_argument("--commit", required=True, help="full 40-character commit id to read")
    parser.add_argument("--metadata", required=True, type=Path, help="reviewed public source metadata JSON")
    parser.add_argument("--output", required=True, type=Path, help="directory for generated JSON artifacts")
    arguments = parser.parse_args(argv)
    try:
        generated_at = datetime.now(timezone.utc).isoformat(timespec="seconds")
        snapshot = build_snapshot(arguments.repo, arguments.commit, arguments.metadata, generated_at=generated_at)
        snapshot_bytes = _json_bytes(snapshot)
        manifest = {
            "schemaVersion": 1,
            "commit": snapshot["commit"],
            "revision": hashlib.sha256(snapshot_bytes).hexdigest(),
            "generatedAt": generated_at,
        }
        manifest_bytes = _json_bytes(manifest)
        _write_artifacts(arguments.output, snapshot_bytes, manifest_bytes)
    except SnapshotBuildError as exc:
        parser.error(str(exc))
        return 2
    print(
        json.dumps(
            {
                "commit": snapshot["commit"],
                "revision": manifest["revision"],
                "modules": len(snapshot["modules"]),
                "cleanupSites": len(snapshot["cleanup"].get("sites", [])),
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
