from __future__ import annotations

import concurrent.futures
import contextlib
import gzip
import hashlib
import io
import json
import os
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock

from tools import match_workbench as module


def _sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _descriptor(path: Path) -> dict[str, object]:
    return {"path": str(path), "size_bytes": path.stat().st_size, "sha256": _sha256(path)}


def _write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, separators=(",", ":"), ensure_ascii=False) + "\n", encoding="utf-8")


def _rehash(value: dict[str, object], field: str) -> dict[str, object]:
    body = dict(value)
    body.pop(field, None)
    canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
    body[field] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
    return body


def _report(function: str, *, exact: bool = False, large: bool = False) -> dict[str, object]:
    rows: list[dict[str, object]] = []
    if not exact:
        rows.append({"diff_kind": "REG_SWAP", "instruction": {"formatted": "mr r3,r4"}})
    if large:
        rows.extend({"diff_kind": "NOP", "instruction": {"formatted": "nop"}} for _ in range(400))
    return {
        "left": {
            "symbols": [
                {
                    "name": function,
                    "kind": "SYMBOL_FUNCTION",
                    "size": "4",
                    "target_symbol": 0,
                    "match_percent": 100.0 if exact else 75.0,
                    "instructions": rows,
                }
            ]
        },
        "right": {"symbols": [{"name": function, "kind": "SYMBOL_FUNCTION", "size": "4"}]},
    }


def _assessment_report(
    *,
    focus_match: float = 75.0,
    focus_size: str = "4",
    focus_candidate_size: str = "4",
    sibling_match: float = 100.0,
    sibling_size: str = "8",
    sibling_candidate_size: str = "8",
    sibling_diff_kind: str | None = None,
) -> dict[str, object]:
    focus_rows = [] if focus_match == 100.0 else [
        {"diff_kind": "REG_SWAP", "instruction": {"formatted": "mr r3,r4"}}
    ]
    sibling_rows = [] if sibling_match == 100.0 and sibling_diff_kind is None else [
        {
            "diff_kind": sibling_diff_kind or "REG_SWAP",
            "instruction": {"formatted": "mr r5,r6"},
        }
    ]
    return {
        "left": {
            "symbols": [
                {
                    "name": "focus",
                    "kind": "SYMBOL_FUNCTION",
                    "size": focus_size,
                    "target_symbol": 0,
                    "match_percent": focus_match,
                    "instructions": focus_rows,
                },
                {
                    "name": "sibling",
                    "kind": "SYMBOL_FUNCTION",
                    "size": sibling_size,
                    "target_symbol": 1,
                    "match_percent": sibling_match,
                    "instructions": sibling_rows,
                },
            ]
        },
        "right": {
            "symbols": [
                {"name": "focus", "kind": "SYMBOL_FUNCTION", "size": focus_candidate_size},
                {"name": "sibling", "kind": "SYMBOL_FUNCTION", "size": sibling_candidate_size},
            ]
        },
    }


