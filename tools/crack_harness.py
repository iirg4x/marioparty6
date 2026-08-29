#!/usr/bin/env python3
"""Run one explicitly approved, bounded natural-C crack cell."""

from __future__ import annotations

import argparse
import difflib
import hashlib
import hmac
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
import threading
import time
from datetime import datetime, timedelta, timezone
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.recovery_pass import serialized_build_lock


APPROVAL_SCHEMA = "crack_harness_approval/v1"
WINNING_CELL_EVIDENCE_SCHEMA = "crack_winning_cell_evidence/v1"
RESULT_SCHEMA = "crack_harness_result/v1"
REPORT_SCHEMA = "CRACK_REPORT/v1"
PERMIT_SCHEMA = "crack_harness_resume_permit/v1"
DEFAULT_ACTIVE_SECONDS = 30 * 60
DEFAULT_STORAGE_BYTES = 512 * 1024 * 1024
DEFAULT_STATE_ROOT = Path("build/crack-harness")
MANAGER_PERMIT_KEY = Path(r"C:\Users\Anony\.codex\manager-secrets\mp6-crack-harness.key")
MANAGER_ISSUER = "mp6-crack-manager"
MANAGER_KEY_ID = "7a40e303f395fcfb25894819a19ad75488430e16a7d590aa1bc6370738b8591f"
MAX_CLOCK_SKEW_SECONDS = 30
TOOLCHAIN_MANIFEST_KEY = "b6764a1e5883ea1a096bfe4f8b888b93f1740f0f4046eb6149e0fe1d64cc6d90"
MAX_RETAINED_OWNER_BYTES = 16 * 1024 * 1024
MAX_RETAINED_GLOBAL_BYTES = 64 * 1024 * 1024
MAX_COMPACT_TERMINAL_BYTES = 1024 * 1024
EXACT_REPORT_FIELDS = {
    "schema", "status", "completed", "authority_advanced", "owner", "function",
    "task_id", "base_commit", "approval_sha256", "source_sha256",
    "target_object_sha256", "candidate_object_sha256", "result", "proof_receipts",
    "predicted_rows", "completed_at", "report_sha256",
}
EXACT_REPORT_RESULT_FIELDS = {
    "strict_percent", "data_percent", "target_bytes", "candidate_bytes", "owner_gain",
}
EXACT_REPORT_PROOF_RECEIPTS = {
    "precompile", "strict", "data", "focus", "siblings", "physical", "assess", "record",
}
EXACT_RESULT_RECEIPTS = EXACT_REPORT_PROOF_RECEIPTS | {"compile"}
COMMAND_RECEIPT_FIELDS = {
    "argv_sha256", "returncode", "active_seconds", "stdout_sha256", "stderr_sha256",
}
OBJDIFF_PIN = {
    "path": r"C:\Users\Anony\.codex\tools\objdiff\v3.8.0\objdiff-cli.exe",
    "version": "3.8.0", "size": 7161344,
    "sha256": "3023818f7fdd2f2dc6ade16e68d2c37f5f5754f96881d18d68ddfce77ced15e1",
}
HOOKS = ("strict", "data", "focus", "siblings", "physical")
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
_TEST_STATE_TOKEN = object()
REPARSE_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
CANONICAL_SCRIPTS = {
    "canonical_admission": "tools/candidate_compile_admission.py",
    "canonical_record": "tools/candidate_compile_admission.py",
    "compile": "tools/crack_evidence_bundle.py",
    "proof_strict": "tools/crack_harness.py",
    "proof_data": "tools/crack_harness.py",
    "proof_focus": "tools/crack_harness.py",
    "proof_siblings": "tools/crack_harness.py",
    "proof_physical": "tools/crack_harness.py",
    "assessment": "tools/crack_harness.py",
}


class CrackHarnessError(ValueError):
    """An approval, path, command, proof, or state violated the harness contract."""


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _digest_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _verify_objdiff_pin() -> None:
    path = Path(OBJDIFF_PIN["path"])
    if (
        not path.is_file() or path.stat().st_size != OBJDIFF_PIN["size"]
        or _digest_file(path) != OBJDIFF_PIN["sha256"]
    ):
        raise CrackHarnessError("central objdiff executable does not match the pinned manifest")


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _digest_json(value: Any) -> str:
    return _digest_bytes(_canonical(value))


def _approval_permit_identity(value: Mapping[str, Any]) -> str:
    """Cycle-free identity signed by the manager before permit_sha256 exists."""

    unsigned = dict(value)
    unsigned.pop("permit_sha256", None)
    return _digest_json(unsigned)


def _read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise CrackHarnessError(f"JSON file does not exist: {path}") from exc
    except json.JSONDecodeError as exc:
        raise CrackHarnessError(
            f"invalid JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc


def _atomic_json(path: Path, value: Any) -> None:
    _safe_mkdir(path.parent)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, prefix=path.name,
        suffix=".tmp", delete=False
    ) as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)


def _atomic_copy(source: Path, target: Path) -> None:
    _assert_no_indirection(source)
    _safe_mkdir(target.parent)
    _assert_no_indirection(target.parent)
    with source.open("rb") as incoming, tempfile.NamedTemporaryFile(
        "wb", dir=target.parent, prefix=target.name, suffix=".tmp", delete=False
    ) as outgoing:
        shutil.copyfileobj(incoming, outgoing, 1024 * 1024)
        outgoing.flush()
        os.fsync(outgoing.fileno())
        temporary = Path(outgoing.name)
    os.replace(temporary, target)


