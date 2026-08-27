import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import tools.match_workbench as match
from tools.recovery_memory import RecoveryMemory, sync_match_workbenches
from tools.tests.test_match_workbench import _descriptor, _report, _write_json


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        text=True,
        capture_output=True,
        check=True,
    )


class MatchWorkbenchGlobalMemoryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        run(self.root, "git", "init", "-q", "-b", "main")
        run(self.root, "git", "config", "user.email", "test@example.com")
        run(self.root, "git", "config", "user.name", "Test")
        self.target = self.root / "target.o"
        self.source = self.root / "candidate.c"
        self.object = self.root / "candidate.o"
        self.strict = self.root / "strict.json"
        self.data = self.root / "data.json"
        self.target.write_bytes(b"target-bytes")
        self.source.write_text("int fn(void) { return 1; }\n", encoding="utf-8")
        self.object.write_bytes(b"candidate-object")
        _write_json(self.strict, _report("fn"))
        _write_json(self.data, _report("fn", exact=True))
        self.manifest = self.root / "request.json"
        _write_json(
            self.manifest,
            {
                "schema": match.REQUEST_SCHEMA,
                "schema_version": 1,
                "session_id": "global-memory-test",
                "owner": "main:board/example",
                "unit": "example.c",
                "function": "fn",
                "target": _descriptor(self.target),
                "context": {
                    "base_commit": "base-commit",
                    "toolchain_key": "GC/2.6",
                    "compiler": None,
                    "compile_argv": [],
                    "compile_inputs": [],
                },
                "policy": {
                    "max_workers": 2,
                    "max_report_bytes": 2_000_000,
                    "max_compact_bytes": 16_000,
                    "allowed_job_kinds": ["artifact-fact"],
                },
            },
        )
        run(self.root, "git", "add", ".")
        run(self.root, "git", "commit", "-qm", "fixture")
        self.workspace = self.root / "build" / "match-a"
        match.init_workspace(self.root, self.manifest, self.workspace)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def attestation(self, workspace: Path, label: str, source: Path, obj: Path) -> Path:
        path = self.root / f"{label}-attestation.json"
        match.create_compile_attestation(
            self.root,
            workspace,
            source=source,
            object_path=obj,
            output=path,
            producer_kind="test-fixture",
            producer_command=[],
            notes="global memory integration test",
        )
        return path

    def test_lookup_admission_record_and_cross_workspace_hit(self) -> None:
        lookup = match.lookup_matches(
            self.root,
            self.workspace,
            self.source,
            shape_key="direct-typed-consumer",
            hypothesis="consume result directly",
            axis="lifetime",
        )
        self.assertEqual(lookup["status"], "new")
        self.assertEqual(lookup["global_memory"]["status"], "admitted")
        recorded = match.record_candidate(
            self.root,
            self.workspace,
            candidate_id="c001",
            source=self.source,
            object_path=self.object,
            compile_attestation=self.attestation(
                self.workspace, "c001", self.source, self.object
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="consume result directly",
            axis="lifetime",
            status="exact",
            reason="zero rows",
        )
        self.assertEqual(recorded["global_memory"]["status"], "recorded")

        second_workspace = self.root / "build" / "match-b"
        match.init_workspace(self.root, self.manifest, second_workspace)
        copied_source = self.root / "copy.c"
        copied_source.write_bytes(self.source.read_bytes())
        second = match.lookup_matches(
            self.root, second_workspace, copied_source
        )
        self.assertEqual(second["status"], "known_global_source")
        self.assertTrue(second["skip_compile"])
        self.assertEqual(
            second["global_memory"]["experiment"]["object_sha256"],
            recorded["record"]["object"]["sha256"],
        )
        synced = sync_match_workbenches(self.root)
        self.assertEqual(synced["failures"], [])
        self.assertEqual(synced["imported"], 0)
        self.assertGreaterEqual(
            synced["unchanged"] + synced["observations_imported"], 1
        )

    def test_pre_registry_workbench_history_is_imported_at_startup(self) -> None:
        historical = self.root / "historical.c"
        historical_object = self.root / "historical.o"
        historical.write_text("int fn(void) { return 3; }\n", encoding="utf-8")
        historical_object.write_bytes(b"historical-object")
        with mock.patch.object(
            match, "recovery_memory_available", return_value=False
        ), mock.patch(
            "tools.recovery_memory.recovery_memory_available", return_value=False
        ):
            lookup = match.lookup_matches(
                self.root, self.workspace, historical
            )
            self.assertEqual(lookup["status"], "new")
            recorded = match.record_candidate(
                self.root,
                self.workspace,
                candidate_id="historical-c001",
                source=historical,
                object_path=historical_object,
                compile_attestation=self.attestation(
                    self.workspace,
                    "historical-c001",
                    historical,
                    historical_object,
                ),
                strict_report=self.strict,
                data_report=self.data,
                hypothesis="historical source shape",
                axis="lifetime",
                status="measured",
                reason="pre-registry candidate",
            )
            self.assertEqual(
                recorded["global_memory"]["status"],
                "unavailable_non_git_fixture",
            )

        synced = sync_match_workbenches(self.root)
        self.assertEqual(synced["failures"], [])
        self.assertEqual(synced["imported"], 1)
        context = RecoveryMemory.for_root(self.root).context_memory(
            "main:board/example", "fn"
        )
        self.assertEqual(len(context["experiments"]), 1)
        self.assertEqual(context["experiments"][0]["status"], "nonexact")
        self.assertEqual(
            context["experiments"][0]["candidate_id"], "historical-c001"
        )

    def test_record_without_lookup_and_postcompile_lookup_fail_closed(self) -> None:
        source = self.root / "new.c"
        obj = self.root / "new.o"
        source.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        obj.write_bytes(b"new-object")
        with self.assertRaisesRegex(match.MatchError, "no pending central"):
            match.record_candidate(
                self.root,
                self.workspace,
                candidate_id="c002",
                source=source,
                object_path=obj,
                compile_attestation=self.attestation(
                    self.workspace, "c002", source, obj
                ),
                strict_report=self.strict,
                data_report=self.data,
                hypothesis="new source",
                axis="lifetime",
            )
        lookup = match.lookup_matches(
            self.root, self.workspace, source, obj
        )
        self.assertEqual(lookup["status"], "post_compile_lookup_not_admitted")
        self.assertTrue(lookup["skip_compile"])


if __name__ == "__main__":
    unittest.main()
