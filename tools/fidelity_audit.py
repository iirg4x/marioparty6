#!/usr/bin/env python3
"""Code-fidelity audit for REL modules: does the C read like original source, or like matching tricks?

source_shape_check.py checks file structure. This reads the code itself, in every file configure.py
compiles for a module plus the module-local files they include, and reports things that steer the
compiler or invent content instead of recovering it. It is read-only; every finding names file and line.

Categories (each finding is a prompt for review, not an automatic verdict):

  steering        #pragma (other than once), inline asm, __declspec/__attribute__, per-file compiler
                  flags or compiler versions that differ from the module's ordinary objects
  visibility      MWCC-only declaration guards (#if defined(__MWERKS__) ... ): context differences that
                  should be documented and evidence-backed
  invented-data   file-scope pad/filler/dummy/unused/reserved storage, long raw-hex initializers
  no-op-code      arithmetic that changes nothing (+0, -0, *1, /1, |0, ^0, <<0, >>0), !!, x = x,
                  null-pointer offset tricks, empty statements, do {} while (0), double integer casts
  offset-access   struct fields reached through casts and byte offsets (*(f32 *)(record + OFFSET),
                  ((u8 *)obj)[12]) instead of a struct type, named offset macros included
  raw-numbers     data file numbers and flag ids written as raw numbers (hex with 5+ digits, decimal with
                  6+ digits) where the original source used names; sound ids passed to the audio calls
                  as numbers; strings written as character codes
  intrinsics      __rlwinm, __cntlzw, __fabs and similar compiler intrinsics
  placeholders    decompiler names (temp_r31, var_r30, sp10, arg0, uVar1, local_8, DAT_, FUN_, M2C_), the same
                  names renamed to a numbered generic word (localValue5, value9, temporary1, parameter0),
                  m2c goto labels (goto block_20) and offset-named struct fields (field5C, unk_1C)
  probe-comments  comments that mention probes, steering, hacks, forcing, permuter, regalloc, matching
  comments        code for main should explain itself to a reader: a file description, a comment on every
                  function of more than three lines, and no comments about the recovery process (MWCC,
                  retail, inferred, evidence ...) in place of comments about the game
  layout          lines over 100 columns (rows of data tables excepted) and blank lines in a row; the
                  cleanup rewraps both mechanically

Usage:
  fidelity_audit.py MODULE [MODULE ...]
  fidelity_audit.py --faithful-from BOUNDARY.json      (modules whose source_shape result is PASS)
  options: --root REPO  --json OUT.json  --markdown OUT.md  --show N (examples per category)
           --address-names (also count fn_1_XXXX / lbl_1_bss_XX names; a measurement, not yet a gate)
"""
import argparse
import json
import re
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parents[4]
ORDINARY_FLAGS = {"-pooldata off"}


# ---------------------------------------------------------------------------------------------- configure.py

def rel_block(configure, module):
    """Text of the Rel(...) call for this module."""
    match = re.search(r"Rel\(\s*[\"']%s[\"']" % re.escape(module), configure)
    if not match:
        return ""
    depth = 0
    for index in range(match.start() + 3, len(configure)):
        if configure[index] == "(":
            depth += 1
        elif configure[index] == ")":
            depth -= 1
            if depth == 0:
                return configure[match.start():index + 1]
    return ""


OBJECT = re.compile(r"Object\(\s*(\w+)\s*,\s*[\"']([^\"']+)[\"'](.*?)\)\s*,?\s*$", re.MULTILINE)


def object_calls(block):
    """The argument text of every Object(...) call in a Rel block, single- or multi-line."""
    calls = []
    for match in re.finditer(r"\bObject\(", block):
        depth = 0
        for index in range(match.end() - 1, len(block)):
            if block[index] == "(":
                depth += 1
            elif block[index] == ")":
                depth -= 1
                if depth == 0:
                    calls.append(block[match.end():index])
                    break
    return calls


def module_objects(configure, module):
    """[{"name", "source", "flags": [...], "mw_version": text or None, "status"}] for the module."""
    objects = []
    for arguments in object_calls(rel_block(configure, module)):
        head = re.match(r"\s*(\w+)\s*,\s*[\"']([^\"']+)[\"'](.*)", arguments, re.DOTALL)
        if not head:
            continue
        status, name, rest = head.groups()
        source = re.search(r"source\s*=\s*[\"']([^\"']+)[\"']", rest)
        flags = re.search(r"extra_cflags\s*=\s*\[([^\]]*)\]", rest)
        version = re.search(r"mw_version\s*=\s*([\w\.\"'/]+)", rest)
        objects.append({
            "status": status,
            "name": name,
            "source": source.group(1) if source else name,
            "flags": re.findall(r"[\"']([^\"']*)[\"']", flags.group(1)) if flags else [],
            "mw_version": version.group(1) if version else None,
        })
    return objects


# ---------------------------------------------------------------------------------------------- C text

def split_code_and_comments(text):
    """(code, comments): the same text twice, once with comments blanked and once with only comments.

    Newlines are kept in both, so line numbers stay valid. String and character literals keep their
    quotes in the code text, with their contents blanked, so rules never match inside a string."""
    code, comments = [], []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            quote, j = c, i + 1
            while j < n and text[j] != quote and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            literal = text[i:j + 1]
            code.append(quote + " " * (len(literal) - 2) + literal[-1:] if len(literal) >= 2 else literal)
            comments.append(re.sub(r"[^\n]", " ", literal))
            i = j + 1
        elif text.startswith("/*", i) or text.startswith("//", i):
            end = text.find("*/", i + 2) + 2 if text.startswith("/*", i) else text.find("\n", i)
            if end <= 1 or end == -1:
                end = n
            comment = text[i:end]
            code.append(re.sub(r"[^\n]", " ", comment))
            comments.append(comment)
            i = end
        else:
            code.append(c)
            comments.append("\n" if c == "\n" else " ")
            i += 1
    return "".join(code), "".join(comments)


def line_of(text, offset):
    return text.count("\n", 0, offset) + 1


# ---------------------------------------------------------------------------------------------- rules

