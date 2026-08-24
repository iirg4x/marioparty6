from __future__ import annotations

import hashlib
import importlib.util
import json
import os
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest import mock


TOOL = Path(__file__).resolve().parents[1] / "capsule_stack_home_native.py"
SPEC = importlib.util.spec_from_file_location("capsule_stack_home_native_test_target", TOOL)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class FakeBackend:
    """Deterministic backend that models breakpoints and one-step rearming."""

    capabilities = {name: True for name in MODULE.REQUIRED_CAPABILITIES}

    def __init__(
        self,
        *,
        wrong_prefix: bool = False,
        duplicate_list_pointer: bool = False,
        reused_varinfo: bool = False,
        disconnect: bool = False,
        missing_write_step: bool = False,
        partial: bool = False,
        function: str = "mbCapListDebug",
        dynamic_object: bool = False,
        boundary_noise: bool = False,
        skip_inbound_writes: bool = False,
        stack_depth_noise: bool = False,
        generic_growth_stage: str | None = None,
        varinfo_rebind_stage: str | None = None,
        write_hook_sequence: tuple[str, ...] | None = None,
        invalid_object_name: bool = False,
        mutate_object_name: bool = False,
        empty_entry_list: bool = False,
        empty_exit_list: bool = False,
    ) -> None:
        self.wrong_prefix = wrong_prefix
        self.duplicate_list_pointer = duplicate_list_pointer
        self.reused_varinfo = reused_varinfo
        self.disconnect = disconnect
        self.missing_write_step = missing_write_step
        self.partial = partial
        self.function = function
        self.dynamic_object = dynamic_object
        self.boundary_noise = boundary_noise
        self.skip_inbound_writes = skip_inbound_writes
        self.stack_depth_noise = stack_depth_noise
        self.generic_growth_stage = generic_growth_stage
        self.varinfo_rebind_stage = varinfo_rebind_stage
        self.write_hook_sequence = write_hook_sequence
        self.invalid_object_name = invalid_object_name
        self.mutate_object_name = mutate_object_name
        self.empty_entry_list = empty_entry_list
        self.empty_exit_list = empty_exit_list
        self.phase = "entry"
        self.live_reads: list[int] = []
        self.mutations: list[tuple[str, int]] = []
        self.active: set[int] = set()
        self.snapshot_count = 0
        self.field_value = 0
        self.register_value = 0
        self.closed = False

    def read_image(self, address: int, size: int) -> bytes:
        self.live_reads.append(address)
        if self.wrong_prefix and address == int(MODULE.HOOK_BY_ID["object_write_1"]["address"]):
            return bytes([0]) * size
        return bytes.fromhex(next(row["prefix"] for row in MODULE.HOOKS if int(row["address"]) == address))[:size]

    def install_breakpoint(self, address: int) -> None:
        if address in self.active:
            raise MODULE.Rejected(f"duplicate fake install 0x{address:08x}")
        self.active.add(address)
        self.mutations.append(("install", address))

    def read_memory(self, address: int, size: int) -> bytes:
        del address
        return bytes(size)

    def remove_breakpoint(self, address: int) -> None:
        self.active.discard(address)
        self.mutations.append(("remove", address))

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None:
        del thread_id
        self.active.discard(address)
        self.mutations.append(("step", address))
        if rearm:
            self.active.add(address)

    def read_register(self, thread_id: int, name: str) -> int:
        del thread_id
        if name == "ebx":
            return 0x1100 if self.dynamic_object else 0x1000
        if name == "eax":
            return self.register_value
        raise AssertionError(name)

    def snapshot_objects(self) -> list[dict[str, object]]:
        self.snapshot_count += 1
        if self.empty_entry_list and self.snapshot_count == 1:
            return []
        if self.empty_exit_list and self.snapshot_count >= 4:
            return []
        if self.duplicate_list_pointer:
            return [
                {"pointer": 0x1000, "varinfo_pointer": 0x2000, "name": "playerPosCur", "datatype": 1},
                {"pointer": 0x1000, "varinfo_pointer": 0x2000, "name": "playerPosCur", "datatype": 1},
            ]
        varinfo = 0x3000 if self.reused_varinfo and self.snapshot_count >= 3 else 0x2000
        if self.varinfo_rebind_stage == "captured":
            if self.snapshot_count >= 3:
                rows = [
                    {"pointer": 0x1000, "varinfo_pointer": 0x2100},
                    {"pointer": 0x1400, "varinfo_pointer": 0x2200},
                ]
            else:
                rows = [
                    {"pointer": 0x1000, "varinfo_pointer": 0x2000},
                    {"pointer": 0x1300, "varinfo_pointer": 0x2100},
                ]
        elif self.varinfo_rebind_stage == "generic":
            rows = [
                {"pointer": 0x1000, "varinfo_pointer": 0x2000},
                {
                    "pointer": 0x1400 if self.snapshot_count >= 2 else 0x1300,
                    "varinfo_pointer": 0x2100,
                },
            ]
        else:
            rows = [{"pointer": 0x1000, "varinfo_pointer": varinfo}]
        if self.dynamic_object and self.snapshot_count >= 3:
            rows.append({"pointer": 0x1100, "varinfo_pointer": 0x4000})
        growth_threshold = {"pre": 2, "post": 3, "exit": 4}.get(self.generic_growth_stage)
        if growth_threshold is not None and self.snapshot_count >= growth_threshold:
            rows.append({"pointer": 0x1200, "varinfo_pointer": 0x5000})
        names = {
            0x1000: (
                "bad0x1234name"
                if self.invalid_object_name
                else "playerRot" if self.mutate_object_name and self.snapshot_count >= 2 else "playerPosCur"
            ),
            0x1100: "playerRot",
            0x1200: "coinVel",
            0x1300: "coinPos",
            0x1400: "togezoPos",
        }
        for row in rows:
            row["name"] = names[int(row["pointer"])]
            row["datatype"] = 1
        return rows

    def snapshot_varinfo(self, pointer: int) -> dict[str, int]:
        return {"home_value": pointer & 0x7F}

    def read_object_stack_field(self, pointer: int) -> int:
        if pointer not in ({0x1000, 0x1100} if self.dynamic_object else {0x1000}):
            raise MODULE.Rejected("unexpected fake Object pointer")
        return self.field_value

    def write_metadata(self, pointer: int, value: int) -> dict[str, object]:
        if pointer not in ({0x1000, 0x1100} if self.dynamic_object else {0x1000}):
            raise MODULE.Rejected("unexpected fake write pointer")
        return {"read_count": 0, "escape": False}

    def current_function(self) -> str:
        return self.function

    def read_execution_state(self, thread_id: int) -> dict[str, int]:
        del thread_id
        return {
            "eip": 0x4000,
            "esp": 0x1100 if self.phase == "outside" else (0x0F00 if self.phase == "inside" else 0x1000),
            "ebp": 0x1200,
        }

    def run(self, session: MODULE.CaptureSession) -> None:
        session.on_process_started()
        if self.disconnect:
            session.on_disconnect("fake transport disconnected")
            return
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["function_filter"]["address"]), 1)
        session.on_single_step(1)
        if self.partial:
            return
        if self.boundary_noise:
            address = int(MODULE.HOOK_BY_ID[MODULE.WRITE_HOOK_IDS[0]]["address"])
            session.on_breakpoint(address, 1)
            session.on_single_step(1)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["allocation_pre"]["address"]), 1)
        session.on_single_step(1)
        self.phase = "inside"
        if self.stack_depth_noise:
            self.phase = "outside"
            address = int(MODULE.HOOK_BY_ID[MODULE.WRITE_HOOK_IDS[0]]["address"])
            session.on_breakpoint(address, 1)
            session.on_single_step(1)
            self.phase = "inside"
        if not self.skip_inbound_writes:
            for index, hook_id in enumerate(self.write_hook_sequence or MODULE.WRITE_HOOK_IDS):
                self.register_value = 16 + index
                self.field_value = 16 + index
                if self.dynamic_object:
                    self.register_value = 16 + index
                address = int(MODULE.HOOK_BY_ID[hook_id]["address"])
                session.on_breakpoint(address, 1)
                if self.missing_write_step:
                    break
                session.on_single_step(1)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["allocation_post"]["address"]), 1)
        session.on_single_step(1)
        self.phase = "after"
        if self.boundary_noise:
            address = int(MODULE.HOOK_BY_ID[MODULE.WRITE_HOOK_IDS[0]]["address"])
            session.on_breakpoint(address, 1)
            session.on_single_step(1)
        session.on_process_exit(0)

    def close(self) -> None:
        self.closed = True