def _assert_no_indirection(path: Path, *, missing_leaf: bool = False) -> None:
    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for index, part in enumerate(absolute.parts[1:], 1):
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            if missing_leaf and index == len(absolute.parts) - 1:
                return
            raise CrackHarnessError(f"path component does not exist: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise CrackHarnessError(f"path indirection is forbidden: {current}")


def _safe_mkdir(path: Path) -> None:
    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for part in absolute.parts[1:]:
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            try:
                current.mkdir()
                continue
            except FileExistsError:
                info = current.lstat()
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise CrackHarnessError(f"path indirection is forbidden: {current}")
        if not stat.S_ISDIR(info.st_mode):
            raise CrackHarnessError(f"directory path is not a directory: {current}")


def _inside(root: Path, path: Path) -> bool:
    try:
        return os.path.normcase(os.path.commonpath((root, path))) == os.path.normcase(
            os.fspath(root)
        )
    except ValueError:
        return False


def _bound_path(root: Path, value: Any, label: str, *, exists: bool = True) -> Path:
    if not isinstance(value, str) or not value or "\x00" in value:
        raise CrackHarnessError(f"{label} must be a non-empty path")
    raw = Path(value).expanduser()
    path = Path(os.path.abspath(raw if raw.is_absolute() else root / raw))
    if not _inside(root, path):
        raise CrackHarnessError(f"{label} escapes repository root: {path}")
    _assert_no_indirection(path, missing_leaf=not exists)
    if exists and not path.is_file():
        raise CrackHarnessError(f"{label} is not a regular file: {path}")
    return path


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise CrackHarnessError(f"{label} must be non-empty text")
    return value.strip()


def _sha(value: Any, label: str) -> str:
    result = _text(value, label)
    if result != result.lower() or SHA_RE.fullmatch(result) is None:
        raise CrackHarnessError(f"{label} must be a lowercase SHA-256 digest")
    return result


def _validate_winning_cell_selection(
    root: Path, selection: Any, candidate_sha256: str,
    predicted_rows: Sequence[str], owner: str, function: str,
) -> None:
    """Require one evidence-backed winning cell before any compile is legal."""

    required = {
        "strategy", "rank", "evidence", "candidate_sha256",
        "predicted_rows_sha256", "alternatives_compiled", "negative_controls",
        "pivot_if_unranked", "source_class",
    }
    if not isinstance(selection, Mapping) or set(selection) != required:
        raise CrackHarnessError(
            "selection must be a strict closed winning-cell-first object"
        )
    if selection.get("strategy") != "winning_cell_first":
        raise CrackHarnessError("selection.strategy must be winning_cell_first")
    rank = selection.get("rank")
    if type(rank) is not int or rank != 1:
        raise CrackHarnessError("selection.rank must be exactly 1")
    evidence = selection.get("evidence")
    if not isinstance(evidence, Mapping) or set(evidence) != {"path", "sha256"}:
        raise CrackHarnessError("selection.evidence must bind one path and sha256")
    evidence_path = _bound_path(
        root, evidence.get("path"), "selection.evidence.path", exists=True
    )
    expected_evidence_sha256 = _sha(
        evidence.get("sha256"), "selection.evidence.sha256"
    )
    if _digest_file(evidence_path) != expected_evidence_sha256:
        raise CrackHarnessError(
            f"selection evidence hash mismatch: {evidence_path}"
        )
    selected_candidate = _sha(
        selection.get("candidate_sha256"), "selection.candidate_sha256"
    )
    if selected_candidate != candidate_sha256:
        raise CrackHarnessError(
            "selection.candidate_sha256 does not match approval candidate"
        )
    selected_rows = _sha(
        selection.get("predicted_rows_sha256"),
        "selection.predicted_rows_sha256",
    )
    if selected_rows != _digest_json(list(predicted_rows)):
        raise CrackHarnessError(
            "selection.predicted_rows_sha256 does not match predicted_rows"
        )
    for key in ("alternatives_compiled", "negative_controls"):
        if type(selection.get(key)) is not int or selection.get(key) != 0:
            raise CrackHarnessError(f"selection.{key} must be exactly 0")
    if selection.get("pivot_if_unranked") is not True:
        raise CrackHarnessError("selection.pivot_if_unranked must be true")
    source_class = _text(selection.get("source_class"), "selection.source_class")

    evidence_value = _read_json(evidence_path)
    evidence_required = {
        "schema", "owner", "function", "strategy", "rank",
        "candidate_sha256", "predicted_rows_sha256",
        "alternatives_compiled", "negative_controls", "pivot_if_unranked",
        "source_class", "inputs", "causal_prediction",
    }
    if not isinstance(evidence_value, Mapping) or set(evidence_value) != evidence_required:
        raise CrackHarnessError(
            "selection evidence must be a strict crack_winning_cell_evidence/v1 object"
        )
    evidence_bindings = {
        "schema": WINNING_CELL_EVIDENCE_SCHEMA,
        "owner": owner,
        "function": function,
        "strategy": "winning_cell_first",
        "rank": 1,
        "candidate_sha256": candidate_sha256,
        "predicted_rows_sha256": _digest_json(list(predicted_rows)),
        "alternatives_compiled": 0,
        "negative_controls": 0,
        "pivot_if_unranked": True,
        "source_class": source_class,
    }
    for key, expected in evidence_bindings.items():
        if evidence_value.get(key) != expected:
            raise CrackHarnessError(
                f"selection evidence does not bind approval {key}"
            )
    inputs = evidence_value.get("inputs")
    if (
        not isinstance(inputs, list) or not 1 <= len(inputs) <= 16
        or any(not isinstance(item, Mapping) for item in inputs)
    ):
        raise CrackHarnessError("selection evidence inputs must contain 1-16 files")
    seen_inputs: set[tuple[str, str]] = set()
    for index, item in enumerate(inputs):
        if set(item) != {"path", "sha256", "role"}:
            raise CrackHarnessError(
                f"selection evidence input {index} must bind path, sha256, and role"
            )
        input_path = _bound_path(
            root, item.get("path"), f"selection evidence input {index}.path"
        )
        input_sha256 = _sha(
            item.get("sha256"), f"selection evidence input {index}.sha256"
        )
        role = _text(item.get("role"), f"selection evidence input {index}.role")
        identity = (input_path.relative_to(root).as_posix(), role)
        if identity in seen_inputs:
            raise CrackHarnessError("selection evidence inputs contain a duplicate")
        seen_inputs.add(identity)
        if _digest_file(input_path) != input_sha256:
            raise CrackHarnessError(
                f"selection evidence input hash mismatch: {input_path}"
            )
    causal = evidence_value.get("causal_prediction")
    if (
        not isinstance(causal, Mapping)
        or set(causal) != {"earliest_divergence", "predicted_effect", "predicted_rows"}
    ):
        raise CrackHarnessError("selection evidence causal_prediction is not strict")
    _text(causal.get("earliest_divergence"), "causal_prediction.earliest_divergence")
    _text(causal.get("predicted_effect"), "causal_prediction.predicted_effect")
    if causal.get("predicted_rows") != list(predicted_rows):
        raise CrackHarnessError(
            "selection evidence causal_prediction does not bind predicted_rows"
        )


def _timestamp(value: Any, label: str) -> datetime:
    try:
        result = datetime.fromisoformat(_text(value, label).replace("Z", "+00:00"))
    except ValueError as exc:
        raise CrackHarnessError(f"{label} must be an ISO-8601 timestamp") from exc
    if result.tzinfo is None:
        raise CrackHarnessError(f"{label} must include a timezone")
    return result.astimezone(timezone.utc)


def _argv(value: Any, label: str) -> list[str]:
    if not isinstance(value, list) or not value or not all(
        isinstance(item, str) and item and "\x00" not in item for item in value
    ):
        raise CrackHarnessError(f"{label} must be a non-empty argv string array")
    return list(value)


def _command(root: Path, value: Any, label: str, kind: str) -> dict[str, Any]:
    if not isinstance(value, Mapping) or value.get("kind") != kind:
        raise CrackHarnessError(f"{label} must be a typed {kind} command descriptor")
    if set(value) != {"kind", "argv", "executable", "script"}:
        raise CrackHarnessError(f"{label} has an open or incomplete command descriptor")
    argv = _argv(value.get("argv"), f"{label}.argv")
    executable = value.get("executable")
    if not isinstance(executable, Mapping) or set(executable) != {"path", "sha256"}:
        raise CrackHarnessError(f"{label}.executable must bind path and sha256")
    controller_python = Path(sys.executable).resolve()
    controller_python_text = str(controller_python)
    if executable.get("path") != controller_python_text or argv[0] != controller_python_text:
        raise CrackHarnessError(
            f"{label} executable must be the exact controller interpreter {controller_python_text}"
        )
    controller_python_sha256 = _digest_file(controller_python)
    if executable.get("sha256") != controller_python_sha256:
        raise CrackHarnessError(f"{label} executable SHA-256 is not the controller interpreter pin")
    executable_path = Path(os.path.abspath(Path(str(executable["path"])).expanduser()))
    if not executable_path.is_file() or _digest_file(executable_path) != _sha(executable["sha256"], f"{label}.executable.sha256"):
        raise CrackHarnessError(f"{label} executable hash mismatch: {executable_path}")
    if Path(argv[0]).resolve() != executable_path.resolve():
        raise CrackHarnessError(f"{label}.argv[0] is not the pinned executable")
    script = value.get("script")
    script_path = None
    if script is not None:
        if not isinstance(script, Mapping) or set(script) != {"path", "sha256"}:
            raise CrackHarnessError(f"{label}.script must bind path and sha256")
        script_path = _bound_path(root, script.get("path"), f"{label}.script.path")
        if _digest_file(script_path) != _sha(script.get("sha256"), f"{label}.script.sha256"):
            raise CrackHarnessError(f"{label} script hash mismatch: {script_path}")
        argv_script = argv[1].replace("{CONTROLLER_ROOT}", str(root)) if len(argv) > 1 else ""
        if len(argv) < 2 or Path(argv_script).resolve() != script_path.resolve():
            raise CrackHarnessError(f"{label}.argv[1] is not the pinned script")
    canonical_script = (root / CANONICAL_SCRIPTS[kind]).resolve()
    if script_path is None or script_path.resolve() != canonical_script:
        raise CrackHarnessError(
            f"{label} must use canonical registered script {canonical_script}"
        )
    return {
        "kind": kind, "argv": argv, "executable": executable_path,
        "executable_sha256": _sha(executable["sha256"], f"{label}.executable.sha256"),
        "script": script_path,
        "script_sha256": (
            _sha(script["sha256"], f"{label}.script.sha256")
            if isinstance(script, Mapping) else None
        ),
    }


def _validate_command_receipt(value: Any, label: str) -> dict[str, Any]:
    """Validate the sealed result of one reviewed command invocation."""

    if not isinstance(value, Mapping) or set(value) != COMMAND_RECEIPT_FIELDS:
        raise CrackHarnessError(f"{label} is not a complete command receipt")
    for key in ("argv_sha256", "stdout_sha256", "stderr_sha256"):
        _sha(value.get(key), f"{label}.{key}")
    returncode = value.get("returncode")
    if type(returncode) is not int or returncode != 0:
        raise CrackHarnessError(f"{label}.returncode is not a successful exit")
    active_seconds = value.get("active_seconds")
    if isinstance(active_seconds, bool) or not isinstance(active_seconds, (int, float)):
        raise CrackHarnessError(f"{label}.active_seconds is not numeric")
    try:
        active_value = float(active_seconds)
    except OverflowError as exc:
        raise CrackHarnessError(f"{label}.active_seconds is not finite") from exc
    if not math.isfinite(active_value) or active_value < 0:
        raise CrackHarnessError(f"{label}.active_seconds is not finite")
    return dict(value)


def _limits(value: Any) -> dict[str, int]:
    if value is None:
        value = {}
    if not isinstance(value, Mapping):
        raise CrackHarnessError("limits must be an object")
    if set(value) - {"active_seconds", "temporary_bytes", "candidates"}:
        raise CrackHarnessError("limits is a strict closed object")
    active = value.get("active_seconds", DEFAULT_ACTIVE_SECONDS)
    storage = value.get("temporary_bytes", DEFAULT_STORAGE_BYTES)
    candidates = value.get("candidates", 1)
    for item, label in (
        (active, "limits.active_seconds"),
        (storage, "limits.temporary_bytes"),
        (candidates, "limits.candidates"),
    ):
        if isinstance(item, bool) or not isinstance(item, int) or item < 1:
            raise CrackHarnessError(f"{label} must be a positive integer")
    if candidates != 1:
        raise CrackHarnessError("one approval must authorize exactly one candidate")
    if active > DEFAULT_ACTIVE_SECONDS or storage > DEFAULT_STORAGE_BYTES:
        raise CrackHarnessError("hard time/storage limits are non-elevatable")
    return {"active_seconds": active, "temporary_bytes": storage, "candidates": 1}


def _validate_natural_cell(
    base: Path, candidate: Path, start: int, end: int,
    base_span_sha256: str | None = None,
) -> None:
    try:
        before = base.read_bytes()
        after = candidate.read_bytes()
        if b"\0" in before or b"\0" in after:
            raise CrackHarnessError("natural-C cell contains NUL")
        old_lines = before.decode("utf-8").splitlines(keepends=True)
        new_lines = after.decode("utf-8").splitlines(keepends=True)
    except UnicodeDecodeError as exc:
        raise CrackHarnessError("natural-C cell must be UTF-8") from exc
    if end > len(old_lines):
        raise CrackHarnessError("approved function span exceeds the sealed base")
    span_bytes = "".join(old_lines[start - 1:end]).encode("utf-8")
    if base_span_sha256 is not None and _digest_bytes(span_bytes) != base_span_sha256:
        raise CrackHarnessError("approved function span hash does not match the sealed base")
    changes = [opcode for opcode in difflib.SequenceMatcher(a=old_lines, b=new_lines, autojunk=False).get_opcodes() if opcode[0] != "equal"]
    if not changes or len(changes) > 3:
        raise CrackHarnessError("candidate must contain one bounded semantic cell of at most three hunks")
    changed_lines = 0
    changed_text_parts = []
    for _, old_start, old_end, new_start, new_end in changes:
        if old_start < start - 1 or old_end > end:
            raise CrackHarnessError("candidate changes lines outside the approved function span")
        changed_lines += (old_end - old_start) + (new_end - new_start)
        changed_text_parts.extend(new_lines[new_start:new_end])
    if changed_lines > 80:
        raise CrackHarnessError("natural-C cell exceeds the 80 changed-line budget")
    changed_text = "".join(changed_text_parts)
    forbidden = (
        r"\b(?:asm|__asm|volatile|register)\b",
        r"\b(?:padding|pad_bytes|dead_code)\b|\b_pad\w*",
        r"\bif\s*\(\s*(?:0|false)\s*\)",
        r"__attribute__|#\s*pragma\s+(?:inline|optimization)",
    )
    if any(re.search(pattern, changed_text, re.I) for pattern in forbidden):
        raise CrackHarnessError("candidate contains a forbidden shaping marker")


def _validate_admission_argv(root: Path, approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["precompile"]
    canonical_script = (root / "tools/candidate_compile_admission.py").resolve()
    if descriptor["script"] is None or descriptor["script"].resolve() != canonical_script:
        raise CrackHarnessError("canonical admission must use tools/candidate_compile_admission.py")
    expected = [
        str(descriptor["executable"]), str(canonical_script),
        "--root", str(root), "admit",
        "--owner", approval["owner"],
        "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--json",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("canonical admission argv is not the exact supported front-door shape")


def _validate_proof_adapter_argv(
    root: Path, approval: Mapping[str, Any], name: str,
) -> None:
    descriptor = approval["commands"][name]
    source_relative = approval["_paths"]["source"].relative_to(root).as_posix()
    expected = [
        str(descriptor["executable"]), "{CONTROLLER_ROOT}/tools/crack_harness.py",
        "proof-adapter", "--kind", name,
        "--owner", approval["owner"], "--function", approval["function"],
        "--candidate-source", f"{{RUN_ROOT}}/{source_relative}",
        "--candidate-source-sha256", approval["candidate"]["sha256"],
        "--approved-target-object-sha256", approval["target_sha256"],
        "--target-object", "{OUT_ROOT}/target.o",
        "--candidate-object", "{OUT_ROOT}/candidate.o",
        "--baseline-strict-report", "{OUT_ROOT}/baseline-strict.json",
        "--baseline-data-report", "{OUT_ROOT}/baseline-data.json",
        "--candidate-strict-report", "{OUT_ROOT}/candidate-strict.json",
        "--candidate-data-report", "{OUT_ROOT}/candidate-data.json",
        "--physical-receipt", "{OUT_ROOT}/physical.json",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError(
            f"commands.{name} argv is not the exact canonical proof-adapter shape"
        )


def _validate_record_descriptor(root: Path, approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["record"]
    expected = [
        str(descriptor["executable"]),
        str((root / "tools/candidate_compile_admission.py").resolve()),
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("commands.record must be the canonical record front door")


def _validate_compile_argv(approval: Mapping[str, Any]) -> None:
    descriptor = approval["commands"]["compile"]
    expected = [
        str(descriptor["executable"]), "{CONTROLLER_ROOT}/tools/crack_evidence_bundle.py",
        "--root", "{RUN_ROOT}", "--context", "{OUT_ROOT}/approval-context.json",
        "--out", "{OUT_ROOT}",
    ]
    if descriptor["argv"] != expected:
        raise CrackHarnessError("commands.compile argv is not the exact two-phase evidence-bundle front door")


def _expand_argv(
    argv: Sequence[str], run_root: Path, out_root: Path, controller_root: Path,
) -> list[str]:
    expanded = [
        item.replace("{RUN_ROOT}", str(run_root)).replace("{OUT_ROOT}", str(out_root)).replace(
            "{CONTROLLER_ROOT}", str(controller_root)
        )
        for item in argv
    ]
    if any("{" in item or "}" in item for item in expanded):
        raise CrackHarnessError("reviewed argv contains an unsupported placeholder")
    for index, item in enumerate(expanded[2:], start=2):
        candidate = Path(item)
        path_like = (
            candidate.is_absolute() or item.startswith(("./", "../", ".\\", "..\\"))
            or "/../" in item or "\\..\\" in item
        )
        if not path_like:
            continue
        if not candidate.is_absolute() or not (
            _inside(run_root, candidate) or _inside(out_root, candidate)
        ):
            raise CrackHarnessError(
                f"reviewed argv[{index}] path escapes the disposable writable roots"
            )
    return expanded


def _proof_adapter_payload(
    *, kind: str, owner: str, function: str, candidate_source: Path,
    candidate_source_sha256: str, approved_target_object_sha256: str,
    target_object: Path, candidate_object: Path,
    baseline_strict_report: Path, baseline_data_report: Path,
    candidate_strict_report: Path, candidate_data_report: Path,
    physical_receipt: Path,
) -> dict[str, Any]:
    """Derive one closed proof from canonical, hash-bound objdiff artifacts."""
    from tools import focus_symbol_report as focus_report

    source_sha = _digest_file(candidate_source)
    if source_sha != _sha(candidate_source_sha256, "candidate_source_sha256"):
        raise CrackHarnessError("proof adapter candidate source hash mismatch")
    target_sha = _digest_file(target_object)
    if target_sha != _sha(approved_target_object_sha256, "approved_target_object_sha256"):
        raise CrackHarnessError("proof adapter target object is not the approved target")
    candidate_sha = _digest_file(candidate_object)
    baseline = focus_report.build_from_paths(
        strict_report_path=baseline_strict_report,
        data_report_path=baseline_data_report,
        function=function,
        expected_strict_report_sha256=_digest_file(baseline_strict_report),
        expected_data_report_sha256=_digest_file(baseline_data_report),
    )
    candidate = focus_report.build_from_paths(
        strict_report_path=candidate_strict_report,
        data_report_path=candidate_data_report,
        function=function,
        expected_strict_report_sha256=_digest_file(candidate_strict_report),
        expected_data_report_sha256=_digest_file(candidate_data_report),
        physical_receipt_path=physical_receipt,
        expected_physical_receipt_sha256=_digest_file(physical_receipt),
        require_physical=False,
    )
    report_sha = _sha(candidate.get("artifact_sha256"), "focus artifact_sha256")
    common = {
        "owner": owner, "function": function,
        "candidate_source_sha256": source_sha,
        "target_object_sha256": target_sha,
        "candidate_object_sha256": candidate_sha,
        "report_sha256": report_sha,
    }
    channels = candidate["channels"]
    if kind in {"strict", "data"}:
        metric = channels[kind]["metric"]
        prefix = "strict" if kind == "strict" else "data"
        return {
            **common, "schema": f"crack_proof_{kind}/v1",
            f"{prefix}_percent": metric["match_percent"],
            "target_bytes": metric["target_size"],
            "candidate_bytes": metric["candidate_size"],
            "differences": metric["diff_rows"],
        }
    if kind == "focus":
        return {
            **common, "schema": "crack_proof_focus/v1",
            "differing_rows": channels["strict"]["metric"]["diff_rows"],
        }
    if kind == "siblings":
        before = {
            (channel, _digest_json(identity))
            for channel in ("strict", "data")
            for identity in baseline["channels"][channel]["protected_siblings"]["exact_identities"]
        }
        after = {
            (channel, _digest_json(identity))
            for channel in ("strict", "data")
            for identity in channels[channel]["protected_siblings"]["exact_identities"]
        }
        return {
            **common, "schema": "crack_proof_siblings/v1",
            "protected_total": len(before), "protected_losses": len(before - after),
        }
    if kind == "physical":
        physical = candidate["physical_relocations"]
        differences = physical.get("physical_relocation_differences", [])
        return {
            **common, "schema": "crack_proof_physical/v1",
            "target_count": physical.get("target", {}).get("physical_relocation_count", 0),
            "candidate_count": physical.get("candidate", {}).get("physical_relocation_count", 0),
            "differences": len(differences),
        }
    if kind == "assess":
        before = baseline["channels"]["strict"]["metric"]["match_percent"]
        after = channels["strict"]["metric"]["match_percent"]
        data_before = baseline["channels"]["data"]["metric"]["match_percent"]
        data_after = channels["data"]["metric"]["match_percent"]
        data_diff_before = baseline["channels"]["data"]["metric"]["diff_rows"]
        data_diff_after = channels["data"]["metric"]["diff_rows"]
        return {
            "schema": "crack_assessment/v1", "owner": owner,
            "function": function, "candidate_source_sha256": source_sha,
            "target_object_sha256": target_sha,
            "candidate_object_sha256": candidate_sha,
            "owner_gain": float(after) - float(before),
            "data_gain": float(data_after) - float(data_before),
            "data_diff_delta": int(data_diff_after) - int(data_diff_before),
        }
    raise CrackHarnessError(f"unsupported proof adapter kind: {kind}")


EVIDENCE_BASELINE_FILES = (
    "target.o", "baseline-candidate.o", "baseline-strict.json", "baseline-data.json",
    "baseline-receipt.json",
)
EVIDENCE_CANDIDATE_FILES = (
    "candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json",
    "candidate-receipt.json",
)


def _clear_evidence(out_root: Path, names: Sequence[str]) -> None:
    for name in names:
        path = out_root / name
        if path.exists():
            if not path.is_file() or path.is_symlink():
                raise CrackHarnessError(f"evidence output is not a plain file: {path}")
            path.unlink()


def _evidence_hashes(out_root: Path, names: Sequence[str]) -> dict[str, str]:
    result = {}
    for name in names:
        path = out_root / name
        if not path.is_file() or path.is_symlink():
            raise CrackHarnessError(f"evidence bundle omitted plain output: {name}")
        result[name] = _digest_file(path)
    return result


def _validate_evidence_descriptor(
    descriptor: Any, path: Path, label: str,
) -> None:
    if not isinstance(descriptor, Mapping) or set(descriptor) != {"sha256", "size_bytes"}:
        raise CrackHarnessError(f"{label} descriptor is invalid")
    if (
        _sha(descriptor.get("sha256"), f"{label}.sha256") != _digest_file(path)
        or not isinstance(descriptor.get("size_bytes"), int)
        or isinstance(descriptor.get("size_bytes"), bool)
        or descriptor["size_bytes"] != path.stat().st_size
    ):
        raise CrackHarnessError(f"{label} descriptor does not bind its artifact")


def _validate_evidence_receipt(
    out_root: Path, approval: Mapping[str, Any], context: Mapping[str, Any], phase: str,
) -> dict[str, Any]:
    path = out_root / f"{phase}-receipt.json"
    receipt = _read_json(path)
    if not isinstance(receipt, Mapping):
        raise CrackHarnessError(f"{phase} evidence receipt is invalid")
    unsigned = dict(receipt)
    digest = unsigned.pop("receipt_sha256", None)
    if digest != _digest_json(unsigned):
        raise CrackHarnessError(f"{phase} evidence receipt digest is invalid")
    expected = {
        "schema": "crack_evidence_phase_receipt/v1", "phase": phase,
        "owner": approval["owner"], "function": approval["function"],
        "unit": approval["unit"],
        "source_relpath": approval["_paths"]["source"].relative_to(approval["_root"]).as_posix(),
        "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "approval_context_sha256": context["context_sha256"],
        "phase_nonce": _digest_bytes((context["context_sha256"] + ":" + phase).encode()),
        "issued_at": approval["issued_at"], "authority_advanced": False,
    }
    for key, value in expected.items():
        if receipt.get(key) != value:
            raise CrackHarnessError(f"{phase} evidence receipt {key} is not approval-bound")
    artifacts = receipt.get("artifacts")
    required = (
        {"target.o", "baseline-candidate.o", "baseline-strict.json", "baseline-data.json"}
        if phase == "baseline" else
        {"target.o", "candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json"}
    )
    if not isinstance(artifacts, Mapping) or set(artifacts) != required:
        raise CrackHarnessError(f"{phase} evidence receipt artifact set is invalid")
    for name, descriptor in artifacts.items():
        _validate_evidence_descriptor(descriptor, out_root / name, f"{phase}.{name}")
    if not isinstance(receipt.get("tools"), Mapping) or not receipt["tools"]:
        raise CrackHarnessError(f"{phase} evidence receipt lacks pinned tool descriptors")
    return dict(receipt)


def _validate_evidence_context(
    out_root: Path, approval: Mapping[str, Any], context: Mapping[str, Any], *, completed: bool,
) -> dict[str, Any]:
    value = _read_json(out_root / "evidence-context.json")
    if not isinstance(value, Mapping):
        raise CrackHarnessError("evidence context is invalid")
    unsigned = dict(value)
    digest = unsigned.pop("evidence_context_sha256", None)
    if digest != _digest_json(unsigned):
        raise CrackHarnessError("evidence context digest is invalid")
    expected = {
        "schema": "crack_evidence_bundle_context/v1", "owner": approval["owner"],
        "function": approval["function"], "unit": approval["unit"],
        "source_relpath": approval["_paths"]["source"].relative_to(approval["_root"]).as_posix(),
        "target_sha256": approval["target_sha256"], "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "approval_context_sha256": context["context_sha256"], "authority_advanced": False,
        "phase_nonces": {
            phase: _digest_bytes((context["context_sha256"] + ":" + phase).encode())
            for phase in ("baseline", "candidate")
        },
    }
    for key, expected_value in expected.items():
        if value.get(key) != expected_value:
            raise CrackHarnessError(f"evidence context {key} is not approval-bound")
    _validate_evidence_descriptor(value.get("baseline_receipt"), out_root / "baseline-receipt.json", "baseline receipt")
    if completed:
        if value.get("completed") is not True:
            raise CrackHarnessError("candidate evidence context is not complete")
        _validate_evidence_descriptor(value.get("candidate_receipt"), out_root / "candidate-receipt.json", "candidate receipt")
    elif "candidate_receipt" in value or "completed" in value:
        raise CrackHarnessError("baseline evidence context claims candidate completion")
    return dict(value)


def _create_disposable_worktree(root: Path, destination: Path, base_commit: str) -> None:
    _safe_mkdir(destination.parent)
    completed = subprocess.run(
        ["git", "worktree", "add", "--detach", str(destination), base_commit],
        cwd=root, text=True, capture_output=True, check=False,
    )
    if completed.returncode != 0:
        raise CrackHarnessError(f"cannot create disposable worktree: {completed.stderr.strip()}")


def _remove_disposable_worktree(root: Path, destination: Path) -> None:
    completed = subprocess.run(
        ["git", "worktree", "remove", "--force", str(destination)],
        cwd=root, text=True, capture_output=True, check=False,
    )
    subprocess.run(
        ["git", "worktree", "prune"], cwd=root,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
    )
    if completed.returncode != 0 or destination.exists():
        raise CrackHarnessError(
            f"disposable worktree cleanup failed; global STOP remains: {destination}"
        )


def _scavenge_disposable_worktrees(root: Path, state: Path) -> None:
    for path in state.glob("owners/*/*/latest/temp/worktree"):
        if path.exists():
            _remove_disposable_worktree(root, path)
    for name in ("temp", "raw", "logs", "out"):
        for path in state.glob(f"owners/*/*/latest/{name}"):
            if path.exists():
                _assert_no_indirection(path)
                shutil.rmtree(path)
    attempt = state / "attempt.json"
    if attempt.is_file():
        value = _read_json(attempt)
        if not isinstance(value, Mapping):
            raise CrackHarnessError("abandoned attempt receipt is invalid")
        unsigned = dict(value); digest = unsigned.pop("attempt_sha256", None)
        required = {"schema", "run_dir", "source_path", "approval_path", "approval_sha256", "disposable_paths"}
        if set(unsigned) != required or unsigned.get("schema") != "crack_harness_attempt/v1" or digest != _digest_json(unsigned):
            raise CrackHarnessError("abandoned attempt receipt integrity failed")
        run_dir = Path(os.path.abspath(str(value["run_dir"])))
        run_parts = run_dir.relative_to(state).parts if _inside(state, run_dir) else ()
        if len(run_parts) != 4 or run_parts[0] != "owners" or run_parts[-1] != "latest":
            raise CrackHarnessError("abandoned attempt run path escapes exact latest state")
        source = _bound_path(root, value["source_path"], "abandoned approved source")
        if not _is_tracked(root, source):
            raise CrackHarnessError("abandoned approved source is not tracked")
        approval_path = _bound_path(root, value["approval_path"], "abandoned approval")
        if _digest_file(approval_path) != _sha(value["approval_sha256"], "abandoned approval_sha256"):
            raise CrackHarnessError("abandoned approval hash drifted")
        approval_value = _read_json(approval_path)
        if not isinstance(approval_value, Mapping):
            raise CrackHarnessError("abandoned approval is invalid")
        expected_paths = {
            _bound_path(root, approval_value[name]["path"], f"abandoned {name}")
            for name in ("base", "candidate")
        } | {approval_path}
        raw_paths = value["disposable_paths"]
        if not isinstance(raw_paths, list) or len(raw_paths) != 4:
            raise CrackHarnessError("abandoned disposable set is invalid")
        paths = {Path(os.path.abspath(str(raw))) for raw in raw_paths}
        permit_matches = [path for path in paths - expected_paths if path.is_file() and _digest_file(path) == approval_value.get("permit_sha256")]
        if len(permit_matches) != 1 or paths != expected_paths | set(permit_matches) or source in paths:
            raise CrackHarnessError("abandoned disposables do not match the approved exact set")
        for path in paths:
            if not _inside(root, path):
                raise CrackHarnessError("abandoned disposable path escapes repository")
            if path.exists():
                if _is_tracked(root, path):
                    raise CrackHarnessError(f"abandoned disposable is tracked: {path}")
                path.unlink()
        attempt.unlink()
def _finalize_cleanup_results(state: Path) -> None:
    """Seal successful startup cleanup without changing crack proof status."""

    for path in state.glob("owners/*/*/latest/result.json"):
        value = _read_json(path)
        if (
            not isinstance(value, Mapping)
            or value.get("status") not in {"exact", "improved"}
            or value.get("cleanup_status") not in {"pending", "cleanup_incomplete"}
        ):
            continue
        run_dir = path.parent
        if any((run_dir / name).exists() for name in ("temp", "raw", "logs", "out")):
            continue
        body = dict(value)
        body.pop("result_sha256", None)
        body["cleanup_status"] = "complete"
        errors = body.get("cleanup_errors")
        body["cleanup_errors"] = (
            [str(item)[:1000] for item in errors[:8]]
            if isinstance(errors, list) else []
        )
        _atomic_json(path, {**body, "result_sha256": _digest_json(body)})


def _retry_retention_maintenance(state: Path) -> list[str]:
    result_paths = list(state.glob("owners/*/*/latest/result.json"))
    protected = {path.parent.parent for path in result_paths}
    errors: list[str] = []
    for path in result_paths:
        try:
            _gc_owner(
                path.parent, MAX_RETAINED_OWNER_BYTES,
                protected={path.parent.parent},
            )
        except BaseException as exc:
            errors.append(f"owner retention maintenance: {exc}"[:1000])
    try:
        _gc_global(state, MAX_RETAINED_GLOBAL_BYTES, protected=protected)
    except BaseException as exc:
        errors.append(f"global retention maintenance: {exc}"[:1000])
    if errors:
        for path in result_paths:
            value = _read_json(path)
            if not isinstance(value, Mapping) or value.get("cleanup_status") not in {
                "pending", "cleanup_incomplete",
            }:
                continue
            body = dict(value)
            body.pop("result_sha256", None)
            prior = body.get("cleanup_errors")
            combined = list(prior) if isinstance(prior, list) else []
            body["cleanup_status"] = "cleanup_incomplete"
            body["cleanup_errors"] = [str(item)[:1000] for item in (combined + errors)[-8:]]
            _atomic_json(path, {**body, "result_sha256": _digest_json(body)})
    else:
        _finalize_cleanup_results(state)
    return errors


def load_approval(
    root: Path, path: Path, *, allow_applied_source: bool = False
) -> dict[str, Any]:
    root = root.resolve()
    approval_path = _bound_path(root, os.fspath(path), "approval")
    value = _read_json(approval_path)
    if not isinstance(value, Mapping) or value.get("schema") != APPROVAL_SCHEMA:
        raise CrackHarnessError(f"approval schema must be {APPROVAL_SCHEMA}")
    approval = dict(value)
    permit_identity_sha256 = _approval_permit_identity(value)
    commands_sha256 = _digest_json(value.get("commands"))
    allowed = {
        "schema", "approval_id", "owner", "task_id", "function",
        "unit", "base_commit", "toolchain_key", "target_sha256", "permit_sha256", "issued_at", "expires_at",
        "source", "base", "candidate", "function_span", "predicted_rows", "selection",
        "commands", "campaign", "limits",
    }
    unknown = sorted(set(approval) - allowed)
    if unknown:
        raise CrackHarnessError("approval contains unknown fields: " + ", ".join(unknown))
    approval["_permit_identity_sha256"] = permit_identity_sha256
    approval["_commands_sha256"] = commands_sha256
    approval_id = _text(approval.get("approval_id"), "approval_id")
    if re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", approval_id) is None:
        raise CrackHarnessError("approval_id contains unsafe characters")
    _text(approval.get("owner"), "owner")
    _text(approval.get("task_id"), "task_id")
    _text(approval.get("unit"), "unit")
    _text(approval.get("base_commit"), "base_commit")
    if _sha(approval.get("toolchain_key"), "toolchain_key") != TOOLCHAIN_MANIFEST_KEY:
        raise CrackHarnessError("toolchain_key is not the canonical Ninja-inclusive manifest")
    _sha(approval.get("target_sha256"), "target_sha256")
    _sha(approval.get("permit_sha256"), "permit_sha256")
    function = _text(approval.get("function"), "function")
    if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", function) is None:
        raise CrackHarnessError("function must be a C identifier")
    issued_at = _timestamp(approval.get("issued_at"), "issued_at")
    expires_at = _timestamp(approval.get("expires_at"), "expires_at")
    now = datetime.now(timezone.utc)
    if issued_at > now + timedelta(seconds=MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("approval issued_at is unacceptably in the future")
    if expires_at > now + timedelta(seconds=DEFAULT_ACTIVE_SECONDS + MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("approval expires_at exceeds the current hard attempt horizon")
    if expires_at <= now:
        raise CrackHarnessError("approval has expired")
    if (expires_at - issued_at).total_seconds() > DEFAULT_ACTIVE_SECONDS:
        raise CrackHarnessError("approval lifetime exceeds the hard 1800-second attempt window")
    paths: dict[str, Path] = {}
    for name in ("source", "base", "candidate"):
        if not isinstance(approval.get(name), Mapping) or set(approval[name]) != {"path", "sha256"}:
            raise CrackHarnessError(f"{name} must be a path/hash object")
    for name in ("source", "base", "candidate"):
        descriptor = approval.get(name)
        assert isinstance(descriptor, Mapping)
        item = _bound_path(root, descriptor.get("path"), f"{name}.path")
        expected = _sha(descriptor.get("sha256"), f"{name}.sha256")
        actual = _digest_file(item)
        applied_source = (
            name == "source"
            and allow_applied_source
            and actual == _sha(approval["candidate"].get("sha256"), "candidate.sha256")
        )
        if actual != expected and not applied_source:
            raise CrackHarnessError(f"{name} SHA-256 mismatch: {item}")
        paths[name] = item
        if name == "source":
            approval["_source_applied"] = applied_source
    if approval["source"]["sha256"] != approval["base"]["sha256"]:
        raise CrackHarnessError("source and sealed base must start byte-identical")
    if paths["source"] == paths["candidate"] or paths["base"] == paths["candidate"]:
        raise CrackHarnessError("candidate must be a separate sealed natural-C cell")
    if approval["candidate"]["sha256"] == approval["base"]["sha256"]:
        raise CrackHarnessError("candidate does not differ from the sealed base")
    approval["_paths"] = paths
    approval["_root"] = root
    span = approval.get("function_span")
    if not isinstance(span, Mapping) or set(span) != {"start_line", "end_line", "base_span_sha256"}:
        raise CrackHarnessError("function_span must bind lines and base_span_sha256")
    start, end = span.get("start_line"), span.get("end_line")
    if not all(isinstance(item, int) and not isinstance(item, bool) and item > 0 for item in (start, end)) or start > end:
        raise CrackHarnessError("function_span is invalid")
    span_sha = _sha(span.get("base_span_sha256"), "function_span.base_span_sha256")
    _validate_natural_cell(paths["base"], paths["candidate"], start, end, span_sha)
    rows = approval.get("predicted_rows")
    if not isinstance(rows, list) or not rows or not all(
        isinstance(row, str) and row.strip() for row in rows
    ):
        raise CrackHarnessError("predicted_rows must be a non-empty string array")
    _validate_winning_cell_selection(
        root, approval.get("selection"), approval["candidate"]["sha256"], rows,
        approval["owner"], approval["function"],
    )
    commands = approval.get("commands")
    if not isinstance(commands, Mapping):
        raise CrackHarnessError("commands must be an object")
    required = ("precompile", "compile", *HOOKS, "assess", "record")
    kinds = {"precompile": "canonical_admission", "compile": "compile", "assess": "assessment", "record": "canonical_record"}
    kinds.update({name: f"proof_{name}" for name in HOOKS})
    approval["commands"] = {name: _command(root, commands.get(name), f"commands.{name}", kinds[name]) for name in required}
    _validate_admission_argv(root, approval)
    _validate_compile_argv(approval)
    for name in (*HOOKS, "assess"):
        _validate_proof_adapter_argv(root, approval, name)
    _validate_record_descriptor(root, approval)
    for name in required:
        if name in {"precompile", "record"}:
            continue
        joined = "\0".join(approval["commands"][name]["argv"])
        if "{RUN_ROOT}" not in joined or "{OUT_ROOT}" not in joined:
            raise CrackHarnessError(f"commands.{name} must use RUN_ROOT and OUT_ROOT placeholders")
        if str(root) in joined:
            raise CrackHarnessError(f"commands.{name} embeds a production writable path")
    approval["limits"] = _limits(approval.get("limits"))
    campaign = approval.get("campaign")
    if not isinstance(campaign, Mapping) or set(campaign) != {"id", "quota"}:
        raise CrackHarnessError("campaign must be a closed id/quota object")
    campaign_id = _text(campaign.get("id"), "campaign.id")
    if re.fullmatch(r"[A-Za-z0-9_.-]{1,96}", campaign_id) is None:
        raise CrackHarnessError("campaign.id contains unsafe characters")
    quota = campaign.get("quota", 1)
    if quota != 1:
        raise CrackHarnessError("function campaign quota is hard-fixed at one")
    approval["_approval_path"] = approval_path
    approval["_approval_sha256"] = _digest_file(approval_path)
    approval["_paths"] = paths
    return approval


def _state_root(
    root: Path, value: str | Path | None = None, *, _test_token: object | None = None,
) -> Path:
    if value is not None and _test_token is not _TEST_STATE_TOKEN:
        raise CrackHarnessError("production harness state root is fixed and cannot be overridden")
    raw = Path(value or DEFAULT_STATE_ROOT).expanduser()
    path = Path(os.path.abspath(raw if raw.is_absolute() else root / raw))
    if not _inside(root, path):
        raise CrackHarnessError(f"state root escapes repository root: {path}")
    _assert_no_indirection(root)
    if path.exists():
        _assert_no_indirection(path)
    return path


def _load_permit(
    root: Path, approval: Mapping[str, Any], permit_path: Path, state: Path,
    *, manager_key_path: Path = MANAGER_PERMIT_KEY,
    expected_key_id: str = MANAGER_KEY_ID,
) -> tuple[dict[str, Any], Path]:
    path = _bound_path(root, os.fspath(permit_path), "manager resume permit")
    value = _read_json(path)
    required = {
        "schema", "permit_id", "issuer", "resume",
        "owner", "task_id", "function", "campaign_id", "stop_nonce",
        "approval_id", "approval_identity_sha256", "commands_sha256",
        "source_relpath", "source_sha256", "base_sha256", "candidate_sha256",
        "base_commit", "toolchain_key", "target_sha256",
        "issued_at", "deadline", "key_id", "signature",
    }
    if not isinstance(value, Mapping) or set(value) != required:
        raise CrackHarnessError("resume permit is not a strict closed object")
    permit = dict(value)
    if permit.get("schema") != PERMIT_SCHEMA or permit.get("resume") is not True:
        raise CrackHarnessError("resume permit is not manager-issued resume authority")
    _text(permit.get("permit_id"), "permit_id")
    if permit.get("issuer") != MANAGER_ISSUER:
        raise CrackHarnessError("resume permit issuer is not the fixed manager authority")
    key_path = manager_key_path.resolve()
    if _inside(root, key_path) or not key_path.is_file() or key_path.is_symlink():
        raise CrackHarnessError("manager permit key must be a plain file outside the repository")
    secret = key_path.read_bytes()
    if len(secret) != 32:
        raise CrackHarnessError("manager permit key must contain exactly 32 raw bytes")
    key_id = _digest_bytes(secret)
    if key_id != expected_key_id:
        raise CrackHarnessError("manager permit key file does not match the fixed key ID")
    if permit.get("key_id") != key_id:
        raise CrackHarnessError("resume permit key_id does not bind the manager key")
    unsigned = dict(permit)
    signature = unsigned.pop("signature", None)
    expected_signature = hmac.new(secret, _canonical(unsigned), hashlib.sha256).hexdigest()
    if not isinstance(signature, str) or not hmac.compare_digest(signature, expected_signature):
        raise CrackHarnessError("resume permit signature is invalid")
    if _digest_file(path) != approval["permit_sha256"]:
        raise CrackHarnessError("cell approval does not bind this exact assignment permit")
    for key, expected in (("owner", approval["owner"]), ("task_id", approval["task_id"]), ("function", approval["function"]), ("campaign_id", approval["campaign"]["id"])):
        if permit.get(key) != expected:
            raise CrackHarnessError(f"assignment permit does not bind {key}")
    exact_bindings = {
        "approval_id": approval["approval_id"],
        "approval_identity_sha256": approval["_permit_identity_sha256"],
        "commands_sha256": approval["_commands_sha256"],
        "source_relpath": approval["_paths"]["source"].relative_to(root).as_posix(),
        "source_sha256": approval["source"]["sha256"],
        "base_sha256": approval["base"]["sha256"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "base_commit": approval["base_commit"],
        "toolchain_key": approval["toolchain_key"],
        "target_sha256": approval["target_sha256"],
    }
    for key, expected in exact_bindings.items():
        if permit.get(key) != expected:
            raise CrackHarnessError(f"assignment permit does not bind exact {key}")
    permit_issued = _timestamp(permit.get("issued_at"), "permit issued_at")
    permit_expires = _timestamp(permit.get("deadline"), "permit deadline")
    now = datetime.now(timezone.utc)
    if permit_issued > now + timedelta(seconds=MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("resume permit issued_at is unacceptably in the future")
    if permit_expires > now + timedelta(seconds=DEFAULT_ACTIVE_SECONDS + MAX_CLOCK_SKEW_SECONDS):
        raise CrackHarnessError("resume permit deadline exceeds the current hard attempt horizon")
    if permit_expires <= now:
        raise CrackHarnessError("resume permit has expired")
    if (permit_expires - permit_issued).total_seconds() > DEFAULT_ACTIVE_SECONDS:
        raise CrackHarnessError("resume permit exceeds the hard 1800-second attempt window")
    approval_expires = _timestamp(approval.get("expires_at"), "approval expires_at")
    if permit_expires > approval_expires:
        raise CrackHarnessError("resume permit deadline outlives the bound approval")
    _verify_stop(state, path, permit)
    permit["_permit_sha256"] = _digest_file(path)
    return permit, path


def _verify_stop(state: Path, permit_path: Path, permit: Mapping[str, Any]) -> None:
    stop = state / "STOP"
    stop_value = _read_json(stop)
    expected_stop = {
        "schema": "crack_harness_stop/v1", "stopped": True,
        "authorized_permit_sha256": _digest_file(permit_path),
        "stop_nonce": _sha(permit.get("stop_nonce"), "stop_nonce"),
    }
    if not isinstance(stop_value, Mapping) or dict(stop_value) != expected_stop:
        raise CrackHarnessError("global STOP does not authenticate this exact manager assignment permit")


def _tree_size(path: Path) -> int:
    total = 0
    if not path.exists():
        return 0
    for parent, dirs, files in os.walk(path, followlinks=False):
        base = Path(parent)
        for name in dirs:
            item = base / name
            info = item.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE):
                raise CrackHarnessError(f"temporary path indirection is forbidden: {item}")
        for name in files:
            item = base / name
            info = item.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE):
                raise CrackHarnessError(f"temporary path indirection is forbidden: {item}")
            total += info.st_size
    return total


def _git(root: Path, *args: str) -> str:
    completed = subprocess.run(
        ["git", *args], cwd=root, text=True, capture_output=True, check=False
    )
    if completed.returncode != 0:
        raise CrackHarnessError(f"git {' '.join(args)} failed: {completed.stderr.strip()}")
    return completed.stdout.strip()


def _is_tracked(root: Path, path: Path) -> bool:
    completed = subprocess.run(
        ["git", "ls-files", "--error-unmatch", "--", str(path.relative_to(root))],
        cwd=root, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
    )
    return completed.returncode == 0


def _verify_repository(root: Path, approval: Mapping[str, Any], *, allow_source: bool) -> None:
    _assert_no_indirection(root)
    _assert_no_indirection(root / ".git")
    common_raw = Path(_git(root, "rev-parse", "--git-common-dir"))
    common = Path(os.path.abspath(common_raw if common_raw.is_absolute() else root / common_raw))
    _assert_no_indirection(common)
    if _git(root, "rev-parse", "HEAD") != approval["base_commit"]:
        raise CrackHarnessError("Git HEAD does not equal approved base_commit")
    if not _is_tracked(root, approval["_paths"]["source"]):
        raise CrackHarnessError("approved live source is not tracked at base_commit")
    dirty = []
    for line in _git(root, "status", "--porcelain=v1", "--untracked-files=no").splitlines():
        relative = (line[3:] if len(line) > 2 and line[2] == " " else line[2:]).strip().replace("\\", "/")
        source_relative = approval["_paths"]["source"].relative_to(root).as_posix()
        if not allow_source or relative != source_relative:
            dirty.append(relative)
    if dirty:
        raise CrackHarnessError("unapproved tracked writes: " + ", ".join(dirty))


def _checkpoint(
    root: Path, approval_path: Path, approval: Mapping[str, Any],
    permit_path: Path, permit: Mapping[str, Any], state: Path, *, allow_source: bool,
) -> None:
    _validate_winning_cell_selection(
        root, approval.get("selection"), approval["candidate"]["sha256"],
        approval["predicted_rows"], approval["owner"], approval["function"],
    )
    now = datetime.now(timezone.utc)
    approval_expires = _timestamp(approval.get("expires_at"), "approval expires_at")
    permit_expires = _timestamp(permit.get("deadline"), "permit deadline")
    if permit_expires > approval_expires:
        raise CrackHarnessError("resume permit deadline outlives the bound approval")
    if now >= approval_expires or now >= permit_expires:
        raise CrackHarnessError("bound approval/permit window expired during transaction")
    if _digest_file(approval_path) != approval["_approval_sha256"]:
        raise CrackHarnessError("approval changed during transaction")
    if _digest_file(permit_path) != permit["_permit_sha256"]:
        raise CrackHarnessError("resume permit changed during transaction")
    _verify_stop(state, permit_path, permit)
    _verify_objdiff_pin()
    for name in ("base", "candidate"):
        if _digest_file(approval["_paths"][name]) != approval[name]["sha256"]:
            raise CrackHarnessError(f"{name} changed during transaction")
    for name, descriptor in approval["commands"].items():
        if _digest_file(descriptor["executable"]) != descriptor["executable_sha256"]:
            raise CrackHarnessError(f"{name} executable changed during transaction")
        if descriptor["script"] is not None and _digest_file(descriptor["script"]) != descriptor["script_sha256"]:
            raise CrackHarnessError(f"{name} script changed during transaction")
    expected_source = approval["candidate" if allow_source else "source"]["sha256"]
    if _digest_file(approval["_paths"]["source"]) != expected_source:
        raise CrackHarnessError("live source changed outside the approved cell")
    _verify_repository(root, approval, allow_source=allow_source)


def _record_commit_value(path: Path) -> Mapping[str, Any] | None:
    if not path.is_file():
        return None
    try:
        return _validate_record_commit(_read_json(path), "record commit")
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return None


def _validate_record_commit(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, Mapping):
        raise CrackHarnessError(f"{label} is not a typed object")
    body = dict(value)
    digest = body.pop("commit_sha256", None)
    required = {
        "schema", "outcome", "candidate_sha256", "record_payload_sha256",
        "record_sha256",
    }
    if (
        set(body) != required
        or body.get("schema") != "crack_harness_record_commit/v1"
        or body.get("outcome") not in {"improved", "exact"}
        or digest != _digest_json(body)
    ):
        raise CrackHarnessError(f"{label} digest or schema is invalid")
    _sha(body.get("candidate_sha256"), f"{label}.candidate_sha256")
    _sha(body.get("record_payload_sha256"), f"{label}.record_payload_sha256")
    _sha(body.get("record_sha256"), f"{label}.record_sha256")
    return dict(value)


def _report_int(value: Any, label: str) -> int:
    if type(value) is not int or value < 0:
        raise CrackHarnessError(f"{label} must be a non-negative integer")
    return value


def _report_signed_int(value: Any, label: str) -> int:
    if type(value) is not int:
        raise CrackHarnessError(f"{label} must be an integer")
    return value


def _report_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise CrackHarnessError(f"{label} must be numeric")
    try:
        result = float(value)
    except OverflowError as exc:
        raise CrackHarnessError(f"{label} must be finite") from exc
    if not math.isfinite(result):
        raise CrackHarnessError(f"{label} must be finite")
    return result


def _validate_exact_report(
    report: Mapping[str, Any], result: Mapping[str, Any], binding: Mapping[str, Any],
    record_commit: Mapping[str, Any] | None = None,
) -> None:
    """Validate the complete compact CRACK_REPORT used for exact recovery.

    This is deliberately shared by report sealing and interrupted-transaction
    recovery.  A self-digest authenticates bytes, not their meaning, so every
    field that makes an exact result retainable is checked here as well.
    """

    if set(report) != EXACT_REPORT_FIELDS or report.get("schema") != REPORT_SCHEMA:
        raise CrackHarnessError("exact report is not the canonical CRACK_REPORT/v1 schema")
    report_digest = _sha(report.get("report_sha256"), "report_sha256")
    report_body = dict(report)
    report_body.pop("report_sha256")
    if report_digest != _digest_json(report_body):
        raise CrackHarnessError("exact report digest is invalid")
    if report.get("status") != "exact" or report.get("completed") is not True:
        raise CrackHarnessError("exact report is not a completed exact result")
    if report.get("authority_advanced") is not False:
        raise CrackHarnessError("exact report must preserve authority_advanced=false")

    owner = _text(report.get("owner"), "report owner")
    function = _text(report.get("function"), "report function")
    task_id = _text(report.get("task_id"), "report task_id")
    base_commit = _text(report.get("base_commit"), "report base_commit")
    approval_sha256 = _sha(report.get("approval_sha256"), "report approval_sha256")
    source_sha256 = _sha(report.get("source_sha256"), "report source_sha256")
    target_object_sha256 = _sha(
        report.get("target_object_sha256"), "report target_object_sha256"
    )
    candidate_object_sha256 = _sha(
        report.get("candidate_object_sha256"), "report candidate_object_sha256"
    )
    completed_at = _timestamp(report.get("completed_at"), "report completed_at")

    if not isinstance(binding, Mapping) or binding.get("status") != "exact":
        raise CrackHarnessError("exact report central binding is missing or non-exact")
    bound_target_object_sha256 = _sha(
        binding.get("target_object_sha256"), "binding.target_object_sha256"
    )
    if (
        owner != binding.get("owner")
        or function != binding.get("function")
        or source_sha256 != binding.get("source_sha256")
        or target_object_sha256 != bound_target_object_sha256
        or candidate_object_sha256 != binding.get("object_sha256")
    ):
        raise CrackHarnessError("exact report does not bind the central owner/function/source/object")

    result_fields = {
        "schema", "approval_id", "approval_sha256", "owner", "task_id", "function",
        "base_commit", "campaign_id", "candidate_sha256", "base_sha256", "status",
        "reason", "owner_gain", "predicted_rows", "receipts", "finished_at",
        "source_restored", "cleanup_status", "cleanup_errors", "authority_advanced",
        "result_sha256", "report_sha256",
    }
    if set(result) != result_fields or result.get("schema") != RESULT_SCHEMA:
        raise CrackHarnessError("exact terminal result is not the canonical result schema")
    result_digest = _sha(result.get("result_sha256"), "result_sha256")
    result_body = dict(result)
    result_body.pop("result_sha256")
    if result_digest != _digest_json(result_body):
        raise CrackHarnessError("exact terminal result digest is invalid")
    if (
        result.get("status") != "exact"
        or result.get("authority_advanced") is not False
        or result.get("source_restored") is not False
        or result.get("report_sha256") != report_digest
        or result.get("owner") != owner
        or result.get("function") != function
        or result.get("task_id") != task_id
        or result.get("base_commit") != base_commit
        or result.get("approval_sha256") != approval_sha256
        or result.get("candidate_sha256") != source_sha256
    ):
        raise CrackHarnessError("exact terminal result does not bind the exact report")
    if _timestamp(result.get("finished_at"), "result finished_at") != completed_at:
        raise CrackHarnessError("exact report completed_at does not match result finished_at")
    if not isinstance(result.get("reason"), str) or not result["reason"]:
        raise CrackHarnessError("exact terminal result reason is missing")
    if result.get("cleanup_status") not in {"pending", "complete", "cleanup_incomplete"}:
        raise CrackHarnessError("exact terminal cleanup status is invalid")
    cleanup_errors = result.get("cleanup_errors")
    if (
        not isinstance(cleanup_errors, list) or len(cleanup_errors) > 8
        or any(not isinstance(item, str) or len(item) > 1000 for item in cleanup_errors)
    ):
        raise CrackHarnessError("exact terminal cleanup errors are invalid")

    predicted_rows = report.get("predicted_rows")
    if (
        not isinstance(predicted_rows, list) or not predicted_rows
        or any(not isinstance(item, str) for item in predicted_rows)
        or predicted_rows != result.get("predicted_rows")
    ):
        raise CrackHarnessError("exact report predicted_rows are not result-bound")

    report_result = report.get("result")
    if (
        not isinstance(report_result, Mapping)
        or set(report_result) != EXACT_REPORT_RESULT_FIELDS
    ):
        raise CrackHarnessError("exact report result is incomplete")
    report_strict = _report_number(report_result.get("strict_percent"), "report strict_percent")
    report_data = _report_number(report_result.get("data_percent"), "report data_percent")
    report_target_bytes = _report_int(report_result.get("target_bytes"), "report target_bytes")
    report_candidate_bytes = _report_int(report_result.get("candidate_bytes"), "report candidate_bytes")
    report_gain = _report_number(report_result.get("owner_gain"), "report owner_gain")
    if report_strict != 100 or report_data != 100 or report_target_bytes != report_candidate_bytes or report_gain <= 0:
        raise CrackHarnessError("exact report result does not prove exactness")

    receipts = result.get("receipts")
    if not isinstance(receipts, Mapping):
        raise CrackHarnessError("exact terminal receipts are missing")
    receipt_names = set(receipts)
    if not EXACT_RESULT_RECEIPTS <= receipt_names or receipt_names - (EXACT_RESULT_RECEIPTS | {"secondary_failures"}):
        raise CrackHarnessError("exact terminal is missing a required receipt")
    proof_receipts = report.get("proof_receipts")
    if not isinstance(proof_receipts, Mapping) or set(proof_receipts) != EXACT_REPORT_PROOF_RECEIPTS:
        raise CrackHarnessError("exact report is missing a required proof receipt")

    compact_summaries: dict[str, Mapping[str, Any]] = {}
    for name in EXACT_REPORT_PROOF_RECEIPTS:
        receipt = receipts.get(name)
        if not isinstance(receipt, Mapping) or set(receipt) != {
            "schema", "hook", "ok", "summary", "payload_sha256", "command",
        }:
            raise CrackHarnessError(f"exact {name} receipt is not a compact typed receipt")
        if (
            receipt.get("schema") != "crack_harness_receipt/v1"
            or receipt.get("hook") != name
            or receipt.get("ok") is not True
        ):
            raise CrackHarnessError(f"exact {name} receipt has the wrong schema or hook")
        payload_sha256 = _sha(receipt.get("payload_sha256"), f"{name}.payload_sha256")
        summary = receipt.get("summary")
        _validate_command_receipt(receipt.get("command"), f"{name}.command")
        if not isinstance(summary, Mapping):
            raise CrackHarnessError(f"exact {name} receipt summary is invalid")
        entry = proof_receipts.get(name)
        if (
            not isinstance(entry, Mapping) or set(entry) != {"sha256", "summary"}
            or _sha(entry.get("sha256"), f"report proof {name}.sha256") != payload_sha256
            or entry.get("summary") != summary
        ):
            raise CrackHarnessError(f"report proof receipt {name} does not bind terminal receipt")
        compact_summaries[name] = summary

    compile_receipt = receipts.get("compile")
    if (
        not isinstance(compile_receipt, Mapping)
        or set(compile_receipt) != {"schema", "hook", "baseline_command", "candidate_command"}
        or compile_receipt.get("schema") != "crack_harness_receipt/v1"
        or compile_receipt.get("hook") != "compile"
        or not isinstance(compile_receipt.get("baseline_command"), Mapping)
        or not isinstance(compile_receipt.get("candidate_command"), Mapping)
    ):
        raise CrackHarnessError("exact compile receipt is incomplete")
    _validate_command_receipt(
        compile_receipt["baseline_command"], "compile.baseline_command"
    )
    _validate_command_receipt(
        compile_receipt["candidate_command"], "compile.candidate_command"
    )

    precompile = compact_summaries["precompile"]
    if set(precompile) != {
        "status", "reused", "skip_compile", "input_key", "admission_token",
        "expires_at", "authority_advanced",
    }:
        raise CrackHarnessError("exact precompile receipt is incomplete")
    if (
        precompile.get("status") != "admitted"
        or type(precompile.get("reused")) is not bool
        or precompile.get("skip_compile") is not False
        or precompile.get("authority_advanced") is not False
    ):
        raise CrackHarnessError("exact precompile receipt is not an admitted non-authoritative result")
    admission_input_key = _sha(precompile.get("input_key"), "precompile input_key")
    admission_token = _text(precompile.get("admission_token"), "precompile admission_token")
    _timestamp(precompile.get("expires_at"), "precompile expires_at")

    proof_common = {
        "schema", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "report_sha256",
    }
    object_pair: tuple[str, str] | None = None
    proof_artifact_sha: str | None = None
    for name in ("strict", "data", "focus", "siblings", "physical"):
        summary = compact_summaries[name]
        extras = {
            "strict": {"strict_percent", "target_bytes", "candidate_bytes", "differences"},
            "data": {"data_percent", "target_bytes", "candidate_bytes", "differences"},
            "focus": {"differing_rows"},
            "siblings": {"protected_total", "protected_losses"},
            "physical": {"target_count", "candidate_count", "differences"},
        }[name]
        if set(summary) != proof_common | extras:
            raise CrackHarnessError(f"exact {name} proof summary is incomplete")
        if (
            summary.get("owner") != owner
            or summary.get("function") != function
            or summary.get("candidate_source_sha256") != source_sha256
        ):
            raise CrackHarnessError(f"exact {name} proof summary is not source-bound")
        target = _sha(summary.get("target_object_sha256"), f"{name}.target_object_sha256")
        candidate = _sha(summary.get("candidate_object_sha256"), f"{name}.candidate_object_sha256")
        artifact = _sha(summary.get("report_sha256"), f"{name}.report_sha256")
        if object_pair is None:
            object_pair = (target, candidate)
            proof_artifact_sha = artifact
        elif object_pair != (target, candidate) or proof_artifact_sha != artifact:
            raise CrackHarnessError("exact proof summaries disagree on object/artifact identity")
        if name == "strict":
            if (
                _report_number(summary.get("strict_percent"), "strict_percent") != 100
                or _report_int(summary.get("target_bytes"), "strict.target_bytes")
                != _report_int(summary.get("candidate_bytes"), "strict.candidate_bytes")
                or _report_int(summary.get("differences"), "strict.differences") != 0
            ):
                raise CrackHarnessError("strict proof summary is not exact")
        elif name == "data":
            if (
                _report_number(summary.get("data_percent"), "data_percent") != 100
                or _report_int(summary.get("target_bytes"), "data.target_bytes")
                != _report_int(summary.get("candidate_bytes"), "data.candidate_bytes")
                or _report_int(summary.get("differences"), "data.differences") != 0
            ):
                raise CrackHarnessError("data proof summary is not exact")
        elif name == "focus":
            if _report_int(summary.get("differing_rows"), "focus.differing_rows") != 0:
                raise CrackHarnessError("focus proof summary is not exact")
        elif name == "siblings":
            _report_int(summary.get("protected_total"), "siblings.protected_total")
            if _report_int(summary.get("protected_losses"), "siblings.protected_losses") != 0:
                raise CrackHarnessError("siblings proof summary is not exact")
        else:
            if (
                _report_int(summary.get("target_count"), "physical.target_count")
                != _report_int(summary.get("candidate_count"), "physical.candidate_count")
                or _report_int(summary.get("differences"), "physical.differences") != 0
            ):
                raise CrackHarnessError("physical proof summary is not exact")

    assert object_pair is not None
    if target_object_sha256 != object_pair[0] or candidate_object_sha256 != object_pair[1]:
        raise CrackHarnessError("exact report object identity does not bind proof summaries")
    if candidate_object_sha256 != _sha(binding.get("object_sha256"), "binding.object_sha256"):
        raise CrackHarnessError("exact report candidate object does not bind central record")
    if (
        report_result.get("strict_percent") != compact_summaries["strict"].get("strict_percent")
        or report_result.get("data_percent") != compact_summaries["data"].get("data_percent")
        or report_result.get("target_bytes") != compact_summaries["strict"].get("target_bytes")
        or report_result.get("candidate_bytes") != compact_summaries["strict"].get("candidate_bytes")
    ):
        raise CrackHarnessError("exact report result does not bind strict/data proof summaries")

    assessment = compact_summaries["assess"]
    if set(assessment) != {
        "schema", "owner", "function", "candidate_source_sha256", "target_object_sha256",
        "candidate_object_sha256", "owner_gain", "data_gain", "data_diff_delta",
    } or assessment.get("schema") != "crack_assessment/v1":
        raise CrackHarnessError("exact assessment receipt is incomplete")
    if (
        assessment.get("owner") != owner
        or assessment.get("function") != function
        or assessment.get("candidate_source_sha256") != source_sha256
        or assessment.get("target_object_sha256") != object_pair[0]
        or assessment.get("candidate_object_sha256") != object_pair[1]
    ):
        raise CrackHarnessError("exact assessment is not object/source-bound")
    assessment_gain = _report_number(assessment.get("owner_gain"), "assessment.owner_gain")
    if (
        assessment_gain <= 0
        or _report_number(assessment.get("data_gain"), "assessment.data_gain") < 0
        or _report_signed_int(assessment.get("data_diff_delta"), "assessment.data_diff_delta") > 0
    ):
        raise CrackHarnessError("exact assessment does not prove a non-regressing gain")
    if report_gain != assessment_gain or _report_number(result.get("owner_gain"), "result.owner_gain") != assessment_gain:
        raise CrackHarnessError("exact owner gain is not assessment-bound")

    record = compact_summaries["record"]
    if set(record) != {
        "schema", "recorded", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "outcome",
        "admission_token_sha256", "admission_input_key", "record_sha256",
    } or record.get("schema") != "crack_central_record_receipt/v1":
        raise CrackHarnessError("exact central record summary is incomplete")
    if (
        record.get("recorded") is not True
        or record.get("owner") != owner
        or record.get("function") != function
        or record.get("candidate_source_sha256") != source_sha256
        or record.get("target_object_sha256") != object_pair[0]
        or record.get("candidate_object_sha256") != object_pair[1]
        or record.get("outcome") != "exact"
        or record.get("admission_input_key") != admission_input_key
        or record.get("admission_token_sha256") != _digest_bytes(admission_token.encode("utf-8"))
    ):
        raise CrackHarnessError("exact central record summary is not admission-bound")
    record_sha256 = _sha(record.get("record_sha256"), "record.record_sha256")
    if record_commit is not None:
        record_commit = _validate_record_commit(record_commit, "exact record commit")
        if record_commit.get("record_sha256") != record_sha256:
            raise CrackHarnessError("exact record SHA does not bind the central record commit")


def _valid_terminal_result(
    root: Path, path: Path, record_commit_path: Path, binding: Any,
) -> bool:
    try:
        if not path.is_file() or not isinstance(binding, Mapping):
            return False
        value = _read_json(path)
        if not isinstance(value, Mapping) or value.get("status") not in {"exact", "improved"}:
            return False
        body = dict(value)
        digest = body.pop("result_sha256", None)
        record = value.get("receipts", {}).get("record", {}) if isinstance(value.get("receipts"), Mapping) else {}
        commit = _record_commit_value(record_commit_path)
        summary = record.get("summary") if isinstance(record, Mapping) else None
        if (
            digest != _digest_json(body)
            or not isinstance(record, Mapping)
            or not isinstance(summary, Mapping)
            or _digest_json(summary) != record.get("payload_sha256")
            or commit is None
            or commit.get("record_payload_sha256") != record.get("payload_sha256")
            or commit.get("outcome") != value.get("status")
            or commit.get("candidate_sha256") != value.get("candidate_sha256")
        ):
            return False
        bound_target_object_sha256 = _sha(
            binding.get("target_object_sha256"),
            "binding.target_object_sha256",
        )
        if commit.get("record_sha256") != summary.get("record_sha256"):
            return False
        expected = {
            "schema": "crack_central_record_receipt/v1", "recorded": True,
            "owner": binding.get("owner"), "function": binding.get("function"),
            "candidate_source_sha256": binding.get("source_sha256"),
            "candidate_object_sha256": binding.get("object_sha256"),
            "outcome": binding.get("status"),
            "admission_input_key": binding.get("input_key"),
        }
        expected["target_object_sha256"] = bound_target_object_sha256
        if any(summary.get(key) != expected_value for key, expected_value in expected.items()):
            return False
        if (
            value.get("owner") != binding.get("owner")
            or value.get("function") != binding.get("function")
            or value.get("candidate_sha256") != binding.get("source_sha256")
            or value.get("status") != binding.get("status")
        ):
            return False
        terminal_record_sha256 = _sha(
            summary.get("record_sha256"),
            "terminal record.record_sha256",
        )
        if not _central_record_matches(
            root, binding, record_sha256=terminal_record_sha256,
        ):
            return False
        if value.get("status") != "exact":
            return True
        report_path = path.parent / "CRACK_REPORT_v1.json"
        if not report_path.is_file():
            return False
        report = _read_json(report_path)
        if not isinstance(report, Mapping):
            return False
        _validate_exact_report(report, value, binding, record_commit=commit)
        return True
    except (CrackHarnessError, OSError, TypeError, ValueError):
        return False
def _valid_exact_record_commit(path: Path, candidate_sha256: str) -> bool:
    value = _record_commit_value(path)
    return bool(
        value is not None and value.get("outcome") == "exact"
        and value.get("candidate_sha256") == candidate_sha256
    )


def _invalidate_central_record(
    root: Path, binding: Any, *, record_sha256: str | None = None,
) -> None:
    if binding is None:
        return
    memory_required = {
        "input_key", "owner", "function", "source_sha256", "object_sha256",
        "candidate_record_sha256", "status",
    }
    required = memory_required | {"target_object_sha256"}
    if not isinstance(binding, Mapping) or set(binding) != required:
        raise CrackHarnessError("transaction central record binding is invalid")
    try:
        from tools.recovery_memory import RecoveryMemory, RecoveryMemoryError

        memory_binding = {
            "input_key": binding["input_key"],
            "owner": binding["owner"],
            "function": binding["function"],
            "target_sha256": binding["target_object_sha256"],
            "source_sha256": binding["source_sha256"],
            "object_sha256": binding["object_sha256"],
            "candidate_record_sha256": binding["candidate_record_sha256"],
            "status": binding["status"],
        }
        if record_sha256 is not None:
            memory_binding["record_sha256"] = record_sha256
        result = RecoveryMemory.for_root(root).invalidate_retained(**memory_binding)
    except (OSError, RecoveryMemoryError) as exc:
        raise CrackHarnessError(
            "cannot reconcile the exact central retained experiment before rollback"
        ) from exc
    if result.get("status") not in {"missing", "invalidated"}:
        raise CrackHarnessError("central retained experiment reconciliation failed")


def _central_record_matches(
    root: Path, binding: Any, *, record_sha256: str | None = None,
) -> bool:
    memory_required = {
        "input_key", "owner", "function", "source_sha256", "object_sha256",
        "candidate_record_sha256", "status",
    }
    required = memory_required | {"target_object_sha256"}
    if not isinstance(binding, Mapping) or set(binding) != required:
        return False
    try:
        from tools.recovery_memory import RecoveryMemory, RecoveryMemoryError

        memory_binding = {
            "input_key": binding["input_key"],
            "owner": binding["owner"],
            "function": binding["function"],
            "target_sha256": binding["target_object_sha256"],
            "source_sha256": binding["source_sha256"],
            "object_sha256": binding["object_sha256"],
            "candidate_record_sha256": binding["candidate_record_sha256"],
            "status": binding["status"],
        }
        if record_sha256 is not None:
            memory_binding["record_sha256"] = record_sha256
        return RecoveryMemory.for_root(root).retained_matches(**memory_binding)
    except (OSError, RecoveryMemoryError):
        return False


def _recover_interrupted(root: Path, state: Path) -> None:
    journal = state / "transaction.json"
    if not journal.is_file():
        return
    value = _read_json(journal)
    if not isinstance(value, Mapping):
        raise CrackHarnessError("interrupted transaction journal is invalid")
    unsigned = dict(value)
    digest = unsigned.pop("transaction_sha256", None)
    required = {
        "schema", "source_relpath", "baseline_snapshot", "baseline_sha256",
        "approval_sha256", "target_object_sha256", "candidate_sha256",
        "result_path", "worktree",
        "record_commit_path", "central_record_binding",
    }
    if set(unsigned) != required or unsigned.get("schema") != "crack_harness_transaction/v1" or digest != _digest_json(unsigned):
        raise CrackHarnessError("interrupted transaction journal integrity failed")
    source_relpath = _text(value.get("source_relpath"), "transaction source_relpath")
    source = _bound_path(root, source_relpath, "transaction source")
    baseline = Path(os.path.abspath(str(value.get("baseline_snapshot"))))
    result_path = Path(os.path.abspath(str(value.get("result_path", ""))))
    record_commit_path = Path(os.path.abspath(str(value.get("record_commit_path", ""))))
    worktree = Path(os.path.abspath(str(value.get("worktree", ""))))
    result_parts = result_path.relative_to(state).parts if _inside(state, result_path) else ()
    if len(result_parts) != 5 or result_parts[0] != "owners" or result_parts[-2:] != ("latest", "result.json"):
        raise CrackHarnessError("interrupted transaction result path escapes latest state")
    run_dir = result_path.parent
    expected_temp = run_dir / "temp"
    if baseline != expected_temp / "baseline.snapshot" or worktree != expected_temp / "worktree":
        raise CrackHarnessError("interrupted transaction disposable paths escape the exact run temp")
    if record_commit_path != run_dir / "record.commit.json":
        raise CrackHarnessError("interrupted transaction record path escapes latest state")
    for path in (source, baseline, run_dir, expected_temp):
        if path.exists():
            _assert_no_indirection(path)
    if not _is_tracked(root, source):
        raise CrackHarnessError("interrupted transaction source is not the approved tracked source")
    expected = _sha(value.get("baseline_sha256"), "transaction baseline_sha256")
    _sha(value.get("approval_sha256"), "transaction approval_sha256")
    target_object_sha256 = _sha(
        value.get("target_object_sha256"), "transaction target_object_sha256"
    )
    _sha(value.get("candidate_sha256"), "transaction candidate_sha256")
    if not baseline.is_file() or _digest_file(baseline) != expected:
        raise CrackHarnessError("interrupted transaction baseline is unavailable")
    candidate_sha = value.get("candidate_sha256")
    record_commit_exists = record_commit_path.is_file()
    record_commit = _record_commit_value(record_commit_path)
    if record_commit_exists and record_commit is None:
        # A malformed local commit cannot be used to authenticate, or to
        # invalidate, a central row. Leave the row untouched and fail closed
        # rather than allowing a rewritten journal/commit to delete it.
        raise CrackHarnessError(
            "interrupted transaction record commit is invalid; central record retained"
        )
    record_commit_sha256 = (
        record_commit.get("record_sha256") if record_commit is not None else None
    )
    binding = value.get("central_record_binding")
    terminal_binding = binding
    if isinstance(binding, Mapping):
        bound_target_object_sha256 = _sha(
            binding.get("target_object_sha256"),
            "transaction central_record_binding.target_object_sha256",
        )
        if bound_target_object_sha256 != target_object_sha256:
            raise CrackHarnessError(
                "transaction target object does not match its central record binding"
            )
        terminal_binding = dict(binding)
    terminal_valid = _valid_terminal_result(
        root, result_path, record_commit_path, terminal_binding
    )
    retained = (
        isinstance(candidate_sha, str) and source.is_file()
        and _digest_file(source) == candidate_sha
        and terminal_valid
    )
    if not retained:
        # The journal is written before record.  If record committed but the
        # local terminal commit did not, delete only that exact bound row before
        # restoring source so central memory and the live tree cannot diverge.
        _invalidate_central_record(
            root,
            value.get("central_record_binding"),
            record_sha256=record_commit_sha256,
        )
        _atomic_copy(baseline, source)
        exact_record_only = _valid_exact_record_commit(record_commit_path, str(candidate_sha))
        if exact_record_only:
            _atomic_json(state / "RECOVERY_REQUIRED.json", {
                "schema": "crack_harness_recovery_required/v1",
                "reason": "central exact record committed without a hash-bound CRACK_REPORT; source rolled back",
                "candidate_sha256": candidate_sha,
            })
        result_path.unlink(missing_ok=True)
        (run_dir / "CRACK_REPORT_v1.json").unlink(missing_ok=True)
        record_commit_path.unlink(missing_ok=True)
    journal.unlink()
    if terminal_valid and record_commit_path.is_file():
        record_commit_path.unlink(missing_ok=True)
    if worktree.exists():
        _remove_disposable_worktree(root, worktree)
    if baseline.parent.exists():
        shutil.rmtree(baseline.parent, ignore_errors=False)


def _run_command(
    argv: Sequence[str], *, root: Path, run_temp: Path, deadline: float,
    storage_limit: int, expect_json: bool, extra_env: Mapping[str, str] | None = None,
    production_root: Path | None = None, state_root: Path | None = None,
) -> tuple[dict[str, Any] | None, dict[str, Any]]:
    remaining = deadline - time.monotonic()
    if remaining <= 0:
        raise CrackHarnessError("active-time limit exceeded")
    env = os.environ.copy()
    for secret_name in tuple(env):
        if secret_name.upper().startswith("CRACK_HARNESS_MANAGER_"):
            env.pop(secret_name, None)
    env.update({
        "CRACK_HARNESS_TEMP": os.fspath(run_temp),
        "CRACK_HARNESS_ROOT": os.fspath(root),
    })
    env.update(extra_env or {})
    # Reviewed Python front doors may import controller-owned modules.  Never
    # let those imports materialize __pycache__ beside the immutable tooling;
    # all durable command output must remain in the disposable roots.
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    env.pop("PYTHONPYCACHEPREFIX", None)
    for redirected_name in (
        "MP6_RECOVERY_MEMORY", "MP6_AGENT_QUEUE", "GIT_DIR", "GIT_COMMON_DIR",
        "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_OBJECT_DIRECTORY",
        "GIT_ALTERNATE_OBJECT_DIRECTORIES",
    ):
        env.pop(redirected_name, None)
    started = time.monotonic()
    production_manifest = (
        _repo_manifest(production_root, state_root) if production_root and state_root else None
    )
    state_manifest = (
        _tree_manifest(state_root, (run_temp,)) if state_root else None
    )
    next_manifest_check = time.monotonic()
    process = subprocess.Popen(
        list(argv), cwd=root, env=env,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        start_new_session=os.name != "nt",
        creationflags=(
            getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
            | getattr(subprocess, "CREATE_SUSPENDED", 0x00000004)
            if os.name == "nt" else 0
        ),
    )
    job_handle = _assign_windows_job(process)
    _resume_windows_process(process, job_handle)
    captured = {"stdout": bytearray(), "stderr": bytearray()}
    overflow = threading.Event()
    capture_lock = threading.Lock()

    def drain(name: str, stream: Any) -> None:
        while True:
            chunk = stream.read(65536)
            if not chunk:
                return
            with capture_lock:
                if sum(len(value) for value in captured.values()) + len(chunk) > 1024 * 1024:
                    overflow.set()
                    return
                captured[name].extend(chunk)

    readers = [
        threading.Thread(target=drain, args=("stdout", process.stdout), daemon=True),
        threading.Thread(target=drain, args=("stderr", process.stderr), daemon=True),
    ]
    for reader in readers:
        reader.start()
    try:
        while process.poll() is None:
            if time.monotonic() >= deadline:
                _terminate_process(process)
                raise CrackHarnessError("active-time limit exceeded")
            if _tree_size(run_temp) > storage_limit:
                _terminate_process(process)
                raise CrackHarnessError("temporary-storage limit exceeded")
            if overflow.is_set():
                _terminate_process(process)
                raise CrackHarnessError("command output exceeded 1 MiB compact-output limit")
            if production_manifest is not None and time.monotonic() >= next_manifest_check:
                current_production = _repo_manifest(production_root, state_root)
                if current_production != production_manifest:
                    _terminate_process(process)
                    raise CrackHarnessError(
                        "reviewed command wrote outside the disposable worktree: "
                        + _manifest_delta(production_manifest, current_production)
                    )
                if _tree_manifest(state_root, (run_temp,)) != state_manifest:
                    _terminate_process(process)
                    raise CrackHarnessError("reviewed command wrote outside its monitored run root")
                next_manifest_check = time.monotonic() + 0.25
            time.sleep(0.02)
    except BaseException:
        _terminate_process(process)
        try:
            _quiesce_windows_job(job_handle, terminate=True)
        finally:
            _close_windows_job(job_handle)
        for reader in readers:
            reader.join(timeout=2.0)
        if process.stdout:
            process.stdout.close()
        if process.stderr:
            process.stderr.close()
        raise
    process.wait(timeout=5.0)
    try:
        _quiesce_windows_job(job_handle, terminate=True)
    finally:
        _close_windows_job(job_handle)
    for reader in readers:
        reader.join(timeout=2.0)
    if process.stdout:
        process.stdout.close()
    if process.stderr:
        process.stderr.close()
    if production_manifest is not None:
        current_production = _repo_manifest(production_root, state_root)
        if current_production != production_manifest:
            raise CrackHarnessError(
                "reviewed command wrote outside the disposable worktree: "
                + _manifest_delta(production_manifest, current_production)
            )
    if state_manifest is not None and _tree_manifest(state_root, (run_temp,)) != state_manifest:
        raise CrackHarnessError("reviewed command wrote outside its monitored run root")
    stdout = bytes(captured["stdout"]).decode("utf-8", errors="replace")
    stderr = bytes(captured["stderr"]).decode("utf-8", errors="replace")
    elapsed = round(time.monotonic() - started, 6)
    if overflow.is_set() or len(captured["stdout"]) + len(captured["stderr"]) > 1024 * 1024:
        raise CrackHarnessError("command output exceeded 1 MiB compact-output limit")
    receipt = {
        "argv_sha256": _digest_json(list(argv)),
        "returncode": process.returncode,
        "active_seconds": elapsed,
        "stdout_sha256": _digest_bytes(stdout.encode("utf-8")),
        "stderr_sha256": _digest_bytes(stderr.encode("utf-8")),
    }
    if process.returncode != 0:
        detail = stderr.strip() or stdout.strip() or "no diagnostic"
        raise CrackHarnessError(f"reviewed command failed ({process.returncode}): {detail[:500]}")
    if _tree_size(run_temp) > storage_limit:
        raise CrackHarnessError("temporary-storage limit exceeded")
    if not expect_json:
        return None, receipt
    try:
        value = json.loads(stdout)
    except json.JSONDecodeError as exc:
        raise CrackHarnessError("reviewed proof command did not emit one JSON object") from exc
    if not isinstance(value, Mapping):
        raise CrackHarnessError("reviewed proof command output must be a JSON object")
    return dict(value), receipt


def _terminate_process(process: subprocess.Popen[str]) -> None:
    """Terminate the complete reviewed-command process group when possible."""
    if process.poll() is not None:
        return
    if os.name == "nt":
        job_handle = getattr(process, "_crack_harness_job", None)
        if job_handle:
            import ctypes
            ctypes.windll.kernel32.TerminateJobObject(job_handle, 1)
        completed = subprocess.run(
            ["taskkill", "/PID", str(process.pid), "/T", "/F"],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False,
        )
        if completed.returncode != 0 and process.poll() is None:
            process.kill()
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    try:
        process.wait(timeout=5.0)
    except subprocess.TimeoutExpired:
        process.kill()
        try:
            process.wait(timeout=5.0)
        except subprocess.TimeoutExpired as exc:
            raise CrackHarnessError("reviewed process did not quiesce after termination") from exc


def _quiesce_windows_job(
    handle: int | None, *, terminate: bool, timeout: float = 5.0,
) -> None:
    if not handle or os.name != "nt":
        return
    import ctypes
    from ctypes import wintypes

    class BASIC_ACCOUNTING(ctypes.Structure):
        _fields_ = [
            ("TotalUserTime", ctypes.c_longlong),
            ("TotalKernelTime", ctypes.c_longlong),
            ("ThisPeriodTotalUserTime", ctypes.c_longlong),
            ("ThisPeriodTotalKernelTime", ctypes.c_longlong),
            ("TotalPageFaultCount", wintypes.DWORD),
            ("TotalProcesses", wintypes.DWORD),
            ("ActiveProcesses", wintypes.DWORD),
            ("TotalTerminatedProcesses", wintypes.DWORD),
        ]

    kernel = ctypes.windll.kernel32
    if terminate and not kernel.TerminateJobObject(handle, 1):
        raise CrackHarnessError("cannot terminate reviewed Windows Job")
    deadline = time.monotonic() + timeout
    accounting = BASIC_ACCOUNTING()
    while True:
        if not kernel.QueryInformationJobObject(
            handle, 1, ctypes.byref(accounting), ctypes.sizeof(accounting), None
        ):
            raise CrackHarnessError("cannot query reviewed Windows Job quiescence")
        if accounting.ActiveProcesses == 0:
            return
        if time.monotonic() >= deadline:
            raise CrackHarnessError("reviewed Windows Job retained live descendants")
        time.sleep(0.02)


def _assign_windows_job(process: subprocess.Popen[Any]) -> int | None:
    """Put the command in a kill-on-close Job so exited parents cannot orphan children."""
    if os.name != "nt":
        return None
    import ctypes
    from ctypes import wintypes

    class BASIC_LIMITS(ctypes.Structure):
        _fields_ = [
            ("PerProcessUserTimeLimit", ctypes.c_longlong),
            ("PerJobUserTimeLimit", ctypes.c_longlong),
            ("LimitFlags", wintypes.DWORD),
            ("MinimumWorkingSetSize", ctypes.c_size_t),
            ("MaximumWorkingSetSize", ctypes.c_size_t),
            ("ActiveProcessLimit", wintypes.DWORD),
            ("Affinity", ctypes.c_size_t),
            ("PriorityClass", wintypes.DWORD),
            ("SchedulingClass", wintypes.DWORD),
        ]

    class IO_COUNTERS(ctypes.Structure):
        _fields_ = [(name, ctypes.c_ulonglong) for name in (
            "ReadOperationCount", "WriteOperationCount", "OtherOperationCount",
            "ReadTransferCount", "WriteTransferCount", "OtherTransferCount",
        )]

    class EXTENDED_LIMITS(ctypes.Structure):
        _fields_ = [
            ("BasicLimitInformation", BASIC_LIMITS),
            ("IoInfo", IO_COUNTERS),
            ("ProcessMemoryLimit", ctypes.c_size_t),
            ("JobMemoryLimit", ctypes.c_size_t),
            ("PeakProcessMemoryUsed", ctypes.c_size_t),
            ("PeakJobMemoryUsed", ctypes.c_size_t),
        ]

    kernel = ctypes.windll.kernel32
    kernel.CreateJobObjectW.restype = wintypes.HANDLE
    handle = kernel.CreateJobObjectW(None, None)
    if not handle:
        process.kill()
        raise CrackHarnessError("cannot create Windows containment job")
    limits = EXTENDED_LIMITS()
    limits.BasicLimitInformation.LimitFlags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    if not kernel.SetInformationJobObject(handle, 9, ctypes.byref(limits), ctypes.sizeof(limits)):
        kernel.CloseHandle(handle); process.kill()
        raise CrackHarnessError("cannot configure Windows containment job")
    if not kernel.AssignProcessToJobObject(handle, wintypes.HANDLE(process._handle)):
        kernel.CloseHandle(handle); process.kill()
        raise CrackHarnessError("cannot assign reviewed command to Windows containment job")
    process._crack_harness_job = handle
    return handle


def _resume_windows_process(process: subprocess.Popen[Any], job_handle: int | None) -> None:
    """Resume only after Job assignment, closing the Windows child-spawn race."""
    if os.name != "nt":
        return
    import ctypes
    from ctypes import wintypes

    # subprocess closes the primary-thread handle; NtResumeProcess safely resumes
    # every thread while the process handle remains owned by Popen.
    result = ctypes.windll.ntdll.NtResumeProcess(wintypes.HANDLE(process._handle))
    if result != 0:
        if job_handle:
            ctypes.windll.kernel32.TerminateJobObject(job_handle, 1)
        process.kill()
        _close_windows_job(job_handle)
        raise CrackHarnessError("cannot resume contained Windows reviewed command")


def _close_windows_job(handle: int | None) -> None:
    if handle and os.name == "nt":
        import ctypes
        ctypes.windll.kernel32.CloseHandle(handle)


def _repo_manifest(root: Path, state: Path) -> dict[str, tuple[int, int]]:
    return _tree_manifest(root, (state,))


def _manifest_delta(
    before: Mapping[str, tuple[int, int]], after: Mapping[str, tuple[int, int]],
) -> str:
    changed = sorted(
        path for path in set(before) | set(after) if before.get(path) != after.get(path)
    )
    if not changed:
        return "unknown path"
    summary = ", ".join(changed[:8])
    return summary + (f" (+{len(changed) - 8} more)" if len(changed) > 8 else "")


def _tree_manifest(
    root: Path, excluded: Sequence[Path] = (),
) -> dict[str, tuple[int, int]]:
    result: dict[str, tuple[int, int]] = {}
    for parent, dirs, files in os.walk(root, followlinks=False):
        base = Path(parent)
        retained_dirs = []
        for name in dirs:
            candidate = base / name
            info = candidate.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(
                getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
            ):
                raise CrackHarnessError(f"tree path indirection is forbidden: {candidate}")
            if not any(_inside(item, candidate) for item in excluded):
                retained_dirs.append(name)
        dirs[:] = retained_dirs
        for name in files:
            path = base / name
            if any(_inside(item, path) for item in excluded):
                continue
            info = path.lstat()
            if stat.S_ISLNK(info.st_mode) or bool(
                getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
            ):
                raise CrackHarnessError(f"tree path indirection is forbidden: {path}")
            result[path.relative_to(root).as_posix()] = (info.st_size, info.st_mtime_ns)
    return result


def _compact_receipt(name: str, payload: Mapping[str, Any], command: Mapping[str, Any]) -> dict[str, Any]:
    if name == "record":
        if len(_canonical(payload)) > 64 * 1024 or not all(
            isinstance(value, (str, int, float, bool)) or value is None
            for value in payload.values()
        ):
            raise CrackHarnessError("central record receipt exceeds its compact closed form")
        summary = dict(payload)
    else:
        summary = {}
    for key in (() if name == "record" else sorted(payload, key=str)[:32]):
        value = payload[key]
        if isinstance(value, (str, int, float, bool)) or value is None:
            summary[str(key)] = value if not isinstance(value, str) else value[:500]
        elif isinstance(value, list) and len(value) <= 64 and all(
            isinstance(item, (str, int, float, bool)) or item is None for item in value
        ):
            summary[str(key)] = [item[:200] if isinstance(item, str) else item for item in value]
    return {
        "schema": "crack_harness_receipt/v1",
        "hook": name,
        "ok": payload.get("ok", True),
        "summary": summary,
        "payload_sha256": _digest_json(payload),
        "command": dict(command),
    }


def _validate_proof(
    name: str, payload: Mapping[str, Any], approval: Mapping[str, Any],
    object_pair: tuple[str, str] | None,
) -> tuple[dict[str, Any], tuple[str, str], bool]:
    common = {
        "schema", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "report_sha256",
    }
    extra = {
        "strict": {"strict_percent", "target_bytes", "candidate_bytes", "differences"},
        "data": {"data_percent", "target_bytes", "candidate_bytes", "differences"},
        "focus": {"differing_rows"},
        "siblings": {"protected_total", "protected_losses"},
        "physical": {"target_count", "candidate_count", "differences"},
    }[name]
    if set(payload) != common | extra or payload.get("schema") != f"crack_proof_{name}/v1":
        raise CrackHarnessError(f"{name} proof payload is not the strict typed schema")
    for key, expected in (
        ("owner", approval["owner"]), ("function", approval["function"]),
        ("candidate_source_sha256", approval["candidate"]["sha256"]),
    ):
        if payload.get(key) != expected:
            raise CrackHarnessError(f"{name} proof does not bind {key}")
    target = _sha(payload.get("target_object_sha256"), f"{name}.target_object_sha256")
    candidate = _sha(payload.get("candidate_object_sha256"), f"{name}.candidate_object_sha256")
    _sha(payload.get("report_sha256"), f"{name}.report_sha256")
    pair = (target, candidate)
    if target != approval["target_sha256"]:
        raise CrackHarnessError(f"{name} proof target object is not the approved target")
    if object_pair is not None and pair != object_pair:
        raise CrackHarnessError("proof hooks disagree on target/candidate object identity")
    for key in extra:
        value = payload.get(key)
        if (
            isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(float(value))
            or value < 0
        ):
            raise CrackHarnessError(f"{name}.{key} must be non-negative numeric evidence")
    if name == "strict":
        exact = payload["strict_percent"] == 100 and payload["differences"] == 0 and payload["target_bytes"] == payload["candidate_bytes"]
    elif name == "data":
        exact = payload["data_percent"] == 100 and payload["differences"] == 0 and payload["target_bytes"] == payload["candidate_bytes"]
    elif name == "focus":
        exact = payload["differing_rows"] == 0
    elif name == "siblings":
        exact = payload["protected_losses"] == 0
    else:
        exact = payload["differences"] == 0 and payload["target_count"] == payload["candidate_count"]
    return dict(payload), pair, exact


def _validate_assessment(
    payload: Mapping[str, Any], approval: Mapping[str, Any],
    object_pair: tuple[str, str],
) -> float:
    if set(payload) != {"schema", "owner", "function", "candidate_source_sha256", "target_object_sha256", "candidate_object_sha256", "owner_gain", "data_gain", "data_diff_delta"} or payload.get("schema") != "crack_assessment/v1":
        raise CrackHarnessError("assessment payload is not the strict typed schema")
    if (
        payload.get("owner") != approval["owner"]
        or payload.get("function") != approval["function"]
        or payload.get("candidate_source_sha256") != approval["candidate"]["sha256"]
        or payload.get("target_object_sha256") != object_pair[0]
        or payload.get("candidate_object_sha256") != object_pair[1]
    ):
        raise CrackHarnessError("assessment does not bind this candidate")
    gain = payload.get("owner_gain")
    data_gain = payload.get("data_gain")
    if any(
        isinstance(value, bool)
        or not isinstance(value, (int, float))
        or not math.isfinite(float(value))
        for value in (gain, data_gain)
    ):
        raise CrackHarnessError("assessment gains must be finite numeric values")
    if isinstance(payload.get("data_diff_delta"), bool) or not isinstance(payload.get("data_diff_delta"), int):
        raise CrackHarnessError("assessment.data_diff_delta must be an integer")
    return float(gain)


def _validate_record(
    payload: Mapping[str, Any], approval: Mapping[str, Any], outcome: str,
    object_pair: tuple[str, str], admission_token: str, admission_input_key: str,
) -> None:
    required = {
        "schema", "recorded", "owner", "function", "candidate_source_sha256",
        "target_object_sha256", "candidate_object_sha256", "outcome",
        "admission_token_sha256", "admission_input_key", "record_sha256",
    }
    if set(payload) != required or payload.get("schema") != "crack_central_record_receipt/v1" or payload.get("recorded") is not True:
        raise CrackHarnessError("central outcome was not recorded with a strict typed receipt")
    expected = {
        "owner": approval["owner"], "function": approval["function"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "target_object_sha256": object_pair[0],
        "candidate_object_sha256": object_pair[1], "outcome": outcome,
        "admission_token_sha256": _digest_bytes(admission_token.encode("utf-8")),
        "admission_input_key": admission_input_key,
    }
    if any(payload.get(key) != value for key, value in expected.items()):
        raise CrackHarnessError("central record receipt does not bind the measured outcome")
    _sha(payload.get("record_sha256"), "record_sha256")


def _run_canonical_record(
    root: Path, approval: Mapping[str, Any], outcome: str,
    object_pair: tuple[str, str], admission_token: str, admission_input_key: str,
    proof_payloads: Mapping[str, Mapping[str, Any]], assessment: Mapping[str, Any],
    *, run_temp: Path, deadline: float, storage_limit: int,
) -> tuple[dict[str, Any], dict[str, Any]]:
    descriptor = approval["commands"]["record"]
    report_sha = proof_payloads["strict"]["report_sha256"]
    argv = [
        str(descriptor["executable"]), str(descriptor["script"]),
        "--root", str(root), "record",
        "--owner", approval["owner"], "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--object-sha256", object_pair[1], "--status", outcome,
        "--reason", f"bounded harness outcome: {outcome}",
        "--admission-token", admission_token,
        "--candidate-id", approval["approval_id"],
        "--candidate-record-sha256", _digest_json(assessment),
        "--strict-report-sha256", proof_payloads["strict"]["report_sha256"],
        "--data-report-sha256", proof_payloads["data"]["report_sha256"],
        "--report-sha256", report_sha, "--workspace", "crack-harness",
        "--json",
    ]
    canonical, command = _run_command(
        argv, root=root, run_temp=run_temp, deadline=deadline,
        storage_limit=storage_limit, expect_json=True,
    )
    if not isinstance(canonical, Mapping):
        raise CrackHarnessError(
            "canonical recovery memory did not return an authenticated experiment row"
        )
    if canonical.get("status") != "recorded" or canonical.get("authority_advanced") is not False:
        raise CrackHarnessError("canonical recovery memory did not record exactly one measured outcome")
    experiment = canonical.get("experiment")
    if not isinstance(experiment, Mapping):
        raise CrackHarnessError(
            "canonical recovery memory did not return its authenticated experiment row"
        )
    required_experiment = {
        "input_key", "owner", "function_name", "target_sha256",
        "source_sha256", "object_sha256", "status", "record_sha256",
    }
    if not required_experiment <= set(experiment):
        raise CrackHarnessError(
            "canonical recovery memory experiment row is incomplete"
        )
    expected_experiment = {
        "input_key": admission_input_key,
        "owner": approval["owner"],
        "function_name": approval["function"],
        "target_sha256": approval["target_sha256"],
        "source_sha256": approval["candidate"]["sha256"],
        "object_sha256": object_pair[1],
        "status": outcome,
    }
    if any(
        experiment.get(key) != expected
        for key, expected in expected_experiment.items()
    ):
        raise CrackHarnessError(
            "canonical recovery memory experiment row does not bind the measured outcome"
        )
    central_record_sha256 = _sha(
        experiment.get("record_sha256"),
        "central experiment.record_sha256",
    )
    payload = {
        "schema": "crack_central_record_receipt/v1", "recorded": True,
        "owner": approval["owner"], "function": approval["function"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "target_object_sha256": object_pair[0],
        "candidate_object_sha256": object_pair[1], "outcome": outcome,
        "admission_token_sha256": _digest_bytes(admission_token.encode("utf-8")),
        "admission_input_key": admission_input_key,
        # This is the central DB experiment digest, not a digest of the CLI
        # response envelope. Recovery must authenticate against this value.
        "record_sha256": central_record_sha256,
    }
    return payload, command


def _run_canonical_discard(
    root: Path, approval: Mapping[str, Any], admission_token: str,
    admission_input_key: str,
    *, run_temp: Path, deadline: float, storage_limit: int,
) -> tuple[dict[str, Any], dict[str, Any]]:
    descriptor = approval["commands"]["record"]
    argv = [
        str(descriptor["executable"]), str(descriptor["script"]),
        "--root", str(root), "discard",
        "--owner", approval["owner"], "--function", approval["function"],
        "--base-commit", approval["base_commit"],
        "--toolchain-key", approval["toolchain_key"],
        "--target-sha256", approval["target_sha256"],
        "--source-sha256", approval["candidate"]["sha256"],
        "--source-path", str(approval["_paths"]["source"]),
        "--admission-token", admission_token, "--json",
    ]
    payload, command = _run_command(
        argv, root=root, run_temp=run_temp, deadline=deadline,
        storage_limit=storage_limit, expect_json=True,
    )
    assert payload is not None
    if (
        set(payload) != {"status", "input_key", "authority_advanced"}
        or payload.get("status") not in {"discarded", "missing"}
        or payload.get("input_key") != admission_input_key
    ):
        raise CrackHarnessError("canonical recovery memory did not discard the pending admission")
    _sha(payload.get("input_key"), "discard input_key")
    if payload.get("authority_advanced") is not False:
        raise CrackHarnessError("discard unexpectedly advanced central authority")
    return dict(payload), command


def _run_dir(state_root: Path, approval: Mapping[str, Any]) -> Path:
    owner = re.sub(r"[^A-Za-z0-9_.-]+", "_", approval["owner"]).strip("_") or "owner"
    owner += "-" + _digest_bytes(str(approval["owner"]).encode("utf-8"))[:12]
    function = re.sub(r"[^A-Za-z0-9_.-]+", "_", approval["function"]).strip("_")
    return state_root / "owners" / owner / function / "latest"


def _result_path(state_root: Path, approval: Mapping[str, Any]) -> Path:
    return _run_dir(state_root, approval) / "result.json"


def _function_results(state_root: Path, approval: Mapping[str, Any]) -> list[dict[str, Any]]:
    path = _result_path(state_root, approval)
    if not path.is_file():
        return []
    value = _read_json(path)
    if (
        isinstance(value, Mapping)
        and value.get("owner") == approval["owner"]
        and value.get("function") == approval["function"]
    ):
        return [dict(value)]
    return []


def _function_key(approval: Mapping[str, Any]) -> str:
    return _digest_json({
        "owner": approval["owner"], "function": approval["function"],
    })


def _function_consumed(run_dir: Path, approval: Mapping[str, Any]) -> bool:
    path = run_dir.parent / "latest-function.json"
    if path.is_file():
        _assert_no_indirection(path)
        value = _read_json(path)
        required = {
            "schema", "function_key", "owner", "function",
            "first_campaign_id", "consumed",
        }
        if not isinstance(value, Mapping) or set(value) != required:
            raise CrackHarnessError("function tombstone is malformed")
        valid = (
            value.get("schema") == "crack_harness_function_tombstone/v1"
            and value.get("function_key") == _function_key(approval)
            and value.get("owner") == approval["owner"]
            and value.get("function") == approval["function"]
            and isinstance(value.get("first_campaign_id"), str)
            and bool(value.get("first_campaign_id"))
            and value.get("consumed") is True
        )
        if not valid:
            raise CrackHarnessError("function tombstone binding is invalid")
        return True
    legacy = run_dir.parent / "latest-campaign.json"
    if not legacy.is_file():
        return False
    _assert_no_indirection(legacy)
    value = _read_json(legacy)
    if (
        not isinstance(value, Mapping)
        or set(value) != {"schema", "campaign_key", "campaign_id", "consumed"}
        or value.get("schema") != "crack_harness_campaign_tombstone/v1"
        or SHA_RE.fullmatch(str(value.get("campaign_key"))) is None
        or not isinstance(value.get("campaign_id"), str)
        or not value.get("campaign_id")
        or value.get("consumed") is not True
    ):
        raise CrackHarnessError("legacy campaign tombstone is malformed")
    # Its containing directory is already the exact owner/function namespace.
    # Treat every valid legacy cell as function consumption so changing the
    # campaign identifier cannot bypass the historical one-cell gate.
    return True


def _consume_function(run_dir: Path, approval: Mapping[str, Any]) -> None:
    path = run_dir.parent / "latest-function.json"
    _safe_mkdir(path.parent)
    key = _function_key(approval)
    if _function_consumed(run_dir, approval):
        raise CrackHarnessError("function already consumed its one lifetime cell")
    _atomic_json(path, {
        "schema": "crack_harness_function_tombstone/v1", "function_key": key,
        "owner": approval["owner"], "function": approval["function"],
        "first_campaign_id": approval["campaign"]["id"], "consumed": True,
    })


def _dry_run_core(root: Path, approval_path: Path, state: Path) -> dict[str, Any]:
    _assert_no_indirection(Path(os.path.abspath(root)))
    approval = load_approval(root, approval_path, allow_applied_source=True)
    existing = _result_path(state, approval)
    results = _function_results(state, approval)
    blockers = []
    if results:
        blockers.append("function already has a terminal result")
    if _function_consumed(_run_dir(state, approval), approval):
        blockers.append("function tombstone forbids another lifetime cell")
    if approval.get("_source_applied") and not existing.exists():
        blockers.append("source is already the candidate without a terminal result")
    if any(item.get("status") in {"failed", "no_gain"} for item in results):
        blockers.append("function is terminal after a failed or no-gain candidate")
    if len(results) >= approval["campaign"].get("quota", 1):
        blockers.append("function lifetime quota exhausted")
    return {
        "schema": "crack_harness_dry_run/v1",
        "status": "ready" if not blockers else "blocked",
        "approval_id": approval["approval_id"],
        "approval_sha256": approval["_approval_sha256"],
        "owner": approval["owner"],
        "task_id": approval["task_id"],
        "base_commit": approval["base_commit"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "predicted_rows": approval["predicted_rows"],
        "limits": approval["limits"],
        "blockers": blockers,
        "authority_advanced": False,
    }


def dry_run(root: Path, approval_path: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _dry_run_core(root, approval_path, _state_root(root))


def _dry_run_for_test(root: Path, approval_path: Path, *, state_root: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _dry_run_core(root, approval_path, _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN))


def _cleanup_raw(run_dir: Path) -> None:
    for name in ("temp", "raw", "logs"):
        path = run_dir / name
        if path.exists():
            _assert_no_indirection(path)
            shutil.rmtree(path)


def _gc_owner(
    run_dir: Path, byte_limit: int, *, protected: set[Path] | None = None,
) -> None:
    owner_dir = run_dir.parents[1]
    protected_paths = {Path(os.path.abspath(item)) for item in (protected or set())}
    entries = sorted(
        (
            item for item in owner_dir.iterdir()
            if item.is_dir() and Path(os.path.abspath(item)) not in protected_paths
        ),
        key=lambda item: item.stat().st_mtime,
    )
    while _tree_size(owner_dir) > byte_limit and entries:
        victim = entries.pop(0)
        _assert_no_indirection(victim)
        shutil.rmtree(victim)
    if _tree_size(owner_dir) > byte_limit:
        raise CrackHarnessError("owner retained state exceeds the hard 16 MiB cap")


def _gc_global(
    state: Path, byte_limit: int, *, protected: set[Path] | None = None,
) -> None:
    owners = state / "owners"
    if not owners.exists():
        return
    protected_paths = {Path(os.path.abspath(item)) for item in (protected or set())}
    entries = sorted(
        (
            function_dir
            for owner_dir in owners.iterdir() if owner_dir.is_dir()
            for function_dir in owner_dir.iterdir()
            if function_dir.is_dir()
            and Path(os.path.abspath(function_dir)) not in protected_paths
        ),
        key=lambda item: item.stat().st_mtime,
    )
    while _tree_size(state) > byte_limit and entries:
        victim = entries.pop(0)
        _assert_no_indirection(victim)
        shutil.rmtree(victim)
    if _tree_size(state) > byte_limit:
        raise CrackHarnessError("global retained state exceeds the hard 64 MiB cap")


def _run_approved_core(
    root: Path, approval_path: Path, *, permit_path: Path, state: Path,
    manager_key_path: Path, expected_key_id: str,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    approval = load_approval(root, approval_path, allow_applied_source=True)
    _safe_mkdir(state)
    permit, permit_file = _load_permit(
        root, approval, permit_path, state, manager_key_path=manager_key_path,
        expected_key_id=expected_key_id,
    )
    run_dir = _run_dir(state, approval)
    transaction_lock = state / ".transaction.lock"
    with serialized_build_lock(transaction_lock, 55.0):
        try:
            return _run_locked(root, approval_path, approval, permit, permit_file, state, run_dir)
        except BaseException as primary:
            cleanup_errors: list[str] = []
            try:
                if (state / "transaction.json").exists():
                    _recover_interrupted(root, state)
            except BaseException as exc:
                cleanup_errors.append(f"recovery: {exc}"[:1000])
            try:
                _cleanup_raw(run_dir)
            except BaseException as exc:
                cleanup_errors.append(f"raw cleanup: {exc}"[:1000])
            for disposable in (
                approval["_paths"]["base"], approval["_paths"]["candidate"],
                permit_file, approval["_approval_path"],
            ):
                try:
                    if disposable.exists() and not _is_tracked(root, disposable):
                        disposable.unlink()
                except BaseException as exc:
                    cleanup_errors.append(f"delete {disposable.name}: {exc}"[:1000])
            try:
                (state / "attempt.json").unlink(missing_ok=True)
            except BaseException as exc:
                cleanup_errors.append(f"delete attempt: {exc}"[:1000])
            diagnostic_body = {
                "schema": "crack_harness_failure_diagnostic/v1",
                "approval_sha256": approval["_approval_sha256"],
                "owner": approval["owner"], "function": approval["function"],
                "primary_reason": str(primary)[:1000],
                "cleanup_errors": cleanup_errors[:8], "finished_at": _now(),
            }
            try:
                _atomic_json(run_dir.parent / "latest-failure.json", {
                    **diagnostic_body,
                    "diagnostic_sha256": _digest_json(diagnostic_body),
                })
            except BaseException as exc:
                cleanup_errors.append(f"diagnostic seal: {exc}"[:1000])
            if cleanup_errors and hasattr(primary, "add_note"):
                primary.add_note("cleanup diagnostics: " + "; ".join(cleanup_errors))
            raise


def run_approved(
    root: Path, approval_path: Path, *, permit_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _run_approved_core(
        root, approval_path, permit_path=permit_path, state=_state_root(root),
        manager_key_path=MANAGER_PERMIT_KEY, expected_key_id=MANAGER_KEY_ID,
    )


def _run_approved_for_test(
    root: Path, approval_path: Path, *, permit_path: Path, state_root: Path,
    manager_key_path: Path,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    state = _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN)
    return _run_approved_core(
        root, approval_path, permit_path=permit_path, state=state,
        manager_key_path=manager_key_path, expected_key_id=_digest_file(manager_key_path),
    )


def _run_locked(
    root: Path, approval_path: Path, approval: dict[str, Any],
    permit: Mapping[str, Any], permit_file: Path, state: Path, run_dir: Path,
) -> dict[str, Any]:
    _recover_interrupted(root, state)
    _scavenge_disposable_worktrees(root, state)
    maintenance_errors = _retry_retention_maintenance(state)
    if maintenance_errors:
        raise CrackHarnessError(
            "retained cleanup/maintenance remains incomplete: "
            + "; ".join(maintenance_errors)
        )
    if (state / "RECOVERY_REQUIRED.json").exists():
        raise CrackHarnessError("recorded interrupted winner requires manager recovery review")
    readiness = _dry_run_core(root, approval_path, state)
    if readiness["status"] != "ready":
        raise CrackHarnessError("; ".join(readiness["blockers"]))
    _verify_repository(root, approval, allow_source=False)
    for disposable in (approval["_paths"]["base"], approval["_paths"]["candidate"], approval["_approval_path"], permit_file):
        if _is_tracked(root, disposable):
            raise CrackHarnessError(f"disposable approval artifact is tracked and cannot be deleted: {disposable}")
    if _function_consumed(run_dir, approval):
        raise CrackHarnessError("function already consumed its one lifetime cell")
    if run_dir.exists():
        _assert_no_indirection(run_dir)
        shutil.rmtree(run_dir)
    attempt_body = {
        "schema": "crack_harness_attempt/v1",
        "run_dir": str(run_dir),
        "source_path": str(approval["_paths"]["source"]),
        "approval_path": str(approval["_approval_path"]),
        "approval_sha256": approval["_approval_sha256"],
        "disposable_paths": [
            str(approval["_paths"]["base"]), str(approval["_paths"]["candidate"]),
            str(permit_file), str(approval["_approval_path"]),
        ],
    }
    _atomic_json(state / "attempt.json", {
        **attempt_body, "attempt_sha256": _digest_json(attempt_body)
    })
    _consume_function(run_dir, approval)
    _safe_mkdir(run_dir)
    temp = run_dir / "temp"
    _safe_mkdir(temp)
    worktree = temp / "worktree"
    out_root = temp / "out"
    _safe_mkdir(out_root)
    paths = approval["_paths"]
    context_body = {
        "schema": "crack_evidence_context/v1", "owner": approval["owner"],
        "function": approval["function"], "unit": approval["unit"],
        "source_relpath": paths["source"].relative_to(root).as_posix(),
        "target_sha256": approval["target_sha256"],
        "base_source_sha256": approval["base"]["sha256"],
        "candidate_source_sha256": approval["candidate"]["sha256"],
        "base_commit": approval["base_commit"],
        "approval_sha256": approval["_approval_sha256"],
        "toolchain_key": approval["toolchain_key"], "issued_at": approval["issued_at"],
        "objdiff": OBJDIFF_PIN,
    }
    context = {**context_body, "context_sha256": _digest_json(context_body)}
    context_path = out_root / "approval-context.json"
    _atomic_json(context_path, context)
    remaining_assignment = (
        _timestamp(permit["deadline"], "permit deadline") - datetime.now(timezone.utc)
    ).total_seconds()
    if remaining_assignment <= 0:
        raise CrackHarnessError("assignment permit deadline elapsed before compile")
    deadline = time.monotonic() + min(
        approval["limits"]["active_seconds"], remaining_assignment
    )
    receipts: dict[str, Any] = {}
    proof_payloads: dict[str, Any] = {}
    status = "failed"
    reason = "unclassified failure"
    assessment: dict[str, Any] = {}
    proof_exact: dict[str, bool] = {}
    object_pair: tuple[str, str] | None = None
    admission_token = ""
    admission_input_key = ""
    admission_closed = False
    source_replaced = False
    active_hook = "precompile"
    secondary_failures: list[str] = []
    primary_exception: BaseException | None = None
    try:
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        admission, receipt = _run_command(
            approval["commands"]["precompile"]["argv"], root=root, run_temp=temp,
            deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
            expect_json=True
        )
        assert admission is not None
        receipts["precompile"] = _compact_receipt("precompile", admission, receipt)
        admission_fields = {
            "status", "reused", "skip_compile", "input_key", "admission_token",
            "expires_at", "authority_advanced",
        }
        if set(admission) != admission_fields or admission.get("status") != "admitted" or admission.get("skip_compile") is not False or admission.get("authority_advanced") is not False:
            raise CrackHarnessError(f"canonical precompile admission denied: {admission.get('status')}")
        admission_token = _text(admission.get("admission_token"), "precompile admission_token")
        admission_input_key = _sha(admission.get("input_key"), "precompile input_key")
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        _create_disposable_worktree(root, worktree, approval["base_commit"])
        if _tree_size(temp) > approval["limits"]["temporary_bytes"]:
            raise CrackHarnessError("disposable worktree exceeds the hard ephemeral-storage limit")
        worktree_source = worktree / paths["source"].relative_to(root)
        if _digest_file(worktree_source) != approval["base"]["sha256"]:
            raise CrackHarnessError("disposable worktree source does not equal sealed baseline")
        lock = state / ".serialized-compile.lock"
        with serialized_build_lock(lock, min(55.0, max(0.1, deadline - time.monotonic()))):
            active_hook = "compile"
            compile_argv = _expand_argv(
                approval["commands"]["compile"]["argv"], worktree, out_root, root
            )
            evidence_env = {
                "CRACK_HARNESS_OUT_ROOT": str(out_root),
                "CRACK_HARNESS_OWNER": approval["owner"],
                "CRACK_HARNESS_FUNCTION": approval["function"],
                "CRACK_HARNESS_UNIT": approval["unit"],
                "CRACK_HARNESS_SOURCE_PATH": paths["source"].relative_to(root).as_posix(),
                "CRACK_HARNESS_TARGET_SHA256": approval["target_sha256"],
                "CRACK_HARNESS_BASE_COMMIT": approval["base_commit"],
                "CRACK_HARNESS_APPROVAL_SHA256": approval["_approval_sha256"],
                "CRACK_HARNESS_CONTEXT_PATH": str(context_path),
                "CRACK_HARNESS_CONTEXT_SHA256": context["context_sha256"],
                "CRACK_HARNESS_ISSUED_AT": approval["issued_at"],
            }
            _clear_evidence(out_root, (*EVIDENCE_BASELINE_FILES, *EVIDENCE_CANDIDATE_FILES, "evidence-context.json"))
            baseline_env = {
                **evidence_env, "CRACK_HARNESS_PHASE": "baseline",
                "CRACK_HARNESS_PHASE_NONCE": _digest_bytes((context["context_sha256"] + ":baseline").encode()),
            }
            _, baseline_receipt = _run_command(
                compile_argv, root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=False, production_root=root, state_root=state,
                extra_env=baseline_env,
            )
            baseline_hashes = _evidence_hashes(out_root, EVIDENCE_BASELINE_FILES)
            sealed_baseline_receipt = _validate_evidence_receipt(
                out_root, approval, context, "baseline"
            )
            _validate_evidence_context(out_root, approval, context, completed=False)
            if _digest_file(out_root / "target.o") != approval["target_sha256"]:
                raise CrackHarnessError("baseline evidence target.o is not the approved target")
            if any((out_root / name).exists() for name in EVIDENCE_CANDIDATE_FILES):
                raise CrackHarnessError("baseline evidence phase emitted candidate outputs")
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            _atomic_copy(paths["candidate"], worktree_source)
            _clear_evidence(out_root, EVIDENCE_CANDIDATE_FILES)
            candidate_env = {
                **evidence_env, "CRACK_HARNESS_PHASE": "candidate",
                "CRACK_HARNESS_PHASE_NONCE": _digest_bytes((context["context_sha256"] + ":candidate").encode()),
            }
            _, candidate_receipt = _run_command(
                compile_argv, root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=False, production_root=root, state_root=state,
                extra_env=candidate_env,
            )
            if _evidence_hashes(out_root, EVIDENCE_BASELINE_FILES) != baseline_hashes:
                raise CrackHarnessError("candidate evidence phase changed sealed baseline outputs")
            _evidence_hashes(out_root, EVIDENCE_CANDIDATE_FILES)
            if _validate_evidence_receipt(out_root, approval, context, "baseline") != sealed_baseline_receipt:
                raise CrackHarnessError("candidate evidence phase changed the baseline receipt")
            _validate_evidence_receipt(out_root, approval, context, "candidate")
            _validate_evidence_context(out_root, approval, context, completed=True)
            receipts["compile"] = {
                "schema": "crack_harness_receipt/v1", "hook": "compile",
                "baseline_command": baseline_receipt, "candidate_command": candidate_receipt,
            }
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            for name in HOOKS:
                active_hook = name
                payload, command_receipt = _run_command(
                    _expand_argv(approval["commands"][name]["argv"], worktree, out_root, root), root=worktree, run_temp=temp,
                    deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                    expect_json=True, production_root=root, state_root=state
                )
                assert payload is not None
                typed, object_pair, is_exact = _validate_proof(name, payload, approval, object_pair)
                receipts[name] = _compact_receipt(name, payload, command_receipt)
                proof_payloads[name] = typed
                proof_exact[name] = is_exact
                _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            active_hook = "assess"
            payload, command_receipt = _run_command(
                _expand_argv(approval["commands"]["assess"]["argv"], worktree, out_root, root), root=worktree, run_temp=temp,
                deadline=deadline, storage_limit=approval["limits"]["temporary_bytes"],
                expect_json=True, production_root=root, state_root=state
            )
            assert payload is not None
            assert object_pair is not None
            gain = _validate_assessment(payload, approval, object_pair)
            assessment = dict(payload)
            receipts["assess"] = _compact_receipt("assess", payload, command_receipt)
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
        assert object_pair is not None
        nonregression = (
            proof_exact.get("siblings") is True
            and proof_exact.get("physical") is True
            and float(assessment["data_gain"]) >= 0
            and assessment["data_diff_delta"] <= 0
            and proof_payloads["strict"]["target_bytes"] == proof_payloads["strict"]["candidate_bytes"]
            and proof_payloads["data"]["target_bytes"] == proof_payloads["data"]["candidate_bytes"]
        )
        exact = gain > 0 and nonregression and all(proof_exact.values())
        if exact:
            status = "exact"
            reason = "measurable owner gain and every approved proof passed"
        elif gain <= 0:
            status = "no_gain"
            reason = "candidate produced no measurable owner gain"
        elif not nonregression:
            status = "no_gain"
            reason = "positive focus gain rejected because a closed proof channel regressed"
        else:
            status = "improved"
            reason = "measurable owner gain retained; exact proof remains incomplete"
        if status in {"exact", "improved"}:
            baseline_snapshot = temp / "baseline.snapshot"
            _atomic_copy(paths["base"], baseline_snapshot)
            transaction_body = {
                "schema": "crack_harness_transaction/v1", "source": str(paths["source"]),
                "source_relpath": paths["source"].relative_to(root).as_posix(),
                "baseline_snapshot": str(baseline_snapshot), "baseline_sha256": approval["base"]["sha256"],
                "approval_sha256": approval["_approval_sha256"],
                "target_object_sha256": approval["target_sha256"],
                "candidate_sha256": approval["candidate"]["sha256"],
                "result_path": str(run_dir / "result.json"), "worktree": str(worktree),
                "record_commit_path": str(run_dir / "record.commit.json"),
                "central_record_binding": {
                    "input_key": admission_input_key,
                    "owner": approval["owner"],
                    "function": approval["function"],
                    "source_sha256": approval["candidate"]["sha256"],
                    "target_object_sha256": approval["target_sha256"],
                    "object_sha256": object_pair[1],
                    "candidate_record_sha256": _digest_json(assessment),
                    "status": status,
                },
            }
            transaction_body.pop("source")
            _atomic_json(state / "transaction.json", {
                **transaction_body, "transaction_sha256": _digest_json(transaction_body)
            })
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=False)
            _atomic_copy(worktree_source, paths["source"])
            source_replaced = True
            _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=True)
        if status in {"exact", "improved"}:
            active_hook = "record"
            record_payload, record_command = _run_canonical_record(
                root, approval, status, object_pair, admission_token, admission_input_key,
                proof_payloads, assessment, run_temp=temp, deadline=deadline,
                storage_limit=approval["limits"]["temporary_bytes"],
            )
            _validate_record(record_payload, approval, status, object_pair, admission_token, admission_input_key)
            receipts["record"] = _compact_receipt("record", record_payload, record_command)
            admission_closed = True
            commit_body = {
                "schema": "crack_harness_record_commit/v1", "outcome": status,
                "candidate_sha256": approval["candidate"]["sha256"],
                "record_payload_sha256": receipts["record"]["payload_sha256"],
                "record_sha256": receipts["record"]["summary"]["record_sha256"],
            }
            _atomic_json(run_dir / "record.commit.json", {**commit_body, "commit_sha256": _digest_json(commit_body)})
        else:
            active_hook = "discard"
            discard_payload, discard_command = _run_canonical_discard(
                root, approval, admission_token, admission_input_key,
                run_temp=temp, deadline=deadline,
                storage_limit=approval["limits"]["temporary_bytes"],
            )
            receipts["discard"] = _compact_receipt("discard", discard_payload, discard_command)
            admission_closed = True
        _checkpoint(root, approval["_approval_path"], approval, permit_file, permit, state, allow_source=source_replaced)
    except (Exception, KeyboardInterrupt) as exc:
        primary_exception = exc
        reason = str(exc)
        status = "failed"
        descriptor = approval["commands"].get(active_hook, {})
        receipts["failure"] = {
            "schema": "crack_harness_failure_receipt/v1",
            "hook": active_hook,
            "reason": reason[:1000],
            "argv_sha256": _digest_json(descriptor.get("argv", [])),
        }
        if source_replaced:
            try:
                _atomic_copy(paths["base"], paths["source"])
                source_replaced = False
            except BaseException as rollback_exc:
                secondary_failures.append(f"source rollback: {rollback_exc}")
        if admission_token and not admission_closed:
            try:
                discard_payload, discard_command = _run_canonical_discard(
                    root, approval, admission_token, admission_input_key,
                    run_temp=temp, deadline=deadline,
                    storage_limit=approval["limits"]["temporary_bytes"],
                )
                receipts["discard"] = _compact_receipt("discard", discard_payload, discard_command)
                admission_closed = True
            except BaseException as discard_exc:
                secondary_failures.append(f"admission discard: {discard_exc}")
                receipts["discard_failure"] = {
                    "schema": "crack_harness_failure_receipt/v1", "hook": "discard",
                    "reason": str(discard_exc)[:1000],
                }
    try:
        _checkpoint(
            root, approval["_approval_path"], approval, permit_file, permit, state,
            allow_source=source_replaced,
        )
    except BaseException as checkpoint_exc:
        if status != "failed":
            raise
        secondary_failures.append(f"terminal checkpoint: {checkpoint_exc}")
    if secondary_failures:
        receipts["secondary_failures"] = {
            "schema": "crack_harness_secondary_failures/v1",
            "items": [item[:1000] for item in secondary_failures[:8]],
        }
    if status == "failed" and source_replaced:
        assert primary_exception is not None
        if hasattr(primary_exception, "add_note"):
            primary_exception.add_note("; ".join(secondary_failures))
        raise primary_exception
    finished = _now()
    result_body = {
        "schema": RESULT_SCHEMA,
        "approval_id": approval["approval_id"],
        "approval_sha256": approval["_approval_sha256"],
        "owner": approval["owner"],
        "task_id": approval["task_id"],
        "function": approval["function"],
        "base_commit": approval["base_commit"],
        "campaign_id": approval["campaign"]["id"],
        "candidate_sha256": approval["candidate"]["sha256"],
        "base_sha256": approval["base"]["sha256"],
        "status": status,
        "reason": reason,
        "owner_gain": assessment.get("owner_gain"),
        "predicted_rows": approval["predicted_rows"],
        "receipts": receipts,
        "finished_at": finished,
        "source_restored": status not in {"exact", "improved"} and not source_replaced,
        "cleanup_status": "pending",
        "cleanup_errors": [],
        "authority_advanced": False,
    }
    result = {**result_body, "result_sha256": _digest_json(result_body)}
    if len(_canonical(result)) > MAX_COMPACT_TERMINAL_BYTES:
        raise CrackHarnessError("terminal result exceeds the hard 1 MiB compact cap")
    if status == "improved":
        _atomic_json(run_dir / "result.json", result)
    if status == "exact":
        report_body = {
            "schema": REPORT_SCHEMA,
            "status": "exact",
            "completed": True,
            "authority_advanced": False,
            "owner": approval["owner"],
            "function": approval["function"],
            "task_id": approval["task_id"],
            "base_commit": approval["base_commit"],
            "approval_sha256": approval["_approval_sha256"],
            "source_sha256": approval["candidate"]["sha256"],
            "target_object_sha256": object_pair[0],
            "candidate_object_sha256": object_pair[1],
            "result": {
                "strict_percent": proof_payloads["strict"]["strict_percent"],
                "data_percent": proof_payloads["data"]["data_percent"],
                "target_bytes": proof_payloads["strict"]["target_bytes"],
                "candidate_bytes": proof_payloads["strict"]["candidate_bytes"],
                "owner_gain": assessment["owner_gain"],
            },
            "proof_receipts": {
                name: {
                    "sha256": receipt["payload_sha256"],
                    "summary": receipt["summary"],
                }
                for name, receipt in receipts.items()
                if "payload_sha256" in receipt
            },
            "predicted_rows": approval["predicted_rows"],
            "completed_at": finished,
        }
        report = {**report_body, "report_sha256": _digest_json(report_body)}
        if len(_canonical(report)) > MAX_COMPACT_TERMINAL_BYTES:
            raise CrackHarnessError("exact report exceeds the hard 1 MiB compact cap")
        result["report_sha256"] = report["report_sha256"]
        unhashed = dict(result)
        unhashed.pop("result_sha256", None)
        result["result_sha256"] = _digest_json(unhashed)
        record_commit = _record_commit_value(run_dir / "record.commit.json")
        if record_commit is None:
            raise CrackHarnessError("exact central record commit is missing or invalid")
        _validate_exact_report(
            report,
            result,
            {
                "owner": approval["owner"],
                "function": approval["function"],
                "source_sha256": approval["candidate"]["sha256"],
                "target_object_sha256": approval["target_sha256"],
                "object_sha256": object_pair[1],
                "status": "exact",
            },
            record_commit=record_commit,
        )
        _atomic_json(run_dir / "CRACK_REPORT_v1.json", report)
        _atomic_json(run_dir / "result.json", result)
    journal = state / "transaction.json"
    cleanup_errors: list[str] = []

    def secondary(label: str, callback: Any) -> None:
        try:
            callback()
        except BaseException as exc:
            cleanup_errors.append(f"{label}: {exc}"[:1000])

    if status in {"exact", "improved"}:
        if journal.exists():
            secondary("transaction journal cleanup", journal.unlink)
        if journal.exists():
            cleanup_errors.append(
                "transaction journal remains; disposable cleanup deferred"[:1000]
            )
        else:
            secondary(
                "record commit cleanup",
                lambda: (run_dir / "record.commit.json").unlink(missing_ok=True),
            )
            if worktree.exists():
                secondary(
                    "disposable worktree cleanup",
                    lambda: _remove_disposable_worktree(root, worktree),
                )
            secondary("raw/temp cleanup", lambda: _cleanup_raw(run_dir))
            for disposable in (
                paths["base"], paths["candidate"], permit_file,
                approval["_approval_path"],
            ):
                secondary(
                    f"delete {disposable.name}",
                    lambda item=disposable: item.unlink(missing_ok=True),
                )
            secondary(
                "delete attempt receipt",
                lambda: (state / "attempt.json").unlink(missing_ok=True),
            )
        secondary(
            "owner retention maintenance",
            lambda: _gc_owner(
                run_dir, MAX_RETAINED_OWNER_BYTES,
                protected={run_dir.parent},
            ),
        )
        secondary(
            "global retention maintenance",
            lambda: _gc_global(
                state, MAX_RETAINED_GLOBAL_BYTES,
                protected={run_dir.parent},
            ),
        )
        terminal_body = dict(result)
        terminal_body.pop("result_sha256", None)
        terminal_body["cleanup_status"] = (
            "cleanup_incomplete" if cleanup_errors else "complete"
        )
        terminal_body["cleanup_errors"] = cleanup_errors[:8]
        result = {**terminal_body, "result_sha256": _digest_json(terminal_body)}
        try:
            _atomic_json(run_dir / "result.json", result)
        except BaseException as exc:
            cleanup_errors.append(f"seal cleanup metadata: {exc}"[:1000])
            terminal_body["cleanup_status"] = "cleanup_incomplete"
            terminal_body["cleanup_errors"] = cleanup_errors[:8]
            result = {
                **terminal_body, "result_sha256": _digest_json(terminal_body)
            }
        return result

    if status == "failed":
        if journal.exists():
            secondary("transaction journal cleanup", journal.unlink)
        secondary(
            "record commit cleanup",
            lambda: (run_dir / "record.commit.json").unlink(missing_ok=True),
        )
        if worktree.exists():
            secondary(
                "failed disposable worktree cleanup",
                lambda: _remove_disposable_worktree(root, worktree),
            )
        secondary("failed raw/temp cleanup", lambda: _cleanup_raw(run_dir))
        for disposable in (
            paths["base"], paths["candidate"], permit_file,
            approval["_approval_path"],
        ):
            secondary(
                f"failed delete {disposable.name}",
                lambda item=disposable: item.unlink(missing_ok=True),
            )
        secondary(
            "failed delete attempt receipt",
            lambda: (state / "attempt.json").unlink(missing_ok=True),
        )
        if run_dir.exists():
            secondary(
                "failed run directory cleanup",
                lambda: (_assert_no_indirection(run_dir), shutil.rmtree(run_dir)),
            )
        secondary(
            "failed owner retention maintenance",
            lambda: _gc_owner(run_dir, MAX_RETAINED_OWNER_BYTES),
        )
        secondary(
            "failed global retention maintenance",
            lambda: _gc_global(state, MAX_RETAINED_GLOBAL_BYTES),
        )
        diagnostic_body = {
            "schema": "crack_harness_failure_diagnostic/v1",
            "approval_sha256": approval["_approval_sha256"],
            "owner": approval["owner"], "function": approval["function"],
            "primary_reason": reason[:1000],
            "cleanup_errors": cleanup_errors[:8], "finished_at": finished,
        }
        try:
            _atomic_json(run_dir.parent / "latest-failure.json", {
                **diagnostic_body,
                "diagnostic_sha256": _digest_json(diagnostic_body),
            })
        except BaseException as exc:
            # Never replace the primary failure even if durable diagnostics are
            # temporarily unwritable; the returned sealed failure remains primary.
            cleanup_errors.append(f"failure diagnostic seal: {exc}"[:1000])
        return result

    if journal.exists():
        journal.unlink()
    (run_dir / "record.commit.json").unlink(missing_ok=True)
    try:
        if worktree.exists():
            _remove_disposable_worktree(root, worktree)
        _cleanup_raw(run_dir)
    except Exception as exc:
        cleanup_errors.append(str(exc)[:1000])
    finally:
        for disposable in (paths["base"], paths["candidate"], permit_file, approval["_approval_path"]):
            try:
                if disposable.exists():
                    disposable.unlink()
            except Exception as exc:
                cleanup_errors.append(f"delete {disposable.name}: {exc}"[:1000])
        try:
            (state / "attempt.json").unlink(missing_ok=True)
        except Exception as exc:
            cleanup_errors.append(f"delete attempt receipt: {exc}"[:1000])
    if cleanup_errors:
        raise CrackHarnessError("terminal cleanup failed: " + "; ".join(cleanup_errors))
    if status == "no_gain" and run_dir.exists():
        _assert_no_indirection(run_dir)
        shutil.rmtree(run_dir)
    _gc_owner(run_dir, MAX_RETAINED_OWNER_BYTES)
    _gc_global(state, MAX_RETAINED_GLOBAL_BYTES)
    return result


def _status_core(root: Path, state: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    results = []
    if state.exists():
        with serialized_build_lock(state / ".transaction.lock", 55.0):
            _recover_interrupted(root, state)
            _scavenge_disposable_worktrees(root, state)
            _retry_retention_maintenance(state)
        for path in state.glob("owners/*/*/latest/result.json"):
            try:
                value = _read_json(path)
            except CrackHarnessError:
                continue
            if isinstance(value, Mapping):
                results.append(dict(value))
    return {
        "schema": "crack_harness_status/v1",
        "state_root": str(state),
        "results": sorted(results, key=lambda item: str(item.get("finished_at", ""))),
        "temporary_bytes": _tree_size(state) if state.exists() else 0,
        "global_stop": (state / "STOP").is_file(),
        "interrupted_transaction": (state / "transaction.json").is_file(),
        "authority_advanced": False,
    }


def status(root: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _status_core(root, _state_root(root))


def _status_for_test(root: Path, *, state_root: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    return _status_core(root, _state_root(root, state_root, _test_token=_TEST_STATE_TOKEN))


def add_crack_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser("crack", help="run one approved bounded crack cell")
    commands = parser.add_subparsers(dest="crack_command", required=True)
    for name in ("dry-run", "run"):
        command = commands.add_parser(name)
        command.add_argument("--approval", required=True)
        if name != "dry-run":
            command.add_argument("--permit", required=True)
    status_parser = commands.add_parser("status")
    return parser


def _add_proof_adapter_parser(subparsers: Any) -> None:
    parser = subparsers.add_parser("proof-adapter", help=argparse.SUPPRESS)
    parser.add_argument("--kind", required=True, choices=[*HOOKS, "assess"])
    parser.add_argument("--owner", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--candidate-source", required=True, type=Path)
    parser.add_argument("--candidate-source-sha256", required=True)
    parser.add_argument("--approved-target-object-sha256", required=True)
    parser.add_argument("--target-object", required=True, type=Path)
    parser.add_argument("--candidate-object", required=True, type=Path)
    parser.add_argument("--baseline-strict-report", required=True, type=Path)
    parser.add_argument("--baseline-data-report", required=True, type=Path)
    parser.add_argument("--candidate-strict-report", required=True, type=Path)
    parser.add_argument("--candidate-data-report", required=True, type=Path)
    parser.add_argument("--physical-receipt", required=True, type=Path)


def run_crack_command(args: argparse.Namespace, *, root: Path) -> int:
    if args.crack_command == "status":
        value = status(root)
    elif args.crack_command == "dry-run":
        value = dry_run(root, Path(args.approval))
    else:
        value = run_approved(
            root, Path(args.approval),
            permit_path=Path(args.permit)
        )
    print(json.dumps(value, indent=2, sort_keys=True))
    if value.get("status") in {"no_gain", "improved"}:
        print("PIVOT_REQUIRED", file=sys.stderr)
    return 2 if value.get("status") in {"failed", "blocked", "no_gain", "improved"} else 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    subparsers = parser.add_subparsers(dest="command", required=True)
    add_crack_parser(subparsers)
    _add_proof_adapter_parser(subparsers)
    args = parser.parse_args(argv)
    try:
        if args.command == "proof-adapter":
            value = _proof_adapter_payload(
                kind=args.kind, owner=args.owner, function=args.function,
                candidate_source=args.candidate_source,
                candidate_source_sha256=args.candidate_source_sha256,
                approved_target_object_sha256=args.approved_target_object_sha256,
                target_object=args.target_object, candidate_object=args.candidate_object,
                baseline_strict_report=args.baseline_strict_report,
                baseline_data_report=args.baseline_data_report,
                candidate_strict_report=args.candidate_strict_report,
                candidate_data_report=args.candidate_data_report,
                physical_receipt=args.physical_receipt,
            )
            print(_canonical(value).decode("utf-8"))
            return 0
        return run_crack_command(args, root=Path(args.root).resolve())
    except CrackHarnessError as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
