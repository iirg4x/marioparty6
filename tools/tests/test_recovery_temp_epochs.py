from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

from tools import recovery_temp_epochs as epochs


RESET = "0xFE3BF"
RESTORE = "0xFE333"
PHASE = "0x10873E"
UNKNOWN = "0x1234"


def _descriptor(path: str, digest: str = "a" * 64, size: int = 1) -> dict[str, object]:
    return {"path": path, "sha256": digest, "size_bytes": size}


def _lane_state(after: int, *, high: int = 259, saved: int = 32, default: int = 32) -> dict[str, object]:
    return {
        "status": "CAPTURED",
        "observed_lane": 0,
        "arrays": {
            "current": [after],
            "high_water": [high],
            "saved_base": [saved],
            "phase_default": [default],
        },
    }


def _event(index: int, before: int, after: int, *, ip: str = UNKNOWN,
           token: str | None = None, kind: str = "1e",
           names: tuple[str, ...] = (), lhs: tuple[str, ...] = (),
           rhs: tuple[str, ...] = (), state: dict[str, object] | None = None,
           lane: str | None = None) -> dict[str, object]:
    event: dict[str, object] = {
        "schema": epochs.EVENT_SCHEMA,
        "status": "CAPTURED",
        "session_id": "session-test",
        "function": "test_function",
        "thread_ordinal": 0,
        "sequence": index,
        "event_ordinal": index + 1,
        "event_id": f"event-{index}",
        "counter_before": before,
        "counter_after": after,
        "counter_delta": after - before,
        "instruction_pointer_rva": ip,
        "codegen_token": token or f"token-{index}",
        "codegen_fields": {"expression_kind": kind},
        "observed_expression_names": list(names),
        "assignment_owners": {"lhs": list(lhs), "rhs": list(rhs)},
    }
    if state is not None:
        event["temporary_lane_state"] = state
    if lane is not None:
        event["lane"] = lane
    return event


def _capture(events: list[dict[str, object]], *, first_before: int | None = None,
             session_id: str = "session-test", function: str = "test_function",
             lane: str | None = None) -> dict[str, object]:
    events = copy.deepcopy(events)
    for event in events:
        event["session_id"] = session_id
        event["function"] = function
        if lane is not None:
            event["lane"] = lane
    return {
        "schema": epochs.CAPTURE_SCHEMA,
        "status": "CAPTURED",
        "diagnostic_only": True,
        "authority_advanced": False,
        "function": function,
        "session_id": session_id,
        "target_thread_ordinal": 0,
        "source": _descriptor("source.c"),
        "compiler": _descriptor("mwcceppc.exe", epochs.PINNED_COMPILER_SHA256, 2),
        "baseline_object": _descriptor("baseline.o", "b" * 64, 3),
        "output_object": _descriptor("output.o", "c" * 64, 4),
        "producer": {
            "expression_capture": {"sha256": "d" * 64, "size_bytes": 1}
        },
        "counter_before_first_write": first_before if first_before is not None else (events[0]["counter_before"] if events else 0),
        "event_count": len(events),
        "events": events,
    }


