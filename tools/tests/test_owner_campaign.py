from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

from tools import owner_campaign as campaign


HOOK = r'''from __future__ import annotations
import hashlib, json, os, pathlib, sys, time

source = pathlib.Path(sys.argv[1])
output = pathlib.Path(sys.argv[2])
log = pathlib.Path(sys.argv[3])
text = source.read_text(encoding="utf-8")
log.parent.mkdir(parents=True, exist_ok=True)
with log.open("a", encoding="utf-8") as stream:
    stream.write(os.environ["OWNER_CAMPAIGN_PHASE"] + ":" + hashlib.sha256(source.read_bytes()).hexdigest() + "\n")
if os.environ["OWNER_CAMPAIGN_PHASE"] == "final_owner":
    receipts = {name: hashlib.sha256((name + os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"]).encode()).hexdigest() for name in ("source_link", "siblings", "full_owner", "linked")}
    body = {
        "schema": "owner_campaign_final_owner/v1",
        "campaign_id": os.environ["OWNER_CAMPAIGN_ID"],
        "manifest_sha256": os.environ["OWNER_CAMPAIGN_MANIFEST_SHA256"],
        "owner": os.environ["OWNER_CAMPAIGN_OWNER"],
        "unit": os.environ["OWNER_CAMPAIGN_UNIT"],
        "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"],
        "base_commit": os.environ["OWNER_CAMPAIGN_BASE_COMMIT"],
        "source_sha256": os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"],
        "target_object_sha256": os.environ["OWNER_CAMPAIGN_TARGET_SHA256"],
        "toolchain_sha256": os.environ["OWNER_CAMPAIGN_TOOLCHAIN_SHA256"],
        "source_link_exact": True, "protected_exact": True,
        "full_owner_exact": True, "linked_exact": True,
        "source_built_object_sha256": hashlib.sha256(("source-object:" + os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"]).encode()).hexdigest(),
        "linked_binary_sha256": hashlib.sha256(("linked:" + os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"]).encode()).hexdigest(),
        "linker_input_manifest_sha256": hashlib.sha256(("manifest:" + os.environ["OWNER_CAMPAIGN_SOURCE_SHA256"]).encode()).hexdigest(),
        "clean_build": True, "matching_source": True,
        "fallback_asm_used": False, "nonmatching_fallback_linked": False,
        "dtk_checksum_exact": True,
        "proof_receipts": receipts, "completed_at": "2026-08-31T00:00:00Z",
    }
    body["final_owner_sha256"] = hashlib.sha256(json.dumps(body, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(body, sort_keys=True, separators=(",", ":")), encoding="utf-8")
    raise SystemExit(0)
if "INFRA" in text:
    raise SystemExit(7)
if "SLOW" in text:
    barrier = log.parent / "parallel-barrier"
    barrier.mkdir(parents=True, exist_ok=True)
    marker = barrier / hashlib.sha256(source.read_bytes()).hexdigest()
    marker.write_text("ready", encoding="utf-8")
    deadline = time.monotonic() + 5.0
    while len(list(barrier.iterdir())) < 2:
        if time.monotonic() >= deadline:
            raise SystemExit(8)
        time.sleep(0.01)
    time.sleep(0.1)
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
receipts = {name: hashlib.sha256((name + source_sha).encode()).hexdigest() for name in ("strict", "data", "physical", "siblings", "source_link", "focus")}
protected_names = [
    item for item in os.environ["OWNER_CAMPAIGN_PROTECTED_FUNCTIONS"].split(",")
    if item and item != os.environ["OWNER_CAMPAIGN_FUNCTION"]
]
protected_total = len(protected_names)
focus_body = {
    "schema": "owner_campaign_focus_evidence/v1",
    "owner": os.environ["OWNER_CAMPAIGN_OWNER"],
    "function": os.environ["OWNER_CAMPAIGN_FUNCTION"],
    "unit": os.environ["OWNER_CAMPAIGN_UNIT"],
    "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"],
    "base_commit": os.environ["OWNER_CAMPAIGN_BASE_COMMIT"],
    "source_sha256": source_sha,
    "target_object_sha256": os.environ["OWNER_CAMPAIGN_TARGET_SHA256"],
    "strict_rows": [f"strict:{os.environ['OWNER_CAMPAIGN_FUNCTION']}:row:{index}:kind=DIFF_ARG_MISMATCH:target={index}:candidate={index}" for index in range(diff)],
    "data_rows": [f"data:{os.environ['OWNER_CAMPAIGN_FUNCTION']}:row:{index}:kind=DIFF_ARG_MISMATCH:target={index}:candidate={index}" for index in range(data_diff)],
    "physical_differences": [f"physical:{index}" for index in range(physical)],
    "strict_row_ids": [f"strict:{os.environ['OWNER_CAMPAIGN_FUNCTION']}:row:{index}:kind=DIFF_ARG_MISMATCH:target={index}:candidate={index}" for index in range(diff)],
    "data_row_ids": [f"data:{os.environ['OWNER_CAMPAIGN_FUNCTION']}:row:{index}:kind=DIFF_ARG_MISMATCH:target={index}:candidate={index}" for index in range(data_diff)],
    "physical_difference_ids": [f"physical:{index}" for index in range(physical)],
    "physical_target_identity_sha256": hashlib.sha256(b"physical-target").hexdigest(),
    "physical_candidate_identity_sha256": hashlib.sha256(b"physical-target").hexdigest() if physical == 0 else hashlib.sha256(("physical-candidate:" + str(physical)).encode()).hexdigest(),
    "strict_row_count": diff, "data_row_count": data_diff,
    "physical_target_count": 5, "physical_candidate_count": 5,
    "physical_difference_count": physical,
    "protected_total": protected_total,
    "protected_losses": 1 if "LOSS" in text else 0,
    "sibling_identities": protected_names,
    "sibling_digest": hashlib.sha256(json.dumps(protected_names, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
}
focus_body["strict_row_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["strict_row_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_body["data_row_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["data_row_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_body["physical_difference_ids_sha256"] = hashlib.sha256(json.dumps(focus_body["physical_difference_ids"], sort_keys=True, separators=(",", ":")).encode()).hexdigest()
focus_body["focus_evidence_sha256"] = hashlib.sha256(json.dumps(focus_body, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
receipts["focus"] = focus_body["focus_evidence_sha256"]
candidate_object_sha = hashlib.sha256(("object:" + source_sha).encode()).hexdigest()
if linked:
    command = "mwcc source.c -o candidate.o"
    source_link = {
        "schema": "owner_campaign_source_link_proof/v1", "source_path": os.environ["OWNER_CAMPAIGN_SOURCE_PATH"],
        "source_sha256": source_sha, "candidate_object_path": "candidate.o",
        "candidate_object_sha256": candidate_object_sha, "object_origin": "reconstructed_source",
        "fallback_asm_used": False, "nonmatching_fallback_linked": False, "authority_advanced": False,
        "original_proof_sha256": hashlib.sha256(b"original").hexdigest(), "compiler_command_count": 1,
        "compiler_commands_sha256": hashlib.sha256(json.dumps([command], sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
        "paired_compile_command_sha256": hashlib.sha256(command.encode()).hexdigest(),
        "paired_compile_commands": [command], "before_response_sha256": None, "after_response_sha256": None,
    }
else:
    source_link = {
        "schema": "owner_campaign_source_link_pending/v1", "campaign_id": os.environ["OWNER_CAMPAIGN_ID"],
        "owner": os.environ["OWNER_CAMPAIGN_OWNER"], "unit": os.environ["OWNER_CAMPAIGN_UNIT"],
        "function": os.environ["OWNER_CAMPAIGN_FUNCTION"], "source_sha256": source_sha,
        "candidate_object_sha256": candidate_object_sha, "status": "not_proven", "authority_advanced": False,
    }
object_proof = {"schema": "owner_campaign_object_proof/v1", "owner": os.environ["OWNER_CAMPAIGN_OWNER"], "unit": os.environ["OWNER_CAMPAIGN_UNIT"], "function": os.environ["OWNER_CAMPAIGN_FUNCTION"], "candidate_object_path": "candidate.o", "candidate_object_sha256": candidate_object_sha, "candidate_object_size": size, "source_sha256": source_sha, "authority_advanced": False}
toolchain_proof = {"schema": "owner_campaign_toolchain_proof/v1", "descriptor_sha256": os.environ["OWNER_CAMPAIGN_TOOLCHAIN_SHA256"], "manifest_sha256": None, "components": {}, "authority_advanced": False}
proofs = {"source_link": source_link, "object": object_proof, "toolchain": toolchain_proof}
for proof in proofs.values():
    proof["proof_sha256"] = hashlib.sha256(json.dumps(proof, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
receipts.update({name: proof["proof_sha256"] for name, proof in proofs.items()})
body = {
    "schema": "owner_campaign_measurement/v1",
    "phase": os.environ["OWNER_CAMPAIGN_PHASE"],
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
        "physical_target_count": 5, "physical_candidate_count": 5,
        "physical_differences": physical,
        "protected_total": protected_total,
        "protected_losses": 1 if "LOSS" in text else 0,
        "source_link_exact": linked,
    },
    "report_receipts": receipts,
    "proofs": proofs,
    "focus_evidence": focus_body,
    "exact_report": None,
}
if os.environ.get("OWNER_CAMPAIGN_RECONSTRUCTION") == "1":
    residual_count = max(diff, data_diff)
    residual_rows = []
    for index in range(residual_count):
        row_ids = {
            "strict": [focus_body["strict_row_ids"][index]] if index < diff else [],
            "data": [focus_body["data_row_ids"][index]] if index < data_diff else [],
        }
        residual_rows.append({
            "anchor_index": index,
            "channels": [name for name in ("data", "strict") if row_ids[name]],
            "row_ids": row_ids,
            "target": {"index": index, "present": True},
            "candidate": {"index": index, "present": True},
            "target_row_sha256": hashlib.sha256(("target:" + str(index)).encode()).hexdigest(),
            "candidate_row_sha256": hashlib.sha256(("candidate:" + str(index)).encode()).hexdigest(),
        })
    clusters = []
    if residual_rows:
        clusters.append({
            "cluster_id": "cluster-000", "first_index": 0,
            "last_index": residual_count - 1,
            "row_indices": list(range(residual_count)),
            "residual_event_count": residual_count,
            "channels": [name for name in ("data", "strict") if focus_body[name + "_row_ids"]],
            "strict_row_ids": focus_body["strict_row_ids"],
            "data_row_ids": focus_body["data_row_ids"],
            "window_ids": [], "mirror_group": "mirror-000",
            "mirror_group_size": 1, "mirror_occurrence": 0,
        })
    physical_rows = [{"index": index} for index in range(physical)]
    physical_ids = focus_body["physical_difference_ids"]
    status = "READY"
    exact_possible = physical == 0
    exact_reason = (
        "size_control_flow_and_physical_invariants_closed"
        if exact_possible else "physical_relocation_difference"
    )
    source_span = {
        "function": body["function"], "start_line": 1, "end_line": 1,
        "start_offset": 0, "end_offset": len(text),
        "span_sha256": hashlib.sha256(text.encode()).hexdigest(),
    }
    physical_payload = {
        "status": "exact" if physical == 0 else "mismatch",
        "status_known": True,
        "target": {"count": 5, "count_valid": True,
                   "relocations_sha256": focus_body["physical_target_identity_sha256"]},
        "candidate": {"count": 5, "count_valid": True,
                      "relocations_sha256": focus_body["physical_candidate_identity_sha256"]},
        "difference_count": physical, "difference_count_valid": True,
        "differences": physical_rows,
        "differences_sha256": hashlib.sha256(json.dumps(physical_rows, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
        "difference_ids": physical_ids,
        "difference_ids_sha256": hashlib.sha256(json.dumps(physical_ids, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
    }
    reconstruction = {
        "schema": "owner_campaign_reconstruction_packet/v1", "schema_version": 1,
        "owner": body["owner"], "unit": body["unit"], "function": body["function"],
        "source_path": body["source_path"], "source_sha256": source_sha,
        "frontier_source_sha256": source_sha,
        "base_commit": body["base_commit"],
        "target_object_sha256": body["target_object_sha256"],
        "candidate_object_sha256": candidate_object_sha,
        "toolchain_sha256": body["toolchain_sha256"],
        "parent_frontier_sha256": None, "source_span": source_span,
        "source_span_sha256": hashlib.sha256(json.dumps(source_span, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
        "focus_artifact_sha256": focus_body["focus_evidence_sha256"],
        "focus_report_schema": "focus_symbol_report/v1",
        "strict_residuals": focus_body["strict_row_ids"],
        "data_residuals": focus_body["data_row_ids"],
        "strict_residual_count": len(focus_body["strict_row_ids"]),
        "data_residual_count": len(focus_body["data_row_ids"]),
        "residual_event_count": residual_count,
        "residual_rows_complete": True, "residual_rows_total_count": residual_count,
        "residual_rows_full_sha256": hashlib.sha256(json.dumps(residual_rows, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
        "residual_rows": residual_rows,
        "causal_clusters_complete": True, "causal_clusters_total_count": len(clusters),
        "causal_clusters_full_sha256": hashlib.sha256(json.dumps(clusters, sort_keys=True, separators=(",", ":")).encode()).hexdigest(),
        "causal_clusters": clusters, "causal_cluster_count": len(clusters),
        "selected_cluster_count": len(clusters),
        "instruction_windows_complete": True, "instruction_windows": [],
        "decomposition_regions": [],
        "status": status, "exact_terminal_possible": exact_possible,
        "exact_terminal_reason": exact_reason,
        "target_first_signal": {
            "status": status, "reason": "fixture",
            "exact_terminal_possible": exact_possible,
            "exact_terminal_reason": exact_reason,
            "first_residual_index": 0 if residual_rows else None,
            "cluster_count": len(clusters), "owner_inference": "none",
            "next_action": "CRACK", "decomposition_required": False,
            "decomposition_regions": [],
        },
        "control_flow": {"target": {}, "candidate": {}},
        "stack_relative": {"target": {}, "candidate": {}},
        "machine_summary": {"target": {}, "candidate": {}},
        "physical_relocations": physical_payload,
        "physical_relocation_differences": physical_rows,
        "physical_difference_ids": physical_ids,
        "physical_difference_ids_sha256": physical_payload["difference_ids_sha256"],
        "diagnostic_only": True, "authority_advanced": False,
        "reconstruction_policy": {
            "target_first": True, "donor_required": False,
            "history_required": False, "compile_authorized": False,
            "window_radius": 0, "cluster_gap": 8,
            "source_text_emitted": False, "source_patch_emitted": False,
            "broad_residual_requires_decomposition": False,
        },
    }
    reconstruction["packet_sha256"] = hashlib.sha256(json.dumps(
        reconstruction, sort_keys=True, separators=(",", ":")
    ).encode()).hexdigest()
    body["reconstruction_evidence"] = reconstruction
body["measurement_sha256"] = hashlib.sha256(json.dumps(body, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(body, sort_keys=True, separators=(",", ":")), encoding="utf-8")
'''


