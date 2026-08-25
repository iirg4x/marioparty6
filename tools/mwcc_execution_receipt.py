#!/usr/bin/env python3
"""Append-only authenticated execution receipts for MWCC diagnostics.

The journal is deliberately independent from capture production.  It records
only already-materialized, hash-bound artifacts and refuses authority-bearing
or unmeasured runs.  Every JSONL row hashes its canonical content and the
previous row, while appends use the operating system's append primitive under
an exclusive advisory lock.

This remains diagnostic-only.  Python on Windows does not expose a portable
handle-relative ``openat``/ACL-pinning primitive, so a hostile process with
permission to rewrite path components can race the reparse/identity checks and
the final open.  The validator rejects every observable reparse point, hard
link, alias, and descriptor drift and rechecks under the journal lock, but it
does not claim a production-security boundary against that OS-level race.
"""

from __future__ import annotations

import argparse
import contextlib
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import re
import stat
import sys
import tempfile
import time
from typing import Any, Iterator, Mapping, Sequence


REQUEST_SCHEMA = "mwcc_execution_receipt_request/v1"
RECEIPT_SCHEMA = "mwcc_execution_receipt/v1"
VALIDATION_SCHEMA = "mwcc_execution_receipt_validation/v1"
JOURNAL_VALIDATION_SCHEMA = "mwcc_execution_receipt_journal_validation/v1"
MEASUREMENT_SCHEMA = "mwcc_active_seconds_measurement/v1"
MEASUREMENT_EVENT_SCHEMA = "mwcc_active_seconds_measurement_event/v1"
MEASUREMENT_GENERATOR = "tools/mwcc_execution_receipt.py measure-stop"
MAX_MEASUREMENT_SECONDS = 24 * 60 * 60

_SHA256 = re.compile(r"[0-9a-f]{64}\Z")
_IDENTIFIER = re.compile(r"[A-Za-z0-9][A-Za-z0-9._:-]{0,127}\Z")
_RAW_POINTER = re.compile(r"0[xX][0-9a-fA-F]+")
_PLACEHOLDER = re.compile(
    r"(?:\{\{[^{}]+\}\}|\$\{[^{}]+\}|<\s*(?:placeholder|todo|tbd|replace[_ -]?me)[^>]*>"
    r"|\b(?:placeholder|replace[_ -]?me|todo|tbd)\b)",
    re.IGNORECASE,
)
_DESCRIPTOR_KEYS = frozenset({"path", "size_bytes", "sha256"})
_JOIN_STATUSES = frozenset({"MATCHED_AUTHENTICATED", "UNKNOWN"})


class Rejected(ValueError):
    """A request or journal failed its authenticated closed contract."""


def _canonical(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
        + "\n"
    ).encode("utf-8")


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise Rejected(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _decode_json(payload: bytes, label: str) -> dict[str, Any]:
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise Rejected(f"{label} is not UTF-8: {exc}") from exc
    try:
        value = json.loads(text, object_pairs_hook=_reject_duplicates)
    except Rejected:
        raise
    except json.JSONDecodeError as exc:
        raise Rejected(f"invalid {label}:{exc.lineno}:{exc.colno}: {exc.msg}") from exc
    except ValueError as exc:
        raise Rejected(f"invalid numeric value in {label}: {exc}") from exc
    if not isinstance(value, dict):
        raise Rejected(f"{label} must contain one JSON object")
    return value


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise Rejected(f"{label} must be an object")
    unknown = sorted(set(value) - allowed)
    if unknown:
        raise Rejected(f"{label} has unsupported keys: {', '.join(unknown)}")
    missing = sorted(required - set(value))
    if missing:
        raise Rejected(f"{label} is missing required keys: {', '.join(missing)}")
    return dict(value)


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise Rejected(f"{label} must be a non-empty NUL-free string")
    if _PLACEHOLDER.search(value):
        raise Rejected(f"{label} contains placeholder text")
    if _RAW_POINTER.search(value):
        raise Rejected(f"{label} contains raw pointer/address text")
    return value


def _path_text(value: Any, label: str) -> str:
    """Validate an authenticated descriptor path without scanning its components.

    Absolute paths are subsequently checked for indirection, file type, size,
    and digest.  A hexadecimal-looking directory name is therefore path data,
    not serialized runtime pointer evidence.  Relative bare address spellings
    remain fail-closed.
    """

    if not isinstance(value, str) or not value or "\x00" in value:
        raise Rejected(f"{label} must be a non-empty NUL-free string")
    if _PLACEHOLDER.search(value):
        raise Rejected(f"{label} contains placeholder text")
    if _RAW_POINTER.search(value) and not Path(value).expanduser().is_absolute():
        raise Rejected(f"{label} contains raw pointer/address text")
    return value


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label)
    if not _IDENTIFIER.fullmatch(result):
        raise Rejected(f"{label} contains unsupported identifier characters")
    return result


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or not _SHA256.fullmatch(value):
        raise Rejected(f"{label} must be a lowercase 64-hex SHA-256")
    return value


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise Rejected(f"{label} must be an integer >= {minimum}")
    return value


