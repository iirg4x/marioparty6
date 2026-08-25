#!/usr/bin/env python3
"""Seal an external trust root for one authenticated MWCC capture lane.

This is a post-capture evidence tool.  It never launches a compiler, changes
source, or advances Board admission.  The package, active-time receipt, child
request, event streams, and envelope must already exist and agree exactly.
"""

from __future__ import annotations

import argparse
import json
import os
import re
from pathlib import Path
from typing import Any, Mapping, Sequence

import capsule_same_session_capture as capture
import mwcc_execution_receipt as execution


SCHEMA = "mwcc_post_capture_trust_seal/v1"
PACKAGE_SCHEMA = "player_gc27_current_source_capture_package/v2"
PACKAGE_REQUEST_SCHEMA = "player_gc27_phys_reg_capture_request/v490"
MEASUREMENT_SCHEMA = "mwcc_active_seconds_measurement/v1"
LANES = frozenset({"retained", "v491"})
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


class Rejected(ValueError):
    """Raised when any post-capture binding is incomplete or inconsistent."""


def _reject(message: str) -> "NoReturn":
    raise Rejected(message)


def _regular(path: Path, label: str) -> Path:
    try:
        resolved = path.resolve(strict=True)
    except OSError as exc:
        raise Rejected(f"{label} is missing: {exc}") from exc
    if path.is_symlink() or not resolved.is_file():
        _reject(f"{label} is not a regular file")
    return resolved


def _directory(path: Path, label: str) -> Path:
    try:
        resolved = path.resolve(strict=True)
    except OSError as exc:
        raise Rejected(f"{label} is missing: {exc}") from exc
    if path.is_symlink() or not resolved.is_dir():
        _reject(f"{label} is not a regular directory")
    return resolved


def _load(path: Path, label: str) -> tuple[Path, Mapping[str, Any]]:
    path = _regular(path, label)
    try:
        value = capture.strict_json_loads(path.read_text(encoding="utf-8"), label)
    except (OSError, UnicodeError) as exc:
        raise Rejected(f"cannot read {label}: {exc}") from exc
    if not isinstance(value, Mapping):
        _reject(f"{label} must be a JSON object")
    return path, value


def _descriptor(path: Path) -> dict[str, Any]:
    path = _regular(path, "descriptor file")
    return {"path": str(path), "size": path.stat().st_size, "sha256": capture.sha256(path)}


def _digest(value: Any, label: str) -> str:
    if not isinstance(value, str) or not SHA256_RE.fullmatch(value):
        _reject(f"{label} must be a lowercase SHA-256 digest")
    return value


def _session(value: Any, label: str) -> str:
    try:
        return capture._safe_session_id(value)
    except capture.Rejected as exc:
        raise Rejected(f"{label} is not canonical") from exc


def _identity(value: Any, label: str, *, base: Path | None = None) -> dict[str, Any]:
    if not isinstance(value, Mapping):
        _reject(f"{label} must be a descriptor")
    if not {"path", "size", "sha256"}.issubset(value):
        _reject(f"{label} descriptor is incomplete")
    raw = value["path"]
    if not isinstance(raw, str) or not raw:
        _reject(f"{label}.path is invalid")
    path = Path(raw)
    if not path.is_absolute():
        if base is None:
            _reject(f"{label}.path must be absolute")
        path = base / path
    actual = _descriptor(path)
    size = value["size"]
    if isinstance(size, bool) or not isinstance(size, int) or size < 0:
        _reject(f"{label}.size is invalid")
    digest = _digest(value["sha256"], f"{label}.sha256")
    if actual["size"] != size or actual["sha256"] != digest:
        _reject(f"{label} bytes do not match")
    return actual


def _same_identity(left: Mapping[str, Any], right: Mapping[str, Any], label: str) -> None:
    if left["size"] != right["size"] or left["sha256"] != right["sha256"]:
        _reject(f"{label} identity mismatch")
    left_path = Path(str(left["path"])).resolve(strict=True)
    right_path = Path(str(right["path"])).resolve(strict=True)
    if left_path != right_path:
        _reject(f"{label} path mismatch")


def _same_bytes(left: Mapping[str, Any], right: Mapping[str, Any], label: str) -> None:
    if left["size"] != right["size"] or left["sha256"] != right["sha256"]:
        _reject(f"{label} byte identity mismatch")


def _self_digest(value: Mapping[str, Any], key: str, label: str) -> str:
    digest = _digest(value.get(key), f"{label}.{key}")
    unsigned = {name: item for name, item in value.items() if name != key}
    if capture.canonical_hash(unsigned) != digest:
        _reject(f"{label} self-digest mismatch")
    return digest