class NormalizedNamesTests(unittest.TestCase):
    def test_duplicate_ids_without_tree_references_are_rejected(self):
        first, second = _event(0, 32, 33), _event(1, 33, 34)
        second['event_id'] = first['event_id']
        with self.assertRaisesRegex(epochs.CaptureError, 'duplicate capture event_id'):
            epochs.analyze_capture(_capture([first, second]), 32)

    def tree(self):
        def obj(name):
            return {"status": "CAPTURED", "native_kind": 56, "token": name + "-expr",
                    "object": {"name_status": "named", "name": name, "token": name + "-obj"}}
        return {"schema": "mwcc_normalized_expression_tree/v1", "tree": {
            "status": "CAPTURED", "native_kind": 24, "token": "root",
            "left": {"native_kind": 49, "status": "CAPTURED", "operand": {
                "native_kind": 15, "status": "CAPTURED", "left": obj("GwPlayer"),
                "right": {"native_kind": 9, "status": "CAPTURED", "left": obj("targetPlayer"),
                          "right": {"integer_constant": 264}}}}}}

    def test_bitfield_names_and_same_capture_reference(self):
        first = _event(0, 120, 121, names=("legacy",))
        first["normalized_expression_tree"] = self.tree()
        second = _event(1, 121, 122)
        second["normalized_expression_tree"] = {
            "schema": "mwcc_normalized_expression_tree_reference/v1",
            "reference_event_id": "event-0", "expression_token": "root"}
        _, events = epochs._validate_capture(_capture([first, second]))
        self.assertEqual(events[0]["source_expression_names"], ["GwPlayer", "legacy", "targetPlayer"])
        self.assertEqual(events[1]["source_expression_names"], ["GwPlayer", "targetPlayer"])
        self.assertEqual(events[1]["source_expression_name_sources"]["normalized_tree"]["event_id"], "event-0")

    def test_missing_ambiguous_and_mismatched_reference_fail_closed(self):
        for ref, token, duplicate in [("other-capture-event", "root", False),
                                      ("event-0", "wrong", False), ("event-0", "root", True)]:
            first = _event(0, 120, 121)
            first["normalized_expression_tree"] = self.tree()
            second = _event(1, 121, 122)
            second["normalized_expression_tree"] = {
                "schema": "mwcc_normalized_expression_tree_reference/v1",
                "reference_event_id": ref, "expression_token": token}
            if duplicate:
                second["event_id"] = "event-0"
            with self.assertRaises(epochs.CaptureError):
                epochs._validate_capture(_capture([first, second]))

    def test_malformed_named_object_and_bounded_tree(self):
        event = _event(0, 120, 121)
        event["normalized_expression_tree"] = self.tree()
        tree = event["normalized_expression_tree"]["tree"]
        tree["left"]["operand"]["left"]["object"]["name"] = None
        with self.assertRaises(epochs.CaptureError):
            epochs._validate_capture(_capture([event]))
        for _ in range(70):
            tree = {"operand": tree}
        event["normalized_expression_tree"]["tree"] = tree
        with self.assertRaises(epochs.CaptureError):
            epochs._validate_capture(_capture([event]))

    def test_null_reference_remains_nameless(self):
        first = _event(0, 120, 121)
        first["normalized_expression_tree"] = {"schema": "mwcc_normalized_expression_tree/v1",
                                                "tree": {"reason": "null_expression", "status": "UNKNOWN"}}
        second = _event(1, 121, 122)
        second["normalized_expression_tree"] = {"schema": "mwcc_normalized_expression_tree_reference/v1",
                                                 "reference_event_id": "event-0", "expression_token": None}
        _, events = epochs._validate_capture(_capture([first, second]))
        self.assertEqual(events[1]["source_expression_names"], [])