def _positive_seconds(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise Rejected(f"{label} must be a measured number")
    try:
        result = float(value)
    except (OverflowError, ValueError) as exc:
        raise Rejected(f"{label} must be a finite measured number") from exc
    if not math.isfinite(result) or result <= 0:
        raise Rejected(f"{label} must be finite and > 0")
    return result


def _assert_regular_private(path: Path, label: str) -> os.stat_result:
    _assert_no_indirection(path, label)
    try:
        info = path.lstat()
    except FileNotFoundError as exc:
        raise Rejected(f"{label} does not exist: {path}") from exc
    except OSError as exc:
        raise Rejected(f"cannot inspect {label} {path}: {exc}") from exc
    if stat.S_ISLNK(info.st_mode) or not stat.S_ISREG(info.st_mode):
        raise Rejected(f"{label} must be a non-symlink regular file: {path}")
    if info.st_nlink != 1:
        raise Rejected(f"{label} must have exactly one hard link: {path}")
    return info


def _assert_no_indirection(path: Path, label: str, *, allow_missing: bool = False) -> None:
    current = path
    leaf = True
    while True:
        try:
            info = current.lstat()
        except FileNotFoundError:
            if leaf and not allow_missing:
                raise Rejected(f"{label} does not exist: {path}")
        except OSError as exc:
            raise Rejected(f"cannot inspect {label} {current}: {exc}") from exc
        else:
            reparse_flag = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
            is_reparse = bool(getattr(info, "st_file_attributes", 0) & reparse_flag)
            if stat.S_ISLNK(info.st_mode) or is_reparse:
                raise Rejected(f"{label} uses a symlink/reparse path: {current}")
        parent = current.parent
        if parent == current:
            break
        current = parent
        leaf = False


def _absolute(path: Path) -> Path:
    return Path(os.path.abspath(os.fspath(path.expanduser())))


def _resolve_path(value: Any, base: Path, label: str) -> Path:
    raw = _path_text(value, label)
    path = Path(raw).expanduser()
    if not path.is_absolute():
        path = base / path
    return _absolute(path)


def descriptor(path: Path) -> dict[str, Any]:
    path = _absolute(path)
    info = _assert_regular_private(path, "descriptor file")
    return {
        "path": os.fspath(path),
        "size_bytes": info.st_size,
        "sha256": _sha256_file(path),
    }


def _authenticated_descriptor(value: Any, base: Path, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed=set(_DESCRIPTOR_KEYS),
        required=set(_DESCRIPTOR_KEYS),
        label=label,
    )
    path = _resolve_path(item["path"], base, f"{label}.path")
    claimed_size = _integer(item["size_bytes"], f"{label}.size_bytes")
    claimed_sha = _sha(item["sha256"], f"{label}.sha256")
    actual = descriptor(path)
    if actual["size_bytes"] != claimed_size or actual["sha256"] != claimed_sha:
        raise Rejected(
            f"{label} descriptor drift: expected {claimed_sha}/{claimed_size}, "
            f"found {actual['sha256']}/{actual['size_bytes']}"
        )
    return actual


def _same_file(left: Path, right: Path) -> bool:
    try:
        return os.path.samefile(left, right)
    except FileNotFoundError:
        return False
    except OSError as exc:
        raise Rejected(f"cannot compare file identity for {left} and {right}: {exc}") from exc


def _reject_aliases(items: Sequence[tuple[str, Path]]) -> None:
    for left_index, (left_label, left_path) in enumerate(items):
        left_name = os.path.normcase(os.fspath(_absolute(left_path)))
        for right_label, right_path in items[left_index + 1 :]:
            right_name = os.path.normcase(os.fspath(_absolute(right_path)))
            if left_name == right_name or _same_file(left_path, right_path):
                raise Rejected(
                    f"{left_label} aliases {right_label}: {left_path} / {right_path}"
                )


def _utc(value: Any, label: str) -> datetime:
    text = _text(value, label)
    if not text.endswith("Z"):
        raise Rejected(f"{label} must be an RFC3339 UTC timestamp ending in Z")
    try:
        result = datetime.fromisoformat(text[:-1] + "+00:00")
    except ValueError as exc:
        raise Rejected(f"{label} is not a valid RFC3339 UTC timestamp") from exc
    if result.tzinfo != timezone.utc:
        raise Rejected(f"{label} must use UTC")
    return result


def _validate_measurement_receipt(
    descriptor_value: Mapping[str, Any], expected_active_seconds: float
) -> dict[str, Any]:
    path = Path(str(descriptor_value["path"]))
    _assert_regular_private(path, "measurement receipt")
    value = _decode_json(path.read_bytes(), "measurement receipt")
    item = _closed(
        value,
        allowed={
            "schema",
            "generator",
            "clock",
            "intervals",
            "active_seconds",
            "measurement_complete",
            "diagnostic_only",
            "authority_advanced",
        },
        required={
            "schema",
            "generator",
            "clock",
            "intervals",
            "active_seconds",
            "measurement_complete",
            "diagnostic_only",
            "authority_advanced",
        },
        label="measurement receipt",
    )
    if item["schema"] != MEASUREMENT_SCHEMA:
        raise Rejected(f"measurement receipt.schema must be {MEASUREMENT_SCHEMA}")
    if item["generator"] != MEASUREMENT_GENERATOR:
        raise Rejected(f"measurement receipt.generator must be {MEASUREMENT_GENERATOR}")
    clock = _text(item["clock"], "measurement receipt.clock")
    if clock != "perf_counter_ns+monotonic_ns":
        raise Rejected(
            "measurement receipt.clock must be perf_counter_ns+monotonic_ns"
        )
    if item["measurement_complete"] is not True:
        raise Rejected("measurement receipt.measurement_complete must be true")
    if item["diagnostic_only"] is not True or item["authority_advanced"] is not False:
        raise Rejected(
            "measurement receipt must be diagnostic_only=true and authority_advanced=false"
        )
    measured = _positive_seconds(
        item["active_seconds"], "measurement receipt.active_seconds"
    )
    if measured != expected_active_seconds:
        raise Rejected(
            "measurement receipt.active_seconds does not equal request.active_seconds"
        )
    if (
        not isinstance(item["intervals"], list)
        or not 1 <= len(item["intervals"]) <= 1024
    ):
        raise Rejected("measurement receipt.intervals must contain 1-1024 rows")
    intervals: list[dict[str, Any]] = []
    total_perf_ns = 0
    previous_perf_end: int | None = None
    previous_mono_end: int | None = None
    previous_utc_end: datetime | None = None
    for index, raw in enumerate(item["intervals"]):
        interval = _closed(
            raw,
            allowed={
                "started_utc",
                "stopped_utc",
                "start_perf_counter_ns",
                "end_perf_counter_ns",
                "start_monotonic_ns",
                "end_monotonic_ns",
            },
            required={
                "started_utc",
                "stopped_utc",
                "start_perf_counter_ns",
                "end_perf_counter_ns",
                "start_monotonic_ns",
                "end_monotonic_ns",
            },
            label=f"measurement receipt.intervals[{index}]",
        )
        started_utc = _utc(
            interval["started_utc"],
            f"measurement receipt.intervals[{index}].started_utc",
        )
        stopped_utc = _utc(
            interval["stopped_utc"],
            f"measurement receipt.intervals[{index}].stopped_utc",
        )
        start_perf = _integer(
            interval["start_perf_counter_ns"],
            f"measurement receipt.intervals[{index}].start_perf_counter_ns",
            minimum=1,
        )
        end_perf = _integer(
            interval["end_perf_counter_ns"],
            f"measurement receipt.intervals[{index}].end_perf_counter_ns",
            minimum=1,
        )
        start_mono = _integer(
            interval["start_monotonic_ns"],
            f"measurement receipt.intervals[{index}].start_monotonic_ns",
            minimum=1,
        )
        end_mono = _integer(
            interval["end_monotonic_ns"],
            f"measurement receipt.intervals[{index}].end_monotonic_ns",
            minimum=1,
        )
        if end_perf <= start_perf or end_mono <= start_mono or stopped_utc <= started_utc:
            raise Rejected(
                f"measurement receipt.intervals[{index}] must have positive boundaries"
            )
        if (
            previous_perf_end is not None
            and (
                start_perf < previous_perf_end
                or start_mono < previous_mono_end  # type: ignore[operator]
                or started_utc < previous_utc_end  # type: ignore[operator]
            )
        ):
            raise Rejected("measurement receipt intervals overlap or are out of order")
        perf_delta = end_perf - start_perf
        mono_delta = end_mono - start_mono
        if perf_delta > MAX_MEASUREMENT_SECONDS * 1_000_000_000:
            raise Rejected(f"measurement receipt.intervals[{index}] is stale")
        if abs(perf_delta - mono_delta) > 50_000_000:
            raise Rejected(
                f"measurement receipt.intervals[{index}] clocks disagree by more than 50ms"
            )
        total_perf_ns += perf_delta
        previous_perf_end = end_perf
        previous_mono_end = end_mono
        previous_utc_end = stopped_utc
        intervals.append(
            {
                "started_utc": interval["started_utc"],
                "stopped_utc": interval["stopped_utc"],
                "start_perf_counter_ns": start_perf,
                "end_perf_counter_ns": end_perf,
                "start_monotonic_ns": start_mono,
                "end_monotonic_ns": end_mono,
            }
        )
    computed = total_perf_ns / 1_000_000_000
    if measured != computed:
        raise Rejected(
            "measurement receipt interval durations do not exactly equal active_seconds"
        )
    return {
        "schema": MEASUREMENT_SCHEMA,
        "generator": MEASUREMENT_GENERATOR,
        "clock": clock,
        "intervals": intervals,
        "active_seconds": measured,
        "measurement_complete": True,
        "diagnostic_only": True,
        "authority_advanced": False,
    }


def _source_span_join(value: Any, label: str) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={"status", "reason"},
        required={"status", "reason"},
        label=label,
    )
    status = _text(item["status"], f"{label}.status")
    if status not in _JOIN_STATUSES:
        raise Rejected(f"{label}.status must be MATCHED_AUTHENTICATED or UNKNOWN")
    reason = item["reason"]
    if status == "UNKNOWN":
        reason = _text(reason, f"{label}.reason")
    elif reason is not None:
        raise Rejected(f"{label}.reason must be null when status is MATCHED_AUTHENTICATED")
    return {"status": status, "reason": reason}


