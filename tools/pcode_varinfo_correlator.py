#!/usr/bin/env python3
"""Audit the possible join between MWCC VarInfo objects and PCode vregs.

The GC/2.6 traces used by the Capsule recovery work expose two useful but
different views of one compile.  The allocator trace contains named VarInfo
objects and their final register state.  The PCode trace contains virtual
register occurrence spans and instruction memory-object labels.  The current
producer deliberately does *not* expose a pointer-free object identity or
def/use IDs at the boundary between those views.

This tool therefore produces an evidence report, not a guessed join.  A
memory-object label shared by an instruction and an allocator name contributes
an auditable candidate fingerprint.  It is reported as ``UNRESOLVED_EVIDENCE``
unless the authenticated source inventory explicitly binds that source object
to vreg IDs.  Missing, duplicate, or conflicting evidence is retained in the
report as UNKNOWN/AMBIGUOUS rather than being silently assigned.

An authenticated join additionally requires external SHA-256 anchors for the
raw allocator, v2 PCode, and v3 trace bytes.  Those anchors must come from the
trusted capture receipt; they must never be computed from or echoed out of an
untrusted payload supplied to this tool.

The implementation is standard-library-only and accepts both the authenticated
v2 trace and the diagnostic v3 serial trace.  Version 3 has a separate,
direct-ownership evidence path: it may resolve a source object only when the
same-session frontend/ownership packet explicitly binds exactly one source
object to exactly one virtual register and the packet's external provenance
anchors authenticate that claim.  It never derives a virtual-register ID from
physical-register fields or from a source name.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
import hashlib
import json
import math
import os
from pathlib import Path
import re
import sys
import unicodedata
from typing import Any, Iterable, Mapping, Sequence


SCHEMA_NAME = "mwcc_pcode_varinfo_correlator/v1"
SUPPORTED_ALLOCATOR_SCHEMA = "mwcc_allocator_trace/v1"
SUPPORTED_PCODE_SCHEMAS = {"mwcc_gc26_pcode_trace/v2", "mwcc_gc26_pcode_trace/v3"}
AUTHENTICATED_VREG_STATUSES = {"AUTHENTICATED", "EXACT", "PROVEN"}
SHA256_PATTERN = re.compile(r"^[0-9a-fA-F]{64}$")
VREG_PATTERN = re.compile(r"^[rf][0-9]+$")
# The v3 producer reserves r0..r31/f0..f31 for physical homes.  A direct v3
# ownership row is a virtual-register identity and therefore must be in the
# producer's virtual range; keeping this stricter than the v2 parser also
# prevents forged physical-register values from becoming ownership evidence.
# Physical homes occupy r0..r31/f0..f31 in the v3 producer.  The first
# virtual register is therefore 32; do not let r30/r31 (or a bool/int coercion)
# become an authenticated ownership edge.
V3_VREG_PATTERN = re.compile(r"^[rf](?:3[2-9]|[4-9][0-9]|[1-9][0-9]{2,})$")
V3_SUCCESS_STATUSES = {"EXACT", "CAPTURED", "SUCCESS", "COMPLETE", "OK"}


def _report_hash(value: Mapping[str, Any]) -> str:
    unsigned = {key: item for key, item in value.items() if key != "report_sha256"}
    payload = json.dumps(
        unsigned,
        ensure_ascii=True,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _seal_report(value: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result["report_sha256"] = _report_hash(result)
    return result
V3_TOP_LEVEL_KEYS = {
    "schema",
    "tool_version",
    "status",
    "diagnostic_only",
    "board_admission",
    "exactness_claim",
    "function",
    "pcode_status",
    "stages",
    "provenance",
    "authentication",
    "frontend_join",
    "source_inventory",
    "ownership",
    "liveness",
    "first_unobservable_edge",
    "limitations",
}
V3_AUTH_KEYS = {
    "status",
    "reason",
    "manifest_sha256",
    "compiler_path",
    "compiler_sha256",
    "compiler_size",
    "source_path",
    "source_sha256",
    "source_size",
    "ownership_path",
    "ownership_sha256",
    "ownership_size",
    "ownership_events_path",
    "ownership_events_sha256",
    "ownership_events_size",
    "function",
    "cwd",
    "argv",
    "session_id",
    "process_id",
    # Compatibility fields emitted by the v2-compatible authentication
    # envelope.  They remain evidence only; the external trust root is the
    # authority for every artifact identity.
    "source_hash_authenticated",
    "source_provenance",
    "compiler_provenance",
    "ownership_provenance",
    "manifest_path",
    "normalized_v3_path",
    "normalized_v3_sha256",
    "normalized_v3_size",
    "pcode_v3_path",
    "pcode_v3_sha256",
    "pcode_v3_size",
    "pcode_path",
    "pcode_sha256",
    "pcode_size",
}
V3_JOIN_KEYS = {"status", "reason", "session", "direct_object_vregs"}
V3_DIRECT_ROW_KEYS = {
    "object_ordinal",
    "kind",
    "name",
    "datatype",
    "type_code",
    "size",
    "vreg_id",
    "status",
}
V3_SOURCE_INVENTORY_KEYS = {"status", "reason", "locals", "arguments"}
V3_SOURCE_ROW_KEYS = {
    "kind",
    "ordinal",
    "compiler_list_order",
    "name",
    "datatype",
    "type_code",
    "size",
    "vreg_ids",
    "vreg_status",
}
V3_SESSION_KEYS = {
    "session_id",
    "process_id",
    "function",
    "source",
    "compiler",
    "argv",
    "cwd",
    "snapshot_complete",
    "source_capture_stage",
    "regalloc_capture_banks",
}
V3_STAGE_KEYS = {
    "schema",
    "compiler_version",
    "stage",
    "blocks",
    "instructions",
    "limitations",
}
V3_STAGE_IDENTITY_KEYS = {"number", "name"}
V3_BLOCK_KEYS = {
    "id",
    "order",
    "successors",
    "predecessors",
    "labels",
    "loop_weight",
    "block_flags",
}
V3_OPERAND_KEYS = {
    "index",
    "kind",
    "effect",
    "register_class",
    "register",
    "virtual_register",
    "immediate",
    "target_block",
    "object_reference",
}
V3_INSTRUCTION_KEYS = {
    "order",
    "block",
    "block_order",
    "instruction_order",
    "opcode",
    "mnemonic",
    "argc",
    "useID",
    "defID",
    "flags",
    "sourceoffset",
    "operands",
}
V3_LIVENESS_KEYS = {"status", "label", "reason", "blocks"}
V3_LIVENESS_BLOCK_KEYS = {"id", "live_in", "live_out"}
V3_EVIDENCE_KEYS = {"status", "value"}
V3_PROVENANCE_KEYS = {
    "manifest_sha256",
    "source_sha256",
    "compiler_sha256",
    "ownership_sha256",
    "ownership_events_sha256",
    "source_provenance",
    "compiler_provenance",
    "ownership_provenance",
}
V3_STAGE_NAMES = {
    "backend-00-initial-code.pcode.json",
    "backend-01-before-regalloc.pcode.json",
    "backend-02-after-regalloc.pcode.json",
}

# Values crossing the correlator boundary are evidence, not a debugger dump.
# Keep the address spelling deliberately narrow: all raw address/pointer
# spellings used by the producer are hexadecimal, while SHA-256 fields are
# handled separately by the schema validator.  A path may legitimately carry
# a directory/file component such as ``0x0``; only the explicitly anchored
# path fields below are allowed to do so.
RAW_POINTER_TEXT_PATTERN = re.compile(r"0[xX][0-9a-fA-F]+")
# A bare address is not allowed to rely on a ``0x`` prefix.  Runtime traces
# commonly expose Wii addresses as eight-digit hexadecimal strings and some
# transports stringify native addresses as decimal.  Keep this deliberately
# conservative (digit-leading, six or more characters) so ordinary source
# identifiers and authenticated SHA-256 values are not classified as
# addresses; digest fields are exempted explicitly below.
RAW_BARE_ADDRESS_TEXT_PATTERN = re.compile(r"(?<![A-Za-z0-9_.])[0-9][0-9A-Fa-f]{5,}(?![A-Za-z0-9_.])")
# Capture-local identifiers deliberately contain long hexadecimal components,
# but they are closed, typed identifiers rather than serialized native
# addresses.  Permit them only under their schema-known field names and only
# when the complete value matches the producer's fail-closed token grammar.
SESSION_ID_PATTERN = re.compile(r"session-[0-9a-f]{16}\Z")
OBJECT_TOKEN_PATTERN = re.compile(
    r"(?:local|argument)-session-[0-9a-f]{16}-[0-9]{6}\Z"
)
PCODE_TOKEN_PATTERN = re.compile(r"pcode-session-[0-9a-f]{16}-[0-9]{6}\Z")
IG_TOKEN_PATTERN = re.compile(
    r"(?:hidden-)?ig-session-[0-9a-f]{16}-[0-9]{6}\Z"
)
EVENT_ID_PATTERN = re.compile(r"session-[0-9a-f]{16}-e[0-9]{6}\Z")
GENERATION_ID_PATTERN = re.compile(r"(?:object|varinfo)-generation-[0-9]{6}\Z")
OPAQUE_CAPTURE_ID_PATTERNS = {
    "session_id": SESSION_ID_PATTERN,
    "object_token": OBJECT_TOKEN_PATTERN,
    "source_object_token": OBJECT_TOKEN_PATTERN,
    "destination_object_token": OBJECT_TOKEN_PATTERN,
    "pcode_token": PCODE_TOKEN_PATTERN,
    "pcode_tokens": PCODE_TOKEN_PATTERN,
    "ig_token": IG_TOKEN_PATTERN,
    "hidden_owner_token": IG_TOKEN_PATTERN,
    "event_id": EVENT_ID_PATTERN,
    "event_ids": EVENT_ID_PATTERN,
    "evidence_event_ids": EVENT_ID_PATTERN,
    "generation_id": GENERATION_ID_PATTERN,
    "generation_ids": GENERATION_ID_PATTERN,
    "object_generation_id": GENERATION_ID_PATTERN,
    "varinfo_generation_id": GENERATION_ID_PATTERN,
    # Authenticated four-byte instruction encoding, cross-checked against the
    # integer PPC word by the machine-event validator.
    "ppc_bytes": re.compile(r"[0-9a-f]{8}\Z"),
}
# Only a reviewed C lvalue address-of expression is allowed in the two source
# chronology argument paths.  This is intentionally narrower than a general
# C expression: it accepts ``&masuPos`` and ``&savedPos[playerNo]`` (plus
# member/index suffixes), but not ``&&x``, bitwise ``a & b``, pointer arithmetic,
# calls, casts, or hexadecimal/decimal address payloads.
SOURCE_ADDRESS_OF_EXPRESSION_PATTERN = re.compile(
    r"\s*&\s*[A-Za-z_][A-Za-z0-9_]*"
    r"(?:\s*(?:->|\.)\s*[A-Za-z_][A-Za-z0-9_]*"
    r"|\s*\[\s*(?:[A-Za-z_][A-Za-z0-9_]*|[0-9]+)\s*\])*\s*"
    r"$"
)
POINTER_KEY_PARTS = frozenset(
    {
        "address",
        "addresses",
        "addr",
        "addrs",
        "pointer",
        "pointers",
        "ptr",
        "ptrs",
        "raw_pointer",
        "raw_pointers",
        "raw_address",
        "raw_addresses",
        "object_pointer",
        "object_pointers",
        "varinfo_pointer",
        "varinfo_pointers",
        "ig_node",
        "ig_nodes",
        "ig_node_id",
        "ig_node_ids",
        "ig_node_pointer",
        "ig_node_pointers",
        "ig_pointer",
        "ig_pointers",
        "thread_id",
        "process_id",
    }
)
EXTERNALLY_ANCHORED_PATH_KEYS = frozenset(
    {
        # Authentication artifact descriptors.
        "manifest_path",
        "source_path",
        "compiler_path",
        "ownership_path",
        "ownership_events_path",
        "pcode_path",
        "normalized_v3_path",
        # The same immutable paths are repeated in the authenticated session
        # packet under these schema-known names.
        "source",
        "compiler",
        "cwd",
    }
)
# These causal-map artifact records intentionally omit ``size`` because their
# payload digest is already carried by the authenticated producer packet.  A
# bare ``path`` key must not become a general escape hatch, so admit it only at
# these exact schema locations and only when its containing mapping has the
# complete expected shape.
SCHEMA_ANCHORED_PATH_RECORDS = {
    ("capture", "path"): frozenset({"path", "sha256"}),
    ("source_span_manifest", "path"): frozenset({"path", "sha256"}),
    ("frontend_chronology", "path"): frozenset(
        {"status", "path", "sha256", "packet_sha256", "events"}
    ),
    ("source_evaluation_chronology", "source", "path"): frozenset(
        {"path", "sha256", "function", "function_lines", "body_lines"}
    ),
}
TEXT_REASON_KEYS = frozenset({"reason", "reasons"})
TEXT_LIMITATION_KEYS = frozenset({"limitation", "limitations"})


def _source_address_of_expression_path(path: tuple[str, ...]) -> bool:
    """Return whether *path* is a reviewed source-expression container.

    Source chronology is the only place where a unary ``&`` is source syntax
    rather than a serialized runtime address.  Keep the allowance tied to the
    authenticated causal-map field names; generic ``arguments`` or ``rhs``
    keys are not sufficient because they could carry arbitrary payload text.
    """

    return path[-3:] in {
        ("source_evaluation_chronology", "calls", "arguments"),
        ("source_evaluation_chronology", "assignments", "rhs"),
        ("joined_objects", "call_return_chronology", "arguments"),
    } or path[-2:] == ("call_return_chronology", "arguments")


def _reviewed_source_text_path(path: tuple[str, ...]) -> bool:
    """Identify exact donor_cfg source slices, never generic free text."""

    return path[-4:] in {
        ("source_evaluation_chronology", "assignments", "span", "snippet"),
        ("source_evaluation_chronology", "calls", "span", "snippet"),
        ("source_evaluation_chronology", "control_events", "span", "snippet"),
    } or path[-3:] == (
        "source_evaluation_chronology", "control_events", "condition"
    )


@dataclass(frozen=True)
class ExternalTrustRoot:
    """Out-of-band trust anchors required for an authenticated v3 join.

    The normalized producer's manifest is evidence, never an authority for
    these values.  The allocator/PCode fields extend the learning producer's
    artifact root so this consumer can bind every raw input to exact bytes.
    """

    manifest_path: str | Path | None = None
    manifest_sha256: str | None = None
    manifest_size: int | None = None
    source_path: str | Path | None = None
    source_sha256: str | None = None
    source_size: int | None = None
    compiler_path: str | Path | None = None
    compiler_sha256: str | None = None
    compiler_size: int | None = None
    ownership_path: str | Path | None = None
    ownership_sha256: str | None = None
    ownership_size: int | None = None
    ownership_events_path: str | Path | None = None
    ownership_events_sha256: str | None = None
    ownership_events_size: int | None = None
    allocator_path: str | Path | None = None
    allocator_sha256: str | None = None
    allocator_size: int | None = None
    pcode_path: str | Path | None = None
    pcode_sha256: str | None = None
    pcode_size: int | None = None
    pcode_v3_path: str | Path | None = None
    pcode_v3_sha256: str | None = None
    pcode_v3_size: int | None = None
    # ``normalized_v3_*`` is the explicit receipt spelling used by the v3
    # capture producer.  Keep ``pcode_v3_*`` as a source-compatible alias for
    # callers of the original correlator API; coalescing below rejects any
    # disagreement instead of silently selecting one authority.
    normalized_v3_path: str | Path | None = None
    normalized_v3_sha256: str | None = None
    normalized_v3_size: int | None = None
    function: str | None = None
    cwd: str | Path | None = None
    argv: tuple[str, ...] | list[str] | None = None
    session_id: str | None = None
    process_id: int | None = None


class CorrelatorError(ValueError):
    """Raised when a trace cannot be safely audited."""


def _strict_json_key(key: Any, where: str) -> str:
    if not isinstance(key, str):
        raise CorrelatorError(f"{where}: JSON object keys must be strings")
    # Every accepted producer schema uses ASCII field names.  Rejecting other
    # code points closes the remaining visually-confusable spellings (for
    # example Cyrillic ``а`` versus ASCII ``a``) that Unicode normalization
    # alone cannot identify safely.
    if not key.isascii():
        raise CorrelatorError(f"{where}: non-ASCII/confusable JSON key is forbidden")
    # Reject compatibility/canonicalization variants instead of allowing a
    # visually equivalent key to bypass a closed schema or collide later.
    if unicodedata.normalize("NFC", key) != key:
        raise CorrelatorError(f"{where}: Unicode-normalized JSON key is forbidden")
    if unicodedata.normalize("NFKC", key) != key:
        raise CorrelatorError(f"{where}: confusable JSON key is forbidden")
    return key


def _duplicate_key_object(pairs: list[tuple[Any, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    normalized: dict[str, str] = {}
    for index, (raw_key, value) in enumerate(pairs):
        key = _strict_json_key(raw_key, f"JSON object key {index}")
        folded = key.casefold()
        if key in result:
            raise CorrelatorError(f"duplicate JSON key {key!r}")
        if folded in normalized:
            raise CorrelatorError(
                f"confusable/case-folded JSON keys {normalized[folded]!r} and {key!r}"
            )
        result[key] = value
        normalized[folded] = key
    return result


def _reject_json_constant(value: str) -> Any:
    raise CorrelatorError(f"non-finite JSON number {value!r} is forbidden")


def _strict_json_loads(raw: bytes | str, *, label: str) -> Any:
    try:
        value = json.loads(
            raw,
            object_pairs_hook=_duplicate_key_object,
            parse_constant=_reject_json_constant,
        )
    except CorrelatorError:
        raise
    except (UnicodeDecodeError, json.JSONDecodeError, TypeError, ValueError) as error:
        raise CorrelatorError(f"{label}: {error}") from error
    return value


def _strict_json_file(path: Path) -> Any:
    try:
        return _strict_json_loads(path.read_bytes(), label=str(path))
    except OSError as error:
        raise CorrelatorError(f"cannot read {path}: {error}") from error


def _reject_nonfinite(value: Any, where: str = "$") -> None:
    if isinstance(value, float):
        if not math.isfinite(value):
            raise CorrelatorError(f"{where}: non-finite number is forbidden")
    elif isinstance(value, Mapping):
        for key, child in value.items():
            _strict_json_key(key, f"{where} key")
            _reject_nonfinite(child, f"{where}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            _reject_nonfinite(child, f"{where}[{index}]")


def _pointer_key_name(key: str) -> bool:
    """Return whether *key* can expose a native pointer/address identity.

    Producer revisions have used both ``ig_node`` and variants such as
    ``ig_node_id``/``igNodePointer``.  Compare a separator-normalized spelling
    and inspect tokens so a new suffix or a camel-case variant cannot bypass
    the pointer-free boundary.
    """

    normalized = re.sub(r"[^a-z0-9]+", "_", key.casefold()).strip("_")
    compact = normalized.replace("_", "")
    if normalized in POINTER_KEY_PARTS or compact in {
        item.replace("_", "") for item in POINTER_KEY_PARTS
    }:
        return True
    tokens = set(normalized.split("_"))
    if "pointer" in tokens or "pointers" in tokens or "ptr" in tokens:
        return True
    if "address" in tokens or "addresses" in tokens or "addr" in tokens:
        return True
    # ``ig_node`` is an opaque implementation label, not a source identity.
    # Cover suffixes/prefixes and camel-case spellings (``igNodeRef``).
    if compact.startswith("ignode") or compact.endswith("ignode"):
        return True
    if "ignode" in compact:
        return True
    return False


def _absolute_path_text(value: str) -> bool:
    """Recognize path-shaped argv elements without interpreting their bytes."""

    return bool(
        re.match(r"^[A-Za-z]:[\\/]", value)
        or value.startswith("/")
        or value.startswith("\\\\")
    )


def _reject_pointer_material(
    value: Any,
    where: str = "$",
    *,
    _path_allowed: bool = False,
    _argv_element: bool = False,
    _path: tuple[str, ...] = (),
    _field_key: str | None = None,
) -> None:
    """Reject raw pointer keys, values, and free-text address spellings.

    This is intentionally independent of the closed-schema validator.  The
    latter rejects unknown fields, while this walk also protects known free
    text fields (``reason``/``limitations``) and nested values.  Errors name
    only a schema location; the offending value is never copied into a report
    or exception string.
    """

    if isinstance(value, Mapping):
        descriptor_path_field = (
            set(value) == {"path", "sha256", "size"}
            and isinstance(value.get("path"), str)
            and isinstance(value.get("sha256"), str)
            and isinstance(value.get("size"), int)
            and not isinstance(value.get("size"), bool)
        )
        for raw_key, child in value.items():
            if not isinstance(raw_key, str):
                raise CorrelatorError(f"{where}: JSON object keys must be strings")
            key = raw_key.casefold()
            if _pointer_key_name(raw_key) and key not in {
                "process_id",
                "thread_id",
                # A closed machine-emission structure containing a stack
                # base-register identity and signed stack offset.  It never
                # contains a native address and is validated before this
                # defense-in-depth walk.
                "address_definition",
            }:
                raise CorrelatorError(f"{where}: raw pointer/address key is forbidden")
            if key in TEXT_REASON_KEYS:
                if child is not None and not isinstance(child, str):
                    raise CorrelatorError(f"{where}.{raw_key}: reason must be a string or null")
            elif key in TEXT_LIMITATION_KEYS:
                if child is not None and (
                    not isinstance(child, list)
                    or any(not isinstance(item, str) for item in child)
                ):
                    raise CorrelatorError(f"{where}.{raw_key}: limitations must be a string list or null")
            child_path = (*_path, key)
            expected_record_keys = SCHEMA_ANCHORED_PATH_RECORDS.get(child_path)
            schema_path_field = (
                key == "path"
                and expected_record_keys is not None
                and set(value) == expected_record_keys
            )
            child_path_allowed = key in EXTERNALLY_ANCHORED_PATH_KEYS or (
                key == "path" and (descriptor_path_field or schema_path_field)
            )
            _reject_pointer_material(
                child,
                f"{where}.{raw_key}",
                _path_allowed=child_path_allowed,
                _argv_element=(key == "argv"),
                _path=child_path,
                _field_key=key,
            )
        return
    if isinstance(value, list):
        for index, child in enumerate(value):
            _reject_pointer_material(
                child,
                f"{where}[{index}]",
                _path_allowed=_path_allowed,
                _argv_element=_argv_element,
                _path=_path,
                _field_key=_field_key,
            )
        return
    if not isinstance(value, str):
        return

    # Source chronology call arguments and assignment RHS values are reviewed
    # C source text, not a debugger transport.  Permit only the narrow
    # address-of grammar and only at authenticated source-expression paths.
    # Ordinary arguments such as ``playerNo`` or ``100.0`` continue through
    # the normal text checks.
    if "&" in value:
        if _source_address_of_expression_path(_path):
            if SOURCE_ADDRESS_OF_EXPRESSION_PATTERN.fullmatch(value) is not None:
                return
            raise CorrelatorError(f"{where}: raw pointer/address text is forbidden")
        elif _reviewed_source_text_path(_path):
            # The full text is still checked below for hexadecimal/decimal
            # address material.  This exception covers only C operators in an
            # authenticated source slice (``&``, ``&&`` or bitwise ``&``).
            pass
        else:
            raise CorrelatorError(f"{where}: raw pointer/address text is forbidden")

    if _argv_element and _absolute_path_text(value):
        # Exact argv is subsequently compared with the external trust root.
        # Permit a hexadecimal-looking directory component only when the
        # complete element is path-shaped; bare address text or flag payloads
        # remain forbidden.
        return

    if _path_allowed:
        # A path field is allowed to contain an address-looking component, but
        # a bare address is still not a path and must not be smuggled through
        # the path exception.
        if _absolute_path_text(value):
            return

    # SHA-256 values are authenticated digests, not runtime addresses.  They
    # are accepted only in explicitly named digest fields; all other bare
    # digit-leading address spellings remain fail-closed.
    opaque_pattern = OPAQUE_CAPTURE_ID_PATTERNS.get(_field_key or "")
    if opaque_pattern is not None and opaque_pattern.fullmatch(value) is not None:
        return
    digest_field = bool(_field_key and _field_key.casefold().endswith("sha256"))
    if not digest_field and (
        RAW_POINTER_TEXT_PATTERN.search(value)
        or RAW_BARE_ADDRESS_TEXT_PATTERN.search(value)
    ):
        raise CorrelatorError(f"{where}: raw pointer/address text is forbidden")


def _closed_keys(value: Any, allowed: set[str], where: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise CorrelatorError(f"{where}: expected an object")
    seen: dict[str, str] = {}
    for raw_key in value:
        key = _strict_json_key(raw_key, f"{where} key")
        folded = key.casefold()
        if folded in seen:
            raise CorrelatorError(
                f"{where}: confusable/case-folded keys {seen[folded]!r} and {key!r}"
            )
        seen[folded] = key
        if key not in allowed:
            raise CorrelatorError(f"{where}: unknown key {key!r}")
    return value


def _validate_v3_closed_shape(trace: Mapping[str, Any]) -> list[str]:
    """Reject unknown/confusable keys recursively in normalized v3 input."""

    errors: list[str] = []

    def check(value: Any, allowed: set[str], where: str) -> Mapping[str, Any] | None:
        try:
            return _closed_keys(value, allowed, where)
        except CorrelatorError as error:
            errors.append(str(error))
            return None

    def integer(value: Any, where: str, *, allow_none: bool = True) -> None:
        if allow_none and value is None:
            return
        if isinstance(value, bool) or not isinstance(value, int):
            errors.append(f"{where}: expected an integer")

    def strings(value: Any, where: str, *, allow_none: bool = True) -> None:
        if allow_none and value is None:
            return
        if not isinstance(value, list) or any(not isinstance(item, str) or not item for item in value):
            errors.append(f"{where}: expected a non-empty string list")

    def evidence(value: Any, where: str) -> None:
        # Normalized v3 fields normally carry an explicit {status,value}
        # envelope.  A few early diagnostic packets emitted a scalar directly;
        # retain that read-only compatibility while still validating wrapped
        # values and rejecting non-finite numbers recursively.
        if not isinstance(value, Mapping):
            if isinstance(value, (str, int, bool)) or value is None:
                return
            errors.append(f"{where}: expected an evidence object or scalar")
            return
        item = check(value, V3_EVIDENCE_KEYS, where)
        if item is None:
            return
        status = item.get("status")
        if status not in {"EXACT", "UNKNOWN"}:
            errors.append(f"{where}.status: invalid v3 evidence status")
        elif status == "UNKNOWN" and item.get("value") is not None:
            errors.append(f"{where}: UNKNOWN evidence must carry null value")

    def session(value: Any, where: str) -> None:
        item = check(value, V3_SESSION_KEYS, where)
        if item is None:
            return
        for key in ("session_id", "function", "source", "compiler", "cwd", "source_capture_stage"):
            if key in item and (not isinstance(item.get(key), str) or not item.get(key)):
                errors.append(f"{where}.{key}: expected a non-empty string")
        if "process_id" in item:
            integer(item.get("process_id"), f"{where}.process_id", allow_none=False)
        if "snapshot_complete" in item and not isinstance(item.get("snapshot_complete"), bool):
            errors.append(f"{where}.snapshot_complete: expected a boolean")
        if "argv" in item:
            strings(item.get("argv"), f"{where}.argv", allow_none=False)
        if "regalloc_capture_banks" in item:
            strings(item.get("regalloc_capture_banks"), f"{where}.regalloc_capture_banks", allow_none=False)

    def direct_row(value: Any, where: str) -> None:
        item = check(value, V3_DIRECT_ROW_KEYS, where)
        if item is None:
            return
        if isinstance(item.get("object_ordinal"), bool) or not isinstance(
            item.get("object_ordinal"), int
        ):
            errors.append(f"{where}.object_ordinal: expected an integer")
        if "kind" in item and (not isinstance(item.get("kind"), str) or item.get("kind") not in {"local", "argument"}):
            errors.append(f"{where}.kind: expected local or argument")
        if "name" in item and (not isinstance(item.get("name"), str) or not item.get("name")):
            errors.append(f"{where}.name: expected a non-empty string")
        for key in ("datatype", "type_code", "size"):
            if key in item:
                integer(item.get(key), f"{where}.{key}")
        if "vreg_id" not in item or not isinstance(item.get("vreg_id"), str) or not item.get("vreg_id"):
            errors.append(f"{where}.vreg_id: expected a non-empty string")
        if item.get("status") != "AUTHENTICATED":
            errors.append(f"{where}.status: expected AUTHENTICATED")

    def join(value: Any, where: str) -> None:
        item = check(value, V3_JOIN_KEYS, where)
        if item is None:
            return
        if item.get("status") not in {"AUTHENTICATED", "UNKNOWN"}:
            errors.append(f"{where}.status: invalid v3 join status")
        if item.get("session") is not None:
            session(item.get("session"), f"{where}.session")
        rows = item.get("direct_object_vregs")
        if not isinstance(rows, list):
            errors.append(f"{where}.direct_object_vregs: expected a list")
        else:
            for index, row in enumerate(rows):
                direct_row(row, f"{where}.direct_object_vregs[{index}]")

    def source_row(value: Any, where: str) -> None:
        item = check(value, V3_SOURCE_ROW_KEYS, where)
        if item is None:
            return
        if "kind" in item and (not isinstance(item.get("kind"), str) or item.get("kind") not in {"local", "argument"}):
            errors.append(f"{where}.kind: expected local or argument")
        if isinstance(item.get("ordinal"), bool) or not isinstance(item.get("ordinal"), int):
            errors.append(f"{where}.ordinal: expected an integer")
        if isinstance(item.get("compiler_list_order"), bool) or not isinstance(item.get("compiler_list_order"), int):
            errors.append(f"{where}.compiler_list_order: expected an integer")
        if not isinstance(item.get("name"), str) or not item.get("name"):
            errors.append(f"{where}.name: expected a non-empty string")
        for key in ("datatype", "size"):
            value = item.get(key)
            if value is None or isinstance(value, bool) or not isinstance(value, int):
                errors.append(f"{where}.{key}: expected an integer")
        if "type_code" in item:
            value = item.get("type_code")
            if isinstance(value, bool) or not isinstance(value, int):
                errors.append(f"{where}.type_code: expected an integer")
        if not isinstance(item.get("vreg_ids"), list):
            errors.append(f"{where}.vreg_ids: expected a list")
        if item.get("vreg_status") not in {"AUTHENTICATED", "UNKNOWN"}:
            errors.append(f"{where}.vreg_status: invalid v3 ownership status")

    def stage(value: Any, where: str) -> None:
        item = check(value, V3_STAGE_KEYS, where)
        if item is None:
            return
        stage_id = item.get("stage")
        if stage_id is not None:
            stage_map = check(stage_id, V3_STAGE_IDENTITY_KEYS, f"{where}.stage")
            if stage_map is not None:
                integer(stage_map.get("number"), f"{where}.stage.number")
                if "name" in stage_map and (not isinstance(stage_map.get("name"), str) or not stage_map.get("name")):
                    errors.append(f"{where}.stage.name: expected a non-empty string")
        blocks = item.get("blocks")
        if blocks is not None:
            if not isinstance(blocks, list):
                errors.append(f"{where}.blocks: expected a list")
            else:
                for index, block in enumerate(blocks):
                    block_map = check(block, V3_BLOCK_KEYS, f"{where}.blocks[{index}]")
                    if block_map is None:
                        continue
                    integer(block_map.get("id"), f"{where}.blocks[{index}].id")
                    integer(block_map.get("order"), f"{where}.blocks[{index}].order")
                    for key in ("successors", "predecessors", "labels", "block_flags"):
                        if key in block_map and not isinstance(block_map.get(key), list):
                            errors.append(f"{where}.blocks[{index}].{key}: expected a list")
                    if "loop_weight" in block_map and (
                        isinstance(block_map.get("loop_weight"), bool)
                        or not isinstance(block_map.get("loop_weight"), (int, float))
                    ):
                        errors.append(f"{where}.blocks[{index}].loop_weight: expected a number")
        instructions = item.get("instructions")
        if instructions is not None:
            if not isinstance(instructions, list):
                errors.append(f"{where}.instructions: expected a list")
            else:
                for index, instruction in enumerate(instructions):
                    normalized = check(
                        instruction,
                        V3_INSTRUCTION_KEYS,
                        f"{where}.instructions[{index}]",
                    )
                    if normalized is None:
                        continue
                    for field in (
                        "opcode",
                        "mnemonic",
                        "argc",
                        "useID",
                        "defID",
                        "flags",
                        "sourceoffset",
                    ):
                        if field in normalized:
                            evidence(normalized[field], f"{where}.instructions[{index}].{field}")
                    operands = normalized.get("operands")
                    if operands is not None and not isinstance(operands, list):
                        errors.append(f"{where}.instructions[{index}].operands: expected a list")
                    if isinstance(operands, list):
                        for operand_index, operand in enumerate(operands):
                            operand_map = check(
                                operand,
                                V3_OPERAND_KEYS,
                                f"{where}.instructions[{index}].operands[{operand_index}]",
                            )
                            if operand_map is None:
                                continue
                            for field in (
                                "effect",
                                "register_class",
                                "register",
                                "virtual_register",
                                "immediate",
                                "target_block",
                                "object_reference",
                            ):
                                if field in operand_map:
                                    evidence(
                                        operand_map[field],
                                        f"{where}.instructions[{index}].operands[{operand_index}].{field}",
                                    )

    top = check(trace, V3_TOP_LEVEL_KEYS, "pcode.v3")
    if top is None:
        return errors
    if top.get("status") != "UNKNOWN":
        errors.append("pcode.v3.status: expected UNKNOWN")
    if top.get("pcode_status") is not None and top.get("pcode_status") not in {"EXACT", "UNKNOWN"}:
        errors.append("pcode.v3.pcode_status: invalid status")
    if "diagnostic_only" in top and top.get("diagnostic_only") is not True:
        errors.append("pcode.v3.diagnostic_only: expected true")
    for key in ("board_admission", "exactness_claim", "function"):
        if key in top and (not isinstance(top.get(key), str) or not top.get(key)):
            errors.append(f"pcode.v3.{key}: expected a non-empty string")
    if "limitations" in top:
        strings(top.get("limitations"), "pcode.v3.limitations", allow_none=False)
    provenance = top.get("provenance")
    if provenance is not None:
        provenance_map = check(provenance, V3_PROVENANCE_KEYS, "pcode.v3.provenance")
        if provenance_map is not None:
            for key, value in provenance_map.items():
                if key.endswith("sha256") and (not isinstance(value, str) or SHA256_PATTERN.fullmatch(value) is None):
                    errors.append(f"pcode.v3.provenance.{key}: expected a SHA-256 string")
                elif key.endswith("provenance") and (not isinstance(value, str) or not value):
                    errors.append(f"pcode.v3.provenance.{key}: expected a non-empty string")
    auth = top.get("authentication")
    if auth is not None:
        auth_map = check(auth, V3_AUTH_KEYS, "pcode.v3.authentication")
        if auth_map is not None:
            if auth_map.get("status") not in {"AUTHENTICATED", "UNKNOWN"}:
                errors.append("pcode.v3.authentication.status: invalid status")
            for key in ("compiler_path", "source_path", "ownership_path", "ownership_events_path", "manifest_path", "normalized_v3_path", "pcode_v3_path", "pcode_path", "function", "cwd", "session_id"):
                if key in auth_map and (not isinstance(auth_map.get(key), str) or not auth_map.get(key)):
                    errors.append(f"pcode.v3.authentication.{key}: expected a non-empty string")
            for key in ("compiler_size", "source_size", "ownership_size", "ownership_events_size", "normalized_v3_size", "pcode_v3_size", "pcode_size", "process_id"):
                if key in auth_map:
                    integer(auth_map.get(key), f"pcode.v3.authentication.{key}", allow_none=False)
            for key in ("manifest_sha256", "compiler_sha256", "source_sha256", "ownership_sha256", "ownership_events_sha256", "normalized_v3_sha256", "pcode_v3_sha256", "pcode_sha256"):
                if key in auth_map and (not isinstance(auth_map.get(key), str) or SHA256_PATTERN.fullmatch(auth_map.get(key)) is None):
                    errors.append(f"pcode.v3.authentication.{key}: expected a SHA-256 string")
            if "argv" in auth_map:
                strings(auth_map.get("argv"), "pcode.v3.authentication.argv", allow_none=False)
            if "source_hash_authenticated" in auth_map and not isinstance(auth_map.get("source_hash_authenticated"), bool):
                errors.append("pcode.v3.authentication.source_hash_authenticated: expected a boolean")
    inventory = top.get("source_inventory")
    if inventory is not None:
        inventory_map = check(inventory, V3_SOURCE_INVENTORY_KEYS, "pcode.v3.source_inventory")
        if inventory_map is not None:
            if inventory_map.get("status") not in {"CAPTURED", "UNKNOWN"}:
                errors.append("pcode.v3.source_inventory.status: invalid status")
            for container in ("locals", "arguments"):
                rows = inventory_map.get(container)
                if not isinstance(rows, list):
                    errors.append(f"pcode.v3.source_inventory.{container}: expected a list")
                else:
                    for index, row in enumerate(rows):
                        source_row(row, f"pcode.v3.source_inventory.{container}[{index}]")
    for label in ("frontend_join", "ownership"):
        if label in top:
            join(top.get(label), f"pcode.v3.{label}")
    stages = top.get("stages")
    if isinstance(stages, Mapping):
        for key, value in stages.items():
            if key not in V3_STAGE_NAMES:
                errors.append(f"pcode.v3.stages: unknown stage {key!r}")
            stage(value, f"pcode.v3.stages.{key}")
    elif stages is not None:
        errors.append("pcode.v3.stages: expected an object")
    liveness = top.get("liveness")
    if liveness is not None:
        liveness_map = check(liveness, V3_LIVENESS_KEYS, "pcode.v3.liveness")
        if liveness_map is not None:
            if liveness_map.get("status") != "UNKNOWN":
                errors.append("pcode.v3.liveness.status: expected UNKNOWN")
            blocks = liveness_map.get("blocks")
            if not isinstance(blocks, list):
                errors.append("pcode.v3.liveness.blocks: expected a list")
            else:
                for index, block in enumerate(blocks):
                    block_map = check(block, V3_LIVENESS_BLOCK_KEYS, f"pcode.v3.liveness.blocks[{index}]")
                    if block_map is not None:
                        for key in ("live_in", "live_out"):
                            if not isinstance(block_map.get(key), list):
                                errors.append(f"pcode.v3.liveness.blocks[{index}].{key}: expected a list")
    return sorted(set(errors))


def _mapping(value: Any, where: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise CorrelatorError(f"{where}: expected an object")
    return value


def _nonempty_string(value: Any, where: str) -> str:
    if not isinstance(value, str) or not value:
        raise CorrelatorError(f"{where}: expected a non-empty string")
    return value


def _vreg_id(value: Any, where: str) -> str:
    value = _nonempty_string(value, where)
    if VREG_PATTERN.fullmatch(value) is None:
        raise CorrelatorError(f"{where}: expected canonical r<number>/f<number> vreg ID")
    return value


def _integer(value: Any, where: str, *, allow_none: bool = False) -> int | None:
    if allow_none and value is None:
        return None
    if isinstance(value, bool) or not isinstance(value, int):
        raise CorrelatorError(f"{where}: expected an integer")
    return value


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as error:
        raise CorrelatorError(f"cannot read {path}: {error}") from error
    return digest.hexdigest()


def _canonical_external_path(value: str | Path | None, label: str) -> Path:
    if isinstance(value, Path):
        path = value
    elif isinstance(value, str) and value:
        path = Path(value)
    else:
        raise CorrelatorError(f"external trust root: missing {label}")
    # ``Path`` normalizes ``.`` components on Windows during construction, so
    # inspect the caller's spelling before relying on ``resolve``.  This keeps
    # a path such as ``capture/./pcode.json`` from becoming an accepted alias
    # of the anchored canonical path (the distinction matters even when both
    # names resolve to the same inode).
    raw_path = str(value)
    components = re.split(r"[\\/]", raw_path)
    if "." in components or ".." in components:
        raise CorrelatorError(f"external trust root: {label} is not canonical")
    if len(raw_path) > 1 and raw_path.endswith(("/", "\\")):
        raise CorrelatorError(f"external trust root: {label} is not canonical")
    if not path.is_absolute():
        raise CorrelatorError(f"external trust root: {label} must be absolute")
    canonical = path.resolve(strict=False)
    if os.path.normcase(str(path)) != os.path.normcase(str(canonical)):
        raise CorrelatorError(f"external trust root: {label} is not canonical")
    if path.is_symlink():
        raise CorrelatorError(f"external trust root: {label} must not be a symlink")
    return canonical


def _file_anchor(
    path: str | Path | None,
    sha256: str | None,
    size: int | None,
    label: str,
) -> tuple[Path, tuple[int, int] | None]:
    canonical = _canonical_external_path(path, label)
    if (
        not isinstance(sha256, str)
        or SHA256_PATTERN.fullmatch(sha256) is None
        or sha256 != sha256.lower()
    ):
        raise CorrelatorError(f"external trust root: invalid {label} SHA-256")
    if isinstance(size, bool) or not isinstance(size, int) or size < 0:
        raise CorrelatorError(f"external trust root: invalid {label} size")
    try:
        stat = canonical.stat()
    except OSError as error:
        raise CorrelatorError(f"external trust root: cannot stat {label}: {error}") from error
    if not canonical.is_file():
        raise CorrelatorError(f"external trust root: {label} is not a file")
    if getattr(stat, "st_nlink", 1) > 1:
        raise CorrelatorError(f"external trust root: {label} is a hard-link alias")
    observed_size = stat.st_size
    observed_sha = _sha256(canonical)
    if observed_size != size:
        raise CorrelatorError(f"external trust root: {label} size mismatch")
    if observed_sha.lower() != sha256.lower():
        raise CorrelatorError(f"external trust root: {label} SHA-256 mismatch")
    identity = (int(getattr(stat, "st_dev", 0)), int(getattr(stat, "st_ino", 0)))
    return canonical, identity if identity != (0, 0) else None


def _coalesce_anchor(primary: Any, alias: Any, label: str) -> Any:
    if primary is not None and alias is not None:
        if "path" in label.lower() or label.lower() == "cwd":
            try:
                equal = os.path.normcase(
                    str(_canonical_external_path(primary, label))
                ) == os.path.normcase(str(_canonical_external_path(alias, label)))
            except CorrelatorError:
                equal = primary == alias
        elif "sha" in label.lower():
            equal = (
                isinstance(primary, str)
                and isinstance(alias, str)
                and primary.lower() == alias.lower()
            )
        else:
            equal = primary == alias
        if not equal:
            raise CorrelatorError(f"conflicting external anchor values for {label}")
    return primary if primary is not None else alias


def _coerce_external_trust_root(
    trust_root: ExternalTrustRoot | None,
    *,
    allocator_path: str | Path | None,
    pcode_path: str | Path | None,
    pcode_v3_path: str | Path | None,
    ownership_path: str | Path | None,
    primary_schema: str | None,
    expected_source_sha256: str | None,
    expected_compiler_sha256: str | None,
    expected_allocator_trace_sha256: str | None,
    expected_allocator_trace_size: int | None,
    expected_pcode_trace_sha256: str | None,
    expected_pcode_trace_size: int | None,
    expected_pcode_v3_trace_sha256: str | None,
    expected_pcode_v3_trace_size: int | None,
    expected_ownership_sha256: str | None,
) -> ExternalTrustRoot | None:
    """Merge legacy digest aliases without ever silently choosing conflicts."""

    if trust_root is not None and not isinstance(trust_root, ExternalTrustRoot):
        raise CorrelatorError("external trust root must be an ExternalTrustRoot object")
    root = trust_root or ExternalTrustRoot()

    def coalesce(label: str, *candidates: Any) -> Any:
        result: Any = None
        for candidate in candidates:
            result = _coalesce_anchor(result, candidate, label)
        return result

    allocator_sha = coalesce(
        "allocator SHA-256", root.allocator_sha256, expected_allocator_trace_sha256
    )
    allocator_size = coalesce(
        "allocator size", root.allocator_size, expected_allocator_trace_size
    )
    pcode_sha = coalesce(
        "PCode SHA-256", root.pcode_sha256, expected_pcode_trace_sha256
    )
    pcode_size = coalesce("PCode size", root.pcode_size, expected_pcode_trace_size)

    # There are three public spellings for one normalized-v3 artifact in
    # different producer revisions.  In v3-primary mode all three spellings
    # are aliases of the primary PCode input; in v2-primary mode they describe
    # the distinct normalized-v3 sidecar.  Every supplied spelling must agree
    # on path, digest, and size before any report can be authenticated.
    v3_path = coalesce(
        "normalized v3 path",
        root.pcode_v3_path,
        root.normalized_v3_path,
        pcode_v3_path,
    )
    v3_sha = coalesce(
        "normalized v3 SHA-256",
        root.pcode_v3_sha256,
        root.normalized_v3_sha256,
        expected_pcode_v3_trace_sha256,
    )
    v3_size = coalesce(
        "normalized v3 size",
        root.pcode_v3_size,
        root.normalized_v3_size,
        expected_pcode_v3_trace_size,
    )
    if primary_schema == "mwcc_gc26_pcode_trace/v3":
        # A v3 primary's legacy pcode_v3/normalized spellings are not a second
        # file.  Coalesce them against the primary aliases, including paths
        # and sizes (the old implementation only compared the digest subset).
        primary_path = coalesce("primary PCode path", root.pcode_path, pcode_path)
        primary_sha = coalesce(
            "primary PCode SHA-256", pcode_sha, v3_sha
        )
        primary_size = coalesce("primary PCode size", pcode_size, v3_size)
        # Preserve whether an alias was actually supplied.  A missing alias is
        # not invented here; payload validation can therefore distinguish an
        # omitted compatibility field from an explicitly bound one.
        if v3_path is not None:
            v3_path = coalesce("primary PCode path", primary_path, v3_path)
        if v3_sha is not None:
            v3_sha = coalesce("primary PCode SHA-256", primary_sha, v3_sha)
        if v3_size is not None:
            v3_size = coalesce("primary PCode size", primary_size, v3_size)
        pcode_path_value = primary_path
        pcode_sha_value = primary_sha
        pcode_size_value = primary_size
    else:
        pcode_path_value = coalesce("PCode path", root.pcode_path, pcode_path)
        pcode_sha_value = pcode_sha
        pcode_size_value = pcode_size

    values = {
        "manifest_path": root.manifest_path,
        "manifest_sha256": root.manifest_sha256,
        "manifest_size": root.manifest_size,
        "source_path": root.source_path,
        "source_sha256": coalesce("source SHA-256", root.source_sha256, expected_source_sha256),
        "source_size": root.source_size,
        "compiler_path": root.compiler_path,
        "compiler_sha256": coalesce("compiler SHA-256", root.compiler_sha256, expected_compiler_sha256),
        "compiler_size": root.compiler_size,
        # Keep the trust-root path authoritative, but retain a caller path
        # only when it is equal.  _validate_external_trust_root compares the
        # canonical input path explicitly, including ownership_path.
        "ownership_path": coalesce("ownership path", root.ownership_path, ownership_path),
        "ownership_sha256": coalesce(
            "ownership SHA-256", root.ownership_sha256, expected_ownership_sha256
        ),
        "ownership_size": root.ownership_size,
        "ownership_events_path": root.ownership_events_path,
        "ownership_events_sha256": root.ownership_events_sha256,
        "ownership_events_size": root.ownership_events_size,
        "allocator_path": coalesce("allocator path", root.allocator_path, allocator_path),
        "allocator_sha256": allocator_sha,
        "allocator_size": coalesce("allocator size", root.allocator_size, expected_allocator_trace_size),
        "pcode_path": pcode_path_value,
        "pcode_sha256": pcode_sha_value,
        "pcode_size": pcode_size_value,
        "pcode_v3_path": v3_path,
        "pcode_v3_sha256": v3_sha,
        "pcode_v3_size": v3_size,
        "normalized_v3_path": v3_path,
        "normalized_v3_sha256": v3_sha,
        "normalized_v3_size": v3_size,
        "function": root.function,
        "cwd": root.cwd,
        "argv": root.argv,
        "session_id": root.session_id,
        "process_id": root.process_id,
    }
    if trust_root is None and not any(value is not None for value in values.values()):
        return None
    return ExternalTrustRoot(**values)


def _validate_external_trust_root(
    root: ExternalTrustRoot | None,
    *,
    direct_v3: bool,
    primary_v3: bool,
    allocator_path: str | Path | None,
    pcode_path: str | Path | None,
    pcode_v3_path: str | Path | None,
    ownership_path: str | Path | None,
) -> list[str]:
    """Validate the complete out-of-band root without consulting payload data."""

    if root is None:
        return ["external trust root is required for v3 ownership"] if direct_v3 else []
    errors: list[str] = []
    anchors: list[tuple[str, Path, str, tuple[int, int] | None]] = []

    def add_anchor(label: str, path: Any, sha: Any, size: Any) -> Path | None:
        try:
            canonical, identity = _file_anchor(path, sha, size, label)
        except CorrelatorError as error:
            errors.append(str(error))
            return None
        anchors.append((label, canonical, str(sha).lower(), identity))
        return canonical

    if not direct_v3:
        return errors
    required = [
        ("manifest", root.manifest_path, root.manifest_sha256, root.manifest_size),
        ("source", root.source_path, root.source_sha256, root.source_size),
        ("compiler", root.compiler_path, root.compiler_sha256, root.compiler_size),
        ("ownership", root.ownership_path, root.ownership_sha256, root.ownership_size),
        (
            "ownership events",
            root.ownership_events_path,
            root.ownership_events_sha256,
            root.ownership_events_size,
        ),
        ("allocator", root.allocator_path, root.allocator_sha256, root.allocator_size),
        ("PCode", root.pcode_path, root.pcode_sha256, root.pcode_size),
    ]
    # v3-primary has one normalized v3 input (the primary PCode path).  The
    # separate normalized-v3 anchor is required only when v2 remains primary.
    if not primary_v3:
        required.append(
            ("normalized v3", root.pcode_v3_path, root.pcode_v3_sha256, root.pcode_v3_size)
        )
    for label, path, sha, size in required:
        add_anchor(label, path, sha, size)

    canonical_by_label = {label: path for label, path, _, _ in anchors}
    expected_allocator = canonical_by_label.get("allocator")
    expected_pcode = canonical_by_label.get("PCode")
    expected_v3 = canonical_by_label.get("normalized v3")
    def compare_input(label: str, supplied: Any, expected: Path | None) -> None:
        if supplied is None:
            errors.append(f"external trust root: {label} input path is absent")
            return
        try:
            actual = _canonical_external_path(supplied, f"{label} input path")
        except CorrelatorError as error:
            errors.append(str(error))
            return
        if expected is not None and os.path.normcase(str(actual)) != os.path.normcase(str(expected)):
            errors.append(f"external trust root: {label} path does not match input")

    compare_input("allocator", allocator_path, expected_allocator)
    compare_input("PCode", pcode_path, expected_pcode)
    compare_input("ownership", ownership_path, canonical_by_label.get("ownership"))
    if primary_v3:
        # A separately named pcode_v3 path is an API alias in primary-v3 mode;
        # it cannot introduce a second, downgradeable payload.
        if pcode_v3_path is not None:
            compare_input("normalized v3", pcode_v3_path, expected_pcode)
    else:
        compare_input("normalized v3", pcode_v3_path, expected_v3)

    if root.ownership_path is None:
        errors.append("external trust root: ownership path is mandatory")
    for label, path in (
        ("manifest", canonical_by_label.get("manifest")),
        ("source", canonical_by_label.get("source")),
        ("compiler", canonical_by_label.get("compiler")),
        ("ownership", canonical_by_label.get("ownership")),
        ("ownership events", canonical_by_label.get("ownership events")),
        ("allocator", expected_allocator),
        ("PCode", expected_pcode),
    ):
        if path is None:
            errors.append(f"external trust root: {label} anchor is absent")
    if not primary_v3 and expected_v3 is None:
        errors.append("external trust root: normalized v3 anchor is absent")
    if primary_v3 and root.pcode_v3_path is not None and expected_pcode is not None:
        try:
            alias = _canonical_external_path(root.pcode_v3_path, "normalized v3 alias")
            if os.path.normcase(str(alias)) != os.path.normcase(str(expected_pcode)):
                errors.append("external trust root: normalized v3 alias does not match primary PCode")
        except CorrelatorError as error:
            errors.append(str(error))

    for label, digest in (
        ("manifest", root.manifest_sha256),
        ("source", root.source_sha256),
        ("compiler", root.compiler_sha256),
        ("ownership", root.ownership_sha256),
        ("ownership events", root.ownership_events_sha256),
        ("allocator", root.allocator_sha256),
        ("PCode", root.pcode_sha256),
    ):
        if not isinstance(digest, str) or SHA256_PATTERN.fullmatch(digest) is None or digest != digest.lower():
            errors.append(f"external trust root: {label} SHA-256 must be canonical lowercase hexadecimal")
    if not primary_v3:
        for label, digest in (("normalized v3", root.pcode_v3_sha256), ("normalized v3 alias", root.normalized_v3_sha256)):
            if digest is not None and (
                not isinstance(digest, str)
                or SHA256_PATTERN.fullmatch(digest) is None
                or digest != digest.lower()
            ):
                errors.append(
                    f"external trust root: {label} SHA-256 must be canonical lowercase hexadecimal"
                )
    elif root.pcode_v3_sha256 is not None and (
        not isinstance(root.pcode_v3_sha256, str)
        or SHA256_PATTERN.fullmatch(root.pcode_v3_sha256) is None
        or root.pcode_v3_sha256 != root.pcode_v3_sha256.lower()
    ):
        errors.append("external trust root: normalized v3 alias SHA-256 is malformed")
    if root.function is None or not isinstance(root.function, str) or not root.function:
        errors.append("external trust root: function is absent")
    if root.cwd is None:
        errors.append("external trust root: cwd is absent")
    else:
        try:
            cwd = _canonical_external_path(root.cwd, "cwd")
            if not cwd.is_dir():
                errors.append("external trust root: cwd is not a directory")
        except CorrelatorError as error:
            errors.append(str(error))
    if not isinstance(root.argv, (list, tuple)) or not root.argv or any(
        not isinstance(token, str) or not token for token in root.argv
    ):
        errors.append("external trust root: argv is absent or malformed")
    if not isinstance(root.session_id, str) or not root.session_id:
        errors.append("external trust root: session_id is absent")
    if isinstance(root.process_id, bool) or not isinstance(root.process_id, int) or root.process_id <= 0:
        errors.append("external trust root: process_id is absent or malformed")

    if expected_allocator is not None and root.compiler_sha256 is not None:
        pass
    # Distinct trust-root artifacts must not be interchangeable aliases.  The
    # inode test catches hardlinks; the digest test catches same-byte aliases
    # at distinct paths when the filesystem cannot expose inode identity.
    seen_paths: dict[str, str] = {}
    seen_identities: dict[tuple[int, int], str] = {}
    seen_hashes: dict[str, str] = {}
    for label, path, digest, identity in anchors:
        path_key = os.path.normcase(str(path))
        if path_key in seen_paths and seen_paths[path_key] != label:
            errors.append(f"external trust root: {label} aliases {seen_paths[path_key]}")
        seen_paths[path_key] = label
        if identity is not None:
            if identity in seen_identities and seen_identities[identity] != label:
                errors.append(f"external trust root: {label} is a hardlink alias of {seen_identities[identity]}")
            seen_identities[identity] = label
        if digest in seen_hashes and seen_hashes[digest] != label:
            errors.append(f"external trust root: {label} is a same-byte alias of {seen_hashes[digest]}")
        else:
            seen_hashes[digest] = label
    if not primary_v3 and expected_v3 is not None and expected_pcode is not None:
        if os.path.normcase(str(expected_pcode)) == os.path.normcase(str(expected_v3)):
            errors.append("external trust root: v2 and normalized v3 PCode inputs must be distinct")
    return sorted(set(errors))


def load_json(path: str | Path) -> dict[str, Any]:
    """Load one JSON trace with a stable, fail-closed error type."""

    trace_path = Path(path)
    try:
        value = _strict_json_file(trace_path)
    except OSError as error:
        raise CorrelatorError(f"cannot read {trace_path}: {error}") from error
    value = _mapping(value, str(trace_path))
    _reject_nonfinite(value, str(trace_path))
    return dict(value)


def _require_trace_mapping_matches_path(
    trace: Mapping[str, Any],
    path: str | Path | None,
    expected_sha256: str | None,
    label: str,
) -> None:
    """Bind an input mapping to the raw file covered by an external anchor.

    Hashing a trusted path alone is insufficient when a caller can supply a
    different in-memory object to ``correlate``.  When an external receipt
    anchor is present, require the supplied mapping to equal the JSON object
    loaded from that exact path before any authenticated claim is considered.
    The digest check remains separate so whitespace/order changes still fail.
    """

    if expected_sha256 is None or path is None:
        return
    on_disk = load_json(path)
    if dict(trace) != on_disk:
        raise CorrelatorError(
            f"{label} mapping does not match the externally anchored trace at {Path(path)}"
        )


def _known_varinfo(record: Mapping[str, Any]) -> dict[str, Any]:
    raw = record.get("known_varinfo")
    if not isinstance(raw, Mapping):
        return {}
    result: dict[str, Any] = {}
    for key in ("flags", "rclass", "reg", "reg_hi", "usage", "used", "noregister"):
        if key in raw and isinstance(raw[key], int) and not isinstance(raw[key], bool):
            result[key] = raw[key]
    return result


def _record_name(record: Mapping[str, Any], where: str) -> str:
    return _nonempty_string(record.get("name"), f"{where}.name")


def _validate_allocator(trace: Mapping[str, Any]) -> dict[str, Any]:
    schema = _nonempty_string(trace.get("schema"), "allocator.schema")
    if schema != SUPPORTED_ALLOCATOR_SCHEMA:
        raise CorrelatorError(f"allocator.schema: unsupported schema {schema!r}")
    events = trace.get("assignment_events")
    if not isinstance(events, list) or not events:
        raise CorrelatorError("allocator.assignment_events: expected a non-empty list")

    normalized_events: list[dict[str, Any]] = []
    for index, raw_event in enumerate(events):
        event = _mapping(raw_event, f"allocator.assignment_events[{index}]")
        order = _integer(event.get("order"), f"allocator.assignment_events[{index}].order")
        bank = _nonempty_string(event.get("bank"), f"allocator.assignment_events[{index}].bank")
        selected = event.get("selected_object")
        if selected is not None:
            selected = _mapping(selected, f"allocator.assignment_events[{index}].selected_object")
            _record_name(selected, f"allocator.assignment_events[{index}].selected_object")
        after_locals = event.get("after_locals")
        if not isinstance(after_locals, list):
            raise CorrelatorError(
                f"allocator.assignment_events[{index}].after_locals: expected a list"
            )
        locals_normalized: list[dict[str, Any]] = []
        for local_index, raw_local in enumerate(after_locals):
            local = _mapping(raw_local, f"allocator.assignment_events[{index}].after_locals[{local_index}]")
            name = _record_name(local, f"allocator.assignment_events[{index}].after_locals[{local_index}]")
            locals_normalized.append(
                {
                    "name": name,
                    "datatype": local.get("datatype"),
                    "type_code": local.get("type_code"),
                    "known_varinfo": _known_varinfo(local),
                    "compiler_list_order": local_index,
                }
            )
        normalized_events.append(
            {
                "order": order,
                "bank": bank,
                "selected_name": selected.get("name") if selected else None,
                "locals": locals_normalized,
            }
        )

    normalized_events.sort(key=lambda event: (event["order"], event["bank"]))
    first_names = [local["name"] for local in normalized_events[0]["locals"]]
    for event in normalized_events[1:]:
        names = [local["name"] for local in event["locals"]]
        if names != first_names:
            raise CorrelatorError(
                "allocator.assignment_events: after_locals identity/order changes across snapshots"
            )

    final_locals = normalized_events[-1]["locals"]
    duplicates = sorted(
        name for name, count in Counter(local["name"] for local in final_locals).items() if count > 1
    )
    return {
        "schema": schema,
        "function": trace.get("function") or trace.get("target"),
        "target": trace.get("target"),
        "status": trace.get("status"),
        "events": normalized_events,
        "locals": final_locals,
        "duplicate_names": duplicates,
        "limitations": list(trace.get("limitations") or []),
        "compiler": trace.get("compiler"),
    }


def _unwrap(value: Any) -> Any:
    """Return the value from v3's ``{status,value}`` wrapper, if present."""

    if isinstance(value, Mapping) and "value" in value and set(value) <= {"status", "value"}:
        return value.get("value")
    return value


