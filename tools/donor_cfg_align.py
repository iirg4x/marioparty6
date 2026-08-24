"""Conservative donor/source-shape ranking for one authenticated function.

This module is deliberately independent from the match workbench.  It consumes
one objdiff report and two C sources, authenticates the report's candidate /
target pairing, and ranks only source-shape differences that can be described
from lexical and small control-flow facts.  It never edits a source file or
claims that a donor proves a target expression.

The implementation is intentionally standard-library-only so that it remains a
useful read-only probe in a recovery checkout.  The public entry point is
``align_donor_cfg``; ``analyze_alignment`` and ``rank_hypotheses`` are aliases
kept for callers that prefer those names.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any, Iterable, Mapping, Sequence


SCHEMA = "donor_cfg_align/v1"
SCHEMA_VERSION = 1

_CONTROL_WORDS = frozenset(
    {"if", "for", "while", "switch", "sizeof", "return", "do", "else"}
)
_MULTI_OPERATORS = (
    ">>>=",
    "<<=",
    "->*",
    "...",
    "++",
    "--",
    "->",
    "&&",
    "||",
    "==",
    "!=",
    "<=",
    ">=",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "&=",
    "|=",
    "^=",
    "<<",
    ">>",
)
_ASSIGNMENT_RE = re.compile(
    r"(?<![=!<>])"
    r"(?P<lhs>[A-Za-z_]\w*(?:\s*(?:->|\.|\[[^\]]+\])\s*"
    r"(?:[A-Za-z_]\w*|[^\]]+))*?)"
    r"\s*(?P<op>[+\-*/%&|^]?=)(?!=)"
)
_IDENTIFIER_RE = re.compile(r"[A-Za-z_]\w*")
_NUMBER_RE = re.compile(
    r"(?:0[xX][0-9A-Fa-f]+|0[bB][01]+|(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)[uUlLfF]*"
)
_TOKEN_RE = re.compile(
    r"(?:0[xX][0-9A-Fa-f]+|0[bB][01]+|(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)[uUlLfF]*"
    r"|[A-Za-z_]\w*"
    r"|\"(?:\\.|[^\"\\])*\""
    r"|'(?:\\.|[^'\\])*'"
    r"|>>>?=?|<<=?|->|&&|\|\||==|!=|<=|>=|\+\+|--|\+=|-=|\*=|/=|%=|&=|\|=|\^=|[^\s]"
)
_CAST_RE = re.compile(
    r"\(\s*(?:(?:const|volatile|unsigned|signed|long|short|struct|union|enum)\s+)*"
    r"(?:void|char|short|int|long|float|double|size_t|u?int\d*|[us](?:8|16|32|64)|f(?:32|64)|[A-Z][A-Za-z0-9_]*)"
    r"(?:\s*[*]+)?\s*\)"
)


class DonorCfgError(ValueError):
    """Raised when the evidence packet is incomplete or ambiguous."""


@dataclass(frozen=True)
class FunctionInfo:
    name: str
    source_label: str
    source_text: str
    function_text: str
    body_text: str
    source_start: int
    source_end: int
    body_start: int
    body_end: int
    start_line: int
    end_line: int
    body_start_line: int
    body_end_line: int


@dataclass(frozen=True)
class StatementInfo:
    start: int
    end: int
    masked: str
    text: str
    line_start: int
    line_end: int


@dataclass(frozen=True)
class AssignmentInfo:
    lhs: str
    op: str
    rhs: str
    start: int
    end: int
    line_start: int
    line_end: int
    text: str
    constant_only: bool


@dataclass(frozen=True)
class CallInfo:
    callee: str
    args: tuple[str, ...]
    start: int
    end: int
    line_start: int
    line_end: int
    text: str
    direct: bool
    assigned_lhs: str | None
    assignment: AssignmentInfo | None
    ordinal: int = 1


@dataclass(frozen=True)
class EventInfo:
    kind: str
    start: int
    end: int
    line_start: int
    line_end: int
    text: str
    depth: int
    condition: str


@dataclass(frozen=True)
class FunctionModel:
    info: FunctionInfo
    statements: tuple[StatementInfo, ...]
    assignments: tuple[AssignmentInfo, ...]
    calls: tuple[CallInfo, ...]
    events: tuple[EventInfo, ...]
    returns: tuple[EventInfo, ...]
    aggregates: tuple[dict[str, Any], ...]
    casts: tuple[dict[str, Any], ...]


def _fail(message: str) -> None:
    raise DonorCfgError(message)


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _mask_c(text: str) -> str:
    """Mask comments and literals while preserving offsets and newlines."""

    result = list(text)
    state = "code"
    quote = ""
    index = 0
    while index < len(text):
        char = text[index]
        if state == "code":
            if char == "/" and index + 1 < len(text) and text[index + 1] == "/":
                result[index] = " "
                result[index + 1] = " "
                state = "line_comment"
                index += 2
                continue
            if char == "/" and index + 1 < len(text) and text[index + 1] == "*":
                result[index] = " "
                result[index + 1] = " "
                state = "block_comment"
                index += 2
                continue
            if char in {"\"", "'"}:
                quote = char
                result[index] = " "
                state = "literal"
            index += 1
            continue
        if state == "line_comment":
            if char == "\n":
                state = "code"
            else:
                result[index] = " "
            index += 1
            continue
        if state == "block_comment":
            if char == "*" and index + 1 < len(text) and text[index + 1] == "/":
                result[index] = " "
                result[index + 1] = " "
                state = "code"
                index += 2
                continue
            if char != "\n":
                result[index] = " "
            index += 1
            continue
        # literal
        if char == "\\":
            result[index] = " "
            if index + 1 < len(text):
                if text[index + 1] != "\n":
                    result[index + 1] = " "
                index += 2
            else:
                index += 1
            continue
        if char == quote:
            result[index] = " "
            state = "code"
        elif char != "\n":
            result[index] = " "
        index += 1
    if state in {"block_comment", "literal"}:
        _fail("unterminated C comment or literal in source")
    return "".join(result)


def _matching(masked: str, opening: int, open_char: str, close_char: str, label: str) -> int:
    depth = 0
    for index in range(opening, len(masked)):
        char = masked[index]
        if char == open_char:
            depth += 1
        elif char == close_char:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                break
    _fail(f"unmatched {open_char!r} in {label}")
    raise AssertionError("unreachable")


def _line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, max(0, min(offset, len(text)))) + 1


def _line_span(text: str, start: int, end: int) -> tuple[int, int]:
    if end <= start:
        return _line_number(text, start), _line_number(text, start)
    return _line_number(text, start), _line_number(text, end - 1)


def _definition_candidates(masked: str, symbol: str | None, label: str) -> list[dict[str, int | str]]:
    pattern = re.compile(
        rf"(?<![A-Za-z0-9_]){re.escape(symbol)}(?![A-Za-z0-9_])"
        if symbol
        else r"(?<![A-Za-z0-9_])([A-Za-z_]\w*)(?![A-Za-z0-9_])"
    )
    candidates: list[dict[str, int | str]] = []
    for match in pattern.finditer(masked):
        identifier = match.start()
        name = symbol or str(match.group(1))
        prefix_start = max(
            masked.rfind(";", 0, identifier),
            masked.rfind("}", 0, identifier),
            masked.rfind("{", 0, identifier),
        ) + 1
        prefix = masked[prefix_start:identifier]
        if re.search(r"\b(?:if|for|while|switch|return)\s*\([^)]*$", prefix):
            continue
        if re.search(r"(?:->|\.)\s*$", prefix):
            continue
        # ``*`` immediately before a function name is valid pointer-return
        # declaration syntax (for example ``Vec *CapGuideRotYSet(...)``).
        # Other trailing operators still identify an expression rather than
        # a definition; the required ``{`` after the parameter list provides
        # the final definition guard below.
        if re.search(r"(?:=|,|\?|:|\+|-|/|%|!|&|\|)\s*$", prefix):
            continue
        after_name = identifier + len(name)
        while after_name < len(masked) and masked[after_name].isspace():
            after_name += 1
        if after_name >= len(masked) or masked[after_name] != "(":
            continue
        parameter_end = _matching(masked, after_name, "(", ")", f"function {name!r} in {label}")
        body_start = parameter_end + 1
        while body_start < len(masked) and masked[body_start].isspace():
            body_start += 1
        if body_start >= len(masked) or masked[body_start] != "{":
            continue
        body_end = _matching(masked, body_start, "{", "}", f"function {name!r} in {label}")
        candidates.append(
            {
                "name": name,
                "start": prefix_start,
                "body_start": body_start,
                "body_end": body_end,
            }
        )
    return candidates


def _extract_function(
    text: str,
    focus_symbol: str,
    label: str,
    *,
    allow_sole_donor_definition: bool = False,
) -> FunctionInfo:
    focus = focus_symbol.strip()
    if not focus or _IDENTIFIER_RE.fullmatch(focus) is None:
        _fail(f"{label}: focus symbol must be one C identifier")
    masked = _mask_c(text)
    candidates = _definition_candidates(masked, focus, label)
    if not candidates and allow_sole_donor_definition:
        all_candidates = _definition_candidates(masked, None, label)
        if len(all_candidates) == 1:
            candidates = all_candidates
    if not candidates:
        # A function-body input is useful for small probes and still has no
        # ambiguity if it contains exactly one balanced outer body.
        stripped_start = next((index for index, char in enumerate(masked) if not char.isspace()), None)
        if stripped_start is not None and masked[stripped_start] == "{":
            body_end = _matching(masked, stripped_start, "{", "}", f"function {focus!r} in {label}")
            trailing = masked[body_end + 1 :].strip()
            if not trailing:
                start = stripped_start
                end = body_end + 1
                start_line, end_line = _line_span(text, start, end)
                return FunctionInfo(
                    focus,
                    label,
                    text,
                    text[start:end],
                    text[start:end],
                    start,
                    end,
                    start,
                    end,
                    start_line,
                    end_line,
                    start_line,
                    end_line,
                )
        _fail(f"{label}: function {focus!r} was not found")
    if len(candidates) != 1:
        _fail(f"{label}: function {focus!r} is ambiguous ({len(candidates)} definitions)")
    candidate = candidates[0]
    start = int(candidate["start"])
    body_start = int(candidate["body_start"])
    body_end = int(candidate["body_end"])
    end = body_end + 1
    start_line, end_line = _line_span(text, start, end)
    body_start_line, body_end_line = _line_span(text, body_start, end)
    return FunctionInfo(
        str(candidate["name"]),
        label,
        text,
        text[start:end],
        text[body_start:end],
        start,
        end,
        body_start,
        end,
        start_line,
        end_line,
        body_start_line,
        body_end_line,
    )


def _read_text(value: str | Path, label: str) -> tuple[str, str, bytes | None]:
    if isinstance(value, Path):
        path = value
    else:
        path = Path(value)
    if path.is_file():
        try:
            data = path.read_bytes()
        except OSError as exc:
            _fail(f"cannot read {label} {path}: {exc}")
        try:
            return data.decode("utf-8"), str(path), data
        except UnicodeDecodeError as exc:
            _fail(f"{label} {path} is not UTF-8: {exc}")
    if isinstance(value, str) and ("\n" in value or "{" in value or "}" in value):
        return value, label, None
    _fail(f"{label} does not exist: {path}")
    raise AssertionError("unreachable")


def _source_descriptor(info: FunctionInfo, data: bytes | None, label: str) -> dict[str, Any]:
    encoded = data if data is not None else info.source_text.encode("utf-8")
    return {
        "path": label,
        "sha256": _sha256(encoded),
        "function": info.name,
        "function_lines": {"start": info.start_line, "end": info.end_line},
        "body_lines": {"start": info.body_start_line, "end": info.body_end_line},
    }


def _statement_segments(info: FunctionInfo, masked_body: str) -> list[StatementInfo]:
    segments: list[StatementInfo] = []
    start = 0
    paren = 0
    bracket = 0
    for index, char in enumerate(masked_body):
        if char == "(":
            paren += 1
        elif char == ")":
            paren = max(0, paren - 1)
        elif char == "[":
            bracket += 1
        elif char == "]":
            bracket = max(0, bracket - 1)
        elif char == ";" and paren == 0 and bracket == 0:
            end = index + 1
            line_start, line_end = _line_span(info.body_text, start, end)
            text = info.body_text[start:end]
            segments.append(StatementInfo(start, end, masked_body[start:end], text, line_start, line_end))
            start = end
    if masked_body[start:].strip():
        end = len(masked_body)
        line_start, line_end = _line_span(info.body_text, start, end)
        segments.append(
            StatementInfo(start, end, masked_body[start:end], info.body_text[start:end], line_start, line_end)
        )
    return segments


def _clean_expr(value: str) -> str:
    return re.sub(r"\s+", "", value.strip())


def _tokens(value: str) -> list[str]:
    return _TOKEN_RE.findall(value)


def _normalize_identity(value: str) -> str:
    return "".join(_tokens(value))


def _normalize_topology(value: str) -> str:
    tokens = _tokens(value)
    result: list[str] = []
    for index, token in enumerate(tokens):
        if _NUMBER_RE.fullmatch(token):
            result.append("CONST")
        elif token.startswith('"') or token.startswith("'"):
            result.append("STRING")
        elif _IDENTIFIER_RE.fullmatch(token):
            next_token = tokens[index + 1] if index + 1 < len(tokens) else ""
            if next_token == "(":
                result.append("CALL")
            elif index and tokens[index - 1] in {".", "->"}:
                result.append("FIELD")
            else:
                result.append("ID")
        else:
            result.append(token)
    return "".join(result)


def _without_numbers(value: str) -> str:
    # Replace complete numeric tokens only.  A raw regex substitution would
    # turn ``foo1`` and ``foo2`` into the same spelling and misclassify an
    # identifier change as constant-only shaping.
    return "".join(
        "#" if _NUMBER_RE.fullmatch(token) else token for token in _tokens(value)
    )


def _is_constant_only(left: str, right: str) -> bool:
    return _without_numbers(left) == _without_numbers(right) and _normalize_identity(left) != _normalize_identity(right)


def _constant_expression(value: str) -> bool:
    """Return whether an expression contains only literals/operators.

    Numeric suffixes (``1u``/``1.0f``), quoted literals, and a small set of
    punctuation operators are all constants for the source-shape guard.  A
    named identifier, call, or field makes the expression non-constant.
    """

    tokens = _tokens(value)
    if not tokens:
        return False
    for token in tokens:
        if _NUMBER_RE.fullmatch(token) or token.startswith('"') or token.startswith("'"):
            continue
        if token in {"(", ")", "[", "]", "+", "-", "*", "/", "%", "&", "|", "^", "~", "!", "?", ":", ",", "."}:
            continue
        return False
    return True


def _assignment_info(info: FunctionInfo, statements: Sequence[StatementInfo]) -> list[AssignmentInfo]:
    result: list[AssignmentInfo] = []
    for statement in statements:
        match = _ASSIGNMENT_RE.search(statement.masked)
        if match is None:
            continue
        prefix = statement.masked[: match.start()].strip()
        # An assignment embedded in a control condition is not a local
        # source-shape assignment for this tool.
        if re.search(r"\b(?:if|for|while|switch)\s*\([^)]*$", prefix):
            continue
        lhs = _clean_expr(match.group("lhs"))
        op = match.group("op")
        rhs_start = match.end()
        rhs = statement.masked[rhs_start:]
        rhs = rhs.rsplit(";", 1)[0].strip()
        text = statement.text.strip()
        result.append(
            AssignmentInfo(
                lhs,
                op,
                _clean_expr(rhs),
                statement.start,
                statement.end,
                statement.line_start,
                statement.line_end,
                text,
                _constant_expression(rhs),
            )
        )
    return result


def _call_args(masked: str, start: int, end: int) -> tuple[str, ...]:
    value = masked[start:end]
    if not value.strip():
        return ()
    result: list[str] = []
    piece_start = 0
    paren = 0
    bracket = 0
    brace = 0
    for index, char in enumerate(value):
        if char == "(":
            paren += 1
        elif char == ")":
            paren = max(0, paren - 1)
        elif char == "[":
            bracket += 1
        elif char == "]":
            bracket = max(0, bracket - 1)
        elif char == "{":
            brace += 1
        elif char == "}":
            brace = max(0, brace - 1)
        elif char == "," and not paren and not bracket and not brace:
            result.append(value[piece_start:index].strip())
            piece_start = index + 1
    result.append(value[piece_start:].strip())
    return tuple(part for part in result if part)


def _calls(info: FunctionInfo, masked_body: str, assignments: Sequence[AssignmentInfo]) -> list[CallInfo]:
    calls: list[CallInfo] = []
    ordinal_by_name: dict[str, int] = {}
    for match in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", masked_body):
        callee = match.group(1)
        if callee in _CONTROL_WORDS:
            continue
        opening = masked_body.find("(", match.start(), match.end())
        if opening < 0:
            continue
        closing = _matching(masked_body, opening, "(", ")", f"call {callee!r} in {info.source_label}")
        after = closing + 1
        while after < len(masked_body) and masked_body[after].isspace():
            after += 1
        if after < len(masked_body) and masked_body[after] == "{" :
            # This is a nested definition/declaration, not a call.
            continue
        previous = match.start() - 1
        while previous >= 0 and masked_body[previous].isspace():
            previous -= 1
        direct = previous < 0 or masked_body[previous] not in {".", ">"}
        args = _call_args(masked_body, opening + 1, closing)
        line_start, line_end = _line_span(info.body_text, match.start(), closing + 1)
        assignment = next(
            (
                item
                for item in assignments
                if item.start <= match.start() < item.end and item.lhs
            ),
            None,
        )
        ordinal = ordinal_by_name.get(callee, 0) + 1
        ordinal_by_name[callee] = ordinal
        calls.append(
            CallInfo(
                callee,
                args,
                match.start(),
                closing + 1,
                line_start,
                line_end,
                info.body_text[match.start() : closing + 1].strip(),
                direct,
                assignment.lhs if assignment else None,
                assignment,
                ordinal,
            )
        )
    return calls


def _events(info: FunctionInfo, masked_body: str) -> tuple[list[EventInfo], list[EventInfo]]:
    events: list[EventInfo] = []
    returns: list[EventInfo] = []
    depth = 0
    for index, char in enumerate(masked_body):
        if char == "{":
            depth += 1
        elif char == "}":
            depth = max(0, depth - 1)
        if not (char.isalpha() or char == "_"):
            continue
        match = re.match(r"(?:if|else|switch|case|for|while|do)\b", masked_body[index:])
        if match is None:
            return_match = re.match(r"return\b", masked_body[index:])
            if return_match is None:
                continue
            end = index + len(return_match.group(0))
            semicolon = masked_body.find(";", end)
            if semicolon < 0:
                semicolon = min(len(masked_body), end + 120)
            line_start, line_end = _line_span(info.body_text, index, semicolon + 1)
            returns.append(
                EventInfo(
                    "return",
                    index,
                    semicolon + 1,
                    line_start,
                    line_end,
                    info.body_text[index : semicolon + 1].strip(),
                    depth,
                    _normalize_topology(masked_body[end:semicolon]),
                )
            )
            continue
        kind = match.group(0)
        end = index + len(kind)
        condition = ""
        if kind in {"if", "switch", "for", "while"}:
            opening = end
            while opening < len(masked_body) and masked_body[opening].isspace():
                opening += 1
            if opening < len(masked_body) and masked_body[opening] == "(":
                closing = _matching(masked_body, opening, "(", ")", f"{kind} condition in {info.source_label}")
                end = closing + 1
                condition = _normalize_topology(masked_body[opening + 1 : closing])
        line_start, line_end = _line_span(info.body_text, index, end)
        events.append(
            EventInfo(
                kind,
                index,
                end,
                line_start,
                line_end,
                info.body_text[index:end].strip(),
                depth,
                condition,
            )
        )
    return events, returns


def _aggregate_records(info: FunctionInfo, masked_body: str, assignments: Sequence[AssignmentInfo]) -> list[dict[str, Any]]:
    records: dict[str, dict[str, Any]] = {}
    for match in re.finditer(
        r"\b(?:struct|union)\s+[A-Za-z_]\w*\s+([A-Za-z_]\w*)(?:\s*\[[^\]]+\])?\s*(?:[;=])",
        masked_body,
    ):
        name = match.group(1)
        line_start, line_end = _line_span(info.body_text, match.start(), match.end())
        records.setdefault(
            name,
            {
                "name": name,
                "kind": "aggregate",
                "fields": [],
                "line_start": line_start,
                "line_end": line_end,
                "text": info.body_text[match.start() : match.end()].strip(),
            },
        )
    for match in re.finditer(
        r"\b(?:[A-Za-z_]\w*\s*[*]?\s+)([A-Za-z_]\w*)\s*\[[^\]]+\]\s*(?:[;=])",
        masked_body,
    ):
        name = match.group(1)
        line_start, line_end = _line_span(info.body_text, match.start(), match.end())
        records.setdefault(
            name,
            {
                "name": name,
                "kind": "array",
                "fields": [],
                "line_start": line_start,
                "line_end": line_end,
                "text": info.body_text[match.start() : match.end()].strip(),
            },
        )
    for match in re.finditer(
        r"\b([A-Za-z_]\w*)\s*(?:\.|->)\s*([A-Za-z_]\w*)\s*=",
        masked_body,
    ):
        name, field = match.group(1), match.group(2)
        line_start, line_end = _line_span(info.body_text, match.start(), match.end())
        record = records.setdefault(
            name,
            {
                "name": name,
                "kind": "field-owner",
                "fields": [],
                "line_start": line_start,
                "line_end": line_end,
                "text": info.body_text[match.start() : match.end()].strip(),
            },
        )
        if field not in record["fields"]:
            record["fields"].append(field)
            record["line_end"] = line_end
    # A scalar local with a lone field-like assignment is not enough to infer
    # an aggregate.  Require a declaration or at least two distinct fields.
    return [
        record
        for record in records.values()
        if record["kind"] in {"aggregate", "array"} or len(record["fields"]) >= 2
    ]


def _cast_records(info: FunctionInfo, masked_body: str, calls: Sequence[CallInfo]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    for match in _CAST_RE.finditer(masked_body):
        end = match.end()
        line_start, line_end = _line_span(info.body_text, match.start(), end)
        containing = next((call for call in calls if call.start <= match.start() < call.end), None)
        result.append(
            {
                "text": info.body_text[match.start() : end].strip(),
                "line_start": line_start,
                "line_end": line_end,
                "call": containing.callee if containing else None,
                "start": match.start(),
                "end": end,
            }
        )
    return result


def _model(info: FunctionInfo) -> FunctionModel:
    masked_body = _mask_c(info.body_text)
    statements = _statement_segments(info, masked_body)
    assignments = _assignment_info(info, statements)
    calls = _calls(info, masked_body, assignments)
    events, returns = _events(info, masked_body)
    aggregates = _aggregate_records(info, masked_body, assignments)
    casts = _cast_records(info, masked_body, calls)
    return FunctionModel(info, tuple(statements), tuple(assignments), tuple(calls), tuple(events), tuple(returns), tuple(aggregates), tuple(casts))


def _pointer_output(info: FunctionInfo) -> bool:
    """Whether the selected function's return declaration contains a pointer."""

    opening = info.function_text.find("{")
    header = info.function_text if opening < 0 else info.function_text[:opening]
    name_index = header.rfind(info.name)
    if name_index < 0:
        return False
    return "*" in header[:name_index]


