"""Independent verifier for compact owner-campaign evidence and CRACK_REPORT/v1.

The campaign runner is deliberately not imported here.  This module rechecks
the compact evidence's canonical digests, identity bindings, residual identity
sets, physical relocation identity, and source/object/toolchain proof receipts
from an untrusted report.  A report is accepted only when every receipt needed
for its exact claim is either embedded or supplied through an explicit
hash-keyed resolver.

The verifier is diagnostic only: it never writes source, updates a frontier, or
advances authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


REPORT_SCHEMA = "CRACK_REPORT/v1"
MEASUREMENT_SCHEMA = "owner_campaign_measurement/v1"
FOCUS_SCHEMA = "owner_campaign_focus_evidence/v1"
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
COMMIT_RE = re.compile(r"[0-9a-f]{40}\Z")
# Focus and transient measurement receipts retain complete residual identity
# arrays for large owners.  The frontier and final CRACK_REPORT remain 64 KiB.
MAX_REPORT_COMPACT = 64 * 1024
MAX_FOCUS_COMPACT = 256 * 1024
# A measurement includes the complete focus payload plus source/object/
# toolchain receipts.  Keep it bounded by the owner evidence budget while
# allowing the separate 256 KiB focus bound to be fully represented.
MAX_MEASUREMENT_COMPACT = 16 * 1024 * 1024
MAX_IDENTITIES = 2048


class VerificationError(ValueError):
    """The supplied compact evidence cannot support an exact claim."""


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
        raise VerificationError(f"value is not canonical JSON: {exc}") from exc


def _sha_json(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise VerificationError(f"{label} is not a lowercase SHA-256")
    return value


def _commit(value: Any, label: str = "base_commit") -> str:
    if not isinstance(value, str) or COMMIT_RE.fullmatch(value) is None:
        raise VerificationError(f"{label} is not a commit SHA")
    return value


def _closed(value: Any, fields: set[str], label: str) -> dict[str, Any]:
    if not isinstance(value, Mapping) or set(value) != fields:
        actual = sorted(value) if isinstance(value, Mapping) else type(value).__name__
        raise VerificationError(f"{label} has invalid fields: {actual}")
    return dict(value)


def _strings(value: Any, label: str, *, max_items: int = MAX_IDENTITIES) -> list[str]:
    if not isinstance(value, list) or len(value) > max_items:
        raise VerificationError(f"{label} is not a bounded string array")
    if any(not isinstance(item, str) or not item or len(item) > 512 for item in value):
        raise VerificationError(f"{label} contains an invalid identity")
    if len(value) != len(set(value)):
        raise VerificationError(f"{label} contains duplicate identities")
    return list(value)


def _load_json(path: Path, label: str, expected_sha256: str | None = None) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise VerificationError(f"cannot read {label}: {exc}") from exc
    if expected_sha256 is not None and _sha_bytes(raw) != _sha(expected_sha256, label):
        raise VerificationError(f"{label} digest does not match its bound SHA-256")
    try:
        return json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise VerificationError(f"{label} is not valid UTF-8 JSON: {exc}") from exc


def _resolve_receipt(
    name: str,
    expected_sha256: Any,
    *,
    embedded: Mapping[str, Any] | None,
    resolver: Mapping[str, Any] | None,
) -> dict[str, Any]:
    digest = _sha(expected_sha256, f"{name} receipt")
    body: Any = None
    if isinstance(embedded, Mapping):
        body = embedded.get(name)
    if body is None and isinstance(resolver, Mapping):
        body = resolver.get(digest)
    if body is None:
        raise VerificationError(
            f"{name} proof receipt {digest} is not embedded or resolvable"
        )
    if not isinstance(body, Mapping):
        raise VerificationError(f"{name} proof receipt body is not an object")
    result = dict(body)
    receipt_digest = _sha(result.get("proof_sha256"), f"{name}.proof_sha256")
    unsigned = dict(result)
    unsigned.pop("proof_sha256", None)
    if _sha_json(unsigned) != receipt_digest or receipt_digest != digest:
        raise VerificationError(f"{name} proof receipt digest mismatch")
    if result.get("authority_advanced") is not False:
        raise VerificationError(f"{name} proof receipt advanced authority")
    return result


def _verify_focus(
    value: Any,
    *,
    owner: str,
    function: str,
    unit: str | None = None,
    source_path: str | None = None,
    base_commit: str | None = None,
    source_sha256: str,
    target_object_sha256: str,
) -> dict[str, Any]:
    fields = {
        "schema", "owner", "function", "unit", "source_path", "base_commit",
        "source_sha256", "target_object_sha256",
        "strict_rows", "data_rows", "physical_differences",
        "strict_row_ids", "strict_row_ids_sha256", "data_row_ids",
        "data_row_ids_sha256", "physical_difference_ids",
        "physical_difference_ids_sha256", "physical_target_identity_sha256",
        "physical_candidate_identity_sha256", "strict_row_count", "data_row_count",
        "physical_target_count", "physical_candidate_count",
        "physical_difference_count", "protected_total", "protected_losses",
        "sibling_identities", "sibling_digest", "focus_evidence_sha256",
    }
    value = _closed(value, fields, "focus evidence")
    unsigned = dict(value)
    digest = _sha(unsigned.pop("focus_evidence_sha256"), "focus_evidence_sha256")
    if _sha_json(unsigned) != digest:
        raise VerificationError("focus evidence digest is invalid")
    if (
        value["schema"] != FOCUS_SCHEMA
        or value["owner"] != owner
        or value["function"] != function
        or (unit is not None and value["unit"] != unit)
        or (source_path is not None and value["source_path"] != source_path)
        or (base_commit is not None and value["base_commit"] != base_commit)
        or value["source_sha256"] != source_sha256
        or value["target_object_sha256"] != target_object_sha256
    ):
        raise VerificationError("focus evidence identity is invalid")
    _commit(value["base_commit"], "focus.base_commit")
    if not isinstance(value["unit"], str) or not value["unit"]:
        raise VerificationError("focus.unit is invalid")
    if (
        not isinstance(value["source_path"], str)
        or not value["source_path"]
        or Path(value["source_path"]).is_absolute()
        or ".." in Path(value["source_path"]).parts
    ):
        raise VerificationError("focus.source_path is not relative")
    _strings(value["strict_rows"], "strict_rows")
    _strings(value["data_rows"], "data_rows")
    _strings(value["physical_differences"], "physical_differences")
    strict_ids = _strings(value["strict_row_ids"], "strict_row_ids")
    data_ids = _strings(value["data_row_ids"], "data_row_ids")
    physical_ids = _strings(value["physical_difference_ids"], "physical_difference_ids")
    for name, values in (
        ("strict_row_ids", strict_ids),
        ("data_row_ids", data_ids),
        ("physical_difference_ids", physical_ids),
    ):
        declared = _sha(value[f"{name}_sha256"], f"{name}_sha256")
        if declared != _sha_json(values):
            raise VerificationError(f"{name} digest is invalid")
    for name, number in (
        ("strict_row_count", value["strict_row_count"]),
        ("data_row_count", value["data_row_count"]),
        ("physical_target_count", value["physical_target_count"]),
        ("physical_candidate_count", value["physical_candidate_count"]),
        ("physical_difference_count", value["physical_difference_count"]),
        ("protected_total", value["protected_total"]),
        ("protected_losses", value["protected_losses"]),
    ):
        if type(number) is not int or number < 0:
            raise VerificationError(f"focus {name} is invalid")
    if value["strict_row_count"] != len(strict_ids):
        raise VerificationError("strict residual identity count mismatch")
    if value["data_row_count"] != len(data_ids):
        raise VerificationError("data residual identity count mismatch")
    if value["physical_difference_count"] != len(physical_ids):
        raise VerificationError("physical residual identity count mismatch")
    target_identity = _sha(
        value["physical_target_identity_sha256"],
        "physical_target_identity_sha256",
    )
    candidate_identity = _sha(
        value["physical_candidate_identity_sha256"],
        "physical_candidate_identity_sha256",
    )
    siblings = _strings(value["sibling_identities"], "sibling_identities")
    sibling_digest = _sha(value["sibling_digest"], "sibling_digest")
    if sibling_digest != _sha_json(siblings):
        raise VerificationError("protected sibling digest is invalid")
    if value["protected_losses"] != 0 and value["strict_row_count"] == 0:
        raise VerificationError("exact focus evidence has protected sibling losses")
    if value["protected_total"] < len(siblings):
        raise VerificationError("protected total is smaller than sibling census")
    if len(_canonical(value)) > MAX_FOCUS_COMPACT:
        raise VerificationError("focus evidence exceeds 256 KiB compact limit")
    # These fields are intentionally returned rather than discarded.  They are
    # the stable frontier identity arrays consumed by the core CAS validator.
    return {
        **value,
        "_strict_row_ids": strict_ids,
        "_data_row_ids": data_ids,
        "_physical_difference_ids": physical_ids,
        "_physical_target_identity_sha256": target_identity,
        "_physical_candidate_identity_sha256": candidate_identity,
        "_sibling_identities": siblings,
    }


def _verify_proof_binding(
    name: str,
    proof: Mapping[str, Any],
    *,
    report: Mapping[str, Any],
) -> None:
    schema = proof.get("schema")
    if name == "source_link":
        if schema not in {
            "owner_campaign_source_link_proof/v1",
            "owner_campaign_source_link_pending/v1",
        }:
            raise VerificationError("source-link receipt schema is invalid")
        if schema.endswith("pending/v1") or proof.get("status") == "not_proven":
            raise VerificationError("exact report cannot use a pending source-link receipt")
        for field, expected in (
            ("source_sha256", report["source_sha256"]),
            ("candidate_object_sha256", report["candidate_object_sha256"]),
        ):
            if proof.get(field) != expected:
                raise VerificationError(f"source-link {field} is not report-bound")
        for field in ("owner", "unit", "function"):
            if field in proof and field in report and proof[field] != report[field]:
                raise VerificationError(f"source-link {field} is not report-bound")
        if proof.get("object_origin") != "reconstructed_source":
            raise VerificationError("source-link object origin is not source-built")
        if proof.get("fallback_asm_used") is not False or proof.get("nonmatching_fallback_linked") is not False:
            raise VerificationError("source-link proof permits fallback input")
        _sha(proof.get("original_proof_sha256"), "source-link original_proof_sha256")
        command_count = proof.get("compiler_command_count")
        if type(command_count) is not int or command_count < 1:
            raise VerificationError("source-link compiler command count is invalid")
        _sha(proof.get("compiler_commands_sha256"), "source-link compiler_commands_sha256")
        _sha(
            proof.get("paired_compile_command_sha256"),
            "source-link paired_compile_command_sha256",
        )
        paired_commands = proof.get("paired_compile_commands")
        if (
            not isinstance(paired_commands, list)
            or not paired_commands
            or len(paired_commands) > 2
            or any(not isinstance(item, str) or not item for item in paired_commands)
            or not any(
                _sha_bytes(item.encode("utf-8"))
                == proof["paired_compile_command_sha256"]
                for item in paired_commands
            )
        ):
            raise VerificationError("source-link paired compiler command is invalid")
        for field in ("before_response_sha256", "after_response_sha256"):
            if proof.get(field) is not None:
                _sha(proof[field], f"source-link {field}")
    elif name == "object":
        if schema != "owner_campaign_object_proof/v1":
            raise VerificationError("object receipt schema is invalid")
        for field in ("owner", "unit", "function"):
            if field in proof and field in report and proof[field] != report[field]:
                raise VerificationError(f"object {field} is not report-bound")
        if proof.get("candidate_object_sha256") != report["candidate_object_sha256"]:
            raise VerificationError("object proof is not report-bound")
        if proof.get("source_sha256") != report["source_sha256"]:
            raise VerificationError("object proof source is not report-bound")
        if type(proof.get("candidate_object_size")) is not int or proof["candidate_object_size"] < 0:
            raise VerificationError("object proof size is invalid")
    elif name == "toolchain":
        if schema != "owner_campaign_toolchain_proof/v1":
            raise VerificationError("toolchain receipt schema is invalid")
        for field in ("owner", "unit", "function"):
            if field in proof and field in report and proof[field] != report[field]:
                raise VerificationError(f"toolchain {field} is not report-bound")
        if proof.get("descriptor_sha256") != report["toolchain_sha256"]:
            raise VerificationError("toolchain proof is not report-bound")
    else:
        raise VerificationError(f"unsupported proof receipt {name}")


def _verify_pending_source_link(
    proof: Mapping[str, Any], *, measurement: Mapping[str, Any]
) -> None:
    """Validate the non-exact measurement source-link placeholder.

    Snapshot measurements intentionally cannot claim a linked-binary proof.
    They still carry a self-hashed receipt so the missing proof is explicit and
    cannot be silently replaced by a later exact report.
    """

    if proof.get("schema") != "owner_campaign_source_link_pending/v1":
        raise VerificationError("pending source-link receipt schema is invalid")
    if proof.get("status") != "not_proven":
        raise VerificationError("pending source-link receipt status is invalid")
    if proof.get("authority_advanced") is not False:
        raise VerificationError("pending source-link receipt advanced authority")
    for field in ("campaign_id", "owner", "unit", "function", "source_sha256"):
        expected = measurement[field]
        if proof.get(field) != expected:
            raise VerificationError(f"pending source-link {field} is not measurement-bound")
    if proof.get("candidate_object_sha256") != measurement["candidate_object_sha256"]:
        raise VerificationError("pending source-link candidate object is not measurement-bound")


def _expected_report_identity(
    report: Mapping[str, Any], expected: Mapping[str, Any] | None
) -> None:
    if not expected:
        return
    for field in (
        "owner", "function", "campaign_id", "manifest_sha256", "unit",
        "source_path", "base_commit", "source_sha256", "target_object_sha256",
        "candidate_object_sha256", "toolchain_sha256",
    ):
        if field in expected and report.get(field) != expected[field]:
            raise VerificationError(f"report identity mismatch: {field}")


def _verify_report_shape(report: Any) -> dict[str, Any]:
    if not isinstance(report, Mapping):
        raise VerificationError("report is not an object")
    required = {
        "schema", "status", "completed", "authority_advanced", "owner", "function",
        "campaign_id", "manifest_sha256", "unit", "source_path", "base_commit",
        "frontier_sha256", "source_sha256", "target_object_sha256",
        "candidate_object_sha256", "toolchain_sha256", "result", "proof_receipts", "evidence",
        "completed_at", "report_sha256",
    }
    if set(report) != required:
        raise VerificationError("report has noncanonical fields")
    body = dict(report)
    digest = _sha(body.pop("report_sha256"), "report_sha256")
    if _sha_json(body) != digest:
        raise VerificationError("report digest is invalid")
    if report["schema"] != REPORT_SCHEMA or report["status"] != "exact":
        raise VerificationError("report is not an exact CRACK_REPORT/v1")
    if report["completed"] is not True or report["authority_advanced"] is not False:
        raise VerificationError("report terminal flags are invalid")
    for field in (
        "manifest_sha256", "frontier_sha256", "source_sha256",
        "target_object_sha256", "candidate_object_sha256",
        "toolchain_sha256",
    ):
        _sha(report[field], f"report.{field}")
    _commit(report["base_commit"])
    for field in ("owner", "function", "campaign_id", "unit", "source_path"):
        if not isinstance(report[field], str) or not report[field] or "\x00" in report[field]:
            raise VerificationError(f"report.{field} is invalid")
    if Path(report["source_path"]).is_absolute() or ".." in Path(report["source_path"]).parts:
        raise VerificationError("report.source_path is not a relative path")
    return dict(report)


def verify_report(
    report: Mapping[str, Any],
    *,
    focus_evidence: Mapping[str, Any] | None = None,
    proof_receipts: Mapping[str, Any] | None = None,
    expected: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Verify one compact exact report and its bound evidence.

    ``proof_receipts`` is keyed by receipt SHA-256, not by an untrusted name.
    Callers may instead embed proof bodies below ``report['evidence']['proofs']``.
    """

    value = _verify_report_shape(report)
    _expected_report_identity(value, expected)
    result = value["result"]
    if not isinstance(result, Mapping):
        raise VerificationError("report result is invalid")
    required_result = {
        "strict_percent", "data_percent", "target_bytes", "candidate_bytes",
        "strict_difference_count", "data_difference_count",
        "strict_row_ids_sha256", "data_row_ids_sha256",
        "physical_target_count", "physical_candidate_count",
        "physical_difference_count", "physical_difference_ids_sha256",
        "protected_total", "protected_losses", "protected_sibling_digest",
        "source_link_exact",
    }
    if set(result) != required_result:
        raise VerificationError("report result has noncanonical fields")
    if result["strict_percent"] != 100 or result["data_percent"] != 100:
        raise VerificationError("report percentage proof is not exact")
    for field in (
        "target_bytes", "candidate_bytes", "strict_difference_count",
        "data_difference_count", "physical_target_count",
        "physical_candidate_count", "physical_difference_count",
        "protected_total", "protected_losses",
    ):
        if type(result[field]) is not int or result[field] < 0:
            raise VerificationError(f"report result.{field} is invalid")
    if result["target_bytes"] != result["candidate_bytes"]:
        raise VerificationError("report function byte sizes differ")
    if any(result[field] != 0 for field in (
        "strict_difference_count", "data_difference_count",
        "physical_difference_count", "protected_losses",
    )) or result["physical_target_count"] != result["physical_candidate_count"]:
        raise VerificationError("report exact result still has residual differences")
    if result["source_link_exact"] is not True:
        raise VerificationError("report lacks source-link proof")
    for field in (
        "strict_row_ids_sha256", "data_row_ids_sha256",
        "physical_difference_ids_sha256", "protected_sibling_digest",
    ):
        _sha(result[field], f"report result.{field}")

    evidence = value["evidence"]
    if not isinstance(evidence, Mapping):
        raise VerificationError("report evidence is missing")
    evidence_fields = {
        "schema", "owner", "function", "unit", "source_path", "base_commit",
        "source_sha256", "target_object_sha256", "candidate_object_sha256",
        "focus_evidence_sha256", "strict_row_count", "strict_row_ids_sha256",
        "data_row_count", "data_row_ids_sha256", "physical_target_count",
        "physical_candidate_count", "physical_difference_count",
        "physical_difference_ids_sha256", "protected_total", "protected_losses",
        "protected_sibling_identities", "protected_sibling_digest", "proofs",
    }
    if set(evidence) != evidence_fields:
        raise VerificationError("report evidence has noncanonical fields")
    evidence_body = dict(evidence)
    if evidence.get("schema") != "owner_campaign_report_evidence/v1":
        raise VerificationError("report evidence schema is invalid")
    if any(evidence.get(field) != value.get(field) for field in (
        "owner", "function", "unit", "source_path", "base_commit",
        "source_sha256", "target_object_sha256", "candidate_object_sha256",
    )):
        raise VerificationError("report evidence identity is not report-bound")
    _commit(evidence["base_commit"], "evidence.base_commit")
    focus_digest = _sha(evidence["focus_evidence_sha256"], "focus_evidence_sha256")
    focus = focus_evidence
    if focus is None and isinstance(evidence.get("focus"), Mapping):
        # A full focus body can be embedded for a self-contained report.  A
        # summary-only reference must be resolved by the caller.
        candidate_focus = evidence["focus"]
        if candidate_focus.get("schema") == FOCUS_SCHEMA:
            focus = candidate_focus
    if focus is None:
        raise VerificationError(f"focus evidence {focus_digest} is unresolved")
    checked_focus = _verify_focus(
        focus,
        owner=value["owner"],
        function=value["function"],
        unit=value["unit"],
        source_path=value["source_path"],
        base_commit=value["base_commit"],
        source_sha256=value["source_sha256"],
        target_object_sha256=value["target_object_sha256"],
    )
    if checked_focus["focus_evidence_sha256"] != focus_digest:
        raise VerificationError("report focus evidence receipt mismatch")
    pairs = (
        ("strict_row_count", "strict_row_count"),
        ("strict_row_ids_sha256", "strict_row_ids_sha256"),
        ("data_row_count", "data_row_count"),
        ("data_row_ids_sha256", "data_row_ids_sha256"),
        ("physical_target_count", "physical_target_count"),
        ("physical_candidate_count", "physical_candidate_count"),
        ("physical_difference_count", "physical_difference_count"),
        ("physical_difference_ids_sha256", "physical_difference_ids_sha256"),
        ("protected_total", "protected_total"),
        ("protected_losses", "protected_losses"),
        ("protected_sibling_digest", "sibling_digest"),
    )
    for report_field, focus_field in pairs:
        if evidence[report_field] != checked_focus[focus_field]:
            raise VerificationError(f"report evidence {report_field} drifted from focus")
    if result["strict_difference_count"] != evidence["strict_row_count"]:
        raise VerificationError("report strict row count is inconsistent")
    if result["data_difference_count"] != evidence["data_row_count"]:
        raise VerificationError("report data row count is inconsistent")
    if result["physical_difference_count"] != evidence["physical_difference_count"]:
        raise VerificationError("report physical difference count is inconsistent")
    if result["strict_row_ids_sha256"] != evidence["strict_row_ids_sha256"]:
        raise VerificationError("report strict identity digest is inconsistent")
    if result["data_row_ids_sha256"] != evidence["data_row_ids_sha256"]:
        raise VerificationError("report data identity digest is inconsistent")
    if result["physical_difference_ids_sha256"] != evidence["physical_difference_ids_sha256"]:
        raise VerificationError("report physical identity digest is inconsistent")
    if result["protected_sibling_digest"] != evidence["protected_sibling_digest"]:
        raise VerificationError("report sibling digest is inconsistent")

    receipts = value["proof_receipts"]
    if not isinstance(receipts, Mapping):
        raise VerificationError("report proof_receipts is invalid")
    for name in ("source_link", "object", "toolchain"):
        _sha(receipts.get(name), f"proof_receipts.{name}")
    embedded = evidence.get("proofs")
    if not isinstance(embedded, Mapping):
        raise VerificationError("report proof bodies are missing")
    verified_receipts: dict[str, str] = {}
    for name in ("source_link", "object", "toolchain"):
        proof = _resolve_receipt(
            name, receipts[name], embedded=embedded, resolver=proof_receipts
        )
        _verify_proof_binding(name, proof, report=value)
        verified_receipts[name] = proof["proof_sha256"]
    if len(_canonical(value)) > MAX_REPORT_COMPACT:
        raise VerificationError("CRACK_REPORT/v1 exceeds 64 KiB compact limit")
    return {
        "schema": "owner_campaign_verification/v1",
        "report_sha256": value["report_sha256"],
        "owner": value["owner"],
        "function": value["function"],
        "focus_evidence_sha256": focus_digest,
        "strict_row_ids": checked_focus["_strict_row_ids"],
        "data_row_ids": checked_focus["_data_row_ids"],
        "physical_difference_ids": checked_focus["_physical_difference_ids"],
        "verified_receipts": verified_receipts,
        "authority_advanced": False,
        "verified": True,
    }