def _normalise_request(value: Any, base: Path) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={
            "schema",
            "task_id",
            "live_plan",
            "children",
            "active_seconds",
            "active_seconds_measured",
            "measurement_receipt",
            "policy",
        },
        required={
            "schema",
            "task_id",
            "live_plan",
            "children",
            "active_seconds",
            "active_seconds_measured",
            "measurement_receipt",
            "policy",
        },
        label="request",
    )
    if item["schema"] != REQUEST_SCHEMA:
        raise Rejected(f"request.schema must be {REQUEST_SCHEMA}")
    task_id = _identifier(item["task_id"], "request.task_id")
    live_plan = _authenticated_descriptor(item["live_plan"], base, "request.live_plan")
    if not isinstance(item["children"], list) or not 1 <= len(item["children"]) <= 8:
        raise Rejected("request.children must contain 1-8 rows")
    children: list[dict[str, Any]] = []
    labels: set[str] = set()
    for index, raw in enumerate(item["children"]):
        child = _closed(
            raw,
            allowed={"label", "artifact", "source_span_join"},
            required={"label", "artifact", "source_span_join"},
            label=f"request.children[{index}]",
        )
        child_label = _identifier(child["label"], f"request.children[{index}].label")
        if child_label in labels:
            raise Rejected(f"duplicate child label: {child_label}")
        labels.add(child_label)
        children.append(
            {
                "label": child_label,
                "artifact": _authenticated_descriptor(
                    child["artifact"], base, f"request.children[{index}].artifact"
                ),
                "source_span_join": _source_span_join(
                    child["source_span_join"],
                    f"request.children[{index}].source_span_join",
                ),
            }
        )
    active_seconds = _positive_seconds(item["active_seconds"], "request.active_seconds")
    if item["active_seconds_measured"] is not True:
        raise Rejected("request.active_seconds_measured must be true")
    measurement = _authenticated_descriptor(
        item["measurement_receipt"], base, "request.measurement_receipt"
    )
    _validate_measurement_receipt(measurement, active_seconds)
    bound_files: list[tuple[str, Path]] = [
        ("request.live_plan", Path(str(live_plan["path"]))),
        ("request.measurement_receipt", Path(str(measurement["path"]))),
    ]
    bound_files.extend(
        (
            f"request.children[{index}].artifact",
            Path(str(child["artifact"]["path"])),
        )
        for index, child in enumerate(children)
    )
    _reject_aliases(bound_files)
    policy = _closed(
        item["policy"],
        allowed={"diagnostic_only", "authority_advanced"},
        required={"diagnostic_only", "authority_advanced"},
        label="request.policy",
    )
    if policy["diagnostic_only"] is not True or policy["authority_advanced"] is not False:
        raise Rejected(
            "request.policy must be diagnostic_only=true and authority_advanced=false"
        )
    return {
        "schema": REQUEST_SCHEMA,
        "task_id": task_id,
        "live_plan": live_plan,
        "children": children,
        "active_seconds": active_seconds,
        "active_seconds_measured": True,
        "measurement_receipt": measurement,
        "policy": {"diagnostic_only": True, "authority_advanced": False},
    }


