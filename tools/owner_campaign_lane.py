"""Candidate-inbox driver for the autonomous owner-campaign lane.

The driver deliberately stays small.  It does not choose hypotheses, create
Codex tasks, or replace the campaign runtime.  Sol supplies sealed natural-C
candidate descriptors to the campaign inbox; this module discovers at most
five of them and delegates measurement/retention to :mod:`tools.owner_campaign`.

The inbox is a transport boundary, not a second authority boundary.  A
descriptor must pass the same self-digest and campaign binding checks used by
the core loader before it is dispatched.  Proposals carry a selector sidecar
bound to the current frontier and compact proof CAS artifacts.  Terminal
descriptors and their build-root candidate sources are compacted after the
core returns.  Inputs for an infrastructure retry remain intact so the next
invocation can retry them.
"""

from __future__ import annotations

import math
import difflib
import json
import os
from pathlib import Path
import re
import shutil
import tempfile
import time
from collections import Counter
from contextlib import nullcontext
from typing import Any, Callable, Mapping, Sequence

from . import owner_campaign
from . import owner_campaign_selector


INBOX_SCHEMA = "owner_campaign_inbox/v1"
LANE_RESULT_SCHEMA = "owner_campaign_lane_result/v1"
SUPERVISOR_RESULT_SCHEMA = "owner_campaign_supervisor_result/v1"
PROPOSAL_RESULT_SCHEMA = "owner_campaign_proposal/v1"
DEFAULT_BATCH_SIZE = 5
DEFAULT_WATCHDOG_SECONDS = 1800.0
# Keep an empty-inbox supervisor alive for the full watchdog window.  A short
# default idle cutoff would turn ordinary Luna proposal latency into a false
# terminal state and force the Sol parent to restart the lane.
DEFAULT_IDLE_TIMEOUT_SECONDS = DEFAULT_WATCHDOG_SECONDS
DEFAULT_POLL_INTERVAL_SECONDS = 1.0
MAX_BACKOFF_SECONDS = 8.0
DEFAULT_INFRA_RETRY_LIMIT = 3
TERMINAL_STATUSES = frozenset(
    {"deduplicated", "discarded", "exact", "improved", "no_gain"}
)


def inbox_path(root: Path, campaign: Mapping[str, Any]) -> Path:
    """Return the compact per-campaign inbox below ``build/owner-campaign``."""

    return (
        owner_campaign._state_root(Path(root))
        / "inbox"
        / owner_campaign._slug(str(campaign["campaign_id"]))
    )


def _input_path(root: Path, raw: Any, label: str, *, exists: bool = True) -> Path:
    """Resolve a caller-supplied path without accepting indirection."""

    if isinstance(raw, Path):
        value = raw
    elif isinstance(raw, (str, os.PathLike)):
        value = Path(os.fspath(raw))
    else:
        raise owner_campaign.CampaignError(f"{label} is invalid")
    if not str(value) or "\x00" in str(value):
        raise owner_campaign.CampaignError(f"{label} is invalid")
    path = Path(os.path.abspath(value if value.is_absolute() else root / value))
    if not _path_inside(root, path):
        raise owner_campaign.CampaignError(f"{label} escapes the campaign root")
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.is_symlink():
            raise owner_campaign.CampaignError(
                f"{label} uses symlink indirection: {current}"
            )
    indirection_checker = getattr(owner_campaign, "_path_has_indirection", None)
    if callable(indirection_checker) and indirection_checker(root, path):
        raise owner_campaign.CampaignError(f"{label} uses path indirection")
    if exists and not path.is_file():
        raise owner_campaign.CampaignError(f"{label} is not a file: {path}")
    return path


def _campaign_source_path(root: Path, campaign: Mapping[str, Any]) -> Path:
    source = campaign.get("_source")
    if source is None:
        source = campaign.get("source_relpath")
    return _input_path(root, source, "campaign source")


def _allowed_candidate_path(
    root: Path, campaign: Mapping[str, Any], path: Path
) -> bool:
    allowed: list[Path] = []
    for raw in (
        *campaign.get("allowed_source_paths", []),
        *campaign.get("allowed_build_paths", []),
    ):
        try:
            allowed.append(_input_path(root, raw, "allowed candidate root", exists=False))
        except owner_campaign.CampaignError:
            # A manifest may name a not-yet-created build root.  It is still a
            # valid containment root; path validation above remains strict.
            continue
    return any(path == item or _path_inside(item, path) for item in allowed)


def _stable_file_bytes(path: Path, label: str) -> bytes:
    """Read one stable file snapshot; reject replacement during the read."""

    try:
        before = path.stat()
        payload = path.read_bytes()
        after = path.stat()
    except OSError as exc:
        raise owner_campaign.CampaignError(f"{label} cannot be read: {path}") from exc
    if (
        before.st_size != after.st_size
        or before.st_mtime_ns != after.st_mtime_ns
        or before.st_ino != after.st_ino
    ):
        raise owner_campaign.CampaignError(f"{label} changed during proposal")
    return payload


def _masked_c_source(text: str) -> str:
    """Blank comments and literals while preserving source offsets/newlines."""

    chars = list(text)
    length = len(chars)
    index = 0
    while index < length:
        if chars[index] == "/" and index + 1 < length and chars[index + 1] == "/":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < length and chars[index] not in "\r\n":
                chars[index] = " "
                index += 1
            continue
        if chars[index] == "/" and index + 1 < length and chars[index + 1] == "*":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < length:
                if chars[index] == "*" and index + 1 < length and chars[index + 1] == "/":
                    chars[index] = chars[index + 1] = " "
                    index += 2
                    break
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
            continue
        if chars[index] in {'"', "'"}:
            quote = chars[index]
            chars[index] = " "
            index += 1
            while index < length:
                token = chars[index]
                escaped = token == "\\"
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
                if escaped and index < length:
                    if chars[index] not in "\r\n":
                        chars[index] = " "
                    index += 1
                elif token == quote:
                    break
            continue
        index += 1
    return "".join(chars)


def _balanced_close(masked: str, opening: int, left: str, right: str) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        token = masked[index]
        if token == left:
            depth += 1
        elif token == right:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                return None
    return None


