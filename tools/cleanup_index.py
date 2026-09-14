#!/usr/bin/env python3
"""Build the source cleanup index shipped with the MP6 progress viewer.

The index is deliberately a small, dependency-free lexical scanner.  It is
not a C parser and it does not claim that a candidate is a bug.  Its useful
guarantee is that the same snapshot commit produces the same list of
line-addressable syntax candidates without reading the caller's checkout.

Usage::

    python cleanup_index.py --repo C:/path/to/marioparty6 \
        --snapshot C:/path/to/snapshot.json

The snapshot is read first to obtain ``commit``.  Files are then fetched from
that commit through ``git cat-file --batch`` and only the ``cleanup`` key is
added or replaced in the JSON document.
"""

from __future__ import annotations

import argparse
import bisect
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Mapping, Optional, Sequence, Tuple


SOURCE_SUFFIXES = (".c", ".h", ".cpp", ".hpp", ".cc", ".cxx")
COMMIT_RE = re.compile(r"^[0-9a-fA-F]{40}$")


# Keep these ids stable.  The viewer reads the definitions from the generated
# document, so adding a category does not require a frontend release.
CATEGORY_DEFINITIONS: Tuple[Mapping[str, str], ...] = (
    {
        "id": "cast-offset",
        "label": "cast+offset",
        "family": "pointer-access",
        "description": "Pointer cast followed by a numeric offset.",
    },
    {
        "id": "cast-index",
        "label": "cast+index",
        "family": "pointer-access",
        "description": "Pointer cast used as the base of an indexed access.",
    },
    {
        "id": "m2c-field",
        "label": "M2C_FIELD",
        "family": "pointer-access",
        "description": "M2C_FIELD pointer-field access macro.",
    },
    {
        "id": "absolute-address",
        "label": "absolute pointer address",
        "family": "pointer-access",
        "description": "Pointer cast whose operand is an absolute hexadecimal address.",
    },
    {
        "id": "deref-cast",
        "label": "deref cast",
        "family": "pointer-access",
        "description": "Unary dereference applied directly to a pointer cast.",
    },
    {
        "id": "primitive-cast",
        "label": "primitive pointer cast",
        "family": "pointer-cast",
        "description": "Pointer cast to a builtin or fixed-width primitive type.",
    },
    {
        "id": "typed-cast",
        "label": "typed pointer cast",
        "family": "pointer-cast",
        "description": "Pointer cast to a named project or SDK type.",
    },
    {
        "id": "pragma",
        "label": "pragma",
        "family": "match-hack",
        "description": "Compiler pragma outside push/pop plumbing.",
    },
    {
        "id": "must-match",
        "label": "MUST_MATCH",
        "family": "match-hack",
        "description": "MUST_MATCH conditional containing real source code.",
    },
)
CATEGORY_ORDER = {item["id"]: index for index, item in enumerate(CATEGORY_DEFINITIONS)}


class CleanupIndexError(RuntimeError):
    """Raised when the pinned source cannot be read safely."""


@dataclass(frozen=True)
class Token:
    kind: str
    value: str
    start: int
    end: int
    line: int


@dataclass(frozen=True)
class CastCandidate:
    category: str
    start: int
    end: int
    line: int
    end_line: int
    detail: str
    offset: Optional[int] = None


PRIMITIVE_TYPES = {
    "bool",
    "char",
    "double",
    "f32",
    "f64",
    "float",
    "int",
    "long",
    "short",
    "s8",
    "s16",
    "s32",
    "s64",
    "signed",
    "size_t",
    "u8",
    "u16",
    "u32",
    "u64",
    "uchar",
    "uint",
    "undefined",
    "undefined1",
    "undefined2",
    "undefined4",
    "undefined8",
    "unsigned",
    "void",
}
TYPE_QUALIFIERS = {
    "const",
    "class",
    "enum",
    "long",
    "register",
    "signed",
    "struct",
    "typedef",
    "union",
    "unsigned",
    "volatile",
}
CAST_TYPE_DISALLOWED = {
    "!",
    "%",
    "&",
    "&&",
    "||",
    "*",  # removed below; kept here to make the allowed set explicit
    "+",
    ",",
    "-",
    ".",
    "/",
    ":",
    ":?",
    "<",
    "<=",
    "=",
    "==",
    ">",
    ">=",
    "?",
    "[",
    "]",
    "{",
    "}",
}
UNARY_TOKENS = {"&", "*", "+", "-", "!", "~"}
MULTI_TOKENS = (
    ">>>=",
    "<<=",
    ">>=",
    "->*",
    "...",
    "==",
    "!=",
    "<=",
    ">=",
    "&&",
    "||",
    "++",
    "--",
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
    "->",
    "::",
)
MEANINGFUL_REGION_TOKENS = {"{", "}"}


