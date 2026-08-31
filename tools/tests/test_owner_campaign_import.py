from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock

from tools import owner_campaign
from tools import owner_campaign_import
from tools import owner_campaign_manifest


def _digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _seal(value: dict[str, object], field: str) -> dict[str, object]:
    result = dict(value)
    result[field] = _digest(owner_campaign_import._canonical(value))
    return result


def _git(root: Path, *args: str) -> str:
    return subprocess.check_output(["git", *args], cwd=root, text=True).strip()


class LegacyImportTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name).resolve()
        (self.root / "src").mkdir()
        (self.root / "build" / "evidence").mkdir(parents=True)
        self.source = self.root / "src" / "owner.c"
        self.source.write_text("int focus(void) { return 0; }\n", encoding="utf-8")
        self.target = self.root / "build" / "evidence" / "target.o"
        self.target.write_bytes(b"target object")
        self.toolchain = self.root / "build" / "evidence" / "toolchain.json"
        self.toolchain.write_bytes(b"toolchain\n")
        self.producer = self.root / "build" / "evidence" / "producer.py"
        self.producer.write_bytes(b"# fixture\n")
        subprocess.run(["git", "init", "-q"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "core.autocrlf", "false"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "user.email", "test@example.invalid"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "user.name", "Import Test"], cwd=self.root, check=True)
        subprocess.run(["git", "add", "src/owner.c"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "fixture"], cwd=self.root, check=True)
        self.commit = _git(self.root, "rev-parse", "HEAD")
        self.functions = [f"f{index:02d}" for index in range(51)]
        self.protected = self.functions[:50]
        self.manifest_path = self.root / "build" / "campaign.json"
        owner_campaign_manifest.initialize_campaign(
            self.root,
            campaign=self.manifest_path,
            campaign_id="legacy-import-test",
            owner="main:test/owner",
            unit="main/test/owner",
            source_relpath="src/owner.c",
            base_commit=self.commit,
            target_object=self.target,
            toolchain=self.toolchain,
            measurement_producer=self.producer,
            functions=self.functions,
            protected_exact_functions=self.protected,
            allowed_source_paths=["src/owner.c"],
            allowed_build_paths=["build"],
            final_owner_command=["python", "{MEASUREMENT_PRODUCER}"],
        )
        self.campaign = owner_campaign.load_campaign(self.root, self.manifest_path)
        self.source_sha = owner_campaign._digest_file(self.source)
        self.target_sha = owner_campaign._digest_file(self.target)
        self.toolchain_sha = owner_campaign._digest_file(self.toolchain)
        self.legacy = self.root / "legacy"
        self.legacy.mkdir()

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _compact_exact(self, function: str, *, source_sha: str | None = None) -> Path:
        source_sha = source_sha or self.source_sha
        candidate_object = _digest(f"candidate:{function}".encode())
        proof_receipts = {
            name: _digest(f"{function}:{name}".encode())
            for name in ("strict", "data", "physical", "siblings", "source_link")
        }
        body: dict[str, object] = {
            "schema": owner_campaign_import.LEGACY_EXACT_SCHEMA,
            "owner": self.campaign["owner"], "unit": self.campaign["unit"],
            "function": function, "base_commit": self.commit,
            "source_sha256": source_sha, "target_object_sha256": self.target_sha,
            "candidate_object_sha256": candidate_object, "toolchain_sha256": self.toolchain_sha,
            "target_bytes": 128, "candidate_bytes": 128,
            "strict_differences": 0, "data_differences": 0,
            "physical_target_count": 3, "physical_candidate_count": 3,
            "physical_differences": 0, "protected_total": len(self.protected),
            "protected_losses": 0, "source_link_exact": True, "compiled": True, "exact": True,
            "proof_receipts": proof_receipts,
            "completed_at": "2026-08-31T10:00:00Z",
        }
        path = self.legacy / f"{function}.json"
        path.write_bytes(owner_campaign_import._canonical(_seal(body, "report_sha256")))
        return path

    def _compact_outcome(self, function: str) -> Path:
        body: dict[str, object] = {
            "schema": owner_campaign_import.LEGACY_OUTCOME_SCHEMA,
            "owner": self.campaign["owner"], "unit": self.campaign["unit"],
            "function": function, "base_commit": self.commit,
            "source_sha256": self.source_sha, "target_object_sha256": self.target_sha,
            "candidate_object_sha256": _digest(f"neutral-object:{function}".encode()),
            "toolchain_sha256": self.toolchain_sha,
            "candidate_source_sha256": _digest(f"neutral-source:{function}".encode()),
            "status": "no_gain", "compiled": True,
            "strict_difference_delta": 0, "data_difference_delta": 0,
            "physical_difference_delta": 0,
            "completed_at": "2026-08-31T10:01:00Z",
        }
        path = self.legacy / f"{function}-neutral.json"
        path.write_bytes(owner_campaign_import._canonical(_seal(body, "outcome_sha256")))
        return path

    def _old_no_gain_result(self, function: str) -> Path:
        candidate_source = _digest(f"old-candidate-source:{function}".encode())
        candidate_object = _digest(f"old-candidate-object:{function}".encode())
        report_artifact = _digest(f"old-report:{function}".encode())

        def summary(name: str, **extra: object) -> dict[str, object]:
            return {
                "owner": self.campaign["owner"], "function": function,
                "candidate_source_sha256": candidate_source,
                "target_object_sha256": self.target_sha,
                "candidate_object_sha256": candidate_object,
                "report_sha256": report_artifact, **extra,
            }

        receipts: dict[str, object] = {
            "compile": {
                "baseline_command": {"returncode": 0},
                "candidate_command": {"returncode": 0},
            },
            "strict": {"summary": summary("strict", strict_percent=99.0, target_bytes=100, candidate_bytes=96, differences=1)},
            "data": {"summary": summary("data", data_percent=99.0, target_bytes=100, candidate_bytes=96, differences=1)},
            "physical": {"summary": summary("physical", target_count=2, candidate_count=2, differences=0)},
        }
        body: dict[str, object] = {
            "schema": "crack_harness_result/v1", "approval_id": "old-approval",
            "approval_sha256": _digest(b"old approval"), "owner": self.campaign["owner"],
            "task_id": "old-task", "function": function, "base_commit": self.commit,
            "campaign_id": "old-campaign", "attempt_sha256": _digest(b"attempt"),
            "candidate_sha256": candidate_source, "base_sha256": self.source_sha,
            "status": "no_gain", "expected_terminal": "improved",
            "terminal_expectation_met": False, "reason": "neutral",
            "owner_gain": 0, "predicted_rows": [], "receipts": receipts,
            "finished_at": "2026-08-31T10:02:00Z", "source_restored": True,
            "cleanup_status": "complete", "cleanup_errors": [], "authority_advanced": False,
        }
        path = self.legacy / f"{function}-old-result.json"
        path.write_bytes(owner_campaign_import._canonical(_seal(body, "result_sha256")))
        return path

    def test_seed_50_of_51_and_neutral_dedupe(self) -> None:
        exact_paths = [self._compact_exact(function) for function in self.functions[:50]]
        neutral = self._compact_outcome("f50")
        result = owner_campaign_import.import_legacy(
            self.root, self.manifest_path, exact_paths, consumed_paths=[neutral]
        )
        self.assertEqual(result["status"], "imported")
        self.assertEqual(result["exact_count"], 50)
        self.assertEqual(len(result["exact_imported"]), 50)
        status = owner_campaign.campaign_status(self.root, self.campaign)
        self.assertEqual(status["exact_count"], 50)
        self.assertFalse((owner_campaign._function_root(self.root, self.campaign, "f50") / "exact-manifest.json").exists())
        ledger = owner_campaign._function_root(self.root, self.campaign, "f50") / "candidate-results.jsonl"
        self.assertIn('"status":"no_gain"', ledger.read_text(encoding="utf-8"))

    def test_import_is_idempotent(self) -> None:
        paths = [self._compact_exact("f00")]
        first = owner_campaign_import.import_legacy(self.root, self.manifest_path, paths)
        manifest = owner_campaign._owner_root(self.root, self.campaign) / "exact-manifest.json"
        before = manifest.read_bytes()
        second = owner_campaign_import.import_legacy(self.root, self.manifest_path, paths)
        self.assertEqual(first["status"], "imported")
        self.assertEqual(second["status"], "already_imported")
        self.assertEqual(before, manifest.read_bytes())

    def test_old_no_gain_result_is_dedupe_only(self) -> None:
        path = self._old_no_gain_result("f50")
        result = owner_campaign_import.import_legacy(
            self.root, self.manifest_path, consumed_paths=[path]
        )
        self.assertEqual(result["exact_count"], 0)
        self.assertEqual(result["outcome_imported"], ["f50"])
        function_root = owner_campaign._function_root(self.root, self.campaign, "f50")
        self.assertTrue((function_root / "candidate-results.jsonl").is_file())
        self.assertFalse((owner_campaign._owner_root(self.root, self.campaign) / "exact-manifest.json").exists())

    def test_stale_source_is_rejected_before_write(self) -> None:
        path = self._compact_exact("f00", source_sha="0" * 64)
        with self.assertRaises(owner_campaign_import.LegacyImportError):
            owner_campaign_import.import_legacy(self.root, self.manifest_path, [path])
        self.assertFalse((owner_campaign._owner_root(self.root, self.campaign) / "exact-manifest.json").exists())

    def test_forged_exact_gates_are_rejected(self) -> None:
        path = self._compact_exact("f00")
        value = json.loads(path.read_text(encoding="utf-8"))
        value["physical_differences"] = 1
        value["report_sha256"] = owner_campaign_import._digest_json({k: v for k, v in value.items() if k != "report_sha256"})
        path.write_bytes(owner_campaign_import._canonical(value))
        with self.assertRaises(owner_campaign_import.LegacyImportError):
            owner_campaign_import.import_legacy(self.root, self.manifest_path, [path])

    def test_atomic_publication_rolls_back(self) -> None:
        path = self._compact_exact("f00")
        original = owner_campaign._atomic_bytes
        calls = {"count": 0}

        def fail_once(target: Path, payload: bytes) -> None:
            calls["count"] += 1
            if calls["count"] == 2:
                raise OSError("injected publication failure")
            original(target, payload)

        with mock.patch.object(owner_campaign, "_atomic_bytes", fail_once):
            with self.assertRaises(OSError):
                owner_campaign_import.import_legacy(self.root, self.manifest_path, [path])
        owner_root = owner_campaign._owner_root(self.root, self.campaign)
        self.assertFalse((owner_root / "exact-manifest.json").exists())
        self.assertFalse((owner_campaign._state_root(self.root) / "proof-cas" / "reports").exists())


if __name__ == "__main__":
    unittest.main()
