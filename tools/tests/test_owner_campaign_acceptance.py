from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest import mock

from tools import owner_campaign as campaign
from tools.tests.test_owner_campaign import digest_bytes, seal


# This hook is deliberately a tiny black-box compiler/proof stand-in.  It emits
# the public measurement contract and can synchronize candidate processes to
# make races deterministic without reaching into the runtime implementation.
ACCEPTANCE_HOOK = r'''from __future__ import annotations
import hashlib, json, os, pathlib, sys, time

source = pathlib.Path(sys.argv[1])
output = pathlib.Path(sys.argv[2])
log = pathlib.Path(sys.argv[3])
text = source.read_text(encoding="utf-8")
root = log.parent.parent
build = root / "build"
phase = os.environ["OWNER_CAMPAIGN_PHASE"]

def wait_for(prefix: str, count: int) -> None:
    ready = build / (prefix + ".ready." + str(os.getpid()))
    ready.write_text("ready", encoding="utf-8")
    deadline = time.monotonic() + 6.0
    while len(list(build.glob(prefix + ".ready.*"))) < count:
        if time.monotonic() >= deadline:
            raise SystemExit("barrier timed out: " + prefix)
        time.sleep(0.01)

with log.open("a", encoding="utf-8") as stream:
    stream.write(phase + ":" + hashlib.sha256(source.read_bytes()).hexdigest() + "\n")

if phase == "candidate" and "FIVE" in text:
    wait_for("owner-campaign-five", 5)
if phase == "candidate" and "CAS" in text:
    wait_for("owner-campaign-cas", 2)
if phase == "candidate" and "DUP" in text:
    time.sleep(0.45)

if "EXACT" in text:
    diff, data_diff, physical, size, linked = 0, 0, 0, 100, True
elif "IMPROVE" in text:
    diff, data_diff, physical, size, linked = 6, 7, 1, 100, False
elif "LOSS" in text:
    diff, data_diff, physical, size, linked = 3, 3, 0, 100, False
elif "REGRESS" in text:
    diff, data_diff, physical, size, linked = 11, 11, 3, 100, False
else:
    diff, data_diff, physical, size, linked = 10, 10, 2, 96, False

source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
receipts = {
    name: hashlib.sha256((name + source_sha).encode()).hexdigest()
    for name in ("strict", "data", "physical", "siblings", "source_link")
}
focus_body = {
    "schema": "owner_campaign_focus_evidence/v1",
    "owner": os.environ["OWNER_CAMPAIGN_OWNER"],
    "function": os.environ["OWNER_CAMPAIGN_FUNCTION"],
    "unit": os.environ["OWNER_CAMPAIGN_UNIT"],
    "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"],
    "base_commit": os.environ["OWNER_CAMPAIGN_BASE_COMMIT"],
    "source_sha256": os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"],
    "target_object_sha256": os.environ["OWNER_CAMPAIGN_TARGET_SHA256"],
    "strict_rows": [],
    "data_rows": [],
    "physical_differences": [],
    "strict_row_ids": [f"strict:{index}" for index in range(diff)],
    "data_row_ids": [f"data:{index}" for index in range(data_diff)],
    "physical_difference_ids": [f"physical:{index}" for index in range(physical)],
    "physical_target_identity_sha256": hashlib.sha256(b"physical-target").hexdigest(),
    "physical_candidate_identity_sha256": hashlib.sha256(b"physical-target").hexdigest() if physical == 0 else hashlib.sha256(("physical-candidate:" + str(physical)).encode()).hexdigest(),
    "strict_row_count": diff, "data_row_count": data_diff,
    "physical_target_count": 5, "physical_candidate_count": 5,
    "physical_difference_count": physical,
    "protected_total": int(os.environ["OWNER_CAMPAIGN_PROTECTED_TOTAL"]),
    "protected_losses": 1 if "LOSS" in text else 0,
    "sibling_identities": ["sibling", "third"],
    "sibling_digest": hashlib.sha256(json.dumps(["sibling", "third"], sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
}
focus_body["strict_row_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["strict_row_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_body["data_row_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["data_row_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_body["physical_difference_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["physical_difference_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_evidence = dict(focus_body)
focus_evidence["focus_evidence_sha256"] = hashlib.sha256(
    json.dumps(focus_body, sort_keys=True, separators=(",", ":")).encode()
).hexdigest()
receipts["focus"] = focus_evidence["focus_evidence_sha256"]
candidate_object_sha = hashlib.sha256(("object:" + source_sha).encode()).hexdigest()
if linked:
    command = "mwcc source.c -o candidate.o"
    source_link = {"schema": "owner_campaign_source_link_proof/v1", "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"], "source_sha256": source_sha, "candidate_object_path": "candidate.o", "candidate_object_sha256": candidate_object_sha, "object_origin": "reconstructed_source", "fallback_asm_used": False, "nonmatching_fallback_linked": False, "authority_advanced": False, "original_proof_sha256": hashlib.sha256(b"original").hexdigest(), "compiler_command_count": 1, "compiler_commands_sha256": hashlib.sha256(json.dumps([command], sort_keys=True, separators=(",", ":")).encode()).hexdigest(), "paired_compile_command_sha256": hashlib.sha256(command.encode()).hexdigest(), "paired_compile_commands": [command], "before_response_sha256": None, "after_response_sha256": None}
else:
    source_link = {"schema": "owner_campaign_source_link_pending/v1", "campaign_id": os.environ["OWNER_CAMPAIGN_ID"], "owner": os.environ["OWNER_CAMPAIGN_OWNER"], "unit": os.environ["OWNER_CAMPAIGN_UNIT"], "function": os.environ["OWNER_CAMPAIGN_FUNCTION"], "source_sha256": source_sha, "candidate_object_sha256": candidate_object_sha, "status": "not_proven", "authority_advanced": False}
object_proof = {"schema": "owner_campaign_object_proof/v1", "owner": os.environ["OWNER_CAMPAIGN_OWNER"], "unit": os.environ["OWNER_CAMPAIGN_UNIT"], "function": os.environ["OWNER_CAMPAIGN_FUNCTION"], "candidate_object_path": "candidate.o", "candidate_object_sha256": candidate_object_sha, "candidate_object_size": size, "source_sha256": source_sha, "authority_advanced": False}
toolchain_proof = {"schema": "owner_campaign_toolchain_proof/v1", "descriptor_sha256": os.environ["OWNER_CAMPAIGN_TOOLCHAIN_SHA256"], "manifest_sha256": None, "components": {}, "authority_advanced": False}
proofs = {"source_link": source_link, "object": object_proof, "toolchain": toolchain_proof}
for proof in proofs.values():
    proof["proof_sha256"] = hashlib.sha256(json.dumps(proof, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
receipts.update({name: proof["proof_sha256"] for name, proof in proofs.items()})
body = {
    "schema": "owner_campaign_measurement/v1",
    "phase": phase,
    "campaign_id": os.environ["OWNER_CAMPAIGN_ID"],
    "manifest_sha256": os.environ["OWNER_CAMPAIGN_MANIFEST_SHA256"],
    "owner": os.environ["OWNER_CAMPAIGN_OWNER"],
    "unit": os.environ["OWNER_CAMPAIGN_UNIT"],
    "function": os.environ["OWNER_CAMPAIGN_FUNCTION"],
    "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"],
    "base_commit": os.environ["OWNER_CAMPAIGN_BASE_COMMIT"],
    "source_sha256": os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"],
    "target_object_sha256": os.environ["OWNER_CAMPAIGN_TARGET_SHA256"],
    "toolchain_sha256": os.environ["OWNER_CAMPAIGN_TOOLCHAIN_SHA256"],
    "measurement_producer_sha256": os.environ["OWNER_CAMPAIGN_MEASUREMENT_PRODUCER_SHA256"],
    "candidate_object_sha256": candidate_object_sha,
    "metrics": {
        "strict": {"target_bytes": 100, "candidate_bytes": size, "differences": diff},
        "data": {"target_bytes": 100, "candidate_bytes": size, "differences": data_diff},
        "physical_target_count": 5,
        "physical_candidate_count": 5,
        "physical_differences": physical,
        "protected_total": 2,
        "protected_losses": 1 if "LOSS" in text else 0,
        "source_link_exact": linked,
    },
    "report_receipts": receipts,
    "proofs": proofs,
    "focus_evidence": focus_evidence,
    "exact_report": None,
}
body["measurement_sha256"] = hashlib.sha256(
    json.dumps(body, sort_keys=True, separators=(",", ":")).encode()
).hexdigest()
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(body, sort_keys=True, separators=(",", ":")), encoding="utf-8")
'''


