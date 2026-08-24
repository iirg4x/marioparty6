#!/usr/bin/env python3
"""Compose a read-only, fail-closed causal map for one recovery owner.

The command deliberately delegates report interpretation and workbench history
to the installed matching tools.  It only binds their contexts, joins their
outputs, and makes missing lanes explicit as UNKNOWN.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from collections import Counter
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any


if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


REQUEST_SCHEMA = "board_owner_causal_map_request/v1"
OUTPUT_SCHEMA = "board_owner_causal_map/v1"
SCHEMA_VERSION = 1
MAX_TRACER_RECEIPTS = 32
MAX_GRAPH_LOCATIONS = 4096
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


class CausalMapError(ValueError):
    """An input or component context cannot be joined safely."""


def _fail(message: str) -> None:
    raise CausalMapError(message)


def _pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            _fail(f"duplicate JSON key {key!r}")
        result[key] = value
    return result


def _canonical(value: Any) -> bytes:
    try:
        return (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise CausalMapError(f"cannot serialize canonical JSON: {exc}") from exc


def _digest(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _closed(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        _fail(f"{label} must be an object")
    unknown = sorted(set(value) - allowed)
    if unknown:
        _fail(f"{label} contains unknown field {unknown[0]!r}")
    missing = sorted(required - set(value))
    if missing:
        _fail(f"{label} lacks required field {missing[0]!r}")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        _fail(f"{label} must be non-empty text")
    return value.strip()


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        _fail(f"{label} must be an integer >= {minimum}")
    return value


def _sha256(value: Any, label: str) -> str:
    digest = _text(value, label)
    if not SHA256_RE.fullmatch(digest):
        _fail(f"{label} must be a lowercase SHA-256")
    return digest


def _string_array(
    value: Any,
    label: str,
    *,
    maximum: int,
    allow_empty: bool = False,
) -> list[str]:
    if not isinstance(value, list) or (not value and not allow_empty):
        _fail(f"{label} must be a{' possibly empty' if allow_empty else ' non-empty'} array")
    if len(value) > maximum:
        _fail(f"{label} exceeds the limit of {maximum}")
    result = [_text(item, f"{label}[{index}]") for index, item in enumerate(value)]
    if len(set(result)) != len(result):
        _fail(f"{label} contains duplicates")
    return result


def _resolve(root: Path, value: Any, label: str) -> Path:
    raw = _text(value, label)
    path = Path(raw).expanduser()
    if not path.is_absolute():
        path = root / path
    return path.resolve()


def _parse_descriptor(
    value: Any,
    root: Path,
    label: str,
    *,
    extra_allowed: set[str] | None = None,
    extra_required: set[str] | None = None,
) -> dict[str, Any]:
    extras = extra_allowed or set()
    required_extras = extra_required or set()
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256"} | extras,
        required={"path", "size_bytes", "sha256"} | required_extras,
        label=label,
    )
    path = _resolve(root, item.get("path"), f"{label}.path")
    expected_size = _integer(item.get("size_bytes"), f"{label}.size_bytes")
    expected_sha = _sha256(item.get("sha256"), f"{label}.sha256")
    try:
        size = path.stat().st_size
        digest = _sha256_file(path)
    except OSError as exc:
        raise CausalMapError(f"cannot read {label} {path}: {exc}") from exc
    if size != expected_size or digest != expected_sha:
        _fail(f"{label} does not match its declared size/SHA-256: {path}")
    result = {"path": os.fspath(path), "size_bytes": size, "sha256": digest}
    for key in sorted(extras):
        if key in item:
            result[key] = item[key]
    return result


def _parse_optional_descriptor(
    value: Any, root: Path, label: str
) -> dict[str, Any] | None:
    if value is None:
        return None
    return _parse_descriptor(value, root, label)


def _load_request(path: Path, root: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    try:
        raw = path.read_text(encoding="utf-8")
        value = json.loads(raw, object_pairs_hook=_pairs)
    except FileNotFoundError as exc:
        raise CausalMapError(f"request does not exist: {path}") from exc
    except UnicodeDecodeError as exc:
        raise CausalMapError(f"request is not UTF-8: {path}") from exc
    except json.JSONDecodeError as exc:
        raise CausalMapError(
            f"invalid request JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    request = _closed(
        value,
        allowed={
            "schema",
            "schema_version",
            "owner",
            "source",
            "target",
            "compiler",
            "report",
            "workbench",
            "interaction_request",
            "target_assembly",
            "candidate_assembly",
            "donor_registry",
            "tracer_receipts",
            "graph",
            "telemetry",
        },
        required={
            "schema",
            "schema_version",
            "owner",
            "source",
            "target",
            "compiler",
            "report",
            "workbench",
            "interaction_request",
        },
        label="causal-map request",
    )
    if request.get("schema") != REQUEST_SCHEMA or request.get("schema_version") != 1:
        _fail(f"causal-map request schema must be {REQUEST_SCHEMA} version 1")

    source = _parse_descriptor(
        request.get("source"),
        root,
        "request.source",
        extra_allowed={"candidate_id"},
        extra_required={"candidate_id"},
    )
    source["candidate_id"] = _text(source["candidate_id"], "request.source.candidate_id")
    target = _parse_descriptor(request.get("target"), root, "request.target")
    report = _parse_descriptor(
        request.get("report"),
        root,
        "request.report",
        extra_allowed={"kind"},
        extra_required={"kind"},
    )
    report_kind = _text(report["kind"], "request.report.kind")
    if report_kind not in {"strict", "data"}:
        _fail("request.report.kind must be 'strict' or 'data'")
    report["kind"] = report_kind

    compiler_value = _closed(
        request.get("compiler"),
        allowed={"toolchain_key", "compiler_sha256", "context_sha256"},
        required={"toolchain_key", "compiler_sha256", "context_sha256"},
        label="request.compiler",
    )
    compiler = {
        "toolchain_key": _text(
            compiler_value.get("toolchain_key"), "request.compiler.toolchain_key"
        ),
        "compiler_sha256": _sha256(
            compiler_value.get("compiler_sha256"), "request.compiler.compiler_sha256"
        ),
        "context_sha256": _sha256(
            compiler_value.get("context_sha256"), "request.compiler.context_sha256"
        ),
    }

    workbench_value = _closed(
        request.get("workbench"),
        allowed={"path", "session_id", "session_sha256"},
        required={"path", "session_id", "session_sha256"},
        label="request.workbench",
    )
    workbench = {
        "path": os.fspath(
            _resolve(root, workbench_value.get("path"), "request.workbench.path")
        ),
        "session_id": _text(
            workbench_value.get("session_id"), "request.workbench.session_id"
        ),
        "session_sha256": _sha256(
            workbench_value.get("session_sha256"), "request.workbench.session_sha256"
        ),
    }

    telemetry_value = request.get("telemetry", {})
    telemetry_item = _closed(
        telemetry_value,
        allowed={"elapsed_seconds", "active_seconds", "tracer_runs", "donor_searches"},
        required=set(),
        label="request.telemetry",
    )
    telemetry: dict[str, int | float | None] = {}
    for key in ("elapsed_seconds", "active_seconds"):
        item = telemetry_item.get(key)
        if item is not None and (
            isinstance(item, bool) or not isinstance(item, (int, float)) or item <= 0
        ):
            _fail(f"request.telemetry.{key} must be a positive number or null")
        telemetry[key] = item
    for key in ("tracer_runs", "donor_searches"):
        item = telemetry_item.get(key)
        telemetry[key] = None if item is None else _integer(
            item, f"request.telemetry.{key}"
        )

    receipts_value = request.get("tracer_receipts", [])
    if not isinstance(receipts_value, list) or len(receipts_value) > MAX_TRACER_RECEIPTS:
        _fail(f"request.tracer_receipts must be an array of at most {MAX_TRACER_RECEIPTS}")
    receipts: list[dict[str, Any]] = []
    for index, raw_receipt in enumerate(receipts_value):
        receipt = _parse_descriptor(
            raw_receipt,
            root,
            f"request.tracer_receipts[{index}]",
            extra_allowed={"focus_symbols"},
            extra_required={"focus_symbols"},
        )
        receipt["focus_symbols"] = _string_array(
            receipt["focus_symbols"],
            f"request.tracer_receipts[{index}].focus_symbols",
            maximum=4096,
        )
        receipts.append(receipt)

    graph = _parse_graph_request(request.get("graph"), root)
    normalized = {
        "schema": REQUEST_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "owner": _text(request.get("owner"), "request.owner"),
        "source": source,
        "target": target,
        "compiler": compiler,
        "report": report,
        "workbench": workbench,
        "interaction_request": _parse_descriptor(
            request.get("interaction_request"), root, "request.interaction_request"
        ),
        "target_assembly": _parse_optional_descriptor(
            request.get("target_assembly"), root, "request.target_assembly"
        ),
        "candidate_assembly": _parse_optional_descriptor(
            request.get("candidate_assembly"), root, "request.candidate_assembly"
        ),
        "donor_registry": _parse_optional_descriptor(
            request.get("donor_registry"), root, "request.donor_registry"
        ),
        "tracer_receipts": receipts,
        "graph": graph,
        "telemetry": telemetry,
    }
    request_descriptor = {
        "path": os.fspath(path.resolve()),
        "size_bytes": len(raw.encode("utf-8")),
        "sha256": hashlib.sha256(raw.encode("utf-8")).hexdigest(),
    }
    return normalized, request_descriptor


def _parse_graph_request(value: Any, root: Path) -> dict[str, Any] | None:
    if value is None:
        return None
    item = _closed(
        value,
        allowed={"path", "size_bytes", "sha256", "source_locations"},
        required={"path", "size_bytes", "sha256", "source_locations"},
        label="request.graph",
    )
    descriptor = _parse_descriptor(
        {key: item[key] for key in ("path", "size_bytes", "sha256")},
        root,
        "request.graph",
    )
    locations_value = item.get("source_locations")
    if not isinstance(locations_value, list) or len(locations_value) > MAX_GRAPH_LOCATIONS:
        _fail(
            f"request.graph.source_locations must be an array of at most {MAX_GRAPH_LOCATIONS}"
        )
    locations: list[dict[str, str]] = []
    seen: set[tuple[str, str]] = set()
    for index, raw in enumerate(locations_value):
        row = _closed(
            raw,
            allowed={
                "function",
                "node_id",
                "node_label",
                "source_file",
                "source_location",
            },
            required={
                "function",
                "node_id",
                "node_label",
                "source_file",
                "source_location",
            },
            label=f"request.graph.source_locations[{index}]",
        )
        normalized = {
            key: _text(row.get(key), f"request.graph.source_locations[{index}].{key}")
            for key in (
                "function",
                "node_id",
                "node_label",
                "source_file",
                "source_location",
            )
        }
        key = (normalized["function"], normalized["node_id"])
        if key in seen:
            _fail("request.graph.source_locations contains duplicate function/node pairs")
        seen.add(key)
        locations.append(normalized)
    descriptor["source_locations"] = sorted(
        locations, key=lambda row: (row["function"], row["node_id"])
    )
    return descriptor


def _descriptor_matches(actual: Mapping[str, Any], expected: Mapping[str, Any]) -> bool:
    try:
        return (
            Path(str(actual.get("path"))).resolve() == Path(str(expected.get("path"))).resolve()
            and int(actual.get("size_bytes")) == int(expected.get("size_bytes"))
            and str(actual.get("sha256")) == str(expected.get("sha256"))
        )
    except (TypeError, ValueError, OSError):
        return False


def _component_authority(value: Mapping[str, Any], label: str) -> None:
    if value.get("authority_advanced") is not False:
        _fail(f"{label} did not preserve authority_advanced=false")


def _load_workbench_context(
    root: Path, request: Mapping[str, Any]
) -> tuple[Mapping[str, Any], Mapping[str, Any], list[Mapping[str, Any]], dict[str, Any]]:
    from tools import match_workbench as workbench

    workspace = workbench._workspace(request["workbench"]["path"], root)
    session = workbench._load_session(workspace, root)
    if session.get("session_id") != request["workbench"]["session_id"]:
        _fail("request workbench session_id does not match the immutable session")
    if session.get("session_sha256") != request["workbench"]["session_sha256"]:
        _fail("request workbench session_sha256 does not match the immutable session")
    session_request = session.get("request")
    if not isinstance(session_request, Mapping):
        _fail("workbench session request is unavailable")
    if session_request.get("owner") != request["owner"]:
        _fail("request owner does not match the workbench owner")
    target = session_request.get("target")
    if not isinstance(target, Mapping) or not _descriptor_matches(target, request["target"]):
        _fail("request target identity does not match the workbench target")
    context = session_request.get("context")
    if not isinstance(context, Mapping) or context.get("context_complete") is not True:
        _fail("workbench compiler context is incomplete")
    projected = workbench._compile_context_projection(context, "owner causal-map context")
    compiler = projected.get("compiler")
    if not isinstance(compiler, Mapping):
        _fail("workbench session lacks an authenticated compiler descriptor")
    if projected.get("toolchain_key") != request["compiler"]["toolchain_key"]:
        _fail("request toolchain identity does not match the workbench context")
    if compiler.get("sha256") != request["compiler"]["compiler_sha256"]:
        _fail("request compiler identity does not match the workbench context")
    context_sha = workbench._compile_context_sha256(
        context, "owner causal-map compiler context"
    )
    if context_sha != request["compiler"]["context_sha256"]:
        _fail("request compiler context hash does not match the workbench context")

    index = workbench._load_index(workspace, session)
    candidates = [
        workbench._load_candidate(workspace, candidate_id, session)
        for candidate_id in sorted(index["candidates"])
    ]
    for candidate in candidates:
        workbench._require_candidate_compile_attestation(candidate, session)
    current = next(
        (
            candidate
            for candidate in candidates
            if candidate.get("candidate_id") == request["source"]["candidate_id"]
        ),
        None,
    )
    if current is None:
        _fail("request source candidate_id is absent from the workbench")
    source = current.get("source")
    if not isinstance(source, Mapping) or not _descriptor_matches(source, request["source"]):
        _fail("request source identity does not match the selected candidate")
    report = current.get("reports", {}).get(request["report"]["kind"])
    if not isinstance(report, Mapping):
        _fail("selected candidate lacks the requested report kind")
    if (
        report.get("raw_sha256") != request["report"]["sha256"]
        or report.get("raw_size_bytes") != request["report"]["size_bytes"]
    ):
        _fail("request report identity does not match the selected candidate report")

    matrix = workbench.build_matrix(root, workspace)
    _component_authority(matrix, "match workbench matrix")
    if matrix.get("session_id") != session.get("session_id"):
        _fail("matrix belongs to a different workbench session")
    current_rows = [
        row
        for row in matrix.get("rows", [])
        if isinstance(row, Mapping)
        and row.get("candidate_id") == current.get("candidate_id")
    ]
    if len(current_rows) != 1:
        _fail("matrix does not contain exactly one selected candidate row")
    if (
        current_rows[0].get("source_sha256") != request["source"]["sha256"]
        or current_rows[0].get("object_sha256") != current.get("object", {}).get("sha256")
    ):
        _fail("matrix selected candidate identity is inconsistent")
    return session, current, candidates, matrix


def _validate_graph(
    request: Mapping[str, Any], all_functions: set[str]
) -> tuple[dict[str, list[dict[str, str]]], dict[str, Any]]:
    graph = request.get("graph")
    if graph is None:
        return {}, _lane("UNKNOWN", "no explicitly bound Graphify evidence was supplied")
    try:
        value = json.loads(Path(graph["path"]).read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise CausalMapError(f"cannot load bound Graphify graph {graph['path']}: {exc}") from exc
    if not isinstance(value, Mapping) or not isinstance(value.get("nodes"), list):
        _fail("bound Graphify graph lacks a nodes array")
    nodes: dict[str, Mapping[str, Any]] = {}
    for index, node in enumerate(value["nodes"]):
        if not isinstance(node, Mapping):
            _fail(f"bound Graphify node {index} is not an object")
        node_id = node.get("id")
        if isinstance(node_id, str) and node_id:
            if node_id in nodes:
                _fail(f"bound Graphify graph contains duplicate node id {node_id!r}")
            nodes[node_id] = node
    by_function: dict[str, list[dict[str, str]]] = {}
    for row in graph["source_locations"]:
        function = row["function"]
        if function not in all_functions:
            _fail(f"Graphify source location names unknown report function {function!r}")
        node = nodes.get(row["node_id"])
        if node is None:
            _fail(f"Graphify source location names missing node {row['node_id']!r}")
        for request_key, graph_key in (
            ("node_label", "label"),
            ("source_file", "source_file"),
            ("source_location", "source_location"),
        ):
            if node.get(graph_key) != row[request_key]:
                _fail(
                    f"Graphify node {row['node_id']!r} {graph_key} does not match the request"
                )
        by_function.setdefault(function, []).append(dict(row))
    status = "KNOWN" if graph["source_locations"] else "UNKNOWN"
    reason = (
        "all included locations were matched to exact nodes in the bound graph"
        if graph["source_locations"]
        else "a graph was bound but no source locations were declared"
    )
    return by_function, _lane(status, reason)


def _validate_tracer_receipts(
    request: Mapping[str, Any], all_functions: set[str]
) -> tuple[dict[str, list[dict[str, Any]]], dict[str, Any]]:
    from tools.mwcc_fe_chronology import ChronologyError, load_report

    receipts = request.get("tracer_receipts", [])
    if not receipts:
        return {}, _lane(
            "UNKNOWN",
            "no source-aware tracer receipt was supplied; native producer checkpoint A is unavailable",
        )
    by_function: dict[str, list[dict[str, Any]]] = {}
    for descriptor in receipts:
        try:
            report = load_report(descriptor["path"])
        except ChronologyError as exc:
            raise CausalMapError(f"tracer receipt rejected: {exc}") from exc
        provenance = report["provenance"]
        if provenance["source_sha256"] != request["source"]["sha256"]:
            _fail("tracer receipt source hash does not match the bound source")
        if provenance["compiler_sha256"] != request["compiler"]["compiler_sha256"]:
            _fail("tracer receipt compiler hash does not match the bound compiler")
        compact = {
            "report": {
                key: descriptor[key] for key in ("path", "size_bytes", "sha256")
            },
            "producer": report["producer"],
            "provenance": provenance,
            "object_count": len(report["objects"]),
            "object_uids": sorted(str(item["uid"]) for item in report["objects"]),
            "authenticated_home_candidate_count": sum(
                len(item["home_join"]["candidates"]) for item in report["objects"]
            ),
        }
        for function in descriptor["focus_symbols"]:
            if function not in all_functions:
                _fail(f"tracer receipt names unknown report function {function!r}")
            by_function.setdefault(function, []).append(compact)
    return by_function, _lane(
        "BLOCKED",
        "consumer-side chronology receipts validated, but their required native producer remains blocked",
    )


def _lane(status: str, reason: str) -> dict[str, str]:
    if status not in {"KNOWN", "PARTIAL", "UNKNOWN", "BLOCKED"}:
        _fail(f"invalid coverage status {status!r}")
    return {"status": status, "reason": reason}


def _focus_rows(matrix: Mapping[str, Any], symbol: str) -> list[Mapping[str, Any]]:
    from tools import match_workbench as workbench

    return [
        row
        for row in matrix.get("rows", [])
        if isinstance(row, Mapping)
        and (
            workbench._matrix_focus_row(row, symbol, "strict_focus") is not None
            or workbench._matrix_focus_row(row, symbol, "data_focus") is not None
        )
    ]


def _rejected_axes(rows: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    from tools import match_workbench as workbench

    result = []
    for row in rows:
        if not workbench._candidate_is_no_go(row):
            continue
        outcome = row.get("outcome") if isinstance(row.get("outcome"), Mapping) else {}
        result.append(
            {
                "ordinal": row.get("ordinal"),
                "candidate_id": row.get("candidate_id"),
                "axis": row.get("hypothesis_axis"),
                "axis_fingerprint": row.get("axis_fingerprint"),
                "status": outcome.get("status"),
                "reason": outcome.get("reason"),
                "source_sha256": row.get("source_sha256"),
                "object_sha256": row.get("object_sha256"),
            }
        )
    return sorted(result, key=lambda row: (int(row.get("ordinal") or 0), str(row.get("candidate_id"))))


def _earliest_cause(cascade: Mapping[str, Any], symbol: str) -> dict[str, Any]:
    audit = cascade.get("audit")
    if not isinstance(audit, Mapping):
        _fail(f"causal reducer output for {symbol!r} lacks an audit")
    functions = [
        row
        for row in audit.get("functions", [])
        if isinstance(row, Mapping) and row.get("function") == symbol
    ]
    if len(functions) != 1:
        _fail(f"causal reducer output for {symbol!r} is not an unambiguous function")
    candidates = [
        row
        for field in ("clusters", "patterns")
        for row in functions[0].get(field, [])
        if isinstance(row, Mapping)
    ]
    if not candidates:
        return {
            "status": "UNKNOWN",
            "reason": "no supported structural residual classification was emitted",
        }
    candidates.sort(
        key=lambda row: (
            row.get("target_address_start") is None,
            int(row.get("target_address_start") or 0),
            int(row.get("index_start") or 0),
            str(row.get("classification", "")),
        )
    )
    first = candidates[0]
    result = {
        "status": "KNOWN" if first.get("classification") != "unknown" else "UNKNOWN",
        "classification": first.get("classification"),
        "confidence": first.get("confidence"),
        "index_start": first.get("index_start"),
        "index_end": first.get("index_end"),
        "target_address_start": first.get("target_address_start"),
        "target_address_end": first.get("target_address_end"),
        "candidate_address_start": first.get("candidate_address_start"),
        "candidate_address_end": first.get("candidate_address_end"),
        "diff_pair_count": first.get("diff_pair_count"),
        "recommended_source_axis": first.get("recommended_source_axis"),
        "evidence": first.get("evidence"),
    }
    return result


def _relocation_evidence(
    metric: Mapping[str, Any], cascade: Mapping[str, Any], pool: Mapping[str, Any]
) -> dict[str, Any]:
    report_count = sum(
        int(count)
        for kind, count in metric.get("diff_kinds", {}).items()
        if "RELOC" in str(kind).upper()
    )
    audit = cascade.get("audit", {})
    clusters = [
        cluster
        for function in audit.get("functions", [])
        if isinstance(function, Mapping)
        for cluster in function.get("clusters", [])
        if isinstance(cluster, Mapping)
    ]
    causal_count = sum(
        bool(cluster.get("evidence", {}).get("relocation_signal"))
        for cluster in clusters
        if isinstance(cluster.get("evidence"), Mapping)
    )
    decoded = pool.get("decode", {})
    return {
        "report_diff_count": report_count,
        "causal_relocation_cluster_count": causal_count,
        "causal_signal": bool(causal_count),
        "target_pool_consumer_count": decoded.get("target", {}).get("pool_consumer_count"),
        "candidate_pool_consumer_count": decoded.get("candidate", {}).get("pool_consumer_count"),
        "pool_classification_counts": decoded.get("summary", {}).get(
            "classification_counts", {}
        ),
        "physical_relocation_authority": "UNKNOWN",
    }


def _pool_evidence(pool: Mapping[str, Any]) -> dict[str, Any]:
    decoded = pool.get("decode")
    if not isinstance(decoded, Mapping):
        _fail("typed pool decoder output lacks decode evidence")
    return {
        "summary": decoded.get("summary"),
        "groups": decoded.get("groups", []),
        "groups_omitted": decoded.get("groups_omitted", 0),
        "pool_decoder_sha256": pool.get("pool_decoder_sha256"),
    }


def _stack_evidence(stack: Mapping[str, Any]) -> dict[str, Any]:
    return {
        "target_instruction_count": stack.get("target_instruction_count"),
        "target_stack_access_count": stack.get("target_stack_access_count"),
        "stack_slot_count": stack.get("stack_slot_count"),
        "zero_read_slot_count": stack.get("zero_read_slot_count"),
        "excluded_stack_access_count": stack.get("excluded_stack_access_count"),
        "outgoing_call_argument_access_count": stack.get(
            "outgoing_call_argument_access_count"
        ),
        "zero_read_slots": stack.get("zero_read_slots", []),
        "evidence_sha256": _digest(stack),
    }


def _validate_plan(
    plan: Mapping[str, Any],
    residual_symbols: set[str],
    matrix: Mapping[str, Any],
) -> list[dict[str, Any]]:
    _component_authority(plan, "factorial interaction planner")
    if set(plan.get("focus_symbols", [])) != residual_symbols:
        _fail("interaction plan focus_symbols do not cover exactly the residual functions")
    matrix_rows = {
        str(row.get("candidate_id")): row
        for row in matrix.get("rows", [])
        if isinstance(row, Mapping)
    }
    cells = {
        str(cell.get("cell_id")): cell
        for cell in plan.get("cells", [])
        if isinstance(cell, Mapping)
    }
    for cell in cells.values():
        observation = cell.get("observation")
        if not isinstance(observation, Mapping):
            continue
        candidate_id = str(observation.get("candidate_id"))
        row = matrix_rows.get(candidate_id)
        if row is None:
            _fail(f"interaction observation names unknown matrix candidate {candidate_id!r}")
        if (
            observation.get("source_sha256") != row.get("source_sha256")
            or observation.get("object_sha256") != row.get("object_sha256")
        ):
            _fail(f"interaction observation context mismatch for {candidate_id!r}")
    axes = {
        str(axis.get("id")): axis
        for axis in plan.get("axes", [])
        if isinstance(axis, Mapping)
    }
    ranked: list[dict[str, Any]] = []
    for rank, cell_id in enumerate(plan.get("recommended_execution_order", []), 1):
        cell = cells.get(str(cell_id))
        if cell is None or cell.get("action") != "generate_and_compile":
            _fail("interaction recommended_execution_order contains an invalid cell")
        closure = []
        for axis_id, level_id in sorted(cell.get("selection", {}).items()):
            axis = axes.get(str(axis_id))
            if axis is None:
                _fail(f"interaction cell references unknown axis {axis_id!r}")
            levels = {
                str(level.get("id")): level
                for level in axis.get("levels", [])
                if isinstance(level, Mapping)
            }
            level = levels.get(str(level_id))
            if level is None:
                _fail(f"interaction cell references unknown level {axis_id}={level_id}")
            closure.append(
                {
                    "axis": axis_id,
                    "hypothesis": axis.get("hypothesis"),
                    "control_level": axis.get("control_level"),
                    "selected_level": level_id,
                    "source_action": level.get("source_action"),
                    "evidence": level.get("evidence", []),
                    "admissibility": level.get("admissibility"),
                }
            )
        ranked.append(
            {
                "rank": rank,
                "cell_id": cell_id,
                "interaction_order": cell.get("interaction_order"),
                "selection": cell.get("selection"),
                "dependency_closure": closure,
            }
        )
    return ranked


def build_causal_map(root: Path, request_path: Path | str) -> dict[str, Any]:
    """Build one deterministic owner map without writing workbench state."""

    from tools import match_workbench as workbench

    root = root.expanduser().resolve()
    path = Path(request_path).expanduser()
    if not path.is_absolute():
        path = root / path
    request, request_descriptor = _load_request(path.resolve(), root)
    session, current, candidates, matrix = _load_workbench_context(root, request)

    _, report_descriptor, report_value = workbench._assessment_file(
        request["report"]["path"], root, "owner causal-map report"
    )
    if not _descriptor_matches(report_descriptor, request["report"]):
        _fail("live report descriptor changed during causal-map preparation")
    records, counts = workbench._assessment_records(
        report_value, "owner causal-map report"
    )
    function_occurrences = Counter(str(record["name"]) for record in records)
    all_functions = {str(record["name"]) for record in records}
    residual_records = [record for record in records if not record["metric"]["exact"]]
    residual_symbols = {str(record["name"]) for record in residual_records}

    graph_by_function, graph_lane = _validate_graph(request, all_functions)
    tracer_by_function, tracer_lane = _validate_tracer_receipts(
        request, all_functions
    )

    interaction_plan = workbench.plan_candidate_interactions(
        root, request=request["interaction_request"]["path"]
    )
    ranked_next_axes = _validate_plan(interaction_plan, residual_symbols, matrix)

    donor_context: dict[str, Any] | None = None
    donor_by_function: dict[str, Mapping[str, Any]] = {}
    if request["donor_registry"] is not None:
        donor_context = {
            "descriptor": request["donor_registry"],
            "registry_sha256": None,
            "global_rejections": [],
        }
        for symbol in sorted(residual_symbols):
            listing = workbench.list_donor_shapes(
                root,
                request["donor_registry"]["path"],
                focus_symbol=symbol,
                include_rejections=True,
            )
            _component_authority(listing, f"donor registry listing for {symbol}")
            if donor_context["registry_sha256"] is None:
                donor_context["registry_sha256"] = listing.get("registry_sha256")
                donor_context["global_rejections"] = listing.get("rejections", [])
            elif donor_context["registry_sha256"] != listing.get("registry_sha256"):
                _fail("donor registry changed while the causal map was built")
            donor_by_function[symbol] = listing

    functions: list[dict[str, Any]] = []
    telemetry_known = 0
    structural_known = 0
    for record in residual_records:
        symbol = str(record["name"])
        occurrence = int(record["occurrence"])
        ambiguous_symbol = function_occurrences[symbol] != 1
        metric = workbench._residual_metric(record["metric"])
        rows = _focus_rows(matrix, symbol)
        telemetry: Mapping[str, Any] | None = None
        # A symbol-only telemetry query cannot distinguish duplicate report
        # occurrences.  Keep that lane UNKNOWN instead of attaching shared
        # history to either occurrence as if it were unambiguous.
        if rows and not ambiguous_symbol:
            telemetry = workbench.build_function_telemetry(
                root,
                request["workbench"]["path"],
                focus_symbol=symbol,
                elapsed_seconds=request["telemetry"]["elapsed_seconds"],
                active_seconds=request["telemetry"]["active_seconds"],
                tracer_runs=request["telemetry"]["tracer_runs"],
                donor_searches=request["telemetry"]["donor_searches"],
            )
            _component_authority(telemetry, f"telemetry for {symbol}")
            telemetry_known += 1

        base = {
            "identity": record["identity"],
            "symbol": symbol,
            "occurrence": occurrence,
            "metrics": metric,
            "rejected_axes": _rejected_axes(rows),
            "donor_context": (
                {
                    "records": donor_by_function[symbol].get("records", []),
                    "record_count": donor_by_function[symbol].get("record_count", 0),
                    "registry_sha256": donor_by_function[symbol].get("registry_sha256"),
                }
                if symbol in donor_by_function
                else {"status": "UNKNOWN", "reason": "no donor registry was supplied"}
            ),
            "tracer_receipts": tracer_by_function.get(symbol, []),
            "graph_source_locations": graph_by_function.get(symbol, []),
            "telemetry": (
                telemetry
                if telemetry is not None
                else {
                    "status": "UNKNOWN",
                    "reason": "no indexed workbench focus history covers this function",
                }
            ),
        }
        if ambiguous_symbol or not metric.get("paired"):
            reason = (
                "duplicate symbol occurrences make focus evidence ambiguous"
                if ambiguous_symbol
                else "function is unpaired in the bound objdiff report"
            )
            base.update(
                {
                    "earliest_structural_cause": {"status": "UNKNOWN", "reason": reason},
                    "relocations": {"status": "UNKNOWN", "reason": reason},
                    "pool": {"status": "UNKNOWN", "reason": reason},
                    "stack": {"status": "UNKNOWN", "reason": reason},
                    "component_hashes": {
                        "causal_reducer_sha256": None,
                        "pool_decoder_sha256": None,
                        "stack_evidence_sha256": None,
                        "telemetry_sha256": (
                            telemetry.get("telemetry_sha256") if telemetry else None
                        ),
                    },
                }
            )
            functions.append(base)
            continue

        cascade = workbench.reduce_objdiff_cascades(
            root,
            report=request["report"]["path"],
            focus_symbol=symbol,
            target_assembly=(
                request["target_assembly"]["path"]
                if request["target_assembly"] is not None
                else None
            ),
            candidate_assembly=(
                request["candidate_assembly"]["path"]
                if request["candidate_assembly"] is not None
                else None
            ),
            summary_only=False,
        )
        pool = workbench.decode_pool_ownership(
            root,
            report=request["report"]["path"],
            focus_symbol=symbol,
        )
        stack = workbench.inspect_stack_residue(
            root, report=request["report"]["path"], focus_symbol=symbol
        )
        _component_authority(cascade, f"causal reducer for {symbol}")
        _component_authority(pool, f"pool decoder for {symbol}")
        _component_authority(stack, f"stack residue for {symbol}")
        cause = _earliest_cause(cascade, symbol)
        if cause.get("status") == "KNOWN":
            structural_known += 1
        stack_evidence = _stack_evidence(stack)
        base.update(
            {
                "earliest_structural_cause": cause,
                "relocations": _relocation_evidence(metric, cascade, pool),
                "pool": _pool_evidence(pool),
                "stack": stack_evidence,
                "component_hashes": {
                    "causal_reducer_sha256": cascade.get("causal_reducer_sha256"),
                    "pool_decoder_sha256": pool.get("pool_decoder_sha256"),
                    "stack_evidence_sha256": stack_evidence["evidence_sha256"],
                    "telemetry_sha256": (
                        telemetry.get("telemetry_sha256") if telemetry else None
                    ),
                },
            }
        )
        functions.append(base)

    functions.sort(key=lambda row: (str(row["symbol"]), int(row["occurrence"])))
    residual_count = len(functions)
    graph_known = sum(bool(row["graph_source_locations"]) for row in functions)
    donor_known = sum(
        isinstance(row["donor_context"], Mapping)
        and row["donor_context"].get("record_count", 0) > 0
        for row in functions
    )
    coverage = {
        "residual_inventory": _lane(
            "KNOWN", "every nonexact target-side function in the bound report is inventoried"
        ),
        "matrix": _lane("KNOWN", "complete immutable workbench index validated"),
        "telemetry": _lane(
            "KNOWN" if telemetry_known == residual_count else "PARTIAL" if telemetry_known else "UNKNOWN",
            f"{telemetry_known} of {residual_count} residual functions have indexed focus history",
        ),
        "structural_cause": _lane(
            "KNOWN" if structural_known == residual_count else "PARTIAL" if structural_known else "UNKNOWN",
            f"{structural_known} of {residual_count} residual functions have a supported earliest cause",
        ),
        "tracer": tracer_lane,
        "donors": _lane(
            "KNOWN" if donor_known == residual_count and residual_count else "PARTIAL" if donor_known else "UNKNOWN",
            f"{donor_known} of {residual_count} residual functions have function-scoped donor records",
        ),
        "graph_source_locations": (
            _lane(
                "KNOWN" if graph_known == residual_count and residual_count else "PARTIAL",
                f"{graph_known} of {residual_count} residual functions have exact bound graph locations",
            )
            if graph_known
            else graph_lane
        ),
        "physical_relocations": _lane(
            "UNKNOWN", "objdiff relocation signals are report-derived, not physical relocation proof"
        ),
        "consumer_closure": _lane(
            "UNKNOWN", "the causal map does not run linked consumer or retail closure gates"
        ),
    }

    body = {
        "schema": OUTPUT_SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "owner": request["owner"],
        "status": "residuals_present" if residual_count else "no_residuals",
        "bindings": {
            "request": request_descriptor,
            "source": request["source"],
            "target": request["target"],
            "compiler": request["compiler"],
            "report": request["report"],
            "workbench": {
                **request["workbench"],
                "matrix_sha256": matrix.get("matrix_sha256"),
            },
            "interaction_request": request["interaction_request"],
            "target_assembly": request["target_assembly"],
            "candidate_assembly": request["candidate_assembly"],
            "donor_registry": request["donor_registry"],
            "graph": (
                {
                    key: request["graph"][key]
                    for key in ("path", "size_bytes", "sha256")
                }
                if request["graph"] is not None
                else None
            ),
        },
        "inventory": {
            "target_function_count": counts["total"],
            "exact_target_function_count": counts["exact"],
            "residual_function_count": residual_count,
            "functions": functions,
        },
        "rejected_context": {
            "axis_attempts": _rejected_axes(
                [row for row in matrix.get("rows", []) if isinstance(row, Mapping)]
            ),
            "donor_registry": donor_context,
        },
        "next_axes": ranked_next_axes,
        "coverage": coverage,
        "composition": {
            "matrix_schema": matrix.get("schema"),
            "matrix_sha256": matrix.get("matrix_sha256"),
            "interaction_plan_schema": interaction_plan.get("schema"),
            "interaction_plan_sha256": interaction_plan.get(
                "interaction_plan_sha256"
            ),
            "workbench_session_sha256": session.get("session_sha256"),
            "current_candidate_record_sha256": current.get("record_sha256"),
            "validated_candidate_record_count": len(candidates),
            "delegates": [
                "match_workbench.build_matrix",
                "match_workbench.build_function_telemetry",
                "match_workbench.reduce_objdiff_cascades",
                "match_workbench.decode_pool_ownership",
                "match_workbench.inspect_stack_residue",
                "match_workbench.plan_candidate_interactions",
                "match_workbench.list_donor_shapes",
                "mwcc_fe_chronology.load_report",
            ],
        },
        "limitations": [
            "The map composes diagnostic evidence; it does not authenticate original source shape.",
            "Report-derived relocation and pool signals are not physical relocation or linked-retail proof.",
            "Graphify locations identify indexed source nodes only; they do not prove target equivalence.",
            "UNKNOWN and BLOCKED lanes must not be promoted into positive evidence.",
        ],
        "production_modified": False,
        "authority_advanced": False,
    }
    return {**body, "causal_map_sha256": _digest(body)}


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("request", type=Path, help="closed owner causal-map request JSON")
    parser.add_argument("--root", type=Path, default=Path("."), help="repository root")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = build_causal_map(args.root, args.request)
    except (CausalMapError, OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(result, indent=2, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