def digest_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def seal(body: dict[str, object], field: str) -> dict[str, object]:
    result = dict(body)
    result[field] = hashlib.sha256(
        json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()
    return result


def _init_fixture_git(root: Path) -> None:
    """Create a byte-stable fixture repository independent of user Git config."""

    subprocess.run(["git", "init", "-q"], cwd=root, check=True)
    subprocess.run(
        ["git", "config", "core.autocrlf", "false"],
        cwd=root,
        check=True,
    )


class OwnerCampaignTests(unittest.TestCase):
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
        (self.root / "hook.py").write_text(HOOK, encoding="utf-8")
        _init_fixture_git(self.root)
        subprocess.run(["git", "config", "user.email", "test@example.invalid"], cwd=self.root, check=True)
        subprocess.run(["git", "config", "user.name", "Test"], cwd=self.root, check=True)
        subprocess.run(["git", "add", "src/test.c", "hook.py"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "fixture"], cwd=self.root, check=True)
        self.commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=self.root, text=True).strip()
        self.manifest_path = self.root / "build" / "campaign.json"
        body: dict[str, object] = {
            "schema": "owner_campaign/v1", "campaign_id": "test-owner-v1",
            "owner": "main:test/owner", "unit": "main/test/owner",
            "source_relpath": "src/test.c", "base_commit": self.commit,
            "target_object": {"path": "build/evidence/target.o", "sha256": digest_bytes(b"target")},
            "toolchain": {"path": "build/evidence/toolchain.json", "sha256": campaign._digest_file(self.toolchain)},
            "measurement_producer": {
                "path": "hook.py", "sha256": campaign._digest_file(self.root / "hook.py")
            },
            "functions": ["focus", "sibling"], "protected_exact_functions": ["sibling"],
            "allowed_source_paths": ["src/test.c"], "allowed_build_paths": ["build"],
            "forbidden_constructs": [r"\b(?:asm|volatile|register)\b", r"#\s*pragma"],
            "commands": {
                "snapshot": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}", "{SOURCE}", "build/hook/snapshot.json", "{ROOT}/build/invocations.log"],
                    "measurement_relpath": "build/hook/snapshot.json",
                },
                "candidate": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}", "{SOURCE}", "build/hook/candidate.json", "{ROOT}/build/invocations.log"],
                    "measurement_relpath": "build/hook/candidate.json",
                },
                "final_owner": {
                    "argv": [sys.executable, "{MEASUREMENT_PRODUCER}", "{SOURCE}", "build/hook/final-owner.json", "{ROOT}/build/invocations.log"],
                    "measurement_relpath": "build/hook/final-owner.json",
                },
            },
            "cancellation_epoch": 1,
            "limits": {
                "command_timeout_seconds": 20, "scratch_soft_bytes": 32 << 20,
                "scratch_hard_bytes": 64 << 20, "cell_temporary_bytes": 1 << 20,
                "focus_evidence_bytes": 256 << 10,
                "frontier_bytes": 64 << 10, "report_bytes": 64 << 10,
                "dedupe_bytes": 1 << 20, "owner_state_bytes": 16 << 20,
            },
        }
        self.manifest = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(self.manifest), encoding="utf-8")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def load(self) -> dict[str, object]:
        return campaign.load_campaign(self.root, self.manifest_path)

    def candidate(
        self, marker: str, frontier: dict[str, object], name: str = "cell",
        function: str = "focus",
    ) -> Path:
        candidate_source = self.root / "build" / "candidates" / f"{name}.c"
        base_source = self.root / "build" / "candidates" / f"{name}.base.c"
        candidate_source.parent.mkdir(parents=True, exist_ok=True)
        base_source.write_bytes(self.source.read_bytes())
        candidate_source.write_text(f"int focus(void) {{ return 0; }} /* {marker} */\n", encoding="utf-8")
        base_span = base_source.read_bytes()
        candidate_span = candidate_source.read_bytes()
        body: dict[str, object] = {
            "schema": "owner_campaign_candidate/v1", "campaign_id": "test-owner-v1",
            "function": function, "base_frontier_sha256": frontier["frontier_sha256"],
            "base_source": {
                "path": base_source.relative_to(self.root).as_posix(),
                "sha256": campaign._digest_file(base_source),
            },
            "candidate_source": {
                "path": candidate_source.relative_to(self.root).as_posix(),
                "sha256": campaign._digest_file(candidate_source),
            },
            "function_span": {
                "base_start_line": 1, "base_end_line": 1,
                "candidate_start_line": 1, "candidate_end_line": 1,
                "base_sha256": digest_bytes(base_span),
                "candidate_sha256": digest_bytes(candidate_span),
            },
            "hypothesis_family": f"family-{name}", "natural_c": True,
            "rebase_depth": 0,
            "created_at": "2026-08-31T00:00:00Z",
        }
        descriptor = seal(body, "candidate_sha256")
        path = self.root / "build" / "candidates" / f"{name}.json"
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        return path

    def test_manifest_digest_and_closed_fields_are_enforced(self) -> None:
        loaded = self.load()
        self.assertEqual(loaded["manifest_sha256"], self.manifest["manifest_sha256"])
        tampered = dict(self.manifest)
        tampered["owner"] = "main:forged"
        self.manifest_path.write_text(json.dumps(tampered), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "digest"):
            self.load()

    def test_manifest_binds_clean_head_base_blob_and_retained_source(self) -> None:
        self.source.write_text("int focus(void) { return 1; } /* DIRTY */\n", encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "retained frontier"):
            self.load()
        subprocess.run(["git", "checkout", "--", "src/test.c"], cwd=self.root, check=True)
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("IMPROVE", base, "reload")
        )
        self.assertEqual(result["status"], "improved")
        reloaded = self.load()
        self.assertEqual(reloaded["_base_source_sha256"], digest_bytes(
            subprocess.check_output(
                ["git", "show", f"{self.commit}:src/test.c"], cwd=self.root
            )
        ))

        extra = self.root / "extra.txt"
        extra.write_text("new head\n", encoding="utf-8")
        subprocess.run(["git", "add", "extra.txt"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "new head"], cwd=self.root, check=True)
        loaded = self.load()
        self.assertEqual(loaded["base_commit"], self.commit)

    def test_neutral_descendant_head_is_accepted(self) -> None:
        extra = self.root / "workflow-only.txt"
        extra.write_text("workflow-only change\n", encoding="utf-8")
        subprocess.run(["git", "add", "workflow-only.txt"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "workflow-only descendant"], cwd=self.root, check=True)

        loaded = self.load()
        self.assertEqual(loaded["base_commit"], self.commit)
        self.assertEqual(loaded["_base_source_sha256"], digest_bytes(self.source.read_bytes()))

    def test_source_changing_descendant_head_is_rejected(self) -> None:
        self.source.write_text("int focus(void) { return 1; } /* SOURCE DESCENDANT */\n", encoding="utf-8")
        subprocess.run(["git", "add", "src/test.c"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "source descendant"], cwd=self.root, check=True)

        with self.assertRaisesRegex(campaign.CampaignError, "clean campaign source"):
            self.load()

    def test_non_descendant_head_is_rejected(self) -> None:
        subprocess.run(["git", "checkout", "--orphan", "unrelated"], cwd=self.root, check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        subprocess.run(["git", "add", "src/test.c", "hook.py"], cwd=self.root, check=True)
        subprocess.run(["git", "commit", "-qm", "unrelated root"], cwd=self.root, check=True)

        with self.assertRaisesRegex(campaign.CampaignError, "not an ancestor"):
            self.load()

    def test_snapshot_is_cached_by_live_frontier(self) -> None:
        loaded = self.load()
        first = campaign.snapshot_frontier(self.root, loaded, "focus")
        second = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(first["frontier_sha256"], second["frontier_sha256"])

    def test_snapshot_frontiers_deduplicates_and_runs_distinct_functions_concurrently(self) -> None:
        loaded = self.load()
        barrier = threading.Barrier(2, timeout=5)
        calls: list[tuple[str, int]] = []
        calls_lock = threading.Lock()

        def fake_snapshot(
            root: Path, loaded_campaign: dict[str, object], function: str, *,
            force: bool = False, worker: int = 0,
            _defer_maintenance: bool = False,
        ) -> dict[str, object]:
            self.assertEqual(root, self.root)
            self.assertIs(loaded_campaign, loaded)
            self.assertFalse(force)
            self.assertTrue(_defer_maintenance)
            with calls_lock:
                calls.append((function, worker))
            barrier.wait()
            return {"function": function, "worker": worker}

        with mock.patch.object(campaign, "snapshot_frontier", side_effect=fake_snapshot):
            result = campaign.snapshot_frontiers(
                self.root, loaded, ["focus", "sibling", "focus"]
            )
        self.assertEqual(list(result), ["focus", "sibling"])
        self.assertEqual(sorted(calls), [("focus", 0), ("sibling", 1)])

    def test_snapshot_frontiers_propagates_worker_failure_without_mapping(self) -> None:
        loaded = self.load()

        def fake_snapshot(
            root: Path, loaded_campaign: dict[str, object], function: str, *,
            force: bool = False, worker: int = 0,
            _defer_maintenance: bool = False,
        ) -> dict[str, object]:
            if function == "sibling":
                raise campaign.CampaignError("snapshot sentinel")
            return {"function": function, "worker": worker}

        with mock.patch.object(
            campaign, "snapshot_frontier", side_effect=fake_snapshot
        ), self.assertRaisesRegex(campaign.CampaignError, "snapshot sentinel"):
            campaign.snapshot_frontiers(self.root, loaded, ["focus", "sibling"])

    def test_snapshot_workers_steal_skewed_jobs_instead_of_static_partitioning(
        self,
    ) -> None:
        loaded = self.load()
        functions = [f"focus-{index}" for index in range(6)]
        loaded["functions"] = functions
        first_wave = threading.Barrier(5, timeout=5)
        sixth_started = threading.Event()
        workers: dict[str, int] = {}
        lock = threading.Lock()

        def skewed_snapshot(
            root: Path, loaded_campaign: dict[str, object], function: str, *,
            force: bool = False, worker: int = 0,
            _defer_maintenance: bool = False,
        ) -> dict[str, object]:
            index = functions.index(function)
            with lock:
                workers[function] = worker
            if index < 5:
                first_wave.wait()
            if index == 0:
                if not sixth_started.wait(timeout=5):
                    raise AssertionError("sixth snapshot stayed in a static partition")
            elif index == 5:
                sixth_started.set()
            return {"function": function, "worker": worker}

        with mock.patch.object(
            campaign, "snapshot_frontier", side_effect=skewed_snapshot
        ):
            result = campaign.snapshot_frontiers(self.root, loaded, functions)
        self.assertEqual(list(result), functions)
        self.assertTrue(sixth_started.is_set())
        self.assertNotEqual(workers[functions[0]], workers[functions[5]])

    def test_snapshot_frontier_uses_requested_isolated_worker_scratch(self) -> None:
        loaded = self.load()
        original = campaign._ensure_scratch
        calls: list[int] = []

        def capture_scratch(
            root: Path, loaded_campaign: dict[str, object], worker: int = 0,
        ) -> Path:
            calls.append(worker)
            return original(root, loaded_campaign, worker)

        with mock.patch.object(campaign, "_ensure_scratch", side_effect=capture_scratch):
            campaign.snapshot_frontier(self.root, loaded, "focus", worker=3)
        self.assertEqual(calls, [3])
        scratch = (
            self.root / "build" / "owner-campaign" / "scratch"
            / campaign._slug("test-owner-v1") / "repo-3"
        )
        self.assertTrue(scratch.is_dir())

    def test_ready_candidate_starts_before_unrelated_snapshot_releases(self) -> None:
        loaded = self.load()
        frontiers = campaign.snapshot_frontiers(
            self.root, loaded, ["focus", "sibling"]
        )
        first = self.candidate("REGRESS A", frontiers["focus"], "unique-a", "focus")
        second = self.candidate(
            "REGRESS B", frontiers["sibling"], "unique-b", "sibling"
        )
        original_snapshot = campaign.snapshot_frontier
        original_hook = campaign._run_hook
        first_snapshot_entered = threading.Event()
        second_candidate_started = threading.Event()

        def blocked_snapshot(
            root: Path, loaded_campaign: dict[str, object], function: str, *,
            force: bool = False, worker: int = 0,
            _defer_maintenance: bool = False,
        ) -> dict[str, object]:
            if function == "focus":
                first_snapshot_entered.set()
                if not second_candidate_started.wait(timeout=5):
                    raise AssertionError(
                        "unrelated candidate remained behind the snapshot barrier"
                    )
            return original_snapshot(
                root, loaded_campaign, function, force=force, worker=worker,
                _defer_maintenance=_defer_maintenance,
            )

        def capture_hook(
            root: Path, scratch: Path, loaded_campaign: dict[str, object],
            function: str, source_sha256: str, phase: str,
        ) -> dict[str, object]:
            if function == "sibling" and phase == "candidate":
                self.assertTrue(first_snapshot_entered.wait(timeout=5))
                second_candidate_started.set()
            return original_hook(
                root, scratch, loaded_campaign, function, source_sha256, phase
            )

        with mock.patch.object(
            campaign, "snapshot_frontier", side_effect=blocked_snapshot
        ), mock.patch.object(campaign, "_run_hook", side_effect=capture_hook):
            results = campaign.run_loop(self.root, loaded, [first, second])
        self.assertTrue(first_snapshot_entered.is_set())
        self.assertTrue(second_candidate_started.is_set())
        self.assertEqual([item["status"] for item in results], ["no_gain", "no_gain"])

    def test_candidate_workers_steal_skewed_jobs_and_preserve_result_order(
        self,
    ) -> None:
        loaded = self.load()
        descriptors: list[Path] = []
        for index in range(6):
            path = self.root / "build" / "candidates" / f"queue-{index}.json"
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps({
                "function": f"focus-{index}",
                "base_frontier_sha256": "a" * 64,
                "candidate_source": {"sha256": f"{index + 1:064x}"},
            }), encoding="utf-8")
            descriptors.append(path)
        first_wave = threading.Barrier(5, timeout=5)
        sixth_started = threading.Event()
        workers: dict[int, int] = {}
        lock = threading.Lock()

        def skewed_candidate(
            root: Path, loaded_campaign: dict[str, object], path: Path, *,
            worker: int = 0, _defer_maintenance: bool = False,
        ) -> dict[str, object]:
            index = int(path.stem.rsplit("-", 1)[1])
            with lock:
                workers[index] = worker
            if index < 5:
                first_wave.wait()
            if index == 0:
                if not sixth_started.wait(timeout=5):
                    raise AssertionError("sixth candidate stayed in a static partition")
            elif index == 5:
                sixth_started.set()
            return {
                "schema": "owner_campaign_result/v1", "status": "no_gain",
                "candidate": str(index), "authority_advanced": False,
            }

        with mock.patch.object(
            campaign, "run_candidate", side_effect=skewed_candidate
        ):
            results = campaign.run_loop(self.root, loaded, descriptors)
        self.assertEqual([result["candidate"] for result in results], [
            str(index) for index in range(6)
        ])
        self.assertTrue(sixth_started.is_set())
        self.assertNotEqual(workers[0], workers[5])

    def test_five_candidate_batch_runs_one_deferred_maintenance_pass(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        paths = [
            self.candidate(f"REGRESS {index}", frontier, f"batch-{index}")
            for index in range(5)
        ]
        original = campaign._check_limits
        with mock.patch.object(
            campaign, "_check_limits", wraps=original
        ) as maintenance:
            results = campaign.run_loop(self.root, loaded, paths)
        self.assertEqual(maintenance.call_count, 1)
        self.assertEqual([item["status"] for item in results], ["no_gain"] * 5)
        for path in paths:
            self.assertFalse(path.exists())
            self.assertFalse(path.with_suffix(".c").exists())
            self.assertFalse(path.with_suffix(".base.c").exists())

    def test_batch_maintenance_failure_exposes_finalized_primary_results(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        paths = [
            self.candidate(f"REGRESS {index}", frontier, f"maintenance-{index}")
            for index in range(2)
        ]
        with mock.patch.object(
            campaign, "_check_limits",
            side_effect=campaign.CampaignError("maintenance sentinel"),
        ), self.assertRaisesRegex(
            campaign.CampaignError,
            "batch maintenance failed after candidate results were finalized",
        ) as caught:
            campaign.run_loop(self.root, loaded, paths)
        results = caught.exception.candidate_results
        self.assertEqual([item["status"] for item in results], ["no_gain", "no_gain"])
        for path in paths:
            self.assertFalse(path.exists())

    def test_worker_failure_still_runs_one_maintenance_without_masking_primary(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        paths = [
            self.candidate("REGRESS A", frontier, "worker-error-a"),
            self.candidate("REGRESS B", frontier, "worker-error-b"),
        ]
        primary = campaign.CampaignError("worker sentinel")
        maintenance = campaign.CampaignError("maintenance sentinel")
        with mock.patch.object(
            campaign, "run_candidate", side_effect=primary,
        ), mock.patch.object(
            campaign, "_check_limits", side_effect=maintenance,
        ) as check_limits, self.assertRaisesRegex(
            campaign.CampaignError, "worker sentinel"
        ) as caught:
            campaign.run_loop(self.root, loaded, paths)
        self.assertIs(caught.exception, primary)
        self.assertEqual(check_limits.call_count, 1)
        self.assertIn(
            "batch maintenance failed: maintenance sentinel",
            getattr(caught.exception, "__notes__", []),
        )

    def test_snapshot_publishes_and_status_loads_reconstruction_packet(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        digest = frontier["reconstruction_evidence_sha256"]
        path = (
            self.root / "build" / "owner-campaign" / "proof-cas"
            / "reconstruction" / digest[:2] / f"{digest}.json"
        )
        self.assertTrue(path.is_file())
        status = campaign.campaign_status(self.root, loaded)
        self.assertEqual(
            status["reconstruction_evidence"]["focus"]["sha256"], digest
        )
        self.assertEqual(
            status["reconstruction_evidence"]["focus"]["status"],
            frontier["reconstruction_status"],
        )

    def test_old_frontier_without_reconstruction_fields_remains_readable(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        legacy_body = {
            key: value for key, value in frontier.items()
            if key not in {
                "frontier_sha256", "reconstruction_evidence_sha256",
                "reconstruction_status",
            }
        }
        legacy = {**legacy_body, "frontier_sha256": campaign._digest_json(legacy_body)}
        checked = campaign._validate_frontier(legacy, loaded, "focus")
        self.assertNotIn("reconstruction_evidence_sha256", checked)

    def test_reconstruction_cas_gc_preserves_referenced_and_removes_orphan(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        root = self.root / "build" / "owner-campaign" / "proof-cas" / "reconstruction"
        orphan_digest = "f" * 64
        orphan = root / orphan_digest[:2] / f"{orphan_digest}.json"
        orphan.parent.mkdir(parents=True, exist_ok=True)
        orphan.write_text("{}\n", encoding="utf-8")
        campaign._gc_reconstruction_evidence(
            self.root, minimum_age_seconds=0
        )
        self.assertTrue(orphan.exists())
        campaign._gc_reconstruction_evidence(
            self.root, minimum_age_seconds=0
        )
        self.assertFalse(orphan.exists())
        digest = frontier["reconstruction_evidence_sha256"]
        self.assertTrue((root / digest[:2] / f"{digest}.json").is_file())

    def test_tampered_reconstruction_cas_is_rejected_on_load(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        digest = frontier["reconstruction_evidence_sha256"]
        path = (
            self.root / "build" / "owner-campaign" / "proof-cas"
            / "reconstruction" / digest[:2] / f"{digest}.json"
        )
        path.write_text("{}\n", encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "reconstruction evidence"):
            campaign.campaign_status(self.root, loaded)

    def test_current_producer_measurement_cannot_downgrade_to_legacy_envelope(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        source_sha = campaign._digest_file(self.source)
        campaign._sync_scratch_source(
            self.root, scratch, loaded, self.source.read_bytes()
        )
        measurement = campaign._run_hook(
            self.root, scratch, loaded, "focus", source_sha, "snapshot"
        )
        measurement.pop("reconstruction_evidence")
        unsigned = {
            key: value for key, value in measurement.items()
            if key != "measurement_sha256"
        }
        measurement["measurement_sha256"] = campaign._digest_json(unsigned)
        with mock.patch.object(
            campaign, "_measurement_requires_reconstruction", return_value=True
        ):
            with self.assertRaisesRegex(
                campaign.CampaignError, "omitted reconstruction evidence"
            ):
                campaign._validate_measurement(
                    measurement, campaign=loaded, function="focus",
                    phase="snapshot", source_sha256=source_sha,
                )
        lines = (self.root / "build" / "invocations.log").read_text().splitlines()
        self.assertEqual([line.split(":", 1)[0] for line in lines], ["snapshot"])

    def test_nested_manifest_path_keeps_scratch_under_repository_state_root(self) -> None:
        nested = self.root / "build" / "owner-replay" / "campaign.json"
        nested.parent.mkdir(parents=True)
        nested.write_text(json.dumps(self.manifest), encoding="utf-8")
        self.manifest_path = nested

        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)

        expected = self.root / "build" / "owner-campaign" / "scratch"
        self.assertTrue(campaign._inside(expected, scratch))
        self.assertTrue(campaign._scratch_is_owned(loaded, scratch))

    def test_scratch_with_wrong_head_is_recreated_at_bound_base(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        subprocess.run(
            ["git", "commit", "--allow-empty", "-qm", "wrong scratch head"],
            cwd=scratch, check=True,
        )
        self.assertNotEqual(
            subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=scratch, text=True).strip(),
            self.commit,
        )
        repaired = campaign._ensure_scratch(self.root, loaded)
        self.assertEqual(
            subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repaired, text=True).strip(),
            self.commit,
        )
        self.assertEqual(
            campaign._digest_file(repaired / "src" / "test.c"),
            loaded["_base_source_sha256"],
        )

    def test_scratch_disables_autocrlf_and_preserves_base_blob_bytes(self) -> None:
        subprocess.run(
            ["git", "config", "core.autocrlf", "true"], cwd=self.root, check=True
        )
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        source_bytes = (scratch / "src" / "test.c").read_bytes()
        base_blob = subprocess.check_output(
            ["git", "show", f"{self.commit}:src/test.c"], cwd=self.root
        )
        self.assertEqual(source_bytes, base_blob)
        self.assertEqual(campaign._digest_bytes(source_bytes), loaded["_base_source_sha256"])

    def test_five_fresh_workers_materialize_bound_target_in_scratch(self) -> None:
        loaded = self.load()
        relative = Path(loaded["target_object"]["path"])
        self.assertFalse(relative.is_absolute())
        for worker in range(5):
            scratch = campaign._ensure_scratch(self.root, loaded, worker)
            target = scratch / relative
            self.assertTrue(target.is_file())
            self.assertEqual(target.read_bytes(), self.target.read_bytes())
            self.assertEqual(target.stat().st_size, loaded["_target_size"])
            self.assertEqual(
                campaign._digest_file(target),
                loaded["target_object"]["sha256"],
            )

    def test_five_worker_aggregate_over_soft_preserves_config_and_next_hook(
        self,
    ) -> None:
        loaded = self.load()
        scratches: list[Path] = []
        mappings: list[Path] = []
        for worker in range(5):
            scratch = campaign._ensure_scratch(self.root, loaded, worker)
            mapping = scratch / "build" / "config" / "compiler-map.bin"
            mapping.parent.mkdir(parents=True, exist_ok=True)
            mapping.write_bytes(bytes([worker]) * 2048)
            scratches.append(scratch)
            mappings.append(mapping)

        sizes = [campaign._tree_size(scratch) for scratch in scratches]
        per_worker_soft = max(sizes) + 1024
        self.assertGreater(sum(sizes), per_worker_soft)
        loaded["limits"]["scratch_soft_bytes"] = per_worker_soft
        loaded["limits"]["scratch_hard_bytes"] = per_worker_soft + (1 << 20)

        campaign._check_limits(self.root, loaded)
        self.assertTrue(all(mapping.is_file() for mapping in mappings))
        source_sha = campaign._digest_file(
            scratches[4] / loaded["source_relpath"]
        )
        measurement = campaign._run_hook(
            self.root, scratches[4], loaded, "focus", source_sha, "snapshot"
        )
        self.assertEqual(measurement["function"], "focus")
        self.assertTrue(mappings[4].is_file())
        campaign._verify_scratch_target(self.root, scratches[4], loaded)

    def test_oversized_worker_fails_without_deleting_configured_build_state(
        self,
    ) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded, worker=0)
        mapping = scratch / "build" / "config" / "compiler-map.bin"
        mapping.parent.mkdir(parents=True, exist_ok=True)
        mapping.write_bytes(b"persistent compiler mapping" * 512)
        persistent_size = campaign._tree_size(scratch)
        hook_output = scratch / "build" / "hook" / "stale.json"
        hook_output.parent.mkdir(parents=True, exist_ok=True)
        hook_output.write_bytes(b"temporary hook output" * 128)
        loaded["limits"]["scratch_soft_bytes"] = 1
        loaded["limits"]["scratch_hard_bytes"] = persistent_size - 1

        with self.assertRaisesRegex(
            campaign.InfrastructureError,
            "campaign worker 0 scratch exceeds hard limit",
        ):
            campaign._check_limits(self.root, loaded)
        self.assertTrue(mapping.is_file())
        self.assertFalse(hook_output.exists())
        campaign._verify_scratch_target(self.root, scratch, loaded)

    def test_tampered_scratch_target_fails_before_hook(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded, worker=4)
        target = campaign._scratch_target_path(self.root, scratch, loaded)
        target.write_bytes(b"tampered")
        with self.assertRaisesRegex(
            campaign.InfrastructureError, "scratch target object"
        ):
            campaign._run_hook(
                self.root, scratch, loaded, "focus",
                campaign._digest_file(scratch / loaded["source_relpath"]),
                "snapshot",
            )
        invocation_log = self.root / "build" / "invocations.log"
        self.assertFalse(invocation_log.exists())

    def test_absolute_target_binding_preserves_root_containment_contract(self) -> None:
        body = {
            key: value for key, value in self.manifest.items()
            if key != "manifest_sha256"
        }
        body["target_object"] = {
            **body["target_object"], "path": str(self.target.resolve()),
        }
        self.manifest = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(self.manifest), encoding="utf-8")
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded, worker=2)
        relative = self.target.resolve().relative_to(self.root)
        self.assertEqual(
            (scratch / relative).read_bytes(), self.target.read_bytes()
        )

        with tempfile.TemporaryDirectory() as external_directory:
            external = Path(external_directory) / "target.o"
            external.write_bytes(b"target")
            body["target_object"] = {
                "path": str(external), "sha256": digest_bytes(b"target"),
            }
            self.manifest = seal(body, "manifest_sha256")
            self.manifest_path.write_text(
                json.dumps(self.manifest), encoding="utf-8"
            )
            with self.assertRaisesRegex(
                campaign.CampaignError, "escapes the campaign root"
            ):
                self.load()

    def test_git_resolver_prefers_native_windows_git_and_binds_identity(self) -> None:
        fake = self.root / "build" / "git-resolver"
        native = fake / "Git" / "cmd" / "git.exe"
        msys = fake / "devkitPro" / "msys2" / "usr" / "bin" / "git.exe"
        native.parent.mkdir(parents=True)
        msys.parent.mkdir(parents=True)
        native.write_bytes(b"native")
        msys.write_bytes(b"msys")
        self.assertEqual(
            campaign._select_git_executable([msys, native], windows=True),
            native.resolve(),
        )
        loaded = self.load()
        self.assertTrue(Path(loaded["_git_executable"]).is_file())
        self.assertEqual(loaded["_git_sha256"], campaign._digest_file(loaded["_git_executable"]))

    def test_git_resolver_falls_back_when_native_cannot_read_repository(self) -> None:
        fake = self.root / "build" / "git-resolver-fallback"
        native = fake / "Git" / "cmd" / "git.exe"
        msys = fake / "devkitPro" / "msys2" / "usr" / "bin" / "git.exe"
        native.parent.mkdir(parents=True)
        msys.parent.mkdir(parents=True)
        native.write_bytes(b"native")
        msys.write_bytes(b"msys")
        calls: list[tuple[list[str], dict[str, object]]] = []

        def run(argv: list[str], **kwargs: object) -> subprocess.CompletedProcess[str]:
            calls.append((argv, kwargs))
            if argv[-1] == "--version":
                return subprocess.CompletedProcess(argv, 0, "git version 2.45.0\n", "")
            self.assertEqual(argv[1:], ["rev-parse", "--git-dir"])
            if Path(argv[0]).resolve() == native.resolve():
                return subprocess.CompletedProcess(argv, 128, "", "not a repository")
            return subprocess.CompletedProcess(argv, 0, ".git\n", "")

        environment = {
            "ProgramW6432": str(fake),
            "PATH": str(msys.parent),
        }
        with mock.patch.object(campaign.os, "name", "nt"), mock.patch.dict(
            campaign.os.environ, environment, clear=True
        ), mock.patch.object(campaign.subprocess, "run", side_effect=run):
            selected, digest = campaign._resolve_git_executable(self.root)

        self.assertEqual(selected, msys.resolve())
        self.assertEqual(digest, campaign._digest_file(msys))
        repository_probes = [
            (argv, kwargs)
            for argv, kwargs in calls
            if argv[1:] == ["rev-parse", "--git-dir"]
        ]
        self.assertEqual([Path(argv[0]).resolve() for argv, _ in repository_probes], [native.resolve(), msys.resolve()])
        self.assertEqual([kwargs["cwd"] for _, kwargs in repository_probes], [self.root, self.root])

    def test_unowned_scratch_is_never_removed(self) -> None:
        loaded = self.load()
        scratch = campaign._scratch_repo(self.root, loaded)
        scratch.mkdir(parents=True)
        sentinel = scratch / "sentinel.txt"
        sentinel.write_text("owned by somebody else", encoding="utf-8")
        with self.assertRaisesRegex(campaign.InfrastructureError, "campaign-owned identity"):
            campaign._ensure_scratch(self.root, loaded)
        self.assertTrue(sentinel.is_file())

    def test_safe_partial_gain_is_retained_and_continues(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(self.root, loaded, self.candidate("IMPROVE", base))
        self.assertEqual(result["status"], "improved")
        self.assertIn("IMPROVE", self.source.read_text())
        latest = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(latest["generation"], 1)
        self.assertEqual(latest["metrics"]["strict"]["differences"], 6)
        self.assertFalse((campaign._function_root(self.root, loaded, "focus") / "frontier.pending.json").exists())

    def test_snapshot_refreshes_stale_function_after_disjoint_tu_gain(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        original = self.source.read_text(encoding="utf-8")
        self.source.write_text(
            "int unrelated_retained_gain = 1;\n" + original,
            encoding="utf-8",
        )

        refreshed = campaign.snapshot_frontier(self.root, loaded, "focus")

        self.assertEqual(refreshed["generation"], base["generation"] + 1)
        self.assertEqual(
            refreshed["parent_frontier_sha256"], base["frontier_sha256"]
        )
        self.assertEqual(
            refreshed["source_sha256"], campaign._digest_file(self.source)
        )

    def test_no_gain_is_deduplicated_without_source_change(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", base)
        first = campaign.run_candidate(self.root, loaded, path)
        second = campaign.run_candidate(self.root, loaded, self.candidate("REGRESS", base))
        self.assertEqual(first["status"], "no_gain")
        self.assertEqual(second["status"], "deduplicated")
        self.assertIn("BASE", self.source.read_text())

    def test_protected_sibling_loss_rejects_apparent_gain(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(self.root, loaded, self.candidate("LOSS", base))
        self.assertEqual(result["status"], "no_gain")
        self.assertIn("BASE", self.source.read_text())

    def test_infrastructure_failure_is_retryable_and_not_deduped(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("INFRA", base)
        first = campaign.run_loop(self.root, loaded, [path])[0]
        second = campaign.run_loop(self.root, loaded, [path])[0]
        self.assertEqual(first["status"], "infra_retry")
        self.assertEqual(second["status"], "infra_retry")
        dedupe = campaign._dedupe_path(self.root, loaded, "focus")
        self.assertFalse(dedupe.exists())
        self.assertTrue(path.exists(), "infra retry must preserve the descriptor")
        source_path = self.root / "build" / "candidates" / "cell.c"
        self.assertTrue(source_path.exists(), "infra retry must preserve candidate source")

    def test_hook_inputs_and_producer_are_rehashed_before_launch(self) -> None:
        loaded = self.load()
        (self.root / "hook.py").write_text(HOOK + "\n# drift\n", encoding="utf-8")
        with self.assertRaisesRegex(campaign.InfrastructureError, "measurement_producer"):
            campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertFalse((self.root / "build" / "invocations.log").exists())

    def test_measurement_hooks_use_only_hash_bound_scratch_import_root(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        source = self.source.read_bytes()
        source_sha = campaign._digest_bytes(source)
        campaign._sync_scratch_source(self.root, scratch, loaded, source)

        # Execute the producer from its campaign-CAS path, matching deployed
        # production behavior rather than the fixture's live source path.
        cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / loaded["measurement_producer"]["sha256"]
            / "owner_campaign_measure.py"
        )
        cas.parent.mkdir(parents=True, exist_ok=True)
        cas.write_bytes((self.root / "hook.py").read_bytes())
        loaded["_producer"] = cas.resolve()

        environments: list[dict[str, str]] = []
        original = campaign._run_bounded_process

        def capture(*args: object, **kwargs: object) -> object:
            environments.append(dict(kwargs["environment"]))
            return original(*args, **kwargs)

        inherited = {
            "PYTHONPATH": str(self.root / "untrusted-import-root"),
            "PYTHONNOUSERSITE": "untrusted-user-site",
        }
        with mock.patch.dict(os.environ, inherited):
            with mock.patch.object(campaign, "_run_bounded_process", side_effect=capture):
                campaign._run_hook(
                    self.root, scratch, loaded, "focus", source_sha, "candidate"
                )
                campaign._run_final_owner(
                    self.root, scratch, loaded, "focus", source_sha
                )

        self.assertEqual(len(environments), 2)
        expected_root = str(scratch.resolve())
        self.assertEqual(
            [environment["OWNER_CAMPAIGN_PHASE"] for environment in environments],
            ["candidate", "final_owner"],
        )
        for environment in environments:
            self.assertEqual(environment["PYTHONPATH"], expected_root)
            self.assertEqual(environment["PYTHONNOUSERSITE"], "1")
            self.assertNotIn("untrusted-import-root", environment["PYTHONPATH"])
            self.assertNotIn("untrusted-user-site", environment["PYTHONNOUSERSITE"])

    def test_exact_live_measurement_producer_wins_over_matching_cas(self) -> None:
        expected = self.manifest["measurement_producer"]["sha256"]
        cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        cas.parent.mkdir(parents=True, exist_ok=True)
        cas.write_bytes((self.root / "hook.py").read_bytes())

        loaded = self.load()

        self.assertEqual(loaded["_producer"], (self.root / "hook.py").resolve())

    def _use_untracked_deployed_producer(self) -> tuple[Path, str, bytes]:
        """Move the test binding to an untracked deployment path."""

        original = (self.root / "hook.py").read_bytes()
        live = self.root / "build" / "deployed-hook.py"
        live.write_bytes(original)
        body = {
            key: value for key, value in self.manifest.items()
            if key != "manifest_sha256"
        }
        expected = digest_bytes(original)
        body["measurement_producer"] = {
            "path": "build/deployed-hook.py", "sha256": expected
        }
        self.manifest = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(self.manifest), encoding="utf-8")
        return live, expected, original

    def test_drifted_live_producer_uses_exact_contained_cas_snapshot(self) -> None:
        live, expected, original = self._use_untracked_deployed_producer()
        live.write_bytes(original + b"\n# deployed update\n")
        cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        cas.parent.mkdir(parents=True, exist_ok=True)
        cas.write_bytes(original)

        loaded = self.load()

        self.assertEqual(loaded["_producer"], cas.resolve())

    def test_drifted_live_producer_requires_exact_cas_snapshot(self) -> None:
        live, expected, original = self._use_untracked_deployed_producer()
        live.write_bytes(original + b"\n# deployed update\n")
        cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        with self.assertRaisesRegex(campaign.CampaignError, "measurement producer hash drift"):
            self.load()

        cas.parent.mkdir(parents=True, exist_ok=True)
        cas.write_bytes(b"wrong producer snapshot")
        with self.assertRaisesRegex(campaign.CampaignError, "measurement producer hash drift"):
            self.load()

    def test_hook_rejects_replaced_cas_producer_before_subprocess(self) -> None:
        live, expected, original = self._use_untracked_deployed_producer()
        live.write_bytes(original + b"\n# deployed update\n")
        cas = (
            self.root / "build" / "owner-campaign" / "tool-cas"
            / expected / "owner_campaign_measure.py"
        )
        cas.parent.mkdir(parents=True, exist_ok=True)
        cas.write_bytes(original)
        loaded = self.load()
        self.assertEqual(loaded["_producer"], cas.resolve())

        # A directory at the already-loaded CAS path is the portable
        # non-regular/indirection adversarial replacement.  On platforms that
        # permit symlinks, the same gate also rejects a symlink via
        # ``_path_has_indirection``; this test avoids requiring elevated
        # symlink privileges on Windows.
        cas.unlink()
        cas.mkdir()
        launched = False

        def fail_if_launched(*args: object, **kwargs: object) -> object:
            nonlocal launched
            launched = True
            self.fail("measurement subprocess launched after CAS replacement")

        with mock.patch.object(campaign, "_run_bounded_process", side_effect=fail_if_launched):
            with self.assertRaisesRegex(campaign.InfrastructureError, "measurement_producer"):
                campaign._run_hook(
                    self.root,
                    campaign._ensure_scratch(self.root, loaded),
                    loaded,
                    "focus",
                    campaign._digest_bytes(self.source.read_bytes()),
                    "snapshot",
                )
        self.assertFalse(launched)

    def test_snapshot_rehashes_sources_immediately_before_publication(self) -> None:
        loaded = self.load()
        original = campaign._run_hook

        def drift_after_measurement(*args, **kwargs):
            measurement = original(*args, **kwargs)
            self.source.write_text(
                "int focus(void) { return 9; } /* DRIFT */\n", encoding="utf-8"
            )
            return measurement

        with mock.patch.object(campaign, "_run_hook", side_effect=drift_after_measurement):
            with self.assertRaisesRegex(campaign.CampaignError, "drifted before frontier"):
                campaign.snapshot_frontier(self.root, loaded, "focus", force=True)
        latest = campaign._function_root(self.root, loaded, "focus") / "latest-frontier.json"
        self.assertFalse(latest.exists())

    def test_manifest_requires_bound_producer_in_every_command(self) -> None:
        body = {
            key: value for key, value in self.manifest.items()
            if key != "manifest_sha256"
        }
        body["commands"] = dict(body["commands"])
        body["commands"]["candidate"] = dict(body["commands"]["candidate"])
        body["commands"]["candidate"]["argv"] = [sys.executable, "hook.py"]
        forged = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(forged), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "measurement producer"):
            self.load()

    def test_frontier_receipts_and_protected_census_fail_closed(self) -> None:
        loaded = self.load()
        campaign.snapshot_frontier(self.root, loaded, "focus")
        latest = campaign._function_root(self.root, loaded, "focus") / "latest-frontier.json"
        value = json.loads(latest.read_text(encoding="utf-8"))
        value["report_receipts"]["strict"] = "not-a-sha"
        value["frontier_sha256"] = campaign._digest_json({
            key: item for key, item in value.items() if key != "frontier_sha256"
        })
        latest.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "SHA-256"):
            campaign.campaign_status(self.root, loaded)

        value["report_receipts"]["strict"] = "0" * 64
        value["metrics"]["protected_total"] = 0
        value["frontier_sha256"] = campaign._digest_json({
            key: item for key, item in value.items() if key != "frontier_sha256"
        })
        latest.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "protected sibling census"):
            campaign.campaign_status(self.root, loaded)

    def test_measurement_binds_protected_function_identities(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        source = self.source.read_bytes()
        campaign._sync_scratch_source(self.root, scratch, loaded, source)
        measurement = campaign._run_hook(
            self.root, scratch, loaded, "focus", campaign._digest_bytes(source), "snapshot"
        )
        measurement["focus_evidence"]["sibling_identities"] = ["wrong-sibling"]
        focus_body = {
            key: item for key, item in measurement["focus_evidence"].items()
            if key != "focus_evidence_sha256"
        }
        measurement["focus_evidence"]["focus_evidence_sha256"] = campaign._digest_json(
            focus_body
        )
        measurement["measurement_sha256"] = campaign._digest_json({
            key: item for key, item in measurement.items() if key != "measurement_sha256"
        })
        with self.assertRaisesRegex(campaign.CampaignError, "identities"):
            campaign._validate_measurement(
                measurement, campaign=loaded, function="focus", phase="snapshot",
                source_sha256=campaign._digest_bytes(source),
            )

    def test_focus_in_protected_inventory_uses_sibling_census(self) -> None:
        loaded = self.load()
        loaded["protected_exact_functions"] = ["focus", "sibling"]
        scratch = campaign._ensure_scratch(self.root, loaded)
        source = self.source.read_bytes()
        source_sha = campaign._digest_bytes(source)
        campaign._sync_scratch_source(self.root, scratch, loaded, source)

        measurement = campaign._run_hook(
            self.root, scratch, loaded, "focus", source_sha, "snapshot"
        )

        self.assertEqual(measurement["metrics"]["protected_total"], 1)
        self.assertEqual(measurement["metrics"]["protected_losses"], 0)
        self.assertEqual(
            measurement["focus_evidence"]["protected_total"], 1
        )
        self.assertEqual(
            measurement["focus_evidence"]["sibling_identities"], ["sibling"]
        )

        frontier = campaign._frontier_from_measurement(
            loaded, "focus", measurement, parent=None
        )
        self.assertEqual(frontier["metrics"]["protected_total"], 1)
        self.assertEqual(
            campaign._validate_frontier(frontier, loaded, "focus"), frontier
        )

        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("EXACT", base, "focus-exact")
        )
        self.assertEqual(result["status"], "exact")
        report = json.loads(Path(result["exact"]["report_path"]).read_text())
        self.assertEqual(report["result"]["protected_total"], 1)
        self.assertEqual(
            report["evidence"]["protected_sibling_identities"], ["sibling"]
        )

    def test_final_owner_receipt_obeys_compact_limit(self) -> None:
        loaded = self.load()
        loaded["limits"] = {**loaded["limits"], "report_bytes": 32}
        scratch = campaign._ensure_scratch(self.root, loaded)
        source = self.source.read_bytes()
        campaign._sync_scratch_source(self.root, scratch, loaded, source)
        with self.assertRaisesRegex(campaign.InfrastructureError, "compact report"):
            campaign._run_final_owner(
                self.root, scratch, loaded, "focus", campaign._digest_bytes(source)
            )

    def test_final_owner_receipt_rejects_source_and_base_drift(self) -> None:
        loaded = self.load()
        scratch = campaign._ensure_scratch(self.root, loaded)
        source = self.source.read_bytes()
        source_sha = campaign._digest_bytes(source)
        campaign._sync_scratch_source(self.root, scratch, loaded, source)
        receipt = campaign._run_final_owner(self.root, scratch, loaded, "focus", source_sha)
        for field, replacement in (
            ("source_path", "src/wrong.c"), ("base_commit", "0" * 40),
        ):
            drifted = dict(receipt)
            drifted[field] = replacement
            body = {key: value for key, value in drifted.items() if key != "final_owner_sha256"}
            drifted["final_owner_sha256"] = campaign._digest_json(body)
            with self.assertRaisesRegex(campaign.CampaignError, "owner closure"):
                campaign._validate_final_owner_receipt(drifted, loaded, source_sha)

    def test_stale_inflight_reservation_is_reclaimed(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", base, "expired")
        candidate = campaign._load_candidate(self.root, path, loaded, base)
        key = campaign._candidate_key(loaded, base, candidate)
        record = campaign._dedupe_record(
            key=key, function="focus", frontier=base,
            candidate_source_sha256=candidate["_source_sha256"], status="inflight",
        )
        record["finished_at"] = "2000-01-01T00:00:00Z"
        record["result_sha256"] = campaign._digest_json({
            key: item for key, item in record.items() if key != "result_sha256"
        })
        dedupe = campaign._dedupe_path(self.root, loaded, "focus")
        campaign._write_dedupe(loaded, dedupe, [record])
        result = campaign.run_candidate(self.root, loaded, path)
        self.assertEqual(result["status"], "no_gain")

    def test_focus_cas_only_tracks_published_frontiers_and_terminal_inputs_clean(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        focus_root = self.root / "build" / "owner-campaign" / "proof-cas" / "focus"
        self.assertEqual(len(list(focus_root.rglob("*.json"))), 1)
        cell = self.candidate("REGRESS", base, "cleanup")
        source_path = self.root / "build" / "candidates" / "cleanup.c"
        self.assertEqual(campaign.run_candidate(self.root, loaded, cell)["status"], "no_gain")
        self.assertFalse(cell.exists())
        self.assertFalse(source_path.exists())
        self.assertEqual(len(list(focus_root.rglob("*.json"))), 1)

        gain = self.candidate("IMPROVE", base, "focus-gain")
        self.assertEqual(campaign.run_candidate(self.root, loaded, gain)["status"], "improved")
        # The displaced blob is marked during the candidate's maintenance pass
        # and survives one reader grace generation.  A later pass may collect
        # it after rechecking the retained frontier.
        campaign._gc_focus_evidence(self.root, minimum_age_seconds=0)
        blobs = list(focus_root.rglob("*.json"))
        self.assertEqual(len(blobs), 1)
        self.assertNotEqual(blobs[0].stem, base["focus_evidence_sha256"])

    def test_gc_scan_does_not_hold_focus_publication_lock(self) -> None:
        loaded = self.load()
        entered = threading.Event()
        release = threading.Event()
        errors: list[BaseException] = []

        def blocked_gc(
            root: Path, *, minimum_age_seconds: float = 0,
        ) -> None:
            entered.set()
            if not release.wait(timeout=5):
                raise AssertionError("GC scan release timed out")

        def maintain() -> None:
            try:
                campaign._check_limits(self.root, loaded)
            except BaseException as exc:
                errors.append(exc)

        with mock.patch.object(
            campaign, "_gc_focus_evidence", side_effect=blocked_gc
        ):
            worker = threading.Thread(target=maintain)
            worker.start()
            self.assertTrue(entered.wait(timeout=5))
            with campaign._exclusive_lock(
                self.root / "build" / "owner-campaign" / "proof-cas"
                / "focus-cas.lock",
                0.5,
            ):
                pass
            release.set()
            worker.join(timeout=5)
        self.assertFalse(worker.is_alive())
        self.assertEqual(errors, [])

    def test_concurrent_gc_scan_cannot_block_or_delete_snapshot_publication(self) -> None:
        loaded = self.load()
        entered = threading.Event()
        release = threading.Event()
        snapshot_done = threading.Event()
        errors: list[BaseException] = []
        original_references = campaign._evidence_gc_references

        def blocked_references(
            root: Path, digest_field: str, label: str,
        ) -> set[str] | None:
            if digest_field == "focus_evidence_sha256" and not entered.is_set():
                entered.set()
                if not release.wait(timeout=5):
                    raise AssertionError("reference scan release timed out")
            return original_references(root, digest_field, label)

        def maintain() -> None:
            try:
                campaign._check_limits(self.root, loaded)
            except BaseException as exc:
                errors.append(exc)

        published: list[dict[str, object]] = []

        def snapshot() -> None:
            try:
                published.append(campaign.snapshot_frontier(
                    self.root, loaded, "focus", _defer_maintenance=True
                ))
            except BaseException as exc:
                errors.append(exc)
            finally:
                snapshot_done.set()

        with mock.patch.object(
            campaign, "_evidence_gc_references", side_effect=blocked_references
        ):
            gc_worker = threading.Thread(target=maintain)
            gc_worker.start()
            self.assertTrue(entered.wait(timeout=5))
            publisher = threading.Thread(target=snapshot)
            publisher.start()
            self.assertTrue(snapshot_done.wait(timeout=5))
            self.assertEqual(len(published), 1)
            release.set()
            publisher.join(timeout=5)
            gc_worker.join(timeout=5)
        self.assertFalse(publisher.is_alive())
        self.assertFalse(gc_worker.is_alive())
        self.assertEqual(errors, [])
        digest = published[0]["focus_evidence_sha256"]
        blob = (
            self.root / "build" / "owner-campaign" / "proof-cas" / "focus"
            / str(digest)[:2] / f"{digest}.json"
        )
        self.assertTrue(blob.is_file())

    def test_gain_rejects_code_size_and_relocation_count_regressions(self) -> None:
        base = {
            "strict": {"target_bytes": 100, "candidate_bytes": 100, "differences": 10},
            "data": {"target_bytes": 100, "candidate_bytes": 100, "differences": 10},
            "physical_target_count": 5, "physical_candidate_count": 5,
            "physical_differences": 2, "protected_total": 1,
            "protected_losses": 0, "source_link_exact": False,
        }
        code_growth = json.loads(json.dumps(base))
        code_growth["strict"]["differences"] = 8
        code_growth["strict"]["candidate_bytes"] = 116
        self.assertEqual(campaign.assess_gain(base, code_growth), "no_gain")
        relocation_growth = json.loads(json.dumps(base))
        relocation_growth["strict"]["differences"] = 8
        relocation_growth["physical_candidate_count"] = 7
        self.assertEqual(campaign.assess_gain(base, relocation_growth), "no_gain")

    def test_exact_publishes_compact_report_cas_and_manifest(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(self.root, loaded, self.candidate("EXACT", base))
        self.assertEqual(result["status"], "exact")
        receipt = result["exact"]
        self.assertIsNotNone(receipt)
        report = Path(receipt["report_path"])
        self.assertTrue(report.is_file())
        self.assertLessEqual(report.stat().st_size, 64 << 10)
        status = campaign.campaign_status(self.root, loaded)
        self.assertEqual((status["exact_count"], status["total"]), (1, 2))

    def test_terminal_progress_reads_only_valid_exact_manifest(self) -> None:
        loaded = self.load()
        self.assertEqual(
            campaign.campaign_terminal_progress(self.root, loaded),
            {"exact_count": 0, "total": 2, "closed": False},
        )
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("EXACT", base, "progress")
        )
        Path(result["exact"]["report_path"]).unlink()
        self.assertEqual(
            campaign.campaign_terminal_progress(self.root, loaded),
            {"exact_count": 1, "total": 2, "closed": False},
        )

    def test_terminal_progress_fails_closed_on_self_hashed_malformed_manifest(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        campaign.run_candidate(
            self.root, loaded, self.candidate("EXACT", base, "bad-progress")
        )
        path = (
            self.root / "build" / "owner-campaign" / "owners"
            / campaign._slug("main:test/owner") / "exact-manifest.json"
        )
        value = json.loads(path.read_text())
        value["exact"]["focus"]["report_sha256"] = "not-a-sha"
        body = {key: item for key, item in value.items() if key != "exact_manifest_sha256"}
        value["exact_manifest_sha256"] = campaign._digest_json(body)
        path.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "report_sha256"):
            campaign.campaign_terminal_progress(self.root, loaded)

    def test_terminal_progress_reports_closed_single_function_campaign(self) -> None:
        body = {
            key: value for key, value in self.manifest.items()
            if key != "manifest_sha256"
        }
        body["functions"] = ["focus"]
        body["protected_exact_functions"] = []
        self.manifest = seal(body, "manifest_sha256")
        self.manifest_path.write_text(json.dumps(self.manifest), encoding="utf-8")
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        result = campaign.run_candidate(
            self.root, loaded, self.candidate("EXACT", base, "closed-progress")
        )
        self.assertEqual(result["status"], "exact")
        self.assertEqual(
            campaign.campaign_terminal_progress(self.root, loaded),
            {"exact_count": 1, "total": 1, "closed": True},
        )

    def test_pending_frontier_recovers_after_source_cas(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate_path = self.candidate("IMPROVE", base, "pending")
        candidate_value = campaign._load_candidate(self.root, candidate_path, loaded, base)
        scratch = campaign._ensure_scratch(self.root, loaded)
        campaign._sync_scratch_source(self.root, scratch, loaded, candidate_value["_source_bytes"])
        measurement = campaign._run_hook(self.root, scratch, loaded, "focus", candidate_value["_source_sha256"], "candidate")
        frontier = campaign._frontier_from_measurement(loaded, "focus", measurement, parent=base)
        pending_body = {
            "schema": campaign.PENDING_SCHEMA, "base_source_sha256": base["source_sha256"],
            "candidate_source_sha256": candidate_value["_source_sha256"], "frontier": frontier,
            "exact_report": None,
            "final_owner_receipt": None,
        }
        pending = {**pending_body, "pending_sha256": campaign._digest_json(pending_body)}
        directory = campaign._function_root(self.root, loaded, "focus")
        campaign._atomic_json(directory / "frontier.pending.json", pending)
        campaign._atomic_bytes(self.source, candidate_value["_source_bytes"])
        recovered = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(recovered["frontier_sha256"], frontier["frontier_sha256"])
        self.assertFalse((directory / "frontier.pending.json").exists())

    def test_cancel_epoch_stops_hot_loop_without_global_stop(self) -> None:
        loaded = self.load()
        campaign.cancel_campaign(self.root, loaded, 1)
        with self.assertRaisesRegex(campaign.CampaignError, "cancelled"):
            campaign.snapshot_frontier(self.root, loaded, "focus")

    def test_forbidden_construct_fails_before_compile(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("IMPROVE register", base, "forbidden")
        before = (self.root / "build" / "invocations.log").read_text().splitlines()
        with self.assertRaisesRegex(campaign.CampaignError, "forbidden"):
            campaign.run_candidate(self.root, loaded, path)
        after = (self.root / "build" / "invocations.log").read_text().splitlines()
        self.assertEqual(before, after)

    def test_candidate_edit_outside_claimed_function_span_fails_before_compile(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("IMPROVE", base, "escape")
        descriptor = json.loads(path.read_text(encoding="utf-8"))
        source_path = self.root / descriptor["candidate_source"]["path"]
        source_path.write_bytes(source_path.read_bytes() + b"int outside = 1;\r\n")
        descriptor["candidate_source"]["sha256"] = campaign._digest_file(source_path)
        descriptor["candidate_sha256"] = campaign._digest_json({
            key: value for key, value in descriptor.items() if key != "candidate_sha256"
        })
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        before = len((self.root / "build" / "invocations.log").read_text().splitlines())
        with self.assertRaisesRegex(campaign.CampaignError, "function span"):
            campaign.run_candidate(self.root, loaded, path)
        after = len((self.root / "build" / "invocations.log").read_text().splitlines())
        self.assertEqual(before, after)

    def test_candidate_requires_immutable_base_source_binding(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", frontier, "missing-base")
        descriptor = json.loads(path.read_text(encoding="utf-8"))
        descriptor.pop("base_source")
        descriptor["candidate_sha256"] = campaign._digest_json({
            key: value for key, value in descriptor.items()
            if key != "candidate_sha256"
        })
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "strict closed object"):
            campaign._load_candidate(self.root, path, loaded, frontier)

    def test_candidate_rejects_base_source_hash_and_frontier_drift(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", frontier, "base-drift")
        descriptor = json.loads(path.read_text(encoding="utf-8"))
        base_path = self.root / descriptor["base_source"]["path"]
        base_path.write_text("int focus(void) { return 7; }\n", encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "base source hash drift"):
            campaign._load_candidate(self.root, path, loaded, frontier)

        descriptor["base_source"]["sha256"] = campaign._digest_file(base_path)
        descriptor["candidate_sha256"] = campaign._digest_json({
            key: value for key, value in descriptor.items()
            if key != "candidate_sha256"
        })
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "match the frontier"):
            campaign._load_candidate(self.root, path, loaded, frontier)

    def test_candidate_rejects_base_source_path_escape(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", frontier, "base-escape")
        descriptor = json.loads(path.read_text(encoding="utf-8"))
        descriptor["base_source"]["path"] = "../outside.c"
        descriptor["candidate_sha256"] = campaign._digest_json({
            key: value for key, value in descriptor.items()
            if key != "candidate_sha256"
        })
        path.write_text(json.dumps(descriptor), encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "escapes the campaign root"):
            campaign._load_candidate(self.root, path, loaded, frontier)

    def test_candidate_rebase_depth_is_bounded_integer(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        for index, depth in enumerate((-1, 6, True, "0")):
            with self.subTest(depth=depth):
                path = self.candidate("REGRESS", frontier, f"depth-{index}")
                descriptor = json.loads(path.read_text(encoding="utf-8"))
                descriptor["rebase_depth"] = depth
                descriptor["candidate_sha256"] = campaign._digest_json({
                    key: value for key, value in descriptor.items()
                    if key != "candidate_sha256"
                })
                path.write_text(json.dumps(descriptor), encoding="utf-8")
                with self.assertRaisesRegex(campaign.CampaignError, "rebase_depth"):
                    campaign._load_candidate(self.root, path, loaded, frontier)

    def test_candidate_load_separately_rejects_live_source_frontier_drift(self) -> None:
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        path = self.candidate("REGRESS", frontier, "live-drift")
        self.source.write_text("int focus(void) { return 9; }\n", encoding="utf-8")
        with self.assertRaisesRegex(campaign.CampaignError, "live source"):
            campaign._load_candidate(self.root, path, loaded, frontier)

    def test_full_function_replacement_over_80_lines_is_admitted_inside_span(self) -> None:
        """A real full-body recovery must not be rejected as an oversized hunk."""
        loaded = self.load()
        frontier = campaign.snapshot_frontier(self.root, loaded, "focus")
        candidate_source = self.root / "build" / "candidates" / "full-function.c"
        base_source = self.root / "build" / "candidates" / "full-function.base.c"
        candidate_source.parent.mkdir(parents=True, exist_ok=True)
        base_source.write_bytes(self.source.read_bytes())
        candidate_lines = ["int focus(void) {\n"]
        candidate_lines.extend(
            f"    int value_{index} = {index};\n" for index in range(90)
        )
        candidate_lines.extend(["    return value_89;\n", "} /* IMPROVE */\n"])
        candidate_source.write_text("".join(candidate_lines), encoding="utf-8")
        candidate_bytes = candidate_source.read_bytes()
        base_bytes = base_source.read_bytes()
        descriptor_body: dict[str, object] = {
            "schema": "owner_campaign_candidate/v1",
            "campaign_id": "test-owner-v1",
            "function": "focus",
            "base_frontier_sha256": frontier["frontier_sha256"],
            "base_source": {
                "path": base_source.relative_to(self.root).as_posix(),
                "sha256": campaign._digest_file(base_source),
            },
            "candidate_source": {
                "path": candidate_source.relative_to(self.root).as_posix(),
                "sha256": campaign._digest_file(candidate_source),
            },
            "function_span": {
                "base_start_line": 1,
                "base_end_line": len(base_bytes.splitlines()),
                "candidate_start_line": 1,
                "candidate_end_line": len(candidate_lines),
                "base_sha256": digest_bytes(base_bytes),
                "candidate_sha256": digest_bytes(candidate_bytes),
            },
            "hypothesis_family": "full-function-body-recovery",
            "natural_c": True,
            "rebase_depth": 0,
            "created_at": "2026-08-31T00:00:00Z",
        }
        descriptor = seal(descriptor_body, "candidate_sha256")
        descriptor_path = self.root / "build" / "candidates" / "full-function.json"
        descriptor_path.write_text(json.dumps(descriptor), encoding="utf-8")

        admitted = campaign._load_candidate(
            self.root, descriptor_path, loaded, frontier
        )
        self.assertGreater(len(candidate_lines), 80)
        self.assertEqual(admitted["_source_bytes"], candidate_bytes)
        self.assertEqual(admitted["_source_sha256"], digest_bytes(candidate_bytes))

    def test_parallel_candidates_use_isolated_workers_and_one_cas_winner(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        campaign._ensure_scratch(self.root, loaded, 1)
        first = self.candidate("IMPROVE SLOW A", base, "parallel-a")
        second = self.candidate("IMPROVE SLOW B", base, "parallel-b")
        results = campaign.run_loop(self.root, loaded, [first, second])
        self.assertEqual(
            {item["status"] for item in results}, {"improved", "stale_rebase"}
        )
        barrier = self.root / "build" / "parallel-barrier"
        self.assertEqual(len(list(barrier.iterdir())), 2)
        scratch = (
            self.root / "build" / "owner-campaign" / "scratch"
            / campaign._slug("test-owner-v1")
        )
        repos = sorted(path.name for path in scratch.glob("repo-*") if path.is_dir())
        self.assertEqual(repos, ["repo-0", "repo-1"])
        self.assertLessEqual(len(repos), 5)
        latest = campaign.snapshot_frontier(self.root, loaded, "focus")
        self.assertEqual(latest["generation"], 1)
        self.assertEqual(latest["source_sha256"], campaign._digest_file(self.source))
        stale = next(item for item in results if item["status"] == "stale_rebase")
        self.assertTrue((self.root / stale["rebase_input"]["descriptor_path"]).is_file())
        self.assertTrue((self.root / stale["rebase_input"]["candidate_source_path"]).is_file())
        self.assertTrue((self.root / stale["rebase_input"]["base_source_path"]).is_file())
        self.assertEqual(stale["rebase_input"]["rebase_depth"], 0)

    def test_retained_gain_survives_cleanup_failure_with_sealed_outcome(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        with mock.patch.object(
            campaign, "_cleanup_candidate_artifacts",
            side_effect=OSError("cleanup sentinel"),
        ):
            result = campaign.run_candidate(
                self.root, loaded, self.candidate("IMPROVE", base, "cleanup-fail")
            )
        self.assertEqual(result["status"], "improved")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertIn("cleanup sentinel", result["cleanup_errors"][0])
        self.assertEqual(result["result_sha256"], campaign._digest_json({
            key: value for key, value in result.items() if key != "result_sha256"
        }))
        self.assertIn("IMPROVE", self.source.read_text())

    def test_exact_report_survives_cleanup_failure(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        with mock.patch.object(
            campaign, "_cleanup_candidate_artifacts",
            side_effect=OSError("exact cleanup sentinel"),
        ):
            result = campaign.run_candidate(
                self.root, loaded, self.candidate("EXACT", base, "exact-cleanup-fail")
            )
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["cleanup_status"], "cleanup_incomplete")
        self.assertIsNotNone(result["exact"])
        self.assertTrue(Path(result["exact"]["report_path"]).is_file())
        self.assertEqual(result["result_sha256"], campaign._digest_json({
            key: value for key, value in result.items() if key != "result_sha256"
        }))

    def test_timeout_kills_descendants_and_peak_storage_is_enforced(self) -> None:
        scratch = self.root / "build" / "bounded"
        temporary = scratch / "temp"
        temporary.mkdir(parents=True)
        marker = scratch / "descendant.txt"
        child = f"import pathlib,time;time.sleep(0.8);pathlib.Path({str(marker)!r}).write_text('late')"
        parent = f"import subprocess,sys,time;subprocess.Popen([sys.executable,'-c',{child!r}]);time.sleep(10)"
        with self.assertRaisesRegex(campaign.InfrastructureError, "timed out"):
            campaign._run_bounded_process(
                [sys.executable, "-c", parent], cwd=scratch,
                environment={}, timeout=0.2, scratch=scratch,
                temporary_root=temporary, scratch_hard_bytes=8 << 20,
                cell_temporary_bytes=4 << 20,
            )
        time.sleep(1.0)
        self.assertFalse(marker.exists(), "timed-out descendant survived")

        writer = "import pathlib,time;pathlib.Path('temp/large.bin').write_bytes(b'x'*(2<<20));time.sleep(10)"
        with self.assertRaisesRegex(campaign.InfrastructureError, "temporary storage"):
            campaign._run_bounded_process(
                [sys.executable, "-c", writer], cwd=scratch,
                environment={}, timeout=5, scratch=scratch,
                temporary_root=temporary, scratch_hard_bytes=8 << 20,
                cell_temporary_bytes=1 << 20,
            )

        loaded = self.load()
        owner = campaign._owner_root(self.root, loaded)
        owner.mkdir(parents=True, exist_ok=True)
        (owner / "existing.bin").write_bytes(b"x" * 64)
        loaded["limits"] = {**loaded["limits"], "owner_state_bytes": 72}
        with self.assertRaisesRegex(campaign.CampaignError, "peak hard limit"):
            campaign._ensure_state_write_peak(
                self.root, loaded, [(owner / "new.json", b"0123456789")]
            )

    def test_gain_rejects_row_and_physical_identity_migration(self) -> None:
        metrics = {
            "strict": {"target_bytes": 100, "candidate_bytes": 100, "differences": 2},
            "data": {"target_bytes": 100, "candidate_bytes": 100, "differences": 2},
            "physical_target_count": 5, "physical_candidate_count": 5,
            "physical_differences": 1, "protected_total": 1,
            "protected_losses": 0, "source_link_exact": False,
        }
        candidate = json.loads(json.dumps(metrics))
        candidate["strict"]["differences"] = 1
        candidate["data"]["differences"] = 1
        base_focus = {
            "strict_row_ids": ["s:a", "s:b"], "data_row_ids": ["d:a", "d:b"],
            "physical_difference_ids": ["p:old"],
            "physical_target_identity_sha256": "1" * 64,
            "physical_candidate_identity_sha256": "3" * 64,
        }
        migrated = {
            "strict_row_ids": ["s:new"], "data_row_ids": ["d:new"],
            "physical_difference_ids": ["p:new"],
            "physical_target_identity_sha256": "1" * 64,
            "physical_candidate_identity_sha256": "4" * 64,
        }
        self.assertEqual(
            campaign.assess_gain(metrics, candidate, base_focus=base_focus, candidate_focus=migrated),
            "no_gain",
        )
        migrated = {
            "strict_row_ids": ["s:a"], "data_row_ids": ["d:a"],
            "physical_difference_ids": ["p:old"],
            "physical_target_identity_sha256": "2" * 64,
            "physical_candidate_identity_sha256": "3" * 64,
        }
        self.assertEqual(
            campaign.assess_gain(metrics, candidate, base_focus=base_focus, candidate_focus=migrated),
            "no_gain",
        )
        same_rows_migrated = {
            "strict_row_ids": ["s:a"], "data_row_ids": ["d:a"],
            "physical_difference_ids": ["p:old"],
            "physical_target_identity_sha256": "1" * 64,
            "physical_candidate_identity_sha256": "4" * 64,
        }
        self.assertEqual(
            campaign.assess_gain(metrics, candidate, base_focus=base_focus, candidate_focus=same_rows_migrated),
            "no_gain",
        )
        closed_physical = {
            **same_rows_migrated,
            "physical_difference_ids": [],
            "physical_candidate_identity_sha256": "1" * 64,
        }
        closed_candidate = json.loads(json.dumps(candidate))
        closed_candidate["physical_differences"] = 0
        self.assertEqual(
            campaign.assess_gain(metrics, closed_candidate, base_focus=base_focus, candidate_focus=closed_physical),
            "improved",
        )

        exact_candidate = json.loads(json.dumps(closed_candidate))
        exact_candidate["strict"]["differences"] = 0
        exact_candidate["data"]["differences"] = 0
        exact_candidate["source_link_exact"] = True
        exact_focus = {
            **closed_physical,
            "strict_row_ids": [],
            "data_row_ids": [],
        }
        self.assertEqual(
            campaign.assess_gain(
                metrics, exact_candidate,
                base_focus=base_focus, candidate_focus=exact_focus,
            ),
            "exact",
        )

    def test_identical_parallel_candidate_compiles_once(self) -> None:
        loaded = self.load()
        base = campaign.snapshot_frontier(self.root, loaded, "focus")
        first = self.candidate("REGRESS", base, "duplicate-a")
        second = self.candidate("REGRESS", base, "duplicate-b")
        results = campaign.run_loop(self.root, loaded, [first, second])
        self.assertEqual(
            sorted(item["status"] for item in results),
            ["deduplicated", "no_gain"],
        )
        phases = [
            line.split(":", 1)[0]
            for line in (self.root / "build" / "invocations.log").read_text().splitlines()
        ]
        self.assertEqual(phases.count("candidate"), 1)


if __name__ == "__main__":
    unittest.main()