def validate_request(path: Path) -> dict[str, Any]:
    path = _absolute(path)
    _assert_regular_private(path, "request")
    value = _decode_json(path.read_bytes(), "request")
    request = _normalise_request(value, path.parent)
    aliases: list[tuple[str, Path]] = [("request", path)]
    aliases.append(("request.live_plan", Path(str(request["live_plan"]["path"]))))
    aliases.append(
        (
            "request.measurement_receipt",
            Path(str(request["measurement_receipt"]["path"])),
        )
    )
    aliases.extend(
        (
            f"request.children[{index}].artifact",
            Path(str(child["artifact"]["path"])),
        )
        for index, child in enumerate(request["children"])
    )
    _reject_aliases(aliases)
    return request


def _receipt_hash(value: Mapping[str, Any]) -> str:
    return _sha256_bytes(_canonical(value))


def _validate_receipt_row(
    value: Any, *, index: int, previous: str | None
) -> tuple[dict[str, Any], str]:
    item = _closed(
        value,
        allowed={
            "schema",
            "sequence",
            "previous_receipt_sha256",
            "request",
            "request_payload_sha256",
            "task_id",
            "live_plan",
            "children",
            "active_seconds",
            "active_seconds_measured",
            "measurement_receipt",
            "policy",
            "receipt_sha256",
        },
        required={
            "schema",
            "sequence",
            "previous_receipt_sha256",
            "request",
            "request_payload_sha256",
            "task_id",
            "live_plan",
            "children",
            "active_seconds",
            "active_seconds_measured",
            "measurement_receipt",
            "policy",
            "receipt_sha256",
        },
        label=f"journal[{index}]",
    )
    if item["schema"] != RECEIPT_SCHEMA:
        raise Rejected(f"journal[{index}].schema must be {RECEIPT_SCHEMA}")
    if _integer(item["sequence"], f"journal[{index}].sequence") != index:
        raise Rejected(f"journal[{index}] sequence is not contiguous")
    actual_previous = item["previous_receipt_sha256"]
    if previous is None:
        if actual_previous is not None:
            raise Rejected("journal[0].previous_receipt_sha256 must be null")
    elif _sha(actual_previous, f"journal[{index}].previous_receipt_sha256") != previous:
        raise Rejected(f"journal[{index}] previous hash does not match")
    # A journal validation is an evidence validation, not merely a structural
    # parse: every descriptor must still identify the exact bytes on disk.
    request = _authenticated_descriptor(
        item["request"], Path.cwd(), f"journal[{index}].request"
    )
    payload_sha = _sha(
        item["request_payload_sha256"],
        f"journal[{index}].request_payload_sha256",
    )
    if payload_sha != request["sha256"]:
        raise Rejected(f"journal[{index}] request payload hash does not match descriptor")
    # The remaining payload was produced by request validation; re-run the same
    # closed scalar checks without consulting mutable artifact paths.
    _identifier(item["task_id"], f"journal[{index}].task_id")
    for descriptor_label in ("live_plan", "measurement_receipt"):
        _authenticated_descriptor(
            item[descriptor_label],
            Path.cwd(),
            f"journal[{index}].{descriptor_label}",
        )
    if not isinstance(item["children"], list) or not 1 <= len(item["children"]) <= 8:
        raise Rejected(f"journal[{index}].children must contain 1-8 rows")
    seen: set[str] = set()
    for child_index, child_raw in enumerate(item["children"]):
        child = _closed(
            child_raw,
            allowed={"label", "artifact", "source_span_join"},
            required={"label", "artifact", "source_span_join"},
            label=f"journal[{index}].children[{child_index}]",
        )
        label = _identifier(child["label"], f"journal[{index}].children[{child_index}].label")
        if label in seen:
            raise Rejected(f"journal[{index}] has duplicate child label: {label}")
        seen.add(label)
        _authenticated_descriptor(
            child["artifact"],
            Path.cwd(),
            f"journal[{index}].children[{child_index}].artifact",
        )
        _source_span_join(
            child["source_span_join"],
            f"journal[{index}].children[{child_index}].source_span_join",
        )
    _positive_seconds(item["active_seconds"], f"journal[{index}].active_seconds")
    if item["active_seconds_measured"] is not True:
        raise Rejected(f"journal[{index}].active_seconds_measured must be true")
    if item["policy"] != {"diagnostic_only": True, "authority_advanced": False}:
        raise Rejected(f"journal[{index}] has authority-bearing policy")
    bound_request = validate_request(Path(str(request["path"])))
    for field in (
        "task_id",
        "live_plan",
        "children",
        "active_seconds",
        "active_seconds_measured",
        "measurement_receipt",
        "policy",
    ):
        if item[field] != bound_request[field]:
            raise Rejected(
                f"journal[{index}].{field} does not match authenticated request"
            )
    claimed = _sha(item["receipt_sha256"], f"journal[{index}].receipt_sha256")
    unsigned = dict(item)
    del unsigned["receipt_sha256"]
    computed = _receipt_hash(unsigned)
    if claimed != computed:
        raise Rejected(f"journal[{index}] receipt hash mismatch")
    return item, claimed


