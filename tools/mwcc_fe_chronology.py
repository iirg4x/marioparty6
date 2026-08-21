#!/usr/bin/env python3
"""Validate the pointer-free MWCC front-end chronology contract.

The native VarInfo probe currently cannot produce a trustworthy join from an
AST ``EOBJREF`` through PCode and IG nodes into the allocator.  In particular,
the compiler's internal pointers are process-local and the existing trace
does not expose an authenticated object identity at every boundary.  This
module therefore defines the consumer-side contract only: a producer must
provide one stable, pointer-free object UID at every stage, and every stack
home candidate must carry hashes binding it to the same source, compiler, and
trace capture.  Missing, duplicate, or conflicting links are errors; callers
must treat them as UNKNOWN and must not guess a join.

Native producer support remains blocked until a newly authenticated compiler
capture can emit this contract.  The validator intentionally does not attempt
to derive UIDs from addresses, names, list order, or register/stack proximity.

The module is standard-library-only and can be used either as a library or as
``python tools/mwcc_fe_chronology.py REPORT.json``.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any


SCHEMA_NAME = "mwcc_fe_chronology/v1"
SCHEMA_VERSION = 1
SCHEMA_PATH = Path(__file__).with_name("MWCC_FE_CHRONOLOGY_V1.schema.json")

TOP_LEVEL_KEYS = {"schema", "schema_version", "producer", "provenance", "objects"}
PRODUCER_KEYS = {"status", "reason"}
PROVENANCE_KEYS = {"source_sha256", "compiler_sha256", "trace_sha256"}
OBJECT_KEYS = {
    "uid",
    "ast_eobjref",
    "pcode",
    "ig_node",
    "allocator",
    "home_join",
}
NODE_KEYS = {"uid", "id"}
PCODE_KEYS = {"uid", "id", "operation"}
HOME_JOIN_KEYS = {"uid", "candidates"}
HOME_CANDIDATE_KEYS = {"uid", "offset", "size", "authenticated", "evidence"}
EVIDENCE_KEYS = {"source_sha256", "compiler_sha256", "trace_sha256"}

_TOKEN_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_.:-]{0,127}$")
_HEX_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_POINTER_RE = re.compile(r"(?:0x[0-9a-f]{6,}|\b[0-9a-f]{8,}\b)", re.IGNORECASE)


class ChronologyError(ValueError):
    """Raised when a chronology report cannot be safely consumed."""


def _require_mapping(value: Any, where: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ChronologyError(f"{where}: expected an object")
    return value


def _require_keys(value: Mapping[str, Any], expected: set[str], where: str) -> None:
    actual = set(value)
    if actual != expected:
        missing = sorted(str(key) for key in expected - actual)
        extra = sorted(str(key) for key in actual - expected)
        raise ChronologyError(f"{where}: keys differ (missing={missing}, extra={extra})")


def _require_string(value: Any, where: str) -> str:
    if not isinstance(value, str) or not value:
        raise ChronologyError(f"{where}: expected a non-empty string")
    return value


def _require_token(value: Any, where: str) -> str:
    token = _require_string(value, where)
    if not _TOKEN_RE.fullmatch(token):
        raise ChronologyError(f"{where}: expected a pointer-free identifier")
    if _POINTER_RE.search(token):
        raise ChronologyError(f"{where}: pointer-looking identifier is forbidden")
    return token


def _require_sha256(value: Any, where: str) -> str:
    digest = _require_string(value, where)
    if not _HEX_SHA256_RE.fullmatch(digest):
        raise ChronologyError(f"{where}: expected lowercase SHA-256")
    return digest


def _require_integer(value: Any, where: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool):
        raise ChronologyError(f"{where}: expected an integer")
    return value


def _validate_producer(value: Any) -> dict[str, str]:
    producer = dict(_require_mapping(value, "producer"))
    _require_keys(producer, PRODUCER_KEYS, "producer")
    if producer["status"] != "blocked":
        raise ChronologyError(
            "producer.status: native producer is blocked until an authenticated compiler capture exists"
        )
    producer["reason"] = _require_string(producer["reason"], "producer.reason")
    return {"status": "blocked", "reason": producer["reason"]}


def _validate_provenance(value: Any) -> dict[str, str]:
    provenance = dict(_require_mapping(value, "provenance"))
    _require_keys(provenance, PROVENANCE_KEYS, "provenance")
    return {
        key: _require_sha256(provenance[key], f"provenance.{key}")
        for key in sorted(PROVENANCE_KEYS)
    }


def _validate_node(value: Any, where: str, uid: str) -> dict[str, str]:
    node = dict(_require_mapping(value, where))
    _require_keys(node, NODE_KEYS, where)
    node_uid = _require_token(node["uid"], f"{where}.uid")
    if node_uid != uid:
        raise ChronologyError(f"{where}.uid: does not match object uid {uid!r}")
    node_id = _require_token(node["id"], f"{where}.id")
    return {"uid": node_uid, "id": node_id}


def _validate_pcode(value: Any, uid: str) -> list[dict[str, str]]:
    if not isinstance(value, Sequence) or isinstance(value, (str, bytes)) or not value:
        raise ChronologyError("object.pcode: expected a non-empty list")
    events: list[dict[str, str]] = []
    ids: set[str] = set()
    for index, raw_event in enumerate(value):
        where = f"object.pcode[{index}]"
        event = dict(_require_mapping(raw_event, where))
        _require_keys(event, PCODE_KEYS, where)
        event_uid = _require_token(event["uid"], f"{where}.uid")
        if event_uid != uid:
            raise ChronologyError(f"{where}.uid: does not match object uid {uid!r}")
        event_id = _require_token(event["id"], f"{where}.id")
        if event_id in ids:
            raise ChronologyError(f"{where}.id: duplicate PCode event id {event_id!r}")
        ids.add(event_id)
        operation = _require_string(event["operation"], f"{where}.operation")
        if operation not in {"create", "reuse"}:
            raise ChronologyError(
                f"{where}.operation: expected 'create' or 'reuse', got {operation!r}"
            )
        events.append({"uid": event_uid, "id": event_id, "operation": operation})
    return events


def _validate_evidence(
    value: Any,
    where: str,
    provenance: Mapping[str, str],
) -> dict[str, str]:
    evidence = dict(_require_mapping(value, where))
    _require_keys(evidence, EVIDENCE_KEYS, where)
    result = {
        key: _require_sha256(evidence[key], f"{where}.{key}")
        for key in sorted(EVIDENCE_KEYS)
    }
    for key in sorted(EVIDENCE_KEYS):
        if result[key] != provenance[key]:
            raise ChronologyError(
                f"{where}.{key}: does not match report provenance; refusing an unbound candidate"
            )
    return result


def _validate_home_join(
    value: Any,
    uid: str,
    provenance: Mapping[str, str],
) -> dict[str, Any]:
    home_join = dict(_require_mapping(value, "object.home_join"))
    _require_keys(home_join, HOME_JOIN_KEYS, "object.home_join")
    join_uid = _require_token(home_join["uid"], "object.home_join.uid")
    if join_uid != uid:
        raise ChronologyError(f"object.home_join.uid: does not match object uid {uid!r}")
    candidates = home_join["candidates"]
    if not isinstance(candidates, Sequence) or isinstance(candidates, (str, bytes)) or not candidates:
        raise ChronologyError("object.home_join.candidates: expected a non-empty list")

    normalized: list[dict[str, Any]] = []
    seen: set[tuple[int, int]] = set()
    for index, raw_candidate in enumerate(candidates):
        where = f"object.home_join.candidates[{index}]"
        candidate = dict(_require_mapping(raw_candidate, where))
        _require_keys(candidate, HOME_CANDIDATE_KEYS, where)
        candidate_uid = _require_token(candidate["uid"], f"{where}.uid")
        if candidate_uid != uid:
            raise ChronologyError(f"{where}.uid: does not match object uid {uid!r}")
        offset = _require_integer(candidate["offset"], f"{where}.offset")
        size = _require_integer(candidate["size"], f"{where}.size")
        if offset % 4:
            raise ChronologyError(f"{where}.offset: stack-home offsets must be 4-byte aligned")
        if size <= 0:
            raise ChronologyError(f"{where}.size: must be positive")
        key = (offset, size)
        if key in seen:
            raise ChronologyError(f"{where}: duplicate stack-home offset/size candidate")
        seen.add(key)
        if candidate["authenticated"] is not True:
            raise ChronologyError(f"{where}.authenticated: candidate is not authenticated")
        evidence = _validate_evidence(candidate["evidence"], f"{where}.evidence", provenance)
        normalized.append(
            {
                "uid": candidate_uid,
                "offset": offset,
                "size": size,
                "authenticated": True,
                "evidence": evidence,
            }
        )
    return {"uid": join_uid, "candidates": normalized}


def _validate_object(
    value: Any,
    index: int,
    provenance: Mapping[str, str],
) -> dict[str, Any]:
    where = f"objects[{index}]"
    obj = dict(_require_mapping(value, where))
    _require_keys(obj, OBJECT_KEYS, where)
    uid = _require_token(obj["uid"], f"{where}.uid")
    normalized = {
        "uid": uid,
        "ast_eobjref": _validate_node(obj["ast_eobjref"], f"{where}.ast_eobjref", uid),
        "pcode": _validate_pcode(obj["pcode"], uid),
        "ig_node": _validate_node(obj["ig_node"], f"{where}.ig_node", uid),
        "allocator": _validate_node(obj["allocator"], f"{where}.allocator", uid),
        "home_join": _validate_home_join(obj["home_join"], uid, provenance),
    }
    return normalized


def validate_report(value: Mapping[str, Any]) -> dict[str, Any]:
    """Validate one chronology report and return its normalized value.

    Validation is deliberately strict.  A report with an absent or ambiguous
    link is rejected instead of returning a partial join that downstream code
    might mistake for evidence.
    """

    report = dict(_require_mapping(value, "report"))
    _require_keys(report, TOP_LEVEL_KEYS, "report")
    if report["schema"] != SCHEMA_NAME or report["schema_version"] != SCHEMA_VERSION:
        raise ChronologyError("report: unsupported chronology schema")
    if isinstance(report["schema_version"], bool) or not isinstance(
        report["schema_version"], int
    ):
        raise ChronologyError("report.schema_version: expected integer 1")
    producer = _validate_producer(report["producer"])
    provenance = _validate_provenance(report["provenance"])
    objects = report["objects"]
    if not isinstance(objects, Sequence) or isinstance(objects, (str, bytes)) or not objects:
        raise ChronologyError("report.objects: expected a non-empty list")

    normalized_objects: list[dict[str, Any]] = []
    seen_uids: set[str] = set()
    for index, raw_object in enumerate(objects):
        normalized = _validate_object(raw_object, index, provenance)
        uid = normalized["uid"]
        if uid in seen_uids:
            raise ChronologyError(f"objects[{index}].uid: duplicate object UID {uid!r}")
        seen_uids.add(uid)
        normalized_objects.append(normalized)

    return {
        "schema": SCHEMA_NAME,
        "schema_version": SCHEMA_VERSION,
        "producer": producer,
        "provenance": provenance,
        "objects": normalized_objects,
    }


def validate_chronology(value: Mapping[str, Any]) -> dict[str, Any]:
    """Compatibility name for callers that refer to the trace as chronology."""

    return validate_report(value)


def validate_trace(value: Mapping[str, Any]) -> dict[str, Any]:
    """Compatibility name for callers that refer to the report as a trace."""

    return validate_report(value)


def validate(value: Mapping[str, Any]) -> dict[str, Any]:
    """Short validator alias used by small probe drivers."""

    return validate_report(value)


def load_report(path: str | Path) -> dict[str, Any]:
    """Load and validate a UTF-8 JSON chronology report."""

    report_path = Path(path)
    try:
        value = json.loads(report_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ChronologyError(f"invalid chronology report {report_path}: {error}") from error
    return validate_report(value)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("report", type=Path, help="chronology JSON report")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    try:
        report = load_report(_parser().parse_args(argv).report)
    except (ChronologyError, OSError) as error:
        print(f"chronology validation failed: {error}", file=sys.stderr)
        return 2
    print(
        json.dumps(
            {
                "schema": report["schema"],
                "objects": len(report["objects"]),
                "status": "valid",
                "producer_status": report["producer"]["status"],
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
