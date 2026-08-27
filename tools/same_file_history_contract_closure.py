#!/usr/bin/env python3
"""Build and diagnose fail-closed same-file history contract closures.

The preflight reads exactly one immutable Git blob, extracts exactly one named
function definition, and follows only declarations physically present in that
same file.  It never searches history, edits source, or advances recovery
authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import mismatch_cluster_audit as causal_reducer


MANIFEST_SCHEMA = "same_file_history_contract_closure_manifest/v1"
CONTEXT_SCHEMA = "same_file_history_contract_closure_context/v1"
RULE_ID = "same_file_history_contract_closure"
HASH_FIELD = "manifest_sha256"

_COMMIT_RE = re.compile(r"[0-9a-f]{40}")
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_FUNCTION_NAME_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(")
_FRAME_RE = re.compile(
    r"^\s*stwu\s+r1\s*,\s*-(?P<size>(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)\s*$",
    re.IGNORECASE,
)
_SAFE_KINDS = frozenset({"macro", "extern", "typedef", "static_object", "pool_owner"})


class HistoryContractInputError(ValueError):
    """The supplied donor or proof cannot support a closed source package."""


@dataclass(frozen=True)
class SourceSpan:
    symbol: str
    kind: str
    start: int
    end: int
    start_line: int
    end_line: int
    source: str

    @property
    def source_sha256(self) -> str:
        return hashlib.sha256(self.source.encode("utf-8")).hexdigest()


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    ).encode("utf-8")


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _closed(value: Any, *, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise HistoryContractInputError(f"{label} must be a JSON object")
    missing = fields - set(value)
    extra = set(value) - fields
    if missing or extra:
        raise HistoryContractInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 4096) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise HistoryContractInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise HistoryContractInputError(f"{label} exceeds {limit} characters")
    return result


def _source_text(value: Any, label: str, *, limit: int = 1 << 20) -> str:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise HistoryContractInputError(f"{label} must be non-empty source text")
    if len(value) > limit:
        raise HistoryContractInputError(f"{label} exceeds {limit} characters")
    return value


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64)
    if result != result.lower() or _SHA256_RE.fullmatch(result) is None:
        raise HistoryContractInputError(f"{label} must be a lowercase SHA-256")
    return result


def _commit(value: Any, label: str) -> str:
    result = _text(value, label, limit=40)
    if result != result.lower() or _COMMIT_RE.fullmatch(result) is None:
        raise HistoryContractInputError(f"{label} must be one full lowercase Git commit")
    return result


def _bool(value: Any, label: str, expected: bool) -> bool:
    if value is not expected:
        raise HistoryContractInputError(f"{label} must be {str(expected).lower()}")
    return expected


def _uint(
    value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 24
) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise HistoryContractInputError(
            f"{label} must be an integer from {minimum} through {maximum}"
        )
    return value


def _number(value: Any, label: str, *, maximum: float = 100.0) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise HistoryContractInputError(f"{label} must be a finite number")
    result = float(value)
    if not math.isfinite(result) or not 0.0 <= result <= maximum:
        raise HistoryContractInputError(f"{label} is outside the supported range")
    return result


def _source_path(value: str) -> str:
    path = PurePosixPath(value)
    if (
        path.is_absolute()
        or not path.parts
        or any(part in {"", ".", ".."} for part in path.parts)
        or "\\" in value
    ):
        raise HistoryContractInputError("source path must be a safe repository-relative POSIX path")
    return path.as_posix()


def _git(repo: Path, arguments: Sequence[str], *, binary: bool = False) -> bytes | str:
    try:
        completed = subprocess.run(
            ["git", "-C", str(repo), *arguments],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        detail = ""
        if isinstance(exc, subprocess.CalledProcessError):
            detail = exc.stderr.decode("utf-8", errors="replace").strip()
        raise HistoryContractInputError(
            f"immutable donor read failed: {detail or exc}"
        ) from exc
    if binary:
        return completed.stdout
    try:
        return completed.stdout.decode("utf-8").strip()
    except UnicodeDecodeError as exc:
        raise HistoryContractInputError("the donor blob is not UTF-8 source") from exc


def load_immutable_blob(repo: Path, commit: str, source_path: str) -> tuple[str, str]:
    resolved_repo = repo.resolve()
    if not resolved_repo.exists():
        raise HistoryContractInputError(f"repository does not exist: {resolved_repo}")
    sealed_commit = _commit(commit, "commit")
    sealed_path = _source_path(source_path)
    resolved = str(_git(resolved_repo, ["rev-parse", "--verify", sealed_commit])).strip()
    object_type = str(_git(resolved_repo, ["cat-file", "-t", sealed_commit])).strip()
    if resolved != sealed_commit or object_type != "commit":
        raise HistoryContractInputError("the donor commit did not resolve to the supplied immutable identity")
    blob_oid = str(_git(resolved_repo, ["rev-parse", f"{sealed_commit}:{sealed_path}"])).strip()
    raw = _git(resolved_repo, ["show", f"{sealed_commit}:{sealed_path}"], binary=True)
    assert isinstance(raw, bytes)
    try:
        return raw.decode("utf-8"), blob_oid
    except UnicodeDecodeError as exc:
        raise HistoryContractInputError("the donor blob is not UTF-8 source") from exc


def _mask_c(source: str) -> str:
    chars = list(source)
    state = "code"
    index = 0
    while index < len(chars):
        current = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if current == "/" and following == "/":
                chars[index] = chars[index + 1] = " "
                state = "line_comment"
                index += 2
                continue
            if current == "/" and following == "*":
                chars[index] = chars[index + 1] = " "
                state = "block_comment"
                index += 2
                continue
            if current == '"':
                chars[index] = " "
                state = "string"
            elif current == "'":
                chars[index] = " "
                state = "character"
        elif state == "line_comment":
            if current == "\n":
                state = "code"
            else:
                chars[index] = " "
        elif state == "block_comment":
            if current == "*" and following == "/":
                chars[index] = chars[index + 1] = " "
                state = "code"
                index += 2
                continue
            if current != "\n":
                chars[index] = " "
        elif state in {"string", "character"}:
            if current == "\\":
                chars[index] = " "
                if index + 1 < len(chars):
                    if chars[index + 1] != "\n":
                        chars[index + 1] = " "
                    index += 2
                    continue
            terminator = '"' if state == "string" else "'"
            if current == terminator:
                chars[index] = " "
                state = "code"
            elif current != "\n":
                chars[index] = " "
        index += 1
    if state in {"block_comment", "string", "character"}:
        raise HistoryContractInputError("donor source contains an unterminated lexical construct")
    return "".join(chars)


def _matching(masked: str, start: int, opening: str, closing: str) -> int:
    if start >= len(masked) or masked[start] != opening:
        raise HistoryContractInputError(f"expected {opening!r} at source offset {start}")
    depth = 1
    for index in range(start + 1, len(masked)):
        if masked[index] == opening:
            depth += 1
        elif masked[index] == closing:
            depth -= 1
            if depth == 0:
                return index
    raise HistoryContractInputError(f"unclosed {opening!r} at source offset {start}")


def _line(source: str, offset: int) -> int:
    return source.count("\n", 0, offset) + 1


def _make_span(source: str, symbol: str, kind: str, start: int, end: int) -> SourceSpan:
    return SourceSpan(
        symbol=symbol,
        kind=kind,
        start=start,
        end=end,
        start_line=_line(source, start),
        end_line=_line(source, max(start, end - 1)),
        source=source[start:end],
    )


def extract_function(source: str, function: str) -> SourceSpan:
    if _IDENTIFIER_RE.fullmatch(function) is None:
        raise HistoryContractInputError("function must be one C identifier")
    masked = _mask_c(source)
    matches: list[SourceSpan] = []
    for match in re.finditer(rf"\b{re.escape(function)}\s*\(", masked):
        open_paren = masked.find("(", match.start())
        close_paren = _matching(masked, open_paren, "(", ")")
        cursor = close_paren + 1
        while cursor < len(masked) and masked[cursor].isspace():
            cursor += 1
        if cursor >= len(masked) or masked[cursor] != "{":
            continue
        close_brace = _matching(masked, cursor, "{", "}")
        start = source.rfind("\n", 0, match.start()) + 1
        end = close_brace + 1
        if end < len(source) and source[end] == "\r":
            end += 1
        if end < len(source) and source[end] == "\n":
            end += 1
        matches.append(_make_span(source, function, "function", start, end))
    if len(matches) != 1:
        raise HistoryContractInputError(
            f"expected exactly one definition of {function}, found {len(matches)}"
        )
    return matches[0]


def _macro_spans(source: str) -> list[SourceSpan]:
    lines = source.splitlines(keepends=True)
    offsets: list[int] = []
    offset = 0
    for value in lines:
        offsets.append(offset)
        offset += len(value)
    result: list[SourceSpan] = []
    index = 0
    while index < len(lines):
        match = re.match(r"^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)\b", lines[index])
        if match is None:
            index += 1
            continue
        end_index = index
        while lines[end_index].rstrip("\r\n").endswith("\\"):
            end_index += 1
            if end_index >= len(lines):
                raise HistoryContractInputError("macro continuation reaches end of donor source")
        start = offsets[index]
        end = offsets[end_index] + len(lines[end_index])
        result.append(_make_span(source, match.group(1), "macro", start, end))
        index = end_index + 1
    return result


def _blank_preprocessor(masked: str) -> str:
    chars = list(masked)
    lines = masked.splitlines(keepends=True)
    offset = 0
    continuation = False
    for line in lines:
        directive = continuation or re.match(r"^[ \t]*#", line) is not None
        continuation = directive and line.rstrip("\r\n").endswith("\\")
        if directive:
            for index in range(offset, offset + len(line)):
                if chars[index] not in {"\r", "\n"}:
                    chars[index] = " "
        offset += len(line)
    return "".join(chars)


def _top_level_declarations(source: str) -> Iterable[tuple[int, int, str]]:
    masked = _blank_preprocessor(_mask_c(source))
    start = 0
    depth = 0
    index = 0
    while index < len(masked):
        char = masked[index]
        if char == "{" and depth == 0:
            header = masked[start:index].strip()
            is_function = (
                header.endswith(")")
                and not header.startswith("typedef")
                and "=" not in header
            )
            if is_function:
                close = _matching(masked, index, "{", "}")
                start = close + 1
                index = close + 1
                continue
            depth = 1
        elif char == "{" and depth > 0:
            depth += 1
        elif char == "}" and depth > 0:
            depth -= 1
        elif char == ";" and depth == 0:
            raw_start = start
            while raw_start < index and masked[raw_start].isspace():
                raw_start += 1
            if raw_start < index:
                yield raw_start, index + 1, masked[raw_start : index + 1]
            start = index + 1
        index += 1


def _declaration_symbol(masked: str, kind: str) -> str | None:
    if kind == "typedef":
        pointer = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", masked)
        if pointer is not None:
            return pointer.group(1)
    function_names = _FUNCTION_NAME_RE.findall(masked)
    if function_names and kind == "extern":
        return function_names[-1]
    declarator = masked.split("=", 1)[0].rstrip().rstrip(";").rstrip()
    declarator = re.sub(r"\[[^\]]*\]\s*$", "", declarator)
    names = _IDENTIFIER_RE.findall(declarator)
    if not names:
        return None
    return names[-1]


def _contract_spans(source: str) -> list[SourceSpan]:
    result = _macro_spans(source)
    for start, end, masked in _top_level_declarations(source):
        normalized = " ".join(masked.split())
        kind: str | None = None
        if normalized.startswith("extern "):
            kind = "extern"
        elif normalized.startswith("typedef "):
            kind = "typedef"
        elif normalized.startswith("static "):
            if "(" not in normalized.split("=", 1)[0]:
                kind = "static_object"
        elif "const " in f" {normalized} " and "=" in normalized:
            kind = "pool_owner"
        if kind is None:
            continue
        symbol = _declaration_symbol(masked, kind)
        if symbol is not None:
            result.append(_make_span(source, symbol, kind, start, end))
    return result


def _identifier_order(source: str) -> list[str]:
    masked = _mask_c(source)
    return list(dict.fromkeys(match.group(0) for match in _IDENTIFIER_RE.finditer(masked)))


def _contract_index(source: str) -> dict[str, SourceSpan]:
    grouped: dict[str, list[SourceSpan]] = {}
    for span in _contract_spans(source):
        grouped.setdefault(span.symbol, []).append(span)
    ambiguous = {name: items for name, items in grouped.items() if len(items) != 1}
    if ambiguous:
        names = ", ".join(sorted(ambiguous))
        raise HistoryContractInputError(f"ambiguous same-file contract symbols: {names}")
    return {name: items[0] for name, items in grouped.items()}


def _dependency_closure(
    source: str, function_span: SourceSpan
) -> tuple[list[SourceSpan], dict[str, list[str]]]:
    index = _contract_index(source)
    ordered: list[SourceSpan] = []
    dependencies: dict[str, list[str]] = {}
    visiting: set[str] = set()
    visited: set[str] = set()

    def visit(symbol: str) -> None:
        if symbol in visited:
            return
        if symbol in visiting:
            raise HistoryContractInputError(f"same-file contract cycle contains {symbol}")
        visiting.add(symbol)
        span = index[symbol]
        direct = [
            name
            for name in _identifier_order(span.source)
            if name != symbol and name in index
        ]
        dependencies[symbol] = direct
        for name in direct:
            visit(name)
        visiting.remove(symbol)
        visited.add(symbol)
        ordered.append(span)

    for symbol in _identifier_order(function_span.source):
        if symbol != function_span.symbol and symbol in index:
            visit(symbol)
    return ordered, dependencies


def _parse_requirements(values: Sequence[str]) -> dict[str, str]:
    result: dict[str, str] = {}
    for value in values:
        symbol, separator, kind = value.partition("=")
        if (
            not separator
            or _IDENTIFIER_RE.fullmatch(symbol) is None
            or kind not in _SAFE_KINDS
            or symbol in result
        ):
            raise HistoryContractInputError(
                "required contracts must be unique SYMBOL=KIND pairs"
            )
        result[symbol] = kind
    return result


def build_manifest(
    *,
    repo: Path,
    commit: str,
    source_path: str,
    function: str,
    graphify_location: str,
    report_sha256: str,
    destination_file: Path,
    expected_function_sha256: str | None = None,
    expected_destination_sha256: str | None = None,
    required_contracts: Mapping[str, str] | None = None,
) -> dict[str, Any]:
    sealed_commit = _commit(commit, "commit")
    sealed_path = _source_path(source_path)
    source, blob_oid = load_immutable_blob(repo, sealed_commit, sealed_path)
    function_span = extract_function(source, function)
    try:
        destination_bytes = destination_file.resolve().read_bytes()
        destination_source = destination_bytes.decode("utf-8")
    except (OSError, UnicodeDecodeError) as exc:
        raise HistoryContractInputError(
            f"cannot read UTF-8 destination source {destination_file}: {exc}"
        ) from exc
    destination_sha256 = _sha256_bytes(destination_bytes)
    if expected_destination_sha256 is not None and destination_sha256 != _sha256(
        expected_destination_sha256, "expected_destination_sha256"
    ):
        raise HistoryContractInputError("the destination source hash drifted")
    if expected_function_sha256 is not None:
        expected = _sha256(expected_function_sha256, "expected_function_sha256")
        if function_span.source_sha256 != expected:
            raise HistoryContractInputError(
                "the extracted donor function does not match the sealed source hash"
            )
    closure, dependency_edges = _dependency_closure(source, function_span)
    destination_index = _contract_index(destination_source)
    emitted: list[SourceSpan] = []
    satisfied: list[dict[str, Any]] = []
    for span in closure:
        destination_span = destination_index.get(span.symbol)
        if destination_span is None:
            emitted.append(span)
            continue
        if (
            destination_span.kind != span.kind
            or destination_span.source_sha256 != span.source_sha256
        ):
            raise HistoryContractInputError(
                f"destination contract {span.symbol} is present but incompatible with the donor"
            )
        satisfied.append(
            {
                "symbol": span.symbol,
                "kind": span.kind,
                "donor_start_line": span.start_line,
                "destination_start_line": destination_span.start_line,
                "source_sha256": span.source_sha256,
            }
        )
    emitted_symbols = {span.symbol for span in emitted}
    satisfied_symbols = {item["symbol"] for item in satisfied}
    observed = {
        **{span.symbol: span.kind for span in emitted},
        **{item["symbol"]: item["kind"] for item in satisfied},
    }
    required = dict(required_contracts or {})
    for symbol, kind in required.items():
        if _IDENTIFIER_RE.fullmatch(symbol) is None or kind not in _SAFE_KINDS:
            raise HistoryContractInputError("required contract contains an invalid symbol or kind")
        if observed.get(symbol) != kind:
            raise HistoryContractInputError(
                f"required same-file contract {symbol}={kind} was not closed"
            )
    dependencies = [
        {
            "symbol": span.symbol,
            "kind": span.kind,
            "start_line": span.start_line,
            "end_line": span.end_line,
            "source_sha256": span.source_sha256,
            "source": span.source,
            "requires": [
                name for name in dependency_edges[span.symbol] if name in emitted_symbols
            ],
            "satisfied_by_destination": [
                name for name in dependency_edges[span.symbol] if name in satisfied_symbols
            ],
        }
        for span in emitted
    ]
    body: dict[str, Any] = {
        "schema": MANIFEST_SCHEMA,
        "report_sha256": _sha256(report_sha256, "report_sha256"),
        "graphify": {
            "location": _text(graphify_location, "graphify_location", limit=512),
            "bound": True,
            "broad_search_performed": False,
        },
        "donor": {
            "commit": sealed_commit,
            "source_path": sealed_path,
            "blob_oid": blob_oid,
            "source_blob_sha256": _sha256_bytes(source.encode("utf-8")),
        },
        "destination": {
            "source_path": destination_file.resolve().as_posix(),
            "source_sha256": destination_sha256,
        },
        "function": {
            "name": function_span.symbol,
            "start_line": function_span.start_line,
            "end_line": function_span.end_line,
            "source_sha256": function_span.source_sha256,
            "source": function_span.source,
        },
        "dependencies": dependencies,
        "satisfied_dependencies": satisfied,
        "required_contracts": [
            {"symbol": symbol, "kind": kind} for symbol, kind in sorted(required.items())
        ],
        "package_order": [span.symbol for span in emitted] + [function_span.symbol],
        "authority_advanced": False,
    }
    body[HASH_FIELD] = _sha256_bytes(_canonical(body))
    return body


def parse_manifest(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "same-file history contract manifest"
    root = _closed(
        value,
        fields={
            "schema", "report_sha256", "graphify", "donor", "function",
            "destination", "dependencies", "satisfied_dependencies",
            "required_contracts", "package_order",
            "authority_advanced", HASH_FIELD,
        },
        label=label,
    )
    if root.get("schema") != MANIFEST_SCHEMA:
        raise HistoryContractInputError(f"{label}.schema must be {MANIFEST_SCHEMA}")
    supplied_hash = _sha256(root.get(HASH_FIELD), f"{label}.{HASH_FIELD}")
    unhashed = dict(root)
    del unhashed[HASH_FIELD]
    if _sha256_bytes(_canonical(unhashed)) != supplied_hash:
        raise HistoryContractInputError(f"{label} self-hash mismatch")
    _bool(root.get("authority_advanced"), f"{label}.authority_advanced", False)
    graphify = _closed(
        root.get("graphify"),
        fields={"location", "bound", "broad_search_performed"},
        label=f"{label}.graphify",
    )
    _text(graphify.get("location"), f"{label}.graphify.location", limit=512)
    _bool(graphify.get("bound"), f"{label}.graphify.bound", True)
    _bool(
        graphify.get("broad_search_performed"),
        f"{label}.graphify.broad_search_performed",
        False,
    )
    donor = _closed(
        root.get("donor"),
        fields={"commit", "source_path", "blob_oid", "source_blob_sha256"},
        label=f"{label}.donor",
    )
    _commit(donor.get("commit"), f"{label}.donor.commit")
    _source_path(_text(donor.get("source_path"), f"{label}.donor.source_path"))
    _text(donor.get("blob_oid"), f"{label}.donor.blob_oid", limit=64)
    _sha256(donor.get("source_blob_sha256"), f"{label}.donor.source_blob_sha256")
    destination = _closed(
        root.get("destination"),
        fields={"source_path", "source_sha256"},
        label=f"{label}.destination",
    )
    _text(destination.get("source_path"), f"{label}.destination.source_path", limit=4096)
    _sha256(destination.get("source_sha256"), f"{label}.destination.source_sha256")
    function = _closed(
        root.get("function"),
        fields={"name", "start_line", "end_line", "source_sha256", "source"},
        label=f"{label}.function",
    )
    function_name = _text(function.get("name"), f"{label}.function.name", limit=128)
    function_source = _source_text(function.get("source"), f"{label}.function.source")
    if _sha256_bytes(function_source.encode("utf-8")) != _sha256(
        function.get("source_sha256"), f"{label}.function.source_sha256"
    ):
        raise HistoryContractInputError(f"{label}.function source hash mismatch")
    start_line = _uint(function.get("start_line"), f"{label}.function.start_line", minimum=1)
    end_line = _uint(function.get("end_line"), f"{label}.function.end_line", minimum=start_line)
    raw_dependencies = root.get("dependencies")
    if not isinstance(raw_dependencies, list):
        raise HistoryContractInputError(f"{label}.dependencies must be an array")
    dependencies: list[dict[str, Any]] = []
    seen: dict[str, str] = {}
    for index, raw in enumerate(raw_dependencies):
        item_label = f"{label}.dependencies[{index}]"
        item = _closed(
            raw,
            fields={
                "symbol", "kind", "start_line", "end_line", "source_sha256",
                "source", "requires", "satisfied_by_destination",
            },
            label=item_label,
        )
        symbol = _text(item.get("symbol"), f"{item_label}.symbol", limit=128)
        kind = _text(item.get("kind"), f"{item_label}.kind", limit=32)
        if _IDENTIFIER_RE.fullmatch(symbol) is None or kind not in _SAFE_KINDS or symbol in seen:
            raise HistoryContractInputError(f"{item_label} has an invalid or duplicate symbol")
        source_text = _source_text(item.get("source"), f"{item_label}.source")
        source_hash = _sha256(item.get("source_sha256"), f"{item_label}.source_sha256")
        if _sha256_bytes(source_text.encode("utf-8")) != source_hash:
            raise HistoryContractInputError(f"{item_label} source hash mismatch")
        requires = item.get("requires")
        if not isinstance(requires, list) or any(
            not isinstance(name, str) or name not in seen for name in requires
        ):
            raise HistoryContractInputError(
                f"{item_label}.requires must reference earlier package dependencies"
            )
        destination_requires = item.get("satisfied_by_destination")
        if not isinstance(destination_requires, list) or any(
            not isinstance(name, str) for name in destination_requires
        ):
            raise HistoryContractInputError(
                f"{item_label}.satisfied_by_destination must be an identifier array"
            )
        seen[symbol] = kind
        dependencies.append(
            {
                "symbol": symbol,
                "kind": kind,
                "start_line": _uint(item.get("start_line"), f"{item_label}.start_line", minimum=1),
                "end_line": _uint(item.get("end_line"), f"{item_label}.end_line", minimum=1),
                "source_sha256": source_hash,
                "source": source_text,
                "requires": list(requires),
                "satisfied_by_destination": list(destination_requires),
            }
        )
    raw_satisfied = root.get("satisfied_dependencies")
    if not isinstance(raw_satisfied, list):
        raise HistoryContractInputError(f"{label}.satisfied_dependencies must be an array")
    satisfied: list[dict[str, Any]] = []
    satisfied_symbols: dict[str, str] = {}
    for index, raw in enumerate(raw_satisfied):
        item_label = f"{label}.satisfied_dependencies[{index}]"
        item = _closed(
            raw,
            fields={
                "symbol", "kind", "donor_start_line", "destination_start_line",
                "source_sha256",
            },
            label=item_label,
        )
        symbol = _text(item.get("symbol"), f"{item_label}.symbol", limit=128)
        kind = _text(item.get("kind"), f"{item_label}.kind", limit=32)
        if (
            _IDENTIFIER_RE.fullmatch(symbol) is None
            or kind not in _SAFE_KINDS
            or symbol in seen
            or symbol in satisfied_symbols
        ):
            raise HistoryContractInputError(f"{item_label} has an invalid or duplicate symbol")
        normalized = {
            "symbol": symbol,
            "kind": kind,
            "donor_start_line": _uint(
                item.get("donor_start_line"), f"{item_label}.donor_start_line", minimum=1
            ),
            "destination_start_line": _uint(
                item.get("destination_start_line"),
                f"{item_label}.destination_start_line",
                minimum=1,
            ),
            "source_sha256": _sha256(
                item.get("source_sha256"), f"{item_label}.source_sha256"
            ),
        }
        satisfied_symbols[symbol] = kind
        satisfied.append(normalized)
    for item in dependencies:
        if any(name not in satisfied_symbols for name in item["satisfied_by_destination"]):
            raise HistoryContractInputError(
                f"{label} dependency references an unsealed destination contract"
            )
    raw_requirements = root.get("required_contracts")
    if not isinstance(raw_requirements, list):
        raise HistoryContractInputError(f"{label}.required_contracts must be an array")
    requirements: list[dict[str, str]] = []
    for index, raw in enumerate(raw_requirements):
        item = _closed(
            raw,
            fields={"symbol", "kind"},
            label=f"{label}.required_contracts[{index}]",
        )
        symbol = _text(item.get("symbol"), f"{label}.required_contracts[{index}].symbol", limit=128)
        kind = _text(item.get("kind"), f"{label}.required_contracts[{index}].kind", limit=32)
        if seen.get(symbol) != kind and satisfied_symbols.get(symbol) != kind:
            raise HistoryContractInputError(
                f"{label}.required_contracts[{index}] is absent from the emitted package"
            )
        requirements.append({"symbol": symbol, "kind": kind})
    expected_order = list(seen) + [function_name]
    if root.get("package_order") != expected_order:
        raise HistoryContractInputError(f"{label}.package_order does not match dependency order")
    return {
        **dict(root),
        "dependencies": dependencies,
        "satisfied_dependencies": satisfied,
        "required_contracts": requirements,
        "function": {
            "name": function_name,
            "start_line": start_line,
            "end_line": end_line,
            "source_sha256": function["source_sha256"],
            "source": function_source,
        },
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "same-file history contract context"
    root = _closed(
        value,
        fields={
            "schema", "owner", "function", "report_sha256", "manifest",
            "baseline", "failed_preflight", "exact_result", "telemetry",
            "authority_advanced",
        },
        label=label,
    )
    if root.get("schema") != CONTEXT_SCHEMA:
        raise HistoryContractInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    owner = _text(root.get("owner"), f"{label}.owner", limit=192)
    function = _text(root.get("function"), f"{label}.function", limit=128)
    report_sha = _sha256(root.get("report_sha256"), f"{label}.report_sha256")
    _bool(root.get("authority_advanced"), f"{label}.authority_advanced", False)
    manifest = parse_manifest(root.get("manifest"))
    if manifest["function"]["name"] != function or manifest["report_sha256"] != report_sha:
        raise HistoryContractInputError(f"{label} manifest binding drifted")

    baseline_raw = _closed(
        root.get("baseline"),
        fields={
            "objdiff_canonical_sha256", "strict_report_sha256", "target_bytes",
            "candidate_bytes", "target_frame", "candidate_frame",
            "target_physical_relocations", "candidate_physical_relocations",
            "match_percent", "semantically_incomplete",
        },
        label=f"{label}.baseline",
    )
    baseline = {
        "objdiff_canonical_sha256": _sha256(
            baseline_raw.get("objdiff_canonical_sha256"),
            f"{label}.baseline.objdiff_canonical_sha256",
        ),
        "strict_report_sha256": _sha256(
            baseline_raw.get("strict_report_sha256"),
            f"{label}.baseline.strict_report_sha256",
        ),
        "target_bytes": _uint(baseline_raw.get("target_bytes"), f"{label}.baseline.target_bytes", minimum=32),
        "candidate_bytes": _uint(baseline_raw.get("candidate_bytes"), f"{label}.baseline.candidate_bytes", minimum=4),
        "target_frame": _uint(baseline_raw.get("target_frame"), f"{label}.baseline.target_frame", minimum=16),
        "candidate_frame": _uint(baseline_raw.get("candidate_frame"), f"{label}.baseline.candidate_frame", minimum=16),
        "target_physical_relocations": _uint(
            baseline_raw.get("target_physical_relocations"),
            f"{label}.baseline.target_physical_relocations",
            minimum=1,
        ),
        "candidate_physical_relocations": _uint(
            baseline_raw.get("candidate_physical_relocations"),
            f"{label}.baseline.candidate_physical_relocations",
            minimum=0,
        ),
        "match_percent": _number(baseline_raw.get("match_percent"), f"{label}.baseline.match_percent"),
        "semantically_incomplete": _bool(
            baseline_raw.get("semantically_incomplete"),
            f"{label}.baseline.semantically_incomplete",
            True,
        ),
    }
    if not (
        baseline["candidate_bytes"] < baseline["target_bytes"]
        and baseline["candidate_frame"] < baseline["target_frame"]
        and baseline["candidate_physical_relocations"] < baseline["target_physical_relocations"]
        and baseline["match_percent"] < 80.0
    ):
        raise HistoryContractInputError(f"{label}.baseline is not a sealed semantic deficit")

    failed_raw = _closed(
        root.get("failed_preflight"),
        fields={"body_only_attempted", "failure_stage", "missing_symbol", "object_created"},
        label=f"{label}.failed_preflight",
    )
    failed = {
        "body_only_attempted": _bool(
            failed_raw.get("body_only_attempted"), f"{label}.failed_preflight.body_only_attempted", True
        ),
        "failure_stage": _text(failed_raw.get("failure_stage"), f"{label}.failed_preflight.failure_stage"),
        "missing_symbol": _text(failed_raw.get("missing_symbol"), f"{label}.failed_preflight.missing_symbol", limit=128),
        "object_created": _bool(
            failed_raw.get("object_created"), f"{label}.failed_preflight.object_created", False
        ),
    }
    manifest_symbols = {item["symbol"] for item in manifest["dependencies"]}
    if failed["failure_stage"] != "before_object" or failed["missing_symbol"] not in manifest_symbols:
        raise HistoryContractInputError(f"{label}.failed_preflight is not closed by the manifest")

    exact_raw = _closed(
        root.get("exact_result"),
        fields={
            "objdiff_canonical_sha256", "source_sha256", "object_sha256",
            "strict_report_sha256", "data_report_sha256", "candidate_record_sha256",
            "target_bytes", "candidate_bytes", "physical_relocations", "zero_rows",
            "protected_exact_before", "protected_exact_after", "protected_losses",
        },
        label=f"{label}.exact_result",
    )
    exact = {
        name: _sha256(exact_raw.get(name), f"{label}.exact_result.{name}")
        for name in (
            "objdiff_canonical_sha256", "source_sha256", "object_sha256",
            "strict_report_sha256", "data_report_sha256", "candidate_record_sha256",
        )
    }
    exact.update(
        {
            name: _uint(exact_raw.get(name), f"{label}.exact_result.{name}", minimum=0)
            for name in (
                "target_bytes", "candidate_bytes", "physical_relocations", "zero_rows",
                "protected_exact_before", "protected_exact_after", "protected_losses",
            )
        }
    )
    if not (
        exact["target_bytes"] == exact["candidate_bytes"] == baseline["target_bytes"]
        and exact["physical_relocations"] == baseline["target_physical_relocations"]
        and exact["zero_rows"] == 0
        and exact["protected_exact_after"] == exact["protected_exact_before"] + 1
        and exact["protected_losses"] == 0
    ):
        raise HistoryContractInputError(f"{label}.exact_result does not close the sealed deficit")

    telemetry_raw = _closed(
        root.get("telemetry"),
        fields={
            "candidate_launches", "compiled_candidates", "proof_rebuilds", "tracer_runs",
            "donor_searches", "telemetry_complete", "interval_log_sha256",
        },
        label=f"{label}.telemetry",
    )
    telemetry = {
        name: _uint(telemetry_raw.get(name), f"{label}.telemetry.{name}")
        for name in (
            "candidate_launches", "compiled_candidates", "proof_rebuilds",
            "tracer_runs", "donor_searches",
        )
    }
    telemetry["telemetry_complete"] = _bool(
        telemetry_raw.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete", False
    )
    telemetry["interval_log_sha256"] = _sha256(
        telemetry_raw.get("interval_log_sha256"), f"{label}.telemetry.interval_log_sha256"
    )
    if telemetry != {
        "candidate_launches": 2,
        "compiled_candidates": 1,
        "proof_rebuilds": 1,
        "tracer_runs": 0,
        "donor_searches": 1,
        "telemetry_complete": False,
        "interval_log_sha256": telemetry["interval_log_sha256"],
    }:
        raise HistoryContractInputError(f"{label}.telemetry does not bind the one-search Kinoko path")
    return {
        "schema": CONTEXT_SCHEMA,
        "owner": owner,
        "function": function,
        "report_sha256": report_sha,
        "manifest": manifest,
        "baseline": baseline,
        "failed_preflight": failed,
        "exact_result": exact,
        "telemetry": telemetry,
        "authority_advanced": False,
    }


def _frame_size(instructions: Sequence[Any]) -> int | None:
    for instruction in instructions[:16]:
        if instruction.has_instruction:
            match = _FRAME_RE.fullmatch(instruction.formatted)
            if match is not None:
                return int(match.group("size"), 0)
    return None


def _physical_relocations(instructions: Sequence[Any]) -> int:
    return sum(
        1
        for instruction in instructions
        if instruction.relocation
        and instruction.relocation.get("type_name") not in {None, "R_PPC_NONE"}
    )


def evaluate(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    if context is None:
        return {"matched": False, "reason": "no authenticated same-file history contract context was supplied"}
    if pair.name != context["function"]:
        return {"matched": False, "reason": "the history contract context is bound to another function"}
    exact = context["exact_result"]
    if objdiff_canonical_sha256 == exact["objdiff_canonical_sha256"]:
        return {
            "matched": False,
            "reason": "the authenticated history contract result is already exact; no candidate is scheduled",
            "evidence": {"exact_result": exact, "authority_advanced": False},
        }
    baseline = context["baseline"]
    if objdiff_canonical_sha256 != baseline["objdiff_canonical_sha256"]:
        return {"matched": False, "reason": "the history contract context is bound to another objdiff report"}
    target_size = causal_reducer._parse_number(pair.target.get("size")) if pair.target else None
    candidate_size = causal_reducer._parse_number(pair.candidate.get("size")) if pair.candidate else None
    match_percent = pair.candidate.get("match_percent") if pair.candidate else None
    observed = (
        target_size,
        candidate_size,
        _frame_size(target),
        _frame_size(candidate),
        _physical_relocations(target),
        _physical_relocations(candidate),
        float(match_percent) if isinstance(match_percent, (int, float)) else None,
    )
    sealed = (
        baseline["target_bytes"],
        baseline["candidate_bytes"],
        baseline["target_frame"],
        baseline["candidate_frame"],
        baseline["target_physical_relocations"],
        baseline["candidate_physical_relocations"],
        baseline["match_percent"],
    )
    if observed != sealed:
        return {
            "matched": False,
            "reason": "the semantic size/frame/relocation deficit drifted",
            "evidence": {"observed": list(observed), "sealed": list(sealed)},
        }
    manifest = context["manifest"]
    dependency_summary = [
        {
            "symbol": item["symbol"],
            "kind": item["kind"],
            "start_line": item["start_line"],
            "end_line": item["end_line"],
            "source_sha256": item["source_sha256"],
        }
        for item in manifest["dependencies"]
    ]
    return {
        "matched": True,
        "reason": (
            "the focus has a large authenticated semantic deficit and one immutable same-file donor; "
            "the donor body is closed over every referenced same-file contract before compilation"
        ),
        "confidence": 1.0,
        "source_class": "immutable_same_file_history_contract_package",
        "recommendation": (
            "Compile one bounded package containing the authenticated donor body and all manifest-ordered "
            "same-file dependencies; fail before object creation if any contract is absent or ambiguous."
        ),
        "evidence": {
            "manifest_sha256": manifest[HASH_FIELD],
            "donor": manifest["donor"],
            "function": {
                "name": manifest["function"]["name"],
                "source_sha256": manifest["function"]["source_sha256"],
                "start_line": manifest["function"]["start_line"],
                "end_line": manifest["function"]["end_line"],
            },
            "dependencies": dependency_summary,
            "package_order": manifest["package_order"],
            "failed_body_only_preflight": context["failed_preflight"],
            "recommended_cells": [
                {
                    "order": 1,
                    "kind": "authenticated_same_file_history_contract_package",
                    "manifest_sha256": manifest[HASH_FIELD],
                    "acceptance": exact,
                }
            ],
            "suppressed_axes": [
                "body_only_compile",
                "broad_history_search",
                "manual_source_shape_permutations",
                "tracer_capture",
                "dead_or_fake_dependencies",
                "padding",
                "register_shaping",
                "source_retention",
                "promotion",
            ],
            "telemetry": context["telemetry"],
            "authority_advanced": False,
        },
    }


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build one fail-closed same-file history contract manifest."
    )
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--path", required=True, dest="source_path")
    parser.add_argument("--function", required=True)
    parser.add_argument("--graphify-location", required=True)
    parser.add_argument("--report-sha256", required=True)
    parser.add_argument("--destination", required=True, type=Path, dest="destination_file")
    parser.add_argument("--expect-function-sha256")
    parser.add_argument("--expect-destination-sha256")
    parser.add_argument(
        "--require-contract",
        action="append",
        default=[],
        help="required same-file SYMBOL=KIND; repeat for each sealed contract",
    )
    parser.add_argument("--output", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    try:
        manifest = build_manifest(
            repo=args.repo,
            commit=args.commit,
            source_path=args.source_path,
            function=args.function,
            graphify_location=args.graphify_location,
            report_sha256=args.report_sha256,
            destination_file=args.destination_file,
            expected_function_sha256=args.expect_function_sha256,
            expected_destination_sha256=args.expect_destination_sha256,
            required_contracts=_parse_requirements(args.require_contract),
        )
        parse_manifest(manifest)
    except HistoryContractInputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    rendered = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    if args.output is None:
        print(rendered, end="")
    else:
        args.output.write_text(rendered, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
