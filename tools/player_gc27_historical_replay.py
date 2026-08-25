#!/usr/bin/env python3
"""Prepare and compare a fail-closed two-lane Player GC/2.7 replay.

This tool never launches MWCC.  ``prepare`` binds two distinct historical
MoveNum sources to the repaired 13-hook runtime and emits one explicit live
command.  ``compare`` normalizes only documented run-local identities before
comparing the resulting physical-register envelopes with sealed v523e inputs.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Mapping, NoReturn, Sequence


REQUEST_SCHEMA = "player_gc27_historical_replay_request/v1"
PLAN_SCHEMA = "player_gc27_historical_replay_plan/v1"
RECEIPT_SCHEMA = "player_gc27_historical_replay_receipt/v1"
COMPARISON_SCHEMA = "player_gc27_historical_replay_comparison/v1"
RUNTIME_REQUEST_SCHEMA = "player_gc27_phys_reg_capture_request/v490"
SESSION_RE = re.compile(r"^session-[0-9a-f]{16}$")
LANES = ("retained", "v491")
HOOK_ORDER = (
    "function_filter", "allocation_pre", "allocation_post",
    "object_write_0", "object_write_1", "object_write_2", "regalloc",
    "physical_pair_commit", "physical_single_commit", "precolored_commit",
    "pcode_color_pre", "pcode_color_post", "gc27_machine_emit",
)
EXPECTED_HOOKS = {
    "function_filter": (0x00433492, "8b400e8b5006eb08", "function_filter"),
    "allocation_pre": (0x0043367E, "e87d650c00598a44240450", "numeric_stack_alloc_pre"),
    "allocation_post": (0x00433683, "598a4424045068404e4300", "numeric_stack_alloc_post"),
    "object_write_0": (0x004F9D74, "89432e8b530e8b420201e84821f00105e40c", "object_stack_write"),
    "object_write_1": (0x004F9E11, "89432e8b4b0e8b410201e84821f00105dc0c", "object_stack_write"),
    "object_write_2": (0x004F9E98, "89432e8b4b0e8b410201e84821f00105d80c", "object_stack_write"),
    "regalloc": (0x0043598B, "ff74240ce89ca809", "regalloc"),
    "physical_pair_commit": (0x004D0E65, "5d5f5e5bc3", "regalloc_post"),
    "physical_single_commit": (0x004D0F6E, "5d5f5e5bc3", "regalloc_post"),
    "precolored_commit": (0x004D0A7B, "eb768d4000", "regalloc_post"),
    "pcode_color_pre": (0x005086C4, "6689420483c20c83", "pcode_color_diagnostic"),
    "pcode_color_post": (0x005086C8, "83c20c83ed0173d3", "pcode_color_diagnostic"),
    "gc27_machine_emit": (0x004EB21F, "8b178b0a030dd00b5e0001e989018b43", "machine_emit"),
}
IGNORED_RUN_LOCAL_FIELDS = (
    "backend.path", "backend.sha256", "object.path", "request.path",
    "request.sha256", "semantic_gate.actual_object.path",
    "semantic_gate.expected_object.path", "semantic_gate.report",
    "session_id", "events[].object_id", "events[].varinfo_id",
    "pcode_color_rows[].object_id", "source_bindings[].object_id",
    "source_bindings[].varinfo_id",
)


class ReplayError(RuntimeError):
    """A replay input or result failed an authentication gate."""


def _fail(message: str) -> NoReturn:
    raise ReplayError(message)


def _regular(path: Path, *, label: str) -> None:
    if not path.exists() or path.is_symlink() or os.path.islink(path) or not path.is_file():
        _fail(f"{label} is not a regular non-symlink file: {path}")


def _sha256(path: Path) -> str:
    _regular(path, label="hashed file")
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("utf-8")


def _read_json(path: Path, *, label: str) -> dict[str, Any]:
    _regular(path, label=label)
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        _fail(f"{label} is not valid UTF-8 JSON: {exc}")
    if not isinstance(value, dict):
        _fail(f"{label} must contain a JSON object")
    return value


def _write_exclusive(path: Path, value: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    handle = os.open(path, flags, 0o600)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as stream:
            json.dump(value, stream, indent=2, sort_keys=True, ensure_ascii=False)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
    except Exception:
        raise


def _descriptor(path: Path) -> dict[str, Any]:
    return {"path": str(path), "sha256": _sha256(path), "size": path.stat().st_size}


def _validate_descriptor(value: Any, *, label: str) -> Path:
    if not isinstance(value, Mapping):
        _fail(f"{label} descriptor is missing")
    path_value = value.get("path")
    digest = value.get("sha256")
    size = value.get("size")
    if not isinstance(path_value, str) or not path_value:
        _fail(f"{label} descriptor path is missing")
    if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
        _fail(f"{label} descriptor SHA-256 is invalid")
    if not isinstance(size, int) or size <= 0:
        _fail(f"{label} descriptor size is invalid")
    path = Path(path_value)
    _regular(path, label=label)
    if path.stat().st_size != size or _sha256(path) != digest:
        _fail(f"{label} descriptor drift")
    return path


def _hook_row(name: str) -> dict[str, Any]:
    address, prefix, role = EXPECTED_HOOKS[name]
    return {"id": name, "address": address, "prefix": prefix, "role": role}


def _validate_hooks(rows: Any, *, label: str) -> None:
    if not isinstance(rows, list) or len(rows) != 13:
        _fail(f"{label} is not the exact 13-hook profile")
    if [row.get("id") for row in rows if isinstance(row, Mapping)] != list(HOOK_ORDER):
        _fail(f"{label} hook order differs")
    for name, row in zip(HOOK_ORDER, rows):
        if not isinstance(row, Mapping):
            _fail(f"{label}.{name} is not an object")
        expected = _hook_row(name)
        for key, expected_value in expected.items():
            if row.get(key) != expected_value:
                _fail(f"{label}.{name}.{key} differs")
    if "004d03e8" in json.dumps(rows, sort_keys=True).casefold():
        _fail(f"{label} contains the stale 0x004D03E8 hook")


def _replace_session(value: Any, old: str, new: str) -> Any:
    if isinstance(value, str):
        return value.replace(old, new)
    if isinstance(value, list):
        return [_replace_session(item, old, new) for item in value]
    if isinstance(value, dict):
        return {key: _replace_session(item, old, new) for key, item in value.items()}
    return value


def _runtime_assets(root: Path, value: Mapping[str, Any]) -> dict[str, Path]:
    runtime = value.get("runtime")
    if not isinstance(runtime, Mapping):
        _fail("runtime binding is missing")
    receipt_path = _validate_descriptor(runtime.get("package_receipt"), label="runtime package receipt")
    if receipt_path.parent.resolve() != root.resolve():
        _fail("runtime package receipt is not directly inside runtime_package_root")
    receipt = _read_json(receipt_path, label="runtime package receipt")
    if receipt.get("diagnostic_only") is not True or receipt.get("authority_advanced") is not False:
        _fail("runtime package receipt is not closed diagnostic-only")
    assets = {
        "request": root / "backend-request.json",
        "manifest": root / "manifest.json",
        "launcher": root / "inputs" / "launch_movenum_capture.py",
        "backend": root / "backend" / "movenum-pcode-color-v523" / "physical_capture_backend_v523.py",
        "adapter": root / "backend" / "movenum-pcode-color-v523" / "player-phys-reg-trace-v490" / "gc27_phys_reg_adapter.py",
    }
    files = receipt.get("files")
    if not isinstance(files, Mapping):
        _fail("runtime package receipt file closure is missing")
    for role, path in assets.items():
        _regular(path, label=f"runtime {role}")
        try:
            rel = path.resolve().relative_to(root.resolve()).as_posix()
        except ValueError:
            _fail(f"runtime {role} escapes package root")
        descriptor = files.get(rel)
        if not isinstance(descriptor, Mapping) or descriptor.get("sha256") != _sha256(path) or descriptor.get("size") != path.stat().st_size:
            _fail(f"runtime {role} is not authenticated by the package receipt")
    manifest = _read_json(assets["manifest"], label="runtime manifest")
    request = _read_json(assets["request"], label="runtime request")
    _validate_hooks(manifest.get("hooks"), label="runtime manifest hooks")
    union = request.get("custom_hook_union")
    if not isinstance(union, Mapping) or union.get("count") != 13:
        _fail("runtime request custom hook union is incomplete")
    _validate_hooks(union.get("rows"), label="runtime request hooks")
    if request.get("schema") != RUNTIME_REQUEST_SCHEMA:
        _fail("runtime request schema differs")
    assets["receipt"] = receipt_path
    return assets


def _validate_physical_envelope(path: Path, *, lane: str, expected_object: Mapping[str, Any]) -> dict[str, Any]:
    envelope = _read_json(path, label=f"{lane} historical envelope")
    if envelope.get("diagnostic_only") is not True or envelope.get("board_admission") is not False or envelope.get("exactness_claim") is not False:
        _fail(f"{lane} historical envelope admission flags are open")
    if envelope.get("capture_label") != lane or envelope.get("function") != "MoveNumOMExec":
        _fail(f"{lane} historical envelope identity differs")
    events = envelope.get("events")
    pcode = envelope.get("pcode_color_rows")
    bindings = envelope.get("source_bindings")
    if not isinstance(events, list) or envelope.get("event_count") != len(events) or not events:
        _fail(f"{lane} historical event inventory is incomplete")
    if not isinstance(pcode, list) or not pcode or not isinstance(bindings, list) or not bindings:
        _fail(f"{lane} historical PCode/source inventory is incomplete")
    semantic = envelope.get("semantic_gate")
    if not isinstance(semantic, Mapping) or semantic.get("status") != "SEMANTIC_OBJECT_EXACT":
        _fail(f"{lane} historical semantic gate is not exact")
    actual = envelope.get("object")
    if not isinstance(actual, Mapping) or actual.get("sha256") != expected_object.get("sha256") or actual.get("size") != expected_object.get("size"):
        _fail(f"{lane} historical compiled object differs from expected_object")
    semantic_actual = semantic.get("actual_object")
    if (
        not isinstance(semantic_actual, Mapping)
        or semantic_actual.get("sha256") != expected_object.get("sha256")
        or semantic_actual.get("size") != expected_object.get("size")
    ):
        _fail(f"{lane} historical semantic actual object differs from compiled object")
    return envelope


def _validate_input(value: Mapping[str, Any]) -> tuple[dict[str, Any], dict[str, Path]]:
    if value.get("schema") != REQUEST_SCHEMA:
        _fail("unsupported historical replay request schema")
    if value.get("diagnostic_only") is not True or value.get("authority_advanced") is not False or value.get("board_admission") is not False or value.get("exactness_claim") is not False:
        _fail("historical replay request admission flags are open")
    session = value.get("session_id")
    if not isinstance(session, str) or SESSION_RE.fullmatch(session) is None:
        _fail("historical replay session_id is not canonical")
    runtime = value.get("runtime")
    if not isinstance(runtime, Mapping) or not isinstance(runtime.get("package_root"), str):
        _fail("runtime package_root is missing")
    runtime_root = Path(runtime["package_root"])
    if not runtime_root.is_dir() or runtime_root.is_symlink():
        _fail("runtime package_root is not a regular directory")
    assets = _runtime_assets(runtime_root, value)
    lanes = value.get("lanes")
    if not isinstance(lanes, Mapping) or set(lanes) != set(LANES):
        _fail("request must contain exactly retained and v491 lanes")
    identities: dict[str, list[str]] = {key: [] for key in ("source", "function", "expected_object", "template")}
    for lane in LANES:
        row = lanes[lane]
        if not isinstance(row, Mapping) or row.get("label") != lane:
            _fail(f"{lane} lane label differs")
        for name in ("source", "expected_object", "source_span_template", "historical_envelope"):
            path = _validate_descriptor(row.get(name), label=f"{lane} {name}")
            assets[f"{lane}_{name}"] = path
        function = row.get("function")
        if not isinstance(function, Mapping) or function.get("name") != "MoveNumOMExec" or re.fullmatch(r"[0-9a-f]{64}", str(function.get("sha256", ""))) is None:
            _fail(f"{lane} function descriptor is invalid")
        template = _read_json(assets[f"{lane}_source_span_template"], label=f"{lane} source-span template")
        if template.get("function") != "MoveNumOMExec" or template.get("function_sha256") != function.get("sha256") or template.get("unsealed") is not True or template.get("authority_advanced") is not False:
            _fail(f"{lane} source-span template identity differs")
        source = row["source"]
        template_source = template.get("source")
        if not isinstance(template_source, Mapping) or template_source.get("sha256") != source.get("sha256") or template_source.get("size") != source.get("size"):
            _fail(f"{lane} template source binding differs")
        historical = _validate_physical_envelope(
            assets[f"{lane}_historical_envelope"],
            lane=lane,
            expected_object=row["expected_object"],
        )
        semantic = historical.get("semantic_gate")
        semantic_expected = semantic.get("expected_object") if isinstance(semantic, Mapping) else None
        assets[f"{lane}_semantic_expected_object"] = _validate_descriptor(
            semantic_expected,
            label=f"{lane} historical semantic expected object",
        )
        identities["source"].append(str(source["sha256"]))
        identities["function"].append(str(function["sha256"]))
        identities["expected_object"].append(str(row["expected_object"]["sha256"]))
        identities["template"].append(str(row["source_span_template"]["sha256"]))
    for name, hashes in identities.items():
        if len(set(hashes)) != 2:
            _fail(f"retained/v491 {name} identities must be distinct")
    return dict(value), assets


def _unsigned_request_digest(value: Mapping[str, Any]) -> str:
    unsigned = json.loads(json.dumps(value))
    unsigned.pop("request_sha256", None)
    return hashlib.sha256(_canonical(unsigned)).hexdigest()


def prepare(request_path: Path, output_root: Path) -> dict[str, Any]:
    request_path = request_path.resolve(strict=True)
    output_root = output_root.resolve(strict=False)
    value, assets = _validate_input(_read_json(request_path, label="historical replay request"))
    if output_root.exists() or output_root.is_symlink():
        _fail(f"refusing to overwrite output root: {output_root}")
    session = value["session_id"]
    runtime_request = _read_json(assets["request"], label="runtime request")
    old_session = str(runtime_request.get("session_id", ""))
    if SESSION_RE.fullmatch(old_session) is None:
        _fail("runtime request session is not canonical")
    replay_request = _replace_session(runtime_request, old_session, session)
    replay_request["sources"] = {}
    for lane in LANES:
        lane_row = value["lanes"][lane]
        replay_request["sources"][lane] = {
            "label": lane,
            "path": lane_row["source"]["path"],
            "sha256": lane_row["source"]["sha256"],
            "size": lane_row["source"]["size"],
            # The historical envelope authenticates two independent roles:
            # the exact object the replay must reproduce and the object used
            # by the original semantic-equivalence gate.  Preserve the latter
            # here; the former remains sealed in the lane plan and comparator.
            "expected_object": _descriptor(assets[f"{lane}_semantic_expected_object"]),
        }
    replay_request["outputs"] = {
        "run_output": str(output_root / "backend-output"),
        "plan": str(output_root / "live-execution-plan.json"),
    }
    replay_request["session_id"] = session
    replay_request["diagnostic_only"] = True
    replay_request["board_admission"] = False
    replay_request["exactness_claim"] = False
    replay_request["authority_advanced"] = False
    replay_request["compiler_run"] = False
    replay_request["capture_run"] = False
    replay_request["request_sha256"] = _unsigned_request_digest(replay_request)
    one_shot = [
        sys.executable, "-B", str(assets["launcher"]),
        "--request", str(output_root / "backend-request.json"),
        "--backend", str(assets["backend"]),
        "--run-output", str(output_root / "backend-output"),
        "--plan-output", str(output_root / "live-execution-plan.json"),
        "--execute",
    ]
    plan = {
        "schema": PLAN_SCHEMA,
        "package_root": str(output_root),
        "diagnostic_only": True,
        "authority_advanced": False,
        "board_admission": False,
        "exactness_claim": False,
        "session_id": session,
        "request": {"path": str(output_root / "backend-request.json"), "sha256": replay_request["request_sha256"]},
        "runtime": {key: _descriptor(assets[key]) for key in ("launcher", "backend", "adapter", "manifest", "receipt")},
        "lanes": {
            lane: {
                "label": lane,
                "source": dict(value["lanes"][lane]["source"]),
                "function": dict(value["lanes"][lane]["function"]),
                "expected_object": dict(value["lanes"][lane]["expected_object"]),
                "semantic_expected_object": _descriptor(assets[f"{lane}_semantic_expected_object"]),
                "source_span_template": dict(value["lanes"][lane]["source_span_template"]),
                "historical_envelope": dict(value["lanes"][lane]["historical_envelope"]),
            } for lane in LANES
        },
        "one_shot": {"argv": one_shot, "execute_requested": True},
        "comparison_command": [sys.executable, "-B", str(Path(__file__).resolve()), "compare", "--package-root", str(output_root), "--output", str(output_root / "comparison-receipt.json")],
    }
    receipt = {
        "schema": RECEIPT_SCHEMA,
        "package_root": str(output_root),
        "diagnostic_only": True,
        "authority_advanced": False,
        "compiler_run": False,
        "capture_run": False,
        "status": "READY_NOT_EXECUTED",
        "session_id": session,
        "request_sha256": replay_request["request_sha256"],
        "plan_sha256": hashlib.sha256(_canonical(plan)).hexdigest(),
        "ignored_run_local_fields": list(IGNORED_RUN_LOCAL_FIELDS),
        "one_shot": plan["one_shot"],
    }
    output_root.mkdir(parents=True, exist_ok=False)
    _write_exclusive(output_root / "backend-request.json", replay_request)
    _write_exclusive(output_root / "historical-replay-plan.json", plan)
    _write_exclusive(output_root / "package-receipt.json", receipt)
    return receipt


def _binding_map(envelope: Mapping[str, Any]) -> dict[str, tuple[str, str]]:
    result: dict[str, tuple[str, str]] = {}
    for row in envelope.get("source_bindings", []):
        if not isinstance(row, Mapping):
            _fail("source binding row is not an object")
        object_id = row.get("object_id")
        name = row.get("source_name")
        kind = row.get("object_kind")
        if not all(isinstance(item, str) and item for item in (object_id, name, kind)) or object_id in result:
            _fail("source binding identity is incomplete or duplicated")
        result[object_id] = (name, kind)
    return result


def _object_identity(value: Any) -> dict[str, Any]:
    if not isinstance(value, Mapping):
        _fail("object identity is missing")
    return {"sha256": value.get("sha256"), "size": value.get("size")}


def normalize_envelope(envelope: Mapping[str, Any]) -> dict[str, Any]:
    bindings = _binding_map(envelope)
    normalized_events: list[dict[str, Any]] = []
    for event in envelope.get("events", []):
        if not isinstance(event, Mapping):
            _fail("physical event is not an object")
        object_id = event.get("object_id")
        if object_id not in bindings:
            _fail("physical event has no authenticated source binding")
        name, kind = bindings[object_id]
        normalized_events.append({
            key: json.loads(json.dumps(value))
            for key, value in event.items() if key not in ("object_id", "varinfo_id")
        } | {"source_name": name, "object_kind": kind})
    normalized_pcode: list[dict[str, Any]] = []
    for row in envelope.get("pcode_color_rows", []):
        if not isinstance(row, Mapping):
            _fail("PCode row is not an object")
        copy = {key: json.loads(json.dumps(value)) for key, value in row.items() if key != "object_id"}
        object_id = row.get("object_id")
        if object_id is not None:
            if object_id not in bindings:
                _fail("PCode row has no authenticated source binding")
            name, kind = bindings[object_id]
            if copy.get("source_name") not in (None, name) or copy.get("object_kind") not in (None, kind):
                _fail("PCode source binding conflicts with source inventory")
            copy["source_name"] = name
            copy["object_kind"] = kind
        normalized_pcode.append(copy)
    semantic = envelope.get("semantic_gate")
    if not isinstance(semantic, Mapping):
        _fail("semantic gate is missing")
    return {
        "schema": envelope.get("schema"),
        "capture_label": envelope.get("capture_label"),
        "function": envelope.get("function"),
        "status": envelope.get("status"),
        "diagnostic_only": envelope.get("diagnostic_only"),
        "board_admission": envelope.get("board_admission"),
        "exactness_claim": envelope.get("exactness_claim"),
        "event_count": envelope.get("event_count"),
        "events": normalized_events,
        "pcode_color_row_count": len(normalized_pcode),
        "pcode_color_rows": normalized_pcode,
        "source_bindings": sorted(
            ({"source_name": name, "object_kind": kind} for name, kind in bindings.values()),
            key=lambda row: (row["object_kind"], row["source_name"]),
        ),
        "color_nodes": envelope.get("color_nodes"),
        "physical_register_range": envelope.get("physical_register_range"),
        "virtual_register_rejected": envelope.get("virtual_register_rejected"),
        "varinfo_layout": envelope.get("varinfo_layout"),
        "object": _object_identity(envelope.get("object")),
        "semantic_gate": {
            "status": semantic.get("status"),
            "function_count": semantic.get("function_count"),
            "semantic_sections": semantic.get("semantic_sections"),
            "actual_object": _object_identity(semantic.get("actual_object")),
            "expected_object": _object_identity(semantic.get("expected_object")),
        },
    }


def _differences(expected: Any, actual: Any, path: str = "$") -> list[str]:
    if type(expected) is not type(actual):
        return [f"{path}: type {type(expected).__name__} != {type(actual).__name__}"]
    if isinstance(expected, dict):
        result: list[str] = []
        for key in sorted(set(expected) | set(actual)):
            if key not in expected or key not in actual:
                result.append(f"{path}.{key}: missing on {'expected' if key not in expected else 'actual'}")
            else:
                result.extend(_differences(expected[key], actual[key], f"{path}.{key}"))
            if len(result) >= 64:
                break
        return result
    if isinstance(expected, list):
        if len(expected) != len(actual):
            return [f"{path}: length {len(expected)} != {len(actual)}"]
        result = []
        for index, (left, right) in enumerate(zip(expected, actual)):
            result.extend(_differences(left, right, f"{path}[{index}]"))
            if len(result) >= 64:
                break
        return result
    return [] if expected == actual else [f"{path}: {expected!r} != {actual!r}"]


def compare(package_root: Path, output: Path) -> dict[str, Any]:
    if not package_root.exists() or package_root.is_symlink() or os.path.islink(package_root) or not package_root.is_dir():
        _fail(f"historical replay package root is not a regular non-symlink directory: {package_root}")
    package_root = package_root.resolve(strict=True)
    plan_path = package_root / "historical-replay-plan.json"
    plan = _read_json(plan_path, label="historical replay plan")
    receipt = _read_json(package_root / "package-receipt.json", label="historical replay receipt")
    if plan.get("schema") != PLAN_SCHEMA or receipt.get("schema") != RECEIPT_SCHEMA:
        _fail("historical replay package schema differs")
    for label, value in (("plan", plan.get("package_root")), ("receipt", receipt.get("package_root"))):
        if not isinstance(value, str) or not Path(value).is_absolute():
            _fail(f"historical replay {label} package_root is not absolute")
        try:
            bound_root = Path(value).resolve(strict=True)
        except OSError as exc:
            _fail(f"historical replay {label} package_root is unavailable: {exc}")
        if bound_root != package_root:
            _fail(f"historical replay {label} package_root differs")
    if receipt.get("plan_sha256") != hashlib.sha256(_canonical(plan)).hexdigest():
        _fail("historical replay plan digest differs")
    request_binding = plan.get("request")
    if not isinstance(request_binding, Mapping):
        _fail("historical replay request binding is missing")
    request_value = request_binding.get("path")
    if not isinstance(request_value, str) or not Path(request_value).is_absolute():
        _fail("historical replay request path is not absolute")
    request_path = Path(request_value)
    if request_path.resolve(strict=True) != package_root / "backend-request.json":
        _fail("historical replay request path differs")
    request_digest = request_binding.get("sha256")
    request_value_parsed = _read_json(request_path, label="historical replay request")
    if (
        request_digest != request_value_parsed.get("request_sha256")
        or request_digest != _unsigned_request_digest(request_value_parsed)
        or receipt.get("request_sha256") != request_digest
    ):
        _fail("historical replay request digest differs")
    if receipt.get("diagnostic_only") is not True or receipt.get("authority_advanced") is not False:
        _fail("historical replay receipt admission flags are open")
    results: dict[str, Any] = {}
    all_match = True
    for lane in LANES:
        lane_plan = plan.get("lanes", {}).get(lane)
        if not isinstance(lane_plan, Mapping):
            _fail(f"{lane} replay lane is missing")
        baseline_path = _validate_descriptor(lane_plan.get("historical_envelope"), label=f"{lane} historical envelope")
        actual_path = package_root / "backend-output" / lane / "physical-reg.envelope.json"
        baseline = _read_json(baseline_path, label=f"{lane} historical envelope")
        actual = _read_json(actual_path, label=f"{lane} replay envelope")
        expected_object = lane_plan.get("expected_object")
        if not isinstance(expected_object, Mapping):
            _fail(f"{lane} expected object binding is missing")
        _validate_physical_envelope(baseline_path, lane=lane, expected_object=expected_object)
        _validate_physical_envelope(actual_path, lane=lane, expected_object=expected_object)
        expected_norm = normalize_envelope(baseline)
        actual_norm = normalize_envelope(actual)
        diffs = _differences(expected_norm, actual_norm)
        match = not diffs
        all_match = all_match and match
        results[lane] = {
            "status": "MATCH" if match else "MISMATCH",
            "historical": _descriptor(baseline_path),
            "replay": _descriptor(actual_path),
            "normalized_sha256": {
                "historical": hashlib.sha256(_canonical(expected_norm)).hexdigest(),
                "replay": hashlib.sha256(_canonical(actual_norm)).hexdigest(),
            },
            "event_count": actual_norm["event_count"],
            "pcode_color_row_count": actual_norm["pcode_color_row_count"],
            "differences": diffs,
        }
    comparison = {
        "schema": COMPARISON_SCHEMA,
        "diagnostic_only": True,
        "authority_advanced": False,
        "board_admission": False,
        "exactness_claim": False,
        "status": "MATCH" if all_match else "MISMATCH",
        "ignored_run_local_fields": list(IGNORED_RUN_LOCAL_FIELDS),
        "lanes": results,
    }
    _write_exclusive(output, comparison)
    return comparison


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    prepare_parser = sub.add_parser("prepare", help="materialize a closed replay plan; never compile")
    prepare_parser.add_argument("--request", required=True, type=Path)
    prepare_parser.add_argument("--output-root", required=True, type=Path)
    compare_parser = sub.add_parser("compare", help="compare replay outputs against sealed v523e envelopes")
    compare_parser.add_argument("--package-root", required=True, type=Path)
    compare_parser.add_argument("--output", required=True, type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        if args.mode == "prepare":
            result = prepare(args.request, args.output_root)
        else:
            result = compare(args.package_root, args.output)
        print(json.dumps({"status": result["status"], "session_id": result.get("session_id")}, sort_keys=True))
        return 0 if result["status"] != "MISMATCH" else 3
    except ReplayError as exc:
        print(f"REJECTED: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
