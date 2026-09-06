"""One-command private recovery measurement; no live source writes or permits.

Consumes the existing current-evidence index, compiles one supplied natural-C
candidate (or measures an existing object), and compares the whole owner. Source
selection and final source fidelity remain the owner's job. This is not a source
generator, an exactness oracle, or a promotion mechanism.
Compiler recipes are trusted owner input, not arbitrary programs sandboxed by
this module. The evaluator itself writes only its private build artifacts.
"""
from __future__ import annotations

from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath, PureWindowsPath
import re
import sys
import tempfile
import time
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import bounded_process
from tools import compile_recovery_candidate as compiler
from tools import owner_campaign
from tools import recovery_frontier as frontier
from tools import recovery_object_inventory as objects

SCHEMA = "recovery_candidate_evaluation/v1"
BATCH_SCHEMA = "recovery_evaluation_batch/v1"
LIMIT = 512 * 1024
SEEN_LIMIT = 128
BATCH_MAX_JOBS = 8
BATCH_MAX_WORKERS = 3
_DIAGNOSTIC_FUNCTION_LIMIT = 3
_DIAGNOSTIC_CONTEXT_LIMIT = 2
_EVALUATE_STDOUT_LIMIT = 12 * 1024
_BATCH_ID = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,63}\Z")
_BATCH_DEVICE_IDS = {"CON", "PRN", "AUX", "NUL"} | {
    f"{prefix}{number}" for prefix in ("COM", "LPT") for number in range(1, 10)
}


