"""The decision status of each cleanup index site: "decided" when a recorded user decision covers it, else "open".

The index still lists every site (its methodology: no exemption is applied). This module only labels them. A site
is decided when one of DECISIONS covers it, and the site then names that decision. Each decision has a record:
a decision file of the recovery, or the audit's own decision handling. A kind that only looks decided, with no
record, stays open.

Two kinds of coverage:

- by audit rule, the way the recovery's own open-findings count leaves user-decided kinds out (hex stays hex:
  the raw hex number and raw hex initializer rules; sound ids no header names);
- by source form, the forms the pinned audit admits under a decision but an older vendored audit may still list:
  `ASSERT(FALSE);` directly under a case label or as the only statement of an if or else body, and a call of a
  file's empty static inline debug hook with the value retail evaluates.

Audit sites carry their rule as "_rule" until `apply` reads it and removes it, so the published sites keep their
shape; every site gains "status", and decided ones "decision".
"""

from __future__ import annotations

import re
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Optional, Sequence


OPEN = "open"
DECIDED = "decided"

# Records live in the recovery harness's decisions folder (DECISION_FOLDER); the case-label assertion's record is
# the audit's own ASSERT_UNREACHABLE rule, which cites the user's 14 Oct answer.
DECISION_FOLDER = "decisions"
DECISIONS: Sequence[Mapping[str, str]] = (
    {
        "id": "unread-fields",
        "label": "Unread fields keep their offset names",
        "date": "2026-10-09",
        "record": "decisions/20261009-user-unread-fields.json",
        "summary": "A field no code reads keeps its offset name with an explicit never-read comment at its declaration.",
    },
    {
        "id": "hex-stays-hex",
        "label": "Hex stays hex",
        "date": "2026-11-01",
        "record": "decisions/20261101-user-hex-faithful.json",
        "summary": "Numbers keep their hex spelling: no shift rewrites and no named constants (the user's reply: "
                   "\"I choose faithful\").",
    },
    {
        "id": "unnamed-sound-ids",
        "label": "Unnamed sound ids stay numbers",
        "date": "2026-10-27",
        "record": "decisions/20261027-user-unnamed-sound-ids.json",
        "summary": "A sound id no header names stays a number, and the index still lists it; inventing a name stays "
                   "forbidden.",
    },
    {
        "id": "assert-false-case-label",
        "label": "ASSERT(FALSE) under a do-nothing case",
        "date": "2026-10-14",
        "record": "fidelity_audit.py ASSERT_UNREACHABLE",
        "summary": "A case that does nothing but must stay its own jump-table target gets ASSERT(FALSE); directly "
                   "under its label (the user, 14 Oct).",
    },
    {
        "id": "assert-false-if-body-o0",
        "label": "ASSERT(FALSE) as an -O0 if-body",
        "date": "2026-11-04",
        "record": "decisions/20261104-assert-false-empty-if-body-O0.json",
        "summary": "At -O0, an if-body whose statement compiles to nothing is ASSERT(FALSE);, extending the 14 Oct "
                   "case-label decision.",
    },
    {
        "id": "debug-value-hook",
        "label": "Debug hook for a dropped evaluation",
        "date": "2026-10-14",
        "record": "decisions/20261023-debug-hook-register-conversion.json",
        "summary": "A value retail evaluates and drops is passed to an empty static inline debug hook (the user's "
                   "14 Oct \"Allow it\").",
    },
)
DECISION_BY_ID = {decision["id"]: decision for decision in DECISIONS}

# audit rule ("category/rule") -> decision id. These are the kinds the recovery's own count leaves out as decided
# (harness checks.USER_DECIDED_RULES and the module audit's DECIDED hex family).
RULE_DECISIONS: Mapping[str, str] = {
    "raw-numbers/raw hex number": "hex-stays-hex",
    "invented-data/raw hex initializer": "hex-stays-hex",
    "raw-numbers/unnamed audio resource ID": "unnamed-sound-ids",
}

# Every pattern below reads one source line and has bounded repetition.
ASSERT_FALSE = re.compile(r"^[ \t]{0,40}ASSERT[ \t]{0,4}\([ \t]{0,4}FALSE[ \t]{0,4}\)[ \t]{0,4};")
LABEL_LINE = re.compile(r"^[ \t]{0,40}(?:case\b.{0,200}|default|[A-Za-z_]\w{0,80})[ \t]{0,4}:$")
COMMENT_LINE = re.compile(r"^[ \t]{0,80}(?://|/\*|\*(?:\s|/|$))")
IF_BODY_OPEN = re.compile(r"^[ \t]{0,40}(?:\}[ \t]{0,4})?(?:else[ \t]{1,4})?(?:if[ \t]{0,4}\(.{0,400}\)|else)"
                          r"[ \t]{0,4}\{[ \t]{0,4}$")
IF_BODY_HEAD = re.compile(r"^[ \t]{0,40}(?:\}[ \t]{0,4})?(?:else[ \t]{1,4})?(?:if[ \t]{0,4}\(.{0,400}\)|else)"
                          r"[ \t]{0,4}$")
