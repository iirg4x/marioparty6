#!/usr/bin/env python3
"""Materialize one current-base residual census for a recovery owner.

This is deliberately a baseline-only adapter.  It rebuilds the selected
unit from the current source exactly once under the worktree compiler lock,
compares that object with the authenticated retail object, and publishes a
small current-residual binding.  It never creates a candidate, permit,
approval, history directory, or authority-bearing record.

The primary artifact intentionally uses the closed
``crack_current_residual_evidence/v1`` contract consumed by the crack harness.
The closed core binds a compact focus report, a self-hashed physical summary,
and retained target/base objects.  The admissible core is published last, only
after all five files and every object/report binding have been checked.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import signal
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    import sys

    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import crack_evidence_bundle as bundle
from tools import focus_symbol_report
from tools.crack_contract import is_closed_objdiff_unit_name
from tools.recovery_pass import serialized_build_lock


SCHEMA = "crack_current_residual_evidence/v1"
PHYSICAL_SUMMARY_SCHEMA = "crack_current_residual_physical_summary/v1"
RESULT_SCHEMA = "crack_current_residual_materialization/v1"
SHA_RE = re.compile(r"^[0-9a-f]{64}$")
FUNCTION_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
MAX_ARTIFACT_BYTES = 512 * 1024
MAX_PHYSICAL_SUMMARY_BYTES = 128 * 1024
MAX_RESIDUAL_ROWS = 8192
MAX_FOCUS_BYTES = 512 * 1024
MAX_OBJECT_BYTES = 16 * 1024 * 1024
DEFAULT_PROCESS_TIMEOUT = 120.0
PROCESS_TERMINATION_GRACE = 5.0
FOCUS_CONTEXT_RADIUS = 2


class ResidualEvidenceError(ValueError):
    """The current-base census could not be proven without guessing."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _json_sha(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _file_sha(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as handle:
            for chunk in iter(lambda: handle.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise ResidualEvidenceError(f"cannot read {path}: {exc}") from exc
    return digest.hexdigest()


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise ResidualEvidenceError(f"{label} must be a lowercase SHA-256")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ResidualEvidenceError(f"{label} must be nonempty text")
    return value.strip()


def _inside(root: Path, path: Path, label: str) -> Path:
    root_absolute = Path(os.path.abspath(root))
    raw = Path(os.path.expanduser(os.fspath(path)))
    lexical = Path(os.path.abspath(raw if raw.is_absolute() else root_absolute / raw))
    try:
        lexical.relative_to(root_absolute)
    except ValueError as exc:
        raise ResidualEvidenceError(f"{label} escapes repository root: {lexical}") from exc
    try:
        bundle._assert_no_indirection(lexical, missing_leaf=not lexical.exists())
    except (bundle.EvidenceError, OSError) as exc:
        raise ResidualEvidenceError(f"{label} path is not canonical: {lexical}: {exc}") from exc
    return lexical


def _regular_file(root: Path, path: Path, label: str) -> Path:
    value = _inside(root, path, label)
    try:
        bundle._assert_no_indirection(value)
    except (bundle.EvidenceError, OSError) as exc:
        raise ResidualEvidenceError(f"{label} path is not canonical: {value}: {exc}") from exc
    if not value.is_file():
        raise ResidualEvidenceError(f"{label} is missing or not a regular file: {value}")
    return value.resolve()


def _root(root: Path) -> Path:
    value = Path(os.path.abspath(root))
    try:
        bundle._assert_no_indirection(value)
    except (bundle.EvidenceError, OSError) as exc:
        raise ResidualEvidenceError(f"repository root is not canonical: {value}: {exc}") from exc
    if not value.is_dir() or not (value / ".git").exists():
        raise ResidualEvidenceError(f"repository root is not a Git worktree: {value}")
    return value.resolve()


def _function_span_sha(source: Path, start_line: int, end_line: int) -> str:
    if type(start_line) is not int or type(end_line) is not int:
        raise ResidualEvidenceError("function span lines must be integers")
    if start_line < 1 or end_line < start_line:
        raise ResidualEvidenceError("function span must be a nonempty inclusive range")
    try:
        lines = source.read_bytes().splitlines(keepends=True)
    except OSError as exc:
        raise ResidualEvidenceError(f"cannot read source span {source}: {exc}") from exc
    if end_line > len(lines):
        raise ResidualEvidenceError(
            f"function span ends at line {end_line}, source has {len(lines)} lines"
        )
    return hashlib.sha256(b"".join(lines[start_line - 1 : end_line])).hexdigest()


def _descriptor(path: Path, root: Path) -> dict[str, Any]:
    try:
        relative = path.resolve().relative_to(root.resolve()).as_posix()
        size = path.stat().st_size
    except (OSError, ValueError) as exc:
        raise ResidualEvidenceError(f"cannot describe {path}: {exc}") from exc
    return {"path": relative, "sha256": _file_sha(path), "size_bytes": size}


def _unit(root: Path, unit: str) -> tuple[Mapping[str, Any], Path, Path]:
    if not is_closed_objdiff_unit_name(unit):
        raise ResidualEvidenceError(f"unit is not a closed objdiff unit name: {unit!r}")
    config_path = _regular_file(root, root / "objdiff.json", "objdiff config")
    try:
        config = json.loads(config_path.read_bytes())
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ResidualEvidenceError(f"invalid objdiff config {config_path}: {exc}") from exc
    if not isinstance(config, Mapping) or not isinstance(config.get("units"), list):
        raise ResidualEvidenceError("objdiff config units must be an array")
    matches = [
        item for item in config["units"]
        if isinstance(item, Mapping) and item.get("name") == unit
    ]
    if len(matches) != 1:
        raise ResidualEvidenceError(f"objdiff unit {unit!r} resolved {len(matches)} times")
    row = matches[0]
    target_raw, base_raw = row.get("target_path"), row.get("base_path")
    if not isinstance(target_raw, str) or not isinstance(base_raw, str):
        raise ResidualEvidenceError("selected objdiff unit lacks target_path/base_path")
    target = _regular_file(root, root / target_raw, "target object")
    base = _inside(root, root / base_raw, "base object")
    try:
        bundle._assert_no_indirection(base, missing_leaf=True)
    except (bundle.EvidenceError, OSError) as exc:
        raise ResidualEvidenceError(f"base object path is not canonical: {base}: {exc}") from exc
    if target == base:
        raise ResidualEvidenceError("objdiff target and base object paths are identical")
    return row, target, base


def _instruction_address(row: Any) -> str:
    if not isinstance(row, Mapping):
        return "-"
    instruction = row.get("instruction")
    if isinstance(instruction, Mapping):
        value = instruction.get("address")
    else:
        value = row.get("address")
    if value is None:
        return "-"
    return str(value)


def _focus_symbol(document: Mapping[str, Any], side: str, function: str) -> Mapping[str, Any]:
    side_value = document.get(side)
    if not isinstance(side_value, Mapping) or not isinstance(side_value.get("symbols"), list):
        raise ResidualEvidenceError(f"objdiff report.{side}.symbols is not an array")
    matches = [
        item for item in side_value["symbols"]
        if isinstance(item, Mapping) and item.get("name") == function
        and item.get("kind") == "SYMBOL_FUNCTION"
    ]
    if len(matches) != 1:
        raise ResidualEvidenceError(
            f"objdiff report {side} contains {len(matches)} focus symbols named {function!r}"
        )
    if not isinstance(matches[0].get("instructions"), list):
        raise ResidualEvidenceError(f"objdiff report {side} focus has no instructions")
    return matches[0]


def _diff_row_ids(
    document: Mapping[str, Any], function: str, channel: str
) -> list[str]:
    target = _focus_symbol(document, "left", function)
    candidate = _focus_symbol(document, "right", function)
    target_rows = target["instructions"]
    candidate_rows = candidate["instructions"]
    result: list[str] = []
    for index in range(max(len(target_rows), len(candidate_rows))):
        target_row = target_rows[index] if index < len(target_rows) else None
        candidate_row = candidate_rows[index] if index < len(candidate_rows) else None
        target_kind = target_row.get("diff_kind") if isinstance(target_row, Mapping) else None
        candidate_kind = candidate_row.get("diff_kind") if isinstance(candidate_row, Mapping) else None
        if not (isinstance(target_kind, str) and target_kind) and not (
            isinstance(candidate_kind, str) and candidate_kind
        ):
            continue
        kinds = sorted(
            {
                value
                for value in (target_kind, candidate_kind)
                if isinstance(value, str) and value
            }
        )
        result.append(
            f"{channel}:{function}:row:{index}:kind={'+'.join(kinds)}:"
            f"target={_instruction_address(target_row)}:"
            f"candidate={_instruction_address(candidate_row)}"
        )
    return result


def residual_row_ids(
    strict_document: Mapping[str, Any], data_document: Mapping[str, Any], function: str
) -> list[str]:
    """Return stable, aligned residual row IDs from both objdiff channels."""

    if not isinstance(strict_document, Mapping) or not isinstance(data_document, Mapping):
        raise ResidualEvidenceError("strict and data reports must be JSON objects")
    values: list[str] = []
    seen: set[str] = set()
    for channel, document in (("strict", strict_document), ("data", data_document)):
        for row in _diff_row_ids(document, function, channel):
            if row not in seen:
                seen.add(row)
                values.append(row)
    if len(values) > MAX_RESIDUAL_ROWS:
        raise ResidualEvidenceError(
            f"current residual has too many rows ({len(values)} > {MAX_RESIDUAL_ROWS})"
        )
    return values


def physical_row_ids(receipt: Mapping[str, Any], function: str) -> list[str]:
    """Return stable IDs for every independent physical-relocation mismatch."""

    differences = receipt.get("physical_relocation_differences")
    if not isinstance(differences, list):
        raise ResidualEvidenceError("physical receipt differences must be an array")
    return [
        f"physical:{function}:row:{index}:sha256={_json_sha(value)}"
        for index, value in enumerate(differences)
    ]


def focus_residual_row_ids(focus: Mapping[str, Any], function: str) -> list[str]:
    """Reconstruct the canonical residual census from the published focus payload."""

    channels = focus.get("channels")
    if not isinstance(channels, Mapping):
        raise ResidualEvidenceError("focus artifact channels are missing")
    values: list[str] = []
    seen: set[str] = set()
    for channel in ("strict", "data"):
        material = channels.get(channel)
        if not isinstance(material, Mapping):
            raise ResidualEvidenceError(f"focus artifact {channel} channel is missing")
        target = material.get("target")
        candidate = material.get("candidate")
        if not isinstance(target, Mapping) or not isinstance(candidate, Mapping):
            raise ResidualEvidenceError(f"focus artifact {channel} sides are missing")
        target_rows = target.get("rows")
        candidate_rows = candidate.get("rows")
        if not isinstance(target_rows, list) or not isinstance(candidate_rows, list):
            raise ResidualEvidenceError(f"focus artifact {channel} rows are missing")

        def indexed(rows: list[Any], side: str) -> dict[int, Mapping[str, Any]]:
            result: dict[int, Mapping[str, Any]] = {}
            for position, row in enumerate(rows):
                if not isinstance(row, Mapping):
                    raise ResidualEvidenceError(
                        f"focus artifact {channel}.{side}.rows[{position}] is invalid"
                    )
                index = row.get("index")
                if isinstance(index, bool) or not isinstance(index, int) or index < 0:
                    raise ResidualEvidenceError(
                        f"focus artifact {channel}.{side}.rows[{position}] has invalid index"
                    )
                if index in result:
                    raise ResidualEvidenceError(
                        f"focus artifact {channel}.{side}.rows has duplicate index {index}"
                    )
                result[index] = row
            return result

        target_by_index = indexed(target_rows, "target")
        candidate_by_index = indexed(candidate_rows, "candidate")
        for index in sorted(set(target_by_index) | set(candidate_by_index)):
            target_row = target_by_index.get(index)
            candidate_row = candidate_by_index.get(index)
            target_kind = target_row.get("diff_kind") if isinstance(target_row, Mapping) else None
            candidate_kind = candidate_row.get("diff_kind") if isinstance(candidate_row, Mapping) else None
            if not (isinstance(target_kind, str) and target_kind) and not (
                isinstance(candidate_kind, str) and candidate_kind
            ):
                continue
            kinds = sorted(
                {
                    value
                    for value in (target_kind, candidate_kind)
                    if isinstance(value, str) and value
                }
            )
            row_id = (
                f"{channel}:{function}:row:{index}:kind={'+'.join(kinds)}:"
                f"target={_instruction_address(target_row)}:"
                f"candidate={_instruction_address(candidate_row)}"
            )
            if row_id not in seen:
                seen.add(row_id)
                values.append(row_id)

    physical = focus.get("physical_relocations")
    if not isinstance(physical, Mapping):
        raise ResidualEvidenceError("focus artifact physical relocations are missing")
    differences = physical.get("physical_relocation_differences")
    if not isinstance(differences, list):
        raise ResidualEvidenceError("focus artifact physical differences are missing")
    for row_id in physical_row_ids(
        {"physical_relocation_differences": differences}, function
    ):
        if row_id not in seen:
            seen.add(row_id)
            values.append(row_id)
    if len(values) > MAX_RESIDUAL_ROWS:
        raise ResidualEvidenceError(
            f"focus artifact has too many residual rows ({len(values)} > {MAX_RESIDUAL_ROWS})"
        )
    return values


def _closed_descriptor(
    value: Mapping[str, Any], label: str, *, path_required: bool
) -> dict[str, Any]:
    required = {"sha256", "size_bytes"} | ({"path"} if path_required else set())
    if not isinstance(value, Mapping) or set(value) != required:
        raise ResidualEvidenceError(f"{label} must be a strict bound descriptor")
    result = dict(value)
    _sha(result.get("sha256"), f"{label}.sha256")
    size = result.get("size_bytes")
    if isinstance(size, bool) or not isinstance(size, int) or size < 0:
        raise ResidualEvidenceError(f"{label}.size_bytes is invalid")
    if path_required:
        path = result.get("path")
        if (
            not isinstance(path, str)
            or not path
            or Path(path).is_absolute()
            or ".." in Path(path).parts
        ):
            raise ResidualEvidenceError(f"{label}.path must be a safe relative path")
    return result


def _compact_physical_summary(
    receipt: Mapping[str, Any],
    *,
    root: Path,
    owner: str,
    unit: str,
    function: str,
    target: Path,
    base: Path,
    strict: Path,
    data: Path,
) -> dict[str, Any]:
    target_row = receipt.get("target")
    candidate_row = receipt.get("candidate")
    if not isinstance(target_row, Mapping) or not isinstance(candidate_row, Mapping):
        raise ResidualEvidenceError("physical receipt lacks target/candidate summaries")
    differences = receipt.get("physical_relocation_differences")
    exact = receipt.get("physical_relocations_exact")
    if not isinstance(differences, list) or not isinstance(exact, bool):
        raise ResidualEvidenceError("physical receipt has invalid exactness fields")

    def integer(row: Mapping[str, Any], key: str) -> int:
        value = row.get(key)
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise ResidualEvidenceError(f"physical receipt {key} is invalid")
        return value

    body: dict[str, Any] = {
        "schema": PHYSICAL_SUMMARY_SCHEMA,
        "owner": owner,
        "unit": unit,
        "function": function,
        "target_object": _descriptor(target, root),
        "base_object": _descriptor(base, root),
        "strict_report": _descriptor(strict, root),
        "data_report": _descriptor(data, root),
        "target_size": integer(target_row, "size"),
        "base_size": integer(candidate_row, "size"),
        "target_instruction_count": integer(target_row, "instruction_count"),
        "base_instruction_count": integer(candidate_row, "instruction_count"),
        "target_physical_relocation_count": integer(target_row, "physical_relocation_count"),
        "base_physical_relocation_count": integer(candidate_row, "physical_relocation_count"),
        "physical_relocations_exact": exact,
        "physical_difference_count": len(differences),
        "physical_difference_sha256": _json_sha(differences),
        "authority_advanced": False,
    }
    body["physical_summary_sha256"] = _json_sha(body)
    return body


def build_residual_artifact(
    *,
    owner: str,
    function: str,
    base_sha256: str,
    source_sha256: str,
    target_sha256: str,
    base_commit: str,
    unit: str,
    start_line: int,
    end_line: int,
    base_span_sha256: str,
    toolchain_key: str,
    residual_rows: Sequence[str],
    base_object: Mapping[str, Any],
    target_object: Mapping[str, Any],
    focus_report: Mapping[str, Any],
    physical_summary: Mapping[str, Any],
    strict_report: Mapping[str, Any],
    data_report: Mapping[str, Any],
    physical_receipt: Mapping[str, Any],
    producer: Mapping[str, Any],
) -> dict[str, Any]:
    """Build and self-hash the closed harness artifact without I/O."""

    _text(owner, "owner")
    if not isinstance(function, str) or FUNCTION_RE.fullmatch(function) is None:
        raise ResidualEvidenceError("function must be a C identifier")
    for value, label in (
        (base_sha256, "base_sha256"), (source_sha256, "source_sha256"),
        (target_sha256, "target_sha256"), (base_span_sha256, "base_span_sha256"),
        (toolchain_key, "toolchain_key"),
    ):
        _sha(value, label)
    if base_sha256 != source_sha256:
        raise ResidualEvidenceError("base_sha256 and source_sha256 must bind the same current source")
    if re.fullmatch(r"[0-9a-f]{40}", base_commit) is None:
        raise ResidualEvidenceError("base_commit must be a full lowercase Git commit")
    if not is_closed_objdiff_unit_name(unit):
        raise ResidualEvidenceError("unit is not a closed objdiff unit name")
    if type(start_line) is not int or type(end_line) is not int or start_line < 1 or end_line < start_line:
        raise ResidualEvidenceError("function span is invalid")
    rows = list(residual_rows)
    if not rows or len(rows) > MAX_RESIDUAL_ROWS:
        raise ResidualEvidenceError("residual_rows must be a bounded nonempty array")
    if any(not isinstance(row, str) or not row.strip() for row in rows):
        raise ResidualEvidenceError("residual_rows must contain nonempty strings")
    if len(set(rows)) != len(rows):
        raise ResidualEvidenceError("residual_rows must be unique")
    body: dict[str, Any] = {
        "schema": SCHEMA,
        "owner": owner.strip(),
        "function": function,
        "base_commit": base_commit,
        "unit": unit,
        "base_sha256": base_sha256,
        "source_sha256": source_sha256,
        "target_sha256": target_sha256,
        "function_span": {
            "start_line": start_line,
            "end_line": end_line,
            "base_span_sha256": base_span_sha256,
        },
        "toolchain_key": toolchain_key,
        "base_object": _closed_descriptor(base_object, "base_object", path_required=True),
        "target_object": _closed_descriptor(target_object, "target_object", path_required=True),
        "focus_report": _closed_descriptor(focus_report, "focus_report", path_required=True),
        "physical_summary": _closed_descriptor(
            physical_summary, "physical_summary", path_required=True
        ),
        "strict_report": _closed_descriptor(
            strict_report, "strict_report", path_required=False
        ),
        "data_report": _closed_descriptor(data_report, "data_report", path_required=False),
        "physical_receipt": _closed_descriptor(
            physical_receipt, "physical_receipt", path_required=False
        ),
        "producer": _closed_descriptor(producer, "producer", path_required=True),
        "residual_rows": rows,
        "current_source_bound": True,
        "authority_advanced": False,
    }
    body["residual_sha256"] = _json_sha(body)
    rendered = _canonical(body) + b"\n"
    if len(rendered) > MAX_ARTIFACT_BYTES:
        raise ResidualEvidenceError(
            f"residual artifact exceeds {MAX_ARTIFACT_BYTES} bytes: {len(rendered)}"
        )
    return body


def _write_atomic_json(path: Path, value: Mapping[str, Any], limit: int) -> str:
    rendered = _canonical(value) + b"\n"
    if len(rendered) > limit:
        raise ResidualEvidenceError(f"artifact exceeds bounded size ({len(rendered)} > {limit})")
    try:
        bundle._assert_no_indirection(path.parent)
    except (bundle.EvidenceError, OSError) as exc:
        raise ResidualEvidenceError(f"cannot publish below indirect path {path.parent}: {exc}") from exc
    temporary: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            "wb", dir=path.parent, prefix=path.name, suffix=".tmp", delete=False
        ) as handle:
            temporary = Path(handle.name)
            handle.write(rendered)
            handle.flush()
            os.fsync(handle.fileno())
        bundle._assert_no_indirection(path.parent)
        # Do not replace a file that appeared after the initial output check.
        # A concurrent writer must turn this run into a fail-closed terminal
        # result rather than allowing one evidence record to overwrite it.
        if path.exists() or path.is_symlink():
            raise ResidualEvidenceError(f"atomic output appeared during publish: {path}")
        os.replace(temporary, path)
        temporary = None
    except (OSError, bundle.EvidenceError, ResidualEvidenceError) as exc:
        if temporary is not None:
            try:
                temporary.unlink(missing_ok=True)
            except OSError:
                pass
        raise ResidualEvidenceError(f"cannot atomically publish {path}: {exc}") from exc
    return _file_sha(path)