def _span(model: FunctionModel, start: int, end: int, role: str) -> dict[str, Any]:
    info = model.info
    line_start, line_end = _line_span(info.body_text, start, end)
    snippet = info.body_text[start:end].strip()
    if len(snippet) > 480:
        snippet = snippet[:477] + "..."
    return {
        "side": "donor" if role.startswith("donor") else "current",
        "role": role,
        "path": info.source_label,
        "line_start": line_start + info.body_start_line - 1,
        "line_end": line_end + info.body_start_line - 1,
        "start_line": line_start + info.body_start_line - 1,
        "end_line": line_end + info.body_start_line - 1,
        "snippet": snippet,
    }


def _object_span(side: str, path: str, role: str, json_path: str, value: Any) -> dict[str, Any]:
    return {
        "side": side,
        "role": role,
        "path": path,
        "json_path": json_path,
        "value": value,
    }


def _load_report(value: Mapping[str, Any] | str | Path) -> tuple[Mapping[str, Any], str, str | None]:
    if isinstance(value, Mapping):
        return value, "<mapping>", None
    path = Path(value)
    if path.is_file():
        try:
            data = path.read_bytes()
            parsed = json.loads(data.decode("utf-8"))
        except FileNotFoundError:
            _fail(f"objdiff report does not exist: {path}")
        except UnicodeDecodeError as exc:
            _fail(f"objdiff report is not UTF-8: {path}: {exc}")
        except json.JSONDecodeError as exc:
            _fail(f"invalid objdiff report {path}:{exc.lineno}:{exc.colno}: {exc.msg}")
        if not isinstance(parsed, Mapping):
            _fail(f"objdiff report must contain an object: {path}")
        return parsed, str(path), _sha256(data)
    if isinstance(value, str) and value.lstrip().startswith("{"):
        try:
            parsed = json.loads(value)
        except json.JSONDecodeError as exc:
            _fail(f"invalid inline objdiff report:{exc.lineno}:{exc.colno}: {exc.msg}")
        if not isinstance(parsed, Mapping):
            _fail("inline objdiff report must contain an object")
        return parsed, "<inline>", None
    _fail(f"objdiff report does not exist: {path}")
    raise AssertionError("unreachable")


