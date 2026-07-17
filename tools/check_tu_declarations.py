#!/usr/bin/env python3

"""Validate declaration ownership for matching translation units.

This gate is intentionally narrower than a style linter.  It prevents a
matching C owner from resolving reconstructed data or functions through an
unchecked ``extern`` declaration, while preserving target-proven MWCC
visibility and declaration order.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Iterable


IDENT_RE = re.compile(r"[A-Za-z_]\w*")


def _splice_lines(text: str) -> str:
    """Apply C translation phase 2 before declaration scanning."""

    return re.sub(r"\\\r?\n", "", text)


def _mask_comments_and_literals(text: str) -> str:
    """Replace comments and string/character contents while preserving lines."""

    out = list(text)
    i = 0
    state = "code"
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line_comment"
                continue
            if ch == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block_comment"
                continue
            if ch == '"':
                out[i] = " "
                i += 1
                state = "string"
                continue
            if ch == "'":
                out[i] = " "
                i += 1
                state = "char"
                continue
            i += 1
            continue
        if state == "line_comment":
            if ch == "\n":
                state = "code"
            else:
                out[i] = " "
            i += 1
            continue
        if state == "block_comment":
            if ch == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
            else:
                if ch != "\n":
                    out[i] = " "
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
        i += 1
    return "".join(out)


def _file_scope(text: str) -> str:
    cleaned = _mask_comments_and_literals(_splice_lines(text))
    out = list(cleaned)
    depth = 0
    for i, ch in enumerate(cleaned):
        if ch == "{":
            depth += 1
            out[i] = " "
        elif ch == "}":
            out[i] = " "
            depth = max(0, depth - 1)
        elif depth and ch != "\n":
            out[i] = " "
    return "".join(out)


def _mask_preprocessor(text: str) -> str:
    """Mask preprocessor directives while retaining their line structure."""

    out: list[str] = []
    continuation = False
    for line in text.splitlines(keepends=True):
        directive = continuation or line.lstrip().startswith("#")
        if directive:
            ending = "\n" if line.endswith("\n") else ""
            body = line[:-1] if ending else line
            continuation = body.rstrip().endswith("\\")
            out.append(" " * len(body) + ending)
        else:
            continuation = False
            out.append(line)
    return "".join(out)


def _function_definition_symbol(prefix: str) -> str | None:
    """Return the function name when a top-level brace starts a definition."""

    if "=" in prefix or re.search(r"\btypedef\b", prefix):
        return None
    depth = 0
    top_level_opens: list[int] = []
    for index, char in enumerate(prefix):
        if char == "(":
            if depth == 0:
                top_level_opens.append(index)
            depth += 1
        elif char == ")" and depth:
            depth -= 1
    if not top_level_opens or depth:
        return None
    names = IDENT_RE.findall(prefix[: top_level_opens[-1]])
    if not names:
        return None
    symbol = names[-1]
    if symbol in {"if", "for", "while", "switch"}:
        return None
    return symbol


def _file_scope_items(text: str) -> tuple[list[dict[str, object]], list[dict[str, object]]]:
    """Collect top-level semicolon declarations and function definitions."""

    cleaned = _mask_preprocessor(_mask_comments_and_literals(_splice_lines(text)))
    statements: list[dict[str, object]] = []
    definitions: list[dict[str, object]] = []
    buffer: list[str] = []
    line = 1
    buffer_line = 1
    brace_depth = 0
    brace_kind = ""

    for char in cleaned:
        if brace_depth:
            if char == "{":
                brace_depth += 1
            elif char == "}":
                brace_depth -= 1
                if brace_depth == 0:
                    if brace_kind == "declaration":
                        buffer.append(" ")
                    else:
                        buffer = []
                        buffer_line = line
                    brace_kind = ""
            elif char == "\n":
                if brace_kind == "declaration":
                    buffer.append("\n")
                line += 1
            continue

        if char == "{":
            prefix = "".join(buffer)
            symbol = _function_definition_symbol(prefix)
            if symbol is not None:
                definitions.append(
                    {
                        "symbol": symbol,
                        "line": buffer_line,
                        "declaration": _canonical(prefix + ";"),
                    }
                )
                brace_kind = "function"
            else:
                brace_kind = "declaration"
            brace_depth = 1
            continue
        if char == ";":
            statement = "".join(buffer) + ";"
            if statement.strip() != ";":
                statements.append({"text": statement, "line": buffer_line})
            buffer = []
            buffer_line = line
            continue
        buffer.append(char)
        if char == "\n":
            line += 1
            if not "".join(buffer).strip():
                buffer_line = line

    return statements, definitions


def _top_level_commas(declaration: str) -> int:
    depth = 0
    count = 0
    for char in declaration:
        if char in "([":
            depth += 1
        elif char in ")]" and depth:
            depth -= 1
        elif char == "," and depth == 0:
            count += 1
    return count


def _canonical(declaration: str) -> str:
    declaration = re.sub(r"\bextern\s+", "", declaration)
    declaration = re.sub(r"\s+", " ", declaration.strip())
    declaration = re.sub(r"\s*([(),;\[\]*])\s*", r"\1", declaration)
    return declaration


def _declarator_symbol(declaration: str) -> tuple[str, str]:
    body = re.sub(r"^\s*extern\b", "", declaration.strip(), count=1)
    tag = re.fullmatch(r"\s*(?:struct|union|enum)\s+([A-Za-z_]\w*)\s*;\s*", body)
    if tag:
        return tag.group(1), "tag"
    before_init = body.split("=", 1)[0]
    pointer = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", before_init)
    if pointer:
        return pointer.group(1), "object"
    function = re.search(r"\b([A-Za-z_]\w*)\s*\(", before_init)
    if function:
        return function.group(1), "function"
    before_init = re.sub(r"\[[^\]]*\]", " ", before_init)
    names = IDENT_RE.findall(before_init)
    if not names:
        raise ValueError(f"cannot identify declarator: {declaration!r}")
    return names[-1], "object"


def explicit_externs(text: str) -> list[dict[str, object]]:
    result: list[dict[str, object]] = []
    statements, _ = _file_scope_items(text)
    for item in statements:
        statement = str(item["text"])
        if re.match(r"^\s*extern\b", statement) is None:
            continue
        symbol, kind = _declarator_symbol(statement)
        result.append(
            {
                "symbol": symbol,
                "kind": kind,
                "line": item["line"],
                "declaration": _canonical(statement),
                "top_level_commas": _top_level_commas(statement),
            }
        )
    return result


def _direct_declarations(text: str) -> list[dict[str, object]]:
    result: list[dict[str, object]] = []
    statements, _ = _file_scope_items(text)
    for item in statements:
        statement = str(item["text"])
        if re.search(r"\btypedef\b", statement):
            continue
        try:
            symbol, kind = _declarator_symbol(statement)
        except ValueError:
            continue
        result.append(
            {
                "symbol": symbol,
                "kind": kind,
                "line": item["line"],
                "declaration": _canonical(statement),
                "extern": re.match(r"^\s*extern\b", statement) is not None,
                "top_level_commas": _top_level_commas(statement),
            }
        )
    return result


def _prototype_from_scope(scope: str, symbol: str) -> tuple[str, int] | None:
    pattern = re.compile(rf"\b{re.escape(symbol)}\b")
    for match in pattern.finditer(scope):
        cursor = match.end()
        while cursor < len(scope) and scope[cursor].isspace():
            cursor += 1
        if cursor >= len(scope) or scope[cursor] != "(":
            continue
        depth = 0
        while cursor < len(scope):
            if scope[cursor] == "(":
                depth += 1
            elif scope[cursor] == ")":
                depth -= 1
                if depth == 0:
                    cursor += 1
                    break
            cursor += 1
        while cursor < len(scope) and scope[cursor].isspace():
            cursor += 1
        if cursor >= len(scope) or scope[cursor] != ";":
            continue
        line_start = scope.rfind("\n", 0, match.start()) + 1
        declaration = scope[line_start : cursor + 1]
        return _canonical(declaration), scope.count("\n", 0, line_start) + 1
    return None


def prototype(text: str, symbol: str) -> tuple[str, int] | None:
    return _prototype_from_scope(_file_scope(text), symbol)


def _symbol_authority(paths: Iterable[Path]) -> dict[str, str]:
    authority: dict[str, str] = {}
    for path in paths:
        for line in path.read_text(encoding="utf-8").splitlines():
            if "=" not in line:
                continue
            name = line.split("=", 1)[0].strip()
            if not IDENT_RE.fullmatch(name):
                continue
            kind_match = re.search(r"\btype:(\w+)", line)
            authority[name] = kind_match.group(1) if kind_match else "unknown"
    return authority


def _has_include(text: str, include: str) -> bool:
    return bool(
        re.search(rf'^\s*#\s*include\s+"{re.escape(include)}"', text, re.MULTILINE)
    )


def _direct_includes(text: str) -> list[str]:
    text = _splice_lines(text)
    masked_lines = _mask_comments_and_literals(text).splitlines()
    original_lines = text.splitlines()
    result: list[str] = []
    for masked, original in zip(masked_lines, original_lines):
        if re.match(r"^\s*#\s*include\b", masked) is None:
            continue
        match = re.match(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', original)
        if match:
            result.append(match.group(1))
    return result


def _policy_declaration(entry: object) -> tuple[str, str]:
    if isinstance(entry, str):
        return entry, "either"
    if not isinstance(entry, dict):
        raise TypeError(f"invalid declaration policy entry: {entry!r}")
    return str(entry["declaration"]), str(entry["authority"])


def check_policy(policy_path: Path) -> list[str]:
    policy_path = policy_path.resolve()
    root = policy_path.parents[2]
    policy = json.loads(policy_path.read_text(encoding="utf-8"))
    errors: list[str] = []

    dol_authority = _symbol_authority(
        root / Path(path) for path in policy["dol_symbol_files"]
    )
    rel_authority = _symbol_authority(
        root / Path(path) for path in policy["rel_symbol_files"]
    )

    def authority_for(name: str) -> dict[str, str]:
        if name == "dol":
            return dol_authority
        if name == "rel":
            return rel_authority
        merged = dict(dol_authority)
        merged.update(rel_authority)
        return merged

    source_declarations: dict[str, list[dict[str, object]]] = {}
    source_function_definitions: dict[str, list[dict[str, object]]] = {}
    object_definition_counts: dict[str, int] = {}
    function_definition_counts: dict[str, int] = {}
    for source in policy["sources"]:
        rel = source["path"]
        text = (root / rel).read_text(encoding="utf-8")
        declarations = _direct_declarations(text)
        _, definitions = _file_scope_items(text)
        source_declarations[rel] = declarations
        source_function_definitions[rel] = definitions
        for definition in definitions:
            symbol = str(definition["symbol"])
            function_definition_counts[symbol] = (
                function_definition_counts.get(symbol, 0) + 1
            )
        for declaration in declarations:
            if declaration["extern"] or declaration["kind"] != "object":
                continue
            symbol = str(declaration["symbol"])
            object_definition_counts[symbol] = object_definition_counts.get(symbol, 0) + 1

        allowed = source.get("allowed_externs", {})
        seen: dict[str, int] = {}
        for declaration in explicit_externs(text):
            symbol = str(declaration["symbol"])
            seen[symbol] = seen.get(symbol, 0) + 1
            entry = allowed.get(symbol)
            location = f"{rel}:{declaration['line']}"
            if seen[symbol] > 1:
                errors.append(f"{location}: duplicate extern {symbol}")
            if declaration["top_level_commas"]:
                errors.append(f"{location}: comma-packed extern declaration is forbidden")
            if entry is None:
                errors.append(f"{location}: unowned extern {symbol}")
            else:
                expected, _ = _policy_declaration(entry)
                if _canonical(expected) == declaration["declaration"]:
                    continue
                errors.append(
                    f"{location}: {symbol} declaration differs from policy: "
                    f"{declaration['declaration']}"
                )
        missing = sorted(set(allowed) - set(seen))
        for symbol in missing:
            errors.append(f"{rel}: required authenticated extern {symbol} is missing")
        for include in source.get("required_includes", []):
            if not _has_include(text, include):
                errors.append(f'{rel}: required owner include "{include}" is missing')
        if "allowed_includes" in source:
            allowed_includes = source["allowed_includes"]
            actual_includes = _direct_includes(text)
            if set(actual_includes) != set(allowed_includes) or len(actual_includes) != len(
                allowed_includes
            ):
                missing_includes = sorted(set(allowed_includes) - set(actual_includes))
                extra_includes = sorted(set(actual_includes) - set(allowed_includes))
                errors.append(
                    f"{rel}: include set differs from visibility policy "
                    f"(missing={missing_includes}, extra={extra_includes})"
                )

    module_functions = set(policy["module_functions"])
    for source in policy["sources"]:
        rel = source["path"]
        local_definitions: dict[str, int] = {}
        for definition in source_function_definitions[rel]:
            symbol = str(definition["symbol"])
            local_definitions[symbol] = local_definitions.get(symbol, 0) + 1
        allowed_imports = source.get("allowed_import_prototypes", {})
        seen_imports: set[str] = set()
        for declaration in source_declarations[rel]:
            if declaration["kind"] != "function" or declaration["extern"]:
                continue
            symbol = str(declaration["symbol"])
            location = f"{rel}:{declaration['line']}"
            if declaration["top_level_commas"]:
                errors.append(f"{location}: comma-packed prototype is forbidden")
            if symbol in module_functions:
                errors.append(f"{location}: module function {symbol} redeclared in C")
                continue
            definition_count = local_definitions.get(symbol, 0)
            if definition_count == 1:
                continue
            entry = allowed_imports.get(symbol)
            if entry is None:
                errors.append(
                    f"{location}: unowned prototype {symbol} has "
                    f"{definition_count} same-TU definitions"
                )
                continue
            seen_imports.add(symbol)
            expected, authority_name = _policy_declaration(entry)
            if _canonical(expected) != declaration["declaration"]:
                errors.append(f"{location}: {symbol} prototype differs from policy")
            if authority_for(authority_name).get(symbol) != "function":
                errors.append(f"{rel}: imported prototype {symbol} lacks target authority")
        for symbol in sorted(set(allowed_imports) - seen_imports):
            errors.append(f"{rel}: required authenticated prototype {symbol} is missing")

    for header, allowed_includes in policy.get("header_includes", {}).items():
        header_text = (root / header).read_text(encoding="utf-8")
        actual_includes = _direct_includes(header_text)
        if set(actual_includes) != set(allowed_includes) or len(actual_includes) != len(
            allowed_includes
        ):
            missing_includes = sorted(set(allowed_includes) - set(actual_includes))
            extra_includes = sorted(set(actual_includes) - set(allowed_includes))
            errors.append(
                f"{header}: include set differs from visibility policy "
                f"(missing={missing_includes}, extra={extra_includes})"
            )

    globals_path = policy["globals_header"]
    globals_text = (root / globals_path).read_text(encoding="utf-8")
    globals_declarations = _direct_declarations(globals_text)
    if not globals_declarations:
        errors.append(f"{globals_path}: no owner declarations found")
    for declaration in globals_declarations:
        symbol = str(declaration["symbol"])
        location = f"{globals_path}:{declaration['line']}"
        if not declaration["extern"]:
            errors.append(f"{location}: {symbol} must be an extern owner declaration")
        if declaration["top_level_commas"]:
            errors.append(f"{location}: comma-packed owner declaration is forbidden")
        if declaration["kind"] != "object":
            errors.append(f"{globals_path}:{declaration['line']}: {symbol} is not an object")
            continue
        if rel_authority.get(symbol) not in {"object", "label"}:
            errors.append(
                f"{globals_path}:{declaration['line']}: {symbol} lacks target object authority"
            )
        count = object_definition_counts.get(symbol, 0)
        if count != 1:
            errors.append(
                f"{globals_path}:{declaration['line']}: {symbol} has {count} owner definitions"
            )

    for source in policy["sources"]:
        rel = source["path"]
        for symbol, entry in source.get("allowed_externs", {}).items():
            expected, authority_name = _policy_declaration(entry)
            kind = _declarator_symbol(expected)[1]
            allowed_kinds = {"function"} if kind == "function" else {"object", "label"}
            if authority_for(authority_name).get(symbol) not in allowed_kinds:
                errors.append(f"{rel}: authenticated extern {symbol} lacks target authority")

    module_header = policy["module_header"]
    module_text = (root / module_header).read_text(encoding="utf-8")
    module_declarations = _direct_declarations(module_text)
    module_prototypes = [
        declaration
        for declaration in module_declarations
        if declaration["kind"] == "function"
    ]
    module_names = [str(declaration["symbol"]) for declaration in module_prototypes]
    for declaration in module_declarations:
        if declaration["kind"] != "function":
            errors.append(
                f"{module_header}:{declaration['line']}: public header contains "
                f"non-function declaration {declaration['symbol']}"
            )
    if set(module_names) != module_functions or len(module_names) != len(module_functions):
        missing = sorted(module_functions - set(module_names))
        extra = sorted(set(module_names) - module_functions)
        errors.append(
            f"{module_header}: module prototype set differs from policy "
            f"(missing={missing}, extra={extra})"
        )
    for symbol in sorted(module_functions):
        if rel_authority.get(symbol) != "function":
            errors.append(f"{module_header}: module function {symbol} lacks REL authority")
        count = function_definition_counts.get(symbol, 0)
        if count != 1:
            errors.append(f"{module_header}: module function {symbol} has {count} definitions")

    for owner in policy.get("owned_functions", []):
        symbol = owner["symbol"]
        header = owner["header"]
        owner_text = (root / header).read_text(encoding="utf-8")
        found = prototype(owner_text, symbol)
        if found is None:
            errors.append(f"{header}: owned prototype {symbol} is missing")
        elif "declaration" in owner and found[0] != _canonical(owner["declaration"]):
            errors.append(f"{header}: owned prototype {symbol} differs from policy")
        if dol_authority.get(symbol) != "function":
            errors.append(f"{header}: owned function {symbol} lacks DOL authority")
        for rel, declarations in source_declarations.items():
            for declaration in declarations:
                if declaration["kind"] == "function" and declaration["symbol"] == symbol:
                    errors.append(
                        f"{rel}:{declaration['line']}: {symbol} must come from {header}"
                    )

    for mirror in policy.get("mirrored_prototypes", []):
        symbol = mirror["symbol"]
        source = mirror["source"]
        header = mirror["header"]
        left_matches = [
            declaration
            for declaration in source_declarations[source]
            if declaration["kind"] == "function" and declaration["symbol"] == symbol
        ]
        left = left_matches[0] if len(left_matches) == 1 else None
        right = prototype((root / header).read_text(encoding="utf-8"), symbol)
        if left is None or right is None or left["declaration"] != right[0]:
            errors.append(f"{source}: {symbol} must match authoritative {header}")

    return errors


def check_policy_or_exit(policy_path: Path) -> None:
    errors = check_policy(policy_path)
    if errors:
        for error in errors:
            print(f"declaration gate: {error}", file=sys.stderr)
        raise SystemExit(1)
    print("declaration gate: mdpartydll ownership OK")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--policy", type=Path, required=True)
    args = parser.parse_args()
    check_policy_or_exit(args.policy)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