CODE_RULES = {
    "steering": [
        ("pragma", re.compile(r"^[ \t]*#[ \t]*pragma\b(?![ \t]+once\b).*$", re.MULTILINE)),
        ("inline asm", re.compile(r"\b(?:asm|__asm)\b\s*(?:\(|\{|void|int|f32|static)")),
        ("placement attribute", re.compile(r"__declspec\s*\(|__attribute__\s*\(\s*\(")),
        # a function cast to another prototype where it is called, ((void (*)(int, BOOL))mbObjDispSet)(...): the
        # original called it through its header's prototype, so the header is what is wrong
        ("prototype cast at a call", re.compile(
            r"\(\s*\(\s*\w[\w \t\*]*\(\s*\*\s*\)\s*\([^()]*\)\s*\)\s*[A-Za-z_]\w*\s*\)\s*\(")),
    ],
    # library code rebuilt by hand under invented names (27 Oct, user: "not rookie mistakes such as this"). The
    # C++ minigames use MSL C++'s std::vector, std::map/set and cdeque: retail keeps MSL's error exits,
    # fprintf(stderr, "vector::insert length error\n"); abort(). Our source rebuilt each instantiation as its own
    # struct (CategorySequence, InsertTree2C080, TreeNodeBase ...). The fix is the MSL C++ headers and
    "library-lookalike": [
        ("MSL error exit through a data label", re.compile(r"\bfprintf\s*\(\s*stderr\s*,(?!\s*\")")),
        ("MSL compressed pair rebuilt", re.compile(r"[\.>]\s*second\s*\(\s*\)|\bsecond\s*\(\s*\)\s*(?:const\s*)?\{")),
        ("MSL tree node rebuilt", re.compile(r"\bparentColor\b|\bparent\w*\s*\)?\s*&\s*~\s*(?:0x)?1(?:UL|ul|U|u|L)?\b")),
        ("MSL allocator rebuilt", re.compile(r"\bvoid\s+deallocate\s*\(")),
    ],
    "visibility": [
        ("MWCC-only guard", re.compile(r"^[ \t]*#[ \t]*(?:if|elif)[^\n]*__MWERKS__|^[ \t]*#[ \t]*ifn?def[ \t]+__MWERKS__",
                                       re.MULTILINE)),
    ],
    "invented-data": [
        # identifiers that are padding by name: pad, pad0, pad_1C, padding, filler, dummy, unused, reserved
        # (not padNo, padId or other words that merely start with "pad")
        ("pad/filler storage", re.compile(
            r"^[ \t]*(?:static[ \t]+)?(?:const[ \t]+)?\w+[ \t\*]+(?:\w*_)?(?:pad(?:ding)?|filler|dummy|unused|reserved)"
            r"(?:_?[0-9A-Fa-fx]+)?[ \t]*\[",
            re.MULTILINE | re.IGNORECASE)),
        # a string literal kept as a named data object and passed by that name, OSReport(lbl_1_data_8F0, n): the
        # original wrote the literal at the call, which has no name to keep (27 Oct, 389 such calls on main)
        ("string kept as a data label", re.compile(
            r"\b(?:OSReport|OSPanic|OSHalt|printf|fprintf|sprintf|snprintf|vsprintf|strcmp|strncmp|strcpy|strncpy|"
            r"strcat|strlen|strstr|strchr|DCPrintf)\s*\([^;]*?\blbl_\d+_(?:data|rodata|sdata2?)_[0-9A-Fa-f]+\b(?!\s*\[)")),
    ],
    "no-op-code": [
        # the left operand must not be a number: `1 << 0` and `FLAG | 0x0` style constants are ordinary
        ("plus/minus zero", re.compile(r"(?<![\w\.])(?![0-9])[A-Za-z_][\w]*[\)\]]?[ \t]*[\+\-][ \t]*0(?![\w\.xX])(?![ \t]*[\*/])")),
        ("times/divide by one", re.compile(r"(?<![\w\.])(?![0-9])[A-Za-z_][\w]*[\)\]]?[ \t]*[\*/][ \t]*1(?![\w\.xX])")),
        ("or/xor zero", re.compile(r"(?<![\w\.])(?![0-9])[A-Za-z_][\w]*[\)\]]?[ \t]*[\|\^][ \t]*0(?![\w\.xX])")),
        ("shift by zero", re.compile(r"(?<![\w\.])(?![0-9])[A-Za-z_][\w]*[\)\]]?[ \t]*(?:<<|>>)[ \t]*0(?![\w\.xX])")),
        ("double negation", re.compile(r"!!")),
        ("self assignment", re.compile(r"(?<![\w\.>])(\w+)[ \t]*=[ \t]*\1[ \t]*;")),
        ("null-pointer offset", re.compile(r"\(\s*\(\s*\w+\s*\*\s*\)\s*0\s*\)\s*(?:->|\[)|\(\s*\w+\s*\*\s*\)\s*0\s*\+")),
        ("empty statement", re.compile(r";[ \t]*;(?![^\n]*\))")),
        ("do-while-zero", re.compile(r"while[ \t]*\([ \t]*0[ \t]*\)[ \t]*;")),
        ("double integer cast", re.compile(r"\(\s*[us](?:8|16|32)\s*\)\s*\(\s*[us](?:8|16|32)\s*\)")),
    ],
    "offset-access": [
        # struct fields read through casts and byte offsets instead of a struct type:
        # *(f32 *)(record + 0x10), *(s16 *)((u8 *)work + OFFSET), ((u8 *)obj)[12]
        # the base and the offset may each be in parentheses, and the base may be an address
        # (13 Oct, Mount Duel: `*(s16 *)((s8 *)(playerData) + (TOURNAMENT_PLAYER_SCENE_STATE_OFFSET))` and
        # `(u8 *)*(void **)((u8 *)&lbl_1_bss_114 + playerNo * 4)` passed both patterns)
        ("cast-dereference of offset", re.compile(
            r"\*[ \t]*\([ \t]*(?:const[ \t]+)?\w+[ \t]*\*+[ \t]*\)[ \t]*\([ \t]*(?:\([ \t]*(?:const[ \t]+)?\w+[ \t]*\*[ \t]*\)[ \t]*)?"
            r"\(?[ \t]*&?[ \t]*[A-Za-z_][\w\.\->]*[ \t]*\)?[ \t]*[\+\-][ \t]*\(?[ \t]*[A-Za-z0-9_]")),
        ("byte-pointer offset", re.compile(
            r"\([ \t]*\([ \t]*(?:const[ \t]+)?(?:u8|s8|char|unsigned[ \t]+char)[ \t]*\*[ \t]*\)[ \t]*(?:"
            # ((u8 *)(obj) + OFFSET)
            r"\(?[ \t]*[A-Za-z_][\w\.\->]*[ \t]*\)?[ \t]*\)?[ \t]*\+[ \t]*\(?[ \t]*[A-Za-z0-9_]"
            # ((u8 *)obj)[12]; not a cast of buffer arithmetic, (char *)(saveBuf[slot] + offset)
            r"|[A-Za-z_][\w\.\->]*[ \t]*\)[ \t]*\["
            # ((u8 *)&object + OFFSET), ((s8 *)(&object) + 1): an address plus a byte offset, not the address of an
            # element (`(char *)&saveBuf[slot][COMMENT]`)
            r"|\(?[ \t]*&[ \t]*[A-Za-z_][\w\.\->]*[ \t]*\)?[ \t]*\+[ \t]*\(?[ \t]*[A-Za-z0-9_])")),
    ],
    "raw-numbers": [
        # 5+ hex digits in code: data file numbers (0x5F0005), flag ids (0x30004), bit patterns. The original
        # source names these (data-number macros, FLAG_ enums); masks such as 0xFFFF have fewer digits.
        ("raw hex number", re.compile(r"(?<![\w\.])0[xX](?![fF]{5,}\b)(?!80000000\b)[0-9A-Fa-f]{5,}\b")),
        # the same numbers written in decimal: 6291518 (data number 0x60003E), 1073741825U (flag bits)
        ("raw decimal number", re.compile(r"(?<![\w\.])[1-9]\d{5,}[uUlL]*\b(?!\.)")),
        # sound effect, music and stream ids passed as numbers: HuAudFXPlay(1847) tells a reader nothing,
        # the original names the sound. The id is the first argument of the HuAud play calls and msmSePlay,
        # the second of the board voice calls; volumes, pans, delays and handles are not ids.
        ("unnamed audio resource ID", re.compile(
            r"\b(?:HuAud(?:FXPlay\w*|FXEmiterPlay|BGMPlay|JinglePlay|SStream(?:Chan)?Play|SeqPlay)|msmSePlay|"
            # the board's wrappers (27 Oct, w11Dll: mbAudFXPlay(1539) behind a locally invented name)
            r"mbAud(?:FX(?:Obj(?:Set|Create|EmitterSet|EmitterCreate|RefSet)|Play|PosPlay|EmitterPlay)|GuidePlay))"
            r"\s*\(\s*(?:0[xX][0-9A-Fa-f]+|[0-9]+)[uUlL]*\s*(?=[,)])")),
        ("unnamed audio resource ID", re.compile(
            r"\b(?:mbPlayerVoice(?:Pan)?Play|mbObjSePlay)\s*\([^,()]*(?:\([^()]*\)[^,()]*)*,"
            r"\s*(?:0[xX][0-9A-Fa-f]+|[0-9]+)[uUlL]*\s*(?=[,)])")),
    ],
    "intrinsics": [
        ("intrinsic", re.compile(r"\b__(?:rlwinm|rlwimi|rlwnm|cntlzw|lhbrx|lwbrx|sthbrx|stwbrx|dcbz|dcbf|dcbst|sync|eieio|"
                                 r"isync|fabs|fnabs|frsqrte|fres|fsel|setflm|mulhw|mulhwu|abs|labs)\s*\(")),
    ],
    "placeholders": [
        ("decompiler name", re.compile(r"\b(?:temp_[rf]\d+|var_[rf]\d+(?:_\d+)?|sp[0-9A-F]{1,3}|arg\d|saved_reg_\w+|"
                                       r"[ui]Var\d+|[bcfdp]Var\d+|local_[0-9a-f]+|param_\d+|DAT_[0-9a-f]+|FUN_[0-9a-f]+|"
                                       r"M2C_\w+|unksp\w*|undefined[1248]?)\b")),
        ("offset-named field", re.compile(r"(?<=[\.>])(?:field|unk)_?[0-9A-Fa-f]{1,4}\b")),
        # m2c's control-flow labels: `goto block_20;` is decompiler structure, not something a programmer wrote
        ("decompiler goto label", re.compile(r"\bgoto[ \t]+(?:block|loop)_\d+\b")),
        # a sound, stream or music id named in a source file (27 Oct, w11Dll's MSM_SE_BRDTT_NET_START 1539, w03Dll's 17
        # MSM_SE_W03_*): the names belong to msm_se.h; a name made up here pretends to be one
        ("sound id named outside msm_se.h", re.compile(
            r"^[ \t]*#[ \t]*define[ \t]+MSM_(?:SE|STREAM|MUS|GRP)_\w+[ \t]+\(?[ \t]*(?:0[xX][0-9A-Fa-f]+|\d+)", re.MULTILINE)),
        # the same labels named by address: goto L_99B4
        ("address-named goto label", re.compile(r"\bgoto[ \t]+L_(?=[0-9A-Fa-f]*\d)[0-9A-Fa-f]{3,8}\b")),
        # a type invented once per place that uses it, named by an address: InsertTree2C080, Owner9F58,
        # M677Sequence2B544. One original type became many; the original named it once, in a header.
        ("address-named type", re.compile(
            r"\b(?:struct|class|union|enum)\s+([A-Z]\w*?[a-z_](?=[0-9A-F]*\d)[0-9A-F]{4,6}(?:_[0-9A-F]{4,6})*)\s*[:{]")),
    ],
}
# C linkage on C++ code: an extern "C" definition in a .cpp file keeps an address name (fn_1_7008) for what the
# original wrote as a C++ function or a class member, e.g. vector::insert taken out of its class as
# fn_1_7008(RegisteredArray *, ...). The module's entry points are C by necessity. Literals are blanked in the
# code text, so "C" reads as " ".
EXTERN_C_DEFINITION = re.compile(r'\bextern\s+"\s"\s+(?!\{)[^;{}()"]*?\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?\{')
ENTRY_POINTS = {"_prolog", "_epilog", "_unresolved", "ModuleProlog", "ModuleEpilog", "ObjectSetup"}
COMMENT_RULES = {
    "probe-comments": [
        # "steer" and "force" are only process notes next to compiler words: "slot 0 steers around obstacles" and
        # "force an alternate choice" describe the game and are fine
        ("probe/steering note", re.compile(
            r"\b(?:diagnostic|probe|never retain|hack\w*|permut\w*|regalloc|register alloc\w*|to match|"
            r"for matching|matching only|workaround|kludge|"
            r"steer(?:s|ed|ing)?\s+(?:the\s+)?(?:compiler|codegen|code\s*gen|register\w*|allocat\w*|scheduler|inliner)|"
            r"forc(?:e|es|ed|ing)\s+(?:the\s+)?(?:compiler|codegen|register\w*|allocat\w*|inlin\w*|spill\w*|reload\w*)|"
            r"fake\s+(?:symbol|local|variable|temp\w*|store|read|use)s?)\b", re.IGNORECASE)),
    ],
}
HEX_LITERAL = re.compile(r"0[xX][0-9A-Fa-f]{6,}")
INITIALIZER = re.compile(r"=\s*\{", re.MULTILINE)