def _symbols(side: Any) -> list[tuple[int, Mapping[str, Any]]]:
    if not isinstance(side, Mapping):
        return []
    value = side.get("symbols")
    if not isinstance(value, list):
        value = side.get("functions")
    if not isinstance(value, list):
        if _symbol_name(side) is not None:
            return [(0, side)]
        return []
    return [(index, item) for index, item in enumerate(value) if isinstance(item, Mapping)]


def _symbol_name(item: Mapping[str, Any]) -> str | None:
    for key in ("name", "symbol", "function", "label"):
        value = item.get(key)
        if isinstance(value, str) and value:
            return value
    return None


def _symbol_function(item: Mapping[str, Any]) -> bool:
    kind = item.get("kind")
    return kind is None or str(kind).upper() in {"SYMBOL_FUNCTION", "FUNCTION", "FUNC"}


def _pointer(item: Mapping[str, Any], keys: Sequence[str]) -> int | None:
    """Read a legacy pointer without treating malformed values as absent."""

    pointer, _ = _pointer_field(item, keys, "symbol")
    return pointer


def _pointer_field(
    item: Mapping[str, Any],
    keys: Sequence[str],
    label: str,
) -> tuple[int | None, str | None]:
    """Return one pointer and its field, rejecting malformed/conflicting fields.

    Objdiff variants use ``target_symbol``, ``left_symbol``, or
    ``paired_symbol`` for the same relationship.  A present field is evidence,
    not an optional hint: booleans, nulls, out-of-range values, and conflicting
    aliases must not degrade into a same-name pairing.
    """

    values: list[tuple[str, int]] = []
    for key in keys:
        if key not in item:
            continue
        raw = item[key]
        if isinstance(raw, bool):
            _fail(f"{label} pointer {key!r} is not an integer")
        if isinstance(raw, int):
            values.append((key, raw))
            continue
        if isinstance(raw, str) and re.fullmatch(r"-?\d+", raw.strip()):
            values.append((key, int(raw.strip())))
            continue
        _fail(f"{label} pointer {key!r} is malformed")
    if not values:
        return None, None
    unique = {value for _, value in values}
    if len(unique) != 1:
        detail = ", ".join(f"{key}={value}" for key, value in values)
        _fail(f"{label} pointer aliases conflict ({detail})")
    key, value = values[0]
    return value, key


