"""Bounded private measurement memory, never a retention or promotion authority.

Callers bind compiler/header/generated inputs, proof tools and implementations in
context; baseline_index_sha256 binds the complete verified frontier. Hashes, not
names, hypotheses, function lifetimes or match percentages define duplicates.
"""
from __future__ import annotations

from contextlib import contextmanager
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import time

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import compile_recovery_candidate as compiler

SCHEMA = "recovery_search_memory/v1"
MAX_ENTRIES = 128
MAX_BYTES = 512 * 1024
SHA = re.compile(r"[0-9a-f]{64}\Z")
# recovery_evaluate._classify emits the first four outcomes; its allocated
# object comparison emits duplicate_object. recovery_search_batch publishes
# duplicate_semantic_object aliases with their own source/object/receipt and
# hash-bound canonical measurement, without inheriting retention authority.
# Neutral is the legacy v1 measured
# no-gain spelling. Unknown/new terminal states must fail closed, not turn a
# compiler or infrastructure failure into a measured source dead end.
MEASURED_STATUSES = frozenset({"exact", "improved", "rejected", "no_gain",
                               "duplicate_object", "duplicate_semantic_object", "neutral"})


def compact_causal_summary(summary: dict, result: dict) -> dict:
    """Bound diagnostic observations; the containing entry supplies proof bindings.

    Never follow full_detail paths or preserve report/member arrays. Old callers
    and old on-disk entries remain supported.
    """
    out = {"schema": "recovery_search_causal_summary/v1", "diagnostic_only": True,
           "strict": {}, "truncated": False}
    for key in ("closed_mismatches", "new_mismatches", "disposition"):
        value = summary.get(key)
        if value is None or type(value) in (int, bool):
            if key in summary:
                out[key] = value
        elif isinstance(value, str):
            out[key] = value[:160]
    compact = summary.get("strict", {})
    detail = result.get("causal_groups", {}).get("channels", {}).get("strict", {})
    if not isinstance(compact, dict):
        compact = {}
    if not isinstance(detail, dict):
        detail = {}
    scalar_keys = ("closed_groups", "new_groups", "reclassified_groups",
                   "resolved_target_sites", "introduced_target_sites", "residual_rows_delta",
                   "unresolved_row_count", "structural_hazard_count", "instruction_exact")
    for name in sorted(set(compact) | set(detail)):
        row = compact.get(name, {})
        full = detail.get(name, {})
        if not isinstance(row, dict) or not isinstance(full, dict):
            continue
        after, change = full.get("after", full), full.get("change", {})
        if not isinstance(after, dict) or not isinstance(change, dict):
            continue
        item = {"status": str(row.get("status", full.get("status", "unknown")))[:80]}
        for key in scalar_keys:
            value = row.get(key, after.get(key))
            if type(value) in (int, bool) or value is None:
                if key in row or key in after:
                    item[key] = value
        categories = row.get("category_rows", after.get("category_rows", {}))
        if isinstance(categories, dict):
            item["category_rows"] = {str(k)[:80]: v for k, v in sorted(categories.items())[:24]
                                     if type(v) is int}
        for key in ("closed_groups", "new_groups", "reclassified_groups"):
            values = change.get(key)
            if isinstance(values, list):
                ids = sorted(v for v in values if isinstance(v, str))
                item[key + "_count"] = len(ids)
                item[key + "_ids"] = [v[:160] for v in ids[:8]]
                if len(ids) > 8 or any(len(v) > 160 for v in ids):
                    out["truncated"] = True
        groups = after.get("groups", [])
        if isinstance(groups, list):
            unresolved = sorted(g["id"] for g in groups if isinstance(g, dict)
                                and isinstance(g.get("id"), str)
                                and "unknown" in str(g.get("category", g.get("kind", ""))))
            if unresolved:
                item["unresolved_groups_ids"] = [v[:160] for v in unresolved[:8]]
                item["unresolved_groups_count"] = len(unresolved)
                if len(unresolved) > 8 or any(len(v) > 160 for v in unresolved):
                    out["truncated"] = True
        first = after.get("first_machine_divergence", row.get("first_machine_divergence"))
        if isinstance(first, dict):
            site = {k: first[k] for k in ("row", "kind")
                    if type(first.get(k)) is int or isinstance(first.get(k), str)}
            for side in ("target", "candidate"):
                instruction = first.get(side, {})
                if isinstance(instruction, dict):
                    instruction = instruction.get("instruction", instruction)
                    if isinstance(instruction, dict):
                        site[side] = {k: str(instruction[k])[:160] for k in ("address", "formatted")
                                      if k in instruction}
            item["first_machine_divergence"] = site
        if len(out["strict"]) >= 8 or len(_canonical(dict(out, strict={**out["strict"], name: item}))) > 4096:
            out["truncated"] = True
            break
        out["strict"][name] = item
    return out