def _parse_journal(payload: bytes, label: str) -> list[dict[str, Any]]:
    if not payload:
        return []
    if not payload.endswith(b"\n"):
        raise Rejected(f"{label} has a partial final line")
    rows: list[dict[str, Any]] = []
    previous: str | None = None
    for index, line in enumerate(payload.splitlines()):
        if not line:
            raise Rejected(f"{label}[{index}] is empty")
        value = _decode_json(line, f"{label}[{index}]")
        row, previous = _validate_receipt_row(value, index=index, previous=previous)
        if line + b"\n" != _canonical(row):
            raise Rejected(f"{label}[{index}] is not canonically encoded")
        rows.append(row)
    return rows


def validate_journal(path: Path) -> dict[str, Any]:
    path = _absolute(path)
    _assert_regular_private(path, "journal")
    rows = _parse_journal(path.read_bytes(), "journal")
    return {
        "schema": JOURNAL_VALIDATION_SCHEMA,
        "status": "VALID",
        "entry_count": len(rows),
        "head_receipt_sha256": rows[-1]["receipt_sha256"] if rows else None,
        "journal": descriptor(path),
    }


@contextlib.contextmanager
def _locked_append_fd(path: Path) -> Iterator[int]:
    _assert_no_indirection(path, "journal", allow_missing=True)
    path.parent.mkdir(parents=True, exist_ok=True)
    flags = os.O_RDWR | os.O_CREAT | os.O_APPEND
    if hasattr(os, "O_BINARY"):
        flags |= os.O_BINARY
    fd = os.open(path, flags, 0o600)
    locked = False
    try:
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode) or info.st_nlink != 1:
            raise Rejected(f"journal must be a private regular file: {path}")
        if os.name == "nt":
            import msvcrt

            os.lseek(fd, 0, os.SEEK_SET)
            msvcrt.locking(fd, msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(fd, fcntl.LOCK_EX)
        locked = True
        yield fd
    finally:
        if locked:
            if os.name == "nt":
                import msvcrt

                os.lseek(fd, 0, os.SEEK_SET)
                msvcrt.locking(fd, msvcrt.LK_UNLCK, 1)
            else:
                import fcntl

                fcntl.flock(fd, fcntl.LOCK_UN)
        os.close(fd)


def _read_fd(fd: int) -> bytes:
    os.lseek(fd, 0, os.SEEK_SET)
    parts: list[bytes] = []
    while True:
        block = os.read(fd, 1024 * 1024)
        if not block:
            break
        parts.append(block)
    return b"".join(parts)


def _fd_aliases_path(fd: int, path: Path) -> bool:
    try:
        opened = os.fstat(fd)
        other = path.stat()
    except OSError as exc:
        raise Rejected(f"cannot compare open journal with {path}: {exc}") from exc
    if opened.st_ino == 0 or other.st_ino == 0:
        return False
    return (opened.st_dev, opened.st_ino) == (other.st_dev, other.st_ino)


def append_receipt(request_path: Path, journal_path: Path) -> dict[str, Any]:
    request_path = _absolute(request_path)
    journal_path = _absolute(journal_path)
    if os.path.normcase(os.fspath(request_path)) == os.path.normcase(os.fspath(journal_path)):
        raise Rejected("journal path must not replace the request")
    request_descriptor = descriptor(request_path)
    request = validate_request(request_path)
    if descriptor(request_path) != request_descriptor:
        raise Rejected("request changed while it was being validated")
    collision_items: list[tuple[str, Path]] = [
        ("journal", journal_path),
        ("request", request_path),
        ("request.live_plan", Path(str(request["live_plan"]["path"]))),
        (
            "request.measurement_receipt",
            Path(str(request["measurement_receipt"]["path"])),
        ),
    ]
    collision_items.extend(
        (
            f"request.children[{index}].artifact",
            Path(str(child["artifact"]["path"])),
        )
        for index, child in enumerate(request["children"])
    )
    _reject_aliases(collision_items)
    with _locked_append_fd(journal_path) as fd:
        for label, path in collision_items[1:]:
            if _fd_aliases_path(fd, path):
                raise Rejected(f"open journal aliases {label}: {path}")
        prefix = _read_fd(fd)
        rows = _parse_journal(prefix, "journal")
        unsigned = {
            "schema": RECEIPT_SCHEMA,
            "sequence": len(rows),
            "previous_receipt_sha256": rows[-1]["receipt_sha256"] if rows else None,
            "request": request_descriptor,
            "request_payload_sha256": request_descriptor["sha256"],
            "task_id": request["task_id"],
            "live_plan": request["live_plan"],
            "children": request["children"],
            "active_seconds": request["active_seconds"],
            "active_seconds_measured": True,
            "measurement_receipt": request["measurement_receipt"],
            "policy": request["policy"],
        }
        row = {**unsigned, "receipt_sha256": _receipt_hash(unsigned)}
        payload = _canonical(row)
        written = 0
        while written < len(payload):
            count = os.write(fd, payload[written:])
            if count <= 0:
                raise OSError("append returned no progress")
            written += count
        os.fsync(fd)
        # Verify the bytes read back under the same lock before reporting success.
        final_rows = _parse_journal(_read_fd(fd), "journal")
        if final_rows[-1] != row or len(final_rows) != len(rows) + 1:
            raise Rejected("journal append did not produce the expected receipt chain")
        return row


def _now_utc() -> str:
    return (
        datetime.now(timezone.utc)
        .isoformat(timespec="microseconds")
        .replace("+00:00", "Z")
    )


def _event_hash(value: Mapping[str, Any]) -> str:
    return _sha256_bytes(_canonical(value))


def _atomic_create(path: Path, payload: bytes, label: str) -> None:
    path = _absolute(path)
    _assert_no_indirection(path, label, allow_missing=True)
    path.parent.mkdir(parents=True, exist_ok=True)
    _assert_no_indirection(path, label, allow_missing=True)
    if path.exists():
        raise Rejected(f"{label} already exists: {path}")
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "wb",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        try:
            os.link(temporary, path)
        except FileExistsError as exc:
            raise Rejected(f"{label} already exists: {path}") from exc
        temporary.unlink()
        temporary = None
        _assert_regular_private(path, label)
        if os.name != "nt":
            directory_fd = os.open(path.parent, os.O_RDONLY)
            try:
                os.fsync(directory_fd)
            finally:
                os.close(directory_fd)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def _validate_start_event(value: Any) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={
            "schema",
            "event",
            "sequence",
            "started_utc",
            "start_perf_counter_ns",
            "start_monotonic_ns",
            "diagnostic_only",
            "authority_advanced",
            "event_sha256",
        },
        required={
            "schema",
            "event",
            "sequence",
            "started_utc",
            "start_perf_counter_ns",
            "start_monotonic_ns",
            "diagnostic_only",
            "authority_advanced",
            "event_sha256",
        },
        label="measurement start event",
    )
    if item["schema"] != MEASUREMENT_EVENT_SCHEMA or item["event"] != "START":
        raise Rejected("measurement sidecar must begin with a START event")
    if _integer(item["sequence"], "measurement start event.sequence") != 0:
        raise Rejected("measurement start event.sequence must be zero")
    _utc(item["started_utc"], "measurement start event.started_utc")
    _integer(
        item["start_perf_counter_ns"],
        "measurement start event.start_perf_counter_ns",
        minimum=1,
    )
    _integer(
        item["start_monotonic_ns"],
        "measurement start event.start_monotonic_ns",
        minimum=1,
    )
    if item["diagnostic_only"] is not True or item["authority_advanced"] is not False:
        raise Rejected("measurement start event has authority-bearing policy")
    claimed = _sha(item["event_sha256"], "measurement start event.event_sha256")
    unsigned = dict(item)
    del unsigned["event_sha256"]
    if claimed != _event_hash(unsigned):
        raise Rejected("measurement start event hash mismatch")
    return item


