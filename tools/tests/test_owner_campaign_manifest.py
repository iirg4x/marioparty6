from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

from tools import owner_campaign
from tools import owner_campaign_manifest as manifest


def _init_fixture_git(root: Path) -> None:
    """Create a byte-stable fixture repository independent of user Git config."""

    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    subprocess.run(
        ["git", "config", "core.autocrlf", "false"],
        cwd=root,
        check=True,
    )


class OwnerCampaignManifestTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        (self.root / "src").mkdir()
        (self.root / "build").mkdir()
        (self.root / "src" / "test.c").write_text(
            "int focus(void) { return 0; }\n", encoding="utf-8"
        )
        (self.root / "build" / "target.o").write_bytes(b"target")
        (self.root / "build" / "toolchain.json").write_bytes(b"toolchain\n")
        (self.root / "build" / "producer.py").write_text(
            "# test measurement producer\n", encoding="utf-8"
        )
        _init_fixture_git(self.root)
        subprocess.run(
            ["git", "config", "user.email", "test@example.invalid"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Owner Campaign Manifest Test"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(["git", "add", "src/test.c"], cwd=self.root, check=True)
        subprocess.run(
            ["git", "commit", "-qm", "manifest fixture"], cwd=self.root, check=True
        )
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _direct(self, **overrides: object) -> dict[str, object]:
        values: dict[str, object] = {
            "root": self.root,
            "output": Path("build/campaign.json"),
            "campaign_id": "manifest-test-v1",
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "source_relpath": "src/test.c",
            "base_commit": self.commit,
            "target_object": Path("build/target.o"),
            "toolchain": Path("build/toolchain.json"),
            "measurement_producer": Path("build/producer.py"),
            "functions": ["focus"],
            "final_owner_command": [sys.executable, "{MEASUREMENT_PRODUCER}"],
        }
        values.update(overrides)
        return values

    def test_direct_initialization_writes_and_core_validates(self) -> None:
        result = manifest.initialize_campaign(**self._direct())
        self.assertEqual(result["status"], "initialized")
        self.assertEqual(result["manifest_path"], "build/campaign.json")
        self.assertRegex(result["manifest_sha256"], re.compile(r"^[0-9a-f]{64}$"))
        loaded = owner_campaign.load_campaign(
            self.root, self.root / "build" / "campaign.json"
        )
        self.assertEqual(loaded["manifest_sha256"], result["manifest_sha256"])
        raw = json.loads((self.root / "build" / "campaign.json").read_text())
        self.assertEqual(
            raw["target_object"],
            {"path": "build/target.o", "sha256": hashlib.sha256(b"target").hexdigest()},
        )
        self.assertEqual(
            raw["toolchain"],
            {"path": "build/toolchain.json", "sha256": hashlib.sha256(b"toolchain\n").hexdigest()},
        )
        self.assertEqual(
            raw["measurement_producer"],
            {
                "path": "build/producer.py",
                "sha256": hashlib.sha256(
                    (self.root / "build" / "producer.py").read_bytes()
                ).hexdigest(),
            },
        )
        producer_sha256 = raw["measurement_producer"]["sha256"]
        producer_cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / producer_sha256 / "owner_campaign_measure.py"
        )
        self.assertTrue(producer_cas.is_file())
        self.assertEqual(producer_cas.read_bytes(), (self.root / "build" / "producer.py").read_bytes())
        self.assertEqual(
            hashlib.sha256(producer_cas.read_bytes()).hexdigest(), producer_sha256
        )
        self.assertEqual(raw["commands"]["snapshot"]["argv"][1], "{MEASUREMENT_PRODUCER}")
        self.assertEqual(raw["commands"]["candidate"]["argv"][1], "{MEASUREMENT_PRODUCER}")
        self.assertEqual(raw["limits"]["focus_evidence_bytes"], 256 << 10)
        self.assertEqual(raw["limits"]["frontier_bytes"], 64 << 10)
        self.assertEqual(raw["limits"]["report_bytes"], 64 << 10)
        self.assertEqual(raw["limits"]["owner_state_bytes"], 16 << 20)

    def test_existing_manifest_is_idempotently_loaded(self) -> None:
        first = manifest.initialize_campaign(**self._direct())
        second = manifest.initialize_campaign(
            root=self.root, output=Path("build/campaign.json")
        )
        self.assertEqual(first["manifest_sha256"], second["manifest_sha256"])

    def test_agent_cli_initializes_direct_fields(self) -> None:
        script = Path(__file__).parents[1] / "agent.py"
        command = [
            sys.executable,
            str(script),
            "--root",
            str(self.root),
            "owner-campaign",
            "initialize",
            "--campaign",
            "build/cli-campaign.json",
            "--campaign-id",
            "manifest-cli-v1",
            "--owner",
            "main:test/owner",
            "--unit",
            "main/test/owner",
            "--source",
            "src/test.c",
            "--base-commit",
            self.commit,
            "--target-object",
            "build/target.o",
            "--toolchain",
            "build/toolchain.json",
            "--measurement-producer",
            "build/producer.py",
            "--function",
            "focus",
            "--final-owner-command",
            sys.executable,
            "--final-owner-command",
            "{MEASUREMENT_PRODUCER}",
        ]
        completed = subprocess.run(
            command,
            cwd=Path(__file__).parents[2],
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr + completed.stdout)
        result = json.loads(completed.stdout)
        self.assertEqual(result["status"], "initialized")
        owner_campaign.load_campaign(
            self.root, self.root / "build" / "cli-campaign.json"
        )

    def test_draft_hash_drift_is_rejected_before_output(self) -> None:
        draft = self.root / "build" / "draft.json"
        values = self._direct(output=None)
        values.pop("root")
        values.pop("output")
        values["target_object"] = {
            "path": "build/target.o",
            "sha256": "0" * 64,
        }
        values["toolchain"] = {
            "path": "build/toolchain.json",
            "sha256": hashlib.sha256(b"toolchain\n").hexdigest(),
        }
        values["measurement_producer"] = {
            "path": "build/producer.py",
            "sha256": hashlib.sha256(
                (self.root / "build" / "producer.py").read_bytes()
            ).hexdigest(),
        }
        values.pop("final_owner_command")
        draft.write_text(json.dumps(values), encoding="utf-8")
        with self.assertRaisesRegex(manifest.ManifestError, "target object hash drift"):
            manifest.initialize_campaign(
                root=self.root,
                draft=Path("build/draft.json"),
                output=Path("build/campaign.json"),
            )
        self.assertFalse((self.root / "build" / "campaign.json").exists())

    def test_measurement_producer_drift_is_rejected_before_cas_snapshot(self) -> None:
        producer = self.root / "build" / "producer.py"
        expected = hashlib.sha256(producer.read_bytes()).hexdigest()
        draft = self.root / "build" / "producer-draft.json"
        values = self._direct(output=None)
        values.pop("root")
        values.pop("output")
        values["target_object"] = {
            "path": "build/target.o",
            "sha256": hashlib.sha256(b"target").hexdigest(),
        }
        values["toolchain"] = {
            "path": "build/toolchain.json",
            "sha256": hashlib.sha256(b"toolchain\n").hexdigest(),
        }
        values["measurement_producer"] = {
            "path": "build/producer.py", "sha256": expected
        }
        values.pop("final_owner_command")
        draft.write_text(json.dumps(values), encoding="utf-8")
        producer.write_bytes(producer.read_bytes() + b"# drift\n")

        with self.assertRaisesRegex(manifest.ManifestError, "measurement producer hash drift"):
            manifest.initialize_campaign(
                root=self.root,
                draft=Path("build/producer-draft.json"),
                output=Path("build/campaign.json"),
                final_owner_command=[sys.executable, "{MEASUREMENT_PRODUCER}"],
            )
        producer_cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        self.assertFalse(producer_cas.exists())

    def test_wrong_existing_measurement_producer_cas_is_rejected(self) -> None:
        producer = self.root / "build" / "producer.py"
        expected = hashlib.sha256(producer.read_bytes()).hexdigest()
        producer_cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        producer_cas.parent.mkdir(parents=True, exist_ok=True)
        producer_cas.write_bytes(b"wrong producer snapshot")

        with self.assertRaisesRegex(manifest.ManifestError, "measurement producer CAS hash drift"):
            manifest.initialize_campaign(**self._direct())
        self.assertFalse((self.root / "build" / "campaign.json").exists())

    def test_direct_external_path_is_rejected(self) -> None:
        outside = Path(self.temporary.name).parent / "outside-target.o"
        outside.write_bytes(b"outside")
        with self.assertRaisesRegex(manifest.ManifestError, "escapes the repository"):
            manifest.initialize_campaign(
                **self._direct(target_object=outside)
            )

    def test_final_owner_command_is_required(self) -> None:
        values = self._direct()
        values.pop("final_owner_command")
        with self.assertRaisesRegex(manifest.ManifestError, "final_owner command is required"):
            manifest.initialize_campaign(**values)


if __name__ == "__main__":
    unittest.main()