def _metric(item: Mapping[str, Any]) -> Any:
    for key in ("match_percent", "matchPercent", "match", "percent", "score"):
        if key in item:
            return item[key]
    return None


def _instruction_profile(item: Mapping[str, Any]) -> dict[str, Any]:
    rows = item.get("instructions")
    if not isinstance(rows, list):
        rows = item.get("instruction_diff")
    if not isinstance(rows, list):
        rows = []
    kinds: dict[str, int] = {}
    changed = 0
    snippets: list[str] = []
    for row in rows:
        if not isinstance(row, Mapping):
            continue
        kind = row.get("diff_kind", row.get("kind"))
        if kind:
            key = str(kind)
            kinds[key] = kinds.get(key, 0) + 1
            changed += 1
        instruction = row.get("instruction")
        if isinstance(instruction, Mapping):
            formatted = instruction.get("formatted")
            if isinstance(formatted, str) and formatted:
                snippets.append(formatted)
        elif isinstance(row.get("formatted"), str):
            snippets.append(str(row["formatted"]))
    return {
        "changed_rows": changed,
        "diff_kinds": dict(sorted(kinds.items())),
        "instruction_examples": sorted(set(snippets))[:8],
    }


def _report_authentication(
    report: Mapping[str, Any],
    report_path: str,
    report_hash: str | None,
    focus: str,
) -> dict[str, Any]:
    """Resolve one explicit target/candidate pair from common objdiff shapes."""

    target_side: Any = None
    candidate_side: Any = None
    target_path = ""
    candidate_path = ""
    convention = ""
    if isinstance(report.get("target"), Mapping) and isinstance(report.get("candidate"), Mapping):
        target_side = report["target"]
        candidate_side = report["candidate"]
        target_path, candidate_path = "$.target", "$.candidate"
        convention = "target_candidate"
    elif isinstance(report.get("baseline"), Mapping) and isinstance(report.get("candidate"), Mapping):
        target_side = report["baseline"]
        candidate_side = report["candidate"]
        target_path, candidate_path = "$.baseline", "$.candidate"
        convention = "baseline_candidate"
    elif isinstance(report.get("left"), Mapping) and isinstance(report.get("right"), Mapping):
        # objdiff's left symbol carries target_symbol, so left is the
        # candidate and right is the target in the repository's reports.
        candidate_side = report["left"]
        target_side = report["right"]
        target_path, candidate_path = "$.right", "$.left"
        convention = "objdiff_left_candidate"
    else:
        rows = report.get("functions")
        if isinstance(rows, list):
            matches = [
                (index, row)
                for index, row in enumerate(rows)
                if isinstance(row, Mapping) and _symbol_name(row) == focus
            ]
            if len(matches) != 1:
                if not matches:
                    _fail(f"objdiff report has no unambiguous focus symbol {focus!r}")
                _fail(f"objdiff report has ambiguous focus symbol {focus!r}")
            row_index, row = matches[0]
            target_row = row.get("target") if isinstance(row.get("target"), Mapping) else row
            candidate_row = row.get("candidate") if isinstance(row.get("candidate"), Mapping) else row
            target_side = {"symbols": [target_row]}
            candidate_side = {"symbols": [candidate_row]}
            target_path = f"$.functions[{row_index}].target"
            candidate_path = f"$.functions[{row_index}].candidate"
            convention = "compact_function"
        else:
            _fail("objdiff report has no target/candidate symbol sides")

    target_symbols = _symbols(target_side)
    candidate_symbols = _symbols(candidate_side)
    target_matches = [item for item in target_symbols if _symbol_name(item[1]) == focus and _symbol_function(item[1])]
    candidate_matches = [item for item in candidate_symbols if _symbol_name(item[1]) == focus and _symbol_function(item[1])]
    if len(target_matches) > 1 or len(candidate_matches) > 1:
        _fail(f"objdiff report has ambiguous focus symbol {focus!r}")
    target_entry = target_matches[0] if target_matches else None
    candidate_entry = candidate_matches[0] if candidate_matches else None

    # Pair pointers are explicit on normal objdiff output.  Resolve either
    # direction, but validate every pointer on the focus entry before allowing
    # the conservative same-name fallback.  A stale/out-of-range pointer is
    # evidence corruption, not permission to infer a pair by spelling.
    target_pointer: int | None = None
    candidate_pointer: int | None = None
    target_pointer_key: str | None = None
    candidate_pointer_key: str | None = None
    target_pointer_keys = ("candidate_symbol", "left_symbol", "paired_symbol", "target_symbol")
    candidate_pointer_keys = ("target_symbol", "right_symbol", "paired_symbol", "candidate_symbol")

    if target_entry is not None:
        target_pointer, target_pointer_key = _pointer_field(
            target_entry[1], target_pointer_keys, "target"
        )
        if target_pointer is not None and not 0 <= target_pointer < len(candidate_symbols):
            _fail(
                f"target pointer {target_pointer_key}={target_pointer} is out of range "
                f"for {len(candidate_symbols)} candidate symbols"
            )
    if candidate_entry is not None:
        candidate_pointer, candidate_pointer_key = _pointer_field(
            candidate_entry[1], candidate_pointer_keys, "candidate"
        )
        if candidate_pointer is not None and not 0 <= candidate_pointer < len(target_symbols):
            _fail(
                f"candidate pointer {candidate_pointer_key}={candidate_pointer} is out of range "
                f"for {len(target_symbols)} target symbols"
            )
    if target_entry is not None and target_pointer is not None:
        pointed_candidate = next(
            (item for item in candidate_symbols if item[0] == target_pointer), None
        )
        if candidate_entry is not None and candidate_entry[0] != target_pointer:
            _fail(
                f"target pointer {target_pointer_key}={target_pointer} conflicts with "
                f"candidate focus index {candidate_entry[0]}"
            )
        candidate_entry = pointed_candidate
    if candidate_entry is not None and candidate_pointer is not None:
        pointed_target = next(
            (item for item in target_symbols if item[0] == candidate_pointer), None
        )
        if target_entry is not None and target_entry[0] != candidate_pointer:
            _fail(
                f"candidate pointer {candidate_pointer_key}={candidate_pointer} conflicts with "
                f"target focus index {target_entry[0]}"
            )
        target_entry = pointed_target

    # If a focus name exists on only one side, a valid explicit pointer may
    # identify the other side.  Resolve that pointer only after range checks.
    if target_entry is None and candidate_entry is not None:
        candidate_pointer, candidate_pointer_key = _pointer_field(
            candidate_entry[1], candidate_pointer_keys, "candidate"
        )
        if candidate_pointer is not None and not 0 <= candidate_pointer < len(target_symbols):
            _fail(
                f"candidate pointer {candidate_pointer_key}={candidate_pointer} is out of range "
                f"for {len(target_symbols)} target symbols"
            )
        if candidate_pointer is not None:
            target_entry = next(
                (item for item in target_symbols if item[0] == candidate_pointer), None
            )
    if candidate_entry is None and target_entry is not None:
        target_pointer, target_pointer_key = _pointer_field(
            target_entry[1], target_pointer_keys, "target"
        )
        if target_pointer is not None and not 0 <= target_pointer < len(candidate_symbols):
            _fail(
                f"target pointer {target_pointer_key}={target_pointer} is out of range "
                f"for {len(candidate_symbols)} candidate symbols"
            )
        if target_pointer is not None:
            candidate_entry = next(
                (item for item in candidate_symbols if item[0] == target_pointer), None
            )

    # Name fallback is allowed only when no explicit pointer was present on the
    # selected focus record.  Never use it to recover from a pointer conflict.
    if target_entry is None and candidate_entry is not None and candidate_pointer is None:
        same_name = [
            item for item in target_symbols
            if _symbol_name(item[1]) == _symbol_name(candidate_entry[1])
        ]
        if len(same_name) == 1:
            target_entry = same_name[0]
    if candidate_entry is None and target_entry is not None and target_pointer is None:
        same_name = [
            item for item in candidate_symbols
            if _symbol_name(item[1]) == _symbol_name(target_entry[1])
        ]
        if len(same_name) == 1:
            candidate_entry = same_name[0]
    if target_entry is None or candidate_entry is None:
        _fail(f"objdiff report does not pair focus symbol {focus!r} on both sides")
    target_index, target_symbol = target_entry
    candidate_index, candidate_symbol = candidate_entry
    if not _symbol_function(target_symbol) or not _symbol_function(candidate_symbol):
        _fail(f"objdiff focus symbol {focus!r} is not a function on both sides")
    # Re-read after pointer-based resolution so a pointer on the selected
    # counterpart is also checked for conflicts/out-of-range values.
    target_pointer, target_pointer_key = _pointer_field(
        target_symbol, target_pointer_keys, "target"
    )
    candidate_pointer, candidate_pointer_key = _pointer_field(
        candidate_symbol, candidate_pointer_keys, "candidate"
    )
    if target_pointer is not None:
        if not 0 <= target_pointer < len(candidate_symbols):
            _fail(
                f"target pointer {target_pointer_key}={target_pointer} is out of range "
                f"for {len(candidate_symbols)} candidate symbols"
            )
        if target_pointer != candidate_index:
            _fail(
                f"target pointer {target_pointer_key}={target_pointer} conflicts with "
                f"candidate index {candidate_index}"
            )
    if candidate_pointer is not None:
        if not 0 <= candidate_pointer < len(target_symbols):
            _fail(
                f"candidate pointer {candidate_pointer_key}={candidate_pointer} is out of range "
                f"for {len(target_symbols)} target symbols"
            )
        if candidate_pointer != target_index:
            _fail(
                f"candidate pointer {candidate_pointer_key}={candidate_pointer} conflicts with "
                f"target index {target_index}"
            )
    explicit_pair = target_pointer is not None or candidate_pointer is not None
    pair_confidence = 0.94 if explicit_pair else 0.78
    match_value = _metric(candidate_symbol)
    if match_value is None:
        match_value = _metric(target_symbol)
    profile = _instruction_profile(candidate_symbol)
    return {
        "status": "authenticated",
        "report": {"path": report_path, "sha256": report_hash},
        "convention": convention,
        "focus_symbol": focus,
        "target": {
            "symbol": _symbol_name(target_symbol),
            "index": target_index,
            "json_path": f"{target_path}.symbols[{target_index}]",
            "size": target_symbol.get("size"),
            "match_percent": _metric(target_symbol),
        },
        "candidate": {
            "symbol": _symbol_name(candidate_symbol),
            "index": candidate_index,
            "json_path": f"{candidate_path}.symbols[{candidate_index}]",
            "size": candidate_symbol.get("size"),
            "match_percent": _metric(candidate_symbol),
            "instruction_profile": profile,
        },
        "pairing": {
            "explicit": explicit_pair,
            "mode": "pointer" if explicit_pair else "same-name",
            "confidence": pair_confidence,
            "candidate_index": candidate_index,
            "target_index": target_index,
        },
        "match_percent": match_value,
        "evidence": [
            _object_span("report", report_path, "target-focus", f"{target_path}.symbols[{target_index}]", _symbol_name(target_symbol)),
            _object_span("report", report_path, "candidate-focus", f"{candidate_path}.symbols[{candidate_index}]", _symbol_name(candidate_symbol)),
            _object_span("report", report_path, "candidate-target-pair", f"{candidate_path}.symbols[{candidate_index}].target_symbol", target_index),
            _object_span("report", report_path, "candidate-instruction-diff", f"{candidate_path}.symbols[{candidate_index}].instructions", profile),
        ],
    }