def _git(repo: Path, *arguments: str, input_bytes: Optional[bytes] = None) -> bytes:
    command = ["git", "-C", str(repo)] + list(arguments)
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
        raise CleanupIndexError(
            "git command failed ({}): {}".format(" ".join(command), str(detail).strip())
        ) from exc
    return result.stdout


def _validate_commit(repo: Path, commit: str) -> str:
    if not COMMIT_RE.fullmatch(commit):
        raise CleanupIndexError(
            "snapshot.commit must be a full 40-character commit id; got {!r}".format(commit)
        )
    object_type = _git(repo, "cat-file", "-t", commit).decode("ascii", "replace").strip()
    if object_type != "commit":
        raise CleanupIndexError("snapshot.commit does not name a commit: {}".format(commit))
    return commit.lower()


def _snapshot_document(snapshot_path: Path) -> Tuple[dict, str]:
    try:
        document = json.loads(snapshot_path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        raise CleanupIndexError("cannot read snapshot JSON {}: {}".format(snapshot_path, exc)) from exc
    if not isinstance(document, dict):
        raise CleanupIndexError("snapshot JSON must contain an object")
    commit = document.get("commit")
    if not isinstance(commit, str):
        raise CleanupIndexError("snapshot JSON is missing a string commit field")
    return document, commit


def _source_paths(repo: Path, commit: str) -> List[str]:
    raw = _git(repo, "ls-tree", "-r", "--name-only", commit, "--", "src")
    paths = []
    for value in raw.decode("utf-8", "replace").splitlines():
        path = value.strip()
        if path.startswith("src/") and path.lower().endswith(SOURCE_SUFFIXES):
            paths.append(path)
    return sorted(set(paths))


def _pinned_sources(repo: Path, commit: str, paths: Sequence[str]) -> Iterator[Tuple[str, str]]:
    """Yield ``(path, text)`` for a commit using one batch object request."""

    if not paths:
        return
    requests = "".join("{}:{}\n".format(commit, path) for path in paths).encode("utf-8")
    output = _git(repo, "cat-file", "--batch", input_bytes=requests)
    cursor = 0
    for path in paths:
        header_end = output.find(b"\n", cursor)
        if header_end < 0:
            raise CleanupIndexError("truncated git cat-file response for {}".format(path))
        header = output[cursor:header_end].decode("ascii", "replace").split()
        cursor = header_end + 1
        if len(header) != 3:
            raise CleanupIndexError("invalid git cat-file response for {}".format(path))
        object_id, status, size_text = header
        if status != "blob":
            raise CleanupIndexError("{} is not a source blob ({})".format(path, status))
        try:
            size = int(size_text)
        except ValueError as exc:
            raise CleanupIndexError("invalid blob size for {}".format(path)) from exc
        end = cursor + size
        if end > len(output):
            raise CleanupIndexError("truncated source blob for {}".format(path))
        blob = output[cursor:end]
        cursor = end
        if cursor >= len(output) or output[cursor:cursor + 1] != b"\n":
            raise CleanupIndexError("missing blob separator for {}".format(path))
        cursor += 1
        yield path, blob.decode("utf-8", "replace")


def _mask_literals_and_comments(source: str) -> str:
    """Blank comments and literals while retaining exact source offsets/newlines."""

    chars = list(source)
    length = len(source)

    def blank(begin: int, end: int) -> None:
        for position in range(begin, min(end, length)):
            if source[position] not in "\r\n":
                chars[position] = " "

    cursor = 0
    while cursor < length:
        current = source[cursor]
        following = source[cursor + 1] if cursor + 1 < length else ""
        if current == "/" and following == "/":
            end = cursor + 2
            while end < length and source[end] not in "\r\n":
                end += 1
            blank(cursor, end)
            cursor = end
            continue
        if current == "/" and following == "*":
            end = cursor + 2
            while end + 1 < length and not (source[end] == "*" and source[end + 1] == "/"):
                end += 1
            end = min(length, end + 2)
            blank(cursor, end)
            cursor = end
            continue
        if current in ('"', "'"):
            quote = current
            end = cursor + 1
            while end < length:
                if source[end] == "\\":
                    end += 2
                    continue
                if source[end] == quote:
                    end += 1
                    break
                end += 1
            blank(cursor, end)
            cursor = end
            continue
        cursor += 1
    return "".join(chars)


def _line_starts(source: str) -> List[int]:
    starts = [0]
    for index, value in enumerate(source):
        if value == "\n":
            starts.append(index + 1)
    return starts


def _line_number(starts: Sequence[int], position: int) -> int:
    return bisect.bisect_right(starts, max(0, position))


def _tokenize(masked: str, starts: Sequence[int]) -> List[Token]:
    tokens: List[Token] = []
    cursor = 0
    length = len(masked)
    while cursor < length:
        value = masked[cursor]
        if value.isspace():
            cursor += 1
            continue
        if value.isalpha() or value == "_":
            end = cursor + 1
            while end < length and (masked[end].isalnum() or masked[end] == "_"):
                end += 1
            tokens.append(Token("ident", masked[cursor:end], cursor, end, _line_number(starts, cursor)))
            cursor = end
            continue
        if value.isdigit() or (value == "." and cursor + 1 < length and masked[cursor + 1].isdigit()):
            end = cursor + 1
            while end < length and (masked[end].isalnum() or masked[end] in ".'_+"):
                # A plus or minus is only part of an exponent when it follows
                # e/E.  Keeping it here is harmless for normal hex offsets,
                # but stopping at binary operators avoids eating ``p+1``.
                if masked[end] in "+-" and masked[end - 1] not in "eE":
                    break
                end += 1
            tokens.append(Token("number", masked[cursor:end], cursor, end, _line_number(starts, cursor)))
            cursor = end
            continue
        operator = None
        for candidate in MULTI_TOKENS:
            if masked.startswith(candidate, cursor):
                operator = candidate
                break
        if operator is not None:
            end = cursor + len(operator)
            tokens.append(Token("punct", operator, cursor, end, _line_number(starts, cursor)))
            cursor = end
            continue
        tokens.append(Token("punct", value, cursor, cursor + 1, _line_number(starts, cursor)))
        cursor += 1
    return tokens


def _matching_tokens(tokens: Sequence[Token]) -> Dict[int, int]:
    pairs = {"(": ")", "[": "]", "{": "}"}
    closing = {value: key for key, value in pairs.items()}
    stacks: Dict[str, List[int]] = {key: [] for key in pairs}
    matching: Dict[int, int] = {}
    for index, token in enumerate(tokens):
        if token.value in stacks:
            stacks[token.value].append(index)
        elif token.value in closing:
            opening = closing[token.value]
            if stacks[opening]:
                left = stacks[opening].pop()
                matching[left] = index
                matching[index] = left
    return matching


def _identifier_type_name(value: str) -> bool:
    if value in PRIMITIVE_TYPES or value in TYPE_QUALIFIERS:
        return True
    if value.startswith("__") or value.endswith("_t"):
        return True
    # Project and SDK typedefs use upper-case or PascalCase names.  This
    # avoids treating ``(a*b)`` as a pointer cast while retaining names such
    # as ``HuVecF``, ``GXColor`` and ``OM_CAMERA``.
    return bool(value) and (value[0].isupper() or any(character.isupper() for character in value))


def _pointer_cast_type(tokens: Sequence[Token], left: int, right: int) -> Optional[Tuple[str, bool]]:
    body = list(tokens[left + 1:right])
    if not body or not any(token.value == "*" for token in body):
        return None
    if any(token.value in CAST_TYPE_DISALLOWED - {"*"} for token in body):
        return None
    if any(token.value in {"(", ")", "[", "]", "{", "}"} for token in body):
        # Function pointers and array declarators are declarations more often
        # than they are the C-style expression this index is looking for.
        return None
    # A C-style pointer cast ends its type with the pointer star (apart from
    # optional qualifiers).  Requiring that shape rejects multiplication such
    # as ``(A * B)`` without needing a full declaration parser.
    type_tail = [token for token in body if token.value not in {"const", "volatile"}]
    if not type_tail or type_tail[-1].value != "*":
        return None
    identifiers = [token.value for token in body if token.kind == "ident"]
    if not identifiers:
        return None
    tagged_type = any(value in {"class", "enum", "struct", "union"} for value in identifiers)
    if not all(_identifier_type_name(value) for value in identifiers) and not tagged_type:
        return None
    # The only non-identifier punctuation allowed inside a pointer cast type
    # is the pointer star and namespace separator.
    for token in body:
        if token.kind == "punct" and token.value not in {"*", "::"}:
            return None
    type_text = " ".join(token.value for token in body).replace(" *", "*")
    primitive = any(value in PRIMITIVE_TYPES for value in identifiers)
    return type_text, primitive


def _consume_primary(
    tokens: Sequence[Token], pairs: Mapping[int, int], index: int
) -> Tuple[int, bool]:
    """Return the inclusive end token and whether a postfix index was used."""

    if index >= len(tokens):
        return index - 1, False
    if tokens[index].value in UNARY_TOKENS:
        return _consume_primary(tokens, pairs, index + 1)
    if tokens[index].value == "(" and index in pairs:
        end = pairs[index]
        cursor = end + 1
    elif tokens[index].kind in {"ident", "number"}:
        cursor = index + 1
        # Qualified names are one primary expression for our purposes.
        while cursor + 1 < len(tokens) and tokens[cursor].value == "::":
            if tokens[cursor + 1].kind != "ident":
                break
            cursor += 2
        end = cursor - 1
    else:
        return index, False

    has_index = False
    while cursor < len(tokens):
        value = tokens[cursor].value
        if value in {"[", "("} and cursor in pairs:
            end = pairs[cursor]
            has_index = has_index or value == "["
            cursor = end + 1
            continue
        if value in {".", "->"} and cursor + 1 < len(tokens):
            end = cursor + 1
            cursor += 2
            continue
        break
    return end, has_index


def _consume_expression(
    tokens: Sequence[Token],
    pairs: Mapping[int, int],
    index: int,
    cast_left: Optional[int] = None,
) -> Tuple[int, bool, Optional[int]]:
    end, _operand_index = _consume_primary(tokens, pairs, index)
    # Indexing in the operand, as in ``(T *)array[i]``, is an ordinary cast
    # of an array element.  The cast+index form this index records is the
    # distinct ``((T *)p)[i]`` expression handled by the enclosing-group block
    # below.
    has_index = False
    offset: Optional[int] = None
    cursor = end + 1
    # In ``((T *)p)[i]`` the cast operand ends before the closing parenthesis
    # of the grouping expression.  Bubble through that one grouping so the
    # postfix index belongs to the cast site rather than being lost.
    if (
        cast_left is not None
        and cursor < len(tokens)
        and tokens[cursor].value == ")"
        and cursor in pairs
        and pairs[cursor] == cast_left - 1
    ):
        end = cursor
        cursor += 1
        while cursor < len(tokens):
            value = tokens[cursor].value
            if value in {"[", "("} and cursor in pairs:
                end = pairs[cursor]
                has_index = has_index or value == "["
                cursor = end + 1
                continue
            if value in {".", "->"} and cursor + 1 < len(tokens):
                end = cursor + 1
                cursor += 2
                continue
            break
    # Consume only numeric additions/subtractions.  Stopping at another
    # expression keeps two independent casts in ``a + (T*)b`` separate.
    while cursor + 1 < len(tokens) and tokens[cursor].value in {"+", "-"}:
        operand = tokens[cursor + 1]
        if operand.kind != "number":
            break
        parsed = _parse_integer(operand.value)
        if parsed is not None:
            offset = parsed if tokens[cursor].value == "+" else -parsed
        end = cursor + 1
        cursor = end + 1
        # Keep a following binary operator available for a sibling cast.  A
        # postfix index directly on the numeric result is uncommon in C and
        # does not add useful evidence here.
    return end, has_index, offset


def _is_unary_deref(tokens: Sequence[Token], cast_left: int) -> bool:
    if cast_left <= 0 or tokens[cast_left - 1].value != "*":
        return False
    if cast_left < 2:
        return True
    previous = tokens[cast_left - 2]
    if previous.kind == "ident":
        return previous.value in {
            "case",
            "delete",
            "return",
            "sizeof",
            "throw",
            "typeof",
        }
    if previous.kind == "number":
        return False
    return previous.value not in {
        ")",
        "]",
        "}",
        "++",
        "--",
    }


def _parse_integer(value: str) -> Optional[int]:
    cleaned = value.replace("'", "")
    try:
        if cleaned.lower().startswith("0x"):
            digits = re.match(r"0[xX][0-9a-fA-F]+", cleaned)
            if digits is None:
                return None
            return int(digits.group(0), 16)
        if re.fullmatch(r"[0-9]+", cleaned):
            return int(cleaned, 10)
    except ValueError:
        return None
    return None


def _format_offset(offset: int) -> str:
    sign = "+" if offset >= 0 else "-"
    return "{}0x{:X}".format(sign, abs(offset))


def _compact(source: str, start: int, end: int, limit: int = 220) -> str:
    text = re.sub(r"\s+", " ", source[max(0, start):max(start, end)]).strip()
    if len(text) <= limit:
        return text
    return text[: max(0, limit - 1)].rstrip() + "…"


def _cast_candidates(source: str, masked: str, starts: Sequence[int]) -> List[CastCandidate]:
    tokens = _tokenize(masked, starts)
    pairs = _matching_tokens(tokens)
    candidates: List[CastCandidate] = []
    for left, token in enumerate(tokens):
        if token.value != "(" or left not in pairs:
            continue
        right = pairs[left]
        type_info = _pointer_cast_type(tokens, left, right)
        if type_info is None:
            continue
        type_text, primitive = type_info
        operand = right + 1
        if operand >= len(tokens) or tokens[operand].value in {
            ")",
            "]",
            ",",
            ";",
            ":",
            "}",
        }:
            # A type in a declaration/prototype has no cast operand.
            continue
        end_token, has_index, offset = _consume_expression(tokens, pairs, operand, left)
        if end_token < operand:
            continue
        if offset is None:
            # The operand is often wrapped in one grouping expression, for
            # example ``(T*)((u8*)p + 0xE0)``.  The grouping is one primary to
            # the lightweight consumer above, but its numeric addition still
            # belongs to this outer cast access.
            offset = _find_numeric_offset(tokens, operand, end_token)
        deref = _is_unary_deref(tokens, left)
        range_start_token = left
        if deref:
            range_start_token = left - 1
        elif (
            left > 0
            and tokens[left - 1].value == "("
            and left - 1 in pairs
            and pairs[left - 1] <= end_token
        ):
            # Include the opening half of ``((T *)items)[i]`` in the source
            # snippet when the enclosing group was consumed above.
            range_start_token = left - 1
        start = tokens[range_start_token].start
        end = tokens[end_token].end
        if offset is not None:
            category = "cast-offset"
            detail = "pointer cast with {} offset".format(_format_offset(offset))
        elif has_index:
            category = "cast-index"
            detail = "pointer cast used as an indexed access"
        else:
            absolute = _first_operand_integer(tokens, operand, end_token, pairs)
            if absolute is not None and tokens[operand].value not in UNARY_TOKENS:
                category = "absolute-address"
                detail = "pointer cast to absolute address 0x{:X}".format(absolute)
            elif deref:
                category = "deref-cast"
                detail = "dereference of {} pointer cast".format(type_text)
            elif primitive:
                category = "primitive-cast"
                detail = "cast to {}".format(type_text)
            else:
                category = "typed-cast"
                detail = "cast to {}".format(type_text)
        candidates.append(
            CastCandidate(
                category=category,
                start=start,
                end=end,
                line=_line_number(starts, start),
                end_line=_line_number(starts, max(start, end - 1)),
                detail=detail,
                offset=offset if category == "cast-offset" else None,
            )
        )

    # A nested cast is part of the outer expression's evidence.  Emitting it
    # a second time makes an expression such as ``*(f32*)((u8*)p + 0xE0)``
    # look like three independent cleanup sites.
    candidates.sort(key=lambda item: (item.start, -item.end, CATEGORY_ORDER[item.category]))
    kept: List[CastCandidate] = []
    seen = set()
    for candidate in candidates:
        key = (candidate.start, candidate.end, candidate.category)
        if key in seen:
            continue
        seen.add(key)
        if any(parent.start <= candidate.start and candidate.end <= parent.end for parent in kept):
            continue
        kept.append(candidate)
    return kept


def _first_operand_integer(
    tokens: Sequence[Token], start: int, end: int, pairs: Mapping[int, int]
) -> Optional[int]:
    cursor = start
    upper = end
    # Unwrap only grouping parentheses that contain the complete operand.  A
    # loose scan would mistake ``(T*) (address + index)`` for an absolute
    # address merely because its first token is numeric.
    while cursor <= upper and tokens[cursor].value == "(" and cursor in pairs:
        close = pairs[cursor]
        if close != upper:
            return None
        cursor += 1
        upper = close - 1
    if cursor <= upper and tokens[cursor].kind == "number":
        value = tokens[cursor].value.replace("'", "")
        if not value.lower().startswith("0x"):
            return None
        if all(token.value == ")" for token in tokens[cursor + 1: end + 1]):
            return _parse_integer(value)
    return None


def _find_numeric_offset(tokens: Sequence[Token], start: int, end: int) -> Optional[int]:
    """Find arithmetic outside function-call arguments and array indexes."""

    frames: List[bool] = []
    for index in range(max(0, start), min(len(tokens), end + 1)):
        value = tokens[index].value
        if value in {"(", "[", "{"}:
            previous = tokens[index - 1].value if index > 0 else ""
            # The first grouping is the cast operand itself.  Parentheses
            # after an identifier/closing postfix are calls, and all math in
            # their arguments is deliberately ignored.
            is_call = value == "(" and index != start and previous in {
                ")",
                "]",
            }
            if value == "(" and index != start and tokens[index - 1].kind == "ident":
                is_call = True
            frames.append(is_call or value in {"[", "{"} or any(frames))
            continue
        if value in {")",
            "]",
            "}",
        }:
            if frames:
                frames.pop()
            continue
        if value not in {"+", "-"} or any(frames) or index + 1 > end:
            continue
        operand = tokens[index + 1]
        if operand.kind != "number":
            continue
        parsed = _parse_integer(operand.value)
        if parsed is not None:
            return parsed if value == "+" else -parsed
    return None


def _directive_lines(masked: str) -> Iterator[Tuple[int, int, str, str]]:
    """Yield logical preprocessor directives as (first,last,name,body)."""

    lines = masked.splitlines(keepends=True)
    index = 0
    while index < len(lines):
        first = index
        logical = lines[index]
        while logical.rstrip("\r\n").endswith("\\") and index + 1 < len(lines):
            index += 1
            logical += lines[index]
        match = re.match(r"^[ \t]*#[ \t]*([A-Za-z_][A-Za-z0-9_]*)(.*)$", logical, re.DOTALL)
        if match:
            yield first, index, match.group(1).lower(), match.group(2).strip()
        index += 1


def _pragma_candidates(source: str, masked: str, starts: Sequence[int]) -> List[CastCandidate]:
    candidates: List[CastCandidate] = []
    for first, last, name, body in _directive_lines(masked):
        if name != "pragma":
            continue
        lower = body.lower()
        if re.search(r"\b(?:push|pop)(?:_macro)?\b", lower):
            continue
        start = starts[first]
        end = starts[last + 1] if last + 1 < len(starts) else len(source)
        candidates.append(
            CastCandidate(
                category="pragma",
                start=start,
                end=end,
                line=first + 1,
                end_line=last + 1,
                detail="pragma {}".format(re.sub(r"\s+", " ", body).strip()),
            )
        )
    return candidates


def _must_match_candidates(source: str, masked: str, starts: Sequence[int]) -> List[CastCandidate]:
    lines = masked.splitlines(keepends=True)
    directives = {first: (last, name, body) for first, last, name, body in _directive_lines(masked)}
    stack: List[Dict[str, object]] = []
    regions: List[Tuple[int, int, str]] = []
    for index in range(len(lines)):
        directive = directives.get(index)
        if directive is not None:
            last, name, body = directive
            if name in {"if", "ifdef", "ifndef"}:
                condition = body
                must = bool(
                    re.search(r"\bMUST_MATCH\b", condition)
                    and not (name == "ifndef")
                )
                stack.append({"start": index, "must": must, "end": last})
            elif name == "endif" and stack:
                region = stack.pop()
                if bool(region["must"]):
                    regions.append((int(region["start"]), index, body))
            index = last

    candidates: List[CastCandidate] = []
    for first, last, _ in regions:
        body_start = starts[first + 1] if first + 1 < len(starts) else len(source)
        body_end = starts[last] if last < len(starts) else len(source)
        body = masked[body_start:body_end]
        code_lines: List[str] = []
        for line_index, value in enumerate(body.splitlines(keepends=True), start=first + 1):
            if line_index in directives:
                continue
            code_lines.append(value)
        code_tokens = _tokenize("".join(code_lines), [0])
        # A pragma-only region is represented by its individual pragma sites.
        # Braces alone are also not useful evidence of a matching body.
        meaningful = [token for token in code_tokens if token.value not in MEANINGFUL_REGION_TOKENS]
        if not meaningful:
            continue
        start = starts[first]
        end = starts[last + 1] if last + 1 < len(starts) else len(source)
        candidates.append(
            CastCandidate(
                category="must-match",
                start=start,
                end=end,
                line=first + 1,
                end_line=last + 1,
                detail="MUST_MATCH conditional region",
            )
        )
    return candidates


def _m2c_candidates(source: str, masked: str, starts: Sequence[int]) -> List[CastCandidate]:
    tokens = _tokenize(masked, starts)
    pairs = _matching_tokens(tokens)
    candidates: List[CastCandidate] = []
    for index, token in enumerate(tokens):
        if token.kind != "ident" or token.value != "M2C_FIELD":
            continue
        if index + 1 >= len(tokens) or tokens[index + 1].value != "(" or index + 1 not in pairs:
            continue
        end = tokens[pairs[index + 1]].end
        candidates.append(
            CastCandidate(
                category="m2c-field",
                start=token.start,
                end=end,
                line=token.line,
                end_line=_line_number(starts, max(token.start, end - 1)),
                detail="M2C_FIELD pointer-field access",
            )
        )
    return candidates


def scan_source(path: str, source: str) -> List[dict]:
    """Scan one source file and return JSON-compatible site records.

    This public helper is intentionally independent of Git so fixtures can
    test lexical behavior without making a temporary checkout.
    """

    masked = _mask_literals_and_comments(source)
    starts = _line_starts(source)
    candidates = _cast_candidates(source, masked, starts)
    candidates.extend(_m2c_candidates(source, masked, starts))
    candidates.extend(_pragma_candidates(source, masked, starts))
    candidates.extend(_must_match_candidates(source, masked, starts))
    candidates.sort(key=lambda item: (item.start, item.end, CATEGORY_ORDER[item.category]))
    sites: List[dict] = []
    seen = set()
    for candidate in candidates:
        key = (candidate.start, candidate.end, candidate.category)
        if key in seen:
            continue
        seen.add(key)
        site = {
            "path": path,
            "line": candidate.line,
            "endLine": candidate.end_line,
            "category": candidate.category,
            "detail": candidate.detail,
            "source": _compact(source, candidate.start, candidate.end),
        }
        if candidate.category == "must-match":
            # A conditional may contain a whole function.  Keep the index
            # useful as a source navigator without embedding its body.
            first_line_end = source.find("\n", candidate.start)
            if first_line_end < 0:
                first_line_end = len(source)
            site["source"] = _compact(source, candidate.start, first_line_end) + " … #endif"
            site["source"] = site["source"][:220]
        if candidate.offset is not None:
            site["offset"] = candidate.offset
        sites.append(site)
    return sites


def build_cleanup(repo: Path, commit: str, generated_at: Optional[str] = None) -> dict:
    """Build the cleanup payload for an already validated commit id."""

    commit = _validate_commit(repo, commit)
    paths = _source_paths(repo, commit)
    sites: List[dict] = []
    for path, source in _pinned_sources(repo, commit, paths):
        sites.extend(scan_source(path, source))
    sites.sort(
        key=lambda item: (
            item["path"],
            item["line"],
            item["endLine"],
            CATEGORY_ORDER[item["category"]],
            item["source"],
        )
    )
    methodology = [
        "Candidates are syntax-level cleanup signals, not confirmed mismatches or bugs.",
        "The scan covers committed C/C++ sources and headers under src/, including game code, SDK, and support libraries. Assembly files and headers outside src/ are excluded.",
        "Comments, string literals, and character literals are blanked before matching; line numbers refer to the pinned source.",
        "Pointer casts are counted at the outermost expression, so nested casts in one access are not emitted again.",
        "Function declarations and prototypes are skipped when a pointer type has no expression operand.",
        "The scanner is lexical and intentionally non-exhaustive; unsupported type spellings, function-pointer casts, and array declarators are omitted.",
        "Numeric additions and subtractions attached to a pointer-cast expression are indexed as cast+offset; pointer-to-integer and raw-offset heuristics are intentionally omitted.",
        "Pragmas count once except push/pop plumbing. A MUST_MATCH conditional counts once only when it contains non-directive code; pragma-only regions stay under pragma.",
        "Source is read from snapshot.commit through git object storage; the current checkout and binary files are never scanned.",
    ]
    return {
        "schemaVersion": 1,
        "commit": commit,
        "generatedAt": generated_at or datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "scannedFiles": len(paths),
        "categories": [dict(item) for item in CATEGORY_DEFINITIONS],
        "sites": sites,
        "methodology": methodology,
    }


def generate_cleanup(repo: Path, snapshot_path: Path) -> dict:
    """Read the snapshot commit and build its cleanup payload."""

    document, commit = _snapshot_document(snapshot_path)
    return build_cleanup(repo, commit)


def _write_snapshot(snapshot_path: Path, cleanup: dict) -> None:
    """Replace only the cleanup value while retaining compact JSON style."""

    try:
        raw = snapshot_path.read_text(encoding="utf-8")
        document = json.loads(raw)
    except (OSError, ValueError) as exc:
        raise CleanupIndexError("cannot update snapshot JSON {}: {}".format(snapshot_path, exc)) from exc
    if not isinstance(document, dict):
        raise CleanupIndexError("snapshot JSON must contain an object")
    document["cleanup"] = cleanup
    compact = "\n" not in raw and "\r" not in raw
    ensure_ascii = "\\u" in raw
    rendered = json.dumps(
        document,
        ensure_ascii=ensure_ascii,
        separators=(",", ":") if compact else None,
        indent=None if compact else 2,
    )
    if raw.endswith("\r\n"):
        rendered += "\r\n"
    elif raw.endswith("\n"):
        rendered += "\n"
    temporary = snapshot_path.with_name(".{}.{:08x}.tmp".format(snapshot_path.name, os.getpid()))
    try:
        with temporary.open("w", encoding="utf-8", newline="") as handle:
            handle.write(rendered)
        os.replace(str(temporary), str(snapshot_path))
    except OSError as exc:
        try:
            temporary.unlink()
        except OSError:
            pass
        raise CleanupIndexError("cannot update snapshot JSON {}: {}".format(snapshot_path, exc)) from exc


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path, help="git repository containing the pinned source")
    parser.add_argument("--snapshot", required=True, type=Path, help="snapshot JSON whose commit should be scanned")
    arguments = parser.parse_args(argv)
    try:
        cleanup = generate_cleanup(arguments.repo, arguments.snapshot)
        _write_snapshot(arguments.snapshot, cleanup)
    except CleanupIndexError as exc:
        parser.error(str(exc))
        return 2
    counts: Dict[str, int] = {}
    for site in cleanup["sites"]:
        counts[site["category"]] = counts.get(site["category"], 0) + 1
    print(
        json.dumps(
            {
                "commit": cleanup["commit"],
                "scannedFiles": cleanup["scannedFiles"],
                "sites": len(cleanup["sites"]),
                "categories": counts,
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
