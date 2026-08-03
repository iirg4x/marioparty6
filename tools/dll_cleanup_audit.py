#!/usr/bin/env python3
"""Audit reviewed DLL owners for mechanical cleanup debt.

This report deliberately makes no semantic or binary-recovery claims.  It uses
reviewed recovery owners as the positive non-minigame allowlist, excludes every
w0 module except w01Dll, and leaves every unreviewed module excluded when its
game ownership is uncertain.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import tempfile
from collections import defaultdict
from pathlib import Path
from typing import Any, Iterable, Mapping

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.owner_catalog import CatalogError, build_catalog
from tools.recovery_core import quality_findings
from tools.recovery_data import RecoveryError, _mask_c, load, root_from

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
W0_MODULE_RE = re.compile(r"^w0", re.IGNORECASE)
PLACEHOLDER_FILE_RE = re.compile(
    r"(?:^|[_-])(?:pass\d*|tail\d*|extra|shard)(?:[_-]|$)", re.IGNORECASE
)
ADDRESS_SHARD_RE = re.compile(r"_[0-9a-f]{3,}\.(?:c|cc|cpp|cxx)$", re.IGNORECASE)
ADDRESS_IDENTIFIER_RE = re.compile(
    r"\b(?:fn_\d+_[0-9A-Fa-f]+|"
    r"lbl_\d+_(?:data|bss|rodata|lit|text)_[0-9A-Fa-f]+|"
    r"jtbl_\d+_[0-9A-Fa-f]+)\b"
)
FUNCTION_POINTER_INTEGER_CAST_RE = re.compile(
    r"\(\s*[A-Za-z_]\w*\s*\)\s*"
    r"\(\s*(?:u32|uintptr_t|unsigned\s+long)\s*\)"
)


def reviewed_rel_modules(owners: Iterable[Mapping[str, Any]]) -> set[str]:
    """Return modules backed by a reviewed REL owner record."""

    result: set[str] = set()
    for owner in owners:
        owner_id = owner.get("id")
        module = owner.get("module")
        source = owner.get("source")
        if (
            isinstance(owner_id, str)
            and owner_id.startswith("REL:")
            and isinstance(module, str)
            and module
            and isinstance(source, str)
            and source.startswith("src/REL/")
        ):
            result.add(module)
    return result


def classify_module(module: str, reviewed: set[str]) -> tuple[str, str]:
    """Classify without guessing minigame ownership from a module name."""

    if W0_MODULE_RE.match(module) and module.casefold() != "w01dll":
        return "excluded_w0", "all w0 modules except w01Dll are out of scope"
    if module in reviewed:
        return (
            "eligible_reviewed",
            "module has a reviewed REL owner in the non-minigame recovery domain",
        )
    return (
        "uncertain_excluded",
        "no reviewed REL owner authenticates game ownership; excluded by policy",
    )


def forbidden_source_name(path: str) -> list[str]:
    """Return mechanical filename violations without judging owner semantics."""

    name = Path(path).name
    stem = Path(path).stem
    findings: list[str] = []
    if PLACEHOLDER_FILE_RE.search(stem):
        findings.append("placeholder_fragment")
    if ADDRESS_SHARD_RE.search(name):
        findings.append("address_shard")
    return findings


def address_identifiers(path: Path, relative: str) -> list[dict[str, Any]]:
    """Find address-derived identifiers while ignoring comments and literals."""

    masked = _mask_c(
        path.read_text(encoding="utf-8", errors="replace"),
        preserve_preprocessor=True,
    )
    result: list[dict[str, Any]] = []
    for line_number, line in enumerate(masked.splitlines(), 1):
        for match in ADDRESS_IDENTIFIER_RE.finditer(line):
            result.append(
                {
                    "path": relative,
                    "line": line_number,
                    "identifier": match.group(0),
                }
            )
    return result


def summarize_address_identifiers(
    findings: Iterable[Mapping[str, Any]],
) -> list[dict[str, Any]]:
    """Rank identifier debt by use count without making semantic claims."""

    grouped: dict[str, dict[str, list[int]]] = defaultdict(
        lambda: defaultdict(list)
    )
    for finding in findings:
        identifier = str(finding["identifier"])
        path = str(finding["path"])
        grouped[identifier][path].append(int(finding["line"]))
    result: list[dict[str, Any]] = []
    for identifier, paths in grouped.items():
        locations = [
            {"path": path, "lines": sorted(lines)}
            for path, lines in sorted(paths.items())
        ]
        result.append(
            {
                "identifier": identifier,
                "occurrences": sum(len(item["lines"]) for item in locations),
                "locations": locations,
            }
        )
    return sorted(
        result,
        key=lambda item: (-int(item["occurrences"]), str(item["identifier"])),
    )


def function_pointer_integer_casts(
    path: Path, relative: str
) -> list[dict[str, Any]]:
    """Find callback coercion ladders while ignoring comments and literals."""

    masked = _mask_c(
        path.read_text(encoding="utf-8", errors="replace"),
        preserve_preprocessor=True,
    )
    return [
        {
            "path": relative,
            "line": line_number,
            "rule": "function_pointer_integer_cast",
            "message": (
                "function pointers routed through an integer type require "
                "owner-specific evidence"
            ),
            "classification": "unreviewed",
        }
        for line_number, line in enumerate(masked.splitlines(), 1)
        if FUNCTION_POINTER_INTEGER_CAST_RE.search(line)
    ]


def _module_sources(catalog: Mapping[str, Any]) -> dict[str, list[dict[str, Any]]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for owner in catalog.get("owners", []):
        module = owner.get("module")
        source = owner.get("source")
        if (
            isinstance(module, str)
            and module not in {"main", "unconfigured"}
            and isinstance(source, str)
            and Path(source).suffix.lower() in SOURCE_SUFFIXES
        ):
            grouped[module].append(dict(owner))
    return {module: sorted(items, key=lambda item: str(item["source"])) for module, items in grouped.items()}


def build_cleanup_report(
    data: Mapping[str, Any],
    catalog: Mapping[str, Any],
    *,
    modules: Iterable[str] | None = None,
    source_root: str | Path | None = None,
) -> dict[str, Any]:
    """Build a deterministic DLL cleanup report from reviewed ownership."""

    metadata_root = Path(data["root"])
    root = Path(source_root).resolve() if source_root is not None else metadata_root
    if not root.is_dir():
        raise RecoveryError(f"source root does not exist: {root}")
    reviewed = reviewed_rel_modules(data.get("owners", []))
    grouped = _module_sources(catalog)
    requested = set(modules or grouped)
    selected_paths = sorted(
        str(item["source"])
        for module, items in grouped.items()
        if module in requested and classify_module(module, reviewed)[0] == "eligible_reviewed"
        for item in items
        if (root / str(item["source"])).is_file()
    )
    quality_data = dict(data)
    quality_data["root"] = root
    quality = quality_findings(quality_data, full=True) if selected_paths else []
    selected_set = {Path(path).as_posix() for path in selected_paths}
    quality_by_path: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for finding in quality:
        path = Path(str(finding["path"])).as_posix()
        if path in selected_set:
            quality_by_path[path].append(finding)

    records: list[dict[str, Any]] = []
    for module in sorted(requested):
        classification, reason = classify_module(module, reviewed)
        sources = grouped.get(module, [])
        filename_findings: list[dict[str, Any]] = []
        identifiers: list[dict[str, Any]] = []
        source_quality: list[dict[str, Any]] = []
        total_bytes = 0
        if classification == "eligible_reviewed":
            for item in sources:
                relative = str(item["source"])
                path = root / relative
                if not path.is_file():
                    continue
                total_bytes += path.stat().st_size
                for rule in forbidden_source_name(relative):
                    filename_findings.append({"path": relative, "rule": rule})
                identifiers.extend(address_identifiers(path, relative))
                source_quality.extend(quality_by_path.get(Path(relative).as_posix(), []))
                source_quality.extend(function_pointer_integer_casts(path, relative))
        unique_identifiers: list[dict[str, Any]] = []
        seen_identifiers: set[str] = set()
        for finding in identifiers:
            identifier = str(finding["identifier"])
            if identifier in seen_identifiers:
                continue
            seen_identifiers.add(identifier)
            unique_identifiers.append(finding)
        identifier_usage = summarize_address_identifiers(identifiers)
        records.append(
            {
                "module": module,
                "classification": classification,
                "reason": reason,
                "configured_sources": len(sources),
                "source_bytes": total_bytes,
                "filename_findings": filename_findings,
                "address_identifiers": identifiers,
                "unique_address_identifiers": unique_identifiers,
                "address_identifier_usage": identifier_usage,
                "source_quality_findings": source_quality,
                "actionable": bool(filename_findings or identifiers or source_quality),
            }
        )

    records.sort(
        key=lambda item: (
            item["classification"] != "eligible_reviewed",
            not item["actionable"],
            -len(item["filename_findings"]),
            -len(item["unique_address_identifiers"]),
            -len(item["address_identifiers"]),
            -len(item["source_quality_findings"]),
            -item["source_bytes"],
            item["module"].casefold(),
        )
    )
    eligible = [item for item in records if item["classification"] == "eligible_reviewed"]
    return {
        "schema_version": 1,
        "analysis_quality": {
            "semantic_claim": False,
            "binary_claim": False,
            "minigame_name_guess": False,
            "eligibility": "reviewed REL owner allowlist; uncertainty is excluded",
        },
        "metadata_root": str(metadata_root),
        "source_root": str(root),
        "summary": {
            "modules_seen": len(records),
            "eligible_reviewed": len(eligible),
            "actionable_eligible": sum(bool(item["actionable"]) for item in eligible),
            "clean_eligible": sum(not item["actionable"] for item in eligible),
            "excluded_w0": sum(item["classification"] == "excluded_w0" for item in records),
            "uncertain_excluded": sum(item["classification"] == "uncertain_excluded" for item in records),
        },
        "modules": records,
    }


def _write_atomic(root: Path, relative: str, report: Mapping[str, Any]) -> Path:
    destination = (root / relative).resolve()
    build_root = (root / "build").resolve()
    try:
        destination.relative_to(build_root)
    except ValueError as exc:
        raise RecoveryError(f"output must stay under {build_root}: {destination}") from exc
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w",
        encoding="utf-8",
        dir=destination.parent,
        prefix=destination.name,
        suffix=".tmp",
        delete=False,
    ) as handle:
        json.dump(report, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, destination)
    return destination


def _render(
    report: Mapping[str, Any],
    *,
    show_excluded: bool,
    top_identifiers: int = 0,
) -> str:
    summary = report["summary"]
    lines = [
        "DLL cleanup audit",
        (
            f"eligible={summary['eligible_reviewed']} "
            f"actionable={summary['actionable_eligible']} "
            f"clean={summary['clean_eligible']} "
            f"excluded-w0={summary['excluded_w0']} "
            f"uncertain-excluded={summary['uncertain_excluded']}"
        ),
        "module\tfiles\tbytes\tfilenames\tunique-address-ids\taddress-uses\tquality",
    ]
    for item in report["modules"]:
        if item["classification"] != "eligible_reviewed" and not show_excluded:
            continue
        if item["classification"] == "eligible_reviewed":
            lines.append(
                f"{item['module']}\t{item['configured_sources']}\t{item['source_bytes']}\t"
                f"{len(item['filename_findings'])}\t"
                f"{len(item['unique_address_identifiers'])}\t"
                f"{len(item['address_identifiers'])}\t"
                f"{len(item['source_quality_findings'])}"
            )
        else:
            lines.append(f"{item['module']}\t{item['classification']}\t{item['reason']}")
    if top_identifiers:
        lines.extend(
            [
                "",
                "highest-use address identifiers",
                "module\tidentifier\tuses\tfirst-location",
            ]
        )
        for item in report["modules"]:
            if item["classification"] != "eligible_reviewed":
                continue
            for usage in item["address_identifier_usage"][:top_identifiers]:
                first = usage["locations"][0]
                lines.append(
                    f"{item['module']}\t{usage['identifier']}\t"
                    f"{usage['occurrences']}\t{first['path']}:{first['lines'][0]}"
                )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    parser.add_argument(
        "--source-root",
        help="optional worker worktree whose src/ tree is audited with this root's metadata",
    )
    parser.add_argument("--module", action="append", dest="modules")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--show-excluded", action="store_true")
    parser.add_argument(
        "--top-identifiers",
        type=int,
        default=0,
        help="show this many highest-use address identifiers per eligible module",
    )
    parser.add_argument("--strict", action="store_true")
    parser.add_argument(
        "--output", default=None, help="optional JSON path under build/"
    )
    args = parser.parse_args()
    if args.top_identifiers < 0:
        parser.error("--top-identifiers must be non-negative")
    try:
        root = root_from(args.root)
        data = load(root)
        catalog = build_catalog(root, reviewed=data["owners"])
        report = build_cleanup_report(
            data,
            catalog,
            modules=args.modules,
            source_root=args.source_root,
        )
        if args.output:
            destination = _write_atomic(root, args.output, report)
            print(f"wrote {destination}")
        print(
            json.dumps(report, indent=2)
            if args.json
            else _render(
                report,
                show_excluded=args.show_excluded,
                top_identifiers=args.top_identifiers,
            )
        )
        return 1 if args.strict and report["summary"]["actionable_eligible"] else 0
    except (CatalogError, RecoveryError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