def _sha(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _descriptor(path: Path) -> dict:
    return {"path": str(path), "sha256": compiler.digest(path), "size_bytes": path.stat().st_size}


def _implementation_binding() -> dict[str, str]:
    # A parser/classifier fix must invalidate cached measurements even when
    # this entry point did not change in the same release.
    paths = {Path(__file__), Path(frontier.__file__), Path(objects.__file__),
             Path(compiler.__file__), Path(bounded_process.__file__),
             Path(frontier.focus.__file__), Path(__file__).with_name("crack_evidence_bundle.py")}
    return {str(path): compiler.digest(path) for path in sorted(paths)}


def _atomic(path: Path, value: dict) -> None:
    data = frontier.canonical(value) + b"\n"
    if len(data) > LIMIT:
        raise ValueError(f"compact evaluation exceeds {LIMIT} bytes: {path}")
    compiler.atomic(path, data)


def _seen(root: Path, path: Path, key: str) -> dict | None:
    if not path.exists():
        return None
    raw, _ = frontier.read_bound(root, path, LIMIT)
    cache = frontier.load_json(raw)
    if cache.get("schema") != "recovery_evaluation_cache/v1":
        raise ValueError(f"invalid evaluation cache: {path}")
    for item in cache.get("entries", []):
        if item.get("key") != key:
            continue
        result_path = frontier.local(root, path.parent / item["result"])
        if result_path.parent != path.parent:
            raise ValueError("evaluation cache result escapes its directory")
        if result_path.is_file() and compiler.digest(result_path) == item.get("sha256"):
            old = frontier.load_json(result_path.read_bytes())
            if old.get("context_key") == key and old.get("status") != "failed":
                return {"path": str(result_path), "sha256": item["sha256"],
                        "status": old["status"], "candidate_sha256": old["candidate_source"]["sha256"]}
    return None


def _remember(root: Path, path: Path, key: str, out: Path) -> None:
    # Compact lookup, not an attempt cap. A different source/context is always
    # eligible; dropping an old lookup never bans a function.
    with owner_campaign._exclusive_lock(path.with_suffix(".lock"), timeout=5):
        if path.exists():
            raw, _ = frontier.read_bound(root, path, LIMIT)
            cache = frontier.load_json(raw)
            if cache.get("schema") != "recovery_evaluation_cache/v1":
                raise ValueError(f"invalid evaluation cache: {path}")
            entries = cache.get("entries", [])
        else:
            entries = []
        entries = [entry for entry in entries if entry.get("key") != key]
        entries.append({"key": key, "result": out.name, "sha256": compiler.digest(out)})
        _atomic(path, {"schema": "recovery_evaluation_cache/v1", "entries": entries[-SEEN_LIMIT:]})


def _metric_map(rows: list[dict]) -> dict[str, dict]:
    result = {row["function"]: dict(row) for row in rows}
    if len(result) != len(rows):
        raise ValueError("duplicate functions in evidence")
    for row in result.values():
        for field in ("target_bytes", "candidate_bytes", "diff_rows"):
            value = frontier.focus._integer(row[field])
            if value is None or value < 0:
                raise ValueError(f"invalid {field} for {row['function']}")
            row[field] = value
        value = row.get("match_percent")
        if value is not None and (isinstance(value, bool) or not isinstance(value, (int, float))
                                  or not math.isfinite(value) or not 0 <= value <= 100):
            raise ValueError(f"invalid percentage for {row['function']}")
    return result


def _classify(before_strict: list[dict], before_data: list[dict],
              after_strict: list[dict], after_data: list[dict],
              object_comparison: dict, functions: list[str]) -> dict:
    """Conservative measurement gate, independent of percentage-only ranking."""
    regressions, gains, changes = [], [], []
    focus = set(functions)
    for channel, before, after in (("strict", before_strict, after_strict),
                                    ("data", before_data, after_data)):
        left, right = _metric_map(before), _metric_map(after)
        if left.keys() != right.keys():
            regressions.append(f"{channel}: function census changed")
        for name in sorted(left.keys() & right.keys()):
            a, b = left[name], right[name]
            fields = ("candidate_bytes", "diff_rows", "instruction_exact", "match_percent")
            if any(a[k] != b[k] for k in fields):
                changes.append({"channel": channel, "function": name,
                                "before": {k: a[k] for k in fields},
                                "after": {k: b[k] for k in fields}})
            if a["instruction_exact"] and not b["instruction_exact"]:
                regressions.append(f"{channel}:{name}: exact function lost")
            if a["candidate_bytes"] == a["target_bytes"] and b["candidate_bytes"] != b["target_bytes"]:
                regressions.append(f"{channel}:{name}: exact size lost")
            if b["diff_rows"] > a["diff_rows"]:
                regressions.append(f"{channel}:{name}: differing rows increased")
            if a.get("match_percent") is not None and b.get("match_percent") is not None:
                if b["match_percent"] < a["match_percent"]:
                    regressions.append(f"{channel}:{name}: score regressed")
            if name in focus and b["diff_rows"] < a["diff_rows"]:
                gains.append(f"{channel}:{name}: {a['diff_rows']} -> {b['diff_rows']} differing rows")
    if not object_comparison.get("function_census_equal", False):
        regressions.append("object function census changed")
    for name, row in object_comparison.get("functions", {}).items():
        # Relocation placement can move when a .text function is slid.  The
        # inventory retains both views: raw physical rows for diagnostics and
        # normalized rows keyed by canonical callee identity.  Use the latter
        # for monotonic gates whenever present, retaining the raw fallback for
        # older inventories that predate normalized relocation fields.
        normalized_before = frontier.focus._integer(row.get("normalized_diff_before"))
        normalized_after = frontier.focus._integer(row.get("normalized_diff_after"))
        normalized_losses = row.get("closed_normalized_row_losses")
        normalized_loss_count = frontier.focus._integer(row.get("closed_normalized_row_loss_count"))
        use_normalized = (
            normalized_before is not None and normalized_before >= 0
            and normalized_after is not None and normalized_after >= 0
            and isinstance(normalized_losses, list)
            and normalized_loss_count is not None and normalized_loss_count >= 0
        )
        if use_normalized:
            if normalized_loss_count or normalized_losses:
                regressions.append(f"relocation:{name}: canonical relocation rows lost")
            if normalized_after > normalized_before:
                regressions.append(f"relocation:{name}: canonical differences increased")
            if name in focus and normalized_after < normalized_before:
                gains.append(f"relocation:{name}: {normalized_before} -> {normalized_after}")
        else:
            if row.get("closed_physical_row_losses"):
                regressions.append(f"physical:{name}: previously exact relocation rows lost")
            physical_before = frontier.focus._integer(row.get("physical_diff_before"))
            physical_after = frontier.focus._integer(row.get("physical_diff_after"))
            physical_before = 0 if physical_before is None else physical_before
            physical_after = 0 if physical_after is None else physical_after
            if physical_after > physical_before:
                regressions.append(f"physical:{name}: differences increased")
            if name in focus and physical_after < physical_before:
                gains.append(f"physical:{name}: {physical_before} -> {physical_after}")
    # Nontext movement can be a legitimate pool repair. Do not call it a safe
    # retained gain without its separate typed-value/consumer proof.
    review = ["allocated nontext changed; typed data/consumer review required"] if object_comparison.get("allocated_nontext_changed") else []
    strict, data = _metric_map(after_strict), _metric_map(after_data)
    exact = all(name in strict and name in data and strict[name]["instruction_exact"]
                and data[name]["instruction_exact"]
                and object_comparison.get("functions", {}).get(name, {}).get("raw_exact_target")
                and object_comparison.get("functions", {}).get(name, {}).get("candidate_physical_exact")
                for name in functions)
    status = "rejected" if regressions else "exact" if exact else "improved" if gains else "no_gain"
    return {"status": status, "exact_scope": "selected_functions_only", "owner_exact": False,
            "gains": gains, "regressions": sorted(set(regressions)),
            "review_required": review, "metric_changes": changes,
            "retention_ready": status in {"exact", "improved"} and not review}


def _command_context(root: Path, command_json: Path, tools: list[Path]) -> dict:
    command = compiler.load_command_json(command_json)
    if command.count("{source}") != 1 or command.count("{object}") != 1:
        raise ValueError("compiler argv requires exactly one {source} and one {object} argument")
    if any(("{source}" in item and item != "{source}") or
           ("{object}" in item and item != "{object}") for item in command):
        raise ValueError("compiler placeholders must be complete argv cells")
    command = compiler._resolved_command(command, root)
    paths = {Path(command[0]), *(Path(os.path.abspath(root / path)) for path in tools)}
    if not tools:
        raise ValueError("name the compiler executable with --compiler-tool (also when using a wrapper)")
    return {"argv_template": command, "command_json": _descriptor(command_json),
            "tools": {str(path): compiler.digest(path) for path in sorted(paths)},
            "headers": compiler.tree(root / "include"),
            "generated_headers": compiler.tree(root / "build/GP6E01/include"),
            "environment_sha256": _sha(frontier.canonical(dict(os.environ)))}


def _same_baseline_context(root: Path, base: dict, context: dict) -> bool:
    descriptor = base.get("inputs", {}).get("compile_receipt")
    if base.get("compile_binding") != "receipt_hashes_match" or not descriptor:
        return False
    raw, actual = frontier.read_bound(root, Path(descriptor["path"]), frontier.INDEX_LIMIT)
    if actual != descriptor:
        raise ValueError("baseline compiler receipt drifted")
    return frontier.load_json(raw).get("context_sha256") == _sha(frontier.canonical(context))


def _compile_candidate(root: Path, candidate: Path, output: Path, command_json: Path,
                       compiler_tools: list[Path], timeout: float) -> dict:
    context = _command_context(root, command_json, compiler_tools)
    source = _descriptor(candidate)
    argv = [str(candidate) if item == "{source}" else str(output) if item == "{object}" else item
            for item in context["argv_template"]]
    # A fresh invocation-owned path is never allowed to reuse a stale object.
    if output.exists():
        raise ValueError(f"compiler object already exists: {output}")
    started = time.monotonic()
    with owner_campaign._exclusive_lock(root / "build/.compiler-lane.lock", timeout=timeout):
        if context != _command_context(root, command_json, compiler_tools):
            raise ValueError("compiler context drifted before launch")
        remaining = timeout - (time.monotonic() - started)
        result = bounded_process.run(argv, cwd=root, timeout=remaining, max_output=256 * 1024)
    compiler.atomic(output.with_suffix(".stdout.log"), result.stdout)
    compiler.atomic(output.with_suffix(".stderr.log"), result.stderr)
    if result.returncode:
        raise ValueError(f"compiler failed ({result.returncode}): {compiler._diagnostics(result.stdout, result.stderr)}")
    if _descriptor(candidate) != source or context != _command_context(root, command_json, compiler_tools):
        raise ValueError("source or compiler context changed during compilation")
    if not output.is_file() or not 0 < output.stat().st_size <= 16 * 1024 * 1024:
        raise ValueError("compiler did not produce a bounded nonempty object")
    return {"schema": "recovery_candidate_compile/v1", "source_sha256": source["sha256"],
            "object_sha256": compiler.digest(output), "context_sha256": _sha(frontier.canonical(context)),
            "command": argv, "context": context,
            "seconds": time.monotonic() - started, "stdout_sha256": _sha(result.stdout),
            "stderr_sha256": _sha(result.stderr)}


def _report(objdiff: Path, target: Path, candidate: Path, output: Path,
            root: Path, data: bool, timeout: float) -> None:
    args = [str(objdiff), "diff", "-1", str(target), "-2", str(candidate), "-o", str(output), "--format", "json"]
    if data:
        args += ["-c", "functionRelocDiffs=data_value"]
    result = bounded_process.run(args, cwd=root, timeout=timeout, max_output=256 * 1024)
    if result.returncode:
        raise ValueError(f"objdiff failed ({result.returncode}): {compiler._diagnostics(result.stdout, result.stderr)}")
    if not output.is_file() or output.stat().st_size > frontier.REPORT_LIMIT:
        raise ValueError("objdiff report missing or exceeds bounded limit")


def _validate_report_objects(document: dict, target: dict, candidate: dict) -> None:
    """Reject omitted/extra functions and metadata not matching actual objects."""
    for side, obj in (("left", target), ("right", candidate)):
        rows = frontier.focus._symbols(document, side, "candidate report")
        actual = {r["name"]: int(r["size"]) for r in rows if frontier.focus._is_function(r)}
        expected = {name: row["size"] for name, row in obj["functions"].items()}
        if actual != expected:
            raise ValueError(f"{side} report function census/size differs from actual object")
        for row in rows:
            if not frontier.focus._is_function(row):
                continue
            instructions = frontier.focus._rows(row, row["name"])
            if frontier.focus._instruction_count(instructions) * 4 != expected[row["name"]]:
                raise ValueError(f"{side} report instruction coverage differs from object: {row['name']}")


def _focus_evidence(document: dict, functions: list[str]) -> dict:
    result = {}
    sides = {side: frontier.focus._symbols(document, side, "candidate") for side in ("left", "right")}
    for name in functions:
        left = frontier.focus._rows(frontier._stack_function(sides["left"], name, "target"), name)
        right = frontier.focus._rows(frontier._stack_function(sides["right"], name, "candidate"), name)
        summary = frontier._diagnose_rows(left, right)
        differing = []
        for i in range(max(len(left), len(right))):
            a, b = left[i] if i < len(left) else {}, right[i] if i < len(right) else {}
            if frontier._diagnose_kind(a) or frontier._diagnose_kind(b):
                differing.append({"row": i, "target": frontier.instruction(a),
                                  "candidate": frontier.instruction(b),
                                  "kind": a.get("diff_kind") or b.get("diff_kind")})
        summary.update(residuals=differing[:24], residuals_total=len(differing),
                       residuals_truncated=len(differing) > 24)
        result[name] = summary
    return result


def _diagnostic_rows(document: dict, name: str) -> tuple[list[dict], list[dict]] | None:
    sides = {side: frontier.focus._symbols(document, side, "changed-result diagnostic")
             for side in ("left", "right")}
    target = frontier._stack_function(sides["left"], name, "target")
    candidate = frontier._stack_function(sides["right"], name, "candidate")
    if target is None or candidate is None:
        return None
    return (frontier.focus._rows(target, name), frontier.focus._rows(candidate, name))


def _diagnostic_mismatches(target: list[dict], candidate: list[dict]) -> dict[int, tuple[Any, Any]]:
    result: dict[int, tuple[Any, Any]] = {}
    for index in range(max(len(target), len(candidate))):
        target_payload = frontier._diagnose_payload(target[index] if index < len(target) else None, index)
        candidate_payload = frontier._diagnose_payload(candidate[index] if index < len(candidate) else None, index)
        if target_payload != candidate_payload:
            result[index] = (target_payload, candidate_payload)
    return result


def _diagnostic_context(target: list[dict], candidate: list[dict], center: int) -> list[dict]:
    result = []
    for index in (center - 1, center + 1):
        if 0 <= index < max(len(target), len(candidate)):
            result.append({"row": index,
                           "target": frontier._diagnose_row(target[index] if index < len(target) else None, index),
                           "candidate": frontier._diagnose_row(candidate[index] if index < len(candidate) else None, index)})
    return result[:_DIAGNOSTIC_CONTEXT_LIMIT]


def _compact_first_mismatch(summary: dict | None) -> dict | None:
    first = summary.get("first_instruction_mismatch") if isinstance(summary, dict) else None
    if not isinstance(first, dict):
        return None
    return {key: first[key] for key in ("row", "kind", "target", "candidate") if key in first}


def _changed_result_diagnostics(*, root: Path, baseline_documents: dict[str, dict],
                                after_documents: dict[str, dict], strict_path: Path,
                                data_path: Path, metric_changes: list[dict],
                                object_comparison: dict, reports_retained: bool = False) -> dict:
    changes_by_function: dict[str, dict[str, dict]] = {}
    for change in metric_changes:
        if isinstance(change, dict) and isinstance(change.get("function"), str):
            changes_by_function.setdefault(change["function"], {})[change.get("channel", "unknown")] = change
    object_rows = object_comparison.get("functions", {}) if isinstance(object_comparison, dict) else {}
    if not isinstance(object_rows, dict):
        object_rows = {}
    for name, row in object_rows.items():
        if isinstance(row, dict) and row.get("raw_equal_base") is False:
            changes_by_function.setdefault(name, {})
    raw_names = [name for name, row in object_rows.items()
                 if isinstance(row, dict) and row.get("raw_equal_base") is False]
    names = raw_names + [name for name in changes_by_function if name not in raw_names]
    selected = names[:_DIAGNOSTIC_FUNCTION_LIMIT]
    result: dict[str, Any] = {"diagnostic_only": True, "status": "none" if not names else "bounded",
                              "affected_function_count": len(names), "truncated": len(names) > len(selected),
                              "functions": []}
    for name in selected:
        object_row = object_rows.get(name, {}) if isinstance(object_rows, dict) else {}
        if not isinstance(object_row, dict):
            object_row = {}
        if "raw_equal_base" not in object_row:
            object_relation = "unknown"
        elif not object_row["raw_equal_base"]:
            object_relation = "raw_changed"
        else:
            base_relocations = object_row.get("base_relocations")
            candidate_relocations = object_row.get("candidate_relocations")
            if (isinstance(base_relocations, dict) and isinstance(candidate_relocations, dict)
                    and isinstance(base_relocations.get("sha256"), str)
                    and isinstance(candidate_relocations.get("sha256"), str)):
                object_relation = ("relocation_only" if base_relocations["sha256"] != candidate_relocations["sha256"]
                                   else "unchanged")
            else:
                object_relation = "unknown"
        item: dict[str, Any] = {"function": name, "object_relation": object_relation,
                                "metric_changes": {channel: {"before": change.get("before"), "after": change.get("after")}
                                                    for channel, change in changes_by_function[name].items()},
                                "diagnose_argv": (["python", "tools/recovery_frontier.py", "--root", str(root),
                                                   "diagnose", "--strict", str(strict_path), "--data", str(data_path),
                                                   "--function", name] if reports_retained else None),
                                "diagnose_argv_available": reports_retained, "channels": {}}
        for channel, path in (("strict", strict_path), ("data", data_path)):
            after = after_documents.get(channel)
            before = baseline_documents.get(channel)
            channel_item: dict[str, Any] = {}
            try:
                after_rows = _diagnostic_rows(after, name) if after is not None else None
                before_rows = _diagnostic_rows(before, name) if before is not None else None
                if after_rows is None or before_rows is None:
                    raise ValueError("report function missing")
                before_target, before_candidate = before_rows
                after_target, after_candidate = after_rows
                after_summary = frontier._diagnose_rows(after_target, after_candidate)
                before_summary = frontier._diagnose_rows(before_target, before_candidate)
                channel_item["current_first_instruction_mismatch"] = _compact_first_mismatch(after_summary)
                channel_item["existing_first_instruction_mismatch"] = _compact_first_mismatch(before_summary)
                if len(before_target) != len(after_target) or len(before_candidate) != len(after_candidate):
                    raise ValueError("aligned report row count changed")
                for index in range(len(before_target)):
                    if frontier._diagnose_payload(before_target[index], index) != frontier._diagnose_payload(after_target[index], index):
                        raise ValueError("target code stream changed")
                old = _diagnostic_mismatches(before_target, before_candidate)
                new = _diagnostic_mismatches(after_target, after_candidate)
                introduced = sorted(set(new) - set(old))
                changed_existing = sorted(index for index in set(new) & set(old) if new[index] != old[index])
                if introduced:
                    row = introduced[0]
                    channel_item.update(status="new_code_difference", row=row,
                                        mismatch={"target": frontier._diagnose_row(after_target[row], row),
                                                  "candidate": frontier._diagnose_row(after_candidate[row], row)},
                                        context=_diagnostic_context(after_target, after_candidate, row))
                elif changed_existing:
                    channel_item.update(status="unknown", reason="existing row changed; not_newly_proven")
                elif new:
                    channel_item.update(status="existing", reason="canonical code mismatch predates candidate",
                                        row=min(new))
                else:
                    channel_item.update(status="none")
            except (OSError, ValueError, RuntimeError, KeyError, TypeError, IndexError, AttributeError) as exc:
                channel_item.update(status="unknown", reason=f"{str(exc)[:180]}; not_newly_proven")
            item["channels"][channel] = channel_item
        result["functions"].append(item)
    result["unknown_count"] = sum(
        1 for item in result["functions"] for channel in item["channels"].values()
        if channel.get("status") == "unknown"
    )
    return result


def _dispatch_summary(result: dict) -> dict:
    summary = {key: result[key] for key in
               ("status", "functions", "compiler_runs", "objdiff_runs", "retention_ready", "retained", "seconds", "cleanup_errors")}
    summary.update(gains=result.get("gains", [])[:8], regressions=result.get("regressions", [])[:8],
                   regression_count=len(result.get("regressions", [])), reason=result.get("reason"),
                   changed_result_diagnostics=result.get("changed_result_diagnostics"))
    encoded = json.dumps(summary, sort_keys=True).encode("utf-8")
    if len(encoded) > _EVALUATE_STDOUT_LIMIT:
        diagnostic = result.get("changed_result_diagnostics") or {}
        summary["changed_result_diagnostics"] = {
            "diagnostic_only": True, "status": diagnostic.get("status", "unknown"),
            "affected_function_count": diagnostic.get("affected_function_count", 0),
            "unknown_count": diagnostic.get("unknown_count", 0), "truncated": True,
        }
    if len(json.dumps(summary, sort_keys=True).encode("utf-8")) > _EVALUATE_STDOUT_LIMIT:
        diagnostic = result.get("changed_result_diagnostics") or {}
        summary = {
            "status": str(result.get("status", "unknown"))[:128],
            "functions": [str(value)[:96] for value in result.get("functions", [])[:8]],
            "function_count": len(result.get("functions", [])),
            "compiler_runs": result.get("compiler_runs", 0), "objdiff_runs": result.get("objdiff_runs", 0),
            "retention_ready": bool(result.get("retention_ready", False)),
            "retained": bool(result.get("retained", False)), "seconds": result.get("seconds"),
            "cleanup_error_count": len(result.get("cleanup_errors", [])),
            "gains_count": len(result.get("gains", [])), "regressions_count": len(result.get("regressions", [])),
            "reason": str(result.get("reason") or "")[:500],
            "changed_result_diagnostics": {
                "diagnostic_only": True, "status": diagnostic.get("status", "unknown"),
                "affected_function_count": diagnostic.get("affected_function_count", 0),
                "unknown_count": diagnostic.get("unknown_count", 0), "truncated": True,
            },
            "stdout_truncated": True,
        }
    return summary


def _cleanup(directory: Path, root: Path) -> list[str]:
    """Remove only this invocation's explicit, flat temporary output directory."""
    errors = []
    try:
        frontier.local(root, directory).relative_to(root / "build")
        for path in directory.iterdir():
            frontier.local(root, path)
            if not path.is_file():
                raise ValueError(f"unexpected non-file in private evaluation directory: {path}")
            path.unlink()
        directory.rmdir()
    except (OSError, ValueError) as exc:
        errors.append(str(exc))
    return errors


def evaluate(*, root: Path, index: Path, candidate: Path, functions: list[str], out: Path,
             objdiff: Path, readelf: Path, command_json: Path | None = None,
             compiler_tools: list[Path] | None = None, candidate_object: Path | None = None,
             timeout: float = 120, keep_reports: bool = False) -> dict:
    """Measure a candidate without modifying source, baseline, queue or permits."""
    started = time.monotonic()
    root = Path(os.path.abspath(root))
    index, candidate, out = [frontier.local(root, p) for p in (index, candidate, out)]
    out.relative_to(root / "build")
    if not functions or len(set(functions)) != len(functions):
        raise ValueError("distinct focus function names are required")
    if bool(command_json) == bool(candidate_object):
        raise ValueError("supply exactly one compiler command JSON or existing candidate object")
    if not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("positive finite evaluation deadline required")
    if out.exists():
        raise ValueError(f"evaluation result already exists: {out}")
    if out.name in {"seen.json", "seen.lock"}:
        raise ValueError("evaluation output uses a reserved cache name")
    raw, index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    base = frontier.load_json(raw)
    frontier.verify(root, base)
    if base.get("data_functions") is None:
        raise ValueError("baseline index must include strict and data reports")
    if not set(functions) <= {r["function"] for r in base["functions"]}:
        raise ValueError("focus function absent from current evidence")
    source_bytes, source_desc = frontier.read_bound(root, candidate, 4 * 1024 * 1024)
    source_sha = _sha(source_bytes)
    bound_paths = {frontier.local(root, Path(v["path"])) for v in base["inputs"].values()}
    if out in bound_paths | {index, candidate}:
        raise ValueError("evaluation output aliases an input")
    target = frontier.local(root, Path(base["inputs"]["target_object"]["path"]))
    baseline = frontier.local(root, Path(base["inputs"]["candidate_object"]["path"]))
    result: dict[str, Any] = {"schema": SCHEMA, "owner": base["owner"], "functions": functions,
        "authority_advanced": False, "retained": False, "retention_ready": False,
        "linked_exact": None, "source_fidelity": "owner_review_required",
        "baseline_index": index_desc, "candidate_source": source_desc,
        "compiler_runs": 0, "objdiff_runs": 0, "cleanup_errors": [], "stage": "preflight"}
    directory = None
    preserved = []
    cache_path = out.parent / "seen.json"
    try:
        compile_context = (_command_context(root, frontier.local(root, command_json), compiler_tools or [])
                           if command_json else None)
        if (candidate == frontier.local(root, Path(base["inputs"]["source"]["path"]))
                and source_sha == base["inputs"]["source"]["sha256"] and compile_context is not None
                and _same_baseline_context(root, base, compile_context)):
            result.update(status="duplicate_source", reason="candidate is the retained source; nothing to compile")
        else:
            def remaining() -> float:
                seconds = timeout - (time.monotonic() - started)
                if seconds <= 0:
                    raise TimeoutError("evaluation deadline exhausted")
                return seconds
            objdiff = Path(os.path.abspath(objdiff))
            readelf = Path(os.path.abspath(readelf))
            proof_tools = {str(p): compiler.digest(p) for p in (objdiff, readelf)}
            result["proof_tools"] = proof_tools
            context = (compile_context if command_json else
                       {"existing_object": _descriptor(frontier.local(root, candidate_object))})
            result["implementation"] = _implementation_binding()
            key = _sha(frontier.canonical({"index": base["index_sha256"], "source": source_desc,
                       "focus": functions, "compiler": context, "proof_tools": proof_tools,
                       "implementation": result["implementation"]}))
            result["context_key"] = key
            cached = _seen(root, cache_path, key)
            if cached is not None:
                result.update(status="duplicate_source", reason="this exact candidate/context was already measured",
                              reused_measurement=cached)
                frontier.verify(root, base)
                if frontier.read_bound(root, candidate, 4 * 1024 * 1024)[1] != source_desc:
                    raise ValueError("candidate source changed during duplicate lookup")
                result["stage"] = "complete"
                return result
            out.parent.mkdir(parents=True, exist_ok=True)
            directory = Path(tempfile.mkdtemp(prefix=".evaluate-", dir=out.parent))
            if candidate_object is None:
                result["stage"] = "compile"
                candidate_obj = directory / "candidate.o"
                result["compiler_runs"] = 1
                receipt = _compile_candidate(root, candidate, candidate_obj,
                    frontier.local(root, command_json), compiler_tools or [], remaining())
                if (receipt.get("schema") != "recovery_candidate_compile/v1"
                        or receipt.get("source_sha256") != source_sha
                        or receipt.get("object_sha256") != compiler.digest(candidate_obj)):
                    raise ValueError("compiler receipt does not bind this candidate source/object")
                result["compile_receipt"] = receipt
                result["compile_binding"] = "compiler_receipt"
            else:
                candidate_obj = frontier.local(root, candidate_object)
                if candidate_obj == out:
                    raise ValueError("output aliases existing candidate object")
                result["compile_binding"] = "caller_supplied_object; not source proof"
            result["stage"] = "object_inventory"
            target_inventory, base_inventory, candidate_inventory = [objects.inventory(p) for p in (target, baseline, candidate_obj)]
            result["candidate_object"] = _descriptor(candidate_obj)
            result["semantic_object_sha256"] = candidate_inventory["semantic_sha256"]
            result["semantic_object_equal_baseline"] = candidate_inventory["semantic_sha256"] == base_inventory["semantic_sha256"]
            comparison = objects.compare(target_inventory, base_inventory, candidate_inventory, functions)
            result["object_comparison"] = comparison
            if result["semantic_object_equal_baseline"]:
                result.update(status="duplicate_object", reason="allocated object/link semantics unchanged; skip objdiff")
            else:
                result["stage"] = "proof"
                for path in (target, baseline, candidate_obj):
                    check = bounded_process.run([str(readelf), "-SWsWr", "--", str(path)],
                                                cwd=root, timeout=remaining(), max_output=512 * 1024)
                    if check.returncode:
                        raise ValueError(f"independent readelf rejected {path}: {check.returncode}")
                strict_path, data_path = directory / "strict.json", directory / "data.json"
                with ThreadPoolExecutor(max_workers=2) as pool:
                    tasks = [pool.submit(_report, objdiff, target, candidate_obj, path, root, data, remaining())
                             for path, data in ((strict_path, False), (data_path, True))]
                    result["objdiff_runs"] = 2
                    for task in tasks:
                        task.result()
                summaries = {}
                after_documents = {}
                result["next_mismatch_evidence"] = {}
                for channel, path in (("strict", strict_path), ("data", data_path)):
                    document = frontier.load_json(path.read_bytes())
                    after_documents[channel] = document
                    _validate_report_objects(document, target_inventory, candidate_inventory)
                    summaries[channel] = frontier.summarize(document, channel)
                    result[channel + "_report"] = _descriptor(path)
                    result[channel + "_functions"] = summaries[channel]
                    result["next_mismatch_evidence"][channel] = _focus_evidence(document, functions)
                result.update(_classify(base["functions"], base["data_functions"],
                                        summaries["strict"], summaries["data"], comparison, functions))
                baseline_documents = {}
                try:
                    for channel in ("strict", "data"):
                        descriptor = base.get("inputs", {}).get(channel + "_report")
                        if not isinstance(descriptor, dict):
                            raise ValueError(f"baseline {channel} report descriptor missing")
                        baseline_raw, actual = frontier.read_bound(root, Path(descriptor["path"]), frontier.REPORT_LIMIT)
                        if actual != descriptor:
                            raise ValueError(f"baseline {channel} report changed")
                        baseline_document = frontier.load_json(baseline_raw)
                        if not isinstance(baseline_document, dict):
                            raise ValueError(f"baseline {channel} report is not an object")
                        baseline_documents[channel] = baseline_document
                except (OSError, ValueError, RuntimeError, KeyError, TypeError):
                    baseline_documents = {}
                result["changed_result_diagnostics"] = _changed_result_diagnostics(
                    root=root, baseline_documents=baseline_documents, after_documents=after_documents,
                    strict_path=strict_path, data_path=data_path, metric_changes=result.get("metric_changes", []),
                    object_comparison=comparison, reports_retained=keep_reports,
                )
                result["focus"] = {channel: [r for r in summaries[channel] if r["function"] in functions]
                                   for channel in ("strict", "data")}
                if candidate_object is not None:
                    result["retention_ready"] = False
                if base.get("compile_binding") != "receipt_hashes_match":
                    result["review_required"].append("baseline source/object compile receipt absent; use existing owner proof")
                    result["retention_ready"] = False
                result["next_action"] = ("owner source-fidelity review then retain verified candidate" if result["retention_ready"]
                    else "use first_mismatch and closed-channel regressions; do not repeat the same candidate")
            if any(compiler.digest(Path(path)) != sha for path, sha in proof_tools.items()):
                raise ValueError("proof executable changed during evaluation")
            if _descriptor(candidate_obj) != result["candidate_object"]:
                raise ValueError("candidate object changed during evaluation")
        frontier.verify(root, base)
        if frontier.read_bound(root, candidate, 4 * 1024 * 1024)[1] != source_desc:
            raise ValueError("candidate source changed during evaluation")
        if result.get("status") in {"exact", "improved"} and candidate_object is None and directory is not None:
            # Preserve only the useful source/object/receipt, not two whole-TU
            # reports per attempt. A measured gain still needs its object when
            # source fidelity or typed data requires review: do not force an
            # otherwise unnecessary rebuild just because readiness is false.
            for suffix, payload in (("candidate.c", source_bytes),
                                    ("candidate.o", candidate_obj.read_bytes()),
                                    ("compile.json", frontier.canonical(result["compile_receipt"]) + b"\n")):
                destination = out.with_name(out.stem + "." + suffix)
                frontier.local(root, destination)
                if destination.exists() or destination in bound_paths | {index, candidate}:
                    raise ValueError(f"verified candidate output already exists or aliases input: {destination}")
                compiler.atomic(destination, payload)
                preserved.append(destination)
            result["measured_candidate"] = {p.suffix if p.suffix != ".json" else "receipt": _descriptor(p) for p in preserved}
            if result.get("retention_ready"):
                result["verified_candidate"] = result["measured_candidate"]
        result["stage"] = "complete"
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        result.update(status="failed", reason=str(exc)[:8000], retention_ready=False)
        if isinstance(exc, bounded_process.ProcessLimitError):
            result["diagnostics"] = compiler._diagnostics(exc.stdout, exc.stderr)
    finally:
        if result.get("status") == "failed":
            for path in preserved:
                try:
                    frontier.local(root, path).unlink()
                except (OSError, ValueError) as exc:
                    result["cleanup_errors"].append(str(exc))
        if directory is not None:
            if keep_reports and result.get("stage") == "complete":
                result["artifacts"] = {p.name: _descriptor(p) for p in directory.iterdir() if p.is_file()}
                result["retained_private_directory"] = str(directory)
            else:
                result["cleanup_errors"].extend(_cleanup(directory, root))
        if result["cleanup_errors"]:
            result["retention_ready"] = False
        result["seconds"] = time.monotonic() - started
        result["result_sha256"] = _sha(frontier.canonical(result))
        _atomic(out, result)
        if result.get("context_key") and result.get("status") not in {"failed", "duplicate_source"}:
            try:
                _remember(root, cache_path, result["context_key"], out)
            except (OSError, ValueError, RuntimeError) as exc:
                # Cache maintenance cannot erase a successfully measured gain.
                result["cache_warning"] = str(exc)
                result.pop("result_sha256", None)
                result["result_sha256"] = _sha(frontier.canonical(result))
                _atomic(out, result)
    return result


def _batch_relative(root: Path, value: Any, field: str) -> Path:
    """Resolve a manifest path while requiring an owner-root-relative name."""
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{field} must be a non-empty relative path")
    text = value.replace("\\", "/")
    windows = PureWindowsPath(text)
    parts = PurePosixPath(text).parts
    if (windows.is_absolute() or windows.drive or text.startswith("/")
            or not parts or any(part in {"", ".", ".."} for part in parts)):
        raise ValueError(f"{field} must be owner-root-relative")
    return frontier.local(root, root.joinpath(*parts))


def _batch_file(root: Path, value: Any, field: str, limit: int) -> tuple[Path, dict]:
    path = _batch_relative(root, value, field)
    if not path.is_file():
        raise ValueError(f"{field} is not a file: {value}")
    _, descriptor = frontier.read_bound(root, path, limit)
    return path, descriptor


def _batch_manifest(root: Path, manifest: Path) -> tuple[dict, dict, list[dict]]:
    if not isinstance(manifest, Path):
        manifest = Path(manifest)
    raw, manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
    try:
        document = frontier.load_json(raw)
    except (ValueError, TypeError, json.JSONDecodeError) as exc:
        raise ValueError(f"malformed batch manifest: {manifest}") from exc
    if not isinstance(document, dict) or set(document) != {"schema", "jobs"}:
        raise ValueError("batch manifest must contain only schema and jobs")
    if document.get("schema") != BATCH_SCHEMA:
        raise ValueError(f"unsupported batch manifest schema: {document.get('schema')!r}")
    raw_jobs = document.get("jobs")
    if not isinstance(raw_jobs, list) or not raw_jobs or len(raw_jobs) > BATCH_MAX_JOBS:
        raise ValueError(f"batch jobs must contain 1..{BATCH_MAX_JOBS} entries")
    jobs = []
    ids = set()
    folded_ids = set()
    for number, item in enumerate(raw_jobs, 1):
        if not isinstance(item, dict):
            raise ValueError(f"job {number} must be an object")
        allowed = {"id", "candidate", "functions", "candidate_object"}
        if set(item) - allowed or not {"id", "candidate", "functions"} <= set(item):
            raise ValueError(f"job {number} must contain exactly id, candidate, functions and optional candidate_object")
        job_id = item["id"]
        device_name = job_id.upper().rstrip(".").split(".", 1)[0] if isinstance(job_id, str) else ""
        if (not isinstance(job_id, str) or not _BATCH_ID.fullmatch(job_id)
                or job_id.endswith(".") or device_name in _BATCH_DEVICE_IDS
                or job_id in ids or job_id.casefold() in folded_ids):
            raise ValueError(f"job {number} has an unsafe or duplicate id")
        ids.add(job_id)
        folded_ids.add(job_id.casefold())
        functions = item["functions"]
        if (not isinstance(functions, list) or not functions or len(functions) > 128
                or any(not isinstance(name, str) or not name.strip() for name in functions)
                or len(set(functions)) != len(functions)):
            raise ValueError(f"job {job_id} functions must be distinct non-empty names")
        candidate, candidate_desc = _batch_file(root, item["candidate"],
                                                 f"job {job_id} candidate", 4 * 1024 * 1024)
        candidate_object = candidate_object_desc = None
        if "candidate_object" in item:
            candidate_object, candidate_object_desc = _batch_file(
                root, item["candidate_object"], f"job {job_id} candidate_object", 64 * 1024 * 1024)
        jobs.append({"id": job_id, "candidate": candidate, "candidate_desc": candidate_desc,
                     "functions": functions, "candidate_object": candidate_object,
                     "candidate_object_desc": candidate_object_desc})
    return document, manifest_desc, jobs


def _batch_tools(objdiff: Path, readelf: Path) -> dict[str, str]:
    paths = {"objdiff": Path(os.path.abspath(objdiff)), "readelf": Path(os.path.abspath(readelf))}
    if any(not path.is_file() for path in paths.values()):
        raise ValueError("objdiff and readelf must be existing files")
    return {name: compiler.digest(path) for name, path in paths.items()}


def _batch_snapshot(root: Path, index: Path, manifest: Path, manifest_desc: dict,
                    jobs: list[dict], command_json: Path | None, compiler_tools: list[Path],
                    objdiff: Path, readelf: Path, base: dict) -> dict:
    context = (_command_context(root, command_json, compiler_tools) if command_json else None)
    _, index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    _, actual_manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
    if actual_manifest_desc != manifest_desc:
        raise ValueError("batch manifest changed during preflight")
    return {"index": index_desc, "manifest": actual_manifest_desc,
            "jobs": {job["id"]: {"candidate": job["candidate_desc"],
                                  "candidate_object": job["candidate_object_desc"]}
                     for job in jobs},
            "command_context": context,
            "proof_tools": _batch_tools(objdiff, readelf),
            "base_index_sha256": base.get("index_sha256"),
            "implementation": _implementation_binding()}


def _batch_drift(root: Path, index: Path, manifest: Path, snapshot: dict, jobs: list[dict],
                 command_json: Path | None, compiler_tools: list[Path],
                 objdiff: Path, readelf: Path, base: dict) -> list[str]:
    reasons = []
    try:
        current_raw, current_index_desc = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
        if current_index_desc != snapshot["index"]:
            reasons.append("index changed")
        current_base = frontier.load_json(current_raw)
        frontier.verify(root, current_base)
        if current_base.get("index_sha256") != snapshot["base_index_sha256"]:
            reasons.append("index identity changed")
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        reasons.append(f"index verification failed: {str(exc)[:300]}")
    try:
        _, current_manifest_desc = frontier.read_bound(root, manifest, frontier.INDEX_LIMIT)
        if current_manifest_desc != snapshot["manifest"]:
            reasons.append("manifest changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"manifest changed: {str(exc)[:300]}")
    for job in jobs:
        expected = snapshot["jobs"][job["id"]]
        try:
            _, current_candidate_desc = frontier.read_bound(root, job["candidate"], 4 * 1024 * 1024)
            if current_candidate_desc != expected["candidate"]:
                reasons.append(f"candidate changed: {job['id']}")
            if job["candidate_object"] is not None:
                _, current_object_desc = frontier.read_bound(root, job["candidate_object"], 64 * 1024 * 1024)
                if current_object_desc != expected["candidate_object"]:
                    reasons.append(f"candidate object changed: {job['id']}")
        except (OSError, ValueError) as exc:
            reasons.append(f"candidate changed: {job['id']}: {str(exc)[:300]}")
    if command_json:
        try:
            if _command_context(root, command_json, compiler_tools) != snapshot["command_context"]:
                reasons.append("compiler context changed")
        except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
            reasons.append(f"compiler context changed: {str(exc)[:300]}")
    try:
        current_tools = _batch_tools(objdiff, readelf)
        if current_tools != snapshot["proof_tools"]:
            reasons.append("proof tool changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"proof tool changed: {str(exc)[:300]}")
    try:
        if _implementation_binding() != snapshot["implementation"]:
            reasons.append("evaluator implementation changed")
    except (OSError, ValueError) as exc:
        reasons.append(f"evaluator implementation changed: {str(exc)[:300]}")
    return list(dict.fromkeys(reasons))


def _batch_failure(job: dict, reason: str, status: str = "failed") -> dict:
    return {"schema": SCHEMA, "owner": "batch", "functions": job["functions"],
            "status": status, "reason": reason[:8000], "compiler_runs": 0,
            "objdiff_runs": 0, "cleanup_errors": [], "retention_ready": False,
            "retained": False, "authority_advanced": False, "stage": "batch"}


def _batch_run(root: Path, index: Path, job: dict, result_path: Path,
               objdiff: Path, readelf: Path, command_json: Path | None,
               compiler_tools: list[Path], timeout: float) -> dict:
    kwargs = {"root": root, "index": index, "candidate": job["candidate"],
              "functions": job["functions"], "out": result_path, "objdiff": objdiff,
              "readelf": readelf, "timeout": timeout}
    if job["candidate_object"] is not None:
        # A per-job object is an explicit replay request.  It wins over an
        # optional common recipe in mixed manifests; no compile is performed.
        kwargs["candidate_object"] = job["candidate_object"]
    elif command_json is not None:
        kwargs.update(command_json=command_json, compiler_tools=compiler_tools)
    else:
        raise ValueError("batch job has neither candidate_object nor common command_json")
    return evaluate(**kwargs)


def _batch_result_descriptor(root: Path, path: Path) -> dict:
    raw, descriptor = frontier.read_bound(root, path, LIMIT)
    # Loading here catches a corrupt/mock result before it enters the summary.
    document = frontier.load_json(raw)
    if not isinstance(document, dict):
        raise ValueError(f"batch result is not an object: {path}")
    return descriptor


def _batch_alias_result(job: dict, canonical: dict, canonical_path: Path,
                        canonical_id: str, root: Path) -> dict:
    status = canonical.get("status")
    if status in {"failed", "not_scheduled"}:
        return _batch_failure(job, f"collapsed with {status} job {canonical_id}", status)
    result = {"schema": SCHEMA, "owner": "batch", "functions": job["functions"],
              "status": "duplicate_source", "reason": f"identical candidate/context collapsed with {canonical_id}",
              "reused_batch_job": canonical_id, "reused_result": _batch_result_descriptor(root, canonical_path),
              "compiler_runs": 0, "objdiff_runs": 0, "cleanup_errors": [],
              "retention_ready": False, "retained": False, "authority_advanced": False}
    for key in ("gains", "regressions", "strict", "data", "focus"):
        if key in canonical:
            result[key] = canonical[key]
    return result


def _batch_improvement_rows(result: dict) -> int:
    structured = 0
    has_structured = False
    focus = set(result.get("functions", []))
    for change in result.get("metric_changes", []):
        if not isinstance(change, dict) or (focus and change.get("function") not in focus):
            continue
        before, after = change.get("before"), change.get("after")
        if isinstance(before, dict) and isinstance(after, dict):
            try:
                has_structured = True
                structured += max(0, int(before.get("diff_rows", 0)) - int(after.get("diff_rows", 0)))
            except (TypeError, ValueError):
                pass
    if has_structured:
        for value in result.get("gains", []):
            if str(value).startswith(("relocation:", "physical:")):
                match = re.search(r":\s*(-?\d+)\s*->\s*(-?\d+)", str(value))
                if match:
                    structured += max(0, int(match.group(1)) - int(match.group(2)))
        return structured
    total = 0
    for value in result.get("gains", []):
        match = re.search(r":\s*(-?\d+)\s*->\s*(-?\d+)", str(value))
        if match:
            total += max(0, int(match.group(1)) - int(match.group(2)))
    return total


def evaluate_batch(*, root: Path, index: Path, manifest: Path, out: Path,
                   objdiff: Path, readelf: Path, command_json: Path | None = None,
                   compiler_tools: list[Path] | None = None, workers: int = 2,
                   timeout: float = 120) -> dict:
    """Measure a bounded batch of independent candidates using ``evaluate``.

    The manifest and every input are frozen before the first worker is launched.
    Batch scheduling is deliberately measurement-only: successful results are
    retained as per-job evaluator files, while composition and adoption remain
    explicit owner decisions.
    """
    started = time.monotonic()
    root = Path(os.path.abspath(root))
    index = frontier.local(root, Path(index))
    manifest = frontier.local(root, Path(manifest))
    out = frontier.local(root, Path(out))
    out.relative_to(root / "build")
    if out.exists():
        raise ValueError(f"batch result already exists: {out}")
    if not isinstance(workers, int) or isinstance(workers, bool) or not 1 <= workers <= BATCH_MAX_WORKERS:
        raise ValueError(f"workers must be between 1 and {BATCH_MAX_WORKERS}")
    if not isinstance(timeout, (int, float)) or isinstance(timeout, bool) or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("positive finite batch deadline required")
    _, manifest_desc, jobs = _batch_manifest(root, manifest)
    if bool(command_json) and all(job["candidate_object"] is not None for job in jobs):
        # A supplied common recipe is harmless for replay, but it still becomes
        # part of the frozen context and catches tool drift consistently.
        command_json = frontier.local(root, Path(command_json))
    elif command_json:
        command_json = frontier.local(root, Path(command_json))
    compiler_tools = [Path(os.path.abspath(root / Path(tool))) for tool in (compiler_tools or [])]
    if command_json is None:
        if compiler_tools:
            raise ValueError("compiler_tools require a common command_json")
        if any(job["candidate_object"] is None for job in jobs):
            raise ValueError("common command_json is required unless every job supplies candidate_object")
    elif not command_json.is_file():
        raise ValueError(f"command_json is not a file: {command_json}")
    objdiff = Path(os.path.abspath(objdiff))
    readelf = Path(os.path.abspath(readelf))
    raw, _ = frontier.read_bound(root, index, frontier.INDEX_LIMIT)
    base = frontier.load_json(raw)
    frontier.verify(root, base)
    if base.get("data_functions") is None:
        raise ValueError("baseline index must include strict and data reports")
    known_functions = {row["function"] for row in base.get("functions", [])}
    for job in jobs:
        if not set(job["functions"]) <= known_functions:
            raise ValueError(f"job {job['id']} focus function absent from current evidence")
    snapshot = _batch_snapshot(root, index, manifest, manifest_desc, jobs, command_json,
                               compiler_tools, objdiff, readelf, base)
    job_dir = frontier.local(root, out.parent / (out.stem + ".jobs"))
    job_dir.relative_to(root / "build")
    if job_dir.exists():
        raise ValueError(f"batch output directory already exists: {job_dir}")
    job_dir.mkdir(parents=True, exist_ok=False)
    result_paths = {job["id"]: job_dir / f"{job['id']}.json" for job in jobs}
    for path in result_paths.values():
        frontier.local(root, path)
    # Collapse only byte/context/function-equivalent jobs. Paths themselves do
    # not participate, so aliases are measured once and get explicit reuse files.
    groups: dict[str, list[dict]] = {}
    context_key = _sha(frontier.canonical({"command": snapshot["command_context"],
                                           "index": snapshot["index"]["sha256"],
                                           "proof": snapshot["proof_tools"]}))
    for job in jobs:
        key = _sha(frontier.canonical({"candidate": job["candidate_desc"]["sha256"],
                                       "candidate_object": (job["candidate_object_desc"] or {}).get("sha256"),
                                       "functions": sorted(job["functions"]), "context": context_key}))
        groups.setdefault(key, []).append(job)
    canonical = [entries[0] for entries in groups.values()]
    aliases = {job["id"]: entries[0] for entries in groups.values() for job in entries[1:]}
    results: dict[str, dict] = {}
    drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                 compiler_tools, objdiff, readelf, base)
    pending = list(canonical)
    active = {}
    def schedule(pool: ThreadPoolExecutor) -> None:
        nonlocal drift_reasons
        while pending and len(active) < workers and not drift_reasons:
            drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                         compiler_tools, objdiff, readelf, base)
            if drift_reasons:
                break
            job = pending.pop(0)
            active[pool.submit(_batch_run, root, index, job, result_paths[job["id"]],
                               objdiff, readelf, command_json, compiler_tools, timeout)] = job
    with ThreadPoolExecutor(max_workers=workers) as pool:
        schedule(pool)
        while active:
            done, _ = wait(tuple(active), return_when=FIRST_COMPLETED)
            for future in done:
                job = active.pop(future)
                try:
                    result = future.result()
                    if not isinstance(result, dict) or not isinstance(result.get("status"), str):
                        raise ValueError("evaluate returned a non-object result")
                except Exception as exc:
                    result = _batch_failure(job, str(exc))
                if not result_paths[job["id"]].is_file():
                    _atomic(result_paths[job["id"]], result)
                results[job["id"]] = result
            if not drift_reasons:
                drift_reasons = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                                             compiler_tools, objdiff, readelf, base)
            schedule(pool)
    for job in pending:
        result = _batch_failure(job, "batch input drift stopped scheduling", "not_scheduled")
        _atomic(result_paths[job["id"]], result)
        results[job["id"]] = result
    # Fan out aliases only after their canonical result is stable.
    for job in jobs:
        if job["id"] in aliases:
            canonical_job = aliases[job["id"]]
            canonical_path = result_paths[canonical_job["id"]]
            result = _batch_alias_result(job, results[canonical_job["id"]], canonical_path,
                                         canonical_job["id"], root)
            _atomic(result_paths[job["id"]], result)
            results[job["id"]] = result
    final_drift = _batch_drift(root, index, manifest, snapshot, jobs, command_json,
                               compiler_tools, objdiff, readelf, base)
    drift_reasons = list(dict.fromkeys(drift_reasons + final_drift))
    records = []
    result_map = {}
    positive = []
    for job in jobs:
        result = results[job["id"]]
        descriptor = _batch_result_descriptor(root, result_paths[job["id"]])
        record = {"id": job["id"], "status": result.get("status"),
                  "canonical_id": aliases.get(job["id"], job)["id"],
                  "candidate": job["candidate_desc"], "functions": job["functions"],
                  "candidate_object": job["candidate_object_desc"],
                  "result": descriptor, "compiler_runs": result.get("compiler_runs", 0),
                  "objdiff_runs": result.get("objdiff_runs", 0),
                  "retention_ready": bool(result.get("retention_ready", False))}
        records.append(record)
        result_map[job["id"]] = {"status": record["status"], "result": descriptor,
                                  "gains": result.get("gains", [])[:16]}
        if record["status"] in {"exact", "improved"} and job["id"] not in aliases:
            gains = result.get("gains", [])
            positive.append({"id": job["id"], "status": record["status"],
                             "gains": gains[:16], "result": descriptor,
                             "improvement_rows": _batch_improvement_rows(result),
                             "retention_ready": record["retention_ready"]})
    positive.sort(key=lambda item: (item["status"] != "exact", -item["improvement_rows"],
                                    -len(item["gains"]), item["id"]))
    statuses = [record["status"] for record in records]
    if drift_reasons:
        status = "drifted"
    elif any(item == "failed" for item in statuses):
        status = "partial" if any(item in {"exact", "improved"} for item in statuses) else "failed"
    else:
        status = "complete"
    summary = {"schema": BATCH_SCHEMA, "status": status, "manifest": manifest_desc,
               "baseline_index": snapshot["index"], "context_sha256": context_key,
               "proof_tools": snapshot["proof_tools"],
               "jobs": records, "results": result_map,
               "measured_gains": positive, "best_positive_candidates": positive[:3],
               "drift_detected": bool(drift_reasons), "drift_reasons": drift_reasons[:16],
               "retention_ready": bool(positive) and not drift_reasons
                   and all(item["retention_ready"] for item in positive),
               "authority_advanced": False, "retained": False, "adoption_ready": False,
               "composition": None, "compiler_runs": sum(r["compiler_runs"] for r in records),
               "objdiff_runs": sum(r["objdiff_runs"] for r in records),
               "seconds": time.monotonic() - started}
    _atomic(out, summary)
    return summary


