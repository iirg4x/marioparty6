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

from tools.check_tu_declarations import (
    IDENT_RE,
    _direct_declarations,
    _file_scope_items,
    _mask_comments_and_literals,
    _splice_lines,
)
from tools.recovery_data import parse_functions

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)
CALL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".s"}
CALL_KEYWORDS = {"if", "for", "while", "switch", "sizeof", "return"}


class CatalogError(ValueError):
    pass


def _name(node: ast.AST | None) -> str | None:
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Attribute):
        return node.attr
    return None


def _literal(node: ast.AST | None) -> str | None:
    return (
        node.value
        if isinstance(node, ast.Constant) and isinstance(node.value, str)
        else None
    )


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
        logical = logical[len(prefix) :] if logical.startswith(prefix) else logical
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


def _source_facts(text: str, suffix: str) -> dict[str, Any]:
    if suffix == ".s":
        return {
            "functions_defined": [],
            "globals_defined": [],
            "symbol_imports": [],
            "call_symbols": [],
            "tokens": set(),
        }
    functions = {function.symbol for function in parse_functions(text)}
    _, definitions = _file_scope_items(text)
    functions.update(str(item["symbol"]) for item in definitions)
    declarations = _direct_declarations(text)
    globals_defined = {
        str(item["symbol"])
        for item in declarations
        if item.get("kind") == "object" and not item.get("extern")
    }
    imports: list[dict[str, str]] = []
    for item in declarations:
        symbol = str(item.get("symbol"))
        kind = str(item.get("kind"))
        external = bool(item.get("extern")) or (
            kind == "function" and symbol not in functions
        )
        if external:
            imports.append({"symbol": symbol, "kind": kind})
    cleaned = _mask_comments_and_literals(_splice_lines(text))
    calls = {
        symbol
        for symbol in CALL_RE.findall(cleaned)
        if symbol not in CALL_KEYWORDS and symbol not in functions
    }
    return {
        "functions_defined": sorted(functions),
        "globals_defined": sorted(globals_defined),
        "symbol_imports": sorted(
            imports, key=lambda item: (item["symbol"], item["kind"])
        ),
        "call_symbols": sorted(calls),
        "tokens": set(IDENT_RE.findall(cleaned)),
    }


def _edge(symbol: str, owners: Mapping[str, list[str]]) -> dict[str, Any]:
    return {"symbol": symbol, "owners": sorted(set(owners.get(symbol, [])))}


def build_catalog(
    root: str | Path,
    reviewed: Iterable[Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    repo = Path(root).resolve()
    configure = repo / "configure.py"
    if not configure.is_file():
        raise CatalogError(f"missing {configure}")
    try:
        tree = ast.parse(
            configure.read_text(encoding="utf-8"), filename=str(configure)
        )
    except SyntaxError as exc:
        raise CatalogError(f"configure.py is not parseable: {exc}") from exc

    configured: list[dict[str, Any]] = []
    _walk(tree, None, configured)
    unique: dict[str, dict[str, Any]] = {}
    for record in configured:
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
    function_owners: dict[str, list[str]] = defaultdict(list)
    global_owners: dict[str, list[str]] = defaultdict(list)
    facts: dict[str, dict[str, Any]] = {}

    for record in records:
        source = repo / record["source"]
        text = (
            source.read_text(encoding="utf-8", errors="replace")
            if source.is_file()
            else ""
        )
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
        current = _source_facts(text, source.suffix.lower())
        facts[record["id"]] = current
        record["functions_defined"] = current["functions_defined"]
        record["globals_defined"] = current["globals_defined"]
        record["symbol_imports"] = current["symbol_imports"]
        record["call_symbols"] = current["call_symbols"]
        for symbol in current["functions_defined"]:
            function_owners[symbol].append(record["id"])
        for symbol in current["globals_defined"]:
            global_owners[symbol].append(record["id"])

    source_owner = {record["source"]: record["id"] for record in records}
    function_consumers: dict[str, list[str]] = defaultdict(list)
    data_consumers: dict[str, list[str]] = defaultdict(list)
    import_consumers: dict[str, list[str]] = defaultdict(list)

    for record in records:
        owner_id = record["id"]
        current = facts[owner_id]
        call_edges = [
            _edge(symbol, function_owners)
            for symbol in current["call_symbols"]
            if symbol in function_owners
        ]
        data_edges = [
            _edge(symbol, global_owners)
            for symbol in sorted(current["tokens"] & set(global_owners))
            if owner_id not in global_owners[symbol]
        ]
        import_edges: list[dict[str, Any]] = []
        for imported in current["symbol_imports"]:
            symbol = imported["symbol"]
            owners = (
                function_owners if imported["kind"] == "function" else global_owners
            )
            edge = {**imported, "owners": sorted(set(owners.get(symbol, [])))}
            import_edges.append(edge)
            import_consumers[symbol].append(owner_id)
        record["call_edges"] = call_edges
        record["data_edges"] = data_edges
        record["import_edges"] = import_edges
        dependencies: set[str] = {
            source_owner[include]
            for include in record["includes"]
            if include in source_owner
        }
        for edge in [*call_edges, *data_edges, *import_edges]:
            dependencies.update(
                dependency
                for dependency in edge.get("owners", [])
                if dependency != owner_id
            )
        record["depends_on_owners"] = sorted(dependencies)
        for edge in call_edges:
            function_consumers[edge["symbol"]].append(owner_id)
        for edge in data_edges:
            data_consumers[edge["symbol"]].append(owner_id)

    return {
        "schema_version": 2,
        "analysis_quality": {
            "includes": "exact direct source includes",
            "functions": "source definitions and direct call-token approximation",
            "globals": "file-scope declaration and identifier-reference approximation",
            "imports": "direct source declaration approximation",
            "semantic_claim": False,
        },
        "owners": sorted(records, key=lambda item: item["id"]),
        "header_consumers": {
            header: sorted(set(consumers))
            for header, consumers in sorted(header_consumers.items())
        },
        "function_owners": {
            symbol: sorted(set(owners))
            for symbol, owners in sorted(function_owners.items())
        },
        "function_consumers": {
            symbol: sorted(set(consumers))
            for symbol, consumers in sorted(function_consumers.items())
        },
        "global_owners": {
            symbol: sorted(set(owners))
            for symbol, owners in sorted(global_owners.items())
        },
        "data_consumers": {
            symbol: sorted(set(consumers))
            for symbol, consumers in sorted(data_consumers.items())
        },
        "symbol_import_consumers": {
            symbol: sorted(set(consumers))
            for symbol, consumers in sorted(import_consumers.items())
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
