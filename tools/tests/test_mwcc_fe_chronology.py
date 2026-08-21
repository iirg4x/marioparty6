from __future__ import annotations

import copy
import json
from pathlib import Path
import tempfile
import unittest

from tools import mwcc_fe_chronology as chronology


HASHES = {
    "source_sha256": "a" * 64,
    "compiler_sha256": "b" * 64,
    "trace_sha256": "c" * 64,
}


def valid_report() -> dict[str, object]:
    uid = "obj:0001"
    evidence = dict(HASHES)
    return {
        "schema": chronology.SCHEMA_NAME,
        "schema_version": chronology.SCHEMA_VERSION,
        "producer": {
            "status": "blocked",
            "reason": "native producer remains blocked pending authenticated compiler support",
        },
        "provenance": dict(HASHES),
        "objects": [
            {
                "uid": uid,
                "ast_eobjref": {"uid": uid, "id": "ast:0001"},
                "pcode": [
                    {"uid": uid, "id": "pcode:0001", "operation": "create"},
                    {"uid": uid, "id": "pcode:0002", "operation": "reuse"},
                ],
                "ig_node": {"uid": uid, "id": "ig:0001"},
                "allocator": {"uid": uid, "id": "varinfo:0001"},
                "home_join": {
                    "uid": uid,
                    "candidates": [
                        {
                            "uid": uid,
                            "offset": -16,
                            "size": 4,
                            "authenticated": True,
                            "evidence": evidence,
                        }
                    ],
                },
            }
        ],
    }


class MwccFeChronologyTests(unittest.TestCase):
    def test_positive_create_reuse_chain_and_stack_home(self) -> None:
        report = chronology.validate_report(valid_report())
        self.assertEqual(report["objects"][0]["uid"], "obj:0001")
        self.assertEqual(
            [event["operation"] for event in report["objects"][0]["pcode"]],
            ["create", "reuse"],
        )
        self.assertEqual(report["objects"][0]["home_join"]["candidates"][0]["offset"], -16)

    def test_load_report_validates_from_utf8_json(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "chronology.json"
            path.write_text(json.dumps(valid_report()), encoding="utf-8")
            self.assertEqual(chronology.load_report(path)["schema"], chronology.SCHEMA_NAME)

    def test_missing_uid_fails_closed(self) -> None:
        report = valid_report()
        del report["objects"][0]["ig_node"]["uid"]
        with self.assertRaisesRegex(chronology.ChronologyError, "ig_node: keys differ"):
            chronology.validate_report(report)

    def test_duplicate_object_uid_fails_closed(self) -> None:
        report = valid_report()
        report["objects"].append(copy.deepcopy(report["objects"][0]))
        with self.assertRaisesRegex(chronology.ChronologyError, "duplicate object UID"):
            chronology.validate_report(report)

    def test_mismatched_stage_uid_fails_closed(self) -> None:
        report = valid_report()
        report["objects"][0]["allocator"]["uid"] = "obj:0002"
        with self.assertRaisesRegex(chronology.ChronologyError, "allocator.uid"):
            chronology.validate_report(report)

    def test_mismatched_pcode_reuse_uid_fails_closed(self) -> None:
        report = valid_report()
        report["objects"][0]["pcode"][1]["uid"] = "obj:0002"
        with self.assertRaisesRegex(chronology.ChronologyError, r"pcode\[1\].uid"):
            chronology.validate_report(report)

    def test_pointer_like_uid_is_not_a_join_key(self) -> None:
        report = valid_report()
        report["objects"][0]["uid"] = "0x801d08a4"
        with self.assertRaisesRegex(chronology.ChronologyError, "pointer-free"):
            chronology.validate_report(report)

        report = valid_report()
        report["objects"][0]["uid"] = "obj:801d08a4"
        with self.assertRaisesRegex(chronology.ChronologyError, "pointer-looking"):
            chronology.validate_report(report)

    def test_unauthenticated_stack_home_fails_closed(self) -> None:
        report = valid_report()
        report["objects"][0]["home_join"]["candidates"][0]["authenticated"] = False
        with self.assertRaisesRegex(chronology.ChronologyError, "not authenticated"):
            chronology.validate_report(report)

    def test_stack_home_offset_alignment_and_size_are_checked(self) -> None:
        report = valid_report()
        report["objects"][0]["home_join"]["candidates"][0]["offset"] = -14
        with self.assertRaisesRegex(chronology.ChronologyError, "4-byte aligned"):
            chronology.validate_report(report)

        report = valid_report()
        report["objects"][0]["home_join"]["candidates"][0]["size"] = 0
        with self.assertRaisesRegex(chronology.ChronologyError, "must be positive"):
            chronology.validate_report(report)

    def test_duplicate_stack_home_candidate_fails_closed(self) -> None:
        report = valid_report()
        candidate = copy.deepcopy(report["objects"][0]["home_join"]["candidates"][0])
        report["objects"][0]["home_join"]["candidates"].append(candidate)
        with self.assertRaisesRegex(chronology.ChronologyError, "duplicate stack-home"):
            chronology.validate_report(report)

    def test_tampered_provenance_does_not_validate_candidate(self) -> None:
        report = valid_report()
        report["provenance"]["trace_sha256"] = "d" * 64
        with self.assertRaisesRegex(chronology.ChronologyError, "does not match report provenance"):
            chronology.validate_report(report)

    def test_extra_field_is_rejected_instead_of_ignored(self) -> None:
        report = valid_report()
        report["objects"][0]["allocator"]["home_offset_guess"] = -16
        with self.assertRaisesRegex(chronology.ChronologyError, "keys differ"):
            chronology.validate_report(report)

    def test_invalid_pcode_operation_is_rejected(self) -> None:
        report = valid_report()
        report["objects"][0]["pcode"][0]["operation"] = "guess"
        with self.assertRaisesRegex(chronology.ChronologyError, "create.*reuse"):
            chronology.validate_report(report)


if __name__ == "__main__":
    unittest.main()