def _instruction_fields(raw: Mapping[str, Any], index: int, version: int) -> dict[str, Any]:
    where = f"pcode.instructions[{index}]"
    mnemonic = _unwrap(raw.get("mnemonic"))
    if not isinstance(mnemonic, str) or not mnemonic:
        raise CorrelatorError(f"{where}.mnemonic: expected a non-empty string")
    order = raw.get("order", raw.get("instruction_order"))
    order = _integer(_unwrap(order), f"{where}.order")
    block = _unwrap(raw.get("block"))
    if block is not None and not isinstance(block, (str, int)):
        raise CorrelatorError(f"{where}.block: expected a string/integer or null")
    source_line = raw.get("source_line")
    if source_line is None and "sourceoffset" in raw:
        source_line = _unwrap(raw.get("sourceoffset"))
    if source_line is not None and (isinstance(source_line, bool) or not isinstance(source_line, int)):
        source_line = None

    memory_objects = raw.get("memory_objects", [])
    if memory_objects is None:
        memory_objects = []
    if not isinstance(memory_objects, list) or any(not isinstance(item, str) for item in memory_objects):
        raise CorrelatorError(f"{where}.memory_objects: expected a list of strings")

    operands = raw.get("operands")
    operand_names: list[str] = []
    if isinstance(operands, str):
        # The v2 producer emits local/global memory labels in parentheses.  We
        # only retain exact labels from memory_objects; no substring matching.
        for part in operands.split("(")[1:]:
            if ")" in part:
                operand_names.append(part.split(")", 1)[0])
    elif isinstance(operands, list):
        # v3 carries object_reference values, but current captures mark them
        # UNKNOWN.  Preserve only explicit, non-null values.  The producer's
        # ``present`` value is a sentinel, not a source-object label.
        for operand in operands:
            if not isinstance(operand, Mapping):
                continue
            ref = _unwrap(operand.get("object_reference"))
            if isinstance(ref, str) and ref and ref.casefold() != "present":
                operand_names.append(ref)

    # v3's normalized instruction stream intentionally has no vreg identity.
    # Do not accept an additive/forged virtual_registers field there and never
    # infer an ID from the physical register/evidence fields.
    normalized_vregs: list[str] = []
    if version == 2:
        vregs = raw.get("virtual_registers", [])
        if vregs is None:
            vregs = []
        if not isinstance(vregs, list):
            raise CorrelatorError(f"{where}.virtual_registers: expected a list of strings")
        normalized_vregs = [
            _vreg_id(item, f"{where}.virtual_registers[{vreg_index}]")
            for vreg_index, item in enumerate(vregs)
        ]
        if len(set(normalized_vregs)) != len(normalized_vregs):
            raise CorrelatorError(f"{where}.virtual_registers: duplicate vreg IDs")

    return {
        "order": order,
        "block": block,
        "mnemonic": mnemonic,
        "source_line": source_line,
        "memory_objects": sorted(set(memory_objects)),
        "operand_names": sorted(set(operand_names)),
        "virtual_registers": sorted(normalized_vregs),
        "use_id": _unwrap(raw.get("useID")),
        "def_id": _unwrap(raw.get("defID")),
        "version": version,
    }