def _source_span_for_assignment(model: FunctionModel, assignment: AssignmentInfo, role: str) -> dict[str, Any]:
    return _span(model, assignment.start, assignment.end, role)


def _source_span_for_call(model: FunctionModel, call: CallInfo, role: str) -> dict[str, Any]:
    return _span(model, call.start, call.end, role)


def _assignment_lhs_span(
    model: FunctionModel, assignment: AssignmentInfo
) -> tuple[int, int] | None:
    """Return the absolute body offsets occupied by one assignment LHS."""

    masked = _mask_c(model.info.body_text)
    segment = masked[assignment.start : assignment.end]
    match = _ASSIGNMENT_RE.search(segment)
    if match is None:
        return None
    return assignment.start + match.start("lhs"), assignment.start + match.end("lhs")


def _read_occurrences(
    model: FunctionModel,
    start: int,
    base: str,
) -> list[int]:
    """Find reads of ``base`` after ``start``, excluding later write LHSes."""

    if not re.fullmatch(r"[A-Za-z_]\w*", base):
        return []
    masked = _mask_c(model.info.body_text)
    lhs_spans = {
        span
        for assignment in model.assignments
        if (span := _assignment_lhs_span(model, assignment)) is not None
    }
    pattern = re.compile(rf"\b{re.escape(base)}\b")
    return [
        match.start()
        for match in pattern.finditer(masked, start)
        if not any(lhs_start <= match.start() < lhs_end for lhs_start, lhs_end in lhs_spans)
    ]


def _result_lifetime(model: FunctionModel, call: CallInfo) -> dict[str, Any]:
    if call.assignment is None:
        storage = "<direct>"
        start = call.end
    else:
        storage = call.assignment.lhs
        start = call.end
    uses: list[int] = []
    return_use = False
    if storage != "<direct>":
        name = re.match(r"[A-Za-z_]\w*", storage)
        if name is not None:
            base = name.group(0)
            for offset in _read_occurrences(model, start, base):
                line = _line_number(model.info.body_text, offset) + model.info.body_start_line - 1
                uses.append(line)
            pattern = re.compile(rf"\b{re.escape(base)}\b")
            for event in model.returns:
                if event.start >= start and pattern.search(_mask_c(model.info.body_text)[event.start:event.end]):
                    return_use = True
    direct_return = any(event.start <= call.start < event.end for event in model.returns)
    return {
        "callee": call.callee,
        "ordinal": call.ordinal,
        "storage": storage,
        "uses_after": uses,
        "use_count": len(uses),
        "return_use": return_use or direct_return,
    }


def _dead_assignment(model: FunctionModel, assignment: AssignmentInfo | None) -> bool:
    """Detect an assignment whose local value is never read afterward."""

    if assignment is None:
        return False
    base = assignment.lhs.split(".", 1)[0].split("->", 1)[0]
    if not re.fullmatch(r"[A-Za-z_]\w*", base):
        return False
    return not _read_occurrences(model, assignment.end, base)


def _registered_assignment(model: FunctionModel, assignment: AssignmentInfo | None) -> bool:
    """Detect a local explicitly marked ``register`` near an assignment."""

    if assignment is None:
        return False
    base = assignment.lhs.split(".", 1)[0].split("->", 1)[0]
    if not re.fullmatch(r"[A-Za-z_]\w*", base):
        return False
    masked = _mask_c(model.info.body_text)
    registered_names = {
        match.group(1)
        for match in re.finditer(r"\bregister\b[^;{}]*?\b([A-Za-z_]\w*)\s*(?==|;|,)", masked)
    }
    return base in registered_names


