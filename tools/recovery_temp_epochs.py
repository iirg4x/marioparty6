#!/usr/bin/env python3
"""Analyze reused MWCC temporary-counter values from an offline capture.

The native capture is the authority for the counter writes.  This module only
groups exact ``+1`` counter transitions into epochs and labels the three
authenticated GC/2.6 reset sites.  It does not capture, compile, rank source,
or advance recovery authority.
"""
from __future__ import annotations

import argparse
from collections.abc import Mapping, Sequence
from difflib import SequenceMatcher
import hashlib
import json
import os
from pathlib import Path
import re
import sys
from typing import Any


SCHEMA = "recovery_temp_epochs/v1"
COMPARE_SCHEMA = "recovery_temp_epochs_compare/v1"
CAPTURE_SCHEMA = "mwcc_koopa_coin_counter_capture/v1"
EVENT_SCHEMA = "mwcc_kettou_counter_capture_event/v1"
PINNED_COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"
MAX_EVENTS = 2048
MAX_RELATIONSHIPS = 32
MAX_NAMES = 32
MAX_ALLOCATION_GROUPS = 12
MAX_AFFECTED_NODES = 64
MAX_CAPTURE_BYTES = 4 * 1024 * 1024
MAX_DESCRIPTOR_BYTES = 64 * 1024 * 1024
MAX_OUTPUT_BYTES = 1024 * 1024
# Keep comparison reports small enough to print in review tooling.  Totals are
# still retained as scalar counts; only detailed findings are capped.
MAX_COMPARE_GROUPS = 16
MAX_COMPARE_MATCHES = 16
MAX_COMPARE_DIFFERENCES = 16
MAX_COMPARE_RESETS = 16
MAX_COUNTER = 0xFFFFFFFF
RESET_COUNTER_LIMIT = 0x100

_SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
_KNOWN_TRANSITIONS: dict[int, tuple[str, str, str]] = {
    0xFE3BF: (
        "reset_to_saved_base",
        "saved_base",
        "post-store reset current to saved_base after node",
    ),
    0xFE333: (
        "restore_high_water",
        "high_water",
        "post-store restore current to high_water at list end",
    ),
    0x10873E: (
        "phase_normalize_to_default",
        "phase_default",
        "phase normalization current to default",
    ),
}


class CaptureError(ValueError):
    """The supplied capture is not a coherent, bound diagnostic record."""


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise CaptureError(f"{label} must be an object")
    return value


def _string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise CaptureError(f"{label} must be non-empty text")
    return value.strip()


def _integer(value: Any, label: str, *, minimum: int = 0, maximum: int = MAX_COUNTER) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or not minimum <= value <= maximum:
        raise CaptureError(f"{label} must be an integer in [{minimum}, {maximum}]")
    return value


def _sha256(value: Any, label: str) -> str:
    value = _string(value, label)
    if _SHA256_RE.fullmatch(value) is None:
        raise CaptureError(f"{label} must be a lowercase SHA-256")
    return value


def _descriptor(value: Any, label: str) -> dict[str, Any]:
    raw = _mapping(value, label)
    return {
        "path": _string(raw.get("path"), f"{label}.path"),
        "sha256": _sha256(raw.get("sha256"), f"{label}.sha256"),
        "size_bytes": _integer(raw.get("size_bytes"), f"{label}.size_bytes", maximum=MAX_DESCRIPTOR_BYTES),
    }


def _names(value: Any, label: str) -> list[str]:
    if value is None:
        return []
    if isinstance(value, (str, bytes, bytearray)) or not isinstance(value, Sequence):
        raise CaptureError(f"{label} must be a list of names")
    if len(value) > MAX_NAMES:
        raise CaptureError(f"{label} exceeds {MAX_NAMES} names")
    result: list[str] = []
    for index, item in enumerate(value):
        result.append(_string(item, f"{label}[{index}]"))
    return result


def _lane(value: Any, label: str) -> str | None:
    """Normalize an optional native event lane without inventing one."""

    if value is None:
        return None
    return _string(value, label)


def _rva(value: Any, label: str) -> int:
    if isinstance(value, bool):
        raise CaptureError(f"{label} must be a non-negative address")
    if isinstance(value, int):
        result = value
    elif isinstance(value, str):
        text = value.strip()
        try:
            result = int(text, 16 if text.lower().startswith("0x") else 10)
        except ValueError as exc:
            raise CaptureError(f"{label} must be a non-negative address") from exc
    else:
        raise CaptureError(f"{label} must be a non-negative address")
    if not 0 <= result <= MAX_COUNTER:
        raise CaptureError(f"{label} must be a non-negative address")
    return result


def _rva_text(value: int) -> str:
    return f"0x{value:08X}"


def _validate_lane_state(event: Mapping[str, Any], before: int, after: int) -> dict[str, Any]:
    raw = event.get("temporary_lane_state")
    if raw is None:
        return {"status": "values_unobserved", "classification": "native_ip_bound"}
    if not isinstance(raw, Mapping):
        return {"status": "values_unobserved", "classification": "native_ip_bound"}
    state = raw
    status = state.get("status")
    lane = state.get("observed_lane")
    if status != "CAPTURED" or isinstance(lane, bool) or not isinstance(lane, int) or not 0 <= lane < 32:
        return {"status": "values_unobserved", "classification": "native_ip_bound"}
    arrays = state.get("arrays")
    if not isinstance(arrays, Mapping):
        return {"status": "values_unobserved", "classification": "native_ip_bound"}
    values: dict[str, int] = {}
    for name in ("current", "high_water", "saved_base", "phase_default"):
        raw_array = arrays.get(name)
        if isinstance(raw_array, (str, bytes, bytearray)) or not isinstance(raw_array, Sequence):
            return {"status": "values_unobserved", "classification": "native_ip_bound"}
        if lane >= len(raw_array):
            return {"status": "values_unobserved", "classification": "native_ip_bound"}
        try:
            values[name] = _integer(raw_array[lane], f"temporary_lane_state.arrays.{name}[{lane}]")
        except CaptureError:
            return {"status": "values_unobserved", "classification": "native_ip_bound"}
    current_matches = values["current"] == after
    return {
        "status": "observed" if current_matches else "state_mismatch",
        "classification": "lane_state_observed",
        "observed_lane": lane,
        "values": values,
        "current_matches_counter_after": current_matches,
        "counter_before": before,
        "counter_after": after,
    }