CHAR_ARRAY = re.compile(r"\b(?:unsigned\s+|signed\s+)?(?:char|u8|s8)\s+(\w+)\s*\[[^\]]*\]\s*=\s*\{([^{}]*)\}")


def char_code_arrays(code):
    """(offset, detail) for char arrays written as numbers that spell printable text: the original
    source writes "632hakoniwa-PC01st", not { 54, 51, 50, ... }. Same bytes, so the fix is free."""
    hits = []
    for match in CHAR_ARRAY.finditer(code):
        values = [int(v, 0) for v in re.findall(r"-?(?:0[xX][0-9A-Fa-f]+|\d+)", match.group(2))]
        if len(values) < 4 or re.search(r"[A-Za-z_]", re.sub(r"0[xX][0-9A-Fa-f]+", "", match.group(2))):
            continue
        printable = [v for v in values if 32 <= v <= 126]
        text_end = values.index(0) if 0 in values else len(values)
        if text_end >= 3 and all(32 <= v <= 126 for v in values[:text_end]) and len(printable) >= 3:
            text = bytes(values[:text_end]).decode("ascii")
            hits.append((match.start(), f"{match.group(1)} spells \"{text}\" as character codes"))
    return hits


# Decompiler names renamed to a generic word plus a number: sp10 -> localValue0, var_r31 -> value9,
# temp_r3 -> temporary1, temp_f31 -> temporaryFloat0, arg0 -> parameter0. They still name nothing.
# These words never carry a number in real code:
ALWAYS_PLACEHOLDER = re.compile(r"\b(?:localValue|localVar|localVariable|tempValue|temporary(?:Float|Int|Value)?|"
                                r"variable|parameter|argument)\d+\b")
# these can (value1/value2 in a comparator), so they count only when a file numbers three or more of them
# (arg0 is m2c's own spelling and already a decompiler name):
SOMETIMES_PLACEHOLDER = re.compile(r"\b(value|var|val|temp|tmp|local|result|param)(\d+)\b")


FLOAT_LITERAL = re.compile(r"(?<![\w.])(\d+\.\d*|\.\d+)(?:[eE][-+]?\d+)?[fF]\b")


def float_spellings(code):
    """(offset, detail) for float literals written with more digits than a float holds, as a decompiler prints them:
    -0.699999988f is the float -0.7f (27 Oct, w11Dll). A programmer writes the short form; both compile the same."""
    hits = []
    for match in FLOAT_LITERAL.finditer(code):
        literal = match.group(0)[:-1]
        digits = len(re.sub(r"[eE].*", "", literal).replace(".", "").lstrip("0"))
        if digits < 8:
            continue
        try:
            value = struct.unpack("<f", struct.pack("<f", float(literal)))[0]
            for precision in range(1, 10):
                short = "%.*g" % (precision, value)
                if struct.unpack("<f", struct.pack("<f", float(short)))[0] == value:
                    break
        except OverflowError:
            continue                                     # beyond a float's range: not this rule's case
        if len(short.replace(".", "").replace("-", "").lstrip("0")) < digits:
            if "." not in short and "e" not in short:
                short += ".0"
            hits.append((match.start(), f"{match.group(0)} is the float {short}f"))
    return hits


LATE_INCLUDE = re.compile(r"^[ \t]*#[ \t]*include\b", re.MULTILINE)
TOP_DEFINITION = re.compile(r"^(?:static[ \t]+)?(?:inline[ \t]+)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)[ \t]*\([^;{}]*\)\s*\{",
                            re.MULTILINE)


def late_includes(code):
    """Offsets of #include lines after the first function definition: files joined from fragments keep each fragment's
    includes where it started (27 Oct, w06Dll's board_main.c includes game/board/main.h five times mid-file)."""
    first = next((m.start() for m in TOP_DEFINITION.finditer(code)
                  if m.group(1) not in ("if", "while", "for", "switch")), None)
    if first is None:
        return []
    return [m.start() for m in LATE_INCLUDE.finditer(code) if m.start() > first]


def numbered_placeholders(code):
    """(offset, detail) for each distinct renamed placeholder, at its first use in the file."""
    first = {}
    for match in ALWAYS_PLACEHOLDER.finditer(code):
        first.setdefault(match.group(0), match.start())
    stems = {}
    for match in SOMETIMES_PLACEHOLDER.finditer(code):
        stems.setdefault(match.group(1), {}).setdefault(match.group(0), match.start())
    for names in stems.values():
        if len(names) >= 3:
            for name, offset in names.items():
                first.setdefault(name, offset)
    return [(offset, f"{name}: a numbered generic name, not a real one")
            for name, offset in sorted(first.items(), key=lambda item: item[1])]


# Functions and data still named after their address (fn_1_3934, lbl_1_bss_0). Reported only with
# --address-names: main has used these names since the first recoveries, so whether they block a
# faithful module is the user's decision, not this tool's.
ADDRESS_NAME = re.compile(r"\b(?:fn_\d+_[0-9A-Fa-f]+|lbl_\d+_(?:data|bss|rodata|sdata2?|sbss2?)_[0-9A-Fa-f]+)\b")


def address_names(code):
    """(offset, detail) for each distinct address-derived name, at its first use in the file."""
    first = {}
    for match in ADDRESS_NAME.finditer(code):
        first.setdefault(match.group(0), match.start())
    return [(offset, f"{name}: named after its address") for name, offset in sorted(first.items(), key=lambda i: i[1])]


def raw_hex_initializers(code):
    """Offsets of brace initializers holding 8 or more long hex literals (bit patterns, raw words)."""
    hits = []
    for match in INITIALIZER.finditer(code):
        depth, j = 0, match.end() - 1
        while j < len(code):
            if code[j] == "{":
                depth += 1
            elif code[j] == "}":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        count = len(HEX_LITERAL.findall(code[match.end():j]))
        if count >= 8:
            hits.append((match.start(), f"{count} long hex literals in one initializer"))
    return hits


def symmetric_offset(code, match):
    """True when `x + 0` has a sibling `x + N` (N > 0) within 30 lines: a programmer writing parallel
    blocks for object x and object x + 4 writes `x + 0` for symmetry. That is ordinary source style."""
    operand = re.match(r"[A-Za-z_]\w*", match.group(0))
    if not operand:
        return False
    start = code.rfind("\n", 0, max(0, match.start() - 1))
    for _ in range(30):
        start = code.rfind("\n", 0, max(0, start - 1))
    end = match.end()
    for _ in range(30):
        next_line = code.find("\n", end + 1)
        end = len(code) if next_line == -1 else next_line
    window = code[max(0, start):end]
    sibling = re.compile(r"(?<![\w\.])%s[ \t]*\+[ \t]*[1-9]\d*(?![\w\.])" % re.escape(operand.group(0)))
    return bool(sibling.search(window))


def scan_file(path, display, with_address_names=False):
    return scan_text(path.read_text(encoding="utf-8", errors="replace"), display, path.suffix, with_address_names)


