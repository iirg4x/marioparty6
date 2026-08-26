#!/usr/bin/env python3
"""Compose proven CRACK_REPORT lessons with the causal objdiff reducer.

The rules in this module are intentionally evidence-only.  They recognize
narrow instruction/topology signatures, expose the evidence and confidence
used for each diagnosis, and recommend only natural source-shape classes.
They never edit source, retain a candidate, or advance recovery authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import candidate_interaction_planner as interaction_planner
from tools import mismatch_cluster_audit as causal_reducer


SCHEMA = "crack_learning_diagnosis/v8"
SCHEMA_VERSION = 8
HASH_FIELD = "diagnosis_sha256"
ALLOCATOR_CONTEXT_SCHEMA = "allocator_two_register_swap_context/v1"
PARAMETER_ALLOCATION_CONTEXT_SCHEMA = "parameter_allocation_consumer_chain_context/v1"
AGGREGATE_USE_CONTEXT_SCHEMA = "aggregate_use_multiplicity_context/v1"
AGGREGATE_FOLLOWUP_CONTEXT_SCHEMA = "aggregate_two_owner_followup_context/v1"
ADDRESS_TAKEN_CONTEXT_SCHEMA = "address_taken_local_pointer_context/v1"
SAME_TU_SHAPE_CONTEXT_SCHEMA = "same_tu_exact_sibling_shape_context/v1"
SHORT_CIRCUIT_CONTEXT_SCHEMA = "short_circuit_boolean_call_order_context/v1"
EXACT_SIBLING_TRANSFER_CONTEXT_SCHEMA = (
    "dependency_equivalent_exact_sibling_transfer_context/v1"
)
CAPACITY_CONTEXT_SCHEMA = "stack_extent_interface_capacity_context/v1"
BRANCH_CONTEXT_SCHEMA = "loop_branch_destination_context/v1"
RECIPROCAL_CONTEXT_SCHEMA = "reciprocal_source_shape_context/v1"

_REGISTER_RE = re.compile(r"\b(?P<kind>[rRfF])(?P<number>[0-9]|[12][0-9]|3[01])\b")
_STACK_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)",
    re.IGNORECASE,
)
_MEMORY_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*"
    r"(?P<base>r(?:[0-9]|[12][0-9]|3[01]))\s*\)",
    re.IGNORECASE,
)
_ADDI_R1_RE = re.compile(
    r"^\s*addi\s+(?P<destination>r(?:[0-9]|[12][0-9]|3[01]))\s*,\s*r1\s*,\s*"
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*$",
    re.IGNORECASE,
)
_CALL_MNEMONICS = frozenset({"bl", "bla", "bctrl", "blrl"})
_CONDITIONAL_MNEMONICS = frozenset(
    {
        "bc",
        "bca",
        "beq",
        "beqa",
        "bge",
        "bgea",
        "bgt",
        "bgta",
        "ble",
        "blea",
        "blt",
        "blta",
        "bne",
        "bnea",
        "bso",
        "bns",
        "bdnz",
        "bdz",
    }
)
_SWITCH_MNEMONICS = frozenset({"bctr", "bcctr"})
_AGGREGATE_LOADS = frozenset({"lfs", "lfd", "lwz", "lhz", "lha", "lbz"})
_AGGREGATE_STORES = frozenset({"stfs", "stfd", "stw", "sth", "stb"})
_SOURCE_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_SOURCE_LVALUE_RE = re.compile(
    r"[A-Za-z_][A-Za-z0-9_]{0,127}(?:(?:->|\.)[A-Za-z_][A-Za-z0-9_]{0,127}){0,4}"
)
_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_ALLOCATOR_PROOF_FLAGS = (
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "stack_frame_exact",
    "protected_siblings_preserved",
)
_ALLOCATOR_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "varinfo_receipt_sha256",
    "source_boundary_receipt_sha256",
)
_PARAMETER_ALLOCATION_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "protected_siblings_preserved",
)
_PARAMETER_ALLOCATION_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "trace_receipt_sha256",
    "source_boundary_receipt_sha256",
    "same_tu_donor_receipt_sha256",
)
_AGGREGATE_USE_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "protected_siblings_preserved",
)
_AGGREGATE_USE_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "source_use_receipt_sha256",
    "trace_receipt_sha256",
    "exact_precedent_receipt_sha256",
)
_AGGREGATE_FOLLOWUP_PROOF_FLAGS = _AGGREGATE_USE_PROOF_FLAGS
_AGGREGATE_FOLLOWUP_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "aggregate_reconstruction_receipt_sha256",
    "fusion_observation_receipt_sha256",
)
_ADDRESS_TAKEN_PROOF_FLAGS = (
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "protected_siblings_preserved",
)
_ADDRESS_TAKEN_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "source_boundary_receipt_sha256",
    "typed_consumer_receipt_sha256",
)
_SAME_TU_SHAPE_PROOF_FLAGS = (
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "protected_siblings_preserved",
    "donor_strict_exact",
    "donor_data_exact",
    "caller_contract_authenticated",
)
_SAME_TU_SHAPE_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "same_tu_donor_receipt_sha256",
    "caller_contract_receipt_sha256",
    "source_shape_receipt_sha256",
)
_SHORT_CIRCUIT_PROOF_FLAGS = (
    "physical_relocations_exact",
    "data_sections_exact",
    "protected_siblings_preserved",
    "pinned_mwcc_frontend",
    "topology_observation_size_exact",
    "topology_observation_cfg_exact",
    "topology_observation_relocations_exact",
)
_SHORT_CIRCUIT_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "call_order_receipt_sha256",
    "topology_observation_report_sha256",
    "declaration_owner_receipt_sha256",
)
_EXACT_SIBLING_TRANSFER_PROOF_FLAGS = (
    "physical_relocations_exact",
    "data_sections_exact",
    "protected_siblings_preserved",
    "donor_strict_exact",
    "donor_data_exact",
    "dependency_graph_equivalent",
    "capacity_authenticated",
    "pinned_mwcc_frontend",
)
_EXACT_SIBLING_TRANSFER_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "donor_record_sha256",
    "dependency_graph_receipt_sha256",
    "capacity_receipt_sha256",
    "type_boundary_receipt_sha256",
)
_CAPACITY_PROOF_FLAGS = (
    "function_size_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "all_non_extent_structure_exact",
    "protected_siblings_preserved",
)
_CAPACITY_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "stack_extent_receipt_sha256",
    "interface_contract_receipt_sha256",
)
_BRANCH_PROOF_FLAGS = (
    "function_size_exact",
    "stack_frame_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "all_non_branch_rows_exact",
    "protected_siblings_preserved",
)
_BRANCH_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "branch_destination_receipt_sha256",
)
_RECIPROCAL_PROOF_FLAGS = (
    "function_size_exact",
    "data_values_exact",
    "physical_relocations_exact",
    "cfg_calls_exact",
    "all_non_window_rows_exact",
    "protected_siblings_preserved",
)
_RECIPROCAL_PROOF_HASHES = (
    "objdiff_canonical_sha256",
    "strict_report_sha256",
    "data_report_sha256",
    "physical_relocation_receipt_sha256",
    "typed_constant_receipt_sha256",
    "neutral_observation_receipt_sha256",
)

_RULE_ORDER = (
    "explicit_else_return_cfg",
    "loop_branch_destination",
    "assignment_condition_saved_gpr_cycle",
    "allocator_two_register_swap_interaction",
    "parameter_allocation_consumer_chain",
    "aggregate_use_multiplicity",
    "aggregate_two_owner_followup",
    "address_taken_local_pointer_consumer",
    "same_tu_exact_sibling_source_shapes",
    "short_circuit_boolean_call_order",
    "dependency_equivalent_exact_sibling_transfer",
    "stack_extent_interface_capacity",
    "reciprocal_source_shape",
    "switch_case_scoped_fpr_lifetimes",
    "aggregate_self_copy_final_consumer",
)


class LearningInputError(ValueError):
    """An input cannot support a closed, deterministic diagnosis."""


def _canonical(value: Any) -> bytes:
    try:
        return json.dumps(
            value,
            sort_keys=True,
            separators=(",", ":"),
            ensure_ascii=True,
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise LearningInputError(f"input is not canonical JSON: {exc}") from exc


def _sha256(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _with_self_hash(value: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop(HASH_FIELD, None)
    result[HASH_FIELD] = _sha256(_canonical(result))
    return result


def _registers(text: str, kind: str | None = None) -> list[str]:
    result: list[str] = []
    for match in _REGISTER_RE.finditer(text):
        register = f"{match.group('kind').lower()}{int(match.group('number'))}"
        if kind is None or register.startswith(kind):
            result.append(register)
    return result


def _saved(register: str, kind: str) -> bool:
    return register.startswith(kind) and 14 <= int(register[1:]) <= 31


def _without_registers(text: str) -> str:
    return _REGISTER_RE.sub("<reg>", text.lower()).strip()


def _stack_offset(text: str) -> int | None:
    match = _STACK_RE.search(text)
    if match is None:
        return None
    return causal_reducer._parse_number(match.group("offset"))


def _memory_operand(text: str) -> tuple[str, int] | None:
    match = _MEMORY_RE.search(text)
    if match is None:
        return None
    return (
        match.group("base").lower(),
        causal_reducer._parse_number(match.group("offset")),
    )


def _addi_r1_materialization(text: str) -> tuple[str, int] | None:
    match = _ADDI_R1_RE.fullmatch(text)
    if match is None:
        return None
    return match.group("destination").lower(), int(match.group("offset"), 0)


def _pair(document: Mapping[str, Any], symbol: str) -> causal_reducer.FunctionPair:
    try:
        return causal_reducer._focus_pairs(
            causal_reducer._paired_functions(document), symbol
        )[0]
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(
            f"objdiff report rejected ({exc.code}): {exc.message}"
        ) from exc


def _entries(
    pair: causal_reducer.FunctionPair,
) -> tuple[list[causal_reducer.Instruction], list[causal_reducer.Instruction]]:
    try:
        return (
            causal_reducer._entries(pair.target, "target", pair.name),
            causal_reducer._entries(pair.candidate, "candidate", pair.name),
        )
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(
            f"objdiff report rejected ({exc.code}): {exc.message}"
        ) from exc


def _function_size(symbol: Mapping[str, Any] | None) -> int | None:
    if symbol is None:
        return None
    return causal_reducer._parse_number(symbol.get("size"))


def _closed_context(
    value: Any,
    *,
    allowed: set[str],
    required: set[str],
    label: str,
) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise LearningInputError(f"{label} must be an object")
    unknown = sorted(set(value) - allowed)
    if unknown:
        raise LearningInputError(f"{label} contains unknown field {unknown[0]!r}")
    missing = sorted(required - set(value))
    if missing:
        raise LearningInputError(f"{label} lacks required field {missing[0]!r}")
    return value


def _context_text(value: Any, label: str, *, limit: int = 256) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise LearningInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise LearningInputError(f"{label} exceeds {limit} characters")
    return result


def _context_identifier(value: Any, label: str) -> str:
    result = _context_text(value, label, limit=128)
    if _SOURCE_IDENTIFIER_RE.fullmatch(result) is None:
        raise LearningInputError(f"{label} must be a C source identifier")
    return result


def _context_lvalue(value: Any, label: str) -> str:
    result = _context_text(value, label, limit=512)
    if _SOURCE_LVALUE_RE.fullmatch(result) is None:
        raise LearningInputError(
            f"{label} must be a bounded C identifier/member lvalue"
        )
    return result


def _context_sha256(value: Any, label: str) -> str:
    result = _context_text(value, label, limit=64)
    if result != result.lower():
        raise LearningInputError(f"{label} must be lowercase")
    if _SHA256_RE.fullmatch(result) is None:
        raise LearningInputError(f"{label} must be a lowercase SHA-256 digest")
    return result


def _context_uint(
    value: Any,
    label: str,
    *,
    minimum: int = 0,
    maximum: int = 1_000_000,
) -> int:
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or not minimum <= value <= maximum
    ):
        raise LearningInputError(
            f"{label} must be an integer from {minimum} through {maximum}"
        )
    return value


def _context_rows(
    value: Any,
    label: str,
    *,
    minimum_count: int = 1,
    maximum_count: int = 16,
) -> list[int]:
    if not isinstance(value, list) or not minimum_count <= len(value) <= maximum_count:
        raise LearningInputError(
            f"{label} must contain {minimum_count}-{maximum_count} row indices"
        )
    rows = [
        _context_uint(item, f"{label}[{index}]")
        for index, item in enumerate(value)
    ]
    if rows != sorted(set(rows)):
        raise LearningInputError(f"{label} must be sorted and unique")
    return rows


def _parse_allocator_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={"schema", "proofs", "owners", "boundary", "observations"},
        required={"schema", "proofs", "owners", "boundary"},
        label="allocator context",
    )
    if (
        _context_text(context.get("schema"), "allocator context schema")
        != ALLOCATOR_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"allocator context schema must be {ALLOCATOR_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_ALLOCATOR_PROOF_FLAGS) | set(_ALLOCATOR_PROOF_HASHES)
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="allocator context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _ALLOCATOR_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(f"allocator context proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _ALLOCATOR_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"allocator context proofs.{field}"
        )

    raw_owners = context.get("owners")
    if not isinstance(raw_owners, list) or len(raw_owners) != 2:
        raise LearningInputError(
            "allocator context owners must contain exactly two entries"
        )
    owners: list[dict[str, Any]] = []
    owner_fields = {
        "name",
        "usage_class",
        "target_register",
        "candidate_register",
        "lifetime_role",
        "evidence_sha256",
    }
    for index, raw_owner in enumerate(raw_owners):
        owner = _closed_context(
            raw_owner,
            allowed=owner_fields,
            required=owner_fields,
            label=f"allocator context owners[{index}]",
        )
        usage_class = owner.get("usage_class")
        if (
            isinstance(usage_class, bool)
            or not isinstance(usage_class, int)
            or not 0 <= usage_class <= 1_000_000
        ):
            raise LearningInputError(
                f"allocator context owners[{index}].usage_class must be a non-negative integer"
            )
        target_register = _context_text(
            owner.get("target_register"),
            f"allocator context owners[{index}].target_register",
            limit=3,
        ).lower()
        candidate_register = _context_text(
            owner.get("candidate_register"),
            f"allocator context owners[{index}].candidate_register",
            limit=3,
        ).lower()
        if not _saved(target_register, "r") or not _saved(candidate_register, "r"):
            raise LearningInputError(
                f"allocator context owners[{index}] registers must be nonvolatile GPRs"
            )
        lifetime_role = _context_text(
            owner.get("lifetime_role"),
            f"allocator context owners[{index}].lifetime_role",
        )
        if lifetime_role not in {"long_lived", "producer_consumer_boundary"}:
            raise LearningInputError(
                f"allocator context owners[{index}].lifetime_role is unsupported"
            )
        owners.append(
            {
                "name": _context_identifier(
                    owner.get("name"), f"allocator context owners[{index}].name"
                ),
                "usage_class": usage_class,
                "target_register": target_register,
                "candidate_register": candidate_register,
                "lifetime_role": lifetime_role,
                "evidence_sha256": _context_sha256(
                    owner.get("evidence_sha256"),
                    f"allocator context owners[{index}].evidence_sha256",
                ),
            }
        )
    for field in (
        "name",
        "usage_class",
        "target_register",
        "candidate_register",
        "lifetime_role",
    ):
        if len({owner[field] for owner in owners}) != 2:
            raise LearningInputError(
                f"allocator context owner {field} values must be distinct"
            )

    boundary = _closed_context(
        context.get("boundary"),
        allowed={"producer", "consumer", "transformations", "evidence_sha256"},
        required={"producer", "consumer", "transformations", "evidence_sha256"},
        label="allocator context boundary",
    )
    transformations = boundary.get("transformations")
    if not isinstance(transformations, list) or not 1 <= len(transformations) <= 8:
        raise LearningInputError(
            "allocator context boundary.transformations must contain 1-8 entries"
        )
    normalized_transformations = [
        _context_text(
            item, f"allocator context boundary.transformations[{index}]", limit=128
        )
        for index, item in enumerate(transformations)
    ]
    if len(set(normalized_transformations)) != len(normalized_transformations):
        raise LearningInputError(
            "allocator context boundary.transformations must be unique"
        )
    normalized_boundary = {
        "producer": _context_text(
            boundary.get("producer"), "allocator context boundary.producer"
        ),
        "consumer": _context_text(
            boundary.get("consumer"), "allocator context boundary.consumer"
        ),
        "transformations": normalized_transformations,
        "evidence_sha256": _context_sha256(
            boundary.get("evidence_sha256"),
            "allocator context boundary.evidence_sha256",
        ),
    }

    observations = context.get("observations", [])
    if not isinstance(observations, list) or len(observations) > 4:
        raise LearningInputError(
            "allocator context observations must contain at most four entries"
        )
    if any(not isinstance(item, dict) for item in observations):
        raise LearningInputError("allocator context observations must contain objects")
    return {
        "schema": ALLOCATOR_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "owners": sorted(owners, key=lambda item: item["name"]),
        "boundary": normalized_boundary,
        "observations": [dict(item) for item in observations],
    }


def _parse_parameter_allocation_context(
    value: Mapping[str, Any],
) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={"schema", "proofs", "owners", "producer", "consumer_chain"},
        required={"schema", "proofs", "owners", "producer", "consumer_chain"},
        label="parameter allocation context",
    )
    if (
        _context_text(context.get("schema"), "parameter allocation context schema")
        != PARAMETER_ALLOCATION_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            "parameter allocation context schema must be "
            f"{PARAMETER_ALLOCATION_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_PARAMETER_ALLOCATION_PROOF_FLAGS) | set(
        _PARAMETER_ALLOCATION_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="parameter allocation context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _PARAMETER_ALLOCATION_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"parameter allocation context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _PARAMETER_ALLOCATION_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"parameter allocation context proofs.{field}"
        )

    owners = _closed_context(
        context.get("owners"),
        allowed={"parameter", "allocation_result"},
        required={"parameter", "allocation_result"},
        label="parameter allocation context owners",
    )
    normalized_owners: dict[str, dict[str, Any]] = {}
    owner_fields = {
        "name",
        "target_register",
        "candidate_register",
        "evidence_sha256",
    }
    for role in ("parameter", "allocation_result"):
        owner = _closed_context(
            owners.get(role),
            allowed=owner_fields,
            required=owner_fields,
            label=f"parameter allocation context owners.{role}",
        )
        target_register = _context_text(
            owner.get("target_register"),
            f"parameter allocation context owners.{role}.target_register",
            limit=3,
        ).lower()
        candidate_register = _context_text(
            owner.get("candidate_register"),
            f"parameter allocation context owners.{role}.candidate_register",
            limit=3,
        ).lower()
        if not _saved(target_register, "r") or not _saved(candidate_register, "r"):
            raise LearningInputError(
                f"parameter allocation context owners.{role} registers must be nonvolatile GPRs"
            )
        normalized_owners[role] = {
            "name": _context_identifier(
                owner.get("name"),
                f"parameter allocation context owners.{role}.name",
            ),
            "target_register": target_register,
            "candidate_register": candidate_register,
            "evidence_sha256": _context_sha256(
                owner.get("evidence_sha256"),
                f"parameter allocation context owners.{role}.evidence_sha256",
            ),
        }
    for field in ("name", "target_register", "candidate_register"):
        if len({owner[field] for owner in normalized_owners.values()}) != 2:
            raise LearningInputError(
                f"parameter allocation context owner {field} values must be distinct"
            )

    producer = _closed_context(
        context.get("producer"),
        allowed={
            "call_name",
            "call_row",
            "capture_row",
            "return_register",
            "preserve_explicit_identity",
            "evidence_sha256",
        },
        required={
            "call_name",
            "call_row",
            "capture_row",
            "return_register",
            "preserve_explicit_identity",
            "evidence_sha256",
        },
        label="parameter allocation context producer",
    )
    return_register = _context_text(
        producer.get("return_register"),
        "parameter allocation context producer.return_register",
        limit=3,
    ).lower()
    if return_register != "r3":
        raise LearningInputError(
            "parameter allocation context producer.return_register must be r3"
        )
    if producer.get("preserve_explicit_identity") is not True:
        raise LearningInputError(
            "parameter allocation context producer.preserve_explicit_identity must be true"
        )
    normalized_producer = {
        "call_name": _context_identifier(
            producer.get("call_name"),
            "parameter allocation context producer.call_name",
        ),
        "call_row": _context_uint(
            producer.get("call_row"),
            "parameter allocation context producer.call_row",
        ),
        "capture_row": _context_uint(
            producer.get("capture_row"),
            "parameter allocation context producer.capture_row",
        ),
        "return_register": return_register,
        "preserve_explicit_identity": True,
        "evidence_sha256": _context_sha256(
            producer.get("evidence_sha256"),
            "parameter allocation context producer.evidence_sha256",
        ),
    }
    if normalized_producer["capture_row"] != normalized_producer["call_row"] + 1:
        raise LearningInputError(
            "parameter allocation context producer capture must immediately follow the call"
        )

    chain = _closed_context(
        context.get("consumer_chain"),
        allowed={
            "typed_pointer",
            "field_owner",
            "field_name",
            "allocation_result",
            "evaluation_order",
            "consumer_rows",
            "evidence_sha256",
        },
        required={
            "typed_pointer",
            "field_owner",
            "field_name",
            "allocation_result",
            "evaluation_order",
            "consumer_rows",
            "evidence_sha256",
        },
        label="parameter allocation context consumer_chain",
    )
    evaluation_order = chain.get("evaluation_order")
    if evaluation_order != ["field_store", "typed_pointer_copy"]:
        raise LearningInputError(
            "parameter allocation context consumer_chain.evaluation_order must be "
            "['field_store', 'typed_pointer_copy']"
        )
    raw_rows = chain.get("consumer_rows")
    if not isinstance(raw_rows, list) or len(raw_rows) != 2:
        raise LearningInputError(
            "parameter allocation context consumer_chain.consumer_rows must contain two entries"
        )
    consumer_rows = [
        _context_uint(
            item,
            f"parameter allocation context consumer_chain.consumer_rows[{index}]",
        )
        for index, item in enumerate(raw_rows)
    ]
    if consumer_rows[1] != consumer_rows[0] + 1:
        raise LearningInputError(
            "parameter allocation context consumer rows must be adjacent and ordered"
        )
    allocation_name = _context_identifier(
        chain.get("allocation_result"),
        "parameter allocation context consumer_chain.allocation_result",
    )
    if allocation_name != normalized_owners["allocation_result"]["name"]:
        raise LearningInputError(
            "parameter allocation context consumer allocation identity must match its owner"
        )
    normalized_chain = {
        "typed_pointer": _context_identifier(
            chain.get("typed_pointer"),
            "parameter allocation context consumer_chain.typed_pointer",
        ),
        "field_owner": _context_identifier(
            chain.get("field_owner"),
            "parameter allocation context consumer_chain.field_owner",
        ),
        "field_name": _context_identifier(
            chain.get("field_name"),
            "parameter allocation context consumer_chain.field_name",
        ),
        "allocation_result": allocation_name,
        "evaluation_order": list(evaluation_order),
        "consumer_rows": consumer_rows,
        "evidence_sha256": _context_sha256(
            chain.get("evidence_sha256"),
            "parameter allocation context consumer_chain.evidence_sha256",
        ),
    }
    return {
        "schema": PARAMETER_ALLOCATION_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "owners": normalized_owners,
        "producer": normalized_producer,
        "consumer_chain": normalized_chain,
    }


def _parse_aggregate_use_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "owners",
            "aggregate_parameter",
            "copy_groups",
            "independent_consumers",
            "rejected_axes",
        },
        required={
            "schema",
            "proofs",
            "owners",
            "aggregate_parameter",
            "copy_groups",
            "independent_consumers",
            "rejected_axes",
        },
        label="aggregate-use context",
    )
    if (
        _context_text(context.get("schema"), "aggregate-use context schema")
        != AGGREGATE_USE_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"aggregate-use context schema must be {AGGREGATE_USE_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_AGGREGATE_USE_PROOF_FLAGS) | set(_AGGREGATE_USE_PROOF_HASHES)
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="aggregate-use context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _AGGREGATE_USE_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"aggregate-use context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _AGGREGATE_USE_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"aggregate-use context proofs.{field}"
        )

    raw_owners = context.get("owners")
    if not isinstance(raw_owners, list) or not 2 <= len(raw_owners) <= 16:
        raise LearningInputError(
            "aggregate-use context owners must contain two through sixteen entries"
        )
    owner_fields = {
        "name",
        "target_register",
        "candidate_register",
        "evidence_sha256",
    }
    owners: list[dict[str, Any]] = []
    for index, raw_owner in enumerate(raw_owners):
        owner = _closed_context(
            raw_owner,
            allowed=owner_fields,
            required=owner_fields,
            label=f"aggregate-use context owners[{index}]",
        )
        target_register = _context_text(
            owner.get("target_register"),
            f"aggregate-use context owners[{index}].target_register",
            limit=3,
        ).lower()
        candidate_register = _context_text(
            owner.get("candidate_register"),
            f"aggregate-use context owners[{index}].candidate_register",
            limit=3,
        ).lower()
        if not _saved(target_register, "r") or not _saved(candidate_register, "r"):
            raise LearningInputError(
                "aggregate-use context owner registers must be nonvolatile GPRs"
            )
        owners.append(
            {
                "name": _context_identifier(
                    owner.get("name"), f"aggregate-use context owners[{index}].name"
                ),
                "target_register": target_register,
                "candidate_register": candidate_register,
                "evidence_sha256": _context_sha256(
                    owner.get("evidence_sha256"),
                    f"aggregate-use context owners[{index}].evidence_sha256",
                ),
            }
        )
    for field in ("name", "target_register", "candidate_register"):
        if len({owner[field] for owner in owners}) != len(owners):
            raise LearningInputError(
                f"aggregate-use context owner {field} values must be unique"
            )
    owner_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in owners
    }
    cycles = _closed_cycles(owner_mapping)
    if len(cycles) != 1 or len(cycles[0]) != len(owners):
        raise LearningInputError(
            "aggregate-use context owners must describe one complete register cycle"
        )

    aggregate = _closed_context(
        context.get("aggregate_parameter"),
        allowed={
            "name",
            "type",
            "fields",
            "target_register",
            "candidate_register",
            "evidence_sha256",
        },
        required={
            "name",
            "type",
            "fields",
            "target_register",
            "candidate_register",
            "evidence_sha256",
        },
        label="aggregate-use context aggregate_parameter",
    )
    aggregate_name = _context_identifier(
        aggregate.get("name"), "aggregate-use context aggregate_parameter.name"
    )
    aggregate_type = _context_identifier(
        aggregate.get("type"), "aggregate-use context aggregate_parameter.type"
    )
    raw_fields = aggregate.get("fields")
    if not isinstance(raw_fields, list) or not 2 <= len(raw_fields) <= 32:
        raise LearningInputError(
            "aggregate-use context aggregate_parameter.fields must contain two through thirty-two entries"
        )
    fields = [
        _context_identifier(
            item, f"aggregate-use context aggregate_parameter.fields[{index}]"
        )
        for index, item in enumerate(raw_fields)
    ]
    if len(set(fields)) != len(fields):
        raise LearningInputError(
            "aggregate-use context aggregate_parameter.fields must be unique"
        )
    aggregate_target = _context_text(
        aggregate.get("target_register"),
        "aggregate-use context aggregate_parameter.target_register",
        limit=3,
    ).lower()
    aggregate_candidate = _context_text(
        aggregate.get("candidate_register"),
        "aggregate-use context aggregate_parameter.candidate_register",
        limit=3,
    ).lower()
    matching_owner = next(
        (owner for owner in owners if owner["name"] == aggregate_name), None
    )
    if matching_owner is None or (
        matching_owner["target_register"] != aggregate_target
        or matching_owner["candidate_register"] != aggregate_candidate
    ):
        raise LearningInputError(
            "aggregate-use context aggregate parameter must match one sealed owner"
        )
    normalized_aggregate = {
        "name": aggregate_name,
        "type": aggregate_type,
        "fields": fields,
        "target_register": aggregate_target,
        "candidate_register": aggregate_candidate,
        "evidence_sha256": _context_sha256(
            aggregate.get("evidence_sha256"),
            "aggregate-use context aggregate_parameter.evidence_sha256",
        ),
    }

    raw_groups = context.get("copy_groups")
    if not isinstance(raw_groups, list) or not 1 <= len(raw_groups) <= 16:
        raise LearningInputError(
            "aggregate-use context copy_groups must contain one through sixteen entries"
        )
    group_fields = {
        "destination",
        "destination_type",
        "source",
        "fields",
        "consumer",
        "evidence_sha256",
    }
    groups: list[dict[str, Any]] = []
    for index, raw_group in enumerate(raw_groups):
        group = _closed_context(
            raw_group,
            allowed=group_fields,
            required=group_fields,
            label=f"aggregate-use context copy_groups[{index}]",
        )
        group_source = _context_identifier(
            group.get("source"),
            f"aggregate-use context copy_groups[{index}].source",
        )
        group_type = _context_identifier(
            group.get("destination_type"),
            f"aggregate-use context copy_groups[{index}].destination_type",
        )
        group_raw_fields = group.get("fields")
        if not isinstance(group_raw_fields, list):
            raise LearningInputError(
                f"aggregate-use context copy_groups[{index}].fields must be an array"
            )
        group_normalized_fields = [
            _context_identifier(
                item,
                f"aggregate-use context copy_groups[{index}].fields[{field_index}]",
            )
            for field_index, item in enumerate(group_raw_fields)
        ]
        if (
            group_source != aggregate_name
            or group_type != aggregate_type
            or group_normalized_fields != fields
        ):
            raise LearningInputError(
                "aggregate-use context copy groups must cover the complete sealed aggregate in field order"
            )
        groups.append(
            {
                "destination": _context_lvalue(
                    group.get("destination"),
                    f"aggregate-use context copy_groups[{index}].destination",
                ),
                "destination_type": group_type,
                "source": group_source,
                "fields": group_normalized_fields,
                "consumer": _context_text(
                    group.get("consumer"),
                    f"aggregate-use context copy_groups[{index}].consumer",
                    limit=256,
                ),
                "evidence_sha256": _context_sha256(
                    group.get("evidence_sha256"),
                    f"aggregate-use context copy_groups[{index}].evidence_sha256",
                ),
            }
        )
    if len({group["destination"] for group in groups}) != len(groups):
        raise LearningInputError(
            "aggregate-use context copy group destinations must be unique"
        )

    raw_consumers = context.get("independent_consumers")
    if not isinstance(raw_consumers, list) or len(raw_consumers) > 16:
        raise LearningInputError(
            "aggregate-use context independent_consumers must contain at most sixteen entries"
        )
    consumers: list[dict[str, Any]] = []
    for index, raw_consumer in enumerate(raw_consumers):
        consumer = _closed_context(
            raw_consumer,
            allowed={"expression", "fields", "evidence_sha256"},
            required={"expression", "fields", "evidence_sha256"},
            label=f"aggregate-use context independent_consumers[{index}]",
        )
        consumer_fields = consumer.get("fields")
        if not isinstance(consumer_fields, list) or not consumer_fields:
            raise LearningInputError(
                f"aggregate-use context independent_consumers[{index}].fields must be non-empty"
            )
        normalized_consumer_fields = [
            _context_identifier(
                item,
                f"aggregate-use context independent_consumers[{index}].fields[{field_index}]",
            )
            for field_index, item in enumerate(consumer_fields)
        ]
        if len(set(normalized_consumer_fields)) != len(
            normalized_consumer_fields
        ) or not set(normalized_consumer_fields).issubset(fields):
            raise LearningInputError(
                "aggregate-use context independent consumer fields must be a unique subset of aggregate fields"
            )
        consumers.append(
            {
                "expression": _context_text(
                    consumer.get("expression"),
                    f"aggregate-use context independent_consumers[{index}].expression",
                    limit=512,
                ),
                "fields": normalized_consumer_fields,
                "evidence_sha256": _context_sha256(
                    consumer.get("evidence_sha256"),
                    f"aggregate-use context independent_consumers[{index}].evidence_sha256",
                ),
            }
        )

    raw_axes = context.get("rejected_axes")
    if not isinstance(raw_axes, list) or len(raw_axes) > 8:
        raise LearningInputError(
            "aggregate-use context rejected_axes must contain at most eight entries"
        )
    axes: list[dict[str, Any]] = []
    for index, raw_axis in enumerate(raw_axes):
        axis = _closed_context(
            raw_axis,
            allowed={"axis", "candidate_record_sha256", "regressed"},
            required={"axis", "candidate_record_sha256", "regressed"},
            label=f"aggregate-use context rejected_axes[{index}]",
        )
        if axis.get("regressed") is not True:
            raise LearningInputError(
                f"aggregate-use context rejected_axes[{index}].regressed must be true"
            )
        axes.append(
            {
                "axis": _context_identifier(
                    axis.get("axis"),
                    f"aggregate-use context rejected_axes[{index}].axis",
                ),
                "candidate_record_sha256": _context_sha256(
                    axis.get("candidate_record_sha256"),
                    f"aggregate-use context rejected_axes[{index}].candidate_record_sha256",
                ),
                "regressed": True,
            }
        )
    if len({axis["axis"] for axis in axes}) != len(axes):
        raise LearningInputError(
            "aggregate-use context rejected axis names must be unique"
        )

    return {
        "schema": AGGREGATE_USE_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "owners": sorted(owners, key=lambda item: item["name"]),
        "aggregate_parameter": normalized_aggregate,
        "copy_groups": groups,
        "independent_consumers": consumers,
        "rejected_axes": axes,
    }


def _parse_aggregate_followup_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "owners",
            "aggregate_boundary",
            "declaration_axis",
            "fusion_observation",
        },
        required={
            "schema",
            "proofs",
            "owners",
            "aggregate_boundary",
            "declaration_axis",
            "fusion_observation",
        },
        label="aggregate follow-up context",
    )
    if (
        _context_text(context.get("schema"), "aggregate follow-up context schema")
        != AGGREGATE_FOLLOWUP_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"aggregate follow-up context schema must be {AGGREGATE_FOLLOWUP_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_AGGREGATE_FOLLOWUP_PROOF_FLAGS) | set(
        _AGGREGATE_FOLLOWUP_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="aggregate follow-up context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _AGGREGATE_FOLLOWUP_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"aggregate follow-up context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _AGGREGATE_FOLLOWUP_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"aggregate follow-up context proofs.{field}"
        )

    raw_owners = context.get("owners")
    if not isinstance(raw_owners, list) or len(raw_owners) != 2:
        raise LearningInputError(
            "aggregate follow-up context owners must contain exactly two entries"
        )
    owners: list[dict[str, Any]] = []
    owner_fields = {
        "name",
        "type",
        "target_register",
        "candidate_register",
        "evidence_sha256",
    }
    for index, raw_owner in enumerate(raw_owners):
        owner = _closed_context(
            raw_owner,
            allowed=owner_fields,
            required=owner_fields,
            label=f"aggregate follow-up context owners[{index}]",
        )
        target_register = _context_text(
            owner.get("target_register"),
            f"aggregate follow-up context owners[{index}].target_register",
            limit=3,
        ).lower()
        candidate_register = _context_text(
            owner.get("candidate_register"),
            f"aggregate follow-up context owners[{index}].candidate_register",
            limit=3,
        ).lower()
        if not _saved(target_register, "r") or not _saved(candidate_register, "r"):
            raise LearningInputError(
                "aggregate follow-up context owner registers must be nonvolatile GPRs"
            )
        owners.append(
            {
                "name": _context_identifier(
                    owner.get("name"),
                    f"aggregate follow-up context owners[{index}].name",
                ),
                "type": _context_identifier(
                    owner.get("type"),
                    f"aggregate follow-up context owners[{index}].type",
                ),
                "target_register": target_register,
                "candidate_register": candidate_register,
                "evidence_sha256": _context_sha256(
                    owner.get("evidence_sha256"),
                    f"aggregate follow-up context owners[{index}].evidence_sha256",
                ),
            }
        )
    for field in ("name", "target_register", "candidate_register"):
        if len({owner[field] for owner in owners}) != 2:
            raise LearningInputError(
                f"aggregate follow-up context owner {field} values must be unique"
            )
    owner_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in owners
    }
    cycles = _closed_cycles(owner_mapping)
    if len(cycles) != 1 or len(cycles[0]) != 2:
        raise LearningInputError(
            "aggregate follow-up context owners must describe one complete two-register swap"
        )

    aggregate_boundary = _closed_context(
        context.get("aggregate_boundary"),
        allowed={"expression", "already_applied", "evidence_sha256"},
        required={"expression", "already_applied", "evidence_sha256"},
        label="aggregate follow-up context aggregate_boundary",
    )
    if aggregate_boundary.get("already_applied") is not True:
        raise LearningInputError(
            "aggregate follow-up context aggregate_boundary.already_applied must be true"
        )
    normalized_boundary = {
        "expression": _context_text(
            aggregate_boundary.get("expression"),
            "aggregate follow-up context aggregate_boundary.expression",
            limit=512,
        ),
        "already_applied": True,
        "evidence_sha256": _context_sha256(
            aggregate_boundary.get("evidence_sha256"),
            "aggregate follow-up context aggregate_boundary.evidence_sha256",
        ),
    }

    declaration_axis = _closed_context(
        context.get("declaration_axis"),
        allowed={"recommended_order", "evidence_sha256"},
        required={"recommended_order", "evidence_sha256"},
        label="aggregate follow-up context declaration_axis",
    )
    raw_order = declaration_axis.get("recommended_order")
    if not isinstance(raw_order, list) or len(raw_order) != 2:
        raise LearningInputError(
            "aggregate follow-up context declaration_axis.recommended_order must contain two entries"
        )
    recommended_order = [
        _context_identifier(
            item,
            f"aggregate follow-up context declaration_axis.recommended_order[{index}]",
        )
        for index, item in enumerate(raw_order)
    ]
    if set(recommended_order) != {str(owner["name"]) for owner in owners}:
        raise LearningInputError(
            "aggregate follow-up declaration order must contain exactly the two sealed owners"
        )
    normalized_declaration = {
        "recommended_order": recommended_order,
        "evidence_sha256": _context_sha256(
            declaration_axis.get("evidence_sha256"),
            "aggregate follow-up context declaration_axis.evidence_sha256",
        ),
    }

    fusion = _closed_context(
        context.get("fusion_observation"),
        allowed={
            "source_shape",
            "target_size",
            "candidate_size",
            "strict_regressed",
            "topology_changed",
            "candidate_record_sha256",
        },
        required={
            "source_shape",
            "target_size",
            "candidate_size",
            "strict_regressed",
            "topology_changed",
            "candidate_record_sha256",
        },
        label="aggregate follow-up context fusion_observation",
    )
    if fusion.get("strict_regressed") is not True or fusion.get("topology_changed") is not True:
        raise LearningInputError(
            "aggregate follow-up fusion observation must prove both regression and topology change"
        )
    fusion_target_size = _context_uint(
        fusion.get("target_size"),
        "aggregate follow-up context fusion_observation.target_size",
    )
    fusion_candidate_size = _context_uint(
        fusion.get("candidate_size"),
        "aggregate follow-up context fusion_observation.candidate_size",
    )
    if fusion_target_size == fusion_candidate_size:
        raise LearningInputError(
            "aggregate follow-up fusion observation must have a measured size change"
        )
    normalized_fusion = {
        "source_shape": _context_text(
            fusion.get("source_shape"),
            "aggregate follow-up context fusion_observation.source_shape",
            limit=512,
        ),
        "target_size": fusion_target_size,
        "candidate_size": fusion_candidate_size,
        "strict_regressed": True,
        "topology_changed": True,
        "candidate_record_sha256": _context_sha256(
            fusion.get("candidate_record_sha256"),
            "aggregate follow-up context fusion_observation.candidate_record_sha256",
        ),
    }

    return {
        "schema": AGGREGATE_FOLLOWUP_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "owners": sorted(owners, key=lambda item: item["name"]),
        "aggregate_boundary": normalized_boundary,
        "declaration_axis": normalized_declaration,
        "fusion_observation": normalized_fusion,
    }


def _parse_address_taken_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "expected_size_delta",
            "aggregate",
            "incoming_pointer",
            "local_pointer",
            "object_home",
        },
        required={
            "schema",
            "proofs",
            "expected_size_delta",
            "aggregate",
            "incoming_pointer",
            "local_pointer",
            "object_home",
        },
        label="address-taken local pointer context",
    )
    if (
        _context_text(context.get("schema"), "address-taken context schema")
        != ADDRESS_TAKEN_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"address-taken context schema must be {ADDRESS_TAKEN_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_ADDRESS_TAKEN_PROOF_FLAGS) | set(
        _ADDRESS_TAKEN_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="address-taken context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _ADDRESS_TAKEN_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(f"address-taken context proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _ADDRESS_TAKEN_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"address-taken context proofs.{field}"
        )

    expected_size_delta = _context_uint(
        context.get("expected_size_delta"),
        "address-taken context expected_size_delta",
    )
    if expected_size_delta == 0 or expected_size_delta > 64 or expected_size_delta % 4:
        raise LearningInputError(
            "address-taken context expected_size_delta must be 4..64 and instruction-aligned"
        )

    aggregate = _closed_context(
        context.get("aggregate"),
        allowed={"name", "type", "stack_offset", "evidence_sha256"},
        required={"name", "type", "stack_offset", "evidence_sha256"},
        label="address-taken context aggregate",
    )
    aggregate_offset = _context_uint(
        aggregate.get("stack_offset"), "address-taken context aggregate.stack_offset"
    )
    normalized_aggregate = {
        "name": _context_identifier(
            aggregate.get("name"), "address-taken context aggregate.name"
        ),
        "type": _context_identifier(
            aggregate.get("type"), "address-taken context aggregate.type"
        ),
        "stack_offset": aggregate_offset,
        "evidence_sha256": _context_sha256(
            aggregate.get("evidence_sha256"),
            "address-taken context aggregate.evidence_sha256",
        ),
    }

    incoming = _closed_context(
        context.get("incoming_pointer"),
        allowed={
            "name",
            "target_register",
            "candidate_register",
            "evidence_sha256",
        },
        required={
            "name",
            "target_register",
            "candidate_register",
            "evidence_sha256",
        },
        label="address-taken context incoming_pointer",
    )
    incoming_target = _context_text(
        incoming.get("target_register"),
        "address-taken context incoming_pointer.target_register",
        limit=3,
    ).lower()
    incoming_candidate = _context_text(
        incoming.get("candidate_register"),
        "address-taken context incoming_pointer.candidate_register",
        limit=3,
    ).lower()
    if not _saved(incoming_target, "r") or not _saved(incoming_candidate, "r"):
        raise LearningInputError(
            "address-taken incoming pointer registers must be nonvolatile GPRs"
        )
    normalized_incoming = {
        "name": _context_identifier(
            incoming.get("name"), "address-taken context incoming_pointer.name"
        ),
        "target_register": incoming_target,
        "candidate_register": incoming_candidate,
        "evidence_sha256": _context_sha256(
            incoming.get("evidence_sha256"),
            "address-taken context incoming_pointer.evidence_sha256",
        ),
    }

    local = _closed_context(
        context.get("local_pointer"),
        allowed={
            "name",
            "target_register",
            "argument_register",
            "consumer",
            "evidence_sha256",
        },
        required={
            "name",
            "target_register",
            "argument_register",
            "consumer",
            "evidence_sha256",
        },
        label="address-taken context local_pointer",
    )
    local_target = _context_text(
        local.get("target_register"),
        "address-taken context local_pointer.target_register",
        limit=3,
    ).lower()
    argument_register = _context_text(
        local.get("argument_register"),
        "address-taken context local_pointer.argument_register",
        limit=3,
    ).lower()
    if not _saved(local_target, "r") or argument_register not in {
        f"r{number}" for number in range(3, 11)
    }:
        raise LearningInputError(
            "address-taken local pointer must use a nonvolatile owner and a GPR argument register"
        )
    if local_target != incoming_candidate or local_target == incoming_target:
        raise LearningInputError(
            "address-taken local pointer must take the candidate incoming-pointer color while target incoming ownership moves"
        )
    normalized_local = {
        "name": _context_identifier(
            local.get("name"), "address-taken context local_pointer.name"
        ),
        "target_register": local_target,
        "argument_register": argument_register,
        "consumer": _context_identifier(
            local.get("consumer"), "address-taken context local_pointer.consumer"
        ),
        "evidence_sha256": _context_sha256(
            local.get("evidence_sha256"),
            "address-taken context local_pointer.evidence_sha256",
        ),
    }

    object_home = _closed_context(
        context.get("object_home"),
        allowed={"parameter", "target_stack_offset", "evidence_sha256"},
        required={"parameter", "target_stack_offset", "evidence_sha256"},
        label="address-taken context object_home",
    )
    normalized_home = {
        "parameter": _context_identifier(
            object_home.get("parameter"),
            "address-taken context object_home.parameter",
        ),
        "target_stack_offset": _context_uint(
            object_home.get("target_stack_offset"),
            "address-taken context object_home.target_stack_offset",
        ),
        "evidence_sha256": _context_sha256(
            object_home.get("evidence_sha256"),
            "address-taken context object_home.evidence_sha256",
        ),
    }

    return {
        "schema": ADDRESS_TAKEN_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "expected_size_delta": expected_size_delta,
        "aggregate": normalized_aggregate,
        "incoming_pointer": normalized_incoming,
        "local_pointer": normalized_local,
        "object_home": normalized_home,
    }


def _parse_same_tu_shape_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "donor",
            "fixed_array_tail",
            "abi_boundary",
            "zero_chain",
            "combined_cell",
        },
        required={
            "schema",
            "proofs",
            "donor",
            "fixed_array_tail",
            "abi_boundary",
            "zero_chain",
            "combined_cell",
        },
        label="same-TU source-shape context",
    )
    if (
        _context_text(context.get("schema"), "same-TU source-shape context schema")
        != SAME_TU_SHAPE_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"same-TU source-shape context schema must be {SAME_TU_SHAPE_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_SAME_TU_SHAPE_PROOF_FLAGS) | set(
        _SAME_TU_SHAPE_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="same-TU source-shape context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _SAME_TU_SHAPE_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"same-TU source-shape context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _SAME_TU_SHAPE_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"same-TU source-shape context proofs.{field}"
        )

    donor = _closed_context(
        context.get("donor"),
        allowed={
            "symbol",
            "source_location",
            "source_expression",
            "array_bound",
            "evidence_sha256",
        },
        required={
            "symbol",
            "source_location",
            "source_expression",
            "array_bound",
            "evidence_sha256",
        },
        label="same-TU source-shape context donor",
    )
    normalized_donor = {
        "symbol": _context_identifier(
            donor.get("symbol"), "same-TU source-shape context donor.symbol"
        ),
        "source_location": _context_text(
            donor.get("source_location"),
            "same-TU source-shape context donor.source_location",
            limit=512,
        ),
        "source_expression": _context_text(
            donor.get("source_expression"),
            "same-TU source-shape context donor.source_expression",
            limit=512,
        ),
        "array_bound": _context_uint(
            donor.get("array_bound"),
            "same-TU source-shape context donor.array_bound",
            minimum=1,
            maximum=65535,
        ),
        "evidence_sha256": _context_sha256(
            donor.get("evidence_sha256"),
            "same-TU source-shape context donor.evidence_sha256",
        ),
    }

    tail = _closed_context(
        context.get("fixed_array_tail"),
        allowed={"target_rows", "array_bound", "source_expression", "evidence_sha256"},
        required={"target_rows", "array_bound", "source_expression", "evidence_sha256"},
        label="same-TU source-shape context fixed_array_tail",
    )
    normalized_tail = {
        "target_rows": _context_rows(
            tail.get("target_rows"),
            "same-TU source-shape context fixed_array_tail.target_rows",
            minimum_count=5,
            maximum_count=5,
        ),
        "array_bound": _context_uint(
            tail.get("array_bound"),
            "same-TU source-shape context fixed_array_tail.array_bound",
            minimum=1,
            maximum=65535,
        ),
        "source_expression": _context_text(
            tail.get("source_expression"),
            "same-TU source-shape context fixed_array_tail.source_expression",
            limit=512,
        ),
        "evidence_sha256": _context_sha256(
            tail.get("evidence_sha256"),
            "same-TU source-shape context fixed_array_tail.evidence_sha256",
        ),
    }
    if normalized_tail["array_bound"] != normalized_donor["array_bound"]:
        raise LearningInputError(
            "same-TU donor and fixed-array tail must authenticate the same bound"
        )

    abi = _closed_context(
        context.get("abi_boundary"),
        allowed={
            "parameter",
            "parameter_register",
            "producer_type",
            "callee_type",
            "candidate_normalization_row",
            "store_row",
            "caller_symbol",
            "source_location",
            "evidence_sha256",
        },
        required={
            "parameter",
            "parameter_register",
            "producer_type",
            "callee_type",
            "candidate_normalization_row",
            "store_row",
            "caller_symbol",
            "source_location",
            "evidence_sha256",
        },
        label="same-TU source-shape context abi_boundary",
    )
    parameter_register = _context_text(
        abi.get("parameter_register"),
        "same-TU source-shape context abi_boundary.parameter_register",
        limit=3,
    ).lower()
    if parameter_register not in {f"r{number}" for number in range(3, 11)}:
        raise LearningInputError(
            "same-TU source-shape ABI parameter must use a GPR argument register"
        )
    normalized_abi = {
        "parameter": _context_identifier(
            abi.get("parameter"), "same-TU source-shape context abi_boundary.parameter"
        ),
        "parameter_register": parameter_register,
        "producer_type": _context_identifier(
            abi.get("producer_type"),
            "same-TU source-shape context abi_boundary.producer_type",
        ),
        "callee_type": _context_identifier(
            abi.get("callee_type"),
            "same-TU source-shape context abi_boundary.callee_type",
        ),
        "candidate_normalization_row": _context_uint(
            abi.get("candidate_normalization_row"),
            "same-TU source-shape context abi_boundary.candidate_normalization_row",
        ),
        "store_row": _context_uint(
            abi.get("store_row"),
            "same-TU source-shape context abi_boundary.store_row",
        ),
        "caller_symbol": _context_identifier(
            abi.get("caller_symbol"),
            "same-TU source-shape context abi_boundary.caller_symbol",
        ),
        "source_location": _context_text(
            abi.get("source_location"),
            "same-TU source-shape context abi_boundary.source_location",
            limit=512,
        ),
        "evidence_sha256": _context_sha256(
            abi.get("evidence_sha256"),
            "same-TU source-shape context abi_boundary.evidence_sha256",
        ),
    }
    if normalized_abi["producer_type"] == normalized_abi["callee_type"]:
        raise LearningInputError(
            "same-TU source-shape ABI producer and callee types must differ"
        )

    zero = _closed_context(
        context.get("zero_chain"),
        allowed={
            "destination",
            "fields",
            "target_load_row",
            "target_store_rows",
            "candidate_load_rows",
            "candidate_store_rows",
            "source_expression",
            "evidence_sha256",
        },
        required={
            "destination",
            "fields",
            "target_load_row",
            "target_store_rows",
            "candidate_load_rows",
            "candidate_store_rows",
            "source_expression",
            "evidence_sha256",
        },
        label="same-TU source-shape context zero_chain",
    )
    fields = zero.get("fields")
    if not isinstance(fields, list) or len(fields) != 3:
        raise LearningInputError(
            "same-TU source-shape zero chain fields must contain exactly three names"
        )
    normalized_fields = [
        _context_identifier(
            item, f"same-TU source-shape context zero_chain.fields[{index}]"
        )
        for index, item in enumerate(fields)
    ]
    if len(set(normalized_fields)) != 3:
        raise LearningInputError(
            "same-TU source-shape zero chain fields must be distinct"
        )
    normalized_zero = {
        "destination": _context_lvalue(
            zero.get("destination"),
            "same-TU source-shape context zero_chain.destination",
        ),
        "fields": normalized_fields,
        "target_load_row": _context_uint(
            zero.get("target_load_row"),
            "same-TU source-shape context zero_chain.target_load_row",
        ),
        "target_store_rows": _context_rows(
            zero.get("target_store_rows"),
            "same-TU source-shape context zero_chain.target_store_rows",
            minimum_count=3,
            maximum_count=3,
        ),
        "candidate_load_rows": _context_rows(
            zero.get("candidate_load_rows"),
            "same-TU source-shape context zero_chain.candidate_load_rows",
            minimum_count=3,
            maximum_count=3,
        ),
        "candidate_store_rows": _context_rows(
            zero.get("candidate_store_rows"),
            "same-TU source-shape context zero_chain.candidate_store_rows",
            minimum_count=3,
            maximum_count=3,
        ),
        "source_expression": _context_text(
            zero.get("source_expression"),
            "same-TU source-shape context zero_chain.source_expression",
            limit=512,
        ),
        "evidence_sha256": _context_sha256(
            zero.get("evidence_sha256"),
            "same-TU source-shape context zero_chain.evidence_sha256",
        ),
    }

    cell = _closed_context(
        context.get("combined_cell"),
        allowed={
            "candidate_id",
            "target_size",
            "candidate_size",
            "object_sha256",
            "candidate_record_sha256",
        },
        required={
            "candidate_id",
            "target_size",
            "candidate_size",
            "object_sha256",
            "candidate_record_sha256",
        },
        label="same-TU source-shape context combined_cell",
    )
    normalized_cell = {
        "candidate_id": _context_text(
            cell.get("candidate_id"),
            "same-TU source-shape context combined_cell.candidate_id",
            limit=128,
        ),
        "target_size": _context_uint(
            cell.get("target_size"),
            "same-TU source-shape context combined_cell.target_size",
            minimum=4,
        ),
        "candidate_size": _context_uint(
            cell.get("candidate_size"),
            "same-TU source-shape context combined_cell.candidate_size",
            minimum=4,
        ),
        "object_sha256": _context_sha256(
            cell.get("object_sha256"),
            "same-TU source-shape context combined_cell.object_sha256",
        ),
        "candidate_record_sha256": _context_sha256(
            cell.get("candidate_record_sha256"),
            "same-TU source-shape context combined_cell.candidate_record_sha256",
        ),
    }
    if normalized_cell["target_size"] != normalized_cell["candidate_size"]:
        raise LearningInputError(
            "same-TU source-shape combined cell must be function-size exact"
        )

    tail_rows = set(normalized_tail["target_rows"])
    abi_rows = {
        normalized_abi["candidate_normalization_row"],
        normalized_abi["store_row"],
    }
    zero_rows = {
        normalized_zero["target_load_row"],
        *normalized_zero["target_store_rows"],
        *normalized_zero["candidate_load_rows"],
        *normalized_zero["candidate_store_rows"],
    }
    if tail_rows & abi_rows or tail_rows & zero_rows or abi_rows & zero_rows:
        raise LearningInputError(
            "same-TU source-shape residual row groups must be disjoint"
        )
    return {
        "schema": SAME_TU_SHAPE_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "donor": normalized_donor,
        "fixed_array_tail": normalized_tail,
        "abi_boundary": normalized_abi,
        "zero_chain": normalized_zero,
        "combined_cell": normalized_cell,
    }


def _parse_short_circuit_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "mask_tests",
            "shared_boolean",
            "direct_assignment_rejection",
            "topology_observation",
        },
        required={
            "schema",
            "proofs",
            "mask_tests",
            "shared_boolean",
            "direct_assignment_rejection",
            "topology_observation",
        },
        label="short-circuit Boolean context",
    )
    if (
        _context_text(context.get("schema"), "short-circuit Boolean context schema")
        != SHORT_CIRCUIT_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"short-circuit Boolean context schema must be {SHORT_CIRCUIT_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_SHORT_CIRCUIT_PROOF_FLAGS) | set(
        _SHORT_CIRCUIT_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="short-circuit Boolean context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _SHORT_CIRCUIT_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"short-circuit Boolean context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _SHORT_CIRCUIT_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"short-circuit Boolean context proofs.{field}"
        )

    raw_tests = context.get("mask_tests")
    if not isinstance(raw_tests, list) or len(raw_tests) != 2:
        raise LearningInputError(
            "short-circuit Boolean context mask_tests must contain exactly two entries"
        )
    tests: list[dict[str, Any]] = []
    test_fields = {
        "source_left",
        "source_right",
        "source_expression",
        "branch_getter",
        "masu_getter",
        "target_branch_call_row",
        "target_masu_call_row",
        "evidence_sha256",
    }
    for index, raw_test in enumerate(raw_tests):
        test = _closed_context(
            raw_test,
            allowed=test_fields,
            required=test_fields,
            label=f"short-circuit Boolean context mask_tests[{index}]",
        )
        branch_row = _context_uint(
            test.get("target_branch_call_row"),
            f"short-circuit Boolean context mask_tests[{index}].target_branch_call_row",
        )
        masu_row = _context_uint(
            test.get("target_masu_call_row"),
            f"short-circuit Boolean context mask_tests[{index}].target_masu_call_row",
        )
        if branch_row >= masu_row:
            raise LearningInputError(
                "short-circuit target call order must place branch getter before masu getter"
            )
        tests.append(
            {
                "source_left": _context_text(
                    test.get("source_left"),
                    f"short-circuit Boolean context mask_tests[{index}].source_left",
                    limit=512,
                ),
                "source_right": _context_text(
                    test.get("source_right"),
                    f"short-circuit Boolean context mask_tests[{index}].source_right",
                    limit=512,
                ),
                "source_expression": _context_text(
                    test.get("source_expression"),
                    f"short-circuit Boolean context mask_tests[{index}].source_expression",
                    limit=1024,
                ),
                "branch_getter": _context_identifier(
                    test.get("branch_getter"),
                    f"short-circuit Boolean context mask_tests[{index}].branch_getter",
                ),
                "masu_getter": _context_identifier(
                    test.get("masu_getter"),
                    f"short-circuit Boolean context mask_tests[{index}].masu_getter",
                ),
                "target_branch_call_row": branch_row,
                "target_masu_call_row": masu_row,
                "evidence_sha256": _context_sha256(
                    test.get("evidence_sha256"),
                    f"short-circuit Boolean context mask_tests[{index}].evidence_sha256",
                ),
            }
        )
    if len({item["branch_getter"] for item in tests}) != 2 or len(
        {item["masu_getter"] for item in tests}
    ) != 2:
        raise LearningInputError(
            "short-circuit mask tests must name two distinct getter pairs"
        )

    shared = _closed_context(
        context.get("shared_boolean"),
        allowed={
            "target_branch_rows",
            "target_true_assignment_row",
            "target_false_assignment_row",
            "candidate_true_assignment_rows",
            "candidate_false_assignment_row",
            "result_register",
            "evidence_sha256",
        },
        required={
            "target_branch_rows",
            "target_true_assignment_row",
            "target_false_assignment_row",
            "candidate_true_assignment_rows",
            "candidate_false_assignment_row",
            "result_register",
            "evidence_sha256",
        },
        label="short-circuit Boolean context shared_boolean",
    )
    result_register = _context_text(
        shared.get("result_register"),
        "short-circuit Boolean context shared_boolean.result_register",
        limit=3,
    ).lower()
    if not _saved(result_register, "r"):
        raise LearningInputError(
            "short-circuit Boolean result must use a nonvolatile GPR"
        )
    normalized_shared = {
        "target_branch_rows": _context_rows(
            shared.get("target_branch_rows"),
            "short-circuit Boolean context shared_boolean.target_branch_rows",
            minimum_count=2,
            maximum_count=2,
        ),
        "target_true_assignment_row": _context_uint(
            shared.get("target_true_assignment_row"),
            "short-circuit Boolean context shared_boolean.target_true_assignment_row",
        ),
        "target_false_assignment_row": _context_uint(
            shared.get("target_false_assignment_row"),
            "short-circuit Boolean context shared_boolean.target_false_assignment_row",
        ),
        "candidate_true_assignment_rows": _context_rows(
            shared.get("candidate_true_assignment_rows"),
            "short-circuit Boolean context shared_boolean.candidate_true_assignment_rows",
            minimum_count=2,
            maximum_count=2,
        ),
        "candidate_false_assignment_row": _context_uint(
            shared.get("candidate_false_assignment_row"),
            "short-circuit Boolean context shared_boolean.candidate_false_assignment_row",
        ),
        "result_register": result_register,
        "evidence_sha256": _context_sha256(
            shared.get("evidence_sha256"),
            "short-circuit Boolean context shared_boolean.evidence_sha256",
        ),
    }

    rejection = _closed_context(
        context.get("direct_assignment_rejection"),
        allowed={
            "candidate_record_sha256",
            "reversed_call_order",
            "strict_regressed",
            "evidence_sha256",
        },
        required={
            "candidate_record_sha256",
            "reversed_call_order",
            "strict_regressed",
            "evidence_sha256",
        },
        label="short-circuit Boolean context direct_assignment_rejection",
    )
    if rejection.get("reversed_call_order") is not True or rejection.get(
        "strict_regressed"
    ) is not True:
        raise LearningInputError(
            "short-circuit direct-assignment rejection must prove reversed call order and strict regression"
        )
    normalized_rejection = {
        "candidate_record_sha256": _context_sha256(
            rejection.get("candidate_record_sha256"),
            "short-circuit Boolean context direct_assignment_rejection.candidate_record_sha256",
        ),
        "reversed_call_order": True,
        "strict_regressed": True,
        "evidence_sha256": _context_sha256(
            rejection.get("evidence_sha256"),
            "short-circuit Boolean context direct_assignment_rejection.evidence_sha256",
        ),
    }

    observation = _closed_context(
        context.get("topology_observation"),
        allowed={
            "candidate_id",
            "target_size",
            "candidate_size",
            "residual_kind",
            "owners",
            "recommended_declaration_order",
            "candidate_record_sha256",
            "evidence_sha256",
        },
        required={
            "candidate_id",
            "target_size",
            "candidate_size",
            "residual_kind",
            "owners",
            "recommended_declaration_order",
            "candidate_record_sha256",
            "evidence_sha256",
        },
        label="short-circuit Boolean context topology_observation",
    )
    raw_owners = observation.get("owners")
    if not isinstance(raw_owners, list) or len(raw_owners) != 4:
        raise LearningInputError(
            "short-circuit topology observation owners must contain exactly four entries"
        )
    owners: list[dict[str, Any]] = []
    owner_fields = {
        "name",
        "type",
        "target_register",
        "candidate_register",
        "evidence_sha256",
    }
    for index, raw_owner in enumerate(raw_owners):
        owner = _closed_context(
            raw_owner,
            allowed=owner_fields,
            required=owner_fields,
            label=f"short-circuit Boolean context topology_observation.owners[{index}]",
        )
        target_register = _context_text(
            owner.get("target_register"),
            f"short-circuit Boolean context topology_observation.owners[{index}].target_register",
            limit=3,
        ).lower()
        candidate_register = _context_text(
            owner.get("candidate_register"),
            f"short-circuit Boolean context topology_observation.owners[{index}].candidate_register",
            limit=3,
        ).lower()
        if (
            not _saved(target_register, "r")
            or not _saved(candidate_register, "r")
            or target_register == candidate_register
        ):
            raise LearningInputError(
                "short-circuit topology owners must bind distinct nonvolatile GPR colors"
            )
        owners.append(
            {
                "name": _context_identifier(
                    owner.get("name"),
                    f"short-circuit Boolean context topology_observation.owners[{index}].name",
                ),
                "type": _context_identifier(
                    owner.get("type"),
                    f"short-circuit Boolean context topology_observation.owners[{index}].type",
                ),
                "target_register": target_register,
                "candidate_register": candidate_register,
                "evidence_sha256": _context_sha256(
                    owner.get("evidence_sha256"),
                    f"short-circuit Boolean context topology_observation.owners[{index}].evidence_sha256",
                ),
            }
        )
    if (
        len({item["name"] for item in owners}) != 4
        or len({item["target_register"] for item in owners}) != 4
        or len({item["candidate_register"] for item in owners}) != 4
        or {item["target_register"] for item in owners}
        != {item["candidate_register"] for item in owners}
    ):
        raise LearningInputError(
            "short-circuit topology owners must form one closed four-owner register set"
        )
    raw_order = observation.get("recommended_declaration_order")
    if not isinstance(raw_order, list) or len(raw_order) != 4:
        raise LearningInputError(
            "short-circuit recommended declaration order must contain four owners"
        )
    declaration_order = [
        _context_identifier(
            item,
            f"short-circuit Boolean context topology_observation.recommended_declaration_order[{index}]",
        )
        for index, item in enumerate(raw_order)
    ]
    if set(declaration_order) != {item["name"] for item in owners}:
        raise LearningInputError(
            "short-circuit declaration order must name exactly the sealed owners"
        )
    residual_kind = _context_text(
        observation.get("residual_kind"),
        "short-circuit Boolean context topology_observation.residual_kind",
        limit=32,
    )
    if residual_kind != "ARG_ONLY":
        raise LearningInputError(
            "short-circuit topology observation residual_kind must be ARG_ONLY"
        )
    normalized_observation = {
        "candidate_id": _context_text(
            observation.get("candidate_id"),
            "short-circuit Boolean context topology_observation.candidate_id",
            limit=128,
        ),
        "target_size": _context_uint(
            observation.get("target_size"),
            "short-circuit Boolean context topology_observation.target_size",
            minimum=4,
        ),
        "candidate_size": _context_uint(
            observation.get("candidate_size"),
            "short-circuit Boolean context topology_observation.candidate_size",
            minimum=4,
        ),
        "residual_kind": residual_kind,
        "owners": sorted(owners, key=lambda item: item["name"]),
        "recommended_declaration_order": declaration_order,
        "candidate_record_sha256": _context_sha256(
            observation.get("candidate_record_sha256"),
            "short-circuit Boolean context topology_observation.candidate_record_sha256",
        ),
        "evidence_sha256": _context_sha256(
            observation.get("evidence_sha256"),
            "short-circuit Boolean context topology_observation.evidence_sha256",
        ),
    }
    if normalized_observation["target_size"] != normalized_observation["candidate_size"]:
        raise LearningInputError(
            "short-circuit topology observation must be function-size exact"
        )

    return {
        "schema": SHORT_CIRCUIT_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "mask_tests": tests,
        "shared_boolean": normalized_shared,
        "direct_assignment_rejection": normalized_rejection,
        "topology_observation": normalized_observation,
    }


def _parse_exact_sibling_transfer_context(
    value: Mapping[str, Any],
) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "donor",
            "baseline",
            "type_boundary",
            "capacity",
            "combined_cell",
        },
        required={
            "schema",
            "proofs",
            "donor",
            "baseline",
            "type_boundary",
            "capacity",
            "combined_cell",
        },
        label="exact-sibling transfer context",
    )
    if (
        _context_text(context.get("schema"), "exact-sibling transfer context schema")
        != EXACT_SIBLING_TRANSFER_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            "exact-sibling transfer context schema must be "
            f"{EXACT_SIBLING_TRANSFER_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_EXACT_SIBLING_TRANSFER_PROOF_FLAGS) | set(
        _EXACT_SIBLING_TRANSFER_PROOF_HASHES
    )
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="exact-sibling transfer context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _EXACT_SIBLING_TRANSFER_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(
                f"exact-sibling transfer context proofs.{field} must be true"
            )
        normalized_proofs[field] = True
    for field in _EXACT_SIBLING_TRANSFER_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"exact-sibling transfer context proofs.{field}"
        )

    donor = _closed_context(
        context.get("donor"),
        allowed={
            "symbol",
            "source_location",
            "transformation_class",
            "source_expressions",
            "candidate_record_sha256",
            "evidence_sha256",
        },
        required={
            "symbol",
            "source_location",
            "transformation_class",
            "source_expressions",
            "candidate_record_sha256",
            "evidence_sha256",
        },
        label="exact-sibling transfer context donor",
    )
    source_expressions = donor.get("source_expressions")
    if not isinstance(source_expressions, list) or len(source_expressions) != 2:
        raise LearningInputError(
            "exact-sibling transfer donor source_expressions must contain exactly two entries"
        )
    normalized_donor = {
        "symbol": _context_identifier(
            donor.get("symbol"), "exact-sibling transfer donor symbol"
        ),
        "source_location": _context_text(
            donor.get("source_location"),
            "exact-sibling transfer donor source_location",
            limit=512,
        ),
        "transformation_class": _context_identifier(
            donor.get("transformation_class"),
            "exact-sibling transfer donor transformation_class",
        ),
        "source_expressions": [
            _context_text(
                expression,
                f"exact-sibling transfer donor source_expressions[{index}]",
                limit=512,
            )
            for index, expression in enumerate(source_expressions)
        ],
        "candidate_record_sha256": _context_sha256(
            donor.get("candidate_record_sha256"),
            "exact-sibling transfer donor candidate_record_sha256",
        ),
        "evidence_sha256": _context_sha256(
            donor.get("evidence_sha256"),
            "exact-sibling transfer donor evidence_sha256",
        ),
    }
    if normalized_donor["transformation_class"] != "shared_boolean_call_order":
        raise LearningInputError(
            "exact-sibling transfer donor transformation_class must be shared_boolean_call_order"
        )

    baseline = _closed_context(
        context.get("baseline"),
        allowed={"mask_tests", "shared_boolean"},
        required={"mask_tests", "shared_boolean"},
        label="exact-sibling transfer context baseline",
    )
    raw_tests = baseline.get("mask_tests")
    if not isinstance(raw_tests, list) or len(raw_tests) != 2:
        raise LearningInputError(
            "exact-sibling transfer baseline mask_tests must contain exactly two entries"
        )
    normalized_tests: list[dict[str, Any]] = []
    test_fields = {
        "source_left",
        "source_right",
        "source_expression",
        "branch_getter",
        "masu_getter",
        "target_branch_call_row",
        "target_masu_call_row",
        "candidate_masu_call_row",
        "candidate_branch_call_row",
        "evidence_sha256",
    }
    for index, raw_test in enumerate(raw_tests):
        test = _closed_context(
            raw_test,
            allowed=test_fields,
            required=test_fields,
            label=f"exact-sibling transfer baseline mask_tests[{index}]",
        )
        target_branch_row = _context_uint(
            test.get("target_branch_call_row"),
            f"exact-sibling transfer mask_tests[{index}].target_branch_call_row",
        )
        target_masu_row = _context_uint(
            test.get("target_masu_call_row"),
            f"exact-sibling transfer mask_tests[{index}].target_masu_call_row",
        )
        candidate_masu_row = _context_uint(
            test.get("candidate_masu_call_row"),
            f"exact-sibling transfer mask_tests[{index}].candidate_masu_call_row",
        )
        candidate_branch_row = _context_uint(
            test.get("candidate_branch_call_row"),
            f"exact-sibling transfer mask_tests[{index}].candidate_branch_call_row",
        )
        if target_branch_row >= target_masu_row or candidate_masu_row >= candidate_branch_row:
            raise LearningInputError(
                "exact-sibling transfer call rows must encode target branch-before-masu "
                "and candidate masu-before-branch order"
            )
        normalized_tests.append(
            {
                "source_left": _context_text(
                    test.get("source_left"),
                    f"exact-sibling transfer mask_tests[{index}].source_left",
                    limit=512,
                ),
                "source_right": _context_text(
                    test.get("source_right"),
                    f"exact-sibling transfer mask_tests[{index}].source_right",
                    limit=512,
                ),
                "source_expression": _context_text(
                    test.get("source_expression"),
                    f"exact-sibling transfer mask_tests[{index}].source_expression",
                    limit=512,
                ),
                "branch_getter": _context_identifier(
                    test.get("branch_getter"),
                    f"exact-sibling transfer mask_tests[{index}].branch_getter",
                ),
                "masu_getter": _context_identifier(
                    test.get("masu_getter"),
                    f"exact-sibling transfer mask_tests[{index}].masu_getter",
                ),
                "target_branch_call_row": target_branch_row,
                "target_masu_call_row": target_masu_row,
                "candidate_masu_call_row": candidate_masu_row,
                "candidate_branch_call_row": candidate_branch_row,
                "evidence_sha256": _context_sha256(
                    test.get("evidence_sha256"),
                    f"exact-sibling transfer mask_tests[{index}].evidence_sha256",
                ),
            }
        )

    shared = _closed_context(
        baseline.get("shared_boolean"),
        allowed={
            "target_branch_rows",
            "target_true_assignment_row",
            "target_false_assignment_row",
            "candidate_true_assignment_rows",
            "candidate_false_assignment_row",
            "result_register",
            "result_owner",
            "evidence_sha256",
        },
        required={
            "target_branch_rows",
            "target_true_assignment_row",
            "target_false_assignment_row",
            "candidate_true_assignment_rows",
            "candidate_false_assignment_row",
            "result_register",
            "result_owner",
            "evidence_sha256",
        },
        label="exact-sibling transfer baseline shared_boolean",
    )

    def two_rows(raw: Any, label: str) -> list[int]:
        if not isinstance(raw, list) or len(raw) != 2:
            raise LearningInputError(f"{label} must contain exactly two rows")
        result = [_context_uint(item, f"{label}[{index}]") for index, item in enumerate(raw)]
        if len(set(result)) != 2:
            raise LearningInputError(f"{label} rows must be distinct")
        return result

    result_register = _context_text(
        shared.get("result_register"),
        "exact-sibling transfer shared_boolean.result_register",
        limit=3,
    ).lower()
    if not _saved(result_register, "r"):
        raise LearningInputError(
            "exact-sibling transfer Boolean result must use a nonvolatile GPR"
        )
    normalized_shared = {
        "target_branch_rows": two_rows(
            shared.get("target_branch_rows"),
            "exact-sibling transfer shared_boolean.target_branch_rows",
        ),
        "target_true_assignment_row": _context_uint(
            shared.get("target_true_assignment_row"),
            "exact-sibling transfer shared_boolean.target_true_assignment_row",
        ),
        "target_false_assignment_row": _context_uint(
            shared.get("target_false_assignment_row"),
            "exact-sibling transfer shared_boolean.target_false_assignment_row",
        ),
        "candidate_true_assignment_rows": two_rows(
            shared.get("candidate_true_assignment_rows"),
            "exact-sibling transfer shared_boolean.candidate_true_assignment_rows",
        ),
        "candidate_false_assignment_row": _context_uint(
            shared.get("candidate_false_assignment_row"),
            "exact-sibling transfer shared_boolean.candidate_false_assignment_row",
        ),
        "result_register": result_register,
        "result_owner": _context_identifier(
            shared.get("result_owner"),
            "exact-sibling transfer shared_boolean.result_owner",
        ),
        "evidence_sha256": _context_sha256(
            shared.get("evidence_sha256"),
            "exact-sibling transfer shared_boolean.evidence_sha256",
        ),
    }

    boundary = _closed_context(
        context.get("type_boundary"),
        allowed={
            "owner",
            "source_type",
            "consumer_type",
            "target_extsh_rows",
            "target_consumer_call_rows",
            "consumer_symbols",
            "evidence_sha256",
        },
        required={
            "owner",
            "source_type",
            "consumer_type",
            "target_extsh_rows",
            "target_consumer_call_rows",
            "consumer_symbols",
            "evidence_sha256",
        },
        label="exact-sibling transfer type_boundary",
    )

    def three_rows(raw: Any, label: str) -> list[int]:
        if not isinstance(raw, list) or len(raw) != 3:
            raise LearningInputError(f"{label} must contain exactly three rows")
        result = [_context_uint(item, f"{label}[{index}]") for index, item in enumerate(raw)]
        if len(set(result)) != 3:
            raise LearningInputError(f"{label} rows must be distinct")
        return result

    consumers = boundary.get("consumer_symbols")
    if not isinstance(consumers, list) or len(consumers) != 3:
        raise LearningInputError(
            "exact-sibling transfer type_boundary.consumer_symbols must contain three entries"
        )
    normalized_boundary = {
        "owner": _context_identifier(
            boundary.get("owner"), "exact-sibling transfer type_boundary.owner"
        ),
        "source_type": _context_identifier(
            boundary.get("source_type"),
            "exact-sibling transfer type_boundary.source_type",
        ),
        "consumer_type": _context_identifier(
            boundary.get("consumer_type"),
            "exact-sibling transfer type_boundary.consumer_type",
        ),
        "target_extsh_rows": three_rows(
            boundary.get("target_extsh_rows"),
            "exact-sibling transfer type_boundary.target_extsh_rows",
        ),
        "target_consumer_call_rows": three_rows(
            boundary.get("target_consumer_call_rows"),
            "exact-sibling transfer type_boundary.target_consumer_call_rows",
        ),
        "consumer_symbols": [
            _context_identifier(
                symbol,
                f"exact-sibling transfer type_boundary.consumer_symbols[{index}]",
            )
            for index, symbol in enumerate(consumers)
        ],
        "evidence_sha256": _context_sha256(
            boundary.get("evidence_sha256"),
            "exact-sibling transfer type_boundary.evidence_sha256",
        ),
    }
    if (
        normalized_boundary["source_type"] != "int"
        or normalized_boundary["consumer_type"] != "s16"
    ):
        raise LearningInputError(
            "exact-sibling transfer type boundary must be int source to s16 consumers"
        )
    if any(
        extsh_row >= call_row
        for extsh_row, call_row in zip(
            normalized_boundary["target_extsh_rows"],
            normalized_boundary["target_consumer_call_rows"],
        )
    ):
        raise LearningInputError(
            "each exact-sibling transfer extsh row must precede its consumer call row"
        )

    capacity = _closed_context(
        context.get("capacity"),
        allowed={
            "array_name",
            "macro",
            "value",
            "element_size",
            "target_extent_bytes",
            "source_location",
            "evidence_sha256",
        },
        required={
            "array_name",
            "macro",
            "value",
            "element_size",
            "target_extent_bytes",
            "source_location",
            "evidence_sha256",
        },
        label="exact-sibling transfer capacity",
    )
    normalized_capacity = {
        "array_name": _context_identifier(
            capacity.get("array_name"), "exact-sibling transfer capacity.array_name"
        ),
        "macro": _context_identifier(
            capacity.get("macro"), "exact-sibling transfer capacity.macro"
        ),
        "value": _context_uint(
            capacity.get("value"), "exact-sibling transfer capacity.value", minimum=1
        ),
        "element_size": _context_uint(
            capacity.get("element_size"),
            "exact-sibling transfer capacity.element_size",
            minimum=1,
        ),
        "target_extent_bytes": _context_uint(
            capacity.get("target_extent_bytes"),
            "exact-sibling transfer capacity.target_extent_bytes",
            minimum=1,
        ),
        "source_location": _context_text(
            capacity.get("source_location"),
            "exact-sibling transfer capacity.source_location",
            limit=512,
        ),
        "evidence_sha256": _context_sha256(
            capacity.get("evidence_sha256"),
            "exact-sibling transfer capacity.evidence_sha256",
        ),
    }
    if (
        normalized_capacity["value"] * normalized_capacity["element_size"]
        != normalized_capacity["target_extent_bytes"]
    ):
        raise LearningInputError(
            "exact-sibling transfer capacity does not equal value * element_size"
        )

    combined = _closed_context(
        context.get("combined_cell"),
        allowed={
            "candidate_id",
            "target_size",
            "candidate_size",
            "object_sha256",
            "candidate_record_sha256",
        },
        required={
            "candidate_id",
            "target_size",
            "candidate_size",
            "object_sha256",
            "candidate_record_sha256",
        },
        label="exact-sibling transfer combined_cell",
    )
    normalized_combined = {
        "candidate_id": _context_text(
            combined.get("candidate_id"),
            "exact-sibling transfer combined_cell.candidate_id",
            limit=128,
        ),
        "target_size": _context_uint(
            combined.get("target_size"),
            "exact-sibling transfer combined_cell.target_size",
            minimum=1,
        ),
        "candidate_size": _context_uint(
            combined.get("candidate_size"),
            "exact-sibling transfer combined_cell.candidate_size",
            minimum=1,
        ),
        "object_sha256": _context_sha256(
            combined.get("object_sha256"),
            "exact-sibling transfer combined_cell.object_sha256",
        ),
        "candidate_record_sha256": _context_sha256(
            combined.get("candidate_record_sha256"),
            "exact-sibling transfer combined_cell.candidate_record_sha256",
        ),
    }
    if normalized_combined["target_size"] != normalized_combined["candidate_size"]:
        raise LearningInputError(
            "exact-sibling transfer combined cell must be size exact"
        )

    if normalized_donor["source_expressions"] != [
        test["source_expression"] for test in normalized_tests
    ]:
        raise LearningInputError(
            "exact-sibling transfer donor expressions must match the transferred mask tests"
        )

    return {
        "schema": EXACT_SIBLING_TRANSFER_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "donor": normalized_donor,
        "baseline": {
            "mask_tests": normalized_tests,
            "shared_boolean": normalized_shared,
        },
        "type_boundary": normalized_boundary,
        "capacity": normalized_capacity,
        "combined_cell": normalized_combined,
    }


def _parse_capacity_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={
            "schema",
            "proofs",
            "array",
            "producer_contracts",
            "declaration_positions",
        },
        required={
            "schema",
            "proofs",
            "array",
            "producer_contracts",
            "declaration_positions",
        },
        label="capacity context",
    )
    if (
        _context_text(context.get("schema"), "capacity context schema")
        != CAPACITY_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"capacity context schema must be {CAPACITY_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_CAPACITY_PROOF_FLAGS) | set(_CAPACITY_PROOF_HASHES)
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="capacity context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _CAPACITY_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(f"capacity context proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _CAPACITY_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"capacity context proofs.{field}"
        )

    array = _closed_context(
        context.get("array"),
        allowed={
            "name",
            "element_size",
            "candidate_capacity",
            "used_prefix_elements",
            "candidate_extent_bytes",
            "target_extent_bytes",
        },
        required={
            "name",
            "element_size",
            "candidate_capacity",
            "used_prefix_elements",
            "candidate_extent_bytes",
            "target_extent_bytes",
        },
        label="capacity context array",
    )
    normalized_array = {
        "name": _context_identifier(array.get("name"), "capacity context array.name"),
        "element_size": _context_uint(
            array.get("element_size"),
            "capacity context array.element_size",
            minimum=1,
            maximum=4096,
        ),
        "candidate_capacity": _context_uint(
            array.get("candidate_capacity"),
            "capacity context array.candidate_capacity",
            minimum=1,
        ),
        "used_prefix_elements": _context_uint(
            array.get("used_prefix_elements"),
            "capacity context array.used_prefix_elements",
            minimum=1,
        ),
        "candidate_extent_bytes": _context_uint(
            array.get("candidate_extent_bytes"),
            "capacity context array.candidate_extent_bytes",
            minimum=1,
        ),
        "target_extent_bytes": _context_uint(
            array.get("target_extent_bytes"),
            "capacity context array.target_extent_bytes",
            minimum=1,
        ),
    }

    raw_contracts = context.get("producer_contracts")
    if not isinstance(raw_contracts, list) or not 1 <= len(raw_contracts) <= 8:
        raise LearningInputError(
            "capacity context producer_contracts must contain 1-8 entries"
        )
    contracts: list[dict[str, Any]] = []
    contract_fields = {"provider", "source_location", "maximum", "evidence_sha256"}
    for index, raw_contract in enumerate(raw_contracts):
        contract = _closed_context(
            raw_contract,
            allowed=contract_fields,
            required=contract_fields,
            label=f"capacity context producer_contracts[{index}]",
        )
        contracts.append(
            {
                "provider": _context_identifier(
                    contract.get("provider"),
                    f"capacity context producer_contracts[{index}].provider",
                ),
                "source_location": _context_text(
                    contract.get("source_location"),
                    f"capacity context producer_contracts[{index}].source_location",
                    limit=512,
                ),
                "maximum": _context_uint(
                    contract.get("maximum"),
                    f"capacity context producer_contracts[{index}].maximum",
                    minimum=1,
                ),
                "evidence_sha256": _context_sha256(
                    contract.get("evidence_sha256"),
                    f"capacity context producer_contracts[{index}].evidence_sha256",
                ),
            }
        )
    if len({item["provider"] for item in contracts}) != len(contracts):
        raise LearningInputError("capacity context producer providers must be unique")

    raw_positions = context.get("declaration_positions")
    if not isinstance(raw_positions, list) or not 1 <= len(raw_positions) <= 8:
        raise LearningInputError(
            "capacity context declaration_positions must contain 1-8 entries"
        )
    positions = [
        _context_identifier(item, f"capacity context declaration_positions[{index}]")
        for index, item in enumerate(raw_positions)
    ]
    if len(set(positions)) != len(positions):
        raise LearningInputError(
            "capacity context declaration_positions must be unique"
        )
    return {
        "schema": CAPACITY_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "array": normalized_array,
        "producer_contracts": sorted(contracts, key=lambda item: item["provider"]),
        "declaration_positions": positions,
    }


def _parse_branch_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={"schema", "proofs", "branch"},
        required={"schema", "proofs", "branch"},
        label="branch context",
    )
    if (
        _context_text(context.get("schema"), "branch context schema")
        != BRANCH_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"branch context schema must be {BRANCH_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_BRANCH_PROOF_FLAGS) | set(_BRANCH_PROOF_HASHES)
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="branch context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _BRANCH_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(f"branch context proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _BRANCH_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"branch context proofs.{field}"
        )

    branch = _closed_context(
        context.get("branch"),
        allowed={
            "row_index",
            "guard_class",
            "target_destination",
            "candidate_destination",
            "target_relative_target",
            "candidate_relative_target",
        },
        required={
            "row_index",
            "guard_class",
            "target_destination",
            "candidate_destination",
            "target_relative_target",
            "candidate_relative_target",
        },
        label="branch context branch",
    )
    guard_class = _context_identifier(
        branch.get("guard_class"), "branch context branch.guard_class"
    )
    if guard_class != "zero_terminator":
        raise LearningInputError(
            "branch context branch.guard_class must be zero_terminator"
        )
    target_destination = _context_identifier(
        branch.get("target_destination"),
        "branch context branch.target_destination",
    )
    candidate_destination = _context_identifier(
        branch.get("candidate_destination"),
        "branch context branch.candidate_destination",
    )
    destination_classes = {"loop_increment", "loop_exit"}
    if {target_destination, candidate_destination} - destination_classes:
        raise LearningInputError(
            "branch context destinations must be loop_increment or loop_exit"
        )
    if target_destination == candidate_destination:
        raise LearningInputError("branch context destinations must differ")
    return {
        "schema": BRANCH_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "branch": {
            "row_index": _context_uint(
                branch.get("row_index"), "branch context branch.row_index"
            ),
            "guard_class": guard_class,
            "target_destination": target_destination,
            "candidate_destination": candidate_destination,
            "target_relative_target": _context_uint(
                branch.get("target_relative_target"),
                "branch context branch.target_relative_target",
                maximum=0x7FFFFFFF,
            ),
            "candidate_relative_target": _context_uint(
                branch.get("candidate_relative_target"),
                "branch context branch.candidate_relative_target",
                maximum=0x7FFFFFFF,
            ),
        },
    }


def _parse_reciprocal_context(value: Mapping[str, Any]) -> dict[str, Any]:
    context = _closed_context(
        value,
        allowed={"schema", "proofs", "window", "neutral_observation"},
        required={"schema", "proofs", "window", "neutral_observation"},
        label="reciprocal context",
    )
    if (
        _context_text(context.get("schema"), "reciprocal context schema")
        != RECIPROCAL_CONTEXT_SCHEMA
    ):
        raise LearningInputError(
            f"reciprocal context schema must be {RECIPROCAL_CONTEXT_SCHEMA}"
        )

    proof_fields = set(_RECIPROCAL_PROOF_FLAGS) | set(_RECIPROCAL_PROOF_HASHES)
    proofs = _closed_context(
        context.get("proofs"),
        allowed=proof_fields,
        required=proof_fields,
        label="reciprocal context proofs",
    )
    normalized_proofs: dict[str, Any] = {}
    for field in _RECIPROCAL_PROOF_FLAGS:
        if proofs.get(field) is not True:
            raise LearningInputError(f"reciprocal context proofs.{field} must be true")
        normalized_proofs[field] = True
    for field in _RECIPROCAL_PROOF_HASHES:
        normalized_proofs[field] = _context_sha256(
            proofs.get(field), f"reciprocal context proofs.{field}"
        )

    row_fields = {
        "target_variable_row",
        "candidate_variable_row",
        "target_reciprocal_row",
        "candidate_reciprocal_row",
        "multiply_row",
    }
    window = _closed_context(
        context.get("window"),
        allowed=row_fields
        | {"invariant_constant_rows", "denominator", "reciprocal_f32_bits"},
        required=row_fields
        | {"invariant_constant_rows", "denominator", "reciprocal_f32_bits"},
        label="reciprocal context window",
    )
    normalized_window = {
        field: _context_uint(window.get(field), f"reciprocal context window.{field}")
        for field in sorted(row_fields)
    }
    raw_invariants = window.get("invariant_constant_rows")
    if not isinstance(raw_invariants, list) or not 1 <= len(raw_invariants) <= 8:
        raise LearningInputError(
            "reciprocal context window.invariant_constant_rows must contain 1-8 entries"
        )
    invariant_rows = [
        _context_uint(
            item,
            f"reciprocal context window.invariant_constant_rows[{index}]",
        )
        for index, item in enumerate(raw_invariants)
    ]
    if len(set(invariant_rows)) != len(invariant_rows):
        raise LearningInputError(
            "reciprocal context window.invariant_constant_rows must be unique"
        )
    bits = _context_text(
        window.get("reciprocal_f32_bits"),
        "reciprocal context window.reciprocal_f32_bits",
        limit=8,
    )
    if re.fullmatch(r"[0-9a-f]{8}", bits) is None:
        raise LearningInputError(
            "reciprocal context window.reciprocal_f32_bits must be eight lowercase hex digits"
        )
    normalized_window.update(
        {
            "invariant_constant_rows": invariant_rows,
            "denominator": _context_uint(
                window.get("denominator"),
                "reciprocal context window.denominator",
                minimum=2,
                maximum=1 << 24,
            ),
            "reciprocal_f32_bits": bits,
        }
    )
    if not (
        normalized_window["target_variable_row"]
        == normalized_window["candidate_reciprocal_row"]
        and normalized_window["target_reciprocal_row"]
        == normalized_window["candidate_variable_row"]
        and normalized_window["target_variable_row"]
        != normalized_window["target_reciprocal_row"]
    ):
        raise LearningInputError(
            "reciprocal context rows must describe one variable/reciprocal load-order swap"
        )
    variable_rows = {
        normalized_window["target_variable_row"],
        normalized_window["target_reciprocal_row"],
        normalized_window["multiply_row"],
    }
    if len(variable_rows) != 3 or variable_rows & set(invariant_rows):
        raise LearningInputError(
            "reciprocal context window rows must be distinct and disjoint"
        )

    neutral = _closed_context(
        context.get("neutral_observation"),
        allowed={"axis", "baseline_object_sha256", "candidate_object_sha256"},
        required={"axis", "baseline_object_sha256", "candidate_object_sha256"},
        label="reciprocal context neutral_observation",
    )
    axis = _context_identifier(
        neutral.get("axis"), "reciprocal context neutral_observation.axis"
    )
    if axis != "commuted_multiply":
        raise LearningInputError(
            "reciprocal context neutral_observation.axis must be commuted_multiply"
        )
    return {
        "schema": RECIPROCAL_CONTEXT_SCHEMA,
        "proofs": normalized_proofs,
        "window": normalized_window,
        "neutral_observation": {
            "axis": axis,
            "baseline_object_sha256": _context_sha256(
                neutral.get("baseline_object_sha256"),
                "reciprocal context neutral_observation.baseline_object_sha256",
            ),
            "candidate_object_sha256": _context_sha256(
                neutral.get("candidate_object_sha256"),
                "reciprocal context neutral_observation.candidate_object_sha256",
            ),
        },
    }


def _evaluation(
    rule_id: str,
    *,
    matched: bool,
    reason: str,
    confidence: float | None = None,
    source_class: str | None = None,
    recommendation: str | None = None,
    evidence: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "rule_id": rule_id,
        "matched": matched,
        "reason": reason,
        "evidence": dict(evidence or {}),
    }
    if matched:
        assert confidence is not None
        assert source_class is not None
        assert recommendation is not None
        result.update(
            {
                "confidence": confidence,
                "source_class": source_class,
                "recommendation": recommendation,
                "limitations": [
                    "The diagnosis ranks a natural source-shape class; it does not prove original spelling or provenance.",
                    "Do not edit or retain source from this result alone; strict/data/physical-relocation/section and protected-sibling gates remain required.",
                ],
            }
        )
    return result


def _explicit_else_evaluation(audit: Mapping[str, Any]) -> dict[str, Any]:
    matches = [
        item
        for item in audit.get("hypotheses", [])
        if isinstance(item, Mapping)
        and item.get("classification") == "explicit_else_return_epilogue"
    ]
    if not matches:
        return _evaluation(
            "explicit_else_return_cfg",
            matched=False,
            reason="the installed causal reducer found no explicit else-return epilogue topology",
        )
    primary = matches[0]
    evidence = primary.get("evidence")
    return _evaluation(
        "explicit_else_return_cfg",
        matched=True,
        reason="the installed causal reducer matched its narrow explicit else-return CFG signature",
        confidence=float(primary.get("confidence", 0.0)),
        source_class="explicit_else_return_control_flow",
        recommendation="Test an explicit else-return control-flow form around the guarded body.",
        evidence={
            "causal_classification": primary.get("classification"),
            "causal_rank": primary.get("rank"),
            "causal_evidence": dict(evidence) if isinstance(evidence, Mapping) else {},
        },
    )


def _loop_branch_destination_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "loop_branch_destination"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated loop branch-destination context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the branch context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or target_size != candidate_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate function sizes are not exact",
            evidence={"target_size": target_size, "candidate_size": candidate_size},
        )
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if target_frame is None or target_frame != candidate_frame:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate stack frames are not exact and measurable",
            evidence={"target_frame": target_frame, "candidate_frame": candidate_frame},
        )

    rows = causal_reducer._paired_records(target, candidate)
    mismatch_rows = [
        index
        for index, (left, right) in enumerate(rows)
        if causal_reducer._instruction_mismatch(left, right)
    ]
    row_index = context["branch"]["row_index"]
    if mismatch_rows != [row_index] or row_index >= len(rows):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the report does not contain exactly the context-bound branch residual",
            evidence={
                "context_row_index": row_index,
                "mismatch_rows": mismatch_rows,
            },
        )
    left, right = rows[row_index]
    if (
        left is None
        or right is None
        or not left.has_instruction
        or not right.has_instruction
        or left.mnemonic != right.mnemonic
        or left.mnemonic not in _CONDITIONAL_MNEMONICS
        or causal_reducer._relocation_diff(left, right)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sole residual is not one relocation-identical conditional branch",
        )
    target_relative = causal_reducer._branch_relative(left)
    candidate_relative = causal_reducer._branch_relative(right)
    if (
        target_relative is None
        or candidate_relative is None
        or target_relative == candidate_relative
        or target_relative != context["branch"]["target_relative_target"]
        or candidate_relative != context["branch"]["candidate_relative_target"]
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the physical branch destinations do not match the sealed semantic classification",
            evidence={
                "target_relative_target": target_relative,
                "candidate_relative_target": candidate_relative,
                "context_branch": context["branch"],
            },
        )
    if not (
        context["branch"]["target_destination"] == "loop_exit"
        and context["branch"]["candidate_destination"] == "loop_increment"
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the context is not the reviewed candidate-increment versus target-exit class",
            evidence={"context_branch": context["branch"]},
        )
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an otherwise exact loop has one authenticated zero-terminator branch whose "
            "candidate destination is the increment and target destination is the loop exit"
        ),
        confidence=0.99,
        source_class="explicit_else_break_loop_terminator",
        recommendation=(
            "Test one natural explicit else-break cell for the zero terminator; do not run "
            "generic CFG or identical-arm permutations."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "row_index": row_index,
            "mnemonic": left.mnemonic,
            "target_relative_target": target_relative,
            "candidate_relative_target": candidate_relative,
            "guard_class": context["branch"]["guard_class"],
            "target_destination": context["branch"]["target_destination"],
            "candidate_destination": context["branch"]["candidate_destination"],
            "proofs": context["proofs"],
        },
    )


def _stack_extent_interface_capacity_evaluation(
    pair: causal_reducer.FunctionPair,
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "stack_extent_interface_capacity"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated stack-extent/interface-capacity context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the capacity context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or target_size != candidate_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate function sizes are not exact",
            evidence={"target_size": target_size, "candidate_size": candidate_size},
        )

    array = context["array"]
    element_size = array["element_size"]
    candidate_extent = array["candidate_extent_bytes"]
    target_extent = array["target_extent_bytes"]
    if candidate_extent != array["candidate_capacity"] * element_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the candidate array capacity does not reproduce its sealed byte extent",
            evidence={"array": array},
        )
    if array["used_prefix_elements"] > array["candidate_capacity"]:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the used prefix exceeds the candidate array capacity",
            evidence={"array": array},
        )
    missing_extent = target_extent - candidate_extent
    if (
        missing_extent <= 0
        or target_extent % element_size != 0
        or missing_extent % element_size != 0
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target-only extent is not a positive whole-element capacity delta",
            evidence={
                "element_size": element_size,
                "candidate_extent_bytes": candidate_extent,
                "target_extent_bytes": target_extent,
                "missing_extent_bytes": missing_extent,
            },
        )
    predicted_capacity = target_extent // element_size
    extra_elements = missing_extent // element_size
    contract_maxima = sorted(
        {int(item["maximum"]) for item in context["producer_contracts"]}
    )
    if contract_maxima != [predicted_capacity]:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the authenticated producer maxima do not converge on the measured target capacity",
            evidence={
                "predicted_capacity": predicted_capacity,
                "contract_maxima": contract_maxima,
                "producer_contracts": context["producer_contracts"],
            },
        )
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "a positive whole-element target stack extent and authenticated producer maxima "
            "independently converge on one live array capacity"
        ),
        confidence=0.99,
        source_class="live_array_capacity_from_stack_extent_and_interface_contract",
        recommendation=(
            "Test only the predicted live capacity across the sealed declaration positions; "
            "do not model the extent as padding, dead storage, or register shaping."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "array_name": array["name"],
            "element_size": element_size,
            "used_prefix_elements": array["used_prefix_elements"],
            "candidate_capacity": array["candidate_capacity"],
            "candidate_extent_bytes": candidate_extent,
            "target_extent_bytes": target_extent,
            "missing_extent_bytes": missing_extent,
            "extra_elements": extra_elements,
            "predicted_capacity": predicted_capacity,
            "producer_contracts": context["producer_contracts"],
            "declaration_positions": context["declaration_positions"],
            "proofs": context["proofs"],
        },
    )


def _relocation_type_signature(
    item: causal_reducer.Instruction,
) -> tuple[tuple[str, Any], ...] | None:
    if item.relocation is None:
        return None
    return tuple(
        (field, item.relocation[field])
        for field in ("type", "type_name")
        if field in item.relocation
    )


def _mapped_pool_relocation_text(item: causal_reducer.Instruction) -> str | None:
    """Normalize only report-authenticated SDA21 pool-owner aliases."""

    if item.relocation is None or item.relocation.get("type_name") != "R_PPC_EMB_SDA21":
        return None
    return re.sub(
        r"[A-Za-z_.$@][A-Za-z0-9_.$@]*@sda21",
        "<pool-owner>@sda21",
        item.formatted.lower(),
    )


def _mapped_pool_relocation_alias_pair(
    left: causal_reducer.Instruction,
    right: causal_reducer.Instruction,
) -> bool:
    left_text = _mapped_pool_relocation_text(left)
    right_text = _mapped_pool_relocation_text(right)
    return (
        left.diff_kind is None
        and right.diff_kind is None
        and left_text is not None
        and right_text is not None
        and _relocation_type_signature(left) == _relocation_type_signature(right)
        and left.relocation is not None
        and right.relocation is not None
        and left.relocation.get("addend") == right.relocation.get("addend")
        and _registers(left.formatted) == _registers(right.formatted)
        and left_text == right_text
    )


def _equivalent_outside_learning_window(
    left: causal_reducer.Instruction | None,
    right: causal_reducer.Instruction | None,
) -> bool:
    if left is None or right is None:
        return left is right
    if left.has_instruction != right.has_instruction:
        return False
    if not left.has_instruction:
        return True
    if left.mnemonic != right.mnemonic or causal_reducer._relocation_diff(left, right):
        return False
    if left.mnemonic in causal_reducer._BRANCH_MNEMONICS:
        return causal_reducer._branch_relative(left) == causal_reducer._branch_relative(
            right
        )
    return left.formatted == right.formatted


def _reciprocal_source_shape_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "reciprocal_source_shape"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated reciprocal-source-shape context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the reciprocal context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or target_size != candidate_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate function sizes are not exact",
            evidence={"target_size": target_size, "candidate_size": candidate_size},
        )

    window = context["window"]
    denominator = window["denominator"]
    if denominator & (denominator - 1):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed denominator is not a power of two with an exact binary reciprocal",
            evidence={"denominator": denominator},
        )
    reciprocal_bits = struct.pack(">f", 1.0 / denominator).hex()
    if reciprocal_bits != window["reciprocal_f32_bits"]:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed f32 literal is not the exact reciprocal of the denominator",
            evidence={
                "denominator": denominator,
                "computed_reciprocal_f32_bits": reciprocal_bits,
                "context_reciprocal_f32_bits": window["reciprocal_f32_bits"],
            },
        )
    neutral = context["neutral_observation"]
    if neutral["baseline_object_sha256"] != neutral["candidate_object_sha256"]:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the commuted-multiply control was not proved compiler-neutral by object identity",
            evidence={"neutral_observation": neutral},
        )

    rows = causal_reducer._paired_records(target, candidate)
    all_window_rows = set(window["invariant_constant_rows"]) | {
        window["target_variable_row"],
        window["target_reciprocal_row"],
        window["multiply_row"],
    }
    if not all(index < len(rows) for index in all_window_rows):
        return _evaluation(
            rule_id,
            matched=False,
            reason="one or more reciprocal window rows are outside the focus function",
        )
    outside_residuals = [
        index
        for index, (left, right) in enumerate(rows)
        if index not in all_window_rows
        and not _equivalent_outside_learning_window(left, right)
    ]
    if outside_residuals:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the report has physical residuals outside the sealed reciprocal window",
            evidence={"outside_residual_rows": outside_residuals},
        )

    invariant_evidence: list[dict[str, Any]] = []
    for index in window["invariant_constant_rows"]:
        left, right = rows[index]
        if (
            left is None
            or right is None
            or not left.has_instruction
            or not right.has_instruction
            or left.mnemonic != "lfs"
            or right.mnemonic != "lfs"
            or left.relocation is None
            or right.relocation is None
            or _relocation_type_signature(left) != _relocation_type_signature(right)
            or _registers(left.formatted, "f")[:1]
            != _registers(right.formatted, "f")[:1]
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="an invariant constant row is not one typed, relocation-compatible f32 load",
                evidence={"row_index": index},
            )
        invariant_evidence.append(
            {
                "row_index": index,
                "target_formatted": left.formatted,
                "candidate_formatted": right.formatted,
                "relocation_type": [
                    list(item) for item in (_relocation_type_signature(left) or ())
                ],
            }
        )

    target_variable, candidate_reciprocal = rows[window["target_variable_row"]]
    target_reciprocal, candidate_variable = rows[window["target_reciprocal_row"]]
    multiply_target, multiply_candidate = rows[window["multiply_row"]]
    load_items = (
        target_variable,
        candidate_reciprocal,
        target_reciprocal,
        candidate_variable,
    )
    if any(
        item is None or not item.has_instruction or item.mnemonic != "lfs"
        for item in load_items
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the reciprocal seam is not a two-row f32 load-order swap",
        )
    assert target_variable is not None
    assert candidate_reciprocal is not None
    assert target_reciprocal is not None
    assert candidate_variable is not None
    if (
        target_variable.relocation is not None
        or candidate_variable.relocation is not None
        or target_reciprocal.relocation is None
        or candidate_reciprocal.relocation is None
        or _relocation_type_signature(target_reciprocal)
        != _relocation_type_signature(candidate_reciprocal)
        or _without_registers(target_variable.formatted)
        != _without_registers(candidate_variable.formatted)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="variable and reciprocal operands do not preserve their authenticated physical classes",
        )
    target_variable_register = _registers(target_variable.formatted, "f")[:1]
    candidate_variable_register = _registers(candidate_variable.formatted, "f")[:1]
    target_reciprocal_register = _registers(target_reciprocal.formatted, "f")[:1]
    candidate_reciprocal_register = _registers(candidate_reciprocal.formatted, "f")[:1]
    if (
        len(target_variable_register) != 1
        or len(candidate_variable_register) != 1
        or len(target_reciprocal_register) != 1
        or len(candidate_reciprocal_register) != 1
        or target_variable_register != candidate_reciprocal_register
        or target_reciprocal_register != candidate_variable_register
        or target_variable_register == target_reciprocal_register
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the two swapped loads do not exchange the exact multiply input registers",
        )
    if (
        multiply_target is None
        or multiply_candidate is None
        or not multiply_target.has_instruction
        or not multiply_candidate.has_instruction
        or multiply_target.mnemonic != "fmuls"
        or multiply_target.formatted != multiply_candidate.formatted
        or causal_reducer._relocation_diff(multiply_target, multiply_candidate)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the consuming single-precision multiply is not physically exact",
        )
    multiply_registers = _registers(multiply_target.formatted, "f")
    if len(multiply_registers) != 3 or set(multiply_registers[1:]) != {
        target_variable_register[0],
        target_reciprocal_register[0],
    }:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the swapped loads do not feed both operands of the sealed fmuls",
            evidence={"multiply_formatted": multiply_target.formatted},
        )

    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an otherwise exact function has one authenticated variable/reciprocal f32 "
            "load-order swap, an exact fmuls consumer, and an object-identical commuted control"
        ),
        confidence=0.99,
        source_class="exact_power_of_two_division_source_shape",
        recommendation=(
            f"Test one natural division by {denominator}.0f cell and suppress further "
            "commutative multiply permutations."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "denominator": denominator,
            "reciprocal_f32_bits": reciprocal_bits,
            "target_variable_row": window["target_variable_row"],
            "target_reciprocal_row": window["target_reciprocal_row"],
            "multiply_row": window["multiply_row"],
            "target_variable_register": target_variable_register[0],
            "target_reciprocal_register": target_reciprocal_register[0],
            "invariant_constant_rows": invariant_evidence,
            "neutral_observation": neutral,
            "proofs": context["proofs"],
        },
    )


def _compatible_register_only_pair(
    left: causal_reducer.Instruction,
    right: causal_reducer.Instruction,
) -> bool:
    if not left.has_instruction or not right.has_instruction:
        return False
    if left.mnemonic != right.mnemonic:
        return False
    if causal_reducer._relocation_diff(left, right):
        # Objdiff can map two physical pool-owner names to one exact value and
        # therefore emit no residual row even though the normalized target_name
        # strings differ.  Accept only that report-authenticated alias class:
        # both rows must be unmarked, have the same relocation type/addend, and
        # use the same registers.  A real strict relocation residual remains
        # rejected.
        return _mapped_pool_relocation_alias_pair(left, right)
    if left.mnemonic in causal_reducer._BRANCH_MNEMONICS:
        return causal_reducer._branch_relative(left) == causal_reducer._branch_relative(
            right
        )
    return _without_registers(left.formatted) == _without_registers(
        right.formatted
    ) or _mapped_pool_relocation_alias_pair(left, right)


def _closed_cycles(mapping: Mapping[str, str]) -> list[list[str]]:
    if set(mapping) != set(mapping.values()):
        return []
    cycles: list[list[str]] = []
    visited: set[str] = set()
    for start in sorted(mapping):
        if start in visited:
            continue
        cycle: list[str] = []
        current = start
        while current not in cycle and current not in visited:
            cycle.append(current)
            visited.add(current)
            current = mapping[current]
        if current != start:
            return []
        if len(cycle) > 1:
            cycles.append(cycle)
    return cycles


def _call_result_consumers(
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    mapping: Mapping[str, str],
) -> list[dict[str, Any]]:
    rows = causal_reducer._paired_records(target, candidate)
    result: list[dict[str, Any]] = []
    for call_index, (left_call, right_call) in enumerate(rows):
        if (
            left_call is None
            or right_call is None
            or left_call.mnemonic not in _CALL_MNEMONICS
            or right_call.mnemonic != left_call.mnemonic
        ):
            continue
        for capture_index in range(call_index + 1, min(len(rows), call_index + 4)):
            left_capture, right_capture = rows[capture_index]
            if left_capture is None or right_capture is None:
                continue
            left_regs = _registers(left_capture.formatted, "r")
            right_regs = _registers(right_capture.formatted, "r")
            if (
                left_capture.mnemonic != "mr"
                or right_capture.mnemonic != "mr"
                or len(left_regs) != 2
                or len(right_regs) != 2
                or left_regs[1] != "r3"
                or right_regs[1] != "r3"
                or mapping.get(left_regs[0]) != right_regs[0]
            ):
                continue
            for compare_index in range(
                capture_index + 1, min(len(rows), capture_index + 4)
            ):
                left_compare, right_compare = rows[compare_index]
                if left_compare is None or right_compare is None:
                    continue
                if (
                    not left_compare.mnemonic.startswith("cmp")
                    or right_compare.mnemonic != left_compare.mnemonic
                    or left_regs[0] not in _registers(left_compare.formatted, "r")
                    or right_regs[0] not in _registers(right_compare.formatted, "r")
                ):
                    continue
                branch_index = next(
                    (
                        index
                        for index in range(
                            compare_index + 1, min(len(rows), compare_index + 3)
                        )
                        if rows[index][0] is not None
                        and rows[index][1] is not None
                        and rows[index][0].mnemonic in _CONDITIONAL_MNEMONICS
                        and rows[index][1].mnemonic == rows[index][0].mnemonic
                        and causal_reducer._branch_relative(rows[index][0])
                        == causal_reducer._branch_relative(rows[index][1])
                    ),
                    None,
                )
                if branch_index is not None:
                    result.append(
                        {
                            "call_index": call_index,
                            "capture_index": capture_index,
                            "compare_index": compare_index,
                            "branch_index": branch_index,
                            "target_result_register": left_regs[0],
                            "candidate_result_register": right_regs[0],
                        }
                    )
                    break
            if result and result[-1]["call_index"] == call_index:
                break
    return result


def _assignment_condition_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
) -> dict[str, Any]:
    if _function_size(pair.target) != _function_size(pair.candidate):
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="target and candidate function sizes differ",
        )
    rows = causal_reducer._paired_records(target, candidate)
    if any(
        left is None or right is None or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="the residual is not an operation-, CFG-, relocation-, and immediate-identical register-only difference",
        )

    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_regs = _registers(left.formatted)
        right_regs = _registers(right.formatted)
        if len(left_regs) != len(right_regs):
            return _evaluation(
                "assignment_condition_saved_gpr_cycle",
                matched=False,
                reason="a register-only row has a different operand count",
            )
        row_mismatch = False
        for target_reg, candidate_reg in zip(left_regs, right_regs):
            if target_reg == candidate_reg:
                continue
            if not (_saved(target_reg, "r") and _saved(candidate_reg, "r")):
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the register difference is not confined to nonvolatile GPRs",
                )
            if mapping.get(target_reg, candidate_reg) != candidate_reg:
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the target-to-candidate GPR mapping is inconsistent",
                )
            if reverse.get(candidate_reg, target_reg) != target_reg:
                return _evaluation(
                    "assignment_condition_saved_gpr_cycle",
                    matched=False,
                    reason="the saved-GPR mapping is not one-to-one",
                )
            mapping[target_reg] = candidate_reg
            reverse[candidate_reg] = target_reg
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)

    cycles = _closed_cycles(mapping)
    if not cycles or max(map(len, cycles)) < 3:
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="no closed saved-GPR cycle of length three or greater is present",
            evidence={"register_mapping": dict(sorted(mapping.items()))},
        )
    consumers = _call_result_consumers(target, candidate, mapping)
    if not consumers:
        return _evaluation(
            "assignment_condition_saved_gpr_cycle",
            matched=False,
            reason="the saved-GPR cycle has no call-result assignment immediately consumed by a condition",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    return _evaluation(
        "assignment_condition_saved_gpr_cycle",
        matched=True,
        reason="an otherwise identical function contains a closed saved-GPR cycle joined to an immediately consumed call-result assignment",
        confidence=0.96,
        source_class="assignment_in_consuming_condition",
        recommendation="Test a natural condition that combines the existing result assignment with its immediate comparison.",
        evidence={
            "target_size": _function_size(pair.target),
            "candidate_size": _function_size(pair.candidate),
            "register_mapping": dict(sorted(mapping.items())),
            "cycles": cycles,
            "mismatch_rows": mismatch_rows,
            "call_result_consumers": consumers,
            "structural_invariants": [
                "mnemonic_sequence",
                "branch_relative_targets",
                "relocations",
                "non_register_operands",
            ],
        },
    )


def _allocator_interaction_request(
    *,
    focus_symbol: str,
    context: Mapping[str, Any],
) -> dict[str, Any]:
    owners = {str(item["lifetime_role"]): item for item in context["owners"]}
    long_lived = owners["long_lived"]
    boundary_owner = owners["producer_consumer_boundary"]
    boundary = context["boundary"]
    request = {
        "schema": interaction_planner.REQUEST_SCHEMA,
        "planner_id": f"allocator-two-register-swap-{focus_symbol}",
        "focus_symbols": [focus_symbol],
        "axes": [
            {
                "id": "declaration_chronology",
                "hypothesis": (
                    "The authenticated long-lived owner must enter frontend chronology "
                    "before the owner born at the producer-consumer boundary."
                ),
                "control_level": "existing",
                "levels": [
                    {
                        "id": "existing",
                        "topology_token": "existing-declaration-chronology",
                        "source_action": "Keep the measured declaration chronology.",
                        "evidence": [
                            f"VarInfo usage class {long_lived['usage_class']} for {long_lived['name']}",
                            str(long_lived["evidence_sha256"]),
                        ],
                        "admissibility": "natural",
                    },
                    {
                        "id": "long-lived-first",
                        "topology_token": "long-lived-owner-declared-first",
                        "source_action": (
                            f"Declare the authenticated long-lived owner {long_lived['name']} "
                            f"before {boundary_owner['name']}."
                        ),
                        "evidence": [
                            (
                                f"target {long_lived['name']}={long_lived['target_register']} "
                                f"candidate={long_lived['candidate_register']}"
                            ),
                            str(context["proofs"]["varinfo_receipt_sha256"]),
                        ],
                        "admissibility": "natural",
                    },
                ],
            },
            {
                "id": "value_identity_boundary",
                "hypothesis": (
                    "The producer result must die where the retained consumer value is born, "
                    "at the authenticated source boundary."
                ),
                "control_level": "split",
                "levels": [
                    {
                        "id": "split",
                        "topology_token": "split-producer-consumer-identity",
                        "source_action": "Keep the measured split producer and consumer identities.",
                        "evidence": [
                            f"producer {boundary['producer']}",
                            str(boundary["evidence_sha256"]),
                        ],
                        "admissibility": "natural",
                    },
                    {
                        "id": "fused",
                        "topology_token": "fused-producer-consumer-boundary",
                        "source_action": (
                            f"Fuse {boundary['producer']} into {boundary['consumer']} across "
                            f"the authenticated transformations: {', '.join(boundary['transformations'])}."
                        ),
                        "evidence": [
                            f"boundary owner {boundary_owner['name']} usage class {boundary_owner['usage_class']}",
                            str(context["proofs"]["source_boundary_receipt_sha256"]),
                        ],
                        "admissibility": "natural",
                    },
                ],
            },
        ],
        "constraints": [],
        "observations": context["observations"],
        "max_cells": 4,
    }
    try:
        interaction_planner._parse_request(request)
    except interaction_planner.InteractionPlanError as exc:
        raise LearningInputError(
            f"allocator context cannot form a closed interaction request: {exc}"
        ) from exc
    return request


def _allocator_two_register_swap_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "allocator_two_register_swap_interaction"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated allocator two-register-swap context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the allocator context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or target_size != candidate_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate function sizes are not exact",
            evidence={"target_size": target_size, "candidate_size": candidate_size},
        )
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if target_frame is None or target_frame != candidate_frame:
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate stack frames are not exact and measurable",
            evidence={"target_frame": target_frame, "candidate_frame": candidate_frame},
        )

    rows = causal_reducer._paired_records(target, candidate)
    if not rows or any(
        left is None or right is None or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason=(
                "the residual is not an operation-, CFG-, relocation-, immediate-, "
                "and row-count-identical register-only difference"
            ),
        )

    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_registers = _registers(left.formatted)
        right_registers = _registers(right.formatted)
        if len(left_registers) != len(right_registers):
            return _evaluation(
                rule_id,
                matched=False,
                reason="a register-only row has a different operand count",
            )
        row_mismatch = False
        for target_register, candidate_register in zip(left_registers, right_registers):
            if target_register == candidate_register:
                continue
            if not (_saved(target_register, "r") and _saved(candidate_register, "r")):
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the residual is not confined to nonvolatile GPR ownership",
                )
            if mapping.get(target_register, candidate_register) != candidate_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is inconsistent",
                )
            if reverse.get(candidate_register, target_register) != target_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is not one-to-one",
                )
            mapping[target_register] = candidate_register
            reverse[candidate_register] = target_register
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)

    cycles = _closed_cycles(mapping)
    if len(cycles) != 1 or len(cycles[0]) != 2 or len(mapping) != 2:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the register residual is not one complete two-register swap",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    context_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in context["owners"]
    }
    if mapping != context_mapping:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the VarInfo owner mapping does not authenticate the physical swap",
            evidence={
                "physical_mapping": dict(sorted(mapping.items())),
                "context_mapping": dict(sorted(context_mapping.items())),
            },
        )

    request = _allocator_interaction_request(
        focus_symbol=pair.name,
        context=context,
    )
    normalized_request = interaction_planner._parse_request(request)
    observed = {
        tuple(sorted(item["selection"].items()))
        for item in normalized_request["observations"]
    }
    axes = normalized_request["axes"]
    selections = [
        {
            axes[0]["id"]: left["id"],
            axes[1]["id"]: right["id"],
        }
        for left in axes[0]["levels"]
        for right in axes[1]["levels"]
    ]
    missing = [
        dict(sorted(selection.items()))
        for selection in selections
        if tuple(sorted(selection.items())) not in observed
    ]
    owners_by_role = {str(item["lifetime_role"]): item for item in context["owners"]}
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an otherwise exact function contains one complete two-register GPR swap, "
            "authenticated by VarInfo owners and a producer-consumer identity boundary"
        ),
        confidence=0.99,
        source_class="allocator_two_register_swap_factorial_interaction",
        recommendation=(
            "Run the emitted bounded interaction request; compile only missing cells and "
            "do not perform global declaration or register-shaping permutations."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "register_mapping": dict(sorted(mapping.items())),
            "cycle": cycles[0],
            "mismatch_rows": mismatch_rows,
            "owners": owners_by_role,
            "boundary": context["boundary"],
            "proofs": context["proofs"],
            "interaction_request": request,
            "interaction_request_canonical_sha256": _sha256(_canonical(request)),
            "observed_selection_count": len(observed),
            "missing_selections": missing,
            "structural_invariants": [
                "function_size",
                "stack_frame",
                "mnemonic_sequence",
                "branch_relative_targets",
                "relocations",
                "non_register_operands",
                "data_values",
                "protected_siblings",
            ],
        },
    )


def _parameter_allocation_consumer_chain_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "parameter_allocation_consumer_chain"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated parameter/allocation consumer-chain context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the parameter/allocation context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (
        target_size is None
        or target_size != candidate_size
        or target_frame is None
        or target_frame != candidate_frame
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate size/frame are not exact and measurable",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
            },
        )

    rows = causal_reducer._paired_records(target, candidate)
    if not rows or any(
        left is None or right is None or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason=(
                "the residual is not an operation-, CFG-, relocation-, immediate-, "
                "and row-count-identical register-only difference"
            ),
        )
    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_registers = _registers(left.formatted)
        right_registers = _registers(right.formatted)
        if len(left_registers) != len(right_registers):
            return _evaluation(
                rule_id,
                matched=False,
                reason="a register-only row has a different operand count",
            )
        row_mismatch = False
        for target_register, candidate_register in zip(left_registers, right_registers):
            if target_register == candidate_register:
                continue
            if not (_saved(target_register, "r") and _saved(candidate_register, "r")):
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the residual is not confined to nonvolatile GPR ownership",
                )
            if mapping.get(target_register, candidate_register) != candidate_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is inconsistent",
                )
            if reverse.get(candidate_register, target_register) != target_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is not one-to-one",
                )
            mapping[target_register] = candidate_register
            reverse[candidate_register] = target_register
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)
    cycles = _closed_cycles(mapping)
    if len(cycles) != 1 or len(cycles[0]) != 2 or len(mapping) != 2:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the register residual is not one complete two-register swap",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    context_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in context["owners"].values()
    }
    if mapping != context_mapping:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the authenticated parameter/allocation owners do not match the physical swap",
            evidence={
                "physical_mapping": dict(sorted(mapping.items())),
                "context_mapping": dict(sorted(context_mapping.items())),
            },
        )

    producer = context["producer"]
    chain = context["consumer_chain"]
    relevant_rows = {
        producer["call_row"],
        producer["capture_row"],
        *chain["consumer_rows"],
    }
    if not all(index < len(rows) for index in relevant_rows):
        return _evaluation(
            rule_id,
            matched=False,
            reason="a producer/consumer boundary row lies outside the function",
        )
    call_target, call_candidate = rows[producer["call_row"]]
    if (
        call_target is None
        or call_candidate is None
        or call_target.mnemonic != "bl"
        or call_candidate.formatted != call_target.formatted
        or call_target.formatted != f"bl {producer['call_name']}"
        or causal_reducer._relocation_diff(call_target, call_candidate)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed allocation producer call is not physically exact",
        )
    allocation_owner = context["owners"]["allocation_result"]
    capture_target, capture_candidate = rows[producer["capture_row"]]
    expected_target_capture = (
        f"mr {allocation_owner['target_register']}, {producer['return_register']}"
    )
    expected_candidate_capture = (
        f"mr {allocation_owner['candidate_register']}, {producer['return_register']}"
    )
    if (
        capture_target is None
        or capture_candidate is None
        or capture_target.formatted != expected_target_capture
        or capture_candidate.formatted != expected_candidate_capture
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason=(
                "the target does not preserve the producer return in the authenticated "
                "allocation-result identity"
            ),
            evidence={
                "expected_target_capture": expected_target_capture,
                "expected_candidate_capture": expected_candidate_capture,
            },
        )
    if chain["consumer_rows"][0] != producer["capture_row"] + 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed consumers are not immediately adjacent to the producer capture",
        )
    field_target, field_candidate = rows[chain["consumer_rows"][0]]
    copy_target, copy_candidate = rows[chain["consumer_rows"][1]]
    target_alloc = allocation_owner["target_register"]
    candidate_alloc = allocation_owner["candidate_register"]
    if (
        field_target is None
        or field_candidate is None
        or field_target.mnemonic != "stw"
        or field_candidate.mnemonic != "stw"
        or _registers(field_target.formatted, "r")[:1] != [target_alloc]
        or _registers(field_candidate.formatted, "r")[:1] != [candidate_alloc]
        or _without_registers(field_target.formatted)
        != _without_registers(field_candidate.formatted)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the first consumer is not the authenticated allocation-result field store",
        )
    target_copy_registers = (
        _registers(copy_target.formatted, "r") if copy_target is not None else []
    )
    candidate_copy_registers = (
        _registers(copy_candidate.formatted, "r") if copy_candidate is not None else []
    )
    if (
        copy_target is None
        or copy_candidate is None
        or copy_target.mnemonic != "mr"
        or copy_candidate.mnemonic != "mr"
        or len(target_copy_registers) != 2
        or len(candidate_copy_registers) != 2
        or target_copy_registers[1] != target_alloc
        or candidate_copy_registers[1] != candidate_alloc
        or target_copy_registers[0] != candidate_copy_registers[0]
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the second consumer is not the authenticated typed-pointer copy",
        )

    source_expression = (
        f"{chain['typed_pointer']} = {chain['field_owner']}->{chain['field_name']} = "
        f"{chain['allocation_result']}"
    )
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an otherwise exact function has one complete parameter/allocation-result GPR "
            "swap, while the target preserves the producer identity across an adjacent field "
            "store and typed-pointer copy"
        ),
        confidence=0.99,
        source_class="parameter_allocation_result_consumer_chain",
        recommendation=(
            f"Test one natural consumer-chain cell `{source_expression};`; preserve the "
            "explicit allocation-result local, suppress parameter declaration-order cells, "
            "and suppress producer-eliminating fusion."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "register_mapping": dict(sorted(mapping.items())),
            "cycle": cycles[0],
            "mismatch_rows": mismatch_rows,
            "owners": context["owners"],
            "producer": producer,
            "consumer_chain": chain,
            "source_expression": source_expression,
            "physical_boundary": {
                "call_row": producer["call_row"],
                "capture_row": producer["capture_row"],
                "consumer_rows": chain["consumer_rows"],
                "target_capture": capture_target.formatted,
                "candidate_capture": capture_candidate.formatted,
                "target_field_store": field_target.formatted,
                "candidate_field_store": field_candidate.formatted,
                "target_typed_copy": copy_target.formatted,
                "candidate_typed_copy": copy_candidate.formatted,
            },
            "suppressed_axes": [
                {
                    "axis": "parameter_declaration_chronology",
                    "reason": "a function parameter cannot be redeclared to perturb local chronology",
                },
                {
                    "axis": "producer_elimination",
                    "reason": "the target immediately captures r3 into the saved allocation-result owner",
                },
            ],
            "proofs": context["proofs"],
        },
    )


def _frame_size(entries: Sequence[causal_reducer.Instruction]) -> int | None:
    for item in entries[:24]:
        if item.mnemonic not in {"stwu", "stdu"}:
            continue
        offset = _stack_offset(item.formatted)
        if offset is not None and offset < 0:
            return -offset
    return None


def _causal_stack_deltas(audit: Mapping[str, Any]) -> list[int]:
    result: set[int] = set()
    for group in audit.get("causal_groups", []):
        if (
            not isinstance(group, Mapping)
            or group.get("classification") != "stack_home_uniform_delta"
        ):
            continue
        signature = group.get("signature", [])
        if not isinstance(signature, list):
            continue
        for part in signature[1:]:
            # The reducer deliberately uses tuple signatures internally and
            # only converts the outer tuple when building its JSON object.
            if isinstance(part, (list, tuple)):
                result.update(
                    value for value in part if isinstance(value, int) and value != 0
                )
            elif isinstance(part, int) and part != 0:
                result.add(part)
    return sorted(result)


def _preceded_by_call(
    entries: Sequence[causal_reducer.Instruction], index: int
) -> bool:
    return any(
        entries[prior].has_instruction and entries[prior].mnemonic in _CALL_MNEMONICS
        for prior in range(max(0, index - 3), index)
    )


def _aggregate_use_multiplicity_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "aggregate_use_multiplicity"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated aggregate-use multiplicity context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the aggregate-use context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (
        target_size is None
        or target_size != candidate_size
        or target_frame is None
        or target_frame != candidate_frame
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="target and candidate size/frame are not exact and measurable",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
            },
        )

    rows = causal_reducer._paired_records(target, candidate)
    if not rows or any(
        left is None or right is None or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason=(
                "the residual is not operation-, CFG-, relocation-, immediate-, "
                "and row-count-identical register-only evidence"
            ),
        )

    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_registers = _registers(left.formatted)
        right_registers = _registers(right.formatted)
        if len(left_registers) != len(right_registers):
            return _evaluation(
                rule_id,
                matched=False,
                reason="a register-only row has a different operand count",
            )
        row_mismatch = False
        for target_register, candidate_register in zip(left_registers, right_registers):
            if target_register == candidate_register:
                continue
            if not (_saved(target_register, "r") and _saved(candidate_register, "r")):
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the residual is not confined to nonvolatile GPR ownership",
                )
            if mapping.get(target_register, candidate_register) != candidate_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is inconsistent",
                )
            if reverse.get(candidate_register, target_register) != target_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is not one-to-one",
                )
            mapping[target_register] = candidate_register
            reverse[candidate_register] = target_register
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)

    cycles = _closed_cycles(mapping)
    if len(cycles) != 1 or len(cycles[0]) != len(mapping) or len(mapping) < 2:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the residual is not one complete saved-GPR ownership cycle",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    context_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in context["owners"]
    }
    if mapping != context_mapping:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the authenticated aggregate owners do not match the physical cycle",
            evidence={
                "physical_mapping": dict(sorted(mapping.items())),
                "context_mapping": dict(sorted(context_mapping.items())),
            },
        )

    aggregate = context["aggregate_parameter"]
    source_expressions = [
        f"{group['destination']} = *{group['source']}"
        for group in context["copy_groups"]
    ]
    independent_expressions = [
        consumer["expression"] for consumer in context["independent_consumers"]
    ]
    expression_text = "`; `".join(source_expressions)
    preserve_text = (
        " Preserve the independently authenticated consumers unchanged."
        if independent_expressions
        else ""
    )
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an otherwise exact function has one complete saved-GPR ownership cycle, "
            "and the sealed source-use receipt identifies complete member-wise copies "
            "from one live aggregate parameter into real same-type destinations"
        ),
        confidence=0.99,
        source_class="complete_aggregate_copy_use_boundary",
        recommendation=(
            f"Test only the complete aggregate-copy cells `{expression_text};`."
            f"{preserve_text} Suppress input aliases and declaration-order shaping."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "register_mapping": dict(sorted(mapping.items())),
            "cycle": cycles[0],
            "mismatch_rows": mismatch_rows,
            "owners": context["owners"],
            "aggregate_parameter": aggregate,
            "copy_groups": context["copy_groups"],
            "source_expressions": source_expressions,
            "preserved_independent_consumers": context["independent_consumers"],
            "rejected_axes": context["rejected_axes"],
            "suppressed_axes": ["input_pointer_aliases", "parameter_declaration_order"],
            "proofs": context["proofs"],
        },
    )


def _aggregate_two_owner_followup_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "aggregate_two_owner_followup"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated aggregate two-owner follow-up context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the aggregate follow-up context is bound to a different canonical objdiff report",
            evidence={
                "expected_objdiff_canonical_sha256": objdiff_canonical_sha256,
                "context_objdiff_canonical_sha256": context["proofs"][
                    "objdiff_canonical_sha256"
                ],
            },
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (
        target_size is None
        or target_size != candidate_size
        or target_frame is None
        or target_frame != candidate_frame
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the post-aggregate target and candidate size/frame are not exact and measurable",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
            },
        )
    fusion = context["fusion_observation"]
    if fusion["target_size"] != target_size:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the rejected fusion observation is bound to a different target size",
            evidence={
                "report_target_size": target_size,
                "fusion_target_size": fusion["target_size"],
            },
        )

    rows = causal_reducer._paired_records(target, candidate)
    if not rows or any(
        left is None or right is None or not _compatible_register_only_pair(left, right)
        for left, right in rows
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason=(
                "the post-aggregate residual is not operation-, CFG-, relocation-, "
                "immediate-, and row-count-identical register-only evidence"
            ),
        )

    mapping: dict[str, str] = {}
    reverse: dict[str, str] = {}
    mismatch_rows: list[int] = []
    for index, (left, right) in enumerate(rows):
        assert left is not None and right is not None
        left_registers = _registers(left.formatted)
        right_registers = _registers(right.formatted)
        if len(left_registers) != len(right_registers):
            return _evaluation(
                rule_id,
                matched=False,
                reason="a post-aggregate register-only row has a different operand count",
            )
        row_mismatch = False
        for target_register, candidate_register in zip(left_registers, right_registers):
            if target_register == candidate_register:
                continue
            if not (_saved(target_register, "r") and _saved(candidate_register, "r")):
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the post-aggregate residual is not confined to nonvolatile GPR ownership",
                )
            if mapping.get(target_register, candidate_register) != candidate_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is inconsistent",
                )
            if reverse.get(candidate_register, target_register) != target_register:
                return _evaluation(
                    rule_id,
                    matched=False,
                    reason="the target-to-candidate register mapping is not one-to-one",
                )
            mapping[target_register] = candidate_register
            reverse[candidate_register] = target_register
            row_mismatch = True
        if row_mismatch:
            mismatch_rows.append(index)

    cycles = _closed_cycles(mapping)
    if len(cycles) != 1 or len(cycles[0]) != 2 or len(mapping) != 2:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the post-aggregate residual is not one complete two-register swap",
            evidence={
                "register_mapping": dict(sorted(mapping.items())),
                "cycles": cycles,
                "mismatch_rows": mismatch_rows,
            },
        )
    context_mapping = {
        str(owner["target_register"]): str(owner["candidate_register"])
        for owner in context["owners"]
    }
    if mapping != context_mapping:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the authenticated typed owners do not match the physical two-register swap",
            evidence={
                "physical_mapping": dict(sorted(mapping.items())),
                "context_mapping": dict(sorted(context_mapping.items())),
            },
        )

    order = context["declaration_axis"]["recommended_order"]
    recommended_cell = {
        "declaration_chronology": list(order),
        "expression_topology": "split",
    }
    suppressed_cells = [
        {
            "declaration_chronology": "existing",
            "expression_topology": "fused",
            "reason": "measured topology change and strict regression",
        },
        {
            "declaration_chronology": list(order),
            "expression_topology": "fused",
            "reason": "do not combine a rejected topology-changing axis",
        },
    ]
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "after an authenticated aggregate reconstruction, the otherwise exact "
            "function has one complete typed two-owner GPR swap and a separately "
            "measured expression-fusion axis changes topology and regresses strictness"
        ),
        confidence=0.99,
        source_class="post_aggregate_typed_owner_declaration_chronology",
        recommendation=(
            f"Keep `{context['aggregate_boundary']['expression']}` and the split "
            f"producer/consumer statements. Compile only the declaration-order cell "
            f"{order[0]} before {order[1]}; do not combine it with expression fusion."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "register_mapping": dict(sorted(mapping.items())),
            "cycle": cycles[0],
            "mismatch_rows": mismatch_rows,
            "owners": context["owners"],
            "aggregate_boundary": context["aggregate_boundary"],
            "declaration_axis": context["declaration_axis"],
            "fusion_observation": fusion,
            "recommended_cells": [recommended_cell],
            "suppressed_cells": suppressed_cells,
            "proofs": context["proofs"],
        },
    )


def _address_taken_local_pointer_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "address_taken_local_pointer_consumer"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated address-taken local pointer context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the address-taken context is bound to a different canonical objdiff report",
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if (
        target_size is None
        or candidate_size is None
        or target_size - candidate_size != context["expected_size_delta"]
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the measured function-size delta does not match the sealed pointer-lifetime seam",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "expected_size_delta": context["expected_size_delta"],
            },
        )
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (
        target_frame is None
        or candidate_frame is None
        or target_frame <= candidate_frame
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target does not have the larger measurable frame required by the named local pointer lifetime",
            evidence={
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
            },
        )

    aggregate_offset = int(context["aggregate"]["stack_offset"])
    incoming = context["incoming_pointer"]
    local = context["local_pointer"]
    object_home = context["object_home"]
    consumer = str(local["consumer"])

    def call_rows(entries: Sequence[causal_reducer.Instruction]) -> list[int]:
        pattern = re.compile(rf"\b{re.escape(consumer)}\b")
        return [
            index
            for index, item in enumerate(entries)
            if item.has_instruction
            and item.mnemonic in _CALL_MNEMONICS
            and pattern.search(item.formatted) is not None
        ]

    target_calls = call_rows(target)
    candidate_calls = call_rows(candidate)
    if len(target_calls) != 1 or len(candidate_calls) != 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the typed consumer call is not uniquely present on both report sides",
            evidence={
                "consumer": consumer,
                "target_call_rows": target_calls,
                "candidate_call_rows": candidate_calls,
            },
        )

    target_materializations = [
        (index, materialization)
        for index, item in enumerate(target)
        if item.has_instruction
        and (materialization := _addi_r1_materialization(item.formatted)) is not None
        and materialization
        == (str(local["target_register"]), aggregate_offset)
    ]
    candidate_materializations = [
        (index, materialization)
        for index, item in enumerate(candidate)
        if item.has_instruction
        and (materialization := _addi_r1_materialization(item.formatted)) is not None
        and materialization
        == (str(local["argument_register"]), aggregate_offset)
    ]
    if len(target_materializations) != 1 or len(candidate_materializations) != 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the saved target address and direct candidate argument materializations are not unique and exact",
            evidence={
                "aggregate_stack_offset": aggregate_offset,
                "target_materializations": target_materializations,
                "candidate_materializations": candidate_materializations,
            },
        )

    target_materialization_row = target_materializations[0][0]
    candidate_materialization_row = candidate_materializations[0][0]
    target_call_row = target_calls[0]
    candidate_call_row = candidate_calls[0]
    target_copy_rows = [
        index
        for index, item in enumerate(target)
        if target_materialization_row < index < target_call_row
        and item.mnemonic == "mr"
        and _registers(item.formatted)
        == [str(local["argument_register"]), str(local["target_register"])]
    ]
    if (
        len(target_copy_rows) != 1
        or not 0 < target_call_row - target_materialization_row <= 12
        or not 0 < candidate_call_row - candidate_materialization_row <= 12
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target saved-pointer copy or bounded call chronology is absent",
            evidence={
                "target_materialization_row": target_materialization_row,
                "target_copy_rows": target_copy_rows,
                "target_call_row": target_call_row,
                "candidate_materialization_row": candidate_materialization_row,
                "candidate_call_row": candidate_call_row,
            },
        )

    target_incoming_rows = [
        index
        for index, item in enumerate(target)
        if item.mnemonic == "mr"
        and _registers(item.formatted)
        == [str(incoming["target_register"]), str(local["argument_register"])]
    ]
    candidate_incoming_rows = [
        index
        for index, item in enumerate(candidate)
        if item.mnemonic == "mr"
        and _registers(item.formatted)
        == [str(incoming["candidate_register"]), str(local["argument_register"])]
    ]
    if len(target_incoming_rows) != 1 or len(candidate_incoming_rows) != 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the incoming aggregate-pointer owner colors are not uniquely authenticated",
            evidence={
                "target_incoming_rows": target_incoming_rows,
                "candidate_incoming_rows": candidate_incoming_rows,
            },
        )

    home_offset = int(object_home["target_stack_offset"])
    target_home_rows = [
        index
        for index, item in enumerate(target[:32])
        if item.mnemonic == "stw"
        and _registers(item.formatted)[:1] == ["r3"]
        and _stack_offset(item.formatted) == home_offset
    ]
    candidate_home_rows = [
        index
        for index, item in enumerate(candidate[:32])
        if item.mnemonic == "stw"
        and _registers(item.formatted)[:1] == ["r3"]
        and _stack_offset(item.formatted) == home_offset
    ]
    if len(target_home_rows) != 1 or candidate_home_rows:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target-only parameter home is not uniquely present",
            evidence={
                "target_home_rows": target_home_rows,
                "candidate_home_rows": candidate_home_rows,
                "target_stack_offset": home_offset,
            },
        )

    expression = (
        f"{local['name']} = &{context['aggregate']['name']}; "
        f"pass {local['name']} to {consumer}"
    )
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "the target uniquely materializes an address-taken local aggregate in a "
            "saved GPR, copies that owner into the typed call argument, moves the "
            "incoming pointer to a different saved owner, and adds one target-only "
            "parameter home while the candidate passes the local address directly"
        ),
        confidence=0.99,
        source_class="live_typed_pointer_to_address_taken_local_at_consumer_boundary",
        recommendation=(
            f"Test exactly one live `{context['aggregate']['type']} *{local['name']}` "
            f"bound to `&{context['aggregate']['name']}` immediately before {consumer}; "
            "suppress declaration-only and artificial-lifetime permutations."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "size_delta": target_size - candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "aggregate": context["aggregate"],
            "incoming_pointer": incoming,
            "local_pointer": local,
            "object_home": object_home,
            "target_home_row": target_home_rows[0],
            "target_incoming_row": target_incoming_rows[0],
            "candidate_incoming_row": candidate_incoming_rows[0],
            "target_materialization_row": target_materialization_row,
            "target_copy_row": target_copy_rows[0],
            "candidate_direct_materialization_row": candidate_materialization_row,
            "target_call_row": target_call_row,
            "candidate_call_row": candidate_call_row,
            "source_expression": expression,
            "suppressed_axes": [
                "declaration_order_only",
                "dead_pointer_storage",
                "artificial_lifetime_extension",
            ],
            "proofs": context["proofs"],
        },
    )


def _same_tu_exact_sibling_shape_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "same_tu_exact_sibling_source_shapes"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated same-TU exact-sibling source-shape context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the same-TU source-shape context is bound to a different canonical objdiff report",
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    cell = context["combined_cell"]
    if (
        target_size != cell["target_size"]
        or candidate_size is None
        or candidate_size >= target_size
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the baseline size does not have the sealed smaller-candidate shape",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "expected_target_size": cell["target_size"],
            },
        )

    rows = causal_reducer._paired_records(target, candidate)

    def row(index: int) -> tuple[causal_reducer.Instruction | None, causal_reducer.Instruction | None] | None:
        if not 0 <= index < len(rows):
            return None
        return rows[index]

    tail = context["fixed_array_tail"]
    expected_tail_mnemonics = ["li", "srawi", "srwi", "subfc", "adde"]
    tail_evidence: list[dict[str, Any]] = []
    for index, expected_mnemonic in zip(
        tail["target_rows"], expected_tail_mnemonics
    ):
        pair_row = row(index)
        if pair_row is None:
            return _evaluation(
                rule_id,
                matched=False,
                reason="a sealed fixed-array tail row is outside the focus function",
            )
        left, right = pair_row
        if (
            left is None
            or not left.has_instruction
            or left.mnemonic != expected_mnemonic
            or (right is not None and right.has_instruction)
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the target-only fixed-array Boolean lowering is not li/srawi/srwi/subfc/adde",
                evidence={"row_index": index, "expected_mnemonic": expected_mnemonic},
            )
        tail_evidence.append(
            {"row_index": index, "target_formatted": left.formatted}
        )

    if tail["source_expression"] != context["donor"]["source_expression"]:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the focus tail expression does not equal the authenticated same-TU donor expression",
        )

    abi = context["abi_boundary"]
    normalization_pair = row(abi["candidate_normalization_row"])
    store_pair = row(abi["store_row"])
    if normalization_pair is None or store_pair is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed ABI normalization rows are outside the focus function",
        )
    target_normalization, candidate_normalization = normalization_pair
    target_store, candidate_store = store_pair
    candidate_normalization_registers = (
        _registers(candidate_normalization.formatted, "r")
        if candidate_normalization is not None and candidate_normalization.has_instruction
        else []
    )
    if (
        (target_normalization is not None and target_normalization.has_instruction)
        or candidate_normalization is None
        or candidate_normalization.mnemonic != "extsh"
        or len(candidate_normalization_registers) != 2
        or candidate_normalization_registers[1] != abi["parameter_register"]
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the candidate-only callee normalization is not one extsh from the sealed argument register",
        )
    normalized_register = candidate_normalization_registers[0]
    if (
        target_store is None
        or candidate_store is None
        or not target_store.has_instruction
        or not candidate_store.has_instruction
        or target_store.mnemonic != "stw"
        or candidate_store.mnemonic != "stw"
        or _memory_operand(target_store.formatted)
        != _memory_operand(candidate_store.formatted)
        or _registers(target_store.formatted, "r")[:1] != [abi["parameter_register"]]
        or _registers(candidate_store.formatted, "r")[:1] != [normalized_register]
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target direct parameter store and candidate normalized store do not share one physical consumer",
        )

    zero = context["zero_chain"]
    target_load_pair = row(zero["target_load_row"])
    if target_load_pair is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the sealed zero-chain target load row is outside the focus function",
        )
    target_load = target_load_pair[0]
    if (
        target_load is None
        or not target_load.has_instruction
        or target_load.mnemonic != "lfs"
        or target_load.relocation is None
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the zero chain does not begin with one typed target f32 load",
        )
    target_load_registers = _registers(target_load.formatted, "f")
    if len(target_load_registers) != 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target zero load does not define one FPR",
        )
    target_store_items: list[causal_reducer.Instruction] = []
    for index in zero["target_store_rows"]:
        pair_row = row(index)
        item = pair_row[0] if pair_row is not None else None
        if (
            item is None
            or not item.has_instruction
            or item.mnemonic != "stfs"
            or _registers(item.formatted, "f")[:1] != target_load_registers
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the target zero stores do not all consume the single loaded FPR",
                evidence={"row_index": index},
            )
        target_store_items.append(item)

    candidate_load_items: list[causal_reducer.Instruction] = []
    for index in zero["candidate_load_rows"]:
        pair_row = row(index)
        item = pair_row[1] if pair_row is not None else None
        if (
            item is None
            or not item.has_instruction
            or item.mnemonic != "lfs"
            or item.relocation is None
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the candidate zero source is not three typed f32 loads",
                evidence={"row_index": index},
            )
        candidate_load_items.append(item)
    candidate_store_items: list[causal_reducer.Instruction] = []
    for index in zero["candidate_store_rows"]:
        pair_row = row(index)
        item = pair_row[1] if pair_row is not None else None
        if item is None or not item.has_instruction or item.mnemonic != "stfs":
            return _evaluation(
                rule_id,
                matched=False,
                reason="the candidate zero source is not three separate stores",
                evidence={"row_index": index},
            )
        candidate_store_items.append(item)
    for load_item, store_item in zip(candidate_load_items, candidate_store_items):
        if _registers(load_item.formatted, "f")[:1] != _registers(
            store_item.formatted, "f"
        )[:1]:
            return _evaluation(
                rule_id,
                matched=False,
                reason="a candidate zero store does not consume its corresponding loaded FPR",
            )

    target_operands = [_memory_operand(item.formatted) for item in target_store_items]
    candidate_operands = [
        _memory_operand(item.formatted) for item in candidate_store_items
    ]
    target_offsets = [item[1] if item is not None else None for item in target_operands]
    candidate_offsets = [
        item[1] if item is not None else None for item in candidate_operands
    ]
    if (
        any(offset is None for offset in target_offsets + candidate_offsets)
        or len({item[0] for item in target_operands if item is not None}) != 1
        or len({item[0] for item in candidate_operands if item is not None}) != 1
        or target_operands[0][0] != candidate_operands[0][0]
        or len(set(target_offsets)) != 3
        or target_offsets != list(reversed(candidate_offsets))
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target does not reverse the same three candidate field homes with one shared zero value",
            evidence={
                "target_store_offsets": target_offsets,
                "candidate_store_offsets": candidate_offsets,
            },
        )

    learning_rows = set(tail["target_rows"]) | {
        abi["candidate_normalization_row"],
        abi["store_row"],
        zero["target_load_row"],
        *zero["target_store_rows"],
        *zero["candidate_load_rows"],
        *zero["candidate_store_rows"],
    }
    outside_residuals = [
        index
        for index, (left, right) in enumerate(rows)
        if index not in learning_rows
        and not _equivalent_outside_learning_window(left, right)
    ]
    if outside_residuals:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the report has physical residuals outside the three sealed source-shape groups",
            evidence={"outside_residual_rows": outside_residuals},
        )

    scheduled_cell = {
        "id": cell["candidate_id"],
        "source_actions": [
            tail["source_expression"],
            f"declare callee parameter {abi['parameter']} as {abi['callee_type']} while preserving the {abi['producer_type']} producer boundary",
            zero["source_expression"],
        ],
        "expected_target_size": cell["target_size"],
        "expected_candidate_size": cell["candidate_size"],
        "expected_object_sha256": cell["object_sha256"],
        "candidate_record_sha256": cell["candidate_record_sha256"],
    }
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "three nonoverlapping physical groups match an exact same-TU fixed-array "
            "tail, a caller-authenticated narrow-producer/wide-callee boundary, and "
            "one right-associative aggregate zero chain"
        ),
        confidence=0.99,
        source_class="same_tu_exact_sibling_and_consumer_contract_combined_cell",
        recommendation=(
            "Compile only the emitted combined natural-C cell; do not schedule fresh "
            "declaration, ABI, literal-load, or Boolean-CFG permutations."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "donor": context["donor"],
            "fixed_array_tail": {
                **tail,
                "target_instructions": tail_evidence,
            },
            "abi_boundary": {
                **abi,
                "normalized_register": normalized_register,
                "target_store_formatted": target_store.formatted,
                "candidate_store_formatted": candidate_store.formatted,
            },
            "zero_chain": {
                **zero,
                "target_store_offsets": target_offsets,
                "candidate_store_offsets": candidate_offsets,
            },
            "scheduled_cells": [scheduled_cell],
            "suppressed_axes": [
                "declaration_order_permutations",
                "independent_boolean_materialization",
                "guessed_prototype_changes",
                "separate_zero_literal_loads",
            ],
            "proofs": context["proofs"],
        },
    )


def _short_circuit_boolean_call_order_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "short_circuit_boolean_call_order"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated short-circuit Boolean call-order context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the short-circuit context is bound to a different canonical objdiff report",
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    observation = context["topology_observation"]
    if (
        target_size != observation["target_size"]
        or candidate_size is None
        or candidate_size <= target_size
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the baseline does not have the sealed larger-candidate Boolean topology",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "expected_target_size": observation["target_size"],
            },
        )

    rows = causal_reducer._paired_records(target, candidate)

    def side(index: int, which: int) -> causal_reducer.Instruction | None:
        if not 0 <= index < len(rows):
            return None
        return rows[index][which]

    call_evidence: list[dict[str, Any]] = []
    for index, test in enumerate(context["mask_tests"]):
        branch_call = side(test["target_branch_call_row"], 0)
        masu_call = side(test["target_masu_call_row"], 0)
        branch_pattern = re.compile(rf"\b{re.escape(test['branch_getter'])}\b")
        masu_pattern = re.compile(rf"\b{re.escape(test['masu_getter'])}\b")
        if (
            branch_call is None
            or masu_call is None
            or branch_call.mnemonic not in _CALL_MNEMONICS
            or masu_call.mnemonic not in _CALL_MNEMONICS
            or branch_pattern.search(branch_call.formatted) is None
            or masu_pattern.search(masu_call.formatted) is None
            or test["target_branch_call_row"] >= test["target_masu_call_row"]
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="a target mask test does not call the branch getter before the masu getter",
                evidence={"mask_test_index": index},
            )
        if (
            test["masu_getter"] not in test["source_left"]
            or test["branch_getter"] not in test["source_right"]
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the sealed source order does not write masu getter before branch getter for MWCC right-to-left evaluation",
                evidence={"mask_test_index": index},
            )
        call_evidence.append(
            {
                "source_expression": test["source_expression"],
                "written_left": test["source_left"],
                "written_right": test["source_right"],
                "target_call_order": [
                    {
                        "row": test["target_branch_call_row"],
                        "callee": test["branch_getter"],
                    },
                    {
                        "row": test["target_masu_call_row"],
                        "callee": test["masu_getter"],
                    },
                ],
            }
        )

    shared = context["shared_boolean"]
    first_branch = side(shared["target_branch_rows"][0], 0)
    second_branch = side(shared["target_branch_rows"][1], 0)
    true_assignment = side(shared["target_true_assignment_row"], 0)
    false_assignment = side(shared["target_false_assignment_row"], 0)
    if any(
        item is None or not item.has_instruction
        for item in (first_branch, second_branch, true_assignment, false_assignment)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target shared-Boolean rows are incomplete",
        )
    assert first_branch is not None
    assert second_branch is not None
    assert true_assignment is not None
    assert false_assignment is not None
    result_register = shared["result_register"]
    if (
        first_branch.mnemonic != "bne"
        or second_branch.mnemonic != "beq"
        or first_branch.branch_dest != true_assignment.address
        or second_branch.branch_dest != false_assignment.address
        or true_assignment.mnemonic != "li"
        or false_assignment.mnemonic != "li"
        or _registers(true_assignment.formatted, "r")[:1] != [result_register]
        or _registers(false_assignment.formatted, "r")[:1] != [result_register]
        or not true_assignment.formatted.rstrip().endswith(", 1")
        or not false_assignment.formatted.rstrip().endswith(", 0")
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the target branches do not converge on one shared true/false assignment pair",
            evidence={
                "first_branch": first_branch.formatted,
                "second_branch": second_branch.formatted,
                "first_destination": first_branch.branch_dest,
                "second_destination": second_branch.branch_dest,
                "true_address": true_assignment.address,
                "false_address": false_assignment.address,
            },
        )

    candidate_true: list[dict[str, Any]] = []
    for index in shared["candidate_true_assignment_rows"]:
        item = side(index, 1)
        if (
            item is None
            or not item.has_instruction
            or item.mnemonic != "li"
            or _registers(item.formatted, "r")[:1] != [result_register]
            or not item.formatted.rstrip().endswith(", 1")
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the candidate does not duplicate the true assignment at the sealed rows",
                evidence={"row_index": index},
            )
        candidate_true.append({"row": index, "formatted": item.formatted})
    candidate_false = side(shared["candidate_false_assignment_row"], 1)
    if (
        candidate_false is None
        or not candidate_false.has_instruction
        or candidate_false.mnemonic != "li"
        or _registers(candidate_false.formatted, "r")[:1] != [result_register]
        or not candidate_false.formatted.rstrip().endswith(", 0")
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the candidate false assignment is absent at the sealed row",
        )

    mapping = {
        owner["target_register"]: owner["candidate_register"]
        for owner in observation["owners"]
    }
    cycles = _closed_cycles(mapping)
    if len(cycles) != 1 or len(cycles[0]) != 4:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the exact-topology observation is not one closed four-owner GPR cycle",
            evidence={"register_mapping": dict(sorted(mapping.items())), "cycles": cycles},
        )

    boolean_owner = next(
        owner["name"] for owner in observation["owners"] if owner["type"] == "BOOL"
    )
    expression = (
        f"if (({context['mask_tests'][0]['source_expression']}) || "
        f"({context['mask_tests'][1]['source_expression']})) "
        f"{{ {boolean_owner} = TRUE; }} else {{ {boolean_owner} = FALSE; }}"
    )
    scheduled_cells = [
        {
            "id": "explicit-if-else-right-to-left-call-order",
            "source_class": "shared_true_false_short_circuit_if_else",
            "source_expression": expression,
            "suppressed_alternative": context["direct_assignment_rejection"],
        },
        {
            "id": observation["candidate_id"],
            "requires_previous_cell": "explicit-if-else-right-to-left-call-order",
            "source_class": "typed_four_owner_declaration_chronology_after_exact_topology",
            "declaration_order": observation["recommended_declaration_order"],
            "candidate_record_sha256": observation["candidate_record_sha256"],
        },
    ]
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "the target calls each branch getter before its masu getter, routes bne/beq "
            "to one shared true/false pair, and the baseline duplicates the true arm; a "
            "measured exact-topology precursor then leaves one sealed four-owner cycle"
        ),
        confidence=0.99,
        source_class="short_circuit_shared_boolean_then_typed_owner_chronology",
        recommendation=(
            "Compile the explicit if/else with source-commuted AND operands first; only "
            "after topology is exact, compile the one sealed declaration-order cell."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "mask_tests": call_evidence,
            "shared_boolean": {
                **shared,
                "first_branch_destination": first_branch.branch_dest,
                "second_branch_destination": second_branch.branch_dest,
                "candidate_true_assignments": candidate_true,
                "candidate_false_assignment": candidate_false.formatted,
            },
            "topology_observation": observation,
            "register_mapping": dict(sorted(mapping.items())),
            "register_cycle": cycles[0],
            "scheduled_cells": scheduled_cells,
            "suppressed_axes": [
                "direct_boolean_assignment",
                "call_order_guessing",
                "declaration_permutations_before_topology",
                "dead_boolean_temporaries",
            ],
            "proofs": context["proofs"],
        },
    )


def _dependency_equivalent_exact_sibling_transfer_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    context: Mapping[str, Any] | None,
    objdiff_canonical_sha256: str,
) -> dict[str, Any]:
    rule_id = "dependency_equivalent_exact_sibling_transfer"
    if context is None:
        return _evaluation(
            rule_id,
            matched=False,
            reason="no authenticated dependency-equivalent exact-sibling context was supplied",
        )
    if context["proofs"]["objdiff_canonical_sha256"] != objdiff_canonical_sha256:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the exact-sibling context is bound to a different canonical objdiff report",
        )

    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    combined = context["combined_cell"]
    if (
        target_size != combined["target_size"]
        or candidate_size is None
        or candidate_size <= target_size
        or combined["candidate_size"] != target_size
        or context["donor"]["symbol"] == pair.name
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the baseline size or distinct exact-donor boundary is not the sealed transfer case",
            evidence={
                "target_size": target_size,
                "candidate_size": candidate_size,
                "combined_size": combined["candidate_size"],
                "donor_symbol": context["donor"]["symbol"],
            },
        )

    rows = causal_reducer._paired_records(target, candidate)

    def side(index: int, which: int) -> causal_reducer.Instruction | None:
        if not 0 <= index < len(rows):
            return None
        return rows[index][which]

    call_evidence: list[dict[str, Any]] = []
    for index, test in enumerate(context["baseline"]["mask_tests"]):
        target_branch = side(test["target_branch_call_row"], 0)
        target_masu = side(test["target_masu_call_row"], 0)
        candidate_masu = side(test["candidate_masu_call_row"], 1)
        candidate_branch = side(test["candidate_branch_call_row"], 1)

        def is_call(item: causal_reducer.Instruction | None, symbol: str) -> bool:
            return bool(
                item is not None
                and item.has_instruction
                and item.mnemonic in _CALL_MNEMONICS
                and re.search(rf"\b{re.escape(symbol)}\b", item.formatted)
            )

        if not (
            is_call(target_branch, test["branch_getter"])
            and is_call(target_masu, test["masu_getter"])
            and is_call(candidate_masu, test["masu_getter"])
            and is_call(candidate_branch, test["branch_getter"])
            and test["masu_getter"] in test["source_left"]
            and test["branch_getter"] in test["source_right"]
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the sibling transfer does not reproduce the sealed target/candidate call-order inversion",
                evidence={"mask_test_index": index},
            )
        call_evidence.append(
            {
                "source_expression": test["source_expression"],
                "target_call_order": [
                    test["branch_getter"],
                    test["masu_getter"],
                ],
                "candidate_call_order": [
                    test["masu_getter"],
                    test["branch_getter"],
                ],
            }
        )

    shared = context["baseline"]["shared_boolean"]
    first_branch = side(shared["target_branch_rows"][0], 0)
    second_branch = side(shared["target_branch_rows"][1], 0)
    true_assignment = side(shared["target_true_assignment_row"], 0)
    false_assignment = side(shared["target_false_assignment_row"], 0)
    if any(
        item is None or not item.has_instruction
        for item in (first_branch, second_branch, true_assignment, false_assignment)
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the transferred target shared-Boolean rows are incomplete",
        )
    assert first_branch is not None
    assert second_branch is not None
    assert true_assignment is not None
    assert false_assignment is not None
    result_register = shared["result_register"]
    if (
        first_branch.mnemonic != "bne"
        or second_branch.mnemonic != "beq"
        or first_branch.branch_dest != true_assignment.address
        or second_branch.branch_dest != false_assignment.address
        or true_assignment.mnemonic != "li"
        or false_assignment.mnemonic != "li"
        or _registers(true_assignment.formatted, "r")[:1] != [result_register]
        or _registers(false_assignment.formatted, "r")[:1] != [result_register]
        or not true_assignment.formatted.rstrip().endswith(", 1")
        or not false_assignment.formatted.rstrip().endswith(", 0")
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the transferred target topology does not converge on one shared Boolean pair",
        )
    for row_index in shared["candidate_true_assignment_rows"]:
        item = side(row_index, 1)
        if (
            item is None
            or not item.has_instruction
            or item.mnemonic != "li"
            or _registers(item.formatted, "r")[:1] != [result_register]
            or not item.formatted.rstrip().endswith(", 1")
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the baseline does not contain both duplicated candidate true assignments",
                evidence={"row_index": row_index},
            )
    candidate_false = side(shared["candidate_false_assignment_row"], 1)
    if (
        candidate_false is None
        or not candidate_false.has_instruction
        or candidate_false.mnemonic != "li"
        or _registers(candidate_false.formatted, "r")[:1] != [result_register]
        or not candidate_false.formatted.rstrip().endswith(", 0")
    ):
        return _evaluation(
            rule_id,
            matched=False,
            reason="the baseline candidate false assignment is absent",
        )

    boundary = context["type_boundary"]
    extsh_evidence: list[dict[str, Any]] = []
    source_registers: set[str] = set()
    for index, (extsh_row, call_row, consumer) in enumerate(
        zip(
            boundary["target_extsh_rows"],
            boundary["target_consumer_call_rows"],
            boundary["consumer_symbols"],
        )
    ):
        target_extsh = side(extsh_row, 0)
        candidate_extsh = side(extsh_row, 1)
        target_consumer = side(call_row, 0)
        registers = (
            _registers(target_extsh.formatted, "r")
            if target_extsh is not None and target_extsh.has_instruction
            else []
        )
        if (
            target_extsh is None
            or not target_extsh.has_instruction
            or target_extsh.mnemonic != "extsh"
            or len(registers) < 2
            or candidate_extsh is None
            or candidate_extsh.has_instruction
            or call_row != extsh_row + 1
            or target_consumer is None
            or not target_consumer.has_instruction
            or target_consumer.mnemonic not in _CALL_MNEMONICS
            or re.search(rf"\b{re.escape(consumer)}\b", target_consumer.formatted)
            is None
        ):
            return _evaluation(
                rule_id,
                matched=False,
                reason="the int-to-s16 boundary is not three target-only adjacent extsh/call pairs",
                evidence={"boundary_index": index},
            )
        source_registers.add(registers[1])
        extsh_evidence.append(
            {
                "extsh_row": extsh_row,
                "call_row": call_row,
                "consumer": consumer,
                "formatted": target_extsh.formatted,
            }
        )
    if len(source_registers) != 1:
        return _evaluation(
            rule_id,
            matched=False,
            reason="the three target extsh rows do not normalize one source owner",
            evidence={"source_registers": sorted(source_registers)},
        )

    capacity = context["capacity"]
    boolean_expression = (
        f"if (({context['baseline']['mask_tests'][0]['source_expression']}) || "
        f"({context['baseline']['mask_tests'][1]['source_expression']})) "
        f"{{ {shared['result_owner']} = TRUE; }} else "
        f"{{ {shared['result_owner']} = FALSE; }}"
    )
    scheduled_cell = {
        "id": combined["candidate_id"],
        "source_class": "dependency_equivalent_exact_sibling_plus_proved_type_boundary",
        "donor_symbol": context["donor"]["symbol"],
        "transferred_expression": boolean_expression,
        "type_declaration": f"int {boundary['owner']}",
        "capacity_declaration": (
            f"s16 {capacity['array_name']}[{capacity['macro']}]"
        ),
        "expected_object_sha256": combined["object_sha256"],
        "candidate_record_sha256": combined["candidate_record_sha256"],
    }
    return _evaluation(
        rule_id,
        matched=True,
        reason=(
            "an exact dependency-equivalent sibling supplies the complete shared-Boolean "
            "transformation, while three target-only extsh/call pairs independently prove "
            "the remaining int-to-s16 owner boundary"
        ),
        confidence=0.99,
        source_class="exact_sibling_semantic_transfer_with_independent_type_boundary",
        recommendation=(
            "Transfer the exact sibling Boolean/call-order source, apply only the sealed "
            "int owner boundary and authenticated array capacity, then compile one cell."
        ),
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "donor": context["donor"],
            "call_order": call_evidence,
            "shared_boolean": shared,
            "type_boundary": {
                **boundary,
                "source_register": next(iter(source_registers)),
                "extsh_calls": extsh_evidence,
            },
            "capacity": capacity,
            "scheduled_cells": [scheduled_cell],
            "suppressed_axes": [
                "fresh_boolean_cfg_permutations",
                "declaration_order_permutations",
                "s16_link_owner",
                "capacity_guessing",
            ],
            "proofs": context["proofs"],
        },
    )


def _switch_fpr_evaluation(
    pair: causal_reducer.FunctionPair,
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    audit: Mapping[str, Any],
) -> dict[str, Any]:
    target_frame = _frame_size(target)
    candidate_frame = _frame_size(candidate)
    if (
        target_frame is None
        or candidate_frame is None
        or target_frame <= candidate_frame
    ):
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the target does not have a larger measurable stack frame",
        )
    frame_delta = target_frame - candidate_frame
    stack_deltas = _causal_stack_deltas(audit)
    if frame_delta > 256 or not any(
        abs(value) == frame_delta for value in stack_deltas
    ):
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the causal reducer did not corroborate the prologue frame delta with a uniform stack-home delta",
            evidence={
                "target_frame": target_frame,
                "candidate_frame": candidate_frame,
                "frame_delta": frame_delta,
                "causal_stack_deltas": stack_deltas,
            },
        )
    if not any(item.mnemonic in _SWITCH_MNEMONICS for item in target):
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="the focus has no indirect switch dispatch instruction",
            evidence={"frame_delta": frame_delta, "causal_stack_deltas": stack_deltas},
        )

    captures: list[dict[str, Any]] = []
    rows = causal_reducer._paired_records(target, candidate)
    for index, (left, right) in enumerate(rows):
        if (
            left is None
            or left.mnemonic != "fmr"
            or not _preceded_by_call(target, index)
        ):
            continue
        registers = _registers(left.formatted, "f")
        if len(registers) < 2 or registers[1] != "f1" or not _saved(registers[0], "f"):
            continue
        candidate_registers = (
            _registers(right.formatted, "f") if right is not None else []
        )
        if (
            right is not None
            and right.has_instruction
            and right.mnemonic == "fmr"
            and candidate_registers == registers
        ):
            continue
        captures.append(
            {
                "index": index,
                "target_result_register": registers[0],
                "candidate_mnemonic": (
                    right.mnemonic
                    if right is not None and right.has_instruction
                    else None
                ),
            }
        )
    if len(captures) < 3:
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="fewer than three target-only nonvolatile FPR call-result lifetimes are present",
            evidence={
                "frame_delta": frame_delta,
                "causal_stack_deltas": stack_deltas,
                "result_captures": captures,
            },
        )
    target_size = _function_size(pair.target)
    candidate_size = _function_size(pair.candidate)
    if target_size is None or candidate_size is None or target_size <= candidate_size:
        return _evaluation(
            "switch_case_scoped_fpr_lifetimes",
            matched=False,
            reason="target-only FPR lifetimes are not accompanied by a larger target function",
        )
    return _evaluation(
        "switch_case_scoped_fpr_lifetimes",
        matched=True,
        reason="switch dispatch, a corroborated frame delta, and multiple target-only nonvolatile FPR result captures occur together",
        confidence=0.97,
        source_class="switch_case_scoped_used_result_locals",
        recommendation="Test used floating-point call-result locals scoped to the individual switch cases that consume them.",
        evidence={
            "target_size": target_size,
            "candidate_size": candidate_size,
            "target_frame": target_frame,
            "candidate_frame": candidate_frame,
            "frame_delta": frame_delta,
            "causal_stack_deltas": stack_deltas,
            "switch_mnemonics": sorted(
                {item.mnemonic for item in target if item.mnemonic in _SWITCH_MNEMONICS}
            ),
            "result_captures": captures,
        },
    )


def _copy_run(
    entries: Sequence[causal_reducer.Instruction],
    *,
    corresponding: Sequence[causal_reducer.Instruction] | None = None,
    require_asymmetry: bool,
) -> dict[str, Any] | None:
    # "Final" is established by the absence of later calls, not by a
    # percentage of function length.  Keep the physical search bounded to the
    # last 64 aligned rows so short functions are not treated differently.
    start_floor = max(0, len(entries) - 64)
    for start in range(start_floor, len(entries)):
        for end in range(start + 4, min(len(entries), start + 10) + 1):
            window = entries[start:end]
            if any(not item.has_instruction for item in window):
                continue
            offsets = [_stack_offset(item.formatted) for item in window]
            if any(offset is None for offset in offsets):
                continue
            loads = [
                offset
                for item, offset in zip(window, offsets)
                if item.mnemonic in _AGGREGATE_LOADS
            ]
            stores = [
                offset
                for item, offset in zip(window, offsets)
                if item.mnemonic in _AGGREGATE_STORES
            ]
            if (
                len(loads) < 3
                or len(loads) != len(stores)
                or sorted(loads) != sorted(stores)
            ):
                continue
            if len(loads) + len(stores) != len(window):
                continue
            if require_asymmetry:
                assert corresponding is not None
                other = corresponding[start:end]
                if len(other) != len(window) or any(
                    item.has_instruction for item in other
                ):
                    continue
            consumers = [
                {
                    "index": index,
                    "formatted": entries[index].formatted,
                }
                for index in range(end, min(len(entries), end + 12))
                if entries[index].has_instruction
                and entries[index].mnemonic in _CALL_MNEMONICS
            ]
            if not consumers:
                continue
            if any(item.mnemonic in _CALL_MNEMONICS for item in entries[end + 12 :]):
                continue
            return {
                "index_start": start,
                "index_end": end - 1,
                "component_count": len(loads),
                "stack_offsets": sorted(loads),
                "mnemonics": [item.mnemonic for item in window],
                "final_consumers": consumers,
            }
    return None


def _exact_donor_evidence(
    document: Mapping[str, Any], donor_symbols: Sequence[str]
) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    for symbol in donor_symbols:
        if symbol in seen:
            continue
        seen.add(symbol)
        pair = _pair(document, symbol)
        if not causal_reducer._is_exact_pair(pair):
            continue
        target, candidate = _entries(pair)
        target_copy = _copy_run(target, require_asymmetry=False)
        candidate_copy = _copy_run(candidate, require_asymmetry=False)
        if target_copy is None or candidate_copy is None:
            continue
        signature_keys = ("component_count", "mnemonics")
        if any(target_copy[key] != candidate_copy[key] for key in signature_keys):
            continue
        result.append(
            {
                "symbol": symbol,
                "target_match_percent": (
                    pair.target.get("match_percent") if pair.target else None
                ),
                "candidate_match_percent": (
                    pair.candidate.get("match_percent") if pair.candidate else None
                ),
                "copy": target_copy,
                "signature_sha256": _sha256(
                    _canonical({key: target_copy[key] for key in signature_keys})
                ),
            }
        )
    return result


def _aggregate_self_copy_evaluation(
    document: Mapping[str, Any],
    target: Sequence[causal_reducer.Instruction],
    candidate: Sequence[causal_reducer.Instruction],
    donor_symbols: Sequence[str],
) -> dict[str, Any]:
    focus_copy = _copy_run(target, corresponding=candidate, require_asymmetry=True)
    if focus_copy is None:
        return _evaluation(
            "aggregate_self_copy_final_consumer",
            matched=False,
            reason="no target-only aggregate self-copy occurs at the final consumer boundary",
        )
    donors = _exact_donor_evidence(document, donor_symbols)
    compatible = [
        donor
        for donor in donors
        if donor["copy"]["component_count"] == focus_copy["component_count"]
        and donor["copy"]["mnemonics"] == focus_copy["mnemonics"]
    ]
    if not compatible:
        return _evaluation(
            "aggregate_self_copy_final_consumer",
            matched=False,
            reason="the focus signature has no explicitly named, exact same-report/TU donor with the same copy shape",
            evidence={
                "focus_copy": focus_copy,
                "requested_donor_symbols": list(dict.fromkeys(donor_symbols)),
                "exact_donors": donors,
            },
        )
    return _evaluation(
        "aggregate_self_copy_final_consumer",
        matched=True,
        reason="a target-only final-consumer self-copy has an exact structural donor in the same object/TU report",
        confidence=0.98,
        source_class="used_aggregate_self_assignment_at_final_consumer",
        recommendation="Test a natural aggregate self-assignment immediately before the final consumers, following the exact same-TU donor shape.",
        evidence={
            "focus_copy": focus_copy,
            "same_tu_basis": "focus and donor are paired functions in the same objdiff object report",
            "exact_donors": compatible,
        },
    )


def diagnose_document(
    document: Mapping[str, Any],
    *,
    focus_symbol: str,
    same_tu_donor_symbols: Sequence[str] = (),
    allocator_context: Mapping[str, Any] | None = None,
    parameter_allocation_context: Mapping[str, Any] | None = None,
    aggregate_use_context: Mapping[str, Any] | None = None,
    aggregate_followup_context: Mapping[str, Any] | None = None,
    address_taken_context: Mapping[str, Any] | None = None,
    same_tu_shape_context: Mapping[str, Any] | None = None,
    short_circuit_context: Mapping[str, Any] | None = None,
    exact_sibling_transfer_context: Mapping[str, Any] | None = None,
    capacity_context: Mapping[str, Any] | None = None,
    branch_context: Mapping[str, Any] | None = None,
    reciprocal_context: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Return a self-hashed, authority-free diagnosis for one function."""

    if not isinstance(document, Mapping):
        raise LearningInputError("objdiff report must be a JSON object")
    if not isinstance(focus_symbol, str) or not focus_symbol.strip():
        raise LearningInputError("focus_symbol must be non-empty text")
    focus = focus_symbol.strip()
    if any(
        not isinstance(value, str) or not value.strip()
        for value in same_tu_donor_symbols
    ):
        raise LearningInputError("same_tu_donor_symbols must contain non-empty text")
    donors = tuple(value.strip() for value in same_tu_donor_symbols)
    normalized_allocator_context = (
        _parse_allocator_context(allocator_context)
        if allocator_context is not None
        else None
    )
    normalized_parameter_allocation_context = (
        _parse_parameter_allocation_context(parameter_allocation_context)
        if parameter_allocation_context is not None
        else None
    )
    normalized_aggregate_use_context = (
        _parse_aggregate_use_context(aggregate_use_context)
        if aggregate_use_context is not None
        else None
    )
    normalized_aggregate_followup_context = (
        _parse_aggregate_followup_context(aggregate_followup_context)
        if aggregate_followup_context is not None
        else None
    )
    normalized_address_taken_context = (
        _parse_address_taken_context(address_taken_context)
        if address_taken_context is not None
        else None
    )
    normalized_same_tu_shape_context = (
        _parse_same_tu_shape_context(same_tu_shape_context)
        if same_tu_shape_context is not None
        else None
    )
    normalized_short_circuit_context = (
        _parse_short_circuit_context(short_circuit_context)
        if short_circuit_context is not None
        else None
    )
    normalized_exact_sibling_transfer_context = (
        _parse_exact_sibling_transfer_context(exact_sibling_transfer_context)
        if exact_sibling_transfer_context is not None
        else None
    )
    normalized_capacity_context = (
        _parse_capacity_context(capacity_context)
        if capacity_context is not None
        else None
    )
    normalized_branch_context = (
        _parse_branch_context(branch_context) if branch_context is not None else None
    )
    normalized_reciprocal_context = (
        _parse_reciprocal_context(reciprocal_context)
        if reciprocal_context is not None
        else None
    )
    pair = _pair(document, focus)
    target, candidate = _entries(pair)
    objdiff_canonical_sha256 = _sha256(_canonical(document))
    try:
        audit = causal_reducer.audit_document(
            document,
            focus_symbol=focus,
            include_exact_residuals=True,
            summary_only=False,
        )
    except causal_reducer.AuditInputError as exc:
        raise LearningInputError(
            f"causal reducer rejected report ({exc.code}): {exc.message}"
        ) from exc
    if audit.get("fail_closed") or audit.get("status") != "ok":
        raise LearningInputError(
            "causal reducer did not produce a closed successful audit"
        )

    evaluations = [
        _explicit_else_evaluation(audit),
        _loop_branch_destination_evaluation(
            pair,
            target,
            candidate,
            normalized_branch_context,
            objdiff_canonical_sha256,
        ),
        _assignment_condition_evaluation(pair, target, candidate),
        _allocator_two_register_swap_evaluation(
            pair,
            target,
            candidate,
            normalized_allocator_context,
            objdiff_canonical_sha256,
        ),
        _parameter_allocation_consumer_chain_evaluation(
            pair,
            target,
            candidate,
            normalized_parameter_allocation_context,
            objdiff_canonical_sha256,
        ),
        _aggregate_use_multiplicity_evaluation(
            pair,
            target,
            candidate,
            normalized_aggregate_use_context,
            objdiff_canonical_sha256,
        ),
        _aggregate_two_owner_followup_evaluation(
            pair,
            target,
            candidate,
            normalized_aggregate_followup_context,
            objdiff_canonical_sha256,
        ),
        _address_taken_local_pointer_evaluation(
            pair,
            target,
            candidate,
            normalized_address_taken_context,
            objdiff_canonical_sha256,
        ),
        _same_tu_exact_sibling_shape_evaluation(
            pair,
            target,
            candidate,
            normalized_same_tu_shape_context,
            objdiff_canonical_sha256,
        ),
        _short_circuit_boolean_call_order_evaluation(
            pair,
            target,
            candidate,
            normalized_short_circuit_context,
            objdiff_canonical_sha256,
        ),
        _dependency_equivalent_exact_sibling_transfer_evaluation(
            pair,
            target,
            candidate,
            normalized_exact_sibling_transfer_context,
            objdiff_canonical_sha256,
        ),
        _stack_extent_interface_capacity_evaluation(
            pair,
            normalized_capacity_context,
            objdiff_canonical_sha256,
        ),
        _reciprocal_source_shape_evaluation(
            pair,
            target,
            candidate,
            normalized_reciprocal_context,
            objdiff_canonical_sha256,
        ),
        _switch_fpr_evaluation(pair, target, candidate, audit),
        _aggregate_self_copy_evaluation(document, target, candidate, donors),
    ]
    if tuple(item["rule_id"] for item in evaluations) != _RULE_ORDER:
        raise AssertionError("rule evaluation order drifted")
    tool_path = Path(__file__).resolve()
    reducer_path = Path(causal_reducer.__file__).resolve()
    body = {
        "schema": SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "focus_symbol": focus,
        "inputs": {
            "objdiff_canonical_sha256": objdiff_canonical_sha256,
            "same_tu_donor_symbols": list(dict.fromkeys(donors)),
            "allocator_context_canonical_sha256": (
                _sha256(_canonical(normalized_allocator_context))
                if normalized_allocator_context is not None
                else None
            ),
            "parameter_allocation_context_canonical_sha256": (
                _sha256(_canonical(normalized_parameter_allocation_context))
                if normalized_parameter_allocation_context is not None
                else None
            ),
            "aggregate_use_context_canonical_sha256": (
                _sha256(_canonical(normalized_aggregate_use_context))
                if normalized_aggregate_use_context is not None
                else None
            ),
            "aggregate_followup_context_canonical_sha256": (
                _sha256(_canonical(normalized_aggregate_followup_context))
                if normalized_aggregate_followup_context is not None
                else None
            ),
            "address_taken_context_canonical_sha256": (
                _sha256(_canonical(normalized_address_taken_context))
                if normalized_address_taken_context is not None
                else None
            ),
            "same_tu_shape_context_canonical_sha256": (
                _sha256(_canonical(normalized_same_tu_shape_context))
                if normalized_same_tu_shape_context is not None
                else None
            ),
            "short_circuit_context_canonical_sha256": (
                _sha256(_canonical(normalized_short_circuit_context))
                if normalized_short_circuit_context is not None
                else None
            ),
            "exact_sibling_transfer_context_canonical_sha256": (
                _sha256(_canonical(normalized_exact_sibling_transfer_context))
                if normalized_exact_sibling_transfer_context is not None
                else None
            ),
            "capacity_context_canonical_sha256": (
                _sha256(_canonical(normalized_capacity_context))
                if normalized_capacity_context is not None
                else None
            ),
            "branch_context_canonical_sha256": (
                _sha256(_canonical(normalized_branch_context))
                if normalized_branch_context is not None
                else None
            ),
            "reciprocal_context_canonical_sha256": (
                _sha256(_canonical(normalized_reciprocal_context))
                if normalized_reciprocal_context is not None
                else None
            ),
        },
        "implementations": {
            "learning_rules": {
                "path": tool_path.name,
                "sha256": _sha256(tool_path.read_bytes()),
            },
            "causal_reducer": {
                "path": reducer_path.name,
                "schema_version": audit.get("schema_version"),
                "sha256": _sha256(reducer_path.read_bytes()),
            },
            "interaction_planner": {
                "path": Path(interaction_planner.__file__).name,
                "schema": interaction_planner.REQUEST_SCHEMA,
                "sha256": _sha256(Path(interaction_planner.__file__).read_bytes()),
            },
        },
        "evaluations": evaluations,
        "diagnoses": [dict(item) for item in evaluations if item["matched"]],
        "limitations": [
            "These rules compose deterministic physical signatures; they do not infer semantic variable names or original-source provenance.",
            "Recommendations are natural source classes only and never authorize source edits, candidate retention, promotion, or authority advancement.",
            "An exact donor is evidence for source shape only; the focus still requires its own complete proof chain.",
        ],
        "authority_advanced": False,
    }
    return _with_self_hash(body)