def _actual_call_origin(event: Mapping[str, Any]) -> dict[str, Any] | None:
    """Keep native return provenance separate from the enclosing CodeGen node.

    Only follow the callee's unary operand chain: argument names never identify
    the returned value's producer. Physical temporary numbers confer no source
    ranking or retention authority.
    """
    if "actual_call_origin" not in event:
        return None
    label = f"event {event.get('event_id')}.actual_call_origin"
    raw = _mapping(event["actual_call_origin"], label)
    if raw.get("status") != "CAPTURED" or raw.get("binding") != "native_return_allocation_frame":
        raise CaptureError(f"{label} requires CAPTURED native_return_allocation_frame")
    if _rva(event.get("next_instruction_pointer_rva"), label + ".hook") != 0x00128903:
        raise CaptureError(f"{label} appears outside the GC/2.6 return-allocation hook")

    def text(value: Any, field: str) -> str:
        value = _string(value, label + "." + field)
        if len(value) > 256:
            raise CaptureError(f"{label}.{field} exceeds 256 characters")
        return value

    call_token = text(raw.get("call_expression_token"), "call_expression_token")
    result_token = text(raw.get("result_descriptor_token"), "result_descriptor_token")
    result_kind = _integer(raw.get("result_native_kind"), label + ".result_native_kind", maximum=0xFFFF)
    wrapper = _mapping(raw.get("normalized_expression_tree"), label + ".normalized_expression_tree")
    if wrapper.get("schema") != "mwcc_normalized_expression_tree/v1":
        raise CaptureError(f"{label} requires a normalized expression tree")
    tree = _mapping(wrapper.get("tree"), label + ".tree")
    if tree.get("status") != "CAPTURED" or tree.get("native_kind") != 54 or tree.get("token") != call_token:
        raise CaptureError(f"{label} tree must bind the captured kind-54 call token")
    node = _mapping(tree.get("callee"), label + ".callee")
    chain: list[dict[str, Any]] = []
    callee_object = None
    for depth in range(6):
        if node.get("status") != "CAPTURED":
            raise CaptureError(f"{label} callee chain is not CAPTURED")
        kind = _integer(node.get("native_kind"), label + ".callee.native_kind", maximum=0xFFFF)
        chain.append({"token": text(node.get("token"), "callee.token"), "native_kind": kind})
        if "object" in node:
            if kind != 56:
                raise CaptureError(f"{label} callee object must have native kind 56")
            obj = _mapping(node["object"], label + ".callee.object")
            callee_object = {"token": text(obj.get("token"), "callee.object.token"),
                             "name_status": text(obj.get("name_status"), "callee.object.name_status")}
            if obj.get("name") is not None:
                callee_object["name"] = text(obj["name"], "callee.object.name")
            if callee_object["name_status"] == "named" and "name" not in callee_object:
                raise CaptureError(f"{label} named callee object lacks a name")
            break
        if "operand" not in node:
            break  # Valid complex indirect callee; no argument-name inference.
        if depth == 5:
            raise CaptureError(f"{label} callee operand chain exceeds depth 6")
        node = _mapping(node["operand"], label + ".callee.operand")
    return {"status": "CAPTURED", "binding": raw["binding"],
            "call_expression_token": call_token, "result_descriptor_token": result_token,
            "result_native_kind": result_kind, "callee_kind": "direct" if len(chain) == 1 and callee_object else "indirect",
            "callee_object": callee_object, "callee_operand_chain": chain,
            "diagnostic_only": True, "authority_advanced": False}


def _tree_names(events: Sequence[Mapping[str, Any]], index: int,
                by_id: Mapping[str, list[int]]) -> tuple[list[str], dict[str, Any]]:
    """Resolve only same-capture trees; object names require captured object nodes."""
    origin = index
    visited: set[int] = set()
    while True:
        if index in visited or len(visited) >= 64:
            raise CaptureError("normalized tree reference cycle or depth limit")
        visited.add(index)
        wrapper = events[index].get("normalized_expression_tree")
        if wrapper is None:
            if index != origin:
                raise CaptureError("normalized tree reference has no tree")
            return [], {"kind": "absent"}
        wrapper = _mapping(wrapper, "normalized_expression_tree")
        if wrapper.get("schema") == "mwcc_normalized_expression_tree/v1":
            tree = _mapping(wrapper.get("tree"), "normalized_expression_tree.tree")
            break
        if wrapper.get("schema") != "mwcc_normalized_expression_tree_reference/v1":
            raise CaptureError("unknown normalized tree schema")
        reference = _string(wrapper.get("reference_event_id"), "normalized tree reference_event_id")
        matches = by_id.get(reference, [])
        if len(matches) != 1:
            raise CaptureError("normalized tree reference is missing or ambiguous")
        target = matches[0]
        target_wrapper = _mapping(events[target].get("normalized_expression_tree"), "referenced tree")
        target_token = (_mapping(target_wrapper.get("tree"), "referenced tree node").get("token")
                        if target_wrapper.get("schema") == "mwcc_normalized_expression_tree/v1"
                        else target_wrapper.get("expression_token"))
        null_tree = target_wrapper.get("tree") == {"reason": "null_expression", "status": "UNKNOWN"}
        if wrapper.get("expression_token") is None and null_tree:
            return [], {"kind": "null_expression", "event_id": events[target]["event_id"]}
        if not wrapper.get("expression_token") or wrapper.get("expression_token") != target_token:
            raise CaptureError("normalized tree reference expression token mismatch")
        index = target
    names: set[str] = set()
    pending = [(tree, 0)]
    count = 0
    while pending:
        node, depth = pending.pop()
        count += 1
        if count > 4096 or depth > 64:
            raise CaptureError("normalized tree traversal limit")
        if isinstance(node, Mapping):
            if node.get("native_kind") == 56 and node.get("status") == "CAPTURED":
                obj = _mapping(node.get("object"), "normalized object node")
                if obj.get("name_status") == "named":
                    _string(obj.get("token"), "normalized object token")
                    names.update(_names([obj.get("name")], "normalized object name"))
            pending.extend((value, depth + 1) for key, value in node.items()
                           if key not in {"type", "object"} and isinstance(value, (Mapping, list)))
        elif isinstance(node, list):
            pending.extend((value, depth + 1) for value in node)
    return sorted(names), {"kind": "normalized_expression_tree", "event_id": events[index]["event_id"],
                           "expression_token": tree.get("token"), "names": sorted(names)}