def verify_measurement(
    measurement: Mapping[str, Any],
    *,
    expected: Mapping[str, Any] | None = None,
    proof_receipts: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Verify a compact adapter measurement before it enters frontier CAS."""

    if not isinstance(measurement, Mapping):
        raise VerificationError("measurement is not an object")
    measurement_fields = {
        "schema", "phase", "campaign_id", "manifest_sha256", "owner", "unit",
        "function", "source_path", "base_commit", "source_sha256",
        "target_object_sha256", "toolchain_sha256", "measurement_producer_sha256",
        "candidate_object_sha256", "metrics", "report_receipts", "proofs",
        "focus_evidence", "exact_report", "measurement_sha256",
    }
    if set(measurement) != measurement_fields:
        raise VerificationError("measurement has noncanonical fields")
    body = dict(measurement)
    digest = _sha(body.pop("measurement_sha256", None), "measurement_sha256")
    if _sha_json(body) != digest:
        raise VerificationError("measurement digest is invalid")
    if measurement.get("schema") != MEASUREMENT_SCHEMA:
        raise VerificationError("measurement schema is invalid")
    if measurement.get("phase") not in {"snapshot", "candidate"}:
        raise VerificationError("measurement phase is invalid")
    for field in (
        "manifest_sha256", "source_sha256", "target_object_sha256",
        "toolchain_sha256", "measurement_producer_sha256", "candidate_object_sha256",
    ):
        _sha(measurement.get(field), f"measurement.{field}")
    _commit(measurement.get("base_commit"), "measurement.base_commit")
    for field in ("source_path", "owner", "unit", "function", "campaign_id"):
        value = measurement.get(field)
        if not isinstance(value, str) or not value or "\x00" in value:
            raise VerificationError(f"measurement.{field} is invalid")
    if Path(measurement["source_path"]).is_absolute() or ".." in Path(measurement["source_path"]).parts:
        raise VerificationError("measurement.source_path is not relative")
    if expected:
        for field, expected_value in expected.items():
            if field in measurement and measurement[field] != expected_value:
                raise VerificationError(f"measurement identity mismatch: {field}")
    focus = _verify_focus(
        measurement.get("focus_evidence"),
        owner=measurement["owner"],
        function=measurement["function"],
        unit=measurement["unit"],
        source_path=measurement["source_path"],
        base_commit=measurement["base_commit"],
        source_sha256=measurement["source_sha256"],
        target_object_sha256=measurement["target_object_sha256"],
    )
    if measurement["report_receipts"].get("focus") != focus["focus_evidence_sha256"]:
        raise VerificationError("measurement focus receipt mismatch")
    metrics = measurement.get("metrics")
    if not isinstance(metrics, Mapping):
        raise VerificationError("measurement metrics are missing")
    metric_fields = {
        "strict", "data", "physical_target_count", "physical_candidate_count",
        "physical_differences", "protected_total", "protected_losses",
        "source_link_exact",
    }
    if set(metrics) != metric_fields:
        raise VerificationError("measurement metrics have noncanonical fields")
    strict_metric = metrics.get("strict")
    data_metric = metrics.get("data")
    if not isinstance(strict_metric, Mapping) or not isinstance(data_metric, Mapping):
        raise VerificationError("measurement channel metrics are missing")
    channel_fields = {"target_bytes", "candidate_bytes", "differences"}
    for channel_name, channel in (("strict", strict_metric), ("data", data_metric)):
        if set(channel) != channel_fields:
            raise VerificationError(f"measurement {channel_name} metrics have noncanonical fields")
        for field, number in channel.items():
            if type(number) is not int or number < 0:
                raise VerificationError(f"measurement {channel_name}.{field} is invalid")
    for field in (
        "physical_target_count", "physical_candidate_count", "physical_differences",
        "protected_total", "protected_losses",
    ):
        number = metrics.get(field)
        if type(number) is not int or number < 0:
            raise VerificationError(f"measurement metrics.{field} is invalid")
    if type(metrics.get("source_link_exact")) is not bool:
        raise VerificationError("measurement source_link_exact is invalid")
    metric_pairs = (
        (strict_metric.get("differences"), focus["strict_row_count"], "strict"),
        (data_metric.get("differences"), focus["data_row_count"], "data"),
        (metrics.get("physical_target_count"), focus["physical_target_count"], "physical target"),
        (metrics.get("physical_candidate_count"), focus["physical_candidate_count"], "physical candidate"),
        (metrics.get("physical_differences"), focus["physical_difference_count"], "physical difference"),
        (metrics.get("protected_total"), focus["protected_total"], "protected total"),
        (metrics.get("protected_losses"), focus["protected_losses"], "protected losses"),
    )
    for actual, expected_value, label in metric_pairs:
        if actual != expected_value:
            raise VerificationError(f"measurement {label} metric drifted from focus")
    receipts = measurement.get("report_receipts")
    proofs = measurement.get("proofs")
    if not isinstance(receipts, Mapping) or not isinstance(proofs, Mapping):
        raise VerificationError("measurement proof receipts/bodies are missing")
    required_receipts = {"strict", "data", "physical", "siblings", "source_link"}
    if not required_receipts <= set(receipts) or len(receipts) > 16:
        raise VerificationError("measurement report receipts are incomplete")
    for name, digest in receipts.items():
        _sha(digest, f"measurement report receipt {name}")
    if set(proofs) != {"source_link", "object", "toolchain"}:
        raise VerificationError("measurement proof bodies are noncanonical")
    for name in ("source_link", "object", "toolchain"):
        proof = _resolve_receipt(
            name, receipts.get(name), embedded=proofs, resolver=proof_receipts
        )
        # Build a report-like identity for shared proof checks.
        fake_report = {
            "owner": measurement["owner"],
            "unit": measurement["unit"],
            "function": measurement["function"],
            "source_sha256": measurement["source_sha256"],
            "candidate_object_sha256": measurement["candidate_object_sha256"],
            "toolchain_sha256": measurement["toolchain_sha256"],
        }
        if (
            name == "source_link"
            and metrics.get("source_link_exact") is False
            and proof.get("schema") == "owner_campaign_source_link_pending/v1"
        ):
            _verify_pending_source_link(proof, measurement=measurement)
        else:
            _verify_proof_binding(name, proof, report=fake_report)
    if len(_canonical(measurement)) > MAX_MEASUREMENT_COMPACT:
        raise VerificationError("measurement exceeds 256 KiB compact limit")
    return {
        "schema": "owner_campaign_measurement_verification/v1",
        "measurement_sha256": digest,
        "focus_evidence_sha256": focus["focus_evidence_sha256"],
        "verified": True,
        "authority_advanced": False,
    }


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path, nargs="?")
    parser.add_argument("--measurement", type=Path)
    parser.add_argument("--focus", type=Path)
    parser.add_argument("--proof", action="append", default=[], metavar="SHA=PATH")
    parser.add_argument("--output", type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        if args.measurement is not None and args.report is not None:
            raise VerificationError("choose a report or --measurement, not both")
        if args.measurement is None and args.report is None:
            raise VerificationError("a report path or --measurement is required")
        focus = _load_json(args.focus, "focus evidence") if args.focus else None
        resolver: dict[str, Any] = {}
        for value in args.proof:
            if "=" not in value:
                raise VerificationError("--proof must be SHA256=PATH")
            digest, raw_path = value.split("=", 1)
            resolver[_sha(digest, "--proof SHA256")] = _load_json(
                Path(raw_path), "proof receipt", expected_sha256=digest
            )
        if args.measurement is not None:
            measurement = _load_json(args.measurement, "measurement")
            result = verify_measurement(measurement, proof_receipts=resolver)
        else:
            report = _load_json(args.report, "CRACK_REPORT/v1")
            result = verify_report(
                report,
                focus_evidence=focus,
                proof_receipts=resolver,
            )
        payload = _canonical(result).decode("utf-8") + "\n"
        if args.output:
            args.output.write_text(payload, encoding="utf-8")
        else:
            print(payload, end="")
        return 0
    except VerificationError as exc:
        print(f"owner_campaign_verify: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