def _function_span(text: str, function: str, label: str) -> tuple[int, int, bytes]:
    """Find one C function definition and return inclusive source line bounds."""

    masked = _masked_c_source(text)
    pattern = re.compile(r"\b" + re.escape(function) + r"\s*\(")
    matches: list[tuple[int, int]] = []
    for match in pattern.finditer(masked):
        opening = masked.find("(", match.start(), match.end())
        closing = _balanced_close(masked, opening, "(", ")")
        if closing is None:
            continue
        after = closing + 1
        while after < len(masked) and masked[after].isspace():
            after += 1
        if after >= len(masked) or masked[after] != "{":
            continue
        body_close = _balanced_close(masked, after, "{", "}")
        if body_close is None:
            raise owner_campaign.CampaignError(f"{label} has an unterminated body")
        boundary = max(masked.rfind("}", 0, match.start()), masked.rfind(";", 0, match.start()))
        first = match.start() if boundary < 0 else boundary + 1
        while first < match.start() and masked[first].isspace():
            first += 1
        start = text.rfind("\n", 0, first) + 1
        start_line = text.count("\n", 0, start) + 1
        # The closing brace belongs to the function's line even when the line
        # has a trailing newline.  ``splitlines(keepends=True)`` accepts the
        # complete line by this inclusive line number.
        end_line = text.count("\n", 0, body_close) + 1
        matches.append((start_line, end_line))
    if len(matches) != 1:
        if not matches:
            raise owner_campaign.CampaignError(f"{label} definition is not found")
        raise owner_campaign.CampaignError(f"{label} definition is ambiguous")
    start_line, end_line = matches[0]
    lines = text.splitlines(keepends=True)
    span = "".join(lines[start_line - 1:end_line]).encode("utf-8")
    return start_line, end_line, span


def _frontier_for_proposal(
    root: Path, campaign: Mapping[str, Any], function: str
) -> dict[str, Any]:
    """Read and validate the persisted current frontier without measuring."""

    reader = getattr(owner_campaign, "_read_latest_frontier", None)
    if callable(reader) and "_source" in campaign:
        frontier = reader(root, campaign, function)
    else:
        path = (
            owner_campaign._function_root(root, campaign, function)
            / "latest-frontier.json"
        )
        try:
            frontier = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, UnicodeError, json.JSONDecodeError) as exc:
            raise owner_campaign.CampaignError(
                f"current frontier is unavailable for {function}"
            ) from exc
        if not isinstance(frontier, Mapping):
            raise owner_campaign.CampaignError("current frontier is invalid")
        frontier = dict(frontier)
        digest = frontier.pop("frontier_sha256", None)
        if not _is_hex_sha(digest) or _canonical_digest(frontier) != digest:
            raise owner_campaign.CampaignError("current frontier digest is invalid")
        frontier["frontier_sha256"] = digest
    if not isinstance(frontier, Mapping):
        raise owner_campaign.CampaignError("current frontier is unavailable")
    result = dict(frontier)
    if result.get("function") != function or not _is_hex_sha(result.get("frontier_sha256")):
        raise owner_campaign.CampaignError("current frontier function binding is invalid")
    if not _is_hex_sha(result.get("source_sha256")):
        raise owner_campaign.CampaignError("current frontier source binding is invalid")
    return result


def _proposal_descriptor_matches(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier_sha256: str,
    candidate_sha256: str,
) -> bool:
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return False
    for descriptor_path in inbox.rglob("*.json"):
        if descriptor_path.name.startswith("."):
            continue
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is None:
            continue
        descriptor, source_path = sealed
        if (
            descriptor.get("function") == function
            and descriptor.get("base_frontier_sha256") == frontier_sha256
            and descriptor.get("candidate_source", {}).get("sha256") == candidate_sha256
        ):
            return True
    dedupe_reader = getattr(owner_campaign, "_dedupe_records", None)
    dedupe_path = getattr(owner_campaign, "_dedupe_path", None)
    if callable(dedupe_reader) and callable(dedupe_path) and "_source" in campaign:
        ledger = dedupe_path(root, campaign, function)
        if ledger.is_file():
            for record in dedupe_reader(ledger):
                if (
                    record.get("base_frontier_sha256") == frontier_sha256
                    and record.get("candidate_source_sha256") == candidate_sha256
                ):
                    return True
    return False


def _read_json_artifact(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise owner_campaign.CampaignError(f"{label} is not readable JSON") from exc
    if not isinstance(value, Mapping):
        raise owner_campaign.CampaignError(f"{label} is not a JSON object")
    return dict(value)


def _focus_artifact_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier: Mapping[str, Any],
) -> tuple[Path, str, dict[str, Any]]:
    """Load the exact focus CAS object named by the current frontier."""

    digest = frontier.get("focus_evidence_sha256")
    if not _is_hex_sha(digest):
        raise owner_campaign.CampaignError("current frontier focus binding is missing")
    state_root_reader = getattr(owner_campaign, "_state_root", None)
    if not callable(state_root_reader):
        raise owner_campaign.CampaignError("campaign state root is unavailable")
    path = (
        state_root_reader(root) / "proof-cas" / "focus" / digest[:2]
        / f"{digest}.json"
    )
    path = _input_path(root, path, "frontier focus artifact")
    try:
        actual = owner_campaign._digest_file(path)
    except OSError as exc:
        raise owner_campaign.CampaignError("frontier focus artifact cannot be hashed") from exc
    value = _read_json_artifact(path, "frontier focus artifact")
    validator = getattr(owner_campaign, "_validate_focus_evidence", None)
    if callable(validator) and "_source" in campaign and "limits" in campaign:
        try:
            value = dict(validator(value, campaign, function, frontier["source_sha256"]))
        except owner_campaign.CampaignError:
            raise
    else:
        internal = value.get("focus_evidence_sha256")
        if not _is_hex_sha(internal):
            raise owner_campaign.CampaignError("frontier focus self-digest is missing")
        body = dict(value)
        body.pop("focus_evidence_sha256", None)
        if _canonical_digest(body) != internal or internal != digest:
            raise owner_campaign.CampaignError("frontier focus self-digest is invalid")
    internal = value.get("focus_evidence_sha256")
    if internal != digest:
        raise owner_campaign.CampaignError("frontier focus binding drift")
    for key, expected in (
        ("function", function),
        ("source_sha256", frontier.get("source_sha256")),
        ("unit", frontier.get("unit", campaign.get("unit"))),
    ):
        if key in value and value[key] != expected:
            raise owner_campaign.CampaignError(f"frontier focus {key} binding drift")
    return path, actual, value