IF_BODY_BRACE = re.compile(r"^[ \t]{0,40}\{[ \t]{0,4}$")
IF_BODY_CLOSE = re.compile(r"^[ \t]{0,40}\}")
# `static inline void DebugValue(f32 value) { }`: the hook takes one value and does nothing
HOOK_DEFINITION = re.compile(r"^[ \t]{0,8}(?:static[ \t]{1,4})?inline[ \t]{1,4}(?:static[ \t]{1,4})?void[ \t]{1,4}"
                             r"(Debug[A-Z]\w{0,40})[ \t]{0,4}\([ \t]{0,4}[A-Za-z_][\w \t\*]{0,60}?[A-Za-z_]\w{0,40}"
                             r"[ \t]{0,4}\)[ \t]{0,4}\{[ \t]{0,4}\}", re.MULTILINE)
HOOK_CALL = re.compile(r"^[ \t]{0,40}(Debug[A-Z]\w{0,40})[ \t]{0,4}\((.{1,300})\)[ \t]{0,4};[ \t]{0,4}$")
HOOK_CATEGORIES = ("placeholder-statement", "no-op")

FIELD_CATEGORIES = ("offset-field", "decompiler-name", "numbered-name")
# names.py UNREAD_NOTE, plus the passive wording used by the fields/refactor tasks.
UNREAD_COMMENT = re.compile(r"\bnever[ \t]+reads?\b", re.IGNORECASE)
FIELD_DECLARATION = re.compile(
    r"^[ \t]*(?:[A-Za-z_]\w*[ \t]+)+[* \t]*"
    r"((?:field|unk(?:nown)?)_?[0-9A-Fa-f]{1,4}|value_?[0-9A-Fa-f]{2,4})"
    r"[ \t]*(?:\[[^\]\n]*\][ \t]*)*(?::[ \t]*\d+[ \t]*)?;[ \t\r]*$")
SOURCE_PART = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.DOTALL)
RECORD_HEAD = re.compile(r"\b(?:struct|union|class)\b[^;{}()]*$")


def _field_declarations(text: str) -> Dict[int, str]:
    """Commented offset declarations directly in a record, never locals, globals or uses."""
    comments = [""] * len(text.split("\n"))

    def blank(match):
        value = match.group()
        if value.startswith(("/*", "//")):
            first = text.count("\n", 0, match.start())
            for index, line in enumerate(value.split("\n"), first):
                comments[index] += line
        return re.sub(r"[^\n]", " ", value)

    code = SOURCE_PART.sub(blank, text)
    declarations, stack, head = {}, [], ""
    for index, line in enumerate(code.split("\n")):
        declaration = FIELD_DECLARATION.fullmatch(line)
        if declaration and stack and stack[-1] and any(
                UNREAD_COMMENT.search(comments[k]) for k in range(max(0, index - 1), index + 1)):
            declarations[index] = declaration.group(1)
        # A method body introduces a non-record scope even inside a class. Inheritance is allowed.
        for part in re.split(r"([{};])", line):
            if part == "{":
                stack.append(bool(RECORD_HEAD.search(head)))
                head = ""
            elif part == "}":
                if stack:
                    stack.pop()
                head = ""
            elif part == ";":
                head = ""
            else:
                head += part
        head += "\n"
    return declarations


def field_read_anywhere(sources: Mapping[str, str], field: str) -> bool:
    """Snapshot adapter of harness/header_fields.py read_anywhere: the identical use pattern and predicate.

    The snapshot replaces git grep's src/include lines. Like the harness, a read on ANY struct with this field
    name blocks the decision; only a plain assignment (including array/member targets) is write-only.
    Keep this predicate in sync with the harness; the local regression compares both implementations.
    """
    use = re.compile(r"(?:->|\.)\s*" + field + r"\b((?:\s*(?:\.|->)\s*\w+|\s*\[[^\]]*\])*)\s*(=(?!=))?")
    return any(not match.group(2) for text in sources.values() if field in text
               for line in text.splitlines() for match in use.finditer(line))


def _is_code(line: str) -> bool:
    return bool(line.strip()) and not COMMENT_LINE.match(line)


def code_neighbours(lines: Sequence[str], index: int):
    """(line before, line before that, line after) of lines[index], skipping blank and comment lines; the audit's
    own code_neighbours."""
    before: List[int] = []
    k = index - 1
    while k >= 0 and len(before) < 2:
        if _is_code(lines[k]):
            before.append(k)
        k -= 1
    after = next((k for k in range(index + 1, len(lines)) if _is_code(lines[k])), None)
    return (lines[before[0]] if before else "", lines[before[1]] if len(before) > 1 else "",
            lines[after] if after is not None else "")


