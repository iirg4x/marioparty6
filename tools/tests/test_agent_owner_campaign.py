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
from tools import owner_campaign_reconstruction as reconstruction


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

    def test_propose_cli_forwards_selection_terminal_and_counts(self) -> None:
        candidate = self.root / "build" / "candidate.c"
        candidate.write_text("int focus(void) { return 1; }\n", encoding="utf-8")
        expected = {"schema": "owner_campaign_proposal/v1", "status": "queued"}
        with patch.object(
            owner_campaign,
            "load_campaign",
            return_value={"functions": ["focus"]},
        ), patch(
            "tools.owner_campaign_lane.propose_candidate",
            return_value=expected,
        ) as propose:
            code, result, _ = self._run_agent(
                "crack",
                "propose",
                "--campaign",
                "build/campaign.json",
                "--function",
                "focus",
                "--candidate-source",
                "build/candidate.c",
                "--hypothesis-family",
                "direct-owner",
                "--expected-terminal",
                "improved",
                "--predicted-row",
                "strict:focus:row:1",
                "--predicted-remaining-strict",
                "3",
                "--predicted-remaining-data",
                "4",
                "--predicted-remaining-physical",
                "5",
            )
        self.assertEqual(code, 0)
        self.assertEqual(result, expected)
        self.assertEqual(propose.call_args.kwargs["expected_terminal"], "improved")
        self.assertEqual(
            propose.call_args.kwargs["predicted_rows"], ["strict:focus:row:1"]
        )
        self.assertEqual(
            propose.call_args.kwargs["predicted_remaining_counts"],
            {"strict": 3, "data": 4, "physical": 5},
        )

    def test_snapshot_dispatches_loaded_campaign_and_prints_compact_binding(self) -> None:
        frontier = self._snapshot_frontier()
        with patch.object(
            owner_campaign, "snapshot_frontier", return_value=frontier
        ) as snapshotter:
            code, result, _ = self._run_agent(
                "owner-campaign",
                "snapshot",
                "--campaign",
                "build/campaign.json",
                "--function",
                "focus",
            )

        self.assertEqual(code, 0)
        self.assertEqual(result["schema"], "owner_campaign_snapshot/v1")
        self.assertEqual(result["status"], "snapshot")
        self.assertEqual(result["function"], "focus")
        self.assertEqual(result["frontier_sha256"], frontier["frontier_sha256"])
        self.assertEqual(
            result["focus_evidence_sha256"], frontier["focus_evidence_sha256"]
        )
        self.assertEqual(result["authority_advanced"], False)
        self.assertEqual(snapshotter.call_count, 1)
        self.assertEqual(snapshotter.call_args.args[0], self.root)
        self.assertEqual(snapshotter.call_args.args[2], "focus")

    def test_snapshot_cli_is_idempotent_for_the_same_frontier(self) -> None:
        frontier = self._snapshot_frontier()
        argv = (
            "owner-campaign",
            "snapshot",
            "--campaign",
            "build/campaign.json",
            "--function",
            "focus",
        )
        with patch.object(
            owner_campaign, "snapshot_frontier", return_value=frontier
        ) as snapshotter:
            first_code, first, _ = self._run_agent(*argv)
            second_code, second, _ = self._run_agent(*argv)

        self.assertEqual(first_code, 0)
        self.assertEqual(second_code, 0)
        self.assertEqual(first, second)
        self.assertEqual(snapshotter.call_count, 2)
        self.assertEqual(
            [entry.args[2] for entry in snapshotter.call_args_list],
            ["focus", "focus"],
        )

    def test_snapshot_rejects_function_outside_campaign_scope(self) -> None:
        code, result, text = self._run_agent(
            "owner-campaign",
            "snapshot",
            "--campaign",
            "build/campaign.json",
            "--function",
            "outside",
        )

        self.assertEqual(code, 2)
        self.assertEqual(result, text.strip())
        self.assertIn("function is outside campaign scope: outside", text)

    def test_snapshot_never_enters_legacy_compile_or_control_paths(self) -> None:
        frontier = self._snapshot_frontier()
        with patch.object(
            owner_campaign, "snapshot_frontier", return_value=frontier
        ), patch.object(
            agent,
            "run_crack_command",
            side_effect=AssertionError("legacy crack harness entered"),
        ), patch.object(
            owner_campaign,
            "run_candidate",
            side_effect=AssertionError("candidate compile entered"),
        ):
            code, result, _ = self._run_agent(
                "owner-campaign",
                "snapshot",
                "--campaign",
                "build/campaign.json",
                "--function",
                "focus",
            )

        self.assertEqual(code, 0)
        self.assertEqual(result["status"], "snapshot")

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

    def _snapshot_frontier(self) -> dict[str, object]:
        return {
            "campaign_id": "cli-owner-v1",
            "manifest_sha256": self._manifest_sha256(),
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "function": "focus",
            "source_relpath": "src/test.c",
            "source_sha256": _digest(self.source.read_bytes()),
            "target_object_sha256": _digest(self.target.read_bytes()),
            "toolchain_sha256": _digest(self.toolchain.read_bytes()),
            "candidate_object_sha256": "c" * 64,
            "frontier_sha256": "f" * 64,
            "focus_evidence_sha256": "e" * 64,
            "parent_frontier_sha256": None,
            "generation": 0,
        }

    def _write_reconstruction_packet(
        self,
        frontier: dict[str, object],
        *,
        status: str = "READY",
        exact_terminal_possible: bool = True,
    ) -> dict[str, object]:
        target_row = {
            "index": 1,
            "instruction": {
                "address": "0x1004",
                "formatted": "lwz r3,0x10(r1)",
                "size": 4,
                "parts": [{"opcode": "lwz"}],
            },
        }
        candidate_row = {
            "index": 1,
            "instruction": {
                "address": "0x1004",
                "formatted": "lwz r4,0x14(r1)",
                "size": 4,
                "parts": [{"opcode": "lwz"}],
            },
        }
        physical = status == "UNKNOWN" or not exact_terminal_possible
        report: dict[str, object] = {
            "schema": "focus_symbol_report/v1",
            "owner": frontier["owner"],
            "unit": frontier["unit"],
            "function": frontier["function"],
            "source_path": frontier["source_relpath"],
            "base_commit": self.commit,
            "source_sha256": frontier["source_sha256"],
            "target_object_sha256": frontier["target_object_sha256"],
            "candidate_object_sha256": frontier["candidate_object_sha256"],
            "toolchain_sha256": frontier["toolchain_sha256"],
            "strict_row_ids": ["strict:focus:row:1:"],
            "data_row_ids": [],
            "channels": {
                "strict": {
                    "metric": {"target_size": 4, "candidate_size": 4, "diff_rows": 1},
                    "target": {"instruction_count": 1, "rows": [target_row]},
                    "candidate": {"instruction_count": 1, "rows": [candidate_row]},
                },
                "data": {
                    "metric": {"target_size": 4, "candidate_size": 4, "diff_rows": 0},
                    "target": {"instruction_count": 1, "rows": []},
                    "candidate": {"instruction_count": 1, "rows": []},
                },
            },
            "physical_relocations": {
                "status": "mismatch" if physical else "exact",
                "target": {"physical_relocation_count": 0, "physical_relocations": []},
                "candidate": {"physical_relocation_count": 0, "physical_relocations": []},
                "physical_relocation_differences": (
                    [{"offset": 4, "target": ["helper"], "candidate": ["other"]}]
                    if physical else []
                ),
            },
        }
        report["artifact_sha256"] = reconstruction.canonical_sha256(report)
        packet = reconstruction.build_packet(
            report,
            {
                "owner": frontier["owner"],
                "unit": frontier["unit"],
                "function": frontier["function"],
                "source_path": frontier["source_relpath"],
                "source_sha256": frontier["source_sha256"],
                "base_commit": self.commit,
                "target_object_sha256": frontier["target_object_sha256"],
                "candidate_object_sha256": frontier["candidate_object_sha256"],
                "toolchain_sha256": frontier["toolchain_sha256"],
                "frontier_source_sha256": frontier["source_sha256"],
            },
            {"function": frontier["function"], "start_line": 1, "end_line": 1},
        )
        path = (
            owner_campaign._state_root(self.root) / "proof-cas" / "reconstruction"
            / packet["packet_sha256"][:2] / f"{packet['packet_sha256']}.json"
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(packet, ensure_ascii=False, sort_keys=True, separators=(",", ":")),
            encoding="utf-8",
        )
        return packet

    def test_reconstruct_command_returns_verified_packet_summary(self) -> None:
        frontier = self._snapshot_frontier()
        packet = self._write_reconstruction_packet(frontier)
        frontier["reconstruction_evidence_sha256"] = packet["packet_sha256"]
        frontier["reconstruction_status"] = packet["status"]
        frontier_body = dict(frontier)
        frontier_body.pop("frontier_sha256", None)
        frontier["frontier_sha256"] = owner_campaign._digest_json(frontier_body)
        with patch.object(
            owner_campaign, "snapshot_frontier", return_value=frontier
        ) as snapshotter:
            code, result, _ = self._run_agent(
                "owner-campaign",
                "reconstruct",
                "--campaign",
                "build/campaign.json",
                "--function",
                "focus",
            )
        self.assertEqual(code, 0)
        self.assertEqual(result["schema"], "owner_campaign_reconstruction_result/v1")
        self.assertEqual(result["status"], "READY")
        self.assertEqual(result["packet_sha256"], packet["packet_sha256"])
        self.assertEqual(result["next_action"], "CRACK")
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(snapshotter.call_count, 1)

        with patch.object(
            owner_campaign, "snapshot_frontier", return_value=frontier
        ):
            alias_code, alias_result, _ = self._run_agent(
                "crack",
                "context",
                "--campaign",
                "build/campaign.json",
                "--function",
                "focus",
            )
        self.assertEqual(alias_code, 0)
        self.assertEqual(alias_result, result)


if __name__ == "__main__":
    unittest.main()