def scan_text(text, display, suffix, with_address_names=False):
    """The findings of one source text; `suffix` (".c", ".cpp", ".h") selects the language rules. The cleanup index
    scans git objects through this, without files on disk."""
    code, comments = split_code_and_comments(text)
    lines = text.split("\n")
    findings = []

    def add(category, rule, offset, detail=None, source=code):
        number = line_of(source, offset)
        excerpt = lines[number - 1].strip()[:160] if number - 1 < len(lines) else ""
        findings.append({"category": category, "rule": rule, "file": display, "line": number,
                         "text": detail or excerpt})

    # macro definitions (`#define FLAG (1 << 0)`) are ordinary constants, not no-op code
    code_without_directives = re.sub(r"^[ \t]*#[^\n]*", lambda m: " " * len(m.group(0)), code, flags=re.MULTILINE)
    for category, rules in CODE_RULES.items():
        # a #define is how a raw number gets its name, so raw-number rules do not look at directives either
        text_for_rules = code_without_directives if category in ("no-op-code", "raw-numbers") else code
        for rule, pattern in rules:
            for match in pattern.finditer(text_for_rules):
                if rule == "plus/minus zero" and symmetric_offset(code, match):
                    continue
                add(category, rule, match.start())
    for category, rules in COMMENT_RULES.items():
        for rule, pattern in rules:
            for match in pattern.finditer(comments):
                add(category, rule, match.start(), source=comments)
    for offset, detail in raw_hex_initializers(code):
        add("invented-data", "raw hex initializer", offset, detail=detail)
    for offset, detail in char_code_arrays(code):
        add("raw-numbers", "string as character codes", offset, detail=detail)
    for offset, detail in numbered_placeholders(code):
        add("placeholders", "numbered generic name", offset, detail=detail)
    for offset, detail in float_spellings(code):
        add("raw-numbers", "float in decompiler spelling", offset, detail=detail)
    for category, rule, offset, detail in retail_string_findings(text, code, display):
        add(category, rule, offset, detail=detail)
    if suffix in (".c", ".cpp", ".cp"):
        for offset in late_includes(code):
            add("invented-structure", "include after code", offset)
    if suffix in (".cpp", ".cp"):
        for match in EXTERN_C_DEFINITION.finditer(code):
            if match.group(1) not in ENTRY_POINTS:
                add("linkage", "C linkage on C++ code", match.start(),
                    detail=f"{match.group(1)}: extern \"C\" definition in a C++ file")
    if with_address_names:
        for offset, detail in address_names(code):
            add("address-names", "address-derived name", offset, detail=detail)
    if suffix == ".c":
        for rule, line_number, detail in comment_findings(text, code, comments):
            findings.append({"category": "comments", "rule": rule, "file": display, "line": line_number,
                             "text": detail})
    for rule, line_number, detail in layout_findings(lines):
        findings.append({"category": "layout", "rule": rule, "file": display, "line": line_number, "text": detail})
    return findings


# ---------------------------------------------------------------------------------------------- layout

LINE_LIMIT = 100


def is_table_row(line):
    """One row of a data table (`{ DLL_m601dll, MSM_GRP_M601, -1 },`): tables read best one row per line."""
    stripped = line.strip()
    return stripped.startswith("{") and stripped.rstrip(",;").endswith("}")


def layout_findings(lines):
    """(rule, line, detail) for lines a reader has to scroll sideways for, and for blank lines in a row."""
    out, previous_blank = [], False
    for number, line in enumerate(lines, 1):
        columns = len(line.rstrip("\r").expandtabs(4))
        if columns > LINE_LIMIT and not is_table_row(line):
            out.append(("line over 100 columns", number, f"{columns} columns: {line.strip()[:100]}"))
        blank = not line.strip()
        if blank and previous_blank and number < len(lines):     # the piece after the final newline is no line
            out.append(("blank lines in a row", number, "more than one blank line in a row"))
        previous_blank = blank
    return out


# ---------------------------------------------------------------------------------------------- comments
#
# Code that reaches main should read like the user's player.c example: the file says what it is for,
# each real function says what it does and when it runs, struct fields and odd steps are explained,
# and comments speak to a reader of the game code, not about how the code was recovered.

FUNCTION_START = re.compile(r"^(?!\s)(?!typedef\b)(?:static\s+)?(?:inline\s+)?[A-Za-z_][\w \*]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*\{?\s*$")
PROCESS_WORDS = re.compile(
    r"\b(?:MWCC|mwcc|retail|recover(?:ed|y)|inferred|target[- ]backed|provider|host (?:build|interface|contract)|"
    r"register alloc\w*|byte[- ]identical|diagnostic|matching|evidence|provenance|consumer|"   # not bare "allocation":
    # the game's own memory and sprite allocation is not a process note (12 Oct, mgtimer.c: 15 of 16 hits were)
    r"original (?:type|name|helper)|historical|observed)\b",
    re.IGNORECASE)


def function_bodies(code):
    """[(name, first line index, body line count)] for function definitions at file scope."""
    lines = code.split("\n")
    found = []
    depth = 0
    for index, line in enumerate(lines):
        if depth == 0:
            match = FUNCTION_START.match(line)
            opens_here = line.rstrip().endswith("{")
            opens_next = index + 1 < len(lines) and lines[index + 1].strip() == "{"
            if match and (opens_here or opens_next) and match.group(1) not in ("if", "for", "while", "switch"):
                end, body_depth = index, 0
                for end in range(index, len(lines)):
                    body_depth += lines[end].count("{") - lines[end].count("}")
                    if body_depth == 0 and "{" in "".join(lines[index:end + 1]):
                        break
                found.append((match.group(1), index, end - index))
        depth += line.count("{") - line.count("}")
    return found


def comment_findings(text, code, comments):
    """(rule, line, detail) for missing file and function comments and for process-note comments."""
    out = []
    text_lines, comment_lines = text.split("\n"), comments.split("\n")

    # 1. the file opens with a description: a comment before the first line of code, #include lines
    #    included (a file that only includes another one still needs a line saying why it exists)
    first_code = next((i for i, l in enumerate(code.split("\n")) if l.strip()), len(comment_lines))
    if not any(comment_lines[i].strip() for i in range(0, first_code)):
        out.append(("no file description", 1, "file does not open with a comment saying what it is for"))

    # 2. every function of more than three lines has a comment just above it
    for name, start, length in function_bodies(code):
        if length <= 4:
            continue
        above = [comment_lines[i].strip() for i in range(max(0, start - 3), start)]
        if not any(above):
            out.append(("uncommented function", start + 1, f"{name}: {length} lines, no comment above"))

    # 3. comments that talk about the recovery process instead of the game
    for i, line in enumerate(comment_lines):
        if PROCESS_WORDS.search(line):
            out.append(("process note", i + 1, text_lines[i].strip()[:140]))
    return out



# ---------------------------------------------------------------------------------------------- statements


# statements that make the compiler emit what retail has without being what the original wrote: `(void)0;`, a
# variable cast to void, an if with an empty body (user, 12 Oct: "i want faithfulness"; 10 Oct: tricks only as
# diagnostics, never kept). The original had real statements there (often debug macros a release build
# compiles to nothing); until their form is found the module is not faithful
DIAGNOSTIC_STATEMENT = re.compile(r"^\s*(?:\(void\)\s*(?:0|[A-Za-z_]\w*)\s*;|(?:else\s+)?if\s*\(.*\)\s*\{\s*\}\s*$)")
DIAGNOSTIC_RULE = "faithfulness/diagnostic statement"
# a statement that only reads (`table[i];`, `p->flags & 4;`): retail's address arithmetic proves a statement was
# private workspace, held from main as a diagnostic until the user decides on its natural form
STATEMENT_KEYWORDS = re.compile(r"^(?:return|break|continue|goto|case|default|do|else|while|for|if|switch|typedef|"
                                r"extern|static|struct|union|enum|const|volatile|register|asm|delete|throw)\b")
DECLARATION_LINE = re.compile(r"^[A-Za-z_][\w:<>]*(?:\s+[A-Za-z_][\w:<>]*|\s*(?:\*\s*)+[A-Za-z_]\w*)+(?:\[[^\]]*\])*"
                              r"(?:\s*,\s*(?:\*\s*)*[A-Za-z_]\w*(?:\[[^\]]*\])*)*$")