def _hypothesis(
    *,
    kind: str,
    subject: str,
    title: str,
    recommendation: str,
    score: float,
    evidence: Sequence[Mapping[str, Any]],
    change: str | None = None,
    guard: str | None = None,
    requires_prototype: bool = False,
) -> dict[str, Any]:
    score = round(max(0.0, min(0.99, score)), 3)
    label = "high" if score >= 0.8 else "medium" if score >= 0.6 else "low"
    result: dict[str, Any] = {
        "kind": kind,
        "category": kind,
        "subject": subject,
        "title": title,
        "recommendation": recommendation,
        "confidence": {"score": score, "label": label},
        "confidence_score": score,
        "confidence_label": label,
        "safe": True,
        "evidence": [dict(item) for item in evidence],
        "evidence_spans": [dict(item) for item in evidence],
        "guard": guard or "donor source shape only; compile and re-check target consumers before adoption",
    }
    if change is not None:
        result["change"] = change
    if requires_prototype:
        result["requires_prototype_evidence"] = True
    return result


def _report_evidence(auth: Mapping[str, Any]) -> list[dict[str, Any]]:
    evidence = auth.get("evidence")
    if not isinstance(evidence, list):
        return []
    return [dict(item) for item in evidence if isinstance(item, Mapping)]


def _line_span_for_record(model: FunctionModel, record: Mapping[str, Any], role: str) -> dict[str, Any]:
    base_line = model.info.body_start_line - 1
    line_start = int(record.get("line_start", 1)) + base_line
    line_end = int(record.get("line_end", 1)) + base_line
    return {
        "side": "donor" if role.startswith("donor") else "current",
        "role": role,
        "path": model.info.source_label,
        "line_start": line_start,
        "line_end": line_end,
        "start_line": line_start,
        "end_line": line_end,
        "snippet": str(record.get("text", "")),
    }


