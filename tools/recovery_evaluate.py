"""One-command private recovery measurement; no live source writes or permits.

Consumes the existing current-evidence index, compiles one supplied natural-C
candidate (or measures an existing object), and compares the whole owner. Source
selection and final source fidelity remain the owner's job. This is not a source
generator, an exactness oracle, or a promotion mechanism.
Compiler recipes are trusted owner input, not arbitrary programs sandboxed by
this module. The evaluator itself writes only its private build artifacts.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import math
import os
from pathlib import Path
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
LIMIT = 512 * 1024
SEEN_LIMIT = 128


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
                result["next_mismatch_evidence"] = {}
                for channel, path in (("strict", strict_path), ("data", data_path)):
                    document = frontier.load_json(path.read_bytes())
                    _validate_report_objects(document, target_inventory, candidate_inventory)
                    summaries[channel] = frontier.summarize(document, channel)
                    result[channel + "_report"] = _descriptor(path)
                    result[channel + "_functions"] = summaries[channel]
                    result["next_mismatch_evidence"][channel] = _focus_evidence(document, functions)
                result.update(_classify(base["functions"], base["data_functions"],
                                        summaries["strict"], summaries["data"], comparison, functions))
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
    summary = {k: result[k] for k in ("status", "functions", "compiler_runs", "objdiff_runs", "retention_ready", "retained", "seconds", "cleanup_errors")}
    summary.update(gains=result.get("gains", [])[:8], regressions=result.get("regressions", [])[:8],
                   regression_count=len(result.get("regressions", [])), reason=result.get("reason"))
    print(json.dumps(summary, sort_keys=True))
    return 2 if result["status"] == "failed" else 0


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    add_arguments(parser)
    raise SystemExit(dispatch(parser.parse_args()))
