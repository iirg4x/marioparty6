#!/usr/bin/env python3

###
# Generates a ctx.c file, usable for "Context" on https://decomp.me.
#
# Usage:
#   python3 tools/decompctx.py src/file.cpp
#
# This is an include flattener, not a C preprocessor. Conditional compiler or
# system includes require the actual compiler's preprocessing path. For MWCC:
#   mwcceppc.exe <original defines/include/language flags> -P -EP <source> -o <context.i>
# Remove the original -c, -MMD and object-output options. For C, feed the result
# to m2c with -t ppc-mwcc-c --context context.i (ppc alone selects C++).
#
# If changes are made, please submit a PR to
# https://github.com/encounter/dtk-template
###

import argparse
import os
import re
import sys
import tempfile
import locale
from pathlib import Path
from typing import List

script_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.abspath(os.path.join(script_dir, ".."))
src_dir = os.path.join(root_dir, "src")
include_dirs = [
    os.path.join(root_dir, "include"),
    # Add additional include directories here
]

include_pattern = re.compile(r'^#\s*include\s*[<"](.+?)[>"]\s*(?://.*|/\*.*?\*/\s*)?$')
guard_pattern = re.compile(r"^#ifndef\s+(.*)$")

class ContextError(ValueError):
    pass


class ImportState:
    def __init__(self, root=None):
        self.root = Path(root or root_dir).resolve()
        self.includes = [self.root / "include"]
        self.guards = set()
        self.active = []
        self.dependencies = set()
        self.bytes_read = 0

    def chain(self, leaf):
        return " -> ".join([*(str(p) for p in self.active), str(leaf)])


def _guard(text):
    # Recognize the conventional opening pair, not arbitrary #if semantics.
    prefix = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S).lstrip()
    match = re.match(r'#ifndef[ \t]+([A-Za-z_]\w*)[ \t]*\r?\n\s*#define[ \t]+\1(?:[ \t]*\r?\n)', prefix)
    return match[1] if match else None


def import_h_file(in_file: str, r_path: str, deps: List[str], state=None) -> str:
    state = state or ImportState()
    for path in [state.root / r_path / in_file, *(p / in_file for p in state.includes)]:
        if path.is_file():
            return import_c_file(str(path), deps, state)
    raise ContextError("Missing required include: " + state.chain(in_file))


def import_c_file(in_file: str, deps: List[str], state=None, *, root=None) -> str:
    state = state or ImportState(root)
    path = (state.root / in_file).resolve()
    if len(state.active) >= 128 or len(state.dependencies) >= 4096:
        raise ContextError("Include traversal bound exceeded: " + state.chain(path))
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise ContextError("Cannot read include chain: " + state.chain(path)) from exc
    state.bytes_read += len(raw)
    if state.bytes_read > 16 * 1024 * 1024:
        raise ContextError("Context input size bound exceeded")
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        text = raw.decode(locale.getpreferredencoding(False), errors="strict")
    relative = os.path.relpath(path, state.root)
    if path not in state.dependencies:
        state.dependencies.add(path)
        deps.append(relative)
    guard = _guard(text)
    if guard and guard in state.guards:
        return ""
    if path in state.active:
        raise ContextError("Unguarded include cycle: " + state.chain(path))
    if guard:
        state.guards.add(guard)
    state.active.append(path)
    try:
        return process_file(relative, text.splitlines(keepends=True), deps, state)
    finally:
        state.active.pop()


def process_file(in_file: str, lines: List[str], deps: List[str], state=None) -> str:
    state = state or ImportState()
    out_text = ""
    for idx, line in enumerate(lines):
        if idx == 0:
            print("Processing file", in_file)
        include_match = include_pattern.match(line.strip())
        if include_match and not include_match[1].endswith(".s"):
            out_text += f'/* "{in_file}" line {idx} "{include_match[1]}" */\n'
            out_text += import_h_file(include_match[1], os.path.dirname(in_file), deps, state)
            out_text += f'/* end "{include_match[1]}" */\n'
        else:
            out_text += line

    return out_text


def sanitize_path(path: str) -> str:
    return path.replace("\\", "/").replace(" ", "\\ ")


def _atomic(path, text):
    path = Path(path)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                         prefix=path.name+".", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(text)
        os.replace(temporary, path)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Create a decomp.me context by flattening includes, not preprocessing C.",
        epilog="Conditional compiler/system includes require the actual compiler: "
               "MWCC <original defines/include/language flags> -P -EP <source> -o <context.i> "
               "(remove -c, -MMD and object-output options). For C use m2c "
               "-t ppc-mwcc-c --context context.i; ppc alone selects C++.",
    )
    parser.add_argument(
        "c_file",
        help="""File from which to create context""",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="""Output file""",
        default="ctx.c",
    )
    parser.add_argument(
        "-d",
        "--depfile",
        help="""Dependency file""",
    )
    parser.add_argument("--root", type=Path, default=Path(root_dir), help="Source/include/output root")
    args = parser.parse_args(argv)

    deps = []
    try:
        output = import_c_file(args.c_file, deps, root=args.root)
        dependency_text = sanitize_path(args.output) + ":" + "".join(f" \\\n\t{sanitize_path(dep)}" for dep in deps)
        _atomic(args.root / args.output, output)
        if args.depfile:
            _atomic(args.root / args.depfile, dependency_text)
    except (OSError, UnicodeError, ContextError) as exc:
        print(str(exc), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