def _validate_stop_event(value: Any, start: Mapping[str, Any]) -> dict[str, Any]:
    item = _closed(
        value,
        allowed={
            "schema",
            "event",
            "sequence",
            "previous_event_sha256",
            "stopped_utc",
            "end_perf_counter_ns",
            "end_monotonic_ns",
            "active_seconds",
            "closed_receipt",
            "diagnostic_only",
            "authority_advanced",
            "event_sha256",
        },
        required={
            "schema",
            "event",
            "sequence",
            "previous_event_sha256",
            "stopped_utc",
            "end_perf_counter_ns",
            "end_monotonic_ns",
            "active_seconds",
            "closed_receipt",
            "diagnostic_only",
            "authority_advanced",
            "event_sha256",
        },
        label="measurement stop event",
    )
    if item["schema"] != MEASUREMENT_EVENT_SCHEMA or item["event"] != "STOP":
        raise Rejected("measurement sidecar second row must be a STOP event")
    if _integer(item["sequence"], "measurement stop event.sequence") != 1:
        raise Rejected("measurement stop event.sequence must be one")
    if _sha(
        item["previous_event_sha256"],
        "measurement stop event.previous_event_sha256",
    ) != start["event_sha256"]:
        raise Rejected("measurement stop event previous hash mismatch")
    _utc(item["stopped_utc"], "measurement stop event.stopped_utc")
    _integer(
        item["end_perf_counter_ns"],
        "measurement stop event.end_perf_counter_ns",
        minimum=1,
    )
    _integer(
        item["end_monotonic_ns"],
        "measurement stop event.end_monotonic_ns",
        minimum=1,
    )
    active_seconds = _positive_seconds(
        item["active_seconds"], "measurement stop event.active_seconds"
    )
    closed = _authenticated_descriptor(
        item["closed_receipt"], Path.cwd(), "measurement stop event.closed_receipt"
    )
    _validate_measurement_receipt(closed, active_seconds)
    if item["diagnostic_only"] is not True or item["authority_advanced"] is not False:
        raise Rejected("measurement stop event has authority-bearing policy")
    claimed = _sha(item["event_sha256"], "measurement stop event.event_sha256")
    unsigned = dict(item)
    del unsigned["event_sha256"]
    if claimed != _event_hash(unsigned):
        raise Rejected("measurement stop event hash mismatch")
    return item


