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
             lines_of: Optional[Dict[str, List[str]]] = None) -> Optional[str]:
    """The id of the decision covering `site`, or None (open)."""
    by_rule = RULE_DECISIONS.get(str(site.get("_rule") or ""))
    if by_rule:
        return by_rule
    category = site.get("category")
    if category != "invented-assert" and category not in HOOK_CATEGORIES:
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
    if category == "invented-assert":
        return assert_false_form(lines, index)
    return "debug-value-hook" if debug_hook_call(text, lines, index) else None


def apply(sites: Iterable[dict], sources: Mapping[str, str]) -> None:
    """Give every site its status (and decided ones their decision), and drop the private "_rule"."""
    lines_of: Dict[str, List[str]] = {}
    for site in sites:
        decision = classify(site, sources, lines_of)
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
                    "do-nothing case or as an -O0 if-body, and the debug hook for a dropped evaluation.")


__all__ = ["DECISIONS", "RULE_DECISIONS", "apply", "classify", "summary", "METHODOLOGY_NOTE", "OPEN", "DECIDED"]