def auth(function: str = "mbCapListDebug") -> dict[str, object]:
    artifacts = {
        name: {"path": f"{name}.bin", "size": 1, "sha256": f"{index:064x}"}
        for index, name in enumerate(("source", "baseline", "compiler", "producer", "debugger", "emulator", "gdb"), 1)
    }
    artifacts["source"] = {
        "path": "C:/fixture/src/board/capthrow.c",
        "size": 1,
        "sha256": f"{1:064x}",
    }
    authority = {
        "schema": MODULE.AUTHORITY_SCHEMA,
        "source": dict(artifacts["source"]),
        "function": {
            "name": function,
            "sha256": "a" * 64,
            "source_sha256": artifacts["source"]["sha256"],
        },
        "artifacts": {name: dict(row) for name, row in artifacts.items()},
    }
    return {
        "request": {
            "function": function,
            "function_sha256": "a" * 64,
            "argv": ["mwcceppc.exe", "-c", "capsule.c"],
            "cwd": "C:/fixture",
            "authority": authority,
        },
        "request_sha256": "b" * 64,
        "request_path": Path("request.json"),
        "artifacts": artifacts,
    }


def capture_fixture_auth(root: Path) -> dict[str, object]:
    """Authenticated-on-disk fixture for the complete producer boundary."""

    artifacts: dict[str, dict[str, object]] = {}
    for name, data in (
        ("source", b"authenticated capsule source\n"),
        ("baseline", b"authenticated baseline\n"),
        ("compiler", b"authenticated compiler\n"),
        ("producer", b"authenticated producer\n"),
        ("debugger", b"authenticated debugger\n"),
        ("emulator", b"authenticated emulator\n"),
        ("gdb", b"authenticated gdb\n"),
    ):
        path = (
            root / "src" / "board" / "capthrow.c"
            if name == "source"
            else root / f"{name}.bin"
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        row: dict[str, object] = {
            "path": str(path.resolve()),
            "size": path.stat().st_size,
            "sha256": MODULE.sha256(path),
        }
        if name == "compiler":
            row.update(
                {
                    "profile": "GC/2.6 build107",
                    "image_base": "0x00400000",
                    "hooks": [{**hook, "validated": True} for hook in MODULE._request_hook_rows()],
                }
            )
        artifacts[name] = row
    request_path = root / "request.json"
    request_path.write_bytes(b"authenticated request fixture\n")
    output_dir = root / "capture"
    output_dir.mkdir()
    request_sha256 = hashlib.sha256(request_path.read_bytes()).hexdigest()
    authority = {
        "schema": MODULE.AUTHORITY_SCHEMA,
        "source": dict(artifacts["source"]),
        "function": {
            "name": "mbCapListDebug",
            "sha256": "a" * 64,
            "source_sha256": artifacts["source"]["sha256"],
        },
        "artifacts": {name: dict(row) for name, row in artifacts.items()},
    }
    return {
        "request": {
            "function": "mbCapListDebug",
            "function_sha256": "a" * 64,
            "argv": ["mwcceppc.exe", "-c", "capsule.c"],
            "cwd": str(root.resolve()),
            "authority": authority,
        },
        "request_sha256": request_sha256,
        "request_path": request_path,
        "paths": {
            "cwd": root.resolve(),
            "output_dir": output_dir.resolve(),
            "event_stream": output_dir / "events.jsonl",
            "packet": output_dir / "trace.packet.json",
            "candidate": output_dir / "capsule.o",
        },
        "artifacts": artifacts,
    }


def dot_alias(path: Path) -> str:
    separator = "\\" if "\\" in str(path) else "/"
    return f"{path.parent}{separator}.{separator}{path.name}"


def mixed_separator_alias(path: Path) -> str:
    text = str(path)
    if "/" in text:
        index = text.index("/")
        return text[:index] + "\\" + text[index + 1 :]
    index = text.index("\\")
    return text[:index] + "/" + text[index + 1 :]


def case_alias(path: Path) -> str:
    text = str(path)
    return text.upper() if text != text.upper() else text.lower()


class CapsuleStackHomeNativeTests(unittest.TestCase):
    def _capture_fixture(self, root: Path) -> tuple[dict[str, object], dict[str, object]]:
        fixture_auth = capture_fixture_auth(root)
        with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
            packet = MODULE.capture_with_backend(fixture_auth["request_path"], FakeBackend())
        return fixture_auth, packet

    def test_complete_fake_capture_has_canonical_events_and_three_derived_posts(self) -> None:
        backend = FakeBackend()
        packet = MODULE.CaptureSession(auth(), backend).run()
        self.assertEqual(packet["status"], "CAPTURED_UNKNOWN_OWNERSHIP")
        events = packet["events"]
        self.assertEqual([row["sequence"] for row in events], list(range(len(events))))
        self.assertEqual([row["event_id"] for row in events], [f"{'b' * 16}-e{index:06d}" for index in range(len(events))])
        self.assertEqual(events[0]["event_kind"], "function_entry")
        self.assertEqual(events[-1]["event_kind"], "function_exit")
        self.assertEqual(sum(row["event_kind"] == "object_stack_write_pre" for row in events), 3)
        self.assertEqual(sum(row["event_kind"] == "object_stack_write_post" for row in events), 3)
        self.assertEqual(packet["residues"], [
            {"target_slot": 16, "write_count": 1, "read_count": 0, "escape": False, "owner": "UNKNOWN"},
            {"target_slot": 17, "write_count": 1, "read_count": 0, "escape": False, "owner": "UNKNOWN"},
            {"target_slot": 18, "write_count": 1, "read_count": 0, "escape": False, "owner": "UNKNOWN"},
        ])
        for event in events:
            self.assertNotIn("pointer", event)
            self.assertNotIn("address", event)
            self.assertNotIn("thread_id", event)
        tokens = {
            token
            for event in events
            for key in ("object_token", "varinfo_token")
            for token in ([event[key]] if key in event else [])
        }
        self.assertIn("object-000000", tokens)
        self.assertIn("varinfo-000000", tokens)

    def test_allocation_created_object_is_bound_only_at_post_snapshot(self) -> None:
        backend = FakeBackend(dynamic_object=True)
        packet = MODULE.CaptureSession(auth(), backend).run()
        self.assertEqual(packet["status"], "CAPTURED_UNKNOWN_OWNERSHIP")
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["object_tokens"], ["object-000000", "object-000001"])
        self.assertEqual(compiler_list["varinfo_tokens"], ["varinfo-000000", "varinfo-000001"])
        self.assertEqual(compiler_list["object_varinfo_pairs"], [
            {"object_token": "object-000000", "varinfo_token": "varinfo-000000"},
            {"object_token": "object-000001", "varinfo_token": "varinfo-000001"},
        ])
        allocation = [event for event in packet["events"] if event["event_kind"] == "numeric_stack_alloc_pre"][0]
        self.assertEqual(allocation["object_tokens"], ["object-000000"])
        allocation = [event for event in packet["events"] if event["event_kind"] == "numeric_stack_alloc_post"][0]
        self.assertEqual(allocation["object_tokens"], ["object-000000", "object-000001"])
        self.assertEqual(sum(event["event_kind"] == "varinfo_home_snapshot" for event in packet["events"]), 2)
        for event in packet["events"]:
            self.assertNotIn("pointer", event)
            self.assertNotIn("address", event)

    def test_empty_function_entry_list_can_be_populated_at_allocator_boundary(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend(empty_entry_list=True)).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["object_tokens"], ["object-000000"])
        self.assertEqual(compiler_list["objects"][0]["name"], "playerPosCur")

    def test_unlinked_function_exit_list_preserves_captured_inventory(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend(empty_exit_list=True)).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["object_tokens"], ["object-000000"])
        self.assertEqual(compiler_list["objects"][0]["name"], "playerPosCur")

    def test_pre_and_post_boundary_write_noise_is_ignored(self) -> None:
        backend = FakeBackend(boundary_noise=True)
        packet = MODULE.CaptureSession(auth(), backend).run()
        self.assertEqual(
            [event["derived_from"] for event in packet["events"] if event["event_kind"] == "object_stack_write_post"],
            list(MODULE.WRITE_HOOK_IDS),
        )
        reasons = [row["reason"] for row in packet["unknown"]]
        self.assertEqual(len(reasons), 2)
        self.assertTrue(all("outside the authenticated numeric allocation" in reason for reason in reasons))

    def test_same_thread_stack_depth_noise_is_ignored(self) -> None:
        backend = FakeBackend(stack_depth_noise=True)
        packet = MODULE.CaptureSession(auth(), backend).run()
        self.assertEqual(
            [event["derived_from"] for event in packet["events"] if event["event_kind"] == "object_stack_write_post"],
            list(MODULE.WRITE_HOOK_IDS),
        )
        self.assertEqual(len(packet["unknown"]), 1)
        self.assertIn("stack depth", packet["unknown"][0]["reason"])

    def test_list_growth_before_allocation_is_identity_only(self) -> None:
        backend = FakeBackend(boundary_noise=True, generic_growth_stage="pre")
        packet = MODULE.CaptureSession(auth(), backend).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        allocation_pre = next(event for event in packet["events"] if event["event_kind"] == "numeric_stack_alloc_pre")
        self.assertEqual(compiler_list["object_tokens"], ["object-000000", "object-000001"])
        self.assertEqual(allocation_pre["object_tokens"], compiler_list["object_tokens"])
        self.assertEqual(len([event for event in packet["events"] if event["event_kind"] == "object_stack_write_post"]), 3)
        self.assertEqual(len(packet["unknown"]), 2)

    def test_list_growth_after_allocation_is_not_allocation_evidence(self) -> None:
        backend = FakeBackend(generic_growth_stage="exit")
        packet = MODULE.CaptureSession(auth(), backend).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        allocation_post = next(event for event in packet["events"] if event["event_kind"] == "numeric_stack_alloc_post")
        self.assertEqual(compiler_list["object_tokens"], ["object-000000", "object-000001"])
        self.assertEqual(allocation_post["object_tokens"], ["object-000000"])
        snapshots = [event for event in packet["events"] if event["event_kind"] == "varinfo_home_snapshot"]
        self.assertEqual(len(snapshots), 2)
        self.assertEqual(len([event for event in packet["events"] if event["event_kind"] == "object_stack_write_post"]), 3)

    def test_authenticated_write_hook_multiplicity_is_recorded_without_alternative_site_requirement(self) -> None:
        hook_id = MODULE.WRITE_HOOK_IDS[0]
        packet = MODULE.CaptureSession(
            auth(), FakeBackend(write_hook_sequence=(hook_id,) * 38)
        ).run()
        pre_events = [event for event in packet["events"] if event["event_kind"] == "object_stack_write_pre"]
        post_events = [event for event in packet["events"] if event["event_kind"] == "object_stack_write_post"]
        self.assertEqual(len(pre_events), 38)
        self.assertEqual(len(post_events), 38)
        self.assertEqual([event["write_site"] for event in pre_events], [
            f"0x{int(MODULE.HOOK_BY_ID[hook_id]['address']):08x}"
        ] * 38)
        self.assertEqual([event["derived_from"] for event in post_events], [hook_id] * 38)
        self.assertEqual(packet["residues"][0]["write_count"], 1)
        self.assertEqual(sum(row["write_count"] for row in packet["residues"]), 38)

    def test_missing_in_boundary_writes_remains_fail_closed(self) -> None:
        backend = FakeBackend(boundary_noise=True, skip_inbound_writes=True)
        with self.assertRaisesRegex(MODULE.Rejected, "numeric allocation pair does not enclose"):
            MODULE.CaptureSession(auth(), backend).run()

    def test_live_prefixes_are_checked_before_first_mutation(self) -> None:
        backend = FakeBackend()
        MODULE.CaptureSession(auth(), backend).run()
        first_mutation = next(index for index, item in enumerate(backend.mutations) if item[0] == "install")
        self.assertEqual(first_mutation, 0)
        self.assertEqual(len(backend.live_reads), len(MODULE.HOOKS))

    def test_wrong_prefix_fails_without_breakpoint_mutation(self) -> None:
        backend = FakeBackend(wrong_prefix=True)
        with self.assertRaisesRegex(MODULE.Rejected, "live hook prefix mismatch"):
            MODULE.CaptureSession(auth(), backend).run()
        self.assertEqual(backend.mutations, [])

    def test_transport_capability_gap_fails_closed(self) -> None:
        backend = FakeBackend()
        backend.capabilities = dict(backend.capabilities)
        backend.capabilities["single_step"] = False
        with self.assertRaisesRegex(MODULE.Rejected, "capability gap"):
            MODULE.CaptureSession(auth(), backend)

    def test_disconnect_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "disconnected"):
            MODULE.CaptureSession(auth(), FakeBackend(disconnect=True)).run()

    def test_missing_write_post_step_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "missing post-step|derived single-step"):
            MODULE.CaptureSession(auth(), FakeBackend(missing_write_step=True)).run()

    def test_duplicate_list_pointer_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "duplicate/reused Object pointer"):
            MODULE.CaptureSession(auth(), FakeBackend(duplicate_list_pointer=True)).run()

    def test_reused_pointer_with_changed_varinfo_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "captured Object identity was recycled"):
            MODULE.CaptureSession(auth(), FakeBackend(reused_varinfo=True)).run()

    def test_recycled_varinfo_pointer_for_unrelated_object_is_a_new_generation(self) -> None:
        packet = MODULE.CaptureSession(
            auth(), FakeBackend(varinfo_rebind_stage="generic")
        ).run()
        self.assertEqual(packet["status"], "CAPTURED_UNKNOWN_OWNERSHIP")
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["object_tokens"], [
            "object-000000", "object-000001", "object-000002",
        ])
        self.assertEqual(compiler_list["varinfo_tokens"], [
            "varinfo-000000", "varinfo-000001", "varinfo-000002",
        ])
        self.assertEqual(packet["unknown"], [])

    def test_recycled_varinfo_pointer_for_captured_object_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "captured Object identity was recycled"):
            MODULE.CaptureSession(
                auth(), FakeBackend(varinfo_rebind_stage="captured")
            ).run()

    def test_compiler_list_pair_relation_rejects_cross_generation_join(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend()).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        compiler_list["object_varinfo_pairs"][0]["varinfo_token"] = "varinfo-000001"
        with self.assertRaisesRegex(MODULE.Rejected, "duplicate compiler-list Object/VarInfo pair|coverage|not token bound"):
            MODULE.canonical_event_bytes(packet["events"])

    def test_compiler_object_inventory_rejects_varinfo_binding_tamper(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend(dynamic_object=True)).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        compiler_list["objects"][0]["varinfo_token"] = "varinfo-000001"
        with self.assertRaisesRegex(MODULE.Rejected, "inventory VarInfo binding mismatch"):
            MODULE.canonical_event_bytes(packet["events"])

    def test_noncanonical_compiler_object_name_is_unknown_and_not_serialized(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend(invalid_object_name=True)).run()
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["objects"][0]["name_status"], "UNKNOWN")
        self.assertIsNone(compiler_list["objects"][0]["name"])
        self.assertNotIn("bad0x1234name", json.dumps(packet, sort_keys=True))

    def test_compiler_object_metadata_mutation_within_generation_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "metadata changed within one capture generation"):
            MODULE.CaptureSession(auth(), FakeBackend(mutate_object_name=True)).run()

    def test_partial_function_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "without process exit"):
            MODULE.CaptureSession(auth(), FakeBackend(partial=True)).run()

    def test_nonzero_compiler_exit_fails_closed(self) -> None:
        backend = FakeBackend()
        original = backend.run

        def nonzero(session: MODULE.CaptureSession) -> None:
            original(session)

        backend.run = nonzero  # type: ignore[method-assign]
        with self.assertRaisesRegex(MODULE.Rejected, "compiler process exited"):
            session = MODULE.CaptureSession(auth(), backend)
            session.on_process_started()
            session.on_process_exit(1)

    def test_noncanonical_function_fails_closed(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "canonical C function identifier"):
            MODULE.CaptureSession(auth("CapEffCrackCreate;bad"), FakeBackend())

    def test_arbitrary_authorized_board_function_uses_existing_capture_contract(self) -> None:
        packet = MODULE.CaptureSession(
            auth("mbev_CapTogezo"), FakeBackend(function="mbev_CapTogezo")
        ).run()
        self.assertEqual(packet["function"], "mbev_CapTogezo")
        self.assertEqual(packet["binding"]["function"], "mbev_CapTogezo")
        compiler_list = next(event for event in packet["events"] if event["event_kind"] == "compiler_list")
        self.assertEqual(compiler_list["objects"][0], {
            "object_token": "object-000000",
            "varinfo_token": "varinfo-000000",
            "name": "playerPosCur",
            "name_status": "EXACT",
            "datatype": 1,
        })
        home = next(event for event in packet["events"] if event["event_kind"] == "varinfo_home_snapshot")
        self.assertEqual(home["varinfo_token"], compiler_list["objects"][0]["varinfo_token"])

    def test_authority_function_mismatch_fails_closed(self) -> None:
        context = auth("mbev_CapTogezo")
        context["request"]["authority"]["function"]["name"] = "mbev_CapPatapata"
        with self.assertRaisesRegex(MODULE.Rejected, "authority function name mismatch"):
            MODULE.CaptureSession(context, FakeBackend(function="mbev_CapTogezo"))

    def test_arbitrary_function_without_external_authority_fails_closed(self) -> None:
        context = auth("mbev_CapTogezo")
        del context["request"]["authority"]
        with self.assertRaisesRegex(MODULE.Rejected, "request.authority"):
            MODULE.CaptureSession(context, FakeBackend(function="mbev_CapTogezo"))

    def test_non_board_source_authority_fails_closed(self) -> None:
        context = auth("mbev_CapTogezo")
        non_board = "C:/fixture/src/game/capthrow.c"
        context["artifacts"]["source"]["path"] = non_board
        context["request"]["authority"]["source"]["path"] = non_board
        with self.assertRaisesRegex(MODULE.Rejected, "Board C source path"):
            MODULE.CaptureSession(context, FakeBackend(function="mbev_CapTogezo"))

    def test_cap_select_masu_player_keeps_existing_authority_contract(self) -> None:
        packet = MODULE.CaptureSession(auth("CapSelectMasuPlayer"), FakeBackend(function="CapSelectMasuPlayer")).run()
        self.assertEqual(packet["function"], "CapSelectMasuPlayer")
        self.assertEqual(packet["binding"]["function"], "CapSelectMasuPlayer")

    def test_canonical_event_bytes_rejects_reordered_or_pointerful_events(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend()).run()
        events = packet["events"]
        events[1]["sequence"] = 99
        with self.assertRaisesRegex(MODULE.Rejected, "sequence"):
            MODULE.canonical_event_bytes(events)
        events = packet["events"]
        events[1]["sequence"] = 1
        events[1]["raw_address"] = 0x1000
        with self.assertRaisesRegex(MODULE.Rejected, "raw pointer/address"):
            MODULE.canonical_event_bytes(events)

    def test_closed_hook_manifest_rejects_missing_duplicate_reordered_and_unknown(self) -> None:
        hooks = MODULE._request_hook_rows()
        for variant in (hooks[:-1], hooks + [dict(hooks[-1])], list(reversed(hooks))):
            with self.assertRaisesRegex(MODULE.Rejected, "complete pinned hook set|order"):
                MODULE._validate_hook_rows(variant, "test.hooks")
        duplicate = [dict(row) for row in hooks]
        duplicate[1]["id"] = duplicate[0]["id"]
        with self.assertRaisesRegex(MODULE.Rejected, "duplicate|order"):
            MODULE._validate_hook_rows(duplicate, "test.hooks")
        unknown = [dict(row) for row in hooks]
        unknown[0]["id"] = "unknown_role"
        with self.assertRaisesRegex(MODULE.Rejected, "order"):
            MODULE._validate_hook_rows(unknown, "test.hooks")

    def test_event_recursive_allowlist_rejects_alias_owner_path_and_boolean_values(self) -> None:
        cases = (
            ("owner", "field allowlist"),
            ("path", "field allowlist"),
            ("event_id", "non-canonical"),
        )
        for key, message in cases:
            packet = MODULE.CaptureSession(auth(), FakeBackend()).run()
            events = packet["events"]
            if key == "event_id":
                events[2][key] = "owner/forged"
            else:
                events[2][key] = "forged"
            with self.assertRaisesRegex(MODULE.Rejected, message):
                MODULE.canonical_event_bytes(events)
        events = packet["events"]
        events[0]["sequence"] = True
        with self.assertRaisesRegex(MODULE.Rejected, "integer"):
            MODULE.canonical_event_bytes(events)

    def test_event_tokens_and_pairs_reject_cross_allocation_or_missing_write_site(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend()).run()
        events = packet["events"]
        events[3]["allocation_id"] = "alloc-999999"
        with self.assertRaisesRegex(MODULE.Rejected, "allocation pair|allocation"):
            MODULE.canonical_event_bytes(events)
        packet = MODULE.CaptureSession(auth(), FakeBackend()).run()
        events = packet["events"]
        first_write = next(index for index, row in enumerate(events) if row["event_kind"] == "object_stack_write_pre")
        events[first_write]["write_site"] = events[first_write + 2]["write_site"]
        with self.assertRaisesRegex(MODULE.Rejected, "closed write-hook chronology|Object write site|pair chronology"):
            MODULE.canonical_event_bytes(events)

    def test_duplicate_json_keys_are_rejected(self) -> None:
        with self.assertRaisesRegex(MODULE.Rejected, "duplicate JSON key"):
            MODULE._load_json('{"schema":"one","schema":"two"}', "fixture")

    def test_backend_runtime_exception_is_normalized(self) -> None:
        backend = FakeBackend()

        def explode(_session: MODULE.CaptureSession) -> None:
            raise RuntimeError("transport exploded")

        backend.run = explode  # type: ignore[method-assign]
        with self.assertRaisesRegex(MODULE.Rejected, "native transport failure: RuntimeError"):
            MODULE.CaptureSession(auth(), backend).run()

    def test_postflight_failure_removes_event_and_packet_outputs(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth = auth()
            fixture_auth["request_path"] = root / "request.json"
            fixture_auth["paths"] = {
                "event_stream": root / "events.jsonl",
                "packet": root / "trace.packet.json",
            }
            fixture_auth["request_path"].write_text("fixture", encoding="utf-8")
            backend = FakeBackend()
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth), mock.patch.object(
                MODULE, "postflight", side_effect=MODULE.Rejected("postflight identity changed")
            ):
                with self.assertRaisesRegex(MODULE.Rejected, "postflight identity changed"):
                    MODULE.capture_with_backend(fixture_auth["request_path"], backend)
            self.assertFalse(fixture_auth["paths"]["event_stream"].exists())
            self.assertFalse(fixture_auth["paths"]["packet"].exists())
            self.assertTrue(backend.closed)

    def test_capture_with_backend_round_trip_has_a_valid_final_seal(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                validated = MODULE.validate_packet(packet_path)
            self.assertEqual(validated, packet)
            self.assertEqual(
                packet["packet_sha256"],
                MODULE.canonical_hash({key: value for key, value in packet.items() if key != "packet_sha256"}),
            )

    def test_summarize_joins_sealed_identity_to_stack_home_without_ownership(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            output_path = packet_path.parent / "summary.json"
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                summary = MODULE.summarize_packet(
                    str(packet_path), ["playerPosCur"], str(output_path)
                )
            self.assertEqual(summary["schema"], MODULE.SUMMARY_SCHEMA)
            self.assertEqual(summary["status"], MODULE.SUMMARY_STATUS)
            self.assertFalse(summary["authority_advanced"])
            self.assertFalse(summary["board_admission"])
            self.assertFalse(summary["exactness_claim"])
            mapping = summary["mappings"][0]
            self.assertEqual(mapping["name"], "playerPosCur")
            self.assertEqual(mapping["object_token"], "object-000000")
            self.assertEqual(mapping["varinfo_token"], "varinfo-000000")
            self.assertEqual(mapping["mapped_slots"], [16, 17, 18])
            self.assertEqual(mapping["owner"], "UNKNOWN")
            self.assertEqual(len(mapping["stack_home_writes"]), 3)
            self.assertTrue(all(row["write_observed"] for row in mapping["stack_home_writes"]))
            self.assertEqual(
                summary["summary_sha256"],
                MODULE.canonical_hash(
                    {key: value for key, value in summary.items() if key != "summary_sha256"}
                ),
            )
            self.assertEqual(json.loads(output_path.read_text(encoding="utf-8")), summary)
            self.assertEqual(summary["packet"]["packet_sha256"], packet["packet_sha256"])

    def test_summarize_is_deterministic_for_the_same_packet_and_name_order(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            first = MODULE._summarize_validated_packet(packet, packet_path, ["playerPosCur"])
            second = MODULE._summarize_validated_packet(packet, packet_path, ["playerPosCur"])
            self.assertEqual(first, second)
            self.assertEqual(first["summary_sha256"], second["summary_sha256"])

    def test_summarize_rejects_duplicate_unavailable_and_unwritten_names(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            with self.assertRaisesRegex(MODULE.Rejected, "duplicate compiler Object name"):
                MODULE._summarize_validated_packet(
                    packet, packet_path, ["playerPosCur", "playerPosCur"]
                )
            with self.assertRaisesRegex(MODULE.Rejected, "unavailable: coinVel"):
                MODULE._summarize_validated_packet(packet, packet_path, ["coinVel"])

        dynamic = MODULE.CaptureSession(auth(), FakeBackend(dynamic_object=True)).run()
        with self.assertRaisesRegex(MODULE.Rejected, r"no authenticated Object\+0x2e"):
            MODULE._summarize_validated_packet(
                dynamic, Path("unneeded-after-rejection.packet.json"), ["playerPosCur"]
            )

    def test_summarize_rejects_ambiguous_exact_name(self) -> None:
        packet = MODULE.CaptureSession(auth(), FakeBackend(dynamic_object=True)).run()
        compiler_list = next(
            event for event in packet["events"] if event["event_kind"] == "compiler_list"
        )
        compiler_list["objects"][1]["name"] = "playerPosCur"
        with self.assertRaisesRegex(MODULE.Rejected, "ambiguous: playerPosCur"):
            MODULE._summarize_validated_packet(
                packet, Path("unneeded-after-rejection.packet.json"), ["playerPosCur"]
            )

    def test_summarize_command_requires_name_and_output(self) -> None:
        args = MODULE.parser().parse_args(
            [
                "summarize",
                "C:/capture/trace.packet.json",
                "--name",
                "playerPosCur",
                "--output",
                "C:/capture/summary.json",
            ]
        )
        self.assertEqual(args.command, "summarize")
        self.assertEqual(args.name, ["playerPosCur"])
        self.assertEqual(args.output, "C:/capture/summary.json")

    def test_validate_accepts_canonical_paths_with_hex_components(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory) / "0x0" / "0x123"
            root.mkdir(parents=True)
            fixture_auth, packet = self._capture_fixture(root)
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                self.assertEqual(MODULE.validate_packet(str(fixture_auth["paths"]["packet"])), packet)

    def test_validate_rejects_dot_alias_for_authenticated_packet_path(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet has non-canonical path spelling"):
                    MODULE.validate_packet(dot_alias(packet_path))

    def test_validate_rejects_mixed_separator_alias_for_authenticated_packet_path(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet has non-canonical path spelling"):
                    MODULE.validate_packet(mixed_separator_alias(packet_path))

    def test_validate_rejects_case_alias_for_authenticated_packet_path(self) -> None:
        if os.name != "nt":
            self.skipTest("case-alias identity requires the Windows filesystem")
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet has non-canonical path spelling"):
                    MODULE.validate_packet(case_alias(packet_path))

    def test_validate_rejects_dot_alias_for_event_stream_path(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            packet = json.loads(packet_path.read_text(encoding="utf-8"))
            event_path = Path(packet["event_stream"]["path"])
            packet["event_stream"]["path"] = dot_alias(event_path)
            packet_path.write_text(json.dumps(MODULE.seal(packet), sort_keys=True), encoding="utf-8")
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet.event_stream.path has non-canonical path spelling"):
                    MODULE.validate_packet(str(packet_path))

    def test_validate_rejects_mixed_separator_alias_for_event_stream_path(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            packet = json.loads(packet_path.read_text(encoding="utf-8"))
            event_path = Path(packet["event_stream"]["path"])
            packet["event_stream"]["path"] = mixed_separator_alias(event_path)
            packet_path.write_text(json.dumps(MODULE.seal(packet), sort_keys=True), encoding="utf-8")
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet.event_stream.path has non-canonical path spelling"):
                    MODULE.validate_packet(str(packet_path))

    def test_validate_rejects_leading_or_trailing_whitespace_event_stream_path_before_text_normalization(self) -> None:
        for whitespace in (" ", "\t"):
            with self.subTest(whitespace=repr(whitespace)):
                with TemporaryDirectory() as directory:
                    root = Path(directory)
                    fixture_auth, _packet = self._capture_fixture(root)
                    packet_path = fixture_auth["paths"]["packet"]
                    packet = json.loads(packet_path.read_text(encoding="utf-8"))
                    event_path = packet["event_stream"]["path"]
                    packet["event_stream"]["path"] = whitespace + event_path + whitespace
                    packet_path.write_text(json.dumps(MODULE.seal(packet), sort_keys=True), encoding="utf-8")
                    with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                        with self.assertRaisesRegex(MODULE.Rejected, "packet.event_stream.path has non-canonical path spelling"):
                            MODULE.validate_packet(str(packet_path))

    def test_validate_rejects_case_alias_for_event_stream_path(self) -> None:
        if os.name != "nt":
            self.skipTest("case-alias identity requires the Windows filesystem")
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            packet = json.loads(packet_path.read_text(encoding="utf-8"))
            event_path = Path(packet["event_stream"]["path"])
            packet["event_stream"]["path"] = case_alias(event_path)
            packet_path.write_text(json.dumps(MODULE.seal(packet), sort_keys=True), encoding="utf-8")
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet.event_stream.path has non-canonical path spelling"):
                    MODULE.validate_packet(str(packet_path))

    def test_validate_rejects_a_packet_copied_outside_authenticated_output_path(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            copied_path = root / "copied.packet.json"
            copied_path.write_bytes(packet_path.read_bytes())
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet path is not the authenticated output"):
                    MODULE.validate_packet(copied_path)

    def test_validate_rejects_event_function_different_from_packet_function(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture_auth, _packet = self._capture_fixture(root)
            packet_path = fixture_auth["paths"]["packet"]
            packet = json.loads(packet_path.read_text(encoding="utf-8"))
            for event in packet["events"]:
                event["function"] = "CapCheckComPath"
            event_bytes = MODULE.canonical_event_bytes(packet["events"])
            event_path = Path(packet["event_stream"]["path"])
            event_path.write_bytes(event_bytes)
            packet["event_stream"] = {
                "path": str(event_path),
                "size_bytes": len(event_bytes),
                "sha256": hashlib.sha256(event_bytes).hexdigest(),
            }
            packet_path.write_text(json.dumps(MODULE.seal(packet), sort_keys=True), encoding="utf-8")
            with mock.patch.object(MODULE, "authenticate_request", return_value=fixture_auth):
                with self.assertRaisesRegex(MODULE.Rejected, "packet/event function binding"):
                    MODULE.validate_packet(packet_path)

    def test_compiler_source_operand_resolves_against_authenticated_cwd(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            compiler_cwd = root / "compiler-cwd"
            output_dir = compiler_cwd / "capture"
            compiler_cwd.mkdir()
            output_dir.mkdir()
            source = compiler_cwd / "capsule.c"
            source.write_text("fixture\n", encoding="utf-8")
            _argv, candidate = MODULE._compiler_args(
                ["-c", "capsule.c", "-o", "capture/capsule.o"],
                source,
                output_dir,
                compiler_cwd,
            )
            self.assertEqual(candidate, output_dir / "capsule.o")

    def test_compiler_args_rejects_a_duplicated_compiler_executable(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output_dir = root / "capture"
            output_dir.mkdir()
            source = root / "capsule.c"
            source.write_text("fixture\n", encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "duplicated compiler executable"):
                MODULE._compiler_args(
                    ["mwcceppc.exe", "-c", "capsule.c", "-o", "capture/capsule.o"],
                    source,
                    output_dir,
                    root,
                )

    def test_native_object_list_rejects_cycles_and_truncated_nodes(self) -> None:
        backend = MODULE.NativeWow64Backend(None, 0, 0)
        backend.base = MODULE.KNOWN_IMAGE_BASE
        with mock.patch.object(backend, "_runtime", side_effect=lambda address: address), mock.patch.object(
            backend, "_u8", return_value=1
        ), mock.patch.object(backend, "_read_name", return_value="fixture"), mock.patch.object(
            backend, "_u32_required", side_effect=[0x1000, 0x2000, 0x3000, 0x1000]
        ):
            with self.assertRaisesRegex(MODULE.Rejected, "Object list contains a cycle"):
                backend.snapshot_objects()

        backend = MODULE.NativeWow64Backend(None, 0, 0)
        backend.base = MODULE.KNOWN_IMAGE_BASE
        with mock.patch.object(backend, "_runtime", side_effect=lambda address: address), mock.patch.object(
            backend, "_u32_required", side_effect=[MODULE.Rejected("native Object list node is truncated")]
        ):
            with self.assertRaisesRegex(MODULE.Rejected, "truncated"):
                backend.snapshot_objects()

    def test_native_cleanup_restoration_failure_is_surfaceable_and_retains_evidence(self) -> None:
        backend = MODULE.NativeWow64Backend(None, 0, 0)
        backend.breakpoints[0x1234] = 0x90

        def fail_write(_address: int, _data: bytes) -> None:
            raise RuntimeError("write protection")

        backend._write = fail_write  # type: ignore[method-assign]
        with self.assertRaisesRegex(MODULE.Rejected, "restoration failed"):
            backend.close()
        self.assertEqual(backend.cleanup_evidence["failed_breakpoints"], [0x1234])
        self.assertIn(0x1234, backend.breakpoints)

    def test_native_thread_cleanup_deduplicates_aliases_and_repeated_close(self) -> None:
        native = mock.Mock()
        native.kernel32.CloseHandle.return_value = 1
        backend = MODULE.NativeWow64Backend(native, 0, 41)
        backend.threads[7] = 41
        backend.exited = True
        backend.close()
        backend.close()
        native.kernel32.CloseHandle.assert_called_once_with(41)
        self.assertEqual(backend.cleanup_evidence["closed_threads"], [0])

    def test_native_thread_cleanup_failure_is_cached_without_reclosing(self) -> None:
        native = mock.Mock()
        native.kernel32.CloseHandle.return_value = 0
        backend = MODULE.NativeWow64Backend(native, 0, 41)
        backend.exited = True
        with self.assertRaisesRegex(MODULE.Rejected, "handle cleanup failed"):
            backend.close()
        with self.assertRaisesRegex(MODULE.Rejected, "handle cleanup failed"):
            backend.close()
        native.kernel32.CloseHandle.assert_called_once_with(41)

    def test_native_thread_cleanup_skips_invalid_and_zero_handles(self) -> None:
        native = mock.Mock()
        native.kernel32.GetHandleInformation.return_value = 0
        native.ERROR_INVALID_HANDLE = 6
        with mock.patch.object(MODULE.ctypes, "get_last_error", return_value=6):
            backend = MODULE.NativeWow64Backend(native, 0, 0)
            backend.threads[0] = 0
            backend.threads[7] = 41
            backend.threads[8] = 0
            backend.exited = True
            backend.close()
        native.kernel32.CloseHandle.assert_not_called()
        self.assertEqual(backend.cleanup_evidence["skipped_threads"], [0, 7])


if __name__ == "__main__":
    unittest.main()