def _parse_measurement_sidecar(payload: bytes) -> list[dict[str, Any]]:
    if not payload or not payload.endswith(b"\n"):
        raise Rejected("measurement sidecar is empty or has a partial final line")
    lines = payload.splitlines()
    if len(lines) not in {1, 2}:
        raise Rejected("measurement sidecar must contain START and optional STOP rows")
    start = _validate_start_event(_decode_json(lines[0], "measurement sidecar[0]"))
    if lines[0] + b"\n" != _canonical(start):
        raise Rejected("measurement sidecar[0] is not canonically encoded")
    rows = [start]
    if len(lines) == 2:
        stop = _validate_stop_event(
            _decode_json(lines[1], "measurement sidecar[1]"), start
        )
        if lines[1] + b"\n" != _canonical(stop):
            raise Rejected("measurement sidecar[1] is not canonically encoded")
        rows.append(stop)
    return rows


def measure_start(open_receipt_path: Path) -> dict[str, Any]:
    path = _absolute(open_receipt_path)
    unsigned = {
        "schema": MEASUREMENT_EVENT_SCHEMA,
        "event": "START",
        "sequence": 0,
        "started_utc": _now_utc(),
        "start_perf_counter_ns": time.perf_counter_ns(),
        "start_monotonic_ns": time.monotonic_ns(),
        "diagnostic_only": True,
        "authority_advanced": False,
    }
    row = {**unsigned, "event_sha256": _event_hash(unsigned)}
    _atomic_create(path, _canonical(row), "open measurement receipt")
    _parse_measurement_sidecar(path.read_bytes())
    return row


