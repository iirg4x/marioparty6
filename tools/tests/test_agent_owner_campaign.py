from __future__ import annotations

from contextlib import redirect_stdout
import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from tools import agent
from tools import owner_campaign


def _digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _seal(body: dict[str, object], field: str) -> dict[str, object]:
    value = dict(body)
    value[field] = _digest(
        json.dumps(
            body,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    )
    return value


def _init_fixture_git(root: Path) -> None:
    """Create a byte-stable fixture repository independent of user Git config."""

    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    subprocess.run(
        ["git", "config", "core.autocrlf", "false"],
        cwd=root,
        check=True,
    )


class AgentOwnerCampaignCLITests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        (self.root / "src").mkdir()
        (self.root / "build" / "evidence").mkdir(parents=True)
        self.source = self.root / "src" / "test.c"
        self.source.write_text(
            "int focus(void) { return 0; }\n",
            encoding="utf-8",
        )
        self.target = self.root / "build" / "evidence" / "target.o"
        self.target.write_bytes(b"target")
        self.toolchain = self.root / "build" / "evidence" / "toolchain.json"
        self.toolchain.write_bytes(b"{}\n")
        self.producer = self.root / "build" / "evidence" / "producer.py"
        self.producer.write_bytes(b"# campaign measurement fixture\n")

        _init_fixture_git(self.root)
        subprocess.run(
            ["git", "config", "user.email", "test@example.invalid"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Owner Campaign CLI Test"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(
            ["git", "add", "src/test.c"], cwd=self.root, check=True
        )
        subprocess.run(
            ["git", "commit", "-qm", "campaign cli fixture"],
            cwd=self.root,
            check=True,
        )
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=self.root,
            text=True,
        ).strip()

        body: dict[str, object] = {
            "schema": "owner_campaign/v1",
            "campaign_id": "cli-owner-v1",
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "source_relpath": "src/test.c",
            "base_commit": self.commit,
            "target_object": {
                "path": "build/evidence/target.o",
                "sha256": _digest(b"target"),
            },
            "toolchain": {
                "path": "build/evidence/toolchain.json",
                "sha256": _digest(b"{}\n"),
            },
            "measurement_producer": {
                "path": "build/evidence/producer.py",
                "sha256": _digest(b"# campaign measurement fixture\n"),
            },
            "functions": ["focus"],
            "protected_exact_functions": [],
            "allowed_source_paths": ["src/test.c"],
            "allowed_build_paths": ["build"],
            "forbidden_constructs": [
                r"\b(?:asm|volatile|register)\b",
                r"#\s*pragma",
            ],
            "commands": {
                "snapshot": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}"],
                    "measurement_relpath": "build/evidence/snapshot.json",
                },
                "candidate": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}"],
                    "measurement_relpath": "build/evidence/candidate.json",
                },
                "final_owner": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}"],
                    "measurement_relpath": "build/evidence/final-owner.json",
                },
            },
            "cancellation_epoch": 0,
            "limits": {
                "command_timeout_seconds": 20,
                "scratch_soft_bytes": 32 << 20,
                "scratch_hard_bytes": 64 << 20,
                "cell_temporary_bytes": 1 << 20,
                "focus_evidence_bytes": 256 << 10,
                "frontier_bytes": 64 << 10,
                "report_bytes": 64 << 10,
                "dedupe_bytes": 1 << 20,
                "owner_state_bytes": 16 << 20,
            },
        }
        self.manifest_path = self.root / "build" / "campaign.json"
        self.manifest_path.write_text(
            json.dumps(_seal(body, "manifest_sha256")),
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _run_agent(self, *argv: str) -> tuple[int, object, str]:
        previous = sys.argv
        sys.argv = [
            "agent.py",
            "--root",
            str(self.root),
            *argv,
        ]
        output = io.StringIO()
        try:
            with redirect_stdout(output):
                result = agent.main()
        finally:
            sys.argv = previous
        text = output.getvalue()
        try:
            payload: object = json.loads(text)
        except json.JSONDecodeError:
            payload = text.strip()
        return result, payload, text

    def test_status_and_run_dispatch_against_valid_manifest(self) -> None:
        status_code, status, _ = self._run_agent(
            "owner-campaign",
            "status",
            "--campaign",
            "build/campaign.json",
        )
        self.assertEqual(status_code, 0)
        self.assertEqual(status["schema"], "owner_campaign_status/v1")
        self.assertEqual(status["campaign_id"], "cli-owner-v1")
        self.assertEqual((status["exact_count"], status["total"]), (0, 1))

        idle_code, idle, _ = self._run_agent(
            "owner-campaign",
            "run",
            "--campaign",
            "build/campaign.json",
            "--once",
        )
        self.assertEqual(idle_code, 0)
        self.assertEqual(idle["status"], "idle")
        self.assertEqual(idle["dispatched"], 0)

        with patch.object(owner_campaign, "run_loop", return_value=[]):
            run_code, run, _ = self._run_agent(
                "owner-campaign",
                "run",
                "--campaign",
                "build/campaign.json",
                "--candidate",
                "build/candidates/explicit.json",
            )
        self.assertEqual(run_code, 0)
        self.assertEqual(run, [])

    def test_crack_loop_alias_dispatches_to_owner_loop(self) -> None:
        with patch.object(owner_campaign, "run_loop", return_value=[]):
            code, result, _ = self._run_agent(
                "crack",
                "loop",
                "--campaign",
                "build/campaign.json",
                "--candidate",
                "build/candidates/explicit.json",
            )
        self.assertEqual(code, 0)
        self.assertEqual(result, [])

    def test_initialize_validates_existing_manifest(self) -> None:
        code, result, _ = self._run_agent(
            "owner-campaign",
            "initialize",
            "--campaign",
            "build/campaign.json",
        )
        self.assertEqual(code, 0)
        self.assertEqual(result["schema"], "owner_campaign_init/v1")
        self.assertEqual(result["status"], "initialized")
        self.assertEqual(result["manifest_sha256"], self._manifest_sha256())

    def test_v2_commands_do_not_enter_legacy_or_control_paths(self) -> None:
        forbidden = {
            "stop",
            "permit.json",
            "hmac.key",
            "approval.json",
        }
        control_root = self.root / "build" / "owner-campaign"
        control_root.mkdir(parents=True)
        for name in forbidden:
            (control_root / name).write_text("sentinel", encoding="utf-8")

        original_open = Path.open

        def guarded_open(path: Path, *args: object, **kwargs: object):
            if path.name.lower() in forbidden:
                raise AssertionError(f"legacy control path opened: {path}")
            return original_open(path, *args, **kwargs)

        hmac_loaded = "hmac" in sys.modules
        with patch.object(
            agent,
            "run_crack_command",
            side_effect=AssertionError("legacy crack harness entered"),
        ), patch.object(Path, "open", guarded_open), patch.object(
            owner_campaign, "run_loop", return_value=[]
        ):
            for command in (
                ("owner-campaign", "initialize"),
                ("owner-campaign", "status"),
                ("owner-campaign", "run"),
                ("crack", "loop"),
            ):
                extra = (
                    ("--candidate", "build/candidates/explicit.json")
                    if command[-1] == "run" or command == ("crack", "loop")
                    else ()
                )
                code, _, _ = self._run_agent(
                    *command,
                    "--campaign",
                    "build/campaign.json",
                    *extra,
                )
                self.assertEqual(code, 0, command)

        self.assertEqual("hmac" in sys.modules, hmac_loaded)
        self.assertNotIn("tools.owner_campaign.hmac", sys.modules)

    def _manifest_sha256(self) -> str:
        value = json.loads(self.manifest_path.read_text(encoding="utf-8"))
        return value["manifest_sha256"]


if __name__ == "__main__":
    unittest.main()