def _physical_cas_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    frontier: Mapping[str, Any],
    focus_path: Path,
    focus_sha256: str,
    focus: Mapping[str, Any],
) -> tuple[Path, str, dict[str, Any]]:
    """Materialize a compact physical CAS projection from the focus CAS.

    The campaign measurement keeps relocation detail inside its authenticated
    focus object.  The selector contract has a separate physical reference, so
    this projection gives that channel a durable, independently hash-bound
    object without retaining the large raw report.
    """

    state_root_reader = getattr(owner_campaign, "_state_root", None)
    if not callable(state_root_reader):
        raise owner_campaign.CampaignError("campaign state root is unavailable")
    physical_ids = focus.get("physical_difference_ids")
    if not isinstance(physical_ids, list):
        raise owner_campaign.CampaignError("frontier focus physical rows are missing")
    report_receipts = frontier.get("report_receipts")
    physical_receipt = (
        report_receipts.get("physical")
        if isinstance(report_receipts, Mapping) else None
    )
    if not _is_hex_sha(physical_receipt):
        raise owner_campaign.CampaignError("frontier physical receipt is missing")
    body: dict[str, Any] = {
        "schema": "owner_campaign_physical_summary/v1",
        "campaign_id": frontier.get("campaign_id", campaign.get("campaign_id")),
        "owner": frontier.get("owner", campaign.get("owner")),
        "unit": frontier.get("unit", campaign.get("unit")),
        "function": frontier.get("function"),
        "source_sha256": frontier.get("source_sha256"),
        "target_object_sha256": frontier.get("target_object_sha256"),
        "focus_artifact_sha256": focus_sha256,
        "physical_receipt_sha256": physical_receipt,
        "physical_target_identity_sha256": focus.get("physical_target_identity_sha256"),
        "physical_candidate_identity_sha256": focus.get("physical_candidate_identity_sha256"),
        "physical_target_count": focus.get("physical_target_count"),
        "physical_candidate_count": focus.get("physical_candidate_count"),
        "physical_difference_count": focus.get("physical_difference_count"),
        "physical_difference_ids": list(physical_ids),
    }
    required = (
        "campaign_id", "owner", "unit", "function", "source_sha256",
        "target_object_sha256", "physical_target_identity_sha256",
        "physical_candidate_identity_sha256",
    )
    if any(not isinstance(body.get(key), str) or not body[key] for key in required):
        raise owner_campaign.CampaignError("frontier physical identity is incomplete")
    for key in (
        "source_sha256", "target_object_sha256", "physical_target_identity_sha256",
        "physical_candidate_identity_sha256",
    ):
        if not _is_hex_sha(body[key]):
            raise owner_campaign.CampaignError(f"frontier physical {key} is invalid")
    for key in (
        "physical_target_count", "physical_candidate_count", "physical_difference_count",
    ):
        if type(body[key]) is not int or body[key] < 0:
            raise owner_campaign.CampaignError(f"frontier physical {key} is invalid")
    if body["physical_difference_count"] != len(physical_ids):
        raise owner_campaign.CampaignError("frontier physical row count is inconsistent")
    body["physical_summary_sha256"] = _canonical_digest(body)
    digest = body["physical_summary_sha256"]
    path = (
        state_root_reader(root) / "proof-cas" / "physical" / digest[:2]
        / f"{digest}.json"
    )
    path = _input_path(root, path, "physical summary artifact", exists=False)
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file():
        if _read_json_artifact(path, "physical summary artifact") != body:
            raise owner_campaign.CampaignError("physical summary CAS publication drift")
    else:
        ensure_peak = getattr(owner_campaign, "_ensure_state_write_peak", None)
        payload = owner_campaign._canonical(body) + b"\n"
        if callable(ensure_peak) and "limits" in campaign:
            ensure_peak(root, campaign, [(path, payload)])
        owner_campaign._atomic_json(
            path,
            body,
            limit=(campaign.get("limits", {}).get("frontier_bytes") if isinstance(campaign.get("limits"), Mapping) else None),
        )
    try:
        file_sha256 = owner_campaign._digest_file(path)
    except OSError as exc:
        raise owner_campaign.CampaignError("physical summary CAS publication failed") from exc
    return path, file_sha256, body


