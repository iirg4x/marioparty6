#!/usr/bin/env python3
"""Build a bounded, target-first reconstruction packet for one owner.

The packet is deliberately a *reconstruction aid*, not a source oracle.  It
starts with the bytes which are already available in a ``focus_symbol_report``
and keeps the target/candidate context needed to reason about a natural C
source boundary.  Donor/history records are neither accepted nor required.

The module is pure by default: :func:`build_packet` consumes mappings and
returns a self-hashed mapping.  It does not compile, edit source, retain a
candidate, or advance authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any


SCHEMA = "owner_campaign_reconstruction_packet/v1"
SCHEMA_VERSION = 1
MAX_OUTPUT_BYTES = 256 * 1024
DEFAULT_WINDOW_RADIUS = 8
DEFAULT_CLUSTER_GAP = 8
MAX_WINDOW_RADIUS = 64
MAX_CLUSTER_GAP = 256
MAX_RESIDUAL_ROWS = 4096
MAX_WINDOWS = 512
MAX_SUMMARY_ENTRIES = 1024
MAX_RELOCATION_CONTEXT = 64
MAX_TEXT_BYTES = 512
MAX_NESTED_VALUE_BYTES = 8192

# A large function is useful as a pivot signal, but its complete focus report
# is not useful as a reconstruction packet.  Keep this budget deliberately
# small and deterministic.  These limits apply only to packets whose residual
# event/cluster topology already requires UNKNOWN; READY packets are never
# silently truncated.
MAX_BROAD_EVENTS = 8
MAX_BROAD_CLUSTERS = 8
# A broad packet carries identifiers as evidence, not as a replacement for the
# producer's full report.  Keep the retained census deliberately small: row
# identities may be long (the production target-anchored form contains a
# canonical instruction digest), and the bounded packet must remain below the
# 256 KiB contract even for the legal 4096-row input limit.
MAX_BROAD_IDS = 16
MAX_BROAD_CLUSTER_IDS = 4
MAX_BROAD_SUMMARY_ENTRIES = 8
MAX_BROAD_STACK_HOMES = 16
MAX_BROAD_WINDOW_RADIUS = 4
MAX_BROAD_WINDOWS = 8
MAX_BROAD_PHYSICAL_DIFFERENCES = 8
MAX_READY_EVENTS = 64
MAX_READY_CLUSTERS = 3

_PACKET_KEYS = frozenset(
    {
        "schema",
        "schema_version",
        "owner",
        "unit",
        "function",
        "source_path",
        "source_sha256",
        "base_commit",
        "target_object_sha256",
        "candidate_object_sha256",
        "toolchain_sha256",
        "frontier_source_sha256",
        "parent_frontier_sha256",
        "source_span",
        "source_span_sha256",
        "focus_artifact_sha256",
        "focus_report_schema",
        "strict_residuals",
        "data_residuals",
        "strict_residual_count",
        "data_residual_count",
        "strict_residuals_complete",
        "strict_residuals_total_count",
        "strict_residuals_full_sha256",
        "strict_residuals_omitted_count",
        "strict_residuals_omitted_sha256",
        "data_residuals_complete",
        "data_residuals_total_count",
        "data_residuals_full_sha256",
        "data_residuals_omitted_count",
        "data_residuals_omitted_sha256",
        "residual_event_count",
        "residual_rows_complete",
        "residual_rows_total_count",
        "residual_rows_full_sha256",
        "residual_rows_omitted_count",
        "residual_rows_omitted_sha256",
        "residual_rows",
        "causal_clusters_complete",
        "causal_clusters_total_count",
        "causal_clusters_full_sha256",
        "causal_clusters_omitted_count",
        "causal_clusters_omitted_sha256",
        "causal_clusters_retained_sha256",
        "causal_clusters",
        "causal_cluster_count",
        "selected_cluster_count",
        "instruction_windows_complete",
        "instruction_windows",
        "decomposition_regions",
        "status",
        "exact_terminal_possible",
        "exact_terminal_reason",
        "target_first_signal",
        "control_flow",
        "stack_relative",
        "machine_summary",
        "physical_relocations",
        "physical_relocation_differences",
        "physical_difference_ids",
        "physical_difference_ids_sha256",
        "reconstruction_policy",
        "authority_advanced",
        "diagnostic_only",
        "packet_sha256",
    }
)
_SIGNAL_KEYS = frozenset(
    {
        "status",
        "reason",
        "exact_terminal_possible",
        "exact_terminal_reason",
        "first_residual_index",
        "cluster_count",
        "owner_inference",
        "next_action",
        "decomposition_required",
        "decomposition_regions",
    }
)
_POLICY_KEYS = frozenset(
    {
        "target_first",
        "donor_required",
        "history_required",
        "window_radius",
        "cluster_gap",
        "source_text_emitted",
        "source_patch_emitted",
        "compile_authorized",
        "broad_residual_requires_decomposition",
    }
)
_PACKET_OPTIONAL_KEYS = frozenset(
    {
        # These census fields were added after the initial packet producer.
        # READY packets from that producer remain readable; newly generated
        # packets always emit the complete channel census, and UNKNOWN broad
        # packets require it in verify_packet().
        "strict_residuals_complete",
        "strict_residuals_total_count",
        "strict_residuals_full_sha256",
        "strict_residuals_omitted_count",
        "strict_residuals_omitted_sha256",
        "data_residuals_complete",
        "data_residuals_total_count",
        "data_residuals_full_sha256",
        "data_residuals_omitted_count",
        "data_residuals_omitted_sha256",
        "residual_rows_omitted_count",
        "residual_rows_omitted_sha256",
        "causal_clusters_omitted_count",
        "causal_clusters_omitted_sha256",
        "causal_clusters_retained_sha256",
    }
)
_PACKET_REQUIRED_KEYS = _PACKET_KEYS - _PACKET_OPTIONAL_KEYS

_SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
_COMMIT_RE = re.compile(r"^[0-9a-f]{40}$")
_MNEMONIC_RE = re.compile(r"^\s*([A-Za-z][A-Za-z0-9_.]*)")
_STACK_RE = re.compile(
    r"(?P<offset>[+-]?(?:0[xX][0-9a-fA-F]+|\d+))\s*\(\s*(?P<base>r1|sp)\s*\)",
    re.IGNORECASE,
)
_REGISTER_RE = re.compile(r"\b[rf](?:[0-9]|[12][0-9]|3[01])\b", re.IGNORECASE)
_FUNCTION_TOKEN_RE = re.compile(r"(?<![A-Za-z0-9_$])(?P<name>[A-Za-z_][A-Za-z0-9_$]*)\s*\(")

_CALL_MNEMONICS = frozenset({"bl", "bla", "blrl", "bctrl", "bctrl."})
_BRANCH_MNEMONICS = frozenset(
    {
        "b",
        "ba",
        "bc",
        "bca",
        "bcl",
        "bcla",
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
        "bdnz",
        "bdz",
        "bso",
        "bns",
        "bclr",
        "bcctr",
        "bflr",
    }
)
_STORE_PREFIXES = ("st", "psq_st", "stmw", "stmw.")
_LOAD_PREFIXES = ("l", "psq_l", "lmw", "lmw.")


class ReconstructionPacketError(ValueError):
    """The supplied focus evidence cannot safely produce a packet."""

    def __init__(self, message: str, *, code: str = "invalid_input") -> None:
        super().__init__(message)
        self.code = code


def _canonical(value: Any) -> bytes:
    try:
        return json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise ReconstructionPacketError(
            f"value is not canonical JSON: {exc}", code="noncanonical_input"
        ) from exc


def canonical_json(value: Any) -> bytes:
    """Return the exact compact JSON representation used for all digests."""

    return _canonical(value)


def canonical_sha256(value: Any) -> str:
    """Hash a JSON value using the packet's deterministic canonical encoding."""

    return hashlib.sha256(_canonical(value)).hexdigest()


def _mask_c_comments_and_literals(source_text: str) -> str:
    """Mask C comments/strings while preserving offsets and newlines."""

    chars = list(source_text)
    state = "normal"
    index = 0
    while index < len(chars):
        current = chars[index]
        following = chars[index + 1] if index + 1 < len(chars) else ""
        if state == "normal":
            if current == "/" and following == "/":
                chars[index] = chars[index + 1] = " "
                index += 2
                state = "line_comment"
                continue
            if current == "/" and following == "*":
                chars[index] = chars[index + 1] = " "
                index += 2
                state = "block_comment"
                continue
            if current == '"':
                chars[index] = " "
                index += 1
                state = "string"
                continue
            if current == "'":
                chars[index] = " "
                index += 1
                state = "char"
                continue
            index += 1
            continue
        if state == "line_comment":
            if current == "\n":
                state = "normal"
            else:
                chars[index] = " "
            index += 1
            continue
        if state == "block_comment":
            if current == "*" and following == "/":
                chars[index] = chars[index + 1] = " "
                index += 2
                state = "normal"
            else:
                if current != "\n":
                    chars[index] = " "
                index += 1
            continue
        if state in {"string", "char"}:
            if current == "\\":
                chars[index] = " "
                if index + 1 < len(chars) and chars[index + 1] != "\n":
                    chars[index + 1] = " "
                    index += 2
                else:
                    index += 1
                continue
            if (state == "string" and current == '"') or (state == "char" and current == "'"):
                chars[index] = " "
                index += 1
                state = "normal"
                continue
            if current != "\n":
                chars[index] = " "
            index += 1
            continue
    return "".join(chars)


def _brace_depth_before(masked: str, position: int) -> int:
    depth = 0
    for character in masked[:position]:
        if character == "{":
            depth += 1
        elif character == "}":
            depth = max(0, depth - 1)
    return depth


def source_span_metadata(source_text: str, function: str) -> dict[str, Any]:
    """Return deterministic line/offset/hash metadata for a C function body.

    The scanner ignores braces, parentheses, and function-like text in C
    comments, string literals, and character literals.  The returned span
    starts at the beginning of the line containing the function token and ends
    immediately after its balanced closing brace.  No source text is emitted.
    """

    if not isinstance(source_text, str):
        raise ReconstructionPacketError("source_text must be text", code="source_span")
    name = _text(function, "function")
    masked = _mask_c_comments_and_literals(source_text)
    match: re.Match[str] | None = None
    opening: int | None = None
    for candidate in _FUNCTION_TOKEN_RE.finditer(masked):
        if candidate.group("name") != name:
            continue
        line_start = masked.rfind("\n", 0, candidate.start()) + 1
        if masked[line_start:candidate.start()].lstrip().startswith("#"):
            # Function-like macro bodies are not C definitions even when the
            # replacement text contains balanced braces.
            continue
        # C definitions are file-scope declarations.  Requiring zero brace
        # depth prevents a call such as ``target_fn(); { ... }`` inside an
        # enclosing function from being mistaken for the definition.
        if _brace_depth_before(masked, candidate.start()) != 0:
            continue
        paren_start = masked.find("(", candidate.start(), candidate.end())
        if paren_start < 0:
            continue
        depth = 0
        close: int | None = None
        for index in range(paren_start, len(masked)):
            character = masked[index]
            if character == "(":
                depth += 1
            elif character == ")":
                depth -= 1
                if depth == 0:
                    close = index
                    break
        if close is None:
            continue
        # A definition has a body before its next declaration terminator.  A
        # call followed by an if/loop block is therefore not mistaken for the
        # definition.
        brace = masked.find("{", close + 1)
        semicolon = masked.find(";", close + 1)
        if brace < 0 or (semicolon >= 0 and semicolon < brace):
            continue
        match = candidate
        opening = brace
        break
    if match is None or opening is None:
        raise ReconstructionPacketError(
            f"function definition {name!r} was not found", code="source_span"
        )

    depth = 0
    closing: int | None = None
    for index in range(opening, len(masked)):
        character = masked[index]
        if character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                closing = index
                break
    if closing is None:
        raise ReconstructionPacketError(
            f"function definition {name!r} has unbalanced braces", code="source_span"
        )
    start = source_text.rfind("\n", 0, match.start()) + 1
    end = closing + 1
    span = source_text[start:end]
    start_line = source_text.count("\n", 0, start) + 1
    end_line = source_text.count("\n", 0, closing) + 1
    return {
        "function": name,
        "start_line": start_line,
        "end_line": end_line,
        "start_offset": start,
        "end_offset": end,
        "span_sha256": hashlib.sha256(span.encode("utf-8")).hexdigest(),
    }


def _text(value: Any, label: str, *, max_bytes: int = 1024) -> str:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise ReconstructionPacketError(f"{label} must be non-empty text")
    if len(value.encode("utf-8")) > max_bytes:
        raise ReconstructionPacketError(f"{label} exceeds {max_bytes} UTF-8 bytes")
    return value


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label, max_bytes=64)
    if _SHA256_RE.fullmatch(result) is None:
        raise ReconstructionPacketError(f"{label} must be a lowercase SHA-256")
    return result


def _commit(value: Any, label: str) -> str:
    result = _text(value, label, max_bytes=40)
    if _COMMIT_RE.fullmatch(result) is None:
        raise ReconstructionPacketError(f"{label} must be a lowercase 40-character commit")
    return result