class MatchWorkbenchTests(unittest.TestCase):
    maxDiff = None

    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.target = self.root / "target.o"
        self.source = self.root / "candidate.c"
        self.object = self.root / "candidate.o"
        self.strict = self.root / "strict.json"
        self.data = self.root / "data.json"
        self.target.write_bytes(b"target-bytes")
        self.source.write_text("int fn(void) { return 1; }\n", encoding="utf-8")
        self.object.write_bytes(b"object-bytes")
        _write_json(self.strict, _report("fn"))
        _write_json(self.data, _report("fn", exact=True))
        self.manifest = self.root / "request.json"
        _write_json(self.manifest, self._manifest())
        self.workspace = self.root / "build" / "match"

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _manifest(self, *, session_id: str = "session-1", **request_overrides: object) -> dict[str, object]:
        value: dict[str, object] = {
            "schema": module.REQUEST_SCHEMA,
            "schema_version": 1,
            "session_id": session_id,
            "owner": "REL:demo:fn",
            "unit": "demo.c",
            "function": "fn",
            "target": _descriptor(self.target),
            "context": {
                "base_commit": "abcdef1234567890",
                "toolchain_key": "GC/1.3.2",
                "compiler": None,
                "compile_argv": [],
                "compile_inputs": [],
            },
            "policy": {
                "max_workers": 4,
                "max_report_bytes": 2_000_000,
                "max_compact_bytes": 16_000,
                "allowed_job_kinds": ["artifact-fact", "cfg", "safe-probe"],
            },
        }
        value.update(request_overrides)
        return value

    def _init(self, *, workspace: Path | None = None) -> dict[str, object]:
        return module.init_workspace(self.root, self.manifest, workspace or self.workspace)

    def _record(
        self,
        candidate_id: str = "c1",
        *,
        source: Path | None = None,
        object_path: Path | None = None,
        strict_report: Path | None = None,
        data_report: Path | None = None,
        hypothesis: str = "natural candidate",
        axis: str = "register-lifetime",
        status: str = "measured",
        reason: str = "candidate measured",
        focus_symbol: str | None = None,
    ) -> dict[str, object]:
        return module.record_candidate(
            self.root,
            self.workspace,
            candidate_id=candidate_id,
            source=source or self.source,
            object_path=object_path or self.object,
            strict_report=strict_report or self.strict,
            data_report=data_report,
            hypothesis=hypothesis,
            axis=axis,
            status=status,
            reason=reason,
            focus_symbol=focus_symbol,
        )

    def _job_script(self, name: str = "probe.py", body: str | None = None) -> Path:
        path = self.root / name
        path.write_text(
            textwrap.dedent(
                body
                or """
                import json, os, pathlib, sys, time
                output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
                output.mkdir(parents=True, exist_ok=True)
                time.sleep(float(sys.argv[1]) if len(sys.argv) > 1 else 0.05)
                (output / "result.json").write_text(json.dumps({"readonly": os.environ.get("MATCH_WORKBENCH_READ_ONLY")}), encoding="utf-8")
                print("probe-ok", flush=True)
                """
            ),
            encoding="utf-8",
        )
        return path

    def _jobs(
        self,
        script: Path,
        *,
        resource_class: str = "read_only_subprocess",
        job_ids: tuple[str, ...] = ("j1",),
        distinct: bool = False,
        max_output_bytes: int | None = None,
        timeout_seconds: int = 10,
        env: dict[str, str] | None = None,
    ) -> Path:
        jobs: list[dict[str, object]] = []
        for job_id in job_ids:
            job: dict[str, object] = {
                "job_id": job_id,
                "kind": "safe-probe",
                "resource_class": resource_class,
                "executable": _descriptor(Path(sys.executable)),
                # Include a harmless per-job argument when requested so the
                # fingerprints differ and the bounded executor has real
                # independent work to schedule.  With ``distinct=False``
                # aliases intentionally collapse to one fingerprint.
                "argv": [str(script), "0.20", job_id] if distinct else [str(script)],
                "cwd": str(self.root),
                "inputs": [_descriptor(script)],
                "outputs": ["result.json"],
                "timeout_seconds": timeout_seconds,
            }
            if max_output_bytes is not None:
                job["max_output_bytes"] = max_output_bytes
            if env is not None:
                job["env"] = dict(env)
            jobs.append(job)
        path = self.root / f"jobs-{resource_class}.json"
        _write_json(path, {"schema": module.JOBS_SCHEMA, "schema_version": 1, "jobs": jobs})
        return path

    def test_init_self_hash_idempotency_and_immutable_conflict(self) -> None:
        first = self._init()
        self.assertEqual(first["status"], "initialized")
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        self.assertEqual(session["session_sha256"], first["session"]["session_sha256"])
        body = dict(session)
        body["authority_advanced"] = True
        session_path.write_text(json.dumps(body), encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "self-hash mismatch"):
            module.lookup_matches(self.root, self.workspace, self.source)

        # Restore a clean workspace and prove a repeat is a no-op.
        session_path.write_text(json.dumps(first["session"], separators=(",", ":")), encoding="utf-8")
        second = self._init()
        self.assertEqual(second["status"], "unchanged")
        conflict = self.root / "conflict.json"
        _write_json(conflict, self._manifest(session_id="session-2"))
        with self.assertRaisesRegex(module.MatchError, "different immutable session"):
            module.init_workspace(self.root, conflict, self.workspace)

    def test_target_and_session_descriptor_mutation_fail_closed(self) -> None:
        self._init()
        original_target = self.target.read_bytes()
        self.target.write_bytes(b"mutated-target")
        with self.assertRaisesRegex(module.MatchError, "session target changed"):
            module.lookup_matches(self.root, self.workspace, self.source)
        self.target.write_bytes(original_target)

        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session["request"]["target"]["sha256"] = "0" * 64
        body = dict(session)
        body.pop("session_sha256", None)
        canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        session["session_sha256"] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
        session_path.write_text(json.dumps(session, separators=(",", ":")), encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "session target changed"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_target_cas_mutation_fails_closed(self) -> None:
        initialized = self._init()
        target_cas = self.workspace / initialized["session"]["target_blob"]["cas_path"]
        target_cas.write_bytes(b"tampered-target-cas")
        with self.assertRaisesRegex(module.MatchError, "session target CAS"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_self_hashed_session_cannot_rebind_target_away_from_manifest(self) -> None:
        self._init()
        replacement = self.root / "replacement-target.o"
        replacement.write_bytes(b"replacement-target")
        replacement_descriptor = _descriptor(replacement)
        replacement_relative = (
            f"cas/blobs/target/{replacement_descriptor['sha256'][:2]}/"
            f"{replacement_descriptor['sha256']}.bin"
        )
        replacement_cas = self.workspace / replacement_relative
        replacement_cas.parent.mkdir(parents=True, exist_ok=True)
        replacement_cas.write_bytes(replacement.read_bytes())
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session["request"]["target"] = replacement_descriptor
        session["target_blob"] = {
            "kind": "target",
            "sha256": replacement_descriptor["sha256"],
            "size_bytes": replacement_descriptor["size_bytes"],
            "cas_path": replacement_relative,
            "dedup_hit": False,
        }
        _write_json(session_path, _rehash(session, "session_sha256"))
        with self.assertRaisesRegex(module.MatchError, "request manifest"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_manifest_duplicate_key_unknown_field_and_descriptor_mismatch_rejected(self) -> None:
        duplicate = self.root / "duplicate.json"
        duplicate.write_text(
            '{"schema":"match_workbench_request/v1","schema":"match_workbench_request/v1"}\n',
            encoding="utf-8",
        )
        with self.assertRaisesRegex(module.MatchError, "duplicate JSON key"):
            module.init_workspace(self.root, duplicate, self.workspace)

        unknown = self._manifest()
        unknown["unexpected"] = True
        unknown_path = self.root / "unknown.json"
        _write_json(unknown_path, unknown)
        with self.assertRaisesRegex(module.MatchError, "unknown field"):
            module.init_workspace(self.root, unknown_path, self.workspace)

        mismatch = self._manifest()
        mismatch["target"] = {**_descriptor(self.target), "size_bytes": self.target.stat().st_size + 1}
        mismatch_path = self.root / "mismatch.json"
        _write_json(mismatch_path, mismatch)
        with self.assertRaisesRegex(module.MatchError, "descriptor mismatch"):
            module.init_workspace(self.root, mismatch_path, self.workspace)

        with self.assertRaisesRegex(module.MatchError, "workspace must stay beneath"):
            module.init_workspace(self.root, self.manifest, self.root.parent / "outside-workbench")

    def test_descriptor_rejects_hardlink(self) -> None:
        hardlink = self.root / "target-hardlink.o"
        os.link(self.target, hardlink)
        manifest = self._manifest()
        manifest["target"] = _descriptor(hardlink)
        path = self.root / "hardlink.json"
        _write_json(path, manifest)
        with self.assertRaisesRegex(module.MatchError, "hard link"):
            module.init_workspace(self.root, path, self.workspace)

    def test_workbench_lock_rejects_hardlink_alias(self) -> None:
        self.workspace.mkdir(parents=True)
        victim = self.root / "lock-victim"
        victim.write_bytes(b"")
        os.link(victim, self.workspace / ".workbench.lock")
        with self.assertRaisesRegex(module.MatchError, "lock must have exactly one hard link"):
            module.init_workspace(self.root, self.manifest, self.workspace)

    def test_descriptor_rejects_symlink_when_supported(self) -> None:
        symlink = self.root / "target-symlink.o"
        try:
            symlink.symlink_to(self.target)
        except (OSError, NotImplementedError) as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        symlink_manifest = self._manifest()
        symlink_manifest["target"] = {"path": str(symlink), **{k: _descriptor(self.target)[k] for k in ("size_bytes", "sha256")}}
        symlink_path = self.root / "symlink.json"
        _write_json(symlink_path, symlink_manifest)
        with self.assertRaisesRegex(module.MatchError, "indirection"):
            module.init_workspace(self.root, symlink_path, self.root / "build" / "symlink")

    def test_record_cas_reports_deterministic_gzip_and_idempotency(self) -> None:
        self._init()
        first = self._record(data_report=self.data)
        self.assertEqual(first["status"], "recorded")
        record = first["record"]
        body = dict(record)
        claimed = body.pop("record_sha256")
        canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        self.assertEqual(claimed, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        for kind in ("source_blob", "object_blob"):
            blob = self.workspace / record[kind]["cas_path"]
            self.assertTrue(blob.is_file())
            self.assertEqual(_sha256(blob), record[kind]["sha256"])
        strict_info = record["reports"]["strict"]
        data_info = record["reports"]["data"]
        self.assertEqual(strict_info["codec"], "gzip")
        self.assertEqual(strict_info["raw_sha256"], _sha256(self.strict))
        self.assertEqual(data_info["raw_sha256"], _sha256(self.data))
        for info in (strict_info, data_info):
            cached = self.workspace / info["cas_path"]
            with gzip.open(cached, "rb") as stream:
                self.assertEqual(stream.read(), (self.strict if info is strict_info else self.data).read_bytes())
            self.assertEqual(info["compressed_size_bytes"], cached.stat().st_size)

        repeated = self._record(data_report=self.data)
        self.assertEqual(repeated["status"], "unchanged")
        self.assertEqual(repeated["record"], record)

        source2 = self.root / "candidate-copy.c"
        source2.write_bytes(self.source.read_bytes())
        object2 = self.root / "candidate-copy.o"
        object2.write_bytes(self.object.read_bytes())
        strict2 = self.root / "strict-copy.json"
        strict2.write_bytes(self.strict.read_bytes())
        duplicate = self._record("c2", source=source2, object_path=object2, strict_report=strict2, data_report=self.data)
        self.assertEqual(duplicate["status"], "duplicate")
        self.assertEqual(duplicate["record"]["duplicate_of"], "c1")
        self.assertNotEqual(
            duplicate["record"]["source_context_key"], record["source_context_key"]
        )
        self.assertTrue(duplicate["record"]["reports"]["strict"]["dedup_hit"])
        self.assertEqual(duplicate["record"]["reports"]["strict"]["cas_path"], strict_info["cas_path"])

        jobs = self._jobs(self._job_script(), job_ids=("reuse",))
        module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        reused = module.diagnose_candidate(self.root, self.workspace, "c2", jobs)
        self.assertEqual(reused["summary"], {"ran": 1, "cached": 0, "failed": 0})
        self.assertEqual(reused["jobs"][0]["cache_status"], "ran")

    def test_missing_or_corrupt_report_cas_fails_matrix(self) -> None:
        self._init()
        recorded = self._record(data_report=self.data)
        strict_cas = self.workspace / recorded["record"]["reports"]["strict"]["cas_path"]
        strict_cas.unlink()
        with self.assertRaisesRegex(module.MatchError, "report"):
            module.build_matrix(self.root, self.workspace)

        second_workspace = self.root / "build" / "match-corrupt"
        module.init_workspace(self.root, self.manifest, second_workspace)
        second = module.record_candidate(
            self.root,
            second_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        data_cas = second_workspace / second["record"]["reports"]["data"]["cas_path"]
        data_cas.write_bytes(b"not-a-gzip-report")
        with self.assertRaisesRegex(module.MatchError, "report"):
            module.build_matrix(self.root, second_workspace)

    def test_lookup_source_and_object_indexes_skip_work(self) -> None:
        self._init()
        self.assertEqual(module.lookup_matches(self.root, self.workspace, self.source)["status"], "new")
        first = self._record()
        normalized_alias = module.lookup_matches(
            self.root, self.workspace, self.source.relative_to(self.root)
        )
        self.assertEqual(normalized_alias["status"], "known_source")
        self.assertEqual(
            normalized_alias["source_context_key"], first["record"]["source_context_key"]
        )
        source_copy = self.root / "source-copy.c"
        source_copy.write_bytes(self.source.read_bytes())
        object_other = self.root / "other.o"
        object_other.write_bytes(b"different-object")
        source_hit = module.lookup_matches(self.root, self.workspace, source_copy, object_other)
        self.assertEqual(source_hit["status"], "new")
        self.assertFalse(source_hit["skip_compile"])
        self.assertFalse(source_hit["skip_diagnostics"])
        self.assertNotEqual(source_hit["source_context_key"], first["record"]["source_context_key"])

        # The same bytes are a distinct compile context when the compiler sees
        # a different source path/basename, so a different object is permitted.
        second = self._record("c2", source=source_copy, object_path=object_other)
        self.assertEqual(second["status"], "recorded")
        self.assertNotEqual(
            second["record"]["source_context_key"], first["record"]["source_context_key"]
        )

        object_third = self.root / "third.o"
        object_third.write_bytes(b"third-object")
        original_path_conflict = module.lookup_matches(
            self.root, self.workspace, self.source, object_third
        )
        self.assertEqual(original_path_conflict["status"], "conflict")
        self.assertIn("same frozen source/context", original_path_conflict["reason"])

        other_source = self.root / "other.c"
        other_source.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object_hit = module.lookup_matches(self.root, self.workspace, other_source, self.object)
        self.assertEqual(object_hit["status"], "known_object")
        self.assertFalse(object_hit["skip_compile"])
        self.assertFalse(object_hit["skip_diagnostics"])
        self.assertEqual(object_hit["diagnostic_reuse_candidate_id"], "c1")

    def test_legacy_source_context_record_remains_readable_only_at_its_recorded_path(self) -> None:
        self._init()
        recorded = self._record()
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate.pop("compile_input_identity")
        session = module._load_session(self.workspace, self.root)
        legacy_key = module._legacy_context_key(session, candidate["source"]["sha256"])
        candidate["source_context_key"] = legacy_key
        candidate = _rehash(candidate, "record_sha256")
        _write_json(candidate_path, candidate)

        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["source_context_index"] = {legacy_key: "c1"}
        index["last_record_sha256"] = candidate["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))

        same_path = module.lookup_matches(self.root, self.workspace, self.source)
        self.assertEqual(same_path["status"], "known_source")
        self.assertEqual(same_path["source_context_key"], legacy_key)
        self.assertEqual(module.build_matrix(self.root, self.workspace)["aggregate"]["candidate_count"], 1)

        copied = self.root / "legacy-copy.c"
        copied.write_bytes(self.source.read_bytes())
        copied_lookup = module.lookup_matches(self.root, self.workspace, copied)
        self.assertEqual(copied_lookup["status"], "new")
        self.assertNotEqual(copied_lookup["source_context_key"], legacy_key)

    def test_lookup_rejects_same_byte_source_replacement_during_identity_check(self) -> None:
        self._init()
        self._record()
        original_loader = module._load_candidate
        replaced = False

        def replace_source(*args: object, **kwargs: object) -> object:
            nonlocal replaced
            if not replaced:
                replacement = self.root / "lookup-replacement.c"
                replacement.write_bytes(self.source.read_bytes())
                os.replace(replacement, self.source)
                replaced = True
            return original_loader(*args, **kwargs)

        with mock.patch.object(module, "_load_candidate", side_effect=replace_source):
            with self.assertRaisesRegex(module.MatchError, "identity changed"):
                module.lookup_matches(self.root, self.workspace, self.source)

    def test_record_rejects_same_byte_source_replacement_before_cas_copy(self) -> None:
        self._init()
        original_load_index = module._load_index
        replaced = False

        def replace_source(*args: object, **kwargs: object) -> object:
            nonlocal replaced
            if not replaced:
                replacement = self.root / "record-replacement.c"
                replacement.write_bytes(self.source.read_bytes())
                os.replace(replacement, self.source)
                replaced = True
            return original_load_index(*args, **kwargs)

        with mock.patch.object(module, "_load_index", side_effect=replace_source):
            with self.assertRaisesRegex(module.MatchError, "identity changed"):
                self._record()
        self.assertFalse((self.workspace / "candidates" / "c1.json").exists())
        self.assertFalse(any(path.name.endswith(".tmp") for path in self.workspace.rglob("*")))

    def test_lookup_fails_closed_for_missing_indexed_record_or_candidate_cas(self) -> None:
        self._init()
        self._record()
        index = json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))
        candidate_path = self.workspace / index["candidates"]["c1"]
        candidate_path.unlink()
        with self.assertRaisesRegex(module.MatchError, "candidate record"):
            module.lookup_matches(self.root, self.workspace, self.source)

        cas_workspace = self.root / "build" / "lookup-cas"
        module.init_workspace(self.root, self.manifest, cas_workspace)
        recorded = module.record_candidate(
            self.root,
            cas_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        source_cas = cas_workspace / recorded["record"]["source_blob"]["cas_path"]
        source_cas.unlink()
        with self.assertRaisesRegex(module.MatchError, "candidate source_blob CAS|path component does not exist"):
            module.lookup_matches(self.root, cas_workspace, self.source)

        corrupt_workspace = self.root / "build" / "lookup-corrupt-cas"
        module.init_workspace(self.root, self.manifest, corrupt_workspace)
        corrupt = module.record_candidate(
            self.root,
            corrupt_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        object_cas = corrupt_workspace / corrupt["record"]["object_blob"]["cas_path"]
        object_cas.write_bytes(b"corrupt-object-cas")
        with self.assertRaisesRegex(module.MatchError, "candidate object_blob CAS"):
            module.lookup_matches(self.root, corrupt_workspace, self.source)

    def test_concurrent_record_calls_have_one_record_and_no_partial_state(self) -> None:
        self._init()
        def record() -> dict[str, object]:
            return self._record()

        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            results = list(pool.map(lambda _: record(), range(4)))
        self.assertEqual(sum(result["status"] == "recorded" for result in results), 1)
        self.assertEqual(sum(result["status"] == "unchanged" for result in results), 3)
        candidate_path = self.workspace / "candidates" / "c1.json"
        json.loads(candidate_path.read_text(encoding="utf-8"))
        self.assertFalse(any(path.name.endswith(".tmp") for path in self.workspace.rglob("*")))
        index = json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))
        self.assertEqual(index["sequence"], 1)

    def test_record_rejects_indexed_missing_candidate_and_recovers_exact_final_append(self) -> None:
        self._init()
        first = self._record("c1")
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate_path.unlink()
        with self.assertRaisesRegex(module.MatchError, "immutable candidate index entry"):
            self._record("c1")

        recovery_workspace = self.root / "build" / "recover-append"
        module.init_workspace(self.root, self.manifest, recovery_workspace)
        module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        source2 = self.root / "recovery-source-2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "recovery-object-2.o"
        object2.write_bytes(b"recovery-object-two")
        strict2 = self.root / "recovery-strict-2.json"
        _write_json(strict2, _report("fn"))
        module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c2",
            source=source2,
            object_path=object2,
            strict_report=strict2,
            data_report=None,
            hypothesis="natural candidate",
            axis="layout",
        )
        index_path = recovery_workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["candidates"].pop("c2")
        for mapping_name in ("source_context_index", "object_index"):
            index[mapping_name] = {
                key: value for key, value in index[mapping_name].items() if value != "c2"
            }
        index["sequence"] = 1
        c1_record = json.loads((recovery_workspace / "candidates" / "c1.json").read_text(encoding="utf-8"))
        index["last_record_sha256"] = c1_record["record_sha256"]
        index_path.write_text(json.dumps(_rehash(index, "index_sha256"), separators=(",", ":")), encoding="utf-8")

        recovered = module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c2",
            source=source2,
            object_path=object2,
            strict_report=strict2,
            data_report=None,
            hypothesis="natural candidate",
            axis="layout",
        )
        self.assertEqual(recovered["status"], "unchanged")
        final_index = json.loads(index_path.read_text(encoding="utf-8"))
        self.assertEqual(final_index["sequence"], 2)
        self.assertEqual(final_index["candidates"]["c2"], "candidates/c2.json")

    def test_diagnose_fails_closed_for_missing_indexed_result_without_running_job(self) -> None:
        self._init()
        self._record()
        marker = self.root / "missing-result-ran.marker"
        script = self._job_script(
            name="missing-result-probe.py",
            body="""
            import os, pathlib
            pathlib.Path(os.environ["MISSING_RESULT_MARKER"]).write_text("ran", encoding="utf-8")
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            """,
        )
        jobs = self._jobs(script, env={"MISSING_RESULT_MARKER": str(marker)})
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertTrue(marker.is_file())
        marker.unlink()
        result_path = self.workspace / "diagnostics" / f"{first['jobs'][0]['fingerprint']}.json"
        result_path.unlink()

        failed = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = failed["jobs"][0]
        self.assertEqual(row["status"], "failed")
        self.assertIn("no result event", row["error"])
        self.assertFalse(marker.exists(), "an indexed missing result must not launch the diagnostic")

    def test_matrix_rejects_orphan_candidate_and_diagnostic_context_records(self) -> None:
        self._init()
        self._record()
        orphan_candidate = self.workspace / "candidates" / "orphan.json"
        orphan_candidate.write_text("{}\n", encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "candidate index"):
            module.build_matrix(self.root, self.workspace)

        orphan_candidate.unlink()
        jobs = self._jobs(self._job_script("orphan-context.py"), job_ids=("orphan-context",))
        batch = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        old_fingerprint = batch["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{old_fingerprint}.json"
        result = json.loads(result_path.read_text(encoding="utf-8"))
        result["candidate_source_sha256"] = "0" * 64
        session = module._load_session(self.workspace, self.root)
        forged_identity = {
            "source": {"sha256": result["candidate_source_sha256"]},
            "object": {"sha256": result["candidate_object_sha256"]},
            "source_context_key": result["source_context_key"],
        }
        new_fingerprint = module._job_fingerprint(session, forged_identity, result["job_spec"])
        old_output_root = self.workspace / "job-output" / old_fingerprint
        new_output_root = self.workspace / "job-output" / new_fingerprint
        old_output_root.rename(new_output_root)
        for output in result["outputs"]:
            relative = Path(output["path"]).relative_to(old_output_root)
            output["path"] = str(new_output_root / relative)
        result["fingerprint"] = new_fingerprint
        forged_result_path = self.workspace / "diagnostics" / f"{new_fingerprint}.json"
        forged_result_path.write_text(
            json.dumps(_rehash(result, "result_sha256"), separators=(",", ":")),
            encoding="utf-8",
        )
        result_path.unlink()
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["diagnostic_index"].pop(old_fingerprint)
        index["diagnostic_index"][new_fingerprint] = f"diagnostics/{new_fingerprint}.json"
        index_path.write_text(
            json.dumps(_rehash(index, "index_sha256"), separators=(",", ":")),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(module.MatchError, "producer binding"):
            module.build_matrix(self.root, self.workspace)

    def test_parallel_diagnostics_are_isolated_sorted_and_cached(self) -> None:
        self._init()
        self._record()
        script = self._job_script()
        jobs = self._jobs(script, job_ids=("j2", "j1", "j3"), distinct=True)
        real_pool = module.ThreadPoolExecutor
        observed_workers: list[int | None] = []

        class RecordingPool(real_pool):
            def __init__(self, *args: object, **kwargs: object) -> None:
                observed_workers.append(kwargs.get("max_workers", args[0] if args else None))
                super().__init__(*args, **kwargs)

        with mock.patch.object(module, "ThreadPoolExecutor", RecordingPool):
            first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs, max_workers=2)
        self.assertEqual(observed_workers, [2], "diagnostics must use the requested bounded worker count")
        self.assertEqual([row["requested_job_id"] for row in first["jobs"]], ["j1", "j2", "j3"])
        self.assertEqual(first["summary"], {"ran": 3, "cached": 0, "failed": 0})
        self.assertTrue(all(row["status"] == "passed" for row in first["jobs"]))
        self.assertTrue(all(row["cache_status"] == "ran" for row in first["jobs"]))
        output_roots = list((self.workspace / "job-output").iterdir())
        self.assertEqual(len(output_roots), 3)
        self.assertTrue(
            all(json.loads((path / "result.json").read_text(encoding="utf-8"))["readonly"] == "1" for path in output_roots)
        )

        # A repeated request is a pure CAS cache hit.  Alias jobs in one batch
        # also prove that duplicate diagnostic fingerprints are reused.
        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs, max_workers=2)
        self.assertEqual(second["summary"], {"ran": 0, "cached": 3, "failed": 0})
        self.assertTrue(all(row["cache_status"] == "cached" for row in second["jobs"]))
        aliases = self._jobs(script, job_ids=("alias-a", "alias-b"), distinct=False)
        aliased = module.diagnose_candidate(self.root, self.workspace, "c1", aliases, max_workers=2)
        self.assertEqual(aliased["summary"], {"ran": 1, "cached": 1, "failed": 0})
        self.assertEqual([row["cache_status"] for row in aliased["jobs"]], ["ran", "deduplicated_in_run"])
        renamed = self._jobs(script, job_ids=("alias-c", "alias-d"), distinct=False)
        renamed_result = module.diagnose_candidate(self.root, self.workspace, "c1", renamed, max_workers=2)
        self.assertEqual(renamed_result["summary"], {"ran": 0, "cached": 2, "failed": 0})

    def test_deleted_cached_diagnostic_output_never_returns_stale_cache(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script(), job_ids=("cache-check",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        first_output = Path(first["jobs"][0]["outputs"][0]["path"])
        self.assertTrue(first_output.is_file())
        first_output.unlink()

        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = second["jobs"][0]
        self.assertNotEqual(row["cache_status"], "cached")
        self.assertIn(row["status"], {"passed", "failed"})
        if row["status"] == "passed":
            self.assertTrue(first_output.is_file(), "a safe rerun must recreate the private output")

    def test_malformed_cached_output_descriptor_fails_without_raw_exception(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script(), job_ids=("cache-shape",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = first["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{fingerprint}.json"
        result = json.loads(result_path.read_text(encoding="utf-8"))
        result["outputs"][0].pop("sha256")
        _write_json(result_path, _rehash(result, "result_sha256"))
        rerun = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(rerun["summary"]["failed"], 1)
        with self.assertRaisesRegex(module.MatchError, "required field"):
            module.build_matrix(self.root, self.workspace)

    def test_indexed_diagnostic_rejects_forged_byte_accounting_and_job_labels(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("event-accounting.py"), job_ids=("accounting",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = first["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{fingerprint}.json"
        original = json.loads(result_path.read_text(encoding="utf-8"))

        forged_bytes = dict(original)
        forged_bytes["output_bytes"] = 0
        _write_json(result_path, _rehash(forged_bytes, "result_sha256"))
        with self.assertRaisesRegex(module.MatchError, "byte accounting mismatch"):
            module.build_matrix(self.root, self.workspace)

        forged_label = dict(original)
        forged_label["kind"] = "forged-label"
        _write_json(result_path, _rehash(forged_label, "result_sha256"))
        with self.assertRaisesRegex(module.MatchError, "job labels do not match"):
            module.build_matrix(self.root, self.workspace)

    def test_diagnostics_are_isolated_for_distinct_source_contexts(self) -> None:
        self._init()
        self._record("c1")
        source2 = self.root / "source-context-2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "object-context-2.o"
        object2.write_bytes(b"object-context-two")
        self._record("c2", source=source2, object_path=object2)
        jobs = self._jobs(self._job_script("isolation.py"), job_ids=("same-job",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        second = module.diagnose_candidate(self.root, self.workspace, "c2", jobs)
        self.assertEqual(first["jobs"][0]["cache_status"], "ran")
        self.assertEqual(second["jobs"][0]["cache_status"], "ran")
        self.assertNotEqual(
            first["jobs"][0]["candidate_object_sha256"],
            second["jobs"][0]["candidate_object_sha256"],
        )
        output_roots = list((self.workspace / "job-output").iterdir())
        self.assertEqual(len(output_roots), 2)

    def test_matrix_does_not_attribute_same_object_diagnostics_to_other_source_context(self) -> None:
        self._init()
        self._record("c1")
        source2 = self.root / "source-context-same-object.c"
        source2.write_text("int fn(void) { return 3; }\n", encoding="utf-8")
        # The immutable object is intentionally shared, while the source
        # context differs.  A diagnostic result for c1 must not be projected
        # onto c2 merely because the object hash is equal.
        self._record("c2", source=source2, object_path=self.object)
        jobs = self._jobs(self._job_script("same-object.py"), job_ids=("same-object",))
        module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertIn(rows["c1"]["diagnostic_status"], {"available", "passed_read_only"})
        self.assertEqual(rows["c2"]["diagnostic_status"], "not_run")
        self.assertEqual(rows["c2"]["next_action"], "run_read_only_diagnostics_for_source_context")

    def test_matrix_ignores_unindexed_diagnostic_result(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("unindexed.py"), job_ids=("unindexed",))
        batch = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = batch["jobs"][0]["fingerprint"]
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["diagnostic_index"].pop(fingerprint, None)
        index_body = dict(index)
        index_body.pop("index_sha256", None)
        canonical = json.dumps(index_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        index["index_sha256"] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
        index_path.write_text(json.dumps(index, separators=(",", ":")), encoding="utf-8")
        try:
            matrix = module.build_matrix(self.root, self.workspace)
        except module.MatchError as exc:
            self.assertIn("diagnostic index", str(exc))
        else:
            self.assertEqual(matrix["rows"][0]["diagnostic_status"], "not_run")

    def test_diagnostic_stdout_over_bound_is_rejected(self) -> None:
        script = self._job_script(
            name="verbose-probe.py",
            body="""
            import os, pathlib, sys
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            sys.stdout.write("x" * 4096)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertTrue(row["stdout_truncated"])
        self.assertNotEqual(row["status"], "passed")

    def test_declared_output_file_over_bound_is_rejected(self) -> None:
        script = self._job_script(
            name="large-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_bytes(b"x" * 4096)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertNotEqual(row["status"], "passed")
        self.assertTrue(row.get("output_limit_exceeded") or "output" in row.get("error", "").lower())

    def test_stdout_stderr_and_declared_outputs_share_one_budget(self) -> None:
        script = self._job_script(
            name="combined-output-probe.py",
            body="""
            import os, pathlib, sys
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            sys.stdout.write("s" * 400)
            sys.stderr.write("e" * 400)
            (output / "result.json").write_bytes(b"o" * 400)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertEqual(row["status"], "failed")
        self.assertTrue(row["output_limit_exceeded"])
        self.assertGreater(row["output_bytes"], 1024)

    def test_undeclared_private_output_is_rejected_and_indexed_as_failure(self) -> None:
        script = self._job_script(
            name="undeclared-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            (output / "undeclared.bin").write_bytes(b"not-declared")
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(result["summary"], {"ran": 1, "cached": 0, "failed": 1})
        self.assertEqual(result["jobs"][0]["status"], "failed")
        self.assertIn("undeclared private outputs", result["jobs"][0]["error"])
        matrix = module.build_matrix(self.root, self.workspace)
        self.assertEqual(matrix["rows"][0]["diagnostic_status"], "failed")

    def test_declared_output_directory_is_cleaned_and_indexed_as_failure(self) -> None:
        script = self._job_script(
            name="directory-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            (output / "result.json").mkdir(parents=True)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(result["summary"], {"ran": 1, "cached": 0, "failed": 1})
        self.assertIn("not a regular file", result["jobs"][0]["error"])
        self.assertFalse(Path(result["jobs"][0]["outputs"][0]["path"]).exists())
        matrix = module.build_matrix(self.root, self.workspace)
        self.assertEqual(matrix["rows"][0]["diagnostic_status"], "failed")

    def test_timeout_change_creates_a_new_diagnostic_fingerprint(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("timeout-fingerprint.py"), timeout_seconds=10)
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        first_row = first["jobs"][0]
        jobs_value = json.loads(jobs.read_text(encoding="utf-8"))
        jobs_value["jobs"][0]["timeout_seconds"] = 11
        _write_json(jobs, jobs_value)
        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        second_row = second["jobs"][0]
        self.assertNotEqual(first_row["fingerprint"], second_row["fingerprint"])
        self.assertEqual(second_row["cache_status"], "ran")
        self.assertEqual(len(list((self.workspace / "diagnostics").glob("*.json"))), 2)

    def test_arbitrary_parent_environment_is_not_inherited(self) -> None:
        script = self._job_script(
            name="environment-probe.py",
            body="""
            import json, os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text(json.dumps({"secret": os.environ.get("MATCH_TEST_SECRET", "missing")}), encoding="utf-8")
            """,
        )
        self._init()
        self._record()
        old_secret = os.environ.get("MATCH_TEST_SECRET")
        os.environ["MATCH_TEST_SECRET"] = "must-not-leak"
        try:
            jobs = self._jobs(script)
            result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        finally:
            if old_secret is None:
                os.environ.pop("MATCH_TEST_SECRET", None)
            else:
                os.environ["MATCH_TEST_SECRET"] = old_secret
        output_path = Path(result["jobs"][0]["outputs"][0]["path"])
        self.assertEqual(json.loads(output_path.read_text(encoding="utf-8"))["secret"], "missing")

    def test_serial_native_proof_compiler_and_authority_resources_rejected_before_subprocess(self) -> None:
        self._init()
        self._record()
        marker = self.root / "executed.marker"
        script = self._job_script(
            body="""
            import pathlib, os
            pathlib.Path(os.environ["EXECUTED_MARKER"]).write_text("executed", encoding="utf-8")
            """
        )
        for resource in ("compiler", "native_debug", "proof", "authority", "retail_link"):
            jobs = self._jobs(script, resource_class=resource, env={"EXECUTED_MARKER": str(marker)})
            with self.assertRaisesRegex(module.MatchError, "serial resource class"):
                module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertFalse(marker.exists())

    def test_job_input_toctou_is_reported_and_output_escape_rejected(self) -> None:
        self._init()
        self._record()
        mutable = self.root / "mutable.input"
        mutable.write_text("before", encoding="utf-8")
        script = self._job_script(
            name="mutate.py",
            body="""
            import os, pathlib
            pathlib.Path(os.environ["MUTABLE_INPUT"]).write_text("after", encoding="utf-8")
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            """,
        )
        jobs_path = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        jobs = json.loads(jobs_path.read_text(encoding="utf-8"))
        jobs["jobs"][0]["inputs"].append(_descriptor(mutable))
        _write_json(jobs_path, jobs)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs_path)
        self.assertEqual(result["summary"]["failed"], 1)
        self.assertIn("changed from its authenticated descriptor", result["jobs"][0]["error"])

        escape = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        escaped = json.loads(escape.read_text(encoding="utf-8"))
        escaped["jobs"][0]["outputs"] = ["../outside.json"]
        _write_json(escape, escaped)
        with self.assertRaisesRegex(module.MatchError, "output must be a relative contained path"):
            module.diagnose_candidate(self.root, self.workspace, "c1", escape)

        embedded = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        embedded_value = json.loads(embedded.read_text(encoding="utf-8"))
        embedded_value["jobs"][0]["argv"] = [str(script), "--config={workspace}/index.json"]
        _write_json(embedded, embedded_value)
        with self.assertRaisesRegex(module.MatchError, "placeholders must occupy the entire"):
            module.diagnose_candidate(self.root, self.workspace, "c1", embedded)

    def test_matrix_is_self_hashed_deterministic_and_points_to_next_action(self) -> None:
        self._init()
        first = self._record(data_report=self.data)
        source2 = self.root / "source2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "object2.o"
        object2.write_bytes(b"object-two")
        strict2 = self.root / "strict2.json"
        _write_json(strict2, _report("fn", exact=True))
        self._record("c2", source=source2, object_path=object2, strict_report=strict2, data_report=self.data, axis="layout")
        matrix = module.build_matrix(self.root, self.workspace)
        matrix_body = dict(matrix)
        matrix_hash = matrix_body.pop("matrix_sha256")
        canonical = json.dumps(matrix_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        self.assertEqual(matrix_hash, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        self.assertEqual([row["candidate_id"] for row in matrix["rows"]], ["c1", "c2"])
        self.assertEqual(matrix["aggregate"]["candidate_count"], 2)
        self.assertEqual(matrix["rows"][0]["next_action"], "continue_one_axis_matching")
        self.assertEqual(
            matrix["rows"][1]["next_action"],
            "authenticate_report_binding_then_run_serial_proof_and_closure",
        )
        self.assertEqual(matrix, module.build_matrix(self.root, self.workspace))

    def test_matrix_rejected_is_fail_closed_but_retained_keeps_closure_action(self) -> None:
        self._init()
        rejected = self._record(
            "rejected",
            data_report=self.data,
            status="rejected",
            reason="target-shaped probe is a no-go",
        )
        source2 = self.root / "retained-source.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "retained-object.o"
        object2.write_bytes(b"retained-object")
        strict2 = self.root / "retained-strict.json"
        _write_json(strict2, _report("fn", exact=True))
        retained = self._record(
            "retained",
            source=source2,
            object_path=object2,
            strict_report=strict2,
            data_report=self.data,
            status="retained",
            reason="natural exact candidate",
        )

        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(rejected["record"]["outcome"]["status"], "rejected")
        self.assertEqual(rows["rejected"]["next_action"], "do_not_advance_rejected_candidate")
        self.assertEqual(
            rows["retained"]["next_action"],
            "authenticate_report_binding_then_run_serial_proof_and_closure",
        )
        self.assertFalse(rows["rejected"]["next_action"].startswith("authenticate"))
        self.assertEqual(retained["record"]["outcome"]["status"], "retained")

    def test_record_focus_symbol_drives_compacts_and_legacy_defaults_to_session(self) -> None:
        self._init()
        focused_strict = self.root / "focused-strict.json"
        focused_data = self.root / "focused-data.json"
        _write_json(focused_strict, _report("other", exact=False))
        _write_json(focused_data, _report("other", exact=True))
        focused = self._record(
            "focused",
            strict_report=focused_strict,
            data_report=focused_data,
            focus_symbol="other",
        )
        record = focused["record"]
        self.assertEqual(record["focus_symbol"], "other")
        self.assertEqual(record["reports"]["strict"]["compact"]["focus"]["name"], "other")
        self.assertEqual(record["reports"]["data"]["compact"]["focus"]["name"], "other")

        focused_body = dict(record)
        focused_hash = focused_body.pop("record_sha256")
        canonical = json.dumps(
            focused_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ) + "\n"
        self.assertEqual(focused_hash, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        self.assertEqual(
            module.record_candidate(
                self.root,
                self.workspace,
                candidate_id="focused",
                source=self.source,
                object_path=self.object,
                strict_report=focused_strict,
                data_report=focused_data,
                hypothesis="natural candidate",
                axis="register-lifetime",
                focus_symbol="other",
            )["status"],
            "unchanged",
        )
        cli_output = io.StringIO()
        with contextlib.redirect_stdout(cli_output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "record",
                        "--workspace",
                        str(self.workspace),
                        "--candidate-id",
                        "cli",
                        "--source",
                        str(self.source),
                        "--object",
                        str(self.object),
                        "--strict-report",
                        str(focused_strict),
                        "--data-report",
                        str(focused_data),
                        "--hypothesis",
                        "natural candidate",
                        "--axis",
                        "register-lifetime",
                        "--focus-symbol",
                        "other",
                        "--json",
                    ]
                ),
                0,
            )
        cli_record = json.loads(cli_output.getvalue())["record"]
        self.assertEqual(cli_record["focus_symbol"], "other")

        legacy = self._record("legacy")
        self.assertNotIn("focus_symbol", legacy["record"])
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(rows["legacy"]["focus_symbol"], "fn")
        self.assertEqual(rows["legacy"]["strict_focus"]["name"], "fn")

    def test_focus_symbol_is_optional_but_candidate_schema_remains_closed(self) -> None:
        self._init()
        self._record("c1", focus_symbol="other")
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate["unexpected"] = True
        _write_json(candidate_path, _rehash(candidate, "record_sha256"))
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["last_record_sha256"] = json.loads(candidate_path.read_text(encoding="utf-8"))["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))
        with self.assertRaisesRegex(module.MatchError, "candidate record contains unknown field"):
            module.build_matrix(self.root, self.workspace)

    def test_matrix_rejects_malformed_compact_focus_without_raw_exception(self) -> None:
        self._init()
        self._record(data_report=self.data)
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate["reports"]["strict"]["compact"]["focus"] = "not-an-object"
        candidate = _rehash(candidate, "record_sha256")
        _write_json(candidate_path, candidate)
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["last_record_sha256"] = candidate["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))
        with self.assertRaisesRegex(module.MatchError, "compact.focus"):
            module.build_matrix(self.root, self.workspace)

    def test_duplicate_object_with_different_report_does_not_reuse_evidence(self) -> None:
        self._init()
        self._record("c1")
        exact = self.root / "strict-exact-duplicate.json"
        _write_json(exact, _report("fn", exact=True))
        second = self._record("c2", strict_report=exact)
        self.assertEqual(second["record"]["duplicate_of"], "c1")
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(
            rows["c2"]["next_action"],
            "run_read_only_diagnostics_for_source_context",
        )

    def test_assess_reports_focus_delta_counts_changed_siblings_and_central_json(self) -> None:
        baseline_strict = self.root / "baseline-strict.json"
        candidate_strict = self.root / "candidate-strict.json"
        baseline_data = self.root / "baseline-data.json"
        candidate_data = self.root / "candidate-data.json"
        _write_json(
            baseline_strict,
            _assessment_report(
                focus_match=75.0,
                focus_size="4",
                focus_candidate_size="4",
                sibling_match=100.0,
                sibling_size="8",
                sibling_candidate_size="8",
            ),
        )
        _write_json(
            candidate_strict,
            _assessment_report(
                focus_match=100.0,
                focus_size="4",
                focus_candidate_size="6",
                sibling_match=100.0,
                sibling_size="8",
                sibling_candidate_size="9",
            ),
        )
        _write_json(baseline_data, _assessment_report(focus_match=75.0))
        _write_json(candidate_data, _assessment_report(focus_match=100.0))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline_strict),
                        "--candidate-strict",
                        str(candidate_strict),
                        "--baseline-data",
                        str(baseline_data),
                        "--candidate-data",
                        str(candidate_data),
                        "--focus-symbol",
                        "focus",
                    ]
                ),
                0,
            )
        result = json.loads(output.getvalue())
        self.assertEqual(result["schema"], module.ASSESSMENT_SCHEMA)
        self.assertEqual(result["verdict"], "accepted")
        strict = result["reports"]["strict"]
        self.assertEqual(strict["exact_function_counts"], {"before": {"exact": 1, "total": 2}, "after": {"exact": 2, "total": 2}})
        self.assertEqual(strict["focus"]["before"]["size"], 4)
        self.assertEqual(strict["focus"]["after"]["candidate_size"], 6)
        self.assertEqual(strict["focus"]["delta"]["match_percent"], 25)
        self.assertEqual(strict["focus"]["delta"]["diff_kind_delta"], {"REG_SWAP": -1})
        changed = [row for row in result["changed_siblings"] if row["report"] == "strict"]
        self.assertEqual([row["symbol"] for row in changed], ["sibling"])
        self.assertEqual(changed[0]["delta"]["candidate_size"], 1)
        self.assertEqual(result["regressions"], [])

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "assess",
                "--baseline-strict",
                str(baseline_strict),
                "--candidate-strict",
                str(candidate_strict),
                "--focus-symbol",
                "focus",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout)["verdict"], "accepted")

    def test_assess_rejects_previously_exact_sibling_regression(self) -> None:
        baseline = self.root / "regression-baseline.json"
        candidate = self.root / "regression-candidate.json"
        _write_json(baseline, _assessment_report(focus_match=75.0, sibling_match=100.0))
        _write_json(candidate, _assessment_report(focus_match=100.0, sibling_match=95.0))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                        "--json",
                    ]
                ),
                1,
            )
        result = json.loads(output.getvalue())
        self.assertEqual(result["verdict"], "rejected")
        self.assertEqual(len(result["regressions"]), 1)
        self.assertEqual(result["regressions"][0]["symbol"], "sibling")
        self.assertEqual(result["regressions"][0]["reason"], "previously_exact_sibling_regressed")

    def test_assess_rejects_missing_or_unpaired_focus(self) -> None:
        baseline = self.root / "focus-baseline.json"
        candidate = self.root / "focus-candidate.json"
        _write_json(baseline, _assessment_report())
        _write_json(candidate, _assessment_report())

        missing_output = io.StringIO()
        with contextlib.redirect_stdout(missing_output):
            missing_status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline),
                    "--candidate-strict",
                    str(candidate),
                    "--focus-symbol",
                    "missing",
                ]
            )
        self.assertEqual(missing_status, 2)
        self.assertIn("lacks requested focus symbol", missing_output.getvalue())

        unpaired = _assessment_report()
        unpaired["right"]["symbols"][0]["name"] = "different"
        unpaired["left"]["symbols"][0]["target_symbol"] = 99
        _write_json(candidate, unpaired)
        unpaired_output = io.StringIO()
        with contextlib.redirect_stdout(unpaired_output):
            unpaired_status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline),
                    "--candidate-strict",
                    str(candidate),
                    "--focus-symbol",
                    "focus",
                ]
            )
        self.assertEqual(unpaired_status, 2)
        self.assertIn("is not paired", unpaired_output.getvalue())

    def test_assess_rejects_invalid_target_index_even_with_same_name(self) -> None:
        baseline = self.root / "target-index-baseline.json"
        candidate = self.root / "target-index-candidate.json"
        _write_json(baseline, _assessment_report())

        missing = object()
        for label, target_index in (
            ("absent", missing),
            ("null", None),
            ("out-of-range", 99),
        ):
            malformed = _assessment_report()
            focus = malformed["left"]["symbols"][0]
            if target_index is missing:
                del focus["target_symbol"]
            else:
                focus["target_symbol"] = target_index
            # The right-side symbol deliberately keeps the same name.  A
            # canonical objdiff report must trust only target_symbol.
            _write_json(candidate, malformed)
            output = io.StringIO()
            with self.subTest(target_symbol=label), contextlib.redirect_stdout(output):
                status = module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                    ]
                )
            self.assertEqual(status, 2)
            self.assertIn("is not paired", output.getvalue())

    def test_assess_rejects_metadata_or_symbol_free_reports(self) -> None:
        baseline = self.root / "shape-baseline.json"
        candidate = self.root / "shape-candidate.json"
        _write_json(candidate, _assessment_report())
        for malformed in (
            {"metadata": {"tool": "objdiff"}},
            {"left": {"symbols": []}, "right": {"symbols": []}},
        ):
            _write_json(baseline, malformed)
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                status = module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                    ]
                )
            self.assertEqual(status, 2)
            self.assertIn("error:", output.getvalue())

    def test_assess_rejects_strict_data_focus_pairing_mismatch(self) -> None:
        baseline_strict = self.root / "pair-baseline-strict.json"
        candidate_strict = self.root / "pair-candidate-strict.json"
        baseline_data = self.root / "pair-baseline-data.json"
        candidate_data = self.root / "pair-candidate-data.json"
        _write_json(baseline_strict, _assessment_report())
        _write_json(candidate_strict, _assessment_report())
        data_baseline = _assessment_report()
        data_candidate = _assessment_report()
        data_baseline["right"]["symbols"][0]["name"] = "different"
        data_candidate["right"]["symbols"][0]["name"] = "different"
        _write_json(baseline_data, data_baseline)
        _write_json(candidate_data, data_candidate)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline_strict),
                    "--candidate-strict",
                    str(candidate_strict),
                    "--baseline-data",
                    str(baseline_data),
                    "--candidate-data",
                    str(candidate_data),
                    "--focus-symbol",
                    "focus",
                ]
            )
        self.assertEqual(status, 2)
        self.assertIn("strict/data focus pairing mismatch", output.getvalue())

    def test_missing_index_with_existing_records_fails_closed(self) -> None:
        self._init()
        self._record()
        (self.workspace / "index.json").unlink()
        with self.assertRaisesRegex(module.MatchError, "index"):
            module.build_matrix(self.root, self.workspace)

    def test_direct_module_cli_and_central_agent_routing(self) -> None:
        manifest_arg = str(self.manifest)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(module.main(["--root", str(self.root), "init", manifest_arg, "--workspace", str(self.workspace), "--json"]), 0)
        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [sys.executable, str(central), "--root", str(self.root), "match", "lookup", "--workspace", str(self.workspace), "--source", str(self.source), "--json"],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout)["status"], "new")

    def test_default_text_diagnose_and_matrix_output_has_operational_counts(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("text-output.py"), job_ids=("text-output",))
        diagnose_text = io.StringIO()
        with contextlib.redirect_stdout(diagnose_text):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "diagnose",
                        "--workspace",
                        str(self.workspace),
                        "--candidate-id",
                        "c1",
                        "--jobs",
                        str(jobs),
                    ]
                ),
                0,
            )
        diagnose_output = diagnose_text.getvalue().lower()
        self.assertIn("ran", diagnose_output)
        self.assertIn("failed", diagnose_output)

        matrix_text = io.StringIO()
        with contextlib.redirect_stdout(matrix_text):
            self.assertEqual(
                module.main(["--root", str(self.root), "matrix", "--workspace", str(self.workspace)]),
                0,
            )
        matrix_output = matrix_text.getvalue().lower()
        self.assertIn("candidate", matrix_output)
        self.assertIn("next", matrix_output)


if __name__ == "__main__":
    unittest.main()
