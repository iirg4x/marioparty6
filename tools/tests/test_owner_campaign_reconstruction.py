from __future__ import annotations

import copy
import hashlib
import unittest

from tools import owner_campaign_measure as measure
from tools import owner_campaign_reconstruction as reconstruction


FUNCTION = "mbev_CapExample"
BINDING = {
    "owner": "main:board/captrap",
    "unit": "main/board/captrap",
    "function": FUNCTION,
    "source_path": "src/board/captrap.c",
    "source_sha256": "a" * 64,
    "base_commit": "b" * 40,
    "target_object_sha256": "c" * 64,
    "candidate_object_sha256": "f" * 64,
    "toolchain_sha256": "d" * 64,
    "frontier_source_sha256": "a" * 64,
}
SOURCE_SPAN = {
    "start_line": 100,
    "end_line": 150,
    "start_column": 1,
    "end_column": 2,
}


def _instruction(address: int, text: str, *, diff: str | None = None) -> dict[str, object]:
    row: dict[str, object] = {
        "instruction": {
            "address": hex(address),
            "formatted": text,
            "size": 4,
            "parts": [{"opcode": text.split()[0]}],
        }
    }
    if diff is not None:
        row["diff_kind"] = diff
    return row


def _report(*, residual_indexes: tuple[int, ...] = (), physical: bool = False) -> dict[str, object]:
    target_rows = [
        _instruction(0x1000 + index * 4, "blr" if index == 24 else "lwz r3,0x10(r1)")
        for index in range(32)
    ]
    target_rows[1] = _instruction(0x1004, "bl helper")
    target_rows[2] = _instruction(0x1008, "beq 0x1040")
    candidate_rows = copy.deepcopy(target_rows)
    for index in residual_indexes:
        target_rows[index]["diff_kind"] = "DIFF_ARG_MISMATCH"
        candidate_rows[index]["diff_kind"] = "DIFF_ARG_MISMATCH"
        candidate_rows[index]["instruction"]["formatted"] = "lwz r4,0x14(r1)"

    target_reloc = {
        "offset": 4,
        "type_name": "R_PPC_REL24",
        "target_name": "helper",
    }
    candidate_reloc = copy.deepcopy(target_reloc)
    differences = []
    if physical:
        candidate_reloc["target_name"] = "other"
        differences = [{"offset": 4, "target": ["helper", "other"]}]
    body: dict[str, object] = {
        "schema": "focus_symbol_report/v1",
        "function": FUNCTION,
        "channels": {
            "strict": {
                "metric": {
                    "target_size": 32 * 4,
                    "candidate_size": 32 * 4,
                    "diff_rows": len(residual_indexes),
                },
                "target": {
                    "instruction_count": 32,
                    "rows": target_rows,
                },
                "candidate": {
                    "instruction_count": 32,
                    "rows": candidate_rows,
                },
            },
            "data": {
                "metric": {
                    "target_size": 32 * 4,
                    "candidate_size": 32 * 4,
                    "diff_rows": len(residual_indexes),
                },
                "target": {"rows": copy.deepcopy(target_rows)},
                "candidate": {"rows": copy.deepcopy(candidate_rows)},
            },
        },
        "physical_relocations": {
            "status": "mismatch" if physical else "exact",
            "target": {
                "physical_relocation_count": 1,
                "physical_relocations": [target_reloc],
            },
            "candidate": {
                "physical_relocation_count": 1,
                "physical_relocations": [candidate_reloc],
            },
            "physical_relocation_differences": differences,
        },
    }
    body["artifact_sha256"] = reconstruction.canonical_sha256(body)
    return body


def _build(report: dict[str, object], **kwargs: object) -> dict[str, object]:
    return reconstruction.build_packet(
        report,
        BINDING,
        SOURCE_SPAN,
        **kwargs,
    )