def _vreg_chronology(capture: Mapping[str, Any]) -> dict[str, dict[str, Any]]:
    chronology = capture.get("vreg_chronology")
    if not isinstance(chronology, Mapping):
        return {}
    rows = chronology.get("vregs")
    if not isinstance(rows, list):
        return {}
    result: dict[str, dict[str, Any]] = {}
    for index, raw in enumerate(rows):
        item = _mapping(raw, f"pcode.vreg_chronology.vregs[{index}]")
        vreg_id = _vreg_id(item.get("vreg_id"), f"pcode.vreg_chronology.vregs[{index}].vreg_id")
        if vreg_id in result:
            raise CorrelatorError(
                f"pcode.vreg_chronology.vregs[{index}].vreg_id: duplicate ID {vreg_id!r}"
            )
        # The trace is a diagnostic, so retain only scalar/list fields needed
        # by the fingerprint and tolerate future additive fields.
        def integer_or_none(key: str) -> int | None:
            value = item.get(key)
            return value if isinstance(value, int) and not isinstance(value, bool) else None

        result[vreg_id] = {
            "vreg_id": vreg_id,
            "creation_order": integer_or_none("creation_order"),
            "first_occurrence": integer_or_none("first_occurrence"),
            "last_occurrence": integer_or_none("last_occurrence"),
            "occurrence_count": integer_or_none("occurrence_count"),
            "crossed_call_count": len(item.get("crossed_call_orders", []))
            if isinstance(item.get("crossed_call_orders"), list)
            else None,
            "blocks": sorted(str(value) for value in item.get("blocks", []) if isinstance(value, (str, int))),
            "source_lines": sorted(value for value in item.get("source_lines", []) if isinstance(value, int)),
            "interval_kind": item.get("interval_kind"),
            "reuse_status": item.get("reuse_status"),
        }
    return result


