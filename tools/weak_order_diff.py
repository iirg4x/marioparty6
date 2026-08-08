#!/usr/bin/env python3
"""Compare symbol ordering in two MP6 ELF/object files.

This is an MP6-native, read-only successor to the ordering helper used by
zeldaret/tp (``tools/utilities/weak_order_diff.py``,
https://github.com/zeldaret/tp).  The original helper
implicitly built a source object and used a repository-local readelf binary.
This utility deliberately accepts both input objects and the readelf program
explicitly (or resolves a conservative readelf name from ``PATH``); it never
invokes a build tool and never writes a source or build artifact.

The report is a diagnostic inventory.  A symbol/order difference can identify
an emitted-layout constraint, but it does not by itself authenticate a source
declaration order or justify moving a definition.
Compiler-local ``@N`` names are normalized to ``@``; repeated local-pool rows
therefore carry sequence and multiplicity evidence only, not identity-level
mapping between target and source pools.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from collections import Counter, deque
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Iterable, Mapping, Sequence


__all__ = [
    "CATEGORIES",
    "ReadelfError",
    "Symbol",
    "build_report",
    "compare_orders",
    "main",
    "normalize_symbol_name",
    "orders_from_symbols",
    "normalize_orders",
    "parse_readelf_symbols",
    "parse_symbols",
    "readelf_symbols",
    "render_human",
    "resolve_readelf",
]


CATEGORIES = ("functions", "data", "local_pools")
DEFAULT_READELF_NAMES = ("powerpc-eabi-readelf", "readelf")


class ReadelfError(RuntimeError):
    """Raised when readelf cannot be resolved or its output cannot be read."""


@dataclass(frozen=True)
class Symbol:
    """One row from an ELF symbol table.

    ``section`` is retained as the readelf section-index token because object
    files may use named indices (``UND``, ``ABS``) as well as numbers.
    """

    number: int
    value: int
    size: int
    symbol_type: str
    binding: str
    visibility: str
    section: str
    name: str


_SYMBOL_RE = re.compile(
    r"^\s*(?P<number>\d+):\s+"
    r"(?P<value>[0-9A-Fa-f]+)\s+"
    r"(?P<size>\d+)\s+"
    r"(?P<symbol_type>\S+)\s+"
    r"(?P<binding>\S+)\s+"
    r"(?P<visibility>\S+)\s+"
    r"(?P<section>\S+)"
    r"(?:\s+(?P<name>.*?))?\s*$"
)
_SYMBOL_TABLE_RE = re.compile(r"^\s*Symbol table '([^']+)' contains")
_LOCAL_POOL_RE = re.compile(r"^@\d+$")
_GCC_LOCAL_POOL_RE = re.compile(r"^\.LC(?:[0-9A-Za-z_.-]+)$")
_SYNTHETIC_DATA_RE = re.compile(
    r"^lbl_[0-9a-f]+_(?:data|bss)_[0-9a-f]+$", re.IGNORECASE
)
_SUFFIX_RE = re.compile(r"^(?P<prefix>\S+\$)\d+$")


def _parse_int(value: str, *, base: int = 10) -> int:
    try:
        return int(value, base)
    except ValueError:
        # A few readelf variants print a hexadecimal size with a 0x prefix.
        return int(value, 0)


def parse_readelf_symbols(output: str | bytes) -> list[Symbol]:
    """Parse symbol rows emitted by ``readelf -Ws``.

    Readelf adds version-specific headings and warnings around the table, so
    only rows matching the stable ``Num: Value Size Type Bind Vis Ndx Name``
    shape are consumed.  An empty table is valid and returns an empty list.
    """

    if isinstance(output, bytes):
        output = output.decode("utf-8", errors="replace")
    unscoped: list[Symbol] = []
    tables: dict[str, list[Symbol]] = {}
    current_table: str | None = None
    for line in output.splitlines():
        table = _SYMBOL_TABLE_RE.match(line)
        if table is not None:
            current_table = table.group(1)
            tables.setdefault(current_table, [])
            continue
        match = _SYMBOL_RE.match(line)
        if match is None:
            continue
        values = match.groupdict()
        name = (values.get("name") or "").strip()
        symbol = Symbol(
            number=_parse_int(values["number"]),
            value=_parse_int(values["value"], base=16),
            size=_parse_int(values["size"]),
            symbol_type=values["symbol_type"],
            binding=values["binding"],
            visibility=values["visibility"],
            section=values["section"],
            name=name,
        )
        if current_table is None:
            unscoped.append(symbol)
        else:
            tables[current_table].append(symbol)
    if ".symtab" in tables:
        return [*unscoped, *tables[".symtab"]]
    symbols: list[Symbol] = [*unscoped]
    for rows in tables.values():
        symbols.extend(rows)
    return symbols


# Short aliases make the parser convenient for small scripts and tests while
# retaining the descriptive public name above.
parse_symbols = parse_readelf_symbols


def normalize_symbol_name(name: str) -> str:
    """Normalize compiler-generated names for cross-object comparison.

    MWCC numbers literal-pool owners as ``@N`` and may number local COMDAT
    suffixes as ``name$N``.  The number is an emission-local identity rather
    than a stable source name, so the former becomes ``@`` and the latter keeps
    only its stem.  Other names remain untouched.
    """

    name = name.strip()
    if _LOCAL_POOL_RE.fullmatch(name):
        return "@"
    suffix = _SUFFIX_RE.fullmatch(name)
    if suffix is not None:
        return suffix.group("prefix")
    return name


def _section_sort_key(section: str) -> tuple[int, str]:
    if section.isdigit():
        return (0, f"{int(section):010d}")
    return (1, section)


def _symbol_sort_key(symbol: Symbol) -> tuple[tuple[int, str], int, str, int]:
    # Value is an object-relative offset.  Section index first avoids sorting a
    # later section's low offset ahead of an earlier section's high offset.
    return (_section_sort_key(symbol.section), symbol.value, symbol.name, symbol.number)


def _is_defined(symbol: Symbol) -> bool:
    return symbol.section not in {"UND", "UNDEF"}


def _category(symbol: Symbol) -> str | None:
    if not _is_defined(symbol) or symbol.visibility == "HIDDEN":
        return None
    if _SYNTHETIC_DATA_RE.fullmatch(symbol.name):
        return None
    if _LOCAL_POOL_RE.fullmatch(symbol.name) or _GCC_LOCAL_POOL_RE.fullmatch(
        symbol.name
    ):
        return "local_pools"
    if symbol.symbol_type == "FUNC":
        return "functions"
    if symbol.symbol_type in {"OBJECT", "COMMON", "TLS", "IFUNC"}:
        return "data"
    # FILE, SECTION, NOTYPE, and implementation-specific rows are metadata or
    # labels rather than stable data/function owners for this diagnostic.
    return None


def orders_from_symbols(symbols: Iterable[Symbol]) -> dict[str, list[str]]:
    """Return normalized physical order grouped by function/data/local pool."""

    orders: dict[str, list[str]] = {category: [] for category in CATEGORIES}
    for symbol in sorted(symbols, key=_symbol_sort_key):
        category = _category(symbol)
        if category is not None:
            orders[category].append(normalize_symbol_name(symbol.name))
    return orders


normalize_orders = orders_from_symbols


def resolve_readelf(readelf_path: str | Path | None = None) -> str:
    """Resolve an explicit readelf path or a safe executable name from PATH."""

    if readelf_path is not None:
        requested = str(readelf_path)
        candidate = Path(requested).expanduser()
        if candidate.is_file():
            return str(candidate)
        resolved = shutil.which(requested)
        if resolved:
            return resolved
        raise ReadelfError(f"readelf executable not found: {requested}")

    for name in DEFAULT_READELF_NAMES:
        resolved = shutil.which(name)
        if resolved:
            return resolved
    names = ", ".join(DEFAULT_READELF_NAMES)
    raise ReadelfError(f"no readelf executable found on PATH (tried {names})")


def _validate_readelf_argument(readelf_path: str | Path | None) -> str:
    """Validate an already-selected executable without re-running public resolution."""

    if readelf_path is None:
        return resolve_readelf()
    requested = str(readelf_path)
    candidate = Path(requested).expanduser()
    if candidate.is_file():
        return str(candidate)
    resolved = shutil.which(requested)
    if resolved:
        return resolved
    raise ReadelfError(f"readelf executable not found: {requested}")


Runner = Callable[..., subprocess.CompletedProcess[str]]


def readelf_symbols(
    object_path: str | Path,
    *,
    readelf_path: str | Path | None = None,
    runner: Runner = subprocess.run,
) -> list[Symbol]:
    """Read and parse symbols without invoking a build or mutating inputs."""

    path = Path(object_path).expanduser()
    if not path.is_file():
        raise ReadelfError(f"object/ELF path does not exist: {path}")
    # The CLI resolves once and passes the pinned executable to both reads.
    # Direct callers may omit it to use the safe PATH fallback.
    executable = _validate_readelf_argument(readelf_path)
    command = [executable, "-Ws", "--", str(path)]
    try:
        result = runner(command, capture_output=True, text=True, check=False)
    except OSError as exc:
        raise ReadelfError(f"failed to execute readelf {executable}: {exc}") from exc
    stdout = result.stdout
    if isinstance(stdout, bytes):
        stdout = stdout.decode("utf-8", errors="replace")
    if result.returncode:
        stderr = result.stderr or "readelf failed"
        if isinstance(stderr, bytes):
            stderr = stderr.decode("utf-8", errors="replace")
        raise ReadelfError(f"readelf failed for {path}: {str(stderr).strip()}")
    return parse_readelf_symbols(stdout or "")


def _missing_and_extra(
    target: Sequence[str], source: Sequence[str]
) -> tuple[list[str], list[str]]:
    source_counts = Counter(source)
    missing: list[str] = []
    for name in target:
        if source_counts[name]:
            source_counts[name] -= 1
        else:
            missing.append(name)
    target_counts = Counter(target)
    extra: list[str] = []
    for name in source:
        if target_counts[name]:
            target_counts[name] -= 1
        else:
            extra.append(name)
    return missing, extra


def _order_inversions(
    target: Sequence[str], source: Sequence[str]
) -> list[dict[str, Any]]:
    source_positions: dict[str, deque[int]] = {}
    for index, name in enumerate(source):
        source_positions.setdefault(name, deque()).append(index)
    common_target: list[tuple[int, str, int]] = []
    for index, name in enumerate(target):
        positions = source_positions.get(name)
        if positions:
            common_target.append((index, name, positions.popleft()))
    inversions: list[dict[str, Any]] = []
    for left_index, (target_left, left, source_left) in enumerate(common_target):
        for target_right, right, source_right in common_target[left_index + 1 :]:
            if left == right:
                continue
            if source_left > source_right:
                inversions.append(
                    {
                        "first": left,
                        "second": right,
                        "target_indices": [target_left, target_right],
                        "source_indices": [source_left, source_right],
                    }
                )
    return inversions


def compare_orders(
    target_orders: Mapping[str, Sequence[str]],
    source_orders: Mapping[str, Sequence[str]],
) -> dict[str, dict[str, Any]]:
    """Compare normalized target/source sequences for every report category."""

    differences: dict[str, dict[str, Any]] = {}
    for category in CATEGORIES:
        target = list(target_orders.get(category, ()))
        source = list(source_orders.get(category, ()))
        missing, extra = _missing_and_extra(target, source)
        inversions = _order_inversions(target, source)
        differences[category] = {
            "target": target,
            "source": source,
            "target_order": target,
            "source_order": source,
            "missing": missing,
            "extra": extra,
            "missing_in_source": missing,
            "extra_in_source": extra,
            "order_inversions": inversions,
            "inversions": len(inversions),
            "order_match": not missing and not extra and not inversions,
            "order_matches": not missing and not extra and not inversions,
        }
    return differences


def build_report(
    target_path: str | Path,
    source_path: str | Path,
    target_orders: Mapping[str, Sequence[str]],
    source_orders: Mapping[str, Sequence[str]],
    readelf_path: str | Path | None = None,
) -> dict[str, Any]:
    """Build the stable JSON-serializable report payload."""

    return {
        "readelf": str(readelf_path) if readelf_path is not None else None,
        "target": {"path": str(target_path), "orders": {k: list(target_orders.get(k, ())) for k in CATEGORIES}},
        "source": {"path": str(source_path), "orders": {k: list(source_orders.get(k, ())) for k in CATEGORIES}},
        "differences": compare_orders(target_orders, source_orders),
    }


def _format_names(names: Sequence[str]) -> str:
    return ", ".join(names) if names else "none"


def render_human(report: Mapping[str, Any]) -> str:
    """Render a concise human-readable report from :func:`build_report`."""

    target = report.get("target", {})
    source = report.get("source", {})
    readelf = report.get("readelf") or "<unknown>"
    lines = [
        f"Target: {target.get('path', '<unknown>')}",
        f"Source: {source.get('path', '<unknown>')}",
        f"Readelf: {readelf}",
    ]
    target_orders = target.get("orders", {})
    source_orders = source.get("orders", {})
    differences = report.get("differences", {})
    for category in CATEGORIES:
        diff = differences.get(category, {})
        lines.extend(
            [
                "",
                f"{category}:",
                f"  target order: {_format_names(target_orders.get(category, []))}",
                f"  source order: {_format_names(source_orders.get(category, []))}",
                f"  missing in source: {_format_names(diff.get('missing_in_source', []))}",
                f"  extra in source: {_format_names(diff.get('extra_in_source', []))}",
            ]
        )
        inversions = diff.get("order_inversions", [])
        if inversions:
            lines.append("  order inversions:")
            for inversion in inversions:
                lines.append(
                    "    "
                    f"{inversion['first']} before {inversion['second']} in target "
                    f"but reversed in source"
                )
        else:
            lines.append("  order inversions: none")
    return "\n".join(lines)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Compare normalized function, data, and local-pool ordering "
            "between two ELF/object files."
        )
    )
    parser.add_argument("target_path", nargs="?", help="target ELF/object path")
    parser.add_argument("source_path", nargs="?", help="source ELF/object path")
    parser.add_argument("--target", dest="target_option", help="target ELF/object path")
    parser.add_argument("--source", dest="source_option", help="source ELF/object path")
    parser.add_argument(
        "--readelf",
        "--readelf-path",
        dest="readelf_path",
        help="explicit readelf executable (default: powerpc-eabi-readelf/readelf on PATH)",
    )
    parser.add_argument("--json", action="store_true", help="emit JSON instead of text")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    target = args.target_option or args.target_path
    source = args.source_option or args.source_path
    if not target or not source:
        _parser().error("target and source ELF/object paths are required")
    try:
        readelf_path = resolve_readelf(args.readelf_path)
        target_symbols = readelf_symbols(target, readelf_path=readelf_path)
        source_symbols = readelf_symbols(source, readelf_path=readelf_path)
        target_orders = orders_from_symbols(target_symbols)
        source_orders = orders_from_symbols(source_symbols)
        report = build_report(
            target,
            source,
            target_orders,
            source_orders,
            readelf_path=readelf_path,
        )
    except ReadelfError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(render_human(report))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