def _rank_model_differences(current: FunctionModel, donor: FunctionModel, auth: Mapping[str, Any]) -> tuple[list[dict[str, Any]], list[str]]:
    report_evidence = _report_evidence(auth)
    auth_score = float(auth.get("pairing", {}).get("confidence", 0.78))
    hypotheses: list[dict[str, Any]] = []
    closed: set[str] = set()
    pointer_output = _pointer_output(current.info) or _pointer_output(donor.info)
    explicit_pair = bool(auth.get("pairing", {}).get("explicit"))
    if pointer_output:
        closed.add("pointer-output aggregate/assignment shaping is closed")
    if not explicit_pair:
        closed.add("non-explicit target/candidate pairing closes assignment/aggregate shaping")

    current_by_name: dict[str, list[CallInfo]] = {}
    donor_by_name: dict[str, list[CallInfo]] = {}
    for call in current.calls:
        current_by_name.setdefault(call.callee, []).append(call)
    for call in donor.calls:
        donor_by_name.setdefault(call.callee, []).append(call)
    all_callees = sorted(set(current_by_name) | set(donor_by_name))
    for callee in all_callees:
        current_calls = current_by_name.get(callee, [])
        donor_calls = donor_by_name.get(callee, [])
        for index in range(min(len(current_calls), len(donor_calls))):
            current_call = current_calls[index]
            donor_call = donor_calls[index]
            if len(current_call.args) != len(donor_call.args):
                hypotheses.append(
                    _hypothesis(
                        kind="call_argument_topology",
                        subject=f"{callee}#{index + 1}",
                        title=f"{callee} has {len(donor_call.args)} donor arguments versus {len(current_call.args)} current arguments",
                        recommendation="Compare the authenticated call boundary and prototype before changing argument count or nesting.",
                        score=0.55 + auth_score * 0.35,
                        evidence=[
                            _source_span_for_call(donor, donor_call, "donor-call"),
                            _source_span_for_call(current, current_call, "current-call"),
                            *report_evidence,
                        ],
                    )
                )
                continue
            for arg_index, (current_arg, donor_arg) in enumerate(zip(current_call.args, donor_call.args), 1):
                current_identity = _normalize_identity(current_arg)
                donor_identity = _normalize_identity(donor_arg)
                if current_identity == donor_identity:
                    continue
                if _is_constant_only(current_arg, donor_arg):
                    closed.add("constant-only argument differences are closed")
                    continue
                current_topology = _normalize_topology(current_arg)
                donor_topology = _normalize_topology(donor_arg)
                kind = "call_argument_identity" if current_topology == donor_topology else "call_argument_topology"
                hypotheses.append(
                    _hypothesis(
                        kind=kind,
                        subject=f"{callee}#{index + 1}:arg{arg_index}",
                        title=f"{callee} argument {arg_index} differs in donor identity/topology",
                        recommendation=f"Review donor expression {donor_arg.strip()} at the authenticated call boundary; current expression is {current_arg.strip()}.",
                        score=0.59 + auth_score * 0.36,
                        evidence=[
                            _source_span_for_call(donor, donor_call, "donor-call-argument"),
                            _source_span_for_call(current, current_call, "current-call-argument"),
                            *report_evidence,
                        ],
                        change="donor_expression_vs_current_expression",
                    )
                )
        if len(donor_calls) > len(current_calls):
            for donor_call in donor_calls[len(current_calls) :]:
                if donor_call.assignment is not None and donor_call.assignment.constant_only:
                    closed.add("constant-only call assignments are closed")
                    continue
                if _registered_assignment(donor, donor_call.assignment) or (
                    donor_call.assignment is not None and re.search(r"\bregister\b", donor_call.assignment.text)
                ):
                    closed.add("register storage shaping is closed")
                    continue
                if _dead_assignment(donor, donor_call.assignment):
                    closed.add("unused/dead-local call results are closed")
                    continue
                hypotheses.append(
                    _hypothesis(
                        kind="direct_call_inlining",
                        subject=f"{callee}#{donor_call.ordinal}",
                        title=f"donor retains direct call {callee} while current source has no corresponding call",
                        recommendation="Check whether the target preserves a direct helper-call boundary instead of relying on an inline expansion.",
                        score=0.53 + auth_score * 0.34,
                        evidence=[_source_span_for_call(donor, donor_call, "donor-direct-call"), *report_evidence],
                        change="donor_call_present_current_call_absent",
                    )
                )
        elif len(current_calls) > len(donor_calls):
            for current_call in current_calls[len(donor_calls) :]:
                if current_call.assignment is not None and current_call.assignment.constant_only:
                    closed.add("constant-only call assignments are closed")
                    continue
                if _registered_assignment(current, current_call.assignment) or (
                    current_call.assignment is not None and re.search(r"\bregister\b", current_call.assignment.text)
                ):
                    closed.add("register storage shaping is closed")
                    continue
                if _dead_assignment(current, current_call.assignment):
                    closed.add("unused/dead-local call results are closed")
                    continue
                hypotheses.append(
                    _hypothesis(
                        kind="direct_call_inlining",
                        subject=f"{callee}#{current_call.ordinal}",
                        title=f"current source retains direct call {callee} while donor source has no corresponding call",
                        recommendation="Treat the donor's missing call as an inlining-boundary clue only; do not force a helper boundary without target evidence.",
                        score=0.48 + auth_score * 0.31,
                        evidence=[_source_span_for_call(current, current_call, "current-direct-call"), *report_evidence],
                        change="current_call_present_donor_call_absent",
                    )
                )

    # Assignment presence is matched by operation and expression topology first
    # so a donor's temporary name is not mistaken for a missing semantic value.
    current_assignments = (
        []
        if pointer_output or not explicit_pair
        else [item for item in current.assignments if not item.constant_only]
    )
    donor_assignments = (
        []
        if pointer_output or not explicit_pair
        else [item for item in donor.assignments if not item.constant_only]
    )
    for item in current.assignments:
        if item.constant_only:
            closed.add("constant-only assignments are closed")
    for item in donor.assignments:
        if item.constant_only:
            closed.add("constant-only assignments are closed")
    current_keys = [(item.op, _normalize_topology(item.rhs)) for item in current_assignments]
    donor_keys = [(item.op, _normalize_topology(item.rhs)) for item in donor_assignments]
    used_current: set[int] = set()
    for donor_index, donor_assignment in enumerate(donor_assignments):
        key = donor_keys[donor_index]
        match_index = next((index for index, item in enumerate(current_keys) if index not in used_current and item == key), None)
        if match_index is not None:
            used_current.add(match_index)
            continue
        if _registered_assignment(donor, donor_assignment) or re.search(r"\bregister\b", donor_assignment.text):
            closed.add("register storage shaping is closed")
            continue
        if _dead_assignment(donor, donor_assignment):
            closed.add("unused/dead-local assignments are closed")
            continue
        hypotheses.append(
            _hypothesis(
                kind="missing_assignment",
                subject=donor_assignment.lhs,
                title=f"donor contains assignment to {donor_assignment.lhs} with no current counterpart",
                recommendation="Review whether this non-constant assignment is the target's missing source-level materialization.",
                score=0.5 + auth_score * 0.33,
                evidence=[_source_span_for_assignment(donor, donor_assignment, "donor-assignment"), *report_evidence],
                change="donor_assignment_missing_current",
            )
        )
    used_donor: set[int] = set()
    for current_index, current_assignment in enumerate(current_assignments):
        key = current_keys[current_index]
        match_index = next((index for index, item in enumerate(donor_keys) if index not in used_donor and item == key), None)
        if match_index is not None:
            used_donor.add(match_index)
            continue
        if _registered_assignment(current, current_assignment) or re.search(r"\bregister\b", current_assignment.text):
            closed.add("register storage shaping is closed")
            continue
        if _dead_assignment(current, current_assignment):
            closed.add("unused/dead-local assignments are closed")
            continue
        hypotheses.append(
            _hypothesis(
                kind="extra_assignment",
                subject=current_assignment.lhs,
                title=f"current contains assignment to {current_assignment.lhs} with no donor counterpart",
                recommendation="Treat the donor's absence as a source-shape clue only; remove an assignment only after target consumers support it.",
                score=0.46 + auth_score * 0.3,
                evidence=[_source_span_for_assignment(current, current_assignment, "current-assignment"), *report_evidence],
                change="current_assignment_extra_donor_absent",
            )
        )

    # Result lifetime compares call-result storage and later uses.  It catches
    # the common ``t = VECMag(...); return t`` versus direct-return distinction.
    for callee in sorted(set(current_by_name) & set(donor_by_name)):
        for current_call, donor_call in zip(current_by_name[callee], donor_by_name[callee]):
            current_life = _result_lifetime(current, current_call)
            donor_life = _result_lifetime(donor, donor_call)
            if current_life == donor_life:
                continue
            if (
                current_life["storage"] == donor_life["storage"]
                and current_life["use_count"] == donor_life["use_count"]
                and current_life["return_use"] == donor_life["return_use"]
            ):
                continue
            if current_call.assignment and current_call.assignment.constant_only:
                closed.add("constant-only result shaping is closed")
                continue
            if (
                _registered_assignment(current, current_call.assignment)
                or (
                    current_call.assignment is not None
                    and re.search(r"\bregister\b", current_call.assignment.text)
                )
            ) or (
                _registered_assignment(donor, donor_call.assignment)
                or (
                    donor_call.assignment is not None
                    and re.search(r"\bregister\b", donor_call.assignment.text)
                )
            ):
                closed.add("register storage shaping is closed")
                continue
            if _dead_assignment(current, current_call.assignment) or _dead_assignment(donor, donor_call.assignment):
                closed.add("unused/dead-local result shaping is closed")
                continue
            hypotheses.append(
                _hypothesis(
                    kind="result_lifetime",
                    subject=f"{callee}#{current_call.ordinal}",
                    title=f"{callee} result lifetime differs between donor and current source",
                    recommendation=f"Review donor result storage {donor_life['storage']} and uses after the call before changing materialization.",
                    score=0.57 + auth_score * 0.34,
                    evidence=[
                        _source_span_for_call(donor, donor_call, "donor-result-call"),
                        _source_span_for_call(current, current_call, "current-result-call"),
                        *(
                            [_source_span_for_assignment(donor, donor_call.assignment, "donor-result-assignment")]
                            if donor_call.assignment is not None
                            else []
                        ),
                        *(
                            [_source_span_for_assignment(current, current_call.assignment, "current-result-assignment")]
                            if current_call.assignment is not None
                            else []
                        ),
                        *report_evidence,
                    ],
                    change="donor_result_lifetime_vs_current",
                )
            )

    current_events = [(item.kind, item.condition, item.depth) for item in current.events]
    donor_events = [(item.kind, item.condition, item.depth) for item in donor.events]
    if current_events != donor_events:
        differing = next(
            (index for index in range(min(len(current_events), len(donor_events))) if current_events[index] != donor_events[index]),
            min(len(current_events), len(donor_events)),
        )
        if differing < len(current.events) and differing < len(donor.events):
            current_event = current.events[differing]
            donor_event = donor.events[differing]
            if _is_constant_only(current_event.condition, donor_event.condition):
                closed.add("constant-only branch conditions are closed")
            else:
                hypotheses.append(
                    _hypothesis(
                        kind="branch_loop_shape",
                        subject=f"event#{differing + 1}",
                        title="branch/loop event sequence differs between donor and current source",
                        recommendation="Compare the donor's branch/loop nesting and condition topology against target control-flow evidence.",
                        score=0.52 + auth_score * 0.33,
                        evidence=[
                            _span(donor, donor_event.start, donor_event.end, "donor-control-event"),
                            _span(current, current_event.start, current_event.end, "current-control-event"),
                            *report_evidence,
                        ],
                        change="donor_control_shape_vs_current",
                    )
                )
        elif len(current_events) != len(donor_events):
            event = donor.events[-1] if len(donor_events) > len(current_events) else current.events[-1]
            model = donor if len(donor_events) > len(current_events) else current
            hypotheses.append(
                _hypothesis(
                    kind="branch_loop_shape",
                    subject=f"event-count:{len(donor_events)}:{len(current_events)}",
                    title="donor and current source have different branch/loop event counts",
                    recommendation="Use target CFG branch and back-edge evidence before adding or removing control flow.",
                    score=0.49 + auth_score * 0.31,
                    evidence=[_span(model, event.start, event.end, "donor-control-event" if model is donor else "current-control-event"), *report_evidence],
                    change="donor_control_count_vs_current",
                )
            )

    current_aggregate = {str(item["name"]): item for item in current.aggregates}
    donor_aggregate = {str(item["name"]): item for item in donor.aggregates}
    aggregate_shape_open = not pointer_output and explicit_pair
    if aggregate_shape_open and sorted((item["kind"], tuple(item["fields"])) for item in current_aggregate.values()) != sorted((item["kind"], tuple(item["fields"])) for item in donor_aggregate.values()):
        donor_item = next(iter(donor_aggregate.values()), None)
        current_item = next(iter(current_aggregate.values()), None)
        if donor_item or current_item:
            evidence: list[Mapping[str, Any]] = []
            if donor_item:
                evidence.append(_line_span_for_record(donor, donor_item, "donor-aggregate"))
            if current_item:
                evidence.append(_line_span_for_record(current, current_item, "current-aggregate"))
            evidence.extend(report_evidence)
            hypotheses.append(
                _hypothesis(
                    kind="aggregate_temporary",
                    subject=str((donor_item or current_item)["name"]),
                    title="aggregate temporary declaration/field topology differs",
                    recommendation="Check aggregate materialization and field-use order against target loads/stores before reshaping locals.",
                    score=0.48 + auth_score * 0.3,
                    evidence=evidence,
                    change="donor_aggregate_shape_vs_current",
                )
            )

    current_casts = [(item.get("call"), _normalize_identity(str(item.get("text", "")))) for item in current.casts]
    donor_casts = [(item.get("call"), _normalize_identity(str(item.get("text", "")))) for item in donor.casts]
    if current_casts != donor_casts and (current.casts or donor.casts):
        donor_item = donor.casts[0] if donor.casts else None
        current_item = current.casts[0] if current.casts else None
        evidence = []
        if donor_item:
            evidence.append(_line_span_for_record(donor, donor_item, "donor-prototype-cast"))
        if current_item:
            evidence.append(_line_span_for_record(current, current_item, "current-prototype-cast"))
        evidence.extend(report_evidence)
        hypotheses.append(
            _hypothesis(
                kind="prototype_cast",
                subject=str((donor_item or current_item or {}).get("call") or "cast"),
                title="prototype-dependent cast spelling differs between donor and current source",
                recommendation="Verify the declaration visible at the call site before adopting the donor cast; do not add a cast solely to shape registers.",
                score=0.43 + auth_score * 0.24,
                evidence=evidence,
                change="donor_cast_vs_current_cast",
                guard="prototype evidence required; register and constant shaping remain closed",
                requires_prototype=True,
            )
        )

    # Deterministic de-duplication and ordering.  A report may produce the same
    # source clue through call, assignment, and lifetime passes; retain each
    # distinct category but never vary order with input dictionary ordering.
    unique: dict[tuple[str, str, str, str], dict[str, Any]] = {}
    for item in hypotheses:
        evidence = item.get("evidence", [])
        donor_line = next((int(row.get("line_start", 0)) for row in evidence if row.get("side") == "donor"), 0)
        current_line = next((int(row.get("line_start", 0)) for row in evidence if row.get("side") == "current"), 0)
        key = (str(item["kind"]), str(item["subject"]), str(donor_line), str(current_line))
        unique.setdefault(key, item)
    order = {
        "call_argument_identity": 1,
        "call_argument_topology": 2,
        "direct_call_inlining": 3,
        "result_lifetime": 4,
        "missing_assignment": 5,
        "extra_assignment": 6,
        "branch_loop_shape": 7,
        "aggregate_temporary": 8,
        "prototype_cast": 9,
    }
    ranked = sorted(
        unique.values(),
        key=lambda item: (
            -float(item["confidence"]["score"]),
            order.get(str(item["kind"]), 99),
            str(item["subject"]),
            str(item.get("title", "")),
        ),
    )
    for rank, item in enumerate(ranked, 1):
        item["rank"] = rank
    return ranked, sorted(closed)