def digest_json(value: object) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def _init_fixture_git(root: Path) -> None:
    """Create a byte-stable fixture repository independent of user Git config."""

    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    subprocess.run(
        ["git", "config", "core.autocrlf", "false"],
        cwd=root,
        check=True,
    )


class OwnerCampaignAcceptanceTests(unittest.TestCase):
    """Black-box acceptance coverage for the owner-campaign v2 runtime."""

    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name).resolve()
        (self.root / "src").mkdir()
        (self.root / "build" / "evidence").mkdir(parents=True)
        self.source = self.root / "src" / "test.c"
        self.source.write_text("int focus(void) { return 0; } /* BASE */\n", encoding="utf-8")
        self.target = self.root / "build" / "evidence" / "target.o"
        self.target.write_bytes(b"target")
        self.toolchain = self.root / "build" / "evidence" / "toolchain.json"
        self.toolchain.write_text("{}\n", encoding="utf-8")
        (self.root / "hook.py").write_text(ACCEPTANCE_HOOK, encoding="utf-8")

        _init_fixture_git(self.root)
        subprocess.run(
            ["git", "config", "user.email", "test@example.invalid"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Acceptance Test"],
            cwd=self.root,
            check=True,
        )
        subprocess.run(["git", "add", "src/test.c", "hook.py"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "fixture"], cwd=self.root, check=True)
        self.commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=self.root, text=True
        ).strip()

        self.manifest_path = self.root / "build" / "campaign.json"
        body: dict[str, object] = {
            "schema": "owner_campaign/v1",
            "campaign_id": "acceptance-owner-v2",
            "owner": "main:test/owner",
            "unit": "main/test/owner",
            "source_relpath": "src/test.c",
            "base_commit": self.commit,
            "target_object": {
                "path": "build/evidence/target.o",
                "sha256": digest_bytes(b"target"),
            },
            "toolchain": {
                "path": "build/evidence/toolchain.json",
                "sha256": campaign._digest_file(self.toolchain),
            },
            "measurement_producer": {
                "path": "hook.py",
                "sha256": campaign._digest_file(self.root / "hook.py"),
            },
            # The hook's protected census is two, so retain two protected
            # functions while leaving multiple functions for exact-manifest
            # progress without invoking final-owner closure.
            "functions": ["focus", "sibling", "third"],
            "protected_exact_functions": ["sibling", "third"],
            "allowed_source_paths": ["src/test.c"],
            "allowed_build_paths": ["build"],
            "forbidden_constructs": [r"\b(?:asm|volatile|register)\b", r"#\s*pragma"],
            "commands": {
                "snapshot": {
                    "argv": [
                        sys.executable,
                        "{MEASUREMENT_PRODUCER}",
                        "{SOURCE}",
                        "build/hook/snapshot.json",
                        "{ROOT}/build/invocations.log",
                    ],
                    "measurement_relpath": "build/hook/snapshot.json",
                },
                "candidate": {
                    "argv": [
                        sys.executable,
                        "{MEASUREMENT_PRODUCER}",
                        "{SOURCE}",
                        "build/hook/candidate.json",
                        "{ROOT}/build/invocations.log",
                    ],
                    "measurement_relpath": "build/hook/candidate.json",
                },
                "final_owner": {
                    "argv": [
                        sys.executable,
                        "{MEASUREMENT_PRODUCER}",
                        "{SOURCE}",
                        "build/hook/final-owner.json",
                        "{ROOT}/build/invocations.log",
                    ],
                    "measurement_relpath": "build/hook/final-owner.json",
                },
            },
            "cancellation_epoch": 1,
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
        self.manifest = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(self.manifest), encoding="utf-8")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def load(self) -> dict[str, object]:
        return campaign.load_campaign(self.root, self.manifest_path)

    def candidate(
        self,
        marker: str,
        frontier: dict[str, object],
        name: str = "cell",
        *,
        source_path: Path | None = None,
        hypothesis_family: str | None = None,
    ) -> Path:
        source = source_path or self.root / "build" / "candidates" / f"{name}.c"
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text(
            f"int focus(void) {{ return 0; }} /* {marker} */\n", encoding="utf-8"
        )
        base_span = self.source.read_bytes()
        candidate_span = source.read_bytes()
        body: dict[str, object] = {
            "schema": "owner_campaign_candidate/v1",
            "campaign_id": "acceptance-owner-v2",
            "function": "focus",
            "base_frontier_sha256": frontier["frontier_sha256"],
            "candidate_source": {
                "path": source.relative_to(self.root).as_posix(),
                "sha256": campaign._digest_file(source),
            },
            "function_span": {
                "base_start_line": 1, "base_end_line": 1,
                "candidate_start_line": 1, "candidate_end_line": 1,
                "base_sha256": digest_bytes(base_span),
                "candidate_sha256": digest_bytes(candidate_span),
            },
            "hypothesis_family": hypothesis_family or f"family-{name}",
            "natural_c": True,
            "created_at": "2026-08-31T00:00:00Z",
        }
        descriptor = seal(body, "candidate_sha256")
        path = self.root / "build" / "candidates" / f"{name}.json"
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        return path

    def owner_state_root(self) -> Path:
        owners = list((self.root / "build" / "owner-campaign" / "owners").glob("*"))
        self.assertEqual(len(owners), 1)
        return owners[0]

    def function_state_root(self, function: str = "focus") -> Path:
        matches = list(self.owner_state_root().glob("*"))
        candidates = [path for path in matches if (path / "latest-frontier.json").is_file()]
        self.assertEqual(len(candidates), 1)
        return candidates[0]

    def invocation_lines(self) -> list[str]:
        path = self.root / "build" / "invocations.log"
        return path.read_text(encoding="utf-8").splitlines() if path.exists() else []

    def test_five_workers_run_in_parallel_with_isolated_scratch(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidates = [
            self.candidate(f"IMPROVE FIVE lane-{index}", base, f"five-{index}")
            for index in range(5)
        ]

        started = time.monotonic()
        results = campaign.run_loop(self.root, loaded, candidates)
        elapsed = time.monotonic() - started

        self.assertEqual(len(results), 5)
        self.assertEqual(
            {item["status"] for item in results}, {"improved", "stale_rebase"}
        )
        self.assertEqual(sum(item["status"] == "improved" for item in results), 1)
        # The five hooks synchronize before returning.  Serial execution would
        # time out each barrier; successful completion demonstrates five live
        # end-to-end workers rather than a merely bounded executor setting.
        self.assertLess(elapsed, 5.0)
        scratch_root = self.root / "build" / "owner-campaign" / "scratch"
        repos = sorted(
            path.name
            for path in scratch_root.rglob("repo-*")
            if path.is_dir()
        )
        self.assertEqual(repos, [f"repo-{index}" for index in range(5)])
        self.assertEqual(
            sum(line.startswith("candidate:") for line in self.invocation_lines()), 5
        )

    def test_concurrent_duplicate_candidate_is_compiled_once(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        shared_source = self.root / "build" / "candidates" / "duplicate.c"
        first = self.candidate(
            "REGRESS DUP", base, "duplicate-a", source_path=shared_source,
            hypothesis_family="duplicate-a",
        )
        second = self.candidate(
            "REGRESS DUP", base, "duplicate-b", source_path=shared_source,
            hypothesis_family="duplicate-b",
        )

        results = campaign.run_loop(self.root, loaded, [first, second])

        self.assertEqual(
            [item["status"] for item in results], ["no_gain", "deduplicated"]
        )
        self.assertEqual(
            sum(line.startswith("candidate:") for line in self.invocation_lines()), 1,
            "the same candidate key must not compile twice under concurrent dispatch",
        )
        ledger_paths = list(self.owner_state_root().rglob("candidate-results.jsonl"))
        self.assertEqual(len(ledger_paths), 1)
        self.assertEqual(len(ledger_paths[0].read_text(encoding="utf-8").splitlines()), 1)
        self.assertIn("BASE", self.source.read_text(encoding="utf-8"))

    def test_stale_frontier_cas_keeps_one_winner_and_rejects_losers(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        first = self.candidate("IMPROVE CAS first", base, "cas-first")
        second = self.candidate("IMPROVE CAS second", base, "cas-second")

        results = campaign.run_loop(self.root, loaded, [first, second])
        winner = next(item for item in results if item["status"] == "improved")
        self.assertEqual(
            {item["status"] for item in results}, {"improved", "stale_rebase"}
        )
        latest = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(latest["generation"], 1)
        self.assertEqual(latest["parent_frontier_sha256"], base["frontier_sha256"])
        self.assertEqual(latest["frontier_sha256"], winner["frontier_sha256"])
        self.assertEqual(latest["source_sha256"], winner["source_sha256"])
        self.assertEqual(campaign._digest_file(self.source), winner["source_sha256"])
        self.assertEqual(
            sum(line.startswith("candidate:") for line in self.invocation_lines()), 2
        )

    def test_pending_publication_recovers_after_interrupted_retention(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate = self.candidate("IMPROVE pending", base, "pending")
        original_atomic_json = campaign._atomic_json
        interrupted = False

        def crash_at_frontier(
            path: Path, value: object, *, limit: int | None = None
        ) -> None:
            nonlocal interrupted
            if Path(path).name == "latest-frontier.json" and not interrupted:
                interrupted = True
                raise RuntimeError("simulated interruption during frontier publication")
            original_atomic_json(path, value, limit=limit)

        with mock.patch.object(campaign, "_atomic_json", side_effect=crash_at_frontier):
            with self.assertRaisesRegex(RuntimeError, "interruption"):
                campaign.run_candidate(self.root, loaded, candidate)

        pending = list(self.owner_state_root().rglob("frontier.pending.json"))
        self.assertEqual(len(pending), 1)
        self.assertIn("IMPROVE", self.source.read_text(encoding="utf-8"))

        recovered = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(recovered["generation"], 1)
        self.assertEqual(recovered["source_sha256"], campaign._digest_file(self.source))
        self.assertEqual(recovered["metrics"]["strict"]["differences"], 6)
        self.assertFalse(pending[0].exists())
        self.assertEqual(
            sum(line.startswith("candidate:") for line in self.invocation_lines()), 1
        )
        self.assertEqual(
            sum(line.startswith("snapshot:") for line in self.invocation_lines()), 1,
            "recovery must use pending state instead of recompiling the snapshot",
        )
        self.assertEqual(
            campaign.snapshot_frontier(self.root, loaded, "focus")["frontier_sha256"],
            recovered["frontier_sha256"],
        )

    def test_cancellation_epoch_stops_loop_without_source_mutation(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate = self.candidate("IMPROVE cancelled", base, "cancelled")
        before = self.source.read_bytes()
        campaign.cancel_campaign(self.root, loaded, loaded["cancellation_epoch"])

        with self.assertRaisesRegex(campaign.CampaignError, "cancelled"):
            campaign.run_loop(self.root, loaded, [candidate])

        self.assertEqual(self.source.read_bytes(), before)
        self.assertEqual(
            sum(line.startswith("candidate:") for line in self.invocation_lines()), 0
        )
        controls = list(self.owner_state_root().glob("campaign-control.json"))
        self.assertEqual(len(controls), 1)
        control = json.loads(controls[0].read_text(encoding="utf-8"))
        self.assertEqual(control["control_sha256"], digest_json({key: control[key] for key in control if key != "control_sha256"}))

    def test_scratch_hard_limit_is_enforced_after_a_cell(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        scratch_bytes = campaign.campaign_status(self.root, loaded)["scratch_bytes"]
        self.assertGreater(scratch_bytes, 1)
        loaded["limits"] = {
            **loaded["limits"],
            "scratch_hard_bytes": scratch_bytes - 1,
        }
        candidate = self.candidate("REGRESS scratch-limit", base, "scratch-limit")

        with self.assertRaisesRegex(campaign.CampaignError, "scratch"):
            campaign.run_candidate(self.root, loaded, candidate)

    def test_owner_state_hard_limit_is_enforced_after_a_cell(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        retained_bytes = campaign.campaign_status(self.root, loaded)["retained_bytes"]
        self.assertGreater(retained_bytes, 1)
        loaded["limits"] = {
            **loaded["limits"],
            "owner_state_bytes": retained_bytes + 1,
        }
        candidate = self.candidate("REGRESS owner-limit", base, "owner-limit")

        with self.assertRaisesRegex(campaign.CampaignError, "owner state"):
            campaign.run_candidate(self.root, loaded, candidate)

    def test_partial_gain_is_monotonic_and_regressions_do_not_replace_frontier(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        gain = self.candidate("IMPROVE partial", base, "partial-gain")
        gained = campaign.run_candidate(self.root, loaded, gain)
        self.assertEqual(gained["status"], "improved")

        retained = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(retained["generation"], base["generation"] + 1)
        self.assertEqual(retained["parent_frontier_sha256"], base["frontier_sha256"])
        self.assertLessEqual(
            retained["metrics"]["strict"]["differences"],
            base["metrics"]["strict"]["differences"],
        )
        self.assertLessEqual(
            retained["metrics"]["data"]["differences"],
            base["metrics"]["data"]["differences"],
        )
        self.assertLessEqual(
            retained["metrics"]["physical_differences"],
            base["metrics"]["physical_differences"],
        )
        retained_source = retained["source_sha256"]

        regression = self.candidate("REGRESS partial", retained, "partial-regress")
        loss = self.candidate("LOSS partial", retained, "partial-loss")
        self.assertEqual(campaign.run_candidate(self.root, loaded, regression)["status"], "no_gain")
        self.assertEqual(campaign.run_candidate(self.root, loaded, loss)["status"], "no_gain")

        after = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(after["frontier_sha256"], retained["frontier_sha256"])
        self.assertEqual(after["generation"], retained["generation"])
        self.assertEqual(after["source_sha256"], retained_source)
        self.assertEqual(campaign._digest_file(self.source), retained_source)

    def test_exact_publishes_content_addressed_compact_report(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("EXACT report", base, "exact-report")
        )

        self.assertEqual(result["status"], "exact")
        receipt = result["exact"]
        self.assertIsInstance(receipt, dict)
        report_path = Path(receipt["report_path"])
        self.assertTrue(report_path.is_file())
        self.assertLessEqual(report_path.stat().st_size, loaded["limits"]["report_bytes"])
        self.assertEqual(report_path.parent.parent.name, "reports")
        self.assertEqual(report_path.stem, receipt["report_sha256"])
        report = json.loads(report_path.read_text(encoding="utf-8"))
        self.assertEqual(report["schema"], "CRACK_REPORT/v1")
        self.assertEqual(report["status"], "exact")
        self.assertTrue(report["completed"])
        self.assertFalse(report["authority_advanced"])
        self.assertEqual(
            report["report_sha256"],
            digest_json({key: report[key] for key in report if key != "report_sha256"}),
        )
        self.assertEqual(report["frontier_sha256"], result["frontier_sha256"])
        self.assertEqual(report["source_sha256"], result["source_sha256"])
        self.assertEqual(report["result"]["strict_percent"], 100)
        self.assertEqual(report["result"]["data_percent"], 100)
        self.assertEqual(report["result"]["physical_difference_count"], 0)
        self.assertEqual(report["result"]["protected_losses"], 0)

        manifests = list(self.owner_state_root().glob("exact-manifest.json"))
        self.assertEqual(len(manifests), 1)
        exact_manifest = json.loads(manifests[0].read_text(encoding="utf-8"))
        self.assertEqual(exact_manifest["exact"]["focus"]["report_sha256"], receipt["report_sha256"])
        self.assertEqual(exact_manifest["exact_manifest_sha256"], digest_json({key: exact_manifest[key] for key in exact_manifest if key != "exact_manifest_sha256"}))
        status = campaign.campaign_status(self.root, loaded)
        self.assertEqual((status["exact_count"], status["total"]), (1, 3))

    def test_v2_ignores_malformed_stop_hmac_permit_and_approval_artifacts(self) -> None:
        loaded = self.load()
        legacy_paths = [
            self.root / "STOP",
            self.root / "HMAC",
            self.root / "permit",
            self.root / "build" / "STOP",
            self.root / "build" / "hmac-permit.json",
            self.root / "build" / "approval.json",
            self.root / "build" / "predicted-row.json",
        ]
        for path in legacy_paths:
            path.write_text("this is intentionally not a valid control packet", encoding="utf-8")

        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("REGRESS legacy-free", base, "legacy-free")
        )

        self.assertEqual(result["status"], "no_gain")
        state_root = self.root / "build" / "owner-campaign"
        forbidden_names = {"stop", "hmac", "permit", "approval", "predicted-row"}
        observed = {
            path.name.casefold()
            for path in state_root.rglob("*")
            if path.is_file()
        }
        self.assertTrue(
            observed.isdisjoint(forbidden_names),
            f"v2 state unexpectedly created legacy control artifacts: {observed & forbidden_names}",
        )


if __name__ == "__main__":
    unittest.main()
