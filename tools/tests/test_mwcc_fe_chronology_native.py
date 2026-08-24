from __future__ import annotations

import copy
import unittest

from tools import mwcc_fe_chronology_native as chronology


HASHES = {
    "source_sha256": "a" * 64,
    "compiler_sha256": "b" * 64,
    "trace_sha256": "c" * 64,
    "session_id": "session-0123456789abcdef",
}


def hook_bytes() -> dict[str, str]:
    return {str(row["id"]): str(row["prefix"]) for row in chronology.HOOKS}


def complete_packet() -> dict[str, object]:
    session = chronology.FrontendChronologySession(HASHES)
    session.on_process_started(hook_bytes=hook_bytes())
    session.on_hook("reset")
    session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)
    session.on_hook("generic_insert_0", pointer=0x1000)
    session.on_hook("generic_insert_1", pointer=0x1100)
    session.on_hook("generic_insert_2", pointer=0x1200)
    session.on_hook("bulk_object_link", object_pointers=[0x1000, 0x1100, 0x1200])
    session.on_hook("target_boundary", phase="exit", function=chronology.TARGET_FUNCTION)
    session.on_post_allocation_snapshot(
        [
            {"pointer": 0x1000, "varinfo_pointer": 0x2000, "home_value": -32},
            {"pointer": 0x1100, "varinfo_pointer": 0x2100, "home_value": -36},
            {"pointer": 0x1200, "varinfo_pointer": 0x2200, "home_value": -40},
        ]
    )
    return session.on_process_exit()