def _validate_capture(document: Mapping[str, Any]) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    if document.get("schema") != CAPTURE_SCHEMA:
        raise CaptureError(f"capture schema must be {CAPTURE_SCHEMA}")
    if document.get("status") != "CAPTURED":
        raise CaptureError("capture status must be CAPTURED")
    if document.get("diagnostic_only") is not True or document.get("authority_advanced") is not False:
        raise CaptureError("capture must be diagnostic-only and non-authoritative")
    function = _string(document.get("function"), "capture.function")
    session_id = _string(document.get("session_id"), "capture.session_id")
    target_thread = _integer(document.get("target_thread_ordinal"), "capture.target_thread_ordinal", maximum=0xFFFF)
    declared_lane = _lane(document.get("lane"), "capture.lane")
    source = _descriptor(document.get("source"), "capture.source")
    compiler = _descriptor(document.get("compiler"), "capture.compiler")
    if compiler["sha256"] != PINNED_COMPILER_SHA256:
        raise CaptureError("capture.compiler.sha256 is not the pinned GC/2.6 compiler")
    baseline = _descriptor(document.get("baseline_object"), "capture.baseline_object")
    output = _descriptor(document.get("output_object"), "capture.output_object")
    events = document.get("events")
    if isinstance(events, (str, bytes, bytearray)) or not isinstance(events, list):
        raise CaptureError("capture.events must be a list")
    if len(events) > MAX_EVENTS:
        raise CaptureError(f"capture.events exceeds {MAX_EVENTS} events")
    event_count = _integer(document.get("event_count"), "capture.event_count", maximum=MAX_EVENTS)
    if event_count != len(events):
        raise CaptureError("capture.event_count does not match events")
    first_before = _integer(document.get("counter_before_first_write"), "capture.counter_before_first_write")
    normalized: list[dict[str, Any]] = []
    previous_ordinal: int | None = None
    process_id: int | None = None
    event_lane: str | None = None
    observed_lanes: set[int] = set()
    for index, raw_event in enumerate(events):
        event = _mapping(raw_event, f"capture.events[{index}]")
        if event.get("schema") != EVENT_SCHEMA:
            raise CaptureError(f"capture.events[{index}].schema is not {EVENT_SCHEMA}")
        if event.get("status") != "CAPTURED":
            raise CaptureError(f"capture.events[{index}].status must be CAPTURED")
        if event.get("session_id") != session_id:
            raise CaptureError(f"capture.events[{index}] crosses session_id")
        if event.get("function") != function:
            raise CaptureError(f"capture.events[{index}] crosses function")
        if event.get("thread_ordinal") != target_thread:
            raise CaptureError(f"capture.events[{index}] crosses target thread")
        lane = _lane(event.get("lane"), f"capture.events[{index}].lane")
        if declared_lane is not None and lane is not None and lane != declared_lane:
            raise CaptureError(f"capture.events[{index}] crosses capture.lane")
        if lane is not None:
            if event_lane is None:
                event_lane = lane
            elif event_lane != lane:
                raise CaptureError(f"capture.events[{index}] crosses lane")
        sequence = _integer(event.get("sequence"), f"capture.events[{index}].sequence", maximum=MAX_EVENTS)
        if sequence != index:
            raise CaptureError(f"capture.events[{index}].sequence has a gap or reorder")
        ordinal = _integer(event.get("event_ordinal"), f"capture.events[{index}].event_ordinal", maximum=0xFFFFFFFF)
        if previous_ordinal is not None and ordinal <= previous_ordinal:
            raise CaptureError(f"capture.events[{index}].event_ordinal is not increasing")
        previous_ordinal = ordinal
        event_process = event.get("process_id")
        if event_process is not None:
            event_process = _integer(event_process, f"capture.events[{index}].process_id", maximum=0xFFFFFFFF)
            if process_id is None:
                process_id = event_process
            elif process_id != event_process:
                raise CaptureError(f"capture.events[{index}] crosses process_id")
        before = _integer(event.get("counter_before"), f"capture.events[{index}].counter_before")
        after = _integer(event.get("counter_after"), f"capture.events[{index}].counter_after")
        delta = event.get("counter_delta")
        if isinstance(delta, bool) or not isinstance(delta, int) or delta != after - before:
            raise CaptureError(f"capture.events[{index}].counter_delta does not match counters")
        token = _string(event.get("codegen_token"), f"capture.events[{index}].codegen_token")
        fields = event.get("codegen_fields")
        if fields is not None:
            fields = dict(_mapping(fields, f"capture.events[{index}].codegen_fields"))
        observed = _names(event.get("observed_expression_names"), f"capture.events[{index}].observed_expression_names")
        owners = event.get("assignment_owners")
        if owners is None:
            assignment = {"lhs": [], "rhs": []}
        else:
            owner_map = _mapping(owners, f"capture.events[{index}].assignment_owners")
            assignment = {
                "lhs": _names(owner_map.get("lhs"), f"capture.events[{index}].assignment_owners.lhs"),
                "rhs": _names(owner_map.get("rhs"), f"capture.events[{index}].assignment_owners.rhs"),
            }
        normalized.append({
            "index": index,
            "sequence": sequence,
            "event_ordinal": ordinal,
            "event_id": _string(event.get("event_id"), f"capture.events[{index}].event_id"),
            "function": function,
            "lane": lane or declared_lane,
            "instruction_pointer": _rva(event.get("instruction_pointer_rva"), f"capture.events[{index}].instruction_pointer_rva"),
            "codegen_token": token,
            "codegen_fields": fields or {},
            "codegen_kind": fields.get("expression_kind") if fields else None,
            "counter_before": before,
            "counter_after": after,
            "counter_delta": delta,
            "source_expression_names": observed,
            "assignment_owners": assignment,
            "lane_state": _validate_lane_state(event, before, after),
            "actual_call_origin": _actual_call_origin(event),
        })
        lane_state = normalized[-1]["lane_state"]
        if lane_state.get("status") == "observed":
            observed_lane = lane_state.get("observed_lane")
            if isinstance(observed_lane, int):
                observed_lanes.add(observed_lane)
    by_id: dict[str, list[int]] = {}
    for index, event in enumerate(normalized):
        if event['event_id'] in by_id:
            raise CaptureError('duplicate capture event_id: ' + event['event_id'])
        by_id.setdefault(event["event_id"], []).append(index)
    for index, event in enumerate(normalized):
        names, provenance = _tree_names(events, index, by_id)
        legacy = event["source_expression_names"]
        event["source_expression_name_sources"] = {
            "observed_expression_names": legacy, "normalized_tree": provenance}
        event["source_expression_names"] = sorted(set(legacy) | set(names))
    if normalized and normalized[0]["counter_before"] != first_before:
        raise CaptureError("capture.counter_before_first_write does not bind first event")
    binding = {
        "source": source,
        "compiler": compiler,
        "baseline_object": baseline,
        "output_object": output,
        "session_id": session_id,
        "function": function,
        "lane": event_lane or declared_lane,
        "target_thread_ordinal": target_thread,
    }
    if len(observed_lanes) == 1:
        binding["observed_lane"] = next(iter(observed_lanes))
        binding["observed_lane_status"] = "observed"
    elif observed_lanes:
        binding["observed_lane"] = None
        binding["observed_lane_status"] = "ambiguous"
    else:
        binding["observed_lane"] = None
        binding["observed_lane_status"] = "unobserved"
    return binding, normalized


def _transition(event: Mapping[str, Any], *, kind: str, destination: str | None,
                description: str, epoch: int, reason: str | None = None) -> dict[str, Any]:
    result = {
        "event_index": event["index"],
        "sequence": event["sequence"],
        "event_ordinal": event["event_ordinal"],
        "instruction_pointer_rva": _rva_text(event["instruction_pointer"]),
        "from": event["counter_before"],
        "to": event["counter_after"],
        "delta": event["counter_delta"],
        "kind": kind,
        "epoch_after": epoch,
        "lane_state": event["lane_state"],
        "responsible_source_nodes": _source_nodes(event),
    }
    if destination is not None:
        result["destination"] = destination
        lane_state = event["lane_state"]
        if lane_state.get("status") == "observed":
            values = lane_state.get("values", {})
            destination_value = values.get(destination)
            if isinstance(destination_value, int):
                result["destination_value"] = destination_value
                if destination_value == event["counter_after"]:
                    result["destination_observation"] = "observed"
                    result["classification"] = "native_state_qualified"
                else:
                    result["destination_observation"] = "mismatch"
                    result["classification"] = "contradicted_by_lane_state"
                    result["expected_kind"] = kind
                    result["kind"] = "unknown_transition"
                    result["reason"] = (
                        f"captured {destination} value {destination_value} does not match "
                        f"counter_after {event['counter_after']}"
                    )
            else:
                result["destination_observation"] = "values_unobserved"
                result["classification"] = "native_ip_bound"
        elif lane_state.get("status") == "values_unobserved":
            result["destination_observation"] = "values_unobserved"
            result["classification"] = "native_ip_bound"
        else:
            result["destination_observation"] = "state_mismatch"
            result["classification"] = "contradicted_by_lane_state"
            result["expected_kind"] = kind
            result["kind"] = "unknown_transition"
            result["reason"] = "captured temporary lane state does not match counter_after"
    if description:
        result["description"] = description
    if reason:
        result["reason"] = reason
    return result


def _source_nodes(event: Mapping[str, Any]) -> list[str]:
    owners = event.get("assignment_owners", {})
    return sorted({
        name
        for name in event.get("source_expression_names", [])
        + owners.get("lhs", [])
        + owners.get("rhs", [])
        if isinstance(name, str)
    })


def _owner(birth: Mapping[str, Any]) -> dict[str, Any]:
    return {
        "source_expression_names": birth["source_expression_names"],
        "source_expression_name_sources": birth.get("source_expression_name_sources"),
        "assignment_owners": birth["assignment_owners"],
        "codegen_token": birth["codegen_token"],
        "codegen_kind": birth["codegen_kind"],
        "actual_call_origin": birth.get("actual_call_origin"),
    }


def _owner_key(birth: Mapping[str, Any]) -> str:
    # The existing relationship classifies enclosing CodeGen ownership, not
    # individual native return descriptors. Keep these two identities separate.
    owner = _owner(birth)
    owner.pop("actual_call_origin", None)
    return json.dumps(owner, sort_keys=True, separators=(",", ":"))


def _known_transition(event: Mapping[str, Any]) -> tuple[str, str, str] | None:
    info = _KNOWN_TRANSITIONS.get(event["instruction_pointer"])
    if info is None:
        return None
    kind, destination, description = info
    before = event["counter_before"]
    delta = event["counter_delta"]
    if kind == "reset_to_saved_base" and before <= RESET_COUNTER_LIMIT:
        return None
    if kind == "restore_high_water" and delta <= 0:
        return None
    if kind == "phase_normalize_to_default" and delta >= 0:
        return None
    return kind, destination, description