def _validate_output(root: Path, output: Path) -> Path:
    value = _inside(root, output, "output")
    parent = value.parent
    try:
        parent.mkdir(parents=True, exist_ok=True)
        bundle._assert_no_indirection(parent)
    except (OSError, bundle.EvidenceError) as exc:
        raise ResidualEvidenceError(f"output parent is not canonical: {parent}: {exc}") from exc
    if value.exists() or value.is_symlink():
        raise ResidualEvidenceError(f"output already exists; stale evidence is forbidden: {value}")
    return value


def _git(
    repository: Path,
    arguments: Sequence[str],
    label: str,
    *,
    timeout: float = DEFAULT_PROCESS_TIMEOUT,
) -> str:
    """Run a Git helper under the same bounded process policy."""

    return _run_bounded(
        ["git", *arguments], cwd=repository, label=label, timeout=timeout
    ).strip()


def _verify_base_commit(repository: Path, commit: str, *, timeout: float) -> None:
    """Verify one full object id without revision-suffix shell ambiguity.

    Some Windows/MSYS Git launchers rewrite braces in ``^{commit}`` even when
    Python supplies an argv vector directly.  A full 40-hex object id plus
    ``cat-file -t`` proves both object identity and commit type without using
    revision syntax that those launchers can mutate.
    """

    object_type = _git(
        repository,
        ["cat-file", "-t", commit],
        "base commit verification",
        timeout=timeout,
    )
    if object_type != "commit":
        raise ResidualEvidenceError("base_commit does not name a commit object")


