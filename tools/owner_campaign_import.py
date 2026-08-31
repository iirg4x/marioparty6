#!/usr/bin/env python3
"""Import authenticated per-cell recovery evidence into an owner campaign.

The v2 campaign is deliberately independent of the old approval/permit
protocol.  This module is the offline migration boundary: it reads legacy
JSON receipts, validates their meaning against an already initialised and
clean v2 campaign, and publishes only normalized v2 CAS/frontier state.  It
never reads or writes STOP, approvals, permits, or source files.

Inputs are compact receipts, not arbitrary prose.  Two legacy forms are
accepted:

* ``CRACK_REPORT/v1`` exact reports emitted by the old harness; and
* ``owner_campaign_legacy_exact/v1`` / ``owner_campaign_legacy_outcome/v1``
  compact receipts used by offline migration tools.

The output is deterministic for a given receipt.  Publication is protected by
the campaign CAS lock and rolls back every file touched by an interrupted
transaction.  Re-importing the same receipt is therefore a no-op.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from typing import Any, Iterable, Mapping, Sequence

try:  # package import
    from . import owner_campaign
except ImportError:  # pragma: no cover - direct script execution
    import owner_campaign  # type: ignore


LEGACY_IMPORT_SCHEMA = "owner_campaign_legacy_import/v1"
LEGACY_EXACT_SCHEMA = "owner_campaign_legacy_exact/v1"
LEGACY_OUTCOME_SCHEMA = "owner_campaign_legacy_outcome/v1"
LEGACY_INDEX_SCHEMA = "owner_campaign_legacy_index/v1"
REPORT_SCHEMA = "CRACK_REPORT/v1"
RESULT_SCHEMA = "crack_harness_result/v1"

LEGACY_EXACT_FIELDS = {
    "schema", "owner", "unit", "function", "base_commit", "source_sha256",
    "target_object_sha256", "candidate_object_sha256", "toolchain_sha256",
    "target_bytes", "candidate_bytes", "strict_differences", "data_differences",
    "physical_target_count", "physical_candidate_count", "physical_differences",
    "protected_total", "protected_losses", "source_link_exact", "compiled", "exact",
    "proof_receipts", "completed_at", "report_sha256",
}
LEGACY_OUTCOME_FIELDS = {
    "schema", "owner", "unit", "function", "base_commit", "source_sha256",
    "target_object_sha256", "candidate_object_sha256", "toolchain_sha256",
    "candidate_source_sha256", "status", "compiled", "strict_difference_delta",
    "data_difference_delta", "physical_difference_delta", "completed_at",
    "outcome_sha256",
}
LEGACY_INDEX_FIELDS = {
    "schema", "campaign_id", "manifest_sha256", "owner", "unit", "function",
    "legacy_kind", "legacy_path_sha256", "legacy_report_sha256", "source_sha256",
    "target_object_sha256", "candidate_object_sha256", "candidate_source_sha256",
    "status", "compiled", "imported_at", "legacy_index_sha256",
}


class LegacyImportError(owner_campaign.CampaignError):
    """A legacy receipt is invalid or cannot be safely migrated."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _digest_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _digest_json(value: Any) -> str:
    return _digest_bytes(_canonical(value))


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or owner_campaign.SHA_RE.fullmatch(value) is None:
        raise LegacyImportError(f"{label} is not a SHA-256")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise LegacyImportError(f"{label} is invalid")
    return value


def _strict(value: Any, fields: set[str], label: str) -> dict[str, Any]:
    if not isinstance(value, Mapping) or set(value) != fields:
        raise LegacyImportError(f"{label} is not a strict closed object")
    return dict(value)


def _timestamp(value: Any, label: str) -> str:
    if not isinstance(value, str):
        raise LegacyImportError(f"{label} is not a timestamp")
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise LegacyImportError(f"{label} is not a timestamp") from exc
    if parsed.tzinfo is None:
        raise LegacyImportError(f"{label} is not timezone-aware")
    return parsed.astimezone(dt.timezone.utc).isoformat().replace("+00:00", "Z")


def _input_file(raw: Path | str) -> Path:
    path = Path(raw).expanduser()
    try:
        details = path.lstat()
    except OSError as exc:
        raise LegacyImportError(f"legacy receipt is unreadable: {path}: {exc}") from exc
    if not path.is_file() or path.is_symlink() or getattr(details, "st_file_attributes", 0) & 0x400:
        raise LegacyImportError(f"legacy receipt must be a regular non-indirected file: {path}")
    return path.resolve()