def _validate_pcode(trace: Mapping[str, Any], allocator_names: set[str]) -> dict[str, Any]:
    schema = _nonempty_string(trace.get("schema"), "pcode.schema")
    if schema not in SUPPORTED_PCODE_SCHEMAS:
        raise CorrelatorError(f"pcode.schema: unsupported schema {schema!r}")
    version = 2 if schema.endswith("/v2") else 3
    capture = trace.get("capture") if version == 2 else trace
    capture = _mapping(capture, "pcode.capture")
    pcode = capture.get("pcode") if version == 2 else None
    if version == 2:
        pcode = _mapping(pcode, "pcode.capture.pcode")
        stage_key = "backend-00-initial-code.txt"
        stage = pcode.get(stage_key)
        if not isinstance(stage, Mapping):
            raise CorrelatorError(f"pcode.capture.pcode.{stage_key}: missing initial stage")
    else:
        stages = _mapping(trace.get("stages"), "pcode.stages")
        stage_key = "backend-00-initial-code.pcode.json"
        stage = stages.get(stage_key)
        if not isinstance(stage, Mapping):
            raise CorrelatorError(f"pcode.stages.{stage_key}: missing initial stage")

    raw_instructions = stage.get("instructions")
    if not isinstance(raw_instructions, list):
        raise CorrelatorError("pcode initial stage.instructions: expected a list")
    instructions = [
        _instruction_fields(_mapping(raw, f"pcode.instructions[{index}]"), index, version)
        for index, raw in enumerate(raw_instructions)
    ]

    chronology = _vreg_chronology(capture) if version == 2 else {}
    profiles: dict[str, dict[str, Any]] = {}
    vreg_instruction_orders: dict[str, list[int]] = defaultdict(list)
    vreg_instruction_rows: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for instruction in instructions:
        names = set(instruction["memory_objects"]) | set(instruction["operand_names"])
        for name in sorted(names & allocator_names):
            profile = profiles.setdefault(
                name,
                {
                    "name": name,
                    "instruction_orders": [],
                    "mnemonics": Counter(),
                    "blocks": set(),
                    "source_lines": set(),
                    "vreg_ids": set(),
                },
            )
            profile["instruction_orders"].append(instruction["order"])
            profile["mnemonics"][instruction["mnemonic"]] += 1
            if instruction["block"] is not None:
                profile["blocks"].add(str(instruction["block"]))
            if instruction["source_line"] is not None:
                profile["source_lines"].add(instruction["source_line"])
            profile["vreg_ids"].update(instruction["virtual_registers"])
        for vreg_id in instruction["virtual_registers"]:
            vreg_instruction_orders[vreg_id].append(instruction["order"])
            vreg_instruction_rows[vreg_id].append(instruction)

    normalized_profiles: dict[str, dict[str, Any]] = {}
    for name, profile in profiles.items():
        vreg_ids = sorted(profile["vreg_ids"])
        normalized_profiles[name] = {
            "name": name,
            "instruction_count": len(profile["instruction_orders"]),
            "instruction_orders": sorted(profile["instruction_orders"]),
            "mnemonics": dict(sorted(profile["mnemonics"].items())),
            "blocks": sorted(profile["blocks"]),
            "source_lines": sorted(profile["source_lines"]),
            "vreg_ids": vreg_ids,
            "vreg_fingerprints": [
                _vreg_fingerprint(
                    vreg_id,
                    chronology.get(vreg_id),
                    vreg_instruction_orders.get(vreg_id, []),
                    vreg_instruction_rows.get(vreg_id, []),
                )
                for vreg_id in vreg_ids
            ],
        }

    vreg_ids = set(chronology) | set(vreg_instruction_orders)
    pcode_vregs = {
        vreg_id: _vreg_fingerprint(
            vreg_id,
            chronology.get(vreg_id),
            vreg_instruction_orders.get(vreg_id, []),
            vreg_instruction_rows.get(vreg_id, []),
        )
        for vreg_id in sorted(vreg_ids)
    }

    limitations = list(trace.get("limitations") or [])
    if version == 2:
        limitations.extend(capture.get("limitations") or [])
    else:
        limitations.extend(trace.get("limitations") or [])
    trace_function_raw = trace.get("function")
    capture_function_raw = capture.get("function")
    trace_function = (
        _nonempty_string(trace_function_raw, "pcode.function")
        if trace_function_raw is not None
        else None
    )
    capture_function = (
        _nonempty_string(capture_function_raw, "pcode.capture.function")
        if capture_function_raw is not None
        else None
    )
    if trace_function and capture_function and trace_function != capture_function:
        raise CorrelatorError("pcode.function and pcode.capture.function differ")

    trace_status_raw = trace.get("status")
    capture_status_raw = capture.get("capture_status")
    trace_status = (
        _nonempty_string(trace_status_raw, "pcode.status")
        if trace_status_raw is not None
        else None
    )
    capture_status = (
        _nonempty_string(capture_status_raw, "pcode.capture.capture_status")
        if capture_status_raw is not None
        else None
    )
    if trace_status and capture_status and trace_status.upper() != capture_status.upper():
        raise CorrelatorError("pcode.status and pcode.capture.capture_status differ")

    v3_contract = _validate_v3_contract(trace) if version == 3 else None
    if v3_contract and v3_contract.get("valid"):
        # Direct ownership is the v3 identity source.  These fingerprints are
        # deliberately chronology-free and instruction-free; adding a stage
        # physical register here would violate the ownership contract.
        for vreg_id in sorted(set(v3_contract.get("direct_rows", {}).values())):
            pcode_vregs.setdefault(
                vreg_id,
                _vreg_fingerprint(vreg_id, None, [], []),
            )

    return {
        "schema": schema,
        "version": version,
        "function": capture_function or trace_function,
        "status": trace_status,
        "capture_status": capture_status,
        "authentication": trace.get("authentication") if isinstance(trace.get("authentication"), Mapping) else {},
        "stage": stage_key,
        "instructions": instructions,
        "profiles": normalized_profiles,
        "vregs": pcode_vregs,
        "limitations": sorted(set(str(item) for item in limitations)),
        "capture": capture,
        "v3_contract": v3_contract,
    }


def _vreg_fingerprint(
    vreg_id: str,
    chronology: Mapping[str, Any] | None,
    orders: Iterable[int],
    rows: Iterable[Mapping[str, Any]],
) -> dict[str, Any]:
    rows = list(rows)
    mnemonics = Counter(row["mnemonic"] for row in rows)
    blocks = {str(row["block"]) for row in rows if row.get("block") is not None}
    source_lines = {row["source_line"] for row in rows if row.get("source_line") is not None}
    result: dict[str, Any] = {
        "vreg_id": vreg_id,
        "instruction_orders": sorted(set(orders)),
        "instruction_count": len(rows),
        "mnemonics": dict(sorted(mnemonics.items())),
        "blocks": sorted(blocks),
        "source_lines": sorted(source_lines),
    }
    if chronology:
        result["chronology"] = dict(chronology)
    else:
        result["chronology"] = None
    return result


def _v3_vreg_id(value: Any, where: str) -> str:
    """Validate a producer v3 virtual-register identity.

    The v3 producer's direct map is the only accepted identity source.  Its
    virtual registers start at 32; accepting a physical home here would make
    an otherwise unauthenticated operand field look like an ownership edge.
    """

    value = _nonempty_string(value, where)
    if V3_VREG_PATTERN.fullmatch(value) is None:
        raise CorrelatorError(f"{where}: expected canonical v3 virtual-register ID")
    return value


def _upper_status(value: Any) -> str | None:
    return value.upper() if isinstance(value, str) and value else None


def _v3_provenance_fields(trace: Mapping[str, Any]) -> dict[str, list[str]]:
    """Collect explicit manifest/provenance values from a v3 packet.

    Producer revisions have used both a top-level ``provenance`` envelope and
    the v2-compatible ``authentication.artifacts`` envelope.  Accept either
    spelling, but only for explicit SHA/provenance fields; source/compiler
    *paths*, names, and instruction labels are never provenance.
    """

    fields: dict[str, list[str]] = {
        "source_sha256": [],
        "compiler_sha256": [],
        "ownership_sha256": [],
        "ownership_events_sha256": [],
        "manifest_sha256": [],
        "source_provenance": [],
        "compiler_provenance": [],
        "ownership_provenance": [],
    }
    sha_keys = {
        "source_sha256": "source_sha256",
        "source_hash": "source_sha256",
        "compiler_sha256": "compiler_sha256",
        "compiler_hash": "compiler_sha256",
        "ownership_sha256": "ownership_sha256",
        "ownership_hash": "ownership_sha256",
        "ownership_digest": "ownership_sha256",
        "ownership_events_sha256": "ownership_events_sha256",
        "ownership_events_hash": "ownership_events_sha256",
        "ownership_events_digest": "ownership_events_sha256",
        "manifest_sha256": "manifest_sha256",
        "manifest_hash": "manifest_sha256",
    }
    provenance_keys = {
        "source_provenance": "source_provenance",
        "compiler_provenance": "compiler_provenance",
        "ownership_provenance": "ownership_provenance",
    }

    def visit(value: Any, parent_key: str = "") -> None:
        if isinstance(value, Mapping):
            for raw_key, child in value.items():
                if not isinstance(raw_key, str):
                    continue
                key = raw_key.casefold()
                if key in sha_keys and isinstance(child, str) and child:
                    fields[sha_keys[key]].append(child)
                elif key in provenance_keys and isinstance(child, str) and child:
                    fields[provenance_keys[key]].append(child)
                elif key == "sha256" and isinstance(child, str) and child:
                    parent = parent_key.casefold()
                    parent_aliases = {
                        "source": "source_sha256",
                        "source_provenance": "source_sha256",
                        "compiler": "compiler_sha256",
                        "compiler_provenance": "compiler_sha256",
                        "ownership": "ownership_sha256",
                        "ownership_provenance": "ownership_sha256",
                        "ownership_events": "ownership_events_sha256",
                        "manifest": "manifest_sha256",
                    }
                    if parent in parent_aliases:
                        fields[parent_aliases[parent]].append(child)
                visit(child, raw_key)
        elif isinstance(value, list):
            for child in value:
                visit(child, parent_key)

    visit(trace)
    return {key: sorted(set(values)) for key, values in fields.items()}


