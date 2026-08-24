from __future__ import annotations

import copy
import json
from pathlib import Path
import tempfile
import unittest

from tools import stack_object_lifetime_reducer as reducer


REAL_ROOT = Path(
    r"C:\Users\Anony\.codex\mp6-wt-process-efficiency-adopt-v1"
) / "build" / "capthrow-match-v461"
HAS_REAL_ACCEPTANCE = (REAL_ROOT / "results" / "jango-v7-ab-move-shift-camera-chain" / "strict.json").is_file()


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


class FakeStackHomeProducer:
    def __init__(self, packet: dict[str, object], summary: dict[str, object]) -> None:
        self.packet = packet
        self.summary = summary

    @staticmethod
    def canonical_hash(value: object) -> str:
        return reducer._digest(value)

    def validate_packet(self, _path: Path) -> dict[str, object]:
        return self.packet

    def _summarize_validated_packet(
        self, packet: dict[str, object], _path: Path, names: list[str]
    ) -> dict[str, object]:
        if packet is not self.packet or names != ["motionId", "hookMtx"]:
            raise AssertionError("reducer did not compose the supplied packet/names")
        return self.summary


class ReducerAdversarialTests(unittest.TestCase):
    def test_external_relocation_change_is_semantic_but_local_labels_normalize(self) -> None:
        row = {
            "instruction": {
                "formatted": "bl symbol",
                "relocation": {"type": 10, "type_name": "R_PPC_REL24", "target_symbol": 0},
            }
        }
        self.assertNotEqual(
            reducer._instruction_shape(row, ["callee_a"]),
            reducer._instruction_shape(row, ["callee_b"]),
        )
        self.assertEqual(
            reducer._instruction_shape(row, ["lbl_80001234"]),
            reducer._instruction_shape(row, ["@317"]),
        )