def assert_false_form(lines: Sequence[str], index: int) -> Optional[str]:
    """The decision id of `ASSERT(FALSE);` at lines[index]: directly under a case or default label, or the only
    statement of an if or else body; else None (the audit's invented_assertion read the other way round)."""
    if index < 0 or index >= len(lines) or not ASSERT_FALSE.match(lines[index]):
        return None
    previous, before_previous, following = code_neighbours(lines, index)
    if LABEL_LINE.match(previous.rstrip()):
        return "assert-false-case-label"
    opens = IF_BODY_OPEN.match(previous) or (IF_BODY_BRACE.match(previous) and IF_BODY_HEAD.match(before_previous))
    if opens and IF_BODY_CLOSE.match(following):
        return "assert-false-if-body-o0"
    return None


def debug_hook_call(text: str, lines: Sequence[str], index: int) -> bool:
    """True when lines[index] calls, with an argument, an empty one-parameter debug hook its file defines."""
    if index < 0 or index >= len(lines):
        return False
    call = HOOK_CALL.match(lines[index])
    return bool(call) and call.group(1) in set(HOOK_DEFINITION.findall(text))


def classify(site: Mapping[str, object], sources: Mapping[str, str],
             lines_of: Optional[Dict[str, List[str]]] = None,
             fields_of: Optional[Dict[str, Dict[int, str]]] = None,
             readers: Optional[Dict[str, bool]] = None) -> Optional[str]:
    """The id of the decision covering `site`, or None (open)."""
    by_rule = RULE_DECISIONS.get(str(site.get("_rule") or ""))
    if by_rule:
        return by_rule
    category = site.get("category")
    if category != "invented-assert" and category not in HOOK_CATEGORIES + FIELD_CATEGORIES:
        return None
    path = str(site.get("path") or "")
    text = sources.get(path)
    if text is None:
        return None
    cache = lines_of if lines_of is not None else {}
    lines = cache.get(path)
    if lines is None:
        lines = cache[path] = text.split("\n")
    index = int(site.get("line") or 0) - 1
    if category in FIELD_CATEGORIES:
        fields = fields_of if fields_of is not None else {}
        if path not in fields:
            fields[path] = _field_declarations(text) if UNREAD_COMMENT.search(text) else {}
        field = fields[path].get(index)
        if field is None:
            return None
        # Numbered-name findings identify one name, even when other declarations share the line.
        named = re.match(r"([A-Za-z_]\w*):", str(site.get("detail") or ""))
        if named and named.group(1) != field:
            return None
        seen = readers if readers is not None else {}
        if field not in seen:
            seen[field] = field_read_anywhere(sources, field)
        return None if seen[field] else "unread-fields"
    if category == "invented-assert":
        return assert_false_form(lines, index)
    return "debug-value-hook" if debug_hook_call(text, lines, index) else None


def apply(sites: Iterable[dict], sources: Mapping[str, str]) -> None:
    """Give every site its status (and decided ones their decision), and drop the private "_rule"."""
    lines_of: Dict[str, List[str]] = {}
    fields_of: Dict[str, Dict[int, str]] = {}
    readers: Dict[str, bool] = {}
    for site in sites:
        decision = classify(site, sources, lines_of, fields_of, readers)
        site.pop("_rule", None)
        site["status"] = DECIDED if decision else OPEN
        if decision:
            site["decision"] = decision
        else:
            site.pop("decision", None)


def summary(sites: Sequence[Mapping[str, object]], record_root: Optional[Path] = None) -> dict:
    """{"statusCounts", "decisions"} for the document: open and decided counts, and each decision with its
    record and the number of sites it covers. With `record_root` (the folder holding the records' decisions
    folder, local dashboard only) each record also gets its full path and whether that file exists."""
    counts = {OPEN: 0, DECIDED: 0}
    covered: Dict[str, int] = {decision["id"]: 0 for decision in DECISIONS}
    for site in sites:
        status = site.get("status")
        counts[DECIDED if status == DECIDED else OPEN] += 1
        if status == DECIDED:
            covered[str(site.get("decision"))] = covered.get(str(site.get("decision")), 0) + 1
    decisions = []
    for decision in DECISIONS:
        entry = dict(decision)
        entry["sites"] = covered.get(decision["id"], 0)
        if record_root is not None and decision["record"].startswith(DECISION_FOLDER + "/"):
            path = Path(record_root) / decision["record"]
            entry["recordPath"] = str(path)
            entry["recordExists"] = path.is_file()
        decisions.append(entry)
    return {"statusCounts": counts, "decisions": decisions}


METHODOLOGY_NOTE = ("Every site has a status: decided when a recorded user decision covers it, otherwise open. A "
                    "decided site names its decision and its record, and stays listed: the status hides nothing. "
                    "Decided kinds: hex numbers (hex stays hex), sound ids no header names, ASSERT(FALSE) under a "
                    "do-nothing case or as an -O0 if-body, the debug hook for a dropped evaluation, and offset-named "
                    "field declarations with a never-read comment and no reader under the harness fields check.")


__all__ = ["DECISIONS", "RULE_DECISIONS", "apply", "classify", "summary", "METHODOLOGY_NOTE", "OPEN", "DECIDED"]