class MwccFeChronologyNativeTests(unittest.TestCase):
    def test_authenticated_hook_plan_has_requested_gc26_sites(self) -> None:
        self.assertEqual(
            [int(row["address"]) for row in chronology.HOOKS],
            [0x4F0A4A, 0x4E91DC, 0x4F113B, 0x4F4DC9, 0x55CBE2, 0x433492],
        )
        observed = {}

        def read_image(address: int, size: int) -> bytes:
            row = chronology.HOOK_BY_ADDRESS[address]
            value = bytes.fromhex(str(row["prefix"]))
            self.assertEqual(size, len(value))
            observed[address] = value
            return value

        result = chronology.validate_hook_image(read_image)
        self.assertEqual(set(result), set(chronology.HOOK_ORDER))
        self.assertEqual(len(observed), len(chronology.HOOKS))

    def test_complete_capture_is_pointer_free_and_hash_bound(self) -> None:
        packet = complete_packet()
        validated = chronology.validate_packet(packet)
        self.assertEqual(validated["status"], "CAPTURED_UNKNOWN_OWNERSHIP")
        self.assertEqual(validated["provenance"], HASHES)
        self.assertFalse(validated["authority_advanced"])
        self.assertEqual(len(validated["generations"]), 3)
        serialized = chronology.json.dumps(packet, sort_keys=True)
        self.assertNotIn("0x1000", serialized)
        self.assertNotIn('"pointer":', serialized)
        self.assertTrue(all("generation_id" in row for row in validated["generations"]))

    def test_packet_binds_arbitrary_canonical_function_and_session(self) -> None:
        session = chronology.FrontendChronologySession(
            HASHES,
            function="CapSelectMasuPlayer",
        )
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("target_boundary", phase="entry", function="CapSelectMasuPlayer")
        session.on_hook("generic_insert_0", pointer=0x1000)
        session.on_hook("bulk_object_link", object_pointers=[0x1000])
        session.on_post_allocation_snapshot(
            [{"pointer": 0x1000, "varinfo_pointer": 0x2000, "home_value": -32}]
        )
        packet = session.on_process_exit()
        validated = chronology.validate_packet(packet)
        self.assertEqual(validated["function"], "CapSelectMasuPlayer")
        self.assertEqual(validated["provenance"]["session_id"], HASHES["session_id"])

    def test_wrong_hook_opcode_fails_closed(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        bad = hook_bytes()
        bad["generic_insert_1"] = "90" * (len(bytes.fromhex(bad["generic_insert_1"])) + 0)
        with self.assertRaisesRegex(chronology.Rejected, "wrong opcode"):
            session.on_process_started(hook_bytes=bad)

    def test_authority_advance_claim_fails_closed(self) -> None:
        packet = complete_packet()
        packet["authority_advanced"] = True
        packet = chronology.seal(packet)
        with self.assertRaisesRegex(chronology.Rejected, "policy mismatch"):
            chronology.validate_packet(packet)

    def test_duplicate_insertion_pointer_is_ambiguous(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x1000)
        with self.assertRaisesRegex(chronology.Rejected, "duplicate/ambiguous"):
            session.on_hook("generic_insert_1", pointer=0x1000)

    def test_missing_bulk_link_fails_closed(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x1000)
        session.on_hook("generic_insert_1", pointer=0x1100)
        session.on_hook("generic_insert_2", pointer=0x1200)
        with self.assertRaisesRegex(chronology.Rejected, "missing insertion"):
            session.on_hook("bulk_object_link", object_pointers=[0x1000, 0x1100, 0x1300])

    def test_ambiguous_post_allocation_join_fails_closed(self) -> None:
        packet = complete_packet()
        tampered = copy.deepcopy(packet)
        bindings = tampered["events"][-1]["bindings"]
        bindings[1]["varinfo_generation_id"] = bindings[0]["varinfo_generation_id"]
        tampered["packet_sha256"] = chronology.canonical_hash(
            {key: value for key, value in tampered.items() if key != "packet_sha256"}
        )
        with self.assertRaisesRegex(chronology.Rejected, "ambiguous"):
            chronology.validate_packet(tampered)

    def test_incomplete_hook_coverage_fails_at_close(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)
        session.on_hook("generic_insert_0", pointer=0x1000)
        session.on_hook("generic_insert_1", pointer=0x1100)
        with self.assertRaisesRegex(chronology.Rejected, "bulk-link or post-allocation chronology"):
            session.on_process_exit()

    def test_reset_delimited_epochs_select_only_target_epoch(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())

        # A prior function owns its own reset-delimited Object identities.
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x1000)
        session.on_hook("generic_insert_2", pointer=0x1010)

        # The target epoch starts at the next reset and may have codegen both
        # before and after its function-entry boundary.  The boundary is an
        # entry observation; the following reset closes this epoch.
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x2000)
        session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)
        session.on_hook("generic_insert_1", pointer=0x2010)
        session.on_hook("bulk_object_link", object_pointers=[0x2000, 0x2010])
        session.on_post_allocation_snapshot(
            [
                {"pointer": 0x2000, "varinfo_pointer": 0x3000, "home_value": -32},
                {"pointer": 0x2010, "varinfo_pointer": 0x3010, "home_value": -36},
            ]
        )

        # Later functions must not contaminate the selected packet.
        session.on_hook("reset")
        session.on_hook("generic_insert_2", pointer=0x4000)
        packet = session.on_process_exit()
        chronology.validate_packet(packet)

        self.assertEqual(len(packet["generations"]), 2)
        self.assertEqual(packet["hook_coverage"]["reset"], 1)
        self.assertEqual(packet["hook_coverage"]["generic_insert_0"], 1)
        self.assertEqual(packet["hook_coverage"]["generic_insert_1"], 1)
        self.assertEqual(packet["hook_coverage"]["generic_insert_2"], 0)
        self.assertEqual(packet["events"][0]["event_kind"], "reset")
        self.assertEqual(
            [event["phase"] for event in packet["events"] if event["event_kind"] == "target_boundary"],
            ["entry"],
        )
        serialized = chronology.json.dumps(packet, sort_keys=True)
        self.assertNotIn("0x1000", serialized)
        self.assertNotIn("0x4000", serialized)

    def test_multiple_target_epochs_fail_closed(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x1000)
        session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)
        session.on_hook("generic_insert_1", pointer=0x1010)
        session.on_hook("bulk_object_link", object_pointers=[0x1000, 0x1010])
        session.on_post_allocation_snapshot(
            [
                {"pointer": 0x1000, "varinfo_pointer": 0x2000, "home_value": -32},
                {"pointer": 0x1010, "varinfo_pointer": 0x2010, "home_value": -36},
            ]
        )
        session.on_hook("reset")
        with self.assertRaisesRegex(chronology.Rejected, "ambiguous multiple target epochs"):
            session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)

    def test_target_entry_followed_by_reset_is_not_a_boundary_failure(self) -> None:
        session = chronology.FrontendChronologySession(HASHES)
        session.on_process_started(hook_bytes=hook_bytes())
        session.on_hook("reset")
        session.on_hook("generic_insert_0", pointer=0x5000)
        session.on_hook("bulk_object_link", object_pointers=[0x5000])
        session.on_hook("target_boundary", phase="entry", function=chronology.TARGET_FUNCTION)
        session.on_post_allocation_snapshot(
            [{"pointer": 0x5000, "varinfo_pointer": 0x6000, "home_value": -32}]
        )
        # This is the next function's locals-list reset, not a duplicate
        # target epoch.  It must leave the selected target evidence intact.
        session.on_hook("reset")
        session.on_hook("generic_insert_2", pointer=0x7000)
        packet = session.on_process_exit()
        chronology.validate_packet(packet)
        self.assertEqual(len(packet["generations"]), 1)
        self.assertEqual(packet["hook_coverage"]["generic_insert_2"], 0)
        self.assertEqual(packet["hook_coverage"]["reset"], 1)


if __name__ == "__main__":
    unittest.main()
