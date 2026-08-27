#!/usr/bin/env python3
"""Fail-closed source-link provenance gate for whole-owner closure.

Retail output equality is not owner-closure proof when the configured owner is
still ``NonMatching`` and the linker therefore selects an extracted target
object.  This module validates a closed evidence packet, binds the configured
status to the selected link-manifest entry, and distinguishes a genuinely
source-linked closure from a fallback-linked checksum.

An optional, equally closed sub-packet can diagnose the minimum live,
addressable, read-only one-f32 owner behind a single SDA21 seam.  That diagnosis
is evidence-only.  This module never edits source, changes configuration,
retains a candidate, promotes an owner, or advances authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence


CONTEXT_SCHEMA = "source_linked_owner_closure_context/v1"
RESULT_SCHEMA = "source_linked_owner_closure/v1"
LINK_MANIFEST_SCHEMA = "source_link_owner_manifest/v1"
RULE_ID = "source_linked_owner_closure_provenance"
HASH_FIELD = "closure_sha256"

_SHA256_RE = re.compile(r"[0-9a-f]{64}")
_IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]{0,127}")
_F32_BITS_RE = re.compile(r"[0-9a-f]{8}")
_RAW_TARGET_NAME_RE = re.compile(r"(?:lbl_|unk_|fn_1_|0x)[0-9a-fA-F_]+")
_CONFIGURED_STATUSES = {"Matching", "NonMatching"}
_OBJECT_ORIGINS = {"reconstructed_source", "extracted_target_fallback"}


class SourceLinkedClosureInputError(ValueError):
    """The supplied packet cannot safely support a closure diagnosis."""


def _closed(
    value: Any, *, allowed: set[str], required: set[str], label: str
) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise SourceLinkedClosureInputError(f"{label} must be a JSON object")
    missing = required - set(value)
    extra = set(value) - allowed
    if missing or extra:
        raise SourceLinkedClosureInputError(
            f"{label} fields are not closed; missing={sorted(missing)}, extra={sorted(extra)}"
        )
    return value


def _text(value: Any, label: str, *, limit: int = 1024) -> str:
    if not isinstance(value, str) or not value.strip():
        raise SourceLinkedClosureInputError(f"{label} must be non-empty text")
    result = value.strip()
    if len(result) > limit:
        raise SourceLinkedClosureInputError(f"{label} exceeds {limit} characters")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, limit=64).lower()
    if _SHA256_RE.fullmatch(result) is None:
        raise SourceLinkedClosureInputError(f"{label} must be lowercase SHA-256")
    return result


def _identifier(value: Any, label: str) -> str:
    result = _text(value, label, limit=128)
    if _IDENTIFIER_RE.fullmatch(result) is None:
        raise SourceLinkedClosureInputError(f"{label} must be a C identifier")
    return result


def _bool(value: Any, label: str) -> bool:
    if not isinstance(value, bool):
        raise SourceLinkedClosureInputError(f"{label} must be a Boolean")
    return value


def _uint(value: Any, label: str, *, minimum: int = 0, maximum: int = 1 << 31) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise SourceLinkedClosureInputError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise SourceLinkedClosureInputError(
            f"{label} must be from {minimum} through {maximum}"
        )
    return value


def _repo_path(value: Any, label: str) -> str:
    raw = _text(value, label, limit=512).replace("\\", "/")
    path = PurePosixPath(raw)
    if path.is_absolute() or ".." in path.parts or ":" in raw:
        raise SourceLinkedClosureInputError(f"{label} must be repository-relative")
    return path.as_posix()


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        sort_keys=True,
        separators=(",", ":"),
        ensure_ascii=False,
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical_bytes(value)).hexdigest()


def _parse_configure(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "source_path",
        "configured_status",
        "configure_sha256",
        "status_receipt_sha256",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    status = _text(item.get("configured_status"), f"{label}.configured_status", limit=32)
    if status not in _CONFIGURED_STATUSES:
        raise SourceLinkedClosureInputError(
            f"{label}.configured_status must be Matching or NonMatching"
        )
    return {
        "source_path": _repo_path(item.get("source_path"), f"{label}.source_path"),
        "configured_status": status,
        "configure_sha256": _sha256(item.get("configure_sha256"), f"{label}.configure_sha256"),
        "status_receipt_sha256": _sha256(
            item.get("status_receipt_sha256"), f"{label}.status_receipt_sha256"
        ),
    }


def _parse_candidate(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "source_sha256",
        "object_path",
        "object_sha256",
        "strict_report_sha256",
        "data_report_sha256",
        "compile_attestation_sha256",
        "candidate_record_sha256",
        "functions_exact",
        "functions_total",
        "strict_diff_rows",
        "data_diff_rows",
        "physical_relocations_exact",
        "physical_relocations_total",
        "protected_sibling_losses",
        "owner_sections_exact",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    hashes = {
        field: _sha256(item.get(field), f"{label}.{field}")
        for field in (
            "source_sha256",
            "object_sha256",
            "strict_report_sha256",
            "data_report_sha256",
            "compile_attestation_sha256",
            "candidate_record_sha256",
        )
    }
    counts = {
        field: _uint(item.get(field), f"{label}.{field}")
        for field in (
            "functions_exact",
            "functions_total",
            "strict_diff_rows",
            "data_diff_rows",
            "physical_relocations_exact",
            "physical_relocations_total",
            "protected_sibling_losses",
        )
    }
    return {
        **hashes,
        **counts,
        "object_path": _repo_path(item.get("object_path"), f"{label}.object_path"),
        "owner_sections_exact": _bool(
            item.get("owner_sections_exact"), f"{label}.owner_sections_exact"
        ),
    }


def _parse_manifest(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "schema",
        "manifest_file_sha256",
        "manifest_canonical_sha256",
        "owner",
        "source_path",
        "configured_status",
        "selected_object_path",
        "selected_object_sha256",
        "object_origin",
        "clean_build",
        "build_receipt_sha256",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    if _text(item.get("schema"), f"{label}.schema") != LINK_MANIFEST_SCHEMA:
        raise SourceLinkedClosureInputError(
            f"{label}.schema must be {LINK_MANIFEST_SCHEMA}"
        )
    status = _text(item.get("configured_status"), f"{label}.configured_status", limit=32)
    if status not in _CONFIGURED_STATUSES:
        raise SourceLinkedClosureInputError(
            f"{label}.configured_status must be Matching or NonMatching"
        )
    origin = _text(item.get("object_origin"), f"{label}.object_origin", limit=64)
    if origin not in _OBJECT_ORIGINS:
        raise SourceLinkedClosureInputError(
            f"{label}.object_origin must identify reconstructed source or target fallback"
        )
    normalized = {
        "schema": LINK_MANIFEST_SCHEMA,
        "manifest_file_sha256": _sha256(
            item.get("manifest_file_sha256"), f"{label}.manifest_file_sha256"
        ),
        "owner": _text(item.get("owner"), f"{label}.owner", limit=256),
        "source_path": _repo_path(item.get("source_path"), f"{label}.source_path"),
        "configured_status": status,
        "selected_object_path": _repo_path(
            item.get("selected_object_path"), f"{label}.selected_object_path"
        ),
        "selected_object_sha256": _sha256(
            item.get("selected_object_sha256"), f"{label}.selected_object_sha256"
        ),
        "object_origin": origin,
        "clean_build": _bool(item.get("clean_build"), f"{label}.clean_build"),
        "build_receipt_sha256": _sha256(
            item.get("build_receipt_sha256"), f"{label}.build_receipt_sha256"
        ),
    }
    supplied = _sha256(
        item.get("manifest_canonical_sha256"), f"{label}.manifest_canonical_sha256"
    )
    if canonical_sha256(normalized) != supplied:
        raise SourceLinkedClosureInputError(
            f"{label}.manifest_canonical_sha256 does not hash the normalized manifest"
        )
    normalized["manifest_canonical_sha256"] = supplied
    return normalized


def _parse_retail_output(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "link_manifest_canonical_sha256",
        "configured_files_exact",
        "configured_files_total",
        "checksum_receipt_sha256",
        "main_binary_sha256",
        "retail_main_binary_sha256",
        "main_binary_byte_identical",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    return {
        "link_manifest_canonical_sha256": _sha256(
            item.get("link_manifest_canonical_sha256"),
            f"{label}.link_manifest_canonical_sha256",
        ),
        "configured_files_exact": _uint(
            item.get("configured_files_exact"), f"{label}.configured_files_exact"
        ),
        "configured_files_total": _uint(
            item.get("configured_files_total"), f"{label}.configured_files_total", minimum=1
        ),
        "checksum_receipt_sha256": _sha256(
            item.get("checksum_receipt_sha256"), f"{label}.checksum_receipt_sha256"
        ),
        "main_binary_sha256": _sha256(
            item.get("main_binary_sha256"), f"{label}.main_binary_sha256"
        ),
        "retail_main_binary_sha256": _sha256(
            item.get("retail_main_binary_sha256"),
            f"{label}.retail_main_binary_sha256",
        ),
        "main_binary_byte_identical": _bool(
            item.get("main_binary_byte_identical"), f"{label}.main_binary_byte_identical"
        ),
    }


def _parse_addressable_owner(
    value: Any, *, label: str, focus_function: str
) -> dict[str, Any] | None:
    if value is None:
        return None
    root_fields = {"target", "source", "relocation", "controls"}
    item = _closed(value, allowed=root_fields, required=root_fields, label=label)

    target_fields = {
        "symbol",
        "section",
        "size_bytes",
        "alignment",
        "value_bits",
        "read_only",
        "symbol_extent_sealed",
        "creation_order",
        "section_receipt_sha256",
        "chronology_receipt_sha256",
    }
    target_raw = _closed(
        item.get("target"), allowed=target_fields, required=target_fields, label=f"{label}.target"
    )
    target_bits = _text(target_raw.get("value_bits"), f"{label}.target.value_bits", limit=8).lower()
    if _F32_BITS_RE.fullmatch(target_bits) is None:
        raise SourceLinkedClosureInputError(f"{label}.target.value_bits must be eight hex digits")
    target = {
        "symbol": _text(target_raw.get("symbol"), f"{label}.target.symbol", limit=256),
        "section": _text(target_raw.get("section"), f"{label}.target.section", limit=64),
        "size_bytes": _uint(target_raw.get("size_bytes"), f"{label}.target.size_bytes"),
        "alignment": _uint(target_raw.get("alignment"), f"{label}.target.alignment", minimum=1),
        "value_bits": target_bits,
        "read_only": _bool(target_raw.get("read_only"), f"{label}.target.read_only"),
        "symbol_extent_sealed": _bool(
            target_raw.get("symbol_extent_sealed"), f"{label}.target.symbol_extent_sealed"
        ),
        "creation_order": _uint(
            target_raw.get("creation_order"), f"{label}.target.creation_order"
        ),
        "section_receipt_sha256": _sha256(
            target_raw.get("section_receipt_sha256"), f"{label}.target.section_receipt_sha256"
        ),
        "chronology_receipt_sha256": _sha256(
            target_raw.get("chronology_receipt_sha256"),
            f"{label}.target.chronology_receipt_sha256",
        ),
    }

    source_fields = {
        "name",
        "declaration_class",
        "element_count",
        "size_bytes",
        "initializer_bits",
        "section",
        "read_only",
        "use_count",
        "semantic_consumer",
        "creation_order",
        "source_order_receipt_sha256",
    }
    source_raw = _closed(
        item.get("source"), allowed=source_fields, required=source_fields, label=f"{label}.source"
    )
    source_name = _identifier(source_raw.get("name"), f"{label}.source.name")
    if _RAW_TARGET_NAME_RE.fullmatch(source_name) is not None:
        raise SourceLinkedClosureInputError(
            f"{label}.source.name must be semantic rather than address-derived"
        )
    initializer_bits = _text(
        source_raw.get("initializer_bits"), f"{label}.source.initializer_bits", limit=8
    ).lower()
    if _F32_BITS_RE.fullmatch(initializer_bits) is None:
        raise SourceLinkedClosureInputError(
            f"{label}.source.initializer_bits must be eight hex digits"
        )
    semantic_consumer = _text(
        source_raw.get("semantic_consumer"), f"{label}.source.semantic_consumer", limit=512
    )
    if re.search(rf"\b{re.escape(source_name)}\b", semantic_consumer) is None:
        raise SourceLinkedClosureInputError(
            f"{label}.source.semantic_consumer must consume the semantic owner name"
        )
    source = {
        "name": source_name,
        "declaration_class": _text(
            source_raw.get("declaration_class"), f"{label}.source.declaration_class", limit=64
        ),
        "element_count": _uint(
            source_raw.get("element_count"), f"{label}.source.element_count"
        ),
        "size_bytes": _uint(source_raw.get("size_bytes"), f"{label}.source.size_bytes"),
        "initializer_bits": initializer_bits,
        "section": _text(source_raw.get("section"), f"{label}.source.section", limit=64),
        "read_only": _bool(source_raw.get("read_only"), f"{label}.source.read_only"),
        "use_count": _uint(source_raw.get("use_count"), f"{label}.source.use_count"),
        "semantic_consumer": semantic_consumer,
        "creation_order": _uint(
            source_raw.get("creation_order"), f"{label}.source.creation_order"
        ),
        "source_order_receipt_sha256": _sha256(
            source_raw.get("source_order_receipt_sha256"),
            f"{label}.source.source_order_receipt_sha256",
        ),
    }

    relocation_fields = {
        "type",
        "count",
        "consumer_function",
        "target_owner",
        "candidate_owner",
        "physical_identity",
        "receipt_sha256",
    }
    relocation_raw = _closed(
        item.get("relocation"),
        allowed=relocation_fields,
        required=relocation_fields,
        label=f"{label}.relocation",
    )
    relocation = {
        "type": _text(relocation_raw.get("type"), f"{label}.relocation.type", limit=64),
        "count": _uint(relocation_raw.get("count"), f"{label}.relocation.count"),
        "consumer_function": _identifier(
            relocation_raw.get("consumer_function"), f"{label}.relocation.consumer_function"
        ),
        "target_owner": _text(
            relocation_raw.get("target_owner"), f"{label}.relocation.target_owner", limit=256
        ),
        "candidate_owner": _identifier(
            relocation_raw.get("candidate_owner"), f"{label}.relocation.candidate_owner"
        ),
        "physical_identity": _bool(
            relocation_raw.get("physical_identity"), f"{label}.relocation.physical_identity"
        ),
        "receipt_sha256": _sha256(
            relocation_raw.get("receipt_sha256"), f"{label}.relocation.receipt_sha256"
        ),
    }

    control_fields = {
        "direct_scalar_literal_rejected",
        "automatic_or_volatile_rejected",
        "synthetic_target_label_absent",
        "padding_absent",
        "register_shaping_absent",
        "control_receipt_sha256",
    }
    controls_raw = _closed(
        item.get("controls"), allowed=control_fields, required=control_fields, label=f"{label}.controls"
    )
    controls = {
        field: _bool(controls_raw.get(field), f"{label}.controls.{field}")
        for field in control_fields
        if field != "control_receipt_sha256"
    }
    controls["control_receipt_sha256"] = _sha256(
        controls_raw.get("control_receipt_sha256"), f"{label}.controls.control_receipt_sha256"
    )

    failures: list[str] = []
    if target["section"] != ".sdata2" or source["section"] != ".sdata2":
        failures.append("the target and source owner must both be read-only .sdata2")
    if target["size_bytes"] != 4 or source["size_bytes"] != 4:
        failures.append("the target and source owner must each have exact four-byte extent")
    if target["alignment"] != 4:
        failures.append("the target owner must have four-byte alignment")
    if not target["read_only"] or not target["symbol_extent_sealed"] or not source["read_only"]:
        failures.append("read-only target extent and source ownership must be sealed")
    if source["declaration_class"] != "one_element_read_only_float_array":
        failures.append("the source class must be one_element_read_only_float_array")
    if source["element_count"] != 1 or source["use_count"] != 1:
        failures.append("the source owner must have one element and one live consumer")
    if source["initializer_bits"] != target["value_bits"]:
        failures.append("the source initializer must reproduce the target f32 bits")
    if source["creation_order"] != target["creation_order"]:
        failures.append("source and target creation chronology must agree")
    if relocation != {
        **relocation,
        "type": "R_PPC_EMB_SDA21",
        "count": 1,
        "consumer_function": focus_function,
        "target_owner": target["symbol"],
        "candidate_owner": source_name,
        "physical_identity": True,
    }:
        failures.append("one exact SDA21 consumer must bind the target and semantic source owners")
    if not all(value is True for key, value in controls.items() if key != "control_receipt_sha256"):
        failures.append("all shaping and simpler-source controls must be closed")
    if failures:
        raise SourceLinkedClosureInputError(f"{label} is not sealed: " + "; ".join(failures))

    return {
        "target": target,
        "source": source,
        "relocation": relocation,
        "controls": controls,
    }


def _parse_telemetry(value: Any, label: str) -> dict[str, Any]:
    fields = {
        "telemetry_complete",
        "excluded_from_measured_crack_per_hour",
        "no_imputation",
        "interval_log_sha256",
    }
    item = _closed(value, allowed=fields, required=fields, label=label)
    complete = _bool(item.get("telemetry_complete"), f"{label}.telemetry_complete")
    excluded = _bool(
        item.get("excluded_from_measured_crack_per_hour"),
        f"{label}.excluded_from_measured_crack_per_hour",
    )
    no_imputation = _bool(item.get("no_imputation"), f"{label}.no_imputation")
    if not no_imputation or (not complete and not excluded):
        raise SourceLinkedClosureInputError(
            f"{label} incomplete evidence must be excluded and all evidence must forbid imputation"
        )
    return {
        "telemetry_complete": complete,
        "excluded_from_measured_crack_per_hour": excluded,
        "no_imputation": True,
        "interval_log_sha256": _sha256(
            item.get("interval_log_sha256"), f"{label}.interval_log_sha256"
        ),
    }


def parse_context(value: Mapping[str, Any]) -> dict[str, Any]:
    label = "source-linked owner closure context"
    fields = {
        "schema",
        "report_artifact_sha256",
        "owner",
        "focus_function",
        "objdiff_canonical_sha256",
        "configure",
        "candidate",
        "target",
        "link_manifest",
        "retail_output",
        "addressable_owner",
        "telemetry",
        "authority_advanced",
    }
    context = _closed(value, allowed=fields, required=fields, label=label)
    if _text(context.get("schema"), f"{label}.schema") != CONTEXT_SCHEMA:
        raise SourceLinkedClosureInputError(f"{label}.schema must be {CONTEXT_SCHEMA}")
    if _bool(context.get("authority_advanced"), f"{label}.authority_advanced"):
        raise SourceLinkedClosureInputError(f"{label}.authority_advanced must be false")

    owner = _text(context.get("owner"), f"{label}.owner", limit=256)
    focus = _identifier(context.get("focus_function"), f"{label}.focus_function")
    target_raw = _closed(
        context.get("target"),
        allowed={"object_sha256"},
        required={"object_sha256"},
        label=f"{label}.target",
    )
    return {
        "schema": CONTEXT_SCHEMA,
        "report_artifact_sha256": _sha256(
            context.get("report_artifact_sha256"), f"{label}.report_artifact_sha256"
        ),
        "owner": owner,
        "focus_function": focus,
        "objdiff_canonical_sha256": _sha256(
            context.get("objdiff_canonical_sha256"), f"{label}.objdiff_canonical_sha256"
        ),
        "configure": _parse_configure(context.get("configure"), f"{label}.configure"),
        "candidate": _parse_candidate(context.get("candidate"), f"{label}.candidate"),
        "target": {
            "object_sha256": _sha256(
                target_raw.get("object_sha256"), f"{label}.target.object_sha256"
            )
        },
        "link_manifest": _parse_manifest(
            context.get("link_manifest"), f"{label}.link_manifest"
        ),
        "retail_output": _parse_retail_output(
            context.get("retail_output"), f"{label}.retail_output"
        ),
        "addressable_owner": _parse_addressable_owner(
            context.get("addressable_owner"),
            label=f"{label}.addressable_owner",
            focus_function=focus,
        ),
        "telemetry": _parse_telemetry(context.get("telemetry"), f"{label}.telemetry"),
        "authority_advanced": False,
    }


def _candidate_proof_errors(candidate: Mapping[str, Any]) -> list[str]:
    errors: list[str] = []
    if candidate["functions_exact"] != candidate["functions_total"]:
        errors.append("not every owner function is exact")
    if candidate["strict_diff_rows"] or candidate["data_diff_rows"]:
        errors.append("strict or data diff rows remain")
    if candidate["physical_relocations_exact"] != candidate["physical_relocations_total"]:
        errors.append("physical relocation identity is incomplete")
    if candidate["protected_sibling_losses"]:
        errors.append("protected sibling loss is nonzero")
    if not candidate["owner_sections_exact"]:
        errors.append("owner section closure is incomplete")
    return errors


def _addressable_owner_diagnosis(value: Mapping[str, Any] | None) -> dict[str, Any] | None:
    if value is None:
        return None
    return {
        "matched": True,
        "source_class": "minimum_live_addressable_read_only_f32_owner",
        "reason": (
            "one sealed four-byte read-only .sdata2 owner has one exact SDA21 consumer; "
            "the semantic one-element source owner reproduces its bits and creation chronology "
            "while simpler or shaping controls are closed"
        ),
        "recommendation": (
            "Audit a semantic, live, addressable one-f32 owner at the sealed creation boundary; "
            "do not use raw target labels, padding, volatile storage, or register shaping."
        ),
        "evidence": value,
        "authority_advanced": False,
    }


def evaluate(
    context: Mapping[str, Any], *, focus_symbol: str | None = None,
    objdiff_canonical_sha256: str | None = None,
) -> dict[str, Any]:
    if (focus_symbol is None) != (objdiff_canonical_sha256 is None):
        raise SourceLinkedClosureInputError(
            "focus_symbol and objdiff_canonical_sha256 must be supplied together"
        )
    normalized = parse_context(context)
    if focus_symbol is not None:
        focus = _identifier(focus_symbol, "focus_symbol")
        objdiff_sha = _sha256(objdiff_canonical_sha256, "objdiff_canonical_sha256")
        if (
            normalized["focus_function"] != focus
            or normalized["objdiff_canonical_sha256"] != objdiff_sha
        ):
            body = {
                "schema": RESULT_SCHEMA,
                "matched": False,
                "status": "CONTEXT_NOT_BOUND_TO_FOCUS",
                "closure_ready": False,
                "reason": "the closure context is not bound to this focus report",
                "context_sha256": canonical_sha256(normalized),
                "authority_advanced": False,
            }
            return _with_self_hash(body)

    configure = normalized["configure"]
    candidate = normalized["candidate"]
    target = normalized["target"]
    manifest = normalized["link_manifest"]
    retail = normalized["retail_output"]
    blocked: list[str] = []

    blocked.extend(_candidate_proof_errors(candidate))
    if configure["configured_status"] != manifest["configured_status"]:
        blocked.append("configure status and link-manifest status differ")
    if configure["source_path"] != manifest["source_path"]:
        blocked.append("configure source path and link-manifest source path differ")
    if normalized["owner"] != manifest["owner"]:
        blocked.append("owner identity and link-manifest owner differ")

    fallback_linked = configure["configured_status"] == "NonMatching"
    if fallback_linked:
        blocked.append(
            "owner is NonMatching, so a retail checksum can be satisfied by target fallback"
        )
        if (
            manifest["object_origin"] != "extracted_target_fallback"
            or manifest["selected_object_sha256"] != target["object_sha256"]
        ):
            blocked.append("the NonMatching manifest does not honestly bind its fallback object")
    else:
        if manifest["object_origin"] != "reconstructed_source":
            blocked.append("Matching closure did not select a reconstructed-source object")
        if manifest["selected_object_path"] != candidate["object_path"]:
            blocked.append("linked object path does not equal the verified candidate object path")
        if manifest["selected_object_sha256"] != candidate["object_sha256"]:
            blocked.append("linked object hash does not equal the verified candidate object hash")

    if not manifest["clean_build"]:
        blocked.append("link manifest is not from a clean build")
    if retail["link_manifest_canonical_sha256"] != manifest["manifest_canonical_sha256"]:
        blocked.append("retail output receipt is not bound to this link manifest")
    if retail["configured_files_exact"] != retail["configured_files_total"]:
        blocked.append("not every configured retail output is exact")
    if (
        not retail["main_binary_byte_identical"]
        or retail["main_binary_sha256"] != retail["retail_main_binary_sha256"]
    ):
        blocked.append("rebuilt main binary is not byte-identical to retail")

    closure_ready = not blocked
    if closure_ready:
        status = "SOURCE_LINK_CLOSURE_VERIFIED"
        reason = (
            "the owner is Matching, the clean link manifest selects the exact verified "
            "candidate object path and hash, and the manifest-bound retail outputs are exact"
        )
        recommendation = (
            "Record this authority-false validation in the owner closure receipt, then let the "
            "owning orchestrator apply its independent promotion and landing gates."
        )
        source_class = "source_linked_owner_closure_gate"
    elif fallback_linked:
        status = "BLOCKED_FALLBACK_LINKED"
        reason = (
            "the owner remains NonMatching; retail checksum equality is fallback-linked and "
            "cannot authenticate reconstructed source"
        )
        recommendation = (
            "Do not close the owner. Configure it Matching, rebuild cleanly, and bind the exact "
            "linked candidate object path/hash before rerunning retail checksums."
        )
        source_class = "fallback_linked_checksum_rejection"
    else:
        status = "BLOCKED_SOURCE_LINK_PROVENANCE"
        reason = "the Matching owner lacks a complete candidate-to-manifest-to-output identity chain"
        recommendation = (
            "Repair the listed provenance or proof gaps and rerun; checksum equality alone is insufficient."
        )
        source_class = "incomplete_source_link_provenance"

    body = {
        "schema": RESULT_SCHEMA,
        "matched": True,
        "status": status,
        "closure_ready": closure_ready,
        "reason": reason,
        "source_class": source_class,
        "recommendation": recommendation,
        "owner": normalized["owner"],
        "focus_function": normalized["focus_function"],
        "configured_status": configure["configured_status"],
        "candidate_object": {
            "path": candidate["object_path"],
            "sha256": candidate["object_sha256"],
        },
        "linked_object": {
            "path": manifest["selected_object_path"],
            "sha256": manifest["selected_object_sha256"],
            "origin": manifest["object_origin"],
        },
        "manifest_canonical_sha256": manifest["manifest_canonical_sha256"],
        "retail_checksum_exact": (
            retail["configured_files_exact"] == retail["configured_files_total"]
            and retail["main_binary_byte_identical"]
            and retail["main_binary_sha256"] == retail["retail_main_binary_sha256"]
        ),
        "blocked_reasons": blocked,
        "addressable_owner_diagnosis": _addressable_owner_diagnosis(
            normalized["addressable_owner"]
        ),
        "telemetry": normalized["telemetry"],
        "report_artifact_sha256": normalized["report_artifact_sha256"],
        "context_sha256": canonical_sha256(normalized),
        "implementation_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "limitations": [
            "This validator consumes authenticated receipts; it does not compile, link, or inspect source by itself.",
            "An addressable-owner diagnosis ranks an evidence class only and does not authenticate original spelling.",
            "Closure-ready output remains authority-free and does not retain, promote, land, or update progress.",
        ],
        "authority_advanced": False,
    }
    return _with_self_hash(body)


def _with_self_hash(body: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(body)
    result[HASH_FIELD] = canonical_sha256(result)
    return result


def verify_self_hash(value: Mapping[str, Any]) -> bool:
    expected = value.get(HASH_FIELD)
    if not isinstance(expected, str):
        return False
    body = {key: item for key, item in value.items() if key != HASH_FIELD}
    return expected == canonical_sha256(body)


def _load_json(path: Path) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise SourceLinkedClosureInputError(f"cannot read context {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise SourceLinkedClosureInputError(f"invalid JSON in context {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise SourceLinkedClosureInputError("context must contain a JSON object")
    return value


def _write_json(path: Path, value: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(value, indent=2, sort_keys=True) + "\n"
    fd, temp_name = tempfile.mkstemp(prefix=f".{path.name}.", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp_name, path)
    except BaseException:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass
        raise


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--context", required=True, type=Path)
    parser.add_argument("--focus-symbol")
    parser.add_argument("--objdiff-canonical-sha256")
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--require-closure",
        action="store_true",
        help="return exit status 2 when the valid diagnostic does not prove source-linked closure",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    try:
        result = evaluate(
            _load_json(args.context),
            focus_symbol=args.focus_symbol,
            objdiff_canonical_sha256=args.objdiff_canonical_sha256,
        )
    except SourceLinkedClosureInputError as exc:
        print(f"source-linked closure input rejected: {exc}")
        return 1
    if args.output is not None:
        _write_json(args.output, result)
    else:
        print(json.dumps(result, indent=2, sort_keys=True))
    if args.require_closure and not result.get("closure_ready"):
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