def _v3_source_inventory(trace: Mapping[str, Any]) -> tuple[dict[str, Any], list[str]]:
    """Normalize v3 local/argument rows without using names as identity."""

    reasons: list[str] = []
    raw_inventory = trace.get("source_inventory")
    if not isinstance(raw_inventory, Mapping):
        return {
            "status": None,
            "reason": "source_inventory is absent",
            "locals": [],
            "arguments": [],
        }, ["source_inventory is absent"]

    status = raw_inventory.get("status") if isinstance(raw_inventory.get("status"), str) else None
    if status != "CAPTURED":
        reasons.append("v3 source inventory status is not CAPTURED")
    normalized: dict[str, list[dict[str, Any]]] = {"locals": [], "arguments": []}
    all_rows: list[dict[str, Any]] = []
    seen_keys: set[tuple[str, int]] = set()
    duplicate_ordinals: set[int] = set()
    for container, expected_kind in (("locals", "local"), ("arguments", "argument")):
        raw_rows = raw_inventory.get(container)
        if not isinstance(raw_rows, list):
            reasons.append(f"v3 source inventory {container} is not a list")
            continue
        for index, raw in enumerate(raw_rows):
            where = f"pcode.source_inventory.{container}[{index}]"
            if not isinstance(raw, Mapping):
                reasons.append(f"{where}: expected an object")
                continue
            row = dict(raw)
            try:
                ordinal = _integer(row.get("ordinal"), f"{where}.ordinal")
                if ordinal is None or ordinal < 0:
                    raise CorrelatorError(f"{where}.ordinal: expected a non-negative integer")
                kind = row.get("kind", expected_kind)
                if kind != expected_kind:
                    raise CorrelatorError(
                        f"{where}.kind: expected {expected_kind!r} for {container}"
                    )
                name = _record_name(row, where)
                compiler_list_order = row.get("compiler_list_order", index)
                compiler_list_order = _integer(
                    compiler_list_order, f"{where}.compiler_list_order"
                )
                if compiler_list_order is None or compiler_list_order < 0:
                    raise CorrelatorError(
                        f"{where}.compiler_list_order: expected a non-negative integer"
                    )
                datatype = _integer(row.get("datatype"), f"{where}.datatype")
                if datatype is None or datatype < 0:
                    raise CorrelatorError(f"{where}.datatype: expected a non-negative integer")
                type_code = row.get("type_code", datatype)
                type_code = _integer(type_code, f"{where}.type_code")
                if type_code is None or type_code < 0:
                    raise CorrelatorError(f"{where}.type_code: expected a non-negative integer")
                size = _integer(row.get("size"), f"{where}.size")
                if size is None or size < 0:
                    raise CorrelatorError(f"{where}.size: expected a non-negative integer")
                raw_vregs = row.get("vreg_ids")
                if not isinstance(raw_vregs, list):
                    raise CorrelatorError(f"{where}.vreg_ids: expected a list")
                vreg_ids = [
                    _v3_vreg_id(value, f"{where}.vreg_ids[{vreg_index}]")
                    for vreg_index, value in enumerate(raw_vregs)
                ]
                if len(set(vreg_ids)) != len(vreg_ids):
                    raise CorrelatorError(f"{where}.vreg_ids: duplicate IDs")
                vreg_status = row.get("vreg_status") if isinstance(row.get("vreg_status"), str) else None
                if vreg_status not in {"AUTHENTICATED", "UNKNOWN"}:
                    raise CorrelatorError(
                        f"{where}.vreg_status: expected AUTHENTICATED or UNKNOWN"
                    )
                # A captured direct-ownership row is intentionally all-or-
                # nothing.  UNKNOWN rows cannot carry a guessed ID and an
                # authenticated row cannot claim zero or multiple IDs.
                if vreg_status == "AUTHENTICATED" and len(vreg_ids) != 1:
                    raise CorrelatorError(
                        f"{where}.vreg_ids: exactly one ID is required for AUTHENTICATED ownership"
                    )
                if vreg_status == "UNKNOWN" and vreg_ids:
                    raise CorrelatorError(
                        f"{where}.vreg_ids: UNKNOWN ownership must not carry IDs"
                    )
            except CorrelatorError as error:
                reasons.append(str(error))
                continue

            identity = (expected_kind, int(ordinal))
            if identity in seen_keys:
                reasons.append(f"{where}: duplicate kind/ordinal source object")
            if any(existing[1] == int(ordinal) for existing in seen_keys):
                # ``kind`` is part of the source-object identity.  Duplicate
                # ordinals across local/argument lists are allowed only when
                # every ownership edge carries the matching kind.
                duplicate_ordinals.add(int(ordinal))
            seen_keys.add(identity)
            if any(existing.get("name") == name for existing in all_rows):
                reasons.append(f"{where}.name: duplicate source-object name")
            normalized_row = {
                "kind": expected_kind,
                "ordinal": int(ordinal),
                "compiler_list_order": int(compiler_list_order),
                "name": name,
                "datatype": int(datatype),
                "type_code": int(type_code),
                "size": int(size),
                "vreg_ids": sorted(vreg_ids),
                "vreg_status": vreg_status,
            }
            normalized[container].append(normalized_row)
            all_rows.append(normalized_row)

    # Preserve source list order as evidence, but reject duplicate compiler
    # ordinals within a kind because they defeat the source-object identity.
    for container in ("locals", "arguments"):
        orders = [row["compiler_list_order"] for row in normalized[container]]
        if len(set(orders)) != len(orders):
            reasons.append(f"v3 source inventory {container} has duplicate compiler_list_order")

    return {
        "status": status,
        "reason": raw_inventory.get("reason"),
        "locals": normalized["locals"],
        "arguments": normalized["arguments"],
        "objects": all_rows,
        "duplicate_ordinals": sorted(duplicate_ordinals),
    }, reasons


def _v3_direct_rows(
    value: Any,
    label: str,
    source_rows: Sequence[Mapping[str, Any]],
) -> tuple[dict[tuple[str | None, int], str], list[str]]:
    """Validate one v3 frontend_join/ownership direct map."""

    reasons: list[str] = []
    if not isinstance(value, Mapping):
        return {}, [f"v3 {label} is absent"]
    if value.get("status") != "AUTHENTICATED":
        reasons.append(f"v3 {label} status is not AUTHENTICATED")
    raw_rows = value.get("direct_object_vregs")
    if not isinstance(raw_rows, list):
        return {}, reasons + [f"v3 {label}.direct_object_vregs is not a list"]
    source_by_key = {
        (str(row["kind"]), int(row["ordinal"])): row for row in source_rows
    }
    source_by_ordinal: dict[int, list[Mapping[str, Any]]] = defaultdict(list)
    for row in source_rows:
        source_by_ordinal[int(row["ordinal"])].append(row)
    result: dict[tuple[str | None, int], str] = {}
    seen_vregs: set[str] = set()
    for index, raw in enumerate(raw_rows):
        where = f"pcode.{label}.direct_object_vregs[{index}]"
        if not isinstance(raw, Mapping):
            reasons.append(f"{where}: expected an object")
            continue
        try:
            ordinal = _integer(raw.get("object_ordinal"), f"{where}.object_ordinal")
            if ordinal is None or ordinal < 0:
                raise CorrelatorError(f"{where}.object_ordinal: expected a non-negative integer")
            kind = raw.get("kind")
            candidates = source_by_ordinal.get(int(ordinal), [])
            if not candidates:
                raise CorrelatorError(f"{where}: source object ordinal is missing")
            if kind is None:
                if len(candidates) != 1:
                    raise CorrelatorError(
                        f"{where}: kind is required for an ambiguous source-object ordinal"
                    )
                source_row = candidates[0]
                kind = source_row.get("kind")
            else:
                source_row = source_by_key.get((str(kind), int(ordinal)))
                if source_row is None:
                    raise CorrelatorError(f"{where}.kind: does not match source inventory kind")
            if raw.get("status") != "AUTHENTICATED":
                raise CorrelatorError(f"{where}.status: ownership is not AUTHENTICATED")
            vreg_id = _v3_vreg_id(raw.get("vreg_id"), f"{where}.vreg_id")
            # New normalized packets carry the complete edge tuple.  Older
            # packets only carry ordinal/vreg; derive the remaining immutable
            # fields from the authenticated inventory, but reject any supplied
            # value that disagrees rather than allowing a partial forged edge.
            for field in ("name", "datatype", "type_code", "size"):
                if field not in raw:
                    continue
                expected = source_row.get(field)
                actual = raw.get(field)
                if field == "name":
                    if not isinstance(actual, str) or not actual:
                        raise CorrelatorError(f"{where}.{field}: expected a non-empty string")
                elif isinstance(actual, bool) or not isinstance(actual, int):
                    raise CorrelatorError(f"{where}.{field}: expected an integer")
                if actual != expected:
                    raise CorrelatorError(f"{where}.{field}: does not match source inventory")
            key = (str(kind), int(ordinal))
            if key in result:
                raise CorrelatorError(f"{where}: duplicate source-object ownership row")
            if vreg_id in seen_vregs:
                raise CorrelatorError(f"{where}: virtual register ownership is reused")
            result[key] = vreg_id
            seen_vregs.add(vreg_id)
        except CorrelatorError as error:
            reasons.append(str(error))

    expected_keys = set(source_by_key)
    owned_keys = set(result)
    if owned_keys != expected_keys:
        missing = sorted(expected_keys - owned_keys)
        extra = sorted(owned_keys - expected_keys)
        if missing:
            reasons.append("v3 direct ownership is missing source objects: " + ", ".join(map(str, missing)))
        if extra:
            reasons.append("v3 direct ownership contains unknown source objects: " + ", ".join(map(str, extra)))
    if len(result) != len(source_rows):
        reasons.append("v3 direct ownership is not a strict one-to-one source-object map")
    return result, reasons


def _validate_v3_contract(trace: Mapping[str, Any]) -> dict[str, Any]:
    """Normalize and validate v3's direct source-object ownership contract."""

    reasons: list[str] = []
    shape_errors = _validate_v3_closed_shape(trace)
    reasons.extend(shape_errors)
    status = trace.get("status") if isinstance(trace.get("status"), str) else None
    pcode_status = trace.get("pcode_status") if isinstance(trace.get("pcode_status"), str) else None
    # The normalized producer's status tuple is intentionally exact.  A
    # packet claiming a different success spelling must remain diagnostic and
    # can never be upgraded by a later direct-map check.
    if status != "UNKNOWN":
        reasons.append("v3 top-level status must be UNKNOWN")
    if pcode_status != "EXACT":
        reasons.append("v3 pcode_status must be EXACT")
    for key in ("capture_status", "result_status"):
        value = trace.get(key)
        if value is not None:
            reasons.append(f"v3 {key} is not an allowed top-level field")

    inventory, inventory_reasons = _v3_source_inventory(trace)
    reasons.extend(inventory_reasons)
    source_rows = list(inventory.get("objects") or [])
    if not source_rows:
        reasons.append("v3 source inventory has no local or argument objects")

    def authenticated_session(value: Any, label: str) -> Mapping[str, Any] | None:
        """Validate the small session identity needed by both direct joins.

        The producer schema carries a full capture session.  Keep the adapter
        tolerant of additive fields, but do not let a missing or mismatched
        session turn a direct row into an authenticated ownership edge.
        """

        if not isinstance(value, Mapping) or value.get("status") != "AUTHENTICATED":
            reasons.append(f"v3 {label} status is not AUTHENTICATED")
            return None
        session = value.get("session")
        if not isinstance(session, Mapping):
            reasons.append(f"v3 {label}.session is absent for AUTHENTICATED evidence")
            return None
        required_identity = ("session_id", "process_id", "function", "source", "compiler", "argv", "cwd")
        for field in required_identity:
            if field not in session:
                reasons.append(f"v3 {label}.session.{field} is absent")
        session_id = session.get("session_id")
        if not isinstance(session_id, str) or not session_id:
            reasons.append(f"v3 {label}.session.session_id is absent")
        if isinstance(session.get("process_id"), bool) or not isinstance(session.get("process_id"), int) or session.get("process_id") <= 0:
            reasons.append(f"v3 {label}.session.process_id is malformed")
        trace_function = trace.get("function")
        session_function = session.get("function")
        if session_function is not None and (
            not isinstance(session_function, str)
            or not session_function
            or (isinstance(trace_function, str) and session_function != trace_function)
        ):
            reasons.append(f"v3 {label}.session.function is not bound to the trace function")
        for field in ("source", "compiler", "cwd"):
            if not isinstance(session.get(field), str) or not session.get(field):
                reasons.append(f"v3 {label}.session.{field} is malformed")
        if not isinstance(session.get("argv"), list) or any(
            not isinstance(token, str) or not token for token in session.get("argv", [])
        ):
            reasons.append(f"v3 {label}.session.argv is malformed")
        if session.get("snapshot_complete") is not True:
            reasons.append(f"v3 {label}.session.snapshot_complete is not true")
        return session

    frontend_value = trace.get("frontend_join")
    ownership_value = trace.get("ownership")
    frontend_session = authenticated_session(frontend_value, "frontend_join")
    ownership_session = authenticated_session(ownership_value, "ownership")
    if frontend_session is not None and ownership_session is not None:
        for field in ("session_id", "process_id", "function", "source", "compiler", "argv", "cwd"):
            if frontend_session.get(field) != ownership_session.get(field):
                reasons.append(f"v3 frontend_join and ownership sessions differ at {field}")

    frontend_rows, frontend_reasons = _v3_direct_rows(
        frontend_value, "frontend_join", source_rows
    )
    ownership_rows, ownership_reasons = _v3_direct_rows(
        ownership_value, "ownership", source_rows
    )
    reasons.extend(frontend_reasons)
    reasons.extend(ownership_reasons)
    if frontend_rows != ownership_rows:
        reasons.append("v3 frontend_join and ownership direct maps differ")
    if not reasons:
        direct_by_key = {
            (kind, ordinal): vreg_id for (kind, ordinal), vreg_id in frontend_rows.items()
        }
        for row in source_rows:
            ordinal = int(row["ordinal"])
            key = (str(row["kind"]), ordinal)
            declared = list(row.get("vreg_ids") or [])
            if len(declared) != 1 or direct_by_key.get(key) != declared[0]:
                reasons.append(
                    "v3 source inventory and direct ownership disagree for source object "
                    + str(ordinal)
                )
                break
    # The two direct edges must describe the same complete source-object
    # tuple.  The vreg-only map comparison above intentionally remains the
    # canonical identity check; metadata carried by either edge is checked
    # against the inventory here so a name/type/size swap cannot survive.
    if not reasons:
        for label, value in (("frontend_join", frontend_value), ("ownership", ownership_value)):
            raw_rows = value.get("direct_object_vregs") if isinstance(value, Mapping) else []
            for index, raw in enumerate(raw_rows if isinstance(raw_rows, list) else []):
                key = (str(raw.get("kind")) if raw.get("kind") is not None else None, raw.get("object_ordinal"))
                if key[0] is None:
                    candidates = [row for row in source_rows if row.get("ordinal") == key[1]]
                    source_row = candidates[0] if len(candidates) == 1 else None
                else:
                    source_row = next((row for row in source_rows if (row.get("kind"), row.get("ordinal")) == key), None)
                if source_row is None:
                    continue
                for field in ("name", "datatype", "type_code", "size"):
                    if field in raw and raw.get(field) != source_row.get(field):
                        reasons.append(f"v3 {label}.direct_object_vregs[{index}].{field} differs from source inventory")

    # If a packet has any malformed/unknown ownership claim, discard every
    # partial row.  A forged unique name or vreg must never survive as a
    # partially authenticated map.
    valid = not reasons
    direct_rows = frontend_rows if valid else {}
    if valid and len(direct_rows) != len(source_rows):
        valid = False
        reasons.append("v3 direct ownership cardinality is not one-to-one")
        direct_rows = {}
    if valid and any(row.get("vreg_status") != "AUTHENTICATED" for row in source_rows):
        valid = False
        reasons.append("v3 source inventory contains unauthenticated ownership rows")
        direct_rows = {}
    return {
        "valid": valid,
        "reasons": sorted(set(reasons)),
        "top_level_status": status,
        "pcode_status": pcode_status,
        "source_inventory": inventory,
        "source_rows": source_rows,
        "frontend_join": trace.get("frontend_join"),
        "ownership": trace.get("ownership"),
        "direct_rows": direct_rows,
        "provenance_fields": _v3_provenance_fields(trace),
        "shape_valid": not shape_errors,
        "shape_errors": sorted(set(shape_errors)),
    }


def _allocator_source_inventory(
    allocator: Mapping[str, Any], pcode: Mapping[str, Any]
) -> tuple[dict[str, dict[str, Any]], set[str]]:
    capture = pcode.get("capture")
    if not isinstance(capture, Mapping):
        return {}, set()
    inventory = capture.get("source_inventory")
    if not isinstance(inventory, Mapping):
        return {}, set()
    rows = inventory.get("locals")
    if not isinstance(rows, list):
        return {}, set()
    result: dict[str, dict[str, Any]] = {}
    ambiguous_names: set[str] = set()
    vreg_claims: dict[str, set[str]] = defaultdict(set)
    for index, raw in enumerate(rows):
        row = _mapping(raw, f"pcode.capture.source_inventory.locals[{index}]")
        name = _nonempty_string(
            row.get("name"), f"pcode.capture.source_inventory.locals[{index}].name"
        )
        raw_vregs = row.get("vreg_ids", [])
        if not isinstance(raw_vregs, list):
            raise CorrelatorError(
                f"pcode.capture.source_inventory.locals[{index}].vreg_ids: expected a list"
            )
        vreg_ids = [
            _vreg_id(
                value,
                f"pcode.capture.source_inventory.locals[{index}].vreg_ids[{vreg_index}]",
            )
            for vreg_index, value in enumerate(raw_vregs)
        ]
        if len(set(vreg_ids)) != len(vreg_ids):
            raise CorrelatorError(
                f"pcode.capture.source_inventory.locals[{index}].vreg_ids: duplicate IDs"
            )
        if name in result:
            raise CorrelatorError(
                f"pcode.capture.source_inventory.locals[{index}].name: duplicate name {name!r}"
            )
        result[name] = {
            "name": name,
            "compiler_list_order": row.get("compiler_list_order", index),
            "vreg_ids": sorted(vreg_ids),
            "vreg_status": row.get("vreg_status"),
            "home": row.get("home"),
            "home_kind": row.get("home_kind"),
        }
        for vreg_id in vreg_ids:
            vreg_claims[vreg_id].add(name)
    for names in vreg_claims.values():
        if len(names) > 1:
            raise CorrelatorError(
                "pcode.capture.source_inventory.locals: one vreg ID is claimed by multiple names: "
                + ", ".join(sorted(names))
            )
    return result, ambiguous_names