def add_batch_arguments(parser: Any) -> None:
    parser.add_argument("--index", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, required=True)
    parser.add_argument("--readelf", type=Path, required=True)
    parser.add_argument("--command-json", type=Path)
    parser.add_argument("--compiler-tool", type=Path, action="append", dest="compiler_tools")
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument("--timeout", type=float, default=120)


def dispatch_batch(args: Any) -> int:
    values = vars(args).copy()
    values.pop("action", None)
    result = evaluate_batch(**values)
    summary = {key: result[key] for key in
               ("status", "drift_detected", "drift_reasons", "compiler_runs", "objdiff_runs", "retention_ready", "seconds")}
    summary.update(jobs=[{"id": row["id"], "status": row["status"]} for row in result["jobs"]],
                   best_positive_candidates=result["best_positive_candidates"])
    print(json.dumps(summary, sort_keys=True))
    return 2 if result["status"] in {"failed", "drifted"} else 0


def add_arguments(parser: Any) -> None:
    parser.add_argument("--index", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--function", action="append", required=True, dest="functions")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, required=True)
    parser.add_argument("--readelf", type=Path, required=True)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--command-json", type=Path)
    group.add_argument("--candidate-object", type=Path)
    parser.add_argument("--compiler-tool", type=Path, action="append", dest="compiler_tools")
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--keep-reports", action="store_true")


def dispatch(args: Any) -> int:
    values = vars(args).copy()
    values.pop("action", None)
    result = evaluate(**values)
    print(json.dumps(_dispatch_summary(result), sort_keys=True))
    return 2 if result["status"] == "failed" else 0


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    add_arguments(parser)
    raise SystemExit(dispatch(parser.parse_args()))