def align_donor_cfg(
    report: Mapping[str, Any] | str | Path,
    *,
    focus_symbol: str,
    current_source: str | Path,
    donor_source: str | Path,
    donor_symbol: str | None = None,
) -> dict[str, Any]:
    """Authenticate one report pair and return conservative donor hypotheses."""

    focus = focus_symbol.strip()
    if not focus or _IDENTIFIER_RE.fullmatch(focus) is None:
        _fail("focus_symbol must be one C identifier")
    report_value, report_path, report_hash = _load_report(report)
    auth = _report_authentication(report_value, report_path, report_hash, focus)
    current_text, current_label, current_data = _read_text(current_source, "current source")
    donor_text, donor_label, donor_data = _read_text(donor_source, "donor source")
    current_info = _extract_function(current_text, focus, f"current source {current_label}")
    donor_info = _extract_function(
        donor_text,
        donor_symbol or focus,
        f"donor source {donor_label}",
        # A differently named donor must match the caller's explicit symbol.
        # Never treat an unrelated sole function as source-shape evidence.
        allow_sole_donor_definition=False,
    )
    current_model = _model(current_info)
    donor_model = _model(donor_info)
    hypotheses, closed = _rank_model_differences(current_model, donor_model, auth)
    current_descriptor = _source_descriptor(current_info, current_data, current_label)
    donor_descriptor = _source_descriptor(donor_info, donor_data, donor_label)
    result = {
        "schema": SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbol": focus,
        "authentication": auth,
        "target": auth["target"],
        "candidate": auth["candidate"],
        "current": current_descriptor,
        "donor": donor_descriptor,
        "current_source": current_descriptor,
        "donor_source": donor_descriptor,
        "alignment": {
            "target_symbol": auth["target"]["symbol"],
            "candidate_symbol": auth["candidate"]["symbol"],
            "candidate_instruction_profile": auth["candidate"].get("instruction_profile", {}),
            "current_control_events": len(current_model.events),
            "donor_control_events": len(donor_model.events),
            "current_calls": len(current_model.calls),
            "donor_calls": len(donor_model.calls),
        },
        "hypotheses": hypotheses,
        "ranked_hypotheses": hypotheses,
        "closed": closed,
        "verdict": "ranked" if hypotheses else "no_safe_hypotheses",
        "evidence_class": "donor_source_shape_only",
        "target_proof": False,
        "safe_to_apply": False,
        "auto_edit": False,
    }
    return result


def analyze_alignment(*args: Any, **kwargs: Any) -> dict[str, Any]:
    """Compatibility alias for :func:`align_donor_cfg`."""

    return align_donor_cfg(*args, **kwargs)


def analyze(*args: Any, **kwargs: Any) -> dict[str, Any]:
    """Short compatibility alias for callers embedding the read-only probe."""

    return align_donor_cfg(*args, **kwargs)


def source_chronology(source: str | Path, *, symbol: str) -> dict[str, Any]:
    """Return deterministic source spans for one explicitly selected function.

    This is a read-only parser view, not an ownership claim.  The same-session
    producer may join these spans only through an independently authenticated
    Object token/name binding; absent or ambiguous bindings remain UNKNOWN.
    """

    focus = symbol.strip()
    if not focus or _IDENTIFIER_RE.fullmatch(focus) is None:
        _fail("symbol must be one C identifier")
    text, label, data = _read_text(source, "source chronology")
    info = _extract_function(text, focus, f"source chronology {label}")
    model = _model(info)

    assignments = [
        {
            "ordinal": index,
            "lhs": row.lhs,
            "operator": row.op,
            "rhs": row.rhs,
            "span": _source_span_for_assignment(model, row, "current-assignment"),
        }
        for index, row in enumerate(model.assignments, 1)
    ]
    calls = [
        {
            "evaluation_ordinal": index,
            "callee": row.callee,
            "arguments": list(row.args),
            "assigned_lhs": row.assigned_lhs,
            "span": _source_span_for_call(model, row, "current-call-return"),
        }
        for index, row in enumerate(model.calls, 1)
    ]
    control = [
        {
            "ordinal": index,
            "kind": row.kind,
            "condition": row.condition,
            "depth": row.depth,
            "span": _span(model, row.start, row.end, "current-control-event"),
        }
        for index, row in enumerate(model.events, 1)
    ]
    result = {
        "schema": "donor_cfg_source_chronology/v1",
        "function": focus,
        "source": _source_descriptor(info, data, label),
        "assignments": assignments,
        "calls": calls,
        "control_events": control,
        "authority_advanced": False,
    }
    result["chronology_sha256"] = _sha256(
        json.dumps(result, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    )
    return result


def rank_hypotheses(*args: Any, **kwargs: Any) -> list[dict[str, Any]]:
    """Return only the deterministic ranked hypothesis list."""

    return align_donor_cfg(*args, **kwargs)["hypotheses"]


def _render_human(result: Mapping[str, Any]) -> str:
    lines = [
        f"{result.get('schema', SCHEMA)} focus={result.get('focus_symbol', '')}",
        f"authentication: {result.get('authentication', {}).get('status', 'rejected')} "
        f"candidate={result.get('candidate', {}).get('symbol')} target={result.get('target', {}).get('symbol')} "
        f"match={result.get('authentication', {}).get('match_percent')}",
        f"verdict: {result.get('verdict')} (safe_to_apply={result.get('safe_to_apply')})",
    ]
    hypotheses = result.get("hypotheses")
    if isinstance(hypotheses, list) and hypotheses:
        lines.append("hypotheses:")
        for item in hypotheses:
            confidence = item.get("confidence", {})
            lines.append(
                f"  {item.get('rank')}. {item.get('kind')} [{confidence.get('label')} {confidence.get('score')}] "
                f"{item.get('title')}"
            )
            lines.append(f"     {item.get('recommendation')}")
            evidence = item.get("evidence", [])
            for span in evidence[:4] if isinstance(evidence, list) else []:
                if span.get("side") == "report":
                    lines.append(f"     evidence report:{span.get('json_path')}")
                else:
                    lines.append(
                        f"     evidence {span.get('side')}:{span.get('path')}:{span.get('line_start')}-{span.get('line_end')} "
                        f"{span.get('snippet', '').strip()}"
                    )
    else:
        lines.append("hypotheses: none")
    closed = result.get("closed")
    if isinstance(closed, list) and closed:
        lines.append("closed:")
        lines.extend(f"  - {item}" for item in closed)
    return "\n".join(lines)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="rank authenticated donor-backed C source-shape hypotheses without editing"
    )
    parser.add_argument(
        "--report",
        "--objdiff-report",
        "--objdiff",
        "--target-report",
        "--candidate-report",
        "--target-candidate-report",
        "--report-path",
        dest="report",
        required=True,
        help="target/candidate objdiff JSON report",
    )
    parser.add_argument("--focus-symbol", "--focus", dest="focus_symbol", required=True)
    parser.add_argument(
        "--current-source",
        "--current",
        "--current-function",
        dest="current_source",
        required=True,
    )
    parser.add_argument(
        "--donor-source",
        "--donor",
        "--donor-function",
        dest="donor_source",
        required=True,
    )
    parser.add_argument("--donor-symbol", default=None, help="optional donor function name")
    parser.add_argument("--json", action="store_true", help="emit deterministic JSON")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = align_donor_cfg(
            args.report,
            focus_symbol=args.focus_symbol,
            current_source=args.current_source,
            donor_source=args.donor_source,
            donor_symbol=args.donor_symbol,
        )
    except DonorCfgError as exc:
        if args.json:
            print(
                json.dumps(
                    {
                        "schema": SCHEMA,
                        "schema_version": SCHEMA_VERSION,
                        "status": "rejected",
                        "error": str(exc),
                        "hypotheses": [],
                        "evidence_class": "donor_source_shape_only",
                        "target_proof": False,
                        "safe_to_apply": False,
                        "auto_edit": False,
                    },
                    ensure_ascii=False,
                    sort_keys=True,
                    indent=2,
                )
            )
        else:
            print(f"donor_cfg_align: rejected: {exc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(result, ensure_ascii=False, sort_keys=True, indent=2))
    else:
        print(_render_human(result))
    return 0


if __name__ == "__main__":  # pragma: no cover - exercised by CLI smoke tests
    raise SystemExit(main())