CALL = re.compile(r"[A-Za-z_]\w*\s*\(|[)\]]\s*\(")      # f(...), (*f)(...), table[i](...)
ASSIGNMENT = re.compile(r"(?<![=!<>])=(?!=)|<<=|>>=|\+\+|--")
MACRO_STATEMENT = re.compile(r"^[A-Z_][A-Z0-9_]*$")          # PIXEL_CPU_PAUSE; a statement-like macro
BITFIELD = re.compile(r"^[A-Za-z_][\w\s]*\s[A-Za-z_]\w*\s*:\s*\d+$")
LABEL_LINE = re.compile(r"^\s*(?:case\b.*|default|[A-Za-z_]\w*)\s*:$")
LEADING_LABEL = re.compile(r"^[A-Za-z_]\w*\s*:(?!:)(?!\s*\d+\s*;)\s*")   # a label before a statement on its line (not `u8 : 6;`)
UNNAMED_BITFIELD = re.compile(r"^[A-Za-z_][\w\s]*:\s*\d+$")      # `u8 : 6;` (padding bits, no name)
COMMENT_LINE = re.compile(r"^\s*(?://|/\*|\*(?:\s|/|$))")
# the SDK's assertion macros (include/dolphin/os.h) compile to `((void)0)` without _DEBUG, as every REL is built:
# the condition never reaches retail, so an assertion in game code is the held no-op statement with an invented
# evidence (a sibling decomp's same assert) or the user's decision
ASSERT_STATEMENT = re.compile(r"^\s*(?:ASSERT|ASSERTMSG[0-9]?|ASSERTLINE|ASSERTMSG[0-9]?LINE|ASSERTMSGLINEV)\s*\(")
# the one assertion the user accepted (14 Oct 05:10Z, m658dll fn_1_5E48/5EFC): `ASSERT(FALSE);` right under a case
# or default label marks an unsupported case. Release builds compile it to ((void)0), and that expression statement
# keeps the case its own jump-table target (an empty `;`, `break;` or empty macro does not), so no condition is
# retail strings: the original's own text names functions and variables (27 Oct: m657's fn_1_47EC prints
# "Error: M657ComPlayerSet() - !*com != NULL"; hu_archive.cpp's assert text says `idx`, the code `index`)
FUNCTION_START = re.compile(r"^[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*\)\s*\{", re.MULTILINE)
ADDRESS_FUNCTION = re.compile(r"fn_(?:\d+_)?[0-9A-Fa-f]+$")
STRING_SPAN = re.compile(r'"[^"\n]*"')
NAMED_IN_STRING = re.compile(r"^(?:\s*(?:Error|ERROR|Warning|WARNING)\s*:?\s*)?([A-Za-z_][A-Za-z0-9_]{3,})\s*\(\s*\)")
ASSERT_CALL = re.compile(r"__msl_assertion_failed\s*\(\s*\"")
ASSERT_ORIGIN = re.compile(r'\s*,\s*"([^"\n]+)"\s*,\s*(\d+)')       # after the text: the file and line
SOURCE_LITERAL = re.compile(r"[A-Za-z_][\w-]*\.(?:c|cpp|cp|h|hpp)")
LINE_AFTER = re.compile(r"\s*,\s*(0x[0-9A-Fa-f]+|\d+)\b")
IDENTIFIER = re.compile(r"\b[A-Za-z_]\w*\b")
C_WORDS = {"NULL", "TRUE", "FALSE", "true", "false", "sizeof", "this", "nullptr"}


def _bodies(code):
    """(name, body start, body end) of every function definition; strings and comments are blanked in `code`."""
    for match in FUNCTION_START.finditer(code):
        depth, at = 0, match.end() - 1
        while at < len(code):
            if code[at] == "{":
                depth += 1
            elif code[at] == "}":
                depth -= 1
                if depth == 0:
                    break
            at += 1
        yield match.group(1), match.end(), at


def retail_string_findings(text, code, display=""):
    """[(category, rule, offset, detail)]: an address-named function a retail string in it names, an assert text
    that names variables its code does not use, an assert whose retail file name is another file (the original
    wrote that code there, often a header's inline), and MSL's assert written out by hand."""
    found = []
    defined = {name for name, _start, _end in _bodies(code)}
    for name, start, end in _bodies(code):
        if not ADDRESS_FUNCTION.fullmatch(name):
            continue
        body = code[start:end]
        for span in STRING_SPAN.finditer(code, start, end):
            literal = text[span.start() + 1:span.end() - 1]
            named = NAMED_IN_STRING.match(literal)
            if not named or named.group(1) in defined or re.search(r"\b" + named.group(1) + r"\s*\(", body):
                continue
            found.append(("placeholders", "function a retail string names", span.start(),
                          f"{name} prints \"{literal[:80]}\": the original called it {named.group(1)}"))
            break
    for call in ASSERT_CALL.finditer(code):
        quote = call.end() - 1
        close = code.find('"', quote + 1)
        literal = text[quote + 1:close]
        statement = max(code.rfind(";", 0, call.start()), code.rfind("{", 0, call.start()),
                        code.rfind("}", 0, call.start()))
        condition = code[statement + 1:call.start()]
        written = set(IDENTIFIER.findall(condition))
        missing = sorted(set(IDENTIFIER.findall(literal)) - written - C_WORDS)
        origin = ASSERT_ORIGIN.match(text, close + 1)
        place = (f"; the original assert is {origin.group(1)}:{origin.group(2)}, so written as assert() it sits "
                 f"on line {origin.group(2)} (this is line {text.count(chr(10), 0, call.start()) + 1})") if origin else ""
        found.append(("library-lookalike", "MSL assert written out", call.start(),
                      f"assert({literal[:80]}) written out as its expansion{place}"))
        own = display.replace("\\", "/").rsplit("/", 1)[-1]
        if origin and own and origin.group(1) != own:
            found.append(("invented-structure", "assert from another file", call.start(),
                          f"the retail assert is {origin.group(1)}:{origin.group(2)}; the original wrote this "
                          f"code in {origin.group(1)}"))
        if missing and written:
            found.append(("placeholders", "assert text names other variables", call.start(),
                          f"the retail assert text \"{literal[:80]}\" names {', '.join(missing[:4])}, "
                          f"which the code does not use"))
    # __FILE__ and __LINE__ written as the retail values (OSPanic("data.c", 66, ...)): the original wrote the
    # macros, so the call sits on that line of that file
    for span in STRING_SPAN.finditer(code):
        literal = text[span.start() + 1:span.end() - 1]
        number = LINE_AFTER.match(text, span.end())
        if not SOURCE_LITERAL.fullmatch(literal) or not number or "__msl_assertion_failed" in code[
                max(0, span.start() - 60):span.start()]:
            continue
        found.append(("library-lookalike", "__FILE__ and __LINE__ written out", span.start(),
                      f"\"{literal}\", {number.group(1)}: the original wrote __FILE__, __LINE__ on line "
                      f"{int(number.group(1), 0)} of {literal} (this is line {text.count(chr(10), 0, span.start()) + 1})"))
    return found


# invented
ASSERT_UNREACHABLE = re.compile(r"^\s*ASSERT\s*\(\s*FALSE\s*\)\s*;")
NOT_A_CAST_CALL = re.compile(r"\b(?:sizeof)\s*\(")
# a cast to a basic type, so `(u32)(s16)x` is not read as a call `)(` (19 Oct: m647dll's discarded sign shift)
BASIC_CAST = re.compile(r"(?<![\w\s])\s*\(\s*(?:(?:const|volatile|unsigned|signed)\s+)*(?:u8|s8|u16|s16|u32|s32|u64|s64|f32|f64|"
                        r"int|char|short|long|float|double|BOOL|bool|size_t)\s*\**\s*\)")


def invented_assertion(line, previous):
    """An SDK assertion statement whose condition retail cannot show, except `ASSERT(FALSE);` directly under a
    case or default label (ASSERT_UNREACHABLE)."""
    if not ASSERT_STATEMENT.match(line):
        return False
    return not (ASSERT_UNREACHABLE.match(line) and LABEL_LINE.match(previous.rstrip()))
STATEMENT_ENDS = (";", "{", "}", ":")


VOID_NAME = re.compile(r"^\s*\(void\)\s*([A-Za-z_]\w*)\s*;")
VOID_OTHER = re.compile(r"^\s*\(void\)\s*(?:-?[0-9.]+f?|\(.*\))\s*;")
SELF_ASSIGNMENT = re.compile(r"^\s*([A-Za-z_][\w.\->\[\]]*)\s*=\s*\1\s*;")
PRIVATE_HEADER_RULE = "faithfulness/private header copy"
REPEATED_INCLUDE_RULE = "layout/repeated include"
INCLUDE_LINE = re.compile(r'^\s*#\s*include\s+([<"][^>"]+[>"])')
NUL_PADDING = re.compile(r'(?:\\0){2,}"')        # "text\n\0\0\0" (12 Oct: hsfdraw.c, colman.c, mic.c)
FUNCTION_HEAD = re.compile(r"^(?:static\s+|inline\s+)*[A-Za-z_][\w\s\*]*?\b(\w+)\s*\(([^;{]*)\)\s*\{?\s*$")


def padding_statements(text):
    """[(line number, line, reason)] of statements that only steer MWCC: `(void)name;` (except the
    unused-parameter idiom: a parameter the function never uses otherwise), `(void)<constant or expression>;`,
    `x = x;`, `(void)0;` and empty ifs. Discarded comparisons (`*sum == stored;`) are left: they read as real slips."""
    lines = text.splitlines()
    found = []
    previous = ""
    for index, line in enumerate(lines):
        stripped = line.strip()
        reason = None
        before, previous = previous, (line if stripped and not COMMENT_LINE.match(line) else previous)
        if VOID_NAME.match(line):
            name = VOID_NAME.match(line).group(1)
            head = next((k for k in range(index, -1, -1) if FUNCTION_HEAD.match(lines[k])
                         and not lines[k].startswith((" ", "\t"))
                         and FUNCTION_HEAD.match(lines[k]).group(1) not in ("if", "for", "while", "switch")), None)
            params = FUNCTION_HEAD.match(lines[head]).group(2) if head is not None else ""
            depth, end = 0, len(lines) - 1
            for k in range(head if head is not None else 0, len(lines)):
                depth += lines[k].count("{") - lines[k].count("}")
                if depth == 0 and k > (head or 0) and "{" in "".join(lines[head or 0:k + 1]):
                    end = k
                    break
            uses = sum(len(re.findall(rf"\b{name}\b", l)) for l in lines[(head or 0) + 1:end + 1] if not VOID_NAME.match(l))
            if not (re.search(rf"\b{name}\b", params) and uses == 0):
                reason = f"`(void){name};` only raises {name}'s reference count for MWCC's register choice"
        elif VOID_OTHER.match(line) or DIAGNOSTIC_STATEMENT.search(line):
            reason = "a statement that does nothing at run time, there only to steer the compiler"
        elif SELF_ASSIGNMENT.match(line):
            reason = "`x = x;` is not code a programmer writes; find the original form (a macro, a conversion)"
        elif NUL_PADDING.search(line):
            reason = ("explicit NULs inside a string literal fill retail's data layout; nobody writes them. Find what "
                      "really sat there (another string, a table, alignment of the next object)")
        # m647dll's `(u32)(s16)inputResult >> 31;` passed as one and went to a checker as an exemption claim)
        elif dead_evaluation(line, before) and not re.search(r"==|!=|<=|>=|(?<![<>])[<>](?![<>=])",
                                                             stripped.replace("->", ".")):
            reason = "a statement that only reads; find the construct the original had (a call or macro whose result it ignored)"
        elif empty_if_block(lines, index):
            reason = "a statement that does nothing at run time, there only to steer the compiler"
        if reason:
            found.append((index + 1, stripped, reason + "; find the form a former developer would take for the "
                                                       "actual source, with every object identical"))
    return found