class ActualCallOriginTests(unittest.TestCase):
    def event(self):
        event = _event(0, 32, 33, names=("OuterCall", "argumentName"))
        event["next_instruction_pointer_rva"] = "0x00128903"
        event["actual_call_origin"] = {
            "status": "CAPTURED", "binding": "native_return_allocation_frame",
            "call_expression_token": "inner-call", "result_descriptor_token": "result",
            "result_native_kind": 0,
            "normalized_expression_tree": {"schema": "mwcc_normalized_expression_tree/v1", "tree": {
                "status": "CAPTURED", "native_kind": 54, "token": "inner-call",
                "callee": {"status": "CAPTURED", "native_kind": 56, "token": "inner-callee",
                           "object": {"token": "inner-object", "name_status": "named", "name": "InnerCall"}},
                "arguments": {"items": [{"name": "NotTheCallee"}]},
            }},
        }
        return event

    def test_inner_origin_does_not_replace_enclosing_names(self):
        capture = _capture([self.event()])
        _, normalized = epochs._validate_capture(capture)
        result = epochs.analyze_capture(capture, 32)
        birth = result["births"][0]
        self.assertEqual(birth["source_expression_names"], ["OuterCall", "argumentName"])
        origin = birth["actual_call_origin"]
        self.assertEqual(origin["callee_object"]["name"], "InnerCall")
        self.assertEqual(origin, normalized[0]["actual_call_origin"])
        self.assertEqual(result["constraint"]["actual_call_origins"], [origin])
        self.assertNotIn("NotTheCallee", json.dumps(origin))

    def test_indirect_operand_chain(self):
        event = self.event()
        tree = event["actual_call_origin"]["normalized_expression_tree"]["tree"]
        tree["callee"] = {"status": "CAPTURED", "native_kind": 48, "token": "indirect", "operand": tree["callee"]}
        origin = epochs.analyze_capture(_capture([event]), 32)["births"][0]["actual_call_origin"]
        self.assertEqual(origin["callee_kind"], "indirect")
        self.assertEqual(len(origin["callee_operand_chain"]), 2)

    def test_malformed_or_wrong_hook_rejected(self):
        for field, value in (("status", "FAILED"), ("binding", "guessed"),
                             ("call_expression_token", "wrong"), ("result_native_kind", True),
                             ("normalized_expression_tree", {})):
            with self.subTest(field=field):
                event = self.event()
                event["actual_call_origin"][field] = value
                with self.assertRaises(epochs.CaptureError):
                    epochs.analyze_capture(_capture([event]), 32)
        event = self.event()
        event["next_instruction_pointer_rva"] = "0x00128904"
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(_capture([event]), 32)

    def test_legacy_absence(self):
        birth = epochs.analyze_capture(_capture([_event(0, 32, 33)]), 32)["births"][0]
        self.assertIsNone(birth["actual_call_origin"])

    def test_operand_depth_bound(self):
        event = self.event()
        tree = event["actual_call_origin"]["normalized_expression_tree"]["tree"]
        for depth in range(6):
            tree["callee"] = {"status": "CAPTURED", "native_kind": 48,
                              "token": f"operand-{depth}", "operand": tree["callee"]}
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(_capture([event]), 32)


def _shaped_capture(
    groups: list[tuple[str, int]], *, session_id: str, lane: str = "pcode"
) -> dict[str, object]:
    events = [_event(0, 0, 0, token=f"{session_id}-init", kind="init", lane=lane)]
    counter = 0
    for group_index, (name, count) in enumerate(groups):
        for _ in range(count):
            events.append(_event(
                len(events), counter, counter + 1,
                token=f"{session_id}-group-{group_index}", kind="1e",
                names=(name,), lhs=(name,), lane=lane,
            ))
            counter += 1
    return _capture(events, first_before=0, session_id=session_id, lane=lane)