def _load_json(path: Path, *, label: str = "objdiff report") -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise LearningInputError(f"cannot read {label} {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise LearningInputError(f"invalid JSON in {label} {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise LearningInputError(f"{label} {path} must contain a JSON object")
    return value


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Apply evidence-only CRACK_REPORT learning rules to one objdiff function."
    )
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--function", required=True, dest="focus_symbol")
    parser.add_argument(
        "--same-tu-donor",
        action="append",
        default=[],
        dest="same_tu_donors",
        help="explicitly named exact donor function from the same object report",
    )
    parser.add_argument(
        "--allocator-context",
        type=Path,
        help=(
            "authenticated allocator_two_register_swap_context/v1 JSON with proof, "
            "VarInfo owner, boundary, and optional measured-cell evidence"
        ),
    )
    parser.add_argument(
        "--parameter-allocation-context",
        type=Path,
        help=(
            "authenticated parameter_allocation_consumer_chain_context/v1 JSON "
            "with parameter/allocation owner, producer capture, and ordered consumer proof"
        ),
    )
    parser.add_argument(
        "--aggregate-use-context",
        type=Path,
        help=(
            "authenticated aggregate_use_multiplicity_context/v1 JSON with exact "
            "saved-GPR owners, complete member-copy groups, and preserved consumers"
        ),
    )
    parser.add_argument(
        "--aggregate-followup-context",
        type=Path,
        help=(
            "authenticated aggregate_two_owner_followup_context/v1 JSON with a "
            "post-aggregate two-owner swap, declaration order, and rejected fusion proof"
        ),
    )
    parser.add_argument(
        "--address-taken-context",
        type=Path,
        help=(
            "authenticated address_taken_local_pointer_context/v1 JSON with the "
            "target home, incoming owner, local address owner, and typed call boundary"
        ),
    )
    parser.add_argument(
        "--same-tu-shape-context",
        type=Path,
        help=(
            "authenticated same_tu_exact_sibling_shape_context/v1 JSON with "
            "fixed-array tail, caller ABI, zero-chain, and exact donor evidence"
        ),
    )
    parser.add_argument(
        "--short-circuit-context",
        type=Path,
        help=(
            "authenticated short_circuit_boolean_call_order_context/v1 JSON with "
            "target call order, shared Boolean blocks, rejected direct assignment, "
            "and exact-topology owner-cycle evidence"
        ),
    )
    parser.add_argument(
        "--exact-sibling-transfer-context",
        type=Path,
        help=(
            "authenticated dependency_equivalent_exact_sibling_transfer_context/v1 "
            "JSON with exact donor, dependency-equivalent Boolean topology, capacity, "
            "and independent int-to-s16 consumer-boundary evidence"
        ),
    )
    parser.add_argument(
        "--capacity-context",
        type=Path,
        help=(
            "authenticated stack_extent_interface_capacity_context/v1 JSON with "
            "stack extent, live array, producer maxima, and bounded declaration positions"
        ),
    )
    parser.add_argument(
        "--branch-context",
        type=Path,
        help=(
            "authenticated loop_branch_destination_context/v1 JSON with the sole "
            "conditional row and increment/exit destination proof"
        ),
    )
    parser.add_argument(
        "--reciprocal-context",
        type=Path,
        help=(
            "authenticated reciprocal_source_shape_context/v1 JSON with exact-size, "
            "typed-literal, load-window, relocation, and compiler-neutral control proof"
        ),
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    try:
        result = diagnose_document(
            _load_json(args.report),
            focus_symbol=args.focus_symbol,
            same_tu_donor_symbols=args.same_tu_donors,
            allocator_context=(
                _load_json(args.allocator_context, label="allocator context")
                if args.allocator_context is not None
                else None
            ),
            parameter_allocation_context=(
                _load_json(
                    args.parameter_allocation_context,
                    label="parameter allocation context",
                )
                if args.parameter_allocation_context is not None
                else None
            ),
            aggregate_use_context=(
                _load_json(args.aggregate_use_context, label="aggregate-use context")
                if args.aggregate_use_context is not None
                else None
            ),
            aggregate_followup_context=(
                _load_json(
                    args.aggregate_followup_context,
                    label="aggregate follow-up context",
                )
                if args.aggregate_followup_context is not None
                else None
            ),
            address_taken_context=(
                _load_json(
                    args.address_taken_context,
                    label="address-taken local pointer context",
                )
                if args.address_taken_context is not None
                else None
            ),
            same_tu_shape_context=(
                _load_json(
                    args.same_tu_shape_context,
                    label="same-TU exact-sibling source-shape context",
                )
                if args.same_tu_shape_context is not None
                else None
            ),
            short_circuit_context=(
                _load_json(
                    args.short_circuit_context,
                    label="short-circuit Boolean call-order context",
                )
                if args.short_circuit_context is not None
                else None
            ),
            exact_sibling_transfer_context=(
                _load_json(
                    args.exact_sibling_transfer_context,
                    label="dependency-equivalent exact-sibling transfer context",
                )
                if args.exact_sibling_transfer_context is not None
                else None
            ),
            capacity_context=(
                _load_json(args.capacity_context, label="capacity context")
                if args.capacity_context is not None
                else None
            ),
            branch_context=(
                _load_json(args.branch_context, label="branch context")
                if args.branch_context is not None
                else None
            ),
            reciprocal_context=(
                _load_json(args.reciprocal_context, label="reciprocal context")
                if args.reciprocal_context is not None
                else None
            ),
        )
    except LearningInputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    print(json.dumps(result, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