def _authentication_gate(
    allocator: Mapping[str, Any],
    pcode: Mapping[str, Any],
    *,
    pcode_v3: Mapping[str, Any] | None,
    allocator_path: str | Path | None,
    pcode_path: str | Path | None,
    pcode_v3_path: str | Path | None,
    expected_source_sha256: str | None,
    expected_compiler_sha256: str | None,
    expected_allocator_trace_sha256: str | None,
    expected_pcode_trace_sha256: str | None,
    expected_pcode_v3_trace_sha256: str | None,
) -> tuple[bool, list[str], str | None, str | None]:
    """Validate the provenance contract needed for a resolved join.

    Raw trace anchors are checked against the exact bytes at the supplied
    paths.  The expected values are external receipt data, not values trusted
    from the JSON payloads themselves.
    """

    reasons: list[str] = []
    allocator_function = allocator.get("function")
    allocator_target = allocator.get("target")
    pcode_function = pcode.get("function")
    if not isinstance(allocator_function, str) or not allocator_function:
        reasons.append("allocator function is absent")
    if not isinstance(pcode_function, str) or not pcode_function:
        reasons.append("PCode function is absent")
    if (
        isinstance(allocator_function, str)
        and isinstance(pcode_function, str)
        and allocator_function != pcode_function
    ):
        reasons.append("allocator and PCode functions differ")
    if (
        isinstance(allocator_target, str)
        and allocator_target
        and isinstance(allocator_function, str)
        and allocator_target != allocator_function
    ):
        reasons.append("allocator function and target differ")
    if pcode_v3 is not None:
        pcode_v3_function = pcode_v3.get("function")
        if not isinstance(pcode_v3_function, str) or not pcode_v3_function:
            reasons.append("PCode v3 function is absent")
        elif isinstance(pcode_function, str) and pcode_v3_function != pcode_function:
            reasons.append("PCode v2 and v3 functions differ")
    else:
        reasons.append("PCode v3 trace is absent")

    if str(allocator.get("status") or "").upper() != "CAPTURED":
        reasons.append("allocator trace status is not CAPTURED")

    if pcode.get("version") != 2:
        reasons.append("only authenticated PCode v2 may resolve a join")
    if str(pcode.get("status") or "").upper() != "CAPTURED":
        reasons.append("PCode trace status is not CAPTURED")
    capture = pcode.get("capture")
    if not isinstance(capture, Mapping) or str(capture.get("capture_status") or "").upper() != "CAPTURED":
        reasons.append("PCode capture status is not CAPTURED")
    inventory = capture.get("source_inventory") if isinstance(capture, Mapping) else None
    if not isinstance(inventory, Mapping) or str(inventory.get("status") or "").upper() != "CAPTURED":
        reasons.append("source inventory status is not CAPTURED")

    authentication = pcode.get("authentication")
    if not isinstance(authentication, Mapping):
        authentication = {}
    if authentication.get("source_hash_authenticated") is not True:
        reasons.append("source hash is not explicitly authenticated")
    provenance = authentication.get("source_provenance")
    if not isinstance(provenance, str) or not provenance:
        reasons.append("source provenance is absent")

    artifacts = authentication.get("artifacts")
    artifacts = artifacts if isinstance(artifacts, Mapping) else {}

    source_artifact = artifacts.get("source")
    compiler_artifact = artifacts.get("compiler")
    source_sha = source_artifact.get("sha256") if isinstance(source_artifact, Mapping) else None
    compiler_sha = compiler_artifact.get("sha256") if isinstance(compiler_artifact, Mapping) else None
    if not isinstance(source_sha, str) or SHA256_PATTERN.fullmatch(source_sha) is None:
        reasons.append("authenticated source SHA-256 is absent or malformed")
        source_sha = None
    if not isinstance(compiler_sha, str) or SHA256_PATTERN.fullmatch(compiler_sha) is None:
        reasons.append("authenticated compiler SHA-256 is absent or malformed")
        compiler_sha = None

    allocator_compiler = allocator.get("compiler")
    allocator_compiler_sha = (
        allocator_compiler.get("sha256") if isinstance(allocator_compiler, Mapping) else None
    )
    if not isinstance(allocator_compiler_sha, str) or SHA256_PATTERN.fullmatch(allocator_compiler_sha) is None:
        reasons.append("allocator compiler SHA-256 is absent or malformed")
    elif compiler_sha is not None and allocator_compiler_sha.lower() != compiler_sha.lower():
        reasons.append("allocator and PCode compiler SHA-256 values differ")

    if source_sha is not None and isinstance(provenance, str):
        source_marker = source_sha[:8].upper()
        if source_marker not in provenance.upper():
            reasons.append("source provenance does not bind the source SHA-256 prefix")
    if expected_source_sha256 is None:
        reasons.append("external expected source SHA-256 is absent")
    elif not isinstance(expected_source_sha256, str) or SHA256_PATTERN.fullmatch(expected_source_sha256) is None:
        raise CorrelatorError("expected_source_sha256: expected a 64-digit SHA-256")
    elif source_sha is not None and source_sha.lower() != expected_source_sha256.lower():
        reasons.append("authenticated source SHA-256 differs from the external expected digest")
    if expected_compiler_sha256 is None:
        reasons.append("external expected compiler SHA-256 is absent")
    elif not isinstance(expected_compiler_sha256, str) or SHA256_PATTERN.fullmatch(expected_compiler_sha256) is None:
        raise CorrelatorError("expected_compiler_sha256: expected a 64-digit SHA-256")
    elif compiler_sha is not None and compiler_sha.lower() != expected_compiler_sha256.lower():
        reasons.append("authenticated compiler SHA-256 differs from the external expected digest")

    def check_raw_trace_anchor(
        label: str,
        key: str,
        path: str | Path | None,
        expected: str | None,
    ) -> None:
        if expected is None:
            reasons.append(f"external expected {label} trace SHA-256 is absent")
            return
        if SHA256_PATTERN.fullmatch(expected) is None:
            raise CorrelatorError(f"expected_{key}_trace_sha256: expected a 64-digit SHA-256")
        if path is None:
            reasons.append(f"{label} trace path is absent for raw-byte verification")
            return
        actual = _sha256(Path(path))
        if actual.lower() != expected.lower():
            reasons.append(f"{label} raw trace SHA-256 differs from the external expected digest")

    check_raw_trace_anchor(
        "allocator", "allocator", allocator_path, expected_allocator_trace_sha256
    )
    check_raw_trace_anchor("PCode v2", "pcode", pcode_path, expected_pcode_trace_sha256)
    check_raw_trace_anchor(
        "PCode v3", "pcode_v3", pcode_v3_path, expected_pcode_v3_trace_sha256
    )
    return not reasons, reasons, source_sha, compiler_sha


def _v3_authentication_gate(
    allocator: Mapping[str, Any],
    pcode_v3: Mapping[str, Any],
    *,
    primary: bool,
    allocator_path: str | Path | None,
    pcode_path: str | Path | None,
    pcode_v3_path: str | Path | None,
    expected_source_sha256: str | None,
    expected_compiler_sha256: str | None,
    expected_allocator_trace_sha256: str | None,
    expected_pcode_trace_sha256: str | None,
    expected_pcode_v3_trace_sha256: str | None,
    expected_ownership_sha256: str | None,
    ownership_path: str | Path | None,
    external_root: ExternalTrustRoot | None = None,
    external_trust_errors: Sequence[str] = (),
) -> tuple[bool, list[str], str | None, str | None, str | None]:
    """Authenticate a v3 direct-ownership packet.

    The v3 packet is diagnostic and intentionally has no v2 chronology.  Its
    direct map is usable only when the source/compiler/ownership provenance is
    explicit and the caller supplies external receipt hashes for the exact
    raw traces.  A failed v3 gate is reported as UNKNOWN by ``correlate``;
    unlike the legacy v2 gate it is not an exception path.
    """

    reasons: list[str] = list(external_trust_errors)
    contract = pcode_v3.get("v3_contract")
    if not isinstance(contract, Mapping):
        reasons.append("v3 ownership contract is absent")
    else:
        reasons.extend(str(item) for item in contract.get("reasons", []) if item)
        if contract.get("valid") is not True:
            reasons.append("v3 ownership contract is not authenticated")

    allocator_function = allocator.get("function")
    pcode_function = pcode_v3.get("function")
    if not isinstance(allocator_function, str) or not allocator_function:
        reasons.append("allocator function is absent")
    if not isinstance(pcode_function, str) or not pcode_function:
        reasons.append("PCode v3 function is absent")
    if (
        isinstance(allocator_function, str)
        and isinstance(pcode_function, str)
        and allocator_function != pcode_function
    ):
        reasons.append("allocator and PCode v3 functions differ")
    allocator_target = allocator.get("target")
    if (
        isinstance(allocator_target, str)
        and allocator_target
        and isinstance(allocator_function, str)
        and allocator_target != allocator_function
    ):
        reasons.append("allocator function and target differ")
    if str(allocator.get("status") or "").upper() != "CAPTURED":
        reasons.append("allocator trace status is not CAPTURED")
    if pcode_v3.get("version") != 3:
        reasons.append("v3 direct ownership requires a PCode v3 trace")

    authentication = pcode_v3.get("authentication")
    if not isinstance(authentication, Mapping):
        reasons.append("v3 authentication envelope is absent")
        authentication = {}
        authentication_fields: Mapping[str, Any] = {}
    else:
        authentication_status = authentication.get("status")
        if authentication_status != "AUTHENTICATED":
            reasons.append("v3 authentication status is not AUTHENTICATED")
        # Older v2-compatible envelopes expose this explicit marker.  It is
        # additive compatibility only; the v3 manifest status and artifact
        # hashes remain mandatory below.
        if "source_hash_authenticated" in authentication and authentication.get(
            "source_hash_authenticated"
        ) is not True:
            reasons.append("v3 source hash is not explicitly authenticated")
        raw_source_provenance = authentication.get("source_provenance")
        if raw_source_provenance is not None and (
            not isinstance(raw_source_provenance, str) or not raw_source_provenance
        ):
            reasons.append("v3 source provenance is absent")
        authentication_fields = _v3_provenance_fields(authentication)

        trace_function = pcode_v3.get("function")
        authenticated_function = authentication.get("function")
        if authenticated_function is not None and (
            not isinstance(authenticated_function, str)
            or not authenticated_function
            or (
                isinstance(trace_function, str)
                and authenticated_function != trace_function
            )
        ):
            reasons.append("v3 authentication function is not bound to the trace")

        sessions = [
            value.get("session")
            for value in (pcode_v3.get("frontend_join"), pcode_v3.get("ownership"))
            if isinstance(value, Mapping)
            and value.get("status") == "AUTHENTICATED"
        ]
        auth_session_id = authentication.get("session_id")
        auth_process_id = authentication.get("process_id")
        auth_source_path = authentication.get("source_path")
        auth_compiler_path = authentication.get("compiler_path")
        for session in sessions:
            if not isinstance(session, Mapping):
                continue
            if isinstance(auth_session_id, str) and auth_session_id:
                if session.get("session_id") != auth_session_id:
                    reasons.append("v3 authentication session is not bound to direct ownership")
            if isinstance(auth_process_id, int) and not isinstance(auth_process_id, bool):
                if session.get("process_id") != auth_process_id:
                    reasons.append("v3 authentication process is not bound to direct ownership")
            if isinstance(auth_source_path, str) and auth_source_path:
                if session.get("source") != auth_source_path:
                    reasons.append("v3 authentication source path is not bound to direct ownership")
            if isinstance(auth_compiler_path, str) and auth_compiler_path:
                if session.get("compiler") != auth_compiler_path:
                    reasons.append("v3 authentication compiler path is not bound to direct ownership")

        if external_root is not None:
            identity_pairs = (
                ("function", pcode_v3.get("function"), external_root.function),
                ("session_id", authentication.get("session_id"), external_root.session_id),
                ("process_id", authentication.get("process_id"), external_root.process_id),
                ("cwd", authentication.get("cwd"), external_root.cwd),
                ("argv", authentication.get("argv"), list(external_root.argv) if isinstance(external_root.argv, tuple) else external_root.argv),
            )
            for label, observed, expected in identity_pairs:
                expected_value = str(expected) if label == "cwd" and isinstance(expected, Path) else expected
                if observed != expected_value:
                    reasons.append(f"v3 authentication {label} differs from the external trust root")
            if authentication.get("function") != external_root.function:
                reasons.append("v3 authentication function differs from the external trust root")
            for session in sessions:
                if not isinstance(session, Mapping):
                    continue
                for label, observed, expected in (
                    ("session_id", session.get("session_id"), external_root.session_id),
                    ("process_id", session.get("process_id"), external_root.process_id),
                    ("function", session.get("function"), external_root.function),
                    ("source", session.get("source"), str(external_root.source_path)),
                    ("compiler", session.get("compiler"), str(external_root.compiler_path)),
                    ("cwd", session.get("cwd"), str(external_root.cwd)),
                    ("argv", session.get("argv"), list(external_root.argv) if isinstance(external_root.argv, tuple) else external_root.argv),
                ):
                    if observed != expected:
                        reasons.append(f"v3 ownership session {label} differs from the external trust root")

    # A payload-level provenance object is useful for reporting and conflict
    # detection, but it is not an authentication source.  Only artifact hashes
    # inside the authenticated envelope can pass this gate.
    fields = authentication_fields

    def one_value(key: str, label: str) -> str | None:
        values = [value for value in fields.get(key, []) if isinstance(value, str) and value]
        unique = sorted(set(values))
        if len(unique) > 1:
            reasons.append(f"v3 {label} provenance has conflicting values")
            return None
        return unique[0] if unique else None

    source_sha = one_value("source_sha256", "source")
    compiler_sha = one_value("compiler_sha256", "compiler")
    ownership_sha = one_value("ownership_sha256", "ownership")
    ownership_events_sha = one_value("ownership_events_sha256", "ownership events")
    manifest_sha = one_value("manifest_sha256", "manifest")
    source_provenance = one_value("source_provenance", "source")
    ownership_provenance = one_value("ownership_provenance", "ownership")
    if source_sha is None or SHA256_PATTERN.fullmatch(source_sha) is None:
        reasons.append("v3 manifest-bound source SHA-256 is absent or malformed")
        source_sha = None
    if compiler_sha is None or SHA256_PATTERN.fullmatch(compiler_sha) is None:
        reasons.append("v3 manifest-bound compiler SHA-256 is absent or malformed")
        compiler_sha = None
    if ownership_sha is None or SHA256_PATTERN.fullmatch(ownership_sha) is None:
        reasons.append("v3 manifest-bound ownership SHA-256 is absent or malformed")
        ownership_sha = None
    if manifest_sha is None or SHA256_PATTERN.fullmatch(manifest_sha) is None:
        reasons.append("v3 manifest SHA-256 is absent or malformed")
        manifest_sha = None
    if ownership_events_sha is None or SHA256_PATTERN.fullmatch(ownership_events_sha) is None:
        reasons.append("v3 ownership-events SHA-256 is absent or malformed")
        ownership_events_sha = None

    # Payload hashes are claims, never authority.  Every claim must bind to an
    # independently supplied canonical trust-root descriptor, including the
    # normalized v3 input and its two ownership artifacts.
    if external_root is None:
        reasons.append("external trust root is absent")
    else:
        trusted_hashes = {
            "source": external_root.source_sha256,
            "compiler": external_root.compiler_sha256,
            "ownership": external_root.ownership_sha256,
            "ownership events": external_root.ownership_events_sha256,
            "manifest": external_root.manifest_sha256,
        }
        payload_hashes = {
            "source": source_sha,
            "compiler": compiler_sha,
            "ownership": ownership_sha,
            "ownership events": ownership_events_sha,
            "manifest": manifest_sha,
        }
        for label, trusted in trusted_hashes.items():
            observed = payload_hashes[label]
            if not isinstance(trusted, str):
                reasons.append(f"external trust root {label} SHA-256 is absent")
            elif observed is None or observed.lower() != trusted.lower():
                reasons.append(f"v3 {label} hash does not match the external trust root")

        auth_path_fields = {
            "source": ("source_path", external_root.source_path, "source_size", external_root.source_size),
            "compiler": ("compiler_path", external_root.compiler_path, "compiler_size", external_root.compiler_size),
            "ownership": ("ownership_path", external_root.ownership_path, "ownership_size", external_root.ownership_size),
            "ownership events": ("ownership_events_path", external_root.ownership_events_path, "ownership_events_size", external_root.ownership_events_size),
        }
        for label, (path_key, trusted_path, size_key, trusted_size) in auth_path_fields.items():
            auth_path = authentication.get(path_key) if isinstance(authentication, Mapping) else None
            auth_size = authentication.get(size_key) if isinstance(authentication, Mapping) else None
            if not isinstance(trusted_path, (str, Path)) or not isinstance(auth_path, str):
                reasons.append(f"v3 {label} path is not externally bound")
            else:
                try:
                    expected_path = _canonical_external_path(trusted_path, f"{label} trust path")
                    actual_path = _canonical_external_path(auth_path, f"{label} payload path")
                    if expected_path != actual_path:
                        reasons.append(f"v3 {label} payload path differs from the external trust root")
                except CorrelatorError as error:
                    reasons.append(str(error))
            if isinstance(trusted_size, bool) or not isinstance(trusted_size, int) or auth_size != trusted_size:
                reasons.append(f"v3 {label} payload size differs from the external trust root")

        def check_payload_artifact(
            label: str,
            path_keys: Sequence[str],
            sha_keys: Sequence[str],
            size_keys: Sequence[str],
            trusted_path: str | Path | None,
            trusted_sha: str | None,
            trusted_size: int | None,
            *,
            required: bool,
        ) -> None:
            """Bind every payload spelling of one raw PCode artifact."""

            payload_path: Any = None
            payload_sha: Any = None
            payload_size: Any = None
            present = False
            for key in path_keys:
                if key in authentication:
                    present = True
                    candidate = authentication.get(key)
                    if payload_path is None:
                        payload_path = candidate
                    elif candidate != payload_path:
                        reasons.append(f"v3 {label} payload path aliases conflict")
            for key in sha_keys:
                if key in authentication:
                    present = True
                    candidate = authentication.get(key)
                    if payload_sha is None:
                        payload_sha = candidate
                    elif candidate != payload_sha:
                        reasons.append(f"v3 {label} payload SHA-256 aliases conflict")
            for key in size_keys:
                if key in authentication:
                    present = True
                    candidate = authentication.get(key)
                    if payload_size is None:
                        payload_size = candidate
                    elif candidate != payload_size:
                        reasons.append(f"v3 {label} payload size aliases conflict")
            if not required and not present:
                return
            if not isinstance(trusted_path, (str, Path)):
                reasons.append(f"external trust root {label} path is absent")
            elif not isinstance(payload_path, str):
                reasons.append(f"v3 {label} payload path is absent")
            else:
                try:
                    expected_path = _canonical_external_path(trusted_path, f"{label} trust path")
                    actual_path = _canonical_external_path(payload_path, f"{label} payload path")
                    if expected_path != actual_path:
                        reasons.append(f"v3 {label} payload path differs from the external trust root")
                except CorrelatorError as error:
                    reasons.append(str(error))
            if not isinstance(trusted_sha, str) or SHA256_PATTERN.fullmatch(trusted_sha) is None:
                reasons.append(f"external trust root {label} SHA-256 is absent")
            elif not isinstance(payload_sha, str) or SHA256_PATTERN.fullmatch(payload_sha) is None:
                reasons.append(f"v3 {label} payload SHA-256 is absent or malformed")
            elif payload_sha.lower() != trusted_sha.lower():
                reasons.append(f"v3 {label} payload SHA-256 differs from the external trust root")
            if isinstance(trusted_size, bool) or not isinstance(trusted_size, int):
                reasons.append(f"external trust root {label} size is absent or malformed")
            elif isinstance(payload_size, bool) or not isinstance(payload_size, int) or payload_size != trusted_size:
                reasons.append(f"v3 {label} payload size differs from the external trust root")

        check_payload_artifact(
            "PCode",
            ("pcode_path",),
            ("pcode_sha256",),
            ("pcode_size",),
            external_root.pcode_path,
            external_root.pcode_sha256,
            external_root.pcode_size,
            # The compatibility envelope may omit these self-describing
            # fields because a normalized packet cannot embed its own raw-byte
            # digest without a circular hash.  When a producer emits any
            # spelling, however, all fields become mandatory and are checked
            # against the independent root.
            required=any(
                key in authentication
                for key in ("pcode_path", "pcode_sha256", "pcode_size")
            ),
        )
        # In v3-primary mode this is a compatibility alias of the same input;
        # in v2-primary mode it is the required normalized-v3 sidecar.  Both
        # spellings are checked together so a forged alias cannot be omitted
        # or silently selected as the authority.
        normalized_required = any(
            key in authentication
            for key in (
                "normalized_v3_path",
                "normalized_v3_sha256",
                "normalized_v3_size",
                "pcode_v3_path",
                "pcode_v3_sha256",
                "pcode_v3_size",
            )
        )
        check_payload_artifact(
            "normalized v3",
            ("normalized_v3_path", "pcode_v3_path"),
            ("normalized_v3_sha256", "pcode_v3_sha256"),
            ("normalized_v3_size", "pcode_v3_size"),
            external_root.pcode_v3_path if not primary else (external_root.pcode_v3_path or external_root.pcode_path),
            external_root.pcode_v3_sha256 if not primary else (external_root.pcode_v3_sha256 or external_root.pcode_sha256),
            external_root.pcode_v3_size if not primary else (external_root.pcode_v3_size if external_root.pcode_v3_size is not None else external_root.pcode_size),
            required=normalized_required,
        )

        auth_manifest_path = authentication.get("manifest_path") if isinstance(authentication, Mapping) else None
        if auth_manifest_path is not None and isinstance(external_root.manifest_path, (str, Path)):
            try:
                if _canonical_external_path(auth_manifest_path, "manifest payload path") != _canonical_external_path(external_root.manifest_path, "manifest trust path"):
                    reasons.append("v3 manifest payload path differs from the external trust root")
            except CorrelatorError as error:
                reasons.append(str(error))
        for field, expected in (
            ("manifest_sha256", external_root.manifest_sha256),
            ("source_sha256", external_root.source_sha256),
            ("compiler_sha256", external_root.compiler_sha256),
            ("ownership_sha256", external_root.ownership_sha256),
            ("ownership_events_sha256", external_root.ownership_events_sha256),
        ):
            if isinstance(authentication, Mapping) and field in authentication and authentication.get(field) != expected:
                reasons.append(f"v3 authentication.{field} differs from the external trust root")
    if source_provenance is not None and source_sha is not None:
        # Preserve the v2 marker binding when a producer emits it, while
        # allowing manifest IDs that are deliberately opaque strings.
        marker = source_sha[:8].upper()
        if re.search(r"[0-9a-fA-F]{8}", source_provenance) and marker not in source_provenance.upper():
            reasons.append("v3 source provenance does not bind the source SHA-256 prefix")

    allocator_compiler = allocator.get("compiler")
    allocator_compiler_sha = (
        allocator_compiler.get("sha256") if isinstance(allocator_compiler, Mapping) else None
    )
    if not isinstance(allocator_compiler_sha, str) or SHA256_PATTERN.fullmatch(allocator_compiler_sha) is None:
        reasons.append("allocator compiler SHA-256 is absent or malformed")
    elif compiler_sha is not None and allocator_compiler_sha.lower() != compiler_sha.lower():
        reasons.append("allocator and v3 compiler SHA-256 values differ")

    trusted_source_sha256 = expected_source_sha256 or (external_root.source_sha256 if external_root else None)
    trusted_compiler_sha256 = expected_compiler_sha256 or (external_root.compiler_sha256 if external_root else None)
    trusted_ownership_sha256 = expected_ownership_sha256 or (external_root.ownership_sha256 if external_root else None)
    if trusted_source_sha256 is None:
        reasons.append("external expected source SHA-256 is absent")
    elif not isinstance(trusted_source_sha256, str) or SHA256_PATTERN.fullmatch(trusted_source_sha256) is None:
        raise CorrelatorError("expected_source_sha256: expected a 64-digit SHA-256")
    elif source_sha is not None and source_sha.lower() != trusted_source_sha256.lower():
        reasons.append("v3 source SHA-256 differs from the external expected digest")
    if trusted_compiler_sha256 is None:
        reasons.append("external expected compiler SHA-256 is absent")
    elif not isinstance(trusted_compiler_sha256, str) or SHA256_PATTERN.fullmatch(trusted_compiler_sha256) is None:
        raise CorrelatorError("expected_compiler_sha256: expected a 64-digit SHA-256")
    elif compiler_sha is not None and compiler_sha.lower() != trusted_compiler_sha256.lower():
        reasons.append("v3 compiler SHA-256 differs from the external expected digest")
    if trusted_ownership_sha256 is None:
        reasons.append("external expected ownership SHA-256 is absent")
    elif not isinstance(trusted_ownership_sha256, str) or SHA256_PATTERN.fullmatch(trusted_ownership_sha256) is None:
        raise CorrelatorError("expected_ownership_sha256: expected a 64-digit SHA-256")
    else:
        if ownership_sha is None:
            reasons.append("ownership provenance has no SHA-256 for external verification")
        elif ownership_sha.lower() != trusted_ownership_sha256.lower():
            reasons.append("v3 ownership SHA-256 differs from the external expected digest")
        if ownership_path is not None and trusted_ownership_sha256 is not None:
            actual = _sha256(Path(ownership_path))
            if actual.lower() != trusted_ownership_sha256.lower():
                reasons.append("ownership raw trace SHA-256 differs from the external expected digest")

    def check_raw_trace_anchor(
        label: str,
        key: str,
        path: str | Path | None,
        expected: str | None,
    ) -> None:
        if expected is None:
            reasons.append(f"external expected {label} trace SHA-256 is absent")
            return
        if SHA256_PATTERN.fullmatch(expected) is None:
            raise CorrelatorError(f"expected_{key}_trace_sha256: expected a 64-digit SHA-256")
        if path is None:
            reasons.append(f"{label} trace path is absent for raw-byte verification")
            return
        actual = _sha256(Path(path))
        if actual.lower() != expected.lower():
            reasons.append(f"{label} raw trace SHA-256 differs from the external expected digest")

    check_raw_trace_anchor(
        "allocator",
        "allocator",
        allocator_path,
        expected_allocator_trace_sha256 or (external_root.allocator_sha256 if external_root else None),
    )
    if primary:
        primary_expected = expected_pcode_trace_sha256 or expected_pcode_v3_trace_sha256 or (
            external_root.pcode_sha256 if external_root else None
        )
        check_raw_trace_anchor("PCode v3", "pcode", pcode_path, primary_expected)
    else:
        check_raw_trace_anchor(
            "PCode v3",
            "pcode_v3",
            pcode_v3_path,
            expected_pcode_v3_trace_sha256 or (external_root.pcode_v3_sha256 if external_root else None),
        )
    return not reasons, sorted(set(reasons)), source_sha, compiler_sha, ownership_sha