def _canonical(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode()


def _sha(value: str) -> str:
    if not isinstance(value, str) or not SHA.fullmatch(value):
        raise ValueError("search memory requires a SHA-256")
    return value


def normalize_hypothesis_binding(binding: dict) -> dict:
    """Validate a bounded caller-authored causal description, not equivalence proof.

    source_sha256 binds the reconstruction on which the hypothesis was formed;
    candidate identity remains separately bound by the measured entry.
    """
    if not isinstance(binding, dict):
        raise ValueError("hypothesis binding must be an object")
    allowed = {"family", "scope", "source_sha256", "participants", "boundary",
               "causal_evidence_ids", "producer", "dependencies"}
    if set(binding) - allowed:
        raise ValueError("unknown hypothesis binding fields")
    out = {"source_sha256": _sha(binding.get("source_sha256"))}
    for key in ("family", "scope", "boundary", "producer"):
        value = binding.get(key)
        if value is None and key == "producer":
            continue
        if not isinstance(value, str) or not value.strip() or len(value) > 256:
            raise ValueError(f"invalid hypothesis {key}")
        out[key] = value
    for key in ("participants", "causal_evidence_ids", "dependencies"):
        values = binding.get(key, [] if key == "dependencies" else None)
        if (not isinstance(values, list) or len(values) > 16
                or (key == "participants" and not values)
                or any(not isinstance(v, str) or not v.strip() or len(v) > 256 for v in values)):
            raise ValueError(f"invalid hypothesis {key}")
        out[key] = sorted(set(values))
    if len(_canonical(out)) > 2048:
        raise ValueError("oversized hypothesis binding")
    return out


class SearchMemory:
    def __init__(self, root: Path, path: Path):
        self.root = Path(root).resolve()
        self.path = self._local(path)
        if not self.path.is_relative_to(self.root / "build"):
            raise ValueError(f"search memory must be under build/: {self.path}")

    def _local(self, path: Path | str) -> Path:
        path = Path(path)
        resolved = (path if path.is_absolute() else self.root / path).resolve()
        if not resolved.is_relative_to(self.root):
            raise ValueError(f"search memory path escapes root: {path}")
        return resolved

    @contextmanager
    def _lock(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        lock = self.path.with_suffix(self.path.suffix + ".lock")
        # OS locks are released on process death; the lock file is not a tombstone.
        with lock.open("a+b") as stream:
            stream.seek(0)
            if not stream.read(1):
                stream.write(b"0")
                stream.flush()
            deadline = time.monotonic() + 10
            while True:
                try:
                    if os.name == "nt":
                        import msvcrt
                        stream.seek(0)
                        msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
                    else:
                        import fcntl
                        fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
                    break
                except OSError:
                    if time.monotonic() >= deadline:
                        raise TimeoutError(f"search memory lock busy: {lock}")
                    time.sleep(.02)
            try:
                yield
            finally:
                if os.name == "nt":
                    stream.seek(0)
                    msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
                else:
                    fcntl.flock(stream, fcntl.LOCK_UN)

    def _read(self) -> dict:
        if not self.path.exists():
            return {"schema": SCHEMA, "entries": [], "latest_frontier": None}
        if self.path.stat().st_size > MAX_BYTES:
            raise ValueError(f"oversized search memory: {self.path}")
        state = json.loads(self.path.read_text(encoding="utf-8"))
        if state.get("schema") != SCHEMA or not isinstance(state.get("entries"), list) or len(state["entries"]) > MAX_ENTRIES:
            raise ValueError(f"invalid search memory: {self.path}")
        return state

    def _write(self, state: dict) -> None:
        payload = _canonical(state) + b"\n"
        if len(payload) > MAX_BYTES:
            raise ValueError(f"oversized search memory: {self.path}")
        compiler.atomic(self.path, payload)

    def _context(self, context: dict) -> str:
        _sha(context["baseline_index_sha256"])
        for field in ("compiler", "proof_tools", "implementation"):
            if not context.get(field):
                raise ValueError(f"search memory context missing {field}")
        return hashlib.sha256(_canonical(context)).hexdigest()

    def _descriptor(self, path: Path | str) -> dict:
        path = self._local(path)
        return {"path": str(path), "sha256": compiler.digest(path), "size_bytes": path.stat().st_size}

    def _matches_descriptor(self, descriptor: dict) -> bool:
        expected = dict(descriptor, path=str(self._local(descriptor["path"])))
        return self._descriptor(descriptor["path"]) == expected

    def _valid(self, entry: dict, source_lookup: bool) -> bool:
        try:
            desc = entry["result"]
            if self._descriptor(desc["path"]) != desc:
                return False
            result = json.loads(self._local(desc["path"]).read_text(encoding="utf-8"))
            if result["baseline_index"]["sha256"] != entry["baseline_index_sha256"]:
                return False
            if result["candidate_source"]["sha256"] != entry["source_sha256"] or result["candidate_object"]["sha256"] != entry["object_sha256"]:
                return False
            if not self._matches_descriptor(result["candidate_source"]):
                return False
            obj = result["candidate_object"]
            if self._local(obj["path"]).exists() and not self._matches_descriptor(obj):
                return False
            receipt = result.get("compile_receipt")
            if entry.get("compile_receipt"):
                receipt_desc = entry["compile_receipt"]
                if self._descriptor(receipt_desc["path"]) != receipt_desc:
                    return False
                receipt = json.loads(self._local(receipt_desc["path"]).read_text(encoding="utf-8"))
            if receipt is not None:
                if receipt.get("schema") != "recovery_candidate_compile/v1" or receipt.get("source_sha256") != entry["source_sha256"] or receipt.get("object_sha256") != entry["object_sha256"] or receipt.get("context_sha256") != entry["compiler_context_sha256"]:
                    return False
            return (not source_lookup or receipt is not None) and result.get("status") in MEASURED_STATUSES
        except (OSError, ValueError, KeyError, TypeError):
            return False

    def _lookup(self, context: dict, digest: str, field: str) -> dict | None:
        key = self._context(context)
        _sha(digest)
        with self._lock():
            state = self._read()
            latest = state.get("latest_frontier")
            if latest and latest["sha256"] != context["baseline_index_sha256"]:
                return None
            for entry in reversed(state["entries"]):
                if entry["context_sha256"] == key and entry[field] == digest and self._valid(entry, field == "source_sha256"):
                    return entry
        return None

    def lookup_source(self, context: dict, source_sha256: str) -> dict | None:
        return self._lookup(context, source_sha256, "source_sha256")

    def lookup_neutral_source(self, context: dict, source_sha256: str,
                              baseline_object: dict) -> dict | None:
        """Reuse compile identity only; never reuse prior proof/admission status."""
        self._context(context)
        _sha(source_sha256)
        compiler_key = compiler.context_digest(context["compiler"])
        try:
            if not self._matches_descriptor(baseline_object):
                return None
        except (OSError, ValueError, KeyError, TypeError):
            return None
        with self._lock():
            state = self._read()
            latest = state.get("latest_frontier")
            if latest and latest["sha256"] != context["baseline_index_sha256"]:
                return None
            for entry in reversed(state["entries"]):
                if (entry.get("source_sha256") == source_sha256
                        and entry.get("compiler_context_sha256") == compiler_key
                        and entry.get("object_sha256") == baseline_object["sha256"]
                        and self._valid(entry, True)):
                    return {"status": "known_identical_baseline_object",
                            "source_sha256": source_sha256,
                            "object_sha256": entry["object_sha256"],
                            "compiler_context_sha256": compiler_key,
                            "receipt_evidence": entry.get("compile_receipt", entry["result"]),
                            "baseline_object": baseline_object,
                            "authority_advanced": False}
        return None

    def lookup_object(self, context: dict, object_sha256: str) -> dict | None:
        return self._lookup(context, object_sha256, "object_sha256")

    def lookup_hypothesis(self, context: dict, binding: dict, *,
                          source_sha256: str | None = None,
                          object_sha256: str | None = None) -> dict:
        """Return at most eight diagnostics; a family never exhausts a function.

        Renamed files have no semantic significance. Changed source boundaries,
        producers or coupled dependencies remain distinct hypotheses. Even equal
        bindings are only warnings unless exact measured hashes validate anew.
        """
        key = self._context(context)
        bound = normalize_hypothesis_binding(binding)
        for digest in (source_sha256, object_sha256):
            if digest is not None:
                _sha(digest)
        out = {"decision": "unseen_hypothesis", "suppress_compile": False,
               "diagnostic_only": True, "authority_advanced": False, "matches": []}
        with self._lock():
            state = self._read()
            latest = state.get("latest_frontier")
            if latest and latest["sha256"] != context["baseline_index_sha256"]:
                return out
            for entry in reversed(state["entries"]):
                if entry.get("context_sha256") != key or not self._valid(entry, False):
                    continue
                prior = entry.get("hypothesis_binding")
                if prior is None:
                    continue
                try:
                    prior = normalize_hypothesis_binding(prior)
                except ValueError:
                    continue
                if prior != bound:
                    continue
                exact_source = source_sha256 == entry["source_sha256"] and self._valid(entry, True)
                exact_object = object_sha256 == entry["object_sha256"]
                # When both are supplied, a contradictory object invalidates the
                # source duplicate request rather than concealing that conflict.
                if object_sha256 is not None and not exact_object:
                    exact_source = False
                exact = exact_source or exact_object
                if exact:
                    out["decision"] = "exact_measured_duplicate"
                elif out["decision"] == "unseen_hypothesis":
                    out["decision"] = "related_hypothesis_warning"
                out["suppress_compile"] |= exact_source
                if len(out["matches"]) < 8:
                    out["matches"].append({"source_sha256": entry["source_sha256"],
                        "object_sha256": entry["object_sha256"], "result": entry["result"],
                        "disposition": entry.get("disposition"),
                        "causal_summary": entry.get("causal_summary", {}),
                        "exact_source": exact_source, "exact_object": exact_object})
        return out

    def record(self, context: dict, source_sha256: str, object_sha256: str,
               result_path: Path, hypothesis: str = "", compile_receipt_path: Path | None = None,
               causal_summary: dict | None = None, hypothesis_binding: dict | None = None) -> dict:
        entry = {"context_sha256": self._context(context),
                 "compiler_context_sha256": compiler.context_digest(context["compiler"]),
                 "baseline_index_sha256": context["baseline_index_sha256"],
                 "source_sha256": _sha(source_sha256), "object_sha256": _sha(object_sha256),
                 "result": self._descriptor(result_path), "hypothesis": str(hypothesis)[:1024]}
        if compile_receipt_path is not None:
            entry["compile_receipt"] = self._descriptor(compile_receipt_path)
        if hypothesis_binding is not None:
            entry["hypothesis_binding"] = normalize_hypothesis_binding(hypothesis_binding)
        result = json.loads(self._local(result_path).read_text(encoding="utf-8"))
        entry["disposition"] = result.get("status")
        # Store only explicit bounded causal summaries, not full objdiff reports.
        summary = causal_summary or {}
        entry["causal_summary"] = compact_causal_summary(summary, result)
        if len(_canonical(entry)) > 8192 or not self._valid(entry, False):
            raise ValueError(f"invalid or oversized measurement for search memory: {result_path}")
        with self._lock():
            state = self._read()
            if state.get("latest_frontier") and state["latest_frontier"]["sha256"] != context["baseline_index_sha256"]:
                raise ValueError("measurement baseline differs from latest frontier")
            state["entries"] = [old for old in state["entries"] if (old["context_sha256"], old["source_sha256"]) != (entry["context_sha256"], source_sha256)]
            state["entries"].append(entry)
            state["entries"] = state["entries"][-MAX_ENTRIES:]
            while len(_canonical(state)) + 1 > MAX_BYTES:
                state["entries"].pop(0)
            self._write(state)
        return entry

    def advance_frontier(self, index_path: Path, expected_parent_index_sha256: str) -> dict:
        """Track an externally verified retained index; never retain source here."""
        _sha(expected_parent_index_sha256)
        descriptor = self._descriptor(index_path)
        with self._lock():
            state = self._read()
            parent = state.get("latest_frontier")
            if parent and parent["sha256"] != expected_parent_index_sha256:
                raise ValueError("latest frontier does not match expected parent index")
            if not parent and any(e["baseline_index_sha256"] != expected_parent_index_sha256 for e in state["entries"]):
                raise ValueError("search memory entries do not match expected parent index")
            if self._descriptor(index_path) != descriptor:
                raise ValueError(f"frontier changed during advancement: {index_path}")
            state["latest_frontier"] = descriptor
            if descriptor["sha256"] != expected_parent_index_sha256:
                state["entries"] = []
            self._write(state)
        return descriptor