def _validate_process_timeout(value: float) -> float:
    """Validate the per-subprocess deadline independently of the build lock."""

    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ResidualEvidenceError("process_timeout must be a positive finite number")
    timeout = float(value)
    if not math.isfinite(timeout) or timeout <= 0.0:
        raise ResidualEvidenceError("process_timeout must be a positive finite number")
    return timeout


def _terminate_process_tree(process: subprocess.Popen[str]) -> list[str]:
    """Terminate a timed-out process and its descendants within a bounded grace period."""

    errors: list[str] = []
    pid = getattr(process, "pid", None)
    if not isinstance(pid, int) or pid <= 0:
        errors.append("timed-out process had no usable PID")
        return errors

    if os.name == "nt":
        # CREATE_NEW_PROCESS_GROUP lets taskkill own the complete compiler tree.
        try:
            killed = subprocess.run(
                ["taskkill", "/PID", str(pid), "/T", "/F"],
                text=True,
                capture_output=True,
                check=False,
                timeout=PROCESS_TERMINATION_GRACE,
            )
            if killed.returncode not in (0, 128) and process.poll() is None:
                errors.append(
                    "taskkill failed: "
                    + (killed.stderr.strip() or killed.stdout.strip() or "no diagnostic")[:500]
                )
        except (OSError, subprocess.TimeoutExpired) as exc:
            errors.append(f"taskkill failed: {exc}")
    else:
        try:
            process_group = os.getpgid(pid)
        except OSError as exc:
            process_group = None
            if process.poll() is None:
                errors.append(f"cannot find timed-out process group: {exc}")
        if process_group is not None:
            try:
                os.killpg(process_group, signal.SIGTERM)
            except ProcessLookupError:
                pass
            except OSError as exc:
                errors.append(f"SIGTERM failed: {exc}")

    try:
        process.wait(timeout=PROCESS_TERMINATION_GRACE)
    except subprocess.TimeoutExpired:
        if os.name != "nt":
            try:
                process_group = os.getpgid(pid)
            except OSError:
                process_group = None
            if process_group is not None:
                try:
                    os.killpg(process_group, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                except OSError as exc:
                    errors.append(f"SIGKILL failed: {exc}")
        try:
            process.kill()
        except ProcessLookupError:
            pass
        except OSError as exc:
            errors.append(f"process kill failed: {exc}")
        try:
            process.wait(timeout=PROCESS_TERMINATION_GRACE)
        except subprocess.TimeoutExpired:
            errors.append("timed-out process did not exit after forced termination")
        except OSError as exc:
            errors.append(f"process wait failed: {exc}")
    except OSError as exc:
        errors.append(f"process wait failed: {exc}")
    return errors


def _run_bounded(
    command: Sequence[str], *, cwd: Path, label: str, timeout: float
) -> str:
    """Run one tool subprocess with a materializer-owned process-tree deadline."""

    process_timeout = _validate_process_timeout(timeout)
    popen_kwargs: dict[str, Any] = {
        "cwd": cwd,
        "text": True,
        "stdin": subprocess.DEVNULL,
        "stdout": subprocess.PIPE,
        "stderr": subprocess.PIPE,
    }
    if os.name == "nt":
        creation_flags = getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
        if creation_flags:
            popen_kwargs["creationflags"] = creation_flags
    else:
        popen_kwargs["start_new_session"] = True
    try:
        process = subprocess.Popen([str(value) for value in command], **popen_kwargs)
    except OSError as exc:
        raise ResidualEvidenceError(f"{label} could not start: {exc}") from exc

    try:
        stdout, stderr = process.communicate(timeout=process_timeout)
    except subprocess.TimeoutExpired as exc:
        termination_errors = _terminate_process_tree(process)
        try:
            process.communicate(timeout=PROCESS_TERMINATION_GRACE)
        except subprocess.TimeoutExpired:
            # Do not wait indefinitely for a descendant that inherited a pipe.
            try:
                process.kill()
            except (OSError, ProcessLookupError):
                pass
            for stream in (process.stdout, process.stderr):
                if stream is not None:
                    try:
                        stream.close()
                    except OSError:
                        pass
        detail = (
            "; termination: " + "; ".join(termination_errors)
            if termination_errors
            else "; process tree terminated"
        )
        raise ResidualEvidenceError(
            f"{label} timed out after {process_timeout:.3f}s{detail}"
        ) from exc
    except OSError as exc:
        _terminate_process_tree(process)
        raise ResidualEvidenceError(f"{label} process communication failed: {exc}") from exc

    if process.returncode:
        detail = (stderr or "").strip() or (stdout or "").strip() or "no diagnostic"
        raise ResidualEvidenceError(
            f"{label} failed ({process.returncode}): {detail[:1000]}"
        )
    return stdout or ""


def _verify_objdiff_bounded(path: Path, timeout: float) -> dict[str, Any]:
    if not path.is_file():
        raise bundle.EvidenceError(f"pinned objdiff is missing: {path}")
    actual = _file_sha(path)
    if actual != bundle.OBJDFF_SHA256:
        raise bundle.EvidenceError(f"objdiff SHA-256 drifted: {actual} != {bundle.OBJDFF_SHA256}")
    version = _run_bounded(
        [str(path), "--version"], cwd=path.parent, label="objdiff version", timeout=timeout
    ).strip()
    if version not in {
        f"objdiff-cli.exe {bundle.OBJDFF_VERSION}",
        f"objdiff-cli {bundle.OBJDFF_VERSION}",
    }:
        raise bundle.EvidenceError(f"objdiff version drifted: {version!r}")
    result = bundle._descriptor(path)
    result.update({"version": bundle.OBJDFF_VERSION, "expected_sha256": bundle.OBJDFF_SHA256})
    return result


def _verify_readelf_bounded(path: Path, timeout: float) -> dict[str, Any]:
    if not path.is_file():
        raise bundle.EvidenceError(f"pinned PowerPC readelf is missing: {path}")
    version = _run_bounded(
        [str(path), "--version"], cwd=path.parent, label="readelf version", timeout=timeout
    ).splitlines()
    first = version[0] if version else ""
    if "GNU readelf" not in first or "2.42" not in first:
        raise bundle.EvidenceError(
            f"PowerPC readelf is not pinned binutils {bundle.BINUTILS_TAG}: {first!r}"
        )
    result = bundle._descriptor(path)
    result.update({"binutils_tag": bundle.BINUTILS_TAG, "version_line": first})
    return result


def _verify_ninja_bounded(path: Path, timeout: float) -> dict[str, Any]:
    if not path.is_file():
        raise bundle.EvidenceError(f"pinned Ninja is missing: {path}")
    actual = _file_sha(path)
    if actual != bundle.NINJA_SHA256:
        raise bundle.EvidenceError(f"Ninja SHA-256 drifted: {actual} != {bundle.NINJA_SHA256}")
    version = _run_bounded(
        [str(path), "--version"], cwd=path.parent, label="Ninja version", timeout=timeout
    ).strip()
    if version != bundle.NINJA_VERSION:
        raise bundle.EvidenceError(f"Ninja version drifted: {version!r}")
    result = bundle._descriptor(path)
    result.update({"version": bundle.NINJA_VERSION, "expected_sha256": bundle.NINJA_SHA256})
    return result


def _ensure_configured_bounded(
    root: Path, toolchain: Mapping[str, Any], ninja: Path, timeout: float
) -> Path:
    """Stage retail/configure the detached worktree using only bounded commands."""

    build_ninja = root / "build.ninja"
    objdiff_config = root / "objdiff.json"
    orig = root / "orig"
    allowed_existing = {orig / "GP6E01" / ".gitkeep"}
    existing = {path for path in orig.rglob("*") if path.is_file()} if orig.exists() else set()
    if existing - allowed_existing or any(path.is_symlink() for path in orig.rglob("*")):
        raise bundle.EvidenceError("detached worktree contains unsealed preexisting orig input")
    orig.mkdir(exist_ok=True)
    retail_copy = orig / "GP6E01"
    retail_copy.mkdir(parents=True, exist_ok=True)
    try:
        shutil.copytree(
            toolchain["orig"]["path_object"], retail_copy,
            copy_function=shutil.copyfile, dirs_exist_ok=True,
        )
        if not build_ninja.is_file() or not objdiff_config.is_file():
            configure = [
                sys.executable, "configure.py", "configure",
                "--binutils", str(toolchain["binutils"]["path_object"]),
                "--compilers", str(toolchain["compilers"]["path_object"]),
                "--dtk", str(toolchain["dtk"]["path_object"]),
                "--sjiswrap", str(toolchain["sjiswrap"]["path_object"]),
            ]
            _run_bounded(
                configure, cwd=root, label="detached worktree configure", timeout=timeout
            )
            _run_bounded(
                [str(ninja), "-j1", "build/GP6E01/config.json"],
                cwd=root, label="retail object split", timeout=timeout,
            )
            _run_bounded(
                configure, cwd=root, label="detached worktree reconfigure", timeout=timeout
            )
    except BaseException as primary:
        try:
            bundle._remove_staged_retail(retail_copy)
        except BaseException as cleanup:
            # Preserve a timeout/nonzero compiler result as the terminal cause;
            # the outer materializer records cleanup failures separately.
            try:
                primary.add_note(f"staged retail cleanup failed: {cleanup}")
            except AttributeError:
                pass
        raise
    if not build_ninja.is_file() or not objdiff_config.is_file():
        bundle._remove_staged_retail(retail_copy)
        raise bundle.EvidenceError("configuration did not publish build.ninja and objdiff.json")
    return retail_copy


def _run_objdiff_bounded(
    objdiff: Path,
    target: Path,
    candidate: Path,
    output: Path,
    *,
    data: bool,
    root: Path,
    timeout: float,
) -> None:
    """Run objdiff through the bounded materializer runner."""

    bundle._assert_no_indirection(target)
    bundle._assert_no_indirection(candidate)
    bundle._assert_no_indirection(output.parent)
    if output.exists() or output.is_symlink():
        bundle._assert_no_indirection(output)
    temp = output.with_name(output.name + ".tmp")
    if temp.exists() or temp.is_symlink():
        bundle._assert_no_indirection(temp)
        temp.unlink()
    command = [
        str(objdiff), "diff", "-1", str(target), "-2", str(candidate),
        "-o", str(temp), "--format", "json-pretty",
    ]
    if data:
        command += ["-c", "functionRelocDiffs=data_value"]
    _run_bounded(
        command, cwd=root, label="objdiff data" if data else "objdiff strict", timeout=timeout
    )
    document = bundle._load_json(temp, "objdiff report")
    if not isinstance(document.get("left"), Mapping) or not isinstance(document.get("right"), Mapping):
        raise bundle.EvidenceError("objdiff report lacks real left/right object evidence")
    bundle._assert_no_indirection(temp)
    bundle._assert_no_indirection(output.parent)
    os.replace(temp, output)


def _physical_receipt_bounded(
    target: Path,
    candidate: Path,
    function: str,
    strict_report: Path,
    readelf: Path,
    timeout: float,
) -> dict[str, Any]:
    """Build the physical receipt after bounded readelf probes."""

    for path in (target, candidate):
        _run_bounded(
            [str(readelf), "-SWsWr", "--", str(path)],
            cwd=path.parent,
            label="PowerPC readelf",
            timeout=timeout,
        )
    target_row = bundle._parse_elf_relocations(target, function)
    candidate_row = bundle._parse_elf_relocations(candidate, function)
    target_effective = [
        {key: row[key] for key in ("offset", "type", "effective_target")}
        for row in target_row["physical_relocations"]
    ]
    candidate_effective = [
        {key: row[key] for key in ("offset", "type", "effective_target")}
        for row in candidate_row["physical_relocations"]
    ]
    differences: list[dict[str, Any]] = []
    if target_effective != candidate_effective:
        differences.append({"target": target_effective, "candidate": candidate_effective})
    receipt: dict[str, Any] = {
        "schema": bundle.PHYSICAL_SCHEMA,
        "authority_advanced": False,
        "report": bundle._descriptor(strict_report),
        "function": function,
        "target": target_row,
        "candidate": candidate_row,
        "physical_relocations_exact": not differences,
        "physical_relocation_differences": differences,
        "symbol_attribution_aliases": [],
    }
    receipt["receipt_sha256"] = bundle._json_sha(receipt)
    return receipt


def _create_disposable_worktree(
    repository: Path,
    parent: Path,
    base_commit: str,
    *,
    process_timeout: float = DEFAULT_PROCESS_TIMEOUT,
) -> Path:
    """Create a detached checkout that is the only writable build root."""

    _text(base_commit, "base_commit")
    try:
        placeholder = Path(tempfile.mkdtemp(prefix=".crack-current-residual-worktree-", dir=parent))
        placeholder.rmdir()
    except OSError as exc:
        raise ResidualEvidenceError(f"cannot allocate disposable worktree path: {exc}") from exc
    try:
        bundle._assert_no_indirection(parent)
        _git(
            repository,
            ["worktree", "add", "--detach", str(placeholder), base_commit],
            "disposable worktree creation",
            timeout=process_timeout,
        )
    except BaseException as primary:
        rollback_errors: list[str] = []
        try:
            try:
                _run_bounded(
                    ["git", "worktree", "remove", "--force", str(placeholder)],
                    cwd=repository,
                    label="disposable worktree rollback",
                    timeout=process_timeout,
                )
            except ResidualEvidenceError as rollback:
                # A missing worktree is the expected Git 128 rollback result.
                if "failed (128)" not in str(rollback):
                    rollback_errors.append(str(rollback))
            try:
                _run_bounded(
                    ["git", "worktree", "prune"],
                    cwd=repository,
                    label="disposable worktree prune",
                    timeout=process_timeout,
                )
            except ResidualEvidenceError as prune:
                rollback_errors.append(str(prune))
            if placeholder.exists():
                shutil.rmtree(placeholder)
        except OSError as exc:
            rollback_errors.append(str(exc))
        for error in rollback_errors:
            try:
                primary.add_note(f"disposable worktree rollback: {error}")
            except AttributeError:
                pass
        raise
    try:
        _root(placeholder)
    except BaseException:
        # The worktree exists in Git's administrative metadata; attempt a
        # best-effort removal before exposing the failure.
        try:
            _git(
                repository,
                ["worktree", "remove", "--force", str(placeholder)],
                "disposable worktree rollback",
                timeout=process_timeout,
            )
            _git(
                repository,
                ["worktree", "prune"],
                "disposable worktree prune",
                timeout=process_timeout,
            )
        except ResidualEvidenceError:
            pass
        raise
    return placeholder


def _remove_disposable_worktree(
    repository: Path,
    worktree: Path,
    *,
    process_timeout: float = DEFAULT_PROCESS_TIMEOUT,
) -> str | None:
    """Remove a detached checkout and return, rather than raise, cleanup errors."""

    try:
        bundle._assert_no_indirection(worktree)
        try:
            _run_bounded(
                ["git", "worktree", "remove", "--force", str(worktree)],
                cwd=repository,
                label="disposable worktree cleanup",
                timeout=process_timeout,
            )
        except ResidualEvidenceError as exc:
            return f"disposable worktree cleanup failed: {exc}"
        if worktree.exists():
            return "disposable worktree cleanup failed: worktree remains"
        try:
            _run_bounded(
                ["git", "worktree", "prune"],
                cwd=repository,
                label="disposable worktree prune",
                timeout=process_timeout,
            )
        except ResidualEvidenceError as exc:
            return f"disposable worktree prune failed: {exc}"
    except (OSError, bundle.EvidenceError, ResidualEvidenceError) as exc:
        return f"disposable worktree cleanup failed: {exc}"
    return None


def _overlay_source(source: Path, worktree: Path, repository: Path, expected_sha256: str) -> Path:
    try:
        relative = source.relative_to(repository)
    except ValueError as exc:
        raise ResidualEvidenceError("source is not relative to the authoritative repository") from exc
    destination = _inside(worktree, worktree / relative, "disposable source")
    try:
        bundle._assert_no_indirection(destination.parent)
        if destination.exists() or destination.is_symlink():
            bundle._assert_no_indirection(destination)
        destination.parent.mkdir(parents=True, exist_ok=True)
        bundle._assert_no_indirection(destination.parent)
        temporary = destination.with_name(destination.name + ".current.tmp")
        if temporary.exists() or temporary.is_symlink():
            bundle._assert_no_indirection(temporary)
            temporary.unlink()
        shutil.copyfile(source, temporary)
        bundle._assert_no_indirection(temporary)
        os.replace(temporary, destination)
    except (OSError, bundle.EvidenceError) as exc:
        raise ResidualEvidenceError(f"cannot overlay current source into disposable worktree: {exc}") from exc
    if _file_sha(destination) != expected_sha256:
        raise ResidualEvidenceError("disposable source overlay hash drifted")
    return destination


def _publish_bundle(files: Sequence[tuple[Path, bytes]]) -> dict[Path, str]:
    """Publish all evidence files with rollback on a partial publish failure."""

    def withdraw(paths: list[Path]) -> list[str]:
        """Withdraw published paths, retaining any paths that still need retry."""

        rollback_errors: list[str] = []
        remaining: list[Path] = []
        for path in reversed(paths):
            try:
                bundle._assert_no_indirection(path)
                path.unlink(missing_ok=True)
            except (OSError, bundle.EvidenceError) as rollback_exc:
                rollback_errors.append(f"{path.name}: {rollback_exc}")
                remaining.append(path)
        paths[:] = list(reversed(remaining))
        return rollback_errors

    if not files or len({path for path, _ in files}) != len(files):
        raise ResidualEvidenceError("evidence publication paths must be unique and nonempty")
    temporary: list[tuple[Path, Path]] = []
    published: list[Path] = []
    rollback_required = False
    publication_error: ResidualEvidenceError | None = None
    try:
        for path, payload in files:
            if path.exists() or path.is_symlink():
                raise ResidualEvidenceError(f"atomic output appeared during publish: {path}")
            with tempfile.NamedTemporaryFile(
                "wb", dir=path.parent, prefix=path.name, suffix=".tmp", delete=False
            ) as handle:
                temporary_path = Path(handle.name)
                handle.write(payload)
                handle.flush()
                os.fsync(handle.fileno())
            bundle._assert_no_indirection(temporary_path)
            temporary.append((path, temporary_path))
        for path, temporary_path in temporary:
            bundle._assert_no_indirection(path.parent)
            if path.exists() or path.is_symlink():
                raise ResidualEvidenceError(f"atomic output appeared during publish: {path}")
            os.replace(temporary_path, path)
            published.append(path)
        temporary.clear()
        return {path: _file_sha(path) for path, _ in files}
    except (OSError, bundle.EvidenceError, ResidualEvidenceError) as exc:
        rollback_required = True
        rollback_errors = withdraw(published)
        detail = (
            "; rollback incomplete: " + "; ".join(rollback_errors)
            if rollback_errors else ""
        )
        publication_error = ResidualEvidenceError(
            f"cannot atomically publish evidence bundle: {exc}{detail}"
        )
        if published:
            try:
                publication_error.add_note(
                    "evidence rollback remains required for: "
                    + ", ".join(path.name for path in published)
                )
            except AttributeError:
                pass
        raise publication_error from exc
    finally:
        if rollback_required and published:
            retry_errors = withdraw(published)
            if retry_errors and publication_error is not None:
                try:
                    publication_error.add_note(
                        "evidence rollback retry remains incomplete: "
                        + "; ".join(retry_errors)
                    )
                except AttributeError:
                    pass
        for _, temporary_path in temporary:
            try:
                temporary_path.unlink(missing_ok=True)
            except OSError:
                pass


def _sanitize_focus_artifact(value: Mapping[str, Any], worktree: Path) -> dict[str, Any]:
    """Publish bounded causal focus evidence without losing residual identity.

    ``focus_symbol_report`` intentionally carries every strict instruction and
    every physical relocation.  That is useful as an in-process proof adapter,
    but it scales with the whole function instead of the residual.  The current
    residual record keeps each differing strict row plus a two-row causal
    context window, every data-diff row, the full raw payload digests, and every
    physical difference.  Exact physical arrays are replaced by their count
    and digest.  The raw reports/receipt remain hash-bound by ``input_binding``.
    """

    root = worktree.resolve()

    def clean(item: Any) -> Any:
        if isinstance(item, Mapping):
            result: dict[str, Any] = {}
            for key, child in item.items():
                if key == "path" and isinstance(child, str):
                    try:
                        Path(child).resolve().relative_to(root)
                    except ValueError:
                        pass
                    else:
                        continue
                result[key] = clean(child)
            return result
        if isinstance(item, list):
            return [clean(child) for child in item]
        return item

    result = clean(value)
    if not isinstance(result, dict):
        raise ResidualEvidenceError("focus artifact sanitizer produced invalid data")

    channels = result.get("channels")
    if not isinstance(channels, dict):
        raise ResidualEvidenceError("focus artifact sanitizer found no channels")
    for channel_name in ("strict", "data"):
        channel = channels.get(channel_name)
        if not isinstance(channel, dict):
            raise ResidualEvidenceError(
                f"focus artifact sanitizer found no {channel_name} channel"
            )
        for side_name in ("target", "candidate"):
            side = channel.get(side_name)
            rows = side.get("rows") if isinstance(side, dict) else None
            if not isinstance(side, dict) or not isinstance(rows, list):
                raise ResidualEvidenceError(
                    f"focus artifact sanitizer found invalid {channel_name}.{side_name} rows"
                )
            if channel_name == "strict":
                diff_indices = {
                    row.get("index")
                    for row in rows
                    if isinstance(row, Mapping)
                    and isinstance(row.get("index"), int)
                    and not isinstance(row.get("index"), bool)
                    and isinstance(row.get("diff_kind"), str)
                    and row.get("diff_kind")
                }
                retained_indices = {
                    index
                    for diff_index in diff_indices
                    for index in range(
                        max(0, diff_index - FOCUS_CONTEXT_RADIUS),
                        diff_index + FOCUS_CONTEXT_RADIUS + 1,
                    )
                }
                side["rows"] = [
                    row
                    for row in rows
                    if isinstance(row, Mapping) and row.get("index") in retained_indices
                ]
                side["rows_kind"] = "diff_context"
                side["context_radius"] = FOCUS_CONTEXT_RADIUS
            else:
                side["rows"] = [
                    row
                    for row in rows
                    if isinstance(row, Mapping)
                    and isinstance(row.get("diff_kind"), str)
                    and row.get("diff_kind")
                ]
                side["rows_kind"] = "diff_only"

    physical = result.get("physical_relocations")
    if not isinstance(physical, dict):
        raise ResidualEvidenceError("focus artifact sanitizer found no physical evidence")
    for side_name in ("target", "candidate"):
        side = physical.get(side_name)
        if not isinstance(side, dict):
            continue
        relocations = side.pop("physical_relocations", None)
        if relocations is not None:
            if not isinstance(relocations, list):
                raise ResidualEvidenceError(
                    f"focus artifact sanitizer found invalid {side_name} physical relocations"
                )
            side["physical_relocation_payload_sha256"] = _json_sha(relocations)

    policies = result.get("policies")
    if isinstance(policies, dict):
        policies["strict_rows"] = "diff_rows_plus_two_row_context_with_full_raw_digest"
        policies["physical_relocations"] = "differences_plus_full_payload_digest"
    result.pop("artifact_sha256", None)
    result["artifact_sha256"] = _json_sha(result)
    return result


def materialize_current_residual(
    *,
    root: Path,
    base_commit: str,
    owner: str,
    unit: str,
    function: str,
    source: Path,
    source_sha256: str,
    target_sha256: str,
    toolchain_key: str,
    start_line: int,
    end_line: int,
    output: Path,
    base_sha256: str | None = None,
    manifest_path: Path | None = None,
    build_lock: Path | None = None,
    build_lock_timeout: float = 55.0,
    process_timeout: float = DEFAULT_PROCESS_TIMEOUT,
) -> dict[str, Any]:
    """Compile current source once in a disposable worktree and publish its census."""

    repository = _root(root)
    owner_text = _text(owner, "owner")
    commit = _text(base_commit, "base_commit")
    if re.fullmatch(r"[0-9a-f]{40}", commit) is None:
        raise ResidualEvidenceError("base_commit must be a full lowercase Git commit")
    bounded_process_timeout = _validate_process_timeout(process_timeout)
    _verify_base_commit(repository, commit, timeout=bounded_process_timeout)
    if not isinstance(function, str) or FUNCTION_RE.fullmatch(function) is None:
        raise ResidualEvidenceError("function must be a C identifier")
    expected_source = _sha(source_sha256, "source_sha256")
    expected_target = _sha(target_sha256, "target_sha256")
    expected_toolchain = _sha(toolchain_key, "toolchain_key")
    source_path = _regular_file(repository, source, "source")
    if _file_sha(source_path) != expected_source:
        raise ResidualEvidenceError("source SHA-256 drifted")
    expected_base = expected_source if base_sha256 is None else _sha(base_sha256, "base_sha256")
    if expected_base != expected_source:
        raise ResidualEvidenceError("base_sha256 must equal the current source SHA-256")
    span_sha = _function_span_sha(source_path, start_line, end_line)

    output_path = _validate_output(repository, output)
    physical_output = _validate_output(
        repository, output_path.with_name(output_path.name + ".physical.json")
    )
    focus_output = _validate_output(
        repository, output_path.with_name(output_path.name + ".focus.json")
    )
    target_output = _validate_output(
        repository, output_path.with_name(output_path.name + ".target.o")
    )
    base_output = _validate_output(
        repository, output_path.with_name(output_path.name + ".base.o")
    )
    manifest = Path(manifest_path) if manifest_path is not None else bundle.DEFAULT_TOOLCHAIN_MANIFEST
    lock_path = _inside(
        repository,
        Path(build_lock) if build_lock is not None else repository / "build" / ".compiler-lane.lock",
        "build lock",
    )

    worktree: Path | None = None
    retail_copy: Path | None = None
    completed_result: dict[str, Any] | None = None
    primary_error: BaseException | None = None
    try:
        with serialized_build_lock(lock_path, build_lock_timeout):
            if _file_sha(source_path) != expected_source:
                raise ResidualEvidenceError("source changed while waiting for the build lock")
            worktree = _create_disposable_worktree(
                repository,
                output_path.parent,
                commit,
                process_timeout=bounded_process_timeout,
            )
            disposable_source = _overlay_source(
                source_path, worktree, repository, expected_source
            )
            if _function_span_sha(disposable_source, start_line, end_line) != span_sha:
                raise ResidualEvidenceError("disposable function span drifted")
            try:
                toolchain = bundle._load_toolchain(manifest, expected_toolchain)
                objdiff = Path(toolchain["objdiff"]["path_object"])
                readelf = Path(toolchain["binutils"]["path_object"]) / "powerpc-eabi-readelf.exe"
                ninja = Path(toolchain["ninja"]["path_object"])
                _verify_objdiff_bounded(objdiff, bounded_process_timeout)
                _verify_readelf_bounded(readelf, bounded_process_timeout)
                _verify_ninja_bounded(ninja, bounded_process_timeout)
                retail_copy = _ensure_configured_bounded(
                    worktree, toolchain, ninja, bounded_process_timeout
                )
                target_path, base_path = bundle._unit_paths(worktree, unit)
            except (bundle.EvidenceError, OSError) as exc:
                raise ResidualEvidenceError(f"toolchain/build configuration failed: {exc}") from exc
            if not target_path.is_file() or _file_sha(target_path) != expected_target:
                raise ResidualEvidenceError("selected target object is missing or hash-drifted")
            if base_path.exists() or base_path.is_symlink():
                bundle._assert_no_indirection(base_path)
                if not base_path.is_file():
                    raise ResidualEvidenceError("stale base object is not a regular file")
                base_path.unlink()
            try:
                compile_stdout = _run_bounded(
                    [str(ninja), "-j1", str(base_path.relative_to(worktree))],
                    cwd=worktree,
                    label="current base object build",
                    timeout=bounded_process_timeout,
                )
            except (bundle.EvidenceError, OSError) as exc:
                raise ResidualEvidenceError(f"current base build failed: {exc}") from exc
            if not base_path.is_file() or base_path.is_symlink():
                raise ResidualEvidenceError("current base build produced no object")
            if _file_sha(source_path) != expected_source:
                raise ResidualEvidenceError("authoritative source changed during current base build")
            if _file_sha(disposable_source) != expected_source:
                raise ResidualEvidenceError("disposable source changed during current base build")
            if _file_sha(target_path) != expected_target:
                raise ResidualEvidenceError("target object changed during current base build")

            proof_root = worktree / "build" / ".current-residual-proof"
            proof_root.mkdir(parents=True, exist_ok=False)
            strict_path = proof_root / "strict.json"
            data_path = proof_root / "data.json"
            physical_path = proof_root / "physical.json"
            try:
                _run_objdiff_bounded(
                    objdiff, target_path, base_path, strict_path,
                    data=False, root=worktree, timeout=bounded_process_timeout,
                )
                _run_objdiff_bounded(
                    objdiff, target_path, base_path, data_path,
                    data=True, root=worktree, timeout=bounded_process_timeout,
                )
                physical = _physical_receipt_bounded(
                    target_path, base_path, function, strict_path, readelf,
                    bounded_process_timeout,
                )
                bundle._atomic_json(physical_path, physical)
                focus = focus_symbol_report.build_from_paths(
                    strict_report_path=strict_path,
                    data_report_path=data_path,
                    function=function,
                    expected_strict_report_sha256=_file_sha(strict_path),
                    expected_data_report_sha256=_file_sha(data_path),
                    physical_receipt_path=physical_path,
                    expected_physical_receipt_sha256=_file_sha(physical_path),
                    require_physical=False,
                )
                focus = _sanitize_focus_artifact(focus, worktree)
                strict_document = json.loads(strict_path.read_bytes())
                data_document = json.loads(data_path.read_bytes())
            except (
                bundle.EvidenceError,
                focus_symbol_report.FocusReportError,
                OSError,
                json.JSONDecodeError,
            ) as exc:
                raise ResidualEvidenceError(f"current residual proof adapter failed: {exc}") from exc
            rows = residual_row_ids(strict_document, data_document, function)
            rows.extend(physical_row_ids(physical, function))
            focus_rows = focus_residual_row_ids(focus, function)
            if focus_rows != rows:
                raise ResidualEvidenceError(
                    "published focus residual rows do not match strict/data/physical census"
                )
            if not rows:
                raise ResidualEvidenceError(
                    "current base is exact; no residual rows to materialize"
                )
            if len(set(rows)) != len(rows):
                raise ResidualEvidenceError("current residual rows are not unique")
            source_bytes = source_path.read_bytes()
            disposable_source_bytes = disposable_source.read_bytes()
            target_bytes = target_path.read_bytes()
            base_bytes = base_path.read_bytes()
            if hashlib.sha256(source_bytes).hexdigest() != expected_source:
                raise ResidualEvidenceError("authoritative source drifted before publication")
            if hashlib.sha256(disposable_source_bytes).hexdigest() != expected_source:
                raise ResidualEvidenceError("disposable source drifted before publication")
            if hashlib.sha256(target_bytes).hexdigest() != expected_target:
                raise ResidualEvidenceError("target object drifted before publication")
            focus_bytes = _canonical(focus) + b"\n"
            if len(target_bytes) > MAX_OBJECT_BYTES or len(base_bytes) > MAX_OBJECT_BYTES:
                raise ResidualEvidenceError("selected owner object exceeds compact evidence limit")
            if len(focus_bytes) > MAX_FOCUS_BYTES:
                raise ResidualEvidenceError("focus artifact exceeds compact evidence limit")

            def stable_descriptor(path: Path, payload: bytes) -> dict[str, Any]:
                return {
                    "path": path.relative_to(repository).as_posix(),
                    "sha256": hashlib.sha256(payload).hexdigest(),
                    "size_bytes": len(payload),
                }

            physical_summary = _compact_physical_summary(
                physical,
                root=worktree,
                owner=owner_text,
                unit=unit,
                function=function,
                target=target_path,
                base=base_path,
                strict=strict_path,
                data=data_path,
            )
            physical_summary.update(
                {
                    "base_commit": commit,
                    "source": {
                        "path": source_path.relative_to(repository).as_posix(),
                        "sha256": expected_source,
                        "size_bytes": len(source_bytes),
                    },
                    "target_object": stable_descriptor(target_output, target_bytes),
                    "base_object": stable_descriptor(base_output, base_bytes),
                    "focus_report": stable_descriptor(focus_output, focus_bytes),
                    "strict_report": {
                        "sha256": _file_sha(strict_path),
                        "size_bytes": strict_path.stat().st_size,
                    },
                    "data_report": {
                        "sha256": _file_sha(data_path),
                        "size_bytes": data_path.stat().st_size,
                    },
                }
            )
            physical_summary.pop("physical_summary_sha256", None)
            physical_summary["physical_summary_sha256"] = _json_sha(physical_summary)

            physical_bytes = _canonical(physical_summary) + b"\n"
            if len(physical_bytes) > MAX_PHYSICAL_SUMMARY_BYTES:
                raise ResidualEvidenceError("physical summary exceeds compact evidence limit")
            producer_path = Path(__file__).resolve()
            expected_producer_path = (repository / "tools" / "crack_current_residual.py").resolve()
            if producer_path != expected_producer_path or not producer_path.is_file():
                raise ResidualEvidenceError(
                    "materializer producer is not the repository-bound tool"
                )
            strict_descriptor = {
                "sha256": _file_sha(strict_path),
                "size_bytes": strict_path.stat().st_size,
            }
            data_descriptor = {
                "sha256": _file_sha(data_path),
                "size_bytes": data_path.stat().st_size,
            }
            physical_receipt_descriptor = {
                "sha256": _file_sha(physical_path),
                "size_bytes": physical_path.stat().st_size,
            }
            artifact = build_residual_artifact(
                owner=owner_text,
                function=function,
                base_sha256=expected_base,
                source_sha256=expected_source,
                target_sha256=expected_target,
                base_commit=commit,
                unit=unit,
                start_line=start_line,
                end_line=end_line,
                base_span_sha256=span_sha,
                toolchain_key=expected_toolchain,
                residual_rows=rows,
                base_object=stable_descriptor(base_output, base_bytes),
                target_object=stable_descriptor(target_output, target_bytes),
                focus_report=stable_descriptor(focus_output, focus_bytes),
                physical_summary=stable_descriptor(physical_output, physical_bytes),
                strict_report=strict_descriptor,
                data_report=data_descriptor,
                physical_receipt=physical_receipt_descriptor,
                producer=stable_descriptor(
                    producer_path, producer_path.read_bytes()
                ),
            )
            core_bytes = _canonical(artifact) + b"\n"
            if len(core_bytes) > MAX_ARTIFACT_BYTES:
                raise ResidualEvidenceError("residual artifact exceeds compact evidence limit")
            published = _publish_bundle(
                [
                    (target_output, target_bytes),
                    (base_output, base_bytes),
                    (focus_output, focus_bytes),
                    (physical_output, physical_bytes),
                    # Publish the admissible core last.  It can never become
                    # visible before every descriptor it authenticates.
                    (output_path, core_bytes),
                ]
            )
            completed_result = {
                "schema": RESULT_SCHEMA,
                "status": "materialized",
                "owner": owner_text,
                "unit": unit,
                "function": function,
                "base_commit": commit,
                "artifact": output_path.relative_to(repository).as_posix(),
                "artifact_sha256": published[output_path],
                "physical_summary": physical_output.relative_to(repository).as_posix(),
                "physical_summary_sha256": published[physical_output],
                "focus_report": focus_output.relative_to(repository).as_posix(),
                "focus_report_sha256": published[focus_output],
                "target_object": target_output.relative_to(repository).as_posix(),
                "target_object_sha256": published[target_output],
                "base_object": base_output.relative_to(repository).as_posix(),
                "base_object_sha256": published[base_output],
                "residual_row_count": len(rows),
                "compile_stdout_sha256": hashlib.sha256(compile_stdout.encode()).hexdigest(),
                "cleanup_incomplete": False,
                "authority_advanced": False,
            }
            return completed_result
    except BaseException as exc:
        primary_error = exc
        raise
    finally:
        cleanup_errors: list[str] = []
        if retail_copy is not None:
            try:
                bundle._remove_staged_retail(retail_copy)
            except (OSError, bundle.EvidenceError) as exc:
                cleanup_errors.append(f"staged retail cleanup failed: {exc}")
        if worktree is not None:
            cleanup_error = _remove_disposable_worktree(
                repository,
                worktree,
                process_timeout=bounded_process_timeout,
            )
            if cleanup_error is not None:
                cleanup_errors.append(cleanup_error)
        if cleanup_errors:
            if completed_result is not None:
                withdrawal_errors: list[str] = []
                for path in (
                    output_path,
                    physical_output,
                    focus_output,
                    target_output,
                    base_output,
                ):
                    try:
                        bundle._assert_no_indirection(path)
                        path.unlink(missing_ok=True)
                    except (OSError, bundle.EvidenceError) as exc:
                        withdrawal_errors.append(f"cannot withdraw {path.name}: {exc}")
                detail = cleanup_errors + withdrawal_errors
                raise ResidualEvidenceError(
                    "current residual cleanup failed; evidence is not admissible: "
                    + "; ".join(detail)
                )
            elif primary_error is not None:
                for error in cleanup_errors:
                    try:
                        primary_error.add_note(error)
                    except AttributeError:
                        pass
            else:
                raise ResidualEvidenceError("; ".join(cleanup_errors))


# Short alias for callers that prefer the command's noun.
materialize = materialize_current_residual


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--base-commit", required=True)
    parser.add_argument("--owner", required=True)
    parser.add_argument("--unit", required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--source-sha256", required=True)
    parser.add_argument("--base-sha256")
    parser.add_argument("--target-sha256", required=True)
    parser.add_argument("--toolchain-key", required=True)
    parser.add_argument("--span-start", type=int, required=True)
    parser.add_argument("--span-end", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--build-lock", type=Path)
    parser.add_argument("--build-lock-timeout", type=float, default=55.0)
    parser.add_argument("--process-timeout", type=float, default=DEFAULT_PROCESS_TIMEOUT)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = materialize_current_residual(
            root=args.root,
            base_commit=args.base_commit,
            owner=args.owner,
            unit=args.unit,
            function=args.function,
            source=args.source,
            source_sha256=args.source_sha256,
            base_sha256=args.base_sha256,
            target_sha256=args.target_sha256,
            toolchain_key=args.toolchain_key,
            start_line=args.span_start,
            end_line=args.span_end,
            output=args.output,
            manifest_path=args.manifest,
            build_lock=args.build_lock,
            build_lock_timeout=args.build_lock_timeout,
            process_timeout=args.process_timeout,
        )
    except (ResidualEvidenceError, OSError) as exc:
        print(f"crack current residual: {exc}", file=__import__("sys").stderr)
        return 2
    print(json.dumps(result, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