def _join_status(
    allocator_name: str,
    allocator_duplicates: set[str],
    profile: Mapping[str, Any] | None,
    inventory_row: Mapping[str, Any] | None,
    known_vregs: Mapping[str, Any],
    inventory_ambiguous: set[str],
    authentication_valid: bool,
    authentication_reasons: Sequence[str],
    *,
    direct_v3: bool = False,
) -> tuple[str, str, float, list[str]]:
    if allocator_name in allocator_duplicates:
        return (
            "AMBIGUOUS",
            "none",
            0.0,
            ["allocator name is duplicated; list order is not an authenticated identity"],
        )
    if allocator_name in inventory_ambiguous:
        return (
            "AMBIGUOUS",
            "none",
            0.0,
            ["source inventory name or vreg claim is duplicated"],
        )
    if direct_v3 and inventory_row is None:
        # A v3 memory-object/name overlap is never a fallback identity.  The
        # direct map must name this source object explicitly.
        return (
            "UNKNOWN",
            "none",
            0.0,
            ["v3 direct ownership has no source-inventory object for this allocator name"],
        )
    if inventory_row:
        declared = list(inventory_row.get("vreg_ids") or [])
        state = inventory_row.get("vreg_status")
        if direct_v3:
            if len(declared) != 1 or state != "AUTHENTICATED":
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    ["v3 direct ownership requires exactly one AUTHENTICATED vreg ID"]
                    + list(authentication_reasons),
                )
            if not authentication_valid:
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    ["v3 direct ownership failed provenance authentication"]
                    + list(authentication_reasons),
                )
            missing = sorted(set(declared) - set(known_vregs))
            if missing:
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    [f"authenticated v3 ownership names missing vregs: {', '.join(missing)}"],
                )
            # Unlike v2, chronology and an instruction-side vreg occurrence
            # are not required: the same-session direct edge is the evidence.
            return (
                "MATCHED_AUTHENTICATED",
                "high",
                1.0,
                ["v3 direct ownership explicitly authenticates the source-object vreg"],
            )
        if declared and state in AUTHENTICATED_VREG_STATUSES:
            if not authentication_valid:
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    ["source inventory binding failed provenance authentication"]
                    + list(authentication_reasons),
                )
            missing = sorted(set(declared) - set(known_vregs))
            if missing:
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    [f"authenticated source inventory names missing vregs: {', '.join(missing)}"],
                )
            unobserved = sorted(
                vreg_id
                for vreg_id in declared
                if known_vregs[vreg_id].get("instruction_count", 0) <= 0
                or known_vregs[vreg_id].get("chronology") is None
            )
            if unobserved:
                return (
                    "UNKNOWN",
                    "none",
                    0.0,
                    [
                        "authenticated source inventory names vregs without both chronology and instruction evidence: "
                        + ", ".join(unobserved)
                    ],
                )
            return (
                "MATCHED_AUTHENTICATED",
                "high",
                1.0,
                ["source inventory explicitly authenticates the vreg IDs"],
            )
        if declared and state not in AUTHENTICATED_VREG_STATUSES:
            return (
                "UNRESOLVED_EVIDENCE",
                "low",
                0.2,
                ["source inventory vreg_ids are present but not authenticated"],
            )
    if profile is None:
        return (
            "UNMATCHED_ALLOCATOR_OBJECT",
            "none",
            0.0,
            ["no shared PCode memory-object reference"],
        )
    vregs = list(profile.get("vreg_ids") or [])
    if not vregs:
        return (
            "UNRESOLVED_NO_VREG",
            "none",
            0.0,
            ["shared instruction references exist, but no initial-stage vreg is attached"],
        )
    if len(vregs) > 1:
        return (
            "UNRESOLVED_EVIDENCE",
            "low",
            0.2,
            [
                "multiple vregs share the memory-object fingerprint",
                "PCode useID/defID and frontend identity join are unavailable",
            ],
        )
    return (
        "UNRESOLVED_EVIDENCE",
        "low",
        0.2,
        [
            "one candidate vreg shares the memory-object fingerprint",
            "PCode useID/defID and frontend identity join are unavailable",
        ],
    )