def measure_stop(open_receipt_path: Path, closed_receipt_path: Path) -> dict[str, Any]:
    open_path = _absolute(open_receipt_path)
    closed_path = _absolute(closed_receipt_path)
    _assert_regular_private(open_path, "open measurement receipt")
    _assert_no_indirection(closed_path, "closed measurement receipt", allow_missing=True)
    _reject_aliases(
        [("open measurement receipt", open_path), ("closed measurement receipt", closed_path)]
    )
    with _locked_append_fd(open_path) as fd:
        rows = _parse_measurement_sidecar(_read_fd(fd))
        if len(rows) != 1:
            raise Rejected("open measurement receipt is already closed")
        if closed_path.exists():
            raise Rejected(f"closed measurement receipt already exists: {closed_path}")
        start = rows[0]
        stopped_utc = _now_utc()
        end_perf = time.perf_counter_ns()
        end_mono = time.monotonic_ns()
        start_perf = int(start["start_perf_counter_ns"])
        start_mono = int(start["start_monotonic_ns"])
        perf_delta = end_perf - start_perf
        mono_delta = end_mono - start_mono
        if perf_delta <= 0 or mono_delta <= 0:
            raise Rejected("measurement interval is zero or negative")
        if perf_delta > MAX_MEASUREMENT_SECONDS * 1_000_000_000:
            raise Rejected("open measurement receipt is stale")
        if abs(perf_delta - mono_delta) > 50_000_000:
            raise Rejected("measurement clocks disagree by more than 50ms")
        active_seconds = perf_delta / 1_000_000_000
        closed = {
            "schema": MEASUREMENT_SCHEMA,
            "generator": MEASUREMENT_GENERATOR,
            "clock": "perf_counter_ns+monotonic_ns",
            "intervals": [
                {
                    "started_utc": start["started_utc"],
                    "stopped_utc": stopped_utc,
                    "start_perf_counter_ns": start_perf,
                    "end_perf_counter_ns": end_perf,
                    "start_monotonic_ns": start_mono,
                    "end_monotonic_ns": end_mono,
                }
            ],
            "active_seconds": active_seconds,
            "measurement_complete": True,
            "diagnostic_only": True,
            "authority_advanced": False,
        }
        created = False
        stop_appended = False
        try:
            _atomic_create(
                closed_path,
                _canonical(closed),
                "closed measurement receipt",
            )
            created = True
            closed_descriptor = descriptor(closed_path)
            _validate_measurement_receipt(closed_descriptor, active_seconds)
            stop_unsigned = {
                "schema": MEASUREMENT_EVENT_SCHEMA,
                "event": "STOP",
                "sequence": 1,
                "previous_event_sha256": start["event_sha256"],
                "stopped_utc": stopped_utc,
                "end_perf_counter_ns": end_perf,
                "end_monotonic_ns": end_mono,
                "active_seconds": active_seconds,
                "closed_receipt": closed_descriptor,
                "diagnostic_only": True,
                "authority_advanced": False,
            }
            stop = {**stop_unsigned, "event_sha256": _event_hash(stop_unsigned)}
            payload = _canonical(stop)
            written = 0
            while written < len(payload):
                count = os.write(fd, payload[written:])
                if count <= 0:
                    raise OSError("measurement stop append returned no progress")
                written += count
            os.fsync(fd)
            stop_appended = True
            final_rows = _parse_measurement_sidecar(_read_fd(fd))
            if len(final_rows) != 2 or final_rows[1] != stop:
                raise Rejected("measurement stop marker was not durably appended")
            return closed
        except Exception:
            if created and not stop_appended and closed_path.exists():
                closed_path.unlink()
            raise


def _request_validation(path: Path) -> dict[str, Any]:
    request = validate_request(path)
    counts = {status: 0 for status in sorted(_JOIN_STATUSES)}
    for child in request["children"]:
        counts[child["source_span_join"]["status"]] += 1
    return {
        "schema": VALIDATION_SCHEMA,
        "status": "READY",
        "request": descriptor(_absolute(path)),
        "task_id": request["task_id"],
        "child_count": len(request["children"]),
        "source_span_status_counts": counts,
        "active_seconds": request["active_seconds"],
        "policy": request["policy"],
    }


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    validate_request_parser = subparsers.add_parser(
        "validate-request",
        help="validate one closed execution-receipt request and all bound files",
    )
    validate_request_parser.add_argument("request", type=Path)
    append_parser = subparsers.add_parser(
        "append", help="append one validated request to a hash-chained JSONL journal"
    )
    append_parser.add_argument("request", type=Path)
    append_parser.add_argument("journal", type=Path)
    validate_journal_parser = subparsers.add_parser(
        "validate-journal", help="revalidate every journal row, request, and artifact"
    )
    validate_journal_parser.add_argument("journal", type=Path)
    measure_start_parser = subparsers.add_parser(
        "measure-start",
        help=(
            "exclusively create an open diagnostic active-time sidecar using "
            "UTC, perf_counter_ns, and monotonic_ns"
        ),
    )
    measure_start_parser.add_argument("open_receipt", type=Path)
    measure_stop_parser = subparsers.add_parser(
        "measure-stop",
        help=(
            "consume an open sidecar and atomically publish a closed, measured "
            "mwcc_active_seconds_measurement/v1 receipt"
        ),
    )
    measure_stop_parser.add_argument("open_receipt", type=Path)
    measure_stop_parser.add_argument("closed_receipt", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        if args.command == "validate-request":
            result = _request_validation(args.request)
        elif args.command == "append":
            result = append_receipt(args.request, args.journal)
        elif args.command == "validate-journal":
            result = validate_journal(args.journal)
        elif args.command == "measure-start":
            result = measure_start(args.open_receipt)
        else:
            result = measure_stop(args.open_receipt, args.closed_receipt)
    except (OSError, Rejected) as exc:
        print(f"REJECTED: {exc}", file=sys.stderr)
        return 2
    sys.stdout.buffer.write(_canonical(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
