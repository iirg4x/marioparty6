from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

from tools import owner_campaign_measure as adapter
from tools import owner_campaign_reconstruction as reconstruction
from tools.owner_campaign_verify import VerificationError, verify_measurement, verify_report


def _sha(value: object) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


def _focus(*, differences: int = 0) -> dict[str, object]:
    rows = [
        {
            "index": index,
            "diff_kind": "DIFF_ARG_MISMATCH",
            "instruction": {"address": 100 + index, "formatted": "op r3,r4"},
        }
        for index in range(differences)
    ]
    channel = {
        "metric": {"target_size": 32, "candidate_size": 32, "diff_rows": differences},
        "target": {"rows": copy.deepcopy(rows), "instruction_count": 8},
        "candidate": {"rows": copy.deepcopy(rows), "instruction_count": 8},
        "protected_siblings": {
            "sibling_count": 1,
            "exact_sibling_count": 1,
            "exact_identities": ["sibling"],
        },
    }
    relocations = [
        {"offset": 16, "type": "SDA21", "effective_target": "@1"},
        {"offset": 20, "type": "REL24", "effective_target": "fn"},
    ]
    return {
        "channels": {"strict": channel, "data": copy.deepcopy(channel)},
        "physical_relocations": {
            "target": {
                "physical_relocation_count": 2,
                "physical_relocations": copy.deepcopy(relocations),
            },
            "candidate": {
                "physical_relocation_count": 2,
                "physical_relocations": copy.deepcopy(relocations),
            },
            "physical_relocation_differences": [],
        },
    }


def _identity() -> adapter.Identity:
    return adapter.Identity(
        phase="candidate",
        campaign_id="campaign",
        manifest_sha256="a" * 64,
        owner="main:board/test",
        unit="main/board/test",
        function="fn",
        source_sha256="b" * 64,
        target_object_sha256="c" * 64,
        toolchain_sha256="d" * 64,
        base_commit="e" * 40,
        source_path="src/test.c",
    )


def _measurement(*, exact: bool) -> dict[str, object]:
    with tempfile.TemporaryDirectory() as raw:
        root = Path(raw)
        candidate = root / "candidate.o"
        candidate.write_bytes(b"candidate")
        strict = root / "strict.json"
        data = root / "data.json"
        physical = root / "physical.json"
        strict.write_text("strict", encoding="utf-8")
        data.write_text("data", encoding="utf-8")
        physical.write_text("physical", encoding="utf-8")
        args = adapter._parser().parse_args(["--protected-total", "1"])
        kwargs: dict[str, object] = {}
        if exact:
            command = "mwcc -o build/test.o src/test.c"
            proof = {
                "schema": "owner_campaign_source_compile_proof/v1",
                "source_path": "src/test.c",
                "source_sha256": "b" * 64,
                "candidate_object_path": "build/test.o",
                "candidate_object_sha256": hashlib.sha256(b"candidate").hexdigest(),
                "compiler_commands": [command],
                "paired_compile_command_sha256": adapter._sha_bytes(command.encode()),
                "object_origin": "reconstructed_source",
                "fallback_asm_used": False,
                "nonmatching_fallback_linked": False,
                "authority_advanced": False,
            }
            kwargs.update(source_link_exact=True, source_link_proof=adapter._seal(proof, "proof_sha256"))
        return adapter._measurement(
            _identity(), _focus(differences=0 if exact else 1), args,
            strict_path=strict, data_path=data, physical_path=physical,
            candidate=candidate, **kwargs,
        )