def _selection_evidence_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_path: Path,
    candidate_sha256: str,
    hypothesis_family: str,
    frontier: Mapping[str, Any],
    expected_terminal: str,
    predicted_rows: Sequence[str] | None,
    predicted_remaining_counts: Mapping[str, int] | None,
) -> tuple[dict[str, Any], Path, str, list[str], dict[str, int]]:
    """Build the selector sidecar and its current frontier evidence bindings."""

    if expected_terminal not in {"exact", "improved"}:
        raise owner_campaign.CampaignError("expected terminal is invalid")
    focus_path, focus_sha, focus = _focus_artifact_for_proposal(
        root, campaign, function, frontier
    )
    physical_path, physical_sha, physical = _physical_cas_for_proposal(
        root, campaign, frontier, focus_path, focus_sha, focus
    )
    try:
        strict_rows, data_rows, physical_rows = owner_campaign_selector._artifact_row_groups(
            focus, physical
        )
        residual = owner_campaign_selector._ordered_union(
            strict_rows, data_rows, physical_rows
        )
    except owner_campaign_selector.SelectionError as exc:
        raise owner_campaign.CampaignError(str(exc)) from exc
    if not residual:
        raise owner_campaign.CampaignError("current frontier has no residual rows")
    if predicted_rows is None:
        if expected_terminal != "exact":
            raise owner_campaign.CampaignError(
                "improved proposals require predicted rows and remaining counts"
            )
        predicted = list(residual)
    else:
        predicted = list(predicted_rows)
    if not predicted or len(predicted) != len(set(predicted)):
        raise owner_campaign.CampaignError("predicted rows are invalid")
    if any(not isinstance(row, str) or not row or len(row) > 512 for row in predicted):
        raise owner_campaign.CampaignError("predicted rows are invalid")
    if not set(predicted) <= set(residual):
        raise owner_campaign.CampaignError("predicted rows are outside current residual")
    current_counts = {
        "strict": len(strict_rows), "data": len(data_rows),
        "physical": len(physical_rows),
    }
    expected_counts = {
        channel: current_counts[channel] - sum(
            row.startswith(f"{channel}:") for row in predicted
        )
        for channel in current_counts
    }
    if expected_terminal == "exact":
        if set(predicted) != set(residual):
            raise owner_campaign.CampaignError(
                "exact proposals must predict every current residual row"
            )
        # Preserve the canonical current-frontier order in the sealed sidecar;
        # callers may supply the complete set in any order.
        predicted = list(residual)
        if predicted_remaining_counts is not None:
            if set(predicted_remaining_counts) != set(current_counts) or any(
                type(predicted_remaining_counts.get(channel)) is not int
                or predicted_remaining_counts[channel] != 0
                for channel in current_counts
            ):
                raise owner_campaign.CampaignError(
                    "exact proposals require zero remaining counts"
                )
        counts = {"strict": 0, "data": 0, "physical": 0}
    else:
        if not isinstance(predicted_remaining_counts, Mapping):
            raise owner_campaign.CampaignError(
                "improved proposals require predicted remaining counts"
            )
        if set(predicted_remaining_counts) != set(current_counts):
            raise owner_campaign.CampaignError(
                "predicted remaining counts must include strict, data, and physical"
            )
        counts = {}
        for channel, current in current_counts.items():
            value = predicted_remaining_counts[channel]
            if type(value) is not int or value < 0 or value > current:
                raise owner_campaign.CampaignError(
                    f"predicted remaining {channel} count is invalid"
                )
            counts[channel] = value
        if counts != expected_counts:
            raise owner_campaign.CampaignError(
                "predicted remaining counts do not match predicted rows"
            )
    protected = focus.get("sibling_digest", focus.get("protected_sibling_digest"))
    if not _is_hex_sha(protected):
        raise owner_campaign.CampaignError("current frontier sibling digest is missing")
    source_relpath = campaign.get("source_relpath", frontier.get("source_relpath"))
    unit = frontier.get("unit", campaign.get("unit"))
    toolchain = frontier.get("toolchain_sha256")
    if not isinstance(source_relpath, str) or not source_relpath:
        raise owner_campaign.CampaignError("current frontier source path is missing")
    if not isinstance(unit, str) or not unit:
        raise owner_campaign.CampaignError("current frontier unit is missing")
    if not _is_hex_sha(toolchain):
        raise owner_campaign.CampaignError("current frontier toolchain binding is missing")
    campaign_id = campaign.get("campaign_id", frontier.get("campaign_id"))
    owner = campaign.get("owner", frontier.get("owner"))
    if not isinstance(campaign_id, str) or not campaign_id:
        raise owner_campaign.CampaignError("current frontier campaign binding is missing")
    if not isinstance(owner, str) or not owner:
        raise owner_campaign.CampaignError("current frontier owner binding is missing")
    if not _is_hex_sha(frontier.get("source_sha256")):
        raise owner_campaign.CampaignError("current frontier source binding is invalid")
    frontier_path = (
        owner_campaign._function_root(root, campaign, function)
        / "latest-frontier.json"
    )
    frontier_path = _input_path(root, frontier_path, "current frontier")
    if _read_json_artifact(frontier_path, "current frontier") != dict(frontier):
        raise owner_campaign.CampaignError("current frontier file drift")
    body: dict[str, Any] = {
        "schema": owner_campaign_selector.SCHEMA,
        "status": "RANKED_SOURCE_CLASS",
        "selection_kind": "RANKED_SOURCE_CLASS",
        "expected_terminal": expected_terminal,
        "campaign_id": campaign_id,
        "owner": owner,
        "unit": unit,
        "function": function,
        "rank": 1,
        "source_class": hypothesis_family,
        "candidate": {
            "path": candidate_path.relative_to(root).as_posix(),
            "sha256": candidate_sha256,
        },
        "frontier": {
            "path": frontier_path.relative_to(root).as_posix(),
            "sha256": frontier["frontier_sha256"],
            "source_sha256": frontier["source_sha256"],
            "function": function,
            "unit": unit,
            "toolchain_sha256": toolchain,
        },
        "base_frontier_sha256": frontier["frontier_sha256"],
        "source_path": source_relpath,
        "source_sha256": frontier["source_sha256"],
        "toolchain_sha256": toolchain,
        "focus_artifact": {
            "path": focus_path.relative_to(root).as_posix(),
            "sha256": focus_sha,
        },
        "physical_artifact": {
            "path": physical_path.relative_to(root).as_posix(),
            "sha256": physical_sha,
        },
        "residual_rows": residual,
        "predicted_rows": predicted,
        "predicted_remaining_counts": counts,
        "protected_sibling_digest": protected,
        "ownership_complete": True,
        "candidate_count": 1,
    }
    return (
        {**body, "evidence_sha256": _canonical_digest(body)},
        focus_path,
        focus_sha,
        residual,
        counts,
    )