IF_OPEN = re.compile(r"^\s*if\s*\(.*\)\s*\{\s*$")
BLOCK_CLOSE = re.compile(r"^\s*\}\s*$")


def empty_if_block(lines, index):
    """True when lines[index] opens a standalone if whose block closes on the next code line with nothing in it
    and no `else` follows: the empty if DIAGNOSTIC_STATEMENT finds on one line, written over two (`if (...) {` /
    `}`). An empty else-if branch is part of a chain and stays real code
    (w03Dll board.c's particle branches, game/wipe.c's WIPE_MODE_OUT)."""
    if not IF_OPEN.match(lines[index]):
        return False
    following = [k for k in range(index + 1, len(lines)) if lines[k].strip() and not COMMENT_LINE.match(lines[k])]
    if not following or not BLOCK_CLOSE.match(lines[following[0]]):
        return False
    after = following[1] if len(following) > 1 else None
    return after is None or not lines[after].lstrip().startswith("else")


def dead_evaluation(line, previous):
    """True for a whole-line statement that only reads: no assignment, increment or call, not a declaration,
    and starting a statement (the line before ended one, so a call's last argument line is not taken)."""
    text = LEADING_LABEL.sub("", line.strip())          # `done: return changed;`, `finish: ;` (13 Oct)
    if not text.endswith(";") or text.startswith(("#", "}", "{")) or COMMENT_LINE.match(text) \
            or STATEMENT_KEYWORDS.match(text) or "}" in text or "operator" in text or "template" in text:
        return False
    before = previous.rstrip()
    if before.endswith("\\") or (before and not before.endswith(STATEMENT_ENDS) and not before.endswith("*/")
                                 and not before.lstrip().startswith(("#", "//"))):
        return False                                     # a macro's or an expression's continuation line
    if before.endswith(":") and not LABEL_LINE.match(before):
        return False                                     # the ':' of a conditional expression, not a label
    if before.count("(") > before.count(")"):
        return False                                     # inside an open parenthesis: a `for (i = 0;` header's condition
    body = text[:-1].strip()
    if not body or text.count("(") != text.count(")") or text.count("[") != text.count("]"):
        return False
    if ASSIGNMENT.search(body) or CALL.search(BASIC_CAST.sub(" ", NOT_A_CAST_CALL.sub("", body))):
        return False
    # several members declared on one line of a struct (`s32 index, field_04; s16 model;`), each piece on its own
    # (13 Oct: m644dll's and m658dll's struct lines read as statements that only read)
    pieces = [piece.strip() for piece in body.split(";")] if "(" not in body else [body]
    return not any(DECLARATION_LINE.match(piece) or MACRO_STATEMENT.match(piece) or BITFIELD.match(piece)
                   or UNNAMED_BITFIELD.match(piece) for piece in pieces if piece)


# what joining per-function fragments and decompiler output into one file leaves behind (user, 13 Oct: "is this a
# comprehensive faithfulness pr of these modules?" - main's mgmfreedll menu.c declared 62 of its own variables
# extern, w05Dll's text_000A0.c kept 47 comments with no function under them, game/mic.c kept Ghidra's
# `/* 80092aa0 HuMCMount */` and `/* WARNING: Globals starting with '_' ... */` lines). None of it was in any audit
DECOMPILER_COMMENT = re.compile(r"/\*\s*(?:[0-9a-fA-F]{8}\s+[A-Za-z_]\w*\s*\*/|0x[0-9A-Fa-f]+\s*\*/|extern\s*\*/|"
                                r"WARNING:\s*(?:Globals starting with|"
                                r"Removing unreachable block|Could not recover|Subroutine does not return|Unknown "
                                r"calling convention|Type propagation|Variable defined which should be unmapped|"
                                r"Control flow encountered|Exception while decompiling|Inlined function))")
EXTERN_DATA = re.compile(r"^extern\s+(?!\"C\")[^;(]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*;")
DATA_DEFINITION = re.compile(r"^(?:static\s+)?(?:const\s+)?[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*"
                             r"(?:=|;)")
TOP_COMMENT = re.compile(r"^(?:/\*.*\*/|//.*)\s*$")
PROTOTYPE_LINE = re.compile(r"^(?:extern\s+|static\s+)*[A-Za-z_][\w\s\*]*?\b\w+\s*\([^;{]*\)\s*;\s*$")
SCAFFOLD_RULES = {"decompiler": "faithfulness/decompiler comment", "orphan": "comments/orphan comment",
                  "own extern": "faithfulness/own data declared extern", "repeat": "layout/repeated declaration"}


def scaffolding(text):
    """{rule key: [line numbers]} of fragment and decompiler leftovers in a C file:
    decompiler   Ghidra's address and WARNING comments, bare offset tags (`/* 0xB5B4 */`), `/* extern */`;
    orphan       a file-scope comment followed, after a blank line, by another comment or by declarations: it
                 describes a function that is not under it (the file's first comment, its header, is left);
    own extern   `extern T name;` of data the file itself defines (fragment preludes);
    repeat       the same `extern` declaration again."""
    lines = text.splitlines()
    found = {key: [] for key in SCAFFOLD_RULES}
    defined, externs = {}, {}
    for number, line in enumerate(lines, 1):
        if DECOMPILER_COMMENT.search(line):
            found["decompiler"].append(number)
        if not line or line[0] in " \t{}#/*":
            continue
        extern = EXTERN_DATA.match(line)
        if extern:
            name = extern.group(1)
            if name in externs and line.strip() == lines[externs[name] - 1].strip():
                found["repeat"].append(number)
            externs.setdefault(name, number)
            continue
        definition = DATA_DEFINITION.match(line)
        if definition and not PROTOTYPE_LINE.match(line) and not line.startswith(("typedef", "return")):
            defined.setdefault(definition.group(1), number)
    # only an extern after the definition repeats it: one before the definition can be what retail needs, since MWCC
    # reaches a global already defined in the file through a cached section base and one only declared by its own
    # address (27 Oct, w11Dll: moving the model handles above fn_1_1F4 cost a register and changed the code)
    found["own extern"] = sorted(n for name, n in externs.items() if name in defined and n > defined[name])
    first_comment = next((n for n, line in enumerate(lines, 1) if line.strip()), 0)
    for number, line in enumerate(lines, 1):
        if number == first_comment or not TOP_COMMENT.match(line) or DECOMPILER_COMMENT.search(line):
            continue
        if number < len(lines) and lines[number].strip():
            continue                                    # the comment sits on what it describes
        rest = [k for k in range(number, len(lines)) if lines[k].strip() and not DECOMPILER_COMMENT.search(lines[k])]
        following = lines[rest[0]] if rest else ""
        # a section comment heads the commented function below it; a function's comment with only another
        # unattached comment, declarations or includes after it describes nothing here
        if TOP_COMMENT.match(following):
            unattached = rest[0] + 1 >= len(lines) or not lines[rest[0] + 1].strip()
        else:
            unattached = not following or following.startswith("#include") or bool(
                EXTERN_DATA.match(following) or PROTOTYPE_LINE.match(following))
        if unattached and not line.lstrip("/* ").startswith(("clang-format", "=", "-")):
            found["orphan"].append(number)
    return found

def statement_findings(text, display):
    """The statement and leftover rules above as findings of one C/C++ text: placeholder statements (a read alone,
    `(void)x;`, `x = x;`, an empty if, an SDK assertion with an invented condition), decompiler comments, data the
    file declares extern although it defines it, and comments left without their function."""
    findings = []
    for number, line, reason in padding_statements(text):
        findings.append({"category": "statements", "rule": "placeholder statement", "file": display, "line": number,
                         "text": f"{line[:100]}: {reason}"})
    previous = ""
    for number, line in enumerate(text.splitlines(), 1):
        if invented_assertion(line, previous):
            findings.append({"category": "statements", "rule": "assertion with an invented condition", "file": display,
                             "line": number, "text": line.strip()[:160]})
        if line.strip() and not COMMENT_LINE.match(line):
            previous = line
    lines = text.split("\n")
    rules = {"decompiler": ("comments", "decompiler comment"), "orphan": ("comments", "comment without its function"),
             "own extern": ("declarations", "own data declared extern")}
    for key, numbers in scaffolding(text).items():
        if key in rules:
            for number in numbers:
                findings.append({"category": rules[key][0], "rule": rules[key][1], "file": display, "line": number,
                                 "text": lines[number - 1].strip()[:160]})
    return findings


