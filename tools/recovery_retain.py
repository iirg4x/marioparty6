"""Independently reproduce a reviewed gain and publish a recoverable frontier.

No git, queue, permit or clean-main authority is changed. Source and index are
separate atomic replacements, bridged by one bounded durable pending journal.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import sys
import time

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import compile_recovery_candidate as compiler
from tools import recovery_evaluate as evaluator
from tools import recovery_frontier as frontier
from tools.recovery_search_memory import SearchMemory

LIMIT = 256 * 1024
EVALUATION_LIMIT = 8 * 1024 * 1024


def _local(root: Path, path: Path | str) -> Path:
    return frontier.local(root, Path(path))


def _desc(path: Path) -> dict:
    return {"path": str(path), "sha256": compiler.digest(path), "size_bytes": path.stat().st_size}


def _check(root: Path, desc: dict) -> Path:
    path = _local(root, desc["path"])
    if compiler.digest(path) != desc["sha256"] or path.stat().st_size != desc["size_bytes"]:
        raise ValueError(f"retention artifact drift: {path}")
    return path


def _read(path: Path, limit: int = LIMIT) -> dict:
    if path.stat().st_size > limit:
        raise ValueError(f"retention record too large: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def _write(path: Path, doc: dict) -> None:
    data = frontier.canonical(doc) + b"\n"
    if len(data) > LIMIT:
        raise ValueError(f"retention record too large: {path}")
    compiler.atomic(path, data)


def _gate(doc: dict, index_sha: str, source_sha: str, functions: list[str]) -> None:
    if (doc.get("schema") != evaluator.SCHEMA or doc.get("status") not in {"exact", "improved"}
            or doc.get("regressions") != [] or doc.get("review_required") != []
            or doc.get("cleanup_errors") != [] or doc.get("stage") != "complete"
            or doc.get("baseline_index", {}).get("sha256") != index_sha
            or doc.get("candidate_source", {}).get("sha256") != source_sha
            or sorted(doc.get("functions", [])) != sorted(functions)):
        raise ValueError("measurement is not a bound, reviewed, regression-free gain")


def _context(root: Path, spec: dict) -> dict:
    return compiler.preflight_context(root=root, scratch=_local(root, spec["scratch"]),
                                      command=spec["command"], tools=[Path(p) for p in spec["tools"]])


def _advance_memory(root: Path, index: Path, parent_sha: str) -> dict:
    """Advisory continuation only: never make a published gain depend on cache."""
    try:
        memory = SearchMemory(root, root / "build/recovery-search-memory.json")
        try:
            descriptor = memory.advance_frontier(index, parent_sha)
        except ValueError:
            # A resumed publication may already have advanced this exact index.
            # The memory API still checks its parent atomically on this retry.
            descriptor = memory.advance_frontier(index, compiler.digest(index))
        return {"status": "advanced", "frontier": descriptor}
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as exc:
        return {"status": "warning", "reason": str(exc)[:800]}


def _resume(root: Path, directory: Path) -> dict:
    journal = directory / "pending.json"
    doc = _read(journal)
    if doc.get("schema") != "recovery_retention_pending/v1":
        raise ValueError(f"invalid retention journal: {journal}")
    live, index = [_local(root, doc[k]["path"]) for k in ("old_source", "old_index")]
    index.relative_to(root / "build")
    if not live.is_relative_to(root / "src"):
        raise ValueError("retention source must remain under src/")
    for descriptor in doc["artifacts"]:
        _check(root, descriptor)
    for descriptor in doc.get("proof_tools", []):
        if _desc(Path(descriptor["path"])) != descriptor:
            raise ValueError("retention proof implementation/tool drift")
    if compiler.context_digest(_context(root, doc["compile_context"])) != doc["compile_context_sha256"]:
        raise ValueError("retention compiler/header context drift")
    staged_source = _check(root, doc["new_source"])
    staged_index = _check(root, doc["new_index"])
    source_sha, index_sha = compiler.digest(live), compiler.digest(index)
    if source_sha not in {doc["old_source"]["sha256"], doc["new_source"]["sha256"]}:
        raise ValueError("concurrent source change; pending gain preserved")
    if index_sha not in {doc["old_index"]["sha256"], doc["new_index"]["sha256"]}:
        raise ValueError("concurrent index change; pending gain preserved")
    if index_sha == doc["new_index"]["sha256"] and source_sha != doc["new_source"]["sha256"]:
        raise ValueError("published index has unexpected source; refusing overwrite")
    value = _read(staged_index)
    # Check every proof input before the first live write, substituting the
    # staged source for the one input which is deliberately not published yet.
    for role, descriptor in value["inputs"].items():
        if role == "source":
            if _local(root, descriptor["path"]) != live or descriptor["sha256"] != doc["new_source"]["sha256"]:
                raise ValueError("pending frontier source binding differs")
        else:
            _check(root, descriptor)
    payload = {k: v for k, v in value.items() if k != "index_sha256"}
    if hashlib.sha256(frontier.canonical(payload)).hexdigest() != value.get("index_sha256"):
        raise ValueError("pending frontier digest differs")
    if source_sha != doc["new_source"]["sha256"]:
        if compiler.digest(index) != index_sha or compiler.digest(live) != source_sha:
            raise ValueError("concurrent source/index drift before source publication")
        compiler.atomic(live, staged_source.read_bytes())
    if compiler.digest(live) != doc["new_source"]["sha256"]:
        raise ValueError("source changed after publication; pending gain preserved")
    if compiler.digest(index) != index_sha:
        raise ValueError("index changed after source publication; pending gain preserved")
    if index_sha != doc["new_index"]["sha256"]:
        frontier.publish(root, index, value)
    if compiler.digest(index) != doc["new_index"]["sha256"]:
        raise ValueError("index publication differs; pending gain preserved")
    frontier.verify(root, value)
    memory = _advance_memory(root, index, doc["old_index"]["sha256"])
    journal.unlink()
    result = {"status": "retained", "retained": True, "source": _desc(live), "index": _desc(index),
              "authority_advanced": False, "linked_exact": None, "search_memory": memory}
    if "source_lineage" in doc:
        result["source_lineage"] = doc["source_lineage"]
    return result


def resume_pending(root: Path, out_dir: Path) -> dict:
    root = Path(root).resolve()
    directory = _local(root, out_dir)
    directory.relative_to(root / "build")
    with SearchMemory(root, root / "build/recovery-retention-lock.json")._lock():
        return _resume(root, directory)


def _retain_gain(root: Path, index: Path, candidate: Path, functions: list[str],
                measured_result: Path, scratch: Path, source_relpath: str,
                object_relpath: str, compiler_script: Path, objdiff: Path,
                readelf: Path, tools: list[Path] | None, out_dir: Path, timeout: float = 120,
                source_reviewed: bool = False, working_source: dict | None = None) -> dict:
    if isinstance(timeout, bool) or not isinstance(timeout, (int, float)) or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("retention timeout must be finite and positive")
    deadline = time.monotonic() + timeout

    def remaining() -> float:
        seconds = deadline - time.monotonic()
        if seconds <= 0:
            raise TimeoutError("retention overall deadline exhausted")
        return seconds

    if source_reviewed is not True:
        raise ValueError("explicit source review is required for retention")
    root = Path(root).resolve()
    index, candidate, measured_result, scratch, directory = [
        _local(root, p) for p in (index, candidate, measured_result, scratch, out_dir)]
    live = _local(root, source_relpath)
    live.relative_to(root / "src")
    index.relative_to(root / "build")
    directory.relative_to(root / "build")
    if candidate == live or directory.exists():
        raise ValueError("retention requires private candidate and new output directory")
    script = compiler._tool_path(compiler_script, root)
    command = ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script)]
    tool_paths = [script, *(tools or [])]
    with SearchMemory(root, root / "build/recovery-retention-lock.json")._lock():
        old_source, old_index, source_desc, result_desc = map(_desc, (live, index, candidate, measured_result))
        base = _read(index)
        frontier.verify(root, base)
        if _local(root, base["inputs"]["source"]["path"]) != live:
            raise ValueError("retention requires the live champion as measurement baseline")
        measured = _read(measured_result, EVALUATION_LIMIT)
        _gate(measured, old_index["sha256"], source_desc["sha256"], functions)
        lineage = measured.get("source_lineage")
        working = lineage.get("working_source") if isinstance(lineage, dict) else None
        if working_source is not None:
            if working is not None and working != working_source:
                raise ValueError("requested working lineage differs from measurement")
            working = working_source
            if lineage is None:
                lineage = evaluator.source_lineage(root, index, base, working, candidate.read_bytes())
        if lineage is not None:
            if evaluator.source_lineage(root, index, base, working, candidate.read_bytes()) != lineage:
                raise ValueError("retention source lineage differs from measured lineage")
        # Search batches measure a frozen copy; exact bytes bind the selected
        # candidate without requiring its user-facing filename to be identical.
        _check(root, measured["candidate_source"])
        expected_object = _check(root, measured["candidate_object"])
        context_spec = {"scratch": str(scratch), "command": command, "tools": [str(p) for p in tool_paths]}
        context = _context(root, context_spec)
        watched = [old_source, old_index, source_desc, result_desc, _desc(expected_object),
                   _desc(_local(root, measured["candidate_source"]["path"]))]
        watched += [_desc(Path(p).resolve()) for p in (objdiff, readelf)]
        watched += [_desc(Path(p)) for p in evaluator._implementation_binding()]
        if working is not None:
            _, lineage_watched = evaluator.working_source_binding(root, index, base, working)
            watched += [_desc(Path(p)) for p in lineage_watched]

        def unchanged() -> None:
            remaining()
            for descriptor in watched:
                # Proof executables can live outside the owner root.
                path = Path(descriptor["path"])
                if _desc(path) != descriptor:
                    raise ValueError(f"retention input drift: {path}")
            if _context(root, context_spec) != context:
                raise ValueError("retention compiler/header context drift")
            frontier.verify(root, base)

        unchanged()
        directory.mkdir(parents=True, exist_ok=False)
        frozen = directory / "candidate.c"
        compiler.atomic(frozen, candidate.read_bytes())
        output = directory / "candidate.o"
        receipt = compiler.compile_candidate(root=root, scratch=scratch, source=frozen, output=output,
            source_relpath=source_relpath, object_relpath=object_relpath, command=command,
            tools=tool_paths, timeout=remaining())
        receipt_path = output.with_suffix(output.suffix + ".receipt.json")
        disk_receipt = _read(receipt_path)
        if (disk_receipt != receipt or receipt.get("schema") != "recovery_candidate_compile/v1"
                or receipt.get("source_sha256") != source_desc["sha256"]
                or receipt.get("object_sha256") != compiler.digest(output)
                or receipt.get("object_sha256") != measured["candidate_object"]["sha256"]
                or receipt.get("context_sha256") != compiler.context_digest(context)
                or receipt.get("tools") != context["tools"]
                or receipt.get("header_set_sha256") != hashlib.sha256(json.dumps(context["headers"], sort_keys=True).encode()).hexdigest()
                or receipt.get("generated_header_set_sha256") != hashlib.sha256(json.dumps(context["generated_headers"], sort_keys=True).encode()).hexdigest()):
            raise ValueError("independent compiler receipt/source/object/context differs")
        unchanged()
        lineage_kwargs = {"working_source": working} if working is not None else {}
        fresh = evaluator.evaluate(root=root, index=index, candidate=frozen, functions=functions,
            out=directory / "evaluation.json", objdiff=Path(objdiff), readelf=Path(readelf),
            candidate_object=output, timeout=remaining(), keep_reports=True, **lineage_kwargs)
        _gate(fresh, old_index["sha256"], source_desc["sha256"], functions)
        if lineage is not None and fresh.get("source_lineage") != lineage:
            raise ValueError("fresh evaluation source lineage differs")
        if fresh.get("candidate_object", {}).get("sha256") != compiler.digest(output):
            raise ValueError("fresh evaluation object differs")
        strict = _check(root, fresh["artifacts"]["strict.json"])
        data = _check(root, fresh["artifacts"]["data.json"])
        value = frontier.snapshot(root=root, owner=base["owner"], source=frozen,
            target=_local(root, base["inputs"]["target_object"]["path"]), candidate=output,
            strict=strict, data=data, toolchain_key=base["toolchain_key"], compile_receipt=receipt_path)
        value["inputs"]["source"]["path"] = live.relative_to(root).as_posix()
        value.pop("index_sha256", None)
        value["index_sha256"] = hashlib.sha256(frontier.canonical(value)).hexdigest()
        staged_index = directory / "frontier.json"
        _write(staged_index, value)
        unchanged()
        # Tools outside root are rebound through compiler context; proof tools
        # remain explicitly hashed and are rechecked in _resume below.
        proof_watch = [_desc(output), _desc(receipt_path), _desc(strict), _desc(data),
                       _desc(directory / "evaluation.json")]
        pending = {"schema": "recovery_retention_pending/v1",
            "old_source": old_source, "old_index": old_index, "new_source": _desc(frozen),
            "new_index": _desc(staged_index), "artifacts": proof_watch,
            "proof_tools": [_desc(Path(p).resolve()) for p in (objdiff, readelf)]
                           + [_desc(Path(p)) for p in evaluator._implementation_binding()],
            "compile_context": context_spec, "compile_context_sha256": compiler.context_digest(context)}
        if lineage is not None:
            pending["source_lineage"] = lineage
            pending["artifacts"] += [_desc(Path(p)) for p in lineage_watched]
        _write(directory / "pending.json", pending)
        return _resume(root, directory)


def retain_gain(root: Path, index: Path, candidate: Path, functions: list[str],
                measured_result: Path, scratch: Path, source_relpath: str,
                object_relpath: str, compiler_script: Path, objdiff: Path,
                readelf: Path, tools: list[Path] | None, out_dir: Path, timeout: float = 120,
                source_reviewed: bool = False, working_source: dict | None = None) -> dict:
    """Retain under one deadline; failed pre-journal attempts keep compact proof."""
    root = Path(root).resolve()
    directory = _local(root, out_dir)
    directory.relative_to(root / "build")
    existed = directory.exists()
    try:
        return _retain_gain(root, index, candidate, functions, measured_result, scratch,
                            source_relpath, object_relpath, compiler_script, objdiff,
                            readelf, tools, directory, timeout, source_reviewed, working_source)
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as exc:
        if not existed and directory.is_dir() and not (directory / "pending.json").exists():
            # Only named files in this newly created attempt are disposable.
            # Never recurse or erase the source, receipt, compact result or journal.
            paths = [directory / "candidate.o", directory / "strict.json", directory / "data.json"]
            for scratch_dir in directory.glob(".evaluate-*"):
                if scratch_dir.is_dir() and not scratch_dir.is_symlink():
                    paths.extend(scratch_dir / name for name in ("candidate.o", "strict.json", "data.json"))
            errors = []
            for path in paths:
                try:
                    resolved = _local(root, path)
                    if resolved.is_relative_to(directory):
                        resolved.unlink(missing_ok=True)
                except (OSError, ValueError) as cleanup:
                    errors.append(str(cleanup)[:300])
            _write(directory / "failure.json", {"status": "failed", "retained": False,
                "reason": str(exc)[:2000], "cleanup_errors": errors[:8], "authority_advanced": False})
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--resume-pending", action="store_true")
    parser.add_argument("--source-reviewed", action="store_true")
    for name in ("index", "candidate", "measured-result", "scratch", "compiler-script", "objdiff", "readelf"):
        parser.add_argument("--" + name, type=Path)
    for name in ("source-relpath", "object-relpath"):
        parser.add_argument("--" + name)
    parser.add_argument("--function", dest="functions", action="append")
    parser.add_argument("--tool", dest="tools", action="append", type=Path, default=[])
    parser.add_argument("--timeout", type=float, default=120)
    args = vars(parser.parse_args())
    try:
        resume = args.pop("resume_pending")
        if resume:
            result = resume_pending(args["root"], args["out_dir"])
        else:
            if any(args[k] is None for k in ("index", "candidate", "measured_result", "scratch", "compiler_script", "objdiff", "readelf", "source_relpath", "object_relpath", "functions")):
                raise ValueError("retention requires all candidate, compiler and measurement inputs")
            result = retain_gain(**args)
        print(json.dumps(result, sort_keys=True))
        return 0
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as exc:
        print(f"retention: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
