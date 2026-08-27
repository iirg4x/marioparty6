#!/usr/bin/env python3
"""Validate repeated target-opcode evidence for governed low-level-source readiness.

This module is diagnostic only.  It can establish that a sealed, repeated
opcode fingerprint and a bounded natural-C exhaustion record are ready for a
separate governed review.  It never grants authorization, schedules a source
candidate, edits source, or advances recovery authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path
from typing import Any, Mapping


CONTEXT_SCHEMA = "repeated_opcode_low_level_readiness_context/v1"
RESULT_SCHEMA = "repeated_opcode_low_level_readiness/v1"
RULE_ID = "repeated_opcode_low_level_source_readiness"
HASH_FIELD = "readiness_sha256"

_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SOURCE_CLASS = "target_proven_low_level_source"
_CONTROL_RESULTS = {
    "object_identical_nonexact",
    "different_nonexact_object",
    "regressed",
    "inadmissible",
}


class RepeatedOpcodeReadinessInputError(ValueError):
    """The supplied packet cannot safely support a readiness diagnosis."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise RepeatedOpcodeReadinessInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise RepeatedOpcodeReadinessInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 512) -> str:
    if not isinstance(value, str) or not value.strip():
        raise RepeatedOpcodeReadinessInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise RepeatedOpcodeReadinessInputError(f"{label} exceeds {limit} characters")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise RepeatedOpcodeReadinessInputError(f"{label} must be lowercase SHA-256")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise RepeatedOpcodeReadinessInputError(f"{label} must be a C identifier")
    return result


def _bool(value: Any, label: str, expected: bool | None = None) -> bool:
    if not isinstance(value, bool):
        raise RepeatedOpcodeReadinessInputError(f"{label} must be a Boolean")
    if expected is not None and value is not expected:
        raise RepeatedOpcodeReadinessInputError(
            f"{label} must be {str(expected).lower()}"
        )
    return value


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 30) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise RepeatedOpcodeReadinessInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise RepeatedOpcodeReadinessInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _hex_bytes(value: Any, label: str) -> bytes:
    text = _text(value, label, limit=4096).lower()
    if len(text) % 2 or re.fullmatch(r"[0-9a-f]+", text) is None:
        raise RepeatedOpcodeReadinessInputError(f"{label} must be even-length hex bytes")
    return bytes.fromhex(text)


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode(
        "utf-8"
    )


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical_bytes(value)).hexdigest()


