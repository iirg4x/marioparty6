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

Since 27 October 2026 the index shows all unfaithful code (user: "Clean up
index in both local and main should show all unfaithful code"). Besides the
lexical pointer and pragma scan below, every game-code file goes through the
recovery's faithfulness audit (``fidelity_audit.py`` next to this file, the
same rules the recovery's own checks gate on), and the whole tree goes through its
cross-file rules: declarations that disagree, helpers written in several files,
file names made from addresses. No exemption is applied.
"""

from __future__ import annotations

import argparse
import bisect
import importlib.util
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, Iterator, List, Mapping, Optional, Sequence, Tuple


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
# Families group categories in the viewer's filter. The viewer reads them from the document.
FAMILY_DEFINITIONS: Tuple[Mapping[str, str], ...] = (
    {"id": "library-rebuilt", "label": "Library code rebuilt",
     "description": "Library or header code written again by hand instead of used: MSL C++ containers, inline helpers."},
    {"id": "declarations", "label": "Declarations",
     "description": "Declarations that disagree between files or repeat a header's."},
    {"id": "statements", "label": "Placeholder statements",
     "description": "Statements that only steer the compiler, and assertions with invented conditions."},
    {"id": "match-hack", "label": "Match workarounds",
     "description": "Pragmas, inline asm, compiler guards, intrinsics and prototype casts."},
    {"id": "invented-structure", "label": "Invented structure",
     "description": "Linkage and structure the original did not have."},
    {"id": "invented-data", "label": "Invented data", "description": "Padding, raw words and strings kept as labels."},
    {"id": "raw-numbers", "label": "Raw numbers", "description": "Numbers the original wrote as names."},
    {"id": "placeholders", "label": "Decompiler and address names",
     "description": "Names from the decompiler or made from addresses: locals, fields, labels, types, files."},
    {"id": "comments", "label": "Process comments",
     "description": "Comments about the recovery process or left by the decompiler."},
    {"id": "pointer-access", "label": "Pointer access", "description": "Fields reached through casts and offsets."},
    {"id": "pointer-cast", "label": "Pointer casts", "description": "Pointer casts."},
)

# The audit's rules, one category each: (audit category, rule) -> (id, label, family, description). A rule of None
# covers every rule of that audit category.
AUDIT_CATEGORIES: Mapping[Tuple[str, Optional[str]], Tuple[str, str, str, str]] = {
    ("library-lookalike", "MSL error exit through a data label"): (
        "msl-error-label", "MSL error exit by label", "library-rebuilt",
        "A container error message passed to fprintf(stderr) by a data label: MSL C++ code rebuilt by hand."),
    ("library-lookalike", "MSL compressed pair rebuilt"): (
        "msl-compressed-pair", "MSL compressed pair", "library-rebuilt",
        "first()/second() accessors of MSL's __compressed_pair, rebuilt instead of using std:: containers."),
    ("library-lookalike", "MSL tree node rebuilt"): (
        "msl-tree-node", "MSL tree node", "library-rebuilt",
        "A red-black tree node with its colour packed into the parent pointer: MSL's tree rebuilt."),
    ("library-lookalike", "MSL allocator rebuilt"): (
        "msl-allocator", "MSL allocator", "library-rebuilt", "A hand-written allocate/deallocate pair: std::allocator rebuilt."),
    ("duplicates", "helper defined in several files"): (
        "duplicate-helper", "helper in several files", "library-rebuilt",
        "An inline helper with the same body in more than one file: header or library code written again per file."),
    ("linkage", "C linkage on C++ code"): (
        "cpp-extern-c", "extern \"C\" in C++", "invented-structure",
        "An extern \"C\" definition in a C++ file, keeping an address name for a C++ function or class member."),
    ("declarations", "local prototype of a header function"): (
        "local-prototype", "local prototype", "declarations",
        "A function declared again in a source file although a header declares it; the original included the header."),
    ("declarations", "function declared with different types"): (
        "conflicting-prototype", "conflicting prototype", "declarations",
        "One function declared with different return or parameter types in different files."),
    ("declarations", "declared without parameters"): (
        "unprototyped", "f() declaration", "declarations",
        "A C declaration with an empty parameter list, leaving the arguments unchecked, where others give parameters."),
    ("declarations", "global declared with different types"): (
        "conflicting-global", "conflicting global", "declarations", "One global declared with different types in different files."),
    ("declarations", "own data declared extern"): (
        "own-extern", "own data extern", "declarations", "A file declares extern data it has already defined (a repeat)."),
    ("statements", "placeholder statement"): (
        "placeholder-statement", "placeholder statement", "statements",
        "A statement that only reads, (void)x;, x = x; or an empty if: there to steer the compiler."),
    ("statements", "assertion with an invented condition"): (
        "invented-assert", "invented assertion", "statements",
        "An SDK assertion whose condition never reaches retail, so it was invented."),
    ("no-op-code", None): (
        "no-op", "no-op code", "statements",
        "Arithmetic that changes nothing (+0, *1, |0, <<0, !!), empty statements, do-while-zero, double casts, x = x."),
    ("steering", "inline asm"): ("inline-asm", "inline asm", "match-hack", "Inline assembly in game code."),
    ("steering", "placement attribute"): (
        "placement-attribute", "__declspec/__attribute__", "match-hack", "__declspec or __attribute__ in game code."),
    ("steering", "prototype cast at a call"): (
        "prototype-cast", "prototype cast", "match-hack",
        "A function cast to another prototype where it is called; its header prototype is what is wrong."),
    ("visibility", "MWCC-only guard"): (
        "mwcc-guard", "__MWERKS__ guard", "match-hack", "Declarations visible only to MWCC through #if __MWERKS__."),
    ("intrinsics", "intrinsic"): ("intrinsic", "intrinsic", "match-hack", "A compiler intrinsic in game code."),
    ("invented-data", "pad/filler storage"): (
        "pad-storage", "pad storage", "invented-data", "Storage named pad, filler, dummy, unused or reserved."),
    ("invented-data", "raw hex initializer"): (
        "raw-hex-initializer", "raw hex initializer", "invented-data",
        "An initializer of eight or more long hex words: data not given its types."),
    ("invented-data", "string kept as a data label"): (
        "string-label", "string as data label", "invented-data",
        "A string literal kept as a named data object and passed by that name."),
    ("raw-numbers", "raw hex number"): (
        "raw-number", "raw number", "raw-numbers", "A data number, flag id or bit pattern the original wrote as a name."),
    ("raw-numbers", "raw decimal number"): (
        "raw-number", "raw number", "raw-numbers", "A data number, flag id or bit pattern the original wrote as a name."),
    ("raw-numbers", "unnamed audio resource ID"): (
        "audio-id", "sound id as a number", "raw-numbers",
        "A sound, music or stream id passed as a number. An id msm_se.h or msm_stream.h names must use the name; an id "
        "neither names stays a number until it is named (the user's decision), and is listed so it is found then."),
    ("raw-numbers", "string as character codes"): (
        "char-codes", "string as char codes", "raw-numbers", "A string written as character codes."),
    ("placeholders", "decompiler name"): (
        "decompiler-name", "decompiler name", "placeholders", "temp_r31, var_r30, sp10, uVar1 and other decompiler names."),
    ("placeholders", "offset-named field"): (
        "offset-field", "offset-named field", "placeholders", "A struct field named by its offset (unk1C, field5C)."),
    ("placeholders", "numbered generic name"): (
        "numbered-name", "numbered generic name", "placeholders", "A decompiler name renamed to a word and a number."),
    ("placeholders", "decompiler goto label"): (
        "decompiler-label", "decompiler label", "placeholders", "A jump target named by the decompiler or by an address."),
    ("placeholders", "address-named goto label"): (
        "decompiler-label", "decompiler label", "placeholders", "A jump target named by the decompiler or by an address."),
    ("placeholders", "address-named type"): (
        "address-type", "address-named type", "placeholders",
        "A type named by an address (InsertTree2C080): one original type rebuilt once per place."),
    ("file-names", "address-named source file"): (
        "address-file", "address-named file", "placeholders", "A source file named by an address (text_00118.c)."),
    ("probe-comments", None): (
        "process-comment", "process comment", "comments", "A comment about probes, permuters, matching or registers."),
    ("comments", "process note"): (
        "process-comment", "process comment", "comments", "A comment about probes, permuters, matching or registers."),
    ("comments", "decompiler comment"): (
        "decompiler-comment", "decompiler comment", "comments", "Ghidra's address and WARNING comments, bare offset tags."),
    ("comments", "comment without its function"): (
        "orphan-comment", "orphan comment", "comments", "A comment left behind without the function it described."),
    # from the hands-on w11Dll review (27 Oct)
    ("placeholders", "sound id named outside msm_se.h"): (
        "local-sound-name", "sound name made up locally", "invented-data",
        "A sound, stream or music id given a made-up name in a source file; write the number until msm_se.h names it."),
    ("raw-numbers", "float in decompiler spelling"): (
        "float-spelling", "decompiler float spelling", "raw-numbers",
        "A float written with more digits than it holds (-0.699999988f for -0.7f), as a decompiler prints it."),
    ("invented-structure", "include after code"): (
        "late-include", "include after code", "invented-structure",
        "An #include after the first function: fragments joined into one file kept their own includes."),
    ("declarations", "prototype of another file's function"): (
        "missing-header-prototype", "prototype without a header", "declarations",
        "Another file's function declared in a source file because its own header does not declare it."),
}
# Covered by the lexical scan above (pointer access, pragmas), or documentation and layout rather than code.
AUDIT_SKIPPED = {("steering", "pragma"), ("offset-access", None), ("comments", "no file description"),
                 ("comments", "uncommented function"), ("layout", None), ("address-names", None)}
GAME_CODE = ("src/game/", "src/board/", "src/REL/", "include/game/", "include/REL/", "include/board/")
# An address name whose original name an original artifact shows (27 Oct: Nintendo's link map on the Mario Kart:
# Double Dash!! debug disc names MSL, TRK and SDK data MP6 still calls lbl_). The names are in
# cleanup_known_names.json next to this file; each one is listed until the code takes it.
KNOWN_NAME = ("known-original-name", "original name known", "placeholders",
              "An address name (fn_, lbl_) whose original name an original artifact shows: a link map, a $Id line, "
              "a retail string. It must take that name.")
KNOWN_FILE = ("known-original-file", "original file name known", "placeholders",
              "A module's retail assert texts name the source file they were compiled from (__FILE__), and no file of "
              "the module, its framework or the headers has that name: the code is in a file named otherwise.")
COUNTER_DRIFT = ("compiler-counter", "compiler counter differs", "invented-structure",
                 "MWCC numbers a function's statics (name$N) from one counter that the source and headers before it "
                 "advance. Nintendo's link map shows another number: the file or its headers are shaped differently "
                 "from Nintendo's, though the bytes match.")
SDK_DEBUG = ("sdk-debug-name", "name differs from Nintendo's debug info", "placeholders",
             "An SDK or MSL parameter, local or struct member named otherwise than in Nintendo's own DWARF: the debug "
             "SDK libraries on the Interactive Multi-Game Demo Disk (July 2004) were built with debug info.")
KNOWN_NAMES_FILE = "cleanup_known_names.json"
SDK_DEBUG_FILE = "sdk_debug_names.json"
SDK_FOLDERS = ("src/dolphin/", "src/MSL_C.PPCEABI.bare.H/", "src/Runtime.PPCEABI.H/", "src/amcstubs/", "src/OdemuExi2/",
               "src/odenotstub/", "include/dolphin/")
SYMBOL_FILES = re.compile(r"config/(?:GP6E01|dll/rels/[^/]+)/symbols\.txt")

for _id, _label, _family, _description in list(AUDIT_CATEGORIES.values()) + [KNOWN_NAME, KNOWN_FILE, COUNTER_DRIFT,
                                                                             SDK_DEBUG]:
    if all(item["id"] != _id for item in CATEGORY_DEFINITIONS):
        CATEGORY_DEFINITIONS += ({"id": _id, "label": _label, "family": _family, "description": _description},)
CATEGORY_ORDER = {item["id"]: index for index, item in enumerate(CATEGORY_DEFINITIONS)}


def _load_audit() -> Any:
    """fidelity_audit.py next to this file (the site and the dashboard vendor it), else the recovery's own copy."""
    here = Path(__file__).resolve()
    candidates = [here.with_name("fidelity_audit.py")]
    candidates += [parent / "shape-v1" / "tools" / "fidelity_audit.py" for parent in here.parents]
    path = next((candidate for candidate in candidates if candidate.is_file()), None)
    if path is None:
        raise ImportError("fidelity_audit.py is missing next to cleanup_index.py")
    spec = importlib.util.spec_from_file_location("mp6_fidelity_audit", path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


_AUDIT = _load_audit()


def _audit_category(finding: Mapping[str, Any]) -> Optional[Tuple[str, str, str, str]]:
    category, rule = finding["category"], finding["rule"]
    if (category, rule) in AUDIT_SKIPPED or (category, None) in AUDIT_SKIPPED:
        return None
    known = AUDIT_CATEGORIES.get((category, rule)) or AUDIT_CATEGORIES.get((category, None))
    if known:
        return known
    slug = re.sub(r"[^a-z0-9]+", "-", rule.lower()).strip("-")
    return (slug, rule, category, "{} ({}).".format(rule, category))


def _audit_sites(findings: Iterable[Mapping[str, Any]], sources: Mapping[str, str]) -> List[dict]:
    """Site records of audit findings; the source excerpt is the finding's line."""
    sites = []
    lines_of: Dict[str, List[str]] = {}
    for finding in findings:
        mapped = _audit_category(finding)
        if mapped is None:
            continue
        path = finding["file"]
        lines = lines_of.setdefault(path, sources.get(path, "").split("\n"))
        line = max(1, int(finding["line"] or 1))
        excerpt = lines[line - 1].strip() if line - 1 < len(lines) else ""
        sites.append({"path": path, "line": line, "endLine": line, "category": mapped[0],
                      "detail": str(finding["text"])[:300], "source": " ".join(excerpt.split())[:220]})
    return sites


def is_game_code(path: str) -> bool:
    return path.startswith(GAME_CODE)


def scan_repository(sources: Mapping[str, str]) -> List[dict]:
    """The audit's cross-file rules over every file at once: {path: text} -> site records."""
    return _audit_sites(_AUDIT.repository_findings(dict(sources)), sources)


def _load_known(section: str = "names") -> List[Mapping[str, Any]]:
    """One section of cleanup_known_names.json: "names" [{"address_name", "name", "where", "evidence"}],
    "files" [{"module", "file", "evidence"}] or "counters" [{"source", "static", "ours", "original", "evidence"}]."""
    path = Path(__file__).resolve().with_name(KNOWN_NAMES_FILE)
    if not path.is_file():
        return []
    return json.loads(path.read_text(encoding="utf-8")).get(section, [])


def _load_known_names() -> List[Mapping[str, Any]]:
    return _load_known("names")


def scan_known_files(texts: Mapping[str, str]) -> List[dict]:
    """A module whose retail asserts name a source file that neither the module, the framework nor any header has."""
    modules: Dict[str, List[str]] = {}
    present = set()
    for path in texts:
        name = path.rsplit("/", 1)[-1].lower()
        parts = path.split("/")
        if path.startswith("src/REL/") and len(parts) > 3:
            modules.setdefault(parts[2].lower(), []).append(path)
            present.add((parts[2].lower(), name))
        if path.startswith(("src/REL/framework/", "include/")):
            present.add(("*", name))
    sites = []
    for entry in _load_known("files"):
        module, name = entry["module"].lower(), entry["file"].lower()
        files = sorted(modules.get(module, []))
        if not files or (module, name) in present or ("*", name) in present:
            continue
        sites.append({"path": files[0], "line": 1, "endLine": 1, "category": KNOWN_FILE[0],
                      "detail": "{}: {}; no file of the module has that name".format(entry["module"], entry["evidence"]),
                      "source": entry["file"]})
    return sites


def scan_counter_drift(texts: Mapping[str, str]) -> List[dict]:
    """The declaration of each function static whose compiler number differs from Nintendo's link map."""
    sites = []
    for entry in _load_known("counters"):
        source = texts.get(entry["source"])
        if source is None:
            continue
        lines = source.split("\n")
        declaration = re.compile(r"\bstatic\b[^;]*\b" + re.escape(entry["static"]) + r"\b")
        number = next((n for n, line in enumerate(lines, 1) if declaration.search(line)), 1)
        sites.append({"path": entry["source"], "line": number, "endLine": number, "category": COUNTER_DRIFT[0],
                      "detail": "{0}${1} here, {0}${2} in Nintendo's build ({3})".format(
                          entry["static"], entry["ours"], entry["original"], entry["evidence"]),
                      "source": " ".join(lines[number - 1].split())[:220] if number - 1 < len(lines) else ""})
    return sites


FUNCTION_DEFINITION = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\(([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*)\)"
                                 r"\s*\{", re.MULTILINE)
LOCAL_DECLARATION = re.compile(r"^\s*(?:(?:const|volatile|static|register|unsigned|signed|struct|union|enum)\s+)*"
                               r"([A-Za-z_]\w*)(?:\s+|\s*\*+\s*)([A-Za-z_]\w*(?:\s*\[[^\]]*\])*(?:\s*=[^,;]*)?"
                               r"(?:\s*,\s*\**\s*[A-Za-z_]\w*(?:\s*\[[^\]]*\])*(?:\s*=[^,;]*)?)*)\s*;", re.MULTILINE)
STRUCT_DEFINITION = re.compile(r"\b(?:typedef\s+)?struct\s+([A-Za-z_]\w*)?\s*\{")
NOT_A_TYPE = {"return", "goto", "case", "else", "do", "while", "if", "for", "switch", "break", "continue", "sizeof"}


def _parameter_names(text: str) -> List[str]:
    names = []
    for part in text.split(","):
        part = re.sub(r"\[[^\]]*\]", "", part).strip()
        if part in ("", "void", "..."):
            continue
        found = re.search(r"\(\s*\*\s*([A-Za-z_]\w*)\s*\)", part) or re.search(r"([A-Za-z_]\w*)\s*$", part)
        names.append(found.group(1) if found else "?")
    return names


def _block_end(code: str, brace: int) -> int:
    depth = 0
    for at in range(brace, len(code)):
        depth += {"{": 1, "}": -1}.get(code[at], 0)
        if depth == 0:
            return at
    return len(code)


def scan_sdk_debug_names(texts: Mapping[str, str]) -> List[dict]:
    """SDK and MSL parameters, locals and struct members named otherwise than in Nintendo's DWARF."""
    path = Path(__file__).resolve().with_name(SDK_DEBUG_FILE)
    if not path.is_file():
        return []
    table = json.loads(path.read_text(encoding="utf-8"))
    sites = []

    def site(file, offset, source, detail):
        line = source.count("\n", 0, offset) + 1
        excerpt = source.split("\n")[line - 1] if line - 1 < source.count("\n") + 1 else ""
        sites.append({"path": file, "line": line, "endLine": line, "category": SDK_DEBUG[0], "detail": detail[:300],
                      "source": " ".join(excerpt.split())[:220]})

    for file in sorted(texts):
        if not file.startswith(SDK_FOLDERS):
            continue
        text = texts[file]
        code = _mask_literals_and_comments(text)
        original_file = table["files"].get(file.rsplit("/", 1)[-1].lower()) if file.endswith(".c") else None
        if original_file:
            for match in FUNCTION_DEFINITION.finditer(code):
                original = original_file["functions"].get(match.group(1))
                if not original:
                    continue
                ours = _parameter_names(match.group(2))
                if len(ours) == len(original["params"]) and ours != original["params"]:
                    site(file, match.start(), text, "{}: Nintendo's debug info names the parameters ({}); here ({})".format(
                        match.group(1), ", ".join(original["params"]), ", ".join(ours)))
                if not original["locals"]:
                    continue
                body = code[match.end():_block_end(code, match.end() - 1)]
                extra = set()
                for declaration in LOCAL_DECLARATION.finditer(body):
                    if declaration.group(1) in NOT_A_TYPE:
                        continue
                    for part in declaration.group(2).split(","):
                        found = re.match(r"\s*\**\s*([A-Za-z_]\w*)", part)
                        if found and found.group(1) not in NOT_A_TYPE:
                            extra.add(found.group(1))
                extra -= set(original["locals"]) | set(original["params"])
                if extra:
                    site(file, match.start(), text, "{}: locals {} are not Nintendo's; its debug info has {}".format(
                        match.group(1), ", ".join(sorted(extra)), ", ".join(original["locals"])))
        for match in STRUCT_DEFINITION.finditer(code):
            end = _block_end(code, match.end() - 1)
            tail = re.match(r"\s*([A-Za-z_]\w*)", code[end + 1:end + 80])
            name = match.group(1) or (tail.group(1) if tail else None)
            members = table["structs"].get(name or "") or table["structs"].get(tail.group(1) if tail else "")
            if not members:
                continue
            # preprocessor lines (#if/#endif around members) are not members
            body = re.sub(r"^[ \t]*#[^\n]*", "", code[match.end():end], flags=re.MULTILINE)
            depth, ours = 0, []
            for statement in re.split(r";", body):
                depth_before = depth
                depth += statement.count("{") - statement.count("}")
                if depth_before == 0 and depth == 0:
                    found = re.search(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*(?::\s*\d+\s*)?$", statement.strip())
                    if found:
                        ours.append(found.group(1))
            extra = [member for member in ours if member not in members]
            if extra:
                site(file, match.start(), text, "struct {}: members {} are not Nintendo's; its debug info has {}".format(
                    name, ", ".join(extra[:6]), ", ".join(members[:12])))
    return sites


def scan_known_names(texts: Mapping[str, str]) -> List[dict]:
    """Every place a source or a symbols.txt still uses an address name whose original name is known, every
    module missing a file its retail asserts name, every static whose compiler number is not Nintendo's, and every
    SDK name Nintendo's debug info gives otherwise."""
    names = {entry["address_name"]: entry for entry in _load_known_names()}
    extra = scan_known_files(texts) + scan_counter_drift(texts) + scan_sdk_debug_names(texts)
    if not names:
        return extra
    pattern = re.compile(r"\b(" + "|".join(re.escape(name) for name in sorted(names)) + r")\b")
    sites = []
    for path in sorted(texts):
        if not (path.startswith(("src/", "include/")) or SYMBOL_FILES.fullmatch(path)):
            continue
        for number, line in enumerate(texts[path].split("\n"), 1):
            for match in pattern.finditer(line):
                entry = names[match.group(1)]
                sites.append({"path": path, "line": number, "endLine": number, "category": KNOWN_NAME[0],
                              "detail": "{} is {} in {} ({})".format(match.group(1), entry["name"], entry["where"],
                                                                     entry["evidence"])[:300],
                              "source": " ".join(line.split())[:220]})
    return sites + extra


def _symbol_files(repo: Path, commit: str) -> Dict[str, str]:
    """config/GP6E01/symbols.txt and every REL's symbols.txt at the commit."""
    raw = _git(repo, "ls-tree", "-r", "--name-only", commit, "--", "config")
    paths = [p.strip() for p in raw.decode("utf-8", "replace").splitlines() if SYMBOL_FILES.fullmatch(p.strip())]
    return dict(_pinned_sources(repo, commit, paths))


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
    raw = _git(repo, "ls-tree", "-r", "--name-only", commit, "--", "src", "include")
    paths = []
    for value in raw.decode("utf-8", "replace").splitlines():
        path = value.strip()
        if path.startswith(("src/", "include/")) and path.lower().endswith(SOURCE_SUFFIXES):
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
    if is_game_code(path):
        suffix = path[path.rfind("."):].lower()
        findings = _AUDIT.scan_text(source, path, suffix)
        if suffix in (".c", ".cpp", ".cp"):
            findings += _AUDIT.statement_findings(source, path)
        sites.extend(_audit_sites(findings, {path: source}))
    return sites


def build_cleanup(repo: Path, commit: str, generated_at: Optional[str] = None) -> dict:
    """Build the cleanup payload for an already validated commit id."""

    commit = _validate_commit(repo, commit)
    paths = _source_paths(repo, commit)
    sources = dict(_pinned_sources(repo, commit, paths))
    methodology = METHODOLOGY + [
        "Source is read from snapshot.commit through git object storage; the current checkout and binary files are never scanned.",
    ]
    return cleanup_document(sources, commit, generated_at, methodology, symbols=_symbol_files(repo, commit))


METHODOLOGY = [
    "The index lists all unfaithful code: every place the source differs from what the original programmers wrote, as far as the rules can tell. No exemption is applied, including evidence-based ones the recovery once accepted.",
    "Game code (src/game, src/board, src/REL and their headers under include/) goes through the recovery's faithfulness audit: library code rebuilt by hand, placeholder statements, invented assertions, match workarounds, raw numbers, decompiler and address names, process comments.",
    "Cross-file rules compare the whole tree: one function or global declared with different types, prototypes repeated outside their header, inline helpers written in several files, file names made from addresses. A REL's own names are compared within that REL; DOL functions everywhere.",
    "Every C/C++ file under src/ and include/, SDK and libraries included, also goes through the lexical pointer-cast, pragma and MUST_MATCH scan.",
    "Comments, string literals, and character literals are blanked before matching; line numbers refer to the scanned source.",
    "Pointer casts are counted at the outermost expression, so nested casts in one access are not emitted again. Pragmas count once except push/pop plumbing. A MUST_MATCH conditional counts once only when it contains non-directive code.",
    "Function and data names made from addresses (fn_1_XXXX, lbl_1_data_XXX) are not listed: the project keeps them until the original names are known.",
    "Once an original artifact shows the name (a link map, a $Id line, a retail string), the address name is listed as 'original name known' wherever a source or a symbols.txt still uses it. The artifacts so far: Hudson's own engine left unstripped on Hudson Selection Vol. 4 (huos.elf and huos.MAP, matched to MP6 by code or by symbol order), Nintendo's link map on the Mario Kart: Double Dash!! debug disc, and MP6's own retail assert texts, which name source files.",
]


def cleanup_document(sources: Mapping[str, str], commit: str, generated_at: Optional[str] = None,
                     methodology: Optional[Sequence[str]] = None, scan: Any = None, repository: Any = None,
                     symbols: Optional[Mapping[str, str]] = None) -> dict:
    """The cleanup payload for {path: text}, shared by the published site (git objects) and the local dashboard
    (the working tree, which passes cached versions of `scan` and `repository`). `symbols` holds the symbols.txt
    files, read only for address names whose original name is known."""

    sites: List[dict] = []
    for path in sorted(sources):
        sites.extend((scan or scan_source)(path, sources[path]))
    sites.extend((repository or scan_repository)(sources))
    sites.extend(scan_known_names({**dict(sources), **dict(symbols or {})}))
    categories = [dict(item) for item in CATEGORY_DEFINITIONS]
    known = {item["id"] for item in categories}
    order = dict(CATEGORY_ORDER)
    for site in sites:
        if site["category"] not in known:                 # a rule added to the audit after this table
            known.add(site["category"])
            order[site["category"]] = len(order)
            categories.append({"id": site["category"], "label": site["category"].replace("-", " "),
                               "family": "match-hack", "description": site.get("detail", "")[:120]})
    sites.sort(key=lambda item: (item["path"], item["line"], item["endLine"], order[item["category"]], item["source"]))
    return {
        "schemaVersion": 1,
        "commit": commit,
        "generatedAt": generated_at or datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "scannedFiles": len(sources),
        "families": [dict(item) for item in FAMILY_DEFINITIONS],
        "categories": categories,
        "sites": sites,
        "methodology": list(methodology if methodology is not None else METHODOLOGY),
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