def correlate(
    allocator_trace: Mapping[str, Any],
    pcode_trace: Mapping[str, Any],
    *,
    pcode_v3_trace: Mapping[str, Any] | None = None,
    trust_root: ExternalTrustRoot | None = None,
    allocator_path: str | Path | None = None,
    pcode_path: str | Path | None = None,
    pcode_v3_path: str | Path | None = None,
    expected_source_sha256: str | None = None,
    expected_source_size: int | None = None,
    source_path: str | Path | None = None,
    expected_compiler_sha256: str | None = None,
    expected_compiler_size: int | None = None,
    compiler_path: str | Path | None = None,
    expected_manifest_sha256: str | None = None,
    expected_manifest_size: int | None = None,
    manifest_path: str | Path | None = None,
    expected_allocator_trace_sha256: str | None = None,
    expected_allocator_trace_size: int | None = None,
    expected_pcode_trace_sha256: str | None = None,
    expected_pcode_trace_size: int | None = None,
    expected_pcode_v3_trace_sha256: str | None = None,
    expected_pcode_v3_trace_size: int | None = None,
    normalized_v3_path: str | Path | None = None,
    normalized_v3_sha256: str | None = None,
    normalized_v3_size: int | None = None,
    expected_ownership_sha256: str | None = None,
    expected_ownership_size: int | None = None,
    ownership_path: str | Path | None = None,
    expected_ownership_events_sha256: str | None = None,
    expected_ownership_events_size: int | None = None,
    ownership_events_path: str | Path | None = None,
    function: str | None = None,
    cwd: str | Path | None = None,
    argv: Sequence[str] | None = None,
    session_id: str | None = None,
    process_id: int | None = None,
) -> dict[str, Any]:
    """Build a deterministic, fail-closed VarInfo/PCode audit report.

    An authenticated source-inventory claim requires external expected
    SHA-256 values for the raw allocator and the applicable PCode trace.  The
    values must come from a trusted capture receipt and are checked against
    the exact bytes at ``*_path``; payload-provided or self-derived values are
    not provenance.  Version 3 direct ownership is diagnostic when any of
    those anchors or its manifest-bound provenance is absent.
    """

    if not isinstance(allocator_trace, Mapping) or not isinstance(pcode_trace, Mapping):
        raise CorrelatorError("allocator and PCode traces must be objects")
    _reject_nonfinite(allocator_trace, "allocator")
    _reject_nonfinite(pcode_trace, "pcode")
    _reject_pointer_material(allocator_trace, "allocator")
    _reject_pointer_material(pcode_trace, "pcode")
    optional_v3_pointer_error: CorrelatorError | None = None
    if pcode_v3_trace is not None:
        if not isinstance(pcode_v3_trace, Mapping):
            raise CorrelatorError("PCode v3 trace must be an object")
        _reject_nonfinite(pcode_v3_trace, "pcode.v3")
        try:
            _reject_pointer_material(pcode_v3_trace, "pcode.v3")
        except CorrelatorError as error:
            # An optional v3 sidecar must not disappear into the v2 path when
            # its pointer/text boundary is malformed.  Keep the error
            # location-only and route it through the structured UNKNOWN gate.
            if pcode_trace.get("schema") == "mwcc_gc26_pcode_trace/v2":
                optional_v3_pointer_error = error
            else:
                raise

    primary_schema = pcode_trace.get("schema")
    if primary_schema not in SUPPORTED_PCODE_SCHEMAS:
        raise CorrelatorError(f"pcode.schema: unsupported schema {primary_schema!r}")
    preflight_v3_errors: list[str] = []
    if optional_v3_pointer_error is not None:
        preflight_v3_errors.append(f"malformed optional v3 sidecar: {optional_v3_pointer_error}")
    try:
        normalized_input_path = _coalesce_anchor(pcode_v3_path, normalized_v3_path, "normalized v3 path")
        normalized_input_sha = _coalesce_anchor(
            expected_pcode_v3_trace_sha256,
            normalized_v3_sha256,
            "normalized v3 SHA-256",
        )
        normalized_input_size = _coalesce_anchor(
            expected_pcode_v3_trace_size,
            normalized_v3_size,
            "normalized v3 size",
        )
    except CorrelatorError as error:
        if pcode_v3_trace is not None or primary_schema == "mwcc_gc26_pcode_trace/v3":
            return _seal_report({
                "schema": SCHEMA_NAME,
                "status": "ERROR",
                "fail_closed": True,
                "authority_advanced": False,
                "error": {"type": type(error).__name__, "message": str(error)},
            })
        raise
    if pcode_v3_trace is not None and pcode_v3_trace.get("schema") != "mwcc_gc26_pcode_trace/v3":
        # A v2 sidecar in the v3 slot would silently downgrade the direct
        # ownership contract.  Reject it before any candidate is considered.
        preflight_v3_errors.append("pcode_v3_trace must use the normalized v3 schema")
    if primary_schema == "mwcc_gc26_pcode_trace/v3" and pcode_v3_trace is not None:
        if dict(pcode_v3_trace) != dict(pcode_trace):
            preflight_v3_errors.append("conflicting primary/alias v3 PCode payloads")

    try:
        root = _coerce_external_trust_root(
            trust_root,
            allocator_path=allocator_path,
            pcode_path=pcode_path,
            pcode_v3_path=normalized_input_path,
            ownership_path=ownership_path,
            primary_schema=primary_schema,
            expected_source_sha256=expected_source_sha256,
            expected_compiler_sha256=expected_compiler_sha256,
            expected_allocator_trace_sha256=expected_allocator_trace_sha256,
            expected_allocator_trace_size=expected_allocator_trace_size,
            expected_pcode_trace_sha256=expected_pcode_trace_sha256,
            expected_pcode_trace_size=expected_pcode_trace_size,
            expected_pcode_v3_trace_sha256=normalized_input_sha,
            expected_pcode_v3_trace_size=normalized_input_size,
            expected_ownership_sha256=expected_ownership_sha256,
        )
        # Fill the explicit artifact aliases that are not part of the legacy
        # coalescer without allowing a primary/alias disagreement.
        if root is not None:
            root = ExternalTrustRoot(
                **{
                    **root.__dict__,
                    "manifest_path": _coalesce_anchor(root.manifest_path, manifest_path, "manifest path"),
                    "manifest_sha256": _coalesce_anchor(root.manifest_sha256, expected_manifest_sha256, "manifest SHA-256"),
                    "manifest_size": _coalesce_anchor(root.manifest_size, expected_manifest_size, "manifest size"),
                    "source_path": _coalesce_anchor(root.source_path, source_path, "source path"),
                    "source_size": _coalesce_anchor(root.source_size, expected_source_size, "source size"),
                    "compiler_path": _coalesce_anchor(root.compiler_path, compiler_path, "compiler path"),
                    "compiler_size": _coalesce_anchor(root.compiler_size, expected_compiler_size, "compiler size"),
                    "ownership_size": _coalesce_anchor(root.ownership_size, expected_ownership_size, "ownership size"),
                    "ownership_events_path": _coalesce_anchor(root.ownership_events_path, ownership_events_path, "ownership events path"),
                    "ownership_events_sha256": _coalesce_anchor(root.ownership_events_sha256, expected_ownership_events_sha256, "ownership events SHA-256"),
                    "ownership_events_size": _coalesce_anchor(root.ownership_events_size, expected_ownership_events_size, "ownership events size"),
                    "function": _coalesce_anchor(root.function, function, "function"),
                    "cwd": _coalesce_anchor(root.cwd, cwd, "cwd"),
                    "argv": _coalesce_anchor(root.argv, argv, "argv"),
                    "session_id": _coalesce_anchor(root.session_id, session_id, "session_id"),
                    "process_id": _coalesce_anchor(root.process_id, process_id, "process_id"),
                }
            )
    except CorrelatorError as error:
        # A caller-provided trust-root conflict is itself evidence failure,
        # not an unchecked exception path.  Keep the legacy fail-fast
        # behaviour for the older digest-only API, whose callers rely on
        # CorrelatorError for malformed command-line anchors.
        if (
            trust_root is not None
            or primary_schema == "mwcc_gc26_pcode_trace/v3"
            or pcode_v3_trace is not None
        ):
            return _seal_report({
                "schema": SCHEMA_NAME,
                "status": "ERROR",
                "fail_closed": True,
                "authority_advanced": False,
                "error": {"type": type(error).__name__, "message": str(error)},
            })
        raise
    trust_errors = _validate_external_trust_root(
        root,
        direct_v3=(primary_schema == "mwcc_gc26_pcode_trace/v3" or pcode_v3_trace is not None),
        primary_v3=primary_schema == "mwcc_gc26_pcode_trace/v3",
        allocator_path=allocator_path,
        pcode_path=pcode_path,
        pcode_v3_path=normalized_input_path,
        ownership_path=ownership_path,
    )
    trust_errors.extend(preflight_v3_errors)

    _require_trace_mapping_matches_path(
        allocator_trace,
        allocator_path,
        root.allocator_sha256 if root is not None else expected_allocator_trace_sha256,
        "allocator",
    )
    primary_expected_pcode_sha256 = expected_pcode_trace_sha256
    if primary_schema == "mwcc_gc26_pcode_trace/v3" and primary_expected_pcode_sha256 is None:
        # ``--pcode`` may now be the v3 primary input.  Keep the historical
        # v2 argument name while accepting the explicit v3 anchor alias.
        primary_expected_pcode_sha256 = normalized_input_sha
    _require_trace_mapping_matches_path(
        pcode_trace,
        pcode_path,
        (
            root.pcode_sha256
            if root is not None and root.pcode_sha256 is not None
            else primary_expected_pcode_sha256
        ),
        "PCode v3" if primary_schema == "mwcc_gc26_pcode_trace/v3" else "PCode v2",
    )
    if pcode_v3_trace is not None:
        _require_trace_mapping_matches_path(
            pcode_v3_trace,
            normalized_input_path,
            (
                root.pcode_v3_sha256
                if root is not None and root.pcode_v3_sha256 is not None
                else normalized_input_sha
            ),
            "PCode v3",
        )

    allocator = _validate_allocator(allocator_trace)

    def invalid_v3(value: Mapping[str, Any], error: BaseException) -> dict[str, Any]:
        # Keep the report shape usable for diagnostic callers, but make every
        # candidate UNKNOWN.  A malformed v3 packet must not become an
        # exception-driven downgrade or a partially authenticated map.
        return {
            "schema": value.get("schema") if isinstance(value.get("schema"), str) else "mwcc_gc26_pcode_trace/v3",
            "version": 3,
            "function": value.get("function") if isinstance(value.get("function"), str) else None,
            "status": value.get("status") if isinstance(value.get("status"), str) else "UNKNOWN",
            "capture_status": None,
            "authentication": value.get("authentication") if isinstance(value.get("authentication"), Mapping) else {},
            "stage": "backend-00-initial-code.pcode.json",
            "instructions": [],
            "profiles": {},
            "vregs": {},
            "limitations": [f"malformed v3 input: {error}"],
            "capture": value,
            "v3_contract": {
                "valid": False,
                "reasons": [f"malformed v3 input: {error}"],
                "top_level_status": value.get("status"),
                "pcode_status": value.get("pcode_status"),
                "source_inventory": {"status": None, "reason": str(error), "locals": [], "arguments": [], "objects": []},
                "source_rows": [],
                "frontend_join": value.get("frontend_join"),
                "ownership": value.get("ownership"),
                "direct_rows": {},
                "provenance_fields": {},
            },
        }

    allocator_names = {local["name"] for local in allocator["locals"]}
    optional_v3_validation_error: CorrelatorError | None = None
    try:
        pcode = _validate_pcode(pcode_trace, allocator_names)
    except CorrelatorError as error:
        if primary_schema == "mwcc_gc26_pcode_trace/v3":
            pcode = invalid_v3(pcode_trace, error)
        else:
            raise
    pcode_v3 = None
    if pcode_v3_trace is not None:
        try:
            pcode_v3 = _validate_pcode(pcode_v3_trace, allocator_names)
        except CorrelatorError as error:
            optional_v3_validation_error = error
            pcode_v3 = invalid_v3(pcode_v3_trace, error)

    # Preserve the v2 path exactly when no v3 source-object inventory is
    # present.  A v3 packet (primary or optional) takes precedence only when
    # it actually carries the local/argument ownership contract.
    pcode_v3_candidate = pcode if pcode.get("version") == 3 else pcode_v3
    v3_contract = (
        pcode_v3_candidate.get("v3_contract")
        if isinstance(pcode_v3_candidate, Mapping)
        else None
    )
    v3_source_rows = (
        list(v3_contract.get("source_rows") or [])
        if isinstance(v3_contract, Mapping)
        else []
    )
    v3_capture = (
        pcode_v3_candidate.get("capture")
        if isinstance(pcode_v3_candidate, Mapping)
        else None
    )
    v3_inventory_present = isinstance(v3_capture, Mapping) and isinstance(
        v3_capture.get("source_inventory"), Mapping
    )
    # A v3 primary is always handled by the diagnostic direct path, even when
    # its inventory is malformed or absent.  An optional v3 packet enters the
    # path when it carries an inventory envelope; a minimal legacy diagnostic
    # v3 sidecar remains harmlessly additive to the v2 flow.
    optional_v3_contract = (
        pcode_v3.get("v3_contract")
        if isinstance(pcode_v3, Mapping)
        else None
    )
    optional_v3_shape_invalid = isinstance(optional_v3_contract, Mapping) and optional_v3_contract.get(
        "shape_valid"
    ) is False
    optional_v3_claimed_contract = isinstance(pcode_v3_trace, Mapping) and any(
        key in pcode_v3_trace
        for key in (
            "pcode_status",
            "provenance",
            "authentication",
            "source_inventory",
            "frontend_join",
            "ownership",
        )
    )
    optional_v3_contract_invalid = (
        optional_v3_claimed_contract
        and isinstance(optional_v3_contract, Mapping)
        and optional_v3_contract.get("valid") is not True
    )
    optional_v3_invalid = (
        optional_v3_validation_error is not None
        or optional_v3_shape_invalid
        or optional_v3_contract_invalid
        or bool(preflight_v3_errors)
    )
    direct_v3 = (
        pcode.get("version") == 3
        or v3_inventory_present
        or optional_v3_invalid
    )
    if direct_v3:
        inventory = {}
        inventory_ambiguous: set[str] = set()
        for row in v3_source_rows:
            name = row["name"]
            if name in inventory:
                inventory_ambiguous.add(name)
            else:
                inventory[name] = dict(row)
        # Keep a duplicate name visible to the joiner while making its status
        # ambiguous; direct ordinal ownership remains preserved in the report.
        for name in inventory_ambiguous:
            inventory.pop(name, None)
    else:
        inventory, inventory_ambiguous = _allocator_source_inventory(allocator, pcode)

    source_sha: str | None = None
    compiler_sha: str | None = None
    ownership_sha: str | None = None
    if direct_v3:
        v3_authentication_valid, v3_authentication_reasons, source_sha, compiler_sha, ownership_sha = _v3_authentication_gate(
            allocator,
            pcode_v3_candidate,
            primary=pcode.get("version") == 3,
            allocator_path=allocator_path,
            pcode_path=pcode_path,
            pcode_v3_path=normalized_input_path,
            expected_source_sha256=expected_source_sha256,
            expected_compiler_sha256=expected_compiler_sha256,
            expected_allocator_trace_sha256=expected_allocator_trace_sha256,
            expected_pcode_trace_sha256=primary_expected_pcode_sha256,
            expected_pcode_v3_trace_sha256=normalized_input_sha,
            expected_ownership_sha256=expected_ownership_sha256,
            ownership_path=ownership_path,
            external_root=root,
            external_trust_errors=trust_errors,
        )
        authentication_valid = v3_authentication_valid
        authentication_reasons = v3_authentication_reasons
    else:
        authentication_valid, authentication_reasons, source_sha, compiler_sha = _authentication_gate(
            allocator,
            pcode,
            pcode_v3=pcode_v3,
            allocator_path=allocator_path,
            pcode_path=pcode_path,
            pcode_v3_path=pcode_v3_path,
            expected_source_sha256=expected_source_sha256,
            expected_compiler_sha256=expected_compiler_sha256,
            expected_allocator_trace_sha256=expected_allocator_trace_sha256,
            expected_pcode_trace_sha256=expected_pcode_trace_sha256,
            expected_pcode_v3_trace_sha256=expected_pcode_v3_trace_sha256,
        )
        authenticated_claims = [
            row
            for row in inventory.values()
            if row.get("vreg_ids") and row.get("vreg_status") in AUTHENTICATED_VREG_STATUSES
        ]
        # Keep v2's historical fail-fast contract.  v3 direct ownership uses
        # diagnostic UNKNOWN rows instead (handled by _join_status).
        if authenticated_claims and not authentication_valid:
            raise CorrelatorError(
                "authenticated source-inventory claim failed provenance gate: "
                + "; ".join(authentication_reasons)
            )

    known_vregs: dict[str, dict[str, Any]] = dict(pcode["vregs"])
    if direct_v3 and isinstance(v3_contract, Mapping) and v3_contract.get("valid") is True:
        for vreg_id in sorted(set(v3_contract.get("direct_rows", {}).values())):
            known_vregs.setdefault(vreg_id, _vreg_fingerprint(vreg_id, None, [], []))
    allocator_name_set = {local["name"] for local in allocator["locals"]}
    mappings: list[dict[str, Any]] = []
    for local in allocator["locals"]:
        name = local["name"]
        profile = pcode["profiles"].get(name)
        status, confidence, score, reasons = _join_status(
            name,
            set(allocator["duplicate_names"]),
            profile,
            inventory.get(name),
            known_vregs,
            inventory_ambiguous,
            authentication_valid,
            authentication_reasons,
            direct_v3=direct_v3,
        )
        mapping: dict[str, Any] = {
            "allocator": {
                "name": name,
                "compiler_list_order": local["compiler_list_order"],
                "datatype": local.get("datatype"),
                "type_code": local.get("type_code"),
                "known_varinfo": local.get("known_varinfo", {}),
            },
            "status": status,
            "confidence": {"label": confidence, "score": score},
            "reasons": reasons,
        }
        if profile is not None:
            mapping["pcode_fingerprint"] = profile
        if inventory.get(name) is not None:
            mapping["source_inventory"] = inventory[name]
            if direct_v3:
                mapping["owned_vreg_ids"] = list(inventory[name].get("vreg_ids") or [])
            if status == "MATCHED_AUTHENTICATED" and profile is not None:
                declared = set(inventory[name].get("vreg_ids") or [])
                observed = set(profile.get("vreg_ids") or [])
                extra_candidates = sorted(observed - declared)
                if extra_candidates:
                    mapping["unbound_candidate_vregs"] = extra_candidates
                    mapping["reasons"].append(
                        "additional shared-instruction vregs remain unbound by the authenticated inventory"
                    )
        mappings.append(mapping)

    referenced_vregs = {
        vreg_id
        for profile in pcode["profiles"].values()
        for vreg_id in profile.get("vreg_ids", [])
    }
    if direct_v3 and isinstance(v3_contract, Mapping) and v3_contract.get("valid") is True:
        referenced_vregs.update(v3_contract.get("direct_rows", {}).values())
    unmatched_vregs = [
        {
            **known_vregs[vreg_id],
            "status": "UNMATCHED_PCODE_VREG",
            "reasons": [
                "vreg occurs in PCode but no allocator memory-object fingerprint names it",
                "no source-object identity is available to infer ownership",
            ],
        }
        for vreg_id in sorted(set(known_vregs) - referenced_vregs)
    ]

    v3_inventory_report: dict[str, Any] | None = None
    v3_ownership_report: dict[str, Any] | None = None
    v3_frontend_report: dict[str, Any] | None = None
    v3_source_object_mappings: list[dict[str, Any]] = []
    if direct_v3 and isinstance(v3_contract, Mapping):
        raw_inventory = v3_contract.get("source_inventory")
        if isinstance(raw_inventory, Mapping):
            v3_inventory_report = {
                "status": raw_inventory.get("status"),
                "reason": raw_inventory.get("reason"),
                "locals": raw_inventory.get("locals", []),
                "arguments": raw_inventory.get("arguments", []),
            }
        for label, key in (("frontend_join", "frontend"), ("ownership", "ownership")):
            value = v3_contract.get(label)
            if isinstance(value, Mapping):
                normalized = {
                    "status": value.get("status"),
                    "reason": value.get("reason"),
                    "direct_object_vregs": value.get("direct_object_vregs", []),
                }
                if key == "frontend":
                    v3_frontend_report = normalized
                else:
                    v3_ownership_report = normalized
        direct_by_key = {
            (kind, ordinal): vreg_id
            for (kind, ordinal), vreg_id in (v3_contract.get("direct_rows") or {}).items()
        }
        for source_row in v3_contract.get("source_rows", []):
            ordinal = source_row.get("ordinal")
            kind = source_row.get("kind")
            v3_source_object_mappings.append(
                {
                    "kind": source_row.get("kind"),
                    "ordinal": ordinal,
                    "name": source_row.get("name"),
                    "vreg_ids": list(source_row.get("vreg_ids") or []),
                    "status": (
                        "MATCHED_AUTHENTICATED"
                        if authentication_valid and (kind, ordinal) in direct_by_key
                        else "UNKNOWN"
                    ),
                    "reasons": ([] if authentication_valid else list(authentication_reasons)),
                }
            )

    limitations = [
        "This is an audit-only report; it never invents an object-to-vreg join.",
        "Allocator process pointers, list nodes, and image bases are not stable identity keys.",
        "Compiler list order is retained as evidence but is not used as identity.",
        "A shared memory-object label is a candidate fingerprint, not authenticated frontend identity.",
        "A resolved mapping requires an authenticated source_inventory vreg_ids binding.",
    ]
    limitations.extend(allocator["limitations"])
    limitations.extend(pcode["limitations"])
    if direct_v3 and isinstance(pcode_v3_candidate, Mapping):
        limitations.append("PCode v3 has no chronology/liveness identity; direct ownership is the only v3 join path.")
        limitations.extend(pcode_v3_candidate["limitations"])
    elif pcode_v3:
        limitations.append("PCode v3 has no chronology/liveness identity; direct ownership is the only v3 join path.")
        limitations.extend(pcode_v3["limitations"])
    if direct_v3:
        limitations.append("v3 local/argument ownership is capture-local and requires strict one-to-one ordinals.")
    limitations = sorted(set(str(item) for item in limitations))

    status_counts = Counter(mapping["status"] for mapping in mappings)
    authentication = pcode.get("authentication")
    artifacts = authentication.get("artifacts") if isinstance(authentication, Mapping) else {}
    artifacts = artifacts if isinstance(artifacts, Mapping) else {}

    def safe_path_sha(path: str | Path | None) -> str | None:
        if path is None:
            return None
        try:
            return _sha256(Path(path))
        except CorrelatorError:
            return None

    def artifact_sha(name: str) -> str | None:
        artifact = artifacts.get(name)
        return artifact.get("sha256") if isinstance(artifact, Mapping) and isinstance(artifact.get("sha256"), str) else None

    report: dict[str, Any] = {
        "schema": SCHEMA_NAME,
        "status": "AUDIT_ONLY",
        "fail_closed": True,
        "authority_advanced": False,
        "function": allocator["function"] or pcode["function"],
        "provenance": {
            "allocator_trace_sha256": safe_path_sha(allocator_path),
            "pcode_trace_sha256": safe_path_sha(pcode_path),
            "pcode_v3_trace_sha256": (
                safe_path_sha(normalized_input_path)
                if pcode_v3_path
                else safe_path_sha(pcode_path)
                if direct_v3 and pcode.get("version") == 3 and pcode_path
                else None
            ),
            "allocator_schema": allocator["schema"],
            "pcode_schema": pcode["schema"],
            "pcode_v3_schema": (
                pcode_v3_candidate["schema"]
                if direct_v3 and isinstance(pcode_v3_candidate, Mapping)
                else pcode_v3["schema"]
                if pcode_v3
                else None
            ),
            "source_sha256": source_sha or artifact_sha("source"),
            "compiler_sha256": compiler_sha or artifact_sha("compiler"),
            "ownership_sha256": ownership_sha,
        },
        "authentication_gate": {
            "valid": authentication_valid,
            "reasons": authentication_reasons,
        },
        "summary": {
            "allocator_object_count": len(mappings),
            "pcode_vreg_count": len(known_vregs),
            "shared_memory_object_count": sum(1 for name in allocator_name_set if name in pcode["profiles"]),
            "pcode_vreg_candidate_count": len(referenced_vregs),
            "unmatched_pcode_vreg_count": len(unmatched_vregs),
            "mapping_status_counts": dict(sorted(status_counts.items())),
        },
        "mappings": mappings,
        "unmatched_pcode_vregs": unmatched_vregs,
        "limitations": limitations,
    }
    if v3_inventory_report is not None:
        report["source_inventory"] = v3_inventory_report
        report["source_object_mappings"] = v3_source_object_mappings
    if v3_frontend_report is not None:
        report["frontend_join"] = v3_frontend_report
    if v3_ownership_report is not None:
        report["ownership"] = v3_ownership_report
    return _seal_report(report)


def human_summary(report: Mapping[str, Any]) -> str:
    """Render a compact human summary without hiding unresolved rows."""

    summary = _mapping(report.get("summary"), "report.summary")
    lines = [
        f"{report.get('function') or '<unknown>'}: {report.get('status')} (fail_closed={report.get('fail_closed')})",
        "allocator objects={allocator_object_count} pcode vregs={pcode_vreg_count} shared={shared_memory_object_count} candidates={pcode_vreg_candidate_count} unmatched_vregs={unmatched_pcode_vreg_count}".format(**summary),
        "statuses: " + ", ".join(f"{key}={value}" for key, value in sorted((summary.get("mapping_status_counts") or {}).items())),
    ]
    for row in report.get("mappings", []):
        allocator = row.get("allocator", {})
        profile = row.get("pcode_fingerprint") or {}
        vregs = ",".join(profile.get("vreg_ids", [])) or "-"
        lines.append(
            f"  {allocator.get('compiler_list_order', '?'):>3} {allocator.get('name', '<unknown>'):<24} {row.get('status', 'UNKNOWN'):<24} vregs={vregs} refs={profile.get('instruction_count', 0)}"
        )
    if report.get("unmatched_pcode_vregs"):
        lines.append(f"unmatched PCode vregs: {len(report['unmatched_pcode_vregs'])}")
    return "\n".join(lines)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("--allocator", required=True, type=Path, help="mwcc_allocator_trace/v1 JSON")
    parser.add_argument("--pcode", required=True, type=Path, help="mwcc_gc26_pcode_trace/v2 or v3 JSON")
    parser.add_argument("--pcode-v3", type=Path, help="optional diagnostic v3 serial PCode JSON")
    parser.add_argument(
        "--expected-source-sha256",
        help="external expected SHA-256 for the exact source used by the trace",
    )
    parser.add_argument(
        "--expected-compiler-sha256",
        help="external expected SHA-256 for the compiler used by both traces",
    )
    parser.add_argument(
        "--expected-allocator-trace-sha256",
        help="trusted capture-receipt SHA-256 for exact allocator trace bytes",
    )
    parser.add_argument(
        "--expected-pcode-trace-sha256",
        help="trusted capture-receipt SHA-256 for exact v2 PCode trace bytes",
    )
    parser.add_argument(
        "--expected-pcode-v3-trace-sha256",
        help="trusted capture-receipt SHA-256 for exact v3 trace bytes",
    )
    parser.add_argument(
        "--expected-ownership-sha256",
        help="trusted capture-receipt SHA-256 for the v3 ownership manifest/sidecar",
    )
    output = parser.add_mutually_exclusive_group()
    output.add_argument("--json", action="store_true", help="emit compact JSON (default)")
    output.add_argument("--human", action="store_true", help="emit a compact human summary")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        allocator = load_json(args.allocator)
        pcode = load_json(args.pcode)
        pcode_v3 = load_json(args.pcode_v3) if args.pcode_v3 else None
        report = correlate(
            allocator,
            pcode,
            pcode_v3_trace=pcode_v3,
            allocator_path=args.allocator,
            pcode_path=args.pcode,
            pcode_v3_path=args.pcode_v3,
            expected_source_sha256=args.expected_source_sha256,
            expected_compiler_sha256=args.expected_compiler_sha256,
            expected_allocator_trace_sha256=args.expected_allocator_trace_sha256,
            expected_pcode_trace_sha256=args.expected_pcode_trace_sha256,
            expected_pcode_v3_trace_sha256=args.expected_pcode_v3_trace_sha256,
            expected_ownership_sha256=args.expected_ownership_sha256,
        )
    except (CorrelatorError, OSError, TypeError, ValueError) as error:
        failure = {
            "schema": SCHEMA_NAME,
            "status": "ERROR",
            "fail_closed": True,
            "authority_advanced": False,
            "error": {"type": type(error).__name__, "message": str(error)},
        }
        failure = _seal_report(failure)
        print(json.dumps(failure, sort_keys=True, separators=(",", ":")))
        return 2
    if args.human:
        print(human_summary(report))
    else:
        print(json.dumps(report, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