def _birth(event: Mapping[str, Any], epoch: int, responsible: Mapping[str, Any] | None,
           vreg: int) -> dict[str, Any]:
    return {
        "event_index": event["index"],
        "sequence": event["sequence"],
        "event_ordinal": event["event_ordinal"],
        "event_id": event["event_id"],
        "epoch": epoch,
        "vreg": vreg,
        "counter_before": event["counter_before"],
        "counter_after": event["counter_after"],
        "instruction_pointer_rva": _rva_text(event["instruction_pointer"]),
        **_owner(event),
        "epoch_boundary": responsible,
    }


def _reset_transition(transitions: Sequence[Mapping[str, Any]]) -> Mapping[str, Any] | None:
    for transition in transitions:
        expected_kind = transition.get("expected_kind", transition.get("kind"))
        if expected_kind == "reset_to_saved_base":
            return transition
    return None


def _constraint(births: list[dict[str, Any]], relationships: list[dict[str, Any]],
                unknown_count: int, transitions: Sequence[Mapping[str, Any]],
                allocation_groups: list[dict[str, Any]]) -> dict[str, Any]:
    nodes = {
        name
        for birth in births
        for name in birth["source_expression_names"]
        + birth["assignment_owners"]["lhs"]
        + birth["assignment_owners"]["rhs"]
    }
    for transition in transitions:
        if transition.get("kind") == "reset_to_saved_base":
            nodes.update(transition.get("responsible_source_nodes", []))
    for group in allocation_groups:
        nodes.update(group["source_expression_names"])
        nodes.update(group["assignment_owners"]["lhs"])
        nodes.update(group["assignment_owners"]["rhs"])
    all_nodes = sorted(nodes)
    nodes = all_nodes[:MAX_AFFECTED_NODES]
    reused = len(births) > 1
    reset = _reset_transition(transitions)
    unknown_before_reset = (
        reset is not None
        and any(
            transition.get("kind") == "unknown_transition"
            and transition.get("event_index", -1) < reset.get("event_index", -1)
            for transition in transitions
        )
    )
    qualified_reset = (
        reset is not None
        and reset.get("kind") == "reset_to_saved_base"
        and reset.get("destination_observation") == "observed"
        and not unknown_before_reset
    )
    minimum_reduction = 0
    reset_evidence: dict[str, Any] = {"status": "not_observed"}
    if reset is not None:
        if reset.get("destination_observation") == "observed":
            reset_evidence = {
                "status": "observed",
                "event_index": reset["event_index"],
                "instruction_pointer_rva": reset["instruction_pointer_rva"],
                "from": reset["from"],
                "to": reset["to"],
                "destination": reset.get("destination"),
            }
        elif reset.get("destination_observation") == "values_unobserved":
            reset_evidence = {
                "status": "values_unobserved",
                "event_index": reset["event_index"],
                "instruction_pointer_rva": reset["instruction_pointer_rva"],
                "from": reset["from"],
                "to": reset["to"],
                "destination": reset.get("destination"),
            }
        else:
            reset_evidence = {
                "status": "contradicted",
                "event_index": reset["event_index"],
                "instruction_pointer_rva": reset["instruction_pointer_rva"],
            }
        if unknown_before_reset:
            reset_evidence = {
                "status": "unknown_before_reset",
                "event_index": reset["event_index"],
                "instruction_pointer_rva": reset["instruction_pointer_rva"],
                "reason": "counter continuity has an unknown transition before reset",
            }
        if qualified_reset:
            minimum_reduction = max(0, reset["from"] - RESET_COUNTER_LIMIT)
    if reused:
        if qualified_reset and minimum_reduction:
            required = (
                f"reduce allocations before {reset['instruction_pointer_rva']} by at least "
                f"{minimum_reduction} counter slots so current is <= 0x100 before reset, "
                "or change initial/max producer ownership through a real expression boundary"
            )
            text = (
                f"The requested temporary is reused after a qualified saved_base reset: "
                f"counter {reset['from']} exceeds 0x100 by {minimum_reduction}. "
                "Keep the allocation order within that numeric budget, or change the "
                "initial/max producer ownership through a real expression boundary."
            )
        elif reset is not None and reset.get("destination_observation") == "observed":
            required = (
                "keep allocation order within the observed counter budget before reset, "
                "or change initial/max producer ownership through a real expression boundary"
            )
            text = (
                "The requested temporary is reused across an observed native reset; "
                "the required change is numeric allocation/order or a real expression boundary."
            )
        else:
            required = (
                "change numeric allocation/order or establish a real expression boundary; "
                "no unsupported source-scope conclusion is inferred"
            )
            text = (
                "The requested temporary is reused across compiler epochs, but the reset "
                "destination is not fully observed. An unchanged local birth shape is not sufficient."
            )
    else:
        text = "No reuse of the requested temporary was observed in this bounded capture."
        required = "none inferred"
    return {
        "status": "actionable" if reused else "no_reuse_observed",
        "affected_source_nodes": nodes,
        "actual_call_origins": [birth["actual_call_origin"] for birth in births
                                if birth.get("actual_call_origin") is not None][:MAX_RELATIONSHIPS],
        "affected_source_nodes_truncated": len(all_nodes) > MAX_AFFECTED_NODES,
        "required_epoch_lifetime_change": required,
        "local_unchanged_birth_shape_sufficient": False if reused else None,
        "unknown_transition_count": unknown_count,
        "relationships_considered": len(relationships),
        "reset_evidence": reset_evidence,
        "numeric_constraint_status": (
            "qualified" if qualified_reset else "not_applicable" if not reused else "unknown"
        ),
        "minimum_allocation_reduction_before_reset": minimum_reduction,
        "allocation_groups_before_reset": allocation_groups,
        "text": text,
    }