class OwnerCampaignVerifyTests(unittest.TestCase):
    def _with_reconstruction(self, value: dict[str, object]) -> dict[str, object]:
        focus = _focus(differences=1)
        focus["schema"] = "focus_symbol_report/v1"
        focus["strict_row_ids"] = value["focus_evidence"]["strict_row_ids"]
        focus["data_row_ids"] = value["focus_evidence"]["data_row_ids"]
        packet = reconstruction.build_packet(
            focus,
            {
                "owner": value["owner"], "unit": value["unit"],
                "function": value["function"], "source_path": value["source_path"],
                "source_sha256": value["source_sha256"],
                "frontier_source_sha256": value["source_sha256"],
                "base_commit": value["base_commit"],
                "target_object_sha256": value["target_object_sha256"],
                "candidate_object_sha256": value["candidate_object_sha256"],
                "toolchain_sha256": value["toolchain_sha256"],
            },
            {"function": "fn", "start_line": 1, "end_line": 1,
             "start_offset": 0, "end_offset": 1, "span_sha256": "f" * 64},
        )
        result = copy.deepcopy(value)
        result["reconstruction_evidence"] = packet
        unsigned = dict(result)
        unsigned.pop("measurement_sha256")
        result["measurement_sha256"] = _sha(unsigned)
        return result

    def test_snapshot_pending_source_link_is_explicitly_valid(self) -> None:
        value = _measurement(exact=False)
        checked = verify_measurement(value)
        self.assertTrue(checked["verified"])
        self.assertFalse(value["metrics"]["source_link_exact"])

    def test_exact_measurement_verifies_all_embedded_proofs(self) -> None:
        value = _measurement(exact=True)
        checked = verify_measurement(value)
        self.assertTrue(checked["verified"])
        self.assertEqual(checked["measurement_sha256"], value["measurement_sha256"])
        self.assertEqual(checked["focus_evidence_sha256"], value["focus_evidence"]["focus_evidence_sha256"])

    def test_tampered_reconstruction_packet_is_rejected_even_when_measurement_resealed(self) -> None:
        value = self._with_reconstruction(_measurement(exact=False))
        forged = copy.deepcopy(value)
        packet = dict(forged["reconstruction_evidence"])
        packet["candidate_object_sha256"] = "9" * 64
        forged["reconstruction_evidence"] = reconstruction.seal(packet)
        unsigned = dict(forged)
        unsigned.pop("measurement_sha256")
        forged["measurement_sha256"] = _sha(unsigned)
        with self.assertRaisesRegex(VerificationError, "identity is not measurement-bound"):
            verify_measurement(forged)

    def test_candidate_address_change_does_not_migrate_target_identity(self) -> None:
        identity = _identity()
        args = adapter._parser().parse_args(["--protected-total", "1"])
        first = adapter._focus_evidence(identity, _focus(differences=1), args)
        changed = _focus(differences=1)
        changed["channels"]["strict"]["candidate"]["rows"][0]["instruction"]["address"] = 0x4000
        second = adapter._focus_evidence(identity, changed, args)
        self.assertEqual(first["strict_row_ids"], second["strict_row_ids"])

    def test_relocation_identity_digest_is_order_independent_and_value_bound(self) -> None:
        identity = _identity()
        args = adapter._parser().parse_args(["--protected-total", "1"])
        first = adapter._focus_evidence(identity, _focus(), args)
        changed = _focus()
        changed["physical_relocations"]["target"]["physical_relocations"].reverse()
        changed["physical_relocations"]["candidate"]["physical_relocations"].reverse()
        reordered = adapter._focus_evidence(identity, changed, args)
        self.assertEqual(first["physical_target_identity_sha256"], reordered["physical_target_identity_sha256"])
        changed["physical_relocations"]["candidate"]["physical_relocations"][0]["effective_target"] = "@changed"
        altered = adapter._focus_evidence(identity, changed, args)
        self.assertNotEqual(first["physical_candidate_identity_sha256"], altered["physical_candidate_identity_sha256"])

    def test_measurement_focus_and_metric_drift_is_rejected_after_resealing(self) -> None:
        value = _measurement(exact=True)
        forged = copy.deepcopy(value)
        forged["metrics"]["strict"]["differences"] = 1
        unsigned = dict(forged)
        unsigned.pop("measurement_sha256")
        forged["measurement_sha256"] = _sha(unsigned)
        with self.assertRaisesRegex(VerificationError, "metric drifted"):
            verify_measurement(forged)

    def test_measurement_base_commit_drift_is_rejected_even_when_resealed(self) -> None:
        value = _measurement(exact=True)
        forged = copy.deepcopy(value)
        forged["base_commit"] = "f" * 40
        unsigned = dict(forged)
        unsigned.pop("measurement_sha256")
        forged["measurement_sha256"] = _sha(unsigned)
        with self.assertRaisesRegex(VerificationError, "focus evidence identity"):
            verify_measurement(forged)

    def test_measurement_wrong_phase_is_rejected_even_when_resealed(self) -> None:
        value = _measurement(exact=False)
        forged = copy.deepcopy(value)
        forged["phase"] = "diagnostic"
        unsigned = dict(forged)
        unsigned.pop("measurement_sha256")
        forged["measurement_sha256"] = _sha(unsigned)
        with self.assertRaisesRegex(VerificationError, "phase is invalid"):
            verify_measurement(forged)

    def test_measurement_extra_metric_is_rejected_even_when_resealed(self) -> None:
        value = _measurement(exact=False)
        forged = copy.deepcopy(value)
        forged["metrics"]["forged"] = 1
        unsigned = dict(forged)
        unsigned.pop("measurement_sha256")
        forged["measurement_sha256"] = _sha(unsigned)
        with self.assertRaisesRegex(VerificationError, "metrics have noncanonical"):
            verify_measurement(forged)

    def test_exact_report_binds_focus_counts_and_proof_bodies(self) -> None:
        measurement = _measurement(exact=True)
        focus = measurement["focus_evidence"]
        metrics = measurement["metrics"]
        receipts = measurement["report_receipts"]
        evidence = {
            "schema": "owner_campaign_report_evidence/v1",
            "owner": measurement["owner"],
            "function": measurement["function"],
            "unit": measurement["unit"],
            "source_path": measurement["source_path"],
            "base_commit": measurement["base_commit"],
            "source_sha256": measurement["source_sha256"],
            "target_object_sha256": measurement["target_object_sha256"],
            "candidate_object_sha256": measurement["candidate_object_sha256"],
            "focus_evidence_sha256": focus["focus_evidence_sha256"],
            "strict_row_count": focus["strict_row_count"],
            "strict_row_ids_sha256": focus["strict_row_ids_sha256"],
            "data_row_count": focus["data_row_count"],
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": focus["physical_target_count"],
            "physical_candidate_count": focus["physical_candidate_count"],
            "physical_difference_count": focus["physical_difference_count"],
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": focus["protected_total"],
            "protected_losses": focus["protected_losses"],
            "protected_sibling_identities": focus["sibling_identities"],
            "protected_sibling_digest": focus["sibling_digest"],
            "proofs": measurement["proofs"],
        }
        result = {
            "strict_percent": 100,
            "data_percent": 100,
            "target_bytes": metrics["strict"]["target_bytes"],
            "candidate_bytes": metrics["strict"]["candidate_bytes"],
            "strict_difference_count": 0,
            "data_difference_count": 0,
            "strict_row_ids_sha256": focus["strict_row_ids_sha256"],
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": metrics["physical_target_count"],
            "physical_candidate_count": metrics["physical_candidate_count"],
            "physical_difference_count": 0,
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": metrics["protected_total"],
            "protected_losses": 0,
            "protected_sibling_digest": focus["sibling_digest"],
            "source_link_exact": True,
        }
        body = {
            "schema": "CRACK_REPORT/v1",
            "status": "exact",
            "completed": True,
            "authority_advanced": False,
            "owner": measurement["owner"],
            "function": measurement["function"],
            "campaign_id": measurement["campaign_id"],
            "manifest_sha256": measurement["manifest_sha256"],
            "unit": measurement["unit"],
            "source_path": measurement["source_path"],
            "base_commit": measurement["base_commit"],
            "frontier_sha256": "1" * 64,
            "source_sha256": measurement["source_sha256"],
            "target_object_sha256": measurement["target_object_sha256"],
            "candidate_object_sha256": measurement["candidate_object_sha256"],
            "toolchain_sha256": measurement["toolchain_sha256"],
            "result": result,
            "proof_receipts": {
                "source_link": receipts["source_link"],
                "object": receipts["object"],
                "toolchain": receipts["toolchain"],
                "strict": receipts["strict"],
                "data": receipts["data"],
            },
            "evidence": evidence,
            "completed_at": "2026-08-31T00:00:00Z",
        }
        report = {**body, "report_sha256": _sha(body)}
        checked = verify_report(report, focus_evidence=focus)
        self.assertTrue(checked["verified"])


if __name__ == "__main__":
    unittest.main()