@unittest.skipUnless(HAS_REAL_ACCEPTANCE, "concrete CapThrow acceptance artifacts are absent")
class RealAcceptanceTests(unittest.TestCase):
    def bind_jango_v7(self) -> dict[str, object]:
        case = "jango-v7-ab-move-shift-camera-chain"
        return reducer.bind_case(
            observation_id="jango-v7",
            function="mbev_CapJango",
            strict_report=REAL_ROOT / "results" / case / "strict.json",
            source=REAL_ROOT / "candidates" / case / "capthrow.c",
            varinfo_report=REAL_ROOT / "traces" / "jango-v7-ab" / "mbev_CapJango.varinfo.json",
            names=["hookMtx", "motionId", "motFile"],
        )

    def bind_patapata(self, case: str) -> dict[str, object]:
        return reducer.bind_case(
            observation_id=case,
            function="mbev_CapPatapata",
            strict_report=REAL_ROOT / "results" / case / "strict.json",
            source=REAL_ROOT / "candidates" / case / "capthrow.c",
            names=["motionId", "motFile"],
        )

    def test_real_jango_tuple_is_exact_home_only_and_interval_bound(self) -> None:
        bound = self.bind_jango_v7()
        self.assertEqual(
            bound["artifacts"]["strict_report"]["sha256"],
            "d1926e2a63ddb9579540bfab8d1952a5c097c2817373a1e13a42cc732fffb0d5",
        )
        self.assertEqual(
            bound["artifacts"]["source"]["sha256"],
            "8d8188bb2f056978a0bd50f1f2b4cf2a91a55fd005cd560c1642ffedf7f3a71b",
        )
        self.assertEqual(
            bound["artifacts"]["varinfo_report"]["sha256"],
            "5302ae042d7d7053a9de689e537a6f04e547bdb20994709e608362bf4a823f62",
        )
        self.assertEqual(bound["strict"]["difference_class"], "home_only")
        self.assertEqual(bound["strict"]["target_frame_size"], 0x150)
        self.assertEqual(bound["strict"]["source_frame_size"], 0x160)
        interval = bound["objects"]["hookMtx"]["earliest_source_use_interval"]
        self.assertEqual((interval["start_line"], interval["end_line"]), (1000, 1079))
        self.assertEqual(
            interval["interval_sha256"],
            "9daa22aaa577b58bb9185dff4a82d4795cd43f471c313801261292e704bb46d3",
        )
        self.assertFalse(bound["authority_advanced"])
        self.assertEqual(
            bound["bound_sha256"],
            reducer._digest({key: value for key, value in bound.items() if key != "bound_sha256"}),
        )

    def test_real_patapata_reports_prove_stride_class_and_scope_no_go(self) -> None:
        cases = [
            "patapata-v10-abc-structural-chronology",
            "patapata-v11-two-slot-motion-table",
            "patapata-v12-loop-scoped-motfile",
        ]
        bounds = [self.bind_patapata(case) for case in cases]
        conclusions, axes, _forbidden = reducer._derive(
            bounds,
            motion_object="motionId",
            allocation_object="motFile",
            matrix_object=None,
            stack_homes={},
        )
        self.assertEqual(
            [bound["artifacts"]["strict_report"]["sha256"] for bound in bounds],
            [
                "8f380ebaac5f17c6f0ad695a14cf4c69679f3aec1a19eb08f94294429b2e7d14",
                "4a38e6c32bea7042b34a8109409b13fdedc67ccb810cd76d1d8964958ddd341e",
                "4a38e6c32bea7042b34a8109409b13fdedc67ccb810cd76d1d8964958ddd341e",
            ],
        )
        self.assertEqual(conclusions["proven_motion_stride"]["bytes"], 8)
        self.assertEqual(conclusions["remaining_allocation_class"]["bytes"], 16)
        self.assertEqual(
            conclusions["earliest_source_use_interval"]["interval_sha256"],
            "3b2c67a8965dc1beefe7f169be7af8097fae017ea9958b117e072c820287635c",
        )
        self.assertEqual(
            conclusions["difference_classes"]["comparisons"],
            ["home_only", "source_only_no_object_effect"],
        )
        self.assertEqual(conclusions["lexical_scope_no_go"], ["motFile"])
        self.assertIn("preserve-existing-allocation-scope-test-chronology", [axis["id"] for axis in axes])
        self.assertNotIn("narrow-existing-allocation-class-scope", [axis["id"] for axis in axes])

    def test_fresh_jango_best_and_four_player_capacity_tie_are_retained(self) -> None:
        best = "jango-v11-bc-motion-reuse-y100"
        best_bound = reducer.bind_case(
            observation_id=best,
            function="mbev_CapJango",
            strict_report=REAL_ROOT / "results" / best / "strict.json",
            source=REAL_ROOT / "candidates" / best / "capthrow.c",
            names=["hookMtx", "motionId"],
        )
        self.assertEqual(
            best_bound["artifacts"]["source"]["sha256"],
            "1749d5f7941c74510290a0ca1007e6890e8e5c7d975d0366bec7b8d13ce60a9b",
        )
        self.assertEqual(
            best_bound["artifacts"]["strict_report"]["sha256"],
            "9e8d0deb57713526701eb4dffdb1e839c6951fc2bf7197d3e3816072e32e425b",
        )
        self.assertEqual(
            reducer._descriptor(REAL_ROOT / "results" / best / "capthrow.o", "object")["sha256"],
            "1ac3e58242a03c40e3fe03c7ca2583bdb22fadc3668c1db122318ad433be86a5",
        )
        self.assertEqual(
            reducer._descriptor(REAL_ROOT / "results" / best / "data.json", "data")["sha256"],
            "f3718d69e97de21e53701ff31bff06150be144913d47a55567a8a9901ed991f8",
        )
        self.assertEqual(best_bound["strict"]["difference_class"], "home_only")
        self.assertEqual(best_bound["strict"]["diff_counts"], {"DIFF_ARG_MISMATCH": 161, "MATCH": 606})
        self.assertEqual(best_bound["strict"]["target_frame_size"], best_bound["strict"]["source_frame_size"])

        case = "jango-v12-ab-camera-reuse-player-capacity-y100"
        bound = reducer.bind_case(
            observation_id=case,
            function="mbev_CapJango",
            strict_report=REAL_ROOT / "results" / case / "strict.json",
            source=REAL_ROOT / "candidates" / case / "capthrow.c",
            names=["hookMtx", "motionId"],
        )
        self.assertEqual(
            bound["artifacts"]["strict_report"]["sha256"],
            "3321060096173222c2a269cead3fca33c272fff4f790347587ca268749007ba0",
        )
        motion = bound["objects"]["motionId"]
        self.assertEqual(motion["dimension_expressions"], ["GW_PLAYER_MAX"])
        self.assertEqual(motion["dimensions"], [4])
        self.assertEqual(motion["extent_bytes"], 16)
        self.assertEqual(
            motion["symbolic_capacity_ties"],
            [{"symbol": "GW_PLAYER_MAX", "elements": 4, "status": "SOURCE_DECLARATION_TIE"}],
        )

    def test_generic_stack_home_is_composed_for_exact_0x34_plus_0x8_event(self) -> None:
        bound = self.bind_jango_v7()
        packet: dict[str, object] = {
            "events": [
                {
                    "sequence": 5,
                    "event_kind": "object_stack_write_pre",
                    "object_token": "object-000000",
                    "target_slot": 0x34,
                },
                {
                    "sequence": 6,
                    "event_kind": "object_stack_write_pre",
                    "object_token": "object-000001",
                    "target_slot": 0x44,
                },
            ]
        }
        summary: dict[str, object] = {
            "authority_advanced": False,
            "binding": {"source_sha256": bound["artifacts"]["source"]["sha256"]},
            "requested_names": ["motionId", "hookMtx"],
            "mappings": [
                {
                    "name": "motionId",
                    "object_token": "object-000000",
                    "varinfo_token": "varinfo-000000",
                    "varinfo_home_snapshot": {"home_value": 0x34},
                    "mapped_slots": [0x34],
                    "owner": "UNKNOWN",
                },
                {
                    "name": "hookMtx",
                    "object_token": "object-000001",
                    "varinfo_token": "varinfo-000001",
                    "varinfo_home_snapshot": {"home_value": 0x3C},
                    "mapped_slots": [0x44],
                    "owner": "UNKNOWN",
                },
            ],
        }
        summary["summary_sha256"] = reducer._digest(summary)
        producer = FakeStackHomeProducer(packet, summary)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            bound_path = root / "bound.json"
            packet_path = root / "packet.json"
            summary_path = root / "summary.json"
            request_path = root / "request.json"
            write_json(bound_path, bound)
            write_json(packet_path, packet)
            write_json(summary_path, summary)
            request = {
                "schema": reducer.REQUEST_SCHEMA,
                "case_id": "jango-stack-lifetime",
                "function": "mbev_CapJango",
                "focus": {
                    "motion_object": "motionId",
                    "allocation_object": None,
                    "matrix_object": "hookMtx",
                },
                "bound_reports": [reducer._descriptor(bound_path, "bound")],
                "stack_homes": [
                    {
                        "observation_id": "jango-v7",
                        "packet": reducer._descriptor(packet_path, "packet"),
                        "summary": reducer._descriptor(summary_path, "summary"),
                    }
                ],
            }
            write_json(request_path, request)
            report = reducer.reduce_request(request_path, module=producer)
        event = report["conclusions"]["authentic_lifetime_event"]
        self.assertEqual(event["classification"], "scoped_aggregate_coalescing_with_outgoing_call_area")
        self.assertEqual(event["pre_matrix_reservation"], 0x34)
        self.assertEqual(event["base_shift"], 0x8)
        self.assertEqual(
            report["conclusions"]["earliest_frontend_stack_object_event"]["name"],
            "motionId",
        )
        self.assertEqual(
            report["conclusions"]["lowest_home_stack_object_event"]["target_slot"],
            0x34,
        )
        self.assertEqual(
            {row["id"] for row in report["forbidden_axes"]},
            {"dead-local-or-padding", "register-volatile-shaping", "fake-use-or-dead-branch"},
        )
        self.assertFalse(report["authority_advanced"])
        self.assertEqual(
            report["report_sha256"],
            reducer._digest({key: value for key, value in report.items() if key != "report_sha256"}),
        )

    def test_tampered_real_bound_self_hash_fails_closed(self) -> None:
        bound = self.bind_jango_v7()
        tampered = copy.deepcopy(bound)
        tampered["strict"]["target_frame_size"] += 16
        with self.assertRaisesRegex(reducer.ReducerError, "self-hash mismatch"):
            reducer._validate_bound(tampered, "tampered")


if __name__ == "__main__":
    unittest.main()