def _allocation_groups(events: Sequence[Mapping[str, Any]],
                       transitions: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    reset = _reset_transition(transitions)
    if reset is None or reset.get("kind") != "reset_to_saved_base":
        return []
    if any(
        transition.get("kind") == "unknown_transition"
        and transition.get("event_index", -1) < reset.get("event_index", -1)
        for transition in transitions
    ):
        return []
    groups: dict[str, dict[str, Any]] = {}
    for event in events:
        if event["index"] >= reset["event_index"]:
            break
        if event["counter_delta"] != 1 or event["counter_after"] != event["counter_before"] + 1:
            continue
        key = json.dumps({
            "codegen_token": event["codegen_token"],
            "source_expression_names": event["source_expression_names"],
            "assignment_owners": event["assignment_owners"],
        }, sort_keys=True, separators=(",", ":"))
        group = groups.get(key)
        if group is None:
            group = {
                "codegen_token": event["codegen_token"],
                "codegen_kind": event["codegen_kind"],
                "source_expression_names": event["source_expression_names"],
                "assignment_owners": event["assignment_owners"],
                "count": 0,
                "first_event_index": event["index"],
                "last_event_index": event["index"],
            }
            groups[key] = group
        group["count"] += 1
        group["last_event_index"] = event["index"]
    return sorted(groups.values(), key=lambda group: (-group["count"], group["codegen_token"]))[:MAX_ALLOCATION_GROUPS]


def analyze_capture(document: Mapping[str, Any], vreg: int) -> dict[str, Any]:
    """Return a bounded epoch report for one requested backend temporary."""
    vreg = _integer(vreg, "vreg")
    binding, events = _validate_capture(document)
    epoch = 0
    previous_after: int | None = None
    responsible: dict[str, Any] | None = None
    births: list[dict[str, Any]] = []
    transitions: list[dict[str, Any]] = []
    unknowns: list[dict[str, Any]] = []
    for event in events:
        before, after, delta = event["counter_before"], event["counter_after"], event["counter_delta"]
        if previous_after is not None and before != previous_after:
            epoch += 1
            responsible = _transition(
                event, kind="unknown_transition", destination=None,
                description="counter continuity gap; transition is unknown", epoch=epoch,
                reason=f"previous counter ended at {previous_after}, event starts at {before}",
            )
            transitions.append(responsible)
            if len(unknowns) < MAX_RELATIONSHIPS:
                unknowns.append(responsible)
        ip_info = _known_transition(event)
        is_birth = delta == 1 and after == before + 1
        if is_birth:
            if before == vreg:
                births.append(_birth(event, epoch, responsible, vreg))
            previous_after = after
            continue
        if event["index"] == 0 and before == document["counter_before_first_write"] and delta >= 0:
            responsible = None
            if delta > 0:
                transitions.append(_transition(
                    event, kind="initialization", destination=None,
                    description="initial counter setup before the first birth", epoch=epoch,
                ))
            previous_after = after
            continue
        if ip_info is not None:
            kind, destination, description = ip_info
            epoch += 1
            responsible = _transition(
                event, kind=kind, destination=destination,
                description=description, epoch=epoch,
            )
            if responsible["kind"] == "unknown_transition" and len(unknowns) < MAX_RELATIONSHIPS:
                unknowns.append(responsible)
        else:
            epoch += 1
            responsible = _transition(
                event, kind="unknown_transition", destination=None,
                description="unrecognized counter transition; no generic wrap assumed",
                epoch=epoch,
            )
            if len(unknowns) < MAX_RELATIONSHIPS:
                unknowns.append(responsible)
        transitions.append(responsible)
        previous_after = after

    relationships: list[dict[str, Any]] = []
    for first_index, first in enumerate(births):
        for second in births[first_index + 1:]:
            if len(relationships) >= MAX_RELATIONSHIPS:
                break
            boundary = second.get("epoch_boundary")
            boundary_known = isinstance(boundary, Mapping) and boundary.get("kind") != "unknown_transition"
            relationships.append({
                "first_birth_event": first["event_id"],
                "second_birth_event": second["event_id"],
                "first_epoch": first["epoch"],
                "second_epoch": second["epoch"],
                "relationship": "same_observed_codegen_owner" if _owner_key(first) == _owner_key(second)
                else "distinct_codegen_owner",
                "first_owner": _owner(first),
                "second_owner": _owner(second),
                "epoch_boundary_known": boundary_known,
                "second_birth_boundary": boundary,
            })
        if len(relationships) >= MAX_RELATIONSHIPS:
            break

    transition_count = len(transitions)
    unknown_count = sum(transition["kind"] == "unknown_transition" for transition in transitions)
    birth_count = len(births)
    allocation_groups = _allocation_groups(events, transitions)
    if birth_count > 1:
        status = "reused"
    elif birth_count == 1:
        status = "single_birth"
    else:
        status = "no_birth"
    result = {
        "schema": SCHEMA,
        "schema_version": 1,
        "status": status,
        "diagnostic_only": True,
        "authority_advanced": False,
        "capture_schema": CAPTURE_SCHEMA,
        "binding": binding,
        "vreg": vreg,
        "event_count": len(events),
        "epoch_count": epoch + 1,
        "birth_count": birth_count,
        "transition_count": transition_count,
        "unknown_transition_count": unknown_count,
        "births": births[:MAX_RELATIONSHIPS],
        "relationships": relationships,
        "transitions": transitions[:MAX_RELATIONSHIPS],
        "unknown_transitions": unknowns,
        "constraint": _constraint(births, relationships, unknown_count, transitions, allocation_groups),
        "truncated": {
            "births": birth_count > MAX_RELATIONSHIPS,
            "relationships": birth_count * (birth_count - 1) // 2 > MAX_RELATIONSHIPS,
            "transitions": transition_count > MAX_RELATIONSHIPS,
            "unknown_transitions": unknown_count > MAX_RELATIONSHIPS,
        },
    }
    return result


def _is_birth_event(event: Mapping[str, Any]) -> bool:
    return event["counter_delta"] == 1 and event["counter_after"] == event["counter_before"] + 1


def _annotation(event: Mapping[str, Any]) -> tuple[tuple[Any, ...], str, str | None]:
    """Return a hashable source annotation and an explicit certainty label."""

    kind = event.get("codegen_kind")
    names = tuple(event.get("source_expression_names", ()))
    owners = event.get("assignment_owners", {})
    lhs = tuple(owners.get("lhs", ())) if isinstance(owners, Mapping) else ()
    rhs = tuple(owners.get("rhs", ())) if isinstance(owners, Mapping) else ()
    signature = (kind, names, lhs, rhs)
    if kind is None or not isinstance(kind, (str, int, float, bool)):
        return signature, "UNKNOWN", "missing_codegen_kind"
    if not names and not lhs and not rhs:
        return signature, "UNKNOWN", "missing_source_annotation"
    if len(set(names)) != len(names) or len(set(lhs)) != len(lhs) or len(set(rhs)) != len(rhs):
        return signature, "UNKNOWN", "ambiguous_source_annotation"
    # Multiple observed names without an owner set cannot identify which
    # source node caused the allocation.  Keep the evidence, but do not call
    # two such groups equal merely because their lists happen to agree.
    if len(names) > 1 and not lhs and not rhs:
        return signature, "UNKNOWN", "ambiguous_source_names"
    return signature, "known", None


def _annotation_json(signature: tuple[Any, ...]) -> dict[str, Any]:
    kind, names, lhs, rhs = signature
    return {
        "codegen_kind": kind if isinstance(kind, (str, int, float, bool)) else None,
        "source_expression_names": list(names),
        "assignment_owners": {"lhs": list(lhs), "rhs": list(rhs)},
    }


def _source_groups(
    events: Sequence[Mapping[str, Any]], vreg: int
) -> tuple[list[dict[str, Any]], int, int, list[int], bool]:
    """Group adjacent births by local token plus annotation.

    The token only separates groups within one capture.  It is deliberately
    absent from the returned matching signature, since native sessions mint
    independent tokens for the same source expression.
    """

    groups: list[dict[str, Any]] = []
    total_births = 0
    requested_births: list[int] = []
    total_group_count = 0
    truncated = False
    current_key: tuple[Any, ...] | None = None
    current: dict[str, Any] | None = None
    reset_index = 0

    for event in events:
        if not _is_birth_event(event):
            current_key = None
            current = None
            if _known_transition(event) is not None:
                reset_index += 1
            continue
        total_births += 1
        if event["counter_before"] == vreg:
            requested_births.append(event["index"])
        signature, annotation_status, unknown_reason = _annotation(event)
        local_key = (event["codegen_token"], signature)
        if current is not None and current_key == local_key:
            current["last_event_index"] = event["index"]
            current["last_sequence"] = event["sequence"]
            current["last_event_ordinal"] = event["event_ordinal"]
            current["count"] += 1
            if event["counter_before"] == vreg:
                current["requested_birth_count"] += 1
            continue

        total_group_count += 1
        current_key = local_key
        current = {
            "group_index": total_group_count - 1,
            "first_event_index": event["index"],
            "last_event_index": event["index"],
            "first_sequence": event["sequence"],
            "last_sequence": event["sequence"],
            "first_event_ordinal": event["event_ordinal"],
            "last_event_ordinal": event["event_ordinal"],
            "count": 1,
            "requested_birth_count": 1 if event["counter_before"] == vreg else 0,
            "reset_index": reset_index,
            "annotation_status": annotation_status,
            "unknown_reason": unknown_reason,
            "signature": _annotation_json(signature),
            "codegen_token": event["codegen_token"],
        }
        # Retain the bounded event-derived sequence for matching.  Only the
        # published detail is capped; otherwise a real late change can be
        # hidden behind the first sixteen groups.
        groups.append(current)
    if len(groups) > MAX_COMPARE_GROUPS:
        truncated = True
    return groups, total_group_count, total_births, requested_births, truncated


def _reset_records(
    events: Sequence[Mapping[str, Any]], vreg: int
) -> tuple[list[dict[str, Any]], int, list[int], bool]:
    records: list[dict[str, Any]] = []
    total_births = 0
    requested_births: list[int] = []
    reset_count = 0
    truncated = False
    for event in events:
        if _is_birth_event(event):
            total_births += 1
            if event["counter_before"] == vreg:
                requested_births.append(event["index"])
            continue
        info = _known_transition(event)
        if info is None:
            continue
        kind, destination, _description = info
        reset_count += 1
        record = {
            "reset_index": reset_count - 1,
            "event_index": event["index"],
            "sequence": event["sequence"],
            "event_ordinal": event["event_ordinal"],
            "instruction_pointer_rva": _rva_text(event["instruction_pointer"]),
            "kind": kind,
            "destination": destination,
            "from": event["counter_before"],
            "to": event["counter_after"],
            "total_births_before_reset": total_births,
            "birth_count_before_reset": total_births,
            "requested_births_before_reset": len(requested_births),
        }
        # As with source groups, keep all event-derived reset boundaries for
        # comparison and cap only the serialized detail in compare_captures.
        records.append(record)
    if len(records) > MAX_COMPARE_RESETS:
        truncated = True
    return records, reset_count, requested_births, truncated


def _reuse_state(count: int) -> dict[str, Any]:
    if count <= 0:
        status = "no_birth"
    elif count == 1:
        status = "single_birth"
    else:
        status = "reused"
    return {"status": status, "birth_count": count, "reused": count > 1}


def _reuse_summary(
    analysis: Mapping[str, Any], resets: Sequence[Mapping[str, Any]], requested_births: Sequence[int]
) -> dict[str, Any]:
    per_reset: list[dict[str, Any]] = []
    for reset in resets:
        event_index = reset["event_index"]
        before = sum(index < event_index for index in requested_births)
        after = sum(index > event_index for index in requested_births)
        per_reset.append({
            "reset_index": reset["reset_index"],
            "event_index": event_index,
            "before_reset": _reuse_state(before),
            "after_reset": _reuse_state(after),
        })
    if per_reset:
        before_first = per_reset[0]["before_reset"]
        after_first = per_reset[0]["after_reset"]
    else:
        before_first = {"status": "no_reset_observed", "birth_count": len(requested_births), "reused": len(requested_births) > 1}
        after_first = {"status": "no_reset_observed", "birth_count": 0, "reused": False}
    overall_count = analysis.get("birth_count")
    if isinstance(overall_count, bool) or not isinstance(overall_count, int):
        overall_count = len(requested_births)
    return {
        "vreg": analysis["vreg"],
        "overall": _reuse_state(overall_count),
        "before_first_reset": before_first,
        "after_first_reset": after_first,
        "per_reset": per_reset[:MAX_COMPARE_RESETS],
        "reset_count": len(per_reset),
    }


def _binding_descriptor_key(binding: Mapping[str, Any], name: str) -> tuple[Any, Any]:
    descriptor = binding.get(name)
    if not isinstance(descriptor, Mapping):
        return None, None
    return descriptor.get("sha256"), descriptor.get("size_bytes")


def _same_lane(base: Mapping[str, Any], candidate: Mapping[str, Any]) -> bool | None:
    if base.get("lane") != candidate.get("lane"):
        return False
    base_status = base.get("observed_lane_status")
    candidate_status = candidate.get("observed_lane_status")
    if base_status == candidate_status == "observed":
        return base.get("observed_lane") == candidate.get("observed_lane")
    if base_status in {"ambiguous", "observed"} or candidate_status in {"ambiguous", "observed"}:
        return None
    return True


def _binding_comparison(base: Mapping[str, Any], candidate: Mapping[str, Any]) -> dict[str, Any]:
    same_compiler = _binding_descriptor_key(base, "compiler") == _binding_descriptor_key(candidate, "compiler")
    same_function = base.get("function") == candidate.get("function")
    same_lane = _same_lane(base, candidate)
    reasons: list[str] = []
    if not same_compiler:
        reasons.append("compiler_drift")
    if not same_function:
        reasons.append("function_drift")
    if same_lane is False:
        reasons.append("lane_drift")
    elif same_lane is None:
        reasons.append("lane_identity_unknown")
    return {
        "compatible": same_compiler and same_function and same_lane is True,
        "same_compiler": same_compiler,
        "same_function": same_function,
        "same_lane": same_lane,
        "reasons": reasons,
        "base": {
            "compiler": base.get("compiler"),
            "function": base.get("function"),
            "lane": base.get("lane"),
            "observed_lane": base.get("observed_lane"),
            "observed_lane_status": base.get("observed_lane_status"),
        },
        "candidate": {
            "compiler": candidate.get("compiler"),
            "function": candidate.get("function"),
            "lane": candidate.get("lane"),
            "observed_lane": candidate.get("observed_lane"),
            "observed_lane_status": candidate.get("observed_lane_status"),
        },
    }


def _analysis_summary(analysis: Mapping[str, Any]) -> dict[str, Any]:
    return {
        "status": analysis.get("status"),
        "event_count": analysis.get("event_count"),
        "epoch_count": analysis.get("epoch_count"),
        "birth_count": analysis.get("birth_count"),
        "transition_count": analysis.get("transition_count"),
        "unknown_transition_count": analysis.get("unknown_transition_count"),
    }


def _annotation_decoder_key(document: Mapping[str, Any]) -> tuple[str, str] | None:
    """Return the capture's expression-name decoder identity when present."""

    producer = document.get("producer")
    candidates: list[Any] = [document.get("annotation_decoder"), document.get("expression_decoder")]
    if isinstance(producer, Mapping):
        candidates.extend((producer.get("expression_capture"), producer.get("annotation_decoder")))
    for value in candidates:
        if isinstance(value, Mapping):
            for field in ("sha256", "version", "tool_version", "id"):
                identity = value.get(field)
                if isinstance(identity, str) and identity.strip():
                    return field, identity.strip()
        elif isinstance(value, str) and value.strip():
            return "value", value.strip()
    return None


def _annotation_comparison(
    base: Mapping[str, Any], candidate: Mapping[str, Any]
) -> dict[str, Any]:
    base_key = _annotation_decoder_key(base)
    candidate_key = _annotation_decoder_key(candidate)
    if base_key is not None and candidate_key is not None and base_key == candidate_key:
        status = "comparable"
        reason = None
    elif base_key is not None and candidate_key is not None:
        status = "UNKNOWN"
        reason = "annotation_decoder_drift"
    else:
        status = "UNKNOWN"
        reason = "annotation_decoder_metadata_missing"
    result: dict[str, Any] = {
        "status": status,
        "base_decoder": base_key[1] if base_key is not None else None,
        "candidate_decoder": candidate_key[1] if candidate_key is not None else None,
    }
    if reason is not None:
        result["reason"] = reason
    return result


def _group_match_key(group: Mapping[str, Any]) -> tuple[Any, ...] | None:
    if group.get("annotation_status") != "known":
        return None
    signature = group.get("signature")
    if not isinstance(signature, Mapping):
        return None
    owners = signature.get("assignment_owners")
    if not isinstance(owners, Mapping):
        return None
    return (
        signature.get("codegen_kind"),
        tuple(signature.get("source_expression_names", ())),
        tuple(owners.get("lhs", ())),
        tuple(owners.get("rhs", ())),
    )


def _group_ref(group: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if group is None:
        return None
    return {
        key: group.get(key)
        for key in (
            "group_index", "first_event_index", "last_event_index", "first_sequence",
            "last_sequence", "count", "requested_birth_count", "reset_index",
            "annotation_status", "unknown_reason", "signature",
        )
    }


def _difference_order(item: Mapping[str, Any]) -> tuple[int, int]:
    """Sort a finding by its earliest source event, including capped output."""

    indices: list[int] = []
    for key in ("base_event_index", "candidate_event_index"):
        value = item.get(key)
        if isinstance(value, int):
            indices.append(value)
    for key in ("base_group", "candidate_group"):
        group = item.get(key)
        if isinstance(group, Mapping) and isinstance(group.get("first_event_index"), int):
            indices.append(group["first_event_index"])
    return (min(indices) if indices else MAX_COUNTER, len(indices))


def _same_reset_shapes(
    base: Sequence[Mapping[str, Any]], candidate: Sequence[Mapping[str, Any]]
) -> bool:
    return [
        (item.get("kind"), item.get("destination")) for item in base
    ] == [
        (item.get("kind"), item.get("destination")) for item in candidate
    ]


def compare_captures(
    base: Mapping[str, Any], candidate: Mapping[str, Any], vreg: int
) -> dict[str, Any]:
    """Compare temporary-birth chronology from two independent captures.

    Each capture is validated and analyzed independently.  Session IDs and
    codegen tokens are retained as local binding evidence, never as equality
    keys.  A compiler/function/lane drift skips group comparison entirely.
    """

    base_analysis = analyze_capture(base, vreg)
    candidate_analysis = analyze_capture(candidate, vreg)
    base_binding, base_events = _validate_capture(base)
    candidate_binding, candidate_events = _validate_capture(candidate)
    identity = _binding_comparison(base_binding, candidate_binding)
    annotation_comparability = _annotation_comparison(base, candidate)
    annotation_comparable = annotation_comparability["status"] == "comparable"
    base_groups, base_group_total, base_birth_total, base_requested, base_groups_truncated = _source_groups(base_events, vreg)
    candidate_groups, candidate_group_total, candidate_birth_total, candidate_requested, candidate_groups_truncated = _source_groups(candidate_events, vreg)
    base_resets, base_reset_total, base_requested, base_resets_truncated = _reset_records(base_events, vreg)
    candidate_resets, candidate_reset_total, candidate_requested, candidate_resets_truncated = _reset_records(candidate_events, vreg)
    published_base_groups = base_groups[:MAX_COMPARE_GROUPS]
    published_candidate_groups = candidate_groups[:MAX_COMPARE_GROUPS]
    published_base_resets = base_resets[:MAX_COMPARE_RESETS]
    published_candidate_resets = candidate_resets[:MAX_COMPARE_RESETS]
    base_reuse = _reuse_summary(base_analysis, base_resets, base_requested)
    candidate_reuse = _reuse_summary(candidate_analysis, candidate_resets, candidate_requested)

    matched_groups: list[dict[str, Any]] = []
    matched_group_total = 0
    differences: list[dict[str, Any]] = []
    difference_total = 0
    unknown_total = 0
    earliest: dict[str, Any] | None = None

    def add_difference(item: dict[str, Any], *, unknown: bool = False) -> None:
        nonlocal difference_total, unknown_total, earliest
        difference_total += 1
        if unknown:
            unknown_total += 1
        if earliest is None or _difference_order(item) < _difference_order(earliest):
            earliest = item
        if len(differences) < MAX_COMPARE_DIFFERENCES:
            differences.append(item)

    def add_matched(item: dict[str, Any]) -> None:
        nonlocal matched_group_total
        matched_group_total += 1
        if len(matched_groups) < MAX_COMPARE_MATCHES:
            matched_groups.append(item)

    comparison_skipped = not identity["compatible"] or not annotation_comparable
    if not identity["compatible"]:
        for reason in identity["reasons"]:
            add_difference({"kind": "binding_drift", "reason": reason}, unknown=True)
    if not annotation_comparable:
        add_difference({
            "kind": "annotation_comparability_unknown",
            "status": "UNKNOWN",
            "reason": annotation_comparability["reason"],
        }, unknown=True)
    if identity["compatible"] and annotation_comparable:
        base_signatures: dict[tuple[Any, ...], int] = {}
        candidate_signatures: dict[tuple[Any, ...], int] = {}
        for group in base_groups:
            key = _group_match_key(group)
            if key is not None:
                base_signatures[key] = base_signatures.get(key, 0) + 1
        for group in candidate_groups:
            key = _group_match_key(group)
            if key is not None:
                candidate_signatures[key] = candidate_signatures.get(key, 0) + 1

        def match_key(group: Mapping[str, Any], side: str, index: int) -> tuple[Any, ...]:
            key = _group_match_key(group)
            if key is None:
                return (f"__unknown_{side}_group__", index)
            own_signatures = base_signatures if side == "base" else candidate_signatures
            other_signatures = candidate_signatures if side == "base" else base_signatures
            if own_signatures.get(key, 0) != 1 or other_signatures.get(key, 0) > 1:
                # Repeated signatures do not identify a unique source event;
                # force a side-local mismatch so the result remains UNKNOWN.
                return (f"__ambiguous_{side}_group__", index, key)
            return key

        def uniquely_matched(group: Mapping[str, Any], side: str) -> bool:
            key = _group_match_key(group)
            own_signatures = base_signatures if side == "base" else candidate_signatures
            other_signatures = candidate_signatures if side == "base" else base_signatures
            return (
                key is not None
                and own_signatures.get(key, 0) == 1
                and other_signatures.get(key, 0) <= 1
            )

        base_keys = [
            match_key(group, "base", index)
            for index, group in enumerate(base_groups)
        ]
        candidate_keys = [
            match_key(group, "candidate", index)
            for index, group in enumerate(candidate_groups)
        ]
        matcher = SequenceMatcher(a=base_keys, b=candidate_keys, autojunk=False)
        for tag, base_start, base_end, candidate_start, candidate_end in matcher.get_opcodes():
            if tag == "equal":
                for offset in range(base_end - base_start):
                    left = base_groups[base_start + offset]
                    right = candidate_groups[candidate_start + offset]
                    delta = right["count"] - left["count"]
                    matched = {
                        "base_group_index": left["group_index"],
                        "candidate_group_index": right["group_index"],
                        "signature": left["signature"],
                        "base_count": left["count"],
                        "candidate_count": right["count"],
                        "allocation_count_delta": delta,
                        "status": "matched" if delta == 0 else "allocation_count_changed",
                        "base_event_index": left["first_event_index"],
                        "candidate_event_index": right["first_event_index"],
                    }
                    add_matched(matched)
                    if delta:
                        add_difference({"kind": "allocation_count_changed", **matched})
                continue

            span = max(base_end - base_start, candidate_end - candidate_start)
            for offset in range(span):
                left = base_groups[base_start + offset] if base_start + offset < base_end else None
                right = candidate_groups[candidate_start + offset] if candidate_start + offset < candidate_end else None
                left_known = left is not None and uniquely_matched(left, "base")
                right_known = right is not None and uniquely_matched(right, "candidate")
                if left is not None and right is not None and not left_known and not right_known:
                    add_difference({
                        "kind": "unknown_group",
                        "status": "UNKNOWN",
                        "base_group": _group_ref(left),
                        "candidate_group": _group_ref(right),
                        "reason": (
                            "duplicate_source_signature"
                            if (_group_match_key(left) is not None
                                and _group_match_key(right) is not None)
                            else "source_annotation_unknown_on_both_sides"
                        ),
                    }, unknown=True)
                elif left is not None and right is not None:
                    add_difference({
                        "kind": "group_replaced",
                        "base_group": _group_ref(left),
                        "candidate_group": _group_ref(right),
                    }, unknown=not left_known or not right_known)
                elif left is not None:
                    add_difference({
                        "kind": "group_removed" if left_known else "unknown_group",
                        "status": "UNKNOWN" if not left_known else "changed",
                        "base_group": _group_ref(left),
                    }, unknown=not left_known)
                elif right is not None:
                    add_difference({
                        "kind": "group_inserted" if right_known else "unknown_group",
                        "status": "UNKNOWN" if not right_known else "changed",
                        "candidate_group": _group_ref(right),
                    }, unknown=not right_known)

        if (base_reset_total != candidate_reset_total
                or not _same_reset_shapes(base_resets, candidate_resets)):
            add_difference({
                "kind": "reset_boundary_changed",
                "base_reset_count": base_reset_total,
                "candidate_reset_count": candidate_reset_total,
                "base": published_base_resets,
                "candidate": published_candidate_resets,
            }, unknown=False)
        if base_reuse["overall"] != candidate_reuse["overall"]:
            add_difference({
                "kind": "requested_vreg_reuse_changed",
                "base": base_reuse["overall"],
                "candidate": candidate_reuse["overall"],
                "vreg": vreg,
            }, unknown=False)

        # A bounded prefix cannot prove that a later source group/reset is
        # unchanged.  Keep the useful prefix findings, but make the report
        # explicitly UNKNOWN instead of silently claiming equality.
        if (base_groups_truncated or candidate_groups_truncated
                or base_resets_truncated or candidate_resets_truncated):
            add_difference({
                "kind": "comparison_truncated",
                "status": "UNKNOWN",
                "reason": "detailed source groups or reset boundaries exceeded the report bound",
            }, unknown=True)

    if not identity["compatible"]:
        status = "incompatible"
    elif not annotation_comparable or unknown_total:
        status = "unknown"
    elif difference_total:
        status = "changed"
    else:
        status = "same"
    return {
        "schema": COMPARE_SCHEMA,
        "schema_version": 1,
        "status": status,
        "comparison_status": status,
        "diagnostic_only": True,
        "authority_advanced": False,
        "vreg": vreg,
        "comparison_skipped": comparison_skipped,
        "annotation_comparison_skipped": not annotation_comparable,
        "comparison": identity,
        "annotation_comparability": annotation_comparability,
        "binding": {"base": base_binding, "candidate": candidate_binding},
        "session_binding": {
            "base_session_id": base_binding["session_id"],
            "candidate_session_id": candidate_binding["session_id"],
            "same_session": base_binding["session_id"] == candidate_binding["session_id"],
            "independent_sessions": base_binding["session_id"] != candidate_binding["session_id"],
            "session_tokens_compared": False,
        },
        "analyses": {"base": _analysis_summary(base_analysis), "candidate": _analysis_summary(candidate_analysis)},
        "source_expression_groups": {
            "base": published_base_groups,
            "candidate": published_candidate_groups,
        },
        "group_counts": {
            "base": {"returned": len(published_base_groups), "total": base_group_total},
            "candidate": {"returned": len(published_candidate_groups), "total": candidate_group_total},
        },
        "matched_groups": matched_groups,
        "matched_group_count": matched_group_total,
        "allocation_totals": {
            "base": base_birth_total,
            "candidate": candidate_birth_total,
            "delta": candidate_birth_total - base_birth_total,
        },
        "allocation_count_delta": candidate_birth_total - base_birth_total,
        "differences": differences,
        "difference_count": difference_total,
        "unknown_count": unknown_total,
        "unknowns": [item for item in differences if item.get("status") == "UNKNOWN" or item.get("kind") in {"unknown_group", "binding_drift"}],
        "earliest_causal_difference": earliest,
        "births_before_each_reset": {
            "base": published_base_resets,
            "candidate": published_candidate_resets,
        },
        "reset_counts": {"base": base_reset_total, "candidate": candidate_reset_total},
        "requested_vreg_reuse": {"base": base_reuse, "candidate": candidate_reuse},
        "truncated": {
            "base_groups": base_groups_truncated,
            "candidate_groups": candidate_groups_truncated,
            "base_resets": base_resets_truncated,
            "candidate_resets": candidate_resets_truncated,
            "matched_groups": matched_group_total > len(matched_groups),
            "differences": difference_total > len(differences),
            "unknowns": unknown_total > sum(
                item.get("status") == "UNKNOWN" or item.get("kind") in {"unknown_group", "binding_drift"}
                for item in differences
            ),
        },
    }


def _capture_descriptor(path: Path, raw: bytes) -> dict[str, Any]:
    return {"path": str(path.resolve()), "sha256": hashlib.sha256(raw).hexdigest(), "size_bytes": len(raw)}


def _rehash_descriptor(label: str, descriptor: Mapping[str, Any]) -> None:
    path = Path(descriptor["path"]).expanduser().resolve()
    try:
        size = path.stat().st_size
    except OSError as exc:
        raise CaptureError(f"{label} is not readable: {path}") from exc
    if not path.is_file():
        raise CaptureError(f"{label} is not a regular file: {path}")
    if size > MAX_DESCRIPTOR_BYTES:
        raise CaptureError(f"{label} exceeds {MAX_DESCRIPTOR_BYTES} bytes: {path}")
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise CaptureError(f"{label} is not readable: {path}") from exc
    if size != descriptor["size_bytes"] or digest.hexdigest() != descriptor["sha256"]:
        raise CaptureError(f"{label} changed since capture: {path}")


def _validate_bound_files(binding: Mapping[str, Any]) -> None:
    for label in ("source", "compiler", "baseline_object", "output_object"):
        _rehash_descriptor(f"capture.{label}", _mapping(binding[label], f"capture.{label}"))


def analyze_path(capture: Path, vreg: int) -> dict[str, Any]:
    capture = Path(capture).expanduser().resolve()
    raw = capture.read_bytes()
    if len(raw) > MAX_CAPTURE_BYTES:
        raise CaptureError(f"capture exceeds {MAX_CAPTURE_BYTES} bytes: {capture}")
    try:
        document = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise CaptureError(f"invalid capture JSON {capture}: {exc}") from exc
    result = analyze_capture(_mapping(document, "capture"), vreg)
    _validate_bound_files(result["binding"])
    # Coordinates, when captured, belong to the enclosing CodeGen node; names
    # alone never establish a source line or an exact child identity.
    from tools import recovery_expression_join as expression_join
    _, events = _validate_capture(document)
    births = {item["event_id"] for item in result.get("births", [])}
    origins = []
    for event in events:
        if event["event_id"] not in births:
            continue
        fields = event["codegen_fields"]
        origins.append({"event_id": event["event_id"],
                        "source_offset": fields.get("source_offset"),
                        "expression_kind": fields.get("expression_kind"),
                        "direct_callee_name": fields.get("direct_callee_name")})
    source = Path(result["binding"]["source"]["path"]).read_bytes()
    result["source_regions"] = expression_join.actionable_source_regions(
        source, result["binding"]["function"], origins,
        result["binding"]["source"]["sha256"], reuse_observed=result["status"] == "reused")
    result["capture"] = _capture_descriptor(capture, raw)
    return result


def _encoded(result: Mapping[str, Any]) -> bytes:
    raw = json.dumps(result, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8") + b"\n"
    if len(raw) > MAX_OUTPUT_BYTES:
        raise CaptureError(f"analysis output exceeds {MAX_OUTPUT_BYTES} bytes")
    return raw


def _write_atomic(path: Path, raw: bytes) -> None:
    path = path.expanduser().resolve()
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        temporary.write_bytes(raw)
        os.replace(temporary, path)
    except OSError:
        try:
            temporary.unlink()
        except OSError:
            pass
        raise


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, required=True, help="captured compiler counter JSON")
    parser.add_argument("--vreg", type=int, required=True, help="requested temporary counter value")
    parser.add_argument("--out", type=Path, help="optional atomic JSON output path")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        result = analyze_path(args.capture, args.vreg)
        raw = _encoded(result)
        if args.out is None:
            sys.stdout.buffer.write(raw)
        else:
            if args.out.expanduser().resolve() == args.capture.expanduser().resolve():
                raise CaptureError("refusing to overwrite capture input")
            _write_atomic(args.out, raw)
            print(json.dumps({"status": result["status"], "out": str(args.out.expanduser().resolve())}, sort_keys=True))
        return 0
    except (OSError, CaptureError, TypeError, ValueError) as exc:
        print(f"recovery temp epochs: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
