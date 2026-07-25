#!/usr/bin/env python3
"""Generate an operational owner/dependency catalog without semantic claims."""

from __future__ import annotations

import argparse
import ast
import json
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Iterable, Mapping

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".s"}


class CatalogError(ValueError):
    pass


def _name(node: ast.AST | None) -> str | None:
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Attribute):
        return node.attr
    return None


def _literal(node: ast.AST | None) -> str | None:
    return node.value if isinstance(node, ast.Constant) and isinstance(node.value, str) else None


def _object_record(call: ast.Call, rel: str | None) -> dict[str, Any] | None:
    if _name(call.func) != "Object" or len(call.args) < 2:
        return None
    path = _literal(call.args[1])
    if not path:
        return None
    status = _name(call.args[0]) or "Unknown"
    source = path if path.startswith("src/") else f"src/{path}"
    logical = path.rsplit(".", 1)[0]
    if rel:
        prefix = f"REL/{rel}/"
        logical = logical[len(prefix):] if logical.startswith(prefix) else logical
        owner_id = f"REL:{rel}:{logical}"
        module = rel
    else:
        owner_id = f"main:{logical}"
        module = "main"
    return {
        "id": owner_id,
        "module": module,
        "object": path,
        "source": source,
        "configured_status": status,
    }


def _walk(node: ast.AST, rel: str | None, records: list[dict[str, Any]]) -> None:
    if isinstance(node, ast.Call) and _name(node.func) == "Rel":
        rel_name = _literal(node.args[0]) if node.args else None
        for child in [*node.args[1:], *(kw.value for kw in node.keywords)]:
            _walk(child, rel_name or rel, records)
        return
    if isinstance(node, ast.Call):
        record = _object_record(node, rel)
        if record:
            records.append(record)
    for child in ast.iter_child_nodes(node):
        _walk(child, rel, records)


def _resolve_include(root: Path, source: Path, include: str) -> str:
    candidates = [
        root / "include" / include,
        source.parent / include,
        root / include,
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate.relative_to(root).as_posix()
    return f"include/{include}"


def _merge_reviewed(
    records: list[dict[str, Any]], reviewed: Iterable[Mapping[str, Any]] | None
) -> None:
    by_source = {item["source"]: item for item in records}
    for owner in reviewed or []:
        source = owner.get("source")
        if not isinstance(source, str):
            continue
        target = by_source.get(source)
        if target:
            target["reviewed_owner_id"] = owner.get("id")
            target["compiler"] = owner.get("compiler")
            target["recovery_status"] = owner.get("status")
            target["tags"] = owner.get("tags", [])


def build_catalog(
    root: str | Path,
    reviewed: Iterable[Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    repo = Path(root).resolve()
    configure = repo / "configure.py"
    if not configure.is_file():
        raise CatalogError(f"missing {configure}")
    try:
        tree = ast.parse(configure.read_text(encoding="utf-8"), filename=str(configure))
    except SyntaxError as exc:
        raise CatalogError(f"configure.py is not parseable: {exc}") from exc
    records: list[dict[str, Any]] = []
    _walk(tree, None, records)
    unique: dict[str, dict[str, Any]] = {}
    for record in records:
        unique.setdefault(record["id"], record)
    records = sorted(unique.values(), key=lambda item: item["id"])

    configured_sources = {item["source"] for item in records}
    for source in sorted((repo / "src").rglob("*")):
        if not source.is_file() or source.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        relative = source.relative_to(repo).as_posix()
        if relative in configured_sources:
            continue
        logical = relative[len("src/") :].rsplit(".", 1)[0]
        records.append(
            {
                "id": f"unconfigured:{logical}",
                "module": "unconfigured",
                "object": None,
                "source": relative,
                "configured_status": "Unconfigured",
            }
        )

    _merge_reviewed(records, reviewed)
    header_consumers: dict[str, list[str]] = defaultdict(list)
    for record in records:
        source = repo / record["source"]
        text = source.read_text(encoding="utf-8", errors="replace") if source.is_file() else ""
        includes = sorted(
            {
                _resolve_include(repo, source, include)
                for include in INCLUDE_RE.findall(text)
            }
        )
        record["includes"] = includes
        record["size_bytes"] = source.stat().st_size if source.is_file() else 0
        record["exists"] = source.is_file()
        for include in includes:
            header_consumers[include].append(record["id"])

    for record in records:
        dependencies: set[str] = set()
        for include in record["includes"]:
            for candidate in records:
                if candidate["source"] == include:
                    dependencies.add(candidate["id"])
        record["depends_on_owners"] = sorted(dependencies)
    return {
        "schema_version": 1,
        "owners": sorted(records, key=lambda item: item["id"]),
        "header_consumers": {
            header: sorted(set(consumers))
            for header, consumers in sorted(header_consumers.items())
        },
    }


def write_catalog(catalog: Mapping[str, Any], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(catalog, indent=2) + "\n", encoding="utf-8")


def find_owner(catalog: Mapping[str, Any], query: str) -> list[dict[str, Any]]:
    exact = [
        dict(item)
        for item in catalog.get("owners", [])
        if query in {item.get("id"), item.get("source"), item.get("object")}
    ]
    if exact:
        return exact
    lowered = query.lower()
    return [
        dict(item)
        for item in catalog.get("owners", [])
        if lowered in json.dumps(item, sort_keys=True).lower()
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    sub = parser.add_subparsers(dest="command", required=True)
    build = sub.add_parser("build")
    build.add_argument("--output", default="build/context/owner-catalog.json")
    query = sub.add_parser("query")
    query.add_argument("value")
    query.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        catalog = build_catalog(args.root)
        if args.command == "build":
            destination = Path(args.root).resolve() / args.output
            write_catalog(catalog, destination)
            print(f"wrote {destination}: {len(catalog['owners'])} owners")
            return 0
        matches = find_owner(catalog, args.value)
        if args.json:
            print(json.dumps(matches, indent=2))
        else:
            for item in matches:
                print(
                    f"{item['id']}  {item['configured_status']}  "
                    f"{item['source']}  {item['size_bytes']} bytes"
                )
        return 0 if matches else 1
    except CatalogError as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
