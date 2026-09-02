from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

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

    def test_agent_cli_snapshots_external_release_inputs(self) -> None:
        script = Path(__file__).parents[1] / "agent.py"
        with tempfile.TemporaryDirectory() as external_raw:
            external = Path(external_raw).resolve()
            toolchain = external / "toolchain.json"
            producer = external / "owner_campaign_measure.py"
            toolchain.write_bytes(b"external toolchain\n")
            producer.write_bytes(b"# external producer\n")
            command = [
                sys.executable,
                str(script),
                "--root",
                str(self.root),
                "owner-campaign",
                "initialize",
                "--campaign",
                "build/external-cli-campaign.json",
                "--campaign-id",
                "external-cli-v1",
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
                str(toolchain),
                "--measurement-producer",
                str(producer),
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
        loaded = owner_campaign.load_campaign(
            self.root, self.root / "build" / "external-cli-campaign.json"
        )
        self.assertTrue(str(loaded["toolchain"]["path"]).startswith(
            "build/owner-campaign/tool-cas/"
        ))
        self.assertTrue(str(loaded["measurement_producer"]["path"]).startswith(
            "build/owner-campaign/tool-cas/"
        ))

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

    def test_external_tools_are_snapshotted_into_campaign_cas(self) -> None:
        with tempfile.TemporaryDirectory() as external_raw:
            external = Path(external_raw).resolve()
            toolchain = external / "central-toolchain.json"
            producer = external / "released-owner-campaign-measure.py"
            toolchain.write_bytes(b"central toolchain\n")
            producer.write_bytes(b"# released producer\n")

            result = manifest.initialize_campaign(
                **self._direct(
                    toolchain=toolchain,
                    measurement_producer=producer,
                )
            )

        self.assertEqual(result["status"], "initialized")
        raw = json.loads((self.root / "build" / "campaign.json").read_text())
        toolchain_sha = hashlib.sha256(b"central toolchain\n").hexdigest()
        producer_sha = hashlib.sha256(b"# released producer\n").hexdigest()
        self.assertEqual(
            raw["toolchain"],
            {
                "path": (
                    "build/owner-campaign/tool-cas/"
                    f"{toolchain_sha}/toolchain.json"
                ),
                "sha256": toolchain_sha,
            },
        )
        self.assertEqual(
            raw["measurement_producer"],
            {
                "path": (
                    "build/owner-campaign/tool-cas/"
                    f"{producer_sha}/owner_campaign_measure.py"
                ),
                "sha256": producer_sha,
            },
        )
        self.assertEqual(
            (self.root / raw["toolchain"]["path"]).read_bytes(),
            b"central toolchain\n",
        )
        self.assertEqual(
            (self.root / raw["measurement_producer"]["path"]).read_bytes(),
            b"# released producer\n",
        )
        owner_campaign.load_campaign(
            self.root, self.root / "build" / "campaign.json"
        )

    def test_external_toolchain_wrong_existing_cas_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as external_raw:
            toolchain = Path(external_raw).resolve() / "toolchain.json"
            toolchain.write_bytes(b"central toolchain\n")
            expected = hashlib.sha256(toolchain.read_bytes()).hexdigest()
            cas = (
                self.root / "build" / "owner-campaign" / "tool-cas"
                / expected / "toolchain.json"
            )
            cas.parent.mkdir(parents=True, exist_ok=True)
            cas.write_bytes(b"wrong toolchain snapshot")

            with self.assertRaisesRegex(manifest.ManifestError, "toolchain CAS hash drift"):
                manifest.initialize_campaign(
                    **self._direct(toolchain=toolchain)
                )
        self.assertFalse((self.root / "build" / "campaign.json").exists())

    def test_final_owner_command_is_required(self) -> None:
        values = self._direct()
        values.pop("final_owner_command")
        with self.assertRaisesRegex(manifest.ManifestError, "final_owner command is required"):
            manifest.initialize_campaign(**values)

    def test_dirty_tracked_context_is_snapshotted_and_materialized(self) -> None:
        tools = self.root / "tools"
        tools.mkdir()
        context = tools / "lane_tool.py"
        context.write_text("BASE = 1\n", encoding="utf-8")
        subprocess.run(["git", "add", "tools/lane_tool.py"], cwd=self.root, check=True)
        subprocess.run(
            ["git", "commit", "-qm", "add lane context"], cwd=self.root, check=True
        )
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()
        source = self.root / "src" / "test.c"
        source.write_text("int focus(void) { return 1; }\n", encoding="utf-8")
        context.write_text("DIRTY = 2\n", encoding="utf-8")

        manifest.initialize_campaign(**self._direct())
        raw = json.loads((self.root / "build" / "campaign.json").read_text())
        self.assertEqual(
            [item["path"] for item in raw["tracked_context"]],
            ["src/test.c", "tools/lane_tool.py"],
        )
        loaded = owner_campaign.load_campaign(
            self.root, self.root / "build" / "campaign.json"
        )
        scratch = owner_campaign._ensure_scratch(self.root, loaded)
        self.assertEqual(
            (scratch / "tools" / "lane_tool.py").read_text(encoding="utf-8"),
            "DIRTY = 2\n",
        )
        self.assertEqual(
            (scratch / "src" / "test.c").read_text(encoding="utf-8"),
            "int focus(void) { return 0; }\n",
        )

        # Independent lane work may continue on the already-bound context;
        # the campaign keeps using its immutable CAS snapshot.
        context.write_text("DIRTY = 3\n", encoding="utf-8")
        reloaded = owner_campaign.load_campaign(
            self.root, self.root / "build" / "campaign.json"
        )
        scratch = owner_campaign._ensure_scratch(self.root, reloaded)
        self.assertEqual(
            (scratch / "tools" / "lane_tool.py").read_text(encoding="utf-8"),
            "DIRTY = 2\n",
        )
        (scratch / "tools" / "lane_tool.py").write_text(
            "scratch drift\n", encoding="utf-8"
        )
        with self.assertRaisesRegex(
            owner_campaign.InfrastructureError, "scratch tracked context hash drift"
        ):
            owner_campaign._verify_hook_inputs(reloaded, scratch)
        scratch = owner_campaign._ensure_scratch(self.root, reloaded)
        self.assertEqual(
            (scratch / "tools" / "lane_tool.py").read_text(encoding="utf-8"),
            "DIRTY = 2\n",
        )

        source.write_text("int focus(void) { return 2; }\n", encoding="utf-8")
        with self.assertRaisesRegex(owner_campaign.CampaignError, "retained frontier"):
            owner_campaign.load_campaign(
                self.root, self.root / "build" / "campaign.json"
            )

    def test_new_unbound_tracked_write_is_rejected_after_initialization(self) -> None:
        manifest.initialize_campaign(**self._direct())
        added = self.root / "new_tracked.txt"
        added.write_text("new\n", encoding="utf-8")
        subprocess.run(["git", "add", "new_tracked.txt"], cwd=self.root, check=True)
        with self.assertRaisesRegex(
            owner_campaign.CampaignError, "unapproved tracked writes"
        ):
            owner_campaign.load_campaign(
                self.root, self.root / "build" / "campaign.json"
            )

    def test_tracked_context_cas_drift_is_rejected(self) -> None:
        source = self.root / "src" / "test.c"
        source.write_text("int focus(void) { return 1; }\n", encoding="utf-8")
        manifest.initialize_campaign(**self._direct())
        raw = json.loads((self.root / "build" / "campaign.json").read_text())
        descriptor = raw["tracked_context"][0]
        cas = owner_campaign._tracked_context_cas_path(
            self.root, descriptor["sha256"]
        )
        cas.write_bytes(b"drift")
        with self.assertRaisesRegex(owner_campaign.CampaignError, "CAS hash drift"):
            owner_campaign.load_campaign(
                self.root, self.root / "build" / "campaign.json"
            )

    def test_tracked_context_count_is_bounded(self) -> None:
        first = self.root / "first.txt"
        second = self.root / "second.txt"
        first.write_text("base\n", encoding="utf-8")
        second.write_text("base\n", encoding="utf-8")
        subprocess.run(["git", "add", "first.txt", "second.txt"], cwd=self.root, check=True)
        subprocess.run(
            ["git", "commit", "-qm", "add context fixtures"], cwd=self.root, check=True
        )
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()
        first.write_text("one\n", encoding="utf-8")
        second.write_text("two\n", encoding="utf-8")
        with mock.patch.object(owner_campaign, "MAX_TRACKED_CONTEXT_FILES", 1):
            with self.assertRaisesRegex(manifest.ManifestError, "file-count"):
                manifest.initialize_campaign(**self._direct())


if __name__ == "__main__":
    unittest.main()