class RecoveryTempEpochsTests(unittest.TestCase):
    def test_multiple_epochs_owners_and_numeric_budget(self) -> None:
        events = [
            _event(0, 32, 33, token="masu-owner", names=("masuId",), lhs=("masuId",)),
            _event(1, 33, 259, ip=RESTORE, token="high-owner", state=_lane_state(259, high=259)),
            _event(2, 259, 32, ip=RESET, token="remove-owner", names=("mbPlayerCapsuleRemove",),
                   state=_lane_state(32, high=259, saved=32)),
            _event(3, 32, 33, token="max-owner", kind="13", names=("mbPlayerCapsuleMaxGet",)),
        ]
        result = epochs.analyze_capture(_capture(events), 32)

        self.assertEqual(result["status"], "reused")
        self.assertEqual(result["birth_count"], 2)
        self.assertEqual(result["relationships"][0]["relationship"], "distinct_codegen_owner")
        self.assertEqual(result["relationships"][0]["second_birth_boundary"]["kind"], "reset_to_saved_base")
        constraint = result["constraint"]
        self.assertEqual(constraint["minimum_allocation_reduction_before_reset"], 3)
        self.assertEqual(constraint["numeric_constraint_status"], "qualified")
        self.assertEqual(constraint["allocation_groups_before_reset"][0]["count"], 1)
        self.assertIn("0x100", constraint["required_epoch_lifetime_change"])
        self.assertIn("real expression boundary", constraint["required_epoch_lifetime_change"])
        self.assertNotIn("braces", constraint["text"])

    def test_restoration_is_not_a_birth(self) -> None:
        events = [
            _event(0, 32, 33, token="first"),
            _event(1, 33, 259, ip=RESTORE, state=_lane_state(259, high=259)),
            _event(2, 259, 260, token="after-restore"),
        ]
        result = epochs.analyze_capture(_capture(events), 32)

        self.assertEqual(result["birth_count"], 1)
        self.assertEqual(result["status"], "single_birth")
        self.assertEqual([item["kind"] for item in result["transitions"]], ["restore_high_water"])

    def test_unknown_ip_has_no_generic_wrap_interpretation(self) -> None:
        events = [
            _event(0, 0, 1, token="first"),
            _event(1, 1, 0, ip=UNKNOWN, token="unknown"),
            _event(2, 0, 1, token="second"),
        ]
        result = epochs.analyze_capture(_capture(events), 0)

        self.assertEqual(result["birth_count"], 2)
        self.assertEqual(result["unknown_transition_count"], 1)
        self.assertEqual(result["births"][1]["epoch_boundary"]["kind"], "unknown_transition")
        self.assertIn("no generic wrap", result["transitions"][0]["description"])

    def test_counter_gap_is_unknown_before_numeric_budget(self) -> None:
        events = [
            _event(0, 32, 33, token="first"),
            _event(1, 35, 36, token="gap"),
            _event(2, 36, 259, ip=RESTORE, state=_lane_state(259, high=259)),
            _event(3, 259, 32, ip=RESET, state=_lane_state(32, high=259, saved=32)),
            _event(4, 32, 33, token="second"),
        ]
        result = epochs.analyze_capture(_capture(events), 32)

        self.assertEqual(result["unknown_transition_count"], 1)
        self.assertEqual(result["constraint"]["reset_evidence"]["status"], "unknown_before_reset")
        self.assertEqual(result["constraint"]["minimum_allocation_reduction_before_reset"], 0)
        self.assertEqual(result["constraint"]["allocation_groups_before_reset"], [])

    def test_session_and_sequence_coherence(self) -> None:
        base = _capture([_event(0, 0, 1)])
        cross_session = copy.deepcopy(base)
        cross_session["events"][0]["session_id"] = "other"
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(cross_session, 0)

        sequence_gap = copy.deepcopy(base)
        sequence_gap["events"][0]["sequence"] = 1
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(sequence_gap, 0)

    def test_no_reuse_and_counter_boundaries(self) -> None:
        one = epochs.analyze_capture(_capture([_event(0, 0, 1)]), 0)
        self.assertEqual(one["status"], "single_birth")
        self.assertEqual(one["constraint"]["status"], "no_reuse_observed")
        self.assertEqual(one["constraint"]["numeric_constraint_status"], "not_applicable")

        maximum = epochs.MAX_COUNTER
        no_overflow_birth = epochs.analyze_capture(
            _capture([_event(0, maximum, maximum), _event(1, maximum, maximum - 1)]), maximum
        )
        self.assertEqual(no_overflow_birth["birth_count"], 0)
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(_capture([]), -1)
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(_capture([]), epochs.MAX_COUNTER + 1)

    def test_reset_condition_boundaries(self) -> None:
        at_limit = epochs.analyze_capture(_capture([
            _event(0, 0, 1, token="first"),
            _event(1, 256, 0, ip=RESET, state=_lane_state(0, high=256, saved=0, default=0)),
            _event(2, 0, 1, token="second"),
        ]), 0)
        self.assertEqual(at_limit["birth_count"], 2)
        self.assertEqual(at_limit["births"][1]["epoch_boundary"]["kind"], "unknown_transition")

        above_limit = epochs.analyze_capture(_capture([
            _event(0, 0, 1, token="first"),
            _event(1, 257, 0, ip=RESET, state=_lane_state(0, high=257, saved=0, default=0)),
            _event(2, 0, 1, token="second"),
        ]), 0)
        self.assertEqual(above_limit["constraint"]["minimum_allocation_reduction_before_reset"], 1)

    def test_resource_bounds_and_relationship_cap(self) -> None:
        too_many = [_event(index, index, index + 1) for index in range(epochs.MAX_EVENTS + 1)]
        with self.assertRaises(epochs.CaptureError):
            epochs.analyze_capture(_capture(too_many), 0)

        events = [_event(0, 32, 33, token="birth-0")]
        index = 1
        for birth_index in range(1, 10):
            events.append(_event(index, 33, 259, ip=RESTORE,
                                 state=_lane_state(259, high=259)))
            index += 1
            events.append(_event(index, 259, 32, ip=RESET,
                                 state=_lane_state(32, high=259, saved=32)))
            index += 1
            events.append(_event(index, 32, 33, token=f"birth-{birth_index}"))
            index += 1
        result = epochs.analyze_capture(_capture(events), 32)
        self.assertEqual(result["birth_count"], 10)
        self.assertEqual(len(result["relationships"]), epochs.MAX_RELATIONSHIPS)
        self.assertTrue(result["truncated"]["relationships"])
        self.assertLessEqual(len(epochs._encoded(result)), epochs.MAX_OUTPUT_BYTES)

    def test_compare_same_capture_keeps_binding_and_is_not_authority(self) -> None:
        capture = _shaped_capture([("first", 1)], session_id="same-session")
        result = epochs.compare_captures(capture, copy.deepcopy(capture), 0)

        self.assertEqual(result["schema"], epochs.COMPARE_SCHEMA)
        self.assertEqual(result["status"], "same")
        self.assertFalse(result["authority_advanced"])
        self.assertFalse(result["session_binding"]["independent_sessions"])
        self.assertTrue(result["session_binding"]["same_session"])
        self.assertEqual(result["allocation_count_delta"], 0)
        self.assertEqual(result["matched_group_count"], 1)
        self.assertEqual(result["matched_groups"][0]["allocation_count_delta"], 0)

    def test_compare_cross_session_matches_shape_without_matching_tokens(self) -> None:
        base = _shaped_capture([("first", 2), ("second", 1)], session_id="base-session")
        candidate = _shaped_capture([("first", 2), ("second", 1)], session_id="candidate-session")
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "same")
        self.assertTrue(result["session_binding"]["independent_sessions"])
        self.assertFalse(result["session_binding"]["session_tokens_compared"])
        self.assertEqual(result["matched_group_count"], 2)
        self.assertEqual(result["differences"], [])
        self.assertNotEqual(
            result["source_expression_groups"]["base"][0]["codegen_token"],
            result["source_expression_groups"]["candidate"][0]["codegen_token"],
        )

    def test_compare_reports_inserted_group_and_allocation_delta(self) -> None:
        base = _shaped_capture([("first", 1), ("last", 1)], session_id="base-session")
        candidate = _shaped_capture(
            [("first", 1), ("inserted", 1), ("last", 1)], session_id="candidate-session"
        )
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "changed")
        self.assertEqual(result["allocation_count_delta"], 1)
        self.assertEqual(result["matched_group_count"], 2)
        self.assertEqual(
            [item["kind"] for item in result["differences"]], ["group_inserted"]
        )
        self.assertEqual(result["earliest_causal_difference"]["kind"], "group_inserted")

    def test_compare_reports_removed_group(self) -> None:
        base = _shaped_capture([("first", 1), ("removed", 1), ("last", 1)], session_id="base-session")
        candidate = _shaped_capture([("first", 1), ("last", 1)], session_id="candidate-session")
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "changed")
        self.assertEqual(result["allocation_count_delta"], -1)
        self.assertEqual([item["kind"] for item in result["differences"]], ["group_removed"])
        self.assertEqual(result["earliest_causal_difference"]["kind"], "group_removed")

    def test_compare_marks_ambiguous_source_names_unknown(self) -> None:
        base = _capture([
            _event(0, 0, 1, token="base-ambiguous", names=("left", "right"), lane="pcode")
        ], first_before=0, session_id="base-session", lane="pcode")
        candidate = _capture([
            _event(0, 0, 1, token="candidate-ambiguous", names=("left", "right"), lane="pcode")
        ], first_before=0, session_id="candidate-session", lane="pcode")
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "unknown")
        self.assertEqual(result["source_expression_groups"]["base"][0]["unknown_reason"],
                         "ambiguous_source_names")
        self.assertEqual(result["differences"][0]["kind"], "unknown_group")
        self.assertEqual(result["differences"][0]["status"], "UNKNOWN")

    def test_compare_does_not_attribute_decoder_version_drift_to_source(self) -> None:
        base = _shaped_capture([("first", 1)], session_id="base-session")
        candidate = _shaped_capture([("renamed_by_decoder", 1)], session_id="candidate-session")
        candidate["producer"] = {
            "expression_capture": {"sha256": "e" * 64, "size_bytes": 1}
        }
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "unknown")
        self.assertTrue(result["comparison_skipped"])
        self.assertEqual(result["annotation_comparability"]["status"], "UNKNOWN")
        self.assertEqual(result["annotation_comparability"]["reason"], "annotation_decoder_drift")
        self.assertEqual(result["matched_group_count"], 0)
        self.assertEqual(result["differences"][0]["kind"], "annotation_comparability_unknown")
        self.assertNotIn("group_replaced", [item["kind"] for item in result["differences"]])

        candidate.pop("producer")
        result = epochs.compare_captures(base, candidate, 0)
        self.assertEqual(result["annotation_comparability"]["reason"], "annotation_decoder_metadata_missing")
        self.assertEqual(result["matched_group_count"], 0)

    def test_compare_keeps_duplicate_signatures_ambiguous(self) -> None:
        base = _shaped_capture(
            [("same", 1), ("middle", 1), ("same", 1)], session_id="base-session"
        )
        candidate = _shaped_capture(
            [("same", 1), ("middle", 1), ("same", 1)], session_id="candidate-session"
        )
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "unknown")
        self.assertTrue(any(item["kind"] == "unknown_group" for item in result["differences"]))
        self.assertNotEqual(result["matched_group_count"], 3)

    def test_compare_finds_late_change_beyond_published_group_prefix(self) -> None:
        count = epochs.MAX_COMPARE_GROUPS + 2
        base_groups = [(f"name-{index}", 1) for index in range(count)]
        candidate_groups = list(base_groups)
        candidate_groups[-1] = ("late-changed", 1)
        result = epochs.compare_captures(
            _shaped_capture(base_groups, session_id="base-session"),
            _shaped_capture(candidate_groups, session_id="candidate-session"),
            0,
        )

        self.assertTrue(result["truncated"]["base_groups"])
        self.assertTrue(result["truncated"]["candidate_groups"])
        self.assertEqual(result["earliest_causal_difference"]["kind"], "group_replaced")
        self.assertEqual(result["earliest_causal_difference"]["base_group"]["group_index"], count - 1)

    def test_compare_marks_missing_annotations_unknown(self) -> None:
        base = _capture([_event(0, 0, 1, token="base-unknown")], first_before=0,
                        session_id="base-session", lane="pcode")
        candidate = _capture([_event(0, 0, 1, token="candidate-unknown")], first_before=0,
                             session_id="candidate-session", lane="pcode")
        result = epochs.compare_captures(base, candidate, 0)

        self.assertEqual(result["status"], "unknown")
        self.assertGreaterEqual(result["unknown_count"], 1)
        self.assertEqual(result["matched_group_count"], 0)
        self.assertEqual(result["earliest_causal_difference"]["kind"], "unknown_group")
        self.assertEqual(result["source_expression_groups"]["base"][0]["annotation_status"], "UNKNOWN")

    def test_compare_skips_compiler_function_or_lane_drift(self) -> None:
        base = _shaped_capture([("first", 1)], session_id="base-session")
        function_drift = _shaped_capture(
            [("first", 1)], session_id="candidate-session", lane="pcode"
        )
        function_drift["function"] = "different_function"
        for event in function_drift["events"]:
            event["function"] = "different_function"
        result = epochs.compare_captures(base, function_drift, 0)
        self.assertEqual(result["status"], "incompatible")
        self.assertTrue(result["comparison_skipped"])
        self.assertIn("function_drift", result["comparison"]["reasons"])
        self.assertEqual(result["matched_groups"], [])

        lane_drift = _shaped_capture([("first", 1)], session_id="candidate-session", lane="stack")
        result = epochs.compare_captures(base, lane_drift, 0)
        self.assertEqual(result["status"], "incompatible")
        self.assertIn("lane_drift", result["comparison"]["reasons"])

    def test_compare_reports_births_before_reset_and_requested_reuse(self) -> None:
        events = [
            _event(0, 32, 32, token="init", lane="pcode"),
            _event(1, 32, 33, token="first", names=("first",), lhs=("first",), lane="pcode"),
            _event(2, 33, 259, ip=RESTORE, token="restore", lane="pcode",
                   state=_lane_state(259, high=259, saved=32)),
            _event(3, 259, 260, token="middle", names=("middle",), lhs=("middle",), lane="pcode"),
            _event(4, 260, 32, ip=RESET, token="reset", lane="pcode",
                   state=_lane_state(32, high=259, saved=32)),
            _event(5, 32, 33, token="second", names=("second",), lhs=("second",), lane="pcode"),
        ]
        base = _capture(events, first_before=32, session_id="base-session", lane="pcode")
        candidate = copy.deepcopy(base)
        candidate["session_id"] = "candidate-session"
        for event in candidate["events"]:
            event["session_id"] = "candidate-session"
            event["codegen_token"] = "candidate-" + str(event["codegen_token"])
        result = epochs.compare_captures(base, candidate, 32)

        self.assertEqual(result["status"], "same")
        reset = next(
            item for item in result["births_before_each_reset"]["base"]
            if item["kind"] == "reset_to_saved_base"
        )
        self.assertEqual(reset["total_births_before_reset"], 2)
        reuse = result["requested_vreg_reuse"]["base"]
        self.assertEqual(reuse["before_first_reset"]["status"], "single_birth")
        self.assertEqual(reuse["after_first_reset"]["status"], "single_birth")
        self.assertEqual(reuse["overall"]["status"], "reused")

    def test_compare_bounds_source_groups(self) -> None:
        groups = [(f"name-{index}", 1) for index in range(epochs.MAX_COMPARE_GROUPS + 4)]
        result = epochs.compare_captures(
            _shaped_capture(groups, session_id="base-session"),
            _shaped_capture(groups, session_id="candidate-session"),
            0,
        )
        self.assertTrue(result["truncated"]["base_groups"])
        self.assertTrue(result["truncated"]["candidate_groups"])
        self.assertEqual(result["group_counts"]["base"]["total"], len(groups))
        self.assertEqual(result["group_counts"]["base"]["returned"], epochs.MAX_COMPARE_GROUPS)

    def test_cli_rehashes_bindings_and_writes_output(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            files = {}
            for name, content in {
                "source.c": b"source",
                "compiler.exe": b"compiler",
                "baseline.o": b"baseline",
                "output.o": b"output",
            }.items():
                path = root / name
                path.write_bytes(content)
                files[name] = {"path": str(path), "sha256": hashlib.sha256(content).hexdigest(), "size_bytes": len(content)}
            capture = _capture([_event(0, 0, 1)])
            capture["source"] = files["source.c"]
            capture["compiler"] = files["compiler.exe"]
            capture["baseline_object"] = files["baseline.o"]
            capture["output_object"] = files["output.o"]
            capture_path = root / "capture.json"
            output_path = root / "report.json"
            capture_path.write_text(json.dumps(capture), encoding="utf-8")

            pinned = epochs.PINNED_COMPILER_SHA256
            epochs.PINNED_COMPILER_SHA256 = files["compiler.exe"]["sha256"]
            try:
                self.assertEqual(epochs.main([
                    "--capture", str(capture_path), "--vreg", "0", "--out", str(output_path)
                ]), 0)
            finally:
                epochs.PINNED_COMPILER_SHA256 = pinned
            report = json.loads(output_path.read_text(encoding="utf-8"))
            self.assertEqual(report["status"], "single_birth")


if __name__ == "__main__":
    unittest.main()