def _broad_report(count: int) -> dict[str, object]:
    """Create a large, legal focus report for bounded-packet tests."""

    report = _report()
    target_rows = []
    candidate_rows = []
    for index in range(count):
        target = _instruction(0x2000 + index * 4, "lwz r3,0x10(r1)")
        candidate = _instruction(0x3000 + index * 4, "lwz r4,0x14(r1)")
        target["diff_kind"] = "DIFF_ARG_MISMATCH"
        candidate["diff_kind"] = "DIFF_ARG_MISMATCH"
        target_rows.append(target)
        candidate_rows.append(candidate)
    for channel in ("strict", "data"):
        material = report["channels"][channel]
        material["metric"].update(
            {"target_size": count * 4, "candidate_size": count * 4, "diff_rows": count}
        )
        material["target"]["rows"] = copy.deepcopy(target_rows)
        material["candidate"]["rows"] = copy.deepcopy(candidate_rows)
        material["target"]["instruction_count"] = count
        material["candidate"]["instruction_count"] = count
    report["artifact_sha256"] = reconstruction.canonical_sha256(
        {key: value for key, value in report.items() if key != "artifact_sha256"}
    )
    return report


class OwnerCampaignReconstructionTests(unittest.TestCase):
    def test_one_row_retains_context_and_summaries(self) -> None:
        packet = _build(_report(residual_indexes=(5,)))
        self.assertEqual(packet["schema"], reconstruction.SCHEMA)
        self.assertFalse(packet["authority_advanced"])
        self.assertEqual(packet["strict_residual_count"], 1)
        self.assertEqual(packet["data_residual_count"], 1)
        self.assertEqual(packet["residual_event_count"], 1)
        self.assertEqual(packet["status"], "READY")
        self.assertTrue(packet["exact_terminal_possible"])
        self.assertEqual(len(packet["causal_clusters"]), 1)
        self.assertEqual(packet["causal_clusters"][0]["row_indices"], [5])
        self.assertEqual(len(packet["instruction_windows"]), 1)
        self.assertEqual(packet["instruction_windows"][0]["start_index"], 0)
        self.assertIn(16, packet["stack_relative"]["target"]["offsets"])
        self.assertEqual(packet["control_flow"]["target"]["calls"]["count"], 1)
        self.assertEqual(packet["control_flow"]["target"]["branches"]["count"], 1)
        reconstruction.verify_packet(packet)

    def test_mirrored_rows_are_two_clusters_but_one_event_per_channel_pair(self) -> None:
        packet = _build(_report(residual_indexes=(3, 25)))
        self.assertEqual(packet["residual_event_count"], 2)
        self.assertEqual(len(packet["causal_clusters"]), 2)
        self.assertEqual(packet["causal_clusters"][0]["channels"], ["data", "strict"])
        self.assertEqual(packet["causal_clusters"][1]["channels"], ["data", "strict"])
        self.assertEqual(len(packet["instruction_windows"]), 2)
        self.assertEqual(packet["causal_clusters"][0]["window_ids"], ["window-000"])
        self.assertEqual(packet["causal_clusters"][1]["window_ids"], ["window-001"])

    def test_absolute_address_shift_does_not_fake_cfg_drift(self) -> None:
        report = _report(residual_indexes=(5,))
        for channel in ("strict", "data"):
            for row in report["channels"][channel]["candidate"]["rows"]:
                address = int(row["instruction"]["address"], 0)
                row["instruction"]["address"] = hex(address + 0x100)
        unsigned = {
            key: value for key, value in report.items()
            if key != "artifact_sha256"
        }
        report["artifact_sha256"] = reconstruction.canonical_sha256(unsigned)

        packet = _build(report)

        self.assertEqual(packet["status"], "READY")
        self.assertEqual(
            packet["target_first_signal"]["reason"],
            "bounded_target_first_residuals",
        )

    def test_physical_only_keeps_relocation_difference_without_fake_code_cluster(self) -> None:
        packet = _build(_report(physical=True))
        self.assertEqual(packet["strict_residual_count"], 0)
        self.assertEqual(packet["data_residual_count"], 0)
        self.assertEqual(packet["causal_clusters"], [])
        self.assertEqual(packet["instruction_windows"], [])
        self.assertEqual(packet["physical_relocations"]["difference_count"], 1)
        self.assertFalse(packet["exact_terminal_possible"])
        self.assertEqual(len(packet["physical_relocation_differences"]), 1)

    def test_exact_report_has_empty_residual_context(self) -> None:
        packet = _build(_report())
        self.assertEqual(packet["residual_rows"], [])
        self.assertEqual(packet["causal_clusters"], [])
        self.assertEqual(packet["instruction_windows"], [])
        self.assertEqual(packet["physical_relocations"]["status"], "exact")
        self.assertEqual(packet["status"], "READY")
        reconstruction.verify_packet(packet)

    def test_binding_and_focus_hash_drift_fail_closed(self) -> None:
        report = _report(residual_indexes=(5,))
        wrong_binding = dict(BINDING)
        wrong_binding["source_sha256"] = "f" * 64
        report["source_sha256"] = "a" * 64
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        with self.assertRaisesRegex(reconstruction.ReconstructionPacketError, "source_sha256"):
            reconstruction.build_packet(report, wrong_binding, SOURCE_SPAN)

        tampered = copy.deepcopy(report)
        tampered["channels"]["strict"]["target"]["rows"][0]["instruction"]["formatted"] = "nop"
        with self.assertRaisesRegex(reconstruction.ReconstructionPacketError, "focus artifact"):
            _build(tampered)

    def test_size_limit_is_deterministic_and_fail_closed(self) -> None:
        with self.assertRaises(reconstruction.ReconstructionPacketError) as raised:
            _build(_report(residual_indexes=(5,)), max_output_bytes=512)
        self.assertEqual(raised.exception.code, "output_limit")

    def test_source_span_metadata_alias_and_self_hash_are_bound(self) -> None:
        report = _report(residual_indexes=(5,))
        packet = reconstruction.build_packet(
            report,
            BINDING,
            source_span_metadata=SOURCE_SPAN,
        )
        self.assertEqual(packet["source_span"], SOURCE_SPAN)
        self.assertEqual(packet["focus_artifact_sha256"], report["artifact_sha256"])
        self.assertEqual(packet["source_span_sha256"], reconstruction.canonical_sha256(SOURCE_SPAN))
        self.assertEqual(
            packet["packet_sha256"],
            hashlib.sha256(
                reconstruction.canonical_json(
                    {key: value for key, value in packet.items() if key != "packet_sha256"}
                )
            ).hexdigest(),
        )

    def test_source_span_scanner_ignores_comments_strings_and_nested_braces(self) -> None:
        source = (
            "/* fake_fn() { } */\n"
            "int fake_fn(void);\n"
            "int real_fn(int value) {\n"
            "    const char *text = \"} /* not a brace */\";\n"
            "    /* { a comment brace } */\n"
            "    if (value) { return 1; }\n"
            "    return 0;\n"
            "}\n"
        )
        metadata = reconstruction.source_span_metadata(source, "real_fn")
        self.assertEqual(metadata["start_line"], 3)
        self.assertEqual(metadata["end_line"], 8)
        start = metadata["start_offset"]
        end = metadata["end_offset"]
        self.assertEqual(
            metadata["span_sha256"],
            hashlib.sha256(source[start:end].encode("utf-8")).hexdigest(),
        )
        with self.assertRaises(reconstruction.ReconstructionPacketError):
            reconstruction.source_span_metadata(source, "missing")

    def test_source_span_scanner_skips_calls_before_definition(self) -> None:
        source = (
            "int real_fn(void);\n"
            "int main(void) { return real_fn(); }\n"
            "int real_fn(void) { return 7; }\n"
        )
        metadata = reconstruction.source_span_metadata(source, "real_fn")
        self.assertEqual(metadata["start_line"], 3)
        self.assertEqual(metadata["end_line"], 3)

    def test_source_span_scanner_skips_function_like_macro_body(self) -> None:
        source = (
            "#define real_fn() { return 99; }\n"
            "int real_fn(void) { return 7; }\n"
        )
        metadata = reconstruction.source_span_metadata(source, "real_fn")
        self.assertEqual(metadata["start_line"], 2)

    def test_verify_rejects_minimal_and_resealed_tampered_packets(self) -> None:
        minimal = reconstruction.seal(
            {
                "schema": reconstruction.SCHEMA,
                "schema_version": reconstruction.SCHEMA_VERSION,
                "authority_advanced": False,
            }
        )
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "field set"
        ):
            reconstruction.verify_packet(minimal)

        packet = _build(_report(residual_indexes=(5,)))
        tampered = dict(packet)
        tampered["strict_residual_count"] = 99
        tampered = reconstruction.seal(tampered)
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "strict residual count"
        ):
            reconstruction.verify_packet(tampered)

    def test_forged_row_identity_is_rejected(self) -> None:
        report = _report(residual_indexes=(5,))
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "row identity"
        ):
            reconstruction.build_packet(
                report,
                BINDING,
                SOURCE_SPAN,
                strict_row_ids=["FORGED"],
                data_row_ids=["FORGED"],
            )

    def test_production_target_instruction_id_is_recomputed_and_accepted(self) -> None:
        report = _report(residual_indexes=(5,))
        strict_ids = measure._stable_row_ids(report, "strict", FUNCTION)
        data_ids = measure._stable_row_ids(report, "data", FUNCTION)

        packet = reconstruction.build_packet(
            report,
            BINDING,
            SOURCE_SPAN,
            strict_row_ids=strict_ids,
            data_row_ids=data_ids,
        )

        self.assertEqual(packet["strict_residuals"], strict_ids)
        self.assertEqual(packet["data_residuals"], data_ids)
        reconstruction.verify_packet(packet)

    def test_row_override_does_not_reseal_tampered_focus(self) -> None:
        report = _report(residual_indexes=(5,))
        strict_ids = measure._stable_row_ids(report, "strict", FUNCTION)
        data_ids = measure._stable_row_ids(report, "data", FUNCTION)
        report["channels"]["strict"]["target"]["rows"][5]["instruction"][
            "formatted"
        ] = "forged r0,r0,r0"

        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "focus artifact"
        ):
            reconstruction.build_packet(
                report,
                BINDING,
                SOURCE_SPAN,
                strict_row_ids=strict_ids,
                data_row_ids=data_ids,
            )

    def test_compact_physical_payload_is_valid_count_evidence(self) -> None:
        report = _report(residual_indexes=(5,))
        physical = report["physical_relocations"]
        for side in ("target", "candidate"):
            value = physical[side]
            rows = value.pop("physical_relocations")
            value["physical_relocation_payload_sha256"] = (
                reconstruction.canonical_sha256(rows)
            )
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        packet = _build(report)
        self.assertEqual(packet["status"], "READY")
        self.assertTrue(packet["exact_terminal_possible"])
        self.assertTrue(packet["physical_relocations"]["target"]["count_valid"])
        reconstruction.verify_packet(packet)

    def test_physical_compaction_retains_actual_changed_pair(self) -> None:
        report = _report(residual_indexes=(5,), physical=True)
        physical = report["physical_relocations"]
        physical["physical_relocation_differences"] = [
            {
                "target": physical["target"]["physical_relocations"],
                "candidate": physical["candidate"]["physical_relocations"],
            }
        ]
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        packet = _build(report)
        difference = packet["physical_relocations"]["differences"][0]
        self.assertEqual(difference["changed_pair_count"], 1)
        self.assertEqual(difference["changed_pairs"][0]["pair_index"], 0)
        self.assertEqual(
            difference["changed_pairs"][0]["target"]["target_name"], "helper"
        )
        self.assertEqual(
            difference["changed_pairs"][0]["candidate"]["target_name"], "other"
        )

    def test_negative_physical_count_is_rejected_by_builder(self) -> None:
        report = _report(residual_indexes=(5,))
        report["physical_relocations"]["target"]["physical_relocation_count"] = -1
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "physical.target"
        ):
            _build(report)

    def test_broad_residual_returns_compact_decomposition_packet(self) -> None:
        report = _report()
        target_rows = []
        candidate_rows = []
        for index in range(80):
            target = _instruction(0x2000 + index * 4, "lwz r3,0x10(r1)")
            candidate = _instruction(0x3000 + index * 4, "lwz r4,0x14(r1)")
            target["diff_kind"] = "DIFF_ARG_MISMATCH"
            candidate["diff_kind"] = "DIFF_ARG_MISMATCH"
            target_rows.append(target)
            candidate_rows.append(candidate)
        for channel in ("strict", "data"):
            material = report["channels"][channel]
            material["metric"].update(
                {"target_size": 320, "candidate_size": 320, "diff_rows": 80}
            )
            material["target"]["rows"] = copy.deepcopy(target_rows)
            material["candidate"]["rows"] = copy.deepcopy(candidate_rows)
            material["target"]["instruction_count"] = 80
            material["candidate"]["instruction_count"] = 80
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        packet = _build(report)
        self.assertEqual(packet["status"], "UNKNOWN")
        self.assertFalse(packet["exact_terminal_possible"])
        self.assertEqual(packet["target_first_signal"]["next_action"], "DECOMPOSE")
        self.assertTrue(packet["decomposition_regions"])
        self.assertLess(len(reconstruction.canonical_json(packet)), 256 * 1024)
        reconstruction.verify_packet(packet)

    def test_maximum_legal_broad_census_is_compact_and_verified(self) -> None:
        for count in (1000, 4096):
            report = _broad_report(count)
            packet = _build(report)
            self.assertEqual(packet["status"], "UNKNOWN")
            self.assertEqual(packet["target_first_signal"]["next_action"], "DECOMPOSE")
            self.assertFalse(packet["exact_terminal_possible"])
            self.assertLessEqual(
                len(reconstruction.canonical_json(packet)),
                reconstruction.MAX_OUTPUT_BYTES,
            )
            self.assertEqual(packet["strict_residuals_total_count"], count)
            self.assertEqual(packet["data_residuals_total_count"], count)
            self.assertEqual(
                packet["strict_residuals_omitted_count"],
                count - len(packet["strict_residuals"]),
            )
            self.assertEqual(
                packet["data_residuals_omitted_count"],
                count - len(packet["data_residuals"]),
            )
            strict_ids = set(packet["strict_residuals"])
            data_ids = set(packet["data_residuals"])
            for cluster in packet["causal_clusters"]:
                self.assertTrue(set(cluster["strict_row_ids"]) <= strict_ids)
                self.assertTrue(set(cluster["data_row_ids"]) <= data_ids)
            reconstruction.verify_packet(packet)

    def test_broad_omitted_digest_hashes_actual_events(self) -> None:
        report = _broad_report(1000)
        packet = _build(report)
        channels = reconstruction._channels(report)
        events, residual_ids, _target, _candidate = reconstruction._build_residual_events(
            report, channels, function=FUNCTION
        )
        retained = {reconstruction.canonical_sha256(item) for item in packet["residual_rows"]}
        omitted_events = [
            event
            for event in events
            if reconstruction.canonical_sha256(event) not in retained
        ]
        self.assertEqual(
            packet["residual_rows_omitted_sha256"],
            reconstruction.canonical_sha256(omitted_events),
        )
        omitted_strict = [
            row_id for row_id in residual_ids["strict"] if row_id not in set(packet["strict_residuals"])
        ]
        self.assertEqual(
            packet["strict_residuals_omitted_sha256"],
            reconstruction.canonical_sha256(omitted_strict),
        )
        self.assertNotEqual(
            packet["residual_rows_omitted_sha256"],
            packet["residual_rows_full_sha256"],
        )

    def test_broad_forged_omission_or_cluster_reference_is_rejected(self) -> None:
        packet = _build(_broad_report(1000))
        forged_omission = reconstruction.seal(dict(packet))
        forged_omission["residual_rows_omitted_sha256"] = packet["residual_rows_full_sha256"]
        forged_omission = reconstruction.seal(forged_omission)
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError, "omitted digest is not an omitted set"
        ):
            reconstruction.verify_packet(forged_omission)

        forged_cluster = copy.deepcopy(packet)
        forged_cluster["causal_clusters"][0]["strict_row_ids"].append("strict:forged")
        forged_cluster = reconstruction.seal(forged_cluster)
        with self.assertRaisesRegex(
            reconstruction.ReconstructionPacketError,
            "full digest mismatch|escape residual census",
        ):
            reconstruction.verify_packet(forged_cluster)

    def test_broad_physical_difference_census_is_compact(self) -> None:
        report = _broad_report(1000)
        physical = report["physical_relocations"]
        differences = [
            {"offset": index * 4, "target": ["target"], "candidate": ["candidate"]}
            for index in range(1000)
        ]
        physical["physical_relocation_differences"] = differences
        physical["difference_count"] = len(differences)
        physical["physical_difference_ids"] = [
            f"physical:row:{index}:sha256={'a' * 64}" for index in range(1000)
        ]
        report["artifact_sha256"] = reconstruction.canonical_sha256(
            {key: value for key, value in report.items() if key != "artifact_sha256"}
        )
        packet = _build(report)
        self.assertLessEqual(
            len(reconstruction.canonical_json(packet)), reconstruction.MAX_OUTPUT_BYTES
        )
        self.assertEqual(
            packet["physical_relocations"]["differences_total_count"], 1000
        )
        self.assertEqual(
            packet["physical_relocations"]["differences_omitted_count"],
            1000 - reconstruction.MAX_BROAD_PHYSICAL_DIFFERENCES,
        )
        reconstruction.verify_packet(packet)


if __name__ == "__main__":
    unittest.main()
