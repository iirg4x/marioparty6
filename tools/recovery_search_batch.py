"""Compile 1..8 root-reviewed natural-C candidates, then measure the whole owner.

Manifest schema recovery_search_batch/v1: root_reviewed=true, causal_family,
baseline_index_sha256, live_source_sha256, and candidates containing id, source,
sha256, functions, and evidence [{path, sha256}]. Review is a declaration of
human/root source review, not an organicity proof or a new permit mechanism.
No candidate generation, live source adoption, or frontier mutation occurs.
"""
from __future__ import annotations

import argparse
import json
import math
import os
from pathlib import Path
import sys
import time

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import compile_recovery_candidate as compiler
from tools import recovery_evaluate as evaluator
from tools import recovery_frontier as frontier
from tools import recovery_search_memory
from tools import recovery_causal_groups

SCHEMA = "recovery_search_batch/v1"
MAX_CANDIDATES = 8
MAX_SOURCE_BYTES = 4 * 1024 * 1024


def _usable_cached(root: Path, entry: dict | None) -> dict | None:
    """Reject stale/cyclic alias chains before reusing a source or object."""
    if not entry:
        return None
    descriptor = entry["result"]
    visited = set()
    try:
        for _ in range(128):
            path = frontier.local(root, Path(descriptor["path"]))
            if path in visited:
                return None
            visited.add(path)
            raw, actual = frontier.read_bound(root, path, evaluator.LIMIT)
            if actual["sha256"] != descriptor["sha256"]:
                return None
            document = frontier.load_json(raw)
            if document.get("status") in {None, "failed", "not_scheduled", "drifted"}:
                return None
            descriptor = document.get("canonical_measurement")
            if descriptor is None:
                return entry
    except (OSError, ValueError, KeyError, TypeError):
        return None
    return None


def _bound(root: Path, value: str | Path, expected: str, limit: int) -> tuple[Path, bytes]:
    path = frontier.local(root, Path(value))
    raw, descriptor = frontier.read_bound(root, path, limit)
    if descriptor["sha256"] != expected:
        raise ValueError(f"input hash mismatch: {path}")
    return path, raw


def _load(root: Path, manifest: Path, index: Path, live: Path) -> tuple[dict, list[dict], dict]:
    raw, desc = frontier.read_bound(root, manifest, 128 * 1024)
    doc = frontier.load_json(raw)
    if not isinstance(doc, dict) or doc.get("schema") != SCHEMA or doc.get("root_reviewed") is not True:
        raise ValueError(f"{manifest}: expected root-reviewed {SCHEMA}")
    family = doc.get("causal_family")
    if not isinstance(family, str) or not family.strip() or len(family) > 1000:
        raise ValueError("one bounded causal_family description is required")
    _bound(root, index, doc.get("baseline_index_sha256"), frontier.INDEX_LIMIT)
    _bound(root, live, doc.get("live_source_sha256"), MAX_SOURCE_BYTES)
    items = doc.get("candidates")
    if not isinstance(items, list) or not 1 <= len(items) <= MAX_CANDIDATES:
        raise ValueError("candidates must contain 1..8 entries")
    frozen = {str(manifest): desc["sha256"], str(index): compiler.digest(index), str(live): compiler.digest(live)}
    if doc.get("working_source") is not None:
        base = frontier.load_json(index.read_bytes())
        _, working_frozen = evaluator.working_source_binding(root, index, base, doc["working_source"])
        frozen.update(working_frozen)
    jobs, ids = [], set()
    for item in items:
        if not isinstance(item, dict):
            raise ValueError("candidate must be an object")
        name = item.get("id")
        if (not isinstance(name, str) or not evaluator._BATCH_ID.fullmatch(name)
                or name.endswith(".") or name.upper().split(".")[0] in evaluator._BATCH_DEVICE_IDS
                or name.casefold() in ids):
            raise ValueError("unsafe or duplicate candidate id")
        ids.add(name.casefold())
        source, data = _bound(root, item["source"], item["sha256"], MAX_SOURCE_BYTES)
        if source.suffix.lower() not in {".c", ".cp", ".cpp"}:
            raise ValueError(f"candidate must be a C/C++ source file: {source}")
        functions = item.get("functions")
        if (not isinstance(functions, list) or not functions or len(functions) > 128
                or any(not isinstance(v, str) or not v.strip() for v in functions)
                or len(set(functions)) != len(functions)):
            raise ValueError(f"{name}: distinct focus functions required")
        evidence = item.get("evidence")
        if not isinstance(evidence, list) or not 1 <= len(evidence) <= 16:
            raise ValueError(f"{name}: 1..16 hash-bound evidence descriptors required")
        for entry in evidence:
            path, _ = _bound(root, entry["path"], entry["sha256"], frontier.REPORT_LIMIT)
            frozen[str(path)] = entry["sha256"]
        frozen[str(source)] = item["sha256"]
        if item.get("hypothesis_binding") is not None:
            item["hypothesis_binding"] = recovery_search_memory.normalize_hypothesis_binding(item["hypothesis_binding"])
            generation_sha = doc["working_source"]["sha256"] if doc.get("working_source") is not None else doc["live_source_sha256"]
            if item["hypothesis_binding"]["source_sha256"] != generation_sha:
                raise ValueError("hypothesis binding must name the canonical generation source")
        if doc.get("working_source") is not None:
            item["source_lineage"] = evaluator.source_lineage(root, index, base, doc["working_source"], data)
        jobs.append(dict(item, source=source, data=data))
    return doc, jobs, frozen