def _propose_candidate_locked(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_source: Path,
    hypothesis_family: str,
    *,
    expected_terminal: str,
    predicted_rows: Sequence[str] | None,
    predicted_remaining_counts: Mapping[str, int] | None,
) -> dict[str, Any]:
    source_path = _campaign_source_path(root, campaign)
    source_bytes = _stable_file_bytes(source_path, "campaign source")
    source_sha256 = owner_campaign._digest_bytes(source_bytes)
    frontier = _frontier_for_proposal(root, campaign, function)
    if frontier["source_sha256"] != source_sha256:
        raise owner_campaign.CampaignError("current source has drifted from frontier")
    candidate_bytes = _stable_file_bytes(candidate_source, "candidate source")
    candidate_sha256 = owner_campaign._digest_bytes(candidate_bytes)
    if candidate_sha256 == source_sha256:
        raise owner_campaign.CampaignError("candidate source is byte-identical to frontier")
    try:
        base_text = source_bytes.decode("utf-8")
        candidate_text = candidate_bytes.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise owner_campaign.CampaignError("candidate source is not UTF-8 natural C") from exc
    if "\x00" in candidate_text:
        raise owner_campaign.CampaignError("candidate source contains NUL")
    base_start, base_end, base_span = _function_span(
        base_text, function, "base function"
    )
    candidate_start, candidate_end, candidate_span = _function_span(
        candidate_text, function, "candidate function"
    )
    base_lines = base_text.splitlines(keepends=True)
    candidate_lines = candidate_text.splitlines(keepends=True)
    if (
        base_lines[: base_start - 1] != candidate_lines[: candidate_start - 1]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise owner_campaign.CampaignError("candidate edits escape the named function")
    matcher = difflib.SequenceMatcher(
        a=base_lines, b=candidate_lines, autojunk=False
    )
    changed = False
    added: list[str] = []
    for tag, base_a, base_b, candidate_a, candidate_b in matcher.get_opcodes():
        if tag == "equal":
            continue
        changed = True
        if (
            base_a < base_start - 1
            or base_b > base_end
            or candidate_a < candidate_start - 1
            or candidate_b > candidate_end
        ):
            raise owner_campaign.CampaignError("candidate edits cross the named function")
        added.extend(candidate_lines[candidate_a:candidate_b])
    if not changed:
        raise owner_campaign.CampaignError("candidate has no source change")
    added_text = "".join(added)
    for pattern in campaign.get("forbidden_constructs", []):
        try:
            found = re.search(pattern, added_text)
        except re.error as exc:
            raise owner_campaign.CampaignError(
                f"invalid forbidden construct regex: {pattern}"
            ) from exc
        if found:
            raise owner_campaign.CampaignError(
                f"candidate contains forbidden construct: {pattern}"
            )
    if _proposal_descriptor_matches(
        root, campaign, function, frontier["frontier_sha256"], candidate_sha256
    ):
        raise owner_campaign.CampaignError("duplicate candidate is already queued or measured")

    function_slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", function).strip("_") or "function"
    name = f"{function_slug}-{candidate_sha256[:24]}"
    inbox = inbox_path(root, campaign)
    inbox.mkdir(parents=True, exist_ok=True)
    final_dir = inbox / name
    if final_dir.exists() or final_dir.is_symlink():
        raise owner_campaign.CampaignError("duplicate candidate destination")
    created_at = owner_campaign._now()
    descriptor_body: dict[str, Any] = {
        "schema": owner_campaign.CANDIDATE_SCHEMA,
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "base_frontier_sha256": frontier["frontier_sha256"],
        "candidate_source": {
            "path": (final_dir / "candidate.c").relative_to(root).as_posix(),
            "sha256": candidate_sha256,
        },
        "function_span": {
            "base_start_line": base_start,
            "base_end_line": base_end,
            "candidate_start_line": candidate_start,
            "candidate_end_line": candidate_end,
            "base_sha256": owner_campaign._digest_bytes(base_span),
            "candidate_sha256": owner_campaign._digest_bytes(candidate_span),
        },
        "hypothesis_family": hypothesis_family,
        "natural_c": True,
        "created_at": created_at,
    }
    descriptor = {
        **descriptor_body,
        "candidate_sha256": _canonical_digest(descriptor_body),
    }
    selection, _focus_path, _focus_sha, residual, counts = _selection_evidence_for_proposal(
        root,
        campaign,
        function,
        final_dir / "candidate.c",
        candidate_sha256,
        hypothesis_family,
        frontier,
        expected_terminal,
        predicted_rows,
        predicted_remaining_counts,
    )
    physical_ref = selection.get("physical_artifact")
    if not isinstance(physical_ref, Mapping):
        raise owner_campaign.CampaignError("selection physical artifact binding is missing")
    _physical_path = _input_path(
        root, physical_ref.get("path"), "physical summary artifact"
    )
    _physical_sha = physical_ref.get("sha256")
    if not _is_hex_sha(_physical_sha):
        raise owner_campaign.CampaignError("selection physical artifact hash is invalid")
    stage: Path | None = None
    try:
        stage = Path(tempfile.mkdtemp(prefix=f".{name}.", dir=inbox))
        owner_campaign._atomic_bytes(stage / "candidate.c", candidate_bytes)
        owner_campaign._atomic_json(stage / "candidate.json", descriptor)
        owner_campaign._atomic_json(stage / "candidate.selection.json", selection)
        # Recheck both inputs immediately before the directory rename.  This
        # closes the source/frontier race without making a candidate compile.
        current_source = _stable_file_bytes(source_path, "campaign source")
        if owner_campaign._digest_bytes(current_source) != source_sha256:
            raise owner_campaign.CampaignError("campaign source drifted during proposal")
        current_frontier = _frontier_for_proposal(root, campaign, function)
        if current_frontier["frontier_sha256"] != frontier["frontier_sha256"]:
            raise owner_campaign.CampaignError("frontier advanced during proposal")
        if owner_campaign._digest_file(_focus_path) != _focus_sha:
            raise owner_campaign.CampaignError("frontier focus artifact drifted during proposal")
        if owner_campaign._digest_file(_physical_path) != _physical_sha:
            raise owner_campaign.CampaignError("physical summary artifact drifted during proposal")
        os.replace(stage, final_dir)
        stage = None
    except BaseException:
        if stage is not None:
            try:
                shutil.rmtree(stage)
            except OSError:
                pass
        raise
    descriptor_path = final_dir / "candidate.json"
    return {
        "schema": PROPOSAL_RESULT_SCHEMA,
        "status": "queued",
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "hypothesis_family": hypothesis_family,
        "base_frontier_sha256": frontier["frontier_sha256"],
        "candidate_source_sha256": candidate_sha256,
        "candidate_source": (final_dir / "candidate.c").relative_to(root).as_posix(),
        "candidate_descriptor": descriptor_path.relative_to(root).as_posix(),
        "descriptor_sha256": owner_campaign._digest_file(descriptor_path),
        "candidate_selection": (final_dir / "candidate.selection.json").relative_to(root).as_posix(),
        "selection_sha256": owner_campaign._digest_file(final_dir / "candidate.selection.json"),
        "selection_evidence": (final_dir / "candidate.selection.json").relative_to(root).as_posix(),
        "selection_evidence_sha256": selection["evidence_sha256"],
        "expected_terminal": expected_terminal,
        "predicted_rows": residual if predicted_rows is None else list(predicted_rows),
        "predicted_remaining_counts": counts,
        "created_at": created_at,
        "authority_advanced": False,
    }


def propose_candidate(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_source: Path,
    hypothesis_family: str,
    *,
    expected_terminal: str = "exact",
    predicted_rows: Sequence[str] | None = None,
    predicted_remaining_counts: Mapping[str, int] | None = None,
) -> dict[str, Any]:
    """Seal one worker source into the inbox against the current frontier.

    This is intentionally a source-only front door.  It does not select a
    winner, compile, retain, or consult legacy approval state.
    """

    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function or function not in campaign.get("functions", []):
        raise owner_campaign.CampaignError("function is outside campaign scope")
    if not isinstance(hypothesis_family, str) or not hypothesis_family.strip() or "\x00" in hypothesis_family:
        raise owner_campaign.CampaignError("hypothesis family is invalid")
    hypothesis_family = hypothesis_family.strip()
    if len(hypothesis_family) > 256:
        raise owner_campaign.CampaignError("hypothesis family is too long")
    candidate_path = _input_path(root, candidate_source, "candidate source")
    if candidate_path == _campaign_source_path(root, campaign):
        raise owner_campaign.CampaignError("candidate source must be distinct from campaign source")
    if not _allowed_candidate_path(root, campaign, candidate_path):
        raise owner_campaign.CampaignError("candidate source is outside campaign allowed paths")
    lock_factory = getattr(owner_campaign, "_frontier_lock_chain", None)
    if callable(lock_factory) and "_source" in campaign and "limits" in campaign:
        context = lock_factory(root, campaign, function)
    else:
        context = nullcontext()
    with context:
        return _propose_candidate_locked(
            root, campaign, function, candidate_path, hypothesis_family,
            expected_terminal=expected_terminal,
            predicted_rows=predicted_rows,
            predicted_remaining_counts=predicted_remaining_counts,
        )


submit_candidate = propose_candidate
enqueue_candidate = propose_candidate
queue_candidate = propose_candidate
propose = propose_candidate


def _canonical_digest(value: Any) -> str:
    return owner_campaign._digest_json(value)


def _is_hex_sha(value: Any) -> bool:
    return isinstance(value, str) and owner_campaign.SHA_RE.fullmatch(value) is not None


def _path_inside(root: Path, path: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def _relative_path(root: Path, raw: Any) -> Path | None:
    if not isinstance(raw, str) or not raw or "\x00" in raw:
        return None
    candidate = Path(raw)
    if candidate.is_absolute():
        return None
    path = Path(os.path.abspath(root / candidate))
    if not _path_inside(root, path):
        return None
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.is_symlink():
            return None
    return path


def _under_allowed_build(root: Path, path: Path, campaign: Mapping[str, Any]) -> bool:
    for raw in campaign.get("allowed_build_paths", []):
        allowed = _relative_path(root, raw)
        if allowed is not None and (path == allowed or _path_inside(allowed, path)):
            return True
    return False


def _sealed_descriptor(
    root: Path, campaign: Mapping[str, Any], path: Path
) -> tuple[dict[str, Any], Path] | None:
    """Read only enough to identify a sealed descriptor without consuming it.

    Full candidate validation, including the current frontier binding and
    natural-C source checks, remains in ``owner_campaign.run_candidate``.  A
    malformed or stale inbox file is ignored rather than making an empty lane
    look successful or poisoning a batch of otherwise valid cells.
    """

    try:
        if path.is_symlink():
            return None
        current = root
        for part in path.relative_to(root).parts:
            current = current / part
            if current.is_symlink():
                return None
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError):
        return None
    if not isinstance(value, Mapping):
        return None
    fields = getattr(owner_campaign, "CANDIDATE_FIELDS", frozenset())
    if set(value) != set(fields):
        return None
    body = dict(value)
    digest = body.pop("candidate_sha256", None)
    if not _is_hex_sha(digest) or _canonical_digest(body) != digest:
        return None
    if (
        value.get("schema") != owner_campaign.CANDIDATE_SCHEMA
        or value.get("campaign_id") != campaign.get("campaign_id")
        or value.get("natural_c") is not True
        or not isinstance(value.get("function"), str)
        or value.get("function") not in campaign.get("functions", [])
        or not isinstance(value.get("hypothesis_family"), str)
        or not value.get("hypothesis_family")
        or not _is_hex_sha(value.get("base_frontier_sha256"))
    ):
        return None
    source = value.get("candidate_source")
    if not isinstance(source, Mapping) or set(source) != {"path", "sha256"}:
        return None
    source_path = _relative_path(root, source.get("path"))
    if source_path is None or not _under_allowed_build(root, source_path, campaign):
        return None
    source_sha = source.get("sha256")
    try:
        if (
            not _is_hex_sha(source_sha)
            or not source_path.is_file()
            or owner_campaign._digest_file(source_path) != source_sha
        ):
            return None
    except OSError:
        return None
    return dict(value), source_path


def discover_candidates(
    root: Path, campaign: Mapping[str, Any], *, limit: int = DEFAULT_BATCH_SIZE
) -> list[Path]:
    """Return deterministic, sealed candidate descriptors ready for dispatch."""

    if type(limit) is not int or limit < 1:
        raise ValueError("candidate inbox limit must be a positive integer")
    root = Path(os.path.abspath(root))
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return []
    found: list[tuple[str, str, Path]] = []
    try:
        entries = sorted(
            inbox.rglob("*.json"),
            key=lambda item: item.relative_to(inbox).as_posix(),
        )
    except OSError:
        return []
    for path in entries:
        if path.name.startswith("."):
            continue
        sealed = _sealed_descriptor(root, campaign, path)
        if sealed is None:
            continue
        descriptor, _source = sealed
        created = descriptor.get("created_at")
        # ISO timestamps sort lexically; malformed timestamps are still kept
        # deterministic after well-formed candidates and are fully validated
        # by the core loader before compile.
        created_key = created if isinstance(created, str) else ""
        found.append((created_key, path.relative_to(inbox).as_posix(), path))
    found.sort(key=lambda item: (item[0], item[1]))
    return [item[2] for item in found[:limit]]


def _source_referenced_by_pending(
    root: Path, campaign: Mapping[str, Any], source_path: Path, *, exclude: Path
) -> bool:
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return False
    for descriptor_path in inbox.rglob("*.json"):
        if descriptor_path == exclude:
            continue
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is not None and sealed[1] == source_path:
            return True
    return False


def _compact_terminal_input(
    root: Path, campaign: Mapping[str, Any], descriptor_path: Path
) -> list[str]:
    """Remove a terminal descriptor and an unshared build-root source."""

    removed: list[str] = []
    try:
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        source_path = sealed[1] if sealed is not None else None
        if descriptor_path.exists():
            descriptor_path.unlink()
            removed.append(descriptor_path.relative_to(root).as_posix())
        if (
            source_path is not None
            and source_path.exists()
            and _under_allowed_build(root, source_path, campaign)
            and not _source_referenced_by_pending(
                root, campaign, source_path, exclude=descriptor_path
            )
        ):
            source_path.unlink()
            removed.append(source_path.relative_to(root).as_posix())
    except (OSError, ValueError) as exc:
        # The measurement result is still authoritative.  Surface cleanup
        # trouble to the caller while leaving an infra retry distinguishable.
        removed.append(f"cleanup-error:{descriptor_path}:{exc}")
    return removed


def _result_status(value: Any) -> str:
    return value.get("status", "unknown") if isinstance(value, Mapping) else "unknown"


def run_inbox(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int = DEFAULT_BATCH_SIZE,
) -> dict[str, Any]:
    """Dispatch one bounded inbox batch through the core campaign loop."""

    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    root = Path(os.path.abspath(root))
    descriptors = discover_candidates(root, campaign, limit=max_candidates)
    if not descriptors:
        return {
            "schema": LANE_RESULT_SCHEMA,
            "status": "idle",
            "reason": "candidate_inbox_empty",
            "campaign_id": campaign["campaign_id"],
            "discovered": 0,
            "dispatched": 0,
            "results": [],
            "cleaned": [],
            "preserved_infrastructure": [],
            "authority_advanced": False,
        }

    # A real v2 manifest always uses the selector.  The small descriptor-only
    # fallback is retained for old unit/replay callers that predate the
    # selection sidecar; it is unreachable for a loaded v2 campaign because
    # ``base_commit`` is mandatory there.  This keeps replay compatibility
    # without allowing a production campaign to compile an unranked batch.
    selection: dict[str, Any] | None = None
    selection_required = "base_commit" in campaign or any(
        any(path.is_file() for path in owner_campaign_selector.selection_evidence_paths(descriptor))
        for descriptor in descriptors
    )
    dispatch_descriptors = descriptors
    if selection_required:
        selection = owner_campaign_selector.select_winning_candidate(
            root, campaign, descriptors
        )
        if selection.get("status") != owner_campaign_selector.SELECTED:
            return {
                "schema": LANE_RESULT_SCHEMA,
                "status": "selection_unknown",
                "reason": selection.get("reason", "no deterministic winner"),
                "campaign_id": campaign["campaign_id"],
                "discovered": len(descriptors),
                "dispatched": 0,
                "results": [],
                "cleaned": [],
                "preserved_infrastructure": [
                    path.relative_to(root).as_posix() for path in descriptors
                ],
                "selection": selection,
                "authority_advanced": False,
            }
        selected_path = Path(selection["selected"]["descriptor_path"])
        dispatch_descriptors = [selected_path]

    try:
        results = owner_campaign.run_loop(root, campaign, dispatch_descriptors)
    except owner_campaign.InfrastructureError as exc:
        # A batch-level infrastructure error must not consume its inputs.
        results = [
            {
                "schema": "owner_campaign_result/v1",
                "status": "infra_retry",
                "candidate": str(path),
                "reason": str(exc)[:1000],
                "authority_advanced": False,
            }
            for path in dispatch_descriptors
        ]

    cleaned: list[str] = []
    preserved: list[str] = []
    for index, descriptor_path in enumerate(dispatch_descriptors):
        result = results[index] if index < len(results) else {
            "status": "infra_retry",
            "reason": "core returned fewer results than dispatched",
        }
        status = _result_status(result)
        if status in TERMINAL_STATUSES:
            cleaned.extend(_compact_terminal_input(root, campaign, descriptor_path))
            if selection is not None:
                evidence_path = Path(selection["selected"]["evidence_path"])
                try:
                    if evidence_path.is_file():
                        evidence_path.unlink()
                        cleaned.append(evidence_path.relative_to(root).as_posix())
                except (OSError, ValueError) as exc:
                    cleaned.append(f"cleanup-error:{evidence_path}:{exc}")
        else:
            preserved.append(descriptor_path.relative_to(root).as_posix())

    statuses = [_result_status(item) for item in results]
    if not statuses:
        status = "infra_retry"
    elif all(item == "infra_retry" for item in statuses):
        status = "infra_retry"
    else:
        status = "processed"
    return {
        "schema": LANE_RESULT_SCHEMA,
        "status": status,
        "campaign_id": campaign["campaign_id"],
        "discovered": len(descriptors),
        "dispatched": len(dispatch_descriptors),
        "results": list(results),
        "cleaned": cleaned,
        "preserved_infrastructure": preserved,
        "selection": selection,
        "authority_advanced": False,
    }


def _duration(
    value: float | int | None,
    *,
    default: float,
    label: str,
    minimum: float = 0.0,
) -> float:
    selected = default if value is None else value
    if isinstance(selected, bool):
        raise ValueError(f"{label} must be a finite number")
    try:
        result = float(selected)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{label} must be a finite number") from exc
    if not math.isfinite(result) or result < minimum:
        raise ValueError(f"{label} must be at least {minimum:g} seconds")
    return result


def _terminal_result(
    campaign: Mapping[str, Any],
    *,
    status: str,
    reason: str,
    started: float,
    clock: Callable[[], float],
    batches: int,
    dispatched: int,
    outcomes: Counter[str],
    last_batch_status: str | None,
) -> dict[str, Any]:
    elapsed = max(0.0, clock() - started)
    return {
        "schema": SUPERVISOR_RESULT_SCHEMA,
        "status": status,
        "reason": reason,
        "campaign_id": campaign["campaign_id"],
        "batches": batches,
        "dispatched": dispatched,
        "outcomes": dict(sorted(outcomes.items())),
        "last_batch_status": last_batch_status,
        "elapsed_seconds": round(elapsed, 3),
        "authority_advanced": False,
    }


def _campaign_terminal_state(
    root: Path, campaign: Mapping[str, Any]
) -> tuple[str | None, str | None]:
    """Return a terminal campaign state, if one is observable now.

    Cancellation is checked through the core's signed control record. Other
    core failures are infrastructure-terminal for the supervisor: continuing
    would only rediscover the same invalid state in a tight loop.
    """

    try:
        owner_campaign._check_cancelled(root, campaign)
    except owner_campaign.CampaignError as exc:
        if "campaign is cancelled at the active epoch" in str(exc):
            return "cancelled", str(exc)
        return "infrastructure_terminal", str(exc)[:1000]
    try:
        state = owner_campaign.campaign_status(root, campaign)
    except owner_campaign.CampaignError as exc:
        return "infrastructure_terminal", str(exc)[:1000]
    if (
        isinstance(state, Mapping)
        and state.get("exact_count") == state.get("total")
        and isinstance(state.get("total"), int)
        and state["total"] > 0
    ):
        return "closed", "all campaign functions are exact"
    return None, None


def run_supervisor(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int = DEFAULT_BATCH_SIZE,
    idle_timeout_seconds: float | int | None = DEFAULT_IDLE_TIMEOUT_SECONDS,
    watchdog_seconds: float | int | None = DEFAULT_WATCHDOG_SECONDS,
    poll_interval_seconds: float | int | None = DEFAULT_POLL_INTERVAL_SECONDS,
    infra_retry_limit: int = DEFAULT_INFRA_RETRY_LIMIT,
    clock: Callable[[], float] | None = None,
    sleeper: Callable[[float], None] | None = None,
) -> dict[str, Any]:
    """Keep a Sol-owned campaign live until a bounded terminal state.

    The supervisor resolves the lane's sealed evidence through the deterministic
    winning-cell selector, dispatches at most its single rank-1 candidate, and
    waits with bounded exponential backoff when no candidate is supportable or
    an infrastructure retry is pending. Portfolio scope and cross-owner
    priorities remain outside the lane. ``--once`` uses :func:`run_inbox`
    instead for deterministic snapshots and tests.
    """

    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    if type(infra_retry_limit) is not int or infra_retry_limit < 1:
        raise ValueError("infra_retry_limit must be a positive integer")
    idle_timeout = _duration(
        idle_timeout_seconds,
        default=DEFAULT_IDLE_TIMEOUT_SECONDS,
        label="idle_timeout_seconds",
    )
    watchdog = _duration(
        watchdog_seconds,
        default=DEFAULT_WATCHDOG_SECONDS,
        label="watchdog_seconds",
    )
    poll_interval = _duration(
        poll_interval_seconds,
        default=DEFAULT_POLL_INTERVAL_SECONDS,
        label="poll_interval_seconds",
        minimum=0.01,
    )
    clock = clock or time.monotonic
    sleeper = sleeper or time.sleep
    root = Path(os.path.abspath(root))
    started = clock()
    idle_started = started
    delay = poll_interval
    infra_retries = 0
    batches = 0
    dispatched = 0
    outcomes: Counter[str] = Counter()
    last_batch_status: str | None = None

    while True:
        now = clock()
        if now - started >= watchdog:
            return _terminal_result(
                campaign,
                status="watchdog_timeout",
                reason="supervisor watchdog expired",
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )

        terminal, reason = _campaign_terminal_state(root, campaign)
        if terminal is not None:
            return _terminal_result(
                campaign,
                status=terminal,
                reason=reason or terminal,
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )

        descriptors = discover_candidates(root, campaign, limit=max_candidates)
        if descriptors:
            try:
                batch = run_inbox(
                    root, campaign, max_candidates=max_candidates
                )
            except owner_campaign.CampaignError as exc:
                return _terminal_result(
                    campaign,
                    status="infrastructure_terminal",
                    reason=str(exc)[:1000],
                    started=started,
                    clock=clock,
                    batches=batches,
                    dispatched=dispatched,
                    outcomes=outcomes,
                    last_batch_status=last_batch_status,
                )
            batches += 1
            dispatched += int(batch.get("dispatched", len(descriptors)))
            batch_results = batch.get("results", [])
            if not isinstance(batch_results, list):
                batch_results = []
            batch_statuses = [_result_status(item) for item in batch_results]
            outcomes.update(batch_statuses)
            last_batch_status = str(batch.get("status", "unknown"))
            if last_batch_status == "selection_unknown":
                return _terminal_result(
                    campaign,
                    status="pivot_required",
                    reason=str(batch.get("reason", "selection returned UNKNOWN"))[:1000],
                    started=started,
                    clock=clock,
                    batches=batches,
                    dispatched=dispatched,
                    outcomes=outcomes,
                    last_batch_status=last_batch_status,
                )
            infra_only = bool(batch_statuses) and all(
                status in {"infra_retry", "stale_rebase", "stale"}
                for status in batch_statuses
            )
            if last_batch_status == "infra_retry" or infra_only:
                infra_retries += 1
                if infra_retries >= infra_retry_limit:
                    return _terminal_result(
                        campaign,
                        status="infrastructure_terminal",
                        reason="infrastructure retry limit reached",
                        started=started,
                        clock=clock,
                        batches=batches,
                        dispatched=dispatched,
                        outcomes=outcomes,
                        last_batch_status=last_batch_status,
                    )
                # Retryable inputs remain in the inbox. Sleep before looking
                # again so stale/retry descriptors cannot spin the supervisor.
                now = clock()
                remaining = max(0.0, watchdog - (now - started))
                sleeper(min(delay, remaining))
                delay = min(MAX_BACKOFF_SECONDS, max(poll_interval, delay * 2))
            else:
                infra_retries = 0
                idle_started = clock()
                delay = poll_interval
            continue

        now = clock()
        if now - idle_started >= idle_timeout:
            return _terminal_result(
                campaign,
                status="idle_timeout",
                reason="candidate inbox stayed empty until idle timeout",
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )
        remaining_idle = max(0.0, idle_timeout - (now - idle_started))
        remaining_watchdog = max(0.0, watchdog - (now - started))
        sleeper(min(delay, remaining_idle, remaining_watchdog))
        delay = min(MAX_BACKOFF_SECONDS, max(poll_interval, delay * 2))


def run_owner_campaign_inbox(
    root: Path, campaign: Mapping[str, Any], *, max_candidates: int = DEFAULT_BATCH_SIZE
) -> dict[str, Any]:
    """Compatibility name for callers that treat the inbox as a lane."""

    return run_inbox(root, campaign, max_candidates=max_candidates)


__all__ = [
    "DEFAULT_BATCH_SIZE",
    "DEFAULT_IDLE_TIMEOUT_SECONDS",
    "DEFAULT_POLL_INTERVAL_SECONDS",
    "DEFAULT_WATCHDOG_SECONDS",
    "INBOX_SCHEMA",
    "LANE_RESULT_SCHEMA",
    "PROPOSAL_RESULT_SCHEMA",
    "SUPERVISOR_RESULT_SCHEMA",
    "TERMINAL_STATUSES",
    "discover_candidates",
    "enqueue_candidate",
    "inbox_path",
    "propose",
    "propose_candidate",
    "queue_candidate",
    "run_inbox",
    "run_owner_campaign_inbox",
    "run_supervisor",
    "submit_candidate",
]