def _read_json_file(path: Path) -> tuple[dict[str, Any], str]:
    try:
        raw = path.read_bytes()
        value = json.loads(raw.decode("utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise LegacyImportError(f"legacy receipt is unreadable: {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise LegacyImportError(f"legacy receipt is not an object: {path}")
    return dict(value), _digest_bytes(raw)


def _campaign_clean(root: Path, campaign: Mapping[str, Any]) -> None:
    """Require a clean initialized campaign before offline state migration."""

    try:
        result = subprocess.run(
            owner_campaign._git_argv(campaign, "status", "--porcelain", "--untracked-files=no"),
            cwd=root,
            capture_output=True,
            check=False,
        )
    except OSError as exc:
        raise LegacyImportError(f"campaign clean-state check failed: {exc}") from exc
    if result.returncode:
        detail = (result.stderr or result.stdout).decode("utf-8", "replace")[:400]
        raise LegacyImportError(f"campaign clean-state check failed: {detail}")
    if result.stdout.strip():
        raise LegacyImportError("legacy import requires a clean initialized campaign")
    if _digest_bytes(campaign["_source"].read_bytes()) != campaign["_base_source_sha256"]:
        raise LegacyImportError("campaign source does not match its base commit")


def _check_campaign_identity(
    campaign: Mapping[str, Any], value: Mapping[str, Any], *, function: str,
    source_sha256: str | None = None,
) -> None:
    expected = {
        "owner": campaign["owner"], "unit": campaign["unit"],
        "function": function, "base_commit": campaign["base_commit"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
    }
    for key, expected_value in expected.items():
        if value.get(key) != expected_value:
            raise LegacyImportError(f"legacy {key} is not campaign-bound")
    source = source_sha256 if source_sha256 is not None else value.get("source_sha256")
    _sha(source, "legacy source_sha256")
    if source != _digest_bytes(campaign["_source"].read_bytes()):
        raise LegacyImportError("legacy source hash is stale or not the clean campaign source")
    if function not in campaign["functions"]:
        raise LegacyImportError(f"legacy function is outside campaign scope: {function}")


def _integer(value: Any, label: str, *, minimum: int = 0) -> int:
    if type(value) is not int or value < minimum:
        raise LegacyImportError(f"{label} is invalid")
    return value


def _proof_summary(
    receipts: Mapping[str, Any], name: str,
) -> Mapping[str, Any]:
    raw = receipts.get(name)
    if not isinstance(raw, Mapping) or set(raw) != {"sha256", "summary"}:
        raise LegacyImportError(f"legacy proof receipt {name} is incomplete")
    _sha(raw.get("sha256"), f"legacy proof receipt {name}.sha256")
    summary = raw.get("summary")
    if not isinstance(summary, Mapping):
        raise LegacyImportError(f"legacy proof receipt {name}.summary is invalid")
    return summary


def _old_exact(value: Mapping[str, Any], file_sha256: str) -> dict[str, Any]:
    """Validate the old harness CRACK_REPORT meaning, not merely its digest."""

    if value.get("schema") != REPORT_SCHEMA:
        raise LegacyImportError("legacy exact report schema is invalid")
    report = dict(value)
    report_digest = _sha(report.pop("report_sha256", None), "legacy report_sha256")
    if report_digest != _digest_json(report):
        raise LegacyImportError("legacy exact report digest is invalid")
    if value.get("status") != "exact" or value.get("completed") is not True:
        raise LegacyImportError("legacy report is not a completed exact result")
    if value.get("authority_advanced") is not False:
        raise LegacyImportError("legacy report advanced authority")
    for name in ("owner", "function", "task_id", "base_commit"):
        _text(value.get(name), f"legacy {name}")
    source_sha = _sha(value.get("source_sha256"), "legacy source_sha256")
    target_sha = _sha(value.get("target_object_sha256"), "legacy target_object_sha256")
    candidate_sha = _sha(value.get("candidate_object_sha256"), "legacy candidate_object_sha256")
    completed_at = _timestamp(value.get("completed_at"), "legacy completed_at")
    result = value.get("result")
    if not isinstance(result, Mapping) or set(result) != {
        "strict_percent", "data_percent", "target_bytes", "candidate_bytes", "owner_gain",
    }:
        raise LegacyImportError("legacy exact report result is incomplete")
    if (
        result.get("strict_percent") != 100
        or result.get("data_percent") != 100
        or _integer(result.get("target_bytes"), "legacy target_bytes")
        != _integer(result.get("candidate_bytes"), "legacy candidate_bytes")
        or not isinstance(result.get("owner_gain"), (int, float))
        or isinstance(result.get("owner_gain"), bool)
        or float(result["owner_gain"]) <= 0
    ):
        raise LegacyImportError("legacy report result does not prove exactness")
    receipts = value.get("proof_receipts")
    required = {"precompile", "strict", "data", "focus", "siblings", "physical", "assess", "record"}
    if not isinstance(receipts, Mapping) or set(receipts) != required:
        raise LegacyImportError("legacy exact proof receipt set is incomplete")

    object_pair: tuple[str, str] | None = None
    proof_artifact: str | None = None
    for name in ("strict", "data", "focus", "siblings", "physical"):
        summary = _proof_summary(receipts, name)
        common = {"owner", "function", "candidate_source_sha256", "target_object_sha256", "candidate_object_sha256", "report_sha256"}
        expected_extra = {
            "strict": {"strict_percent", "target_bytes", "candidate_bytes", "differences"},
            "data": {"data_percent", "target_bytes", "candidate_bytes", "differences"},
            "focus": {"differing_rows"},
            "siblings": {"protected_total", "protected_losses"},
            "physical": {"target_count", "candidate_count", "differences"},
        }[name]
        if set(summary) != common | expected_extra:
            raise LegacyImportError(f"legacy {name} proof summary is incomplete")
        if summary["owner"] != value["owner"] or summary["function"] != value["function"] or summary["candidate_source_sha256"] != source_sha:
            raise LegacyImportError(f"legacy {name} proof summary is not source-bound")
        pair = (
            _sha(summary["target_object_sha256"], f"legacy {name} target_object_sha256"),
            _sha(summary["candidate_object_sha256"], f"legacy {name} candidate_object_sha256"),
        )
        artifact = _sha(summary["report_sha256"], f"legacy {name} report_sha256")
        if object_pair is None:
            object_pair, proof_artifact = pair, artifact
        elif pair != object_pair or artifact != proof_artifact:
            raise LegacyImportError("legacy proof summaries disagree on object/report identity")
        if name == "strict":
            if summary["strict_percent"] != 100 or summary["target_bytes"] != summary["candidate_bytes"] or summary["differences"] != 0:
                raise LegacyImportError("legacy strict proof is not exact")
        elif name == "data":
            if summary["data_percent"] != 100 or summary["target_bytes"] != summary["candidate_bytes"] or summary["differences"] != 0:
                raise LegacyImportError("legacy data proof is not exact")
        elif name == "focus":
            if summary["differing_rows"] != 0:
                raise LegacyImportError("legacy focus proof is not exact")
        elif name == "siblings":
            if summary["protected_losses"] != 0:
                raise LegacyImportError("legacy sibling proof has protected loss")
        elif name == "physical":
            if summary["target_count"] != summary["candidate_count"] or summary["differences"] != 0:
                raise LegacyImportError("legacy physical proof is not exact")
    assert object_pair is not None and proof_artifact is not None
    if object_pair != (target_sha, candidate_sha):
        raise LegacyImportError("legacy report object identity does not bind proof summaries")
    strict_summary = _proof_summary(receipts, "strict")
    if (
        result["target_bytes"] != strict_summary["target_bytes"]
        or result["candidate_bytes"] != strict_summary["candidate_bytes"]
    ):
        raise LegacyImportError("legacy report byte counts do not bind strict proof")
    assess = _proof_summary(receipts, "assess")
    assess_fields = {
        "schema", "owner", "function", "candidate_source_sha256", "target_object_sha256",
        "candidate_object_sha256", "owner_gain", "data_gain", "data_diff_delta", "physical_diff_delta",
    }
    optional_size = {
        "baseline_data_target_bytes", "baseline_data_candidate_bytes", "data_target_bytes",
        "data_candidate_bytes", "size_diff_delta",
    }
    if set(assess) not in {assess_fields, assess_fields | optional_size} or assess.get("schema") != "crack_assessment/v1":
        raise LegacyImportError("legacy assessment proof is incomplete")
    if (
        assess.get("owner") != value["owner"] or assess.get("function") != value["function"]
        or assess.get("candidate_source_sha256") != source_sha
        or assess.get("target_object_sha256") != target_sha
        or assess.get("candidate_object_sha256") != candidate_sha
        or not isinstance(assess.get("owner_gain"), (int, float))
        or isinstance(assess.get("owner_gain"), bool) or float(assess["owner_gain"]) <= 0
        or not isinstance(assess.get("data_gain"), (int, float))
        or float(assess["data_gain"]) < 0
        or _integer(assess.get("data_diff_delta"), "legacy data_diff_delta", minimum=-2**63) > 0
        or _integer(assess.get("physical_diff_delta"), "legacy physical_diff_delta", minimum=-2**63) > 0
    ):
        raise LegacyImportError("legacy assessment does not prove a non-regressing exact gain")
    if float(assess["owner_gain"]) != float(result["owner_gain"]):
        raise LegacyImportError("legacy report gain is not assessment-bound")
    if optional_size <= set(assess):
        for name in optional_size - {"size_diff_delta"}:
            _integer(assess.get(name), f"legacy assessment {name}")
        expected_size_delta = (
            abs(assess["data_target_bytes"] - assess["data_candidate_bytes"])
            - abs(assess["baseline_data_target_bytes"] - assess["baseline_data_candidate_bytes"])
        )
        size_delta = assess["size_diff_delta"]
        if type(size_delta) is not int or size_delta != expected_size_delta or size_delta > 0:
            raise LegacyImportError("legacy assessment size delta is not non-regressing")
    record = _proof_summary(receipts, "record")
    if set(record) != {
        "schema", "recorded", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "outcome",
        "admission_token_sha256", "admission_input_key", "record_sha256",
    } or record.get("schema") != "crack_central_record_receipt/v1":
        raise LegacyImportError("legacy central record proof is incomplete")
    if (
        record.get("recorded") is not True or record.get("owner") != value["owner"]
        or record.get("function") != value["function"]
        or record.get("candidate_source_sha256") != source_sha
        or record.get("target_object_sha256") != target_sha
        or record.get("candidate_object_sha256") != candidate_sha
        or record.get("outcome") != "exact"
    ):
        raise LegacyImportError("legacy central record does not prove exact outcome")
    _sha(record.get("record_sha256"), "legacy record_sha256")
    _sha(record.get("admission_token_sha256"), "legacy admission_token_sha256")
    _sha(record.get("admission_input_key"), "legacy admission_input_key")
    precompile = _proof_summary(receipts, "precompile")
    if set(precompile) != {
        "status", "reused", "skip_compile", "input_key", "admission_token", "expires_at", "authority_advanced",
    } or precompile.get("status") != "admitted" or precompile.get("skip_compile") is not False or precompile.get("authority_advanced") is not False:
        raise LegacyImportError("legacy precompile proof is not an admitted compile")
    _sha(precompile.get("input_key"), "legacy precompile input_key")
    _text(precompile.get("admission_token"), "legacy admission_token")
    _timestamp(precompile.get("expires_at"), "legacy expires_at")
    return {
        "kind": "exact", "legacy_schema": REPORT_SCHEMA, "legacy_report_sha256": report_digest,
        "legacy_file_sha256": file_sha256, "owner": value["owner"], "function": value["function"],
        "unit": None, "base_commit": value["base_commit"], "source_sha256": source_sha,
        "target_object_sha256": target_sha, "candidate_object_sha256": candidate_sha,
        "toolchain_sha256": None, "target_bytes": int(result["target_bytes"]),
        "candidate_bytes": int(result["candidate_bytes"]), "strict_differences": 0,
        "data_differences": 0, "physical_target_count": _integer(_proof_summary(receipts, "physical")["target_count"], "legacy physical target_count"),
        "physical_candidate_count": _integer(_proof_summary(receipts, "physical")["candidate_count"], "legacy physical candidate_count"),
        "physical_differences": 0, "protected_total": _integer(_proof_summary(receipts, "siblings")["protected_total"], "legacy protected_total"),
        "protected_losses": 0, "source_link_exact": True, "compiled": True,
        "completed_at": completed_at, "proof_receipts": dict(receipts),
    }


def _compact_exact(value: Mapping[str, Any], file_sha256: str) -> dict[str, Any]:
    report = _strict(value, LEGACY_EXACT_FIELDS, "legacy compact exact receipt")
    digest = _sha(report.pop("report_sha256", None), "legacy compact report_sha256")
    if digest != _digest_json(report):
        raise LegacyImportError("legacy compact exact digest is invalid")
    if report.get("schema") != LEGACY_EXACT_SCHEMA or report.get("compiled") is not True or report.get("exact") is not True:
        raise LegacyImportError("legacy compact receipt is not a completed exact result")
    for key in ("owner", "unit", "function", "base_commit"):
        _text(report.get(key), f"legacy compact {key}")
    for key in ("source_sha256", "target_object_sha256", "candidate_object_sha256", "toolchain_sha256"):
        _sha(report.get(key), f"legacy compact {key}")
    completed_at = _timestamp(report["completed_at"], "legacy compact completed_at")
    for key in ("target_bytes", "candidate_bytes", "strict_differences", "data_differences", "physical_target_count", "physical_candidate_count", "physical_differences", "protected_total", "protected_losses"):
        _integer(report[key], f"legacy compact {key}")
    if (
        report["strict_differences"] != 0 or report["data_differences"] != 0
        or report["target_bytes"] != report["candidate_bytes"]
        or report["physical_differences"] != 0
        or report["physical_target_count"] != report["physical_candidate_count"]
        or report["protected_losses"] != 0 or report["source_link_exact"] is not True
    ):
        raise LegacyImportError("legacy compact exactness gates are not closed")
    receipts = report["proof_receipts"]
    if not isinstance(receipts, Mapping) or set(receipts) != {"strict", "data", "physical", "siblings", "source_link"}:
        raise LegacyImportError("legacy compact proof receipts are incomplete")
    for name, receipt in receipts.items():
        _sha(receipt, f"legacy compact proof receipt {name}")
    return {
        "kind": "exact", "legacy_schema": LEGACY_EXACT_SCHEMA, "legacy_report_sha256": digest,
        "legacy_file_sha256": file_sha256, "owner": report["owner"], "function": report["function"],
        "unit": report["unit"], "base_commit": report["base_commit"],
        "source_sha256": report["source_sha256"], "target_object_sha256": report["target_object_sha256"],
        "candidate_object_sha256": report["candidate_object_sha256"], "toolchain_sha256": report["toolchain_sha256"],
        "target_bytes": report["target_bytes"], "candidate_bytes": report["candidate_bytes"],
        "strict_differences": 0, "data_differences": 0,
        "physical_target_count": report["physical_target_count"], "physical_candidate_count": report["physical_candidate_count"],
        "physical_differences": 0, "protected_total": report["protected_total"], "protected_losses": 0,
        "source_link_exact": True, "compiled": True, "completed_at": completed_at,
        "proof_receipts": dict(receipts),
    }


def _old_outcome(value: Mapping[str, Any], file_sha256: str, campaign: Mapping[str, Any]) -> dict[str, Any]:
    """Validate a compiled old no-gain result for dedupe-only migration."""

    if value.get("schema") == LEGACY_OUTCOME_SCHEMA:
        outcome = _strict(value, LEGACY_OUTCOME_FIELDS, "legacy compact outcome")
        digest_field = "outcome_sha256"
        legacy_digest: str | None = None
    elif value.get("schema") == RESULT_SCHEMA:
        fields = {
            "schema", "approval_id", "approval_sha256", "owner", "task_id", "function",
            "base_commit", "campaign_id", "attempt_sha256", "candidate_sha256", "base_sha256",
            "status", "expected_terminal", "terminal_expectation_met", "reason", "owner_gain",
            "predicted_rows", "receipts", "finished_at", "source_restored", "cleanup_status",
            "cleanup_errors", "authority_advanced", "result_sha256",
        }
        old_result = _strict(value, fields, "legacy terminal result")
        # Verify the legacy digest against the legacy object before adapting
        # its fields.  The normalized v2 record intentionally has a different
        # schema and therefore must not be used for this check.
        old_body = dict(old_result)
        old_digest = _sha(old_body.pop("result_sha256", None), "legacy result_sha256")
        if old_digest != _digest_json(old_body):
            raise LegacyImportError("legacy compiled outcome digest is invalid")
        legacy_digest = old_digest
        if old_result["authority_advanced"] is not False:
            raise LegacyImportError("legacy compiled outcome advanced authority")
        if _sha(old_result["base_sha256"], "legacy result base_sha256") != campaign["_base_source_sha256"]:
            raise LegacyImportError("legacy compiled outcome base source is stale")
        for name in ("approval_id", "owner", "task_id", "function", "base_commit", "campaign_id", "candidate_sha256"):
            _text(old_result[name], f"legacy result {name}")
        outcome = {
            "schema": LEGACY_OUTCOME_SCHEMA, "owner": old_result["owner"], "unit": campaign["unit"],
            "function": old_result["function"], "base_commit": old_result["base_commit"],
            "source_sha256": campaign["_base_source_sha256"],
            "target_object_sha256": None, "candidate_object_sha256": None,
            "toolchain_sha256": campaign["toolchain"]["sha256"],
            "candidate_source_sha256": old_result["candidate_sha256"], "status": old_result["status"],
            "compiled": True, "strict_difference_delta": 0, "data_difference_delta": 0,
            "physical_difference_delta": 0, "completed_at": old_result["finished_at"],
            "outcome_sha256": old_digest, "_old": old_result,
        }
        # The old result has already been checked above.  Skip the compact
        # schema digest check below because this adapted mapping is new data.
        digest_field = None
    else:
        raise LegacyImportError("legacy outcome schema is unsupported")
    if digest_field is not None:
        body = dict(outcome)
        digest = _sha(body.pop(digest_field, None), f"legacy {digest_field}")
        if digest != _digest_json(body):
            raise LegacyImportError("legacy compiled outcome digest is invalid")
        legacy_digest = digest
    if outcome.get("status") not in {"no_gain", "stale"} or outcome.get("compiled") is not True:
        # An improved legacy result is not an exact proof and must not be
        # silently promoted as a v2 frontier.  It needs a current v2
        # measurement; migration records only consumed neutral/stale cells.
        raise LegacyImportError("legacy outcome is not a compiled neutral dedupe result")
    candidate_source = _sha(outcome.get("candidate_source_sha256"), "legacy candidate_source_sha256")
    target = outcome.get("target_object_sha256")
    candidate_object = outcome.get("candidate_object_sha256")
    old = outcome.get("_old")
    if isinstance(old, Mapping):
        receipts = old.get("receipts")
        if not isinstance(receipts, Mapping):
            raise LegacyImportError("legacy no-gain result lacks compile receipts")
        compile_receipt = receipts.get("compile")
        if not isinstance(compile_receipt, Mapping):
            raise LegacyImportError("legacy no-gain result lacks a compile receipt")
        commands = [
            compile_receipt.get("baseline_command"),
            compile_receipt.get("candidate_command"),
        ]
        if any(
            not isinstance(command, Mapping)
            or command.get("returncode") != 0
            for command in commands
        ):
            raise LegacyImportError("legacy no-gain compile receipt is not successful")
        pair: tuple[str, str] | None = None
        for name in ("strict", "data", "physical"):
            receipt = receipts.get(name)
            if not isinstance(receipt, Mapping):
                raise LegacyImportError(f"legacy no-gain result lacks {name} receipt")
            summary = receipt.get("summary")
            if not isinstance(summary, Mapping):
                raise LegacyImportError(f"legacy no-gain {name} summary is invalid")
            if name in {"strict", "data"}:
                if summary.get("candidate_source_sha256") != candidate_source:
                    raise LegacyImportError(f"legacy no-gain {name} source binding is stale")
                pair_now = (
                    _sha(summary.get("target_object_sha256"), f"legacy no-gain {name} target object"),
                    _sha(summary.get("candidate_object_sha256"), f"legacy no-gain {name} candidate object"),
                )
                if pair is None:
                    pair = pair_now
                elif pair != pair_now:
                    raise LegacyImportError("legacy no-gain object summaries disagree")
        if pair is None:
            raise LegacyImportError("legacy no-gain object summaries are missing")
        target, candidate_object = pair
    else:
        _sha(target, "legacy target_object_sha256")
        _sha(candidate_object, "legacy candidate_object_sha256")
    outcome["target_object_sha256"] = target
    outcome["candidate_object_sha256"] = candidate_object
    _check_campaign_identity(
        campaign, outcome, function=outcome["function"],
        source_sha256=outcome.get("source_sha256"),
    )
    return {
        "kind": "outcome", "legacy_schema": value.get("schema"), "legacy_report_sha256": legacy_digest,
        "legacy_file_sha256": file_sha256, "owner": outcome["owner"], "function": outcome["function"],
        "unit": campaign["unit"], "base_commit": outcome["base_commit"],
        "base_source_sha256": campaign["_base_source_sha256"],
        "source_sha256": outcome["source_sha256"], "target_object_sha256": target,
        "candidate_object_sha256": candidate_object, "toolchain_sha256": campaign["toolchain"]["sha256"],
        "candidate_source_sha256": candidate_source, "status": outcome["status"], "compiled": True,
        "strict_difference_delta": _integer(outcome.get("strict_difference_delta", 0), "legacy strict_difference_delta", minimum=-2**63),
        "data_difference_delta": _integer(outcome.get("data_difference_delta", 0), "legacy data_difference_delta", minimum=-2**63),
        "physical_difference_delta": _integer(outcome.get("physical_difference_delta", 0), "legacy physical_difference_delta", minimum=-2**63),
        "completed_at": _timestamp(outcome["completed_at"], "legacy completed_at"),
    }


def _parse_receipt(path: Path, campaign: Mapping[str, Any], *, outcome: bool) -> dict[str, Any]:
    value, file_sha = _read_json_file(path)
    if outcome:
        return _old_outcome(value, file_sha, campaign)
    if value.get("schema") == REPORT_SCHEMA:
        result = _old_exact(value, file_sha)
        # Old reports predate unit/toolchain fields; these are supplied by the
        # clean campaign authority, never guessed from prose.
        result["unit"] = campaign["unit"]
        result["toolchain_sha256"] = campaign["toolchain"]["sha256"]
        return result
    if value.get("schema") == LEGACY_EXACT_SCHEMA:
        return _compact_exact(value, file_sha)
    raise LegacyImportError(f"unsupported legacy exact receipt schema: {value.get('schema')!r}")


def _verify_record_campaign(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> None:
    function = record["function"]
    if function not in campaign["functions"]:
        raise LegacyImportError(f"legacy function is outside campaign scope: {function}")
    if record["owner"] != campaign["owner"] or record["unit"] != campaign["unit"] or record["base_commit"] != campaign["base_commit"]:
        raise LegacyImportError("legacy owner/unit/base binding is stale")
    if record["source_sha256"] != _digest_bytes(campaign["_source"].read_bytes()):
        raise LegacyImportError("legacy source hash is stale")
    if record["target_object_sha256"] != campaign["target_object"]["sha256"]:
        raise LegacyImportError("legacy target object is not campaign-bound")
    if record["toolchain_sha256"] != campaign["toolchain"]["sha256"]:
        raise LegacyImportError("legacy toolchain is not campaign-bound")
    if record["protected_total"] != len(campaign["protected_exact_functions"]):
        raise LegacyImportError("legacy protected sibling census is incomplete")
    if record["protected_losses"] != 0 or record["source_link_exact"] is not True:
        raise LegacyImportError("legacy exact protected/source-link gates are not closed")


def _legacy_base_key(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> str:
    return _digest_json({
        "schema": "owner_campaign_legacy_base/v1", "campaign": campaign["manifest_sha256"],
        "function": record["function"], "base_commit": campaign["base_commit"],
        "source": record["source_sha256"], "target": record["target_object_sha256"],
        "toolchain": record["toolchain_sha256"],
    })


def _candidate_key(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> str:
    return _digest_json({
        "schema": "owner_campaign_legacy_candidate/v1", "campaign": campaign["manifest_sha256"],
        "function": record["function"], "candidate_source": record.get("candidate_source_sha256", record["source_sha256"]),
        "candidate_object": record["candidate_object_sha256"],
        # The old harness often emitted multiple receipts for one compiled
        # source/object pair.  The candidate identity, not the report path or
        # result status, is the dedupe key.  Keep the base source in the key so
        # the same candidate can be intentionally measured against a new
        # frontier without collapsing unrelated work.
        "base_source": record.get("base_source_sha256", record["source_sha256"]),
        "target": record["target_object_sha256"], "toolchain": record["toolchain_sha256"],
    })


def _focus(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> dict[str, Any]:
    # Legacy receipts counted the complete protected inventory, including the
    # function being imported.  v2 keeps the focus function's exactness in its
    # own strict/data/physical metrics, so its sibling census must exclude the
    # selected focus just like every live v2 measurement does.
    siblings = list(
        owner_campaign._protected_sibling_functions(campaign, record["function"])
    )
    body: dict[str, Any] = {
        "schema": "owner_campaign_focus_evidence/v1", "owner": campaign["owner"],
        "function": record["function"], "unit": campaign["unit"],
        "source_path": campaign["source_relpath"], "base_commit": campaign["base_commit"],
        "source_sha256": record["source_sha256"], "target_object_sha256": campaign["target_object"]["sha256"],
        "strict_rows": [], "data_rows": [], "physical_differences": [],
        "sibling_identities": siblings, "strict_row_ids": [],
        "strict_row_ids_sha256": _digest_json([]), "data_row_ids": [],
        "data_row_ids_sha256": _digest_json([]), "physical_difference_ids": [],
        "physical_difference_ids_sha256": _digest_json([]),
        "physical_target_identity_sha256": _digest_json({"legacy_report": record["legacy_report_sha256"], "side": "target"}),
        "physical_candidate_identity_sha256": _digest_json({"legacy_report": record["legacy_report_sha256"], "side": "target"}),
        "strict_row_count": 0, "data_row_count": 0,
        "physical_target_count": record["physical_target_count"],
        "physical_candidate_count": record["physical_candidate_count"],
        "physical_difference_count": 0, "protected_total": len(siblings),
        "protected_losses": 0, "sibling_digest": _digest_json(siblings),
    }
    body["focus_evidence_sha256"] = _digest_json(body)
    return body


def _proofs(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> dict[str, dict[str, Any]]:
    common = {
        "owner": campaign["owner"], "unit": campaign["unit"],
        "function": record["function"], "source_sha256": record["source_sha256"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "candidate_object_sha256": record["candidate_object_sha256"],
        "legacy_report_sha256": record["legacy_report_sha256"],
        "authority_advanced": False,
    }
    result: dict[str, dict[str, Any]] = {}
    for name in ("source_link", "object", "toolchain"):
        body = {
            "schema": f"owner_campaign_import_{name}_proof/v1", **common,
            **({"toolchain_sha256": record["toolchain_sha256"]} if name == "toolchain" else {}),
        }
        body["proof_sha256"] = _digest_json(body)
        result[name] = body
    return result


def _normalized_state(
    campaign: Mapping[str, Any], record: Mapping[str, Any],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    focus = _focus(campaign, record)
    proofs = _proofs(campaign, record)
    receipts = {name: proof["proof_sha256"] for name, proof in proofs.items()}
    receipts.update({
        "strict": _sha(_proof_summary(record["proof_receipts"], "strict")["sha256"], "legacy strict receipt")
        if record["legacy_schema"] == REPORT_SCHEMA else _sha(record["proof_receipts"]["strict"], "legacy strict receipt"),
        "data": _sha(_proof_summary(record["proof_receipts"], "data")["sha256"], "legacy data receipt")
        if record["legacy_schema"] == REPORT_SCHEMA else _sha(record["proof_receipts"]["data"], "legacy data receipt"),
        "physical": _sha(_proof_summary(record["proof_receipts"], "physical")["sha256"], "legacy physical receipt")
        if record["legacy_schema"] == REPORT_SCHEMA else _sha(record["proof_receipts"]["physical"], "legacy physical receipt"),
        "siblings": _sha(_proof_summary(record["proof_receipts"], "siblings")["sha256"], "legacy sibling receipt")
        if record["legacy_schema"] == REPORT_SCHEMA else _sha(record["proof_receipts"]["siblings"], "legacy sibling receipt"),
    })
    metrics = {
        "strict": {"target_bytes": record["target_bytes"], "candidate_bytes": record["candidate_bytes"], "differences": 0},
        "data": {"target_bytes": record["target_bytes"], "candidate_bytes": record["candidate_bytes"], "differences": 0},
        "physical_target_count": record["physical_target_count"], "physical_candidate_count": record["physical_candidate_count"],
        "physical_differences": 0, "protected_total": focus["protected_total"],
        "protected_losses": 0, "source_link_exact": True,
    }
    frontier_body = {
        "schema": owner_campaign.FRONTIER_SCHEMA, "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "unit": campaign["unit"], "function": record["function"],
        "source_relpath": campaign["source_relpath"], "source_sha256": record["source_sha256"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
        "candidate_object_sha256": record["candidate_object_sha256"], "metrics": metrics,
        "report_receipts": receipts, "focus_evidence_sha256": focus["focus_evidence_sha256"],
        "parent_frontier_sha256": None, "generation": 0, "retained_at": record["completed_at"],
    }
    frontier = {**frontier_body, "frontier_sha256": _digest_json(frontier_body)}
    report_body = {
        "schema": owner_campaign.REPORT_SCHEMA, "status": "exact", "completed": True,
        "authority_advanced": False, "owner": campaign["owner"], "function": record["function"],
        "campaign_id": campaign["campaign_id"], "manifest_sha256": campaign["manifest_sha256"],
        "unit": campaign["unit"], "source_path": campaign["source_relpath"],
        "base_commit": campaign["base_commit"], "frontier_sha256": frontier["frontier_sha256"],
        "source_sha256": record["source_sha256"], "target_object_sha256": campaign["target_object"]["sha256"],
        "candidate_object_sha256": record["candidate_object_sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
        "result": {
            "strict_percent": 100, "data_percent": 100, "target_bytes": record["target_bytes"],
            "candidate_bytes": record["candidate_bytes"], "strict_difference_count": 0,
            "data_difference_count": 0, "strict_row_ids_sha256": focus["strict_row_ids_sha256"],
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": record["physical_target_count"],
            "physical_candidate_count": record["physical_candidate_count"], "physical_difference_count": 0,
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": focus["protected_total"], "protected_losses": 0,
            "protected_sibling_digest": focus["sibling_digest"], "source_link_exact": True,
        },
        "proof_receipts": receipts,
        "evidence": {
            "schema": "owner_campaign_report_evidence/v1", "owner": campaign["owner"],
            "function": record["function"], "unit": campaign["unit"],
            "source_path": campaign["source_relpath"], "base_commit": campaign["base_commit"],
            "source_sha256": record["source_sha256"], "target_object_sha256": campaign["target_object"]["sha256"],
            "candidate_object_sha256": record["candidate_object_sha256"],
            "focus_evidence_sha256": focus["focus_evidence_sha256"], "strict_row_count": 0,
            "strict_row_ids_sha256": focus["strict_row_ids_sha256"], "data_row_count": 0,
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": record["physical_target_count"],
            "physical_candidate_count": record["physical_candidate_count"], "physical_difference_count": 0,
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": focus["protected_total"], "protected_losses": 0,
            "protected_sibling_identities": list(focus["sibling_identities"]),
            "protected_sibling_digest": focus["sibling_digest"], "proofs": proofs,
        },
        "completed_at": record["completed_at"],
    }
    report = {**report_body, "report_sha256": _digest_json(report_body)}
    return focus, frontier, report, proofs


def _json_payload(value: Any) -> bytes:
    return _canonical(value) + b"\n"


class _Transaction:
    """Small rollback journal for a bounded set of campaign state files."""

    def __init__(self) -> None:
        self._writes: list[tuple[Path, bytes]] = []
        self._before: dict[Path, bytes | None] = {}
        self._published: list[Path] = []

    def add(self, path: Path, payload: bytes) -> None:
        if path.exists():
            try:
                current = path.read_bytes()
            except OSError as exc:
                raise LegacyImportError(f"existing migration artifact is unreadable: {path}") from exc
            if current != payload:
                raise LegacyImportError(f"migration artifact conflict: {path}")
            return
        for index, (staged_path, staged_payload) in enumerate(self._writes):
            if staged_path != path:
                continue
            if staged_payload == payload:
                return
            # Exact and consumed imports may legitimately target the same
            # per-function JSONL ledger in one transaction.  Merge only
            # canonical, digest-bound records; every other path remains a
            # strict single-value write and conflicts fail closed.
            if path.suffix != ".jsonl":
                raise LegacyImportError(f"migration artifact conflict: {path}")
            try:
                previous = [json.loads(line) for line in staged_payload.decode("utf-8").splitlines() if line]
                incoming = [json.loads(line) for line in payload.decode("utf-8").splitlines() if line]
            except (UnicodeError, json.JSONDecodeError) as exc:
                raise LegacyImportError(f"migration ledger staging is invalid: {path}") from exc
            merged: list[Mapping[str, Any]] = []
            identities: dict[str, Mapping[str, Any]] = {}
            for item in [*previous, *incoming]:
                if not isinstance(item, Mapping):
                    raise LegacyImportError(f"migration ledger record is invalid: {path}")
                identity = item.get("candidate_key") or item.get("legacy_index_sha256")
                if not isinstance(identity, str):
                    raise LegacyImportError(f"migration ledger identity is missing: {path}")
                prior = identities.get(identity)
                if prior is not None and dict(prior) != dict(item):
                    raise LegacyImportError(f"migration ledger identity conflict: {path}")
                if prior is None:
                    identities[identity] = item
                    merged.append(item)
            self._writes[index] = (path, b"".join(_json_payload(item) for item in merged))
            return
        self._writes.append((path, payload))

    def commit(self) -> None:
        try:
            for path, payload in self._writes:
                self._before[path] = path.read_bytes() if path.exists() else None
                owner_campaign._atomic_bytes(path, payload)
                self._published.append(path)
        except BaseException as exc:
            try:
                self.rollback()
            except BaseException as rollback_exc:
                raise LegacyImportError(f"legacy import rollback failed: {rollback_exc}") from exc
            raise

    def rollback(self) -> None:
        for path in reversed(self._published):
            before = self._before.get(path)
            if before is None:
                path.unlink(missing_ok=True)
            else:
                owner_campaign._atomic_bytes(path, before)
        self._published.clear()
        # Atomic writers create parent directories before replacing a file.
        # Remove only empty directories that this bounded transaction may have
        # created; never recursively delete a campaign or repository tree.
        for path, _payload in reversed(self._writes):
            parent = path.parent
            while parent != parent.parent:
                try:
                    parent.rmdir()
                except OSError:
                    break
                parent = parent.parent


def _existing_manifest(root: Path, campaign: Mapping[str, Any]) -> tuple[dict[str, Any] | None, Path]:
    path = owner_campaign._owner_root(root, campaign) / "exact-manifest.json"
    if not path.is_file():
        return None, path
    return owner_campaign._validate_exact_manifest(
        root, campaign, owner_campaign._read_json(path, "exact manifest")
    ), path


def _existing_dedupe(path: Path) -> list[dict[str, Any]]:
    return owner_campaign._dedupe_records(path)


def _legacy_index_record(campaign: Mapping[str, Any], record: Mapping[str, Any]) -> dict[str, Any]:
    body = {
        "schema": LEGACY_INDEX_SCHEMA, "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "unit": campaign["unit"], "function": record["function"], "legacy_kind": record["kind"],
        "legacy_path_sha256": record["legacy_file_sha256"], "legacy_report_sha256": record["legacy_report_sha256"],
        "source_sha256": record["source_sha256"], "target_object_sha256": record["target_object_sha256"],
        "candidate_object_sha256": record["candidate_object_sha256"],
        "candidate_source_sha256": record.get("candidate_source_sha256", record["source_sha256"]),
        "status": "exact" if record["kind"] == "exact" else record["status"],
        "compiled": True, "imported_at": record["completed_at"],
    }
    return {**body, "legacy_index_sha256": _digest_json(body)}


def _dedupe_record(
    *, key: str, function: str, frontier: Mapping[str, Any],
    candidate_source_sha256: str, status: str, record: Mapping[str, Any],
    strict_delta: int = 0, data_delta: int = 0, physical_delta: int = 0,
) -> dict[str, Any]:
    """Build a deterministic v2 dedupe record from a legacy timestamp."""

    body = owner_campaign._dedupe_record(
        key=key, function=function, frontier=frontier,
        candidate_source_sha256=candidate_source_sha256, status=status,
        strict_delta=strict_delta, data_delta=data_delta,
        physical_delta=physical_delta,
    )
    unsigned = dict(body)
    unsigned.pop("result_sha256", None)
    unsigned["finished_at"] = record["completed_at"]
    return {**unsigned, "result_sha256": _digest_json(unsigned)}


def _publish(
    root: Path, campaign: Mapping[str, Any], exact_records: Sequence[Mapping[str, Any]],
    outcome_records: Sequence[Mapping[str, Any]],
) -> dict[str, Any]:
    owner_root = owner_campaign._owner_root(root, campaign)
    owner_root.mkdir(parents=True, exist_ok=True)
    exact_manifest, manifest_path = _existing_manifest(root, campaign)
    exact_map: dict[str, Any] = dict(exact_manifest["exact"] if exact_manifest else {})
    owner_closure = exact_manifest["owner_closure"] if exact_manifest else None
    tx = _Transaction()
    imported_exact: list[str] = []
    imported_outcomes: list[str] = []
    for record in exact_records:
        function = record["function"]
        focus, frontier, report, _proofs_value = _normalized_state(campaign, record)
        owner_campaign._validate_focus_evidence(focus, campaign, function, record["source_sha256"])
        owner_campaign._validate_frontier(frontier, campaign, function)
        owner_campaign._validate_exact_report(report, campaign, frontier)
        report_path = owner_campaign._state_root(root) / "proof-cas" / "reports" / report["report_sha256"][:2] / f"{report['report_sha256']}.json"
        focus_path = owner_campaign._state_root(root) / "proof-cas" / "focus" / focus["focus_evidence_sha256"][:2] / f"{focus['focus_evidence_sha256']}.json"
        function_root = owner_campaign._function_root(root, campaign, function)
        latest_path = function_root / "latest-frontier.json"
        tx.add(focus_path, _json_payload(focus))
        tx.add(report_path, _json_payload(report))
        tx.add(latest_path, _json_payload(frontier))
        entry = {
            "source_sha256": frontier["source_sha256"], "frontier_sha256": frontier["frontier_sha256"],
            "report_sha256": report["report_sha256"],
        }
        previous = exact_map.get(function)
        if previous is not None and previous != entry:
            raise LegacyImportError(f"exact manifest conflict for function: {function}")
        if previous is None:
            imported_exact.append(function)
            exact_map[function] = entry
        # Dedupe records make imported compiled exact cells idempotent without
        # pretending that an uncompiled legacy proposal was consumed.
        dedupe_path = function_root / "candidate-results.jsonl"
        current_dedupe = _existing_dedupe(dedupe_path)
        key = _candidate_key(campaign, record)
        if not any(item["candidate_key"] == key for item in current_dedupe):
            dedupe = _dedupe_record(
                key=key, function=function, frontier=frontier,
                candidate_source_sha256=record["source_sha256"], status="exact",
                record=record,
            )
            payload = b"".join(_json_payload(item) for item in [*current_dedupe, dedupe])
            tx.add(dedupe_path, payload)
        index_path = function_root / "legacy-imports.jsonl"
        index = _legacy_index_record(campaign, record)
        prior_index: list[dict[str, Any]] = []
        if index_path.is_file():
            for line in index_path.read_text(encoding="utf-8").splitlines():
                raw = json.loads(line)
                if set(raw) != LEGACY_INDEX_FIELDS or raw["legacy_index_sha256"] != _digest_json({k: v for k, v in raw.items() if k != "legacy_index_sha256"}):
                    raise LegacyImportError(f"legacy import index is corrupt: {index_path}")
                prior_index.append(raw)
        if not any(item["legacy_index_sha256"] == index["legacy_index_sha256"] for item in prior_index):
            tx.add(index_path, b"".join(_json_payload(item) for item in [*prior_index, index]))
        # Validate exact manifest entry conflicts before transaction commit.
    for record in outcome_records:
        function = record["function"]
        function_root = owner_campaign._function_root(root, campaign, function)
        dedupe_path = function_root / "candidate-results.jsonl"
        current_dedupe = _existing_dedupe(dedupe_path)
        key = _candidate_key(campaign, record)
        if not any(item["candidate_key"] == key for item in current_dedupe):
            # A neutral imported result has no frontier.  The stable synthetic
            # base still preserves the legacy compile identity in v2's ledger.
            base_frontier = {"frontier_sha256": _legacy_base_key(campaign, record)}
            dedupe = _dedupe_record(
                key=key, function=function, frontier=base_frontier,
                candidate_source_sha256=record["candidate_source_sha256"],
                status=record["status"], strict_delta=record["strict_difference_delta"],
                data_delta=record["data_difference_delta"], physical_delta=record["physical_difference_delta"],
                record=record,
            )
            tx.add(dedupe_path, b"".join(_json_payload(item) for item in [*current_dedupe, dedupe]))
            imported_outcomes.append(function)
        index_path = function_root / "legacy-imports.jsonl"
        index = _legacy_index_record(campaign, record)
        prior_index: list[dict[str, Any]] = []
        if index_path.is_file():
            for line in index_path.read_text(encoding="utf-8").splitlines():
                raw = json.loads(line)
                if set(raw) != LEGACY_INDEX_FIELDS or raw["legacy_index_sha256"] != _digest_json({k: v for k, v in raw.items() if k != "legacy_index_sha256"}):
                    raise LegacyImportError(f"legacy import index is corrupt: {index_path}")
                prior_index.append(raw)
        if not any(item["legacy_index_sha256"] == index["legacy_index_sha256"] for item in prior_index):
            tx.add(index_path, b"".join(_json_payload(item) for item in [*prior_index, index]))

    if exact_records:
        closes_owner = len(exact_map) == len(campaign["functions"])
        if closes_owner and owner_closure is None:
            # A legacy exact-function import cannot invent the final linked
            # owner proof.  50/51 seeding is valid; full closure is not.
            raise LegacyImportError("legacy exact import lacks final owner closure proof")
        body = {
            "schema": owner_campaign.EXACT_MANIFEST_SCHEMA, "campaign_id": campaign["campaign_id"],
            "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
            "exact": dict(sorted(exact_map.items())), "total": len(campaign["functions"]),
            "owner_closure": owner_closure, "updated_at": exact_manifest["updated_at"] if exact_manifest else exact_records[-1]["completed_at"],
        }
        manifest = {**body, "exact_manifest_sha256": _digest_json(body)}
        tx.add(manifest_path, _json_payload(manifest))
    if not tx._writes:
        return {
            "schema": LEGACY_IMPORT_SCHEMA, "status": "already_imported",
            "campaign_id": campaign["campaign_id"], "owner": campaign["owner"],
            "exact_imported": [], "outcome_imported": [], "exact_count": len(exact_map),
            "total": len(campaign["functions"]), "authority_advanced": False,
        }
    # All generated state is bounded by the same v2 state limits before any
    # path is replaced.
    owner_campaign._ensure_state_write_peak(root, campaign, tx._writes)
    timeout = float(campaign["limits"]["command_timeout_seconds"])
    with owner_campaign._exclusive_lock(owner_root / "source-cas.lock", timeout):
        tx.commit()
        try:
            if exact_records:
                owner_campaign._validate_exact_manifest(root, campaign, owner_campaign._read_json(manifest_path, "exact manifest"))
        except BaseException:
            tx.rollback()
            raise
    return {
        "schema": LEGACY_IMPORT_SCHEMA, "status": "imported",
        "campaign_id": campaign["campaign_id"], "owner": campaign["owner"],
        "exact_imported": sorted(set(imported_exact)), "outcome_imported": sorted(set(imported_outcomes)),
        "exact_count": len(exact_map), "total": len(campaign["functions"]),
        "authority_advanced": False,
    }


def import_legacy(
    root: Path | str, campaign_path: Path | str,
    legacy_paths: Sequence[Path | str] = (),
    *, consumed_paths: Sequence[Path | str] = (),
) -> dict[str, Any]:
    """Import old exact reports and compiled consumed outcomes once.

    ``legacy_paths`` are exact receipts.  ``consumed_paths`` are compiled
    non-exact outcomes and are written only to dedupe; they never produce an
    exact manifest or a frontier.  All inputs are validated before publication.
    """

    root_path = Path(os.path.abspath(root))
    campaign_file = owner_campaign._bound_path(root_path, str(campaign_path), "campaign manifest")
    campaign = owner_campaign.load_campaign(root_path, campaign_file)
    _campaign_clean(root_path, campaign)
    exact_records: list[dict[str, Any]] = []
    outcome_records: list[dict[str, Any]] = []
    seen_exact: set[tuple[str, str]] = set()
    seen_outcome: set[tuple[str, str]] = set()
    for raw in legacy_paths:
        record = _parse_receipt(_input_file(raw), campaign, outcome=False)
        _verify_record_campaign(campaign, record)
        identity = (record["function"], record["legacy_report_sha256"])
        if identity not in seen_exact:
            exact_records.append(record)
            seen_exact.add(identity)
    for raw in consumed_paths:
        record = _parse_receipt(_input_file(raw), campaign, outcome=True)
        _verify_record_campaign(campaign, {
            **record, "protected_total": len(campaign["protected_exact_functions"]),
            "protected_losses": 0, "source_link_exact": True,
        })
        identity = (record["function"], record["legacy_report_sha256"])
        if identity not in seen_outcome:
            outcome_records.append(record)
            seen_outcome.add(identity)
    return _publish(root_path, campaign, exact_records, outcome_records)


def import_legacy_receipts(
    root: Path | str, campaign_path: Path | str,
    legacy_paths: Sequence[Path | str] = (), *, consumed_paths: Sequence[Path | str] = (),
) -> dict[str, Any]:
    """Compatibility alias used by migration callers."""

    return import_legacy(root, campaign_path, legacy_paths, consumed_paths=consumed_paths)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    parser.add_argument("--campaign", "--manifest", required=True)
    parser.add_argument("--legacy-exact", "--exact", action="append", default=[])
    parser.add_argument("--legacy-consumed", "--consumed", action="append", default=[])
    args = parser.parse_args(argv)
    try:
        value = import_legacy(
            Path(args.root), Path(args.campaign), args.legacy_exact,
            consumed_paths=args.legacy_consumed,
        )
    except owner_campaign.CampaignError as exc:
        parser.error(str(exc))
    print(json.dumps(value, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":  # pragma: no cover
    raise SystemExit(main())