def run_batch(*, root: Path, index: Path, scratch: Path, source_relpath: str,
              object_relpath: str, compiler_script: Path, manifest: Path, out: Path,
              objdiff: Path, readelf: Path, tools: list[Path] | None = None,
              workers: int = 2, timeout: float = 120) -> dict:
    if isinstance(timeout, bool) or not isinstance(timeout, (int, float)) or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError("batch timeout must be finite and positive")
    deadline = time.monotonic() + timeout

    def remaining() -> float:
        seconds = deadline - time.monotonic()
        if seconds <= 0:
            raise TimeoutError("batch overall deadline exhausted")
        return seconds

    root = Path(os.path.abspath(root))
    index, scratch, manifest, out = [frontier.local(root, Path(p)) for p in (index, scratch, manifest, out)]
    out.relative_to(root / "build")
    live = frontier.local(root, root / source_relpath)
    doc, jobs, frozen = _load(root, manifest, index, live)
    base = frontier.load_json(index.read_bytes())
    frontier.verify(root, base)
    known = {r["function"] for r in base["functions"]}
    if base.get("data_functions") is None or any(not set(j["functions"]) <= known for j in jobs):
        raise ValueError("strict/data whole-owner baseline and known focus functions required")
    script = compiler._tool_path(compiler_script, root)
    tool_paths = [script, *(tools or [])]
    command = ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script)]
    context = compiler.preflight_context(root=root, scratch=scratch, command=command, tools=tool_paths)
    proof = {str(Path(p).absolute()): compiler.digest(Path(p).absolute()) for p in (objdiff, readelf)}
    frozen.update(proof)
    memory = recovery_search_memory.SearchMemory(root, root / "build/recovery-search-memory.json")
    memory_context = {"compiler": context, "baseline_index_sha256": compiler.digest(index),
                      "proof_tools": proof, "implementation": evaluator._implementation_binding(),
                      "runner_sha256": compiler.digest(Path(__file__)),
                      "memory_sha256": compiler.digest(Path(recovery_search_memory.__file__))}
    frozen.update(memory_context["implementation"])
    frozen[str(Path(__file__))] = memory_context["runner_sha256"]
    frozen[str(Path(recovery_search_memory.__file__))] = memory_context["memory_sha256"]
    directory = frontier.local(root, out.parent / (out.stem + ".search"))
    if out.exists() or directory.exists():
        raise ValueError(f"batch output already exists: {out} or {directory}")
    if not isinstance(workers, int) or isinstance(workers, bool) or not 1 <= workers <= 3:
        raise ValueError("workers must be 1..3")

    def check() -> None:
        remaining()
        for path, sha in frozen.items():
            if compiler.digest(Path(path)) != sha:
                raise ValueError(f"batch input drift: {path}")
        if compiler.preflight_context(root=root, scratch=scratch, command=command, tools=tool_paths) != context:
            raise ValueError("batch compiler/header context drift")
        frontier.verify(root, base)

    check()
    directory.mkdir(parents=True, exist_ok=False)
    records, evaluation_jobs, source_seen, object_seen, semantic_seen = [], [], {}, {}, {}
    result = {"schema": SCHEMA, "status": "complete", "causal_family": doc["causal_family"],
              "manifest_sha256": compiler.digest(manifest), "context_sha256": compiler.context_digest(context),
              "owner_function_count": len(known), "candidates": records,
              "authority_advanced": False, "retained": False, "adoption_ready": False,
              "ranked_admissible_gains": []}
    try:
        # All supplied bytes were hash-frozen by _load. Materialize only jobs
        # without validated prior measurements, before the first launch.
        for job in jobs:
            job["memory_context"] = dict(memory_context, functions=sorted(job["functions"]))
            job["previous"] = _usable_cached(root, memory.lookup_source(job["memory_context"], job["sha256"]))
            job["neutral_identity"] = (None if job["previous"] else memory.lookup_neutral_source(
                job["memory_context"], job["sha256"], base["inputs"]["candidate_object"]))
            if job["previous"] or job["neutral_identity"]:
                job.pop("data")
                continue
            job["frozen"] = directory / (job["id"] + ".c")
            compiler.atomic(job["frozen"], job.pop("data"))
            frozen[str(job["frozen"])] = job["sha256"]
        for job in jobs:
            check()
            record = {"id": job["id"], "source_sha256": job["sha256"], "evidence": job["evidence"]}
            if "source_lineage" in job:
                record["source_lineage"] = job["source_lineage"]
            if job.get("hypothesis_binding") is not None:
                record["hypothesis_memory"] = memory.lookup_hypothesis(job["memory_context"], job["hypothesis_binding"], source_sha256=job["sha256"])
            records.append(record)
            previous = job["previous"]
            if job["neutral_identity"]:
                record.update(status="known_identical_baseline_object", compile_identity=job["neutral_identity"])
                continue
            if previous:
                record.update(status="duplicate_source", reused_measurement=previous)
                continue
            source_key = (job["sha256"], tuple(sorted(job["functions"])))
            if source_key in source_seen:
                record.update(status="duplicate_source", canonical_id=source_seen[source_key])
                continue
            source_seen[source_key] = job["id"]
            output = directory / (job["id"] + ".o")
            try:
                receipt = compiler.compile_candidate(root=root, scratch=scratch, source=job["frozen"],
                    output=output, source_relpath=source_relpath, object_relpath=object_relpath,
                    command=command, tools=tool_paths, timeout=remaining())
                record["compile_receipt"] = receipt
                if receipt["source_sha256"] != job["sha256"] or compiler.digest(output) != receipt["object_sha256"]:
                    raise ValueError("compile receipt binding mismatch")
                check()
            except (OSError, ValueError, RuntimeError) as exc:
                record.update(status="compile_failed", reason=str(exc)[:2000])
                check()  # A failed compile may continue; context drift must stop.
                continue
            object_sha = receipt["object_sha256"]
            previous = _usable_cached(root, memory.lookup_object(job["memory_context"], object_sha))
            if previous:
                record.update(status="duplicate_object", reused_measurement=previous)
                continue
            object_key = (object_sha, tuple(sorted(job["functions"])))
            if object_key in object_seen:
                record.update(status="duplicate_object", canonical_id=object_seen[object_key])
                continue
            object_seen[object_key] = job["id"]
            # Ignore only non-linking metadata (e.g. STT_FILE names), never
            # allocated bytes, symbol contracts, or relocation semantics.
            inventory = evaluator.objects.inventory(output)
            semantic_sha = inventory["semantic_sha256"]
            record["semantic_object_sha256"] = semantic_sha
            semantic_key = (semantic_sha, tuple(sorted(job["functions"])))
            if semantic_key in semantic_seen:
                record.update(status="duplicate_semantic_object", canonical_id=semantic_seen[semantic_key])
                continue
            semantic_seen[semantic_key] = job["id"]
            record["status"] = "compiled"
            evaluation_jobs.append({"id": job["id"], "candidate": job["frozen"].relative_to(root).as_posix(),
                                    "candidate_object": output.relative_to(root).as_posix(), "functions": job["functions"]})
            if doc.get("working_source") is not None:
                evaluation_jobs[-1]["working_source"] = doc["working_source"]
        check()
        if evaluation_jobs:
            eval_manifest = directory / "evaluate-manifest.json"
            evaluator._atomic(eval_manifest, {"schema": evaluator.BATCH_SCHEMA, "jobs": evaluation_jobs})
            measurement = evaluator.evaluate_batch(root=root, index=index, manifest=eval_manifest,
                out=directory / "evaluation.json", objdiff=objdiff, readelf=readelf, workers=workers, timeout=remaining())
            result["measurement"] = measurement
            result["ranked_admissible_gains"] = measurement.get("measured_gains", [])
            if measurement["status"] != "complete":
                result["status"] = measurement["status"]
            for record in records:
                measured = measurement.get("results", {}).get(record["id"])
                if measured:
                    record.update(status=measured["status"], evaluation=measured["result"])
                    job = next(j for j in jobs if j["id"] == record["id"])
                    result_path = frontier.local(root, Path(measured["result"]["path"]))
                    measured_doc = frontier.load_json(result_path.read_bytes())
                    causal = measured_doc.get("causal_groups", {})
                    summaries = causal.get("channels", {}).get("strict", {})
                    record["causal_groups"] = {"diagnostic_only": True, "strict": {
                        name: {"status": row.get("status"),
                               "ranking_tuple": row.get("after", row).get("ranking_tuple"),
                               "category_rows": row.get("after", row).get("category_rows"),
                               "closed_groups": row.get("change", row).get("closed_groups_count"),
                               "new_groups": row.get("change", row).get("new_groups_count"),
                               "reclassified_groups": row.get("change", row).get("reclassified_groups_count"),
                               "resolved_target_sites": row.get("change", row).get("resolved_target_site_count"),
                               "introduced_target_sites": row.get("change", row).get("introduced_target_site_count"),
                               "residual_rows_delta": row.get("change", row).get("residual_rows_delta")}
                        for name, row in summaries.items()}, "full_detail": measured["result"]}
                    record["diagnostic_ranking"] = [
                        list(recovery_causal_groups.ranking_tuple(row["after"]))
                        for row in summaries.values() if row.get("status") == "observed"]
                    if measured_doc.get("status") not in {"failed", "duplicate_source"}:
                        try:
                            memory.record(job["memory_context"], job["sha256"],
                                record["compile_receipt"]["object_sha256"], result_path,
                                hypothesis=doc["causal_family"],
                                hypothesis_binding=job.get("hypothesis_binding"),
                                compile_receipt_path=directory / (job["id"] + ".o.receipt.json"),
                                causal_summary=record["causal_groups"])
                        except (OSError, ValueError, RuntimeError) as exc:
                            record["memory_warning"] = str(exc)[:800]
            rank_by_id = {r["id"]: r.get("diagnostic_ranking", []) for r in records}
            for gain in result["ranked_admissible_gains"]:
                gain["diagnostic_ranking"] = rank_by_id[gain["id"]]
            # Exactness and physical/protected-channel admission remain evaluator
            # decisions. Observation groups order only admitted comparable gains.
            result["ranked_admissible_gains"].sort(key=lambda r: (
                r["status"] != "exact", not bool(r["diagnostic_ranking"]),
                r["diagnostic_ranking"], -r.get("improvement_rows", 0), r["id"]))
        check()
        # Duplicate sources need their own immutable source/receipt binding so
        # subsequent batches can skip compilation, without inheriting a gain.
        by_id = {record["id"]: record for record in records}
        for record in records:
            if record["status"] not in {"duplicate_object", "duplicate_semantic_object"}:
                continue
            canonical = by_id.get(record.get("canonical_id"), {})
            descriptor = canonical.get("evaluation") or record.get("reused_measurement", {}).get("result")
            if descriptor is None or result["status"] == "drifted":
                continue
            canonical_path = frontier.local(root, Path(descriptor["path"]))
            if compiler.digest(canonical_path) != descriptor["sha256"]:
                raise ValueError(f"canonical measurement drift: {canonical_path}")
            canonical_doc = frontier.load_json(canonical_path.read_bytes())
            if canonical_doc.get("status") not in {"exact", "improved", "rejected", "no_gain",
                                                    "duplicate_object", "duplicate_semantic_object"}:
                continue
            job = next(j for j in jobs if j["id"] == record["id"])
            output = directory / (job["id"] + ".o")
            alias = {"schema": "recovery_search_alias/v1", "status": record["status"],
                     "candidate_source": evaluator._descriptor(job["frozen"]),
                     "candidate_object": evaluator._descriptor(output),
                     "baseline_index": evaluator._descriptor(index),
                     "compile_receipt": record["compile_receipt"],
                     "canonical_measurement": descriptor,
                     "semantic_object_sha256": record.get("semantic_object_sha256"),
                     "evaluated_scope": {"functions": job["functions"],
                                         "owner_function_count": len(known),
                                         "measurement": "canonical object only; equal allocated/link semantics"},
                     "retention_ready": False, "adoption_ready": False, "authority_advanced": False}
            alias_path = directory / (job["id"] + ".alias.json")
            evaluator._atomic(alias_path, alias)
            record["evaluation"] = evaluator._descriptor(alias_path)
            try:
                memory.record(job["memory_context"], job["sha256"], record["compile_receipt"]["object_sha256"],
                              alias_path, hypothesis=doc["causal_family"],
                              hypothesis_binding=job.get("hypothesis_binding"),
                              compile_receipt_path=directory / (job["id"] + ".o.receipt.json"))
            except (OSError, ValueError, RuntimeError) as exc:
                record["memory_warning"] = str(exc)[:800]
        check()
        if any(r["status"] == "compile_failed" for r in records) and result["status"] == "complete":
            result["status"] = "partial"
    except (OSError, ValueError, RuntimeError) as exc:
        result.update(status="drifted", reason=str(exc)[:2000], ranked_admissible_gains=[])
    finally:
        # Only this newly created directory's named objects are disposable.
        # Keep compact receipts/results and frozen C; retain gain objects only.
        winners = {r["id"] for r in result["ranked_admissible_gains"]}
        for job in jobs:
            if job["id"] not in winners:
                (directory / (job["id"] + ".o")).unlink(missing_ok=True)
        evaluator._atomic(out, result)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("root", "index", "scratch", "compiler-script", "manifest", "out", "objdiff", "readelf"):
        parser.add_argument("--" + name, required=True, type=Path)
    for name in ("source-relpath", "object-relpath"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--tool", dest="tools", action="append", type=Path, default=[])
    parser.add_argument("--workers", type=int, default=2)
    parser.add_argument("--timeout", type=float, default=120)
    try:
        args = parser.parse_args()
        result = run_batch(**vars(args))
        print(json.dumps({"status": result["status"], "out": str(args.out),
                          "candidates": [{"id": r["id"], "status": r["status"]} for r in result["candidates"]],
                          "ranked_admissible_gains": [r["id"] for r in result["ranked_admissible_gains"]],
                          "authority_advanced": False}, sort_keys=True))
        return 0 if result["status"] == "complete" else 2
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        print(f"search batch: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