# ---------------------------------------------------------------------------------------------- whole tree

# Rules that need every file at once (27 Oct, user: "Clean up index in both local and main should show all unfaithful
# code"): code the original wrote once and the recovery wrote again, declarations that disagree, and file names made
# from addresses. Only game code is judged: the SDK and runtime sources keep their own original conventions.

GAME_CODE = ("src/game/", "src/board/", "src/REL/", "include/game/", "include/REL/", "include/board/")
ADDRESS_FILE = re.compile(r"(?:^|_)(?=[0-9A-Fa-f]*\d)[0-9A-Fa-f]{4,8}$")
DECLARATION = re.compile(
    r"^[ \t]*((?:(?:extern|static|inline)[ \t]+)*)((?:const[ \t]+)?[A-Za-z_][\w \t\*]*?)[ \t\*]*\b([A-Za-z_]\w*)[ \t]*"
    r"\(([^;{}()]*(?:\([^()]*\)[^;{}()]*)*)\)[ \t]*;", re.MULTILINE)
EXTERN_GLOBAL = re.compile(r"^[ \t]*extern[ \t]+((?:const[ \t]+)?[A-Za-z_][\w \t\*]*?)[ \t\*]*\b([A-Za-z_]\w*)[ \t]*"
                         r"(\[[^\]]*\])?[ \t]*;", re.MULTILINE)
DEFINITION = re.compile(r"^(?:static[ \t]+)?(?:inline[ \t]+)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)[ \t]*\([^;{}]*\)\s*\{",
                        re.MULTILINE)
INLINE_DEFINITION = re.compile(r"^(?:static[ \t]+)?inline[ \t]+(?:static[ \t]+)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)"
                               r"[ \t]*\([^;{)]*\)\s*\{", re.MULTILINE)
NOT_A_TYPE = {"return", "if", "while", "for", "switch", "sizeof", "else", "case", "do", "goto", "typedef"}
TYPE_WORDS = {"const", "volatile", "unsigned", "signed", "struct", "union", "enum", "register", "void", "int", "char",
              "short", "long", "float", "double"}
# typedef spellings of one type: declarations that differ only in these do not disagree
SAME_TYPE = {"f32": "float", "f64": "double", "s8": "signed char", "u8": "unsigned char", "s16": "short",
             "u16": "unsigned short", "s32": "int", "u32": "unsigned int", "BOOL": "int", "long": "int",
             "unsigned long": "unsigned int", "signed long": "int", "HuVecF": "Vec", "HuVec2F": "Vec2d"}
# the engine's branch table declares every target as `void f();` on purpose (an asm jump table)
JUMP_TABLES = {"src/game/kerent.c"}


def is_game_path(display):
    return display.startswith(GAME_CODE)


def link_scope(display):
    """The program a name belongs to: each REL links on its own, everything else is the DOL."""
    parts = display.split("/")
    if parts[:2] == ["src", "REL"] and len(parts) > 3:
        return "REL:" + parts[2]
    if parts[:2] == ["include", "REL"] and len(parts) > 2:
        module = parts[2] if len(parts) > 3 else re.match(r"[A-Za-z0-9]+", parts[2]).group(0)
        return "REL:" + module
    return "DOL"


def brace_depths(code):
    """depth[i]: how many braces enclose offset i, not counting `extern "C" {` blocks."""
    depths, depth, stack = [0] * (len(code) + 1), 0, []
    for index, char in enumerate(code):
        depths[index] = depth
        if char == "{":
            linkage = re.search(r'extern\s+"\s"\s*$', code[max(0, index - 24):index]) is not None
            stack.append(linkage)
            depth += 0 if linkage else 1
        elif char == "}" and stack:
            depth -= 0 if stack.pop() else 1
    depths[len(code)] = depth
    return depths


TYPEDEF = re.compile(r"^[ \t]*typedef[ \t]+((?:(?:const|volatile|unsigned|signed|struct|union|enum)[ \t]+)*"
                     r"[A-Za-z_]\w*[ \t\*]*)\b([A-Za-z_]\w*)[ \t]*;", re.MULTILINE)


def normal_type(text, aliases=SAME_TYPE):
    """One spelling per type: typedef names resolved, struct/union/enum tags and const dropped."""
    text = " ".join(re.sub(r"\b(?:struct|union|enum|const|volatile)\b", " ", text).replace("*", " * ").split())
    for _ in range(4):                                       # typedefs of typedefs
        resolved = " ".join(aliases.get(word, word) for word in text.split())
        if resolved == text:
            break
        text = resolved
    for alias, plain in SAME_TYPE.items():
        text = re.sub(r"(?<![\w])%s(?![\w])" % re.escape(alias), plain, text)
    return text.replace(" ", "")


FUNCTION_POINTER_TYPEDEF = re.compile(r"^[ \t]*typedef[ \t]+([^;()]+?)[ \t]*\([ \t]*\*[ \t]*([A-Za-z_]\w*)[ \t]*\)"
                                      r"[ \t]*\(([^;()]*)\)[ \t]*;", re.MULTILINE)


def top_level_split(text):
    """Split at commas outside parentheses: `int, void (*f)(int, int)` is two parameters."""
    parts, depth, start = [], 0, 0
    for index, char in enumerate(text):
        depth += {"(": 1, ")": -1}.get(char, 0)
        if char == "," and depth == 0:
            parts.append(text[start:index])
            start = index + 1
    parts.append(text[start:])
    return parts


def function_pointer_type(param, aliases):
    """`void (*hook)(int no)` -> 'void(*)(int)'; None when the parameter is not a function pointer."""
    match = re.match(r"^\s*(.+?)\(\s*\*\s*[A-Za-z_]?\w*\s*\)\s*\((.*)\)\s*$", param, re.S)
    if not match:
        return None
    inner = parameter_types(match.group(2), False, aliases) or ()
    return normal_type(match.group(1), aliases) + "(*)(" + ",".join(inner or ("void",)) + ")"


def parameter_types(params, cpp, aliases=SAME_TYPE):
    """('int', 'Vec*') for 'int no, Vec *pos'; None for `()` in C, where it leaves the arguments unchecked."""
    params = params.strip()
    if params == "":
        return () if cpp else None
    if params == "void":
        return ()
    types = []
    for param in top_level_split(params):
        pointer = function_pointer_type(param, aliases)
        if pointer:
            types.append(pointer)
            continue
        words = re.findall(r"[A-Za-z_]\w*|\*|\.\.\.|\[[^\]]*\]", param)
        if len(words) > 1 and re.match(r"[A-Za-z_]", words[-1]) and words[-1] not in TYPE_WORDS \
                and not (len(words) == 2 and words[0] in ("struct", "union", "enum")):
            words = words[:-1]
        types.append(normal_type(" ".join("*" if w.startswith("[") else w for w in words), aliases))
    return tuple(types)