def _parse_tool_identity(value: Any, label: str) -> dict[str, str]:
    item = _closed(
        value,
        allowed={"version", "sha256"},
        required={"version", "sha256"},
        label=label,
    )
    return {
        "version": _text(item.get("version"), f"{label}.version", limit=128),
        "sha256": _sha256(item.get("sha256"), f"{label}.sha256"),
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "repeated opcode readiness context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "owner",
        "configured_compiler",
        "toolchain",
        "candidate",
        "target",
        "opcode_inventory",
        "groups",
        "natural_c_exhaustion",
        "governed_low_level_source",
        "exact_result",
        "telemetry",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise RepeatedOpcodeReadinessInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    _bool(context.get("authority_advanced"), f"{label}.authority_advanced", False)

    compiler_raw = _closed(
        context.get("configured_compiler"),
        allowed={"version", "sha256", "wrapper_sha256"},
        required={"version", "sha256", "wrapper_sha256"},
        label=f"{label}.configured_compiler",
    )
    compiler = {
        "version": _text(
            compiler_raw.get("version"), f"{label}.configured_compiler.version", limit=128
        ),
        "sha256": _sha256(
            compiler_raw.get("sha256"), f"{label}.configured_compiler.sha256"
        ),
        "wrapper_sha256": _sha256(
            compiler_raw.get("wrapper_sha256"),
            f"{label}.configured_compiler.wrapper_sha256",
        ),
    }
    toolchain_raw = _closed(
        context.get("toolchain"),
        allowed={"dtk", "objdiff"},
        required={"dtk", "objdiff"},
        label=f"{label}.toolchain",
    )
    toolchain = {
        "dtk": _parse_tool_identity(toolchain_raw.get("dtk"), f"{label}.toolchain.dtk"),
        "objdiff": _parse_tool_identity(
            toolchain_raw.get("objdiff"), f"{label}.toolchain.objdiff"
        ),
    }

    candidate_raw = _closed(
        context.get("candidate"),
        allowed={"source_sha256", "object_sha256"},
        required={"source_sha256", "object_sha256"},
        label=f"{label}.candidate",
    )
    candidate = {
        field: _sha256(candidate_raw.get(field), f"{label}.candidate.{field}")
        for field in ("source_sha256", "object_sha256")
    }
    target_raw = _closed(
        context.get("target"),
        allowed={"object_sha256"},
        required={"object_sha256"},
        label=f"{label}.target",
    )
    target = {
        "object_sha256": _sha256(
            target_raw.get("object_sha256"), f"{label}.target.object_sha256"
        )
    }

    raw_inventory = context.get("opcode_inventory")
    if not isinstance(raw_inventory, list) or len(raw_inventory) < 2:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.opcode_inventory must contain at least two spans"
        )
    inventory: list[dict[str, Any]] = []
    inventory_by_id: dict[str, dict[str, Any]] = {}
    ranges_by_function: dict[str, list[tuple[int, int]]] = {}
    site_fields = {
        "id",
        "function",
        "object_start",
        "object_end",
        "bytes",
        "sha256",
        "operation",
        "aggregate_type",
        "target_mnemonics",
        "objdiff_canonical_sha256",
        "unresolved_residual",
        "producer_consumer_authenticated",
        "eligible",
        "exclusion_reason",
    }
    for index, raw in enumerate(raw_inventory):
        site_label = f"{label}.opcode_inventory[{index}]"
        site = _closed(raw, allowed=site_fields, required=site_fields, label=site_label)
        site_id = _text(site.get("id"), f"{site_label}.id", limit=128)
        if site_id in inventory_by_id:
            raise RepeatedOpcodeReadinessInputError(f"{site_label}.id is duplicated")
        function = _identifier(site.get("function"), f"{site_label}.function")
        start = _uint(site.get("object_start"), f"{site_label}.object_start")
        end = _uint(site.get("object_end"), f"{site_label}.object_end", minimum=1)
        if end <= start or (end - start) % 4:
            raise RepeatedOpcodeReadinessInputError(
                f"{site_label} must have a positive word-aligned extent"
            )
        for prior_start, prior_end in ranges_by_function.setdefault(function, []):
            if start < prior_end and prior_start < end:
                raise RepeatedOpcodeReadinessInputError(
                    f"{site_label} overlaps another span in {function}"
                )
        ranges_by_function[function].append((start, end))
        payload = _hex_bytes(site.get("bytes"), f"{site_label}.bytes")
        if len(payload) != end - start:
            raise RepeatedOpcodeReadinessInputError(
                f"{site_label}.bytes length does not match the object extent"
            )
        digest = _sha256(site.get("sha256"), f"{site_label}.sha256")
        if hashlib.sha256(payload).hexdigest() != digest:
            raise RepeatedOpcodeReadinessInputError(
                f"{site_label}.sha256 does not hash the supplied bytes"
            )
        raw_mnemonics = site.get("target_mnemonics")
        if not isinstance(raw_mnemonics, list) or not raw_mnemonics:
            raise RepeatedOpcodeReadinessInputError(
                f"{site_label}.target_mnemonics must be a non-empty list"
            )
        mnemonics = [
            _text(item, f"{site_label}.target_mnemonics[{i}]", limit=64)
            for i, item in enumerate(raw_mnemonics)
        ]
        if len(mnemonics) * 4 != len(payload):
            raise RepeatedOpcodeReadinessInputError(
                f"{site_label}.target_mnemonics must map one instruction per word"
            )
        unresolved = _bool(
            site.get("unresolved_residual"), f"{site_label}.unresolved_residual"
        )
        authenticated = _bool(
            site.get("producer_consumer_authenticated"),
            f"{site_label}.producer_consumer_authenticated",
        )
        eligible = _bool(site.get("eligible"), f"{site_label}.eligible")
        exclusion_reason = site.get("exclusion_reason")
        if eligible:
            if not unresolved or not authenticated or exclusion_reason is not None:
                raise RepeatedOpcodeReadinessInputError(
                    f"{site_label} eligible spans require unresolved authenticated evidence and no exclusion"
                )
        else:
            if not isinstance(exclusion_reason, str) or not exclusion_reason.strip():
                raise RepeatedOpcodeReadinessInputError(
                    f"{site_label} ineligible spans require an exclusion reason"
                )
            exclusion_reason = exclusion_reason.strip()
        normalized = {
            "id": site_id,
            "function": function,
            "object_start": start,
            "object_end": end,
            "bytes": payload.hex(),
            "sha256": digest,
            "operation": _text(site.get("operation"), f"{site_label}.operation", limit=128),
            "aggregate_type": _text(
                site.get("aggregate_type"), f"{site_label}.aggregate_type", limit=128
            ),
            "target_mnemonics": mnemonics,
            "objdiff_canonical_sha256": _sha256(
                site.get("objdiff_canonical_sha256"),
                f"{site_label}.objdiff_canonical_sha256",
            ),
            "unresolved_residual": unresolved,
            "producer_consumer_authenticated": authenticated,
            "eligible": eligible,
            "exclusion_reason": exclusion_reason,
        }
        inventory.append(normalized)
        inventory_by_id[site_id] = normalized
    inventory.sort(
        key=lambda item: (
            item["function"],
            item["object_start"],
            item["object_end"],
            item["id"],
        )
    )

    raw_groups = context.get("groups")
    if not isinstance(raw_groups, list) or not raw_groups:
        raise RepeatedOpcodeReadinessInputError(f"{label}.groups must be non-empty")
    groups: list[dict[str, Any]] = []
    assigned: set[str] = set()
    group_keys: set[tuple[str, str, str]] = set()
    group_fields = {
        "operation",
        "helper_symbol",
        "aggregate_type",
        "fingerprint_sha256",
        "site_ids",
        "semantic_contract",
        "expected_source_class",
        "target_mnemonics",
    }
    for index, raw in enumerate(raw_groups):
        group_label = f"{label}.groups[{index}]"
        group = _closed(raw, allowed=group_fields, required=group_fields, label=group_label)
        operation = _text(group.get("operation"), f"{group_label}.operation", limit=128)
        aggregate_type = _text(
            group.get("aggregate_type"), f"{group_label}.aggregate_type", limit=128
        )
        fingerprint = _sha256(
            group.get("fingerprint_sha256"), f"{group_label}.fingerprint_sha256"
        )
        key = (operation, aggregate_type, fingerprint)
        if key in group_keys:
            raise RepeatedOpcodeReadinessInputError(f"{group_label} duplicates another group")
        group_keys.add(key)
        site_ids = group.get("site_ids")
        if (
            not isinstance(site_ids, list)
            or len(site_ids) < 2
            or len(set(site_ids)) != len(site_ids)
        ):
            raise RepeatedOpcodeReadinessInputError(
                f"{group_label}.site_ids must contain at least two unique sites"
            )
        normalized_ids: list[str] = []
        group_mnemonics = group.get("target_mnemonics")
        if not isinstance(group_mnemonics, list) or not group_mnemonics:
            raise RepeatedOpcodeReadinessInputError(
                f"{group_label}.target_mnemonics must be non-empty"
            )
        normalized_mnemonics = [
            _text(item, f"{group_label}.target_mnemonics[{i}]", limit=64)
            for i, item in enumerate(group_mnemonics)
        ]
        for raw_site_id in site_ids:
            site_id = _text(raw_site_id, f"{group_label}.site_ids", limit=128)
            if site_id in assigned:
                raise RepeatedOpcodeReadinessInputError(
                    f"{group_label} assigns {site_id} more than once"
                )
            site = inventory_by_id.get(site_id)
            if site is None or not site["eligible"]:
                raise RepeatedOpcodeReadinessInputError(
                    f"{group_label} may assign only known eligible spans"
                )
            if (
                site["operation"] != operation
                or site["aggregate_type"] != aggregate_type
                or site["sha256"] != fingerprint
                or site["target_mnemonics"] != normalized_mnemonics
            ):
                raise RepeatedOpcodeReadinessInputError(
                    f"{group_label} does not exactly match site {site_id}"
                )
            assigned.add(site_id)
            normalized_ids.append(site_id)
        if _text(
            group.get("expected_source_class"), f"{group_label}.expected_source_class"
        ) != _SOURCE_CLASS:
            raise RepeatedOpcodeReadinessInputError(
                f"{group_label}.expected_source_class must be {_SOURCE_CLASS}"
            )
        groups.append(
            {
                "operation": operation,
                "helper_symbol": _identifier(
                    group.get("helper_symbol"), f"{group_label}.helper_symbol"
                ),
                "aggregate_type": aggregate_type,
                "fingerprint_sha256": fingerprint,
                "site_ids": sorted(normalized_ids),
                "semantic_contract": _text(
                    group.get("semantic_contract"),
                    f"{group_label}.semantic_contract",
                    limit=1024,
                ),
                "expected_source_class": _SOURCE_CLASS,
                "target_mnemonics": normalized_mnemonics,
            }
        )
    eligible_ids = {item["id"] for item in inventory if item["eligible"]}
    if assigned != eligible_ids:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.groups must assign every eligible span exactly once"
        )
    groups.sort(
        key=lambda item: (
            item["operation"],
            item["helper_symbol"],
            item["aggregate_type"],
            item["fingerprint_sha256"],
        )
    )

    exhaustion_raw = _closed(
        context.get("natural_c_exhaustion"),
        allowed={
            "bounded",
            "all_admissible_controls_exhausted",
            "unknown_evidence_used",
            "repeat_trace_required",
            "control_corpus_sha256",
            "controls",
        },
        required={
            "bounded",
            "all_admissible_controls_exhausted",
            "unknown_evidence_used",
            "repeat_trace_required",
            "control_corpus_sha256",
            "controls",
        },
        label=f"{label}.natural_c_exhaustion",
    )
    for field, expected in (
        ("bounded", True),
        ("all_admissible_controls_exhausted", True),
        ("unknown_evidence_used", False),
        ("repeat_trace_required", False),
    ):
        _bool(
            exhaustion_raw.get(field),
            f"{label}.natural_c_exhaustion.{field}",
            expected,
        )
    raw_controls = exhaustion_raw.get("controls")
    if not isinstance(raw_controls, list) or not raw_controls:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.natural_c_exhaustion.controls must be non-empty"
        )
    controls: list[dict[str, Any]] = []
    control_ids: set[str] = set()
    control_operations: set[str] = set()
    control_fields = {
        "id",
        "operation",
        "source_shape",
        "source_sha256",
        "object_sha256",
        "result_class",
        "target_sequence_emitted",
        "admissible",
    }
    for index, raw in enumerate(raw_controls):
        control_label = f"{label}.natural_c_exhaustion.controls[{index}]"
        control = _closed(
            raw, allowed=control_fields, required=control_fields, label=control_label
        )
        control_id = _text(control.get("id"), f"{control_label}.id", limit=128)
        if control_id in control_ids:
            raise RepeatedOpcodeReadinessInputError(f"{control_label}.id is duplicated")
        control_ids.add(control_id)
        operation = _text(control.get("operation"), f"{control_label}.operation", limit=128)
        result_class = _text(
            control.get("result_class"), f"{control_label}.result_class", limit=64
        )
        if result_class not in _CONTROL_RESULTS:
            raise RepeatedOpcodeReadinessInputError(
                f"{control_label}.result_class is not a sealed nonexact result"
            )
        _bool(
            control.get("target_sequence_emitted"),
            f"{control_label}.target_sequence_emitted",
            False,
        )
        _bool(control.get("admissible"), f"{control_label}.admissible", True)
        controls.append(
            {
                "id": control_id,
                "operation": operation,
                "source_shape": _text(
                    control.get("source_shape"), f"{control_label}.source_shape", limit=1024
                ),
                "source_sha256": _sha256(
                    control.get("source_sha256"), f"{control_label}.source_sha256"
                ),
                "object_sha256": _sha256(
                    control.get("object_sha256"), f"{control_label}.object_sha256"
                ),
                "result_class": result_class,
                "target_sequence_emitted": False,
                "admissible": True,
            }
        )
        control_operations.add(operation)
    controls.sort(key=lambda item: (item["operation"], item["id"]))
    group_operations = {item["operation"] for item in groups}
    if not group_operations <= control_operations:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.natural_c_exhaustion lacks a control for every operation"
        )
    control_corpus_sha256 = _sha256(
        exhaustion_raw.get("control_corpus_sha256"),
        f"{label}.natural_c_exhaustion.control_corpus_sha256",
    )
    if canonical_sha256(controls) != control_corpus_sha256:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.natural_c_exhaustion.control_corpus_sha256 does not hash controls"
        )
    exhaustion = {
        "bounded": True,
        "all_admissible_controls_exhausted": True,
        "unknown_evidence_used": False,
        "repeat_trace_required": False,
        "control_corpus_sha256": control_corpus_sha256,
        "controls": controls,
    }

    governed_raw = _closed(
        context.get("governed_low_level_source"),
        allowed={
            "source_class",
            "policy_sha256",
            "instance_request_sha256",
            "validator_sha256",
            "validation_receipt_sha256",
            "explicit_user_authorization",
            "validator_result",
            "symbolic_operands_only",
            "fixed_physical_registers",
            "raw_words",
            "object_patching",
            "authority_advanced",
        },
        required={
            "source_class",
            "policy_sha256",
            "instance_request_sha256",
            "validator_sha256",
            "validation_receipt_sha256",
            "explicit_user_authorization",
            "validator_result",
            "symbolic_operands_only",
            "fixed_physical_registers",
            "raw_words",
            "object_patching",
            "authority_advanced",
        },
        label=f"{label}.governed_low_level_source",
    )
    if _text(
        governed_raw.get("source_class"), f"{label}.governed_low_level_source.source_class"
    ) != _SOURCE_CLASS:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.governed_low_level_source.source_class must be {_SOURCE_CLASS}"
        )
    authorized = _bool(
        governed_raw.get("explicit_user_authorization"),
        f"{label}.governed_low_level_source.explicit_user_authorization",
    )
    validator_result = _text(
        governed_raw.get("validator_result"),
        f"{label}.governed_low_level_source.validator_result",
        limit=32,
    )
    receipt = governed_raw.get("validation_receipt_sha256")
    if authorized:
        if validator_result != "PASS" or receipt is None:
            raise RepeatedOpcodeReadinessInputError(
                f"{label}.governed_low_level_source authorized instances require PASS and a receipt"
            )
        receipt = _sha256(
            receipt, f"{label}.governed_low_level_source.validation_receipt_sha256"
        )
    else:
        if validator_result != "NOT_RUN" or receipt is not None:
            raise RepeatedOpcodeReadinessInputError(
                f"{label}.governed_low_level_source pending instances require NOT_RUN and no receipt"
            )
    for field, expected in (
        ("symbolic_operands_only", True),
        ("fixed_physical_registers", False),
        ("raw_words", False),
        ("object_patching", False),
        ("authority_advanced", False),
    ):
        _bool(
            governed_raw.get(field),
            f"{label}.governed_low_level_source.{field}",
            expected,
        )
    governed = {
        "source_class": _SOURCE_CLASS,
        "policy_sha256": _sha256(
            governed_raw.get("policy_sha256"),
            f"{label}.governed_low_level_source.policy_sha256",
        ),
        "instance_request_sha256": _sha256(
            governed_raw.get("instance_request_sha256"),
            f"{label}.governed_low_level_source.instance_request_sha256",
        ),
        "validator_sha256": _sha256(
            governed_raw.get("validator_sha256"),
            f"{label}.governed_low_level_source.validator_sha256",
        ),
        "validation_receipt_sha256": receipt,
        "explicit_user_authorization": authorized,
        "validator_result": validator_result,
        "symbolic_operands_only": True,
        "fixed_physical_registers": False,
        "raw_words": False,
        "object_patching": False,
        "authority_advanced": False,
    }

    exact_raw = _closed(
        context.get("exact_result"),
        allowed={
            "source_sha256",
            "object_sha256",
            "target_object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "focus_functions",
            "helpers",
            "functions_exact",
            "functions_total",
            "strict_diff_rows",
            "data_diff_rows",
            "physical_relocations",
            "relocation_identity",
            "protected_sibling_losses",
            "configured_outputs_exact",
            "configured_outputs_total",
            "main_dol_sha256",
            "main_dol_byte_identical",
        },
        required={
            "source_sha256",
            "object_sha256",
            "target_object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "focus_functions",
            "helpers",
            "functions_exact",
            "functions_total",
            "strict_diff_rows",
            "data_diff_rows",
            "physical_relocations",
            "relocation_identity",
            "protected_sibling_losses",
            "configured_outputs_exact",
            "configured_outputs_total",
            "main_dol_sha256",
            "main_dol_byte_identical",
        },
        label=f"{label}.exact_result",
    )
    exact_hashes = {
        field: _sha256(exact_raw.get(field), f"{label}.exact_result.{field}")
        for field in (
            "source_sha256",
            "object_sha256",
            "target_object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "main_dol_sha256",
        )
    }
    if (
        exact_hashes["source_sha256"] != candidate["source_sha256"]
        or exact_hashes["object_sha256"] != candidate["object_sha256"]
        or exact_hashes["target_object_sha256"] != target["object_sha256"]
    ):
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.exact_result identities must match candidate and target"
        )
    focus_functions = exact_raw.get("focus_functions")
    helpers = exact_raw.get("helpers")
    if not isinstance(focus_functions, list) or not isinstance(helpers, list):
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.exact_result focus_functions/helpers must be lists"
        )
    normalized_functions = [
        _identifier(item, f"{label}.exact_result.focus_functions")
        for item in focus_functions
    ]
    normalized_helpers = [
        _identifier(item, f"{label}.exact_result.helpers") for item in helpers
    ]
    expected_functions = sorted({item["function"] for item in inventory if item["eligible"]})
    expected_helpers = sorted({item["helper_symbol"] for item in groups})
    if sorted(normalized_functions) != expected_functions or sorted(normalized_helpers) != expected_helpers:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.exact_result must cover exactly the eligible functions and helpers"
        )
    functions_exact = _uint(
        exact_raw.get("functions_exact"), f"{label}.exact_result.functions_exact", minimum=1
    )
    functions_total = _uint(
        exact_raw.get("functions_total"), f"{label}.exact_result.functions_total", minimum=1
    )
    outputs_exact = _uint(
        exact_raw.get("configured_outputs_exact"),
        f"{label}.exact_result.configured_outputs_exact",
        minimum=1,
    )
    outputs_total = _uint(
        exact_raw.get("configured_outputs_total"),
        f"{label}.exact_result.configured_outputs_total",
        minimum=1,
    )
    if functions_exact != functions_total or outputs_exact != outputs_total:
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.exact_result must close all functions and configured outputs"
        )
    for field in ("strict_diff_rows", "data_diff_rows", "protected_sibling_losses"):
        if _uint(exact_raw.get(field), f"{label}.exact_result.{field}") != 0:
            raise RepeatedOpcodeReadinessInputError(f"{label}.exact_result.{field} must be zero")
    _bool(
        exact_raw.get("relocation_identity"),
        f"{label}.exact_result.relocation_identity",
        True,
    )
    _bool(
        exact_raw.get("main_dol_byte_identical"),
        f"{label}.exact_result.main_dol_byte_identical",
        True,
    )
    exact = {
        **exact_hashes,
        "focus_functions": sorted(normalized_functions),
        "helpers": sorted(normalized_helpers),
        "functions_exact": functions_exact,
        "functions_total": functions_total,
        "strict_diff_rows": 0,
        "data_diff_rows": 0,
        "physical_relocations": _uint(
            exact_raw.get("physical_relocations"),
            f"{label}.exact_result.physical_relocations",
            minimum=1,
        ),
        "relocation_identity": True,
        "protected_sibling_losses": 0,
        "configured_outputs_exact": outputs_exact,
        "configured_outputs_total": outputs_total,
        "main_dol_byte_identical": True,
    }

    telemetry_raw = _closed(
        context.get("telemetry"),
        allowed={
            "telemetry_complete",
            "excluded_from_measured_crack_per_hour",
            "no_imputation",
            "interval_log_sha256",
        },
        required={
            "telemetry_complete",
            "excluded_from_measured_crack_per_hour",
            "no_imputation",
            "interval_log_sha256",
        },
        label=f"{label}.telemetry",
    )
    telemetry_complete = _bool(
        telemetry_raw.get("telemetry_complete"), f"{label}.telemetry.telemetry_complete"
    )
    excluded = _bool(
        telemetry_raw.get("excluded_from_measured_crack_per_hour"),
        f"{label}.telemetry.excluded_from_measured_crack_per_hour",
    )
    no_imputation = _bool(
        telemetry_raw.get("no_imputation"), f"{label}.telemetry.no_imputation"
    )
    if not telemetry_complete and (not excluded or not no_imputation):
        raise RepeatedOpcodeReadinessInputError(
            f"{label}.telemetry incomplete evidence must be excluded without imputation"
        )
    telemetry = {
        "telemetry_complete": telemetry_complete,
        "excluded_from_measured_crack_per_hour": excluded,
        "no_imputation": no_imputation,
        "interval_log_sha256": _sha256(
            telemetry_raw.get("interval_log_sha256"),
            f"{label}.telemetry.interval_log_sha256",
        ),
    }

    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": _sha256(
            context.get("report_artifact_sha256"), f"{label}.report_artifact_sha256"
        ),
        "owner": _text(context.get("owner"), f"{label}.owner", limit=256),
        "configured_compiler": compiler,
        "toolchain": toolchain,
        "candidate": candidate,
        "target": target,
        "opcode_inventory": inventory,
        "groups": groups,
        "natural_c_exhaustion": exhaustion,
        "governed_low_level_source": governed,
        "exact_result": exact,
        "telemetry": telemetry,
        "authority_advanced": False,
    }