def _mapping(value: Any, label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ReconstructionPacketError(f"{label} must be an object")
    return value


def _integer(value: Any, label: str, *, minimum: int = 0, maximum: int | None = None) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise ReconstructionPacketError(f"{label} must be an integer >= {minimum}")
    if maximum is not None and value > maximum:
        raise ReconstructionPacketError(f"{label} must be <= {maximum}")
    return value


def _number(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return int(value) if value.is_integer() else None
    if not isinstance(value, str):
        return None
    text = value.strip()
    if not text:
        return None
    try:
        sign = -1 if text.startswith("-") else 1
        unsigned = text[1:] if text[:1] in {"+", "-"} else text
        if unsigned.lower().startswith("0x"):
            return sign * int(unsigned, 16)
        return int(text, 10)
    except ValueError:
        return None


def _json_copy(value: Any, label: str, *, max_bytes: int = MAX_NESTED_VALUE_BYTES) -> Any:
    """Copy JSON values while rejecting ambiguous keys and oversized leaves."""

    if isinstance(value, Mapping):
        result: dict[str, Any] = {}
        for key, item in value.items():
            if not isinstance(key, str):
                raise ReconstructionPacketError(f"{label} has a non-text key")
            result[key] = _json_copy(item, f"{label}.{key}", max_bytes=max_bytes)
        return result
    if isinstance(value, list):
        return [_json_copy(item, f"{label}[{index}]", max_bytes=max_bytes) for index, item in enumerate(value)]
    if isinstance(value, (str, int, float, bool)) or value is None:
        try:
            encoded = _canonical(value)
        except ReconstructionPacketError:
            raise
        if len(encoded) > max_bytes:
            raise ReconstructionPacketError(f"{label} exceeds {max_bytes} canonical bytes")
        return value
    raise ReconstructionPacketError(f"{label} contains an unsupported JSON value")


def _digest_without(value: Mapping[str, Any], field: str) -> str:
    body = dict(value)
    body.pop(field, None)
    return canonical_sha256(body)


def _focus_digest(report: Mapping[str, Any]) -> str:
    """Validate a focus artifact's own digest when one is present."""

    artifact = report.get("artifact_sha256")
    if artifact is not None:
        expected = _sha256(artifact, "focus artifact_sha256")
        actual = _digest_without(report, "artifact_sha256")
        if actual != expected:
            raise ReconstructionPacketError(
                "focus artifact_sha256 does not seal the supplied report", code="focus_hash_drift"
            )
        return expected
    return canonical_sha256(report)


def _check_report_binding(report: Mapping[str, Any], values: Mapping[str, str]) -> None:
    """Compare identity fields if a producer included them in its report."""

    sources: list[Mapping[str, Any]] = [report]
    for field in ("input_binding", "binding", "source"):
        input_binding = report.get(field)
        if isinstance(input_binding, Mapping):
            sources.append(input_binding)
    for key, expected in values.items():
        if expected is None:
            continue
        for source in sources:
            if key in source and source.get(key) != expected:
                raise ReconstructionPacketError(
                    f"focus report {key} drifted from the bound identity", code="binding_drift"
                )


def _channels(report: Mapping[str, Any]) -> dict[str, Mapping[str, Any]]:
    raw = report.get("channels")
    if isinstance(raw, Mapping):
        result: dict[str, Mapping[str, Any]] = {}
        for name in ("strict", "data"):
            value = raw.get(name)
            if isinstance(value, Mapping):
                result[name] = value
        if result:
            return result

    # Small fixture/adapter form: strict and data are directly on the report.
    result = {}
    for name in ("strict", "data"):
        value = report.get(name)
        if isinstance(value, Mapping):
            result[name] = value
    if result:
        return result

    # A raw paired-symbol report can be used without first running the compact
    # focus adapter.  Its target/candidate pair is the strict channel; data is
    # intentionally the same pair because only the source instructions matter
    # for reconstruction.
    if isinstance(report.get("target"), Mapping) and isinstance(report.get("candidate"), Mapping):
        pair = {"target": report["target"], "candidate": report["candidate"]}
        return {"strict": pair}
    return {}


def _side_rows(channel: Mapping[str, Any], side: str) -> list[Mapping[str, Any]]:
    value = channel.get(side)
    if isinstance(value, Mapping):
        rows = value.get("rows")
        if isinstance(rows, list):
            return [_mapping(item, f"channel.{side}.rows[{index}]") for index, item in enumerate(rows)]
        # Some raw reports expose ``instructions`` instead of normalized rows.
        rows = value.get("instructions")
        if isinstance(rows, list):
            return [_mapping(item, f"channel.{side}.instructions[{index}]") for index, item in enumerate(rows)]
    if isinstance(value, list):
        return [_mapping(item, f"channel.{side}[{index}]") for index, item in enumerate(value)]
    return []


def _row_index(row: Mapping[str, Any], position: int) -> int:
    value = _number(row.get("index"))
    return value if value is not None and value >= 0 else position


def _instruction_mapping(row: Mapping[str, Any]) -> Mapping[str, Any] | None:
    nested = row.get("instruction")
    if isinstance(nested, Mapping):
        return nested
    if any(key in row for key in ("formatted", "address", "parts", "mnemonic")):
        return row
    return None


def _formatted(row: Mapping[str, Any]) -> str:
    instruction = _instruction_mapping(row)
    if instruction is None:
        return ""
    value = instruction.get("formatted", instruction.get("text", ""))
    return value if isinstance(value, str) else ""


def _mnemonic(row: Mapping[str, Any]) -> str:
    instruction = _instruction_mapping(row)
    if instruction is not None:
        value = instruction.get("mnemonic")
        if isinstance(value, str) and value:
            return value.lower()
        value = instruction.get("opcode")
        if isinstance(value, str) and value:
            return value.split()[0].lower()
    match = _MNEMONIC_RE.match(_formatted(row))
    return match.group(1).lower() if match else ""


def _address(row: Mapping[str, Any]) -> Any:
    instruction = _instruction_mapping(row)
    if instruction is None:
        return None
    value = instruction.get("address")
    if isinstance(value, (str, int)) and not isinstance(value, bool):
        return value
    return None


def _compact_relocation(value: Any) -> Any:
    if not isinstance(value, Mapping):
        return None
    result: dict[str, Any] = {}
    # Numeric symbol-table indexes do not identify the same object across
    # target/candidate, but keeping the textual target and relocation kind is
    # useful for reconstruction.
    for key in (
        "type",
        "type_name",
        "offset",
        "addend",
        "target_name",
        "effective_target",
        "symbol",
        "symbol_value",
        "target_symbol",
    ):
        if key in value:
            result[key] = _json_copy(value[key], f"relocation.{key}", max_bytes=1024)
    return result or None


def _compact_instruction(row: Mapping[str, Any] | None, index: int) -> dict[str, Any]:
    if row is None:
        return {"index": index, "present": False}
    instruction = _instruction_mapping(row)
    if instruction is None:
        return {
            "index": index,
            "present": False,
            "row_sha256": canonical_sha256(dict(row)),
        }
    result: dict[str, Any] = {"index": index, "present": True}
    address = instruction.get("address")
    if isinstance(address, (str, int)) and not isinstance(address, bool):
        result["address"] = address
    size = _number(instruction.get("size"))
    if size is not None:
        result["size"] = size
    text = _formatted(row)
    if text:
        encoded = text.encode("utf-8")
        if len(encoded) <= MAX_TEXT_BYTES:
            result["formatted"] = text
        else:
            result["formatted_prefix"] = encoded[:MAX_TEXT_BYTES].decode("utf-8", errors="ignore")
            result["formatted_omitted_bytes"] = len(encoded) - MAX_TEXT_BYTES
            result["formatted_sha256"] = hashlib.sha256(encoded).hexdigest()
    mnemonic = _mnemonic(row)
    if mnemonic:
        result["mnemonic"] = mnemonic
    relocation = _compact_relocation(instruction.get("relocation", row.get("relocation")))
    if relocation is not None:
        result["relocation"] = relocation
    for key in ("branch_dest", "branch_target", "target"):
        if key in instruction:
            value = instruction[key]
            if isinstance(value, (str, int)) and not isinstance(value, bool):
                result["branch_target"] = value
                break
    # A parts digest authenticates omitted operand decoration without carrying
    # the usually very large token tree into every window.
    if "parts" in instruction:
        result["parts_sha256"] = canonical_sha256(instruction["parts"])
    if isinstance(row.get("diff_kind"), str) and row.get("diff_kind"):
        result["diff_kind"] = row["diff_kind"]
    return result


def _row_digest(row: Mapping[str, Any] | None) -> str | None:
    return canonical_sha256(dict(row)) if row is not None else None


def _stable_instruction_payload(row: Mapping[str, Any] | None) -> dict[str, Any] | None:
    """Mirror the producer's target-anchored instruction identity payload."""

    if not isinstance(row, Mapping):
        return None
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        return None
    payload = {key: value for key, value in instruction.items() if key != "parts"}
    if "parts" in instruction:
        payload["parts_sha256"] = canonical_sha256(instruction["parts"])
    return payload


def _stable_instruction_row_id(
    channel: str,
    function: str,
    index: int,
    target_row: Mapping[str, Any] | None,
    candidate_row: Mapping[str, Any] | None,
    target_rows: Mapping[int, Mapping[str, Any]],
) -> str:
    """Recompute ``owner_campaign_measure._stable_row_id`` independently."""

    target_payload = _stable_instruction_payload(target_row)
    candidate_payload = _stable_instruction_payload(candidate_row)
    target_kind = target_row.get("diff_kind") if target_row else None
    candidate_kind = candidate_row.get("diff_kind") if candidate_row else None
    if target_payload is not None:
        body: dict[str, Any] = {
            "schema": "owner_campaign_target_instruction_identity/v1",
            "channel": channel,
            "function": function,
            "target_instruction": target_payload,
            "target_kind": target_kind if isinstance(target_kind, str) else None,
        }
    else:
        indexes = sorted(target_rows)
        before = max((item for item in indexes if item < index), default=None)
        after = min((item for item in indexes if item > index), default=None)
        before_payload = (
            _stable_instruction_payload(target_rows[before]) if before is not None else None
        )
        after_payload = (
            _stable_instruction_payload(target_rows[after]) if after is not None else None
        )
        body = {
            "schema": "owner_campaign_candidate_insert_identity/v1",
            "channel": channel,
            "function": function,
            "before_target_sha256": (
                canonical_sha256(before_payload) if before_payload is not None else None
            ),
            "after_target_sha256": (
                canonical_sha256(after_payload) if after_payload is not None else None
            ),
            "candidate_instruction": candidate_payload,
            "candidate_kind": candidate_kind if isinstance(candidate_kind, str) else None,
        }
    return f"{channel}:instruction:{canonical_sha256(body)}"


def _row_id(channel: str, index: int, row: Mapping[str, Any] | None, supplied: str | None) -> str:
    if isinstance(supplied, str) and supplied:
        return supplied
    kind = row.get("diff_kind") if isinstance(row, Mapping) else None
    kind_text = kind if isinstance(kind, str) and kind else "DIFF_UNKNOWN"
    address = _address(row) if row is not None else None
    address_text = str(address) if address is not None else "-"
    return f"{channel}:row:{index}:kind={kind_text}:address={address_text}"


def _validate_row_id(
    row_id: str,
    *,
    channel: str,
    function: str,
    index: int,
    target_row: Mapping[str, Any] | None,
    candidate_row: Mapping[str, Any] | None,
    target_rows: Mapping[int, Mapping[str, Any]],
) -> None:
    """Bind caller-supplied compact identities to the full focus row.

    Producers may include the function name and target/candidate addresses, so
    the complete string is not reconstructed here.  The invariant fields are
    still non-negotiable: channel, aligned row index, and diff kind must agree
    with the full report used to build the packet.
    """

    stable_id = _stable_instruction_row_id(
        channel,
        function,
        index,
        target_row,
        candidate_row,
        target_rows,
    )
    if row_id == stable_id:
        return
    row = target_row if _is_residual(target_row) else candidate_row
    if not row_id.startswith(f"{channel}:") or f":row:{index}:" not in row_id:
        raise ReconstructionPacketError(
            f"{channel} row identity does not bind row {index}", code="row_identity"
        )
    kind = row.get("diff_kind") if isinstance(row, Mapping) else None
    if isinstance(kind, str) and kind and f"kind={kind}" not in row_id:
        raise ReconstructionPacketError(
            f"{channel} row identity kind does not bind row {index}",
            code="row_identity",
        )


def _ids_for(report: Mapping[str, Any], channel: str) -> list[str]:
    raw = report.get(f"{channel}_row_ids")
    if isinstance(raw, list):
        return [item for item in raw if isinstance(item, str) and item]
    material = report.get("channels")
    if isinstance(material, Mapping) and isinstance(material.get(channel), Mapping):
        raw = material[channel].get("row_ids")
        if isinstance(raw, list):
            return [item for item in raw if isinstance(item, str) and item]
    raw = report.get(f"{channel}_rows")
    if isinstance(raw, list) and all(isinstance(item, str) for item in raw):
        return list(raw)
    return []


def _validated_id_override(value: Sequence[str], label: str) -> list[str]:
    if isinstance(value, (str, bytes)) or not isinstance(value, Sequence):
        raise ReconstructionPacketError(f"{label} must be an array", code="row_identity")
    result = [_text(item, f"{label}[{index}]", max_bytes=2048) for index, item in enumerate(value)]
    if len(set(result)) != len(result):
        raise ReconstructionPacketError(f"{label} contains duplicate identities", code="row_identity")
    return result


def _is_residual(row: Mapping[str, Any] | None) -> bool:
    return isinstance(row, Mapping) and isinstance(row.get("diff_kind"), str) and bool(row.get("diff_kind"))


def _indexed_rows(rows: Sequence[Mapping[str, Any]]) -> dict[int, Mapping[str, Any]]:
    result: dict[int, Mapping[str, Any]] = {}
    for position, row in enumerate(rows):
        index = _row_index(row, position)
        # Duplicate indexes are ambiguous for a target-first window.  Do not
        # silently choose one: a forged or malformed focus report could
        # otherwise move a residual to a different source instruction.
        if index in result:
            raise ReconstructionPacketError(
                f"instruction index {index} is duplicated",
                code="duplicate_instruction_index",
            )
        result[index] = row
    return result


def _build_residual_events(
    report: Mapping[str, Any],
    channels: Mapping[str, Mapping[str, Any]],
    *,
    function: str,
) -> tuple[list[dict[str, Any]], dict[str, list[str]], dict[int, Mapping[str, Any]], dict[int, Mapping[str, Any]]]:
    primary_strict = channels.get("strict", {})
    target_rows = _side_rows(primary_strict, "target")
    candidate_rows = _side_rows(primary_strict, "candidate")
    if not target_rows and not candidate_rows:
        for channel in ("data", "strict"):
            material = channels.get(channel, {})
            target_rows = _side_rows(material, "target")
            candidate_rows = _side_rows(material, "candidate")
            if target_rows or candidate_rows:
                break
    target_by_index = _indexed_rows(target_rows)
    candidate_by_index = _indexed_rows(candidate_rows)

    event_by_key: dict[tuple[Any, ...], dict[str, Any]] = {}
    residual_ids: dict[str, list[str]] = {"strict": [], "data": []}
    for channel_name in ("strict", "data"):
        material = channels.get(channel_name)
        if material is None:
            continue
        target = _side_rows(material, "target")
        candidate = _side_rows(material, "candidate")
        target_indexed = _indexed_rows(target)
        candidate_indexed = _indexed_rows(candidate)
        indexes = sorted(set(target_indexed) | set(candidate_indexed))
        supplied_ids = _ids_for(report, channel_name)
        residual_indexes = [
            index
            for index in indexes
            if _is_residual(target_indexed.get(index)) or _is_residual(candidate_indexed.get(index))
        ]
        if len(supplied_ids) == len(residual_indexes):
            supplied_by_index = dict(zip(residual_indexes, supplied_ids))
        elif len(supplied_ids) == len(indexes):
            supplied_by_index = dict(zip(indexes, supplied_ids))
        else:
            supplied_by_index = {}
        for position, index in enumerate(indexes):
            target_row = target_indexed.get(index)
            candidate_row = candidate_indexed.get(index)
            supplied = supplied_by_index.get(index)
            if not (_is_residual(target_row) or _is_residual(candidate_row) or supplied is not None):
                continue
            row_id = _row_id(
                channel_name,
                index,
                target_row if _is_residual(target_row) else candidate_row,
                supplied,
            )
            _validate_row_id(
                row_id,
                channel=channel_name,
                function=function,
                index=index,
                target_row=target_row,
                candidate_row=candidate_row,
                target_rows=target_indexed,
            )
            residual_ids[channel_name].append(row_id)
            # Same aligned strict/data rows represent two views of one causal
            # event.  Address and index protect insertion/deletion rows from
            # being incorrectly merged with a neighboring event.
            key = (index, _address(target_row), _address(candidate_row))
            event = event_by_key.get(key)
            if event is None:
                event = {
                    "anchor_index": index,
                    "channels": [],
                    "row_ids": {"strict": [], "data": []},
                    "target": _compact_instruction(target_row, index),
                    "candidate": _compact_instruction(candidate_row, index),
                    "target_row_sha256": _row_digest(target_row),
                    "candidate_row_sha256": _row_digest(candidate_row),
                }
                event_by_key[key] = event
            if channel_name not in event["channels"]:
                event["channels"].append(channel_name)
            event["row_ids"][channel_name].append(row_id)

    if len(event_by_key) > MAX_RESIDUAL_ROWS:
        raise ReconstructionPacketError(
            f"residual event count exceeds {MAX_RESIDUAL_ROWS}", code="residual_limit"
        )
    events = sorted(
        event_by_key.values(),
        key=lambda item: (item["anchor_index"], tuple(item["channels"]), canonical_sha256(item["row_ids"])),
    )
    for event in events:
        event["channels"] = sorted(event["channels"])
        event["row_ids"]["strict"] = sorted(set(event["row_ids"]["strict"]))
        event["row_ids"]["data"] = sorted(set(event["row_ids"]["data"]))
    residual_ids["strict"] = list(dict.fromkeys(residual_ids["strict"]))
    residual_ids["data"] = list(dict.fromkeys(residual_ids["data"]))
    return events, residual_ids, target_by_index, candidate_by_index


def _compact_entries(entries: list[dict[str, Any]], *, label: str) -> dict[str, Any]:
    if len(entries) <= MAX_SUMMARY_ENTRIES:
        return {"count": len(entries), "entries": entries}
    omitted = entries[MAX_SUMMARY_ENTRIES:]
    return {
        "count": len(entries),
        "entries": entries[:MAX_SUMMARY_ENTRIES],
        "omitted_count": len(omitted),
        "omitted_sha256": canonical_sha256(omitted),
        "omission_reason": f"{label}_entry_limit",
    }


def _instruction_entry(row: Mapping[str, Any], index: int) -> dict[str, Any]:
    result = _compact_instruction(row, index)
    result["row_sha256"] = canonical_sha256(dict(row))
    return result


def _control_flow(rows: Mapping[int, Mapping[str, Any]]) -> dict[str, Any]:
    calls: list[dict[str, Any]] = []
    branches: list[dict[str, Any]] = []
    address_to_index: dict[int, int] = {}
    for index, row in rows.items():
        address = _address(row)
        if address is not None:
            address_to_index[address] = index
    for index in sorted(rows):
        row = rows[index]
        mnemonic = _mnemonic(row)
        if mnemonic in _CALL_MNEMONICS:
            calls.append(_instruction_entry(row, index))
        if mnemonic in _BRANCH_MNEMONICS:
            entry = _instruction_entry(row, index)
            target = _number(entry.get("branch_target"))
            if target is not None and target in address_to_index:
                entry["branch_target_index"] = address_to_index[target]
            branches.append(entry)
    return {
        "instruction_count": len(rows),
        "calls": _compact_entries(calls, label="call"),
        "branches": _compact_entries(branches, label="branch"),
    }


def _stack_access(row: Mapping[str, Any], index: int) -> tuple[int, str, str] | None:
    instruction = _instruction_mapping(row)
    formatted = _formatted(row)
    match = _STACK_RE.search(formatted)
    offset: int | None = None
    if match:
        offset = _number(match.group("offset"))
    elif instruction is not None:
        offset = _number(instruction.get("stack_offset"))
    if offset is None:
        return None
    mnemonic = _mnemonic(row)
    lower = mnemonic.lower()
    if lower.startswith(_STORE_PREFIXES):
        kind = "store"
    elif lower.startswith(_LOAD_PREFIXES):
        kind = "load"
    else:
        kind = "access"
    return offset, kind, mnemonic


def _stack_summary(rows: Mapping[int, Mapping[str, Any]]) -> dict[str, Any]:
    accesses: list[dict[str, Any]] = []
    homes: dict[int, dict[str, Any]] = {}
    for index in sorted(rows):
        parsed = _stack_access(rows[index], index)
        if parsed is None:
            continue
        offset, kind, mnemonic = parsed
        item = {
            "index": index,
            "offset": offset,
            "kind": kind,
            "mnemonic": mnemonic,
            "formatted": _formatted(rows[index])[:MAX_TEXT_BYTES],
        }
        accesses.append(item)
        home = homes.setdefault(offset, {"offset": offset, "loads": 0, "stores": 0, "accesses": 0})
        home["accesses"] += 1
        if kind == "load":
            home["loads"] += 1
        elif kind == "store":
            home["stores"] += 1
    home_entries = [homes[offset] for offset in sorted(homes)]
    result: dict[str, Any] = {
        "access_count": len(accesses),
        "offsets": sorted(homes),
        "homes": home_entries,
    }
    result["accesses"] = _compact_entries(accesses, label="stack")
    return result


def _compact_physical_difference(value: Any, index: int) -> dict[str, Any]:
    """Keep changed relocation facts without copying full relocation arrays."""

    if not isinstance(value, Mapping):
        raise ReconstructionPacketError(
            f"physical difference[{index}] is not an object",
            code="physical_difference",
        )
    result: dict[str, Any] = {"index": index}
    for key in (
        "offset",
        "target_offset",
        "candidate_offset",
        "type",
        "type_name",
        "addend",
        "target_effective_target",
        "candidate_effective_target",
        "effective_target",
    ):
        if key in value:
            result[key] = _json_copy(value[key], f"physical.difference[{index}].{key}")
    for side_name in ("target", "candidate"):
        side = value.get(side_name)
        if not isinstance(side, list):
            if side is not None:
                result[side_name] = _json_copy(
                    side, f"physical.difference[{index}].{side_name}", max_bytes=1024
                )
            continue
        result[f"{side_name}_count"] = len(side)
        result[f"{side_name}_sha256"] = canonical_sha256(side)
        if len(side) <= 8:
            compact_side: list[Any] = []
            for item in side:
                if isinstance(item, Mapping):
                    compact_side.append(_compact_relocation(item) or {})
                else:
                    compact_side.append(_json_copy(item, f"physical.difference[{index}].{side_name}"))
            result[side_name] = compact_side
        else:
            sample: list[Any] = []
            for item in side[:4]:
                sample.append(
                    _compact_relocation(item)
                    if isinstance(item, Mapping)
                    else _json_copy(item, f"physical.difference[{index}].{side_name}")
                )
            result[f"{side_name}_sample"] = sample
    target_rows = value.get("target")
    candidate_rows = value.get("candidate")
    if isinstance(target_rows, list) and isinstance(candidate_rows, list):
        changed: list[dict[str, Any]] = []
        width = max(len(target_rows), len(candidate_rows))
        for pair_index in range(width):
            target_item = target_rows[pair_index] if pair_index < len(target_rows) else None
            candidate_item = (
                candidate_rows[pair_index] if pair_index < len(candidate_rows) else None
            )
            if canonical_sha256(target_item) == canonical_sha256(candidate_item):
                continue
            changed.append(
                {
                    "pair_index": pair_index,
                    "target": (
                        _compact_relocation(target_item)
                        if isinstance(target_item, Mapping)
                        else target_item
                    ),
                    "candidate": (
                        _compact_relocation(candidate_item)
                        if isinstance(candidate_item, Mapping)
                        else candidate_item
                    ),
                }
            )
        result["changed_pair_count"] = len(changed)
        result["changed_pairs_sha256"] = canonical_sha256(changed)
        if len(changed) <= MAX_RELOCATION_CONTEXT:
            result["changed_pairs"] = changed
        else:
            result["changed_pairs"] = changed[:MAX_RELOCATION_CONTEXT]
            result["changed_pairs_omitted_count"] = len(changed) - MAX_RELOCATION_CONTEXT
            result["changed_pairs_omitted_sha256"] = canonical_sha256(
                changed[MAX_RELOCATION_CONTEXT:]
            )
    return result


def _physical(
    report: Mapping[str, Any],
    injected_difference_ids: Sequence[str] | None = None,
) -> dict[str, Any]:
    material = report.get("physical_relocations")
    if not isinstance(material, Mapping):
        material = report.get("physical") if isinstance(report.get("physical"), Mapping) else {}
    differences = material.get("physical_relocation_differences")
    if not isinstance(differences, list):
        differences = material.get("differences")
    if not isinstance(differences, list):
        differences = report.get("physical_difference_rows")
    if not isinstance(differences, list):
        differences = []

    def side(name: str) -> dict[str, Any]:
        value = material.get(name)
        if not isinstance(value, Mapping):
            value = {}
        raw_rows = value.get("physical_relocations")
        rows_present = isinstance(raw_rows, list)
        if not isinstance(raw_rows, list):
            raw_rows = value.get("relocations")
            rows_present = isinstance(raw_rows, list)
        if not isinstance(raw_rows, list):
            raw_rows = []
        rows = [_json_copy(item, f"physical.{name}[{index}]") for index, item in enumerate(raw_rows)]
        declared = value.get("physical_relocation_count")
        if declared is None:
            declared = value.get("count")
        count_raw = value.get("physical_relocation_count")
        if count_raw is None:
            count_raw = value.get("count")
        count = (
            _integer(count_raw, f"physical.{name}.count")
            if count_raw is not None
            else None
        )
        if count is None:
            count = len(rows)
        payload_digest = value.get("physical_relocation_payload_sha256")
        if payload_digest is None:
            payload_digest = value.get("relocations_sha256")
        if payload_digest is None:
            payload_digest = value.get("payload_sha256")
        if payload_digest is not None:
            payload_digest = _sha256(
                payload_digest,
                f"physical.{name}.physical_relocation_payload_sha256",
            )
        declared_count = (
            _integer(declared, f"physical.{name}.declared_count")
            if declared is not None
            else None
        )
        rows_match = rows_present and declared_count == len(rows)
        sealed_omission = (
            not rows_present
            and declared_count is not None
            and payload_digest is not None
        )
        result: dict[str, Any] = {
            "count": count,
            # Current compact focus artifacts intentionally omit hundreds of
            # relocation rows while retaining the independently sealed count
            # and payload digest.  That is complete count evidence; treating
            # the omission as invalid would incorrectly turn bounded code
            # residuals into UNKNOWN/PIVOT.
            "count_valid": rows_match or sealed_omission,
            "relocations_sha256": payload_digest or canonical_sha256(rows),
        }
        if len(rows) <= MAX_RELOCATION_CONTEXT:
            result["relocations"] = rows
        else:
            result["relocations_omitted_count"] = len(rows)
            result["relocations_omission_reason"] = "full_relocation_list_not_needed_for_difference_packet"
        return result

    compact_differences = [
        _compact_physical_difference(item, index)
        for index, item in enumerate(differences)
    ]
    raw_difference_ids: Any = injected_difference_ids
    if raw_difference_ids is None:
        raw_difference_ids = report.get("physical_difference_ids")
    if raw_difference_ids is None:
        raw_difference_ids = material.get("physical_difference_ids")
    if raw_difference_ids is None:
        difference_ids = [
            f"physical:row:{index}:sha256={canonical_sha256(item)}"
            for index, item in enumerate(compact_differences)
        ]
    else:
        if not isinstance(raw_difference_ids, Sequence) or isinstance(raw_difference_ids, (str, bytes)):
            raise ReconstructionPacketError("physical difference IDs must be an array")
        difference_ids = []
        for index, item in enumerate(raw_difference_ids):
            difference_ids.append(_text(item, f"physical_difference_ids[{index}]", max_bytes=2048))
        if len(set(difference_ids)) != len(difference_ids):
            raise ReconstructionPacketError("physical difference IDs contain duplicates")
        if len(difference_ids) != len(compact_differences):
            raise ReconstructionPacketError(
                "physical difference ID count does not match physical differences"
            )
    raw_difference_count = material.get("difference_count")
    declared_difference_count = (
        _integer(raw_difference_count, "physical.difference_count")
        if raw_difference_count is not None
        else None
    )
    status = material.get("status")
    if not isinstance(status, str):
        exact = material.get("physical_relocations_exact")
        status = "exact" if exact is True else "mismatch" if compact_differences else "UNKNOWN"
    return {
        "status": status,
        "status_known": status in {"exact", "mismatch"},
        "target": side("target"),
        "candidate": side("candidate"),
        "difference_count": len(compact_differences),
        "difference_count_valid": (
            declared_difference_count is None
            or declared_difference_count == len(compact_differences)
        ),
        "differences": compact_differences,
        "differences_sha256": canonical_sha256(compact_differences),
        "difference_ids": difference_ids,
        "difference_ids_sha256": canonical_sha256(difference_ids),
    }


def _cluster_events(events: Sequence[Mapping[str, Any]], gap: int) -> list[dict[str, Any]]:
    if not events:
        return []
    clusters: list[list[Mapping[str, Any]]] = []
    current: list[Mapping[str, Any]] = [events[0]]
    for event in events[1:]:
        if event["anchor_index"] - current[-1]["anchor_index"] <= gap:
            current.append(event)
        else:
            clusters.append(current)
            current = [event]
    clusters.append(current)
    result: list[dict[str, Any]] = []
    for number, group in enumerate(clusters):
        row_indices = sorted({int(item["anchor_index"]) for item in group})
        strict_ids = sorted({
            row_id
            for item in group
            for row_id in item["row_ids"]["strict"]
        })
        data_ids = sorted({
            row_id
            for item in group
            for row_id in item["row_ids"]["data"]
        })
        result.append(
            {
                "cluster_id": f"cluster-{number:03d}",
                "first_index": row_indices[0],
                "last_index": row_indices[-1],
                "row_indices": row_indices,
                "residual_event_count": len(group),
                "channels": sorted({channel for item in group for channel in item["channels"]}),
                "strict_row_ids": strict_ids,
                "data_row_ids": data_ids,
                "window_ids": [],
            }
        )
    return result


def _instruction_windows(
    clusters: list[dict[str, Any]],
    target_rows: Mapping[int, Mapping[str, Any]],
    candidate_rows: Mapping[int, Mapping[str, Any]],
    *,
    radius: int,
) -> list[dict[str, Any]]:
    intervals = [
        (max(0, cluster["first_index"] - radius), cluster["last_index"] + radius, cluster["cluster_id"])
        for cluster in clusters
    ]
    if not intervals:
        return []
    intervals.sort(key=lambda item: (item[0], item[1], item[2]))
    merged: list[list[Any]] = []
    for start, end, cluster_id in intervals:
        if merged and start <= merged[-1][1] + 1:
            merged[-1][1] = max(merged[-1][1], end)
            merged[-1][2].append(cluster_id)
        else:
            merged.append([start, end, [cluster_id]])

    windows: list[dict[str, Any]] = []
    for number, (start, end, cluster_ids) in enumerate(merged):
        max_index = max(set(target_rows) | set(candidate_rows), default=-1)
        end = min(end, max_index) if max_index >= 0 else end
        # Normal objdiff rows are contiguous, so placeholders make the window
        # easy to inspect.  A malformed/synthetic report can contain a very
        # large sparse index; never allocate millions of placeholders for it.
        span = end - start + 1
        if span <= 4096:
            indexes = range(start, end + 1)
        else:
            indexes = sorted(
                index for index in set(target_rows) | set(candidate_rows)
                if start <= index <= end
            )
        target = [_compact_instruction(target_rows.get(index), index) for index in indexes]
        if span <= 4096:
            indexes = range(start, end + 1)
        else:
            indexes = sorted(
                index for index in set(target_rows) | set(candidate_rows)
                if start <= index <= end
            )
        candidate = [_compact_instruction(candidate_rows.get(index), index) for index in indexes]
        windows.append(
            {
                "window_id": f"window-{number:03d}",
                "start_index": start,
                "end_index": end,
                "radius": radius,
                "clusters": sorted(cluster_ids),
                "target": target,
                "candidate": candidate,
            }
        )
    if len(windows) > MAX_WINDOWS:
        omitted = windows[MAX_WINDOWS:]
        windows = windows[:MAX_WINDOWS]
        windows.append(
            {
                "window_id": "window-omitted",
                "omitted_count": len(omitted),
                "omitted_sha256": canonical_sha256(omitted),
            }
        )
    for window in windows:
        if "clusters" not in window:
            continue
        for cluster_id in window["clusters"]:
            for cluster in clusters:
                if cluster["cluster_id"] == cluster_id:
                    cluster["window_ids"].append(window["window_id"])
                    break
    return windows


def _broad_reason(
    events: Sequence[Mapping[str, Any]],
    clusters: Sequence[Mapping[str, Any]],
) -> str | None:
    """Return a stable reason when a packet must be a bounded pivot.

    The limits are deliberately the same limits used by ``_status``.  Keeping
    the predicate separate lets :func:`build_packet` decide *before* producing
    the potentially large instruction-window payload.  A READY packet is
    therefore never made smaller merely to satisfy the byte budget.
    """

    if len(events) > MAX_READY_EVENTS:
        return f"broad_residual_too_many_events:{len(events)}>{MAX_READY_EVENTS}"
    if len(clusters) > MAX_READY_CLUSTERS:
        return f"broad_residual_too_many_clusters:{len(clusters)}>{MAX_READY_CLUSTERS}"
    return None


def _bounded_sequence(
    values: Sequence[Any],
    limit: int,
    *,
    label: str,
) -> tuple[list[Any], dict[str, Any] | None]:
    """Keep a deterministic prefix and seal omitted values by digest."""

    material = list(values)
    if len(material) <= limit:
        return material, None
    return material[:limit], {
        "count": len(material),
        "omitted_count": len(material) - limit,
        "omitted_sha256": canonical_sha256(material[limit:]),
        "omission_reason": label,
    }


def _bounded_id_payload(values: Sequence[str], *, label: str) -> tuple[list[str], dict[str, Any] | None]:
    """Bound broad residual IDs without losing the complete census count."""

    bounded, omitted = _bounded_sequence(values, MAX_BROAD_IDS, label=label)
    return [str(value) for value in bounded], omitted


def _bounded_cluster(cluster: Mapping[str, Any]) -> dict[str, Any]:
    """Make one cluster safe to retain in a broad decomposition packet."""

    result = dict(cluster)
    for field in ("row_indices", "strict_row_ids", "data_row_ids"):
        values = result.get(field)
        if not isinstance(values, list):
            continue
        limit = (
            MAX_BROAD_CLUSTER_IDS
            if field in {"strict_row_ids", "data_row_ids"}
            else MAX_BROAD_IDS
        )
        bounded, omitted = _bounded_sequence(values, limit, label=f"broad_{field}")
        result[field] = bounded
        if omitted is not None:
            # The full-cluster digest is over the exact omitted suffix, while
            # first_index/last_index/residual_event_count remain full counts.
            result[f"{field}_complete"] = False
            result[f"{field}_omitted_count"] = omitted["omitted_count"]
            result[f"{field}_omitted_sha256"] = omitted["omitted_sha256"]
        else:
            result[f"{field}_complete"] = True
    return result


def _broad_id_selection(
    full_ids: Sequence[str],
    required_ids: Sequence[str],
) -> tuple[list[str], list[str]]:
    """Select bounded IDs while prioritizing retained-content references.

    Broad packets cannot carry a complete row census, but every identifier
    referenced by a retained event must remain in the top-level census.  The
    event references are passed before cluster references, so a pathological
    cluster cannot evict the actual representative events from the bound.
    """

    full = list(dict.fromkeys(str(item) for item in full_ids))
    full_set = set(full)
    if len(full) <= MAX_BROAD_IDS:
        # A complete census must retain producer order so its full digest can
        # be recomputed by the verifier.  Required-content prioritization is
        # only needed once omission is unavoidable.
        return full, []
    required = list(dict.fromkeys(str(item) for item in required_ids if str(item) in full_set))
    retained = required[:MAX_BROAD_IDS]
    if len(required) > MAX_BROAD_IDS:
        # The caller will mark cluster references beyond this bound as an
        # incomplete census.  Never invent an identity outside the producer's
        # full channel list.
        omitted = [item for item in full if item not in set(retained)]
        return retained, omitted
    retained_set = set(retained)
    for item in full:
        if item in retained_set:
            continue
        if len(retained) >= MAX_BROAD_IDS:
            break
        retained.append(item)
        retained_set.add(item)
    omitted = [item for item in full if item not in retained_set]
    return retained, omitted


def _broad_channel_census(
    full_ids: Sequence[str],
    retained_ids: Sequence[str],
    *,
    channel: str,
) -> dict[str, Any]:
    """Seal full and omitted channel-ID census facts for a broad packet."""

    full = list(dict.fromkeys(str(item) for item in full_ids))
    retained = list(dict.fromkeys(str(item) for item in retained_ids))
    full_set = set(full)
    if any(item not in full_set for item in retained):
        raise ReconstructionPacketError(
            f"broad {channel} retained ID is outside full census", code="row_identity"
        )
    omitted = [item for item in full if item not in set(retained)]
    prefix = f"{channel}_residuals"
    return {
        f"{prefix}_complete": not omitted,
        f"{prefix}_total_count": len(full),
        f"{prefix}_full_sha256": canonical_sha256(full),
        f"{prefix}_omitted_count": len(omitted),
        f"{prefix}_omitted_sha256": canonical_sha256(omitted),
    }


def _reconcile_broad_cross_links(
    full_ids: Mapping[str, Sequence[str]],
    events: Sequence[Mapping[str, Any]],
    clusters: Sequence[Mapping[str, Any]],
    full_clusters: Sequence[Mapping[str, Any]],
) -> tuple[dict[str, list[str]], list[dict[str, Any]], dict[str, Any]]:
    """Keep representative event/cluster references inside the ID census.

    Cluster lists may contain thousands of row IDs.  The bounded packet keeps
    only a small, deterministic subset, but it explicitly records the IDs
    removed from each retained cluster so a decomposition consumer never
    mistakes a partial cluster for a complete one.
    """

    full_by_cluster = {
        item.get("cluster_id"): item
        for item in full_clusters
        if isinstance(item, Mapping) and isinstance(item.get("cluster_id"), str)
    }
    event_refs: dict[str, list[str]] = {"strict": [], "data": []}
    for event in events:
        row_ids = event.get("row_ids") if isinstance(event, Mapping) else None
        if not isinstance(row_ids, Mapping):
            continue
        for channel in event_refs:
            values = row_ids.get(channel)
            if isinstance(values, list):
                event_refs[channel].extend(str(item) for item in values)

    cluster_refs: dict[str, list[str]] = {"strict": [], "data": []}
    for cluster in clusters:
        for channel, field in (("strict", "strict_row_ids"), ("data", "data_row_ids")):
            values = cluster.get(field)
            if isinstance(values, list):
                cluster_refs[channel].extend(str(item) for item in values)

    retained: dict[str, list[str]] = {}
    metadata: dict[str, Any] = {}
    for channel in ("strict", "data"):
        selected, _omitted = _broad_id_selection(
            full_ids.get(channel, []), event_refs[channel] + cluster_refs[channel]
        )
        retained[channel] = selected
        metadata.update(
            _broad_channel_census(full_ids.get(channel, []), selected, channel=channel)
        )

    # Rebuild every retained cluster's channel references from the full source
    # cluster, preserving event references first and then the cluster prefix.
    event_ref_sets = {channel: set(values) for channel, values in event_refs.items()}
    reconciled: list[dict[str, Any]] = []
    for cluster in clusters:
        item = dict(cluster)
        source = full_by_cluster.get(item.get("cluster_id"), cluster)
        for channel, field in (("strict", "strict_row_ids"), ("data", "data_row_ids")):
            source_values = source.get(field)
            if not isinstance(source_values, list):
                source_values = []
            source_values = list(dict.fromkeys(str(value) for value in source_values))
            allowed = set(retained[channel])
            required = [value for value in source_values if value in event_ref_sets[channel]]
            prefix = [
                value
                for value in source_values
                if value in allowed and value not in set(required)
            ]
            kept = list(dict.fromkeys((required + prefix)[:MAX_BROAD_CLUSTER_IDS]))
            omitted = [value for value in source_values if value not in set(kept)]
            item[field] = kept
            item[f"{field}_complete"] = not omitted
            item[f"{field}_omitted_count"] = len(omitted)
            item[f"{field}_omitted_sha256"] = canonical_sha256(omitted)
        reconciled.append(item)
    return retained, reconciled, metadata


def _representative_broad_content(
    events: Sequence[Mapping[str, Any]],
    clusters: Sequence[Mapping[str, Any]],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], dict[str, Any]]:
    """Select first/representative causal regions for an UNKNOWN packet.

    At least the first event of each retained cluster is kept.  The final
    cluster is included when there are more clusters than the prefix budget so
    a broad function does not look as though its tail was never inspected.
    All full-list counts/digests are returned separately and therefore remain
    useful to a later decomposition pass without serializing every window.
    """

    if not clusters:
        selected_cluster_indexes: list[int] = []
    elif len(clusters) <= MAX_BROAD_CLUSTERS:
        selected_cluster_indexes = list(range(len(clusters)))
    else:
        selected_cluster_indexes = list(range(MAX_BROAD_CLUSTERS - 1)) + [len(clusters) - 1]

    selected_clusters = [_bounded_cluster(clusters[index]) for index in selected_cluster_indexes]
    omitted_cluster_material = [
        cluster
        for index, cluster in enumerate(clusters)
        if index not in set(selected_cluster_indexes)
    ]
    selected_ids = {
        cluster.get("cluster_id")
        for cluster in selected_clusters
        if isinstance(cluster.get("cluster_id"), str)
    }

    # Cluster row ranges are authoritative for selection.  Use the source
    # event order as the tie-breaker, and include both ends of a region where
    # the eight-event budget permits it.
    selected_events: list[Mapping[str, Any]] = []
    for cluster in clusters:
        cluster_id = cluster.get("cluster_id")
        if cluster_id not in selected_ids:
            continue
        first = cluster.get("first_index")
        last = cluster.get("last_index")
        region = [
            event
            for event in events
            if isinstance(first, int)
            and isinstance(last, int)
            and first <= event.get("anchor_index", -1) <= last
        ]
        if region:
            selected_events.append(region[0])
            if len(region) > 1:
                selected_events.append(region[-1])

    # Preserve the deterministic event order and then cap it.  If many
    # selected clusters compete for the cap, first events win; this is still
    # representative because every selected cluster has a bounded descriptor.
    unique_events: list[dict[str, Any]] = []
    seen_event_keys: set[str] = set()
    for event in sorted(selected_events, key=lambda item: (item.get("anchor_index", -1), canonical_sha256(item))):
        key = canonical_sha256(event)
        if key in seen_event_keys:
            continue
        seen_event_keys.add(key)
        unique_events.append(dict(event))
    representative_events, _selected_event_overflow = _bounded_sequence(
        unique_events,
        MAX_BROAD_EVENTS,
        label="broad_residual_events",
    )
    representative_clusters, _selected_cluster_overflow = _bounded_sequence(
        selected_clusters,
        MAX_BROAD_CLUSTERS,
        label="broad_causal_clusters",
    )
    # ``unique_events`` is only the candidate representative set.  The omitted
    # census must be calculated against the complete event list, not against
    # that intermediate set (and never against the full list itself).  This is
    # important when the selected clusters are first/tail samples of a broad
    # function: their retained events are not necessarily a simple prefix.
    retained_event_digests = {canonical_sha256(item) for item in representative_events}
    omitted_events = [
        event
        for event in events
        if canonical_sha256(event) not in retained_event_digests
    ]
    metadata: dict[str, Any] = {
        "residual_rows_complete": not omitted_events and len(representative_events) == len(events),
        "residual_rows_total_count": len(events),
        "residual_rows_full_sha256": canonical_sha256(list(events)),
        "causal_clusters_complete": not omitted_cluster_material and len(selected_clusters) == len(clusters),
        "causal_clusters_total_count": len(clusters),
        "causal_clusters_full_sha256": canonical_sha256(list(clusters)),
        "selected_cluster_count": len(representative_clusters),
    }
    if omitted_events:
        metadata["residual_rows_omitted_count"] = len(omitted_events)
        metadata["residual_rows_omitted_sha256"] = canonical_sha256(omitted_events)
    if omitted_cluster_material:
        metadata["causal_clusters_omitted_count"] = len(omitted_cluster_material)
        metadata["causal_clusters_omitted_sha256"] = canonical_sha256(omitted_cluster_material)
    return representative_events, representative_clusters, metadata


def _bounded_summary_entries(
    material: Mapping[str, Any],
    *,
    limit: int,
    label: str,
) -> dict[str, Any]:
    """Bound call/branch/access entries while preserving their total count."""

    result = dict(material)
    entries = result.get("entries")
    if not isinstance(entries, list) or len(entries) <= limit:
        return result
    result["entries"] = entries[:limit]
    result["entries_complete"] = False
    result["omitted_count"] = len(entries) - limit
    result["omitted_sha256"] = canonical_sha256(entries[limit:])
    result["omission_reason"] = label
    return result


def _bounded_control_flow(flow: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(flow)
    for kind in ("calls", "branches"):
        material = result.get(kind)
        if isinstance(material, Mapping):
            result[kind] = _bounded_summary_entries(
                material,
                limit=MAX_BROAD_SUMMARY_ENTRIES,
                label=f"broad_{kind}_entries",
            )
    return result


def _bounded_stack_summary(summary: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(summary)
    offsets = result.get("offsets")
    if isinstance(offsets, list) and len(offsets) > MAX_BROAD_STACK_HOMES:
        result["offsets"] = offsets[:MAX_BROAD_STACK_HOMES]
        result["offsets_complete"] = False
        result["offsets_omitted_count"] = len(offsets) - MAX_BROAD_STACK_HOMES
        result["offsets_omitted_sha256"] = canonical_sha256(offsets[MAX_BROAD_STACK_HOMES:])
    homes = result.get("homes")
    if isinstance(homes, list) and len(homes) > MAX_BROAD_STACK_HOMES:
        result["homes"] = homes[:MAX_BROAD_STACK_HOMES]
        result["homes_complete"] = False
        result["homes_omitted_count"] = len(homes) - MAX_BROAD_STACK_HOMES
        result["homes_omitted_sha256"] = canonical_sha256(homes[MAX_BROAD_STACK_HOMES:])
    accesses = result.get("accesses")
    if isinstance(accesses, Mapping):
        result["accesses"] = _bounded_summary_entries(
            accesses,
            limit=MAX_BROAD_SUMMARY_ENTRIES,
            label="broad_stack_access_entries",
        )
    return result


def _bounded_windows(windows: Sequence[Mapping[str, Any]]) -> list[dict[str, Any]]:
    """Bound broad windows, retaining the edges of an oversized region."""

    result: list[dict[str, Any]] = []
    selected, _ = _bounded_sequence(windows, MAX_BROAD_WINDOWS, label="broad_instruction_windows")
    for window in selected:
        item = dict(window)
        for side in ("target", "candidate"):
            rows = item.get(side)
            if not isinstance(rows, list) or len(rows) <= MAX_BROAD_IDS:
                continue
            edge = MAX_BROAD_IDS // 2
            item[side] = rows[:edge] + rows[-edge:]
            item[f"{side}_complete"] = False
            item[f"{side}_omitted_count"] = len(rows) - (edge * 2)
            item[f"{side}_omitted_sha256"] = canonical_sha256(rows[edge:-edge])
        result.append(item)
    return result


def _bounded_physical(physical: Mapping[str, Any]) -> dict[str, Any]:
    """Bound a broad relocation-difference list without losing its census."""

    result = dict(physical)
    differences = result.get("differences")
    difference_ids = result.get("difference_ids")
    if not isinstance(differences, list) or len(differences) <= MAX_BROAD_PHYSICAL_DIFFERENCES:
        return result
    retained = differences[:MAX_BROAD_PHYSICAL_DIFFERENCES]
    if not isinstance(difference_ids, list):
        difference_ids = []
    retained_ids = difference_ids[: len(retained)]
    omitted = differences[len(retained) :]
    omitted_ids = difference_ids[len(retained_ids) :]
    result.update(
        {
            "difference_count": len(retained),
            "difference_count_valid": True,
            "differences": retained,
            "difference_ids": retained_ids,
            "differences_complete": False,
            "differences_total_count": len(differences),
            "differences_full_sha256": canonical_sha256(differences),
            "differences_omitted_count": len(omitted),
            "differences_omitted_sha256": canonical_sha256(omitted),
            "difference_ids_complete": False,
            "difference_ids_total_count": len(difference_ids),
            "difference_ids_full_sha256": canonical_sha256(difference_ids),
            "difference_ids_omitted_count": len(omitted_ids),
            "difference_ids_omitted_sha256": canonical_sha256(omitted_ids),
            "differences_omission_reason": "broad_physical_difference_limit",
        }
    )
    result["differences_sha256"] = canonical_sha256(retained)
    result["difference_ids_sha256"] = canonical_sha256(retained_ids)
    return result


def _mirror_signature(events: Sequence[Mapping[str, Any]]) -> str:
    """Normalize one residual region without addresses or row identities."""

    normalized: list[dict[str, Any]] = []
    for event in events:
        normalized.append(
            {
                "channels": sorted(event.get("channels", [])),
                "target": {
                    "present": event.get("target", {}).get("present"),
                    "mnemonic": event.get("target", {}).get("mnemonic"),
                    "diff_kind": event.get("target", {}).get("diff_kind"),
                },
                "candidate": {
                    "present": event.get("candidate", {}).get("present"),
                    "mnemonic": event.get("candidate", {}).get("mnemonic"),
                    "diff_kind": event.get("candidate", {}).get("diff_kind"),
                },
            }
        )
    return canonical_sha256(normalized)


def _assign_mirror_groups(
    clusters: Sequence[dict[str, Any]],
    events: Sequence[Mapping[str, Any]],
) -> None:
    """Annotate repeated residual regions with a stable mirror group."""

    signatures: dict[str, int] = {}
    occurrences: dict[str, int] = {}
    for cluster in clusters:
        first = cluster.get("first_index")
        last = cluster.get("last_index")
        members = [
            event
            for event in events
            if isinstance(first, int)
            and isinstance(last, int)
            and first <= event.get("anchor_index", -1) <= last
        ]
        signature = _mirror_signature(members)
        if signature not in signatures:
            signatures[signature] = len(signatures)
        occurrence = occurrences.get(signature, 0)
        occurrences[signature] = occurrence + 1
        cluster["mirror_group"] = f"mirror-{signatures[signature]:03d}"
        cluster["mirror_occurrence"] = occurrence
        cluster["mirror_group_size"] = 0
    for cluster in clusters:
        group = cluster.get("mirror_group")
        if not isinstance(group, str):
            continue
        cluster["mirror_group_size"] = sum(
            1 for peer in clusters if peer.get("mirror_group") == group
        )


def _machine_summary(
    rows: Mapping[int, Mapping[str, Any]],
    stack: Mapping[str, Any],
) -> dict[str, Any]:
    """Summarize full-function register/stack facts for decomposition."""

    registers: set[str] = set()
    saved_registers: set[str] = set()
    for row in rows.values():
        formatted = _formatted(row)
        registers.update(item.lower() for item in _REGISTER_RE.findall(formatted))
        mnemonic = _mnemonic(row)
        if mnemonic.startswith(_STORE_PREFIXES) or mnemonic.startswith(_LOAD_PREFIXES):
            saved_registers.update(
                item.lower()
                for item in _REGISTER_RE.findall(formatted)
                if item.lower().startswith(("r1", "r2", "r3"))
            )
    return {
        "instruction_count": len(rows),
        "registers": sorted(registers),
        "saved_registers": sorted(saved_registers),
        "stack_access_count": stack.get("access_count", 0),
        "stack_offsets": list(stack.get("offsets", [])),
    }


def _flow_signature(flow: Mapping[str, Any]) -> tuple[tuple[str, Any], ...]:
    """Return only topology-bearing control-flow facts for status gating."""

    values: list[tuple[str, Any]] = []
    for kind in ("calls", "branches"):
        material = flow.get(kind)
        if not isinstance(material, Mapping):
            continue
        entries = material.get("entries")
        if not isinstance(entries, list):
            continue
        for entry in entries:
            if not isinstance(entry, Mapping):
                continue
            values.append(
                (
                    kind,
                    (
                        entry.get("index"),
                        entry.get("mnemonic"),
                        entry.get("branch_target_index") if kind == "branches" else None,
                    ),
                )
            )
    return tuple(values)


def _function_sizes(report: Mapping[str, Any]) -> tuple[int | None, int | None]:
    channels = _channels(report)
    for name in ("strict", "data"):
        material = channels.get(name)
        if not isinstance(material, Mapping):
            continue
        metric = material.get("metric")
        if not isinstance(metric, Mapping):
            continue
        target = _number(metric.get("target_size"))
        candidate = _number(metric.get("candidate_size"))
        if target is not None or candidate is not None:
            return target, candidate
    return None, None


def _size_validation(report: Mapping[str, Any]) -> tuple[int | None, int | None, str | None]:
    """Validate target/candidate sizes and require strict/data agreement."""

    channels = _channels(report)
    if set(channels) != {"strict", "data"}:
        return None, None, "missing_strict_or_data_size_channel"
    values: dict[str, tuple[int, int]] = {}
    for name in ("strict", "data"):
        metric = channels[name].get("metric")
        if not isinstance(metric, Mapping):
            return None, None, f"missing_{name}_size_metric"
        target = _number(metric.get("target_size"))
        candidate = _number(metric.get("candidate_size"))
        if target is None or candidate is None or target < 0 or candidate < 0:
            return None, None, f"invalid_{name}_size_metric"
        values[name] = (target, candidate)
    strict_target, strict_candidate = values["strict"]
    data_target, data_candidate = values["data"]
    if (strict_target, strict_candidate) != (data_target, data_candidate):
        return None, None, "cross_channel_size_drift"
    return strict_target, strict_candidate, None


def _status(
    report: Mapping[str, Any],
    events: Sequence[Mapping[str, Any]],
    clusters: Sequence[Mapping[str, Any]],
    physical: Mapping[str, Any],
    control_target: Mapping[str, Any],
    control_candidate: Mapping[str, Any],
) -> tuple[str, str]:
    target_size, candidate_size, size_reason = _size_validation(report)
    if size_reason is not None:
        return "UNKNOWN", size_reason
    if target_size is not None and candidate_size is not None and target_size != candidate_size:
        return "UNKNOWN", "function_size_or_alignment_drift"
    if control_target.get("instruction_count") != control_candidate.get("instruction_count"):
        return "UNKNOWN", "instruction_count_drift"
    if _flow_signature(control_target) != _flow_signature(control_candidate):
        return "UNKNOWN", "control_flow_topology_drift"
    if not physical.get("status_known"):
        return "UNKNOWN", "physical_relocation_status_unknown"
    if not physical.get("difference_count_valid"):
        return "UNKNOWN", "physical_relocation_count_invalid"
    for side in ("target", "candidate"):
        material = physical.get(side)
        if not isinstance(material, Mapping) or not material.get("count_valid"):
            return "UNKNOWN", f"physical_{side}_count_invalid"
    physical_count = physical.get("difference_count")
    if not events:
        if physical_count:
            return "UNKNOWN", "physical_only_residual_has_no_source_anchor"
        return "READY", "no_strict_or_data_residuals"
    if len(events) > MAX_READY_EVENTS:
        return "UNKNOWN", "residual_event_limit_exceeded"
    if len(clusters) > MAX_READY_CLUSTERS:
        return "UNKNOWN", "causal_cluster_limit_exceeded"
    return "READY", "bounded_target_first_residuals"


def _exact_terminal_constraint(
    report: Mapping[str, Any],
    physical: Mapping[str, Any],
    control_target: Mapping[str, Any],
    control_candidate: Mapping[str, Any],
) -> tuple[bool, str]:
    """State whether an exact terminal is still physically possible.

    A packet may be READY for reconstruction while exactness is impossible;
    for example, a code-only mirrored operand cluster with one inherited
    relocation difference is still useful evidence but must not be scheduled as
    an exact-terminal source cell.
    """

    if not physical.get("status_known"):
        return False, "physical_relocation_status_unknown"
    if not physical.get("difference_count_valid"):
        return False, "physical_relocation_count_invalid"
    if any(
        not isinstance(physical.get(side), Mapping)
        or not physical[side].get("count_valid")
        for side in ("target", "candidate")
    ):
        return False, "physical_relocation_count_invalid"
    if physical.get("status") != "exact":
        return False, "physical_relocation_status_not_exact"
    if physical.get("difference_count"):
        return False, "physical_relocation_difference"
    target_size, candidate_size, size_reason = _size_validation(report)
    if size_reason is not None:
        return False, size_reason
    if target_size is not None and candidate_size is not None and target_size != candidate_size:
        return False, "function_size_or_alignment_drift"
    if control_target.get("instruction_count") != control_candidate.get("instruction_count"):
        return False, "instruction_count_drift"
    if _flow_signature(control_target) != _flow_signature(control_candidate):
        return False, "control_flow_topology_drift"
    return True, "size_control_flow_and_physical_invariants_closed"


def _safe_relative_path(value: Any, label: str) -> str:
    """Accept only repository-relative source paths.

    Packets can be loaded after their producer worktree is gone.  Absolute,
    parent-relative, temporary, or disposable paths are therefore not a
    durable source binding and are rejected before they enter the digest.
    """

    result = _text(value, label, max_bytes=4096)
    normalized = result.replace("\\", "/")
    if (
        normalized.startswith("/")
        or re.match(r"^[A-Za-z]:", normalized)
        or ":" in normalized
    ):
        raise ReconstructionPacketError(f"{label} must be repository-relative", code="path_binding")
    parts = normalized.split("/")
    if any(not part or part in {".", ".."} for part in parts):
        raise ReconstructionPacketError(f"{label} contains an indirect path", code="path_binding")
    forbidden = {".git", "tmp", "temp", "disposable", "worktree"}
    if any(part.lower() in forbidden for part in parts):
        raise ReconstructionPacketError(f"{label} points into disposable state", code="path_binding")
    return result


def _reject_embedded_paths(value: Any, label: str) -> None:
    """Reject path-bearing span metadata that escapes the source binding."""

    if isinstance(value, Mapping):
        for key, item in value.items():
            child = f"{label}.{key}"
            if isinstance(key, str) and "path" in key.lower() and isinstance(item, str):
                _safe_relative_path(item, child)
            else:
                _reject_embedded_paths(item, child)
    elif isinstance(value, list):
        for index, item in enumerate(value):
            _reject_embedded_paths(item, f"{label}[{index}]")


def _binding_values(
    binding: Mapping[str, Any] | None,
    explicit: Mapping[str, Any],
    source_span: Mapping[str, Any] | None,
) -> tuple[dict[str, Any], dict[str, Any]]:
    values: dict[str, Any] = {}
    if binding is not None:
        values.update(dict(binding))
    values.update({key: value for key, value in explicit.items() if value is not None})
    aliases = {
        "target_sha256": "target_object_sha256",
        "toolchain_key_sha256": "toolchain_sha256",
        # ``frontier_sha256`` was used by the first draft of this packet.  It
        # is accepted as a parent-frontier alias so snapshots can be created
        # before a new frontier record exists.
        "frontier": "parent_frontier_sha256",
        "frontier_sha256": "parent_frontier_sha256",
    }
    for old, new in aliases.items():
        if new not in values and old in values:
            values[new] = values[old]
    required_text = ("owner", "unit", "function", "source_path")
    for key in required_text:
        values[key] = _text(values.get(key), key)
    values["source_path"] = _safe_relative_path(values["source_path"], "source_path")
    values["source_sha256"] = _sha256(values.get("source_sha256"), "source_sha256")
    values["base_commit"] = _commit(values.get("base_commit"), "base_commit")
    values["target_object_sha256"] = _sha256(
        values.get("target_object_sha256"), "target_object_sha256"
    )
    values["candidate_object_sha256"] = _sha256(
        values.get("candidate_object_sha256"), "candidate_object_sha256"
    )
    values["toolchain_sha256"] = _sha256(values.get("toolchain_sha256"), "toolchain_sha256")
    frontier_source = values.get("frontier_source_sha256", values["source_sha256"])
    values["frontier_source_sha256"] = _sha256(
        frontier_source, "frontier_source_sha256"
    )
    if values["frontier_source_sha256"] != values["source_sha256"]:
        raise ReconstructionPacketError(
            "frontier_source_sha256 must equal source_sha256", code="binding_drift"
        )
    parent_frontier = values.get("parent_frontier_sha256")
    values["parent_frontier_sha256"] = (
        _sha256(parent_frontier, "parent_frontier_sha256")
        if parent_frontier is not None
        else None
    )
    if source_span is None:
        source_span = values.get("source_span")
    span = _mapping(source_span, "source_span")
    span_copy = _json_copy(span, "source_span", max_bytes=MAX_NESTED_VALUE_BYTES)
    if not isinstance(span_copy, Mapping):
        raise ReconstructionPacketError("source_span must be an object")
    _reject_embedded_paths(span_copy, "source_span")
    return (
        {
            key: str(values[key])
            for key in required_text
            + (
                "source_sha256",
                "base_commit",
                "target_object_sha256",
                "candidate_object_sha256",
                "toolchain_sha256",
                "frontier_source_sha256",
            )
        }
        | {"parent_frontier_sha256": values["parent_frontier_sha256"]},
        {"source_span": dict(span_copy)},
    )


def build_packet(
    focus_report: Mapping[str, Any],
    binding: Mapping[str, Any] | None = None,
    source_span: Mapping[str, Any] | None = None,
    *,
    owner: str | None = None,
    unit: str | None = None,
    function: str | None = None,
    source_path: str | None = None,
    source_sha256: str | None = None,
    base_commit: str | None = None,
    target_object_sha256: str | None = None,
    candidate_object_sha256: str | None = None,
    toolchain_sha256: str | None = None,
    frontier_source_sha256: str | None = None,
    parent_frontier_sha256: str | None = None,
    # Backward-compatible input alias.  It is interpreted as a parent frontier
    # and is not emitted as the current frontier identity.
    frontier_sha256: str | None = None,
    source_span_metadata: Mapping[str, Any] | None = None,
    strict_row_ids: Sequence[str] | None = None,
    data_row_ids: Sequence[str] | None = None,
    physical_difference_ids: Sequence[str] | None = None,
    window_radius: int = DEFAULT_WINDOW_RADIUS,
    cluster_gap: int = DEFAULT_CLUSTER_GAP,
    max_output_bytes: int = MAX_OUTPUT_BYTES,
) -> dict[str, Any]:
    """Build one deterministic reconstruction packet from a focus report.

    ``binding`` is the preferred call form for integrations.  Individual
    keyword fields are supported for small callers and fixtures.  The
    ``source_span_metadata`` name is an alias for ``source_span``.
    """

    report = _mapping(focus_report, "focus_report")
    if report.get("schema") != "focus_symbol_report/v1":
        raise ReconstructionPacketError(
            "focus_report.schema must be focus_symbol_report/v1", code="schema_mismatch"
        )
    if report.get("authority_advanced") is True:
        raise ReconstructionPacketError(
            "focus report already advanced authority", code="authority_advanced"
        )
    if source_span is None:
        source_span = source_span_metadata
    if type(window_radius) is not int:
        raise ReconstructionPacketError("window_radius must be an integer")
    if window_radius < 0 or window_radius > MAX_WINDOW_RADIUS:
        raise ReconstructionPacketError(f"window_radius must be between 0 and {MAX_WINDOW_RADIUS}")
    if type(cluster_gap) is not int:
        raise ReconstructionPacketError("cluster_gap must be an integer")
    if cluster_gap < 0 or cluster_gap > MAX_CLUSTER_GAP:
        raise ReconstructionPacketError(f"cluster_gap must be between 0 and {MAX_CLUSTER_GAP}")
    if isinstance(max_output_bytes, bool) or not isinstance(max_output_bytes, int) or max_output_bytes <= 0:
        raise ReconstructionPacketError("max_output_bytes must be a positive integer")
    if max_output_bytes > MAX_OUTPUT_BYTES:
        raise ReconstructionPacketError(f"max_output_bytes cannot exceed {MAX_OUTPUT_BYTES}")

    values, span_material = _binding_values(
        binding,
        {
            "owner": owner,
            "unit": unit,
            "function": function,
            "source_path": source_path,
            "source_sha256": source_sha256,
            "base_commit": base_commit,
            "target_object_sha256": target_object_sha256,
            "candidate_object_sha256": candidate_object_sha256,
            "toolchain_sha256": toolchain_sha256,
            "frontier_source_sha256": frontier_source_sha256,
            "parent_frontier_sha256": parent_frontier_sha256,
            "frontier_sha256": frontier_sha256,
        },
        source_span,
    )
    _check_report_binding(report, values)
    # Authenticate the producer's unmodified focus artifact before accepting
    # compact identity overrides.  The overrides are then independently
    # recomputed against these authenticated rows.
    focus_digest = _focus_digest(report)
    # Validate the producer's original focus digest before accepting any
    # caller-supplied compact row IDs.  Then reseal the post-injection report;
    # otherwise a forged ID list would sit outside the focus digest while still
    # becoming part of the packet's residual census.
    if strict_row_ids is not None or data_row_ids is not None:
        report = dict(report)
        if strict_row_ids is not None:
            report["strict_row_ids"] = _validated_id_override(
                strict_row_ids, "strict_row_ids"
            )
        if data_row_ids is not None:
            report["data_row_ids"] = _validated_id_override(
                data_row_ids, "data_row_ids"
            )
    channels = _channels(report)
    if not channels:
        raise ReconstructionPacketError("focus report has no strict/data channels", code="missing_channels")

    events, residual_ids, target_rows, candidate_rows = _build_residual_events(
        report,
        channels,
        function=values["function"],
    )
    clusters = _cluster_events(events, cluster_gap)
    _assign_mirror_groups(clusters, events)
    broad_reason = _broad_reason(events, clusters)
    full_residual_ids = {
        channel: list(values) for channel, values in residual_ids.items()
    }
    if broad_reason is None:
        packet_events = events
        packet_clusters = clusters
        windows = _instruction_windows(
            packet_clusters,
            target_rows,
            candidate_rows,
            radius=window_radius,
        )
        packet_metadata: dict[str, Any] = {
            "residual_rows_complete": True,
            "residual_rows_total_count": len(events),
            "residual_rows_full_sha256": canonical_sha256(events),
            "causal_clusters_complete": True,
            "causal_clusters_total_count": len(clusters),
            "causal_clusters_full_sha256": canonical_sha256(clusters),
            "selected_cluster_count": len(clusters),
        }
        for channel in ("strict", "data"):
            packet_metadata.update(
                _broad_channel_census(
                    full_residual_ids[channel],
                    full_residual_ids[channel],
                    channel=channel,
                )
            )
        decomposition_regions: list[dict[str, Any]] = []
    else:
        # Do not construct all windows for a broad function.  This is the
        # failure mode that previously grew ev_CapBobleOMExec beyond the packet
        # limit.  Keep a small first/tail decomposition sample and full-list
        # digests/counts so a later region pass can continue without claiming a
        # complete source reconstruction.
        packet_events, packet_clusters, packet_metadata = _representative_broad_content(
            events, clusters
        )
        bounded_ids, packet_clusters, channel_metadata = _reconcile_broad_cross_links(
            full_residual_ids,
            packet_events,
            packet_clusters,
            clusters,
        )
        residual_ids = bounded_ids
        packet_metadata.update(channel_metadata)
        windows = _bounded_windows(
            _instruction_windows(
                packet_clusters,
                target_rows,
                candidate_rows,
                radius=min(window_radius, MAX_BROAD_WINDOW_RADIUS),
            )
        )
        decomposition_regions = [
            {
                key: cluster[key]
                for key in (
                    "cluster_id",
                    "first_index",
                    "last_index",
                    "row_indices",
                    "residual_event_count",
                    "channels",
                    "mirror_group",
                    "mirror_group_size",
                    "window_ids",
                )
                if key in cluster
            }
            for cluster in packet_clusters
        ]
    # Window assignment mutates each retained cluster's ``window_ids``.  Seal
    # the representation that is actually emitted, not the pre-window object.
    if broad_reason is None and packet_metadata.get("causal_clusters_complete"):
        packet_metadata["causal_clusters_full_sha256"] = canonical_sha256(packet_clusters)
    elif broad_reason is not None:
        # A retained cluster may have a deliberately truncated row-ID list or
        # the packet may contain only representative clusters.  Keep the full
        # producer digest above, and separately bind the exact representation
        # that the verifier will inspect after window IDs are attached.
        packet_metadata["causal_clusters_retained_sha256"] = canonical_sha256(packet_clusters)

    # The strict side normally carries the full function.  If it is diff-only,
    # use the data side only when strict has no instructions at all.
    strict_target = _indexed_rows(_side_rows(channels.get("strict", {}), "target"))
    strict_candidate = _indexed_rows(_side_rows(channels.get("strict", {}), "candidate"))
    if not strict_target and not strict_candidate:
        strict_target, strict_candidate = target_rows, candidate_rows
    control_target = _control_flow(strict_target)
    control_candidate = _control_flow(strict_candidate)
    stack_target = _stack_summary(strict_target)
    stack_candidate = _stack_summary(strict_candidate)
    physical = _physical(report, physical_difference_ids)
    if broad_reason is not None:
        physical = _bounded_physical(physical)
    status, status_reason = _status(
        report,
        events,
        clusters,
        physical,
        control_target,
        control_candidate,
    )
    exact_terminal_possible, exact_terminal_reason = _exact_terminal_constraint(
        report,
        physical,
        control_target,
        control_candidate,
    )
    if broad_reason is not None:
        status = "UNKNOWN"
        status_reason = broad_reason
        exact_terminal_possible = False
        exact_terminal_reason = "broad_residual_requires_decomposition"
    if broad_reason is not None:
        output_control_target = _bounded_control_flow(control_target)
        output_control_candidate = _bounded_control_flow(control_candidate)
        output_stack_target = _bounded_stack_summary(stack_target)
        output_stack_candidate = _bounded_stack_summary(stack_candidate)
    else:
        output_control_target = control_target
        output_control_candidate = control_candidate
        output_stack_target = stack_target
        output_stack_candidate = stack_candidate
    source_span_copy = span_material["source_span"]

    body: dict[str, Any] = {
        "schema": SCHEMA,
        "schema_version": SCHEMA_VERSION,
        "owner": values["owner"],
        "unit": values["unit"],
        "function": values["function"],
        "source_path": values["source_path"],
        "source_sha256": values["source_sha256"],
        "base_commit": values["base_commit"],
        "target_object_sha256": values["target_object_sha256"],
        "candidate_object_sha256": values["candidate_object_sha256"],
        "toolchain_sha256": values["toolchain_sha256"],
        "frontier_source_sha256": values["frontier_source_sha256"],
        "parent_frontier_sha256": values["parent_frontier_sha256"],
        "source_span": source_span_copy,
        "source_span_sha256": canonical_sha256(source_span_copy),
        "focus_artifact_sha256": focus_digest,
        "focus_report_schema": report.get("schema"),
        "strict_residuals": residual_ids["strict"],
        "data_residuals": residual_ids["data"],
        "strict_residual_count": len(residual_ids["strict"]),
        "data_residual_count": len(residual_ids["data"]),
        "residual_event_count": len(events),
        **packet_metadata,
        "residual_rows": packet_events,
        "causal_clusters": packet_clusters,
        "causal_cluster_count": len(clusters),
        "instruction_windows_complete": broad_reason is None,
        "decomposition_regions": decomposition_regions,
        "status": status,
        "exact_terminal_possible": exact_terminal_possible,
        "exact_terminal_reason": exact_terminal_reason,
        "target_first_signal": {
            "status": status,
            "reason": status_reason,
            "exact_terminal_possible": exact_terminal_possible,
            "exact_terminal_reason": exact_terminal_reason,
            "first_residual_index": events[0]["anchor_index"] if events else None,
            "cluster_count": len(clusters),
            "owner_inference": "none",
            "next_action": "DECOMPOSE" if broad_reason is not None else "CRACK" if status == "READY" else "PIVOT",
            "decomposition_required": broad_reason is not None,
            "decomposition_regions": decomposition_regions,
        },
        "instruction_windows": windows,
        "control_flow": {"target": output_control_target, "candidate": output_control_candidate},
        "stack_relative": {"target": output_stack_target, "candidate": output_stack_candidate},
        "machine_summary": {
            "target": _machine_summary(strict_target, output_stack_target),
            "candidate": _machine_summary(strict_candidate, output_stack_candidate),
        },
        "physical_relocations": physical,
        "physical_relocation_differences": physical["differences"],
        "physical_difference_ids": physical["difference_ids"],
        "physical_difference_ids_sha256": physical["difference_ids_sha256"],
        "reconstruction_policy": {
            "target_first": True,
            "donor_required": False,
            "history_required": False,
            "window_radius": window_radius,
            "cluster_gap": cluster_gap,
            "source_text_emitted": False,
            "source_patch_emitted": False,
            "compile_authorized": False,
            "broad_residual_requires_decomposition": broad_reason is not None,
        },
        "authority_advanced": False,
        "diagnostic_only": True,
    }
    encoded_body = _canonical(body)
    # Include the digest field in the size check as well.  Returning a packet
    # beyond the contract is never preferable to a deterministic fail-closed
    # diagnostic that tells the caller to narrow the focus report.
    final_size = len(encoded_body) + len(b',"packet_sha256":"') + 64 + 1
    if final_size > max_output_bytes:
        raise ReconstructionPacketError(
            f"reconstruction packet exceeds {max_output_bytes} bytes ({final_size})",
            code="output_limit",
        )
    return seal(body)


# Descriptive aliases keep the pure builder easy to discover from integrations.
build_reconstruction_packet = build_packet
build_from_focus_report = build_packet


def seal(value: Mapping[str, Any]) -> dict[str, Any]:
    """Return a copy sealed with the packet self-digest.

    The helper is intentionally public so a caller adding only an external
    storage descriptor can re-seal a packet without changing the digest rule.
    Callers should not use it to mutate evidence fields after construction.
    """

    result = dict(value)
    result.pop("packet_sha256", None)
    result["packet_sha256"] = canonical_sha256(result)
    return result


_PACKET_REQUIRED_FIELDS = frozenset(
    {
        "schema",
        "schema_version",
        "owner",
        "unit",
        "function",
        "source_path",
        "source_sha256",
        "base_commit",
        "target_object_sha256",
        "candidate_object_sha256",
        "toolchain_sha256",
        "frontier_source_sha256",
        "parent_frontier_sha256",
        "source_span",
        "source_span_sha256",
        "focus_artifact_sha256",
        "focus_report_schema",
        "strict_residuals",
        "data_residuals",
        "strict_residual_count",
        "data_residual_count",
        "residual_event_count",
        "residual_rows_complete",
        "residual_rows_total_count",
        "residual_rows_full_sha256",
        "residual_rows",
        "causal_clusters_complete",
        "causal_clusters_total_count",
        "causal_clusters_full_sha256",
        "causal_clusters",
        "causal_cluster_count",
        "selected_cluster_count",
        "instruction_windows_complete",
        "instruction_windows",
        "decomposition_regions",
        "status",
        "exact_terminal_possible",
        "exact_terminal_reason",
        "target_first_signal",
        "control_flow",
        "stack_relative",
        "machine_summary",
        "physical_relocations",
        "physical_relocation_differences",
        "physical_difference_ids",
        "physical_difference_ids_sha256",
        "reconstruction_policy",
        "authority_advanced",
        "diagnostic_only",
        "packet_sha256",
    }
)

_PACKET_OMISSION_FIELDS = frozenset(
    {
        "strict_residuals_complete",
        "strict_residuals_total_count",
        "strict_residuals_full_sha256",
        "strict_residuals_omitted_count",
        "strict_residuals_omitted_sha256",
        "data_residuals_complete",
        "data_residuals_total_count",
        "data_residuals_full_sha256",
        "data_residuals_omitted_count",
        "data_residuals_omitted_sha256",
        "residual_rows_omitted_count",
        "residual_rows_omitted_sha256",
        "causal_clusters_omitted_count",
        "causal_clusters_omitted_sha256",
        "causal_clusters_retained_sha256",
    }
)


def _verify_string_array(value: Any, label: str) -> list[str]:
    if not isinstance(value, list):
        raise ReconstructionPacketError(f"{label} must be an array", code="packet_shape")
    result: list[str] = []
    for index, item in enumerate(value):
        result.append(_text(item, f"{label}[{index}]", max_bytes=2048))
    if len(result) != len(set(result)):
        raise ReconstructionPacketError(f"{label} contains duplicates", code="packet_shape")
    return result


def _verify_channel_census(
    value: Mapping[str, Any],
    channel: str,
    row_ids: Sequence[str],
    *,
    require: bool,
) -> None:
    """Validate a compact channel census and its actual omitted-set digest.

    The full row-ID list is intentionally not serialized for broad packets.
    The producer still binds it with a full digest and reports the digest of
    the exact omitted sequence.  For a complete census the verifier can
    recompute both values; for an incomplete census it verifies the count
    equation, the retained-subset relation, and the distinct omitted digest
    supplied by the producer.  A broad packet cannot silently fall back to the
    legacy retained-count-only representation.
    """

    prefix = f"{channel}_residuals"
    fields = {
        f"{prefix}_complete",
        f"{prefix}_total_count",
        f"{prefix}_full_sha256",
        f"{prefix}_omitted_count",
        f"{prefix}_omitted_sha256",
    }
    present = fields & set(value)
    if not present:
        if require:
            raise ReconstructionPacketError(
                f"packet {prefix} census is missing", code="packet_shape"
            )
        return
    if present != fields:
        raise ReconstructionPacketError(
            f"packet {prefix} census is incomplete", code="packet_shape"
        )
    complete = value[f"{prefix}_complete"]
    if type(complete) is not bool:
        raise ReconstructionPacketError(
            f"packet.{prefix}_complete must be Boolean", code="packet_shape"
        )
    total = _integer(value[f"{prefix}_total_count"], f"packet.{prefix}_total_count", minimum=0)
    omitted_count = _integer(
        value[f"{prefix}_omitted_count"], f"packet.{prefix}_omitted_count", minimum=0
    )
    full_digest = _sha256(value[f"{prefix}_full_sha256"], f"packet.{prefix}_full_sha256")
    omitted_digest = _sha256(
        value[f"{prefix}_omitted_sha256"], f"packet.{prefix}_omitted_sha256"
    )
    retained = list(row_ids)
    if len(retained) > total:
        raise ReconstructionPacketError(
            f"packet {prefix} retained IDs exceed total", code="packet_shape"
        )
    if complete:
        if total != len(retained) or omitted_count != 0:
            raise ReconstructionPacketError(
                f"packet {prefix} complete census count mismatch", code="packet_shape"
            )
        if full_digest != canonical_sha256(retained):
            raise ReconstructionPacketError(
                f"packet {prefix} full digest mismatch", code="packet_shape"
            )
        if omitted_digest != canonical_sha256([]):
            raise ReconstructionPacketError(
                f"packet {prefix} empty omission digest mismatch", code="packet_shape"
            )
    else:
        if omitted_count < 1 or len(retained) + omitted_count != total:
            raise ReconstructionPacketError(
                f"packet {prefix} omission count mismatch", code="packet_shape"
            )
        # A digest of the complete list is not an omitted-set digest.  This
        # explicit inequality catches the historical broad-packet bug where
        # residual_rows_omitted_sha256 accidentally hashed all events.
        if omitted_digest == full_digest:
            raise ReconstructionPacketError(
                f"packet {prefix} omitted digest is not an omitted set", code="packet_shape"
            )


def _verify_retained_sequence(
    value: Mapping[str, Any],
    *,
    field: str,
    total_field: str,
    complete_field: str,
    full_digest_field: str,
    retained_digest_field: str | None = None,
) -> tuple[list[Any], int, bool]:
    rows = value.get(field)
    if not isinstance(rows, list):
        raise ReconstructionPacketError(f"packet.{field} must be an array", code="packet_shape")
    total = _integer(value.get(total_field), f"packet.{total_field}")
    complete = value.get(complete_field)
    if type(complete) is not bool:
        raise ReconstructionPacketError(
            f"packet.{complete_field} must be Boolean", code="packet_shape"
        )
    _sha256(value.get(full_digest_field), f"packet.{full_digest_field}")
    if len(rows) > total:
        raise ReconstructionPacketError(f"packet.{field} exceeds total count", code="packet_shape")
    omitted_count_field = field.replace("rows", "rows_omitted_count").replace(
        "clusters", "clusters_omitted_count"
    )
    omitted_digest_field = field.replace("rows", "rows_omitted_sha256").replace(
        "clusters", "clusters_omitted_sha256"
    )
    if complete:
        if len(rows) != total:
            raise ReconstructionPacketError(
                f"packet.{field} complete count mismatch", code="packet_shape"
            )
        if canonical_sha256(rows) != value.get(full_digest_field):
            if retained_digest_field is None or value.get(retained_digest_field) != canonical_sha256(rows):
                raise ReconstructionPacketError(
                    f"packet.{field} full digest mismatch", code="packet_shape"
                )
        if omitted_count_field in value or omitted_digest_field in value:
            raise ReconstructionPacketError(
                f"packet.{field} has spurious omission fields", code="packet_shape"
            )
    else:
        omitted = _integer(
            value.get(omitted_count_field), f"packet.{omitted_count_field}", minimum=1
        )
        omitted_digest = _sha256(
            value.get(omitted_digest_field), f"packet.{omitted_digest_field}"
        )
        if len(rows) + omitted != total:
            raise ReconstructionPacketError(
                f"packet.{field} omission count mismatch", code="packet_shape"
            )
        if omitted_digest == value.get(full_digest_field):
            raise ReconstructionPacketError(
                f"packet.{field} omitted digest is not an omitted set", code="packet_shape"
            )
    return rows, total, complete


def verify_packet(packet: Mapping[str, Any]) -> None:
    """Validate a closed reconstruction packet, not merely its self-digest.

    These packets authorize candidate *ranking* inside an autonomous lane.  A
    self-hash alone is therefore insufficient: a caller could otherwise seal a
    three-field object or rewrite counts/terminal gates and have the CAS accept
    it.  The verifier deliberately checks the complete generated shape and all
    locally recomputable cross-links.
    """

    value = _mapping(packet, "packet")
    fields = set(value)
    missing = _PACKET_REQUIRED_FIELDS - fields
    extra = fields - _PACKET_REQUIRED_FIELDS - _PACKET_OMISSION_FIELDS
    if missing or extra:
        raise ReconstructionPacketError(
            f"packet field set is noncanonical (missing={sorted(missing)}, extra={sorted(extra)})",
            code="packet_shape",
        )
    if value.get("schema") != SCHEMA or value.get("schema_version") != SCHEMA_VERSION:
        raise ReconstructionPacketError("packet schema mismatch", code="schema_mismatch")
    if value.get("authority_advanced") is not False:
        raise ReconstructionPacketError("packet advanced authority", code="authority_advanced")
    if value.get("diagnostic_only") is not True:
        raise ReconstructionPacketError("packet is not diagnostic-only", code="packet_shape")
    expected = _sha256(value.get("packet_sha256"), "packet_sha256")
    actual = _digest_without(value, "packet_sha256")
    if actual != expected:
        raise ReconstructionPacketError("packet_sha256 mismatch", code="packet_hash_drift")
    if len(_canonical(value)) > MAX_OUTPUT_BYTES:
        raise ReconstructionPacketError("packet exceeds compact evidence limit", code="output_limit")

    for field in ("owner", "unit", "function"):
        _text(value.get(field), f"packet.{field}")
    _safe_relative_path(value.get("source_path"), "packet.source_path")
    for field in (
        "source_sha256",
        "target_object_sha256",
        "candidate_object_sha256",
        "toolchain_sha256",
        "frontier_source_sha256",
        "source_span_sha256",
        "focus_artifact_sha256",
    ):
        _sha256(value.get(field), f"packet.{field}")
    _commit(value.get("base_commit"), "packet.base_commit")
    parent = value.get("parent_frontier_sha256")
    if parent is not None:
        _sha256(parent, "packet.parent_frontier_sha256")
    if value.get("focus_report_schema") != "focus_symbol_report/v1":
        raise ReconstructionPacketError("packet focus schema mismatch", code="packet_shape")

    span = _mapping(value.get("source_span"), "packet.source_span")
    if canonical_sha256(span) != value.get("source_span_sha256"):
        raise ReconstructionPacketError("packet source span digest mismatch", code="packet_shape")
    start_line = _integer(span.get("start_line"), "packet.source_span.start_line", minimum=1)
    end_line = _integer(span.get("end_line"), "packet.source_span.end_line", minimum=start_line)
    if end_line < start_line:
        raise ReconstructionPacketError("packet source span is reversed", code="packet_shape")
    if "function" in span and span.get("function") != value.get("function"):
        raise ReconstructionPacketError("packet source span function drift", code="packet_shape")
    if "start_offset" in span or "end_offset" in span:
        start_offset = _integer(span.get("start_offset"), "packet.source_span.start_offset")
        end_offset = _integer(span.get("end_offset"), "packet.source_span.end_offset")
        if end_offset <= start_offset:
            raise ReconstructionPacketError("packet source offsets are invalid", code="packet_shape")
    for digest_field in ("span_sha256", "base_span_sha256"):
        if digest_field in span:
            _sha256(span[digest_field], f"packet.source_span.{digest_field}")

    strict_ids = _verify_string_array(value.get("strict_residuals"), "packet.strict_residuals")
    data_ids = _verify_string_array(value.get("data_residuals"), "packet.data_residuals")
    if _integer(value.get("strict_residual_count"), "packet.strict_residual_count") != len(strict_ids):
        raise ReconstructionPacketError("packet strict residual count mismatch", code="packet_shape")
    if _integer(value.get("data_residual_count"), "packet.data_residual_count") != len(data_ids):
        raise ReconstructionPacketError("packet data residual count mismatch", code="packet_shape")
    for channel, row_ids in (("strict", strict_ids), ("data", data_ids)):
        if any(
            not row_id.startswith(f"{channel}:")
            or (
                ":row:" not in row_id
                and not re.fullmatch(rf"{channel}:instruction:[0-9a-f]{{64}}", row_id)
            )
            for row_id in row_ids
        ):
            raise ReconstructionPacketError(
                f"packet {channel} residual identity is malformed", code="row_identity"
            )

    residual_rows, residual_total, residual_complete = _verify_retained_sequence(
        value,
        field="residual_rows",
        total_field="residual_rows_total_count",
        complete_field="residual_rows_complete",
        full_digest_field="residual_rows_full_sha256",
    )
    if _integer(value.get("residual_event_count"), "packet.residual_event_count") != residual_total:
        raise ReconstructionPacketError("packet residual event count mismatch", code="packet_shape")
    clusters, cluster_total, clusters_complete = _verify_retained_sequence(
        value,
        field="causal_clusters",
        total_field="causal_clusters_total_count",
        complete_field="causal_clusters_complete",
        full_digest_field="causal_clusters_full_sha256",
        retained_digest_field="causal_clusters_retained_sha256",
    )
    if _integer(value.get("causal_cluster_count"), "packet.causal_cluster_count") != cluster_total:
        raise ReconstructionPacketError("packet causal cluster count mismatch", code="packet_shape")
    if _integer(value.get("selected_cluster_count"), "packet.selected_cluster_count") != len(clusters):
        raise ReconstructionPacketError("packet selected cluster count mismatch", code="packet_shape")

    windows = value.get("instruction_windows")
    if not isinstance(windows, list):
        raise ReconstructionPacketError("packet instruction windows are invalid", code="packet_shape")
    if type(value.get("instruction_windows_complete")) is not bool:
        raise ReconstructionPacketError("packet window completeness is invalid", code="packet_shape")
    window_ids: set[str] = set()
    for index, window in enumerate(windows):
        item = _mapping(window, f"packet.instruction_windows[{index}]")
        window_id = _text(item.get("window_id"), f"packet.instruction_windows[{index}].window_id")
        if window_id in window_ids:
            raise ReconstructionPacketError("packet has duplicate window IDs", code="packet_shape")
        window_ids.add(window_id)
    strict_set, data_set = set(strict_ids), set(data_ids)
    # Every retained event is a source-anchored view of one or both channel
    # rows.  Validate its references before validating clusters so a compact
    # packet cannot retain an event which points at an omitted/forged ID.
    event_digests: set[str] = set()
    for index, event in enumerate(residual_rows):
        item = _mapping(event, f"packet.residual_rows[{index}]")
        event_digest = canonical_sha256(item)
        if event_digest in event_digests:
            raise ReconstructionPacketError(
                "packet has duplicate residual events", code="packet_shape"
            )
        event_digests.add(event_digest)
        _integer(item.get("anchor_index"), f"packet.residual_rows[{index}].anchor_index", minimum=0)
        channels = _verify_string_array(
            item.get("channels"), f"packet.residual_rows[{index}].channels"
        )
        if not channels or not set(channels) <= {"strict", "data"}:
            raise ReconstructionPacketError(
                "packet residual event channels are invalid", code="packet_shape"
            )
        row_map = _mapping(item.get("row_ids"), f"packet.residual_rows[{index}].row_ids")
        for channel, allowed in (("strict", strict_set), ("data", data_set)):
            row_values = _verify_string_array(
                row_map.get(channel), f"packet.residual_rows[{index}].row_ids.{channel}"
            )
            if row_values and channel not in channels:
                raise ReconstructionPacketError(
                    "packet residual event channel/reference drift", code="packet_shape"
                )
            if not set(row_values) <= allowed:
                raise ReconstructionPacketError(
                    "packet residual event references omitted census ID", code="packet_shape"
                )
        for side in ("target", "candidate"):
            _mapping(item.get(side), f"packet.residual_rows[{index}].{side}")
    cluster_ids: set[str] = set()
    mirror_counts: dict[str, int] = {}
    for index, cluster in enumerate(clusters):
        item = _mapping(cluster, f"packet.causal_clusters[{index}]")
        cluster_id = _text(item.get("cluster_id"), f"packet.causal_clusters[{index}].cluster_id")
        if cluster_id in cluster_ids:
            raise ReconstructionPacketError("packet has duplicate cluster IDs", code="packet_shape")
        cluster_ids.add(cluster_id)
        first = _integer(item.get("first_index"), f"packet.causal_clusters[{index}].first_index")
        last = _integer(item.get("last_index"), f"packet.causal_clusters[{index}].last_index", minimum=first)
        if last < first:
            raise ReconstructionPacketError("packet cluster range is reversed", code="packet_shape")
        rows = item.get("row_indices")
        if not isinstance(rows, list) or any(type(row) is not int for row in rows):
            raise ReconstructionPacketError("packet cluster row indices are invalid", code="packet_shape")
        if rows and (min(rows) < first or max(rows) > last):
            raise ReconstructionPacketError("packet cluster rows escape range", code="packet_shape")
        for field, allowed in (("strict_row_ids", strict_set), ("data_row_ids", data_set)):
            row_ids = _verify_string_array(item.get(field), f"packet.causal_clusters[{index}].{field}")
            if not set(row_ids) <= allowed:
                raise ReconstructionPacketError("packet cluster rows escape residual census", code="packet_shape")
            complete_field = f"{field}_complete"
            omitted_count_field = f"{field}_omitted_count"
            omitted_digest_field = f"{field}_omitted_sha256"
            nested_fields = {
                complete_field,
                omitted_count_field,
                omitted_digest_field,
            }
            present = nested_fields & set(item)
            if present:
                if present != nested_fields or type(item[complete_field]) is not bool:
                    raise ReconstructionPacketError(
                        "packet cluster row census is incomplete", code="packet_shape"
                    )
                omitted_count = _integer(
                    item[omitted_count_field],
                    f"packet.causal_clusters[{index}].{omitted_count_field}",
                    minimum=0,
                )
                omitted_digest = _sha256(
                    item[omitted_digest_field],
                    f"packet.causal_clusters[{index}].{omitted_digest_field}",
                )
                if item[complete_field]:
                    if omitted_count != 0 or omitted_digest != canonical_sha256([]):
                        raise ReconstructionPacketError(
                            "packet cluster complete omission census is invalid",
                            code="packet_shape",
                        )
                elif omitted_count < 1 or omitted_digest == canonical_sha256([]):
                    raise ReconstructionPacketError(
                        "packet cluster omission census is invalid", code="packet_shape"
                    )
        refs = _verify_string_array(item.get("window_ids"), f"packet.causal_clusters[{index}].window_ids")
        if any(ref not in window_ids for ref in refs):
            raise ReconstructionPacketError("packet cluster references unknown window", code="packet_shape")
        if len(rows) != len(set(rows)):
            raise ReconstructionPacketError(
                "packet cluster row indices contain duplicates", code="packet_shape"
            )
        mirror = _text(item.get("mirror_group"), f"packet.causal_clusters[{index}].mirror_group")
        mirror_counts[mirror] = mirror_counts.get(mirror, 0) + 1
        _integer(item.get("mirror_group_size"), f"packet.causal_clusters[{index}].mirror_group_size", minimum=1)
    for cluster in clusters:
        if cluster.get("mirror_group_size") != mirror_counts.get(cluster.get("mirror_group")):
            # Broad packets can omit other members of a mirror group; their
            # declared full size remains larger than the retained count.
            if clusters_complete or cluster.get("mirror_group_size", 0) < mirror_counts.get(cluster.get("mirror_group"), 0):
                raise ReconstructionPacketError("packet mirror-group size mismatch", code="packet_shape")

    # Windows point back to retained clusters as well as clusters pointing to
    # windows.  Check both directions and reject duplicate instruction indexes
    # in a retained window; otherwise a consumer could silently reinterpret a
    # bounded region after serialization.
    for index, window in enumerate(windows):
        item = _mapping(window, f"packet.instruction_windows[{index}]")
        refs = item.get("clusters")
        if refs is not None:
            refs = _verify_string_array(refs, f"packet.instruction_windows[{index}].clusters")
            if any(ref not in cluster_ids for ref in refs):
                raise ReconstructionPacketError(
                    "packet window references unknown cluster", code="packet_shape"
                )
        if "start_index" not in item:
            # The bounded-window omission sentinel is a valid compact marker;
            # its count/digest are checked when present.
            if "omitted_count" in item:
                _integer(item.get("omitted_count"), f"packet.instruction_windows[{index}].omitted_count", minimum=1)
                _sha256(item.get("omitted_sha256"), f"packet.instruction_windows[{index}].omitted_sha256")
            continue
        start = _integer(item.get("start_index"), f"packet.instruction_windows[{index}].start_index", minimum=0)
        end = _integer(item.get("end_index"), f"packet.instruction_windows[{index}].end_index", minimum=start)
        if end < start:
            raise ReconstructionPacketError("packet window range is reversed", code="packet_shape")
        _integer(item.get("radius"), f"packet.instruction_windows[{index}].radius", minimum=0, maximum=MAX_WINDOW_RADIUS)
        for side in ("target", "candidate"):
            rows = item.get(side)
            if not isinstance(rows, list):
                raise ReconstructionPacketError(
                    f"packet window {side} rows are invalid", code="packet_shape"
                )
            try:
                indexed = _indexed_rows(
                    [
                        _mapping(row, f"packet.instruction_windows[{index}].{side}[{n}]")
                        for n, row in enumerate(rows)
                    ]
                )
            except ReconstructionPacketError:
                raise
            if any(row_index < start or row_index > end for row_index in indexed):
                raise ReconstructionPacketError(
                    "packet window instruction escapes range", code="packet_shape"
                )

    status = value.get("status")
    if status not in {"READY", "UNKNOWN"}:
        raise ReconstructionPacketError("packet status is invalid", code="packet_shape")
    exact = value.get("exact_terminal_possible")
    if type(exact) is not bool:
        raise ReconstructionPacketError("packet exact-terminal flag is invalid", code="packet_shape")
    exact_reason = _text(value.get("exact_terminal_reason"), "packet.exact_terminal_reason")
    signal = _mapping(value.get("target_first_signal"), "packet.target_first_signal")
    if (
        signal.get("status") != status
        or signal.get("exact_terminal_possible") is not exact
        or signal.get("exact_terminal_reason") != exact_reason
        or signal.get("cluster_count") != cluster_total
    ):
        raise ReconstructionPacketError("packet target-first signal drift", code="packet_shape")
    action = signal.get("next_action")
    regions = value.get("decomposition_regions")
    if not isinstance(regions, list) or signal.get("decomposition_regions") != regions:
        raise ReconstructionPacketError("packet decomposition regions drift", code="packet_shape")
    for index, region in enumerate(regions):
        item = _mapping(region, f"packet.decomposition_regions[{index}]")
        region_cluster = _text(
            item.get("cluster_id"), f"packet.decomposition_regions[{index}].cluster_id"
        )
        if region_cluster not in cluster_ids:
            raise ReconstructionPacketError(
                "packet decomposition region references unknown cluster", code="packet_shape"
            )
        first = _integer(
            item.get("first_index"), f"packet.decomposition_regions[{index}].first_index", minimum=0
        )
        last = _integer(
            item.get("last_index"),
            f"packet.decomposition_regions[{index}].last_index",
            minimum=first,
        )
        row_indices = item.get("row_indices")
        if not isinstance(row_indices, list) or any(type(row) is not int for row in row_indices):
            raise ReconstructionPacketError(
                "packet decomposition region rows are invalid", code="packet_shape"
            )
        if row_indices and (min(row_indices) < first or max(row_indices) > last):
            raise ReconstructionPacketError(
                "packet decomposition region rows escape range", code="packet_shape"
            )
        for channel, allowed in (("strict", strict_set), ("data", data_set)):
            field = f"{channel}_row_ids"
            if field in item:
                row_values = _verify_string_array(
                    item.get(field), f"packet.decomposition_regions[{index}].{field}"
                )
                if not set(row_values) <= allowed:
                    raise ReconstructionPacketError(
                        "packet decomposition region references omitted census ID",
                        code="packet_shape",
                    )
        window_refs = item.get("window_ids")
        if window_refs is not None:
            window_refs = _verify_string_array(
                window_refs, f"packet.decomposition_regions[{index}].window_ids"
            )
            if any(ref not in window_ids for ref in window_refs):
                raise ReconstructionPacketError(
                    "packet decomposition region references unknown window", code="packet_shape"
                )
    if status == "UNKNOWN" and exact:
        raise ReconstructionPacketError("UNKNOWN packet claims exact terminal", code="packet_shape")
    if action == "DECOMPOSE":
        if status != "UNKNOWN" or not regions or signal.get("decomposition_required") is not True:
            raise ReconstructionPacketError("packet decomposition signal is invalid", code="packet_shape")
    elif action == "CRACK":
        if status != "READY" or regions or signal.get("decomposition_required") is not False:
            raise ReconstructionPacketError("packet crack signal is invalid", code="packet_shape")
    elif action != "PIVOT":
        raise ReconstructionPacketError("packet next action is invalid", code="packet_shape")

    # Broad DECOMPOSE packets are allowed to retain only representative row
    # IDs, but their top-level channel census must still bind the full count,
    # full digest, and the actual omitted-set count/digest.  Legacy READY
    # packets remain readable for compatibility with persisted campaign state;
    # newly generated READY packets carry the same stronger fields.
    require_channel_census = status == "UNKNOWN" and action == "DECOMPOSE"
    _verify_channel_census(
        value,
        "strict",
        strict_ids,
        require=require_channel_census,
    )
    _verify_channel_census(
        value,
        "data",
        data_ids,
        require=require_channel_census,
    )

    physical = _mapping(value.get("physical_relocations"), "packet.physical_relocations")
    differences = physical.get("differences")
    if not isinstance(differences, list):
        raise ReconstructionPacketError("packet physical differences are invalid", code="packet_shape")
    difference_ids = _verify_string_array(
        value.get("physical_difference_ids"), "packet.physical_difference_ids"
    )
    if value.get("physical_relocation_differences") != differences:
        raise ReconstructionPacketError("packet physical difference payload drift", code="packet_shape")
    if physical.get("difference_ids") != difference_ids:
        raise ReconstructionPacketError("packet physical difference identity drift", code="packet_shape")
    if _integer(physical.get("difference_count"), "packet.physical.difference_count") != len(differences):
        raise ReconstructionPacketError("packet physical difference count mismatch", code="packet_shape")
    if len(difference_ids) != len(differences):
        raise ReconstructionPacketError("packet physical ID count mismatch", code="packet_shape")
    if canonical_sha256(difference_ids) != value.get("physical_difference_ids_sha256"):
        raise ReconstructionPacketError("packet physical ID digest mismatch", code="packet_shape")
    if physical.get("difference_ids_sha256") != value.get("physical_difference_ids_sha256"):
        raise ReconstructionPacketError("packet physical digest drift", code="packet_shape")
    if canonical_sha256(differences) != physical.get("differences_sha256"):
        raise ReconstructionPacketError("packet physical payload digest mismatch", code="packet_shape")
    for field, values in (("differences", differences), ("difference_ids", difference_ids)):
        complete_field = f"{field}_complete"
        total_field = f"{field}_total_count"
        full_digest_field = f"{field}_full_sha256"
        omitted_count_field = f"{field}_omitted_count"
        omitted_digest_field = f"{field}_omitted_sha256"
        census_fields = {
            complete_field,
            total_field,
            full_digest_field,
            omitted_count_field,
            omitted_digest_field,
        }
        present = census_fields & set(physical)
        if not present:
            continue
        if present != census_fields or type(physical[complete_field]) is not bool:
            raise ReconstructionPacketError(
                f"packet physical {field} census is incomplete", code="packet_shape"
            )
        total = _integer(physical[total_field], f"packet.physical.{total_field}", minimum=0)
        omitted_count = _integer(
            physical[omitted_count_field],
            f"packet.physical.{omitted_count_field}",
            minimum=0,
        )
        full_digest = _sha256(
            physical[full_digest_field], f"packet.physical.{full_digest_field}"
        )
        omitted_digest = _sha256(
            physical[omitted_digest_field],
            f"packet.physical.{omitted_digest_field}",
        )
        if physical[complete_field]:
            if total != len(values) or omitted_count != 0 or full_digest != canonical_sha256(values):
                raise ReconstructionPacketError(
                    f"packet physical {field} complete census mismatch", code="packet_shape"
                )
            if omitted_digest != canonical_sha256([]):
                raise ReconstructionPacketError(
                    f"packet physical {field} empty omission digest mismatch",
                    code="packet_shape",
                )
        elif omitted_count < 1 or len(values) + omitted_count != total:
            raise ReconstructionPacketError(
                f"packet physical {field} omission count mismatch", code="packet_shape"
            )
    if type(physical.get("status_known")) is not bool or type(physical.get("difference_count_valid")) is not bool:
        raise ReconstructionPacketError("packet physical validity flags are invalid", code="packet_shape")
    for side in ("target", "candidate"):
        material = _mapping(physical.get(side), f"packet.physical.{side}")
        _integer(material.get("count"), f"packet.physical.{side}.count")
        if type(material.get("count_valid")) is not bool:
            raise ReconstructionPacketError("packet physical count validity is invalid", code="packet_shape")
        _sha256(material.get("relocations_sha256"), f"packet.physical.{side}.relocations_sha256")
    if exact and (
        status != "READY"
        or physical.get("status") != "exact"
        or differences
        or not physical.get("status_known")
        or not physical.get("difference_count_valid")
        or not all(physical[side].get("count_valid") for side in ("target", "candidate"))
    ):
        raise ReconstructionPacketError("packet exact-terminal physical gate is open", code="packet_shape")

    policy = _mapping(value.get("reconstruction_policy"), "packet.reconstruction_policy")
    expected_policy = {
        "target_first": True,
        "donor_required": False,
        "history_required": False,
        "source_text_emitted": False,
        "source_patch_emitted": False,
        "compile_authorized": False,
        "broad_residual_requires_decomposition": action == "DECOMPOSE",
    }
    if any(policy.get(field) is not expected_value for field, expected_value in expected_policy.items()):
        raise ReconstructionPacketError("packet reconstruction policy is invalid", code="packet_shape")
    _integer(policy.get("window_radius"), "packet.reconstruction_policy.window_radius", maximum=MAX_WINDOW_RADIUS)
    _integer(policy.get("cluster_gap"), "packet.reconstruction_policy.cluster_gap", maximum=MAX_CLUSTER_GAP)
    for field in ("control_flow", "stack_relative", "machine_summary"):
        material = _mapping(value.get(field), f"packet.{field}")
        if not isinstance(material.get("target"), Mapping) or not isinstance(material.get("candidate"), Mapping):
            raise ReconstructionPacketError(f"packet {field} sides are missing", code="packet_shape")

    # Silence unused-variable warnings in static analyzers while documenting
    # that completeness was intentionally validated above for both lists.
    _ = residual_complete


def _read_json(path: Path, label: str) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ReconstructionPacketError(f"cannot read {label}: {exc}") from exc
    return _mapping(value, label)


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--focus", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--owner", required=True)
    parser.add_argument("--unit", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--source-path", required=True)
    parser.add_argument("--source-sha256", required=True)
    parser.add_argument("--base-commit", required=True)
    parser.add_argument("--target-object-sha256", required=True)
    parser.add_argument("--candidate-object-sha256", required=True)
    parser.add_argument("--toolchain-sha256", required=True)
    parser.add_argument("--frontier-source-sha256")
    parser.add_argument("--parent-frontier-sha256")
    parser.add_argument("--frontier-sha256", help=argparse.SUPPRESS)
    parser.add_argument("--source-span", type=Path, required=True)
    parser.add_argument("--window-radius", type=int, default=DEFAULT_WINDOW_RADIUS)
    parser.add_argument("--cluster-gap", type=int, default=DEFAULT_CLUSTER_GAP)
    parser.add_argument("--max-output-bytes", type=int, default=MAX_OUTPUT_BYTES)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        focus = _read_json(args.focus, "focus report")
        span = _read_json(args.source_span, "source span")
        packet = build_packet(
            focus,
            {
                "owner": args.owner,
                "unit": args.unit,
                "function": args.function,
                "source_path": args.source_path,
                "source_sha256": args.source_sha256,
                "base_commit": args.base_commit,
                "target_object_sha256": args.target_object_sha256,
                "candidate_object_sha256": args.candidate_object_sha256,
                "toolchain_sha256": args.toolchain_sha256,
                "frontier_source_sha256": args.frontier_source_sha256,
                "parent_frontier_sha256": args.parent_frontier_sha256,
                "frontier_sha256": args.frontier_sha256,
            },
            span,
            window_radius=args.window_radius,
            cluster_gap=args.cluster_gap,
            max_output_bytes=args.max_output_bytes,
        )
        payload = _canonical(packet) + b"\n"
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_bytes(payload)
        return 0
    except ReconstructionPacketError as exc:
        print(f"owner_campaign_reconstruction: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
