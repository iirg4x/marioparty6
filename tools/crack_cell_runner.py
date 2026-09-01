"""Compile and measure one natural-C candidate without retaining campaign history.

The runner is deliberately a measurement tool, not a retention or promotion
front door.  It consumes a current-bound residual produced by
``crack_current_residual.py``, overlays one complete candidate source in a
detached worktree, runs exactly one unit build, and replaces one compact result
file.  Cleanup completes before publication; cleanup failure publishes nothing.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import time
from pathlib import Path
from typing import Any, Mapping, Sequence

try:
    from tools import crack_current_residual as residual
    from tools import crack_evidence_bundle as bundle
    from tools import focus_symbol_report
    from tools import owner_campaign
except ImportError:  # direct ``python tools/crack_cell_runner.py`` execution
    import crack_current_residual as residual  # type: ignore
    import crack_evidence_bundle as bundle  # type: ignore
    import focus_symbol_report  # type: ignore
    import owner_campaign  # type: ignore


SCHEMA = "crack_cell_measurement/v1"
MAX_OUTPUT_BYTES = 512 * 1024
LABEL_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]{0,79}")
REGISTER_RE = re.compile(r"(?<![A-Za-z0-9_])(?:r|f)\d+(?![A-Za-z0-9_])")


class CellRunnerError(RuntimeError):
    pass


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _json_sha(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _file_sha(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _atomic_json(path: Path, value: Mapping[str, Any]) -> str:
    payload = _canonical(value) + b"\n"
    if len(payload) > MAX_OUTPUT_BYTES:
        raise CellRunnerError(
            f"compact result exceeds {MAX_OUTPUT_BYTES} bytes: {len(payload)}"
        )
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{os.getpid()}.tmp")
    try:
        with temporary.open("wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)
    return hashlib.sha256(payload).hexdigest()


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise CellRunnerError(f"{label} is unreadable: {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise CellRunnerError(f"{label} must be a JSON object: {path}")
    return value


def _load_baseline(repository: Path, path: Path, function: str) -> tuple[dict[str, Any], dict[str, Any]]:
    try:
        path.resolve().relative_to(repository.resolve())
    except ValueError as exc:
        raise CellRunnerError("current residual path escapes repository") from exc
    if not path.is_file() or path.is_symlink():
        raise CellRunnerError(f"current residual is not a regular file: {path}")
    artifact = _load_json(path, "current residual")
    if artifact.get("schema") != residual.SCHEMA:
        raise CellRunnerError("current residual schema is invalid")
    body = dict(artifact)
    declared = body.pop("residual_sha256", None)
    if not isinstance(declared, str) or _json_sha(body) != declared:
        raise CellRunnerError("current residual self-hash is invalid")
    if artifact.get("function") != function or artifact.get("current_source_bound") is not True:
        raise CellRunnerError("current residual function/current-source binding is stale")
    focus_descriptor = artifact.get("focus_report")
    if not isinstance(focus_descriptor, Mapping):
        raise CellRunnerError("current residual focus descriptor is missing")
    focus_path = (repository / str(focus_descriptor.get("path", ""))).resolve()
    try:
        focus_path.relative_to(repository.resolve())
    except ValueError as exc:
        raise CellRunnerError("current residual focus path escapes repository") from exc
    if (
        not focus_path.is_file() or focus_path.is_symlink()
        or _file_sha(focus_path) != focus_descriptor.get("sha256")
        or focus_path.stat().st_size != focus_descriptor.get("size_bytes")
    ):
        raise CellRunnerError("current residual focus payload drifted")
    focus = _load_json(focus_path, "current residual focus")
    focus_body = dict(focus)
    focus_sha = focus_body.pop("artifact_sha256", None)
    if not isinstance(focus_sha, str) or _json_sha(focus_body) != focus_sha:
        raise CellRunnerError("current residual focus self-hash is invalid")
    return artifact, focus


def _find_bound_source(worktree: Path, candidate: Path, expected_sha: str) -> Path:
    matches = [
        path for path in (worktree / "src").rglob(candidate.name)
        if path.is_file() and not path.is_symlink() and _file_sha(path) == expected_sha
    ]
    if len(matches) != 1:
        raise CellRunnerError(
            f"cannot uniquely resolve bound source for {candidate.name}: {len(matches)} matches"
        )
    return matches[0]


def _git_text(repository: Path, *args: str) -> str:
    process = subprocess.run(
        ["git", "-C", str(repository), *args], capture_output=True, text=True,
        encoding="utf-8", errors="replace", check=False,
    )
    if process.returncode != 0:
        reason = (process.stderr or process.stdout).strip()[:1000]
        raise CellRunnerError(f"Git context check failed ({process.returncode}): {reason}")
    # Porcelain status output uses its first two columns for state.  Preserve
    # leading whitespace so callers can safely slice the path at column 3.
    return process.stdout.rstrip()


def _same_path(left: Path, right: Path) -> bool:
    return os.path.normcase(os.path.abspath(left)) == os.path.normcase(os.path.abspath(right))


def _git_path(value: str) -> Path:
    """Normalize Git-for-Windows `/d/...` output, never caller path input."""

    match = re.fullmatch(r"/([A-Za-z])/(.*)", value.replace("\\", "/"))
    if match:
        return Path(f"{match.group(1).upper()}:/{match.group(2)}")
    normalized = value.replace("\\", "/")
    if normalized == "/home/Anony" or normalized.startswith("/home/Anony/"):
        suffix = normalized[len("/home/Anony"):].lstrip("/")
        return Path.home() / suffix
    return Path(value)


def _authoritative_source(
    repository: Path, candidate: Path, artifact: Mapping[str, Any]
) -> Path:
    if _git_text(repository, "rev-parse", "--show-prefix"):
        raise CellRunnerError("executing repository root is foreign or noncanonical")
    head = _git_text(repository, "rev-parse", "HEAD")
    if head != artifact.get("base_commit"):
        raise CellRunnerError("STALE_CONTEXT: repository HEAD differs from current residual")
    matches = [
        path for path in (repository / "src").rglob(candidate.name)
        if path.is_file() and not path.is_symlink()
        and _file_sha(path) == artifact.get("base_sha256")
    ]
    if len(matches) != 1:
        raise CellRunnerError(
            "STALE_CONTEXT: retained frontier source is absent or ambiguous"
        )
    source = matches[0]
    dirty = _git_text(repository, "status", "--porcelain", "--untracked-files=no")
    allowed = source.relative_to(repository).as_posix()
    for line in dirty.splitlines():
        path = line[3:].strip().strip('"').replace("\\", "/")
        if " -> " in path:
            path = path.split(" -> ", 1)[1]
        if path != allowed:
            raise CellRunnerError(
                f"STALE_CONTEXT: tracked write outside retained frontier source: {path}"
            )
    return source


def _validate_worktree_identity(repository: Path, worktree: Path, base_commit: str) -> None:
    if _git_text(worktree, "rev-parse", "--show-prefix"):
        raise CellRunnerError("disposable worktree root is foreign or noncanonical")
    if _git_text(worktree, "rev-parse", "HEAD") != base_commit:
        raise CellRunnerError("disposable worktree HEAD is stale")
    if _git_text(worktree, "status", "--porcelain", "--untracked-files=no"):
        raise CellRunnerError("disposable worktree is dirty before candidate overlay")
    marker = worktree / ".git"
    if not marker.is_file() or marker.is_symlink():
        raise CellRunnerError("disposable worktree .git marker is invalid")
    text = marker.read_text(encoding="utf-8", errors="strict").strip()
    if not text.lower().startswith("gitdir:"):
        raise CellRunnerError("disposable worktree .git marker is malformed")
    raw = text.split(":", 1)[1].strip()
    windows_drive = re.match(r"^[A-Za-z]:[\\/]", raw) is not None
    git_windows_path = raw.startswith("/home/Anony/") or re.match(
        r"^/[A-Za-z]/", raw
    ) is not None
    if raw.startswith("\\\\") or not (windows_drive or git_windows_path):
        raise CellRunnerError("disposable worktree .git path is foreign/non-Windows")
    gitdir = _git_path(raw).resolve()
    common_raw = _git_text(repository, "rev-parse", "--git-common-dir")
    common = _git_path(common_raw)
    if not common.is_absolute():
        common = repository / common
    common = common.resolve()
    try:
        gitdir.relative_to(common)
    except ValueError as exc:
        raise CellRunnerError("disposable worktree .git path escapes common Git state") from exc


def _validate_candidate_cell(base: Path, candidate: Path, artifact: Mapping[str, Any]) -> None:
    if not candidate.is_file() or candidate.is_symlink():
        raise CellRunnerError(f"candidate source is not a regular file: {candidate}")
    if _file_sha(base) != artifact.get("base_sha256"):
        raise CellRunnerError("disposable base source is stale")
    candidate_scope = artifact.get("candidate_scope")
    if candidate_scope is not None:
        function = artifact.get("function")
        if not isinstance(function, str) or not function:
            raise CellRunnerError("current residual function is missing")
        try:
            base_text = base.read_text(encoding="utf-8")
            candidate_text = candidate.read_text(encoding="utf-8")
            base_start, base_end, _base_span = owner_campaign._find_function_span(
                base_text, function, "current residual base function"
            )
            span = artifact.get("function_span")
            if not isinstance(span, Mapping):
                raise CellRunnerError("current residual function span is missing")
            if (base_start, base_end) != (span.get("start_line"), span.get("end_line")):
                raise CellRunnerError("current residual function span does not bind the source")
            candidate_start, candidate_end, _candidate_span = owner_campaign._find_function_span(
                candidate_text, function, "candidate function"
            )
            owner_campaign.validate_candidate_scope(
                base_text=base_text,
                candidate_text=candidate_text,
                function=function,
                base_start_line=base_start,
                base_end_line=base_end,
                candidate_start_line=candidate_start,
                candidate_end_line=candidate_end,
                base_source_sha256=str(artifact.get("base_sha256")),
                candidate_source_sha256=_file_sha(candidate),
                scope=candidate_scope,
            )
        except (OSError, UnicodeError, owner_campaign.CampaignError) as exc:
            raise CellRunnerError(f"candidate adjacent-helper scope is invalid: {exc}") from exc
        return
    span = artifact.get("function_span")
    if not isinstance(span, Mapping):
        raise CellRunnerError("current residual function span is missing")
    start, end = span.get("start_line"), span.get("end_line")
    if type(start) is not int or type(end) is not int:
        raise CellRunnerError("current residual function span is invalid")
    if residual._function_span_sha(base, start, end) != span.get("base_span_sha256"):
        raise CellRunnerError("current residual function span hash drifted")
    base_lines = base.read_text(encoding="utf-8").splitlines(keepends=True)
    candidate_lines = candidate.read_text(encoding="utf-8").splitlines(keepends=True)
    import difflib
    changes = [
        opcode for opcode in difflib.SequenceMatcher(None, base_lines, candidate_lines).get_opcodes()
        if opcode[0] != "equal"
    ]
    if not changes:
        raise CellRunnerError("candidate source is byte-identical to the current base")
    for tag, i1, i2, _j1, _j2 in changes:
        outside = i1 < start - 1 or i2 > end
        if tag == "insert":
            outside = not (start <= i1 < end)
        if outside:
            raise CellRunnerError("candidate changes escape the bound function span")


def _resolve_toolchain(repository: Path, explicit: Path | None, expected_key: str) -> Path:
    candidates: list[Path] = []
    if explicit is not None:
        candidates = [explicit if explicit.is_absolute() else repository / explicit]
    else:
        default = Path(bundle.DEFAULT_TOOLCHAIN_MANIFEST)
        candidates = [default if default.is_absolute() else repository / default]
    valid: list[Path] = []
    for path in candidates:
        if not path.is_file() or path.is_symlink():
            continue
        try:
            bundle._load_toolchain(path, expected_key)
        except Exception:
            continue
        valid.append(path.resolve())
    if len(valid) != 1:
        raise CellRunnerError("toolchain manifest cannot be uniquely authenticated")
    return valid[0]


def _registers(formatted: Any) -> list[str]:
    return REGISTER_RE.findall(formatted) if isinstance(formatted, str) else []


def _diff_rows(channel: Mapping[str, Any]) -> list[dict[str, Any]]:
    target_rows = channel.get("target", {}).get("rows", [])
    candidate_rows = channel.get("candidate", {}).get("rows", [])
    by_index: dict[int, dict[str, Any]] = {}
    for side, rows in (("target", target_rows), ("candidate", candidate_rows)):
        if not isinstance(rows, list):
            continue
        for row in rows:
            if not isinstance(row, Mapping) or not row.get("diff_kind"):
                continue
            index = row.get("index")
            if type(index) is not int:
                continue
            instruction = row.get("instruction")
            formatted = instruction.get("formatted") if isinstance(instruction, Mapping) else None
            by_index.setdefault(index, {"index": index})[side] = {
                "diff_kind": row.get("diff_kind"),
                "address": instruction.get("address") if isinstance(instruction, Mapping) else None,
                "formatted": formatted,
                "register_operands": _registers(formatted),
            }
    return [by_index[index] for index in sorted(by_index)]


def _channel_summary(channel: Mapping[str, Any]) -> dict[str, Any]:
    metric = channel.get("metric", {})
    target = channel.get("target", {})
    candidate = channel.get("candidate", {})
    return {
        "match_percent": metric.get("match_percent"),
        "target_bytes": metric.get("target_size"),
        "candidate_bytes": metric.get("candidate_size"),
        "target_instruction_count": target.get("instruction_count"),
        "candidate_instruction_count": candidate.get("instruction_count"),
        "difference_count": max(
            int(target.get("diff_row_count", 0)), int(candidate.get("diff_row_count", 0))
        ),
        "differences": _diff_rows(channel),
    }


def _execute_candidate(
    *, repository: Path, worktree: Path, artifact: Mapping[str, Any],
    baseline_focus: Mapping[str, Any], base_source: Path, candidate_source: Path,
    toolchain_manifest: Path, timeout: float,
) -> dict[str, Any]:
    relative_source = base_source.relative_to(repository)
    source = worktree / relative_source
    if not source.is_file() or source.is_symlink():
        raise CellRunnerError("bound source path is absent from disposable worktree")
    source.write_bytes(base_source.read_bytes())
    _validate_candidate_cell(source, candidate_source, artifact)
    source.write_bytes(candidate_source.read_bytes())
    candidate_sha = _file_sha(source)
    loaded = bundle._load_toolchain(toolchain_manifest, str(artifact["toolchain_key"]))
    objdiff = Path(loaded["objdiff"]["path_object"])
    readelf = Path(loaded["binutils"]["path_object"]) / "powerpc-eabi-readelf.exe"
    ninja = Path(loaded["ninja"]["path_object"])
    started = time.monotonic()
    staged: Path | None = None
    proof = worktree / "build" / ".crack-cell-proof"
    try:
        staged = residual._ensure_configured_bounded(worktree, loaded, ninja, timeout)
        target, candidate_object = bundle._unit_paths(worktree, str(artifact["unit"]))
        if not target.is_file() or _file_sha(target) != artifact.get("target_sha256"):
            raise CellRunnerError("target object is missing or stale")
        if candidate_object.exists() or candidate_object.is_symlink():
            bundle._assert_no_indirection(candidate_object)
            candidate_object.unlink()
        try:
            compile_output = residual._run_bounded(
                [str(ninja), "-j1", str(candidate_object.relative_to(worktree))],
                cwd=worktree, label="crack cell candidate build", timeout=timeout,
            )
        except Exception as exc:
            return {
                "compile_status": "failed", "compile_error": str(exc)[:2000],
                "compile_performed": True,
                "compile_stdout_sha256": None, "candidate_source_sha256": candidate_sha,
                "active_seconds": time.monotonic() - started,
            }
        if not candidate_object.is_file() or candidate_object.is_symlink():
            raise CellRunnerError("candidate build produced no regular object")
        if _file_sha(source) != candidate_sha:
            raise CellRunnerError("candidate source changed during compilation")
        proof.mkdir(parents=True, exist_ok=False)
        strict_path, data_path, physical_path = (
            proof / "strict.json", proof / "data.json", proof / "physical.json"
        )
        residual._run_objdiff_bounded(
            objdiff, target, candidate_object, strict_path, data=False,
            root=worktree, timeout=timeout,
        )
        residual._run_objdiff_bounded(
            objdiff, target, candidate_object, data_path, data=True,
            root=worktree, timeout=timeout,
        )
        physical = residual._physical_receipt_bounded(
            target, candidate_object, str(artifact["function"]), strict_path,
            readelf, timeout,
        )
        bundle._atomic_json(physical_path, physical)
        focus = focus_symbol_report.build_from_paths(
            strict_report_path=strict_path, data_report_path=data_path,
            function=str(artifact["function"]),
            expected_strict_report_sha256=_file_sha(strict_path),
            expected_data_report_sha256=_file_sha(data_path),
            physical_receipt_path=physical_path,
            expected_physical_receipt_sha256=_file_sha(physical_path),
            require_physical=False,
        )
        if _file_sha(source) != candidate_sha:
            raise CellRunnerError("candidate source changed during proof generation")
        if _file_sha(target) != artifact.get("target_sha256"):
            raise CellRunnerError("target object changed during proof generation")
        gate = focus_symbol_report.gate_artifacts(
            baseline_focus, focus,
            {"baseline_artifact": {}, "candidate_artifact": {}, "authority_advanced": False},
        )
        channels = focus["channels"]
        physical_focus = focus["physical_relocations"]
        target_physical = physical_focus["target"]
        candidate_physical = physical_focus["candidate"]
        tool_paths = {
            "objdiff": objdiff, "readelf": readelf, "ninja": ninja,
            "dtk": Path(loaded["dtk"]["path_object"]),
        }
        return {
            "compile_status": "success",
            "compile_performed": True,
            "compile_stdout_sha256": hashlib.sha256(compile_output.encode("utf-8")).hexdigest(),
            "candidate_source_sha256": candidate_sha,
            "candidate_object_sha256": _file_sha(candidate_object),
            "candidate_object_size": candidate_object.stat().st_size,
            "channels": {
                "strict": _channel_summary(channels["strict"]),
                "data": _channel_summary(channels["data"]),
            },
            "physical": {
                "status": physical_focus.get("status"),
                "target_count": target_physical.get("physical_relocation_count"),
                "candidate_count": candidate_physical.get("physical_relocation_count"),
                "difference_count": len(physical_focus.get("physical_relocation_differences", [])),
                "differences": physical_focus.get("physical_relocation_differences", []),
            },
            "protected_sibling_losses": {
                name: value.get("missing_exact_siblings", [])
                for name, value in gate.get("channels", {}).items()
            },
            "report_sha256": {
                "strict": _file_sha(strict_path), "data": _file_sha(data_path),
                "physical": _file_sha(physical_path),
            },
            "tool_sha256": {name: _file_sha(path) for name, path in tool_paths.items()},
            "active_seconds": time.monotonic() - started,
        }
    finally:
        if staged is not None:
            bundle._remove_staged_retail(staged)


def run_cell(
    *, root: Path, baseline: Path, candidate: Path, function: str, label: str,
    output: Path | None = None, toolchain: Path | None = None, timeout: float = 180.0,
) -> dict[str, Any]:
    repository = residual._root(root)
    if LABEL_RE.fullmatch(label) is None:
        raise CellRunnerError("label must be 1-80 safe filename characters")
    baseline_path = baseline if baseline.is_absolute() else repository / baseline
    candidate_path = candidate if candidate.is_absolute() else repository / candidate
    output_path = output or repository / "build" / "crack-cell" / f"{label}.json"
    if not output_path.is_absolute():
        output_path = repository / output_path
    output_path = output_path.resolve()
    try:
        output_path.relative_to(repository.resolve())
    except ValueError as exc:
        raise CellRunnerError("output must remain inside repository") from exc
    artifact, baseline_focus = _load_baseline(repository, baseline_path, function)
    if not candidate_path.is_file() or candidate_path.is_symlink():
        raise CellRunnerError(f"candidate source is not a regular file: {candidate_path}")
    candidate_sha = _file_sha(candidate_path)
    try:
        base_source = _authoritative_source(repository, candidate_path, artifact)
        _validate_candidate_cell(base_source, candidate_path, artifact)
    except CellRunnerError as exc:
        if not str(exc).startswith("STALE_CONTEXT:"):
            raise
        body = {
            "schema": SCHEMA, "label": label, "owner": artifact["owner"],
            "unit": artifact["unit"], "function": function,
            "base_commit": artifact["base_commit"],
            "base_source_sha256": artifact["base_sha256"],
            "candidate_source_sha256": candidate_sha,
            "target_object_sha256": artifact["target_sha256"],
            "baseline_residual_sha256": artifact["residual_sha256"],
            "status": "stale_context", "retryable": True,
            "reason": str(exc).split(":", 1)[1].strip(),
            "compile_performed": False, "cell_consumed": False,
            "cleanup_status": "not_needed", "authority_advanced": False,
            "source_retained": False,
        }
        result = {**body, "result_sha256": _json_sha(body)}
        _atomic_json(output_path, result)
        return result
    manifest = _resolve_toolchain(repository, toolchain, str(artifact["toolchain_key"]))
    worktree: Path | None = None
    measurement: dict[str, Any] | None = None
    cleanup_error: str | None = None
    try:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        worktree = residual._create_disposable_worktree(
            repository, output_path.parent, str(artifact["base_commit"]),
            process_timeout=timeout,
        )
        _validate_worktree_identity(repository, worktree, str(artifact["base_commit"]))
        measurement = _execute_candidate(
            repository=repository, worktree=worktree, artifact=artifact,
            baseline_focus=baseline_focus, base_source=base_source,
            candidate_source=candidate_path,
            toolchain_manifest=manifest, timeout=timeout,
        )
    finally:
        if worktree is not None:
            cleanup_error = residual._remove_disposable_worktree(
                repository, worktree, process_timeout=timeout
            )
    if cleanup_error is not None:
        raise CellRunnerError(cleanup_error)
    if measurement is None:
        raise CellRunnerError("candidate measurement ended without a result")
    if _file_sha(candidate_path) != candidate_sha:
        raise CellRunnerError("candidate source changed during measurement")
    body = {
        "schema": SCHEMA,
        "label": label,
        "owner": artifact["owner"], "unit": artifact["unit"],
        "function": function, "base_commit": artifact["base_commit"],
        "base_source_sha256": artifact["base_sha256"],
        "candidate_source_sha256": candidate_sha,
        "target_object_sha256": artifact["target_sha256"],
        "baseline_residual_sha256": artifact["residual_sha256"],
        "baseline_file_sha256": _file_sha(baseline_path),
        "toolchain_manifest_sha256": _file_sha(manifest),
        "producer_sha256": _file_sha(Path(__file__).resolve()),
        "measurement": measurement,
        "cleanup_status": "complete",
        "status": "measured" if measurement.get("compile_status") == "success" else "infrastructure_failed",
        "retryable": measurement.get("compile_status") != "success",
        "compile_performed": bool(measurement.get("compile_performed")),
        "cell_consumed": False,
        "authority_advanced": False, "source_retained": False,
    }
    result = {**body, "result_sha256": _json_sha(body)}
    _atomic_json(output_path, result)
    return result


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--function", required=True)
    parser.add_argument("--label", required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--toolchain", type=Path)
    parser.add_argument("--timeout", type=float, default=180.0)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        result = run_cell(
            root=args.root, baseline=args.baseline, candidate=args.candidate,
            function=args.function, label=args.label, output=args.output,
            toolchain=args.toolchain, timeout=args.timeout,
        )
    except (CellRunnerError, residual.ResidualEvidenceError, OSError) as exc:
        print(f"crack cell runner: {exc}", file=__import__("sys").stderr)
        return 2
    print(json.dumps(result, ensure_ascii=False, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