def _compiler_output(request: Mapping[str, Any]) -> Path:
    argv = request.get("argv")
    cwd = request.get("cwd")
    if not isinstance(argv, list) or not isinstance(cwd, str):
        _reject("child request compiler context is malformed")
    outputs: list[Path] = []
    for index, value in enumerate(argv):
        if value == "-o":
            if index + 1 >= len(argv) or not isinstance(argv[index + 1], str):
                _reject("child request compiler -o operand is missing")
            output = Path(argv[index + 1])
            if not output.is_absolute():
                output = Path(cwd) / output
            outputs.append(output)
    if len(outputs) != 1:
        _reject("child request must contain exactly one compiler output")
    return _regular(outputs[0], "child compiler output")


def _measurement(path: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    path, value = _load(path, "active-time measurement")
    if value.get("schema") != MEASUREMENT_SCHEMA:
        _reject("active-time measurement schema mismatch")
    seconds = value.get("active_seconds")
    if isinstance(seconds, bool) or not isinstance(seconds, (int, float)) or seconds <= 0:
        _reject("active-time measurement must contain positive active_seconds")
    descriptor = _descriptor(path)
    try:
        normalized = execution._validate_measurement_receipt(descriptor, float(seconds))
    except (execution.Rejected, OSError, ValueError) as exc:
        raise Rejected(f"active-time measurement is not complete: {exc}") from exc
    return normalized, descriptor


def seal_trust_root(
    *,
    package_request_path: Path,
    package_receipt_path: Path,
    measurement_path: Path,
    child_request_path: Path,
    envelope_path: Path,
    lane: str,
    output_path: Path,
) -> dict[str, Any]:
    """Validate all provenance and write one ExternalTrustRoot mapping."""

    if lane not in LANES:
        _reject(f"lane must be one of {sorted(LANES)}")
    package_request_path, package = _load(package_request_path, "package request")
    package_receipt_path, receipt = _load(package_receipt_path, "package receipt")
    child_request_path, child = _load(child_request_path, "child request")
    envelope_path = _regular(envelope_path, "child envelope")

    if package.get("schema") != PACKAGE_REQUEST_SCHEMA:
        _reject("package request schema mismatch")
    if receipt.get("schema") != PACKAGE_SCHEMA:
        _reject("package receipt schema mismatch")
    for label, value in (("package request", package), ("package receipt", receipt)):
        if value.get("diagnostic_only") is not True or value.get("board_admission") is not False:
            _reject(f"{label} policy is not diagnostic-only")
        if value.get("exactness_claim") is not False:
            _reject(f"{label} exactness claim is not false")
    if receipt.get("authority_advanced") is not False:
        _reject("package receipt advances authority")
    if any(receipt.get(key) is not False for key in (
        "compiler_or_capture_run", "compiler_or_live_capture_run", "compiler_run", "capture_run"
    )):
        _reject("package receipt prelaunch run flags are not closed")

    package_session = _session(package.get("session_id"), "package request session")
    if _session(receipt.get("session_id"), "package receipt session") != package_session:
        _reject("package request/receipt session mismatch")
    _self_digest(package, "request_sha256", "package request")

    package_root = _directory(Path(str(receipt.get("root", ""))), "package root")
    if package_request_path != package_root / "backend-request.json":
        _reject("package request is not rooted at backend-request.json")
    if package_receipt_path != package_root / "package-receipt.json":
        _reject("package receipt is not rooted at package-receipt.json")
    receipt_request = _identity(receipt.get("request"), "package receipt request", base=package_root)
    _same_identity(receipt_request, _descriptor(package_request_path), "package request")

    for key in ("function", "function_sha256"):
        if receipt.get(key) != package.get(key):
            _reject(f"package request/receipt {key} mismatch")
    function = package.get("function")
    function_sha256 = _digest(package.get("function_sha256"), "package function_sha256")
    source = _identity(package.get("source"), "package source")
    _same_identity(source, _identity(receipt.get("source"), "package receipt source"), "package source")
    sources = package.get("sources")
    if not isinstance(sources, Mapping) or lane not in sources:
        _reject("package request does not define the selected lane")
    lane_source = _identity(sources[lane], f"package sources.{lane}")
    _same_bytes(source, lane_source, "selected lane source")

    package_tool = _identity(package.get("same_session_capture"), "package same-session tool")
    receipt_tool = _identity(receipt.get("current_tool"), "package receipt current tool")
    _same_identity(package_tool, receipt_tool, "same-session tool")
    package_compiler = _identity(package.get("compiler"), "package compiler")
    package_wrapper = _identity(package.get("wrapper"), "package wrapper")
    package_authority = _identity(package.get("authority"), "package authority")

    if child.get("schema") != capture.REQUEST_SCHEMA or child.get("tool_version") != capture.TOOL_VERSION:
        _reject("child request schema/tool version mismatch")
    _self_digest(child, "request_sha256", "child request")
    if _session(child.get("session_id"), "child request session") != package_session:
        _reject("child request session does not match package")
    if child.get("function") != function or child.get("function_sha256") != function_sha256:
        _reject("child function binding does not match package")
    if child.get("diagnostic_only") is not True or child.get("board_admission") is not False or child.get("exactness_claim") is not False:
        _reject("child request policy mismatch")

    child_source = _identity(child.get("source"), "child source")
    _same_identity(child_source, lane_source, "child selected-lane source")
    for key, expected in (
        ("compiler", package_compiler),
        ("wrapper", package_wrapper),
        ("debugger", package_tool),
        ("transport", package_tool),
        ("authority", package_authority),
    ):
        _same_identity(_identity(child.get(key), f"child {key}"), expected, f"child {key}")

    hook_union = package.get("custom_hook_union")
    if not isinstance(hook_union, Mapping) or hook_union.get("count") != 13:
        _reject("package request lacks the complete 13-hook union")
    rows = hook_union.get("rows")
    if child.get("hooks") != rows:
        _reject("child hooks do not match the package 13-hook union")
    capture._validate_hook_rows(rows, "package hooks", compiler_sha256=package_compiler["sha256"])

    expected_object = package.get("expected_object")
    if not isinstance(expected_object, Mapping):
        _reject("package expected object descriptor is missing")
    actual_object = _descriptor(_compiler_output(child))
    expected_size = expected_object.get("size")
    expected_digest = _digest(expected_object.get("sha256"), "package expected object sha256")
    if actual_object["size"] != expected_size or actual_object["sha256"] != expected_digest:
        _reject("child compiler output does not match the package expected object")

    measurement, measurement_descriptor = _measurement(measurement_path)
    paths = child.get("paths")
    if not isinstance(paths, Mapping) or set(paths) != {
        "event_stream_stack", "event_stream_pcode", "envelope"
    }:
        _reject("child output paths are not closed")
    if _regular(Path(str(paths["envelope"])), "request envelope") != envelope_path:
        _reject("explicit envelope path does not match child request")

    trust_mapping: dict[str, Any] = {
        "request": _descriptor(child_request_path),
        "source": child_source,
        "compiler": package_compiler,
        "wrapper": package_wrapper,
        "debugger": package_tool,
        "transport": package_tool,
        "authority": package_authority,
        "event_stream_stack": _descriptor(Path(str(paths["event_stream_stack"]))),
        "event_stream_pcode": _descriptor(Path(str(paths["event_stream_pcode"]))),
        "envelope": _descriptor(envelope_path),
        "function": function,
        "function_sha256": function_sha256,
        "cwd": child.get("cwd"),
        "argv": child.get("argv"),
    }
    root = capture.ExternalTrustRoot.from_mapping(trust_mapping)
    capture.authenticate_request(child_request_path, external_trust_root=root)
    validated = capture.validate_envelope(
        envelope_path,
        external_trust_root=root,
        request_path=child_request_path,
    )

    output = output_path.resolve(strict=False)
    if output.exists() or output.is_symlink():
        _reject("trust-root output already exists")
    capture.write_new(
        output,
        (json.dumps(trust_mapping, indent=2, sort_keys=True) + "\n").encode("utf-8"),
    )
    capture.ExternalTrustRoot.from_mapping(
        capture.strict_json_loads(output.read_text(encoding="utf-8"), "sealed trust root")
    )
    return {
        "schema": SCHEMA,
        "status": "SEALED",
        "lane": lane,
        "session_id": package_session,
        "function": function,
        "request": _descriptor(child_request_path),
        "envelope": _descriptor(envelope_path),
        "trust_root": _descriptor(output),
        "measurement": measurement_descriptor,
        "active_seconds": measurement["active_seconds"],
        "envelope_event_count": validated["event_count"],
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
    }


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--package-request", type=Path, required=True)
    result.add_argument("--package-receipt", type=Path, required=True)
    result.add_argument("--measurement", type=Path, required=True)
    result.add_argument("--child-request", type=Path, required=True)
    result.add_argument("--envelope", type=Path, required=True)
    result.add_argument("--lane", choices=sorted(LANES), required=True)
    result.add_argument("--output", type=Path, required=True)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        result = seal_trust_root(
            package_request_path=args.package_request,
            package_receipt_path=args.package_receipt,
            measurement_path=args.measurement,
            child_request_path=args.child_request,
            envelope_path=args.envelope,
            lane=args.lane,
            output_path=args.output,
        )
    except (Rejected, capture.Rejected, execution.Rejected, OSError, ValueError) as exc:
        print(json.dumps({
            "schema": SCHEMA,
            "status": "UNKNOWN",
            "reason": str(exc),
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
        }, indent=2, sort_keys=True))
        return 2
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