def evaluate(
    context: Mapping[str, Any], *, focus_symbol: str | None = None,
    objdiff_canonical_sha256: str | None = None,
) -> dict[str, Any]:
    if (focus_symbol is None) != (objdiff_canonical_sha256 is None):
        raise RepeatedOpcodeReadinessInputError(
            "focus_symbol and objdiff_canonical_sha256 must be supplied together"
        )
    if focus_symbol is not None:
        focus_symbol = _identifier(focus_symbol, "focus_symbol")
        objdiff_canonical_sha256 = _sha256(
            objdiff_canonical_sha256, "objdiff_canonical_sha256"
        )
    normalized = parse_context(context)
    eligible = [item for item in normalized["opcode_inventory"] if item["eligible"]]
    if focus_symbol is not None:
        sites = [item for item in eligible if item["function"] == focus_symbol]
        if not sites:
            return {"matched": False, "reason": "the context is bound to other functions"}
        if objdiff_canonical_sha256 is None or all(
            item["objdiff_canonical_sha256"] != objdiff_canonical_sha256 for item in sites
        ):
            return {
                "matched": False,
                "reason": "the repeated-opcode context is bound to another objdiff report",
            }
    governed = normalized["governed_low_level_source"]
    authorized = governed["explicit_user_authorization"]
    result = {
        "schema": RESULT_SCHEMA,
        "matched": True,
        "status": (
            "AUTHORIZED_VALIDATED_INSTANCE"
            if authorized
            else "READY_FOR_EXPLICIT_AUTHORIZATION"
        ),
        "source_class": _SOURCE_CLASS,
        "owner": normalized["owner"],
        "groups": [
            {
                "operation": group["operation"],
                "helper_symbol": group["helper_symbol"],
                "aggregate_type": group["aggregate_type"],
                "fingerprint_sha256": group["fingerprint_sha256"],
                "site_count": len(group["site_ids"]),
                "sites": [inventory_id for inventory_id in group["site_ids"]],
                "target_mnemonics": group["target_mnemonics"],
            }
            for group in normalized["groups"]
        ],
        "recommendation": (
            "The governed instance is hash-bound and validator-PASS; retain its authorization "
            "as instance-scoped evidence only."
            if authorized
            else "Request explicit user authorization for this hash-bound governed instance; "
            "do not compile or retain it automatically."
        ),
        "candidate_scheduled": False,
        "suppressed_axes": [
            "additional_natural_c_permutations_after_sealed_exhaustion",
            "repeat_tracer_capture",
            "dead_or_fake_local",
            "padding",
            "fixed_register_source",
            "raw_opcode_words",
            "object_patching",
            "automatic_authorization_retention_or_promotion",
        ],
        "evidence": {
            "report_artifact_sha256": normalized["report_artifact_sha256"],
            "configured_compiler": normalized["configured_compiler"],
            "toolchain": normalized["toolchain"],
            "candidate": normalized["candidate"],
            "target": normalized["target"],
            "natural_c_exhaustion": normalized["natural_c_exhaustion"],
            "governed_low_level_source": governed,
            "exact_result": normalized["exact_result"],
            "telemetry": normalized["telemetry"],
            "context_canonical_sha256": canonical_sha256(normalized),
        },
        "authority_advanced": False,
    }
    result[HASH_FIELD] = canonical_sha256(result)
    return result


def _load_json(path: Path) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise RepeatedOpcodeReadinessInputError(f"cannot read {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise RepeatedOpcodeReadinessInputError(f"{path} must contain a JSON object")
    return value


def _write_atomic(path: Path, payload: str) -> None:
    temporary: str | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
            temporary = stream.name
        os.replace(temporary, path)
        temporary = None
    finally:
        if temporary is not None:
            try:
                os.unlink(temporary)
            except FileNotFoundError:
                pass


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", required=True, type=Path)
    parser.add_argument("--focus-symbol")
    parser.add_argument("--objdiff-canonical-sha256")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    try:
        result = evaluate(
            _load_json(args.context),
            focus_symbol=args.focus_symbol,
            objdiff_canonical_sha256=args.objdiff_canonical_sha256,
        )
    except RepeatedOpcodeReadinessInputError as exc:
        parser.error(str(exc))
    payload = json.dumps(result, sort_keys=True, indent=2) + "\n"
    if args.output is None:
        print(payload, end="")
    else:
        _write_atomic(args.output, payload)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