def repository_findings(sources):
    """Findings that compare files: {display path: text} -> [{category, rule, file, line, text}]."""
    findings = []
    parsed = {}
    aliases = dict(SAME_TYPE)
    for display, text in sources.items():
        code, _comments = split_code_and_comments(text)
        parsed[display] = (code, brace_depths(code))
        for match in FUNCTION_POINTER_TYPEDEF.finditer(code):
            inner = parameter_types(match.group(3), False) or ()
            aliases.setdefault(match.group(2), normal_type(match.group(1)) + "(*)(" + ",".join(inner or ("void",)) + ")")
        for match in TYPEDEF.finditer(code):
            base = " ".join(re.sub(r"\b(?:struct|union|enum|const)\b", " ", match.group(1)).split())
            if base != match.group(2):
                aliases.setdefault(match.group(2), base)

    def add(category, rule, display, offset, detail):
        code = parsed[display][0]
        findings.append({"category": category, "rule": rule, "file": display, "line": line_of(code, offset),
                         "text": detail})

    declarations = defaultdict(list)          # (scope, name) -> [(display, offset, return, params)]
    header_names = {}                         # (scope, name) -> header
    defined = defaultdict(set)                # display -> names defined there
    definers = defaultdict(set)               # (scope, name) -> files defining it with external linkage
    globals_ = defaultdict(list)              # (scope, name) -> [(display, offset, type)]
    helpers = defaultdict(list)               # body -> [(display, offset, name)]
    for display, (code, depths) in parsed.items():
        if display in JUMP_TABLES:
            continue
        suffix = display[display.rfind("."):]
        cpp = suffix in (".cpp", ".cp", ".hpp")
        scope = link_scope(display)
        for match in DEFINITION.finditer(code):
            if depths[match.start()] == 0:
                defined[display].add(match.group(1))
                if not match.group(0).lstrip().startswith("static"):
                    definers[(scope, match.group(1))].add(display)
        for match in DECLARATION.finditer(code):
            storage, returned, name, params = match.groups()
            if depths[match.start()] or name in NOT_A_TYPE or returned.strip() in NOT_A_TYPE or "static" in storage:
                continue
            key = (scope, name)
            declarations[key].append((display, match.start(), normal_type(returned, aliases),
                                      parameter_types(params, cpp, aliases)))
            if display.startswith("include/"):
                header_names.setdefault(key, display)
        for match in EXTERN_GLOBAL.finditer(code):
            if not depths[match.start()]:
                shape = normal_type(match.group(1), aliases) + ("[]" if match.group(3) else "")
                globals_[(scope, match.group(2))].append((display, match.start(), shape))
        if is_game_path(display):
            for match in INLINE_DEFINITION.finditer(code):
                depth, end = 0, match.end() - 1
                while end < len(code):
                    depth += {"{": 1, "}": -1}.get(code[end], 0)
                    if depth == 0:
                        break
                    end += 1
                body = code[match.end():end]
                if body.count(";") >= 1 and len(body) >= 25:
                    key = re.sub(r"\s+", "", re.sub(r"\b%s\b" % re.escape(match.group(1)), "SELF", body))
                    helpers[key].append((display, match.start(), match.group(1)))

    # a DOL function's name is the same function in every REL that calls it
    dol_names = {name for scope, name in declarations if scope == "DOL"}
    merged = defaultdict(list)
    for (scope, name), entries in declarations.items():
        merged[("DOL", name) if name in dol_names else (scope, name)] += entries
    for (scope, name), entries in merged.items():
        typed = {(r, p) for _d, _o, r, p in entries if p is not None}
        conflict = len({r for r, _p in typed}) > 1 or len({p for _r, p in typed}) > 1
        for display, offset, returned, params in entries:
            if not is_game_path(display):
                continue
            if params is None and typed:
                add("declarations", "declared without parameters", display, offset,
                    f"{name}(): no parameter list here, so its arguments go unchecked; elsewhere "
                    f"({', '.join(next(iter(typed))[1]) or 'void'})")
            elif conflict and params is not None:
                others = sorted({f"{r} ({', '.join(p)})" for r, p in typed} - {f"{returned} ({', '.join(params)})"})
                add("declarations", "function declared with different types", display, offset,
                    f"{name}: {returned} ({', '.join(params)}) here; elsewhere {'; '.join(others[:2])}")
        header = header_names.get((scope, name)) or header_names.get(("DOL", name))
        owners = definers.get((scope, name)) or definers.get(("DOL", name)) or set()
        for display, offset, _r, _p in entries:
            if not display.endswith((".c", ".cpp", ".cp")) or not is_game_path(display) or name in defined[display]:
                continue
            if header:
                add("declarations", "local prototype of a header function", display, offset,
                    f"{name}: declared in {header}; the original included it")
            elif owners:
                # no header declares it at all (27 Oct, w11Dll: mbWipeCreate, mbHermiteCalc, mbScrollInit): the header of
                # the defining file is missing it
                add("declarations", "prototype of another file's function", display, offset,
                    f"{name}: defined in {sorted(owners)[0]}, whose header does not declare it; declare it there")
    for (scope, name), entries in globals_.items():
        if len({shape for _d, _o, shape in entries}) > 1:
            for display, offset, shape in entries:
                if is_game_path(display):
                    others = sorted({s for _d, _o, s in entries} - {shape})
                    add("declarations", "global declared with different types", display, offset,
                        f"{name}: {shape} here; elsewhere {', '.join(others[:2])}")
    for key, sites in helpers.items():
        files = sorted({display for display, _o, _n in sites})
        if len(files) > 1:
            names = sorted({name for _d, _o, name in sites})
            for display, offset, name in sites:
                add("duplicates", "helper defined in several files", display, offset,
                    f"{name}: the same body in {len(files)} files ({', '.join(names[:4])}); the original defined it "
                    "once, in a header or library")
    for display in parsed:
        stem = display.rsplit("/", 1)[-1].rsplit(".", 1)[0]
        if is_game_path(display) and display.endswith((".c", ".cpp", ".cp")) and ADDRESS_FILE.search(stem):
            add("file-names", "address-named source file", display, 0,
                f"{display.rsplit('/', 1)[-1]}: named by an address, not by what the file holds")
    return findings


# ---------------------------------------------------------------------------------------------- module

QUOTED_INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+)"', re.MULTILINE)


def is_game_code(root, path):
    """Game code lives under src/REL/ and include/REL/; SDK and runtime library sources are not audited."""
    return (root / "src" / "REL") in path.parents or (root / "include" / "REL") in path.parents


def local_files(root, compiled):
    """Compiled game sources plus the game-code files they include with quoted includes."""
    seen, queue = [], [p.resolve() for p in compiled]
    while queue:
        path = queue.pop(0)
        if path in seen or not path.exists() or not is_game_code(root, path):
            continue
        seen.append(path)
        text = path.read_text(encoding="utf-8", errors="replace")
        for name in QUOTED_INCLUDE.findall(text):
            for base in (path.parent, root / "src", root / "include", root):
                candidate = (base / name).resolve()
                if candidate.exists():
                    queue.append(candidate)
                    break
    return seen


def audit_module(root, configure, module, with_address_names=False):
    objects = module_objects(configure, module)
    findings = []
    ordinary = Counter(tuple(sorted(o["flags"])) for o in objects if o["mw_version"] is None)
    usual_flags = set(ordinary.most_common(1)[0][0]) if ordinary else set()
    for o in objects:
        unusual = [f for f in o["flags"] if f not in usual_flags | ORDINARY_FLAGS and f != "-DMP6_REL_RUNTIME=1"]
        runtime = o["source"].endswith("runtime.c") and o["mw_version"] == "config.linker_version"
        standard = o["mw_version"] in (None, '"GC/1.3.2"')
        if unusual or not (standard or runtime):
            findings.append({"category": "steering", "rule": "per-file compiler setting", "file": o["source"],
                             "line": 0, "text": f"flags {o['flags']} mw_version {o['mw_version']}"})
        if o["status"] != "Matching":
            findings.append({"category": "steering", "rule": "object not Matching", "file": o["source"],
                             "line": 0, "text": o["status"]})
    compiled = [root / "src" / o["source"] for o in objects]
    files = local_files(root, compiled)
    for path in files:
        display = path.relative_to(root).as_posix()
        findings += scan_file(path, display, with_address_names)
    return {"module": module, "objects": len(objects), "files": [p.relative_to(root).as_posix() for p in files],
            "findings": findings}


def faithful_modules(boundary_path):
    data = json.loads(Path(boundary_path).read_text(encoding="utf-8"))
    return sorted(row["module"] for row in data["source_shape"] if row["result"] == "PASS")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("modules", nargs="*")
    parser.add_argument("--faithful-from", type=Path)
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--markdown", type=Path)
    parser.add_argument("--show", type=int, default=0)
    parser.add_argument("--address-names", action="store_true",
                        help="also report fn_1_XXXX / lbl_1_bss_XX names (not a gate unless the user decides so)")
    args = parser.parse_args()
    modules = list(args.modules)
    if args.faithful_from:
        modules += faithful_modules(args.faithful_from)
    if not modules:
        parser.error("give modules or --faithful-from")
    root = args.root.resolve()
    configure = (root / "configure.py").read_text(encoding="utf-8")
    reports = [audit_module(root, configure, m, args.address_names) for m in modules]

    categories = ["steering", "visibility", "invented-data", "no-op-code", "offset-access", "raw-numbers",
                  "intrinsics", "placeholders", "library-lookalike", "linkage", "probe-comments", "comments", "layout"]
    if args.address_names:
        categories.append("address-names")
    header = f"{'module':14s} {'files':>5s} " + " ".join(f"{c[:10]:>10s}" for c in categories)
    lines = [header]
    for r in reports:
        counts = Counter(f["category"] for f in r["findings"])
        lines.append(f"{r['module']:14s} {len(r['files']):5d} " + " ".join(f"{counts.get(c, 0):10d}" for c in categories))
    print("\n".join(lines))
    totals = Counter(f["category"] for r in reports for f in r["findings"])
    rules = Counter((f["category"], f["rule"]) for r in reports for f in r["findings"])
    print("\ntotals by rule:")
    for (category, rule), count in sorted(rules.items()):
        print(f"  {category:14s} {rule:26s} {count:6d}")
    if args.show:
        examples = defaultdict(list)
        for r in reports:
            for f in r["findings"]:
                key = (f["category"], f["rule"])
                if len(examples[key]) < args.show:
                    examples[key].append(f)
        for key, rows in sorted(examples.items()):
            print(f"\n{key[0]} / {key[1]}:")
            for f in rows:
                print(f"  {f['file']}:{f['line']}  {f['text']}")
    if args.json:
        args.json.write_text(json.dumps({"modules": reports, "totals": dict(totals)}, indent=1), encoding="utf-8")
    if args.markdown:
        args.markdown.write_text("```\n" + "\n".join(lines) + "\n```\n", encoding="utf-8")


if __name__ == "__main__":
    main()
