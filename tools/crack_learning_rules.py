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


SCHEMA = "crack_learning_diagnosis/v3"
SCHEMA_VERSION = 3
HASH_FIELD = "diagnosis_sha256"
ALLOCATOR_CONTEXT_SCHEMA = "allocator_two_register_swap_context/v1"
PARAMETER_ALLOCATION_CONTEXT_SCHEMA = "parameter_allocation_consumer_chain_context/v1"
AGGREGATE_USE_CONTEXT_SCHEMA = "aggregate_use_multiplicity_context/v1"
CAPACITY_CONTEXT_SCHEMA = "stack_extent_interface_capacity_context/v1"
BRANCH_CONTEXT_SCHEMA = "loop_branch_destination_context/v1"
RECIPROCAL_CONTEXT_SCHEMA = "reciprocal_source_shape_context/v1"

_REGISTER_RE = re.compile(r"\b(?P<kind>[rRfF])(?P<number>[0-9]|[12][0-9]|3[01])\b")
_STACK_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*r1\s*\)",
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
